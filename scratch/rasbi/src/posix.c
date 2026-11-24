/** Raw posix syscalls exported as functions
 *
 * This file is included in compilations, where syscalls are done via linking
 * with standard library.  Other times syscalls may be implemented directly
 */

#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>

#define RASBI_NO_STD_REDEFINE
#include "../include/defines.h"

void sys_exit(i32 error_code) {
}

#define X86SYSCALL3(number, arg1, arg2, arg3) void

i64 sys_write(u32 fd, const char *buf, u64 count) {
	i64 ret = 0;
	return ret;
}

i64 sys_read(u32 fd, const char *buf, u64 count) {
	i64 ret = 0;
	return ret;
}

i64 sys_stat(const char* filename, void* statbuf) {
	i64 ret = 0;
	return ret;
}

u64 sys_stat_stat_struct_len() {
	return sizeof(struct stat);
}

u64 sys_stat_stat_get_size(void* statbuf) {
	return ((struct stat*)statbuf)->st_size;
}

/** Returns fd on success and -ERROR on failure
 */
i64 sys_open(const char *filename, i32 flags, i32 mode) {
	i64 ret = 0;
	return ret;
}

i32 sys_open_flag_read() {
	return O_RDONLY;
}

i64 sys_close(u32 fd) {
	i64 ret = 0;
	return ret;
}


/** Get argc if stdlib is used
 */
u64 platform_get_argc(int argc) {
	return argc;
}

/** Get argv[index] if stdlib is used
 */
char* platform_get_argv(char** argv, u32 index) {
	return argv[index];
}
