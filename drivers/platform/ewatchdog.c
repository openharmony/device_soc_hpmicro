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
#include "hdf_log.h"
#include "osal_mem.h"
#include "watchdog_if.h"
#include "watchdog_core.h"
#include "hpm_ewdg_drv.h"
#include "hpm_soc.h"

#define HDF_LOG_TAG HPMICRO_WATCHDOG_HDF
#define EWDG_CNT_CLK_FREQ 32768UL

struct HPMWatchdogDevice {
    uint32_t id;
    uint32_t base;
    int32_t status;
    uint32_t timeoutSeconds;
};

static int32_t GetStatus(struct WatchdogCntlr *wdt, int32_t *status)
{
    struct HPMWatchdogDevice *hpmDev = (struct HPMWatchdogDevice *)wdt->priv;
    *status = hpmDev->status;
    HDF_LOGD("ID: %u, GetStatus: %d\n", hpmDev->id, *status);
    return HDF_SUCCESS;
}

static int32_t SetTimeout(struct WatchdogCntlr *wdt, uint32_t seconds)
{
    struct HPMWatchdogDevice *hpmDev = (struct HPMWatchdogDevice *)wdt->priv;

    hpmDev->timeoutSeconds = seconds;
    HDF_LOGD("ID: %u, SetTimeout: %d\n",hpmDev->id, seconds);
    return HDF_SUCCESS;
}

static int32_t GetTimeout(struct WatchdogCntlr *wdt, uint32_t *seconds)
{
    struct HPMWatchdogDevice *hpmDev = (struct HPMWatchdogDevice *)wdt->priv;

    *seconds = hpmDev->timeoutSeconds;
    HDF_LOGD("ID: %u, GetTimeout: %d\n",hpmDev->id, *seconds);
    return HDF_SUCCESS;
}

static int32_t Start(struct WatchdogCntlr *wdt)
{
    struct HPMWatchdogDevice *hpmDev = (struct HPMWatchdogDevice *)wdt->priv;
    EWDG_Type *base = (EWDG_Type *)hpmDev->base;

    ewdg_config_t config;
    ewdg_get_default_config(base, &config);

    config.enable_watchdog = true;
    config.int_rst_config.enable_timeout_reset = true;
    config.ctrl_config.use_lowlevel_timeout = false;
    uint32_t ewdg_src_clk_freq = EWDG_CNT_CLK_FREQ;
    config.ctrl_config.cnt_clk_sel = ewdg_cnt_clk_src_ext_osc_clk;

    /* Set the EWDG reset timeout to 1 second */
    config.cnt_src_freq = ewdg_src_clk_freq;
    config.ctrl_config.timeout_reset_us = hpmDev->timeoutSeconds *1000000;

    /* Initialize the WDG */
    hpm_stat_t status = ewdg_init(base, &config);
    if (status != status_success) {
        printf(" EWDG initialization failed, error_code=%d\n", status);
    }

    hpmDev->status = WATCHDOG_START;
    HDF_LOGD("ID: %u, Start", hpmDev->id);
    return HDF_SUCCESS;
}

static int32_t Stop(struct WatchdogCntlr *wdt)
{
    struct HPMWatchdogDevice *hpmDev = (struct HPMWatchdogDevice *)wdt->priv;
    EWDG_Type *base = (EWDG_Type *)hpmDev->base;
    ewdg_disable(base);
    hpmDev->status = WATCHDOG_STOP;
    HDF_LOGD("ID: %u, Stop", hpmDev->id);
    return HDF_SUCCESS;
}

static int32_t Feed(struct WatchdogCntlr *wdt)
{
    struct HPMWatchdogDevice *hpmDev = (struct HPMWatchdogDevice *)wdt->priv;
    EWDG_Type *base = (EWDG_Type *)hpmDev->base;
    ewdg_refresh(base);
    return HDF_SUCCESS;
}

static struct WatchdogMethod watchdogOps = {
    .getStatus = GetStatus,
    .setTimeout = SetTimeout,
    .getTimeout = GetTimeout,
    .start = Start,
    .stop = Stop,
    .feed = Feed,
};

static int32_t WatchdogDriverBind(struct HdfDeviceObject *device)
{
    int32_t ret = HDF_SUCCESS;
    struct WatchdogCntlr *watchdogCntlr;

    if (device == NULL) {
        ret = HDF_FAILURE;
        HDF_LOGE("Bind: HdfDeviceObject null\n");
        return ret;
    }

    watchdogCntlr = (struct WatchdogCntlr *)OsalMemCalloc(sizeof(struct WatchdogCntlr));
    if (watchdogCntlr == NULL) {
        ret = HDF_ERR_MALLOC_FAIL;
        HDF_LOGE("Bind: Services malloc Failed!!!\n");
        return ret;
    }

    watchdogCntlr->device = device;
    watchdogCntlr->ops = &watchdogOps;

    ret = WatchdogCntlrAdd(watchdogCntlr);
    if (ret) {
        HDF_LOGE("Bind: WatchdogCntlrAdd Failed: %d!!!\n", ret);
        OsalMemFree(watchdogCntlr);
        return ret;
    }

    HDF_LOGI("Bind: successed\n");
    return ret;
}

static int32_t WatchdogDriverInit(struct HdfDeviceObject *device)
{
    int32_t ret = HDF_SUCCESS;

    struct WatchdogCntlr *watchdogCntlr = WatchdogCntlrFromDevice(device);
    if (watchdogCntlr == NULL) {
        ret = HDF_FAILURE;
        HDF_LOGE("Init: get WatchdogCntlr Failed!!!\n");
        goto ERROR1;
    }
 
    struct HPMWatchdogDevice *hpmWdtDev = (struct HPMWatchdogDevice *)OsalMemCalloc(
                                        sizeof(struct HPMWatchdogDevice));
    if (hpmWdtDev == NULL) {
        ret = HDF_ERR_MALLOC_FAIL;
        HDF_LOGE("Init: HPMWatchdogDevice malloc Failed!!!\n");
        goto ERROR1;
    }

    struct DeviceResourceIface *dri = DeviceResourceGetIfaceInstance(HDF_CONFIG_SOURCE);
    if (dri == NULL) {
        ret = HDF_FAILURE;
        HDF_LOGE("Init: get DeviceResourceIface Failed!!!\n");
        goto ERROR2;
    }

    dri->GetUint32(device->property, "id", &hpmWdtDev->id, 0);
    dri->GetUint32(device->property, "base", &hpmWdtDev->base, 0);
    HDF_LOGI("Init: hpmWdtDev->id: %u\n", hpmWdtDev->id);
    HDF_LOGI("Init: hpmWdtDev->base: 0x%X\n", hpmWdtDev->base);

    watchdogCntlr->wdtId = hpmWdtDev->id;
    watchdogCntlr->priv = hpmWdtDev;

    return ret;

ERROR2:
    OsalMemFree(hpmWdtDev);
ERROR1:
    return ret;
}

static void WatchdogDriverRelease(struct HdfDeviceObject *device)
{
    if (device == NULL) {
        return;
    }

    struct WatchdogCntlr *watchdogCntlr = WatchdogCntlrFromDevice(device);
    if (watchdogCntlr) {
        OsalMemFree(watchdogCntlr);
    }
    HDF_LOGI("Release");
    return;
}


struct HdfDriverEntry g_watchdogDriverEntry = {
    .moduleVersion = 1,
    .moduleName = "HPMICRO_WATCHDOG_MODULE_HDF",
    .Bind = WatchdogDriverBind,
    .Init = WatchdogDriverInit,
    .Release = WatchdogDriverRelease,
};

HDF_INIT(g_watchdogDriverEntry);