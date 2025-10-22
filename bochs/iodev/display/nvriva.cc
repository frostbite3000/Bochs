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
#define BX_USE_BINARY_FWD_ROP
#define BX_USE_TERNARY_ROP
#include "bitblt.h"
#include "ddc.h"
#include "pxextract.h"
#include "nvriva.h"
#include "virt_timer.h"

#include "bx_debug/debug.h"

#include <math.h>

#if BX_SUPPORT_NVRIVA

#define LOG_THIS BX_NVRIVA_THIS

#if BX_USE_NVRIVA_SMF
#define VGA_READ(addr,len)       bx_vgacore_c::read_handler(theSvga,addr,len)
#define VGA_WRITE(addr,val,len)  bx_vgacore_c::write_handler(theSvga,addr,val,len)
#define SVGA_READ(addr,len)      svga_read_handler(theSvga,addr,len)
#define SVGA_WRITE(addr,val,len) svga_write_handler(theSvga,addr,val,len)
#else
#define VGA_READ(addr,len)       bx_vgacore_c::read(addr,len)
#define VGA_WRITE(addr,val,len)  bx_vgacore_c::write(addr,val,len)
#define SVGA_READ(addr,len)      svga_read(addr,len)
#define SVGA_WRITE(addr,val,len) svga_write(addr,val,len)
#endif // BX_USE_NVRIVA_SMF

#define NVRIVA_PNPMMIO_SIZE        0x1000000

#define ALIGN(x, a) (((x) + (a) - 1) & ~((a) - 1))

static bx_nvriva_c *theSvga = NULL;

/* enumeration specifying which model of NVidia RIVA we are emulating */
enum
{
    RIVA_TNT,
    RIVA_TNT2_M64,
    MAX_NVRIVA_TYPES
};


void nvriva_init_options(void)
{
  static const char* nvriva_model_list[] = {
    "rivatnt",
    "rivatnt2m64",
    NULL
  };

  bx_param_c *display = SIM->get_param("display");
  bx_list_c *menu = new bx_list_c(display, "nvriva", "NVRiva");
  menu->set_options(menu->SHOW_PARENT);
  new bx_param_enum_c(menu,
    "model",
    "NVidia RIVA model",
    "Selects the NVidia RIVA model to emulate.",
    nvriva_model_list,
    RIVA_TNT, RIVA_TNT);
}

Bit32s nvriva_options_parser(const char *context, int num_params, char *params[])
{
  if (!strcmp(params[0], "nvriva")) {
    bx_list_c *base = (bx_list_c*) SIM->get_param(BXPN_NVRIVA);
    for (int i = 1; i < num_params; i++) {
      if (SIM->parse_param_from_list(context, params[i], base) < 0) {
        BX_ERROR(("%s: unknown parameter for nvriva ignored.", context));
      }
    }
  } else {
    BX_PANIC(("%s: unknown directive '%s'", context, params[0]));
  }
  return 0;
}

Bit32s nvriva_options_save(FILE *fp)
{
  return SIM->write_param_list(fp, (bx_list_c*) SIM->get_param(BXPN_NVRIVA), NULL, 0);
}

PLUGIN_ENTRY_FOR_MODULE(nvriva)
{
  if (mode == PLUGIN_INIT) {
    theSvga = new bx_nvriva_c();
    bx_devices.pluginVgaDevice = theSvga;
    BX_REGISTER_DEVICE_DEVMODEL(plugin, type, theSvga, BX_PLUGIN_NVRIVA);
    // add new configuration parameter for the config interface
    nvriva_init_options();
    // register add-on option for bochsrc and command line
    SIM->register_addon_option("nvriva", nvriva_options_parser, nvriva_options_save);
  } else if (mode == PLUGIN_FINI) {
    SIM->unregister_addon_option("nvriva");
    bx_list_c *menu = (bx_list_c*)SIM->get_param("display");
    menu->remove("nvriva");
    delete theSvga;
  } else if (mode == PLUGIN_PROBE) {
    return (int)PLUGTYPE_VGA;
  } else if (mode == PLUGIN_FLAGS) {
    return PLUGFLAG_PCI;
  }
  return 0; // Success
}

bx_nvriva_c::bx_nvriva_c() : bx_vgacore_c()
{
  // nothing else to do
}

bx_nvriva_c::~bx_nvriva_c()
{
  SIM->get_bochs_root()->remove("nvriva");
  BX_DEBUG(("Exit"));
}

bool bx_nvriva_c::init_vga_extension(void)
{
  // Read in values from config interface
  bx_list_c* base = (bx_list_c*)SIM->get_param(BXPN_NVRIVA);
  Bit32s model_enum = (Bit8u)SIM->get_param_enum("model", base)->get();

  const char* model_string;
  if (model_enum == RIVA_TNT) {
    BX_NVRIVA_THIS card_type = 0x04;
    model_string = "RIVA TNT";
  } else if (model_enum == RIVA_TNT2_M64) {
    BX_NVRIVA_THIS card_type = 0x05;
    model_string = "RIVA TNT2 Model 64";
  } else {
    model_string = nullptr;
    BX_NVRIVA_THIS card_type = 0x00;
    BX_PANIC(("Unknown card type"));
  }

  if (!SIM->is_agp_device(BX_PLUGIN_NVRIVA)) {
    BX_PANIC(("%s should be plugged into AGP slot", model_string));
  }

  BX_NVRIVA_THIS pci_enabled = true;

  BX_NVRIVA_THIS put("NVRIVA");
  // initialize SVGA stuffs.
  BX_NVRIVA_THIS init_iohandlers(svga_read_handler, svga_write_handler, "nvriva");
  DEV_register_ioread_handler(this, svga_read_handler, 0x03B4, "nvriva", 2);
  DEV_register_ioread_handler(this, svga_read_handler, 0x03D0, "nvriva", 7);
  DEV_register_ioread_handler(this, svga_read_handler, 0x03D2, "nvriva", 2);
  DEV_register_ioread_handler(this, svga_read_handler, 0x03C3, "nvriva", 3);
  DEV_register_iowrite_handler(this, svga_write_handler, 0x03D0, "nvriva", 7);
  DEV_register_iowrite_handler(this, svga_write_handler, 0x03D2, "nvriva", 2);
  BX_NVRIVA_THIS svga_init_members();
  BX_NVRIVA_THIS svga_init_pcihandlers();
  BX_NVRIVA_THIS bitblt_init();
  BX_NVRIVA_THIS s.CRTC.max_reg = NVRIVA_CRTC_MAX;
  BX_NVRIVA_THIS s.max_xres = 2048;
  BX_NVRIVA_THIS s.max_yres = 1536;
  BX_INFO(("%s initialized", model_string));
#if BX_DEBUGGER
  // register device for the 'info device' command (calls debug_dump())
  bx_dbg_register_debug_info("nvriva", this);
#endif
  return 1;
}

#define SETUP_BITBLT(num, name, flags) \
  do { \
    BX_NVRIVA_THIS rop_handler[num] = bitblt_rop_fwd_##name; \
    BX_NVRIVA_THIS rop_flags[num] = flags; \
  } while (0);

void bx_nvriva_c::bitblt_init()
{
  for (int i = 0; i < 0x100; i++) {
    SETUP_BITBLT(i, nop, BX_ROP_PATTERN);
  }
  SETUP_BITBLT(0x00, 0, 0);                              // 0
  SETUP_BITBLT(0x05, notsrc_and_notdst, BX_ROP_PATTERN); // PSan
  SETUP_BITBLT(0x0a, notsrc_and_dst, BX_ROP_PATTERN);    // DPna
  SETUP_BITBLT(0x0f, notsrc, BX_ROP_PATTERN);            // Pn
  SETUP_BITBLT(0x11, notsrc_and_notdst, 0);              // DSon
  SETUP_BITBLT(0x22, notsrc_and_dst, 0);                 // DSna
  SETUP_BITBLT(0x33, notsrc, 0);                         // Sn
  SETUP_BITBLT(0x44, src_and_notdst, 0);                 // SDna
  SETUP_BITBLT(0x50, src_and_notdst, 0);                 // PDna
  SETUP_BITBLT(0x55, notdst, 0);                         // Dn
  SETUP_BITBLT(0x5a, src_xor_dst, BX_ROP_PATTERN);       // DPx
  SETUP_BITBLT(0x5f, notsrc_or_notdst, BX_ROP_PATTERN);  // DSan
  SETUP_BITBLT(0x66, src_xor_dst, 0);                    // DSx
  SETUP_BITBLT(0x77, notsrc_or_notdst, 0);               // DSan
  SETUP_BITBLT(0x88, src_and_dst, 0);                    // DSa
  SETUP_BITBLT(0x99, src_notxor_dst, 0);                 // DSxn
  SETUP_BITBLT(0xaa, nop, 0);                            // D
  SETUP_BITBLT(0xad, src_and_dst, BX_ROP_PATTERN);       // DPa
  SETUP_BITBLT(0xaf, notsrc_or_dst, BX_ROP_PATTERN);     // DPno
  SETUP_BITBLT(0xbb, notsrc_or_dst, 0);                  // DSno
  SETUP_BITBLT(0xcc, src, 0);                            // S
  SETUP_BITBLT(0xdd, src_and_notdst, 0);                 // SDna
  SETUP_BITBLT(0xee, src_or_dst, 0);                     // DSo
  SETUP_BITBLT(0xf0, src, BX_ROP_PATTERN);               // P
  SETUP_BITBLT(0xf5, src_or_notdst, BX_ROP_PATTERN);     // PDno
  SETUP_BITBLT(0xfa, src_or_dst, BX_ROP_PATTERN);        // DPo
  SETUP_BITBLT(0xff, 1, 0);                              // 1
}

void bx_nvriva_c::svga_init_members()
{
  // clear all registers.
  BX_NVRIVA_THIS crtc.index = NVRIVA_CRTC_MAX + 1;
  for (int i = 0; i <= NVRIVA_CRTC_MAX; i++)
    BX_NVRIVA_THIS crtc.reg[i] = 0x00;

  BX_NVRIVA_THIS mc_intr_en = 0;
  BX_NVRIVA_THIS mc_enable = 0;
  BX_NVRIVA_THIS bus_intr = 0;
  BX_NVRIVA_THIS bus_intr_en = 0;
  BX_NVRIVA_THIS fifo_intr = 0;
  BX_NVRIVA_THIS fifo_intr_en = 0;
  BX_NVRIVA_THIS fifo_ramht = 0;
  BX_NVRIVA_THIS fifo_ramfc = 0;
  BX_NVRIVA_THIS fifo_ramro = 0;
  BX_NVRIVA_THIS fifo_mode = 0;
  BX_NVRIVA_THIS fifo_cache1_push1 = 0;
  BX_NVRIVA_THIS fifo_cache1_put = 0;
  BX_NVRIVA_THIS fifo_cache1_dma_push = 0;
  BX_NVRIVA_THIS fifo_cache1_dma_instance = 0;
  BX_NVRIVA_THIS fifo_cache1_dma_put = 0;
  BX_NVRIVA_THIS fifo_cache1_dma_get = 0;
  BX_NVRIVA_THIS fifo_cache1_ref_cnt = 0;
  BX_NVRIVA_THIS fifo_cache1_pull0 = 0;
  BX_NVRIVA_THIS fifo_cache1_semaphore = 0;
  BX_NVRIVA_THIS fifo_cache1_get = 0;
  for (int i = 0; i < NVRIVA_CACHE1_SIZE; i++) {
    BX_NVRIVA_THIS fifo_cache1_method[i] = 0;
    BX_NVRIVA_THIS fifo_cache1_data[i] = 0;
  }
  BX_NVRIVA_THIS rma_addr = 0;
  BX_NVRIVA_THIS timer_intr = 0;
  BX_NVRIVA_THIS timer_intr_en = 0;
  BX_NVRIVA_THIS timer_num = 0;
  BX_NVRIVA_THIS timer_den = 0;
  BX_NVRIVA_THIS timer_inittime1 = 0;
  BX_NVRIVA_THIS timer_inittime2 = 0;
  BX_NVRIVA_THIS timer_alarm = 0;
  BX_NVRIVA_THIS graph_intr = 0;
  BX_NVRIVA_THIS graph_nsource = 0;
  BX_NVRIVA_THIS graph_intr_en = 0;
  BX_NVRIVA_THIS graph_ctx_switch1 = 0;
  BX_NVRIVA_THIS graph_ctx_switch2 = 0;
  BX_NVRIVA_THIS graph_ctx_switch4 = 0;
  BX_NVRIVA_THIS graph_ctxctl_cur = 0;
  BX_NVRIVA_THIS graph_status = 0;
  BX_NVRIVA_THIS graph_trapped_addr = 0;
  BX_NVRIVA_THIS graph_trapped_data = 0;
  BX_NVRIVA_THIS graph_notify = 0;
  BX_NVRIVA_THIS graph_fifo = 0;
  BX_NVRIVA_THIS graph_channel_ctx_table = 0;
  BX_NVRIVA_THIS graph_offset0 = 0;
  BX_NVRIVA_THIS graph_pitch0 = 0;
  BX_NVRIVA_THIS crtc_intr = 0;
  BX_NVRIVA_THIS crtc_intr_en = 0;
  BX_NVRIVA_THIS crtc_start = 0;
  BX_NVRIVA_THIS crtc_config = 0;
  BX_NVRIVA_THIS crtc_cursor_offset = 0;
  BX_NVRIVA_THIS crtc_cursor_config = 0;
  BX_NVRIVA_THIS ramdac_cu_start_pos = 0;
  BX_NVRIVA_THIS ramdac_vpll = 0;
  BX_NVRIVA_THIS ramdac_vpll_b = 0;
  BX_NVRIVA_THIS ramdac_pll_select = 0;
  BX_NVRIVA_THIS ramdac_general_control = 0;

  BX_NVRIVA_THIS acquire_active = false;

  memset(BX_NVRIVA_THIS chs, 0x00, sizeof(BX_NVRIVA_THIS chs));
  for (int i = 0; i < NVRIVA_CHANNEL_COUNT; i++) {
    BX_NVRIVA_THIS chs[i].swzs_color_bytes = 1;
    BX_NVRIVA_THIS chs[i].s2d_color_bytes = 1;
    BX_NVRIVA_THIS chs[i].d3d_color_bytes = 1;
    BX_NVRIVA_THIS chs[i].d3d_depth_bytes = 1;
    
    // Initialize fog color to default values
    BX_NVRIVA_THIS chs[i].d3d_fog_color = 0x00000000;
    BX_NVRIVA_THIS chs[i].d3d_fog_color_r = 0;
    BX_NVRIVA_THIS chs[i].d3d_fog_color_g = 0;
    BX_NVRIVA_THIS chs[i].d3d_fog_color_b = 0;
    BX_NVRIVA_THIS chs[i].d3d_fog_color_a = 0;
    BX_NVRIVA_THIS chs[i].d3d_fog_enable = false;
    
    // Initialize control to default values
    BX_NVRIVA_THIS chs[i].d3d_control = 0x00000000;
    BX_NVRIVA_THIS chs[i].d3d_alpha_ref = 0;
    BX_NVRIVA_THIS chs[i].d3d_alpha_func = NV04_TEXTURED_TRIANGLE_CONTROL_ALPHA_FUNC_ALWAYS >> NV04_TEXTURED_TRIANGLE_CONTROL_ALPHA_FUNC__SHIFT;
    BX_NVRIVA_THIS chs[i].d3d_alpha_enable = false;
    BX_NVRIVA_THIS chs[i].d3d_origin_center = true;
    BX_NVRIVA_THIS chs[i].d3d_z_enable = true;
    BX_NVRIVA_THIS chs[i].d3d_z_func = NV04_TEXTURED_TRIANGLE_CONTROL_Z_FUNC_LEQUAL >> NV04_TEXTURED_TRIANGLE_CONTROL_Z_FUNC__SHIFT;
    BX_NVRIVA_THIS chs[i].d3d_cull_mode = NV04_TEXTURED_TRIANGLE_CONTROL_CULL_MODE_NONE >> NV04_TEXTURED_TRIANGLE_CONTROL_CULL_MODE__SHIFT;
    BX_NVRIVA_THIS chs[i].d3d_dither_enable = false;
    BX_NVRIVA_THIS chs[i].d3d_z_perspective_enable = true;
    BX_NVRIVA_THIS chs[i].d3d_z_write = true;
    BX_NVRIVA_THIS chs[i].d3d_z_format = NV04_TEXTURED_TRIANGLE_CONTROL_Z_FORMAT_FIXED >> NV04_TEXTURED_TRIANGLE_CONTROL_Z_FORMAT__SHIFT;
    
    // Initialize blend to default values
    BX_NVRIVA_THIS chs[i].d3d_blend = 0x00000000;
    BX_NVRIVA_THIS chs[i].d3d_texture_map = NV04_TEXTURED_TRIANGLE_BLEND_TEXTURE_MAP_MODULATE >> NV04_TEXTURED_TRIANGLE_BLEND_TEXTURE_MAP__SHIFT;
    BX_NVRIVA_THIS chs[i].d3d_mask_bit = NV04_TEXTURED_TRIANGLE_BLEND_MASK_BIT_LSB >> NV04_TEXTURED_TRIANGLE_BLEND_MASK_BIT__SHIFT;
    BX_NVRIVA_THIS chs[i].d3d_shade_mode = NV04_TEXTURED_TRIANGLE_BLEND_SHADE_MODE_GOURAUD >> NV04_TEXTURED_TRIANGLE_BLEND_SHADE_MODE__SHIFT;
    BX_NVRIVA_THIS chs[i].d3d_texture_perspective_enable = true;
    BX_NVRIVA_THIS chs[i].d3d_specular_enable = false;
    BX_NVRIVA_THIS chs[i].d3d_blend_enable = false;
    BX_NVRIVA_THIS chs[i].d3d_blend_src = NV04_TEXTURED_TRIANGLE_BLEND_SRC_SRC_ALPHA >> NV04_TEXTURED_TRIANGLE_BLEND_SRC__SHIFT;
    BX_NVRIVA_THIS chs[i].d3d_blend_dst = NV04_TEXTURED_TRIANGLE_BLEND_DST_ONE_MINUS_SRC_ALPHA >> NV04_TEXTURED_TRIANGLE_BLEND_DST__SHIFT;
    
    // Initialize filter to default values
    BX_NVRIVA_THIS chs[i].d3d_filter = 0x00000000;
    BX_NVRIVA_THIS chs[i].d3d_kernel_size_x = 1;
    BX_NVRIVA_THIS chs[i].d3d_kernel_size_y = 1;
    BX_NVRIVA_THIS chs[i].d3d_mipmap_dither_enable = false;
    BX_NVRIVA_THIS chs[i].d3d_mipmap_lodbias = 0;
    BX_NVRIVA_THIS chs[i].d3d_minify_filter = NV04_TEXTURED_TRIANGLE_FILTER_MINIFY_NEAREST >> NV04_TEXTURED_TRIANGLE_FILTER_MINIFY__SHIFT;
    BX_NVRIVA_THIS chs[i].d3d_magnify_filter = NV04_TEXTURED_TRIANGLE_FILTER_MAGNIFY_NEAREST >> NV04_TEXTURED_TRIANGLE_FILTER_MAGNIFY__SHIFT;
    BX_NVRIVA_THIS chs[i].d3d_anisotropic_minify_enable = false;
    BX_NVRIVA_THIS chs[i].d3d_anisotropic_magnify_enable = false;
    
    // Initialize offset to default values
    BX_NVRIVA_THIS chs[i].d3d_offset = 0x00000000;
    for (int j = 0; j < 16; j++) {
      BX_NVRIVA_THIS chs[i].d3d_texture_offset[j] = 0x00000000;
    }
    
    // Initialize colorkey to default values
    BX_NVRIVA_THIS chs[i].d3d_colorkey = 0x00000000;
    BX_NVRIVA_THIS chs[i].d3d_colorkey_r = 0;
    BX_NVRIVA_THIS chs[i].d3d_colorkey_g = 0;
    BX_NVRIVA_THIS chs[i].d3d_colorkey_b = 0;
    BX_NVRIVA_THIS chs[i].d3d_colorkey_a = 0;
    BX_NVRIVA_THIS chs[i].d3d_colorkey_enable = false;
  }

  for (int i = 0; i < 4 * 1024 * 1024; i++)
    BX_NVRIVA_THIS unk_regs[i] = 0;

  BX_NVRIVA_THIS svga_unlock_special = 0;
  BX_NVRIVA_THIS svga_needs_update_tile = 1;
  BX_NVRIVA_THIS svga_needs_update_dispentire = 1;
  BX_NVRIVA_THIS svga_needs_update_mode = 0;
  BX_NVRIVA_THIS svga_double_width = 0;

  BX_NVRIVA_THIS svga_xres = 640;
  BX_NVRIVA_THIS svga_yres = 480;
  BX_NVRIVA_THIS svga_bpp = 8;
  BX_NVRIVA_THIS svga_pitch = 640;
  BX_NVRIVA_THIS bank_base[0] = 0;
  BX_NVRIVA_THIS bank_base[1] = 0;

  BX_NVRIVA_THIS hw_cursor.x = 0;
  BX_NVRIVA_THIS hw_cursor.y = 0;
  BX_NVRIVA_THIS hw_cursor.size = 0;
  BX_NVRIVA_THIS hw_cursor.offset = 0;
  BX_NVRIVA_THIS hw_cursor.bpp32 = false;
  BX_NVRIVA_THIS hw_cursor.enabled = false;

  if (BX_NVRIVA_THIS card_type == 0x04) {
    BX_NVRIVA_THIS s.memsize = 16 * 1024 * 1024;
    // Guess
    BX_NVRIVA_THIS straps0_primary_original = 0x000001A2;
  } else if (BX_NVRIVA_THIS card_type == 0x05) {
    BX_NVRIVA_THIS s.memsize = 32 * 1024 * 1024;
    // Guess
    BX_NVRIVA_THIS straps0_primary_original = 0x000001A2;
  }
  BX_NVRIVA_THIS straps0_primary = BX_NVRIVA_THIS straps0_primary_original;
  BX_NVRIVA_THIS ramin_flip = BX_NVRIVA_THIS s.memsize - 32;
  BX_NVRIVA_THIS memsize_mask = BX_NVRIVA_THIS s.memsize - 1;
  BX_NVRIVA_THIS class_mask = 0x0000FFFF;

  // memory allocation.
  if (BX_NVRIVA_THIS s.memory == NULL)
    BX_NVRIVA_THIS s.memory = new Bit8u[BX_NVRIVA_THIS s.memsize];
  memset(BX_NVRIVA_THIS s.memory, 0x00, BX_NVRIVA_THIS s.memsize);
  BX_NVRIVA_THIS disp_ptr = BX_NVRIVA_THIS s.memory;
  BX_NVRIVA_THIS disp_offset = 0;

  // VCLK defaults after reset
  BX_NVRIVA_THIS s.vclk[0] = 25180000;
  BX_NVRIVA_THIS s.vclk[1] = 28325000;
  BX_NVRIVA_THIS s.vclk[2] = 41165000;
  BX_NVRIVA_THIS s.vclk[3] = 36082000;
}

void bx_nvriva_c::reset(unsigned type)
{
  // reset VGA stuffs.
  BX_NVRIVA_THIS bx_vgacore_c::reset(type);
  // reset SVGA stuffs.
  BX_NVRIVA_THIS svga_init_members();
  BX_NVRIVA_THIS ddc.init();
  // Disable ROM shadowing to allow clearing of VRAM
  BX_NVRIVA_THIS pci_conf[0x50] = 0x00;
}

void bx_nvriva_c::register_state(void)
{
  bx_list_c *list = new bx_list_c(SIM->get_bochs_root(), "nvriva", "NVidia RIVA State");
  BX_NVRIVA_THIS vgacore_register_state(list);
  bx_list_c *crtc = new bx_list_c(list, "crtc");
  new bx_shadow_num_c(crtc, "index", &BX_NVRIVA_THIS crtc.index, BASE_HEX);
  new bx_shadow_data_c(crtc, "reg", BX_NVRIVA_THIS crtc.reg, NVRIVA_CRTC_MAX + 1, 1);
  BXRS_PARAM_BOOL(list, svga_unlock_special, BX_NVRIVA_THIS svga_unlock_special);
  BXRS_PARAM_BOOL(list, svga_double_width, BX_NVRIVA_THIS svga_double_width);
  new bx_shadow_num_c(list, "svga_xres", &BX_NVRIVA_THIS svga_xres);
  new bx_shadow_num_c(list, "svga_yres", &BX_NVRIVA_THIS svga_yres);
  new bx_shadow_num_c(list, "svga_pitch", &BX_NVRIVA_THIS svga_pitch);
  new bx_shadow_num_c(list, "svga_bpp", &BX_NVRIVA_THIS svga_bpp);
  new bx_shadow_num_c(list, "svga_dispbpp", &BX_NVRIVA_THIS svga_dispbpp);
  new bx_shadow_num_c(list, "bank_base0", &BX_NVRIVA_THIS bank_base[0], BASE_HEX);
  new bx_shadow_num_c(list, "bank_base1", &BX_NVRIVA_THIS bank_base[1], BASE_HEX);
  bx_list_c *cursor = new bx_list_c(list, "hw_cursor");
  new bx_shadow_num_c(cursor, "x", &BX_NVRIVA_THIS hw_cursor.x, BASE_HEX);
  new bx_shadow_num_c(cursor, "y", &BX_NVRIVA_THIS hw_cursor.y, BASE_HEX);
  new bx_shadow_num_c(cursor, "size", &BX_NVRIVA_THIS hw_cursor.size, BASE_HEX);
  register_pci_state(list);
}

void bx_nvriva_c::after_restore_state(void)
{
  bx_pci_device_c::after_restore_pci_state(nvriva_mem_read_handler);
  BX_NVRIVA_THIS bx_vgacore_c::after_restore_state();
}

void bx_nvriva_c::redraw_area(unsigned x0, unsigned y0,
                               unsigned width, unsigned height)
{
  redraw_area_d((Bit32s)x0, (Bit32s)y0, width, height);
}

void bx_nvriva_c::redraw_area_nd(Bit32u offset, Bit32u width, Bit32u height)
{
  if (BX_NVRIVA_THIS svga_pitch != 0) {
    Bit32u redraw_x = offset % BX_NVRIVA_THIS svga_pitch / (BX_NVRIVA_THIS svga_bpp >> 3);
    Bit32u redraw_y = offset / BX_NVRIVA_THIS svga_pitch;
    BX_NVRIVA_THIS redraw_area_nd(redraw_x, redraw_y, width, height);
  }
}

void bx_nvriva_c::redraw_area_nd(Bit32s x0, Bit32s y0, Bit32u width, Bit32u height)
{
  if (s.y_doublescan) {
    y0 <<= 1;
    height <<= 1;
  }
  if (BX_NVRIVA_THIS svga_double_width) {
    x0 <<= 1;
    width <<= 1;
  }
  BX_NVRIVA_THIS redraw_area_d(x0, y0, width, height);
}

void bx_nvriva_c::redraw_area_d(Bit32s x0, Bit32s y0, Bit32u width, Bit32u height)
{
  if (x0 + (Bit32s)width <= 0 || y0 + (Bit32s)height <= 0)
    return;

  if (!BX_NVRIVA_THIS crtc.reg[0x28]) {
    BX_NVRIVA_THIS bx_vgacore_c::redraw_area(x0, y0, width, height);
    return;
  }

  if (BX_NVRIVA_THIS svga_needs_update_mode)
    return;

  BX_NVRIVA_THIS svga_needs_update_tile = 1;

  unsigned xti, yti, xt0, xt1, yt0, yt1;
  xt0 = x0 <= 0 ? 0 : x0 / X_TILESIZE;
  yt0 = y0 <= 0 ? 0 : y0 / Y_TILESIZE;
  if (x0 < (Bit32s)BX_NVRIVA_THIS svga_xres) {
    xt1 = (x0 + width - 1) / X_TILESIZE;
  } else {
    xt1 = (BX_NVRIVA_THIS svga_xres - 1) / X_TILESIZE;
  }
  if (y0 < (Bit32s)BX_NVRIVA_THIS svga_yres) {
    yt1 = (y0 + height - 1) / Y_TILESIZE;
  } else {
    yt1 = (BX_NVRIVA_THIS svga_yres - 1) / Y_TILESIZE;
  }
  if ((x0 + width) > BX_NVRIVA_THIS svga_xres) {
    BX_NVRIVA_THIS redraw_area_d(0, y0 + 1, x0 + width - BX_NVRIVA_THIS svga_xres, height);
  }
  for (yti=yt0; yti<=yt1; yti++) {
    for (xti=xt0; xti<=xt1; xti++) {
      SET_TILE_UPDATED(BX_NVRIVA_THIS, xti, yti, 1);
    }
  }
}

void bx_nvriva_c::vertical_timer()
{
  bx_vgacore_c::vertical_timer();
  if (BX_NVRIVA_THIS vtimer_toggle) {
    BX_NVRIVA_THIS crtc_intr |= 0x00000001;
    BX_DEBUG(("vertical_timer, crtc_intr_en = %d, mc_intr_en = %d",
      BX_NVRIVA_THIS crtc_intr_en, BX_NVRIVA_THIS mc_intr_en));
    update_irq_level();
  }
  if (BX_NVRIVA_THIS acquire_active) {
    BX_NVRIVA_THIS acquire_active = false;
    for (int i = 0; i < NVRIVA_CHANNEL_COUNT; i++)
      fifo_process(i);
  }
}

bool bx_nvriva_c::nvriva_mem_read_handler(bx_phy_address addr, unsigned len,
                                            void *data, void *param)
{
  if (addr >= BX_NVRIVA_THIS pci_bar[0].addr &&
      addr < (BX_NVRIVA_THIS pci_bar[0].addr + NVRIVA_PNPMMIO_SIZE)) {
    Bit32u offset = addr & (NVRIVA_PNPMMIO_SIZE - 1);
    if (len == 1) {
      *(Bit8u*)data = register_read8(offset);
      BX_DEBUG(("MMIO read from 0x%08x, value 0x%02x", offset, *(Bit8u*)data));
    } else if (len == 2) {
      Bit16u value = (Bit16u)register_read32(offset);
      BX_DEBUG(("MMIO read from 0x%08x, value 0x%04x", offset, value));
      *((Bit16u*)data) = value;
    } else if (len == 4) {
      Bit32u value = register_read32(offset);
      BX_DEBUG(("MMIO read from 0x%08x, value 0x%08x", offset, value));
      *((Bit32u*)data) = value;
    } else {
      BX_PANIC(("MMIO read len %d", len));
    }
    return true;
  }

  Bit8u *data_ptr;
#ifdef BX_LITTLE_ENDIAN
  data_ptr = (Bit8u *) data;
#else // BX_BIG_ENDIAN
  data_ptr = (Bit8u *) data + (len - 1);
#endif
  for (unsigned i = 0; i < len; i++) {
    *data_ptr = BX_NVRIVA_THIS mem_read(addr);
    addr++;
#ifdef BX_LITTLE_ENDIAN
    data_ptr++;
#else // BX_BIG_ENDIAN
    data_ptr--;
#endif
  }
  return true;
}

Bit8u bx_nvriva_c::mem_read(bx_phy_address addr)
{
  if (BX_NVRIVA_THIS pci_rom_size > 0) {
    Bit32u mask = (BX_NVRIVA_THIS pci_rom_size - 1);
    if (((Bit32u)addr & ~mask) == BX_NVRIVA_THIS pci_rom_address) {
      if (BX_NVRIVA_THIS pci_conf[0x30] & 0x01) {
        if (BX_NVRIVA_THIS pci_conf[0x50] == 0x00)
          return BX_NVRIVA_THIS pci_rom[addr & mask];
        else
          return BX_NVRIVA_THIS s.memory[(addr & mask) ^ BX_NVRIVA_THIS ramin_flip];
      } else {
        return 0xff;
      }
    }
  }

  if ((addr >= BX_NVRIVA_THIS pci_bar[1].addr) &&
      (addr < (BX_NVRIVA_THIS pci_bar[1].addr + BX_NVRIVA_THIS s.memsize))) {
    Bit32u offset = addr & BX_NVRIVA_THIS memsize_mask;
    return BX_NVRIVA_THIS s.memory[offset];
  }

  if (!BX_NVRIVA_THIS crtc.reg[0x28])
    return BX_NVRIVA_THIS bx_vgacore_c::mem_read(addr);

  if (addr >= 0xA0000 && addr <= 0xAFFFF) {
    Bit32u offset = addr & 0xffff;
    offset += BX_NVRIVA_THIS bank_base[0];
    offset &= BX_NVRIVA_THIS memsize_mask;
    return BX_NVRIVA_THIS s.memory[offset];
  }

  return 0xff;
}

bool bx_nvriva_c::nvriva_mem_write_handler(bx_phy_address addr, unsigned len,
                                             void *data, void *param)
{
  if (addr >= BX_NVRIVA_THIS pci_bar[0].addr &&
      addr < (BX_NVRIVA_THIS pci_bar[0].addr + NVRIVA_PNPMMIO_SIZE)) {
    Bit32u offset = addr & (NVRIVA_PNPMMIO_SIZE - 1);
    if (len == 1) {
      BX_DEBUG(("MMIO write to 0x%08x, value 0x%02x", offset, *(Bit8u*)data));
      register_write8(offset, *(Bit8u*)data);
    } else if (len == 4) {
      Bit32u value = *((Bit32u*)data);
      BX_DEBUG(("MMIO write to 0x%08x, value 0x%08x", offset, value));
      register_write32(offset, value);
    } else if (len == 8) {
      Bit64u value = *((Bit64u*)data);
      BX_DEBUG(("MMIO write to 0x%08x, value 0x%016" FMT_64 "x", offset, value));
      register_write32(offset, (Bit32u)value);
      register_write32(offset + 4, value >> 32);
    } else {
      BX_PANIC(("MMIO write len %d", len));
    }
    return true;
  }

  Bit8u *data_ptr;
#ifdef BX_LITTLE_ENDIAN
  data_ptr = (Bit8u *) data;
#else // BX_BIG_ENDIAN
  data_ptr = (Bit8u *) data + (len - 1);
#endif
  for (unsigned i = 0; i < len; i++) {
    BX_NVRIVA_THIS mem_write(addr, *data_ptr);
    addr++;
#ifdef BX_LITTLE_ENDIAN
    data_ptr++;
#else // BX_BIG_ENDIAN
    data_ptr--;
#endif
  }
  return true;
}

void bx_nvriva_c::mem_write(bx_phy_address addr, Bit8u value)
{
  if ((addr >= BX_NVRIVA_THIS pci_bar[1].addr) &&
      (addr < (BX_NVRIVA_THIS pci_bar[1].addr + BX_NVRIVA_THIS s.memsize))) {
    unsigned x, y;
    Bit32u offset = addr & BX_NVRIVA_THIS memsize_mask;
    BX_NVRIVA_THIS s.memory[offset] = value;
    if (BX_NVRIVA_THIS svga_pitch != 0) {
      BX_NVRIVA_THIS svga_needs_update_tile = 1;
      offset -= BX_NVRIVA_THIS disp_offset;
      x = (offset % BX_NVRIVA_THIS svga_pitch) / (BX_NVRIVA_THIS svga_bpp / 8);
      y = offset / BX_NVRIVA_THIS svga_pitch;
      if (BX_NVRIVA_THIS s.y_doublescan)
        y <<= 1;
      if (BX_NVRIVA_THIS svga_double_width)
        x <<= 1;
      SET_TILE_UPDATED(BX_NVRIVA_THIS, x / X_TILESIZE, y / Y_TILESIZE, 1);
    }
    return;
  }

  if (!BX_NVRIVA_THIS crtc.reg[0x28]) {
    BX_NVRIVA_THIS bx_vgacore_c::mem_write(addr, value);
    return;
  }

  if (addr >= 0xA0000 && addr <= 0xAFFFF) {
    Bit32u offset;
    unsigned x, y;

    offset = addr & 0xffff;
    if (BX_NVRIVA_THIS crtc.reg[0x1c] & 0x80) {
      BX_NVRIVA_THIS s.memory[offset ^ BX_NVRIVA_THIS ramin_flip] = value;
      return;
    }
    offset += BX_NVRIVA_THIS bank_base[0];
    offset &= BX_NVRIVA_THIS memsize_mask;
    BX_NVRIVA_THIS s.memory[offset] = value;
    BX_NVRIVA_THIS svga_needs_update_tile = 1;
    x = (offset % BX_NVRIVA_THIS svga_pitch) / (BX_NVRIVA_THIS svga_bpp / 8);
    y = offset / BX_NVRIVA_THIS svga_pitch;
    if (BX_NVRIVA_THIS s.y_doublescan)
      y <<= 1;
    if (BX_NVRIVA_THIS svga_double_width)
      x <<= 1;
    SET_TILE_UPDATED(BX_NVRIVA_THIS, x / X_TILESIZE, y / Y_TILESIZE, 1);
  }
}

void bx_nvriva_c::get_text_snapshot(Bit8u **text_snapshot,
                                    unsigned *txHeight, unsigned *txWidth)
{
  BX_NVRIVA_THIS bx_vgacore_c::get_text_snapshot(text_snapshot,txHeight,txWidth);
}

Bit32u bx_nvriva_c::svga_read_handler(void *this_ptr, Bit32u address, unsigned io_len)
{
#if !BX_USE_NVRIVA_SMF
  bx_nvriva_c *class_ptr = (bx_nvriva_c *) this_ptr;

  return class_ptr->svga_read(address, io_len);
}

Bit32u bx_nvriva_c::svga_read(Bit32u address, unsigned io_len)
{
#else
  UNUSED(this_ptr);
#endif // !BX_USE_NVRIVA_SMF

  if (address == 0x03C3 && io_len == 2)
    return VGA_READ(address, 1) | VGA_READ(address + 1, 1) << 8;

  if (address == 0x03d0 || address == 0x03d2) // RMA_ACCESS
  {
      if (io_len == 1) {
        BX_ERROR(("port 0x3d0 access with io_len = 1"));
        return 0;
      }
      Bit8u crtc38 = BX_NVRIVA_THIS crtc.reg[0x38];
      bool rma_enable = crtc38 & 1;
      if (!rma_enable) {
        BX_ERROR(("port 0x3d0 access is disabled"));
        return 0;
      }
      int rma_index = crtc38 >> 1;
      if (rma_index == 1) {
        if (address == 0x03d0) {
          //if (io_len == 2)
          //  BX_DEBUG(("rma: read address 0x%08x (lo)", BX_NVRIVA_THIS rma_addr));
          //else
          //  BX_DEBUG(("rma: read address 0x%08x", BX_NVRIVA_THIS rma_addr));
          return BX_NVRIVA_THIS rma_addr;
        } else {
          //BX_DEBUG(("rma: read address 0x%08x (hi)", BX_NVRIVA_THIS rma_addr));
          return BX_NVRIVA_THIS rma_addr >> 16;
        }
      }
      else if (rma_index == 2) {
        bool vram = false;
        Bit32u offset = BX_NVRIVA_THIS rma_addr;
        if (BX_NVRIVA_THIS rma_addr & 0x80000000) {
          vram = true;
          offset &= ~0x80000000;
        } 
        if ((!vram && offset < NVRIVA_PNPMMIO_SIZE) ||
             (vram && offset < BX_NVRIVA_THIS s.memsize)) {
          Bit32u value = vram ? vram_read32(offset) : register_read32(offset);
          if (address == 0x03d0) {
            if (io_len == 2)
              BX_DEBUG(("rma: read from 0x%08x value 0x????%04x", BX_NVRIVA_THIS rma_addr, value & 0xFFFF));
            else
              BX_DEBUG(("rma: read from 0x%08x value 0x%08x", BX_NVRIVA_THIS rma_addr, value));
            return value;
          } else {
            BX_DEBUG(("rma: read from 0x%08x value 0x%04x????", BX_NVRIVA_THIS rma_addr, value >> 16));
            return value >> 16;
          }
        } else {
          BX_ERROR(("rma: oob read from 0x%08x ignored", BX_NVRIVA_THIS rma_addr));
          return 0xFFFFFFFF;
        }
      }
      else if (rma_index == 3)
        BX_ERROR(("rma: read index 3"));
      else
        BX_PANIC(("rma: read unknown index %d", rma_index));
      return 0;
  }

  if ((io_len == 2) && ((address & 1) == 0)) {
    Bit32u value  = (Bit32u)SVGA_READ(address,1);
           value |= (Bit32u)SVGA_READ(address+1,1) << 8;
    return value;
  }

  if (io_len != 1) {
    BX_PANIC(("SVGA read: io_len != 1"));
  }

  switch (address) {
    case 0x03b4: /* VGA: CRTC Index Register (monochrome emulation modes) */
    case 0x03d4: /* VGA: CRTC Index Register (color emulation modes) */
      return BX_NVRIVA_THIS crtc.index;
    case 0x03b5: /* VGA: CRTC Registers (monochrome emulation modes) */
    case 0x03d5: /* VGA: CRTC Registers (color emulation modes) */
      if (BX_NVRIVA_THIS crtc.index > VGA_CRTC_MAX)
        return BX_NVRIVA_THIS svga_read_crtc(address,BX_NVRIVA_THIS crtc.index);
      else
        break;
    case 0x03c2: /* Input Status 0 */
      BX_DEBUG(("Input Status 0 read"));
      return 0x10; // Monitor presence detection (DAC Sensing)
    default:
      break;
  }

  return VGA_READ(address,io_len);
}

void bx_nvriva_c::svga_write_handler(void *this_ptr, Bit32u address, Bit32u value, unsigned io_len)
{
#if !BX_USE_NVRIVA_SMF
  bx_nvriva_c *class_ptr = (bx_nvriva_c *) this_ptr;
  class_ptr->svga_write(address, value, io_len);
}

void bx_nvriva_c::svga_write(Bit32u address, Bit32u value, unsigned io_len)
{
#else
  UNUSED(this_ptr);
#endif // !BX_USE_NVRIVA_SMF

  if (address == 0x03d0 || address == 0x03d2) // RMA_ACCESS
  {
      if (io_len == 1) {
        BX_DEBUG(("port 0x3d0 access with io_len = 1"));
        return;
      }
      Bit8u crtc38 = BX_NVRIVA_THIS crtc.reg[0x38];
      bool rma_enable = crtc38 & 1;
      if (!rma_enable) {
        BX_ERROR(("port 0x3d0 access is disabled"));
        return;
      }
      int rma_index = crtc38 >> 1;
      if (rma_index == 1) {
        if (address == 0x03d0) {
          if (io_len == 2) {
            BX_NVRIVA_THIS rma_addr &= 0xFFFF0000;
            BX_NVRIVA_THIS rma_addr |= value;
            //BX_DEBUG(("rma: address set to 0x%08x (lo)", BX_NVRIVA_THIS rma_addr));
          } else {
            BX_NVRIVA_THIS rma_addr = value;
            //BX_DEBUG(("rma: address set to 0x%08x", BX_NVRIVA_THIS rma_addr));
          }
        } else {
          BX_NVRIVA_THIS rma_addr &= 0x0000FFFF;
          BX_NVRIVA_THIS rma_addr |= value << 16;
          //BX_DEBUG(("rma: address set to 0x%08x (hi)", BX_NVRIVA_THIS rma_addr));
        }
      } else if (rma_index == 2) {
        BX_ERROR(("rma: write index 2"));
      } else if (rma_index == 3) {
        bool vram = false;
        Bit32u offset = BX_NVRIVA_THIS rma_addr & ~3; // Fix for 6800 GT BIOS
        if (BX_NVRIVA_THIS rma_addr & 0x80000000) {
          vram = true;
          offset &= ~0x80000000;
        }
        if ((!vram && offset < NVRIVA_PNPMMIO_SIZE) || 
             (vram && offset < BX_NVRIVA_THIS s.memsize)) {
          if (address == 0x03d0) {
            if (io_len == 2) {
              Bit32u value32 = vram ? vram_read32(offset) : register_read32(offset);
              value32 &= 0xFFFF0000;
              value32 |= value;
              if (vram)
                vram_write32(offset, value32);
              else
                register_write32(offset, value32);
              BX_DEBUG(("rma: write to 0x%08x value 0x????%04x", BX_NVRIVA_THIS rma_addr, value));
            } else {
              if (vram)
                vram_write32(offset, value);
              else
                register_write32(offset, value);
              BX_DEBUG(("rma: write to 0x%08x value 0x%08x", BX_NVRIVA_THIS rma_addr, value));
            }
          } else {
            Bit32u value32 = vram ? vram_read32(offset) : register_read32(offset);
            value32 &= 0x0000FFFF;
            value32 |= value << 16;
            if (vram)
              vram_write32(offset, value32);
            else
              register_write32(offset, value32);
            BX_DEBUG(("rma: write to 0x%08x value 0x%04x????", BX_NVRIVA_THIS rma_addr, value));
          }
        } else {
          BX_ERROR(("rma: oob write to 0x%08x ignored", BX_NVRIVA_THIS rma_addr));
        }
      } else {
        BX_PANIC(("rma: write unknown index %d", rma_index));
      }
      return;
  }

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
      BX_NVRIVA_THIS crtc.index = value;
      break;
    case 0x03b5: /* VGA: CRTC Registers (monochrome emulation modes) */
    case 0x03d5: /* VGA: CRTC Registers (color emulation modes) */
      if (BX_NVRIVA_THIS crtc.index == 0x01 ||
          BX_NVRIVA_THIS crtc.index == 0x07 ||
          BX_NVRIVA_THIS crtc.index == 0x09 ||
          BX_NVRIVA_THIS crtc.index == 0x0c ||
          BX_NVRIVA_THIS crtc.index == 0x0d ||
          BX_NVRIVA_THIS crtc.index == 0x12 ||
          BX_NVRIVA_THIS crtc.index == 0x13 ||
          BX_NVRIVA_THIS crtc.index == 0x15 ||
          BX_NVRIVA_THIS crtc.index == 0x19 ||
          BX_NVRIVA_THIS crtc.index == 0x25 ||
          BX_NVRIVA_THIS crtc.index == 0x28 ||
          BX_NVRIVA_THIS crtc.index == 0x2D ||
          BX_NVRIVA_THIS crtc.index == 0x41 ||
          BX_NVRIVA_THIS crtc.index == 0x42) {
        BX_NVRIVA_THIS svga_needs_update_mode = 1;
      }
      if (BX_NVRIVA_THIS crtc.index <= VGA_CRTC_MAX) {
        BX_NVRIVA_THIS crtc.reg[BX_NVRIVA_THIS crtc.index] = value;
      } else {
        BX_NVRIVA_THIS svga_write_crtc(address, BX_NVRIVA_THIS crtc.index, value);
        if (BX_NVRIVA_THIS crtc.index == 0x25 ||
            BX_NVRIVA_THIS crtc.index == 0x2D ||
            BX_NVRIVA_THIS crtc.index == 0x41) {
          BX_NVRIVA_THIS calculate_retrace_timing();
        }
        return;
      }
      break;
    default:
      break;
  }

  VGA_WRITE(address,value,io_len);
}

Bit8u nvriva_alpha_wrap(int value)
{
  return -(value >> 8) ^ value;
}

void bx_nvriva_c::draw_hardware_cursor(unsigned xc, unsigned yc, bx_svga_tileinfo_t *info)
{
  Bit16s hwcx = BX_NVRIVA_THIS hw_cursor.x;
  Bit16s hwcy = BX_NVRIVA_THIS hw_cursor.y;
  Bit8u size = BX_NVRIVA_THIS hw_cursor.size;

  if (BX_NVRIVA_THIS svga_double_width) {
    hwcx <<= 1;
    hwcy <<= 1;
    size <<= 1;
  }

  unsigned w, h;
  Bit8u* tile_ptr;
  if (info->snapshot_mode) {
    tile_ptr = bx_gui->get_snapshot_buffer();
    w = BX_NVRIVA_THIS svga_xres;
    h = BX_NVRIVA_THIS svga_yres;
  } else {
    tile_ptr = bx_gui->graphics_tile_get(xc, yc, &w, &h);
  }
  if (BX_NVRIVA_THIS hw_cursor.enabled &&
      (int)xc < hwcx + size &&
      (int)(xc + w) > hwcx &&
      (int)yc < hwcy + size &&
      (int)(yc + h) > hwcy) {
    unsigned cx0 = hwcx > (int)xc ? hwcx : xc;
    unsigned cy0 = hwcy > (int)yc ? hwcy : yc;
    unsigned cx1 = hwcx + size < (int)(xc + w) ? hwcx + size : xc + w;
    unsigned cy1 = hwcy + size < (int)(yc + h) ? hwcy + size : yc + h;

    Bit8u display_color_bytes = BX_NVRIVA_THIS svga_bpp >> 3;
    Bit8u cursor_color_bytes = BX_NVRIVA_THIS hw_cursor.bpp32 ? 4 : 2;
    if (info->bpp == 15) info->bpp = 16;
    tile_ptr += info->pitch * (cy0 - yc) + info->bpp / 8 * (cx0 - xc);
    unsigned pitch = BX_NVRIVA_THIS hw_cursor.size * cursor_color_bytes;
    Bit8u* cursor_ptr = BX_NVRIVA_THIS s.memory + BX_NVRIVA_THIS hw_cursor.offset +
      (pitch * (cy0 - hwcy) >> (int)BX_NVRIVA_THIS s.y_doublescan);
    Bit8u* vid_ptr = BX_NVRIVA_THIS disp_ptr + (BX_NVRIVA_THIS svga_pitch * cy0 >>
      (int)BX_NVRIVA_THIS s.y_doublescan);
    for (unsigned cy = cy0; cy < cy1; cy++) {
      Bit8u* tile_ptr2 = tile_ptr;
      Bit8u* cursor_ptr2 = cursor_ptr + (cursor_color_bytes * (cx0 - hwcx) >>
        (int)BX_NVRIVA_THIS svga_double_width);
      Bit8u* vid_ptr2 = vid_ptr + (display_color_bytes * cx0 >>
        (int)BX_NVRIVA_THIS svga_double_width);
      for (unsigned cx = cx0; cx < cx1; cx++) {
        Bit8u dr, dg, db;
        if (display_color_bytes == 1) {
          if (info->is_indexed) {
            dr = dg = db = vid_ptr2[0];
          } else {
            Bit8u index = vid_ptr2[0];
            dr = (BX_NVRIVA_THIS s.pel.data[index].red << BX_NVRIVA_THIS s.dac_shift);
            dg = (BX_NVRIVA_THIS s.pel.data[index].green << BX_NVRIVA_THIS s.dac_shift);
            db = (BX_NVRIVA_THIS s.pel.data[index].blue << BX_NVRIVA_THIS s.dac_shift);
          }
        } else if (display_color_bytes == 2) {
          EXTRACT_565_TO_888(vid_ptr2[0] << 0 | vid_ptr2[1] << 8, dr, dg, db);
        } else {
          db = vid_ptr2[0];
          dg = vid_ptr2[1];
          dr = vid_ptr2[2];
        }
        Bit32u color;
        if (BX_NVRIVA_THIS hw_cursor.bpp32) {
          if (*(Bit32u*)cursor_ptr2) {
            Bit8u ica = 0xFF - cursor_ptr2[3];
            Bit8u b = nvriva_alpha_wrap(db * ica / 0xFF + cursor_ptr2[0]);
            Bit8u g = nvriva_alpha_wrap(dg * ica / 0xFF + cursor_ptr2[1]);
            Bit8u r = nvriva_alpha_wrap(dr * ica / 0xFF + cursor_ptr2[2]);
            color = b << 0 | g << 8 | r << 16;
          } else {
            color = db << 0 | dg << 8 | dr << 16;
          }
        } else {
          Bit8u alpha, cr, cg, cb;
          EXTRACT_1555_TO_8888(cursor_ptr2[0] << 0 | cursor_ptr2[1] << 8, alpha, cr, cg, cb);
          if (alpha) {
            color = cb << 0 | cg << 8 | cr << 16;
          } else {
            Bit8u b = db ^ cb;
            Bit8u g = dg ^ cg;
            Bit8u r = dr ^ cr;
            color = b << 0 | g << 8 | r << 16;
          }
        }
        if (!info->is_indexed) {
          color = MAKE_COLOUR(
            color, 24, info->red_shift, info->red_mask,
            color, 16, info->green_shift, info->green_mask,
            color, 8, info->blue_shift, info->blue_mask);
          if (info->is_little_endian) {
            for (int i = 0; i < info->bpp; i += 8) {
              *(tile_ptr2++) = (Bit8u)(color >> i);
            }
          } else {
            for (int i = info->bpp - 8; i > -8; i -= 8) {
              *(tile_ptr2++) = (Bit8u)(color >> i);
            }
          }
        } else {
          *(tile_ptr2++) = (Bit8u)color;
        }
        if (!BX_NVRIVA_THIS svga_double_width || (cx & 1)) {
          cursor_ptr2 += cursor_color_bytes;
          vid_ptr2 += display_color_bytes;
        }
      }
      tile_ptr += info->pitch;
      if (!BX_NVRIVA_THIS s.y_doublescan || (cy & 1)) {
        cursor_ptr += pitch;
        vid_ptr += BX_NVRIVA_THIS svga_pitch;
      }
    }
  }
}

Bit16u bx_nvriva_c::get_crtc_vtotal()
{
  return BX_NVRIVA_THIS crtc.reg[6] +
    ((BX_NVRIVA_THIS crtc.reg[7] & 0x01) << 8) +
    ((BX_NVRIVA_THIS crtc.reg[7] & 0x20) << 4) +
    ((BX_NVRIVA_THIS crtc.reg[0x25] & 1) << 10) +
    ((BX_NVRIVA_THIS crtc.reg[0x41] & 1) << 11) + 2;
}

void bx_nvriva_c::get_crtc_params(bx_crtc_params_t* crtcp, Bit32u* vclock)
{
  Bit32u m = BX_NVRIVA_THIS ramdac_vpll & 0xFF;
  Bit32u n = (BX_NVRIVA_THIS ramdac_vpll >> 8) & 0xFF;
  Bit32u p = (BX_NVRIVA_THIS ramdac_vpll >> 16) & 0x07;
  Bit32u mb = 1;
  Bit32u nb = 1;
  if (BX_NVRIVA_THIS card_type == 0x35) {
    m &= 0xF;
    if (BX_NVRIVA_THIS ramdac_vpll & 0x00000080) {
      mb = (BX_NVRIVA_THIS ramdac_vpll >> 4) & 0x7;
      nb = ((BX_NVRIVA_THIS ramdac_vpll >> 21) & 0x18) |
            ((BX_NVRIVA_THIS ramdac_vpll >> 19) & 0x7);
    }
  }
  if ((BX_NVRIVA_THIS ramdac_pll_select & 0x200) != 0 && m && mb) {
    Bit32u crystalFreq = 13500000;
    if (BX_NVRIVA_THIS straps0_primary & 0x00000040)
      crystalFreq = 14318000;
    if (BX_NVRIVA_THIS card_type > 0x20 && (BX_NVRIVA_THIS straps0_primary & 0x00400000) != 0)
      crystalFreq = 27000000;
    *vclock = (Bit32u)((Bit64u)crystalFreq * n * nb / m / mb >> p);
    crtcp->htotal = BX_NVRIVA_THIS crtc.reg[0] +
                    ((BX_NVRIVA_THIS crtc.reg[0x2D] & 1) << 8) + 5;
    crtcp->vtotal = get_crtc_vtotal();
    crtcp->vbstart = BX_NVRIVA_THIS crtc.reg[21] +
                     ((BX_NVRIVA_THIS crtc.reg[7] & 0x08) << 5) +
                     ((BX_NVRIVA_THIS crtc.reg[9] & 0x20) << 4) +
                     ((BX_NVRIVA_THIS crtc.reg[0x25] & 0x08) << 7) +
                     ((BX_NVRIVA_THIS crtc.reg[0x41] & 0x40) << 5);
    crtcp->vrstart = BX_NVRIVA_THIS crtc.reg[16] +
                     ((BX_NVRIVA_THIS crtc.reg[7] & 0x04) << 6) +
                     ((BX_NVRIVA_THIS crtc.reg[7] & 0x80) << 2) +
                     ((BX_NVRIVA_THIS crtc.reg[0x25] & 0x04) << 8) +
                     ((BX_NVRIVA_THIS crtc.reg[0x41] & 0x10) << 7);
  } else {
    bx_vgacore_c::get_crtc_params(crtcp, vclock);
  }
}

void bx_nvriva_c::update(void)
{
  Bit8u crtc28 = BX_NVRIVA_THIS crtc.reg[0x28];

  if (crtc28 & 0x80) {
    crtc28 &= 0x7F;
    // This should not happen - emulation provides single connected display
    BX_DEBUG(("Slaved mode activated"));
  }

  if (crtc28 == 0x00) {
    BX_NVRIVA_THIS bx_vgacore_c::update();
    return;
  }

  if (BX_NVRIVA_THIS svga_needs_update_mode) {
    Bit32u iTopOffset =
       BX_NVRIVA_THIS crtc.reg[0x0d] |
      (BX_NVRIVA_THIS crtc.reg[0x0c] << 8) |
      (BX_NVRIVA_THIS crtc.reg[0x19] & 0x1F) << 16;
    iTopOffset <<= 2;
    iTopOffset += BX_NVRIVA_THIS crtc_start;

    Bit32u iPitch =
       BX_NVRIVA_THIS crtc.reg[0x13] |
      (BX_NVRIVA_THIS crtc.reg[0x19] >> 5 << 8) |
      (BX_NVRIVA_THIS crtc.reg[0x42] >> 6 & 1) << 11;
    iPitch <<= 3;

    Bit8u iBpp = 0;
    if (crtc28 == 0x01)
      iBpp = 8;
    else if (crtc28 == 0x02)
      iBpp = 16;
    else if (crtc28 == 0x03)
      iBpp = 32;
    else
      BX_PANIC(("unknown bpp"));

    Bit32u iWidth =
      (BX_NVRIVA_THIS crtc.reg[1] +
      ((BX_NVRIVA_THIS crtc.reg[0x2D] & 0x02) << 7) + 1) * 8;
    Bit32u iHeight =
      (BX_NVRIVA_THIS crtc.reg[18] |
      ((BX_NVRIVA_THIS crtc.reg[7] & 0x02) << 7) |
      ((BX_NVRIVA_THIS crtc.reg[7] & 0x40) << 3) |
      ((BX_NVRIVA_THIS crtc.reg[0x25] & 0x02) << 9) |
      ((BX_NVRIVA_THIS crtc.reg[0x41] & 0x04) << 9)) + 1;

    if (BX_NVRIVA_THIS s.y_doublescan && iHeight > iWidth) {
      iWidth <<= 1;
      BX_NVRIVA_THIS svga_double_width = true;
    } else {
      BX_NVRIVA_THIS svga_double_width = false;
    }

    if (iWidth != BX_NVRIVA_THIS svga_xres ||
        iHeight != BX_NVRIVA_THIS svga_yres ||
        iBpp != BX_NVRIVA_THIS svga_bpp) {
      BX_INFO(("switched to %u x %u x %u", iWidth, iHeight, iBpp));
    }

    BX_NVRIVA_THIS svga_xres = iWidth;
    BX_NVRIVA_THIS svga_yres = iHeight;
    BX_NVRIVA_THIS svga_bpp = iBpp;
    BX_NVRIVA_THIS svga_dispbpp = iBpp;
    BX_NVRIVA_THIS disp_ptr = BX_NVRIVA_THIS s.memory + iTopOffset;
    BX_NVRIVA_THIS disp_offset = iTopOffset;
    BX_NVRIVA_THIS svga_pitch = iPitch;
    // compatibilty settings for VGA core
    BX_NVRIVA_THIS s.last_xres = iWidth;
    BX_NVRIVA_THIS s.last_yres = iHeight;
    BX_NVRIVA_THIS s.last_bpp = iBpp;
    BX_NVRIVA_THIS s.last_fh = 0;
  }

  unsigned width, height, pitch;

  if (BX_NVRIVA_THIS svga_dispbpp != 4) {
    width  = BX_NVRIVA_THIS svga_xres;
    height = BX_NVRIVA_THIS svga_yres;
    pitch = BX_NVRIVA_THIS svga_pitch;
    if (BX_NVRIVA_THIS svga_needs_update_mode) {
      bx_gui->dimension_update(width, height, 0, 0, BX_NVRIVA_THIS svga_dispbpp);
      BX_NVRIVA_THIS s.last_bpp = BX_NVRIVA_THIS svga_dispbpp;
      BX_NVRIVA_THIS svga_needs_update_mode = 0;
      BX_NVRIVA_THIS svga_needs_update_dispentire = 1;
    }
  } else {
    BX_NVRIVA_THIS determine_screen_dimensions(&height, &width);
    pitch = BX_NVRIVA_THIS s.line_offset;
    if ((width != BX_NVRIVA_THIS s.last_xres) || (height != BX_NVRIVA_THIS s.last_yres) ||
        (BX_NVRIVA_THIS s.last_bpp > 8)) {
      bx_gui->dimension_update(width, height);
      BX_NVRIVA_THIS s.last_xres = width;
      BX_NVRIVA_THIS s.last_yres = height;
      BX_NVRIVA_THIS s.last_bpp = 8;
    }
  }

  if (BX_NVRIVA_THIS svga_needs_update_dispentire) {
    BX_NVRIVA_THIS redraw_area_d(0, 0, width, height);
    BX_NVRIVA_THIS svga_needs_update_dispentire = 0;
  }

  if (!BX_NVRIVA_THIS svga_needs_update_tile)
    return;

  BX_NVRIVA_THIS svga_needs_update_tile = 0;

  unsigned xc, yc, xti, yti, hp;
  unsigned r, c, w, h, x, y;
  int i;
  Bit8u red, green, blue;
  Bit32u colour, row_addr;
  Bit8u * vid_ptr, * vid_ptr2;
  Bit8u * tile_ptr, * tile_ptr2;
  bx_svga_tileinfo_t info;
  Bit8u dac_size = (BX_NVRIVA_THIS s.dac_shift == 0) ? 8 : 6;

  if (bx_gui->graphics_tile_info_common(&info)) {
    if (info.snapshot_mode) {
      vid_ptr = BX_NVRIVA_THIS disp_ptr;
      tile_ptr = bx_gui->get_snapshot_buffer();
      if (tile_ptr != NULL) {
        for (yc = 0; yc < height; yc++) {
          vid_ptr2  = vid_ptr;
          tile_ptr2 = tile_ptr;
          if (BX_NVRIVA_THIS svga_dispbpp != 4) {
            for (xc = 0; xc < width; xc++) {
              memcpy(tile_ptr2, vid_ptr2, (BX_NVRIVA_THIS svga_bpp >> 3));
              if (!BX_NVRIVA_THIS svga_double_width || (xc & 1)) {
                vid_ptr2 += (BX_NVRIVA_THIS svga_bpp >> 3);
              }
              tile_ptr2 += ((info.bpp + 1) >> 3);
            }
            if (!BX_NVRIVA_THIS s.y_doublescan || (yc & 1)) {
              vid_ptr += pitch;
            }
          } else {
            row_addr = BX_NVRIVA_THIS s.CRTC.start_addr + (yc * pitch);
            for (xc = 0; xc < width; xc++) {
              *(tile_ptr2++) = BX_NVRIVA_THIS get_vga_pixel(xc, yc, row_addr, 0xffff, 0, BX_NVRIVA_THIS s.memory);
            }
          }
          tile_ptr += info.pitch;
        }
        draw_hardware_cursor(0, 0, &info);
      }
    } else if (info.is_indexed) {
      switch (BX_NVRIVA_THIS svga_dispbpp) {
        case 4:
          for (yc=0, yti = 0; yc<height; yc+=Y_TILESIZE, yti++) {
            for (xc=0, xti = 0; xc<width; xc+=X_TILESIZE, xti++) {
              if (GET_TILE_UPDATED (xti, yti)) {
                tile_ptr = bx_gui->graphics_tile_get(xc, yc, &w, &h);
                for (r=0; r<h; r++) {
                  y = yc + r;
                  if (BX_NVRIVA_THIS s.y_doublescan) y >>= 1;
                  row_addr = BX_NVRIVA_THIS s.CRTC.start_addr + (y * pitch);
                  tile_ptr2 = tile_ptr;
                  for (c=0; c<w; c++) {
                    x = xc + c;
                    *(tile_ptr2++) = BX_NVRIVA_THIS get_vga_pixel(x, y, row_addr, 0xffff, 0, BX_NVRIVA_THIS s.memory);
                  }
                  tile_ptr += info.pitch;
                }
                draw_hardware_cursor(xc, yc, &info);
                bx_gui->graphics_tile_update_in_place(xc, yc, w, h);
                SET_TILE_UPDATED(BX_NVRIVA_THIS, xti, yti, 0);
              }
            }
          }
          break;
        case 15:
        case 16:
        case 24:
        case 32:
          BX_ERROR(("current guest pixel format is unsupported on indexed colour host displays, svga_dispbpp=%d",
            BX_NVRIVA_THIS svga_dispbpp));
          break;
        case 8:
          hp = BX_NVRIVA_THIS s.attribute_ctrl.horiz_pel_panning & 0x07;
          for (yc=0, yti = 0; yc<height; yc+=Y_TILESIZE, yti++) {
            for (xc=0, xti = 0; xc<width; xc+=X_TILESIZE, xti++) {
              if (GET_TILE_UPDATED (xti, yti)) {
                if (!BX_NVRIVA_THIS s.y_doublescan) {
                  vid_ptr = BX_NVRIVA_THIS disp_ptr + (yc * pitch + xc + hp);
                } else {
                  if (!BX_NVRIVA_THIS svga_double_width) {
                    vid_ptr = BX_NVRIVA_THIS disp_ptr + ((yc >> 1) * pitch + xc + hp);
                  } else {
                    vid_ptr = BX_NVRIVA_THIS disp_ptr + ((yc >> 1) * pitch + ((xc + hp) >> 1));
                  }
                }
                tile_ptr = bx_gui->graphics_tile_get(xc, yc, &w, &h);
                for (r=0; r<h; r++) {
                  vid_ptr2  = vid_ptr;
                  tile_ptr2 = tile_ptr;
                  for (c=0; c<w; c++) {
                    colour = *(vid_ptr2);
                    if (!BX_NVRIVA_THIS svga_double_width || (c & 1)) {
                      vid_ptr2++;
                    }
                    *(tile_ptr2++) = colour;
                  }
                  if (!BX_NVRIVA_THIS s.y_doublescan || (r & 1)) {
                    vid_ptr += pitch;
                  }
                  tile_ptr += info.pitch;
                }
                draw_hardware_cursor(xc, yc, &info);
                bx_gui->graphics_tile_update_in_place(xc, yc, w, h);
                SET_TILE_UPDATED(BX_NVRIVA_THIS, xti, yti, 0);
              }
            }
          }
          break;
      }
    }
    else {
      switch (BX_NVRIVA_THIS svga_dispbpp) {
        case 4:
          for (yc=0, yti=0; yc<height; yc+=Y_TILESIZE, yti++) {
            for (xc=0, xti=0; xc<width; xc+=X_TILESIZE, xti++) {
              if (GET_TILE_UPDATED (xti, yti)) {
                tile_ptr = bx_gui->graphics_tile_get(xc, yc, &w, &h);
                for (r=0; r<Y_TILESIZE; r++) {
                  tile_ptr2 = tile_ptr;
                  y = yc + r;
                  if (BX_NVRIVA_THIS s.y_doublescan) y >>= 1;
                  row_addr = BX_NVRIVA_THIS s.CRTC.start_addr + (y * pitch);
                  for (c=0; c<X_TILESIZE; c++) {
                    x = xc + c;
                    colour = BX_NVRIVA_THIS get_vga_pixel(x, y, row_addr, 0xffff, 0, BX_NVRIVA_THIS s.memory);
                    colour = MAKE_COLOUR(
                      BX_NVRIVA_THIS s.pel.data[colour].red, 6, info.red_shift, info.red_mask,
                      BX_NVRIVA_THIS s.pel.data[colour].green, 6, info.green_shift, info.green_mask,
                      BX_NVRIVA_THIS s.pel.data[colour].blue, 6, info.blue_shift, info.blue_mask);
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
                SET_TILE_UPDATED(BX_NVRIVA_THIS, xti, yti, 0);
              }
            }
          }
          break;
        case 8:
          hp = BX_NVRIVA_THIS s.attribute_ctrl.horiz_pel_panning & 0x07;
          for (yc=0, yti = 0; yc<height; yc+=Y_TILESIZE, yti++) {
            for (xc=0, xti = 0; xc<width; xc+=X_TILESIZE, xti++) {
              if (GET_TILE_UPDATED (xti, yti)) {
                if (!BX_NVRIVA_THIS s.y_doublescan) {
                  vid_ptr = BX_NVRIVA_THIS disp_ptr + (yc * pitch + xc + hp);
                } else {
                  if (!BX_NVRIVA_THIS svga_double_width) {
                    vid_ptr = BX_NVRIVA_THIS disp_ptr + ((yc >> 1) * pitch + xc + hp);
                  } else {
                    vid_ptr = BX_NVRIVA_THIS disp_ptr + ((yc >> 1) * pitch + ((xc + hp) >> 1));
                  }
                }
                tile_ptr = bx_gui->graphics_tile_get(xc, yc, &w, &h);
                for (r=0; r<h; r++) {
                  vid_ptr2  = vid_ptr;
                  tile_ptr2 = tile_ptr;
                  for (c=0; c<w; c++) {
                    colour = *(vid_ptr2);
                    if (!BX_NVRIVA_THIS svga_double_width || (c & 1)) {
                      vid_ptr2++;
                    }
                    colour = MAKE_COLOUR(
                      BX_NVRIVA_THIS s.pel.data[colour].red, dac_size, info.red_shift, info.red_mask,
                      BX_NVRIVA_THIS s.pel.data[colour].green, dac_size, info.green_shift, info.green_mask,
                      BX_NVRIVA_THIS s.pel.data[colour].blue, dac_size, info.blue_shift, info.blue_mask);
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
                  if (!BX_NVRIVA_THIS s.y_doublescan || (r & 1)) {
                    vid_ptr += pitch;
                  }
                  tile_ptr += info.pitch;
                }
                draw_hardware_cursor(xc, yc, &info);
                bx_gui->graphics_tile_update_in_place(xc, yc, w, h);
                SET_TILE_UPDATED(BX_NVRIVA_THIS, xti, yti, 0);
              }
            }
          }
          break;
        case 15:
          hp = BX_NVRIVA_THIS s.attribute_ctrl.horiz_pel_panning & 0x01;
          for (yc=0, yti = 0; yc<height; yc+=Y_TILESIZE, yti++) {
            for (xc=0, xti = 0; xc<width; xc+=X_TILESIZE, xti++) {
              if (GET_TILE_UPDATED (xti, yti)) {
                if (!BX_NVRIVA_THIS s.y_doublescan) {
                  vid_ptr = BX_NVRIVA_THIS disp_ptr + (yc * pitch + ((xc + hp) << 1));
                } else {
                  if (!BX_NVRIVA_THIS svga_double_width) {
                    vid_ptr = BX_NVRIVA_THIS disp_ptr + ((yc >> 1) * pitch + ((xc + hp) << 1));
                  } else {
                    vid_ptr = BX_NVRIVA_THIS disp_ptr + ((yc >> 1) * pitch + xc + (hp << 1));
                  }
                }
                tile_ptr = bx_gui->graphics_tile_get(xc, yc, &w, &h);
                for (r=0; r<h; r++) {
                  vid_ptr2  = vid_ptr;
                  tile_ptr2 = tile_ptr;
                  for (c=0; c<w; c++) {
                    colour = *(vid_ptr2);
                    colour |= *(vid_ptr2+1) << 8;
                    if (!BX_NVRIVA_THIS svga_double_width || (c & 1)) {
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
                  if (!BX_NVRIVA_THIS s.y_doublescan || (r & 1)) {
                    vid_ptr  += pitch;
                  }
                  tile_ptr += info.pitch;
                }
                draw_hardware_cursor(xc, yc, &info);
                bx_gui->graphics_tile_update_in_place(xc, yc, w, h);
                SET_TILE_UPDATED(BX_NVRIVA_THIS, xti, yti, 0);
              }
            }
          }
          break;
        case 16:
          hp = BX_NVRIVA_THIS s.attribute_ctrl.horiz_pel_panning & 0x01;
          for (yc=0, yti = 0; yc<height; yc+=Y_TILESIZE, yti++) {
            for (xc=0, xti = 0; xc<width; xc+=X_TILESIZE, xti++) {
              if (GET_TILE_UPDATED (xti, yti)) {
                if (!BX_NVRIVA_THIS s.y_doublescan) {
                  vid_ptr = BX_NVRIVA_THIS disp_ptr + (yc * pitch + ((xc + hp) << 1));
                } else {
                  if (!BX_NVRIVA_THIS svga_double_width) {
                    vid_ptr = BX_NVRIVA_THIS disp_ptr + ((yc >> 1) * pitch + ((xc + hp) << 1));
                  } else {
                    vid_ptr = BX_NVRIVA_THIS disp_ptr + ((yc >> 1) * pitch + xc + (hp << 1));
                  }
                }
                tile_ptr = bx_gui->graphics_tile_get(xc, yc, &w, &h);
                for (r=0; r<h; r++) {
                  vid_ptr2  = vid_ptr;
                  tile_ptr2 = tile_ptr;
                  for (c=0; c<w; c++) {
                    colour = *(vid_ptr2);
                    colour |= *(vid_ptr2+1) << 8;
                    if (!BX_NVRIVA_THIS svga_double_width || (c & 1)) {
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
                  if (!BX_NVRIVA_THIS s.y_doublescan || (r & 1)) {
                    vid_ptr  += pitch;
                  }
                  tile_ptr += info.pitch;
                }
                draw_hardware_cursor(xc, yc, &info);
                bx_gui->graphics_tile_update_in_place(xc, yc, w, h);
                SET_TILE_UPDATED(BX_NVRIVA_THIS, xti, yti, 0);
              }
            }
          }
          break;
        case 24:
          for (yc=0, yti = 0; yc<height; yc+=Y_TILESIZE, yti++) {
            for (xc=0, xti = 0; xc<width; xc+=X_TILESIZE, xti++) {
              if (GET_TILE_UPDATED (xti, yti)) {
                if (!BX_NVRIVA_THIS s.y_doublescan) {
                  vid_ptr = BX_NVRIVA_THIS disp_ptr + (yc * pitch + 3 * xc);
                } else {
                  if (!BX_NVRIVA_THIS svga_double_width) {
                    vid_ptr = BX_NVRIVA_THIS disp_ptr + ((yc >> 1) * pitch + 3 * xc);
                  } else {
                    vid_ptr = BX_NVRIVA_THIS disp_ptr + ((yc >> 1) * pitch + 3 * (xc >> 1));
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
                    if (!BX_NVRIVA_THIS svga_double_width || (c & 1)) {
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
                  if (!BX_NVRIVA_THIS s.y_doublescan || (r & 1)) {
                    vid_ptr  += pitch;
                  }
                  tile_ptr += info.pitch;
                }
                draw_hardware_cursor(xc, yc, &info);
                bx_gui->graphics_tile_update_in_place(xc, yc, w, h);
                SET_TILE_UPDATED(BX_NVRIVA_THIS, xti, yti, 0);
              }
            }
          }
          break;
        case 32:
          for (yc=0, yti = 0; yc<height; yc+=Y_TILESIZE, yti++) {
            for (xc=0, xti = 0; xc<width; xc+=X_TILESIZE, xti++) {
              if (GET_TILE_UPDATED (xti, yti)) {
                if (!BX_NVRIVA_THIS s.y_doublescan) {
                  vid_ptr = BX_NVRIVA_THIS disp_ptr + (yc * pitch + (xc << 2));
                } else {
                  if (!BX_NVRIVA_THIS svga_double_width) {
                    vid_ptr = BX_NVRIVA_THIS disp_ptr + ((yc >> 1) * pitch + (xc << 2));
                  } else {
                    vid_ptr = BX_NVRIVA_THIS disp_ptr + ((yc >> 1) * pitch + (xc << 1));
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
                    if (!BX_NVRIVA_THIS svga_double_width || (c & 1)) {
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
                  if (!BX_NVRIVA_THIS s.y_doublescan || (r & 1)) {
                    vid_ptr  += pitch;
                  }
                  tile_ptr += info.pitch;
                }
                draw_hardware_cursor(xc, yc, &info);
                bx_gui->graphics_tile_update_in_place(xc, yc, w, h);
                SET_TILE_UPDATED(BX_NVRIVA_THIS, xti, yti, 0);
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

Bit8u bx_nvriva_c::svga_read_crtc(Bit32u address, unsigned index)
{
  if (index <= NVRIVA_CRTC_MAX) {
    Bit8u value = BX_NVRIVA_THIS crtc.reg[index];
    BX_DEBUG(("crtc: index 0x%02x read 0x%02x", index, value));
    return value;
  }
  else
    BX_PANIC(("crtc: unknown index 0x%02x read", index));

  return 0xff;
}

void bx_nvriva_c::svga_write_crtc(Bit32u address, unsigned index, Bit8u value)
{
  if (index == 0x40) {
    if (BX_NVRIVA_THIS crtc.reg[index] != value)
      BX_DEBUG(("crtc: index 0x%02x write 0x%02x [no duplicates]", index, (unsigned)value));
  } else
    BX_DEBUG(("crtc: index 0x%02x write 0x%02x", index, (unsigned)value));

  bool update_cursor_addr = false;

  if (index == 0x1c) {
    if (!(BX_NVRIVA_THIS crtc.reg[index] & 0x80) && (value & 0x80) != 0) {
      // Without clearing this register, Windows 95 hangs after reboot
      BX_NVRIVA_THIS crtc_intr_en = 0x00000000;
      update_irq_level();
    }
  } else if (index == 0x1d || index == 0x1e)
    BX_NVRIVA_THIS bank_base[index - 0x1d] = value * 0x8000;
  else if (index == 0x2f || index == 0x30 || index == 0x31)
    update_cursor_addr = true;
  else if (index == 0x37 || index == 0x3f || index == 0x51) {
    bool scl = value & 0x20;
    bool sda = value & 0x10;
    if (index == 0x3f) {
      BX_NVRIVA_THIS ddc.write(scl, sda);
      BX_NVRIVA_THIS crtc.reg[0x3e] = BX_NVRIVA_THIS ddc.read() & 0x0c;
    } else {
      BX_NVRIVA_THIS crtc.reg[index - 1] = sda << 3 | scl << 2;
    }
  } else if (index == 0x58) {
    // Combined with 0x57, this register makes pair which allows
    //   to access 16 bytes of head-specific variables.
    // Until visible side effects appear, however, it is better to
    //   just disable writes to it which makes reads to return zeroes.
    return;
  }

  if (index <= NVRIVA_CRTC_MAX) {
    BX_NVRIVA_THIS crtc.reg[index] = value;
  }
  else
    BX_PANIC(("crtc: unknown index 0x%02x write", index));

  if (update_cursor_addr) {
    bool prev_enabled = BX_NVRIVA_THIS hw_cursor.enabled;
    BX_NVRIVA_THIS hw_cursor.enabled =
      (BX_NVRIVA_THIS crtc.reg[0x31] & 0x01) ||
      (BX_NVRIVA_THIS crtc_cursor_config & 0x00000001);
    BX_NVRIVA_THIS hw_cursor.offset =
      (BX_NVRIVA_THIS crtc.reg[0x31] >> 2 << 11) |
      (BX_NVRIVA_THIS crtc.reg[0x30] & 0x7F) << 17 |
       BX_NVRIVA_THIS crtc.reg[0x2f] << 24;
    BX_NVRIVA_THIS hw_cursor.offset += BX_NVRIVA_THIS crtc_cursor_offset;
    if (prev_enabled != BX_NVRIVA_THIS hw_cursor.enabled) {
      BX_NVRIVA_THIS redraw_area_nd(BX_NVRIVA_THIS hw_cursor.x, BX_NVRIVA_THIS hw_cursor.y,
        BX_NVRIVA_THIS hw_cursor.size, BX_NVRIVA_THIS hw_cursor.size);
    }
  }
}

Bit8u bx_nvriva_c::register_read8(Bit32u address)
{
  Bit8u value;
  if (address >= 0x1800 && address < 0x1900) {
    value = BX_NVRIVA_THIS pci_conf[address - 0x1800];
  } else if (address >= 0x300000 && address < 0x310000) {
    if (BX_NVRIVA_THIS pci_conf[0x50] == 0x00)
      value = BX_NVRIVA_THIS pci_rom[address - 0x300000];
    else
      value = 0x00;
  } else if ((address >= 0xc0300 && address < 0xc0400) ||
             (address >= 0xc2300 && address < 0xc2400)) {
    Bit32u head = (address >> 13) & 1;
    Bit32u offset = address & 0x00000fff;
    if (offset == 0x3c3 ||
        offset == 0x3c4 || offset == 0x3c5 ||
        offset == 0x3cc || offset == 0x3cf) {
      if (!head)
        value = SVGA_READ(offset, 1);
      else
        value = 0x00;
    } else {
      value = 0xFF;
      BX_PANIC(("Unknown register 0x%08x read", address));
    }
  } else if ((address >= 0x601300 && address < 0x601400) ||
             (address >= 0x603300 && address < 0x603400)) {
    Bit32u head = (address >> 13) & 1;
    Bit32u offset = address & 0x00000fff;
    if (offset == 0x3b4 || offset == 0x3b5 ||
        offset == 0x3c0 || offset == 0x3c1 ||
        offset == 0x3c2 || offset == 0x3d4 ||
        offset == 0x3d5 || offset == 0x3d8 ||
        offset == 0x3da) {
      if (!head)
        value = SVGA_READ(offset, 1);
      else
        value = 0x00;
    } else {
      value = 0xFF;
      BX_PANIC(("Unknown register 0x%08x read", address));
    }
  } else if ((address >= 0x681300 && address < 0x681400) ||
             (address >= 0x683300 && address < 0x683400)) {
    Bit32u head = (address >> 13) & 1;
    Bit32u offset = address & 0x00000fff;
    if (offset >= 0x3c6 && offset <= 0x3c9) {
      if (!head)
        value = SVGA_READ(offset, 1);
      else
        value = 0x00;
    } else {
      value = 0xFF;
      BX_PANIC(("Unknown register 0x%08x read", address));
    }
  } else if (address >= 0x700000 && address < 0x800000) {
    value = BX_NVRIVA_THIS s.memory[(address - 0x700000) ^ BX_NVRIVA_THIS ramin_flip];
  } else {
    value = register_read32(address);
  }
  return value;
}

void bx_nvriva_c::register_write8(Bit32u address, Bit8u value)
{
  if ((address >= 0xc0300 && address < 0xc0400) ||
      (address >= 0xc2300 && address < 0xc2400)) {
    Bit32u head = (address >> 13) & 1;
    Bit32u offset = address & 0x00000fff;
    if (offset == 0x3c2 || offset == 0x3c3 ||
        offset == 0x3c4 || offset == 0x3c5 ||
        offset == 0x3ce || offset == 0x3cf) {
      if (!head)
        SVGA_WRITE(offset, value, 1);
    } else {
      BX_PANIC(("Unknown register 0x%08x write", address));
    }
  } else if ((address >= 0x601300 && address < 0x601400) ||
             (address >= 0x603300 && address < 0x603400)) {
    Bit32u head = (address >> 13) & 1;
    Bit32u offset = address & 0x00000fff;
    if (offset == 0x3b4 || offset == 0x3b5 ||
        offset == 0x3c0 || offset == 0x3c1 ||
        offset == 0x3c2 || offset == 0x3d4 ||
        offset == 0x3d5 || offset == 0x3da) {
      if (!head)
        SVGA_WRITE(offset, value, 1);
    } else {
      BX_PANIC(("Unknown register 0x%08x write", address));
    }
  } else if ((address >= 0x681300 && address < 0x681400) ||
             (address >= 0x683300 && address < 0x683400)) {
    Bit32u head = (address >> 13) & 1;
    Bit32u offset = address & 0x00000fff;
    if (offset >= 0x3c6 && offset <= 0x3c9) {
      if (!head)
        SVGA_WRITE(offset, value, 1);
    } else {
      BX_PANIC(("Unknown register 0x%08x write", address));
    }
  } else if (address >= 0x700000 && address < 0x800000) {
    BX_NVRIVA_THIS s.memory[(address - 0x700000) ^ BX_NVRIVA_THIS ramin_flip] = value;
  } else {
    register_write32(address, (register_read32(address) & ~0xFF) | value);
  }
}

Bit8u bx_nvriva_c::vram_read8(Bit32u address)
{
  return BX_NVRIVA_THIS s.memory[address];
}

Bit16u bx_nvriva_c::vram_read16(Bit32u address)
{
  return
    BX_NVRIVA_THIS s.memory[address + 0] << 0 |
    BX_NVRIVA_THIS s.memory[address + 1] << 8;
}

Bit32u bx_nvriva_c::vram_read32(Bit32u address)
{
  return
    BX_NVRIVA_THIS s.memory[address + 0] << 0 |
    BX_NVRIVA_THIS s.memory[address + 1] << 8 |
    BX_NVRIVA_THIS s.memory[address + 2] << 16 |
    BX_NVRIVA_THIS s.memory[address + 3] << 24;
}

void bx_nvriva_c::vram_write8(Bit32u address, Bit8u value)
{
  BX_NVRIVA_THIS s.memory[address + 0] = value;
}

void bx_nvriva_c::vram_write16(Bit32u address, Bit16u value)
{
  BX_NVRIVA_THIS s.memory[address + 0] = (value >> 0) & 0xFF;
  BX_NVRIVA_THIS s.memory[address + 1] = (value >> 8) & 0xFF;
}

void bx_nvriva_c::vram_write32(Bit32u address, Bit32u value)
{
  BX_NVRIVA_THIS s.memory[address + 0] = (value >> 0) & 0xFF;
  BX_NVRIVA_THIS s.memory[address + 1] = (value >> 8) & 0xFF;
  BX_NVRIVA_THIS s.memory[address + 2] = (value >> 16) & 0xFF;
  BX_NVRIVA_THIS s.memory[address + 3] = (value >> 24) & 0xFF;
}

void bx_nvriva_c::vram_write64(Bit32u address, Bit64u value)
{
  BX_NVRIVA_THIS s.memory[address + 0] = (value >> 0) & 0xFF;
  BX_NVRIVA_THIS s.memory[address + 1] = (value >> 8) & 0xFF;
  BX_NVRIVA_THIS s.memory[address + 2] = (value >> 16) & 0xFF;
  BX_NVRIVA_THIS s.memory[address + 3] = (value >> 24) & 0xFF;
  BX_NVRIVA_THIS s.memory[address + 4] = (value >> 32) & 0xFF;
  BX_NVRIVA_THIS s.memory[address + 5] = (value >> 40) & 0xFF;
  BX_NVRIVA_THIS s.memory[address + 6] = (value >> 48) & 0xFF;
  BX_NVRIVA_THIS s.memory[address + 7] = (value >> 56) & 0xFF;
}

Bit8u bx_nvriva_c::ramin_read8(Bit32u address)
{
  return vram_read8(address ^ BX_NVRIVA_THIS ramin_flip);
}

Bit32u bx_nvriva_c::ramin_read32(Bit32u address)
{
  return vram_read32(address ^ BX_NVRIVA_THIS ramin_flip);
}

void bx_nvriva_c::ramin_write8(Bit32u address, Bit8u value)
{
  vram_write8(address ^ BX_NVRIVA_THIS ramin_flip, value);
}

void bx_nvriva_c::ramin_write32(Bit32u address, Bit32u value)
{
  vram_write32(address ^ BX_NVRIVA_THIS ramin_flip, value);
}

Bit8u bx_nvriva_c::physical_read8(Bit32u address)
{
  Bit8u data;
  DEV_MEM_READ_PHYSICAL(address, 1, &data);
  return data;
}

Bit16u bx_nvriva_c::physical_read16(Bit32u address)
{
  Bit8u data[2];
  DEV_MEM_READ_PHYSICAL(address, 2, data);
  return data[0] << 0 | data[1] << 8;
}

Bit32u bx_nvriva_c::physical_read32(Bit32u address)
{
  Bit8u data[4];
  DEV_MEM_READ_PHYSICAL(address, 4, data);
  return data[0] << 0 | data[1] << 8 | data[2] << 16 | data[3] << 24;
}

void bx_nvriva_c::physical_write8(Bit32u address, Bit8u value)
{
  DEV_MEM_WRITE_PHYSICAL(address, 1, &value);
}

void bx_nvriva_c::physical_write16(Bit32u address, Bit16u value)
{
#ifndef BX_LITTLE_ENDIAN
  Bit8u data[2];
  data[0] = (value >> 0) & 0xFF;
  data[1] = (value >> 8) & 0xFF;
#else
  Bit8u *data = (Bit8u *)(&value);
#endif
  DEV_MEM_WRITE_PHYSICAL(address, 2, data);
}

void bx_nvriva_c::physical_write32(Bit32u address, Bit32u value)
{
#ifndef BX_LITTLE_ENDIAN
  Bit8u data[4];
  data[0] = (value >> 0) & 0xFF;
  data[1] = (value >> 8) & 0xFF;
  data[2] = (value >> 16) & 0xFF;
  data[3] = (value >> 24) & 0xFF;
#else
  Bit8u *data = (Bit8u *)(&value);
#endif
  DEV_MEM_WRITE_PHYSICAL(address, 4, data);
}

void bx_nvriva_c::physical_write64(Bit32u address, Bit64u value)
{
#ifndef BX_LITTLE_ENDIAN
  Bit8u data[8];
  data[0] = (value >> 0) & 0xFF;
  data[1] = (value >> 8) & 0xFF;
  data[2] = (value >> 16) & 0xFF;
  data[3] = (value >> 24) & 0xFF;
  data[4] = (value >> 32) & 0xFF;
  data[5] = (value >> 40) & 0xFF;
  data[6] = (value >> 48) & 0xFF;
  data[7] = (value >> 56) & 0xFF;
#else
  Bit8u *data = (Bit8u *)(&value);
#endif
  DEV_MEM_WRITE_PHYSICAL(address, 8, data);
}

Bit32u bx_nvriva_c::dma_pt_lookup(Bit32u object, Bit32u address)
{
  Bit32u address_adj = address + (ramin_read32(object) >> 20);
  Bit32u page_offset = address_adj & 0xFFF;
  Bit32u page_index = address_adj >> 12;
  Bit32u page = ramin_read32(object + 8 + page_index * 4) & 0xFFFFF000;
  return page | page_offset;
}

Bit32u bx_nvriva_c::dma_lin_lookup(Bit32u object, Bit32u address)
{
  Bit32u adjust = ramin_read32(object) >> 20;
  Bit32u base = ramin_read32(object + 8) & 0xFFFFF000;
  return base + adjust + address;
}

Bit8u bx_nvriva_c::dma_read8(Bit32u object, Bit32u address)
{
  Bit32u flags = ramin_read32(object);
  Bit32u addr_abs;
  if (flags & 0x00002000)
    addr_abs = dma_lin_lookup(object, address);
  else
    addr_abs = dma_pt_lookup(object, address);
  if (flags & 0x00020000)
    return physical_read8(addr_abs);
  else
    return vram_read8(addr_abs);
}

Bit16u bx_nvriva_c::dma_read16(Bit32u object, Bit32u address)
{
  Bit32u flags = ramin_read32(object);
  Bit32u addr_abs;
  if (flags & 0x00002000)
    addr_abs = dma_lin_lookup(object, address);
  else
    addr_abs = dma_pt_lookup(object, address);
  if (flags & 0x00020000)
    return physical_read16(addr_abs);
  else
    return vram_read16(addr_abs);
}

Bit32u bx_nvriva_c::dma_read32(Bit32u object, Bit32u address)
{
  Bit32u flags = ramin_read32(object);
  Bit32u addr_abs;
  if (flags & 0x00002000)
    addr_abs = dma_lin_lookup(object, address);
  else
    addr_abs = dma_pt_lookup(object, address);
  if (flags & 0x00020000)
    return physical_read32(addr_abs);
  else
    return vram_read32(addr_abs);
}

void bx_nvriva_c::dma_write8(Bit32u object, Bit32u address, Bit8u value)
{
  Bit32u flags = ramin_read32(object);
  Bit32u addr_abs;
  if (flags & 0x00002000)
    addr_abs = dma_lin_lookup(object, address);
  else
    addr_abs = dma_pt_lookup(object, address);
  if (flags & 0x00020000)
    physical_write8(addr_abs, value);
  else
    vram_write8(addr_abs, value);
}

void bx_nvriva_c::dma_write16(Bit32u object, Bit32u address, Bit16u value)
{
  Bit32u flags = ramin_read32(object);
  Bit32u addr_abs;
  if (flags & 0x00002000)
    addr_abs = dma_lin_lookup(object, address);
  else
    addr_abs = dma_pt_lookup(object, address);
  if (flags & 0x00020000)
    physical_write16(addr_abs, value);
  else
    vram_write16(addr_abs, value);
}

void bx_nvriva_c::dma_write32(Bit32u object, Bit32u address, Bit32u value)
{
  Bit32u flags = ramin_read32(object);
  Bit32u addr_abs;
  if (flags & 0x00002000)
    addr_abs = dma_lin_lookup(object, address);
  else
    addr_abs = dma_pt_lookup(object, address);
  if (flags & 0x00020000)
    physical_write32(addr_abs, value);
  else
    vram_write32(addr_abs, value);
}

void bx_nvriva_c::dma_write64(Bit32u object, Bit32u address, Bit64u value)
{
  Bit32u flags = ramin_read32(object);
  Bit32u addr_abs;
  if (flags & 0x00002000)
    addr_abs = dma_lin_lookup(object, address);
  else
    addr_abs = dma_pt_lookup(object, address);
  if (flags & 0x00020000)
    physical_write64(addr_abs, value);
  else
    vram_write64(addr_abs, value);
}

void bx_nvriva_c::dma_copy(Bit32u dst_obj, Bit32u dst_addr,
  Bit32u src_obj, Bit32u src_addr, Bit32u byte_count)
{
  Bit32u dst_flags = ramin_read32(dst_obj);
  Bit32u src_flags = ramin_read32(src_obj);
  Bit8u buffer[0x1000];
  Bit32u bytes_left = byte_count;
  while (bytes_left) {
    Bit32u dst_addr_abs;
    Bit32u src_addr_abs;
    if (dst_flags & 0x00002000)
      dst_addr_abs = dma_lin_lookup(dst_obj, dst_addr);
    else
      dst_addr_abs = dma_pt_lookup(dst_obj, dst_addr);
    if (src_flags & 0x00002000)
      src_addr_abs = dma_lin_lookup(src_obj, src_addr);
    else
      src_addr_abs = dma_pt_lookup(src_obj, src_addr);
    Bit32u chunk_bytes = BX_MIN(bytes_left, BX_MIN(
      0x1000 - (dst_addr_abs & 0xFFF),
      0x1000 - (src_addr_abs & 0xFFF)));
    if (src_flags & 0x00020000)
      DEV_MEM_READ_PHYSICAL(src_addr_abs, chunk_bytes, buffer);
    else
      memcpy(buffer, BX_NVRIVA_THIS s.memory + src_addr_abs, chunk_bytes);
    if (dst_flags & 0x00020000)
      DEV_MEM_WRITE_PHYSICAL(dst_addr_abs, chunk_bytes, buffer);
    else
      memcpy(BX_NVRIVA_THIS s.memory + dst_addr_abs, buffer, chunk_bytes);
    dst_addr += chunk_bytes;
    src_addr += chunk_bytes;
    bytes_left -= chunk_bytes;
  }
}

Bit32u bx_nvriva_c::ramfc_address(Bit32u chid, Bit32u offset)
{
  Bit32u ramfc;
  ramfc = (BX_NVRIVA_THIS fifo_ramfc & 0xFFF) << 16;
  Bit32u ramfc_ch_size = 0x80;
  return ramfc + chid * ramfc_ch_size + offset;
}

void bx_nvriva_c::ramfc_write32(Bit32u chid, Bit32u offset, Bit32u value)
{
  ramin_write32(ramfc_address(chid, offset), value);
}

Bit32u bx_nvriva_c::ramfc_read32(Bit32u chid, Bit32u offset)
{
  return ramin_read32(ramfc_address(chid, offset));
}

void bx_nvriva_c::ramht_lookup(Bit32u handle, Bit32u chid, Bit32u* object, Bit8u* engine)
{
  Bit32u ramht_addr = (BX_NVRIVA_THIS fifo_ramht & 0xFFF) << 8;
  Bit32u ramht_bits = ((BX_NVRIVA_THIS fifo_ramht >> 16) & 0xFF) + 9;
  Bit32u ramht_size = 1 << ramht_bits << 3;

  Bit32u hash = 0;
  Bit32u x = handle;
  while (x) {
    hash ^= (x & ((1 << ramht_bits) - 1));
    x >>= ramht_bits;
  }
  hash ^= (chid & 0xF) << (ramht_bits - 4);
  hash = hash << 3;

  Bit32u it = hash;
  Bit32u steps = 1; // for debugging
  do {
    if (ramin_read32(ramht_addr + it) == handle) {
      Bit32u context = ramin_read32(ramht_addr + it + 4);
      Bit32u ctx_chid;
      ctx_chid = (context >> 23) & 0x1F;
      if (chid == ctx_chid) {
        BX_DEBUG(("ramht_lookup: 0x%08x -> 0x%08x, steps: %d", handle, context, steps));
        if (object) {
          *object = (context & 0xFFFFF) << 4;
        }
        if (engine) {
          *engine = (context >> 20) & 0x7;
        }
        return;
      }
    }
    steps++;
    it += 8;
    if (it >= ramht_size)
      it = 0;
  } while (it != hash);

  BX_PANIC(("ramht_lookup failed for 0x%08x", handle));
}

Bit32u nvriva_color_565_to_888(Bit16u value)
{
  Bit8u r, g, b;
  EXTRACT_565_TO_888(value, r, g, b);
  return r << 16 | g << 8 | b;
}

Bit16u nvriva_color_888_to_565(Bit32u value)
{
  return (((value >> 19) & 0x1F) << 11) | (((value >> 10) & 0x3F) << 5) | ((value >> 3) & 0x1F);
}


Bit32u bx_nvriva_c::get_pixel(Bit32u obj, Bit32u ofs, Bit32u x, Bit32u cb)
{
  Bit32u result;
  if (cb == 1)
    result = dma_read8(obj, ofs + x);
  else if (cb == 2)
    result = dma_read16(obj, ofs + x * 2);
  else
    result = dma_read32(obj, ofs + x * 4);
  return result;
}

void bx_nvriva_c::put_pixel(nv04_channel* ch, Bit32u ofs, Bit32u x, Bit32u value)
{
  if (ch->s2d_color_bytes == 1)
    dma_write8(ch->s2d_img_dst, ofs + x, value);
  else if (ch->s2d_color_bytes == 2)
    dma_write16(ch->s2d_img_dst, ofs + x * 2, value);
  else if (ch->s2d_color_fmt == 6)
    dma_write32(ch->s2d_img_dst, ofs + x * 4, value & 0x00FFFFFF);
  else
    dma_write32(ch->s2d_img_dst, ofs + x * 4, value);
}

void bx_nvriva_c::put_pixel_swzs(nv04_channel* ch, Bit32u ofs, Bit32u value)
{
  if (ch->swzs_color_bytes == 1)
    dma_write8(ch->swzs_img_obj, ofs, value);
  else if (ch->swzs_color_bytes == 2)
    dma_write16(ch->swzs_img_obj, ofs, value);
  else
    dma_write32(ch->swzs_img_obj, ofs, value);
}

void bx_nvriva_c::pixel_operation(nv04_channel* ch, Bit32u op,
  Bit32u* dstcolor, const Bit32u* srccolor, Bit32u cb, Bit32u px, Bit32u py)
{
  if (op == 1) {
    Bit8u rop = ch->rop;
    if (BX_NVRIVA_THIS rop_flags[rop]) {
      Bit32u i = py % 8 * 8 + px % 8;
      Bit32u patt_color = ch->patt_type == 1 ?
        ch->patt_data_mono[i] ? ch->patt_fg_color : ch->patt_bg_color :
        ch->patt_data_color[i];
      bx_ternary_rop(rop, (Bit8u*)dstcolor, (Bit8u*)srccolor, (Bit8u*)&patt_color, cb);
    } else {
      BX_NVRIVA_THIS rop_handler[rop]((Bit8u*)dstcolor, (Bit8u*)srccolor, 0, 0, cb, 1);
    }
  } else if (op == 5) {
    if (cb == 4) {
      if (*srccolor) {
        Bit8u sb = *srccolor;
        Bit8u sg = *srccolor >> 8;
        Bit8u sr = *srccolor >> 16;
        Bit8u sa = *srccolor >> 24;
        Bit32u beta = ch->beta;
        if (beta != 0xFFFFFFFF) {
          Bit8u bb = beta;
          Bit8u bg = beta >> 8;
          Bit8u br = beta >> 16;
          Bit8u ba = beta >> 24;
          sb = sb * bb / 0xFF;
          sg = sg * bg / 0xFF;
          sr = sr * br / 0xFF;
          sa = sa * ba / 0xFF;
        }
        Bit8u db = *dstcolor;
        Bit8u dg = *dstcolor >> 8;
        Bit8u dr = *dstcolor >> 16;
        Bit8u da = *dstcolor >> 24;
        Bit8u isa = 0xFF - sa;
        Bit8u b = nvriva_alpha_wrap(db * isa / 0xFF + sb);
        Bit8u g = nvriva_alpha_wrap(dg * isa / 0xFF + sg);
        Bit8u r = nvriva_alpha_wrap(dr * isa / 0xFF + sr);
        Bit8u a = nvriva_alpha_wrap(da * isa / 0xFF + sa);
        *dstcolor = b << 0 | g << 8 | r << 16 | a << 24;
      }
    } else {
      Bit32u beta = ch->beta;
      Bit8u bb = beta;
      Bit8u bg = beta >> 8;
      Bit8u br = beta >> 16;
      Bit8u iba = 0xFF - (beta >> 24);
      Bit8u sb = *srccolor & 0x1F;
      Bit8u sg = (*srccolor >> 5) & 0x3F;
      Bit8u sr = (*srccolor >> 11) & 0x1F;
      Bit8u db = *dstcolor & 0x1F;
      Bit8u dg = (*dstcolor >> 5) & 0x3F;
      Bit8u dr = (*dstcolor >> 11) & 0x1F;
      Bit8u b = (db * iba + sb * bb) / 0xFF;
      Bit8u g = (dg * iba + sg * bg) / 0xFF;
      Bit8u r = (dr * iba + sr * br) / 0xFF;
      *dstcolor = b << 0 | g << 5 | r << 11;
    }
  } else {
    *dstcolor = *srccolor;
  }
}

void bx_nvriva_c::gdi_fillrect(nv04_channel* ch, bool clipped)
{
  Bit16s clipx0;
  Bit16s clipy0;
  Bit16s clipx1;
  Bit16s clipy1;
  if (clipped) {
    clipx0 = ch->gdi_clip_yx0 & 0xFFFF;
    clipy0 = ch->gdi_clip_yx0 >> 16;
    clipx1 = ch->gdi_clip_yx1 & 0xFFFF;
    clipy1 = ch->gdi_clip_yx1 >> 16;
  }
  Bit16s dx;
  Bit16s dy;
  if (clipped) {
    dx = ch->gdi_rect_yx0 & 0xFFFF;
    dy = ch->gdi_rect_yx0 >> 16;
    clipx0 -= dx;
    clipy0 -= dy;
    clipx1 -= dx;
    clipy1 -= dy;
  } else {
    dx = ch->gdi_rect_xy >> 16;
    dy = ch->gdi_rect_xy & 0xFFFF;
  }
  Bit16u width;
  Bit16u height;
  if (clipped) {
    width = (ch->gdi_rect_yx1 & 0xFFFF) - dx;
    height = (ch->gdi_rect_yx1 >> 16) - dy;
  } else {
    width = ch->gdi_rect_wh >> 16;
    height = ch->gdi_rect_wh & 0xFFFF;
  }
  Bit32u pitch = ch->s2d_pitch_dst;
  Bit32u srccolor = ch->gdi_rect_color;
  Bit32u draw_offset = ch->s2d_ofs_dst +
    dy * pitch + dx * ch->s2d_color_bytes;
  Bit32u redraw_offset = dma_lin_lookup(ch->s2d_img_dst, draw_offset) -
    BX_NVRIVA_THIS disp_offset;
  for (Bit16u y = 0; y < height; y++) {
    for (Bit16u x = 0; x < width; x++) {
      if (!clipped || (x >= clipx0 && x < clipx1 && y >= clipy0 && y < clipy1)) {
        Bit32u dstcolor = get_pixel(ch->s2d_img_dst,
          draw_offset, x, ch->s2d_color_bytes);
        pixel_operation(ch, ch->gdi_operation,
          &dstcolor, &srccolor, ch->s2d_color_bytes, dx + x, dy + y);
        put_pixel(ch, draw_offset, x, dstcolor);
      }
    }
    draw_offset += pitch;
  }
  BX_NVRIVA_THIS redraw_area_nd(redraw_offset, width, height);
}

void bx_nvriva_c::gdi_blit(nv04_channel* ch, Bit32u type)
{
  Bit16s dx = ch->gdi_image_xy & 0xFFFF;
  Bit16s dy = ch->gdi_image_xy >> 16;
  Bit16s clipx0 = (ch->gdi_clip_yx0 & 0xFFFF) - dx;
  Bit16s clipy0 = (ch->gdi_clip_yx0 >> 16) - dy;
  Bit16s clipx1 = (ch->gdi_clip_yx1 & 0xFFFF) - dx;
  Bit16s clipy1 = (ch->gdi_clip_yx1 >> 16) - dy;
  Bit32u swidth = ch->gdi_image_swh & 0xFFFF;
  Bit32u dwidth = type ? ch->gdi_image_dwh & 0xFFFF : swidth;
  Bit32u height = ch->gdi_image_swh >> 16;
  Bit32u pitch = ch->s2d_pitch_dst;
  Bit32u bg_color = ch->gdi_bg_color;
  Bit32u fg_color = ch->gdi_fg_color;
  if (ch->s2d_color_bytes == 4 && ch->gdi_color_fmt != 3) {
    bg_color = nvriva_color_565_to_888(bg_color);
    fg_color = nvriva_color_565_to_888(fg_color);
  }
  Bit32u draw_offset = ch->s2d_ofs_dst +
    dy * pitch + dx * ch->s2d_color_bytes;
  Bit32u redraw_offset = dma_lin_lookup(ch->s2d_img_dst, draw_offset) -
    BX_NVRIVA_THIS disp_offset;
  Bit32u bit_index = 0;
  for (Bit16u y = 0; y < height; y++) {
    for (Bit16u x = 0; x < dwidth; x++) {
      if (x >= clipx0 && x < clipx1 && y >= clipy0 && y < clipy1) {
        Bit32u word_offset = bit_index / 32;
        Bit32u bit_offset = bit_index % 32;
        if (ch->gdi_mono_fmt == 1)
          bit_offset ^= 7;
        bool pixel = (ch->gdi_words[word_offset] >> bit_offset) & 1;
        if (type || (!type && pixel)) {
          Bit32u dstcolor = get_pixel(ch->s2d_img_dst,
            draw_offset, x, ch->s2d_color_bytes);
          Bit32u srccolor = pixel ? fg_color : bg_color;
          pixel_operation(ch, ch->gdi_operation,
            &dstcolor, &srccolor, ch->s2d_color_bytes, dx + x, dy + y);
          put_pixel(ch, draw_offset, x, dstcolor);
        }
      }
      bit_index++;
    }
    bit_index += swidth - dwidth;
    draw_offset += pitch;
  }
  BX_NVRIVA_THIS redraw_area_nd(redraw_offset, dwidth, height);
}

void bx_nvriva_c::ifc(nv04_channel* ch, Bit32u word)
{
  Bit32u chromacolor;
  bool chroma_enabled = false;
  if (ch->ifc_color_key_enable) {
    if (ch->ifc_color_bytes == 4) {
      chromacolor = ch->chroma_color & 0x00FFFFFF;
      chroma_enabled = ch->chroma_color & 0xFF000000;
    } else if (ch->ifc_color_bytes == 2) {
      chromacolor = ch->chroma_color & 0x0000FFFF;
      chroma_enabled = ch->chroma_color & 0xFFFF0000;
    } else {
      chromacolor = ch->chroma_color & 0x000000FF;
      chroma_enabled = ch->chroma_color & 0xFFFFFF00;
    }
  }
  for (Bit32u i = 0; i < ch->ifc_pixels_per_word; i++) {
    if (ch->ifc_x >= ch->ifc_clip_x0 && ch->ifc_x < ch->ifc_clip_x1 &&
        ch->ifc_y >= ch->ifc_clip_y0 && ch->ifc_y < ch->ifc_clip_y1) {
      Bit32u srccolor;
      if (ch->ifc_color_bytes == 4)
        srccolor = word;
      else if (ch->ifc_color_bytes == 2)
        srccolor = i == 0 ? word & 0xffff : word >> 16;
      else
        srccolor = (word >> (i * 8)) & 0xff;
      if (!chroma_enabled || srccolor != chromacolor) {
        Bit32u dstcolor = get_pixel(ch->s2d_img_dst,
          ch->ifc_draw_offset, ch->ifc_x, ch->s2d_color_bytes);
        if (ch->ifc_color_bytes == 4 &&
            ch->s2d_color_bytes == 2)
          dstcolor = nvriva_color_565_to_888(dstcolor);
        pixel_operation(ch, ch->ifc_operation, &dstcolor, &srccolor,
          ch->ifc_color_bytes, ch->ifc_ofs_x + ch->ifc_x, ch->ifc_ofs_y + ch->ifc_y);
        if (ch->ifc_color_bytes == 4 &&
            ch->s2d_color_bytes == 2)
          dstcolor = nvriva_color_888_to_565(dstcolor);
        put_pixel(ch, ch->ifc_draw_offset, ch->ifc_x, dstcolor);
      }
    }
    ch->ifc_x++;
    if (ch->ifc_x >= ch->ifc_src_width) {
      BX_NVRIVA_THIS redraw_area_nd(ch->ifc_redraw_offset, ch->ifc_dst_width, 1);
      ch->ifc_draw_offset += ch->s2d_pitch_dst;
      ch->ifc_redraw_offset += ch->s2d_pitch_dst;
      ch->ifc_x = 0;
      ch->ifc_y++;
    }
  }
}

void bx_nvriva_c::iifc(nv04_channel* ch)
{
  Bit16s dx = ch->iifc_yx & 0xFFFF;
  Bit16s dy = ch->iifc_yx >> 16;
  Bit16s clipx0 = ch->clip_x - dx;
  Bit16s clipy0 = ch->clip_y - dy;
  Bit16s clipx1 = clipx0 + ch->clip_width;
  Bit16s clipy1 = clipy0 + ch->clip_height;
  Bit32u swidth = ch->iifc_shw & 0xFFFF;
  Bit32u dwidth = ch->iifc_dhw & 0xFFFF;
  Bit32u height = ch->iifc_dhw >> 16;
  Bit32u pitch = ch->s2d_pitch_dst;
  Bit32u draw_offset = ch->s2d_ofs_dst +
    dy * pitch + dx * ch->s2d_color_bytes;
  Bit32u redraw_offset = dma_lin_lookup(ch->s2d_img_dst, draw_offset) -
    BX_NVRIVA_THIS disp_offset;
  Bit32u symbol_index = 0;
  for (Bit16u y = 0; y < height; y++) {
    for (Bit16u x = 0; x < dwidth; x++) {
      if (x >= clipx0 && x < clipx1 && y >= clipy0 && y < clipy1) {
        Bit8u symbol;
        if (ch->iifc_bpp4) {
          Bit32u word_offset = symbol_index / 8;
          Bit32u symbol_offset = (symbol_index % 8 ^ 1) * 4;
          symbol = ch->iifc_words[word_offset] >> symbol_offset & 0xF;
        } else {
          Bit32u word_offset = symbol_index / 4;
          Bit32u symbol_offset = symbol_index % 4 * 8;
          symbol = ch->iifc_words[word_offset] >> symbol_offset & 0xFF;
        }
        Bit32u dstcolor = get_pixel(ch->s2d_img_dst,
          draw_offset, x, ch->s2d_color_bytes);
        if (ch->iifc_color_bytes == 4) {
          Bit32u srccolor = dma_read32(ch->iifc_palette,
            ch->iifc_palette_ofs + symbol * 4);
          if (ch->s2d_color_bytes == 2)
            dstcolor = nvriva_color_565_to_888(dstcolor);
          pixel_operation(ch, ch->iifc_operation,
            &dstcolor, &srccolor, 4, dx + x, dy + y);
          if (ch->s2d_color_bytes == 2)
            dstcolor = nvriva_color_888_to_565(dstcolor);
        } else if (ch->iifc_color_bytes == 2) {
          Bit32u srccolor = dma_read16(ch->iifc_palette,
            ch->iifc_palette_ofs + symbol * 2);
          pixel_operation(ch, ch->iifc_operation,
            &dstcolor, &srccolor, 2, dx + x, dy + y);
        }
        put_pixel(ch, draw_offset, x, dstcolor);
      }
      symbol_index++;
    }
    symbol_index += swidth - dwidth;
    draw_offset += pitch;
  }
  BX_NVRIVA_THIS redraw_area_nd(redraw_offset, dwidth, height);
}

void bx_nvriva_c::sifc(nv04_channel* ch)
{
  Bit16u dx = ch->sifc_clip_yx & 0xFFFF;
  Bit16u dy = ch->sifc_clip_yx >> 16;
  Bit32u dsdx = (Bit32u)(BX_CONST64(1099511627776) / ch->sifc_dxds);
  Bit32u dtdy = (Bit32u)(BX_CONST64(1099511627776) / ch->sifc_dydt);
  Bit32u swidth = ch->sifc_shw & 0xFFFF;
  Bit32u dwidth = ch->sifc_clip_hw & 0xFFFF;
  Bit32u height = ch->sifc_clip_hw >> 16;
  Bit32u pitch = ch->s2d_pitch_dst;
  Bit32u draw_offset = ch->s2d_ofs_dst + dy * pitch + dx * ch->s2d_color_bytes;
  Bit32u redraw_offset = dma_lin_lookup(ch->s2d_img_dst, draw_offset) -
    BX_NVRIVA_THIS disp_offset;
  Bit32s sx0 = ((ch->sifc_syx & 0xFFFF) << 16) - (dx << 20) - 0x80000;
  Bit32s sy = (ch->sifc_syx & 0xFFFF0000) - (dy << 20) - 0x80000;
  if (sx0 < 0)
    sx0 = 0;
  if (sy < 0)
    sy = 0;
  Bit32u symbol_offset_y = 0;
  for (Bit16u y = 0; y < height; y++) {
    Bit32u sx = sx0;
    for (Bit16u x = 0; x < dwidth; x++) {
      Bit32u dstcolor = get_pixel(ch->s2d_img_dst, draw_offset, x, ch->s2d_color_bytes);
      Bit32u srccolor;
      Bit32u symbol_offset = symbol_offset_y + (sx >> 20);
      if (ch->sifc_color_bytes == 4) {
        srccolor = ch->sifc_words[symbol_offset];
      } else if (ch->sifc_color_bytes == 2) {
        Bit16u *sifc_words16 = (Bit16u*)ch->sifc_words;
        srccolor = sifc_words16[symbol_offset];
      } else {
        Bit8u *sifc_words8 = (Bit8u*)ch->sifc_words;
        srccolor = sifc_words8[symbol_offset];
      }
      if (ch->sifc_color_bytes == 4 &&
          ch->s2d_color_bytes == 2)
        dstcolor = nvriva_color_565_to_888(dstcolor);
      pixel_operation(ch, ch->sifc_operation,
        &dstcolor, &srccolor, ch->sifc_color_bytes, dx + x, dy + y);
      if (ch->sifc_color_bytes == 4 &&
          ch->s2d_color_bytes == 2)
        dstcolor = nvriva_color_888_to_565(dstcolor);
      put_pixel(ch, draw_offset, x, dstcolor);
      sx += dsdx;
    }
    sy += dtdy;
    symbol_offset_y = (sy >> 20) * swidth;
    draw_offset += pitch;
  }
  BX_NVRIVA_THIS redraw_area_nd(redraw_offset, dwidth, height);
}

void bx_nvriva_c::copyarea(nv04_channel* ch)
{
  Bit16u sx = ch->blit_syx & 0xFFFF;
  Bit16u sy = ch->blit_syx >> 16;
  Bit16u dx = ch->blit_dyx & 0xFFFF;
  Bit16u dy = ch->blit_dyx >> 16;
  Bit16u width = ch->blit_hw & 0xFFFF;
  Bit16u height = ch->blit_hw >> 16;
  Bit32u spitch = ch->s2d_pitch_src;
  Bit32u dpitch = ch->s2d_pitch_dst;
  Bit32u src_offset = ch->s2d_ofs_src;
  Bit32u draw_offset = ch->s2d_ofs_dst;
  bool xdir = dx > sx;
  bool ydir = dy > sy;
  src_offset += (sy + ydir * (height - 1)) * spitch + sx * ch->s2d_color_bytes;
  Bit32u redraw_offset = dma_lin_lookup(ch->s2d_img_dst, draw_offset) +
    dy * dpitch + dx * ch->s2d_color_bytes - BX_NVRIVA_THIS disp_offset;
  draw_offset += (dy + ydir * (height - 1)) * dpitch + dx * ch->s2d_color_bytes;
  Bit32u chromacolor;
  bool chroma_enabled = false;
  if (ch->blit_color_key_enable) {
    if (ch->s2d_color_bytes == 4) {
      chromacolor = ch->chroma_color & 0x00FFFFFF;
      chroma_enabled = ch->chroma_color & 0xFF000000;
    } else if (ch->s2d_color_bytes == 2) {
      chromacolor = ch->chroma_color & 0x0000FFFF;
      chroma_enabled = ch->chroma_color & 0xFFFF0000;
    } else {
      chromacolor = ch->chroma_color & 0x000000FF;
      chroma_enabled = ch->chroma_color & 0xFFFFFF00;
    }
  }
  for (Bit16u y = 0; y < height; y++) {
    for (Bit16u x = 0; x < width; x++) {
      Bit16u xa = xdir ? width - x - 1 : x;
      Bit32u srccolor = get_pixel(ch->s2d_img_src,
        src_offset, xa, ch->s2d_color_bytes);
      if (!chroma_enabled || srccolor != chromacolor) {
        Bit32u dstcolor = get_pixel(ch->s2d_img_dst,
          draw_offset, xa, ch->s2d_color_bytes);
        pixel_operation(ch, ch->blit_operation,
          &dstcolor, &srccolor, ch->s2d_color_bytes, dx + x, dy + y);
        put_pixel(ch, draw_offset, xa, dstcolor);
      }
    }
    src_offset += spitch * (1 - 2 * ydir);
    draw_offset += dpitch * (1 - 2 * ydir);
  }
  BX_NVRIVA_THIS redraw_area_nd(redraw_offset, width, height);
}

void bx_nvriva_c::m2mf(nv04_channel* ch)
{
  Bit32u src_offset = ch->m2mf_src_offset;
  Bit32u dst_offset = ch->m2mf_dst_offset;
  for (Bit16u y = 0; y < ch->m2mf_line_count; y++) {
    dma_copy(
      ch->m2mf_dst, dst_offset,
      ch->m2mf_src, src_offset,
      ch->m2mf_line_length);
    src_offset += ch->m2mf_src_pitch;
    dst_offset += ch->m2mf_dst_pitch;
  }
  Bit32u dma_target = ramin_read32(ch->m2mf_dst) >> 12 & 0xFF;
  if (dma_target == 0x03 || dma_target == 0x0b) {
    Bit32u redraw_offset = dma_lin_lookup(ch->m2mf_dst, ch->m2mf_dst_offset) -
      BX_NVRIVA_THIS disp_offset;
    Bit32u width = ch->m2mf_line_length / (BX_NVRIVA_THIS svga_bpp >> 3);
    BX_NVRIVA_THIS redraw_area_nd(redraw_offset, width, ch->m2mf_line_count);
  }
}

Bit32u nvriva_swizzle(Bit32u x, Bit32u y, Bit32u width, Bit32u height)
{
  bool xleft = true;
  bool yleft = true;
  Bit32u xbit = 1;
  Bit32u ybit = 1;
  Bit32u rbit = 1;
  Bit32u r = 0;
  do {
    if (xleft) {
      if ((x & xbit) != 0)
        r |= rbit;
      rbit <<= 1;
      xbit <<= 1;
      xleft = xbit < width;
    }
    if (yleft) {
      if ((y & ybit) != 0)
        r |= rbit;
      rbit <<= 1;
      ybit <<= 1;
      yleft = ybit < height;
    }
  } while (xleft || yleft);
  return r;
}

void bx_nvriva_c::tfc(nv04_channel* ch)
{
  Bit16u dx = ch->tfc_yx & 0xFFFF;
  Bit16u dy = ch->tfc_yx >> 16;
  Bit16s clipx0 = (ch->tfc_clip_wx & 0xFFFF) - dx;
  Bit16s clipy0 = (ch->tfc_clip_hy & 0xFFFF) - dy;
  Bit16s clipx1 = clipx0 + (ch->tfc_clip_wx >> 16);
  Bit16s clipy1 = clipy0 + (ch->tfc_clip_hy >> 16);
  Bit32u width = ch->tfc_hw & 0xFFFF;
  Bit32u height = ch->tfc_hw >> 16;
  Bit32u word_offset = 0;
  if (ch->tfc_swizzled) {
    for (Bit16u y = 0; y < height; y++) {
      for (Bit16u x = 0; x < width; x++) {
        if (x >= clipx0 && x < clipx1 && y >= clipy0 && y < clipy1) {
          Bit32u srccolor;
          if (ch->tfc_color_bytes == 4) {
            srccolor = ch->tfc_words[word_offset];
          } else {
            Bit16u *tfc_words16 = (Bit16u*)ch->tfc_words;
            srccolor = tfc_words16[word_offset];
          }
          put_pixel_swzs(ch, ch->swzs_ofs +
            nvriva_swizzle(x + dx, y + dy, ch->swzs_width, ch->swzs_height) *
            ch->swzs_color_bytes, srccolor);
        }
        word_offset++;
      }
    }
  } else {
    Bit32u pitch = ch->s2d_pitch_dst;
    Bit32u draw_offset = ch->s2d_ofs_dst + dy * pitch + dx * ch->s2d_color_bytes;
    for (Bit16u y = 0; y < height; y++) {
      for (Bit16u x = 0; x < width; x++) {
        if (x >= clipx0 && x < clipx1 && y >= clipy0 && y < clipy1) {
          Bit32u srccolor;
          if (ch->tfc_color_bytes == 4) {
            srccolor = ch->tfc_words[word_offset];
          } else {
            Bit16u *tfc_words16 = (Bit16u*)ch->tfc_words;
            srccolor = tfc_words16[word_offset];
          }
          put_pixel(ch, draw_offset, x, srccolor);
        }
        word_offset++;
      }
      draw_offset += pitch;
    }
  }
}

void bx_nvriva_c::sifm(nv04_channel* ch)
{
  Bit16u dx = ch->sifm_dyx & 0xFFFF;
  Bit16u dy = ch->sifm_dyx >> 16;
//Bit16u swidth = ch->sifm_shw & 0xFFFF;
//Bit16u sheight = ch->sifm_shw >> 16;
  Bit16u dwidth = ch->sifm_dhw & 0xFFFF;
  Bit16u dheight = ch->sifm_dhw >> 16;
  Bit32u spitch = ch->sifm_sfmt & 0xFFFF;
  // SIFM without scaling is used frequently in some operating systems
  if (ch->sifm_dudx == 0x00100000 && ch->sifm_dvdy == 0x00100000) {
    Bit16u sx = (ch->sifm_syx & 0xFFFF) >> 4;
    Bit16u sy = (ch->sifm_syx >> 16) >> 4;
    Bit32u src_offset = ch->sifm_sofs + sy * spitch + sx * ch->sifm_color_bytes;
    if (ch->sifm_swizzled) {
      for (Bit16u y = 0; y < dheight; y++) {
        for (Bit16u x = 0; x < dwidth; x++) {
          Bit32u srccolor = get_pixel(ch->sifm_src, src_offset, x, ch->sifm_color_bytes);
          put_pixel_swzs(ch, ch->swzs_ofs +
            nvriva_swizzle(x + dx, y + dy, ch->swzs_width, ch->swzs_height) *
            ch->swzs_color_bytes, srccolor);
        }
        src_offset += spitch;
      }
    } else {
      Bit32u dpitch = ch->s2d_pitch_dst;
      Bit32u draw_offset = ch->s2d_ofs_dst +
        dy * dpitch + dx * ch->s2d_color_bytes;
      Bit32u redraw_offset = dma_lin_lookup(ch->s2d_img_dst, draw_offset) -
        BX_NVRIVA_THIS disp_offset;
      for (Bit16u y = 0; y < dheight; y++) {
        for (Bit16u x = 0; x < dwidth; x++) {
          Bit32u dstcolor = get_pixel(ch->s2d_img_dst, draw_offset, x, ch->s2d_color_bytes);
          Bit32u srccolor = get_pixel(ch->sifm_src, src_offset, x, ch->sifm_color_bytes);
          if (ch->sifm_color_fmt == 4)
            srccolor |= 0xFF000000;
          pixel_operation(ch, ch->sifm_operation,
            &dstcolor, &srccolor, ch->s2d_color_bytes, dx + x, dy + y);
          put_pixel(ch, draw_offset, x, dstcolor);
        }
        src_offset += spitch;
        draw_offset += dpitch;
      }
      BX_NVRIVA_THIS redraw_area_nd(redraw_offset, dwidth, dheight);
    }
  } else {
    Bit32s sx0 = ((ch->sifm_syx & 0xFFFF) << 16) - (dx << 20) - 0x80000;
    Bit32s sy = (ch->sifm_syx & 0xFFFF0000) - (dy << 20) - 0x80000;
    if (sx0 < 0)
      sx0 = 0;
    if (sy < 0)
      sy = 0;
    if (ch->sifm_swizzled) {
      for (Bit16u y = 0; y < dheight; y++) {
        Bit32u sx = sx0;
        Bit32u src_offset = ch->sifm_sofs + (sy >> 20) * spitch;
        for (Bit16u x = 0; x < dwidth; x++) {
          Bit32u srccolor = get_pixel(ch->sifm_src, src_offset, sx >> 20, ch->sifm_color_bytes);
          put_pixel_swzs(ch, ch->swzs_ofs +
            nvriva_swizzle(x + dx, y + dy, ch->swzs_width, ch->swzs_height) *
            ch->swzs_color_bytes, srccolor);
          sx += ch->sifm_dudx;
        }
        sy += ch->sifm_dvdy;
      }
    } else {
      Bit32u dpitch = ch->s2d_pitch_dst;
      Bit32u draw_offset = ch->s2d_ofs_dst +
        dy * dpitch + dx * ch->s2d_color_bytes;
      Bit32u redraw_offset = dma_lin_lookup(ch->s2d_img_dst, draw_offset) -
        BX_NVRIVA_THIS disp_offset;
      for (Bit16u y = 0; y < dheight; y++) {
        Bit32u sx = sx0;
        Bit32u src_offset = ch->sifm_sofs + (sy >> 20) * spitch;
        for (Bit16u x = 0; x < dwidth; x++) {
          Bit32u dstcolor = get_pixel(ch->s2d_img_dst, draw_offset, x, ch->s2d_color_bytes);
          Bit32u srccolor = get_pixel(ch->sifm_src, src_offset, sx >> 20, ch->sifm_color_bytes);
          if (ch->sifm_color_fmt == 4)
            srccolor |= 0xFF000000;
          pixel_operation(ch, ch->sifm_operation,
            &dstcolor, &srccolor, ch->s2d_color_bytes, dx + x, dy + y);
          put_pixel(ch, draw_offset, x, dstcolor);
          sx += ch->sifm_dudx;
        }
        sy += ch->sifm_dvdy;
        draw_offset += dpitch;
      }
      BX_NVRIVA_THIS redraw_area_nd(redraw_offset, dwidth, dheight);
    }
  }
}

void bx_nvriva_c::execute_clip(nv04_channel* ch, Bit32u method, Bit32u param)
{
  if (method == 0x0c0) {
    ch->clip_x = (Bit16u)param;
    ch->clip_y = param >> 16;
  } else if (method == 0x0c1) {
    ch->clip_width = (Bit16u)param;
    ch->clip_height = param >> 16;
  }
}

void bx_nvriva_c::execute_m2mf(nv04_channel* ch, Bit32u subc, Bit32u method, Bit32u param)
{
  if (method == 0x061)
    ch->m2mf_src = param;
  else if (method == 0x062)
    ch->m2mf_dst = param;
  else if (method == 0x0c3)
    ch->m2mf_src_offset = param;
  else if (method == 0x0c4)
    ch->m2mf_dst_offset = param;
  else if (method == 0x0c5)
    ch->m2mf_src_pitch = param;
  else if (method == 0x0c6)
    ch->m2mf_dst_pitch = param;
  else if (method == 0x0c7)
    ch->m2mf_line_length = param;
  else if (method == 0x0c8)
    ch->m2mf_line_count = param;
  else if (method == 0x0c9)
    ch->m2mf_format = param;
  else if (method == 0x0ca) {
    ch->m2mf_buffer_notify = param;
    m2mf(ch);
    if ((ramin_read32(ch->schs[subc].notifier) & 0xFF) == 0x30) {
      BX_DEBUG(("M2MF notify skipped"));
    } else {
      BX_DEBUG(("M2MF notify 0x%08x",
        ch->schs[subc].notifier));
      dma_write64(ch->schs[subc].notifier, 0x10 + 0x0, get_current_time());
      dma_write32(ch->schs[subc].notifier, 0x10 + 0x8, 0);
      dma_write32(ch->schs[subc].notifier, 0x10 + 0xC, 0);
    }
  }
}

void bx_nvriva_c::execute_rop(nv04_channel* ch, Bit32u method, Bit32u param)
{
  if (method == 0x0c0)
    ch->rop = param;
}

void bx_nvriva_c::execute_patt(nv04_channel* ch, Bit32u method, Bit32u param)
{
  if (method == 0x0c2)
    ch->patt_shape = param;
  else if (method == 0x0c3)
    ch->patt_type = param;
  else if (method == 0x0c4)
    ch->patt_bg_color = param;
  else if (method == 0x0c5)
    ch->patt_fg_color = param;
  else if (method == 0x0c6 || method == 0x0c7) {
    for (Bit32u i = 0; i < 32; i++)
      ch->patt_data_mono[i + (method & 1) * 32] = 1 << (i ^ 7) & param;
  } else if (method >= 0x100 && method < 0x110) {
    Bit32u i = (method - 0x100) * 4;
    ch->patt_data_color[i] = param & 0xFF;
    ch->patt_data_color[i + 1] = (param >> 8) & 0xFF;
    ch->patt_data_color[i + 2] = (param >> 16) & 0xFF;
    ch->patt_data_color[i + 3] = param >> 24;
  } else if (method >= 0x140 && method < 0x160) {
    Bit32u i = (method - 0x140) * 2;
    ch->patt_data_color[i] = param & 0xFFFF;
    ch->patt_data_color[i + 1] = param >> 16;
  } else if (method >= 0x1c0 && method < 0x200)
    ch->patt_data_color[method - 0x1c0] = param;
}

void bx_nvriva_c::execute_gdi(nv04_channel* ch, Bit32u method, Bit32u param)
{
  if (method == 0x0bf)
    ch->gdi_operation = param;
  else if (method == 0x0c0)
    ch->gdi_color_fmt = param;
  else if (method == 0x0c1)
    ch->gdi_mono_fmt = param;
  else if (method == 0x0ff)
    ch->gdi_rect_color = param;
  else if (method >= 0x100 && method < 0x140) {
    if (method & 1) {
      ch->gdi_rect_wh = param;
      gdi_fillrect(ch, false);
    } else
      ch->gdi_rect_xy = param;
  } else if (method == 0x17d)
    ch->gdi_clip_yx0 = param;
  else if (method == 0x17e)
    ch->gdi_clip_yx1 = param;
  else if (method == 0x17f)
    ch->gdi_rect_color = param;
  else if (method >= 0x180 && method < 0x1c0) {
    if (method & 1) {
      ch->gdi_rect_yx1 = param;
      gdi_fillrect(ch, true);
    } else
      ch->gdi_rect_yx0 = param;
  } else if (method == 0x1fb)
    ch->gdi_clip_yx0 = param;
  else if (method == 0x1fc)
    ch->gdi_clip_yx1 = param;
  else if (method == 0x1fd)
    ch->gdi_fg_color = param;
  else if (method == 0x1fe)
    ch->gdi_image_swh = param;
  else if (method == 0x1ff) {
    ch->gdi_image_xy = param;
    Bit32u width = ch->gdi_image_swh & 0xFFFF;
    Bit32u height = ch->gdi_image_swh >> 16;
    Bit32u wordCount = ALIGN(width * height, 32) >> 5;
    if (ch->gdi_words != nullptr)
      delete[] ch->gdi_words;
    ch->gdi_words_ptr = 0;
    ch->gdi_words_left = wordCount;
    ch->gdi_words = new Bit32u[wordCount];
  } else if (method >= 0x200 && method < 0x280) {
    ch->gdi_words[ch->gdi_words_ptr++] = param;
    ch->gdi_words_left--;
    if (!ch->gdi_words_left) {
      gdi_blit(ch, 0);
      delete[] ch->gdi_words;
      ch->gdi_words = nullptr;
    }
  } else if (method == 0x2f9)
    ch->gdi_clip_yx0 = param;
  else if (method == 0x2fa)
    ch->gdi_clip_yx1 = param;
  else if (method == 0x2fb)
    ch->gdi_bg_color = param;
  else if (method == 0x2fc)
    ch->gdi_fg_color = param;
  else if (method == 0x2fd)
    ch->gdi_image_swh = param;
  else if (method == 0x2fe)
    ch->gdi_image_dwh = param;
  else if (method == 0x2ff) {
    ch->gdi_image_xy = param;
    Bit32u width = ch->gdi_image_swh & 0xFFFF;
    Bit32u height = ch->gdi_image_swh >> 16;
    Bit32u wordCount = ALIGN(width * height, 32) >> 5;
    if (ch->gdi_words != nullptr)
      delete[] ch->gdi_words;
    ch->gdi_words_ptr = 0;
    ch->gdi_words_left = wordCount;
    ch->gdi_words = new Bit32u[wordCount];
  } else if (method >= 0x300 && method < 0x380) {
    ch->gdi_words[ch->gdi_words_ptr++] = param;
    ch->gdi_words_left--;
    if (!ch->gdi_words_left) {
      gdi_blit(ch, 1);
      delete[] ch->gdi_words;
      ch->gdi_words = nullptr;
    }
  } else if (method == 0x3fd)
    ch->gdi_clip_yx0 = param;
  else if (method == 0x3fe)
    ch->gdi_clip_yx1 = param;
  else if (method == 0x3ff)
    ch->gdi_fg_color = param;
}

void bx_nvriva_c::execute_swzsurf(nv04_channel* ch, Bit32u method, Bit32u param)
{
  if (method == 0x061)
    ch->swzs_img_obj = param;
  else if (method == 0x0c0) {
    ch->swzs_fmt = param;
    ch->swzs_width = 1 << ((param >> 16) & 0xff);
    ch->swzs_height = 1 << (param >> 24);
    Bit32u color_fmt = param & 0xffff;
    if (color_fmt == 1)          // Y8
      ch->swzs_color_bytes = 1;
    else if (color_fmt == 2 ||   // X1R5G5B5_Z1R5G5B5
             color_fmt == 4)     // R5G6B5
      ch->swzs_color_bytes = 2;
    else if (color_fmt == 0x6 || // X8R8G8B8_Z8R8G8B8
             color_fmt == 0xA || // A8R8G8B8
             color_fmt == 0xB)   // Y32
      ch->swzs_color_bytes = 4;
    else {
      BX_ERROR(("unknown swizzled surface color format: 0x%02x", color_fmt));
    }
  } else if (method == 0x0c1)
    ch->swzs_ofs = param;
}

void bx_nvriva_c::execute_chroma(nv04_channel* ch, Bit32u method, Bit32u param)
{
  if (method == 0x0c0)
    ch->chroma_color_fmt = param;
  else if (method == 0x0c1)
    ch->chroma_color = param;
}

void bx_nvriva_c::execute_imageblit(nv04_channel* ch, Bit32u method, Bit32u param)
{
  if (method == 0x061)
    ch->blit_color_key_enable = (ramin_read32(param) & 0xFF) != 0x30;
  else if (method == 0x0bf)
    ch->blit_operation = param;
  else if (method == 0x0c0)
    ch->blit_syx = param;
  else if (method == 0x0c1)
    ch->blit_dyx = param;
  else if (method == 0x0c2) {
    ch->blit_hw = param;
    copyarea(ch);
  }
}

void bx_nvriva_c::execute_ifc(nv04_channel* ch, Bit32u method, Bit32u param)
{
  if (method == 0x061)
    ch->ifc_color_key_enable = (ramin_read32(param) & 0xFF) != 0x30;
  else if (method == 0x062)
    ch->ifc_clip_enable = (ramin_read32(param) & 0xFF) != 0x30;
  else if (method == 0x0bf)
    ch->ifc_operation = param;
  else if (method == 0x0c0) {
    ch->ifc_color_fmt = param;
    update_color_bytes_ifc(ch);
    ch->ifc_pixels_per_word = 4 / ch->ifc_color_bytes;
  } else if (method == 0x0c1) {
    ch->ifc_x = 0;
    ch->ifc_y = 0;
    ch->ifc_ofs_x = param & 0xFFFF;
    ch->ifc_ofs_y = param >> 16;
    ch->ifc_draw_offset = ch->s2d_ofs_dst +
      ch->ifc_ofs_y * ch->s2d_pitch_dst + ch->ifc_ofs_x * ch->s2d_color_bytes;
    ch->ifc_redraw_offset = dma_lin_lookup(ch->s2d_img_dst,
      ch->ifc_draw_offset) - BX_NVRIVA_THIS disp_offset;
  } else if (method == 0x0c2) {
    ch->ifc_dst_width = param & 0xFFFF;
    ch->ifc_dst_height = param >> 16;
    ch->ifc_clip_x0 = 0;
    ch->ifc_clip_y0 = 0;
    ch->ifc_clip_x1 = ch->ifc_dst_width;
    ch->ifc_clip_y1 = ch->ifc_dst_height;
    if (ch->ifc_clip_enable) {
      Bit32s clipx0 = ch->clip_x - ch->ifc_ofs_x;
      Bit32s clipy0 = ch->clip_y - ch->ifc_ofs_y;
      Bit32s clipx1 = clipx0 + ch->clip_width;
      Bit32s clipy1 = clipy0 + ch->clip_height;
      ch->ifc_clip_x0 = BX_MAX((Bit32s)ch->ifc_clip_x0, clipx0);
      ch->ifc_clip_y0 = BX_MAX((Bit32s)ch->ifc_clip_y0, clipy0);
      ch->ifc_clip_x1 = BX_MIN((Bit32s)ch->ifc_clip_x1, clipx1);
      ch->ifc_clip_y1 = BX_MIN((Bit32s)ch->ifc_clip_y1, clipy1);
    }
  } else if (method == 0x0c3) {
    ch->ifc_src_width = param & 0xFFFF;
    ch->ifc_src_height = param >> 16;
  } else if (method >= 0x100 && method < 0x800) {
    ifc(ch, param);
  }
}

void bx_nvriva_c::execute_surf2d(nv04_channel* ch, Bit32u method, Bit32u param)
{
  ch->s2d_locked = true;
  if (method == 0x061)
    ch->s2d_img_src = param;
  else if (method == 0x062)
    ch->s2d_img_dst = param;
  else if (method == 0x0c0) {
    ch->s2d_color_fmt = param;
    Bit32u s2d_color_bytes_prev = ch->s2d_color_bytes;
    if (ch->s2d_color_fmt == 1) // Y8
      ch->s2d_color_bytes = 1;
    else if (ch->s2d_color_fmt == 4) // R5G6B5
      ch->s2d_color_bytes = 2;
    else if (ch->s2d_color_fmt == 0x6 || // X8R8G8B8_Z8R8G8B8
             ch->s2d_color_fmt == 0xA || // A8R8G8B8
             ch->s2d_color_fmt == 0xB)   // Y32
      ch->s2d_color_bytes = 4;
    else {
      BX_ERROR(("unknown 2d surface color format: 0x%02x",
        ch->s2d_color_fmt));
    }
    if (ch->s2d_color_bytes != s2d_color_bytes_prev &&
        (ch->s2d_color_bytes == 1 || s2d_color_bytes_prev == 1)) {
      update_color_bytes_ifc(ch);
      update_color_bytes_sifc(ch);
    }
  } else if (method == 0x0c1) {
    ch->s2d_pitch_src = param & 0xFFFF;
    ch->s2d_pitch_dst = param >> 16;
  } else if (method == 0x0c2)
    ch->s2d_ofs_src = param;
  else if (method == 0x0c3)
    ch->s2d_ofs_dst = param;
}

void bx_nvriva_c::update_color_bytes_ifc(nv04_channel* ch)
{
  BX_NVRIVA_THIS update_color_bytes(
    ch->s2d_color_fmt, ch->ifc_color_fmt,
    &ch->ifc_color_bytes);
}

void bx_nvriva_c::update_color_bytes_sifc(nv04_channel* ch)
{
  BX_NVRIVA_THIS update_color_bytes(
    ch->s2d_color_fmt, ch->sifc_color_fmt,
    &ch->sifc_color_bytes);
}

void bx_nvriva_c::update_color_bytes_tfc(nv04_channel* ch)
{
  BX_NVRIVA_THIS update_color_bytes(0,
    ch->tfc_color_fmt, &ch->tfc_color_bytes);
}

void bx_nvriva_c::update_color_bytes_iifc(nv04_channel* ch)
{
  BX_NVRIVA_THIS update_color_bytes(0,
    ch->iifc_color_fmt, &ch->iifc_color_bytes);
}

void bx_nvriva_c::update_color_bytes(Bit32u s2d_color_fmt, Bit32u color_fmt, Bit32u* color_bytes)
{
  if (s2d_color_fmt == 1) // Y8
    *color_bytes = 1; // hack
  else if (color_fmt == 1 || // R5G6B5
           color_fmt == 2 || // A1R5G5B5
           color_fmt == 3)   // X1R5G5B5
    *color_bytes = 2;
  else if (color_fmt == 4 || // A8R8G8B8
           color_fmt == 5)   // X8R8G8B8
    *color_bytes = 4;
  else
    BX_ERROR(("unknown color format: 0x%02x", color_fmt));
}

bool bx_nvriva_c::d3d_scissor_clip(nv04_channel* ch, Bit32u* x, Bit32u* y, Bit32u* width, Bit32u* height)
{
  Bit32s scissor_x1 = ch->d3d_scissor_x;
  Bit32s scissor_y1 = ch->d3d_scissor_y;
  Bit32s scissor_x2 = scissor_x1 + ch->d3d_scissor_width;
  Bit32s scissor_y2 = scissor_y1 + ch->d3d_scissor_height;
  Bit32s surf_x2 = *x + *width;
  Bit32s surf_y2 = *y + *height;
  if (scissor_x1 >= surf_x2 || scissor_x2 <= (Bit32s)*x ||
      scissor_y1 >= surf_y2 || scissor_y2 <= (Bit32s)*y)
    return false;
  *x = BX_MAX((Bit32s)*x, scissor_x1);
  *y = BX_MAX((Bit32s)*y, scissor_y1);
  *width = BX_MIN(surf_x2, scissor_x2) - *x;
  *height = BX_MIN(surf_y2, scissor_y2) - *y;
  return true;
}

void bx_nvriva_c::d3d_clear_surface(nv04_channel* ch)
{
  // NV04_3D surface clear with semaphore synchronization
  Bit32u dx = ch->d3d_clip_horizontal & 0xFFFF;
  Bit32u dy = ch->d3d_clip_vertical & 0xFFFF;
  Bit32u width = ch->d3d_clip_horizontal >> 16;
  Bit32u height = ch->d3d_clip_vertical >> 16;
  if (!d3d_scissor_clip(ch, &dx, &dy, &width, &height))
    return;
  
  // Wait for any pending semaphore operations
  if (!d3d_semaphore_wait(ch, 0)) {
    BX_DEBUG(("D3D clear surface: waiting for semaphore"));
    return; // Wait for semaphore to be ready
  }
  
  if (ch->d3d_clear_surface & 0x000000F0) {
    Bit32u pitch = ch->d3d_surface_pitch_a & 0xFFFF;
    Bit32u draw_offset = ch->d3d_surface_color_offset +
      dy * pitch + dx * ch->d3d_color_bytes;
    Bit32u redraw_offset = dma_lin_lookup(ch->d3d_color_obj, draw_offset) -
      BX_NVRIVA_THIS disp_offset;
    for (Bit32u y = 0; y < height; y++) {
      for (Bit32u x = 0; x < width; x++) {
        if (ch->d3d_color_bytes == 2)
          dma_write16(ch->d3d_color_obj, draw_offset + x * 2, ch->d3d_color_clear_value);
        else
          dma_write32(ch->d3d_color_obj, draw_offset + x * 4, ch->d3d_color_clear_value);
      }
      draw_offset += pitch;
    }
    BX_NVRIVA_THIS redraw_area_nd(redraw_offset, width, height);
  }
  if (ch->d3d_clear_surface & 0x00000001) {
    Bit32u pitch = d3d_get_surface_pitch_z(ch);
    Bit32u draw_offset = ch->d3d_surface_zeta_offset +
      dy * pitch + dx * ch->d3d_depth_bytes;
    for (Bit32u y = 0; y < height; y++) {
      for (Bit32u x = 0; x < width; x++) {
        if (ch->d3d_depth_bytes == 2)
          dma_write16(ch->d3d_zeta_obj, draw_offset + x * 2, ch->d3d_zstencil_clear_value);
        else
          dma_write32(ch->d3d_zeta_obj, draw_offset + x * 4, ch->d3d_zstencil_clear_value);
      }
      draw_offset += pitch;
    }
  }
  
  // Notify semaphore that clear operation is complete
  d3d_semaphore_notify(ch, 1);
}

Bit32u bx_nvriva_c::d3d_get_surface_pitch_z(nv04_channel* ch)
{
  return ch->d3d_surface_pitch_z & 0xFFFF;
}

// NV04_3D specific semaphore synchronization functions
void bx_nvriva_c::d3d_semaphore_notify(nv04_channel* ch, Bit32u value)
{
  // NV04_3D DMA_NOTIFY semaphore operation
  if (ch->d3d_semaphore_obj) {
    // Write semaphore value to DMA_NOTIFY object
    dma_write32(ch->d3d_semaphore_obj, 0x0, value);
    BX_DEBUG(("D3D semaphore notify: obj=0x%08x, value=0x%08x", 
      ch->d3d_semaphore_obj, value));
  }
}

bool bx_nvriva_c::d3d_semaphore_wait(nv04_channel* ch, Bit32u expected_value)
{
  // NV04_3D DMA_NOTIFY semaphore wait operation
  if (ch->d3d_semaphore_obj) {
    Bit32u current_value = dma_read32(ch->d3d_semaphore_obj, 0x0);
    if (current_value != expected_value) {
      BX_DEBUG(("D3D semaphore wait: obj=0x%08x, expected=0x%08x, current=0x%08x", 
        ch->d3d_semaphore_obj, expected_value, current_value));
      return false; // Semaphore not ready
    }
    return true; // Semaphore ready
  }
  return true; // No semaphore object, assume ready
}

void bx_nvriva_c::d3d_apply_fog_color(nv04_channel* ch, Bit32u* color)
{
  // Apply fog color to a pixel color if fog is enabled
  if (ch->d3d_fog_enable) {
    // Simple fog application - blend with fog color based on alpha
    float fog_factor = ch->d3d_fog_color_a / 255.0f;
    *color = d3d_blend_fog_color(ch, *color, fog_factor);
  }
}

Bit32u bx_nvriva_c::d3d_blend_fog_color(nv04_channel* ch, Bit32u original_color, float fog_factor)
{
  // Blend original color with fog color
  // Extract original color components
  Bit8u orig_r = (original_color >> 16) & 0xFF;
  Bit8u orig_g = (original_color >> 8) & 0xFF;
  Bit8u orig_b = original_color & 0xFF;
  Bit8u orig_a = (original_color >> 24) & 0xFF;
  
  // Blend with fog color
  Bit8u new_r = (Bit8u)(orig_r * (1.0f - fog_factor) + ch->d3d_fog_color_r * fog_factor);
  Bit8u new_g = (Bit8u)(orig_g * (1.0f - fog_factor) + ch->d3d_fog_color_g * fog_factor);
  Bit8u new_b = (Bit8u)(orig_b * (1.0f - fog_factor) + ch->d3d_fog_color_b * fog_factor);
  Bit8u new_a = (Bit8u)(orig_a * (1.0f - fog_factor) + ch->d3d_fog_color_a * fog_factor);
  
  // Reconstruct color value
  return (new_a << 24) | (new_r << 16) | (new_g << 8) | new_b;
}

bool bx_nvriva_c::d3d_alpha_test(nv04_channel* ch, Bit8u alpha)
{
  // Perform alpha test based on control register settings
  if (!ch->d3d_alpha_enable) {
    return true; // Alpha test disabled, always pass
  }
  
  Bit8u alpha_ref = ch->d3d_alpha_ref;
  Bit8u alpha_func = ch->d3d_alpha_func;
  
  switch (alpha_func) {
    case 0x1: // NEVER
      return false;
    case 0x2: // LESS
      return alpha < alpha_ref;
    case 0x3: // EQUAL
      return alpha == alpha_ref;
    case 0x4: // LEQUAL
      return alpha <= alpha_ref;
    case 0x5: // GREATER
      return alpha > alpha_ref;
    case 0x6: // NOTEQUAL
      return alpha != alpha_ref;
    case 0x7: // GEQUAL
      return alpha >= alpha_ref;
    case 0x8: // ALWAYS
    default:
      return true;
  }
}

bool bx_nvriva_c::d3d_z_test(nv04_channel* ch, float z_value, float z_buffer_value)
{
  // Perform Z-buffer test based on control register settings
  if (!ch->d3d_z_enable) {
    return true; // Z-test disabled, always pass
  }
  
  Bit8u z_func = ch->d3d_z_func;
  
  switch (z_func) {
    case 0x1: // NEVER
      return false;
    case 0x2: // LESS
      return z_value < z_buffer_value;
    case 0x3: // EQUAL
      return z_value == z_buffer_value;
    case 0x4: // LEQUAL
      return z_value <= z_buffer_value;
    case 0x5: // GREATER
      return z_value > z_buffer_value;
    case 0x6: // NOTEQUAL
      return z_value != z_buffer_value;
    case 0x7: // GEQUAL
      return z_value >= z_buffer_value;
    case 0x8: // ALWAYS
    default:
      return true;
  }
}

bool bx_nvriva_c::d3d_cull_triangle(nv04_channel* ch, float v0[4], float v1[4], float v2[4])
{
  // Perform triangle culling based on control register settings
  Bit8u cull_mode = ch->d3d_cull_mode;
  
  switch (cull_mode) {
    case 0x0: // BOTH - cull both front and back faces
      return true; // Always cull
    case 0x1: // NONE - don't cull any faces
      return false; // Never cull
    case 0x2: // CW - cull clockwise faces
    case 0x3: // CCW - cull counter-clockwise faces
    {
      // Calculate triangle normal using cross product
      float v01[3] = {v1[0] - v0[0], v1[1] - v0[1], v1[2] - v0[2]};
      float v02[3] = {v2[0] - v0[0], v2[1] - v0[1], v2[2] - v0[2]};
      
      // Cross product: v01 × v02
      float normal[3] = {
        v01[1] * v02[2] - v01[2] * v02[1],
        v01[2] * v02[0] - v01[0] * v02[2],
        v01[0] * v02[1] - v01[1] * v02[0]
      };
      
      // Determine winding order based on Z component of normal
      bool is_clockwise = normal[2] < 0.0f;
      
      if (cull_mode == 0x2) { // CW
        return is_clockwise;
      } else { // CCW
        return !is_clockwise;
      }
    }
    default:
      return false; // Default to no culling
  }
}

Bit32u bx_nvriva_c::d3d_texture_blend(nv04_channel* ch, Bit32u tex_color, Bit32u vertex_color)
{
  // Apply colorkey transparency first
  tex_color = d3d_apply_colorkey(ch, tex_color);
  
  // Apply texture blending based on texture map mode
  Bit8u texture_map = ch->d3d_texture_map;
  
  // Extract color components
  Bit8u tex_r = (tex_color >> 16) & 0xFF;
  Bit8u tex_g = (tex_color >> 8) & 0xFF;
  Bit8u tex_b = tex_color & 0xFF;
  Bit8u tex_a = (tex_color >> 24) & 0xFF;
  
  Bit8u vert_r = (vertex_color >> 16) & 0xFF;
  Bit8u vert_g = (vertex_color >> 8) & 0xFF;
  Bit8u vert_b = vertex_color & 0xFF;
  Bit8u vert_a = (vertex_color >> 24) & 0xFF;
  
  Bit8u result_r, result_g, result_b, result_a;
  
  switch (texture_map) {
    case 0x1: // DECAL
      result_r = tex_r;
      result_g = tex_g;
      result_b = tex_b;
      result_a = vert_a;
      break;
    case 0x2: // MODULATE
      result_r = (tex_r * vert_r) / 255;
      result_g = (tex_g * vert_g) / 255;
      result_b = (tex_b * vert_b) / 255;
      result_a = vert_a;
      break;
    case 0x3: // DECALALPHA
      result_r = (tex_r * tex_a + vert_r * (255 - tex_a)) / 255;
      result_g = (tex_g * tex_a + vert_g * (255 - tex_a)) / 255;
      result_b = (tex_b * tex_a + vert_b * (255 - tex_a)) / 255;
      result_a = vert_a;
      break;
    case 0x4: // MODULATEALPHA
      result_r = (tex_r * vert_r) / 255;
      result_g = (tex_g * vert_g) / 255;
      result_b = (tex_b * vert_b) / 255;
      result_a = (tex_a * vert_a) / 255;
      break;
    case 0x5: // DECALMASK
      result_r = tex_r;
      result_g = tex_g;
      result_b = tex_b;
      result_a = vert_a;
      break;
    case 0x6: // MODULATEMASK
      result_r = (tex_r * vert_r) / 255;
      result_g = (tex_g * vert_g) / 255;
      result_b = (tex_b * vert_b) / 255;
      result_a = vert_a;
      break;
    case 0x7: // COPY
      result_r = tex_r;
      result_g = tex_g;
      result_b = tex_b;
      result_a = tex_a;
      break;
    case 0x8: // ADD
      result_r = BX_MIN(255, tex_r + vert_r);
      result_g = BX_MIN(255, tex_g + vert_g);
      result_b = BX_MIN(255, tex_b + vert_b);
      result_a = vert_a;
      break;
    default: // Default to modulate
      result_r = (tex_r * vert_r) / 255;
      result_g = (tex_g * vert_g) / 255;
      result_b = (tex_b * vert_b) / 255;
      result_a = vert_a;
      break;
  }
  
  return (result_a << 24) | (result_r << 16) | (result_g << 8) | result_b;
}

Bit32u bx_nvriva_c::d3d_alpha_blend(nv04_channel* ch, Bit32u src_color, Bit32u dst_color)
{
  // Perform alpha blending based on blend factors
  if (!ch->d3d_blend_enable) {
    return src_color; // No blending, return source color
  }
  
  Bit32u src_factor = d3d_get_blend_factor(ch, ch->d3d_blend_src, src_color, dst_color);
  Bit32u dst_factor = d3d_get_blend_factor(ch, ch->d3d_blend_dst, src_color, dst_color);
  
  // Extract color components
  Bit8u src_r = (src_color >> 16) & 0xFF;
  Bit8u src_g = (src_color >> 8) & 0xFF;
  Bit8u src_b = src_color & 0xFF;
  Bit8u src_a = (src_color >> 24) & 0xFF;
  
  Bit8u dst_r = (dst_color >> 16) & 0xFF;
  Bit8u dst_g = (dst_color >> 8) & 0xFF;
  Bit8u dst_b = dst_color & 0xFF;
  Bit8u dst_a = (dst_color >> 24) & 0xFF;
  
  // Blend colors
  Bit8u result_r = (src_r * src_factor + dst_r * dst_factor) / 255;
  Bit8u result_g = (src_g * src_factor + dst_g * dst_factor) / 255;
  Bit8u result_b = (src_b * src_factor + dst_b * dst_factor) / 255;
  Bit8u result_a = (src_a * src_factor + dst_a * dst_factor) / 255;
  
  return (result_a << 24) | (result_r << 16) | (result_g << 8) | result_b;
}

Bit32u bx_nvriva_c::d3d_get_blend_factor(nv04_channel* ch, Bit8u factor, Bit32u src_color, Bit32u dst_color)
{
  // Get blend factor value based on factor type
  Bit8u src_r = (src_color >> 16) & 0xFF;
  Bit8u src_g = (src_color >> 8) & 0xFF;
  Bit8u src_b = src_color & 0xFF;
  Bit8u src_a = (src_color >> 24) & 0xFF;
  
  Bit8u dst_r = (dst_color >> 16) & 0xFF;
  Bit8u dst_g = (dst_color >> 8) & 0xFF;
  Bit8u dst_b = dst_color & 0xFF;
  Bit8u dst_a = (dst_color >> 24) & 0xFF;
  
  switch (factor) {
    case 0x1: // ZERO
      return 0;
    case 0x2: // ONE
      return 255;
    case 0x3: // SRC_COLOR
      return (src_r + src_g + src_b) / 3;
    case 0x4: // ONE_MINUS_SRC_COLOR
      return 255 - (src_r + src_g + src_b) / 3;
    case 0x5: // SRC_ALPHA
      return src_a;
    case 0x6: // ONE_MINUS_SRC_ALPHA
      return 255 - src_a;
    case 0x7: // DST_ALPHA
      return dst_a;
    case 0x8: // ONE_MINUS_DST_ALPHA
      return 255 - dst_a;
    case 0x9: // DST_COLOR
      return (dst_r + dst_g + dst_b) / 3;
    case 0xa: // ONE_MINUS_DST_COLOR
      return 255 - (dst_r + dst_g + dst_b) / 3;
    case 0xb: // SRC_ALPHA_SATURATE
      return BX_MIN(src_a, 255 - dst_a);
    default:
      return 255; // Default to one
  }
}

Bit32u bx_nvriva_c::d3d_sample_texture_filtered(nv04_channel* ch, Bit32u tex_obj, float u, float v, float lod)
{
  // Main texture sampling function that selects appropriate filtering method
  if (lod > 0.0f) {
    // Minification - use minify filter
    switch (ch->d3d_minify_filter) {
      case 0x1: // NEAREST
        return d3d_sample_texture_nearest(ch, tex_obj, u, v, lod);
      case 0x2: // LINEAR
        return d3d_sample_texture_linear(ch, tex_obj, u, v, lod);
      case 0x3: // NEAREST_MIPMAP_NEAREST
      case 0x4: // LINEAR_MIPMAP_NEAREST
      case 0x5: // NEAREST_MIPMAP_LINEAR
      case 0x6: // LINEAR_MIPMAP_LINEAR
        return d3d_sample_texture_mipmap(ch, tex_obj, u, v, lod);
      default:
        return d3d_sample_texture_nearest(ch, tex_obj, u, v, lod);
    }
  } else {
    // Magnification - use magnify filter
    switch (ch->d3d_magnify_filter) {
      case 0x1: // NEAREST
        return d3d_sample_texture_nearest(ch, tex_obj, u, v, lod);
      case 0x2: // LINEAR
        return d3d_sample_texture_linear(ch, tex_obj, u, v, lod);
      default:
        return d3d_sample_texture_nearest(ch, tex_obj, u, v, lod);
    }
  }
}

Bit32u bx_nvriva_c::d3d_sample_texture_nearest(nv04_channel* ch, Bit32u tex_obj, float u, float v, float lod)
{
  // Nearest neighbor texture sampling
  // Convert normalized coordinates to texture coordinates
  int tex_width = 256; // Default texture width
  int tex_height = 256; // Default texture height
  
  int x = (int)(u * tex_width) % tex_width;
  int y = (int)(v * tex_height) % tex_height;
  
  // Sample texture at integer coordinates with offset
  Bit32u tex_offset = y * tex_width + x;
  return dma_read32(tex_obj + ch->d3d_texture_offset[0], tex_offset * 4);
}

Bit32u bx_nvriva_c::d3d_sample_texture_linear(nv04_channel* ch, Bit32u tex_obj, float u, float v, float lod)
{
  // Linear interpolation texture sampling
  int tex_width = 256; // Default texture width
  int tex_height = 256; // Default texture height
  
  float x = u * tex_width;
  float y = v * tex_height;
  
  int x0 = (int)x % tex_width;
  int y0 = (int)y % tex_height;
  int x1 = (x0 + 1) % tex_width;
  int y1 = (y0 + 1) % tex_height;
  
  float fx = x - (int)x;
  float fy = y - (int)y;
  
  // Sample four neighboring texels with offset
  Bit32u c00 = dma_read32(tex_obj + ch->d3d_texture_offset[0], (y0 * tex_width + x0) * 4);
  Bit32u c01 = dma_read32(tex_obj + ch->d3d_texture_offset[0], (y0 * tex_width + x1) * 4);
  Bit32u c10 = dma_read32(tex_obj + ch->d3d_texture_offset[0], (y1 * tex_width + x0) * 4);
  Bit32u c11 = dma_read32(tex_obj + ch->d3d_texture_offset[0], (y1 * tex_width + x1) * 4);
  
  // Extract color components
  Bit8u c00_r = (c00 >> 16) & 0xFF;
  Bit8u c00_g = (c00 >> 8) & 0xFF;
  Bit8u c00_b = c00 & 0xFF;
  Bit8u c00_a = (c00 >> 24) & 0xFF;
  
  Bit8u c01_r = (c01 >> 16) & 0xFF;
  Bit8u c01_g = (c01 >> 8) & 0xFF;
  Bit8u c01_b = c01 & 0xFF;
  Bit8u c01_a = (c01 >> 24) & 0xFF;
  
  Bit8u c10_r = (c10 >> 16) & 0xFF;
  Bit8u c10_g = (c10 >> 8) & 0xFF;
  Bit8u c10_b = c10 & 0xFF;
  Bit8u c10_a = (c10 >> 24) & 0xFF;
  
  Bit8u c11_r = (c11 >> 16) & 0xFF;
  Bit8u c11_g = (c11 >> 8) & 0xFF;
  Bit8u c11_b = c11 & 0xFF;
  Bit8u c11_a = (c11 >> 24) & 0xFF;
  
  // Linear interpolation
  Bit8u r = (Bit8u)(c00_r * (1-fx) * (1-fy) + c01_r * fx * (1-fy) + c10_r * (1-fx) * fy + c11_r * fx * fy);
  Bit8u g = (Bit8u)(c00_g * (1-fx) * (1-fy) + c01_g * fx * (1-fy) + c10_g * (1-fx) * fy + c11_g * fx * fy);
  Bit8u b = (Bit8u)(c00_b * (1-fx) * (1-fy) + c01_b * fx * (1-fy) + c10_b * (1-fx) * fy + c11_b * fx * fy);
  Bit8u a = (Bit8u)(c00_a * (1-fx) * (1-fy) + c01_a * fx * (1-fy) + c10_a * (1-fx) * fy + c11_a * fx * fy);
  
  return (a << 24) | (r << 16) | (g << 8) | b;
}

Bit32u bx_nvriva_c::d3d_sample_texture_mipmap(nv04_channel* ch, Bit32u tex_obj, float u, float v, float lod)
{
  // Mipmap texture sampling
  // Calculate mipmap level based on LOD
  int mip_level = (int)(lod + ch->d3d_mipmap_lodbias);
  mip_level = BX_MAX(0, BX_MIN(7, mip_level)); // Clamp to valid mipmap levels
  
  // Calculate texture dimensions for this mipmap level
  int tex_width = 256 >> mip_level;
  int tex_height = 256 >> mip_level;
  tex_width = BX_MAX(1, tex_width);
  tex_height = BX_MAX(1, tex_height);
  
  // Calculate mipmap offset (simplified - assumes consecutive mipmap levels)
  Bit32u mip_offset = 0;
  for (int i = 0; i < mip_level; i++) {
    int level_width = 256 >> i;
    int level_height = 256 >> i;
    level_width = BX_MAX(1, level_width);
    level_height = BX_MAX(1, level_height);
    mip_offset += level_width * level_height * 4;
  }
  
  // Sample texture at mipmap level with offset
  if (ch->d3d_minify_filter == 0x3 || ch->d3d_minify_filter == 0x5) {
    // Nearest mipmap filtering
    return d3d_sample_texture_nearest(ch, tex_obj + ch->d3d_texture_offset[0] + mip_offset, u, v, lod);
  } else {
    // Linear mipmap filtering
    return d3d_sample_texture_linear(ch, tex_obj + ch->d3d_texture_offset[0] + mip_offset, u, v, lod);
  }
}

Bit32u bx_nvriva_c::d3d_get_texture_offset(nv04_channel* ch)
{
  // Get the current texture offset for texture unit 0
  return ch->d3d_texture_offset[0];
}

bool bx_nvriva_c::d3d_is_colorkey(nv04_channel* ch, Bit32u color)
{
  // Check if a color matches the colorkey
  if (!ch->d3d_colorkey_enable) {
    return false; // Colorkey disabled
  }
  
  // Extract color components
  Bit8u color_r = (color >> 16) & 0xFF;
  Bit8u color_g = (color >> 8) & 0xFF;
  Bit8u color_b = color & 0xFF;
  Bit8u color_a = (color >> 24) & 0xFF;
  
  // Compare with colorkey (exact match)
  return (color_r == ch->d3d_colorkey_r && 
          color_g == ch->d3d_colorkey_g && 
          color_b == ch->d3d_colorkey_b && 
          color_a == ch->d3d_colorkey_a);
}

Bit32u bx_nvriva_c::d3d_apply_colorkey(nv04_channel* ch, Bit32u tex_color)
{
  // Apply colorkey transparency to texture color
  if (d3d_is_colorkey(ch, tex_color)) {
    // Return transparent color (alpha = 0)
    return tex_color & 0x00FFFFFF; // Clear alpha channel
  }
  
  // Return original color if not colorkey
  return tex_color;
}

void bx_nvriva_c::execute_iifc(nv04_channel* ch, Bit32u method, Bit32u param)
{
  if (method == 0x061)
    ch->iifc_palette = param;
  else if (method == 0x0f9)
    ch->iifc_operation = param;
  else if (method == 0x0fa) {
    ch->iifc_color_fmt = param;
    update_color_bytes_iifc(ch);
  } else if (method == 0x0fb)
    ch->iifc_bpp4 = param;
  else if (method == 0x0fc)
    ch->iifc_palette_ofs = param;
  else if (method == 0x0fd)
    ch->iifc_yx = param;
  else if (method == 0x0fe)
    ch->iifc_dhw = param;
  else if (method == 0x0ff) {
    ch->iifc_shw = param;
    Bit32u width = ch->iifc_shw & 0xFFFF;
    Bit32u height = ch->iifc_shw >> 16;
    Bit32u wordCount = ALIGN(width * height * (ch->iifc_bpp4 ? 4 : 8), 32) >> 5;
    if (ch->iifc_words != nullptr)
      delete[] ch->iifc_words;
    ch->iifc_words_ptr = 0;
    ch->iifc_words_left = wordCount;
    ch->iifc_words = new Bit32u[wordCount];
  }
  else if (method >= 0x100 && method < 0x800) {
    ch->iifc_words[ch->iifc_words_ptr++] = param;
    ch->iifc_words_left--;
    if (!ch->iifc_words_left) {
      iifc(ch);
      delete[] ch->iifc_words;
      ch->iifc_words = nullptr;
    }
  }
}

void bx_nvriva_c::execute_sifc(nv04_channel* ch, Bit32u method, Bit32u param)
{
  if (method == 0x0bf)
    ch->sifc_operation = param;
  else if (method == 0x0c0) {
    ch->sifc_color_fmt = param;
    update_color_bytes_sifc(ch);
  } else if (method == 0x0c1)
    ch->sifc_shw = param;
  else if (method == 0x0c2)
    ch->sifc_dxds = param;
  else if (method == 0x0c3)
    ch->sifc_dydt = param;
  else if (method == 0x0c4)
    ch->sifc_clip_yx = param;
  else if (method == 0x0c5)
    ch->sifc_clip_hw = param;
  else if (method == 0x0c6) {
    ch->sifc_syx = param;
    Bit32u width = ch->sifc_shw & 0xFFFF;
    Bit32u height = ch->sifc_shw >> 16;
    Bit32u wordCount = ALIGN(width * height * ch->sifc_color_bytes, 4) >> 2;
    if (ch->sifc_words != nullptr)
      delete[] ch->sifc_words;
    ch->sifc_words_ptr = 0;
    ch->sifc_words_left = wordCount;
    ch->sifc_words = new Bit32u[wordCount];
  }
  else if (method >= 0x100 && method < 0x800) {
    ch->sifc_words[ch->sifc_words_ptr++] = param;
    ch->sifc_words_left--;
    if (!ch->sifc_words_left) {
      sifc(ch);
      delete[] ch->sifc_words;
      ch->sifc_words = nullptr;
    }
  }
}

void bx_nvriva_c::execute_beta(nv04_channel* ch, Bit32u method, Bit32u param)
{
  if (method == 0x0c0)
    ch->beta = param;
}

void bx_nvriva_c::execute_tfc(nv04_channel* ch, Bit32u method, Bit32u param)
{
  if (method == 0x061) {
    Bit8u cls8 = ramin_read32(param);
    ch->tfc_swizzled = cls8 == 0x52 || cls8 == 0x9e;
  } else if (method == 0x0c0) {
    ch->tfc_color_fmt = param;
    update_color_bytes_tfc(ch);
  } else if (method == 0x0c1)
    ch->tfc_yx = param;
  else if (method == 0x0c2) {
    ch->tfc_hw = param;
    ch->tfc_upload = param == 0x01000100 && ch->tfc_yx == 0 &&
      ch->tfc_color_fmt == 4 && ch->s2d_color_fmt == 0xA &&
      ch->s2d_pitch_src == 0x0400 && ch->s2d_pitch_dst == 0x0400;
    if (ch->tfc_upload) {
      ch->tfc_upload_offset = ch->s2d_ofs_dst;
    } else {
      Bit32u width = ch->tfc_hw & 0xFFFF;
      Bit32u height = ch->tfc_hw >> 16;
      Bit32u wordCount = ALIGN(width * height * ch->tfc_color_bytes, 4) >> 2;
      if (ch->tfc_words != nullptr)
        delete[] ch->tfc_words;
      ch->tfc_words_ptr = 0;
      ch->tfc_words_left = wordCount;
      ch->tfc_words = new Bit32u[wordCount];
    }
  } else if (method == 0x0c3)
    ch->tfc_clip_wx = param;
  else if (method == 0x0c4)
    ch->tfc_clip_hy = param;
  else if (method >= 0x100 && method < 0x800) {
    if (ch->tfc_upload) {
      dma_write32(ch->s2d_img_dst, ch->tfc_upload_offset, param);
      ch->tfc_upload_offset += 4;
    } else if (ch->tfc_words != nullptr) {
      ch->tfc_words[ch->tfc_words_ptr++] = param;
      ch->tfc_words_left--;
      if (!ch->tfc_words_left) {
        tfc(ch);
        delete[] ch->tfc_words;
        ch->tfc_words = nullptr;
      }
    }
  }
}

void bx_nvriva_c::execute_sifm(nv04_channel* ch, Bit32u method, Bit32u param)
{
  if (method == 0x061)
    ch->sifm_src = param;
  else if (method == 0x066) {
    Bit8u cls8 = ramin_read32(param);
    ch->sifm_swizzled = cls8 == 0x52 || cls8 == 0x9e;
  } else if (method == 0x0c0) {
    ch->sifm_color_fmt = param;
    if (ch->sifm_color_fmt == 8)        // ???
      ch->sifm_color_bytes = 1;
    else if (ch->sifm_color_fmt == 1 || // A1R5G5B5
             ch->sifm_color_fmt == 2 || // X1R5G5B5
             ch->sifm_color_fmt == 7)   // R5G6B5
      ch->sifm_color_bytes = 2;
    else if (ch->sifm_color_fmt == 3 || // A8R8G8B8
             ch->sifm_color_fmt == 4)   // X8R8G8B8
      ch->sifm_color_bytes = 4;
    else {
      BX_ERROR(("unknown sifm color format: 0x%02x",
        ch->sifm_color_fmt));
    }
  } else if (method == 0x0c1)
    ch->sifm_operation = param;
  else if (method == 0x0c4)
    ch->sifm_dyx = param;
  else if (method == 0x0c5)
    ch->sifm_dhw = param;
  else if (method == 0x0c6)
    ch->sifm_dudx = param;
  else if (method == 0x0c7)
    ch->sifm_dvdy = param;
  else if (method == 0x100)
    ch->sifm_shw = param;
  else if (method == 0x101)
    ch->sifm_sfmt = param;
  else if (method == 0x102)
    ch->sifm_sofs = param;
  else if (method == 0x103) {
    ch->sifm_syx = param;
    sifm(ch);
  }
}

void bx_nvriva_c::execute_d3d(nv04_channel* ch, Bit32u cls, Bit32u method, Bit32u param)
{
  // D3D object assignments
  if (method == 0x060)
    ch->d3d_a_obj = param;
  else if (method == 0x061)
    ch->d3d_b_obj = param;
  else if (method == 0x065)
    ch->d3d_color_obj = param;
  else if (method == 0x066)
    ch->d3d_zeta_obj = param;
  else if (method == 0x067)
    ch->d3d_vertex_a_obj = param;
  else if (method == 0x068)
    ch->d3d_vertex_b_obj = param;
  else if (method == 0x069)
    ch->d3d_semaphore_obj = param;
  else if (method == 0x06a)
    ch->d3d_report_obj = param;
  else if (method == NV04_CONTEXT_SURFACES_3D_CLIP_HORIZONTAL) {
    // NV04_CONTEXT_SURFACES_3D_CLIP_HORIZONTAL
    ch->d3d_clip_horizontal = param;
    Bit32u clip_x = param & 0xFFFF;
    Bit32u clip_w = param >> 16;
    BX_DEBUG(("D3D clip horizontal: x=%d, w=%d", clip_x, clip_w));
  } else if (method == NV04_CONTEXT_SURFACES_3D_CLIP_VERTICAL) {
    // NV04_CONTEXT_SURFACES_3D_CLIP_VERTICAL
    ch->d3d_clip_vertical = param;
    Bit32u clip_y = param & 0xFFFF;
    Bit32u clip_h = param >> 16;
    BX_DEBUG(("D3D clip vertical: y=%d, h=%d", clip_y, clip_h));
  }
  else if (method == NV04_CONTEXT_SURFACES_3D_FORMAT) {
    // NV04_CONTEXT_SURFACES_3D_FORMAT
    ch->d3d_surface_format = param;
    Bit32u format_color = param & 0xFF;
    Bit32u format_depth = (param >> 8) & 0xFF;
    Bit32u format_type = (param >> 8) & 0xFF;
    
    // Handle NV04_3D XML format types
    if (format_type == 0x01) { // NV04_CONTEXT_SURFACES_3D_FORMAT_TYPE_PITCH
      BX_DEBUG(("D3D surface format: PITCH type"));
    } else if (format_type == 0x02) { // NV04_CONTEXT_SURFACES_3D_FORMAT_TYPE_SWIZZLE
      BX_DEBUG(("D3D surface format: SWIZZLE type"));
    }
    
    // Handle color formats from NV04_3D XML
    if (format_color == 0x01)      // X1R5G5B5_Z1R5G5B5
      ch->d3d_color_bytes = 2;
    else if (format_color == 0x02) // X1R5G5B5_X1R5G5B5
      ch->d3d_color_bytes = 2;
    else if (format_color == 0x03) // R5G6B5
      ch->d3d_color_bytes = 2;
    else if (format_color == 0x04) // X8R8G8B8_Z8R8G8B8
      ch->d3d_color_bytes = 4;
    else if (format_color == 0x05) // X8R8G8B8_X8R8G8B8
      ch->d3d_color_bytes = 4;
    else if (format_color == 0x06) // X1A7R8G8B8_Z1A7R8G8B8
      ch->d3d_color_bytes = 4;
    else if (format_color == 0x07) // X1A7R8G8B8_X1A7R8G8B8
      ch->d3d_color_bytes = 4;
    else if (format_color == 0x08) // A8R8G8B8
      ch->d3d_color_bytes = 4;
    else
      ch->d3d_color_bytes = 4; // default
    
    if (format_depth == 0x01)      // Z16
      ch->d3d_depth_bytes = 2;
    else if (format_depth == 0x02) // Z24
      ch->d3d_depth_bytes = 4;
    else if (format_depth == 0x03) // Z32
      ch->d3d_depth_bytes = 4;
    else
      ch->d3d_depth_bytes = 4; // default
  } else if (method == NV04_CONTEXT_SURFACES_3D_PITCH) {
    // NV04_CONTEXT_SURFACES_3D_PITCH
    ch->d3d_surface_pitch_a = param;
    Bit32u pitch_color = param & 0xFFFF;
    Bit32u pitch_zeta = param >> 16;
    BX_DEBUG(("D3D surface pitch: color=%d, zeta=%d", pitch_color, pitch_zeta));
  } else if (method == 0x084) {
    // Additional pitch register (not in NV04_3D XML, but used for compatibility)
    ch->d3d_surface_pitch_z = param;
  }
  else if (method == 0x085)
    ch->d3d_window_offset_x = param;
  else if (method == 0x086)
    ch->d3d_window_offset_y = param;
  else if (method == NV04_CONTEXT_SURFACES_3D_OFFSET_COLOR) {
    // NV04_CONTEXT_SURFACES_3D_OFFSET_COLOR
    ch->d3d_surface_color_offset = param;
    BX_DEBUG(("D3D color offset: 0x%08x", param));
  } else if (method == NV04_CONTEXT_SURFACES_3D_OFFSET_ZETA) {
    // NV04_CONTEXT_SURFACES_3D_OFFSET_ZETA
    ch->d3d_surface_zeta_offset = param;
    BX_DEBUG(("D3D zeta offset: 0x%08x", param));
  }
  else if (method == 0x089)
    ch->d3d_alpha_test_enable = param;
  else if (method == 0x08a)
    ch->d3d_alpha_func = param;
  else if (method == 0x08b)
    ch->d3d_alpha_ref = param;
  else if (method == 0x08c)
    ch->d3d_blend_enable = param;
  else if (method == 0x08d)
    ch->d3d_blend_sfactor_rgb = param;
  else if (method == 0x08e)
    ch->d3d_blend_sfactor_alpha = param;
  else if (method == 0x08f)
    ch->d3d_blend_dfactor_rgb = param;
  else if (method == 0x090)
    ch->d3d_blend_dfactor_alpha = param;
  else if (method == 0x091)
    ch->d3d_blend_equation_rgb = param;
  else if (method == 0x092)
    ch->d3d_blend_equation_alpha = param;
  else if (method == 0x093)
    ch->d3d_blend_color[0] = *(float*)&param;
  else if (method == 0x094)
    ch->d3d_blend_color[1] = *(float*)&param;
  else if (method == 0x095)
    ch->d3d_blend_color[2] = *(float*)&param;
  else if (method == 0x096)
    ch->d3d_blend_color[3] = *(float*)&param;
  else if (method == 0x097)
    ch->d3d_cull_face_enable = param;
  else if (method == 0x098)
    ch->d3d_depth_test_enable = param;
  else if (method == 0x099)
    ch->d3d_depth_write_enable = param;
  else if (method == 0x09a)
    ch->d3d_lighting_enable = param;
  else if (method == 0x09b)
    ch->d3d_depth_func = param;
  else if (method == 0x09c)
    ch->d3d_shade_mode = param;
  else if (method == 0x09d)
    ch->d3d_clip_min = *(float*)&param;
  else if (method == 0x09e)
    ch->d3d_clip_max = *(float*)&param;
  else if (method == 0x09f)
    ch->d3d_cull_face = param;
  else if (method == 0x0a0)
    ch->d3d_front_face = param;
  else if (method == 0x0a1)
    ch->d3d_light_enable_mask = param;
  else if (method == 0x0a2)
    ch->d3d_inverse_model_view_matrix[0] = *(float*)&param;
  else if (method == 0x0a3)
    ch->d3d_inverse_model_view_matrix[1] = *(float*)&param;
  else if (method == 0x0a4)
    ch->d3d_inverse_model_view_matrix[2] = *(float*)&param;
  else if (method == 0x0a5)
    ch->d3d_inverse_model_view_matrix[3] = *(float*)&param;
  else if (method == 0x0a6)
    ch->d3d_inverse_model_view_matrix[4] = *(float*)&param;
  else if (method == 0x0a7)
    ch->d3d_inverse_model_view_matrix[5] = *(float*)&param;
  else if (method == 0x0a8)
    ch->d3d_inverse_model_view_matrix[6] = *(float*)&param;
  else if (method == 0x0a9)
    ch->d3d_inverse_model_view_matrix[7] = *(float*)&param;
  else if (method == 0x0aa)
    ch->d3d_inverse_model_view_matrix[8] = *(float*)&param;
  else if (method == 0x0ab)
    ch->d3d_inverse_model_view_matrix[9] = *(float*)&param;
  else if (method == 0x0ac)
    ch->d3d_inverse_model_view_matrix[10] = *(float*)&param;
  else if (method == 0x0ad)
    ch->d3d_inverse_model_view_matrix[11] = *(float*)&param;
  else if (method == 0x0ae)
    ch->d3d_composite_matrix[0] = *(float*)&param;
  else if (method == 0x0af)
    ch->d3d_composite_matrix[1] = *(float*)&param;
  else if (method == 0x0b0)
    ch->d3d_composite_matrix[2] = *(float*)&param;
  else if (method == 0x0b1)
    ch->d3d_composite_matrix[3] = *(float*)&param;
  else if (method == 0x0b2)
    ch->d3d_composite_matrix[4] = *(float*)&param;
  else if (method == 0x0b3)
    ch->d3d_composite_matrix[5] = *(float*)&param;
  else if (method == 0x0b4)
    ch->d3d_composite_matrix[6] = *(float*)&param;
  else if (method == 0x0b5)
    ch->d3d_composite_matrix[7] = *(float*)&param;
  else if (method == 0x0b6)
    ch->d3d_composite_matrix[8] = *(float*)&param;
  else if (method == 0x0b7)
    ch->d3d_composite_matrix[9] = *(float*)&param;
  else if (method == 0x0b8)
    ch->d3d_composite_matrix[10] = *(float*)&param;
  else if (method == 0x0b9)
    ch->d3d_composite_matrix[11] = *(float*)&param;
  else if (method == 0x0ba)
    ch->d3d_composite_matrix[12] = *(float*)&param;
  else if (method == 0x0bb)
    ch->d3d_composite_matrix[13] = *(float*)&param;
  else if (method == 0x0bc)
    ch->d3d_composite_matrix[14] = *(float*)&param;
  else if (method == 0x0bd)
    ch->d3d_composite_matrix[15] = *(float*)&param;
  else if (method == 0x0be)
    ch->d3d_scissor_x = param;
  else if (method == 0x0bf)
    ch->d3d_scissor_width = param;
  else if (method == 0x0c0)
    ch->d3d_scissor_y = param;
  else if (method == 0x0c1)
    ch->d3d_scissor_height = param;
  else if (method == 0x0c2)
    ch->d3d_shader_program = param;
  else if (method == 0x0c3)
    ch->d3d_shader_obj = param;
  else if (method == 0x0c4)
    ch->d3d_shader_offset = param;
  else if (method == 0x0c5)
    ch->d3d_scene_ambient_color[0] = *(float*)&param;
  else if (method == 0x0c6)
    ch->d3d_scene_ambient_color[1] = *(float*)&param;
  else if (method == 0x0c7)
    ch->d3d_scene_ambient_color[2] = *(float*)&param;
  else if (method == 0x0c8)
    ch->d3d_scene_ambient_color[3] = *(float*)&param;
  else if (method == 0x0c9)
    ch->d3d_viewport_horizontal = param;
  else if (method == 0x0ca)
    ch->d3d_viewport_vertical = param;
  else if (method == 0x0cb)
    ch->d3d_viewport_offset[0] = *(float*)&param;
  else if (method == 0x0cc)
    ch->d3d_viewport_offset[1] = *(float*)&param;
  else if (method == 0x0cd)
    ch->d3d_viewport_offset[2] = *(float*)&param;
  else if (method == 0x0ce)
    ch->d3d_viewport_offset[3] = *(float*)&param;
  else if (method == 0x0cf)
    ch->d3d_viewport_scale[0] = *(float*)&param;
  else if (method == 0x0d0)
    ch->d3d_viewport_scale[1] = *(float*)&param;
  else if (method == 0x0d1)
    ch->d3d_viewport_scale[2] = *(float*)&param;
  else if (method == 0x0d2)
    ch->d3d_viewport_scale[3] = *(float*)&param;
  else if (method == 0x0d3)
    ch->d3d_transform_program[0][0] = param;
  else if (method == 0x0d4)
    ch->d3d_transform_program[0][1] = param;
  else if (method == 0x0d5)
    ch->d3d_transform_program[0][2] = param;
  else if (method == 0x0d6)
    ch->d3d_transform_program[0][3] = param;
  else if (method == 0x0d7)
    ch->d3d_transform_constant[0][0] = *(float*)&param;
  else if (method == 0x0d8)
    ch->d3d_transform_constant[0][1] = *(float*)&param;
  else if (method == 0x0d9)
    ch->d3d_transform_constant[0][2] = *(float*)&param;
  else if (method == 0x0da)
    ch->d3d_transform_constant[0][3] = *(float*)&param;
  else if (method == 0x0db)
    ch->d3d_light_diffuse_color[0][0] = *(float*)&param;
  else if (method == 0x0dc)
    ch->d3d_light_diffuse_color[0][1] = *(float*)&param;
  else if (method == 0x0dd)
    ch->d3d_light_diffuse_color[0][2] = *(float*)&param;
  else if (method == 0x0de)
    ch->d3d_light_infinite_direction[0][0] = *(float*)&param;
  else if (method == 0x0df)
    ch->d3d_light_infinite_direction[0][1] = *(float*)&param;
  else if (method == 0x0e0)
    ch->d3d_light_infinite_direction[0][2] = *(float*)&param;
  else if (method == 0x0e1)
    ch->d3d_normal[0] = *(float*)&param;
  else if (method == 0x0e2)
    ch->d3d_normal[1] = *(float*)&param;
  else if (method == 0x0e3)
    ch->d3d_normal[2] = *(float*)&param;
  else if (method == 0x0e4)
    ch->d3d_diffuse_color[0] = *(float*)&param;
  else if (method == 0x0e5)
    ch->d3d_diffuse_color[1] = *(float*)&param;
  else if (method == 0x0e6)
    ch->d3d_diffuse_color[2] = *(float*)&param;
  else if (method == 0x0e7)
    ch->d3d_diffuse_color[3] = *(float*)&param;
  else if (method == 0x0e8)
    ch->d3d_texcoord[0][0] = *(float*)&param;
  else if (method == 0x0e9)
    ch->d3d_texcoord[0][1] = *(float*)&param;
  else if (method == 0x0ea)
    ch->d3d_texcoord[0][2] = *(float*)&param;
  else if (method == 0x0eb)
    ch->d3d_texcoord[0][3] = *(float*)&param;
  else if (method == 0x0ec)
    ch->d3d_texcoord[1][0] = *(float*)&param;
  else if (method == 0x0ed)
    ch->d3d_texcoord[1][1] = *(float*)&param;
  else if (method == 0x0ee)
    ch->d3d_texcoord[1][2] = *(float*)&param;
  else if (method == 0x0ef)
    ch->d3d_texcoord[1][3] = *(float*)&param;
  else if (method == 0x0f0)
    ch->d3d_texcoord[2][0] = *(float*)&param;
  else if (method == 0x0f1)
    ch->d3d_texcoord[2][1] = *(float*)&param;
  else if (method == 0x0f2)
    ch->d3d_texcoord[2][2] = *(float*)&param;
  else if (method == 0x0f3)
    ch->d3d_texcoord[2][3] = *(float*)&param;
  else if (method == 0x0f4)
    ch->d3d_texcoord[3][0] = *(float*)&param;
  else if (method == 0x0f5)
    ch->d3d_texcoord[3][1] = *(float*)&param;
  else if (method == 0x0f6)
    ch->d3d_texcoord[3][2] = *(float*)&param;
  else if (method == 0x0f7)
    ch->d3d_texcoord[3][3] = *(float*)&param;
  else if (method == 0x0f8)
    ch->d3d_vertex_data_array_offset[0] = param;
  else if (method == 0x0f9)
    ch->d3d_vertex_data_array_format_type[0] = param;
  else if (method == 0x0fa)
    ch->d3d_vertex_data_array_format_size[0] = param;
  else if (method == 0x0fb)
    ch->d3d_vertex_data_array_format_stride[0] = param;
  else if (method == 0x0fc)
    ch->d3d_vertex_data_array_format_dx[0] = (param != 0);
  else if (method == 0x0fd)
    ch->d3d_begin_end = param;
  else if (method == 0x0fe)
    ch->d3d_primitive_done = (param != 0);
  else if (method == 0x0ff)
    ch->d3d_triangle_flip = (param != 0);
  else if (method == 0x100)
    ch->d3d_vertex_index = param;
  else if (method == 0x101)
    ch->d3d_attrib_index = param;
  else if (method == 0x102)
    ch->d3d_comp_index = param;
  else if (method == 0x103)
    ch->d3d_vertex_data[0][0][0] = *(float*)&param;
  else if (method == 0x104)
    ch->d3d_vertex_data[0][0][1] = *(float*)&param;
  else if (method == 0x105)
    ch->d3d_vertex_data[0][0][2] = *(float*)&param;
  else if (method == 0x106)
    ch->d3d_vertex_data[0][0][3] = *(float*)&param;
  else if (method == 0x107)
    ch->d3d_index_array_offset = param;
  else if (method == 0x108)
    ch->d3d_index_array_dma = param;
  else if (method == 0x109)
    ch->d3d_texture_offset[0] = param;
  else if (method == 0x10a)
    ch->d3d_texture_format[0] = param;
  else if (method == 0x10b)
    ch->d3d_texture_address[0] = param;
  else if (method == 0x10c)
    ch->d3d_texture_control0[0] = param;
  else if (method == 0x10d)
    ch->d3d_texture_control1[0] = param;
  else if (method == 0x10e)
    ch->d3d_texture_filter[0] = param;
  else if (method == 0x10f)
    ch->d3d_texture_image_rect[0] = param;
  else if (method == 0x110)
    ch->d3d_texture_palette[0] = param;
  else if (method == 0x111)
    ch->d3d_texture_control3[0] = param;
  // D3D semaphore operations - NV04_3D/NV05_3D specific
  // Based on NV04_3D XML: DMA_NOTIFY at 0x180, DMA_COLOR at 0x184, DMA_ZETA at 0x188
  else if (method == NV04_CONTEXT_SURFACES_3D_DMA_NOTIFY) {
    // NV04_CONTEXT_SURFACES_3D_DMA_NOTIFY / NV04_TEXTURED_TRIANGLE_DMA_NOTIFY
    // Set DMA notification object for semaphore operations
    ch->d3d_semaphore_obj = param;
    BX_DEBUG(("D3D DMA_NOTIFY semaphore object: 0x%08x", param));
  } else if (method == NV04_CONTEXT_SURFACES_3D_DMA_COLOR) {
    // NV04_CONTEXT_SURFACES_3D_DMA_COLOR / NV04_TEXTURED_TRIANGLE_DMA_A
    // Set color buffer DMA object (used for semaphore operations)
    ch->d3d_color_obj = param;
    BX_DEBUG(("D3D DMA_COLOR object: 0x%08x", param));
  } else if (method == NV04_CONTEXT_SURFACES_3D_DMA_ZETA) {
    // NV04_CONTEXT_SURFACES_3D_DMA_ZETA / NV04_TEXTURED_TRIANGLE_DMA_B
    // Set zeta buffer DMA object (used for semaphore operations)
    ch->d3d_zeta_obj = param;
    BX_DEBUG(("D3D DMA_ZETA object: 0x%08x", param));
  } else if (method == 0x75b) {
    // Legacy semaphore offset method (for compatibility)
    ch->d3d_semaphore_offset = param;
    BX_DEBUG(("D3D semaphore offset: 0x%08x", param));
  } else if (method == 0x75c) {
    // Legacy semaphore write method (for compatibility)
    // Write semaphore value using DMA_NOTIFY object
    if (ch->d3d_semaphore_obj) {
      dma_write32(ch->d3d_semaphore_obj, ch->d3d_semaphore_offset, param);
      BX_DEBUG(("D3D semaphore write: obj=0x%08x, offset=0x%08x, value=0x%08x",
        ch->d3d_semaphore_obj, ch->d3d_semaphore_offset, param));
    } else {
      BX_DEBUG(("D3D semaphore write: no semaphore object set"));
    }
  } else if (method == 0x763)
    ch->d3d_zstencil_clear_value = param;
  else if (method == 0x764)
    ch->d3d_color_clear_value = param;
  else if (method == 0x765) {
    ch->d3d_clear_surface = param;
    d3d_clear_surface(ch);
  } else if (method == 0x7a5)
    ch->d3d_transform_execution_mode = param;
  else if (method == 0x7a7)
    ch->d3d_transform_program_load = param;
  else if (method == 0x7a8)
    ch->d3d_transform_program_start = param;
  else if (method == 0x7a9)
    ch->d3d_transform_constant_load = param;
  else if (method == 0x7f1)
    ch->d3d_attrib_color = param;
  else if (method == 0x7f2) {
    ch->d3d_dci = param & 0xf;
    for (Bit32u i = 0; i < 8; i++)
      ch->d3d_attrib_tex_coord[i] = (param >> (i * 4)) & 0xf;
  } else if (method == 0x7f3) {
    for (Bit32u i = 0; i < 2; i++)
      ch->d3d_attrib_tex_coord[i + 8] = (param >> (i * 4)) & 0xf;
  }   else if (method == 0x7f4)
    ch->d3d_tex_coord_count = param;
  // NV04_3D Textured Triangle specific methods
  else if (method == NV04_TEXTURED_TRIANGLE_COLORKEY) {
    // NV04_TEXTURED_TRIANGLE_COLORKEY
    ch->d3d_colorkey = param;
    
    // Extract RGBA components according to NV04_3D XML masks
    ch->d3d_colorkey_b = (param & NV04_TEXTURED_TRIANGLE_COLORKEY_B__MASK) >> NV04_TEXTURED_TRIANGLE_COLORKEY_B__SHIFT;
    ch->d3d_colorkey_g = (param & NV04_TEXTURED_TRIANGLE_COLORKEY_G__MASK) >> NV04_TEXTURED_TRIANGLE_COLORKEY_G__SHIFT;
    ch->d3d_colorkey_r = (param & NV04_TEXTURED_TRIANGLE_COLORKEY_R__MASK) >> NV04_TEXTURED_TRIANGLE_COLORKEY_R__SHIFT;
    ch->d3d_colorkey_a = (param & NV04_TEXTURED_TRIANGLE_COLORKEY_A__MASK) >> NV04_TEXTURED_TRIANGLE_COLORKEY_A__SHIFT;
    
    BX_DEBUG(("D3D textured triangle colorkey: 0x%08x (R=%d, G=%d, B=%d, A=%d)", 
      param, ch->d3d_colorkey_r, ch->d3d_colorkey_g, ch->d3d_colorkey_b, ch->d3d_colorkey_a));
  } else if (method == NV04_TEXTURED_TRIANGLE_OFFSET) {
    // NV04_TEXTURED_TRIANGLE_OFFSET
    ch->d3d_offset = param;
    // Set texture offset for all texture units
    for (int i = 0; i < 16; i++) {
      ch->d3d_texture_offset[i] = param;
    }
    BX_DEBUG(("D3D textured triangle offset: 0x%08x", param));
  } else if (method == NV04_TEXTURED_TRIANGLE_FORMAT) {
    // NV04_TEXTURED_TRIANGLE_FORMAT
    Bit32u format_flags = param;
    bool dma_a = format_flags & 0x01;
    bool dma_b = format_flags & 0x02;
    ch->d3d_colorkey_enable = (format_flags & 0x04) != 0;
    BX_DEBUG(("D3D textured triangle format: dma_a=%d, dma_b=%d, colorkey=%d", 
      dma_a, dma_b, ch->d3d_colorkey_enable));
  } else if (method == NV04_TEXTURED_TRIANGLE_FILTER) {
    // NV04_TEXTURED_TRIANGLE_FILTER
    ch->d3d_filter = param;
    
    // Extract kernel size parameters
    ch->d3d_kernel_size_x = (param & NV04_TEXTURED_TRIANGLE_FILTER_KERNEL_SIZE_X__MASK) >> NV04_TEXTURED_TRIANGLE_FILTER_KERNEL_SIZE_X__SHIFT;
    ch->d3d_kernel_size_y = (param & NV04_TEXTURED_TRIANGLE_FILTER_KERNEL_SIZE_Y__MASK) >> NV04_TEXTURED_TRIANGLE_FILTER_KERNEL_SIZE_Y__SHIFT;
    
    // Extract mipmap parameters
    ch->d3d_mipmap_dither_enable = (param & NV04_TEXTURED_TRIANGLE_FILTER_MIPMAP_DITHER_ENABLE) != 0;
    ch->d3d_mipmap_lodbias = (param & NV04_TEXTURED_TRIANGLE_FILTER_MIPMAP_LODBIAS__MASK) >> NV04_TEXTURED_TRIANGLE_FILTER_MIPMAP_LODBIAS__SHIFT;
    
    // Extract filter modes
    ch->d3d_minify_filter = (param & NV04_TEXTURED_TRIANGLE_FILTER_MINIFY__MASK) >> NV04_TEXTURED_TRIANGLE_FILTER_MINIFY__SHIFT;
    ch->d3d_magnify_filter = (param & NV04_TEXTURED_TRIANGLE_FILTER_MAGNIFY__MASK) >> NV04_TEXTURED_TRIANGLE_FILTER_MAGNIFY__SHIFT;
    
    // Extract anisotropic filtering flags
    ch->d3d_anisotropic_minify_enable = (param & NV04_TEXTURED_TRIANGLE_FILTER_ANISOTROPIC_MINIFY_ENABLE) != 0;
    ch->d3d_anisotropic_magnify_enable = (param & NV04_TEXTURED_TRIANGLE_FILTER_ANISOTROPIC_MAGNIFY_ENABLE) != 0;
    
    BX_DEBUG(("D3D textured triangle filter: 0x%08x (kernel_x=%d, kernel_y=%d, mipmap_dither=%d, lodbias=%d, minify=%d, magnify=%d, aniso_min=%d, aniso_mag=%d)", 
      param, ch->d3d_kernel_size_x, ch->d3d_kernel_size_y, ch->d3d_mipmap_dither_enable, ch->d3d_mipmap_lodbias, 
      ch->d3d_minify_filter, ch->d3d_magnify_filter, ch->d3d_anisotropic_minify_enable, ch->d3d_anisotropic_magnify_enable));
  } else if (method == NV04_TEXTURED_TRIANGLE_BLEND) {
    // NV04_TEXTURED_TRIANGLE_BLEND
    ch->d3d_blend = param;
    
    // Extract texture mapping mode
    ch->d3d_texture_map = (param & NV04_TEXTURED_TRIANGLE_BLEND_TEXTURE_MAP__MASK) >> NV04_TEXTURED_TRIANGLE_BLEND_TEXTURE_MAP__SHIFT;
    
    // Extract mask bit mode
    ch->d3d_mask_bit = (param & NV04_TEXTURED_TRIANGLE_BLEND_MASK_BIT__MASK) >> NV04_TEXTURED_TRIANGLE_BLEND_MASK_BIT__SHIFT;
    
    // Extract shading mode
    ch->d3d_shade_mode = (param & NV04_TEXTURED_TRIANGLE_BLEND_SHADE_MODE__MASK) >> NV04_TEXTURED_TRIANGLE_BLEND_SHADE_MODE__SHIFT;
    
    // Extract feature flags
    ch->d3d_texture_perspective_enable = (param & NV04_TEXTURED_TRIANGLE_BLEND_TEXTURE_PERSPECTIVE_ENABLE) != 0;
    ch->d3d_specular_enable = (param & NV04_TEXTURED_TRIANGLE_BLEND_SPECULAR_ENABLE) != 0;
    ch->d3d_fog_enable = (param & NV04_TEXTURED_TRIANGLE_BLEND_FOG_ENABLE) != 0;
    ch->d3d_blend_enable = (param & NV04_TEXTURED_TRIANGLE_BLEND_BLEND_ENABLE) != 0;
    
    // Extract blend factors
    ch->d3d_blend_src = (param & NV04_TEXTURED_TRIANGLE_BLEND_SRC__MASK) >> NV04_TEXTURED_TRIANGLE_BLEND_SRC__SHIFT;
    ch->d3d_blend_dst = (param & NV04_TEXTURED_TRIANGLE_BLEND_DST__MASK) >> NV04_TEXTURED_TRIANGLE_BLEND_DST__SHIFT;
    
    BX_DEBUG(("D3D textured triangle blend: 0x%08x (texture_map=%d, mask_bit=%d, shade_mode=%d, perspective=%d, specular=%d, fog=%d, blend=%d, src=%d, dst=%d)", 
      param, ch->d3d_texture_map, ch->d3d_mask_bit, ch->d3d_shade_mode, ch->d3d_texture_perspective_enable, 
      ch->d3d_specular_enable, ch->d3d_fog_enable, ch->d3d_blend_enable, ch->d3d_blend_src, ch->d3d_blend_dst));
  } else if (method == NV04_TEXTURED_TRIANGLE_CONTROL) {
    // NV04_TEXTURED_TRIANGLE_CONTROL
    ch->d3d_control = param;
    
    // Extract alpha test parameters
    ch->d3d_alpha_ref = (param & NV04_TEXTURED_TRIANGLE_CONTROL_ALPHA_REF__MASK) >> NV04_TEXTURED_TRIANGLE_CONTROL_ALPHA_REF__SHIFT;
    ch->d3d_alpha_func = (param & NV04_TEXTURED_TRIANGLE_CONTROL_ALPHA_FUNC__MASK) >> NV04_TEXTURED_TRIANGLE_CONTROL_ALPHA_FUNC__SHIFT;
    ch->d3d_alpha_enable = (param & NV04_TEXTURED_TRIANGLE_CONTROL_ALPHA_ENABLE) != 0;
    
    // Extract origin mode
    ch->d3d_origin_center = (param & NV04_TEXTURED_TRIANGLE_CONTROL_ORIGIN__MASK) == NV04_TEXTURED_TRIANGLE_CONTROL_ORIGIN_CENTER;
    
    // Extract Z-buffer parameters
    ch->d3d_z_enable = (param & NV04_TEXTURED_TRIANGLE_CONTROL_Z_ENABLE) != 0;
    ch->d3d_z_func = (param & NV04_TEXTURED_TRIANGLE_CONTROL_Z_FUNC__MASK) >> NV04_TEXTURED_TRIANGLE_CONTROL_Z_FUNC__SHIFT;
    ch->d3d_z_perspective_enable = (param & NV04_TEXTURED_TRIANGLE_CONTROL_Z_PERSPECTIVE_ENABLE) != 0;
    ch->d3d_z_write = (param & NV04_TEXTURED_TRIANGLE_CONTROL_Z_WRITE) != 0;
    ch->d3d_z_format = (param & NV04_TEXTURED_TRIANGLE_CONTROL_Z_FORMAT__MASK) >> NV04_TEXTURED_TRIANGLE_CONTROL_Z_FORMAT__SHIFT;
    
    // Extract culling parameters
    ch->d3d_cull_mode = (param & NV04_TEXTURED_TRIANGLE_CONTROL_CULL_MODE__MASK) >> NV04_TEXTURED_TRIANGLE_CONTROL_CULL_MODE__SHIFT;
    
    // Extract other flags
    ch->d3d_dither_enable = (param & NV04_TEXTURED_TRIANGLE_CONTROL_DITHER_ENABLE) != 0;
    
    BX_DEBUG(("D3D textured triangle control: 0x%08x (alpha_ref=%d, alpha_func=%d, alpha_enable=%d, origin_center=%d, z_enable=%d, z_func=%d, cull_mode=%d, dither=%d, z_write=%d, z_format=%d)", 
      param, ch->d3d_alpha_ref, ch->d3d_alpha_func, ch->d3d_alpha_enable, ch->d3d_origin_center, 
      ch->d3d_z_enable, ch->d3d_z_func, ch->d3d_cull_mode, ch->d3d_dither_enable, ch->d3d_z_write, ch->d3d_z_format));
  } else if (method == NV04_TEXTURED_TRIANGLE_FOGCOLOR) {
    // NV04_TEXTURED_TRIANGLE_FOGCOLOR
    // Extract RGBA components according to NV04_3D XML masks
    ch->d3d_fog_color = param;
    ch->d3d_fog_color_b = (param & 0x000000ff) >> 0;   // FOGCOLOR_B__SHIFT = 0
    ch->d3d_fog_color_g = (param & 0x0000ff00) >> 8;   // FOGCOLOR_G__SHIFT = 8
    ch->d3d_fog_color_r = (param & 0x00ff0000) >> 16;  // FOGCOLOR_R__SHIFT = 16
    ch->d3d_fog_color_a = (param & 0xff000000) >> 24;  // FOGCOLOR_A__SHIFT = 24
    BX_DEBUG(("D3D textured triangle fog color: 0x%08x (R=%d, G=%d, B=%d, A=%d)", 
      param, ch->d3d_fog_color_r, ch->d3d_fog_color_g, ch->d3d_fog_color_b, ch->d3d_fog_color_a));
  } else {
    BX_DEBUG(("execute_d3d: unknown method 0x%03x, param 0x%08x", method, param));
  }
}

bool bx_nvriva_c::execute_command(Bit32u chid, Bit32u subc, Bit32u method, Bit32u param)
{
  bool software_method = false;
  BX_DEBUG(("execute_command: chid 0x%02x, subc 0x%02x, method 0x%03x, param 0x%08x",
    chid, subc, method, param));
  nv04_channel* ch = &BX_NVRIVA_THIS chs[chid];
  if (method == 0x000) {
    if (ch->schs[subc].engine == 0x01) {
      Bit32u word1 = ramin_read32(ch->schs[subc].object + 0x4);
      word1 = (word1 & 0xFFF00000) | (ch->schs[subc].notifier >> 4);
      Bit32u word0 = ramin_read32(ch->schs[subc].object);
      Bit8u cls8 = word0;
      if (cls8 == 0x4a || cls8 == 0x4b) {
        word1 = (word1 & 0xFCFFFFFF) | (ch->gdi_mono_fmt << 24);
      } else if (cls8 == 0x62) {
        ramin_write32(ch->schs[subc].object + 0x8, ch->s2d_img_src >> 4);
        ramin_write32(ch->schs[subc].object + 0xC, ch->s2d_img_dst >> 4);
      } else if (cls8 == 0x60 || cls8 == 0x64) {
        ramin_write32(ch->schs[subc].object + 0x8, ch->iifc_palette >> 4);
        word0 = (word0 & 0xFFC7FFFF) | (ch->iifc_operation << 19);
        ramin_write32(ch->schs[subc].object, word0);
        word1 = (word1 & 0xFFFF00FF) | ((ch->iifc_color_fmt + 9) << 8);
        ramin_write32(ch->schs[subc].object + 0x10, ch->iifc_color_fmt);
      }
      ramin_write32(ch->schs[subc].object + 0x4, word1);
    }
    ramht_lookup(param, chid,
      &ch->schs[subc].object,
      &ch->schs[subc].engine);
    if (ch->schs[subc].engine == 0x01) {
      Bit32u word1 = ramin_read32(ch->schs[subc].object + 0x4);
      ch->schs[subc].notifier = (word1 & 0xFFFFF) << 4;
      Bit32u word0 = ramin_read32(ch->schs[subc].object);
      Bit8u cls8 = word0;
      if (cls8 == 0x48) {
        // Hack for XFree86 4.3.0
        if (!ch->s2d_locked) {
          Bit32u srcdst = ramin_read32(ch->schs[subc].object + 0x8);
          ch->s2d_img_src = (srcdst & 0xFFFF) << 4;
          ch->s2d_img_dst = srcdst >> 16 << 4;
          ch->s2d_color_fmt = 4;
          ch->s2d_color_bytes = 2;
          ch->s2d_pitch_src = BX_NVRIVA_THIS graph_pitch0 & 0xffff;
          ch->s2d_pitch_dst = ch->s2d_pitch_src;
          ch->s2d_ofs_src = BX_NVRIVA_THIS graph_offset0;
          ch->s2d_ofs_dst = BX_NVRIVA_THIS graph_offset0;
        }
      } else if (cls8 == 0x4a || cls8 == 0x4b) {
        ch->gdi_mono_fmt = (word1 >> 24) & 3;
      } else if (cls8 == 0x42) {
        ch->s2d_img_src =
          ramin_read32(ch->schs[subc].object + 0x8) << 4;
        ch->s2d_img_dst =
          ramin_read32(ch->schs[subc].object + 0xC) << 4;
      } else if (cls8 == 0x60 || cls8 == 0x64) {
        ch->iifc_palette =
          ramin_read32(ch->schs[subc].object + 0x8) << 4;
        Bit32u shift = 19;
        ch->iifc_operation = (word0 >> shift) & 7;
        // should be stored somewhere else
        ch->iifc_color_fmt = ramin_read32(ch->schs[subc].object + 0x10);
        if (ch->iifc_color_fmt == 0)
          ch->iifc_color_fmt = 1;
        update_color_bytes_iifc(ch);
      } else if (cls8 == 0x55)
        execute_d3d(ch, word0 & BX_NVRIVA_THIS class_mask, 0, 0);
    } else if (ch->schs[subc].engine == 0x00) {
      software_method = true;
    } else {
      BX_DEBUG(("execute_command: unknown engine %d", ch->schs[subc].engine));
    }
  } else if (method == 0x014) {
    BX_NVRIVA_THIS fifo_cache1_ref_cnt = param;
  } else if (method == 0x018) {
    Bit32u semaphore_obj;
    ramht_lookup(param, chid, &semaphore_obj, nullptr);
    BX_NVRIVA_THIS fifo_cache1_semaphore = semaphore_obj >> 4;
  } else if (method == 0x019) {
    BX_NVRIVA_THIS fifo_cache1_semaphore &= 0x000FFFFF;
    BX_NVRIVA_THIS fifo_cache1_semaphore |= param << 20;
  } else if (method == 0x01a || method == 0x01b) {
    Bit32u semaphore_obj = (BX_NVRIVA_THIS fifo_cache1_semaphore & 0x000FFFFF) << 4;
    Bit32u semaphore_offset = BX_NVRIVA_THIS fifo_cache1_semaphore >> 20;
    if (method == 0x01a) {
      if (dma_read32(semaphore_obj, semaphore_offset) != param) {
        BX_NVRIVA_THIS acquire_active = true;
        return false;
      }
    } else {
      dma_write32(semaphore_obj, semaphore_offset, param);
    }
  } else if (method >= 0x040) {
    if (ch->schs[subc].engine == 0x01) {
      if (method >= 0x060 && method < 0x080)
        ramht_lookup(param, chid, &param, nullptr);
      Bit32u cls = ramin_read32(
        ch->schs[subc].object) & BX_NVRIVA_THIS class_mask;
      Bit8u cls8 = cls;
      BX_DEBUG(("execute_command: obj 0x%08x, class 0x%04x, method 0x%03x, param 0x%08x",
        ch->schs[subc].object, cls, method, param));
      if (cls8 == 0x19)
        execute_clip(ch, method, param);
      else if (cls8 == 0x39)
        execute_m2mf(ch, subc, method, param);
      else if (cls8 == 0x43)
        execute_rop(ch, method, param);
      else if (cls8 == 0x44 || cls8 == 0x18)
        execute_patt(ch, method, param);
      else if (cls8 == 0x4a || cls8 == 0x4b)
        execute_gdi(ch, method, param);
      else if (cls8 == 0x52)
        execute_swzsurf(ch, method, param);
      else if (cls8 == 0x57)
        execute_chroma(ch, method, param);
      else if (cls8 == 0x5f)
        execute_imageblit(ch, method, param);
      else if (cls8 == 0x61 || cls8 == 0x65 || cls8 == 0x21)
        execute_ifc(ch, method, param);
      else if (cls8 == 0x62)
        execute_surf2d(ch, method, param);
      else if (cls8 == 0x60 || cls8 == 0x64)
        execute_iifc(ch, method, param);
      else if (cls8 == 0x66 || cls8 == 0x76)
        execute_sifc(ch, method, param);
      else if (cls8 == 0x72)
        execute_beta(ch, method, param);
      else if (cls8 == 0x7b)
        execute_tfc(ch, method, param);
      else if (cls8 == 0x89)
        execute_sifm(ch, method, param);
      else if (cls8 == 0x55)
        execute_d3d(ch, cls, method, param);
      if (ch->notify_pending) {
        ch->notify_pending = false;
        if ((ramin_read32(ch->schs[subc].notifier) & 0xFF) == 0x30) {
          BX_DEBUG(("execute_command: DMA notify skipped"));
        } else {
          BX_DEBUG(("execute_command: DMA notify 0x%08x",
            ch->schs[subc].notifier));
          dma_write64(ch->schs[subc].notifier, 0x0, get_current_time());
          dma_write32(ch->schs[subc].notifier, 0x8, 0);
          dma_write32(ch->schs[subc].notifier, 0xC, 0);
        }
        if (ch->notify_type) {
          BX_NVRIVA_THIS graph_intr |= 0x00000001;
          update_irq_level();
          BX_NVRIVA_THIS graph_nsource |= 0x00000001;
          BX_NVRIVA_THIS graph_notify = 0x00110000;
          Bit32u notifier = ch->schs[subc].notifier >> 4;
          BX_NVRIVA_THIS graph_ctx_switch1 = notifier;
          BX_NVRIVA_THIS graph_ctx_switch4 = ch->schs[subc].object >> 4;
          BX_NVRIVA_THIS graph_trapped_addr = (method << 2) | (subc << 16) | (chid << 20);
          BX_NVRIVA_THIS graph_trapped_data = param;
          BX_DEBUG(("execute_command: notify interrupt triggered"));
        }
      }
      if (method == 0x041) {
        ch->notify_pending = true;
        ch->notify_type = param;
      } else if (method == 0x060)
        ch->schs[subc].notifier = param;
    } else if (ch->schs[subc].engine == 0x00) {
      software_method = true;
    } else {
      BX_DEBUG(("execute_command: unknown engine %d", ch->schs[subc].engine));
    }
  }
  if (software_method) {
    BX_NVRIVA_THIS fifo_intr |= 0x00000001;
    update_irq_level();
    BX_NVRIVA_THIS fifo_cache1_pull0 |= 0x00000100;
    BX_NVRIVA_THIS fifo_cache1_method[BX_NVRIVA_THIS fifo_cache1_put / 4] = (method << 2) | (subc << 13);
    BX_NVRIVA_THIS fifo_cache1_data[BX_NVRIVA_THIS fifo_cache1_put / 4] = param;
    BX_NVRIVA_THIS fifo_cache1_put += 4;
    if (BX_NVRIVA_THIS fifo_cache1_put == NVRIVA_CACHE1_SIZE * 4)
      BX_NVRIVA_THIS fifo_cache1_put = 0;
    BX_DEBUG(("execute_command: software method"));
  }
  return true;
}

void bx_nvriva_c::fifo_process(Bit32u chid)
{
  Bit32u oldchid = BX_NVRIVA_THIS fifo_cache1_push1 & 0x1F;
  if (oldchid == chid) {
    if (BX_NVRIVA_THIS fifo_cache1_dma_put == BX_NVRIVA_THIS fifo_cache1_dma_get)
      return;
  } else {
    if (ramfc_read32(chid, 0x0) == ramfc_read32(chid, 0x4))
      return;
  }
  if (oldchid != chid) {
    Bit32u sro = 0x30;
    ramfc_write32(oldchid, 0x0, BX_NVRIVA_THIS fifo_cache1_dma_put);
    ramfc_write32(oldchid, 0x4, BX_NVRIVA_THIS fifo_cache1_dma_get);
    ramfc_write32(oldchid, 0x8, BX_NVRIVA_THIS fifo_cache1_ref_cnt);
    ramfc_write32(oldchid, 0xC, BX_NVRIVA_THIS fifo_cache1_dma_instance);
    ramfc_write32(oldchid, sro, BX_NVRIVA_THIS fifo_cache1_semaphore);
    BX_NVRIVA_THIS fifo_cache1_dma_put = ramfc_read32(chid, 0x0);
    BX_NVRIVA_THIS fifo_cache1_dma_get = ramfc_read32(chid, 0x4);
    BX_NVRIVA_THIS fifo_cache1_ref_cnt = ramfc_read32(chid, 0x8);
    BX_NVRIVA_THIS fifo_cache1_dma_instance = ramfc_read32(chid, 0xC);
    BX_NVRIVA_THIS fifo_cache1_semaphore = ramfc_read32(chid, sro);
    BX_NVRIVA_THIS fifo_cache1_push1 = (BX_NVRIVA_THIS fifo_cache1_push1 & ~0x1F) | chid;
  }
  BX_NVRIVA_THIS fifo_cache1_dma_push |= 0x100;
  nv04_channel* ch = &BX_NVRIVA_THIS chs[chid];
  while (BX_NVRIVA_THIS fifo_cache1_dma_get != BX_NVRIVA_THIS fifo_cache1_dma_put) {
    BX_DEBUG(("fifo: processing at 0x%08x", BX_NVRIVA_THIS fifo_cache1_dma_get));
    Bit32u word = dma_read32(
      BX_NVRIVA_THIS fifo_cache1_dma_instance << 4,
      BX_NVRIVA_THIS fifo_cache1_dma_get);
    BX_NVRIVA_THIS fifo_cache1_dma_get += 4;
    if (ch->dma_state.mcnt) {
      if (!execute_command(chid, ch->dma_state.subc, ch->dma_state.mthd, word)) {
        BX_NVRIVA_THIS fifo_cache1_dma_get -= 4;
        break;
      }
      if (!ch->dma_state.ni)
        ch->dma_state.mthd++;
      ch->dma_state.mcnt--;
    } else {
      if ((word & 0xe0000003) == 0x20000000) {
        // old jump
        BX_NVRIVA_THIS fifo_cache1_dma_get = word & 0x1fffffff;
        BX_DEBUG(("fifo: jump to 0x%08x", BX_NVRIVA_THIS fifo_cache1_dma_get));
      } else if ((word & 3) == 1) {
        // jump
        BX_NVRIVA_THIS fifo_cache1_dma_get = word & 0xfffffffc;
        BX_DEBUG(("fifo: jump to 0x%08x", BX_NVRIVA_THIS fifo_cache1_dma_get));
      } else if ((word & 3) == 2) {
        // call
        if (ch->subr_active)
          BX_PANIC(("fifo: call with subroutine active"));
        ch->subr_return = BX_NVRIVA_THIS fifo_cache1_dma_get;
        ch->subr_active = true;
        BX_NVRIVA_THIS fifo_cache1_dma_get = word & 0xfffffffc;
        BX_DEBUG(("fifo: call 0x%08x", BX_NVRIVA_THIS fifo_cache1_dma_get));
      } else if (word == 0x00020000) {
        // return
        if (!ch->subr_active)
          BX_PANIC(("fifo: return with subroutine inactive"));
        BX_DEBUG(("fifo: return to 0x%08x", ch->subr_return));
        BX_NVRIVA_THIS fifo_cache1_dma_get = ch->subr_return;
        ch->subr_active = false;
      } else if ((word & 0xa0030003) == 0) {
        ch->dma_state.mthd = (word >> 2) & 0x7ff;
        ch->dma_state.subc = (word >> 13) & 7;
        ch->dma_state.mcnt = (word >> 18) & 0x7ff;
        ch->dma_state.ni = word & 0x40000000;
      } else {
        BX_PANIC(("fifo: unexpected word 0x%08x", word));
      }
    }
  }
}

Bit64u bx_nvriva_c::get_current_time()
{
  return (BX_NVRIVA_THIS timer_inittime1 +
    bx_pc_system.time_nsec() - BX_NVRIVA_THIS timer_inittime2) & ~BX_CONST64(0x1F);
}

void bx_nvriva_c::set_irq_level(bool level)
{
  DEV_pci_set_irq(BX_NVRIVA_THIS devfunc, BX_NVRIVA_THIS pci_conf[0x3d], level);
}

Bit32u bx_nvriva_c::get_mc_intr()
{
  Bit32u value = 0x00000000;
  if (BX_NVRIVA_THIS bus_intr & BX_NVRIVA_THIS bus_intr_en)
    value |= 0x10000000;
  if (BX_NVRIVA_THIS fifo_intr & BX_NVRIVA_THIS fifo_intr_en)
    value |= 0x00000100;
  if (BX_NVRIVA_THIS graph_intr & BX_NVRIVA_THIS graph_intr_en)
    value |= 0x00001000;
  if (BX_NVRIVA_THIS crtc_intr & BX_NVRIVA_THIS crtc_intr_en)
    value |= 0x01000000;
  return value;
}

void bx_nvriva_c::update_irq_level()
{
  set_irq_level(get_mc_intr() && BX_NVRIVA_THIS mc_intr_en & 1);
}


Bit32u bx_nvriva_c::register_read32(Bit32u address)
{
  Bit32u value;

  if (address == 0x0) {
    if (BX_NVRIVA_THIS card_type == 0x04)
      value = 0x20044001;
    else
      value = BX_NVRIVA_THIS card_type << 20;
  } else if (address == 0x100) {
    value = get_mc_intr();
  } else if (address == 0x140)
    value = BX_NVRIVA_THIS mc_intr_en;
  else if (address == 0x200)
    value = BX_NVRIVA_THIS mc_enable;
  else if (address == 0x1100)
    value = BX_NVRIVA_THIS bus_intr;
  else if (address == 0x1140)
    value = BX_NVRIVA_THIS bus_intr_en;
  else if (address >= 0x1800 && address < 0x1900) {
    Bit32u offset = address - 0x1800;
    value = 
      (BX_NVRIVA_THIS pci_conf[offset + 0] << 0) |
      (BX_NVRIVA_THIS pci_conf[offset + 1] << 8) |
      (BX_NVRIVA_THIS pci_conf[offset + 2] << 16) |
      (BX_NVRIVA_THIS pci_conf[offset + 3] << 24);
  } else if (address == 0x2100) {
    value = BX_NVRIVA_THIS fifo_intr;
  } else if (address == 0x2140) {
    value = BX_NVRIVA_THIS fifo_intr_en;
  } else if (address == 0x2210) {
    value = BX_NVRIVA_THIS fifo_ramht;
  } else if (address == 0x2214) {
    value = BX_NVRIVA_THIS fifo_ramfc;
  } else if (address == 0x2218) {
    value = BX_NVRIVA_THIS fifo_ramro;
  } else if (address == 0x2400) { // PFIFO_RUNOUT_STATUS
    value = 0x00000010;
    if (BX_NVRIVA_THIS fifo_cache1_get != BX_NVRIVA_THIS fifo_cache1_put)
      value = 0x00000000;
  } else if (address == 0x2504) {
    value = BX_NVRIVA_THIS fifo_mode;
  } else if (address == 0x3204) {
    value = BX_NVRIVA_THIS fifo_cache1_push1;
  } else if (address == 0x3210) {
    value = BX_NVRIVA_THIS fifo_cache1_put;
  } else if (address == 0x3214) { // PFIFO_CACHE1_STATUS
    value = 0x00000010;
    if (BX_NVRIVA_THIS fifo_cache1_get != BX_NVRIVA_THIS fifo_cache1_put)
      value = 0x00000000;
  } else if (address == 0x3220) {
    value = BX_NVRIVA_THIS fifo_cache1_dma_push;
  } else if (address == 0x322c) {
    value = BX_NVRIVA_THIS fifo_cache1_dma_instance;
  } else if (address == 0x3230) { // PFIFO_CACHE1_DMA_CTL
    value = 0x80000000;
  } else if (address == 0x3240) {
    value = BX_NVRIVA_THIS fifo_cache1_dma_put;
  } else if (address == 0x3244) {
    value = BX_NVRIVA_THIS fifo_cache1_dma_get;
  } else if (address == 0x3248) {
    value = BX_NVRIVA_THIS fifo_cache1_ref_cnt;
  } else if (address == 0x3250) {
    if (BX_NVRIVA_THIS fifo_cache1_get != BX_NVRIVA_THIS fifo_cache1_put)
      BX_NVRIVA_THIS fifo_cache1_pull0 |= 0x00000100;
    value = BX_NVRIVA_THIS fifo_cache1_pull0;
  } else if (address == 0x3270) {
    value = BX_NVRIVA_THIS fifo_cache1_get;
  } else if (address == 0x9100) {
    value = BX_NVRIVA_THIS timer_intr;
  } else if (address == 0x9140) {
    value = BX_NVRIVA_THIS timer_intr_en;
  } else if (address == 0x9200)
    value = BX_NVRIVA_THIS timer_num;
  else if (address == 0x9210)
    value = BX_NVRIVA_THIS timer_den;
  else if (address == 0x9400)
    value = (Bit32u)get_current_time();
  else if (address == 0x9410)
    value = get_current_time() >> 32;
  else if (address == 0x9420)
    value = BX_NVRIVA_THIS timer_alarm;
  else if ((address >= 0xc0300 && address < 0xc0400) ||
           (address >= 0xc2300 && address < 0xc2400))
    value = register_read8(address);
  else if (address == 0x100000) {
    value = BX_NVRIVA_THIS s.memsize;
  else if (address == 0x101000)
    value = BX_NVRIVA_THIS straps0_primary;
  else if (address >= 0x300000 && address < 0x310000) {
    Bit32u offset = address - 0x300000;
    if (BX_NVRIVA_THIS pci_conf[0x50] == 0x00) {
      value = 
        (BX_NVRIVA_THIS pci_rom[offset + 0] << 0) |
        (BX_NVRIVA_THIS pci_rom[offset + 1] << 8) |
        (BX_NVRIVA_THIS pci_rom[offset + 2] << 16) |
        (BX_NVRIVA_THIS pci_rom[offset + 3] << 24);
    } else {
      value = 0x00000000;
    }
  } else if (address == 0x400100) {
    value = BX_NVRIVA_THIS graph_intr;
  } else if (address == 0x400108) {
    value = BX_NVRIVA_THIS graph_nsource;
  } else if (address == 0x400140) {
    value = BX_NVRIVA_THIS graph_intr_en;
  } else if (address == 0x40014C) {
    value = BX_NVRIVA_THIS graph_ctx_switch1;
  } else if (address == 0x400150) {
    value = BX_NVRIVA_THIS graph_ctx_switch2;
  } else if (address == 0x400158) {
    value = BX_NVRIVA_THIS graph_ctx_switch4;
  } else if (address == 0x40032c) {
    value = BX_NVRIVA_THIS graph_ctxctl_cur;
  } else if (address == 0x400700) {
    value = BX_NVRIVA_THIS graph_status;
  } else if (address == 0x400704) {
    value = BX_NVRIVA_THIS graph_trapped_addr;
  } else if (address == 0x400708) {
    value = BX_NVRIVA_THIS graph_trapped_data;
  } else if (address == 0x400718) {
    value = BX_NVRIVA_THIS graph_notify;
  } else if (address == 0x400720) {
    value = BX_NVRIVA_THIS graph_fifo;
  } else if (address == 0x400780) {
    value = BX_NVRIVA_THIS graph_channel_ctx_table;
  } else if (address == 0x400820 && BX_NVRIVA_THIS card_type == 0x20) {
    value = BX_NVRIVA_THIS graph_offset0;
  } else if (address == 0x400850 && BX_NVRIVA_THIS card_type == 0x20) {
    value = BX_NVRIVA_THIS graph_pitch0;
  } else if (address == 0x600100) {
    value = BX_NVRIVA_THIS crtc_intr;
  } else if (address == 0x600140) {
    value = BX_NVRIVA_THIS crtc_intr_en;
  } else if (address == 0x600800) {
    value = BX_NVRIVA_THIS crtc_start;
  } else if (address == 0x600804) {
    value = BX_NVRIVA_THIS crtc_config;
  } else if (address == 0x600808) {
    value = VGA_READ(0x03da, 1) << 13; // hack
  } else if (address == 0x60080c) {
    value = BX_NVRIVA_THIS crtc_cursor_offset;
  } else if (address == 0x600810) {
    value = BX_NVRIVA_THIS crtc_cursor_config;
  } else if (address == 0x600868) {
    Bit64u display_usec =
      bx_virt_timer.time_usec(BX_NVRIVA_THIS vsync_realtime) - BX_NVRIVA_THIS s.display_start_usec;
    display_usec = display_usec % BX_NVRIVA_THIS s.vtotal_usec;
    value = (Bit32u)(BX_NVRIVA_THIS get_crtc_vtotal() * display_usec / BX_NVRIVA_THIS s.vtotal_usec);
  } else if ((address >= 0x601300 && address < 0x601400) ||
             (address >= 0x603300 && address < 0x603400)) {
    value = register_read8(address);
  } else if (address == 0x680300) {
    value = BX_NVRIVA_THIS ramdac_cu_start_pos;
  } else if (address == 0x680404) { // RAMDAC_NV10_CURSYNC
    value = 0x00000000;
  } else if (address == 0x680508) {
    value = BX_NVRIVA_THIS ramdac_vpll;
  } else if (address == 0x68050c) {
    value = BX_NVRIVA_THIS ramdac_pll_select;
  } else if (address == 0x680578) {
    value = BX_NVRIVA_THIS ramdac_vpll_b;
  } else if (address == 0x680600) {
    value = BX_NVRIVA_THIS ramdac_general_control;
  } else if (address == 0x680828) { // PRAMDAC_FP_HCRTC
    value = 0x00000000; // Second monitor is disconnected
  } else if ((address >= 0x681300 && address < 0x681400) ||
             (address >= 0x683300 && address < 0x683400)) {
    value = register_read8(address);
  } else if (address >= 0x700000 && address < 0x800000) {
    Bit32u offset = address & 0x000fffff;
    if (offset & 3) {
      value =
        ramin_read8(offset + 0) << 0 |
        ramin_read8(offset + 1) << 8 |
        ramin_read8(offset + 2) << 16 |
        ramin_read8(offset + 3) << 24;
    } else {
      value = ramin_read32(offset);
    }
  } else if ((address >= 0x800000 && address < 0xA00000) ||
             (address >= 0xC00000 && address < 0xE00000)) {
    Bit32u chid;
    Bit32u offset;
    if (address >= 0x800000 && address < 0xA00000) {
      chid = (address >> 16) & 0x1F;
      offset = address & 0x1FFF;
    } else {
      chid = (address >> 12) & 0x1FF;
      if (chid >= NVRIVA_CHANNEL_COUNT) {
        BX_PANIC(("Channel id >= 32"));
        chid = 0;
      }
      offset = address & 0x1FF;
    }
    value = 0x00000000;
    Bit32u curchid = BX_NVRIVA_THIS fifo_cache1_push1 & 0x1F;
    if (offset == 0x54 && address >= 0xC00000 && address < 0xE00000) {
      if (BX_NVRIVA_THIS chs[chid].subr_active)
        value = BX_NVRIVA_THIS chs[chid].subr_return;
      else if (curchid == chid)
        value = BX_NVRIVA_THIS fifo_cache1_dma_get;
      else
        value = ramfc_read32(chid, 0x4);
    } else if (offset == 0x10) {
      value = 0xffff;
    } else if (offset >= 0x40 && offset <= 0x48) {
      if (curchid == chid) {
        if (offset == 0x40)
          value = BX_NVRIVA_THIS fifo_cache1_dma_put;
        else if (offset == 0x44)
          value = BX_NVRIVA_THIS fifo_cache1_dma_get;
        else if (offset == 0x48)
          value = BX_NVRIVA_THIS fifo_cache1_ref_cnt;
      } else {
        if (offset == 0x40)
          value = ramfc_read32(chid, 0x0);
        else if (offset == 0x44)
          value = ramfc_read32(chid, 0x4);
        else if (offset == 0x48)
          value = ramfc_read32(chid, 0x8);
      }
    } else {
      BX_ERROR(("Unknown FIFO offset 0x%08x", offset));
    }
  } else {
    value = BX_NVRIVA_THIS unk_regs[address / 4];
  }
  return value;
}

void bx_nvriva_c::register_write32(Bit32u address, Bit32u value)
{
  if (address == 0x140) {
    BX_NVRIVA_THIS mc_intr_en = value;
    update_irq_level();
  } else if (address == 0x200) {
    BX_NVRIVA_THIS mc_enable = value;
  } else if (address >= 0x1800 && address < 0x1900) {
    BX_NVRIVA_THIS pci_write_handler(address - 0x1800, value, 4);
  } else if (address == 0x1100) {
    BX_NVRIVA_THIS bus_intr &= ~value;
    update_irq_level();
  } else if (address == 0x1140) {
    BX_NVRIVA_THIS bus_intr_en = value;
    update_irq_level();
  } else if (address == 0x2100) {
    BX_NVRIVA_THIS fifo_intr &= ~value;
    update_irq_level();
  } else if (address == 0x2140) {
    BX_NVRIVA_THIS fifo_intr_en = value;
    update_irq_level();
  } else if (address == 0x2210) {
    BX_NVRIVA_THIS fifo_ramht = value;
  } else if (address == 0x2214) {
    BX_NVRIVA_THIS fifo_ramfc = value;
  } else if (address == 0x2218) {
    BX_NVRIVA_THIS fifo_ramro = value;
  } else if (address == 0x2504) {
    BX_NVRIVA_THIS fifo_mode = value;
  } else if (address == 0x3204) {
    BX_NVRIVA_THIS fifo_cache1_push1 = value;
  } else if (address == 0x3210) {
    BX_NVRIVA_THIS fifo_cache1_put = value;
  } else if (address == 0x3220) {
    BX_NVRIVA_THIS fifo_cache1_dma_push = value;
  } else if (address == 0x322c) {
    BX_NVRIVA_THIS fifo_cache1_dma_instance = value;
  } else if (address == 0x3240) {
    BX_NVRIVA_THIS fifo_cache1_dma_put = value;
  } else if (address == 0x3244) {
    BX_NVRIVA_THIS fifo_cache1_dma_get = value;
  } else if (address == 0x3248) {
    BX_NVRIVA_THIS fifo_cache1_ref_cnt = value;
  } else if (address == 0x3250) {
    BX_NVRIVA_THIS fifo_cache1_pull0 = value;
  } else if (address == 0x3270) {
    BX_NVRIVA_THIS fifo_cache1_get = value & (NVRIVA_CACHE1_SIZE * 4 - 1);
    if (BX_NVRIVA_THIS fifo_cache1_get != BX_NVRIVA_THIS fifo_cache1_put) {
      BX_NVRIVA_THIS fifo_intr |= 0x00000001;
    } else {
      BX_NVRIVA_THIS fifo_intr &= ~0x00000001;
      BX_NVRIVA_THIS fifo_cache1_pull0 &= ~0x00000100;
    }
    update_irq_level();
  } else if (address == 0x9100) {
    BX_NVRIVA_THIS timer_intr &= ~value;
  } else if (address == 0x9140) {
    BX_NVRIVA_THIS timer_intr_en = value;
  } else if (address == 0x9200) {
    BX_NVRIVA_THIS timer_num = value;
  } else if (address == 0x9210) {
    BX_NVRIVA_THIS timer_den = value;
  } else if (address == 0x9400 || address == 0x9410) {
    BX_NVRIVA_THIS timer_inittime2 = bx_pc_system.time_nsec();
    if (address == 0x9400) {
      BX_NVRIVA_THIS timer_inittime1 = 
        (BX_NVRIVA_THIS timer_inittime1 & BX_CONST64(0xFFFFFFFF00000000)) | value;
    } else {
      BX_NVRIVA_THIS timer_inittime1 =
        (BX_NVRIVA_THIS timer_inittime1 & BX_CONST64(0x00000000FFFFFFFF)) | ((Bit64u)value << 32);
    }
  } else if (address == 0x9420) {
    BX_NVRIVA_THIS timer_alarm = value;
  } else if ((address >= 0xc0300 && address < 0xc0400) ||
             (address >= 0xc2300 && address < 0xc2400)) {
    register_write8(address, value);
  } else if (address == 0x101000) {
    if (value >> 31)
      BX_NVRIVA_THIS straps0_primary = value;
    else
      BX_NVRIVA_THIS straps0_primary = BX_NVRIVA_THIS straps0_primary_original;
  } else if (address == 0x400100) {
    BX_NVRIVA_THIS graph_intr &= ~value;
    update_irq_level();
  } else if (address == 0x400108) {
    BX_NVRIVA_THIS graph_nsource = value;
  } else if (address == 0x400140) {
    BX_NVRIVA_THIS graph_intr_en = value;
    update_irq_level();
  } else if (address == 0x40014C) {
    BX_NVRIVA_THIS graph_ctx_switch1 = value;
  } else if (address == 0x400150) {
    BX_NVRIVA_THIS graph_ctx_switch2 = value;
  } else if (address == 0x400158) {
    BX_NVRIVA_THIS graph_ctx_switch4 = value;
  } else if (address == 0x40032c) {
    BX_NVRIVA_THIS graph_ctxctl_cur = value;
  } else if (address == 0x400700) {
    BX_NVRIVA_THIS graph_status = value;
  } else if (address == 0x400704) {
    BX_NVRIVA_THIS graph_trapped_addr = value;
  } else if (address == 0x400708) {
    BX_NVRIVA_THIS graph_trapped_data = value;
  } else if (address == 0x400718) {
    BX_NVRIVA_THIS graph_notify = value;
  } else if (address == 0x400720) {
    BX_NVRIVA_THIS graph_fifo = value;
  } else if (address == 0x400780) {
    BX_NVRIVA_THIS graph_channel_ctx_table = value;
  } else if (address == 0x400820) {
    BX_NVRIVA_THIS graph_offset0 = value;
  } else if (address == 0x600100) {
    BX_NVRIVA_THIS crtc_intr &= ~value;
    update_irq_level();
  } else if (address == 0x600140) {
    BX_NVRIVA_THIS crtc_intr_en = value;
    update_irq_level();
  } else if (address == 0x600800) {
    BX_NVRIVA_THIS crtc_start = value;
    BX_NVRIVA_THIS svga_needs_update_mode = 1;
  } else if (address == 0x600804) {
    BX_NVRIVA_THIS crtc_config = value;
  } else if (address == 0x60080c) {
    BX_NVRIVA_THIS crtc_cursor_offset = value;
    BX_NVRIVA_THIS hw_cursor.offset = BX_NVRIVA_THIS crtc_cursor_offset;
  } else if (address == 0x600810) {
    BX_NVRIVA_THIS crtc_cursor_config = value;
    BX_NVRIVA_THIS hw_cursor.enabled =
      (BX_NVRIVA_THIS crtc.reg[0x31] & 0x01) || value & 0x00000001;
    BX_NVRIVA_THIS hw_cursor.size = value & 0x00010000 ? 64 : 32;
    BX_NVRIVA_THIS hw_cursor.bpp32 = value & 0x00001000;
  } else if ((address >= 0x601300 && address < 0x601400) ||
             (address >= 0x603300 && address < 0x603400)) {
    register_write8(address, value);
  } else if (address == 0x680300) {
    Bit16s prevx = BX_NVRIVA_THIS hw_cursor.x;
    Bit16s prevy = BX_NVRIVA_THIS hw_cursor.y;
    BX_NVRIVA_THIS ramdac_cu_start_pos = value;
    BX_NVRIVA_THIS hw_cursor.x = (Bit32s)BX_NVRIVA_THIS ramdac_cu_start_pos << 20 >> 20;
    BX_NVRIVA_THIS hw_cursor.y = (Bit32s)BX_NVRIVA_THIS ramdac_cu_start_pos << 4 >> 20;
    if (BX_NVRIVA_THIS hw_cursor.size != 0) {
      BX_NVRIVA_THIS redraw_area_nd(prevx, prevy,
        BX_NVRIVA_THIS hw_cursor.size, BX_NVRIVA_THIS hw_cursor.size);
      BX_NVRIVA_THIS redraw_area_nd(BX_NVRIVA_THIS hw_cursor.x, BX_NVRIVA_THIS hw_cursor.y,
        BX_NVRIVA_THIS hw_cursor.size, BX_NVRIVA_THIS hw_cursor.size);
    }
  } else if (address == 0x680508) {
    BX_NVRIVA_THIS ramdac_vpll = value;
    BX_NVRIVA_THIS calculate_retrace_timing();
  } else if (address == 0x68050c) {
    BX_NVRIVA_THIS ramdac_pll_select = value;
    BX_NVRIVA_THIS calculate_retrace_timing();
  } else if (address == 0x680578) {
    BX_NVRIVA_THIS ramdac_vpll_b = value;
    BX_NVRIVA_THIS calculate_retrace_timing();
  } else if (address == 0x680600) {
    BX_NVRIVA_THIS ramdac_general_control = value;
    BX_NVRIVA_THIS s.dac_shift = (value >> 20) & 1 ? 0 : 2;
  } else if ((address >= 0x681300 && address < 0x681400) ||
             (address >= 0x683300 && address < 0x683400)) {
    register_write8(address, value);
  } else if (address >= 0x700000 && address < 0x800000) {
    ramin_write32(address - 0x700000, value);
  } else if ((address >= 0x800000 && address < 0xA00000) ||
             (address >= 0xC00000 && address < 0xE00000)) {
    Bit32u chid;
    Bit32u offset;
    if (address >= 0x800000 && address < 0xA00000) {
      chid = (address >> 16) & 0x1F;
      offset = address & 0x1FFF;
    } else {
      chid = (address >> 12) & 0x1FF;
      if (chid >= NVRIVA_CHANNEL_COUNT) {
        BX_PANIC(("Channel id >= 32"));
        chid = 0;
      }
      offset = address & 0x1FF;
    }
    if ((BX_NVRIVA_THIS fifo_mode & (1 << chid)) != 0) {
      if (offset == 0x40) {
        Bit32u curchid = BX_NVRIVA_THIS fifo_cache1_push1 & 0x1F;
        if (curchid == chid)
          BX_NVRIVA_THIS fifo_cache1_dma_put = value;
        else
          ramfc_write32(chid, 0x0, value);
        fifo_process(chid);
      }
    } else if (address >= 0x800000 && address < 0xA00000) {
      Bit32u subc = (address >> 13) & 7;
      execute_command(chid, subc, offset / 4, value);
    }
  } else {
    BX_NVRIVA_THIS unk_regs[address / 4] = value;
  }
}

void bx_nvriva_c::svga_init_pcihandlers(void)
{
  BX_NVRIVA_THIS devfunc = 0x00;
  DEV_register_pci_handlers2(BX_NVRIVA_THIS_PTR,
      &BX_NVRIVA_THIS devfunc, BX_PLUGIN_NVRIVA, "NVidia RIVA AGP", true);
  Bit16u devid = 0x0000;
  Bit8u revid = 0x00;
  if (BX_NVRIVA_THIS card_type == 0x04) {
    devid = 0x0020;
    revid = 0x04;
  } else if (BX_NVRIVA_THIS card_type == 0x05) {
    devid = 0x002D;
    revid = 0x15;
  }
  BX_NVRIVA_THIS init_pci_conf(0x10DE, devid, revid, 0x030000, 0x00, BX_PCI_INTA);

  BX_NVRIVA_THIS init_bar_mem(0, NVRIVA_PNPMMIO_SIZE, nvriva_mem_read_handler,
                               nvriva_mem_write_handler);
  BX_NVRIVA_THIS pci_conf[0x14] = 0x08;
  BX_NVRIVA_THIS init_bar_mem(1, BX_NVRIVA_THIS s.memsize, nvriva_mem_read_handler,
                               nvriva_mem_write_handler);
  BX_NVRIVA_THIS pci_rom_address = 0;
  BX_NVRIVA_THIS pci_rom_read_handler = nvriva_mem_read_handler;
  BX_NVRIVA_THIS load_pci_rom(SIM->get_param_string(BXPN_VGA_ROM_PATH)->getptr());

  BX_NVRIVA_THIS pci_conf[0x2c] = 0x43;
  BX_NVRIVA_THIS pci_conf[0x2d] = 0x10;
  if (BX_NVRIVA_THIS card_type == 0x04) {
    BX_NVRIVA_THIS pci_conf[0x2e] = 0x00;
    BX_NVRIVA_THIS pci_conf[0x2f] = 0x02;
  } else if (BX_NVRIVA_THIS card_type == 0x05) {
    BX_NVRIVA_THIS pci_conf[0x2e] = 0x03;
    BX_NVRIVA_THIS pci_conf[0x2f] = 0x40;
  }
  BX_NVRIVA_THIS pci_conf[0x40] = BX_NVRIVA_THIS pci_conf[0x2c];
  BX_NVRIVA_THIS pci_conf[0x41] = BX_NVRIVA_THIS pci_conf[0x2d];
  BX_NVRIVA_THIS pci_conf[0x42] = BX_NVRIVA_THIS pci_conf[0x2e];
  BX_NVRIVA_THIS pci_conf[0x43] = BX_NVRIVA_THIS pci_conf[0x2f];

  BX_NVRIVA_THIS pci_conf[0x06] = 0xB0;
  BX_NVRIVA_THIS pci_conf[0x07] = 0x02;
  BX_NVRIVA_THIS pci_conf[0x34] = 0x60;
  BX_NVRIVA_THIS pci_conf[0x44] = 0x02;
  BX_NVRIVA_THIS pci_conf[0x45] = 0x00;
  if (BX_NVRIVA_THIS card_type == 0x05) {
    BX_NVRIVA_THIS pci_conf[0x46] = 0x20;
  } else {
    BX_NVRIVA_THIS pci_conf[0x46] = 0x10;
  }
  BX_NVRIVA_THIS pci_conf[0x47] = 0x00;
  BX_NVRIVA_THIS pci_conf[0x48] = 0x07;
  BX_NVRIVA_THIS pci_conf[0x49] = 0x00;
  BX_NVRIVA_THIS pci_conf[0x4a] = 0x00;
  BX_NVRIVA_THIS pci_conf[0x4b] = 0x1F;
  BX_NVRIVA_THIS pci_conf[0x54] = 0x01;
  BX_NVRIVA_THIS pci_conf[0x55] = 0x00;
  BX_NVRIVA_THIS pci_conf[0x56] = 0x00;
  BX_NVRIVA_THIS pci_conf[0x57] = 0x00;
  BX_NVRIVA_THIS pci_conf[0x60] = 0x01;
  BX_NVRIVA_THIS pci_conf[0x61] = 0x44;
  BX_NVRIVA_THIS pci_conf[0x62] = 0x01;
  BX_NVRIVA_THIS pci_conf[0x63] = 0x00;
}

void bx_nvriva_c::pci_write_handler(Bit8u address, Bit32u value, unsigned io_len)
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
void bx_nvriva_c::debug_dump(int argc, char **argv)
{
  bx_vgacore_c::debug_dump(argc, argv);
}
#endif

#endif // BX_SUPPORT_NVRIVA
