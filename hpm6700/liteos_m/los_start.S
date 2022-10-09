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
#include "hpm_csr_regs.h"

    .section .start, "ax"

    .global _start
    .type _start,@function

_start:
    /* Initialize global pointer */
    .option push
    .option norelax
    la gp, __global_pointer$
    .option pop

    /* reset mstatus to 0*/
    csrrw x0, mstatus, x0
#ifdef __riscv_flen
    /* Enable FPU */
    li t0, CSR_MSTATUS_FS_MASK
    csrrs t0, mstatus, t0

    /* Initialize FCSR */
    fscsr zero
#endif

#ifdef INIT_EXT_RAM_FOR_DATA
    la t0, _stack_safe
    mv sp, t0
    call _init_ext_ram
#endif

    /* Initialize stack pointer */
    la t0, _stack
    mv sp, t0

#ifndef NO_CLEANUP_AT_START
    /* clean up */
    call _clean_up
#endif

#ifdef __nds_execit
    /* Initialize EXEC.IT table */
    la t0, _ITB_BASE_
    csrw uitb, t0
#endif

#ifndef CONFIG_FREERTOS 
    #define HANDLER_TRAP irq_handler_trap
#else
    #define HANDLER_TRAP freertos_risc_v_trap_handler
#endif

    /* Initial machine trap-vector Base */
    la t0, HalTrapVector
    csrw mtvec, t0


    /* System reset handler */
    call reset_handler

    /* Infinite loop, if returned accidently */
1:    j 1b

    .weak exit
exit:
1:    j 1b

    .section .isr_vector, "ax"
    .weak nmi_handler
nmi_handler:
1:    j 1b

