/*
 * Copyright (c) 2016 Dmitry Osipenko <digetx@gmail.com>
 * Copyright (C) 2012 NVIDIA Corporation.
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

#ifndef TEGRA_2D_H_
#define TEGRA_2D_H_

#include <stdint.h>
#include <stdio.h>

#include <libdrm/tegra.h>

struct tegra_2d_rect {
    float left;
    float top;
    float right;
    float bottom;
};

enum tegra_2d_layout {
    TEGRA_2D_LAYOUT_LINEAR,
    TEGRA_2D_LAYOUT_TILED,
};

#define TEGRA_2D_FORMAT(id, bpp) \
    ((id) << 16 | (bpp))

#define TEGRA_2D_FORMAT_BITS(f) \
    ((f) & 0xff)

#define TEGRA_2D_FORMAT_BYTES(f) \
    (TEGRA_2D_FORMAT_BITS(f) >> 3)

#define TEGRA_2D_FORMAT_ID(f) \
    (((f) >> 16) & 0xff)

enum tegra_2d_format {
    TEGRA_2D_FORMAT_8BIT     = TEGRA_2D_FORMAT(0,  8),
    TEGRA_2D_FORMAT_16BIT    = TEGRA_2D_FORMAT(0, 16),
    TEGRA_2D_FORMAT_32BIT    = TEGRA_2D_FORMAT(0, 32),
    TEGRA_2D_FORMAT_A8R8G8B8 = TEGRA_2D_FORMAT(1, 32),
    TEGRA_2D_FORMAT_A8B8G8R8 = TEGRA_2D_FORMAT(2, 32),
    TEGRA_2D_FORMAT_R8G8B8A8 = TEGRA_2D_FORMAT(3, 32),
    TEGRA_2D_FORMAT_B8G8R8A8 = TEGRA_2D_FORMAT(4, 32),
    TEGRA_2D_FORMAT_A4R4G4B4 = TEGRA_2D_FORMAT(5, 16),
    TEGRA_2D_FORMAT_R4G4B4A4 = TEGRA_2D_FORMAT(6, 16),
    TEGRA_2D_FORMAT_R5G6B5   = TEGRA_2D_FORMAT(7, 16),
    TEGRA_2D_FORMAT_A1R5G5B5 = TEGRA_2D_FORMAT(8, 16),
};

enum tegra_2d_transform {
    TEGRA_2D_TRANSFORM_IDENTITY,
    TEGRA_2D_TRANSFORM_ROT_90,
    TEGRA_2D_TRANSFORM_ROT_180,
    TEGRA_2D_TRANSFORM_ROT_270,
    TEGRA_2D_TRANSFORM_FLIP_X,
    TEGRA_2D_TRANSFORM_FLIP_Y,
    TEGRA_2D_TRANSFORM_TRANSPOSE,
    TEGRA_2D_TRANSFORM_INV_TRANSPOSE,
};

enum tegra_2d_rop {
    TEGRA_2D_ROP_INVALID,
    TEGRA_2D_ROP_CLEAR,
    TEGRA_2D_ROP_AND,
    TEGRA_2D_ROP_AND_REVERSE,
    TEGRA_2D_ROP_AND_INVERTED,
    TEGRA_2D_ROP_NAND,
    TEGRA_2D_ROP_COPY,
    TEGRA_2D_ROP_COPY_INVERTED,
    TEGRA_2D_ROP_XOR,
    TEGRA_2D_ROP_OR,
    TEGRA_2D_ROP_OR_REVERSE,
    TEGRA_2D_ROP_NOR,
    TEGRA_2D_ROP_EQUIV,
    TEGRA_2D_ROP_INVERT,
    TEGRA_2D_ROP_SET,
    TEGRA_2D_ROP_NOOP,
};

enum tegra_2d_blit {
    TEGRA_2D_BLIT_INVALID,
    TEGRA_2D_BLIT_SRC,
    TEGRA_2D_BLIT_DST,
    TEGRA_2D_BLIT_OVER,
    TEGRA_2D_BLIT_OVER_REVERSE,
    TEGRA_2D_BLIT_IN,
    TEGRA_2D_BLIT_IN_REVERSE,
};

enum tegra_2d_result {
    TEGRA_2D_OK,
    TEGRA_2D_INVALID_PARAMETER,
    TEGRA_2D_INVALID_RECT,
    TEGRA_2D_INVALID_BUFFER,
    TEGRA_2D_UNSUPPORTED_BLIT,
    TEGRA_2D_MEMORY_FAILURE,
    TEGRA_2D_STREAM_FAILURE,
    TEGRA_2D_CHANNEL_FAILURE,
    TEGRA_2D_DEVICE_FAILURE,
    TEGRA_2D_FILE_FAILURE,
};

struct tegra_2d_context;
struct tegra_2d_surface;

struct tegra_2d_context * tegra_2d_open(int fd);

void tegra_2d_close(struct tegra_2d_context *ctx);

int tegra_2d_begin(struct tegra_2d_context *ctx);

int tegra_2d_end(struct tegra_2d_context *ctx);

int tegra_2d_set_fill_color(struct tegra_2d_context *ctx,
                            uint32_t color);

int tegra_2d_ctx_set_rop(struct tegra_2d_context *ctx,
                         enum tegra_2d_rop rop);

int tegra_2d_ctx_set_blit_op(struct tegra_2d_context *ctx,
                             enum tegra_2d_blit op);

int tegra_2d_ctx_set_xdir(struct tegra_2d_context *ctx, int enable);

int tegra_2d_ctx_set_ydir(struct tegra_2d_context *ctx, int enable);

int tegra_2d_fill(struct tegra_2d_context *ctx,
                  const struct tegra_2d_surface *dst,
                  const struct tegra_2d_rect *dst_rect);

int tegra_2d_copy(struct tegra_2d_context *ctx,
                  const struct tegra_2d_surface *dst,
                  const struct tegra_2d_surface *src,
                  const struct tegra_2d_rect *dst_rect,
                  const struct tegra_2d_rect *src_rect,
                  enum tegra_2d_transform transform);

int tegra_2d_blit(struct tegra_2d_context *ctx,
                  const struct tegra_2d_surface *dst,
                  const struct tegra_2d_surface *src,
                  const struct tegra_2d_rect *dst_rect,
                  const struct tegra_2d_rect *src_rect,
                  enum tegra_2d_transform transform);

int tegra_2d_allocate_surface(struct tegra_2d_context *ctx,
                              struct tegra_2d_surface **surface,
                              int width, int height, int pitch,
                              enum tegra_2d_layout layout,
                              enum tegra_2d_format format,
                              struct drm_tegra_bo *bo);

int tegra_2d_free_surface(struct tegra_2d_context *ctx,
                          struct tegra_2d_surface **surface);

int tegra_2d_get_surface_pitch(const struct tegra_2d_surface *surface,
                               int *pitch);

int tegra_2d_get_surface_width(const struct tegra_2d_surface *surface,
                               int *width);

int tegra_2d_get_surface_height(const struct tegra_2d_surface *surface,
                                int *height);

int tegra_2d_get_surface_format(const struct tegra_2d_surface *surface,
                                enum tegra_2d_format *format);

int tegra_2d_set_surface_format(struct tegra_2d_surface *surface,
                                enum tegra_2d_format format);

int tegra_2d_surface_acquire_access(struct tegra_2d_context *ctx,
                                    struct tegra_2d_surface *surface,
                                    void **data);

void tegra_2d_surface_release_access(const struct tegra_2d_surface *surface);

#endif
