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

#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>

#include <X11/extensions/Xv.h>

#include <drm.h>
#include <drm_fourcc.h>

#include "xorg-server.h"
#include "xf86drm.h"
#include "xf86drmMode.h"
#include "windowstr.h"

#include "compat-api.h"
#include "video_overlay.h"
#include "tegra_drm.h"

static void mmap_gem(int drm_fd, int gem_fd, void **mapped, size_t size)
{
    struct drm_tegra_gem_mmap gem_mmap = {
        .handle = gem_fd,
        .pad = 0,
        .offset = 0,
    };
    int ret;

    ret = ioctl(drm_fd, DRM_IOCTL_TEGRA_GEM_MMAP, &gem_mmap);

    if (ret) {
        xf86DrvMsg(-1, X_ERROR, "DRM_IOCTL_TEGRA_GEM_MMAP failed %d\n", ret);
        *mapped = MAP_FAILED;
        return;
    }

    *mapped = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED,
                   drm_fd, gem_mmap.offset);

    if (*mapped == MAP_FAILED) {
        xf86DrvMsg(-1, X_ERROR, "GEM mmap'ing failed\n");
    }
}

struct drm_fb * drm_create_yv12_fb(int drm_fd, int width, int height)
{
    struct drm_tegra_gem_create gem;
    struct drm_fb *fb;
    uint32_t bo_handles[3];
    uint32_t pitches[3];
    uint32_t offsets[3];
    uint32_t fb_id;
    int ret;

    if (width <= 0 || width > TEGRA_VIDEO_OVERLAY_MAX_WIDTH)
        return NULL;

    if (height <= 0 || height > TEGRA_VIDEO_OVERLAY_MAX_HEIGHT)
        return NULL;

    pitches[0] = width;
    offsets[0] = 0;
    pitches[1] = width / 2;
    offsets[1] = 0;
    pitches[2] = width / 2;
    offsets[2] = 0;

    gem.size  = width * height;
    gem.flags = 0;

    ret = ioctl(drm_fd, DRM_IOCTL_TEGRA_GEM_CREATE, &gem);

    if (ret) {
        xf86DrvMsg(-1, X_ERROR,
                   "Luma DRM_IOCTL_TEGRA_GEM_CREATE failed %d\n", ret);
        return NULL;
    }

    bo_handles[0] = gem.handle;

    gem.size  = width * height / 4;
    gem.flags = 0;

    ret = ioctl(drm_fd, DRM_IOCTL_TEGRA_GEM_CREATE, &gem);

    if (ret) {
        xf86DrvMsg(-1, X_ERROR,
                   "Cb DRM_IOCTL_TEGRA_GEM_CREATE failed %d\n", ret);
        return NULL;
    }

    bo_handles[1] = gem.handle;

    gem.size  = width * height / 4;
    gem.flags = 0;

    ret = ioctl(drm_fd, DRM_IOCTL_TEGRA_GEM_CREATE, &gem);

    if (ret) {
        xf86DrvMsg(-1, X_ERROR,
                   "Cr DRM_IOCTL_TEGRA_GEM_CREATE failed %d\n", ret);
        return NULL;
    }

    bo_handles[2] = gem.handle;

    ret = drmModeAddFB2(drm_fd, width, height, DRM_FORMAT_YUV420,
                        bo_handles, pitches, offsets, &fb_id, 0);
    if (ret) {
        xf86DrvMsg(-1, X_ERROR, "drmModeAddFB2 failed %d\n", ret);
        return NULL;
    }

    fb = xnfcalloc(1, sizeof(*fb));

    fb->fb_id    = fb_id;
    fb->width    = width;
    fb->height   = height;
    fb->bo_y_id  = bo_handles[0];
    fb->bo_cb_id = bo_handles[1];
    fb->bo_cr_id = bo_handles[2];

    mmap_gem(drm_fd, fb->bo_y_id,  &fb->bo_y_mmap,  width * height);
    mmap_gem(drm_fd, fb->bo_cb_id, &fb->bo_cb_mmap, width * height / 4);
    mmap_gem(drm_fd, fb->bo_cr_id, &fb->bo_cr_mmap, width * height / 4);

    if (fb->bo_y_mmap == MAP_FAILED)
        return NULL;

    if (fb->bo_cb_mmap == MAP_FAILED)
        return NULL;

    if (fb->bo_cr_mmap == MAP_FAILED)
        return NULL;

    return fb;
}

static int plane_type(int drm_fd, int plane_id)
{
    drmModeObjectPropertiesPtr props;
    drmModePropertyPtr prop;
    int plane_type = -EINVAL;
    int i;

    props = drmModeObjectGetProperties(drm_fd, plane_id, DRM_MODE_OBJECT_PLANE);
    if (!props)
        return -ENODEV;

    for (i = 0; i < props->count_props && plane_type == -EINVAL; i++) {
        prop = drmModeGetProperty(drm_fd, props->props[i]);
        if (prop) {
            if (strcmp(prop->name, "type") == 0)
                plane_type = props->prop_values[i];

            drmModeFreeProperty(prop);
        }
    }

    drmModeFreeObjectProperties(props);

    return plane_type;
}

int drm_get_overlay_plane(int drm_fd, int crtc_pipe, uint32_t format,
                          uint32_t *plane_id)
{
    drmModePlaneRes *res;
    drmModePlane *p;
    uint32_t id = 0;
    int i, j;

    res = drmModeGetPlaneResources(drm_fd);
    if (!res)
        return BadValue;

    for (i = 0; i < res->count_planes && !id; i++) {
        p = drmModeGetPlane(drm_fd, res->planes[i]);
        if (!p)
            continue;

        if (!p->crtc_id && (p->possible_crtcs & (1 << crtc_pipe))) {
            if (plane_type(drm_fd, p->plane_id) == DRM_PLANE_TYPE_OVERLAY) {
                for (j = 0; j < p->count_formats; j++) {
                    if (p->formats[j] == format) {
                        id = p->plane_id;
                        break;
                    }
                }
            }
        }

        drmModeFreePlane(p);
    }

    drmModeFreePlaneResources(res);

    if (!id)
        return BadValue;

    *plane_id = id;

    return Success;
}

int drm_get_primary_plane(int drm_fd, int crtc_pipe, uint32_t *plane_id)
{
    drmModePlaneRes *res;
    drmModePlane *p;
    uint32_t id = 0;
    int i;

    res = drmModeGetPlaneResources(drm_fd);
    if (!res)
        return BadValue;

    for (i = 0; i < res->count_planes && !id; i++) {
        p = drmModeGetPlane(drm_fd, res->planes[i]);
        if (!p)
            continue;

        if (p->possible_crtcs & (1 << crtc_pipe)) {
            if (plane_type(drm_fd, p->plane_id) == DRM_PLANE_TYPE_PRIMARY) {
                id = p->plane_id;
                break;
            }
        }

        drmModeFreePlane(p);
    }

    drmModeFreePlaneResources(res);

    if (!id)
        return BadValue;

    *plane_id = id;

    return Success;
}
