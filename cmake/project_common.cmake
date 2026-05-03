###############################
# CMAKE CONFIG
set(CMAKE_USE_RELATIVE_PATHS TRUE)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# verbose build
# set(CMAKE_VERBOSE_MAKEFILE ON CACHE BOOL "ON")

# block verbose build in CI
if ((DEFINED ENV{CI}) AND (CMAKE_VERBOSE_MAKEFILE))
  message(FATAL_ERROR "Enable verbose build in CI is not allowed because it may print key in log!")
endif()

# default to release build if not set
if(NOT DEFINED CMAKE_CONFIGURATION_TYPES OR "${CMAKE_BUILD_TYPE}" STREQUAL "")
  if(NOT DEFINED CMAKE_BUILD_TYPE OR "${CMAKE_BUILD_TYPE}" STREQUAL "")
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "" FORCE)
  endif()
endif()

# enforce release build (not used)
# if(NOT CMAKE_BUILD_TYPE STREQUAL "Release")
#   message(FATAL_ERROR "This project must build in release mode! Otherwise binary size will overflow")
# endif()

###############################
# TOOL CHAIN
# Toolchain is selected via CMAKE_TOOLCHAIN_FILE (CMakePresets.json or -D on the
# command line); this file no longer hard-includes a specific toolchain.

# Standards
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Ccache (optional)
find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
  message(STATUS "Ccache found: ${CCACHE_PROGRAM}")
  set(CMAKE_C_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
  set(CMAKE_CXX_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
endif()

###############################
# MACROS

macro(get_version file)
  file(READ ${file} ver_file)
  string(REGEX MATCH "VERSION_MAJOR ([0-9]*)" _ ${ver_file})
  set(ver_major ${CMAKE_MATCH_1})
  string(REGEX MATCH "VERSION_MINOR ([0-9]*)" _ ${ver_file})
  set(ver_minor ${CMAKE_MATCH_1})
  string(REGEX MATCH "VERSION_PATCH ([0-9]*)" _ ${ver_file})
  set(ver_patch ${CMAKE_MATCH_1})
  set(BUILD_VERSION "${ver_major}.${ver_minor}.${ver_patch}")
endmacro()

macro(get_timestamp)
  string(TIMESTAMP BUILD_TIME "%Y%m%d")
endmacro()

macro(get_commit)
  find_package(Git REQUIRED)
  execute_process(
    COMMAND "${GIT_EXECUTABLE}" rev-parse --short HEAD
    # COMMAND_ECHO STDOUT
    OUTPUT_VARIABLE BUILD_COMMIT
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )
endmacro()

###############################
# PATHS

# src

# output
set(DIR_OUTPUT_PREFIX "${TOP_DIR}/artifacts")
# set(DIR_OUTPUT_PREFIX "${TOP_DIR}/artifacts")
set(DIR_OUTPUT_FACTORY "${DIR_OUTPUT_PREFIX}/factory")
set(DIR_OUTPUT_MISC "${DIR_OUTPUT_PREFIX}/misc")

# utils
set(DIR_UTILS "${TOP_DIR}/utils")

# files
# set(FILE_OUTPUT_ZIP "${DIR_OUTPUT}/ota.zip")

# dependencies
set(PYTHON_VENV_DIR "${TOP_DIR}/.venv")
set(PYTHON_RUNNER exit -1 &&) # placeholder, will be set later

###############################
# COMMON DEPENDENCIES

add_custom_target(
  PROJ_OUT_DIRS
  COMMENT "Ensuring project output directories"
  COMMAND mkdir -p ${DIR_OUTPUT_FACTORY}
  COMMAND mkdir -p ${DIR_OUTPUT_MISC}
  BYPRODUCTS ${DIR_OUTPUT_PREFIX}
)

# python env
set(PYTHON_RUNNER ${PYTHON_VENV_DIR}/bin/python)
add_custom_command(
  OUTPUT ${PYTHON_RUNNER}
  COMMENT "Ensuring project Python environment -> ${PYTHON_RUNNER}"
  COMMAND bash ${DIR_UTILS}/setup_venv.sh
  DEPENDS ${DIR_UTILS}/python_requirements.txt ${DIR_UTILS}/setup_venv.sh
)
add_custom_target(
  PROJ_PYENV
  COMMENT "Ensuring project Python environment -> ${PYTHON_RUNNER}"
  DEPENDS ${PYTHON_RUNNER}
)
set_property(
  TARGET PROJ_PYENV
  APPEND
  PROPERTY ADDITIONAL_CLEAN_FILES ${PYTHON_VENV_DIR}
)

###############################
# COMMON FLAGS
set(PROJ_COMMON_BUILD_DEFINES
  BUILD_ID=\"${BUILD_ID}\"
  PRODUCTION_BUILD=${PRODUCTION_BUILD}
)