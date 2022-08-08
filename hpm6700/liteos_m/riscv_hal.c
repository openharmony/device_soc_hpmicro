/*
 * Copyright (c) 2020-2021 Huawei Device Co., Ltd.
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

#include "riscv_hal.h"
#include "los_debug.h"
#include "soc.h"
#include "los_reg.h"
#include "los_arch_interrupt.h"

#include "hpm_soc.h"
#include "hpm_interrupt.h"
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

VOID HalIrqDisable(UINT32 vector)
{
    if (vector <= RISCV_SYS_MAX_IRQ) {
        CLEAR_CSR(mie, 1 << vector);
    } else {
        intc_m_disable_irq(LITEOS2HPM_IRQ(vector));
    }
}

VOID HalIrqEnable(UINT32 vector)
{
    if (vector <= RISCV_SYS_MAX_IRQ) {
        SET_CSR(mie, 1 << vector);
    } else {
        intc_m_enable_irq(LITEOS2HPM_IRQ(vector));
    }
}

VOID HalSetLocalInterPri(UINT32 interPriNum, UINT16 prior)
{
    intc_set_irq_priority(LITEOS2HPM_IRQ(interPriNum), prior);
}

BOOL HalBackTraceFpCheck(UINT32 value)
{
    if (value >= (UINT32)(UINTPTR)(&__bss_end)) {
        return TRUE;
    }

    if ((value >= (UINT32)(UINTPTR)(&__start_and_irq_stack_top)) && (value < (UINT32)(UINTPTR)(&__except_stack_top))) {
        return TRUE;
    }

    return FALSE;
}

extern UINT32 g_hwiFormCnt[];
STATIC VOID OsMachineExternalInterrupt(VOID *arg)
{
    UINT32 irqNum = intc_m_claim_irq();
    UINT32 liteosIrqNum = HPM2LITEOS_IRQ(irqNum);
    
    if ((irqNum >= OS_RISCV_CUSTOM_IRQ_VECTOR_CNT) || (irqNum == 0)) {
        HalHwiDefaultHandler((VOID *)irqNum);
    }

    g_hwiForm[liteosIrqNum].pfnHook(g_hwiForm[liteosIrqNum].uwParam);
    ++g_hwiFormCnt[liteosIrqNum];

    intc_m_complete_irq(irqNum);
}

VOID HalPlicInit(VOID)
{
    UINT32 ret;
    ret = LOS_HwiCreate(RISCV_MACH_EXT_IRQ, 0x1, 0, OsMachineExternalInterrupt, 0);
    if (ret != LOS_OK) {
        PRINT_ERR("Create machine external failed! ret : 0x%x\n", ret);
    }

    HalIrqEnable(RISCV_MACH_EXT_IRQ);
}



#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
