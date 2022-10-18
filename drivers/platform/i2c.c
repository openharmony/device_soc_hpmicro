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
#include "i2c_if.h"
#include "i2c_core.h"
#include "hpm_i2c_drv.h"
#include <los_interrupt.h>

#define HDF_LOG_TAG HPMICRO_I2C_HDF

struct HPMI2cDevice {
    uint32_t id;
    uint32_t base;
    uint32_t clkFreq;
};

static int32_t hpmTransfer(struct I2cCntlr *cntlr, struct I2cMsg *msgs, int16_t count)
{
    struct HPMI2cDevice *hpmI2cDev = (struct HPMI2cDevice *)cntlr->priv;
    I2C_Type *base = (I2C_Type *)hpmI2cDev->base;

    for (int i = 0; i < count; i++) {
        if (msgs[i].flags & I2C_FLAG_ADDR_10BIT) {
            i2c_enable_10bit_address_mode(base, 1);
        } else {
            i2c_enable_10bit_address_mode(base, 0);
        }

        /*
         * TODO: Another flags: I2C_FLAG_NO_START, I2C_FLAG_STOP, I2C_FLAG_READ_NO_ACK
         */
        if (msgs[i].flags & I2C_FLAG_READ) {
            if (i2c_master_read(base, msgs[i].addr, msgs[i].buf, msgs[i].len) != status_success) {
                HDF_LOGI("i2c read: Failed!!!\n");
                return HDF_FAILURE;
            }
        } else {
            if (i2c_master_write(base, msgs[i].addr, msgs[i].buf, msgs[i].len) != status_success) {
                HDF_LOGI("i2c write: Failed!!!\n");
                return HDF_FAILURE;
            }
        }
    }

    return 0;
}

static struct I2cMethod hpmI2cMethod = {
    .transfer = hpmTransfer,
};


static int32_t I2cDriverBind(struct HdfDeviceObject *device)
{
    int32_t ret = HDF_SUCCESS;

    HDF_LOGI("Bind: successed\n");
    return ret;
}

static int32_t I2cDriverInit(struct HdfDeviceObject *device)
{
    int32_t ret = HDF_SUCCESS;

    struct I2cCntlr *cntlr = (struct I2cCntlr *)OsalMemCalloc(
                                        sizeof(struct I2cCntlr) + sizeof(struct HPMI2cDevice));
    if (cntlr == NULL) {
        ret = HDF_FAILURE;
        HDF_LOGE("Init: cntlr alloc Failed!!!\n");
        return ret;
    }

    struct HPMI2cDevice *hpmI2cDev = (struct HPMI2cDevice *)((char *)cntlr + sizeof(struct I2cCntlr));
    cntlr->priv = hpmI2cDev;
    device->priv = cntlr;

    struct DeviceResourceIface *dri = DeviceResourceGetIfaceInstance(HDF_CONFIG_SOURCE);
    if (dri == NULL) {
        ret = HDF_FAILURE;
        HDF_LOGE("Init: get DeviceResourceIface Failed!!!\n");
        goto ERROR1;
    }

    dri->GetUint32(device->property, "id", &hpmI2cDev->id, 0);
    dri->GetUint32(device->property, "base", &hpmI2cDev->base, 0);
    dri->GetUint32(device->property, "clk_freq", &hpmI2cDev->clkFreq, 0);
    HDF_LOGI("Init: hpmI2cDev->id: %u\n", hpmI2cDev->id);
    HDF_LOGI("Init: hpmI2cDev->base: 0x%X\n", hpmI2cDev->base);
    HDF_LOGI("Init: hpmI2cDev->clkFreq: %u\n", hpmI2cDev->clkFreq);

    cntlr->busId = hpmI2cDev->id;
    cntlr->ops = &hpmI2cMethod;
    ret = I2cCntlrAdd(cntlr);
    if (ret) {
        HDF_LOGE("Init: I2cCntlrAdd Failed!!!\n");
        ret = HDF_FAILURE;
        goto ERROR1;
    }

    I2C_Type *base = (I2C_Type *)hpmI2cDev->base;
    i2c_config_t i2cConfig;
    i2cConfig.i2c_mode = i2c_mode_normal;
    i2cConfig.is_10bit_addressing = 0;
    i2c_init_master(base, hpmI2cDev->clkFreq, &i2cConfig);

    return ret;
ERROR1:
    OsalMemFree(cntlr);
    return ret;
}

static void I2cDriverRelease(struct HdfDeviceObject *device)
{
    if (device == NULL) {
        return;
    }

    struct I2cCntlr *cntlr = (struct I2cCntlr *)device->priv;
    if (cntlr == NULL) {
        HDF_LOGE("Release: cntlr null\n");
        return;
    }

    I2cCntlrRemove(cntlr);
    OsalMemFree(cntlr);

    HDF_LOGI("Release");
    return;
}

struct HdfDriverEntry g_i2cDriverEntry = {
    .moduleVersion = 1,
    .moduleName = "HPMICRO_I2C_MODULE_HDF",
    .Bind = I2cDriverBind,
    .Init = I2cDriverInit,
    .Release = I2cDriverRelease,
};

HDF_INIT(g_i2cDriverEntry);