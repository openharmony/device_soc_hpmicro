/*
 * Copyright (c) 2022 HPMicro
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdlib.h>
#include <stdio.h>
#include "device_resource_if.h"
#include "hdf_device_desc.h"
#include "osal_sem.h"
#include "osal_mem.h"
#include "hdf_log.h"
#include "spi_if.h"
#include "spi_core.h"
#include "hpm_common.h"
#include "hpm_spi_drv.h"
#include <los_interrupt.h>

#define HDF_LOG_TAG HPMICRO_SPI_HDF


struct HPMSpiDevice {
    uint32_t id;
    uint32_t base;
    uint32_t clkFreq;
    struct SpiCfg cfg;
};

static void HpmSpiConfig(struct SpiCntlr *cntlr)
{
    struct HPMSpiDevice *hpmSpiDev = (struct HPMSpiDevice *)cntlr->priv;
    SPI_Type *base = (SPI_Type *)hpmSpiDev->base;
    struct SpiCfg *cfg = &hpmSpiDev->cfg;
    spi_timing_config_t timing_config = {0};
    spi_format_config_t format_config = {0};

    spi_master_get_default_timing_config(&timing_config);
    spi_master_get_default_format_config(&format_config);

    format_config.common_config.data_len_in_bits = 8;
    format_config.common_config.mode = spi_master_mode;

    if (cfg->mode & SPI_CLK_POLARITY) {
        format_config.common_config.cpol = spi_sclk_high_idle;
    } else {
        format_config.common_config.cpol = spi_sclk_low_idle;
    }

    if (cfg->mode & SPI_CLK_PHASE) {
        format_config.common_config.cpha = spi_sclk_sampling_even_clk_edges;
    } else {
        format_config.common_config.cpha = spi_sclk_sampling_odd_clk_edges;
    }

    if (cfg->mode & SPI_MODE_LSBFE) {
        format_config.common_config.lsb = 1;
    } else {
        format_config.common_config.lsb = 0;
    }

    timing_config.master_config.clk_src_freq_in_hz = hpmSpiDev->clkFreq;
    timing_config.master_config.sclk_freq_in_hz = cfg->maxSpeedHz;

    spi_master_timing_init(base, &timing_config);
    spi_format_init(base, &format_config);
}


static int32_t HpmSpiGetCfg(struct SpiCntlr *cntlr, struct SpiCfg *cfg)
{
    struct HPMSpiDevice *hpmSpiDev = (struct HPMSpiDevice *)cntlr->priv;
    *cfg = hpmSpiDev->cfg;

    return 0;
}

static int32_t HpmSpiSetCfg(struct SpiCntlr *cntlr, struct SpiCfg *cfg)
{
    struct HPMSpiDevice *hpmSpiDev = (struct HPMSpiDevice *)cntlr->priv;
    hpmSpiDev->cfg = *cfg;

    HpmSpiConfig(cntlr);
    return 0;
}

static int32_t HpmSpiTransfer(struct SpiCntlr *cntlr, struct SpiMsg *msg, uint32_t count)
{
    int ret;
    struct HPMSpiDevice *hpmSpiDev = (struct HPMSpiDevice *)cntlr->priv;
    SPI_Type *base = (SPI_Type *)hpmSpiDev->base;

    spi_control_config_t config;
    spi_master_get_default_control_config(&config);
    uint32_t rLen;
    uint32_t wLen;

    for (int i = 0; i < count; i++) {
        if (msg[i].wbuf && msg[i].rbuf) {
            config.common_config.trans_mode = spi_trans_write_read_together;
            rLen = msg[i].len;
            wLen = msg[i].len;

        } else if (msg[i].wbuf && !msg[i].rbuf) {
            config.common_config.trans_mode = spi_trans_write_only;
            rLen = 0;
            wLen = msg[i].len;
        } else if (!msg[i].wbuf && msg[i].rbuf) {
            config.common_config.trans_mode = spi_trans_read_only;
            rLen = msg[i].len;
            wLen = 0;
        } else {
            return -1;
        }
        
        ret = spi_transfer(base, &config, NULL, NULL,
                        msg[i].wbuf, wLen,  msg[i].rbuf, rLen);
        if (ret != status_success) {
            return -1;
        }
    }
    
    return 0;
}

static int32_t HpmSpiOpen(struct SpiCntlr *cntlr)
{
    return 0;
}

static int32_t HpmSpiClose(struct SpiCntlr *cntlr)
{
    return 0;
}

static struct SpiCntlrMethod spiCntlrMethod = {
    .GetCfg = HpmSpiGetCfg,
    .SetCfg = HpmSpiSetCfg,
    .Transfer = HpmSpiTransfer,
    .Open = HpmSpiOpen,
    .Close = HpmSpiClose,
};


static int32_t SpiDriverBind(struct HdfDeviceObject *device)
{
    int32_t ret = HDF_SUCCESS;

    struct SpiCntlr *cntlr = SpiCntlrCreate(device);
    if (cntlr == NULL) {
        ret = HDF_FAILURE;
        HDF_LOGE("Bind: SpiCntlrCreate null\n");
        return ret;
    }

    HDF_LOGI("Bind: successed\n");
    return ret;
}

static int32_t SpiDriverInit(struct HdfDeviceObject *device)
{
    int32_t ret = HDF_SUCCESS;

    struct SpiCntlr *cntlr = SpiCntlrFromDevice(device);
    if (cntlr == NULL) {
        ret = HDF_FAILURE;
        HDF_LOGE("Init: SpiCntlrFromDevice null\n");
        return ret;
    }

    struct HPMSpiDevice *hpmSpiDev = (struct HPMSpiDevice *)OsalMemCalloc(
                                        sizeof(struct HPMSpiDevice));
    if (hpmSpiDev == NULL) {
        ret = HDF_ERR_MALLOC_FAIL;
        HDF_LOGE("Init: HPMSpiDevice malloc Failed!!!\n");
        goto ERROR1;
    }

    struct DeviceResourceIface *dri = DeviceResourceGetIfaceInstance(HDF_CONFIG_SOURCE);
    if (dri == NULL) {
        ret = HDF_FAILURE;
        HDF_LOGE("Init: get DeviceResourceIface Failed!!!\n");
        goto ERROR2;
    }

    dri->GetUint32(device->property, "id", &hpmSpiDev->id, 0);
    dri->GetUint32(device->property, "base", &hpmSpiDev->base, 0);
    dri->GetUint32(device->property, "clk_freq", &hpmSpiDev->clkFreq, 0);
    HDF_LOGI("Init: hpmSpiDev->id: %u\n", hpmSpiDev->id);
    HDF_LOGI("Init: hpmSpiDev->base: 0x%X\n", hpmSpiDev->base);
    HDF_LOGI("Init: hpmSpiDev->clkFreq: %u\n", hpmSpiDev->clkFreq);

    cntlr->busNum = hpmSpiDev->id;
    cntlr->priv = hpmSpiDev;
    cntlr->method = &spiCntlrMethod;

    return ret;

ERROR2:
    OsalMemFree(hpmSpiDev);
ERROR1:
    return ret;
}

static void SpiDriverRelease(struct HdfDeviceObject *device)
{
    if (device == NULL) {
        return;
    }

    struct SpiCntlr *cntlr = SpiCntlrFromDevice(device);
    if (cntlr == NULL) {
        HDF_LOGE("Release: SpiCntlrFromDevice null\n");
        return;
    }

    struct HPMSpiDevice *hpmSpiDev = (struct HPMSpiDevice *)cntlr->priv;

    if (hpmSpiDev) {
        OsalMemFree(hpmSpiDev);
    }
    
    SpiCntlrDestroy(cntlr);

    HDF_LOGI("Release");
    return;
}


struct HdfDriverEntry g_spiDriverEntry = {
    .moduleVersion = 1,
    .moduleName = "HPMICRO_SPI_MODULE_HDF",
    .Bind = SpiDriverBind,
    .Init = SpiDriverInit,
    .Release = SpiDriverRelease,
};

HDF_INIT(g_spiDriverEntry);