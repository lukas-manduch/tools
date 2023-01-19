#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>

#define STACK_SIZE 4096
#define u64 unsigned long long
#define i64 signed long long

_Static_assert(sizeof(u64) == 8, "Bad size");
_Static_assert(sizeof(i64) == 8, "Bad size");

#if __STDC_VERSION__ != 201710L
#error "Bad c version, use std=c17"
#endif

struct context {
	char heap[STACK_SIZE];

	char stack[STACK_SIZE];
	unsigned short _stack_pointer;
};

void init_context(struct context* ctx) {
	for (int i = 0; i < STACK_SIZE; i++)
		ctx->stack[i] = 0xFF;
	ctx->_stack_pointer = STACK_SIZE;
}

struct Expression {

};

void* stack_push(struct context* ctx, u64 value) {

}

void* heap_alloc(struct context* context, u64 size) {

}

i64 sys_read(u64 fd, const char *buf, size_t count) {
	i64 ret = 0;
 	__asm__ volatile (
 	        "movq $0, %%rax\n\t"
 	        "movq %1, %%rdi\n\t"
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



void parse_expression(const char* buf, u64 count, struct context ctx) {
	for (int i = 0; i < count || buf[i] != 0; i++) {
		if (i == )

	}

}

/** Parse string defined inside "". 
 * Return -1 on failure, parsed string size on success
 */
int parse_string(
	const char* src, size_t max_size_src, char* dst, size_t max_size_dst) {
	//--
	if (max_size_src < 2 || src[0] != '"') {
		// String doesn't start with braces
		return -1;
	}
	size_t dest_write = 0;

	for (size_t i = 1; i < max_size_src && dest_write < max_size_dst; i++) {
		if (src[i] == '"') {
			goto good;
		}

		if(src[i] == '\\') { // Currently not supported
			return -1;
		}
		dst[dest_write] = src[i];
	}
	// this is bad
	return -1;
good:
	return dest_write;
}

int execute(const char* program, char* return_value, size_t size){

}


int main() {
	char command[2048] = {'a', 'h', 'o', 0};
	sys_read(1, command, 2048);
	command[2047] = '\0';
	printf("%s\n", command);
	//execute(command, )

}
