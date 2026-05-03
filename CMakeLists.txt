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
project(BZH_DAPLink_STM32F411 LANGUAGES C ASM)

# OTG_FS / TinyUSB device stack. Linked by default; flip OFF for size-only
# bring-up runs that don't need USB.
option(BUILD_USB "Compile and link the TinyUSB device stack" ON)

set(SOURCES
  # Startup
  hal/Startup/startup_stm32f411xe.s
  hal/Startup/system_stm32f4xx.c
  hal/Startup/stm32f4xx_it.c
  hal/Startup/syscalls.c
  hal/Startup/sysmem.c

  # SEGGER RTT (vendored under TinyUSB submodule, BSD-3-Clause). Conf.h auto-
  # detects ARMv7-M and uses PRIMASK MRS/MSR for the lock primitive.
  modules/tinyusb/lib/SEGGER_RTT/RTT/SEGGER_RTT.c

  # CMSIS-DAP submodule sources (portable; all hardware glue lives in DAP_config.h)
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
  src/debug.c
  src/uart.c
  src/dap_config_checker.c
  src/ring_buffer.c
  src/dap_led_state.c

)

set(BUILD_OUTPUT_BASENAME "${PROJECT_NAME}-${BUILD_TIME}-${BUILD_COMMIT}")

add_executable(${PROJECT_NAME} ${SOURCES})
set_target_properties(${PROJECT_NAME} PROPERTIES
  SUFFIX ".elf"
  OUTPUT_NAME "${BUILD_OUTPUT_BASENAME}"
)
set_property(TARGET ${PROJECT_NAME} PROPERTY LINK_DEPENDS ${LINKER_SCRIPT})

target_include_directories(${PROJECT_NAME} PRIVATE
  ${CMAKE_SOURCE_DIR}/modules/cmsis_dap/Firmware/Include
  ${CMAKE_SOURCE_DIR}/src
  ${CMAKE_SOURCE_DIR}/modules/tinyusb/lib/SEGGER_RTT/RTT
  ${CMAKE_SOURCE_DIR}/modules/tinyusb/lib/SEGGER_RTT/Config
)

###############################
# Compile options
# Must run BEFORE add_subdirectory(hal/Peripheral) so the stm32_hal OBJECT
# library inherits the MCU flags + STM32F411xE define from add_compile_*().

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

###############################
# SSP stub libraries (for arm-none-eabi GCC15+ tarballs that drop libssp/libssp_nonshared)
# Empty .a archives satisfy the linker when -fstack-protector-* is in the flags.
# If you ever want real stack-smashing protection you need to provide
# __stack_chk_guard / __stack_chk_fail yourself; the project flags currently
# settle for "link-clean" since we already have RTT-based fault diagnostics.
add_library(ssp STATIC IMPORTED GLOBAL)
add_library(ssp_nonshared STATIC IMPORTED GLOBAL)
set_target_properties(ssp PROPERTIES IMPORTED_LOCATION "${CMAKE_CURRENT_BINARY_DIR}/libssp.a")
set_target_properties(ssp_nonshared PROPERTIES IMPORTED_LOCATION "${CMAKE_CURRENT_BINARY_DIR}/libssp_nonshared.a")
add_custom_command(
    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/libssp.a" "${CMAKE_CURRENT_BINARY_DIR}/libssp_nonshared.a"
    COMMAND ${CMAKE_AR} rcs "${CMAKE_CURRENT_BINARY_DIR}/libssp.a"
    COMMAND ${CMAKE_AR} rcs "${CMAKE_CURRENT_BINARY_DIR}/libssp_nonshared.a"
    COMMENT "Creating empty SSP stub libraries"
)
add_custom_target(ssp_stubs DEPENDS
    "${CMAKE_CURRENT_BINARY_DIR}/libssp.a"
    "${CMAKE_CURRENT_BINARY_DIR}/libssp_nonshared.a"
)
add_link_options(-L${CMAKE_CURRENT_BINARY_DIR})
add_dependencies(${PROJECT_NAME} ssp_stubs)

###############################
# HAL
add_subdirectory(hal/Peripheral)
target_link_libraries(${PROJECT_NAME} PRIVATE stm32_hal)

target_compile_options(${PROJECT_NAME} PRIVATE
  ${PROJ_WIDE_C_FLAGS}
  -fmessage-length=0
  -fsigned-char
)
target_link_options(${PROJECT_NAME} PRIVATE
  ${PROJ_WIDE_LD_FLAGS}
  -T${TOP_DIR}/hal/ld/link.ld
  # Look for the empty libssp / libssp_nonshared archives produced by ssp_stubs.
  # Required because nano.specs pulls -lssp* into the link, but the upstream
  # arm-none-eabi tarballs (Arm GCC 14+) no longer ship those.
  -L${PROJECT_BINARY_DIR}
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
# USB stack (gated; Stage 6 will flip BUILD_USB ON)

if(BUILD_USB)
  add_subdirectory(src/usb)
  target_link_libraries(${PROJECT_NAME} PRIVATE usb_stack)
endif()
