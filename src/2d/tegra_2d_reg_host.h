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

#ifndef TEGRA_2D_REG_HOST_H_
#define TEGRA_2D_REG_HOST_H_

#include <stdint.h>
#include "host1x01_hardware.h"
#include "hw_gr2d.h"
#include "tegra_2d_util.h"

#define ADDR(reg) gr2d_##reg##_r()

#define OPCODE_INCR(word, reg, count)                                   \
    hw->block.op = host1x_opcode_nonincr(ADDR(reg), count)

#define OPCODE_NONINCR(op, reg, count)                                  \
    hw->block.op = host1x_opcode_nonincr(ADDR(reg), count)

#define OPCODE_MASK(op, addr, mask)                                     \
    CT_ASSERT((mask) <= 0xffff);                                        \
    hw->block.op = host1x_opcode_mask(addr, mask)

#define OPCODE_MASK2(op, reg0, reg1)                                    \
    CT_ASSERT(ADDR(reg0) < ADDR(reg1));                                 \
    CT_ASSERT(ADDR(reg1) - ADDR(reg0) < 16);                            \
    OPCODE_MASK(op, ADDR(reg0),                                         \
                host1x_mask2(ADDR(reg0), ADDR(reg0)) |                  \
                host1x_mask2(ADDR(reg0), ADDR(reg1)))

#define OPCODE_MASK3(op, reg0, reg1, reg2)                              \
    CT_ASSERT(ADDR(reg0) < ADDR(reg1));                                 \
    CT_ASSERT(ADDR(reg1) < ADDR(reg2));                                 \
    CT_ASSERT(ADDR(reg2) - ADDR(reg0) < 16);                            \
    OPCODE_MASK(op, ADDR(reg0),                                         \
                host1x_mask2(ADDR(reg0), ADDR(reg0)) |                  \
                host1x_mask2(ADDR(reg0), ADDR(reg1)) |                  \
                host1x_mask2(ADDR(reg0), ADDR(reg2)))

#define OPCODE_MASK4(op, reg0, reg1, reg2, reg3)                        \
    CT_ASSERT(ADDR(reg0) < ADDR(reg1));                                 \
    CT_ASSERT(ADDR(reg1) < ADDR(reg2));                                 \
    CT_ASSERT(ADDR(reg2) < ADDR(reg3));                                 \
    CT_ASSERT(ADDR(reg3) - ADDR(reg0) < 16);                            \
    OPCODE_MASK(op, ADDR(reg0),                                         \
                host1x_mask2(ADDR(reg0), ADDR(reg0)) |                  \
                host1x_mask2(ADDR(reg0), ADDR(reg1)) |                  \
                host1x_mask2(ADDR(reg0), ADDR(reg2)) |                  \
                host1x_mask2(ADDR(reg0), ADDR(reg3)))

#define OPCODE_MASK5(op, reg0, reg1, reg2, reg3, reg4)                  \
    CT_ASSERT(ADDR(reg0) < ADDR(reg1));                                 \
    CT_ASSERT(ADDR(reg1) < ADDR(reg2));                                 \
    CT_ASSERT(ADDR(reg2) < ADDR(reg3));                                 \
    CT_ASSERT(ADDR(reg3) < ADDR(reg4));                                 \
    CT_ASSERT(ADDR(reg4) - ADDR(reg0) < 16);                            \
    OPCODE_MASK(op, ADDR(reg0),                                         \
                host1x_mask2(ADDR(reg0), ADDR(reg0)) |                  \
                host1x_mask2(ADDR(reg0), ADDR(reg1)) |                  \
                host1x_mask2(ADDR(reg0), ADDR(reg2)) |                  \
                host1x_mask2(ADDR(reg0), ADDR(reg3)) |                  \
                host1x_mask2(ADDR(reg0), ADDR(reg4)))

#define OPCODE_MASK6(op, reg0, reg1, reg2, reg3, reg4, reg5)            \
    CT_ASSERT(ADDR(reg0) < ADDR(reg1));                                 \
    CT_ASSERT(ADDR(reg1) < ADDR(reg2));                                 \
    CT_ASSERT(ADDR(reg2) < ADDR(reg3));                                 \
    CT_ASSERT(ADDR(reg3) < ADDR(reg4));                                 \
    CT_ASSERT(ADDR(reg4) < ADDR(reg5));                                 \
    CT_ASSERT(ADDR(reg5) - ADDR(reg0) < 16);                            \
    OPCODE_MASK(op, ADDR(reg0),                                         \
                host1x_mask2(ADDR(reg0), ADDR(reg0)) |                  \
                host1x_mask2(ADDR(reg0), ADDR(reg1)) |                  \
                host1x_mask2(ADDR(reg0), ADDR(reg2)) |                  \
                host1x_mask2(ADDR(reg0), ADDR(reg3)) |                  \
                host1x_mask2(ADDR(reg0), ADDR(reg4)) |                  \
                host1x_mask2(ADDR(reg0), ADDR(reg5)))

#define OPCODE_MASK7(op, reg0, reg1, reg2, reg3, reg4, reg5, reg6)      \
    CT_ASSERT(ADDR(reg0) < ADDR(reg1));                                 \
    CT_ASSERT(ADDR(reg1) < ADDR(reg2));                                 \
    CT_ASSERT(ADDR(reg2) < ADDR(reg3));                                 \
    CT_ASSERT(ADDR(reg3) < ADDR(reg4));                                 \
    CT_ASSERT(ADDR(reg4) < ADDR(reg5));                                 \
    CT_ASSERT(ADDR(reg5) < ADDR(reg6));                                 \
    CT_ASSERT(ADDR(reg6) - ADDR(reg0) < 16);                            \
    OPCODE_MASK(op, ADDR(reg0),                                         \
                host1x_mask2(ADDR(reg0), ADDR(reg0)) |                  \
                host1x_mask2(ADDR(reg0), ADDR(reg1)) |                  \
                host1x_mask2(ADDR(reg0), ADDR(reg2)) |                  \
                host1x_mask2(ADDR(reg0), ADDR(reg3)) |                  \
                host1x_mask2(ADDR(reg0), ADDR(reg4)) |                  \
                host1x_mask2(ADDR(reg0), ADDR(reg5)) |                  \
                host1x_mask2(ADDR(reg0), ADDR(reg6)))

#endif
