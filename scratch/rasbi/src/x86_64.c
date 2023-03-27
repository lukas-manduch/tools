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
