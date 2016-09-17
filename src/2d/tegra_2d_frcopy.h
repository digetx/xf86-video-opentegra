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

#ifndef TEGRA_2D_FRCOPY_H_
#define TEGRA_2D_FRCOPY_H_

#include <stdint.h>
#include "tegra_2d.h"
#include "tegra_2d_util.h"

struct tegra_2d_frcopy {
    struct tegra_2d_frcopy_block {
        uint32_t op1;
        uint32_t trigger;
        uint32_t cmdsel;

        uint32_t op2;
        uint32_t controlsecond;
        uint32_t controlmain;

        uint32_t op3;
        uint32_t tilemode;

        uint32_t op4;
        uint32_t dstba;
        uint32_t dstst;
        uint32_t srcba;
        uint32_t srcst;
        uint32_t srcsize;
    } block;
    int is_valid;
    struct drm_tegra_bo *dst_handle;
    struct drm_tegra_bo *src_handle;
    int dst_offset;
    int src_offset;
};

void frcopy_init(struct tegra_2d_frcopy *hw);

int frcopy_set(struct tegra_2d_frcopy *hw,
               const struct tegra_2d_surface_area *dst,
               const struct tegra_2d_surface_area *src,
               enum tegra_2d_transform transform);

void frcopy_dispatch(const struct tegra_2d_frcopy *hw,
                     struct tegra_stream *stream);

#endif
