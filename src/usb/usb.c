#include "usb.h"

#include "tusb.h"
#include "stm32f4xx_hal.h"

/* TinyUSB device interrupt entry. The DCD (synopsys/dwc2) talks directly to
 * OTG_FS registers; we only need to forward the NVIC vector. */
void OTG_FS_IRQHandler(void)
{
    tud_int_handler(0);
}

bool usb_init(void)
{
    /* PA11 = OTG_FS_DM, PA12 = OTG_FS_DP (AF10). The WeAct V3.x board has no
     * external D+ pull-up; we rely on the OTG_FS controller's internal pull-up
     * enabled by the peripheral when it transitions to device-attached. */
    __HAL_RCC_GPIOA_CLK_ENABLE();

    GPIO_InitTypeDef cfg = {0};
    cfg.Pin = GPIO_PIN_11 | GPIO_PIN_12;
    cfg.Mode = GPIO_MODE_AF_PP;
    cfg.Pull = GPIO_NOPULL;
    cfg.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    cfg.Alternate = GPIO_AF10_OTG_FS;
    HAL_GPIO_Init(GPIOA, &cfg);

    __HAL_RCC_USB_OTG_FS_CLK_ENABLE();

    /* Priority must allow sub-priority for the SysTick / USART1 ordering
     * documented in src/uart.c (USART1 = 3.3, SysTick = 15.0). USB at 6.0
     * preempts UART/SysTick but stays below high-priority debug exceptions. */
    HAL_NVIC_SetPriority(OTG_FS_IRQn, 6U, 0U);
    HAL_NVIC_EnableIRQ(OTG_FS_IRQn);

    /* Single rhport 0 -> OTG_FS, device role, full-speed (F411 has no HS PHY). */
    tusb_rhport_init_t init = {
        .role = TUSB_ROLE_DEVICE,
        .speed = TUSB_SPEED_FULL,
    };
    tusb_init(0, &init);

    /* TinyUSB's dwc2 dcd_init pokes BVALOEN/BVALOVAL into GOTGCTL, but those
     * bits do not exist on STM32F411 (added in F7/H7). Without them the
     * controller falls back to its hardware VBUS-sense pin, which on F411 is
     * PA9 -- the same pin we now drive as USART1_TX. The canonical fix on
     * F411 is GCCFG.NOVBUSSENS=1 (RM0383 §22.16.7): disable VBUS sensing,
     * the OTG core assumes VBUS is always present and the D+ pull-up engages
     * on dcd_connect's DCTL.SDIS=0. Mirrors stm32f4xx_ll_usb.c USB_DevDisconnect. */
    USB_OTG_GlobalTypeDef* otg = (USB_OTG_GlobalTypeDef*)USB_OTG_FS_PERIPH_BASE;
    otg->GCCFG |= USB_OTG_GCCFG_NOVBUSSENS;
    otg->GCCFG &= ~(USB_OTG_GCCFG_VBUSASEN | USB_OTG_GCCFG_VBUSBSEN);

    return true;
}

bool usb_deinit(void)
{
    HAL_NVIC_DisableIRQ(OTG_FS_IRQn);
    __HAL_RCC_USB_OTG_FS_CLK_DISABLE();
    return true;
}

void usb_poll(void)
{
    tud_task();
}
