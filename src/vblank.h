/*
 * Copyright © 2013 Keith Packard
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that copyright
 * notice and this permission notice appear in supporting documentation, and
 * that the name of the copyright holders not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  The copyright holders make no representations
 * about the suitability of this software for any purpose.  It is provided "as
 * is" without express or implied warranty.
 *
 * THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL THE COPYRIGHT HOLDERS BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
 * OF THIS SOFTWARE.
 */

#ifndef __TEGRA_VBLANK_H
#define __TEGRA_VBLANK_H

typedef void (*tegra_drm_handler_proc)(uint64_t frame,
                                       uint64_t usec,
                                       void *data);

typedef void (*tegra_drm_abort_proc)(void *data);

/**
 * A tracked handler for an event that will hopefully be generated by
 * the kernel, and what to do when it is encountered.
 */
struct tegra_drm_queue {
    struct xorg_list list;
    xf86CrtcPtr crtc;
    uint32_t seq;
    void *data;
    ScrnInfoPtr scrn;
    tegra_drm_handler_proc handler;
    tegra_drm_abort_proc abort;
};

uint32_t tegra_drm_queue_alloc(xf86CrtcPtr crtc,
                               void *data,
                               tegra_drm_handler_proc handler,
                               tegra_drm_abort_proc abort);

void tegra_drm_abort(ScrnInfoPtr scrn,
                     Bool (*match)(void *data, void *match_data),
                     void *match_data);
void tegra_drm_abort_seq(ScrnInfoPtr scrn, uint32_t seq);

xf86CrtcPtr tegra_dri2_crtc_covering_drawable(DrawablePtr pDraw);
xf86CrtcPtr tegra_covering_crtc(ScrnInfoPtr scrn, BoxPtr box,
                                xf86CrtcPtr desired, BoxPtr crtc_box_ret);

int tegra_get_crtc_ust_msc(xf86CrtcPtr crtc, CARD64 *ust, CARD64 *msc);

uint32_t tegra_crtc_msc_to_kernel_msc(xf86CrtcPtr crtc, uint64_t expect);
uint64_t tegra_kernel_msc_to_crtc_msc(xf86CrtcPtr crtc, uint32_t sequence);

#endif