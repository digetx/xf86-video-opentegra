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
#include "xf86.h"
#include "xf86drm.h"
#include "xf86drmMode.h"
#include "windowstr.h"

#include "compat-api.h"
#include "video_overlay.h"
#include "tegra_drm.h"

#define FD_INVALID  UINT32_MAX

#define MUNMAP_VERBOSE(PTR, SIZE)                       \
    if (munmap(PTR, page_align(SIZE)) != 0)             \
        FatalError("%s: " #PTR " munmap failed: %s\n",  \
                   __FUNCTION__, strerror(errno));

static int page_align(int v)
{
    int pagesize = getpagesize();
    return (((v - 1) / pagesize) + 1) * pagesize;
}

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
        xf86DrvMsg(-1, X_ERROR,
                   "Failed to get GEM mmap FD: %s\n", strerror(-ret));
        *mapped = MAP_FAILED;
        return;
    }

    *mapped = mmap(NULL, page_align(size), PROT_READ | PROT_WRITE, MAP_SHARED,
                   drm_fd, gem_mmap.offset);
    if (*mapped == MAP_FAILED)
        xf86DrvMsg(-1, X_ERROR, "GEM mmap'ing failed: %s\n", strerror(errno));
}

static __attribute__ ((pure)) int fb_size(uint32_t format,
                                          uint32_t width, uint32_t height)
{
    switch (format) {
    case DRM_FORMAT_YUV420:
    case DRM_FORMAT_YUV422:
        return width * height;
    case DRM_FORMAT_XBGR8888:
    case DRM_FORMAT_XRGB8888:
        return width * height * 4;
    case DRM_FORMAT_RGB565:
    case DRM_FORMAT_UYVY:
    case DRM_FORMAT_YUYV:
        return width * height * 2;
    default:
        break;
    }

    return 0;
}

static __attribute__ ((pure)) int fb_size_c(uint32_t format,
                                            uint32_t width, uint32_t height)
{
    switch (format) {
    case DRM_FORMAT_YUV420:
        return width * height / 4;
    case DRM_FORMAT_YUV422:
        return width * height / 2;
    default:
        break;
    }

    return 0;
}

static __attribute__ ((pure)) int format_planar(uint32_t format)
{
    switch (format) {
    case DRM_FORMAT_YUV420:
    case DRM_FORMAT_YUV422:
        return 1;
    default:
        break;
    }

    return 0;
}

static __attribute__ ((pure)) int fb_pitch(uint32_t format, uint32_t width)
{
    switch (format) {
    case DRM_FORMAT_YUV420:
    case DRM_FORMAT_YUV422:
        return width;
    case DRM_FORMAT_XBGR8888:
    case DRM_FORMAT_XRGB8888:
        return width * 4;
    case DRM_FORMAT_RGB565:
    case DRM_FORMAT_UYVY:
    case DRM_FORMAT_YUYV:
        return width * 2;
    default:
        break;
    }

    return 0;
}

static __attribute__ ((pure)) int fb_pitch_c(uint32_t format, uint32_t width)
{
    switch (format) {
    case DRM_FORMAT_YUV420:
    case DRM_FORMAT_YUV422:
        return width / 2;
    default:
        break;
    }

    return 0;
}

static uint32_t create_gem(int drm_fd, uint32_t size)
{
    struct drm_tegra_gem_create gem = {
        .flags = 0,
    };
    int ret;

    gem.size = page_align(size);

    ret = ioctl(drm_fd, DRM_IOCTL_TEGRA_GEM_CREATE, &gem);

    if (ret) {
        xf86DrvMsg(-1, X_ERROR,
                   "Failed to create GEM[0]: %s\n", strerror(-ret));
        return FD_INVALID;
    }

    return gem.handle;
}

static void close_gem(int drm_fd, uint32_t handle)
{
    struct drm_gem_close gem = {
        .handle = handle,
    };
    int ret;

    ret = ioctl(drm_fd, DRM_IOCTL_GEM_CLOSE, &gem);
    if (ret)
        xf86DrvMsg(-1, X_ERROR,
                   "Failed to close GEM: %s\n", strerror(-ret));
}

drm_overlay_fb * drm_create_fb(int drm_fd, uint32_t drm_format,
                               uint32_t width, uint32_t height)
{
    drm_overlay_fb *fb = NULL;
    uint32_t fb_id = FD_INVALID;
    uint32_t bo_handles[3];
    uint32_t pitches[3];
    uint32_t offsets[3];
    int ret;

    if (width == 0 || height == 0)
        return NULL;

    pitches[0] = fb_pitch(drm_format, width);
    offsets[0] = 0;
    pitches[1] = fb_pitch_c(drm_format, width);
    offsets[1] = 0;
    pitches[2] = fb_pitch_c(drm_format, width);
    offsets[2] = 0;

    bo_handles[1] = FD_INVALID;
    bo_handles[2] = FD_INVALID;

    /* Allocate PLANE[0] */
    bo_handles[0] = create_gem(drm_fd, fb_size(drm_format, width, height));

    if (bo_handles[0] == FD_INVALID)
        goto error_cleanup;

    if (!format_planar(drm_format))
        goto create_framebuffer;

    /* Allocate PLANE[1] */
    bo_handles[1] = create_gem(drm_fd, fb_size_c(drm_format, width, height));

    if (bo_handles[1] == FD_INVALID)
        goto error_cleanup;

    /* Allocate PLANE[2] */
    bo_handles[2] = create_gem(drm_fd, fb_size_c(drm_format, width, height));

    if (bo_handles[2] == FD_INVALID)
        goto error_cleanup;

create_framebuffer:
    ret = drmModeAddFB2(drm_fd, width, height, drm_format,
                        bo_handles, pitches, offsets, &fb_id, 0);
    if (ret) {
        xf86DrvMsg(-1, X_ERROR,
                   "Failed to create DRM framebuffer: %s\n", strerror(-ret));
        goto error_cleanup;
    }

    fb = xnfcalloc(1, sizeof(*fb));

    fb->fb_id    = fb_id;
    fb->format   = drm_format;
    fb->width    = width;
    fb->height   = height;
    fb->bo_y_id  = bo_handles[0];
    fb->bo_cb_id = bo_handles[1];
    fb->bo_cr_id = bo_handles[2];

    if (!format_planar(drm_format))
        goto non_planar;

    mmap_gem(drm_fd, fb->bo_y_id, &fb->bo_y_mmap,
             fb_size(drm_format, width, height));

    if (fb->bo_y_mmap == MAP_FAILED)
        goto error_cleanup;

    mmap_gem(drm_fd, fb->bo_cb_id, &fb->bo_cb_mmap,
             fb_size_c(drm_format, width, height));

    if (fb->bo_cb_mmap == MAP_FAILED)
        goto error_cleanup;

    mmap_gem(drm_fd, fb->bo_cr_id, &fb->bo_cr_mmap,
             fb_size_c(drm_format, width, height));

    if (fb->bo_cr_mmap == MAP_FAILED)
        goto error_cleanup;

    return fb;

non_planar:
    mmap_gem(drm_fd, fb->bo_id, &fb->bo_mmap,
             fb_size(drm_format, width, height));

    if (fb->bo_mmap == MAP_FAILED)
        goto error_cleanup;

    return fb;

error_cleanup:
    if (fb != NULL) {
        if (format_planar(drm_format)) {
            if (fb->bo_cr_mmap && fb->bo_cr_mmap != MAP_FAILED)
                MUNMAP_VERBOSE(fb->bo_cr_mmap,
                               fb_size_c(drm_format, width, height));

            if (fb->bo_cb_mmap && fb->bo_cb_mmap != MAP_FAILED)
                MUNMAP_VERBOSE(fb->bo_cb_mmap,
                               fb_size_c(drm_format, width, height));

            if (fb->bo_y_mmap && fb->bo_y_mmap != MAP_FAILED)
                MUNMAP_VERBOSE(fb->bo_y_mmap,
                               fb_size(drm_format, width, height));
        } else {
            if (fb->bo_mmap && fb->bo_mmap != MAP_FAILED)
                MUNMAP_VERBOSE(fb->bo_mmap,
                               fb_size(drm_format, width, height));
        }

        free(fb);
    }

    if (fb_id != FD_INVALID)
        drmModeRmFB(drm_fd, fb_id);

    if (bo_handles[2] != FD_INVALID)
        close_gem(drm_fd, bo_handles[2]);

    if (bo_handles[1] != FD_INVALID)
        close_gem(drm_fd, bo_handles[1]);

    if (bo_handles[0] != FD_INVALID)
        close_gem(drm_fd, bo_handles[0]);

    return NULL;
}

void drm_free_overlay_fb(int drm_fd, drm_overlay_fb *fb)
{
    if (fb == NULL)
        return;

    if (format_planar(fb->format)) {
        MUNMAP_VERBOSE(fb->bo_y_mmap,
                       fb_size(fb->format, fb->width, fb->height));
        MUNMAP_VERBOSE(fb->bo_cb_mmap,
                       fb_size_c(fb->format, fb->width, fb->height));
        MUNMAP_VERBOSE(fb->bo_cr_mmap,
                       fb_size_c(fb->format, fb->width, fb->height));

        close_gem(drm_fd, fb->bo_y_id);
        close_gem(drm_fd, fb->bo_cb_id);
        close_gem(drm_fd, fb->bo_cr_id);
    } else {
        MUNMAP_VERBOSE(fb->bo_mmap, fb_size(fb->format, fb->width, fb->height));

        close_gem(drm_fd, fb->bo_id);
    }

    drmModeRmFB(drm_fd, fb->fb_id);

    free(fb);
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
        return -EFAULT;

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
        return -ENOENT;

    *plane_id = id;

    return 0;
}

int drm_get_primary_plane(int drm_fd, int crtc_pipe, uint32_t *plane_id)
{
    drmModePlaneRes *res;
    drmModePlane *p;
    uint32_t id = 0;
    int i;

    res = drmModeGetPlaneResources(drm_fd);
    if (!res)
        return -EFAULT;

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
        return -ENOENT;

    *plane_id = id;

    return 0;
}

void drm_copy_data_to_fb(drm_overlay_fb *fb, uint8_t *data, int swap)
{
    if (!format_planar(fb->format)) {
        memcpy(fb->bo_mmap, data, fb_size(fb->format, fb->width, fb->height));
        return;
    }

    memcpy(fb->bo_y_mmap, data, fb_size(fb->format, fb->width, fb->height));
    data += fb_size(fb->format, fb->width, fb->height);

    memcpy(swap ? fb->bo_cb_mmap : fb->bo_cr_mmap, data,
           fb_size_c(fb->format, fb->width, fb->height));
    data += fb_size_c(fb->format, fb->width, fb->height);

    memcpy(swap ? fb->bo_cr_mmap : fb->bo_cb_mmap, data,
           fb_size_c(fb->format, fb->width, fb->height));
}
