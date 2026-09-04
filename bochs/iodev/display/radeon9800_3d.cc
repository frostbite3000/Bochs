/////////////////////////////////////////////////////////////////////////
// $Id$
/////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025-2026  The Bochs Project
//
//  ATI Radeon 9800 (R350) emulation: R300 3D engine.
//
//  Vertex assembler / processor (programmable stream control, vertex
//  arrays, the programmable vertex shader interpreter, clipping and the
//  viewport transform), geometry assembly and setup (culling, polygon
//  offset, points, lines), the rasterizer (attribute routing), the
//  programmable fragment shader interpreter with the texture unit, the
//  fog unit, Z/stencil with fast clear mask RAMs and the colour buffer
//  (blending, dithering, ROP, multisample resolve), plus the parallel
//  scanline-interleaved render worker pool.
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

#include <math.h>
#include "vgacore.h"
#include "pci.h"
#include "ddc.h"
#include "radeon9800_regs.h"
#include "radeon9800.h"

#define LOG_THIS BX_R9800_THIS

#define VAPR(off) (vap_regs[((off) - R9800_VAP_BASE) >> 2])

static BX_CPP_INLINE float r3d_clamp01(float v)
{
  return (v > 0.0f) ? (v > 1.0f ? 1.0f : v) : 0.0f;
}

static BX_CPP_INLINE float r3d_clampf(float v, float lo, float hi)
{
  return v < lo ? lo : (v > hi ? hi : v);
}

// US constants are stored as S16E7 (sign, 7-bit exponent biased 63, 16
// mantissa bits): the top 24 bits of an IEEE float with the exponent
// rebased.
static float r3d_float24(Bit32u v)
{
  v &= 0xffffff;
  if (v == 0)
    return 0.0f;
  Bit32u sign = (v >> 23) & 1;
  Bit32u exp = (v >> 16) & 0x7f;
  Bit32u mant = v & 0xffff;
  Bit32u ieee = (sign << 31) | ((exp + 64) << 23) | (mant << 7);
  return r9800_u2f(ieee);
}

// 0.9 fixed to float
static BX_CPP_INLINE float r3d_fix09(Bit32u v)
{
  return (float)(v & 0x3ff) / 511.0f;
}

// ---------------------------------------------------------------------
// Register file
// ---------------------------------------------------------------------

void bx_radeon9800_c::r3d_reset(void)
{
  memset(vap_regs, 0, sizeof(vap_regs));
  memset(r3d_regs, 0, sizeof(r3d_regs));
  memset(vport_alt, 0, sizeof(vport_alt));
  memset(vtx_state, 0, sizeof(vtx_state));
  VAPR(R9800_VAP_CNTL) = 0x00240000;
  VAPR(R9800_VAP_VF_MAX_VTX_INDX) = 0x00ffffff;
  VAPR(R9800_VAP_PVS_VTX_TIMEOUT_REG) = 0xffffffff;
  r3d_regs[R3D(R9800_GB_TILE_CONFIG)] = 0x00000011;
  r3d_regs[R3D(R9800_RB3D_COLOR_CHANNEL_MASK)] = 0x0f;
  r3d_regs[R3D(R9800_RB3D_COLORPITCH0)] = 6 << 21;
  r3d_regs[R3D(R9800_SU_DEPTH_SCALE)] = 0x3f800000;
  r3d_regs[R3D(R9800_SU_REG_DEST)] = 0xf;
  r3d_regs[R3D(R9800_RB3D_BLENDCNTL)] = 0x4;
  r3d_regs[R3D(R9800_SC_CLIP_RULE)] = 0xffff;
  pvs_vec_index = 0;
  pvs_vec_sub = 0;
  zmask_wrindex = zmask_rdindex = 0;
  hiz_wrindex = hiz_rdindex = 0;
  cmask_wrindex = cmask_rdindex = 0;
  zpass_count = 0;
  r3d_serial++;
  draw.active = false;
  draw.vdw_n = 0;
  draw.ring_n = 0;
  draw.poly_n = 0;
}

bool bx_radeon9800_c::r3d_reg_read(Bit32u off, Bit32u *val)
{
  if ((off >= R9800_VAP_BASE) && (off <= R9800_VAP_END)) {
    switch (off) {
      case R9800_VAP_PVS_VECTOR_DATA_REG:
      case R9800_VAP_PVS_VECTOR_DATA_REG_128:
        *val = r3d_pvs_vector_read();
        return true;
      case R9800_VAP_CNTL_STATUS:
        *val = VAPR(R9800_VAP_CNTL_STATUS) & 0x000001ff;   // never busy
        return true;
      default:
        break;
    }
    if ((off >= R9800_VAP_VTX_ST_BASE) && (off <= R9800_VAP_VTX_ST_END)) {
      Bit32u d = (off - R9800_VAP_VTX_ST_BASE) >> 2;
      *val = r9800_f2u(vtx_state[d >> 2][d & 3]);
      return true;
    }
    *val = VAPR(off);
    return true;
  }
  if ((off >= R9800_VAP_VPORT_XSCALE_ALT) && (off <= R9800_VAP_VPORT_ZOFFSET_ALT)) {
    *val = vport_alt[(off - R9800_VAP_VPORT_XSCALE_ALT) >> 2];
    return true;
  }
  if ((off >= R9800_R3D_IMAGE_BASE) && (off <= R9800_R3D_IMAGE_END)) {
    switch (off) {
      case R9800_RB3D_DSTCACHE_CTLSTAT:
      case R9800_ZB_ZCACHE_CTLSTAT:
        *val = 0;
        return true;
      case R9800_ZB_ZPASS_DATA:
        *val = zpass_count;
        return true;
      case R9800_ZB_ZMASK_DWORD:
        *val = zmask_ram[zmask_rdindex & (R9800_ZMASK_RAM_DWORDS - 1)];
        zmask_rdindex += 2;
        return true;
      case R9800_ZB_HIZ_DWORD:
        *val = hiz_ram[hiz_rdindex & (R9800_HIZ_RAM_DWORDS - 1)];
        hiz_rdindex += 2;
        return true;
      case R9800_RB3D_CMASK_DWORD:
        *val = cmask_ram[cmask_rdindex & (R9800_CMASK_RAM_DWORDS - 1)];
        cmask_rdindex += 1;
        return true;
      case R9800_ZB_ZMASK_RDINDEX: *val = zmask_rdindex; return true;
      case R9800_ZB_ZMASK_WRINDEX: *val = zmask_wrindex; return true;
      case R9800_ZB_HIZ_RDINDEX: *val = hiz_rdindex; return true;
      case R9800_ZB_HIZ_WRINDEX: *val = hiz_wrindex; return true;
      case R9800_RB3D_CMASK_RDINDEX: *val = cmask_rdindex; return true;
      case R9800_RB3D_CMASK_WRINDEX: *val = cmask_wrindex; return true;
      default:
        break;
    }
    *val = r3d_regs[R3D(off)];
    return true;
  }
  return false;
}

// PVS vector memory: VECTOR_INDX_REG selects the octword, DATA_REG writes
// consecutive dwords and steps to the next octword after every fourth
void bx_radeon9800_c::r3d_pvs_vector_write(Bit32u val)
{
  Bit32u idx = pvs_vec_index & (R9800_PVS_VECTOR_MEM - 1);
  pvs_vec[idx][pvs_vec_sub & 3] = val;
  pvs_vec_sub++;
  if (pvs_vec_sub >= 4) {
    pvs_vec_sub = 0;
    pvs_vec_index = (pvs_vec_index + 1) & (R9800_PVS_VECTOR_MEM - 1);
  }
  r3d_serial++;
}

Bit32u bx_radeon9800_c::r3d_pvs_vector_read(void)
{
  Bit32u idx = pvs_vec_index & (R9800_PVS_VECTOR_MEM - 1);
  Bit32u v = pvs_vec[idx][pvs_vec_sub & 3];
  pvs_vec_sub++;
  if (pvs_vec_sub >= 4) {
    pvs_vec_sub = 0;
    pvs_vec_index = (pvs_vec_index + 1) & (R9800_PVS_VECTOR_MEM - 1);
  }
  return v;
}

// State-based vertex data (VAP_VTX_ST_*): components accumulate in the
// vertex state memory; the write of the last position component emits a
// vertex, END_OF_PKT closes the primitive.
void bx_radeon9800_c::r3d_vtx_state_write(Bit32u off, Bit32u val)
{
  float f = r9800_u2f(val);
  Bit32u d;
  switch (off) {
    case R9800_VAP_VTX_ST_POS_0_X_2: vtx_state[0][0] = f; return;
    case R9800_VAP_VTX_ST_POS_0_Y_2:
      vtx_state[0][1] = f; vtx_state[0][2] = 0.0f; vtx_state[0][3] = 1.0f;
      break;
    case R9800_VAP_VTX_ST_POS_0_X_3: vtx_state[0][0] = f; return;
    case R9800_VAP_VTX_ST_POS_0_Y_3: vtx_state[0][1] = f; return;
    case R9800_VAP_VTX_ST_POS_0_Z_3:
      vtx_state[0][2] = f; vtx_state[0][3] = 1.0f;
      break;
    case R9800_VAP_VTX_ST_POS_0_W_4:
      vtx_state[0][3] = f;
      break;
    case R9800_VAP_VTX_ST_NORM_0_PKD:
      vtx_state[1][0] = (float)((Bit32s)((val & 0x7ff) << 21) >> 21) / 1023.0f;
      vtx_state[1][1] = (float)((Bit32s)(((val >> 11) & 0x7ff) << 21) >> 21) / 1023.0f;
      vtx_state[1][2] = (float)((Bit32s)(((val >> 22) & 0x3ff) << 22) >> 22) / 511.0f;
      return;
    case R9800_VAP_VTX_ST_USR_CLR_PKD:
      vtx_state[22][0] = ((val >> 16) & 0xff) / 255.0f;
      vtx_state[22][1] = ((val >> 8) & 0xff) / 255.0f;
      vtx_state[22][2] = (val & 0xff) / 255.0f;
      vtx_state[22][3] = (val >> 24) / 255.0f;
      return;
    case R9800_VAP_VTX_ST_END_OF_PKT:
      if (draw.active)
        vap_end();
      return;
    default:
      if ((off >= R9800_VAP_VTX_ST_CLR_0_PKD) && (off <= R9800_VAP_VTX_ST_CLR_7_PKD)) {
        Bit32u c = 2 + ((off - R9800_VAP_VTX_ST_CLR_0_PKD) >> 2);
        vtx_state[c][0] = ((val >> 16) & 0xff) / 255.0f;
        vtx_state[c][1] = ((val >> 8) & 0xff) / 255.0f;
        vtx_state[c][2] = (val & 0xff) / 255.0f;
        vtx_state[c][3] = (val >> 24) / 255.0f;
        return;
      }
      if ((off >= R9800_VAP_VTX_ST_BASE) && (off <= R9800_VAP_VTX_ST_END)) {
        d = (off - R9800_VAP_VTX_ST_BASE) >> 2;
        vtx_state[d >> 2][d & 3] = f;
      }
      return;
  }
  // a complete position: emit the vertex through the VAP
  if (!draw.active) {
    Bit32u vf = VAPR(R9800_VAP_VF_CNTL);
    if (R9800_VF_PRIM_TYPE(vf) == 0)
      return;
    vap_begin(vf);
  }
  {
    r300_vap_in_t in;
    r300_vtx_t v;
    r300_vs_out_t vso;
    // the state memory vectors feed the input memory: vector k -> input k
    Bit32u vsc = VAPR(R9800_VAP_VTX_STATE_CNTL);
    for (int k = 0; k < 16; k++)
      for (int c = 0; c < 4; c++)
        in.v[k][c] = vtx_state[k][c];
    if (vsc & (1 << 18)) {
      // address indirection: PROG_STREAM_CNTL DST_VEC_LOC routes vector k
      for (int k = 0; k < 16; k++) {
        Bit32u psc = VAPR(R9800_VAP_PROG_STREAM_CNTL_0 + (k >> 1) * 4);
        Bit32u fld = (k & 1) ? (psc >> 16) : psc;
        Bit32u loc = (fld >> 8) & 0x1f;
        if (loc < 16)
          for (int c = 0; c < 4; c++) in.v[loc][c] = vtx_state[k][c];
      }
    }
    vap_process_input(&in, &v, &vso);
  }
}

void bx_radeon9800_c::r3d_port_data_write(Bit32u val)
{
  if (!draw.active) {
    Bit32u vf = VAPR(R9800_VAP_VF_CNTL);
    if (R9800_VF_PRIM_TYPE(vf) == 0)
      return;
    vap_begin(vf);
  }
  vap_vertex_dwords(&val, 1);
}

void bx_radeon9800_c::r3d_port_idx_write(Bit32u val)
{
  if (!draw.active) {
    Bit32u vf = VAPR(R9800_VAP_VF_CNTL);
    if (R9800_VF_PRIM_TYPE(vf) == 0)
      return;
    vap_begin(vf);
  }
  if (draw.idx32) {
    vap_index(val);
  } else {
    vap_index(val & 0xffff);
    if (draw.emitted < draw.num)
      vap_index(val >> 16);
  }
}

bool bx_radeon9800_c::r3d_reg_write(Bit32u off, Bit32u val, Bit32u mask)
{
  if ((off >= R9800_VAP_BASE) && (off <= R9800_VAP_END)) {
    if ((off >= R9800_VAP_PORT_DATA0) && (off <= R9800_VAP_PORT_DATA15)) {
      r3d_port_data_write(val);
      return true;
    }
    if ((off >= R9800_VAP_PORT_IDX0) && (off <= R9800_VAP_PORT_IDX15)) {
      r3d_port_idx_write(val);
      return true;
    }
    if ((off >= R9800_VAP_VTX_ST_BASE) && (off <= R9800_VAP_VTX_ST_END_OF_PKT)) {
      r3d_vtx_state_write(off, val);
      return true;
    }
    Bit32u *r = &VAPR(off);
    switch (off) {
      case R9800_VAP_PVS_VECTOR_INDX_REG:
        *r = (*r & ~mask) | (val & mask);
        pvs_vec_index = *r & (R9800_PVS_VECTOR_MEM - 1);
        pvs_vec_sub = 0;
        return true;
      case R9800_VAP_PVS_VECTOR_DATA_REG:
      case R9800_VAP_PVS_VECTOR_DATA_REG_128:
        r3d_pvs_vector_write(val);
        return true;
      case R9800_VAP_PORT_DATA_IDX_128:
        r3d_port_idx_write(val);
        return true;
      case R9800_VAP_VF_CNTL:
        *r = (*r & ~mask) | (val & mask);
        // a new primitive type while a state based draw is open closes it
        if (draw.active && (draw.walk == R9800_VF_WALK_STATE))
          vap_end();
        return true;
      case R9800_VAP_PVS_STATE_FLUSH_REG:
        return true;
      default:
        *r = (*r & ~mask) | (val & mask);
        r3d_serial++;
        return true;
    }
  }
  if ((off >= R9800_VAP_VPORT_XSCALE_ALT) && (off <= R9800_VAP_VPORT_ZOFFSET_ALT)) {
    Bit32u *r = &vport_alt[(off - R9800_VAP_VPORT_XSCALE_ALT) >> 2];
    *r = (*r & ~mask) | (val & mask);
    VAPR(R9800_VAP_VPORT_XSCALE + (off - R9800_VAP_VPORT_XSCALE_ALT)) = *r;
    r3d_serial++;
    return true;
  }
  if ((off >= R9800_R3D_IMAGE_BASE) && (off <= R9800_R3D_IMAGE_END)) {
    Bit32u *r = &r3d_regs[R3D(off)];
    Bit32u merged = (*r & ~mask) | (val & mask);
    switch (off) {
      case R9800_RB3D_DSTCACHE_CTLSTAT:
      case R9800_ZB_ZCACHE_CTLSTAT:
      case R9800_TX_INVALTAGS:
      case R9800_US_RESET:
      case R9800_GA_SOFT_RESET:
        if (on_cp_thread()) raster_flush();
        return true;
      case R9800_ZB_ZPASS_DATA:
        zpass_count = merged;
        return true;
      case R9800_ZB_ZPASS_ADDR:
        *r = merged;
        r3d_zpass_writeback();
        return true;
      case R9800_ZB_ZMASK_WRINDEX:
        zmask_wrindex = merged;
        return true;
      case R9800_ZB_ZMASK_RDINDEX:
        zmask_rdindex = merged;
        return true;
      case R9800_ZB_ZMASK_DWORD:
        if (on_cp_thread()) raster_flush();
        zmask_ram[zmask_wrindex & (R9800_ZMASK_RAM_DWORDS - 1)] = merged;
        zmask_ram[(zmask_wrindex + 1) & (R9800_ZMASK_RAM_DWORDS - 1)] = merged;
        zmask_wrindex += 2;
        return true;
      case R9800_ZB_HIZ_WRINDEX:
        hiz_wrindex = merged;
        return true;
      case R9800_ZB_HIZ_RDINDEX:
        hiz_rdindex = merged;
        return true;
      case R9800_ZB_HIZ_DWORD:
        hiz_ram[hiz_wrindex & (R9800_HIZ_RAM_DWORDS - 1)] = merged;
        hiz_ram[(hiz_wrindex + 1) & (R9800_HIZ_RAM_DWORDS - 1)] = merged;
        hiz_wrindex += 2;
        return true;
      case R9800_RB3D_CMASK_WRINDEX:
        cmask_wrindex = merged;
        return true;
      case R9800_RB3D_CMASK_RDINDEX:
        cmask_rdindex = merged;
        return true;
      case R9800_RB3D_CMASK_DWORD:
        cmask_ram[cmask_wrindex & (R9800_CMASK_RAM_DWORDS - 1)] = merged;
        cmask_wrindex += 1;
        return true;
      default:
        break;
    }
    *r = merged;
    r3d_serial++;
    return true;
  }
  return false;
}

// ZB_ZPASS_ADDR: the pass count is written to the given memory address
void bx_radeon9800_c::r3d_zpass_writeback(void)
{
  Bit32u addr = r3d_regs[R3D(R9800_ZB_ZPASS_ADDR)] & ~3u;
  if (on_cp_thread())
    raster_flush();
  gpu_write32(addr, zpass_count);
}

// 3D_CLEAR_ZMASK / HIZ / CMASK: { start, count, value } fills a mask RAM
void bx_radeon9800_c::r3d_clear_mask_ram(Bit32u *ram, Bit32u ram_size, const Bit32u *pl, Bit32u count)
{
  if (count < 2)
    return;
  Bit32u start = pl[0], n = pl[1];
  Bit32u value = (count >= 3) ? pl[2] : 0;
  if (on_cp_thread())
    raster_flush();
  if (start >= ram_size)
    return;
  if (n > ram_size - start)
    n = ram_size - start;
  for (Bit32u i = 0; i < n; i++)
    ram[start + i] = value;
  r3d_serial++;
}

// 3D_LOAD_VBPNTR: { narrays, [attr01, addr0, addr1] ... }
void bx_radeon9800_c::r3d_load_vbpntr(const Bit32u *pl, Bit32u count)
{
  if (count < 1)
    return;
  Bit32u narrays = pl[0] & 0x1f;
  Bit32u p = 1;
  VAPR(R9800_VAP_VTX_NUM_ARRAYS) = (VAPR(R9800_VAP_VTX_NUM_ARRAYS) & ~0x1fu) | narrays;
  for (Bit32u k = 0; k < narrays; k += 2) {
    if (p >= count) break;
    VAPR(R9800_VAP_VTX_AOS_ATTR01 + (k >> 1) * 12) = pl[p++];
    if (p >= count) break;
    VAPR(R9800_VAP_VTX_AOS_ADDR0 + (k >> 1) * 12) = pl[p++];
    if (k + 1 < narrays) {
      if (p >= count) break;
      VAPR(R9800_VAP_VTX_AOS_ADDR0 + (k >> 1) * 12 + 4) = pl[p++];
    }
  }
  r3d_serial++;
}

// INDX_BUFFER: { ONE_REG_WR | reg, mc address, dword count }: the CP reads
// the dwords and writes them to VAP_PORT_IDX
void bx_radeon9800_c::r3d_indx_buffer(const Bit32u *pl, Bit32u count)
{
  if (count < 3)
    return;
  Bit32u addr = pl[1];
  Bit32u n = pl[2];
  Bit32u reg = (pl[0] & 0x1fff) << 2;
  if (n > 0x100000)
    n = 0x100000;
  Bit32u *buf = new Bit32u[n ? n : 1];
  if (!gpu_read(addr, (Bit8u*)buf, n * 4)) {
    delete [] buf;
    return;
  }
  for (Bit32u i = 0; i < n; i++) {
    Bit32u v = ReadHostDWordFromLittleEndian(&buf[i]);
    if ((reg >= R9800_VAP_PORT_IDX0) && (reg <= R9800_VAP_PORT_IDX15))
      r3d_port_idx_write(v);
    else
      reg_poke(reg, v);
  }
  delete [] buf;
}

bool bx_radeon9800_c::r3d_packet3(Bit32u hdr, const Bit32u *pl, Bit32u count)
{
  Bit32u op = R9800_PM4_T3_OPCODE(hdr);
  switch (op) {
    case R9800_PM4_OP_3D_DRAW_VBUF:
      // { unused, VF_CNTL }
      if (count >= 2)
        r3d_draw_list(pl[1]);
      return true;
    case R9800_PM4_OP_3D_DRAW_VBUF_2:
      if (count >= 1)
        r3d_draw_list(pl[0]);
      return true;
    case R9800_PM4_OP_3D_DRAW_IMMD:
      // { VTX_FMT (legacy), VF_CNTL, data }
      if (count >= 2)
        r3d_draw_embedded(pl[1], &pl[2], count - 2);
      return true;
    case R9800_PM4_OP_3D_DRAW_IMMD_2:
    case R9800_PM4_OP_3D_DRAW_128:
      if (count >= 1)
        r3d_draw_embedded(pl[0], &pl[1], count - 1);
      return true;
    case R9800_PM4_OP_3D_DRAW_INDX:
      if (count >= 2)
        r3d_draw_indices(pl[1], &pl[2], count - 2);
      return true;
    case R9800_PM4_OP_3D_DRAW_INDX_2:
      if (count >= 1)
        r3d_draw_indices(pl[0], &pl[1], count - 1);
      return true;
    case R9800_PM4_OP_3D_LOAD_VBPNTR:
      r3d_load_vbpntr(pl, count);
      return true;
    case R9800_PM4_OP_INDX_BUFFER:
      r3d_indx_buffer(pl, count);
      return true;
    case R9800_PM4_OP_3D_CLEAR_ZMASK:
      r3d_clear_mask_ram(zmask_ram, R9800_ZMASK_RAM_DWORDS, pl, count);
      return true;
    case R9800_PM4_OP_3D_CLEAR_HIZ:
      r3d_clear_mask_ram(hiz_ram, R9800_HIZ_RAM_DWORDS, pl, count);
      return true;
    case R9800_PM4_OP_3D_CLEAR_CMASK:
      r3d_clear_mask_ram(cmask_ram, R9800_CMASK_RAM_DWORDS, pl, count);
      return true;
    case R9800_PM4_OP_3D_RNDR_GEN_INDX_PRIM:
    case R9800_PM4_OP_3D_RNDR_GEN_PRIM:
      // legacy Rage 128 draw packets are not decoded by the R300 microcode
      return true;
    default:
      return false;
  }
}

// =====================================================================
// Vertex assembler / processor
// =====================================================================

// Number of dwords one vertex occupies in the embedded / concatenated
// array stream according to the programmable stream control
static Bit32u r3d_psc_dwords(const Bit32u *vap_regs)
{
  Bit32u total = 0;
  for (int k = 0; k < 16; k++) {
    Bit32u psc = vap_regs[(R9800_VAP_PROG_STREAM_CNTL_0 - R9800_VAP_BASE) / 4 + (k >> 1)];
    Bit32u fld = (k & 1) ? (psc >> 16) : (psc & 0xffff);
    Bit32u dt = fld & 0xf;
    Bit32u skip = (fld >> 4) & 0xf;
    Bit32u n;
    switch (dt) {
      case 0: n = 1; break;
      case 1: n = 2; break;
      case 2: n = 3; break;
      case 3: n = 4; break;
      case 7: n = 2; break;
      default: n = 1; break;
    }
    total += n + skip;
    if (fld & (1 << 13))
      break;
  }
  return total;
}

// Fixed point to float with the SIGNED / NORMALIZE flags and the signed
// normalize method of VAP_PSC_SGN_NORM_CNTL
static BX_CPP_INLINE float r3d_psc_fixed(Bit32u raw, int bits, bool sgn, bool norm, int method)
{
  Bit32u maxu = (bits >= 32) ? 0xffffffffu : ((1u << bits) - 1u);
  if (!sgn) {
    float v = (float)(raw & maxu);
    return norm ? v / (float)maxu : v;
  }
  Bit32s s = (Bit32s)(raw << (32 - bits)) >> (32 - bits);
  if (!norm)
    return (float)s;
  float half = (float)((1u << (bits - 1)) - 1u);
  switch (method) {
    case 1: {
      float v = (float)s / half;
      return v < -1.0f ? -1.0f : v;
    }
    case 2:
      return (2.0f * (float)s + 1.0f) / (float)maxu;
    default:
      return (float)s / half;
  }
}

// Decode a vertex dword stream into the 16 input vectors
void bx_radeon9800_c::vap_psc_decode(const Bit32u *dw, Bit32u ndw, r300_vap_in_t *in)
{
  Bit32u p = 0;
  Bit32u sgn_norm = VAPR(R9800_VAP_PSC_SGN_NORM_CNTL);

  for (int k = 0; k < 16; k++)
    for (int c = 0; c < 4; c++)
      in->v[k][c] = (c == 3) ? 1.0f : 0.0f;

  for (int k = 0; k < 16; k++) {
    Bit32u psc = VAPR(R9800_VAP_PROG_STREAM_CNTL_0 + (k >> 1) * 4);
    Bit32u ext = VAPR(R9800_VAP_PROG_STREAM_CNTL_EXT_0 + (k >> 1) * 4);
    Bit32u fld = (k & 1) ? (psc >> 16) : (psc & 0xffff);
    Bit32u efld = (k & 1) ? (ext >> 16) : (ext & 0xffff);
    Bit32u dt = fld & 0xf;
    Bit32u skip = (fld >> 4) & 0xf;
    Bit32u loc = (fld >> 8) & 0x1f;
    bool last = (fld & (1 << 13)) != 0;
    bool sgn = (fld & (1 << 14)) != 0;
    bool norm = (fld & (1 << 15)) != 0;
    int method = (sgn_norm >> (k * 2)) & 3;
    float v[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
    Bit32u d0 = (p < ndw) ? dw[p] : 0;
    Bit32u d1 = (p + 1 < ndw) ? dw[p + 1] : 0;
    Bit32u d2 = (p + 2 < ndw) ? dw[p + 2] : 0;
    Bit32u d3 = (p + 3 < ndw) ? dw[p + 3] : 0;
    Bit32u used;
    switch (dt) {
      case 0: v[0] = r9800_u2f(d0); used = 1; break;
      case 1: v[0] = r9800_u2f(d0); v[1] = r9800_u2f(d1); used = 2; break;
      case 2: v[0] = r9800_u2f(d0); v[1] = r9800_u2f(d1); v[2] = r9800_u2f(d2); used = 3; break;
      case 3: v[0] = r9800_u2f(d0); v[1] = r9800_u2f(d1); v[2] = r9800_u2f(d2); v[3] = r9800_u2f(d3); used = 4; break;
      case 4:
        v[0] = r3d_psc_fixed(d0 & 0xff, 8, sgn, norm, method);
        v[1] = r3d_psc_fixed((d0 >> 8) & 0xff, 8, sgn, norm, method);
        v[2] = r3d_psc_fixed((d0 >> 16) & 0xff, 8, sgn, norm, method);
        v[3] = r3d_psc_fixed(d0 >> 24, 8, sgn, norm, method);
        used = 1;
        break;
      case 5:
        v[2] = r3d_psc_fixed(d0 & 0xff, 8, sgn, norm, method);
        v[1] = r3d_psc_fixed((d0 >> 8) & 0xff, 8, sgn, norm, method);
        v[0] = r3d_psc_fixed((d0 >> 16) & 0xff, 8, sgn, norm, method);
        v[3] = r3d_psc_fixed(d0 >> 24, 8, sgn, norm, method);
        used = 1;
        break;
      case 6:
        v[0] = r3d_psc_fixed(d0 & 0xffff, 16, sgn, norm, method);
        v[1] = r3d_psc_fixed(d0 >> 16, 16, sgn, norm, method);
        used = 1;
        break;
      case 7:
        v[0] = r3d_psc_fixed(d0 & 0xffff, 16, sgn, norm, method);
        v[1] = r3d_psc_fixed(d0 >> 16, 16, sgn, norm, method);
        v[2] = r3d_psc_fixed(d1 & 0xffff, 16, sgn, norm, method);
        v[3] = r3d_psc_fixed(d1 >> 16, 16, sgn, norm, method);
        used = 2;
        break;
      case 8:
        v[0] = r3d_psc_fixed(d0 & 0x3ff, 10, sgn, norm, method);
        v[1] = r3d_psc_fixed((d0 >> 10) & 0x3ff, 10, sgn, norm, method);
        v[2] = r3d_psc_fixed((d0 >> 20) & 0x3ff, 10, sgn, norm, method);
        used = 1;
        break;
      case 9:
        v[0] = r3d_psc_fixed(d0 & 0x7ff, 11, sgn, norm, method);
        v[1] = r3d_psc_fixed((d0 >> 11) & 0x7ff, 11, sgn, norm, method);
        v[2] = r3d_psc_fixed((d0 >> 22) & 0x3ff, 10, sgn, norm, method);
        used = 1;
        break;
      default:
        used = 1;
        break;
    }
    p += used + skip;
    if (loc < 16) {
      Bit32u we = (efld >> 12) & 0xf;
      float o[4];
      for (int c = 0; c < 4; c++) {
        Bit32u sel = (efld >> (c * 3)) & 7;
        switch (sel) {
          case 0: case 1: case 2: case 3: o[c] = v[sel]; break;
          case 4: o[c] = 0.0f; break;
          case 5: o[c] = 1.0f; break;
          default: o[c] = v[c]; break;
        }
      }
      // a zero write enable (the extension word left at its reset value)
      // writes every component
      if (we == 0) we = 0xf;
      for (int c = 0; c < 4; c++)
        if (we & (1u << c)) in->v[loc][c] = o[c];
    }
    if (last)
      break;
  }
}

// Gather one vertex from the vertex arrays (array of structures)
bool bx_radeon9800_c::vap_fetch_aos(Bit32u idx, r300_vap_in_t *in)
{
  Bit32u narrays = VAPR(R9800_VAP_VTX_NUM_ARRAYS) & 0x1f;
  Bit32u dw[128];
  Bit32u ndw = 0;
  Bit32u minidx = VAPR(R9800_VAP_VF_MIN_VTX_INDX) & 0xffffff;
  Bit32u maxidx = VAPR(R9800_VAP_VF_MAX_VTX_INDX) & 0xffffff;

  if (idx < minidx) idx = minidx;
  if (idx > maxidx) idx = maxidx;
  for (Bit32u i = 0; i < narrays; i++) {
    Bit32u attr = VAPR(R9800_VAP_VTX_AOS_ATTR01 + (i >> 1) * 12);
    Bit32u count = (i & 1) ? ((attr >> 16) & 0x7f) : (attr & 0x7f);
    Bit32u stride = (i & 1) ? ((attr >> 24) & 0x7f) : ((attr >> 8) & 0x7f);
    Bit32u addr = VAPR(R9800_VAP_VTX_AOS_ADDR0 + (i >> 1) * 12 + (i & 1) * 4) & ~3u;
    if (ndw + count > 128)
      count = 128 - ndw;
    if (count == 0)
      continue;
    if (!gpu_read(addr + idx * stride * 4, (Bit8u*)&dw[ndw], count * 4))
      return false;
    for (Bit32u k = 0; k < count; k++)
      dw[ndw + k] = ReadHostDWordFromLittleEndian(&dw[ndw + k]);
    ndw += count;
  }
  vap_psc_decode(dw, ndw, in);
  return true;
}

// ---------------------------------------------------------------------
// Programmable vertex shader interpreter
// ---------------------------------------------------------------------

typedef struct {
  float temp[64][4];
  const r300_vap_in_t *in;
  float out[16][4];
  Bit32u out_written;
  int a0;
  int loop_index;
} r3d_pvs_ctx_t;

static void r3d_pvs_src(bx_radeon9800_c *dev, const r3d_pvs_ctx_t *ctx, const Bit32u (*vec)[4],
                        Bit32u s, Bit32u cbase, Bit32u cmax, float out[4])
{
  Bit32u type = R9800_PVS_SRC_REG_TYPE(s);
  Bit32u off = R9800_PVS_SRC_OFFSET(s);
  float src[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
  UNUSED(dev);
  if (s & R9800_PVS_SRC_REL_ADDR)
    off = (Bit32u)((int)off + ctx->a0);
  switch (type) {
    case R9800_PVS_SRC_REG_TEMP:
      memcpy(src, ctx->temp[off & 31], sizeof(src));
      break;
    case R9800_PVS_SRC_REG_INPUT:
      memcpy(src, ctx->in->v[off & 15], sizeof(src));
      break;
    case R9800_PVS_SRC_REG_CONST:
      if (off <= cmax) {
        Bit32u ci = (R9800_PVS_CONST_START + ((cbase + off) & 0xff)) & (R9800_PVS_VECTOR_MEM - 1);
        for (int c = 0; c < 4; c++)
          src[c] = r9800_u2f(vec[ci][c]);
      }
      break;
    default:
      memcpy(src, ctx->temp[32 + (off & 31)], sizeof(src));
      break;
  }
  for (int c = 0; c < 4; c++) {
    Bit32u sel = R9800_PVS_SRC_SWZ(s, c);
    float v;
    switch (sel) {
      case 0: case 1: case 2: case 3: v = src[sel]; break;
      case 4: v = 0.0f; break;
      case 5: v = 1.0f; break;
      default: v = src[c]; break;
    }
    if (R9800_PVS_SRC_NEG(s, c))
      v = -v;
    out[c] = v;
  }
}

void bx_radeon9800_c::vap_run_pvs(const r300_vap_in_t *in, r300_vs_out_t *out)
{
  r3d_pvs_ctx_t ctx;
  Bit32u cc0 = VAPR(R9800_VAP_PVS_CODE_CNTL_0);
  Bit32u first = cc0 & 0x3ff;
  Bit32u last = (cc0 >> 20) & 0x3ff;
  Bit32u const_cntl = VAPR(R9800_VAP_PVS_CONST_CNTL);
  Bit32u cbase = const_cntl & 0xff;
  Bit32u cmax = (const_cntl >> 16) & 0xff;
  Bit32u fc_opc = VAPR(R9800_VAP_PVS_FLOW_CNTL_OPC);
  int loop_cnt[16];
  Bit32u ret_stack[16];
  int ret_sp = 0;
  Bit32u pc = first;
  int steps = 0;

  memset(ctx.temp, 0, sizeof(ctx.temp));
  memset(ctx.out, 0, sizeof(ctx.out));
  ctx.in = in;
  ctx.out_written = 0;
  ctx.a0 = 0;
  ctx.loop_index = 0;
  for (int i = 0; i < 16; i++) loop_cnt[i] = 0;

  while ((pc <= last) && (pc < R9800_PVS_CODE_LINES) && (steps < 8192)) {
    const Bit32u *code = pvs_vec[pc];
    Bit32u dst = code[0];
    float s0[4], s1[4], s2[4], res[4];
    Bit32u op = R9800_PVS_DST_OPCODE(dst);
    bool math = (dst & R9800_PVS_DST_MATH_INST) != 0;
    steps++;

    r3d_pvs_src(this, &ctx, pvs_vec, code[1], cbase, cmax, s0);
    r3d_pvs_src(this, &ctx, pvs_vec, code[2], cbase, cmax, s1);
    r3d_pvs_src(this, &ctx, pvs_vec, code[3], cbase, cmax, s2);

    if (!math) {
      switch (op) {
        case R9800_VE_DOT_PRODUCT: {
          float d = s0[0] * s1[0] + s0[1] * s1[1] + s0[2] * s1[2] + s0[3] * s1[3];
          res[0] = res[1] = res[2] = res[3] = d;
          break;
        }
        case R9800_VE_MULTIPLY:
          for (int c = 0; c < 4; c++) res[c] = s0[c] * s1[c];
          break;
        case R9800_VE_ADD:
          for (int c = 0; c < 4; c++) res[c] = s0[c] + s1[c];
          break;
        case R9800_VE_MULTIPLY_ADD:
          for (int c = 0; c < 4; c++) res[c] = s0[c] * s1[c] + s2[c];
          break;
        case R9800_VE_DISTANCE_VECTOR:
          res[0] = 1.0f; res[1] = s0[1] * s1[1]; res[2] = s0[2]; res[3] = s1[3];
          break;
        case R9800_VE_FRACTION:
          for (int c = 0; c < 4; c++) res[c] = s0[c] - (float)floor(s0[c]);
          break;
        case R9800_VE_MAXIMUM:
          for (int c = 0; c < 4; c++) res[c] = s0[c] > s1[c] ? s0[c] : s1[c];
          break;
        case R9800_VE_MINIMUM:
          for (int c = 0; c < 4; c++) res[c] = s0[c] < s1[c] ? s0[c] : s1[c];
          break;
        case R9800_VE_SET_GREATER_THAN_EQUAL:
          for (int c = 0; c < 4; c++) res[c] = (s0[c] >= s1[c]) ? 1.0f : 0.0f;
          break;
        case R9800_VE_SET_LESS_THAN:
          for (int c = 0; c < 4; c++) res[c] = (s0[c] < s1[c]) ? 1.0f : 0.0f;
          break;
        case R9800_VE_MULTIPLYX2_ADD:
          for (int c = 0; c < 4; c++) res[c] = 2.0f * s0[c] * s1[c] + s2[c];
          break;
        case R9800_VE_MULTIPLY_CLAMP:
          for (int c = 0; c < 4; c++) res[c] = r3d_clamp01(s0[c] * s1[c]);
          break;
        case R9800_VE_FLT2FIX_DX:
          for (int c = 0; c < 4; c++) res[c] = (float)floor(s0[c]);
          break;
        case R9800_VE_FLT2FIX_DX_RND:
          for (int c = 0; c < 4; c++) res[c] = (float)floor(s0[c] + 0.5f);
          break;
        default:
          for (int c = 0; c < 4; c++) res[c] = s0[c];
          break;
      }
      if (dst & R9800_PVS_DST_VE_SAT)
        for (int c = 0; c < 4; c++) res[c] = r3d_clamp01(res[c]);
    } else {
      float x = s0[0];
      float r = 0.0f;
      bool vec_res = false;
      switch (op) {
        case R9800_ME_EXP_BASE2_DX: {
          float fl = (float)floor(x);
          res[0] = (float)pow(2.0, fl);
          res[1] = x - fl;
          res[2] = (float)pow(2.0, x);
          res[3] = 1.0f;
          vec_res = true;
          break;
        }
        case R9800_ME_LOG_BASE2_DX: {
          float ax = (float)fabs(x);
          if (ax > 0.0f) {
            float l = (float)(log(ax) / log(2.0));
            float fl = (float)floor(l);
            res[0] = fl;
            res[1] = ax / (float)pow(2.0, fl);
            res[2] = l;
          } else {
            res[0] = -3.4e38f; res[1] = 1.0f; res[2] = -3.4e38f;
          }
          res[3] = 1.0f;
          vec_res = true;
          break;
        }
        case R9800_ME_EXP_BASEE_FF: r = (float)exp(x); break;
        case R9800_ME_LIGHT_COEFF_DX: {
          float w = r3d_clampf(s0[3], -128.0f, 128.0f);
          res[0] = 1.0f;
          res[1] = x > 0.0f ? x : 0.0f;
          res[2] = (x > 0.0f) ? (float)pow(s0[1] > 0.0f ? s0[1] : 0.0f, w) : 0.0f;
          res[3] = 1.0f;
          vec_res = true;
          break;
        }
        case R9800_ME_POWER_FUNC_FF:
          r = (float)pow((double)fabs(x), (double)s2[0]);
          break;
        case R9800_ME_POWER_FUNC_FF_CLAMP_B:
        case R9800_ME_POWER_FUNC_FF_CLAMP_B1:
        case R9800_ME_POWER_FUNC_FF_CLAMP_01:
          r = (float)pow((double)r3d_clamp01(x), (double)s2[0]);
          break;
        case R9800_ME_RECIP_DX:
        case R9800_ME_RECIP_FF:
          r = (x != 0.0f) ? 1.0f / x : 3.4e38f;
          break;
        case R9800_ME_RECIP_SQRT_DX:
        case R9800_ME_RECIP_SQRT_FF:
          r = (x != 0.0f) ? 1.0f / (float)sqrt(fabs(x)) : 3.4e38f;
          break;
        case R9800_ME_MULTIPLY: r = x * s1[0]; break;
        case R9800_ME_EXP_BASE2_FULL_DX: r = (float)pow(2.0, x); break;
        case R9800_ME_LOG_BASE2_FULL_DX:
          r = (x != 0.0f) ? (float)(log(fabs(x)) / log(2.0)) : -3.4e38f;
          break;
        case R9800_ME_SIN: r = (float)sin(x); break;
        case R9800_ME_COS: r = (float)cos(x); break;
        default: r = x; break;
      }
      if (!vec_res)
        res[0] = res[1] = res[2] = res[3] = r;
      if (dst & R9800_PVS_DST_ME_SAT)
        for (int c = 0; c < 4; c++) res[c] = r3d_clamp01(res[c]);
    }

    // destination
    {
      Bit32u we = R9800_PVS_DST_WE(dst);
      Bit32u type = R9800_PVS_DST_REG_TYPE(dst);
      Bit32u off = R9800_PVS_DST_OFFSET(dst);
      float *d = NULL;
      switch (type) {
        case R9800_PVS_DST_REG_TEMP: d = ctx.temp[off & 31]; break;
        case R9800_PVS_DST_REG_A0:
          ctx.a0 = (int)floor(res[0]);
          break;
        case R9800_PVS_DST_REG_OUT:
          d = ctx.out[off & 15];
          ctx.out_written |= 1u << (off & 15);
          break;
        case R9800_PVS_DST_REG_OUT_REPL_X:
          d = ctx.out[off & 15];
          ctx.out_written |= 1u << (off & 15);
          res[1] = res[2] = res[3] = res[0];
          break;
        case R9800_PVS_DST_REG_ALT_TEMP: d = ctx.temp[32 + (off & 31)]; break;
        default: break;
      }
      if (d) {
        for (int c = 0; c < 4; c++)
          if (we & (1u << c)) d[c] = res[c];
      }
    }

    // flow control
    Bit32u next = pc + 1;
    for (int e = 0; e < 16; e++) {
      Bit32u opc = (fc_opc >> (e * 2)) & 3;
      if (!opc)
        continue;
      Bit32u fca = VAPR(R9800_VAP_PVS_FLOW_CNTL_ADDRS_0 + e * 4);
      Bit32u act = fca & 0xff, jmp = (fca >> 8) & 0xff, lst = (fca >> 16) & 0xff, rtn = (fca >> 24) & 0xff;
      Bit32u li = VAPR(R9800_VAP_PVS_FLOW_CNTL_LOOP_INDEX_0 + e * 4);
      if (opc == 1) {
        if (pc == act) next = jmp;
      } else if (opc == 2) {
        if (pc == act) {
          loop_cnt[e] = (int)jmp;
          ctx.loop_index = (int)(li & 0xff);
        }
        if ((pc == lst) && (loop_cnt[e] > 0)) {
          loop_cnt[e]--;
          if (loop_cnt[e] > 0) {
            next = rtn;
            ctx.loop_index += (int)(Bit8s)((li >> 8) & 0xff);
          }
        }
      } else {
        if ((pc == act) && (ret_sp < 16)) {
          ret_stack[ret_sp++] = rtn;
          next = jmp;
        } else if ((pc == lst) && (ret_sp > 0)) {
          next = ret_stack[--ret_sp];
        }
      }
    }
    pc = next;
  }

  // Route the output registers to the vertex components: the outputs are
  // numbered densely in the order position, point size, colours, textures
  Bit32u fmt0 = VAPR(R9800_VAP_OUT_VTX_FMT_0);
  Bit32u fmt1 = VAPR(R9800_VAP_OUT_VTX_FMT_1);
  int k = 0;
  memset(out, 0, sizeof(*out));
  out->pos[3] = 1.0f;
  out->psize = 1.0f;
  if (fmt0 & 1) { memcpy(out->pos, ctx.out[k], sizeof(out->pos)); k++; }
  if (fmt0 & (1 << 16)) { out->psize = ctx.out[k][0]; k++; }
  for (int c = 0; c < 4; c++) {
    if (fmt0 & (2u << c)) {
      memcpy(out->col[c], ctx.out[k & 15], sizeof(out->col[c]));
      out->col_present |= 1 << c;
      k++;
    }
  }
  for (int t = 0; t < 8; t++) {
    if ((fmt1 >> (t * 3)) & 7) {
      memcpy(out->tex[t], ctx.out[k & 15], sizeof(out->tex[t]));
      out->tex_present |= 1 << t;
      k++;
    }
  }
}

// TCL bypass: the input vectors are the output vertex, in output order
void bx_radeon9800_c::vap_bypass(const r300_vap_in_t *in, r300_vs_out_t *out)
{
  Bit32u fmt0 = VAPR(R9800_VAP_OUT_VTX_FMT_0);
  Bit32u fmt1 = VAPR(R9800_VAP_OUT_VTX_FMT_1);
  int k = 0;
  memset(out, 0, sizeof(*out));
  out->pos[3] = 1.0f;
  out->psize = 1.0f;
  if (fmt0 & 1) { memcpy(out->pos, in->v[k], sizeof(out->pos)); k++; }
  if (fmt0 & (1 << 16)) { out->psize = in->v[k][0]; k++; }
  for (int c = 0; c < 4; c++) {
    if (fmt0 & (2u << c)) {
      memcpy(out->col[c], in->v[k & 15], sizeof(out->col[c]));
      out->col_present |= 1 << c;
      k++;
    }
  }
  for (int t = 0; t < 8; t++) {
    if ((fmt1 >> (t * 3)) & 7) {
      memcpy(out->tex[t], in->v[k & 15], sizeof(out->tex[t]));
      out->tex_present |= 1 << t;
      k++;
    }
  }
}

// ---------------------------------------------------------------------
// Viewport transform and clipping
// ---------------------------------------------------------------------

void bx_radeon9800_c::vap_vte(const r300_vs_out_t *vso, r300_vtx_t *out)
{
  Bit32u vte = VAPR(R9800_VAP_VTE_CNTL);
  Bit32u fmt1 = r3d_regs[R3D(R9800_GB_VAP_RASTER_VTX_FMT_1)];
  Bit32u fmt0 = r3d_regs[R3D(R9800_GB_VAP_RASTER_VTX_FMT_0)];
  float x = vso->pos[0], y = vso->pos[1], z = vso->pos[2], w = vso->pos[3];
  float w0;

  if (vte & R9800_VTE_W0_FMT)
    w0 = (w != 0.0f) ? 1.0f / w : 0.0f;
  else
    w0 = w;
  if (!(vte & R9800_VTE_XY_FMT)) { x *= w0; y *= w0; }
  if (!(vte & R9800_VTE_Z_FMT)) z *= w0;
  if (vte & R9800_VTE_X_SCALE_ENA)  x *= r9800_u2f(VAPR(R9800_VAP_VPORT_XSCALE));
  if (vte & R9800_VTE_X_OFFSET_ENA) x += r9800_u2f(VAPR(R9800_VAP_VPORT_XOFFSET));
  if (vte & R9800_VTE_Y_SCALE_ENA)  y *= r9800_u2f(VAPR(R9800_VAP_VPORT_YSCALE));
  if (vte & R9800_VTE_Y_OFFSET_ENA) y += r9800_u2f(VAPR(R9800_VAP_VPORT_YOFFSET));
  if (vte & R9800_VTE_Z_SCALE_ENA)  z *= r9800_u2f(VAPR(R9800_VAP_VPORT_ZSCALE));
  if (vte & R9800_VTE_Z_OFFSET_ENA) z += r9800_u2f(VAPR(R9800_VAP_VPORT_ZOFFSET));
  {
    Bit32u ga_off = r3d_regs[R3D(R9800_GA_OFFSET)];
    float sub = (r3d_regs[R3D(R9800_GB_TILE_CONFIG)] & R9800_GB_SUBPIXEL_1_16) ? 16.0f : 12.0f;
    x += (float)(Bit16s)(ga_off & 0xffff) / sub;
    y += (float)(Bit16s)(ga_off >> 16) / sub;
  }
  out->pos[0] = x;
  out->pos[1] = y;
  out->pos[2] = z;
  out->pos[3] = w0;
  out->psize = vso->psize;
  // colours: the present ones compacted in order
  int nc = 0;
  for (int c = 0; c < 4; c++) {
    if (fmt0 & (2u << c)) {
      memcpy(out->col[nc], vso->col[c], sizeof(out->col[nc]));
      nc++;
    }
  }
  for (; nc < 4; nc++) {
    out->col[nc][0] = out->col[nc][1] = out->col[nc][2] = 0.0f;
    out->col[nc][3] = 1.0f;
  }
  // texture components packed per GB_VAP_RASTER_VTX_FMT_1
  int p = 0;
  for (int t = 0; t < 8; t++) {
    int cnt = (fmt1 >> (t * 3)) & 7;
    if (cnt > 4) cnt = 4;
    for (int c = 0; c < cnt; c++) {
      if (p < 32) out->tc[p++] = vso->tex[t][c];
    }
  }
  for (; p < 32; p++) out->tc[p] = 0.0f;
}

static void r3d_vso_lerp(const r300_vs_out_t *a, const r300_vs_out_t *b, float t, r300_vs_out_t *o)
{
  for (int c = 0; c < 4; c++) o->pos[c] = a->pos[c] + (b->pos[c] - a->pos[c]) * t;
  for (int k = 0; k < 4; k++)
    for (int c = 0; c < 4; c++) o->col[k][c] = a->col[k][c] + (b->col[k][c] - a->col[k][c]) * t;
  for (int k = 0; k < 8; k++)
    for (int c = 0; c < 4; c++) o->tex[k][c] = a->tex[k][c] + (b->tex[k][c] - a->tex[k][c]) * t;
  o->psize = a->psize + (b->psize - a->psize) * t;
  o->col_present = a->col_present;
  o->tex_present = a->tex_present;
}

// Sutherland-Hodgman in clip space: 6 frustum planes and the enabled user
// clip planes. Returns the clipped vertex count (0 = fully outside).
int bx_radeon9800_c::vap_clip_poly(r300_vs_out_t *poly, int n, r300_vs_out_t *tmp)
{
  Bit32u clip_cntl = VAPR(R9800_VAP_CLIP_CNTL);
  bool dx_z = (VAPR(R9800_VAP_CNTL) & (1 << 22)) != 0;
  float planes[12][4];
  int np = 0;

  // dist = a*x + b*y + c*z + d*w >= 0 keeps the vertex
  planes[np][0] = 1; planes[np][1] = 0; planes[np][2] = 0; planes[np][3] = 1; np++;   // x >= -w
  planes[np][0] = -1; planes[np][1] = 0; planes[np][2] = 0; planes[np][3] = 1; np++;  // x <= w
  planes[np][0] = 0; planes[np][1] = 1; planes[np][2] = 0; planes[np][3] = 1; np++;
  planes[np][0] = 0; planes[np][1] = -1; planes[np][2] = 0; planes[np][3] = 1; np++;
  planes[np][0] = 0; planes[np][1] = 0; planes[np][2] = 1; planes[np][3] = dx_z ? 0.0f : 1.0f; np++;  // near
  planes[np][0] = 0; planes[np][1] = 0; planes[np][2] = -1; planes[np][3] = 1; np++;   // far
  if (!(clip_cntl & R9800_UCP_CULL_ONLY)) {
    for (int u = 0; u < 6; u++) {
      if (clip_cntl & (1u << u)) {
        for (int c = 0; c < 4; c++)
          planes[np][c] = r9800_u2f(pvs_vec[R9800_PVS_UCP_START + u][c]);
        np++;
      }
    }
  }
  r300_vs_out_t *cur = poly, *nxt = tmp;
  for (int p = 0; p < np; p++) {
    int m = 0;
    if (n == 0) return 0;
    for (int i = 0; i < n; i++) {
      const r300_vs_out_t *a = &cur[i];
      const r300_vs_out_t *b = &cur[(i + 1) % n];
      float da = planes[p][0] * a->pos[0] + planes[p][1] * a->pos[1] + planes[p][2] * a->pos[2] + planes[p][3] * a->pos[3];
      float db = planes[p][0] * b->pos[0] + planes[p][1] * b->pos[1] + planes[p][2] * b->pos[2] + planes[p][3] * b->pos[3];
      bool ina = da >= 0.0f, inb = db >= 0.0f;
      if (ina) {
        if (m < 32) nxt[m++] = *a;
      }
      if (ina != inb) {
        float t = da / (da - db);
        if (m < 32) r3d_vso_lerp(a, b, t, &nxt[m++]);
      }
    }
    r300_vs_out_t *sw = cur; cur = nxt; nxt = sw;
    n = m;
  }
  if (cur != poly)
    for (int i = 0; i < n; i++) poly[i] = cur[i];
  return n;
}

// User clip plane culling only (UCP_CULL_ONLY): drop the primitive when
// every vertex lies outside one enabled plane
static bool r3d_ucp_culled(const Bit32u (*vec)[4], Bit32u clip_cntl, const r300_vs_out_t *v, int n)
{
  if (!(clip_cntl & R9800_UCP_CULL_ONLY))
    return false;
  for (int u = 0; u < 6; u++) {
    if (!(clip_cntl & (1u << u)))
      continue;
    float pl[4];
    for (int c = 0; c < 4; c++) pl[c] = r9800_u2f(vec[R9800_PVS_UCP_START + u][c]);
    bool all_out = true;
    for (int i = 0; i < n; i++) {
      float d = pl[0] * v[i].pos[0] + pl[1] * v[i].pos[1] + pl[2] * v[i].pos[2] + pl[3] * v[i].pos[3];
      if (d >= 0.0f) { all_out = false; break; }
    }
    if (all_out) return true;
  }
  return false;
}

// ---------------------------------------------------------------------
// Primitive assembly (GA / SU)
// ---------------------------------------------------------------------

static BX_CPP_INLINE float r3d_tri_area(const r300_vtx_t *a, const r300_vtx_t *b, const r300_vtx_t *c)
{
  return (b->pos[0] - a->pos[0]) * (c->pos[1] - a->pos[1]) - (c->pos[0] - a->pos[0]) * (b->pos[1] - a->pos[1]);
}

void bx_radeon9800_c::vap_tri(const r300_vs_out_t *a, const r300_vs_out_t *b, const r300_vs_out_t *c)
{
  Bit32u clip_cntl = VAPR(R9800_VAP_CLIP_CNTL);
  Bit32u vte = VAPR(R9800_VAP_VTE_CNTL);
  r300_vs_out_t poly[32], tmp[32];
  int n = 3;
  bool pretransformed = (vte & R9800_VTE_XY_FMT) && !(vte & R9800_VTE_W0_FMT);

  poly[0] = *a; poly[1] = *b; poly[2] = *c;
  if (r3d_ucp_culled(pvs_vec, clip_cntl, poly, 3))
    return;
  if (!(clip_cntl & R9800_CLIP_DISABLE) && !pretransformed) {
    n = vap_clip_poly(poly, 3, tmp);
    if (n < 3)
      return;
  }
  r300_vtx_t wv[32];
  for (int i = 0; i < n; i++)
    vap_vte(&poly[i], &wv[i]);

  const r300_draw_state_t *d = &draw.rs->d;
  // face determination on the original triangle
  float area = r3d_tri_area(&wv[0], &wv[1], &wv[2]);
  if (n > 3) {
    // clipped polygons: use the largest fan triangle for the sign
    float best = 0.0f;
    for (int i = 1; i + 1 < n; i++) {
      float ar = r3d_tri_area(&wv[0], &wv[i], &wv[i + 1]);
      if (fabs(ar) > fabs(best)) best = ar;
    }
    area = best;
  }
  if (area == 0.0f)
    return;
  bool front = d->face_cw ? (area < 0.0f) : (area > 0.0f);
  if (front && d->cull_front) return;
  if (!front && d->cull_back) return;

  int ptype = 2;
  if (d->poly_mode)
    ptype = front ? d->front_ptype : d->back_ptype;
  if (ptype == 0) {
    for (int i = 0; i < n; i++)
      raster_submit(draw.rs, RB_POINT, &wv[i], 1, front);
    return;
  }
  if (ptype == 1) {
    for (int i = 0; i < n; i++) {
      r300_vtx_t seg[2];
      seg[0] = wv[i];
      seg[1] = wv[(i + 1) % n];
      raster_submit(draw.rs, RB_LINE, seg, 2, front);
    }
    return;
  }
  for (int i = 1; i + 1 < n; i++) {
    r300_vtx_t tri[3];
    tri[0] = wv[0]; tri[1] = wv[i]; tri[2] = wv[i + 1];
    raster_submit(draw.rs, RB_TRI, tri, 3, front);
  }
}

void bx_radeon9800_c::vap_line(const r300_vs_out_t *a, const r300_vs_out_t *b)
{
  Bit32u clip_cntl = VAPR(R9800_VAP_CLIP_CNTL);
  Bit32u vte = VAPR(R9800_VAP_VTE_CNTL);
  r300_vs_out_t seg[2] = { *a, *b };
  bool pretransformed = (vte & R9800_VTE_XY_FMT) && !(vte & R9800_VTE_W0_FMT);

  if (r3d_ucp_culled(pvs_vec, clip_cntl, seg, 2))
    return;
  if (!(clip_cntl & R9800_CLIP_DISABLE) && !pretransformed) {
    // clip the segment against the frustum
    bool dx_z = (VAPR(R9800_VAP_CNTL) & (1 << 22)) != 0;
    float planes[12][4];
    int np = 0;
    planes[np][0] = 1; planes[np][1] = 0; planes[np][2] = 0; planes[np][3] = 1; np++;
    planes[np][0] = -1; planes[np][1] = 0; planes[np][2] = 0; planes[np][3] = 1; np++;
    planes[np][0] = 0; planes[np][1] = 1; planes[np][2] = 0; planes[np][3] = 1; np++;
    planes[np][0] = 0; planes[np][1] = -1; planes[np][2] = 0; planes[np][3] = 1; np++;
    planes[np][0] = 0; planes[np][1] = 0; planes[np][2] = 1; planes[np][3] = dx_z ? 0.0f : 1.0f; np++;
    planes[np][0] = 0; planes[np][1] = 0; planes[np][2] = -1; planes[np][3] = 1; np++;
    if (!(clip_cntl & R9800_UCP_CULL_ONLY)) {
      for (int u = 0; u < 6; u++) {
        if (clip_cntl & (1u << u)) {
          for (int c = 0; c < 4; c++)
            planes[np][c] = r9800_u2f(pvs_vec[R9800_PVS_UCP_START + u][c]);
          np++;
        }
      }
    }
    for (int p = 0; p < np; p++) {
      float da = planes[p][0] * seg[0].pos[0] + planes[p][1] * seg[0].pos[1] + planes[p][2] * seg[0].pos[2] + planes[p][3] * seg[0].pos[3];
      float db = planes[p][0] * seg[1].pos[0] + planes[p][1] * seg[1].pos[1] + planes[p][2] * seg[1].pos[2] + planes[p][3] * seg[1].pos[3];
      if ((da < 0.0f) && (db < 0.0f))
        return;
      if (da < 0.0f) {
        r300_vs_out_t o;
        r3d_vso_lerp(&seg[0], &seg[1], da / (da - db), &o);
        seg[0] = o;
      } else if (db < 0.0f) {
        r300_vs_out_t o;
        r3d_vso_lerp(&seg[0], &seg[1], da / (da - db), &o);
        seg[1] = o;
      }
    }
  }
  r300_vtx_t wv[2];
  vap_vte(&seg[0], &wv[0]);
  vap_vte(&seg[1], &wv[1]);
  raster_submit(draw.rs, RB_LINE, wv, 2, 1);
}

void bx_radeon9800_c::vap_point(const r300_vs_out_t *a)
{
  Bit32u clip_cntl = VAPR(R9800_VAP_CLIP_CNTL);
  Bit32u vte = VAPR(R9800_VAP_VTE_CNTL);
  bool pretransformed = (vte & R9800_VTE_XY_FMT) && !(vte & R9800_VTE_W0_FMT);

  if (r3d_ucp_culled(pvs_vec, clip_cntl, a, 1))
    return;
  if (!(clip_cntl & R9800_CLIP_DISABLE) && !pretransformed) {
    float w = a->pos[3];
    bool dx_z = (VAPR(R9800_VAP_CNTL) & (1 << 22)) != 0;
    if ((w <= 0.0f) || (a->pos[2] > w) || (a->pos[2] < (dx_z ? 0.0f : -w)))
      return;
    for (int u = 0; u < 6; u++) {
      if (!(clip_cntl & (1u << u)))
        continue;
      float d = 0.0f;
      for (int c = 0; c < 4; c++) d += r9800_u2f(pvs_vec[R9800_PVS_UCP_START + u][c]) * a->pos[c];
      if (d < 0.0f) return;
    }
  }
  r300_vtx_t wv;
  vap_vte(a, &wv);
  raster_submit(draw.rs, RB_POINT, &wv, 1, 1);
}

// Feed one processed vertex to the primitive assembler
void bx_radeon9800_c::vap_assemble(const r300_vs_out_t *v)
{
  switch (draw.prim) {
    case 1:   // point list
    case 11:  // point sprites
      vap_point(v);
      break;
    case 2:   // line list
      draw.ring[draw.ring_n++] = *v;
      if (draw.ring_n == 2) {
        vap_line(&draw.ring[0], &draw.ring[1]);
        draw.ring_n = 0;
      }
      break;
    case 3:   // line strip
    case 12:  // line loop
      if (draw.ring_n == 0) {
        draw.first = *v;
        draw.prev = *v;
        draw.ring_n = 1;
      } else {
        vap_line(&draw.prev, v);
        draw.prev = *v;
      }
      break;
    case 4:   // triangle list
    case 7:   // triangle with wflags
    case 9:   // 3-vertex point list
    case 10:  // 3-vertex line list
      draw.ring[draw.ring_n++] = *v;
      if (draw.ring_n == 3) {
        if (draw.prim == 9) {
          vap_point(&draw.ring[0]); vap_point(&draw.ring[1]); vap_point(&draw.ring[2]);
        } else if (draw.prim == 10) {
          vap_line(&draw.ring[0], &draw.ring[1]);
          vap_line(&draw.ring[1], &draw.ring[2]);
          vap_line(&draw.ring[2], &draw.ring[0]);
        } else {
          vap_tri(&draw.ring[0], &draw.ring[1], &draw.ring[2]);
        }
        draw.ring_n = 0;
      }
      break;
    case 5:   // triangle fan
    case 15:  // polygon
      if (draw.ring_n == 0) {
        draw.first = *v;
        draw.ring_n = 1;
      } else if (draw.ring_n == 1) {
        draw.prev = *v;
        draw.ring_n = 2;
      } else {
        vap_tri(&draw.first, &draw.prev, v);
        draw.prev = *v;
      }
      break;
    case 6:   // triangle strip
      if (draw.ring_n < 2) {
        draw.ring[draw.ring_n++] = *v;
      } else {
        if (draw.even & 1)
          vap_tri(&draw.ring[1], &draw.ring[0], v);
        else
          vap_tri(&draw.ring[0], &draw.ring[1], v);
        draw.ring[0] = draw.ring[1];
        draw.ring[1] = *v;
        draw.even++;
      }
      break;
    case 8: { // rectangle list: three vertices define an axis aligned rect
      draw.ring[draw.ring_n++] = *v;
      if (draw.ring_n == 3) {
        r300_vs_out_t d = draw.ring[2];
        for (int c = 0; c < 4; c++)
          d.pos[c] = draw.ring[0].pos[c] + draw.ring[2].pos[c] - draw.ring[1].pos[c];
        for (int k = 0; k < 4; k++)
          for (int c = 0; c < 4; c++)
            d.col[k][c] = draw.ring[0].col[k][c] + draw.ring[2].col[k][c] - draw.ring[1].col[k][c];
        for (int k = 0; k < 8; k++)
          for (int c = 0; c < 4; c++)
            d.tex[k][c] = draw.ring[0].tex[k][c] + draw.ring[2].tex[k][c] - draw.ring[1].tex[k][c];
        vap_tri(&draw.ring[0], &draw.ring[1], &draw.ring[2]);
        vap_tri(&draw.ring[0], &draw.ring[2], &d);
        draw.ring_n = 0;
      }
      break;
    }
    case 13:  // quad list
      if (draw.ring_n < 3) {
        draw.ring[draw.ring_n++] = *v;
      } else {
        vap_tri(&draw.ring[0], &draw.ring[1], &draw.ring[2]);
        vap_tri(&draw.ring[0], &draw.ring[2], v);
        draw.ring_n = 0;
      }
      break;
    case 14:  // quad strip
      if (draw.ring_n < 3) {
        draw.ring[draw.ring_n++] = *v;
      } else {
        // v0 v1 / v2 v3: quad (v0, v1, v3, v2)
        vap_tri(&draw.ring[0], &draw.ring[1], v);
        vap_tri(&draw.ring[0], v, &draw.ring[2]);
        draw.ring[0] = draw.ring[2];
        draw.ring[1] = *v;
        draw.ring_n = 2;
      }
      break;
    default:
      break;
  }
}

void bx_radeon9800_c::vap_process_input(const r300_vap_in_t *in, r300_vtx_t *out, r300_vs_out_t *vso)
{
  UNUSED(out);
  if (VAPR(R9800_VAP_CNTL_STATUS) & R9800_VAP_PVS_BYPASS)
    vap_bypass(in, vso);
  else
    vap_run_pvs(in, vso);
  vap_emit(vso);
}

void bx_radeon9800_c::vap_emit(const r300_vs_out_t *vso)
{
  draw.emitted++;
  vap_assemble(vso);
  if ((draw.walk != R9800_VF_WALK_STATE) && draw.num && (draw.emitted >= draw.num))
    vap_end();
}

void bx_radeon9800_c::vap_begin(Bit32u vf_cntl)
{
  if (draw.active)
    vap_end();
  draw.active = true;
  draw.vf_cntl = vf_cntl;
  draw.prim = R9800_VF_PRIM_TYPE(vf_cntl);
  draw.walk = R9800_VF_PRIM_WALK(vf_cntl);
  draw.num = R9800_VF_NUM_VERTICES(vf_cntl);
  draw.idx32 = (vf_cntl & R9800_VF_INDEX_SIZE_32) != 0;
  draw.emitted = 0;
  draw.vdw_n = 0;
  draw.vdw_per_vtx = VAPR(R9800_VAP_VTX_SIZE) & 0x7f;
  if (draw.vdw_per_vtx == 0)
    draw.vdw_per_vtx = r3d_psc_dwords(vap_regs);
  if (draw.vdw_per_vtx > 128) draw.vdw_per_vtx = 128;
  draw.ring_n = 0;
  draw.poly_n = 0;
  draw.even = 0;
  raster_state_capture(draw.rs);
}

void bx_radeon9800_c::vap_end(void)
{
  if (!draw.active)
    return;
  if ((draw.prim == 12) && (draw.ring_n > 0))
    vap_line(&draw.prev, &draw.first);
  draw.active = false;
  draw.ring_n = 0;
  draw.vdw_n = 0;
}

// Vertex data dwords from the command stream
void bx_radeon9800_c::vap_vertex_dwords(const Bit32u *dw, Bit32u n)
{
  for (Bit32u i = 0; i < n; i++) {
    if (!draw.active)
      return;
    if (draw.vdw_n < 128)
      draw.vdw[draw.vdw_n] = dw[i];
    draw.vdw_n++;
    if (draw.vdw_n >= draw.vdw_per_vtx) {
      r300_vap_in_t in;
      r300_vtx_t v;
      r300_vs_out_t vso;
      vap_psc_decode(draw.vdw, draw.vdw_n, &in);
      draw.vdw_n = 0;
      vap_process_input(&in, &v, &vso);
    }
  }
}

void bx_radeon9800_c::vap_index(Bit32u idx)
{
  r300_vap_in_t in;
  r300_vtx_t v;
  r300_vs_out_t vso;
  if (!draw.active)
    return;
  if (!vap_fetch_aos(idx, &in)) {
    draw.emitted++;
    return;
  }
  vap_process_input(&in, &v, &vso);
}

void bx_radeon9800_c::r3d_draw_embedded(Bit32u vf_cntl, const Bit32u *data, Bit32u ndw)
{
  if (R9800_VF_PRIM_TYPE(vf_cntl) == 0)
    return;
  vap_begin(vf_cntl);
  vap_vertex_dwords(data, ndw);
  if (draw.active)
    vap_end();
}

void bx_radeon9800_c::r3d_draw_indices(Bit32u vf_cntl, const Bit32u *idx, Bit32u ndw)
{
  if (R9800_VF_PRIM_TYPE(vf_cntl) == 0)
    return;
  vap_begin(vf_cntl);
  // with no indices in the packet, an INDX_BUFFER packet follows and feeds
  // VAP_PORT_IDX; the draw stays open until all vertices arrived
  for (Bit32u i = 0; i < ndw; i++) {
    if (!draw.active)
      return;
    r3d_port_idx_write(idx[i]);
  }
  if (draw.active && ndw && (draw.emitted >= draw.num))
    vap_end();
}

void bx_radeon9800_c::r3d_draw_list(Bit32u vf_cntl)
{
  Bit32u num = R9800_VF_NUM_VERTICES(vf_cntl);
  Bit32u base = VAPR(R9800_VAP_INDEX_OFFSET);
  if (R9800_VF_PRIM_TYPE(vf_cntl) == 0)
    return;
  vap_begin(vf_cntl);
  for (Bit32u i = 0; (i < num) && draw.active; i++)
    vap_index(base + i);
  if (draw.active)
    vap_end();
}

// =====================================================================
// Draw state capture
// =====================================================================

// Crack the fragment shader registers into the decoded program
void bx_radeon9800_c::r3d_decode_fp(r9800_raster_state_t *rs)
{
  r300_fp_t *fp = &rs->fp;
  const Bit32u *r = rs->r;
  Bit32u cfg = r[R3D(R9800_US_CONFIG)];
  Bit32u off = r[R3D(R9800_US_CODE_OFFSET)];
  int nlevel = cfg & 7;
  int alu_off = off & 0x3f;
  int tex_off = (off >> 13) & 0x1f;

  fp->nodes = (nlevel > 3) ? 4 : nlevel + 1;
  fp->first_tex = (cfg >> 3) & 1;
  for (int i = 0; i < fp->nodes; i++) {
    int lvl = 4 - fp->nodes + i;   // valid levels are the last NLEVEL+1
    Bit32u ca = r[R3D(R9800_US_CODE_ADDR_0) + lvl];
    fp->node[i].alu_start = (int)((ca & 0x3f) + alu_off) & 0x3f;
    fp->node[i].alu_cnt = (int)((ca >> 6) & 0x3f) + 1;
    fp->node[i].tex_start = (int)(((ca >> 12) & 0x1f) + tex_off) & 0x1f;
    fp->node[i].tex_cnt = (int)((ca >> 17) & 0x1f) + 1;
    fp->node[i].rgba_out = (ca >> 22) & 1;
    fp->node[i].w_out = (ca >> 23) & 1;
    if ((i == 0) && !fp->first_tex)
      fp->node[i].tex_cnt = 0;
  }
  fp->pixsize = (int)(r[R3D(R9800_US_PIXSIZE)] & 0x1f) + 1;
  fp->w_fmt = r[R3D(R9800_US_W_FMT)] & 3;
  fp->w_src = (r[R3D(R9800_US_W_FMT)] >> 2) & 1;
  for (int t = 0; t < 4; t++) {
    Bit32u of = r[R3D(R9800_US_OUT_FMT_0) + t];
    fp->out_fmt[t] = of & 0x1f;
    for (int c = 0; c < 4; c++)
      fp->out_sel[t][c] = (of >> (8 + c * 2)) & 3;
    fp->out_sign[t] = (of >> 16) & 0xf;
  }
  for (int i = 0; i < R9800_US_ALU_INSTS; i++) {
    r300_alu_inst_t *a = &fp->alu[i];
    Bit32u ra = r[R3D(R9800_US_ALU_RGB_ADDR_0) + i];
    Bit32u aa = r[R3D(R9800_US_ALU_ALPHA_ADDR_0) + i];
    Bit32u ri = r[R3D(R9800_US_ALU_RGB_INST_0) + i];
    Bit32u ai = r[R3D(R9800_US_ALU_ALPHA_INST_0) + i];
    a->rgb.src[0] = ra & 0x3f;
    a->rgb.src[1] = (ra >> 6) & 0x3f;
    a->rgb.src[2] = (ra >> 12) & 0x3f;
    a->rgb.dst = (ra >> 18) & 0x1f;
    a->rgb.wmask = (ra >> 23) & 7;
    a->rgb.omask = (ra >> 26) & 7;
    a->rgb.target = (ra >> 29) & 3;
    a->rgb.sel[0] = ri & 0x1f;
    a->rgb.mod[0] = (ri >> 5) & 3;
    a->rgb.sel[1] = (ri >> 7) & 0x1f;
    a->rgb.mod[1] = (ri >> 12) & 3;
    a->rgb.sel[2] = (ri >> 14) & 0x1f;
    a->rgb.mod[2] = (ri >> 19) & 3;
    a->rgb.srcp_op = (ri >> 21) & 3;
    a->rgb.op = (ri >> 23) & 0xf;
    a->rgb.omod = (ri >> 27) & 7;
    a->rgb.clamp = (ri >> 30) & 1;
    a->alpha.src[0] = aa & 0x3f;
    a->alpha.src[1] = (aa >> 6) & 0x3f;
    a->alpha.src[2] = (aa >> 12) & 0x3f;
    a->alpha.dst = (aa >> 18) & 0x1f;
    a->alpha.wmask = (aa >> 23) & 1;
    a->alpha.omask = (aa >> 24) & 1;
    a->alpha.target = (aa >> 25) & 3;
    a->alpha_w_out = (aa >> 27) & 1;
    a->alpha.sel[0] = ai & 0x1f;
    a->alpha.mod[0] = (ai >> 5) & 3;
    a->alpha.sel[1] = (ai >> 7) & 0x1f;
    a->alpha.mod[1] = (ai >> 12) & 3;
    a->alpha.sel[2] = (ai >> 14) & 0x1f;
    a->alpha.mod[2] = (ai >> 19) & 3;
    a->alpha.srcp_op = (ai >> 21) & 3;
    a->alpha.op = (ai >> 23) & 0xf;
    a->alpha.omod = (ai >> 27) & 7;
    a->alpha.clamp = (ai >> 30) & 1;
  }
  for (int i = 0; i < R9800_US_TEX_INSTS; i++) {
    Bit32u ti = r[R3D(R9800_US_TEX_INST_0) + i];
    fp->tex[i].src = ti & 0x1f;
    fp->tex[i].dst = (ti >> 6) & 0x1f;
    fp->tex[i].tex_id = (ti >> 11) & 0xf;
    fp->tex[i].op = (ti >> 15) & 7;
  }
  for (int i = 0; i < R9800_US_CONSTS; i++) {
    fp->konst[i][0] = r3d_float24(r[R3D(R9800_US_ALU_CONST_R_0) + i * 4 + 0]);
    fp->konst[i][1] = r3d_float24(r[R3D(R9800_US_ALU_CONST_R_0) + i * 4 + 1]);
    fp->konst[i][2] = r3d_float24(r[R3D(R9800_US_ALU_CONST_R_0) + i * 4 + 2]);
    fp->konst[i][3] = r3d_float24(r[R3D(R9800_US_ALU_CONST_R_0) + i * 4 + 3]);
  }
}

// Bytes per texel for a texture format (0 = block compressed)
static int r3d_tx_bpp(int fmt)
{
  switch (fmt) {
    case 0: case 2: case 5: case 18: return 1;
    case 1: case 3: case 6: case 7: case 10: case 11: case 20: case 21: case 22: case 24: return 2;
    case 4: case 8: case 9: case 12: case 13: case 19: case 23: case 25: case 27: case 30: return 4;
    case 14: case 26: case 28: return 8;
    case 29: return 16;
    case 15: case 16: case 17: case 31: return 0;
    default: return 4;
  }
}

static int r3d_tx_cmp(int fmt)
{
  switch (fmt) {
    case 15: return 1;   // DXT1
    case 16: return 2;   // DXT3
    case 17: return 3;   // DXT5
    case 31: return 4;   // ATI2N
    default: return 0;
  }
}

// Texture descriptors: geometry, level layout, addressing
void bx_radeon9800_c::r3d_decode_textures(r9800_raster_state_t *rs)
{
  const Bit32u *r = rs->r;
  Bit32u en = r[R3D(R9800_TX_ENABLE)];
  Bit32u fb_start = (mc_fb_location & 0xffff) << 16;
  UNUSED(fb_start);

  for (int u = 0; u < R9800_TX_MAPS; u++) {
    r300_tex_desc_t *t = &rs->tex[u];
    Bit32u f0 = r[R3D(R9800_TX_FILTER0_0) + u];
    Bit32u f1 = r[R3D(R9800_TX_FILTER1_0) + u];
    Bit32u fm0 = r[R3D(R9800_TX_FORMAT0_0) + u];
    Bit32u fm1 = r[R3D(R9800_TX_FORMAT1_0) + u];
    Bit32u fm2 = r[R3D(R9800_TX_FORMAT2_0) + u];
    Bit32u of = r[R3D(R9800_TX_OFFSET_0) + u];
    memset(t, 0, sizeof(*t));
    t->en = (en >> u) & 1;
    t->stage_off = R9800_TEX_STAGE_NONE;
    if (!t->en)
      continue;
    t->fmt = (int)(fm1 & 0x1f) | (int)(((fm2 >> 14) & 1) << 5);
    t->fmt &= 0x1f;
    t->w = (int)(fm0 & 0x7ff) + 1;
    t->h = (int)((fm0 >> 11) & 0x7ff) + 1;
    t->depth_log2 = (fm0 >> 22) & 0xf;
    t->levels = (int)((fm0 >> 26) & 0xf) + 1;
    if (t->levels > R300_TEX_LEVELS) t->levels = R300_TEX_LEVELS;
    t->projected = (fm0 >> 30) & 1;
    t->pitch_en = (fm0 >> 31) & 1;
    t->pitch_px = t->pitch_en ? ((fm2 & 0x3fff) + 1) : (Bit32u)t->w;
    t->clamp_s = f0 & 7;
    t->clamp_t = (f0 >> 3) & 7;
    t->clamp_r = (f0 >> 6) & 7;
    t->mag = (f0 >> 9) & 3;
    t->min = (f0 >> 11) & 3;
    t->mip = (f0 >> 13) & 3;
    t->vol = (f0 >> 15) & 3;
    t->max_mip = (f0 >> 17) & 0xf;
    if (t->max_mip >= t->levels) t->max_mip = t->levels - 1;
    t->ck_mode = f1 & 3;
    t->lod_bias = (float)(((Bit32s)((f1 >> 3) & 0x3ff) << 22) >> 22) / 32.0f;
    t->signed_comp = (fm1 >> 5) & 0xf;
    t->sel[0] = (fm1 >> 9) & 7;    // alpha
    t->sel[1] = (fm1 >> 12) & 7;   // red
    t->sel[2] = (fm1 >> 15) & 7;   // green
    t->sel[3] = (fm1 >> 18) & 7;   // blue
    t->gamma = (fm1 >> 21) & 1;
    t->yuv = (fm1 >> 22) & 3;
    t->swap_yuv = (fm1 >> 24) & 1;
    t->coord_type = (fm1 >> 25) & 3;
    t->endian = of & 3;
    t->macro = (of >> 2) & 1;
    t->micro = (of >> 3) & 3;
    if (t->micro == 3) t->micro = 1;
    t->offset = of & 0xffffffe0;
    t->chroma = r[R3D(R9800_TX_CHROMA_KEY_0) + u];
    t->border = r[R3D(R9800_TX_BORDER_COLOR_0) + u];
    t->bpp = r3d_tx_bpp(t->fmt);
    t->cmp = r3d_tx_cmp(t->fmt);
    // level layout: sequential, layers / cube faces consecutive per level
    Bit32u total = 0;
    Bit32u w = (Bit32u)t->w, h = (Bit32u)t->h;
    Bit32u depth = 1u << t->depth_log2;
    Bit32u layers0 = (t->coord_type == 2) ? 6 : ((t->coord_type == 1) ? depth : 1);
    for (int l = 0; l < t->levels; l++) {
      Bit32u pitch_b, rows;
      Bit32u lw = w ? w : 1, lh = h ? h : 1;
      if (t->cmp) {
        Bit32u bw = (lw + 3) >> 2, bh = (lh + 3) >> 2;
        pitch_b = bw * ((t->cmp == 1) ? 8u : 16u);
        if (t->pitch_en && (l == 0))
          pitch_b = ((t->pitch_px + 3) >> 2) * ((t->cmp == 1) ? 8u : 16u);
        pitch_b = (pitch_b + 31) & ~31u;
        rows = bh;
      } else {
        Bit32u pw = (t->pitch_en && (l == 0)) ? t->pitch_px : lw;
        if (t->pitch_en && (l > 0)) {
          pw = t->pitch_px >> l;
          if (pw < lw) pw = lw;
        }
        pitch_b = pw * (Bit32u)t->bpp;
        if (t->macro || t->micro) {
          Bit32u mw, mh;
          if (t->macro) r300_macro_geom((Bit32u)t->bpp, t->micro, &mw, &mh);
          else r300_micro_geom((Bit32u)t->bpp, t->micro, &mw, &mh);
          pitch_b = ((pitch_b + mw * t->bpp - 1) / (mw * t->bpp)) * (mw * t->bpp);
          rows = ((lh + mh - 1) / mh) * mh;
        } else {
          pitch_b = (pitch_b + 31) & ~31u;
          rows = lh;
        }
      }
      Bit32u layers = (t->coord_type == 2) ? 6 : ((t->coord_type == 1) ? (depth >> l ? depth >> l : 1) : 1);
      t->lvl_off[l] = total;
      t->lvl_pitch[l] = pitch_b;
      t->lvl_w[l] = lw;
      t->lvl_h[l] = lh;
      t->lvl_d[l] = layers;
      t->lvl_layer[l] = pitch_b * rows;
      total += t->lvl_layer[l] * layers;
      total = (total + 31) & ~31u;
      w >>= 1; h >>= 1;
    }
    UNUSED(layers0);
    t->total = total;
    Bit32u voff;
    if (mc_is_vram(t->offset, &voff) && ((Bit64u)voff + total <= vram_size))
      t->vram_off = voff;
    else
      t->vram_off = 0xffffffffu;
  }
}

// Stage one level range from system memory into the arena; returns the
// arena offset or R9800_TEX_STAGE_NONE.
Bit32u bx_radeon9800_c::r3d_stage_level(Bit32u key, Bit32u vm, Bit32u len)
{
  for (Bit32u i = 0; i < tex_stage.ent_count; i++)
    if ((tex_stage.ent[i].vm_base == key) && (tex_stage.ent[i].len == len))
      return tex_stage.ent[i].arena_off;
  Bit32u off = (tex_stage.used + 31u) & ~31u;
  if ((tex_stage.arena == NULL) || (off + len > tex_stage.cap) || (off < tex_stage.used)) {
    Bit32u ncap = tex_stage.cap ? tex_stage.cap : (1u << 20);
    while (ncap < off + len) ncap <<= 1;
    Bit8u *na = (Bit8u*)realloc(tex_stage.arena, ncap);
    if (na == NULL)
      return R9800_TEX_STAGE_NONE;
    tex_stage.arena = na;
    tex_stage.cap = ncap;
  }
  if (!gpu_read(vm, tex_stage.arena + off, len))
    memset(tex_stage.arena + off, 0xff, len);
  tex_stage.used = off + len;
  if (tex_stage.ent_count < 256) {
    tex_stage.ent[tex_stage.ent_count].vm_base = key;
    tex_stage.ent[tex_stage.ent_count].len = len;
    tex_stage.ent[tex_stage.ent_count].arena_off = off;
    tex_stage.ent_count++;
  }
  return off;
}

// Stage every enabled texture that is not frame buffer resident
void bx_radeon9800_c::r3d_stage_textures(r9800_raster_state_t *rs)
{
  rs->tex_lo = 0xffffffff;
  rs->tex_hi = 0;
  for (int u = 0; u < R9800_TX_MAPS; u++) {
    r300_tex_desc_t *t = &rs->tex[u];
    if (!t->en || !t->total)
      continue;
    if (t->vram_off != 0xffffffffu) {
      if (t->vram_off < rs->tex_lo) rs->tex_lo = t->vram_off;
      if (t->vram_off + t->total > rs->tex_hi) rs->tex_hi = t->vram_off + t->total;
      continue;
    }
    if (t->total > (64u << 20)) {
      rs->stage_dead = 1;
      continue;
    }
    t->stage_off = r3d_stage_level(t->offset, t->offset, t->total);
    if (t->stage_off == R9800_TEX_STAGE_NONE)
      rs->stage_dead = 1;
  }
}

// Crack every register field the rasterizer consumes into rs->d
void bx_radeon9800_c::r3d_draw_state_derive(r9800_raster_state_t *rs)
{
  r300_draw_state_t *d = &rs->d;
  const Bit32u *r = rs->r;
  Bit32u v;

  memset(d, 0, sizeof(*d));
  // scissor and clip rectangles (coordinates carry the 1440 offset)
  v = r[R3D(R9800_SC_SCISSOR0)];
  d->sx0 = (int)(v & 0x1fff) - R9800_SC_COORD_OFFSET;
  d->sy0 = (int)((v >> 13) & 0x1fff) - R9800_SC_COORD_OFFSET;
  v = r[R3D(R9800_SC_SCISSOR1)];
  d->sx1 = (int)(v & 0x1fff) - R9800_SC_COORD_OFFSET;
  d->sy1 = (int)((v >> 13) & 0x1fff) - R9800_SC_COORD_OFFSET;
  d->clip_rule = r[R3D(R9800_SC_CLIP_RULE)] & 0xffff;
  for (int i = 0; i < 4; i++) {
    Bit32u a = r[R3D(R9800_SC_CLIP_0_A) + i * 2];
    Bit32u b = r[R3D(R9800_SC_CLIP_0_A) + i * 2 + 1];
    d->cx0[i] = (int)(a & 0x1fff) - R9800_SC_COORD_OFFSET;
    d->cy0[i] = (int)((a >> 13) & 0x1fff) - R9800_SC_COORD_OFFSET;
    d->cx1[i] = (int)(b & 0x1fff) - R9800_SC_COORD_OFFSET;
    d->cy1[i] = (int)((b >> 13) & 0x1fff) - R9800_SC_COORD_OFFSET;
  }
  d->sub = (r[R3D(R9800_GB_TILE_CONFIG)] & R9800_GB_SUBPIXEL_1_16) ? 16 : 12;
  d->subf = (float)d->sub;
  d->geom_round = r[R3D(R9800_GA_ROUND_MODE)] & 3;
  // shading
  v = r[R3D(R9800_GA_COLOR_CONTROL)];
  for (int i = 0; i < 4; i++) {
    d->rgb_shade[i] = (v >> (i * 4)) & 3;
    d->a_shade[i] = (v >> (i * 4 + 2)) & 3;
  }
  d->provoke = (v >> 16) & 3;
  {
    Bit32u rg = r[R3D(R9800_GA_SOLID_RG)], ba = r[R3D(R9800_GA_SOLID_BA)];
    d->solid[0] = (float)(Bit16s)(rg >> 16) / 4096.0f;
    d->solid[1] = (float)(Bit16s)(rg & 0xffff) / 4096.0f;
    d->solid[2] = (float)(Bit16s)(ba >> 16) / 4096.0f;
    d->solid[3] = (float)(Bit16s)(ba & 0xffff) / 4096.0f;
  }
  // points / lines
  v = r[R3D(R9800_GA_POINT_SIZE)];
  d->point_hh = (float)(v & 0xffff) / d->subf;
  d->point_hw = (float)(v >> 16) / d->subf;
  v = r[R3D(R9800_GA_POINT_MINMAX)];
  d->point_min = (float)(v & 0xffff) / d->subf;
  d->point_max = (float)(v >> 16) / d->subf;
  v = r[R3D(R9800_GB_ENABLE)];
  d->point_stuff = v & 1;
  d->line_stuff = (v >> 1) & 1;
  d->tri_stuff = (v >> 2) & 1;
  for (int i = 0; i < 8; i++)
    d->tex_src[i] = (v >> (16 + i * 2)) & 3;
  d->ps0 = r9800_u2f(r[R3D(R9800_GA_POINT_S0)]);
  d->pt0 = r9800_u2f(r[R3D(R9800_GA_POINT_T0)]);
  d->ps1 = r9800_u2f(r[R3D(R9800_GA_POINT_S1)]);
  d->pt1 = r9800_u2f(r[R3D(R9800_GA_POINT_T1)]);
  d->ls0 = r9800_u2f(r[R3D(R9800_GA_LINE_S0)]);
  d->ls1 = r9800_u2f(r[R3D(R9800_GA_LINE_S1)]);
  v = r[R3D(R9800_GA_LINE_CNTL)];
  d->line_hw = (float)(v & 0xffff) / d->subf;
  d->line_end = (v >> 16) & 3;
  // culling / polygon mode / offsets
  v = r[R3D(R9800_SU_CULL_MODE)];
  d->cull_front = v & 1;
  d->cull_back = (v >> 1) & 1;
  d->face_cw = (v >> 2) & 1;
  v = r[R3D(R9800_GA_POLY_MODE)];
  d->poly_mode = v & 3;
  d->front_ptype = (v >> 4) & 7;
  d->back_ptype = (v >> 7) & 7;
  if (d->front_ptype > 2) d->front_ptype = 2;
  if (d->back_ptype > 2) d->back_ptype = 2;
  v = r[R3D(R9800_SU_POLY_OFFSET_ENABLE)];
  d->poff_front_en = v & 1;
  d->poff_back_en = (v >> 1) & 1;
  d->poff_fscale = r9800_u2f(r[R3D(R9800_SU_POLY_OFFSET_FRONT_SCALE)]);
  d->poff_foff = r9800_u2f(r[R3D(R9800_SU_POLY_OFFSET_FRONT_OFFSET)]);
  d->poff_bscale = r9800_u2f(r[R3D(R9800_SU_POLY_OFFSET_BACK_SCALE)]);
  d->poff_boff = r9800_u2f(r[R3D(R9800_SU_POLY_OFFSET_BACK_OFFSET)]);
  d->depth_scale = r9800_u2f(r[R3D(R9800_SU_DEPTH_SCALE)]);
  d->depth_off = r9800_u2f(r[R3D(R9800_SU_DEPTH_OFFSET)]);
  d->tex_wrap = r[R3D(R9800_SU_TEX_WRAP)];
  // fog
  v = r[R3D(R9800_FG_FOG_BLEND)];
  d->fog_en = v & 1;
  d->fog_fn = (v >> 1) & 3;
  v = r[R3D(R9800_GB_SELECT)];
  d->fog_sel = v & 7;
  d->depth_sel = (v >> 3) & 1;
  d->w_sel = (v >> 4) & 1;
  d->fog_scale = r9800_u2f(r[R3D(R9800_GA_FOG_SCALE)]);
  d->fog_off = r9800_u2f(r[R3D(R9800_GA_FOG_OFFSET)]);
  d->fog_factor = r3d_fix09(r[R3D(R9800_FG_FOG_FACTOR)]);
  d->fog_col[0] = r3d_fix09(r[R3D(R9800_FG_FOG_COLOR_R)]);
  d->fog_col[1] = r3d_fix09(r[R3D(R9800_FG_FOG_COLOR_G)]);
  d->fog_col[2] = r3d_fix09(r[R3D(R9800_FG_FOG_COLOR_B)]);
  // alpha test
  v = r[R3D(R9800_FG_ALPHA_FUNC)];
  d->af_val = (float)(v & 0xff) / 255.0f;
  d->af_fn = (v >> 8) & 7;
  d->af_en = (v >> 11) & 1;
  d->dith_en = (v >> 20) & 1;
  d->depth_src = r[R3D(R9800_FG_DEPTH_SRC)] & 1;
  // colour buffer
  v = r[R3D(R9800_RB3D_BLENDCNTL)];
  d->blend_en = v & 1;
  d->sep_alpha = (v >> 1) & 1;
  d->discard_src = (v >> 3) & 7;
  d->cfn = (v >> 12) & 7;
  d->csrc = (v >> 16) & 0x3f;
  d->cdst = (v >> 24) & 0x3f;
  v = r[R3D(R9800_RB3D_ABLENDCNTL)];
  d->afn = (v >> 12) & 7;
  d->asrc = (v >> 16) & 0x3f;
  d->adst = (v >> 24) & 0x3f;
  if (!d->sep_alpha) { d->afn = d->cfn; d->asrc = d->csrc; d->adst = d->cdst; }
  v = r[R3D(R9800_RB3D_CONSTANT_COLOR)];
  d->const_col[0] = ((v >> 16) & 0xff) / 255.0f;
  d->const_col[1] = ((v >> 8) & 0xff) / 255.0f;
  d->const_col[2] = (v & 0xff) / 255.0f;
  d->const_col[3] = (v >> 24) / 255.0f;
  d->chan_mask = r[R3D(R9800_RB3D_COLOR_CHANNEL_MASK)] & 0xf;
  v = r[R3D(R9800_RB3D_ROPCNTL)];
  d->rop_en = (v >> 2) & 1;
  d->rop = (v >> 8) & 0xf;
  v = r[R3D(R9800_RB3D_CCTL)];
  d->multiwrite = (int)((v >> 5) & 3) + 1;
  d->clrcmp_en = (v >> 7) & 1;
  d->clrcmp_clr = r[R3D(R9800_RB3D_CLRCMP_CLR)];
  d->clrcmp_msk = r[R3D(R9800_RB3D_CLRCMP_MSK)];
  d->clrcmp_flipe = r[R3D(R9800_RB3D_CLRCMP_FLIPE)];
  v = r[R3D(R9800_RB3D_DITHER_CTL)];
  d->dither_mode = v & 3;
  d->adither_mode = (v >> 2) & 3;
  v = r[R3D(R9800_GB_AA_CONFIG)];
  d->aa_en = v & 1;
  {
    static const int ns[4] = { 2, 3, 4, 6 };
    d->aa_samples = d->aa_en ? ns[(v >> 1) & 3] : 1;
  }
  v = r[R3D(R9800_RB3D_AARESOLVE_CTL)];
  d->aa_resolve = v & 1;
  d->aar_gamma = (v >> 1) & 1;
  d->aar_off = r[R3D(R9800_RB3D_AARESOLVE_OFFSET)] & 0xffffffe0;
  d->aar_pitch_px = (r[R3D(R9800_RB3D_AARESOLVE_PITCH)] & 0x3ffe);
  if (!mc_is_vram(d->aar_off, &d->aar_vram))
    d->aar_vram = 0xffffffffu;
  v = r[R3D(R9800_RB3D_COLORPITCH0)];
  d->cb_pitch_px = v & 0x3ffe;
  d->cb_macro = (v >> 16) & 1;
  d->cb_micro = (v >> 17) & 3;
  if (d->cb_micro == 3) d->cb_micro = 1;
  d->cb_fmt = (v >> 21) & 0xf;
  switch (d->cb_fmt) {
    case 3: case 4: case 11: case 12: case 13: case 15: d->cb_bpp = 2; break;
    case 6: d->cb_bpp = 4; break;
    case 7: d->cb_bpp = 16; break;
    case 9: d->cb_bpp = 1; break;
    case 10: d->cb_bpp = 8; break;
    default: d->cb_bpp = 0; break;
  }
  d->cb_ok = (d->cb_bpp != 0) && (d->cb_pitch_px != 0);
  for (int i = 0; i < 4; i++) {
    d->cb_off[i] = r[R3D(R9800_RB3D_COLOROFFSET0) + i] & 0xffffffe0;
    if (!mc_is_vram(d->cb_off[i], &d->cb_vram[i]))
      d->cb_vram[i] = 0xffffffffu;
  }
  if (d->cb_vram[0] == 0xffffffffu)
    d->cb_ok = false;
  // depth / stencil
  v = r[R3D(R9800_ZB_CNTL)];
  d->sten_en = v & 1;
  d->z_en = (v >> 1) & 1;
  d->z_wr = (v >> 2) & 1;
  d->z_signed = (v >> 3) & 1;
  d->sten_fb = (v >> 4) & 1;
  v = r[R3D(R9800_ZB_ZSTENCILCNTL)];
  d->z_fn = v & 7;
  d->sfn = (v >> 3) & 7;
  d->sfail = (v >> 6) & 7;
  d->szpass = (v >> 9) & 7;
  d->szfail = (v >> 12) & 7;
  d->sfn_bf = (v >> 15) & 7;
  d->sfail_bf = (v >> 18) & 7;
  d->szpass_bf = (v >> 21) & 7;
  d->szfail_bf = (v >> 24) & 7;
  if (!d->sten_fb) {
    d->sfn_bf = d->sfn; d->sfail_bf = d->sfail; d->szpass_bf = d->szpass; d->szfail_bf = d->szfail;
  }
  v = r[R3D(R9800_ZB_STENCILREFMASK)];
  d->sref = v & 0xff;
  d->smask = (v >> 8) & 0xff;
  d->swmask = (v >> 16) & 0xff;
  d->zb_fmt = r[R3D(R9800_ZB_FORMAT)] & 0xf;
  d->zb_bpp = (d->zb_fmt == 2) ? 4 : 2;
  v = r[R3D(R9800_ZB_DEPTHPITCH)];
  d->zb_pitch_px = v & 0x3ffc;
  d->zb_macro = (v >> 16) & 1;
  d->zb_micro = (v >> 17) & 3;
  if (d->zb_micro == 3) d->zb_micro = 1;
  d->zb_off = r[R3D(R9800_ZB_DEPTHOFFSET)] & 0xffffffe0;
  d->zb_ok = (d->zb_pitch_px != 0) && mc_is_vram(d->zb_off, &d->zb_vram);
  d->ztop = r[R3D(R9800_ZB_ZTOP)] & 1;
  v = r[R3D(R9800_ZB_BW_CNTL)];
  d->hiz_en = v & 1;
  d->fast_fill = (v >> 2) & 1;
  d->rd_comp = (v >> 3) & 1;
  d->wr_comp = (v >> 4) & 1;
  d->z_clear = r[R3D(R9800_ZB_DEPTHCLEARVALUE)];
  v = r[R3D(R9800_ZB_DEPTHXY_OFFSET)];
  d->zx_off = (v >> 1) & 0x7ff;
  d->zy_off = (v >> 17) & 0x7ff;
  d->zmask_off = r[R3D(R9800_ZB_ZMASK_OFFSET)];
  d->zmask_pitch = r[R3D(R9800_ZB_ZMASK_PITCH)];
  // rasterizer routing
  v = r[R3D(R9800_RS_INST_COUNT)];
  d->rs_count = (int)(v & 0xf) + 1;
  d->rs_w_en = (v >> 4) & 1;
  v = r[R3D(R9800_RS_COUNT)];
  d->it_count = v & 0x7f;
  d->ic_count = (v >> 7) & 0xf;
  for (int i = 0; i < 16; i++) {
    Bit32u in = r[R3D(R9800_RS_INST_0) + i];
    d->rs_inst[i].tex_id = in & 7;
    d->rs_inst[i].tex_cn = (in >> 3) & 7;
    d->rs_inst[i].tex_addr = (in >> 6) & 0x1f;
    d->rs_inst[i].col_id = (in >> 11) & 7;
    d->rs_inst[i].col_cn = (in >> 14) & 7;
    d->rs_inst[i].col_addr = (in >> 17) & 0x1f;
  }
  for (int i = 0; i < 8; i++) {
    Bit32u ip = r[R3D(R9800_RS_IP_0) + i];
    d->rs_ip[i].tex_ptr = ip & 0x3f;
    d->rs_ip[i].col_ptr = (ip >> 6) & 7;
    d->rs_ip[i].col_fmt = (ip >> 9) & 0xf;
    d->rs_ip[i].sel[0] = (ip >> 13) & 7;
    d->rs_ip[i].sel[1] = (ip >> 16) & 7;
    d->rs_ip[i].sel[2] = (ip >> 19) & 7;
    d->rs_ip[i].sel[3] = (ip >> 22) & 7;
  }
  v = r[R3D(R9800_GB_VAP_RASTER_VTX_FMT_0)];
  d->n_col = 0;
  for (int c = 0; c < 4; c++) if (v & (2u << c)) d->n_col++;
  v = r[R3D(R9800_GB_VAP_RASTER_VTX_FMT_1)];
  d->raster_fmt1 = v;
  d->n_tc = 0;
  for (int t = 0; t < 8; t++) d->n_tc += (v >> (t * 3)) & 7;
  if (d->n_tc > 32) d->n_tc = 32;
  d->tx_enable = r[R3D(R9800_TX_ENABLE)] & 0xffff;
  d->draw_ok = d->cb_ok || (d->z_en && d->zb_ok) || (d->sten_en && d->zb_ok);
  if (!d->cb_ok && !d->zb_ok)
    d->draw_ok = 0;
}

void bx_radeon9800_c::raster_state_capture(r9800_raster_state_t *rs)
{
  // Serial mode never flushes: recycle the staging arena per draw
  if (rb.nthreads <= 1) {
    tex_stage.used = 0;
    tex_stage.ent_count = 0;
  }
  rs->serial = r3d_serial;
  memcpy(rs->r, r3d_regs, sizeof(rs->r));
  rs->stage_dead = 0;
  r3d_decode_fp(rs);
  r3d_decode_textures(rs);
  r3d_draw_state_derive(rs);
  r3d_stage_textures(rs);
}

// =====================================================================
// Texture unit
// =====================================================================

static BX_CPP_INLINE float r3d_half_to_float(Bit32u h)
{
  Bit32u s = (h >> 15) & 1, e = (h >> 10) & 0x1f, m = h & 0x3ff;
  Bit32u f;
  if (e == 0) {
    if (m == 0) {
      f = s << 31;
    } else {
      // denormal
      e = 1;
      while (!(m & 0x400)) { m <<= 1; e--; }
      m &= 0x3ff;
      f = (s << 31) | ((e + 112) << 23) | (m << 13);
    }
  } else if (e == 31) {
    f = (s << 31) | 0x7f800000 | (m << 13);
  } else {
    f = (s << 31) | ((e + 112) << 23) | (m << 13);
  }
  return r9800_u2f(f);
}

static BX_CPP_INLINE float r3d_unorm(Bit32u v, int bits, bool sgn)
{
  Bit32u maxu = (1u << bits) - 1u;
  if (!sgn)
    return (float)v / (float)maxu;
  Bit32s s = (Bit32s)(v << (32 - bits)) >> (32 - bits);
  float r = (float)s / (float)(maxu >> 1);
  return r < -1.0f ? -1.0f : r;
}

static BX_CPP_INLINE float r3d_degamma(float v)
{
  if (v <= 0.04045f) return v / 12.92f;
  return (float)pow((v + 0.055f) / 1.055f, 2.4);
}

static void r3d_yuv_rgb(float y, float u, float v, int clamp, float *out)
{
  float c = y * 255.0f - 16.0f, d = u * 255.0f - 128.0f, e = v * 255.0f - 128.0f;
  float r = (1.164f * c + 1.596f * e) / 255.0f;
  float g = (1.164f * c - 0.391f * d - 0.813f * e) / 255.0f;
  float b = (1.164f * c + 2.018f * d) / 255.0f;
  if (clamp) { r = r3d_clamp01(r); g = r3d_clamp01(g); b = r3d_clamp01(b); }
  out[0] = r; out[1] = g; out[2] = b; out[3] = 1.0f;
}

// DXT block decode of one texel: (A, B, G, R) component order
static void r3d_dxt_texel(const Bit8u *blk, int cmp, int u, int v, float *c)
{
  const Bit8u *col = blk + ((cmp >= 2) ? 8 : 0);
  Bit32u c0 = col[0] | (col[1] << 8), c1 = col[2] | (col[3] << 8);
  Bit32u bits = col[4] | (col[5] << 8) | ((Bit32u)col[6] << 16) | ((Bit32u)col[7] << 24);
  int idx = (v & 3) * 4 + (u & 3);
  Bit32u sel = (bits >> (idx * 2)) & 3;
  float r0 = ((c0 >> 11) & 0x1f) / 31.0f, g0 = ((c0 >> 5) & 0x3f) / 63.0f, b0 = (c0 & 0x1f) / 31.0f;
  float r1 = ((c1 >> 11) & 0x1f) / 31.0f, g1 = ((c1 >> 5) & 0x3f) / 63.0f, b1 = (c1 & 0x1f) / 31.0f;
  float r, g, b, a = 1.0f;
  bool four = (c0 > c1) || (cmp >= 2);
  switch (sel) {
    case 0: r = r0; g = g0; b = b0; break;
    case 1: r = r1; g = g1; b = b1; break;
    case 2:
      if (four) { r = (2 * r0 + r1) / 3; g = (2 * g0 + g1) / 3; b = (2 * b0 + b1) / 3; }
      else { r = (r0 + r1) / 2; g = (g0 + g1) / 2; b = (b0 + b1) / 2; }
      break;
    default:
      if (four) { r = (r0 + 2 * r1) / 3; g = (g0 + 2 * g1) / 3; b = (b0 + 2 * b1) / 3; }
      else { r = g = b = 0.0f; a = 0.0f; }
      break;
  }
  if (cmp == 2) {
    Bit32u an = blk[idx >> 1];
    a = ((idx & 1) ? (an >> 4) : (an & 0xf)) / 15.0f;
  } else if (cmp == 3) {
    Bit32u a0 = blk[0], a1 = blk[1];
    Bit64u aw = 0;
    for (int i = 0; i < 6; i++) aw |= (Bit64u)blk[2 + i] << (i * 8);
    Bit32u ac = (Bit32u)((aw >> (idx * 3)) & 7);
    Bit32u av;
    if (ac == 0) av = a0;
    else if (ac == 1) av = a1;
    else if (a0 > a1) av = ((8 - ac) * a0 + (ac - 1) * a1) / 7;
    else if (ac == 6) av = 0;
    else if (ac == 7) av = 255;
    else av = ((6 - ac) * a0 + (ac - 1) * a1) / 5;
    a = av / 255.0f;
  }
  c[0] = a; c[1] = b; c[2] = g; c[3] = r;
}

// One 8-byte DXT5-style alpha block value (ATI2N channels)
static float r3d_ati2n_chan(const Bit8u *blk, int u, int v)
{
  int idx = (v & 3) * 4 + (u & 3);
  Bit32u a0 = blk[0], a1 = blk[1];
  Bit64u aw = 0;
  for (int i = 0; i < 6; i++) aw |= (Bit64u)blk[2 + i] << (i * 8);
  Bit32u ac = (Bit32u)((aw >> (idx * 3)) & 7);
  Bit32u av;
  if (ac == 0) av = a0;
  else if (ac == 1) av = a1;
  else if (a0 > a1) av = ((8 - ac) * a0 + (ac - 1) * a1) / 7;
  else if (ac == 6) av = 0;
  else if (ac == 7) av = 255;
  else av = ((6 - ac) * a0 + (ac - 1) * a1) / 5;
  return av / 255.0f;
}

// Decode the raw texel at p into its (up to four) components, component 0
// being the lowest bits. YUV formats are converted to RGBA directly
// (their swizzle is ignored) and flagged through *is_rgba.
static void r3d_decode_texel(const r300_tex_desc_t *t, const Bit8u *p, int u_odd, float *c, bool *is_rgba)
{
  int sg = t->signed_comp;
  Bit32u w0, w1;
  *is_rgba = false;
  c[0] = c[1] = c[2] = c[3] = 0.0f;
  switch (t->fmt) {
    case 0:  c[0] = r3d_unorm(p[0], 8, sg & 1); break;
    case 1:  c[0] = r3d_unorm(p[0] | (p[1] << 8), 16, sg & 1); break;
    case 2:  c[0] = r3d_unorm(p[0] & 0xf, 4, sg & 1); c[1] = r3d_unorm(p[0] >> 4, 4, sg & 2); break;
    case 3:  c[0] = r3d_unorm(p[0], 8, sg & 1); c[1] = r3d_unorm(p[1], 8, sg & 2); break;
    case 4:  c[0] = r3d_unorm(p[0] | (p[1] << 8), 16, sg & 1); c[1] = r3d_unorm(p[2] | (p[3] << 8), 16, sg & 2); break;
    case 5:  c[0] = r3d_unorm(p[0] & 3, 2, sg & 1); c[1] = r3d_unorm((p[0] >> 2) & 7, 3, sg & 2); c[2] = r3d_unorm(p[0] >> 5, 3, sg & 4); break;
    case 6:  w0 = p[0] | (p[1] << 8);
             c[0] = r3d_unorm(w0 & 0x1f, 5, sg & 1); c[1] = r3d_unorm((w0 >> 5) & 0x3f, 6, sg & 2); c[2] = r3d_unorm(w0 >> 11, 5, sg & 4); break;
    case 7:  w0 = p[0] | (p[1] << 8);
             c[0] = r3d_unorm(w0 & 0x1f, 5, sg & 1); c[1] = r3d_unorm((w0 >> 5) & 0x1f, 5, sg & 2); c[2] = r3d_unorm(w0 >> 10, 6, sg & 4); break;
    case 8:  w0 = p[0] | (p[1] << 8) | ((Bit32u)p[2] << 16) | ((Bit32u)p[3] << 24);
             c[0] = r3d_unorm(w0 & 0x3ff, 10, sg & 1); c[1] = r3d_unorm((w0 >> 10) & 0x7ff, 11, sg & 2); c[2] = r3d_unorm(w0 >> 21, 11, sg & 4); break;
    case 9:  w0 = p[0] | (p[1] << 8) | ((Bit32u)p[2] << 16) | ((Bit32u)p[3] << 24);
             c[0] = r3d_unorm(w0 & 0x7ff, 11, sg & 1); c[1] = r3d_unorm((w0 >> 11) & 0x7ff, 11, sg & 2); c[2] = r3d_unorm(w0 >> 22, 10, sg & 4); break;
    case 10: w0 = p[0] | (p[1] << 8);
             c[0] = r3d_unorm(w0 & 0xf, 4, sg & 1); c[1] = r3d_unorm((w0 >> 4) & 0xf, 4, sg & 2);
             c[2] = r3d_unorm((w0 >> 8) & 0xf, 4, sg & 4); c[3] = r3d_unorm(w0 >> 12, 4, sg & 8); break;
    case 11: w0 = p[0] | (p[1] << 8);
             c[0] = r3d_unorm(w0 & 0x1f, 5, sg & 1); c[1] = r3d_unorm((w0 >> 5) & 0x1f, 5, sg & 2);
             c[2] = r3d_unorm((w0 >> 10) & 0x1f, 5, sg & 4); c[3] = (float)(w0 >> 15); break;
    case 12:
    case 19:
      c[0] = r3d_unorm(p[0], 8, sg & 1); c[1] = r3d_unorm(p[1], 8, sg & 2);
      c[2] = r3d_unorm(p[2], 8, sg & 4); c[3] = r3d_unorm(p[3], 8, sg & 8);
      if ((t->fmt == 19) && t->yuv) {
        // AVYU444: U, Y, V, A
        float rgb[4];
        r3d_yuv_rgb(c[1], c[0], c[2], t->yuv == 1, rgb);
        rgb[3] = c[3];
        memcpy(c, rgb, sizeof(rgb));
        *is_rgba = true;
      }
      break;
    case 13: w0 = p[0] | (p[1] << 8) | ((Bit32u)p[2] << 16) | ((Bit32u)p[3] << 24);
             c[0] = r3d_unorm(w0 & 0x3ff, 10, sg & 1); c[1] = r3d_unorm((w0 >> 10) & 0x3ff, 10, sg & 2);
             c[2] = r3d_unorm((w0 >> 20) & 0x3ff, 10, sg & 4); c[3] = r3d_unorm(w0 >> 30, 2, sg & 8); break;
    case 14:
      c[0] = r3d_unorm(p[0] | (p[1] << 8), 16, sg & 1); c[1] = r3d_unorm(p[2] | (p[3] << 8), 16, sg & 2);
      c[2] = r3d_unorm(p[4] | (p[5] << 8), 16, sg & 4); c[3] = r3d_unorm(p[6] | (p[7] << 8), 16, sg & 8); break;
    case 18:
      c[0] = p[0] / 255.0f;
      if (t->yuv) {
        c[0] = c[1] = c[2] = r3d_clamp01((p[0] - 16.0f) * 1.164f / 255.0f);
        c[3] = 1.0f;
        *is_rgba = true;
      }
      break;
    case 20: case 21: {
      // packed 4:2:2 pairs: VYUY = V Y0 U Y1, YVYU = Y0 V Y1 U
      const Bit8u *q = p - (u_odd ? 2 : 0);
      float y, u, v;
      if (t->fmt == 20) { v = q[0] / 255.0f; u = q[2] / 255.0f; y = (u_odd ? q[3] : q[1]) / 255.0f; }
      else { y = (u_odd ? q[2] : q[0]) / 255.0f; v = q[1] / 255.0f; u = q[3] / 255.0f; }
      if (t->swap_yuv) { u = (float)(((int)(u * 255.0f) ^ 0x80) & 0xff) / 255.0f; v = (float)(((int)(v * 255.0f) ^ 0x80) & 0xff) / 255.0f; }
      if (t->yuv) {
        r3d_yuv_rgb(y, u, v, t->yuv == 1, c);
        *is_rgba = true;
      } else {
        c[0] = y; c[1] = u; c[2] = v; c[3] = 1.0f;
      }
      break;
    }
    case 22: c[0] = r3d_unorm(p[0] | (p[1] << 8), 16, true); break;
    case 23: c[0] = r3d_unorm(p[0] | (p[1] << 8), 16, true); c[1] = r3d_unorm(p[2] | (p[3] << 8), 16, true); break;
    case 24: c[0] = r3d_half_to_float(p[0] | (p[1] << 8)); break;
    case 25: c[0] = r3d_half_to_float(p[0] | (p[1] << 8)); c[1] = r3d_half_to_float(p[2] | (p[3] << 8)); break;
    case 26:
      c[0] = r3d_half_to_float(p[0] | (p[1] << 8)); c[1] = r3d_half_to_float(p[2] | (p[3] << 8));
      c[2] = r3d_half_to_float(p[4] | (p[5] << 8)); c[3] = r3d_half_to_float(p[6] | (p[7] << 8)); break;
    case 27: w0 = p[0] | (p[1] << 8) | ((Bit32u)p[2] << 16) | ((Bit32u)p[3] << 24); c[0] = r9800_u2f(w0); break;
    case 28:
      w0 = p[0] | (p[1] << 8) | ((Bit32u)p[2] << 16) | ((Bit32u)p[3] << 24);
      w1 = p[4] | (p[5] << 8) | ((Bit32u)p[6] << 16) | ((Bit32u)p[7] << 24);
      c[0] = r9800_u2f(w0); c[1] = r9800_u2f(w1); break;
    case 29:
      for (int k = 0; k < 4; k++) {
        w0 = p[k * 4] | (p[k * 4 + 1] << 8) | ((Bit32u)p[k * 4 + 2] << 16) | ((Bit32u)p[k * 4 + 3] << 24);
        c[k] = r9800_u2f(w0);
      }
      break;
    case 30:
      w0 = p[0] | (p[1] << 8) | ((Bit32u)p[2] << 16);
      c[0] = r3d_float24(w0);
      break;
    default:
      break;
  }
  if (t->gamma && (t->bpp <= 4) && !*is_rgba)
    for (int k = 0; k < 4; k++) c[k] = r3d_degamma(c[k]);
}

// Fetch texel (u, v) of one level / layer as RGBA
void bx_radeon9800_c::r3d_texel(const r9800_raster_state_t *rs, const r300_tex_desc_t *t, int level, int layer,
                                int u, int v, float *out)
{
  const Bit8u *base;
  Bit32u limit, off;
  float c[4];
  bool is_rgba = false;
  Bit8u tmp[16];

  if (t->stage_off != R9800_TEX_STAGE_NONE) {
    base = tex_stage.arena + t->stage_off;
    limit = t->total;
  } else {
    base = BX_R9800_THIS s.memory + t->vram_off;
    limit = vram_size - t->vram_off;
    if (limit > t->total) limit = t->total;
  }
  off = t->lvl_off[level] + (Bit32u)layer * t->lvl_layer[level];
  if (t->cmp) {
    Bit32u bs = (t->cmp == 1) ? 8 : 16;
    Bit32u boff = off + (Bit32u)(v >> 2) * t->lvl_pitch[level] + (Bit32u)(u >> 2) * bs;
    if (boff + bs > limit) {
      memset(tmp, 0, sizeof(tmp));
      base = tmp; boff = 0;
    }
    if (t->cmp == 4) {
      c[0] = r3d_ati2n_chan(base + boff, u, v);
      c[1] = r3d_ati2n_chan(base + boff + 8, u, v);
      c[2] = c[3] = 0.0f;
    } else {
      r3d_dxt_texel(base + boff, t->cmp, u, v, c);
    }
  } else {
    Bit32u bpp = (Bit32u)t->bpp;
    Bit32u toff;
    if (t->fmt == 20 || t->fmt == 21) {
      // 4:2:2: address the even texel of the pair, decode picks the odd
      toff = off + r300_tile_addr((Bit32u)(u & ~1), (Bit32u)v, t->lvl_pitch[level], 2, t->micro, t->macro) + ((u & 1) ? 2 : 0);
    } else {
      toff = off + r300_tile_addr((Bit32u)u, (Bit32u)v, t->lvl_pitch[level], bpp, t->micro, t->macro);
    }
    if ((t->fmt == 20) || (t->fmt == 21)) {
      // fetch the whole 4-byte pair; the decoder picks the odd texel
      Bit32u poff = toff - ((u & 1) ? 2 : 0);
      Bit8u pair[4];
      if (poff + 4 > limit) {
        memset(pair, 0, sizeof(pair));
      } else if (t->endian == 1) {
        pair[0] = base[poff + 1]; pair[1] = base[poff]; pair[2] = base[poff + 3]; pair[3] = base[poff + 2];
      } else if (t->endian == 2) {
        pair[0] = base[poff + 3]; pair[1] = base[poff + 2]; pair[2] = base[poff + 1]; pair[3] = base[poff];
      } else {
        memcpy(pair, base + poff, 4);
      }
      r3d_decode_texel(t, pair + ((u & 1) ? 2 : 0), u & 1, c, &is_rgba);
    } else {
      if (toff + bpp > limit) {
        memset(tmp, 0, sizeof(tmp));
        base = tmp;
        toff = 0;
      }
      if (t->endian == 1) {
        // 16-bit swap
        const Bit8u *s = base + toff;
        for (Bit32u k = 0; k + 1 < bpp; k += 2) { tmp[k] = s[k + 1]; tmp[k + 1] = s[k]; }
        if (bpp == 1) tmp[0] = s[0];
        r3d_decode_texel(t, tmp, 0, c, &is_rgba);
      } else if (t->endian == 2) {
        const Bit8u *s = base + toff;
        for (Bit32u k = 0; k + 3 < bpp; k += 4) { tmp[k] = s[k + 3]; tmp[k + 1] = s[k + 2]; tmp[k + 2] = s[k + 1]; tmp[k + 3] = s[k]; }
        if (bpp < 4) memcpy(tmp, s, bpp);
        r3d_decode_texel(t, tmp, 0, c, &is_rgba);
      } else {
        r3d_decode_texel(t, base + toff, 0, c, &is_rgba);
      }
    }
  }
  // chroma key: texels equal to TX_CHROMA_KEY become transparent black
  if (t->ck_mode) {
    Bit8u raw[16];
    float kc[4];
    bool krgba;
    WriteHostDWordToLittleEndian((Bit32u*)raw, t->chroma);
    memset(raw + 4, 0, 12);
    r3d_decode_texel(t, raw, 0, kc, &krgba);
    if ((fabs(kc[0] - c[0]) < 1.0f / 512.0f) && (fabs(kc[1] - c[1]) < 1.0f / 512.0f) &&
        (fabs(kc[2] - c[2]) < 1.0f / 512.0f) && (fabs(kc[3] - c[3]) < 1.0f / 512.0f)) {
      out[0] = out[1] = out[2] = out[3] = 0.0f;
      return;
    }
  }
  if (is_rgba) {
    memcpy(out, c, 4 * sizeof(float));
    return;
  }
  // swizzle: SEL_ALPHA, SEL_RED, SEL_GREEN, SEL_BLUE
  float sw[4];
  for (int k = 0; k < 4; k++) {
    int sel = t->sel[k];
    sw[k] = (sel < 4) ? c[sel] : ((sel == 4) ? 0.0f : 1.0f);
  }
  out[0] = sw[1]; out[1] = sw[2]; out[2] = sw[3]; out[3] = sw[0];
  UNUSED(rs);
}

// Coordinate wrap; returns -1 for the border colour
static BX_CPP_INLINE int r3d_wrap(int c, int n, int mode)
{
  int m;
  switch (mode) {
    case 0: return ((c % n) + n) % n;
    case 1:
      m = ((c % (2 * n)) + 2 * n) % (2 * n);
      return (m < n) ? m : (2 * n - 1 - m);
    case 2: return (c < 0) ? 0 : (c >= n) ? n - 1 : c;
    case 3:
      if (c < -n) c = -n;
      if (c >= n) c = n - 1;
      return (c < 0) ? (-c - 1) : c;
    case 4: case 6:
      return ((c < 0) || (c >= n)) ? -1 : c;
    default:
      if (c < 0) c = -c - 1;
      return (c >= n) ? -1 : c;
  }
}

// Sample one level (nearest or bilinear)
void bx_radeon9800_c::r3d_tex_level(const r9800_raster_state_t *rs, const r300_tex_desc_t *t, int level, int layer,
                                    float s, float tt, int linear, float *out)
{
  int lw = (int)t->lvl_w[level], lh = (int)t->lvl_h[level];
  float border[4];
  bool need_border = false;
  float fu = s * (float)lw, fv = tt * (float)lh;

  if (!linear) {
    int u = r3d_wrap((int)floor(fu), lw, t->clamp_s);
    int v = r3d_wrap((int)floor(fv), lh, t->clamp_t);
    if ((u < 0) || (v < 0)) {
      Bit8u raw[16];
      bool rgba;
      WriteHostDWordToLittleEndian((Bit32u*)raw, t->border);
      memset(raw + 4, 0, 12);
      r3d_decode_texel(t, raw, 0, out, &rgba);
      if (!rgba) {
        float sw[4], c[4];
        memcpy(c, out, sizeof(c));
        for (int k = 0; k < 4; k++) sw[k] = (t->sel[k] < 4) ? c[t->sel[k]] : ((t->sel[k] == 4) ? 0.0f : 1.0f);
        out[0] = sw[1]; out[1] = sw[2]; out[2] = sw[3]; out[3] = sw[0];
      }
      return;
    }
    r3d_texel(rs, t, level, layer, u, v, out);
    return;
  }
  fu -= 0.5f; fv -= 0.5f;
  int u0 = (int)floor(fu), v0 = (int)floor(fv);
  float wu = fu - (float)u0, wv = fv - (float)v0;
  int uu[2], vv[2];
  uu[0] = r3d_wrap(u0, lw, t->clamp_s); uu[1] = r3d_wrap(u0 + 1, lw, t->clamp_s);
  vv[0] = r3d_wrap(v0, lh, t->clamp_t); vv[1] = r3d_wrap(v0 + 1, lh, t->clamp_t);
  if ((uu[0] < 0) || (uu[1] < 0) || (vv[0] < 0) || (vv[1] < 0)) {
    Bit8u raw[16];
    bool rgba;
    WriteHostDWordToLittleEndian((Bit32u*)raw, t->border);
    memset(raw + 4, 0, 12);
    r3d_decode_texel(t, raw, 0, border, &rgba);
    if (!rgba) {
      float sw[4];
      for (int k = 0; k < 4; k++) sw[k] = (t->sel[k] < 4) ? border[t->sel[k]] : ((t->sel[k] == 4) ? 0.0f : 1.0f);
      border[0] = sw[1]; border[1] = sw[2]; border[2] = sw[3]; border[3] = sw[0];
    }
    need_border = true;
  }
  float c[4][4];
  for (int j = 0; j < 2; j++) {
    for (int i = 0; i < 2; i++) {
      if ((uu[i] < 0) || (vv[j] < 0)) {
        memcpy(c[j * 2 + i], border, sizeof(border));
      } else {
        r3d_texel(rs, t, level, layer, uu[i], vv[j], c[j * 2 + i]);
      }
    }
  }
  UNUSED(need_border);
  for (int k = 0; k < 4; k++) {
    float top = c[0][k] + (c[1][k] - c[0][k]) * wu;
    float bot = c[2][k] + (c[3][k] - c[2][k]) * wu;
    out[k] = top + (bot - top) * wv;
  }
}

// Full sampler: projection, cube face selection, LOD from the screen
// space derivatives, mip filtering, volume slices
void bx_radeon9800_c::r3d_tex_sample(const r9800_raster_state_t *rs, int unit, const float *coord,
                                     const float *dsdx, const float *dsdy, float lod_bias, int proj, float *out)
{
  const r300_tex_desc_t *t = &rs->tex[unit & 15];
  float s = coord[0], tt = coord[1], r = coord[2], q = coord[3];
  float ds[3] = { dsdx[0], dsdx[1], dsdx[2] };
  float dt[3] = { dsdy[0], dsdy[1], dsdy[2] };
  int layer = 0;
  float layer_f = 0.0f;

  if (!t->en) {
    out[0] = out[1] = out[2] = 0.0f;
    out[3] = 1.0f;
    return;
  }
  if (proj || t->projected) {
    float iq = (q != 0.0f) ? 1.0f / q : 0.0f;
    // d(s/q) ~= (ds*q - s*dq) / q^2
    for (int k = 0; k < 3; k++) {
      ds[k] = (ds[k] * q - coord[k] * dsdx[3]) * iq * iq;
      dt[k] = (dt[k] * q - coord[k] * dsdy[3]) * iq * iq;
    }
    s *= iq; tt *= iq; r *= iq;
  }
  if (t->coord_type == 2) {
    // cube map: major axis selects the face
    float ax = (float)fabs(s), ay = (float)fabs(tt), az = (float)fabs(r);
    float ma, sc, tc;
    float dsc[2], dtc[2];
    if ((ax >= ay) && (ax >= az)) {
      ma = ax;
      if (s >= 0) { layer = 0; sc = -r; tc = -tt; dsc[0] = -ds[2]; dsc[1] = -dt[2]; dtc[0] = -ds[1]; dtc[1] = -dt[1]; }
      else        { layer = 1; sc = r;  tc = -tt; dsc[0] = ds[2];  dsc[1] = dt[2];  dtc[0] = -ds[1]; dtc[1] = -dt[1]; }
    } else if (ay >= az) {
      ma = ay;
      if (tt >= 0) { layer = 2; sc = s; tc = r;  dsc[0] = ds[0]; dsc[1] = dt[0]; dtc[0] = ds[2];  dtc[1] = dt[2]; }
      else         { layer = 3; sc = s; tc = -r; dsc[0] = ds[0]; dsc[1] = dt[0]; dtc[0] = -ds[2]; dtc[1] = -dt[2]; }
    } else {
      ma = az;
      if (r >= 0) { layer = 4; sc = s;  tc = -tt; dsc[0] = ds[0];  dsc[1] = dt[0];  dtc[0] = -ds[1]; dtc[1] = -dt[1]; }
      else        { layer = 5; sc = -s; tc = -tt; dsc[0] = -ds[0]; dsc[1] = -dt[0]; dtc[0] = -ds[1]; dtc[1] = -dt[1]; }
    }
    if (ma == 0.0f) ma = 1e-20f;
    float ima = 1.0f / ma;
    s = 0.5f * (sc * ima + 1.0f);
    tt = 0.5f * (tc * ima + 1.0f);
    ds[0] = 0.5f * dsc[0] * ima; dt[0] = 0.5f * dsc[1] * ima;
    ds[1] = 0.5f * dtc[0] * ima; dt[1] = 0.5f * dtc[1] * ima;
  } else if (t->coord_type == 1) {
    int depth = 1 << t->depth_log2;
    layer_f = r * (float)depth;
    // wrap the r coordinate
    float fr = layer_f;
    if (t->clamp_r == 0) { fr = fr - (float)floor(fr / depth) * depth; }
    layer_f = fr;
  }
  // level of detail
  float w = (float)t->lvl_w[0], h = (float)t->lvl_h[0];
  float dux = ds[0] * w, dvx = ds[1] * h, duy = dt[0] * w, dvy = dt[1] * h;
  float rho2x = dux * dux + dvx * dvx, rho2y = duy * duy + dvy * dvy;
  float rho2 = rho2x > rho2y ? rho2x : rho2y;
  float lod = (rho2 > 0.0f) ? 0.5f * (float)(log(rho2) / log(2.0)) : -16.0f;
  lod += t->lod_bias + lod_bias;
  bool magnify = lod <= 0.0f;
  int filter = magnify ? t->mag : t->min;
  int linear = (filter == 2) ? 1 : 0;
  int lmin = t->max_mip, lmax = t->levels - 1;
  if (lmin > lmax) lmin = lmax;
  float lvl = lod;
  if (lvl < (float)lmin) lvl = (float)lmin;
  if (lvl > (float)lmax) lvl = (float)lmax;

  int l0, l1;
  float lf = 0.0f;
  if (magnify || (t->mip == 0)) {
    l0 = l1 = lmin;
    if (!magnify && (t->mip == 0)) l0 = l1 = lmin;
  } else if (t->mip == 1) {
    l0 = l1 = (int)floor(lvl + 0.5f);
  } else {
    l0 = (int)floor(lvl);
    l1 = l0 + 1;
    if (l1 > lmax) l1 = lmax;
    lf = lvl - (float)l0;
  }
  if (l0 < lmin) l0 = lmin;
  if (l1 < lmin) l1 = lmin;
  if (l0 > lmax) l0 = lmax;
  if (l1 > lmax) l1 = lmax;

  float ca[4], cb[4];
  if (t->coord_type == 1) {
    // volume: filter between slices when VOL_FILTER is linear
    int vol_linear = (t->vol == 2) || ((t->vol == 0) && linear);
    int depth0 = (int)t->lvl_d[l0];
    float lp = layer_f * ((float)depth0 / (float)(1 << t->depth_log2));
    int z0 = (int)floor(lp - (vol_linear ? 0.5f : 0.0f));
    float zf = vol_linear ? (lp - 0.5f - (float)z0) : 0.0f;
    int za = r3d_wrap(z0, depth0, t->clamp_r == 0 ? 0 : 2);
    int zb = r3d_wrap(z0 + 1, depth0, t->clamp_r == 0 ? 0 : 2);
    if (za < 0) za = 0;
    if (zb < 0) zb = 0;
    r3d_tex_level(rs, t, l0, za, s, tt, linear, ca);
    if (vol_linear) {
      r3d_tex_level(rs, t, l0, zb, s, tt, linear, cb);
      for (int k = 0; k < 4; k++) ca[k] += (cb[k] - ca[k]) * zf;
    }
    if (l1 != l0) {
      int depth1 = (int)t->lvl_d[l1];
      float lp1 = layer_f * ((float)depth1 / (float)(1 << t->depth_log2));
      int z1 = r3d_wrap((int)floor(lp1), depth1, t->clamp_r == 0 ? 0 : 2);
      if (z1 < 0) z1 = 0;
      r3d_tex_level(rs, t, l1, z1, s, tt, linear, cb);
      for (int k = 0; k < 4; k++) ca[k] += (cb[k] - ca[k]) * lf;
    }
  } else {
    r3d_tex_level(rs, t, l0, layer, s, tt, linear, ca);
    if (l1 != l0) {
      r3d_tex_level(rs, t, l1, layer, s, tt, linear, cb);
      for (int k = 0; k < 4; k++) ca[k] += (cb[k] - ca[k]) * lf;
    }
  }
  memcpy(out, ca, sizeof(ca));
}

// =====================================================================
// Fragment shader (US) interpreter
// =====================================================================

static BX_CPP_INLINE float r3d_mod(float v, int mod)
{
  switch (mod & 3) {
    case 1: return -v;
    case 2: return (float)fabs(v);
    case 3: return -(float)fabs(v);
    default: return v;
  }
}

static BX_CPP_INLINE float r3d_omod(float v, int omod)
{
  switch (omod & 7) {
    case 1: return v * 2.0f;
    case 2: return v * 4.0f;
    case 3: return v * 8.0f;
    case 4: return v * 0.5f;
    case 5: return v * 0.25f;
    case 6: return v * 0.125f;
    default: return v;
  }
}

// Resolve one RGB source argument (3 components) of an ALU instruction
static void r3d_fp_rgb_arg(const r300_alu_inst_t *ai, int k, const float (*temps)[4], const float (*konst)[4],
                           const float *srcp, float *out)
{
  int sel = ai->rgb.sel[k];
  int mod = ai->rgb.mod[k];
  const float *v;
  int src;
  float tmp[4];

  if (sel < 12) {
    // src0/1/2 .rgb .rrr .ggg .bbb
    src = ai->rgb.src[sel / 4];
    v = (src >= 32) ? konst[src & 31] : temps[src & 31];
    switch (sel & 3) {
      case 0: tmp[0] = v[0]; tmp[1] = v[1]; tmp[2] = v[2]; break;
      case 1: tmp[0] = tmp[1] = tmp[2] = v[0]; break;
      case 2: tmp[0] = tmp[1] = tmp[2] = v[1]; break;
      default: tmp[0] = tmp[1] = tmp[2] = v[2]; break;
    }
  } else if (sel < 15) {
    // src.aaa: the alpha channel of the alpha-side sources
    src = ai->alpha.src[sel - 12];
    v = (src >= 32) ? konst[src & 31] : temps[src & 31];
    tmp[0] = tmp[1] = tmp[2] = v[3];
  } else if (sel < 20) {
    switch (sel) {
      case 15: tmp[0] = srcp[0]; tmp[1] = srcp[1]; tmp[2] = srcp[2]; break;
      case 16: tmp[0] = tmp[1] = tmp[2] = srcp[0]; break;
      case 17: tmp[0] = tmp[1] = tmp[2] = srcp[1]; break;
      case 18: tmp[0] = tmp[1] = tmp[2] = srcp[2]; break;
      default: tmp[0] = tmp[1] = tmp[2] = srcp[3]; break;
    }
  } else if (sel < 23) {
    float c = (sel == 20) ? 0.0f : (sel == 21) ? 1.0f : 0.5f;
    tmp[0] = tmp[1] = tmp[2] = c;
  } else if (sel < 26) {
    src = ai->rgb.src[sel - 23];
    v = (src >= 32) ? konst[src & 31] : temps[src & 31];
    tmp[0] = v[1]; tmp[1] = v[2]; tmp[2] = v[0];      // .gbr
  } else if (sel < 29) {
    src = ai->rgb.src[sel - 26];
    v = (src >= 32) ? konst[src & 31] : temps[src & 31];
    tmp[0] = v[2]; tmp[1] = v[0]; tmp[2] = v[1];      // .brg
  } else {
    src = ai->rgb.src[sel - 29];
    v = (src >= 32) ? konst[src & 31] : temps[src & 31];
    const float *va = v;
    int asrc = ai->alpha.src[sel - 29];
    va = (asrc >= 32) ? konst[asrc & 31] : temps[asrc & 31];
    tmp[0] = va[3]; tmp[1] = v[2]; tmp[2] = v[1];     // .abg
  }
  out[0] = r3d_mod(tmp[0], mod);
  out[1] = r3d_mod(tmp[1], mod);
  out[2] = r3d_mod(tmp[2], mod);
}

// Resolve one alpha source argument (scalar)
static float r3d_fp_a_arg(const r300_alu_inst_t *ai, int k, const float (*temps)[4], const float (*konst)[4],
                          const float *srcp)
{
  int sel = ai->alpha.sel[k];
  int mod = ai->alpha.mod[k];
  const float *v;
  int src;
  float r;

  if (sel < 9) {
    // src0/1/2 .r .g .b from the RGB-side sources
    src = ai->rgb.src[sel / 3];
    v = (src >= 32) ? konst[src & 31] : temps[src & 31];
    r = v[sel % 3];
  } else if (sel < 12) {
    src = ai->alpha.src[sel - 9];
    v = (src >= 32) ? konst[src & 31] : temps[src & 31];
    r = v[3];
  } else if (sel < 16) {
    r = srcp[sel - 12];
  } else if (sel == 16) {
    r = 0.0f;
  } else if (sel == 17) {
    r = 1.0f;
  } else {
    r = 0.5f;
  }
  return r3d_mod(r, mod);
}

static BX_CPP_INLINE float r3d_srcp(int op, float a0, float a1)
{
  switch (op & 3) {
    case 0: return 1.0f - 2.0f * a0;
    case 1: return a1 - a0;
    case 2: return a1 + a0;
    default: return 1.0f - a0;
  }
}

bool bx_radeon9800_c::r3d_run_fp(const r9800_raster_state_t *rs, float temps[32][4], const float tc_ddx[32][4],
                                 const float tc_ddy[32][4], Bit32u rs_written, float out[4][4], float *depth,
                                 int *depth_out)
{
  const r300_fp_t *fp = &rs->fp;
  float ddx[32][4], ddy[32][4];
  Bit32u valid = rs_written;

  memcpy(ddx, tc_ddx, sizeof(ddx));
  memcpy(ddy, tc_ddy, sizeof(ddy));
  for (int t = 0; t < 4; t++) {
    out[t][0] = out[t][1] = out[t][2] = 0.0f;
    out[t][3] = 1.0f;
  }
  *depth_out = 0;
  for (int r = 0; r < 32; r++) {
    if (!(valid & (1u << r))) {
      temps[r][0] = temps[r][1] = temps[r][2] = 0.0f;
      temps[r][3] = 1.0f;
      ddx[r][0] = ddx[r][1] = ddx[r][2] = ddx[r][3] = 0.0f;
      ddy[r][0] = ddy[r][1] = ddy[r][2] = ddy[r][3] = 0.0f;
    }
  }

  for (int n = 0; n < fp->nodes; n++) {
    // texture instructions of the node
    for (int i = 0; i < fp->node[n].tex_cnt; i++) {
      const r300_tex_inst_t *ti = &fp->tex[(fp->node[n].tex_start + i) & (R9800_US_TEX_INSTS - 1)];
      const float *c = temps[ti->src & 31];
      float res[4];
      switch (ti->op) {
        case 1:  // LD
          r3d_tex_sample(rs, ti->tex_id, c, ddx[ti->src & 31], ddy[ti->src & 31], 0.0f, 0, res);
          break;
        case 2:  // TEXKILL
          if ((c[0] < 0.0f) || (c[1] < 0.0f) || (c[2] < 0.0f) || (c[3] < 0.0f))
            return false;
          continue;
        case 3:  // PROJ
          r3d_tex_sample(rs, ti->tex_id, c, ddx[ti->src & 31], ddy[ti->src & 31], 0.0f, 1, res);
          break;
        case 4:  // LODBIAS
          r3d_tex_sample(rs, ti->tex_id, c, ddx[ti->src & 31], ddy[ti->src & 31], c[3], 0, res);
          break;
        default:
          continue;
      }
      memcpy(temps[ti->dst & 31], res, sizeof(res));
      // derivatives of the sampled colour: use the coordinate derivatives scaled by the map size
      for (int k = 0; k < 4; k++) { ddx[ti->dst & 31][k] = 0.0f; ddy[ti->dst & 31][k] = 0.0f; }
      valid |= 1u << (ti->dst & 31);
    }
    // ALU instructions of the node
    for (int i = 0; i < fp->node[n].alu_cnt; i++) {
      const r300_alu_inst_t *ai = &fp->alu[(fp->node[n].alu_start + i) & (R9800_US_ALU_INSTS - 1)];
      float srcp[4];
      float a[3], b[3], c[3];
      float aa, ab, ac;
      float rgb[3], alpha, dot = 0.0f;
      // presubtract values for both halves
      {
        int s0 = ai->rgb.src[0], s1 = ai->rgb.src[1];
        const float *v0 = (s0 >= 32) ? fp->konst[s0 & 31] : temps[s0 & 31];
        const float *v1 = (s1 >= 32) ? fp->konst[s1 & 31] : temps[s1 & 31];
        for (int k = 0; k < 3; k++) srcp[k] = r3d_srcp(ai->rgb.srcp_op, v0[k], v1[k]);
        int t0 = ai->alpha.src[0], t1 = ai->alpha.src[1];
        const float *w0 = (t0 >= 32) ? fp->konst[t0 & 31] : temps[t0 & 31];
        const float *w1 = (t1 >= 32) ? fp->konst[t1 & 31] : temps[t1 & 31];
        srcp[3] = r3d_srcp(ai->alpha.srcp_op, w0[3], w1[3]);
      }
      r3d_fp_rgb_arg(ai, 0, temps, fp->konst, srcp, a);
      r3d_fp_rgb_arg(ai, 1, temps, fp->konst, srcp, b);
      r3d_fp_rgb_arg(ai, 2, temps, fp->konst, srcp, c);
      aa = r3d_fp_a_arg(ai, 0, temps, fp->konst, srcp);
      ab = r3d_fp_a_arg(ai, 1, temps, fp->konst, srcp);
      ac = r3d_fp_a_arg(ai, 2, temps, fp->konst, srcp);

      // alpha half first: its scalar result may feed the RGB SOP op
      switch (ai->alpha.op) {
        case 0: alpha = aa * ab + ac; break;
        case 1:
          dot = a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
          if (ai->rgb.op == 2) dot += aa * ab;
          alpha = dot;
          break;
        case 2: alpha = (aa < ab) ? aa : ab; break;
        case 3: alpha = (aa > ab) ? aa : ab; break;
        case 5: alpha = (ac > 0.5f) ? aa : ab; break;
        case 6: alpha = (ac >= 0.0f) ? aa : ab; break;
        case 7: alpha = aa - (float)floor(aa); break;
        case 8: alpha = (float)pow(2.0, aa); break;
        case 9: alpha = (aa > 0.0f) ? (float)(log(aa) / log(2.0)) : -1e30f; break;
        case 10: alpha = (aa != 0.0f) ? 1.0f / aa : 1e30f; break;
        case 11: {
          float t = (float)fabs(aa);
          alpha = (t != 0.0f) ? 1.0f / (float)sqrt(t) : 1e30f;
          break;
        }
        case 12: alpha = (float)sin(aa * 6.283185307179586); break;
        case 13: alpha = (float)cos(aa * 6.283185307179586); break;
        default: alpha = aa; break;
      }
      switch (ai->rgb.op) {
        case 0:
          for (int k = 0; k < 3; k++) rgb[k] = a[k] * b[k] + c[k];
          break;
        case 1:
          dot = a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
          rgb[0] = rgb[1] = rgb[2] = dot;
          break;
        case 2:
          dot = a[0] * b[0] + a[1] * b[1] + a[2] * b[2] + aa * ab;
          rgb[0] = rgb[1] = rgb[2] = dot;
          break;
        case 3:
          dot = a[0] * b[0] + a[1] * b[1] + c[2];
          rgb[0] = rgb[1] = rgb[2] = dot;
          break;
        case 4:
          for (int k = 0; k < 3; k++) rgb[k] = (a[k] < b[k]) ? a[k] : b[k];
          break;
        case 5:
          for (int k = 0; k < 3; k++) rgb[k] = (a[k] > b[k]) ? a[k] : b[k];
          break;
        case 7:
          for (int k = 0; k < 3; k++) rgb[k] = (c[k] > 0.5f) ? a[k] : b[k];
          break;
        case 8:
          for (int k = 0; k < 3; k++) rgb[k] = (c[k] >= 0.0f) ? a[k] : b[k];
          break;
        case 9:
          for (int k = 0; k < 3; k++) rgb[k] = a[k] - (float)floor(a[k]);
          break;
        case 10:
          rgb[0] = rgb[1] = rgb[2] = alpha;
          break;
        default:
          for (int k = 0; k < 3; k++) rgb[k] = a[k];
          break;
      }
      // output modifiers and clamping
      for (int k = 0; k < 3; k++) {
        rgb[k] = r3d_omod(rgb[k], ai->rgb.omod);
        if (ai->rgb.clamp) rgb[k] = r3d_clamp01(rgb[k]);
      }
      alpha = r3d_omod(alpha, ai->alpha.omod);
      if (ai->alpha.clamp) alpha = r3d_clamp01(alpha);
      // write back: temporaries
      if (ai->rgb.wmask) {
        float *d = temps[ai->rgb.dst & 31];
        if (ai->rgb.wmask & 1) d[0] = rgb[0];
        if (ai->rgb.wmask & 2) d[1] = rgb[1];
        if (ai->rgb.wmask & 4) d[2] = rgb[2];
        ddx[ai->rgb.dst & 31][0] = ddx[ai->rgb.dst & 31][1] = ddx[ai->rgb.dst & 31][2] = 0.0f;
        ddy[ai->rgb.dst & 31][0] = ddy[ai->rgb.dst & 31][1] = ddy[ai->rgb.dst & 31][2] = 0.0f;
        valid |= 1u << (ai->rgb.dst & 31);
      }
      if (ai->alpha.wmask) {
        temps[ai->alpha.dst & 31][3] = alpha;
        ddx[ai->alpha.dst & 31][3] = ddy[ai->alpha.dst & 31][3] = 0.0f;
        valid |= 1u << (ai->alpha.dst & 31);
      }
      // write back: outputs
      if (ai->rgb.omask) {
        float *o = out[ai->rgb.target & 3];
        if (ai->rgb.omask & 1) o[0] = rgb[0];
        if (ai->rgb.omask & 2) o[1] = rgb[1];
        if (ai->rgb.omask & 4) o[2] = rgb[2];
      }
      if (ai->alpha.omask)
        out[ai->alpha.target & 3][3] = alpha;
      if (ai->alpha_w_out) {
        *depth = alpha;
        *depth_out = 1;
      }
    }
  }
  return true;
}

// =====================================================================
// Pixel back end: depth / stencil, colour buffer
// =====================================================================

void bx_radeon9800_c::r3d_mark_dirty(Bit32u addr, Bit32u len)
{
  vram_dirty(addr & vram_mask, len);
}

static BX_CPP_INLINE bool r3d_cmp_fn(int fn, Bit32u a, Bit32u b)
{
  switch (fn & 7) {
    case 0: return false;
    case 1: return a < b;
    case 2: return a == b;
    case 3: return a <= b;
    case 4: return a > b;
    case 5: return a != b;
    case 6: return a >= b;
    default: return true;
  }
}

static BX_CPP_INLINE bool r3d_cmp_fnf(int fn, float a, float b)
{
  switch (fn & 7) {
    case 0: return false;
    case 1: return a < b;
    case 2: return a == b;
    case 3: return a <= b;
    case 4: return a > b;
    case 5: return a != b;
    case 6: return a >= b;
    default: return true;
  }
}

static BX_CPP_INLINE Bit32u r3d_sten_op(int op, Bit32u s, Bit32u ref)
{
  switch (op & 7) {
    case 1: return 0;
    case 2: return ref;
    case 3: return (s < 255) ? s + 1 : 255;
    case 4: return (s > 0) ? s - 1 : 0;
    case 5: return (~s) & 0xff;
    case 6: return (s + 1) & 0xff;
    case 7: return (s - 1) & 0xff;
    default: return s;
  }
}

// 16-bit 13E3 depth: 13-bit mantissa, 3-bit exponent (more precision near 0)
static BX_CPP_INLINE Bit32u r3d_z_13e3_enc(float z)
{
  if (z <= 0.0f) return 0;
  if (z >= 1.0f) return 0x1fff;
  int e = 0;
  float m = z;
  while ((e < 7) && (m < 0.5f)) { m *= 2.0f; e++; }
  Bit32u mi = (Bit32u)(m * 8192.0f + 0.5f);
  if (mi > 0x1fff) mi = 0x1fff;
  return (mi & 0x1fff) | ((Bit32u)e << 13);
}

static BX_CPP_INLINE float r3d_z_13e3_dec(Bit32u v)
{
  int e = (v >> 13) & 7;
  float m = (float)(v & 0x1fff) / 8192.0f;
  return m / (float)(1 << e);
}

// Byte offset of a Z buffer sample
static BX_CPP_INLINE Bit32u r3d_zb_addr(const r300_draw_state_t *d, int x, int y, int sample)
{
  Bit32u px = (Bit32u)(x * d->aa_samples + sample) + (Bit32u)d->zx_off;
  Bit32u py = (Bit32u)y + (Bit32u)d->zy_off;
  return d->zb_vram + r300_tile_addr(px, py, d->zb_pitch_px * (Bit32u)d->zb_bpp, (Bit32u)d->zb_bpp,
                                     d->zb_micro, d->zb_macro);
}

// ZMASK fast-clear bookkeeping: one bit per 4x4 block, row major
static BX_CPP_INLINE bool r3d_zmask_index(const r300_draw_state_t *d, int x, int y, Bit32u *word, Bit32u *bit)
{
  Bit32u bpr = (d->zb_pitch_px * (Bit32u)d->aa_samples + 3u) >> 2;
  Bit32u px = (Bit32u)(x * d->aa_samples) + (Bit32u)d->zx_off;
  Bit32u py = (Bit32u)y + (Bit32u)d->zy_off;
  Bit32u pitch = d->zmask_pitch ? (d->zmask_pitch & 0xfff) : bpr;
  if (!pitch) return false;
  Bit32u idx = (py >> 2) * pitch + (px >> 2) + (d->zmask_off & 0xfffff);
  *word = idx >> 5;
  *bit = idx & 31;
  return *word < R9800_ZMASK_RAM_DWORDS;
}

// Depth + stencil test and update for one sample. Returns whether the
// sample passed (colour may be written); *z_passed is set for the ZPASS
// counter.
bool bx_radeon9800_c::r3d_zs_test(const r9800_raster_state_t *rs, int x, int y, int sample, float z, int front,
                                  bool *z_passed)
{
  const r300_draw_state_t *d = &rs->d;
  Bit8u *mem = BX_R9800_THIS s.memory;
  Bit32u addr, cur, newv;
  Bit32u zi_new = 0, zi_cur = 0, zmax = 0;
  Bit32u sten_cur = 0, sten_new = 0;
  bool zpass = true, spass = true;
  bool fast = false;
  Bit32u zw = 0, zbit = 0;

  *z_passed = false;
  if (!d->zb_ok || (!d->z_en && !d->sten_en))
    return true;
  addr = r3d_zb_addr(d, x, y, sample);
  if (addr + d->zb_bpp > vram_size)
    return true;

  // fast clear state of the enclosing block
  if ((d->fast_fill || d->rd_comp) && r3d_zmask_index(d, x, y, &zw, &zbit))
    fast = (zmask_ram[zw] >> zbit) & 1;

  if (d->zb_fmt == 2) {
    cur = fast ? d->z_clear : ReadHostDWordFromLittleEndian((Bit32u*)(mem + addr));
    zi_cur = cur & 0xffffff;
    sten_cur = cur >> 24;
    zmax = 0xffffff;
    zi_new = (Bit32u)(r3d_clamp01(z) * 16777215.0f + 0.5f);
  } else {
    cur = fast ? (d->z_clear & 0xffff) : ReadHostWordFromLittleEndian((Bit16u*)(mem + addr));
    if (d->zb_fmt == 1) {
      zi_new = r3d_z_13e3_enc(r3d_clamp01(z));
      // compare in linear space
      zi_cur = (Bit32u)(r3d_z_13e3_dec(cur) * 65535.0f + 0.5f);
      zmax = 0xffff;
      Bit32u lin_new = (Bit32u)(r3d_clamp01(z) * 65535.0f + 0.5f);
      if (d->z_en)
        zpass = r3d_cmp_fn(d->z_fn, lin_new, zi_cur);
    } else {
      zi_cur = cur & 0xffff;
      zmax = 0xffff;
      zi_new = (Bit32u)(r3d_clamp01(z) * 65535.0f + 0.5f);
    }
  }
  if (d->z_en && (d->zb_fmt != 1))
    zpass = r3d_cmp_fn(d->z_fn, zi_new, zi_cur);

  if (d->sten_en && (d->zb_fmt == 2)) {
    int sfn = front ? d->sfn : d->sfn_bf;
    spass = r3d_cmp_fn(sfn, d->sref & d->smask, sten_cur & d->smask);
    int op;
    if (!spass) op = front ? d->sfail : d->sfail_bf;
    else if (!zpass) op = front ? d->szfail : d->szfail_bf;
    else op = front ? d->szpass : d->szpass_bf;
    sten_new = r3d_sten_op(op, sten_cur, d->sref);
    sten_new = (sten_cur & ~d->swmask) | (sten_new & d->swmask);
  } else {
    sten_new = sten_cur;
  }

  bool write_z = d->z_en && d->z_wr && zpass && spass;
  bool write_s = d->sten_en && (d->zb_fmt == 2) && (sten_new != sten_cur);
  if (write_z || write_s) {
    if (fast) {
      // decompress the block first (block may straddle worker rows)
      BX_LOCK(zmask_mutex);
      if ((zmask_ram[zw] >> zbit) & 1) {
        int bx0 = (x * d->aa_samples) & ~3, by0 = y & ~3;
        for (int yy = 0; yy < 4; yy++) {
          for (int xx = 0; xx < 4; xx++) {
            Bit32u a = r3d_zb_addr(d, (bx0 + xx) / d->aa_samples, by0 + yy, (bx0 + xx) % d->aa_samples);
            if (a + d->zb_bpp > vram_size) continue;
            if (d->zb_bpp == 4) WriteHostDWordToLittleEndian((Bit32u*)(mem + a), d->z_clear);
            else WriteHostWordToLittleEndian((Bit16u*)(mem + a), (Bit16u)d->z_clear);
          }
        }
        zmask_ram[zw] &= ~(1u << zbit);
      }
      BX_UNLOCK(zmask_mutex);
    }
    if (d->zb_fmt == 2) {
      newv = (write_z ? zi_new : zi_cur) | ((write_s ? sten_new : sten_cur) << 24);
      WriteHostDWordToLittleEndian((Bit32u*)(mem + addr), newv);
    } else {
      WriteHostWordToLittleEndian((Bit16u*)(mem + addr), (Bit16u)(write_z ? zi_new : cur));
    }
    UNUSED(zmax);
  }
  *z_passed = zpass && spass;
  return zpass && spass;
}

// Colour buffer pixel decode / encode
static void r3d_cb_decode(int fmt, const Bit8u *p, float *c)
{
  Bit32u v;
  switch (fmt) {
    case 3:  // ARGB1555
      v = p[0] | (p[1] << 8);
      c[0] = ((v >> 10) & 0x1f) / 31.0f; c[1] = ((v >> 5) & 0x1f) / 31.0f; c[2] = (v & 0x1f) / 31.0f;
      c[3] = (float)(v >> 15);
      break;
    case 4:  // RGB565
      v = p[0] | (p[1] << 8);
      c[0] = (v >> 11) / 31.0f; c[1] = ((v >> 5) & 0x3f) / 63.0f; c[2] = (v & 0x1f) / 31.0f; c[3] = 1.0f;
      break;
    case 6:  // ARGB8888
      c[0] = p[2] / 255.0f; c[1] = p[1] / 255.0f; c[2] = p[0] / 255.0f; c[3] = p[3] / 255.0f;
      break;
    case 7:  // ARGB32323232 float
      c[2] = r9800_u2f(p[0] | (p[1] << 8) | ((Bit32u)p[2] << 16) | ((Bit32u)p[3] << 24));
      c[1] = r9800_u2f(p[4] | (p[5] << 8) | ((Bit32u)p[6] << 16) | ((Bit32u)p[7] << 24));
      c[0] = r9800_u2f(p[8] | (p[9] << 8) | ((Bit32u)p[10] << 16) | ((Bit32u)p[11] << 24));
      c[3] = r9800_u2f(p[12] | (p[13] << 8) | ((Bit32u)p[14] << 16) | ((Bit32u)p[15] << 24));
      break;
    case 9:  // I8
      c[0] = c[1] = c[2] = p[0] / 255.0f; c[3] = 1.0f;
      break;
    case 10: // ARGB16161616
      c[2] = (p[0] | (p[1] << 8)) / 65535.0f; c[1] = (p[2] | (p[3] << 8)) / 65535.0f;
      c[0] = (p[4] | (p[5] << 8)) / 65535.0f; c[3] = (p[6] | (p[7] << 8)) / 65535.0f;
      break;
    case 11: case 12: { // VYUY / YVYU: treat the pair as luma + shared chroma
      float y = ((fmt == 11) ? p[1] : p[0]) / 255.0f;
      float u = ((fmt == 11) ? p[0] : p[1]) / 255.0f;
      float rgb[4];
      r3d_yuv_rgb(y, u, u, 1, rgb);
      c[0] = rgb[0]; c[1] = rgb[1]; c[2] = rgb[2]; c[3] = 1.0f;
      break;
    }
    case 13: // UV88
      c[0] = p[0] / 255.0f; c[1] = p[1] / 255.0f; c[2] = 0.0f; c[3] = 1.0f;
      break;
    case 15: // ARGB4444
      v = p[0] | (p[1] << 8);
      c[0] = ((v >> 8) & 0xf) / 15.0f; c[1] = ((v >> 4) & 0xf) / 15.0f; c[2] = (v & 0xf) / 15.0f; c[3] = (v >> 12) / 15.0f;
      break;
    default:
      c[0] = c[1] = c[2] = 0.0f; c[3] = 1.0f;
      break;
  }
}

// Ordered dither offsets (4x4 Bayer), in units of one LSB
static const float r3d_bayer[4][4] = {
  { 0.0f / 16.0f,  8.0f / 16.0f,  2.0f / 16.0f, 10.0f / 16.0f },
  { 12.0f / 16.0f, 4.0f / 16.0f, 14.0f / 16.0f,  6.0f / 16.0f },
  { 3.0f / 16.0f, 11.0f / 16.0f,  1.0f / 16.0f,  9.0f / 16.0f },
  { 15.0f / 16.0f, 7.0f / 16.0f, 13.0f / 16.0f,  5.0f / 16.0f }
};

static BX_CPP_INLINE Bit32u r3d_quant(float v, int bits, int dither, int x, int y)
{
  float maxv = (float)((1 << bits) - 1);
  float f = r3d_clamp01(v) * maxv;
  if (dither == 1) f += 0.5f;
  else if (dither >= 2) f += r3d_bayer[y & 3][x & 3];
  Bit32u q = (Bit32u)f;
  if (q > (Bit32u)maxv) q = (Bit32u)maxv;
  return q;
}

static Bit32u r3d_cb_encode(int fmt, const float *c, int dither, int adither, int x, int y, Bit8u *p, int *bytes)
{
  Bit32u v = 0;
  switch (fmt) {
    case 3:
      v = (r3d_quant(c[0], 5, dither, x, y) << 10) | (r3d_quant(c[1], 5, dither, x, y) << 5) |
          r3d_quant(c[2], 5, dither, x, y) | (r3d_quant(c[3], 1, adither, x, y) << 15);
      p[0] = v & 0xff; p[1] = v >> 8; *bytes = 2;
      break;
    case 4:
      v = (r3d_quant(c[0], 5, dither, x, y) << 11) | (r3d_quant(c[1], 6, dither, x, y) << 5) |
          r3d_quant(c[2], 5, dither, x, y);
      p[0] = v & 0xff; p[1] = v >> 8; *bytes = 2;
      break;
    case 6:
      v = (r3d_quant(c[3], 8, adither, x, y) << 24) | (r3d_quant(c[0], 8, dither, x, y) << 16) |
          (r3d_quant(c[1], 8, dither, x, y) << 8) | r3d_quant(c[2], 8, dither, x, y);
      p[0] = v & 0xff; p[1] = (v >> 8) & 0xff; p[2] = (v >> 16) & 0xff; p[3] = v >> 24; *bytes = 4;
      break;
    case 7: {
      Bit32u f[4] = { r9800_f2u(c[2]), r9800_f2u(c[1]), r9800_f2u(c[0]), r9800_f2u(c[3]) };
      for (int k = 0; k < 4; k++) {
        p[k * 4] = f[k] & 0xff; p[k * 4 + 1] = (f[k] >> 8) & 0xff;
        p[k * 4 + 2] = (f[k] >> 16) & 0xff; p[k * 4 + 3] = f[k] >> 24;
      }
      *bytes = 16;
      v = f[2];
      break;
    }
    case 9:
      v = r3d_quant(c[0], 8, dither, x, y);
      p[0] = (Bit8u)v; *bytes = 1;
      break;
    case 10: {
      Bit32u q[4] = { r3d_quant(c[2], 16, 0, x, y), r3d_quant(c[1], 16, 0, x, y),
                      r3d_quant(c[0], 16, 0, x, y), r3d_quant(c[3], 16, 0, x, y) };
      for (int k = 0; k < 4; k++) { p[k * 2] = q[k] & 0xff; p[k * 2 + 1] = q[k] >> 8; }
      *bytes = 8;
      v = q[2];
      break;
    }
    case 11: case 12: {
      // luma / chroma of the RGB value
      float yy = 0.257f * c[0] + 0.504f * c[1] + 0.098f * c[2] + 16.0f / 255.0f;
      float uu = -0.148f * c[0] - 0.291f * c[1] + 0.439f * c[2] + 128.0f / 255.0f;
      float vv = 0.439f * c[0] - 0.368f * c[1] - 0.071f * c[2] + 128.0f / 255.0f;
      Bit32u qy = r3d_quant(yy, 8, 1, x, y), qu = r3d_quant(uu, 8, 1, x, y), qv = r3d_quant(vv, 8, 1, x, y);
      if (fmt == 11) { p[0] = (x & 1) ? qu : qv; p[1] = qy; }
      else { p[0] = qy; p[1] = (x & 1) ? qu : qv; }
      *bytes = 2;
      v = p[0] | (p[1] << 8);
      break;
    }
    case 13:
      p[0] = r3d_quant(c[0], 8, dither, x, y); p[1] = r3d_quant(c[1], 8, dither, x, y); *bytes = 2;
      v = p[0] | (p[1] << 8);
      break;
    case 15:
      v = (r3d_quant(c[3], 4, adither, x, y) << 12) | (r3d_quant(c[0], 4, dither, x, y) << 8) |
          (r3d_quant(c[1], 4, dither, x, y) << 4) | r3d_quant(c[2], 4, dither, x, y);
      p[0] = v & 0xff; p[1] = v >> 8; *bytes = 2;
      break;
    default:
      *bytes = 0;
      break;
  }
  return v;
}

static BX_CPP_INLINE Bit32u r3d_cb_addr(const r300_draw_state_t *d, Bit32u base, int x, int y, int sample)
{
  Bit32u px = (Bit32u)(x * d->aa_samples + sample);
  return base + r300_tile_addr(px, (Bit32u)y, d->cb_pitch_px * (Bit32u)d->cb_bpp, (Bit32u)d->cb_bpp,
                               d->cb_micro, d->cb_macro);
}

bool bx_radeon9800_c::r3d_cb_read(const r9800_raster_state_t *rs, int x, int y, int sample, int target, float *col)
{
  const r300_draw_state_t *d = &rs->d;
  if (!d->cb_ok || (d->cb_vram[target & 3] == 0xffffffffu)) {
    col[0] = col[1] = col[2] = 0.0f; col[3] = 1.0f;
    return false;
  }
  Bit32u addr = r3d_cb_addr(d, d->cb_vram[target & 3], x, y, sample);
  if (addr + d->cb_bpp > vram_size) {
    col[0] = col[1] = col[2] = 0.0f; col[3] = 1.0f;
    return false;
  }
  r3d_cb_decode(d->cb_fmt, BX_R9800_THIS s.memory + addr, col);
  return true;
}

// Blend factor
static void r3d_blend_factor(int code, const float *src, const float *dst, const float *cc, int alpha_only, float *f)
{
  float sa = src[3], da = dst[3];
  switch (code) {
    case 1: case 32:
      f[0] = f[1] = f[2] = f[3] = 0.0f; break;
    case 2: case 33:
      f[0] = f[1] = f[2] = f[3] = 1.0f; break;
    case 3: case 34:
      for (int k = 0; k < 4; k++) f[k] = src[k];
      break;
    case 4: case 35:
      for (int k = 0; k < 4; k++) f[k] = 1.0f - src[k];
      break;
    case 5: case 38:
      f[0] = f[1] = f[2] = f[3] = sa; break;
    case 6: case 39:
      f[0] = f[1] = f[2] = f[3] = 1.0f - sa; break;
    case 7: case 40:
      f[0] = f[1] = f[2] = f[3] = da; break;
    case 8: case 41:
      f[0] = f[1] = f[2] = f[3] = 1.0f - da; break;
    case 9: case 36:
      for (int k = 0; k < 4; k++) f[k] = dst[k];
      break;
    case 10: case 37:
      for (int k = 0; k < 4; k++) f[k] = 1.0f - dst[k];
      break;
    case 11: case 42: {
      float m = (sa < 1.0f - da) ? sa : 1.0f - da;
      f[0] = f[1] = f[2] = m; f[3] = 1.0f;
      break;
    }
    case 12:  // BOTH_SRC_ALPHA: src factor sa, dst factor 1-sa
      f[0] = f[1] = f[2] = f[3] = alpha_only ? (1.0f - sa) : sa; break;
    case 13:  // BOTH_INV_SRC_ALPHA
      f[0] = f[1] = f[2] = f[3] = alpha_only ? sa : (1.0f - sa); break;
    case 43:
      for (int k = 0; k < 4; k++) f[k] = cc[k];
      break;
    case 44:
      for (int k = 0; k < 4; k++) f[k] = 1.0f - cc[k];
      break;
    case 45:
      f[0] = f[1] = f[2] = f[3] = cc[3]; break;
    case 46:
      f[0] = f[1] = f[2] = f[3] = 1.0f - cc[3]; break;
    default:
      f[0] = f[1] = f[2] = f[3] = 1.0f; break;
  }
}

static BX_CPP_INLINE float r3d_blend_fn(int fn, float s, float d, float fs, float fd)
{
  switch (fn & 7) {
    case 0: return r3d_clamp01(s * fs + d * fd);
    case 1: return s * fs + d * fd;
    case 2: return r3d_clamp01(s * fs - d * fd);
    case 3: return s * fs - d * fd;
    case 4: return (s < d) ? s : d;
    case 5: return (s > d) ? s : d;
    case 6: return r3d_clamp01(d * fd - s * fs);
    default: return d * fd - s * fs;
  }
}

static BX_CPP_INLINE Bit32u r3d_rop2(int rop, Bit32u s, Bit32u d)
{
  switch (rop & 15) {
    case 0: return 0;
    case 1: return ~(s | d);
    case 2: return ~s & d;
    case 3: return ~s;
    case 4: return s & ~d;
    case 5: return ~d;
    case 6: return s ^ d;
    case 7: return ~(s & d);
    case 8: return s & d;
    case 9: return ~(s ^ d);
    case 10: return d;
    case 11: return ~s | d;
    case 12: return s;
    case 13: return s | ~d;
    case 14: return s | d;
    default: return 0xffffffff;
  }
}

// Write one shaded colour to a colour buffer sample (blend, ROP, mask)
void bx_radeon9800_c::r3d_cb_write(const r9800_raster_state_t *rs, int x, int y, int sample, const float *col,
                                   const float *dst_in)
{
  const r300_draw_state_t *d = &rs->d;
  Bit8u *mem = BX_R9800_THIS s.memory;
  float src[4], dst[4], res[4];
  Bit8u enc[16];
  int bytes;

  if (!d->cb_ok)
    return;
  memcpy(src, col, sizeof(src));
  // DISCARD_SRC_PIXELS
  if (d->discard_src) {
    bool a0 = src[3] <= 0.0f, c0 = (src[0] <= 0.0f) && (src[1] <= 0.0f) && (src[2] <= 0.0f);
    bool a1 = src[3] >= 1.0f, c1 = (src[0] >= 1.0f) && (src[1] >= 1.0f) && (src[2] >= 1.0f);
    switch (d->discard_src) {
      case 1: if (a0) return; break;
      case 2: if (c0) return; break;
      case 3: if (a0 && c0) return; break;
      case 4: if (a1) return; break;
      case 5: if (c1) return; break;
      case 6: if (a1 && c1) return; break;
      default: break;
    }
  }
  for (int t = 0; t < d->multiwrite; t++) {
    Bit32u base = d->cb_vram[t];
    if (base == 0xffffffffu)
      continue;
    Bit32u addr = r3d_cb_addr(d, base, x, y, sample);
    if (addr + d->cb_bpp > vram_size)
      continue;
    Bit8u *p = mem + addr;
    if (dst_in && (t == 0)) memcpy(dst, dst_in, sizeof(dst));
    else r3d_cb_decode(d->cb_fmt, p, dst);
    if (d->blend_en) {
      float fs[4], fd[4];
      r3d_blend_factor(d->csrc, src, dst, d->const_col, 0, fs);
      r3d_blend_factor(d->cdst, src, dst, d->const_col, 1, fd);
      for (int k = 0; k < 3; k++)
        res[k] = r3d_blend_fn(d->cfn, src[k], dst[k], fs[k], fd[k]);
      r3d_blend_factor(d->asrc, src, dst, d->const_col, 0, fs);
      r3d_blend_factor(d->adst, src, dst, d->const_col, 1, fd);
      res[3] = r3d_blend_fn(d->afn, src[3], dst[3], fs[3], fd[3]);
    } else {
      memcpy(res, src, sizeof(res));
    }
    // channel mask (R, G, B, A)
    if (!(d->chan_mask & 1)) res[2] = dst[2];
    if (!(d->chan_mask & 2)) res[1] = dst[1];
    if (!(d->chan_mask & 4)) res[0] = dst[0];
    if (!(d->chan_mask & 8)) res[3] = dst[3];
    int dith = d->dith_en ? ((d->dither_mode > 1) ? d->dither_mode : 2) : d->dither_mode;
    Bit32u packed = r3d_cb_encode(d->cb_fmt, res, dith, d->adither_mode, x, y, enc, &bytes);
    if (!bytes)
      continue;
    if (d->rop_en && (bytes <= 4)) {
      Bit32u cur = 0;
      for (int k = 0; k < bytes; k++) cur |= (Bit32u)p[k] << (k * 8);
      packed = r3d_rop2(d->rop, packed, cur);
      for (int k = 0; k < bytes; k++) enc[k] = (packed >> (k * 8)) & 0xff;
    }
    if (d->clrcmp_en && (bytes <= 4)) {
      Bit32u cur = 0;
      for (int k = 0; k < bytes; k++) cur |= (Bit32u)p[k] << (k * 8);
      bool match = ((cur & d->clrcmp_msk) == (d->clrcmp_clr & d->clrcmp_msk));
      if (d->clrcmp_flipe & 1) match = !match;
      if (match)
        continue;
    }
    memcpy(p, enc, bytes);
    if (t == 0)
      r3d_mark_dirty(addr, bytes);
  }
}

// AA resolve: average the samples of pixel (x, y) into the resolve buffer
void bx_radeon9800_c::r3d_aa_resolve(const r9800_raster_state_t *rs, int x, int y)
{
  const r300_draw_state_t *d = &rs->d;
  Bit32u dst_vram;
  float acc[4] = { 0, 0, 0, 0 }, c[4];
  Bit8u enc[16];
  int bytes;

  if (!d->aa_resolve || !d->cb_ok || (d->aa_samples <= 1) || !d->aar_pitch_px)
    return;
  dst_vram = d->aar_vram;
  if (dst_vram == 0xffffffffu)
    return;
  for (int s = 0; s < d->aa_samples; s++) {
    r3d_cb_read(rs, x, y, s, 0, c);
    if (d->aar_gamma)
      for (int k = 0; k < 3; k++) c[k] = r3d_degamma(c[k]);
    for (int k = 0; k < 4; k++) acc[k] += c[k];
  }
  for (int k = 0; k < 4; k++) acc[k] /= (float)d->aa_samples;
  if (d->aar_gamma)
    for (int k = 0; k < 3; k++) acc[k] = (acc[k] <= 0.0031308f) ? acc[k] * 12.92f : (float)(1.055 * pow(acc[k], 1.0 / 2.4) - 0.055);
  Bit32u addr = dst_vram + r300_tile_addr((Bit32u)x, (Bit32u)y, d->aar_pitch_px * (Bit32u)d->cb_bpp,
                                           (Bit32u)d->cb_bpp, d->cb_micro, d->cb_macro);
  if (addr + d->cb_bpp > vram_size)
    return;
  r3d_cb_encode(d->cb_fmt, acc, 0, 0, x, y, enc, &bytes);
  if (bytes) {
    memcpy(BX_R9800_THIS s.memory + addr, enc, bytes);
    r3d_mark_dirty(addr, bytes);
  }
}

// =====================================================================
// Fragment pipeline: RS outputs -> shader -> fog / alpha test -> Z ->
// colour buffer
// =====================================================================

static BX_CPP_INLINE bool r9800_row_owned(Bit32s y, int thr_id, int thr_mask)
{
  if (thr_mask > 0)
    return (y & thr_mask) == thr_id;
  if (thr_mask == 0)
    return true;
  return ((Bit32u)y % (Bit32u)(-thr_mask)) == (Bit32u)thr_id;
}

// Clip rectangles + rule
static BX_CPP_INLINE bool r3d_clip_pass(const r300_draw_state_t *d, int x, int y)
{
  int idx = 0;
  for (int i = 0; i < 4; i++) {
    if ((x >= d->cx0[i]) && (x <= d->cx1[i]) && (y >= d->cy0[i]) && (y <= d->cy1[i]))
      idx |= 1 << i;
  }
  return (d->clip_rule >> idx) & 1;
}

void bx_radeon9800_c::r3d_fragment(const r9800_raster_state_t *rs, int x, int y, r300_frag_t *f)
{
  const r300_draw_state_t *d = &rs->d;
  float out[4][4];
  float depth = 0.0f;
  int depth_out = 0;
  float z = f->z;
  Bit32u pass_mask = 0;
  bool zp;

  // early Z: with ZTOP the depth / stencil test runs before the shader
  if (d->ztop) {
    for (int s = 0; s < d->aa_samples; s++) {
      if (!(f->cov & (1u << s))) continue;
      if (r3d_zs_test(rs, x, y, s, z, f->front, &zp))
        pass_mask |= 1u << s;
    }
    if (!pass_mask)
      return;
  }
  if (!r3d_run_fp(rs, f->temps, f->ddx, f->ddy, f->written, out, &depth, &depth_out))
    return;
  if (depth_out && d->depth_src)
    z = r3d_clamp01(depth);
  // fog
  if (d->fog_en) {
    float v = f->fog_v, amount;
    switch (d->fog_fn) {
      case 0: amount = r3d_clamp01(d->fog_scale * v + d->fog_off); break;
      case 1: amount = r3d_clamp01(1.0f - (float)exp(-(d->fog_scale * v) / 0.0933)); break;
      case 2: {
        float e = (d->fog_scale * v) / 0.3f;
        amount = r3d_clamp01(1.0f - (float)exp(-(double)e * e));
        break;
      }
      default: amount = d->fog_factor; break;
    }
    for (int k = 0; k < 3; k++)
      out[0][k] = out[0][k] * (1.0f - amount) + d->fog_col[k] * amount;
  }
  // alpha test
  if (d->af_en && !r3d_cmp_fnf(d->af_fn, out[0][3], d->af_val))
    return;
  // late Z
  if (!d->ztop) {
    for (int s = 0; s < d->aa_samples; s++) {
      if (!(f->cov & (1u << s))) continue;
      if (r3d_zs_test(rs, x, y, s, z, f->front, &zp))
        pass_mask |= 1u << s;
    }
    if (!pass_mask)
      return;
  }
  zpass_count++;
  // colour
  if (d->cb_ok) {
    for (int s = 0; s < d->aa_samples; s++) {
      if (!(pass_mask & (1u << s))) continue;
      r3d_cb_write(rs, x, y, s, out[0], NULL);
    }
    if (d->aa_resolve && (d->aa_samples > 1))
      r3d_aa_resolve(rs, x, y);
  }
}

// ---------------------------------------------------------------------
// Attribute plane setup shared by the primitive rasterizers
// ---------------------------------------------------------------------

// A screen-space plane: v(x, y) = a*x + b*y + c
typedef struct {
  float a, b, c;
} r3d_plane_t;

static BX_CPP_INLINE void r3d_plane_setup(const float *x, const float *y, const float *v, float inv_area, r3d_plane_t *p)
{
  // v(x,y) from three points via the barycentric gradients
  float dv1 = v[1] - v[0], dv2 = v[2] - v[0];
  float dx1 = x[1] - x[0], dx2 = x[2] - x[0];
  float dy1 = y[1] - y[0], dy2 = y[2] - y[0];
  p->a = (dv1 * dy2 - dv2 * dy1) * inv_area;
  p->b = (dv2 * dx1 - dv1 * dx2) * inv_area;
  p->c = v[0] - p->a * x[0] - p->b * y[0];
}

static BX_CPP_INLINE float r3d_plane_eval(const r3d_plane_t *p, float x, float y)
{
  return p->a * x + p->b * y + p->c;
}

// Everything the RS needs to fill one fragment from the planes
typedef struct {
  r3d_plane_t q;                 // 1/W
  r3d_plane_t z;
  r3d_plane_t col[4][4];         // colour * q
  r3d_plane_t tc[32];            // texcoord * q
  r3d_plane_t st[2];             // stuffed s, t (linear)
  float       flat_col[4][4];    // provoking vertex colours
  Bit32u      col_mode[4];       // 0 solid 1 flat 2 gouraud (rgb) ; alpha in bits 4..
  Bit32u      a_mode[4];
} r3d_interp_t;

static const float r3d_aa_pos[4][6][2] = {
  { { 0.25f, 0.25f }, { 0.75f, 0.75f }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 } },
  { { 0.5f, 0.1875f }, { 0.1875f, 0.8125f }, { 0.8125f, 0.8125f }, { 0, 0 }, { 0, 0 }, { 0, 0 } },
  { { 0.375f, 0.125f }, { 0.875f, 0.375f }, { 0.125f, 0.625f }, { 0.625f, 0.875f }, { 0, 0 }, { 0, 0 } },
  { { 0.0625f, 0.5f }, { 0.3125f, 0.125f }, { 0.4375f, 0.8125f }, { 0.6875f, 0.1875f }, { 0.8125f, 0.5625f }, { 0.9375f, 0.9375f } }
};

static BX_CPP_INLINE int r3d_aa_pattern(int samples)
{
  switch (samples) {
    case 2: return 0;
    case 3: return 1;
    case 4: return 2;
    case 6: return 3;
    default: return -1;
  }
}

// Fill the RS stack frame of the fragment at pixel centre (px, py)
static void r3d_rs_fill(const r9800_raster_state_t *rs, const r3d_interp_t *ip, float px, float py, int stuffed,
                        r300_frag_t *f)
{
  const r300_draw_state_t *d = &rs->d;
  float q = r3d_plane_eval(&ip->q, px, py);
  float iq = (q != 0.0f) ? 1.0f / q : 0.0f;
  float tc[32], tcx[32], tcy[32];
  float col[4][4], colx[4][4], coly[4][4];
  float qx = ip->q.a, qy = ip->q.b;

  f->written = 0;
  f->w = (q != 0.0f) ? 1.0f / q : 0.0f;
  f->z = r3d_plane_eval(&ip->z, px, py);
  f->dzdx = ip->z.a;
  f->dzdy = ip->z.b;
  // perspective-correct texcoords with derivatives (quotient rule)
  for (int i = 0; i < d->n_tc; i++) {
    float p = r3d_plane_eval(&ip->tc[i], px, py);
    tc[i] = p * iq;
    tcx[i] = (ip->tc[i].a * q - p * qx) * iq * iq;
    tcy[i] = (ip->tc[i].b * q - p * qy) * iq * iq;
  }
  for (int i = d->n_tc; i < 32; i++) tc[i] = tcx[i] = tcy[i] = 0.0f;
  for (int c = 0; c < 4; c++) {
    for (int k = 0; k < 4; k++) {
      Bit32u mode = (k == 3) ? ip->a_mode[c] : ip->col_mode[c];
      if (mode == 0) {
        col[c][k] = d->solid[k]; colx[c][k] = coly[c][k] = 0.0f;
      } else if (mode == 1) {
        col[c][k] = ip->flat_col[c][k]; colx[c][k] = coly[c][k] = 0.0f;
      } else {
        float p = r3d_plane_eval(&ip->col[c][k], px, py);
        col[c][k] = p * iq;
        colx[c][k] = (ip->col[c][k].a * q - p * qx) * iq * iq;
        coly[c][k] = (ip->col[c][k].b * q - p * qy) * iq * iq;
      }
    }
  }
  float st[2] = { r3d_plane_eval(&ip->st[0], px, py), r3d_plane_eval(&ip->st[1], px, py) };
  float stx[2] = { ip->st[0].a, ip->st[1].a };
  float sty[2] = { ip->st[0].b, ip->st[1].b };

  // RS instructions
  for (int i = 0; i < d->rs_count; i++) {
    const int tex_id = d->rs_inst[i].tex_id & 7;
    if (d->rs_inst[i].tex_cn & 1) {
      float *dst = f->temps[d->rs_inst[i].tex_addr & 31];
      float *dx = f->ddx[d->rs_inst[i].tex_addr & 31];
      float *dy = f->ddy[d->rs_inst[i].tex_addr & 31];
      int ptr = d->rs_ip[tex_id].tex_ptr;
      bool stuff = stuffed && (d->tex_src[tex_id] != 0);
      for (int k = 0; k < 4; k++) {
        int sel = d->rs_ip[tex_id].sel[k];
        if (sel >= 4) {
          dst[k] = (sel == 4) ? 0.0f : 1.0f;
          dx[k] = dy[k] = 0.0f;
        } else if (stuff) {
          if (sel < 2) { dst[k] = st[sel]; dx[k] = stx[sel]; dy[k] = sty[sel]; }
          else { dst[k] = (sel == 2) ? 0.0f : 1.0f; dx[k] = dy[k] = 0.0f; }
        } else {
          int idx = ptr + sel;
          if (idx < 32) { dst[k] = tc[idx]; dx[k] = tcx[idx]; dy[k] = tcy[idx]; }
          else { dst[k] = (sel == 3) ? 1.0f : 0.0f; dx[k] = dy[k] = 0.0f; }
        }
      }
      f->written |= 1u << (d->rs_inst[i].tex_addr & 31);
    }
    if (d->rs_inst[i].col_cn) {
      const int col_id = d->rs_inst[i].col_id & 7;
      float *dst = f->temps[d->rs_inst[i].col_addr & 31];
      float *dx = f->ddx[d->rs_inst[i].col_addr & 31];
      float *dy = f->ddy[d->rs_inst[i].col_addr & 31];
      int cp = d->rs_ip[col_id].col_ptr & 3;
      // back-face colour: the next colour set
      if ((d->rs_inst[i].col_cn == 3) && !f->front)
        cp = (cp + 1) & 3;
      float r = col[cp][0], g = col[cp][1], b = col[cp][2], a = col[cp][3];
      float rx = colx[cp][0], gx = colx[cp][1], bx = colx[cp][2], ax = colx[cp][3];
      float ry = coly[cp][0], gy = coly[cp][1], by = coly[cp][2], ay = coly[cp][3];
      switch (d->rs_ip[col_id].col_fmt) {
        case 1: a = 0.0f; ax = ay = 0.0f; break;                               // RGB0
        case 2: a = 1.0f; ax = ay = 0.0f; break;                               // RGB1
        case 3: r = g = b = 0.0f; rx = gx = bx = ry = gy = by = 0.0f; break;   // 000A
        case 4: r = g = b = a = 0.0f; rx = gx = bx = ax = ry = gy = by = ay = 0.0f; break;
        case 5: r = g = b = 0.0f; a = 1.0f; rx = gx = bx = ax = ry = gy = by = ay = 0.0f; break;
        case 6: r = g = b = 1.0f; rx = gx = bx = ry = gy = by = 0.0f; break;   // 111A
        case 7: r = g = b = 1.0f; a = 0.0f; rx = gx = bx = ax = ry = gy = by = ay = 0.0f; break;
        case 8: r = g = b = a = 1.0f; rx = gx = bx = ax = ry = gy = by = ay = 0.0f; break;
        case 9: r = g = b = a; rx = gx = bx = ax; ry = gy = by = ay; a = 0.0f; ax = ay = 0.0f; break; // 0AAA
        case 10: r = g = b = a; rx = gx = bx = ax; ry = gy = by = ay; a = 1.0f; ax = ay = 0.0f; break; // 1AAA
        default: break;
      }
      dst[0] = r; dst[1] = g; dst[2] = b; dst[3] = a;
      dx[0] = rx; dx[1] = gx; dx[2] = bx; dx[3] = ax;
      dy[0] = ry; dy[1] = gy; dy[2] = by; dy[3] = ay;
      f->written |= 1u << (d->rs_inst[i].col_addr & 31);
    }
  }
  if (d->rs_w_en) {
    int wa = (rs->r[R3D(R9800_RS_COUNT)] >> 12) & 0x1f;   // W_ADDR
    float w = f->w;
    for (int k = 0; k < 4; k++) { f->temps[wa][k] = w; f->ddx[wa][k] = 0.0f; f->ddy[wa][k] = 0.0f; }
    f->written |= 1u << wa;
  }
  // fog source
  switch (d->fog_sel) {
    case 0: case 1: case 2: case 3: f->fog_v = col[d->fog_sel][3]; break;
    case 4: f->fog_v = f->w; break;
    default: f->fog_v = f->z; break;
  }
  if (d->depth_sel && (d->fog_sel > 4))
    f->fog_v = f->w;
}

// Polygon offset in depth units
static BX_CPP_INLINE float r3d_poly_offset(const r300_draw_state_t *d, int front, float dzdx, float dzdy)
{
  int en = front ? d->poff_front_en : d->poff_back_en;
  if (!en)
    return 0.0f;
  float scale = front ? d->poff_fscale : d->poff_bscale;
  float off = front ? d->poff_foff : d->poff_boff;
  float slope = (float)fabs(dzdx);
  if (fabs(dzdy) > slope) slope = (float)fabs(dzdy);
  float ds = (d->depth_scale != 0.0f) ? d->depth_scale : 1.0f;
  // scale is per sub-pixel, offset in integer depth units
  return (scale / d->subf) * slope + off / ds;
}

static BX_CPP_INLINE float r3d_snap(float v, float sub)
{
  return (float)floor(v * sub + 0.5f) / sub;
}

// Cylindrical texture wrap: bring b and c within half a turn of a
static void r3d_apply_tex_wrap(const r300_draw_state_t *d, r300_vtx_t *v, int n)
{
  if (!d->tex_wrap || (n < 2))
    return;
  // walk the packed tc[] layout (GB_VAP_RASTER_VTX_FMT_1 component counts)
  int idx = 0;
  for (int t = 0; (t < 8) && (idx < d->n_tc); t++) {
    int cnt = (int)((d->raster_fmt1 >> (t * 3)) & 7);
    if (cnt > 4) cnt = 4;
    for (int c = 0; (c < cnt) && (idx < d->n_tc); c++, idx++) {
      if (!((d->tex_wrap >> (t * 4 + c)) & 1))
        continue;
      for (int k = 1; k < n; k++) {
        float dlt = v[k].tc[idx] - v[0].tc[idx];
        if (dlt > 0.5f) v[k].tc[idx] -= 1.0f;
        else if (dlt < -0.5f) v[k].tc[idx] += 1.0f;
      }
    }
  }
}

// Set up the interpolation planes of a triangle
static void r3d_interp_setup(const r9800_raster_state_t *rs, const r300_vtx_t *v0, const r300_vtx_t *v1,
                             const r300_vtx_t *v2, const float *x, const float *y, float inv_area,
                             r3d_interp_t *ip, int provoke)
{
  const r300_draw_state_t *d = &rs->d;
  const r300_vtx_t *v[3] = { v0, v1, v2 };
  float q[3], t[3];

  for (int i = 0; i < 3; i++) q[i] = v[i]->pos[3];
  r3d_plane_setup(x, y, q, inv_area, &ip->q);
  for (int i = 0; i < 3; i++) t[i] = v[i]->pos[2];
  r3d_plane_setup(x, y, t, inv_area, &ip->z);
  for (int i = 0; i < d->n_tc; i++) {
    for (int k = 0; k < 3; k++) t[k] = v[k]->tc[i] * q[k];
    r3d_plane_setup(x, y, t, inv_area, &ip->tc[i]);
  }
  for (int c = 0; c < 4; c++) {
    ip->col_mode[c] = (Bit32u)d->rgb_shade[c];
    ip->a_mode[c] = (Bit32u)d->a_shade[c];
    for (int k = 0; k < 4; k++) {
      ip->flat_col[c][k] = v[provoke]->col[c][k];
      for (int j = 0; j < 3; j++) t[j] = v[j]->col[c][k] * q[j];
      r3d_plane_setup(x, y, t, inv_area, &ip->col[c][k]);
    }
  }
  ip->st[0].a = ip->st[0].b = ip->st[0].c = 0.0f;
  ip->st[1].a = ip->st[1].b = ip->st[1].c = 0.0f;
}

// ---------------------------------------------------------------------
// Triangle
// ---------------------------------------------------------------------

void bx_radeon9800_c::r3d_tri(const r9800_raster_state_t *rs, int thr_id, int thr_mask, const r300_vtx_t *a,
                              const r300_vtx_t *b, const r300_vtx_t *c, int front)
{
  const r300_draw_state_t *d = &rs->d;
  r300_vtx_t vv[3];
  float x[3], y[3];
  r3d_interp_t ip;
  r300_frag_t f;

  if (!d->draw_ok)
    return;
  vv[0] = *a; vv[1] = *b; vv[2] = *c;
  r3d_apply_tex_wrap(d, vv, 3);
  for (int i = 0; i < 3; i++) {
    x[i] = r3d_snap(vv[i].pos[0], d->subf);
    y[i] = r3d_snap(vv[i].pos[1], d->subf);
  }
  float area = (x[1] - x[0]) * (y[2] - y[0]) - (x[2] - x[0]) * (y[1] - y[0]);
  if (area == 0.0f)
    return;
  // orient so that the edge functions are positive inside: area > 0
  bool swapped = false;
  if (area < 0.0f) {
    r300_vtx_t tv = vv[1]; vv[1] = vv[2]; vv[2] = tv;
    float tx = x[1]; x[1] = x[2]; x[2] = tx;
    float ty = y[1]; y[1] = y[2]; y[2] = ty;
    area = -area;
    swapped = true;
  }
  float inv_area = 1.0f / area;
  // provoking vertex in submission order (0 first, 1 second, 2/3 third/last)
  int provoke = (d->provoke == 0) ? 0 : (d->provoke == 1) ? 1 : 2;
  if (swapped && (provoke == 1)) provoke = 2;
  else if (swapped && (provoke == 2)) provoke = 1;
  r3d_interp_setup(rs, &vv[0], &vv[1], &vv[2], x, y, inv_area, &ip, provoke);
  // stuffed (s, t) for triangles: barycentric of vertices 1 and 2
  if (d->tri_stuff) {
    float s[3] = { 0.0f, 1.0f, 0.0f }, t[3] = { 0.0f, 0.0f, 1.0f };
    r3d_plane_setup(x, y, s, inv_area, &ip.st[0]);
    r3d_plane_setup(x, y, t, inv_area, &ip.st[1]);
  }

  // bounding box clamped to the scissor
  int bx0 = (int)floor(x[0] < x[1] ? (x[0] < x[2] ? x[0] : x[2]) : (x[1] < x[2] ? x[1] : x[2]));
  int bx1 = (int)ceil(x[0] > x[1] ? (x[0] > x[2] ? x[0] : x[2]) : (x[1] > x[2] ? x[1] : x[2]));
  int by0 = (int)floor(y[0] < y[1] ? (y[0] < y[2] ? y[0] : y[2]) : (y[1] < y[2] ? y[1] : y[2]));
  int by1 = (int)ceil(y[0] > y[1] ? (y[0] > y[2] ? y[0] : y[2]) : (y[1] > y[2] ? y[1] : y[2]));
  if (bx0 < d->sx0) bx0 = d->sx0;
  if (by0 < d->sy0) by0 = d->sy0;
  if (bx1 > d->sx1) bx1 = d->sx1;
  if (by1 > d->sy1) by1 = d->sy1;
  if (bx0 < 0) bx0 = 0;
  if (by0 < 0) by0 = 0;
  if (bx1 > 8191) bx1 = 8191;
  if (by1 > 8191) by1 = 8191;
  if ((bx0 > bx1) || (by0 > by1))
    return;

  // edge functions E_i(p) = (x_j - x_i)*(py - y_i) - (y_j - y_i)*(px - x_i)
  float ex[3], ey[3], ec[3];
  bool tl[3];
  for (int i = 0; i < 3; i++) {
    int j = (i + 1) % 3;
    ex[i] = -(y[j] - y[i]);
    ey[i] = (x[j] - x[i]);
    ec[i] = -(ex[i] * x[i] + ey[i] * y[i]);
    // top-left rule: an edge is top if horizontal and going left, left if going up (y decreasing)
    tl[i] = ((y[j] == y[i]) && (x[j] < x[i])) || (y[j] < y[i]);
  }
  int pat = r3d_aa_pattern(d->aa_samples);
  float poff_base = r3d_poly_offset(d, front, ip.z.a, ip.z.b);
  f.front = front;

  for (int py = by0; py <= by1; py++) {
    if (!r9800_row_owned(py, thr_id, thr_mask))
      continue;
    float fy = (float)py + 0.5f;
    for (int px = bx0; px <= bx1; px++) {
      float fx = (float)px + 0.5f;
      Bit32u cov = 0;
      if (pat < 0) {
        bool in = true;
        for (int i = 0; i < 3; i++) {
          float e = ex[i] * fx + ey[i] * fy + ec[i];
          if ((e < 0.0f) || ((e == 0.0f) && !tl[i])) { in = false; break; }
        }
        if (!in) continue;
        cov = 1;
      } else {
        for (int s = 0; s < d->aa_samples; s++) {
          float sx = (float)px + r3d_aa_pos[pat][s][0], sy = (float)py + r3d_aa_pos[pat][s][1];
          bool in = true;
          for (int i = 0; i < 3; i++) {
            float e = ex[i] * sx + ey[i] * sy + ec[i];
            if ((e < 0.0f) || ((e == 0.0f) && !tl[i])) { in = false; break; }
          }
          if (in) cov |= 1u << s;
        }
        if (!cov) continue;
      }
      if (!r3d_clip_pass(d, px, py))
        continue;
      r3d_rs_fill(rs, &ip, fx, fy, d->tri_stuff, &f);
      f.z += poff_base;
      f.cov = cov;
      r3d_fragment(rs, px, py, &f);
    }
  }
}

// ---------------------------------------------------------------------
// Line
// ---------------------------------------------------------------------

void bx_radeon9800_c::r3d_line(const r9800_raster_state_t *rs, int thr_id, int thr_mask, const r300_vtx_t *a,
                               const r300_vtx_t *b)
{
  const r300_draw_state_t *d = &rs->d;
  r300_vtx_t vv[2];
  r3d_interp_t ip;
  r300_frag_t f;

  if (!d->draw_ok)
    return;
  vv[0] = *a; vv[1] = *b;
  r3d_apply_tex_wrap(d, vv, 2);
  float x0 = r3d_snap(vv[0].pos[0], d->subf), y0 = r3d_snap(vv[0].pos[1], d->subf);
  float x1 = r3d_snap(vv[1].pos[0], d->subf), y1 = r3d_snap(vv[1].pos[1], d->subf);
  float dx = x1 - x0, dy = y1 - y0;
  float len2 = dx * dx + dy * dy;
  if (len2 == 0.0f) {
    r3d_point(rs, thr_id, thr_mask, a);
    return;
  }
  float width = d->line_hw * 2.0f;
  if (width < 1.0f) width = 1.0f;
  float hw = width * 0.5f;
  // planes along the line: build a degenerate triangle with a third point off the line
  float px2 = x0 - dy, py2 = y0 + dx;
  float x[3] = { x0, x1, px2 }, y[3] = { y0, y1, py2 };
  float area = (x[1] - x[0]) * (y[2] - y[0]) - (x[2] - x[0]) * (y[1] - y[0]);
  float inv_area = 1.0f / area;
  // third vertex = first vertex (attributes constant across the line's width)
  int provoke = (d->provoke == 0) ? 0 : 1;
  r3d_interp_setup(rs, &vv[0], &vv[1], &vv[0], x, y, inv_area, &ip, provoke);
  if (d->line_stuff) {
    float s[3] = { d->ls0, d->ls1, d->ls0 }, t[3] = { 0.0f, 0.0f, 0.0f };
    r3d_plane_setup(x, y, s, inv_area, &ip.st[0]);
    r3d_plane_setup(x, y, t, inv_area, &ip.st[1]);
  }
  float poff_base = r3d_poly_offset(d, 1, ip.z.a, ip.z.b);
  f.front = 1;
  f.cov = (d->aa_samples > 1) ? ((1u << d->aa_samples) - 1u) : 1u;

  // major axis stepping (diamond exit approximated by half-open end)
  bool xmajor = fabs(dx) >= fabs(dy);
  if (xmajor) {
    float xs = x0, xe = x1;
    float ys = y0, ye = y1;
    if (xs > xe) { float t = xs; xs = xe; xe = t; t = ys; ys = ye; ye = t; }
    int ix0 = (int)floor(xs + 0.5f), ix1 = (int)ceil(xe - 0.5f);
    if (d->line_end == 0) ix1 = (int)floor(xe + 0.5f) - 1;
    if (ix1 < ix0) ix1 = ix0;
    float slope = (ye - ys) / (xe - xs);
    for (int px = ix0; px <= ix1; px++) {
      if ((px < d->sx0) || (px > d->sx1)) continue;
      float fx = (float)px + 0.5f;
      float cy = ys + (fx - xs) * slope;
      int py0 = (int)floor(cy - hw + 0.5f), py1 = (int)ceil(cy + hw - 0.5f);
      if (py1 < py0) py1 = py0;
      for (int py = py0; py <= py1; py++) {
        if ((py < d->sy0) || (py > d->sy1)) continue;
        if (!r9800_row_owned(py, thr_id, thr_mask)) continue;
        if (!r3d_clip_pass(d, px, py)) continue;
        // attributes along the line: project the pixel onto the axis
        r3d_rs_fill(rs, &ip, fx, cy, d->line_stuff, &f);
        f.z += poff_base;
        r3d_fragment(rs, px, py, &f);
      }
    }
  } else {
    float xs = x0, xe = x1;
    float ys = y0, ye = y1;
    if (ys > ye) { float t = xs; xs = xe; xe = t; t = ys; ys = ye; ye = t; }
    int iy0 = (int)floor(ys + 0.5f), iy1 = (int)ceil(ye - 0.5f);
    if (d->line_end == 0) iy1 = (int)floor(ye + 0.5f) - 1;
    if (iy1 < iy0) iy1 = iy0;
    float slope = (xe - xs) / (ye - ys);
    for (int py = iy0; py <= iy1; py++) {
      if ((py < d->sy0) || (py > d->sy1)) continue;
      if (!r9800_row_owned(py, thr_id, thr_mask)) continue;
      float fy = (float)py + 0.5f;
      float cx = xs + (fy - ys) * slope;
      int px0 = (int)floor(cx - hw + 0.5f), px1 = (int)ceil(cx + hw - 0.5f);
      if (px1 < px0) px1 = px0;
      for (int px = px0; px <= px1; px++) {
        if ((px < d->sx0) || (px > d->sx1)) continue;
        if (!r3d_clip_pass(d, px, py)) continue;
        r3d_rs_fill(rs, &ip, cx, fy, d->line_stuff, &f);
        f.z += poff_base;
        r3d_fragment(rs, px, py, &f);
      }
    }
  }
}

// ---------------------------------------------------------------------
// Point (square, optionally a sprite with stuffed texture coordinates)
// ---------------------------------------------------------------------

void bx_radeon9800_c::r3d_point(const r9800_raster_state_t *rs, int thr_id, int thr_mask, const r300_vtx_t *v)
{
  const r300_draw_state_t *d = &rs->d;
  r3d_interp_t ip;
  r300_frag_t f;

  if (!d->draw_ok)
    return;
  float cx = r3d_snap(v->pos[0], d->subf), cy = r3d_snap(v->pos[1], d->subf);
  float hw = d->point_hw, hh = d->point_hh;
  if (v->psize > 0.0f) {
    float ps = v->psize;
    if (ps < d->point_min) ps = d->point_min;
    if ((d->point_max > 0.0f) && (ps > d->point_max)) ps = d->point_max;
    hw = hh = ps * 0.5f;
  }
  if (hw < 0.5f) hw = 0.5f;
  if (hh < 0.5f) hh = 0.5f;
  // constant attribute planes
  float x[3] = { cx, cx + 1.0f, cx }, y[3] = { cy, cy, cy + 1.0f };
  r3d_interp_setup(rs, v, v, v, x, y, 1.0f, &ip, 0);
  for (int c = 0; c < 4; c++) {
    if (ip.col_mode[c] == 2) ip.col_mode[c] = 1;
    if (ip.a_mode[c] == 2) ip.a_mode[c] = 1;
  }
  // sprite coordinates across the square
  if (d->point_stuff) {
    float xl = cx - hw, xr = cx + hw, yt = cy - hh, yb = cy + hh;
    ip.st[0].a = (d->ps1 - d->ps0) / (xr - xl);
    ip.st[0].b = 0.0f;
    ip.st[0].c = d->ps0 - ip.st[0].a * xl;
    ip.st[1].a = 0.0f;
    ip.st[1].b = (d->pt1 - d->pt0) / (yb - yt);
    ip.st[1].c = d->pt0 - ip.st[1].b * yt;
  }
  int px0 = (int)floor(cx - hw + 0.5f), px1 = (int)ceil(cx + hw - 0.5f) - 1;
  int py0 = (int)floor(cy - hh + 0.5f), py1 = (int)ceil(cy + hh - 0.5f) - 1;
  if (px1 < px0) px1 = px0;
  if (py1 < py0) py1 = py0;
  if (px0 < d->sx0) px0 = d->sx0;
  if (py0 < d->sy0) py0 = d->sy0;
  if (px1 > d->sx1) px1 = d->sx1;
  if (py1 > d->sy1) py1 = d->sy1;
  f.front = 1;
  f.cov = (d->aa_samples > 1) ? ((1u << d->aa_samples) - 1u) : 1u;
  for (int py = py0; py <= py1; py++) {
    if (!r9800_row_owned(py, thr_id, thr_mask)) continue;
    for (int px = px0; px <= px1; px++) {
      if (!r3d_clip_pass(d, px, py)) continue;
      r3d_rs_fill(rs, &ip, (float)px + 0.5f, (float)py + 0.5f, d->point_stuff, &f);
      r3d_fragment(rs, px, py, &f);
    }
  }
}

// =====================================================================
// Parallel rasterizer: deferred batch fanned out across persistent,
// scanline-interleaved worker threads
// =====================================================================

BX_THREAD_FUNC(radeon9800_raster_worker_func, indata)
{
  rb_worker_t *w = (rb_worker_t *)indata;
  w->dev->raster_worker_main(w);
  BX_THREAD_EXIT;
}

void bx_radeon9800_c::raster_worker_main(rb_worker_t *w)
{
  for (;;) {
    bx_wait_sem(&w->wake);
    if (!raster_run)
      break;
    for (Bit32u i = 0; i < rb.cmd_count; i++) {
      const rb_cmd_t *c = &rb.cmds[i];
      const r9800_raster_state_t *rs = &rb.states[c->state_idx];
      if (c->py1 - c->py0 + 1 < rb.nthreads) {
        bool own = false;
        for (Bit32s y = c->py0; y <= c->py1; y++)
          if (r9800_row_owned(y, w->id, w->mask)) { own = true; break; }
        if (!own)
          continue;
      }
      if (c->kind == RB_TRI)
        r3d_tri(rs, w->id, w->mask, &c->v[0], &c->v[1], &c->v[2], c->front);
      else if (c->kind == RB_LINE)
        r3d_line(rs, w->id, w->mask, &c->v[0], &c->v[1]);
      else
        r3d_point(rs, w->id, w->mask, &c->v[0]);
    }
    bx_set_sem(&w->done);
  }
}

void bx_radeon9800_c::raster_init(void)
{
  int n = render_threads;
  if (n < 1) n = 1;
  if (n > R9800_RASTER_MAX_WORKERS) n = R9800_RASTER_MAX_WORKERS;
  memset(&rb, 0, sizeof(rb));
  rb.nthreads = n;
  rb.mask = (n & (n - 1)) ? -n : n - 1;
  rb_reset_written();
  BX_INIT_MUTEX(zmask_mutex);
  raster_run = true;
  if (n <= 1)
    return;
  for (int i = 0; i < n; i++) {
    rb_worker_t *w = &rb.workers[i];
    w->dev = this;
    w->id = i;
    w->mask = rb.mask;
    bx_create_sem(&w->wake);
    bx_create_sem(&w->done);
    BX_THREAD_CREATE(radeon9800_raster_worker_func, w, w->thread);
  }
}

void bx_radeon9800_c::raster_close(void)
{
  if (!raster_run)
    return;
  raster_run = false;
  if (rb.nthreads > 1) {
    for (int i = 0; i < rb.nthreads; i++)
      bx_set_sem(&rb.workers[i].wake);
    for (int i = 0; i < rb.nthreads; i++) {
      BX_THREAD_JOIN(rb.workers[i].thread);
      bx_destroy_sem(&rb.workers[i].wake);
      bx_destroy_sem(&rb.workers[i].done);
    }
  }
  BX_FINI_MUTEX(zmask_mutex);
  if (rb.cmds) free(rb.cmds);
  if (rb.states) free(rb.states);
  rb.cmds = NULL;
  rb.states = NULL;
  rb.nthreads = 1;
}

void bx_radeon9800_c::rb_run_parallel(void)
{
  for (int i = 0; i < rb.nthreads; i++)
    bx_set_sem(&rb.workers[i].wake);
  for (int i = 0; i < rb.nthreads; i++)
    bx_wait_sem(&rb.workers[i].done);
}

// Render the current batch and reset it
void bx_radeon9800_c::raster_flush(void)
{
  if ((rb.nthreads > 1) && (rb.cmd_count != 0)) {
    rb_run_parallel();
    rb.cmd_count = 0;
    rb.state_count = 0;
    rb.rt_valid = false;
    rb.rt_z_valid = false;
    rb_reset_written();
    cp_batch_pending = 0;
  }
  // recycle the texture staging arena (workers are idle)
  tex_stage.used = 0;
  tex_stage.ent_count = 0;
}

void bx_radeon9800_c::raster_abandon(void)
{
  rb.cmd_count = 0;
  rb.state_count = 0;
  rb.rt_valid = false;
  rb.rt_z_valid = false;
  rb_reset_written();
  cp_batch_pending = 0;
}

void bx_radeon9800_c::rb_reset_written(void)
{
  rb.wr_valid = false;
  rb.wr_c_lo = rb.wr_z_lo = 0xffffffff;
  rb.wr_c_hi = rb.wr_z_hi = 0;
}

static void rb_rows_to_bytes(Bit32u base, Bit32u stride, Bit32u tile_rows, Bit32s y0, Bit32s y1, Bit32u *lo, Bit32u *hi)
{
  Bit64u a, b;
  if (!stride || (y1 < y0))
    return;
  if (y0 < 0) y0 = 0;
  if (tile_rows > 1) {
    a = (Bit64u)base + (Bit64u)tile_rows * ((Bit32u)y0 / tile_rows) * stride;
    b = (Bit64u)base + (Bit64u)tile_rows * (((Bit32u)y1 / tile_rows) + 1u) * stride;
  } else {
    a = (Bit64u)base + (Bit64u)(Bit32u)y0 * stride;
    b = (Bit64u)base + ((Bit64u)(Bit32u)y1 + 1u) * stride;
  }
  if (a > 0xffffffffull) a = 0xffffffffull;
  if (b > 0xffffffffull) b = 0xffffffffull;
  if (b <= a)
    return;
  *lo = (Bit32u)a;
  *hi = (Bit32u)b;
}

static BX_CPP_INLINE bool rb_rng_hit(Bit32u alo, Bit32u ahi, Bit32u blo, Bit32u bhi)
{
  return (ahi > alo) && (bhi > blo) && (alo < bhi) && (ahi > blo);
}

static bool rb_tex_hits(const r9800_raster_state_t *rs, Bit32u lo, Bit32u hi)
{
  return rb_rng_hit(rs->tex_lo, rs->tex_hi, lo, hi);
}

static void rb_prim_rows(const r9800_raster_state_t *rs, const r300_vtx_t *v, int n, Bit32s *y0, Bit32s *y1)
{
  float ymin = v[0].pos[1], ymax = v[0].pos[1];
  float ext = 0.0f;
  for (int k = 1; k < n; k++) {
    if (v[k].pos[1] < ymin) ymin = v[k].pos[1];
    if (v[k].pos[1] > ymax) ymax = v[k].pos[1];
  }
  if (n == 1) {
    ext = rs->d.point_hh;
    if ((v[0].psize > 0.0f) && (v[0].psize * 0.5f > ext)) ext = v[0].psize * 0.5f;
    if (rs->d.point_max * 0.5f > ext) ext = rs->d.point_max * 0.5f;
  } else if (n == 2) {
    ext = rs->d.line_hw;
  }
  if (!(ymin <= ymax) || (ymin < -65536.0f) || (ymax > 65536.0f)) {
    *y0 = -0x3fffffff;
    *y1 = 0x3fffffff;
    return;
  }
  *y0 = (Bit32s)floor(ymin - ext) - 2;
  *y1 = (Bit32s)ceil(ymax + ext) + 2;
}

static void rb_prim_ranges(const r9800_raster_state_t *rs, Bit32s y0, Bit32s y1,
                           Bit32u *clo, Bit32u *chi, Bit32u *zlo, Bit32u *zhi)
{
  const r300_draw_state_t *d = &rs->d;
  *clo = *zlo = 0xffffffff;
  *chi = *zhi = 0;
  if (y0 < d->sy0) y0 = d->sy0;
  if (y1 > d->sy1) y1 = d->sy1;
  if (y1 < y0)
    return;
  if (d->cb_ok) {
    Bit32u stride = d->cb_pitch_px * (Bit32u)d->cb_bpp;
    Bit32u tr = r300_tile_rows((Bit32u)d->cb_bpp, (Bit32u)d->cb_micro, (Bit32u)d->cb_macro);
    Bit32u lo = 0xffffffff, hi = 0;
    for (int t = 0; t < d->multiwrite; t++) {
      if (d->cb_vram[t] == 0xffffffffu) continue;
      Bit32u a = 0xffffffff, b = 0;
      rb_rows_to_bytes(d->cb_vram[t], stride, tr, y0, y1, &a, &b);
      if (b > a) { if (a < lo) lo = a; if (b > hi) hi = b; }
    }
    if (d->aa_resolve && (d->aa_samples > 1) && (d->aar_vram != 0xffffffffu)) {
      Bit32u a = 0xffffffff, b = 0;
      rb_rows_to_bytes(d->aar_vram, d->aar_pitch_px * (Bit32u)d->cb_bpp, tr, y0, y1, &a, &b);
      if (b > a) { if (a < lo) lo = a; if (b > hi) hi = b; }
    }
    *clo = lo; *chi = hi;
  }
  if (d->zb_ok && (d->z_en || d->sten_en)) {
    Bit32u stride = d->zb_pitch_px * (Bit32u)d->zb_bpp;
    Bit32u tr = r300_tile_rows((Bit32u)d->zb_bpp, (Bit32u)d->zb_micro, (Bit32u)d->zb_macro);
    rb_rows_to_bytes(d->zb_vram, stride, tr, y0 + d->zy_off, y1 + d->zy_off, zlo, zhi);
  }
}

Bit32u bx_radeon9800_c::rb_intern_state(const r9800_raster_state_t *rs)
{
  if ((rb.state_count > 0) && (rb.states[rb.state_count - 1].serial == rs->serial) &&
      (rb.states[rb.state_count - 1].stage_dead == rs->stage_dead) &&
      (memcmp(rb.states[rb.state_count - 1].tex, rs->tex, sizeof(rs->tex)) == 0))
    return rb.state_count - 1;
  if (rb.state_count == rb.state_cap) {
    rb.state_cap = rb.state_cap ? rb.state_cap * 2u : 64u;
    rb.states = (r9800_raster_state_t*)realloc(rb.states, rb.state_cap * sizeof(*rb.states));
  }
  rb.states[rb.state_count] = *rs;
  return rb.state_count++;
}

void bx_radeon9800_c::rb_guard_rt(const r9800_raster_state_t *rs)
{
  const r300_draw_state_t *d = &rs->d;
  bool zt = d->zb_ok && (d->z_en || d->sten_en);
  Bit32u cpitch = d->cb_pitch_px * (Bit32u)d->cb_bpp;
  Bit32u zpitch = d->zb_pitch_px * (Bit32u)d->zb_bpp;
  if (rb.rt_valid &&
      ((d->cb_ok && ((d->cb_vram[0] != rb.rt_c_off) || (cpitch != rb.rt_c_pitch) || ((Bit32u)d->cb_bpp != rb.rt_c_bpp))) ||
       (zt && rb.rt_z_valid && ((d->zb_vram != rb.rt_z_off) || (zpitch != rb.rt_z_pitch)))))
    raster_flush();
  if (d->cb_ok) {
    rb.rt_valid = true;
    rb.rt_c_off = d->cb_vram[0];
    rb.rt_c_pitch = cpitch;
    rb.rt_c_bpp = (Bit32u)d->cb_bpp;
  }
  if (zt) {
    rb.rt_z_valid = true;
    rb.rt_z_off = d->zb_vram;
    rb.rt_z_pitch = zpitch;
  }
}

void bx_radeon9800_c::rb_enqueue(const r9800_raster_state_t *rs, int kind, const r300_vtx_t *v, int n, int front)
{
  rb_cmd_t *cmd;
  Bit32u clo, chi, zlo, zhi;
  Bit32s y0, y1;

  rb_prim_rows(rs, v, n, &y0, &y1);
  rb_prim_ranges(rs, y0, y1, &clo, &chi, &zlo, &zhi);

  // self-feedback (render to a sampled texture): render inline
  if (rb_tex_hits(rs, clo, chi) || rb_tex_hits(rs, zlo, zhi)) {
    raster_flush();
    if (kind == RB_TRI)
      r3d_tri(rs, 0, 0, &v[0], &v[1], &v[2], front);
    else if (kind == RB_LINE)
      r3d_line(rs, 0, 0, &v[0], &v[1]);
    else
      r3d_point(rs, 0, 0, &v[0]);
    return;
  }
  // textures sampled from what the pending batch writes: flush first
  if (rb.wr_valid && (rb_tex_hits(rs, rb.wr_c_lo, rb.wr_c_hi) || rb_tex_hits(rs, rb.wr_z_lo, rb.wr_z_hi)))
    raster_flush();
  rb_guard_rt(rs);

  if (rb.cmd_count == rb.cmd_cap) {
    rb.cmd_cap = rb.cmd_cap ? rb.cmd_cap * 2u : 4096u;
    rb.cmds = (rb_cmd_t*)realloc(rb.cmds, rb.cmd_cap * sizeof(*rb.cmds));
  }
  cmd = &rb.cmds[rb.cmd_count++];
  cmd->state_idx = rb_intern_state(rs);
  cmd->kind = (Bit32u)kind;
  cmd->py0 = y0;
  cmd->py1 = y1;
  cmd->front = front;
  for (int k = 0; k < n; k++)
    cmd->v[k] = v[k];
  if (chi > clo) {
    if (clo < rb.wr_c_lo) rb.wr_c_lo = clo;
    if (chi > rb.wr_c_hi) rb.wr_c_hi = chi;
    rb.wr_valid = true;
  }
  if (zhi > zlo) {
    if (zlo < rb.wr_z_lo) rb.wr_z_lo = zlo;
    if (zhi > rb.wr_z_hi) rb.wr_z_hi = zhi;
    rb.wr_valid = true;
  }
  cp_batch_pending = 1;
}

void bx_radeon9800_c::raster_submit(const r9800_raster_state_t *rs, int kind, const r300_vtx_t *v, int n, int front)
{
  if (rs->stage_dead || !rs->d.draw_ok)
    return;
  if (rb.nthreads <= 1) {
    if (kind == RB_TRI)
      r3d_tri(rs, 0, 0, &v[0], &v[1], &v[2], front);
    else if (kind == RB_LINE)
      r3d_line(rs, 0, 0, &v[0], &v[1]);
    else
      r3d_point(rs, 0, 0, &v[0]);
    return;
  }
  rb_enqueue(rs, kind, v, n, front);
}

#endif // BX_SUPPORT_PCI && BX_SUPPORT_RADEON9800
