###############################
# Cross-toolchain: arm-none-eabi auto-fetched into project tree.
# On first configure, downloads Arm GNU Toolchain from developer.arm.com into
# ${TOP_DIR}/arm_toolchain/ (gitignored). Subsequent configures are no-op once
# the extracted tree is present. For a system-installed toolchain instead, use
# cmake/toolchain_system.cmake.

cmake_minimum_required(VERSION 3.21)
include(FetchContent)

set(ARM_TOOLCHAIN_VERSION "14.2.rel1" CACHE STRING "Arm GNU toolchain version")
set(TOOL_CHAIN_PREFIX "arm-none-eabi" CACHE STRING "Toolchain prefix")

# Dual-mode: included from CMakeLists.txt (TOP_DIR set) OR loaded as
# CMAKE_TOOLCHAIN_FILE before TOP_DIR exists. Fall back to the parent of cmake/.
if(NOT DEFINED TOP_DIR)
  get_filename_component(TOP_DIR "${CMAKE_CURRENT_LIST_DIR}/.." ABSOLUTE)
endif()

set(BZH_TOOLCHAIN_DIR "${TOP_DIR}/arm_toolchain")
set(ARM_TOOLCHAIN_ROOT "${BZH_TOOLCHAIN_DIR}/arm-gnu-toolchain-${ARM_TOOLCHAIN_VERSION}")
set(TOOLCHAIN_BIN "${ARM_TOOLCHAIN_ROOT}/bin")

if(CMAKE_HOST_SYSTEM_NAME STREQUAL "Linux")
  if(CMAKE_HOST_SYSTEM_PROCESSOR MATCHES "aarch64")
    set(ARM_TOOLCHAIN_HOST "aarch64")
  else()
    set(ARM_TOOLCHAIN_HOST "x86_64")
  endif()
  set(ARM_TOOLCHAIN_EXT "tar.xz")
elseif(CMAKE_HOST_SYSTEM_NAME STREQUAL "Darwin")
  if(CMAKE_HOST_SYSTEM_PROCESSOR MATCHES "arm64")
    set(ARM_TOOLCHAIN_HOST "darwin-arm64")
  else()
    set(ARM_TOOLCHAIN_HOST "darwin-x86_64")
  endif()
  set(ARM_TOOLCHAIN_EXT "tar.xz")
elseif(CMAKE_HOST_SYSTEM_NAME STREQUAL "Windows")
  set(ARM_TOOLCHAIN_HOST "mingw-w64-i686")
  set(ARM_TOOLCHAIN_EXT "zip")
else()
  message(FATAL_ERROR "Unsupported host system: ${CMAKE_HOST_SYSTEM_NAME}")
endif()

set(ARM_TOOLCHAIN_NAME "arm-gnu-toolchain-${ARM_TOOLCHAIN_VERSION}-${ARM_TOOLCHAIN_HOST}-${TOOL_CHAIN_PREFIX}")
set(ARM_TOOLCHAIN_URL "https://developer.arm.com/-/media/Files/downloads/gnu/${ARM_TOOLCHAIN_VERSION}/binrel/${ARM_TOOLCHAIN_NAME}.${ARM_TOOLCHAIN_EXT}")

if(NOT EXISTS "${TOOLCHAIN_BIN}/${TOOL_CHAIN_PREFIX}-gcc")
  message(STATUS "Downloading Arm GNU Toolchain ${ARM_TOOLCHAIN_VERSION} from ${ARM_TOOLCHAIN_URL}")
  file(MAKE_DIRECTORY "${BZH_TOOLCHAIN_DIR}")
  FetchContent_Declare(
    arm_toolchain
    URL "${ARM_TOOLCHAIN_URL}"
    SOURCE_DIR "${ARM_TOOLCHAIN_ROOT}"
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
  )
  FetchContent_MakeAvailable(arm_toolchain)
  message(STATUS "Arm GNU Toolchain installed to ${ARM_TOOLCHAIN_ROOT}")
else()
  message(STATUS "Using existing Arm GNU Toolchain at ${ARM_TOOLCHAIN_ROOT}")
endif()

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR ARM)
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

set(CMAKE_ASM_COMPILER "${TOOLCHAIN_BIN}/${TOOL_CHAIN_PREFIX}-gcc"  CACHE FILEPATH "ASM compiler"  FORCE)
set(CMAKE_C_COMPILER   "${TOOLCHAIN_BIN}/${TOOL_CHAIN_PREFIX}-gcc"  CACHE FILEPATH "C compiler"    FORCE)
set(CMAKE_CXX_COMPILER "${TOOLCHAIN_BIN}/${TOOL_CHAIN_PREFIX}-g++"  CACHE FILEPATH "C++ compiler"  FORCE)
set(CMAKE_OBJCOPY      "${TOOLCHAIN_BIN}/${TOOL_CHAIN_PREFIX}-objcopy" CACHE FILEPATH "objcopy"    FORCE)
set(CMAKE_OBJDUMP      "${TOOLCHAIN_BIN}/${TOOL_CHAIN_PREFIX}-objdump" CACHE FILEPATH "objdump"    FORCE)
set(CMAKE_SIZE         "${TOOLCHAIN_BIN}/${TOOL_CHAIN_PREFIX}-size"    CACHE FILEPATH "size"       FORCE)
set(CMAKE_AR           "${TOOLCHAIN_BIN}/${TOOL_CHAIN_PREFIX}-gcc-ar"     CACHE FILEPATH "ar"      FORCE)
set(CMAKE_RANLIB       "${TOOLCHAIN_BIN}/${TOOL_CHAIN_PREFIX}-gcc-ranlib" CACHE FILEPATH "ranlib"  FORCE)

execute_process(
  COMMAND ${CMAKE_C_COMPILER} -print-sysroot
  OUTPUT_VARIABLE ARM_GCC_SYSROOT
  OUTPUT_STRIP_TRAILING_WHITESPACE
)
if(ARM_GCC_SYSROOT)
  set(CMAKE_SYSROOT ${ARM_GCC_SYSROOT} CACHE PATH "Sysroot" FORCE)
endif()

set(CMAKE_FIND_ROOT_PATH "${ARM_TOOLCHAIN_ROOT}")
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
