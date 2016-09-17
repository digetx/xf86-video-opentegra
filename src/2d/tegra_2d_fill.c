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

static int fill_area(struct tegra_2d_context *ctx,
                     const struct tegra_2d_surface_area *dst_area)
{
    int result;

    result = g2fill_set(&ctx->g2fill, dst_area, ctx->color);
    if (result != TEGRA_2D_OK)
        return result;

    g2fill_dispatch(&ctx->g2fill, &ctx->stream);

    return TEGRA_2D_OK;
}

int tegra_2d_fill(struct tegra_2d_context *ctx,
                  const struct tegra_2d_surface *dst,
                  const struct tegra_2d_rect *dst_rect)
{
    struct tegra_2d_rect vdst_rect;
    struct tegra_2d_surface_area dst_area;
    int result;

    if (ctx == NULL)
        return TEGRA_2D_INVALID_PARAMETER;

    if (dst == NULL)
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

    dst_area.mem_offset = terga_2d_mem_get_offset(&ctx->allocator, dst->data);

    compute_surface_area(&dst_area, dst, &vdst_rect);

    result = fill_area(ctx, &dst_area);
    if (result != TEGRA_2D_OK)
        return result;

    return TEGRA_2D_OK;
}
