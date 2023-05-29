#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "../include/defines.h"


void sys_exit(i32 error_code) {
	__asm__ (
		"movq $60, %%rax\n\t"
		"movl %0, %%edi\n\t"
		"syscall"
		: // empty
		: "rm" (error_code)
		);
}

#define X86SYSCALL3(number, arg1, arg2, arg3) void

i64 sys_write(u32 fd, const char *buf, u64 count) {
	i64 ret = 0;
 	__asm__ volatile (
 	        "movq $1, %%rax\n\t"
 	        "movl %1, %%edi\n\t"
 	        "movq %2, %%rsi\n\t"
 	        "movq %3, %%rdx\n\t"
		"syscall\n\t"
		"movq %%rax, %0 \n\t"
 		 : "=rm" (ret)
 		 : "rm"  (fd), "rm" (buf), "rm" (count)
 		 : "rax" , "rdi", "rsi", "rdx" /* These two idk */, "r11", "rcx"
 	       );
	return ret;
}

i64 sys_read(u32 fd, const char *buf, u64 count) {
	i64 ret = 0;
 	__asm__ volatile (
 	        "movq $0, %%rax\n\t"
 	        "movl %1, %%edi\n\t"
 	        "movq %2, %%rsi\n\t"
 	        "movq %3, %%rdx\n\t"
		"syscall\n\t"
		"movq %%rax, %0 \n\t"
 		 : "=rm" (ret)
 		 : "rm"  (fd), "rm" (buf), "rm" (count)
 		 : "rax" , "rdi", "rsi", "rdx" /* These two idk */, "r11", "rcx"
 	       );
	return ret;
}

/** 0 on sucess negative on failure
 */
i64 sys_stat(const char* filename, void* statbuf) {
	i64 ret = 0;
 	__asm__ volatile (
 	        "movq $4, %%rax\n\t"
 	        "movq %1, %%rdi\n\t"
 	        "movq %2, %%rsi\n\t"
		"syscall\n\t"
		"movq %%rax, %0 \n\t"
 		 : "=rm" (ret)
 		 : "rm"  (filename), "rm" (statbuf)
 		 : "rax" , "rdi", "rsi" /* These two idk */, "r11", "rcx"
 	       );
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
 	__asm__ volatile (
 	        "movq $2, %%rax\n\t"
 	        "movq %1, %%rdi\n\t"
 	        "movl %2, %%esi\n\t"
 	        "movl %3, %%edx\n\t"
		"syscall\n\t"
		"movq %%rax, %0 \n\t"
 		 : "=rm" (ret)
 		 : "rm"  (filename), "rm" (flags), "rm" (mode)
 		 : "rax" , "rdi", "rsi", "rdx" /* These two idk */, "r11", "rcx"
 	       );
	return ret;
}

i32 sys_open_flag_read() {
	return O_RDONLY;
}

i64 sys_close(u32 fd) {
	i64 ret = 0;
 	__asm__ volatile (
 	        "movq $3, %%rax\n\t"
 	        "movl %1, %%edi\n\t"
		"syscall\n\t"
		"movq %%rax, %0 \n\t"
 		 : "=rm" (ret)
 		 : "rm"  (fd)
 		 : "rax" , "rdi" /* These two idk */, "r11", "rcx"
 	       );
	return ret;
}

/** Return pointer to 0 in stack, that is pushed before entry point call.
 * This climbs down base pointers, so this must be turned on.
 */
void* _platform_stack_bot() {
	u64* ret;
	__asm__ volatile (
			"movq (%%rbp), %%rax\n\t"
			"leaq (%%rbp), %%rcx\n\t"
			"1:cmpq $0, %%rax\n\t"
			"je 2f\n\t"
			"leaq (%%rax), %%rcx\n\t"
			"movq (%%rax), %%rax\n\t"
			"jmp 1b\n\t"
			"2:movq %%rcx, %0 \n\t"
			: "=rm" (ret)
			:
			: "rax", "rcx"
			);
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
