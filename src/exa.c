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

#include "host1x.h"
#include "driver.h"
#include "exa.h"
#include "compat-api.h"

#define EXA_ALIGN(offset, align) \
    (((offset) + (align) - 1) & ~((align) - 1))

#define ROP3_SRCCOPY 0xcc

#define ErrorMsg(fmt, args...) \
    xf86DrvMsg(pScrn->scrnIndex, X_ERROR, "%s:%d/%s(): " #fmt, __FILE__, \
               __LINE__, __func__, ##args)

static const uint8_t rop3[] = {
    0x00, /* GXclear */
    0x88, /* GXand */
    0x44, /* GXandReverse */
    0xcc, /* GXcopy */
    0x22, /* GXandInverted */
    0xaa, /* GXnoop */
    0x66, /* GXxor */
    0xee, /* GXor */
    0x11, /* GXnor */
    0x99, /* GXequiv */
    0x55, /* GXinvert */
    0xdd, /* GXorReverse */
    0x33, /* GXcopyInverted */
    0xbb, /* GXorInverted */
    0x77, /* GXnand */
    0xff, /* GXset */
};

typedef struct {
    struct drm_tegra_bo *bo;
} TegraPixmapRec, *TegraPixmapPtr;

static inline unsigned int TegraEXAPitch(unsigned int width, unsigned int bpp)
{
    unsigned int pitch;

    /*
     * Alignment to 16 bytes isn't strictly necessary for all buffers, but
     * there are cases where X's software rendering fallbacks crash when a
     * buffer's pitch is too small (which happens for very small, low-bpp
     * pixmaps).
     */
    pitch = EXA_ALIGN(width * bpp / 8, 16);
    if (!pitch)
        pitch = 16;

    return pitch;
}

static int TegraEXAMarkSync(ScreenPtr pScreen)
{
    /* TODO: implement */

    return 0;
}

static void TegraEXAWaitMarker(ScreenPtr pScreen, int marker)
{
    /* TODO: implement */
}

static Bool TegraEXAPrepareAccess(PixmapPtr pPix, int index)
{
    ScrnInfoPtr pScrn = xf86ScreenToScrn(pPix->drawable.pScreen);
    TegraPixmapPtr priv = exaGetPixmapDriverPrivate(pPix);
    int err;

    err = drm_tegra_bo_map(priv->bo, &pPix->devPrivate.ptr);
    if (err < 0) {
        ErrorMsg("failed to map buffer object: %d\n", err);
        return FALSE;
    }

    return TRUE;
}

static void TegraEXAFinishAccess(PixmapPtr pPix, int index)
{
    ScrnInfoPtr pScrn = xf86ScreenToScrn(pPix->drawable.pScreen);
    TegraPixmapPtr priv = exaGetPixmapDriverPrivate(pPix);
    int err;

    err = drm_tegra_bo_unmap(priv->bo);
    if (err < 0)
        ErrorMsg("failed to unmap buffer object: %d\n", err);
}

static Bool TegraEXAPixmapIsOffscreen(PixmapPtr pPix)
{
    TegraPixmapPtr priv = exaGetPixmapDriverPrivate(pPix);

    return priv && priv->bo;
}

static void *TegraEXACreatePixmap2(ScreenPtr pScreen, int width, int height,
                                   int depth, int usage_hint, int bitsPerPixel,
                                   int *new_fb_pitch)
{
    TegraPixmapPtr pixmap;

    pixmap = calloc(1, sizeof(*pixmap));
    if (!pixmap)
        return NULL;

    /*
     * Alignment to 16 bytes isn't strictly necessary for all buffers, but
     * there are cases where X's software rendering fallbacks crash when a
     * buffer's pitch is too small (which happens for very small, low-bpp
     * pixmaps).
     */
    *new_fb_pitch = TegraEXAPitch(width, bitsPerPixel);

    return pixmap;
}

static void TegraEXADestroyPixmap(ScreenPtr pScreen, void *driverPriv)
{
    TegraPixmapPtr priv = driverPriv;

    drm_tegra_bo_unref(priv->bo);
    free(priv);
}

static Bool TegraEXAModifyPixmapHeader(PixmapPtr pPixmap, int width,
                                       int height, int depth, int bitsPerPixel,
                                       int devKind, pointer pPixData)
{
    ScrnInfoPtr pScrn = xf86ScreenToScrn(pPixmap->drawable.pScreen);
    TegraPixmapPtr priv = exaGetPixmapDriverPrivate(pPixmap);
    TegraPtr tegra = TegraPTR(pScrn);
    unsigned int bpp, stride, size;
    Bool ret;
    int err;

    ret = miModifyPixmapHeader(pPixmap, width, height, depth, bitsPerPixel,
                               devKind, pPixData);
    if (!ret)
        return ret;

    if (pPixData) {
        void *scanout;

        scanout = drmmode_map_front_bo(&tegra->drmmode);

        if (pPixData == scanout) {
            uint32_t handle = tegra->drmmode.front_bo->handle;

            size = pPixmap->drawable.width * pPixmap->drawable.height *
                   pPixmap->drawable.bitsPerPixel / 8;

            err = drm_tegra_bo_wrap(&priv->bo, tegra->drm, handle, 0, size);
            if (err < 0)
                return FALSE;

            return TRUE;
        }

        /*
         * The pixmap can't be used for hardware acceleration, so dispose of
         * it.
         */
        pPixmap->devPrivate.ptr = pPixData;
        pPixmap->devKind = devKind;

        drm_tegra_bo_unref(priv->bo);
        priv->bo = NULL;

        return FALSE;
    }

    width = pPixmap->drawable.width;
    height = pPixmap->drawable.height;
    depth = pPixmap->drawable.depth;
    bpp = pPixmap->drawable.bitsPerPixel;
    stride = TegraEXAPitch(width, bpp);
    size = stride * height;

    if (priv->bo) {
        drm_tegra_bo_unref(priv->bo);
        priv->bo = NULL;
    }

    if (!priv->bo) {
        err = drm_tegra_bo_new(&priv->bo, tegra->drm, 0, size);
        if (err < 0) {
            ErrorMsg("failed to allocate %ux%u (%zu) buffer object: %d\n",
                     width, height, size, err);
            return FALSE;
        }
    }

    return TRUE;
}

static Bool TegraEXAPrepareSolid(PixmapPtr pPixmap, int op, Pixel planemask,
                                 Pixel color)
{
    ScrnInfoPtr pScrn = xf86ScreenToScrn(pPixmap->drawable.pScreen);
    TegraPixmapPtr priv = exaGetPixmapDriverPrivate(pPixmap);
    unsigned int bpp = pPixmap->drawable.bitsPerPixel;
    TegraEXAPtr tegra = TegraPTR(pScrn)->exa;
    struct drm_tegra_pushbuf *pb;
    uint32_t value;
    int err;

    if (!tegra->gr2d)
        return FALSE;

    /*
     * It should be possible to support all GX* raster operations given the
     * mapping in the rop3 table, but none other than GXcopy have been
     * validated.
     */
    if (op != GXcopy)
        return FALSE;

    /*
     * Support only 32-bit fills for now. Adding support for 16-bit fills
     * should be easy.
     */
    if (bpp != 32 && bpp != 16)
        return FALSE;

    err = drm_tegra_job_new(&tegra->job, tegra->gr2d);
    if (err < 0)
        return FALSE;

    err = drm_tegra_pushbuf_new(&tegra->pushbuf, tegra->job, tegra->bo, 0);
    if (err < 0)
        goto free_job;

    pb = tegra->pushbuf;

    *pb->ptr++ = HOST1X_OPCODE_SETCL(0, HOST1X_CLASS_GR2D, 0);
    *pb->ptr++ = HOST1X_OPCODE_MASK(0x9, 0x9);
    *pb->ptr++ = 0x0000003a;
    *pb->ptr++ = 0x00000000;

    *pb->ptr++ = HOST1X_OPCODE_NONINCR(0x35, 1);
    *pb->ptr++ = color;

    /* 4 or 2 bytes per pixel */
    if (bpp == 32)
        value = 2 << 16;
    else
        value = 1 << 16;

    /* fill mode, turbo-fill */
    value |= (1 << 6) | (1 << 2);

    *pb->ptr++ = HOST1X_OPCODE_MASK(0x1e, 0x7);
    *pb->ptr++ = 0x00000000; /* controlsecond */
    *pb->ptr++ = value; /* controlmain */
    *pb->ptr++ = rop3[op]; /* ropfade */

    *pb->ptr++ = HOST1X_OPCODE_MASK(0x2b, 0x9);

    err = drm_tegra_pushbuf_relocate(pb, priv->bo, 0, 0);
    if (err < 0)
        goto free_job;

    *pb->ptr++ = 0xdeadbeef;
    *pb->ptr++ = exaGetPixmapPitch(pPixmap);

    /* non-tiled */
    *pb->ptr++ = HOST1X_OPCODE_NONINCR(0x46, 1);
    *pb->ptr++ = 0;

    return TRUE;

free_job:
    drm_tegra_job_free(tegra->job);
    tegra->pushbuf = NULL;
    tegra->job = NULL;

    return FALSE;
}

static void TegraEXASolid(PixmapPtr pPixmap, int x1, int y1, int x2, int y2)
{
    ScrnInfoPtr pScrn = xf86ScreenToScrn(pPixmap->drawable.pScreen);
    TegraEXAPtr tegra = TegraPTR(pScrn)->exa;
    struct drm_tegra_pushbuf *pb;
    int err;

    pb = tegra->pushbuf;

    *pb->ptr++ = HOST1X_OPCODE_MASK(0x38, 0x5);
    *pb->ptr++ = (y2 - y1) << 16 | (x2 - x1);
    *pb->ptr++ = y1 << 16 | x1;

    err = drm_tegra_pushbuf_sync(pb, DRM_TEGRA_SYNCPT_COND_OP_DONE);
    if (err < 0)
        ErrorMsg("failed to insert syncpoint increment: %d\n", err);
}

static void TegraEXADoneSolid(PixmapPtr pPixmap)
{
    ScrnInfoPtr pScrn = xf86ScreenToScrn(pPixmap->drawable.pScreen);
    TegraEXAPtr tegra = TegraPTR(pScrn)->exa;
    struct drm_tegra_fence *fence;
    int err;

    err = drm_tegra_job_submit(tegra->job, &fence);
    if (err < 0) {
        ErrorMsg("failed to submit job: %d\n", err);
        goto free_job;
    }

    err = drm_tegra_fence_wait(fence);
    if (err < 0)
        ErrorMsg("failed to wait for fence: %d\n", err);

    drm_tegra_fence_free(fence);

free_job:
    drm_tegra_job_free(tegra->job);
    tegra->pushbuf = NULL;
    tegra->job = NULL;
}

static Bool TegraEXAPrepareCopy(PixmapPtr pSrcPixmap, PixmapPtr pDstPixmap,
                                int dx, int dy, int op, Pixel planemask)
{
    ScrnInfoPtr pScrn = xf86ScreenToScrn(pDstPixmap->drawable.pScreen);
    TegraPixmapPtr src = exaGetPixmapDriverPrivate(pSrcPixmap);
    TegraPixmapPtr dst = exaGetPixmapDriverPrivate(pDstPixmap);
    TegraEXAPtr tegra = TegraPTR(pScrn)->exa;
    struct drm_tegra_pushbuf *pb;
    int err;

    if (!tegra->gr2d)
        return FALSE;

    /*
     * It should be possible to support all GX* raster operations given the
     * mapping in the rop3 table, but none other than GXcopy have been
     * validated.
     */
    if (op != GXcopy)
        return FALSE;

    /*
     * Support only 32-bit to 32-bit copies for now. The hardware should be
     * able to do 32-bit to 16-bit copies as well, but some restrictions
     * apply.
     */
    if (pSrcPixmap->drawable.bitsPerPixel != 32 ||
        pDstPixmap->drawable.bitsPerPixel != 32)
        return FALSE;

    err = drm_tegra_job_new(&tegra->job, tegra->gr2d);
    if (err < 0) {
        ErrorMsg("failed to create job: %d\n", err);
        return FALSE;
    }

    err = drm_tegra_pushbuf_new(&tegra->pushbuf, tegra->job, tegra->bo, 0);
    if (err < 0) {
        ErrorMsg("failed to create push buffer: %d\n", err);
        goto free_job;
    }

    pb = tegra->pushbuf;

    *pb->ptr++ = HOST1X_OPCODE_SETCL(0, HOST1X_CLASS_GR2D, 0);
    *pb->ptr++ = HOST1X_OPCODE_MASK(0x9, 0x9);
    *pb->ptr++ = 0x0000003a;
    *pb->ptr++ = 0x00000000;

    *pb->ptr++ = HOST1X_OPCODE_MASK(0x01e, 0x5);
    *pb->ptr++ = 0x00000000; /* controlsecond */
    *pb->ptr++ = rop3[op]; /* ropfade */

    *pb->ptr++ = HOST1X_OPCODE_NONINCR(0x046, 1);
    /*
     * [20:20] destination write tile mode (0: linear, 1: tiled)
     * [ 0: 0] tile mode Y/RGB (0: linear, 1: tiled)
     */
    *pb->ptr++ = 0x00000000; /* tilemode */

    *pb->ptr++ = HOST1X_OPCODE_MASK(0x2b, 0x149);

    err = drm_tegra_pushbuf_relocate(pb, dst->bo, 0, 0);
    if (err < 0) {
        ErrorMsg("destination pixmap relocation failed: %d\n", err);
        goto free_job;
    }

    *pb->ptr++ = 0xdeadbeef; /* dstba */
    *pb->ptr++ = exaGetPixmapPitch(pDstPixmap); /* dstst */

    err = drm_tegra_pushbuf_relocate(pb, src->bo, 0, 0);
    if (err < 0) {
        ErrorMsg("source pixmap relocation failed: %d\n", err);
        goto free_job;
    }

    *pb->ptr++ = 0xdeadbeef; /* srcba */
    *pb->ptr++ = exaGetPixmapPitch(pSrcPixmap); /* srcst */

    return TRUE;

free_job:
    drm_tegra_job_free(tegra->job);
    tegra->pushbuf = NULL;
    tegra->job = NULL;

    return FALSE;
}

static void TegraEXACopy(PixmapPtr pDstPixmap, int srcX, int srcY, int dstX,
                         int dstY, int width, int height)
{
    ScrnInfoPtr pScrn = xf86ScreenToScrn(pDstPixmap->drawable.pScreen);
    TegraEXAPtr tegra = TegraPTR(pScrn)->exa;
    struct drm_tegra_pushbuf *pb;
    int err;
    uint32_t controlmain;

    pb = tegra->pushbuf;

    /*
     * [20:20] source color depth (0: mono, 1: same)
     * [17:16] destination color depth (0: 8 bpp, 1: 16 bpp, 2: 32 bpp)
     * [10:10] y-direction (0: increment, 1: decrement)
     * [9:9] x-direction (0: increment, 1: decrement)
     */
    controlmain = (1 << 20) | (2 << 16);

    if (dstX > srcX) {
        controlmain |= 1 << 9;
        srcX += width - 1;
        dstX += width - 1;
    }

    if (dstY > srcY) {
        controlmain |= 1 << 10;
        srcY += height - 1;
        dstY += height - 1;
    }

    *pb->ptr++ = HOST1X_OPCODE_INCR(0x01f, 1);
    *pb->ptr++ = controlmain;

    *pb->ptr++ = HOST1X_OPCODE_INCR(0x37, 0x4);
    *pb->ptr++ = height << 16 | width; /* srcsize */
    *pb->ptr++ = height << 16 | width; /* dstsize */
    *pb->ptr++ = srcY << 16 | srcX; /* srcps */
    *pb->ptr++ = dstY << 16 | dstX; /* dstps */

    err = drm_tegra_pushbuf_sync(pb, DRM_TEGRA_SYNCPT_COND_OP_DONE);
    if (err < 0)
        ErrorMsg("failed to insert syncpoint increment: %d\n", err);
}

static void TegraEXADoneCopy(PixmapPtr pDstPixmap)
{
    ScrnInfoPtr pScrn = xf86ScreenToScrn(pDstPixmap->drawable.pScreen);
    TegraEXAPtr tegra = TegraPTR(pScrn)->exa;
    struct drm_tegra_fence *fence;
    int err;

    err = drm_tegra_job_submit(tegra->job, &fence);
    if (err < 0) {
        ErrorMsg("failed to submit job: %d\n", err);
        goto free_job;
    }

    err = drm_tegra_fence_wait(fence);
    if (err < 0)
        ErrorMsg("failed to wait for fence: %d\n", err);

    drm_tegra_fence_free(fence);

free_job:
    drm_tegra_job_free(tegra->job);
    tegra->pushbuf = NULL;
    tegra->job = NULL;
}

static Bool TegraEXACheckComposite(int op, PicturePtr pSrcPicture,
                                   PicturePtr pMaskPicture,
                                   PicturePtr pDstPicture)
{
    ScrnInfoPtr pScrn = xf86ScreenToScrn(pDstPicture->pDrawable->pScreen);
    TegraEXAPtr tegra = TegraPTR(pScrn)->exa;

    if (!tegra->gr2d)
        return FALSE;

    /*
     * It should be possible to support all GX* raster operations given the
     * mapping in the rop3 table, but none other than GXcopy have been
     * validated.
     */
    if (op != GXcopy)
        return FALSE;

    /* TODO: implement */
    return FALSE;
}

static Bool TegraEXAPrepareComposite(int op, PicturePtr pSrcPicture,
                                     PicturePtr pMaskPicture,
                                     PicturePtr pDstPicture, PixmapPtr pSrc,
                                     PixmapPtr pMask, PixmapPtr pDst)
{
    ScrnInfoPtr pScrn = xf86ScreenToScrn(pDst->drawable.pScreen);
    TegraEXAPtr tegra = TegraPTR(pScrn)->exa;

    if (!tegra->gr2d)
        return FALSE;

    /*
     * It should be possible to support all GX* raster operations given the
     * mapping in the rop3 table, but none other than GXcopy have been
     * validated.
     */
    if (op != GXcopy)
        return FALSE;

    /* TODO: implement */
    return FALSE;
}

static void TegraEXAComposite(PixmapPtr pDst, int srcX, int srcY, int maskX,
                              int maskY, int dstX, int dstY, int width,
                              int height)
{
    /* TODO: implement */
}

static void TegraEXADoneComposite(PixmapPtr pDst)
{
    /* TODO: implement */
}

static Bool
TegraEXADownloadFromScreen(PixmapPtr pSrc, int x, int y, int w, int h,
                           char *dst, int pitch)
{
    return FALSE;
}

void TegraEXAScreenInit(ScreenPtr pScreen)
{
    ScrnInfoPtr pScrn = xf86ScreenToScrn(pScreen);
    TegraPtr tegra = TegraPTR(pScrn);
    ExaDriverPtr exa;
    TegraEXAPtr priv;
    int err;

    exa = exaDriverAlloc();
    if (!exa) {
        ErrorMsg("EXA allocation failed\n");
        return;
    }

    priv = calloc(1, sizeof(*priv));
    if (!priv) {
        ErrorMsg("EXA allocation failed\n");
        goto free_exa;
    }

    err = drm_tegra_channel_open(&priv->gr2d, tegra->drm, DRM_TEGRA_GR2D);
    if (err < 0) {
        ErrorMsg("failed to open 2D channel: %d\n", err);
        goto free_priv;
    }

    err = drm_tegra_bo_new(&priv->bo, tegra->drm, 0, 4096);
    if (err < 0) {
        ErrorMsg("failed to create buffer: %d\n", err);
        goto close_gr2d;
    }

    exa->exa_major = EXA_VERSION_MAJOR;
    exa->exa_minor = EXA_VERSION_MINOR;
    exa->pixmapOffsetAlign = 256;
    exa->pixmapPitchAlign = 64;
    exa->flags = EXA_SUPPORTS_PREPARE_AUX |
                 EXA_OFFSCREEN_PIXMAPS |
                 EXA_HANDLES_PIXMAPS;

    exa->maxX = 8192;
    exa->maxY = 8192;

    exa->MarkSync = TegraEXAMarkSync;
    exa->WaitMarker = TegraEXAWaitMarker;

    exa->PrepareAccess = TegraEXAPrepareAccess;
    exa->FinishAccess = TegraEXAFinishAccess;
    exa->PixmapIsOffscreen = TegraEXAPixmapIsOffscreen;

    exa->CreatePixmap2 = TegraEXACreatePixmap2;
    exa->DestroyPixmap = TegraEXADestroyPixmap;
    exa->ModifyPixmapHeader = TegraEXAModifyPixmapHeader;

    exa->PrepareSolid = TegraEXAPrepareSolid;
    exa->Solid = TegraEXASolid;
    exa->DoneSolid = TegraEXADoneSolid;

    exa->PrepareCopy = TegraEXAPrepareCopy;
    exa->Copy = TegraEXACopy;
    exa->DoneCopy = TegraEXADoneCopy;

    exa->CheckComposite = TegraEXACheckComposite;
    exa->PrepareComposite = TegraEXAPrepareComposite;
    exa->Composite = TegraEXAComposite;
    exa->DoneComposite = TegraEXADoneComposite;

    exa->DownloadFromScreen = TegraEXADownloadFromScreen;

    if (!exaDriverInit(pScreen, exa)) {
        ErrorMsg("EXA initialization failed\n");
        goto put_bo;
    }

    priv->driver = exa;
    tegra->exa = priv;

    return;

put_bo:
    drm_tegra_bo_unref(priv->bo);
close_gr2d:
    drm_tegra_channel_close(priv->gr2d);
free_priv:
    free(priv);
free_exa:
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

        drm_tegra_channel_close(priv->gr2d);
        drm_tegra_bo_unref(priv->bo);
        free(priv);
    }
}

/* vim: set et sts=4 sw=4 ts=4: */
