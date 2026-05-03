#include "systick.h"

#include <assert.h>

#include "stm32f4xx_hal.h"

volatile uint64_t SysTick_count = 0;

/* The vendor `Reset_Handler -> __libc_init_array -> main` path doesn't enable
 * SysTick; the STM32 HAL pulls in its own SysTick_Handler via HAL_IncTick().
 * We override the handler here so SysTick drives our 64-bit tick AND HAL_Delay
 * keeps working (HAL_GetTick() reads the value HAL_IncTick() updates). */
void SysTick_Handler(void)
{
    SysTick_count++;
    HAL_IncTick();
}

void systick_init_config(uint64_t ticks)
{
    /* SysTick_Config sets LOAD = ticks-1, VAL = 0, and CTRL = CLKSOURCE|TICKINT|ENABLE.
     * It returns non-zero if the reload value doesn't fit in 24 bits. */
    if ( SysTick_Config((uint32_t)ticks) != 0U )
    {
        while ( 1 )
        {
        }
    }
    HAL_NVIC_SetPriority(SysTick_IRQn, 15U, 0U);
}

void systick_init(void)
{
    assert((SystemCoreClock >= 1000U) && ((SystemCoreClock % 1000U) == 0U)); // ensure integer ms step with current clock
    const uint64_t ticks_per_ms = (uint64_t)(SystemCoreClock / 1000U);
    systick_init_config(ticks_per_ms);
}

void systick_deinit(void)
{
    HAL_NVIC_DisableIRQ(SysTick_IRQn);
    SysTick->CTRL = 0;
}

uint64_t systick_get_tick(void)
{
    uint64_t snapshot;
    do
    {
        snapshot = SysTick_count;
    }
    while ( snapshot != SysTick_count ); // guard against 64-bit tear between reads
    return snapshot;
}

void systick_delay_ms(uint32_t delay_ms)
{
    const uint64_t start = systick_get_tick();
    while ( (systick_get_tick() - start) < delay_ms )
    {
        __NOP();
    }
}
