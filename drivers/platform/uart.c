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
#include "uart_if.h"
#include "uart_core.h"
#include "hpm_uart_drv.h"
#include <los_interrupt.h>

#define HDF_LOG_TAG HPMICRO_UART_HDF
#define HPM_QUEUE_SIZE 64

struct UartQueue {
    uint8_t buf[HPM_QUEUE_SIZE];
    uint16_t wr;
    uint16_t rd;
};

struct HPMUartDevice {
    struct OsalSem rxSem;
    struct UartQueue rxQueue;
    uint32_t id;
    uint32_t base;
    uint32_t irq;
    uint32_t clkFreq;
    struct UartAttribute attribute;
    uint32_t baudRate;
    uint8_t isRxBlock;
    uint8_t isRxDmaEn;
    uint8_t isTxDmaEn;
};

static uint32_t queue_get_cnt(struct UartQueue *q) 
{
    return (q->wr + HPM_QUEUE_SIZE - q->rd) % HPM_QUEUE_SIZE;
}

static int queue_push(struct UartQueue *q, uint8_t *data, uint16_t len) 
{
    uint32_t save = LOS_IntLock();
    for (int i = 0; i < len; i++) {
        q->buf[q->wr] = data[i];
        q->wr++;
        q->wr %= HPM_QUEUE_SIZE;

        /* overflow */
        if (q->wr == q->rd) {
            q->rd++;
            q->wr %= HPM_QUEUE_SIZE;
        }
    }
    LOS_IntRestore(save);

    return 0;
}

static uint16_t queue_pop(struct UartQueue *q, uint8_t *data, uint16_t len) 
{
    uint32_t save = LOS_IntLock();
    uint32_t max = queue_get_cnt(q);

    max = max > len? len : max;

    for (int i = 0; i < max; i++) {
        data[i] = q->buf[q->rd];
        q->rd++;
        q->rd %= HPM_QUEUE_SIZE;
    }

    LOS_IntRestore(save);
    return max;
}

static __attribute__((section(".interrupt.text"))) VOID HpmUartIsr(VOID *parm)
{
    struct UartHost *host = (struct UartHost *)parm;
    struct HPMUartDevice *hpmUartDev = (struct HPMUartDevice *)host->priv;
    UART_Type *base = (UART_Type *)hpmUartDev->base;

    if (uart_get_irq_id(base) & uart_intr_id_rx_data_avail) {
        uint8_t c;
        if (status_success == uart_receive_byte(base, &c)) {
            queue_push(&hpmUartDev->rxQueue, &c, 1);
            OsalSemPost(&hpmUartDev->rxSem);
        }
    }
}

static void UartConfig(struct UartHost *host)
{
    struct HPMUartDevice *hpmUartDev = (struct HPMUartDevice *)host->priv;
    UART_Type *base = (UART_Type *)hpmUartDev->base;
    uart_disable_irq(base, uart_intr_rx_data_avail_or_timeout);

    uart_config_t config = {0};
   
    uart_default_config(base, &config);
    config.src_freq_in_hz = hpmUartDev->clkFreq; /* IP freq in HZ*/
    config.baudrate = hpmUartDev->baudRate;

    if (hpmUartDev->attribute.dataBits == UART_ATTR_DATABIT_7) {
        config.word_length = word_length_7_bits;
    } else if (hpmUartDev->attribute.dataBits == UART_ATTR_DATABIT_6) {
        config.word_length = word_length_6_bits;
    } else if (hpmUartDev->attribute.dataBits == UART_ATTR_DATABIT_5) {
        config.word_length = word_length_5_bits;
    } else {
        config.word_length = word_length_8_bits;
    }

    if (hpmUartDev->attribute.stopBits == UART_ATTR_STOPBIT_1P5) {
        config.num_of_stop_bits = stop_bits_1_5;
    } else if (hpmUartDev->attribute.stopBits == UART_ATTR_STOPBIT_2) {
        config.num_of_stop_bits = stop_bits_2;
    } else {
        config.num_of_stop_bits = stop_bits_1;
    }

    if (hpmUartDev->attribute.parity == UART_ATTR_PARITY_ODD) {
        config.parity = parity_odd;
    } else if (hpmUartDev->attribute.parity == UART_ATTR_PARITY_EVEN) {
        config.parity = parity_even;
    } else if (hpmUartDev->attribute.parity == UART_ATTR_PARITY_MARK) {
        config.parity = parity_always_1;
    } else if (hpmUartDev->attribute.parity == UART_ATTR_PARITY_SPACE) {
        config.parity = parity_always_0;
    } else {
        config.parity = parity_none;
    }

    uart_init(base, &config);  
    uart_enable_irq(base, uart_intr_rx_data_avail_or_timeout);
}

static int32_t Init(struct UartHost *host)
{
    struct HPMUartDevice *hpmUartDev = (struct HPMUartDevice *)host->priv;

    hpmUartDev->baudRate = 115200;
    hpmUartDev->isRxBlock = 1;
    hpmUartDev->isRxDmaEn = 0;
    hpmUartDev->isTxDmaEn = 0;
    hpmUartDev->rxQueue.rd = 0;
    hpmUartDev->rxQueue.wr = 0;
    hpmUartDev->attribute.dataBits = UART_ATTR_DATABIT_8;
    hpmUartDev->attribute.parity = UART_ATTR_PARITY_NONE;
    hpmUartDev->attribute.stopBits = UART_ATTR_STOPBIT_1;
    hpmUartDev->attribute.rts = UART_ATTR_RTS_DIS;
    hpmUartDev->attribute.cts = UART_ATTR_CTS_DIS;

    UartConfig(host);

    return 0;
}

static int32_t Deinit(struct UartHost *host)
{
    struct HPMUartDevice *hpmUartDev = (struct HPMUartDevice *)host->priv;
    UART_Type *base = (UART_Type *)hpmUartDev->base;
    uart_disable_irq(base, uart_intr_rx_data_avail_or_timeout);

    return 0;
}

static int32_t Read(struct UartHost *host, uint8_t *data, uint32_t size)
{
    struct HPMUartDevice *hpmUartDev = (struct HPMUartDevice *)host->priv;

    if (hpmUartDev->isRxBlock) {
        while (queue_get_cnt(&hpmUartDev->rxQueue) < size) {
            OsalSemWait(&hpmUartDev->rxSem, 10000);
        }
    }
    
    return queue_pop(&hpmUartDev->rxQueue, data, size);
}

static int32_t Write(struct UartHost *host, uint8_t *data, uint32_t size)
{
    struct HPMUartDevice *hpmUartDev = (struct HPMUartDevice *)host->priv;
    UART_Type *base = (UART_Type *)hpmUartDev->base;

    uart_send_data(base, data, size);

    return size;
}

static int32_t GetBaud(struct UartHost *host, uint32_t *baudRate)
{
    struct HPMUartDevice *hpmUartDev = (struct HPMUartDevice *)host->priv;
    *baudRate = hpmUartDev->baudRate;

    return 0;
}

static int32_t SetBaud(struct UartHost *host, uint32_t baudRate)
{
    struct HPMUartDevice *hpmUartDev = (struct HPMUartDevice *)host->priv;
    hpmUartDev->baudRate = baudRate;
    UartConfig(host);

    return 0;
}

static int32_t GetAttribute(struct UartHost *host, struct UartAttribute *attribute)
{
    struct HPMUartDevice *hpmUartDev = (struct HPMUartDevice *)host->priv;
    *attribute = hpmUartDev->attribute;

    return 0;
}

static int32_t SetAttribute(struct UartHost *host, struct UartAttribute *attribute)
{
    struct HPMUartDevice *hpmUartDev = (struct HPMUartDevice *)host->priv;
    hpmUartDev->attribute = *attribute;
    UartConfig(host);

    return 0;
}

static int32_t SetTransMode(struct UartHost *host, enum UartTransMode mode)
{
    struct HPMUartDevice *hpmUartDev = (struct HPMUartDevice *)host->priv;
 
    if (mode == UART_MODE_RD_BLOCK) {
        hpmUartDev->isRxBlock = 1;
    } else if (mode == UART_MODE_RD_NONBLOCK) {
        hpmUartDev->isRxBlock = 0;
    } else if (mode == UART_MODE_DMA_RX_EN) {
        hpmUartDev->isRxDmaEn = 1;
    } else if (mode == UART_MODE_DMA_RX_DIS) {
        hpmUartDev->isRxDmaEn = 0;
    } else if (mode == UART_MODE_DMA_TX_EN) {
        hpmUartDev->isTxDmaEn = 1;
    } else if (mode == UART_MODE_DMA_TX_DIS) {
        hpmUartDev->isTxDmaEn = 0;
    }

    UartConfig(host);

    return 0;
}

static struct UartHostMethod uartHostMethod = {
    .Init = Init,
    .Deinit = Deinit,
    .Read = Read,
    .Write = Write,
    .GetBaud = GetBaud,
    .SetBaud = SetBaud,
    .GetAttribute = GetAttribute,
    .SetAttribute = SetAttribute,
    .SetTransMode = SetTransMode,
};

static int32_t UartDriverBind(struct HdfDeviceObject *device)
{
    int32_t ret = HDF_SUCCESS;

    struct UartHost *host = UartHostCreate(device);
    if (host == NULL) {
        ret = HDF_FAILURE;
        HDF_LOGE("Bind: UartHostCreate null\n");
        return ret;
    }

    HDF_LOGI("Bind: successed\n");
    return ret;
}

static int32_t UartDriverInit(struct HdfDeviceObject *device)
{
    int32_t ret = HDF_SUCCESS;

    struct UartHost *host = UartHostFromDevice(device);
    if (host == NULL) {
        ret = HDF_FAILURE;
        HDF_LOGE("Init: UartHostFromDevice null\n");
        return ret;
    }

    struct HPMUartDevice *hpmUartDev = (struct HPMUartDevice *)OsalMemCalloc(
                                        sizeof(struct HPMUartDevice));
    if (hpmUartDev == NULL) {
        ret = HDF_ERR_MALLOC_FAIL;
        HDF_LOGE("Init: HPMUartDevice malloc Failed!!!\n");
        goto ERROR1;
    }

    OsalSemInit(&hpmUartDev->rxSem, 0);

    struct DeviceResourceIface *dri = DeviceResourceGetIfaceInstance(HDF_CONFIG_SOURCE);
    if (dri == NULL) {
        ret = HDF_FAILURE;
        HDF_LOGE("Init: get DeviceResourceIface Failed!!!\n");
        goto ERROR2;
    }

    dri->GetUint32(device->property, "id", &hpmUartDev->id, 0);
    dri->GetUint32(device->property, "base", &hpmUartDev->base, 0);
    dri->GetUint32(device->property, "irq_num", &hpmUartDev->irq, 0);
    dri->GetUint32(device->property, "clk_freq", &hpmUartDev->clkFreq, 0);
    HDF_LOGI("Init: hpmUartDev->id: %u\n", hpmUartDev->id);
    HDF_LOGI("Init: hpmUartDev->base: 0x%X\n", hpmUartDev->base);
    HDF_LOGI("Init: hpmUartDev->irq: %u\n", hpmUartDev->irq);
    HDF_LOGI("Init: hpmUartDev->clkFreq: %u\n", hpmUartDev->clkFreq);

    host->num = hpmUartDev->id;
    host->priv = hpmUartDev;
    host->method = &uartHostMethod;

    HwiIrqParam irqParam;
    irqParam.pDevId = host;
    LOS_HwiCreate(HPM2LITEOS_IRQ(hpmUartDev->irq), 1, 0, (HWI_PROC_FUNC)HpmUartIsr, &irqParam);
    LOS_HwiEnable(HPM2LITEOS_IRQ(hpmUartDev->irq));

    return ret;

ERROR2:
    OsalSemDestroy(&hpmUartDev->rxSem);
    OsalMemFree(hpmUartDev);
ERROR1:
    return ret;
}

static void UartDriverRelease(struct HdfDeviceObject *device)
{
    if (device == NULL) {
        return;
    }

    struct UartHost *host = UartHostFromDevice(device);
    if (host == NULL) {
        HDF_LOGE("Release: UartHostFromDevice null\n");
        return;
    }

    struct HPMUartDevice *hpmUartDev = (struct HPMUartDevice *)host->priv;

    if (hpmUartDev) {
        OsalSemDestroy(&hpmUartDev->rxSem);
        OsalMemFree(hpmUartDev);
    }
    
    UartHostDestroy(host);

    HDF_LOGI("Release");
    return;
}


struct HdfDriverEntry g_uartDriverEntry = {
    .moduleVersion = 1,
    .moduleName = "HPMICRO_UART_MODULE_HDF",
    .Bind = UartDriverBind,
    .Init = UartDriverInit,
    .Release = UartDriverRelease,
};

HDF_INIT(g_uartDriverEntry);