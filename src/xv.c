/*
 * Copyright 2016-2017 Dmitry Osipenko <digetx@gmail.com>
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

#include "driver.h"

#define ErrorMsg(fmt, args...) \
    xf86DrvMsg(scrn->scrnIndex, X_ERROR, "%s:%d/%s(): " fmt, __FILE__, \
               __LINE__, __func__, ##args)

#define MAKE_ATOM(a) MakeAtom(a, sizeof(a) - 1, TRUE)

#define DEFAULT_COLOR_KEY 0xFF4AF6

static Atom xvColorKey;

typedef struct TegraOverlay {
    drm_overlay_fb *fb;
    uint32_t plane_id;
    Bool visible;
    Bool ready;
    int crtc_id;

    int src_x;
    int src_y;
    int src_w;
    int src_h;

    int dst_x;
    int dst_y;
    int dst_w;
    int dst_h;

    uint32_t color_key;
    Bool ckey_enb;
} TegraOverlay, *TegraOverlayPtr;

typedef struct TegraVideo {
    TegraOverlay overlay[2];
    drm_overlay_fb *old_fb;
    drm_overlay_fb *fb;

    uint8_t passthrough_data[PASSTHROUGH_DATA_SIZE];
    Bool passthrough;

    uint32_t color_key;
    Bool ckey_probed;
} TegraVideo, *TegraVideoPtr;

typedef struct TegraXvAdaptor {
    XF86VideoAdaptorRec xv;
    DevUnion dev_union;
    TegraVideo private;
} TegraXvAdaptor, *TegraXvAdaptorPtr;

static XF86ImageRec XvImages[] = {
    XVIMAGE_YUY2,
    XVIMAGE_YV12,
    XVIMAGE_I420,
    XVIMAGE_UYVY,
    XVMC_YV12,
    XVMC_RGB565,
    XVMC_XRGB8888,
    XVMC_XBGR8888,
};

static XF86VideoFormatRec XvFormats[] = {
    {
        .depth = 24,
        .class = TrueColor,
    },
};

static XF86AttributeRec XvAttributes[] = {
    {
        .flags      = XvSettable | XvGettable,
        .min_value  = 0,
        .max_value  = 0xFFFFFF,
        .name       = (char *)"XV_COLORKEY",
    },
};

static XF86VideoEncodingRec XvEncoding[] = {
    {
        .id               = 0,
        .name             = "XV_IMAGE",
        .width            = TEGRA_VIDEO_OVERLAY_MAX_WIDTH,
        .height           = TEGRA_VIDEO_OVERLAY_MAX_HEIGHT,
        .rate.numerator   = 1,
        .rate.denominator = 1,
    },
};

static Bool xv_fourcc_valid(int format_id)
{
    switch (format_id) {
    case FOURCC_YUY2:
    case FOURCC_YV12:
    case FOURCC_I420:
    case FOURCC_UYVY:
        return TRUE;

    case FOURCC_PASSTHROUGH_YV12:
    case FOURCC_PASSTHROUGH_RGB565:
    case FOURCC_PASSTHROUGH_XRGB8888:
    case FOURCC_PASSTHROUGH_XBGR8888:
        return TRUE;

    default:
        break;
    }

    return FALSE;
}

static uint32_t xv_fourcc_to_drm(int format_id)
{
    switch (format_id) {
    case FOURCC_YUY2:
        return DRM_FORMAT_YUYV;

    case FOURCC_YV12:
        return DRM_FORMAT_YUV420;

    case FOURCC_I420:
        return DRM_FORMAT_YUV420;

    case FOURCC_UYVY:
        return DRM_FORMAT_UYVY;

    case FOURCC_PASSTHROUGH_YV12:
        return DRM_FORMAT_YUV420;

    case FOURCC_PASSTHROUGH_RGB565:
        return DRM_FORMAT_RGB565;

    case FOURCC_PASSTHROUGH_XRGB8888:
        return DRM_FORMAT_XRGB8888;

    case FOURCC_PASSTHROUGH_XBGR8888:
        return DRM_FORMAT_XBGR8888;

    default:
        FatalError("%s: Shouldn't be here; format_id %d\n",
                   __FUNCTION__, format_id);
        break;
    }

    return 0xFFFFFFFF;
}

static Bool TegraVideoOverlayInitialize(TegraVideoPtr priv, ScrnInfoPtr scrn,
                                        int overlay_id)
{
    TegraOverlayPtr overlay = &priv->overlay[overlay_id];
    TegraPtr tegra          = TegraPTR(scrn);
    Bool ret                = FALSE;
    drmModeResPtr res;
    uint32_t plane_id;

    if (overlay->ready)
        return TRUE;

    res = drmModeGetResources(tegra->fd);
    if (!res)
        goto end;

    if (overlay_id > res->count_crtcs)
        goto end;

    if (drm_get_overlay_plane(tegra->fd, overlay_id,
                              DRM_FORMAT_YUV420, &plane_id) < 0)
        goto end;

    overlay->crtc_id   = res->crtcs[overlay_id];
    overlay->plane_id  = plane_id;
    overlay->ready     = TRUE;

    ret = TRUE;
end:
    free(res);

    return ret;
}

static Bool TegraVideoOverlaySetColorKey(TegraVideoPtr priv,
                                         ScrnInfoPtr scrn,
                                         int id, uint32_t color_key,
                                         Bool enable)
{
    TegraPtr tegra          = TegraPTR(scrn);
    TegraOverlayPtr overlay = &priv->overlay[id];
    drmModeObjectPropertiesPtr properties;
    drmModePropertyPtr property;
    drmModeAtomicReqPtr req;
    uint32_t plane_id;
    uint32_t ckey_abgr = 0;
    unsigned int ckey_not_changed = 0;
    unsigned int i;
    int ret = 0;

    if (overlay->ckey_enb != enable)
        ckey_not_changed += 1;

    if (overlay->color_key != color_key)
        ckey_not_changed += 2;

    if (ckey_not_changed == 0)
        return TRUE;

    req = drmModeAtomicAlloc();
    if (!req)
        return FALSE;

    if (overlay->color_key == color_key)
        goto ckey_enable;

    /* convert ARGB to ABGR */
    ckey_abgr |= (color_key & 0xff00ff00);
    ckey_abgr |= (color_key & 0x000000ff) << 16;
    ckey_abgr |= (color_key & 0x00ff0000) >> 16;

    properties = drmModeObjectGetProperties(tegra->fd, overlay->crtc_id,
                                            DRM_MODE_OBJECT_CRTC);
    if (!properties)
        ret = -1;

    for (i = 0; ret >= 0 && i < properties->count_props; i++) {
        property = drmModeGetProperty(tegra->fd,
                                      properties->props[i]);
        if (!property)
            continue;

        if (!strcmp(property->name, "color key 0 lower margin") ||
            !strcmp(property->name, "color key 0 upper margin"))
        {
            ret = drmModeAtomicAddProperty(req, overlay->crtc_id,
                                           property->prop_id,
                                           ckey_abgr);
            ckey_not_changed--;
        }

        free(property);
    }

    free(properties);

    if (ret < 0)
        goto atomic_free;

ckey_enable:
    if (overlay->ckey_enb == enable)
        goto atomic_commit;

    ret = drm_get_primary_plane(tegra->fd, id, &plane_id);
    if (ret)
        goto atomic_free;

    properties = drmModeObjectGetProperties(tegra->fd, plane_id,
                                            DRM_MODE_OBJECT_PLANE);
    if (!properties)
        ret = -1;

    for (i = 0; ret >= 0 && i < properties->count_props; i++) {
        property = drmModeGetProperty(tegra->fd,
                                      properties->props[i]);
        if (!property)
            continue;

        if (!strcmp(property->name, "color key 0 enabled")) {
            ret = drmModeAtomicAddProperty(req, plane_id,
                                           property->prop_id,
                                           enable);
            i = properties->count_props;
            ckey_not_changed--;
        }

        free(property);
    }

    free(properties);

atomic_commit:
    if (ckey_not_changed)
        ret = -1;

    if (ret >= 0)
        ret = drmModeAtomicCommit(tegra->fd, req, 0, NULL);

atomic_free:
    drmModeAtomicFree(req);

    if (ret < 0)
        return FALSE;

    overlay->color_key = color_key;
    overlay->ckey_enb = enable;

    return TRUE;
}

static void TegraVideoVSync(TegraVideoPtr priv, ScrnInfoPtr scrn, int id)
{
    TegraOverlayPtr overlay = &priv->overlay[id];
    TegraPtr tegra          = TegraPTR(scrn);
    int ret;

    drmVBlank vblank = {
        .request = {
            .type = DRM_VBLANK_RELATIVE,
            .sequence = 1,
        },
    };

    if (!overlay->ready)
        return;

    vblank.request.type |= id << DRM_VBLANK_HIGH_CRTC_SHIFT,

    ret = drmWaitVBlank(tegra->fd, &vblank);
    if (ret < 0)
        ErrorMsg("DRM VBlank failed: %s\n", strerror(-ret));
}

static void TegraVideoOverlayShow(TegraVideoPtr priv,
                                  ScrnInfoPtr scrn,
                                  int overlay_id,
                                  int src_x, int src_y,
                                  int src_w, int src_h,
                                  int dst_x, int dst_y,
                                  int dst_w, int dst_h)
{
    TegraOverlayPtr overlay = &priv->overlay[overlay_id];
    TegraPtr tegra          = TegraPTR(scrn);
    drm_overlay_fb *fb      = priv->fb;
    int ret;

    if (overlay->src_x == src_x &&
        overlay->src_y == src_y &&
        overlay->src_w == src_w &&
        overlay->src_h == src_h &&
        overlay->dst_x == dst_x &&
        overlay->dst_y == dst_y &&
        overlay->dst_w == dst_w &&
        overlay->dst_h == dst_h &&
        overlay->fb    == fb)
            return;

    overlay->src_x = src_x;
    overlay->src_y = src_y;
    overlay->src_w = src_w;
    overlay->src_h = src_h;
    overlay->dst_x = dst_x;
    overlay->dst_y = dst_y;
    overlay->dst_w = dst_w;
    overlay->dst_h = dst_h;
    overlay->fb    = fb;

    ret = drmModeSetPlane(tegra->fd,
                          overlay->plane_id,
                          overlay->crtc_id,
                          fb->fb_id, 0,
                          dst_x, dst_y,
                          dst_w, dst_h,
                          src_x, src_y,
                          src_w << 16, src_h << 16);
    if (ret < 0)
        ErrorMsg("DRM set plane failed: %s\n", strerror(-ret));
    else
        overlay->visible = TRUE;
}

static void TegraVideoOverlayClose(TegraVideoPtr priv, ScrnInfoPtr scrn,
                                   int id, Bool cleanup)
{
    TegraOverlayPtr overlay = &priv->overlay[id];
    TegraPtr tegra          = TegraPTR(scrn);
    int ret;

    if (!overlay->ready)
        return;

    if (cleanup)
        TegraVideoOverlaySetColorKey(priv, scrn, id, DEFAULT_COLOR_KEY,
                                     FALSE);

    ret = drmModeSetPlane(tegra->fd, overlay->plane_id, id,
                          0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
    if (ret < 0)
        ErrorMsg("Failed to close overlay\n");

    overlay->visible = FALSE;
    overlay->fb      = NULL;
}

static void TegraVideoDestroyFramebuffer(TegraVideoPtr priv, ScrnInfoPtr scrn,
                                         drm_overlay_fb **fb)
{
    TegraPtr tegra = TegraPTR(scrn);

    drm_free_overlay_fb(tegra->fd, *fb);
    *fb = NULL;
}

static Bool TegraCloseBo(ScrnInfoPtr scrn, uint32_t handle)
{
    TegraPtr tegra = TegraPTR(scrn);
    struct drm_gem_close args;
    int ret;

    memset(&args, 0, sizeof(args));
    args.handle = handle;

    ret = drmIoctl(tegra->fd, DRM_IOCTL_GEM_CLOSE, &args);
    if (ret < 0) {
            ErrorMsg("Failed to close GEM %s\n", strerror(-ret));
            return ret;
    }

    return TRUE;
}

static Bool TegraImportBo(ScrnInfoPtr scrn, uint32_t flink, uint32_t *handle)
{
    TegraPtr tegra = TegraPTR(scrn);
    struct drm_gem_open args;
    int ret;

    memset(&args, 0, sizeof(args));
    args.name = flink;

    ret = drmIoctl(tegra->fd, DRM_IOCTL_GEM_OPEN, &args);
    if (ret < 0) {
            ErrorMsg("Failed to open GEM by name %s\n", strerror(-ret));
            return ret;
    }

    *handle = args.handle;

    return TRUE;
}

static Bool TegraVideoOverlayCreateFB(TegraVideoPtr priv, ScrnInfoPtr scrn,
                                      uint32_t drm_format,
                                      uint32_t width, uint32_t height,
                                      Bool passthrough,
                                      uint8_t *passthrough_data)
{
    TegraPtr tegra     = TegraPTR(scrn);
    uint32_t *flinks   = (uint32_t*) (passthrough_data +  0);
    uint32_t *pitches  = (uint32_t*) (passthrough_data + 12);
    uint32_t *offsets  = (uint32_t*) (passthrough_data + 24);
    uint32_t bo_handles[3];
    drm_overlay_fb *fb;
    int i = 0;

    if (priv->fb &&
        priv->fb->format  == drm_format &&
        priv->fb->width   == width &&
        priv->fb->height  == height &&
        priv->passthrough == passthrough)
    {
        if (!passthrough)
            return TRUE;

        if (memcmp(passthrough_data, priv->passthrough_data,
                   PASSTHROUGH_DATA_SIZE) == 0)
            return TRUE;
    }

    if (passthrough) {
        switch (drm_format) {
        case DRM_FORMAT_YUV420:
            i = 3;
            break;
        default:
            i = 1;
        }

        for (; i > 0; i--) {
            if (!TegraImportBo(scrn, flinks[i - 1], &bo_handles[i - 1]))
                goto fail;
        }

        fb = drm_create_fb_from_handle(tegra->fd, drm_format, width, height,
                                       bo_handles, pitches, offsets);
    } else {
        fb = drm_create_fb(tegra->fd, drm_format, width, height);
    }

    if (fb == NULL) {
        goto fail;
    }

    if (passthrough) {
        memcpy(priv->passthrough_data, passthrough_data,
               PASSTHROUGH_DATA_SIZE);
    }

    priv->old_fb      = priv->fb;
    priv->fb          = fb;
    priv->passthrough = passthrough;

    return TRUE;

fail:
    ErrorMsg("Failed to create framebuffer\n");

    for (; i < TEGRA_ARRAY_SIZE(bo_handles) && i > 0; i--) {
        TegraCloseBo(scrn, bo_handles[i - 1]);
    }

    return FALSE;
}

static void TegraVideoOverlayBestSize(ScrnInfoPtr scrn, Bool motion,
                                      short vid_w, short vid_h,
                                      short dst_w, short dst_h,
                                      unsigned int *actual_w,
                                      unsigned int *actual_h,
                                      void *data)
{
    *actual_w = dst_w;
    *actual_h = dst_h;
}

static int TegraVideoOverlaySetAttribute(ScrnInfoPtr scrn, Atom attribute,
                                         INT32 value, void *data)
{
    TegraVideoPtr priv = data;
    int id;

    if (attribute != xvColorKey)
        return BadMatch;

    for (id = 0; id < TEGRA_ARRAY_SIZE(priv->overlay); id++) {
        if (!TegraVideoOverlayInitialize(priv, scrn, id))
            return BadImplementation;

        if (!TegraVideoOverlaySetColorKey(priv, scrn, id, value, TRUE))
            return BadImplementation;
    }

    priv->color_key = value;
    priv->ckey_probed = TRUE;

    return Success;
}

static int TegraVideoOverlayGetAttribute(ScrnInfoPtr scrn, Atom attribute,
                                         INT32 *value, void *data)
{
    TegraVideoPtr priv = data;
    int id;

    if (attribute != xvColorKey)
        return BadMatch;

    *value = priv->color_key;

    if (priv->ckey_probed)
        return Success;

    for (id = 0; id < TEGRA_ARRAY_SIZE(priv->overlay); id++) {
        if (!TegraVideoOverlayInitialize(priv, scrn, id))
            return BadImplementation;

        if (!TegraVideoOverlaySetColorKey(priv, scrn, id, priv->color_key,
                                          TRUE))
            return BadImplementation;
    }

    priv->ckey_probed = TRUE;

    return Success;
}

static void TegraVideoOverlayStop(ScrnInfoPtr scrn, void *data, Bool cleanup)
{
    TegraVideoPtr priv = data;
    int id;

    for (id = 0; id < TEGRA_ARRAY_SIZE(priv->overlay); id++)
        TegraVideoOverlayClose(priv, scrn, id, cleanup);

    if (cleanup)
        TegraVideoDestroyFramebuffer(priv, scrn, &priv->fb);

    if (cleanup) {
        priv->color_key = DEFAULT_COLOR_KEY;
        priv->ckey_probed = FALSE;
    }
}

static void TegraVideoOverlayPutImageOnOverlay(TegraVideoPtr priv,
                                               ScrnInfoPtr scrn,
                                               int overlay_id,
                                               short src_x, short src_y,
                                               short dst_x, short dst_y,
                                               short src_w, short src_h,
                                               short dst_w, short dst_h,
                                               short width, short height,
                                               DrawablePtr draw)
{
    TegraOverlayPtr overlay       = &priv->overlay[overlay_id];
    xf86CrtcConfigPtr xf86_config = XF86_CRTC_CONFIG_PTR(scrn);
    xf86CrtcPtr crtc              = xf86_config->crtc[overlay_id];

    if (!overlay->ready)
        return;

    if (!overlay->visible)
        return;

    dst_x -= crtc->x;
    dst_y -= crtc->y;

    TegraVideoOverlayShow(priv, scrn, overlay_id,
                          src_x, src_y, src_w, src_h,
                          dst_x, dst_y, dst_w, dst_h);
}

static int TegraVideoOverlayCheckVisibility(TegraVideoPtr priv,
                                            ScrnInfoPtr scrn,
                                            DrawablePtr draw,
                                            int id)
{
    TegraOverlayPtr overlay = &priv->overlay[id];
    int coverage;

    if (overlay->ready)
        coverage = tegra_crtc_coverage(draw, id);
    else
        coverage = 0;

    overlay->visible = !!coverage;

    if (!overlay->visible)
        TegraVideoOverlayClose(priv, scrn, id, FALSE);

    return coverage;
}

static int TegraVideoOverlayPutImage(ScrnInfoPtr scrn,
                                     short src_x, short src_y,
                                     short dst_x, short dst_y,
                                     short src_w, short src_h,
                                     short dst_w, short dst_h,
                                     int format,
                                     unsigned char *buf,
                                     short width,
                                     short height,
                                     Bool vblankSync,
                                     RegionPtr clipBoxes,
                                     void *data, DrawablePtr draw)
{
    TegraVideoPtr priv = data;
    Bool visible       = FALSE;
    Bool passthrough   = FALSE;
    int best_coverage  = 0;
    int best_id        = 0;
    int coverage;
    int id;

    if (!xv_fourcc_valid(format))
        return BadImplementation;

    switch (format) {
    case FOURCC_PASSTHROUGH_YV12:
    case FOURCC_PASSTHROUGH_RGB565:
    case FOURCC_PASSTHROUGH_XRGB8888:
    case FOURCC_PASSTHROUGH_XBGR8888:
        passthrough = TRUE;
    }

    for (id = 0; id < TEGRA_ARRAY_SIZE(priv->overlay); id++)
        if (!TegraVideoOverlayInitialize(priv, scrn, id))
            return BadImplementation;

    if (!TegraVideoOverlayCreateFB(priv, scrn, xv_fourcc_to_drm(format),
                                   width, height, passthrough, buf))
        return BadImplementation;

    for (id = 0; id < TEGRA_ARRAY_SIZE(priv->overlay); id++) {
        coverage = TegraVideoOverlayCheckVisibility(priv, scrn, draw, id);

        if (coverage > best_coverage) {
            best_coverage = coverage;
            best_id = id;
            visible = TRUE;
        }
    }

    if (!visible)
        return Success;

    if (vblankSync)
        TegraVideoVSync(priv, scrn, best_id);

    if (!passthrough)
        drm_copy_data_to_fb(priv->fb, buf, format == FOURCC_I420);

    for (id = 0; id < TEGRA_ARRAY_SIZE(priv->overlay); id++)
        TegraVideoOverlayPutImageOnOverlay(priv, scrn, id,
                                           src_x, src_y,
                                           dst_x, dst_y,
                                           src_w, src_h,
                                           dst_w, dst_h,
                                           width, height,
                                           draw);
    if (priv->old_fb)
        TegraVideoDestroyFramebuffer(priv, scrn, &priv->old_fb);

    return Success;
}

static int TegraVideoOverlayQuery(ScrnInfoPtr scrn,
                                  int id,
                                  unsigned short *w, unsigned short *h,
                                  int *pitches, int *offsets)
{
    int size;

    switch (id) {
    case FOURCC_YUY2:
        if (pitches)
            pitches[0] = (*w) * 2;

        if (offsets)
            offsets[0] = 0;

        size = (*w) * (*h) * 2;
        break;
    case FOURCC_YV12:
    case FOURCC_I420:
        if (pitches) {
            pitches[0] = (*w);
            pitches[1] = (*w) / 2;
            pitches[2] = (*w) / 2;
        }

        if (offsets) {
            offsets[0] = 0;
            offsets[1] = (*w) * (*h);
            offsets[2] = (*w) * (*h) * 5 / 4;
        }

        size = (*w) * (*h) * 3 / 2;
        break;
    case FOURCC_UYVY:
        if (pitches)
            pitches[0] = (*w) * 2;

        if (offsets)
            offsets[0] = 0;

        size = (*w) * (*h) * 2;
        break;
    case FOURCC_PASSTHROUGH_YV12:
    case FOURCC_PASSTHROUGH_RGB565:
    case FOURCC_PASSTHROUGH_XRGB8888:
    case FOURCC_PASSTHROUGH_XBGR8888:
        size = PASSTHROUGH_DATA_SIZE;
        break;
    default:
        return BadValue;
    }

    return size;
}

void TegraXvScreenInit(ScreenPtr pScreen)
{
    ScrnInfoPtr scrn = xf86ScreenToScrn(pScreen);
    XF86VideoAdaptorPtr xvAdaptor;
    TegraXvAdaptorPtr adaptor;

    if (noXvExtension)
        return;

    adaptor = xnfcalloc(1, sizeof(*adaptor));

    adaptor->xv.type                 = XvWindowMask | XvInputMask | XvImageMask;
    adaptor->xv.name                 = (char *)"Opentegra Video Overlay";
    adaptor->xv.nEncodings           = 1;
    adaptor->xv.pEncodings           = XvEncoding;
    adaptor->xv.pFormats             = XvFormats;
    adaptor->xv.nFormats             = TEGRA_ARRAY_SIZE(XvFormats);
    adaptor->xv.pAttributes          = XvAttributes;
    adaptor->xv.nAttributes          = TEGRA_ARRAY_SIZE(XvAttributes);
    adaptor->xv.pImages              = XvImages;
    adaptor->xv.nImages              = TEGRA_ARRAY_SIZE(XvImages);
    adaptor->xv.StopVideo            = TegraVideoOverlayStop;
    adaptor->xv.SetPortAttribute     = TegraVideoOverlaySetAttribute;
    adaptor->xv.GetPortAttribute     = TegraVideoOverlayGetAttribute;
    adaptor->xv.QueryBestSize        = TegraVideoOverlayBestSize;
    adaptor->xv.PutImage             = TegraVideoOverlayPutImage;
    adaptor->xv.QueryImageAttributes = TegraVideoOverlayQuery;
    adaptor->xv.nPorts               = 1;
    adaptor->xv.pPortPrivates        = &adaptor->dev_union;
    adaptor->xv.pPortPrivates[0].ptr = &adaptor->private;
    adaptor->xv.flags                = VIDEO_OVERLAID_IMAGES;
    adaptor->private.color_key       = DEFAULT_COLOR_KEY;

    xvColorKey = MAKE_ATOM("XV_COLORKEY");
    xvAdaptor = &adaptor->xv;

    if (!xf86XVScreenInit(pScreen, &xvAdaptor, 1)) {
        free(adaptor);
        return;
    }

    xf86DrvMsg(scrn->scrnIndex, X_INFO, "XV adaptor initialized\n");
}
