/*
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 * 
 * Copyright(c) 2023 Diego Asanza <f.asanza@gmail.com>
 */

#include <errno.h>
#include <sys/stat.h>

extern char __end__;
extern char __HeapLimit;

extern void sh_end( void );

static char *brk = &__end__;

void delay(int dly)
{
    while (dly--)
        ;
}

void SystemInit()
{
}

extern int sh_putchar(char buf);

int _write(int file, char *buf, int nbytes)
{
    while(nbytes--) {
        sh_putchar(*buf++);
    }
    return 0;
}

void *_sbrk(int nbytes)
{
    void *prev_brk;

    if (nbytes < 0)
    {
        /* Returning memory to the heap. */
        nbytes = -nbytes;
        if (brk - nbytes < &__end__)
        {
            prev_brk = (void *)-1;
        }
        else
        {
            prev_brk = brk;
            brk -= nbytes;
        }
    }
    else
    {
        /* Allocating memory from the heap. */
        if (&__HeapLimit - brk >= nbytes)
        {
            prev_brk = brk;
            brk += nbytes;
        }
        else
        {
            errno = ENOMEM;
            prev_brk = (void *)-1;
        }
    }

    return prev_brk;
}

int _close(int file)
{
    // errno = EBADF;

    return -1; /* Always fails */

} /* _close () */

int _fstat(int file,
           struct stat *st)
{
    st->st_mode = S_IFCHR;
    return 0;

} /* _fstat () */

int _isatty(int file)
{
    return 1;

} /* _isatty () */

int _lseek(int file,
           int offset,
           int whence)
{
    return 0;

} /* _lseek () */

int _read(int file,
          char *ptr,
          int len)
{
    return 0; /* EOF */

} /* _read () */

int _exit(int rc) {
    sh_end();
    return 0;
}

int _kill(int pid, int sig) {
    /* do nothing */
    return 0;
}

int _getpid( void ) {
    /* do nothing */
    return 0;
}
