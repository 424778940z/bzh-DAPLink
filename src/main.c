#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#include "ch32v30x.h"

#include "systick.h"
#include "uart3.h"

#include "debug.h"
#include "usb/usb.h"
#include "tusb.h"

#include "DAP.h"

static void int_to_unicode(uint32_t value, uint8_t* pbuf, uint8_t len)
{
    for ( int i = 0; i < len; i++ )
    {
        if ( (value >> 28) < 0xA )
        {
            pbuf[2 * i] = (value >> 28) + '0';
        }
        else
        {
            pbuf[2 * i] = (value >> 28) + 'A' - 10;
        }

        pbuf[2 * i + 1] = 0;

        value = value << 4;
    }
}

#if defined(DAP_PWR_OUT)
static void enable_power_output(void)
{
    GPIO_InitTypeDef GPIO_InitStructure = {0};

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    GPIO_ResetBits(GPIOA, GPIO_Pin_5);
    GPIO_ResetBits(GPIOB, GPIO_Pin_12);
}
#endif

void main(void)
{
    systick_init();
    uart3_init(115200);

    debug_init();
    debug_printf("=== DAP STARTUP ===\r\n");

    // disable unused pins
    GPIO_InitTypeDef GPIO_InitStruct;
    // PA9 is connected to PA13(SWDIO) internally on CH32V305FBP6
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_9;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStruct);
    // PB13 is connected to PA14(SWDCLK) on WCH-LinkE
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_13;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOB, &GPIO_InitStruct);
    // PC6 is connected to SWDIO via 1k resistor on WCH-LinkE
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_6;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOC, &GPIO_InitStruct);

#if defined(DAP_PWR_OUT)
    enable_power_output();
#endif
    DAP_Setup();

    usb_init();

    while ( 1 )
    {
        tud_task();
    }
}
