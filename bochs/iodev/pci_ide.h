/////////////////////////////////////////////////////////////////////////
// $Id$
/////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2004-2021  The Bochs Project
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

#ifndef BX_IODEV_PCIIDE_H
#define BX_IODEV_PCIIDE_H

#if BX_USE_PIDE_SMF
#  define BX_PIDE_SMF  static
#  define BX_PIDE_THIS thePciIdeController->
#  define BX_PIDE_THIS_PTR thePciIdeController
#else
#  define BX_PIDE_SMF
#  define BX_PIDE_THIS this->
#  define BX_PIDE_THIS_PTR this
#endif

// AHCI mode constants
#define AHCI_MAX_PORTS          6       // C200 supports up to 6 SATA ports
#define AHCI_CMD_SLOTS          32      // Command slots per port
#define AHCI_ABAR_SIZE          0x1100  // AHCI BAR size (Generic Host Control + 6 ports)

// AHCI Generic Host Control Register offsets
#define AHCI_REG_CAP            0x00    // Host Capabilities
#define AHCI_REG_GHC            0x04    // Global Host Control
#define AHCI_REG_IS             0x08    // Interrupt Status
#define AHCI_REG_PI             0x0C    // Ports Implemented
#define AHCI_REG_VS             0x10    // AHCI Version
#define AHCI_REG_CCC_CTL        0x14    // Command Completion Coalescing Control
#define AHCI_REG_CCC_PORTS      0x18    // Command Completion Coalescing Ports
#define AHCI_REG_EM_LOC         0x1C    // Enclosure Management Location
#define AHCI_REG_EM_CTL         0x20    // Enclosure Management Control
#define AHCI_REG_CAP2           0x24    // Host Capabilities Extended
#define AHCI_REG_BOHC           0x28    // BIOS/OS Handoff Control and Status

// AHCI Port Register offsets (relative to port base)
#define AHCI_PORT_BASE          0x100   // First port register offset
#define AHCI_PORT_SIZE          0x80    // Size of each port register block

#define AHCI_PxCLB              0x00    // Command List Base Address (low)
#define AHCI_PxCLBU             0x04    // Command List Base Address (high)
#define AHCI_PxFB               0x08    // FIS Base Address (low)
#define AHCI_PxFBU              0x0C    // FIS Base Address (high)
#define AHCI_PxIS               0x10    // Interrupt Status
#define AHCI_PxIE               0x14    // Interrupt Enable
#define AHCI_PxCMD              0x18    // Command and Status
#define AHCI_PxTFD              0x20    // Task File Data
#define AHCI_PxSIG              0x24    // Signature
#define AHCI_PxSSTS             0x28    // SATA Status (SCR0: SStatus)
#define AHCI_PxSCTL             0x2C    // SATA Control (SCR2: SControl)
#define AHCI_PxSERR             0x30    // SATA Error (SCR1: SError)
#define AHCI_PxSACT             0x34    // SATA Active (SCR3: SActive)
#define AHCI_PxCI               0x38    // Command Issue
#define AHCI_PxSNTF             0x3C    // SATA Notification
#define AHCI_PxFBS              0x40    // FIS-based Switching Control
#define AHCI_PxDEVSLP           0x44    // Device Sleep
#define AHCI_PxVS               0x70    // Vendor Specific

// AHCI GHC register bits
#define AHCI_GHC_AE             (1 << 31)   // AHCI Enable
#define AHCI_GHC_MRSM           (1 << 2)    // MSI Revert to Single Message
#define AHCI_GHC_IE             (1 << 1)    // Interrupt Enable
#define AHCI_GHC_HR             (1 << 0)    // HBA Reset

// AHCI CAP register bits
#define AHCI_CAP_S64A           (1 << 31)   // 64-bit Addressing
#define AHCI_CAP_SNCQ           (1 << 30)   // Native Command Queuing
#define AHCI_CAP_SSNTF          (1 << 29)   // SNotification Register
#define AHCI_CAP_SMPS           (1 << 28)   // Mechanical Presence Switch
#define AHCI_CAP_SSS            (1 << 27)   // Staggered Spin-up
#define AHCI_CAP_SALP           (1 << 26)   // Aggressive Link Power Management
#define AHCI_CAP_SAL            (1 << 25)   // Activity LED
#define AHCI_CAP_SCLO           (1 << 24)   // Command List Override
#define AHCI_CAP_ISS_MASK       (0xF << 20) // Interface Speed Support
#define AHCI_CAP_ISS_GEN1       (1 << 20)   // Gen 1 (1.5 Gbps)
#define AHCI_CAP_ISS_GEN2       (2 << 20)   // Gen 2 (3.0 Gbps)
#define AHCI_CAP_ISS_GEN3       (3 << 20)   // Gen 3 (6.0 Gbps)
#define AHCI_CAP_SAM            (1 << 18)   // AHCI Mode Only
#define AHCI_CAP_SPM            (1 << 17)   // Port Multiplier
#define AHCI_CAP_FBSS           (1 << 16)   // FIS-based Switching
#define AHCI_CAP_PMD            (1 << 15)   // PIO Multiple DRQ Block
#define AHCI_CAP_SSC            (1 << 14)   // Slumber State Capable
#define AHCI_CAP_PSC            (1 << 13)   // Partial State Capable
#define AHCI_CAP_CCCS           (1 << 7)    // Command Completion Coalescing

// AHCI PxCMD register bits
#define AHCI_PxCMD_ICC_MASK     (0xF << 28) // Interface Communication Control
#define AHCI_PxCMD_ASP          (1 << 27)   // Aggressive Slumber / Partial
#define AHCI_PxCMD_ALPE         (1 << 26)   // Aggressive Link Power Management Enable
#define AHCI_PxCMD_DLAE         (1 << 25)   // Drive LED on ATAPI Enable
#define AHCI_PxCMD_ATAPI        (1 << 24)   // Device is ATAPI
#define AHCI_PxCMD_APSTE        (1 << 23)   // Automatic Partial to Slumber Transitions Enable
#define AHCI_PxCMD_FBSCP        (1 << 22)   // FIS-based Switching Capable Port
#define AHCI_PxCMD_ESP          (1 << 21)   // External SATA Port
#define AHCI_PxCMD_CPD          (1 << 20)   // Cold Presence Detection
#define AHCI_PxCMD_MPSP         (1 << 19)   // Mechanical Presence Switch Attached
#define AHCI_PxCMD_HPCP         (1 << 18)   // Hot Plug Capable Port
#define AHCI_PxCMD_PMA          (1 << 17)   // Port Multiplier Attached
#define AHCI_PxCMD_CPS          (1 << 16)   // Cold Presence State
#define AHCI_PxCMD_CR           (1 << 15)   // Command List Running
#define AHCI_PxCMD_FR           (1 << 14)   // FIS Receive Running
#define AHCI_PxCMD_MPSS         (1 << 13)   // Mechanical Presence Switch State
#define AHCI_PxCMD_CCS_MASK     (0x1F << 8) // Current Command Slot
#define AHCI_PxCMD_FRE          (1 << 4)    // FIS Receive Enable
#define AHCI_PxCMD_CLO          (1 << 3)    // Command List Override
#define AHCI_PxCMD_POD          (1 << 2)    // Power On Device
#define AHCI_PxCMD_SUD          (1 << 1)    // Spin-Up Device
#define AHCI_PxCMD_ST           (1 << 0)    // Start

// AHCI PxIS (Interrupt Status) bits
#define AHCI_PxIS_CPDS          (1 << 31)   // Cold Port Detect Status
#define AHCI_PxIS_TFES          (1 << 30)   // Task File Error Status
#define AHCI_PxIS_HBFS          (1 << 29)   // Host Bus Fatal Error Status
#define AHCI_PxIS_HBDS          (1 << 28)   // Host Bus Data Error Status
#define AHCI_PxIS_IFS           (1 << 27)   // Interface Fatal Error Status
#define AHCI_PxIS_INFS          (1 << 26)   // Interface Non-fatal Error Status
#define AHCI_PxIS_OFS           (1 << 24)   // Overflow Status
#define AHCI_PxIS_IPMS          (1 << 23)   // Incorrect Port Multiplier Status
#define AHCI_PxIS_PRCS          (1 << 22)   // PhyRdy Change Status
#define AHCI_PxIS_DMPS          (1 << 7)    // Device Mechanical Presence Status
#define AHCI_PxIS_PCS           (1 << 6)    // Port Connect Change Status
#define AHCI_PxIS_DPS           (1 << 5)    // Descriptor Processed
#define AHCI_PxIS_UFS           (1 << 4)    // Unknown FIS Interrupt
#define AHCI_PxIS_SDBS          (1 << 3)    // Set Device Bits Interrupt
#define AHCI_PxIS_DSS           (1 << 2)    // DMA Setup FIS Interrupt
#define AHCI_PxIS_PSS           (1 << 1)    // PIO Setup FIS Interrupt
#define AHCI_PxIS_DHRS          (1 << 0)    // Device to Host Register FIS Interrupt

// AHCI PxTFD (Task File Data) bits
#define AHCI_PxTFD_ERR_MASK     (0xFF << 8) // Error
#define AHCI_PxTFD_STS_BSY      (1 << 7)    // Busy
#define AHCI_PxTFD_STS_DRQ      (1 << 3)    // Data Request
#define AHCI_PxTFD_STS_ERR      (1 << 0)    // Error

// AHCI PxSSTS (SStatus) bits
#define AHCI_PxSSTS_IPM_MASK    (0xF << 8)  // Interface Power Management
#define AHCI_PxSSTS_SPD_MASK    (0xF << 4)  // Current Interface Speed
#define AHCI_PxSSTS_DET_MASK    (0xF)       // Device Detection
#define AHCI_PxSSTS_DET_NONE    0           // No device detected
#define AHCI_PxSSTS_DET_PRESENT 1           // Device present but no Phy
#define AHCI_PxSSTS_DET_ONLINE  3           // Device present and Phy established

// AHCI SATA mode types
#define AHCI_SATA_MODE_IDE      0           // IDE/Legacy mode
#define AHCI_SATA_MODE_AHCI     1           // AHCI mode

// AHCI Port State structure
typedef struct {
  Bit32u clb;           // Command List Base Address (low)
  Bit32u clbu;          // Command List Base Address (high)
  Bit32u fb;            // FIS Base Address (low)
  Bit32u fbu;           // FIS Base Address (high)
  Bit32u is;            // Interrupt Status
  Bit32u ie;            // Interrupt Enable
  Bit32u cmd;           // Command and Status
  Bit32u tfd;           // Task File Data
  Bit32u sig;           // Signature
  Bit32u ssts;          // SATA Status (SStatus)
  Bit32u sctl;          // SATA Control (SControl)
  Bit32u serr;          // SATA Error (SError)
  Bit32u sact;          // SATA Active
  Bit32u ci;            // Command Issue
  Bit32u sntf;          // SATA Notification
  Bit32u fbs;           // FIS-based Switching Control
  bool   device_present; // Device attached to this port
} ahci_port_t;

// AHCI HBA State structure
typedef struct {
  Bit32u cap;           // Host Capabilities
  Bit32u ghc;           // Global Host Control
  Bit32u is;            // Interrupt Status
  Bit32u pi;            // Ports Implemented
  Bit32u vs;            // AHCI Version
  Bit32u ccc_ctl;       // Command Completion Coalescing Control
  Bit32u ccc_ports;     // Command Completion Coalescing Ports
  Bit32u em_loc;        // Enclosure Management Location
  Bit32u em_ctl;        // Enclosure Management Control
  Bit32u cap2;          // Host Capabilities Extended
  Bit32u bohc;          // BIOS/OS Handoff Control
  ahci_port_t port[AHCI_MAX_PORTS];
} ahci_state_t;

class bx_pci_ide_c : public bx_pci_ide_stub_c {
public:
  bx_pci_ide_c();
  virtual ~bx_pci_ide_c();
  virtual void init(void);
  virtual void reset(unsigned type);
  virtual bool bmdma_present(void);
  virtual void bmdma_start_transfer(Bit8u channel);
  virtual void bmdma_set_irq(Bit8u channel);
  virtual void register_state(void);
  virtual void after_restore_state(void);
  static Bit64s param_save_handler(void *devptr, bx_param_c *param);
  static void param_restore_handler(void *devptr, bx_param_c *param, Bit64s val);
#if !BX_USE_PIDE_SMF
  Bit64s param_save(bx_param_c *param);
  void param_restore(bx_param_c *param, Bit64s val);
#endif

  virtual void pci_write_handler(Bit8u address, Bit32u value, unsigned io_len);

  static void timer_handler(void *);
  BX_PIDE_SMF void timer(void);

private:

  Bit8u devfunc;

  struct {
    unsigned chipset;
    unsigned sata_mode;  // IDE or AHCI mode
    struct {
      bool cmd_ssbm;
      bool cmd_rwcon;
      Bit8u  status;
      Bit32u dtpr;
      Bit32u prd_current;
      int timer_index;
      Bit8u *buffer;
      Bit8u *buffer_top;
      Bit8u *buffer_idx;
      bool data_ready;
    } bmdma[2];
    ahci_state_t ahci;   // AHCI HBA state
  } s;

  // AHCI functions
  BX_PIDE_SMF void ahci_reset(void);
  BX_PIDE_SMF void ahci_port_reset(int port);
  BX_PIDE_SMF Bit32u ahci_read(Bit32u offset, unsigned io_len);
  BX_PIDE_SMF void ahci_write(Bit32u offset, Bit32u value, unsigned io_len);
  BX_PIDE_SMF Bit32u ahci_port_read(int port, Bit32u offset);
  BX_PIDE_SMF void ahci_port_write(int port, Bit32u offset, Bit32u value);
  BX_PIDE_SMF void ahci_check_cmd(int port);
  BX_PIDE_SMF void ahci_process_cmd(int port, int slot);
  BX_PIDE_SMF void ahci_port_irq(int port);
  BX_PIDE_SMF void ahci_update_irq(void);

  static Bit32u read_handler(void *this_ptr, Bit32u address, unsigned io_len);
  static void   write_handler(void *this_ptr, Bit32u address, Bit32u value, unsigned io_len);
  static bool   ahci_read_handler(bx_phy_address addr, unsigned len, void *data, void *param);
  static bool   ahci_write_handler(bx_phy_address addr, unsigned len, void *data, void *param);
#if !BX_USE_PIDE_SMF
  Bit32u read(Bit32u address, unsigned io_len);
  void   write(Bit32u address, Bit32u value, unsigned io_len);
#endif
};

#endif
