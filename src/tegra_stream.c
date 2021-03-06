/*
 * Copyright (c) 2016-2017 Dmitry Osipenko <digetx@gmail.com>
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
 * THE SOFTWARE IS PROVIDED "AS IS\n", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 *
 * Authors:
 *    Arto Merilainen <amerilainen@nvidia.com>
 */

#include "xorg-server.h"
#include "xf86.h"

#include "host1x.h"
#include "tegra_stream.h"

#define ErrorMsg(fmt, args...) \
    xf86DrvMsg(-1, X_ERROR, "%s:%d/%s(): " fmt, \
               __FILE__, __LINE__, __func__, ##args)

/*
 * tegra_stream_create(channel)
 *
 * Create a stream for given channel. This function preallocates several
 * command buffers for later usage to improve performance. Streams are
 * used for generating command buffers opcode by opcode using
 * tegra_stream_push().
 */

int tegra_stream_create(struct tegra_stream *stream)
{
    stream->status = TEGRADRM_STREAM_FREE;

    return 0;
}

/*
 * tegra_stream_destroy(stream)
 *
 * Destroy the given stream object. All resrouces are released.
 */

void tegra_stream_destroy(struct tegra_stream *stream)
{
    if (!stream)
        return;

    tegra_stream_wait_fence(stream->last_fence);
    tegra_stream_put_fence(stream->last_fence);
    drm_tegra_job_free(stream->job);
}

int tegra_stream_cleanup(struct tegra_stream *stream)
{
    if (!stream)
        return -1;

    drm_tegra_job_free(stream->job);

    stream->job = NULL;
    stream->status = TEGRADRM_STREAM_FREE;

    return 0;
}

/*
 * tegra_stream_flush(stream, fence)
 *
 * Send the current contents of stream buffer. The stream must be
 * synchronized correctly (we cannot send partial streams). If
 * pointer to fence is given, the fence will contain the syncpoint value
 * that is reached when operations in the buffer are finished.
 */

int tegra_stream_flush(struct tegra_stream *stream)
{
    struct drm_tegra_fence *fence;
    int result = 0;

    if (!stream)
        return -1;

    tegra_stream_wait_fence(stream->last_fence);
    tegra_stream_put_fence(stream->last_fence);
    stream->last_fence = NULL;

    /* Reflushing is fine */
    if (stream->status == TEGRADRM_STREAM_FREE)
        return 0;

    /* Return error if stream is constructed badly */
    if (stream->status != TEGRADRM_STREAM_READY) {
        result = -1;
        goto cleanup;
    }

    result = drm_tegra_job_submit(stream->job, &fence);
    if (result != 0) {
        ErrorMsg("drm_tegra_job_submit() failed %d\n", result);
        result = -1;
        goto cleanup;
    }

    result = drm_tegra_fence_wait_timeout(fence, 1000);
    if (result != 0) {
        ErrorMsg("drm_tegra_fence_wait_timeout() failed %d\n", result);
        result = -1;
    }

    drm_tegra_fence_free(fence);

cleanup:
    tegra_stream_cleanup(stream);

    return result;
}

struct tegra_fence * tegra_stream_submit(struct tegra_stream *stream, bool gr2d)
{
    struct drm_tegra_fence *fence;
    struct tegra_fence *f;
    int result;

    if (!stream)
        return NULL;

    f = stream->last_fence;

    /* Resubmitting is fine */
    if (stream->status == TEGRADRM_STREAM_FREE)
        return f;

    /* Return error if stream is constructed badly */
    if (stream->status != TEGRADRM_STREAM_READY) {
        result = -1;
        goto cleanup;
    }

    result = drm_tegra_job_submit(stream->job, &fence);
    if (result != 0) {
        ErrorMsg("drm_tegra_job_submit() failed %d\n", result);
        result = -1;
    } else {
        f = tegra_stream_create_fence(fence, gr2d);
        tegra_stream_put_fence(stream->last_fence);
        stream->last_fence = f;
    }

cleanup:
    drm_tegra_job_free(stream->job);

    stream->job = NULL;
    stream->status = TEGRADRM_STREAM_FREE;

    return f;
}

struct tegra_fence * tegra_stream_ref_fence(struct tegra_fence *f, void *opaque)
{
    if (f) {
        f->opaque = opaque;
        f->refcnt++;
    }

    return f;
}

struct tegra_fence * tegra_stream_get_last_fence(struct tegra_stream *stream)
{
    if (stream->last_fence)
        return tegra_stream_ref_fence(stream->last_fence,
                                      stream->last_fence->opaque);

    return NULL;
}

struct tegra_fence * tegra_stream_create_fence(struct drm_tegra_fence *fence,
                                               bool gr2d)
{
    struct tegra_fence *f = calloc(1, sizeof(*f));

    if (f) {
        f->fence = fence;
        f->gr2d = gr2d;
    }

    return f;
}

bool tegra_stream_wait_fence(struct tegra_fence *f)
{
    int result;

    if (f && f->fence) {
        result = drm_tegra_fence_wait_timeout(f->fence, 1000);
        if (result != 0) {
            ErrorMsg("drm_tegra_fence_wait_timeout() failed %d\n", result);
        }

        drm_tegra_fence_free(f->fence);
        f->fence = NULL;

        return true;
    }

    return false;
}

void tegra_stream_put_fence(struct tegra_fence *f)
{
    if (f && --f->refcnt < 0) {
        drm_tegra_fence_free(f->fence);
        free(f);
    }
}

/*
 * tegra_stream_begin(stream, num_words, fence, num_fences, num_syncpt_incrs,
 *          num_relocs, class_id)
 *
 * Start constructing a stream.
 *  - num_words refer to the maximum number of words the stream can contain.
 *  - fence is a pointer to a table that contains syncpoint preconditions
 *    before the stream execution can start.
 *  - num_fences indicate the number of elements in the fence table.
 *  - num_relocs indicate the number of memory references in the buffer.
 *  - class_id refers to the class_id that is selected in the beginning of a
 *    stream. If no class id is given, the default class id (=usually the
 *    client device's class) is selected.
 *
 * This function verifies that the current buffer has enough room for holding
 * the whole stream (this is computed using num_words and num_relocs). The
 * function blocks until the stream buffer is ready for use.
 */

int tegra_stream_begin(struct tegra_stream *stream,
                       struct drm_tegra_channel *channel)
{
    int ret;

    /* check stream and its state */
    if (!(stream && stream->status == TEGRADRM_STREAM_FREE)) {
        ErrorMsg("Stream status isn't FREE\n");
        return -1;
    }

    ret = drm_tegra_job_new(&stream->job, channel);
    if (ret != 0) {
        ErrorMsg("drm_tegra_job_new() failed %d\n", ret);
        return -1;
    }

    ret = drm_tegra_pushbuf_new(&stream->buffer.pushbuf, stream->job);
    if (ret != 0) {
        ErrorMsg("drm_tegra_pushbuf_new() failed %d\n", ret);
        drm_tegra_job_free(stream->job);
        return -1;
    }

    stream->class_id = 0;
    stream->status = TEGRADRM_STREAM_CONSTRUCT;
    stream->op_done_synced = false;

    return 0;
}

/*
 * tegra_stream_push_reloc(stream, h, offset)
 *
 * Push a memory reference to the stream.
 */

int tegra_stream_push_reloc(struct tegra_stream *stream,
                            struct drm_tegra_bo *bo,
                            unsigned offset)
{
    int ret;

    if (!(stream && stream->status == TEGRADRM_STREAM_CONSTRUCT)) {
        ErrorMsg("Stream status isn't CONSTRUCT\n");
        return -1;
    }

    ret = drm_tegra_pushbuf_relocate(stream->buffer.pushbuf,
                                     bo, offset, 0);
    if (ret != 0) {
        stream->status = TEGRADRM_STREAM_CONSTRUCTION_FAILED;
        ErrorMsg("drm_tegra_pushbuf_relocate() failed %d\n", ret);
        return -1;
    }

    return 0;
}

/*
 * tegra_stream_push(stream, word)
 *
 * Push a single word to given stream.
 */

int tegra_stream_push(struct tegra_stream *stream, uint32_t word)
{
    if (!(stream && stream->status == TEGRADRM_STREAM_CONSTRUCT)) {
        ErrorMsg("Stream status isn't CONSTRUCT\n");
        return -1;
    }

    *stream->buffer.pushbuf->ptr++ = word;
    stream->op_done_synced = false;

    return 0;
}

/*
 * tegra_stream_push_setclass(stream, class_id)
 *
 * Push "set class" opcode to the stream. Do nothing if the class is already
 * active
 */

int tegra_stream_push_setclass(struct tegra_stream *stream, unsigned class_id)
{
    int result;

    if (stream->class_id == class_id)
        return 0;

    result = tegra_stream_push(stream, HOST1X_OPCODE_SETCL(0, class_id, 0));

    if (result == 0)
        stream->class_id = class_id;

    return result;
}

/*
 * tegra_stream_end(stream)
 *
 * Mark end of stream. This function pushes last syncpoint increment for
 * marking end of stream.
 */

int tegra_stream_end(struct tegra_stream *stream)
{
    int ret;

    if (!(stream && stream->status == TEGRADRM_STREAM_CONSTRUCT)) {
        ErrorMsg("Stream status isn't CONSTRUCT\n");
        return -1;
    }

    if (stream->op_done_synced)
        goto ready;

    ret = drm_tegra_pushbuf_sync(stream->buffer.pushbuf,
                                 DRM_TEGRA_SYNCPT_COND_OP_DONE);
    if (ret != 0) {
        stream->status = TEGRADRM_STREAM_CONSTRUCTION_FAILED;
        ErrorMsg("drm_tegra_pushbuf_sync() failed %d\n", ret);
        return -1;
    }

ready:
    stream->status = TEGRADRM_STREAM_READY;
    stream->op_done_synced = false;

    return 0;
}

/*
 * tegra_reloc (variable, handle, offset)
 *
 * This function creates a reloc allocation. The function should be used in
 * conjunction with tegra_stream_push_words.
 */

struct tegra_reloc tegra_reloc(const void *var_ptr, struct drm_tegra_bo *bo,
                               uint32_t offset, uint32_t var_offset)
{
    struct tegra_reloc reloc = {var_ptr, bo, offset, var_offset};
    return reloc;
}

/*
 * tegra_stream_push_words(stream, addr, words, ...)
 *
 * Push words from given address to stream. The function takes
 * reloc structs as its argument. You can generate the structs with tegra_reloc
 * function.
 */

int tegra_stream_push_words(struct tegra_stream *stream, const void *addr,
                            unsigned words, int num_relocs, ...)
{
    struct tegra_reloc reloc_arg;
    va_list ap;
    uint32_t *pushbuf_ptr;
    int ret;

    if (!(stream && stream->status == TEGRADRM_STREAM_CONSTRUCT)) {
        ErrorMsg("Stream status isn't CONSTRUCT\n");
        return -1;
    }

    ret = drm_tegra_pushbuf_prepare(stream->buffer.pushbuf, words);
    if (ret != 0) {
        stream->status = TEGRADRM_STREAM_CONSTRUCTION_FAILED;
        ErrorMsg("drm_tegra_pushbuf_prepare() failed %d\n", ret);
        return -1;
    }

    /* Class id should be set explicitly, for simplicity. */
    if (stream->class_id == 0) {
        stream->status = TEGRADRM_STREAM_CONSTRUCTION_FAILED;
        ErrorMsg("HOST1X class not specified\n");
        return -1;
    }

    /* Copy the contents */
    pushbuf_ptr = stream->buffer.pushbuf->ptr;
    memcpy(pushbuf_ptr, addr, words * sizeof(uint32_t));

    /* Copy relocs */
    va_start(ap, num_relocs);
    for (; num_relocs; num_relocs--) {
        reloc_arg = va_arg(ap, struct tegra_reloc);

        stream->buffer.pushbuf->ptr  = pushbuf_ptr;
        stream->buffer.pushbuf->ptr += reloc_arg.var_offset / sizeof(uint32_t);

        ret = drm_tegra_pushbuf_relocate(stream->buffer.pushbuf, reloc_arg.bo,
                                         reloc_arg.offset, 0);
        if (ret != 0) {
            stream->status = TEGRADRM_STREAM_CONSTRUCTION_FAILED;
            ErrorMsg("drm_tegra_pushbuf_relocate() failed %d\n", ret);
            break;
        }
    }
    va_end(ap);

    stream->buffer.pushbuf->ptr = pushbuf_ptr + words;

    return ret ? -1 : 0;
}

int tegra_stream_prep(struct tegra_stream *stream, uint32_t words)
{
    int ret;

    if (!(stream && stream->status == TEGRADRM_STREAM_CONSTRUCT)) {
        ErrorMsg("Stream status isn't CONSTRUCT\n");
        return -1;
    }

    ret = drm_tegra_pushbuf_prepare(stream->buffer.pushbuf, words);
    if (ret != 0) {
        stream->status = TEGRADRM_STREAM_CONSTRUCTION_FAILED;
        ErrorMsg("drm_tegra_pushbuf_prepare() failed %d\n", ret);
        return -1;
    }

    return 0;
}

int tegra_stream_sync(struct tegra_stream *stream,
                      enum drm_tegra_syncpt_cond cond)
{
    int ret;

    if (!(stream && stream->status == TEGRADRM_STREAM_CONSTRUCT)) {
        ErrorMsg("Stream status isn't CONSTRUCT\n");
        return -1;
    }

    ret = drm_tegra_pushbuf_sync(stream->buffer.pushbuf, cond);
    if (ret != 0) {
        stream->status = TEGRADRM_STREAM_CONSTRUCTION_FAILED;
        ErrorMsg("drm_tegra_pushbuf_sync() failed %d\n", ret);
        return -1;
    }

    if (cond == DRM_TEGRA_SYNCPT_COND_OP_DONE)
        stream->op_done_synced = true;

    return 0;
}

int tegra_stream_pushf(struct tegra_stream *stream, float f)
{
    union {
        uint32_t u;
        float f;
    } value;

    value.f = f;

    return tegra_stream_push(stream, value.u);
}
