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

#ifndef TEGRA_2D_UTIL_H_
#define TEGRA_2D_UTIL_H_

#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <math.h>

#include <string.h>

#define ASSERT(a) \
    if (!a) \
        fprintf(stderr, "%s: %d: Assertion failed %s\n", __FILE__, __LINE__, #a);
#define CT_ASSERT(x) ((void) sizeof(char[1 - 2*!(x)]))

#define MIN(a, b) (a < b ? a : b)
#define MAX(a, b) (a > b ? a : b)

#define CLAMP(x, l, h) (((x) < (l)) ? (l) : ((x) > (h) ? (h) : (x)))

#define ROUND_UP(v, a) (v + (a - 1)) & ~(a - 1)

#define RECT_WIDTH(r)  ((r).right - (r).left)
#define RECT_HEIGHT(r) ((r).bottom - (r).top)

#define FLOAT_TO_FIXED_6_12(fp) (((int32_t) (fp * 4096.0f + 0.5f)) & ((1<<18)-1))
#define FLOAT_TO_FIXED_0_8(fp) (((int32_t) (fp * 256.0f + 0.5f)) & ((1<<8)-1))

#define SET_ERROR(fmt, args...) \
    fprintf(stderr, "%s: %d: (TEGRA2D ERROR) " fmt, __func__, __LINE__, ##args);

struct tegra_2d_surface_area {
    const struct tegra_2d_surface *surface;
    uint32_t mem_offset;
    float xpos;
    float ypos;
    float width;
    float height;
};

void compute_surface_area(struct tegra_2d_surface_area *area,
                          const struct tegra_2d_surface *surface,
                          const struct tegra_2d_rect *rect);

int validate_rect(const struct tegra_2d_surface *surface,
                  const struct tegra_2d_rect *rect);

int equal_size_area(const struct tegra_2d_surface_area *a,
                    const struct tegra_2d_surface_area *b,
                    enum tegra_2d_transform transform);

int transform_swaps_xy(enum tegra_2d_transform transform);

#endif
