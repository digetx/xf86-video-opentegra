/*
 * Copyright (c) Dmitry Osipenko 2018
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
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

pseq_to_dw_exec_nb = 7	// the number of 'EXEC' block where DW happens
alu_buffer_size = 1	// number of .rgba regs carried through pipeline

.uniforms
	[5].l = "src_fmt_alpha";

	[6].l = "mask_has_per_component_alpha";
	[6].h = "mask_fmt_alpha";
	[7].l = "mask_swap_bgr";
	[7].h = "mask_clamp_to_border";

	[8].l = "dst_fmt_alpha";
	[8].h = "src_clamp_to_border";

.asm

EXEC
	MFU:	sfu:  rcp r4
		mul0: bar, sfu, bar0
		mul1: bar, sfu, bar1
		ipl:  t0.fp20, t0.fp20, t0.fp20, t0.fp20

	// Emulate clamp-to-border for mask
	ALU:
		ALU0:	MAD  lp.lh, r3, #1, -#1
		ALU1:	MAD  lp.lh, r5, #1, -#1

	ALU:
		ALU0:	CSEL lp.lh,   r2, u7.h, #0 (this)
		ALU1:	CSEL lp.lh, alu0, #0, u7.h (other)
		ALU2:	CSEL lp.lh,   r3, u7.h, #0 (other)
		ALU3:	CSEL lp.lh, alu1, #0, u7.h

	// kill the pixel if mask is opaque
	ALU:
		ALU0:	CSEL kill, -alu0, #1, #0
;

EXEC
	// Emulate clamp-to-border for src
	ALU:
		ALU0:	MAD  lp.lh, r0, #1, -#1
		ALU1:	MAD  lp.lh, r1, #1, -#1

	ALU:
		ALU0:	CSEL lp.lh,   r0, u8.h, #0 (this)
		ALU1:	CSEL lp.lh, alu0, #0, u8.h (other)
		ALU2:	CSEL lp.lh,   r1, u8.h, #0 (other)
		ALU3:	CSEL lp.lh, alu1, #0, u8.h

	// kill the pixel if src is opaque
	ALU:
		ALU0:	CSEL kill, -alu0, #1, #0
;

EXEC
	// sample tex1 (mask)
	TEX:	tex r2, r3, tex1, r2, r3, r0

	// tmp = mask_fmt_alpha ? mask.a : 1.0
	ALU:
		ALU0:	CSEL lp.lh, -u6.h, r3.h, #1

		// swap mask ABGR to ARGB if needed
		ALU1:	CSEL lp.lh, -u7.l, r3.l, r2.l
		ALU2:	CSEL lp.lh, -u7.l, r2.l, r3.l

	// mask.r = mask_has_per_component_alpha ? mask.r : tmp
	// mask.g = mask_has_per_component_alpha ? mask.g : tmp
	// mask.b = mask_has_per_component_alpha ? mask.b : tmp
	// mask.a = dst_fmt_alpha ? tmp : 0.0
	ALU:
		ALU0:	CSEL r2.l, -u6.l, alu1, alu0
		ALU1:	CSEL r2.h, -u6.l, r2.h, alu0
		ALU2:	CSEL r3.l, -u6.l, alu2, alu0
		ALU3:	MAD  r3.h, alu0, #1, #0
;

EXEC
	// sample tex0 (src)
	TEX:	tex r0, r1, tex0, r0, r1, r2

	// src.a = src_fmt_alpha ? src.a : 1.0
	ALU:
		ALU0:	CSEL r1.h, -u5.l, r1.h, #1
;

EXEC
	ALU:
		ALU0:	MAD  r0.l, -r1.h, r2.l, #1
		ALU1:	MAD  r0.h, -r1.h, r2.h, #1
		ALU2:	MAD  r1.l, -r1.h, r3.l, #1
		ALU3:	MAD  r1.h, -r1.h, r3.h, #1

	ALU:
		ALU0:	MAD  lp.lh, alu0, #1, -#1 (this)
		ALU1:	MAD  lp.lh, alu1, #1, -#1 (other)
		ALU2:	MAD  lp.lh, alu2, #1, -#1 (other)
		ALU3:	MAD  lp.lh, alu3, #1, -#1

	// kill the pixel if src.aaaa * mask.bgra == 0
	ALU:
		ALU0:	CSEL kill, -alu0, #0, #1
;

EXEC
	// fetch dst pixel to r2,r3
	PSEQ:	0x0081000A

	// r2,r3 = (1 - src.aaaa * mask.bgra) * dst.bgra
	ALU:
		ALU0:	MAD  r0.l, r0.l, r2.l, #0
		ALU1:	MAD  r0.h, r0.h, r2.h, #0
		ALU2:	MAD  r1.l, r1.l, r3.l, #0
		ALU3:	MAD  r1.h, r1.h, r3.h, #0
;

EXEC
	ALU:
		ALU0:	MAD  lp.lh, r0.l, #1, -r2.l
		ALU1:	MAD  lp.lh, r0.h, #1, -r2.h
		ALU2:	MAD  lp.lh, r1.l, #1, -r3.l
		ALU3:	MAD  lp.lh, r1.h, #1, -r3.h

	ALU:
		ALU0:	MAD  lp.lh, abs(alu0), #1, #0 (this)
		ALU1:	MAD  lp.lh, abs(alu1), #1, #0 (other)
		ALU2:	MAD  lp.lh, abs(alu2), #1, #0 (other)
		ALU3:	MAD  lp.lh, abs(alu3), u8.l, #0

	// kill the pixel if dst is unchanged
	ALU:
		ALU0:	CSEL kill, -alu0, #0, #1
		ALU1:	CSEL r1.h, -u8.l, r1.h, #0

	DW:	store rt1, r0, r1
;
