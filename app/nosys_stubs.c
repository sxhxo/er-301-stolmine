/*
 * Stub implementations of POSIX / newlib syscalls.
 *
 * xpack GCC 12.3.1's libstdc++.a internally references filesystem
 * operations (chmod, chdir, mkdir, getcwd, pathconf) and threading
 * helpers (sleep, usleep).  The ER-301 runs bare-metal with no OS,
 * so these are never called; the linker just needs the symbols.
 *
 * We provide all stubs ourselves so we don't depend on -lnosys
 * (which xpack's newlib may not ship, or may conflict with).
 */

#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

/* ---- filesystem (from libstdc++ fs_ops.o) ---- */

int chmod(const char *path, mode_t mode)
{
    (void)path; (void)mode;
    errno = ENOSYS;
    return -1;
}

int chdir(const char *path)
{
    (void)path;
    errno = ENOSYS;
    return -1;
}

int mkdir(const char *path, mode_t mode)
{
    (void)path; (void)mode;
    errno = ENOSYS;
    return -1;
}

long pathconf(const char *path, int name)
{
    (void)path; (void)name;
    errno = ENOSYS;
    return -1;
}

char *getcwd(char *buf, size_t size)
{
    (void)buf; (void)size;
    errno = ENOSYS;
    return NULL;
}

/* ---- threading (from libstdc++ thread.o) ---- */

unsigned int sleep(unsigned int seconds)
{
    (void)seconds;
    return 0;
}

int usleep(useconds_t usec)
{
    (void)usec;
    return 0;
}
