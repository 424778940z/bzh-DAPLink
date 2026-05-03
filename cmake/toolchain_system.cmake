###############################
# Cross-toolchain: arm-none-eabi from system PATH.
# Use this preset when arm-none-eabi-gcc is already installed on the host
# (apt: gcc-arm-none-eabi, brew: arm-none-eabi-gcc, AUR: arm-none-eabi-*, etc.)
# For a self-contained download instead, use cmake/toolchain_arm.cmake.

cmake_minimum_required(VERSION 3.21)

if(NOT TOOL_CHAIN_PREFIX)
  set(TOOL_CHAIN_PREFIX "arm-none-eabi")
endif()

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR ARM)
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

find_program(CMAKE_ASM_COMPILER NAMES ${TOOL_CHAIN_PREFIX}-gcc REQUIRED)
find_program(CMAKE_C_COMPILER   NAMES ${TOOL_CHAIN_PREFIX}-gcc REQUIRED)
find_program(CMAKE_CXX_COMPILER NAMES ${TOOL_CHAIN_PREFIX}-g++ REQUIRED)
find_program(CMAKE_OBJCOPY      NAMES ${TOOL_CHAIN_PREFIX}-objcopy REQUIRED)
find_program(CMAKE_OBJDUMP      NAMES ${TOOL_CHAIN_PREFIX}-objdump REQUIRED)
find_program(CMAKE_SIZE         NAMES ${TOOL_CHAIN_PREFIX}-size    REQUIRED)
# gcc-ar / gcc-ranlib so LTO works correctly across archive members
find_program(CMAKE_AR     NAMES ${TOOL_CHAIN_PREFIX}-gcc-ar     REQUIRED)
find_program(CMAKE_RANLIB NAMES ${TOOL_CHAIN_PREFIX}-gcc-ranlib REQUIRED)

execute_process(
  COMMAND ${CMAKE_C_COMPILER} -print-sysroot
  OUTPUT_VARIABLE ARM_GCC_SYSROOT
  OUTPUT_STRIP_TRAILING_WHITESPACE
)
if(ARM_GCC_SYSROOT)
  set(CMAKE_SYSROOT ${ARM_GCC_SYSROOT} CACHE PATH "Sysroot" FORCE)
endif()

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

message(STATUS "CMAKE_SYSROOT:      ${CMAKE_SYSROOT}")
message(STATUS "CMAKE_ASM_COMPILER: ${CMAKE_ASM_COMPILER}")
message(STATUS "CMAKE_C_COMPILER:   ${CMAKE_C_COMPILER}")
message(STATUS "CMAKE_CXX_COMPILER: ${CMAKE_CXX_COMPILER}")
message(STATUS "CMAKE_OBJCOPY:      ${CMAKE_OBJCOPY}")
message(STATUS "CMAKE_OBJDUMP:      ${CMAKE_OBJDUMP}")
message(STATUS "CMAKE_SIZE:         ${CMAKE_SIZE}")
message(STATUS "CMAKE_AR:           ${CMAKE_AR}")
message(STATUS "CMAKE_RANLIB:       ${CMAKE_RANLIB}")
