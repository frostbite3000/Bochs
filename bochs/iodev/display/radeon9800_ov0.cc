/////////////////////////////////////////////////////////////////////////
// $Id$
/////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025-2026  The Bochs Project
//
//  ATI Radeon 9800 (R350) emulation: OV0 hardware video overlay window
//  and front-end scaler (register latch model, double-buffered through
//  the OV0_REG_LOAD_CNTL lock), the programmable colour space transform
//  (OV0_LIN_TRANS_A-F), the overlay gamma ramp and the DVD subpicture
//  block. The compositor scales the YUV/RGB source window into the
//  display tiles.
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

#define OV0_ACT(off) (ov0.active[R9800_OV0_REG(off)])

// ---------------------------------------------------------------------
// Latch model
// ---------------------------------------------------------------------

void bx_radeon9800_c::ov0_apply(void)
{
  memcpy(ov0.active, ov0.shadow, sizeof(ov0.active));
  ov0_update();
}

void bx_radeon9800_c::ov0_update(void)
{
  Bit32u scale = OV0_ACT(R9800_OV0_SCALE_CNTL);
  Bit32u strt = OV0_ACT(R9800_OV0_Y_X_START);
  Bit32u end = OV0_ACT(R9800_OV0_Y_X_END);
  bool was = ov0_enabled;
  int crtc_sel = (scale & R9800_OV0_SCALER_CRTC_SEL) ? 1 : 0;

  ov0_x1 = strt & 0xfff;
  ov0_y1 = (strt >> 16) & 0xfff;
  ov0_x2 = end & 0xfff;
  ov0_y2 = (end >> 16) & 0xfff;
  ov0_enabled = (scale & R9800_OV0_SCALER_ENABLE) && !(scale & R9800_OV0_SCALER_SOFT_RESET) &&
                disp_ext && (ov0_y2 > ov0_y1) && (ov0_x2 > ov0_x1) && (crtc_sel == disp_crtc);
  if (was || ov0_enabled)
    needs_update_dispentire = true;
}

bool bx_radeon9800_c::ov0_reg_read(Bit32u off, Bit32u *val)
{
  if ((off >= R9800_OV0_LIN_TRANS_A) && (off <= R9800_OV0_LIN_TRANS_F)) {
    *val = ov0_lin_trans[(off - R9800_OV0_LIN_TRANS_A) >> 2];
    return true;
  }
  if ((off >= R9800_OV0_GAMMA_000_00F) && (off <= R9800_OV0_GAMMA_040_07F)) {
    *val = ov0_gamma_regs[(off - R9800_OV0_GAMMA_000_00F) >> 2];
    return true;
  }
  if ((off >= R9800_OV0_GAMMA_380_3BF) && (off <= R9800_OV0_GAMMA_3C0_3FF)) {
    *val = ov0_gamma_regs[16 + ((off - R9800_OV0_GAMMA_380_3BF) >> 2)];
    return true;
  }
  if ((off >= R9800_OV0_GAMMA_080_0BF) && (off <= R9800_OV0_GAMMA_340_37F)) {
    *val = ov0_gamma_regs[4 + ((off - R9800_OV0_GAMMA_080_0BF) >> 2)];
    return true;
  }
  if ((off < R9800_OV0_BLOCK_BASE) || (off > R9800_OV0_BLOCK_END))
    return false;
  if (off == R9800_OV0_REG_LOAD_CNTL) {
    *val = (ov0_reg_load_cntl & R9800_OV0_LD_CTL_LOCK)
      ? (ov0_reg_load_cntl | R9800_OV0_LD_CTL_LOCK_READBACK)
      : (ov0_reg_load_cntl & ~R9800_OV0_LD_CTL_LOCK_READBACK);
    return true;
  }
  *val = ov0.shadow[R9800_OV0_REG(off)];
  return true;
}

bool bx_radeon9800_c::ov0_reg_write(Bit32u off, Bit32u val, Bit32u mask)
{
  int idx;
  if ((off >= R9800_OV0_LIN_TRANS_A) && (off <= R9800_OV0_LIN_TRANS_F)) {
    Bit32u *r = &ov0_lin_trans[(off - R9800_OV0_LIN_TRANS_A) >> 2];
    *r = (*r & ~mask) | (val & mask);
    if (ov0_enabled) needs_update_dispentire = true;
    return true;
  }
  if ((off >= R9800_OV0_GAMMA_000_00F) && (off <= R9800_OV0_GAMMA_040_07F)) {
    Bit32u *r = &ov0_gamma_regs[(off - R9800_OV0_GAMMA_000_00F) >> 2];
    *r = (*r & ~mask) | (val & mask);
    return true;
  }
  if ((off >= R9800_OV0_GAMMA_380_3BF) && (off <= R9800_OV0_GAMMA_3C0_3FF)) {
    Bit32u *r = &ov0_gamma_regs[16 + ((off - R9800_OV0_GAMMA_380_3BF) >> 2)];
    *r = (*r & ~mask) | (val & mask);
    return true;
  }
  if ((off >= R9800_OV0_GAMMA_080_0BF) && (off <= R9800_OV0_GAMMA_340_37F)) {
    Bit32u *r = &ov0_gamma_regs[4 + ((off - R9800_OV0_GAMMA_080_0BF) >> 2)];
    *r = (*r & ~mask) | (val & mask);
    return true;
  }
  if ((off < R9800_OV0_BLOCK_BASE) || (off > R9800_OV0_BLOCK_END))
    return false;
  if (off == R9800_OV0_REG_LOAD_CNTL) {
    bool was_locked = (ov0_reg_load_cntl & R9800_OV0_LD_CTL_LOCK) != 0;
    ov0_reg_load_cntl = ((ov0_reg_load_cntl & ~mask) | (val & mask)) & ~R9800_OV0_LD_CTL_LOCK_READBACK;
    if (was_locked && !(ov0_reg_load_cntl & R9800_OV0_LD_CTL_LOCK))
      ov0_apply();
    return true;
  }
  idx = R9800_OV0_REG(off);
  ov0.shadow[idx] = (ov0.shadow[idx] & ~mask) | (val & mask);
  if (!(ov0_reg_load_cntl & R9800_OV0_LD_CTL_LOCK)) {
    ov0.active[idx] = ov0.shadow[idx];
    if ((off == R9800_OV0_SCALE_CNTL) || (off == R9800_OV0_Y_X_START) || (off == R9800_OV0_Y_X_END))
      ov0_update();
    else if (ov0_enabled)
      needs_update_dispentire = true;
  }
  return true;
}

void bx_radeon9800_c::ov0_reset(void)
{
  memset(&ov0, 0, sizeof(ov0));
  memset(&subpic, 0, sizeof(subpic));
  memset(ov0_lin_trans, 0, sizeof(ov0_lin_trans));
  // identity gamma ramp: 16 entry segments with slope 0x100
  for (int i = 0; i < 18; i++) ov0_gamma_regs[i] = 0x100;
  ov0_reg_load_cntl = 0;
  ov0_enabled = false;
  ov0_x1 = ov0_y1 = ov0_x2 = ov0_y2 = 0;
}

// Once per scanout frame: commit the subpicture shadow set and repaint
// a live overlay window (its source buffers change without VRAM tracking)
void bx_radeon9800_c::ov0_frame_latch(void)
{
  Bit32u fc = crtc[disp_crtc].frame_count;
  if (subpic.frame_stamp != fc) {
    subpic.frame_stamp = fc;
    memcpy(subpic.active, subpic.regs, sizeof(subpic.active));
    memcpy(subpic.apal, subpic.pal, sizeof(subpic.apal));
  }
  if (ov0_enabled && disp_ext) {
    int x1 = ov0_x1, y1 = ov0_y1;
    int x2 = ov0_x2, y2 = ov0_y2;
    if (x2 > (int)disp_xres) x2 = disp_xres;
    if (y2 > (int)disp_yres) y2 = disp_yres;
    if ((x2 > x1) && (y2 > y1))
      redraw_area((Bit32s)x1, (Bit32s)y1, (Bit32u)(x2 - x1), (Bit32u)(y2 - y1));
  }
}

// ---------------------------------------------------------------------
// DVD subpicture block (0x0540-0x0588)
// ---------------------------------------------------------------------

bool bx_radeon9800_c::subpic_reg_read(Bit32u off, Bit32u *val)
{
  if ((off < R9800_SUBPIC_BLOCK_BASE) || (off > R9800_SUBPIC_BLOCK_END))
    return false;
  if (off == R9800_SUBPIC_PALETTE_DATA)
    *val = subpic.pal[subpic.regs[R9800_SUBPIC_REG(R9800_SUBPIC_PALETTE_INDEX)] & 0xf];
  else if (off == R9800_SUBPIC_CNTL)
    *val = subpic.regs[0] & ~R9800_SUBPIC_CNTL_NEW_FRAME;
  else
    *val = subpic.regs[R9800_SUBPIC_REG(off)];
  return true;
}

bool bx_radeon9800_c::subpic_reg_write(Bit32u off, Bit32u val, Bit32u mask)
{
  Bit32u idx;
  if ((off < R9800_SUBPIC_BLOCK_BASE) || (off > R9800_SUBPIC_BLOCK_END))
    return false;
  idx = R9800_SUBPIC_REG(off);
  subpic.regs[idx] = (subpic.regs[idx] & ~mask) | (val & mask);
  if (off == R9800_SUBPIC_PALETTE_DATA)
    subpic.pal[subpic.regs[R9800_SUBPIC_REG(R9800_SUBPIC_PALETTE_INDEX)] & 0xf] = subpic.regs[idx];
  return true;
}

// Blend the subpicture into one video pixel (YCbCr, before CSC)
void bx_radeon9800_c::subpic_blend(int out_x, int out_row, int *y, int *cb, int *cr)
{
  Bit32u *r = subpic.active;

  if (!(r[R9800_SUBPIC_REG(R9800_SUBPIC_CNTL)] & R9800_SUBPIC_CNTL_DISPLAY_EN))
    return;
  Bit32u pitch = r[R9800_SUBPIC_REG(R9800_SUBPIC_PITCH_LENGTH)] & 0xffff;
  if (!pitch)
    return;
  int su = (int)((r[R9800_SUBPIC_REG(R9800_SUBPIC_H_ACC)] + (Bit32u)out_x * r[R9800_SUBPIC_REG(R9800_SUBPIC_H_STEP)]) >> 16);
  int sv = (int)((r[R9800_SUBPIC_REG(R9800_SUBPIC_V_ACC)] + (Bit32u)out_row * r[R9800_SUBPIC_REG(R9800_SUBPIC_V_STEP)]) >> 16);
  Bit32u das = r[R9800_SUBPIC_REG(R9800_SUBPIC_DAREA_START)];
  Bit32u dae = r[R9800_SUBPIC_REG(R9800_SUBPIC_DAREA_END)];
  int x0 = das & 0x3ff, y0 = (das >> 16) & 0x3ff;
  int x1 = dae & 0x3ff, y1 = (dae >> 16) & 0x3ff;
  if ((su < x0) || (su > x1) || (sv < y0) || (sv > y1))
    return;
  Bit32u a = ov0_buf_addr(r[R9800_SUBPIC_REG(R9800_SUBPIC_PXD_A)]) + (Bit32u)sv * pitch + ((Bit32u)su >> 2);
  int cl = (BX_R9800_THIS s.memory[a & vram_mask] >> ((su & 3) * 2)) & 3;
  Bit32u cc = r[R9800_SUBPIC_REG(R9800_SUBPIC_COLOR_CONTRAST)];
  Bit32u hlt = r[R9800_SUBPIC_REG(R9800_SUBPIC_HL_TOP)];
  Bit32u hlb = r[R9800_SUBPIC_REG(R9800_SUBPIC_HL_BOTTOM)];
  if (((hlb & 0x3ff) > (hlt & 0x3ff)) && (((hlb >> 16) & 0x3ff) > ((hlt >> 16) & 0x3ff)) &&
      (su >= (int)(hlt & 0x3ff)) && (su <= (int)(hlb & 0x3ff)) &&
      (sv >= (int)((hlt >> 16) & 0x3ff)) && (sv <= (int)((hlb >> 16) & 0x3ff)))
    cc = r[R9800_SUBPIC_REG(R9800_SUBPIC_HL_COLOR_CONTRAST)];
  int k = (cc >> (cl * 4)) & 0xf;
  if (!k)
    return;
  Bit32u pe = subpic.apal[(cc >> (16 + cl * 4)) & 0xf];
  int sy = (pe >> 16) & 0xff, scb = (pe >> 8) & 0xff, scr = pe & 0xff;
  *y = (*y * (15 - k) + sy * k + 7) / 15;
  *cb = (*cb * (15 - k) + scb * k + 7) / 15;
  *cr = (*cr * (15 - k) + scr * k + 7) / 15;
}

// ---------------------------------------------------------------------
// Compositor
// ---------------------------------------------------------------------

// Overlay buffer address: the VID_BUF address field [27:4] is relative to
// OV0_BASE_ADDR (the frame buffer MC base); drivers that store the full MC
// address in the buffer register are covered by the OR.
Bit32u bx_radeon9800_c::ov0_buf_addr(Bit32u buf)
{
  Bit32u base = OV0_ACT(R9800_OV0_BASE_ADDR);
  Bit32u mc = (buf & 0x0ffffff0) | (base & 0xf0000000);
  Bit32u off;
  if ((base & 0x0fffffff) && !(buf & 0xf0000000))
    mc = base + (buf & 0x0ffffff0);
  if (!mc_is_vram(mc, &off))
    off = mc & vram_mask;
  return off;
}

static BX_CPP_INLINE Bit32u ov0_h_accum_init(Bit32u reg, Bit32u int_mask)
{
  return (((reg >> 28) & int_mask) << 20) | (reg & 0x000f8000);
}

static BX_CPP_INLINE int ov0_s15(Bit32u v)
{
  return ((Bit32s)((v & 0x7fff) << 17)) >> 17;
}

static BX_CPP_INLINE int ov0_s13(Bit32u v)
{
  return ((Bit32s)((v & 0x1fff) << 19)) >> 19;
}

// Colour space conversion: the programmable LIN_TRANS matrix (S4.11
// coefficients, offsets in half units of a 10-bit range), or ITU-R 601
// when bypassed. Output 8 bit per channel.
void bx_radeon9800_c::ov0_csc(int y, int cb, int cr, int *r, int *g, int *b)
{
  Bit32u scale = OV0_ACT(R9800_OV0_SCALE_CNTL);
  if (scale & R9800_OV0_SCALER_LIN_TRANS_BYPASS) {
    int c = y - 16, d = cb - 128, e = cr - 128;
    *r = (298 * c + 409 * e + 128) >> 8;
    *g = (298 * c - 100 * d - 208 * e + 128) >> 8;
    *b = (298 * c + 516 * d + 128) >> 8;
  } else {
    // A = {RCb [15:1], RY [31:17]}  B = {ROff [12:0], RCr [31:17]} ... (x2048)
    int ry = ov0_s15(ov0_lin_trans[0] >> 17), rcb = ov0_s15(ov0_lin_trans[0] >> 1);
    int rcr = ov0_s15(ov0_lin_trans[1] >> 17), roff = ov0_s13(ov0_lin_trans[1]);
    int gy = ov0_s15(ov0_lin_trans[2] >> 17), gcb = ov0_s15(ov0_lin_trans[2] >> 1);
    int gcr = ov0_s15(ov0_lin_trans[3] >> 17), goff = ov0_s13(ov0_lin_trans[3]);
    int by = ov0_s15(ov0_lin_trans[4] >> 17), bcb = ov0_s15(ov0_lin_trans[4] >> 1);
    int bcr = ov0_s15(ov0_lin_trans[5] >> 17), boff = ov0_s13(ov0_lin_trans[5]);
    if ((ry | rcb | rcr | gy | gcb | gcr | by | bcb | bcr) == 0) {
      int c = y - 16, d = cb - 128, e = cr - 128;
      *r = (298 * c + 409 * e + 128) >> 8;
      *g = (298 * c - 100 * d - 208 * e + 128) >> 8;
      *b = (298 * c + 516 * d + 128) >> 8;
    } else {
      // inputs are 10-bit scaled, coefficients x2048, offsets x2 on 10 bits
      int y10 = y << 2, cb10 = cb << 2, cr10 = cr << 2;
      int rr = ((ry * y10 + rcb * cb10 + rcr * cr10) >> 11) + (roff >> 1);
      int gg = ((gy * y10 + gcb * cb10 + gcr * cr10) >> 11) + (goff >> 1);
      int bb = ((by * y10 + bcb * cb10 + bcr * cr10) >> 11) + (boff >> 1);
      *r = rr >> 2; *g = gg >> 2; *b = bb >> 2;
    }
  }
  if (*r < 0) *r = 0; else if (*r > 255) *r = 255;
  if (*g < 0) *g = 0; else if (*g > 255) *g = 255;
  if (*b < 0) *b = 0; else if (*b > 255) *b = 255;
}

// Overlay gamma ramp: 18 piecewise linear segments {slope [15:0], offset
// [31:16]} over the 10-bit range; identity when the ramp holds the reset
// values or GAMMA_SEL selects the bypass curve.
int bx_radeon9800_c::ov0_gamma(int v)
{
  Bit32u scale = OV0_ACT(R9800_OV0_SCALE_CNTL);
  int sel = (scale & R9800_OV0_SCALER_GAMMA_SEL_MASK) >> R9800_OV0_SCALER_GAMMA_SEL_SHIFT;
  if (sel == 0)
    return v;
  static const int seg_start[18] = { 0x000, 0x010, 0x020, 0x040, 0x080, 0x0c0, 0x100, 0x140, 0x180,
                                     0x1c0, 0x200, 0x240, 0x280, 0x2c0, 0x300, 0x340, 0x380, 0x3c0 };
  int v10 = v << 2;
  int seg = 17;
  for (int i = 0; i < 18; i++) {
    if ((i == 17) || (v10 < seg_start[i + 1])) { seg = i; break; }
  }
  Bit32u e = ov0_gamma_regs[seg];
  int slope = e & 0xffff;
  int offset = (e >> 16) & 0xffff;
  if ((slope == 0x100) && (offset == 0))
    return v;
  int out = ((offset & 0x3ff) << 0) + (((v10 - seg_start[seg]) * slope) >> 8);
  out >>= 2;
  if (out < 0) out = 0; else if (out > 255) out = 255;
  return out;
}

static BX_CPP_INLINE int ov0_key_fn(Bit32u fn, int eq)
{
  switch (fn & 7) {
    case 1: return 1;
    case 4: return eq;
    case 5: return !eq;
    default: return 0;
  }
}

// Graphics key compares the raw framebuffer pixel, before the LUT/DAC
bool bx_radeon9800_c::ov0_gfx_key_match(int x, int y)
{
  Bit32u lo = OV0_ACT(R9800_OV0_GRPH_KEY_CLR_LOW);
  Bit32u hi = OV0_ACT(R9800_OV0_GRPH_KEY_CLR_HIGH);
  int bpb = (disp_bpp == 15) ? 2 : ((disp_bpp + 7) >> 3);
  unsigned sy = disp_dblscan ? (y >> 1) : y;
  unsigned sx = disp_hdbl ? (x >> 1) : x;
  Bit32u ma = scanout_addr(sy, sx, (disp_bpp == 4) ? (sx >> 1) : sx * (Bit32u)bpb);
  Bit32u px;
  Bit8u *vram = BX_R9800_THIS s.memory;
  switch (bpb) {
    case 1:
      px = vram[ma & vram_mask];
      // 8 bpp: the key compares the 8-bit index in every channel field
      px = px | (px << 8) | (px << 16);
      break;
    case 2:
      px = vram[ma & vram_mask] | ((Bit32u)vram[(ma + 1) & vram_mask] << 8);
      if (disp_bpp == 16)
        px = (((px >> 11) & 0x1f) << 19) | (((px >> 5) & 0x3f) << 10) | ((px & 0x1f) << 3);
      else
        px = (((px >> 10) & 0x1f) << 19) | (((px >> 5) & 0x1f) << 11) | ((px & 0x1f) << 3);
      break;
    default:
      px = vram[ma & vram_mask] | ((Bit32u)vram[(ma + 1) & vram_mask] << 8) | ((Bit32u)vram[(ma + 2) & vram_mask] << 16);
      break;
  }
  // per channel range compare
  for (int sh = 0; sh < 24; sh += 8) {
    Bit32u c = (px >> sh) & 0xff, l = (lo >> sh) & 0xff, h = (hi >> sh) & 0xff;
    if ((c < l) || (c > h))
      return false;
  }
  return true;
}

// Composite the overlay window into one display tile (or the snapshot)
void bx_radeon9800_c::draw_overlay(unsigned xc, unsigned yc, bx_svga_tileinfo_t *info)
{
  if (!ov0_enabled || disp_dac_const || disp_blank)
    return;

  Bit32u scale = OV0_ACT(R9800_OV0_SCALE_CNTL);
  int fmt = (scale & R9800_OV0_SCALER_FORMAT_MASK) >> R9800_OV0_SCALER_FORMAT_SHIFT;
  Bit8u *vram = BX_R9800_THIS s.memory;
  Bit32u vmask = vram_mask;

  switch (fmt) {
    case R9800_OV0_FMT_15BPP:
    case R9800_OV0_FMT_16BPP:
    case R9800_OV0_FMT_32BPP:
    case R9800_OV0_FMT_YUV9:
    case R9800_OV0_FMT_YUV12:
    case R9800_OV0_FMT_VYUY422:
    case R9800_OV0_FMT_YVYU422:
    case R9800_OV0_FMT_AYUV444:
      break;
    default:
      return;
  }

  unsigned w, h;
  Bit8u *tile_ptr;
  if (info->snapshot_mode) {
    tile_ptr = bx_gui->get_snapshot_buffer();
    w = disp_xres;
    h = disp_yres;
  } else {
    tile_ptr = bx_gui->graphics_tile_get(xc, yc, &w, &h);
  }
  if (tile_ptr == NULL)
    return;
  if (info->bpp == 15) info->bpp = 16;
  unsigned tbytes = info->bpp / 8;

  int x1 = ov0_x1, x2 = ov0_x2, y1 = ov0_y1, y2 = ov0_y2;
  if (x2 > (int)disp_xres) x2 = disp_xres;
  if (y2 > (int)disp_yres) y2 = disp_yres;
  int tx0 = (int)xc, ty0 = (int)yc, tx1 = (int)(xc + w), ty1 = (int)(yc + h);
  int cx0 = x1 > tx0 ? x1 : tx0;
  int cy0 = y1 > ty0 ? y1 : ty0;
  int cx1 = x2 < tx1 ? x2 : tx1;
  int cy1 = y2 < ty1 ? y2 : ty1;
  if ((cx0 >= cx1) || (cy0 >= cy1))
    return;

  // Scale factors
  Bit32u h_inc_reg = OV0_ACT(R9800_OV0_H_INC);
  Bit32u step_reg = OV0_ACT(R9800_OV0_STEP_BY);
  Bit32u v_inc = OV0_ACT(R9800_OV0_V_INC);
  int ecp_div = (pll_regs[R9800_PLL_VCLK_ECP_CNTL] >> R9800_ECP_DIV_SHIFT) & 3;
  int step_y = step_reg & 0xff;
  int step_c = (step_reg >> 8) & 0xff;
  if (step_y < 1) step_y = 1;
  if (step_c < 1) step_c = 1;
  Bit32u h_step_y = ((h_inc_reg & 0xffff) << (step_y - 1)) << (8 - ecp_div);
  Bit32u h_step_c = ((h_inc_reg >> 16) << (step_c - 1)) << (8 - ecp_div);
  Bit32u h_acc0_y = ov0_h_accum_init(OV0_ACT(R9800_OV0_P1_H_ACCUM_INIT), 0xf);
  Bit32u h_acc0_c = ov0_h_accum_init(OV0_ACT(R9800_OV0_P23_H_ACCUM_INIT), 0x7);
  Bit32u h_bias_y = ((0x28000 + ((h_inc_reg & 0xffff) << 3)) & 0xff800) << 4;
  Bit32u h_bias_c = ((0x28000 + (((h_inc_reg >> 16) & 0xffff) << 3)) & 0xff800) << 4;
  Bit32u h_rel0_y = (h_acc0_y > h_bias_y) ? h_acc0_y - h_bias_y : 0;
  Bit32u h_rel0_c = (h_acc0_c > h_bias_c) ? h_acc0_c - h_bias_c : 0;
  Bit32u p1_se = OV0_ACT(R9800_OV0_P1_X_START_END);
  Bit32u p2_se = OV0_ACT(R9800_OV0_P2_X_START_END);
  int p1_x0 = (p1_se >> 16) & 0x7f;
  int p1_xe = p1_se & 0xfff;
  int p2_x0 = (p2_se >> 16) & 0x7f;
  int p2_xe = p2_se & 0xfff;
  Bit32u v_acc0_y = OV0_ACT(R9800_OV0_P1_V_ACCUM_INIT) & 0x03ff8000;
  Bit32u v_acc0_c = OV0_ACT(R9800_OV0_P23_V_ACCUM_INIT) & 0x01ff8000;
  int src_h_y = ((OV0_ACT(R9800_OV0_P1_BLANK_LINES_AT_TOP) >> 16) & 0xfff) + 1;
  int src_h_c = ((OV0_ACT(R9800_OV0_P23_BLANK_LINES_AT_TOP) >> 16) & 0x7ff) + 1;
  bool nearest_h = (scale & R9800_OV0_SCALER_PICK_NEAREST_H) != 0;
  bool nearest_v = (scale & R9800_OV0_SCALER_PICK_NEAREST_V) != 0;
  int chroma_shift_v = (fmt == R9800_OV0_FMT_YUV9) ? 2 : 1;

  int buf_n = OV0_ACT(R9800_OV0_AUTO_FLIP_CNTL) & 7;
  if (buf_n > 5) buf_n = 5;
  Bit32u buf0 = ov0.active[R9800_OV0_REG(R9800_OV0_VID_BUF0_BASE_ADRS) + buf_n];
  Bit32u buf1 = ov0.active[R9800_OV0_REG(R9800_OV0_VID_BUF0_BASE_ADRS) + ((buf_n < 4) ? buf_n + 1 : 5)];
  Bit32u buf2 = ov0.active[R9800_OV0_REG(R9800_OV0_VID_BUF0_BASE_ADRS) + ((buf_n < 3) ? buf_n + 2 : 5)];
  Bit32u pitch0 = OV0_ACT(R9800_OV0_VID_BUF_PITCH0_VALUE) & 0xfffff;
  Bit32u pitch1 = OV0_ACT(R9800_OV0_VID_BUF_PITCH1_VALUE) & 0xfffff;
  Bit32u abuf0 = ov0_buf_addr(buf0), abuf1 = ov0_buf_addr(buf1), abuf2 = ov0_buf_addr(buf2);

  Bit32u colour_cntl = OV0_ACT(R9800_OV0_COLOUR_CNTL);
  int bright = (int)((Bit8s)((colour_cntl & 0x7f) << 1)) >> 1;
  int sat_u = (colour_cntl >> 8) & 0x1f;
  int sat_v = (colour_cntl >> 16) & 0x1f;
  Bit32u key_cntl = OV0_ACT(R9800_OV0_KEY_CNTL);
  Bit32u vkey_fn = key_cntl & R9800_OV0_VIDEO_KEY_FN_MASK;
  Bit32u gkey_fn = (key_cntl & R9800_OV0_GRAPHIC_KEY_FN_MASK) >> R9800_OV0_GRAPHIC_KEY_FN_SHIFT;
  Bit32u vkey_lo = OV0_ACT(R9800_OV0_VID_KEY_CLR_LOW);
  Bit32u vkey_hi = OV0_ACT(R9800_OV0_VID_KEY_CLR_HIGH);
  bool mix_and = (key_cntl & R9800_OV0_CMP_MIX_AND) != 0;
  bool sp_on = (subpic.active[R9800_SUBPIC_REG(R9800_SUBPIC_CNTL)] & R9800_SUBPIC_CNTL_DISPLAY_EN) != 0;
  bool signed_uv = (scale & R9800_OV0_SCALER_SIGNED_UV) != 0;

  for (int y = cy0; y < cy1; y++) {
    int out_row = y - y1;
    Bit32u vacc = v_acc0_y + (Bit32u)out_row * v_inc;
    int src_y = (int)((vacc - 0x180000) >> 20);
    Bit32u vfrac = nearest_v ? 0 : ((vacc >> 12) & 0xff);
    if (src_y < 0) src_y = 0;
    if (src_y > src_h_y - 1) src_y = src_h_y - 1;
    int src_y2 = (src_y + 1 < src_h_y) ? src_y + 1 : src_y;
    int src_yc = (int)((v_acc0_c + (Bit32u)out_row * (v_inc >> chroma_shift_v) - 0x180000) >> 20);
    if (src_yc < 0) src_yc = 0;
    if (src_yc > src_h_c - 1) src_yc = src_h_c - 1;
    Bit32u py = (buf0 & 1) ? pitch1 : pitch0;
    Bit32u base_y = abuf0 + (Bit32u)src_y * py;
    Bit32u base_y2 = abuf0 + (Bit32u)src_y2 * py;
    Bit32u base_u = abuf1 + (Bit32u)src_yc * ((buf1 & 1) ? pitch1 : pitch0);
    Bit32u base_v = abuf2 + (Bit32u)src_yc * ((buf2 & 1) ? pitch1 : pitch0);
    Bit8u *tp = tile_ptr + info->pitch * (y - ty0) + tbytes * (cx0 - tx0);

    for (int x = cx0; x < cx1; x++) {
      int i = x - x1;
      Bit32u hacc = h_rel0_y + (Bit32u)i * h_step_y;
      int sx = p1_x0 + (int)(hacc >> 20);
      Bit32u hfrac = nearest_h ? 0 : ((hacc >> 12) & 0xff);
      int sxc;
      Bit32u rgb, vpx = 0;
      int yy = 0, cb = 0, cr = 0;
      int r = 0, g = 0, b = 0;

      if (sx < 0) sx = 0;
      if (sx > p1_xe) sx = p1_xe;
      int sx2 = (sx < p1_xe) ? sx + 1 : sx;
      switch (fmt) {
        case R9800_OV0_FMT_YUV9:
        case R9800_OV0_FMT_YUV12: {
          int cshift = (fmt == R9800_OV0_FMT_YUV9) ? 2 : 1;
          sxc = p2_x0 + (int)((h_rel0_c + (Bit32u)i * h_step_c) >> 20);
          if (sxc < 0) sxc = 0;
          if (sxc > p2_xe) sxc = p2_xe;
          // bilinear luma
          int y00 = vram[(base_y + sx) & vmask], y01 = vram[(base_y + sx2) & vmask];
          int y10 = vram[(base_y2 + sx) & vmask], y11 = vram[(base_y2 + sx2) & vmask];
          int yt = (y00 * (256 - (int)hfrac) + y01 * (int)hfrac) >> 8;
          int yb = (y10 * (256 - (int)hfrac) + y11 * (int)hfrac) >> 8;
          yy = (yt * (256 - (int)vfrac) + yb * (int)vfrac) >> 8;
          cb = vram[(base_u + sxc) & vmask];
          cr = vram[(base_v + sxc) & vmask];
          UNUSED(cshift);
          vpx = ((Bit32u)cr << 16) | ((Bit32u)cb << 8) | (Bit32u)yy;
          break;
        }
        case R9800_OV0_FMT_VYUY422:
          yy = vram[(base_y + sx * 2) & vmask];
          cb = vram[(base_y + (sx & ~1) * 2 + 1) & vmask];
          cr = vram[(base_y + (sx & ~1) * 2 + 3) & vmask];
          if (!nearest_h && (sx2 != sx)) {
            int yn = vram[(base_y + sx2 * 2) & vmask];
            yy = (yy * (256 - (int)hfrac) + yn * (int)hfrac) >> 8;
          }
          vpx = ((Bit32u)cr << 16) | ((Bit32u)cb << 8) | (Bit32u)yy;
          break;
        case R9800_OV0_FMT_YVYU422:
          yy = vram[(base_y + sx * 2 + 1) & vmask];
          cb = vram[(base_y + (sx & ~1) * 2) & vmask];
          cr = vram[(base_y + (sx & ~1) * 2 + 2) & vmask];
          if (!nearest_h && (sx2 != sx)) {
            int yn = vram[(base_y + sx2 * 2 + 1) & vmask];
            yy = (yy * (256 - (int)hfrac) + yn * (int)hfrac) >> 8;
          }
          vpx = ((Bit32u)cr << 16) | ((Bit32u)cb << 8) | (Bit32u)yy;
          break;
        case R9800_OV0_FMT_AYUV444: {
          Bit32u a = (base_y + sx * 4) & vmask;
          vpx = vram[a] | ((Bit32u)vram[(a + 1) & vmask] << 8) | ((Bit32u)vram[(a + 2) & vmask] << 16) | ((Bit32u)vram[(a + 3) & vmask] << 24);
          cr = vpx & 0xff; cb = (vpx >> 8) & 0xff; yy = (vpx >> 16) & 0xff;
          break;
        }
        case R9800_OV0_FMT_15BPP:
        case R9800_OV0_FMT_16BPP:
          vpx = vram[(base_y + sx * 2) & vmask] | ((Bit32u)vram[(base_y + sx * 2 + 1) & vmask] << 8);
          break;
        default: {
          Bit32u a = (base_y + sx * 4) & vmask;
          vpx = vram[a] | ((Bit32u)vram[(a + 1) & vmask] << 8) | ((Bit32u)vram[(a + 2) & vmask] << 16) | ((Bit32u)vram[(a + 3) & vmask] << 24);
          break;
        }
      }
      switch (fmt) {
        case R9800_OV0_FMT_15BPP:
          rgb = (((vpx >> 10) & 0x1f) << 19) | (((vpx >> 10) & 0x1c) << 14) |
                (((vpx >> 5) & 0x1f) << 11) | (((vpx >> 5) & 0x1c) << 6) |
                ((vpx & 0x1f) << 3) | ((vpx & 0x1c) >> 2);
          break;
        case R9800_OV0_FMT_16BPP:
          rgb = (((vpx >> 11) & 0x1f) << 19) | (((vpx >> 11) & 0x1c) << 14) |
                (((vpx >> 5) & 0x3f) << 10) | (((vpx >> 5) & 0x30) << 4) |
                ((vpx & 0x1f) << 3) | ((vpx & 0x1c) >> 2);
          break;
        case R9800_OV0_FMT_32BPP:
          rgb = vpx & 0xffffff;
          break;
        default:
          if (signed_uv) { cb ^= 0x80; cr ^= 0x80; }
          yy = yy + bright;
          if (yy < 0) yy = 0; else if (yy > 255) yy = 255;
          cb = 128 + (((cb - 128) * sat_u) >> 4);
          cr = 128 + (((cr - 128) * sat_v) >> 4);
          if (sp_on)
            subpic_blend(i, out_row, &yy, &cb, &cr);
          ov0_csc(yy, cb, cr, &r, &g, &b);
          r = ov0_gamma(r); g = ov0_gamma(g); b = ov0_gamma(b);
          rgb = ((Bit32u)r << 16) | ((Bit32u)g << 8) | (Bit32u)b;
          break;
      }
      int gcmp = ov0_key_fn(gkey_fn, ov0_gfx_key_match(x, y) ? 1 : 0);
      int veq = 1;
      for (int sh = 0; sh < 24; sh += 8) {
        Bit32u c = (vpx >> sh) & 0xff, l = (vkey_lo >> sh) & 0xff, hh = (vkey_hi >> sh) & 0xff;
        if ((c < l) || (c > hh)) { veq = 0; break; }
      }
      int vcmp = ov0_key_fn(vkey_fn, veq);
      bool show_gfx = mix_and ? (gcmp && vcmp) : (gcmp || vcmp);
      if (!show_gfx) {
        if (info->is_indexed) {
          tp[0] = (Bit8u)rgb;
        } else {
          Bit32u colour = MAKE_COLOUR(rgb, 24, info->red_shift, info->red_mask,
                                      rgb, 16, info->green_shift, info->green_mask,
                                      rgb, 8, info->blue_shift, info->blue_mask);
          if (info->is_little_endian) {
            for (int k = 0; k < info->bpp; k += 8) *(tp + k / 8) = (Bit8u)(colour >> k);
          } else {
            for (int k = info->bpp - 8, j = 0; k > -8; k -= 8, j++) *(tp + j) = (Bit8u)(colour >> k);
          }
        }
      }
      tp += tbytes;
    }
  }
}

#endif // BX_SUPPORT_PCI && BX_SUPPORT_RADEON9800
