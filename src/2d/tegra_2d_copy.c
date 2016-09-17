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

#include <stdlib.h>
#include "tegra_2d.h"
#include "tegra_2d_context.h"
#include "tegra_2d_surface.h"
#include "tegra_2d_util.h"
#include "tegra_2d_g2copy.h"
#include "tegra_2d_frcopy.h"
#include "tegra_2d_sbcopy.h"

static int copy_area(struct tegra_2d_context *ctx,
                     const struct tegra_2d_surface_area *dst_area,
                     const struct tegra_2d_surface_area *src_area,
                     enum tegra_2d_transform transform)
{
    int result = -1;

    ctx->sbcopy.is_valid = 0;

    if (ctx->rop == TEGRA_2D_ROP_INVALID && ctx->blit == TEGRA_2D_BLIT_INVALID)
        return TEGRA_2D_INVALID_PARAMETER;

    if (ctx->rop == TEGRA_2D_ROP_COPY)
        result = frcopy_set(&ctx->frcopy, dst_area, src_area, transform);

    if (result != TEGRA_2D_OK) {
        if (ctx->rop != TEGRA_2D_ROP_INVALID)
            result = g2copy_set(&ctx->g2copy, dst_area, src_area, transform,
                                ctx->rop, ctx->xdir, ctx->ydir);
    }

    if (result != TEGRA_2D_OK) {
        if (ctx->blit != TEGRA_2D_BLIT_INVALID)
            result = sbcopy_set(&ctx->sbcopy, dst_area, src_area, transform,
                                ctx->blit);
    }

    if (result != TEGRA_2D_OK)
        return TEGRA_2D_UNSUPPORTED_BLIT;

    if (ctx->sbcopy.is_valid) {
        sbcopy_dispatch(&ctx->sbcopy, &ctx->stream);
    } else {
        if (ctx->frcopy.is_valid)
            frcopy_dispatch(&ctx->frcopy, &ctx->stream);
        else
            g2copy_dispatch(&ctx->g2copy, &ctx->stream);
    }

    return TEGRA_2D_OK;
}

int tegra_2d_copy(struct tegra_2d_context *ctx,
                  const struct tegra_2d_surface *dst,
                  const struct tegra_2d_surface *src,
                  const struct tegra_2d_rect *dst_rect,
                  const struct tegra_2d_rect *src_rect,
                  enum tegra_2d_transform transform)
{
    struct tegra_2d_rect vdst_rect;
    struct tegra_2d_rect vsrc_rect;
    struct tegra_2d_surface_area dst_area;
    struct tegra_2d_surface_area src_area;
    int result;

    if (ctx == NULL)
        return TEGRA_2D_INVALID_PARAMETER;

    if (dst == NULL)
        return TEGRA_2D_INVALID_PARAMETER;

    if (src == NULL)
        return TEGRA_2D_INVALID_PARAMETER;

    if (dst_rect == NULL) {
        vdst_rect.left = 0;
        vdst_rect.top = 0;
        vdst_rect.right = dst->width;
        vdst_rect.bottom = dst->height;
    } else {
        result = validate_rect(dst, dst_rect);
        if (result != TEGRA_2D_OK)
            return result;

        vdst_rect = *dst_rect;
    }

    if (src_rect == NULL) {
        vsrc_rect.left = 0;
        vsrc_rect.top = 0;
        vsrc_rect.right = src->width;
        vsrc_rect.bottom = src->height;
    } else {
        result = validate_rect(src, src_rect);
        if (result != TEGRA_2D_OK)
            return result;

        vsrc_rect = *src_rect;
    }

    dst_area.mem_offset = terga_2d_mem_get_offset(&ctx->allocator, dst->data);
    src_area.mem_offset = terga_2d_mem_get_offset(&ctx->allocator, src->data);

    compute_surface_area(&dst_area, dst, &vdst_rect);
    compute_surface_area(&src_area, src, &vsrc_rect);

    return copy_area(ctx, &dst_area, &src_area, transform);
}

int tegra_2d_set_fill_color(struct tegra_2d_context *ctx,
                            uint32_t color)
{
    if (ctx == NULL)
        return TEGRA_2D_INVALID_PARAMETER;

    ctx->color = color;

    return TEGRA_2D_OK;
}
