/*
 * Supplemental stubs for POSIX functions that xpack newlib's -lnosys
 * and -lc do NOT provide, but which are referenced by libstdc++.a
 * (filesystem and threading support).
 *
 * These are declared weak so they don't conflict with -lnosys or -lc.
 */
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

/* ---- filesystem (from libstdc++ fs_ops.o) ---- */

int chdir(const char *path) __attribute__((weak));
int chdir(const char *path) { (void)path; errno = ENOSYS; return -1; }

int chmod(const char *path, mode_t mode) __attribute__((weak));
int chmod(const char *path, mode_t mode) { (void)path; (void)mode; errno = ENOSYS; return -1; }

char *getcwd(char *buf, size_t size) __attribute__((weak));
char *getcwd(char *buf, size_t size) { (void)buf; (void)size; errno = ENOSYS; return NULL; }

int mkdir(const char *path, mode_t mode) __attribute__((weak));
int mkdir(const char *path, mode_t mode) { (void)path; (void)mode; errno = ENOSYS; return -1; }

long pathconf(const char *path, int name) __attribute__((weak));
long pathconf(const char *path, int name) { (void)path; (void)name; errno = ENOSYS; return -1; }

/* ---- threading (from libstdc++ thread.o) ---- */

unsigned int sleep(unsigned int seconds) __attribute__((weak));
unsigned int sleep(unsigned int seconds) { (void)seconds; return 0; }

int usleep(useconds_t usec) __attribute__((weak));
int usleep(useconds_t usec) { (void)usec; return 0; }
