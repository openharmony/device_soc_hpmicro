/*
 * Copyright (c) 2022 HPMicro
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this list of
 *    conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice, this list
 *    of conditions and the following disclaimer in the documentation and/or other materials
 *    provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors may be used
 *    to endorse or promote products derived from this software without specific prior written
 *    permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdarg.h>
#include <stdio.h>
#include "securec.h"
#include "uart.h"
#include "los_debug.h"
#include "los_interrupt.h"

static void dputs(char const *s, int (*pFputc)(int n, void *file), void *file)
{
    unsigned int intSave;

    intSave = LOS_IntLock();
    while (*s) {
        pFputc(*s++, file);
    }
    LOS_IntRestore(intSave);
}

int __wrap_printf(char const  *fmt, ...)
{
    char logBuf[1024];
    va_list sap;
    va_start(sap, fmt);
    int len = vsnprintf_s(logBuf, sizeof(logBuf), sizeof(logBuf) - 1, fmt, sap);   
    va_end(sap);
    if (len > 0) {
        dputs(logBuf, UartPutc, 0);
    } else {
        dputs("printf error!\n", UartPutc, 0);
    }
    return len;
}

/* enable hilog output in LOSCFG_BASE_CORE_HILOG mode */
int HiLogWriteInternal(const char *buffer, size_t bufLen)
{
    const int BUFF_TAIL_LEN = 2;

    if (!buffer) {
        return -1;
    }

    // because it's called as HiLogWriteInternal(buf, strlen(buf) + 1)
    if (bufLen < BUFF_TAIL_LEN) {
        return 0;
    }

    if (buffer[bufLen - BUFF_TAIL_LEN] != '\n') {
        printf("%s\n", buffer);
    } else {
        dputs(buffer, UartPutc, 0);
    }
    return 0;
}
