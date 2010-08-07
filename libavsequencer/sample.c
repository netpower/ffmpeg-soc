/*
 * Implement AVSequencer samples management
 * Copyright (c) 2010 Sebastian Vater <cdgs.basty@googlemail.com>
 *
 * This file is part of FFmpeg.
 *
 * FFmpeg is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * FFmpeg is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with FFmpeg; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/**
 * @file
 * Implement AVSequencer samples management.
 */

#include "libavsequencer/avsequencer.h"

AVSequencerSample *avseq_sample_create(void) {
    return av_mallocz(sizeof(AVSequencerSample));
}

int avseq_sample_open(AVSequencerInstrument *instrument, AVSequencerSample *sample,
                      int16_t *data, uint32_t length) {
    AVSequencerSample **sample_list = instrument->sample_list;
    uint8_t samples                 = instrument->samples;
    int res;

    if (!sample || !++samples) {
        return AVERROR_INVALIDDATA;
    } else if (!(sample_list = av_realloc(sample_list, samples * sizeof(AVSequencerSample *)))) {
        av_log(sample, AV_LOG_ERROR, "cannot allocate sample storage container.\n");
        return AVERROR(ENOMEM);
    }

    sample->bits_per_sample = 16;
    sample->rate            = 8363; // NTSC frequency (60 Hz sequencers), for PAL use 8287
    sample->rate_max        = 0xFFFFFFFF;
    sample->global_volume   = 255;
    sample->volume          = 255;
    sample->panning         = -128;

    if (length && (res = avseq_sample_data_open(sample, data, length)) < 0) {
        av_free(sample_list);
        return res;
    }

    sample_list[samples]    = sample;
    instrument->sample_list = sample_list;
    instrument->samples     = samples;

    return 0;
}

int avseq_sample_data_open(AVSequencerSample *sample, int16_t *data, uint32_t samples) {
    uint32_t size = FFALIGN(samples * sample->bits_per_sample, 8) >> 3;

    if (!sample) {
        return AVERROR_INVALIDDATA;
    } else if (data) {
        sample->flags = AVSEQ_SAMPLE_FLAG_REDIRECT;
    } else if (!(data = av_mallocz(size))) {
        av_log(sample, AV_LOG_ERROR, "cannot allocate sample data.\n");
        return AVERROR(ENOMEM);
    }

    sample->data    = data;
    sample->size    = size;
    sample->samples = samples;

    return 0;
}