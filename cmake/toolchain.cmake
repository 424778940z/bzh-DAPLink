###############################
# Bootstrap: WCH RISC-V toolchain
#
# Platform toolchain packs live on the orphan `toolchain-hs-wch-wchlinkE`
# branch of this repo (LFS-tracked). On first configure, we fetch that branch
# from the project's own `origin` remote (reusing whatever credentials the
# user already proved during the project clone), pull only the host-appropriate
# pack zip from LFS, and extract it into wch_tools/. No re-clone, no second
# auth dance. Subsequent configures are no-ops once the extracted toolchain
# is present.
#
# Fork-friendliness: GitHub does not replicate LFS storage to forks, so a
# fresh fork of this repo will carry the LFS *pointers* but not the binary
# blobs. To handle that transparently, the LFS step tries `origin` first
# and, on failure, falls back to WCH_TOOLCHAIN_CANONICAL_URL via a
# temporary remote. The canonical upstream is public, so HTTPS reads need
# no authentication.
#
# wch_tools/ is fully derivative and gitignored on main; cmake owns its
# entire content.
#
# Requires: git and git-lfs on PATH at configure time; project must be a
# git work tree with an `origin` remote.
#
# Overrides:
#   -DWCH_TOOLCHAIN_GIT_REPO=<url>      fetch from this URL instead of
#                                       `origin` (registered as a temporary
#                                       remote so git-lfs can resolve its
#                                       endpoint). Disables canonical
#                                       fallback — caller is in charge.
#   -DWCH_TOOLCHAIN_GIT_TAG=<ref>       pin to a specific commit/tag/branch
#   -DWCH_TOOLCHAIN_CANONICAL_URL=<url> override the LFS fallback target
#                                       (forks of forks may want this)

# This file is dual-mode: included from CMakeLists.txt (TOP_DIR already set)
# OR loaded as CMAKE_TOOLCHAIN_FILE (TOP_DIR not yet defined — fall back to
# the parent of cmake/, which is the project root).
if(NOT DEFINED TOP_DIR)
  get_filename_component(TOP_DIR "${CMAKE_CURRENT_LIST_DIR}/.." ABSOLUTE)
endif()

set(WCH_TOOLCHAIN_GIT_REPO ""
    CACHE STRING "Optional URL override; default uses the project's `origin` remote")
set(WCH_TOOLCHAIN_GIT_TAG
    "wch-mrs-v2.3.0"
    CACHE STRING "Ref holding <platform>.zip toolchain packs")
set(WCH_TOOLCHAIN_CANONICAL_URL
    "https://github.com/424778940z/bzh-DAPLink.git"
    CACHE STRING "Public upstream URL used as LFS fallback for forks")

if(CMAKE_HOST_SYSTEM_NAME MATCHES "Windows")
  set(_wch_pack_name "windows.zip")
elseif(CMAKE_HOST_SYSTEM_NAME MATCHES "Linux")
  set(_wch_pack_name "linux.zip")
elseif(CMAKE_HOST_SYSTEM_NAME MATCHES "Darwin")
  message(FATAL_ERROR "macOS WCH toolchain pack not yet published")
else()
  message(FATAL_ERROR "Unsupported host system: ${CMAKE_HOST_SYSTEM_NAME}")
endif()

set(_wch_dir "${TOP_DIR}/wch_tools")

if(NOT EXISTS "${_wch_dir}/toolchain")
  find_program(GIT_LFS_EXECUTABLE NAMES git-lfs REQUIRED
    DOC "git-lfs is required to fetch the toolchain pack blobs from LFS storage")

  # Make git/git-lfs fail fast on missing auth instead of opening a terminal
  # or GUI credential prompt mid-configure. Forks hitting the canonical
  # fallback should surface a clean error if canonical itself is unreachable
  # or auth-walled, rather than silently hanging behind a popup.
  set(ENV{GIT_TERMINAL_PROMPT} "0")
  set(ENV{GIT_ASKPASS} "")
  set(ENV{SSH_ASKPASS} "")
  unset(ENV{DISPLAY})
  unset(ENV{WAYLAND_DISPLAY})

  if(WCH_TOOLCHAIN_GIT_REPO)
    set(_wch_remote "_wch_toolchain_override")
    # Idempotent re-register in case a previous run was interrupted.
    execute_process(
      COMMAND git remote remove ${_wch_remote}
      WORKING_DIRECTORY "${TOP_DIR}"
      OUTPUT_QUIET ERROR_QUIET
    )
    execute_process(
      COMMAND git remote add ${_wch_remote} "${WCH_TOOLCHAIN_GIT_REPO}"
      WORKING_DIRECTORY "${TOP_DIR}"
      RESULT_VARIABLE _wch_add_rc
    )
    if(NOT _wch_add_rc EQUAL 0)
      message(FATAL_ERROR "Failed to register override remote ${WCH_TOOLCHAIN_GIT_REPO}")
    endif()
    # Explicit override means the caller chose that URL; do not second-guess
    # them by silently retrying canonical on LFS failure.
    set(_wch_allow_canonical_fallback FALSE)
  else()
    set(_wch_remote "origin")
    set(_wch_allow_canonical_fallback TRUE)
  endif()

  message(STATUS "Fetching WCH toolchain ref ${WCH_TOOLCHAIN_GIT_TAG} from ${_wch_remote} (pack: ${_wch_pack_name})")

  execute_process(
    COMMAND git fetch --depth=1 ${_wch_remote} ${WCH_TOOLCHAIN_GIT_TAG}
    WORKING_DIRECTORY "${TOP_DIR}"
    RESULT_VARIABLE _wch_fetch_rc
  )
  if(NOT _wch_fetch_rc EQUAL 0)
    message(FATAL_ERROR "git fetch ${_wch_remote} ${WCH_TOOLCHAIN_GIT_TAG} failed (exit ${_wch_fetch_rc}); check that the remote is reachable and the ref exists")
  endif()

  # Resolve FETCH_HEAD to a SHA so both git-lfs and cat-file accept it
  # uniformly (FETCH_HEAD is a magic ref that some git-lfs paths don't honor).
  execute_process(
    COMMAND git rev-parse FETCH_HEAD
    WORKING_DIRECTORY "${TOP_DIR}"
    OUTPUT_VARIABLE _wch_fetch_sha
    OUTPUT_STRIP_TRAILING_WHITESPACE
    RESULT_VARIABLE _wch_rev_rc
  )
  if(NOT _wch_rev_rc EQUAL 0)
    message(FATAL_ERROR "Failed to resolve FETCH_HEAD after fetch")
  endif()

  execute_process(
    COMMAND ${GIT_LFS_EXECUTABLE} fetch ${_wch_remote} ${_wch_fetch_sha} --include=${_wch_pack_name}
    WORKING_DIRECTORY "${TOP_DIR}"
    RESULT_VARIABLE _wch_lfs_rc
  )
  if(NOT _wch_lfs_rc EQUAL 0)
    if(_wch_allow_canonical_fallback)
      # Most likely cause: this is a fork. GitHub doesn't copy LFS storage
      # across forks, so origin holds the pointer but no blob. Retry against
      # the canonical upstream, which serves the blob over public HTTPS.
      message(STATUS "LFS lookup from ${_wch_remote} failed (likely a fork without mirrored LFS); falling back to ${WCH_TOOLCHAIN_CANONICAL_URL}")
      set(_wch_canonical_remote "_wch_toolchain_canonical")
      execute_process(
        COMMAND git remote remove ${_wch_canonical_remote}
        WORKING_DIRECTORY "${TOP_DIR}"
        OUTPUT_QUIET ERROR_QUIET
      )
      execute_process(
        COMMAND git remote add ${_wch_canonical_remote} "${WCH_TOOLCHAIN_CANONICAL_URL}"
        WORKING_DIRECTORY "${TOP_DIR}"
        RESULT_VARIABLE _wch_can_add_rc
      )
      if(NOT _wch_can_add_rc EQUAL 0)
        message(FATAL_ERROR "Failed to register canonical fallback remote ${WCH_TOOLCHAIN_CANONICAL_URL}")
      endif()
      execute_process(
        COMMAND ${GIT_LFS_EXECUTABLE} fetch ${_wch_canonical_remote} ${_wch_fetch_sha} --include=${_wch_pack_name}
        WORKING_DIRECTORY "${TOP_DIR}"
        RESULT_VARIABLE _wch_lfs_rc2
      )
      execute_process(
        COMMAND git remote remove ${_wch_canonical_remote}
        WORKING_DIRECTORY "${TOP_DIR}"
        OUTPUT_QUIET ERROR_QUIET
      )
      if(NOT _wch_lfs_rc2 EQUAL 0)
        message(FATAL_ERROR "LFS fetch failed from both ${_wch_remote} (exit ${_wch_lfs_rc}) and ${WCH_TOOLCHAIN_CANONICAL_URL} (exit ${_wch_lfs_rc2})")
      endif()
    else()
      message(FATAL_ERROR "git lfs fetch ${_wch_remote} ${_wch_fetch_sha} --include=${_wch_pack_name} failed (exit ${_wch_lfs_rc})")
    endif()
  endif()

  # Materialize the pack zip via the standard LFS pipeline: read the pointer
  # from the tree at FETCH_HEAD, pipe it through `git lfs smudge` which uses
  # the local LFS object store populated above. No separate work tree.
  set(_wch_pack_path "${CMAKE_BINARY_DIR}/${_wch_pack_name}")
  execute_process(
    COMMAND git cat-file -p "${_wch_fetch_sha}:${_wch_pack_name}"
    COMMAND ${GIT_LFS_EXECUTABLE} smudge
    WORKING_DIRECTORY "${TOP_DIR}"
    OUTPUT_FILE "${_wch_pack_path}"
    RESULT_VARIABLE _wch_smudge_rc
  )
  if(NOT _wch_smudge_rc EQUAL 0)
    message(FATAL_ERROR "Failed to materialize ${_wch_pack_name} from LFS (exit ${_wch_smudge_rc})")
  endif()

  if(WCH_TOOLCHAIN_GIT_REPO)
    execute_process(
      COMMAND git remote remove ${_wch_remote}
      WORKING_DIRECTORY "${TOP_DIR}"
      OUTPUT_QUIET ERROR_QUIET
    )
  endif()

  message(STATUS "Extracting ${_wch_pack_name} -> ${_wch_dir}")
  file(ARCHIVE_EXTRACT
    INPUT       "${_wch_pack_path}"
    DESTINATION "${_wch_dir}"
  )
  file(REMOVE "${_wch_pack_path}")
else()
  message(STATUS "WCH toolchain already unpacked at ${_wch_dir}/toolchain")
endif()

###############################
# Toolchain configuration
set(TOOLCHAIN_DIR "${_wch_dir}/toolchain")
set(TOOL_CHAIN_PREFIX "riscv-wch-elf")
if(WIN32)
  set(TOOL_CHAIN_SUFFIX ".exe")
else()
  set(TOOL_CHAIN_SUFFIX "")
endif()

message("TOOL_CHAIN_PREFIX=${TOOL_CHAIN_PREFIX}")
message("TOOL_CHAIN_SUFFIX=${TOOL_CHAIN_SUFFIX}")
message("TOOLCHAIN_DIR=${TOOLCHAIN_DIR}")

# cross options
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR RISCV)
set(CMAKE_TRY_COMPILE_TARGET_TYPE "STATIC_LIBRARY")
# set root path
set(CMAKE_FIND_ROOT_PATH ${TOOLCHAIN_DIR})
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
# set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM BOTH)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE NEVER)
# search programs
find_program(CMAKE_ASM_COMPILER NAMES ${TOOL_CHAIN_PREFIX}-gcc PATHS ${TOOLCHAIN_DIR}/bin REQUIRED NO_DEFAULT_PATH)
find_program(CMAKE_C_COMPILER NAMES ${TOOL_CHAIN_PREFIX}-gcc PATHS ${TOOLCHAIN_DIR}/bin REQUIRED NO_DEFAULT_PATH)
find_program(CMAKE_CXX_COMPILER NAMES ${TOOL_CHAIN_PREFIX}-g++ PATHS ${TOOLCHAIN_DIR}/bin REQUIRED NO_DEFAULT_PATH)
find_program(CMAKE_AR NAMES ${TOOL_CHAIN_PREFIX}-ar PATHS ${TOOLCHAIN_DIR}/bin REQUIRED NO_DEFAULT_PATH)
find_program(CMAKE_OBJCOPY NAMES ${TOOL_CHAIN_PREFIX}-objcopy PATHS ${TOOLCHAIN_DIR}/bin REQUIRED NO_DEFAULT_PATH)
find_program(CMAKE_OBJDUMP NAMES ${TOOL_CHAIN_PREFIX}-objdump PATHS ${TOOLCHAIN_DIR}/bin REQUIRED NO_DEFAULT_PATH)
find_program(CMAKE_SIZE NAMES ${TOOL_CHAIN_PREFIX}-size PATHS ${TOOLCHAIN_DIR}/bin REQUIRED NO_DEFAULT_PATH)
# set(CMAKE_ASM_COMPILER "${TOOLCHAIN_DIR}/bin/${TOOL_CHAIN_PREFIX}-gcc${TOOL_CHAIN_SUFFIX}")
# set(CMAKE_C_COMPILER "${TOOLCHAIN_DIR}/bin/${TOOL_CHAIN_PREFIX}-gcc${TOOL_CHAIN_SUFFIX}")
# set(CMAKE_CXX_COMPILER "${TOOLCHAIN_DIR}/bin/${TOOL_CHAIN_PREFIX}-g++${TOOL_CHAIN_SUFFIX}")
# set(CMAKE_AR "${TOOLCHAIN_DIR}/bin/${TOOL_CHAIN_PREFIX}-ar${TOOL_CHAIN_SUFFIX}")
# set(CMAKE_OBJCOPY "${TOOLCHAIN_DIR}/bin/${TOOL_CHAIN_PREFIX}-objcopy${TOOL_CHAIN_SUFFIX}")
# set(CMAKE_OBJDUMP "${TOOLCHAIN_DIR}/bin/${TOOL_CHAIN_PREFIX}-objdump${TOOL_CHAIN_SUFFIX}")
# set(CMAKE_SIZE "${TOOLCHAIN_DIR}/bin/${TOOL_CHAIN_PREFIX}-size${TOOL_CHAIN_SUFFIX}")
# generator
find_program(CMAKE_MAKE_PROGRAM NAMES ninja REQUIRED)
# set(CMAKE_MAKE_PROGRAM "ninja")
# check tools
if((NOT CMAKE_ASM_COMPILER) OR (NOT CMAKE_C_COMPILER) OR (NOT CMAKE_CXX_COMPILER) OR (NOT CMAKE_OBJCOPY))
    message(FATAL_ERROR "Valid toolchain not found")
endif()
# get sysroot
execute_process(COMMAND ${CMAKE_C_COMPILER} -print-sysroot
    OUTPUT_VARIABLE RISCV_GCC_SYSROOT OUTPUT_STRIP_TRAILING_WHITESPACE)
set(CMAKE_SYSROOT ${RISCV_GCC_SYSROOT})

message("CMAKE_ASM_COMPILER=${CMAKE_ASM_COMPILER}")
message("CMAKE_C_COMPILER=${CMAKE_C_COMPILER}")
message("CMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}")
message("CMAKE_AR=${CMAKE_AR}")
message("CMAKE_OBJCOPY=${CMAKE_OBJCOPY}")
message("CMAKE_OBJDUMP=${CMAKE_OBJDUMP}")
message("CMAKE_SIZE=${CMAKE_SIZE}")
message("CMAKE_SYSROOT=${CMAKE_SYSROOT}")
message("CMAKE_MAKE_PROGRAM=${CMAKE_MAKE_PROGRAM}")
