/*
 * Stub implementations of POSIX syscalls needed when linking libstdc++
 * from xpack GCC 12.3.1 for bare-metal ARM.
 *
 * Provides everything that would normally come from -lnosys plus
 * additional syscalls that newlib's internal reentrant wrappers
 * reference (_fstat, _kill, _getpid, etc.) and libstdc++ filesystem
 * / threading references (chdir, mkdir, sleep, etc.).
 */

#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

/* ---- newlib reent syscalls (_-prefixed) ---- */

int _close(int fd)            { (void)fd; errno = EBADF; return -1; }
int _fstat(int fd, struct stat *st)
{
    (void)fd; (void)st;
    errno = EBADF;
    return -1;
}
int _getpid(void)             { return 1; }
int _isatty(int fd)           { (void)fd; return 0; }
int _kill(int pid, int sig)   { (void)pid; (void)sig; errno = EINVAL; return -1; }
int _link(const char *o, const char *n)
{
    (void)o; (void)n;
    errno = ENOSYS; return -1;
}
off_t _lseek(int fd, off_t off, int whence)
{
    (void)fd; (void)off; (void)whence;
    errno = EBADF; return -1;
}
int _open(const char *path, int flags, ...)
{
    (void)path; (void)flags;
    errno = ENOENT; return -1;
}
ssize_t _read(int fd, void *buf, size_t nbyte)
{
    (void)fd; (void)buf; (void)nbyte;
    return 0;
}
int _stat(const char *path, struct stat *st)
{
    (void)path; (void)st;
    errno = ENOENT; return -1;
}
int _times(struct tms *buf)   { (void)buf; return -1; }
int _unlink(const char *path) { (void)path; errno = ENOENT; return -1; }
ssize_t _write(int fd, const void *buf, size_t nbyte)
{
    (void)fd; (void)buf; (void)nbyte;
    return nbyte;
}
int _gettimeofday(struct timeval *tv, void *tz)
{
    (void)tv; (void)tz;
    errno = ENOSYS; return -1;
}
void *_sbrk(ptrdiff_t incr)
{
    extern char __heap_start__[];
    extern char __heap_end__[];
    static char *heap_ptr = NULL;
    if (heap_ptr == NULL) heap_ptr = __heap_start__;
    char *prev = heap_ptr;
    if (heap_ptr + incr > __heap_end__) {
        errno = ENOMEM;
        return (void *)-1;
    }
    heap_ptr += incr;
    return prev;
}

/* ---- filesystem (from libstdc++ fs_ops.o) ---- */

int chdir(const char *path)
{
    (void)path; errno = ENOSYS; return -1;
}
int chmod(const char *path, mode_t mode)
{
    (void)path; (void)mode; errno = ENOSYS; return -1;
}
char *getcwd(char *buf, size_t size)
{
    (void)buf; (void)size; errno = ENOSYS; return NULL;
}
int mkdir(const char *path, mode_t mode)
{
    (void)path; (void)mode; errno = ENOSYS; return -1;
}
long pathconf(const char *path, int name)
{
    (void)path; (void)name; errno = ENOSYS; return -1;
}

/* ---- threading (from libstdc++ thread.o) ---- */

unsigned int sleep(unsigned int seconds)
{
    (void)seconds; return 0;
}
int usleep(useconds_t usec)
{
    (void)usec; return 0;
}
