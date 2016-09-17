/*
 * Copyright (c) 2016 Dmitry Osipenko <digetx@gmail.com>
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
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef TEGRA2D_MEM_ALLOCATOR_H_
#define TEGRA2D_MEM_ALLOCATOR_H_

#include <libdrm/tegra.h>

struct terga_2d_mem_allocator {
    struct drm_tegra_bo *bo;
    void *mem_pool;
};

int terga_2d_init_mem_allocator(struct terga_2d_mem_allocator *ma,
                                struct drm_tegra *drm, size_t size);

int terga_2d_release_mem_allocator(struct terga_2d_mem_allocator *ma);

void * terga_2d_mem_alloc(struct terga_2d_mem_allocator *ma, size_t size);

void * terga_2d_mem_realloc(struct terga_2d_mem_allocator *ma,
                            void *ptr, size_t size);

void terga_2d_mem_free(struct terga_2d_mem_allocator *ma, void *ptr);

uint32_t terga_2d_mem_get_offset(struct terga_2d_mem_allocator *ma, void *ptr);

uint32_t terga_2d_mem_used(struct terga_2d_mem_allocator *ma);

struct drm_tegra_bo * terga_2d_mem_get_bo(struct terga_2d_mem_allocator *ma);

#endif
