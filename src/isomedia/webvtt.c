/*
 *          GPAC - Multimedia Framework C SDK
 *
 *          Authors: Cyril Concolato
 *          Copyright (c) Telecom ParisTech 2000-2012
 *                  All rights reserved
 *
 *  This file is part of GPAC / ISO Media File Format sub-project
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

#include <gpac/internal/isomedia_dev.h>
#include <gpac/constants.h>

#ifndef GPAC_DISABLE_ISOM

typedef struct 
{
    GF_ISOM_BOX
    char *string;
} GF_StringBox;

typedef struct
{
    GF_ISOM_BOX
    GF_StringBox *id;
    GF_StringBox *time;
    GF_StringBox *settings;
    GF_StringBox *payload;
} GF_VTTCueBox;

typedef struct
{
    GF_ISOM_SAMPLE_ENTRY_FIELDS
    GF_StringBox *config;
} GF_WebVTTSampleEntryBox;

GF_Box *boxstring_New(u32 type) {
    ISOM_DECL_BOX_ALLOC(GF_StringBox, type);
    return (GF_Box *)tmp;
}

static GF_Box *boxstring_new_with_data(u32 type, const char *string) {
    ISOM_DECL_BOX_ALLOC(GF_StringBox, type);
    if (string) tmp->string = gf_strdup(string);
    return (GF_Box *)tmp;
}

GF_Box *vtcu_New() 
{
    ISOM_DECL_BOX_ALLOC(GF_VTTCueBox, GF_ISOM_BOX_TYPE_VTCU);
    return (GF_Box *)tmp;
}

GF_Box *vtte_New() {
    ISOM_DECL_BOX_ALLOC(GF_Box, GF_ISOM_BOX_TYPE_VTTE);
    return (GF_Box *)tmp;
}

GF_Box *wvtt_New() 
{
    ISOM_DECL_BOX_ALLOC(GF_WebVTTSampleEntryBox, GF_ISOM_BOX_TYPE_WVTT);
    return (GF_Box *)tmp;
}

void boxstring_del(GF_Box *s)
{
    GF_StringBox *box = (GF_StringBox *)s;
    if (box->string) gf_free(box->string);
    gf_free(box);
}

void vtcu_del(GF_Box *s) 
{
    GF_VTTCueBox *box = (GF_VTTCueBox *)s;
    if (box->id) gf_isom_box_del((GF_Box *)box->id);
    if (box->settings) gf_isom_box_del((GF_Box *)box->settings);
    if (box->payload) gf_isom_box_del((GF_Box *)box->payload);
}

void vtte_del(GF_Box *s)
{
    gf_free(s);
}

void wvtt_del(GF_Box *s)
{
    GF_WebVTTSampleEntryBox *wvtt = (GF_WebVTTSampleEntryBox *)s;
    if (wvtt->config) gf_isom_box_del((GF_Box *)wvtt->config);
    gf_free(s);
}

GF_Err boxstring_Read(GF_Box *s, GF_BitStream *bs)
{
    GF_StringBox *box = (GF_StringBox *)s;
    box->string = (char *)gf_malloc((u32)(s->size+1));
    gf_bs_read_data(bs, box->string, (u32)(s->size));
    box->string[(u32)(s->size)] = 0;
    return GF_OK;
}

static GF_Err vtcu_Add(GF_Box *s, GF_Box *box)
{
    GF_VTTCueBox *cuebox = (GF_VTTCueBox *)s;
    switch(box->type) {
    case GF_ISOM_BOX_TYPE_CTIM: 
        cuebox->time = (GF_StringBox *)box;
        break;
    case GF_ISOM_BOX_TYPE_IDEN: 
        cuebox->id = (GF_StringBox *)box;
        break;
    case GF_ISOM_BOX_TYPE_STTG: 
        cuebox->settings = (GF_StringBox *)box;
        break;
    case GF_ISOM_BOX_TYPE_PAYL: 
        cuebox->payload = (GF_StringBox *)box;
        break;
    default:
        return gf_isom_box_add_default(s, box);
    }
    return GF_OK;
}

GF_Err vtcu_Read(GF_Box *s, GF_BitStream *bs)
{
    GF_VTTCueBox *cuebox = (GF_VTTCueBox *)s;
    return gf_isom_read_box_list(s, bs, vtcu_Add);
}

GF_Err vtte_Read(GF_Box *s, GF_BitStream *bs)
{
    return gf_isom_read_box_list(s, bs, gf_isom_box_add_default);
}

static GF_Err wvtt_Add(GF_Box *s, GF_Box *box)
{
    GF_WebVTTSampleEntryBox *wvtt = (GF_WebVTTSampleEntryBox *)s;
    switch(box->type) {
    case GF_ISOM_BOX_TYPE_VTTC: 
        wvtt->config = (GF_StringBox *)box;
        break;
    default:
        return gf_isom_box_add_default(s, box);
    }
    return GF_OK;
}

GF_Err wvtt_Read(GF_Box *s, GF_BitStream *bs)
{
    GF_WebVTTSampleEntryBox *wvtt = (GF_WebVTTSampleEntryBox *)s;
    return gf_isom_read_box_list(s, bs, wvtt_Add);
}

#ifndef GPAC_DISABLE_ISOM_WRITE
GF_Err boxstring_Write(GF_Box *s, GF_BitStream *bs)
{
    GF_Err e;
    GF_StringBox *box = (GF_StringBox *)s;
    e = gf_isom_box_write_header(s, bs);
    if (e) return e;
    if (box->string) {
        gf_bs_write_data(bs, box->string, (u32)(box->size-8));
    }
    return e;
}

GF_Err vtcu_Write(GF_Box *s, GF_BitStream *bs)
{
    GF_Err e;
    GF_VTTCueBox *cuebox = (GF_VTTCueBox *)s;
    e = gf_isom_box_write_header(s, bs);
    if (e) return e;
    if (cuebox->id) {
        e = gf_isom_box_write((GF_Box *)cuebox->id, bs);
        if (e) return e;
    }
    if (cuebox->settings) {
        e = gf_isom_box_write((GF_Box *)cuebox->settings, bs);
        if (e) return e;
    }
    if (cuebox->payload) {
        e = gf_isom_box_write((GF_Box *)cuebox->payload, bs);
    }
    return e;
}

GF_Err vtte_Write(GF_Box *s, GF_BitStream *bs)
{
    GF_Err e;
    e = gf_isom_box_write_header(s, bs);
    return e;
}

GF_Err wvtt_Write(GF_Box *s, GF_BitStream *bs)
{
    GF_Err e;
    GF_WebVTTSampleEntryBox *wvtt = (GF_WebVTTSampleEntryBox *)s;
    e = gf_isom_box_write_header(s, bs);
    if (wvtt->config) gf_isom_box_write((GF_Box *)wvtt->config, bs);
    return e;
}

GF_Err boxstring_Size(GF_Box *s)
{
    GF_Err e;
    GF_StringBox *box = (GF_StringBox *)s;
    e = gf_isom_box_get_size(s);
    if (e) return e;
    
    box->size += strlen(box->string);
    return GF_OK;
}

GF_Err vtcu_Size(GF_Box *s)
{
    GF_Err e;
    GF_VTTCueBox *cuebox = (GF_VTTCueBox *)s;
    e = gf_isom_box_get_size(s);
    if (e) return e;
    if (cuebox->id) {
        e = gf_isom_box_size((GF_Box *)cuebox->id);
        if (e) return e;
        cuebox->size += cuebox->id->size;
    }
    if (cuebox->settings) {
        e = gf_isom_box_size((GF_Box *)cuebox->settings);
        if (e) return e;
        cuebox->size += cuebox->settings->size;
    }
    if (cuebox->payload) {
        e = gf_isom_box_size((GF_Box *)cuebox->payload);
        if (e) return e;
        cuebox->size += cuebox->payload->size;
    }
    return GF_OK;
}

GF_Err vtte_Size(GF_Box *s)
{
    return gf_isom_box_get_size(s);
}

GF_Err wvtt_Size(GF_Box *s)
{
    GF_Err e;
    GF_WebVTTSampleEntryBox *wvtt = (GF_WebVTTSampleEntryBox *)s;
    e = gf_isom_box_get_size(s);
    if (e) return e;
    if (wvtt->config) {
        e = gf_isom_box_size((GF_Box *)wvtt->config);
        if (e) return e;
        wvtt->size += wvtt->config->size;
    }
    return e;
}
#endif

#ifndef GPAC_DISABLE_ISOM_WRITE

GF_Err gf_isom_update_webvtt_description(GF_ISOFile *movie, u32 trackNumber, u32 descriptionIndex, const char *config)
{
    GF_TrackBox *trak;
    GF_Err e;
    GF_WebVTTSampleEntryBox *wvtt;

    if (!descriptionIndex) return GF_BAD_PARAM;
    e = CanAccessMovie(movie, GF_ISOM_OPEN_WRITE);
    if (e) return e;
    
    trak = gf_isom_get_track_from_file(movie, trackNumber);
    if (!trak || !trak->Media) return GF_BAD_PARAM;

    switch (trak->Media->handler->handlerType) {
    case GF_ISOM_MEDIA_TEXT:
        break;
    default:
        return GF_BAD_PARAM;
    }

    wvtt = (GF_WebVTTSampleEntryBox*)gf_list_get(trak->Media->information->sampleTable->SampleDescription->other_boxes, descriptionIndex - 1);
    if (!wvtt) return GF_BAD_PARAM;
    switch (wvtt->type) {
    case GF_ISOM_BOX_TYPE_WVTT:
        break;
    default:
        return GF_BAD_PARAM;
    }

    trak->Media->mediaHeader->modificationTime = gf_isom_get_mp4time();

    wvtt->config = (GF_StringBox *)boxstring_new_with_data(GF_ISOM_BOX_TYPE_VTTC, config);
    return e;
}

GF_Err gf_isom_new_webvtt_description(GF_ISOFile *movie, u32 trackNumber, GF_TextSampleDescriptor *desc, char *URLname, char *URNname, u32 *outDescriptionIndex)
{
    GF_TrackBox *trak;
    GF_Err e;
    u32 dataRefIndex;
    GF_WebVTTSampleEntryBox *wvtt;

    e = CanAccessMovie(movie, GF_ISOM_OPEN_WRITE);
    if (e) return e;
    
    trak = gf_isom_get_track_from_file(movie, trackNumber);
    if (!trak || !trak->Media) return GF_BAD_PARAM;

    switch (trak->Media->handler->handlerType) {
    case GF_ISOM_MEDIA_TEXT:
        break;
    default:
        return GF_BAD_PARAM;
    }

    //get or create the data ref
    e = Media_FindDataRef(trak->Media->information->dataInformation->dref, URLname, URNname, &dataRefIndex);
    if (e) return e;
    if (!dataRefIndex) {
        e = Media_CreateDataRef(trak->Media->information->dataInformation->dref, URLname, URNname, &dataRefIndex);
        if (e) return e;
    }
    trak->Media->mediaHeader->modificationTime = gf_isom_get_mp4time();

    wvtt = (GF_WebVTTSampleEntryBox *) gf_isom_box_new(GF_ISOM_BOX_TYPE_WVTT);
    wvtt->dataReferenceIndex = dataRefIndex;
    gf_list_add(trak->Media->information->sampleTable->SampleDescription->other_boxes, wvtt);
    if (outDescriptionIndex) *outDescriptionIndex = gf_list_count(trak->Media->information->sampleTable->SampleDescription->other_boxes);
    return e;
}

static GFINLINE GF_Err webvtt_write_cue(GF_BitStream *bs, GF_WebVTTCue *cue)
{
    GF_Err e;
    GF_VTTCueBox *cuebox;
    if (!cue) return GF_OK;

    cuebox = (GF_VTTCueBox *)vtcu_New(); 
    if (cue->id) {
        cuebox->id = (GF_StringBox *)boxstring_new_with_data(GF_ISOM_BOX_TYPE_IDEN, cue->id);
    }
    if (cue->settings) {
        cuebox->settings = (GF_StringBox *)boxstring_new_with_data(GF_ISOM_BOX_TYPE_STTG, cue->settings);
    }
    if (cue->text) {
        cuebox->payload = (GF_StringBox *)boxstring_new_with_data(GF_ISOM_BOX_TYPE_PAYL, cue->text);
    }

    e = gf_isom_box_size((GF_Box *)cuebox);
    if (!e) e = gf_isom_box_write((GF_Box *)cuebox, bs);

    gf_isom_box_del((GF_Box *)cuebox);
    return e;
}

GF_WebVTTCue *gf_webvtt_cue_new()
{
    GF_WebVTTCue *cue;
    GF_SAFEALLOC(cue, GF_WebVTTCue);
    return cue;
}

GF_ISOSample *gf_isom_webvtt_to_sample(GF_WebVTTSample *samp)
{
    GF_Err e = GF_OK;
    GF_ISOSample *res;
    GF_BitStream *bs;
    u32 i;
    GF_WebVTTCue *cue;
    if (!samp) return NULL;

    bs = gf_bs_new(NULL, 0, GF_BITSTREAM_WRITE);

    i=0;
    while ((cue = (GF_WebVTTCue *)gf_list_enum(samp->cues, &i))) {
        e = webvtt_write_cue(bs, cue);
        if (e) break;
    }
    /* TODO: insert an VTTEmptyCueBox if not other cue box was added */
    if (e) {
        gf_bs_del(bs);
        return NULL;
    }
    res = gf_isom_sample_new();
    if (!res) {
        gf_bs_del(bs);
        return NULL;
    }
    gf_bs_get_content(bs, &res->data, &res->dataLength);
    gf_bs_del(bs);
    res->IsRAP = 1;
    return res;
}

#endif /*GPAC_DISABLE_ISOM_WRITE*/

#ifndef GPAC_DISABLE_ISOM_DUMP
GF_Err DumpBox(GF_Box *a, FILE * trace);
void gf_box_dump_done(char *name, GF_Box *ptr, FILE *trace);

GF_Err boxstring_dump(GF_Box *a, FILE * trace)
{
    GF_StringBox *sbox = (GF_StringBox *)a;
    fprintf(trace, "<StringBox string=\"%s\">\n", sbox->string);
    DumpBox(a, trace);
    gf_box_dump_done("StringBox", a, trace);
    return GF_OK;
}

GF_Err vtcu_dump(GF_Box *a, FILE * trace)
{
    GF_VTTCueBox *cuebox = (GF_VTTCueBox *)a;
    fprintf(trace, "<WebVTTCueBox>\n");
    DumpBox(a, trace);
    if (cuebox->id) boxstring_dump((GF_Box *)cuebox->id, trace);
    if (cuebox->settings) boxstring_dump((GF_Box *)cuebox->settings, trace);
    if (cuebox->payload) boxstring_dump((GF_Box *)cuebox->payload, trace);
    gf_box_dump_done("WebVTTCueBox", a, trace);
    return GF_OK;
}

GF_Err vtte_dump(GF_Box *a, FILE * trace) 
{
    fprintf(trace, "<WebVTTEmptyCueBox>\n");
    DumpBox(a, trace);
    gf_box_dump_done("WebVTTEmptyCueBox", a, trace);
    return GF_OK;
}

GF_Err wvtt_dump(GF_Box *a, FILE * trace)
{
    GF_WebVTTSampleEntryBox *cuebox = (GF_WebVTTSampleEntryBox *)a;
    fprintf(trace, "<WebVTTSampleEntryBox>\n");
    DumpBox(a, trace);
    if (cuebox->config) boxstring_dump((GF_Box *)cuebox->config, trace);
    gf_box_dump_done("WebVTTSampleEntryBox", a, trace);
    return GF_OK;
}
#endif

/* mark the overlapped cue in the previous sample as split */
/* duplicate the cue, mark it as split and adjust its timing */
/* adjust the end of the overlapped cue in the previous sample */
GF_WebVTTCue *gf_webvtt_cue_split_at(GF_WebVTTCue *cue, GF_WebVTTTimestamp *time)
{
    GF_WebVTTCue *dup_cue;

    cue->split         = GF_TRUE;
    cue->orig_start    = cue->start;
    cue->orig_end      = cue->end;

    GF_SAFEALLOC(dup_cue, GF_WebVTTCue);
    dup_cue->split          = GF_TRUE;
    dup_cue->start          = *time;
    dup_cue->end            = cue->end;
    dup_cue->orig_start     = cue->orig_start;
    dup_cue->orig_end       = cue->orig_end;
    dup_cue->id             = gf_strdup(cue->id);
    dup_cue->settings       = gf_strdup(cue->settings);
    dup_cue->text           = gf_strdup(cue->text);

    cue->end = *time;
    return dup_cue;
}

GF_Err gf_isom_webvtt_cue_add_property(GF_WebVTTCue *cue, GF_WebVTTCuePropertyType type, char *text_data, u32 text_len)
{
    char **prop;
    u32 len;
    if (!cue) return GF_BAD_PARAM;
    if (!text_len) return GF_OK;
    switch(type)
    {
    case WEBVTT_ID:
        prop = &cue->id;
        break;
    case WEBVTT_SETTINGS:
        prop = &cue->settings;
        break;
    case WEBVTT_PAYLOAD:
        prop = &cue->text;
        break;
    case WEBVTT_TIME:
        prop = &cue->time;
        break;
    }
    if (*prop) {
        len = strlen(*prop);
        *prop = (char*)gf_realloc(*prop, sizeof(char) * (len + text_len + 1) );
        strcpy(*prop + len, text_data);
    } else {
        len = 0;
        *prop = gf_strdup(text_data);
    }
    return GF_OK;
}

GF_WebVTTSample *gf_isom_new_webvtt_sample()
{
    GF_WebVTTSample *samp;
    GF_SAFEALLOC(samp, GF_WebVTTSample);
    if (!samp) return NULL;
    samp->cues = gf_list_new();
    return samp;
}

GF_Err gf_isom_webvtt_reset(GF_WebVTTSample *samp)
{
    return GF_OK;
}

static void gf_isom_delete_webvtt_cue(GF_WebVTTCue * cue)
{
    if (cue) {
        if (cue->id)        gf_free(cue->id);
        if (cue->settings)  gf_free(cue->settings);
        if (cue->time)      gf_free(cue->time);
        if (cue->text)      gf_free(cue->text);
        gf_free(cue);
    }
}

void gf_isom_delete_webvtt_sample(GF_WebVTTSample * samp)
{
    while (gf_list_count(samp->cues)) {
        GF_WebVTTCue *cue = (GF_WebVTTCue *)gf_list_get(samp->cues, 0);
        gf_list_rem(samp->cues, 0);
        gf_isom_delete_webvtt_cue(cue);
    }
    gf_list_del(samp->cues);
    gf_free(samp);
}

GF_WebVTTCue *gf_isom_parse_webvtt_cue(GF_BitStream *bs)
{
    GF_WebVTTCue    *cue;
    GF_StringBox    *box;
    GF_SAFEALLOC(cue, GF_WebVTTCue);

    if (!bs || !gf_bs_available(bs)) return cue;
    while (gf_bs_available(bs)) {
        GF_Box *a;
        GF_Err e = gf_isom_parse_box(&a, bs);
        if (!e) {
            switch (a->type) {
            case GF_ISOM_BOX_TYPE_IDEN:
                box = (GF_StringBox *)a;
                cue->id = gf_strdup(box->string);
                break;
            case GF_ISOM_BOX_TYPE_STTG:
                box = (GF_StringBox *)a;
                cue->settings = gf_strdup(box->string);
                break;
            case GF_ISOM_BOX_TYPE_PAYL:
                box = (GF_StringBox *)a;
                cue->text = gf_strdup(box->string);
                break;
            case GF_ISOM_BOX_TYPE_CTIM:
                box = (GF_StringBox *)a;
                cue->time = gf_strdup(box->string);
                break;
            default:
                gf_isom_box_del(a);
                break;
            }
        }
    }
    return cue;
}

GF_WebVTTSample *gf_isom_parse_webvtt_sample(GF_BitStream *bs)
{
    GF_WebVTTSample *s;
    GF_WebVTTCue    *cue;

    s = gf_isom_new_webvtt_sample();
    /*empty sample*/
    if (!bs || !gf_bs_available(bs)) return s;

    while (gf_bs_available(bs)) {
        GF_Box *a;
        GF_Err e = gf_isom_parse_box(&a, bs);
        if (!e) {
            switch (a->type) {
            case GF_ISOM_BOX_TYPE_VTTE:
                /* empty cue */
                break;
            case GF_ISOM_BOX_TYPE_VTCU:
                /* real cue */
                cue = gf_isom_parse_webvtt_cue(bs);
                gf_list_add(s->cues, cue);
            default:
                gf_isom_box_del(a);
                break;
            }
        }
    }
    return s;
}

GF_WebVTTSample *gf_isom_parse_webvtt_sample_from_data(char *data, u32 dataLength)
{
    GF_WebVTTSample *s;
    GF_BitStream *bs;
    /*empty text sample*/
    if (!data || !dataLength) {
        return gf_isom_new_webvtt_sample();
    }
    
    bs = gf_bs_new(data, dataLength, GF_BITSTREAM_READ);
    s = gf_isom_parse_webvtt_sample(bs);
    gf_bs_del(bs);
    return s;
}

GF_Err gf_isom_get_webvtt_esd(GF_MediaBox *mdia, GF_ESD **out_esd)
{
    GF_BitStream *bs;
    u32 count, i;
    Bool has_v_info;
    GF_List *sampleDesc;
    GF_ESD *esd;
    GF_TrackBox *tk;

    *out_esd = NULL;
    sampleDesc = mdia->information->sampleTable->SampleDescription->other_boxes;
    count = gf_list_count(sampleDesc);
    if (!count) return GF_ISOM_INVALID_MEDIA;
    
    esd = gf_odf_desc_esd_new(2);
    esd->decoderConfig->streamType = GF_STREAM_TEXT;
    esd->decoderConfig->objectTypeIndication = 0x08;

    bs = gf_bs_new(NULL, 0, GF_BITSTREAM_WRITE);

    /*Base3GPPFormat*/
    gf_bs_write_u8(bs, 0x10);
    /*MPEGExtendedFormat*/
    gf_bs_write_u8(bs, 0x10);
    /*profileLevel*/
    gf_bs_write_u8(bs, 0x10);
    gf_bs_write_u24(bs, mdia->mediaHeader->timeScale);
    gf_bs_write_int(bs, 0, 1);  /*no alt formats*/
    gf_bs_write_int(bs, 2, 2);  /*only out-of-band-band sample desc*/
    gf_bs_write_int(bs, 1, 1);  /*we will write sample desc*/

    /*write v info if any visual track in this movie*/
    has_v_info = GF_FALSE;
    i=0;
    while ((tk = (GF_TrackBox*)gf_list_enum(mdia->mediaTrack->moov->trackList, &i))) {
        if (tk->Media->handler && (tk->Media->handler->handlerType == GF_ISOM_MEDIA_VISUAL)) {
            has_v_info = GF_TRUE;
        }
    }
    gf_bs_write_int(bs, has_v_info, 1);

    gf_bs_write_int(bs, 0, 3);  /*reserved, spec doesn't say the values*/
    gf_bs_write_u8(bs, mdia->mediaTrack->Header->layer);
    gf_bs_write_u16(bs, mdia->mediaTrack->Header->width>>16);
    gf_bs_write_u16(bs, mdia->mediaTrack->Header->height>>16);

    /*write desc*/
    gf_bs_write_u8(bs, count);
    for (i=0; i<count; i++) {
        //GF_Tx3gSampleEntryBox *a;
        //a = (GF_Tx3gSampleEntryBox *) gf_list_get(sampleDesc, i);
        //if ((a->type != GF_ISOM_BOX_TYPE_TX3G) && (a->type != GF_ISOM_BOX_TYPE_TEXT) ) continue;
        //gf_isom_write_tx3g(a, bs, i+1, SAMPLE_INDEX_OFFSET);
    }
    if (has_v_info) {
        u32 trans;
        gf_bs_write_u16(bs, 0);
        gf_bs_write_u16(bs, 0);
        trans = mdia->mediaTrack->Header->matrix[6]; trans >>= 16;
        gf_bs_write_u16(bs, trans);
        trans = mdia->mediaTrack->Header->matrix[7]; trans >>= 16;
        gf_bs_write_u16(bs, trans);
    }

    gf_bs_get_content(bs, &esd->decoderConfig->decoderSpecificInfo->data, &esd->decoderConfig->decoderSpecificInfo->dataLength);
    gf_bs_del(bs);
    *out_esd = esd;
    return GF_OK;
}

#endif /*GPAC_DISABLE_ISOM*/
