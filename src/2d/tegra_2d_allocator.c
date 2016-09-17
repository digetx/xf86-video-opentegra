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

#include "tegra_2d.h"
#include "tegra_2d_util.h"
#include "tegra_2d_allocator.h"

#include "TLSF/tlsf.h"

int terga_2d_init_mem_allocator(struct terga_2d_mem_allocator *ma,
                                struct drm_tegra *drm, size_t size)
{
    int ret;

    if (ma == NULL)
        return TEGRA_2D_INVALID_PARAMETER;

    if (drm == NULL)
        return TEGRA_2D_INVALID_PARAMETER;

    memset(ma, 0, sizeof(*ma));

    ret = drm_tegra_bo_new(&ma->bo, drm, 0, size);

    if (ret != 0) {
        SET_ERROR("drm_tegra_bo_new() failed %d\n", ret);
        goto err_cleanup;
    }

    ret = drm_tegra_bo_map(ma->bo, &ma->mem_pool);

    if (ret != 0) {
        SET_ERROR("drm_tegra_bo_map() failed %d\n", ret);
        goto err_cleanup;
    }

    ret = init_memory_pool(size, ma->mem_pool);

    if (ret < 0) {
        SET_ERROR("init_memory_pool() failed %d\n", ret);
        goto err_cleanup;
    }

    return TEGRA_2D_OK;

err_cleanup:
    if (ma->mem_pool)
        drm_tegra_bo_unmap(ma->bo);

    if (ma->bo)
        drm_tegra_bo_unref(ma->bo);

    return TEGRA_2D_MEMORY_FAILURE;
}

int terga_2d_release_mem_allocator(struct terga_2d_mem_allocator *ma)
{
    int ret;

    if (ma == NULL)
        return TEGRA_2D_INVALID_PARAMETER;

    destroy_memory_pool(ma->mem_pool);

    ret = drm_tegra_bo_unmap(ma->bo);

    if (ret != 0)
        SET_ERROR("drm_tegra_bo_unmap() failed\n");

    drm_tegra_bo_unref(ma->bo);

    if (ret != 0)
        return TEGRA_2D_MEMORY_FAILURE;

    return TEGRA_2D_OK;
}

void * terga_2d_mem_alloc(struct terga_2d_mem_allocator *ma, size_t size)
{
    return malloc_ex(size, ma->mem_pool);
}

void * terga_2d_mem_realloc(struct terga_2d_mem_allocator *ma,
                            void *ptr, size_t size)
{
    return realloc_ex(ptr, size, ma->mem_pool);
}

void terga_2d_mem_free(struct terga_2d_mem_allocator *ma, void *ptr)
{
    free_ex(ptr, ma->mem_pool);
}

uint32_t terga_2d_mem_get_offset(struct terga_2d_mem_allocator *ma, void *ptr)
{
    ptrdiff_t offset = (uint8_t *)ptr - (uint8_t *)ma->mem_pool;

    if (ptr == NULL)
        return 0;

    return offset;
}

uint32_t terga_2d_mem_used(struct terga_2d_mem_allocator *ma)
{
    return get_used_size(ma->mem_pool);
}

struct drm_tegra_bo * terga_2d_mem_get_bo(struct terga_2d_mem_allocator *ma)
{
    return ma->bo;
}
