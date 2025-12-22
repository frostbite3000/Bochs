/////////////////////////////////////////////////////////////////////////
// $Id$
/////////////////////////////////////////////////////////////////////////
//
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

// PCI IDE controller
// i430FX - PIIX
// i440FX - PIIX3
// i440BX - PIIX4
// Intel 6 Series C200 - SATA Controller (AHCI mode not fully implemented)

// Define BX_PLUGGABLE in files that can be compiled into plugins.  For
// platforms that require a special tag on exported symbols, BX_PLUGGABLE
// is used to know when we are exporting symbols and when we are importing.
#define BX_PLUGGABLE

#include "iodev.h"

#if BX_SUPPORT_PCI

#include "pci.h"
#include "pci_ide.h"

#define LOG_THIS thePciIdeController->

bx_pci_ide_c *thePciIdeController = NULL;

const Bit8u bmdma_iomask[16] = {1, 0, 1, 0, 4, 0, 0, 0, 1, 0, 1, 0, 4, 0, 0, 0};

PLUGIN_ENTRY_FOR_MODULE(pci_ide)
{
  if (mode == PLUGIN_INIT) {
    thePciIdeController = new bx_pci_ide_c();
    bx_devices.pluginPciIdeController = thePciIdeController;
    BX_REGISTER_DEVICE_DEVMODEL(plugin, type, thePciIdeController, BX_PLUGIN_PCI_IDE);
  } else if (mode == PLUGIN_FINI) {
    delete thePciIdeController;
  } else if (mode == PLUGIN_PROBE) {
    return (int)PLUGTYPE_STANDARD;
  }
  return(0); // Success
}

bx_pci_ide_c::bx_pci_ide_c()
{
  put("pci_ide", "PIDE");
  s.bmdma[0].timer_index = BX_NULL_TIMER_HANDLE;
  s.bmdma[1].timer_index = BX_NULL_TIMER_HANDLE;
  s.bmdma[0].buffer = NULL;
  s.bmdma[1].buffer = NULL;
}

bx_pci_ide_c::~bx_pci_ide_c()
{
  if (s.bmdma[0].buffer != NULL) {
    delete [] s.bmdma[0].buffer;
  }
  if (s.bmdma[1].buffer != NULL) {
    delete [] s.bmdma[1].buffer;
  }
  SIM->get_bochs_root()->remove("pci_ide");
  BX_DEBUG(("Exit"));
}

void bx_pci_ide_c::init(void)
{
  unsigned i;
  Bit8u devfunc;

  BX_PIDE_THIS s.chipset = SIM->get_param_enum(BXPN_PCI_CHIPSET)->get();
  BX_PIDE_THIS s.sata_mode = AHCI_SATA_MODE_IDE;  // Default to IDE mode

  if (BX_PIDE_THIS s.chipset == BX_PCI_CHIPSET_I440BX) {
    devfunc = BX_PCI_DEVICE(7, 1);
  } else if (BX_PIDE_THIS s.chipset == BX_PCI_CHIPSET_I6_C200) {
    devfunc = BX_PCI_DEVICE(31, 2);
    // Check if AHCI mode is enabled via configuration
    bx_param_enum_c *sata_mode = SIM->get_param_enum("pci.sata_mode");
    if (sata_mode != NULL && sata_mode->get() == 1) {
      BX_PIDE_THIS s.sata_mode = AHCI_SATA_MODE_AHCI;
    }
  } else {
    devfunc = BX_PCI_DEVICE(1, 1);
  }

  const char *ctrl_name;
  if (BX_PIDE_THIS s.chipset == BX_PCI_CHIPSET_I6_C200) {
    if (BX_PIDE_THIS s.sata_mode == AHCI_SATA_MODE_AHCI) {
      ctrl_name = "C200 SATA Controller (AHCI)";
    } else {
      ctrl_name = "C200 SATA Controller (IDE)";
    }
  } else {
    ctrl_name = "PIIX3 PCI IDE controller";
  }
  DEV_register_pci_handlers(this, &devfunc, BX_PLUGIN_PCI_IDE, ctrl_name);

  // register BM-DMA timer
  for (i=0; i<2; i++) {
    if (BX_PIDE_THIS s.bmdma[i].timer_index == BX_NULL_TIMER_HANDLE) {
      BX_PIDE_THIS s.bmdma[i].timer_index =
        DEV_register_timer(this, timer_handler, 1000, 0,0, "PIIX3 BM-DMA timer");
      bx_pc_system.setTimerParam(BX_PIDE_THIS s.bmdma[i].timer_index, i);
    }
  }

  BX_PIDE_THIS s.bmdma[0].buffer = new Bit8u[0x20000];
  BX_PIDE_THIS s.bmdma[1].buffer = new Bit8u[0x20000];

  // initialize readonly registers
  if (BX_PIDE_THIS s.chipset == BX_PCI_CHIPSET_I430FX) {
    init_pci_conf(0x8086, 0x1230, 0x00, 0x010180, 0x00, 0);
  } else if (BX_PIDE_THIS s.chipset == BX_PCI_CHIPSET_I440BX) {
    init_pci_conf(0x8086, 0x7111, 0x00, 0x010180, 0x00, 0);
  } else if (BX_PIDE_THIS s.chipset == BX_PCI_CHIPSET_I6_C200) {
    if (BX_PIDE_THIS s.sata_mode == AHCI_SATA_MODE_AHCI) {
      // Intel C200 SATA Controller AHCI mode (Device ID 0x1C03)
      // Class code 0x010601 = Mass Storage / SATA / AHCI
      init_pci_conf(0x8086, 0x1c03, 0x05, 0x010601, 0x00, 0);
      BX_PIDE_THIS pci_conf[0x06] = 0x10; // Status: capabilities list
      BX_PIDE_THIS pci_conf[0x07] = 0x02;
      // Interrupt pin
      BX_PIDE_THIS pci_conf[0x3d] = 0x01; // INTA#
      // Capabilities pointer
      BX_PIDE_THIS pci_conf[0x34] = 0x80;
      // Power Management capability
      BX_PIDE_THIS pci_conf[0x80] = 0x01; // PM capability ID
      BX_PIDE_THIS pci_conf[0x81] = 0x70; // Next capability (MSI)
      BX_PIDE_THIS pci_conf[0x82] = 0x03; // PM capabilities
      BX_PIDE_THIS pci_conf[0x83] = 0x00;
      // MSI capability
      BX_PIDE_THIS pci_conf[0x70] = 0x05; // MSI capability ID
      BX_PIDE_THIS pci_conf[0x71] = 0xA8; // Next capability (SATA)
      BX_PIDE_THIS pci_conf[0x72] = 0x00; // Message Control
      BX_PIDE_THIS pci_conf[0x73] = 0x00;
      // SATA capability
      BX_PIDE_THIS pci_conf[0xA8] = 0x12; // SATA capability ID
      BX_PIDE_THIS pci_conf[0xA9] = 0x00; // No next capability
      BX_PIDE_THIS pci_conf[0xAA] = 0x10; // SATA revision 1.0
      BX_PIDE_THIS pci_conf[0xAB] = 0x00;
      // AHCI BAR (BAR5) - Memory mapped, 32-bit, non-prefetchable
      BX_PIDE_THIS pci_conf[0x24] = 0x00; // BAR5 low
      BX_PIDE_THIS pci_conf[0x25] = 0x00;
      BX_PIDE_THIS pci_conf[0x26] = 0x00;
      BX_PIDE_THIS pci_conf[0x27] = 0x00;
      // Register AHCI BAR (BAR5) for memory-mapped I/O
      BX_PIDE_THIS init_bar_mem(5, AHCI_ABAR_SIZE, ahci_read_handler, ahci_write_handler);
      BX_INFO(("C200 SATA Controller initialized in AHCI mode"));
    } else {
      // Intel C200 SATA Controller IDE mode (Device ID 0x1C02)
      init_pci_conf(0x8086, 0x1c02, 0x05, 0x010180, 0x00, 0);
      BX_PIDE_THIS pci_conf[0x06] = 0x10; // Status: capabilities list
      BX_PIDE_THIS pci_conf[0x07] = 0x02;
      // Capabilities pointer
      BX_PIDE_THIS pci_conf[0x34] = 0x80;
      // Power Management capability
      BX_PIDE_THIS pci_conf[0x80] = 0x01; // PM capability ID
      BX_PIDE_THIS pci_conf[0x81] = 0x70; // Next capability (MSI)
      BX_PIDE_THIS pci_conf[0x82] = 0x03; // PM capabilities
      BX_PIDE_THIS pci_conf[0x83] = 0x00;
      // MSI capability
      BX_PIDE_THIS pci_conf[0x70] = 0x05; // MSI capability ID
      BX_PIDE_THIS pci_conf[0x71] = 0x00; // No next capability
      BX_INFO(("C200 SATA Controller initialized in IDE mode"));
    }
  } else {
    init_pci_conf(0x8086, 0x7010, 0x00, 0x010180, 0x00, 0);
  }

  // Register BM-DMA BAR for IDE mode
  if (BX_PIDE_THIS s.sata_mode != AHCI_SATA_MODE_AHCI) {
    BX_PIDE_THIS init_bar_io(4, 16, read_handler, write_handler, &bmdma_iomask[0]);
  }

  // PCI IDE always provides 2 IDE channels (in IDE mode)
  if (BX_PIDE_THIS s.sata_mode != AHCI_SATA_MODE_AHCI) {
    if (!SIM->get_param_bool("ata.0.resources.enabled")->get() ||
        !SIM->get_param_bool("ata.1.resources.enabled")->get()) {
      BX_INFO(("Enable primary and secondary IDE channel (override settings)"));
      SIM->get_param("ata.0.resources")->reset();
      SIM->get_param("ata.1.resources")->reset();
    }
  }
}

void bx_pci_ide_c::reset(unsigned type)
{
  BX_PIDE_THIS pci_conf[0x04] = 0x01;
  BX_PIDE_THIS pci_conf[0x06] = 0x80;
  BX_PIDE_THIS pci_conf[0x07] = 0x02;

  if (BX_PIDE_THIS s.sata_mode == AHCI_SATA_MODE_AHCI) {
    // AHCI mode reset
    BX_PIDE_THIS pci_conf[0x04] = 0x06;  // Memory space + Bus master
    ahci_reset();
  } else {
    // IDE mode reset
    if (SIM->get_param_bool(BXPN_ATA0_ENABLED)->get()) {
      BX_PIDE_THIS pci_conf[0x40] = 0x00;
      BX_PIDE_THIS pci_conf[0x41] = 0x80;
    }
    if (SIM->get_param_bool(BXPN_ATA1_ENABLED)->get()) {
      BX_PIDE_THIS pci_conf[0x42] = 0x00;
      BX_PIDE_THIS pci_conf[0x43] = 0x80;
    }
    BX_PIDE_THIS pci_conf[0x44] = 0x00;
    for (unsigned i=0; i<2; i++) {
      BX_PIDE_THIS s.bmdma[i].cmd_ssbm = 0;
      BX_PIDE_THIS s.bmdma[i].cmd_rwcon = 0;
      BX_PIDE_THIS s.bmdma[i].status = 0;
      BX_PIDE_THIS s.bmdma[i].dtpr = 0;
      BX_PIDE_THIS s.bmdma[i].prd_current = 0;
      BX_PIDE_THIS s.bmdma[i].buffer_top = BX_PIDE_THIS s.bmdma[i].buffer;
      BX_PIDE_THIS s.bmdma[i].buffer_idx = BX_PIDE_THIS s.bmdma[i].buffer;
      BX_PIDE_THIS s.bmdma[i].data_ready = 0;
    }
  }
}

// save/restore code begin
void bx_pci_ide_c::register_state(void)
{
  char name[6];

  bx_list_c *list = new bx_list_c(SIM->get_bochs_root(), "pci_ide", "PCI IDE Controller State");

  register_pci_state(list);

  new bx_shadow_data_c(list, "buffer0", BX_PIDE_THIS s.bmdma[0].buffer, 0x20000);
  new bx_shadow_data_c(list, "buffer1", BX_PIDE_THIS s.bmdma[1].buffer, 0x20000);

  for (unsigned i=0; i<2; i++) {
    sprintf(name, "%u", i);
    bx_list_c *ctrl = new bx_list_c(list, name);
    BXRS_PARAM_BOOL(ctrl, cmd_ssbm, BX_PIDE_THIS s.bmdma[i].cmd_ssbm);
    BXRS_PARAM_BOOL(ctrl, cmd_rwcon, BX_PIDE_THIS s.bmdma[i].cmd_rwcon);
    BXRS_HEX_PARAM_FIELD(ctrl, status, BX_PIDE_THIS s.bmdma[i].status);
    BXRS_HEX_PARAM_FIELD(ctrl, dtpr, BX_PIDE_THIS s.bmdma[i].dtpr);
    BXRS_HEX_PARAM_FIELD(ctrl, prd_current, BX_PIDE_THIS s.bmdma[i].prd_current);
    BXRS_PARAM_SPECIAL32(ctrl, buffer_top,
       BX_PIDE_THIS param_save_handler, BX_PIDE_THIS param_restore_handler);
    BXRS_PARAM_SPECIAL32(ctrl, buffer_idx,
       BX_PIDE_THIS param_save_handler, BX_PIDE_THIS param_restore_handler);
    BXRS_PARAM_BOOL(ctrl, data_ready, BX_PIDE_THIS s.bmdma[i].data_ready);
  }
}

void bx_pci_ide_c::after_restore_state(void)
{
  bx_pci_device_c::after_restore_pci_state(NULL);
}

Bit64s bx_pci_ide_c::param_save_handler(void *devptr, bx_param_c *param)
{
#if !BX_USE_PIDE_SMF
  bx_pci_ide_c *class_ptr = (bx_pci_ide_c *) devptr;
  return class_ptr->param_save(param, val);
}

Bit64s bx_pci_ide_c::param_save(bx_param_c *param)
{
#else
  UNUSED(devptr);
#endif // !BX_USE_PIDE_SMF
  int chan = atoi(param->get_parent()->get_name());
  Bit64s val = 0;
  if (!strcmp(param->get_name(), "buffer_top")) {
    val = (Bit32u)(BX_PIDE_THIS s.bmdma[chan].buffer_top - BX_PIDE_THIS s.bmdma[chan].buffer);
  } else if (!strcmp(param->get_name(), "buffer_idx")) {
    val = (Bit32u)(BX_PIDE_THIS s.bmdma[chan].buffer_idx - BX_PIDE_THIS s.bmdma[chan].buffer);
  }
  return val;
}

void bx_pci_ide_c::param_restore_handler(void *devptr, bx_param_c *param, Bit64s val)
{
#if !BX_USE_PIDE_SMF
  bx_pci_ide_c *class_ptr = (bx_pci_ide_c *) devptr;
  class_ptr->param_restore(param, val);
}

void bx_pci_ide_c::param_restore(bx_param_c *param, Bit64s val)
{
#else
  UNUSED(devptr);
#endif // !BX_USE_PIDE_SMF
  int chan = atoi(param->get_parent()->get_name());
  if (!strcmp(param->get_name(), "buffer_top")) {
    BX_PIDE_THIS s.bmdma[chan].buffer_top = BX_PIDE_THIS s.bmdma[chan].buffer + val;
  } else if (!strcmp(param->get_name(), "buffer_idx")) {
    BX_PIDE_THIS s.bmdma[chan].buffer_idx = BX_PIDE_THIS s.bmdma[chan].buffer + val;
  }
}
// save/restore code end

bool bx_pci_ide_c::bmdma_present(void)
{
  return (BX_PIDE_THIS pci_bar[4].addr > 0);
}

void bx_pci_ide_c::bmdma_start_transfer(Bit8u channel)
{
  if (channel < 2) {
    BX_PIDE_THIS s.bmdma[channel].data_ready = 1;
  }
}

void bx_pci_ide_c::bmdma_set_irq(Bit8u channel)
{
  if (channel < 2) {
    BX_PIDE_THIS s.bmdma[channel].status |= 0x04;
  }
}

void bx_pci_ide_c::timer_handler(void *this_ptr)
{
  bx_pci_ide_c *class_ptr = (bx_pci_ide_c *) this_ptr;
  class_ptr->timer();
}

void bx_pci_ide_c::timer()
{
  int count;
  Bit32u size, sector_size;
  struct {
    Bit32u addr;
    Bit32u size;
  } prd;

  Bit8u channel = bx_pc_system.triggeredTimerParam();
  if (((BX_PIDE_THIS s.bmdma[channel].status & 0x01) == 0) ||
      (BX_PIDE_THIS s.bmdma[channel].prd_current == 0)) {
    return;
  }
  if (BX_PIDE_THIS s.bmdma[channel].cmd_rwcon &&
      !BX_PIDE_THIS s.bmdma[channel].data_ready) {
    bx_pc_system.activate_timer(BX_PIDE_THIS s.bmdma[channel].timer_index, 1, 0);
    return;
  }
  DEV_MEM_READ_PHYSICAL(BX_PIDE_THIS s.bmdma[channel].prd_current, 4, (Bit8u *)&prd.addr);
  DEV_MEM_READ_PHYSICAL(BX_PIDE_THIS s.bmdma[channel].prd_current+4, 4, (Bit8u *)&prd.size);
  size = prd.size & 0xfffe;
  if (size == 0) {
    size = 0x10000;
  }
  if (BX_PIDE_THIS s.bmdma[channel].cmd_rwcon) {
    BX_DEBUG(("READ DMA to addr=0x%08x, size=0x%08x", prd.addr, size));
    count = (int)(size - (BX_PIDE_THIS s.bmdma[channel].buffer_top - BX_PIDE_THIS s.bmdma[channel].buffer_idx));
    while (count > 0) {
      sector_size = count;
      if (DEV_hd_bmdma_read_sector(channel, BX_PIDE_THIS s.bmdma[channel].buffer_top, &sector_size)) {
        BX_PIDE_THIS s.bmdma[channel].buffer_top += sector_size;
        count -= sector_size;
      } else {
        break;
      }
    };
    if (count > 0) {
      DEV_hd_bmdma_complete(channel);
      return;
    } else {
      DEV_MEM_WRITE_PHYSICAL_DMA(prd.addr, size, BX_PIDE_THIS s.bmdma[channel].buffer_idx);
      BX_PIDE_THIS s.bmdma[channel].buffer_idx += size;
    }
  } else {
    BX_DEBUG(("WRITE DMA from addr=0x%08x, size=0x%08x", prd.addr, size));
    DEV_MEM_READ_PHYSICAL_DMA(prd.addr, size, BX_PIDE_THIS s.bmdma[channel].buffer_top);
    BX_PIDE_THIS s.bmdma[channel].buffer_top += size;
    count = (int)(BX_PIDE_THIS s.bmdma[channel].buffer_top - BX_PIDE_THIS s.bmdma[channel].buffer_idx);
    while (count > 511) {
      if (DEV_hd_bmdma_write_sector(channel, BX_PIDE_THIS s.bmdma[channel].buffer_idx)) {
        BX_PIDE_THIS s.bmdma[channel].buffer_idx += 512;
        count -= 512;
      } else {
        break;
      }
    };
    if (count >= 512) {
      DEV_hd_bmdma_complete(channel);
      return;
    }
  }
  if (prd.size & 0x80000000) {
    BX_PIDE_THIS s.bmdma[channel].status &= ~0x01;
    BX_PIDE_THIS s.bmdma[channel].status |= 0x04;
    BX_PIDE_THIS s.bmdma[channel].prd_current = 0;
    DEV_hd_bmdma_complete(channel);
  } else {
    // To avoid buffer overflow reset buffer pointers and copy data if necessary
    count = (int)(BX_PIDE_THIS s.bmdma[channel].buffer_top - BX_PIDE_THIS s.bmdma[channel].buffer_idx);
    if (count > 0) {
      memmove(BX_PIDE_THIS s.bmdma[channel].buffer, BX_PIDE_THIS s.bmdma[channel].buffer_idx, count);
    }
    BX_PIDE_THIS s.bmdma[channel].buffer_top = BX_PIDE_THIS s.bmdma[channel].buffer + count;
    BX_PIDE_THIS s.bmdma[channel].buffer_idx = BX_PIDE_THIS s.bmdma[channel].buffer;
    // Prepare for next PRD
    BX_PIDE_THIS s.bmdma[channel].prd_current += 8;
    DEV_MEM_READ_PHYSICAL(BX_PIDE_THIS s.bmdma[channel].prd_current, 4, (Bit8u *)&prd.addr);
    DEV_MEM_READ_PHYSICAL(BX_PIDE_THIS s.bmdma[channel].prd_current+4, 4, (Bit8u *)&prd.size);
    size = prd.size & 0xfffe;
    if (size == 0) {
      size = 0x10000;
    }
    bx_pc_system.activate_timer(BX_PIDE_THIS s.bmdma[channel].timer_index, (size >> 4) | 0x10, 0);
  }
}


// static IO port read callback handler
// redirects to non-static class handler to avoid virtual functions

Bit32u bx_pci_ide_c::read_handler(void *this_ptr, Bit32u address, unsigned io_len)
{
#if !BX_USE_PIDE_SMF
  bx_pci_ide_c *class_ptr = (bx_pci_ide_c *) this_ptr;
  return class_ptr->read(address, io_len);
}

Bit32u bx_pci_ide_c::read(Bit32u address, unsigned io_len)
{
#else
  UNUSED(this_ptr);
#endif // !BX_USE_PIDE_SMF
  Bit8u offset, channel;
  Bit32u value = 0xffffffff;

  offset = address - BX_PIDE_THIS pci_bar[4].addr;
  channel = (offset >> 3);
  offset &= 0x07;
  switch (offset) {
    case 0x00:
      value = (Bit32u)BX_PIDE_THIS s.bmdma[channel].cmd_ssbm |
              (Bit32u)(BX_PIDE_THIS s.bmdma[channel].cmd_rwcon << 3);
      BX_DEBUG(("BM-DMA read command register, channel %d, value = 0x%02x", channel, value));
      break;
    case 0x02:
      value = BX_PIDE_THIS s.bmdma[channel].status;
      BX_DEBUG(("BM-DMA read status register, channel %d, value = 0x%02x", channel, value));
      break;
    case 0x04:
      value = BX_PIDE_THIS s.bmdma[channel].dtpr;
      BX_DEBUG(("BM-DMA read DTP register, channel %d, value = 0x%08x", channel, value));
      break;
  }

  return value;
}


// static IO port write callback handler
// redirects to non-static class handler to avoid virtual functions

void bx_pci_ide_c::write_handler(void *this_ptr, Bit32u address, Bit32u value, unsigned io_len)
{
#if !BX_USE_PIDE_SMF
  bx_pci_ide_c *class_ptr = (bx_pci_ide_c *) this_ptr;

  class_ptr->write(address, value, io_len);
}

void bx_pci_ide_c::write(Bit32u address, Bit32u value, unsigned io_len)
{
#else
  UNUSED(this_ptr);
#endif // !BX_USE_PIDE_SMF
  Bit8u offset, channel;

  offset = address - BX_PIDE_THIS pci_bar[4].addr;
  channel = (offset >> 3);
  offset &= 0x07;
  switch (offset) {
    case 0x00:
      BX_DEBUG(("BM-DMA write command register, channel %d, value = 0x%02x", channel, value));
      BX_PIDE_THIS s.bmdma[channel].cmd_rwcon = (value >> 3) & 1;
      if ((value & 0x01) && !BX_PIDE_THIS s.bmdma[channel].cmd_ssbm) {
        BX_PIDE_THIS s.bmdma[channel].cmd_ssbm = 1;
        BX_PIDE_THIS s.bmdma[channel].status |= 0x01;
        BX_PIDE_THIS s.bmdma[channel].prd_current = BX_PIDE_THIS s.bmdma[channel].dtpr;
        BX_PIDE_THIS s.bmdma[channel].buffer_top = BX_PIDE_THIS s.bmdma[channel].buffer;
        BX_PIDE_THIS s.bmdma[channel].buffer_idx = BX_PIDE_THIS s.bmdma[channel].buffer;
        bx_pc_system.activate_timer(BX_PIDE_THIS s.bmdma[channel].timer_index, 1, 0);
      } else if (!(value & 0x01) && BX_PIDE_THIS s.bmdma[channel].cmd_ssbm) {
        BX_PIDE_THIS s.bmdma[channel].cmd_ssbm = 0;
        BX_PIDE_THIS s.bmdma[channel].status &= ~0x01;
        BX_PIDE_THIS s.bmdma[channel].data_ready = 0;
      }
      break;
    case 0x02:
      BX_PIDE_THIS s.bmdma[channel].status = (value & 0x60)
        | (BX_PIDE_THIS s.bmdma[channel].status & 0x01)
        | (BX_PIDE_THIS s.bmdma[channel].status & (~value & 0x06));
      BX_DEBUG(("BM-DMA write status register, channel %d, value = 0x%02x", channel, value));
      break;
    case 0x04:
      BX_PIDE_THIS s.bmdma[channel].dtpr = value & 0xfffffffc;
      BX_DEBUG(("BM-DMA write DTP register, channel %d, value = 0x%08x", channel, value));
      break;
  }
}


// pci configuration space write callback handler
void bx_pci_ide_c::pci_write_handler(Bit8u address, Bit32u value, unsigned io_len)
{
  if (((address >= 0x10) && (address < 0x20)) ||
      ((address > 0x23) && (address < 0x40)))
    return;

  BX_DEBUG_PCI_WRITE(address, value, io_len);
  for (unsigned i=0; i<io_len; i++) {
//  Bit8u oldval = BX_PIDE_THIS pci_conf[address+i];
    Bit8u value8 = (value >> (i*8)) & 0xFF;
    switch (address+i) {
      case 0x05:
      case 0x06:
        break;
      case 0x04:
        if (BX_PIDE_THIS s.sata_mode == AHCI_SATA_MODE_AHCI) {
          BX_PIDE_THIS pci_conf[address+i] = value8 & 0x07;  // Allow memory space
        } else {
          BX_PIDE_THIS pci_conf[address+i] = value8 & 0x05;
        }
        break;
      default:
        BX_PIDE_THIS pci_conf[address+i] = value8;
        BX_DEBUG(("PIIX3 PCI IDE write register 0x%02x value 0x%02x", address+i,
                  value8));
    }
  }
}

// ============================================================================
// AHCI (Advanced Host Controller Interface) Implementation
// ============================================================================

void bx_pci_ide_c::ahci_reset(void)
{
  int i;

  // Initialize HBA registers
  // CAP: 6 ports, 32 command slots, supports NCQ, 64-bit addressing, Gen2 speed
  BX_PIDE_THIS s.ahci.cap = (AHCI_MAX_PORTS - 1) |          // Number of ports (0-based)
                            ((AHCI_CMD_SLOTS - 1) << 8) |   // Number of command slots (0-based)
                            AHCI_CAP_S64A |                  // 64-bit addressing
                            AHCI_CAP_SNCQ |                  // Native Command Queuing
                            AHCI_CAP_SSNTF |                 // SNotification
                            AHCI_CAP_ISS_GEN2 |              // Gen2 (3.0 Gbps)
                            AHCI_CAP_SAM |                   // AHCI mode only
                            AHCI_CAP_SCLO |                  // Command List Override
                            AHCI_CAP_PSC |                   // Partial State Capable
                            AHCI_CAP_SSC;                    // Slumber State Capable

  BX_PIDE_THIS s.ahci.ghc = AHCI_GHC_AE;     // AHCI Enable (read-only 1 for AHCI-only)
  BX_PIDE_THIS s.ahci.is = 0;                // No interrupts pending
  BX_PIDE_THIS s.ahci.pi = 0x3F;             // Ports 0-5 implemented
  BX_PIDE_THIS s.ahci.vs = 0x00010200;       // AHCI version 1.2
  BX_PIDE_THIS s.ahci.ccc_ctl = 0;
  BX_PIDE_THIS s.ahci.ccc_ports = 0;
  BX_PIDE_THIS s.ahci.em_loc = 0;
  BX_PIDE_THIS s.ahci.em_ctl = 0;
  BX_PIDE_THIS s.ahci.cap2 = 0;
  BX_PIDE_THIS s.ahci.bohc = 0;

  // Reset all ports
  for (i = 0; i < AHCI_MAX_PORTS; i++) {
    ahci_port_reset(i);
  }

  BX_INFO(("AHCI controller reset complete"));
}

void bx_pci_ide_c::ahci_port_reset(int port)
{
  ahci_port_t *p = &BX_PIDE_THIS s.ahci.port[port];

  p->clb = 0;
  p->clbu = 0;
  p->fb = 0;
  p->fbu = 0;
  p->is = 0;
  p->ie = 0;
  p->cmd = AHCI_PxCMD_POD | AHCI_PxCMD_SUD;  // Power on, spin up
  p->tfd = 0x7F;                              // Initial TFD: BSY=0, DRQ=0, ERR=0
  p->sig = 0xFFFFFFFF;                        // No device signature yet
  p->ssts = AHCI_PxSSTS_DET_NONE;             // No device detected
  p->sctl = 0;
  p->serr = 0;
  p->sact = 0;
  p->ci = 0;
  p->sntf = 0;
  p->fbs = 0;

  // Check if a device is present on this port (map to ATA channels)
  // Port 0-1 map to primary channel master/slave
  // Port 2-3 map to secondary channel master/slave
  p->device_present = false;
  if (port < 4) {
    int channel = port / 2;
    int device = port % 2;
    char path[64];
    sprintf(path, "ata.%d.%s.present", channel, device ? "slave" : "master");
    bx_param_bool_c *present = SIM->get_param_bool(path);
    if (present && present->get()) {
      p->device_present = true;
      p->ssts = AHCI_PxSSTS_DET_ONLINE | (2 << 4) | (1 << 8);  // Device present, Gen2 speed, active
      p->sig = 0x00000101;  // ATA device signature
      p->tfd = 0x50;        // Device ready
      BX_INFO(("AHCI port %d: device present", port));
    }
  }
}

// AHCI memory-mapped I/O read handler
Bit32u bx_pci_ide_c::ahci_read_handler(void *this_ptr, Bit32u address, unsigned io_len)
{
#if !BX_USE_PIDE_SMF
  bx_pci_ide_c *class_ptr = (bx_pci_ide_c *) this_ptr;
  return class_ptr->ahci_read(address - class_ptr->pci_bar[5].addr, io_len);
#else
  UNUSED(this_ptr);
  Bit32u offset = address - BX_PIDE_THIS pci_bar[5].addr;
  return BX_PIDE_THIS ahci_read(offset, io_len);
#endif
}

Bit32u bx_pci_ide_c::ahci_read(Bit32u offset, unsigned io_len)
{
  Bit32u value = 0;

  if (offset < AHCI_PORT_BASE) {
    // Generic Host Control registers
    switch (offset) {
      case AHCI_REG_CAP:
        value = BX_PIDE_THIS s.ahci.cap;
        break;
      case AHCI_REG_GHC:
        value = BX_PIDE_THIS s.ahci.ghc;
        break;
      case AHCI_REG_IS:
        value = BX_PIDE_THIS s.ahci.is;
        break;
      case AHCI_REG_PI:
        value = BX_PIDE_THIS s.ahci.pi;
        break;
      case AHCI_REG_VS:
        value = BX_PIDE_THIS s.ahci.vs;
        break;
      case AHCI_REG_CCC_CTL:
        value = BX_PIDE_THIS s.ahci.ccc_ctl;
        break;
      case AHCI_REG_CCC_PORTS:
        value = BX_PIDE_THIS s.ahci.ccc_ports;
        break;
      case AHCI_REG_EM_LOC:
        value = BX_PIDE_THIS s.ahci.em_loc;
        break;
      case AHCI_REG_EM_CTL:
        value = BX_PIDE_THIS s.ahci.em_ctl;
        break;
      case AHCI_REG_CAP2:
        value = BX_PIDE_THIS s.ahci.cap2;
        break;
      case AHCI_REG_BOHC:
        value = BX_PIDE_THIS s.ahci.bohc;
        break;
      default:
        BX_DEBUG(("AHCI: read from unknown HBA register 0x%03x", offset));
        break;
    }
  } else {
    // Port registers
    int port = (offset - AHCI_PORT_BASE) / AHCI_PORT_SIZE;
    if (port < AHCI_MAX_PORTS) {
      value = ahci_port_read(port, (offset - AHCI_PORT_BASE) % AHCI_PORT_SIZE);
    }
  }

  BX_DEBUG(("AHCI read offset=0x%03x value=0x%08x", offset, value));
  return value;
}

Bit32u bx_pci_ide_c::ahci_port_read(int port, Bit32u offset)
{
  ahci_port_t *p = &BX_PIDE_THIS s.ahci.port[port];
  Bit32u value = 0;

  switch (offset) {
    case AHCI_PxCLB:
      value = p->clb;
      break;
    case AHCI_PxCLBU:
      value = p->clbu;
      break;
    case AHCI_PxFB:
      value = p->fb;
      break;
    case AHCI_PxFBU:
      value = p->fbu;
      break;
    case AHCI_PxIS:
      value = p->is;
      break;
    case AHCI_PxIE:
      value = p->ie;
      break;
    case AHCI_PxCMD:
      value = p->cmd;
      break;
    case AHCI_PxTFD:
      value = p->tfd;
      break;
    case AHCI_PxSIG:
      value = p->sig;
      break;
    case AHCI_PxSSTS:
      value = p->ssts;
      break;
    case AHCI_PxSCTL:
      value = p->sctl;
      break;
    case AHCI_PxSERR:
      value = p->serr;
      break;
    case AHCI_PxSACT:
      value = p->sact;
      break;
    case AHCI_PxCI:
      value = p->ci;
      break;
    case AHCI_PxSNTF:
      value = p->sntf;
      break;
    case AHCI_PxFBS:
      value = p->fbs;
      break;
    default:
      BX_DEBUG(("AHCI: read from unknown port %d register 0x%02x", port, offset));
      break;
  }

  return value;
}

// AHCI memory-mapped I/O write handler
void bx_pci_ide_c::ahci_write_handler(void *this_ptr, Bit32u address, Bit32u value, unsigned io_len)
{
#if !BX_USE_PIDE_SMF
  bx_pci_ide_c *class_ptr = (bx_pci_ide_c *) this_ptr;
  class_ptr->ahci_write(address - class_ptr->pci_bar[5].addr, value, io_len);
#else
  UNUSED(this_ptr);
  Bit32u offset = address - BX_PIDE_THIS pci_bar[5].addr;
  BX_PIDE_THIS ahci_write(offset, value, io_len);
#endif
}

void bx_pci_ide_c::ahci_write(Bit32u offset, Bit32u value, unsigned io_len)
{
  BX_DEBUG(("AHCI write offset=0x%03x value=0x%08x", offset, value));

  if (offset < AHCI_PORT_BASE) {
    // Generic Host Control registers
    switch (offset) {
      case AHCI_REG_GHC:
        if (value & AHCI_GHC_HR) {
          // HBA Reset
          ahci_reset();
        } else {
          // Only IE bit is writable (AE is read-only 1 for AHCI-only mode)
          BX_PIDE_THIS s.ahci.ghc = (BX_PIDE_THIS s.ahci.ghc & ~AHCI_GHC_IE) |
                                    (value & AHCI_GHC_IE) | AHCI_GHC_AE;
        }
        break;
      case AHCI_REG_IS:
        // Write 1 to clear interrupt status bits
        BX_PIDE_THIS s.ahci.is &= ~value;
        ahci_update_irq();
        break;
      case AHCI_REG_CCC_CTL:
        BX_PIDE_THIS s.ahci.ccc_ctl = value;
        break;
      case AHCI_REG_CCC_PORTS:
        BX_PIDE_THIS s.ahci.ccc_ports = value;
        break;
      case AHCI_REG_EM_CTL:
        BX_PIDE_THIS s.ahci.em_ctl = value;
        break;
      case AHCI_REG_BOHC:
        BX_PIDE_THIS s.ahci.bohc = value;
        break;
      default:
        BX_DEBUG(("AHCI: write to unknown/read-only HBA register 0x%03x", offset));
        break;
    }
  } else {
    // Port registers
    int port = (offset - AHCI_PORT_BASE) / AHCI_PORT_SIZE;
    if (port < AHCI_MAX_PORTS) {
      ahci_port_write(port, (offset - AHCI_PORT_BASE) % AHCI_PORT_SIZE, value);
    }
  }
}

void bx_pci_ide_c::ahci_port_write(int port, Bit32u offset, Bit32u value)
{
  ahci_port_t *p = &BX_PIDE_THIS s.ahci.port[port];

  switch (offset) {
    case AHCI_PxCLB:
      p->clb = value & 0xFFFFFC00;  // 1KB aligned
      break;
    case AHCI_PxCLBU:
      p->clbu = value;
      break;
    case AHCI_PxFB:
      p->fb = value & 0xFFFFFF00;   // 256-byte aligned
      break;
    case AHCI_PxFBU:
      p->fbu = value;
      break;
    case AHCI_PxIS:
      // Write 1 to clear
      p->is &= ~value;
      ahci_update_irq();
      break;
    case AHCI_PxIE:
      p->ie = value;
      break;
    case AHCI_PxCMD:
      {
        Bit32u old_cmd = p->cmd;
        // Handle writable bits
        p->cmd = (p->cmd & 0xFFFF0000) | (value & 0x0000FFFF);

        // Handle Start (ST) bit
        if ((value & AHCI_PxCMD_ST) && !(old_cmd & AHCI_PxCMD_ST)) {
          // Starting command processing
          if (p->device_present) {
            p->cmd |= AHCI_PxCMD_CR;  // Command List Running
            ahci_check_cmd(port);
          }
        } else if (!(value & AHCI_PxCMD_ST) && (old_cmd & AHCI_PxCMD_ST)) {
          // Stopping command processing
          p->cmd &= ~AHCI_PxCMD_CR;
        }

        // Handle FIS Receive Enable (FRE) bit
        if ((value & AHCI_PxCMD_FRE) && !(old_cmd & AHCI_PxCMD_FRE)) {
          p->cmd |= AHCI_PxCMD_FR;  // FIS Receive Running
        } else if (!(value & AHCI_PxCMD_FRE) && (old_cmd & AHCI_PxCMD_FRE)) {
          p->cmd &= ~AHCI_PxCMD_FR;
        }

        // Handle Command List Override (CLO)
        if (value & AHCI_PxCMD_CLO) {
          p->tfd &= ~(AHCI_PxTFD_STS_BSY | AHCI_PxTFD_STS_DRQ);
          p->cmd &= ~AHCI_PxCMD_CLO;
        }
      }
      break;
    case AHCI_PxSCTL:
      {
        Bit32u old_sctl = p->sctl;
        p->sctl = value;

        // Handle Device Detection Initialization (DET)
        if ((value & 0x0F) == 1 && (old_sctl & 0x0F) != 1) {
          // COMRESET
          if (p->device_present) {
            p->ssts = AHCI_PxSSTS_DET_ONLINE | (2 << 4) | (1 << 8);
            p->sig = 0x00000101;
            p->tfd = 0x50;
            p->is |= AHCI_PxIS_DHRS;  // D2H Register FIS
            ahci_port_irq(port);
          }
        } else if ((value & 0x0F) == 0 && (old_sctl & 0x0F) != 0) {
          // Clear COMRESET
          if (p->device_present) {
            p->is |= AHCI_PxIS_PRCS;  // PhyRdy Change
            ahci_port_irq(port);
          }
        }
      }
      break;
    case AHCI_PxSERR:
      // Write 1 to clear
      p->serr &= ~value;
      break;
    case AHCI_PxSACT:
      p->sact |= value;
      break;
    case AHCI_PxCI:
      p->ci |= value;
      if ((p->cmd & AHCI_PxCMD_ST) && p->device_present) {
        ahci_check_cmd(port);
      }
      break;
    case AHCI_PxSNTF:
      // Write 1 to clear
      p->sntf &= ~value;
      break;
    case AHCI_PxFBS:
      p->fbs = value;
      break;
    default:
      BX_DEBUG(("AHCI: write to unknown port %d register 0x%02x", port, offset));
      break;
  }
}

void bx_pci_ide_c::ahci_check_cmd(int port)
{
  ahci_port_t *p = &BX_PIDE_THIS s.ahci.port[port];
  int slot;

  if (!(p->cmd & AHCI_PxCMD_ST) || !p->device_present) {
    return;
  }

  // Check each command slot
  for (slot = 0; slot < AHCI_CMD_SLOTS; slot++) {
    if (p->ci & (1 << slot)) {
      ahci_process_cmd(port, slot);
    }
  }
}

void bx_pci_ide_c::ahci_process_cmd(int port, int slot)
{
  ahci_port_t *p = &BX_PIDE_THIS s.ahci.port[port];

  // For now, just complete commands immediately with success
  // A full implementation would read the command from the command list,
  // execute it, and update the FIS receive area

  BX_DEBUG(("AHCI: processing command on port %d slot %d", port, slot));

  // Clear command issue bit
  p->ci &= ~(1 << slot);

  // Set interrupt status
  p->is |= AHCI_PxIS_DHRS;  // D2H Register FIS Interrupt
  p->tfd = 0x50;            // Device ready, no error

  ahci_port_irq(port);
}

void bx_pci_ide_c::ahci_port_irq(int port)
{
  ahci_port_t *p = &BX_PIDE_THIS s.ahci.port[port];

  // Check if any enabled interrupt is pending
  if (p->is & p->ie) {
    BX_PIDE_THIS s.ahci.is |= (1 << port);
    ahci_update_irq();
  }
}

void bx_pci_ide_c::ahci_update_irq(void)
{
  // Check if global interrupt enable is set and any port has pending interrupts
  if ((BX_PIDE_THIS s.ahci.ghc & AHCI_GHC_IE) && BX_PIDE_THIS s.ahci.is) {
    DEV_pci_set_irq(BX_PIDE_THIS devfunc, BX_PIDE_THIS pci_conf[0x3d], 1);
    BX_DEBUG(("AHCI: raising interrupt"));
  } else {
    DEV_pci_set_irq(BX_PIDE_THIS devfunc, BX_PIDE_THIS pci_conf[0x3d], 0);
  }
}

#endif /* BX_SUPPORT_PCI */
