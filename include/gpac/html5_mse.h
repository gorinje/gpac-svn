/*
 *			GPAC - Multimedia Framework C SDK
 *
 *			Authors: Cyril Concolato
 *			Copyright (c) Telecom ParisTech 2013-
 *					All rights reserved
 *
 *  This file is part of GPAC / HTML Media Source header
 *
 *  GPAC is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  GPAC is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */


#ifndef _GF_HTMLMSE_H_
#define _GF_HTMLMSE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <gpac/setup.h>

#include <gpac/html5_media.h>
#include <gpac/internal/smjs_api.h>

typedef enum 
{
    MEDIA_SOURCE_CLOSED = 0,
    MEDIA_SOURCE_OPEN   = 1,
    MEDIA_SOURCE_ENDED  = 2,
} GF_HTML_MediaSource_ReadyState;

typedef enum 
{
    MEDIA_SOURCE_ABORT_MODE_NONE            = 0,
    MEDIA_SOURCE_ABORT_MODE_CONTINUATION    = 1,
    MEDIA_SOURCE_ABORT_MODE_OFFSET          = 2,
} GF_HTML_MediaSource_AbortMode;

typedef enum 
{
    MEDIA_SOURCE_APPEND_STATE_WAITING_FOR_SEGMENT   = 0,
    MEDIA_SOURCE_APPEND_STATE_PARSING_INIT_SEGMENT  = 1,
    MEDIA_SOURCE_APPEND_STATE_PARSING_MEDIA_SEGMENT = 2
} GF_HTML_MediaSource_AppendState;

typedef struct
{
    /* Pointer back to the MediaSource object to which this source buffer is attached */
    struct _html_mediasource *mediasource;

    /* JavaScript counterpart for this object*/
    JSObject                *_this;

    /* MSE defined properties */
    Bool                    updating;
    GF_HTML_MediaTimeRanges buffered;
    double                  timestampOffset;
    double                  appendWindowStart;
    double                  appendWindowEnd;
    u32                     timescale;

    GF_HTML_MediaSource_AppendState append_state;
    Bool                    buffer_full_flag;
    GF_HTML_MediaSource_AbortMode   abort_mode;
    double                  continuation_timestamp;
    Bool                    continuation_timestamp_flag;
    double                  highest_end_timestamp;
    Bool                    highest_end_timestamp_set;
    Bool                    first_init_segment;

    double                  remove_start;
    double                  remove_end;

    /* GPAC internal objects */
    GF_List                 *tracks;         /* Media tracks associated to this source buffer */
    GF_List                 *input_buffer;  /* List of ArrayBuffer objects as given by appendBuffer call (only one buffer for now)*/
    void                    *prev_buffer;  /* Previous ArrayBuffer (for garbage collection)*/
    GF_InputService         *parser;        /* Media parser */
    GF_ObjectDescriptor     *service_desc;  /* MPEG-4 Object descriptor as returned by the media parser */
    Bool                    parser_connected;
    Bool                    parsing;
    GF_Mutex                *parser_mutex;
    GF_Thread               *parser_thread;
    GF_Thread               *remove_thread;

} GF_HTML_SourceBuffer;

typedef struct
{
    /* JavaScript counterpart for this object */
    JSObject                *_this;

    GF_List                 *list;
} GF_HTML_SourceBufferList;

typedef enum
{
    DURATION_NAN        = 0,
    DURATION_INFINITY   = 1,
    DURATION_VALUE      = 2
} GF_HTML_MediaSource_DurationType;

typedef struct _html_mediasource
{
    /* JavaScript context associated to all the objects */
    JSContext               *c;

    /* JavaScript counterpart for this object*/
    JSObject                *_this;

    GF_HTML_SourceBufferList sourceBuffers;
    GF_HTML_SourceBufferList activeSourceBuffers;

    double  duration;
    GF_HTML_MediaSource_DurationType    durationType;

    u32     readyState;

    /* URL created by the call to createObjectURL on this MediaSource*/
    char    *blobURI;

    /* GPAC Terminal Service object 
       it is associated to this MediaSource when the Media element uses the blobURI of this MediaSource
       should be NULL when the MediaSource is not open 
       we use only one service object for all sourceBuffers
       */
    GF_ClientService *service;
} GF_HTML_MediaSource;

GF_HTML_MediaSource *gf_mse_media_source_new();

GF_HTML_SourceBuffer   *gf_mse_source_buffer_new(GF_HTML_MediaSource *mediasource);
GF_Err                  gf_mse_source_buffer_load_parser(GF_HTML_SourceBuffer *sourcebuffer, const char *mime);
void                    gf_mse_source_buffer_del(GF_HTML_SourceBuffer *sb);
GF_Err                  gf_mse_source_buffer_abort(GF_HTML_SourceBuffer *sb, GF_HTML_MediaSource_AbortMode mode);
void                    gf_mse_source_buffer_append_arraybuffer(GF_HTML_SourceBuffer *sb, GF_HTML_ArrayBuffer *buffer);
void                    gf_mse_source_buffer_update_buffered(GF_HTML_SourceBuffer *sb);
u32                     gf_mse_source_buffer_remove(void *par);

typedef struct
{
    char        *data;
    u32         size;
    GF_SLHeader sl_header;
    Bool        is_compressed;
    Bool        is_new_data;
    GF_Err      status;
} GF_MSE_Packet;

GF_Err gf_mse_proxy(GF_InputService *parser, GF_NetworkCommand *command);
void gf_mse_packet_del(GF_MSE_Packet *packet);

#ifdef __cplusplus
}
#endif

#endif	

