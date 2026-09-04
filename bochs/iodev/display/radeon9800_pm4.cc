/////////////////////////////////////////////////////////////////////////
// $Id$
/////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025-2026  The Bochs Project
//
//  ATI Radeon 9800 (R350) emulation: PM4 command processor (CP).
//
//  Drivers submit 2D and 3D work as PM4 packets in a ring buffer in
//  system memory (bus-mastered through the AGP aperture or the PCI GART)
//  or in local video memory. Execution is split across two threads: the
//  CPU thread copies ring dwords into a device-local FIFO on every write
//  pointer write (the "pump"), splicing in indirect buffers, and the CP
//  thread parses and executes the packets from that FIFO. The PIO command
//  stream aperture feeds the same FIFO.
//
//  This library is free software; you can redistribute it and/or
//  modify it under the terms of the GNU Lesser General Public
//  License as published by the Free Software Foundation; either
//  version 2 of the License, or (at your option) any later version.
//
//  This library is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with this library; if not, write to the Free Software
//  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
//
/////////////////////////////////////////////////////////////////////////

#define BX_PLUGGABLE

#include "iodev.h"
#if BX_SUPPORT_PCI && BX_SUPPORT_RADEON9800

#include "vgacore.h"
#include "pci.h"
#include "ddc.h"
#include "radeon9800_regs.h"
#include "radeon9800.h"
#include "virt_timer.h"

#define LOG_THIS BX_R9800_THIS

// Thread-local marker: set on the CP thread only, so register dispatch
// can tell engine pokes coming from the packet executor apart from
// direct CPU accesses.
#if defined(_MSC_VER)
static __declspec(thread) int radeon9800_on_cp = 0;
#else
static __thread int radeon9800_on_cp = 0;
#endif

bool bx_radeon9800_c::on_cp_thread(void)
{
  return radeon9800_on_cp != 0;
}

BX_THREAD_FUNC(radeon9800_cp_thread_func, indata)
{
  bx_radeon9800_c *dev = (bx_radeon9800_c *)indata;
  radeon9800_on_cp = 1;
  dev->cp_thread_main();
  BX_THREAD_EXIT;
}

// ---------------------------------------------------------------------
// Lifecycle
// ---------------------------------------------------------------------

void bx_radeon9800_c::pm4_reset(void)
{
  pm4_drain_wait();
  if (cp_thread_started)
    bx_virt_timer.deactivate_timer(pump_timer_id);
  cp_rb_base = 0;
  cp_rb_cntl = 0;
  cp_rb_rptr_addr = 0;
  cp_rb_rptr = 0;
  cp_rb_wptr = 0;
  cp_rb_wptr_delay = 0;
  cp_ib_base = 0;
  cp_ib_bufsz = 0;
  cp_csq_cntl = 0;
  cp_csq_mode = 0;
  cp_resync_addr = 0;
  cp_resync_data = 0;
  cp_me_cntl = 0;
  cp_me_ram_addr = 0;
  cp_me_ram_raddr = 0;
  cp_csq_addr = 0;
  scratch_umsk = 0;
  scratch_addr = 0;
  memset(gui_scratch, 0, sizeof(gui_scratch));
  cp_retire_rptr = 0;
  cp_shadow_last = 0xffffffff;
  pm4_ind_busy = false;
  pm4_ind_pending = false;
  pump_frame_rem = 0;
  pump_ib_state = 0;
  pump_ib_addr = 0;
}

void bx_radeon9800_c::pm4_thread_init(void)
{
  if (cp_thread_started)
    return;
  cp_fifo = new Bit32u[R9800_CP_FIFO_DWORDS];
  cp_fifo_rptr = new Bit32u[R9800_CP_FIFO_DWORDS];
  cp_fifo_tag = new Bit8u[R9800_CP_FIFO_DWORDS];
  cp_pl = new Bit32u[R9800_PM4_MAX_PAYLOAD];
  ind_pl = new Bit32u[R9800_PM4_MAX_PAYLOAD];
  cp_fifo_rd = 0;
  cp_fifo_wr = 0;
  cp_retire_rptr = 0;
  cp_retire_pending = 0;
  cp_batch_pending = 0;
  cp_shadow_last = 0xffffffff;
  cp_executing = 0;
  cp_drain_req = 0;
  cp_abort = 0;
  gui_idle_event = 0;
  pump_frame_rem = 0;
  pump_ib_state = 0;
  BX_INIT_MUTEX(cp_mutex);
  bx_create_sem(&cp_wake_sem);
  bx_create_sem(&cp_idle_sem);
  bx_create_sem(&cp_flip_sem);
  cp_thread_run = true;
  cp_thread_started = true;
  BX_THREAD_CREATE(radeon9800_cp_thread_func, this, cp_thread_var);
}

void bx_radeon9800_c::pm4_thread_close(void)
{
  if (!cp_thread_started)
    return;
  cp_thread_run = false;
  cp_abort = 1;
  bx_set_sem(&cp_wake_sem);
  bx_set_sem(&cp_flip_sem);
  BX_THREAD_JOIN(cp_thread_var);
  bx_destroy_sem(&cp_wake_sem);
  bx_destroy_sem(&cp_idle_sem);
  bx_destroy_sem(&cp_flip_sem);
  BX_FINI_MUTEX(cp_mutex);
  delete [] cp_fifo; cp_fifo = NULL;
  delete [] cp_fifo_rptr; cp_fifo_rptr = NULL;
  delete [] cp_fifo_tag; cp_fifo_tag = NULL;
  delete [] cp_pl; cp_pl = NULL;
  delete [] ind_pl; ind_pl = NULL;
  cp_thread_started = false;
}

// ---------------------------------------------------------------------
// Engine state predicates
// ---------------------------------------------------------------------

Bit32u bx_radeon9800_c::pm4_ring_mask(void)
{
  Bit32u l2 = cp_rb_cntl & R9800_RB_BUFSZ_MASK;
  if ((l2 == 0) || (l2 > 22))
    return 0x3ffff;
  return (1u << (l2 + 1)) - 1;
}

// The primary stream is bus-mastered in the PRIBM modes
bool bx_radeon9800_c::pm4_ring_bm(void)
{
  Bit32u mode = cp_csq_cntl >> R9800_CSQ_MODE_SHIFT;
  return (mode == R9800_CSQ_PRIBM_INDDIS) || (mode == R9800_CSQ_PRIBM_INDBM);
}

bool bx_radeon9800_c::pm4_active(void)
{
  return (cp_fifo_rd != cp_fifo_wr) || cp_executing || cp_batch_pending ||
         (pm4_ring_bm() && (cp_rb_rptr != (cp_rb_wptr & pm4_ring_mask())));
}

static BX_CPP_INLINE Bit32u radeon9800_cp_fifo_space(Bit32u wr, Bit32u rd)
{
  return R9800_CP_FIFO_DWORDS - (wr - rd);
}

// Wait for the executor to free `need` FIFO dwords (CPU thread)
void bx_radeon9800_c::cp_fifo_reserve(Bit32u wr, Bit32u need)
{
  int waited = 0;
  if (radeon9800_cp_fifo_space(wr, cp_fifo_rd) >= need)
    return;
  int was_draining = cp_drain_req;
  cp_drain_req = 1;
  bx_set_sem(&cp_flip_sem);
  while (cp_thread_run && (radeon9800_cp_fifo_space(wr, cp_fifo_rd) < need) && (waited < 250)) {
    bx_set_sem(&cp_wake_sem);
    bx_wait_sem(&cp_idle_sem);
    waited++;
  }
  cp_drain_req = was_draining;
  if (radeon9800_cp_fifo_space(wr, cp_fifo_rd) < need)
    BX_ERROR(("IB splice gave up waiting for FIFO space: need=%u", need));
}

// Copy one indirect buffer into the FIFO behind its submit packet
Bit32u bx_radeon9800_c::pm4_splice_ib(Bit32u wr, Bit32u off, Bit32u n, Bit32u rptr)
{
  if (!n)
    return wr;
  if (n > R9800_CP_FIFO_DWORDS / 2) {
    BX_ERROR(("IB too large to splice: off=%08x n=%u", off, n));
    return wr;
  }
  cp_fifo_reserve(wr, n);
  if (radeon9800_cp_fifo_space(wr, cp_fifo_rd) < n)
    return wr;
  for (Bit32u i = 0; i < n; i++) {
    Bit32u v;
    if (!gpu_read32(off + i * 4, &v)) {
      BX_ERROR(("IB splice dead bus: off=%08x n=%u at=%u", off, n, i));
      return wr - i;
    }
    cp_fifo[wr & R9800_CP_FIFO_MASK] = v;
    cp_fifo_rptr[wr & R9800_CP_FIFO_MASK] = rptr;
    cp_fifo_tag[wr & R9800_CP_FIFO_MASK] = R9800_CP_TAG_IB;
    wr++;
  }
  return wr;
}

// Read pointer writeback to the host mailbox
void bx_radeon9800_c::pm4_rptr_writeback(void)
{
  Bit32u retire = cp_retire_rptr;
  if (cp_rb_rptr_addr && !(cp_rb_cntl & R9800_RB_NO_UPDATE) && (retire != cp_shadow_last)) {
    if (gpu_write32(cp_rb_rptr_addr & ~3u, retire))
      cp_shadow_last = retire;
  }
}

// Pump: ring -> local FIFO copy (CPU thread). Returns 1 when the ring is
// fully fetched, 0 when the FIFO is full, -1 on a dead bus read.
int bx_radeon9800_c::pm4_pump(void)
{
  Bit32u mask, wptr, wr;
  bool copied = false;
  int result = 1;

  if (!pm4_ring_bm())
    return 1;
  mask = pm4_ring_mask();
  wptr = cp_rb_wptr & mask;
  wr = cp_fifo_wr;

  while (cp_rb_rptr != wptr) {
    Bit32u v;
    if (radeon9800_cp_fifo_space(wr, cp_fifo_rd) < 4) {
      cp_fifo_reserve(wr, 4);
      if (radeon9800_cp_fifo_space(wr, cp_fifo_rd) < 4) {
        result = 0;
        break;
      }
    }
    if (!gpu_read32(cp_rb_base + cp_rb_rptr * 4, &v)) {
      result = -1;
      break;
    }
    cp_rb_rptr = (cp_rb_rptr + 1) & mask;
    cp_fifo[wr & R9800_CP_FIFO_MASK] = v;
    cp_fifo_rptr[wr & R9800_CP_FIFO_MASK] = cp_rb_rptr;
    cp_fifo_tag[wr & R9800_CP_FIFO_MASK] = 0;
    wr++;
    copied = true;

    if (pump_frame_rem) {
      pump_frame_rem--;
      if (pump_ib_state == 1) {
        pump_ib_addr = v;
        pump_ib_state = 2;
      } else if (pump_ib_state == 2) {
        pump_ib_state = 0;
        wr = pm4_splice_ib(wr, pump_ib_addr, v, cp_rb_rptr);
      }
    } else {
      switch (R9800_PM4_TYPE(v)) {
        case 0:
          pump_frame_rem = R9800_PM4_COUNT(v);
          // PACKET0(CP_IB_BASE, 2) { base, size }: indirect buffer submit
          if ((v & 0xffff9fff) == ((R9800_CP_IB_BASE >> 2) | 0x00010000))
            pump_ib_state = 1;
          break;
        case 1:
          pump_frame_rem = 2;
          break;
        case 3:
          pump_frame_rem = R9800_PM4_COUNT(v);
          break;
        default:
          break;
      }
    }
    if (!pump_ib_state) {
      BX_LOCK(cp_mutex);
      cp_fifo_wr = wr;
      BX_UNLOCK(cp_mutex);
    }
  }
  BX_LOCK(cp_mutex);
  cp_fifo_wr = wr;
  BX_UNLOCK(cp_mutex);

  if (copied)
    bx_set_sem(&cp_wake_sem);

  {
    Bit32u retire = cp_retire_rptr;
    pm4_rptr_writeback();
    if ((result == 0) || (retire != cp_rb_rptr))
      bx_virt_timer.activate_timer(pump_timer_id, 100, 0);
  }
  return result;
}

void bx_radeon9800_c::pm4_kick(void)
{
  pm4_pump();
}

// SOFT_RESET recovery pulse: abandon pending CP work
void bx_radeon9800_c::pm4_gui_reset(void)
{
  if (on_cp_thread())
    return;
  if (cp_thread_started && cp_executing) {
    cp_abort = 1;
    bx_set_sem(&cp_wake_sem);
    while (cp_executing)
      bx_wait_sem(&cp_idle_sem);
    cp_abort = 0;
  }
  BX_LOCK(cp_mutex);
  cp_fifo_rd = cp_fifo_wr;
  BX_UNLOCK(cp_mutex);
  pm4_ind_busy = false;
  pm4_ind_pending = false;
  pump_frame_rem = 0;
  pump_ib_state = 0;
  cp_retire_rptr = cp_rb_rptr;
}

// Does a register belong to the CP fetch-control block? A stream that
// writes them is a parse desync; refusing the poke keeps the fetch engine
// from repointing itself.
bool bx_radeon9800_c::pm4_reg_in_fetch_block(Bit32u reg)
{
  return ((reg >= R9800_CP_RB_BASE) && (reg <= R9800_CP_RB_RPTR_WR)) ||
         (reg == R9800_CP_CSQ_CNTL) || (reg == R9800_CP_ME_RAM_ADDR) ||
         (reg == R9800_CP_ME_RAM_DATAH) || (reg == R9800_CP_ME_RAM_DATAL);
}

// Wait until the engine is idle: ring fetched, FIFO drained, executor
// between packets (CPU thread only).
void bx_radeon9800_c::pm4_drain_wait(void)
{
  if (!cp_thread_started || on_cp_thread())
    return;
  if (!pm4_active()) {
    raster_flush();
    return;
  }
  cp_drain_req = 1;
  bx_set_sem(&cp_flip_sem);
  for (long iter = 0;; iter++) {
    int pumped = pm4_pump();
    if (!pm4_active())
      break;
    bool wedged = false;
    if ((pumped == -1) && (cp_fifo_rd == cp_fifo_wr))
      wedged = true;
    if ((iter >= 8) && cp_executing && (cp_fifo_rd == cp_fifo_wr) &&
        (cp_rb_rptr == (cp_rb_wptr & pm4_ring_mask())))
      wedged = true;
    if (wedged) {
      if (cp_executing) {
        cp_abort = 1;
        bx_set_sem(&cp_wake_sem);
        while (cp_executing)
          bx_wait_sem(&cp_idle_sem);
        cp_abort = 0;
      }
      break;
    }
    bx_set_sem(&cp_wake_sem);
    bx_wait_sem(&cp_idle_sem);
  }
  raster_flush();
  pm4_rptr_writeback();
  cp_drain_req = 0;
}

void bx_radeon9800_c::pm4_flip_notify(void)
{
  if (cp_thread_started)
    bx_set_sem(&cp_flip_sem);
}

// Queue a direct CPU register write behind the engine's pending work as
// a synthetic one-register PACKET0.
bool bx_radeon9800_c::pm4_enqueue_write(Bit32u off, Bit32u val)
{
  Bit32u rd, wr;
  if (!cp_thread_started)
    return false;
  if ((cp_rb_rptr != (cp_rb_wptr & pm4_ring_mask())) && (pm4_pump() != 1))
    return false;
  rd = cp_fifo_rd;
  wr = cp_fifo_wr;
  if (R9800_CP_FIFO_DWORDS - (wr - rd) < 2)
    return false;
  cp_fifo[wr & R9800_CP_FIFO_MASK] = (off >> 2) & 0x1fff;
  cp_fifo[(wr + 1) & R9800_CP_FIFO_MASK] = val;
  cp_fifo_rptr[wr & R9800_CP_FIFO_MASK] = 0xffffffff;
  cp_fifo_rptr[(wr + 1) & R9800_CP_FIFO_MASK] = 0xffffffff;
  cp_fifo_tag[wr & R9800_CP_FIFO_MASK] = 0;
  cp_fifo_tag[(wr + 1) & R9800_CP_FIFO_MASK] = 0;
  BX_LOCK(cp_mutex);
  cp_fifo_wr = wr + 2;
  BX_UNLOCK(cp_mutex);
  bx_set_sem(&cp_wake_sem);
  return true;
}

// PIO command stream: one raw dword into the queue
bool bx_radeon9800_c::pm4_enqueue_pio(Bit32u val)
{
  Bit32u wr;
  if (!cp_thread_started || on_cp_thread())
    return false;
  if ((cp_rb_rptr != (cp_rb_wptr & pm4_ring_mask())) && (pm4_pump() != 1))
    return false;
  wr = cp_fifo_wr;
  cp_fifo_reserve(wr, 1);
  if (radeon9800_cp_fifo_space(wr, cp_fifo_rd) < 1)
    return false;
  cp_fifo[wr & R9800_CP_FIFO_MASK] = val;
  cp_fifo_rptr[wr & R9800_CP_FIFO_MASK] = 0xffffffff;
  cp_fifo_tag[wr & R9800_CP_FIFO_MASK] = 0;
  BX_LOCK(cp_mutex);
  cp_fifo_wr = wr + 1;
  BX_UNLOCK(cp_mutex);
  bx_set_sem(&cp_wake_sem);
  return true;
}

// Queue an indirect buffer fired by a direct CPU write of CP_IB_BUFSZ
// behind the engine's pending work: a synthetic PACKET0(CP_IB_BASE, 2)
// submit followed by the buffer body spliced from guest memory now.
bool bx_radeon9800_c::pm4_enqueue_indirect(Bit32u off, Bit32u n)
{
  Bit32u wr;

  if (!cp_thread_started || on_cp_thread())
    return false;
  if (n == 0)
    return true;
  if (n > R9800_CP_FIFO_DWORDS / 2)
    return false;
  if ((cp_rb_rptr != (cp_rb_wptr & pm4_ring_mask())) && (pm4_pump() != 1))
    return false;
  wr = cp_fifo_wr;
  cp_fifo_reserve(wr, 3 + n);
  if (radeon9800_cp_fifo_space(wr, cp_fifo_rd) < 3 + n)
    return false;
  cp_fifo[wr & R9800_CP_FIFO_MASK] = (R9800_CP_IB_BASE >> 2) | 0x00010000;
  cp_fifo[(wr + 1) & R9800_CP_FIFO_MASK] = off;
  cp_fifo[(wr + 2) & R9800_CP_FIFO_MASK] = n;
  for (Bit32u i = 0; i < 3; i++) {
    cp_fifo_rptr[(wr + i) & R9800_CP_FIFO_MASK] = 0xffffffff;
    cp_fifo_tag[(wr + i) & R9800_CP_FIFO_MASK] = 0;
  }
  wr = pm4_splice_ib(wr + 3, off, n, 0xffffffff);
  BX_LOCK(cp_mutex);
  cp_fifo_wr = wr;
  BX_UNLOCK(cp_mutex);
  bx_set_sem(&cp_wake_sem);
  return true;
}

// ---------------------------------------------------------------------
// Executor (CP thread)
// ---------------------------------------------------------------------

// Blocking dword pop; returns false only on shutdown or an explicit abort
bool bx_radeon9800_c::cp_get(Bit32u *val)
{
  Bit32u rd = cp_fifo_rd;
  while (rd == cp_fifo_wr) {
    if (!cp_thread_run || cp_abort)
      return false;
    bx_set_sem(&cp_idle_sem);
    bx_wait_sem(&cp_wake_sem);
  }
  *val = cp_fifo[rd & R9800_CP_FIFO_MASK];
  Bit32u rr = cp_fifo_rptr[rd & R9800_CP_FIFO_MASK];
  if (rr != 0xffffffff)
    cp_retire_pending = rr;
  BX_LOCK(cp_mutex);
  cp_fifo_rd = rd + 1;
  BX_UNLOCK(cp_mutex);
  return true;
}

// Pop one spliced indirect buffer dword, or return false when the next
// FIFO dword is not body
bool bx_radeon9800_c::cp_get_ib(Bit32u *val)
{
  Bit32u rd = cp_fifo_rd;
  if ((rd == cp_fifo_wr) || (cp_fifo_tag[rd & R9800_CP_FIFO_MASK] != R9800_CP_TAG_IB))
    return false;
  *val = cp_fifo[rd & R9800_CP_FIFO_MASK];
  BX_LOCK(cp_mutex);
  cp_fifo_rd = rd + 1;
  BX_UNLOCK(cp_mutex);
  return true;
}

// Type-3 dispatch shared by the ring executor and the indirect walker
void bx_radeon9800_c::pm4_exec_packet3(Bit32u hdr, Bit32u *pl, Bit32u n)
{
  Bit32u op = R9800_PM4_T3_OPCODE(hdr);
  switch (op) {
    case R9800_PM4_OP_NOP:
      return;
    case R9800_PM4_OP_WAIT_FOR_IDLE:
      raster_flush();
      return;
    case R9800_PM4_OP_LOAD_MICROCODE:
      // microcode words ride in the payload: {addr, [datah, datal]...}
      if (n >= 1) {
        Bit32u a = pl[0] & 0xff;
        for (Bit32u i = 1; i + 1 < n; i += 2) {
          cp_me_ram[a][0] = pl[i];
          cp_me_ram[a][1] = pl[i + 1];
          a = (a + 1) & 0xff;
        }
      }
      return;
    default:
      break;
  }
  if (!r3d_packet3(hdr, pl, n)) {
    if ((op != R9800_PM4_OP_SET_SCISSORS) && (op != R9800_PM4_OP_NEXT_CHAR))
      raster_flush();
    r2d_packet3(hdr, pl, n);
  }
}

void bx_radeon9800_c::cp_packet(Bit32u hdr)
{
  Bit32u count;
  Bit32u *pl = cp_pl;

  switch (R9800_PM4_TYPE(hdr)) {
    case 0: {
      Bit32u reg = R9800_PM4_T0_REG(hdr);
      count = R9800_PM4_COUNT(hdr);
      for (Bit32u i = 0; i < count; i++) {
        Bit32u v;
        if (!cp_get(&v))
          return;
        Bit32u treg = (hdr & R9800_PM4_T0_ONE_REG_WR) ? reg : reg + i * 4;
        if (!pm4_reg_in_fetch_block(treg))
          reg_poke(treg, v);
      }
      break;
    }
    case 1:
      for (Bit32u i = 0; i < 2; i++) {
        Bit32u v;
        Bit32u treg = i ? R9800_PM4_T1_REG1(hdr) : R9800_PM4_T1_REG0(hdr);
        if (!cp_get(&v))
          return;
        if (!pm4_reg_in_fetch_block(treg))
          reg_poke(treg, v);
      }
      break;
    case 2:
      break;
    case 3: {
      Bit32u n;
      count = R9800_PM4_COUNT(hdr);
      n = count > R9800_PM4_MAX_PAYLOAD ? R9800_PM4_MAX_PAYLOAD : count;
      for (Bit32u i = 0; i < count; i++) {
        Bit32u v;
        if (!cp_get(&v))
          return;
        if (i < n)
          pl[i] = v;
      }
      pm4_exec_packet3(hdr, pl, n);
      break;
    }
    default:
      break;
  }
}

void bx_radeon9800_c::cp_thread_main(void)
{
  bool did_work = false;

  while (cp_thread_run) {
    Bit32u hdr;
    cp_executing = 1;
    if (cp_fifo_rd == cp_fifo_wr) {
      raster_flush();
      if (did_work) {
        did_work = false;
        gui_idle_event = 1;
      }
      cp_executing = 0;
      bx_set_sem(&cp_idle_sem);
      bx_wait_sem(&cp_wake_sem);
      continue;
    }
    if (cp_get(&hdr)) {
      cp_packet(hdr);
      did_work = true;
    }
    cp_retire_rptr = cp_retire_pending;
    cp_executing = 0;
  }
  cp_executing = 0;
  bx_set_sem(&cp_idle_sem);
}

// Indirect buffer execution: PACKET0(CP_IB_BASE, 2) { mc_addr, dwords }
void bx_radeon9800_c::pm4_run_indirect(void)
{
  if (pm4_ind_busy) {
    pm4_ind_pending = true;
    return;
  }
  pm4_ind_busy = true;
  in_indirect = true;

  do {
    Bit32u off = cp_ib_base;
    Bit32u n = cp_ib_bufsz;
    Bit32u pos = 0;
    bool spliced = false;

    {
      Bit32u rd = cp_fifo_rd;
      spliced = on_cp_thread() && (rd != cp_fifo_wr) && (cp_fifo_tag[rd & R9800_CP_FIFO_MASK] == R9800_CP_TAG_IB);
    }
    pm4_ind_pending = false;

    while (pos < n) {
      Bit32u hdr, count;
      Bit32u *pl = ind_pl;

      if (spliced) {
        if (!cp_get_ib(&hdr))
          break;
      } else if (!gpu_read32(off + pos * 4, &hdr)) {
        BX_DEBUG(("IB dead bus: addr=%08x pos=%u size=%u", off, pos, n));
        break;
      }
      pos++;

      switch (R9800_PM4_TYPE(hdr)) {
        case 0: {
          Bit32u reg = R9800_PM4_T0_REG(hdr);
          count = R9800_PM4_COUNT(hdr);
          for (Bit32u i = 0; (i < count) && (pos < n); i++, pos++) {
            Bit32u v;
            if (spliced) {
              if (!cp_get_ib(&v)) break;
            } else if (!gpu_read32(off + pos * 4, &v)) {
              break;
            }
            Bit32u treg = (hdr & R9800_PM4_T0_ONE_REG_WR) ? reg : reg + i * 4;
            if (!pm4_reg_in_fetch_block(treg))
              reg_poke(treg, v);
          }
          break;
        }
        case 1:
          for (Bit32u i = 0; (i < 2) && (pos < n); i++, pos++) {
            Bit32u v;
            Bit32u treg = i ? R9800_PM4_T1_REG1(hdr) : R9800_PM4_T1_REG0(hdr);
            if (spliced) {
              if (!cp_get_ib(&v)) break;
            } else if (!gpu_read32(off + pos * 4, &v)) {
              break;
            }
            if (!pm4_reg_in_fetch_block(treg))
              reg_poke(treg, v);
          }
          break;
        case 2:
          break;
        case 3: {
          Bit32u m, filled = 0;
          count = R9800_PM4_COUNT(hdr);
          m = count > R9800_PM4_MAX_PAYLOAD ? R9800_PM4_MAX_PAYLOAD : count;
          for (Bit32u i = 0; (i < count) && (pos < n); i++, pos++) {
            Bit32u v = 0;
            if (spliced) {
              if (!cp_get_ib(&v)) break;
            } else {
              gpu_read32(off + pos * 4, &v);
            }
            if (i < m)
              pl[i] = v;
            filled = i + 1;
          }
          if (m > filled)
            m = filled;
          pm4_exec_packet3(hdr, pl, m);
          break;
        }
        default:
          break;
      }
    }
    // discard the rest of a tagged body this walk did not consume
    if (on_cp_thread()) {
      Bit32u junk;
      while (cp_get_ib(&junk))
        pos++;
    }
  } while (pm4_ind_pending);

  in_indirect = false;
  pm4_ind_busy = false;
}

// WAIT_UNTIL CRTC_PFLIP: stall the engine until a pending flip is
// consumed by scanout (CP thread)
void bx_radeon9800_c::pm4_wait_until(Bit32u val)
{
  if (on_cp_thread() && (val & R9800_WAIT_CRTC_PFLIP)) {
    raster_flush();
    while (cp_thread_run && !cp_drain_req && crtc[0].offset_pending && !crtc[0].offset_lock)
      bx_wait_sem(&cp_flip_sem);
  }
  if (val & (R9800_WAIT_2D_IDLECLEAN | R9800_WAIT_3D_IDLECLEAN))
    raster_flush();
}

// GUI_SCRATCH_REGn: the CP mirrors scratch registers whose SCRATCH_UMSK
// bit is set to memory at SCRATCH_ADDR + n*4 (driver fences)
void bx_radeon9800_c::pm4_scratch_write(int n, Bit32u val)
{
  gui_scratch[n] = val;
  if ((scratch_umsk & (1u << n)) && scratch_addr)
    gpu_write32((scratch_addr & ~3u) + (Bit32u)n * 4u, val);
}

// ---------------------------------------------------------------------
// Register file interface
// ---------------------------------------------------------------------

bool bx_radeon9800_c::pm4_reg_read(Bit32u off, Bit32u *val)
{
  if ((off >= R9800_GUI_SCRATCH_REG0) && (off <= R9800_GUI_SCRATCH_REG5)) {
    *val = gui_scratch[(off - R9800_GUI_SCRATCH_REG0) >> 2];
    return true;
  }
  if ((off >= R9800_CP_CSQ_APER_PRIMARY) && (off <= R9800_CP_CSQ_APER_INDIRECT_END)) {
    *val = 0;
    return true;
  }
  switch (off) {
    case R9800_CP_RB_BASE:      *val = cp_rb_base; return true;
    case R9800_CP_RB_CNTL:      *val = cp_rb_cntl; return true;
    case R9800_CP_RB_RPTR_ADDR: *val = cp_rb_rptr_addr; return true;
    case R9800_CP_RB_RPTR:      *val = cp_retire_rptr; return true;
    case R9800_CP_RB_WPTR:      *val = cp_rb_wptr; return true;
    case R9800_CP_RB_WPTR_DELAY: *val = cp_rb_wptr_delay; return true;
    case R9800_CP_RB_RPTR_WR:   *val = cp_rb_rptr; return true;
    case R9800_CP_IB_BASE:      *val = cp_ib_base; return true;
    case R9800_CP_IB_BUFSZ:     *val = cp_ib_bufsz; return true;
    case R9800_CP_CSQ_CNTL:
      *val = (cp_csq_cntl & 0xf0000000);
      return true;
    case R9800_CP_CSQ_MODE:     *val = cp_csq_mode; return true;
    case R9800_SCRATCH_UMSK:    *val = scratch_umsk; return true;
    case R9800_SCRATCH_ADDR:    *val = scratch_addr; return true;
    case R9800_CP_RESYNC_ADDR:  *val = cp_resync_addr; return true;
    case R9800_CP_RESYNC_DATA:  *val = cp_resync_data; return true;
    case R9800_CP_ME_CNTL:      *val = cp_me_cntl; return true;
    case R9800_CP_ME_RAM_ADDR:  *val = cp_me_ram_addr; return true;
    case R9800_CP_ME_RAM_RADDR: *val = cp_me_ram_raddr; return true;
    case R9800_CP_ME_RAM_DATAH: *val = cp_me_ram[cp_me_ram_raddr & 0xff][0]; return true;
    case R9800_CP_ME_RAM_DATAL:
      *val = cp_me_ram[cp_me_ram_raddr & 0xff][1];
      cp_me_ram_raddr = (cp_me_ram_raddr + 1) & 0xff;
      return true;
    case R9800_CP_STAT:
      *val = pm4_active() ? (R9800_CP_STAT_CP_BUSY | R9800_CP_STAT_CMDSTRM_BUSY | R9800_CP_STAT_CSI_BUSY |
                             R9800_CP_STAT_CSQ_PRIMARY_BUSY) : 0;
      return true;
    case R9800_CP_CSQ_ADDR:     *val = cp_csq_addr; return true;
    case R9800_CP_CSQ_DATA:     *val = 0; return true;
    case R9800_CP_CSQ_STAT:
    case R9800_CP_CSQ2_STAT:    *val = 0; return true;
    case R9800_ISYNC_CNTL:      *val = isync_cntl; return true;
    case R9800_RBBM_GUICNTL:    *val = rbbm_guicntl; return true;
    case R9800_WAIT_UNTIL:      *val = 0; return true;
    default:
      break;
  }
  return false;
}

bool bx_radeon9800_c::pm4_reg_write(Bit32u off, Bit32u val, Bit32u mask)
{
  // Direct CPU writes that reconfigure the ring fetch engine quiesce the
  // executor first (WPTR is the submit doorbell and pumps instead).
  if (!on_cp_thread() && (off != R9800_CP_RB_WPTR) && pm4_reg_in_fetch_block(off))
    pm4_drain_wait();

  if ((off >= R9800_GUI_SCRATCH_REG0) && (off <= R9800_GUI_SCRATCH_REG5)) {
    int n = (off - R9800_GUI_SCRATCH_REG0) >> 2;
    pm4_scratch_write(n, (gui_scratch[n] & ~mask) | (val & mask));
    return true;
  }
  if ((off >= R9800_CP_CSQ_APER_PRIMARY) && (off <= R9800_CP_CSQ_APER_INDIRECT_END)) {
    // PIO stream dwords that could not be queued (executor down) run inline
    if (on_cp_thread())
      return true;
    Bit32u rd = cp_fifo_rd, wr = cp_fifo_wr;
    if (radeon9800_cp_fifo_space(wr, rd) >= 1) {
      pm4_enqueue_pio(val);
    }
    return true;
  }

#define MERGE(field) ((field) = ((field) & ~mask) | (val & mask))
  switch (off) {
    case R9800_CP_RB_BASE:
      MERGE(cp_rb_base);
      cp_rb_base &= 0xfffffffc;
      return true;
    case R9800_CP_RB_CNTL:
      MERGE(cp_rb_cntl);
      return true;
    case R9800_CP_RB_RPTR_ADDR:
      MERGE(cp_rb_rptr_addr);
      return true;
    case R9800_CP_RB_RPTR:
      // read-only status; a write realigns the executor's view
      MERGE(cp_rb_rptr);
      cp_rb_rptr &= pm4_ring_mask();
      cp_retire_rptr = cp_rb_rptr;
      pump_frame_rem = 0;
      pump_ib_state = 0;
      return true;
    case R9800_CP_RB_RPTR_WR:
      if (cp_rb_cntl & R9800_RB_RPTR_WR_ENA) {
        Bit32u v = val & mask & 0x7fffffff;
        cp_rb_rptr = v & pm4_ring_mask();
        cp_retire_rptr = cp_rb_rptr;
        cp_shadow_last = 0xffffffff;
        pump_frame_rem = 0;
        pump_ib_state = 0;
      }
      return true;
    case R9800_CP_RB_WPTR: {
      Bit32u wmask = pm4_ring_mask();
      Bit32u owptr = cp_rb_wptr & wmask;
      MERGE(cp_rb_wptr);
      cp_rb_wptr &= 0x7fffffff;
      if (pm4_ring_bm()) {
        Bit32u nwptr = cp_rb_wptr & wmask;
        // a backward move is a ring rebase, not a submit
        if (((owptr - nwptr) & wmask) < ((nwptr - owptr) & wmask)) {
          cp_rb_rptr = nwptr;
          pump_frame_rem = 0;
          pump_ib_state = 0;
        }
      }
      if (!on_cp_thread())
        pm4_pump();
      return true;
    }
    case R9800_CP_RB_WPTR_DELAY: MERGE(cp_rb_wptr_delay); return true;
    case R9800_CP_IB_BASE:  MERGE(cp_ib_base); return true;
    case R9800_CP_IB_BUFSZ:
      MERGE(cp_ib_bufsz);
      cp_ib_bufsz &= 0x001fffff;
      if (!on_cp_thread() && cp_thread_started) {
        // Direct CPU submit: keep it behind the queued engine work
        if (pm4_enqueue_indirect(cp_ib_base, cp_ib_bufsz))
          return true;
        pm4_drain_wait();
      }
      pm4_run_indirect();
      return true;
    case R9800_CP_CSQ_CNTL: {
      MERGE(cp_csq_cntl);
      if (!on_cp_thread() && pm4_ring_bm())
        pm4_pump();
      return true;
    }
    case R9800_CP_CSQ_MODE: MERGE(cp_csq_mode); return true;
    case R9800_SCRATCH_UMSK:
      MERGE(scratch_umsk);
      scratch_umsk &= 0x3f;
      return true;
    case R9800_SCRATCH_ADDR: MERGE(scratch_addr); return true;
    case R9800_CP_RESYNC_ADDR: MERGE(cp_resync_addr); return true;
    case R9800_CP_RESYNC_DATA:
      MERGE(cp_resync_data);
      // the resync token is mirrored to the address programmed before it
      if (cp_resync_addr)
        gpu_write32(cp_resync_addr & ~3u, cp_resync_data);
      return true;
    case R9800_CP_ME_CNTL: MERGE(cp_me_cntl); return true;
    case R9800_CP_ME_RAM_ADDR:
      MERGE(cp_me_ram_addr);
      cp_me_ram_addr &= 0xff;
      return true;
    case R9800_CP_ME_RAM_RADDR:
      MERGE(cp_me_ram_raddr);
      cp_me_ram_raddr &= 0xff;
      return true;
    case R9800_CP_ME_RAM_DATAH:
      cp_me_ram[cp_me_ram_addr & 0xff][0] = (cp_me_ram[cp_me_ram_addr & 0xff][0] & ~mask) | (val & mask);
      return true;
    case R9800_CP_ME_RAM_DATAL:
      cp_me_ram[cp_me_ram_addr & 0xff][1] = (cp_me_ram[cp_me_ram_addr & 0xff][1] & ~mask) | (val & mask);
      cp_me_ram_addr = (cp_me_ram_addr + 1) & 0xff;
      return true;
    case R9800_CP_STAT: return true;
    case R9800_CP_CSQ_ADDR: MERGE(cp_csq_addr); return true;
    case R9800_CP_CSQ_DATA: return true;
    case R9800_CP_CSQ_STAT:
    case R9800_CP_CSQ2_STAT:
      return true;
    case R9800_WAIT_UNTIL:
      pm4_wait_until(val & mask);
      return true;
    case R9800_ISYNC_CNTL: MERGE(isync_cntl); return true;
    case R9800_RBBM_GUICNTL: MERGE(rbbm_guicntl); return true;
    default:
      break;
  }
#undef MERGE
  return false;
}

#endif // BX_SUPPORT_PCI && BX_SUPPORT_RADEON9800
