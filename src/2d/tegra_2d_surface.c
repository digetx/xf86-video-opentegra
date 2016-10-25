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
#include "tegra_2d_context.h"
#include "tegra_2d_surface.h"
#include "tegra_2d_util.h"

static int compute_requirements(struct tegra_2d_surface *surface,
                                uint32_t *num_bytes)
{
    uint32_t bytes_per_line;
    uint32_t bytes_per_pixel;
    uint32_t max_size = 0;
    uint32_t pitch;

    bytes_per_pixel = TEGRA_2D_FORMAT_BYTES(surface->format);

    switch (bytes_per_pixel) {
    case 1:
        max_size = 32768;
        break;
    case 2:
        max_size = 16384;
        break;
    case 4:
        max_size = 8192;
        break;
    }

    if (surface->width == 0 || surface->width > max_size) {
        SET_ERROR("Invalid surface width %d\n", surface->width);
        return TEGRA_2D_INVALID_PARAMETER;
    }

    if (surface->height == 0 || surface->height > max_size) {
        SET_ERROR("Invalid surface height %d\n", surface->height);
        return TEGRA_2D_INVALID_PARAMETER;
    }

    bytes_per_line = surface->width * bytes_per_pixel;

    if (surface->layout == TEGRA_2D_LAYOUT_LINEAR) {
        pitch = ROUND_UP(bytes_per_line, 32);
    } else {
        pitch = ROUND_UP(bytes_per_line, 64);
    }

    if (surface->pitch == 0)
        surface->pitch = pitch;

    if (bytes_per_line > surface->pitch || surface->pitch > 0xFFFF) {
        SET_ERROR("Invalid surface pitch %d bytes_per_line %d\n",
                  surface->pitch, bytes_per_line);
        return TEGRA_2D_INVALID_PARAMETER;
    }

    *num_bytes = surface->height * pitch;

    return TEGRA_2D_OK;
}

int tegra_2d_allocate_surface(struct tegra_2d_context *ctx,
                              struct tegra_2d_surface **surface,
                              int width, int height, int pitch,
                              enum tegra_2d_layout layout,
                              enum tegra_2d_format format,
                              struct drm_tegra_bo *bo,
                              int from_pool)
{
    struct tegra_2d_surface *surf = *surface;
    uint32_t num_bytes;
    int flags = 0;
    int result;

    if (ctx == NULL)
        return TEGRA_2D_INVALID_PARAMETER;

    if (surface == NULL) {
        SET_ERROR("surface is NULL\n");
        return TEGRA_2D_INVALID_PARAMETER;
    }

    switch (format) {
    case TEGRA_2D_FORMAT_8BIT:
    case TEGRA_2D_FORMAT_16BIT:
    case TEGRA_2D_FORMAT_32BIT:
    case TEGRA_2D_FORMAT_A8R8G8B8:
    case TEGRA_2D_FORMAT_A8B8G8R8:
    case TEGRA_2D_FORMAT_R8G8B8A8:
    case TEGRA_2D_FORMAT_B8G8R8A8:
    case TEGRA_2D_FORMAT_A4R4G4B4:
    case TEGRA_2D_FORMAT_R4G4B4A4:
    case TEGRA_2D_FORMAT_R5G6B5:
    case TEGRA_2D_FORMAT_A1R5G5B5:
        break;
    default:
        SET_ERROR("surface format is NULL\n");
        result = TEGRA_2D_INVALID_PARAMETER;
        goto err_cleanup;
    }

    switch (layout) {
    case TEGRA_2D_LAYOUT_LINEAR:
    case TEGRA_2D_LAYOUT_TILED:
        break;
    default:
        result = TEGRA_2D_INVALID_PARAMETER;
        goto err_cleanup;
    }

    if (surf == NULL)
        surf = calloc(1, sizeof(*surf));

    if (surf == NULL) {
        result = TEGRA_2D_MEMORY_FAILURE;
        SET_ERROR("Can't allocate surface struct\n");
        goto err_cleanup;
    }

    surf->width  = width;
    surf->height = height;
    surf->pitch  = pitch;
    surf->layout = layout;
    surf->format = format;

    result = compute_requirements(surf, &num_bytes);
    if (result != TEGRA_2D_OK) {
        SET_ERROR("compute_requirements() failed\n");
        goto err_cleanup;
    }

    if (surf->layout == TEGRA_2D_LAYOUT_TILED)
        flags |= DRM_TEGRA_GEM_CREATE_TILED;

    if (!bo) {
        uint32_t total = terga_2d_mem_used(&ctx->allocator);
        SET_ERROR("pool mem used %d kb (%d mb)\n", total, total / 1024 / 1024);
    }

    if (surf->bo && surf->data == NULL)
        drm_tegra_bo_unref(surf->bo);

    if (!bo && !from_pool) {
        terga_2d_mem_free(&ctx->allocator, surf->data);
        surf->data = NULL;

        result = drm_tegra_bo_new(&surf->bo, ctx->drm, flags, num_bytes);

        if (result != 0) {
            SET_ERROR("drm_tegra_bo_new() failed %d\n", result);
            result = TEGRA_2D_MEMORY_FAILURE;
            goto err_cleanup;
        }
    } else if (bo) {
        surf->bo = drm_tegra_bo_ref(bo);
        terga_2d_mem_free(&ctx->allocator, surf->data);
        surf->data = NULL;
    } else {
        surf->data = terga_2d_mem_realloc(&ctx->allocator, surf->data,
                                          num_bytes);
        if (surf->data != NULL)
            surf->bo = terga_2d_mem_get_bo(&ctx->allocator);
    }

    if (!surf->bo) {
        SET_ERROR("Can't allocate pool memory\n");
        result = TEGRA_2D_MEMORY_FAILURE;
        goto err_cleanup;
    }

    *surface = surf;

    return TEGRA_2D_OK;

err_cleanup:
    if (surf != NULL) {
        if (surf->bo && surf->data == NULL)
            drm_tegra_bo_unref(surf->bo);
        terga_2d_mem_free(&ctx->allocator, surf->data);
        free(surf);
    }
    *surface = NULL;
    return result;
}

int tegra_2d_free_surface(struct tegra_2d_context *ctx,
                          struct tegra_2d_surface **surface)
{
    struct tegra_2d_surface *surf;

    if (ctx == NULL)
        return TEGRA_2D_INVALID_PARAMETER;

    if (surface == NULL)
        return TEGRA_2D_INVALID_PARAMETER;

    surf = *surface;

    if (!surf->data)
        drm_tegra_bo_unref(surf->bo);

    terga_2d_mem_free(&ctx->allocator, surf->data);

    free(surf);

    *surface = NULL;

    return TEGRA_2D_OK;
}

int tegra_2d_get_surface_pitch(const struct tegra_2d_surface *surface,
                               int *pitch)
{
    if (surface == NULL)
        return TEGRA_2D_INVALID_PARAMETER;

    if (pitch == NULL)
        return TEGRA_2D_INVALID_PARAMETER;

    *pitch = surface->pitch;

    return TEGRA_2D_OK;
}

int tegra_2d_get_surface_width(const struct tegra_2d_surface *surface,
                               int *width)
{
    if (surface == NULL)
        return TEGRA_2D_INVALID_PARAMETER;

    if (width == NULL)
        return TEGRA_2D_INVALID_PARAMETER;

    *width = surface->width;

    return TEGRA_2D_OK;
}

int tegra_2d_get_surface_height(const struct tegra_2d_surface *surface,
                                int *height)
{
    if (surface == NULL)
        return TEGRA_2D_INVALID_PARAMETER;

    if (height == NULL)
        return TEGRA_2D_INVALID_PARAMETER;

    *height = surface->height;

    return TEGRA_2D_OK;
}

int tegra_2d_get_surface_format(const struct tegra_2d_surface *surface,
                                enum tegra_2d_format *format)
{
    if (surface == NULL)
        return TEGRA_2D_INVALID_PARAMETER;

    if (format == NULL)
        return TEGRA_2D_INVALID_PARAMETER;

    *format = surface->format;

    return TEGRA_2D_OK;
}

int tegra_2d_set_surface_format(struct tegra_2d_surface *surface,
                                enum tegra_2d_format format)
{
    if (surface == NULL)
        return TEGRA_2D_INVALID_PARAMETER;

    if (surface->format == format)
        return TEGRA_2D_OK;

    switch (surface->format) {
    case TEGRA_2D_FORMAT_16BIT:
    case TEGRA_2D_FORMAT_32BIT:
        break;
    default:
        return TEGRA_2D_INVALID_PARAMETER;
    }

    if (TEGRA_2D_FORMAT_BYTES(surface->format) != TEGRA_2D_FORMAT_BYTES(format))
        return TEGRA_2D_INVALID_PARAMETER;

    surface->format = format;

    return TEGRA_2D_OK;
}

int tegra_2d_surface_acquire_access(struct tegra_2d_context *ctx,
                                    struct tegra_2d_surface *surface,
                                    void **data)
{
    if (ctx == NULL)
        return TEGRA_2D_INVALID_PARAMETER;

    if (surface == NULL)
        return TEGRA_2D_INVALID_PARAMETER;

    if (data == NULL)
        return TEGRA_2D_INVALID_PARAMETER;

    if (!surface->data) {
        if (drm_tegra_bo_map(surface->bo, data)) {
            SET_ERROR("drm_tegra_bo_map() failed\n");
            return TEGRA_2D_MEMORY_FAILURE;
        }
    } else {
        *data = surface->data;
    }

    return TEGRA_2D_OK;
}

void tegra_2d_surface_release_access(const struct tegra_2d_surface *surface)
{
    if (surface->data)
        return;

    if (drm_tegra_bo_unmap(surface->bo))
        SET_ERROR("drm_tegra_bo_unmap() failed\n");
}

int tegra_2d_surface_get_bo(const struct tegra_2d_surface *surface,
                            struct drm_tegra_bo **bo)
{
    if (surface == NULL) {
        SET_ERROR("surface == NULL\n");
        return TEGRA_2D_INVALID_PARAMETER;
    }

    if (bo == NULL)
        return TEGRA_2D_INVALID_PARAMETER;

    if (surface->bo == NULL) {
        SET_ERROR("surface->bo == NULL\n");
        return TEGRA_2D_INVALID_BUFFER;
    }

    *bo = surface->bo;

    return TEGRA_2D_OK;
}
