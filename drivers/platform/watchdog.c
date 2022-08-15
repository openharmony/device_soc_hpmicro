/*
 * Copyright (c) 2022 HPMicro.
 *
 * This file is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include <stdlib.h>
#include <stdio.h>
#include "device_resource_if.h"
#include "hdf_device_desc.h"
#include "hdf_log.h"
#include "watchdog_if.h"
#include "watchdog_core.h"
#include "hpm_wdg_drv.h"

#define HDF_LOG_TAG HPMICRO_WATCHDOG_HDF

static int32_t WatchdogDriverBind(struct HdfDeviceObject *device)
{
    HDF_LOGI("Bind\n");
    return HDF_SUCCESS;
}

static int32_t WatchdogDriverInit(struct HdfDeviceObject *device)
{
    int32_t ret = HDF_SUCCESS;
    HDF_LOGI("Init\n");

    struct DeviceResourceIface *dri = DeviceResourceGetIfaceInstance(HDF_CONFIG_SOURCE);

    uint32_t id_value = 0;
    uint32_t base;
    dri->GetUint32(device->property, "id", &id_value, 0);
    dri->GetUint32(device->property, "base", &base, 0);

    HDF_LOGI("id = %u\n", id_value);
    HDF_LOGI("base = 0x%X\n", base);
    return ret;
}

static void WatchdogDriverRelease(struct HdfDeviceObject *device)
{
    HDF_LOGE("Release");

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