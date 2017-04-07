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

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "private.h"

#define HOST1X_OPCODE_NONINCR(offset, count) \
    ((0x2 << 28) | (((offset) & 0xfff) << 16) | ((count) & 0xffff))

static inline unsigned long
xorg_drm_tegra_pushbuf_get_offset(struct xorg_drm_tegra_pushbuf *pushbuf)
{
	struct xorg_drm_tegra_pushbuf_private *priv = pushbuf_priv(pushbuf);
	struct xorg_drm_tegra_bo *bo = priv->bo;

	return (unsigned long)pushbuf->ptr - (unsigned long)bo->map;
}

drm_public
int xorg_drm_tegra_pushbuf_new(struct xorg_drm_tegra_pushbuf **pushbufp,
			  struct xorg_drm_tegra_job *job,
			  struct xorg_drm_tegra_bo *bo,
			  unsigned long offset)
{
	struct xorg_drm_tegra_pushbuf_private *pushbuf;
	void *ptr;
	int err;

	pushbuf = calloc(1, sizeof(*pushbuf));
	if (!pushbuf)
		return -ENOMEM;

	pushbuf->bo = xorg_drm_tegra_bo_get(bo);
	DRMINITLISTHEAD(&pushbuf->list);
	pushbuf->job = job;

	err = xorg_drm_tegra_bo_map(bo, &ptr);
	if (err < 0) {
		xorg_drm_tegra_bo_put(bo);
		free(pushbuf);
		return err;
	}

	pushbuf->start = pushbuf->base.ptr = ptr + offset;
	pushbuf->offset = offset;

	DRMLISTADD(&pushbuf->list, &job->pushbufs);
	job->num_pushbufs++;

	*pushbufp = &pushbuf->base;

	return 0;
}

drm_public
int xorg_drm_tegra_pushbuf_free(struct xorg_drm_tegra_pushbuf *pushbuf)
{
	struct xorg_drm_tegra_pushbuf_private *priv = pushbuf_priv(pushbuf);

	if (!pushbuf)
		return -EINVAL;

	xorg_drm_tegra_bo_unmap(priv->bo);
	xorg_drm_tegra_bo_put(priv->bo);
	DRMLISTDEL(&priv->list);
	free(priv);

	return 0;
}

drm_public
int xorg_drm_tegra_pushbuf_relocate(struct xorg_drm_tegra_pushbuf *pushbuf,
			       struct xorg_drm_tegra_bo *target,
			       unsigned long offset,
			       unsigned long shift)
{
	struct xorg_drm_tegra_pushbuf_private *priv = pushbuf_priv(pushbuf);
	struct drm_tegra_reloc reloc;
	int err;

	memset(&reloc, 0, sizeof(reloc));
	reloc.cmdbuf.handle = priv->bo->handle;
	reloc.cmdbuf.offset = xorg_drm_tegra_pushbuf_get_offset(pushbuf);
	reloc.target.handle = target->handle;
	reloc.target.offset = offset;
	reloc.shift = shift;

	err = xorg_drm_tegra_job_add_reloc(priv->job, &reloc);
	if (err < 0)
		return err;

	return 0;
}

drm_public
int xorg_drm_tegra_pushbuf_sync(struct xorg_drm_tegra_pushbuf *pushbuf,
			   enum xorg_drm_tegra_syncpt_cond cond)
{
	struct xorg_drm_tegra_pushbuf_private *priv = pushbuf_priv(pushbuf);

	if (cond >= XORG_DRM_TEGRA_SYNCPT_COND_MAX)
		return -EINVAL;

	*pushbuf->ptr++ = HOST1X_OPCODE_NONINCR(0x0, 0x1);
	*pushbuf->ptr++ = cond << 8 | priv->job->syncpt;
	priv->job->increments++;

	return 0;
}
