#include <sys/types.h>
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

i64 sys_stat() {
	return -1;
}

u64 sys_stat_stat_struct_len() {
	return sizeof(struct stat);
}
