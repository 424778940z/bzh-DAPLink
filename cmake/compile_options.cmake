
set(PROJ_WIDE_COMMON_FLAGS
  # MCU
  -march=rv32imacxw
  -mabi=ilp32
  -msmall-data-limit=8
  -msave-restore

  # options
  -ffreestanding
  -fno-builtin
  -fno-common
  -fno-exceptions
  -fsingle-precision-constant
  # -fstack-protector-all # not work well with FreeRTOS, don't try
  -fstack-protector-strong
  -fstack-usage
  -fvisibility=internal
  -fdata-sections
  -ffunction-sections
  -Wl,--gc-sections
  $<$<CONFIG:Release>:-flto=auto>

  # spec
  --specs=nano.specs
  --specs=nosys.specs
  # -u _printf_float

  # misc options
  # -ffile-prefix-map="old"="new"

  # -Wl,--wrap=__stack_chk_fail # wrap for it to work properly under FreeRTOS
  # -Wl,--wrap=__stack_chk_init # hack for catching PC points to STACK TOP, should not be used normally

  # Note:
  # there is a long existing gcc linker bug, which lto and wrap will conflict, see https://sourceware.org/bugzilla/show_bug.cgi?id=24415
  # as a workaround, wrapped function must add attribute "[[gnu::used]]" to force marking as used
)

set(PROJ_WIDE_C_FLAGS
  ${PROJ_WIDE_COMMON_FLAGS}

  # build type
  $<$<CONFIG:Debug>:-ggdb>
  $<$<CONFIG:Debug>:-Os>

  $<$<CONFIG:Release>:-g0>
  $<$<CONFIG:Release>:-Os>

  -Werror
  -W
  -Wall
  -Wextra
  -Wformat=2

  # -Wstrict-prototypes
  -Wmultichar
  -Wpointer-arith

  # manual warnings
  -Wno-error=cpp
  -Wno-error=comment

  # unused things
  # -Wno-unused-parameter
  -Wno-error=unused-parameter
  # -Wno-unused-function
  -Wno-error=unused-function
  # -Wno-unused-variable
  -Wno-error=unused-variable

  # picky options
  -Wshadow
  # -Wno-error=shadow
  # -Wundef
  # -Wno-undef
  -Wno-error=undef
  -Wredundant-decls
  # -Wno-redundant-decls
  # -Wno-implicit-fallthrough
  -Wno-error=implicit-fallthrough
)

set(PROJ_WIDE_LD_FLAGS
  ${PROJ_WIDE_COMMON_FLAGS}
  -static
  # -nostdlib
  -nostartfiles
  $<$<CONFIG:Release>:-s>
)

set(PROJ_WIDE_LIBRARIES
  m
  gcc
)

set(PROJ_WIDE_INCLUDE_DIRS
)

set(PROJ_WIDE_DEFINES
  "CH32V30x_D8C"
  "HSE_VALUE=12000000U"
  # DAP_CDC and DEBUG_OUTPUT_UART both want UART3 -- pick at most one.
  # Compile-time conflict check lives in src/uart3_conflict_checker.c.
  # $<$<CONFIG:Debug>:"DEBUG_OUTPUT=DEBUG_OUTPUT_UART">
  # $<$<CONFIG:Debug>:"DEBUG_OUTPUT=DEBUG_OUTPUT_SDI">
  "$<$<CONFIG:Debug>:DEBUG_OUTPUT=DEBUG_OUTPUT_RTT>"
  "$<$<CONFIG:Release>:DEBUG_OUTPUT=DEBUG_OUTPUT_NONE>"
  # Pin the SEGGER RTT control block to .rtt_cb (placed first in RAM by
  # hal/ld/link.ld, so &_SEGGER_RTT == ORIGIN(RAM) = 0x20000000) and the up/down
  # ring buffers to .rtt_buf (placed right after). Two sections so the
  # control-block address stays fixed even if the buffer sizes change.
  # Lets cortex-debug use a literal "address": "0x20000000" with no scanning.
  "SEGGER_RTT_SECTION=\".rtt_cb\""
  "SEGGER_RTT_BUFFER_SECTION=\".rtt_buf\""
  "DAP_FW_V1"
  "DAP_FW_V2"
  "DAP_CDC"
)
