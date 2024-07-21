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

/** Return pointer to 0 in stack, that is pushed before entry point call.
 * This climbs down base pointers, so this must be turned on.
 */
void* _platform_stack_bot() {
	u64* ret = 0;
	return ret;
}

/** Get argc from bottom of the stack.
 * This will only work, if compiled program uses ebp
 */
u64 platform_get_argc() {
	u64* stack_bot = _platform_stack_bot();
	stack_bot++;
	return *stack_bot;
}

/** Get arg[index] from bottom of the stack.
 * This will only work, if compiled program uses ebp
 */
char* platform_get_argv(u32 index) {
	u64* stack_bot = _platform_stack_bot();
	stack_bot++;
	stack_bot++;
	while (index--)
		stack_bot++;
	return (char*)(*stack_bot);
}
