/////////////////////////////////////////////////////////////////////////
// $Id$
/////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025-2026  The Bochs Project
//
//  ATI Radeon 9800 (R350) emulation: device shell, PCI configuration
//  (primary VGA function and the secondary display function), register
//  file dispatch, memory controller address space, VGA passthrough and
//  the display block (PLL / CRTC1 / CRTC2 / DAC / palettes / hardware
//  cursor / scanout).
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
#include "bx_debug/debug.h"

#define LOG_THIS BX_R9800_THIS

static bx_radeon9800_c *theSvga = NULL;

// ---------------------------------------------------------------------
// Configuration
// ---------------------------------------------------------------------

static const char *radeon9800_memsize_list[] = {
  "64",
  "128",
  "256",
  NULL
};

static int radeon9800_cfg_threads = 1;
static int radeon9800_cfg_asicrev = 0;   // CFG_ATI_REV_ID: 0 = A11, 1 = A12, 2 = A13
// Register access trace (bochsrc 'trace='): bit 0 logs writes, bit 1 logs
// reads with runs of identical results collapsed, bit 2 reports a guest
// spinning on one register. 0 disables it and costs nothing.
static int radeon9800_cfg_trace = 0;

static void radeon9800_init_options(void)
{
  bx_param_enum_c *model = SIM->get_param_enum(BXPN_VGA_EXT_MODEL);
  model->set_enabled(1);
  model->set_label("Radeon 9800 memory size (MB)");
  model->set_description("Size of the Radeon 9800 video memory in MB");
  model->set_choices(radeon9800_memsize_list, 1, 0);
}

static void radeon9800_cleanup_options(void)
{
  bx_param_enum_c *model = SIM->get_param_enum(BXPN_VGA_EXT_MODEL);
  model->set_choices(NULL, 0, 0);
  model->set_label("Model");
  model->set_enabled(0);
}

Bit32s bx_radeon9800_c::options_parser(const char *context, int num_params, char *params[])
{
  if (!strcmp(params[0], "radeon9800")) {
    for (int i = 1; i < num_params; i++) {
      if (!strncmp(params[i], "threads=", 8)) {
        int n = atoi(&params[i][8]);
        if (n < 1) n = 1;
        if (n > R9800_RASTER_MAX_WORKERS) n = R9800_RASTER_MAX_WORKERS;
        radeon9800_cfg_threads = n;
      } else if (!strncmp(params[i], "memory=", 7)) {
        SIM->get_param_enum(BXPN_VGA_EXT_MODEL)->set_by_name(&params[i][7]);
      } else if (!strncmp(params[i], "asicrev=", 8)) {
        int n = atoi(&params[i][8]);
        if (n < 0) n = 0;
        if (n > 15) n = 15;
        radeon9800_cfg_asicrev = n;
      } else if (!strncmp(params[i], "trace=", 6)) {
        int n = atoi(&params[i][6]);
        if (n < 0) n = 0;
        if (n > 31) n = 31;
        radeon9800_cfg_trace = n;
      } else {
        if (theSvga != NULL)
          theSvga->error("%s: unknown parameter '%s' for radeon9800 ignored.", context, params[i]);
      }
    }
  }
  return 0;
}

Bit32s bx_radeon9800_c::options_save(FILE *fp)
{
  return fprintf(fp, "radeon9800: threads=%d, asicrev=%d, trace=%d\n",
                 radeon9800_cfg_threads, radeon9800_cfg_asicrev, radeon9800_cfg_trace);
}

// ---------------------------------------------------------------------
// Plugin entry point
// ---------------------------------------------------------------------

PLUGIN_ENTRY_FOR_MODULE(radeon9800)
{
  if (mode == PLUGIN_INIT) {
    theSvga = new bx_radeon9800_c();
    bx_devices.pluginVgaDevice = theSvga;
    BX_REGISTER_DEVICE_DEVMODEL(plugin, type, theSvga, BX_PLUGIN_RADEON9800);
    radeon9800_init_options();
    SIM->register_addon_option("radeon9800", bx_radeon9800_c::options_parser, bx_radeon9800_c::options_save);
  } else if (mode == PLUGIN_FINI) {
    radeon9800_cleanup_options();
    SIM->unregister_addon_option("radeon9800");
    delete theSvga;
    theSvga = NULL;
  } else if (mode == PLUGIN_PROBE) {
    return (int)PLUGTYPE_VGA;
  } else if (mode == PLUGIN_FLAGS) {
    return PLUGFLAG_PCI;
  }
  return 0; // Success
}

// ---------------------------------------------------------------------
// Construction / initialisation
// ---------------------------------------------------------------------

bx_radeon9800_c::bx_radeon9800_c() : bx_vgacore_c()
{
  put("R9800");
  cp_fifo = NULL;
  cp_fifo_rptr = NULL;
  cp_fifo_tag = NULL;
  cp_pl = NULL;
  ind_pl = NULL;
  cp_thread_started = false;
  cp_thread_run = false;
  raster_run = false;
  sec = NULL;
  memset(&rb, 0, sizeof(rb));
  memset(&tex_stage, 0, sizeof(tex_stage));
  memset(&s2d_dst, 0, sizeof(s2d_dst));
  memset(&s2d_src, 0, sizeof(s2d_src));
  memset(&draw, 0, sizeof(draw));
  vline_timer_id = BX_NULL_TIMER_HANDLE;
  pump_timer_id = BX_NULL_TIMER_HANDLE;
  in_indirect = false;
}

bx_radeon9800_c::~bx_radeon9800_c()
{
  pm4_thread_close();
  raster_close();
  if (tex_stage.arena != NULL) free(tex_stage.arena);
  if (s2d_dst.arena != NULL) free(s2d_dst.arena);
  if (s2d_src.arena != NULL) free(s2d_src.arena);
  if (draw.rs != NULL) delete draw.rs;
  if (sec != NULL) delete sec;
  if (BX_R9800_THIS s.memory != NULL) {
    delete [] BX_R9800_THIS s.memory;
    BX_R9800_THIS s.memory = NULL;
  }
  if (SIM->get_bochs_root())
    SIM->get_bochs_root()->remove("radeon9800");
  BX_DEBUG(("Exit"));
}

bool bx_radeon9800_c::init_vga_extension(void)
{
  is_agp = SIM->is_agp_device(BX_PLUGIN_RADEON9800);
  if (!SIM->is_pci_device(BX_PLUGIN_RADEON9800) && !is_agp) {
    BX_PANIC(("ATI Radeon 9800 must be assigned to a PCI or AGP slot"));
  }
  BX_R9800_THIS pci_enabled = true;

  BX_R9800_THIS init_iohandlers(svga_read_handler, svga_write_handler, "radeon9800");

  // Video memory: 64, 128 or 256 MB (the model parameter of the vga extension)
  vram_size = atoi(SIM->get_param_enum(BXPN_VGA_EXT_MODEL)->get_selected()) << 20;
  if ((vram_size != (64u << 20)) && (vram_size != (128u << 20)) && (vram_size != (256u << 20)))
    vram_size = 128u << 20;
  vram_mask = vram_size - 1;
  BX_R9800_THIS s.memsize = vram_size;
  if (BX_R9800_THIS s.memory == NULL)
    BX_R9800_THIS s.memory = new Bit8u[vram_size + 64];
  memset(BX_R9800_THIS s.memory, 0, vram_size + 64);

  BX_R9800_THIS s.max_xres = 2560;
  BX_R9800_THIS s.max_yres = 2048;
  BX_R9800_THIS s.CRTC.max_reg = 0x18;

  render_threads = radeon9800_cfg_threads;
  asic_rev = radeon9800_cfg_asicrev;
  trace_mask = radeon9800_cfg_trace;
  ref_freq_hz = R9800_REF_FREQ_HZ;
  vga_disabled = false;

  if (vline_timer_id == BX_NULL_TIMER_HANDLE) {
    vline_timer_id = bx_virt_timer.register_timer(this, vline_timer_handler, 1000, 0, 0, 0, "radeon9800 vline");
  }
  if (pump_timer_id == BX_NULL_TIMER_HANDLE) {
    pump_timer_id = bx_virt_timer.register_timer(this, pump_timer_handler, 100, 0, 0, 0, "radeon9800 cp pump");
  }

  draw.rs = new r9800_raster_state_t;
  memset(draw.rs, 0, sizeof(*draw.rs));

  init_pcihandlers();
  pm4_thread_init();
  raster_init();
  init_members();
  ddc.init();
  chip_reset();

  BX_INFO(("ATI Radeon 9800 (R350, %s, %u MB, %d render thread%s) initialized",
           is_agp ? "AGP" : "PCI", vram_size >> 20, render_threads,
           (render_threads == 1) ? "" : "s"));
  if (trace_mask)
    BX_INFO(("register access trace enabled (trace=%d)", trace_mask));
#if BX_DEBUGGER
  bx_dbg_register_debug_info("radeon9800", this);
#endif
  return 1;
}

void bx_radeon9800_c::init_members(void)
{
  mm_index = 0;
  trace_rd_off = 0xffffffff;
  trace_rd_val = 0;
  trace_rd_idx = 0xffffffff;
  trace_rd_run = 0;
  trace_rd_valid = false;
  trace_run_off = 0xffffffff;
  trace_run_idx = 0xffffffff;
  trace_run_len = 0;
  trace_run_first = 0;
  trace_run_last = 0;
  trace_run_varies = false;
  trace_blk_total = 0;
  trace_blk_n = 0;
  trace_irq_count = 0;
  trace_irq_level = false;
  trace_hb_frames = 0;
  trace_fence_probes = 0;
  memset(trace_blk_off, 0, sizeof(trace_blk_off));
  memset(trace_blk_cnt, 0, sizeof(trace_blk_cnt));
  trace_wr_pos = 0;
  trace_wr_seen = 0;
  trace_polls = 0;
  memset(trace_wr_off, 0, sizeof(trace_wr_off));
  memset(trace_wr_val, 0, sizeof(trace_wr_val));
  memset(trace_wr_idx, 0, sizeof(trace_wr_idx));
  memset(bios_scratch, 0, sizeof(bios_scratch));
  memset(pll_regs, 0, sizeof(pll_regs));
  memset(ppll_work, 0, sizeof(ppll_work));
  memset(p2pll_work, 0, sizeof(p2pll_work));
  memset(ext_crtc, 0, sizeof(ext_crtc));
  memset(crtc, 0, sizeof(crtc));
  memset(palette2, 0, sizeof(palette2));
  memset(palette30, 0, sizeof(palette30));
  disp_ext = false;
  disp_crtc = 0;
  disp_output = R9800_OUT_DAC1;
  disp_xres = 640;
  disp_yres = 480;
  disp_bpp = 8;
  disp_pitch = 640;
  disp_base = 0;
  vga_banked_mode = false;
  scanout_tiled = false;
  scanout_micro = scanout_macro = scanout_x0 = scanout_y0 = 0;
  disp_dblscan = false;
  disp_hdbl = false;
  disp_blank = false;
  disp_dac_const = false;
  disp_dac_const_color = 0;
  needs_update_mode = true;
  needs_update_tile = true;
  needs_update_dispentire = true;
  ext_last = false;
  gui_idle_event = 0;
  irq_dirty = false;
  timing_dirty = false;
  palette_dirty = false;
  hostdata_active = false;
  hostdata_ndw = 0;
  memset(scl_palette, 0, sizeof(scl_palette));
  memset(gui_scratch, 0, sizeof(gui_scratch));
  memset(brush_data, 0, sizeof(brush_data));
  memset(cp_me_ram, 0, sizeof(cp_me_ram));
  memset(pvs_vec, 0, sizeof(pvs_vec));
  memset(zmask_ram, 0, sizeof(zmask_ram));
  memset(hiz_ram, 0, sizeof(hiz_ram));
  memset(cmask_ram, 0, sizeof(cmask_ram));
}

// Power-on defaults for the chip core registers
void bx_radeon9800_c::chip_reset(void)
{
  pm4_drain_wait();

  mm_index = 0;
  timing_valid = false;
  gui_xres = gui_yres = gui_bpp = 0;
  memset(bios_scratch, 0, sizeof(bios_scratch));
  // System-BIOS compatibility seed for BIOS_1_SCRATCH (hardware resets it
  // to 0). The video BIOS init entry stores {ROM segment, PCI bus/devfn
  // handed in AX} into BIOS_1_SCRATCH; the Bochs ROM BIOS rom_scan passes
  // AX = 0xF000 instead of the device address. The R350 ROM tolerates this
  // (its runtime IO-base locator scans PCI configuration space for its own
  // vendor/device ID), but the register is pre-seeded with exactly what the
  // init entry would have written so BIOS code paths that consult it first
  // see a sane value. A firmware that passes the real device address
  // overwrites this before its first use.
  bios_scratch[1] = 0xf000c000;

  gen_int_cntl = 0;
  gen_int_status = 0;
  gui_idle_event = 0;
  gen_int_update();

  bus_cntl = R9800_BUS_CNTL_DEFAULT;
  bus_cntl1 = 0;
  bus_cntl_mstr = 0;
  config_cntl = vga_disabled ? R9800_CFG_VGA_IO_DIS : R9800_CFG_VGA_RAM_EN;
  config_memsize = vram_size;
  test_debug_cntl = 0;
  test_debug_mux = 0;
  hw_debug = 0;
  host_path_cntl = 0x00000180;
  hdp_fb_location = 0;
  mem_cntl = 0x00000002;            // 256-bit memory interface (4 channels)
  mem_timing_cntl = 0;
  // Frame buffer at MC address 0 until the video BIOS programs the memory
  // controller; the AGP range starts empty.
  mc_fb_location = ((vram_size - 1) & 0xffff0000);
  mc_agp_location = 0x0000ffff;
  mem_init_lat_timer = 0x3fffffff;
  mem_sdram_mode_reg = R9800_MEM_CFG_TYPE_DDR;
  agp_base = 0;
  agp_base_2 = 0;
  agp_cntl = 0x00000020;
  memset(mem_io_cntl, 0, sizeof(mem_io_cntl));
  mc_read_cntl_ab = 0;
  mc_init_misc_lat_timer = 0;
  mc_init_gfx_lat_timer = 0;
  mc_debug = 0;
  mc_chp_io_oe_cntl_ab = 0;
  videomux_cntl = 0;
  memset(mdgpio, 0, sizeof(mdgpio));
  seprom_cntl1 = 0;
  seprom_cntl2 = 0;
  mpp_gp_config = 0;
  mpp_tb_config = 0;
  aic_cntl = 0;
  aic_pt_base = 0;
  aic_lo_addr = 0;
  aic_hi_addr = 0;
  aic_tlb_addr = 0;
  aic_tlb_data = 0;
  mc_ind_index = 0;
  memset(mc_ind_regs, 0, sizeof(mc_ind_regs));
  rbbm_soft_reset = 0;
  rbbm_cntl = 0;
  surface_cntl = 0;
  memset(surf_lower, 0, sizeof(surf_lower));
  memset(surf_upper, 0, sizeof(surf_upper));
  memset(surf_info, 0, sizeof(surf_info));
  surf_xlate_on = false;
  surface_access_flags = 0;
  memset(gui_debug, 0, sizeof(gui_debug));
  memset(gpio, 0, sizeof(gpio));
  memset(i2c_cntl, 0, sizeof(i2c_cntl));
  memset(dvi_i2c, 0, sizeof(dvi_i2c));
  cap_int_cntl = 0;
  cap_int_status = 0;
  fcp_cntl = 0;
  memset(tv_regs, 0, sizeof(tv_regs));
  memset(cap0_regs, 0, sizeof(cap0_regs));
  memset(viph_regs, 0, sizeof(viph_regs));
  viph_reg_addr = 0;
  viph_reg_data = 0;
  disp_misc_cntl = 0;
  dac_macro_cntl = 0;
  disp_pwr_man = 0;
  disp_test_debug_cntl = 0;
  disp_hw_debug = 0;
  disp_merge_cntl = 0xff000000;
  disp_output_cntl = 0;
  disp2_merge_cntl = 0;
  disp_tv_out_cntl = 0;
  memset(disp_lin_trans, 0, sizeof(disp_lin_trans));
  tv_dac_cntl = 0;
  memset(ov1_regs, 0, sizeof(ov1_regs));
  ddc.write(1, 1);

  display_reset();
  ov0_reset();
  r2d_reset();
  pm4_reset();
  r3d_reset();
  update_banking();
}

void bx_radeon9800_c::reset(unsigned type)
{
  static const struct reset_vals_t {
    unsigned      addr;
    unsigned char val;
  } reset_vals[] = {
    { 0x04, 0x00 }, { 0x05, 0x00 },
    { 0x0c, 0x00 }, { 0x0d, 0x00 },
    { 0x3c, 0xff },
    { 0x54, 0x00 }, { 0x55, 0x00 },
    { 0x60, 0x00 }, { 0x61, 0x00 }, { 0x62, 0x00 }, { 0x63, 0x00 },
  };
  for (unsigned i = 0; i < sizeof(reset_vals) / sizeof(*reset_vals); ++i) {
    pci_conf[reset_vals[i].addr] = reset_vals[i].val;
  }
  if (sec != NULL)
    sec->reset_pci();
  BX_R9800_THIS bx_vgacore_c::reset(type);
  chip_reset();
  needs_update_mode = true;
  needs_update_dispentire = true;
}

// ---------------------------------------------------------------------
// PCI configuration space
// ---------------------------------------------------------------------

static const Bit8u radeon9800_iomask[256] = {
  7,1,3,1,7,1,3,1,7,1,3,1,7,1,3,1,7,1,3,1,7,1,3,1,7,1,3,1,7,1,3,1,
  7,1,3,1,7,1,3,1,7,1,3,1,7,1,3,1,7,1,3,1,7,1,3,1,7,1,3,1,7,1,3,1,
  7,1,3,1,7,1,3,1,7,1,3,1,7,1,3,1,7,1,3,1,7,1,3,1,7,1,3,1,7,1,3,1,
  7,1,3,1,7,1,3,1,7,1,3,1,7,1,3,1,7,1,3,1,7,1,3,1,7,1,3,1,7,1,3,1,
  7,1,3,1,7,1,3,1,7,1,3,1,7,1,3,1,7,1,3,1,7,1,3,1,7,1,3,1,7,1,3,1,
  7,1,3,1,7,1,3,1,7,1,3,1,7,1,3,1,7,1,3,1,7,1,3,1,7,1,3,1,7,1,3,1,
  7,1,3,1,7,1,3,1,7,1,3,1,7,1,3,1,7,1,3,1,7,1,3,1,7,1,3,1,7,1,3,1,
  7,1,3,1,7,1,3,1,7,1,3,1,7,1,3,1,7,1,3,1,7,1,3,1,7,1,3,1,7,1,3,1
};

void bx_radeon9800_c::init_pcihandlers(void)
{
  devfunc = 0x00;
  DEV_register_pci_handlers2(BX_R9800_THIS_PTR, &devfunc, BX_PLUGIN_RADEON9800,
                             is_agp ? "ATI Radeon 9800 AGP" : "ATI Radeon 9800 PCI", is_agp);

  // multi-function header: the secondary display function lives at fn 1
  init_pci_conf(R9800_PCI_VENDOR, R9800_PCI_DEVICE, R9800_PCI_REVISION, 0x030000, 0x80, BX_PCI_INTA);

  // BAR0: 128 MB prefetchable framebuffer aperture
  pci_conf[0x10] = 0x08;
  init_bar_mem(0, R9800_LFB_SIZE, mem_read_handler, mem_write_handler);
  // BAR1: 256 byte IO block
  init_bar_io(1, R9800_IO_SIZE, io_read_handler, io_write_handler, radeon9800_iomask);
  // BAR2: 64 KB register aperture
  init_bar_mem(2, R9800_MMIO_SIZE, mem_read_handler, mem_write_handler);

  load_pci_rom(SIM->get_param_string(BXPN_VGA_ROM_PATH)->getptr(), mem_read_handler);

  // Status: capabilities list, 66 MHz, fast back-to-back, medium DEVSEL
  pci_conf[0x06] = 0xb0;
  pci_conf[0x07] = 0x02;
  pci_conf[0x0c] = 0x08;  // cache line size (32 bytes)
  // Subsystem ID (written by the ROM straps on hardware)
  pci_conf[0x2c] = R9800_SUBSYS_VENDOR & 0xff;
  pci_conf[0x2d] = R9800_SUBSYS_VENDOR >> 8;
  pci_conf[0x2e] = R9800_SUBSYS_ID & 0xff;
  pci_conf[0x2f] = R9800_SUBSYS_ID >> 8;
  pci_conf[0x34] = is_agp ? R9800_PCI_CAP_PTR : R9800_PM_CAP_OFF;
  pci_conf[0x3e] = 0x08; // min grant
  pci_conf[0x3f] = 0x00; // max latency
  // Power management capability
  pci_conf[R9800_PM_CAP_OFF + 0] = R9800_PM_CAP_ID;
  pci_conf[R9800_PM_CAP_OFF + 1] = 0x00;
  pci_conf[R9800_PM_CAP_OFF + 2] = R9800_PM_PMC & 0xff;
  pci_conf[R9800_PM_CAP_OFF + 3] = R9800_PM_PMC >> 8;
  if (is_agp) {
    pci_conf[R9800_AGP_CAP_OFF + 0] = R9800_AGP_CAP_ID;
    pci_conf[R9800_AGP_CAP_OFF + 1] = R9800_AGP_NEXT_PTR;
    pci_conf[R9800_AGP_CAP_OFF + 2] = R9800_AGP_REV;
    pci_conf[R9800_AGP_CAP_OFF + 3] = 0x00;
    pci_conf[R9800_AGP_CAP_OFF + 4] = R9800_AGP_STATUS & 0xff;
    pci_conf[R9800_AGP_CAP_OFF + 5] = (R9800_AGP_STATUS >> 8) & 0xff;
    pci_conf[R9800_AGP_CAP_OFF + 6] = (R9800_AGP_STATUS >> 16) & 0xff;
    pci_conf[R9800_AGP_CAP_OFF + 7] = (R9800_AGP_STATUS >> 24) & 0xff;
  }

  // Secondary function (1002:4e68)
  sec = new bx_radeon9800_sec_c(this);
  sec->devfunc = is_agp ? 0x01 : (Bit8u)((devfunc & 0xf8) | 0x01);
  sec->init_pci(is_agp);
}

Bit32u bx_radeon9800_c::pci_config_read(Bit8u address, unsigned io_len)
{
  return pci_read_handler(address, io_len);
}

Bit32u bx_radeon9800_c::pci_read_handler(Bit8u address, unsigned io_len)
{
  Bit32u value = 0;
  for (unsigned i = 0; i < io_len; i++) {
    Bit8u a = (Bit8u)(address + i);
    Bit8u b;
    // 0x4c-0x4f alias the subsystem ID registers (ROM strap programming path)
    if ((a >= 0x4c) && (a <= 0x4f))
      b = pci_conf[0x2c + (a - 0x4c)];
    else
      b = pci_conf[a];
    value |= (Bit32u)b << (i * 8);
  }
  return value;
}

void bx_radeon9800_c::pci_write_handler(Bit8u address, Bit32u value, unsigned io_len)
{
  Bit8u value8, oldval;

  if ((address >= 0x1c) && (address < 0x2c))
    return;

  BX_DEBUG_PCI_WRITE(address, value, io_len);
  for (unsigned i = 0; i < io_len; i++) {
    Bit8u a = (Bit8u)(address + i);
    value8 = (value >> (i * 8)) & 0xff;
    oldval = pci_conf[a];
    switch (a) {
      case 0x04:
        value8 &= 0x47;
        // Enabling bus mastering resumes a ring armed while blocked
        if ((value8 ^ oldval) & 0x04) {
          pci_conf[0x04] = value8;
          if ((value8 & 0x04) && cp_thread_started)
            pm4_kick();
        }
        break;
      case 0x05:
        value8 = oldval;
        break;
      case 0x06:
        value8 = oldval;
        break;
      case 0x07:
        // status bits are write-one-to-clear
        value8 = oldval & ~(value8 & 0xf9);
        break;
      case 0x0c:
      case 0x0d:
        break;
      case 0x2c: case 0x2d: case 0x2e: case 0x2f:
        // subsystem ID is programmable through the 0x4c alias only
        value8 = oldval;
        break;
      case 0x4c: case 0x4d: case 0x4e: case 0x4f:
        pci_conf[0x2c + (a - 0x4c)] = value8;
        value8 = oldval;
        break;
      case 0x54:
        value8 &= 0x03;
        value8 |= (oldval & 0x00);
        break;
      case 0x55:
        // PME_En [8], PME_Status [15] (RW1C)
        value8 = (Bit8u)((oldval & ~0x81) | (value8 & 0x01)) & (Bit8u)~((value8 & 0x80) ? 0x80 : 0x00);
        break;
      case 0x60: case 0x61: case 0x62: case 0x63:
        if (is_agp)
          value8 &= (R9800_AGP_COMMAND_MASK >> ((a - 0x60) * 8)) & 0xff;
        else
          value8 = oldval;
        break;
      default:
        value8 = oldval;
    }
    pci_conf[a] = value8;
  }
}

// ---------------------------------------------------------------------
// Secondary display function
// ---------------------------------------------------------------------

bx_radeon9800_sec_c::bx_radeon9800_sec_c(bx_radeon9800_c *primary)
{
  dev = primary;
  devfunc = 1;
}

bx_radeon9800_sec_c::~bx_radeon9800_sec_c()
{
}

void bx_radeon9800_sec_c::init_pci(bool is_agp)
{
  DEV_register_pci_handlers2(this, &devfunc, BX_PLUGIN_RADEON9800,
                             "ATI Radeon 9800 (Secondary)", is_agp);
  init_pci_conf(R9800_PCI_VENDOR, R9800_PCI_DEVICE_SEC, R9800_PCI_REVISION, 0x038000, 0x00, 0);
  pci_conf[0x10] = 0x08;
  init_bar_mem(0, R9800_LFB_SIZE, mem_read_handler, mem_write_handler);
  init_bar_mem(1, R9800_MMIO_SIZE, mem_read_handler, mem_write_handler);
  pci_conf[0x06] = 0xb0;
  pci_conf[0x07] = 0x02;
  pci_conf[0x0c] = 0x08;
  pci_conf[0x2c] = R9800_SUBSYS_VENDOR & 0xff;
  pci_conf[0x2d] = R9800_SUBSYS_VENDOR >> 8;
  pci_conf[0x2e] = R9800_SUBSYS_ID & 0xff;
  pci_conf[0x2f] = R9800_SUBSYS_ID >> 8;
  pci_conf[0x34] = R9800_PM_CAP_OFF_SEC;
  pci_conf[R9800_PM_CAP_OFF_SEC + 0] = R9800_PM_CAP_ID;
  pci_conf[R9800_PM_CAP_OFF_SEC + 1] = 0x00;
  pci_conf[R9800_PM_CAP_OFF_SEC + 2] = R9800_PM_PMC & 0xff;
  pci_conf[R9800_PM_CAP_OFF_SEC + 3] = R9800_PM_PMC >> 8;
}

void bx_radeon9800_sec_c::reset_pci(void)
{
  pci_conf[0x04] = 0x00;
  pci_conf[0x05] = 0x00;
  pci_conf[0x0c] = 0x08;
  pci_conf[0x0d] = 0x00;
  pci_conf[0x54] = 0x00;
  pci_conf[0x55] = 0x00;
}

void bx_radeon9800_sec_c::pci_write_handler(Bit8u address, Bit32u value, unsigned io_len)
{
  Bit8u value8, oldval;

  if ((address >= 0x1c) && (address < 0x2c))
    return;
  for (unsigned i = 0; i < io_len; i++) {
    Bit8u a = (Bit8u)(address + i);
    value8 = (value >> (i * 8)) & 0xff;
    oldval = pci_conf[a];
    switch (a) {
      case 0x04:
        value8 &= 0x47;
        break;
      case 0x0c:
      case 0x0d:
        break;
      case 0x54:
        value8 &= 0x03;
        break;
      case 0x55:
        value8 = (Bit8u)((oldval & ~0x81) | (value8 & 0x01)) & (Bit8u)~((value8 & 0x80) ? 0x80 : 0x00);
        break;
      default:
        value8 = oldval;
    }
    pci_conf[a] = value8;
  }
}

void bx_radeon9800_sec_c::register_sec_state(bx_list_c *parent)
{
  bx_list_c *list = new bx_list_c(parent, "secondary");
  register_pci_state(list);
}

void bx_radeon9800_sec_c::after_restore(void)
{
  after_restore_pci_state();
}

bool bx_radeon9800_sec_c::mem_read_handler(bx_phy_address addr, unsigned len, void *data, void *param)
{
  bx_radeon9800_sec_c *s = (bx_radeon9800_sec_c *)param;
  Bit8u *data_ptr = (Bit8u *)data;

  if ((s->pci_bar[1].addr != 0) && (addr >= s->pci_bar[1].addr) &&
      (addr < (s->pci_bar[1].addr + R9800_MMIO_SIZE))) {
    s->dev->mmio_read((Bit32u)(addr - s->pci_bar[1].addr), len, data_ptr);
    return true;
  }
  if ((s->pci_bar[0].addr != 0) && (addr >= s->pci_bar[0].addr) &&
      (addr < (s->pci_bar[0].addr + R9800_LFB_SIZE))) {
    return s->dev->lfb_read((Bit32u)(addr - s->pci_bar[0].addr), len, data_ptr);
  }
  memset(data_ptr, 0xff, len);
  return true;
}

bool bx_radeon9800_sec_c::mem_write_handler(bx_phy_address addr, unsigned len, void *data, void *param)
{
  bx_radeon9800_sec_c *s = (bx_radeon9800_sec_c *)param;
  Bit8u *data_ptr = (Bit8u *)data;

  if ((s->pci_bar[1].addr != 0) && (addr >= s->pci_bar[1].addr) &&
      (addr < (s->pci_bar[1].addr + R9800_MMIO_SIZE))) {
    s->dev->mmio_write((Bit32u)(addr - s->pci_bar[1].addr), len, data_ptr);
    return true;
  }
  if ((s->pci_bar[0].addr != 0) && (addr >= s->pci_bar[0].addr) &&
      (addr < (s->pci_bar[0].addr + R9800_LFB_SIZE))) {
    return s->dev->lfb_write((Bit32u)(addr - s->pci_bar[0].addr), len, data_ptr);
  }
  return true;
}

// ---------------------------------------------------------------------
// Interrupts (GEN_INT_CNTL / GEN_INT_STATUS): INTA is level-triggered
// while any enabled source has its latched status bit set.
// ---------------------------------------------------------------------

void bx_radeon9800_c::set_irq_level(bool level)
{
  if (level && !trace_irq_level)
    trace_irq_count++;
  trace_irq_level = level;
  DEV_pci_set_irq(devfunc, pci_conf[0x3d], level);
}

void bx_radeon9800_c::gen_int_update(void)
{
  // INTA is driven from the CPU thread only: a change made by a ring packet
  // on the CP thread is folded in by the next display update / vertical timer
  if (on_cp_thread()) {
    irq_dirty = true;
    return;
  }
  set_irq_level((gen_int_cntl & gen_int_status & R9800_INT_SUPPORTED) != 0);
}

// Re-evaluate deferred CPU-thread side effects of CP-thread register writes
void bx_radeon9800_c::fold_deferred(void)
{
  if (irq_dirty) {
    irq_dirty = false;
    set_irq_level((gen_int_cntl & gen_int_status & R9800_INT_SUPPORTED) != 0);
  }
  if (timing_dirty) {
    timing_dirty = false;
    timing_recalc();
  }
  if (palette_dirty) {
    palette_dirty = false;
    palette_sync_gui();
    needs_update_dispentire = true;
  }
}

void bx_radeon9800_c::timing_update(void)
{
  if (on_cp_thread())
    timing_dirty = true;
  else
    timing_recalc();
}

// Hand the CRTC timing to the VGA core only when it actually changed: the
// core's calculate_retrace_timing() restarts its vertical timer.
void bx_radeon9800_c::timing_recalc(void)
{
  bx_crtc_params_t p;
  Bit32u vclock = 0;

  get_crtc_params(&p, &vclock);
  if (timing_valid && (vclock == timing_vclock) &&
      (p.htotal == timing_last.htotal) && (p.vtotal == timing_last.vtotal) &&
      (p.vbstart == timing_last.vbstart) && (p.vrstart == timing_last.vrstart))
    return;
  timing_valid = true;
  timing_last = p;
  timing_vclock = vclock;
  bx_vgacore_c::calculate_retrace_timing();
}

// Fold the engine's busy->idle event into GEN_INT_STATUS[19] (CPU thread only)
void bx_radeon9800_c::gen_int_fold_gui_idle(void)
{
  if (gui_idle_event) {
    gui_idle_event = 0;
    gen_int_status |= R9800_INT_GUI_IDLE;
    gen_int_update();
  }
}

// ---------------------------------------------------------------------
// Timers
// ---------------------------------------------------------------------

void bx_radeon9800_c::vline_timer_handler(void *this_ptr)
{
  bx_radeon9800_c *class_ptr = (bx_radeon9800_c *)this_ptr;
  class_ptr->gen_int_status |= R9800_INT_CRTC_VLINE;
  if (class_ptr->crtc2_gen_cntl & R9800_CRTC_EN)
    class_ptr->gen_int_status |= R9800_INT_CRTC2_VLINE;
  class_ptr->gen_int_update();
}

void bx_radeon9800_c::pump_timer_handler(void *this_ptr)
{
  bx_radeon9800_c *class_ptr = (bx_radeon9800_c *)this_ptr;
  class_ptr->pm4_pump();
}

// Microseconds into the current frame (0 = display start)
Bit32u bx_radeon9800_c::get_display_usec(void)
{
  Bit64u display_usec = bx_virt_timer.time_usec(BX_R9800_THIS vsync_realtime) - BX_R9800_THIS s.display_start_usec;
  if (BX_R9800_THIS s.vtotal_usec == 0) return 0;
  return (Bit32u)(display_usec % BX_R9800_THIS s.vtotal_usec);
}

Bit32u bx_radeon9800_c::get_current_line(void)
{
  if (BX_R9800_THIS s.htotal_usec == 0) return 0;
  return get_display_usec() / BX_R9800_THIS s.htotal_usec;
}

bool bx_radeon9800_c::in_vblank(void)
{
  return get_display_usec() >= BX_R9800_THIS s.vblank_usec;
}

// The VGA core's vertical timer fires twice per frame: once at the end of
// the vertical retrace (toggle = 1) and once at display start (toggle = 0).
void bx_radeon9800_c::vertical_timer(void)
{
  bx_vgacore_c::vertical_timer();
  fold_deferred();
  if ((trace_mask & 8) && BX_R9800_THIS vtimer_toggle) {
    if (++trace_hb_frames >= R9800_TRACE_HB_FRAMES) {
      trace_hb_frames = 0;
      trace_heartbeat();
    }
  }
  if (BX_R9800_THIS vtimer_toggle) {
    // vertical blank / sync started
    crtc[0].vblank_save = true;
    crtc[0].frame_count = (crtc[0].frame_count + 1) & 0x1fffff;
    gen_int_fold_gui_idle();
    gen_int_status |= R9800_INT_CRTC_VBLANK | R9800_INT_CRTC_VSYNC;
    if (crtc2_gen_cntl & R9800_CRTC_EN) {
      crtc[1].vblank_save = true;
      crtc[1].frame_count = (crtc[1].frame_count + 1) & 0x1fffff;
      gen_int_status |= R9800_INT_CRTC2_VBLANK | R9800_INT_CRTC2_VSYNC;
    }
    gen_int_update();

    // Deferred PLL atomic updates
    if (ppll_update_pending) {
      ppll_commit();
      ppll_update_pending = false;
      timing_recalc();
    }
    if (p2pll_update_pending) {
      p2pll_work[0] = pll_regs[R9800_PLL_P2PLL_REF_DIV];
      p2pll_work[1] = pll_regs[R9800_PLL_P2PLL_DIV_0];
      p2pll_update_pending = false;
      timing_recalc();
    }
    // Page flips: a pending CRTC_OFFSET takes effect at vertical blank
    for (int c = 0; c < 2; c++) {
      if (crtc[c].offset_pending && !crtc[c].offset_lock) {
        latch_crtc_offset(c);
        pm4_flip_notify();
      }
    }
    // OV0 / subpicture register latch happens once per frame
    ov0_frame_latch();
    // Hardware cursor tuple is picked up whole at vertical blank
    cursor_frame_latch();
  } else {
    // Display start: schedule the CRTC_VLINE compare-line event
    int c = disp_crtc;
    Bit32u cmp = crtc[c].vline & 0x7ff;
    Bit32u vtotal = disp_ext ? ((crtc[c].v_total_disp & 0x7ff) + 1) : 0;
    if (disp_ext && (cmp < vtotal) && (BX_R9800_THIS s.htotal_usec > 0)) {
      Bit32u usec = cmp * BX_R9800_THIS s.htotal_usec;
      if (usec == 0) usec = 1;
      bx_virt_timer.activate_timer(vline_timer_id, usec, 0);
    }
  }
}

// ---------------------------------------------------------------------
// Save / restore
// ---------------------------------------------------------------------

void bx_radeon9800_c::register_state(void)
{
  bx_list_c *list = new bx_list_c(SIM->get_bochs_root(), "radeon9800", "Radeon 9800 State");
  BX_R9800_THIS vgacore_register_state(list);
  new bx_shadow_data_c(list, "vram", BX_R9800_THIS s.memory, vram_size);
  new bx_shadow_num_c(list, "mm_index", &mm_index, BASE_HEX);
  new bx_shadow_data_c(list, "bios_scratch", (Bit8u*)bios_scratch, sizeof(bios_scratch));
  new bx_shadow_num_c(list, "bus_cntl", &bus_cntl, BASE_HEX);
  new bx_shadow_num_c(list, "bus_cntl1", &bus_cntl1, BASE_HEX);
  new bx_shadow_num_c(list, "config_cntl", &config_cntl, BASE_HEX);
  new bx_shadow_num_c(list, "config_memsize", &config_memsize, BASE_HEX);
  new bx_shadow_num_c(list, "host_path_cntl", &host_path_cntl, BASE_HEX);
  new bx_shadow_num_c(list, "hdp_fb_location", &hdp_fb_location, BASE_HEX);
  new bx_shadow_num_c(list, "mem_cntl", &mem_cntl, BASE_HEX);
  new bx_shadow_num_c(list, "mem_timing_cntl", &mem_timing_cntl, BASE_HEX);
  new bx_shadow_num_c(list, "mc_fb_location", &mc_fb_location, BASE_HEX);
  new bx_shadow_num_c(list, "mc_agp_location", &mc_agp_location, BASE_HEX);
  new bx_shadow_num_c(list, "agp_base", &agp_base, BASE_HEX);
  new bx_shadow_num_c(list, "agp_base_2", &agp_base_2, BASE_HEX);
  new bx_shadow_num_c(list, "agp_cntl", &agp_cntl, BASE_HEX);
  new bx_shadow_num_c(list, "aic_cntl", &aic_cntl, BASE_HEX);
  new bx_shadow_num_c(list, "aic_pt_base", &aic_pt_base, BASE_HEX);
  new bx_shadow_num_c(list, "aic_lo_addr", &aic_lo_addr, BASE_HEX);
  new bx_shadow_num_c(list, "aic_hi_addr", &aic_hi_addr, BASE_HEX);
  new bx_shadow_num_c(list, "mc_ind_index", &mc_ind_index, BASE_HEX);
  new bx_shadow_data_c(list, "mc_ind_regs", (Bit8u*)mc_ind_regs, sizeof(mc_ind_regs));
  new bx_shadow_num_c(list, "rbbm_soft_reset", &rbbm_soft_reset, BASE_HEX);
  new bx_shadow_num_c(list, "surface_cntl", &surface_cntl, BASE_HEX);
  new bx_shadow_data_c(list, "surf_lower", (Bit8u*)surf_lower, sizeof(surf_lower));
  new bx_shadow_data_c(list, "surf_upper", (Bit8u*)surf_upper, sizeof(surf_upper));
  new bx_shadow_data_c(list, "surf_info", (Bit8u*)surf_info, sizeof(surf_info));
  new bx_shadow_data_c(list, "gpio", (Bit8u*)gpio, sizeof(gpio));
  new bx_shadow_num_c(list, "gen_int_cntl", &gen_int_cntl, BASE_HEX);
  new bx_shadow_num_c(list, "gen_int_status", &gen_int_status, BASE_HEX);
  new bx_shadow_num_c(list, "disp_output_cntl", &disp_output_cntl, BASE_HEX);
  new bx_shadow_num_c(list, "disp_merge_cntl", &disp_merge_cntl, BASE_HEX);
  new bx_shadow_num_c(list, "dac_cntl2", &dac_cntl2, BASE_HEX);
  new bx_shadow_num_c(list, "tv_dac_cntl", &tv_dac_cntl, BASE_HEX);
  new bx_shadow_data_c(list, "tv_regs", (Bit8u*)tv_regs, sizeof(tv_regs));
  new bx_shadow_data_c(list, "fp_regs", (Bit8u*)fp_regs, sizeof(fp_regs));
  new bx_shadow_data_c(list, "fp2_regs", (Bit8u*)fp2_regs, sizeof(fp2_regs));

  bx_list_c *disp = new bx_list_c(list, "display");
  new bx_shadow_data_c(disp, "pll_regs", (Bit8u*)pll_regs, sizeof(pll_regs));
  new bx_shadow_data_c(disp, "ppll_work", (Bit8u*)ppll_work, sizeof(ppll_work));
  new bx_shadow_data_c(disp, "p2pll_work", (Bit8u*)p2pll_work, sizeof(p2pll_work));
  new bx_shadow_num_c(disp, "clock_cntl_index", &clock_cntl_index, BASE_HEX);
  BXRS_PARAM_BOOL(disp, ppll_update_pending, ppll_update_pending);
  BXRS_PARAM_BOOL(disp, p2pll_update_pending, p2pll_update_pending);
  new bx_shadow_num_c(disp, "crtc_gen_cntl", &crtc_gen_cntl, BASE_HEX);
  new bx_shadow_num_c(disp, "crtc_ext_cntl", &crtc_ext_cntl, BASE_HEX);
  new bx_shadow_num_c(disp, "crtc2_gen_cntl", &crtc2_gen_cntl, BASE_HEX);
  new bx_shadow_num_c(disp, "dac_cntl", &dac_cntl, BASE_HEX);
  new bx_shadow_num_c(disp, "dac_mask_prog", &dac_mask_prog, BASE_HEX);
  new bx_shadow_num_c(disp, "dac_ext_cntl", &dac_ext_cntl, BASE_HEX);
  new bx_shadow_num_c(disp, "mem_vga_wp_sel", &mem_vga_wp_sel, BASE_HEX);
  new bx_shadow_num_c(disp, "mem_vga_rp_sel", &mem_vga_rp_sel, BASE_HEX);
  new bx_shadow_num_c(disp, "palette_index", &palette_index, BASE_HEX);
  new bx_shadow_data_c(disp, "palette2", (Bit8u*)palette2, sizeof(palette2));
  new bx_shadow_data_c(disp, "palette30", (Bit8u*)palette30, sizeof(palette30));
  new bx_shadow_data_c(disp, "ext_crtc", ext_crtc, sizeof(ext_crtc));
  new bx_shadow_data_c(disp, "crtc", (Bit8u*)crtc, sizeof(crtc));
  BXRS_PARAM_BOOL(disp, cur_lat_en, cur_lat_en);
  new bx_shadow_num_c(disp, "cur_lat_mode", &cur_lat_mode, BASE_HEX);
  new bx_shadow_num_c(disp, "cur_lat_offset", &cur_lat_offset, BASE_HEX);
  new bx_shadow_num_c(disp, "cur_lat_posn", &cur_lat_posn, BASE_HEX);
  new bx_shadow_num_c(disp, "cur_lat_hvoff", &cur_lat_hvoff, BASE_HEX);
  new bx_shadow_num_c(disp, "cur_lat_clr0", &cur_lat_clr0, BASE_HEX);
  new bx_shadow_num_c(disp, "cur_lat_clr1", &cur_lat_clr1, BASE_HEX);

  bx_list_c *eng = new bx_list_c(list, "gui");
  new bx_shadow_num_c(eng, "dp_gui_master_cntl", &dp_gui_master_cntl, BASE_HEX);
  new bx_shadow_num_c(eng, "dp_brush_frgd_clr", &dp_brush_frgd_clr, BASE_HEX);
  new bx_shadow_num_c(eng, "dp_brush_bkgd_clr", &dp_brush_bkgd_clr, BASE_HEX);
  new bx_shadow_data_c(eng, "brush_data", (Bit8u*)brush_data, sizeof(brush_data));
  new bx_shadow_num_c(eng, "dp_src_frgd_clr", &dp_src_frgd_clr, BASE_HEX);
  new bx_shadow_num_c(eng, "dp_src_bkgd_clr", &dp_src_bkgd_clr, BASE_HEX);
  new bx_shadow_num_c(eng, "dp_cntl", &dp_cntl, BASE_HEX);
  new bx_shadow_num_c(eng, "dp_datatype", &dp_datatype, BASE_HEX);
  new bx_shadow_num_c(eng, "dp_mix", &dp_mix, BASE_HEX);
  new bx_shadow_num_c(eng, "dp_write_mask", &dp_write_mask, BASE_HEX);
  new bx_shadow_num_c(eng, "clr_cmp_clr_src", &clr_cmp_clr_src, BASE_HEX);
  new bx_shadow_num_c(eng, "clr_cmp_clr_dst", &clr_cmp_clr_dst, BASE_HEX);
  new bx_shadow_num_c(eng, "clr_cmp_cntl", &clr_cmp_cntl, BASE_HEX);
  new bx_shadow_num_c(eng, "clr_cmp_mask", &clr_cmp_mask, BASE_HEX);
  new bx_shadow_num_c(eng, "aux_sc_cntl", &aux_sc_cntl, BASE_HEX);
  new bx_shadow_data_c(eng, "aux_sc_rect", (Bit8u*)aux_sc_rect, sizeof(aux_sc_rect));
  new bx_shadow_num_c(eng, "default_pitch_offset", &default_pitch_offset, BASE_HEX);
  new bx_shadow_num_c(eng, "default_sc_bottom_right", &default_sc_bottom_right, BASE_HEX);
  new bx_shadow_num_c(eng, "sc_top_left", &sc_top_left, BASE_HEX);
  new bx_shadow_num_c(eng, "sc_bottom_right", &sc_bottom_right, BASE_HEX);
  new bx_shadow_num_c(eng, "src_offset", &src_offset, BASE_HEX);
  new bx_shadow_num_c(eng, "src_pitch", &src_pitch, BASE_HEX);
  new bx_shadow_num_c(eng, "dst_offset", &dst_offset, BASE_HEX);
  new bx_shadow_num_c(eng, "dst_pitch", &dst_pitch, BASE_HEX);
  new bx_shadow_num_c(eng, "src_tile", &src_tile, BASE_HEX);
  new bx_shadow_num_c(eng, "dst_tile", &dst_tile, BASE_HEX);
  new bx_shadow_num_c(eng, "gui_dst_x", &gui_dst_x);
  new bx_shadow_num_c(eng, "gui_dst_y", &gui_dst_y);
  new bx_shadow_num_c(eng, "gui_src_x", &gui_src_x);
  new bx_shadow_num_c(eng, "gui_src_y", &gui_src_y);
  new bx_shadow_num_c(eng, "gui_dst_w", &gui_dst_w);
  new bx_shadow_num_c(eng, "gui_dst_h", &gui_dst_h);
  new bx_shadow_data_c(eng, "gui_scratch", (Bit8u*)gui_scratch, sizeof(gui_scratch));
  new bx_shadow_num_c(eng, "scratch_umsk", &scratch_umsk, BASE_HEX);
  new bx_shadow_num_c(eng, "scratch_addr", &scratch_addr, BASE_HEX);
  new bx_shadow_data_c(eng, "scl_palette", (Bit8u*)scl_palette, sizeof(scl_palette));

  bx_list_c *pm4 = new bx_list_c(list, "cp");
  new bx_shadow_num_c(pm4, "rb_base", &cp_rb_base, BASE_HEX);
  new bx_shadow_num_c(pm4, "rb_cntl", &cp_rb_cntl, BASE_HEX);
  new bx_shadow_num_c(pm4, "rb_rptr_addr", &cp_rb_rptr_addr, BASE_HEX);
  new bx_shadow_num_c(pm4, "rb_rptr", &cp_rb_rptr, BASE_HEX);
  new bx_shadow_num_c(pm4, "rb_wptr", &cp_rb_wptr, BASE_HEX);
  new bx_shadow_num_c(pm4, "ib_base", &cp_ib_base, BASE_HEX);
  new bx_shadow_num_c(pm4, "ib_bufsz", &cp_ib_bufsz, BASE_HEX);
  new bx_shadow_num_c(pm4, "csq_cntl", &cp_csq_cntl, BASE_HEX);
  new bx_shadow_num_c(pm4, "csq_mode", &cp_csq_mode, BASE_HEX);
  new bx_shadow_num_c(pm4, "me_ram_addr", &cp_me_ram_addr, BASE_HEX);
  new bx_shadow_num_c(pm4, "vc_debug_config", &cp_vc_debug_config, BASE_HEX);
  new bx_shadow_data_c(pm4, "me_ram", (Bit8u*)cp_me_ram, sizeof(cp_me_ram));

  bx_list_c *t3 = new bx_list_c(list, "r3d");
  new bx_shadow_data_c(t3, "vap_regs", (Bit8u*)vap_regs, sizeof(vap_regs));
  new bx_shadow_data_c(t3, "r3d_regs", (Bit8u*)r3d_regs, sizeof(r3d_regs));
  new bx_shadow_data_c(t3, "vport_alt", (Bit8u*)vport_alt, sizeof(vport_alt));
  new bx_shadow_data_c(t3, "pvs_vec", (Bit8u*)pvs_vec, sizeof(pvs_vec));
  new bx_shadow_num_c(t3, "pvs_vec_index", &pvs_vec_index, BASE_HEX);
  new bx_shadow_data_c(t3, "zmask_ram", (Bit8u*)zmask_ram, sizeof(zmask_ram));
  new bx_shadow_data_c(t3, "hiz_ram", (Bit8u*)hiz_ram, sizeof(hiz_ram));
  new bx_shadow_data_c(t3, "cmask_ram", (Bit8u*)cmask_ram, sizeof(cmask_ram));
  new bx_shadow_data_c(t3, "vtx_state", (Bit8u*)vtx_state, sizeof(vtx_state));

  bx_list_c *ov = new bx_list_c(list, "ov0");
  new bx_shadow_data_c(ov, "shadow", (Bit8u*)ov0.shadow, sizeof(ov0.shadow));
  new bx_shadow_data_c(ov, "active", (Bit8u*)ov0.active, sizeof(ov0.active));
  new bx_shadow_num_c(ov, "reg_load_cntl", &ov0_reg_load_cntl, BASE_HEX);
  new bx_shadow_data_c(ov, "lin_trans", (Bit8u*)ov0_lin_trans, sizeof(ov0_lin_trans));
  new bx_shadow_data_c(ov, "gamma", (Bit8u*)ov0_gamma_regs, sizeof(ov0_gamma_regs));
  new bx_shadow_data_c(ov, "subpic_regs", (Bit8u*)subpic.regs, sizeof(subpic.regs));
  new bx_shadow_data_c(ov, "subpic_pal", (Bit8u*)subpic.pal, sizeof(subpic.pal));
  register_pci_state(list);
  if (sec != NULL)
    sec->register_sec_state(list);
}

void bx_radeon9800_c::after_restore_state(void)
{
  bx_pci_device_c::after_restore_pci_state();
  if (sec != NULL)
    sec->after_restore();
  BX_R9800_THIS bx_vgacore_c::after_restore_state();
  update_banking();
  surf_xlate_on = false;
  for (int n = 0; n < 8; n++)
    if (surf_info[n] & (R9800_SURF_TILE_MASK | R9800_SURF_PITCH_MASK)) surf_xlate_on = true;
  ov0_update();
  cursor_publish(0);
  cursor_publish(1);
  timing_valid = false;
  timing_recalc();
  gui_xres = gui_yres = gui_bpp = 0;
  needs_update_mode = true;
  needs_update_dispentire = true;
  r3d_serial++;
  gen_int_update();
  if (!cp_thread_started) {
    pm4_thread_init();
  }
}

// ---------------------------------------------------------------------
// Legacy VGA I/O passthrough
// ---------------------------------------------------------------------

Bit32u bx_radeon9800_c::svga_read_handler(void *this_ptr, Bit32u address, unsigned io_len)
{
  bx_radeon9800_c *class_ptr = (bx_radeon9800_c *)this_ptr;
  return class_ptr->svga_read(address, io_len);
}

void bx_radeon9800_c::svga_write_handler(void *this_ptr, Bit32u address, Bit32u value, unsigned io_len)
{
  bx_radeon9800_c *class_ptr = (bx_radeon9800_c *)this_ptr;
  class_ptr->svga_write(address, value, io_len);
}

bool bx_radeon9800_c::ext_mode(void)
{
  return (crtc_gen_cntl & R9800_CRTC_EXT_DISP_EN) != 0;
}

Bit32u bx_radeon9800_c::svga_read(Bit32u address, unsigned io_len)
{
  if (io_len == 2) {
    Bit32u v = svga_read(address, 1);
    v |= svga_read(address + 1, 1) << 8;
    return v;
  }
  if (vga_disabled || (config_cntl & R9800_CFG_VGA_IO_DIS))
    return 0xff;

  // The VGA I/O DAC addresses are gated in extended modes
  if ((address >= 0x3c6) && (address <= 0x3c9) && disp_ext && !(dac_cntl & R9800_DAC_VGA_ADR_EN))
    return 0xff;

  switch (address) {
    case 0x03c2: // Input Status 0: monitor sense asserted
      return bx_vgacore_c::read(address, 1) | 0x10;
    case 0x03c6:
      return dac_mask_prog;
    case 0x03b5:
    case 0x03d5:
      if (BX_R9800_THIS s.CRTC.address > 0x18) {
        if (BX_R9800_THIS s.CRTC.address <= 0x3f)
          return ext_crtc[BX_R9800_THIS s.CRTC.address];
        return 0xff;
      }
      break;
    default:
      break;
  }
  return bx_vgacore_c::read(address, io_len);
}

void bx_radeon9800_c::svga_write(Bit32u address, Bit32u value, unsigned io_len)
{
  if (io_len == 2) {
    svga_write(address, value & 0xff, 1);
    svga_write(address + 1, (value >> 8) & 0xff, 1);
    return;
  }
  if (vga_disabled || (config_cntl & R9800_CFG_VGA_IO_DIS))
    return;
  if ((address >= 0x3c6) && (address <= 0x3c9) && disp_ext && !(dac_cntl & R9800_DAC_VGA_ADR_EN))
    return;

  switch (address) {
    case 0x03c6:
      dac_mask_prog = (Bit8u)value;
      break;
    case 0x03c9:
      if (disp_ext) needs_update_dispentire = true;
      break;
    case 0x03b4:
    case 0x03d4:
      BX_R9800_THIS s.CRTC.address = value & 0x3f;
      if (BX_R9800_THIS s.CRTC.address > 0x18)
        return;
      break;
    case 0x03b5:
    case 0x03d5:
      if (BX_R9800_THIS s.CRTC.address > 0x18) {
        if (BX_R9800_THIS s.CRTC.address <= 0x3f)
          ext_crtc[BX_R9800_THIS s.CRTC.address] = (Bit8u)value;
        return;
      }
      if ((BX_R9800_THIS s.CRTC.address < 7) && (BX_R9800_THIS s.CRTC.reg[0x11] & 0x80))
        return;
      break;
    default:
      break;
  }
  bx_vgacore_c::write(address, value, io_len, 0);

  // VGA vertical-display-end writes through to the CRTC_V_TOTAL_DISP readback
  if (((address == 0x3d5) || (address == 0x3b5)) &&
      ((BX_R9800_THIS s.CRTC.address == 0x07) || (BX_R9800_THIS s.CRTC.address == 0x12))) {
    Bit32u vdisp = BX_R9800_THIS s.CRTC.reg[0x12] |
                   ((BX_R9800_THIS s.CRTC.reg[7] & 0x02) << 7) |
                   ((BX_R9800_THIS s.CRTC.reg[7] & 0x40) << 3);
    crtc[0].v_total_disp = (crtc[0].v_total_disp & ~0x07ff0000) | (vdisp << 16);
  }
}

// IO BAR1: register file offsets 0x00-0xff
Bit32u bx_radeon9800_c::io_read_handler(void *this_ptr, Bit32u address, unsigned io_len)
{
  bx_radeon9800_c *class_ptr = (bx_radeon9800_c *)this_ptr;
  Bit32u off = (address - class_ptr->pci_bar[1].addr) & 0xff;
  Bit32u v = class_ptr->reg_read32(off & 0xfc) >> ((off & 3) * 8);
  if (io_len == 1) v &= 0xff;
  else if (io_len == 2) v &= 0xffff;
  return v;
}

void bx_radeon9800_c::io_write_handler(void *this_ptr, Bit32u address, Bit32u value, unsigned io_len)
{
  bx_radeon9800_c *class_ptr = (bx_radeon9800_c *)this_ptr;
  Bit32u off = (address - class_ptr->pci_bar[1].addr) & 0xff;
  Bit32u sh = (off & 3) * 8;
  Bit32u mask = (io_len == 1) ? 0xff : (io_len == 2) ? 0xffff : 0xffffffff;
  class_ptr->reg_write(off & 0xfc, value << sh, mask << sh);
}

// ---------------------------------------------------------------------
// Memory controller address space
// ---------------------------------------------------------------------

bool bx_radeon9800_c::bus_master_ok(void)
{
  return (pci_conf[0x04] & 0x04) && !(bus_cntl & R9800_BUS_MASTER_DIS);
}

// Bochs memory accesses must stay within one 4 KB page: the memory handler
// is looked up for the start address and receives the whole length, and the
// i440BX AGP aperture handler translates only the page of the start address
// through the GART.
void bx_radeon9800_c::phys_read_chunked(Bit32u addr, Bit8u *dst, Bit32u len)
{
  while (len) {
    Bit32u n = 0x1000 - (addr & 0xfff);
    if (n > len) n = len;
    DEV_MEM_READ_PHYSICAL(addr, n, dst);
    addr += n;
    dst += n;
    len -= n;
  }
}

void bx_radeon9800_c::phys_write_chunked(Bit32u addr, const Bit8u *src, Bit32u len)
{
  while (len) {
    Bit32u n = 0x1000 - (addr & 0xfff);
    if (n > len) n = len;
    DEV_MEM_WRITE_PHYSICAL(addr, n, (Bit8u*)src);
    addr += n;
    src += n;
    len -= n;
  }
}

bool bx_radeon9800_c::bus_read_phys(Bit32u phys, Bit8u *dst, Bit32u len)
{
  if (!bus_master_ok())
    return false;
  phys_read_chunked(phys, dst, len);
  return true;
}

bool bx_radeon9800_c::bus_write_phys(Bit32u phys, const Bit8u *src, Bit32u len)
{
  if (!bus_master_ok())
    return false;
  phys_write_chunked(phys, src, len);
  return true;
}

// MC address inside the local frame buffer window?
bool bx_radeon9800_c::mc_is_vram(Bit32u mc, Bit32u *vram_off)
{
  Bit32u fb_start = (mc_fb_location & 0xffff) << 16;
  Bit32u fb_top = (mc_fb_location & 0xffff0000) | 0xffff;
  if ((fb_start <= fb_top) && (mc >= fb_start) && (mc <= fb_top)) {
    *vram_off = (mc - fb_start) & vram_mask;
    return true;
  }
  return false;
}

// Does an MC address land in a window the memory controller actually maps?
// Anything outside them falls back to a raw bus-master access at the same
// numeric address, so treating a small value as an address would scribble
// over low guest physical memory.
bool bx_radeon9800_c::mc_addr_is_mapped(Bit32u mc)
{
  Bit32u off;
  Bit32u agp_start = (mc_agp_location & 0xffff) << 16;
  Bit32u agp_top = (mc_agp_location & 0xffff0000) | 0xffff;

  if (mc_is_vram(mc, &off))
    return true;
  if (agp_base && (agp_start <= agp_top) && (mc >= agp_start) && (mc <= agp_top))
    return true;
  if ((aic_cntl & R9800_AIC_TRANSLATE_EN) && (mc >= aic_lo_addr) && (mc <= aic_hi_addr))
    return true;
  return false;
}

// Resolve an MC address: kind 0 = local VRAM (addr = byte offset), kind 1 =
// system memory (addr = physical bus address). The AGP window maps through
// AGP_BASE (the host bridge aperture translates through its GART), the PCI
// GART (AIC) window walks the page table, everything else is a direct bus
// master access.
bool bx_radeon9800_c::mc_resolve(Bit32u mc, Bit32u *kind, Bit32u *addr)
{
  Bit32u off;
  if (mc_is_vram(mc, &off)) {
    *kind = 0;
    *addr = off;
    return true;
  }
  Bit32u agp_start = (mc_agp_location & 0xffff) << 16;
  Bit32u agp_top = (mc_agp_location & 0xffff0000) | 0xffff;
  if ((agp_start <= agp_top) && (mc >= agp_start) && (mc <= agp_top)) {
    if (!agp_base)
      return false;
    *kind = 1;
    *addr = agp_base + (mc - agp_start);
    return true;
  }
  if ((aic_cntl & R9800_AIC_TRANSLATE_EN) && (mc >= aic_lo_addr) && (mc <= aic_hi_addr)) {
    Bit32u pte, entry = (aic_pt_base & 0xfffffff0) + ((mc - aic_lo_addr) >> 12) * 4;
    Bit8u buf[4];
    if (!bus_master_ok())
      return false;
    DEV_MEM_READ_PHYSICAL(entry, 4, buf);
    pte = ReadHostDWordFromLittleEndian((Bit32u*)buf);
    *kind = 1;
    *addr = (pte & 0xfffff000) | (mc & 0xfff);
    return true;
  }
  *kind = 1;
  *addr = mc;
  return true;
}

bool bx_radeon9800_c::gpu_read(Bit32u mc, Bit8u *dst, Bit32u len)
{
  while (len) {
    Bit32u n = 0x1000 - (mc & 0xfff);
    Bit32u kind, addr;
    if (n > len) n = len;
    if (!mc_resolve(mc, &kind, &addr))
      return false;
    if (kind == 0) {
      for (Bit32u i = 0; i < n; i++)
        dst[i] = BX_R9800_THIS s.memory[(addr + i) & vram_mask];
    } else {
      if (!bus_read_phys(addr, dst, n))
        return false;
    }
    mc += n;
    dst += n;
    len -= n;
  }
  return true;
}

bool bx_radeon9800_c::gpu_write(Bit32u mc, const Bit8u *src, Bit32u len)
{
  while (len) {
    Bit32u n = 0x1000 - (mc & 0xfff);
    Bit32u kind, addr;
    if (n > len) n = len;
    if (!mc_resolve(mc, &kind, &addr))
      return false;
    if (kind == 0) {
      for (Bit32u i = 0; i < n; i++)
        BX_R9800_THIS s.memory[(addr + i) & vram_mask] = src[i];
      vram_dirty(addr & vram_mask, n);
    } else {
      if (!bus_write_phys(addr, src, n))
        return false;
    }
    mc += n;
    src += n;
    len -= n;
  }
  return true;
}

bool bx_radeon9800_c::gpu_read32(Bit32u mc, Bit32u *val)
{
  Bit8u buf[4];
  if (!gpu_read(mc & ~3u, buf, 4))
    return false;
  *val = ReadHostDWordFromLittleEndian((Bit32u*)buf);
  return true;
}

bool bx_radeon9800_c::gpu_write32(Bit32u mc, Bit32u val)
{
  Bit8u buf[4];
  WriteHostDWordToLittleEndian((Bit32u*)buf, val);
  return gpu_write(mc & ~3u, buf, 4);
}

// SURFACE0-7 host data path tiling windows: an HDP access whose MC address
// lies in a window is re-addressed through the surface's tiling.
Bit32u bx_radeon9800_c::hdp_surf_xlate(Bit32u mc)
{
  for (int n = 0; n < 8; n++) {
    Bit32u info = surf_info[n];
    Bit32u tile = info & R9800_SURF_TILE_MASK;
    Bit32u pitch = (info & R9800_SURF_PITCH_MASK) * 8;   // pitch in 8-byte units
    if (!tile || !pitch || (mc < surf_lower[n]) || (mc > surf_upper[n]))
      continue;
    Bit32u off = mc - surf_lower[n];
    Bit32u y = off / pitch, x = off % pitch;
    Bit32u micro = (tile & R9800_SURF_TILE_MICRO) ? 1 : 0;
    Bit32u macro = (tile & R9800_SURF_TILE_MACRO) ? 1 : 0;
    return surf_lower[n] + r300_tile_addr(x, y, pitch, 1, micro, macro);
  }
  return mc;
}

// BAR0 offset -> VRAM offset (the HDP aperture starts at the frame buffer)
Bit32u bx_radeon9800_c::vram_addr_xlate(Bit32u off)
{
  if (surf_xlate_on) {
    Bit32u fb_start = (mc_fb_location & 0xffff) << 16;
    Bit32u mc = hdp_surf_xlate(fb_start + off);
    return (mc - fb_start) & vram_mask;
  }
  return off & vram_mask;
}

// ---------------------------------------------------------------------
// Memory handlers: BAR0 (LFB), BAR2 (registers), ROM, legacy VGA window
// ---------------------------------------------------------------------

// Mark the display tiles covering VRAM bytes [addr, addr+len) as dirty
void bx_radeon9800_c::vram_dirty(Bit32u addr, Bit32u len)
{
  if (cur_lat_en) {
    Bit32u ca = addr & vram_mask;
    Bit32u csz = (cur_lat_mode == 2) ? 64u * 64u * 4u : 64u * 16u;
    if ((ca < cur_lat_offset + csz) && (ca + len > cur_lat_offset))
      cur_bitmap_dirty = true;
  }
  if (!disp_ext || (disp_pitch == 0)) {
    BX_R9800_THIS s.vga_mem_updated |= 1;
    bx_vgacore_c::vga_redraw_area(0, 0, BX_R9800_THIS s.last_xres, BX_R9800_THIS s.last_yres);
    return;
  }
  if (scanout_tiled) {
    needs_update_dispentire = true;
    needs_update_tile = true;
    return;
  }
  addr &= vram_mask;
  Bit32u end = addr + len;
  Bit32u dend = disp_base + disp_pitch * (disp_dblscan ? (disp_yres + 1) / 2 : disp_yres);
  if ((end <= disp_base) || (addr >= dend))
    return;
  if (addr < disp_base) addr = disp_base;
  if (end > dend) end = dend;
  Bit32u y0 = (addr - disp_base) / disp_pitch;
  Bit32u y1 = (end - 1 - disp_base) / disp_pitch;
  if (disp_dblscan) { y0 <<= 1; y1 = (y1 << 1) + 1; }
  Bit32u yt0 = y0 / Y_TILESIZE;
  Bit32u yt1 = y1 / Y_TILESIZE;
  Bit32u xt1 = (disp_xres - 1) / X_TILESIZE;
  for (Bit32u yti = yt0; yti <= yt1; yti++)
    for (Bit32u xti = 0; xti <= xt1; xti++)
      SET_TILE_UPDATED(BX_R9800_THIS, xti, yti, 1);
  needs_update_tile = true;
}

bool bx_radeon9800_c::lfb_read(Bit32u off, unsigned len, Bit8u *data)
{
  off &= (R9800_LFB_SIZE - 1);
  Bit32u endian = (surface_cntl >> 20) & 3;
  if (off >= vram_size) {
    // Aperture 1 (the second half of a 128 MB window on a 64 MB board)
    endian = (surface_cntl >> 22) & 3;
    off &= vram_mask;
  }
  for (unsigned i = 0; i < len; i++) {
    Bit32u a = (off + i);
    if (endian == 1) a ^= 1; else if (endian == 2) a ^= 3;
    a = vram_addr_xlate(a);
    data[i] = BX_R9800_THIS s.memory[a & vram_mask];
  }
  return true;
}

bool bx_radeon9800_c::lfb_write(Bit32u off, unsigned len, Bit8u *data)
{
  off &= (R9800_LFB_SIZE - 1);
  Bit32u endian = (surface_cntl >> 20) & 3;
  if (off >= vram_size) {
    endian = (surface_cntl >> 22) & 3;
    off &= vram_mask;
  }
  Bit32u lo = 0xffffffff, hi = 0;
  for (unsigned i = 0; i < len; i++) {
    Bit32u a = (off + i);
    if (endian == 1) a ^= 1; else if (endian == 2) a ^= 3;
    a = vram_addr_xlate(a) & vram_mask;
    BX_R9800_THIS s.memory[a] = data[i];
    if (a < lo) lo = a;
    if (a > hi) hi = a;
  }
  if (hi >= lo) vram_dirty(lo, hi - lo + 1);
  return true;
}

void bx_radeon9800_c::mmio_read(Bit32u off, unsigned len, Bit8u *data)
{
  Bit32u a = off & R9800_REG_APER_MASK;
  if (len == 4) {
    WriteHostDWordToLittleEndian((Bit32u*)data, reg_read32(a & 0xfffc));
  } else if (len == 2) {
    Bit16u v = (Bit16u)(reg_read32(a & 0xfffc) >> ((a & 2) * 8));
    WriteHostWordToLittleEndian((Bit16u*)data, v);
  } else if (len == 1) {
    data[0] = (Bit8u)(reg_read32(a & 0xfffc) >> ((a & 3) * 8));
  } else {
    for (unsigned i = 0; i < len; i += 4)
      WriteHostDWordToLittleEndian((Bit32u*)(data + i), reg_read32((a + i) & 0xfffc));
  }
}

void bx_radeon9800_c::mmio_write(Bit32u off, unsigned len, Bit8u *data)
{
  Bit32u a = off & R9800_REG_APER_MASK;
  if (len == 4) {
    reg_write(a & 0xfffc, ReadHostDWordFromLittleEndian((Bit32u*)data), 0xffffffff);
  } else if (len == 2) {
    Bit32u sh = (a & 2) * 8;
    reg_write(a & 0xfffc, (Bit32u)ReadHostWordFromLittleEndian((Bit16u*)data) << sh, 0xffff << sh);
  } else if (len == 1) {
    Bit32u sh = (a & 3) * 8;
    reg_write(a & 0xfffc, (Bit32u)data[0] << sh, 0xff << sh);
  } else {
    for (unsigned i = 0; i < len; i += 4)
      reg_write((a + i) & 0xfffc, ReadHostDWordFromLittleEndian((Bit32u*)(data + i)), 0xffffffff);
  }
}

bool bx_radeon9800_c::mem_read_handler(bx_phy_address addr, unsigned len, void *data, void *param)
{
  bx_radeon9800_c *class_ptr = (bx_radeon9800_c *)param;
  Bit8u *data_ptr = (Bit8u *)data;

  // Expansion ROM
  if (class_ptr->pci_bar[PCI_ROM_BAR].size > 0) {
    Bit32u mask = class_ptr->pci_bar[PCI_ROM_BAR].size - 1;
    if (((Bit32u)addr & ~mask) == class_ptr->pci_bar[PCI_ROM_BAR].addr) {
      for (unsigned i = 0; i < len; i++) {
        if (class_ptr->pci_conf[0x30] & 0x01)
          data_ptr[i] = class_ptr->pci_rom[(addr + i) & mask];
        else
          data_ptr[i] = 0xff;
      }
      return true;
    }
  }
  // BAR2: register aperture
  if ((class_ptr->pci_bar[2].addr != 0) && (addr >= class_ptr->pci_bar[2].addr) &&
      (addr < (class_ptr->pci_bar[2].addr + R9800_MMIO_SIZE))) {
    class_ptr->mmio_read((Bit32u)(addr - class_ptr->pci_bar[2].addr), len, data_ptr);
    return true;
  }
  // BAR0: framebuffer aperture
  if ((class_ptr->pci_bar[0].addr != 0) && (addr >= class_ptr->pci_bar[0].addr) &&
      (addr < (class_ptr->pci_bar[0].addr + R9800_LFB_SIZE))) {
    return class_ptr->lfb_read((Bit32u)(addr - class_ptr->pci_bar[0].addr), len, data_ptr);
  }
  // Legacy VGA window
  for (unsigned i = 0; i < len; i++) {
    data_ptr[i] = class_ptr->mem_read(addr + i);
  }
  return true;
}

bool bx_radeon9800_c::mem_write_handler(bx_phy_address addr, unsigned len, void *data, void *param)
{
  bx_radeon9800_c *class_ptr = (bx_radeon9800_c *)param;
  Bit8u *data_ptr = (Bit8u *)data;

  if ((class_ptr->pci_bar[2].addr != 0) && (addr >= class_ptr->pci_bar[2].addr) &&
      (addr < (class_ptr->pci_bar[2].addr + R9800_MMIO_SIZE))) {
    class_ptr->mmio_write((Bit32u)(addr - class_ptr->pci_bar[2].addr), len, data_ptr);
    return true;
  }
  if ((class_ptr->pci_bar[0].addr != 0) && (addr >= class_ptr->pci_bar[0].addr) &&
      (addr < (class_ptr->pci_bar[0].addr + R9800_LFB_SIZE))) {
    return class_ptr->lfb_write((Bit32u)(addr - class_ptr->pci_bar[0].addr), len, data_ptr);
  }
  for (unsigned i = 0; i < len; i++) {
    class_ptr->mem_write(addr + i, data_ptr[i]);
  }
  return true;
}

// Legacy VGA aperture. The A0000 window is banked through MEM_VGA_WP/RP_SEL
// whenever the extended display path is enabled (or VGA_MEM_PS_EN is set);
// otherwise the VGA core owns it.
Bit8u bx_radeon9800_c::mem_read(bx_phy_address addr)
{
  if (vga_disabled || !(config_cntl & R9800_CFG_VGA_RAM_EN))
    return 0xff;
  bool paged = disp_ext || (crtc_ext_cntl & R9800_VGA_MEM_PS_EN);
  if (paged && (addr >= 0xa0000) && (addr <= 0xbffff)) {
    Bit32u off = ((Bit32u)addr & 0x7fff) + bank_r[((Bit32u)addr >> 15) & 1];
    return BX_R9800_THIS s.memory[off & vram_mask];
  }
  return bx_vgacore_c::mem_read(addr);
}

void bx_radeon9800_c::mem_write(bx_phy_address addr, Bit8u value)
{
  if (vga_disabled || !(config_cntl & R9800_CFG_VGA_RAM_EN))
    return;
  bool paged = disp_ext || (crtc_ext_cntl & R9800_VGA_MEM_PS_EN);
  if (paged && (addr >= 0xa0000) && (addr <= 0xbffff)) {
    Bit32u off = (((Bit32u)addr & 0x7fff) + bank_w[((Bit32u)addr >> 15) & 1]) & vram_mask;
    BX_R9800_THIS s.memory[off] = value;
    vram_dirty(off, 1);
    needs_update_dispentire = true;
    return;
  }
  bx_vgacore_c::mem_write(addr, value);
}

void bx_radeon9800_c::get_text_snapshot(Bit8u **text_snapshot, unsigned *txHeight, unsigned *txWidth)
{
  BX_R9800_THIS bx_vgacore_c::get_text_snapshot(text_snapshot, txHeight, txWidth);
}

// VGA aperture banking (MEM_VGA_WP/RP_SEL page the A0000 window)
void bx_radeon9800_c::update_banking(void)
{
  bool paged = disp_ext || (crtc_ext_cntl & R9800_VGA_MEM_PS_EN);
  Bit32u unit = (crtc_ext_cntl & R9800_VGA_ATI_LINEAR) ? (R9800_VGA_PAGE_SIZE * 4) : R9800_VGA_PAGE_SIZE;
  if (paged) {
    bank_w[0] = (mem_vga_wp_sel & 0x3ff) * unit;
    bank_w[1] = ((mem_vga_wp_sel >> 16) & 0x3ff) * unit;
    bank_r[0] = (mem_vga_rp_sel & 0x3ff) * unit;
    bank_r[1] = ((mem_vga_rp_sel >> 16) & 0x3ff) * unit;
  } else {
    bank_w[0] = bank_r[0] = 0x0000;
    bank_w[1] = bank_r[1] = 0x8000;
  }
}

// ---------------------------------------------------------------------
// Central register file
// ---------------------------------------------------------------------

// GPIO DDC ports: bit 0/1 = A (output level), 8/9 = Y (input level),
// 16/17 = EN (output enable); line 1 = clock, line 0 = data. Only the VGA
// connector (GPIO_VGA_DDC) has a monitor attached.
Bit32u bx_radeon9800_c::ddc_gpio_read(int port)
{
  Bit32u v = gpio[port] & ~0x00000300u;
  if (port == 0) {
    Bit8u st = ddc.read();   // bit 2 = clock line, bit 3 = data line
    if (st & 0x04) v |= 0x200;
    if (st & 0x08) v |= 0x100;
  } else {
    // open-drain lines float high unless driven low by the card
    if (!((gpio[port] & 0x20000) && !(gpio[port] & 0x2))) v |= 0x200;
    if (!((gpio[port] & 0x10000) && !(gpio[port] & 0x1))) v |= 0x100;
  }
  return v;
}

void bx_radeon9800_c::ddc_gpio_write(int port)
{
  if (port != 0)
    return;
  Bit32u g = gpio[0];
  ddc.write(!((g & 0x20000) && !(g & 0x2)), !((g & 0x10000) && !(g & 0x1)));
}

Bit32u bx_radeon9800_c::core_reg_read(Bit32u off, bool *hit)
{
  *hit = true;
  switch (off) {
    case R9800_MM_INDEX:
      return mm_index;
    case R9800_MM_DATA:
      if (mm_index & R9800_MM_INDEX_MM_APER) {
        Bit32u a = (mm_index & R9800_MM_INDEX_MM_ADDR) & vram_mask;
        return ReadHostDWordFromLittleEndian((Bit32u*)&BX_R9800_THIS s.memory[a]);
      }
      if ((mm_index & 0xfffc) == R9800_MM_DATA)
        return 0;
      return reg_read32(mm_index & 0xfffc);
    case R9800_BIOS_0_SCRATCH: case R9800_BIOS_0_SCRATCH + 4: case R9800_BIOS_0_SCRATCH + 8:
    case R9800_BIOS_0_SCRATCH + 12: case R9800_BIOS_0_SCRATCH + 16: case R9800_BIOS_0_SCRATCH + 20:
    case R9800_BIOS_0_SCRATCH + 24: case R9800_BIOS_7_SCRATCH:
      return bios_scratch[(off - R9800_BIOS_0_SCRATCH) >> 2];
    case R9800_BUS_CNTL:  return bus_cntl;
    case R9800_BUS_CNTL1: return bus_cntl1;
    case R9800_BUS_CNTL_MSTR: return bus_cntl_mstr;
    case R9800_GEN_INT_CNTL: return gen_int_cntl;
    case R9800_GEN_INT_STATUS:
      gen_int_fold_gui_idle();
      return gen_int_status;
    case R9800_GPIO_VGA_DDC: return ddc_gpio_read(0);
    case R9800_GPIO_DVI_DDC: return ddc_gpio_read(1);
    case R9800_GPIO_MONID:   return ddc_gpio_read(2);
    case R9800_GPIO_CRT2_DDC: return ddc_gpio_read(3);
    case R9800_I2C_CNTL_0:   return i2c_cntl[0] & ~0x00001000u; // GO self clears, DONE set
    case R9800_I2C_CNTL_1:   return i2c_cntl[1];
    case R9800_I2C_DATA:     return 0xff;
    case R9800_DVI_I2C_CNTL_0: return dvi_i2c[0] & ~0x00001000u;
    case R9800_DVI_I2C_CNTL_1: return dvi_i2c[1];
    case R9800_DVI_I2C_DATA: return 0xff;
    case R9800_VIPH_REG_ADDR: return viph_reg_addr;
    case R9800_VIPH_REG_DATA: return 0;
    case R9800_CONFIG_CNTL:
      return (config_cntl & ~R9800_CFG_ATI_REV_MASK) | ((Bit32u)asic_rev << R9800_CFG_ATI_REV_SHIFT);
    case R9800_CONFIG_XSTRAP:
      // ROM straps of the Medion 9800 XL image: MULTI_FUNC=2 (two functions,
      // AGP capability on function 0), AP_SIZE=1 (2 x 128 MB apertures),
      // HDCP_DISABLE=1
      return 0x20010020u | (vga_disabled ? 1u : 0u) | (is_agp ? 0u : (1u << 24));
    case R9800_CONFIG_BONDS: return 0;
    case R9800_RBBM_SOFT_RESET: return rbbm_soft_reset;
    case R9800_RBBM_STATUS_ALT:
    case R9800_RBBM_STATUS:
    case R9800_RBBM_STATUS_2: {
      Bit32u v = R9800_RBBM_FIFO_FREE;
      if (pm4_active())
        v = 0x20 | R9800_RBBM_GUI_ACTIVE | R9800_RBBM_CP_CMDSTRM_BUSY | R9800_RBBM_E2_BUSY |
            R9800_RBBM_RB3D_BUSY | R9800_RBBM_VAP_BUSY | R9800_RBBM_GA_BUSY;
      return v;
    }
    case R9800_RBBM_CNTL: return rbbm_cntl;
    case R9800_CONFIG_MEMSIZE: return config_memsize;
    case R9800_CONFIG_APER_0_BASE: return pci_bar[0].addr;
    case R9800_CONFIG_APER_1_BASE: return (sec != NULL) ? sec->bar0_addr() : (pci_bar[0].addr + (R9800_LFB_SIZE >> 1));
    case R9800_CONFIG_APER_SIZE: return R9800_LFB_SIZE;
    case R9800_CONFIG_REG_1_BASE: return pci_bar[2].addr;
    case R9800_CONFIG_REG_APER_SIZE: return R9800_MMIO_SIZE;
    case R9800_CONFIG_MEMSIZE_EMB: return 0;
    case R9800_TEST_DEBUG_CNTL: return test_debug_cntl;
    case R9800_TEST_DEBUG_MUX: return test_debug_mux;
    case R9800_HW_DEBUG: return hw_debug;
    case R9800_TEST_DEBUG_OUT: return 0;
    case R9800_HOST_PATH_CNTL: return host_path_cntl;
    case R9800_HDP_FB_LOCATION: return hdp_fb_location;
    case R9800_MEM_CNTL: return mem_cntl;
    case R9800_MEM_TIMING_CNTL: return mem_timing_cntl;
    case R9800_MC_FB_LOCATION: return mc_fb_location;
    case R9800_MC_AGP_LOCATION: return mc_agp_location;
    case R9800_MC_STATUS: return R9800_MC_IDLE | 0x0f;   // MEM_PWRUP_COMPLETE for all four channels
    case R9800_MEM_INIT_LAT_TIMER: return mem_init_lat_timer;
    case R9800_MEM_SDRAM_MODE_REG: return mem_sdram_mode_reg;
    case R9800_AGP_BASE_2: return agp_base_2;
    case R9800_MEM_IO_CNTL_A0: case R9800_MEM_IO_CNTL_A1:
    case R9800_MEM_IO_CNTL_B0: case R9800_MEM_IO_CNTL_B1:
      return mem_io_cntl[(off - R9800_MEM_IO_CNTL_A0) >> 2];
    case R9800_AGP_BASE: return agp_base;
    case R9800_AGP_CNTL: return agp_cntl;
    case R9800_MC_READ_CNTL_AB: return mc_read_cntl_ab;
    case R9800_MC_INIT_MISC_LAT_TIMER: return mc_init_misc_lat_timer;
    case R9800_MC_INIT_GFX_LAT_TIMER: return mc_init_gfx_lat_timer;
    case R9800_MC_DEBUG: return mc_debug;
    case R9800_MC_CHP_IO_OE_CNTL_AB: return mc_chp_io_oe_cntl_ab;
    case R9800_VIDEOMUX_CNTL: return videomux_cntl;
    case R9800_MDGPIO_MASK: case R9800_MDGPIO_A: case R9800_MDGPIO_EN: case R9800_MDGPIO_Y:
      return mdgpio[(off - R9800_MDGPIO_MASK) >> 2];
    case R9800_SEPROM_CNTL1: return seprom_cntl1;
    case R9800_SEPROM_CNTL2: return seprom_cntl2;
    case R9800_MPP_GP_CONFIG: return mpp_gp_config;
    case R9800_MPP_TB_CONFIG_ALT: return mpp_tb_config;
    case R9800_AIC_CNTL: return aic_cntl;
    case R9800_AIC_STAT: return 0;
    case R9800_AIC_PT_BASE: return aic_pt_base;
    case R9800_AIC_LO_ADDR: return aic_lo_addr;
    case R9800_AIC_HI_ADDR: return aic_hi_addr;
    case R9800_AIC_TLB_ADDR: return aic_tlb_addr;
    case R9800_AIC_TLB_DATA: return aic_tlb_data;
    case R9800_MC_IND_INDEX: return mc_ind_index;
    case R9800_MC_IND_DATA: return mc_ind_regs[mc_ind_index & R9800_MC_IND_ADDR_MASK];
    case R9800_SURFACE_CNTL: return surface_cntl;
    case R9800_SURFACE_ACCESS_FLAGS: return surface_access_flags;
    case R9800_SURFACE_ACCESS_CLR: return 0;
    case R9800_GUI_DEBUG0: case R9800_GUI_DEBUG0 + 4: case R9800_GUI_DEBUG0 + 8:
    case R9800_GUI_DEBUG0 + 12: case R9800_GUI_DEBUG0 + 16: case R9800_GUI_DEBUG5:
      return gui_debug[(off - R9800_GUI_DEBUG0) >> 2];
    case R9800_CAP_INT_CNTL: return cap_int_cntl;
    case R9800_CAP_INT_STATUS: return cap_int_status;
    case R9800_FCP_CNTL: return fcp_cntl;
    case R9800_DISP_MISC_CNTL: return disp_misc_cntl;
    case R9800_DAC_MACRO_CNTL: return dac_macro_cntl;
    case R9800_DISP_PWR_MAN: return disp_pwr_man;
    case R9800_DISP_TEST_DEBUG_CNTL: return disp_test_debug_cntl;
    case R9800_DISP_HW_DEBUG: return disp_hw_debug;
    case R9800_DISP_MERGE_CNTL: return disp_merge_cntl;
    case R9800_DISP_OUTPUT_CNTL: return disp_output_cntl;
    case R9800_DISP2_MERGE_CNTL: return disp2_merge_cntl;
    case R9800_DISP_TV_OUT_CNTL: return disp_tv_out_cntl;
    case R9800_DISP_LIN_TRANS_GRPH_A: case R9800_DISP_LIN_TRANS_GRPH_A + 4:
    case R9800_DISP_LIN_TRANS_GRPH_A + 8: case R9800_DISP_LIN_TRANS_GRPH_A + 12:
    case R9800_DISP_LIN_TRANS_GRPH_A + 16: case R9800_DISP_LIN_TRANS_GRPH_A + 20:
    case R9800_DISP_LIN_TRANS_GRPH_F: case R9800_DISP_LIN_TRANS_GRPH_G: case R9800_DISP_LIN_TRANS_GRPH_H:
      return disp_lin_trans[(off - R9800_DISP_LIN_TRANS_GRPH_A) >> 2];
    case R9800_TV_DAC_CNTL:
      // no load on the TV DAC: RDACDET/GDACDET/BDACDET read 0
      return tv_dac_cntl & ~R9800_TV_DAC_DACDET_MASK;
    case R9800_OV1_Y_X_START: case R9800_OV1_Y_X_END: case R9800_OV1_PIPELINE_CNTL:
      return ov1_regs[(off - R9800_OV1_Y_X_START) >> 2];
    case R9800_VIPH_TIMEOUT_STAT: return 0;
    default:
      break;
  }
  if ((off >= R9800_SURFACE0_LOWER_BOUND) && (off <= R9800_SURFACE7_INFO)) {
    Bit32u n = (off - 0x0b00) >> 4;
    switch (off & 0x0c) {
      case 0x04: return surf_lower[n];
      case 0x08: return surf_upper[n];
      case 0x0c: return surf_info[n];
      default: break;
    }
  }
  if ((off >= R9800_TV_BLOCK_BASE) && (off <= R9800_TV_BLOCK_END) && (off != R9800_TV_DAC_CNTL))
    return tv_regs[(off - R9800_TV_BLOCK_BASE) >> 2];
  if ((off >= R9800_CAP0_BLOCK_BASE) && (off <= R9800_CAP0_BLOCK_END))
    return cap0_regs[(off - R9800_CAP0_BLOCK_BASE) >> 2];
  if ((off >= R9800_VIPH_CH0_DATA) && (off < R9800_VIPH_TIMEOUT_STAT))
    return viph_regs[(off - R9800_VIPH_CH0_DATA) >> 2];
  if ((off >= R9800_CONFIG_MIRROR_BASE) && (off <= R9800_CONFIG_MIRROR_END))
    return pci_read_handler((Bit8u)(off - R9800_CONFIG_MIRROR_BASE), 4);
  *hit = false;
  return 0;
}

// =====================================================================
// Register access trace (off unless 'radeon9800: trace=N' is set)
// =====================================================================

static const struct { Bit32u off; const char *name; } radeon9800_trace_names[] = {
  { R9800_MM_INDEX, "MM_INDEX" }, { R9800_MM_DATA, "MM_DATA" },
  { R9800_CLOCK_CNTL_INDEX, "CLOCK_CNTL_INDEX" }, { R9800_CLOCK_CNTL_DATA, "CLOCK_CNTL_DATA" },
  { R9800_BUS_CNTL, "BUS_CNTL" },
  { R9800_GEN_INT_CNTL, "GEN_INT_CNTL" }, { R9800_GEN_INT_STATUS, "GEN_INT_STATUS" },
  { R9800_CRTC_GEN_CNTL, "CRTC_GEN_CNTL" }, { R9800_CRTC_EXT_CNTL, "CRTC_EXT_CNTL" },
  { R9800_DAC_CNTL, "DAC_CNTL" }, { R9800_CRTC_STATUS, "CRTC_STATUS" },
  { R9800_CONFIG_CNTL, "CONFIG_CNTL" }, { R9800_RBBM_SOFT_RESET, "RBBM_SOFT_RESET" },
  { R9800_MC_STATUS, "MC_STATUS" },
  { R9800_CRTC_H_TOTAL_DISP, "CRTC_H_TOTAL_DISP" }, { R9800_CRTC_V_TOTAL_DISP, "CRTC_V_TOTAL_DISP" },
  { R9800_CRTC_VLINE_CRNT_VLINE, "CRTC_VLINE_CRNT_VLINE" },
  { R9800_CRTC_OFFSET, "CRTC_OFFSET" }, { R9800_CRTC_OFFSET_CNTL, "CRTC_OFFSET_CNTL" },
  { R9800_CRTC_PITCH, "CRTC_PITCH" }, { R9800_DISPLAY_BASE_ADDR, "DISPLAY_BASE_ADDR" },
  { R9800_FP_GEN_CNTL, "FP_GEN_CNTL" }, { R9800_FP2_GEN_CNTL, "FP2_GEN_CNTL" },
  { R9800_CRTC2_GEN_CNTL, "CRTC2_GEN_CNTL" },
  { R9800_CP_RB_RPTR, "CP_RB_RPTR" }, { R9800_CP_RB_WPTR, "CP_RB_WPTR" },
  { R9800_CP_STAT, "CP_STAT" }, { R9800_CP_CSQ_STAT, "CP_CSQ_STAT" },
  { R9800_TV_DAC_CNTL, "TV_DAC_CNTL" }, { R9800_SURFACE_CNTL, "SURFACE_CNTL" },
  { R9800_DISP_OUTPUT_CNTL, "DISP_OUTPUT_CNTL" }, { R9800_RBBM_STATUS, "RBBM_STATUS" },
  { R9800_WAIT_UNTIL, "WAIT_UNTIL" },
  { 0xffffffff, NULL }
};

static const char *radeon9800_trace_name(Bit32u off)
{
  for (int i = 0; radeon9800_trace_names[i].name != NULL; i++)
    if (radeon9800_trace_names[i].off == off)
      return radeon9800_trace_names[i].name;
  return "";
}

// The indexed apertures carry the register that is really being touched:
// report the selector alongside the access so a PLL or MM_DATA poll is
// identifiable.
Bit32u bx_radeon9800_c::trace_index_of(Bit32u off)
{
  if (off == R9800_CLOCK_CNTL_DATA)
    return clock_cntl_index & 0x3f;
  if (off == R9800_MM_DATA)
    return mm_index & 0xfffc;
  return 0xffffffff;
}

// Emit a pending run of identical reads
void bx_radeon9800_c::trace_read_flush(void)
{
  if (!trace_rd_valid || (trace_rd_run == 0))
    return;
  if (trace_mask & 2) {
    if (trace_rd_idx != 0xffffffff) {
      BX_INFO(("trace: R %04x[%02x] %-20s = %08x  x%u", trace_rd_off, trace_rd_idx,
               radeon9800_trace_name(trace_rd_off), trace_rd_val, trace_rd_run));
    } else {
      BX_INFO(("trace: R %04x      %-20s = %08x  x%u", trace_rd_off,
               radeon9800_trace_name(trace_rd_off), trace_rd_val, trace_rd_run));
    }
  }
  trace_rd_run = 0;
  trace_rd_valid = false;
}

// Engine state and the register writes that led up to a stall
void bx_radeon9800_c::trace_state_dump(void)
{
  BX_INFO(("trace:   crtc_gen=%08x crtc2_gen=%08x crtc_ext=%08x dac=%08x disp_out=%08x",
           crtc_gen_cntl, crtc_ext_cntl, crtc2_gen_cntl, dac_cntl, disp_output_cntl));
  BX_INFO(("trace:   disp_ext=%d disp_crtc=%d disp_output=%d timing_valid=%d ppll_pend=%d p2pll_pend=%d",
           (int)disp_ext, disp_crtc, disp_output, (int)timing_valid,
           (int)ppll_update_pending, (int)p2pll_update_pending));
  BX_INFO(("trace:   int_cntl=%08x int_status=%08x irqs_delivered=%u irq_line=%d",
           gen_int_cntl, gen_int_status, trace_irq_count, (int)trace_irq_level));
  BX_INFO(("trace:   cp_rptr=%08x cp_wptr=%08x rb_cntl=%08x rb_base=%08x scratch_addr=%08x umsk=%02x",
           cp_rb_rptr, cp_rb_wptr, cp_rb_cntl, cp_rb_base, scratch_addr, scratch_umsk));
  BX_INFO(("trace:   preceding register writes (oldest first):"));
  Bit32u n = (trace_wr_seen < (Bit32u)R9800_TRACE_WRITES) ? trace_wr_seen : (Bit32u)R9800_TRACE_WRITES;
  Bit32u first = (trace_wr_pos + R9800_TRACE_WRITES - n) % R9800_TRACE_WRITES;
  for (Bit32u i = 0; i < n; i++) {
    Bit32u k = (first + i) % R9800_TRACE_WRITES;
    if (trace_wr_idx[k] != 0xffffffff) {
      BX_INFO(("trace:     W %04x[%02x] %-20s = %08x", trace_wr_off[k], trace_wr_idx[k],
               radeon9800_trace_name(trace_wr_off[k]), trace_wr_val[k]));
    } else {
      BX_INFO(("trace:     W %04x      %-20s = %08x", trace_wr_off[k],
               radeon9800_trace_name(trace_wr_off[k]), trace_wr_val[k]));
    }
  }
}

// One register read over and over, whatever it returns
void bx_radeon9800_c::trace_poll_report(void)
{
  if (trace_polls >= 6)
    return;
  trace_polls++;
  if (trace_run_varies) {
    BX_INFO(("trace: guest is spinning on register %04x %s (%u reads, value varies, first %08x last %08x)",
             trace_run_off, radeon9800_trace_name(trace_run_off),
             trace_run_len, trace_run_first, trace_run_last));
  } else {
    BX_INFO(("trace: guest is spinning on register %04x %s = %08x (%u reads)",
             trace_run_off, radeon9800_trace_name(trace_run_off),
             trace_run_last, trace_run_len));
  }
  trace_state_dump();
}

// A wait loop that touches several registers never trips the single
// register detector, so a long run of accesses confined to a handful of
// registers is reported as a stall in its own right.
void bx_radeon9800_c::trace_block_report(void)
{
  if (trace_polls >= 6)
    return;
  trace_polls++;
  BX_INFO(("trace: guest appears stuck: %u register accesses touching only %d registers",
           trace_blk_total, trace_blk_n));
  for (int i = 0; i < trace_blk_n; i++) {
    BX_INFO(("trace:   %04x %-20s  x%u", trace_blk_off[i],
             radeon9800_trace_name(trace_blk_off[i]), trace_blk_cnt[i]));
  }
  trace_state_dump();
}

void bx_radeon9800_c::trace_block_note(Bit32u off)
{
  int i;
  trace_blk_total++;
  for (i = 0; i < trace_blk_n; i++) {
    if (trace_blk_off[i] == off)
      break;
  }
  if (i < trace_blk_n) {
    trace_blk_cnt[i]++;
  } else if (trace_blk_n < R9800_TRACE_BLOCK_REGS) {
    trace_blk_off[trace_blk_n] = off;
    trace_blk_cnt[trace_blk_n] = 1;
    trace_blk_n++;
  } else {
    // more registers than a wait loop would touch: this is real work
    trace_blk_n = 0;
    trace_blk_total = 0;
    return;
  }
  if (trace_blk_total >= (Bit32u)R9800_TRACE_BLOCK) {
    if (trace_mask & 4)
      trace_block_report();
    trace_blk_n = 0;
    trace_blk_total = 0;
  }
}

// Report which memory-controller window an address the engine will write
// through lands in. An address matching no window still "succeeds" as a
// direct bus-master access, so a fence programmed outside every window is
// written into nothing and the failure is otherwise invisible.
void bx_radeon9800_c::trace_mc_probe(const char *what, Bit32u mc)
{
  Bit32u kind = 0, addr = 0, off = 0;
  const char *win;
  Bit32u agp_start = (mc_agp_location & 0xffff) << 16;
  Bit32u agp_top = (mc_agp_location & 0xffff0000) | 0xffff;
  bool aic_on = (aic_cntl & R9800_AIC_TRANSLATE_EN) != 0;

  if (!mc_resolve(mc, &kind, &addr)) {
    BX_INFO(("trace: %s mc=%08x does not resolve at all", what, mc));
    return;
  }
  if (mc_is_vram(mc, &off))
    win = "local VRAM";
  else if ((agp_start <= agp_top) && (mc >= agp_start) && (mc <= agp_top))
    win = "AGP window";
  else if (aic_on && (mc >= aic_lo_addr) && (mc <= aic_hi_addr))
    win = "PCI GART";
  else
    win = "NO WINDOW MATCHED, written into nothing";
  BX_INFO(("trace: %s mc=%08x -> %s, %s addr %08x", what, mc, win,
           kind ? "bus" : "vram", addr));
  BX_INFO(("trace:   fb_loc=%08x agp_loc=%08x agp_base=%08x aic %08x..%08x en=%d",
           mc_fb_location, mc_agp_location, agp_base, aic_lo_addr, aic_hi_addr, (int)aic_on));
}

// Periodic state dump. A driver stuck on a completion fence in memory
// never touches a register while it spins, so no access-based detector can
// see it. Watching the engine and the fence values over time can.
void bx_radeon9800_c::trace_heartbeat(void)
{
  BX_INFO(("trace: hb disp_ext=%d crtc=%d crtc_gen=%08x int_cntl=%08x int_status=%08x irqs=%u",
           (int)disp_ext, disp_crtc, crtc_gen_cntl, gen_int_cntl, gen_int_status, trace_irq_count));
  BX_INFO(("trace: hb cp rb_base=%08x rb_cntl=%08x rptr=%08x wptr=%08x csq_cntl=%08x exec=%d batch=%d",
           cp_rb_base, cp_rb_cntl, cp_rb_rptr, cp_rb_wptr, cp_csq_cntl,
           (int)cp_executing, (int)cp_batch_pending));
  BX_INFO(("trace: hb fifo rd=%u wr=%u  scratch addr=%08x umsk=%02x  fences %08x %08x %08x %08x %08x %08x",
           cp_fifo_rd, cp_fifo_wr, scratch_addr, scratch_umsk,
           gui_scratch[0], gui_scratch[1], gui_scratch[2],
           gui_scratch[3], gui_scratch[4], gui_scratch[5]));
  // What the driver actually sees when it polls memory, as opposed to the
  // register copies above: the retired read pointer and its mailbox, and
  // the mirrored fences.
  {
    Bit32u mail = 0xffffffff, f[6];
    if (cp_rb_rptr_addr)
      gpu_read32(cp_rb_rptr_addr & ~3u, &mail);
    for (int i = 0; i < 6; i++) {
      f[i] = 0xffffffff;
      if (scratch_addr)
        gpu_read32((scratch_addr & ~3u) + (Bit32u)i * 4u, &f[i]);
    }
    BX_INFO(("trace: hb retire=%08x shadow=%08x rptr_addr=%08x mailbox reads %08x",
             cp_retire_rptr, cp_shadow_last, cp_rb_rptr_addr, mail));
    BX_INFO(("trace: hb fences in memory %08x %08x %08x %08x %08x %08x",
             f[0], f[1], f[2], f[3], f[4], f[5]));
  }
}

void bx_radeon9800_c::trace_reg_read(Bit32u off, Bit32u val)
{
  Bit32u idx = trace_index_of(off);

  trace_block_note(off);

  // A wait loop reads one register over and over. The value it returns may
  // well change each time (a counter, a status register), so the spin is
  // tracked per register rather than per result.
  if ((off == trace_run_off) && (idx == trace_run_idx)) {
    trace_run_len++;
    if (val != trace_run_last) {
      trace_run_last = val;
      trace_run_varies = true;
    }
    if ((trace_mask & 4) && (trace_run_len == R9800_TRACE_POLL_RUN))
      trace_poll_report();
  } else {
    trace_run_off = off;
    trace_run_idx = idx;
    trace_run_first = val;
    trace_run_last = val;
    trace_run_varies = false;
    trace_run_len = 1;
  }

  if ((off == trace_rd_off) && (val == trace_rd_val) && (idx == trace_rd_idx) && trace_rd_valid) {
    trace_rd_run++;
    return;
  }
  trace_read_flush();
  trace_rd_off = off;
  trace_rd_val = val;
  trace_rd_idx = idx;
  trace_rd_run = 1;
  trace_rd_valid = true;
}

void bx_radeon9800_c::trace_reg_write(Bit32u off, Bit32u val, Bit32u mask)
{
  Bit32u idx = trace_index_of(off);
  trace_block_note(off);
  trace_read_flush();
  trace_wr_off[trace_wr_pos] = off;
  trace_wr_val[trace_wr_pos] = val;
  trace_wr_idx[trace_wr_pos] = idx;
  trace_wr_pos = (trace_wr_pos + 1) % R9800_TRACE_WRITES;
  trace_wr_seen++;
  if (trace_mask & 1) {
    if (idx != 0xffffffff) {
      BX_INFO(("trace: W %04x[%02x] %-20s = %08x mask %08x", off, idx,
               radeon9800_trace_name(off), val, mask));
    } else {
      BX_INFO(("trace: W %04x      %-20s = %08x mask %08x", off,
               radeon9800_trace_name(off), val, mask));
    }
  }
}

Bit32u bx_radeon9800_c::reg_read32(Bit32u off)
{
  Bit32u v;
  bool hit;

  off &= 0xfffc;
  v = core_reg_read(off, &hit);
  if (!hit &&
      !display_reg_read(off, &v) &&
      !ov0_reg_read(off, &v) &&
      !subpic_reg_read(off, &v) &&
      !r2d_reg_read(off, &v) &&
      !pm4_reg_read(off, &v) &&
      !r3d_reg_read(off, &v))
    v = 0;
  if (trace_mask && !on_cp_thread())
    trace_reg_read(off, v);
  return v;
}

bool bx_radeon9800_c::core_reg_write(Bit32u off, Bit32u val, Bit32u mask)
{
#define MERGE(field) ((field) = ((field) & ~mask) | val)
  switch (off) {
    case R9800_MM_INDEX:
      MERGE(mm_index);
      return true;
    case R9800_MM_DATA:
      if (mm_index & R9800_MM_INDEX_MM_APER) {
        Bit32u a = (mm_index & R9800_MM_INDEX_MM_ADDR) & vram_mask;
        Bit32u *p = (Bit32u*)&BX_R9800_THIS s.memory[a];
        Bit32u cur = ReadHostDWordFromLittleEndian(p);
        WriteHostDWordToLittleEndian(p, (cur & ~mask) | val);
        vram_dirty(a, 4);
        return true;
      }
      if ((mm_index & 0xfffc) == R9800_MM_DATA)
        return true;
      reg_write(mm_index & 0xfffc, val, mask);
      return true;
    case R9800_BIOS_0_SCRATCH: case R9800_BIOS_0_SCRATCH + 4: case R9800_BIOS_0_SCRATCH + 8:
    case R9800_BIOS_0_SCRATCH + 12: case R9800_BIOS_0_SCRATCH + 16: case R9800_BIOS_0_SCRATCH + 20:
    case R9800_BIOS_0_SCRATCH + 24: case R9800_BIOS_7_SCRATCH:
      MERGE(bios_scratch[(off - R9800_BIOS_0_SCRATCH) >> 2]);
      return true;
    case R9800_BUS_CNTL:
      MERGE(bus_cntl);
      bus_cntl &= ~R9800_BUS_CNTL_WO_MASK;
      if (cp_thread_started && !on_cp_thread())
        pm4_kick();
      return true;
    case R9800_BUS_CNTL1: MERGE(bus_cntl1); return true;
    case R9800_BUS_CNTL_MSTR: MERGE(bus_cntl_mstr); return true;
    case R9800_GEN_INT_CNTL:
      MERGE(gen_int_cntl);
      gen_int_update();
      return true;
    case R9800_GEN_INT_STATUS:
      // acknowledge by writing 1; SW_INT_FIRE [26] raises SW_INT [25]
      gen_int_status &= ~(val & R9800_INT_ACK_MASK);
      if (val & R9800_INT_SW_FIRE)
        gen_int_status |= R9800_INT_SW;
      gen_int_update();
      return true;
    case R9800_GPIO_VGA_DDC: MERGE(gpio[0]); ddc_gpio_write(0); return true;
    case R9800_GPIO_DVI_DDC: MERGE(gpio[1]); return true;
    case R9800_GPIO_MONID:   MERGE(gpio[2]); return true;
    case R9800_GPIO_CRT2_DDC: MERGE(gpio[3]); return true;
    case R9800_I2C_CNTL_0:
      MERGE(i2c_cntl[0]);
      // no device answers on the hardware I2C engine: DONE and NACK
      if (i2c_cntl[0] & 0x1000) i2c_cntl[0] = (i2c_cntl[0] & ~0x1000u) | 0x3;
      return true;
    case R9800_I2C_CNTL_1: MERGE(i2c_cntl[1]); return true;
    case R9800_I2C_DATA: MERGE(i2c_cntl[2]); return true;
    case R9800_DVI_I2C_CNTL_0:
      MERGE(dvi_i2c[0]);
      if (dvi_i2c[0] & 0x1000) dvi_i2c[0] = (dvi_i2c[0] & ~0x1000u) | 0x3;
      return true;
    case R9800_DVI_I2C_CNTL_1: MERGE(dvi_i2c[1]); return true;
    case R9800_DVI_I2C_DATA: MERGE(dvi_i2c[2]); return true;
    case R9800_VIPH_REG_ADDR: MERGE(viph_reg_addr); return true;
    case R9800_VIPH_REG_DATA: MERGE(viph_reg_data); return true;
    case R9800_CONFIG_CNTL:
      MERGE(config_cntl);
      config_cntl &= ~R9800_CFG_ATI_REV_MASK;
      return true;
    case R9800_CONFIG_XSTRAP:
    case R9800_CONFIG_BONDS:
    case R9800_RBBM_STATUS:
    case R9800_RBBM_STATUS_2:
    case R9800_RBBM_STATUS_ALT:
      return true;
    case R9800_RBBM_SOFT_RESET: {
      Bit32u old = rbbm_soft_reset;
      MERGE(rbbm_soft_reset);
      if (!(old & (R9800_SOFT_RESET_CP | R9800_SOFT_RESET_SE | R9800_SOFT_RESET_E2)) &&
          (rbbm_soft_reset & (R9800_SOFT_RESET_CP | R9800_SOFT_RESET_SE | R9800_SOFT_RESET_E2))) {
        pm4_gui_reset();
        raster_abandon();
        hostdata_active = false;
        hostdata_ndw = 0;
        draw.active = false;
      }
      return true;
    }
    case R9800_RBBM_CNTL: MERGE(rbbm_cntl); return true;
    case R9800_CONFIG_MEMSIZE:
      MERGE(config_memsize);
      config_memsize &= 0x1fffffff;
      return true;
    case R9800_TEST_DEBUG_CNTL: MERGE(test_debug_cntl); return true;
    case R9800_TEST_DEBUG_MUX: MERGE(test_debug_mux); return true;
    case R9800_HW_DEBUG: MERGE(hw_debug); return true;
    case R9800_TEST_DEBUG_OUT: return true;
    case R9800_HOST_PATH_CNTL:
      MERGE(host_path_cntl);
      host_path_cntl &= ~(R9800_HDP_SOFT_RESET | R9800_HDP_READ_BUFFER_INV);
      return true;
    case R9800_HDP_FB_LOCATION: MERGE(hdp_fb_location); return true;
    case R9800_MEM_CNTL: MERGE(mem_cntl); return true;
    case R9800_MEM_TIMING_CNTL: MERGE(mem_timing_cntl); return true;
    case R9800_MC_FB_LOCATION:
      if (!on_cp_thread()) pm4_drain_wait();
      MERGE(mc_fb_location);
      needs_update_mode = true;
      needs_update_dispentire = true;
      return true;
    case R9800_MC_AGP_LOCATION:
      if (!on_cp_thread()) pm4_drain_wait();
      MERGE(mc_agp_location);
      return true;
    case R9800_MC_STATUS: return true;
    case R9800_MEM_INIT_LAT_TIMER: MERGE(mem_init_lat_timer); return true;
    case R9800_MEM_SDRAM_MODE_REG: MERGE(mem_sdram_mode_reg); return true;
    case R9800_AGP_BASE_2: MERGE(agp_base_2); return true;
    case R9800_MEM_IO_CNTL_A0: case R9800_MEM_IO_CNTL_A1:
    case R9800_MEM_IO_CNTL_B0: case R9800_MEM_IO_CNTL_B1:
      MERGE(mem_io_cntl[(off - R9800_MEM_IO_CNTL_A0) >> 2]);
      return true;
    case R9800_AGP_BASE:
      if (!on_cp_thread()) pm4_drain_wait();
      MERGE(agp_base);
      agp_base &= 0xfffff000;
      return true;
    case R9800_AGP_CNTL: MERGE(agp_cntl); return true;
    case R9800_MC_READ_CNTL_AB: MERGE(mc_read_cntl_ab); return true;
    case R9800_MC_INIT_MISC_LAT_TIMER: MERGE(mc_init_misc_lat_timer); return true;
    case R9800_MC_INIT_GFX_LAT_TIMER: MERGE(mc_init_gfx_lat_timer); return true;
    case R9800_MC_DEBUG: MERGE(mc_debug); return true;
    case R9800_MC_CHP_IO_OE_CNTL_AB: MERGE(mc_chp_io_oe_cntl_ab); return true;
    case R9800_VIDEOMUX_CNTL: MERGE(videomux_cntl); return true;
    case R9800_MDGPIO_MASK: case R9800_MDGPIO_A: case R9800_MDGPIO_EN: case R9800_MDGPIO_Y:
      MERGE(mdgpio[(off - R9800_MDGPIO_MASK) >> 2]);
      return true;
    case R9800_SEPROM_CNTL1: MERGE(seprom_cntl1); return true;
    case R9800_SEPROM_CNTL2: MERGE(seprom_cntl2); return true;
    case R9800_MPP_GP_CONFIG: MERGE(mpp_gp_config); return true;
    case R9800_MPP_TB_CONFIG_ALT: MERGE(mpp_tb_config); return true;
    case R9800_AIC_CNTL:
      if (!on_cp_thread()) pm4_drain_wait();
      MERGE(aic_cntl);
      return true;
    case R9800_AIC_STAT: return true;
    case R9800_AIC_PT_BASE:
      if (!on_cp_thread()) pm4_drain_wait();
      MERGE(aic_pt_base);
      return true;
    case R9800_AIC_LO_ADDR:
      if (!on_cp_thread()) pm4_drain_wait();
      MERGE(aic_lo_addr);
      return true;
    case R9800_AIC_HI_ADDR:
      if (!on_cp_thread()) pm4_drain_wait();
      MERGE(aic_hi_addr);
      return true;
    case R9800_AIC_TLB_ADDR: MERGE(aic_tlb_addr); return true;
    case R9800_AIC_TLB_DATA: MERGE(aic_tlb_data); return true;
    case R9800_MC_IND_INDEX: MERGE(mc_ind_index); return true;
    case R9800_MC_IND_DATA:
      if (mc_ind_index & R9800_MC_IND_WR_EN)
        MERGE(mc_ind_regs[mc_ind_index & R9800_MC_IND_ADDR_MASK]);
      return true;
    case R9800_SURFACE_CNTL: MERGE(surface_cntl); return true;
    case R9800_SURFACE_ACCESS_FLAGS: return true;
    case R9800_SURFACE_ACCESS_CLR: surface_access_flags = 0; return true;
    case R9800_GUI_DEBUG0: case R9800_GUI_DEBUG0 + 4: case R9800_GUI_DEBUG0 + 8:
    case R9800_GUI_DEBUG0 + 12: case R9800_GUI_DEBUG0 + 16: case R9800_GUI_DEBUG5:
      MERGE(gui_debug[(off - R9800_GUI_DEBUG0) >> 2]);
      return true;
    case R9800_CAP_INT_CNTL: MERGE(cap_int_cntl); return true;
    case R9800_CAP_INT_STATUS: cap_int_status &= ~val; return true;
    case R9800_FCP_CNTL: MERGE(fcp_cntl); return true;
    case R9800_DISP_MISC_CNTL: MERGE(disp_misc_cntl); return true;
    case R9800_DAC_MACRO_CNTL: MERGE(dac_macro_cntl); return true;
    case R9800_DISP_PWR_MAN: MERGE(disp_pwr_man); return true;
    case R9800_DISP_TEST_DEBUG_CNTL: MERGE(disp_test_debug_cntl); return true;
    case R9800_DISP_HW_DEBUG: MERGE(disp_hw_debug); return true;
    case R9800_DISP_MERGE_CNTL: MERGE(disp_merge_cntl); return true;
    case R9800_DISP_OUTPUT_CNTL: {
      Bit32u old = disp_output_cntl;
      MERGE(disp_output_cntl);
      if ((old ^ disp_output_cntl) & 0x0f) {
        scanout_refresh();
        needs_update_mode = true;
        needs_update_dispentire = true;
        timing_update();
      }
      return true;
    }
    case R9800_DISP2_MERGE_CNTL: MERGE(disp2_merge_cntl); return true;
    case R9800_DISP_TV_OUT_CNTL: MERGE(disp_tv_out_cntl); return true;
    case R9800_DISP_LIN_TRANS_GRPH_A: case R9800_DISP_LIN_TRANS_GRPH_A + 4:
    case R9800_DISP_LIN_TRANS_GRPH_A + 8: case R9800_DISP_LIN_TRANS_GRPH_A + 12:
    case R9800_DISP_LIN_TRANS_GRPH_A + 16: case R9800_DISP_LIN_TRANS_GRPH_A + 20:
    case R9800_DISP_LIN_TRANS_GRPH_F: case R9800_DISP_LIN_TRANS_GRPH_G: case R9800_DISP_LIN_TRANS_GRPH_H:
      MERGE(disp_lin_trans[(off - R9800_DISP_LIN_TRANS_GRPH_A) >> 2]);
      return true;
    case R9800_TV_DAC_CNTL: {
      Bit32u old = tv_dac_cntl;
      MERGE(tv_dac_cntl);
      if ((old ^ tv_dac_cntl) & (R9800_TV_DAC_RDACPD | R9800_TV_DAC_GDACPD | R9800_TV_DAC_BDACPD | R9800_TV_DAC_NBLANK)) {
        scanout_refresh();
        needs_update_mode = true;
        needs_update_dispentire = true;
      }
      return true;
    }
    case R9800_OV1_Y_X_START: case R9800_OV1_Y_X_END: case R9800_OV1_PIPELINE_CNTL:
      MERGE(ov1_regs[(off - R9800_OV1_Y_X_START) >> 2]);
      return true;
    case R9800_VIPH_TIMEOUT_STAT: return true;
    case R9800_CONFIG_APER_0_BASE:
    case R9800_CONFIG_APER_1_BASE:
    case R9800_CONFIG_APER_SIZE:
    case R9800_CONFIG_REG_1_BASE:
    case R9800_CONFIG_REG_APER_SIZE:
    case R9800_CONFIG_MEMSIZE_EMB:
      return true;
    default:
      break;
  }
#undef MERGE

  if ((off >= R9800_SURFACE0_LOWER_BOUND) && (off <= R9800_SURFACE7_INFO) && ((off & 0x0c) != 0)) {
    Bit32u n = (off - 0x0b00) >> 4;
    Bit32u *r;
    switch (off & 0x0c) {
      case 0x04: r = &surf_lower[n]; break;
      case 0x08: r = &surf_upper[n]; break;
      default:   r = &surf_info[n];  break;
    }
    *r = (*r & ~mask) | val;
    surf_xlate_on = false;
    for (n = 0; n < 8; n++)
      if ((surf_info[n] & R9800_SURF_TILE_MASK) && (surf_info[n] & R9800_SURF_PITCH_MASK))
        surf_xlate_on = true;
    return true;
  }
  if ((off >= R9800_TV_BLOCK_BASE) && (off <= R9800_TV_BLOCK_END)) {
    Bit32u *r = &tv_regs[(off - R9800_TV_BLOCK_BASE) >> 2];
    *r = (*r & ~mask) | val;
    return true;
  }
  if ((off >= R9800_CAP0_BLOCK_BASE) && (off <= R9800_CAP0_BLOCK_END)) {
    Bit32u *r = &cap0_regs[(off - R9800_CAP0_BLOCK_BASE) >> 2];
    *r = (*r & ~mask) | val;
    return true;
  }
  if ((off >= R9800_VIPH_CH0_DATA) && (off < R9800_VIPH_TIMEOUT_STAT)) {
    Bit32u *r = &viph_regs[(off - R9800_VIPH_CH0_DATA) >> 2];
    *r = (*r & ~mask) | val;
    return true;
  }
  if ((off >= R9800_CONFIG_MIRROR_BASE) && (off <= R9800_CONFIG_MIRROR_END)) {
    pci_write_handler_common((Bit8u)(off - R9800_CONFIG_MIRROR_BASE), val, 4);
    return true;
  }
  return false;
}

void bx_radeon9800_c::reg_write(Bit32u off, Bit32u val, Bit32u mask)
{
  off &= 0xfffc;
  val &= mask;

  if (trace_mask && !on_cp_thread())
    trace_reg_write(off, val, mask);

  if (core_reg_write(off, val, mask))
    return;

  // Direct CPU writes into the GUI / 3D engine blocks apply synchronously:
  // drain pending ring/CP work first. WAIT_UNTIL queues behind the engine.
  if ((off >= 0x1400) && (off < 0x5000) && !on_cp_thread() && cp_thread_started) {
    if ((off == R9800_WAIT_UNTIL) && (mask == 0xffffffff) && pm4_enqueue_write(off, val))
      return;
    if ((off >= R9800_CP_CSQ_APER_PRIMARY) && (off <= R9800_CP_CSQ_APER_INDIRECT_END)) {
      // PIO command stream aperture: dwords enter the CP queue
      if (pm4_enqueue_pio(val))
        return;
    }
    pm4_drain_wait();
  }

  if (display_reg_write(off, val, mask)) return;
  if (ov0_reg_write(off, val, mask)) return;
  if (subpic_reg_write(off, val, mask)) return;
  if (r2d_reg_write(off, val, mask)) return;
  if (pm4_reg_write(off, val, mask)) return;
  if (r3d_reg_write(off, val, mask)) return;
}

// =====================================================================
// Display block: PLL register file, CRTC1/CRTC2, DAC / palettes, cursor
// =====================================================================

static const int radeon9800_post_div[8] = { 1, 2, 4, 8, 3, 16, 6, 12 };

void bx_radeon9800_c::ppll_commit(void)
{
  for (int i = 0; i < 5; i++)
    ppll_work[i] = pll_regs[R9800_PLL_PPLL_REF_DIV + i];
}

double bx_radeon9800_c::ppll_vco_hz(int sel)
{
  Bit32u div = ppll_work[1 + (sel & 3)];
  // R300 programs the reference divider through REF_DIV_ACC [27:18]
  Bit32u m = (ppll_work[0] & R9800_PPLL_REF_DIV_ACC_MASK) >> R9800_PPLL_REF_DIV_ACC_SHIFT;
  if (m == 0) m = ppll_work[0] & R9800_PPLL_REF_DIV_MASK;
  Bit32u n = div & R9800_PPLL_FB_DIV_MASK;
  if (m < 2) m = 2;
  if (n < 4) n = 4;
  return ref_freq_hz * (double)n / (double)m;
}

double bx_radeon9800_c::p2pll_hz(void)
{
  Bit32u m = p2pll_work[0] & R9800_PPLL_REF_DIV_MASK;
  Bit32u n = p2pll_work[1] & R9800_PPLL_FB_DIV_MASK;
  if (m < 2) m = 2;
  if (n < 4) n = 4;
  return ref_freq_hz * (double)n / (double)m / radeon9800_post_div[(p2pll_work[1] >> R9800_PPLL_POST_DIV_SHIFT) & 7];
}

double bx_radeon9800_c::dot_clock_hz(void)
{
  int sel;
  if (disp_crtc == 1) {
    if ((pll_regs[R9800_PLL_PIXCLKS_CNTL] & R9800_PIX2CLK_SRC_SEL_MASK) != R9800_PIX2CLK_SRC_P2PLL)
      return ref_freq_hz;
    return p2pll_hz();
  }
  if ((pll_regs[R9800_PLL_VCLK_ECP_CNTL] & R9800_VCLK_SRC_SEL_MASK) != R9800_VCLK_SRC_PPLL)
    return ref_freq_hz;
  if (crtc_extended(0))
    sel = (clock_cntl_index >> R9800_PLL_DIV_SEL_SHIFT) & 3;
  else
    sel = BX_R9800_THIS s.misc_output.clock_select & 3;
  Bit32u div = ppll_work[1 + sel];
  return ppll_vco_hz(sel) / radeon9800_post_div[(div >> R9800_PPLL_POST_DIV_SHIFT) & 7];
}

double bx_radeon9800_c::spll_hz(void)
{
  Bit32u r = pll_regs[R9800_PLL_M_SPLL_REF_FB_DIV];
  Bit32u m = r & 0xff, n = (r >> 16) & 0xff;
  if (m < 2) m = 2;
  if (n < 2) n = 2;
  return 2.0 * ref_freq_hz * (double)n / (double)m;
}

double bx_radeon9800_c::mpll_hz(void)
{
  Bit32u r = pll_regs[R9800_PLL_M_SPLL_REF_FB_DIV];
  Bit32u m = r & 0xff, n = (r >> 8) & 0xff;
  if (m < 2) m = 2;
  if (n < 2) n = 2;
  return 2.0 * ref_freq_hz * (double)n / (double)m;
}

#define R9800_PCICLK_HZ 66666666.0

double bx_radeon9800_c::test_clock_hz(void)
{
  int sel = (test_debug_mux >> 8) & 0x1f;
  int ppll_sel = (clock_cntl_index >> R9800_PLL_DIV_SEL_SHIFT) & 3;
  switch (sel) {
    case 0x01: return ref_freq_hz;
    case 0x02: return ppll_vco_hz(ppll_sel) / 2.0;
    case 0x05:
    case 0x06: return ppll_vco_hz(ppll_sel);
    case 0x0b: return spll_hz();
    case 0x0c: return spll_hz() / 2.0;
    case 0x0f:
      switch (pll_regs[R9800_PLL_SCLK_CNTL] & 0x7) {
        case 0: return R9800_PCICLK_HZ;
        case 1: return spll_hz();
        case 2: return spll_hz() / 2.0;
        case 3: return spll_hz() / 4.0;
        case 4: return spll_hz() / 8.0;
        default: return 0.0;
      }
    case 0x13: return mpll_hz();
    case 0x14: return mpll_hz() / 2.0;
    case 0x16:
      switch (pll_regs[R9800_PLL_MCLK_CNTL] & 0x7) {
        case 0: return R9800_PCICLK_HZ;
        case 1: return mpll_hz();
        case 2: return mpll_hz() / 2.0;
        case 3: return mpll_hz() / 4.0;
        case 4: return mpll_hz() / 8.0;
        case 7: return ref_freq_hz;
        default: return 0.0;
      }
    default: return 0.0;
  }
}

#define R9800_PLL_TEST_ACCESS_S (16.0 / 27000000.0)

// An atomic divider update is held back to the next vertical blank only
// while the CRTC it feeds is really scanning. During a mode set the PLL
// and the CRTC are in reset and no vertical blank can arrive, so a
// deferred update would never complete and a driver polling
// PPLL_ATOMIC_UPDATE_R would spin forever.
bool bx_radeon9800_c::pll_can_defer(int which)
{
  Bit32u cntl = pll_regs[which ? R9800_PLL_P2PLL_CNTL : R9800_PLL_PPLL_CNTL];
  Bit32u gen = which ? crtc2_gen_cntl : crtc_gen_cntl;
  if (cntl & (R9800_PPLL_RESET | R9800_PPLL_SLEEP))
    return false;
  if (!(gen & R9800_CRTC_EN))
    return false;
  return timing_valid;
}

// Retire a deferred update whose vertical blank can no longer arrive
void bx_radeon9800_c::pll_settle(void)
{
  if (ppll_update_pending && !pll_can_defer(0)) {
    ppll_commit();
    ppll_update_pending = false;
    timing_update();
  }
  if (p2pll_update_pending && !pll_can_defer(1)) {
    p2pll_work[0] = pll_regs[R9800_PLL_P2PLL_REF_DIV];
    p2pll_work[1] = pll_regs[R9800_PLL_P2PLL_DIV_0];
    p2pll_update_pending = false;
    timing_update();
  }
}

Bit32u bx_radeon9800_c::pll_read(void)
{
  int idx = clock_cntl_index & R9800_PLL_ADDR_MASK;
  Bit32u v = pll_regs[idx];

  switch (idx) {
    case R9800_PLL_PPLL_REF_DIV:
    case R9800_PLL_PPLL_DIV_0: case R9800_PLL_PPLL_DIV_0 + 1:
    case R9800_PLL_PPLL_DIV_0 + 2: case R9800_PLL_PPLL_DIV_3:
      pll_settle();
      v = (v & ~R9800_PPLL_ATOMIC_UPDATE) | (ppll_update_pending ? R9800_PPLL_ATOMIC_UPDATE : 0);
      break;
    case R9800_PLL_P2PLL_REF_DIV:
    case R9800_PLL_P2PLL_DIV_0:
      pll_settle();
      v = (v & ~R9800_PPLL_ATOMIC_UPDATE) | (p2pll_update_pending ? R9800_PPLL_ATOMIC_UPDATE : 0);
      break;
    case R9800_PLL_PLL_TEST_CNTL: {
      // TEST_COUNT [31:24]: free-running counter of the test-mux clock
      Bit8u count = pll_test_count_base;
      if (!(pll_regs[R9800_PLL_PLL_TEST_CNTL] & (1 << 9))) {
        double hz = test_clock_hz();
        if (hz > 0.0) {
          pll_test_acc += R9800_PLL_TEST_ACCESS_S * hz;
          double elapsed_us = (double)(bx_virt_timer.time_usec(0) - pll_test_zero_usec);
          Bit64u ticks = (Bit64u)(elapsed_us * hz / 1000000.0 + pll_test_acc);
          count = (Bit8u)(pll_test_count_base + ticks);
        }
      }
      v = (v & 0x00ffffff) | ((Bit32u)count << 24);
      break;
    }
    case R9800_PLL_CLK_PWRMGT_CNTL:
      v &= ~(1u << 16);  // MC_BUSY clear
      break;
    default:
      break;
  }
  return v;
}

void bx_radeon9800_c::pll_write(Bit32u val, Bit32u mask)
{
  int idx = clock_cntl_index & R9800_PLL_ADDR_MASK;
  Bit32u merged;

  if (!(clock_cntl_index & R9800_PLL_WR_EN))
    return;
  merged = (pll_regs[idx] & ~mask) | (val & mask);
  switch (idx) {
    case R9800_PLL_PPLL_REF_DIV:
    case R9800_PLL_PPLL_DIV_0: case R9800_PLL_PPLL_DIV_0 + 1:
    case R9800_PLL_PPLL_DIV_0 + 2: case R9800_PLL_PPLL_DIV_3: {
      pll_regs[idx] = merged & ~R9800_PPLL_ATOMIC_UPDATE;
      Bit32u cntl = pll_regs[R9800_PLL_PPLL_CNTL];
      bool atomic = (cntl & (R9800_PPLL_ATOMIC_UPDATE_EN | R9800_PPLL_VGA_ATOMIC_UPDATE_EN)) != 0;
      bool requested = (mask & R9800_PPLL_ATOMIC_UPDATE) && (merged & R9800_PPLL_ATOMIC_UPDATE);
      if (atomic && requested && (cntl & R9800_PPLL_ATOMIC_UPDATE_VSYNC) && pll_can_defer(0)) {
        ppll_update_pending = true;
        return;
      }
      if (atomic && !requested)
        return;
      ppll_commit();
      ppll_update_pending = false;
      timing_update();
      return;
    }
    case R9800_PLL_P2PLL_REF_DIV:
    case R9800_PLL_P2PLL_DIV_0: {
      pll_regs[idx] = merged & ~R9800_PPLL_ATOMIC_UPDATE;
      Bit32u cntl = pll_regs[R9800_PLL_P2PLL_CNTL];
      bool atomic = (cntl & (R9800_PPLL_ATOMIC_UPDATE_EN | R9800_PPLL_VGA_ATOMIC_UPDATE_EN)) != 0;
      bool requested = (mask & R9800_PPLL_ATOMIC_UPDATE) && (merged & R9800_PPLL_ATOMIC_UPDATE);
      if (atomic && requested && (cntl & R9800_PPLL_ATOMIC_UPDATE_VSYNC) && pll_can_defer(1)) {
        p2pll_update_pending = true;
        return;
      }
      if (atomic && !requested)
        return;
      p2pll_work[0] = pll_regs[R9800_PLL_P2PLL_REF_DIV];
      p2pll_work[1] = pll_regs[R9800_PLL_P2PLL_DIV_0];
      p2pll_update_pending = false;
      timing_update();
      return;
    }
    case R9800_PLL_PLL_TEST_CNTL:
      if (mask & 0xff000000) {
        pll_test_count_base = merged >> 24;
        pll_test_zero_usec = bx_virt_timer.time_usec(0);
        pll_test_acc = 0;
      }
      pll_regs[idx] = merged & 0x00ffffff;
      return;
    case R9800_PLL_VCLK_ECP_CNTL:
    case R9800_PLL_HTOTAL_CNTL:
    case R9800_PLL_PIXCLKS_CNTL:
    case R9800_PLL_HTOTAL2_CNTL:
      pll_regs[idx] = merged;
      timing_update();
      return;
    default:
      pll_regs[idx] = merged;
      return;
  }
}

// Push the CRTC1 palette to the GUI (CPU thread)
void bx_radeon9800_c::palette_sync_gui(void)
{
  for (unsigned i = 0; i < 256; i++) {
    bx_gui->palette_change_common(i, BX_R9800_THIS s.pel.data[i].red << BX_R9800_THIS s.dac_shift,
                                  BX_R9800_THIS s.pel.data[i].green << BX_R9800_THIS s.dac_shift,
                                  BX_R9800_THIS s.pel.data[i].blue << BX_R9800_THIS s.dac_shift);
  }
}

// Palette dword path (PALETTE_INDEX / PALETTE_DATA / PALETTE_30_DATA).
// DAC_CNTL2 PALETTE_ACC_CTL selects the CRTC2 palette.
void bx_radeon9800_c::palette_data_write(Bit32u val, Bit32u mask, bool wide)
{
  int idx = palette_index & 0xff;
  Bit32u cur, merged;
  Bit32u r, g, b;

  if (dac_cntl2 & R9800_DAC2_PALETTE_ACC_CTL) {
    Bit32u p = palette2[idx];
    if (wide)
      cur = ((p & 0xff) << 2) | (((p >> 8) & 0xff) << 12) | (((p >> 16) & 0xff) << 22);
    else
      cur = p;
    merged = (cur & ~mask) | (val & mask);
    if (wide)
      palette2[idx] = ((merged >> 2) & 0xff) | (((merged >> 12) & 0xff) << 8) | (((merged >> 22) & 0xff) << 16);
    else
      palette2[idx] = merged & 0x00ffffff;
    if (disp_crtc == 1) needs_update_dispentire = true;
    palette_index = (palette_index & ~0xff) | ((idx + 1) & 0xff);
    return;
  }
  if (wide) {
    cur = palette30[idx][2] | ((Bit32u)palette30[idx][1] << 10) | ((Bit32u)palette30[idx][0] << 20);
    merged = (cur & ~mask) | (val & mask);
    b = merged & 0x3ff;
    g = (merged >> 10) & 0x3ff;
    r = (merged >> 20) & 0x3ff;
  } else {
    if (dac_cntl & R9800_DAC_8BIT_EN)
      cur = BX_R9800_THIS s.pel.data[idx].blue | (BX_R9800_THIS s.pel.data[idx].green << 8) |
            (BX_R9800_THIS s.pel.data[idx].red << 16);
    else
      cur = (BX_R9800_THIS s.pel.data[idx].blue & 0x3f) | ((BX_R9800_THIS s.pel.data[idx].green & 0x3f) << 8) |
            ((BX_R9800_THIS s.pel.data[idx].red & 0x3f) << 16);
    merged = (cur & ~mask) | (val & mask);
    b = (merged & 0xff) << 2;
    g = ((merged >> 8) & 0xff) << 2;
    r = ((merged >> 16) & 0xff) << 2;
  }
  palette30[idx][0] = (Bit16u)r;
  palette30[idx][1] = (Bit16u)g;
  palette30[idx][2] = (Bit16u)b;
  Bit8u r8 = (Bit8u)(r >> 2), g8 = (Bit8u)(g >> 2), b8 = (Bit8u)(b >> 2);
  if (!(dac_cntl & R9800_DAC_8BIT_EN)) {
    b8 &= 0x3f; g8 &= 0x3f; r8 &= 0x3f;
  }
  BX_R9800_THIS s.pel.data[idx].red = r8;
  BX_R9800_THIS s.pel.data[idx].green = g8;
  BX_R9800_THIS s.pel.data[idx].blue = b8;
  if (on_cp_thread()) {
    palette_dirty = true;
  } else {
    bx_gui->palette_change_common(idx, r8 << BX_R9800_THIS s.dac_shift, g8 << BX_R9800_THIS s.dac_shift,
                                  b8 << BX_R9800_THIS s.dac_shift);
  }
  needs_update_dispentire = true;
  palette_index = (palette_index & ~0xff) | ((idx + 1) & 0xff);
}

Bit32u bx_radeon9800_c::palette_data_read(bool wide)
{
  int idx = (palette_index >> 16) & 0xff;
  Bit32u v;
  if (dac_cntl2 & R9800_DAC2_PALETTE_ACC_CTL) {
    Bit32u p = palette2[idx];
    if (wide)
      v = ((p & 0xff) << 2) | (((p >> 8) & 0xff) << 12) | (((p >> 16) & 0xff) << 22);
    else
      v = p;
  } else if (wide) {
    v = palette30[idx][2] | ((Bit32u)palette30[idx][1] << 10) | ((Bit32u)palette30[idx][0] << 20);
  } else {
    v = BX_R9800_THIS s.pel.data[idx].blue | (BX_R9800_THIS s.pel.data[idx].green << 8) |
        (BX_R9800_THIS s.pel.data[idx].red << 16);
  }
  palette_index = (palette_index & ~0xff0000) | (((idx + 1) & 0xff) << 16);
  return v;
}

void bx_radeon9800_c::latch_crtc_offset(int c)
{
  crtc[c].offset_latched = crtc[c].offset;
  crtc[c].offset_pending = false;
  if (c == disp_crtc) {
    needs_update_mode = true;
    needs_update_dispentire = true;
  }
}

// A programmed scanout offset is picked up at the next vertical blank.
// A CRTC that is not scanning produces none, so a base programmed during
// a mode set would otherwise stay pending and the display would keep
// scanning from the previous, stale address.
void bx_radeon9800_c::crtc_settle(int c)
{
  Bit32u gen = c ? crtc2_gen_cntl : crtc_gen_cntl;
  if (!crtc[c].offset_pending || crtc[c].offset_lock)
    return;
  if ((gen & R9800_CRTC_EN) && timing_valid)
    return;
  latch_crtc_offset(c);
}

// CUR_LOCK is one flag surfaced as bit 31 of the three geometry registers:
// while locked, writes land only in the programmed images; the write that
// clears the lock publishes the whole tuple.
void bx_radeon9800_c::cursor_publish(int c)
{
  if (crtc[c].cur_lock)
    return;
  crtc[c].cur_offset_act = crtc[c].cur_offset;
  crtc[c].cur_posn_act = crtc[c].cur_posn;
  crtc[c].cur_hvoff_act = crtc[c].cur_hvoff;
}

void bx_radeon9800_c::cursor_rect(Bit32u posn, Bit32u hvoff, int *x0, int *y0, int *x1, int *y1)
{
  *x0 = (posn >> 16) & 0x7ff;
  *y0 = posn & 0x7ff;
  *x1 = *x0 + 64 - (int)((hvoff >> 16) & 0x3f);
  *y1 = *y0 + 64 - (int)(hvoff & 0x3f);
}

// Vertical blank: latch the cursor tuple of the scanned-out CRTC
void bx_radeon9800_c::cursor_frame_latch(void)
{
  int c = disp_crtc;
  Bit32u gen = c ? crtc2_gen_cntl : crtc_gen_cntl;
  bool en = disp_ext && ((gen & R9800_CRTC_CUR_EN) != 0);
  Bit32u mode = (gen & R9800_CRTC_CUR_MODE_MASK) >> R9800_CRTC_CUR_MODE_SHIFT;
  Bit32u off;
  bool changed = (en != cur_lat_en);
  // cursor image address = DISPLAY_BASE_ADDR + CUR_OFFSET (MC address)
  if (!mc_is_vram(crtc[c].display_base + (crtc[c].cur_offset_act & 0x0ffffff0), &off))
    off = (crtc[c].cur_offset_act & 0x0ffffff0) & vram_mask;
  if (en && !changed) {
    changed = (off != cur_lat_offset) || (crtc[c].cur_posn_act != cur_lat_posn) ||
              (crtc[c].cur_hvoff_act != cur_lat_hvoff) || (crtc[c].cur_clr0 != cur_lat_clr0) ||
              (crtc[c].cur_clr1 != cur_lat_clr1) || (mode != cur_lat_mode) || cur_bitmap_dirty;
  }
  if (!changed) {
    cur_bitmap_dirty = false;
    return;
  }
  int ox0, oy0, ox1, oy1, nx0, ny0, nx1, ny1;
  bool old_en = cur_lat_en;
  cursor_rect(cur_lat_posn, cur_lat_hvoff, &ox0, &oy0, &ox1, &oy1);
  cur_lat_en = en;
  cur_lat_mode = mode;
  cur_lat_offset = off;
  cur_lat_posn = crtc[c].cur_posn_act;
  cur_lat_hvoff = crtc[c].cur_hvoff_act;
  cur_lat_clr0 = crtc[c].cur_clr0;
  cur_lat_clr1 = crtc[c].cur_clr1;
  cur_bitmap_dirty = false;
  cursor_rect(cur_lat_posn, cur_lat_hvoff, &nx0, &ny0, &nx1, &ny1);
  if (!disp_ext)
    return;

  int bx0 = 0x7fffffff, by0 = 0x7fffffff, bx1 = 0, by1 = 0;
  if (old_en && (ox1 > ox0) && (oy1 > oy0)) {
    redraw_area(ox0, oy0, ox1 - ox0, oy1 - oy0);
    if (ox0 < bx0) bx0 = ox0;
    if (oy0 < by0) by0 = oy0;
    if (ox1 > bx1) bx1 = ox1;
    if (oy1 > by1) by1 = oy1;
  }
  if (en && (nx1 > nx0) && (ny1 > ny0)) {
    redraw_area(nx0, ny0, nx1 - nx0, ny1 - ny0);
    if (nx0 < bx0) bx0 = nx0;
    if (ny0 < by0) by0 = ny0;
    if (nx1 > bx1) bx1 = nx1;
    if (ny1 > by1) by1 = ny1;
  }
  if (needs_update_mode || needs_update_dispentire || !ext_last)
    return;
  if ((bx1 > bx0) && (by1 > by0))
    paint_tiles_in(bx0, by0, bx1, by1);
}

// Paint the dirty display tiles intersecting [x0,x1) x [y0,y1) now
void bx_radeon9800_c::paint_tiles_in(int x0, int y0, int x1, int y1)
{
  bx_svga_tileinfo_t info;
  Bit8u pel8[256 * 3];
  unsigned w, h;

  if (!bx_gui->graphics_tile_info_common(&info))
    return;
  if (info.snapshot_mode)
    return;
  if (info.is_indexed && (disp_bpp != 8) && (disp_bpp != 4))
    return;
  if (x0 < 0) x0 = 0;
  if (y0 < 0) y0 = 0;
  if (x1 > (int)disp_xres) x1 = disp_xres;
  if (y1 > (int)disp_yres) y1 = disp_yres;
  if ((x0 >= x1) || (y0 >= y1))
    return;
  for (int i = 0; i < 256; i++) {
    if (disp_crtc == 1) {
      pel8[i * 3 + 0] = (Bit8u)(palette2[i] >> 16);
      pel8[i * 3 + 1] = (Bit8u)(palette2[i] >> 8);
      pel8[i * 3 + 2] = (Bit8u)palette2[i];
    } else {
      pel8[i * 3 + 0] = BX_R9800_THIS s.pel.data[i].red;
      pel8[i * 3 + 1] = BX_R9800_THIS s.pel.data[i].green;
      pel8[i * 3 + 2] = BX_R9800_THIS s.pel.data[i].blue;
    }
  }
  unsigned xt0 = x0 / X_TILESIZE, xt1 = (x1 - 1) / X_TILESIZE;
  unsigned yt0 = y0 / Y_TILESIZE, yt1 = (y1 - 1) / Y_TILESIZE;
  for (unsigned yti = yt0; yti <= yt1; yti++) {
    for (unsigned xti = xt0; xti <= xt1; xti++) {
      if (!GET_TILE_UPDATED(xti, yti))
        continue;
      unsigned xc = xti * X_TILESIZE, yc = yti * Y_TILESIZE;
      paint_tile(xc, yc, &info, pel8);
      bx_gui->graphics_tile_get(xc, yc, &w, &h);
      bx_gui->graphics_tile_update_in_place(xc, yc, w, h);
      SET_TILE_UPDATED(BX_R9800_THIS, xti, yti, 0);
    }
  }
  bx_gui->flush();
}

void bx_radeon9800_c::snapshot_take(int c)
{
  Bit32u hcount = 0;
  if (BX_R9800_THIS s.htotal_usec > 0) {
    Bit32u htot = (crtc[c].h_total_disp & 0x3ff) + 1;
    Bit32u line_usec = get_display_usec() % BX_R9800_THIS s.htotal_usec;
    hcount = (line_usec * htot / BX_R9800_THIS s.htotal_usec) & 0x3ff;
  }
  crtc[c].snapshot_vh_counts = hcount | ((get_current_line() & 0x7ff) << 16);
  crtc[c].snapshot_f_count = crtc[c].frame_count & 0x1fffff;
  gen_int_status |= c ? R9800_INT_SNAPSHOT2 : R9800_INT_SNAPSHOT;
  gen_int_update();
}

// Which CRTC feeds the primary DAC (the emulated monitor)
// The emulated monitor follows the first active output, in the order
// primary DAC, internal TMDS, TV DAC, external TMDS/DVO. An output is
// active when it is powered and its source CRTC runs an extended mode
// (CRTC1: EXT_DISP_EN, CRTC2: CRTC2_EN). With nothing active the primary
// DAC shows CRTC1 (VGA core).
// Does a CRTC drive an extended (non-VGA) mode? CRTC_EXT_DISP_EN selects
// the extended display path, but a driver may reprogram CRTC_GEN_CNTL for
// its own mode and leave that bit alone: a running CRTC whose pixel format
// is one the VGA core cannot express (15/16/24/32 bpp) with the extended
// horizontal timing programmed is an extended mode as well. CRTC2 has no
// VGA path at all, so being enabled is enough.
bool bx_radeon9800_c::crtc_extended(int c)
{
  Bit32u pw;
  if (c)
    return (crtc2_gen_cntl & R9800_CRTC_EN) != 0;
  if (crtc_gen_cntl & R9800_CRTC_EXT_DISP_EN)
    return true;
  pw = (crtc_gen_cntl >> R9800_CRTC_PIX_WIDTH_SHIFT) & 0xf;
  return ((crtc_gen_cntl & R9800_CRTC_EN) != 0) && (pw >= 3) && (pw <= 6) &&
         (((crtc[0].h_total_disp >> 16) & 0x1ff) != 0);
}

int bx_radeon9800_c::scanout_crtc(void)
{
  Bit32u fp = fp_regs[(R9800_FP_GEN_CNTL - R9800_FP_CRTC_H_TOTAL_DISP) >> 2];
  Bit32u fp2 = fp_regs[(R9800_FP2_GEN_CNTL - R9800_FP_CRTC_H_TOTAL_DISP) >> 2];
  bool crtc1_ext = crtc_extended(0);
  bool crtc2_on = crtc_extended(1);
  struct { bool on; int src; int out; } o[4];

  o[0].out = R9800_OUT_DAC1;
  o[0].on = !(dac_cntl & R9800_DAC_PDWN);
  o[0].src = ((disp_output_cntl & R9800_DISP_DAC_SOURCE_MASK) == 1) ? 1 : 0;
  o[1].out = R9800_OUT_TMDS1;
  o[1].on = (fp & R9800_FP_FPON) != 0;
  o[1].src = ((fp & R9800_FP_SRC_SEL_MASK) == R9800_FP_SRC_SEL_CRTC2) ? 1 : 0;
  o[2].out = R9800_OUT_TVDAC;
  o[2].on = (tv_dac_cntl & (R9800_TV_DAC_RDACPD | R9800_TV_DAC_GDACPD | R9800_TV_DAC_BDACPD)) !=
            (R9800_TV_DAC_RDACPD | R9800_TV_DAC_GDACPD | R9800_TV_DAC_BDACPD);
  o[2].src = ((disp_output_cntl & R9800_DISP_TVDAC_SOURCE_MASK) == R9800_DISP_TVDAC_SOURCE_CRTC2) ? 1 : 0;
  o[3].out = R9800_OUT_TMDS2;
  o[3].on = (fp2 & R9800_FP2_ON) != 0;
  o[3].src = ((fp2 & R9800_FP2_SRC_SEL_MASK) == R9800_FP2_SRC_SEL_CRTC2) ? 1 : 0;

  for (int i = 0; i < 4; i++) {
    if (!o[i].on) continue;
    if (o[i].src ? crtc2_on : crtc1_ext) {
      disp_output = o[i].out;
      return o[i].src;
    }
  }
  disp_output = R9800_OUT_DAC1;
  // primary DAC routed to CRTC2 with nothing else running
  if (o[0].on && (o[0].src == 1) && crtc2_on)
    return 1;
  return 0;
}

// Re-evaluate which CRTC / output is shown after a routing register write
void bx_radeon9800_c::scanout_refresh(void)
{
  int old_crtc = disp_crtc;
  bool old_ext = disp_ext;
  int c = scanout_crtc();
  disp_ext = crtc_extended(c);
  if ((c != old_crtc) || (disp_ext != old_ext)) {
    disp_crtc = c;
    update_banking();
    needs_update_mode = true;
    needs_update_dispentire = true;
    timing_update();
  }
}

bool bx_radeon9800_c::display_reg_read(Bit32u off, Bit32u *val)
{
  int c = 0;
  Bit32u base = off;
  // CRTC2 timing / cursor registers mirror the CRTC1 layout at +0x100
  if ((off >= 0x300) && (off <= 0x37c) && (off != R9800_CRTC_TILE_X0_Y0) && (off != R9800_CRTC2_TILE_X0_Y0)) {
    c = 1;
    base = off - 0x100;
  }
  switch (off) {
    case R9800_CLOCK_CNTL_INDEX: *val = clock_cntl_index; return true;
    case R9800_CLOCK_CNTL_DATA:  *val = pll_read(); return true;
    case R9800_CRTC_GEN_CNTL:    *val = crtc_gen_cntl; return true;
    case R9800_CRTC_EXT_CNTL:    *val = crtc_ext_cntl; return true;
    case R9800_CRTC2_GEN_CNTL:   *val = crtc2_gen_cntl; return true;
    case R9800_DAC_CNTL:
      *val = (dac_cntl & 0x00ffff7f) | R9800_DAC_CMP_OUTPUT | ((Bit32u)dac_mask_prog << 24);
      return true;
    case R9800_DAC_CNTL2:
      // CRT2 (TV DAC) load detect never succeeds: no monitor on the second DAC
      *val = dac_cntl2 & ~(R9800_DAC2_CMP_OUTPUT | 0x700);
      return true;
    case R9800_CRTC_STATUS:
    case R9800_CRTC2_STATUS: {
      int s = (off == R9800_CRTC2_STATUS) ? 1 : 0;
      *val = (in_vblank() ? 0x1 : 0x0) | (crtc[s].vblank_save ? 0x2 : 0x0) |
             ((get_current_line() & 1) << 2) | ((crtc[s].frame_count & 1) << 3);
      return true;
    }
    case R9800_MEM_VGA_WP_SEL: *val = mem_vga_wp_sel; return true;
    case R9800_MEM_VGA_RP_SEL: *val = mem_vga_rp_sel; return true;
    case R9800_PALETTE_INDEX:  *val = palette_index; return true;
    case R9800_PALETTE_DATA:   *val = palette_data_read(false); return true;
    case R9800_PALETTE_30_DATA: *val = palette_data_read(true); return true;
    case R9800_N_VIF_COUNT:        *val = n_vif_count; return true;
    case R9800_SNAPSHOT_VIF_COUNT: *val = snapshot_vif_cntl; return true;
    case R9800_CRTC_OFFSET_RIGHT:  *val = crtc_offset_right; return true;
    case R9800_CRTC_TILE_X0_Y0:    *val = crtc[0].tile_x0_y0; return true;
    case R9800_CRTC2_TILE_X0_Y0:   *val = crtc[1].tile_x0_y0; return true;
    case R9800_DAC_EXT_CNTL: *val = dac_ext_cntl; return true;
    case R9800_DAC_CRC_SIG:  *val = dac_crc_sig; return true;
    case R9800_CRTC_MORE_CNTL: *val = crtc_more_cntl; return true;
    case R9800_LVDS_GEN_CNTL: *val = lvds_gen_cntl; return true;
    case R9800_LVDS_PLL_CNTL: *val = lvds_pll_cntl; return true;
    case R9800_LVDS_SS_GEN_CNTL: *val = lvds_ss_gen_cntl; return true;
    case R9800_TMDS_CRC: case R9800_TMDS_TRANSMITTER_CNTL: case R9800_TMDS_PLL_CNTL:
      *val = tmds_regs[(off - R9800_TMDS_CRC) >> 2]; return true;
    case R9800_VGA_BUFFER_CNTL: *val = vga_buffer_cntl; return true;
    case R9800_GRPH_BUFFER_CNTL: *val = crtc[0].grph_buffer_cntl; return true;
    case R9800_GRPH2_BUFFER_CNTL: *val = crtc[1].grph_buffer_cntl; return true;
    case R9800_FP_GEN_CNTL:
      // FP_DETECT_SENSE: no digital panel attached
      *val = fp_regs[(off - R9800_FP_CRTC_H_TOTAL_DISP) >> 2] & ~(Bit32u)R9800_FP_DETECT_SENSE;
      return true;
    case R9800_FP2_GEN_CNTL:
      *val = fp_regs[(off - R9800_FP_CRTC_H_TOTAL_DISP) >> 2] & ~(Bit32u)R9800_FP2_DETECT_SENSE;
      return true;
    case R9800_FP2_2_GEN_CNTL: case R9800_FP_HORZ2_STRETCH: case R9800_FP_VERT2_STRETCH:
      *val = fp2_regs[(off - R9800_FP2_2_GEN_CNTL) >> 2]; return true;
    case R9800_FP_H2_SYNC_STRT_WID: *val = fp2_regs[4]; return true;
    case R9800_FP_V2_SYNC_STRT_WID: *val = fp2_regs[5]; return true;
    default:
      break;
  }
  // flat panel block 0x250-0x2c8 (register storage; CRTC1 shadow timing)
  if ((off >= R9800_FP_CRTC_H_TOTAL_DISP) && (off <= R9800_FP_V_SYNC_STRT_WID) &&
      ((off < R9800_CUR_OFFSET) || (off > R9800_CUR_CLR1)) && (off != R9800_DAC_EXT_CNTL) &&
      (off != R9800_CRTC_MORE_CNTL) && (off != R9800_TMDS_CRC) && (off != R9800_TMDS_TRANSMITTER_CNTL) &&
      (off != R9800_TMDS_PLL_CNTL)) {
    *val = fp_regs[(off - R9800_FP_CRTC_H_TOTAL_DISP) >> 2];
    return true;
  }
  struct r9800_crtc_t *ct = &crtc[c];
  switch (base) {
    case R9800_CRTC_H_TOTAL_DISP:    *val = ct->h_total_disp; return true;
    case R9800_CRTC_H_SYNC_STRT_WID: *val = ct->h_sync_strt_wid; return true;
    case R9800_CRTC_V_TOTAL_DISP:    *val = ct->v_total_disp; return true;
    case R9800_CRTC_V_SYNC_STRT_WID: *val = ct->v_sync_strt_wid; return true;
    case R9800_CRTC_VLINE_CRNT_VLINE:
      *val = (ct->vline & 0x7ff) | ((get_current_line() & 0x7ff) << 16);
      return true;
    case R9800_CRTC_CRNT_FRAME:    *val = ct->frame_count; return true;
    case R9800_SNAPSHOT_VH_COUNTS: *val = ct->snapshot_vh_counts; return true;
    case R9800_SNAPSHOT_F_COUNT:   *val = ct->snapshot_f_count; return true;
    case R9800_CRTC_GUI_TRIG_VLINE: {
      Bit32u strt = ct->gui_trig_vline & 0x7ff;
      Bit32u end = (ct->gui_trig_vline >> 16) & 0x7ff;
      Bit32u line = get_current_line();
      *val = (ct->gui_trig_vline & 0x07ff07ff) | (((line >= strt) && (line <= end)) ? 0x80000000 : 0);
      return true;
    }
    case R9800_CRTC_DEBUG: *val = ct->debug; return true;
    case R9800_CRTC_OFFSET:
      crtc_settle(c);
      *val = (ct->offset & 0x0fffffff) | (ct->offset_pending ? 0x40000000 : 0) | (ct->offset_lock ? 0x80000000 : 0);
      return true;
    case R9800_CRTC_OFFSET_CNTL:
      crtc_settle(c);
      *val = (ct->offset_cntl & 0x3fffffff) | (ct->offset_pending ? 0x40000000 : 0) | (ct->offset_lock ? 0x80000000 : 0);
      return true;
    case R9800_CRTC_PITCH: *val = ct->pitch; return true;
    case R9800_OVR_CLR: *val = ct->ovr_clr; return true;
    case R9800_OVR_WID_LEFT_RIGHT: *val = ct->ovr_wid_lr; return true;
    case R9800_OVR_WID_TOP_BOTTOM: *val = ct->ovr_wid_tb; return true;
    case R9800_DISPLAY_BASE_ADDR: *val = ct->display_base; return true;
    case R9800_CUR_OFFSET: *val = ct->cur_offset | (ct->cur_lock ? 0x80000000 : 0); return true;
    case R9800_CUR_HORZ_VERT_POSN: *val = ct->cur_posn | (ct->cur_lock ? 0x80000000 : 0); return true;
    case R9800_CUR_HORZ_VERT_OFF:  *val = ct->cur_hvoff | (ct->cur_lock ? 0x80000000 : 0); return true;
    case R9800_CUR_CLR0: *val = ct->cur_clr0; return true;
    case R9800_CUR_CLR1: *val = ct->cur_clr1; return true;
    default:
      return false;
  }
}

bool bx_radeon9800_c::display_reg_write(Bit32u off, Bit32u val, Bit32u mask)
{
  int c = 0;
  Bit32u base = off;
  if ((off >= 0x300) && (off <= 0x37c) && (off != R9800_CRTC_TILE_X0_Y0) && (off != R9800_CRTC2_TILE_X0_Y0)) {
    c = 1;
    base = off - 0x100;
  }
#define MERGE(field) ((field) = ((field) & ~mask) | (val & mask))
  switch (off) {
    case R9800_CLOCK_CNTL_INDEX:
      MERGE(clock_cntl_index);
      clock_cntl_index &= 0x3ff;
      timing_update();
      return true;
    case R9800_CLOCK_CNTL_DATA:
      pll_write(val, mask);
      return true;
    case R9800_CRTC_GEN_CNTL: {
      Bit32u was_ext = crtc_gen_cntl & R9800_CRTC_EXT_DISP_EN;
      Bit32u gen_old = crtc_gen_cntl;
      MERGE(crtc_gen_cntl);
      if (!was_ext && (crtc_gen_cntl & R9800_CRTC_EXT_DISP_EN))
        crtc[0].v_disp_active = (crtc[0].v_total_disp >> 16) & 0x7ff;
      scanout_refresh();
      // CRTC_CUR_EN [16] and CRTC_CUR_MODE [22:20] only feed the cursor
      if ((gen_old ^ crtc_gen_cntl) & ~(R9800_CRTC_CUR_EN | R9800_CRTC_CUR_MODE_MASK | R9800_CRTC_ICON_EN)) {
        update_banking();
        needs_update_mode = true;
        needs_update_dispentire = true;
        timing_update();
      }
      return true;
    }
    case R9800_CRTC2_GEN_CNTL: {
      Bit32u gen_old = crtc2_gen_cntl;
      MERGE(crtc2_gen_cntl);
      if (!(gen_old & R9800_CRTC_EN) && (crtc2_gen_cntl & R9800_CRTC_EN))
        crtc[1].v_disp_active = (crtc[1].v_total_disp >> 16) & 0x7ff;
      scanout_refresh();
      if ((gen_old ^ crtc2_gen_cntl) & ~(R9800_CRTC_CUR_EN | R9800_CRTC_CUR_MODE_MASK | R9800_CRTC_ICON_EN)) {
        needs_update_mode = true;
        needs_update_dispentire = true;
        timing_update();
      }
      return true;
    }
    case R9800_CRTC_EXT_CNTL: {
      Bit32u ext_old = crtc_ext_cntl;
      MERGE(crtc_ext_cntl);
      if (ext_old != crtc_ext_cntl) {
        update_banking();
        needs_update_mode = true;
        needs_update_dispentire = true;
      }
      return true;
    }
    case R9800_DAC_CNTL: {
      Bit32u dac_old = dac_cntl;
      MERGE(dac_cntl);
      dac_cntl &= 0x00ffff7f;
      if ((dac_old ^ dac_cntl) & (R9800_DAC_4BPP_PIX_ORDER | R9800_DAC_PDWN | R9800_DAC_8BIT_EN | R9800_DAC_BLANKING)) {
        if ((dac_old ^ dac_cntl) & R9800_DAC_PDWN)
          scanout_refresh();
        needs_update_mode = true;
        needs_update_dispentire = true;
      }
      if (mask & 0xff000000) {
        Bit8u m = (Bit8u)(mask >> 24);
        dac_mask_prog = (Bit8u)((dac_mask_prog & ~m) | ((val >> 24) & m));
      }
      BX_R9800_THIS s.dac_shift = (dac_cntl & R9800_DAC_8BIT_EN) ? 0 : 2;
      return true;
    }
    case R9800_DAC_CNTL2:
      MERGE(dac_cntl2);
      return true;
    case R9800_CRTC_STATUS:
      if ((mask & 0x2) && (val & 0x2))
        crtc[0].vblank_save = false;
      return true;
    case R9800_CRTC2_STATUS:
      if ((mask & 0x2) && (val & 0x2))
        crtc[1].vblank_save = false;
      return true;
    case R9800_MEM_VGA_WP_SEL:
      MERGE(mem_vga_wp_sel);
      mem_vga_wp_sel &= 0x03ff03ff;
      update_banking();
      if (mem_vga_wp_sel != 0)
        vga_banked_mode = true;
      return true;
    case R9800_MEM_VGA_RP_SEL:
      MERGE(mem_vga_rp_sel);
      mem_vga_rp_sel &= 0x03ff03ff;
      update_banking();
      return true;
    case R9800_PALETTE_INDEX:
      MERGE(palette_index);
      palette_index &= 0x00ff00ff;
      return true;
    case R9800_PALETTE_DATA:
      palette_data_write(val, mask, false);
      return true;
    case R9800_PALETTE_30_DATA:
      palette_data_write(val, mask, true);
      return true;
    case R9800_N_VIF_COUNT:
      MERGE(n_vif_count);
      n_vif_count &= 0x800003ff;
      return true;
    case R9800_SNAPSHOT_VIF_COUNT:
      MERGE(snapshot_vif_cntl);
      snapshot_vif_cntl &= 0x03000000;
      if (val & mask & 0x02000000)
        snapshot_take(0);
      return true;
    case R9800_CRTC_OFFSET_RIGHT: MERGE(crtc_offset_right); return true;
    case R9800_CRTC_TILE_X0_Y0:
      MERGE(crtc[0].tile_x0_y0);
      if (disp_crtc == 0) { needs_update_mode = true; needs_update_dispentire = true; }
      return true;
    case R9800_CRTC2_TILE_X0_Y0:
      MERGE(crtc[1].tile_x0_y0);
      if (disp_crtc == 1) { needs_update_mode = true; needs_update_dispentire = true; }
      return true;
    case R9800_DAC_EXT_CNTL: {
      Bit32u ext_old = dac_ext_cntl;
      MERGE(dac_ext_cntl);
      if ((ext_old ^ dac_ext_cntl) & 0x0003fff0) {
        needs_update_mode = true;
        needs_update_dispentire = true;
      }
      return true;
    }
    case R9800_DAC_CRC_SIG: return true;
    case R9800_CRTC_MORE_CNTL: MERGE(crtc_more_cntl); return true;
    case R9800_LVDS_GEN_CNTL: MERGE(lvds_gen_cntl); return true;
    case R9800_LVDS_PLL_CNTL: MERGE(lvds_pll_cntl); return true;
    case R9800_LVDS_SS_GEN_CNTL: MERGE(lvds_ss_gen_cntl); return true;
    case R9800_TMDS_CRC: case R9800_TMDS_TRANSMITTER_CNTL: case R9800_TMDS_PLL_CNTL:
      MERGE(tmds_regs[(off - R9800_TMDS_CRC) >> 2]); return true;
    case R9800_VGA_BUFFER_CNTL: MERGE(vga_buffer_cntl); return true;
    case R9800_GRPH_BUFFER_CNTL: MERGE(crtc[0].grph_buffer_cntl); return true;
    case R9800_GRPH2_BUFFER_CNTL: MERGE(crtc[1].grph_buffer_cntl); return true;
    case R9800_FP2_2_GEN_CNTL: case R9800_FP_HORZ2_STRETCH: case R9800_FP_VERT2_STRETCH:
      MERGE(fp2_regs[(off - R9800_FP2_2_GEN_CNTL) >> 2]); return true;
    case R9800_FP_H2_SYNC_STRT_WID: MERGE(fp2_regs[4]); return true;
    case R9800_FP_V2_SYNC_STRT_WID: MERGE(fp2_regs[5]); return true;
    default:
      break;
  }
  if ((off >= R9800_FP_CRTC_H_TOTAL_DISP) && (off <= R9800_FP_V_SYNC_STRT_WID) &&
      ((off < R9800_CUR_OFFSET) || (off > R9800_CUR_CLR1)) && (off != R9800_DAC_EXT_CNTL) &&
      (off != R9800_CRTC_MORE_CNTL) && (off != R9800_TMDS_CRC) && (off != R9800_TMDS_TRANSMITTER_CNTL) &&
      (off != R9800_TMDS_PLL_CNTL)) {
    Bit32u *fr = &fp_regs[(off - R9800_FP_CRTC_H_TOTAL_DISP) >> 2];
    Bit32u fold = *fr;
    MERGE(*fr);
    // TMDS enable / CRTC source changes reroute the emulated monitor
    if (((off == R9800_FP_GEN_CNTL) && ((fold ^ *fr) & (R9800_FP_FPON | R9800_FP_SRC_SEL_MASK))) ||
        ((off == R9800_FP2_GEN_CNTL) && ((fold ^ *fr) & (R9800_FP2_ON | R9800_FP2_SRC_SEL_MASK)))) {
      scanout_refresh();
      needs_update_mode = true;
      needs_update_dispentire = true;
    }
    return true;
  }
  struct r9800_crtc_t *ct = &crtc[c];
  bool scanned = (c == disp_crtc);
  switch (base) {
    case R9800_CRTC_H_TOTAL_DISP: {
      Bit32u old = ct->h_total_disp;
      MERGE(ct->h_total_disp);
      if (old != ct->h_total_disp) {
        if (scanned) needs_update_mode = true;
        timing_update();
      }
      return true;
    }
    case R9800_CRTC_H_SYNC_STRT_WID:
      MERGE(ct->h_sync_strt_wid);
      timing_update();
      return true;
    case R9800_CRTC_V_TOTAL_DISP: {
      Bit32u old = ct->v_total_disp, old_active = ct->v_disp_active;
      MERGE(ct->v_total_disp);
      if (mask & 0x07ff0000)
        ct->v_disp_active = (ct->v_total_disp >> 16) & 0x7ff;
      if ((old != ct->v_total_disp) || (old_active != ct->v_disp_active)) {
        if (scanned) needs_update_mode = true;
        timing_update();
      }
      return true;
    }
    case R9800_CRTC_V_SYNC_STRT_WID:
      MERGE(ct->v_sync_strt_wid);
      timing_update();
      return true;
    case R9800_CRTC_VLINE_CRNT_VLINE:
      MERGE(ct->vline);
      ct->vline &= 0x7ff;
      return true;
    case R9800_CRTC_CRNT_FRAME:
    case R9800_SNAPSHOT_VH_COUNTS:
    case R9800_SNAPSHOT_F_COUNT:
      return true;
    case R9800_CRTC_GUI_TRIG_VLINE:
      MERGE(ct->gui_trig_vline);
      ct->gui_trig_vline &= 0x07ff07ff;
      return true;
    case R9800_CRTC_DEBUG:
      MERGE(ct->debug);
      return true;
    case R9800_CRTC_OFFSET: {
      Bit32u merged = (ct->offset & ~mask) | (val & mask);
      ct->offset = merged & 0x0fffffff;
      if (mask & 0x80000000)
        ct->offset_lock = (val & 0x80000000) != 0;
      if (mask & 0x0fffffff) {
        // The target buffer must hold the finished frame before scanout
        // latches it: complete any deferred 3D batch now.
        if (on_cp_thread())
          raster_flush();
        else
          pm4_drain_wait();
        ct->offset_pending = true;
        if (ct->offset_cntl & R9800_CRTC_OFFSET_FLIP_CNTL) {
          if (!ct->offset_lock) {
            latch_crtc_offset(c);
            pm4_flip_notify();
          }
        }
      }
      return true;
    }
    case R9800_CRTC_OFFSET_CNTL:
      MERGE(ct->offset_cntl);
      ct->offset_cntl &= 0x3fffffff;
      if (mask & 0x80000000)
        ct->offset_lock = (val & 0x80000000) != 0;
      if (scanned) needs_update_mode = true;
      return true;
    case R9800_CRTC_PITCH:
      MERGE(ct->pitch);
      ct->pitch &= 0x07ff07ff;
      if (scanned) needs_update_mode = true;
      return true;
    case R9800_OVR_CLR: MERGE(ct->ovr_clr); return true;
    case R9800_OVR_WID_LEFT_RIGHT: MERGE(ct->ovr_wid_lr); return true;
    case R9800_OVR_WID_TOP_BOTTOM: MERGE(ct->ovr_wid_tb); return true;
    case R9800_DISPLAY_BASE_ADDR:
      MERGE(ct->display_base);
      if (scanned) { needs_update_mode = true; needs_update_dispentire = true; }
      return true;
    case R9800_CUR_OFFSET:
      MERGE(ct->cur_offset);
      if (mask & 0x80000000)
        ct->cur_lock = (ct->cur_offset & 0x80000000) != 0;
      ct->cur_offset &= 0x0ffffff0;
      cursor_publish(c);
      return true;
    case R9800_CUR_HORZ_VERT_POSN:
      MERGE(ct->cur_posn);
      if (mask & 0x80000000)
        ct->cur_lock = (ct->cur_posn & 0x80000000) != 0;
      ct->cur_posn &= 0x07ff07ff;
      cursor_publish(c);
      return true;
    case R9800_CUR_HORZ_VERT_OFF:
      MERGE(ct->cur_hvoff);
      if (mask & 0x80000000)
        ct->cur_lock = (ct->cur_hvoff & 0x80000000) != 0;
      ct->cur_hvoff &= 0x003f003f;
      cursor_publish(c);
      return true;
    case R9800_CUR_CLR0:
      MERGE(ct->cur_clr0);
      ct->cur_clr0 &= 0x00ffffff;
      return true;
    case R9800_CUR_CLR1:
      MERGE(ct->cur_clr1);
      ct->cur_clr1 &= 0x00ffffff;
      return true;
    default:
      return false;
  }
#undef MERGE
}

void bx_radeon9800_c::display_reset(void)
{
  memset(pll_regs, 0, sizeof(pll_regs));
  pll_regs[R9800_PLL_CLK_PIN_CNTL] = 0x000000ff;
  pll_regs[R9800_PLL_PPLL_CNTL]    = 0x0000cc03;
  pll_regs[R9800_PLL_SPLL_CNTL]    = 0x0000cc03;
  pll_regs[R9800_PLL_MPLL_CNTL]    = 0x0000cc03;
  pll_regs[R9800_PLL_P2PLL_CNTL]   = 0x0000cc03;
  pll_regs[R9800_PLL_AGP_PLL_CNTL] = 0x7a770000;
  pll_regs[R9800_PLL_PLL_TEST_CNTL] = 0x00000200;
  pll_test_count_base = 0;
  pll_test_zero_usec = bx_virt_timer.time_usec(0);
  pll_test_acc = 0;
  clock_cntl_index = 0;
  ppll_update_pending = false;
  p2pll_update_pending = false;
  ppll_commit();
  p2pll_work[0] = p2pll_work[1] = p2pll_work[2] = 0;

  crtc_gen_cntl = R9800_CRTC_GEN_CNTL_DEFAULT;
  crtc_ext_cntl = R9800_CRTC_EXT_CNTL_DEFAULT;
  crtc2_gen_cntl = 0x04000000;
  dac_cntl = R9800_DAC_CNTL_DEFAULT & 0x00ffff7f;
  dac_cntl2 = 0;
  dac_mask_prog = 0xff;
  BX_R9800_THIS s.dac_shift = 2;
  memset(crtc, 0, sizeof(crtc));
  cur_lat_en = false;
  cur_lat_mode = 0;
  cur_lat_offset = 0;
  cur_lat_posn = 0;
  cur_lat_hvoff = 0;
  cur_lat_clr0 = 0;
  cur_lat_clr1 = 0;
  cur_bitmap_dirty = false;
  crtc_offset_right = 0;
  dac_ext_cntl = 0;
  dac_crc_sig = 0;
  crtc_more_cntl = 0;
  lvds_gen_cntl = 0;
  lvds_pll_cntl = 0;
  lvds_ss_gen_cntl = 0;
  memset(tmds_regs, 0, sizeof(tmds_regs));
  memset(fp_regs, 0, sizeof(fp_regs));
  memset(fp2_regs, 0, sizeof(fp2_regs));
  vga_buffer_cntl = 0;
  palette_index = 0;
  n_vif_count = 0;
  snapshot_vif_cntl = 0;
  mem_vga_wp_sel = 0;
  mem_vga_rp_sel = 0;
  disp_ext = false;
  disp_crtc = 0;
  disp_output = R9800_OUT_DAC1;
  update_banking();
  needs_update_mode = true;
  needs_update_dispentire = true;
}

// ---------------------------------------------------------------------
// Timing for the VGA core: vertical timer parameters from the extended CRTC
// ---------------------------------------------------------------------

void bx_radeon9800_c::get_crtc_params(bx_crtc_params_t *crtcp, Bit32u *vclock)
{
  double dot_hz;
  int c = scanout_crtc();

  if (!disp_ext && (c == 0)) {
    bx_vgacore_c::get_crtc_params(crtcp, vclock);
    dot_hz = dot_clock_hz();
    if ((pll_regs[R9800_PLL_VCLK_ECP_CNTL] & R9800_VCLK_SRC_SEL_MASK) == R9800_VCLK_SRC_PPLL) {
      if (dot_hz >= 1000000.0) {
        *vclock = (Bit32u)dot_hz;
        if (BX_R9800_THIS s.x_dotclockdiv2) *vclock >>= 1;
      }
    }
    // The VGA clock select can name a source the core has no frequency
    // for, but the card always drives a pixel clock. Handing back zero
    // makes the core drop the retrace timing, which stops the vertical
    // blank everything else is sequenced from.
    if (*vclock == 0)
      *vclock = (Bit32u)((dot_hz >= 1000000.0) ? dot_hz : ref_freq_hz);
    return;
  }
  dot_hz = dot_clock_hz();
  if (dot_hz < 1000000.0)
    dot_hz = ref_freq_hz;
  *vclock = (Bit32u)dot_hz;

  Bit32u gen = c ? crtc2_gen_cntl : crtc_gen_cntl;
  Bit32u htotal = (crtc[c].h_total_disp & 0x3ff) + 1;
  Bit32u cwidth = ((BX_R9800_THIS s.sequencer.reg1 & 0x01) == 1) ? 8 : 9;
  if (cwidth == 9)
    htotal = (htotal * 8 + 4) / 9;
  Bit32u vtotal = (crtc[c].v_total_disp & 0x7ff) + 1;
  Bit32u vdisp = (crtc[c].v_disp_active & 0x7ff) + 1;
  Bit32u vsync = (crtc[c].v_sync_strt_wid & 0x7ff) + 1;
  if (gen & R9800_CRTC_INTERLACE_EN) {
    vtotal = (vtotal + 1) >> 1;
    vdisp = (vdisp + 1) >> 1;
    vsync = (vsync + 1) >> 1;
  }
  if (htotal < 8) htotal = 8;
  if (vtotal < 8) vtotal = 8;
  crtcp->htotal = (Bit16u)htotal;
  crtcp->vtotal = (Bit16u)vtotal;
  crtcp->vbstart = (Bit16u)vdisp;
  crtcp->vrstart = (Bit16u)vsync;
}

// ---------------------------------------------------------------------
// Scanout
// ---------------------------------------------------------------------

void bx_radeon9800_c::redraw_area(unsigned x0, unsigned y0, unsigned width, unsigned height)
{
  redraw_area((Bit32s)x0, (Bit32s)y0, width, height);
}

void bx_radeon9800_c::redraw_area(Bit32s x0, Bit32s y0, Bit32u width, Bit32u height)
{
  unsigned xti, yti, xt0, xt1, yt0, yt1;

  if (!disp_ext) {
    bx_vgacore_c::redraw_area(x0, y0, width, height);
    return;
  }
  if ((x0 + (Bit32s)width <= 0) || (y0 + (Bit32s)height <= 0))
    return;
  needs_update_tile = true;
  xt0 = x0 <= 0 ? 0 : x0 / X_TILESIZE;
  yt0 = y0 <= 0 ? 0 : y0 / Y_TILESIZE;
  if (x0 < (Bit32s)disp_xres) {
    xt1 = (x0 + width - 1) / X_TILESIZE;
  } else {
    xt1 = (disp_xres - 1) / X_TILESIZE;
  }
  if (y0 < (Bit32s)disp_yres) {
    yt1 = (y0 + height - 1) / Y_TILESIZE;
  } else {
    yt1 = (disp_yres - 1) / Y_TILESIZE;
  }
  for (yti = yt0; yti <= yt1; yti++) {
    for (xti = xt0; xti <= xt1; xti++) {
      SET_TILE_UPDATED(BX_R9800_THIS, xti, yti, 1);
    }
  }
}

// Derive the scanout geometry from the extended CRTC registers
void bx_radeon9800_c::update_mode(void)
{
  vga_banked_mode = false;
  int c = scanout_crtc();
  Bit32u gen = c ? crtc2_gen_cntl : crtc_gen_cntl;
  unsigned pix_width = (gen >> R9800_CRTC_PIX_WIDTH_SHIFT) & 0xf;
  unsigned bpp;

  disp_crtc = c;
  switch (pix_width) {
    case 1: bpp = 4; break;
    case 2: bpp = 8; break;
    case 3: bpp = 15; break;
    case 4: bpp = 16; break;
    case 5: bpp = 24; break;
    case 6: bpp = 32; break;
    default: bpp = 0; break;
  }
  unsigned xres = (((crtc[c].h_total_disp >> 16) & 0x1ff) + 1) * 8;
  unsigned yres = (crtc[c].v_disp_active & 0x7ff) + 1;
  Bit32u pitch = (crtc[c].pitch & 0x7ff) * 8;
  switch (bpp) {
    case 4:  pitch >>= 1; break;
    case 8:  break;
    case 15:
    case 16: pitch <<= 1; break;
    case 24: pitch *= 3; break;
    case 32: pitch <<= 2; break;
    default: break;
  }
  disp_dblscan = (gen & R9800_CRTC_DBL_SCAN_EN) != 0;
  if (c == 0) {
    disp_blank = (bpp == 0) || !(gen & R9800_CRTC_EN) || (gen & R9800_CRTC_DISP_REQ_EN_B) ||
                 ((crtc_ext_cntl & R9800_CRTC_DISPLAY_DIS) && !(dac_ext_cntl & R9800_DAC_FORCE_BLANK_OFF_EN));
  } else {
    disp_blank = (bpp == 0) || !(gen & R9800_CRTC_EN) || (gen & R9800_CRTC_DISP_REQ_EN_B) ||
                 (gen & R9800_CRTC2_DISP_DIS);
  }
  disp_dac_const = false;
  disp_dac_const_color = 0;
  if (disp_output == R9800_OUT_TVDAC) {
    // TV DAC output: blanked while NBLANK is low
    if (!(tv_dac_cntl & R9800_TV_DAC_NBLANK) && !disp_blank)
      disp_dac_const = true;
  } else if (disp_output != R9800_OUT_DAC1) {
    // TMDS outputs carry the CRTC pixels unblanked
  } else if ((dac_cntl & R9800_DAC_PDWN) || ((dac_cntl & R9800_DAC_BLANKING) && !disp_blank)) {
    disp_dac_const = true;
  } else if ((dac_ext_cntl & R9800_DAC_FORCE_DATA_EN) && !disp_blank) {
    Bit32u v = (dac_ext_cntl >> R9800_DAC_FORCE_DATA_SHIFT) & 0x3ff;
    Bit8u v8 = (Bit8u)(v >> 2);
    disp_dac_const = true;
    switch ((dac_ext_cntl >> R9800_DAC_FORCE_DATA_SEL_SHIFT) & 3) {
      case 0: disp_dac_const_color = (Bit32u)v8 << 16; break;
      case 1: disp_dac_const_color = (Bit32u)v8 << 8; break;
      case 2: disp_dac_const_color = v8; break;
      default: disp_dac_const_color = ((Bit32u)v8 << 16) | ((Bit32u)v8 << 8) | v8; break;
    }
  }
  if (bpp == 0) bpp = 8;
  if (xres < 8) xres = 8;
  if (yres < 1) yres = 1;
  // 320-wide double-scanned modes: pixel-doubled to 640 by the CRTC
  disp_hdbl = (xres == 320) && (yres >= 400);
  if (disp_hdbl) xres = 640;
  if (xres > BX_R9800_THIS s.max_xres) xres = BX_R9800_THIS s.max_xres;
  if (yres > BX_R9800_THIS s.max_yres) yres = BX_R9800_THIS s.max_yres;
  if (pitch == 0) pitch = xres * ((bpp + 7) / 8);

  if ((xres != disp_xres) || (yres != disp_yres) || (bpp != disp_bpp)) {
    BX_INFO(("CRTC%d switched to %u x %u x %u", c + 1, xres, yres, bpp));
  }
  disp_xres = xres;
  disp_yres = yres;
  disp_bpp = bpp;
  disp_pitch = pitch;
  // scanout address = DISPLAY_BASE_ADDR + CRTC_OFFSET (MC space)
  crtc_settle(c);
  Bit32u mc = crtc[c].display_base + (crtc[c].offset_latched & 0x0fffffff);
  Bit32u off;
  if (!mc_is_vram(mc, &off))
    off = mc & vram_mask;
  disp_base = off;
  // Tiled display surface (R300 macro / micro tiling with X_Y origin mode)
  Bit32u oc = crtc[c].offset_cntl;
  scanout_tiled = (oc & R9800_CRTC_MACRO_TILE_EN) != 0;
  scanout_macro = scanout_tiled ? 1 : 0;
  scanout_micro = (oc & R9800_CRTC_MICRO_TILE_EN) ? 1 : 0;
  if (scanout_micro && !scanout_tiled) scanout_tiled = true;
  scanout_x0 = scanout_y0 = 0;
  if (scanout_tiled && (oc & R9800_CRTC_X_Y_MODE_EN)) {
    scanout_x0 = crtc[c].tile_x0_y0 & 0xfff;
    scanout_y0 = (crtc[c].tile_x0_y0 >> 16) & 0xfff;
  }
  BX_R9800_THIS s.last_xres = xres;
  BX_R9800_THIS s.last_yres = yres;
  BX_R9800_THIS s.last_bpp = (bpp == 15) ? 16 : bpp;
  BX_R9800_THIS s.last_fh = 0;
  ov0_update();
}

// VRAM byte address of display pixel (sx, sy); xoff = byte offset of the
// pixel inside its scanline for the linear case
Bit32u bx_radeon9800_c::scanout_addr(Bit32u sy, Bit32u sx, Bit32u xoff)
{
  if (scanout_tiled) {
    Bit32u bpp = (disp_bpp + 7) / 8;
    if (disp_bpp == 4) bpp = 1;
    return (disp_base + r300_tile_addr(scanout_x0 + sx, scanout_y0 + sy, disp_pitch, bpp,
                                       scanout_micro, scanout_macro)) & vram_mask;
  }
  return (disp_base + sy * disp_pitch + xoff) & vram_mask;
}

// Hardware cursor: mono 64x64 (16 bytes per line = 8 byte AND mask then
// 8 byte XOR mask, MSB first) or 32-bit ARGB 64x64 (CUR_MODE 2, alpha
// blended over the frame).
void bx_radeon9800_c::draw_hardware_cursor(unsigned xc, unsigned yc, bx_svga_tileinfo_t *info)
{
  if (!cur_lat_en || disp_dac_const || disp_blank)
    return;

  int posx = (cur_lat_posn >> 16) & 0x7ff;
  int posy = cur_lat_posn & 0x7ff;
  int xoff = (cur_lat_hvoff >> 16) & 0x3f;
  int yoff = cur_lat_hvoff & 0x3f;
  int cw = 64 - xoff;
  int ch = 64 - yoff;
  if ((cw <= 0) || (ch <= 0))
    return;
  Bit32u base = cur_lat_offset;
  Bit32u col0 = cur_lat_clr0;
  Bit32u col1 = cur_lat_clr1;
  bool argb = (cur_lat_mode == 2);

  unsigned w, h;
  Bit8u *tile_ptr;
  if (info->snapshot_mode) {
    tile_ptr = bx_gui->get_snapshot_buffer();
    w = disp_xres;
    h = disp_yres;
  } else {
    tile_ptr = bx_gui->graphics_tile_get(xc, yc, &w, &h);
  }
  int tx0 = (int)xc, ty0 = (int)yc, tx1 = (int)(xc + w), ty1 = (int)(yc + h);
  int cx0 = posx, cy0 = posy, cx1 = posx + cw, cy1 = posy + ch;
  if (cx0 < tx0) cx0 = tx0;
  if (cy0 < ty0) cy0 = ty0;
  if (cx1 > tx1) cx1 = tx1;
  if (cy1 > ty1) cy1 = ty1;
  if ((cx0 >= cx1) || (cy0 >= cy1))
    return;
  if (info->bpp == 15) info->bpp = 16;
  unsigned tbytes = info->bpp / 8;
  Bit8u *vram = BX_R9800_THIS s.memory;

  for (int y = cy0; y < cy1; y++) {
    int row = (y - posy) + yoff;
    Bit8u *tp = tile_ptr + info->pitch * (y - ty0) + tbytes * (cx0 - tx0);
    for (int x = cx0; x < cx1; x++) {
      int xx = (x - posx) + xoff;
      Bit32u colour;
      bool draw = false, invert = false;
      Bit32u alpha = 255;
      if (argb) {
        Bit32u a = (base + (Bit32u)row * 256u + (Bit32u)xx * 4u) & vram_mask;
        colour = vram[a] | ((Bit32u)vram[(a + 1) & vram_mask] << 8) |
                 ((Bit32u)vram[(a + 2) & vram_mask] << 16) | ((Bit32u)vram[(a + 3) & vram_mask] << 24);
        alpha = colour >> 24;
        draw = (alpha != 0);
      } else {
        Bit32u laddr = base + (Bit32u)row * 16u;
        int a = (vram[(laddr + (xx >> 3)) & vram_mask] >> (7 - (xx & 7))) & 1;
        int sbit = (vram[(laddr + 8 + (xx >> 3)) & vram_mask] >> (7 - (xx & 7))) & 1;
        if (!a) { draw = true; colour = sbit ? col1 : col0; }
        else if (sbit) { invert = true; colour = 0; }
        else colour = 0;
      }
      if (invert || (draw && (alpha != 255) && !info->is_indexed)) {
        // read back the pixel currently in the tile
        Bit32u cur = 0;
        if (info->is_little_endian) {
          for (unsigned i = 0; i < tbytes; i++) cur |= (Bit32u)tp[i] << (i * 8);
        } else {
          for (unsigned i = 0; i < tbytes; i++) cur = (cur << 8) | tp[i];
        }
        if (invert) {
          colour = ~cur;
          if (info->is_indexed) {
            tp[0] = (Bit8u)colour;
            tp += tbytes;
            continue;
          }
          if (info->is_little_endian) {
            for (unsigned i = 0; i < tbytes; i++) *(tp++) = (Bit8u)(colour >> (i * 8));
          } else {
            for (int i = (int)tbytes - 1; i >= 0; i--) *(tp++) = (Bit8u)(colour >> (i * 8));
          }
          continue;
        }
        // alpha blend the ARGB cursor over the host pixel
        Bit32u hr = (cur & info->red_mask) >> info->red_shift;
        Bit32u hg = (cur & info->green_mask) >> info->green_shift;
        Bit32u hb = (cur & info->blue_mask) >> info->blue_shift;
        int rb = 0, gb = 0, bb = 0;
        for (Bit32u m = info->red_mask >> info->red_shift; m; m >>= 1) rb++;
        for (Bit32u m = info->green_mask >> info->green_shift; m; m >>= 1) gb++;
        for (Bit32u m = info->blue_mask >> info->blue_shift; m; m >>= 1) bb++;
        Bit32u hr8 = rb ? (hr << (8 - rb)) : 0, hg8 = gb ? (hg << (8 - gb)) : 0, hb8 = bb ? (hb << (8 - bb)) : 0;
        Bit32u cr = (colour >> 16) & 0xff, cg = (colour >> 8) & 0xff, cb = colour & 0xff;
        cr = (cr * alpha + hr8 * (255 - alpha)) / 255;
        cg = (cg * alpha + hg8 * (255 - alpha)) / 255;
        cb = (cb * alpha + hb8 * (255 - alpha)) / 255;
        colour = (cr << 16) | (cg << 8) | cb;
      }
      if (draw) {
        if (!info->is_indexed) {
          colour = MAKE_COLOUR(
            colour, 24, info->red_shift, info->red_mask,
            colour, 16, info->green_shift, info->green_mask,
            colour, 8, info->blue_shift, info->blue_mask);
          if (info->is_little_endian) {
            for (int i = 0; i < info->bpp; i += 8) *(tp++) = (Bit8u)(colour >> i);
          } else {
            for (int i = info->bpp - 8; i > -8; i -= 8) *(tp++) = (Bit8u)(colour >> i);
          }
        } else {
          *(tp++) = (Bit8u)colour;
        }
      } else {
        tp += tbytes;
      }
    }
  }
}

// Fetch one framebuffer pixel as 0x00RRGGBB
static BX_CPP_INLINE Bit32u radeon9800_fetch_pixel(const Bit8u *p, unsigned bpp, unsigned x, bool lsb_nibble, Bit8u *pel8, Bit8u dac_shift)
{
  Bit32u v;
  switch (bpp) {
    case 4: {
      Bit8u b = p[0];
      Bit8u idx = ((x & 1) ^ (lsb_nibble ? 1 : 0)) ? (b & 0x0f) : (b >> 4);
      Bit8u *pe = pel8 + idx * 3;
      return ((Bit32u)(pe[0] << dac_shift) << 16) | ((Bit32u)(pe[1] << dac_shift) << 8) | (Bit32u)(pe[2] << dac_shift);
    }
    case 8: {
      Bit8u *pe = pel8 + p[0] * 3;
      return ((Bit32u)(pe[0] << dac_shift) << 16) | ((Bit32u)(pe[1] << dac_shift) << 8) | (Bit32u)(pe[2] << dac_shift);
    }
    case 15:
      v = p[0] | (p[1] << 8);
      return (((v >> 10) & 0x1f) << 19) | (((v >> 10) & 0x1c) << 14) |
             (((v >> 5) & 0x1f) << 11) | (((v >> 5) & 0x1c) << 6) |
             ((v & 0x1f) << 3) | ((v & 0x1c) >> 2);
    case 16:
      v = p[0] | (p[1] << 8);
      return (((v >> 11) & 0x1f) << 19) | (((v >> 11) & 0x1c) << 14) |
             (((v >> 5) & 0x3f) << 10) | (((v >> 5) & 0x30) << 4) |
             ((v & 0x1f) << 3) | ((v & 0x1c) >> 2);
    case 24:
      return p[0] | (p[1] << 8) | (p[2] << 16);
    default:
      return p[0] | (p[1] << 8) | (p[2] << 16);
  }
}

void bx_radeon9800_c::update(void)
{
  unsigned width, height;

  fold_deferred();
  // Routing and mode state can change through registers that are not
  // themselves display registers (indexed writes, ring packets); re-derive
  // it here so the scanned-out CRTC is never stale.
  scanout_refresh();
  if (!disp_ext && (scanout_crtc() == 0)) {
    if (ext_last) {
      ext_last = false;
      gui_xres = gui_yres = gui_bpp = 0;
      BX_R9800_THIS s.last_xres = 0;
      BX_R9800_THIS s.last_yres = 0;
      BX_R9800_THIS s.vga_mem_updated |= 1;
      bx_vgacore_c::redraw_area(0, 0, 4096, 4096);
    }
    bx_vgacore_c::update();
    return;
  }
  ext_last = true;

  if (needs_update_mode) {
    update_mode();
    unsigned bpp = (disp_bpp == 15) ? 16 : ((disp_bpp == 4) ? 8 : disp_bpp);
    if ((disp_xres != gui_xres) || (disp_yres != gui_yres) || (bpp != gui_bpp)) {
      bx_gui->dimension_update(disp_xres, disp_yres, 0, 0, bpp);
      gui_xres = disp_xres;
      gui_yres = disp_yres;
      gui_bpp = bpp;
    }
    needs_update_mode = false;
    needs_update_dispentire = true;
  }
  width = disp_xres;
  height = disp_yres;

  if (scanout_tiled)
    needs_update_dispentire = true;
  if (needs_update_dispentire) {
    redraw_area(0, 0, width, height);
    needs_update_dispentire = false;
  }
  if (!needs_update_tile)
    return;
  needs_update_tile = false;

  unsigned xc, yc, xti, yti, w, h;
  int i;
  Bit32u colour;
  Bit8u *tile_ptr, *tile_ptr2;
  bx_svga_tileinfo_t info;
  Bit8u dac_shift = (disp_crtc == 1) ? 0 : BX_R9800_THIS s.dac_shift;
  Bit8u pel8[256 * 3];
  for (i = 0; i < 256; i++) {
    if (disp_crtc == 1) {
      pel8[i * 3 + 0] = (Bit8u)(palette2[i] >> 16);
      pel8[i * 3 + 1] = (Bit8u)(palette2[i] >> 8);
      pel8[i * 3 + 2] = (Bit8u)palette2[i];
    } else {
      pel8[i * 3 + 0] = BX_R9800_THIS s.pel.data[i].red;
      pel8[i * 3 + 1] = BX_R9800_THIS s.pel.data[i].green;
      pel8[i * 3 + 2] = BX_R9800_THIS s.pel.data[i].blue;
    }
  }
  unsigned pxbytes = (disp_bpp + 7) / 8;
  bool lsb_nibble = (dac_cntl & R9800_DAC_4BPP_PIX_ORDER) != 0;
  Bit8u *vram = BX_R9800_THIS s.memory;

  if (!bx_gui->graphics_tile_info_common(&info)) {
    BX_PANIC(("cannot get svga tile info"));
    return;
  }
  if (info.snapshot_mode) {
    tile_ptr = bx_gui->get_snapshot_buffer();
    if (tile_ptr == NULL)
      return;
    for (yc = 0; yc < height; yc++) {
      unsigned sy = disp_dblscan ? (yc >> 1) : yc;
      tile_ptr2 = tile_ptr;
      for (xc = 0; xc < width; xc++) {
        unsigned sx = disp_hdbl ? (xc >> 1) : xc;
        Bit32u xoff = (disp_bpp == 4) ? (sx >> 1) : sx * pxbytes;
        Bit32u paddr = scanout_addr(sy, sx, xoff);
        if (disp_blank) colour = 0;
        else if (disp_dac_const) colour = disp_dac_const_color;
        else colour = radeon9800_fetch_pixel(&vram[paddr], disp_bpp, sx, lsb_nibble, pel8, dac_shift);
        if (!info.is_indexed) {
          colour = MAKE_COLOUR(colour, 24, info.red_shift, info.red_mask,
                               colour, 16, info.green_shift, info.green_mask,
                               colour, 8, info.blue_shift, info.blue_mask);
          if (info.is_little_endian) {
            for (i = 0; i < info.bpp; i += 8) *(tile_ptr2++) = (Bit8u)(colour >> i);
          } else {
            for (i = info.bpp - 8; i > -8; i -= 8) *(tile_ptr2++) = (Bit8u)(colour >> i);
          }
        } else {
          *(tile_ptr2++) = (disp_bpp == 8) ? vram[paddr] : (Bit8u)colour;
        }
      }
      tile_ptr += info.pitch;
    }
    draw_overlay(0, 0, &info);
    draw_hardware_cursor(0, 0, &info);
    return;
  }

  if (info.is_indexed && (disp_bpp != 8) && (disp_bpp != 4)) {
    BX_ERROR(("current guest pixel format is unsupported on indexed colour host displays, bpp=%d", disp_bpp));
    return;
  }

  for (yc = 0, yti = 0; yc < height; yc += Y_TILESIZE, yti++) {
    for (xc = 0, xti = 0; xc < width; xc += X_TILESIZE, xti++) {
      if (!GET_TILE_UPDATED(xti, yti))
        continue;
      paint_tile(xc, yc, &info, pel8);
      bx_gui->graphics_tile_get(xc, yc, &w, &h);
      bx_gui->graphics_tile_update_in_place(xc, yc, w, h);
      SET_TILE_UPDATED(BX_R9800_THIS, xti, yti, 0);
    }
  }
}

// Render one display tile (framebuffer, then overlay and cursor on top)
void bx_radeon9800_c::paint_tile(unsigned xc, unsigned yc, bx_svga_tileinfo_t *info, Bit8u *pel8)
{
  unsigned w, h, r, c;
  int i;
  Bit32u colour;
  Bit8u *tile_ptr, *tile_ptr2;
  Bit8u dac_shift = (disp_crtc == 1) ? 0 : BX_R9800_THIS s.dac_shift;
  unsigned pxbytes = (disp_bpp + 7) / 8;
  bool lsb_nibble = (dac_cntl & R9800_DAC_4BPP_PIX_ORDER) != 0;
  Bit8u *vram = BX_R9800_THIS s.memory;

  tile_ptr = bx_gui->graphics_tile_get(xc, yc, &w, &h);
  for (r = 0; r < h; r++) {
    unsigned y = yc + r;
    unsigned sy = disp_dblscan ? (y >> 1) : y;
    tile_ptr2 = tile_ptr;
    for (c = 0; c < w; c++) {
      unsigned x = xc + c;
      unsigned sx = disp_hdbl ? (x >> 1) : x;
      Bit32u xoff = (disp_bpp == 4) ? (sx >> 1) : sx * pxbytes;
      Bit32u paddr = scanout_addr(sy, sx, xoff);
      if (disp_blank) colour = 0;
      else if (disp_dac_const) colour = disp_dac_const_color;
      else colour = radeon9800_fetch_pixel(&vram[paddr], disp_bpp, sx, lsb_nibble, pel8, dac_shift);
      if (info->is_indexed) {
        *(tile_ptr2++) = (disp_bpp == 8) ? vram[paddr] : (Bit8u)colour;
      } else {
        colour = MAKE_COLOUR(colour, 24, info->red_shift, info->red_mask,
                             colour, 16, info->green_shift, info->green_mask,
                             colour, 8, info->blue_shift, info->blue_mask);
        if (info->is_little_endian) {
          for (i = 0; i < info->bpp; i += 8) *(tile_ptr2++) = (Bit8u)(colour >> i);
        } else {
          for (i = info->bpp - 8; i > -8; i -= 8) *(tile_ptr2++) = (Bit8u)(colour >> i);
        }
      }
    }
    tile_ptr += info->pitch;
  }
  draw_overlay(xc, yc, info);
  draw_hardware_cursor(xc, yc, info);
}

#if BX_DEBUGGER
void bx_radeon9800_c::debug_dump(int argc, char **argv)
{
  dbg_printf("ATI Radeon 9800 (R350, %s, %u MB, asic rev %d)\n", is_agp ? "AGP" : "PCI", vram_size >> 20, asic_rev);
  if (disp_ext) {
    dbg_printf("extended mode (CRTC%d): %u x %u x %u, pitch %u, base 0x%08x%s\n", disp_crtc + 1, disp_xres, disp_yres,
               disp_bpp, disp_pitch, disp_base, scanout_tiled ? " tiled" : "");
    dbg_printf("CRTC_GEN_CNTL=0x%08x CRTC_EXT_CNTL=0x%08x CRTC2_GEN_CNTL=0x%08x DAC_CNTL=0x%08x\n",
               crtc_gen_cntl, crtc_ext_cntl, crtc2_gen_cntl, dac_cntl);
    dbg_printf("H_TOTAL_DISP=0x%08x V_TOTAL_DISP=0x%08x dot clock %.2f MHz\n",
               crtc[disp_crtc].h_total_disp, crtc[disp_crtc].v_total_disp, dot_clock_hz() / 1e6);
  } else {
    dbg_printf("VGA mode\n");
  }
  dbg_printf("MC_FB_LOCATION=0x%08x MC_AGP_LOCATION=0x%08x AGP_BASE=0x%08x AIC_CNTL=0x%08x\n",
             mc_fb_location, mc_agp_location, agp_base, aic_cntl);
  dbg_printf("GEN_INT_CNTL=0x%08x GEN_INT_STATUS=0x%08x\n", gen_int_cntl, gen_int_status);
  dbg_printf("CP: RB_BASE=0x%08x RB_CNTL=0x%08x RPTR=0x%08x WPTR=0x%08x CSQ_CNTL=0x%08x\n",
             cp_rb_base, cp_rb_cntl, cp_rb_rptr, cp_rb_wptr, cp_csq_cntl);
  dbg_printf("CP: fifo rd=%u wr=%u executing=%d ind_busy=%d batch_pending=%d retire=0x%08x\n",
             cp_fifo_rd, cp_fifo_wr, (int)cp_executing, (int)pm4_ind_busy, (int)cp_batch_pending, cp_retire_rptr);
  dbg_printf("2D: GMC=0x%08x DST_OFFSET=0x%08x DST_PITCH=%u SRC_OFFSET=0x%08x SRC_PITCH=%u DP_DATATYPE=0x%08x DP_MIX=0x%08x\n",
             dp_gui_master_cntl, dst_offset, dst_pitch, src_offset, src_pitch, dp_datatype, dp_mix);
  dbg_printf("3D: VAP_CNTL=0x%08x VF_CNTL=0x%08x OUT_VTX_FMT=0x%08x/0x%08x PVS_CODE_CNTL_0=0x%08x\n",
             vap_regs[(R9800_VAP_CNTL - R9800_VAP_BASE) >> 2], vap_regs[(R9800_VAP_VF_CNTL - R9800_VAP_BASE) >> 2],
             vap_regs[(R9800_VAP_OUT_VTX_FMT_0 - R9800_VAP_BASE) >> 2], vap_regs[(R9800_VAP_OUT_VTX_FMT_1 - R9800_VAP_BASE) >> 2],
             vap_regs[(R9800_VAP_PVS_CODE_CNTL_0 - R9800_VAP_BASE) >> 2]);
  dbg_printf("3D: COLOROFFSET0=0x%08x COLORPITCH0=0x%08x DEPTHOFFSET=0x%08x DEPTHPITCH=0x%08x ZB_CNTL=0x%08x\n",
             r3d_regs[R3D(R9800_RB3D_COLOROFFSET0)], r3d_regs[R3D(R9800_RB3D_COLORPITCH0)],
             r3d_regs[R3D(R9800_ZB_DEPTHOFFSET)], r3d_regs[R3D(R9800_ZB_DEPTHPITCH)], r3d_regs[R3D(R9800_ZB_CNTL)]);
  dbg_printf("3D: US_CONFIG=0x%08x US_PIXSIZE=0x%08x US_CODE_ADDR=0x%08x,0x%08x,0x%08x,0x%08x TX_ENABLE=0x%08x\n",
             r3d_regs[R3D(R9800_US_CONFIG)], r3d_regs[R3D(R9800_US_PIXSIZE)],
             r3d_regs[R3D(R9800_US_CODE_ADDR_0)], r3d_regs[R3D(R9800_US_CODE_ADDR_0) + 1],
             r3d_regs[R3D(R9800_US_CODE_ADDR_0) + 2], r3d_regs[R3D(R9800_US_CODE_ADDR_0) + 3],
             r3d_regs[R3D(R9800_TX_ENABLE)]);
  dbg_printf("OV0: enabled=%d SCALE_CNTL=0x%08x LOAD_CNTL=0x%08x\n", (int)ov0_enabled,
             ov0.active[R9800_OV0_REG(R9800_OV0_SCALE_CNTL)], ov0_reg_load_cntl);

  // "info device radeon9800 vram <file>": write the whole VRAM image to a file
  if ((argc >= 2) && !strcmp(argv[0], "vram")) {
    FILE *fp = fopen(argv[1], "wb");
    if (fp == NULL) {
      dbg_printf("cannot create '%s'\n", argv[1]);
    } else {
      size_t n = fwrite(BX_R9800_THIS s.memory, 1, vram_size, fp);
      fclose(fp);
      dbg_printf("wrote %u bytes of VRAM to '%s'\n", (unsigned)n, argv[1]);
    }
    return;
  }
  // "info device radeon9800 mc,<hexaddr>,<ndwords>": resolve an MC address
  // exactly as the CP / vertex fetch does and print the dwords
  if ((argc >= 3) && !strcmp(argv[0], "mc")) {
    Bit32u addr = (Bit32u)strtoul(argv[1], NULL, 16);
    Bit32u n = (Bit32u)strtoul(argv[2], NULL, 0);
    if (n > 64) n = 64;
    for (Bit32u k = 0; k < n; k++) {
      Bit32u v = 0xdeadbeef;
      bool ok = gpu_read32(addr + k * 4, &v);
      float f;
      memcpy(&f, &v, 4);
      dbg_printf("  [%08x] %08x  %g%s\n", addr + k * 4, v, f, ok ? "" : "  (dead)");
    }
    return;
  }
  // "info device radeon9800 fifo,<file>": dump the CP FIFO
  if ((argc >= 2) && !strcmp(argv[0], "fifo")) {
    FILE *fp = fopen(argv[1], "wb");
    if ((fp == NULL) || (cp_fifo == NULL)) {
      dbg_printf("cannot create '%s' or no FIFO\n", argv[1]);
      if (fp) fclose(fp);
    } else {
      fwrite(cp_fifo, sizeof(Bit32u), R9800_CP_FIFO_DWORDS, fp);
      fwrite(cp_fifo_tag, 1, R9800_CP_FIFO_DWORDS, fp);
      fclose(fp);
      dbg_printf("wrote FIFO (%u dwords + tags) to '%s'\n", R9800_CP_FIFO_DWORDS, argv[1]);
    }
    return;
  }
  // "info device radeon9800 reg,<hexoff>": read one register
  if ((argc >= 2) && !strcmp(argv[0], "reg")) {
    Bit32u off = (Bit32u)strtoul(argv[1], NULL, 16);
    dbg_printf("  [%04x] = %08x\n", off & 0xfffc, reg_read32(off));
    return;
  }
  if (argc > 0) {
    bx_vgacore_c::debug_dump(argc, argv);
  }
}
#endif

#endif // BX_SUPPORT_PCI && BX_SUPPORT_RADEON9800
