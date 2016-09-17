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
#include "tegra_2d_frcopy.h"
#include "tegra_2d_reg_g2sb.h"
#include "tegra_2d_reg_host.h"
#include "tegra_2d_surface.h"

void frcopy_init(struct tegra_2d_frcopy *hw)
{
    GR2D_RESET(trigger);
    GR2D_RESET(cmdsel);
    GR2D_RESET(controlsecond);
    GR2D_RESET(controlmain);
    GR2D_RESET(tilemode);
    GR2D_RESET(dstba);
    GR2D_RESET(dstst);
    GR2D_RESET(srcba);
    GR2D_RESET(srcst);
    GR2D_RESET(srcsize);

    OPCODE_MASK2(op1, trigger, cmdsel);
    OPCODE_MASK2(op2, controlsecond, controlmain);
    OPCODE_NONINCR(op3, tilemode, 1);
    OPCODE_MASK5(op4, dstba, dstst, srcba, srcst, srcsize);

    GR2D_VAL(trigger, trigger, gr2d_srcsize_r());
    GR2D_DEF(cmdsel, sbor2d, g2);
    GR2D_DEF(controlsecond, fr_mode, src_dst_copy);
    GR2D_DEF(controlsecond, fr_readwait, enable);
    GR2D_DEF(controlmain, cmdt, bitblt);
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

static int areas_intersects(const struct tegra_2d_surface *restrict dst,
                            const struct tegra_2d_surface *restrict src,
                            uint32_t src_xpos, uint32_t src_ypos,
                            uint32_t dst_xpos, uint32_t dst_ypos,
                            uint32_t width, uint32_t height)
{
    uint32_t bytes_per_pixel = TEGRA_2D_FORMAT_BYTES(dst->format);
    uint32_t bytes_per_line = bytes_per_pixel * width;
    uint32_t src_offset = compute_offset(dst, src_xpos, src_ypos);
    uint32_t dst_offset = compute_offset(dst, dst_xpos, dst_ypos);
    uint32_t src_end = compute_offset(src, src_xpos, src_ypos + height);
    uint32_t dst_end = compute_offset(dst, dst_xpos, dst_ypos + height);
    uint32_t left, right;

    if (dst->bo != src->bo)
        return 0;

    if (dst_offset == src_offset)
        return 0;

    if (dst_offset > src_end + bytes_per_line)
        return 0;

    if (src_offset > dst_end + bytes_per_line)
        return 0;

    if (src_offset + bytes_per_line < dst_offset) {
        left = src_end;
        right = src_end + bytes_per_line - bytes_per_pixel;
    } else {
        left = src_offset;
        right = src_offset + bytes_per_line - bytes_per_pixel;
    }

    while (height--) {
        dst_offset = compute_offset(dst, dst_xpos, dst_ypos++);

        if (left >= dst_offset)
            if (left < dst_offset + bytes_per_line)
                return 1;

        if (right >= dst_offset)
            if (right < dst_offset + bytes_per_line)
                return 1;
    }

    return 0;
}

int frcopy_set(struct tegra_2d_frcopy *hw,
               const struct tegra_2d_surface_area *dst,
               const struct tegra_2d_surface_area *src,
               enum tegra_2d_transform transform)
{
    uint32_t bytes_per_pixel;
    uint32_t block_size;
    uint32_t align_mask;
    uint32_t dst_xpos = dst->xpos;
    uint32_t dst_ypos = dst->ypos;
    uint32_t src_xpos = src->xpos;
    uint32_t src_ypos = src->ypos;
    uint32_t src_width = src->width;
    uint32_t src_height = src->height;

    hw->is_valid = 0;

    if (src_width > 4096 || src_height > 4096)
        return TEGRA_2D_UNSUPPORTED_BLIT;

    if (TEGRA_2D_FORMAT_BITS(src->surface->format) !=
            TEGRA_2D_FORMAT_BITS(dst->surface->format))
        return TEGRA_2D_UNSUPPORTED_BLIT;

    if (!equal_size_area(src, dst, transform))
        return TEGRA_2D_UNSUPPORTED_BLIT;

    if (dst_xpos > src_xpos || dst_ypos > src_ypos)
        if (areas_intersects(dst->surface, src->surface,
                             src_xpos, src_ypos,
                             dst_xpos, dst_ypos,
                             src_width, src_height))
            return TEGRA_2D_UNSUPPORTED_BLIT;

    hw->dst_handle = dst->surface->bo;
    hw->dst_offset = compute_offset(dst->surface, dst_xpos, dst_ypos) +
        dst->mem_offset;
    hw->src_handle = src->surface->bo;
    hw->src_offset = compute_offset(src->surface, src_xpos, src_ypos) +
        src->mem_offset;

    switch (transform) {

#define CASE(transform, v)                      \
    case TEGRA_2D_TRANSFORM_##transform:        \
        GR2D_DEF(controlsecond, fr_type, v);    \
        break

        CASE(IDENTITY, identity);
        CASE(ROT_90, rot_90);
        CASE(ROT_180, rot_180);
        CASE(ROT_270, rot_270);
        CASE(FLIP_X, flip_x);
        CASE(FLIP_Y, flip_y);
        CASE(TRANSPOSE, trans_lr);
        CASE(INV_TRANSPOSE, trans_rl);

#undef CASE

    default:
        return TEGRA_2D_UNSUPPORTED_BLIT;
    }

    switch (TEGRA_2D_FORMAT_BITS(dst->surface->format)) {

#define CASE(v)                                 \
    case v:                                     \
        GR2D_DEF(controlmain, dstcd, bpp##v);   \
        block_size = 128 / v;                   \
        break

        CASE(8);
        CASE(16);
        CASE(32);

#undef CASE

    default:
        return TEGRA_2D_UNSUPPORTED_BLIT;
    }

    /* Fast rotate operates with a 128bit chunks. */
    bytes_per_pixel = TEGRA_2D_FORMAT_BYTES(dst->surface->format);
    align_mask = block_size - 1;
#define CHECK_ALIGNMENT(v) ((((uint32_t) v) & align_mask) != 0)
    if (((src_width * bytes_per_pixel) % 16) || CHECK_ALIGNMENT(src_height)
        || (hw->dst_offset % 16) || (hw->src_offset % 16))
        return TEGRA_2D_UNSUPPORTED_BLIT;
#undef CHECK_ALIGNMENT

    SET_ERROR("FR dst_xpos %d dst_ypos %d src_xpos %d src_ypos %d width %d height %d dst->bo 0x%08X src->bo 0x%08X\n",
              dst_xpos, dst_ypos, src_xpos, src_ypos, src_width, src_height, dst->surface->bo, src->surface->bo);

    GR2D_DEF(controlmain, srccd, same);

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

    hw->is_valid = 1;
    return TEGRA_2D_OK;
}

void frcopy_dispatch(const struct tegra_2d_frcopy *hw,
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
