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

#include "los_tick.h"
#include "los_task.h"
#include "los_config.h"
#include "los_interrupt.h"
#include "los_debug.h"
#include "uart.h"
#include "board.h"
#include "hpm_mchtmr_drv.h"
#include <ohos_init.h>
#include "hiview_log.h"
#include "los_debug.h"
#if 0
#include "ohos_mem_pool.h"

void *OhosMalloc(MemType type, uint32 size)
{
    if (size == 0) {
        return NULL;
    }
    return malloc(size);
}

void OhosFree(void *ptr)
{
    free(ptr);
}

#endif



boolean HilogProc_Impl(const HiLogContent *hilogContent, uint32 len)
{
    char tempOutStr[128] = {0};
    if (LogContentFmt(tempOutStr, sizeof(tempOutStr), hilogContent) > 0) {
        printf(tempOutStr);
    }
    return TRUE;
}

extern void __libc_fini_array (void);

/*****************************************************************************
 Function    : main
 Description : Main function entry
 Input       : None
 Output      : None
 Return      : None
 *****************************************************************************/
LITE_OS_SEC_TEXT_INIT INT32 main(VOID)
{
#if 0
    if (atexit(__libc_fini_array) != 0) {
        
    };
    __libc_init_array();
#endif

    UINT32 ret;
    board_init();
    UartInit();
    board_print_banner();
    //board_print_clock_freq();

    printf("OHOS start \n\r");


    ret = LOS_KernelInit();
    if (ret != LOS_OK) {
        PRINT_ERR("LiteOS kernel init failed! ERROR: 0x%x\n", ret);
        goto START_FAILED;
    }

    HalPlicInit();
    Uart0RxIrqRegister();

    OHOS_SystemInit();
    /* register hilog output func for mini */
    HiviewRegisterHilogProc(HilogProc_Impl);

#if (LOSCFG_USE_SHELL == 1)
    ret = LosShellInit();
    if (ret != LOS_OK) {
        printf("LosShellInit failed! ERROR: 0x%x\n", ret);
    }
#endif

    LOS_Start();

START_FAILED:
    while (1) {
        __asm volatile("wfi");
    }
}
