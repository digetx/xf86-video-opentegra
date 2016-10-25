/*
 * Copyright © 2014 NVIDIA Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sub license, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
 * IN NO EVENT SHALL TUNGSTEN GRAPHICS AND/OR ITS SUPPLIERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include "xf86.h"

#include "driver.h"
#include "exa.h"
#include "compat-api.h"

#include "2d/tegra_2d.h"
#include "tegra_pixmap.h"

/*
 * Alignment to 16 bytes isn't strictly necessary for all buffers, but
 * there are cases where X's software rendering fallbacks crash when a
 * buffer's pitch is too small (which happens for very small, low-bpp
 * pixmaps).
 */
#define TEGRA_SURFACE_ALIGN(width, bitsPerPixel)                        \
    ((((width * bitsPerPixel + 7) / 8) + (32) - 1) & ~((32) - 1))

#define ErrorMsg(fmt, args...)                                          \
                                                                        \
    xf86DrvMsg(pScrn->scrnIndex, X_ERROR, "%s:%d/%s(): " fmt,           \
               __FILE__, __LINE__, __func__, ##args);                   \

static struct tegra_2d_surface *surf_src_scratch;

static int TegraEXAMarkSync(ScreenPtr pScreen)
{
    return 0;
}

static void TegraEXAWaitMarker(ScreenPtr pScreen, int marker)
{
    /* TODO: implement */
}

static Bool TegraEXAPrepareAccess(PixmapPtr pPixmap, int index)
{
    ScrnInfoPtr pScrn           = xf86ScreenToScrn(pPixmap->drawable.pScreen);
    TegraEXAPtr tegraEXA        = TegraPTR(pScrn)->exa;
    TegraPixmapPtr tegra_pixmap = exaGetPixmapDriverPrivate(pPixmap);
    int result;

    if (tegra_pixmap->surf) {
        ErrorMsg("Surface 0x%08X\n", tegra_pixmap->surf);

        result = tegra_2d_surface_acquire_access(tegraEXA->ctx_2d,
                                                 tegra_pixmap->surf,
                                                 &pPixmap->devPrivate.ptr);
        if (result == TEGRA_2D_OK)
            return TRUE;
    }

    ErrorMsg("failed\n");

    return FALSE;
}

static void TegraEXAFinishAccess(PixmapPtr pPixmap, int index)
{
    ScrnInfoPtr pScrn           = xf86ScreenToScrn(pPixmap->drawable.pScreen);
    TegraPixmapPtr tegra_pixmap = exaGetPixmapDriverPrivate(pPixmap);

    if (tegra_pixmap->surf) {
        ErrorMsg("Surface 0x%08X\n", tegra_pixmap->surf);
        tegra_2d_surface_release_access(tegra_pixmap->surf);
    }
}

/*
 * Indicates whether pixmap is in stored in the "GPU" memory.
 */
static Bool TegraEXAPixmapIsOffscreen(PixmapPtr pPixmap)
{
    ScrnInfoPtr pScrn           = xf86ScreenToScrn(pPixmap->drawable.pScreen);
    TegraPixmapPtr tegra_pixmap = exaGetPixmapDriverPrivate(pPixmap);

    if (tegra_pixmap && tegra_pixmap->surf)
        return TRUE;

    ErrorMsg("pPixmap 0x%08X NOT offscreen\n", pPixmap);

    return FALSE;
}

static void *TegraEXACreatePixmap2(ScreenPtr pScreen, int width, int height,
                                   int depth, int usage_hint, int bitsPerPixel,
                                   int *new_fb_pitch)
{
    ScrnInfoPtr pScrn           = xf86ScreenToScrn(pScreen);
    TegraEXAPtr tegraEXA        = TegraPTR(pScrn)->exa;
    TegraPixmapPtr tegra_pixmap = calloc(1, sizeof(*tegra_pixmap));
    int pitch                   = TEGRA_SURFACE_ALIGN(width, bitsPerPixel);
    int format                  = -1;
    int result;

    ErrorMsg("width %d height %d bitsPerPixel %d depth %d\n",
             width, height, bitsPerPixel, depth);

    if (!tegra_pixmap)
        return NULL;

    tegra_pixmap->from_pool = (usage_hint != TEGRA_PIXMAP_USAGE_DRI);

    return tegra_pixmap;

    switch (depth) {
    case 16:
    case 32:
        break;
    default:
        /*
         * Tegra can't handle it.
         */
        if (depth > 0 && bitsPerPixel <= 0)
            goto no_alloc;
    }

    switch (bitsPerPixel) {
    case 8:
        format = TEGRA_2D_FORMAT_8BIT;
        break;
    case 16:
        format = TEGRA_2D_FORMAT_16BIT;
        break;
    case 32:
        format = TEGRA_2D_FORMAT_32BIT;
        break;
    default:
        /*
         * Tegra can't handle it.
         */
        break;
    }

    /*
     * That was a check for supported depth/bpp?
     */
    if (width <= 0 || height <= 0 || bitsPerPixel <= 0)
        goto no_alloc;

    if (format < 0)
        goto fallback;

    result = tegra_2d_allocate_surface(tegraEXA->ctx_2d,
                                       &tegra_pixmap->surf,
                                       width, height, 0,
                                       TEGRA_2D_LAYOUT_LINEAR,
                                       format,
                                       NULL,
                                       tegra_pixmap->from_pool);
    if (result != TEGRA_2D_OK) {
        ErrorMsg("tegra_2d_allocate_surface() failed %d\n", result);
        goto fallback;
    }

    tegra_2d_get_surface_pitch(tegra_pixmap->surf, new_fb_pitch);

    return tegra_pixmap;

fallback:
    tegra_pixmap->fallback_data = malloc(pitch * height);
no_alloc:
    *new_fb_pitch = pitch;

    return tegra_pixmap;
}

static void TegraEXADestroySurface(ScrnInfoPtr pScrn,
                                   TegraPixmapPtr tegra_pixmap)
{
    TegraEXAPtr tegraEXA = TegraPTR(pScrn)->exa;
    int result;

    free(tegra_pixmap->fallback_data);

    if (tegra_pixmap->surf) {
        result = tegra_2d_free_surface(tegraEXA->ctx_2d, &tegra_pixmap->surf);
        if (result)
            ErrorMsg("tegra_2d_free_surface() failed %d\n", result);
    }
}

static void TegraEXADestroyPixmap(ScreenPtr pScreen, void *driverPriv)
{
    ScrnInfoPtr pScrn           = xf86ScreenToScrn(pScreen);
    TegraPixmapPtr tegra_pixmap = driverPriv;

    ErrorMsg("Surface 0x%08X\n", tegra_pixmap->surf);

    TegraEXADestroySurface(pScrn, tegra_pixmap);
    free(tegra_pixmap);
}

static Bool TegraEXAModifyPixmapHeader(PixmapPtr pPixmap,
                                       int width, int height,
                                       int depth, int bitsPerPixel,
                                       int devKind, pointer pPixData)
{
    ScrnInfoPtr pScrn           = xf86ScreenToScrn(pPixmap->drawable.pScreen);
    TegraPtr tegra              = TegraPTR(pScrn);
    TegraEXAPtr tegraEXA        = TegraPTR(pScrn)->exa;
    TegraPixmapPtr tegra_pixmap = exaGetPixmapDriverPrivate(pPixmap);
    struct drm_tegra_bo *bo     = NULL;
    int format = -1;
    int pitch = 0;
    int result;

    result = miModifyPixmapHeader(pPixmap, width, height, depth, bitsPerPixel,
                                  devKind, pPixData);
    if (result == FALSE)
        return result;

    /*
     * The pixmap can't be used for hardware acceleration, so dispose of it.
     */
    if (pPixData && pPixData != drmmode_map_front_bo(&tegra->drmmode)) {
        pPixmap->devPrivate.ptr = pPixData;
        pPixmap->devKind = devKind;
        return FALSE;
    }

    /*
     * From videocore-exa.c
     *
     * Passed in values could be zero, indicating that existing values
     * should be kept.. miModifyPixmapHeader() will deal with that, but
     * we need to resync to ensure we have the right values in the rest
     * of this function.
     */
    width        = pPixmap->drawable.width;
    height       = pPixmap->drawable.height;
    depth        = pPixmap->drawable.depth;
    bitsPerPixel = pPixmap->drawable.bitsPerPixel;

    ErrorMsg("width %d height %d bitsPerPixel %d depth %d\n",
             width, height, bitsPerPixel, depth);

    if (width <= 0 || height <= 0 || bitsPerPixel <= 0)
        return TRUE;

    switch (bitsPerPixel) {
    case 8:
        format = TEGRA_2D_FORMAT_8BIT;
        break;
    case 16:
        format = TEGRA_2D_FORMAT_16BIT;
        break;
    case 32:
        format = TEGRA_2D_FORMAT_32BIT;
        break;
    default:
        /*
         * Tegra can't handle it.
         */
        goto fallback;
    }

    if (pPixData) {
        /*
         * Pixmap is a display framebuffer.
         */
        bo = tegra->drmmode.front_bo->tegra_bo;
        pitch = tegra->drmmode.front_bo->pitch;
    }

    result = tegra_2d_allocate_surface(tegraEXA->ctx_2d,
                                       &tegra_pixmap->surf,
                                       width, height, pitch,
                                       TEGRA_2D_LAYOUT_LINEAR,
                                       format,
                                       bo,
                                       tegra_pixmap->from_pool);
    if (result != TEGRA_2D_OK) {
        ErrorMsg("tegra_2d_allocate_surface() failed %d\n", result);
        /*
         * Fallback'ing is okay???
         */
        goto fallback;
    }

    tegra_pixmap->display = pPixData ? 1 : 0;

    tegra_2d_get_surface_pitch(tegra_pixmap->surf, &pPixmap->devKind);

    free(tegra_pixmap->fallback_data);
    tegra_pixmap->fallback_data = NULL;

    return TRUE;

fallback:
    ErrorMsg("Fallback width %d height %d pitch %d bitsPerPixel %d\n",
             width, height, pPixmap->devKind, bitsPerPixel);

    if (tegra_pixmap->surf) {
        result = tegra_2d_free_surface(tegraEXA->ctx_2d, &tegra_pixmap->surf);

        if (result != TEGRA_2D_OK)
            ErrorMsg("tegra_2d_free_surface() failed %d\n", result);
    }

//     if (pPixmap->devKind == 0)
        pPixmap->devKind = TEGRA_SURFACE_ALIGN(width, bitsPerPixel);

    tegra_pixmap->fallback_data = realloc(tegra_pixmap->fallback_data,
                                          pPixmap->devKind * height);

    if (tegra_pixmap->fallback_data == NULL)
        ErrorMsg("Fallback allocation failed\n");

    pPixmap->devPrivate.ptr = tegra_pixmap->fallback_data;

    /*
     * TRUE or FALSE???
     */
    return TRUE;
}

static Bool TegraEXAPrepareSolid(PixmapPtr pPixmap, int op, Pixel planemask,
                                 Pixel color)
{
    ScrnInfoPtr pScrn           = xf86ScreenToScrn(pPixmap->drawable.pScreen);
    TegraEXAPtr tegraEXA        = TegraPTR(pScrn)->exa;
    TegraPixmapPtr tegra_pixmap = exaGetPixmapDriverPrivate(pPixmap);
    int result;

    ErrorMsg("\n");
//         return FALSE;

    if (op != GXcopy)
        return FALSE;

    if (planemask != FB_ALLONES)
        return FALSE;

    if (tegra_pixmap->surf == NULL)
        return FALSE;

    result = tegra_2d_begin(tegraEXA->ctx_2d);
    if (result != TEGRA_2D_OK) {
        ErrorMsg("tegra_2d_begin() failed %d\n", result);
        return FALSE;
    }

    result = tegra_2d_ctx_set_rop(tegraEXA->ctx_2d, TEGRA_2D_ROP_COPY);
    if (result != TEGRA_2D_OK) {
        ErrorMsg("tegra_2d_ctx_set_rop() failed %d\n", result);
        goto fail;
    }

    result = tegra_2d_set_fill_color(tegraEXA->ctx_2d, color);
    if (result != TEGRA_2D_OK) {
        ErrorMsg("tegra_2d_set_fill_color() failed %d\n", result);
        goto fail;
    }

    return TRUE;
fail:
    result = tegra_2d_end(tegraEXA->ctx_2d);
    if (result != TEGRA_2D_OK)
        ErrorMsg("tegra_2d_end() failed %d\n", result);

    return FALSE;
}

static void TegraEXASolid(PixmapPtr pPixmap, int x1, int y1, int x2, int y2)
{
    ScrnInfoPtr pScrn           = xf86ScreenToScrn(pPixmap->drawable.pScreen);
    TegraEXAPtr tegraEXA        = TegraPTR(pScrn)->exa;
    TegraPixmapPtr tegra_pixmap = exaGetPixmapDriverPrivate(pPixmap);
    struct tegra_2d_rect rect;
    int result;

    ErrorMsg("x1 %d y1 %d x2 %d y2 %d bitsPerPixel %d depth %d\n",
             x1, y1, x2, y2,
             pPixmap->drawable.bitsPerPixel,
             pPixmap->drawable.depth);

    rect.left   = x1;
    rect.top    = y1;
    rect.right  = x2;
    rect.bottom = y2;

    result = tegra_2d_fill(tegraEXA->ctx_2d,
                           tegra_pixmap->surf,
                           &rect);
    if (result != TEGRA_2D_OK) {
        ErrorMsg("tegra_2d_fill() failed %d\n", result);

        if (result == TEGRA_2D_INVALID_RECT) {
            int width, height;
            tegra_2d_get_surface_width(tegra_pixmap->surf, &width);
            tegra_2d_get_surface_height(tegra_pixmap->surf, &height);
            ErrorMsg("x1 %d y1 %d x2 %d y2 %d width %d height %d\n",
                     x1, y1, x2, y2, width, height);
        }
    }
}

static void TegraEXADoneSolid(PixmapPtr pPixmap)
{
    ScrnInfoPtr pScrn    = xf86ScreenToScrn(pPixmap->drawable.pScreen);
    TegraEXAPtr tegraEXA = TegraPTR(pScrn)->exa;
    int result;

    ErrorMsg("\n");

    result = tegra_2d_end(tegraEXA->ctx_2d);
    if (result != TEGRA_2D_OK)
        ErrorMsg("tegra_2d_end() failed %d\n", result);
}

static Bool TegraEXAPrepareCopy(PixmapPtr pSrcPixmap, PixmapPtr pDstPixmap,
                                int reverse, int upsidedown, int op,
                                Pixel planemask)
{
    ScrnInfoPtr pScrn               = xf86ScreenToScrn(pDstPixmap->drawable.pScreen);
    TegraEXAPtr tegraEXA            = TegraPTR(pScrn)->exa;
    TegraPixmapPtr tegra_pixmap_src = exaGetPixmapDriverPrivate(pSrcPixmap);
    TegraPixmapPtr tegra_pixmap_dst = exaGetPixmapDriverPrivate(pDstPixmap);
    int result;

//     if (upsidedown < 0) {
//         ErrorMsg("upsidedown\n");
//         return FALSE;
//     }
//
//     if (reverse < 0) {
//         ErrorMsg("reverse\n");
//         return FALSE;
//     }

    if (op != GXcopy) {
        ErrorMsg("op != GXcopy\n");
        return FALSE;
    }

    if (planemask != FB_ALLONES) {
        ErrorMsg("planemask != FB_ALLONES\n");
        return FALSE;
    }

    if (tegra_pixmap_src->surf == NULL) {
        ErrorMsg("src is fallback\n");
        return FALSE;
    }

    if (tegra_pixmap_dst->surf == NULL) {
        ErrorMsg("dst is fallback\n");
        return FALSE;
    }

    result = tegra_2d_begin(tegraEXA->ctx_2d);
    if (result != TEGRA_2D_OK) {
        ErrorMsg("tegra_2d_begin() failed %d\n", result);
        return FALSE;
    }

    result = tegra_2d_ctx_set_rop(tegraEXA->ctx_2d, TEGRA_2D_ROP_COPY);
    if (result != TEGRA_2D_OK) {
        ErrorMsg("tegra_2d_ctx_set_rop() failed %d\n", result);
        goto fail;
    }

    ErrorMsg("SRC%s 0x%08X: bitsPerPixel %d depth %d width %d\theight %d\t   DST%s 0x%08X: bitsPerPixel %d depth %d width %d\theight %d\n",
             tegra_pixmap_src->display ? " (display)" : " (pixmap )", tegra_pixmap_src,
             pSrcPixmap->drawable.bitsPerPixel, pSrcPixmap->drawable.depth,
             pSrcPixmap->drawable.width, pSrcPixmap->drawable.height,
             tegra_pixmap_dst->display ? " (display)" : " (pixmap )", tegra_pixmap_dst,
             pDstPixmap->drawable.bitsPerPixel, pDstPixmap->drawable.depth,
             pDstPixmap->drawable.width, pDstPixmap->drawable.height);

    surf_src_scratch = tegra_pixmap_src->surf;

    return TRUE;
fail:
    result = tegra_2d_end(tegraEXA->ctx_2d);
    if (result != TEGRA_2D_OK)
        ErrorMsg("tegra_2d_end() failed %d\n", result);

    return FALSE;
}

static void TegraEXACopy(PixmapPtr pDstPixmap,
                         int srcX, int srcY,
                         int dstX, int dstY,
                         int width, int height)
{
    ScrnInfoPtr pScrn               = xf86ScreenToScrn(pDstPixmap->drawable.pScreen);
    TegraEXAPtr tegraEXA            = TegraPTR(pScrn)->exa;
    TegraPixmapPtr tegra_pixmap_dst = exaGetPixmapDriverPrivate(pDstPixmap);
    struct tegra_2d_rect src_rect;
    struct tegra_2d_rect dst_rect;
    int result;

    ErrorMsg("DST%s 0x%08X: bitsPerPixel %d depth %d width %d\theight %d\tsrcX %d\tsrcY %d\tdstX %d\tdstY %d\n",
             tegra_pixmap_dst->display ? " (display)" : "", tegra_pixmap_dst,
             pDstPixmap->drawable.bitsPerPixel, pDstPixmap->drawable.depth,
             width, height, srcX, srcY, dstX, dstY);

    result = tegra_2d_ctx_set_xdir(tegraEXA->ctx_2d, dstX > srcX);
    if (result != TEGRA_2D_OK)
        ErrorMsg("tegra_2d_ctx_set_xdir() failed %d\n", result);

    result = tegra_2d_ctx_set_ydir(tegraEXA->ctx_2d, dstY > srcY);
    if (result != TEGRA_2D_OK)
        ErrorMsg("tegra_2d_ctx_set_ydir() failed %d\n", result);

    src_rect.left   = srcX;
    src_rect.top    = srcY;
    src_rect.right  = srcX + width;
    src_rect.bottom = srcY + height;

    dst_rect.left   = dstX;
    dst_rect.top    = dstY;
    dst_rect.right  = dstX + width;
    dst_rect.bottom = dstY + height;

    result = tegra_2d_copy(tegraEXA->ctx_2d,
                           tegra_pixmap_dst->surf,
                           surf_src_scratch,
                           &dst_rect,
                           &src_rect,
                           TEGRA_2D_TRANSFORM_IDENTITY);
    if (result != TEGRA_2D_OK)
        ErrorMsg("tegra_2d_copy() failed %d\n", result);
}

static void TegraEXADoneCopy(PixmapPtr pDstPixmap)
{
    ScrnInfoPtr pScrn    = xf86ScreenToScrn(pDstPixmap->drawable.pScreen);
    TegraEXAPtr tegraEXA = TegraPTR(pScrn)->exa;
    int result;

    result = tegra_2d_end(tegraEXA->ctx_2d);
    if (result != TEGRA_2D_OK)
        ErrorMsg("tegra_2d_end() failed %d\n", result);
}

static Bool TegraEXAPrepareComposite(int op,
                                     PicturePtr pSrcPicture,
                                     PicturePtr pMaskPicture,
                                     PicturePtr pDstPicture,
                                     PixmapPtr pSrc,
                                     PixmapPtr pMask,
                                     PixmapPtr pDst)
{
    ScrnInfoPtr pScrn               = xf86ScreenToScrn(pDst->drawable.pScreen);
    TegraEXAPtr tegraEXA            = TegraPTR(pScrn)->exa;
    TegraPixmapPtr tegra_pixmap_src = pSrc ? exaGetPixmapDriverPrivate(pSrc) : NULL;
    TegraPixmapPtr tegra_pixmap_dst = exaGetPixmapDriverPrivate(pDst);
    int result = TEGRA_2D_OK;
    int blit;

    /* Alpha blitting doesn't work, yet */
     return FALSE;

    if (pSrc == NULL) {
        ErrorMsg("pSrc == NULL\n");
        return FALSE;
    }

    if (tegra_pixmap_src->surf == NULL) {
        ErrorMsg("src is fallback\n");
        return FALSE;
    }

    if (tegra_pixmap_dst->surf == NULL) {
        ErrorMsg("dst is fallback\n");
        return FALSE;
    }

    if (pMask != NULL) {
        ErrorMsg("pMask\n");
        return FALSE;
    }

    if (pSrcPicture->repeat) {
        ErrorMsg("pSrcPicture->repeat\n");
        return FALSE;
    }

    switch (pSrcPicture->format) {
    case PICT_a8r8g8b8:
    case PICT_x8r8g8b8:
        result = tegra_2d_set_surface_format(tegra_pixmap_src->surf, TEGRA_2D_FORMAT_A8R8G8B8);
        break;
    case PICT_a8b8g8r8:
    case PICT_x8b8g8r8:
        result = tegra_2d_set_surface_format(tegra_pixmap_src->surf, TEGRA_2D_FORMAT_A8B8G8R8);
        break;
    case PICT_b8g8r8a8:
    case PICT_b8g8r8x8:
        result = tegra_2d_set_surface_format(tegra_pixmap_src->surf, TEGRA_2D_FORMAT_B8G8R8A8);
        break;
    case PICT_r5g6b5:
    case PICT_b5g6r5:
        result = tegra_2d_set_surface_format(tegra_pixmap_src->surf, TEGRA_2D_FORMAT_R5G6B5);
        break;
    case PICT_a1r5g5b5:
    case PICT_x1r5g5b5:
        result = tegra_2d_set_surface_format(tegra_pixmap_src->surf, TEGRA_2D_FORMAT_A1R5G5B5);
        break;
    default:
        ErrorMsg("pSrcPicture->format\n");
        return FALSE;
    }

    if (result != TEGRA_2D_OK) {
        ErrorMsg("tegra_2d_set_surface_format() failed %d\n", result);
        return FALSE;
    }

    switch (pDstPicture->format) {
    case PICT_a8r8g8b8:
    case PICT_x8r8g8b8:
        result = tegra_2d_set_surface_format(tegra_pixmap_dst->surf, TEGRA_2D_FORMAT_A8R8G8B8);
        break;
    case PICT_a8b8g8r8:
    case PICT_x8b8g8r8:
        result = tegra_2d_set_surface_format(tegra_pixmap_dst->surf, TEGRA_2D_FORMAT_A8B8G8R8);
        break;
    case PICT_b8g8r8a8:
    case PICT_b8g8r8x8:
        result = tegra_2d_set_surface_format(tegra_pixmap_dst->surf, TEGRA_2D_FORMAT_B8G8R8A8);
        break;
    case PICT_r5g6b5:
    case PICT_b5g6r5:
        result = tegra_2d_set_surface_format(tegra_pixmap_dst->surf, TEGRA_2D_FORMAT_R5G6B5);
        break;
    case PICT_a1r5g5b5:
    case PICT_x1r5g5b5:
        result = tegra_2d_set_surface_format(tegra_pixmap_dst->surf, TEGRA_2D_FORMAT_A1R5G5B5);
        break;
    default:
        ErrorMsg("pDstPicture->format\n");
        return FALSE;
    }

    if (result != TEGRA_2D_OK) {
        ErrorMsg("tegra_2d_set_surface_format() failed %d\n", result);
        return FALSE;
    }

    switch (op) {
    case PictOpSrc:
        ErrorMsg("PictOpSrc\n");
        blit = TEGRA_2D_BLIT_SRC;
        break;
    case PictOpDst:
        ErrorMsg("PictOpDst\n");
        blit = TEGRA_2D_BLIT_DST;
        break;
    case PictOpOver:
        ErrorMsg("PictOpOver\n");

        switch (pSrcPicture->format) {
        case PICT_a8r8g8b8:
        case PICT_x8r8g8b8:
            break;
        default:
            ErrorMsg("pSrcPicture->format\n");
            return FALSE;
        }

        switch (pDstPicture->format) {
        case PICT_a8r8g8b8:
        case PICT_x8r8g8b8:
            break;
        default:
            ErrorMsg("pSrcPicture->format\n");
            return FALSE;
        }

        blit = TEGRA_2D_BLIT_OVER;
        break;
        /* TODO: implement */
     case PictOpOverReverse:
        ErrorMsg("PictOpOverReverse\n");
        return FALSE;
     case PictOpIn:
        ErrorMsg("PictOpIn\n");
        return FALSE;
     case PictOpInReverse:
        ErrorMsg("PictOpInReverse\n");
        return FALSE;
     case PictOpOut:
        ErrorMsg("PictOpOut\n");
        return FALSE;
     case PictOpOutReverse:
        ErrorMsg("PictOpOutReverse\n");
        return FALSE;
     case PictOpAtop:
        ErrorMsg("PictOpAtop\n");
        return FALSE;
     case PictOpAtopReverse:
        ErrorMsg("PictOpAtopReverse\n");
        return FALSE;
     case PictOpXor:
        ErrorMsg("PictOpXor\n");
        return FALSE;
     case PictOpAdd:
        ErrorMsg("PictOpAdd\n");
        return FALSE;
     case PictOpSaturate:
        ErrorMsg("PictOpSaturate\n");
        return FALSE;
    default:
        return FALSE;
    }

    return TRUE;

    result = tegra_2d_begin(tegraEXA->ctx_2d);
    if (result != TEGRA_2D_OK) {
        ErrorMsg("tegra_2d_begin() failed %d\n", result);
        return FALSE;
    }

    result = tegra_2d_ctx_set_blit_op(tegraEXA->ctx_2d, blit);
    if (result != TEGRA_2D_OK) {
        ErrorMsg("tegra_2d_ctx_set_blit_op() failed %d\n", result);
        goto fail;
    }

    ErrorMsg("SRC: bitsPerPixel %d depth %d    DST: bitsPerPixel %d depth %d width %d height %d\n",
             pSrc->drawable.bitsPerPixel, pSrc->drawable.depth,
             pDst->drawable.bitsPerPixel, pDst->drawable.depth,
             pSrc->drawable.width, pSrc->drawable.height);

    surf_src_scratch = tegra_pixmap_src->surf;

    return TRUE;
fail:
    result = tegra_2d_end(tegraEXA->ctx_2d);
    if (result != TEGRA_2D_OK)
        ErrorMsg("tegra_2d_end() failed %d\n", result);

    return FALSE;
}

static void TegraEXAComposite(PixmapPtr pDst, int srcX, int srcY, int maskX,
                              int maskY, int dstX, int dstY,
                              int width, int height)
{
    ScrnInfoPtr pScrn               = xf86ScreenToScrn(pDst->drawable.pScreen);
    TegraEXAPtr tegraEXA            = TegraPTR(pScrn)->exa;
    TegraPixmapPtr tegra_pixmap_dst = exaGetPixmapDriverPrivate(pDst);
    struct tegra_2d_rect src_rect;
    struct tegra_2d_rect dst_rect;
    int result;

    ErrorMsg("DST%s 0x%08X: bitsPerPixel %d depth %d width %d\theight %d\tsrcX %d\tsrcY %d\tdstX %d\tdstY %d\n",
             tegra_pixmap_dst->display ? " (display)" : "", tegra_pixmap_dst,
             pDst->drawable.bitsPerPixel, pDst->drawable.depth,
             width, height, srcX, srcY, dstX, dstY);

    src_rect.left   = srcX;
    src_rect.top    = srcY;
    src_rect.right  = srcX + width;
    src_rect.bottom = srcY + height;

    dst_rect.left   = dstX;
    dst_rect.top    = dstY;
    dst_rect.right  = dstX + width;
    dst_rect.bottom = dstY + height;

    result = tegra_2d_copy(tegraEXA->ctx_2d,
                           tegra_pixmap_dst->surf,
                           surf_src_scratch,
                           &dst_rect,
                           &src_rect,
                           TEGRA_2D_TRANSFORM_IDENTITY);
    if (result != TEGRA_2D_OK)
        ErrorMsg("tegra_2d_copy() failed %d\n", result);
}

static void TegraEXADoneComposite(PixmapPtr pDstPixmap)
{
    ScrnInfoPtr pScrn    = xf86ScreenToScrn(pDstPixmap->drawable.pScreen);
    TegraEXAPtr tegraEXA = TegraPTR(pScrn)->exa;
    int result;

    result = tegra_2d_end(tegraEXA->ctx_2d);
    if (result != TEGRA_2D_OK)
        ErrorMsg("tegra_2d_end() failed %d\n", result);
}

void TegraEXAScreenInit(ScreenPtr pScreen)
{
    ScrnInfoPtr pScrn = xf86ScreenToScrn(pScreen);
    TegraPtr tegra = TegraPTR(pScrn);
    ExaDriverPtr exa = NULL;
    TegraEXAPtr priv = NULL;

    exa = exaDriverAlloc();
    if (!exa) {
        ErrorMsg("EXA allocation failed\n");
        goto err_cleanup;
    }

    priv = calloc(1, sizeof(*priv));
    if (!priv) {
        ErrorMsg("EXA allocation failed\n");
        goto err_cleanup;
    }

    priv->ctx_2d = tegra_2d_open(tegra->fd);
    if (!priv->ctx_2d) {
        ErrorMsg("Tegra 2d context creation failed\n");
        goto err_cleanup;
    }

    exa->exa_major          = EXA_VERSION_MAJOR;
    exa->exa_minor          = EXA_VERSION_MINOR;
    exa->pixmapOffsetAlign  = 1;
    exa->pixmapPitchAlign   = 32;
    exa->flags              =   EXA_SUPPORTS_OFFSCREEN_OVERLAPS
                              | EXA_SUPPORTS_PREPARE_AUX
                              | EXA_OFFSCREEN_PIXMAPS
                              | EXA_HANDLES_PIXMAPS;
    exa->maxX               = 8192;
    exa->maxY               = 8192;

    exa->MarkSync           = TegraEXAMarkSync;
    exa->WaitMarker         = TegraEXAWaitMarker;

    exa->PrepareAccess      = TegraEXAPrepareAccess;
    exa->FinishAccess       = TegraEXAFinishAccess;
    exa->PixmapIsOffscreen  = TegraEXAPixmapIsOffscreen;

    exa->CreatePixmap2      = TegraEXACreatePixmap2;
    exa->DestroyPixmap      = TegraEXADestroyPixmap;
    exa->ModifyPixmapHeader = TegraEXAModifyPixmapHeader;

    exa->PrepareSolid       = TegraEXAPrepareSolid;
    exa->Solid              = TegraEXASolid;
    exa->DoneSolid          = TegraEXADoneSolid;

    exa->PrepareCopy        = TegraEXAPrepareCopy;
    exa->Copy               = TegraEXACopy;
    exa->DoneCopy           = TegraEXADoneCopy;

    exa->CheckComposite     = NULL;
    exa->PrepareComposite   = TegraEXAPrepareComposite;
    exa->Composite          = TegraEXAComposite;
    exa->DoneComposite      = TegraEXADoneComposite;

    if (!exaDriverInit(pScreen, exa)) {
        ErrorMsg("EXA initialization failed\n");
        goto err_cleanup;
    }

    priv->driver = exa;
    tegra->exa = priv;

    return;

err_cleanup:
    if (priv && priv->ctx_2d)
        tegra_2d_close(priv->ctx_2d);
    free(priv);
    free(exa);
}

void TegraEXAScreenExit(ScreenPtr pScreen)
{
    ScrnInfoPtr pScrn = xf86ScreenToScrn(pScreen);
    TegraPtr tegra = TegraPTR(pScrn);
    TegraEXAPtr priv = tegra->exa;

    if (priv) {
        exaDriverFini(pScreen);
        free(priv->driver);

        tegra_2d_close(priv->ctx_2d);
        free(priv);
    }
}

/* vim: set et sts=4 sw=4 ts=4: */
