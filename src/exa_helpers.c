/*
 * Copyright (c) GRATE-DRIVER project
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

struct tegra_box {
    int x0; int y0;
    int x1; int y1;
};

static Bool exa_helper_is_simple_transform(PictTransformPtr t)
{
    double e[4];

    if (!t)
        return TRUE;

    e[0] = pixman_fixed_to_double(t->matrix[0][0]);
    e[1] = pixman_fixed_to_double(t->matrix[0][1]);
    e[2] = pixman_fixed_to_double(t->matrix[1][0]);
    e[3] = pixman_fixed_to_double(t->matrix[1][1]);

    if ((e[0] > 0.0f && e[1] == 0   && e[2] == 0   && e[3] > 0.0f) ||
        (e[0] == 0   && e[1] < 0.0f && e[2] > 0.0f && e[3] == 0) ||
        (e[0] < 0.0f && e[1] == 0   && e[2] == 0   && e[3] < 0.0f) ||
        (e[0] == 0   && e[1] > 0.0f && e[2] < 0.0f && e[3] == 0))
            return TRUE;

    return FALSE;
}

static Bool exa_helper_is_simple_transform_scale(PictTransformPtr t)
{
    if (!t)
        return TRUE;

    /*
     * This is very unlikely to happen in reality, hence ignore this
     * odd case of texture coordinates scaling to simplify vertex program.
     */
    if (t->matrix[2][0] || t->matrix[2][1])
            return FALSE;

    return TRUE;
}

static void
exa_helper_apply_transform(PictTransformPtr t,
                           struct tegra_box *in,
                           struct tegra_box *out_transformed)
{
    PictVector v;

    if (t) {
        AccelMsg("orig: %d:%d  %d:%d\n", in->x0, in->y0, in->x1, in->y1);

        v.vector[0] = pixman_int_to_fixed(in->x0);
        v.vector[1] = pixman_int_to_fixed(in->y0);
        v.vector[2] = pixman_int_to_fixed(1);

        PictureTransformPoint3d(t, &v);

        out_transformed->x0 = pixman_fixed_to_int(v.vector[0]);
        out_transformed->y0 = pixman_fixed_to_int(v.vector[1]);

        v.vector[0] = pixman_int_to_fixed(in->x1);
        v.vector[1] = pixman_int_to_fixed(in->y1);
        v.vector[2] = pixman_int_to_fixed(1);

        PictureTransformPoint3d(t, &v);

        out_transformed->x1 = pixman_fixed_to_int(v.vector[0]);
        out_transformed->y1 = pixman_fixed_to_int(v.vector[1]);

        AccelMsg("transformed: %d:%d  %d:%d\n",
                 out_transformed->x0,
                 out_transformed->y0,
                 out_transformed->x1,
                 out_transformed->y1);
    } else {
        out_transformed->x0 = in->x0;
        out_transformed->y0 = in->y0;
        out_transformed->x1 = in->x1;
        out_transformed->y1 = in->y1;
    }
}

static void
exa_helper_clip_to_pixmap_area(PixmapPtr pix,
                               struct tegra_box *in,
                               struct tegra_box *out_clipped)
{
    AccelMsg("in: %d:%d  %d:%d\n", in->x0, in->y0, in->x1, in->y1);
    AccelMsg("pix: %d:%d\n", pix->drawable.width, pix->drawable.height);

    if (in->x0 < in->x1) {
        out_clipped->x0 = max(in->x0, 0);
        out_clipped->x1 = min(in->x1, pix->drawable.width);
    } else {
        out_clipped->x0 = max(in->x1, 0);
        out_clipped->x1 = min(in->x0, pix->drawable.width);
    }

    if (in->y0 < in->y1) {
        out_clipped->y0 = max(in->y0, 0);
        out_clipped->y1 = min(in->y1, pix->drawable.height);
    } else {
        out_clipped->y0 = max(in->y1, 0);
        out_clipped->y1 = min(in->y0, pix->drawable.height);
    }

    AccelMsg("out_clipped: %d:%d  %d:%d\n",
             out_clipped->x0, out_clipped->y0,
             out_clipped->x1, out_clipped->y1);
}

static void
exa_helper_get_untransformed(PictTransformPtr t_inv,
                             struct tegra_box *in,
                             struct tegra_box *out_untransformed)
{
    PictVector v;

    if (t_inv) {
        v.vector[0] = pixman_int_to_fixed(in->x0);
        v.vector[1] = pixman_int_to_fixed(in->y0);
        v.vector[2] = pixman_int_to_fixed(1);

        PictureTransformPoint3d(t_inv, &v);

        out_untransformed->x0 = pixman_fixed_to_int(v.vector[0]);
        out_untransformed->y0 = pixman_fixed_to_int(v.vector[1]);

        v.vector[0] = pixman_int_to_fixed(in->x1);
        v.vector[1] = pixman_int_to_fixed(in->y1);
        v.vector[2] = pixman_int_to_fixed(1);

        PictureTransformPoint3d(t_inv, &v);

        out_untransformed->x1 = pixman_fixed_to_int(v.vector[0]);
        out_untransformed->y1 = pixman_fixed_to_int(v.vector[1]);

        AccelMsg("untransformed: %d:%d  %d:%d\n",
                 out_untransformed->x0,
                 out_untransformed->y0,
                 out_untransformed->x1,
                 out_untransformed->y1);
    } else {
        out_untransformed->x0 = in->x0;
        out_untransformed->y0 = in->y0;
        out_untransformed->x1 = in->x1;
        out_untransformed->y1 = in->y1;
    }
}

static void
exa_helper_clip(struct tegra_box *in_out,
                struct tegra_box *clip,
                int offset_x, int offset_y)
{
    AccelMsg("in: %d:%d  %d:%d\n", in_out->x0, in_out->y0, in_out->x1, in_out->y1);
    AccelMsg("clip: %d:%d  %d:%d\n", clip->x0, clip->y0, clip->x1, clip->y1);
    AccelMsg("offset_x: %d offset_y: %d\n", offset_x, offset_y);

    in_out->x0 = max(in_out->x0, clip->x0 + offset_x);
    in_out->y0 = max(in_out->y0, clip->y0 + offset_y);
    in_out->x1 = min(in_out->x1, clip->x1 + offset_x);
    in_out->y1 = min(in_out->y1, clip->y1 + offset_y);

    AccelMsg("out: %d:%d  %d:%d\n",
             in_out->x0, in_out->y0,
             in_out->x1, in_out->y1);
}

static inline Bool
exa_helper_degenerate(struct tegra_box *b)
{
    return (b->x0 >= b->x1 || b->y0 >= b->y1);
}
