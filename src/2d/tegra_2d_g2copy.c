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
#include "tegra_2d_g2copy.h"
#include "tegra_2d_reg_g2sb.h"
#include "tegra_2d_reg_host.h"
#include "tegra_2d_surface.h"

void g2copy_init(struct tegra_2d_g2copy *hw)
{
    GR2D_RESET(trigger);
    GR2D_RESET(cmdsel);
    GR2D_RESET(controlsecond);
    GR2D_RESET(controlmain);
    GR2D_RESET(ropfade);
    GR2D_RESET(tilemode);
    GR2D_RESET(dstba);
    GR2D_RESET(dstst);
    GR2D_RESET(srcba);
    GR2D_RESET(srcst);
    GR2D_RESET(srcsize);
    GR2D_RESET(dstsize);
    GR2D_RESET(srcps);
    GR2D_RESET(dstps);

    OPCODE_MASK2(op1, trigger, cmdsel);
    OPCODE_MASK4(op2, controlsecond, controlmain, ropfade, dstba);
    OPCODE_NONINCR(op3, tilemode, 1);
    OPCODE_MASK7(op4, dstst, srcba, srcst, srcsize, dstsize, srcps, dstps);

    GR2D_VAL(trigger, trigger, gr2d_dstps_r());
    GR2D_DEF(cmdsel, sbor2d, g2);
    GR2D_DEF(controlmain, cmdt, bitblt);
    GR2D_DEF(controlmain, test0bit, enable);
}

static uint32_t rop_coded(enum tegra_2d_rop rop)
{
    switch (rop) {
    case TEGRA_2D_ROP_CLEAR:
        return 0x00;
    case TEGRA_2D_ROP_AND:
        return 0x88;
    case TEGRA_2D_ROP_AND_REVERSE:
        return 0x44;
    case TEGRA_2D_ROP_AND_INVERTED:
        return 0x22;
    case TEGRA_2D_ROP_NAND:
        return 0x77;
    case TEGRA_2D_ROP_COPY:
        return 0xCC;
    case TEGRA_2D_ROP_COPY_INVERTED:
        return 0x33;
    case TEGRA_2D_ROP_XOR:
        return 0x66;
    case TEGRA_2D_ROP_OR:
        return 0xEE;
    case TEGRA_2D_ROP_OR_REVERSE:
        return 0xDD;
    case TEGRA_2D_ROP_NOR:
        return 0x11;
    case TEGRA_2D_ROP_EQUIV:
        return 0x99;
    case TEGRA_2D_ROP_INVERT:
        return 0x55;
    case TEGRA_2D_ROP_SET:
        return 0xFF;
    case TEGRA_2D_ROP_NOOP:
    default:
        return 0xAA;
    }
}

int g2copy_set(struct tegra_2d_g2copy *hw,
               const struct tegra_2d_surface_area *dst,
               const struct tegra_2d_surface_area *src,
               enum tegra_2d_transform transform,
               enum tegra_2d_rop rop,
               int xdir, int ydir)
{
    int transpose;
    int flip_x;
    int flip_y;
    uint32_t max_size;
    uint32_t dst_xpos = dst->xpos;
    uint32_t dst_ypos = dst->ypos;
    uint32_t dst_width = dst->width;
    uint32_t dst_height = dst->height;
    uint32_t src_xpos = src->xpos;
    uint32_t src_ypos = src->ypos;

    ASSERT(hw);
    ASSERT(dst);
    ASSERT(src);

    hw->is_valid = 0;

    if (TEGRA_2D_FORMAT_BITS(src->surface->format) !=
            TEGRA_2D_FORMAT_BITS(dst->surface->format))
        return TEGRA_2D_UNSUPPORTED_BLIT;

    if (!equal_size_area(src, dst, transform))
        return TEGRA_2D_UNSUPPORTED_BLIT;

    hw->dst_handle = dst->surface->bo;
    hw->dst_offset = dst->mem_offset;
    hw->src_handle = src->surface->bo;
    hw->src_offset = src->mem_offset;

    switch (transform) {
#define CASE(transform, t, fx, fy)              \
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
        ASSERT(0);
        return TEGRA_2D_UNSUPPORTED_BLIT;
    }

    if (flip_x)
        return TEGRA_2D_UNSUPPORTED_BLIT;

    if (transpose)
        GR2D_DEF(controlmain, xytdw, enable);
    else
        GR2D_DEF(controlmain, xytdw, disable);

    switch (TEGRA_2D_FORMAT_BITS(dst->surface->format)) {
#define CASE(v, max)                                \
        case v:                                     \
            GR2D_DEF(controlmain, dstcd, bpp##v);   \
            max_size = max;                        \
            break
        CASE(8, 32760);
        CASE(16, 16384);
        CASE(32, 8192);
#undef CASE
    default:
        return TEGRA_2D_UNSUPPORTED_BLIT;
    }

    if (dst_width > max_size || dst_height > max_size)
        return TEGRA_2D_UNSUPPORTED_BLIT;

    GR2D_DEF(controlmain, srccd, same);


    SET_ERROR("dst_xpos %d src_xpos %d dst_ypos %d src_ypos %d xdir %d ydir %d - xdir %d ydir %d\n",
              dst_xpos, src_xpos, dst_ypos, src_ypos,
              dst_xpos > src_xpos, dst_ypos > src_ypos,
              xdir, ydir);

    if (xdir) {
        src_xpos += dst_width - 1;
        dst_xpos += dst_width - 1;
    }

    if (ydir) {
        src_ypos += dst_height - 1;

        if (!flip_y)
            dst_ypos += dst_height - 1;
    }

    GR2D_VAL(controlmain, xdir, xdir);
    GR2D_VAL(controlmain, ydir, ydir);

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
    GR2D_VAL(srcsize, srcwidth, dst_width);
    GR2D_VAL(srcsize, srcheight, dst_height);
    GR2D_VAL(dstsize, dstwidth, dst_width);
    GR2D_VAL(dstsize, dstheight, dst_height);
    GR2D_VAL(srcps, srcx, src_xpos);
    GR2D_VAL(srcps, srcy, src_ypos);
    GR2D_VAL(dstps, dstx, dst_xpos);
    if (flip_y) {
        GR2D_DEF(controlmain, yflip, enable);
        GR2D_VAL(dstps, dsty, dst_ypos + dst_height - 1);
    } else {
        GR2D_DEF(controlmain, yflip, disable);
        GR2D_VAL(dstps, dsty, dst_ypos);
    }

    GR2D_VAL(ropfade, rop, rop_coded(rop));

    hw->is_valid = 1;
    return TEGRA_2D_OK;
}

void g2copy_dispatch(const struct tegra_2d_g2copy *hw,
                     struct tegra_stream *stream)
{
    ASSERT(hw);
    ASSERT(hw->is_valid);
    ASSERT(stream);

    tegra_stream_push_setclass(stream, HOST1X_CLASS_GR2D);

    tegra_stream_push_words(stream, &hw->block,
                            sizeof(hw->block) / sizeof(uint32_t), 2,
                            tegra_reloc(&hw->block.dstba, hw->dst_handle,
                                        hw->dst_offset,
                                        offsetof(typeof(hw->block), dstba)),
                            tegra_reloc(&hw->block.srcba, hw->src_handle,
                                        hw->src_offset,
                                        offsetof(typeof(hw->block), srcba)));
}
