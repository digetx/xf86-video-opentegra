/*
 * Copyright (C) 2012-2013 NVIDIA Corporation.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 *
 * Authors:
 *    Francis Hart <fhart@nvidia.com>
 */

#ifndef TEGRA_2D_REG_G2SB_H
#define TEGRA_2D_REG_G2SB_H

#include <stdint.h>
#include "hw_gr2d.h"

#define GR2D_RESET(reg) \
    hw->block.reg = gr2d_##reg##_reset_val_v()

#define GR2D_VAL(reg, field, value) \
    hw->block.reg = (hw->block.reg & ~gr2d_##reg##_##field##_m()) \
                  | gr2d_##reg##_##field##_f((value))

#define GR2D_DEF(reg, field, def) \
    hw->block.reg = (hw->block.reg & ~gr2d_##reg##_##field##_m()) \
                  | gr2d_##reg##_##field##_f(gr2d_##reg##_##field##_##def##_v())

static inline uint32_t gr2d_controlmain_yflip_disable_v(void)
{
    return gr2d_controlmain_yflip_dsiable_v();
}

static inline uint32_t gr2d_controlmain_turbofill_disable_v(void)
{
    return 0;
}

static inline uint32_t gr2d_controlmain_turbofill_enable_v(void)
{
    return 1;
}

static inline uint32_t gr2d_controlmain_srccd_mono_v(void)
{
    return 0;
}

static inline uint32_t gr2d_controlmain_srccd_same_v(void)
{
    return 1;
}

static inline uint32_t gr2d_controlmain_xytdw_disable_v(void)
{
    return 0;
}

static inline uint32_t gr2d_controlmain_xytdw_enable_v(void)
{
    return 1;
}

static inline uint32_t gr2d_controlmain_srcdir_disable_v(void)
{
    return 0;
}

static inline uint32_t gr2d_controlmain_srcdir_enable_v(void)
{
    return 1;
}

static inline uint32_t gr2d_controlmain_dstdir_disable_v(void)
{
    return 0;
}

static inline uint32_t gr2d_controlmain_dstdir_enable_v(void)
{
    return 1;
}

#endif  // TEGRA_2D_REG_G2SB_H
