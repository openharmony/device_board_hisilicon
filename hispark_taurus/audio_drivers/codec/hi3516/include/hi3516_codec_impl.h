/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef HI3516_CODEC_IMPL_H
#define HI3516_CODEC_IMPL_H

#include "audio_codec_if.h"
#include "securec.h"
#include "audio_control.h"
#include "audio_parse.h"
#include "osal_mem.h"
#include "osal_time.h"

#define ACODEC_REG_BASE         (0x113c0000)
#define ACODEC_MAX_REG_SIZE     (0x1000)

typedef enum {
    ACODEC_I2S_FS_8000 = 0x18,
    ACODEC_I2S_FS_11025 = 0x18,
    ACODEC_I2S_FS_12000 = 0x18,
    ACODEC_I2S_FS_16000 = 0x19,
    ACODEC_I2S_FS_22050 = 0x19,
    ACODEC_I2S_FS_24000 = 0x19,
    ACODEC_I2S_FS_32000 = 0x1a,
    ACODEC_I2S_FS_44100 = 0x1a,
    ACODEC_I2S_FS_48000 = 0x1a,
    ACODEC_I2S_FS_64000 = 0x1b,
    ACODEC_I2S_FS_96000 = 0x1b,
    ACODEC_I2S_FS_BUTT = 0x1c,
} AcodecI2sFs;

typedef enum {
    ACODEC_ADC_MODESEL_6144 = 0x0,
    ACODEC_ADC_MODESEL_4096 = 0x1,
    ACODEC_ADC_MODESEL_BUTT = 0xff,
} AcodecAdcModeSel;

typedef enum {
    AUDIO_CODEC_BIT_WIDTH_16  = 0,   /* 16bit width */
    AUDIO_CODEC_BIT_WIDTH_18  = 1,   /* 18bit width */
    AUDIO_CODEC_BIT_WIDTH_20  = 2,   /* 20bit width */
    AUDIO_CODEC_BIT_WIDTH_24  = 3,   /* 24bit width */
    AUDIO_CODEC_BIT_WIDTH_BUTT,
} AudioCodecBitWidth;

struct CodecDaiParamsVal {
    uint32_t frequencyVal;
    uint32_t formatVal;
    uint32_t channelVal;
};

int32_t Hi3516CodecHalSysInit(struct CodecData *codeData);
int32_t Hi3516CodecRegBitsRead(struct AudioMixerControl *regAttr, uint32_t *regValue);
int32_t Hi3516CodecRegBitsUpdate(struct AudioMixerControl regAttr);
int32_t Hi3516CodecRegDefaultInit(struct AudioRegCfgGroupNode **regCfgGroup);
int32_t Hi3516CodecSetAdcTuneEnable(struct AudioRegCfgGroupNode **regCfgGroup);
int32_t Hi3516CodecDaiParamsUpdate(struct AudioRegCfgGroupNode **regCfgGroup,
    struct CodecDaiParamsVal codecDaiParamsVal);

int32_t Hi3516CodecAiaoSetCtrlOps(const struct AudioKcontrol *kcontrol,
    const struct AudioCtrlElemValue *elemValue);
int32_t Hi3516CodecAiaoGetCtrlOps(const struct AudioKcontrol *kcontrol,
    struct AudioCtrlElemValue *elemValue);
#endif // __HI3516_ACODEC_H__
