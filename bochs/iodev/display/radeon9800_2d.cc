/////////////////////////////////////////////////////////////////////////
// $Id$
/////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025-2026  The Bochs Project
//
//  ATI Radeon 9800 (R350) emulation: 2D GUI engine.
//
//  Datapath interpreter: solid / pattern paint, screen-to-screen blit
//  (plain and keyed), Bresenham lines, mono expansion and host data,
//  driven either by direct register writes or by PM4 type-3 packets.
//  Surfaces are addressed in the memory controller address space and may
//  be macro / micro tiled.
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

#define LOG_THIS BX_R9800_THIS

// ---------------------------------------------------------------------
// Datapath register accessors. The GMC is a write-through alias of
// DP_MIX / DP_DATATYPE; executors consume the active registers.
// ---------------------------------------------------------------------

#define DP_DST_DT()   ((dp_datatype >> 8) & 0xf)
#define DP_BRUSH_DT() (dp_datatype & 0xf)
#define DP_SRC_DT()   ((dp_datatype >> 16) & 3)
#define DP_LSB_FIRST() ((dp_datatype >> 30) & 1)
#define DP_ROP3()     ((Bit8u)((dp_mix >> 16) & 0xff))
#define DP_SRC_SOURCE() ((dp_mix >> 8) & 7)

// Destination datatype -> bytes per pixel
static int r2d_bpp(Bit32u dst_datatype)
{
  switch (dst_datatype) {
    case 3: case 4: case 15: return 2;
    case 5: return 3;
    case 6: case 14: return 4;
    case 11: case 12: case 13: return 2;
    default: return 1;
  }
}

// ROP3: evaluate any of the 256 ternary raster ops bitwise
Bit32u radeon9800_rop3(Bit8u rop, Bit32u p, Bit32u s, Bit32u d)
{
  Bit32u r = 0;
  if (rop & 0x01) r |= ~p & ~s & ~d;
  if (rop & 0x02) r |= ~p & ~s &  d;
  if (rop & 0x04) r |= ~p &  s & ~d;
  if (rop & 0x08) r |= ~p &  s &  d;
  if (rop & 0x10) r |=  p & ~s & ~d;
  if (rop & 0x20) r |=  p & ~s &  d;
  if (rop & 0x40) r |=  p &  s & ~d;
  if (rop & 0x80) r |=  p &  s &  d;
  return r;
}

static BX_CPP_INLINE void r2d_row_fill(Bit8u *dst, const Bit8u *px, int bpp, int count)
{
  Bit32u total = (Bit32u)count * (Bit32u)bpp;
  Bit32u done;
  if (bpp == 1) {
    memset(dst, px[0], (size_t)count);
    return;
  }
  memcpy(dst, px, (size_t)bpp);
  for (done = (Bit32u)bpp; done < total;) {
    Bit32u chunk = done < total - done ? done : total - done;
    memcpy(dst + done, dst, (size_t)chunk);
    done += chunk;
  }
}

// DP_WRITE_MASK gates the dst bitplanes a 2D op may modify (per pixel byte lane)
static BX_CPP_INLINE void r2d_store(Bit8u *dp, Bit8u v, Bit8u mb)
{
  *dp = (Bit8u)((v & mb) | (*dp & ~mb));
}

// 2D auxiliary scissor: only subtractive rects carve
bool radeon9800_aux_sc_pass_2d(Bit32u cntl, const Bit32u rect[3][4], int x, int y)
{
  for (int i = 0; i < 3; i++) {
    if (!(cntl & (1u << (i * 2))) || !(cntl & (2u << (i * 2))))
      continue;
    if ((x >= r9800_sx14(rect[i][0])) && (x <= r9800_sx14(rect[i][1])) &&
        (y >= r9800_sx14(rect[i][2])) && (y <= r9800_sx14(rect[i][3])))
      return false;
  }
  return true;
}

static bool r2d_aux_accepts_rect(Bit32u cntl, const Bit32u rect[3][4], int x0, int y0, int x1, int y1)
{
  for (int i = 0; i < 3; i++) {
    if (!(cntl & (1u << (i * 2))) || !(cntl & (2u << (i * 2))))
      continue;
    if ((x0 <= r9800_sx14(rect[i][1])) && (x1 >= r9800_sx14(rect[i][0])) &&
        (y0 <= r9800_sx14(rect[i][3])) && (y1 >= r9800_sx14(rect[i][2])))
      return false;
  }
  return true;
}

// ---------------------------------------------------------------------
// Colour compare (CLR_CMP_*) evaluated per pixel at the output stage
// ---------------------------------------------------------------------

int bx_radeon9800_c::r2d_ccmp_setup(bool has_src, int sbpp, int dbpp, struct r9800_ccmp *cc)
{
  int sel = (clr_cmp_cntl >> 24) & 3;
  int fs = clr_cmp_cntl & 7;
  int fd = (clr_cmp_cntl >> 8) & 7;
  Bit32u spm = (sbpp >= 4) ? 0xffffffff : ((1u << (sbpp * 8)) - 1u);
  Bit32u dpm = (dbpp >= 4) ? 0xffffffff : ((1u << (dbpp * 8)) - 1u);

  cc->fn_src = ((fs == 1) || (fs == 4) || (fs == 5) || (fs == 7)) ? fs : 0;
  cc->fn_dst = ((fd == 1) || (fd == 4) || (fd == 5)) ? fd : 0;
  cc->src_on = has_src && ((sel == 1) || (sel == 2));
  cc->dst_on = ((sel == 0) || (sel == 2));
  cc->smask = clr_cmp_mask & spm;
  cc->dmask = clr_cmp_mask & dpm;
  cc->key_src = clr_cmp_clr_src & cc->smask;
  cc->key_dst = clr_cmp_clr_dst & cc->dmask;
  cc->flip = dp_src_frgd_clr & dpm;
  return (cc->src_on && (cc->fn_src != 0)) || (cc->dst_on && (cc->fn_dst != 0));
}

// 0 = suppress the store, 1 = store the datapath result, 2 = store d ^ flip
static BX_CPP_INLINE int r2d_ccmp_px(const struct r9800_ccmp *cc, Bit32u s, Bit32u d)
{
  int flip = 0;
  if (cc->src_on) {
    bool eq = ((s & cc->smask) == cc->key_src);
    if ((cc->fn_src == 1) || ((cc->fn_src == 4) && !eq) || ((cc->fn_src == 5) && eq))
      return 0;
    if (cc->fn_src == 7) {
      if (!eq) return 0;
      flip = 1;
    }
  }
  if (cc->dst_on) {
    bool eq = ((d & cc->dmask) == cc->key_dst);
    if ((cc->fn_dst == 1) || ((cc->fn_dst == 4) && !eq) || ((cc->fn_dst == 5) && eq))
      return 0;
  }
  return flip ? 2 : 1;
}

// ---------------------------------------------------------------------
// Card-space resolver: MC addresses are local VRAM or system memory
// ---------------------------------------------------------------------

bool bx_radeon9800_c::card_read_block(Bit32u vm, Bit8u *dst, Bit32u len)
{
  return gpu_read(vm, dst, len);
}

bool bx_radeon9800_c::card_write_block(Bit32u vm, const Bit8u *src, Bit32u len)
{
  if (!len)
    return true;
  return gpu_write(vm, src, len);
}

bool bx_radeon9800_c::span_stage_grow(r9800_span_stage_t *st, Bit32u need)
{
  if (need <= st->cap)
    return true;
  Bit32u ncap = st->cap ? st->cap : (1u << 20);
  while (ncap < need) ncap <<= 1;
  Bit8u *na = (Bit8u*)realloc(st->arena, ncap);
  if (na == NULL)
    return false;
  st->arena = na;
  st->cap = ncap;
  return true;
}

bool bx_radeon9800_c::span_stage_acquire(r9800_span_stage_t *st, Bit32u vm, Bit32u extent)
{
  if (!extent)
    return false;
  if (st->active && (st->vm == vm) && (st->len >= extent))
    return true;
  if (st->active && (st->vm == vm)) {
    Bit32u old_len = st->len;
    if (!span_stage_grow(st, extent))
      return false;
    if (!card_read_block(vm + old_len, st->arena + old_len, extent - old_len))
      return false;
    st->len = extent;
    return true;
  }
  if (st->active)
    span_stage_writeback(st);
  if (!span_stage_grow(st, extent))
    return false;
  if (!card_read_block(vm, st->arena, extent))
    return false;
  st->vm = vm;
  st->len = extent;
  st->active = true;
  return true;
}

void bx_radeon9800_c::span_stage_writeback(r9800_span_stage_t *st)
{
  if (!st->active)
    return;
  if (st->arena && st->len)
    card_write_block(st->vm, st->arena, st->len);
  st->active = false;
}

// Map [lo, lo+len) of MC space: a direct window into VRAM when the span is
// frame buffer resident, a staged copy otherwise.
bool bx_radeon9800_c::surf_map(r9800_surf_t *s, r9800_span_stage_t *st, Bit32u lo, Bit32u len)
{
  Bit32u m, off0, off1;
  if (!len) len = 1;
  if (mc_is_vram(lo, &off0) && mc_is_vram(lo + len - 1, &off1) && (off1 >= off0)) {
    Bit32u fb_start = (mc_fb_location & 0xffff) << 16;
    s->base = BX_R9800_THIS s.memory;
    s->rel = fb_start;
    s->mask = vram_mask;
    s->st = NULL;
    return true;
  }
  if (!span_stage_acquire(st, lo, len))
    return false;
  m = 1;
  while (m < len) m <<= 1;
  s->base = st->arena;
  s->rel = lo;
  s->mask = m - 1u;
  s->st = st;
  return true;
}

void bx_radeon9800_c::r2d_surf_commit(r9800_surf_t *s)
{
  if (s->st) {
    span_stage_writeback(s->st);
    s->st = NULL;
  }
}

void bx_radeon9800_c::r2d_surf_release(r9800_surf_t *s)
{
  if (s->st) {
    s->st->active = false;
    s->st = NULL;
  }
}

void bx_radeon9800_c::r2d_mark_dirty(const r9800_surf_t *s, Bit32u addr, Bit32u len)
{
  Bit32u off;
  if (s->st)
    return;
  if (mc_is_vram(addr, &off))
    vram_dirty(off, len);
}

// ---------------------------------------------------------------------
// Engine state
// ---------------------------------------------------------------------

void bx_radeon9800_c::r2d_reset(void)
{
  dp_gui_master_cntl = 0;
  dp_brush_frgd_clr = 0;
  dp_brush_bkgd_clr = 0;
  brush_yx = 0;
  brush_scale = 0;
  dp_src_frgd_clr = 0;
  dp_src_bkgd_clr = 0;
  dp_cntl = 0;
  dp_datatype = 0;
  dp_mix = 0;
  sc_top_left = 0;
  sc_bottom_right = 0x1fff1fff;
  default_sc_bottom_right = 0x1fff1fff;
  default_pitch_offset = 0;
  default_pitch_reg = 0;
  dp_write_mask = 0xffffffff;
  clr_cmp_cntl = 0;
  clr_cmp_clr_src = 0;
  clr_cmp_clr_dst = 0;
  clr_cmp_mask = 0;
  aux_sc_cntl = 0;
  memset(aux_sc_rect, 0, sizeof(aux_sc_rect));
  src_offset = dst_offset = 0;
  src_pitch = dst_pitch = 0;
  src_tile = dst_tile = 0;
  src_sc_right = src_sc_bottom = 0;
  gui_dst_x = gui_dst_y = gui_src_x = gui_src_y = 0;
  gui_dst_w = gui_dst_h = 0;
  bres_err = bres_inc = bres_dec = bres_lnth = 0;
  dp_cntl_line = 0;
  dst_line_start = dst_line_end = dst_line_patcount = 0;
  isync_cntl = 0;
  rbbm_guicntl = 0;
  dstcache_ctlstat = 0;
  rb2d_dstcache_mode = 0;
  hostdata_active = false;
  hostdata_ndw = 0;
}

// SRC/DST_PITCH_OFFSET packing: offset[21:0] in 1 KB units, pitch[29:22] in
// 64-byte units, tile mode [31:30] (1 = macro, 2 = micro, 3 = both)
void bx_radeon9800_c::r2d_set_pitch_offset(bool is_dst, Bit32u val)
{
  Bit32u off = (val & 0x003fffff) << 10;
  Bit32u pitch = ((val >> 22) & 0xff) << 6;
  Bit32u tile = val >> 30;
  if (is_dst) {
    dst_offset = off;
    dst_pitch = pitch;
    dst_tile = tile;
  } else {
    src_offset = off;
    src_pitch = pitch;
    src_tile = tile;
  }
}

// Apply a DP_GUI_MASTER_CNTL image
void bx_radeon9800_c::r2d_apply_gmc(Bit32u gmc)
{
  dp_gui_master_cntl = gmc;
  if (!(gmc & R9800_GMC_SRC_PITCH_OFFSET_CNTL))
    r2d_set_pitch_offset(false, default_pitch_offset);
  if (!(gmc & R9800_GMC_DST_PITCH_OFFSET_CNTL))
    r2d_set_pitch_offset(true, default_pitch_offset);
  if (!(gmc & R9800_GMC_SRC_CLIPPING)) {
    src_sc_right = default_sc_bottom_right & 0x3fff;
    src_sc_bottom = (default_sc_bottom_right >> 16) & 0x3fff;
  }
  if (!(gmc & R9800_GMC_DST_CLIPPING)) {
    sc_top_left = 0;
    sc_bottom_right = default_sc_bottom_right;
  }
  if (gmc & R9800_GMC_WR_MSK_DIS) {
    dp_write_mask = 0xffffffff;
    clr_cmp_mask = 0xffffffff;
  }
  if (gmc & R9800_GMC_CLR_CMP_CNTL_DIS)
    clr_cmp_cntl &= ~0x00000707;
  if (gmc & R9800_GMC_AUX_CLIP_DIS)
    aux_sc_cntl &= ~R9800_AUX_SC_ENB_MASK;

  dp_mix = (dp_mix & ~0x00ff0700) | (gmc & 0x00ff0000) | ((Bit32u)R9800_GMC_SRC_SOURCE(gmc) << 8);
  dp_datatype = (dp_datatype & ~0xc0030f0f) |
                ((Bit32u)R9800_GMC_DST_DATATYPE(gmc) << 8) |
                (Bit32u)R9800_GMC_BRUSH_TYPE(gmc) |
                ((Bit32u)R9800_GMC_SRC_DATATYPE(gmc) << 16) |
                (((gmc >> 14) & 1u) << 30) | (((gmc >> 15) & 1u) << 31);
  dp_cntl |= R9800_DP_CNTL_DST_X_DIR | R9800_DP_CNTL_DST_Y_DIR | R9800_DP_CNTL_POLY_LINE;
}

void bx_radeon9800_c::r2d_clip(int *x0, int *y0, int *x1, int *y1)
{
  *x0 = sc_top_left & 0x3fff;
  *y0 = (sc_top_left >> 16) & 0x3fff;
  *x1 = sc_bottom_right & 0x3fff;
  *y1 = (sc_bottom_right >> 16) & 0x3fff;
}

static BX_CPP_INLINE Bit32u r2d_tiled_addr(Bit32u base, Bit32u pitch_b, Bit32u tile, int x, int y, int bpp)
{
  Bit32u micro = (tile & 2) ? 1 : 0;
  Bit32u macro = (tile & 1) ? 1 : 0;
  if (!tile || !pitch_b)
    return base + (Bit32u)y * pitch_b + (Bit32u)x * (Bit32u)bpp;
  return base + r300_tile_addr((Bit32u)x, (Bit32u)y, pitch_b, (Bit32u)bpp, micro, macro);
}

Bit32u bx_radeon9800_c::r2d_dst_addr(int x, int y, int bpp)
{
  return r2d_tiled_addr(dst_offset, dst_pitch, dst_tile, x, y, bpp);
}

Bit32u bx_radeon9800_c::r2d_src_addr(int x, int y, int bpp)
{
  return r2d_tiled_addr(src_offset, src_pitch, src_tile, x, y, bpp);
}

// Resolve the dst/src window for a 2D op touching rows [y0..y1] up to
// column x1. Tiled surfaces map the byte range of the enclosing tile rows,
// which is contiguous.
bool bx_radeon9800_c::r2d_map_span(r9800_surf_t *s, r9800_span_stage_t *st, Bit32u surf_base,
                                   Bit32u pitch_b, int bpp, Bit32u tile, int y0, int y1, int x1)
{
  Bit32u lo, len;

  if (y0 < 0) y0 = 0;
  if ((y1 < y0) || (x1 < 0) || !pitch_b) {
    s->base = BX_R9800_THIS s.memory;
    s->rel = (mc_fb_location & 0xffff) << 16;
    s->mask = vram_mask;
    s->st = NULL;
    return true;
  }
  if (tile) {
    Bit32u rows = r300_tile_rows((Bit32u)bpp, (tile & 2) ? 1 : 0, (tile & 1) ? 1 : 0);
    Bit32u ty0 = (Bit32u)y0 / rows, ty1 = (Bit32u)y1 / rows;
    lo = surf_base + ty0 * rows * pitch_b;
    len = (ty1 - ty0 + 1) * rows * pitch_b;
  } else {
    lo = surf_base + (Bit32u)y0 * pitch_b;
    len = (Bit32u)(y1 - y0) * pitch_b + ((Bit32u)x1 + 1u) * (Bit32u)bpp;
  }
  return surf_map(s, st, lo, len);
}

// ---------------------------------------------------------------------
// Executors
// ---------------------------------------------------------------------

// Fill one rectangle through the current ROP3. mono8x8 = 8x8 mono pattern
// (byte n = row n, MSB first), col8x8 = 64 dst-format pixels; both NULL
// for solid. Patterns are screen-aligned.
void bx_radeon9800_c::r2d_paint_rect(int x, int y, int w, int h, const Bit8u *mono8x8, const Bit8u *col8x8, bool mono_la)
{
  int bpp = r2d_bpp(DP_DST_DT());
  Bit8u rop = DP_ROP3();
  Bit32u wmask = dp_write_mask;
  bool aux_on = (aux_sc_cntl & R9800_AUX_SC_ENB_MASK) != 0;
  struct r9800_ccmp cc;
  int cca;
  r9800_surf_t sd;
  int cx0, cy0, cx1, cy1;

  r2d_clip(&cx0, &cy0, &cx1, &cy1);
  if (x < cx0) { w -= cx0 - x; x = cx0; }
  if (y < cy0) { h -= cy0 - y; y = cy0; }
  if (x + w - 1 > cx1) w = cx1 - x + 1;
  if (y + h - 1 > cy1) h = cy1 - y + 1;
  if ((w <= 0) || (h <= 0))
    return;
  cca = r2d_ccmp_setup(false, bpp, bpp, &cc);

  bool aux_ok = !aux_on || r2d_aux_accepts_rect(aux_sc_cntl, aux_sc_rect, x, y, x + w - 1, y + h - 1);
  bool fast = aux_ok && (wmask == 0xffffffff) && (rop == 0xf0) && !mono8x8 && !col8x8 && !cca && !dst_tile;
  Bit8u fpx[4];
  if (fast)
    for (int b = 0; b < bpp; b++)
      fpx[b] = (Bit8u)((dp_brush_frgd_clr >> ((b & 3) * 8)) & 0xff);

  if (!r2d_map_span(&sd, &s2d_dst, dst_offset, dst_pitch, bpp, dst_tile, y, y + h - 1, x + w - 1))
    return;

  for (int row = 0; row < h; row++) {
    Bit32u addr = r2d_dst_addr(x, y + row, bpp);
    if (fast) {
      Bit8u *run = r9800_surf_run(&sd, addr, (Bit32u)w * bpp);
      if (run) {
        r2d_row_fill(run, fpx, bpp, w);
        r2d_mark_dirty(&sd, addr, (Bit32u)w * bpp);
        continue;
      }
    }
    for (int col = 0; col < w; col++) {
      Bit32u pat = dp_brush_frgd_clr;
      const Bit8u *patpx = NULL;
      Bit32u a = dst_tile ? r2d_dst_addr(x + col, y + row, bpp) : addr + (Bit32u)col * bpp;

      if (aux_on && !radeon9800_aux_sc_pass_2d(aux_sc_cntl, aux_sc_rect, x + col, y + row))
        continue;
      if (mono8x8) {
        int set = (mono8x8[(y + row) & 7] >> (7 - ((x + col) & 7))) & 1;
        if (!set && mono_la)
          continue;
        pat = set ? dp_brush_frgd_clr : dp_brush_bkgd_clr;
      } else if (col8x8) {
        patpx = &col8x8[(((y + row) & 7) * 8 + ((x + col) & 7)) * bpp];
      }
      if (cca) {
        Bit32u dpx = 0;
        for (int b = 0; b < bpp; b++)
          dpx |= (Bit32u)*r9800_surf_at(&sd, a + (Bit32u)b) << (b * 8);
        if (!r2d_ccmp_px(&cc, 0, dpx))
          continue;
      }
      for (int b = 0; b < bpp; b++) {
        Bit8u *dp = r9800_surf_at(&sd, a + (Bit32u)b);
        Bit8u pb = patpx ? patpx[b] : ((pat >> ((b & 3) * 8)) & 0xff);
        r2d_store(dp, (Bit8u)radeon9800_rop3(rop, pb, 0, *dp), (Bit8u)(wmask >> ((b & 3) * 8)));
      }
      if (dst_tile)
        r2d_mark_dirty(&sd, a, bpp);
    }
    if (!dst_tile)
      r2d_mark_dirty(&sd, addr, (Bit32u)w * bpp);
  }
  r2d_surf_commit(&sd);
}

// Line segment (Bresenham) through the ROP3 P operand and the scissor.
// Brush types 6/7 carry a 32x1 line pattern.
void bx_radeon9800_c::r2d_line(int x0, int y0, int x1, int y1, Bit32u pat32, bool pat_en, bool pat_la, int *phase)
{
  int bpp = r2d_bpp(DP_DST_DT());
  Bit8u rop = DP_ROP3();
  Bit32u wmask = dp_write_mask;
  bool aux_on = (aux_sc_cntl & R9800_AUX_SC_ENB_MASK) != 0;
  struct r9800_ccmp cc;
  int cca = r2d_ccmp_setup(false, bpp, bpp, &cc);
  r9800_surf_t sd;
  int cx0, cy0, cx1, cy1;
  int dx = x1 > x0 ? x1 - x0 : x0 - x1;
  int dy = y1 > y0 ? y1 - y0 : y0 - y1;
  int sx = x0 < x1 ? 1 : -1;
  int sy = y0 < y1 ? 1 : -1;
  int err = dx - dy;
  int bx1, by0, by1;

  r2d_clip(&cx0, &cy0, &cx1, &cy1);
  by0 = (y0 < y1 ? y0 : y1); if (by0 < cy0) by0 = cy0;
  by1 = (y0 > y1 ? y0 : y1); if (by1 > cy1) by1 = cy1;
  bx1 = (x0 > x1 ? x0 : x1); if (bx1 > cx1) bx1 = cx1;

  if (!r2d_map_span(&sd, &s2d_dst, dst_offset, dst_pitch, bpp, dst_tile, by0, by1, bx1))
    return;

  for (;;) {
    int set = 1;
    bool at_end = (x0 == x1) && (y0 == y1);

    if (at_end && !(dp_cntl & R9800_DP_CNTL_DST_LAST_PEL))
      break;
    if (pat_en) {
      set = (pat32 >> (31 - (*phase & 31))) & 1;
      (*phase)++;
    }
    if ((set || !pat_la) && (x0 >= cx0) && (x0 <= cx1) && (y0 >= cy0) && (y0 <= cy1) &&
        (!aux_on || radeon9800_aux_sc_pass_2d(aux_sc_cntl, aux_sc_rect, x0, y0))) {
      Bit32u a = r2d_dst_addr(x0, y0, bpp);
      Bit32u pat = set ? dp_brush_frgd_clr : dp_brush_bkgd_clr;
      int cok = 1;
      if (cca) {
        Bit32u dpx = 0;
        for (int b = 0; b < bpp; b++)
          dpx |= (Bit32u)*r9800_surf_at(&sd, a + (Bit32u)b) << (b * 8);
        cok = r2d_ccmp_px(&cc, 0, dpx);
      }
      for (int b = 0; cok && (b < bpp); b++) {
        Bit8u *dp = r9800_surf_at(&sd, a + (Bit32u)b);
        Bit8u pb = (pat >> ((b & 3) * 8)) & 0xff;
        r2d_store(dp, (Bit8u)radeon9800_rop3(rop, pb, 0, *dp), (Bit8u)(wmask >> ((b & 3) * 8)));
      }
      r2d_mark_dirty(&sd, a, bpp);
    }
    if (at_end)
      break;
    int e2 = err * 2;
    if (e2 > -dy) { err -= dy; x0 += sx; }
    if (e2 <  dx) { err += dx; y0 += sy; }
  }
  r2d_surf_commit(&sd);
}

// Register-triggered Bresenham line (XFree86 / Linux 2D path)
void bx_radeon9800_c::r2d_bres_line(Bit32u dir)
{
  int bpp = r2d_bpp(DP_DST_DT());
  Bit8u rop = DP_ROP3();
  Bit32u wmask = dp_write_mask;
  bool aux_on = (aux_sc_cntl & R9800_AUX_SC_ENB_MASK) != 0;
  bool ymajor = (dir & R9800_DP_LINE_Y_MAJOR) != 0;
  int xstep = (dir & R9800_DP_LINE_X_DIR) ? 1 : -1;
  int ystep = (dir & R9800_DP_LINE_Y_DIR) ? 1 : -1;
  bool zero_pos = (!ymajor && (ystep < 0)) || (ymajor && (xstep < 0));
  int n = (int)(bres_lnth & 0x3fff);
  Bit32s inc = (Bit32s)(bres_inc << 12) >> 12;
  Bit32s dec = (Bit32s)(bres_dec << 12) >> 12;
  Bit32s e = (Bit32s)(bres_err << 12) >> 12;
  int x = gui_dst_x, y = gui_dst_y;
  struct r9800_ccmp cc;
  int cca = r2d_ccmp_setup(false, bpp, bpp, &cc);
  r9800_surf_t sd;
  int cx0, cy0, cx1, cy1;
  int xa, ya, xb, yb;

  if (n <= 0)
    return;
  r2d_clip(&cx0, &cy0, &cx1, &cy1);

  {
    int px = x, py = y;
    Bit32s pe = e;
    xa = xb = px;
    ya = yb = py;
    for (int i = 0; i < n; i++) {
      if (px < xa) xa = px;
      if (px > xb) xb = px;
      if (py < ya) ya = py;
      if (py > yb) yb = py;
      if (zero_pos ? (pe >= 0) : (pe > 0)) { if (ymajor) px += xstep; else py += ystep; pe += dec; }
      pe += inc;
      if (ymajor) py += ystep; else px += xstep;
    }
  }
  if (ya < cy0) ya = cy0;
  if (yb > cy1) yb = cy1;
  if (xb > cx1) xb = cx1;
  if (xb < 0) xb = 0;
  if (ya > yb)
    return;
  if (!r2d_map_span(&sd, &s2d_dst, dst_offset, dst_pitch, bpp, dst_tile, ya, yb, xb))
    return;

  for (int i = 0; i < n; i++) {
    if ((x >= cx0) && (x <= cx1) && (y >= cy0) && (y <= cy1) &&
        (!aux_on || radeon9800_aux_sc_pass_2d(aux_sc_cntl, aux_sc_rect, x, y))) {
      Bit32u a = r2d_dst_addr(x, y, bpp);
      Bit32u pat = dp_brush_frgd_clr;
      int cok = 1;
      if (cca) {
        Bit32u dpx = 0;
        for (int b = 0; b < bpp; b++)
          dpx |= (Bit32u)*r9800_surf_at(&sd, a + (Bit32u)b) << (b * 8);
        cok = r2d_ccmp_px(&cc, 0, dpx);
      }
      for (int b = 0; cok && (b < bpp); b++) {
        Bit8u *dp = r9800_surf_at(&sd, a + (Bit32u)b);
        Bit8u pb = (pat >> ((b & 3) * 8)) & 0xff;
        r2d_store(dp, (Bit8u)radeon9800_rop3(rop, pb, 0, *dp), (Bit8u)(wmask >> ((b & 3) * 8)));
      }
      r2d_mark_dirty(&sd, a, bpp);
    }
    if (zero_pos ? (e >= 0) : (e > 0)) { if (ymajor) x += xstep; else y += ystep; e += dec; }
    e += inc;
    if (ymajor) y += ystep; else x += xstep;
  }
  r2d_surf_commit(&sd);
}

// Screen-to-screen blit through the ROP3 with an optional pattern brush
void bx_radeon9800_c::r2d_blit_rect_pat(int sx, int sy, int dx, int dy, int w, int h,
                                        const Bit8u *pat8x8, const Bit8u *col8x8, bool mono_la)
{
  int bpp = r2d_bpp(DP_DST_DT());
  Bit8u rop = DP_ROP3();
  Bit32u pat = dp_brush_frgd_clr;
  Bit32u wmask = dp_write_mask;
  bool aux_on = (aux_sc_cntl & R9800_AUX_SC_ENB_MASK) != 0;
  struct r9800_ccmp cc;
  int cca;
  r9800_surf_t sdst, ssrc;
  int cx0, cy0, cx1, cy1;

  r2d_clip(&cx0, &cy0, &cx1, &cy1);
  if (dx < cx0) { w -= cx0 - dx; sx += cx0 - dx; dx = cx0; }
  if (dy < cy0) { h -= cy0 - dy; sy += cy0 - dy; dy = cy0; }
  if (dx + w - 1 > cx1) w = cx1 - dx + 1;
  if (dy + h - 1 > cy1) h = cy1 - dy + 1;
  if ((w <= 0) || (h <= 0))
    return;
  cca = r2d_ccmp_setup(true, bpp, bpp, &cc);

  Bit8u *rowbuf = new Bit8u[(size_t)w * bpp];
  if (!r2d_map_span(&sdst, &s2d_dst, dst_offset, dst_pitch, bpp, dst_tile, dy, dy + h - 1, dx + w - 1)) {
    delete [] rowbuf;
    return;
  }
  if (!r2d_map_span(&ssrc, &s2d_src, src_offset, src_pitch, bpp, src_tile, sy < 0 ? 0 : sy,
                    sy + h - 1 < 0 ? 0 : sy + h - 1, sx + w - 1 < 0 ? 0 : sx + w - 1)) {
    r2d_surf_release(&sdst);
    delete [] rowbuf;
    return;
  }

  bool top_down = !(dy > sy);
  bool fast = !aux_on && (wmask == 0xffffffff) && (rop == 0xcc) && !cca && !(pat8x8 && mono_la) && !dst_tile && !src_tile;

  for (int i = 0; i < h; i++) {
    int row = top_down ? i : (h - 1 - i);
    Bit32u saddr = r2d_src_addr(sx, sy + row, bpp);
    Bit32u daddr = r2d_dst_addr(dx, dy + row, bpp);

    if (src_tile) {
      for (int col = 0; col < w; col++) {
        Bit32u sa = r2d_src_addr(sx + col, sy + row, bpp);
        for (int b = 0; b < bpp; b++)
          rowbuf[col * bpp + b] = *r9800_surf_at(&ssrc, sa + (Bit32u)b);
      }
    } else {
      for (int b = 0; b < w * bpp; b++)
        rowbuf[b] = *r9800_surf_at(&ssrc, saddr + (Bit32u)b);
    }
    if (fast) {
      Bit8u *run = r9800_surf_run(&sdst, daddr, (Bit32u)w * bpp);
      if (run) {
        memcpy(run, rowbuf, (size_t)w * bpp);
        r2d_mark_dirty(&sdst, daddr, (Bit32u)w * bpp);
        continue;
      }
    }
    for (int col = 0; col < w; col++) {
      Bit32u pcol = 0;
      const Bit8u *patpx = NULL;
      Bit32u da = dst_tile ? r2d_dst_addr(dx + col, dy + row, bpp) : daddr + (Bit32u)col * bpp;

      if (aux_on && !radeon9800_aux_sc_pass_2d(aux_sc_cntl, aux_sc_rect, dx + col, dy + row))
        continue;
      if (pat8x8) {
        int pset = (pat8x8[(dy + row) & 7] >> (7 - ((dx + col) & 7))) & 1;
        if (!pset && mono_la)
          continue;
        pcol = pset ? dp_brush_frgd_clr : dp_brush_bkgd_clr;
      } else if (col8x8) {
        patpx = &col8x8[(((dy + row) & 7) * 8 + ((dx + col) & 7)) * bpp];
      }
      if (cca) {
        Bit32u rb0 = (Bit32u)col * bpp;
        Bit32u spx = 0, dpx = 0;
        for (int b = 0; b < bpp; b++) {
          spx |= (Bit32u)rowbuf[rb0 + (Bit32u)b] << (b * 8);
          dpx |= (Bit32u)*r9800_surf_at(&sdst, da + (Bit32u)b) << (b * 8);
        }
        int cr = r2d_ccmp_px(&cc, spx, dpx);
        if (!cr)
          continue;
        if (cr == 2) {
          for (int b = 0; b < bpp; b++)
            r2d_store(r9800_surf_at(&sdst, da + (Bit32u)b), (Bit8u)((dpx ^ cc.flip) >> (b * 8)),
                      (Bit8u)(wmask >> ((b & 3) * 8)));
          continue;
        }
      }
      for (int b = 0; b < bpp; b++) {
        Bit32u rbi = (Bit32u)col * bpp + (Bit32u)b;
        Bit8u *dp = r9800_surf_at(&sdst, da + (Bit32u)b);
        Bit8u pb = patpx ? patpx[b] : pat8x8 ? (Bit8u)((pcol >> ((b & 3) * 8)) & 0xff) : (Bit8u)((pat >> ((b & 3) * 8)) & 0xff);
        r2d_store(dp, (Bit8u)radeon9800_rop3(rop, pb, rowbuf[rbi], *dp), (Bit8u)(wmask >> ((b & 3) * 8)));
      }
      if (dst_tile)
        r2d_mark_dirty(&sdst, da, bpp);
    }
    if (!dst_tile)
      r2d_mark_dirty(&sdst, daddr, (Bit32u)w * bpp);
  }
  r2d_surf_release(&ssrc);
  r2d_surf_commit(&sdst);
  delete [] rowbuf;
}

void bx_radeon9800_c::r2d_blit_rect(int sx, int sy, int dx, int dy, int w, int h)
{
  r2d_blit_rect_pat(sx, sy, dx, dy, w, h, NULL, NULL, false);
}

// Transparent (colour keyed) screen-to-screen blit
void bx_radeon9800_c::r2d_blit_rect_key(int sx, int sy, int dx, int dy, int w, int h,
                                        Bit32u key, Bit32u kmask, bool key_eq_skip)
{
  int bpp = r2d_bpp(DP_DST_DT());
  Bit8u rop = DP_ROP3();
  Bit32u pat = dp_brush_frgd_clr;
  Bit32u wmask = dp_write_mask;
  bool aux_on = (aux_sc_cntl & R9800_AUX_SC_ENB_MASK) != 0;
  Bit32u pmask = (bpp >= 4) ? 0xffffffff : ((1u << (bpp * 8)) - 1);
  r9800_surf_t sdst, ssrc;
  int cx0, cy0, cx1, cy1;

  r2d_clip(&cx0, &cy0, &cx1, &cy1);
  if (dx < cx0) { w -= cx0 - dx; sx += cx0 - dx; dx = cx0; }
  if (dy < cy0) { h -= cy0 - dy; sy += cy0 - dy; dy = cy0; }
  if (dx + w - 1 > cx1) w = cx1 - dx + 1;
  if (dy + h - 1 > cy1) h = cy1 - dy + 1;
  if ((w <= 0) || (h <= 0))
    return;
  kmask &= pmask;
  key &= kmask;

  if (!r2d_map_span(&sdst, &s2d_dst, dst_offset, dst_pitch, bpp, dst_tile, dy, dy + h - 1, dx + w - 1))
    return;
  if (!r2d_map_span(&ssrc, &s2d_src, src_offset, src_pitch, bpp, src_tile, sy < 0 ? 0 : sy,
                    sy + h - 1 < 0 ? 0 : sy + h - 1, sx + w - 1 < 0 ? 0 : sx + w - 1)) {
    r2d_surf_release(&sdst);
    return;
  }
  for (int row = 0; row < h; row++) {
    for (int col = 0; col < w; col++) {
      Bit32u sa = r2d_src_addr(sx + col, sy + row, bpp);
      Bit32u da = r2d_dst_addr(dx + col, dy + row, bpp);
      Bit32u sv = 0;
      if (aux_on && !radeon9800_aux_sc_pass_2d(aux_sc_cntl, aux_sc_rect, dx + col, dy + row))
        continue;
      for (int b = 0; b < bpp; b++)
        sv |= (Bit32u)*r9800_surf_at(&ssrc, sa + (Bit32u)b) << (b * 8);
      if (((sv & kmask) == key) == key_eq_skip)
        continue;
      for (int b = 0; b < bpp; b++) {
        Bit8u *dp = r9800_surf_at(&sdst, da + (Bit32u)b);
        Bit8u pb = (pat >> ((b & 3) * 8)) & 0xff;
        Bit8u sb = (sv >> (b * 8)) & 0xff;
        r2d_store(dp, (Bit8u)radeon9800_rop3(rop, pb, sb, *dp), (Bit8u)(wmask >> ((b & 3) * 8)));
      }
      r2d_mark_dirty(&sdst, da, bpp);
    }
  }
  r2d_surf_release(&ssrc);
  r2d_surf_commit(&sdst);
}

// Mono expand: set bits paint the foreground; clear bits write the
// background or are left alone per the source datatype. bitpitch in bits.
void bx_radeon9800_c::r2d_mono_rect(int x, int y, int w, int h, const Bit8u *bits, Bit32u bitpitch,
                                    const Bit8u *pat8x8, const Bit8u *patcol)
{
  int bpp = r2d_bpp(DP_DST_DT());
  Bit8u rop = DP_ROP3();
  bool use_bkgd = (DP_SRC_DT() == 0);
  bool lsb_first = DP_LSB_FIRST() != 0;
  Bit32u wmask = dp_write_mask;
  bool aux_on = (aux_sc_cntl & R9800_AUX_SC_ENB_MASK) != 0;
  struct r9800_ccmp cc;
  int cca = r2d_ccmp_setup(true, bpp, bpp, &cc);
  r9800_surf_t sd;
  int cx0, cy0, cx1, cy1;

  r2d_clip(&cx0, &cy0, &cx1, &cy1);
  if (!r2d_map_span(&sd, &s2d_dst, dst_offset, dst_pitch, bpp, dst_tile, y < cy0 ? cy0 : y,
                    y + h - 1 > cy1 ? cy1 : y + h - 1, x + w - 1 > cx1 ? cx1 : x + w - 1))
    return;

  for (int row = 0; row < h; row++) {
    if ((y + row < cy0) || (y + row > cy1))
      continue;
    for (int col = 0; col < w; col++) {
      Bit32u bit = (Bit32u)row * bitpitch + col;
      int set;
      if ((x + col < cx0) || (x + col > cx1))
        continue;
      if (aux_on && !radeon9800_aux_sc_pass_2d(aux_sc_cntl, aux_sc_rect, x + col, y + row))
        continue;
      if (lsb_first)
        set = (bits[bit >> 3] >> (bit & 7)) & 1;
      else
        set = (bits[bit >> 3] >> (7 - (bit & 7))) & 1;
      if (!set && !use_bkgd)
        continue;
      Bit32u fg = set ? dp_src_frgd_clr : dp_src_bkgd_clr;
      Bit32u a = r2d_dst_addr(x + col, y + row, bpp);
      Bit32u pat = dp_brush_frgd_clr;
      const Bit8u *patpx = NULL;
      if (pat8x8) {
        int pset = (pat8x8[(y + row) & 7] >> (7 - ((x + col) & 7))) & 1;
        pat = pset ? dp_brush_frgd_clr : dp_brush_bkgd_clr;
      } else if (patcol) {
        patpx = &patcol[(((y + row) & 7) * 8 + ((x + col) & 7)) * bpp];
      }
      if (cca) {
        Bit32u dpx = 0;
        for (int b = 0; b < bpp; b++)
          dpx |= (Bit32u)*r9800_surf_at(&sd, a + (Bit32u)b) << (b * 8);
        int cr = r2d_ccmp_px(&cc, fg, dpx);
        if (!cr)
          continue;
        if (cr == 2) {
          for (int b = 0; b < bpp; b++)
            r2d_store(r9800_surf_at(&sd, a + (Bit32u)b), (Bit8u)((dpx ^ cc.flip) >> (b * 8)), (Bit8u)(wmask >> ((b & 3) * 8)));
          continue;
        }
      }
      for (int b = 0; b < bpp; b++) {
        Bit8u *dp = r9800_surf_at(&sd, a + (Bit32u)b);
        Bit8u sb = (fg >> ((b & 3) * 8)) & 0xff;
        Bit8u pb = patpx ? patpx[b] : ((pat >> ((b & 3) * 8)) & 0xff);
        r2d_store(dp, (Bit8u)radeon9800_rop3(rop, pb, sb, *dp), (Bit8u)(wmask >> ((b & 3) * 8)));
      }
      r2d_mark_dirty(&sd, a, bpp);
    }
  }
  r2d_surf_commit(&sd);
}

// Colour host data: pixels in the dst format, rows back-to-back
void bx_radeon9800_c::r2d_host_color_rect(int x, int y, int w, int h, const Bit8u *px, Bit32u avail)
{
  int bpp = r2d_bpp(DP_DST_DT());
  Bit8u rop = DP_ROP3();
  Bit32u wmask = dp_write_mask;
  bool aux_on = (aux_sc_cntl & R9800_AUX_SC_ENB_MASK) != 0;
  Bit32u stride = (Bit32u)w * (Bit32u)bpp;
  struct r9800_ccmp cc;
  int cca = r2d_ccmp_setup(true, bpp, bpp, &cc);
  r9800_surf_t sd;
  int cx0, cy0, cx1, cy1;

  r2d_clip(&cx0, &cy0, &cx1, &cy1);
  if (!r2d_map_span(&sd, &s2d_dst, dst_offset, dst_pitch, bpp, dst_tile, y < cy0 ? cy0 : y,
                    y + h - 1 > cy1 ? cy1 : y + h - 1, x + w - 1 > cx1 ? cx1 : x + w - 1))
    return;

  for (int row = 0; row < h; row++) {
    if ((y + row < cy0) || (y + row > cy1))
      continue;
    Bit32u addr = r2d_dst_addr(x, y + row, bpp);
    const Bit8u *srow = px + (Bit32u)row * stride;
    bool fast = !aux_on && !cca && (rop == 0xcc) && (wmask == 0xffffffff) && !dst_tile &&
                (x >= cx0) && (x + w - 1 <= cx1) && ((Bit32u)(row + 1) * stride <= avail);
    if (fast) {
      Bit8u *run = r9800_surf_run(&sd, addr, stride);
      if (run) {
        memcpy(run, srow, stride);
        r2d_mark_dirty(&sd, addr, stride);
        continue;
      }
    }
    for (int col = 0; col < w; col++) {
      if ((x + col < cx0) || (x + col > cx1))
        continue;
      if ((Bit32u)row * stride + ((Bit32u)col + 1) * bpp > avail)
        goto done;
      if (aux_on && !radeon9800_aux_sc_pass_2d(aux_sc_cntl, aux_sc_rect, x + col, y + row))
        continue;
      Bit32u a = dst_tile ? r2d_dst_addr(x + col, y + row, bpp) : addr + (Bit32u)col * bpp;
      if (cca) {
        Bit32u spx = 0, dpx = 0;
        for (int b = 0; b < bpp; b++) {
          spx |= (Bit32u)srow[(Bit32u)col * bpp + b] << (b * 8);
          dpx |= (Bit32u)*r9800_surf_at(&sd, a + (Bit32u)b) << (b * 8);
        }
        int cr = r2d_ccmp_px(&cc, spx, dpx);
        if (!cr)
          continue;
        if (cr == 2) {
          for (int b = 0; b < bpp; b++)
            r2d_store(r9800_surf_at(&sd, a + (Bit32u)b), (Bit8u)((dpx ^ cc.flip) >> (b * 8)), (Bit8u)(wmask >> ((b & 3) * 8)));
          continue;
        }
      }
      for (int b = 0; b < bpp; b++) {
        Bit8u *dp = r9800_surf_at(&sd, a + (Bit32u)b);
        Bit8u sv = srow[(Bit32u)col * bpp + b];
        Bit8u pb = (dp_brush_frgd_clr >> ((b & 3) * 8)) & 0xff;
        r2d_store(dp, (Bit8u)radeon9800_rop3(rop, pb, sv, *dp), (Bit8u)(wmask >> ((b & 3) * 8)));
      }
      if (dst_tile)
        r2d_mark_dirty(&sd, a, bpp);
    }
    if (!dst_tile)
      r2d_mark_dirty(&sd, addr, (Bit32u)w * bpp);
  }
done:
  r2d_surf_commit(&sd);
}

void bx_radeon9800_c::r2d_span_rect(int y, int h, int xl, int xr, const Bit8u *mono8x8, const Bit8u *col8x8, bool mono_la)
{
  if ((xr <= xl) || (h <= 0))
    return;
  r2d_paint_rect(xl, y, xr - xl, h, mono8x8, col8x8, mono_la);
}

// Parse the brush data block riding after the slot dwords
bool bx_radeon9800_c::r2d_brush_block(Bit32u g, const Bit32u *pl, Bit32u count, Bit32u *pp, const Bit8u **mono,
                                      const Bit8u **col, bool *mono_la, Bit32u *linepat, bool *line_en)
{
  Bit32u p = *pp;

  *mono = NULL; *col = NULL; *mono_la = false; *linepat = 0; *line_en = false;
  switch (R9800_GMC_BRUSH_TYPE(g)) {
    case 15:
      break;
    case 13:
      if (p >= count) return false;
      dp_brush_frgd_clr = pl[p++];
      break;
    case 0:
      if (p + 3 >= count) return false;
      dp_brush_bkgd_clr = pl[p++];
      dp_brush_frgd_clr = pl[p++];
      brush_data[0] = pl[p];
      brush_data[1] = pl[p + 1];
      *mono = (const Bit8u*)&pl[p];
      p += 2;
      break;
    case 1:
      if (p + 2 >= count) return false;
      dp_brush_frgd_clr = pl[p++];
      brush_data[0] = pl[p];
      brush_data[1] = pl[p + 1];
      *mono = (const Bit8u*)&pl[p];
      *mono_la = true;
      p += 2;
      break;
    case 10: {
      Bit32u nb = (64u * (Bit32u)r2d_bpp(R9800_GMC_DST_DATATYPE(g)) + 3) / 4;
      if (p + nb > count) return false;
      *col = (const Bit8u*)&pl[p];
      p += nb;
      break;
    }
    case 6:
      if (p + 2 >= count) return false;
      dp_brush_bkgd_clr = pl[p++];
      dp_brush_frgd_clr = pl[p++];
      *linepat = pl[p++];
      *line_en = true;
      break;
    case 7:
      if (p + 1 >= count) return false;
      dp_brush_frgd_clr = pl[p++];
      *linepat = pl[p++];
      *line_en = true;
      *mono_la = true;
      break;
    default:
      return false;
  }
  if (g & R9800_GMC_LD_BRUSH_Y_X) {
    if (p >= count) return false;
    brush_yx = pl[p++];
  }
  *pp = p;
  return true;
}

// ---------------------------------------------------------------------
// PM4 type-3 dispatch
// ---------------------------------------------------------------------

void bx_radeon9800_c::r2d_packet3(Bit32u hdr, const Bit32u *pl, Bit32u count)
{
  Bit32u op = R9800_PM4_T3_OPCODE(hdr);
  Bit32u g;
  Bit32u p = 1;

  if (op == R9800_PM4_OP_NOP)
    return;
  if (count < 1)
    goto unhandled;
  g = pl[0];

  switch (op) {
    case R9800_PM4_OP_CNTL_PAINT:
    case R9800_PM4_OP_CNTL_PAINT_MULTI:
    case R9800_PM4_OP_CNTL_POLYLINE:
    case R9800_PM4_OP_CNTL_POLYSCANLINES:
    {
      const Bit8u *pat = NULL;
      const Bit8u *col = NULL;
      bool mono_la, line_en;
      Bit32u linepat;

      r2d_apply_gmc(g);
      if (g & R9800_GMC_DST_PITCH_OFFSET_CNTL) {
        if (p >= count) goto unhandled;
        r2d_set_pitch_offset(true, pl[p++]);
      }
      if (g & R9800_GMC_DST_CLIPPING) {
        if (p + 1 >= count) goto unhandled;
        sc_top_left = pl[p++];
        sc_bottom_right = pl[p++];
      }
      if (!r2d_brush_block(g, pl, count, &p, &pat, &col, &mono_la, &linepat, &line_en))
        goto unhandled;
      if (op == R9800_PM4_OP_CNTL_POLYLINE) {
        int phase = 0;
        for (; p + 1 < count; p++)
          r2d_line(r9800_sx14(pl[p]), r9800_sx14(pl[p] >> 16),
                   r9800_sx14(pl[p + 1]), r9800_sx14(pl[p + 1] >> 16),
                   linepat, line_en, mono_la, &phase);
        return;
      }
      if (op == R9800_PM4_OP_CNTL_POLYSCANLINES) {
        Bit32u nent;
        if (p >= count)
          return;
        nent = pl[p++];
        for (Bit32u e = 0; (e < nent) && (p + 1 < count); e++) {
          Bit32u nspan = pl[p++];
          int yy = r9800_sx14(pl[p]);
          int hh = (pl[p] >> 16) & 0x3fff;
          p++;
          for (Bit32u sp = 0; (sp < nspan) && (p < count); sp++, p++)
            r2d_span_rect(yy, hh, r9800_sx14(pl[p]), r9800_sx14(pl[p] >> 16), pat, col, mono_la);
        }
        return;
      }
      for (; p + 1 < count; p += 2) {
        int x, y, w, h;
        if (op == R9800_PM4_OP_CNTL_PAINT) {
          x = r9800_sx14(pl[p]);
          y = r9800_sx14(pl[p] >> 16);
          w = r9800_sx14(pl[p + 1]) - x + 1;
          h = r9800_sx14(pl[p + 1] >> 16) - y + 1;
        } else {
          x = r9800_sx14(pl[p] >> 16);
          y = r9800_sx14(pl[p]);
          w = (pl[p + 1] >> 16) & 0x3fff;
          h = pl[p + 1] & 0x3fff;
        }
        r2d_paint_rect(x, y, w, h, pat, col, mono_la);
      }
      return;
    }

    case R9800_PM4_OP_CNTL_BITBLT:
    case R9800_PM4_OP_CNTL_BITBLT_MULTI:
    {
      const Bit8u *pat = NULL;
      const Bit8u *col = NULL;
      bool mono_la, line_en;
      Bit32u linepat;

      r2d_apply_gmc(g);
      if (g & R9800_GMC_SRC_PITCH_OFFSET_CNTL) {
        if (p >= count) goto unhandled;
        r2d_set_pitch_offset(false, pl[p++]);
      }
      if (g & R9800_GMC_DST_PITCH_OFFSET_CNTL) {
        if (p >= count) goto unhandled;
        r2d_set_pitch_offset(true, pl[p++]);
      }
      if (g & R9800_GMC_SRC_CLIPPING) {
        if (p >= count) goto unhandled;
        src_sc_right = pl[p] & 0x3fff;
        src_sc_bottom = (pl[p] >> 16) & 0x3fff;
        p++;
      }
      if (g & R9800_GMC_DST_CLIPPING) {
        if (p + 1 >= count) goto unhandled;
        sc_top_left = pl[p++];
        sc_bottom_right = pl[p++];
      }
      if (!r2d_brush_block(g, pl, count, &p, &pat, &col, &mono_la, &linepat, &line_en))
        goto unhandled;
      if ((count - p) % 3)
        goto unhandled;
      for (; p + 2 < count; p += 3) {
        int sx = r9800_sx14(pl[p] >> 16), sy = r9800_sx14(pl[p]);
        int dx = r9800_sx14(pl[p + 1] >> 16), dy = r9800_sx14(pl[p + 1]);
        int w = (pl[p + 2] >> 16) & 0x3fff, h = pl[p + 2] & 0x3fff;
        r2d_blit_rect_pat(sx, sy, dx, dy, w, h, pat, col, mono_la);
      }
      return;
    }

    case R9800_PM4_OP_CNTL_TRANS_BITBLT:
    {
      int sx, sy, dx, dy, w, h;
      int q = 1;

      r2d_apply_gmc(g);
      if (g & R9800_GMC_SRC_PITCH_OFFSET_CNTL) {
        if (q >= (int)count) goto unhandled;
        r2d_set_pitch_offset(false, pl[q++]);
      }
      if (g & R9800_GMC_DST_PITCH_OFFSET_CNTL) {
        if (q >= (int)count) goto unhandled;
        r2d_set_pitch_offset(true, pl[q++]);
      }
      if (g & R9800_GMC_SRC_CLIPPING) {
        if (q >= (int)count) goto unhandled;
        q++;
      }
      if (g & R9800_GMC_DST_CLIPPING) {
        if (q + 1 >= (int)count) goto unhandled;
        sc_top_left = pl[q++];
        sc_bottom_right = pl[q++];
      }
      if (q + 5 >= (int)count)
        goto unhandled;
      clr_cmp_cntl = pl[q];
      clr_cmp_clr_src = pl[q + 1];
      clr_cmp_mask = pl[q + 2];
      sx = r9800_sx14(pl[q + 3] >> 16); sy = r9800_sx14(pl[q + 3]);
      dx = r9800_sx14(pl[q + 4] >> 16); dy = r9800_sx14(pl[q + 4]);
      w = (pl[q + 5] >> 16) & 0x3fff; h = pl[q + 5] & 0x3fff;
      if ((w <= 0) || (h <= 0))
        return;
      {
        int kb = r2d_bpp(DP_DST_DT());
        struct r9800_ccmp cc;
        int cca = r2d_ccmp_setup(true, kb, kb, &cc);
        if (cca && cc.src_on && ((cc.fn_src == 4) || (cc.fn_src == 5)) && (!cc.dst_on || (cc.fn_dst == 0)))
          r2d_blit_rect_key(sx, sy, dx, dy, w, h, cc.key_src, cc.smask, cc.fn_src == 5);
        else
          r2d_blit_rect(sx, sy, dx, dy, w, h);
      }
      return;
    }

    case R9800_PM4_OP_CNTL_SMALLTEXT:
    {
      int x, y;
      r2d_apply_gmc(g);
      if (g & R9800_GMC_DST_PITCH_OFFSET_CNTL) {
        if (p >= count) goto unhandled;
        r2d_set_pitch_offset(true, pl[p++]);
      }
      if (g & R9800_GMC_DST_CLIPPING) {
        if (p + 1 >= count) goto unhandled;
        sc_top_left = pl[p++];
        sc_bottom_right = pl[p++];
      }
      if (p + 1 >= count)
        goto unhandled;
      dp_src_frgd_clr = pl[p++];
      x = r9800_sx14(pl[p]);
      y = r9800_sx14(pl[p] >> 16);
      p++;
      while (p < count) {
        Bit32u geom = pl[p++];
        int adv = (Bit8s)(geom & 0xff);
        int ascent = (Bit8s)((geom >> 8) & 0xff);
        int w = (geom >> 16) & 0xff;
        int h = (geom >> 24) & 0xff;
        Bit32u nd = ((Bit32u)w * h + 31) / 32;
        x += adv;
        if ((w == 0) || (h == 0))
          continue;
        if (count - p < nd)
          break;
        r2d_mono_rect(x, y - ascent, w, h, (const Bit8u*)&pl[p], (Bit32u)w, NULL, NULL);
        p += nd;
      }
      return;
    }

    case R9800_PM4_OP_CNTL_HOSTDATA_BLT:
    {
      int x, y, w, h;
      Bit32u nd, srcdt;
      const Bit8u *bpat = NULL;
      const Bit8u *bcol = NULL;
      bool bmono_la, bline_en;
      Bit32u blinepat;

      r2d_apply_gmc(g);
      if (g & R9800_GMC_DST_PITCH_OFFSET_CNTL) {
        if (p >= count) goto unhandled;
        r2d_set_pitch_offset(true, pl[p++]);
      }
      if (g & R9800_GMC_DST_CLIPPING) {
        if (p + 1 >= count) goto unhandled;
        sc_top_left = pl[p++];
        sc_bottom_right = pl[p++];
      }
      srcdt = R9800_GMC_SRC_DATATYPE(g);
      if (!r2d_brush_block(g, pl, count, &p, &bpat, &bcol, &bmono_la, &blinepat, &bline_en))
        goto unhandled;
      if (p + 4 >= count)
        goto unhandled;
      dp_src_frgd_clr = pl[p++];
      dp_src_bkgd_clr = pl[p++];
      x = r9800_sx14(pl[p]);
      y = r9800_sx14(pl[p] >> 16);
      h = (pl[p + 1] >> 16) & 0x3fff;
      w = pl[p + 1] & 0x3fff;
      nd = pl[p + 2];
      p += 3;
      if ((w <= 0) || (h <= 0))
        goto unhandled;
      if (srcdt == 3) {
        r2d_host_color_rect(x, y, w, h, (const Bit8u*)&pl[p], (count - p) * 4);
      } else {
        if ((count - p < nd) || ((Bit64u)nd * 4 < (Bit64u)((((Bit32u)w + 7) & ~7u) / 8) * (Bit32u)h))
          goto unhandled;
        r2d_mono_rect(x, y, w, h, (const Bit8u*)&pl[p], ((Bit32u)w + 7) & ~7u, bpat, bcol);
      }
      return;
    }

    case R9800_PM4_OP_NEXT_CHAR:
    {
      int x, y, w, h;
      Bit32u nd, srcdt;
      if (count < 2)
        goto unhandled;
      srcdt = DP_SRC_DT();
      x = r9800_sx14(pl[0]);
      y = r9800_sx14(pl[0] >> 16);
      h = (pl[1] >> 16) & 0x3fff;
      w = pl[1] & 0x3fff;
      nd = count - 2;
      if ((w <= 0) || (h <= 0))
        goto unhandled;
      if (srcdt == 3) {
        if (nd * 4 < (Bit32u)w * (Bit32u)h * (Bit32u)r2d_bpp(DP_DST_DT()))
          goto unhandled;
        r2d_host_color_rect(x, y, w, h, (const Bit8u*)&pl[2], (count - 2) * 4);
      } else {
        if ((Bit64u)nd * 32 < (Bit64u)w * (Bit64u)h)
          goto unhandled;
        r2d_mono_rect(x, y, w, h, (const Bit8u*)&pl[2], (Bit32u)w, NULL, NULL);
      }
      return;
    }

    case R9800_PM4_OP_PLY_NEXTSCAN:
    {
      int y, xl, xr;
      if (count < 2)
        goto unhandled;
      y = (int)(pl[0] & 0xffff);
      xl = (int)(pl[1] & 0xffff);
      xr = (int)((pl[1] >> 16) & 0xffff);
      if (xr <= xl)
        return;
      {
        Bit32u bt = DP_BRUSH_DT();
        bool pat_mono = (bt == 0) || (bt == 1);
        r2d_paint_rect(xl, y, xr - xl, 1, pat_mono ? (const Bit8u*)brush_data : NULL, NULL, bt == 1);
      }
      return;
    }

    case R9800_PM4_OP_SET_SCISSORS:
      if (count < 2)
        goto unhandled;
      sc_top_left = pl[0];
      sc_bottom_right = pl[1];
      return;

    case R9800_PM4_OP_LOAD_PALETTE:
      if (count >= 2) {
        Bit32u n = count - 1;
        if (n > 256) n = 256;
        for (Bit32u k = 0; k < n; k++)
          scl_palette[k] = pl[k + 1];
      }
      return;

    default:
      break;
  }

unhandled:
  BX_DEBUG(("unhandled 2D packet op=0x%02x count=%u g=0x%08x", op, count, (count >= 1) ? pl[0] : 0));
}

// ---------------------------------------------------------------------
// Register-triggered ops (direct 2D path)
// ---------------------------------------------------------------------

void bx_radeon9800_c::r2d_fill_solid(int x, int y, int w, int h, Bit32u color)
{
  int bpp = r2d_bpp(DP_DST_DT());
  Bit32u wmask = dp_write_mask;
  bool aux_on = (aux_sc_cntl & R9800_AUX_SC_ENB_MASK) != 0;
  struct r9800_ccmp cc;
  int cca;
  r9800_surf_t sd;
  int cx0, cy0, cx1, cy1;

  r2d_clip(&cx0, &cy0, &cx1, &cy1);
  if (x < cx0) { w -= cx0 - x; x = cx0; }
  if (y < cy0) { h -= cy0 - y; y = cy0; }
  if (x + w - 1 > cx1) w = cx1 - x + 1;
  if (y + h - 1 > cy1) h = cy1 - y + 1;
  if ((w <= 0) || (h <= 0))
    return;
  cca = r2d_ccmp_setup(false, bpp, bpp, &cc);
  if (!r2d_map_span(&sd, &s2d_dst, dst_offset, dst_pitch, bpp, dst_tile, y, y + h - 1, x + w - 1))
    return;
  for (int row = 0; row < h; row++) {
    for (int col = 0; col < w; col++) {
      Bit32u a = r2d_dst_addr(x + col, y + row, bpp);
      if (aux_on && !radeon9800_aux_sc_pass_2d(aux_sc_cntl, aux_sc_rect, x + col, y + row))
        continue;
      if (cca) {
        Bit32u dpx = 0;
        for (int b = 0; b < bpp; b++)
          dpx |= (Bit32u)*r9800_surf_at(&sd, a + (Bit32u)b) << (b * 8);
        if (!r2d_ccmp_px(&cc, 0, dpx))
          continue;
      }
      for (int b = 0; b < bpp; b++)
        r2d_store(r9800_surf_at(&sd, a + (Bit32u)b), (Bit8u)(color >> ((b & 3) * 8)), (Bit8u)(wmask >> ((b & 3) * 8)));
      r2d_mark_dirty(&sd, a, bpp);
    }
  }
  r2d_surf_commit(&sd);
}

// Paint the accumulated register-triggered HOST_DATA stream
void bx_radeon9800_c::r2d_hostdata_paint(void)
{
  int w = hostdata_w, h = hostdata_h;
  Bit32u have = hostdata_ndw;

  if ((w <= 0) || (h <= 0) || (have == 0))
    return;
  if (hostdata_srcdt == 3) {
    r2d_host_color_rect(hostdata_x, hostdata_y, w, h, (const Bit8u*)hostdata_buf, have * 4);
  } else {
    Bit32u dpw = ((Bit32u)w + 31) >> 5;
    Bit32u bitpitch = dpw * 32;
    if (have < dpw * (Bit32u)h)
      h = (int)(have / dpw);
    if (h > 0)
      r2d_mono_rect(hostdata_x, hostdata_y, w, h, (const Bit8u*)hostdata_buf, bitpitch, NULL, NULL);
  }
}

void bx_radeon9800_c::r2d_hostdata_word(Bit32u off, Bit32u val)
{
  if (!hostdata_active)
    return;
  // RBBM_GUICNTL HOST_DATA_SWAP
  switch (rbbm_guicntl & 3) {
    case 1: val = ((val & 0x00ff00ff) << 8) | ((val >> 8) & 0x00ff00ff); break;
    case 2: val = bx_bswap32(val); break;
    case 3: val = (val << 16) | (val >> 16); break;
    default: break;
  }
  if (hostdata_ndw < (Bit32u)(sizeof(hostdata_buf) / sizeof(hostdata_buf[0])))
    hostdata_buf[hostdata_ndw++] = val;
  if (off == R9800_HOST_DATA_LAST) {
    r2d_hostdata_paint();
    hostdata_ndw = 0;
    hostdata_active = false;
    gui_idle_event = 1;
  }
}

void bx_radeon9800_c::r2d_gui_op_run(int w, int h)
{
  Bit32u gmc = dp_gui_master_cntl;
  int src = DP_SRC_SOURCE();

  if ((w <= 0) || (h <= 0))
    return;
  if (gmc & R9800_GMC_3D_FCN_EN) {
    r2d_fill_solid(gui_dst_x, gui_dst_y, w, h, dp_brush_frgd_clr);
    return;
  }
  if (src == 2) {
    int sx = gui_src_x, sy = gui_src_y;
    int dx = gui_dst_x, dy = gui_dst_y;
    if (!(dp_cntl & R9800_DP_CNTL_DST_X_DIR)) { sx -= w - 1; dx -= w - 1; }
    if (!(dp_cntl & R9800_DP_CNTL_DST_Y_DIR)) { sy -= h - 1; dy -= h - 1; }
    Bit32u bt = DP_BRUSH_DT();
    bool pat_mono = (bt == 0) || (bt == 1);
    r2d_blit_rect_pat(sx, sy, dx, dy, w, h, pat_mono ? (const Bit8u*)brush_data : NULL, NULL, bt == 1);
  } else if (src >= 3) {
    hostdata_active = true;
    hostdata_x = gui_dst_x;
    hostdata_y = gui_dst_y;
    hostdata_w = w;
    hostdata_h = h;
    hostdata_srcdt = DP_SRC_DT();
    hostdata_ndw = 0;
  } else {
    Bit32u bt = DP_BRUSH_DT();
    bool pat_mono = (bt == 0) || (bt == 1);
    int x = gui_dst_x, y = gui_dst_y;
    if (!(dp_cntl & R9800_DP_CNTL_DST_X_DIR)) x -= w - 1;
    if (!(dp_cntl & R9800_DP_CNTL_DST_Y_DIR)) y -= h - 1;
    r2d_paint_rect(x, y, w, h, pat_mono ? (const Bit8u*)brush_data : NULL, NULL, bt == 1);
  }
}

void bx_radeon9800_c::r2d_gui_op(int w, int h)
{
  if (on_cp_thread())
    raster_flush();
  r2d_gui_op_run(w, h);
  gui_idle_event = 1;
}

// ---------------------------------------------------------------------
// Register file interface
// ---------------------------------------------------------------------

bool bx_radeon9800_c::r2d_reg_read(Bit32u off, Bit32u *val)
{
  if ((off >= R9800_BRUSH_DATA0) && (off <= R9800_BRUSH_DATA63)) {
    *val = brush_data[(off - R9800_BRUSH_DATA0) >> 2];
    return true;
  }
  if ((off >= R9800_AUX1_SC_LEFT) && (off <= R9800_AUX3_SC_BOTTOM)) {
    Bit32u idx = (off - R9800_AUX1_SC_LEFT) >> 2;
    *val = aux_sc_rect[idx >> 2][idx & 3];
    return true;
  }
  if ((off >= R9800_HOST_DATA0) && (off <= R9800_HOST_DATA_LAST)) {
    *val = 0;
    return true;
  }
  int bpp = r2d_bpp(DP_DST_DT());
  switch (off) {
    case R9800_DP_GUI_MASTER_CNTL:
    case R9800_DP_GUI_MASTER_CNTL_C: *val = dp_gui_master_cntl; return true;
    case R9800_DP_BRUSH_BKGD_CLR:  *val = dp_brush_bkgd_clr; return true;
    case R9800_DP_BRUSH_FRGD_CLR:  *val = dp_brush_frgd_clr; return true;
    case R9800_BRUSH_Y_X:          *val = brush_yx; return true;
    case R9800_BRUSH_SCALE:        *val = brush_scale; return true;
    case R9800_DP_SRC_FRGD_CLR:    *val = dp_src_frgd_clr; return true;
    case R9800_DP_SRC_BKGD_CLR:    *val = dp_src_bkgd_clr; return true;
    case R9800_CLR_CMP_CLR_SRC:    *val = clr_cmp_clr_src; return true;
    case R9800_CLR_CMP_CLR_DST:    *val = clr_cmp_clr_dst; return true;
    case R9800_CLR_CMP_CNTL:       *val = clr_cmp_cntl; return true;
    case R9800_CLR_CMP_MASK:       *val = clr_cmp_mask; return true;
    case R9800_AUX_SC_CNTL:        *val = aux_sc_cntl; return true;
    case R9800_DP_CNTL:            *val = dp_cntl; return true;
    case R9800_DP_DATATYPE:        *val = dp_datatype; return true;
    case R9800_DP_MIX:             *val = dp_mix; return true;
    case R9800_DP_WRITE_MASK:      *val = dp_write_mask; return true;
    case R9800_DP_CNTL_XDIR_YDIR_YMAJOR: *val = dp_cntl_line; return true;
    case R9800_DEFAULT_PITCH_OFFSET: *val = default_pitch_offset; return true;
    case R9800_DEFAULT_PITCH:      *val = default_pitch_reg; return true;
    case R9800_DEFAULT_SC_BOTTOM_RIGHT: *val = default_sc_bottom_right; return true;
    case R9800_SC_TOP_LEFT:
    case R9800_SC_TOP_LEFT_C:      *val = sc_top_left; return true;
    case R9800_SC_BOTTOM_RIGHT:
    case R9800_SC_BOTTOM_RIGHT_C:  *val = sc_bottom_right; return true;
    case R9800_DST_OFFSET:  *val = dst_offset; return true;
    case R9800_SRC_OFFSET:  *val = src_offset; return true;
    case R9800_DST_PITCH:   *val = bpp ? dst_pitch / bpp : dst_pitch; return true;
    case R9800_SRC_PITCH:   *val = bpp ? src_pitch / bpp : src_pitch; return true;
    case R9800_DST_PITCH_OFFSET:
    case R9800_DST_PITCH_OFFSET_C:
      *val = (dst_offset >> 10) | ((dst_pitch >> 6) << 22) | (dst_tile << 30);
      return true;
    case R9800_SRC_PITCH_OFFSET:
      *val = (src_offset >> 10) | ((src_pitch >> 6) << 22) | (src_tile << 30);
      return true;
    case R9800_DST_WIDTH:   *val = gui_dst_w; return true;
    case R9800_DST_HEIGHT:  *val = gui_dst_h; return true;
    case R9800_DST_X:       *val = (Bit32u)gui_dst_x & 0x3fff; return true;
    case R9800_DST_Y:       *val = (Bit32u)gui_dst_y & 0x3fff; return true;
    case R9800_SRC_X:       *val = (Bit32u)gui_src_x & 0x3fff; return true;
    case R9800_SRC_Y:       *val = (Bit32u)gui_src_y & 0x3fff; return true;
    case R9800_SC_LEFT:     *val = sc_top_left & 0x3fff; return true;
    case R9800_SC_TOP:      *val = (sc_top_left >> 16) & 0x3fff; return true;
    case R9800_SC_RIGHT:    *val = sc_bottom_right & 0x3fff; return true;
    case R9800_SC_BOTTOM:   *val = (sc_bottom_right >> 16) & 0x3fff; return true;
    case R9800_SRC_SC_RIGHT:  *val = src_sc_right; return true;
    case R9800_SRC_SC_BOTTOM: *val = src_sc_bottom; return true;
    case R9800_SRC_SC_BOTTOM_RIGHT: *val = src_sc_right | (src_sc_bottom << 16); return true;
    case R9800_DST_LINE_START: *val = dst_line_start; return true;
    case R9800_DST_LINE_END:   *val = dst_line_end; return true;
    case R9800_DST_LINE_PATCOUNT: *val = dst_line_patcount; return true;
    case R9800_DST_BRES_ERR:  *val = bres_err; return true;
    case R9800_DST_BRES_INC:  *val = bres_inc; return true;
    case R9800_DST_BRES_DEC:  *val = bres_dec; return true;
    case R9800_DST_BRES_LNTH:
    case R9800_DST_BRES_LNTH_SUB: *val = bres_lnth; return true;
    case R9800_DSTCACHE_CTLSTAT:
    case R9800_RB2D_DSTCACHE_CTLSTAT:
    case R9800_RB3D_DSTCACHE_CTLSTAT_LEGACY:
    case R9800_RB3D_ZCACHE_CTLSTAT_LEGACY:
      *val = 0; return true;   // caches always idle
    case R9800_RB2D_DSTCACHE_MODE:
    case R9800_RB3D_DSTCACHE_MODE_LEGACY:
    case R9800_RB3D_ZCACHE_MODE_LEGACY:
      *val = rb2d_dstcache_mode; return true;
    default:
      break;
  }
  return false;
}

bool bx_radeon9800_c::r2d_reg_write(Bit32u off, Bit32u val, Bit32u mask)
{
#define MERGE(field) ((field) = ((field) & ~mask) | (val & mask))
  if ((off >= R9800_BRUSH_DATA0) && (off <= R9800_BRUSH_DATA63)) {
    MERGE(brush_data[(off - R9800_BRUSH_DATA0) >> 2]);
    return true;
  }
  if ((off >= R9800_AUX1_SC_LEFT) && (off <= R9800_AUX3_SC_BOTTOM)) {
    Bit32u idx = (off - R9800_AUX1_SC_LEFT) >> 2;
    Bit32u *r = &aux_sc_rect[idx >> 2][idx & 3];
    *r = ((*r & ~mask) | (val & mask)) & 0x3fff;
    return true;
  }
  if ((off >= R9800_HOST_DATA0) && (off <= R9800_HOST_DATA_LAST)) {
    r2d_hostdata_word(off, val & mask);
    return true;
  }
  int bpp = r2d_bpp(DP_DST_DT());
  switch (off) {
    case R9800_DP_GUI_MASTER_CNTL:
    case R9800_DP_GUI_MASTER_CNTL_C:
      r2d_apply_gmc((dp_gui_master_cntl & ~mask) | (val & mask));
      return true;
    case R9800_SRC_PITCH_OFFSET:
      r2d_set_pitch_offset(false, val & mask);
      return true;
    case R9800_DST_PITCH_OFFSET:
    case R9800_DST_PITCH_OFFSET_C:
      r2d_set_pitch_offset(true, val & mask);
      return true;
    case R9800_SRC_Y_X:
      gui_src_x = r9800_sx14(val & mask);
      gui_src_y = r9800_sx14((val & mask) >> 16);
      return true;
    case R9800_DST_Y_X:
      gui_dst_x = r9800_sx14(val & mask);
      gui_dst_y = r9800_sx14((val & mask) >> 16);
      return true;
    case R9800_SRC_X_Y:
      gui_src_x = r9800_sx14((val & mask) >> 16);
      gui_src_y = r9800_sx14(val & mask);
      return true;
    case R9800_DST_X_Y:
      gui_dst_x = r9800_sx14((val & mask) >> 16);
      gui_dst_y = r9800_sx14(val & mask);
      return true;
    case R9800_DST_WIDTH_HEIGHT:
      gui_dst_w = ((val & mask) >> 16) & 0x3fff;
      gui_dst_h = (val & mask) & 0x3fff;
      r2d_gui_op((int)gui_dst_w, (int)gui_dst_h);
      return true;
    case R9800_DST_HEIGHT_WIDTH:
    case R9800_DST_HEIGHT_WIDTH_8:
    case R9800_DST_HEIGHT_WIDTH_BW:
      gui_dst_w = (val & mask) & 0x3fff;
      gui_dst_h = ((val & mask) >> 16) & 0x3fff;
      r2d_gui_op((int)gui_dst_w, (int)gui_dst_h);
      return true;
    case R9800_DST_OFFSET:
      dst_offset = (val & mask) & 0xfffffff0;
      return true;
    case R9800_SRC_OFFSET:
      src_offset = (val & mask) & 0xfffffff0;
      return true;
    case R9800_DST_PITCH:
      dst_pitch = ((val & mask) & 0x3fff) * (Bit32u)bpp;
      return true;
    case R9800_SRC_PITCH:
      src_pitch = ((val & mask) & 0x3fff) * (Bit32u)bpp;
      return true;
    case R9800_DST_WIDTH:  gui_dst_w = (val & mask) & 0x3fff; return true;
    case R9800_DST_HEIGHT: gui_dst_h = (val & mask) & 0x3fff; return true;
    case R9800_DST_X: gui_dst_x = r9800_sx14(val & mask); return true;
    case R9800_DST_Y: gui_dst_y = r9800_sx14(val & mask); return true;
    case R9800_SRC_X: gui_src_x = r9800_sx14(val & mask); return true;
    case R9800_SRC_Y: gui_src_y = r9800_sx14(val & mask); return true;
    case R9800_DST_X_SUB: gui_dst_x = r9800_sx14((val & mask) >> 6); return true;
    case R9800_DST_Y_SUB: gui_dst_y = r9800_sx14((val & mask) >> 6); return true;
    case R9800_SC_LEFT:
      sc_top_left = (sc_top_left & ~0x3fffu) | ((val & mask) & 0x3fff);
      return true;
    case R9800_SC_TOP:
      sc_top_left = (sc_top_left & ~(0x3fffu << 16)) | (((val & mask) & 0x3fff) << 16);
      return true;
    case R9800_SC_RIGHT:
      sc_bottom_right = (sc_bottom_right & ~0x3fffu) | ((val & mask) & 0x3fff);
      return true;
    case R9800_SC_BOTTOM:
      sc_bottom_right = (sc_bottom_right & ~(0x3fffu << 16)) | (((val & mask) & 0x3fff) << 16);
      return true;
    case R9800_SRC_SC_RIGHT:  src_sc_right = (val & mask) & 0x3fff; return true;
    case R9800_SRC_SC_BOTTOM: src_sc_bottom = (val & mask) & 0x3fff; return true;
    case R9800_SRC_SC_BOTTOM_RIGHT:
      src_sc_right = (val & mask) & 0x3fff;
      src_sc_bottom = ((val & mask) >> 16) & 0x3fff;
      return true;
    case R9800_DST_WIDTH_X:
      gui_dst_x = r9800_sx14(val & mask);
      gui_dst_w = ((val & mask) >> 16) & 0x3fff;
      r2d_gui_op((int)gui_dst_w, (int)gui_dst_h);
      return true;
    case R9800_DST_WIDTH_X_INCY:
      gui_dst_x = r9800_sx14(val & mask);
      gui_dst_w = ((val & mask) >> 16) & 0x3fff;
      r2d_gui_op((int)gui_dst_w, (int)gui_dst_h);
      gui_dst_y += (Bit32s)gui_dst_h;
      return true;
    case R9800_DST_HEIGHT_Y:
      gui_dst_y = r9800_sx14(val & mask);
      gui_dst_h = ((val & mask) >> 16) & 0x3fff;
      r2d_gui_op((int)gui_dst_w, (int)gui_dst_h);
      return true;
    case R9800_DST_LINE_START:
      MERGE(dst_line_start);
      return true;
    case R9800_DST_LINE_END: {
      MERGE(dst_line_end);
      // the write of the end point draws the line
      int phase = 0;
      Bit32u bt = DP_BRUSH_DT();
      bool pat_en = (bt == 6) || (bt == 7);
      if (on_cp_thread())
        raster_flush();
      r2d_line(r9800_sx14(dst_line_start), r9800_sx14(dst_line_start >> 16),
               r9800_sx14(dst_line_end), r9800_sx14(dst_line_end >> 16),
               brush_data[0], pat_en, bt == 7, &phase);
      gui_idle_event = 1;
      return true;
    }
    case R9800_DST_LINE_PATCOUNT: MERGE(dst_line_patcount); return true;
    case R9800_DST_BRES_ERR:  MERGE(bres_err);  return true;
    case R9800_DST_BRES_INC:  MERGE(bres_inc);  return true;
    case R9800_DST_BRES_DEC:  MERGE(bres_dec);  return true;
    case R9800_DP_CNTL_XDIR_YDIR_YMAJOR:
      MERGE(dp_cntl_line);
      return true;
    case R9800_DST_BRES_LNTH:
    case R9800_DST_BRES_LNTH_SUB:
      MERGE(bres_lnth);
      if (on_cp_thread())
        raster_flush();
      r2d_bres_line(dp_cntl_line);
      gui_idle_event = 1;
      return true;
    case R9800_DP_BRUSH_BKGD_CLR: MERGE(dp_brush_bkgd_clr); return true;
    case R9800_DP_BRUSH_FRGD_CLR: MERGE(dp_brush_frgd_clr); return true;
    case R9800_BRUSH_Y_X:         MERGE(brush_yx); return true;
    case R9800_BRUSH_SCALE:       MERGE(brush_scale); return true;
    case R9800_DP_SRC_FRGD_CLR:   MERGE(dp_src_frgd_clr); return true;
    case R9800_DP_SRC_BKGD_CLR:   MERGE(dp_src_bkgd_clr); return true;
    case R9800_CLR_CMP_CLR_SRC:   MERGE(clr_cmp_clr_src); return true;
    case R9800_CLR_CMP_CLR_DST:   MERGE(clr_cmp_clr_dst); return true;
    case R9800_CLR_CMP_CNTL:      MERGE(clr_cmp_cntl); return true;
    case R9800_CLR_CMP_MASK:      MERGE(clr_cmp_mask); return true;
    case R9800_AUX_SC_CNTL:       MERGE(aux_sc_cntl); return true;
    case R9800_DP_CNTL:
      MERGE(dp_cntl);
      // DST_TILE [4:3]: the destination tiling can be changed here too
      if (mask & 0x18) {
        Bit32u t = (dp_cntl >> R9800_DP_CNTL_DST_TILE_SHIFT) & 3;
        dst_tile = t;
      }
      return true;
    case R9800_DP_DATATYPE:       MERGE(dp_datatype); return true;
    case R9800_DP_MIX:            MERGE(dp_mix); return true;
    case R9800_DP_WRITE_MASK:     MERGE(dp_write_mask); return true;
    case R9800_DEFAULT_PITCH_OFFSET: MERGE(default_pitch_offset); return true;
    case R9800_DEFAULT_PITCH: {
      MERGE(default_pitch_reg);
      Bit32u pb = (default_pitch_reg & 0x3fff) * (Bit32u)bpp;
      default_pitch_offset = (default_pitch_offset & 0xc03fffff) | (((pb >> 6) & 0xff) << 22);
      return true;
    }
    case R9800_DEFAULT_SC_BOTTOM_RIGHT: MERGE(default_sc_bottom_right); return true;
    case R9800_SC_TOP_LEFT:
    case R9800_SC_TOP_LEFT_C:     MERGE(sc_top_left); return true;
    case R9800_SC_BOTTOM_RIGHT:
    case R9800_SC_BOTTOM_RIGHT_C: MERGE(sc_bottom_right); return true;
    case R9800_DSTCACHE_CTLSTAT:
    case R9800_RB2D_DSTCACHE_CTLSTAT:
    case R9800_RB3D_DSTCACHE_CTLSTAT_LEGACY:
    case R9800_RB3D_ZCACHE_CTLSTAT_LEGACY:
      if (on_cp_thread()) raster_flush();
      return true;
    case R9800_RB2D_DSTCACHE_MODE:
    case R9800_RB3D_DSTCACHE_MODE_LEGACY:
    case R9800_RB3D_ZCACHE_MODE_LEGACY:
      MERGE(rb2d_dstcache_mode);
      return true;
    case R9800_RE_TOP_LEFT_LEGACY:
    case R9800_RE_WIDTH_HEIGHT_LEGACY:
    case R9800_RB3D_ZPASS_DATA_LEGACY:
    case R9800_RB3D_ZPASS_ADDR_LEGACY:
      return true;
    default:
      break;
  }
#undef MERGE
  return false;
}

#endif // BX_SUPPORT_PCI && BX_SUPPORT_RADEON9800
