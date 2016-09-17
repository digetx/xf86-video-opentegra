/*
 * Copyright 2016 Dmitry Osipenko <digetx@gmail.com>
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
#include "config.h"
#endif

#include <fcntl.h>

#include <X11/extensions/Xv.h>
#include <drm_fourcc.h>

#include "xorg-server.h"
#include "xf86drm.h"
#include "xf86drmMode.h"
#include "xf86Crtc.h"
#include "xf86xv.h"
#include "fourcc.h"
#include "windowstr.h"

#include "compat-api.h"
#include "driver.h"
#include "video_overlay.h"

#define MAKE_ATOM(a) MakeAtom(a, sizeof(a) - 1, TRUE)

struct drm_tegra_set_color_key {
	/* input */
	__u32 crtc_mask;	/* Display controllers to use that key */
	__u32 key_id;		/* Specify what color key to set, 0 or 1 */
	__u32 upper;		/* Color key itself in XRGB_8888 format */
	__u32 lower;		/* in range lower..upper */
};

#define DRM_TEGRA_PLANE_BLEND_CONFIG_NOKEY	0
#define DRM_TEGRA_PLANE_BLEND_CONFIG_1WIN	1
#define DRM_TEGRA_PLANE_BLEND_CONFIG_2WIN_X	2
#define DRM_TEGRA_PLANE_BLEND_CONFIG_2WIN_Y	3
#define DRM_TEGRA_PLANE_BLEND_CONFIG_3WIN_XY	4

#define DRM_TEGRA_PLANE_BLEND_CONTROL_FIX_WEIGHT	0
#define DRM_TEGRA_PLANE_BLEND_CONTROL_ALPHA_WEIGHT	1
#define DRM_TEGRA_PLANE_BLEND_CONTROL_DEPENDENT_WEIGHT	2

struct drm_tegra_plane_set_blending {
	/* input */
	__u32 plane_id;
	__u32 blend_config;	/* Specify blending configuration to set */
	__u32 blend_control;
	__u32 blend_weight0;
	__u32 blend_weight1;
	__u32 use_color_key0;	/* Ignored by the NOKEY blending config */
	__u32 use_color_key1;	/* Ignored by the NOKEY blending config */
	__u32 pad;
};

#define DRM_TEGRA_SET_COLOR_KEY		0x0e
#define DRM_TEGRA_PLANE_SET_BLENDING	0x0f
#define DRM_IOCTL_TEGRA_SET_COLOR_KEY DRM_IOW(DRM_COMMAND_BASE + DRM_TEGRA_SET_COLOR_KEY, struct drm_tegra_set_color_key)
#define DRM_IOCTL_TEGRA_PLANE_SET_BLENDING DRM_IOW(DRM_COMMAND_BASE + DRM_TEGRA_PLANE_SET_BLENDING, struct drm_tegra_plane_set_blending)

static Atom xvColorKey;

typedef struct TegraOverlay {
    Bool visible;
    uint32_t plane_id;
    drm_overlay_fb *fb;

    uint32_t color_key;

    int src_x;
    int src_y;
    int src_w;
    int src_h;

    int dst_x;
    int dst_y;
    int dst_w;
    int dst_h;
} TegraOverlay, *TegraOverlayPtr;

typedef struct TegraVideo {
    drmmode_crtc_private_ptr drmmode_crtc_priv;
    uint32_t primary_plane_id;
    TegraOverlayPtr overlay;
} TegraVideo, *TegraVideoPtr;

static XvImageRec XvImages[] = {
    XVIMAGE_YUY2,
    XVIMAGE_YV12,
    XVIMAGE_I420,
    XVIMAGE_UYVY,
//     XVMC_YV12,
//     XVMC_RGB565,
//     XVMC_RGB888,
//     XVMC_BGR888,
};

static XvFormatRec XvFormats[] = {
    {
        .depth  = 24,
        .visual = 0,
    },
};

static XvAttributeRec XvAttributes[] = {
    {
        .flags      = XvSettable | XvGettable,
        .min_value  = 0,
        .max_value  = 0xFFFFFF,
        .name       = (char *)"XV_COLORKEY",
    },
};

static Bool xv_passthrough(int format_id)
{
    switch (format_id) {
    case FOURCC_PASSTHROUGH_YV12:
    case FOURCC_PASSTHROUGH_RGB565:
    case FOURCC_PASSTHROUGH_RGB888:
    case FOURCC_PASSTHROUGH_BGR888:
        return TRUE;
    default:
        break;
    }

    return FALSE;
}

static Bool xv_fourcc_valid(int format_id)
{
    switch (format_id) {
    case FOURCC_YUY2:
    case FOURCC_YV12:
    case FOURCC_I420:
    case FOURCC_UYVY:
        return TRUE;
    default:
        break;
    }

    return FALSE;
}

static const char * format_name(int format_id)
{
    switch (format_id) {
    case FOURCC_PASSTHROUGH_YV12:
        return "FOURCC_PASSTHROUGH_YV12";
    case FOURCC_PASSTHROUGH_RGB565:
        return "FOURCC_PASSTHROUGH_RGB565";
    case FOURCC_PASSTHROUGH_RGB888:
        return "FOURCC_PASSTHROUGH_RGB888";
    case FOURCC_PASSTHROUGH_BGR888:
        return "FOURCC_PASSTHROUGH_BGR888";
    case FOURCC_YUY2:
        return "FOURCC_YUY2";
    case FOURCC_YV12:
        return "FOURCC_YV12";
    case FOURCC_I420:
        return "FOURCC_I420";
    case FOURCC_UYVY:
        return "FOURCC_UYVY";
    default:
        break;
    }

    return "Bad format!";
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
    default:
        FatalError("%s: Shouldn't be here; format_id %d\n",
                   __FUNCTION__, format_id);
        break;
    }

    return 0xFFFFFFFF;
}

static int TegraVideoSetColorKey(XvPortPtr port)
{
    TegraVideoPtr priv      = port->devPriv.ptr;
    TegraOverlayPtr overlay = priv->overlay;
    ScrnInfoPtr scrn        = xf86ScreenToScrn(port->pAdaptor->pScreen);
    uint32_t crtc_pipe      = priv->drmmode_crtc_priv->crtc_pipe;
    int drm_fd              = priv->drmmode_crtc_priv->drmmode->fd;
    int ret;

    struct drm_tegra_set_color_key set_color_key = {
        .crtc_mask = 1 << crtc_pipe,
        .key_id = 0,
        .upper = overlay->color_key,
        .lower = overlay->color_key,
    };

    xf86DrvMsg(-1, X_INFO, "%s: key 0x%08X\n", __func__, overlay->color_key);

    ret = drmIoctl(drm_fd, DRM_IOCTL_TEGRA_SET_COLOR_KEY, &set_color_key);
    if (ret < 0) {
        xf86DrvMsg(scrn->scrnIndex, X_ERROR,
                   "%s: Failed to set color key failed pipe %d: %s\n",
                   __FUNCTION__, crtc_pipe, strerror(errno));
        return BadImplementation;
    }

    return Success;
}

static int TegraVideoOverlayEnableColorKey(XvPortPtr port)
{
    TegraVideoPtr priv = port->devPriv.ptr;
    ScrnInfoPtr scrn   = xf86ScreenToScrn(port->pAdaptor->pScreen);
    int drm_fd         = priv->drmmode_crtc_priv->drmmode->fd;
    int ret;

    struct drm_tegra_plane_set_blending blending_2win_b = {
        .plane_id       = priv->primary_plane_id,
        .blend_config   = DRM_TEGRA_PLANE_BLEND_CONFIG_2WIN_X,
        .blend_control  = DRM_TEGRA_PLANE_BLEND_CONTROL_FIX_WEIGHT,
        .blend_weight0  = 0x00,
        .blend_weight1  = 0x00,
        .use_color_key0 = 1,
        .use_color_key1 = 0,
    };

    ret = drmIoctl(drm_fd, DRM_IOCTL_TEGRA_PLANE_SET_BLENDING, &blending_2win_b);
    if (ret < 0) {
        xf86DrvMsg(scrn->scrnIndex, X_ERROR,
                   "%s: Failed to set primary plane 2win_b blending: %s\n",
                   __FUNCTION__, strerror(errno));
        return BadImplementation;
    }

    return Success;
}

static int TegraVideoOverlayDisableColorKey(XvPortPtr port)
{
    TegraVideoPtr priv = port->devPriv.ptr;
    ScrnInfoPtr scrn   = xf86ScreenToScrn(port->pAdaptor->pScreen);
    int drm_fd         = priv->drmmode_crtc_priv->drmmode->fd;
    int ret;

    struct drm_tegra_plane_set_blending blending_2win_b = {
        .plane_id       = priv->primary_plane_id,
        .blend_config   = DRM_TEGRA_PLANE_BLEND_CONFIG_2WIN_X,
        .blend_control  = DRM_TEGRA_PLANE_BLEND_CONTROL_FIX_WEIGHT,
        .blend_weight0  = 0x00,
        .blend_weight1  = 0x00,
        .use_color_key0 = 0,
        .use_color_key1 = 0,
    };

    ret = drmIoctl(drm_fd, DRM_IOCTL_TEGRA_PLANE_SET_BLENDING, &blending_2win_b);
    if (ret < 0) {
        xf86DrvMsg(scrn->scrnIndex, X_ERROR,
                   "%s: Failed to set primary plane 2win_b blending: %s\n",
                   __FUNCTION__, strerror(errno));
        return BadImplementation;
    }

    return Success;
}

static Bool TegraVideoOverlayVisible(int display_width, int display_height,
                                     int x0, int y0, int w, int h)
{
    if (x0 >= display_width)
        return FALSE;

    if (y0 >= display_height)
        return FALSE;

    if (x0 + w <= 0)
        return FALSE;

    if (y0 + h <= 0)
        return FALSE;

    return TRUE;
}

static void TegraVideoVSync(XvPortPtr port)
{
    TegraVideoPtr priv = port->devPriv.ptr;
    ScrnInfoPtr scrn   = xf86ScreenToScrn(port->pAdaptor->pScreen);
    uint32_t crtc_pipe = priv->drmmode_crtc_priv->crtc_pipe;
    int drm_fd         = priv->drmmode_crtc_priv->drmmode->fd;
    int ret;

    drmVBlank vblank = {
        .request = {
            .type = DRM_VBLANK_RELATIVE,
            .sequence = 1,
        },
    };

    vblank.request.type |= crtc_pipe << DRM_VBLANK_HIGH_CRTC_SHIFT,

    ret = drmWaitVBlank(drm_fd, &vblank);
    if (ret < 0)
        xf86DrvMsg(scrn->scrnIndex, X_ERROR,
                   "DRM VBlank failed: %s\n", strerror(-ret));
}

static void TegraVideoOverlayShow(XvPortPtr port,
                                  int src_x, int src_y,
                                  int src_w, int src_h,
                                  int dst_x, int dst_y,
                                  int dst_w, int dst_h)
{
    TegraVideoPtr priv      = port->devPriv.ptr;
    TegraOverlayPtr overlay = priv->overlay;
    ScrnInfoPtr scrn        = xf86ScreenToScrn(port->pAdaptor->pScreen);
    drm_overlay_fb *fb      = overlay->fb;
    uint32_t crtc_id        = priv->drmmode_crtc_priv->mode_crtc->crtc_id;
    int drm_fd              = priv->drmmode_crtc_priv->drmmode->fd;
    int ret;

    if (!overlay->visible) {
        TegraVideoSetColorKey(port);
        TegraVideoOverlayEnableColorKey(port);
        goto update_plane;
    }

    if (overlay->src_x != src_x)
        goto update_plane;

    if (overlay->src_y != src_y)
        goto update_plane;

    if (overlay->src_w != src_w)
        goto update_plane;

    if (overlay->src_h != src_h)
        goto update_plane;

    if (overlay->dst_x != dst_x)
        goto update_plane;

    if (overlay->dst_y != dst_y)
        goto update_plane;

    if (overlay->dst_w != dst_w)
        goto update_plane;

    if (overlay->dst_h != dst_h)
        goto update_plane;

    return;

update_plane:
    overlay->src_x = src_x;
    overlay->src_y = src_y;
    overlay->src_w = src_w;
    overlay->src_h = src_h;
    overlay->dst_x = dst_x;
    overlay->dst_y = dst_y;
    overlay->dst_w = dst_w;
    overlay->dst_h = dst_h;

    ret = drmModeSetPlane(drm_fd,
                          overlay->plane_id,
                          crtc_id,
                          fb->fb_id, 0,
                          dst_x, dst_y,
                          dst_w, dst_h,
                          src_x, src_y,
                          src_w << 16, src_h << 16);
    if (ret < 0)
        xf86DrvMsg(scrn->scrnIndex, X_ERROR,
                   "DRM set plane failed: %s\n", strerror(-ret));
    else
        overlay->visible = TRUE;
}

static void TegraVideoOverlayClose(XvPortPtr port)
{
    TegraVideoPtr priv      = port->devPriv.ptr;
    TegraOverlayPtr overlay = priv->overlay;
    ScrnInfoPtr scrn        = xf86ScreenToScrn(port->pAdaptor->pScreen);
    uint32_t crtc_id        = priv->drmmode_crtc_priv->mode_crtc->crtc_id;
    int drm_fd              = priv->drmmode_crtc_priv->drmmode->fd;
    int ret;

    xf86DrvMsg(-1, X_INFO, "%s: overlay->visible %d\n", __func__, overlay->visible);

    if (!overlay->visible)
        return;

    ret = drmModeSetPlane(drm_fd, overlay->plane_id, crtc_id,
                          0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
    if (ret < 0)
        xf86DrvMsg(scrn->scrnIndex, X_ERROR, "Failed to close overlay\n");

    TegraVideoOverlayDisableColorKey(port);

    overlay->visible = FALSE;
}

static int TegraVideoOverlayCreateFB(XvPortPtr port, uint32_t drm_format,
                                     uint32_t width, uint32_t height)
{
    TegraVideoPtr priv      = port->devPriv.ptr;
    TegraOverlayPtr overlay = priv->overlay;
    ScrnInfoPtr scrn        = xf86ScreenToScrn(port->pAdaptor->pScreen);
    drm_overlay_fb *fb      = overlay->fb;
    int drm_fd              = priv->drmmode_crtc_priv->drmmode->fd;

    xf86DrvMsg(-1, X_INFO, "%s: fb %d\n", __func__, !!fb);

    if (!fb)
        goto create_fb;

    if (fb->format != drm_format)
        goto recreate_fb;

    if (fb->width != width)
        goto recreate_fb;

    if (fb->height != height)
        goto recreate_fb;

    return Success;

recreate_fb:
    drm_free_overlay_fb(drm_fd, fb);

create_fb:
    fb = drm_create_fb(drm_fd, drm_format, width, height);

    if (fb == NULL) {
        xf86DrvMsg(scrn->scrnIndex, X_ERROR,
                   "Failed to create overlay framebuffer\n");
        return BadImplementation;
    }

    overlay->fb = fb;

    return Success;
}

static int TegraVideoOverlayBestSize(XvPortPtr port, CARD8 motion,
                                     CARD16 vid_w, CARD16 vid_h,
                                     CARD16 dst_w, CARD16 dst_h,
                                     unsigned int *actual_w,
                                     unsigned int *actual_h)
{
    *actual_w = dst_w;
    *actual_h = dst_h;

    xf86DrvMsg(-1, X_INFO, "%s:\n", __func__);

    return Success;
}

static int TegraVideoOverlaySetAttribute(XvPortPtr port, Atom attribute,
                                         INT32 value)
{
    TegraVideoPtr priv      = port->devPriv.ptr;
    TegraOverlayPtr overlay = priv->overlay;

    xf86DrvMsg(-1, X_INFO, "%s:\n", __func__);

    if (attribute == xvColorKey) {
        overlay->color_key = value;
        TegraVideoSetColorKey(port);
    } else {
        return BadMatch;
    }

    return Success;
}

static int TegraVideoOverlayGetAttribute(XvPortPtr port, Atom attribute,
                                         INT32 *value)
{
    TegraVideoPtr priv      = port->devPriv.ptr;
    TegraOverlayPtr overlay = priv->overlay;

    xf86DrvMsg(-1, X_INFO, "%s:\n", __func__);

    if (attribute == xvColorKey) {
        *value = overlay->color_key;
    } else {
        return BadMatch;
    }

    return Success;
}

static int TegraVideoOverlayStop(XvPortPtr port, DrawablePtr draw)
{
    TegraVideoPtr priv      = port->devPriv.ptr;
    TegraOverlayPtr overlay = priv->overlay;
    int drm_fd              = priv->drmmode_crtc_priv->drmmode->fd;

    xf86DrvMsg(-1, X_INFO, "%s: +\n", __func__);

    TegraVideoOverlayClose(port);
    drm_free_overlay_fb(drm_fd, overlay->fb);
    overlay->fb = NULL;

    xf86DrvMsg(-1, X_INFO, "%s: -\n", __func__);

    return Success;
}

static int TegraVideoOverlayPutImage(DrawablePtr draw, XvPortPtr port, GCPtr gc,
                                     INT16 src_x, INT16 src_y,
                                     CARD16 src_w, CARD16 src_h,
                                     INT16 dst_x, INT16 dst_y,
                                     CARD16 dst_w, CARD16 dst_h,
                                     XvImagePtr format,
                                     unsigned char *buf,
                                     Bool sync,
                                     CARD16 width, CARD16 height)
{
    TegraVideoPtr priv      = port->devPriv.ptr;
    TegraOverlayPtr overlay = priv->overlay;
    ScrnInfoPtr scrn        = xf86ScreenToScrn(port->pAdaptor->pScreen);
    DisplayModePtr mode     = scrn->currentMode;

    xf86DrvMsg(scrn->scrnIndex, X_INFO,
               "%s: draw %p src_x %d src_y %d; src_w %d src_h %d; draw->x %d draw->y %d; dst_x %d dst_y %d; dst_w %d dst_h %d; width %d height %d format %s CrtcHDisplay %d CrtcVDisplay %d\n",
               __func__, draw, src_x, src_y, src_w, src_h, draw->x, draw->y, dst_x, dst_y, dst_w, dst_h, width, height, format_name(format->id), mode->CrtcHDisplay, mode->CrtcVDisplay);

    if (!TegraVideoOverlayVisible(mode->CrtcHDisplay, mode->CrtcVDisplay,
                                  draw->x + dst_x, draw->y + dst_y,
                                  dst_w, dst_h)) {
        TegraVideoOverlayClose(port);
        return Success;
    }

    if (!xv_fourcc_valid(format->id)) {
        xf86DrvMsg(scrn->scrnIndex, X_ERROR,
                   "Unsupported FOURCC 0x%08X (%s)\n",
                   format->id, format_name(format->id));
        return BadValue;
    }

    if (TegraVideoOverlayCreateFB(port, xv_fourcc_to_drm(format->id),
                                  width, height) != Success)
        return BadImplementation;

    if (sync)
        TegraVideoVSync(port);

    drm_copy_data_to_fb(overlay->fb, buf, format->id == FOURCC_I420);

    TegraVideoOverlayShow(port,
                          src_x, src_y, src_w, src_h,
                          draw->x + dst_x, draw->y + dst_y, dst_w, dst_h);

    /* Overlay won't be closed without it */
    port->pDraw = draw;

    return Success;
}

static int TegraVideoOverlayQuery(XvPortPtr port, XvImagePtr format,
                                  unsigned short *w, unsigned short *h,
                                  int *pitches, int *offsets)
{
    int size;

//     xf86DrvMsg(-1, X_INFO, "%s: %s\n", __func__, format_name(format->id));

    switch (format->id) {
//     case FOURCC_PASSTHROUGH_YV12:
//         if (pitches) {
//             pitches[0] = (*w);
//             pitches[1] = (*w) / 2;
//             pitches[2] = (*w) / 2;
//         }
//
//         if (offsets) {
//             offsets[0] = 0;
//             offsets[1] = (*w) * (*h);
//             offsets[2] = (*w) * (*h) * 5 / 4;
//         }
//
//         size = (*w) * (*h) * 3  / 2;
//         break;
//     case FOURCC_PASSTHROUGH_RGB565:
//         if (pitches)
//             pitches[0] = (*w) * (*h) * 2;
//
//         if (offsets)
//             offsets[0] = 0;
//
//         size = (*w) * (*h) * 2;
//         break;
//     case FOURCC_PASSTHROUGH_RGB888:
//         if (pitches)
//             pitches[0] = (*w) * (*h) * 4;
//
//         if (offsets)
//             offsets[0] = 0;
//
//         size = (*w) * (*h) * 4;
//         break;
//     case FOURCC_PASSTHROUGH_BGR888:
//         if (pitches)
//             pitches[0] = (*w) * (*h) * 4;
//
//         if (offsets)
//             offsets[0] = 0;
//
//         size = (*w) * (*h) * 4;
//         break;
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
    default:
        return BadValue;
    }

    return size;
}

static void TegraVideoIgnoreGenericAdaptors(ScreenPtr pScreen)
{
    ScrnInfoPtr scrn = xf86ScreenToScrn(pScreen);
    XF86VideoAdaptorPtr *adaptors = NULL;

    if (xf86LoaderCheckSymbol("xf86XVListGenericAdaptors") == FALSE)
        return;

    if (xf86XVListGenericAdaptors(scrn, &adaptors) != 0)
        xf86DrvMsg(scrn->scrnIndex, X_ERROR,
                   "Ignoring generic xf86XV adaptors\n");

    free(adaptors);
}

void TegraXvInit(ScreenPtr pScreen)
{
    ScrnInfoPtr scrn              = xf86ScreenToScrn(pScreen);
    xf86CrtcConfigPtr config      = XF86_CRTC_CONFIG_PTR(scrn);
    xf86OutputPtr output          = config->output[config->compat_output];
    xf86CrtcPtr crtc              = output->crtc;
    drmmode_crtc_private_ptr priv = crtc->driver_private;
    drmmode_ptr drmmode           = priv->drmmode;
    TegraVideoPtr port_priv;
    TegraOverlayPtr overlay;
    XvAdaptorPtr adaptor;
    XvScreenPtr xv;
    XvPortPtr port;
    uint32_t primary_plane_id;
    uint32_t overlay_plane_id;

    if (noXvExtension)
        return;

    if (drmSetClientCap(drmmode->fd, DRM_CLIENT_CAP_UNIVERSAL_PLANES, 1)) {
        xf86DrvMsg(scrn->scrnIndex, X_ERROR, "Failed to set universal planes CAP\n");
        return;
    }

    if (drm_get_primary_plane(drmmode->fd, priv->crtc_pipe,
                              &primary_plane_id) != 0) {
        xf86DrvMsg(scrn->scrnIndex, X_ERROR, "Failed to get primary plane\n");
        return;
    }

    if (drm_get_overlay_plane(drmmode->fd, priv->crtc_pipe,
                              DRM_FORMAT_YUV420, &overlay_plane_id) != 0) {
        xf86DrvMsg(scrn->scrnIndex, X_ERROR, "Failed to get overlay plane\n");
        return;
    }

    TegraVideoIgnoreGenericAdaptors(pScreen);

    if (XvScreenInit(pScreen) != Success)
        return;

    xv = dixLookupPrivate(&pScreen->devPrivates, XvGetScreenKey());

    port_priv = xnfcalloc(1, sizeof(*port_priv));
    overlay   = xnfcalloc(1, sizeof(*overlay));
    adaptor   = xnfcalloc(1, sizeof(*adaptor));
    port      = xnfcalloc(1, sizeof(*port));

    port_priv->overlay           = overlay;
    port_priv->primary_plane_id  = primary_plane_id;
    port_priv->drmmode_crtc_priv = priv;

    overlay->plane_id  = overlay_plane_id;
    overlay->color_key = 0x000000;

    port->id          = FakeClientID(0);
    port->pAdaptor    = adaptor;
    port->pNotify     = NULL;
    port->pDraw       = NULL;
    port->client      = NULL;
    port->grab.client = NULL;
    port->time        = currentTime;
    port->devPriv.ptr = port_priv;

    adaptor->type                           = XvInputMask | XvImageMask;
    adaptor->pScreen                        = pScreen;
    adaptor->name                           = (char *)"Opentegra Video Overlay";
    adaptor->nEncodings                     = 1;
    adaptor->pEncodings                     = xnfalloc(sizeof(XvEncodingRec));
    adaptor->pEncodings[0].id               = 0;
    adaptor->pEncodings[0].pScreen          = pScreen;
    adaptor->pEncodings[0].name             = (char *)"XV_IMAGE";
    adaptor->pEncodings[0].width            = TEGRA_VIDEO_OVERLAY_MAX_WIDTH;
    adaptor->pEncodings[0].height           = TEGRA_VIDEO_OVERLAY_MAX_HEIGHT;
    adaptor->pEncodings[0].rate.numerator   = 1;
    adaptor->pEncodings[0].rate.denominator = 1;
    adaptor->pFormats                       = XvFormats;
    adaptor->nFormats                       = ARRAY_SIZE(XvFormats);
    adaptor->pAttributes                    = XvAttributes;
    adaptor->nAttributes                    = ARRAY_SIZE(XvAttributes);
    adaptor->pImages                        = XvImages;
    adaptor->nImages                        = ARRAY_SIZE(XvImages);
    adaptor->ddPutVideo                     = NULL;
    adaptor->ddPutStill                     = NULL;
    adaptor->ddGetVideo                     = NULL;
    adaptor->ddGetStill                     = NULL;
    adaptor->ddStopVideo                    = TegraVideoOverlayStop;
    adaptor->ddSetPortAttribute             = TegraVideoOverlaySetAttribute;
    adaptor->ddGetPortAttribute             = TegraVideoOverlayGetAttribute;
    adaptor->ddQueryBestSize                = TegraVideoOverlayBestSize;
    adaptor->ddPutImage                     = TegraVideoOverlayPutImage;
    adaptor->ddQueryImageAttributes         = TegraVideoOverlayQuery;
    adaptor->base_id                        = port->id;
    adaptor->pPorts                         = port;
    adaptor->nPorts                         = 1;

    AddResource(port->id, XvGetRTPort(), port);

    xv->nAdaptors = 1;
    xv->pAdaptors = adaptor;

    xvColorKey = MAKE_ATOM("XV_COLORKEY");

    xf86DrvMsg(scrn->scrnIndex, X_INFO, "%s (XV) initialized\n", adaptor->name);
}
