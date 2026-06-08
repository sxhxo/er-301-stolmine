/*
 * Stub implementations of POSIX syscalls needed by xpack GCC 12.3.1's
 * libstdc++ filesystem and threading support (fs_ops.o, thread.o).
 *
 * The ER-301 firmware runs bare-metal with no OS — these functions are
 * never called at runtime; the linker only needs them to resolve
 * references from libstdc++ internal objects that get pulled in.
 *
 * NOTE: chmod/chdir/mkdir/getcwd/pathconf are NOT provided here because
 * -lnosys already provides them.  We only add what -lnosys doesn't cover.
 * If -lnosys is not linked, uncomment the full set below.
 */

#include <errno.h>
#include <sys/types.h>
#include <unistd.h>

/* Thread support: libstdc++ std::this_thread::__sleep_for */
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
