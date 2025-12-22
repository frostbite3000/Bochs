/////////////////////////////////////////////////////////////////////////
// $Id$
/////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2002-2021  The Bochs Project
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

#ifndef BX_IODEV_PCI_BRIDGE_H
#define BX_IODEV_PCI_BRIDGE_H

#if BX_USE_PCI_SMF
#  define BX_PCI_SMF  static
#  define BX_PCI_THIS thePciBridge->
#else
#  define BX_PCI_SMF
#  define BX_PCI_THIS this->
#endif

#define BX_PCI_DEVICE(device, function) ((device)<<3 | (function))

// PCI Express link speeds
#define PCIE_LNKCAP_SLS_2_5GT   0x01
#define PCIE_LNKCAP_SLS_5_0GT   0x02
#define PCIE_LNKCAP_SLS_8_0GT   0x03

// PCI Express link widths
#define PCIE_LNKCAP_MLW_X1      0x01
#define PCIE_LNKCAP_MLW_X4      0x04
#define PCIE_LNKCAP_MLW_X8      0x08
#define PCIE_LNKCAP_MLW_X16     0x10

enum {
  BX_PCI_INTA = 1,
  BX_PCI_INTB = 2,
  BX_PCI_INTC = 3,
  BX_PCI_INTD = 4
};

class bx_pci_vbridge_c;
class bx_pcie_bridge_c;

class bx_pci_bridge_c : public bx_pci_device_c {
public:
  bx_pci_bridge_c();
  virtual ~bx_pci_bridge_c();
  virtual void init(void);
  virtual void reset(unsigned type);
  virtual void register_state(void);
  virtual void after_restore_state(void);

  static bool agp_ap_read_handler(bx_phy_address addr, unsigned len, void *data, void *param);
  static bool agp_ap_write_handler(bx_phy_address addr, unsigned len, void *data, void *param);

  bool agp_aperture_read(bx_phy_address addr, unsigned len, Bit8u *data, bool agp);
  bool agp_aperture_write(bx_phy_address addr, unsigned len, Bit8u *data, bool agp);

  virtual void pci_write_handler(Bit8u address, Bit32u value, unsigned io_len);
#if BX_DEBUGGER
  virtual void debug_dump(int argc, char **argv);
#endif

private:
  void smram_control(Bit8u value);

  unsigned chipset;
  Bit8u DRBA[8];
  Bit8u dram_detect;
  Bit32u gart_base;
  bx_pci_vbridge_c *vbridge;
  bx_pcie_bridge_c *pcie_bridge;
};

class bx_pci_vbridge_c : public bx_pci_device_c {
public:
  bx_pci_vbridge_c();
  virtual ~bx_pci_vbridge_c();
  virtual void init(void);
  virtual void reset(unsigned type);
  virtual void register_state(void);
  virtual void after_restore_state(void);

  virtual void pci_write_handler(Bit8u address, Bit32u value, unsigned io_len);
};

// PCIe Root Port (PCI Express bridge) - similar to AGP bridge but for PCIe
class bx_pcie_bridge_c : public bx_pci_device_c {
public:
  bx_pcie_bridge_c();
  virtual ~bx_pcie_bridge_c();
  virtual void init(void);
  virtual void reset(unsigned type);
  virtual void register_state(void);
  virtual void after_restore_state(void);

  virtual void pci_write_handler(Bit8u address, Bit32u value, unsigned io_len);

private:
  Bit8u slot_nr;
  Bit8u root_port_nr;
};
#endif
