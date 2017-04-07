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
#include <string.h>

#include "private.h"

drm_public
int xorg_drm_tegra_fence_wait_timeout(struct xorg_drm_tegra_fence *fence,
				 unsigned long timeout)
{
	struct drm_tegra_syncpt_wait args;
	int err;

	memset(&args, 0, sizeof(args));
	args.id = fence->syncpt;
	args.thresh = fence->value;
	args.timeout = timeout;

	while (true) {
		err = ioctl(fence->drm->fd, DRM_IOCTL_TEGRA_SYNCPT_WAIT, &args);
		if (err < 0) {
			if (errno == EINTR)
				continue;

			xf86DrvMsg(-1, X_INFO, "%s: %d: IOCTL failed\n", __func__, __LINE__);

			perror("DRM_IOCTL_TEGRA_SYNCPT_WAIT\n");
			return -errno;
		}

		break;
	}

	return 0;
}

drm_public
int xorg_drm_tegra_fence_wait(struct xorg_drm_tegra_fence *fence)
{
	return xorg_drm_tegra_fence_wait_timeout(fence, -1);
}

drm_public
void xorg_drm_tegra_fence_free(struct xorg_drm_tegra_fence *fence)
{
	free(fence);
}
