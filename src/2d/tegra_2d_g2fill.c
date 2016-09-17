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
#include "tegra_2d_g2fill.h"
#include "tegra_2d_reg_g2sb.h"
#include "tegra_2d_reg_host.h"
#include "tegra_2d_surface.h"

void g2fill_init(struct tegra_2d_g2fill *hw)
{
    GR2D_RESET(trigger);
    GR2D_RESET(cmdsel);
    GR2D_RESET(controlsecond);
    GR2D_RESET(controlmain);
    GR2D_RESET(ropfade);
    GR2D_RESET(dstba);
    GR2D_RESET(dstst);
    GR2D_RESET(srcfgc);
    GR2D_RESET(dstsize);
    GR2D_RESET(dstps);

    OPCODE_MASK2(op1, trigger, cmdsel);
    OPCODE_MASK3(op2, controlsecond, controlmain, ropfade);
    OPCODE_MASK5(op3, dstba, dstst, srcfgc, dstsize, dstps);

    GR2D_VAL(trigger, trigger, gr2d_dstps_r());
    GR2D_DEF(cmdsel, sbor2d, g2);
    GR2D_DEF(controlmain, cmdt, bitblt);
    GR2D_DEF(controlmain, turbofill, enable);
    GR2D_DEF(controlmain, srcsld, enable);
    GR2D_DEF(controlmain, test0bit, enable);
    GR2D_VAL(ropfade, rop, 0xCC);
}

int g2fill_set(struct tegra_2d_g2fill *hw,
               const struct tegra_2d_surface_area *dst,
               uint32_t color)
{
    int max_size;
    uint32_t dst_xpos = dst->xpos;
    uint32_t dst_ypos = dst->ypos;
    uint32_t dst_width = dst->width;
    uint32_t dst_height = dst->height;

    ASSERT(hw);
    ASSERT(dst);

    hw->is_valid = 0;

    hw->dst_handle = dst->surface->bo;
    hw->dst_offset = dst->mem_offset;

    switch (TEGRA_2D_FORMAT_BITS(dst->surface->format)) {

#define CASE(v, max)                            \
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
    GR2D_VAL(dstst, dsts, dst->surface->pitch);
    GR2D_VAL(srcfgc, srcfgc, color);
    GR2D_VAL(dstsize, dstwidth, dst_width);
    GR2D_VAL(dstsize, dstheight, dst_height);
    GR2D_VAL(dstps, dstx, dst_xpos);
    GR2D_VAL(dstps, dsty, dst_ypos);

    hw->is_valid = 1;
    return TEGRA_2D_OK;
}

void g2fill_dispatch(const struct tegra_2d_g2fill *hw,
                     struct tegra_stream *stream)
{
    ASSERT(hw);
    ASSERT(hw->is_valid);
    ASSERT(stream);

    tegra_stream_push_setclass(stream, HOST1X_CLASS_GR2D);

    tegra_stream_push_words(stream, &hw->block,
                            sizeof(hw->block) / sizeof(uint32_t), 1,
                            tegra_reloc(&hw->block.dstba, hw->dst_handle,
                                        hw->dst_offset,
                                        offsetof(typeof(hw->block), dstba)));
}
