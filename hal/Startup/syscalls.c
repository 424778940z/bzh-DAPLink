/**
  * syscalls.c - newlib lowlevel stubs for embedded targets.
  *
  * Adapted from STM32CubeMX's auto-generated syscalls (CC0/permissive). Layout
  * mirrors onekey-firmware-pro2/sys/sys_api/syscalls.c so the newlib hooks each
  * have one canonical home. _sbrk lives in sysmem.c next to this file. _write
  * is wired to SEGGER RTT channel 0 -- when DEBUG_OUTPUT_RTT is selected the
  * banner from SEGGER_RTT_Init() is what comes out of stdout.
  */

#include <errno.h>
#include <reent.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/times.h>
#include <time.h>

#include "SEGGER_RTT.h"

extern int __io_putchar(int ch) __attribute__((weak));
extern int __io_getchar(void) __attribute__((weak));

char* __env[1] = {0};
char** environ = __env;

int _getpid(void)
{
    return 1;
}

int _kill(int pid, int sig)
{
    (void)pid;
    (void)sig;
    errno = EINVAL;
    return -1;
}

void _exit(int status)
{
    _kill(status, -1);
    while ( 1 )
    {
    }
}

__attribute__((weak)) int _read(int file, char* ptr, int len)
{
    (void)file;
    int idx;
    for ( idx = 0; idx < len; idx++ )
    {
        *ptr++ = __io_getchar();
    }
    return len;
}

int _write(int file, const void* ptr, size_t len)
{
    (void)file;
    SEGGER_RTT_Write(0, ptr, len);
    return (int)len;
}

int _write_r(struct _reent* r, int file, const void* ptr, size_t len)
{
    (void)r;
    (void)file;
    SEGGER_RTT_Write(0, ptr, len);
    return (int)len;
}

int _close(int file)
{
    (void)file;
    return -1;
}

int _fstat(int file, struct stat* st)
{
    (void)file;
    st->st_mode = S_IFCHR;
    return 0;
}

int _isatty(int file)
{
    (void)file;
    return 1;
}

int _lseek(int file, int ptr, int dir)
{
    (void)file;
    (void)ptr;
    (void)dir;
    return 0;
}

int _open(char* path, int flags, ...)
{
    (void)path;
    (void)flags;
    return -1;
}

int _wait(int* status)
{
    (void)status;
    errno = ECHILD;
    return -1;
}

int _unlink(char* name)
{
    (void)name;
    errno = ENOENT;
    return -1;
}

clock_t _times(struct tms* buf)
{
    (void)buf;
    return -1;
}

int _stat(const char* file, struct stat* st)
{
    (void)file;
    st->st_mode = S_IFCHR;
    return 0;
}

int _link(char* old, char* new)
{
    (void)old;
    (void)new;
    errno = EMLINK;
    return -1;
}

int _fork(void)
{
    errno = EAGAIN;
    return -1;
}

int _execve(char* name, char** argv, char** env)
{
    (void)name;
    (void)argv;
    (void)env;
    errno = ENOMEM;
    return -1;
}

/* Required because Reset_Handler -> __libc_init_array calls _init() / _fini().
 * We keep them empty: there are no global C++ ctors/dtors in this firmware,
 * and CMSIS-DAP / TinyUSB / HAL all initialise from main(). */
__attribute__((weak)) void _init(void)
{
}

__attribute__((weak)) void _fini(void)
{
}

/* libc interface — linker --wrap=__stack_chk_fail; ((used)) survives LTO. */
__attribute__((used, noreturn)) void __wrap___stack_chk_fail(void)
{
    __asm volatile("cpsid i" ::: "memory");
    while ( 1 )
    {
    }
}
