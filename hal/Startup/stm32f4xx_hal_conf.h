/**
  * stm32f4xx_hal_conf.h - HAL configuration for BZH_DAPLink_STM32F411 (WeAct).
  *
  * Trimmed to the modules this firmware actually uses:
  *  RCC (+EX), GPIO, CORTEX, DMA (+EX), PWR (+EX),
  *  PCD (+EX) for OTG_FS device, UART for the CDC bridge.
  * Add modules here only when src/ adds a real consumer.
  */

#ifndef __STM32F4xx_HAL_CONF_H
#define __STM32F4xx_HAL_CONF_H

#ifdef __cplusplus
extern "C" {
#endif

/* ########################## Module Selection ############################## */
#define HAL_MODULE_ENABLED
#define HAL_CORTEX_MODULE_ENABLED
#define HAL_DMA_MODULE_ENABLED
#define HAL_FLASH_MODULE_ENABLED
#define HAL_GPIO_MODULE_ENABLED
#define HAL_PCD_MODULE_ENABLED
#define HAL_PWR_MODULE_ENABLED
#define HAL_RCC_MODULE_ENABLED
#define HAL_UART_MODULE_ENABLED

/* ########################## HSE/HSI Values adaptation ##################### */
/* WeAct Black Pill V3.x ships a 25 MHz HSE crystal (Y2). Override at compile
 * time via -DHSE_VALUE=... in cmake/compile_options.cmake; this default is the
 * common case. */
#if !defined(HSE_VALUE)
  #define HSE_VALUE 25000000U
#endif
#if !defined(HSE_STARTUP_TIMEOUT)
  #define HSE_STARTUP_TIMEOUT 100U
#endif

#if !defined(HSI_VALUE)
  #define HSI_VALUE 16000000U
#endif

#if !defined(LSI_VALUE)
  #define LSI_VALUE 32000U
#endif

#if !defined(LSE_VALUE)
  #define LSE_VALUE 32768U
#endif
#if !defined(LSE_STARTUP_TIMEOUT)
  #define LSE_STARTUP_TIMEOUT 5000U
#endif

#if !defined(EXTERNAL_CLOCK_VALUE)
  #define EXTERNAL_CLOCK_VALUE 12288000U
#endif

/* ########################### System Configuration ######################### */
#define VDD_VALUE                 3300U
#define TICK_INT_PRIORITY         0x0FU
#define USE_RTOS                  0U
#define PREFETCH_ENABLE           1U
#define INSTRUCTION_CACHE_ENABLE  1U
#define DATA_CACHE_ENABLE         1U

#define USE_HAL_PCD_REGISTER_CALLBACKS  0U
#define USE_HAL_UART_REGISTER_CALLBACKS 0U

/* ########################## Assert Selection ############################## */
/* No HAL_ASSERT (default off). Define USE_FULL_ASSERT and provide
 * `assert_failed(uint8_t*, uint32_t)` to opt in. */

/* Includes ------------------------------------------------------------------*/
#ifdef HAL_RCC_MODULE_ENABLED
  #include "stm32f4xx_hal_rcc.h"
#endif
#ifdef HAL_GPIO_MODULE_ENABLED
  #include "stm32f4xx_hal_gpio.h"
#endif
#ifdef HAL_DMA_MODULE_ENABLED
  #include "stm32f4xx_hal_dma.h"
#endif
#ifdef HAL_CORTEX_MODULE_ENABLED
  #include "stm32f4xx_hal_cortex.h"
#endif
#ifdef HAL_FLASH_MODULE_ENABLED
  #include "stm32f4xx_hal_flash.h"
#endif
#ifdef HAL_PWR_MODULE_ENABLED
  #include "stm32f4xx_hal_pwr.h"
#endif
#ifdef HAL_UART_MODULE_ENABLED
  #include "stm32f4xx_hal_uart.h"
#endif
#ifdef HAL_PCD_MODULE_ENABLED
  #include "stm32f4xx_hal_pcd.h"
#endif

/* Exported macro ------------------------------------------------------------*/
#ifdef USE_FULL_ASSERT
  #define assert_param(expr) ((expr) ? (void)0U : assert_failed((uint8_t *)__FILE__, __LINE__))
  void assert_failed(uint8_t* file, uint32_t line);
#else
  #define assert_param(expr) ((void)0U)
#endif

#ifdef __cplusplus
}
#endif

#endif /* __STM32F4xx_HAL_CONF_H */
