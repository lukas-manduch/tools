#include <sys/types.h>

#define DEBUG 1
#define LOCAL static
#define NULL 0
#define FALSE 0
#define TRUE 1
#define STACK_SIZE 4096
#define HEAP_SIZE 4096
#define U32_MAX 0xffffffff
#define u64 unsigned long long
#define u32 unsigned int
#define i32 int
#define i64 signed long long

_Static_assert(sizeof(u64) == 8, "Bad size");
_Static_assert(sizeof(i64) == 8, "Bad size");
_Static_assert(sizeof(u32) == 4, "Bad size");
_Static_assert(sizeof(i32) == 4, "Bad size");

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
#if __STDC_VERSION__ != 201710L
#error "Bad c version, use std=c17"
#endif

struct AllocEntry {
	u32 size;
	unsigned char pad1;
	unsigned char pad2;
	unsigned char free; // Is following block free (1) or taken (0)?
	unsigned char next; // Is there next block ? (1) or is this the end (0)
};
_Static_assert(sizeof(struct AllocEntry) == 8, "Bad size AllocEntry");

struct context {
	char heap[HEAP_SIZE];

	char stack[STACK_SIZE];
	unsigned short _stack_pointer;
};

enum ExpressionType {
	SYMBOL   = 1 << 1, 
	STRING   = 1 << 2,
	CONS     = 1 << 3,
	NUMBER   = 1 << 4,
	FUNCTION = 1 << 5,
	NIL      = 1 << 6,
};

struct StringExpression {
	u32 size;
	char content[];
};


struct _listExpression {
	struct ConsCell* cell;
};

typedef struct ExpressionT {
	u32 expr_type;
	u32 pad;
	union {
		u64 value64;
		struct StringExpression value_string;
		struct _listExpression value_list;
	};
} Expression;

struct ConsCell {
	struct ExpressionT* car;
	struct ExpressionT* cdr;
};
static inline u64 round8(u64 num) {
	return num + (num % 8);
}

#if DEBUG == 0
#define DEBUG_ERROR(str) void()
#else
void __debug_print(const char* c) {
	u64 str_size = 0;
	while (c[str_size++] != 0)
		/**/;

	sys_write(0, c, str_size);
	sys_write(0, "\n", 1);
}

void print_cstring(const char *str) {
	u64 i = 0;
	while(str[i++] != 0) ;
	sys_write(0, str, i);
}

void print_expression(const struct ExpressionT* expression) {
	if (!expression) {
		print_cstring("[NULL PTR]");
		return;
	}
	switch(expression->expr_type) {
	case STRING:
		{
			u64 size = expression->value_string.size;
			sys_write(0, expression->value_string.content, size);
			break;

		}
	case CONS:
		{
			sys_write(0, "( ", 2);
			if (expression->value_list.cell != NULL) {
				struct ConsCell *curr = expression->value_list.cell;
				print_expression(curr->car);
				sys_write(0, " ", 1);
				print_expression(curr->cdr);

			}
			sys_write(0, " )", 2);
			break;
		}
	default:
		print_cstring("[NOT_IMPLEMENTED]");
	}

}

#define DEBUG_ERROR(str) __debug_print(str)
#endif


void* heap_alloc(struct context* context, u64 size) {
	u32 minimal_alloc = sizeof(struct AllocEntry) + 8;
	u32 requested = round8(size);
	struct AllocEntry* ret = NULL;
	struct AllocEntry *entry = (struct AllocEntry*)context->heap;
	while (TRUE) {
#ifdef DEBUG
		if (entry->pad1 != 0xda || entry->pad2 != 0xde) {
			DEBUG_ERROR("Invalid alloc entry");
			return NULL;
		}
#endif
		if (entry->free && entry->size >= requested) {
			break;
		}
		if (!entry->next) { // This was the last block
			DEBUG_ERROR("Memory is full");
			return NULL;
		}
		entry = (struct AllocEntry*)
			((char*)entry + sizeof(*entry) + entry->size);
	}
	// Chunk cannot be split
	if (entry->size <= requested + minimal_alloc) {
		entry->free = 0;
		ret = entry;
		return (char*)ret + sizeof(struct AllocEntry);
	}
	// We have to split chunk
	u32 rest = entry->size - requested - sizeof(struct AllocEntry);
	unsigned char has_next = entry->next;
	entry->free = 0;
	entry->size = requested;
	entry->next = 1;
	ret = entry;
	entry = (struct AllocEntry*)
		((char*)entry + sizeof(struct AllocEntry) + entry->size);
	// Now we have new entry
	entry->size = rest;
	entry->next = has_next;
	entry->free = 1;
	entry->pad1 = 0xda;
	entry->pad2 = 0xde;
	return (char*)ret + sizeof(struct AllocEntry);
}


LOCAL int init_heap(void* data, u64 size) {
	if (size <= sizeof(struct AllocEntry)) {
		DEBUG_ERROR("Heap is too small");
		return -1;
	}
	if (size > U32_MAX) {
		DEBUG_ERROR("32 bit heap only!");
		return -1;
	}
	// "Zero out" memory
	for (u64 i = 0; i < size; i++) {
		u64 order = i & 3;
		char* cdata = (char*)data;
		switch (order) {
			case 0:
				cdata[i] = 0xde;
				break;
			case 1:
				cdata[i] = 0xad;
				break;
			case 2:
				cdata[i] = 0xbe;
				break;
			case 3:
				cdata[i] = 0xef;
				break;
		}
	}
	struct AllocEntry* entry = (struct AllocEntry*) data;
	entry->size = size - sizeof(struct AllocEntry);
	entry->next = 0;
	entry->free = 1;
	entry->pad1 = 0xda;
	entry->pad2 = 0xde;
	return 0;
}

void init_context(struct context* ctx) {
	for (int i = 0; i < STACK_SIZE; i++)
		ctx->stack[i] = 0xAA;
	ctx->_stack_pointer = STACK_SIZE;
	init_heap(ctx->heap, HEAP_SIZE); 
}


LOCAL void* stack_push_var(struct context* ctx, u64 size) {
	if ((size % 8) != 0) {
		return NULL; // Unaligned alloc, not supported
	}
	if (ctx->_stack_pointer < size) {
		return NULL; // We are out of memory
	}

	ctx->_stack_pointer -= size;
	return &(ctx->stack[ctx->_stack_pointer]);
}

/** Push value on stack and return pointer to value
 */
void* stack_push_u64(struct context* ctx, u64 value) {
	u64* addr = stack_push_var(ctx, sizeof(value));
	if (addr == NULL)
		return NULL;
	*addr = value;
	return addr;
}


u32 c_strlen(const char* str) {
	u32 len = 0;
	while (str[len] != 0) len++;
	return len;
}

// PARSERS:

LOCAL i32 parser_find_next_char(const char* buf, u64 start_position, u64 count) {
	for (u64 i = start_position; i < count; i++) {
		switch(buf[i]) {
			case ' ':
			case '\t':
			case '\n':
				continue;
			default:
				return i;
		}
	}
	return -1;
}

Expression* parser_parse_string(
		const char* buf,
		u64 max_size,
		/* out */ u64* num_processed,
		struct context* ctx) {
	Expression *ret = NULL;
	if (max_size < 2 || buf[0] != '"') {
		DEBUG_ERROR("Wrong string to parse");
		return NULL;
	}
	u64 final_size = 0;
	u32 found_end = FALSE;
	// Calculate expected size after parsing
	// TODO: Skip over special symbols
	for (u64 i = 1; i < max_size; i++) {
		if (buf[i] == '"') {
			found_end = TRUE;
			break;
		}
		final_size++;
	}

	if (!found_end) {
		DEBUG_ERROR("String is not ended properly");
		return NULL;
	}

	ret = heap_alloc(ctx, sizeof(Expression) + final_size);
	if (!ret) {
		DEBUG_ERROR("Allocation failed");
		return ret;
	}

	ret->expr_type = STRING;


	// Copy string
	u64 processed = 1;
	for (u64 i = 1, dest = 0; i < max_size; i++) {
		processed++;
		if (buf[i] == '"') {
			// Now breaking here is guaranteed
			break;
		}
		ret->value_string.content[dest++] = buf[i];
	}
	ret->value_string.size = final_size;
	if (num_processed)
		*num_processed = processed;
	return ret;
}

struct ExpressionT* parser_parse_expression(const char* buf, u32 count, struct context* ctx, u64* out_processed) {
	Expression* ret = NULL;
	for (i64 i = 0; i < count || buf[i] != 0; i++) {
		char c = buf[i];
		if (c == '(') { // Begin of expression
			// int j = i+1;
			ret = heap_alloc(ctx, sizeof(Expression));
			if (!ret) {
				DEBUG_ERROR("Memory fail - parser");
				return NULL;
			}

			ret->expr_type = CONS;
			ret->value_list.cell = NULL;
			struct ConsCell* cons = (struct ConsCell*)
				heap_alloc(ctx, sizeof(struct ConsCell));
			if (!cons) {
				DEBUG_ERROR("Cons allocation failed");
				return NULL;
			}

			cons->car = NULL;
			cons->cdr = NULL;
			ret->value_list.cell = cons;
			
			while (1) { // Load everythin inside expression
				i = parser_find_next_char(buf,i+1, count);
				if (i < 0) {
					// TODO: Free memory
					DEBUG_ERROR("Cannot find closing brace");
					return ret;
				}
				if (buf[i] == ')') { // Parsing finished
					return ret;
				}

				u64 skip = 0;
				Expression* es = parser_parse_expression(buf+i, count - i, ctx, &skip);
				i += skip - 1;
				struct ConsCell *cons = heap_alloc(ctx, sizeof(struct ConsCell));
				if (!cons) {
					DEBUG_ERROR("Cannot allocate cons");
					return NULL;
				}
				cons->car = es;
				cons->cdr = NULL;
				if (ret->value_list.cell != NULL) {
					ret->value_list.cell = cons;
				}
			}
			DEBUG_ERROR("Unclosed expression");
			return NULL;
		} else if (c == '"')  {
			u64 string_size = 0;
			struct ExpressionT* es =
				parser_parse_string(buf+i, count-i, &string_size, ctx);
			if (es == NULL) {
				DEBUG_ERROR("Invalid string parse");
			}
			if (out_processed)
				*out_processed =  string_size + i;
			return es;
		}
		else {
			DEBUG_ERROR("Invalid character");
			return NULL;
		}
	}
	DEBUG_ERROR("Wrong");
	return NULL;
}


/** Parse string defined inside "". 
 * Return -1 on failure, parsed string size on success
 */
int _parser_parse_string(
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

// END PARSERS


void _start() {
	const char* command = "(\"some long string here\")";
	struct context ctx;
	init_context(&ctx);
	Expression* ex = parser_parse_expression(command, c_strlen(command), &ctx, NULL);
	print_expression(ex);
	//sys_write(0, ex->value_string.content, 8);
	sys_exit(17);
}
