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

#ifndef TEGRA_2D_CONTEXT_H_
#define TEGRA_2D_CONTEXT_H_

#include "tegra_stream.h"
#include "tegra_2d_allocator.h"
#include "tegra_2d_g2fill.h"
#include "tegra_2d_g2copy.h"
#include "tegra_2d_frcopy.h"
#include "tegra_2d_sbcopy.h"

struct tegra_2d_context {
    struct drm_tegra *drm;
    struct terga_2d_mem_allocator allocator;
    struct tegra_stream stream;
    struct tegra_2d_g2fill g2fill;
    struct tegra_2d_g2copy g2copy;
    struct tegra_2d_frcopy frcopy;
    struct tegra_2d_sbcopy sbcopy;

    uint32_t color;
    enum tegra_2d_rop rop;
    enum tegra_2d_blit blit;
    unsigned xdir:1;
    unsigned ydir:1;

    uint32_t total_size;
};

#endif
