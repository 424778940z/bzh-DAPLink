###############################
# stm32_hal: STM32CubeF4 HAL drivers (curated).
#
# Pulls source files directly from the submodule at
# ${TOP_DIR}/modules/STM32CubeF4 -- nothing copied into the working tree.
# Add a driver here only when src/ adds a real consumer.

set(STM32CUBE_DIR "${TOP_DIR}/modules/STM32CubeF4")
set(STM32_HAL_DRIVER_DIR "${STM32CUBE_DIR}/Drivers/STM32F4xx_HAL_Driver")
set(STM32_CMSIS_DIR "${STM32CUBE_DIR}/Drivers/CMSIS")

if(NOT EXISTS "${STM32_HAL_DRIVER_DIR}/Src/stm32f4xx_hal.c")
  message(FATAL_ERROR
    "STM32F4xx HAL driver missing at ${STM32_HAL_DRIVER_DIR}.\n"
    "Run: git submodule update --init --recursive modules/STM32CubeF4")
endif()

set(STM32_HAL_SOURCES
  ${STM32_HAL_DRIVER_DIR}/Src/stm32f4xx_hal.c
  ${STM32_HAL_DRIVER_DIR}/Src/stm32f4xx_hal_cortex.c
  ${STM32_HAL_DRIVER_DIR}/Src/stm32f4xx_hal_dma.c
  ${STM32_HAL_DRIVER_DIR}/Src/stm32f4xx_hal_dma_ex.c
  ${STM32_HAL_DRIVER_DIR}/Src/stm32f4xx_hal_flash.c
  ${STM32_HAL_DRIVER_DIR}/Src/stm32f4xx_hal_flash_ex.c
  ${STM32_HAL_DRIVER_DIR}/Src/stm32f4xx_hal_gpio.c
  ${STM32_HAL_DRIVER_DIR}/Src/stm32f4xx_hal_pcd.c
  ${STM32_HAL_DRIVER_DIR}/Src/stm32f4xx_hal_pcd_ex.c
  ${STM32_HAL_DRIVER_DIR}/Src/stm32f4xx_hal_pwr.c
  ${STM32_HAL_DRIVER_DIR}/Src/stm32f4xx_hal_pwr_ex.c
  ${STM32_HAL_DRIVER_DIR}/Src/stm32f4xx_hal_rcc.c
  ${STM32_HAL_DRIVER_DIR}/Src/stm32f4xx_hal_rcc_ex.c
  ${STM32_HAL_DRIVER_DIR}/Src/stm32f4xx_hal_uart.c
  ${STM32_HAL_DRIVER_DIR}/Src/stm32f4xx_ll_usb.c
)

add_library(stm32_hal OBJECT ${STM32_HAL_SOURCES})

target_include_directories(stm32_hal PUBLIC
  ${STM32_CMSIS_DIR}/Core/Include
  ${STM32_CMSIS_DIR}/Device/ST/STM32F4xx/Include
  ${STM32_HAL_DRIVER_DIR}/Inc
  ${TOP_DIR}/hal/Startup           # stm32f4xx_hal_conf.h
)

# HAL driver sources trip a few of the project's strict warnings on -Werror;
# silence at the boundary so the application code stays strict.
#   - unused-parameter / unused-but-set-variable: cosmetic in HAL macros
#   - redundant-decls: stm32f4xx_hal_flash_ex.c re-declares FLASH_WaitForLastOperation
#   - cast-align / shadow / format / pedantic: variants seen across HAL versions
target_compile_options(stm32_hal PRIVATE
  -Wno-unused-parameter
  -Wno-unused-but-set-variable
  -Wno-redundant-decls
  -Wno-error=cast-align
  -Wno-error=shadow
)
