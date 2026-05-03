
set(PROJ_WIDE_COMMON_FLAGS
  # MCU: STM32F411CEU6 = Cortex-M4F (ARMv7E-M, single-precision FPU)
  -mthumb
  -mcpu=cortex-m4
  -mfloat-abi=hard
  -mfpu=fpv4-sp-d16
  -mtune=cortex-m4

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

  # Own __stack_chk_fail (in syscalls.c). Without the wrap, the linker pulls
  # libc_a-stack_protector.o out of libc_nano.a, which transitively references
  # _exit/_kill/_getpid; under -flto=auto the LTO plugin then drops our
  # syscalls.c definitions of those, leaving them undefined at final link.
  -Wl,--wrap=__stack_chk_fail
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
  "STM32F411xE"
  "USE_HAL_DRIVER"
  "HSE_VALUE=25000000U"      # WeAct Black Pill V3.x ships a 25 MHz Y2 HSE crystal.
  # DAP_CDC and DEBUG_OUTPUT_UART both want USART1 -- pick at most one.
  # Compile-time conflict check lives in src/uart_conflict_checker.c.
  # $<$<CONFIG:Debug>:"DEBUG_OUTPUT=DEBUG_OUTPUT_UART">
  "$<$<CONFIG:Debug>:DEBUG_OUTPUT=DEBUG_OUTPUT_RTT>"
  "$<$<CONFIG:Release>:DEBUG_OUTPUT=DEBUG_OUTPUT_NONE>"
  # Pin the SEGGER RTT control block to .rtt_cb (placed first in RAM by
  # hal/ld/link.ld, so &_SEGGER_RTT == ORIGIN(RAM) = 0x20000000) and the up/down
  # ring buffers to .rtt_buf (placed right after). Two sections so the
  # control-block address stays fixed even if the buffer sizes change.
  # Lets cortex-debug use a literal "address": "0x20000000" with no scanning.
  "SEGGER_RTT_SECTION=\".rtt_cb\""
  "SEGGER_RTT_BUFFER_SECTION=\".rtt_buf\""
  # STM32F411 OTG_FS only has 4 IN endpoints (including EP0): EP 0/1/2/3.
  # Each function below claims the following endpoint slots:
  #   DAP_FW_V1 (HID DAP v1) -> 1 IN + 1 OUT
  #   DAP_FW_V2 (Vendor DAP v2 / WinUSB) -> 1 IN + 1 OUT
  #   DAP_CDC   (USB CDC ACM bridged to USART1) -> 1 notif IN + 1 data IN + 1 data OUT
  # Enabling all three needs 4 IN endpoints + EP0 = 5 IN slots, which busts
  # the F411 hardware limit and makes TinyUSB STALL SET_CONFIGURATION inside
  # dcd_edpt_open. Pick at most TWO of the three. The compile-time check in
  # src/dap_config_checker.c enforces this. Recommended default: V2 + CDC
  # (V2 = OpenOCD/pyOCD prefer bulk, CDC = USART1 bridge to /dev/ttyACMx).
  # Other valid combos: {V1+V2}, {V1+CDC}.
  "DAP_FW_V1"
  # "DAP_FW_V2"
  "DAP_CDC"
)
