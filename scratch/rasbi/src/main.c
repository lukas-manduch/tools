#include <sys/types.h>

#define DEBUG 1
#define LOCAL static
#define INLINE static inline
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

// These two are really important to be inlined even in debug builds :)
__attribute__((always_inline)) static inline void sys_stack_push_u64(u64 value)  {
	__asm__ volatile ("push %0\n\t"
			:
			: "rm" (value));
}

__attribute__((always_inline)) static inline u64 sys_stack_pop_u64() {
	u64 result;
	__asm__ volatile ("pop %0\n\t"
			: "=m" (result)
			);
	return result;
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

enum ExpressionType {
	SYMBOL   = 1 << 1,
	STRING   = 1 << 2,
	CONS     = 1 << 3,
	MEMORY   = 1 << 4,
};

enum RuntimeErrors {
	RUNTIME_ARGUMENT_COUNT,
};

struct StringExpression {
	u32 size;
	char content[];
};

// TODO: This probably should be value, pointer is just more work

struct _listExpression {
	struct ConsCell* cell;
};

struct _memoryExpression {
	u32 size;
	u32 taken;
	char mem[];
};

typedef struct ExpressionT {
	u32 expr_type;
	u32 pad;
	union {
		u64 value64;
		struct StringExpression value_string;
		struct _listExpression value_list;
		struct _memoryExpression value_memory;
		struct StringExpression value_symbol; // Symbols are strings
	};
} Expression;

struct ConsCell {
	struct ExpressionT* car;
	struct ExpressionT* cdr;
};

/** This is whole state of parser and interpreter.
 */
struct context {
	char heap[HEAP_SIZE];

	char stack[STACK_SIZE];
	unsigned short _stack_pointer;
	struct ExpressionT* symbol_table; // This shall be CONS reference
	struct ExpressionT* program; // This shall be CONS reference
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
			sys_write(0, "\"", 1);
			sys_write(0, expression->value_string.content, size);
			sys_write(0, "\"", 1);
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
			sys_write(0, ")", 2);
			break;
		}
	case SYMBOL:
		{
			sys_write(0, expression->value_string.content, expression->value_string.size);
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
	ctx->symbol_table = NULL;
	ctx->program = NULL;
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

/** Get pointer to stack.
 * Useful to retrieve local variables or arguments.
 */
void* stack_get_ptr(struct context* ctx, i64 offset) {
	if ((offset % 8) != 0) {
		return NULL;
	}
	return &ctx->stack[ctx->_stack_pointer + offset];
}

// ============================
//   LIBRARY
// ============================

INLINE u32 c_strlen(const char* str) {
	u32 len = 0;
	while (str[len] != 0) len++;
	return len;
}

void c_strcpy_s(char *dest, u64 destsz, const char *src ) {
	for (u64 i = 0; i < destsz && src[i] != 0; i++) {
		dest[i] = src[i];
	}
}

i64 c_strcmp(const char* lhs, const char* rhs) {
	while (*lhs && (*lhs == *rhs)) {
		lhs++; rhs++;
	}
	return ((i64) *lhs) - ( (i64) *rhs);
}

INLINE i32 is_alphabet(char c) {
	if (c >= 'a' && c <= 'z')
		return TRUE;
	if (c >= 'A' && c <= 'Z')
		return TRUE;
	return FALSE;
}

INLINE i32 is_space(char c) {
	switch (c) {
		case ' ':
		case '\n':
		case '\t':
			return TRUE;
		default:
			return FALSE;
	}
}


/** Writes number in decimal notation to buffer.
 *  Returns number of characters written or -1 on small buffer
 */
i64 c_itoa10(i64 value, char* buffer, u64 buf_size) {
	// First compute number of digits. Idk how to write logarithms
	// without stdlib, so let's just be stupid
	i64 tmp_value = value;
	u64 digit_count = 0;
	while (tmp_value != 0) {
		tmp_value /= 10;
		digit_count++;
	}
	if (value < 0)
		digit_count++; // For minus sign

	if (digit_count == 0) // Handle 0
		digit_count = 1;

	if (digit_count > buf_size) {
		DEBUG_ERROR("Cannot convert number to string (bufsize)");
		return -1;
	}

	if (value == 0) {
		*buffer = '0';
		return 1;
	}

	if (value < 0) {
		*buffer = '-';
		value *= -1;
	}

	buffer += digit_count - 1;

	while (value != 0) {
		u64 reminder = value % 10;
		value /= 10;
		*buffer = reminder + '0';
		buffer--;
	}
	return digit_count;
}


// ============================
//   END LIBRARY
// ============================

// ============================
//   TYPES
// ============================

/** Allocate chunk of memory on heap.
 * Return pointer to new entry, or NULL on error.
 */
struct ExpressionT* type_mem_alloc(struct context* ctx, u32 size) {
	struct ExpressionT* new_expr  =
		heap_alloc(ctx, sizeof(struct ExpressionT) + size);
	if (new_expr == NULL) {
		DEBUG_ERROR("Cannot allocate memory chunk.");
		return NULL;
	}
	new_expr->expr_type = MEMORY;
	new_expr->value_memory.size = size;
	new_expr->value_memory.taken = 0;
	return new_expr;
}

/**  Get memory address of indexed element, or NULL.
 */
u64* type_mem_get_u64(struct ExpressionT* expr, u32 index) {
	const u32 value_size = sizeof(u64);
	u32 min_required = (index + 1) * value_size;
	if (expr->expr_type != MEMORY || expr->value_memory.taken < min_required) {
		return NULL;
	}
	return (u64*)&expr->value_memory.mem[index*value_size];
}

/** Push back to array on next free index.
 * Returns index which was taken, or -1 on full array.
 */
i64 type_mem_push_u64(struct ExpressionT* expr, u64 value) {
	const u32 value_size = sizeof(u64);
	i64 ret = -1;
	if (expr->expr_type != MEMORY
		|| (expr->value_memory.taken % value_size != 0)
		|| (expr->value_memory.taken+value_size) > expr->value_memory.size ) {
		//-----
		DEBUG_ERROR("Cannot push value");
		return ret;
	}

	ret = expr->value_memory.taken / value_size; // Index of inserted member

	void* address = &expr->value_memory.mem[expr->value_memory.taken];
	*(u64*)(address) = value;
	expr->value_memory.taken += value_size;
	return ret;
}

/** Get length of taken memory in bytes.
 * Returns number of used bytes or -1 on error
 */
i64 type_mem_get_len(struct ExpressionT* expr) {
	if(expr && expr->expr_type == MEMORY) {
		return expr->value_memory.taken;
	}
	return -1;
}

// i64 type_mem_memset(struct ExpressionT* expr, u64 length, unsigned char value) {
//
// }

// ============================
//   END TYPES
// ============================
/** Allocate empty string expression on context heap.
 *
 */
struct ExpressionT* alloc_string(struct context* ctx, u64 length) {
	struct ExpressionT* expr_str =
		heap_alloc(ctx, sizeof(struct ExpressionT) + length);
	if (!expr_str) {
		DEBUG_ERROR("String allocation failed");
		return NULL;
	}
	expr_str->expr_type = STRING;
	expr_str->value_string.size = length;
	return expr_str;
}

/** Allocate symbol expression on context.
 * For now, we are storing symbols as string. Only differenet thing is flag in
 * expression type
 */
struct ExpressionT* alloc_symbol(struct context* ctx, u64 length) {
	struct ExpressionT* ret = alloc_string(ctx, length);
	if (ret)
		ret->expr_type = SYMBOL;
	return ret;
}
// ============================
//   PARSERS
// ============================

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

/** Subparser for symbols (names)
 */
Expression* parser_parse_symbol(
		const char* buf,
		u64 max_size,
		/* out */ u64* num_processed,
		struct context* ctx) {
	u32 symbol_size = 0;
	for (; symbol_size < max_size; symbol_size++) {
		if (is_alphabet(buf[symbol_size]))
			continue;
		if (buf[symbol_size] == 0 || buf[symbol_size] == ')')
			break;
		if (is_space(buf[symbol_size]))
			break;
		DEBUG_ERROR("Invalid symbol");
		return NULL;
	}
	struct ExpressionT* ret = alloc_symbol(ctx, symbol_size);
	if (!ret) {
		DEBUG_ERROR("Symbol allocation failed");
		return ret; // NULL
	}
	c_strcpy_s(ret->value_string.content, symbol_size, buf);
	if (num_processed)
		*num_processed = symbol_size;
	return ret;
}

/** Subparser for strings
 *
 * In future it should support constructs like \n and \"
 */
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

/** Generic parser method, given string parses first expression until the end.
 *
 * Returns expression representing whole parsed code or NULL.
 * This method might call itself recursively
 */
struct ExpressionT* parser_parse_expression(const char* buf, u32 count, struct context* ctx, u64* out_processed) {
	Expression* ret = NULL;
	for (i64 i = 0; i < count || buf[i] != 0; i++) {
		char c = buf[i];
		if (c == '(') { // This will be new list
			// int j = i+1;
			ret = heap_alloc(ctx, sizeof(Expression));
			if (!ret) {
				DEBUG_ERROR("Memory fail - parser");
				return NULL;
			}

			ret->expr_type = CONS;
			ret->value_list.cell = NULL;

			while (1) { // Load everythin inside expression
				i = parser_find_next_char(buf,i+1, count);
				if (i < 0) {
					// TODO: Free memory
					DEBUG_ERROR("Cannot find closing brace");
					return ret;
				}
				if (buf[i] == ')') { // Parsing finished
					if (out_processed)
						*out_processed = i+1;
					return ret;
				}

				u64 skip = 0;
				Expression* es = parser_parse_expression(buf+i, count - i, ctx, &skip);
				if (!es) {
					DEBUG_ERROR("Invalid expression");
					return NULL;
				}
				i += skip - 1;
				struct ConsCell *cons = heap_alloc(ctx, sizeof(struct ConsCell));
				if (!cons) {
					DEBUG_ERROR("Cannot allocate cons");
					return NULL;
				}
				cons->car = es;
				cons->cdr = NULL;
				Expression* consExpr = heap_alloc(ctx, sizeof(struct ExpressionT));
				if (!consExpr) {
					DEBUG_ERROR("Cons wapper alloc failed");
					return NULL;
				}
				consExpr->expr_type = CONS;
				consExpr->value_list.cell = cons;
				if (ret->value_list.cell == NULL) {
					ret->value_list.cell = cons;
				} else {
					struct ConsCell* cur = ret->value_list.cell;
					while (cur->cdr != NULL)
						cur = cur->cdr->value_list.cell;
					cur->cdr = consExpr;
				}
			}
			DEBUG_ERROR("Unclosed expression");
			return NULL;
		} else if (c == '"')  { // Parse string
			u64 string_size = 0;
			struct ExpressionT* es =
				parser_parse_string(buf+i, count-i, &string_size, ctx);
			if (es == NULL) {
				DEBUG_ERROR("Invalid string parse");
				return NULL;
			}
			if (out_processed)
				*out_processed =  string_size + i;
			return es;
		}
		else if (is_alphabet(c)) { // Parse symbol
			u64 string_size = 0;
			Expression* expr = parser_parse_symbol(buf+i, count-i, &string_size, ctx);
			if (!expr) {
				DEBUG_ERROR("Cannot parse symbol");
				return NULL;
			}
			if (out_processed)
				*out_processed =  string_size + i;
			return expr;
		}
		else {
			DEBUG_ERROR("Invalid character");
			return NULL;
		}
	}
	DEBUG_ERROR("Wrong");
	return NULL;
}

/** Main parser method, used to call parser from outside
 *  All structure are allocated on context heap
 */
int parse_program(struct context* ctx, const char* buf, u64 size) {
	Expression *exp = parser_parse_expression(buf, size, ctx, NULL);
	if  (!exp) {
		return -1;
	}
	ctx->program = exp;
	return 0;
}

// ============================
//   END PARSERS
// ============================

INLINE struct ExpressionT* list_get_cdr(struct ExpressionT* curr) {
	if (curr && curr->expr_type == CONS) {
		return curr->value_list.cell->cdr;
	}
	return NULL;
}

INLINE struct ExpressionT* list_get_car(struct ExpressionT* curr) {
	if (curr && curr->expr_type == CONS && curr->value_list.cell->car) {
		return curr->value_list.cell->car;
	}
	return NULL;
}

// ============================
//   INTERPRETER
// ============================

INLINE u64 interpreter_get_arg_count(struct context* ctx) {
	return *(u64*)stack_get_ptr(ctx, 0);
}


/** Get argument from stack
 * arg_pos is order of argument that function will retrieve. Indexing starts
 * at 1
 */
INLINE struct ExpressionT* interpreter_get_arg(struct context* ctx, short arg_pos) {
	u64* arg_count = (u64*)stack_get_ptr(ctx, 0);
	if (!arg_count || *arg_count == 0) {
		DEBUG_ERROR("Abort, stack return NULL");
		return NULL; // TMP
	}
	u64 position = (*arg_count + 1) * 8 - arg_pos*8;
	struct ExpressionT** exp =
		(struct ExpressionT**)stack_get_ptr(ctx, position);
	if (!exp) {
		DEBUG_ERROR("Abort! stack get returns NULL");
		return NULL; // Temporary
	}
	return *exp;
}

int interpreter_push_args(struct context* ctx, struct ExpressionT* rest) {
	u64 arg_count = 0;
	while (rest) {
		struct ExpressionT * val = list_get_car(rest);
		if (!val) {
			DEBUG_ERROR("Invalid function call, (parser error)");
			return -1;
		}
		void *stackp = stack_push_u64(ctx, (u64)val);
		if (!stackp) {
			DEBUG_ERROR("Stack push failed");
			return -1;
		}
		rest = list_get_cdr(rest);
		arg_count++;
	}
	stack_push_u64(ctx, arg_count);
	return 0;
}


/** Check if given expression is function call.
 *  Returns either TRUE or FALSE.
 */
i64 interpreter_is_expr_function_call(struct ExpressionT* expr) {
	if (!expr || expr->expr_type != CONS) {
		return FALSE;
	}
	struct ExpressionT* first = list_get_car(expr);
	if (first && first->expr_type == SYMBOL) {
		return TRUE;
	}
	return FALSE;
}


/** Get nested function call pointer.
 * Positions are indexed from 1. Position 2 means - try to find second function
 * call and return its pointer.
 *
 * Returns pointer to function call or NULL
 */
struct ExpressionT* interpreter_get_nested_func(
		struct ExpressionT* expr, u64 position) {
	if (!expr || expr->expr_type != CONS) {
		DEBUG_ERROR("Expression is not a list (get nested func)");
		return NULL;
	}
	u64 found_count = 0;
	struct ExpressionT* current = (expr);
	while (current) {
		struct ExpressionT* value = list_get_car(current);
		if (interpreter_is_expr_function_call(value)) {
			found_count++;
			if (found_count == position) {
				return value;
			}
		}
		current = list_get_cdr(current);
	}
	return NULL;
}

void runtime_concat(struct context* ctx); // TMP

u64 _interpreter_count_expr_nodes_internal(struct ExpressionT* expr) {
	if (expr->expr_type != CONS) {
		return 0;
	}
	struct ExpressionT* car = list_get_car(expr);
	if (car->expr_type != SYMBOL) {
		DEBUG_ERROR("Parsing some list - unsupported yet");
		return 0;
	}

	struct ExpressionT* cons_expr = list_get_cdr(expr);
	u64 total_children = 0;
	while (cons_expr) {
		total_children += _interpreter_count_expr_nodes_internal(
			list_get_car(cons_expr));
		cons_expr = list_get_cdr(cons_expr);
	}
	return total_children + 1;
}

/** Return count of function calls in expression and it's children.
 * This function is recursive.
 */
u64 interpreter_count_expr_nodes(struct context* ctx) {
	if (!ctx->program) {
		DEBUG_ERROR("Program not parsed");
		return 0;
	}
	return _interpreter_count_expr_nodes_internal(ctx->program);
}

int interpreter_call_function(struct context* ctx, struct ExpressionT* expr) {
	struct ExpressionT* func = list_get_car(expr);
	if (!func) {
		DEBUG_ERROR("Func error");
		return -1;
	}
	if (func->expr_type == SYMBOL) {
		DEBUG_ERROR("Executing function");
		sys_write(1, func->value_symbol.content, func->value_symbol.size);
	}

	stack_push_u64(ctx, 0); // Space for return value

	if (interpreter_push_args(ctx, list_get_cdr(expr)) != 0) {
		return -1;
	}

	// Let's assume its concat
	runtime_concat(ctx);
	return 0;
}


struct ExpressionT* interpreter_build_parent_graph(struct context* ctx) {
	u64 node_count = interpreter_count_expr_nodes(ctx);
	if (!node_count) {
		DEBUG_ERROR("Parent graph err");
		return NULL;
	}
	struct ExpressionT* graph = type_mem_alloc(
		ctx, node_count*sizeof(struct ExpressionT*)); // pointer size

	type_mem_push_u64(graph, 0); // Parent of first is 0

	struct ExpressionT* current = NULL;
	struct ExpressionT* tmp = NULL;
	u64 position = 0;
	i64 push_ret = -1;

	sys_stack_push_u64(NULL);
	sys_stack_push_u64(NULL);

	sys_stack_push_u64((u64)ctx->program);
	sys_stack_push_u64(0);

	while(1) {
		// Stack shouldn't be used here
		position = sys_stack_pop_u64();
		current = (struct ExpressionT*)sys_stack_pop_u64();

		if (current == NULL && position == 0) { // Everything is done
			break;
		}

		tmp = interpreter_get_nested_func(current, ++position);
		if (tmp == NULL) { // There is nowhere to dive deeper, so
				   // let's go one up
			continue;
		}

		push_ret = type_mem_push_u64(graph, (u64)current); // We are pushing parent
		if (push_ret == -1) {
			DEBUG_ERROR("Cannot build graph (maybe cleanup)");
			return NULL;
		}

		// We found next function
		// First preserve current position
		sys_stack_push_u64((u64)current);
		sys_stack_push_u64(position);
		// Now set up for next iteration
		sys_stack_push_u64((u64)tmp);
		sys_stack_push_u64(0);
	}

	return graph;
}


int execute(struct context* ctx) {
	if(!ctx->program) {
		return -1;
	}

	struct ExpressionT* parents = interpreter_build_parent_graph(ctx);
	if (!parents) {
		return -1;
	}
	u64 current_function;
	return interpreter_call_function(ctx, ctx->program);
}

// ============================
//   END INTERPRETER
// ============================


// ============================
//   RUNTIME FUNCTIONS
// ============================


// struct Expression* _copy_string_heap(
// 		struct context* ctx, const char* str, u64 str_size) {
//
// }
//
// LOCAL struct Expression* runtime_get_error(struct context* ctx, u64 number) {
// 	switch (number) {
// 		case RUNTIME_ARGUMENT_COUNT:
// 			{
// 				break;
// 			}
// 		default:
// 			return NULL;
// 	}
// }

void runtime_concat(struct context* ctx) {
	u64 argcount = interpreter_get_arg_count(ctx);
	if (argcount != 2) {
		DEBUG_ERROR("Wrong arg count, fixme message");
		return;
	}
	struct ExpressionT *arg1, *arg2;
	arg1 = interpreter_get_arg(ctx, 1);
	arg2 = interpreter_get_arg(ctx, 2);
	if (arg1->expr_type != STRING || arg2->expr_type != STRING) {
		DEBUG_ERROR("Fail here, bad argument types");
		return;
	}
	u64 final_size = arg1->value_string.size + arg2->value_string.size;
	struct ExpressionT* result = alloc_string(ctx, final_size);
	if (!result) {
		DEBUG_ERROR("Fail here, cannot allocate fixme");
		return;
	}
	c_strcpy_s(result->value_string.content,
		arg1->value_string.size, arg1->value_string.content);
	c_strcpy_s(result->value_string.content+arg1->value_string.size,
		arg2->value_string.size, arg2->value_string.content);

	sys_write(1, result->value_string.content, result->value_string.size);


}

/** Get the length of linked list.
 * Returns length on success, -1 on failure.
 */
i64 runtime_list_length(struct ExpressionT* first) {
	i64 list_length = 0;
	while (first && first->value_list.cell) {
		if (first->expr_type != CONS) { // This list is bad ... error
			DEBUG_ERROR("Bad list to count");
			return -1;
		}
		if (first->value_list.cell->car == NULL) { // Weird but ok probably
			DEBUG_ERROR("Empty cons cell ll");
			return list_length;
		}
		list_length++;
		first = first->value_list.cell->cdr;

	}
	return list_length;
}

// ============================
//   END RUNTIME FUNCTIONS
// ============================


#if DEBUG == 1
#include "src/tests.c"
#endif

void _start() {
#if DEBUG == 1
	run_tests();
#endif
	//const char* command = "(concat \"some long string here\"  \"\" \" And this one \" yolo)";
	const char* command = "( concat \"some string \" (concat \"other\" \" string\" ) )";
	struct context ctx;
	init_context(&ctx);
	if (parse_program(&ctx, command, c_strlen(command)) < 0) {
		const char* err_parse = "Cannot parse program";
		sys_write(1, err_parse, c_strlen(err_parse));
		sys_exit(1);
	}
	if (execute(&ctx) < 0) {
		const char* err_exec  = "Cannot execute program";
		sys_write(1, err_exec , c_strlen(err_exec ));
		sys_exit(1);

	}
	sys_exit(0);
}
