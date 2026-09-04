/////////////////////////////////////////////////////////////////////////
// $Id$
/////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025-2026  The Bochs Project
//
//  ATI Radeon 9800 (R350) -- register and identity constants.
//
//  Register offsets and field layouts follow the Radeon 9800 Databook,
//  the R3xx 3D Register Reference Guide, the Linux DRM radeon / r300
//  sources (radeon_reg.h, r300_reg.h, r100d.h, r300d.h), the XFree86 /
//  Haiku Radeon drivers and the Mesa r300 driver.
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

#ifndef BX_IODEV_RADEON9800_REGS_H
#define BX_IODEV_RADEON9800_REGS_H

// ------------------------------------------------------------------
// PCI identity (R350 "NH": Radeon 9800 PRO / 9800 XL boards)
// ------------------------------------------------------------------
#define R9800_PCI_VENDOR        0x1002
#define R9800_PCI_DEVICE        0x4e48  // R350 [Radeon 9800 PRO] primary
#define R9800_PCI_DEVICE_SEC    0x4e68  // R350 [Radeon 9800 PRO] secondary
#define R9800_PCI_REVISION      0x00
// Subsystem identity taken from the ROM straps of the Medion 9800 XL
// image (SUBSYS_VEN_ID / SUBSYS_ID at ROM offset 0x76 / 0x78):
// Hightech Information System (HIS) 0x1787, board 0x8500
#define R9800_SUBSYS_VENDOR     0x1787
#define R9800_SUBSYS_ID         0x8500

// Capability chain: CAP_PTR 0x58 -> AGP 2.0 (next 0x50) -> PM v2 (next 0)
#define R9800_PCI_CAP_PTR       0x58
#define R9800_AGP_CAP_OFF       0x58
#define R9800_AGP_CAP_ID        0x02
#define R9800_AGP_NEXT_PTR      0x50
#define R9800_AGP_REV           0x20
// AGP status: RQ=80 (0x4f<<24), SBA, FW, rates x1/x2/x4
#define R9800_AGP_STATUS        0x4f000217
#define R9800_AGP_COMMAND_MASK  0xff000317
#define R9800_PM_CAP_OFF        0x50
#define R9800_PM_CAP_ID         0x01
#define R9800_PM_PMC            0x0602  // PM v2, D1 and D2 supported
#define R9800_PM_CAP_OFF_SEC    0x50

// BAR layout (both functions): BAR0 = 128 MB prefetchable frame buffer
// aperture, BAR1 = 256 byte IO block (fn 0) / 64 KB register aperture
// (fn 1), BAR2 = 64 KB register aperture (fn 0). ROM 128 KB.
#define R9800_LFB_SIZE          0x08000000
#define R9800_IO_SIZE           0x00000100
#define R9800_MMIO_SIZE         0x00010000
#define R9800_REG_APER_MASK     0x0000ffff

// Reference oscillator (XTALIN)
#define R9800_REF_FREQ_HZ       27000000.0

// ------------------------------------------------------------------
// Register file offsets
// ------------------------------------------------------------------
#define R9800_MM_INDEX              0x0000
#define R9800_MM_DATA               0x0004
#define R9800_MM_INDEX_MM_APER      0x80000000
#define R9800_MM_INDEX_MM_ADDR      0x7ffffffc
#define R9800_CLOCK_CNTL_INDEX      0x0008
#define R9800_CLOCK_CNTL_DATA       0x000c
#define R9800_PLL_ADDR_MASK         0x3f
#define R9800_PLL_WR_EN             (1 << 7)
#define R9800_PLL_DIV_SEL_SHIFT     8
#define R9800_BIOS_0_SCRATCH        0x0010
#define R9800_BIOS_7_SCRATCH        0x002c
#define R9800_BUS_CNTL              0x0030
#define R9800_BUS_CNTL1             0x0034
#define R9800_BUS_CNTL_DEFAULT      0x880f4d43
#define R9800_BUS_CNTL_WO_MASK      0x00000006
#define R9800_BUS_MASTER_DIS        (1 << 6)
#define R9800_MEM_VGA_WP_SEL        0x0038
#define R9800_MEM_VGA_RP_SEL        0x003c
#define R9800_VGA_PAGE_SIZE         0x2000
#define R9800_GEN_INT_CNTL          0x0040
#define R9800_GEN_INT_STATUS        0x0044
#define R9800_INT_CRTC_VBLANK       (1u << 0)
#define R9800_INT_CRTC_VLINE        (1u << 1)
#define R9800_INT_CRTC_VSYNC        (1u << 2)
#define R9800_INT_SNAPSHOT          (1u << 3)
#define R9800_INT_FP_DETECT         (1u << 4)
#define R9800_INT_CRTC2_VLINE       (1u << 5)
#define R9800_INT_CRTC2_VSYNC       (1u << 6)
#define R9800_INT_SNAPSHOT2         (1u << 7)
#define R9800_INT_CAP0              (1u << 8)
#define R9800_INT_CRTC2_VBLANK      (1u << 9)
#define R9800_INT_FP2_DETECT        (1u << 10)
#define R9800_INT_VSYNC_DIFF        (1u << 11)
#define R9800_INT_DMA_VIPH0         (1u << 12)
#define R9800_INT_I2C               (1u << 17)
#define R9800_INT_GUI_IDLE          (1u << 19)
#define R9800_INT_VIPH              (1u << 24)
#define R9800_INT_SW                (1u << 25)
#define R9800_INT_SW_FIRE           (1u << 26)
#define R9800_INT_GEYSERVILLE       (1u << 27)
#define R9800_INT_HDCP              (1u << 28)
#define R9800_INT_DVI_I2C           (1u << 29)
#define R9800_INT_GUIDMA            (1u << 30)
#define R9800_INT_VIDDMA            (1u << 31)
#define R9800_INT_SUPPORTED   (R9800_INT_CRTC_VBLANK | R9800_INT_CRTC_VLINE | R9800_INT_CRTC_VSYNC | \
                               R9800_INT_SNAPSHOT | R9800_INT_FP_DETECT | R9800_INT_CRTC2_VLINE | \
                               R9800_INT_CRTC2_VSYNC | R9800_INT_SNAPSHOT2 | R9800_INT_CRTC2_VBLANK | \
                               R9800_INT_FP2_DETECT | R9800_INT_GUI_IDLE | R9800_INT_SW | \
                               R9800_INT_I2C | R9800_INT_DVI_I2C)
#define R9800_INT_ACK_MASK    (R9800_INT_SUPPORTED | R9800_INT_VSYNC_DIFF | R9800_INT_CAP0 | \
                               R9800_INT_GUIDMA | R9800_INT_VIDDMA | R9800_INT_VIPH | R9800_INT_GEYSERVILLE | R9800_INT_HDCP)

#define R9800_CRTC_GEN_CNTL         0x0050
#define R9800_CRTC_EXT_CNTL         0x0054
#define R9800_DAC_CNTL              0x0058
#define R9800_CRTC_STATUS           0x005c
#define R9800_GPIO_VGA_DDC          0x0060
#define R9800_GPIO_DVI_DDC          0x0064
#define R9800_GPIO_MONID            0x0068
#define R9800_GPIO_CRT2_DDC         0x006c
#define R9800_BUS_CNTL_MSTR         0x0070
#define R9800_DAC_CNTL2             0x007c
#define R9800_VIPH_REG_ADDR         0x0080
#define R9800_VIPH_REG_DATA         0x0084
#define R9800_I2C_CNTL_0            0x0090
#define R9800_I2C_CNTL_1            0x0094
#define R9800_I2C_DATA              0x0098
#define R9800_PALETTE_INDEX         0x00b0
#define R9800_PALETTE_DATA          0x00b4
#define R9800_PALETTE_30_DATA       0x00b8
#define R9800_CONFIG_CNTL           0x00e0
#define R9800_CFG_VGA_RAM_EN        (1 << 8)
#define R9800_CFG_VGA_IO_DIS        (1 << 9)
#define R9800_CFG_ATI_REV_SHIFT     16
#define R9800_CFG_ATI_REV_MASK      0x000f0000
#define R9800_CONFIG_XSTRAP         0x00e4
#define R9800_CONFIG_BONDS          0x00e8
#define R9800_RBBM_SOFT_RESET       0x00f0
#define R9800_SOFT_RESET_CP         (1 << 0)
#define R9800_SOFT_RESET_HI         (1 << 1)
#define R9800_SOFT_RESET_SE         (1 << 2)
#define R9800_SOFT_RESET_RE         (1 << 3)
#define R9800_SOFT_RESET_PP         (1 << 4)
#define R9800_SOFT_RESET_E2         (1 << 5)
#define R9800_SOFT_RESET_RB         (1 << 6)
#define R9800_SOFT_RESET_HDP        (1 << 7)
#define R9800_SOFT_RESET_MC         (1 << 8)
#define R9800_SOFT_RESET_AIC        (1 << 9)
#define R9800_SOFT_RESET_VIP        (1 << 10)
#define R9800_SOFT_RESET_DISP       (1 << 11)
#define R9800_SOFT_RESET_CG         (1 << 12)
#define R9800_SOFT_RESET_GA         (1 << 13)
#define R9800_SOFT_RESET_IDCT       (1 << 14)
#define R9800_RBBM_STATUS_ALT       0x00f4
#define R9800_CONFIG_MEMSIZE        0x00f8
#define R9800_CONFIG_APER_0_BASE    0x0100
#define R9800_CONFIG_APER_1_BASE    0x0104
#define R9800_CONFIG_APER_SIZE      0x0108
#define R9800_CONFIG_REG_1_BASE     0x010c
#define R9800_CONFIG_REG_APER_SIZE  0x0110
#define R9800_CONFIG_MEMSIZE_EMB    0x0114
#define R9800_TEST_DEBUG_CNTL       0x0120
#define R9800_TEST_DEBUG_MUX        0x0124
#define R9800_HW_DEBUG              0x0128
#define R9800_TEST_DEBUG_OUT        0x012c
#define R9800_HOST_PATH_CNTL        0x0130
#define R9800_HDP_APER_CNTL         (1 << 23)
#define R9800_HDP_SOFT_RESET        (1 << 26)
#define R9800_HDP_READ_BUFFER_INV   (1 << 27)
#define R9800_HDP_FB_LOCATION       0x0134
#define R9800_MEM_CNTL              0x0140
#define R9800_MEM_TIMING_CNTL       0x0144
#define R9800_MC_FB_LOCATION        0x0148
#define R9800_MC_AGP_LOCATION       0x014c
#define R9800_MC_STATUS             0x0150
#define R9800_MC_IDLE               (1 << 4)
#define R9800_MEM_INIT_LAT_TIMER    0x0154
#define R9800_MEM_SDRAM_MODE_REG    0x0158
#define R9800_MEM_CFG_TYPE_DDR      (1u << 30)
#define R9800_AGP_BASE_2            0x015c
#define R9800_MEM_IO_CNTL_A0        0x0160
#define R9800_MEM_IO_CNTL_A1        0x0164
#define R9800_MEM_IO_CNTL_B0        0x0168
#define R9800_MEM_IO_CNTL_B1        0x016c
#define R9800_AGP_BASE              0x0170
#define R9800_AGP_CNTL              0x0174
#define R9800_MC_READ_CNTL_AB       0x017c
#define R9800_MC_INIT_MISC_LAT_TIMER 0x0180
#define R9800_MC_INIT_GFX_LAT_TIMER 0x0184
#define R9800_MC_DEBUG              0x0188
#define R9800_MC_CHP_IO_OE_CNTL_AB  0x018c
#define R9800_VIDEOMUX_CNTL         0x0190
#define R9800_MDGPIO_MASK           0x01a8
#define R9800_MDGPIO_A              0x01ac
#define R9800_MDGPIO_EN             0x01b0
#define R9800_MDGPIO_Y              0x01b4
#define R9800_SEPROM_CNTL1          0x01c0
#define R9800_SEPROM_CNTL2          0x01c4
#define R9800_MPP_GP_CONFIG         0x01c8
#define R9800_MPP_TB_CONFIG_ALT     0x01cc
#define R9800_AIC_CNTL              0x01d0
#define R9800_AIC_TRANSLATE_EN      (1 << 0)
#define R9800_AIC_STAT              0x01d4
#define R9800_AIC_PT_BASE           0x01d8
#define R9800_AIC_LO_ADDR           0x01dc
#define R9800_AIC_HI_ADDR           0x01e0
#define R9800_AIC_TLB_ADDR          0x01e4
#define R9800_AIC_TLB_DATA          0x01e8
#define R9800_MC_IND_INDEX          0x01f8
#define R9800_MC_IND_ADDR_MASK      0x3f
#define R9800_MC_IND_WR_EN          (1 << 8)
#define R9800_MC_IND_DATA           0x01fc
#define R9800_MC_IND_REGS           0x40

// CRTC1
#define R9800_CRTC_H_TOTAL_DISP     0x0200
#define R9800_CRTC_H_SYNC_STRT_WID  0x0204
#define R9800_CRTC_V_TOTAL_DISP     0x0208
#define R9800_CRTC_V_SYNC_STRT_WID  0x020c
#define R9800_CRTC_VLINE_CRNT_VLINE 0x0210
#define R9800_CRTC_CRNT_FRAME       0x0214
#define R9800_CRTC_GUI_TRIG_VLINE   0x0218
#define R9800_CRTC_DEBUG            0x021c
#define R9800_CRTC_OFFSET_RIGHT     0x0220
#define R9800_CRTC_OFFSET           0x0224
#define R9800_CRTC_OFFSET_CNTL      0x0228
#define R9800_CRTC_PITCH            0x022c
#define R9800_OVR_CLR               0x0230
#define R9800_OVR_WID_LEFT_RIGHT    0x0234
#define R9800_OVR_WID_TOP_BOTTOM    0x0238
#define R9800_DISPLAY_BASE_ADDR     0x023c
#define R9800_SNAPSHOT_VH_COUNTS    0x0240
#define R9800_SNAPSHOT_F_COUNT      0x0244
#define R9800_N_VIF_COUNT           0x0248
#define R9800_SNAPSHOT_VIF_COUNT    0x024c
#define R9800_FP_CRTC_H_TOTAL_DISP  0x0250
#define R9800_FP_CRTC_V_TOTAL_DISP  0x0254
#define R9800_CRT_CRTC_H_SYNC_STRT_WID 0x0258
#define R9800_CRT_CRTC_V_SYNC_STRT_WID 0x025c
#define R9800_CUR_OFFSET            0x0260
#define R9800_CUR_HORZ_VERT_POSN    0x0264
#define R9800_CUR_HORZ_VERT_OFF     0x0268
#define R9800_CUR_CLR0              0x026c
#define R9800_CUR_CLR1              0x0270
#define R9800_FP_HORZ_VERT_ACTIVE   0x0278
#define R9800_CRTC_MORE_CNTL        0x027c
#define R9800_DAC_EXT_CNTL          0x0280
#define R9800_FP_GEN_CNTL           0x0284
#define R9800_FP2_GEN_CNTL          0x0288
#define R9800_FP_HORZ_STRETCH       0x028c
#define R9800_FP_VERT_STRETCH       0x0290
#define R9800_FP_H_SYNC_STRT_WID    0x02c4
#define R9800_FP_V_SYNC_STRT_WID    0x02c8
#define R9800_DAC_CRC_SIG           0x02cc
#define R9800_LVDS_GEN_CNTL         0x02d0
#define R9800_LVDS_PLL_CNTL         0x02d4
#define R9800_TMDS_CRC              0x02a0
#define R9800_TMDS_TRANSMITTER_CNTL 0x02a4
#define R9800_TMDS_PLL_CNTL         0x02a8
#define R9800_DVI_I2C_CNTL_0        0x02e0
#define R9800_DVI_I2C_CNTL_1        0x02e4
#define R9800_DVI_I2C_DATA          0x02e8
#define R9800_LVDS_SS_GEN_CNTL      0x02ec
#define R9800_GRPH_BUFFER_CNTL      0x02f0
#define R9800_VGA_BUFFER_CNTL       0x02f4
// CRTC2
#define R9800_CRTC2_H_TOTAL_DISP    0x0300
#define R9800_CRTC2_H_SYNC_STRT_WID 0x0304
#define R9800_CRTC2_V_TOTAL_DISP    0x0308
#define R9800_CRTC2_V_SYNC_STRT_WID 0x030c
#define R9800_CRTC2_VLINE_CRNT_VLINE 0x0310
#define R9800_CRTC2_CRNT_FRAME      0x0314
#define R9800_CRTC2_GUI_TRIG_VLINE  0x0318
#define R9800_CRTC2_DEBUG           0x031c
#define R9800_CRTC2_OFFSET          0x0324
#define R9800_CRTC2_OFFSET_CNTL     0x0328
#define R9800_CRTC2_PITCH           0x032c
#define R9800_OVR2_CLR              0x0330
#define R9800_OVR2_WID_LEFT_RIGHT   0x0334
#define R9800_OVR2_WID_TOP_BOTTOM   0x0338
#define R9800_DISPLAY2_BASE_ADDR    0x033c
#define R9800_SNAPSHOT2_VH_COUNTS   0x0340
#define R9800_SNAPSHOT2_F_COUNT     0x0344
#define R9800_CRTC_TILE_X0_Y0       0x0350
#define R9800_CRTC2_TILE_X0_Y0      0x0358
#define R9800_CUR2_OFFSET           0x0360
#define R9800_CUR2_HORZ_VERT_POSN   0x0364
#define R9800_CUR2_HORZ_VERT_OFF    0x0368
#define R9800_CUR2_CLR0             0x036c
#define R9800_CUR2_CLR1             0x0370
#define R9800_FP2_2_GEN_CNTL        0x0388
#define R9800_FP_HORZ2_STRETCH      0x038c
#define R9800_FP_VERT2_STRETCH      0x0390
#define R9800_FP_H2_SYNC_STRT_WID   0x03c4
#define R9800_FP_V2_SYNC_STRT_WID   0x03c8
#define R9800_GRPH2_BUFFER_CNTL     0x03f0
#define R9800_CRTC2_GEN_CNTL        0x03f8
#define R9800_CRTC2_STATUS          0x03fc

// CRTC field layout
#define R9800_CRTC_DBL_SCAN_EN      (1 << 0)
#define R9800_CRTC_INTERLACE_EN     (1 << 1)
#define R9800_CRTC_PIX_WIDTH_SHIFT  8
#define R9800_CRTC_ICON_EN          (1 << 15)
#define R9800_CRTC_CUR_EN           (1 << 16)
#define R9800_CRTC_CUR_MODE_SHIFT   20
#define R9800_CRTC_CUR_MODE_MASK    (7 << 20)
#define R9800_CRTC_EXT_DISP_EN      (1 << 24)
#define R9800_CRTC_EN               (1 << 25)
#define R9800_CRTC_DISP_REQ_EN_B    (1 << 26)
#define R9800_CRTC2_DISP_DIS        (1 << 23)
#define R9800_CRTC_GEN_CNTL_DEFAULT 0x04000000
#define R9800_CRTC_EXT_CNTL_DEFAULT 0x00008000
#define R9800_CRTC_VGA_XOVERSCAN    (1 << 0)
#define R9800_VGA_ATI_LINEAR        (1 << 3)
#define R9800_XCRT_CNT_EN           (1 << 6)
#define R9800_CRTC_HSYNC_DIS        (1 << 8)
#define R9800_CRTC_VSYNC_DIS        (1 << 9)
#define R9800_CRTC_DISPLAY_DIS      (1 << 10)
#define R9800_CRTC_SYNC_TRISTAT     (1 << 11)
#define R9800_CRTC_CRT_ON           (1 << 15)
#define R9800_VGA_MEM_PS_EN         (1 << 19)
#define R9800_DAC_CNTL_DEFAULT      0xff00000a
#define R9800_DAC_BLANKING          (1 << 2)
#define R9800_DAC_CMP_EN            (1 << 3)
#define R9800_DAC_CMP_OUTPUT        (1 << 7)
#define R9800_DAC_8BIT_EN           (1 << 8)
#define R9800_DAC_4BPP_PIX_ORDER    (1 << 9)
#define R9800_DAC_TVO_EN            (1 << 10)
#define R9800_DAC_VGA_ADR_EN        (1 << 13)
#define R9800_DAC_PDWN              (1 << 15)
#define R9800_DAC_CRC_EN            (1 << 19)
#define R9800_DAC2_PALETTE_ACC_CTL  (1 << 5)
#define R9800_DAC2_CMP_EN           (1 << 7)
#define R9800_DAC2_CMP_OUTPUT       (1 << 11)
#define R9800_CRTC_STATUS_DEFAULT   0x00000000
#define R9800_CRTC_TILE_EN          (1 << 15)
#define R9800_CRTC_OFFSET_FLIP_CNTL (1 << 16)
#define R9800_CRTC_X_Y_MODE_EN      (1 << 9)
#define R9800_CRTC_MICRO_TILE_BUFFER_DIS (3 << 10)
#define R9800_CRTC_MICRO_TILE_EN    (1 << 13)
#define R9800_CRTC_MACRO_TILE_EN    (1 << 15)
#define R9800_DAC_FORCE_BLANK_OFF_EN (1 << 4)
#define R9800_DAC_FORCE_DATA_EN     (1 << 5)
#define R9800_DAC_FORCE_DATA_SEL_SHIFT 6
#define R9800_DAC_FORCE_DATA_SHIFT  8
#define R9800_FP_DETECT_SENSE       (1 << 8)
#define R9800_FP2_DETECT_SENSE      (1 << 8)

// OV0 hardware video overlay
#define R9800_OV0_BLOCK_BASE        0x0400
#define R9800_OV0_BLOCK_END         0x04ff
#define R9800_OV0_REG(off)          (((off) - R9800_OV0_BLOCK_BASE) >> 2)
#define R9800_OV0_Y_X_START         0x0400
#define R9800_OV0_Y_X_END           0x0404
#define R9800_OV0_PIPELINE_CNTL     0x0408
#define R9800_OV0_EXCLUSIVE_VERT    0x040c
#define R9800_OV0_REG_LOAD_CNTL     0x0410
#define R9800_OV0_LD_CTL_LOCK               0x00000001
#define R9800_OV0_LD_CTL_VBLANK_DURING_LOCK 0x00000002
#define R9800_OV0_LD_CTL_STALL_GUI          0x00000004
#define R9800_OV0_LD_CTL_LOCK_READBACK      0x00000008
#define R9800_OV0_SCALE_CNTL        0x0420
#define R9800_OV0_SCALER_PICK_NEAREST_H 0x00000004
#define R9800_OV0_SCALER_PICK_NEAREST_V 0x00000008
#define R9800_OV0_SCALER_SIGNED_UV  0x00000010
#define R9800_OV0_SCALER_GAMMA_SEL_MASK 0x000000e0
#define R9800_OV0_SCALER_GAMMA_SEL_SHIFT 5
#define R9800_OV0_SCALER_FORMAT_MASK 0x00000f00
#define R9800_OV0_SCALER_FORMAT_SHIFT 8
#define R9800_OV0_FMT_15BPP         0x3
#define R9800_OV0_FMT_16BPP         0x4
#define R9800_OV0_FMT_32BPP         0x6
#define R9800_OV0_FMT_YUV9          0x9
#define R9800_OV0_FMT_YUV12         0xa
#define R9800_OV0_FMT_VYUY422       0xb
#define R9800_OV0_FMT_YVYU422       0xc
#define R9800_OV0_FMT_AYUV444       0xe
#define R9800_OV0_SCALER_ADAPTIVE_DEINT 0x00001000
#define R9800_OV0_SCALER_TEMPORAL_DEINT 0x00002000
#define R9800_OV0_SCALER_CRTC_SEL   0x00004000
#define R9800_OV0_SCALER_SMART_SWITCH 0x00008000
#define R9800_OV0_SCALER_DOUBLE_BUFFER 0x01000000
#define R9800_OV0_SCALER_DIS_LIMIT  0x08000000
#define R9800_OV0_SCALER_LIN_TRANS_BYPASS 0x10000000
#define R9800_OV0_SCALER_INT_EMU    0x20000000
#define R9800_OV0_SCALER_ENABLE     0x40000000
#define R9800_OV0_SCALER_SOFT_RESET 0x80000000
#define R9800_OV0_V_INC             0x0424
#define R9800_OV0_P1_V_ACCUM_INIT   0x0428
#define R9800_OV0_P23_V_ACCUM_INIT  0x042c
#define R9800_OV0_P1_BLANK_LINES_AT_TOP  0x0430
#define R9800_OV0_P23_BLANK_LINES_AT_TOP 0x0434
#define R9800_OV0_BASE_ADDR         0x043c
#define R9800_OV0_VID_BUF0_BASE_ADRS 0x0440
#define R9800_OV0_VID_BUF_PITCH0_VALUE 0x0460
#define R9800_OV0_VID_BUF_PITCH1_VALUE 0x0464
#define R9800_OV0_AUTO_FLIP_CNTL    0x0470
#define R9800_OV0_DEINTERLACE_PATTERN 0x0474
#define R9800_OV0_SUBMIT_HISTORY    0x0478
#define R9800_OV0_H_INC             0x0480
#define R9800_OV0_STEP_BY           0x0484
#define R9800_OV0_P1_H_ACCUM_INIT   0x0488
#define R9800_OV0_P23_H_ACCUM_INIT  0x048c
#define R9800_OV0_P1_X_START_END    0x0494
#define R9800_OV0_P2_X_START_END    0x0498
#define R9800_OV0_P3_X_START_END    0x049c
#define R9800_OV0_FILTER_CNTL       0x04a0
#define R9800_OV0_FOUR_TAP_COEF_0   0x04b0
#define R9800_OV0_FOUR_TAP_COEF_4   0x04c0
#define R9800_OV0_FLAG_CNTL         0x04dc
#define R9800_OV0_SLICE_CNTL        0x04d0
#define R9800_OV0_VID_KEY_CLR_LOW   0x04e4
#define R9800_OV0_VID_KEY_CLR_HIGH  0x04e8
#define R9800_OV0_GRPH_KEY_CLR_LOW  0x04ec
#define R9800_OV0_GRPH_KEY_CLR_HIGH 0x04f0
#define R9800_OV0_COLOUR_CNTL       0x04e0
#define R9800_OV0_KEY_CNTL          0x04f4
#define R9800_OV0_VIDEO_KEY_FN_MASK 0x00000003
#define R9800_OV0_GRAPHIC_KEY_FN_MASK 0x00000030
#define R9800_OV0_GRAPHIC_KEY_FN_SHIFT 4
#define R9800_OV0_CMP_MIX_AND       0x00000100
#define R9800_OV0_TEST              0x04f8
#define R9800_ECP_DIV_SHIFT         8
#define R9800_OV0_LIN_TRANS_A       0x0d20
#define R9800_OV0_LIN_TRANS_F       0x0d34
#define R9800_OV0_GAMMA_000_00F     0x0d40
#define R9800_OV0_GAMMA_040_07F     0x0d4c
#define R9800_OV0_GAMMA_380_3BF     0x0d50
#define R9800_OV0_GAMMA_3C0_3FF     0x0d54
#define R9800_OV0_GAMMA_080_0BF     0x0e00
#define R9800_OV0_GAMMA_340_37F     0x0e2c

// DVD subpicture block
#define R9800_SUBPIC_BLOCK_BASE     0x0540
#define R9800_SUBPIC_BLOCK_END      0x0588
#define R9800_SUBPIC_REG(off)       (((off) - R9800_SUBPIC_BLOCK_BASE) >> 2)
#define R9800_SUBPIC_CNTL           0x0540
#define R9800_SUBPIC_CNTL_DISPLAY_EN 0x00000001
#define R9800_SUBPIC_CNTL_NEW_FRAME 0x00000002
#define R9800_SUBPIC_COLOR_CONTRAST 0x0544
#define R9800_SUBPIC_DAREA_START    0x054c
#define R9800_SUBPIC_DAREA_END      0x0550
#define R9800_SUBPIC_V_STEP         0x0554
#define R9800_SUBPIC_H_STEP         0x0558
#define R9800_SUBPIC_PXD_A          0x055c
#define R9800_SUBPIC_PITCH_LENGTH   0x056c
#define R9800_SUBPIC_HL_COLOR_CONTRAST 0x0570
#define R9800_SUBPIC_HL_TOP         0x0574
#define R9800_SUBPIC_HL_BOTTOM      0x0578
#define R9800_SUBPIC_PALETTE_INDEX  0x057c
#define R9800_SUBPIC_PALETTE_DATA   0x0580
#define R9800_SUBPIC_H_ACC          0x0584
#define R9800_SUBPIC_V_ACC          0x0588

// OV1 (second overlay window, register storage only)
#define R9800_OV1_Y_X_START         0x0600
#define R9800_OV1_Y_X_END           0x0604
#define R9800_OV1_PIPELINE_CNTL     0x0608

// Command processor
#define R9800_CP_RB_BASE            0x0700
#define R9800_CP_RB_CNTL            0x0704
#define R9800_RB_BUFSZ_MASK         0x3f
#define R9800_RB_BLKSZ_SHIFT        8
#define R9800_BUF_SWAP_SHIFT        16
#define R9800_MAX_FETCH_SHIFT       18
#define R9800_RB_NO_UPDATE          (1 << 27)
#define R9800_RB_RPTR_WR_ENA        (1u << 31)
#define R9800_CP_RB_RPTR_ADDR       0x070c
#define R9800_CP_RB_RPTR            0x0710
#define R9800_CP_RB_WPTR            0x0714
#define R9800_CP_RB_WPTR_DELAY      0x0718
#define R9800_CP_RB_RPTR_WR         0x071c
#define R9800_CP_IB_BASE            0x0738
#define R9800_CP_IB_BUFSZ           0x073c
#define R9800_CP_CSQ_CNTL           0x0740
#define R9800_CSQ_MODE_SHIFT        28
#define R9800_CSQ_PRIDIS_INDDIS     0
#define R9800_CSQ_PRIPIO_INDDIS     1
#define R9800_CSQ_PRIBM_INDDIS      2
#define R9800_CSQ_PRIPIO_INDBM      3
#define R9800_CSQ_PRIBM_INDBM       4
#define R9800_CSQ_PRIPIO_INDPIO     15
#define R9800_CP_CSQ_MODE           0x0744
#define R9800_SCRATCH_UMSK          0x0770
#define R9800_SCRATCH_ADDR          0x0774
#define R9800_CP_RESYNC_ADDR        0x0778
#define R9800_CP_RESYNC_DATA        0x077c
#define R9800_CP_ME_CNTL            0x07d0
#define R9800_CP_ME_RAM_ADDR        0x07d4
#define R9800_CP_ME_RAM_RADDR       0x07d8
#define R9800_CP_ME_RAM_DATAH       0x07dc
#define R9800_CP_ME_RAM_DATAL       0x07e0
#define R9800_CP_STAT               0x07c0
#define R9800_CP_STAT_CSF_PRIMARY_BUSY (1 << 9)
#define R9800_CP_STAT_CSQ_PRIMARY_BUSY (1 << 11)
#define R9800_CP_STAT_CSI_BUSY      (1 << 13)
#define R9800_CP_STAT_CMDSTRM_BUSY  (1 << 30)
#define R9800_CP_STAT_CP_BUSY       (1u << 31)
#define R9800_CP_CSQ_ADDR           0x07f0
#define R9800_CP_CSQ_DATA           0x07f4
#define R9800_CP_CSQ_STAT           0x07f8
#define R9800_CP_CSQ2_STAT          0x07fc
#define R9800_CP_CSQ_APER_PRIMARY   0x1000
#define R9800_CP_CSQ_APER_PRIMARY_END 0x11fc
#define R9800_CP_CSQ_APER_INDIRECT  0x1300
#define R9800_CP_CSQ_APER_INDIRECT_END 0x13fc
#define R9800_CP_ME_RAM_SIZE        256

// PM4 packet headers
#define R9800_PM4_TYPE(h)           ((h) >> 30)
#define R9800_PM4_COUNT(h)          ((((h) >> 16) & 0x3fff) + 1)
#define R9800_PM4_T0_REG(h)         (((h) & 0x1fff) * 4)
#define R9800_PM4_T0_ONE_REG_WR     0x00008000
#define R9800_PM4_T1_REG0(h)        (((h) & 0x7ff) * 4)
#define R9800_PM4_T1_REG1(h)        ((((h) >> 11) & 0x7ff) * 4)
#define R9800_PM4_T3_OPCODE(h)      (((h) >> 8) & 0xff)
#define R9800_PM4_OP_NOP            0x10
#define R9800_PM4_OP_NEXT_CHAR      0x19
#define R9800_PM4_OP_PLY_NEXTSCAN   0x1d
#define R9800_PM4_OP_SET_SCISSORS   0x1e
#define R9800_PM4_OP_3D_RNDR_GEN_INDX_PRIM 0x23
#define R9800_PM4_OP_LOAD_MICROCODE 0x24
#define R9800_PM4_OP_3D_RNDR_GEN_PRIM 0x25
#define R9800_PM4_OP_WAIT_FOR_IDLE  0x26
#define R9800_PM4_OP_3D_DRAW_VBUF   0x28
#define R9800_PM4_OP_3D_DRAW_IMMD   0x29
#define R9800_PM4_OP_3D_DRAW_INDX   0x2a
#define R9800_PM4_OP_LOAD_PALETTE   0x2c
#define R9800_PM4_OP_3D_LOAD_VBPNTR 0x2f
#define R9800_PM4_OP_3D_CLEAR_ZMASK 0x32
#define R9800_PM4_OP_INDX_BUFFER    0x33
#define R9800_PM4_OP_3D_DRAW_VBUF_2 0x34
#define R9800_PM4_OP_3D_DRAW_IMMD_2 0x35
#define R9800_PM4_OP_3D_DRAW_INDX_2 0x36
#define R9800_PM4_OP_3D_CLEAR_HIZ   0x37
#define R9800_PM4_OP_3D_CLEAR_CMASK 0x38
#define R9800_PM4_OP_3D_DRAW_128    0x39
#define R9800_PM4_OP_CNTL_PAINT     0x91
#define R9800_PM4_OP_CNTL_BITBLT    0x92
#define R9800_PM4_OP_CNTL_SMALLTEXT 0x93
#define R9800_PM4_OP_CNTL_HOSTDATA_BLT 0x94
#define R9800_PM4_OP_CNTL_POLYLINE  0x95
#define R9800_PM4_OP_CNTL_SCALING   0x96
#define R9800_PM4_OP_CNTL_TRANS_SCALING 0x97
#define R9800_PM4_OP_CNTL_POLYSCANLINES 0x98
#define R9800_PM4_OP_CNTL_PAINT_MULTI 0x9a
#define R9800_PM4_OP_CNTL_BITBLT_MULTI 0x9b
#define R9800_PM4_OP_CNTL_TRANS_BITBLT 0x9c

// TV out block (register storage only)
#define R9800_TV_BLOCK_BASE         0x0800
#define R9800_TV_BLOCK_END          0x08fc
#define R9800_TV_DAC_CNTL           0x088c
#define R9800_TV_DAC_DACDET_MASK    0xe0000000

// Capture (register storage only)
#define R9800_CAP_INT_CNTL          0x0908
#define R9800_CAP_INT_STATUS        0x090c
#define R9800_FCP_CNTL              0x0910
#define R9800_CAP0_BLOCK_BASE       0x0920
#define R9800_CAP0_BLOCK_END        0x09fc

// Surface translation windows / AGP
#define R9800_SURFACE_CNTL          0x0b00
#define R9800_SURF_NONSURF_AP0_SWP  (3 << 20)
#define R9800_SURF_NONSURF_AP1_SWP  (3 << 22)
#define R9800_SURFACE0_LOWER_BOUND  0x0b04
#define R9800_SURFACE7_INFO         0x0b7c
#define R9800_SURF_TILE_MACRO       (1 << 16)
#define R9800_SURF_TILE_MICRO       (2 << 16)
#define R9800_SURF_TILE_MASK        (3 << 16)
#define R9800_SURF_AP0_SWP          (3 << 20)
#define R9800_SURF_AP1_SWP          (3 << 22)
#define R9800_SURF_PITCH_MASK       0x00003fff
#define R9800_SURFACE_ACCESS_FLAGS  0x0bf8
#define R9800_SURFACE_ACCESS_CLR    0x0bfc

// VIPH data path (register storage only)
#define R9800_VIPH_CH0_DATA         0x0c00
#define R9800_VIPH_TIMEOUT_STAT     0x0c50

// Display misc
#define R9800_DISP_MISC_CNTL        0x0d00
#define R9800_DAC_MACRO_CNTL        0x0d04
#define R9800_DISP_PWR_MAN          0x0d08
#define R9800_DISP_TEST_DEBUG_CNTL  0x0d10
#define R9800_DISP_HW_DEBUG         0x0d14
#define R9800_DISP_MERGE_CNTL       0x0d60
#define R9800_DISP_OUTPUT_CNTL      0x0d64
#define R9800_DISP_DAC_SOURCE_MASK  0x03
#define R9800_DISP_DAC2_SOURCE_MASK 0x0c
#define R9800_DISP_TV_OUT_CNTL      0x0d6c
#define R9800_DISP2_MERGE_CNTL      0x0d68
#define R9800_DISP_LIN_TRANS_GRPH_A 0x0d80
#define R9800_DISP_LIN_TRANS_GRPH_F 0x0d98
#define R9800_DISP_LIN_TRANS_GRPH_G 0x0d9c
#define R9800_DISP_LIN_TRANS_GRPH_H 0x0da0

#define R9800_RBBM_STATUS           0x0e40
#define R9800_RBBM_FIFOCNT_MASK     0x7f
#define R9800_RBBM_FIFO_FREE        0x40
#define R9800_RBBM_HIRQ_ON_RBB      (1 << 8)
#define R9800_RBBM_CPRQ_ON_RBB      (1 << 9)
#define R9800_RBBM_CP_CMDSTRM_BUSY  (1 << 16)
#define R9800_RBBM_E2_BUSY          (1 << 17)
#define R9800_RBBM_RB2D_BUSY        (1 << 18)
#define R9800_RBBM_RB3D_BUSY        (1 << 19)
#define R9800_RBBM_VAP_BUSY         (1 << 20)
#define R9800_RBBM_RE_BUSY          (1 << 21)
#define R9800_RBBM_TAM_BUSY         (1 << 22)
#define R9800_RBBM_TDM_BUSY         (1 << 23)
#define R9800_RBBM_PB_BUSY          (1 << 24)
#define R9800_RBBM_TIM_BUSY         (1 << 25)
#define R9800_RBBM_GA_BUSY          (1 << 26)
#define R9800_RBBM_CBA2D_BUSY       (1 << 27)
#define R9800_RBBM_GUI_ACTIVE       (1u << 31)
#define R9800_RBBM_CNTL             0x0e44
#define R9800_RBBM_STATUS_2         0x0e48  // alias of RBBM_STATUS on some parts

// PCI configuration space mirror
#define R9800_CONFIG_MIRROR_BASE    0x0f00
#define R9800_CONFIG_MIRROR_END     0x0fff

// ------------------------------------------------------------------
// PLL register file (via CLOCK_CNTL_INDEX/DATA)
// ------------------------------------------------------------------
#define R9800_PLL_REGS              0x40
#define R9800_PLL_CLK_PIN_CNTL      0x01
#define R9800_PLL_PPLL_CNTL         0x02
#define R9800_PLL_PPLL_REF_DIV      0x03
#define R9800_PLL_PPLL_DIV_0        0x04
#define R9800_PLL_PPLL_DIV_3        0x07
#define R9800_PLL_VCLK_ECP_CNTL     0x08
#define R9800_PLL_HTOTAL_CNTL       0x09
#define R9800_PLL_M_SPLL_REF_FB_DIV 0x0a
#define R9800_PLL_AGP_PLL_CNTL      0x0b
#define R9800_PLL_SPLL_CNTL         0x0c
#define R9800_PLL_SCLK_CNTL         0x0d
#define R9800_PLL_MPLL_CNTL         0x0e
#define R9800_PLL_MDLL_CKO          0x0f
#define R9800_PLL_MDLL_RDCKA        0x10
#define R9800_PLL_MDLL_RDCKB        0x11
#define R9800_PLL_MCLK_CNTL         0x12
#define R9800_PLL_PLL_TEST_CNTL     0x13
#define R9800_PLL_CLK_PWRMGT_CNTL   0x14
#define R9800_PLL_PLL_PWRMGT_CNTL   0x15
#define R9800_PLL_CG_TEST_MACRO_RW_READ 0x16
#define R9800_PLL_SCLK_CNTL2        0x1e
#define R9800_PLL_MCLK_MISC         0x1f
#define R9800_PLL_TV_PLL_FINE_CNTL  0x20
#define R9800_PLL_TV_PLL_CNTL       0x21
#define R9800_PLL_TV_PLL_CNTL1      0x22
#define R9800_PLL_TV_DAC_CNTL       0x23
#define R9800_PLL_P2PLL_CNTL        0x2a
#define R9800_PLL_P2PLL_REF_DIV     0x2b
#define R9800_PLL_P2PLL_DIV_0       0x2c
#define R9800_PLL_PIXCLKS_CNTL      0x2d
#define R9800_PLL_HTOTAL2_CNTL      0x2e
#define R9800_PLL_SCLK_MORE_CNTL    0x35
#define R9800_PPLL_RESET            (1 << 0)
#define R9800_PPLL_SLEEP            (1 << 1)
#define R9800_PPLL_ATOMIC_UPDATE_EN (1 << 16)
#define R9800_PPLL_VGA_ATOMIC_UPDATE_EN (1 << 17)
#define R9800_PPLL_ATOMIC_UPDATE_VSYNC (1 << 18)
#define R9800_PPLL_ATOMIC_UPDATE    (1 << 15)
#define R9800_PPLL_REF_DIV_MASK     0x3ff
#define R9800_PPLL_REF_DIV_ACC_SHIFT 18
#define R9800_PPLL_REF_DIV_ACC_MASK (0x3ff << 18)
#define R9800_PPLL_FB_DIV_MASK      0x7ff
#define R9800_PPLL_POST_DIV_SHIFT   16
#define R9800_VCLK_SRC_SEL_MASK     0x3
#define R9800_VCLK_SRC_PPLL         3
#define R9800_PIX2CLK_SRC_SEL_MASK  0x3
#define R9800_PIX2CLK_SRC_P2PLL     3

// ------------------------------------------------------------------
// 2D GUI engine
// ------------------------------------------------------------------
#define R9800_DST_OFFSET            0x1404
#define R9800_DST_PITCH             0x1408
#define R9800_DST_WIDTH             0x140c
#define R9800_DST_HEIGHT            0x1410
#define R9800_SRC_X                 0x1414
#define R9800_SRC_Y                 0x1418
#define R9800_DST_X                 0x141c
#define R9800_DST_Y                 0x1420
#define R9800_SRC_PITCH_OFFSET      0x1428
#define R9800_DST_PITCH_OFFSET      0x142c
#define R9800_SRC_Y_X               0x1434
#define R9800_DST_Y_X               0x1438
#define R9800_DST_HEIGHT_WIDTH      0x143c
#define R9800_DP_GUI_MASTER_CNTL    0x146c
#define R9800_BRUSH_SCALE           0x1470
#define R9800_BRUSH_Y_X             0x1474
#define R9800_DP_BRUSH_BKGD_CLR     0x1478
#define R9800_DP_BRUSH_FRGD_CLR     0x147c
#define R9800_BRUSH_DATA0           0x1480
#define R9800_BRUSH_DATA63          0x157c
#define R9800_DST_WIDTH_X           0x1588
#define R9800_DST_HEIGHT_WIDTH_8    0x158c
#define R9800_SRC_X_Y               0x1590
#define R9800_DST_X_Y               0x1594
#define R9800_DST_WIDTH_HEIGHT      0x1598
#define R9800_DST_WIDTH_X_INCY      0x159c
#define R9800_DST_HEIGHT_Y          0x15a0
#define R9800_DST_X_SUB             0x15a4
#define R9800_DST_Y_SUB             0x15a8
#define R9800_SRC_OFFSET            0x15ac
#define R9800_SRC_PITCH             0x15b0
#define R9800_DST_HEIGHT_WIDTH_BW   0x15b4
#define R9800_CLR_CMP_CNTL          0x15c0
#define R9800_CLR_CMP_CLR_SRC       0x15c4
#define R9800_CLR_CMP_CLR_DST       0x15c8
#define R9800_CLR_CMP_MASK          0x15cc
#define R9800_DP_SRC_FRGD_CLR       0x15d8
#define R9800_DP_SRC_BKGD_CLR       0x15dc
#define R9800_GUI_SCRATCH_REG0      0x15e0
#define R9800_GUI_SCRATCH_REG5      0x15f4
#define R9800_DST_LINE_START        0x1600
#define R9800_DST_LINE_END          0x1604
#define R9800_DST_LINE_PATCOUNT     0x1608
#define R9800_DST_BRES_ERR          0x1628
#define R9800_DST_BRES_INC          0x162c
#define R9800_DST_BRES_DEC          0x1630
#define R9800_DST_BRES_LNTH         0x1634
#define R9800_DST_BRES_LNTH_SUB     0x1638
#define R9800_SC_LEFT               0x1640
#define R9800_SC_RIGHT              0x1644
#define R9800_SC_TOP                0x1648
#define R9800_SC_BOTTOM             0x164c
#define R9800_SRC_SC_RIGHT          0x1654
#define R9800_SRC_SC_BOTTOM         0x165c
#define R9800_AUX_SC_CNTL           0x1660
#define R9800_AUX_SC_ENB_MASK       0x15u
#define R9800_AUX1_SC_LEFT          0x1664
#define R9800_AUX3_SC_BOTTOM        0x1690
#define R9800_GUI_DEBUG0            0x16a0
#define R9800_GUI_DEBUG5            0x16b4
#define R9800_DP_CNTL               0x16c0
#define R9800_DP_CNTL_DST_X_DIR     (1 << 0)
#define R9800_DP_CNTL_DST_Y_DIR     (1 << 1)
#define R9800_DP_CNTL_DST_TILE_SHIFT 3
#define R9800_DP_CNTL_DST_LAST_PEL  (1 << 5)
#define R9800_DP_CNTL_POLY_LINE     (1 << 15)
#define R9800_DP_DATATYPE           0x16c4
#define R9800_DP_MIX                0x16c8
#define R9800_DP_WRITE_MASK         0x16cc
#define R9800_DP_CNTL_XDIR_YDIR_YMAJOR 0x16d0
#define R9800_DP_LINE_Y_MAJOR       (1u << 2)
#define R9800_DP_LINE_Y_DIR         (1u << 15)
#define R9800_DP_LINE_X_DIR         (1u << 31)
#define R9800_DEFAULT_PITCH_OFFSET  0x16e0
#define R9800_DEFAULT_PITCH         0x16e4
#define R9800_DEFAULT_SC_BOTTOM_RIGHT 0x16e8
#define R9800_SC_TOP_LEFT           0x16ec
#define R9800_SC_BOTTOM_RIGHT       0x16f0
#define R9800_SRC_SC_BOTTOM_RIGHT   0x16f4
#define R9800_RB2D_DSTCACHE_MODE    0x3428
#define R9800_RB2D_DSTCACHE_CTLSTAT 0x342c
#define R9800_DSTCACHE_CTLSTAT      0x1714
#define R9800_WAIT_UNTIL            0x1720
#define R9800_WAIT_CRTC_PFLIP       (1 << 0)
#define R9800_WAIT_2D_IDLECLEAN     (1 << 16)
#define R9800_WAIT_3D_IDLECLEAN     (1 << 17)
#define R9800_WAIT_HOST_IDLECLEAN   (1 << 18)
#define R9800_ISYNC_CNTL            0x1724
#define R9800_RBBM_GUICNTL          0x172c
#define R9800_HOST_DATA0            0x17c0
#define R9800_HOST_DATA7            0x17dc
#define R9800_HOST_DATA_LAST        0x17e0
#define R9800_DST_PITCH_OFFSET_C    0x1c80
#define R9800_DP_GUI_MASTER_CNTL_C  0x1c84
#define R9800_SC_TOP_LEFT_C         0x1c88
#define R9800_SC_BOTTOM_RIGHT_C     0x1c8c
#define R9800_RB3D_ZPASS_DATA_LEGACY 0x3290
#define R9800_RB3D_ZPASS_ADDR_LEGACY 0x3294
#define R9800_RB3D_ZCACHE_MODE_LEGACY 0x3250
#define R9800_RB3D_ZCACHE_CTLSTAT_LEGACY 0x3254
#define R9800_RB3D_DSTCACHE_MODE_LEGACY 0x3258
#define R9800_RB3D_DSTCACHE_CTLSTAT_LEGACY 0x325c
#define R9800_RE_TOP_LEFT_LEGACY    0x26c0
#define R9800_RE_WIDTH_HEIGHT_LEGACY 0x1c44

// DP_GUI_MASTER_CNTL fields
#define R9800_GMC_SRC_PITCH_OFFSET_CNTL (1 << 0)
#define R9800_GMC_DST_PITCH_OFFSET_CNTL (1 << 1)
#define R9800_GMC_SRC_CLIPPING      (1 << 2)
#define R9800_GMC_DST_CLIPPING      (1 << 3)
#define R9800_GMC_BRUSH_TYPE(g)     (((g) >> 4) & 0xf)
#define R9800_GMC_DST_DATATYPE(g)   (((g) >> 8) & 0xf)
#define R9800_GMC_SRC_DATATYPE(g)   (((g) >> 12) & 0x3)
#define R9800_GMC_ROP3(g)           (((g) >> 16) & 0xff)
#define R9800_GMC_SRC_SOURCE(g)     (((g) >> 24) & 0x7)
#define R9800_GMC_3D_FCN_EN         (1u << 27)
#define R9800_GMC_CLR_CMP_CNTL_DIS  (1 << 28)
#define R9800_GMC_AUX_CLIP_DIS      (1 << 29)
#define R9800_GMC_WR_MSK_DIS        (1 << 30)
#define R9800_GMC_LD_BRUSH_Y_X      (1u << 31)

// ------------------------------------------------------------------
// R300 3D engine register blocks
// ------------------------------------------------------------------
// Vertex assembler / processor (0x2000-0x24ff)
#define R9800_VAP_BASE              0x2000
#define R9800_VAP_END               0x24ff
#define R9800_VAP_PORT_DATA0        0x2000
#define R9800_VAP_PORT_DATA15       0x203c
#define R9800_VAP_PORT_IDX0         0x2040
#define R9800_VAP_PORT_IDX15        0x207c
#define R9800_VAP_CNTL              0x2080
#define R9800_VAP_VF_CNTL           0x2084
#define R9800_VF_PRIM_TYPE(v)       ((v) & 0xf)
#define R9800_VF_PRIM_WALK(v)       (((v) >> 4) & 3)
#define R9800_VF_WALK_STATE         0
#define R9800_VF_WALK_INDICES       1
#define R9800_VF_WALK_LIST          2
#define R9800_VF_WALK_EMBEDDED      3
#define R9800_VF_INDEX_SIZE_32      (1 << 11)
#define R9800_VF_NUM_VERTICES(v)    ((v) >> 16)
#define R9800_VAP_INDEX_OFFSET      0x208c
#define R9800_VAP_OUT_VTX_FMT_0     0x2090
#define R9800_VAP_OUT_VTX_FMT_1     0x2094
#define R9800_VAP_VPORT_XSCALE      0x2098
#define R9800_VAP_VPORT_XOFFSET     0x209c
#define R9800_VAP_VPORT_YSCALE      0x20a0
#define R9800_VAP_VPORT_YOFFSET     0x20a4
#define R9800_VAP_VPORT_ZSCALE      0x20a8
#define R9800_VAP_VPORT_ZOFFSET     0x20ac
#define R9800_VAP_VPORT_XSCALE_ALT  0x1d98
#define R9800_VAP_VPORT_ZOFFSET_ALT 0x1dac
#define R9800_VAP_VTE_CNTL          0x20b0
#define R9800_VTE_X_SCALE_ENA       (1 << 0)
#define R9800_VTE_X_OFFSET_ENA      (1 << 1)
#define R9800_VTE_Y_SCALE_ENA       (1 << 2)
#define R9800_VTE_Y_OFFSET_ENA      (1 << 3)
#define R9800_VTE_Z_SCALE_ENA       (1 << 4)
#define R9800_VTE_Z_OFFSET_ENA      (1 << 5)
#define R9800_VTE_XY_FMT            (1 << 8)
#define R9800_VTE_Z_FMT             (1 << 9)
#define R9800_VTE_W0_FMT            (1 << 10)
#define R9800_VAP_VTX_SIZE          0x20b4
#define R9800_VAP_PORT_DATA_IDX_128 0x20b8
#define R9800_VAP_VTX_NUM_ARRAYS    0x20c0
#define R9800_VAP_VTX_AOS_ATTR01    0x20c4
#define R9800_VAP_VTX_AOS_ADDR0     0x20c8
#define R9800_VAP_VTX_AOS_ADDR15    0x2120
#define R9800_VAP_VF_MAX_VTX_INDX   0x2134
#define R9800_VAP_VF_MIN_VTX_INDX   0x2138
#define R9800_VAP_CNTL_STATUS       0x2140
#define R9800_VAP_PVS_BYPASS        (1 << 8)
#define R9800_VAP_PROG_STREAM_CNTL_0 0x2150
#define R9800_VAP_PROG_STREAM_CNTL_7 0x216c
#define R9800_VAP_VTX_STATE_CNTL    0x2180
#define R9800_VAP_VSM_VTX_ASSM      0x2184
#define R9800_VAP_PSC_SGN_NORM_CNTL 0x21dc
#define R9800_VAP_PROG_STREAM_CNTL_EXT_0 0x21e0
#define R9800_VAP_PROG_STREAM_CNTL_EXT_7 0x21fc
#define R9800_VAP_PVS_VECTOR_INDX_REG 0x2200
#define R9800_VAP_PVS_VECTOR_DATA_REG 0x2204
#define R9800_VAP_PVS_VECTOR_DATA_REG_128 0x2208
#define R9800_VAP_CLIP_CNTL         0x221c
#define R9800_CLIP_DISABLE          (1 << 16)
#define R9800_UCP_CULL_ONLY         (1 << 17)
#define R9800_VAP_GB_VERT_CLIP_ADJ  0x2220
#define R9800_VAP_GB_VERT_DISC_ADJ  0x2224
#define R9800_VAP_GB_HORZ_CLIP_ADJ  0x2228
#define R9800_VAP_GB_HORZ_DISC_ADJ  0x222c
#define R9800_VAP_PVS_FLOW_CNTL_ADDRS_0 0x2230
#define R9800_VAP_PVS_FLOW_CNTL_ADDRS_15 0x226c
#define R9800_VAP_PVS_STATE_FLUSH_REG 0x2284
#define R9800_VAP_PVS_VTX_TIMEOUT_REG 0x2288
#define R9800_VAP_PVS_FLOW_CNTL_LOOP_INDEX_0 0x2290
#define R9800_VAP_PVS_FLOW_CNTL_LOOP_INDEX_15 0x22cc
#define R9800_VAP_PVS_CODE_CNTL_0   0x22d0
#define R9800_VAP_PVS_CONST_CNTL    0x22d4
#define R9800_VAP_PVS_CODE_CNTL_1   0x22d8
#define R9800_VAP_PVS_FLOW_CNTL_OPC 0x22dc
#define R9800_VAP_VTX_ST_BASE       0x2300
#define R9800_VAP_VTX_ST_POS_0_W_4  0x230c
#define R9800_VAP_VTX_ST_END        0x246c
#define R9800_VAP_VTX_ST_CLR_0_PKD  0x2470
#define R9800_VAP_VTX_ST_CLR_7_PKD  0x248c
#define R9800_VAP_VTX_ST_POS_0_X_2  0x2490
#define R9800_VAP_VTX_ST_POS_0_Y_2  0x2494
#define R9800_VAP_VTX_ST_NORM_0_PKD 0x2498
#define R9800_VAP_VTX_ST_USR_CLR_PKD 0x249c
#define R9800_VAP_VTX_ST_POS_0_X_3  0x24a0
#define R9800_VAP_VTX_ST_POS_0_Y_3  0x24a4
#define R9800_VAP_VTX_ST_POS_0_Z_3  0x24a8
#define R9800_VAP_VTX_ST_END_OF_PKT 0x24ac
// PVS vector memory layout (octwords)
#define R9800_PVS_CODE_START        0
#define R9800_PVS_CODE_LINES        256
#define R9800_PVS_CONST_START       512
#define R9800_PVS_CONST_VECS        256
#define R9800_PVS_UCP_START         1024
#define R9800_PVS_POINT_VPORT_SCALE 1030
#define R9800_PVS_POINT_GEN_TEX     1031
#define R9800_PVS_VECTOR_MEM        2048
// PVS instruction encoding
#define R9800_PVS_DST_OPCODE(d)     ((d) & 0x3f)
#define R9800_PVS_DST_MATH_INST     (1 << 6)
#define R9800_PVS_DST_MACRO_INST    (1 << 7)
#define R9800_PVS_DST_REG_TYPE(d)   (((d) >> 8) & 0x7)
#define R9800_PVS_DST_REG_TEMP      0
#define R9800_PVS_DST_REG_A0        1
#define R9800_PVS_DST_REG_OUT       2
#define R9800_PVS_DST_REG_OUT_REPL_X 3
#define R9800_PVS_DST_REG_ALT_TEMP  4
#define R9800_PVS_DST_REG_INPUT     5
#define R9800_PVS_DST_ADDR_MODE_1   (1 << 12)
#define R9800_PVS_DST_OFFSET(d)     (((d) >> 13) & 0x7f)
#define R9800_PVS_DST_WE(d)         (((d) >> 20) & 0xf)
#define R9800_PVS_DST_VE_SAT        (1 << 24)
#define R9800_PVS_DST_ME_SAT        (1 << 25)
#define R9800_PVS_DST_ADDR_MODE_0   (1u << 31)
#define R9800_PVS_SRC_REG_TYPE(s)   ((s) & 0x3)
#define R9800_PVS_SRC_REG_TEMP      0
#define R9800_PVS_SRC_REG_INPUT     1
#define R9800_PVS_SRC_REG_CONST     2
#define R9800_PVS_SRC_REG_ALT_TEMP  3
#define R9800_PVS_SRC_ADDR_MODE_0   (1 << 2)
#define R9800_PVS_SRC_REL_ADDR      (1 << 4)
#define R9800_PVS_SRC_OFFSET(s)     (((s) >> 5) & 0xff)
#define R9800_PVS_SRC_SWZ(s, c)     (((s) >> (13 + 3 * (c))) & 0x7)
#define R9800_PVS_SRC_NEG(s, c)     (((s) >> (25 + (c))) & 0x1)
#define R9800_PVS_SRC_ADDR_MODE_1   (1 << 29)
// vector engine opcodes
#define R9800_VE_NOP                0
#define R9800_VE_DOT_PRODUCT        1
#define R9800_VE_MULTIPLY           2
#define R9800_VE_ADD                3
#define R9800_VE_MULTIPLY_ADD       4
#define R9800_VE_DISTANCE_VECTOR    5
#define R9800_VE_FRACTION           6
#define R9800_VE_MAXIMUM            7
#define R9800_VE_MINIMUM            8
#define R9800_VE_SET_GREATER_THAN_EQUAL 9
#define R9800_VE_SET_LESS_THAN      10
#define R9800_VE_MULTIPLYX2_ADD     11
#define R9800_VE_MULTIPLY_CLAMP     12
#define R9800_VE_FLT2FIX_DX         13
#define R9800_VE_FLT2FIX_DX_RND     14
// math engine opcodes (MATH_INST set: 64 + n)
#define R9800_ME_EXP_BASE2_DX       1
#define R9800_ME_LOG_BASE2_DX       2
#define R9800_ME_EXP_BASEE_FF       3
#define R9800_ME_LIGHT_COEFF_DX     4
#define R9800_ME_POWER_FUNC_FF      5
#define R9800_ME_RECIP_DX           6
#define R9800_ME_RECIP_FF           7
#define R9800_ME_RECIP_SQRT_DX      8
#define R9800_ME_RECIP_SQRT_FF      9
#define R9800_ME_MULTIPLY           10
#define R9800_ME_EXP_BASE2_FULL_DX  11
#define R9800_ME_LOG_BASE2_FULL_DX  12
#define R9800_ME_POWER_FUNC_FF_CLAMP_B  13
#define R9800_ME_POWER_FUNC_FF_CLAMP_B1 14
#define R9800_ME_POWER_FUNC_FF_CLAMP_01 15
#define R9800_ME_SIN                16
#define R9800_ME_COS                17

// Graphics backend (0x4000-0x40ff)
#define R9800_GB_VAP_RASTER_VTX_FMT_0 0x4000
#define R9800_GB_VAP_RASTER_VTX_FMT_1 0x4004
#define R9800_GB_ENABLE             0x4008
#define R9800_GB_MSPOS0             0x4010
#define R9800_GB_MSPOS1             0x4014
#define R9800_GB_TILE_CONFIG        0x4018
#define R9800_GB_SUBPIXEL_1_16      (1 << 16)
#define R9800_GB_SELECT             0x401c
#define R9800_GB_AA_CONFIG          0x4020
#define R9800_GB_FIFO_SIZE          0x4024
#define R9800_GB_Z_PEQ_CONFIG       0x4028
// Texture unit
#define R9800_TX_INVALTAGS          0x4100
#define R9800_TX_ENABLE             0x4104
// Geometry assembly (0x4200-0x42ff)
#define R9800_GA_POINT_S0           0x4200
#define R9800_GA_POINT_T0           0x4204
#define R9800_GA_POINT_S1           0x4208
#define R9800_GA_POINT_T1           0x420c
#define R9800_GA_TRIANGLE_STIPPLE   0x4214
#define R9800_GA_POINT_SIZE         0x421c
#define R9800_GA_POINT_MINMAX       0x4230
#define R9800_GA_LINE_CNTL          0x4234
#define R9800_GA_LINE_STIPPLE_CONFIG 0x4238
#define R9800_GA_LINE_STIPPLE_VALUE 0x4260
#define R9800_GA_LINE_S0            0x4264
#define R9800_GA_LINE_S1            0x4268
#define R9800_GA_ENHANCE            0x4274
#define R9800_GA_COLOR_CONTROL      0x4278
#define R9800_GA_SOLID_RG           0x427c
#define R9800_GA_SOLID_BA           0x4280
#define R9800_GA_POLY_MODE          0x4288
#define R9800_GA_ROUND_MODE         0x428c
#define R9800_GA_OFFSET             0x4290
#define R9800_GA_FOG_SCALE          0x4294
#define R9800_GA_FOG_OFFSET         0x4298
#define R9800_GA_SOFT_RESET         0x429c
#define R9800_SU_TEX_WRAP           0x42a0
#define R9800_SU_POLY_OFFSET_FRONT_SCALE 0x42a4
#define R9800_SU_POLY_OFFSET_FRONT_OFFSET 0x42a8
#define R9800_SU_POLY_OFFSET_BACK_SCALE 0x42ac
#define R9800_SU_POLY_OFFSET_BACK_OFFSET 0x42b0
#define R9800_SU_POLY_OFFSET_ENABLE 0x42b4
#define R9800_SU_CULL_MODE          0x42b8
#define R9800_SU_DEPTH_SCALE        0x42c0
#define R9800_SU_DEPTH_OFFSET       0x42c4
#define R9800_SU_REG_DEST           0x42c8
// Rasterizer
#define R9800_RS_COUNT              0x4300
#define R9800_RS_INST_COUNT         0x4304
#define R9800_RS_IP_0               0x4310
#define R9800_RS_IP_7               0x432c
#define R9800_RS_INST_0             0x4330
#define R9800_RS_INST_15            0x436c
// Scan converter
#define R9800_SC_HYPERZ             0x43a4
#define R9800_SC_EDGERULE           0x43a8
#define R9800_SC_CLIP_0_A           0x43b0
#define R9800_SC_CLIP_3_B           0x43cc
#define R9800_SC_CLIP_RULE          0x43d0
#define R9800_SC_SCISSOR0           0x43e0
#define R9800_SC_SCISSOR1           0x43e4
#define R9800_SC_SCREENDOOR         0x43e8
#define R9800_SC_COORD_OFFSET       1440
// Texture state (16 maps)
#define R9800_TX_FILTER0_0          0x4400
#define R9800_TX_FILTER1_0          0x4440
#define R9800_TX_FORMAT0_0          0x4480
#define R9800_TX_FORMAT1_0          0x44c0
#define R9800_TX_FORMAT2_0          0x4500
#define R9800_TX_OFFSET_0           0x4540
#define R9800_TX_CHROMA_KEY_0       0x4580
#define R9800_TX_BORDER_COLOR_0     0x45c0
#define R9800_TX_MAPS               16
// Fragment shader
#define R9800_US_CONFIG             0x4600
#define R9800_US_PIXSIZE            0x4604
#define R9800_US_CODE_OFFSET        0x4608
#define R9800_US_RESET              0x460c
#define R9800_US_CODE_ADDR_0        0x4610
#define R9800_US_CODE_ADDR_3        0x461c
#define R9800_US_TEX_INST_0         0x4620
#define R9800_US_TEX_INST_31        0x469c
#define R9800_US_OUT_FMT_0          0x46a4
#define R9800_US_OUT_FMT_3          0x46b0
#define R9800_US_W_FMT              0x46b4
#define R9800_US_ALU_RGB_ADDR_0     0x46c0
#define R9800_US_ALU_ALPHA_ADDR_0   0x47c0
#define R9800_US_ALU_RGB_INST_0     0x48c0
#define R9800_US_ALU_ALPHA_INST_0   0x49c0
#define R9800_US_ALU_CONST_R_0      0x4c00
#define R9800_US_ALU_CONST_A_31     0x4dfc
#define R9800_US_ALU_INSTS          64
#define R9800_US_TEX_INSTS          32
#define R9800_US_CONSTS             32
// Fog
#define R9800_FG_FOG_BLEND          0x4bc0
#define R9800_FG_FOG_FACTOR         0x4bc4
#define R9800_FG_FOG_COLOR_R        0x4bc8
#define R9800_FG_FOG_COLOR_G        0x4bcc
#define R9800_FG_FOG_COLOR_B        0x4bd0
#define R9800_FG_ALPHA_FUNC         0x4bd4
#define R9800_FG_DEPTH_SRC          0x4bd8
// Color buffer
#define R9800_RB3D_CCTL             0x4e00
#define R9800_RB3D_BLENDCNTL        0x4e04
#define R9800_RB3D_ABLENDCNTL       0x4e08
#define R9800_RB3D_COLOR_CHANNEL_MASK 0x4e0c
#define R9800_RB3D_CONSTANT_COLOR   0x4e10
#define R9800_RB3D_COLOR_CLEAR_VALUE 0x4e14
#define R9800_RB3D_ROPCNTL          0x4e18
#define R9800_RB3D_CLRCMP_FLIPE     0x4e1c
#define R9800_RB3D_CLRCMP_CLR       0x4e20
#define R9800_RB3D_CLRCMP_MSK       0x4e24
#define R9800_RB3D_COLOROFFSET0     0x4e28
#define R9800_RB3D_COLOROFFSET3     0x4e34
#define R9800_RB3D_COLORPITCH0      0x4e38
#define R9800_RB3D_COLORPITCH3      0x4e44
#define R9800_RB3D_DSTCACHE_CTLSTAT 0x4e4c
#define R9800_RB3D_DITHER_CTL       0x4e50
#define R9800_RB3D_CMASK_OFFSET0    0x4e54
#define R9800_RB3D_CMASK_OFFSET3    0x4e60
#define R9800_RB3D_CMASK_PITCH0     0x4e64
#define R9800_RB3D_CMASK_WRINDEX    0x4e74
#define R9800_RB3D_CMASK_DWORD      0x4e78
#define R9800_RB3D_CMASK_RDINDEX    0x4e7c
#define R9800_RB3D_AARESOLVE_OFFSET 0x4e80
#define R9800_RB3D_AARESOLVE_PITCH  0x4e84
#define R9800_RB3D_AARESOLVE_CTL    0x4e88
#define R9800_RB3D_DISCARD_SRC_PIXEL_LTE_THRESHOLD 0x4ea0
#define R9800_RB3D_DISCARD_SRC_PIXEL_GTE_THRESHOLD 0x4ea4
// Z buffer
#define R9800_ZB_CNTL               0x4f00
#define R9800_ZB_ZSTENCILCNTL       0x4f04
#define R9800_ZB_STENCILREFMASK     0x4f08
#define R9800_ZB_FORMAT             0x4f10
#define R9800_ZB_ZTOP               0x4f14
#define R9800_ZB_ZCACHE_CTLSTAT     0x4f18
#define R9800_ZB_BW_CNTL            0x4f1c
#define R9800_ZB_DEPTHOFFSET        0x4f20
#define R9800_ZB_DEPTHPITCH         0x4f24
#define R9800_ZB_DEPTHCLEARVALUE    0x4f28
#define R9800_ZB_ZMASK_OFFSET       0x4f30
#define R9800_ZB_ZMASK_PITCH        0x4f34
#define R9800_ZB_ZMASK_WRINDEX      0x4f38
#define R9800_ZB_ZMASK_DWORD        0x4f3c
#define R9800_ZB_ZMASK_RDINDEX      0x4f40
#define R9800_ZB_HIZ_OFFSET         0x4f44
#define R9800_ZB_HIZ_WRINDEX        0x4f48
#define R9800_ZB_HIZ_DWORD          0x4f4c
#define R9800_ZB_HIZ_RDINDEX        0x4f50
#define R9800_ZB_HIZ_PITCH          0x4f54
#define R9800_ZB_ZPASS_DATA         0x4f58
#define R9800_ZB_ZPASS_ADDR         0x4f5c
#define R9800_ZB_DEPTHXY_OFFSET     0x4f60
// on-chip mask RAMs (dwords)
#define R9800_ZMASK_RAM_DWORDS      0x4000
#define R9800_HIZ_RAM_DWORDS        0x4000
#define R9800_CMASK_RAM_DWORDS      0x4000

// 3D register image window kept by the raster state
#define R9800_R3D_IMAGE_BASE        0x4000
#define R9800_R3D_IMAGE_DWORDS      0x400
#define R9800_R3D_IMAGE_END         0x4fff
#define R3D(off)                    (((off) - R9800_R3D_IMAGE_BASE) >> 2)

#endif // BX_IODEV_RADEON9800_REGS_H
