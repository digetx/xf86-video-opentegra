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

#include "tegra_2d.h"
#include "tegra_2d_surface.h"
#include "tegra_2d_util.h"

void compute_surface_area(struct tegra_2d_surface_area *area,
                          const struct tegra_2d_surface *surface,
                          const struct tegra_2d_rect *rect)
{
    ASSERT(area);
    ASSERT(surface);
    ASSERT(rect);

    area->surface = surface;
    area->xpos = rect->left;
    area->ypos = rect->top;
    area->width = RECT_WIDTH(*rect);
    area->height = RECT_HEIGHT(*rect);
}

int validate_rect(const struct tegra_2d_surface *surface,
                  const struct tegra_2d_rect *rect)
{
    if (rect->left < 0 || rect->left >= rect->right || rect->top < 0 ||
        rect->top >= rect->bottom ||
        rect->right > surface->width ||
        rect->bottom > surface->height)
            return TEGRA_2D_INVALID_RECT;

    return TEGRA_2D_OK;
}

int transform_swaps_xy(enum tegra_2d_transform transform)
{
    switch (transform) {
    case TEGRA_2D_TRANSFORM_ROT_90:
    case TEGRA_2D_TRANSFORM_ROT_270:
    case TEGRA_2D_TRANSFORM_TRANSPOSE:
    case TEGRA_2D_TRANSFORM_INV_TRANSPOSE:
        return 1;
    default:
    case TEGRA_2D_TRANSFORM_IDENTITY:
    case TEGRA_2D_TRANSFORM_ROT_180:
    case TEGRA_2D_TRANSFORM_FLIP_X:
    case TEGRA_2D_TRANSFORM_FLIP_Y:
        return 0;
    }
}

int equal_size_area(const struct tegra_2d_surface_area *a,
                    const struct tegra_2d_surface_area *b,
                    enum tegra_2d_transform transform)
{
#define CHECK(fa, fb)               \
    if (fabs(fa - fb) >= 0.001f)    \
        return 0

    if (transform_swaps_xy(transform)) {
        CHECK(a->width, b->height);
        CHECK(a->height, b->width);
    } else {
        CHECK(a->width, b->width);
        CHECK(b->height, b->height);
    }

#undef CHECK

    return 1;
}
