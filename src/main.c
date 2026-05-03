#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#include "stm32f4xx_hal.h"
#include "SEGGER_RTT.h"

#include "systick.h"
#include "uart.h"
#include "usb/usb.h"

#include "DAP_config.h"
#include "DAP.h"

/* The on-board PC13 LED is owned by the CMSIS-DAP layer (DAP_Setup +
 * LED_CONNECTED_OUT / LED_RUNNING_OUT in DAP_config.h). It blinks during
 * actual probe traffic; firmware liveness is observable via RTT heartbeat. */

static void SystemClock_Config(void);

int main(void)
{
    HAL_Init();
    SystemClock_Config();
    systick_init();

    SEGGER_RTT_Init();
    printf("=== DAP STARTUP ===\r\n");
    printf("STM32F411CE @ %lu Hz, HSE=%u\r\n", (unsigned long)SystemCoreClock, (unsigned)HSE_VALUE);

    uart_init(115200);
    DAP_Setup();
    usb_init();

    uint64_t last_heartbeat = 0;
    uint32_t heartbeat_n = 0;
    while ( 1 )
    {
        usb_poll();

        uint64_t now = systick_get_tick();
        if ( (now - last_heartbeat) >= 1000U )
        {
            printf("tick %lu (%lu ms)\r\n", (unsigned long)++heartbeat_n, (unsigned long)now);
            last_heartbeat = now;
        }
    }
}

/* HSE 25 MHz (Y2 on the WeAct V3.x core board) feeds the main PLL:
 *   PLLM = 25  -> 1 MHz reference
 *   PLLN = 192 -> 192 MHz VCO  (must be a multiple of 48 so PLLQ can divide
 *                               down to exactly 48.000 MHz for OTG_FS; using
 *                               PLLN=200 yields 50 MHz on PLLQ which clocks
 *                               the USB PHY 4% off-spec and produces EILSEQ
 *                               (-84) CRC errors on longer FS bulk responses)
 *   PLLP = 2   -> 96 MHz SYSCLK (HCLK)  (down from the 100 MHz max)
 *   PLLQ = 4   -> 48 MHz for OTG_FS / RNG / SDIO
 * APB2 = HCLK / 1 = 96 MHz, APB1 = HCLK / 2 = 48 MHz (both within F411 limits). */
static void SystemClock_Config(void)
{
    RCC_OscInitTypeDef osc = {0};
    RCC_ClkInitTypeDef clk = {0};

    __HAL_RCC_PWR_CLK_ENABLE();
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

    osc.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    osc.HSEState = RCC_HSE_ON;
    osc.PLL.PLLState = RCC_PLL_ON;
    osc.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    osc.PLL.PLLM = 25;
    osc.PLL.PLLN = 192;
    osc.PLL.PLLP = RCC_PLLP_DIV2;
    osc.PLL.PLLQ = 4;
    if ( HAL_RCC_OscConfig(&osc) != HAL_OK )
    {
        while ( 1 )
        {
        }
    }

    clk.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    clk.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    clk.AHBCLKDivider = RCC_SYSCLK_DIV1;
    clk.APB1CLKDivider = RCC_HCLK_DIV2;
    clk.APB2CLKDivider = RCC_HCLK_DIV1;
    /* 100 MHz with VOS scale 1 needs 3 wait states (RM0383 §3.5.1). */
    if ( HAL_RCC_ClockConfig(&clk, FLASH_LATENCY_3) != HAL_OK )
    {
        while ( 1 )
        {
        }
    }
}

