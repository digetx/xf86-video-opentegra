/*
 * Copyright (c) 2016 Dmitry Osipenko <digetx@gmail.com>
 * Copyright (C) 2012-2013 NVIDIA Corporation.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 *
 * Authors:
 *    Francis Hart <fhart@nvidia.com>
 */

#include "tegra_stream.h"
#include "tegra_2d_sbcopy.h"
#include "tegra_2d_reg_g2sb.h"
#include "tegra_2d_reg_host.h"
#include "tegra_2d_surface.h"

void sbcopy_init(struct tegra_2d_sbcopy *hw)
{
    GR2D_RESET(trigger);
    GR2D_RESET(cmdsel);
    GR2D_RESET(vdda);
    GR2D_RESET(vddaini);
    GR2D_RESET(hdda);
    GR2D_RESET(hddainils);
    GR2D_RESET(sbformat);
    GR2D_RESET(controlsb);
    GR2D_RESET(controlsecond);
    GR2D_RESET(controlmain);
    GR2D_RESET(dstba);
    GR2D_RESET(tilemode);
    GR2D_RESET(srcba_sb_surfbase);
    GR2D_RESET(dstba_sb_surfbase);
    GR2D_RESET(dstst);
    GR2D_RESET(srcba);
    GR2D_RESET(srcst);
    GR2D_RESET(srcsize);
    GR2D_RESET(dstsize);

    OPCODE_MASK6(op1, trigger, cmdsel, vdda, vddaini, hdda, hddainils);
    OPCODE_MASK5(op2, sbformat, controlsb, controlsecond, controlmain,
             dstba);
    OPCODE_MASK3(op3, tilemode, srcba_sb_surfbase, dstba_sb_surfbase);
    OPCODE_MASK5(op4, dstst, srcba, srcst, srcsize, dstsize);

    GR2D_VAL(trigger, trigger, gr2d_dstsize_r());
    GR2D_DEF(cmdsel, sbor2d, sb);
    GR2D_DEF(controlmain, cmdt, bitblt);
    GR2D_DEF(controlmain, srcdir, enable);
    GR2D_DEF(controlmain, dstdir, enable);
    GR2D_DEF(controlmain, test0bit, enable);
}

static uint32_t compute_offset(const struct tegra_2d_surface *restrict surface,
                               uint32_t xpos, uint32_t ypos)
{
    uint32_t offset;
    uint32_t bytes_per_pixel = TEGRA_2D_FORMAT_BYTES(surface->format);
    uint32_t pixels_per_line = surface->pitch / bytes_per_pixel;

    if (surface->layout == TEGRA_2D_LAYOUT_LINEAR) {
        offset = ypos * surface->pitch;
        offset += xpos * bytes_per_pixel;
    } else {
        uint32_t xb = xpos * bytes_per_pixel;
        offset = 16 * pixels_per_line * (ypos / 16);
        offset += 256 * (xb / 16);
        offset += 16 * (ypos % 16);
        offset += xb % 16;
    }

    return offset;
}

int sbcopy_set(struct tegra_2d_sbcopy *hw,
               const struct tegra_2d_surface_area *dst,
               const struct tegra_2d_surface_area *src,
               enum tegra_2d_transform transform,
               enum tegra_2d_blit blit)
{
    int transpose;
    int flip_x;
    int flip_y;
    float inv_scale_x;
    float inv_scale_y;
    uint32_t dst_xpos = dst->xpos;
    uint32_t dst_ypos = dst->ypos;
    uint32_t dst_width = dst->width;
    uint32_t dst_height = dst->height;
    uint32_t src_xpos = src->xpos;
    uint32_t src_ypos = src->ypos;
    uint32_t src_width = src->width;
    uint32_t src_height = src->height;

    ASSERT(hw);
    ASSERT(dst);
    ASSERT(src);

    hw->is_valid = 0;

    hw->dst_handle = dst->surface->bo;
    hw->dst_offset = compute_offset(dst->surface, dst_xpos, dst_ypos) +
        dst->mem_offset;
    hw->src_handle = src->surface->bo;
    hw->src_offset = compute_offset(src->surface, src_xpos, src_ypos) +
        src->mem_offset;

    switch (transform) {

#define CASE(transform, t, fx, fy)          \
    case TEGRA_2D_TRANSFORM_##transform:    \
        transpose = t;                      \
        flip_x = fx;                        \
        flip_y = fy;                        \
        break

        CASE(IDENTITY, 0, 0, 0);
        CASE(ROT_90, 1, 0, 1);
        CASE(ROT_180, 0, 1, 1);
        CASE(ROT_270, 1, 1, 0);
        CASE(FLIP_X, 0, 1, 0);
        CASE(FLIP_Y, 0, 0, 1);
        CASE(TRANSPOSE, 1, 0, 0);
        CASE(INV_TRANSPOSE, 1, 1, 1);

#undef CASE

    default:
        SET_ERROR("TEGRA_2D_UNSUPPORTED_BLIT\n");
        return TEGRA_2D_UNSUPPORTED_BLIT;
    }

    if (flip_x)
        return TEGRA_2D_UNSUPPORTED_BLIT;

#define FMT(sf, df) \
    (TEGRA_2D_FORMAT_ID(sf) | (TEGRA_2D_FORMAT_ID(df) << 16))

    switch (FMT(src->surface->format, dst->surface->format)) {

#define CASE(sf, df, sif, dif)                              \
    case FMT(TEGRA_2D_FORMAT_##sf, TEGRA_2D_FORMAT_##df):   \
        GR2D_DEF(sbformat, sifmt, sif);                     \
        GR2D_DEF(sbformat, difmt, dif);                     \
        break

        CASE(R5G6B5, R5G6B5, b5g6r5, b5g6r5);
        CASE(R5G6B5, A1R5G5B5, b5g6r5, b5g6r5bs);
        CASE(R5G6B5, A8B8G8R8, b5g6r5, r8g8b8a8);
        CASE(R5G6B5, A8R8G8B8, b5g6r5, b8g8r8a8);
        CASE(A1R5G5B5, R5G6B5, b5g6r5bs, b5g6r5);
        CASE(A1R5G5B5, A1R5G5B5, b5g6r5bs, b5g6r5bs);
        CASE(A1R5G5B5, A8B8G8R8, b5g6r5bs, r8g8b8a8);
        CASE(A1R5G5B5, A8R8G8B8, b5g6r5bs, b8g8r8a8);
        CASE(A8B8G8R8, R5G6B5, r8g8b8a8, b5g6r5);
        CASE(A8B8G8R8, A1R5G5B5, r8g8b8a8, b5g6r5bs);
        CASE(A8B8G8R8, A8B8G8R8, r8g8b8a8, r8g8b8a8);
        CASE(A8B8G8R8, A8R8G8B8, r8g8b8a8, b8g8r8a8);
        CASE(A8R8G8B8, R5G6B5, b8g8r8a8, b5g6r5);
        CASE(A8R8G8B8, A1R5G5B5, b8g8r8a8, b5g6r5bs);
        CASE(A8R8G8B8, A8B8G8R8, b8g8r8a8, r8g8b8a8);
        CASE(A8R8G8B8, A8R8G8B8, b8g8r8a8, b8g8r8a8);

#undef CASE

    default:
        SET_ERROR("TEGRA_2D_UNSUPPORTED_BLIT\n");
        return TEGRA_2D_UNSUPPORTED_BLIT;
    }

//     switch (blit) {
//     case TEGRA_2D_BLIT_SRC:
//         GR2D_VAL(controlmain, alpen, 0);
//         GR2D_VAL(controlmain, srcbas, 0);
//         break;
//     case TEGRA_2D_BLIT_DST:
//         GR2D_VAL(controlmain, alpen, 0);
//         GR2D_VAL(controlmain, srcbas, 1);
//         break;
//     case TEGRA_2D_BLIT_OVER:
// //     case TEGRA_2D_BLIT_OVER_REVERSE:
//         switch (src->surface->format) {
//         case TEGRA_2D_FORMAT_A8R8G8B8:
//         case TEGRA_2D_FORMAT_A8B8G8R8:
//         case TEGRA_2D_FORMAT_B8G8R8A8:
//             break;
//         default:
//             SET_ERROR("TEGRA_2D_UNSUPPORTED_BLIT\n");
//             return TEGRA_2D_UNSUPPORTED_BLIT;
//         }
//
//         switch (dst->surface->format) {
//         case TEGRA_2D_FORMAT_A8R8G8B8:
//         case TEGRA_2D_FORMAT_A8B8G8R8:
//         case TEGRA_2D_FORMAT_B8G8R8A8:
//             break;
//         default:
//             SET_ERROR("TEGRA_2D_UNSUPPORTED_BLIT\n");
//             return TEGRA_2D_UNSUPPORTED_BLIT;
//         }
//
// //         GR2D_VAL(controlmain, alpen, 1);
//         GR2D_VAL(controlmain, srcbas, 0);
//         GR2D_DEF(controlsecond, alptype, pls8bpp);
//
//         if (blit== TEGRA_2D_BLIT_OVER_REVERSE)
//             GR2D_DEF(controlsecond, alpsrcordst, enable);
//         else
//             GR2D_DEF(controlsecond, alpsrcordst, disable);
//
// //         GR2D_VAL(alphablend, alphainv, 1);
//
//         GR2D_VAL(controlmain, srct, 2);
//         GR2D_VAL(controlmain, dstt, 2);
//         break;
//     default:
//         SET_ERROR("TEGRA_2D_UNSUPPORTED_BLIT\n");
//         return TEGRA_2D_UNSUPPORTED_BLIT;
//     }

    if (transpose) {
        GR2D_DEF(controlmain, xytdw, enable);
        inv_scale_x = ((float)src->surface->width - 1) /
            (dst->surface->height - 1);
        inv_scale_y = ((float)src->surface->height - 1) /
            (dst->surface->width - 1);
    } else {
        GR2D_DEF(controlmain, xytdw, disable);
        inv_scale_x = ((float)src->surface->width - 1) /
            (dst->surface->width - 1);
        inv_scale_y = ((float)src->surface->height - 1) /
            (dst->surface->height - 1);
    }

#define CHECK_SCALE(inv)                    \
    if (inv > 64.0f || inv < 1.0f/4096.0f)  \
    {\
        SET_ERROR("TEGRA_2D_UNSUPPORTED_BLIT\n");\
        return TEGRA_2D_UNSUPPORTED_BLIT;\
    }

    CHECK_SCALE(inv_scale_x);
    CHECK_SCALE(inv_scale_y);

#undef CHECK_SCALE

    GR2D_DEF(controlsb, discsc, disable);

    if (inv_scale_x == 1.0f)
        GR2D_DEF(controlsb, hftype, disable);
    else if (inv_scale_x < 1.0f)
        GR2D_DEF(controlsb, hftype, interp);
    else if (inv_scale_x < 1.3f)
        GR2D_DEF(controlsb, hftype, lpf1);
    else if (inv_scale_x < 2.0f)
        GR2D_DEF(controlsb, hftype, lpf3);
    else
        GR2D_DEF(controlsb, hftype, lpf6);

    if (inv_scale_y == 1.0f) {
        GR2D_DEF(controlsb, vfen, disable);
    } else {
        GR2D_DEF(controlsb, vfen, enable);

        if (inv_scale_y < 1.0f)
            GR2D_DEF(controlsb, vftype, interp);
        else if (inv_scale_y < 1.3f)
            GR2D_DEF(controlsb, vftype, avg25_interp75);
        else if (inv_scale_y < 2.0f)
            GR2D_DEF(controlsb, vftype, avg50_interp50);
        else
            GR2D_DEF(controlsb, vftype, avg);
    }

    if (flip_y) {
        GR2D_DEF(controlmain, yflip, enable);
        hw->dst_offset += dst->surface->pitch * (dst->height - 1);
    } else {
        GR2D_DEF(controlmain, yflip, disable);
    }

    switch (TEGRA_2D_FORMAT_BITS(dst->surface->format)) {

#define CASE(v)                                 \
    case v:                                     \
        GR2D_DEF(controlmain, dstcd, bpp##v);   \
        break

        CASE(8);
        CASE(16);
        CASE(32);

#undef CASE

    default:
        SET_ERROR("TEGRA_2D_UNSUPPORTED_BLIT\n");
        return TEGRA_2D_UNSUPPORTED_BLIT;
    }

    GR2D_VAL(vdda, vdstep, FLOAT_TO_FIXED_6_12(inv_scale_y));
    GR2D_VAL(vddaini, vdtini, FLOAT_TO_FIXED_0_8(src->ypos));
    GR2D_VAL(hdda, hdstep, FLOAT_TO_FIXED_6_12(inv_scale_x));
    GR2D_VAL(hddainils, hdini, FLOAT_TO_FIXED_0_8(src->xpos));

    if (dst->surface->layout == TEGRA_2D_LAYOUT_TILED)
        GR2D_DEF(tilemode, dst_wr_tile_mode, tiled);
    else
        GR2D_DEF(tilemode, dst_wr_tile_mode, linear);

    if (src->surface->layout == TEGRA_2D_LAYOUT_TILED)
        GR2D_DEF(tilemode, src_y_tile_mode, tiled);
    else
        GR2D_DEF(tilemode, src_y_tile_mode, linear);

    GR2D_VAL(dstst, dsts, dst->surface->pitch);
    GR2D_VAL(srcst, srcs, src->surface->pitch);
    GR2D_VAL(srcsize, srcwidth, src_width - 1);
    GR2D_VAL(srcsize, srcheight, src_height - 1);
    GR2D_VAL(dstsize, dstwidth, dst_width - 1);
    GR2D_VAL(dstsize, dstheight, dst_height - 1);

    hw->is_valid = 1;
    return TEGRA_2D_OK;
}

void sbcopy_dispatch(const struct tegra_2d_sbcopy *hw,
                     struct tegra_stream *stream)
{
    ASSERT(hw);
    ASSERT(hw->is_valid);
    ASSERT(stream);

    tegra_stream_push_setclass(stream, HOST1X_CLASS_GR2D_SB);

    tegra_stream_push_words(stream, &hw->block,
                            sizeof(hw->block) / sizeof(uint32_t), 4,
                            tegra_reloc(&hw->block.dstba,
                                        hw->dst_handle, hw->dst_offset,
                                offsetof(typeof(hw->block), dstba)),
                            tegra_reloc(&hw->block.srcba_sb_surfbase,
                                        hw->src_handle, hw->src_offset,
                                offsetof(typeof(hw->block), srcba_sb_surfbase)),
                            tegra_reloc(&hw->block.dstba_sb_surfbase,
                                        hw->dst_handle, hw->dst_offset,
                                offsetof(typeof(hw->block), dstba_sb_surfbase)),
                            tegra_reloc(&hw->block.srcba,
                                        hw->src_handle, hw->src_offset,
                                offsetof(typeof(hw->block), srcba)));
}
