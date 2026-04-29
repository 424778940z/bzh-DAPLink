#include "systick.h"

#include <assert.h>
#include "ch32v30x.h"

volatile uint64_t SysTick_count = 0;

void SysTick_Handler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void SysTick_Handler(void)
{
    SysTick->SR = 0; // clear CNTIF
    SysTick_count++;
}

void systick_init_config(uint64_t ticks)
{
    SysTick->CTLR = 0;
    SysTick->SR = 0;     // clear CNTIF before enabling
    SysTick->CMP = ticks;
    SysTick->CNT = 0;
    SysTick->CTLR = 0xF; // HCLK, auto-reload, interrupt enable, counter enable

    NVIC_SetPriority(SysTick_IRQn, 15);
    NVIC_EnableIRQ(SysTick_IRQn);
}

void systick_init(void)
{
    assert((SystemCoreClock >= 1000U) && ((SystemCoreClock % 1000U) == 0U)); // ensure integer ms step with current clock
    const uint64_t ticks_per_ms = (uint64_t)(SystemCoreClock / 1000U) - 1U;
    systick_init_config(ticks_per_ms);
}

void systick_deinit(void)
{
    NVIC_DisableIRQ(SysTick_IRQn);
    SysTick->CTLR = 0;
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
