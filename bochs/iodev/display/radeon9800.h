/////////////////////////////////////////////////////////////////////////
// $Id$
/////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025-2026  The Bochs Project
//
//  ATI Radeon 9800 (R350) emulation
//
//  The device model follows the structure of the Bochs Rage 128 PRO
//  emulation: a VGA-compatible core with the extended CRTC / PLL / DAC
//  display block (two CRTCs), a synchronous 2D GUI engine, the PM4
//  command processor (CP) whose packets execute on a worker thread, a
//  software R300 3D pipeline (programmable vertex shader, rasterizer,
//  programmable fragment shader, texture unit, fog, Z/stencil, colour
//  buffer) with optional scanline-interleaved render workers, and the
//  OV0 video overlay.
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

#ifndef BX_IODEV_RADEON9800_H
#define BX_IODEV_RADEON9800_H

#if BX_SUPPORT_RADEON9800

#include "bxthread.h"

// The engines run on worker threads and every module needs the object
// pointer: the "static member functions" trick is never used.
#define BX_R9800_THIS this->
#define BX_R9800_THIS_PTR this

#define R9800_VRAM_MAX (256u * 1024u * 1024u)

// CP FIFO between the CPU-thread ring pump and the executor thread:
// 2^19 dwords = 2 MB.
#define R9800_CP_FIFO_DWORDS (1u << 19)
#define R9800_CP_FIFO_MASK   (R9800_CP_FIFO_DWORDS - 1)
#define R9800_CP_TAG_IB      1
#define R9800_PM4_MAX_PAYLOAD 16384

#define R9800_RASTER_MAX_WORKERS 8
// Register writes kept for the trace's poll report
#define R9800_TRACE_WRITES 96
// Identical consecutive reads of one register before it is called a spin
#define R9800_TRACE_POLL_RUN 200000
// Register accesses confined to a few registers before it is called a stall
#define R9800_TRACE_BLOCK 400000
// Distinct registers a stalled loop may touch before it looks like real work
#define R9800_TRACE_BLOCK_REGS 32
// Display frames between heartbeat state dumps (trace bit 3)
#define R9800_TRACE_HB_FRAMES 120

// 14-bit signed coordinate field (S.14.0)
static inline int r9800_sx14(Bit32u v)
{
  return ((Bit32s)((v & 0x3fff) << 18)) >> 18;
}

static inline float r9800_u2f(Bit32u u)
{
  float f;
  memcpy(&f, &u, 4);
  return f;
}

static inline Bit32u r9800_f2u(float f)
{
  Bit32u u;
  memcpy(&u, &f, 4);
  return u;
}

// ---------------------------------------------------------------------
// R300 surface tiling. Micro tiles are 32 bytes, macro tiles 2 KB; the
// pixel geometry of both depends on the pixel size (Mesa r300 texture
// descriptor tables). bpp = bytes per pixel (1, 2, 4, 8, 16); micro = 0
// linear, 1 tiled, 2 square (16-bit only); macro = 0/1.
// ---------------------------------------------------------------------
static inline void r300_micro_geom(Bit32u bpp, Bit32u micro, Bit32u *tw, Bit32u *th)
{
  if (micro == 2) { *tw = 4; *th = 4; return; }   // 16 bpp square: 8 B x 4
  switch (bpp) {
    case 1:  *tw = 8; *th = 4; break;
    case 2:  *tw = 8; *th = 2; break;
    case 4:  *tw = 4; *th = 2; break;
    case 8:  *tw = 2; *th = 2; break;
    default: *tw = 1; *th = 2; break;
  }
}

static inline void r300_macro_geom(Bit32u bpp, Bit32u micro, Bit32u *mw, Bit32u *mh)
{
  if (micro == 2) { *mw = 32; *mh = 32; return; }
  if (micro == 0) {
    switch (bpp) {
      case 1:  *mw = 256; *mh = 8; break;
      case 2:  *mw = 128; *mh = 8; break;
      case 4:  *mw = 64;  *mh = 8; break;
      case 8:  *mw = 32;  *mh = 8; break;
      default: *mw = 16;  *mh = 8; break;
    }
  } else {
    switch (bpp) {
      case 1:  *mw = 64; *mh = 32; break;
      case 2:  *mw = 64; *mh = 16; break;
      case 4:  *mw = 32; *mh = 16; break;
      case 8:  *mw = 16; *mh = 16; break;
      default: *mw = 8;  *mh = 16; break;
    }
  }
}

// Byte offset of pixel (x, y) in a surface with the given tiling
static inline Bit32u r300_tile_addr(Bit32u x, Bit32u y, Bit32u pitch_b, Bit32u bpp, Bit32u micro, Bit32u macro)
{
  Bit32u tw, th, intra, off;
  if (!micro && !macro)
    return y * pitch_b + x * bpp;
  if (!macro) {
    r300_micro_geom(bpp, micro, &tw, &th);
    Bit32u tiles_per_row = pitch_b / (tw * bpp);
    if (!tiles_per_row) tiles_per_row = 1;
    intra = (y % th) * (tw * bpp) + (x % tw) * bpp;
    return ((y / th) * tiles_per_row + x / tw) * 32u + intra;
  }
  Bit32u mw, mh;
  r300_macro_geom(bpp, micro, &mw, &mh);
  Bit32u macros_per_row = pitch_b / (mw * bpp);
  if (!macros_per_row) macros_per_row = 1;
  off = ((y / mh) * macros_per_row + x / mw) * 2048u;
  Bit32u mx = x % mw, my = y % mh;
  if (!micro)
    return off + my * (mw * bpp) + mx * bpp;
  r300_micro_geom(bpp, micro, &tw, &th);
  Bit32u mt_cols = mw / tw;
  intra = ((my / th) * mt_cols + mx / tw) * 32u + (my % th) * (tw * bpp) + (mx % tw) * bpp;
  return off + intra;
}

// Height of a tile row (rows that share one contiguous byte range)
static inline Bit32u r300_tile_rows(Bit32u bpp, Bit32u micro, Bit32u macro)
{
  Bit32u tw, th;
  if (macro) { r300_macro_geom(bpp, micro, &tw, &th); return th; }
  if (micro) { r300_micro_geom(bpp, micro, &tw, &th); return th; }
  return 1;
}

// ---------------------------------------------------------------------
// 3D pipeline data
// ---------------------------------------------------------------------

// One vertex as handed from the VAP to the rasterizer (window space;
// pos[3] holds 1/W). Colours are the present colours compacted in order,
// tc[] the texture coordinate components packed as GB_VAP_RASTER_VTX_FMT_1
// describes them.
typedef struct {
  float pos[4];
  float col[4][4];
  float tc[32];
  float psize;
} r300_vtx_t;

// One decoded fragment ALU instruction half (RGB or alpha)
typedef struct {
  Bit8u src[3];     // 0..31 pixel stack frame, 32..63 constant
  Bit8u dst;
  Bit8u wmask;      // RGB: bits R,G,B ; alpha: bit 0
  Bit8u omask;
  Bit8u target;
  Bit8u sel[3];
  Bit8u mod[3];
  Bit8u srcp_op;
  Bit8u op;
  Bit8u omod;
  Bit8u clamp;
} r300_alu_half_t;

typedef struct {
  r300_alu_half_t rgb;
  r300_alu_half_t alpha;
  Bit8u alpha_w_out;   // OMASK_W: alpha result to the depth output
} r300_alu_inst_t;

typedef struct {
  Bit8u src, dst, tex_id, op;
} r300_tex_inst_t;

typedef struct {
  int nodes;
  struct {
    int alu_start, alu_cnt, tex_start, tex_cnt, rgba_out, w_out;
  } node[4];
  int first_tex;
  int pixsize;
  int w_fmt, w_src;
  int out_fmt[4];
  int out_sel[4][4];
  int out_sign[4];
  r300_alu_inst_t alu[R9800_US_ALU_INSTS];
  r300_tex_inst_t tex[R9800_US_TEX_INSTS];
  float konst[R9800_US_CONSTS][4];
} r300_fp_t;

#define R300_TEX_LEVELS 13
#define R9800_TEX_STAGE_NONE 0xffffffffu

// Resolved sampler state of one texture map
typedef struct {
  bool   en;
  int    fmt;
  int    w, h, depth_log2;
  int    levels;          // number of mip levels
  int    max_mip;         // finest level to use
  int    coord_type;      // 0 = 2D, 1 = 3D, 2 = cube
  int    projected;
  int    clamp_s, clamp_t, clamp_r;
  int    mag, min, mip, vol;
  float  lod_bias;
  int    yuv, swap_yuv, gamma;
  int    sel[4];          // swizzle: A, R, G, B
  int    signed_comp;
  int    ck_mode;
  Bit32u chroma, border;
  int    micro, macro, endian;
  Bit32u offset;          // MC address of the base map
  Bit32u pitch_px;        // level 0 pitch in texels
  int    pitch_en;
  int    bpp;             // bytes per texel (0 = block compressed)
  int    cmp;             // 0 none, 1 DXT1, 2 DXT3, 3 DXT5, 4 ATI2N
  Bit32u lvl_off[R300_TEX_LEVELS];
  Bit32u lvl_pitch[R300_TEX_LEVELS];   // bytes per row (of texels or blocks)
  Bit32u lvl_w[R300_TEX_LEVELS], lvl_h[R300_TEX_LEVELS], lvl_d[R300_TEX_LEVELS];
  Bit32u lvl_layer[R300_TEX_LEVELS];   // bytes per layer / face
  Bit32u total;           // total bytes of the whole map
  Bit32u stage_off;       // staging arena offset or R9800_TEX_STAGE_NONE
  Bit32u vram_off;        // VRAM offset when resident
} r300_tex_desc_t;

// Flat per-draw state: every register field the rasterizer consumes,
// cracked once per draw.
typedef struct {
  int    draw_ok;
  // scissor / clip rectangles (pixel coordinates)
  int    sx0, sy0, sx1, sy1;
  int    clip_rule;
  int    cx0[4], cy0[4], cx1[4], cy1[4];
  int    sub;
  float  subf;
  int    geom_round;
  // shading
  int    rgb_shade[4], a_shade[4];
  int    provoke;
  float  solid[4];
  // points / lines
  float  point_hw, point_hh;
  float  point_min, point_max;
  int    point_stuff, line_stuff, tri_stuff;
  int    tex_src[8];
  float  ps0, pt0, ps1, pt1;
  float  ls0, ls1;
  float  line_hw;
  int    line_end;
  // culling / polygon mode
  int    cull_front, cull_back, face_cw;
  int    poly_mode, front_ptype, back_ptype;
  int    poff_front_en, poff_back_en;
  float  poff_fscale, poff_foff, poff_bscale, poff_boff;
  float  depth_scale, depth_off;
  Bit32u tex_wrap;
  // fog
  int    fog_en, fog_fn, fog_sel, depth_sel, w_sel;
  float  fog_scale, fog_off, fog_factor;
  float  fog_col[3];
  // alpha test / dither
  int    af_en, af_fn;
  float  af_val;
  int    dith_en;
  int    depth_src;
  // colour buffer
  int    blend_en, sep_alpha;
  int    cfn, csrc, cdst, afn, asrc, adst;
  int    discard_src;
  float  const_col[4];
  Bit32u chan_mask;
  int    rop_en, rop;
  int    clrcmp_en;
  Bit32u clrcmp_clr, clrcmp_msk, clrcmp_flipe;
  int    dither_mode, adither_mode;
  int    aa_en, aa_samples;
  int    aa_resolve, aar_gamma;
  Bit32u aar_off, aar_pitch_px;
  Bit32u aar_vram;          // VRAM offset of the resolve target (0xffffffff = none)
  int    multiwrite;
  int    cb_fmt, cb_bpp, cb_micro, cb_macro;
  Bit32u cb_pitch_px;
  Bit32u cb_off[4];
  Bit32u cb_vram[4];
  bool   cb_ok;
  // depth / stencil
  int    z_en, z_wr, z_fn, z_signed;
  int    sten_en, sten_fb;
  int    sfn, sfail, szpass, szfail;
  int    sfn_bf, sfail_bf, szpass_bf, szfail_bf;
  Bit32u sref, smask, swmask;
  int    zb_fmt, zb_bpp, zb_micro, zb_macro;
  Bit32u zb_pitch_px;
  Bit32u zb_off, zb_vram;
  bool   zb_ok;
  int    ztop;
  int    fast_fill, rd_comp, wr_comp, hiz_en;
  Bit32u z_clear;
  int    zx_off, zy_off;
  Bit32u zmask_off, zmask_pitch;
  // rasterizer routing
  int    rs_count;
  int    rs_w_en;
  int    it_count, ic_count;
  struct { int tex_id, tex_cn, tex_addr, col_id, col_cn, col_addr; } rs_inst[16];
  struct { int tex_ptr, col_ptr, col_fmt, sel[4]; } rs_ip[8];
  int    n_col, n_tc;
  Bit32u raster_fmt1;       // GB_VAP_RASTER_VTX_FMT_1 (tc[] packing)
  // texture enables
  Bit32u tx_enable;
} r300_draw_state_t;

// Per-draw rasterizer state snapshot: a primitive is rasterized under the
// engine state latched when it was submitted.
typedef struct {
  Bit32u serial;
  Bit32u r[R9800_R3D_IMAGE_DWORDS];
  r300_fp_t fp;
  r300_tex_desc_t tex[R9800_TX_MAPS];
  r300_draw_state_t d;
  int    stage_dead;
  Bit32u tex_lo, tex_hi;     // VRAM byte span sampled by enabled textures
} r9800_raster_state_t;

// One fragment as handed from the scan converter to the pixel pipeline
typedef struct {
  float  temps[32][4];       // pixel stack frame as loaded by the RS
  float  ddx[32][4];         // screen-space derivatives of the same
  float  ddy[32][4];
  Bit32u written;            // temporaries the RS wrote
  float  z;                  // depth (0..1 window space)
  float  dzdx, dzdy;         // depth slopes (polygon offset)
  float  w;                  // clip-space W
  float  fog_v;              // fog source value (vertex fog)
  Bit32u cov;                // sample coverage bit mask
  int    front;
} r300_frag_t;

// One deferred primitive in the parallel raster batch
#define RB_TRI   0
#define RB_LINE  1
#define RB_POINT 2

typedef struct {
  Bit32u    state_idx;
  Bit32u    kind;
  Bit32s    py0, py1;
  int       front;
  r300_vtx_t v[3];
} rb_cmd_t;

class bx_radeon9800_c;

typedef struct {
  bx_radeon9800_c *dev;
  int  id;
  int  mask;
  BX_THREAD_VAR(thread);
  bx_thread_sem_t wake;
  bx_thread_sem_t done;
} rb_worker_t;

// Staging of a system-memory resident surface span
typedef struct {
  Bit8u *arena;
  Bit32u cap;
  Bit32u vm;
  Bit32u len;
  bool   active;
} r9800_span_stage_t;

// Surface window for the synchronous 2D executors: local VRAM or a staged
// copy of a system-memory span.
typedef struct {
  Bit8u *base;
  Bit32u rel;
  Bit32u mask;
  r9800_span_stage_t *st;
} r9800_surf_t;

static inline Bit8u *r9800_surf_at(const r9800_surf_t *s, Bit32u card_addr)
{
  return &s->base[(card_addr - s->rel) & s->mask];
}

static inline Bit8u *r9800_surf_run(const r9800_surf_t *s, Bit32u card_addr, Bit32u len)
{
  Bit32u idx = (card_addr - s->rel) & s->mask;
  if ((Bit64u)idx + len > (Bit64u)s->mask + 1u)
    return NULL;
  return &s->base[idx];
}

// Colour-compare predicate state (2D)
struct r9800_ccmp {
  int    src_on, dst_on;
  int    fn_src, fn_dst;
  Bit32u key_src, key_dst;
  Bit32u smask, dmask;
  Bit32u flip;
};

Bit32u radeon9800_rop3(Bit8u rop, Bit32u p, Bit32u s, Bit32u d);
bool radeon9800_aux_sc_pass_2d(Bit32u cntl, const Bit32u rect[3][4], int x, int y);

// The VAP input vertex: 16 vectors of 4 floats
typedef struct {
  float v[16][4];
} r300_vap_in_t;

// Vertex after the vertex shader, before the viewport transform
typedef struct {
  float pos[4];
  float col[4][4];
  float tex[8][4];
  float psize;
  int   col_present, tex_present;
} r300_vs_out_t;

// ---------------------------------------------------------------------
// Secondary PCI function (1002:4e68, class 0380): exposes the frame
// buffer aperture and the register file a second time.
// ---------------------------------------------------------------------
class bx_radeon9800_sec_c : public bx_pci_device_c {
public:
  bx_radeon9800_sec_c(bx_radeon9800_c *primary);
  virtual ~bx_radeon9800_sec_c();
  void init_pci(bool is_agp);
  void reset_pci(void);
  virtual void pci_write_handler(Bit8u address, Bit32u value, unsigned io_len);
  void register_sec_state(bx_list_c *parent);
  void after_restore(void);
  Bit32u bar0_addr(void) { return pci_bar[0].addr; }
  static bool mem_read_handler(bx_phy_address addr, unsigned len, void *data, void *param);
  static bool mem_write_handler(bx_phy_address addr, unsigned len, void *data, void *param);
  Bit8u devfunc;
  bx_radeon9800_c *dev;
};

class bx_radeon9800_c : public bx_vgacore_c {
public:
  bx_radeon9800_c();
  virtual ~bx_radeon9800_c();

  virtual bool init_vga_extension(void);
  virtual void get_crtc_params(bx_crtc_params_t *crtcp, Bit32u *vclock);
  virtual void reset(unsigned type);
  virtual void redraw_area(unsigned x0, unsigned y0, unsigned width, unsigned height);
  virtual Bit8u mem_read(bx_phy_address addr);
  virtual void mem_write(bx_phy_address addr, Bit8u value);
  virtual void get_text_snapshot(Bit8u **text_snapshot, unsigned *txHeight, unsigned *txWidth);
  virtual void register_state(void);
  virtual void after_restore_state(void);
  virtual void vertical_timer(void);
#if BX_SUPPORT_PCI
  virtual void pci_write_handler(Bit8u address, Bit32u value, unsigned io_len);
  virtual Bit32u pci_read_handler(Bit8u address, unsigned io_len);
#endif
#if BX_DEBUGGER
  virtual void debug_dump(int argc, char **argv);
#endif

  // Config option support (radeon9800.cc)
  static Bit32s options_parser(const char *context, int num_params, char *params[]);
  static Bit32s options_save(FILE *fp);

  // ---- entry points used by the worker threads / secondary function ----
  void cp_thread_main(void);
  void raster_worker_main(rb_worker_t *w);
  bool cp_thread_run;
  bool raster_run;
  bool lfb_read(Bit32u off, unsigned len, Bit8u *data);
  bool lfb_write(Bit32u off, unsigned len, Bit8u *data);
  void mmio_read(Bit32u off, unsigned len, Bit8u *data);
  void mmio_write(Bit32u off, unsigned len, Bit8u *data);
  Bit32u pci_config_read(Bit8u address, unsigned io_len);
  friend class bx_radeon9800_sec_c;

protected:
  virtual void update(void);

private:
  // ---- radeon9800.cc: plugin glue, PCI, MMIO/IO dispatch, chip core ----
  static Bit32u svga_read_handler(void *this_ptr, Bit32u address, unsigned io_len);
  static void   svga_write_handler(void *this_ptr, Bit32u address, Bit32u value, unsigned io_len);
  Bit32u svga_read(Bit32u address, unsigned io_len);
  void   svga_write(Bit32u address, Bit32u value, unsigned io_len);
  static Bit32u io_read_handler(void *this_ptr, Bit32u address, unsigned io_len);
  static void   io_write_handler(void *this_ptr, Bit32u address, Bit32u value, unsigned io_len);
  static bool mem_read_handler(bx_phy_address addr, unsigned len, void *data, void *param);
  static bool mem_write_handler(bx_phy_address addr, unsigned len, void *data, void *param);
  static void vline_timer_handler(void *);
  static void pump_timer_handler(void *);

  void init_members(void);
  void init_pcihandlers(void);
  void chip_reset(void);
  void set_irq_level(bool level);
  void gen_int_update(void);
  void gen_int_fold_gui_idle(void);
  void fold_deferred(void);
  void timing_update(void);
  void update_banking(void);

  Bit32u reg_read32(Bit32u off);
  void   reg_write(Bit32u off, Bit32u val, Bit32u mask);
  void   reg_poke(Bit32u off, Bit32u val) { reg_write(off, val, 0xffffffff); }
  Bit32u core_reg_read(Bit32u off, bool *hit);
  bool   core_reg_write(Bit32u off, Bit32u val, Bit32u mask);

  // GPU (MC) address space
  bool   mc_is_vram(Bit32u mc, Bit32u *vram_off);
  bool   mc_addr_is_mapped(Bit32u mc);
  bool   mc_resolve(Bit32u mc, Bit32u *kind, Bit32u *addr);
  bool   gpu_read(Bit32u mc, Bit8u *dst, Bit32u len);
  bool   gpu_write(Bit32u mc, const Bit8u *src, Bit32u len);
  bool   gpu_read32(Bit32u mc, Bit32u *val);
  bool   gpu_write32(Bit32u mc, Bit32u val);
  bool   bus_master_ok(void);
  bool   bus_read_phys(Bit32u phys, Bit8u *dst, Bit32u len);
  bool   bus_write_phys(Bit32u phys, const Bit8u *src, Bit32u len);
  Bit32u vram_addr_xlate(Bit32u off);
  Bit32u hdp_surf_xlate(Bit32u mc);
  void   vram_dirty(Bit32u addr, Bit32u len);
  static void phys_read_chunked(Bit32u addr, Bit8u *dst, Bit32u len);
  static void phys_write_chunked(Bit32u addr, const Bit8u *src, Bit32u len);

  // ---- radeon9800.cc: display block ----
  bool   display_reg_read(Bit32u off, Bit32u *val);
  bool   display_reg_write(Bit32u off, Bit32u val, Bit32u mask);
  void   display_reset(void);
  void   ppll_commit(void);
  double ppll_vco_hz(int sel);
  double p2pll_hz(void);
  double dot_clock_hz(void);
  double spll_hz(void);
  double mpll_hz(void);
  double test_clock_hz(void);
  Bit32u pll_read(void);
  void   pll_write(Bit32u val, Bit32u mask);
  bool   pll_can_defer(int which);
  void   pll_settle(void);
  void   palette_data_write(Bit32u val, Bit32u mask, bool wide);
  Bit32u palette_data_read(bool wide);
  void   palette_sync_gui(void);
  void   update_mode(void);
  void   latch_crtc_offset(int c);
  void   crtc_settle(int c);
  void   cursor_publish(int c);
  void   cursor_frame_latch(void);
  void   timing_recalc(void);
  void   cursor_rect(Bit32u posn, Bit32u hvoff, int *x0, int *y0, int *x1, int *y1);
  void   paint_tile(unsigned xc, unsigned yc, bx_svga_tileinfo_t *info, Bit8u *pel8);
  void   paint_tiles_in(int x0, int y0, int x1, int y1);
  void   draw_hardware_cursor(unsigned xc, unsigned yc, bx_svga_tileinfo_t *info);
  Bit32u get_display_usec(void);
  Bit32u get_current_line(void);
  bool   in_vblank(void);
  void   snapshot_take(int c);
  void   redraw_area(Bit32s x0, Bit32s y0, Bit32u width, Bit32u height);
  Bit32u scanout_addr(Bit32u sy, Bit32u sx, Bit32u xoff);
  int    scanout_crtc(void);
  bool   crtc_extended(int c);
  void   scanout_refresh(void);
  bool   ext_mode(void);
  void   ddc_gpio_write(int port);
  Bit32u ddc_gpio_read(int port);

  // ---- radeon9800_2d.cc: card-space resolver ----
  bool   card_read_block(Bit32u vm, Bit8u *dst, Bit32u len);
  bool   card_write_block(Bit32u vm, const Bit8u *src, Bit32u len);
  bool   span_stage_grow(r9800_span_stage_t *st, Bit32u need);
  bool   span_stage_acquire(r9800_span_stage_t *st, Bit32u vm, Bit32u extent);
  void   span_stage_writeback(r9800_span_stage_t *st);
  bool   surf_map(r9800_surf_t *s, r9800_span_stage_t *st, Bit32u lo, Bit32u len);

  // ---- radeon9800_2d.cc: 2D GUI engine ----
  bool   r2d_reg_read(Bit32u off, Bit32u *val);
  bool   r2d_reg_write(Bit32u off, Bit32u val, Bit32u mask);
  void   r2d_reset(void);
  void   r2d_apply_gmc(Bit32u gmc);
  void   r2d_set_pitch_offset(bool is_dst, Bit32u val);
  void   r2d_clip(int *x0, int *y0, int *x1, int *y1);
  bool   r2d_map_span(r9800_surf_t *s, r9800_span_stage_t *st, Bit32u surf_base,
                      Bit32u pitch_px, int bpp, Bit32u tile, int y0, int y1, int x1);
  void   r2d_surf_commit(r9800_surf_t *s);
  void   r2d_surf_release(r9800_surf_t *s);
  void   r2d_mark_dirty(const r9800_surf_t *s, Bit32u addr, Bit32u len);
  int    r2d_ccmp_setup(bool has_src, int sbpp, int dbpp, struct r9800_ccmp *cc);
  Bit32u r2d_dst_addr(int x, int y, int bpp);
  Bit32u r2d_src_addr(int x, int y, int bpp);
  void   r2d_paint_rect(int x, int y, int w, int h, const Bit8u *mono8x8, const Bit8u *col8x8, bool mono_la);
  void   r2d_line(int x0, int y0, int x1, int y1, Bit32u pat32, bool pat_en, bool pat_la, int *phase);
  void   r2d_bres_line(Bit32u dir);
  void   r2d_blit_rect_pat(int sx, int sy, int dx, int dy, int w, int h, const Bit8u *pat8x8, const Bit8u *col8x8, bool mono_la);
  void   r2d_blit_rect(int sx, int sy, int dx, int dy, int w, int h);
  void   r2d_blit_rect_key(int sx, int sy, int dx, int dy, int w, int h, Bit32u key, Bit32u kmask, bool key_eq_skip);
  void   r2d_mono_rect(int x, int y, int w, int h, const Bit8u *bits, Bit32u bitpitch, const Bit8u *pat8x8, const Bit8u *patcol);
  void   r2d_host_color_rect(int x, int y, int w, int h, const Bit8u *px, Bit32u avail);
  bool   r2d_brush_block(Bit32u g, const Bit32u *pl, Bit32u count, Bit32u *pp, const Bit8u **mono,
                         const Bit8u **col, bool *mono_la, Bit32u *linepat, bool *line_en);
  void   r2d_packet3(Bit32u hdr, const Bit32u *pl, Bit32u count);
  void   r2d_fill_solid(int x, int y, int w, int h, Bit32u color);
  void   r2d_hostdata_paint(void);
  void   r2d_hostdata_word(Bit32u off, Bit32u val);
  void   r2d_gui_op_run(int w, int h);
  void   r2d_gui_op(int w, int h);
  void   r2d_span_rect(int y, int h, int xl, int xr, const Bit8u *mono8x8, const Bit8u *col8x8, bool mono_la);

  // ---- radeon9800_pm4.cc: command processor ----
  bool   pm4_reg_read(Bit32u off, Bit32u *val);
  bool   pm4_reg_write(Bit32u off, Bit32u val, Bit32u mask);
  void   pm4_reset(void);
  void   pm4_thread_init(void);
  void   pm4_thread_close(void);
  Bit32u pm4_ring_mask(void);
  bool   pm4_ring_bm(void);
  bool   pm4_active(void);
  int    pm4_pump(void);
  void   pm4_kick(void);
  void   pm4_gui_reset(void);
  void   pm4_drain_wait(void);
  void   pm4_flip_notify(void);
  bool   pm4_enqueue_write(Bit32u off, Bit32u val);
  bool   pm4_enqueue_indirect(Bit32u off, Bit32u n);
  bool   pm4_enqueue_pio(Bit32u val);
  Bit32u pm4_splice_ib(Bit32u wr, Bit32u off, Bit32u n, Bit32u rptr);
  void   cp_fifo_reserve(Bit32u wr, Bit32u need);
  bool   cp_get(Bit32u *val);
  bool   cp_get_ib(Bit32u *val);
  void   cp_packet(Bit32u hdr);
  void   pm4_run_indirect(void);
  bool   pm4_reg_in_fetch_block(Bit32u reg);
  void   pm4_exec_packet3(Bit32u hdr, Bit32u *pl, Bit32u n);
  void   pm4_wait_until(Bit32u val);
  void   pm4_scratch_write(int n, Bit32u val);
  void   pm4_rptr_writeback(void);
  bool   on_cp_thread(void);
  Bit32u pm4_timestamp(void);
  void   dma_gui_run(Bit32u table);

  // ---- radeon9800_3d.cc: 3D engine ----
  bool   r3d_reg_read(Bit32u off, Bit32u *val);
  bool   r3d_reg_write(Bit32u off, Bit32u val, Bit32u mask);
  void   r3d_reset(void);
  bool   r3d_packet3(Bit32u hdr, const Bit32u *pl, Bit32u count);
  void   r3d_pvs_vector_write(Bit32u val);
  Bit32u r3d_pvs_vector_read(void);
  void   r3d_vtx_state_write(Bit32u off, Bit32u val);
  void   r3d_port_data_write(Bit32u val);
  void   r3d_port_idx_write(Bit32u val);
  // VAP / draw
  void   vap_begin(Bit32u vf_cntl);
  void   vap_end(void);
  void   vap_vertex_dwords(const Bit32u *dw, Bit32u n);
  void   vap_index(Bit32u idx);
  void   vap_process_input(const r300_vap_in_t *in, r300_vtx_t *out, r300_vs_out_t *vso);
  bool   vap_fetch_aos(Bit32u idx, r300_vap_in_t *in);
  void   vap_psc_decode(const Bit32u *dw, Bit32u ndw, r300_vap_in_t *in);
  void   vap_run_pvs(const r300_vap_in_t *in, r300_vs_out_t *out);
  void   vap_bypass(const r300_vap_in_t *in, r300_vs_out_t *out);
  void   vap_emit(const r300_vs_out_t *vso);
  void   vap_prim_flush(void);
  void   vap_assemble(const r300_vs_out_t *v);
  void   vap_tri(const r300_vs_out_t *a, const r300_vs_out_t *b, const r300_vs_out_t *c);
  void   vap_line(const r300_vs_out_t *a, const r300_vs_out_t *b);
  void   vap_point(const r300_vs_out_t *a);
  int    vap_clip_poly(r300_vs_out_t *poly, int n, r300_vs_out_t *tmp);
  void   vap_vte(const r300_vs_out_t *vso, r300_vtx_t *out);
  void   r3d_draw_embedded(Bit32u vf_cntl, const Bit32u *data, Bit32u ndw);
  void   r3d_draw_indices(Bit32u vf_cntl, const Bit32u *idx, Bit32u ndw);
  void   r3d_draw_list(Bit32u vf_cntl);
  void   r3d_indx_buffer(const Bit32u *pl, Bit32u count);
  void   r3d_load_vbpntr(const Bit32u *pl, Bit32u count);
  void   r3d_clear_mask_ram(Bit32u *ram, Bit32u ram_size, const Bit32u *pl, Bit32u count);
  // raster state
  void   raster_state_capture(r9800_raster_state_t *rs);
  void   r3d_decode_fp(r9800_raster_state_t *rs);
  void   r3d_decode_textures(r9800_raster_state_t *rs);
  void   r3d_draw_state_derive(r9800_raster_state_t *rs);
  Bit32u r3d_stage_level(Bit32u key, Bit32u vm, Bit32u len);
  void   r3d_stage_textures(r9800_raster_state_t *rs);
  // rasterization
  void   r3d_tri(const r9800_raster_state_t *rs, int thr_id, int thr_mask,
                 const r300_vtx_t *a, const r300_vtx_t *b, const r300_vtx_t *c, int front);
  void   r3d_line(const r9800_raster_state_t *rs, int thr_id, int thr_mask,
                  const r300_vtx_t *a, const r300_vtx_t *b);
  void   r3d_point(const r9800_raster_state_t *rs, int thr_id, int thr_mask, const r300_vtx_t *v);
  void   r3d_tex_sample(const r9800_raster_state_t *rs, int unit, const float *coord, const float *dsdx,
                        const float *dsdy, float lod_bias, int proj, float *out);
  void   r3d_texel(const r9800_raster_state_t *rs, const r300_tex_desc_t *t, int level, int layer,
                   int u, int v, float *out);
  void   r3d_tex_level(const r9800_raster_state_t *rs, const r300_tex_desc_t *t, int level, int layer,
                       float s, float tt, int linear, float *out);
  bool   r3d_run_fp(const r9800_raster_state_t *rs, float temps[32][4], const float tc_ddx[32][4],
                    const float tc_ddy[32][4], Bit32u rs_written, float out[4][4], float *depth, int *depth_out);
  void   r3d_fragment(const r9800_raster_state_t *rs, int x, int y, r300_frag_t *f);
  bool   r3d_zs_test(const r9800_raster_state_t *rs, int x, int y, int sample, float z, int front, bool *z_passed);
  void   r3d_cb_write(const r9800_raster_state_t *rs, int x, int y, int sample, const float *col, const float *dst_in);
  bool   r3d_cb_read(const r9800_raster_state_t *rs, int x, int y, int sample, int target, float *col);
  void   r3d_aa_resolve(const r9800_raster_state_t *rs, int x, int y);
  void   r3d_mark_dirty(Bit32u addr, Bit32u len);
  // raster batch / worker pool
  void   raster_init(void);
  void   raster_close(void);
  void   raster_flush(void);
  void   raster_abandon(void);
  void   raster_submit(const r9800_raster_state_t *rs, int kind, const r300_vtx_t *v, int n, int front);
  void   rb_enqueue(const r9800_raster_state_t *rs, int kind, const r300_vtx_t *v, int n, int front);
  Bit32u rb_intern_state(const r9800_raster_state_t *rs);
  void   rb_run_parallel(void);
  void   rb_guard_rt(const r9800_raster_state_t *rs);
  void   rb_reset_written(void);
  void   r3d_zpass_writeback(void);

  // ---- radeon9800_ov0.cc: video overlay + subpicture ----
  bool   ov0_reg_read(Bit32u off, Bit32u *val);
  bool   ov0_reg_write(Bit32u off, Bit32u val, Bit32u mask);
  void   ov0_reset(void);
  void   ov0_apply(void);
  void   ov0_update(void);
  bool   subpic_reg_read(Bit32u off, Bit32u *val);
  bool   subpic_reg_write(Bit32u off, Bit32u val, Bit32u mask);
  void   ov0_frame_latch(void);
  void   draw_overlay(unsigned xc, unsigned yc, bx_svga_tileinfo_t *info);
  bool   ov0_gfx_key_match(int x, int y);
  void   subpic_blend(int out_x, int out_row, int *y, int *cb, int *cr);
  Bit32u ov0_buf_addr(Bit32u buf);
  void   ov0_csc(int y, int cb, int cr, int *r, int *g, int *b);
  int    ov0_gamma(int v);

  // =====================================================================
  // State
  // =====================================================================
  Bit8u  devfunc;
  bool   is_agp;
  Bit32u vram_size;
  Bit32u vram_mask;
  int    render_threads;
  int    asic_rev;
  int    vline_timer_id;
  int    pump_timer_id;
  bool   vga_disabled;
  double ref_freq_hz;
  bx_radeon9800_sec_c *sec;

  // chip core
  Bit32u mm_index;
  Bit32u bios_scratch[8];
  Bit32u bus_cntl;
  Bit32u bus_cntl1;
  Bit32u bus_cntl_mstr;
  Bit32u config_cntl;
  Bit32u config_memsize;
  Bit32u test_debug_cntl;
  Bit32u test_debug_mux;
  Bit32u hw_debug;
  Bit32u host_path_cntl;
  Bit32u hdp_fb_location;
  Bit32u mem_cntl;
  Bit32u mem_timing_cntl;
  Bit32u mc_fb_location;
  Bit32u mc_agp_location;
  Bit32u mem_init_lat_timer;
  Bit32u mem_sdram_mode_reg;
  Bit32u agp_base;
  Bit32u agp_base_2;
  Bit32u agp_cntl;
  Bit32u mem_io_cntl[4];
  Bit32u mc_read_cntl_ab;
  Bit32u mc_init_misc_lat_timer;
  Bit32u mc_init_gfx_lat_timer;
  Bit32u mc_debug;
  Bit32u mc_chp_io_oe_cntl_ab;
  Bit32u videomux_cntl;
  Bit32u mdgpio[4];
  Bit32u seprom_cntl1;
  Bit32u seprom_cntl2;
  Bit32u mpp_gp_config;
  Bit32u mpp_tb_config;
  Bit32u aic_cntl;
  Bit32u aic_pt_base;
  Bit32u aic_lo_addr;
  Bit32u aic_hi_addr;
  Bit32u aic_tlb_addr;
  Bit32u aic_tlb_data;
  Bit32u mc_ind_index;
  Bit32u mc_ind_regs[R9800_MC_IND_REGS];
  Bit32u rbbm_soft_reset;
  Bit32u rbbm_cntl;
  Bit32u surface_cntl;
  Bit32u surf_lower[8];
  Bit32u surf_upper[8];
  Bit32u surf_info[8];
  bool   surf_xlate_on;
  Bit32u gui_debug[6];
  Bit32u gpio[4];
  Bit32u i2c_cntl[3];
  Bit32u dvi_i2c[3];
  Bit32u gen_int_cntl;
  Bit32u gen_int_status;
  Bit32u cap_int_cntl;
  Bit32u cap_int_status;
  Bit32u fcp_cntl;
  Bit32u tv_regs[64];
  Bit32u cap0_regs[56];
  Bit32u viph_regs[24];
  Bit32u viph_reg_addr;
  Bit32u viph_reg_data;
  Bit32u disp_misc_cntl;
  Bit32u dac_macro_cntl;
  Bit32u disp_pwr_man;
  Bit32u disp_test_debug_cntl;
  Bit32u disp_hw_debug;
  Bit32u disp_merge_cntl;
  Bit32u disp_output_cntl;
  Bit32u disp2_merge_cntl;
  Bit32u disp_tv_out_cntl;
  Bit32u disp_lin_trans[8];
  Bit32u tv_dac_cntl;
  Bit32u ov1_regs[3];
  Bit32u surface_access_flags;
  volatile int gui_idle_event;
  volatile bool irq_dirty;
  volatile bool timing_dirty;
  bool   timing_valid;
  bx_crtc_params_t timing_last;
  Bit32u timing_vclock;
  unsigned gui_xres, gui_yres, gui_bpp;
  volatile bool palette_dirty;

  // Display block: two CRTCs
  Bit32u pll_regs[R9800_PLL_REGS];
  Bit32u clock_cntl_index;
  bool   ppll_update_pending;
  Bit32u ppll_work[5];
  bool   p2pll_update_pending;
  Bit32u p2pll_work[3];
  Bit8u  pll_test_count_base;
  Bit64u pll_test_zero_usec;
  double pll_test_acc;
  Bit32u crtc_gen_cntl;
  Bit32u crtc_ext_cntl;
  Bit32u crtc2_gen_cntl;
  Bit32u bank_w[2];
  Bit32u bank_r[2];
  Bit32u dac_cntl;
  Bit32u dac_cntl2;
  Bit8u  dac_mask_prog;
  struct r9800_crtc_t {
    Bit32u h_total_disp;
    Bit32u h_sync_strt_wid;
    Bit32u v_total_disp;
    Bit32u v_disp_active;
    Bit32u v_sync_strt_wid;
    Bit32u vline;
    Bit32u gui_trig_vline;
    Bit32u debug;
    Bit32u offset;
    Bit32u offset_latched;
    Bit32u offset_cntl;
    Bit32u pitch;
    Bit32u display_base;
    Bit32u tile_x0_y0;
    Bit32u ovr_clr;
    Bit32u ovr_wid_lr;
    Bit32u ovr_wid_tb;
    Bit32u cur_offset;
    Bit32u cur_posn;
    Bit32u cur_hvoff;
    Bit32u cur_offset_act;
    Bit32u cur_posn_act;
    Bit32u cur_hvoff_act;
    bool   cur_lock;
    Bit32u cur_clr0;
    Bit32u cur_clr1;
    Bit32u frame_count;
    Bit32u snapshot_vh_counts;
    Bit32u snapshot_f_count;
    Bit32u grph_buffer_cntl;
    bool   offset_pending;
    bool   offset_lock;
    bool   vblank_save;
  } crtc[2];
  // Cursor tuple as latched at the last vertical blank
  bool   cur_lat_en;
  Bit32u cur_lat_mode;
  Bit32u cur_lat_offset;
  Bit32u cur_lat_posn;
  Bit32u cur_lat_hvoff;
  Bit32u cur_lat_clr0;
  Bit32u cur_lat_clr1;
  volatile bool cur_bitmap_dirty;
  Bit32u crtc_offset_right;
  Bit32u n_vif_count;
  Bit32u snapshot_vif_cntl;
  Bit32u fp_regs[24];
  Bit32u fp2_regs[8];
  Bit32u dac_ext_cntl;
  Bit32u dac_crc_sig;
  Bit32u crtc_more_cntl;
  Bit32u lvds_gen_cntl;
  Bit32u lvds_pll_cntl;
  Bit32u lvds_ss_gen_cntl;
  Bit32u tmds_regs[3];
  Bit32u vga_buffer_cntl;
  Bit32u mem_vga_wp_sel;
  Bit32u mem_vga_rp_sel;
  Bit32u palette_index;
  Bit32u palette2[256];      // CRTC2 palette (0x00RRGGBB, 8 bit)
  Bit16u palette30[256][3];  // 10-bit palette shadow (CRTC1)
  Bit8u  ext_crtc[0x40];

  // Derived scanout state
  bool     disp_ext;
  int      disp_crtc;
  int      disp_output;      // R9800_OUT_*: the output feeding the emulated monitor
  unsigned disp_xres, disp_yres, disp_bpp, disp_pitch;
  Bit32u   disp_base;
  bool     disp_dblscan;
  bool     disp_hdbl;
  bool     scanout_tiled;
  Bit32u   scanout_micro, scanout_macro, scanout_x0, scanout_y0;
  bool     disp_blank;
  bool     disp_dac_const;
  Bit32u   disp_dac_const_color;
  bool     needs_update_mode;
  bool     needs_update_tile;
  bool     needs_update_dispentire;
  bool     ext_last;
  bool     vga_banked_mode;

  // 2D GUI engine context
  Bit32u dp_gui_master_cntl;
  Bit32u dp_brush_frgd_clr;
  Bit32u dp_brush_bkgd_clr;
  Bit32u brush_yx;
  Bit32u brush_scale;
  Bit32u brush_data[64];
  Bit32u dp_src_frgd_clr;
  Bit32u dp_src_bkgd_clr;
  Bit32u dp_cntl;
  Bit32u dp_datatype;
  Bit32u dp_mix;
  Bit32u dp_write_mask;
  Bit32u clr_cmp_clr_src;
  Bit32u clr_cmp_clr_dst;
  Bit32u clr_cmp_cntl;
  Bit32u clr_cmp_mask;
  Bit32u aux_sc_cntl;
  Bit32u aux_sc_rect[3][4];
  Bit32u default_pitch_offset;
  Bit32u default_pitch_reg;
  Bit32u default_sc_bottom_right;
  Bit32u sc_top_left;
  Bit32u sc_bottom_right;
  Bit32u src_offset;
  Bit32u src_pitch;
  Bit32u dst_offset;
  Bit32u dst_pitch;
  Bit32u src_tile;
  Bit32u dst_tile;
  Bit32u src_sc_right;
  Bit32u src_sc_bottom;
  Bit32s gui_dst_x, gui_dst_y, gui_src_x, gui_src_y;
  Bit32u gui_dst_w, gui_dst_h;
  Bit32u bres_err, bres_inc, bres_dec, bres_lnth;
  Bit32u dp_cntl_line;
  Bit32u dst_line_start, dst_line_end, dst_line_patcount;
  Bit32u gui_scratch[6];
  Bit32u scratch_umsk;
  Bit32u scratch_addr;
  Bit32u isync_cntl;
  Bit32u rbbm_guicntl;
  Bit32u dstcache_ctlstat;
  Bit32u rb2d_dstcache_mode;
  Bit32u scl_palette[256];
  bool   hostdata_active;
  int    hostdata_x, hostdata_y, hostdata_w, hostdata_h;
  int    hostdata_srcdt;
  Bit32u hostdata_ndw;
  Bit32u hostdata_buf[1024];
  r9800_span_stage_t s2d_dst;
  r9800_span_stage_t s2d_src;

  // Command processor
  Bit32u cp_rb_base;
  Bit32u cp_rb_cntl;
  Bit32u cp_rb_rptr_addr;
  Bit32u cp_rb_rptr;
  Bit32u cp_rb_wptr;
  Bit32u cp_rb_wptr_delay;
  Bit32u cp_ib_base;
  Bit32u cp_ib_bufsz;
  Bit32u cp_csq_cntl;
  Bit32u cp_csq_mode;
  Bit32u cp_resync_addr;
  Bit32u cp_resync_data;
  Bit32u cp_me_cntl;
  Bit32u cp_me_ram_addr;
  Bit32u cp_me_ram_raddr;
  Bit32u cp_me_ram[R9800_CP_ME_RAM_SIZE][2];
  Bit32u cp_csq_addr;
  Bit32u cp_vc_debug_config;
  Bit32u cp_dma_table_addr;   // 0x0780: address handed to the engine, purpose unconfirmed
  bool   pm4_ind_busy;
  bool   pm4_ind_pending;
  Bit32u pump_frame_rem;
  Bit32u pump_ib_state;
  Bit32u pump_ib_addr;
  Bit32u *cp_fifo;
  Bit32u *cp_fifo_rptr;
  Bit8u  *cp_fifo_tag;
  volatile Bit32u cp_fifo_rd;
  volatile Bit32u cp_fifo_wr;
  volatile Bit32u cp_retire_rptr;
  Bit32u cp_retire_pending;
  Bit32u cp_shadow_last;
  Bit32u *cp_pl;
  Bit32u *ind_pl;
  volatile int cp_executing;
  volatile int cp_batch_pending;
  volatile int cp_drain_req;
  volatile int cp_abort;
  bool   cp_thread_started;
  BX_THREAD_VAR(cp_thread_var);
  bx_thread_sem_t cp_wake_sem;
  bx_thread_sem_t cp_idle_sem;
  bx_thread_sem_t cp_flip_sem;
  BX_MUTEX(cp_mutex);
  bool   in_indirect;

  // 3D engine: register images
  Bit32u vap_regs[0x140];             // 0x2000-0x24ff
  Bit32u r3d_regs[R9800_R3D_IMAGE_DWORDS]; // 0x4000-0x4fff
  Bit32u vport_alt[6];
  Bit32u pvs_vec[R9800_PVS_VECTOR_MEM][4];
  Bit32u pvs_vec_index;
  Bit32u pvs_vec_sub;
  Bit32u zmask_ram[R9800_ZMASK_RAM_DWORDS];
  Bit32u hiz_ram[R9800_HIZ_RAM_DWORDS];
  Bit32u cmask_ram[R9800_CMASK_RAM_DWORDS];
  Bit32u zmask_wrindex, zmask_rdindex;
  Bit32u hiz_wrindex, hiz_rdindex;
  Bit32u cmask_wrindex, cmask_rdindex;
  volatile Bit32u zpass_count;
  Bit32u r3d_serial;
  BX_MUTEX(zmask_mutex);
  // VAP state-based vertex assembly
  float  vtx_state[27][4];
  // active draw
  struct {
    bool   active;
    Bit32u vf_cntl;
    Bit32u prim, walk, num;
    bool   idx32;
    Bit32u emitted;
    Bit32u vdw[128];
    Bit32u vdw_n;
    Bit32u vdw_per_vtx;
    r300_vs_out_t ring[3];
    int    ring_n;
    r300_vs_out_t first, prev;
    int    poly_n;
    int    even;
    r9800_raster_state_t *rs;
  } draw;
  // texture staging arena for system-memory levels
  struct {
    Bit8u *arena;
    Bit32u cap;
    Bit32u used;
    Bit32u ent_count;
    struct {
      Bit32u vm_base, len, arena_off;
    } ent[256];
  } tex_stage;
  // parallel raster batch
  struct {
    int nthreads;
    int mask;
    rb_cmd_t *cmds;
    Bit32u cmd_count, cmd_cap;
    r9800_raster_state_t *states;
    Bit32u state_count, state_cap;
    bool   rt_valid;
    Bit32u rt_c_off, rt_c_pitch, rt_c_bpp;
    bool   rt_z_valid;
    Bit32u rt_z_off, rt_z_pitch;
    bool   wr_valid;
    Bit32u wr_c_lo, wr_c_hi, wr_z_lo, wr_z_hi;
    rb_worker_t workers[R9800_RASTER_MAX_WORKERS];
  } rb;

  // OV0 overlay + subpicture
  struct {
    Bit32u shadow[64];
    Bit32u active[64];
  } ov0;
  Bit32u ov0_reg_load_cntl;
  Bit32u ov0_lin_trans[6];
  Bit32u ov0_gamma_regs[18];
  bool   ov0_enabled;
  int    ov0_x1, ov0_y1, ov0_x2, ov0_y2;
  struct {
    Bit32u regs[19];
    Bit32u pal[16];
    Bit32u active[19];
    Bit32u apal[16];
    Bit32u frame_stamp;
  } subpic;

  // ---- register access trace (bochsrc 'radeon9800: trace=N', 0 = off) ----
  int    trace_mask;
  Bit32u trace_rd_off, trace_rd_val, trace_rd_idx, trace_rd_run;
  bool   trace_rd_valid;
  // Run of consecutive reads of one register whatever it returns: catches a
  // wait loop on a counter or a status register that keeps changing
  Bit32u trace_run_off, trace_run_idx, trace_run_len;
  Bit32u trace_run_first, trace_run_last;
  bool   trace_run_varies;
  Bit32u trace_wr_off[R9800_TRACE_WRITES];
  Bit32u trace_wr_val[R9800_TRACE_WRITES];
  Bit32u trace_wr_idx[R9800_TRACE_WRITES];
  Bit32u trace_wr_pos, trace_wr_seen;
  Bit32u trace_polls;
  Bit32u trace_blk_off[R9800_TRACE_BLOCK_REGS], trace_blk_cnt[R9800_TRACE_BLOCK_REGS];
  Bit32u trace_blk_total;
  int    trace_blk_n;
  Bit32u trace_irq_count;   // interrupts actually delivered to the guest
  bool   trace_irq_level;
  Bit32u trace_hb_frames;
  Bit32u trace_fence_probes;
  void   trace_heartbeat(void);
  void   trace_mc_probe(const char *what, Bit32u mc);
  void   trace_reg_write(Bit32u off, Bit32u val, Bit32u mask);
  void   trace_reg_read(Bit32u off, Bit32u val);
  void   trace_read_flush(void);
  void   trace_poll_report(void);
  void   trace_block_note(Bit32u off);
  void   trace_block_report(void);
  void   trace_state_dump(void);
  Bit32u trace_index_of(Bit32u off);

  // DDC / I2C
  bx_ddc_c ddc;
};

#endif // BX_SUPPORT_RADEON9800

#endif // BX_IODEV_RADEON9800_H
