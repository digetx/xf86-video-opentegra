/*
 * Copyright © 2012, 2013 Thierry Reding
 * Copyright © 2013 Erik Faye-Lund
 * Copyright © 2014 NVIDIA Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE COPYRIGHT HOLDER(S) OR AUTHOR(S) BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef __XORG_DRM_TEGRA_H__
#define __XORG_DRM_TEGRA_H__ 1

#include <stdint.h>
#include <stdlib.h>

#include <tegra_drm.h>

enum xorg_drm_tegra_class {
	XORG_DRM_TEGRA_GR2D,
	XORG_DRM_TEGRA_GR3D,
};

struct xorg_drm_tegra_bo;
struct xorg_drm_tegra;

int xorg_drm_tegra_new(struct xorg_drm_tegra **drmp, int fd);
void xorg_drm_tegra_close(struct xorg_drm_tegra *drm);

int xorg_drm_tegra_bo_new(struct xorg_drm_tegra_bo **bop, struct xorg_drm_tegra *drm,
		     uint32_t flags, uint32_t size);
int xorg_drm_tegra_bo_wrap(struct xorg_drm_tegra_bo **bop, struct xorg_drm_tegra *drm,
		      uint32_t handle, uint32_t flags, uint32_t size);
struct xorg_drm_tegra_bo *xorg_drm_tegra_bo_get(struct xorg_drm_tegra_bo *bo);
void xorg_drm_tegra_bo_put(struct xorg_drm_tegra_bo *bo);
int xorg_drm_tegra_bo_get_handle(struct xorg_drm_tegra_bo *bo, uint32_t *handle);
int xorg_drm_tegra_bo_map(struct xorg_drm_tegra_bo *bo, void **ptr);
int xorg_drm_tegra_bo_unmap(struct xorg_drm_tegra_bo *bo);

struct xorg_drm_tegra_channel;
struct xorg_drm_tegra_job;

struct xorg_drm_tegra_pushbuf {
	uint32_t *ptr;
};

struct xorg_drm_tegra_fence;

enum xorg_drm_tegra_syncpt_cond {
	XORG_DRM_TEGRA_SYNCPT_COND_IMMEDIATE,
	XORG_DRM_TEGRA_SYNCPT_COND_OP_DONE,
	XORG_DRM_TEGRA_SYNCPT_COND_RD_DONE,
	XORG_DRM_TEGRA_SYNCPT_COND_WR_SAFE,
	XORG_DRM_TEGRA_SYNCPT_COND_MAX,
};

int xorg_drm_tegra_channel_open(struct xorg_drm_tegra_channel **channelp,
			   struct xorg_drm_tegra *drm,
			   enum xorg_drm_tegra_class client);
int xorg_drm_tegra_channel_close(struct xorg_drm_tegra_channel *channel);

int xorg_drm_tegra_job_new(struct xorg_drm_tegra_job **jobp,
		      struct xorg_drm_tegra_channel *channel);
int xorg_drm_tegra_job_free(struct xorg_drm_tegra_job *job);
int xorg_drm_tegra_job_submit(struct xorg_drm_tegra_job *job,
			 struct xorg_drm_tegra_fence **fencep);

int xorg_drm_tegra_pushbuf_new(struct xorg_drm_tegra_pushbuf **pushbufp,
			  struct xorg_drm_tegra_job *job,
			  struct xorg_drm_tegra_bo *bo,
			  unsigned long offset);
int xorg_drm_tegra_pushbuf_free(struct xorg_drm_tegra_pushbuf *pushbuf);
int xorg_drm_tegra_pushbuf_relocate(struct xorg_drm_tegra_pushbuf *pushbuf,
			       struct xorg_drm_tegra_bo *target,
			       unsigned long offset,
			       unsigned long shift);
int xorg_drm_tegra_pushbuf_sync(struct xorg_drm_tegra_pushbuf *pushbuf,
			   enum xorg_drm_tegra_syncpt_cond cond);

int xorg_drm_tegra_fence_wait_timeout(struct xorg_drm_tegra_fence *fence,
				 unsigned long timeout);
int xorg_drm_tegra_fence_wait(struct xorg_drm_tegra_fence *fence);
void xorg_drm_tegra_fence_free(struct xorg_drm_tegra_fence *fence);

#endif /* __XORG_DRM_TEGRA_H__ */
