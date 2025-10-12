/////////////////////////////////////////////////////////////////////////
// $Id$
/////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025  The Bochs Project
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
#include "pci.h"
#define BX_USE_BINARY_ROP
#include "bitblt.h"
#include "ddc.h"
#include "radeon.h"
#include "virt_timer.h"

#include "bx_debug/debug.h"

#if BX_SUPPORT_RADEON

#define LOG_THIS BX_RADEON_THIS

#if BX_USE_RADEON_SMF
#define VGA_READ(addr,len)       bx_vgacore_c::read_handler(theSvga,addr,len)
#define VGA_WRITE(addr,val,len)  bx_vgacore_c::write_handler(theSvga,addr,val,len)
#define SVGA_READ(addr,len)      svga_read_handler(theSvga,addr,len)
#define SVGA_WRITE(addr,val,len) svga_write_handler(theSvga,addr,val,len)
#else
#define VGA_READ(addr,len)       bx_vgacore_c::read(addr,len)
#define VGA_WRITE(addr,val,len)  bx_vgacore_c::write(addr,val,len)
#define SVGA_READ(addr,len)      svga_read(addr,len)
#define SVGA_WRITE(addr,val,len) svga_write(addr,val,len)
#endif // BX_USE_RADEON_SMF

const Bit8u radeon_iomask[256] = {4,0,0,0,7,1,3,1,7,1,3,1,7,1,3,1,7,1,3,1,
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

// PCI 0x00: vendor, 0x02: device
#define PCI_VENDOR_ATI                0x1002
#define PCI_DEVICE_RADEON8500         0x514C
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

#define STRAPS0_PRIMARY             0x00008BC1

static bx_radeon_c *theSvga = NULL;

PLUGIN_ENTRY_FOR_MODULE(radeon)
{
  if (mode == PLUGIN_INIT) {
    theSvga = new bx_radeon_c();
    bx_devices.pluginVgaDevice = theSvga;
    BX_REGISTER_DEVICE_DEVMODEL(plugin, type, theSvga, BX_PLUGIN_RADEON);
  } else if (mode == PLUGIN_FINI) {
    delete theSvga;
  } else if (mode == PLUGIN_PROBE) {
    return (int)PLUGTYPE_VGA;
  } else if (mode == PLUGIN_FLAGS) {
    return PLUGFLAG_PCI;
  }
  return 0; // Success
}

bx_radeon_c::bx_radeon_c() : bx_vgacore_c()
{
  // nothing else to do
}

bx_radeon_c::~bx_radeon_c()
{
  SIM->get_bochs_root()->remove("radeon");
  BX_DEBUG(("Exit"));
}

bool bx_radeon_c::init_vga_extension(void)
{
  const char* model = "Radeon 8500";
  if (!SIM->is_agp_device(BX_PLUGIN_RADEON)) {
    BX_PANIC(("%s should be plugged into AGP slot", model));
  }

  BX_RADEON_THIS pci_enabled = true;

  BX_RADEON_THIS put("RADEON");
  // initialize SVGA stuffs.
  DEV_register_ioread_handler(this, svga_read_handler, 0x03B4, "radeon", 2);
  DEV_register_ioread_handler(this, svga_read_handler, 0x03D0, "radeon", 7);
  DEV_register_ioread_handler(this, svga_read_handler, 0x03D2, "radeon", 2);
  DEV_register_ioread_handler(this, svga_read_handler, 0x03C3, "radeon", 3);
  DEV_register_iowrite_handler(this, svga_write_handler, 0x03D0, "radeon", 7);
  DEV_register_iowrite_handler(this, svga_write_handler, 0x03D2, "radeon", 2);
  BX_RADEON_THIS init_iohandlers(svga_read_handler, svga_write_handler);
  BX_RADEON_THIS svga_init_members();
  BX_RADEON_THIS svga_init_pcihandlers();
  BX_INFO(("%s initialized", model));
  BX_RADEON_THIS s.max_xres = 1600;
  BX_RADEON_THIS s.max_yres = 1200;
#if BX_DEBUGGER
  // register device for the 'info device' command (calls debug_dump())
  bx_dbg_register_debug_info("radeon", this);
#endif
  return 1;
}

void bx_radeon_c::svga_init_members()
{
  unsigned i;

  BX_RADEON_THIS svga_unlock_special = 0;
  BX_RADEON_THIS svga_needs_update_tile = 1;
  BX_RADEON_THIS svga_needs_update_dispentire = 1;
  BX_RADEON_THIS svga_needs_update_mode = 0;
  BX_RADEON_THIS svga_double_width = 0;

  BX_RADEON_THIS svga_xres = 640;
  BX_RADEON_THIS svga_yres = 480;
  BX_RADEON_THIS svga_bpp = 8;
  BX_RADEON_THIS svga_pitch = 640;
  BX_RADEON_THIS bank_base[0] = 0;
  BX_RADEON_THIS bank_base[1] = 0;

  BX_RADEON_THIS hw_cursor.x = 0;
  BX_RADEON_THIS hw_cursor.y = 0;
  BX_RADEON_THIS hw_cursor.size = 0;

  // memory allocation.
  if (BX_RADEON_THIS s.memory == NULL)
    BX_RADEON_THIS s.memory = new Bit8u[BX_RADEON_THIS s.memsize];
  memset(BX_RADEON_THIS s.memory, 0x00, BX_RADEON_THIS s.memsize);
  BX_RADEON_THIS disp_ptr = BX_RADEON_THIS s.memory;

  BX_RADEON_THIS s.memsize = 128 * 1024 * 1024;

  BX_RADEON_THIS hidden_dac.lockindex = 5;
  BX_RADEON_THIS hidden_dac.data = 0;

  BX_RADEON_THIS memsize_mask = BX_RADEON_THIS s.memsize - 1;

  // VCLK defaults after reset
  BX_RADEON_THIS sequencer.reg[0x0b] = 0x66;
  BX_RADEON_THIS sequencer.reg[0x0c] = 0x5b;
  BX_RADEON_THIS sequencer.reg[0x0d] = 0x45;
  BX_RADEON_THIS sequencer.reg[0x0e] = 0x7e;
  BX_RADEON_THIS sequencer.reg[0x1b] = 0x3b;
  BX_RADEON_THIS sequencer.reg[0x1c] = 0x2f;
  BX_RADEON_THIS sequencer.reg[0x1d] = 0x30;
  BX_RADEON_THIS sequencer.reg[0x1e] = 0x33;
  BX_RADEON_THIS s.vclk[0] = 25180000;
  BX_RADEON_THIS s.vclk[1] = 28325000;
  BX_RADEON_THIS s.vclk[2] = 41165000;
  BX_RADEON_THIS s.vclk[3] = 36082000;
}

void bx_radeon_c::reset(unsigned type)
{
  // reset VGA stuffs.
  BX_RADEON_THIS bx_vgacore_c::reset(type);
  // reset SVGA stuffs.
  BX_RADEON_THIS svga_init_members();
}

void bx_radeon_c::register_state(void)
{
  bx_list_c *list = new bx_list_c(SIM->get_bochs_root(), "radeon", "radeon State");
  BX_RADEON_THIS vgacore_register_state(list);
  bx_list_c *crtc = new bx_list_c(list, "crtc");
  new bx_shadow_num_c(crtc, "index", &BX_RADEON_THIS crtc.index, BASE_HEX);
  new bx_shadow_data_c(crtc, "reg", BX_RADEON_THIS crtc.reg, RADEON_CRTC_MAX + 1, 1);
  bx_list_c *sequ = new bx_list_c(list, "sequencer");
  new bx_shadow_num_c(sequ, "index", &BX_RADEON_THIS sequencer.index, BASE_HEX);
  new bx_shadow_data_c(sequ, "reg", BX_RADEON_THIS sequencer.reg, CIRRUS_SEQUENCER_MAX + 1, 1);
  bx_list_c *ctrl = new bx_list_c(list, "control");
  new bx_shadow_num_c(ctrl, "index", &BX_RADEON_THIS control.index, BASE_HEX);
  new bx_shadow_data_c(ctrl, "reg", BX_RADEON_THIS control.reg, CIRRUS_CONTROL_MAX + 1, 1);
  new bx_shadow_num_c(ctrl, "shadow_reg0", &BX_RADEON_THIS control.shadow_reg0, BASE_HEX);
  new bx_shadow_num_c(ctrl, "shadow_reg1", &BX_RADEON_THIS control.shadow_reg1, BASE_HEX);
  bx_list_c *hdac = new bx_list_c(list, "hidden_dac");
  new bx_shadow_num_c(hdac, "lockindex", &BX_RADEON_THIS hidden_dac.lockindex, BASE_HEX);
  new bx_shadow_num_c(hdac, "data", &BX_RADEON_THIS hidden_dac.data, BASE_HEX);
  new bx_shadow_data_c(hdac, "palette", BX_RADEON_THIS hidden_dac.palette, 48, 1);
  BXRS_PARAM_BOOL(list, svga_unlock_special, BX_RADEON_THIS svga_unlock_special);
  BXRS_PARAM_BOOL(list, svga_double_width, BX_RADEON_THIS svga_double_width);
  new bx_shadow_num_c(list, "svga_xres", &BX_RADEON_THIS svga_xres);
  new bx_shadow_num_c(list, "svga_yres", &BX_RADEON_THIS svga_yres);
  new bx_shadow_num_c(list, "svga_pitch", &BX_RADEON_THIS svga_pitch);
  new bx_shadow_num_c(list, "svga_bpp", &BX_RADEON_THIS svga_bpp);
  new bx_shadow_num_c(list, "svga_dispbpp", &BX_RADEON_THIS svga_dispbpp);
  new bx_shadow_num_c(list, "bank_base0", &BX_RADEON_THIS bank_base[0], BASE_HEX);
  new bx_shadow_num_c(list, "bank_base1", &BX_RADEON_THIS bank_base[1], BASE_HEX);
  new bx_shadow_num_c(list, "ext_latch0", &BX_RADEON_THIS ext_latch[0], BASE_HEX);
  new bx_shadow_num_c(list, "ext_latch1", &BX_RADEON_THIS ext_latch[1], BASE_HEX);
  new bx_shadow_num_c(list, "ext_latch2", &BX_RADEON_THIS ext_latch[2], BASE_HEX);
  new bx_shadow_num_c(list, "ext_latch3", &BX_RADEON_THIS ext_latch[3], BASE_HEX);
  bx_list_c *cursor = new bx_list_c(list, "hw_cursor");
  new bx_shadow_num_c(cursor, "x", &BX_RADEON_THIS hw_cursor.x, BASE_HEX);
  new bx_shadow_num_c(cursor, "y", &BX_RADEON_THIS hw_cursor.y, BASE_HEX);
  new bx_shadow_num_c(cursor, "size", &BX_RADEON_THIS hw_cursor.size, BASE_HEX);
  register_pci_state(list);
}

void bx_radeon_c::after_restore_state(void)
{
  bx_pci_device_c::after_restore_pci_state(radeon_mem_read_handler);
  BX_RADEON_THIS bx_vgacore_c::after_restore_state();
}

void bx_radeon_c::redraw_area(unsigned x0, unsigned y0, unsigned width,
                                   unsigned height)
{
  unsigned xti, yti, xt0, xt1, yt0, yt1;

  if (!BX_RADEON_THIS crtc.reg[0x28]) {
    BX_RADEON_THIS bx_vgacore_c::redraw_area(x0,y0,width,height);
    return;
  }

  if (BX_RADEON_THIS svga_needs_update_mode) {
    return;
  }

  BX_RADEON_THIS svga_needs_update_tile = 1;

  xt0 = x0 / X_TILESIZE;
  yt0 = y0 / Y_TILESIZE;
  if (x0 < BX_RADEON_THIS svga_xres) {
    xt1 = (x0 + width  - 1) / X_TILESIZE;
  } else {
    xt1 = (BX_RADEON_THIS svga_xres - 1) / X_TILESIZE;
  }
  if (y0 < BX_RADEON_THIS svga_yres) {
    yt1 = (y0 + height - 1) / Y_TILESIZE;
  } else {
    yt1 = (BX_RADEON_THIS svga_yres - 1) / Y_TILESIZE;
  }
  if ((x0 + width) > svga_xres) {
    BX_RADEON_THIS redraw_area(0, y0 + 1, x0 + width - svga_xres, height);
  }
  for (yti=yt0; yti<=yt1; yti++) {
    for (xti=xt0; xti<=xt1; xti++) {
      SET_TILE_UPDATED(BX_RADEON_THIS, xti, yti, 1);
    }
  }
}

bool bx_radeon_c::radeon_mem_read_handler(bx_phy_address addr, unsigned len,
                                        void *data, void *param)
{
  Bit8u *data_ptr;
#ifdef BX_LITTLE_ENDIAN
  data_ptr = (Bit8u *) data;
#else // BX_BIG_ENDIAN
  data_ptr = (Bit8u *) data + (len - 1);
#endif
  for (unsigned i = 0; i < len; i++) {
    *data_ptr = BX_RADEON_THIS mem_read(addr);
    addr++;
#ifdef BX_LITTLE_ENDIAN
    data_ptr++;
#else // BX_BIG_ENDIAN
    data_ptr--;
#endif
  }
  return 1;
}

Bit8u bx_radeon_c::mem_read(bx_phy_address addr)
{
  unsigned io_len;

  if ((BX_RADEON_THIS pci_enabled) && (BX_RADEON_THIS pci_rom_size > 0)) {
    Bit32u mask = (BX_RADEON_THIS pci_rom_size - 1);
    if (((Bit32u)addr & ~mask) == BX_RADEON_THIS pci_rom_address) {
      if (BX_RADEON_THIS pci_conf[0x30] & 0x01) {
        return BX_RADEON_THIS pci_rom[addr & mask];
      } else {
        return 0xff;
      }
    }
  }

  if ((addr >= BX_RADEON_THIS pci_bar[0].addr) &&
      (addr < (BX_RADEON_THIS pci_bar[0].addr + s.memsize))) {
    Bit32u offset = addr & BX_RADEON_THIS memsize_mask;
    return *(BX_RADEON_THIS s.memory + offset);
  }

  if ((addr >= BX_RADEON_THIS pci_bar[2].addr) &&
      (addr < (BX_RADEON_THIS pci_bar[2].addr + 0x10000))) {
    Bit32u offset = addr & (0x10000 - 1);
    Bit32u value = register_read(offset & ~3, io_len);
    int shift = (offset & 3) * 8;
    if (shift == 0)
      BX_ERROR(("MMIO read from 0x%08x, value 0x%08x", offset, value));
    return value >> shift;
  }

  if (!BX_RADEON_THIS dpdatatype)
    return BX_RADEON_THIS bx_vgacore_c::mem_read(addr);

  return 0xFF;
}

bool bx_radeon_c::radeon_mem_write_handler(bx_phy_address addr, unsigned len,
                                         void *data, void *param)
{
  Bit8u *data_ptr;
#ifdef BX_LITTLE_ENDIAN
  data_ptr = (Bit8u *) data;
#else // BX_BIG_ENDIAN
  data_ptr = (Bit8u *) data + (len - 1);
#endif
  for (unsigned i = 0; i < len; i++) {
    BX_RADEON_THIS mem_write(addr, *data_ptr);
    addr++;
#ifdef BX_LITTLE_ENDIAN
    data_ptr++;
#else // BX_BIG_ENDIAN
    data_ptr--;
#endif
  }
  return 1;
}

void bx_radeon_c::mem_write(bx_phy_address addr, Bit8u value)
{
  unsigned io_len;

  if ((addr >= BX_RADEON_THIS pci_bar[0].addr) &&
      (addr < (BX_RADEON_THIS pci_bar[0].addr + s.memsize))) {
    unsigned x, y;
    Bit32u offset = addr & BX_RADEON_THIS memsize_mask;
    *(BX_RADEON_THIS s.memory + offset) = value;
    BX_RADEON_THIS svga_needs_update_tile = 1;
    x = (offset % BX_RADEON_THIS svga_pitch) / (BX_RADEON_THIS svga_bpp / 8);
    y = offset / BX_RADEON_THIS svga_pitch;
    SET_TILE_UPDATED(BX_RADEON_THIS, x / X_TILESIZE, y / Y_TILESIZE, 1);
    return;
  }

  if ((addr >= BX_RADEON_THIS pci_bar[2].addr) &&
      (addr < (BX_RADEON_THIS pci_bar[2].addr + 0x10000))) {
    Bit32u offset = addr & (0x10000 - 1);
    Bit32u value32 = register_read(offset & ~3);
    int shift = (offset & 3) * 8;
    Bit32u mask = ~(0xFF << shift);
    value32 = (value32 & mask) | (value << shift);
    if (shift == 24)
      BX_ERROR(("MMIO write to 0x%08x, value 0x%08x", offset & ~3, value32));
    register_write(offset & ~3, value32, io_len);
    return;
  }

  if (!BX_RADEON_THIS dpdatatype) {
    BX_RADEON_THIS bx_vgacore_c::mem_write(addr, value);
    return;
  }

  if (addr >= 0xA0000 && addr <= 0xAFFFF) {
    Bit32u offset;
    unsigned x, y;

    offset = addr & 0xffff;
    offset += bank_base[0];
    offset &= BX_RADEON_THIS memsize_mask;
    *(BX_RADEON_THIS s.memory + offset) = value;
    BX_RADEON_THIS svga_needs_update_tile = 1;
    x = (offset % BX_RADEON_THIS svga_pitch) / (BX_RADEON_THIS svga_bpp / 8);
    y = offset / BX_RADEON_THIS svga_pitch;
    SET_TILE_UPDATED(BX_RADEON_THIS, x / X_TILESIZE, y / Y_TILESIZE, 1);
  }
}

void bx_radeon_c::get_text_snapshot(Bit8u **text_snapshot,
                                    unsigned *txHeight, unsigned *txWidth)
{
  BX_RADEON_THIS bx_vgacore_c::get_text_snapshot(text_snapshot,txHeight,txWidth);
}

Bit32u bx_radeon_c::svga_read_handler(void *this_ptr, Bit32u address, unsigned io_len)
{
#if !BX_USE_CIRRUS_SMF
  bx_radeon_c *class_ptr = (bx_radeon_c *) this_ptr;

  return class_ptr->svga_read(address, io_len);
}

Bit32u bx_radeon_c::svga_read(Bit32u address, unsigned io_len)
{
#else
  UNUSED(this_ptr);
#endif // !BX_USE_CIRRUS_SMF

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
      return BX_RADEON_THIS crtc.index;
    case 0x03b5: /* VGA: CRTC Registers (monochrome emulation modes) */
    case 0x03d5: /* VGA: CRTC Registers (color emulation modes) */
      if (BX_RADEON_THIS crtc.index > VGA_CRTC_MAX)
        return BX_RADEON_THIS svga_read_crtc(address,BX_RADEON_THIS crtc.index);
      else
        break;
    case 0x03c4: /* VGA: Sequencer Index Register */
      return BX_RADEON_THIS sequencer.index;
    case 0x03c5: /* VGA: Sequencer Registers */
      if (BX_RADEON_THIS sequencer.index > VGA_SEQENCER_MAX)
        BX_PANIC(("sequencer: unknown index 0x%02x read", BX_RADEON_THIS sequencer.index));
      break;
    case 0x03c6: /* Hidden DAC */
      break;
    case 0x03c8: /* PEL write address */
      BX_RADEON_THIS hidden_dac.lockindex = 0;
      break;
    case 0x03c9: /* PEL Data Register, hiddem pel colors 00..0F */
      break;
    case 0x03ce: /* VGA: Graphics Controller Index Register */
      return BX_RADEON_THIS control.index;
    case 0x03cf: /* VGA: Graphics Controller Registers */
      if (BX_RADEON_THIS control.index > VGA_CONTROL_MAX)
        BX_PANIC(("control: unknown index 0x%02x read", BX_RADEON_THIS control.index));
      break;
    default:
      break;
  }

  return VGA_READ(address,io_len);
}

void bx_radeon_c::svga_write_handler(void *this_ptr, Bit32u address, Bit32u value, unsigned io_len)
{
#if !BX_USE_CIRRUS_SMF
  bx_radeon_c *class_ptr = (bx_radeon_c *) this_ptr;
  class_ptr->svga_write(address, value, io_len);
}

void bx_radeon_c::svga_write(Bit32u address, Bit32u value, unsigned io_len)
{
#else
  UNUSED(this_ptr);
#endif // !BX_USE_CIRRUS_SMF

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
      BX_RADEON_THIS crtc.index = value & 0x3f;
      break;
    case 0x03b5: /* VGA: CRTC Registers (monochrome emulation modes) */
    case 0x03d5: /* VGA: CRTC Registers (color emulation modes) */
      if (BX_RADEON_THIS crtc.index <= VGA_CRTC_MAX) {
        BX_RADEON_THIS crtc.reg[BX_RADEON_THIS crtc.index] = value;
      } else {
        BX_RADEON_THIS svga_write_crtc(address, BX_RADEON_THIS crtc.index, value);
        return;
      }
      break;
    case 0x03c4: /* VGA: Sequencer Index Register */
      BX_RADEON_THIS sequencer.index = value;
      break;
    case 0x03c5: /* VGA: Sequencer Registers */
      if (BX_RADEON_THIS sequencer.index > VGA_SEQENCER_MAX)
        BX_PANIC(("sequencer: unknown index 0x%02x write", BX_RADEON_THIS sequencer.index));
      // BX_RADEON_THIS svga_write_sequencer(address,BX_RADEON_THIS sequencer.index,value);
      break;
    case 0x03c6: /* Hidden DAC */
      break;
    case 0x03c9: /* PEL Data Register, hidden pel colors 00..0F */
      break;
    case 0x03ce: /* VGA: Graphics Controller Index Register */
      BX_RADEON_THIS control.index = value;
      break;
    case 0x03cf: /* VGA: Graphics Controller Registers */
      if (BX_RADEON_THIS control.index > VGA_CONTROL_MAX)
        BX_PANIC(("control: unknown index 0x%02x write", BX_RADEON_THIS control.index));
      break;
    default:
      break;
  }

  VGA_WRITE(address,value,io_len);
}

void bx_radeon_c::draw_hardware_cursor(unsigned xc, unsigned yc, bx_svga_tileinfo_t *info)
{
  Bit16u hwcx = BX_RADEON_THIS hw_cursor.x;
  Bit16u hwcy = BX_RADEON_THIS hw_cursor.y;
  Bit16u size = BX_RADEON_THIS hw_cursor.size;

  if (BX_RADEON_THIS svga_double_width) {
    hwcx <<= 1; // FIXME: untested
  }
  if ((size > 0) &&
      (xc < (unsigned)(hwcx + size)) &&
      ((xc + X_TILESIZE) > hwcx) &&
      (yc < (unsigned)(hwcy + size)) &&
      ((yc + Y_TILESIZE) > hwcy)) {
    int i;
    unsigned w, h, pitch, cx, cy, cx0, cy0, cx1, cy1;

    Bit8u * tile_ptr, * tile_ptr2;
    Bit8u * plane0_ptr, *plane0_ptr2;
    Bit8u * plane1_ptr, *plane1_ptr2;
    unsigned long fgcol, bgcol;
    Bit32u hwc_offset;
    Bit64u plane0, plane1;

    cx0 = hwcx > xc ? hwcx : xc;
    cy0 = hwcy > yc ? hwcy : yc;
    cx1 = (unsigned)(hwcx + size) < (xc + X_TILESIZE) ? hwcx + size : xc + X_TILESIZE;
    cy1 = (unsigned)(hwcy + size) < (yc + Y_TILESIZE) ? hwcy + size : yc + Y_TILESIZE;

    if (info->bpp == 15) info->bpp = 16;
    tile_ptr = bx_gui->graphics_tile_get(xc, yc, &w, &h) +
               info->pitch * (cy0 - yc) + (info->bpp / 8) * (cx0 - xc);
    if (BX_RADEON_THIS svga_dispbpp == 4) {
      hwc_offset = 0x200000 - 16384; // VGA
    } else {
      hwc_offset = BX_RADEON_THIS memsize_mask - 16383; // Cirrus
    }
    plane0_ptr = BX_RADEON_THIS s.memory + hwc_offset;

    switch (size) {
      case 32:
        plane0_ptr += (BX_RADEON_THIS sequencer.reg[0x13] & 0x3f) * 256;
        plane1_ptr = plane0_ptr + 128;
        pitch = 4;
        break;

      case 64:
        plane0_ptr += (BX_RADEON_THIS sequencer.reg[0x13] & 0x3c) * 256;
        plane1_ptr = plane0_ptr + 8;
        pitch = 16;
        break;

      default:
        BX_ERROR(("unsupported hardware cursor size"));
        return;
        break;
    }

    if (!info->is_indexed) {
      fgcol = MAKE_COLOUR(
        BX_RADEON_THIS hidden_dac.palette[45], 6, info->red_shift, info->red_mask,
        BX_RADEON_THIS hidden_dac.palette[46], 6, info->green_shift, info->green_mask,
        BX_RADEON_THIS hidden_dac.palette[47], 6, info->blue_shift, info->blue_mask);
      bgcol = MAKE_COLOUR(
        BX_RADEON_THIS hidden_dac.palette[0], 6, info->red_shift, info->red_mask,
        BX_RADEON_THIS hidden_dac.palette[1], 6, info->green_shift, info->green_mask,
        BX_RADEON_THIS hidden_dac.palette[2], 6, info->blue_shift, info->blue_mask);
    } else {
      // FIXME: this is a hack that works in Windows guests
      // TODO: compare hidden DAC entries with DAC entries to find nearest match
      fgcol = (BX_RADEON_THIS svga_dispbpp == 4) ? 0x3f : 0xff;
      bgcol = 0x00;
    }

    plane0_ptr += pitch * (cy0 - hwcy);
    plane1_ptr += pitch * (cy0 - hwcy);
    for (cy=cy0; cy<cy1; cy++) {
      tile_ptr2 = tile_ptr + (info->bpp/8) * (cx1 - cx0) - 1;
      plane0_ptr2 = plane0_ptr;
      plane1_ptr2 = plane1_ptr;
      plane0 = plane1 = 0;
      for (i=0; i<size; i+=8) {
        plane0 = (plane0 << 8) | *(plane0_ptr2++);
        plane1 = (plane1 << 8) | *(plane1_ptr2++);
      }
      plane0 >>= hwcx+size - cx1;
      plane1 >>= hwcx+size - cx1;
      for (cx=cx0; cx<cx1; cx++) {
        if (plane0 & 1) {
          if (plane1 & 1) {
            if (info->is_little_endian) {
              for (i=info->bpp-8; i>-8; i-=8) {
                *(tile_ptr2--) = (Bit8u)(fgcol >> i);
              }
            } else {
              for (i=0; i<info->bpp; i+=8) {
                *(tile_ptr2--) = (Bit8u)(fgcol >> i);
              }
            }
          } else {
            for (i=0; i<info->bpp; i+=8) {
              *(tile_ptr2--) ^= 0xff;
            }
          }
        } else {
          if (plane1 & 1) {
            if (info->is_little_endian) {
              for (i=info->bpp-8; i>-8; i-=8) {
                *(tile_ptr2--) = (Bit8u)(bgcol >> i);
              }
            } else {
              for (i=0; i<info->bpp; i+=8) {
                *(tile_ptr2--) = (Bit8u)(bgcol >> i);
              }
            }
          } else {
            tile_ptr2 -= (info->bpp/8);
          }
        }
        plane0 >>= 1;
        plane1 >>= 1;
      }
      tile_ptr += info->pitch;
      plane0_ptr += pitch;
      plane1_ptr += pitch;
    }
  }
}

void bx_radeon_c::update(void)
{
  unsigned width, height, pitch;

  if (!BX_RADEON_THIS dpdatatype) {
    BX_RADEON_THIS bx_vgacore_c::update();
    return;
  }

  if (BX_RADEON_THIS svga_needs_update_mode) {
    if (BX_RADEON_THIS gencntl && BX_RADEON_THIS extcntl) {
      Bit8u iBpp;
      switch (BX_RADEON_THIS gencntl & BX_RADEON_THIS dpdatatype) {
        case 0x00000002:
          iBpp = 8;
          break;
        case 0x00000003:
          iBpp = 15;
          break;
        case 0x00000004:
          iBpp = 16;
          break;
        case 0x00000005:
          iBpp = 24;
          break;
        case 0x00000006:
          iBpp = 32;
          break;
        default:
          BX_PANIC(("unknown bpp"));
          break;
        }

      Bit32u iHeight = BX_RADEON_THIS hdisp; 
      Bit64u iWidth = BX_RADEON_THIS vdisp;
      determine_screen_dimensions(&iHeight, &iWidth);

      BX_INFO(("switched to %u x %u x %u", iWidth, iHeight, iBpp));

      BX_RADEON_THIS svga_xres = iWidth;
      BX_RADEON_THIS svga_yres = iHeight;
      BX_RADEON_THIS svga_bpp = iBpp;
      BX_RADEON_THIS svga_dispbpp = iBpp;
      BX_RADEON_THIS svga_pitch = svga_xres * iBpp / 8;
      // compatibilty settings for VGA core
      BX_RADEON_THIS s.last_xres = iWidth;
      BX_RADEON_THIS s.last_yres = iHeight;
      BX_RADEON_THIS s.last_bpp = iBpp;
      BX_RADEON_THIS s.last_fh = 0;
    }
  } 

  if (BX_RADEON_THIS svga_dispbpp != 4) {
    width  = BX_RADEON_THIS svga_xres;
    height = BX_RADEON_THIS svga_yres;
    pitch = BX_RADEON_THIS svga_pitch;
    if (BX_RADEON_THIS svga_needs_update_mode) {
      bx_gui->dimension_update(width, height, 0, 0, BX_RADEON_THIS svga_dispbpp);
      BX_RADEON_THIS s.last_bpp = BX_RADEON_THIS svga_dispbpp;
      BX_RADEON_THIS svga_needs_update_mode = 0;
      BX_RADEON_THIS svga_needs_update_dispentire = 1;
    }
  } else {
    BX_RADEON_THIS determine_screen_dimensions(&height, &width);
    pitch = BX_RADEON_THIS s.line_offset;
    if ((width != BX_RADEON_THIS s.last_xres) || (height != BX_RADEON_THIS s.last_yres) ||
        (BX_RADEON_THIS s.last_bpp > 8)) {
      bx_gui->dimension_update(width, height);
      BX_RADEON_THIS s.last_xres = width;
      BX_RADEON_THIS s.last_yres = height;
      BX_RADEON_THIS s.last_bpp = 8;
    }
  }

  if (BX_RADEON_THIS svga_needs_update_dispentire) {
    BX_RADEON_THIS redraw_area(0,0,width,height);
    BX_RADEON_THIS svga_needs_update_dispentire = 0;
  }

  if (!BX_RADEON_THIS svga_needs_update_tile) {
    return;
  }
  BX_RADEON_THIS svga_needs_update_tile = 0;

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
      vid_ptr = BX_RADEON_THIS disp_ptr;
      tile_ptr = bx_gui->get_snapshot_buffer();
      if (tile_ptr != NULL) {
        for (yc = 0; yc < height; yc++) {
          if (BX_RADEON_THIS svga_dispbpp != 4) {
            memcpy(tile_ptr, vid_ptr, info.pitch);
            vid_ptr += pitch;
          } else {
            tile_ptr2 = tile_ptr;
            row_addr = BX_RADEON_THIS s.CRTC.start_addr + (yc * pitch);
            for (xc = 0; xc < width; xc++) {
              *(tile_ptr2++) = BX_RADEON_THIS get_vga_pixel(xc, yc, row_addr, 0xffff, 0, BX_RADEON_THIS s.memory);
            }
          }
          tile_ptr += info.pitch;
        }
      }
    } else if (info.is_indexed) {
      switch (BX_RADEON_THIS svga_dispbpp) {
        case 4:
          for (yc=0, yti = 0; yc<height; yc+=Y_TILESIZE, yti++) {
            for (xc=0, xti = 0; xc<width; xc+=X_TILESIZE, xti++) {
              if (GET_TILE_UPDATED (xti, yti)) {
                tile_ptr = bx_gui->graphics_tile_get(xc, yc, &w, &h);
                for (r=0; r<h; r++) {
                  y = yc + r;
                  if (BX_RADEON_THIS s.y_doublescan) y >>= 1;
                  row_addr = BX_RADEON_THIS s.CRTC.start_addr + (y * pitch);
                  tile_ptr2 = tile_ptr;
                  for (c=0; c<w; c++) {
                    x = xc + c;
                    *(tile_ptr2++) = BX_RADEON_THIS get_vga_pixel(x, y, row_addr, 0xffff, 0, BX_RADEON_THIS s.memory);
                  }
                  tile_ptr += info.pitch;
                }
                draw_hardware_cursor(xc, yc, &info);
                bx_gui->graphics_tile_update_in_place(xc, yc, w, h);
                SET_TILE_UPDATED(BX_RADEON_THIS, xti, yti, 0);
              }
            }
          }
          break;
        case 15:
        case 16:
        case 24:
        case 32:
          BX_ERROR(("current guest pixel format is unsupported on indexed colour host displays, svga_dispbpp=%d",
            BX_RADEON_THIS svga_dispbpp));
          break;
        case 8:
          hp = BX_RADEON_THIS s.attribute_ctrl.horiz_pel_panning & 0x07;
          for (yc=0, yti = 0; yc<height; yc+=Y_TILESIZE, yti++) {
            for (xc=0, xti = 0; xc<width; xc+=X_TILESIZE, xti++) {
              if (GET_TILE_UPDATED (xti, yti)) {
                if (!BX_RADEON_THIS s.y_doublescan) {
                  vid_ptr = BX_RADEON_THIS disp_ptr + (yc * pitch + xc + hp);
                } else {
                  if (!BX_RADEON_THIS svga_double_width) {
                    vid_ptr = BX_RADEON_THIS disp_ptr + ((yc >> 1) * pitch + xc + hp);
                  } else {
                    vid_ptr = BX_RADEON_THIS disp_ptr + ((yc >> 1) * pitch + ((xc + hp) >> 1));
                  }
                }
                tile_ptr = bx_gui->graphics_tile_get(xc, yc, &w, &h);
                for (r=0; r<h; r++) {
                  vid_ptr2  = vid_ptr;
                  tile_ptr2 = tile_ptr;
                  for (c=0; c<w; c++) {
                    colour = *(vid_ptr2);
                    if (!BX_RADEON_THIS svga_double_width || (c & 1)) {
                      vid_ptr2++;
                    }
                    *(tile_ptr2++) = colour;
                  }
                  if (!BX_RADEON_THIS s.y_doublescan || (r & 1)) {
                    vid_ptr += pitch;
                  }
                  tile_ptr += info.pitch;
                }
                draw_hardware_cursor(xc, yc, &info);
                bx_gui->graphics_tile_update_in_place(xc, yc, w, h);
                SET_TILE_UPDATED(BX_RADEON_THIS, xti, yti, 0);
              }
            }
          }
          break;
      }
    }
    else {
      switch (BX_RADEON_THIS svga_dispbpp) {
        case 4:
          for (yc=0, yti=0; yc<height; yc+=Y_TILESIZE, yti++) {
            for (xc=0, xti=0; xc<width; xc+=X_TILESIZE, xti++) {
              if (GET_TILE_UPDATED (xti, yti)) {
                tile_ptr = bx_gui->graphics_tile_get(xc, yc, &w, &h);
                for (r=0; r<Y_TILESIZE; r++) {
                  tile_ptr2 = tile_ptr;
                  y = yc + r;
                  if (BX_RADEON_THIS s.y_doublescan) y >>= 1;
                  row_addr = BX_RADEON_THIS s.CRTC.start_addr + (y * pitch);
                  for (c=0; c<X_TILESIZE; c++) {
                    x = xc + c;
                    colour = BX_RADEON_THIS get_vga_pixel(x, y, row_addr, 0xffff, 0, BX_RADEON_THIS s.memory);
                    colour = MAKE_COLOUR(
                      BX_RADEON_THIS s.pel.data[colour].red, 6, info.red_shift, info.red_mask,
                      BX_RADEON_THIS s.pel.data[colour].green, 6, info.green_shift, info.green_mask,
                      BX_RADEON_THIS s.pel.data[colour].blue, 6, info.blue_shift, info.blue_mask);
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
                draw_hardware_cursor(xc, yc, &info);
                bx_gui->graphics_tile_update_in_place(xc, yc, w, h);
                SET_TILE_UPDATED(BX_RADEON_THIS, xti, yti, 0);
              }
            }
          }
          break;
        case 8:
          hp = BX_RADEON_THIS s.attribute_ctrl.horiz_pel_panning & 0x07;
          for (yc=0, yti = 0; yc<height; yc+=Y_TILESIZE, yti++) {
            for (xc=0, xti = 0; xc<width; xc+=X_TILESIZE, xti++) {
              if (GET_TILE_UPDATED (xti, yti)) {
                if (!BX_RADEON_THIS s.y_doublescan) {
                  vid_ptr = BX_RADEON_THIS disp_ptr + (yc * pitch + xc + hp);
                } else {
                  if (!BX_RADEON_THIS svga_double_width) {
                    vid_ptr = BX_RADEON_THIS disp_ptr + ((yc >> 1) * pitch + xc + hp);
                  } else {
                    vid_ptr = BX_RADEON_THIS disp_ptr + ((yc >> 1) * pitch + ((xc + hp) >> 1));
                  }
                }
                tile_ptr = bx_gui->graphics_tile_get(xc, yc, &w, &h);
                for (r=0; r<h; r++) {
                  vid_ptr2  = vid_ptr;
                  tile_ptr2 = tile_ptr;
                  for (c=0; c<w; c++) {
                    colour = *(vid_ptr2);
                    if (!BX_RADEON_THIS svga_double_width || (c & 1)) {
                      vid_ptr2++;
                    }
                    colour = MAKE_COLOUR(
                      BX_RADEON_THIS s.pel.data[colour].red, 6, info.red_shift, info.red_mask,
                      BX_RADEON_THIS s.pel.data[colour].green, 6, info.green_shift, info.green_mask,
                      BX_RADEON_THIS s.pel.data[colour].blue, 6, info.blue_shift, info.blue_mask);
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
                  if (!BX_RADEON_THIS s.y_doublescan || (r & 1)) {
                    vid_ptr += pitch;
                  }
                  tile_ptr += info.pitch;
                }
                draw_hardware_cursor(xc, yc, &info);
                bx_gui->graphics_tile_update_in_place(xc, yc, w, h);
                SET_TILE_UPDATED(BX_RADEON_THIS, xti, yti, 0);
              }
            }
          }
          break;
        case 15:
          hp = BX_RADEON_THIS s.attribute_ctrl.horiz_pel_panning & 0x01;
          for (yc=0, yti = 0; yc<height; yc+=Y_TILESIZE, yti++) {
            for (xc=0, xti = 0; xc<width; xc+=X_TILESIZE, xti++) {
              if (GET_TILE_UPDATED (xti, yti)) {
                if (!BX_RADEON_THIS s.y_doublescan) {
                  vid_ptr = BX_RADEON_THIS disp_ptr + (yc * pitch + ((xc + hp) << 1));
                } else {
                  if (!BX_RADEON_THIS svga_double_width) {
                    vid_ptr = BX_RADEON_THIS disp_ptr + ((yc >> 1) * pitch + ((xc + hp) << 1));
                  } else {
                    vid_ptr = BX_RADEON_THIS disp_ptr + ((yc >> 1) * pitch + xc + (hp << 1));
                  }
                }
                tile_ptr = bx_gui->graphics_tile_get(xc, yc, &w, &h);
                for (r=0; r<h; r++) {
                  vid_ptr2  = vid_ptr;
                  tile_ptr2 = tile_ptr;
                  for (c=0; c<w; c++) {
                    colour = *(vid_ptr2);
                    colour |= *(vid_ptr2+1) << 8;
                    if (!BX_RADEON_THIS svga_double_width || (c & 1)) {
                      vid_ptr2 += 2;
                    }
                    colour = MAKE_COLOUR(
                      colour & 0x001f, 5, info.blue_shift, info.blue_mask,
                      colour & 0x03e0, 10, info.green_shift, info.green_mask,
                      colour & 0x7c00, 15, info.red_shift, info.red_mask);
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
                  if (!BX_RADEON_THIS s.y_doublescan || (r & 1)) {
                    vid_ptr  += pitch;
                  }
                  tile_ptr += info.pitch;
                }
                draw_hardware_cursor(xc, yc, &info);
                bx_gui->graphics_tile_update_in_place(xc, yc, w, h);
                SET_TILE_UPDATED(BX_RADEON_THIS, xti, yti, 0);
              }
            }
          }
          break;
        case 16:
          hp = BX_RADEON_THIS s.attribute_ctrl.horiz_pel_panning & 0x01;
          for (yc=0, yti = 0; yc<height; yc+=Y_TILESIZE, yti++) {
            for (xc=0, xti = 0; xc<width; xc+=X_TILESIZE, xti++) {
              if (GET_TILE_UPDATED (xti, yti)) {
                if (!BX_RADEON_THIS s.y_doublescan) {
                  vid_ptr = BX_RADEON_THIS disp_ptr + (yc * pitch + ((xc + hp) << 1));
                } else {
                  if (!BX_RADEON_THIS svga_double_width) {
                    vid_ptr = BX_RADEON_THIS disp_ptr + ((yc >> 1) * pitch + ((xc + hp) << 1));
                  } else {
                    vid_ptr = BX_RADEON_THIS disp_ptr + ((yc >> 1) * pitch + xc + (hp << 1));
                  }
                }
                tile_ptr = bx_gui->graphics_tile_get(xc, yc, &w, &h);
                for (r=0; r<h; r++) {
                  vid_ptr2  = vid_ptr;
                  tile_ptr2 = tile_ptr;
                  for (c=0; c<w; c++) {
                    colour = *(vid_ptr2);
                    colour |= *(vid_ptr2+1) << 8;
                    if (!BX_RADEON_THIS svga_double_width || (c & 1)) {
                      vid_ptr2 += 2;
                    }
                    colour = MAKE_COLOUR(
                      colour & 0x001f, 5, info.blue_shift, info.blue_mask,
                      colour & 0x07e0, 11, info.green_shift, info.green_mask,
                      colour & 0xf800, 16, info.red_shift, info.red_mask);
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
                  if (!BX_RADEON_THIS s.y_doublescan || (r & 1)) {
                    vid_ptr  += pitch;
                  }
                  tile_ptr += info.pitch;
                }
                draw_hardware_cursor(xc, yc, &info);
                bx_gui->graphics_tile_update_in_place(xc, yc, w, h);
                SET_TILE_UPDATED(BX_RADEON_THIS, xti, yti, 0);
              }
            }
          }
          break;
        case 24:
          for (yc=0, yti = 0; yc<height; yc+=Y_TILESIZE, yti++) {
            for (xc=0, xti = 0; xc<width; xc+=X_TILESIZE, xti++) {
              if (GET_TILE_UPDATED (xti, yti)) {
                if (!BX_RADEON_THIS s.y_doublescan) {
                  vid_ptr = BX_RADEON_THIS disp_ptr + (yc * pitch + 3 * xc);
                } else {
                  if (!BX_RADEON_THIS svga_double_width) {
                    vid_ptr = BX_RADEON_THIS disp_ptr + ((yc >> 1) * pitch + 3 * xc);
                  } else {
                    vid_ptr = BX_RADEON_THIS disp_ptr + ((yc >> 1) * pitch + 3 * (xc >> 1));
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
                    if (!BX_RADEON_THIS svga_double_width || (c & 1)) {
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
                  if (!BX_RADEON_THIS s.y_doublescan || (r & 1)) {
                    vid_ptr  += pitch;
                  }
                  tile_ptr += info.pitch;
                }
                draw_hardware_cursor(xc, yc, &info);
                bx_gui->graphics_tile_update_in_place(xc, yc, w, h);
                SET_TILE_UPDATED(BX_RADEON_THIS, xti, yti, 0);
              }
            }
          }
          break;
        case 32:
          for (yc=0, yti = 0; yc<height; yc+=Y_TILESIZE, yti++) {
            for (xc=0, xti = 0; xc<width; xc+=X_TILESIZE, xti++) {
              if (GET_TILE_UPDATED (xti, yti)) {
                if (!BX_RADEON_THIS s.y_doublescan) {
                  vid_ptr = BX_RADEON_THIS disp_ptr + (yc * pitch + (xc << 2));
                } else {
                  if (!BX_RADEON_THIS svga_double_width) {
                    vid_ptr = BX_RADEON_THIS disp_ptr + ((yc >> 1) * pitch + (xc << 2));
                  } else {
                    vid_ptr = BX_RADEON_THIS disp_ptr + ((yc >> 1) * pitch + (xc << 1));
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
                    if (!BX_RADEON_THIS svga_double_width || (c & 1)) {
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
                  if (!BX_RADEON_THIS s.y_doublescan || (r & 1)) {
                    vid_ptr  += pitch;
                  }
                  tile_ptr += info.pitch;
                }
                draw_hardware_cursor(xc, yc, &info);
                bx_gui->graphics_tile_update_in_place(xc, yc, w, h);
                SET_TILE_UPDATED(BX_RADEON_THIS, xti, yti, 0);
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

Bit8u bx_radeon_c::svga_read_crtc(Bit32u address, unsigned index)
{
  if (index <= RADEON_CRTC_MAX) {
    Bit8u value = BX_RADEON_THIS crtc.reg[index];
    BX_ERROR(("crtc: index 0x%02x read 0x%02x", index, value));
    return value;
  }
  else
    BX_PANIC(("crtc: unknown index 0x%02x read", index));

  return 0xff;
}

void bx_radeon_c::svga_write_crtc(Bit32u address, unsigned index, Bit8u value)
{
  BX_ERROR(("crtc: index 0x%02x write 0x%02x", index, (unsigned)value));

  if (index == 0x19) {
    Bit32u offset = 
      (BX_RADEON_THIS crtc.reg[0x0c] << 8) +
      BX_RADEON_THIS crtc.reg[0x0d] +
      ((value & 0x1F) << 16);
    offset <<= 2;
    BX_RADEON_THIS disp_ptr = BX_RADEON_THIS s.memory + offset;
  }
  else if (index == 0x1d || index == 0x1e) {
    BX_RADEON_THIS bank_base[index - 0x1d] = value * 0x8000;
  } else if (index == 0x28 && value != 0x00) {
    BX_RADEON_THIS svga_needs_update_mode = 1;
  }

  if (index <= RADEON_CRTC_MAX) {
    BX_RADEON_THIS crtc.reg[index] = value;
  }
  else
    BX_PANIC(("crtc: unknown index 0x%02x write", index));
}

Bit32u bx_radeon_c::read_handler(void *this_ptr, Bit32u address, unsigned io_len)
{
  bx_radeon_c *class_ptr = (bx_radeon_c*)this_ptr;
  return class_ptr->register_read(address, io_len);
}

Bit32u bx_radeon_c::register_read(Bit32u address, unsigned io_len)
{
  Bit32u value = 0xFFFFFFFF;

  if (address == 0x0050)
    value = BX_RADEON_THIS gencntl;
  else if (address == 0x0054)
    value = BX_RADEON_THIS extcntl;
  else if (address == 0x0200)
    value = BX_RADEON_THIS hdisp;
  else if (address == 0x0208)
    value = BX_RADEON_THIS vdisp;
  else if (address == 0x16C4)
    value = BX_RADEON_THIS dpdatatype;
  else if (address >= 0x0010 && address < 0x0011) {
    Bit32u offset = address - 0x0010;
    value = 
      (BX_RADEON_THIS pci_rom[offset + 0] << 0) |
      (BX_RADEON_THIS pci_rom[offset + 1] << 8) |
      (BX_RADEON_THIS pci_rom[offset + 2] << 16) |
      (BX_RADEON_THIS pci_rom[offset + 3] << 24);
  } else if (address >= 0x0018 && address < 0x0019) {
    Bit32u offset = address - 0x0018;
    value = 
      (BX_RADEON_THIS pci_conf[offset + 0] << 0) |
      (BX_RADEON_THIS pci_conf[offset + 1] << 8) |
      (BX_RADEON_THIS pci_conf[offset + 2] << 16) |
      (BX_RADEON_THIS pci_conf[offset + 3] << 24);
  }
  else if (address >= 0x0058 && address < 0x007c) {
    value = BX_RADEON_THIS ramdac[(address - 0x0058) / 4];
  }

  return value;
}

void bx_radeon_c::write_handler(void *this_ptr, Bit32u address, Bit32u value, unsigned io_len)
{
  bx_radeon_c *class_ptr = (bx_radeon_c*)this_ptr;
  return class_ptr->register_write(address, value, io_len);
}

void bx_radeon_c::register_write(Bit32u address, Bit32u value, unsigned io_len)
{
  if (address == 0x0050) {
    BX_RADEON_THIS gencntl = value;
  } else if (address == 0x0054) {
    BX_RADEON_THIS extcntl = value;
  } else if (address >= 0x0018 && address < 0x0019) {
    BX_RADEON_THIS pci_write_handler(address - 0x0018, value, 4);
  }
  else if (address >= 0x0058 && address < 0x007c) {
    BX_RADEON_THIS ramdac[(address - 0x) / 4] = value;
  }
}


/////////////////////////////////////////////////////////////////////////
//
// PCI support
//
/////////////////////////////////////////////////////////////////////////

void bx_radeon_c::svga_init_pcihandlers(void)
{
    Bit8u devfunc = 0x00;
    DEV_register_pci_handlers2(BX_RADEON_THIS_PTR, &devfunc, BX_PLUGIN_RADEON, "Radeon AGP", true);
    BX_RADEON_THIS init_pci_conf(PCI_VENDOR_ATI, PCI_DEVICE_RADEON8500, 0x00, 0x030000, 0x00, BX_PCI_INTA);

    BX_RADEON_THIS pci_conf[0x10] = 0x08;
    BX_RADEON_THIS pci_conf[0x14] = 0x01;
    BX_RADEON_THIS init_bar_mem(0, BX_RADEON_THIS s.memsize, radeon_mem_read_handler,
                                 radeon_mem_write_handler);
    BX_RADEON_THIS init_bar_io(1, 256, read_handler, write_handler, radeon_iomask);
    BX_RADEON_THIS init_bar_mem(2, 0x10000, radeon_mem_read_handler,
                                radeon_mem_write_handler);
    BX_RADEON_THIS pci_rom_address = 0;
    BX_RADEON_THIS pci_rom_read_handler = radeon_mem_read_handler;
    BX_RADEON_THIS load_pci_rom(SIM->get_param_string(BXPN_VGA_ROM_PATH)->getptr());

    BX_RADEON_THIS pci_conf[0x0c] = 0x08;
    BX_RADEON_THIS pci_conf[0x0d] = 0x20;
    BX_RADEON_THIS pci_conf[0x2c] = 0x3A;
    BX_RADEON_THIS pci_conf[0x2d] = 0x00;
    BX_RADEON_THIS pci_conf[0x2e] = 0x02;
    BX_RADEON_THIS pci_conf[0x2f] = 0x10;
    BX_RADEON_THIS pci_conf[0x40] = BX_RADEON_THIS pci_conf[0x2c];
    BX_RADEON_THIS pci_conf[0x41] = BX_RADEON_THIS pci_conf[0x2d];
    BX_RADEON_THIS pci_conf[0x42] = BX_RADEON_THIS pci_conf[0x2e];
    BX_RADEON_THIS pci_conf[0x43] = BX_RADEON_THIS pci_conf[0x2f];
    BX_RADEON_THIS pci_conf[0x04] = 0x87;
    BX_RADEON_THIS pci_conf[0x05] = 0x01;
    BX_RADEON_THIS pci_conf[0x06] = 0xB0;
    BX_RADEON_THIS pci_conf[0x07] = 0x02;
    BX_RADEON_THIS pci_conf[0x34] = 0x58;
    BX_RADEON_THIS pci_conf[0x58] = 0x02;
    BX_RADEON_THIS pci_conf[0x59] = 0x00;
    BX_RADEON_THIS pci_conf[0x5a] = 0x20;
    BX_RADEON_THIS pci_conf[0x5b] = 0x00;
    BX_RADEON_THIS pci_conf[0x5c] = 0x07;
    BX_RADEON_THIS pci_conf[0x5d] = 0x00;
    BX_RADEON_THIS pci_conf[0x5e] = 0x00;
    BX_RADEON_THIS pci_conf[0x5f] = 0x1F;
    BX_RADEON_THIS pci_conf[0x54] = 0x01;
    BX_RADEON_THIS pci_conf[0x55] = 0x00;
    BX_RADEON_THIS pci_conf[0x56] = 0x00;
    BX_RADEON_THIS pci_conf[0x57] = 0x00;
    BX_RADEON_THIS pci_conf[0x50] = 0x01;
    BX_RADEON_THIS pci_conf[0x51] = 0x58;
    BX_RADEON_THIS pci_conf[0x52] = 0x02;
    BX_RADEON_THIS pci_conf[0x53] = 0x00;
}

void bx_radeon_c::pci_write_handler(Bit8u address, Bit32u value, unsigned io_len)
{
  Bit8u value8, oldval;

  if ((address >= 0x1c) && (address < 0x2c))
    return;

  BX_DEBUG_PCI_WRITE(address, value, io_len);
  for (unsigned i=0; i<io_len; i++) {
    value8 = (value >> (i*8)) & 0xFF;
    oldval = pci_conf[address+i];
    switch (address + i) {
      case 0x06:
      case 0x07:
        value8 = oldval;
        break;
    }
    pci_conf[address+i] = value8;
  }
}

#if BX_DEBUGGER
void bx_radeon_c::debug_dump(int argc, char **argv)
{
  bx_vgacore_c::debug_dump(argc, argv);
}
#endif

#endif // BX_SUPPORT_RADEON
