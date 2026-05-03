/*
 * Copyright (c) 2013-2021 ARM Limited. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * ----------------------------------------------------------------------
 *
 * $Date:        16. June 2021
 * $Revision:    V2.1.0
 *
 * Project:      CMSIS-DAP Configuration
 * Title:        DAP_config.h CMSIS-DAP Configuration File (Template)
 *
 *---------------------------------------------------------------------------*/

#ifndef __DAP_CONFIG_H__
#define __DAP_CONFIG_H__

#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

//**************************************************************************************************
/**
\defgroup DAP_Config_Debug_gr CMSIS-DAP Debug Unit Information
\ingroup DAP_ConfigIO_gr
@{
Provides definitions about the hardware and configuration of the Debug Unit.

This information includes:
 - Definition of Cortex-M processor parameters used in CMSIS-DAP Debug Unit.
 - Debug Unit Identification strings (Vendor, Product, Serial Number).
 - Debug Unit communication packet size.
 - Debug Access Port supported modes and settings (JTAG/SWD and SWO).
 - Optional information about a connected Target Device (for Evaluation Boards).
*/

#ifdef _RTE_
  #include "RTE_Components.h"
  #include CMSIS_device_header
#else
  #include "stm32f4xx_hal.h"
  #include "systick.h"
  #include "device_id.h"
#endif

/// Processor Clock of the Cortex-M MCU used in the Debug Unit.
/// This value is used to calculate the SWD/JTAG clock speed.
#define CPU_CLOCK 96000000U ///< Specifies the CPU Clock in Hz.

/// Number of processor cycles for I/O Port write operations.
/// This value is used to calculate the SWD/JTAG clock speed that is generated with I/O
/// Port write operations in the Debug Unit by a Cortex-M MCU. Most Cortex-M processors
/// require 2 processor cycles for a I/O Port Write operation.  If the Debug Unit uses
/// a Cortex-M0+ processor with high-speed peripheral I/O only 1 processor cycle might be
/// required.
#define IO_PORT_WRITE_CYCLES 2U ///< I/O Cycles: 2=default, 1=Cortex-M0+ fast I/0.

/// Indicate that Serial Wire Debug (SWD) communication mode is available at the Debug Access Port.
/// This information is returned by the command \ref DAP_Info as part of <b>Capabilities</b>.
#define DAP_SWD 1 ///< SWD Mode:  1 = available, 0 = not available.

/// Indicate that JTAG communication mode is available at the Debug Port.
/// This information is returned by the command \ref DAP_Info as part of <b>Capabilities</b>.
#define DAP_JTAG 1 ///< JTAG Mode: 1 = available, 0 = not available.

/// Configure maximum number of JTAG devices on the scan chain connected to the Debug Access Port.
/// This setting impacts the RAM requirements of the Debug Unit. Valid range is 1 .. 255.
#define DAP_JTAG_DEV_CNT 8U ///< Maximum number of JTAG devices on scan chain.

/// Default communication mode on the Debug Access Port.
/// Used for the command \ref DAP_Connect when Port Default mode is selected.
#define DAP_DEFAULT_PORT 1U ///< Default JTAG/SWJ Port Mode: 1 = SWD, 2 = JTAG.

/// Default communication speed on the Debug Access Port for SWD and JTAG mode.
/// Used to initialize the default SWD/JTAG clock frequency.
/// The command \ref DAP_SWJ_Clock can be used to overwrite this default setting.
#define DAP_DEFAULT_SWJ_CLOCK 4000000U ///< Default SWD/JTAG clock frequency in Hz.

/// Maximum Package Size for Command and Response data.
/// This configuration settings is used to optimize the communication performance with the
/// debugger and depends on the USB peripheral. Typical vales are 64 for Full-speed USB HID or WinUSB,
/// 1024 for High-speed USB HID and 512 for High-speed USB WinUSB.
#define DAP_PACKET_SIZE 64U ///< Specifies Packet Size in bytes.

/// Maximum Package Buffers for Command and Response data.
/// This configuration settings is used to optimize the communication performance with the
/// debugger and depends on the USB peripheral. For devices with limited RAM or USB buffer the
/// setting can be reduced (valid range is 1 .. 255).
#define DAP_PACKET_COUNT 8U ///< Specifies number of packets buffered.

/// Indicate that UART Serial Wire Output (SWO) trace is available.
/// This information is returned by the command \ref DAP_Info as part of <b>Capabilities</b>.
#define SWO_UART 0 ///< SWO UART:  1 = available, 0 = not available.

/// USART Driver instance number for the UART SWO.
#define SWO_UART_DRIVER 0 ///< USART Driver instance number (Driver_USART#).

/// Maximum SWO UART Baudrate.
#define SWO_UART_MAX_BAUDRATE 10000000U ///< SWO UART Maximum Baudrate in Hz.

/// Indicate that Manchester Serial Wire Output (SWO) trace is available.
/// This information is returned by the command \ref DAP_Info as part of <b>Capabilities</b>.
#define SWO_MANCHESTER 0 ///< SWO Manchester:  1 = available, 0 = not available.

/// SWO Trace Buffer Size.
#define SWO_BUFFER_SIZE 4096U ///< SWO Trace Buffer Size in bytes (must be 2^n).

/// SWO Streaming Trace.
#define SWO_STREAM 0 ///< SWO Streaming Trace: 1 = available, 0 = not available.

/// Clock frequency of the Test Domain Timer. Timer value is returned with \ref TIMESTAMP_GET.
#define TIMESTAMP_CLOCK 0U ///< Timestamp clock in Hz (0 = timestamps not supported).

/// Indicate that UART Communication Port is available.
/// This information is returned by the command \ref DAP_Info as part of <b>Capabilities</b>.
#define DAP_UART 0 ///< DAP UART:  1 = available, 0 = not available.

/// USART Driver instance number for the UART Communication Port.
#define DAP_UART_DRIVER 1 ///< USART Driver instance number (Driver_USART#).

/// UART Receive Buffer Size.
#define DAP_UART_RX_BUFFER_SIZE 1024U ///< Uart Receive Buffer Size in bytes (must be 2^n).

/// UART Transmit Buffer Size.
#define DAP_UART_TX_BUFFER_SIZE 1024U ///< Uart Transmit Buffer Size in bytes (must be 2^n).

/// Indicate that UART Communication via USB COM Port is available.
/// This information is returned by the command \ref DAP_Info as part of <b>Capabilities</b>.
#define DAP_UART_USB_COM_PORT 0 ///< USB COM Port:  1 = available, 0 = not available.

/// Debug Unit is connected to fixed Target Device.
/// The Debug Unit may be part of an evaluation board and always connected to a fixed
/// known device. In this case a Device Vendor, Device Name, Board Vendor and Board Name strings
/// are stored and may be used by the debugger or IDE to configure device parameters.
#define TARGET_FIXED         0           ///< Target: 1 = known, 0 = unknown;

#define TARGET_DEVICE_VENDOR "Arm"       ///< String indicating the Silicon Vendor
#define TARGET_DEVICE_NAME   "Cortex-M"  ///< String indicating the Target Device
#define TARGET_BOARD_VENDOR  "Arm"       ///< String indicating the Board Vendor
#define TARGET_BOARD_NAME    "Arm board" ///< String indicating the Board Name

#if TARGET_FIXED != 0
  #include <string.h>
static const char TargetDeviceVendor[] = TARGET_DEVICE_VENDOR;
static const char TargetDeviceName[] = TARGET_DEVICE_NAME;
static const char TargetBoardVendor[] = TARGET_BOARD_VENDOR;
static const char TargetBoardName[] = TARGET_BOARD_NAME;
#endif

/** Get Vendor Name string.
\param str Pointer to buffer to store the string (max 60 characters).
\return String length (including terminating NULL character) or 0 (no string).
*/
__STATIC_INLINE uint8_t DAP_GetVendorString(char* str)
{
    (void)str;
    return (0U);
}

/** Get Product Name string.
\param str Pointer to buffer to store the string (max 60 characters).
\return String length (including terminating NULL character) or 0 (no string).
*/
__STATIC_INLINE uint8_t DAP_GetProductString(char* str)
{
    (void)str;
    return (0U);
}

/** Get Serial Number string.
\param str Pointer to buffer to store the string (max 60 characters).
\return String length (including terminating NULL character) or 0 (no string).
*/
__STATIC_INLINE uint8_t DAP_GetSerNumString(char* str)
{
    return device_id_get_hashed((uint8_t*)str, 9); // 8 chars + null terminator = 9
}

/** Get Target Device Vendor string.
\param str Pointer to buffer to store the string (max 60 characters).
\return String length (including terminating NULL character) or 0 (no string).
*/
__STATIC_INLINE uint8_t DAP_GetTargetDeviceVendorString(char* str)
{
#if TARGET_FIXED != 0
    uint8_t len;

    strcpy(str, TargetDeviceVendor);
    len = (uint8_t)(strlen(TargetDeviceVendor) + 1U);
    return (len);
#else
    (void)str;
    return (0U);
#endif
}

/** Get Target Device Name string.
\param str Pointer to buffer to store the string (max 60 characters).
\return String length (including terminating NULL character) or 0 (no string).
*/
__STATIC_INLINE uint8_t DAP_GetTargetDeviceNameString(char* str)
{
#if TARGET_FIXED != 0
    uint8_t len;

    strcpy(str, TargetDeviceName);
    len = (uint8_t)(strlen(TargetDeviceName) + 1U);
    return (len);
#else
    (void)str;
    return (0U);
#endif
}

/** Get Target Board Vendor string.
\param str Pointer to buffer to store the string (max 60 characters).
\return String length (including terminating NULL character) or 0 (no string).
*/
__STATIC_INLINE uint8_t DAP_GetTargetBoardVendorString(char* str)
{
#if TARGET_FIXED != 0
    uint8_t len;

    strcpy(str, TargetBoardVendor);
    len = (uint8_t)(strlen(TargetBoardVendor) + 1U);
    return (len);
#else
    (void)str;
    return (0U);
#endif
}

/** Get Target Board Name string.
\param str Pointer to buffer to store the string (max 60 characters).
\return String length (including terminating NULL character) or 0 (no string).
*/
__STATIC_INLINE uint8_t DAP_GetTargetBoardNameString(char* str)
{
#if TARGET_FIXED != 0
    uint8_t len;

    strcpy(str, TargetBoardName);
    len = (uint8_t)(strlen(TargetBoardName) + 1U);
    return (len);
#else
    (void)str;
    return (0U);
#endif
}

/** Get Product Firmware Version string.
\param str Pointer to buffer to store the string (max 60 characters).
\return String length (including terminating NULL character) or 0 (no string).
*/
__STATIC_INLINE uint8_t DAP_GetProductFirmwareVersionString(char* str)
{
    memcpy((unsigned char*)str, "V1.0", strlen("V1.0") + 1);
    return strlen("V1.0") + 1;
}

///@}

//**************************************************************************************************
/**
\defgroup DAP_Config_PortIO_gr CMSIS-DAP Hardware I/O Pin Access
\ingroup DAP_ConfigIO_gr
@{

Standard I/O Pins of the CMSIS-DAP Hardware Debug Port support standard JTAG mode
and Serial Wire Debug (SWD) mode. In SWD mode only 2 pins are required to implement the debug
interface of a device. The following I/O Pins are provided:

JTAG I/O Pin                 | SWD I/O Pin          | CMSIS-DAP Hardware pin mode
---------------------------- | -------------------- | ---------------------------------------------
TCK: Test Clock              | SWCLK: Clock         | Output Push/Pull
TMS: Test Mode Select        | SWDIO: Data I/O      | Output Push/Pull; Input (for receiving data)
TDI: Test Data Input         |                      | Output Push/Pull
TDO: Test Data Output        |                      | Input
nTRST: Test Reset (optional) |                      | Output Open Drain with pull-up resistor
nRESET: Device Reset         | nRESET: Device Reset | Output Open Drain with pull-up resistor


DAP Hardware I/O Pin Access Functions
-------------------------------------
The various I/O Pins are accessed by functions that implement the Read, Write, Set, or Clear to
these I/O Pins.

For the SWDIO I/O Pin there are additional functions that are called in SWD I/O mode only.
This functions are provided to achieve faster I/O that is possible with some advanced GPIO
peripherals that can independently write/read a single I/O pin without affecting any other pins
of the same I/O port. The following SWDIO I/O Pin functions are provided:
 - \ref PIN_SWDIO_OUT_ENABLE to enable the output mode from the DAP hardware.
 - \ref PIN_SWDIO_OUT_DISABLE to enable the input mode to the DAP hardware.
 - \ref PIN_SWDIO_IN to read from the SWDIO I/O pin with utmost possible speed.
 - \ref PIN_SWDIO_OUT to write to the SWDIO I/O pin with utmost possible speed.
*/

// Configure DAP I/O pins ------------------------------
#define SWCLK_PORT      GPIOA
#define SWCLK_PIN       GPIO_PIN_14
#define SWCLK_PIN_INDEX 14
#define SWDIO_PORT      GPIOA
#define SWDIO_PIN       GPIO_PIN_13
#define SWDIO_PIN_INDEX 13

#define JTAG_TCK_PORT   SWCLK_PORT
#define JTAG_TCK_PIN    SWCLK_PIN
#define JTAG_TMS_PORT   SWDIO_PORT
#define JTAG_TMS_PIN    SWDIO_PIN
#define JTAG_TDI_PORT   GPIOB
#define JTAG_TDI_PIN    GPIO_PIN_15
#define JTAG_TDO_PORT   GPIOB
#define JTAG_TDO_PIN    GPIO_PIN_14

// nRESET output: PA8 on the WeAct board, adjacent to PA9 USART1_TX so the
// probe-side cabling stays in one block (no spider-web wiring).
#define nRESET_PORT     GPIOA
#define nRESET_PIN      GPIO_PIN_8

// On the WeAct V3.x core board there is exactly one user-controllable LED on
// PC13 (active-low: anode tied to 3V3 via R5 1.5K; pin pulled low to light).
// Both LED_CONNECTED and LED_RUNNING are mapped here. Their state is OR-ed in
// software (see led_state_* / led_pc13_drive() below) so either DAP signal
// can light the LED without colliding with the other.
#define LED_CONNECTED_PORT GPIOC
#define LED_CONNECTED_PIN  GPIO_PIN_13
#define LED_RUNNING_PORT   GPIOC
#define LED_RUNNING_PIN    GPIO_PIN_13

// CMSIS-DAP LED state bits, OR-ed onto PC13 by led_pc13_drive(). Storage in
// src/dap_led_state.c so the LED_*_OUT inlines below stay header-only.
extern volatile uint8_t led_state_connected;
extern volatile uint8_t led_state_running;

__STATIC_FORCEINLINE void led_pc13_drive(void)
{
    if ( led_state_connected || led_state_running )
        GPIOC->BSRR = (uint32_t)GPIO_PIN_13 << 16; // PC13 LOW = LED ON
    else
        GPIOC->BSRR = (uint32_t)GPIO_PIN_13;       // PC13 HIGH = LED OFF
}

// SWDIO / SWCLK on PA13 / PA14 are the MCU's own SWD pins. The OTG_FS / GPIO
// reclaim happens implicitly when MODER[pin] is rewritten away from AF: no
// AFIO SWJ-disable remap exists on STM32F4 (that was a CH32V305 thing). This
// stub kept for upstream-source-diff parity; body intentionally empty.
__STATIC_INLINE void sdi_gpio_ctrl(bool sdi_active)
{
    (void)sdi_active;
}

/** Setup JTAG I/O pins: TCK, TMS, TDI, TDO, nTRST, and nRESET.
Configures the DAP Hardware I/O pins for JTAG mode:
 - TCK, TMS, TDI, nTRST, nRESET to output mode and set to high level.
 - TDO to input mode.
*/
__STATIC_INLINE void PORT_JTAG_SETUP(void)
{
    GPIO_InitTypeDef cfg = {0};
    cfg.Mode = GPIO_MODE_OUTPUT_PP;
    cfg.Pull = GPIO_NOPULL;
    cfg.Speed = GPIO_SPEED_FREQ_HIGH;

    JTAG_TCK_PORT->BSRR = JTAG_TCK_PIN;
    JTAG_TMS_PORT->BSRR = JTAG_TMS_PIN;
    JTAG_TDI_PORT->BSRR = JTAG_TDI_PIN;

    cfg.Pin = JTAG_TCK_PIN;
    HAL_GPIO_Init(JTAG_TCK_PORT, &cfg);

    cfg.Pin = JTAG_TMS_PIN;
    HAL_GPIO_Init(JTAG_TMS_PORT, &cfg);

    cfg.Pin = JTAG_TDI_PIN;
    HAL_GPIO_Init(JTAG_TDI_PORT, &cfg);

    cfg.Pin = JTAG_TDO_PIN;
    cfg.Mode = GPIO_MODE_INPUT;
    cfg.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(JTAG_TDO_PORT, &cfg);

    sdi_gpio_ctrl(false);
}

/** Setup SWD I/O pins: SWCLK, SWDIO, and nRESET.
Configures the DAP Hardware I/O pins for Serial Wire Debug (SWD) mode:
 - SWCLK, SWDIO, nRESET to output mode and set to default high level.
 - TDI, nTRST to HighZ mode (pins are unused in SWD mode).
*/
__STATIC_INLINE void PORT_SWD_SETUP(void)
{
    GPIO_InitTypeDef cfg = {0};
    cfg.Mode = GPIO_MODE_OUTPUT_PP;
    cfg.Pull = GPIO_NOPULL;
    cfg.Speed = GPIO_SPEED_FREQ_HIGH;

    SWCLK_PORT->BSRR = SWCLK_PIN;
    SWDIO_PORT->BSRR = SWDIO_PIN;

    cfg.Pin = SWCLK_PIN;
    HAL_GPIO_Init(SWCLK_PORT, &cfg);

    cfg.Pin = SWDIO_PIN;
    HAL_GPIO_Init(SWDIO_PORT, &cfg);

    sdi_gpio_ctrl(false);
}

/** Disable JTAG/SWD I/O Pins.
Disables the DAP Hardware I/O pins which configures:
 - TCK/SWCLK, TMS/SWDIO, TDI, TDO, nTRST, nRESET to High-Z mode.
*/
__STATIC_INLINE void PORT_OFF(void)
{

    GPIO_InitTypeDef cfg = {0};
    cfg.Mode = GPIO_MODE_INPUT;
    cfg.Pull = GPIO_NOPULL;
    cfg.Speed = GPIO_SPEED_FREQ_HIGH;

    cfg.Pin = SWCLK_PIN;
    HAL_GPIO_Init(SWCLK_PORT, &cfg);

    cfg.Pin = SWDIO_PIN;
    HAL_GPIO_Init(SWDIO_PORT, &cfg);

    cfg.Pin = JTAG_TCK_PIN;
    HAL_GPIO_Init(JTAG_TCK_PORT, &cfg);

    cfg.Pin = JTAG_TMS_PIN;
    HAL_GPIO_Init(JTAG_TMS_PORT, &cfg);

    cfg.Pin = JTAG_TDI_PIN;
    HAL_GPIO_Init(JTAG_TDI_PORT, &cfg);

    cfg.Pin = JTAG_TDO_PIN;
    HAL_GPIO_Init(JTAG_TDO_PORT, &cfg);

    sdi_gpio_ctrl(true);
}

// SWCLK/TCK I/O pin -------------------------------------

/** SWCLK/TCK I/O pin: Get Input.
\return Current status of the SWCLK/TCK DAP hardware I/O pin.
*/
__STATIC_FORCEINLINE uint32_t PIN_SWCLK_TCK_IN(void)
{
    return (SWCLK_PORT->IDR & SWCLK_PIN) ? 1 : 0;
}

/** SWCLK/TCK I/O pin: Set Output to High.
Set the SWCLK/TCK DAP hardware I/O pin to high level.
*/
__STATIC_FORCEINLINE void PIN_SWCLK_TCK_SET(void)
{
    SWCLK_PORT->BSRR = SWCLK_PIN;
}

/** SWCLK/TCK I/O pin: Set Output to Low.
Set the SWCLK/TCK DAP hardware I/O pin to low level.
*/
__STATIC_FORCEINLINE void PIN_SWCLK_TCK_CLR(void)
{
    SWCLK_PORT->BSRR = (uint32_t)SWCLK_PIN << 16;
}

// SWDIO/TMS Pin I/O --------------------------------------

/** SWDIO/TMS I/O pin: Get Input.
\return Current status of the SWDIO/TMS DAP hardware I/O pin.
*/
__STATIC_FORCEINLINE uint32_t PIN_SWDIO_TMS_IN(void)
{
    return (SWDIO_PORT->IDR & SWDIO_PIN) ? 1 : 0;
}

/** SWDIO/TMS I/O pin: Set Output to High.
Set the SWDIO/TMS DAP hardware I/O pin to high level.
*/
__STATIC_FORCEINLINE void PIN_SWDIO_TMS_SET(void)
{
    SWDIO_PORT->BSRR = SWDIO_PIN;
}

/** SWDIO/TMS I/O pin: Set Output to Low.
Set the SWDIO/TMS DAP hardware I/O pin to low level.
*/
__STATIC_FORCEINLINE void PIN_SWDIO_TMS_CLR(void)
{
    SWDIO_PORT->BSRR = (uint32_t)SWDIO_PIN << 16;
}

/** SWDIO I/O pin: Get Input (used in SWD mode only).
\return Current status of the SWDIO DAP hardware I/O pin.
*/
__STATIC_FORCEINLINE uint32_t PIN_SWDIO_IN(void)
{
    return PIN_SWDIO_TMS_IN();
}

/** SWDIO I/O pin: Set Output (used in SWD mode only).
\param bit Output value for the SWDIO DAP hardware I/O pin.
*/
__STATIC_FORCEINLINE void PIN_SWDIO_OUT(uint32_t bit)
{
    if ( bit & 1 )
        PIN_SWDIO_TMS_SET();
    else
        PIN_SWDIO_TMS_CLR();
}

/** SWDIO I/O pin: Switch to Output mode (used in SWD mode only).
Configure the SWDIO DAP hardware I/O pin to output mode. This function is
called prior \ref PIN_SWDIO_OUT function calls.
*/
__STATIC_FORCEINLINE void PIN_SWDIO_OUT_ENABLE(void)
{
    PIN_SWDIO_TMS_CLR();

    // STM32 MODER: 2 bits per pin -- 00=input, 01=output, 10=AF, 11=analog.
    // Mask the SWDIO_PIN_INDEX field then write 01 (output).
    SWDIO_PORT->MODER = (SWDIO_PORT->MODER & ~(0x3U << (SWDIO_PIN_INDEX * 2))) | (0x1U << (SWDIO_PIN_INDEX * 2));
}

/** SWDIO I/O pin: Switch to Input mode (used in SWD mode only).
Configure the SWDIO DAP hardware I/O pin to input mode. This function is
called prior \ref PIN_SWDIO_IN function calls.
*/
__STATIC_FORCEINLINE void PIN_SWDIO_OUT_DISABLE(void)
{
    PIN_SWDIO_TMS_SET();

    // Switch SWDIO MODER field to 00 (input). The previous SET call ensures
    // the internal latch is high before high-Z, so we do not glitch low.
    SWDIO_PORT->MODER = (SWDIO_PORT->MODER & ~(0x3U << (SWDIO_PIN_INDEX * 2)));
}

// TDI Pin I/O ---------------------------------------------

/** TDI I/O pin: Get Input.
\return Current status of the TDI DAP hardware I/O pin.
*/
__STATIC_FORCEINLINE uint32_t PIN_TDI_IN(void)
{
    return (JTAG_TDI_PORT->IDR & JTAG_TDI_PIN) ? 1 : 0;
}

/** TDI I/O pin: Set Output.
\param bit Output value for the TDI DAP hardware I/O pin.
*/
__STATIC_FORCEINLINE void PIN_TDI_OUT(uint32_t bit)
{
    if ( bit & 1 )
        JTAG_TDI_PORT->BSRR = JTAG_TDI_PIN;
    else
        JTAG_TDI_PORT->BSRR = (uint32_t)JTAG_TDI_PIN << 16;
}

// TDO Pin I/O ---------------------------------------------

/** TDO I/O pin: Get Input.
\return Current status of the TDO DAP hardware I/O pin.
*/
__STATIC_FORCEINLINE uint32_t PIN_TDO_IN(void)
{
    return (JTAG_TDO_PORT->IDR & JTAG_TDO_PIN) ? 1 : 0;
}

// nTRST Pin I/O -------------------------------------------

/** nTRST I/O pin: Get Input.
\return Current status of the nTRST DAP hardware I/O pin.
*/
__STATIC_FORCEINLINE uint32_t PIN_nTRST_IN(void)
{
    return (0U);
}

/** nTRST I/O pin: Set Output.
\param bit JTAG TRST Test Reset pin status:
           - 0: issue a JTAG TRST Test Reset.
           - 1: release JTAG TRST Test Reset.
*/
__STATIC_FORCEINLINE void PIN_nTRST_OUT(uint32_t bit)
{
    (void)bit;
}

// nRESET Pin I/O------------------------------------------

/** nRESET I/O pin: Get Input.
\return Current status of the nRESET DAP hardware I/O pin.
*/
__STATIC_FORCEINLINE uint32_t PIN_nRESET_IN(void)
{
    return (nRESET_PORT->IDR & nRESET_PIN) ? 1 : 0;
}

/** nRESET I/O pin: Set Output.
\param bit target device hardware reset pin status:
           - 0: issue a device hardware reset.
           - 1: release device hardware reset.
*/
__STATIC_FORCEINLINE void PIN_nRESET_OUT(uint32_t bit)
{
    if ( bit & 1 )
        nRESET_PORT->BSRR = nRESET_PIN;             // open-drain release (high-Z)
    else
        nRESET_PORT->BSRR = (uint32_t)nRESET_PIN << 16; // pull low (asserted)
}

///@}

//**************************************************************************************************
/**
\defgroup DAP_Config_LEDs_gr CMSIS-DAP Hardware Status LEDs
\ingroup DAP_ConfigIO_gr
@{

CMSIS-DAP Hardware may provide LEDs that indicate the status of the CMSIS-DAP Debug Unit.

It is recommended to provide the following LEDs for status indication:
 - Connect LED: is active when the DAP hardware is connected to a debugger.
 - Running LED: is active when the debugger has put the target device into running state.
*/

/** Debug Unit: Set status of Connected LED.
\param bit status of the Connect LED.
           - 1: Connect LED ON: debugger is connected to CMSIS-DAP Debug Unit.
           - 0: Connect LED OFF: debugger is not connected to CMSIS-DAP Debug Unit.
*/
__STATIC_INLINE void LED_CONNECTED_OUT(uint32_t bit)
{
    led_state_connected = (uint8_t)(bit & 1U);
    led_pc13_drive();
}

/** Debug Unit: Set status Target Running LED.
\param bit status of the Target Running LED.
           - 1: Target Running LED ON: program execution in target started.
           - 0: Target Running LED OFF: program execution in target stopped.
*/
__STATIC_INLINE void LED_RUNNING_OUT(uint32_t bit)
{
    led_state_running = (uint8_t)(bit & 1U);
    led_pc13_drive();
}

///@}

//**************************************************************************************************
/**
\defgroup DAP_Config_Timestamp_gr CMSIS-DAP Timestamp
\ingroup DAP_ConfigIO_gr
@{
Access function for Test Domain Timer.

The value of the Test Domain Timer in the Debug Unit is returned by the function \ref TIMESTAMP_GET. By
default, the DWT timer is used.  The frequency of this timer is configured with \ref TIMESTAMP_CLOCK.

*/

/** Get timestamp of Test Domain Timer.
\return Current timestamp value.
*/
__STATIC_INLINE uint32_t TIMESTAMP_GET(void)
{
    return (uint32_t)systick_get_tick();
}

///@}

//**************************************************************************************************
/**
\defgroup DAP_Config_Initialization_gr CMSIS-DAP Initialization
\ingroup DAP_ConfigIO_gr
@{

CMSIS-DAP Hardware I/O and LED Pins are initialized with the function \ref DAP_SETUP.
*/

/** Setup of the Debug Unit I/O pins and LEDs (called when Debug Unit is initialized).
This function performs the initialization of the CMSIS-DAP Hardware I/O Pins and the
Status LEDs. In detail the operation of Hardware I/O and LED pins are enabled and set:
 - I/O clock system enabled.
 - all I/O pins: input buffer enabled, output pins are set to HighZ mode.
 - for nTRST, nRESET a weak pull-up (if available) is enabled.
 - LED output pins are enabled and LEDs are turned off.
*/
__STATIC_INLINE void DAP_SETUP(void)
{
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();

    GPIO_InitTypeDef cfg = {0};
    cfg.Speed = GPIO_SPEED_FREQ_HIGH;

    // LED on PC13 (push-pull; active-low). LED_CONNECTED and LED_RUNNING share
    // this physical pin; OR-combined via led_state_* in led_pc13_drive().
    cfg.Pin = LED_CONNECTED_PIN;          // == LED_RUNNING_PIN == GPIO_PIN_13
    cfg.Mode = GPIO_MODE_OUTPUT_PP;
    cfg.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(LED_CONNECTED_PORT, &cfg);
    LED_CONNECTED_OUT(0);
    LED_RUNNING_OUT(0);

    // nRESET on PA8 (open-drain output, default released = high-Z).
    nRESET_PORT->BSRR = nRESET_PIN;
    cfg.Pin = nRESET_PIN;
    cfg.Mode = GPIO_MODE_OUTPUT_OD;
    cfg.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(nRESET_PORT, &cfg);

    // default port off
    PORT_OFF();
}

/** Reset Target Device with custom specific I/O pin or command sequence.
This function allows the optional implementation of a device specific reset sequence.
It is called when the command \ref DAP_ResetTarget and is for example required
when a device needs a time-critical unlock sequence that enables the debug port.
\return 0 = no device specific reset sequence is implemented.\n
        1 = a device specific reset sequence is implemented.
*/
__STATIC_INLINE uint8_t RESET_TARGET(void)
{
    return (0U); // change to '1' when a device reset sequence is implemented
}

///@}

#endif /* __DAP_CONFIG_H__ */
