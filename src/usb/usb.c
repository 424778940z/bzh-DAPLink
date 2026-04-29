#include "usb.h"

#include "tusb.h"
#include "ch32v30x.h"

__attribute__((interrupt)) void USBHS_IRQHandler(void)
{
#if CFG_TUD_WCH_USBIP_USBHS
    tud_int_handler(0);
#endif
}

__attribute__((interrupt)) void USBFS_IRQHandler(void)
{
#if CFG_TUD_WCH_USBIP_USBFS
    tud_int_handler(0);
#endif
}

bool usb_init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure = {0};
    NVIC_InitTypeDef NVIC_InitStructure = {0};

    __disable_irq();

    // v305/v307: Highspeed USB
    RCC_USBCLK48MConfig(RCC_USBCLK48MCLKSource_USBPHY);
    RCC_USBHSPLLCLKConfig(RCC_HSBHSPLLCLKSource_HSE);
    RCC_USBHSConfig(RCC_USBPLL_Div3);
    RCC_USBHSPLLCKREFCLKConfig(RCC_USBHSPLLCKREFCLK_4M);
    RCC_USBHSPHYPLLALIVEcmd(ENABLE);
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_USBHS, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11 | GPIO_Pin_12;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    uint8_t otg_div;
    switch ( SystemCoreClock )
    {
    case 48000000:
        otg_div = RCC_OTGFSCLKSource_PLLCLK_Div1;
        break;
    case 96000000:
        otg_div = RCC_OTGFSCLKSource_PLLCLK_Div2;
        break;
    case 144000000:
        otg_div = RCC_OTGFSCLKSource_PLLCLK_Div3;
        break;
    default:
        TU_ASSERT(0, false);
        break;
    }
    RCC_OTGFSCLKConfig(otg_div);
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_OTG_FS, ENABLE);

    NVIC_InitStructure.NVIC_IRQChannel = USBHS_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    __enable_irq();

    tusb_rhport_init_t dev_init = {
        .role = TUSB_ROLE_DEVICE,
        .speed = TUSB_SPEED_AUTO,
    };

    // HS device is wired to rhport 0 on WCH HS IP
    tusb_init(0, &dev_init);

    return true;
}
