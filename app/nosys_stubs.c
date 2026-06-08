/*
 * Stub implementations of POSIX syscalls needed by xpack GCC 12.3.1's
 * libstdc++ filesystem support (fs_ops.o).
 *
 * These are only referenced when the linker pulls in filesystem-related
 * object files from libstdc++ (due to some C++ header or inline using
 * std::filesystem internally).  The ER-301 firmware does not use the
 * filesystem library at runtime, so stubs returning -1/ENOSYS are safe.
 */

#include <errno.h>
#include <sys/stat.h>
#include <unistd.h>

int chmod(const char *path, mode_t mode)
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
