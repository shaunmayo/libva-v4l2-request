/*
 * Copyright (C) 2007 Intel Corporation
 * Copyright (C) 2016 Florent Revest <florent.revest@free-electrons.com>
 * Copyright (C) 2018 Paul Kocialkowski <paul.kocialkowski@bootlin.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sub license, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
 * IN NO EVENT SHALL PRECISION INSIGHT AND/OR ITS SUPPLIERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef _SURFACE_H_
#define _SURFACE_H_

#include <linux/videodev2.h>

#include <va/va_backend.h>

#include "object_heap.h"
#include "v4l2.h"

struct dmabuf_h;

#define SURFACE(data, id)                                                      \
	((struct object_surface *)object_heap_lookup(&(data)->surface_heap, id))
#define SURFACE_ID_OFFSET		0x04000000

enum surface_alloc_state {
	SURFACE_NEW = 0,
	SURFACE_ALLOCED
};

struct request_data;
struct bit_stash;
struct mediabuf_qent;

struct object_surface {
	struct object_base base;

	struct request_data * rd;
	VAContextID context_id;

	VASurfaceStatus status;

	enum surface_alloc_state alloc_state;
	unsigned int seq;       /* Sequence of alloc batch */

	struct picdesc pd;

	struct bit_stash * bit_stash;

	const void *source_data;  // The address of the start of the last src chunk
//	unsigned int source_size;

#if 0
	unsigned int source_index;

	struct dmabuf_h * source_dh;
	unsigned int destination_index;
	void *destination_map[VIDEO_MAX_PLANES];
	unsigned int destination_map_lengths[VIDEO_MAX_PLANES];
	unsigned int destination_map_offsets[VIDEO_MAX_PLANES];
	void *destination_data[VIDEO_MAX_PLANES];
	unsigned int destination_sizes[VIDEO_MAX_PLANES];
	unsigned int destination_offsets[VIDEO_MAX_PLANES];
	unsigned int destination_bytesperlines[VIDEO_MAX_PLANES];
	uint64_t destination_modifiers[VIDEO_MAX_PLANES];
	unsigned int destination_planes_count;
	unsigned int destination_buffers_count;
	struct dmabuf_h * destination_dh[VIDEO_MAX_PLANES];
#endif
	struct mediabuf_qent *qent;

	unsigned int slices_size;
	unsigned int slices_count;

	struct timeval timestamp;

	union {
		struct {
			VAPictureParameterBufferMPEG2 picture;
			VASliceParameterBufferMPEG2 slice;
			VAIQMatrixBufferMPEG2 iqmatrix;
			bool iqmatrix_set;
		} mpeg2;
		struct {
			VAIQMatrixBufferH264 matrix;
			VAPictureParameterBufferH264 picture;
			VASliceParameterBufferH264 slice;
		} h264;
		struct {
			VAPictureParameterBufferHEVC picture;
			VASliceParameterBufferHEVC slice;
			VAIQMatrixBufferHEVC iqmatrix;
			bool iqmatrix_set;
		} h265;
	} params;

	bool req_one;           /* First decode of frame */
	bool needs_flush;
};

VAStatus RequestCreateSurfaces2(VADriverContextP context, unsigned int format,
				unsigned int width, unsigned int height,
				VASurfaceID *surfaces_ids,
				unsigned int surfaces_count,
				VASurfaceAttrib *attributes,
				unsigned int attributes_count);
VAStatus RequestCreateSurfaces(VADriverContextP context, int width, int height,
			       int format, int surfaces_count,
			       VASurfaceID *surfaces_ids);
VAStatus RequestDestroySurfaces(VADriverContextP context,
				VASurfaceID *surfaces_ids, int surfaces_count);
VAStatus RequestSyncSurface(VADriverContextP context, VASurfaceID surface_id);
VAStatus RequestQuerySurfaceAttributes(VADriverContextP context,
				       VAConfigID config,
				       VASurfaceAttrib *attributes,
				       unsigned int *attributes_count);
VAStatus RequestQuerySurfaceStatus(VADriverContextP context,
				   VASurfaceID surface_id,
				   VASurfaceStatus *status);
VAStatus RequestPutSurface(VADriverContextP context, VASurfaceID surface_id,
			   void *draw, short src_x, short src_y,
			   unsigned short src_width, unsigned short src_height,
			   short dst_x, short dst_y, unsigned short dst_width,
			   unsigned short dst_height, VARectangle *cliprects,
			   unsigned int cliprects_count, unsigned int flags);
VAStatus RequestLockSurface(VADriverContextP context, VASurfaceID surface_id,
			    unsigned int *fourcc, unsigned int *luma_stride,
			    unsigned int *chroma_u_stride,
			    unsigned int *chroma_v_stride,
			    unsigned int *luma_offset,
			    unsigned int *chroma_u_offset,
			    unsigned int *chroma_v_offset,
			    unsigned int *buffer_name, void **buffer);
VAStatus RequestUnlockSurface(VADriverContextP context, VASurfaceID surface_id);
VAStatus RequestExportSurfaceHandle(VADriverContextP context,
				    VASurfaceID surface_id, uint32_t mem_type,
				    uint32_t flags, void *descriptor);

struct request_data;
struct object_surface;
VAStatus queue_await_completion(struct request_data *driver_data, struct object_surface *surface_object, bool last);

VAStatus surface_attach(struct object_surface *const os,
			struct mediabufs_ctl *const mbc,
			struct dmabufs_ctrl *const dbsc,
			const VAContextID id);

#endif
