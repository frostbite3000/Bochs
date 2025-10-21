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

#if BX_SUPPORT_ATIRAGE

#if BX_USE_ATIRAGE_SMF
#  define BX_ATIRAGE_SMF  static
#  define BX_ATIRAGE_THIS theSvga->
#  define BX_ATIRAGE_THIS_PTR theSvga
#else
#  define BX_ATIRAGE_SMF
#  define BX_ATIRAGE_THIS this->
#  define BX_ATIRAGE_THIS_PTR this
#endif // BX_USE_ATIRAGE_SMF

// 0x3b4,0x3d4
#define VGA_CRTC_MAX 0x18
#define ATIRAGE_CRTC_MAX 0x3f
// 0x3c4
#define VGA_SEQENCER_MAX 0x04
#define CIRRUS_SEQUENCER_MAX 0x1f
// 0x3ce
#define VGA_CONTROL_MAX 0x08
#define CIRRUS_CONTROL_MAX 0x39

// Size of internal cache memory for bitblt. (must be >= 256 and 4-byte aligned)
#define CIRRUS_BLT_CACHESIZE (2048 * 4)

#define ATIRAGE_VIDEO_MEMORY_MB    4

#define ATIRAGE_VIDEO_MEMORY_KB    (ATIRAGE_VIDEO_MEMORY_MB * 1024)
#define ATIRAGE_VIDEO_MEMORY_BYTES (ATIRAGE_VIDEO_MEMORY_KB * 1024)

class bx_atirage_c : public bx_vgacore_c
{
public:
  bx_atirage_c();
  virtual ~bx_atirage_c();

  virtual bool init_vga_extension(void);
  virtual void reset(unsigned type);
  virtual void redraw_area(unsigned x0, unsigned y0,
                           unsigned width, unsigned height);
  virtual Bit8u mem_read(bx_phy_address addr);
  virtual void mem_write(bx_phy_address addr, Bit8u value);
  virtual void get_text_snapshot(Bit8u **text_snapshot,
                                 unsigned *txHeight, unsigned *txWidth);
  virtual void register_state(void);
  virtual void after_restore_state(void);

#if BX_SUPPORT_PCI
  virtual void pci_write_handler(Bit8u address, Bit32u value, unsigned io_len);
#endif
#if BX_DEBUGGER
  virtual void debug_dump(int argc, char **argv);
#endif

protected:
  virtual void update(void);

private:
  static Bit32u svga_read_handler(void *this_ptr, Bit32u address, unsigned io_len);
  static void   svga_write_handler(void *this_ptr, Bit32u address, Bit32u value, unsigned io_len);
#if !BX_USE_ATIRAGE_SMF
  Bit32u svga_read(Bit32u address, unsigned io_len);
  void   svga_write(Bit32u address, Bit32u value, unsigned io_len);
#endif
  BX_ATIRAGE_SMF void   svga_init_members();

  BX_ATIRAGE_SMF void draw_hardware_cursor(unsigned, unsigned, bx_svga_tileinfo_t *);

  // 0x3b4-0x3b5,0x3d4-0x3d5
  BX_ATIRAGE_SMF Bit8u svga_read_crtc(Bit32u address, unsigned index);
  BX_ATIRAGE_SMF void  svga_write_crtc(Bit32u address, unsigned index, Bit8u value);

  BX_ATIRAGE_SMF Bit32u register_read(Bit32u address, unsigned io_len);
  BX_ATIRAGE_SMF void  register_write(Bit32u address, Bit32u value, unsigned io_len);

  static Bit32u read_handler(void *this_ptr, Bit32u address, unsigned io_len);
  static void   write_handler(void *this_ptr, Bit32u address, Bit32u value, unsigned io_len);

  struct {
    Bit8u index;
    Bit8u reg[ATIRAGE_CRTC_MAX+1];
  } crtc; // 0x3b4-5/0x3d4-5
  struct {
    Bit8u index;
    Bit8u reg[CIRRUS_SEQUENCER_MAX+1];
  } sequencer; // 0x3c4-5
  struct {
    Bit8u index;
    Bit8u reg[CIRRUS_CONTROL_MAX+1];
    Bit8u shadow_reg0;
    Bit8u shadow_reg1;
  } control; // 0x3ce-f
  struct {
    unsigned lockindex;
    Bit8u data;
    Bit8u palette[48];
  } hidden_dac; // 0x3c6

  Bit32u ramdac[4];
  Bit32u crtc_offset;
  Bit32u crtc_dac_base;
  Bit32u clock_cntl;
  Bit32u crtc_gen_cntl;

  bool svga_unlock_special;
  bool svga_needs_update_tile;
  bool svga_needs_update_dispentire;
  bool svga_needs_update_mode;
  bool svga_double_width;

  unsigned svga_xres;
  unsigned svga_yres;
  unsigned svga_pitch;
  unsigned svga_bpp;
  unsigned svga_dispbpp;

  Bit32u bank_base[2];
  Bit32u memsize_mask;
  Bit8u *disp_ptr;
  Bit8u ext_latch[4];

  struct {
    bx_bitblt_rop_t rop_handler;
    int pixelwidth;
    int bltwidth;
    int bltheight;
    int dstpitch;
    int srcpitch;
    Bit8u bltmode;
    Bit8u bltmodeext;
    Bit8u bltrop;
    Bit8u *dst;
    Bit32u dstaddr;
    const Bit8u *src;
    Bit32u srcaddr;
#if BX_USE_ATIRAGE_SMF
    void (*bitblt_ptr)();
#else
    void (*bitblt_ptr)(void *this_ptr);
#endif
    Bit8u *memsrc_ptr; // CPU -> video
    Bit8u *memsrc_endptr;
    int memsrc_needed;
    Bit8u *memdst_ptr; // video -> CPU
    Bit8u *memdst_endptr;
    int memdst_bytesperline;
    int memdst_needed;
    Bit8u memsrc[CIRRUS_BLT_CACHESIZE];
    Bit8u memdst[CIRRUS_BLT_CACHESIZE];
  } bitblt;

  struct {
    Bit16u x, y, size;
  } hw_cursor;

  struct {
    Bit16u x, y, w, h;
  } redraw;

  bx_ddc_c ddc;

  bool is_unlocked() { return svga_unlock_special; }

  bool banking_granularity_is_16k() { return !!(control.reg[0x0B] & 0x20); }
  bool banking_is_dual() { return !!(control.reg[0x0B] & 0x01); }

#if BX_SUPPORT_PCI
  BX_ATIRAGE_SMF void svga_init_pcihandlers(void);

  BX_ATIRAGE_SMF bool atirage_mem_read_handler(bx_phy_address addr, unsigned len, void *data, void *param);
  BX_ATIRAGE_SMF bool atirage_mem_write_handler(bx_phy_address addr, unsigned len, void *data, void *param);
#endif
};

#endif // BX_SUPPORT_ATIRAGE
