###############################
# CMake
cmake_minimum_required(VERSION 3.28)
set(CMAKE_VERBOSE_MAKEFILE OFF)

set(TOP_DIR ${CMAKE_CURRENT_LIST_DIR})

include("${TOP_DIR}/cmake/project_common.cmake")

###############################
# Vars
get_timestamp() # provide BUILD_TIME
get_commit() # provide BUILD_COMMIT
# get_version(${src_dir}/version.h) # provide BUILD_VERSION

###############################
# Project
project(BZH_DAPLink_WchLinkE LANGUAGES C ASM)

set(SOURCES
  # Startup
  hal/Startup/core_riscv.c
  hal/Startup/startup_ch32v30x_D8C.S
  hal/Startup/system_ch32v30x.c
  hal/Startup/ch32v30x_it.c

  # Peripheral
  # hal/Peripheral/ch32v30x_bkp.c
  # hal/Peripheral/ch32v30x_crc.c
  hal/Peripheral/ch32v30x_dbgmcu.c
  hal/Peripheral/ch32v30x_dma.c
  # hal/Peripheral/ch32v30x_exti.c
  # hal/Peripheral/ch32v30x_flash.c
  hal/Peripheral/ch32v30x_gpio.c
  # hal/Peripheral/ch32v30x_iwdg.c
  hal/Peripheral/ch32v30x_misc.c
  # hal/Peripheral/ch32v30x_pwr.c
  hal/Peripheral/ch32v30x_rcc.c
  # hal/Peripheral/ch32v30x_rng.c
  hal/Peripheral/ch32v30x_usart.c
  # hal/Peripheral/ch32v30x_wwdg.c

  # USB Device
  # src/USBD/ch32v30x_usbhs_device.c
  # src/USBD/usb_desc.c

  # DAP
  modules/cmsis_dap/Firmware/Source/DAP.c
  modules/cmsis_dap/Firmware/Source/DAP_vendor.c
  modules/cmsis_dap/Firmware/Source/JTAG_DP.c
  modules/cmsis_dap/Firmware/Source/SW_DP.c
  modules/cmsis_dap/Firmware/Source/SWO.c
  modules/cmsis_dap/Firmware/Source/UART.c

  # user
  src/main.c
  src/systick.c
  src/device_id.c
  # src/usb/usb.c
  # src/hid_transfer.c
  # src/vcom_serial.c
  src/debug.c
  src/sdi.c
  src/uart3.c
  src/uart3_conflict_checker.c
  src/ring_buffer.c

  # SEGGER RTT (vendored under TinyUSB submodule, BSD-3-Clause).
  # Has RV32 csrr/csrci lock primitives in SEGGER_RTT_Conf.h, no porting needed.
  modules/tinyusb/lib/SEGGER_RTT/RTT/SEGGER_RTT.c
)

set(BUILD_OUTPUT_BASENAME "${PROJECT_NAME}-${BUILD_TIME}-${BUILD_COMMIT}")

add_executable(${PROJECT_NAME} ${SOURCES})
set_target_properties(${PROJECT_NAME} PROPERTIES
  SUFFIX ".elf"
  OUTPUT_NAME "${BUILD_OUTPUT_BASENAME}"
)
set_property(TARGET ${PROJECT_NAME} PROPERTY LINK_DEPENDS ${LINKER_SCRIPT})

target_include_directories(${PROJECT_NAME} PRIVATE

  ${CMAKE_SOURCE_DIR}/hal/Startup
  ${CMAKE_SOURCE_DIR}/hal/Peripheral

  ${CMAKE_SOURCE_DIR}/src/USBD
  ${CMAKE_SOURCE_DIR}/modules/cmsis_dap/Firmware/Include

  ${CMAKE_SOURCE_DIR}/src

  ${CMAKE_SOURCE_DIR}/modules/tinyusb/lib/SEGGER_RTT/RTT
  ${CMAKE_SOURCE_DIR}/modules/tinyusb/lib/SEGGER_RTT/Config
)

###############################
# Compile options

# common
include("${TOP_DIR}/cmake/compile_options.cmake")
add_compile_options(
  ${PROJ_WIDE_C_FLAGS}
)
add_link_options(
  ${PROJ_WIDE_LD_FLAGS}
)
link_libraries(
  ${PROJ_WIDE_LIBRARIES}
)
include_directories(
  ${PROJ_WIDE_INCLUDE_DIRS}
)
add_compile_definitions(
  ${PROJ_WIDE_DEFINES}
)
target_compile_options(${PROJECT_NAME} PRIVATE
  ${PROJ_WIDE_C_FLAGS}
  -fmessage-length=0
  -fsigned-char
)
target_link_options(${PROJECT_NAME} PRIVATE
  ${PROJ_WIDE_LD_FLAGS}
  -T${TOP_DIR}/hal/ld/link.ld
  -Wl,-Map,${PROJECT_BINARY_DIR}/${BUILD_OUTPUT_BASENAME}.map
  -Wl,--print-memory-usage
  -Wl,--cref
)
target_include_directories(${PROJECT_NAME} PRIVATE
  ${PROJ_WIDE_INCLUDE_DIRS}
)
target_compile_definitions(${PROJECT_NAME} PRIVATE
  ${PROJ_WIDE_DEFINES}
)

set(HEX_FILE ${PROJECT_BINARY_DIR}/${BUILD_OUTPUT_BASENAME}.hex)
set(BIN_FILE ${PROJECT_BINARY_DIR}/${BUILD_OUTPUT_BASENAME}.bin)
set(LST_FILE ${PROJECT_BINARY_DIR}/${BUILD_OUTPUT_BASENAME}.lst)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
  COMMAND ${CMAKE_OBJCOPY} -Oihex $<TARGET_FILE:${PROJECT_NAME}> ${HEX_FILE}
  COMMAND ${CMAKE_OBJCOPY} -Obinary $<TARGET_FILE:${PROJECT_NAME}> ${BIN_FILE}
  COMMAND ${CMAKE_OBJDUMP} --all-headers --demangle --disassemble $<TARGET_FILE:${PROJECT_NAME}> > ${LST_FILE}
  COMMAND ${CMAKE_SIZE} --format=berkeley $<TARGET_FILE:${PROJECT_NAME}>
)

###############################
# Dependencies

add_subdirectory(src/usb)
target_include_directories(
  usb_stack PRIVATE
  ${CMAKE_SOURCE_DIR}/hal/Startup
  ${CMAKE_SOURCE_DIR}/hal/Peripheral
)
target_link_libraries(${PROJECT_NAME} PRIVATE usb_stack)
