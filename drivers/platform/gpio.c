/*
 * Copyright (c) 2022 HPMicro.
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
#include "gpio_if.h"
#include "gpio_core.h"
#include <hpm_gpio_drv.h>
#include <los_interrupt.h>

#define HDF_LOG_TAG HPMICRO_GPIO_HDF

struct HPMGpioDevice {
    uint32_t base;
    const char *name;
    uint32_t port_num;
    uint32_t irq_num;
};


static int32_t Write(struct GpioCntlr *ctl, uint16_t local, uint16_t val)
{
    struct HPMGpioDevice *hpmGpioDev = (struct HPMGpioDevice *)ctl->priv;
    GPIO_Type *base = (GPIO_Type *)hpmGpioDev->base;
    uint32_t port_num = hpmGpioDev->port_num;

    gpio_write_pin(base, port_num, local, (val == GPIO_VAL_LOW) ? 0 : 1);

    return HDF_SUCCESS;
}

static int32_t Read(struct GpioCntlr *ctl, uint16_t local, uint16_t *val)
{
    struct HPMGpioDevice *hpmGpioDev = (struct HPMGpioDevice *)ctl->priv;
    GPIO_Type *base = (GPIO_Type *)hpmGpioDev->base;
    uint32_t port_num = hpmGpioDev->port_num;

    if (gpio_read_pin(base, port_num, local)) {
        *val = GPIO_VAL_HIGH;
    } else {
        *val = GPIO_VAL_LOW;
    }

    return HDF_SUCCESS;
}

int32_t SetDir(struct GpioCntlr *ctl, uint16_t local, uint16_t dir)
{
    struct HPMGpioDevice *hpmGpioDev = (struct HPMGpioDevice *)ctl->priv;
    GPIO_Type *base = (GPIO_Type *)hpmGpioDev->base;
    uint32_t port_num = hpmGpioDev->port_num;

    if (dir == GPIO_DIR_IN) {
        gpio_set_pin_input(base, port_num, local);
    } else {
        gpio_set_pin_output(base, port_num, local);
    }
    HDF_LOGD("GPIO: [%s-%hu] SetDir [%hu]", hpmGpioDev->name, local, dir);
    return HDF_SUCCESS;
}

int32_t GetDir(struct GpioCntlr *ctl, uint16_t local, uint16_t *dir)
{
    return HDF_SUCCESS;
}

static int32_t SetIrq(struct GpioCntlr *ctl, uint16_t local, uint16_t mode)
{
    struct HPMGpioDevice *hpmGpioDev = (struct HPMGpioDevice *)ctl->priv;
    GPIO_Type *base = (GPIO_Type *)hpmGpioDev->base;
    uint32_t port_num = hpmGpioDev->port_num;

    if (mode & GPIO_IRQ_TRIGGER_RISING) {
        gpio_config_pin_interrupt(base, port_num, local, gpio_interrupt_trigger_edge_rising);
    } else if (mode & GPIO_IRQ_TRIGGER_FALLING) {
        gpio_config_pin_interrupt(base, port_num, local, gpio_interrupt_trigger_edge_falling);
    } else if (mode & GPIO_IRQ_TRIGGER_HIGH) {
        gpio_config_pin_interrupt(base, port_num, local, gpio_interrupt_trigger_level_high);
    } else if (mode & GPIO_IRQ_TRIGGER_LOW) {
        gpio_config_pin_interrupt(base, port_num, local, gpio_interrupt_trigger_level_low);
    } else {

    }

    return HDF_SUCCESS;
}

static int32_t UnsetIrq(struct GpioCntlr *ctl, uint16_t local)
{
    return HDF_SUCCESS;
}

static int32_t EnableIrq(struct GpioCntlr *ctl, uint16_t local)
{
    struct HPMGpioDevice *hpmGpioDev = (struct HPMGpioDevice *)ctl->priv;
    GPIO_Type *base = (GPIO_Type *)hpmGpioDev->base;
    uint32_t port_num = hpmGpioDev->port_num;

    gpio_enable_pin_interrupt(base, port_num, local);

    return HDF_SUCCESS;
}

static int32_t DisableIrq(struct GpioCntlr *ctl, uint16_t local)
{
    struct HPMGpioDevice *hpmGpioDev = (struct HPMGpioDevice *)ctl->priv;
    GPIO_Type *base = (GPIO_Type *)hpmGpioDev->base;
    uint32_t port_num = hpmGpioDev->port_num;

    gpio_disable_pin_interrupt(base, port_num, local);

    return HDF_SUCCESS;
}

static struct GpioMethod gpioOps = {
    .write = Write,
    .read = Read,
    .setDir = SetDir,
    .getDir = GetDir,
    .setIrq = SetIrq,
    .unsetIrq = UnsetIrq,
    .enableIrq = EnableIrq,
    .disableIrq = DisableIrq,
};

static int32_t GpioHcsResurceInit(struct GpioCntlr *ctl)
{
    int ret = HDF_SUCCESS;
    struct HdfDeviceObject *device = ctl->device.hdfDev;
    struct HPMGpioDevice *hpmGpioDev = (struct HPMGpioDevice *)ctl->priv;

    struct DeviceResourceIface *dri = DeviceResourceGetIfaceInstance(HDF_CONFIG_SOURCE);
    if (dri == NULL) {
        HDF_LOGE("get DeviceResourceIface Failed!!!");
        return HDF_FAILURE;
    }

    dri->GetUint32(device->property, "base", &hpmGpioDev->base, 0);
    dri->GetString(device->property, "name", &hpmGpioDev->name, NULL);
    dri->GetUint32(device->property, "port_num", &hpmGpioDev->port_num, 0);
    dri->GetUint32(device->property, "irq_num", &hpmGpioDev->irq_num, 0);
    dri->GetUint16(device->property, "count", &ctl->count, 0);
    ctl->start = ctl->count * hpmGpioDev->port_num;

    HDF_LOGI("GPIO: base: 0x%X", hpmGpioDev->base);
    HDF_LOGI("GPIO: name: %s", hpmGpioDev->name);
    HDF_LOGI("GPIO: port_num: %u", hpmGpioDev->port_num);
    HDF_LOGI("GPIO: irq_num: %u", hpmGpioDev->irq_num);
    HDF_LOGI("GPIO: count: %u", ctl->count);
    HDF_LOGI("GPIO: start: %u", ctl->start);

    return ret;
}

static __attribute__((section(".interrupt.text"))) VOID hpm_gpio_isr(VOID *parm)
{
    struct GpioCntlr *ctl = (struct GpioCntlr *)parm;
    struct HPMGpioDevice *hpmGpioDev = (struct HPMGpioDevice *)ctl->priv;
    GPIO_Type *base = (GPIO_Type *)hpmGpioDev->base;
    uint32_t port_num = hpmGpioDev->port_num;

    for (uint32_t local = 0; local < ctl->count; local++) {
        if (gpio_check_clear_interrupt_flag(base, port_num, local)) {
            GpioCntlrIrqCallback(ctl, local);
        }
    }
}

static int32_t GpioDriverBind(struct HdfDeviceObject *device)
{
    HDF_LOGI("Bind: It's not to be run");
    return HDF_SUCCESS;
}

static int32_t GpioDriverInit(struct HdfDeviceObject *device)
{
    int32_t ret = HDF_SUCCESS;
    struct HPMGpioDevice *hpmGpioDev;
    struct GpioCntlr *ctl = (struct GpioCntlr *)OsalMemCalloc(sizeof(struct GpioCntlr) + sizeof(struct HPMGpioDevice));
    ctl->priv = (char *)ctl + sizeof(struct GpioCntlr);
    hpmGpioDev = (struct HPMGpioDevice *)ctl->priv;
    ctl->ops = &gpioOps;

    PlatformDeviceSetHdfDev(&ctl->device, device);

    ret = GpioHcsResurceInit(ctl);
    if (ret) {
        HDF_LOGE("Init: GpioHcsResurceInit Failed!!!\n");
        goto ERROR;
    }

    ret = GpioCntlrAdd(ctl);
    if (ret) {
        HDF_LOGE("Init: GpioCntlrAdd Fualed, name: %s!!!\n", hpmGpioDev->name);
        goto ERROR;
    }

    HwiIrqParam irqParam;
    irqParam.pDevId = ctl;
    LOS_HwiCreate(HPM2LITEOS_IRQ(hpmGpioDev->irq_num), 1, 0, (HWI_PROC_FUNC)hpm_gpio_isr, &irqParam);
    LOS_HwiEnable(HPM2LITEOS_IRQ(hpmGpioDev->irq_num));

    HDF_LOGI("Init");

    return ret;

ERROR:
    OsalMemFree(ctl);
    return ret;
}

static void GpioDriverRelease(struct HdfDeviceObject *device)
{
    if (device == NULL) {
        return;
    }

    struct GpioCntlr *ctl = GpioCntlrFromHdfDev(device);
    if (ctl) {
        OsalMemFree(ctl);
    }
    HDF_LOGI("Release");
    return;
}


struct HdfDriverEntry g_gpioDriverEntry = {
    .moduleVersion = 1,
    .moduleName = "HPMICRO_GPIO_MODULE_HDF",
    .Bind = GpioDriverBind,
    .Init = GpioDriverInit,
    .Release = GpioDriverRelease,
};

HDF_INIT(g_gpioDriverEntry);
