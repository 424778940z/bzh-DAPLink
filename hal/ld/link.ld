/* STM32F411CE GCC linker script
 * Target: WeAct Studio MiniSTM32F4x1 (Black Pill)
 *   Flash: 512K @ 0x08000000
 *   RAM:   128K @ 0x20000000
 *
 * Stack at top of RAM (`_estack`), heap grows up from `_end` to `_heap_end`.
 * SEGGER RTT control block is pinned to the start of RAM via .rtt_cb so
 * cortex-debug / OpenOCD `rtt setup` can use a fixed address (0x20000000)
 * instead of scanning. SEGGER_RTT_SECTION=".rtt_cb" is set in
 * cmake/compile_options.cmake to land the control block in this section.
 */

ENTRY(Reset_Handler)

/* 8 KiB. TinyUSB tu_print_mem (CFG_TUSB_DEBUG=2) plus USB IRQ HPE push can
 * blow past 4 KiB on enumeration; cheap to double on a 128 KiB part.
 * Exposed as STACK_SIZE so hal/Startup/sysmem.c can clamp _sbrk against
 * the MSP reserve (heap_limit_by_stack = _estack - STACK_SIZE). */
__stack_size = 8192;
PROVIDE(STACK_SIZE = __stack_size);

_estack = ORIGIN(RAM) + LENGTH(RAM);

MEMORY
{
  FLASH (rx)  : ORIGIN = 0x08000000, LENGTH = 512K
  RAM   (xrw) : ORIGIN = 0x20000000, LENGTH = 128K
}

SECTIONS
{
  .isr_vector :
  {
    . = ALIGN(4);
    KEEP(*(.isr_vector))
    . = ALIGN(4);
  } >FLASH

  .text :
  {
    . = ALIGN(4);
    *(.text)
    *(.text*)
    *(.glue_7)
    *(.glue_7t)
    *(.eh_frame)
    KEEP(*(.init))
    KEEP(*(.fini))
    . = ALIGN(4);
    _etext = .;
  } >FLASH

  .rodata :
  {
    . = ALIGN(4);
    *(.rodata)
    *(.rodata*)
    . = ALIGN(4);
  } >FLASH

  .ARM.extab :
  {
    . = ALIGN(4);
    *(.ARM.extab* .gnu.linkonce.armextab.*)
    . = ALIGN(4);
  } >FLASH

  .ARM :
  {
    . = ALIGN(4);
    __exidx_start = .;
    *(.ARM.exidx*)
    __exidx_end = .;
    . = ALIGN(4);
  } >FLASH

  .preinit_array :
  {
    . = ALIGN(4);
    PROVIDE_HIDDEN(__preinit_array_start = .);
    KEEP(*(.preinit_array*))
    PROVIDE_HIDDEN(__preinit_array_end = .);
    . = ALIGN(4);
  } >FLASH

  .init_array :
  {
    . = ALIGN(4);
    PROVIDE_HIDDEN(__init_array_start = .);
    KEEP(*(SORT(.init_array.*)))
    KEEP(*(.init_array*))
    PROVIDE_HIDDEN(__init_array_end = .);
    . = ALIGN(4);
  } >FLASH

  .fini_array :
  {
    . = ALIGN(4);
    PROVIDE_HIDDEN(__fini_array_start = .);
    KEEP(*(SORT(.fini_array.*)))
    KEEP(*(.fini_array*))
    PROVIDE_HIDDEN(__fini_array_end = .);
    . = ALIGN(4);
  } >FLASH

  /* SEGGER RTT control block pinned to ORIGIN(RAM) so cortex-debug uses a
   * fixed `rttConfig.address: "0x20000000"` -- no probe-side scanning.
   * NOLOAD: not in flash image, not zero-initialised by C startup;
   * SEGGER_RTT_Init() writes the magic ID at runtime. */
  .rtt_cb (NOLOAD) :
  {
    . = ALIGN(4);
    PROVIDE(_rtt_cb_start = .);
    KEEP(*(.rtt_cb))
    KEEP(*(.rtt_cb.*))
    . = ALIGN(4);
    PROVIDE(_rtt_cb_end = .);
  } >RAM

  _sidata = LOADADDR(.data);

  .data :
  {
    . = ALIGN(4);
    _sdata = .;
    *(.data)
    *(.data*)
    *(.RamFunc)
    *(.RamFunc*)
    . = ALIGN(4);
    _edata = .;
  } >RAM AT>FLASH

  .bss :
  {
    . = ALIGN(4);
    _sbss = .;
    __bss_start__ = _sbss;
    *(.bss)
    *(.bss*)
    *(COMMON)
    . = ALIGN(4);
    _ebss = .;
    __bss_end__ = _ebss;
  } >RAM

  /* Heap region for hal/Startup/sysmem.c:_sbrk -- spans from end-of-bss to
   * the start of the MSP stack reserve (top of RAM minus STACK_SIZE). */
  PROVIDE(_sheap = _ebss);
  PROVIDE(_end = _ebss);    /* legacy alias kept for any newlib hook that
                               still expects the traditional symbol name. */
  PROVIDE(end = _ebss);

  .stack ORIGIN(RAM) + LENGTH(RAM) - __stack_size :
  {
    PROVIDE(_eheap = .);
    . = ALIGN(8);
    . = . + __stack_size;
  } >RAM

  /DISCARD/ :
  {
    libc.a (*)
    libm.a (*)
    libgcc.a (*)
  }

  .ARM.attributes 0 : { *(.ARM.attributes) }
}
