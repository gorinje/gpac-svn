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
    char *string;
} GF_StringBox;

#ifndef GPAC_DISABLE_ISOM_WRITE

GF_Err gf_isom_update_webvtt_description(GF_ISOFile *movie, u32 trackNumber, u32 descriptionIndex, GF_TextSampleDescriptor *desc)
{
    GF_TrackBox *trak;
    GF_Err e;
    u32 i;
    GF_Tx3gSampleEntryBox *txt;

    if (!descriptionIndex || !desc) return GF_BAD_PARAM;
    e = CanAccessMovie(movie, GF_ISOM_OPEN_WRITE);
    if (e) return e;
    
    trak = gf_isom_get_track_from_file(movie, trackNumber);
    if (!trak || !trak->Media || !desc->font_count) return GF_BAD_PARAM;

    switch (trak->Media->handler->handlerType) {
    case GF_ISOM_MEDIA_TEXT:
    case GF_ISOM_MEDIA_SUBT:
        break;
    default:
        return GF_BAD_PARAM;
    }

    txt = (GF_Tx3gSampleEntryBox*)gf_list_get(trak->Media->information->sampleTable->SampleDescription->other_boxes, descriptionIndex - 1);
    if (!txt) return GF_BAD_PARAM;
    switch (txt->type) {
    case GF_ISOM_BOX_TYPE_TX3G:
    case GF_ISOM_BOX_TYPE_TEXT:
        break;
    default:
        return GF_BAD_PARAM;
    }

    trak->Media->mediaHeader->modificationTime = gf_isom_get_mp4time();

    txt->back_color = desc->back_color; 
    txt->default_box = desc->default_pos;
    txt->default_style = desc->default_style;
    txt->displayFlags = desc->displayFlags;
    txt->vertical_justification = desc->vert_justif;
    txt->horizontal_justification = desc->horiz_justif;
    if (txt->font_table) gf_isom_box_del((GF_Box*)txt->font_table);

    txt->font_table = (GF_FontTableBox *)gf_isom_box_new(GF_ISOM_BOX_TYPE_FTAB);
    txt->font_table->entry_count = desc->font_count;
    txt->font_table->fonts = (GF_FontRecord *) gf_malloc(sizeof(GF_FontRecord) * desc->font_count);
    for (i=0; i<desc->font_count; i++) {
        txt->font_table->fonts[i].fontID = desc->fonts[i].fontID;
        if (desc->fonts[i].fontName) txt->font_table->fonts[i].fontName = gf_strdup(desc->fonts[i].fontName);
    }
    return e;
}

GF_Err gf_isom_new_webvtt_description(GF_ISOFile *movie, u32 trackNumber, GF_TextSampleDescriptor *desc, char *URLname, char *URNname, u32 *outDescriptionIndex)
{
    GF_TrackBox *trak;
    GF_Err e;
    u32 dataRefIndex, i;
    GF_Tx3gSampleEntryBox *txt;

    e = CanAccessMovie(movie, GF_ISOM_OPEN_WRITE);
    if (e) return e;
    
    trak = gf_isom_get_track_from_file(movie, trackNumber);
    if (!trak || !trak->Media || !desc->font_count) return GF_BAD_PARAM;

    switch (trak->Media->handler->handlerType) {
    case GF_ISOM_MEDIA_TEXT:
    case GF_ISOM_MEDIA_SUBT:
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

    txt = (GF_Tx3gSampleEntryBox *) gf_isom_box_new(GF_ISOM_BOX_TYPE_TX3G);
    txt->dataReferenceIndex = dataRefIndex;
    gf_list_add(trak->Media->information->sampleTable->SampleDescription->other_boxes, txt);
    if (outDescriptionIndex) *outDescriptionIndex = gf_list_count(trak->Media->information->sampleTable->SampleDescription->other_boxes);

    txt->back_color = desc->back_color; 
    txt->default_box = desc->default_pos;
    txt->default_style = desc->default_style;
    txt->displayFlags = desc->displayFlags;
    txt->vertical_justification = desc->vert_justif;
    txt->horizontal_justification = desc->horiz_justif;
    txt->font_table = (GF_FontTableBox *)gf_isom_box_new(GF_ISOM_BOX_TYPE_FTAB);
    txt->font_table->entry_count = desc->font_count;

    txt->font_table->fonts = (GF_FontRecord *) gf_malloc(sizeof(GF_FontRecord) * desc->font_count);
    for (i=0; i<desc->font_count; i++) {
        txt->font_table->fonts[i].fontID = desc->fonts[i].fontID;
        if (desc->fonts[i].fontName) txt->font_table->fonts[i].fontName = gf_strdup(desc->fonts[i].fontName);
    }
    return e;
}

static GFINLINE GF_Err webvtt_write_box(GF_BitStream *bs, GF_Box *a)
{
    GF_Err e;
    if (!a) return GF_OK;
    e = gf_isom_box_size(a);
    if (!e) e = gf_isom_box_write(a, bs);
    return e;
}

GF_WebVTTCue *gf_webvtt_cue_new()
{
    GF_WebVTTCue *cue;
    GF_SAFEALLOC(cue, GF_WebVTTCue);
    return cue;
}

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

GF_ISOSample *gf_isom_webvtt_to_sample(GF_WebVTTSample *samp)
{
    GF_Err e = GF_OK;
    GF_ISOSample *res;
    GF_BitStream *bs;
    u32 i;
    GF_Box *a;
    if (!samp) return NULL;

    bs = gf_bs_new(NULL, 0, GF_BITSTREAM_WRITE);

    i=0;
    //while ((a = (GF_Box*)gf_list_enum(samp->cues, &i))) {
    //    e = webvtt_write_box(bs, a);
    //    if (e) break;
    //}
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
            case GF_ISOM_BOX_TYPE_VTTC:
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

GF_Err gf_isom_rewrite_webvtt_sample(GF_ISOSample *samp, u32 sampleDescriptionIndex, u32 sample_dur)
{

    return GF_OK;
}

#endif /*GPAC_DISABLE_ISOM*/
