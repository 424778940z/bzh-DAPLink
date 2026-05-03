/**
  * stm32f4xx_it.c - Cortex-M exception handlers.
  *
  * Peripheral IRQ handlers live next to their drivers, not here:
  *   - SysTick_Handler   -> src/systick.c
  *   - OTG_FS_IRQHandler -> src/usb/usb.c
  *   - USART1_IRQHandler -> src/uart.c
  * This file owns only the system-level vectors that don't have a peripheral
  * counterpart.
  */

#include "stm32f4xx_hal.h"

void NMI_Handler(void)
{
    while ( 1 )
    {
    }
}

void HardFault_Handler(void)
{
    while ( 1 )
    {
    }
}

void MemManage_Handler(void)
{
    while ( 1 )
    {
    }
}

void BusFault_Handler(void)
{
    while ( 1 )
    {
    }
}

void UsageFault_Handler(void)
{
    while ( 1 )
    {
    }
}

void SVC_Handler(void)
{
}

void DebugMon_Handler(void)
{
}

void PendSV_Handler(void)
{
}
