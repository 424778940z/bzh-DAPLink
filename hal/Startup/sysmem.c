/**
  * sysmem.c - newlib heap allocator (_sbrk).
  *
  * Layout mirror's onekey's: a single .heap region between bss and the MSP
  * stack reserve. Linker (hal/ld/link.ld) provides _sheap (start), _eheap
  * (end), _estack (top of stack), and STACK_SIZE (reserve below _estack).
  * The heap is bounded both by the .heap region and by (estack - STACK_SIZE)
  * so a runaway malloc cannot collide with the stack.
  */

#include <errno.h>
#include <stddef.h>
#include <stdint.h>

static uint8_t* __sbrk_heap_end = NULL;

void* _sbrk(ptrdiff_t incr)
{
    extern uint8_t _sheap;      /* heap start, defined in linker script */
    extern uint8_t _eheap;      /* heap end (region cap) */
    extern uint8_t _estack;     /* top of stack */
    extern uint32_t STACK_SIZE; /* MSP reserve below _estack */

    if ( __sbrk_heap_end == NULL )
    {
        __sbrk_heap_end = &_sheap;
    }

    const uintptr_t heap_start = (uintptr_t)&_sheap;
    const uintptr_t heap_limit_region = (uintptr_t)&_eheap;
    const uintptr_t heap_limit_stack = (uintptr_t)&_estack - (uintptr_t)&STACK_SIZE;
    const uintptr_t heap_limit = (heap_limit_region < heap_limit_stack) ? heap_limit_region : heap_limit_stack;

    const intptr_t new_heap_end = (intptr_t)(uintptr_t)__sbrk_heap_end + incr;

    if ( new_heap_end < (intptr_t)heap_start || new_heap_end > (intptr_t)heap_limit )
    {
        errno = ENOMEM;
        return (void*)-1;
    }

    uint8_t* prev_heap_end = __sbrk_heap_end;
    __sbrk_heap_end = (uint8_t*)(uintptr_t)new_heap_end;
    return (void*)prev_heap_end;
}
