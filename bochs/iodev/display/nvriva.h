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

#if BX_SUPPORT_NVRIVA

#if BX_USE_NVRIVA_SMF
#  define BX_NVRIVA_SMF  static
#  define BX_NVRIVA_THIS theSvga->
#  define BX_NVRIVA_THIS_PTR theSvga
#else
#  define BX_NVRIVA_SMF
#  define BX_NVRIVA_THIS this->
#  define BX_NVRIVA_THIS_PTR this
#endif // BX_USE_NVRIVA_SMF

// 0x3b4,0x3d4
#define VGA_CRTC_MAX 0x18
#define NVRIVA_CRTC_MAX 0x9F

#define NVRIVA_CHANNEL_COUNT 32
#define NVRIVA_SUBCHANNEL_COUNT 8

// NV04_3D XML register definitions
#define NV04_CONTEXT_SURFACES_3D_DMA_NOTIFY          0x00000180
#define NV04_CONTEXT_SURFACES_3D_DMA_COLOR           0x00000184
#define NV04_CONTEXT_SURFACES_3D_DMA_ZETA            0x00000188
#define NV04_CONTEXT_SURFACES_3D_CLIP_HORIZONTAL     0x000002f8
#define NV04_CONTEXT_SURFACES_3D_CLIP_VERTICAL       0x000002fc
#define NV04_CONTEXT_SURFACES_3D_FORMAT              0x00000300
#define NV04_CONTEXT_SURFACES_3D_PITCH               0x00000308
#define NV04_CONTEXT_SURFACES_3D_OFFSET_COLOR        0x0000030c
#define NV04_CONTEXT_SURFACES_3D_OFFSET_ZETA         0x00000310

// NV04_3D Textured Triangle definitions
#define NV04_TEXTURED_TRIANGLE_DMA_NOTIFY            0x00000180
#define NV04_TEXTURED_TRIANGLE_DMA_A                 0x00000184
#define NV04_TEXTURED_TRIANGLE_DMA_B                 0x00000188
#define NV04_TEXTURED_TRIANGLE_COLORKEY              0x00000300
#define NV04_TEXTURED_TRIANGLE_OFFSET                0x00000304
#define NV04_TEXTURED_TRIANGLE_FORMAT                0x00000308
#define NV04_TEXTURED_TRIANGLE_FILTER                0x0000030c
#define NV04_TEXTURED_TRIANGLE_BLEND                 0x00000310
#define NV04_TEXTURED_TRIANGLE_CONTROL                0x00000314
#define NV04_TEXTURED_TRIANGLE_FOGCOLOR              0x00000318
#define NV04_TEXTURED_TRIANGLE_BLEND_FOG_ENABLE      0x00010000

// NV04_TEXTURED_TRIANGLE_BLEND constants
#define NV04_TEXTURED_TRIANGLE_BLEND_TEXTURE_MAP__MASK        0x0000000f
#define NV04_TEXTURED_TRIANGLE_BLEND_TEXTURE_MAP__SHIFT       0
#define NV04_TEXTURED_TRIANGLE_BLEND_TEXTURE_MAP_DECAL         0x00000001
#define NV04_TEXTURED_TRIANGLE_BLEND_TEXTURE_MAP_MODULATE     0x00000002
#define NV04_TEXTURED_TRIANGLE_BLEND_TEXTURE_MAP_DECALALPHA   0x00000003
#define NV04_TEXTURED_TRIANGLE_BLEND_TEXTURE_MAP_MODULATEALPHA 0x00000004
#define NV04_TEXTURED_TRIANGLE_BLEND_TEXTURE_MAP_DECALMASK     0x00000005
#define NV04_TEXTURED_TRIANGLE_BLEND_TEXTURE_MAP_MODULATEMASK  0x00000006
#define NV04_TEXTURED_TRIANGLE_BLEND_TEXTURE_MAP_COPY          0x00000007
#define NV04_TEXTURED_TRIANGLE_BLEND_TEXTURE_MAP_ADD           0x00000008
#define NV04_TEXTURED_TRIANGLE_BLEND_MASK_BIT__MASK            0x00000030
#define NV04_TEXTURED_TRIANGLE_BLEND_MASK_BIT__SHIFT           4
#define NV04_TEXTURED_TRIANGLE_BLEND_MASK_BIT_LSB             0x00000010
#define NV04_TEXTURED_TRIANGLE_BLEND_MASK_BIT_MSB             0x00000020
#define NV04_TEXTURED_TRIANGLE_BLEND_SHADE_MODE__MASK          0x000000c0
#define NV04_TEXTURED_TRIANGLE_BLEND_SHADE_MODE__SHIFT         6
#define NV04_TEXTURED_TRIANGLE_BLEND_SHADE_MODE_FLAT           0x00000040
#define NV04_TEXTURED_TRIANGLE_BLEND_SHADE_MODE_GOURAUD       0x00000080
#define NV04_TEXTURED_TRIANGLE_BLEND_SHADE_MODE_PHONG         0x000000c0
#define NV04_TEXTURED_TRIANGLE_BLEND_TEXTURE_PERSPECTIVE_ENABLE 0x00000100
#define NV04_TEXTURED_TRIANGLE_BLEND_SPECULAR_ENABLE          0x00001000
#define NV04_TEXTURED_TRIANGLE_BLEND_BLEND_ENABLE             0x00100000
#define NV04_TEXTURED_TRIANGLE_BLEND_SRC__MASK                0x0f000000
#define NV04_TEXTURED_TRIANGLE_BLEND_SRC__SHIFT               24
#define NV04_TEXTURED_TRIANGLE_BLEND_SRC_ZERO                 0x01000000
#define NV04_TEXTURED_TRIANGLE_BLEND_SRC_ONE                  0x02000000
#define NV04_TEXTURED_TRIANGLE_BLEND_SRC_SRC_COLOR            0x03000000
#define NV04_TEXTURED_TRIANGLE_BLEND_SRC_ONE_MINUS_SRC_COLOR   0x04000000
#define NV04_TEXTURED_TRIANGLE_BLEND_SRC_SRC_ALPHA            0x05000000
#define NV04_TEXTURED_TRIANGLE_BLEND_SRC_ONE_MINUS_SRC_ALPHA  0x06000000
#define NV04_TEXTURED_TRIANGLE_BLEND_SRC_DST_ALPHA            0x07000000
#define NV04_TEXTURED_TRIANGLE_BLEND_SRC_ONE_MINUS_DST_ALPHA  0x08000000
#define NV04_TEXTURED_TRIANGLE_BLEND_SRC_DST_COLOR            0x09000000
#define NV04_TEXTURED_TRIANGLE_BLEND_SRC_ONE_MINUS_DST_COLOR  0x0a000000
#define NV04_TEXTURED_TRIANGLE_BLEND_SRC_SRC_ALPHA_SATURATE   0x0b000000
#define NV04_TEXTURED_TRIANGLE_BLEND_DST__MASK                0xf0000000
#define NV04_TEXTURED_TRIANGLE_BLEND_DST__SHIFT               28
#define NV04_TEXTURED_TRIANGLE_BLEND_DST_ZERO                 0x10000000
#define NV04_TEXTURED_TRIANGLE_BLEND_DST_ONE                  0x20000000
#define NV04_TEXTURED_TRIANGLE_BLEND_DST_SRC_COLOR            0x30000000
#define NV04_TEXTURED_TRIANGLE_BLEND_DST_ONE_MINUS_SRC_COLOR  0x40000000
#define NV04_TEXTURED_TRIANGLE_BLEND_DST_SRC_ALPHA            0x50000000
#define NV04_TEXTURED_TRIANGLE_BLEND_DST_ONE_MINUS_SRC_ALPHA  0x60000000
#define NV04_TEXTURED_TRIANGLE_BLEND_DST_DST_ALPHA            0x70000000
#define NV04_TEXTURED_TRIANGLE_BLEND_DST_ONE_MINUS_DST_ALPHA  0x80000000
#define NV04_TEXTURED_TRIANGLE_BLEND_DST_DST_COLOR            0x90000000
#define NV04_TEXTURED_TRIANGLE_BLEND_DST_ONE_MINUS_DST_COLOR  0xa0000000
#define NV04_TEXTURED_TRIANGLE_BLEND_DST_SRC_ALPHA_SATURATE   0xb0000000

// NV04_TEXTURED_TRIANGLE_FILTER constants
#define NV04_TEXTURED_TRIANGLE_FILTER_KERNEL_SIZE_X__MASK     0x000000ff
#define NV04_TEXTURED_TRIANGLE_FILTER_KERNEL_SIZE_X__SHIFT    0
#define NV04_TEXTURED_TRIANGLE_FILTER_KERNEL_SIZE_Y__MASK     0x00007f00
#define NV04_TEXTURED_TRIANGLE_FILTER_KERNEL_SIZE_Y__SHIFT    8
#define NV04_TEXTURED_TRIANGLE_FILTER_MIPMAP_DITHER_ENABLE    0x00008000
#define NV04_TEXTURED_TRIANGLE_FILTER_MIPMAP_LODBIAS__MASK    0x00ff0000
#define NV04_TEXTURED_TRIANGLE_FILTER_MIPMAP_LODBIAS__SHIFT   16
#define NV04_TEXTURED_TRIANGLE_FILTER_MINIFY__MASK            0x07000000
#define NV04_TEXTURED_TRIANGLE_FILTER_MINIFY__SHIFT           24
#define NV04_TEXTURED_TRIANGLE_FILTER_MINIFY_NEAREST          0x01000000
#define NV04_TEXTURED_TRIANGLE_FILTER_MINIFY_LINEAR           0x02000000
#define NV04_TEXTURED_TRIANGLE_FILTER_MINIFY_NEAREST_MIPMAP_NEAREST  0x03000000
#define NV04_TEXTURED_TRIANGLE_FILTER_MINIFY_LINEAR_MIPMAP_NEAREST   0x04000000
#define NV04_TEXTURED_TRIANGLE_FILTER_MINIFY_NEAREST_MIPMAP_LINEAR   0x05000000
#define NV04_TEXTURED_TRIANGLE_FILTER_MINIFY_LINEAR_MIPMAP_LINEAR    0x06000000
#define NV04_TEXTURED_TRIANGLE_FILTER_ANISOTROPIC_MINIFY_ENABLE      0x08000000
#define NV04_TEXTURED_TRIANGLE_FILTER_MAGNIFY__MASK           0x70000000
#define NV04_TEXTURED_TRIANGLE_FILTER_MAGNIFY__SHIFT          28
#define NV04_TEXTURED_TRIANGLE_FILTER_MAGNIFY_NEAREST         0x10000000
#define NV04_TEXTURED_TRIANGLE_FILTER_MAGNIFY_LINEAR          0x20000000
#define NV04_TEXTURED_TRIANGLE_FILTER_ANISOTROPIC_MAGNIFY_ENABLE     0x80000000

// NV04_TEXTURED_TRIANGLE_COLORKEY constants
#define NV04_TEXTURED_TRIANGLE_COLORKEY_B__MASK              0x000000ff
#define NV04_TEXTURED_TRIANGLE_COLORKEY_B__SHIFT             0
#define NV04_TEXTURED_TRIANGLE_COLORKEY_G__MASK              0x0000ff00
#define NV04_TEXTURED_TRIANGLE_COLORKEY_G__SHIFT             8
#define NV04_TEXTURED_TRIANGLE_COLORKEY_R__MASK              0x00ff0000
#define NV04_TEXTURED_TRIANGLE_COLORKEY_R__SHIFT             16
#define NV04_TEXTURED_TRIANGLE_COLORKEY_A__MASK              0xff000000
#define NV04_TEXTURED_TRIANGLE_COLORKEY_A__SHIFT             24

// NV04_TEXTURED_TRIANGLE_CONTROL constants
#define NV04_TEXTURED_TRIANGLE_CONTROL_ALPHA_REF__MASK       0x000000ff
#define NV04_TEXTURED_TRIANGLE_CONTROL_ALPHA_REF__SHIFT      0
#define NV04_TEXTURED_TRIANGLE_CONTROL_ALPHA_FUNC__MASK      0x00000f00
#define NV04_TEXTURED_TRIANGLE_CONTROL_ALPHA_FUNC__SHIFT     8
#define NV04_TEXTURED_TRIANGLE_CONTROL_ALPHA_FUNC_NEVER      0x00000100
#define NV04_TEXTURED_TRIANGLE_CONTROL_ALPHA_FUNC_LESS       0x00000200
#define NV04_TEXTURED_TRIANGLE_CONTROL_ALPHA_FUNC_EQUAL      0x00000300
#define NV04_TEXTURED_TRIANGLE_CONTROL_ALPHA_FUNC_LEQUAL     0x00000400
#define NV04_TEXTURED_TRIANGLE_CONTROL_ALPHA_FUNC_GREATER    0x00000500
#define NV04_TEXTURED_TRIANGLE_CONTROL_ALPHA_FUNC_NOTEQUAL   0x00000600
#define NV04_TEXTURED_TRIANGLE_CONTROL_ALPHA_FUNC_GEQUAL     0x00000700
#define NV04_TEXTURED_TRIANGLE_CONTROL_ALPHA_FUNC_ALWAYS     0x00000800
#define NV04_TEXTURED_TRIANGLE_CONTROL_ALPHA_ENABLE           0x00001000
#define NV04_TEXTURED_TRIANGLE_CONTROL_ORIGIN__MASK          0x00002000
#define NV04_TEXTURED_TRIANGLE_CONTROL_ORIGIN__SHIFT        13
#define NV04_TEXTURED_TRIANGLE_CONTROL_ORIGIN_CENTER         0x00000000
#define NV04_TEXTURED_TRIANGLE_CONTROL_ORIGIN_CORNER         0x00002000
#define NV04_TEXTURED_TRIANGLE_CONTROL_Z_ENABLE              0x00004000
#define NV04_TEXTURED_TRIANGLE_CONTROL_Z_FUNC__MASK          0x000f0000
#define NV04_TEXTURED_TRIANGLE_CONTROL_Z_FUNC__SHIFT        16
#define NV04_TEXTURED_TRIANGLE_CONTROL_Z_FUNC_NEVER          0x00010000
#define NV04_TEXTURED_TRIANGLE_CONTROL_Z_FUNC_LESS           0x00020000
#define NV04_TEXTURED_TRIANGLE_CONTROL_Z_FUNC_EQUAL          0x00030000
#define NV04_TEXTURED_TRIANGLE_CONTROL_Z_FUNC_LEQUAL         0x00040000
#define NV04_TEXTURED_TRIANGLE_CONTROL_Z_FUNC_GREATER        0x00050000
#define NV04_TEXTURED_TRIANGLE_CONTROL_Z_FUNC_NOTEQUAL       0x00060000
#define NV04_TEXTURED_TRIANGLE_CONTROL_Z_FUNC_GEQUAL         0x00070000
#define NV04_TEXTURED_TRIANGLE_CONTROL_Z_FUNC_ALWAYS         0x00080000
#define NV04_TEXTURED_TRIANGLE_CONTROL_CULL_MODE__MASK       0x00300000
#define NV04_TEXTURED_TRIANGLE_CONTROL_CULL_MODE__SHIFT      20
#define NV04_TEXTURED_TRIANGLE_CONTROL_CULL_MODE_BOTH        0x00000000
#define NV04_TEXTURED_TRIANGLE_CONTROL_CULL_MODE_NONE        0x00100000
#define NV04_TEXTURED_TRIANGLE_CONTROL_CULL_MODE_CW          0x00200000
#define NV04_TEXTURED_TRIANGLE_CONTROL_CULL_MODE_CCW         0x00300000
#define NV04_TEXTURED_TRIANGLE_CONTROL_DITHER_ENABLE         0x00400000
#define NV04_TEXTURED_TRIANGLE_CONTROL_Z_PERSPECTIVE_ENABLE  0x00800000
#define NV04_TEXTURED_TRIANGLE_CONTROL_Z_WRITE               0x01000000
#define NV04_TEXTURED_TRIANGLE_CONTROL_Z_FORMAT__MASK        0xc0000000
#define NV04_TEXTURED_TRIANGLE_CONTROL_Z_FORMAT__SHIFT       30
#define NV04_TEXTURED_TRIANGLE_CONTROL_Z_FORMAT_FIXED        0x40000000
#define NV04_TEXTURED_TRIANGLE_CONTROL_Z_FORMAT_FLOAT        0x80000000
#define NVRIVA_CACHE1_SIZE 64

#define BX_ROP_PATTERN 0x01

struct nv04_channel
{
  Bit32u subr_return;
  bool subr_active;
  struct {
    Bit32u mthd;
    Bit32u subc;
    Bit32u mcnt;
    bool ni;
  } dma_state;
  struct {
    Bit32u object;
    Bit8u engine;
    Bit32u notifier;
  } schs[NVRIVA_SUBCHANNEL_COUNT];

  bool notify_pending;
  Bit32u notify_type;

  bool s2d_locked;
  Bit32u s2d_img_src;
  Bit32u s2d_img_dst;
  Bit32u s2d_color_fmt;
  Bit32u s2d_color_bytes;
  Bit32u s2d_pitch_src;
  Bit32u s2d_pitch_dst;
  Bit32u s2d_ofs_src;
  Bit32u s2d_ofs_dst;

  Bit32u swzs_img_obj;
  Bit32u swzs_fmt;
  Bit32u swzs_color_bytes;
  Bit32u swzs_width;
  Bit32u swzs_height;
  Bit32u swzs_ofs;

  bool ifc_color_key_enable;
  bool ifc_clip_enable;
  Bit32u ifc_operation;
  Bit32u ifc_color_fmt;
  Bit32u ifc_color_bytes;
  Bit32u ifc_pixels_per_word;
  Bit32u ifc_x;
  Bit32u ifc_y;
  Bit32u ifc_ofs_x;
  Bit32u ifc_ofs_y;
  Bit32u ifc_draw_offset;
  Bit32u ifc_redraw_offset;
  Bit32u ifc_dst_width;
  Bit32u ifc_dst_height;
  Bit32u ifc_src_width;
  Bit32u ifc_src_height;
  Bit32u ifc_clip_x0;
  Bit32u ifc_clip_y0;
  Bit32u ifc_clip_x1;
  Bit32u ifc_clip_y1;

  Bit32u iifc_palette;
  Bit32u iifc_palette_ofs;
  Bit32u iifc_operation;
  Bit32u iifc_color_fmt;
  Bit32u iifc_color_bytes;
  Bit32u iifc_bpp4;
  Bit32u iifc_yx;
  Bit32u iifc_dhw;
  Bit32u iifc_shw;
  Bit32u iifc_words_ptr;
  Bit32u iifc_words_left;
  Bit32u* iifc_words;

  Bit32u sifc_operation;
  Bit32u sifc_color_fmt;
  Bit32u sifc_color_bytes;
  Bit32u sifc_shw;
  Bit32u sifc_dxds;
  Bit32u sifc_dydt;
  Bit32u sifc_clip_yx;
  Bit32u sifc_clip_hw;
  Bit32u sifc_syx;
  Bit32u sifc_words_ptr;
  Bit32u sifc_words_left;
  Bit32u* sifc_words;

  bool blit_color_key_enable;
  Bit32u blit_operation;
  Bit32u blit_syx;
  Bit32u blit_dyx;
  Bit32u blit_hw;

  bool tfc_swizzled;
  Bit32u tfc_color_fmt;
  Bit32u tfc_color_bytes;
  Bit32u tfc_yx;
  Bit32u tfc_hw;
  Bit32u tfc_clip_wx;
  Bit32u tfc_clip_hy;
  Bit32u tfc_words_ptr;
  Bit32u tfc_words_left;
  Bit32u* tfc_words;
  bool tfc_upload;
  Bit32u tfc_upload_offset;

  Bit32u sifm_src;
  bool sifm_swizzled;
  Bit32u sifm_operation;
  Bit32u sifm_color_fmt;
  Bit32u sifm_color_bytes;
  Bit32u sifm_syx;
  Bit32u sifm_dyx;
  Bit32u sifm_shw;
  Bit32u sifm_dhw;
  Bit32s sifm_dudx;
  Bit32s sifm_dvdy;
  Bit32u sifm_sfmt;
  Bit32u sifm_sofs;

  Bit32u m2mf_src;
  Bit32u m2mf_dst;
  Bit32u m2mf_src_offset;
  Bit32u m2mf_dst_offset;
  Bit32u m2mf_src_pitch;
  Bit32u m2mf_dst_pitch;
  Bit32u m2mf_line_length;
  Bit32u m2mf_line_count;
  Bit32u m2mf_format;
  Bit32u m2mf_buffer_notify;

  Bit32u d3d_a_obj;
  Bit32u d3d_b_obj;
  Bit32u d3d_color_obj;
  Bit32u d3d_zeta_obj;
  Bit32u d3d_vertex_a_obj;
  Bit32u d3d_vertex_b_obj;
  Bit32u d3d_report_obj;
  Bit32u d3d_clip_horizontal;
  Bit32u d3d_clip_vertical;
  Bit32u d3d_surface_format;
  Bit32u d3d_color_bytes;
  Bit32u d3d_depth_bytes;
  Bit32u d3d_surface_pitch_a;
  Bit32u d3d_surface_pitch_z;
  Bit16s d3d_window_offset_x;
  Bit16s d3d_window_offset_y;
  Bit32u d3d_surface_color_offset;
  Bit32u d3d_surface_zeta_offset;
  Bit32u d3d_alpha_test_enable;
  Bit32u d3d_alpha_func;
  Bit32u d3d_alpha_ref;
  Bit32u d3d_blend_enable;
  Bit16u d3d_blend_sfactor_rgb;
  Bit16u d3d_blend_sfactor_alpha;
  Bit16u d3d_blend_dfactor_rgb;
  Bit16u d3d_blend_dfactor_alpha;
  Bit16u d3d_blend_equation_rgb;
  Bit16u d3d_blend_equation_alpha;
  float d3d_blend_color[4];
  Bit32u d3d_cull_face_enable;
  Bit32u d3d_depth_test_enable;
  Bit32u d3d_depth_write_enable;
  Bit32u d3d_lighting_enable;
  Bit32u d3d_depth_func;
  Bit32u d3d_shade_mode;
  float d3d_clip_min;
  float d3d_clip_max;
  Bit32u d3d_cull_face;
  Bit32u d3d_front_face;
  Bit32u d3d_light_enable_mask;
  float d3d_inverse_model_view_matrix[12];
  float d3d_composite_matrix[16];
  Bit32u d3d_scissor_x;
  Bit32u d3d_scissor_width;
  Bit32u d3d_scissor_y;
  Bit32u d3d_scissor_height;
  Bit32u d3d_shader_program;
  Bit32u d3d_shader_obj;
  Bit32u d3d_shader_offset;
  float d3d_scene_ambient_color[4];
  Bit32u d3d_viewport_horizontal;
  Bit32u d3d_viewport_vertical;
  float d3d_viewport_offset[4];
  float d3d_viewport_scale[4];
  Bit32u d3d_transform_program[544][4];
  float d3d_transform_constant[512][4];
  float d3d_light_diffuse_color[8][3];
  float d3d_light_infinite_direction[8][3];
  float d3d_normal[3];
  float d3d_diffuse_color[4];
  float d3d_texcoord[4][4];
  Bit32u d3d_vertex_data_array_offset[16];
  Bit32u d3d_vertex_data_array_format_type[16];
  Bit32u d3d_vertex_data_array_format_size[16];
  Bit32u d3d_vertex_data_array_format_stride[16];
  bool d3d_vertex_data_array_format_dx[16];
  Bit32u d3d_begin_end;
  bool d3d_primitive_done;
  bool d3d_triangle_flip;
  Bit32u d3d_vertex_index;
  Bit32u d3d_attrib_index;
  Bit32u d3d_comp_index;
  float d3d_vertex_data[4][16][4];
  Bit32u d3d_index_array_offset;
  Bit32u d3d_index_array_dma;
  Bit32u d3d_texture_offset[16];
  Bit32u d3d_texture_format[16];
  Bit32u d3d_texture_address[16];
  Bit32u d3d_texture_control0[16];
  Bit32u d3d_texture_control1[16];
  Bit32u d3d_texture_filter[16];
  Bit32u d3d_texture_image_rect[16];
  Bit32u d3d_texture_palette[16];
  Bit32u d3d_texture_control3[16];
  Bit32u d3d_semaphore_obj;
  Bit32u d3d_semaphore_offset;
  Bit32u d3d_zstencil_clear_value;
  Bit32u d3d_color_clear_value;
  Bit32u d3d_clear_surface;
  Bit32u d3d_transform_execution_mode;
  Bit32u d3d_transform_program_load;
  Bit32u d3d_transform_program_start;
  Bit32u d3d_transform_constant_load;
  Bit32u d3d_attrib_color;
  Bit32u d3d_dci;
  Bit32u d3d_attrib_tex_coord[10];
  Bit32u d3d_tex_coord_count;
  
  // NV04_TEXTURED_TRIANGLE_FOGCOLOR support
  Bit32u d3d_fog_color;
  Bit8u d3d_fog_color_r;
  Bit8u d3d_fog_color_g;
  Bit8u d3d_fog_color_b;
  Bit8u d3d_fog_color_a;
  bool d3d_fog_enable;
  
  // NV04_TEXTURED_TRIANGLE_CONTROL support
  Bit32u d3d_control;
  Bit8u d3d_alpha_ref;
  Bit8u d3d_alpha_func;
  bool d3d_alpha_enable;
  bool d3d_origin_center;
  bool d3d_z_enable;
  Bit8u d3d_z_func;
  Bit8u d3d_cull_mode;
  bool d3d_dither_enable;
  bool d3d_z_perspective_enable;
  bool d3d_z_write;
  Bit8u d3d_z_format;
  
  // NV04_TEXTURED_TRIANGLE_BLEND support
  Bit32u d3d_blend;
  Bit8u d3d_texture_map;
  Bit8u d3d_mask_bit;
  Bit8u d3d_shade_mode;
  bool d3d_texture_perspective_enable;
  bool d3d_specular_enable;
  bool d3d_blend_enable;
  Bit8u d3d_blend_src;
  Bit8u d3d_blend_dst;
  
  // NV04_TEXTURED_TRIANGLE_FILTER support
  Bit32u d3d_filter;
  Bit8u d3d_kernel_size_x;
  Bit8u d3d_kernel_size_y;
  bool d3d_mipmap_dither_enable;
  Bit8u d3d_mipmap_lodbias;
  Bit8u d3d_minify_filter;
  Bit8u d3d_magnify_filter;
  bool d3d_anisotropic_minify_enable;
  bool d3d_anisotropic_magnify_enable;
  
  // NV04_TEXTURED_TRIANGLE_OFFSET support
  Bit32u d3d_offset;
  Bit32u d3d_texture_offset;
  
  // NV04_TEXTURED_TRIANGLE_COLORKEY support
  Bit32u d3d_colorkey;
  Bit8u d3d_colorkey_r;
  Bit8u d3d_colorkey_g;
  Bit8u d3d_colorkey_b;
  Bit8u d3d_colorkey_a;
  bool d3d_colorkey_enable;

  Bit8u  rop;

  Bit32u beta;

  Bit16u clip_x;
  Bit16u clip_y;
  Bit16u clip_width;
  Bit16u clip_height;

  Bit32u chroma_color_fmt;
  Bit32u chroma_color;

  Bit32u patt_shape;
  Bit32u patt_type;
  Bit32u patt_bg_color;
  Bit32u patt_fg_color;
  bool patt_data_mono[64];
  Bit32u patt_data_color[64];

  Bit32u gdi_operation;
  Bit32u gdi_color_fmt;
  Bit32u gdi_mono_fmt;
  Bit32u gdi_clip_yx0;
  Bit32u gdi_clip_yx1;
  Bit32u gdi_rect_color;
  Bit32u gdi_rect_xy;
  Bit32u gdi_rect_yx0;
  Bit32u gdi_rect_yx1;
  Bit32u gdi_rect_wh;
  Bit32u gdi_bg_color;
  Bit32u gdi_fg_color;
  Bit32u gdi_image_swh;
  Bit32u gdi_image_dwh;
  Bit32u gdi_image_xy;
  Bit32u gdi_words_ptr;
  Bit32u gdi_words_left;
  Bit32u* gdi_words;
};

class bx_nvriva_c : public bx_vgacore_c
{
public:
  bx_nvriva_c();
  virtual ~bx_nvriva_c();

  virtual bool init_vga_extension(void);
  Bit16u get_crtc_vtotal();
  virtual void get_crtc_params(bx_crtc_params_t* crtcp, Bit32u* vclock);
  virtual void reset(unsigned type);
  virtual void redraw_area(unsigned x0, unsigned y0,
                           unsigned width, unsigned height);
  // doubled coordinates in low resolution modes
  void redraw_area_d(Bit32s x0, Bit32s y0, Bit32u width, Bit32u height);
  // not doubled coordinates in low resolution modes
  void redraw_area_nd(Bit32s x0, Bit32s y0, Bit32u width, Bit32u height);
  void redraw_area_nd(Bit32u offset, Bit32u width, Bit32u height);
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
#if !BX_USE_CIRRUS_SMF
  Bit32u svga_read(Bit32u address, unsigned io_len);
  void   svga_write(Bit32u address, Bit32u value, unsigned io_len);
#endif
  BX_NVRIVA_SMF void   svga_init_members();
  BX_NVRIVA_SMF void   bitblt_init();

  BX_NVRIVA_SMF void draw_hardware_cursor(unsigned, unsigned, bx_svga_tileinfo_t *);

  // 0x3b4-0x3b5,0x3d4-0x3d5
  BX_NVRIVA_SMF Bit8u svga_read_crtc(Bit32u address, unsigned index);
  BX_NVRIVA_SMF void  svga_write_crtc(Bit32u address, unsigned index, Bit8u value);

  BX_NVRIVA_SMF void set_irq_level(bool level);
  BX_NVRIVA_SMF Bit32u get_mc_intr();
  BX_NVRIVA_SMF void update_irq_level();

  BX_NVRIVA_SMF Bit8u register_read8(Bit32u address);
  BX_NVRIVA_SMF void  register_write8(Bit32u address, Bit8u value);
  BX_NVRIVA_SMF Bit32u register_read32(Bit32u address);
  BX_NVRIVA_SMF void  register_write32(Bit32u address, Bit32u value);

  BX_NVRIVA_SMF Bit8u vram_read8(Bit32u address);
  BX_NVRIVA_SMF Bit16u vram_read16(Bit32u address);
  BX_NVRIVA_SMF Bit32u vram_read32(Bit32u address);
  BX_NVRIVA_SMF void vram_write8(Bit32u address, Bit8u value);
  BX_NVRIVA_SMF void vram_write16(Bit32u address, Bit16u value);
  BX_NVRIVA_SMF void vram_write32(Bit32u address, Bit32u value);
  BX_NVRIVA_SMF void vram_write64(Bit32u address, Bit64u value);
  BX_NVRIVA_SMF Bit8u ramin_read8(Bit32u address);
  BX_NVRIVA_SMF Bit32u ramin_read32(Bit32u address);
  BX_NVRIVA_SMF void ramin_write8(Bit32u address, Bit8u value);
  BX_NVRIVA_SMF void ramin_write32(Bit32u address, Bit32u value);
  BX_NVRIVA_SMF Bit8u physical_read8(Bit32u address);
  BX_NVRIVA_SMF Bit16u physical_read16(Bit32u address);
  BX_NVRIVA_SMF Bit32u physical_read32(Bit32u address);
  BX_NVRIVA_SMF void physical_write8(Bit32u address, Bit8u value);
  BX_NVRIVA_SMF void physical_write16(Bit32u address, Bit16u value);
  BX_NVRIVA_SMF void physical_write32(Bit32u address, Bit32u value);
  BX_NVRIVA_SMF void physical_write64(Bit32u address, Bit64u value);
  BX_NVRIVA_SMF Bit8u dma_read8(Bit32u object, Bit32u address);
  BX_NVRIVA_SMF Bit16u dma_read16(Bit32u object, Bit32u address);
  BX_NVRIVA_SMF Bit32u dma_read32(Bit32u object, Bit32u address);
  BX_NVRIVA_SMF void dma_write8(Bit32u object, Bit32u address, Bit8u value);
  BX_NVRIVA_SMF void dma_write16(Bit32u object, Bit32u address, Bit16u value);
  BX_NVRIVA_SMF void dma_write32(Bit32u object, Bit32u address, Bit32u value);
  BX_NVRIVA_SMF void dma_write64(Bit32u object, Bit32u address, Bit64u value);
  BX_NVRIVA_SMF void dma_copy(Bit32u dst_obj, Bit32u dst_addr,
    Bit32u src_obj, Bit32u src_addr, Bit32u byte_count);
  BX_NVRIVA_SMF Bit32u dma_pt_lookup(Bit32u object, Bit32u address);
  BX_NVRIVA_SMF Bit32u dma_lin_lookup(Bit32u object, Bit32u address);
  BX_NVRIVA_SMF Bit32u ramfc_address(Bit32u chid, Bit32u offset);
  BX_NVRIVA_SMF void ramfc_write32(Bit32u chid, Bit32u offset, Bit32u value);
  BX_NVRIVA_SMF Bit32u ramfc_read32(Bit32u chid, Bit32u offset);

  BX_NVRIVA_SMF Bit64u get_current_time();

  BX_NVRIVA_SMF void ramht_lookup(Bit32u handle, Bit32u chid, Bit32u* object, Bit8u* engine);

  BX_NVRIVA_SMF void fifo_process(Bit32u chid);
  BX_NVRIVA_SMF bool execute_command(Bit32u chid, Bit32u subc, Bit32u method, Bit32u param);

  BX_NVRIVA_SMF void update_color_bytes_ifc(nv04_channel* ch);
  BX_NVRIVA_SMF void update_color_bytes_sifc(nv04_channel* ch);
  BX_NVRIVA_SMF void update_color_bytes_tfc(nv04_channel* ch);
  BX_NVRIVA_SMF void update_color_bytes_iifc(nv04_channel* ch);
  BX_NVRIVA_SMF void update_color_bytes(Bit32u s2d_color_fmt, Bit32u color_fmt, Bit32u* color_bytes);

  BX_NVRIVA_SMF void execute_clip(nv04_channel* ch, Bit32u method, Bit32u param);
  BX_NVRIVA_SMF void execute_m2mf(nv04_channel* ch, Bit32u subc, Bit32u method, Bit32u param);
  BX_NVRIVA_SMF void execute_rop(nv04_channel* ch, Bit32u method, Bit32u param);
  BX_NVRIVA_SMF void execute_patt(nv04_channel* ch, Bit32u method, Bit32u param);
  BX_NVRIVA_SMF void execute_gdi(nv04_channel* ch, Bit32u method, Bit32u param);
  BX_NVRIVA_SMF void execute_swzsurf(nv04_channel* ch, Bit32u method, Bit32u param);
  BX_NVRIVA_SMF void execute_chroma(nv04_channel* ch, Bit32u method, Bit32u param);
  BX_NVRIVA_SMF void execute_imageblit(nv04_channel* ch, Bit32u method, Bit32u param);
  BX_NVRIVA_SMF void execute_ifc(nv04_channel* ch, Bit32u method, Bit32u param);
  BX_NVRIVA_SMF void execute_surf2d(nv04_channel* ch, Bit32u method, Bit32u param);
  BX_NVRIVA_SMF void execute_iifc(nv04_channel* ch, Bit32u method, Bit32u param);
  BX_NVRIVA_SMF void execute_sifc(nv04_channel* ch, Bit32u method, Bit32u param);
  BX_NVRIVA_SMF void execute_beta(nv04_channel* ch, Bit32u method, Bit32u param);
  BX_NVRIVA_SMF void execute_tfc(nv04_channel* ch, Bit32u method, Bit32u param);
  BX_NVRIVA_SMF void execute_sifm(nv04_channel* ch, Bit32u method, Bit32u param);
  BX_NVRIVA_SMF void execute_d3d(nv04_channel* ch, Bit32u cls, Bit32u method, Bit32u param);

  BX_NVRIVA_SMF Bit32u get_pixel(Bit32u obj, Bit32u ofs, Bit32u x, Bit32u cb);
  BX_NVRIVA_SMF void put_pixel(nv04_channel* ch, Bit32u ofs, Bit32u x, Bit32u value);
  BX_NVRIVA_SMF void put_pixel_swzs(nv04_channel* ch, Bit32u ofs, Bit32u value);
  BX_NVRIVA_SMF void pixel_operation(nv04_channel* ch, Bit32u op,
    Bit32u* dstcolor, const Bit32u* srccolor, Bit32u cb, Bit32u px, Bit32u py);

  BX_NVRIVA_SMF void gdi_fillrect(nv04_channel* ch, bool clipped);
  BX_NVRIVA_SMF void gdi_blit(nv04_channel* ch, Bit32u type);
  BX_NVRIVA_SMF void ifc(nv04_channel* ch, Bit32u word);
  BX_NVRIVA_SMF void iifc(nv04_channel* ch);
  BX_NVRIVA_SMF void sifc(nv04_channel* ch);
  BX_NVRIVA_SMF void copyarea(nv04_channel* ch);
  BX_NVRIVA_SMF void tfc(nv04_channel* ch);
  BX_NVRIVA_SMF void m2mf(nv04_channel* ch);
  BX_NVRIVA_SMF void sifm(nv04_channel* ch);

  BX_NVRIVA_SMF bool d3d_scissor_clip(nv04_channel* ch, Bit32u* x, Bit32u* y, Bit32u* width, Bit32u* height);
  BX_NVRIVA_SMF void d3d_clear_surface(nv04_channel* ch);
  BX_NVRIVA_SMF void d3d_sample_texture(nv04_channel* ch,
    Bit32u tex_unit, float str[3], float color[4]);
  BX_NVRIVA_SMF void d3d_vertex_shader(nv04_channel* ch, float in[16][4], float out[16][4]);
  BX_NVRIVA_SMF void d3d_pixel_shader(nv04_channel* ch, float in[16][4], float tmp_regs16[64][4], float tmp_regs32[64][4]);
  BX_NVRIVA_SMF void d3d_triangle(nv04_channel* ch, Bit32u base);
  BX_NVRIVA_SMF void d3d_triangle_clipped(nv04_channel* ch, float v0[16][4], float v1[16][4], float v2[16][4]);
  BX_NVRIVA_SMF void d3d_clip_to_screen(nv04_channel* ch, float pos_clip[4], float pos_screen[4]);
  BX_NVRIVA_SMF void d3d_process_vertex(nv04_channel* ch);
  BX_NVRIVA_SMF void d3d_load_vertex(nv04_channel* ch, Bit32u index);
  BX_NVRIVA_SMF Bit32u d3d_get_surface_pitch_z(nv04_channel* ch);
  BX_NVRIVA_SMF void d3d_semaphore_notify(nv04_channel* ch, Bit32u value);
  BX_NVRIVA_SMF bool d3d_semaphore_wait(nv04_channel* ch, Bit32u expected_value);
  BX_NVRIVA_SMF void d3d_apply_fog_color(nv04_channel* ch, Bit32u* color);
  BX_NVRIVA_SMF Bit32u d3d_blend_fog_color(nv04_channel* ch, Bit32u original_color, float fog_factor);
  BX_NVRIVA_SMF bool d3d_alpha_test(nv04_channel* ch, Bit8u alpha);
  BX_NVRIVA_SMF bool d3d_z_test(nv04_channel* ch, float z_value, float z_buffer_value);
  BX_NVRIVA_SMF bool d3d_cull_triangle(nv04_channel* ch, float v0[4], float v1[4], float v2[4]);
  BX_NVRIVA_SMF Bit32u d3d_texture_blend(nv04_channel* ch, Bit32u tex_color, Bit32u vertex_color);
  BX_NVRIVA_SMF Bit32u d3d_alpha_blend(nv04_channel* ch, Bit32u src_color, Bit32u dst_color);
  BX_NVRIVA_SMF Bit32u d3d_get_blend_factor(nv04_channel* ch, Bit8u factor, Bit32u src_color, Bit32u dst_color);
  BX_NVRIVA_SMF Bit32u d3d_sample_texture_filtered(nv04_channel* ch, Bit32u tex_obj, float u, float v, float lod);
  BX_NVRIVA_SMF Bit32u d3d_sample_texture_nearest(nv04_channel* ch, Bit32u tex_obj, float u, float v, float lod);
  BX_NVRIVA_SMF Bit32u d3d_sample_texture_linear(nv04_channel* ch, Bit32u tex_obj, float u, float v, float lod);
  BX_NVRIVA_SMF Bit32u d3d_sample_texture_mipmap(nv04_channel* ch, Bit32u tex_obj, float u, float v, float lod);
  BX_NVRIVA_SMF Bit32u d3d_get_texture_offset(nv04_channel* ch);
  BX_NVRIVA_SMF bool d3d_is_colorkey(nv04_channel* ch, Bit32u color);
  BX_NVRIVA_SMF Bit32u d3d_apply_colorkey(nv04_channel* ch, Bit32u tex_color);

  struct {
    Bit8u index;
    Bit8u reg[NVRIVA_CRTC_MAX+1];
  } crtc; // 0x3b4-5/0x3d4-5

  Bit32u mc_intr_en;
  Bit32u mc_enable;
  Bit32u bus_intr;
  Bit32u bus_intr_en;
  Bit32u fifo_intr;
  Bit32u fifo_intr_en;
  Bit32u fifo_ramht;
  Bit32u fifo_ramfc;
  Bit32u fifo_ramro;
  Bit32u fifo_mode;
  Bit32u fifo_cache1_push1;
  Bit32u fifo_cache1_put;
  Bit32u fifo_cache1_dma_push;
  Bit32u fifo_cache1_dma_instance;
  Bit32u fifo_cache1_dma_put;
  Bit32u fifo_cache1_dma_get;
  Bit32u fifo_cache1_ref_cnt;
  Bit32u fifo_cache1_pull0;
  Bit32u fifo_cache1_semaphore;
  Bit32u fifo_cache1_get;
  Bit32u fifo_grctx_instance;
  Bit32u fifo_cache1_method[NVRIVA_CACHE1_SIZE];
  Bit32u fifo_cache1_data[NVRIVA_CACHE1_SIZE];
  Bit32u rma_addr;
  Bit32u timer_intr;
  Bit32u timer_intr_en;
  Bit32u timer_num;
  Bit32u timer_den;
  Bit64u timer_inittime1;
  Bit64u timer_inittime2;
  Bit32u timer_alarm;
  Bit32u straps0_primary;
  Bit32u straps0_primary_original;
  Bit32u graph_intr;
  Bit32u graph_nsource;
  Bit32u graph_intr_en;
  Bit32u graph_ctx_switch1;
  Bit32u graph_ctx_switch2;
  Bit32u graph_ctx_switch4;
  Bit32u graph_ctxctl_cur;
  Bit32u graph_status;
  Bit32u graph_trapped_addr;
  Bit32u graph_trapped_data;
  Bit32u graph_notify;
  Bit32u graph_fifo;
  Bit32u graph_channel_ctx_table;
  Bit32u graph_offset0;
  Bit32u graph_pitch0;
  Bit32u crtc_intr;
  Bit32u crtc_intr_en;
  Bit32u crtc_start;
  Bit32u crtc_config;
  Bit32u crtc_cursor_offset;
  Bit32u crtc_cursor_config;
  Bit32u ramdac_cu_start_pos;
  Bit32u ramdac_vpll;
  Bit32u ramdac_vpll_b;
  Bit32u ramdac_pll_select;
  Bit32u ramdac_general_control;

  bx_bitblt_rop_t rop_handler[0x100];
  Bit8u rop_flags[0x100];

  bool acquire_active;

  nv04_channel chs[NVRIVA_CHANNEL_COUNT];

  Bit32u unk_regs[4*1024*1024]; // temporary

  bool svga_unlock_special;
  bool svga_needs_update_tile;
  bool svga_needs_update_dispentire;
  bool svga_needs_update_mode;
  bool svga_double_width;

  Bit8u devfunc;
  unsigned svga_xres;
  unsigned svga_yres;
  unsigned svga_pitch;
  unsigned svga_bpp;
  unsigned svga_dispbpp;

  Bit32u card_type;
  Bit32u memsize_mask;
  Bit32u bar2_size;
  Bit32u ramin_flip;
  Bit32u class_mask;

  Bit8u *disp_ptr;
  Bit32u disp_offset;
  Bit32u bank_base[2];

  struct {
    Bit32u offset;
    Bit16s x, y;
    Bit8u size;
    bool bpp32;
    bool enabled;
  } hw_cursor;

  bx_ddc_c ddc;

  bool is_unlocked() { return svga_unlock_special; }

  BX_NVRIVA_SMF void svga_init_pcihandlers(void);

  void vertical_timer();

  BX_NVRIVA_SMF bool nvriva_mem_read_handler(bx_phy_address addr, unsigned len, void *data, void *param);
  BX_NVRIVA_SMF bool nvriva_mem_write_handler(bx_phy_address addr, unsigned len, void *data, void *param);
};

#endif // BX_SUPPORT_NVRIVA
