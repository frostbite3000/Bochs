/////////////////////////////////////////////////////////////////////////
// $Id$
/////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2004 Makoto Suzuki (suzu)
//                     Volker Ruppert (vruppert)
//                     Robin Kay (komadori)
//  Copyright (C) 2004-2025  The Bochs Project
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
#include "vgacore.h"
#define BX_USE_BINARY_FWD_ROP
#define BX_USE_BINARY_BKWD_ROP
#include "bitblt.h"
#include "ddc.h"
#include "pxextract.h"
#include "voodoorush.h"
#include "virt_timer.h"
#include "bxthread.h"

#include "bx_debug/debug.h"

#if BX_SUPPORT_VOODOO

#define LOG_THIS BX_VOODOORUSH_THIS

#if BX_USE_VOODOORUSH_SMF
#define VGA_READ(addr,len)       bx_vgacore_c::read_handler(theSvga,addr,len)
#define VGA_WRITE(addr,val,len)  bx_vgacore_c::write_handler(theSvga,addr,val,len)
#define SVGA_READ(addr,len)      svga_read_handler(theSvga,addr,len)
#define SVGA_WRITE(addr,val,len) svga_write_handler(theSvga,addr,val,len)
#else
#define VGA_READ(addr,len)       bx_vgacore_c::read(addr,len)
#define VGA_WRITE(addr,val,len)  bx_vgacore_c::write(addr,val,len)
#define SVGA_READ(addr,len)      svga_read(addr,len)
#define SVGA_WRITE(addr,val,len) svga_write(addr,val,len)
#endif // BX_USE_VOODOORUSH_SMF

// PCI 0x04: command(word), 0x06(word): status
#define PCI_COMMAND_IOACCESS                0x0001
#define PCI_COMMAND_MEMACCESS               0x0002
#define PCI_COMMAND_BUSMASTER               0x0004
#define PCI_COMMAND_SPECIALCYCLE            0x0008
#define PCI_COMMAND_MEMWRITEINVALID         0x0010
#define PCI_COMMAND_PALETTESNOOPING         0x0020
#define PCI_COMMAND_PARITYDETECTION         0x0040
#define PCI_COMMAND_ADDRESSDATASTEPPING     0x0080
#define PCI_COMMAND_SERR                    0x0100
#define PCI_COMMAND_BACKTOBACKTRANS         0x0200
#define  PCI_STATUS_INTERRUPT               0x0008
#define  PCI_STATUS_CAP_LIST                0x0010
#define  PCI_STATUS_66MHZ                   0x0020
#define  PCI_STATUS_UDF                     0x0040
#define  PCI_STATUS_FAST_BACK               0x0080
#define  PCI_STATUS_PARITY                  0x0100
#define  PCI_STATUS_DEVSEL_MASK             0x0600
#define  PCI_STATUS_DEVSEL_FAST             0x0000
#define  PCI_STATUS_DEVSEL_MEDIUM           0x0200
#define  PCI_STATUS_DEVSEL_SLOW             0x0400
#define  PCI_STATUS_SIG_TARGET_ABORT        0x0800
#define  PCI_STATUS_REC_TARGET_ABORT        0x1000
#define  PCI_STATUS_REC_MASTER_ABORT        0x2000
#define  PCI_STATUS_SIG_SYSTEM_ERROR        0x4000
#define  PCI_STATUS_DETECTED_PARITY         0x8000
// PCI 0x08, 0xff000000 (0x09-0x0b:class,0x08:rev)
#define PCI_CLASS_BASE_DISPLAY        0x03
// PCI 0x08, 0x00ff0000
#define PCI_CLASS_SUB_VGA             0x00
// PCI 0x0c, 0x00ff0000 (0x0c:cacheline,0x0d:latency,0x0e:headertype,0x0f:Built-in self test)
#define PCI_CLASS_HEADERTYPE_00h  0x00
// 0x10-0x3f (headertype 00h)
// PCI 0x10,0x14,0x18,0x1c,0x20,0x24: base address mapping registers
//   0x10: MEMBASE, 0x14: IOBASE(hard-coded in XFree86 3.x)
#define PCI_MAP_MEM                 0x0
#define PCI_MAP_IO                  0x1
#define PCI_MAP_MEM_ADDR_MASK       (~0xf)
#define PCI_MAP_IO_ADDR_MASK        (~0x3)
#define PCI_MAP_MEMFLAGS_32BIT      0x0
#define PCI_MAP_MEMFLAGS_32BIT_1M   0x1
#define PCI_MAP_MEMFLAGS_64BIT      0x4
#define PCI_MAP_MEMFLAGS_CACHEABLE  0x8
// PCI 0x28: cardbus CIS pointer
// PCI 0x2c: subsystem vendor id, 0x2e: subsystem id
// PCI 0x30: expansion ROM base address
// PCI 0x34: 0xffffff00=reserved, 0x000000ff=capabilities pointer
// PCI 0x38: reserved
// PCI 0x3c: 0x3c=int-line, 0x3d=int-pin, 0x3e=min-gnt, 0x3f=maax-lat
// PCI 0x40-0xff: device dependent fields

// default PnP memory and memory-mapped I/O sizes
#define VOODOORUSH_PNPMEM_SIZE          (16 << 20)
#define VOODOORUSH_PNPMMIO_SIZE         0x1000

const Bit8u voodoo_iomask[256] = {4,0,0,0,7,1,3,1,7,1,3,1,7,1,3,1,7,1,3,1,
  7,1,3,1,7,1,3,1,7,1,3,1,7,1,3,1,7,1,3,1,
  7,1,3,1,7,1,3,1,7,1,3,1,7,1,3,1,7,1,3,1,
  7,1,3,1,7,1,3,1,7,1,3,1,7,1,3,1,7,1,3,1,
  7,1,3,1,7,1,3,1,7,1,3,1,7,1,3,1,7,1,3,1,
  7,1,3,1,7,1,3,1,7,1,3,1,7,1,3,1,7,1,3,1,
  7,1,3,1,7,1,3,1,7,1,3,1,7,1,3,1,7,1,3,1,
  7,1,3,1,7,1,3,1,7,1,3,1,7,1,3,1,7,1,3,1,
  7,1,3,1,7,1,3,1,7,1,3,1,7,1,3,1,7,1,3,1,
  7,1,3,1,7,1,3,1,7,1,3,1,7,1,3,1,7,1,3,1,
  7,1,3,1,7,1,3,1,7,1,3,1,7,1,3,1,7,1,3,1,
  7,1,3,1,7,1,3,1,7,1,3,1,7,1,3,1,7,1,3,1,
  7,1,3,1,7,1,3,1,7,1,3,1,7,1,3,1};

static bx_voodoo_rush_c *theSvga = NULL;

#include "voodoo_types.h"
#include "voodoo_data.h"
#include "voodoo_main.h"
voodoo_state *v;
#include "voodoo_func.h"

PLUGIN_ENTRY_FOR_MODULE(voodoorush)
{
  if (mode == PLUGIN_INIT) {
    theSvga = new bx_voodoo_rush_c();
    bx_devices.pluginVgaDevice = theSvga;
    BX_REGISTER_DEVICE_DEVMODEL(plugin, type, theSvga, BX_PLUGIN_VOODOORUSH);
  } else if (mode == PLUGIN_FINI) {
    delete theSvga;
  } else if (mode == PLUGIN_PROBE) {
    return (int)PLUGTYPE_VGA;
  } else if (mode == PLUGIN_FLAGS) {
    return PLUGFLAG_PCI;
  }
  return 0; // Success
}

bx_voodoo_rush_c::bx_voodoo_rush_c() : bx_vgacore_c()
{
  // nothing else to do
}

bx_voodoo_rush_c::~bx_voodoo_rush_c()
{
  SIM->get_bochs_root()->remove("voodoorush");
  BX_DEBUG(("Exit"));
}

bool bx_voodoo_rush_c::init_vga_extension(void)
{
  BX_VOODOORUSH_THIS put("VOODOORUSH");
  // initialize SVGA stuffs.
  BX_VOODOORUSH_THIS bx_vgacore_c::init_iohandlers(svga_read_handler, svga_write_handler, "voodoorush");
  BX_VOODOORUSH_THIS pci_enabled = SIM->is_pci_device("voodoorush");
  BX_VOODOORUSH_THIS svga_init_members();
  BX_VOODOORUSH_THIS svga_init_pcihandlers();
  BX_VOODOORUSH_THIS s.max_xres = 1600;
  BX_VOODOORUSH_THIS s.max_yres = 1200;
#if BX_DEBUGGER
  // register device for the 'info device' command (calls debug_dump())
  bx_dbg_register_debug_info("voodoorush", this);
#endif
  return 1;
}

void bx_voodoo_rush_c::svga_init_members()
{
  unsigned i;

  // clear all registers.
  BX_VOODOORUSH_THIS sequencer.index = VOODOORUSH_SEQENCER_MAX + 1;
  for (i = 0; i <= VOODOORUSH_SEQENCER_MAX; i++)
    BX_VOODOORUSH_THIS sequencer.reg[i] = 0x00;
  BX_VOODOORUSH_THIS control.index = VOODOORUSH_CONTROL_MAX + 1;
  for (i = 0; i <= VOODOORUSH_CONTROL_MAX; i++)
    BX_VOODOORUSH_THIS control.reg[i] = 0x00;
  BX_VOODOORUSH_THIS control.shadow_reg0 = 0x00;
  BX_VOODOORUSH_THIS control.shadow_reg1 = 0x00;
  BX_VOODOORUSH_THIS crtc.index = VOODOORUSH_CRTC_MAX + 1;
  for (i = 0; i <= VOODOORUSH_CRTC_MAX; i++)
    BX_VOODOORUSH_THIS crtc.reg[i] = 0x00;
  BX_VOODOORUSH_THIS bx_vgacore_c::s.CRTC.max_reg = VOODOORUSH_CRTC_MAX;
  BX_VOODOORUSH_THIS hidden_dac.lockindex = 0;
  BX_VOODOORUSH_THIS hidden_dac.data = 0x00;

  BX_VOODOORUSH_THIS svga_unlock_special = 0;
  BX_VOODOORUSH_THIS svga_needs_update_tile = 1;
  BX_VOODOORUSH_THIS svga_needs_update_dispentire = 1;
  BX_VOODOORUSH_THIS svga_needs_update_mode = 0;
  BX_VOODOORUSH_THIS svga_double_width = 0;

  BX_VOODOORUSH_THIS svga_xres = 640;
  BX_VOODOORUSH_THIS svga_yres = 480;
  BX_VOODOORUSH_THIS svga_bpp = 8;
  BX_VOODOORUSH_THIS svga_pitch = 640;
  BX_VOODOORUSH_THIS bank_base[0] = 0;
  BX_VOODOORUSH_THIS bank_base[1] = 0;
  BX_VOODOORUSH_THIS bank_limit[0] = 0;
  BX_VOODOORUSH_THIS bank_limit[1] = 0;

  // svga_reset_bitblt(); // TODO: implement this function

  BX_VOODOORUSH_THIS hw_cursor.x = 0;
  BX_VOODOORUSH_THIS hw_cursor.y = 0;
  BX_VOODOORUSH_THIS hw_cursor.size = 0;

  // memory allocation.
  if (BX_VOODOORUSH_THIS bx_vgacore_c::s.memory == NULL)
    BX_VOODOORUSH_THIS bx_vgacore_c::s.memory = new Bit8u[VOODOORUSH_VIDEO_MEMORY_BYTES];

  BX_VOODOORUSH_THIS hidden_dac.lockindex = 5;
  BX_VOODOORUSH_THIS hidden_dac.data = 0;

  memset(BX_VOODOORUSH_THIS bx_vgacore_c::s.memory, 0xff, VOODOORUSH_VIDEO_MEMORY_BYTES);
  BX_VOODOORUSH_THIS disp_ptr = BX_VOODOORUSH_THIS bx_vgacore_c::s.memory;
  BX_VOODOORUSH_THIS memsize_mask = BX_VOODOORUSH_THIS bx_vgacore_c::s.memsize - 1;

  // VCLK defaults after reset
  BX_VOODOORUSH_THIS sequencer.reg[0x0b] = 0x66;
  BX_VOODOORUSH_THIS sequencer.reg[0x0c] = 0x5b;
  BX_VOODOORUSH_THIS sequencer.reg[0x0d] = 0x45;
  BX_VOODOORUSH_THIS sequencer.reg[0x0e] = 0x7e;
  BX_VOODOORUSH_THIS sequencer.reg[0x1b] = 0x3b;
  BX_VOODOORUSH_THIS sequencer.reg[0x1c] = 0x2f;
  BX_VOODOORUSH_THIS sequencer.reg[0x1d] = 0x30;
  BX_VOODOORUSH_THIS sequencer.reg[0x1e] = 0x33;
  BX_VOODOORUSH_THIS bx_vgacore_c::s.vclk[0] = 25180000;
  BX_VOODOORUSH_THIS bx_vgacore_c::s.vclk[1] = 28325000;
  BX_VOODOORUSH_THIS bx_vgacore_c::s.vclk[2] = 41165000;
  BX_VOODOORUSH_THIS bx_vgacore_c::s.vclk[3] = 36082000;
}

void bx_voodoo_rush_c::reset(unsigned type)
{
  // reset VGA stuffs.
  BX_VOODOORUSH_THIS bx_vgacore_c::reset(type);
  // reset SVGA stuffs.
  BX_VOODOORUSH_THIS svga_init_members();
  BX_VOODOORUSH_THIS ddc.init();
}

void bx_voodoo_rush_c::register_state(void)
{
  bx_list_c *list = new bx_list_c(SIM->get_bochs_root(), "voodoorush", "Voodoo Rush SVGA State");
  BX_VOODOORUSH_THIS vgacore_register_state(list);
  bx_list_c *crtc = new bx_list_c(list, "crtc");
  new bx_shadow_num_c(crtc, "index", &BX_VOODOORUSH_THIS crtc.index, BASE_HEX);
  new bx_shadow_data_c(crtc, "reg", BX_VOODOORUSH_THIS crtc.reg, VOODOORUSH_CRTC_MAX + 1, 1);
  bx_list_c *sequ = new bx_list_c(list, "sequencer");
  new bx_shadow_num_c(sequ, "index", &BX_VOODOORUSH_THIS sequencer.index, BASE_HEX);
  new bx_shadow_data_c(sequ, "reg", BX_VOODOORUSH_THIS sequencer.reg, VOODOORUSH_SEQENCER_MAX + 1, 1);
  bx_list_c *ctrl = new bx_list_c(list, "control");
  new bx_shadow_num_c(ctrl, "index", &BX_VOODOORUSH_THIS control.index, BASE_HEX);
  new bx_shadow_data_c(ctrl, "reg", BX_VOODOORUSH_THIS control.reg, VOODOORUSH_CONTROL_MAX + 1, 1);
  new bx_shadow_num_c(ctrl, "shadow_reg0", &BX_VOODOORUSH_THIS control.shadow_reg0, BASE_HEX);
  new bx_shadow_num_c(ctrl, "shadow_reg1", &BX_VOODOORUSH_THIS control.shadow_reg1, BASE_HEX);
  BXRS_PARAM_BOOL(list, clock_enabled, s.vdraw.clock_enabled);
  BXRS_PARAM_BOOL(list, output_on, s.vdraw.output_on);
  BXRS_PARAM_BOOL(list, override_on, s.vdraw.override_on);
  register_pci_state(list);
}

void bx_voodoo_rush_c::after_restore_state(void)
{
  bx_pci_device_c::after_restore_pci_state(voodoo_rush_mem_read_handler);
  BX_VOODOORUSH_THIS bx_vgacore_c::after_restore_state();
}

void bx_voodoo_rush_c::redraw_area(unsigned x0, unsigned y0, unsigned width,
                                   unsigned height)
{
  unsigned xti, yti, xt0, xt1, yt0, yt1;
  Bit8u iBpp = BX_VOODOORUSH_THIS svga_dispbpp;

  if (iBpp == 0x00) {
    BX_VOODOORUSH_THIS bx_vgacore_c::redraw_area(x0,y0,width,height);
    return;
  }

  if (BX_VOODOORUSH_THIS svga_needs_update_mode) {
    return;
  }

  BX_VOODOORUSH_THIS svga_needs_update_tile = 1;

  xt0 = x0 / X_TILESIZE;
  yt0 = y0 / Y_TILESIZE;
  if (x0 < BX_VOODOORUSH_THIS svga_xres) {
    xt1 = (x0 + width  - 1) / X_TILESIZE;
  } else {
    xt1 = (BX_VOODOORUSH_THIS svga_xres - 1) / X_TILESIZE;
  }
  if (y0 < BX_VOODOORUSH_THIS svga_yres) {
    yt1 = (y0 + height - 1) / Y_TILESIZE;
  } else {
    yt1 = (BX_VOODOORUSH_THIS svga_yres - 1) / Y_TILESIZE;
  }
  if ((x0 + width) > svga_xres) {
    BX_VOODOORUSH_THIS redraw_area(0, y0 + 1, x0 + width - svga_xres, height);
  }
  for (yti=yt0; yti<=yt1; yti++) {
    for (xti=xt0; xti<=xt1; xti++) {
      SET_TILE_UPDATED(BX_VOODOORUSH_THIS, xti, yti, 1);
    }
  }
}

bool bx_voodoo_rush_c::voodoo_rush_mem_read_handler(bx_phy_address addr, unsigned len,
                                        void *data, void *param)
{
  Bit32u val = voodoo_r((addr>>2) & 0x3FFFFF);
  if (len == 4)
    *(Bit32u*)data = val;
  else if ((len == 2) && ((addr & 3) != 3))
    *(Bit16u*)data = val >> ((addr & 3) * 8);
  else if (len == 1)
    *(Bit8u*)data = val >> ((addr & 3) * 8);
  else
    BX_ERROR(("Voodoo Rush mem_read_handler: unknown len=%d", len));

  return 1;
}

Bit8u bx_voodoo_rush_c::mem_read(bx_phy_address addr)
{
  if ((BX_VOODOORUSH_THIS pci_enabled) && (BX_VOODOORUSH_THIS pci_rom_size > 0)) {
    Bit32u mask = (BX_VOODOORUSH_THIS pci_rom_size - 1);
    if (((Bit32u)addr & ~mask) == BX_VOODOORUSH_THIS pci_rom_address) {
      if (BX_VOODOORUSH_THIS pci_conf[0x30] & 0x01) {
        return BX_VOODOORUSH_THIS pci_rom[addr & mask];
      } else {
        return 0xff;
      }
    }
  }

  return 0xFF;
}

bool bx_voodoo_rush_c::voodoo_rush_mem_write_handler(bx_phy_address addr, unsigned len,
                                         void *data, void *param)
{
  // TODO: Implement proper voodoo memory write handling
  return 1;
}

void bx_voodoo_rush_c::mem_write(bx_phy_address addr, Bit8u value)
{
  // For single byte writes, just pass to VGA core
  BX_VOODOORUSH_THIS bx_vgacore_c::mem_write(addr, value);
}

Bit32u bx_voodoo_rush_c::read_handler(void *this_ptr, Bit32u address, unsigned io_len)
{
  bx_voodoo_rush_c *class_ptr = (bx_voodoo_rush_c*)this_ptr;
  return class_ptr->read(address, io_len);
}

Bit32u bx_voodoo_rush_c::read(Bit32u address, unsigned io_len)
{
  static Bit8u lastreg = 0xff;
  Bit32u result;

  Bit8u offset = (Bit8u)(address & 0xff);
  Bit8u reg = (offset >> 2);
  switch (reg) {
    case 0xD0:
      {
        bool scl = address & 0x08;
        bool sda = address & 0x10;
        if (address == 0xD0) {
           ddc.write(scl, sda);
        }
        result = ddc.read() & 0x60;
        break;
      }

    default:
      break;
  }
  lastreg = reg;
  return result;
}

void bx_voodoo_rush_c::write_handler(void *this_ptr, Bit32u address, Bit32u value, unsigned io_len)
{
  bx_voodoo_rush_c *class_ptr = (bx_voodoo_rush_c*)this_ptr;
  return class_ptr->write(address, value, io_len);
}

void bx_voodoo_rush_c::write(Bit32u address, Bit32u value, unsigned io_len)
{
  Bit8u offset = (Bit8u)(address & 0xff);
  Bit8u reg = (offset>>2), dac_idx, fm, k, m, n, shift;
  Bit32u old, mask;
  Bit16u x0, y0;
  bool mode_change = false;

  switch (reg) {
    case 0xD0:
      {
        bool scl = address & 0x08;
        bool sda = address & 0x10;
        if (address == 0xD0) {
          ddc.write(scl, sda);
        }
        break;
      }

    default:
      break;
  }
}

void bx_voodoo_rush_c::get_text_snapshot(Bit8u **text_snapshot,
                                    unsigned *txHeight, unsigned *txWidth)
{
  BX_VOODOORUSH_THIS bx_vgacore_c::get_text_snapshot(text_snapshot,txHeight,txWidth);
}

Bit32u bx_voodoo_rush_c::svga_read_handler(void *this_ptr, Bit32u address, unsigned io_len)
{
#if !BX_USE_VOODOORUSH_SMF
  bx_voodoo_rush_c *class_ptr = (bx_voodoo_rush_c *) this_ptr;

  return class_ptr->svga_read(address, io_len);
}

Bit32u bx_voodoo_rush_c::svga_read(Bit32u address, unsigned io_len)
{
#else
  UNUSED(this_ptr);
#endif // !BX_USE_VOODOORUSH_SMF

  if ((io_len == 2) && ((address & 1) == 0)) {
    Bit32u value;
    value = (Bit32u)SVGA_READ(address,1);
    value |= (Bit32u)SVGA_READ(address+1,1) << 8;
    return value;
  }

  if (io_len != 1) {
    BX_PANIC(("SVGA read: io_len != 1"));
  }

  switch (address) {
    case 0x03b4: /* VGA: CRTC Index Register (monochrome emulation modes) */
    case 0x03d4: /* VGA: CRTC Index Register (color emulation modes) */
      return BX_VOODOORUSH_THIS crtc.index;
    case 0x03b5: /* VGA: CRTC Registers (monochrome emulation modes) */
    case 0x03d5: /* VGA: CRTC Registers (color emulation modes) */
      if (BX_VOODOORUSH_THIS is_unlocked())
        return BX_VOODOORUSH_THIS svga_read_crtc(address,BX_VOODOORUSH_THIS crtc.index);
      break;

    default:
      break;
  }

  return VGA_READ(address,io_len);
}

void bx_voodoo_rush_c::svga_write_handler(void *this_ptr, Bit32u address, Bit32u value, unsigned io_len)
{
#if !BX_USE_VOODOORUSH_SMF
  bx_voodoo_rush_c *class_ptr = (bx_voodoo_rush_c *) this_ptr;
  class_ptr->svga_write(address, value, io_len);
}

void bx_voodoo_rush_c::svga_write(Bit32u address, Bit32u value, unsigned io_len)
{
#else
  UNUSED(this_ptr);
#endif // !BX_USE_VOODOORUSH_SMF

  if ((io_len == 2) && ((address & 1) == 0)) {
    SVGA_WRITE(address,value & 0xff,1);
    SVGA_WRITE(address+1,value >> 8,1);
    return;
  }

  if (io_len != 1) {
    BX_PANIC(("SVGA write: io_len != 1"));
  }

  switch (address) {
    case 0x03b4: /* VGA: CRTC Index Register (monochrome emulation modes) */
    case 0x03d4: /* VGA: CRTC Index Register (color emulation modes) */
      BX_VOODOORUSH_THIS crtc.index = value & 0x3f;
      break;
    case 0x03b5: /* VGA: CRTC Registers (monochrome emulation modes) */
    case 0x03d5: /* VGA: CRTC Registers (color emulation modes) */
      if (BX_VOODOORUSH_THIS is_unlocked()) {
        BX_VOODOORUSH_THIS svga_write_crtc(address,BX_VOODOORUSH_THIS crtc.index,value);
        return;
      }
      break;
    default:
      break;
  }

  VGA_WRITE(address,value,io_len);
}

void bx_voodoo_rush_c::svga_modeupdate(void)
{
  Bit32u iTopOffset, iWidth, iHeight, vclock = 0;
  Bit8u iBpp, iDispBpp;
  bx_crtc_params_t crtcp;
  float hfreq, vfreq;

  iTopOffset = (BX_VOODOORUSH_THIS crtc.reg[0x0c] << 8)
       + BX_VOODOORUSH_THIS crtc.reg[0x0d]
       + ((BX_VOODOORUSH_THIS crtc.reg[0x1C] & 0x19) << 3)
       + ((BX_VOODOORUSH_THIS crtc.reg[0x1C] & 0x11) << 7);

  iHeight = BX_VOODOORUSH_THIS crtc.reg[6]
       + ((BX_VOODOORUSH_THIS crtc.reg[7] & 0x02) << 7)
       + ((BX_VOODOORUSH_THIS crtc.reg[7] & 0x40) << 3)
       + ((BX_VOODOORUSH_THIS crtc.reg[0x1A] & 0x01) << 5);

  iWidth = BX_VOODOORUSH_THIS crtc.reg[0] + ((BX_VOODOORUSH_THIS crtc.reg[0x1B] & 0x01) << 8) + 5;

  // Default to 8bpp for now - TODO: determine from registers
  iBpp = 8;

  switch (iBpp) {
    case 4:
      iDispBpp = 4;
      break;
    case 8:
      iDispBpp = 8;
      break;
    case 16:
      iDispBpp = 16;
      break;
    case 24:
      iDispBpp = 24;
      break;
    case 32:
      iDispBpp = 32;
      break;
    default:
      BX_PANIC(("unknown bpp"));
      break;
  }
  BX_VOODOORUSH_THIS get_crtc_params(&crtcp, &vclock);
  hfreq = vclock / (float)(crtcp.htotal * 8);
  vfreq = hfreq / (float)crtcp.vtotal;
  if ((BX_VOODOORUSH_THIS bx_vgacore_c::s.y_doublescan) && (iHeight > iWidth)) {
    iWidth <<= 1;
    BX_VOODOORUSH_THIS svga_double_width = true;
  } else {
    BX_VOODOORUSH_THIS svga_double_width = false;
  }
  if ((iWidth != BX_VOODOORUSH_THIS svga_xres) || (iHeight != BX_VOODOORUSH_THIS svga_yres)
      || (iDispBpp != BX_VOODOORUSH_THIS svga_dispbpp)) {
    if (!BX_VOODOORUSH_THIS bx_vgacore_c::s.ext_y_dblsize) {
      BX_INFO(("switched to %u x %u x %u @ %.1f Hz", iWidth, iHeight, iDispBpp,
               vfreq));
    } else {
      BX_INFO(("switched to %u x %u x %u @ %.1f Hz (interlaced)", iWidth,
               iHeight, iDispBpp, vfreq / 2.0f));
    }
  }
  BX_VOODOORUSH_THIS svga_xres = iWidth;
  BX_VOODOORUSH_THIS svga_yres = iHeight;
  BX_VOODOORUSH_THIS svga_bpp = iBpp;
  BX_VOODOORUSH_THIS svga_dispbpp = iDispBpp;
  BX_VOODOORUSH_THIS disp_ptr = BX_VOODOORUSH_THIS bx_vgacore_c::s.memory + iTopOffset;
  // compatibilty settings for VGA core
  BX_VOODOORUSH_THIS bx_vgacore_c::s.last_xres = iWidth;
  BX_VOODOORUSH_THIS bx_vgacore_c::s.last_yres = iHeight;
  BX_VOODOORUSH_THIS bx_vgacore_c::s.last_bpp = iDispBpp;
  BX_VOODOORUSH_THIS bx_vgacore_c::s.last_fh = 0;
}

void bx_voodoo_rush_c::update(void)
{
  unsigned width, height, pitch;

  /* skip screen update when the sequencer is in reset mode or video is disabled */
  if (! BX_VOODOORUSH_THIS bx_vgacore_c::s.sequencer.reset1 ||
      ! BX_VOODOORUSH_THIS bx_vgacore_c::s.sequencer.reset2 ||
      ! BX_VOODOORUSH_THIS bx_vgacore_c::s.attribute_ctrl.video_enabled) {
    return;
  }

  if (BX_VOODOORUSH_THIS svga_needs_update_mode) {
    BX_VOODOORUSH_THIS bx_vgacore_c::s.ext_y_dblsize = (BX_VOODOORUSH_THIS crtc.reg[0x1a] & 0x01) > 0;
  }
  if (BX_VOODOORUSH_THIS svga_needs_update_mode) {
    svga_modeupdate();
  }

  if (BX_VOODOORUSH_THIS svga_dispbpp != 4) {
    width  = BX_VOODOORUSH_THIS svga_xres;
    height = BX_VOODOORUSH_THIS svga_yres;
    pitch = BX_VOODOORUSH_THIS svga_pitch;
    if (BX_VOODOORUSH_THIS svga_needs_update_mode) {
      bx_gui->dimension_update(width, height, 0, 0, BX_VOODOORUSH_THIS svga_dispbpp);
      BX_VOODOORUSH_THIS bx_vgacore_c::s.last_bpp = BX_VOODOORUSH_THIS svga_dispbpp;
      BX_VOODOORUSH_THIS svga_needs_update_mode = 0;
      BX_VOODOORUSH_THIS svga_needs_update_dispentire = 1;
    }
  } else {
    BX_VOODOORUSH_THIS determine_screen_dimensions(&height, &width);
    pitch = BX_VOODOORUSH_THIS bx_vgacore_c::s.line_offset;
    if ((width != BX_VOODOORUSH_THIS bx_vgacore_c::s.last_xres) || (height != BX_VOODOORUSH_THIS bx_vgacore_c::s.last_yres) ||
        (BX_VOODOORUSH_THIS bx_vgacore_c::s.last_bpp > 8)) {
      bx_gui->dimension_update(width, height);
      BX_VOODOORUSH_THIS bx_vgacore_c::s.last_xres = width;
      BX_VOODOORUSH_THIS bx_vgacore_c::s.last_yres = height;
      BX_VOODOORUSH_THIS bx_vgacore_c::s.last_bpp = 8;
    }
  }

  if (BX_VOODOORUSH_THIS svga_needs_update_dispentire) {
    BX_VOODOORUSH_THIS redraw_area(0,0,width,height);
    BX_VOODOORUSH_THIS svga_needs_update_dispentire = 0;
  }

  if (!BX_VOODOORUSH_THIS svga_needs_update_tile) {
    return;
  }
  BX_VOODOORUSH_THIS svga_needs_update_tile = 0;

  unsigned xc, yc, xti, yti, hp;
  unsigned r, c, w, h, x, y;
  int i;
  Bit8u red, green, blue;
  Bit32u colour, row_addr;
  Bit8u * vid_ptr, * vid_ptr2;
  Bit8u * tile_ptr, * tile_ptr2;
  bx_svga_tileinfo_t info;

  if (bx_gui->graphics_tile_info_common(&info)) {
    if (info.snapshot_mode) {
      vid_ptr = BX_VOODOORUSH_THIS disp_ptr;
      tile_ptr = bx_gui->get_snapshot_buffer();
      if (tile_ptr != NULL) {
        for (yc = 0; yc < height; yc++) {
          vid_ptr2  = vid_ptr;
          tile_ptr2 = tile_ptr;
          if (BX_VOODOORUSH_THIS svga_dispbpp != 4) {
            for (xc = 0; xc < width; xc++) {
              memcpy(tile_ptr2, vid_ptr2, (BX_VOODOORUSH_THIS svga_bpp >> 3));
              if (!BX_VOODOORUSH_THIS svga_double_width || (xc & 1)) {
                vid_ptr2 += (BX_VOODOORUSH_THIS svga_bpp >> 3);
              }
              tile_ptr2 += ((info.bpp + 1) >> 3);
            }
            if (!BX_VOODOORUSH_THIS bx_vgacore_c::s.y_doublescan || (yc & 1)) {
              vid_ptr += pitch;
            }
          } else {
            row_addr = BX_VOODOORUSH_THIS bx_vgacore_c::s.CRTC.start_addr + (yc * pitch);
            for (xc = 0; xc < width; xc++) {
              *(tile_ptr2++) = BX_VOODOORUSH_THIS get_vga_pixel(xc, yc, row_addr, 0xffff, 0, BX_VOODOORUSH_THIS bx_vgacore_c::s.memory);
            }
          }
          tile_ptr += info.pitch;
        }
      }
    } else if (info.is_indexed) {
      switch (BX_VOODOORUSH_THIS svga_dispbpp) {
        case 4:
          for (yc=0, yti = 0; yc<height; yc+=Y_TILESIZE, yti++) {
            for (xc=0, xti = 0; xc<width; xc+=X_TILESIZE, xti++) {
              if (GET_TILE_UPDATED (xti, yti)) {
                tile_ptr = bx_gui->graphics_tile_get(xc, yc, &w, &h);
                for (r=0; r<h; r++) {
                  y = yc + r;
                  if (BX_VOODOORUSH_THIS bx_vgacore_c::s.y_doublescan) y >>= 1;
                  row_addr = BX_VOODOORUSH_THIS bx_vgacore_c::s.CRTC.start_addr + (y * pitch);
                  tile_ptr2 = tile_ptr;
                  for (c=0; c<w; c++) {
                    x = xc + c;
                    *(tile_ptr2++) = BX_VOODOORUSH_THIS get_vga_pixel(x, y, row_addr, 0xffff, 0, BX_VOODOORUSH_THIS bx_vgacore_c::s.memory);
                  }
                  tile_ptr += info.pitch;
                }
                bx_gui->graphics_tile_update_in_place(xc, yc, w, h);
                SET_TILE_UPDATED(BX_VOODOORUSH_THIS, xti, yti, 0);
              }
            }
          }
          break;
        case 15:
        case 16:
        case 24:
        case 32:
          BX_ERROR(("current guest pixel format is unsupported on indexed colour host displays, svga_dispbpp=%d",
            BX_VOODOORUSH_THIS svga_dispbpp));
          break;
        case 8:
          hp = BX_VOODOORUSH_THIS bx_vgacore_c::s.attribute_ctrl.horiz_pel_panning & 0x07;
          for (yc=0, yti = 0; yc<height; yc+=Y_TILESIZE, yti++) {
            for (xc=0, xti = 0; xc<width; xc+=X_TILESIZE, xti++) {
              if (GET_TILE_UPDATED (xti, yti)) {
                if (!BX_VOODOORUSH_THIS bx_vgacore_c::s.y_doublescan) {
                  vid_ptr = BX_VOODOORUSH_THIS disp_ptr + (yc * pitch + xc + hp);
                } else {
                  if (!BX_VOODOORUSH_THIS svga_double_width) {
                    vid_ptr = BX_VOODOORUSH_THIS disp_ptr + ((yc >> 1) * pitch + xc + hp);
                  } else {
                    vid_ptr = BX_VOODOORUSH_THIS disp_ptr + ((yc >> 1) * pitch + ((xc + hp) >> 1));
                  }
                }
                tile_ptr = bx_gui->graphics_tile_get(xc, yc, &w, &h);
                for (r=0; r<h; r++) {
                  vid_ptr2  = vid_ptr;
                  tile_ptr2 = tile_ptr;
                  for (c=0; c<w; c++) {
                    colour = *(vid_ptr2);
                    if (!BX_VOODOORUSH_THIS svga_double_width || (c & 1)) {
                      vid_ptr2++;
                    }
                    *(tile_ptr2++) = colour;
                  }
                  if (!BX_VOODOORUSH_THIS bx_vgacore_c::s.y_doublescan || (r & 1)) {
                    vid_ptr += pitch;
                  }
                  tile_ptr += info.pitch;
                }
                bx_gui->graphics_tile_update_in_place(xc, yc, w, h);
                SET_TILE_UPDATED(BX_VOODOORUSH_THIS, xti, yti, 0);
              }
            }
          }
          break;
      }
    }
    else {
      switch (BX_VOODOORUSH_THIS svga_dispbpp) {
        case 4:
          for (yc=0, yti=0; yc<height; yc+=Y_TILESIZE, yti++) {
            for (xc=0, xti=0; xc<width; xc+=X_TILESIZE, xti++) {
              if (GET_TILE_UPDATED (xti, yti)) {
                tile_ptr = bx_gui->graphics_tile_get(xc, yc, &w, &h);
                for (r=0; r<Y_TILESIZE; r++) {
                  tile_ptr2 = tile_ptr;
                  y = yc + r;
                  if (BX_VOODOORUSH_THIS bx_vgacore_c::s.y_doublescan) y >>= 1;
                  row_addr = BX_VOODOORUSH_THIS bx_vgacore_c::s.CRTC.start_addr + (y * pitch);
                  for (c=0; c<X_TILESIZE; c++) {
                    x = xc + c;
                    colour = BX_VOODOORUSH_THIS get_vga_pixel(x, y, row_addr, 0xffff, 0, BX_VOODOORUSH_THIS bx_vgacore_c::s.memory);
                    colour = MAKE_COLOUR(
                      BX_VOODOORUSH_THIS bx_vgacore_c::s.pel.data[colour].red, 6, info.red_shift, info.red_mask,
                      BX_VOODOORUSH_THIS bx_vgacore_c::s.pel.data[colour].green, 6, info.green_shift, info.green_mask,
                      BX_VOODOORUSH_THIS bx_vgacore_c::s.pel.data[colour].blue, 6, info.blue_shift, info.blue_mask);
                    if (info.is_little_endian) {
                      for (i=0; i<info.bpp; i+=8) {
                        *(tile_ptr2++) = colour >> i;
                      }
                    } else {
                      for (i=info.bpp-8; i>-8; i-=8) {
                        *(tile_ptr2++) = colour >> i;
                      }
                    }
                  }
                  tile_ptr += info.pitch;
                }
                bx_gui->graphics_tile_update_in_place(xc, yc, w, h);
                SET_TILE_UPDATED(BX_VOODOORUSH_THIS, xti, yti, 0);
              }
            }
          }
          break;
        case 8:
          hp = BX_VOODOORUSH_THIS bx_vgacore_c::s.attribute_ctrl.horiz_pel_panning & 0x07;
          for (yc=0, yti = 0; yc<height; yc+=Y_TILESIZE, yti++) {
            for (xc=0, xti = 0; xc<width; xc+=X_TILESIZE, xti++) {
              if (GET_TILE_UPDATED (xti, yti)) {
                if (!BX_VOODOORUSH_THIS bx_vgacore_c::s.y_doublescan) {
                  vid_ptr = BX_VOODOORUSH_THIS disp_ptr + (yc * pitch + xc + hp);
                } else {
                  if (!BX_VOODOORUSH_THIS svga_double_width) {
                    vid_ptr = BX_VOODOORUSH_THIS disp_ptr + ((yc >> 1) * pitch + xc + hp);
                  } else {
                    vid_ptr = BX_VOODOORUSH_THIS disp_ptr + ((yc >> 1) * pitch + ((xc + hp) >> 1));
                  }
                }
                tile_ptr = bx_gui->graphics_tile_get(xc, yc, &w, &h);
                for (r=0; r<h; r++) {
                  vid_ptr2  = vid_ptr;
                  tile_ptr2 = tile_ptr;
                  for (c=0; c<w; c++) {
                    colour = *(vid_ptr2);
                    if (!BX_VOODOORUSH_THIS svga_double_width || (c & 1)) {
                      vid_ptr2++;
                    }
                    colour = MAKE_COLOUR(
                      BX_VOODOORUSH_THIS bx_vgacore_c::s.pel.data[colour].red, 6, info.red_shift, info.red_mask,
                      BX_VOODOORUSH_THIS bx_vgacore_c::s.pel.data[colour].green, 6, info.green_shift, info.green_mask,
                      BX_VOODOORUSH_THIS bx_vgacore_c::s.pel.data[colour].blue, 6, info.blue_shift, info.blue_mask);
                    if (info.is_little_endian) {
                      for (i=0; i<info.bpp; i+=8) {
                        *(tile_ptr2++) = colour >> i;
                      }
                    } else {
                      for (i=info.bpp-8; i>-8; i-=8) {
                        *(tile_ptr2++) = colour >> i;
                      }
                    }
                  }
                  if (!BX_VOODOORUSH_THIS bx_vgacore_c::s.y_doublescan || (r & 1)) {
                    vid_ptr += pitch;
                  }
                  tile_ptr += info.pitch;
                }
                bx_gui->graphics_tile_update_in_place(xc, yc, w, h);
                SET_TILE_UPDATED(BX_VOODOORUSH_THIS, xti, yti, 0);
              }
            }
          }
          break;
        case 15:
          hp = BX_VOODOORUSH_THIS bx_vgacore_c::s.attribute_ctrl.horiz_pel_panning & 0x01;
          for (yc=0, yti = 0; yc<height; yc+=Y_TILESIZE, yti++) {
            for (xc=0, xti = 0; xc<width; xc+=X_TILESIZE, xti++) {
              if (GET_TILE_UPDATED (xti, yti)) {
                if (!BX_VOODOORUSH_THIS bx_vgacore_c::s.y_doublescan) {
                  vid_ptr = BX_VOODOORUSH_THIS disp_ptr + (yc * pitch + ((xc + hp) << 1));
                } else {
                  if (!BX_VOODOORUSH_THIS svga_double_width) {
                    vid_ptr = BX_VOODOORUSH_THIS disp_ptr + ((yc >> 1) * pitch + ((xc + hp) << 1));
                  } else {
                    vid_ptr = BX_VOODOORUSH_THIS disp_ptr + ((yc >> 1) * pitch + xc + (hp << 1));
                  }
                }
                tile_ptr = bx_gui->graphics_tile_get(xc, yc, &w, &h);
                for (r=0; r<h; r++) {
                  vid_ptr2  = vid_ptr;
                  tile_ptr2 = tile_ptr;
                  for (c=0; c<w; c++) {
                    colour = *(vid_ptr2);
                    colour |= *(vid_ptr2+1) << 8;
                    if (!BX_VOODOORUSH_THIS svga_double_width || (c & 1)) {
                      vid_ptr2 += 2;
                    }
                    if (info.bpp >= 24) {
                      EXTRACT_x555_TO_888(colour, red, green, blue);
                      colour = (red << 16) | (green << 8) | blue;
                    } else {
                      colour = MAKE_COLOUR(
                        colour & 0x001f, 5, info.blue_shift, info.blue_mask,
                        colour & 0x03e0, 10, info.green_shift, info.green_mask,
                        colour & 0x7c00, 15, info.red_shift, info.red_mask);
                    }
                    if (info.is_little_endian) {
                      for (i=0; i<info.bpp; i+=8) {
                        *(tile_ptr2++) = colour >> i;
                      }
                    } else {
                      for (i=info.bpp-8; i>-8; i-=8) {
                        *(tile_ptr2++) = colour >> i;
                      }
                    }
                  }
                  if (!BX_VOODOORUSH_THIS bx_vgacore_c::s.y_doublescan || (r & 1)) {
                    vid_ptr  += pitch;
                  }
                  tile_ptr += info.pitch;
                }
                bx_gui->graphics_tile_update_in_place(xc, yc, w, h);
                SET_TILE_UPDATED(BX_VOODOORUSH_THIS, xti, yti, 0);
              }
            }
          }
          break;
        case 16:
          hp = BX_VOODOORUSH_THIS bx_vgacore_c::s.attribute_ctrl.horiz_pel_panning & 0x01;
          for (yc=0, yti = 0; yc<height; yc+=Y_TILESIZE, yti++) {
            for (xc=0, xti = 0; xc<width; xc+=X_TILESIZE, xti++) {
              if (GET_TILE_UPDATED (xti, yti)) {
                if (!BX_VOODOORUSH_THIS bx_vgacore_c::s.y_doublescan) {
                  vid_ptr = BX_VOODOORUSH_THIS disp_ptr + (yc * pitch + ((xc + hp) << 1));
                } else {
                  if (!BX_VOODOORUSH_THIS svga_double_width) {
                    vid_ptr = BX_VOODOORUSH_THIS disp_ptr + ((yc >> 1) * pitch + ((xc + hp) << 1));
                  } else {
                    vid_ptr = BX_VOODOORUSH_THIS disp_ptr + ((yc >> 1) * pitch + xc + (hp << 1));
                  }
                }
                tile_ptr = bx_gui->graphics_tile_get(xc, yc, &w, &h);
                for (r=0; r<h; r++) {
                  vid_ptr2  = vid_ptr;
                  tile_ptr2 = tile_ptr;
                  for (c=0; c<w; c++) {
                    colour = *(vid_ptr2);
                    colour |= *(vid_ptr2+1) << 8;
                    if (!BX_VOODOORUSH_THIS svga_double_width || (c & 1)) {
                      vid_ptr2 += 2;
                    }
                    if (info.bpp >= 24) {
                      EXTRACT_565_TO_888(colour, red, green, blue);
                      colour = (red << 16) | (green << 8) | blue;
                    } else {
                      colour = MAKE_COLOUR(
                        colour & 0x001f, 5, info.blue_shift, info.blue_mask,
                        colour & 0x07e0, 11, info.green_shift, info.green_mask,
                        colour & 0xf800, 16, info.red_shift, info.red_mask);
                    }
                    if (info.is_little_endian) {
                      for (i=0; i<info.bpp; i+=8) {
                        *(tile_ptr2++) = colour >> i;
                      }
                    } else {
                      for (i=info.bpp-8; i>-8; i-=8) {
                        *(tile_ptr2++) = colour >> i;
                      }
                    }
                  }
                  if (!BX_VOODOORUSH_THIS bx_vgacore_c::s.y_doublescan || (r & 1)) {
                    vid_ptr  += pitch;
                  }
                  tile_ptr += info.pitch;
                }
                bx_gui->graphics_tile_update_in_place(xc, yc, w, h);
                SET_TILE_UPDATED(BX_VOODOORUSH_THIS, xti, yti, 0);
              }
            }
          }
          break;
        case 24:
          for (yc=0, yti = 0; yc<height; yc+=Y_TILESIZE, yti++) {
            for (xc=0, xti = 0; xc<width; xc+=X_TILESIZE, xti++) {
              if (GET_TILE_UPDATED (xti, yti)) {
                if (!BX_VOODOORUSH_THIS bx_vgacore_c::s.y_doublescan) {
                  vid_ptr = BX_VOODOORUSH_THIS disp_ptr + (yc * pitch + 3 * xc);
                } else {
                  if (!BX_VOODOORUSH_THIS svga_double_width) {
                    vid_ptr = BX_VOODOORUSH_THIS disp_ptr + ((yc >> 1) * pitch + 3 * xc);
                  } else {
                    vid_ptr = BX_VOODOORUSH_THIS disp_ptr + ((yc >> 1) * pitch + 3 * (xc >> 1));
                  }
                }
                tile_ptr = bx_gui->graphics_tile_get(xc, yc, &w, &h);
                for (r=0; r<h; r++) {
                  vid_ptr2  = vid_ptr;
                  tile_ptr2 = tile_ptr;
                  for (c=0; c<w; c++) {
                    blue = *(vid_ptr2);
                    green = *(vid_ptr2+1);
                    red = *(vid_ptr2+2);
                    if (!BX_VOODOORUSH_THIS svga_double_width || (c & 1)) {
                      vid_ptr2 += 3;
                    }
                    colour = MAKE_COLOUR(
                      red, 8, info.red_shift, info.red_mask,
                      green, 8, info.green_shift, info.green_mask,
                      blue, 8, info.blue_shift, info.blue_mask);
                    if (info.is_little_endian) {
                      for (i=0; i<info.bpp; i+=8) {
                        *(tile_ptr2++) = colour >> i;
                      }
                    } else {
                      for (i=info.bpp-8; i>-8; i-=8) {
                        *(tile_ptr2++) = colour >> i;
                      }
                    }
                  }
                  if (!BX_VOODOORUSH_THIS bx_vgacore_c::s.y_doublescan || (r & 1)) {
                    vid_ptr  += pitch;
                  }
                  tile_ptr += info.pitch;
                }
                bx_gui->graphics_tile_update_in_place(xc, yc, w, h);
                SET_TILE_UPDATED(BX_VOODOORUSH_THIS, xti, yti, 0);
              }
            }
          }
          break;
        case 32:
          for (yc=0, yti = 0; yc<height; yc+=Y_TILESIZE, yti++) {
            for (xc=0, xti = 0; xc<width; xc+=X_TILESIZE, xti++) {
              if (GET_TILE_UPDATED (xti, yti)) {
                if (!BX_VOODOORUSH_THIS bx_vgacore_c::s.y_doublescan) {
                  vid_ptr = BX_VOODOORUSH_THIS disp_ptr + (yc * pitch + (xc << 2));
                } else {
                  if (!BX_VOODOORUSH_THIS svga_double_width) {
                    vid_ptr = BX_VOODOORUSH_THIS disp_ptr + ((yc >> 1) * pitch + (xc << 2));
                  } else {
                    vid_ptr = BX_VOODOORUSH_THIS disp_ptr + ((yc >> 1) * pitch + (xc << 1));
                  }
                }
                tile_ptr = bx_gui->graphics_tile_get(xc, yc, &w, &h);
                for (r=0; r<h; r++) {
                  vid_ptr2  = vid_ptr;
                  tile_ptr2 = tile_ptr;
                  for (c=0; c<w; c++) {
                    blue = *(vid_ptr2);
                    green = *(vid_ptr2+1);
                    red = *(vid_ptr2+2);
                    if (!BX_VOODOORUSH_THIS svga_double_width || (c & 1)) {
                      vid_ptr2 += 4;
                    }
                    colour = MAKE_COLOUR(
                      red, 8, info.red_shift, info.red_mask,
                      green, 8, info.green_shift, info.green_mask,
                      blue, 8, info.blue_shift, info.blue_mask);
                    if (info.is_little_endian) {
                      for (i=0; i<info.bpp; i+=8) {
                        *(tile_ptr2++) = colour >> i;
                      }
                    } else {
                      for (i=info.bpp-8; i>-8; i-=8) {
                        *(tile_ptr2++) = colour >> i;
                      }
                    }
                  }
                  if (!BX_VOODOORUSH_THIS bx_vgacore_c::s.y_doublescan || (r & 1)) {
                    vid_ptr  += pitch;
                  }
                  tile_ptr += info.pitch;
                }
                bx_gui->graphics_tile_update_in_place(xc, yc, w, h);
                SET_TILE_UPDATED(BX_VOODOORUSH_THIS, xti, yti, 0);
              }
            }
          }
          break;
      }
    }
  }
  else {
    BX_PANIC(("cannot get svga tile info"));
  }
}

void bx_voodoo_rush_c::update_bank_ptr(Bit8u bank_index)
{
  unsigned offset;
  unsigned limit;

  if (BX_VOODOORUSH_THIS banking_is_dual())
    offset = BX_VOODOORUSH_THIS control.reg[0x09 + bank_index];
  else
    offset = BX_VOODOORUSH_THIS control.reg[0x09];

  if (BX_VOODOORUSH_THIS banking_granularity_is_16k())
    offset <<= 14;
  else
    offset <<= 12;

  if (BX_VOODOORUSH_THIS bx_vgacore_c::s.memsize <= offset) {
    limit = 0;
    BX_ERROR(("bank offset %08x is invalid",offset));
  } else {
    limit = BX_VOODOORUSH_THIS bx_vgacore_c::s.memsize - offset;
  }

  if (!BX_VOODOORUSH_THIS banking_is_dual() && (bank_index != 0)) {
    if (limit > 0x8000) {
      offset += 0x8000;
      limit -= 0x8000;
    } else {
      limit = 0;
    }
  }

  if (limit > 0) {
    BX_VOODOORUSH_THIS bank_base[bank_index] = offset;
    BX_VOODOORUSH_THIS bank_limit[bank_index] = limit;
    if ((BX_VOODOORUSH_THIS crtc.reg[0x1b] & 0x02) != 0) {
      BX_VOODOORUSH_THIS bx_vgacore_c::s.ext_offset = BX_VOODOORUSH_THIS bank_base[0];
    }
  } else {
    BX_VOODOORUSH_THIS bank_base[bank_index] = 0;
    BX_VOODOORUSH_THIS bank_limit[bank_index] = 0;
  }
}

Bit8u bx_voodoo_rush_c::svga_read_crtc(Bit32u address, unsigned index)
{
  switch (index) {
    case 0x00: // VGA
    case 0x01: // VGA
    case 0x02: // VGA
    case 0x03: // VGA
    case 0x04: // VGA
    case 0x05: // VGA
    case 0x06: // VGA
    case 0x07: // VGA
    case 0x08: // VGA
    case 0x09: // VGA
    case 0x0a: // VGA
    case 0x0b: // VGA
    case 0x0c: // VGA
    case 0x0d: // VGA
    case 0x0e: // VGA
    case 0x0f: // VGA
    case 0x10: // VGA
    case 0x11: // VGA
    case 0x12: // VGA
    case 0x13: // VGA
    case 0x14: // VGA
    case 0x15: // VGA
    case 0x16: // VGA
    case 0x17: // VGA
    case 0x18: // VGA
      break;
    case 0x19:
    case 0x1A:
    case 0x1B:
    case 0x1C:
      return VGA_READ(address,1);
      break;
    default:
      BX_DEBUG(("CRTC index 0x%02x is unknown(read)", index));
      break;
  }

  if (index <= VGA_CRTC_MAX) {
    return VGA_READ(address,1);
  }

  if (index <= VOODOORUSH_CRTC_MAX) {
    return BX_VOODOORUSH_THIS crtc.reg[index];
  }

  return 0xff;
}

void bx_voodoo_rush_c::svga_write_crtc(Bit32u address, unsigned index, Bit8u value)
{
  BX_DEBUG(("crtc: index 0x%02x write 0x%02x", index, (unsigned)value));

  bool update_pitch = 0;

  switch (index) {
    case 0x00: // VGA
    case 0x02: // VGA
    case 0x03: // VGA
    case 0x04: // VGA
    case 0x05: // VGA
    case 0x06: // VGA
    case 0x08: // VGA
    case 0x0a: // VGA
    case 0x0b: // VGA
    case 0x0e: // VGA
    case 0x0f: // VGA
    case 0x10: // VGA
    case 0x11: // VGA
    case 0x14: // VGA
    case 0x15: // VGA
    case 0x16: // VGA
    case 0x17: // VGA
    case 0x18: // VGA
      break;

    case 0x01: // VGA
    case 0x07: // VGA
    case 0x09: // VGA
    case 0x0c: // VGA (display offset 0x00ff00)
    case 0x0d: // VGA (display offset 0x0000ff)
    case 0x12: // VGA
    case 0x1A: // 0x01: interlaced video mode
    case 0x1D: // 0x80: offset 0x080000 (>=CLGD5434)
      BX_VOODOORUSH_THIS svga_needs_update_mode = 1;
      break;

    case 0x13: // VGA
    case 0x1B: // 0x01: offset 0x010000, 0x0c: offset 0x060000
      update_pitch = 1;
      break;

    case 0x19:
    case 0x1C:
      break;

    default:
      BX_DEBUG(("CRTC index 0x%02x is unknown(write 0x%02x)", index, (unsigned)value));
      return;
  }

  if (index <= VOODOORUSH_CRTC_MAX) {
    BX_VOODOORUSH_THIS crtc.reg[index] = value;
  }
  if (index <= VGA_CRTC_MAX) {
    VGA_WRITE(address,value,1);
  }

  if (update_pitch) {
    if ((BX_VOODOORUSH_THIS crtc.reg[0x1b] & 0x02) != 0) {
      BX_VOODOORUSH_THIS bx_vgacore_c::s.vgamem_mask = 0xfffff;
      BX_VOODOORUSH_THIS bx_vgacore_c::s.ext_start_addr = ((BX_VOODOORUSH_THIS crtc.reg[0x1b] & 0x01) << 16) |
                                        ((BX_VOODOORUSH_THIS crtc.reg[0x1b] & 0x04) << 15);
      BX_VOODOORUSH_THIS bx_vgacore_c::s.ext_offset = BX_VOODOORUSH_THIS bank_base[0];
    } else {
      BX_VOODOORUSH_THIS bx_vgacore_c::s.vgamem_mask = 0x3ffff;
      BX_VOODOORUSH_THIS bx_vgacore_c::s.ext_start_addr = 0;
      BX_VOODOORUSH_THIS bx_vgacore_c::s.ext_offset = 0;
    }
    BX_VOODOORUSH_THIS svga_pitch = (BX_VOODOORUSH_THIS crtc.reg[0x13] << 3) | ((BX_VOODOORUSH_THIS crtc.reg[0x1b] & 0x10) << 7);
    BX_VOODOORUSH_THIS svga_needs_update_mode = 1;
  }
}


/////////////////////////////////////////////////////////////////////////
//
// PCI support
//
/////////////////////////////////////////////////////////////////////////

void bx_voodoo_rush_c::svga_init_pcihandlers(void)
{
  Bit8u devfunc = 0x00;
  DEV_register_pci_handlers(BX_VOODOORUSH_THIS_PTR,
     &devfunc, "voodoorush", "Voodoo Rush PCI");

  // initialize readonly registers
  // if (s.model == VOODOO_RUSH) { // TODO: check model properly
  BX_VOODOORUSH_THIS init_pci_conf(0x1142, 0x643D, 0x02, 0x030000, 0x00, 0);
  // }
  BX_VOODOORUSH_THIS pci_conf[0x04] = (PCI_COMMAND_IOACCESS | PCI_COMMAND_MEMACCESS);
  BX_VOODOORUSH_THIS pci_conf[0x06] = (Bit8u)(PCI_STATUS_66MHZ | PCI_STATUS_DEVSEL_SLOW);

  BX_VOODOORUSH_THIS pci_conf[0x14] = 0x01;

  BX_VOODOORUSH_THIS init_bar_mem(0, 0x1000000, voodoo_rush_mem_read_handler, voodoo_rush_mem_write_handler);
  BX_VOODOORUSH_THIS init_bar_io(1, 256, read_handler, write_handler, voodoo_iomask);

  BX_VOODOORUSH_THIS pci_conf[0x2c] = 0x00;
  BX_VOODOORUSH_THIS pci_conf[0x2d] = 0x00;
  BX_VOODOORUSH_THIS pci_conf[0x2e] = 0x00;
  BX_VOODOORUSH_THIS pci_conf[0x2f] = 0x00;

  BX_VOODOORUSH_THIS pci_rom_address = 0;
  BX_VOODOORUSH_THIS pci_rom_read_handler = voodoo_rush_mem_read_handler;
  BX_VOODOORUSH_THIS load_pci_rom(SIM->get_param_string(BXPN_VGA_ROM_PATH)->getptr());
}

void bx_voodoo_rush_c::pci_write_handler(Bit8u address, Bit32u value, unsigned io_len)
{
  unsigned i;
  unsigned write_addr;
  Bit8u new_value, old_value;

  if ((address > 0x17) && (address < 0x30))
    return;

  BX_DEBUG_PCI_WRITE(address, value, io_len);
  for (i = 0; i < io_len; i++) {
    write_addr = address + i;
    old_value = BX_VOODOORUSH_THIS pci_conf[write_addr];
    new_value = (Bit8u)(value & 0xff);
    switch (write_addr) {
      case 0x04: // command bit0-7
        new_value &= PCI_COMMAND_IOACCESS | PCI_COMMAND_MEMACCESS;
        new_value |= old_value & ~(PCI_COMMAND_IOACCESS | PCI_COMMAND_MEMACCESS);
        break;
      case 0x05: // command bit8-15
        new_value = old_value;
        break;
      case 0x06: // status bit0-7
        new_value &= PCI_STATUS_66MHZ | PCI_STATUS_DEVSEL_SLOW;
        new_value |= old_value & ~(PCI_STATUS_66MHZ | PCI_STATUS_DEVSEL_SLOW);
        break;
      case 0x07: // status bit8-15
        new_value &= PCI_STATUS_66MHZ | PCI_STATUS_DEVSEL_SLOW;
        new_value |= old_value & ~(PCI_STATUS_66MHZ | PCI_STATUS_DEVSEL_SLOW);
        break;

      // read-only.
      case 0x00: case 0x01: // vendor
      case 0x02: case 0x03: // device
      case 0x08: // revision
      case 0x09: case 0x0a: case 0x0b: // class
      case 0x0e: // header type
      case 0x0f: // built-in self test(unimplemented)
        new_value = old_value;
        break;
      default:
        break;
    }
    BX_VOODOORUSH_THIS pci_conf[write_addr] = new_value;
    value >>= 8;
  }
}

#if BX_DEBUGGER
void bx_voodoo_rush_c::debug_dump(int argc, char **argv)
{
  bx_vgacore_c::debug_dump(argc, argv);
}
#endif

#endif // BX_SUPPORT_VOODOO
