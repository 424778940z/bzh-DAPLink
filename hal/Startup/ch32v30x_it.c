/********************************** (C) COPYRIGHT *******************************
 * File Name          : ch32v30x_it.c
 * Author             : WCH
 * Version            : V1.0.0
 * Date               : 2024/03/06
 * Description        : Main Interrupt Service Routines.
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/

#include <stdint.h>

#include "ch32v30x.h"
#include "ch32v30x_it.h"

#include "debug.h"

void NMI_Handler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void HardFault_Handler(void) __attribute__((interrupt("WCH-Interrupt-fast")));

extern char _susrstack[];
extern char _eusrstack[];
extern char _stext[] __attribute__((weak));
extern char _etext[] __attribute__((weak));

// Decode RV32 mcause to a short symbolic tag. Bit 31 distinguishes
// asynchronous interrupts from synchronous exceptions; the low bits
// then index into two separate tables (priv-spec 1.12).
static const char* fault_decode_mcause(uint32_t mcause)
{
    if ( mcause & 0x80000000u )
    {
        switch ( mcause & 0x7FFFFFFFu )
        {
        case 3:
            return "M-soft IRQ";
        case 7:
            return "M-timer IRQ";
        case 11:
            return "M-ext IRQ";
        default:
            return "Unknown IRQ";
        }
    }
    switch ( mcause )
    {
    case 0:
        return "Instr address misaligned";
    case 1:
        return "Instr access fault";
    case 2:
        return "Illegal instruction";
    case 3:
        return "Breakpoint";
    case 4:
        return "Load address misaligned";
    case 5:
        return "Load access fault";
    case 6:
        return "Store/AMO addr misaligned";
    case 7:
        return "Store/AMO access fault";
    case 8:
        return "ECALL from U-mode";
    case 11:
        return "ECALL from M-mode";
    case 12:
        return "Instruction page fault";
    case 13:
        return "Load page fault";
    case 15:
        return "Store/AMO page fault";
    default:
        return "Unknown exception";
    }
}

// Read the live SP. In a WCH-Interrupt-fast handler, the HPE has already
// pushed x1, x5-x7, x10-x17, x28-x31 onto sp (52 bytes). We capture sp
// AFTER the HPE push so the printed stack window starts at the saved
// caller frame, which is what addr2line / a debugger want.
static inline uint32_t fault_read_sp(void)
{
    uint32_t sp;
    __asm volatile("mv %0, sp" : "=r"(sp));
    return sp;
}

static void fault_dump_stack_window(uint32_t sp, uint32_t words)
{
    uint32_t lo = (uint32_t)(uintptr_t)_susrstack;
    uint32_t hi = (uint32_t)(uintptr_t)_eusrstack;
    if ( sp < lo || sp >= hi )
    {
        debug_printf("  SP outside stack region [0x%08lx..0x%08lx)\r\n", (unsigned long)lo, (unsigned long)hi);
        return;
    }
    uint32_t end = sp + words * 4u;
    if ( end > hi )
    {
        end = hi;
    }
    for ( uint32_t addr = sp; addr + 4u <= end; addr += 16u )
    {
        debug_printf(
            "  0x%08lx: 0x%08lx 0x%08lx 0x%08lx 0x%08lx\r\n", (unsigned long)addr,
            (unsigned long)*(volatile uint32_t*)(uintptr_t)(addr + 0u),
            (unsigned long)*(volatile uint32_t*)(uintptr_t)((addr + 4u < end) ? (addr + 4u) : addr),
            (unsigned long)*(volatile uint32_t*)(uintptr_t)((addr + 8u < end) ? (addr + 8u) : addr),
            (unsigned long)*(volatile uint32_t*)(uintptr_t)((addr + 12u < end) ? (addr + 12u) : addr)
        );
    }
}

/*********************************************************************
 * @fn      NMI_Handler
 *
 * @brief   This function handles NMI exception.
 *
 * @return  none
 */
void NMI_Handler(void)
{
    while ( 1 ) {}
}

// Trapped CSR snapshot. Globals (not stack locals) so the values survive any
// re-entry into the handler and can be inspected from gdb at the halt loop:
//
//   (gdb) p/x g_fault_mcause
//   (gdb) p/x g_fault_mepc
//   (gdb) p/x g_fault_mtval
//
// Marked `used` so -Wl,--gc-sections cannot drop them in Release.
__attribute__((used)) volatile uint32_t g_fault_mepc;
__attribute__((used)) volatile uint32_t g_fault_mcause;
__attribute__((used)) volatile uint32_t g_fault_mtval;
__attribute__((used)) volatile uint32_t g_fault_mstatus;
__attribute__((used)) volatile uint32_t g_fault_mtvec;
__attribute__((used)) volatile uint32_t g_fault_sp;
__attribute__((used)) volatile uint32_t g_fault_reentry_count;

/*********************************************************************
 * @fn      HardFault_Handler
 *
 * @brief   Hard Fault handler. Captures RV32 trap CSRs into globals so
 *          they survive even if the print path itself faults, then
 *          attempts a one-shot debug_printf dump (gated by a re-entry
 *          counter so we do not recurse if the fault originated inside
 *          the print path), and halts at while(1) for the debugger.
 *
 * @return  none
 */
void HardFault_Handler(void)
{
    uint32_t this_count = ++g_fault_reentry_count;

    // Capture CSRs only on the first entry so a re-faulting print path
    // cannot clobber the diagnostic of the original fault. After this,
    // gdb can always read g_fault_* to see what triggered the very first HF.
    if ( this_count == 1 )
    {
        g_fault_mepc = __get_MEPC();
        g_fault_mcause = __get_MCAUSE();
        g_fault_mtval = __get_MTVAL();
        g_fault_mstatus = __get_MSTATUS();
        g_fault_mtvec = __get_MTVEC();
        g_fault_sp = fault_read_sp();

        debug_printf("\r\n===== HARDFAULT =====\r\n");
        debug_printf("  cause   : 0x%08lx (%s)\r\n", (unsigned long)g_fault_mcause, fault_decode_mcause(g_fault_mcause));
        debug_printf("  mepc    : 0x%08lx\r\n", (unsigned long)g_fault_mepc);
        debug_printf("  mtval   : 0x%08lx\r\n", (unsigned long)g_fault_mtval);
        debug_printf("  mstatus : 0x%08lx\r\n", (unsigned long)g_fault_mstatus);
        debug_printf("  mtvec   : 0x%08lx\r\n", (unsigned long)g_fault_mtvec);
        debug_printf("  sp      : 0x%08lx\r\n", (unsigned long)g_fault_sp);
        debug_printf("  stack   : [0x%08lx .. 0x%08lx)\r\n",
                     (unsigned long)(uintptr_t)_susrstack, (unsigned long)(uintptr_t)_eusrstack);
        debug_printf("stack window (post-HPE):\r\n");
        fault_dump_stack_window(g_fault_sp, 32u);
        debug_printf("addr2line -e <fw>.elf -afpi 0x%08lx\r\n", (unsigned long)g_fault_mepc);
        debug_printf("===== END HARDFAULT =====\r\n");
    }

    while ( 1 ) {}
}
