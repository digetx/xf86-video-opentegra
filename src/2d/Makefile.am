#  Copyright (c) 2016 Dmitry Osipenko <digetx@gmail.com>
#
#  Permission is hereby granted, free of charge, to any person obtaining a
#  copy of this software and associated documentation files (the "Software"),
#  to deal in the Software without restriction, including without limitation
#  on the rights to use, copy, modify, merge, publish, distribute, sub
#  license, and/or sell copies of the Software, and to permit persons to whom
#  the Software is furnished to do so, subject to the following conditions:
#
#  The above copyright notice and this permission notice (including the next
#  paragraph) shall be included in all copies or substantial portions of the
#  Software.
#
#  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
#  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
#  FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.  IN NO EVENT SHALL
#  THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
#  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
#  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
#  THE SOFTWARE.

SUBDIRS = TLSF

AM_CFLAGS = $(XORG_CFLAGS) $(DRM_CFLAGS) $(CWARNFLAGS)

noinst_LTLIBRARIES = libtegra_2d.la
libtegra_2d_la_LIBADD = TLSF/libtlsf.la

libtegra_2d_la_SOURCES = \
	tegra_2d_context.c \
	tegra_2d_copy.c \
	tegra_2d_fill.c \
	tegra_2d_frcopy.c \
	tegra_2d_g2copy.c \
	tegra_2d_g2fill.c \
	tegra_2d_sbcopy.c \
	tegra_2d_surface.c \
	tegra_2d_util.c \
	tegra_2d_allocator.c \
	tegra_stream.c
