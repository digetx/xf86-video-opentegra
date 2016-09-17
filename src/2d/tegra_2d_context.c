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
#include "tegra_2d_util.h"

struct tegra_2d_context *tegra_2d_open(int fd)
{
    struct tegra_2d_context *ctx;
    int result;

    ctx = calloc(1, sizeof(*ctx));
    if (ctx == NULL)
        return NULL;

    memset(ctx, 0, sizeof(*ctx));

    result = drm_tegra_new(&ctx->drm, fd);
    if (result != 0) {
        free(ctx);
        return NULL;
    }

    result = tegra_stream_create(ctx->drm, &ctx->stream, 0);
    if (result != TEGRA_2D_OK) {
        drm_tegra_close(ctx->drm);
        free(ctx);
        return NULL;
    }

    result = terga_2d_init_mem_allocator(&ctx->allocator, ctx->drm,
                                         48 * 1024 * 1024);
    if (result != TEGRA_2D_OK) {
        tegra_stream_destroy(&ctx->stream);
        drm_tegra_close(ctx->drm);
        free(ctx);
        return NULL;
    }

    g2fill_init(&ctx->g2fill);
    g2copy_init(&ctx->g2copy);
    frcopy_init(&ctx->frcopy);
    sbcopy_init(&ctx->sbcopy);

    return ctx;
}

void tegra_2d_close(struct tegra_2d_context *ctx)
{
    if (ctx) {
        tegra_stream_destroy(&ctx->stream);
        terga_2d_release_mem_allocator(&ctx->allocator);
        drm_tegra_close(ctx->drm);
        free(ctx);
    }
}

static void reset_context(struct tegra_2d_context *ctx)
{
    ctx->color = 0x00000000;
    ctx->rop   = TEGRA_2D_ROP_INVALID;
    ctx->blit  = TEGRA_2D_BLIT_INVALID;
    ctx->xdir  = 0;
    ctx->ydir  = 0;
}

int tegra_2d_begin(struct tegra_2d_context *ctx)
{
    if (ctx == NULL)
        return TEGRA_2D_INVALID_PARAMETER;

    reset_context(ctx);

    return tegra_stream_begin(&ctx->stream);
}

int tegra_2d_end(struct tegra_2d_context *ctx)
{
    if (ctx == NULL)
        return TEGRA_2D_INVALID_PARAMETER;

    tegra_stream_end(&ctx->stream);

    reset_context(ctx);

    return tegra_stream_flush(&ctx->stream);
}

int tegra_2d_ctx_set_rop(struct tegra_2d_context *ctx,
                         enum tegra_2d_rop rop)
{
    if (ctx == NULL)
        return TEGRA_2D_INVALID_PARAMETER;

    ctx->rop = rop;

    return TEGRA_2D_OK;
}

int tegra_2d_ctx_set_blit_op(struct tegra_2d_context *ctx,
                             enum tegra_2d_blit op)
{
    if (ctx == NULL)
        return TEGRA_2D_INVALID_PARAMETER;

    ctx->blit = op;

    return TEGRA_2D_OK;
}

int tegra_2d_ctx_set_xdir(struct tegra_2d_context *ctx, int enable)
{
    if (ctx == NULL)
        return TEGRA_2D_INVALID_PARAMETER;

    ctx->xdir = enable ? 1 : 0;

    return TEGRA_2D_OK;
}

int tegra_2d_ctx_set_ydir(struct tegra_2d_context *ctx, int enable)
{
    if (ctx == NULL)
        return TEGRA_2D_INVALID_PARAMETER;

    ctx->ydir = enable ? 1 : 0;

    return TEGRA_2D_OK;
}
