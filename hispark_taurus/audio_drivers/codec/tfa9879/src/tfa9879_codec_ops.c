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

#include "tfa9879_codec_ops.h"
#include "audio_codec_base.h"
#include "audio_driver_log.h"
#include "gpio_if.h"

#define HDF_LOG_TAG HDF_AUDIO_DRIVER

/* Hi35xx register address */
#define HI35XX_GPIO_REG_BASE_ADDR   (0x112F0000)
#define HI35XX_GPIO_REG_OFFSET      (0x0034)
#define HI35XX_GPIO_REG_REMAP_SIZE  (0x1000)
#define HI35XX_GPIO_REG_VAL         (0x0400)
#define HI35XX_GPIO_NUMBER          (6)
/* TFA9879 I2C Device address 1 1 0 1 1 A2 A1 R/W */
#define TFA9879_I2C_DEV_ADDR        (0x6D) // 1101101
#define TFA9879_I2C_BUS_NUMBER      (6)    // i2c6
#define TFA9879_I2C_REG_DATA_LEN    (2)    // default is 2 byte (16bits)
#define COMM_WAIT_TIMES      (10) // ms

struct I2cTransferParam g_transferParam;

// Init Function
/*
 * Hi35xx gpio0_6 pin init
*/
static int32_t Hi35xxGpioPinInit(void)
{
    int32_t ret;
    char *regGpioBase = 0;
    regGpioBase = (void *)OsalIoRemap(HI35XX_GPIO_REG_BASE_ADDR, HI35XX_GPIO_REG_REMAP_SIZE);
    if (regGpioBase == NULL) {
        AUDIO_DEVICE_LOG_ERR("regGpioBase is null.");
        return HDF_FAILURE;
    }
    SysWritel((uintptr_t)regGpioBase + HI35XX_GPIO_REG_OFFSET, HI35XX_GPIO_REG_VAL); // GPIO0_6
    if (regGpioBase != NULL) {
        OsalIoUnmap(regGpioBase);
    }
    ret = GpioSetDir(HI35XX_GPIO_NUMBER, GPIO_DIR_OUT);
    if (ret != HDF_SUCCESS) {
        AUDIO_DEVICE_LOG_ERR("GpioSetDir failed. ret:%d", ret);
        return ret;
    }
    ret = GpioWrite(HI35XX_GPIO_NUMBER, GPIO_VAL_HIGH);
    if (ret != HDF_SUCCESS) {
        AUDIO_DEVICE_LOG_ERR("GpioWrite failed. ret:%d", ret);
        return ret;
    }
    AUDIO_DEVICE_LOG_DEBUG("success.");
    return HDF_SUCCESS;
}

static int32_t Tfa9879FormatParse(enum AudioFormat format, uint16_t *bitWidth)
{
    // current set default format(standard) for 16/24 bit
    if (bitWidth == NULL) {
        AUDIO_DEVICE_LOG_ERR("input param is NULL");
        return HDF_FAILURE;
    }
    switch (format) {
        case AUDIO_FORMAT_PCM_16_BIT:
            *bitWidth = I2S_SAMPLE_FORMAT_REG_VAL_24;
            break;
        case AUDIO_FORMAT_PCM_24_BIT:
            *bitWidth = I2S_SAMPLE_FORMAT_REG_VAL_24;
            break;
        default:
            AUDIO_DEVICE_LOG_ERR("format: %d is not support.", format);
            return HDF_ERR_NOT_SUPPORT;
    }
    AUDIO_DEVICE_LOG_DEBUG(" success.");
    return HDF_SUCCESS;
}

static int32_t Tfa9879WorkStatusEnable(const struct DaiDevice *device, struct AudioRegCfgGroupNode **regCfgGroup)
{
    int ret;
    uint8_t i;
    struct AudioMixerControl *daiStartupParamsRegCfgItem = NULL;
    uint8_t daiStartupParamsRegCfgItemCount;

    ret = (regCfgGroup == NULL || regCfgGroup[AUDIO_DAI_STARTUP_PATAM_GROUP] == NULL
        || regCfgGroup[AUDIO_DAI_STARTUP_PATAM_GROUP]->regCfgItem == NULL);
    if (ret) {
        AUDIO_DEVICE_LOG_ERR("g_audioRegCfgGroupNode[AUDIO_DAI_STARTUP_PATAM_GROUP] is NULL.");
        return HDF_FAILURE;
    }
    daiStartupParamsRegCfgItem = regCfgGroup[AUDIO_DAI_STARTUP_PATAM_GROUP]->regCfgItem;
    daiStartupParamsRegCfgItemCount = regCfgGroup[AUDIO_DAI_STARTUP_PATAM_GROUP]->itemNum;

    for (i = 0; i < daiStartupParamsRegCfgItemCount; i++) {
        ret = AudioDaiRegUpdate(device, &daiStartupParamsRegCfgItem[i]);
        if (ret != HDF_SUCCESS) {
            AUDIO_DEVICE_LOG_ERR("CodecDaiRegBitsUpdate fail.");
            return HDF_FAILURE;
        }
    }
    AUDIO_DEVICE_LOG_DEBUG("success.");
    return HDF_SUCCESS;
}

// init control reg to default value
static int32_t Tfa9879DeviceCtrlRegInit(const struct CodecDevice *device, struct AudioRegCfgGroupNode **regCfgGroup)
{
    int32_t ret;
    int32_t i;
    struct AudioAddrConfig *initCfg = NULL;

    // Set codec control register(00h-14h) default value
    ret = (device == NULL || device->devData == NULL || device->devData->Write == NULL || regCfgGroup == NULL ||
           regCfgGroup[AUDIO_INIT_GROUP] == NULL || regCfgGroup[AUDIO_INIT_GROUP]->addrCfgItem == NULL);
    if (ret) {
        AUDIO_DRIVER_LOG_ERR("regCfgGroup[AUDIO_INIT_GROUP] is NULL.");
        return HDF_FAILURE;
    }

    initCfg = regCfgGroup[AUDIO_INIT_GROUP]->addrCfgItem;
    if (initCfg == NULL) {
        AUDIO_DRIVER_LOG_ERR("initCfg is NULL.");
        return HDF_FAILURE;
    }

    for (i = 0; i < regCfgGroup[AUDIO_INIT_GROUP]->itemNum; i++) {
        AUDIO_DRIVER_LOG_DEBUG("i=%d, Addr = [0x%2x]", i, initCfg[i].addr);
        ret = device->devData->Write(device, initCfg[i].addr, initCfg[i].value);
        if (ret != HDF_SUCCESS) {
            AUDIO_DRIVER_LOG_ERR("CodecI2cTransfer(write) err, regAttr.regAddr: 0x%x.\n",
                                 initCfg[i].addr);
            return HDF_FAILURE;
        }
        OsalMSleep(COMM_WAIT_TIMES);
    }
    AUDIO_DRIVER_LOG_DEBUG("success.");
    return HDF_SUCCESS;
}

int32_t Tfa9879DeviceInit(struct AudioCard *audioCard, const struct CodecDevice *device)
{
    int32_t ret;
    if ((audioCard == NULL) || (device == NULL) || device->devData == NULL) {
        AUDIO_DEVICE_LOG_ERR("input para is NULL.");
        return HDF_ERR_INVALID_OBJECT;
    }

    ret = Hi35xxGpioPinInit();
    if (ret != HDF_SUCCESS) {
        AUDIO_DEVICE_LOG_ERR("Hi35xxGpioPinInit failed.");
        return HDF_FAILURE;
    }

    g_transferParam.i2cBusNumber = TFA9879_I2C_BUS_NUMBER;
    g_transferParam.i2cDevAddr = TFA9879_I2C_DEV_ADDR;
    g_transferParam.i2cRegDataLen = TFA9879_I2C_REG_DATA_LEN;
    device->devData->privateParam = &g_transferParam;

    // Initial tfa9879 register
    ret = Tfa9879DeviceCtrlRegInit(device, device->devData->regCfgGroup);
    if (ret != HDF_SUCCESS) {
        AUDIO_DEVICE_LOG_ERR("Tfa9879DeviceCtrlRegInit failed.");
        return HDF_FAILURE;
    }

    if (AudioAddControls(audioCard, device->devData->controls, device->devData->numControls) != HDF_SUCCESS) {
        AUDIO_DRIVER_LOG_ERR("add controls failed.");
        return HDF_FAILURE;
    }

    AUDIO_DEVICE_LOG_DEBUG("success.");
    return HDF_SUCCESS;
}

int32_t Tfa9879DaiDeviceInit(struct AudioCard *card, const struct DaiDevice *device)
{
    if (device == NULL || device->devDaiName == NULL || device->devData == NULL) {
        AUDIO_DEVICE_LOG_ERR("input para is NULL.");
        return HDF_FAILURE;
    }
    (void)card;

    g_transferParam.i2cBusNumber = TFA9879_I2C_BUS_NUMBER;
    g_transferParam.i2cDevAddr = TFA9879_I2C_DEV_ADDR;
    g_transferParam.i2cRegDataLen = TFA9879_I2C_REG_DATA_LEN;
    device->devData->privateParam = &g_transferParam;

    AUDIO_DEVICE_LOG_DEBUG("success.");
    return HDF_SUCCESS;
}

int32_t Tfa9879DaiStartup(const struct AudioCard *card, const struct DaiDevice *device)
{
    int ret;

    if (device == NULL || device->devData == NULL ||
        device->devData->regCfgGroup == NULL) {
        AUDIO_DRIVER_LOG_ERR("input para is NULL.");
        return HDF_FAILURE;
    }
    (void)card;

    ret = Tfa9879WorkStatusEnable(device, device->devData->regCfgGroup);
    if (ret != HDF_SUCCESS) {
        AUDIO_DEVICE_LOG_ERR("Tfa9879WorkStatusEnable failed.");
        return HDF_FAILURE;
    }
    AUDIO_DEVICE_LOG_DEBUG("success.");
    return HDF_SUCCESS;
}

int32_t Tfa9879FrequencyParse(uint32_t rate, uint16_t *freq)
{
    if (freq == NULL) {
        AUDIO_DRIVER_LOG_ERR("input param is NULL");
        return HDF_FAILURE;
    }
    switch (rate) {
        case I2S_SAMPLE_FREQUENCY_8000:
            *freq = I2S_SAMPLE_FREQUENCY_REG_VAL_8000;
            break;
        case I2S_SAMPLE_FREQUENCY_11025:
            *freq = I2S_SAMPLE_FREQUENCY_REG_VAL_11025;
            break;
        case I2S_SAMPLE_FREQUENCY_12000:
            *freq = I2S_SAMPLE_FREQUENCY_REG_VAL_12000;
            break;
        case I2S_SAMPLE_FREQUENCY_16000:
            *freq = I2S_SAMPLE_FREQUENCY_REG_VAL_16000;
            break;
        case I2S_SAMPLE_FREQUENCY_22050:
            *freq = I2S_SAMPLE_FREQUENCY_REG_VAL_22050;
            break;
        case I2S_SAMPLE_FREQUENCY_24000:
            *freq = I2S_SAMPLE_FREQUENCY_REG_VAL_24000;
            break;
        case I2S_SAMPLE_FREQUENCY_32000:
            *freq = I2S_SAMPLE_FREQUENCY_REG_VAL_32000;
            break;
        case I2S_SAMPLE_FREQUENCY_44100:
            *freq = I2S_SAMPLE_FREQUENCY_REG_VAL_44100;
            break;
        case I2S_SAMPLE_FREQUENCY_48000:
            *freq = I2S_SAMPLE_FREQUENCY_REG_VAL_48000;
            break;
        case I2S_SAMPLE_FREQUENCY_64000:
            *freq = I2S_SAMPLE_FREQUENCY_REG_VAL_64000;
            break;
        case I2S_SAMPLE_FREQUENCY_88200:
            *freq = I2S_SAMPLE_FREQUENCY_REG_VAL_88200;
            break;
        case I2S_SAMPLE_FREQUENCY_96000:
            *freq = I2S_SAMPLE_FREQUENCY_REG_VAL_96000;
            break;
        default:
            AUDIO_DRIVER_LOG_ERR("rate: %d is not support.", rate);
            return HDF_ERR_NOT_SUPPORT;
    }
    AUDIO_DRIVER_LOG_DEBUG("success.");
    return HDF_SUCCESS;
}

// update external codec I2S frequency
int32_t Tfa9879DaiParamsUpdate(struct DaiDevice *codecDai, struct DaiParamsVal daiParamsVal)
{
    int32_t ret;
    const int itemNum = 3; // current only 3 items(frequency, format, channel)
    struct AudioMixerControl *regAttr = NULL;
    struct AudioRegCfgGroupNode **regCfgGroup = NULL;
    if (codecDai == NULL || codecDai->devData == NULL) {
        AUDIO_DEVICE_LOG_ERR("input para is NULL.");
        return HDF_ERR_INVALID_PARAM;
    }
    regCfgGroup = codecDai->devData->regCfgGroup;
    ret = (regCfgGroup == NULL || regCfgGroup[AUDIO_DAI_PATAM_GROUP] == NULL
           || regCfgGroup[AUDIO_DAI_PATAM_GROUP]->regCfgItem == NULL
           || regCfgGroup[AUDIO_DAI_PATAM_GROUP]->itemNum < itemNum);
    if (ret) {
        AUDIO_DRIVER_LOG_ERR("g_audioRegCfgGroupNode[AUDIO_DAI_PATAM_GROUP] is NULL.");
        return HDF_FAILURE;
    }
    regAttr = regCfgGroup[AUDIO_DAI_PATAM_GROUP]->regCfgItem;
    regAttr[0].value = daiParamsVal.frequencyVal;

    ret = AudioDaiRegUpdate(codecDai, &regAttr[0]);
    if (ret != HDF_SUCCESS) {
        AUDIO_DRIVER_LOG_ERR("set freq failed.");
        return HDF_FAILURE;
    }

    regAttr[1].value = daiParamsVal.formatVal;
    ret = AudioDaiRegUpdate(codecDai, &regAttr[1]);
    if (ret != HDF_SUCCESS) {
        AUDIO_DRIVER_LOG_ERR("set format failed.");
        return HDF_FAILURE;
    }
    regAttr[itemNum - 1].value = daiParamsVal.channelVal;
    ret = AudioDaiRegUpdate(codecDai, &regAttr[itemNum - 1]);
    if (ret != HDF_SUCCESS) {
        AUDIO_DRIVER_LOG_ERR("set channel failed.");
        return HDF_FAILURE;
    }
    AUDIO_DRIVER_LOG_DEBUG("success.");
    return HDF_SUCCESS;
}

int32_t Tfa9879DaiHwParams(const struct AudioCard *card, const struct AudioPcmHwParams *param)
{
    int32_t ret;
    uint16_t frequency, bitWidth;
    struct DaiParamsVal daiParamsVal;
    (void)card;
    if (param == NULL || param->cardServiceName == NULL || card == NULL ||
        card->rtd == NULL || card->rtd->codecDai == NULL || card->rtd->codecDai->devData == NULL) {
        AUDIO_DEVICE_LOG_ERR("input para is NULL.");
        return HDF_ERR_INVALID_PARAM;
    }
    ret = Tfa9879FrequencyParse(param->rate, &frequency);
    if (ret != HDF_SUCCESS) {
        AUDIO_DEVICE_LOG_ERR("Tfa9879FrequencyParse failed.");
        return HDF_ERR_NOT_SUPPORT;
    }
    ret = Tfa9879FormatParse(param->format, &bitWidth);
    if (ret != HDF_SUCCESS) {
        AUDIO_DEVICE_LOG_ERR("Tfa9879FormatParse failed.");
        return HDF_ERR_NOT_SUPPORT;
    }
    daiParamsVal.frequencyVal = frequency;
    daiParamsVal.formatVal = bitWidth;
    daiParamsVal.channelVal = param->channels;
    ret = Tfa9879DaiParamsUpdate(card->rtd->codecDai, daiParamsVal);
    if (ret != HDF_SUCCESS) {
        AUDIO_DEVICE_LOG_ERR("Tfa9879DaiParamsUpdate failed.");
        return HDF_FAILURE;
    }
    AUDIO_DEVICE_LOG_DEBUG("channels = %d, rate = %d, periodSize = %d, \
        periodCount = %d, format = %d, cardServiceName = %s \n",
        param->channels, param->rate, param->periodSize,
        param->periodCount, (uint32_t)param->format, param->cardServiceName);
    AUDIO_DEVICE_LOG_DEBUG("success.");
    return HDF_SUCCESS;
}
