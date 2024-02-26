#include "../include/defines.h"

// ============================
//   PLATFORM
// ============================

// Platform depended code, currently this only works on x86_64 anyway, but in
// future, there could also be arm

void sys_exit(i32 error_code);
i64 sys_read(u32 fd, const char *buf, u64 count);
i64 sys_write(u32 fd, const char *buf, u64 count);
i64 sys_stat(const char* filename, void* statbuf);
u64 sys_stat_stat_struct_len();
u64 sys_stat_stat_get_size(void* statbuf);
i64 sys_open(const char *filename, i32 flags, i32 mode);
i32 sys_open_flag_read();
i32 sys_close(u32 fd);
u64 platform_get_argc();
char* platform_get_argv(u32 index);

// These two are really important to be inlined even in debug builds, because
// otherwise compiler will generate some stack modification instructions

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

// ============================
//   END PLATFORM
// ============================

// ============================
//   FORWARD DECLARATIONS
// ============================


i64 runtime_format(const char* format, char* buffer,
		u64 buffer_size, const u64* argv);

// ============================
//   END FORWARD DECLARATIONS
// ============================
//
struct AllocEntry {
	u32 size;
	unsigned char pad1;
	unsigned char pad2;
	unsigned char free; // Is following block free (1) or taken (0)?
	unsigned char next; // Is there next block ? (1) or is this the end (0)
};
_Static_assert(sizeof(struct AllocEntry) == 8, "Bad size AllocEntry");

/* Belongs to struct ExpressionT */
enum ExpressionType {
	SYMBOL        = 1 << 1,
	STRING        = 1 << 2,
	TYPE_CONS     = 1 << 3,
	MEMORY        = 1 << 4,
	ASSOCA        = 1 << 5,
	TYPE_STRING   = 1 << 6,
	TYPE_ARRAY    = 1 << 7,
	TYPE_NUMBER   = 1 << 8,
};

/* Stage types in struct context  */
enum StageType {
	STAGE_EMPTY         = 0,
	STAGE_SYMBOL_TOKENS = 1,
};

/* Error codes in struct context */
enum ErrorCodes {
	ERROR_SUCCESS           = 0, // No error. Placeholder to protect 0
	ERROR_GENERAL           = 1,
	ERROR_ARGUMENT          = 2,
	ERROR_STACK_ALLOC       = 3, // Unused
	ERROR_OOM               = 4,
	ERROR_SYNTAX            = 5,
	ERROR_BUFFER_SIZE       = 6,
	ERROR_CRITICAL          = 7,

	ERROR_PARSER            = 16,
	ERROR_PARSER_OOM        = 17, // Used by both parser and ast_builder
	ERROR_PARSER_CHAR       = 18,

	ERROR_AST_EXPRESSION    = 32,
	ERROR_AST_UNSUPPORTED   = 33,
	ERROR_AST_SYMBOL_LONG   = 34,
	ERROR_AST_CRITICAL      = 35, // Error in implementation / logic
};

struct StringExpression {
	u32 size;
	char content[];
};

// This is void, because cons can hold pointers to more than just other
// ExpressionTs. E.g. AstNodes in parser
struct _consExpression {
	void* car;
	void* cdr;
};

struct _memoryExpression {
	u32 size;
	u32 taken;
	char mem[];
};

typedef struct ExpressionT {
	u32 expr_type;
	unsigned char pad0;
	unsigned char pad1;
	u16 pad2;
	union {
		struct StringExpression value_string;
		struct _consExpression value_cons;
		struct _memoryExpression value_memory;
		struct StringExpression value_symbol; // Symbols are strings
		i64 value_number;
	};
} Expression;


/* Ast symbols directs interpreter behaviour. They are something
 * like lightweight version of common/emacs lisp syntax.
 *
 * AST_IF - takes two or three lists. First is always evaluated
 * and if it evaulates to true, second list is executed. If list
 * evaluates to false, third list (if exists) is evaluated.
 *
 * AST_FUNC - this is just regular function call.
 *
 * AST_VALUE - Node contains literal value (some type of struct ExpressionT),
 * probably string or number
 *
 * AST_LET - scoped variable - by using (let (()) ... ) block, variable is
 * declared and is availble to be used inside let block. Variable cannot be
 * used outside. This behavior is kinda like clisp's behavior, except that for
 * now having multiple declarations in one let block is not supported. But this
 * can be circumvented by nesting let blocks.
 */
enum AstSymbol {
	AST_IF,
	AST_FUNC,
	AST_VALUE,
	AST_LET,
	AST_PROGN,
	AST_GOTO,
	//AST_TAG,
};

struct AstNode {
	u32 type;
	union {
		// AST_IF
		struct {
			void* condition;
			void* body_true;
			void* body_false;
		} ast_if;

		// AST_FUNC
		struct {
			// basic parser represenation of function call (only
			// for debugging purpose)
			struct ExpressionT* cons;
			// Type: Symbol - Name of the function
			struct ExpressionT* name;
			// Cons of AstNode* to args
			struct ExpressionT* args;
		} ast_func;

		// AST_VALUE
		struct {
			struct ExpressionT* value;
		} ast_value;

		// AST_LET
		struct {
			struct ExpressionT* name;
			// TODO: Change initial_value to AST_NODE
			struct ExpressionT* initial_value;
			struct AstNode* function;
		} ast_let;

		// AST_PROGN
		struct {
			 // Array of function pointers pointing to other AST
			 // structures
			struct ExpressionT* functions;
		} ast_progn;

		// AST_GOTO
		struct {
			struct ExpressionT* symbol;
		} ast_goto;
	};
};

struct ErrorContext {
	union {
		struct {
			const struct ExpressionT* expression;
		} parser_ast_error;

		// Parser didn't expect this character
		struct {
			char text[CONST_ERROR_PARSER_BUFFER_SIZE];
		} parser_character_error;
	};
};

/** This is complete state of parser and interpreter.
 */
struct context {
	char heap[HEAP_SIZE];

	char stack[STACK_SIZE];
	unsigned short _stack_pointer;
	struct ExpressionT* program; // (CONS) parsed program
	struct ExpressionT* parent_table; // (mem) Function call graph
	struct ExpressionT* return_values; // (mem) Return values from funtion graph
	struct ExpressionT* builtins; // (assoca) Builtin functions

	// This variable represents context, that can be passed with this down
	// the function call chain.  Each function may read it or replace it
	// for functions that it will call next.  But after returning, it
	// should be restored to parents value.
	void* stage;
	enum StageType stage_type;
	// Error handling
	u64 error_code;
	struct ErrorContext error_context;
};

// ===================================
//   BEGIN LIBRARY
// ===================================

// Functions that are helpers, but don't have equivalent in libc.  For those
// there is C library.
// All functions here begin with prefix lib_
// TODO: Some of these functions could be generated by macros

INLINE u32 lib_max2_u32(u32 a, u32 b) {
	if (a > b) return a;
	return b;
}

INLINE u32 lib_min2_u32(u32 a, u32 b) {
	if (a < b) return a;
	return b;
}

INLINE u64 lib_min2_u64(u64 a, u64 b) {
	if (a < b) return a;
	return b;
}

// ===================================
//   END LIBRARY
// ===================================

// ============================
//   BEGIN GLOBAL
// ============================

// Functions that manipulate state of the entire program.  This is like runtime
// functions, but not for running program, but for the language itself.
//
// Note: Global in this context means functions that manipulate global state of
// nterpreted program, not of the entire process

static inline u64 round8(u64 num) {
	return num + ((8 - (num % 8))%8);
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
	// case CONS:
	// 	{
	// 		sys_write(0, "( ", 2);
	// 		if (expression->value_list.cell != NULL) {
	// 			struct ConsCell *curr = expression->value_list.cell;
	// 			print_expression(curr->car);
	// 			sys_write(0, " ", 1);
	// 			print_expression(curr->cdr);

	// 		}
	// 		sys_write(0, ")", 2);
	// 		break;
	// 	}
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
	ctx->program = NULL;
	ctx->parent_table = NULL;
	ctx->return_values = NULL;
	ctx->builtins = NULL;
	ctx->error_code = 0;
	ctx->stage = (void*)0;
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

INLINE void* stack_push_aligned(struct context* ctx, u64 size) {
	return stack_push_var(ctx, round8(size));
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

/** Returns pointer to stack, to be preserved.
 * This pointer can later be used to reset stack
 */
u64 stack_get_sp(struct context* ctx) {
	return ctx->_stack_pointer;
}

/** Set stack pointer to sp.
 * Sp should be value returned by stack_get_sp
 */
void stack_set_sp(struct context* ctx, u64 sp) {
	ctx->_stack_pointer = sp;
}

/** Return 1 if error is set. 0 otherwise
 */
INLINE i32 global_is_error(struct context* ctx) {
	return ctx->error_code != 0;
}

INLINE void global_set_error(struct context* ctx, enum ErrorCodes code) {
	if (global_is_error(ctx))
		return;
	ctx->error_code = code;
}

INLINE void global_set_error_parser_oom(struct context* ctx) {
	global_set_error(ctx, ERROR_PARSER_OOM);
}

INLINE void global_set_error_oom(struct context* ctx) {
	global_set_error(ctx, ERROR_OOM);
}

void global_error_syntax(struct context* ctx, struct ExpressionT* _e) {
	if (global_is_error(ctx))
		return;
	global_set_error(ctx, ERROR_SYNTAX);
	_e = _e; // unused for now
}

STATIC void global_set_error_parser_char(struct context* ctx, const char* ptr, u32 length) {
	if (global_is_error(ctx))
		return;

	global_set_error(ctx, ERROR_PARSER_CHAR);
	for (u32 i = 0, max = lib_min2_u32(length, CONST_ERROR_PARSER_BUFFER_SIZE);
			i < max - 1; i++) {
		ctx->error_context.parser_character_error.text[i] = ptr[i];
		ctx->error_context.parser_character_error.text[i+1] = 0;
	}
}

STATIC void global_set_error_ast(struct context* ctx, const struct ExpressionT* ptr) {
	if (global_is_error(ctx)) {
		return;
	}
	global_set_error(ctx, ERROR_AST_EXPRESSION);
	ctx->error_context.parser_ast_error.expression = ptr;
}

STATIC void global_set_error_symbol_long(struct context* ctx, struct ExpressionT* ptr) {
	if (global_is_error(ctx)) {
		return;
	}
	global_set_error(ctx, ERROR_AST_SYMBOL_LONG);
	ctx->error_context.parser_ast_error.expression = ptr;
}

STATIC void global_set_error_unsupported_ast(struct context* ctx, struct ExpressionT* ptr) {
	if (global_is_error(ctx)) {
		return;
	}
	global_set_error(ctx, ERROR_AST_UNSUPPORTED);
	ctx->error_context.parser_ast_error.expression = ptr;
}

INLINE void global_set_error_ast_critical(struct context* ctx) {
	global_set_error(ctx, ERROR_AST_CRITICAL);
}

INLINE u64 global_get_error(struct context* ctx) {
	if (ctx)
		return ctx->error_code;
	return 0;
}

INLINE i32 _global_format_error_wrapper(const char* format_str, char* buffer, u64 buffer_size, const u64* argv) {
	i64 written = runtime_format(format_str, buffer, buffer_size, argv);
	if (written < 0)
		return 1;
	buffer[lib_min2_u64(buffer_size -1, written -1)] = 0;
	return 0;
}

/** Format global error to buffer.
 *  Returns 0 on success.
 *  Returns non-zero on failure
 */
i32 global_format_error(struct context* ctx, char* buffer, u32 max_size) {
	[[maybe_unused]]const char err_format[] =
		{ 'E' , ':', ' ', '%', 'd', ' ', '%', 's' };
	const char err_format_number[] = { 'E' , ':', ' ', '%', 'd'};
	u64 params[2];
	u64 error_code = global_get_error(ctx);

	params[0] = error_code;

	switch (ctx->error_code) {
		case 0:
			if (max_size > 2) {
				buffer[0] = 'o';
				buffer[1] = 'k';
				buffer[2] = 0;
			}
			return 0;
		case ERROR_PARSER_CHAR:
			params[1] = (u64)(ctx->error_context.parser_character_error.text);
			return runtime_format(
					FORMAT_STRING(err_format, "E: %d, parser error near: %s"),
					buffer, max_size, params) < 0;
		case ERROR_AST_EXPRESSION:
		case ERROR_AST_UNSUPPORTED:
		case ERROR_AST_SYMBOL_LONG:
			return runtime_format(err_format_number, buffer,
					max_size, params) < 0;
			// TODO: This should be nicer
		default:
			return _global_format_error_wrapper(err_format_number, buffer, max_size, params);
	}
}

INLINE void* global_get_stage(struct context* ctx) {
	if (ctx)
		return ctx->stage;
	return NULL;
}

INLINE void global_set_stage(struct context* ctx, const enum StageType stage_type, void* ptr) {
	if (ctx) {
		ctx->stage = ptr;
		ctx->stage_type = stage_type;
	}
}

INLINE enum StageType global_get_stage_type(struct context* ctx) {
	if (ctx)
		return ctx->stage_type;
	return 0;
}


// ============================
//   END GLOBAL
// ============================

// ============================
//   BEGIN C LIBRARY
// ============================

// Replacements for standard libc functions.  All functions have same or almost
// same signatures as libc functions, but they all begin with c_ prefix

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

i64 c_strncmp(const char* lhs, const char* rhs, u64 count) {
	while (*lhs && *lhs == *rhs && count) {
		lhs++;rhs++;count--;
	}
	if (!count)
		return 0;
	return *lhs - *rhs;


}

void c_memset(void *dest, unsigned char ch, u64 count) {
	unsigned char *ptr = dest;
	while(count--) {
		*(ptr++) = ch;
	}
}

void c_memcpy(void* dest, const void* src, u64 count) {
	char* destc = dest;
	const char* srcc = src;
	while (count--) {
		*destc = *srcc;
		destc++; srcc++;
	}
}

i32 c_memcmp(const void* ptr1, const void* ptr2, u32 size) {
	const char *p1, *p2;
	p1 = ptr1;
	p2 = ptr2;

	while (size && *p1 == *p2) {
		size--;
		p1++;
		p2++;
	}
	return size == 0 ? 0 : *p1 - *p2;
}

INLINE i32 c_isalpha(char c) {
	if (c >= 'a' && c <= 'z')
		return TRUE;
	if (c >= 'A' && c <= 'Z')
		return TRUE;
	return FALSE;
}

/** Return 1 if given character is digit. 0 otherwise.
 */
INLINE i32 c_isdigit(char c) {
	if (c >= '0' && c <= '9')
		return 1;
	return 0;
}

INLINE i32 c_isspace(char c) {
	switch (c) {
		case ' ':
		case '\n':
		case '\t':
		case '\r':
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

/** Sort array of 64 bit values, given cmp function
 * Performs insertion sort.
 * This is to be used in assocas.
 */
void c_sort64(void* ptr, u32 count, i64 (*cmp)(const void*, const void*) ) {
	u64 *arr = (u64*)ptr;
	for (u32 i = 1; i < count; i++) { // Already sorted part
		if (cmp(&arr[i-1], &arr[i]) <= 0) { // New is also sorted
			continue;
		}
		// This one is not in it's place, so let's move it
		for (u32 j = i; j > 0; j--) {
			if (cmp(&arr[j-1], &arr[j]) > 0) {
				u64 tmp = arr[j];
				arr[j] = arr[j-1];
				arr[j-1] = tmp;
			} else {
				break; // Already on correct spot
			}
		}
	}

}

/** Performs binary search on ARRAY. Element size must be 8 bytes.
 * Returns pointer to matching element or NULL
 */
void* c_bsearch(const void* key, const void* array,
		u32 length, i64 (*cmp)(const void*, const void*)) {
	const u64* arr = array;
	u32 bottom, top, current;
	bottom = 0;
	top = length - 1;

	if (!length) {
		return NULL;
	}

	while (1) {
		current = (top - bottom) / 2 + bottom;
		i64 cmp_result = cmp(key, (const void*)&arr[current]);
		if (cmp_result == 0) {
			return (void*)&arr[current];
		}
		if (top == bottom) {
			return NULL;
		}
		if (cmp_result < 0) {
			top = current;

		} else { /* (cmp_result > 0) */
			bottom = current + 1;
		}
	}
}
// ============================
//   END C LIBRARY
// ============================

// ============================
//   BEGIN  TYPES
// ============================


// ------ SYBMOL -------
// Symbol is string like type, that is used to represent names in parser. This
// type is not used during runtime (only to read parsed code)

/* Safely check if given type is SYMBOL.
 * Returns TRUE or FALSE
 */
INLINE u32 type_is_symbol(struct ExpressionT* expr) {
	if (expr && expr->expr_type == SYMBOL) {
		return TRUE;
	}
	return FALSE;
}

/* Get pointer to string representing symbol.
 * On error returns NULL
 */
INLINE const char* type_symbol_get_str(struct ExpressionT* expr) {
	if (type_is_symbol(expr)) {
		return (expr->value_symbol.content);
	}
	return NULL;
}

/* Get size of string that represents symbol.
 * On error returns 0
 */
INLINE u32 type_symbol_get_size(struct ExpressionT* expr) {
	if (type_is_symbol(expr)) {
		return expr->value_symbol.size;
	}
	return 0;
}

// ------ NUMBER -------

// Type representing non floating numbers. Base is 64 bit signed integer

/** Check if argument is of type number.
 *  Return 1 if type is number
 *  Return 0 if type is not number
 */
INLINE u32 type_isnumber(struct ExpressionT* expr) {
	if (expr && expr->expr_type == TYPE_NUMBER) {
		return 1;
	}
	return 0;
}

/** Allocate number on stack
 */
struct ExpressionT* type_number_alloc_stack(struct context* ctx) {
	struct ExpressionT* expr =
		stack_push_aligned(ctx, sizeof(struct ExpressionT));
	if (expr == NULL) {
		return NULL;
	}
	expr->expr_type = TYPE_NUMBER;
	expr->value_number = 0;
	return expr;
}

i64 type_number_get_value(struct ExpressionT* expr) {
	if (!type_isnumber(expr)) {
		return NULL;
	}
	return expr->value_number;
}

void type_number_set_value(struct ExpressionT* expr, i64 value) {
	if (!type_isnumber(expr))
		return;
	expr->value_number = value;
}

// ------ MEMORY -------

// Memory type is just a chunk of memory allocated on heap.  There are some
// convenience functions to index it as array of 64 bit integers, because it is
// very often used to store pointers.
//
// Memory is sometimes also used as subtype for other types, so this must be
// accounted for when for example calling type_ismem (that is true also for
// different types)

/** Safely check if expr is chunk of memory
 *
 */
INLINE u32 type_ismem(struct ExpressionT* expr) {
	if (expr && ((expr->expr_type) & MEMORY)) {
		return TRUE;
	}
	return FALSE;
}

/** Internal function, check documentation of type_mem_alloc
 */
struct ExpressionT* _type_mem_alloc(struct context* ctx, u32 size, int alloc_on_stack) {
	struct ExpressionT* new_expr;
	if (!alloc_on_stack) { // Alloc heap
		new_expr = heap_alloc(ctx, sizeof(struct ExpressionT) + size);
	} else { // Alloc stack
		new_expr = stack_push_aligned(ctx,
				sizeof(struct ExpressionT) + size);
	}
	if (new_expr == NULL) {
		DEBUG_ERROR("Cannot allocate memory chunk.");
		return NULL;
	}
	new_expr->expr_type = MEMORY;
	new_expr->value_memory.size = size;
	new_expr->value_memory.taken = 0;
	return new_expr;
}

/** Allocate chunk of memory on heap.
 *  Return pointer to new entry
 *  Return NULL on error
 */
INLINE struct ExpressionT* type_mem_alloc(struct context* ctx, u32 size) {
	return _type_mem_alloc(ctx, size, 0);
}

/** Allocate memory on stack.
 *  Return pointer to ExpressionT on success
 *  Return NULL on errro
 */
INLINE struct ExpressionT* type_mem_alloc_stack(struct context* ctx, u32 size) {
	return _type_mem_alloc(ctx, size, 1);
}

/** Get arbitrary location from mem chunk as pointer.
 * Returns NULL if index is out of range, or expr is not valid mem chunk
 */
INLINE void* type_mem_get_loc(struct ExpressionT* expr, u32 index) {
	if (!type_ismem(expr) || expr->value_memory.taken <= index) {
		return NULL;
	}
	return &expr->value_memory.mem[index];
}

/**  Get memory address of indexed element, or NULL.
 */
u64* type_mem_get_u64(struct ExpressionT* expr, u32 index) {
	const u32 value_size = sizeof(u64);
	u32 min_required = (index + 1) * value_size;
	if (!type_ismem(expr) || expr->value_memory.taken < min_required) {
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
	if (!type_ismem(expr)
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
	if(type_ismem(expr)) {
		return expr->value_memory.taken;
	}
	return -1;
}

/** Set mem chunk of length to value and set used size to length
 * Returns 0 on success or -1 on failure
 */
i64 type_mem_memset(struct ExpressionT* expr, unsigned char value, u64 length) {
	if (!type_ismem(expr)
		|| expr->value_memory.size < length) {
		return -1;
	}
	expr->value_memory.taken = length;
	c_memset(expr->value_memory.mem, value, length);
	return 0;
}

// ------ ARRAY ------

struct TypeArrayHeader {
	u32 taken;
	u32 element_size;
};
_Static_assert(sizeof(struct TypeArrayHeader) == 8, "Must be 8 aligned");

/** Return 1 if argument is of type array. 0 otherwise
 */
INLINE u32 type_isarray(struct ExpressionT* expr) {
	if (expr && type_ismem(expr) && (expr->expr_type & TYPE_ARRAY)) {
		return 1;
	}
	return 0;
}

/** Internal function */
INLINE struct TypeArrayHeader* _type_array_get_header(struct ExpressionT* expr) {
	return type_mem_get_loc(expr, 0);
}

/** Internal function */
INLINE void* _type_array_get_start(struct ExpressionT* expr) {
	return type_mem_get_loc(expr, sizeof(struct TypeArrayHeader));
}

/** Allocate array on heap memory.
 *  Return pointer to array on success, NULL otherwise.
 */
struct ExpressionT* type_array_heapalloc(struct context* ctx, u32 elem_count, u32 elem_size) {
	u32 actual_size = elem_count * elem_size + sizeof(struct TypeArrayHeader);
	struct ExpressionT* result = type_mem_alloc(ctx, actual_size);
	if (!result) {
		return NULL;
	}
	type_mem_memset(result, 0, actual_size); // Guaranteed ok
	result->expr_type |= TYPE_ARRAY;
	struct TypeArrayHeader *header = _type_array_get_header(result);
	header->taken = 0;
	header->element_size = elem_size;
	return result;
}

/** Return number of elements currently in array
 *  On error returns 0
 */
u32 type_array_len(struct ExpressionT* expr) {
	if (!type_isarray(expr)) {
		return 0;
	}
	struct TypeArrayHeader *header = _type_array_get_header(expr);
	return header->taken;
}

/** Return pointer to location of data on position INDEX
 */
void* type_array_get(struct ExpressionT* expr, u32 index) {
	if (!type_isarray(expr)) {
		return NULL;
	}
	struct TypeArrayHeader *header = _type_array_get_header(expr);
	if (index > header->taken) {
		return NULL;
	}
	unsigned char* data = _type_array_get_start(expr);
	data += index * header->element_size;
	return data;
}


/** Append element pointed to by DATA to the end of array.
 *  Returns 0 if success
 *  Returns 1 if memory is full
 *  Size of element is determined by array settings
 */
i32 type_array_push_back(struct ExpressionT* expr, const void* data) {
	if (!type_isarray(expr)) {
		return 1;
	}
	struct TypeArrayHeader *header = _type_array_get_header(expr);
	i64 max_size = type_mem_get_len(expr) - sizeof(struct TypeArrayHeader);
	i64 requested_size = (header->taken + 1) * header->element_size;
	if (requested_size > max_size) {
		return 1;
	}
	// Now we know, that element will fit
	header->taken++;
	void* location = type_array_get(expr, header->taken - 1);
	c_memcpy(location, data, header->element_size);
	return 0;
}


// ------ STRING ------

/** Check if type of expr is string.
 * Returns TRUE or FALSE
 */
u32 type_isstring(struct ExpressionT* expr) {
	if (type_ismem(expr) && expr->expr_type & TYPE_STRING)
		return TRUE;
	return FALSE;
}

/** Set length of string to len and wipes all contents.
 * Returns 0 on success, 1 on failure - length is larger than allocated size
 */
u32 type_string_set_length(struct ExpressionT* expr, u32 len) {
	i64 memset_result = type_mem_memset(expr, 0, len);
	if (memset_result != 0)
		return 1;
	return 0;
}

/** Allocate string with max SIZE. String is initally zeroed.
 * Returns pointer to new string or NULL on failure
 */
struct ExpressionT* type_string_alloc(struct context* ctx, u32 size) {
	struct ExpressionT* result = type_mem_alloc(ctx, size);
	if (result) {
		result->expr_type |= TYPE_STRING;
	}
	type_string_set_length(result, size);
	return result;
}

/** Returns length of string.
 */
u32 type_string_get_length(struct ExpressionT* expr) {
	i64 mem_len = type_mem_get_len(expr);
	return mem_len < 0 ? 0 : mem_len;
}

/** Get pointer to first character of string.
 * Returns NULL on error
 */
char* type_string_getp(struct ExpressionT* expr) {
	if (!type_isstring(expr)) {
		return NULL;
	}
	return type_mem_get_loc(expr, 0);
}

// ------ VARCHAR ------

// Varchar is minor type, that cannot be used as object for interpreter.
// Instead it is only used internally, becuase it is space efficient

struct Varchar {
	u16 length;
	char content[];
};

/** Get size of varchar entry rounded up to 8 bytes
 */
u64 type_varchar_get_size(struct Varchar* varchar) {
	u64 result = sizeof(struct Varchar) + varchar->length;
	if ((result % 8) == 0) {
		return result;
	}
	result += 8 - (result % 8);
	return result;
}

/** Create varchar entry in dest with source content.
 */
void type_varchar_create(void* dest, const char* source, u64 source_length) {
	struct Varchar* destination = (struct Varchar*)dest;
	destination->length = source_length;
	// TODO copy
	c_memcpy(destination->content, source, source_length);
}

// ------ ASSOCA ------

/* Associative arr laksjdklasjdlkasj d dsfakjsdhfa e  kfjsd sd fjhf sladf
 * aleskf jsdlf sd lfkj sdkfjew rjdj flsdkfj slkkjdsf asdsj fsdlfj lsdkfj ei
 * jslfjeoifjew
 *
 */

/* Associative array
 *
 * Cutom associative array (dictionary), where keys are strings of arbitrary
 * length and values are always not NULL pointers. This is currently largest
 * type in rasbi.  It is implemented on top of memory slice type, so all data
 * are packed together.
 *
 *
 * - Assoca starts with index pointing to keys in ascending order
 * - Memory location storing all keys and values is below the index
 * - All entries in memory are always in pairs of string (Varchar)| 64 bit
 *   value (usually pointer)
 * - Both key (varcar) and value (pointer) are always aligned at 8 bytes
 * - Everything is only in one slice of memory
 * - On each insert there is one round of insertion sort, this makes worst case
 *   insert O(N) complexity.
 * - On get, assoca performs binary search, thereofre O (logN) worst
 *
 *
 * Example memory layout of array with 4 keys:
 *
 * HEADER         +------------------+
 *   AssocaHeader | entry_count = 4  |
 *                | entry_max = 6    |
 *                | ...              |
 *      entries:  +------------------+
 *                |   addr1          |
 *                |   addr4          |
 *                |   addr3          |
 *                |   addr2          |
 *                |   NULL           |
 *                |   NULL           |
 * BODY           +------------------+
 *            1:  |  varchar (key)   |
 *                |   aaaa           |
 *                |  value = 0xff... |
 *            2:  |  varchar (key)   |
 *                |   zzzz           |
 *                |  value = 0xff... |
 *            3:  |  varchar (key)   |
 *                |   cccc           |
 *                |  value = 0xff... |
 *            4:  |  varchar (key)   |
 *                |   bbbb           |
 *                |  value = 0xff... |
 *                +------------------+
*/

// Relevant functions for working with assoca are here:
u32 type_assoca_delete(struct ExpressionT* expr, const char* key, u32 key_size);
u32 type_assoca_copy(struct ExpressionT* dest, struct ExpressionT* src);
u64 type_assoca_insert(struct ExpressionT* expr, const char* str, u64 size, u64 value);
INLINE struct ExpressionT* type_assoca_alloc(struct context* ctx, u32 count);

/* Standard usage should be to allocate expected size of array (given in
 * expected number of keys). Then insert values until inserts starts failing.
 * This means that array is full and new one (larger) must be allocated and old
 * one copied to new one.
 */

// Implementation of assoca:

struct AssocaHeader {
	// Tells how many entries are stored in "entries" array
	u16 entry_count;
	// Tells how many entries there could potentially be, before this
	// structure would need to be reallocated (before you would hit
	// something else by accessing larger "entries" index).  It doesn't
	// mean that there is enough space to store strings in memory.
	u16 entry_max;
	u32 _pad1;
	struct Varchar* entries[];
};

_Static_assert(sizeof(struct AssocaHeader) == 8, "Bad size AssocaHeader");

/** Check whether expression is assoca.
 * Returns TRUE if is and FALSE if it is not
 */
u32 type_isassoca(struct ExpressionT* expr) {
	if (type_ismem(expr) && expr->expr_type & ASSOCA) {
		return TRUE;
	}
	return FALSE;
}

/** Get pointer to Assoca Header structure
 */
struct AssocaHeader* _type_assoca_get_header(struct ExpressionT* expr) {
	if (!type_isassoca(expr)) {
		return NULL;
	}
	return (struct AssocaHeader*)type_mem_get_loc(expr, 0);
}

/** Compute assoca header size (used and unused space together)
 */
u64 _type_assoca_get_header_size(struct AssocaHeader* header) {
	return round8(sizeof(struct AssocaHeader)) +
		header->entry_max*sizeof(struct Varchar*);
}

/** Insert value on any available position in header.
 * On success return 0. If header is full, return 1.
 */
u64 _type_assoca_header_insert(struct AssocaHeader* header, u64 value) {
	if (header->entry_count >= header->entry_max) {
		return 1;
	}
	u16 target_index = header->entry_count++;
	header->entries[target_index] = (struct Varchar*)value;
	return 0;
}

/** Allocate associative array with size to contain up to COUNT entries.
 *  Note: It is possible to fill available memory with fewer than COUNT
 *  entries, so it is always callers responsibility to check if insert was
 *  successful.
 *
 * Returns pointer on success and NULL on failure (allocation fail)
 */
struct ExpressionT* _type_assoca_alloc(struct context* ctx, u32 count, int alloc_on_stack) {
	// Size to fit approximately count english words
	u32 request_size = round8(sizeof(struct AssocaHeader))
		+ count*sizeof(u64)  // Entries in AssocaHeader
		+ count*sizeof(u64) // References in dictionary
		+ count*sizeof(void*) // Average length of word (my estimate)
		+ count*round8(sizeof(struct Varchar));
	request_size = round8(request_size);
	struct ExpressionT* result;
	if (alloc_on_stack) {
		result = type_mem_alloc_stack(ctx, request_size);
	} else {
		result = type_mem_alloc(ctx, request_size);
	}
	if (!result) {
		DEBUG_ERROR("cannot allocate assoca");
		return NULL;
	}
	result->expr_type |= ASSOCA;
	type_mem_memset(result, 0xdd, request_size); // Always shall succceed
	struct AssocaHeader* header = _type_assoca_get_header(result);
	header->entry_max = count;
	header->entry_count = 0;
	return result;
}

/** Allocate associative array on stack. For details check documentation of
 *  function _type_assoca_alloc
 */
INLINE struct ExpressionT* type_assoca_alloc_stack(struct context* ctx, u32 count) {
	return _type_assoca_alloc(ctx, count, 1);
}

/** Allocate associative array on heap. For details check documentation of
 *  function _type_assoca_alloc
 */
INLINE struct ExpressionT* type_assoca_alloc(struct context* ctx, u32 count) {
	return _type_assoca_alloc(ctx, count, 0);
}

/** Get pointer with largest address
 */
struct Varchar* _type_assoca_get_lastp(struct AssocaHeader* header) {
	struct Varchar* largest = NULL;

	for (u16 i = 0; i < header->entry_count; i++) {
		struct Varchar* current = header->entries[i];
		if (current > largest) {
			largest = current;
		}
	}
	return largest;
}

/** Get pointer to first free space, after last entry.
 */
struct Varchar* _type_assoca_get_free(struct ExpressionT* expr) {
	if (!type_isassoca(expr))
		return 0;
	struct AssocaHeader* header = _type_assoca_get_header(expr);
	if (header->entry_count >= header->entry_max) {
		return NULL;
	}
	struct Varchar* last_entry =  _type_assoca_get_lastp(header);
	if (!last_entry) { // There is no entry yet
		return (struct Varchar*)
			(((char*)header) + _type_assoca_get_header_size(header));
	}
	u64 last_entry_size = type_varchar_get_size(last_entry);
	char* new_entry =
		((char*)last_entry) + last_entry_size + sizeof(void*);
	return (struct Varchar*) new_entry;
}

/** Count available space behind last entry in assoca.
 * This space must fit both value and varchar structure
 */
u64 _type_assoca_count_free_space(struct ExpressionT* expr) {
	if (!type_isassoca(expr)) {
		return 0;
	}
	void* last_possible_space =
		&expr->value_memory.mem[expr->value_memory.taken];
	void* past_last_item = _type_assoca_get_free(expr);
	if (past_last_item == NULL) {
		return 0;
	}
	return last_possible_space - past_last_item;
}

/** Cmp function (sort compatible) for assoca. Works with (struct Varchar*)
 */
i64 _type_assoca_cmp(const void* lhs, const void* rhs) {
	const struct Varchar  *right, *left;
	right = *(struct Varchar**)rhs;
	left = *(struct Varchar**)lhs;
	u16 length = left->length > right->length ? right->length : left->length;
	i64 ret = c_strncmp(left->content, right->content, length);
	if (ret != 0) {
		return ret;
	}
	return right->length - left->length;
}

/** Find STR key in assoca, and return pointer to it's internal structure
 * Performs linear search through all entries.
 * On error returns NULL
 */
struct Varchar* _type_assoca_find_entry(
		struct ExpressionT* expr, const char* str, u64 size) {
	struct Varchar* result = NULL;
	if (!type_isassoca(expr)) {
		return result;
	}
	struct AssocaHeader* header = _type_assoca_get_header(expr);
	for (u32 i = 0; i < header->entry_count; i++) {
		struct Varchar* key = header->entries[i];
		if (key->length != size)
			continue;
		if (c_strncmp(key->content, str,size) != 0)
			continue;
		// match found
		result = key;
		break;
	}
	return (void*)result;
}

/** Find STR key and return pointer to it's Varchar structure
 * Performs binary search
 * Returns pointer to struct Varchar or NULL.
 */
struct Varchar* _type_assoca_find_entry2(
		struct ExpressionT* expr, const char* str, u64 size) {
	if (!type_isassoca(expr) || size == 0) {
		return NULL;
	}
	struct AssocaHeader *header = _type_assoca_get_header(expr);
	// We need to copy string we are looking for to temporary Varchar
	// structure, so we can use cmp methods that are used to sort Varchars
	// in assoca
	char buffer[round8(size+sizeof(struct Varchar))];
	struct Varchar* buffer_ptr = (struct Varchar*)buffer;
	type_varchar_create((void*)buffer, str, size);
	struct Varchar** result =  c_bsearch((void*)&buffer_ptr,
			header->entries, header->entry_count, _type_assoca_cmp);
	if (result) {
		return *result;
	}
	return NULL;
}

void _type_assoca_pack(struct ExpressionT* expr);
void _type_assoca_sort(struct ExpressionT* expr);

/** Insert VALUE to associative array EXPR under key STR (with length SIZE).
 * On success retunrs 0, on error 1.
 * Currently no realocations are supported
 * TODO: This must take void pointers (and verify not NULL probably)
 */
u64 type_assoca_insert(struct ExpressionT* expr, const char* str, u64 size, u64 value) {
	if (!type_isassoca(expr)) {
		return 1;
	}

	// Check if entry exists
	struct Varchar* old_entry = _type_assoca_find_entry2(expr, str, size);
	if (old_entry) { // It exists, so just replace it
		u16 vchar_size = type_varchar_get_size(old_entry);
		u64* destp = (u64*)
			(((char*)old_entry) + vchar_size);
		*destp = value;
		return 0;
	}

	// No entry was found, let's insert
	struct AssocaHeader* header = _type_assoca_get_header(expr);

	// Check if space is available. If not repack and try again
	for (i32 i = 0; i < 2; i++)
	{
		u64 space_left = _type_assoca_count_free_space(expr);
		u64 space_required =
			sizeof(struct Varchar) + size + sizeof(void*);
		space_required = round8(space_required);
		if (space_required > space_left) {
			if (i == 0) {
				_type_assoca_pack(expr);  // Defragmentation
			} else {
				return 1;
			}
		} else {
			break;  // It will fit, so continue
		}
	}

	// Modify header
	struct Varchar* vchar = _type_assoca_get_free(expr);
	if (_type_assoca_header_insert(header, (u64)vchar)) {
		// Insert of index failed, but no permanent damage
		return 1;
	}

	// Actual insertion of value
	vchar->length = size;
	c_memcpy(vchar->content, str, size);
	u64 vchar_size = type_varchar_get_size(vchar); // This will get 8 aligned size
	u64* target_location = (u64*) (((char*)vchar) + vchar_size);
	*target_location = value;

	// Sort index, because now we have appended new value
	_type_assoca_sort(expr);
	return 0;
}

/** Get pointer stored under STR key in associative array EXPR
 * If STR key doesn't exist, NULL is returned
 */
void* type_assoca_get(struct ExpressionT* expr, const char* str, u64 size) {
	struct Varchar* varchar = _type_assoca_find_entry2(expr, str, size);
	if (!varchar) {
		return NULL;
	}

	u64 varchar_size = type_varchar_get_size(varchar);
	u64 *valuep = (u64*)  // This could be void** but this is more logical
		(((char*)varchar) + varchar_size);
	return (void*)*valuep;
}

u32 type_assoca_copy(struct ExpressionT* dest, struct ExpressionT* src) {
    if (!type_isassoca(src) || !type_isassoca(dest)) {
        return 1;
    }
    struct AssocaHeader *header = _type_assoca_get_header(src);
    for (u32 i = 0; i < header->entry_count; i++) {
        struct Varchar *key = header->entries[i];
        if (!key) { // Already deleted entry
            continue;
        }
        char* valuep = (char*)key;
        valuep += type_varchar_get_size(key);
        u64 result =
            type_assoca_insert(dest, key->content, key->length, *((u64*)valuep));
        if (result != 0) {
            return 1;
        }
    }
    return 0;
}

/** Move pointers next to each other, so that there is no space between them.
 * This operation is necessary after deletion, because probably will be some
 * NULL pointer left.
 */
void _type_assoca_squeeze_index(struct ExpressionT* expr) {
	struct AssocaHeader *header = _type_assoca_get_header(expr);
	if (!header) {
		return;
	}
	u32 fixed_count = 0;
	for (u32 dest = 0; dest < header->entry_count; dest++) {
		if (header->entries[dest]) {
			continue;
		}
		fixed_count++;
		// Swap this place
		for (u32 src_index = dest+1;
				src_index < header->entry_count; src_index++) {
			if (header->entries[src_index] == NULL) {
				continue;
			}
			// Found good place to swap
			struct Varchar* tmp = header->entries[dest];
			// tmp really should be NULL anyway
			header->entries[dest] = header->entries[src_index];
			header->entries[src_index] = tmp;
			break;
		}
	}
	header->entry_count -= fixed_count;
}

/** Remove key from assoca.
 * Returns 0 on success, 1 if key doesn't exist
 */
u32 type_assoca_delete(struct ExpressionT* expr, const char* key, u32 key_size) {
	struct AssocaHeader *header = _type_assoca_get_header(expr);
	if (!type_isassoca(expr) || !header) {
		return 1;
	}
	struct Varchar *entry = _type_assoca_find_entry2(expr, key, key_size);
	if(!entry) {
		return 1;
	}
	for (u32 i = 0; i < header->entry_count; i++) {
		if (header->entries[i] == entry) { // Found him
			header->entries[i] = NULL;
		}
	}
	_type_assoca_squeeze_index(expr);
	return 0;
}

/** Sort keys to ascending order
 */
void _type_assoca_sort(struct ExpressionT* expr) {
	if (!type_isassoca(expr)) {
		return;
	}
	struct AssocaHeader* header = _type_assoca_get_header(expr);
	c_sort64(header->entries, header->entry_count, _type_assoca_cmp);
}

/** Shuffle assoca, so that there is no blank space between entries.
 * This is potentialy expensive operation, so it is done only if there is no
 * space left for new entry.
 */
void _type_assoca_pack(struct ExpressionT* expr) {
	struct AssocaHeader *header = _type_assoca_get_header(expr);
	if (!header) {
		return;
	}
	struct Varchar *end = (struct Varchar*) // Point behind last checked addr
		(((char*)header) + _type_assoca_get_header_size(header));
	for (u32 i = 0; i < header->entry_count; i++) {
		struct Varchar *smallest = NULL;
		struct Varchar **smallest_addr = NULL;
		// Find smallest, not yet moved
		for (u32 j = 0; j < header->entry_count; j++) {
			if (header->entries[j] < end) {
				continue; // Throw away already checked
			}
			// Now entry is usable (big enough), but is it the smallest one?
			if (!smallest || header->entries[j] < smallest) {
				smallest = header->entries[j];
				smallest_addr = &header->entries[j];
			}
		}
		if (!smallest) {
			return;
		}
		u64 shift = smallest - end;
		u64 smallest_size = type_varchar_get_size(smallest) + sizeof(void*);
		if (shift) {
			c_memcpy(end, smallest, smallest_size);
		}
		*smallest_addr = end; // Edit entry in array
		end = (struct Varchar*)
			(((char*)end) + smallest_size);
	}
}


// -------- CONS --------

// Cons is one of the basic lisp types. For more details on this type, look for
// common lisp cons.  Here it is used to build linked lists and trees

struct ExpressionT* type_cons_alloc_stack(struct context* ctx) {
	struct ExpressionT* expr =
		stack_push_aligned(ctx, sizeof(struct ExpressionT));
	if (!expr) {
		DEBUG_ERROR("Cons allocation on stack failed.\n");
		return NULL;
	}
	expr->expr_type = TYPE_CONS;
	expr->value_cons.car = NULL;
	expr->value_cons.cdr = NULL;
	return expr;
}

/* Check if expression if of type CONS.
 * Returns TRUE or FALSE
 */
u32 type_iscons(const struct ExpressionT* expr) {
	if(expr && expr->expr_type == TYPE_CONS) {
		return TRUE;
	}
	return FALSE;
}

/* Get/Set CAR/CDR of expr.
 * On get error returns NULL.
 */
INLINE struct ExpressionT* type_cons_car(const struct ExpressionT* cons) {
	if (type_iscons(cons)) {
		return (struct ExpressionT*)cons->value_cons.car;
	}
	return NULL;
}

/** Return content of car, casted to AstNode
 * Behaves same as type_ast_node
 */
INLINE struct AstNode* type_cons_car_ast(struct ExpressionT* cons) {
	return (struct AstNode*)type_cons_car(cons);
}

INLINE void type_cons_set_car(struct ExpressionT* cons, void* car) {
	if (type_iscons(cons)) {
		cons->value_cons.car = car;
	}
}

INLINE struct ExpressionT* type_cons_cdr(const struct ExpressionT* cons) {
	if (type_iscons(cons)) {
		return (struct ExpressionT*)cons->value_cons.cdr;
	}
	return NULL;
}

/** See type_cons_car_ast
 */
INLINE struct AstNode* type_cons_cdr_ast(struct ExpressionT* cons) {
	return (struct AstNode*)type_cons_cdr(cons);
}

INLINE void type_cons_set_cdr(struct ExpressionT* cons, void* cdr) {
	if (type_iscons(cons)) {
		cons->value_cons.cdr = cdr;
	}
}

// ------- AST -----------

// Ast is type used by parser to represent building blocks of program. E.g.
// each if is one ast block...
//
// Ast is allocated on stack, because it is built before interpreter starts
// running the program, it is also never deleted and therefore no heap
// structure overhead is necessary

INLINE struct AstNode* _type_ast_alloc_stack(struct context* ctx) {
	void* memp = stack_push_aligned(ctx, sizeof(struct AstNode));
	c_memset(memp, 0, sizeof(struct AstNode));
	return (struct AstNode*) memp;
}

INLINE u32 type_ast_is(struct AstNode* ast, enum AstSymbol type) {
	return ast && ast->type == type;
}

// AST_FUNC
void* type_ast_alloc_func(struct context* ctx, struct ExpressionT* expr) {
	struct AstNode* node = stack_push_aligned(ctx, sizeof(struct AstNode));
	if (!node) {
		return NULL;
	}
	node->type = AST_FUNC;
	node->ast_func.cons = expr;
	return node;
}

INLINE u32 type_ast_isfunc(struct AstNode* ast) {
	return type_ast_is(ast, AST_FUNC);
}

INLINE struct ExpressionT* type_ast_func_expr(struct AstNode* node) {
	if (!type_ast_isfunc(node))
		return NULL;
	return node->ast_func.cons;
}

// AST_IF

INLINE u32 type_ast_isif(struct AstNode* ast) {
	return type_ast_is(ast, AST_IF);
}

struct AstNode* type_ast_alloc_if(struct context* ctx) {
	struct AstNode* node = stack_push_aligned(ctx, sizeof(struct AstNode));
	if (!node) {
		return NULL;
	}
	node->type = AST_IF;
	node->ast_if.condition = NULL;
	node->ast_if.body_true = NULL;
	node->ast_if.body_false = NULL;
	return node;
}

// AST_VALUE

INLINE struct AstNode* type_ast_alloc_value(struct context* ctx, struct ExpressionT* expr) {
	struct AstNode* node = stack_push_aligned(ctx, sizeof(struct AstNode));
	if (!node) {
		return NULL;
	}
	node->type = AST_VALUE;
	node->ast_value.value = expr;
	return node;
}

INLINE u32 type_ast_isval(struct AstNode* node) {
	return type_ast_is(node, AST_VALUE);
}

// AST_LET
INLINE struct AstNode* type_ast_alloc_let(struct context* ctx) {
	struct AstNode* node = _type_ast_alloc_stack(ctx);
	if (!node) {
		return NULL;
	}
	node->type = AST_LET;
	node->ast_let.name = NULL;
	node->ast_let.initial_value = NULL;
	node->ast_let.function = NULL;
	return node;
}

INLINE u32 type_ast_islet(struct AstNode* node) {
	return type_ast_is(node, AST_LET);
}

// AST_PROGN

INLINE struct AstNode* type_ast_alloc_progn(struct context* ctx) {
	struct AstNode* node = _type_ast_alloc_stack(ctx);
	if (!node) {
		return NULL;
	}
	node->type = AST_PROGN;
	node->ast_progn.functions = NULL;
	return node;
}

/** Return 1 if type is AST_PROGN
 *  0 otherwise
 */
INLINE i32 type_ast_isprogn(struct AstNode* node) {
	return type_ast_is(node, AST_PROGN);
}

// AST_GOTO

/** Return 1 if type is AST_GOTO
 *  0 otherwise
 */
INLINE i32 type_ast_isgoto(struct AstNode* node) {
	return type_ast_is(node, AST_GOTO);
}

/** Allocate ast goto structure on stack and return pointer
 */
struct AstNode* type_ast_alloc_goto(struct context* ctx, struct ExpressionT* symbol) {
	if (!type_is_symbol(symbol)) {
		return NULL;
	}
	struct AstNode* node = _type_ast_alloc_stack(ctx);
	if (!node) {
		return NULL;
	}
	node->type = AST_GOTO;
	node->ast_goto.symbol = symbol;
	return node;
}

// ============================
//   END TYPES
// ============================

/** DEPRECATED Allocate empty string expression on context heap.
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
	// TODO: Maybe this shoul error on finding 0 - eof
	i32 inside_comment = 0;
	for (u64 i = start_position; i < count; i++) {
		if (inside_comment) {
			if (buf[i] == '\n' || buf[i] == '\r')
				inside_comment = 0;
			else
				continue;
		}
		if (buf[i] == ';') {
			inside_comment = 1;
			continue;
		}
		if (c_isspace(buf[i])) {
			continue;
		}
		return i;
	}
	return -1;
}

/** Allocate table of size 256, that has 1 on places representing valid values
 * for symbols. 0 everywhere else
 */
LOCAL struct ExpressionT* _parser_alloc_symbol_tokens(struct context* ctx) {
	struct ExpressionT* table = type_mem_alloc(ctx, 256);
	i64 memset_result = type_mem_memset(table, 0, 256);
	if (!table || memset_result)
		return NULL;

	char* ptr = type_mem_get_loc(table, 0);

	for (u32 i = 'a'; i < 'z'; i++)
		ptr[i] = 1;
	for (u32 i = 'A'; i < 'Z'; i++)
		ptr[i] = 1;

	ptr['+'] = 1;
	ptr['-'] = 1;
	ptr['*'] = 1;
	ptr['/'] = 1;
	ptr['='] = 1;

	ptr['_'] = 1;
	ptr['#'] = 1;
	ptr['!'] = 1;
	ptr['?'] = 1;
	return table;
}

INLINE i32  _parser_is_symbol_token(unsigned char c, const char *tokens) {
	return tokens[c] != 0;
}

/** Subparser for symbols (names)
 */
Expression* parser_parse_symbol(
		const char* buf,
		u64 max_size,
		/* out */ u64* num_processed,
		struct context* ctx) {
	// Check if we were called properly
	if (global_get_stage_type(ctx) != STAGE_SYMBOL_TOKENS) {
		global_set_error(ctx, ERROR_CRITICAL);
		return NULL;
	}
	struct ExpressionT* symbol_table_mem = global_get_stage(ctx);
	const char* symbol_table = type_mem_get_loc(symbol_table_mem, 0);

	u32 symbol_size = 0;
	for (; symbol_size < max_size; symbol_size++) {
		if (buf[symbol_size] == 0 || buf[symbol_size] == ')')
			break;
		if (c_isspace(buf[symbol_size]))
			break;
		if (_parser_is_symbol_token(buf[symbol_size], symbol_table))
			continue;
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

/** Parse number from buffer.
 *  Currently supported number format is base 10 numbers, that may have - sign
 *  before them. Nothing else.
 */
struct ExpressionT* parser_parse_number(const char* buf, u64 max_size,
		/* out */ u64* num_processed, struct context* ctx) {
	// First check syntax and count digits
	u64 character_count = 0;
	i32 has_minus = 0;
	for (; character_count < max_size; character_count++) {
		if (c_isdigit(buf[character_count])) {
			continue;
		}
		if (c_isspace(buf[character_count]) || buf[character_count] == ')') {
			// This is ok finish for us
			break;
		}
		if (character_count == 0 && buf[character_count] == '-') {
			has_minus = 1;
			continue;
		}
		// There is unexpected character. Format error
		character_count = character_count  > 2 ? character_count - 2 : 0;
		global_set_error_parser_char(ctx, &buf[character_count],
				max_size - character_count);
		return NULL;
	}

	// Failsafe
	if (character_count == 0 || (character_count == 1 && has_minus)) {
		global_set_error_parser_char(ctx, &buf[0], max_size);
		return NULL;
	}

	struct ExpressionT* result = type_number_alloc_stack(ctx);

	// --------------------------------------------------------------------
	// Everything is ok. Nowhere to fail from now
	if (num_processed)
		*num_processed = character_count;

	if (has_minus) {
		character_count--;
		buf++;
	}

	i64 final_number = 0;
	i64 multiplier = 1;
	for (u64 i = 0; i < character_count; i++, multiplier *= 10) {
		i64 digit = buf[character_count - i - 1] - '0';
		final_number += digit*multiplier;
	}
	if (has_minus)
		final_number *= -1;
	type_number_set_value(result, final_number);
	return result;
}

/** Subparser for strings
 *
 * In future it should support constructs like \n and \"
 */
struct ExpressionT* parser_parse_string(
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

	ret = type_string_alloc(ctx, final_size);
	if (!ret) {
		DEBUG_ERROR("Parser str alloc failed");
		return NULL;
	}

	char* destp = type_string_getp(ret);
	// Copy string
	u64 processed = 1;
	for (u64 i = 1, dest = 0; i < max_size; i++) {
		processed++;
		if (buf[i] == '"') {
			// Now breaking here is guaranteed
			break;
		}
		destp[dest++] = buf[i];
	}
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
	struct ExpressionT* ret = NULL;
	// First let's determine if this is top level parser method call - if
	// it is, it must initialize symbol token table.
	const u64 old_stage_type = global_get_stage_type(ctx);
	void* old_stage_value = global_get_stage(ctx);
	if (old_stage_type != STAGE_SYMBOL_TOKENS) {
		void* symbol_token_table = _parser_alloc_symbol_tokens(ctx);
		if (!symbol_token_table) {
			global_set_error_parser_oom(ctx);
			goto bad;
		}
		global_set_stage(ctx, STAGE_SYMBOL_TOKENS, symbol_token_table);
	}
	// Now guaranteed to not be NULL
	struct ExpressionT* symbol_tokens_expr = global_get_stage(ctx);
	const char* symbol_tokens = type_mem_get_loc(symbol_tokens_expr, 0);
	// MAIN LOOP
	i64 i = parser_find_next_char(buf, 0, count);
	if (i == -1) {
		global_set_error(ctx, ERROR_PARSER);
		goto bad;
	}
	for (; i < count || buf[i] != 0; i++) {
		char c = buf[i];
		if (c == '(') { // This will be new list
			ret = type_cons_alloc_stack(ctx);
			if (!ret) {
				DEBUG_ERROR("Memory fail - parser");
				goto bad;
			}

			while (1) { // Load everything inside expression
				i = parser_find_next_char(buf,i+1, count);
				if (i < 0) {
					// TODO: Free memory
					DEBUG_ERROR("Cannot find closing brace");
					goto bad;
				}
				if (buf[i] == ')') { // Parsing finished
					if (out_processed)
						*out_processed = i+1;
					goto ret;
				}

				u64 skip = 0;
				struct ExpressionT* es =
					parser_parse_expression(buf+i, count - i, ctx, &skip);
				if (!es) {
					DEBUG_ERROR("Invalid expression");
					goto bad;
				}
				i += skip - 1;

				// Begin list or append to existing list
				if (type_cons_car(ret) == NULL) {
					type_cons_set_car(ret, es);
				} else {
					// Append to the end of current list
					// new cons
					struct ExpressionT* new_cons =
						type_cons_alloc_stack(ctx);
					if (!new_cons) {
						DEBUG_ERROR("Cannot allocate cons");
						goto bad;
					}
					type_cons_set_car(new_cons, es);
					struct ExpressionT* curr = ret;
					while (type_cons_cdr(curr) != NULL) {
						curr = type_cons_cdr(curr);
					}
					type_cons_set_cdr(curr, new_cons);
				}
			}
			DEBUG_ERROR("Unclosed expression");
			goto bad;
		// Parse string
		} else if (c == '"') {
			u64 string_size = 0;
			struct ExpressionT* es =
				parser_parse_string(buf+i, count-i, &string_size, ctx);
			if (es == NULL) {
				DEBUG_ERROR("Invalid string parse");
				goto bad;
			}
			if (out_processed)
				*out_processed =  string_size + i;
			// TODO: Check what happens with ret old value
			ret = es;
			goto ret;
		}
		// Parse number
		else if (c_isdigit(c) ||  // Digit or minus AND digit
				(c == '-' && (i+1) < count && c_isdigit(buf[i+1]))) { // Parse integer
			u64 number_length = 0;
			struct ExpressionT* number =
				parser_parse_number(&buf[i], count - i, &number_length, ctx);
			if (!number) {
				// Error is set by parser_parse_number
				goto bad;
			}
			ret = number;
			if (out_processed) {
				*out_processed = i + number_length;
			}
			goto ret;
		}
		// Parse symbol
		else if (_parser_is_symbol_token(c, symbol_tokens)) {
			u64 string_size = 0;
			struct ExpressionT* expr =
				parser_parse_symbol(buf+i, count-i, &string_size, ctx);
			if (!expr) {
				DEBUG_ERROR("Cannot parse symbol");
				goto bad;
			}
			if (out_processed)
				*out_processed =  string_size + i;
			// TODO: Check what happens with ret old value
			ret = expr;
			goto ret;
		}
		else {
			DEBUG_ERROR("Invalid character");
			global_set_error_parser_char(ctx,
					&buf[i > 2 ? i - 2 : i], count - i);
			goto bad;
		}
	}
bad:
	ret = NULL; // TODO: Free memory
	DEBUG_ERROR("Wrong");
ret:
	// TODO: Free table
	global_set_stage(ctx, old_stage_type, old_stage_value);
	return ret;
}

/** Main parser method, used to call parser from outside
 *  All structure are allocated on context heap
 */
int parse_program(struct context* ctx, const char* buf, u64 size) {
	struct ExpressionT *exp =
		parser_parse_expression(buf, size, ctx, NULL);
	if  (!exp) {
		return -1;
	}
	ctx->program = exp;
	return 0;
}

/** Check that 'if' syntax looks ok.
 * Return 0 on success or 1 on failure
 */
STATIC int parser_ast_verify_if(struct context* ctx, struct ExpressionT* expr) {
	if (!type_iscons(expr)) {
		global_set_error(ctx, ERROR_PARSER);
		return 1;
	}
	struct ExpressionT* curr = type_cons_car(expr);
	// Anything can be as if condition, except null
	if (!curr) {
		global_set_error_ast(ctx, NULL);
		return 1;
	}
	curr = type_cons_cdr(expr);
	if (!type_cons_car(curr)) { // Missing first/true branch
		global_set_error_ast(ctx, curr);
		return 1;
	}
	curr = type_cons_cdr(curr);
	if (!curr) {// Missing second/false branch -- OK
		return 0;
	}
	if (!type_cons_car(curr)) { // Bad out from parser (empty branch)
		global_set_error_ast(ctx, curr);
		return 1;
	}
	if (type_cons_cdr(curr)) { // Too many branches in if
		global_set_error_ast(ctx, type_cons_cdr(curr));
		return 1;
	}
	return 0;
}

STATIC int parser_ast_verify_let(struct ExpressionT* expr) {
	if (!type_iscons(expr)) {
		return 1;
	}
	struct ExpressionT* variable = type_cons_car(type_cons_car(expr));
	struct ExpressionT* body = type_cons_cdr(expr);

	// We don't have any requirements of body, except that someting is
	// present.
	if(!body || !type_iscons(body)) {
		return 1;
	}
	// Variable is nested in one extra () according to standard clisp
	// Variable needs to have name (symbol) and something else (anything
	// valid)
	if (!variable || !type_iscons(variable)) {
		return 1;
	}

	struct ExpressionT* var_name = type_cons_car(variable);
	struct ExpressionT* var_value = type_cons_car(type_cons_cdr(variable));

	if (!type_is_symbol(var_name)) {
		return 1;
	}

	// Now we only accept strings as valid values
	if (!type_isstring(var_value)) {
		return 1;
	}
	return 0;
}

struct AstNode* parser_ast_build(struct context*, struct ExpressionT*);

/** Verify and build AST structure for goto expression.
 */
STATIC struct AstNode* parser_ast_build_goto(struct context* ctx, struct ExpressionT* expr) {
	struct ExpressionT* symbol = type_cons_car(expr);
	if (!type_iscons(expr) || type_cons_cdr(expr) || !type_is_symbol(symbol)) {
		global_set_error_ast(ctx, expr);
		return NULL;
	}
	// Now syntax is good
	struct AstNode* symbol_node = type_ast_alloc_goto(ctx, symbol);
	if (!symbol_node) {
		global_set_error_parser_oom(ctx);
		return NULL;
	}
	return symbol_node;
}

STATIC struct AstNode* parser_ast_build_let(struct context* ctx, struct ExpressionT* expr) {
	if (parser_ast_verify_let(expr)) {
		global_set_error(ctx, ERROR_SYNTAX);
		return NULL;
	}
	struct AstNode* let_node = type_ast_alloc_let(ctx);
	if (!let_node) {
		global_set_error_parser_oom(ctx);
		return NULL;
	}
	struct ExpressionT* var_decl = type_cons_car(type_cons_car(expr));
	let_node->ast_let.name = type_cons_car(var_decl);
	let_node->ast_let.initial_value = type_cons_car(type_cons_cdr(var_decl));

	// TODO: Handle multiple functions as progn. Or don't, but in that case
	// fix verifier for let sexp

	let_node->ast_let.function = parser_ast_build(ctx, type_cons_car(type_cons_cdr(expr)));
	if (!let_node->ast_let.function) {
		return NULL;
	}

	return let_node;
}

STATIC struct AstNode* parser_ast_build_if(struct context* ctx, struct ExpressionT* expr) {
	if(parser_ast_verify_if(ctx, expr)) {
		// parser_ast_verify_if sets error
		return NULL;
	}
	struct AstNode* if_node = type_ast_alloc_if(ctx);
	if (!if_node) {
		global_set_error_parser_oom(ctx);
		return NULL;
	}
	if_node->ast_if.condition = parser_ast_build(ctx, type_cons_car(expr));
	if (!if_node->ast_if.condition) {
		return NULL;
	}
	struct ExpressionT* body = type_cons_cdr(expr);
	if_node->ast_if.body_true = parser_ast_build(ctx, type_cons_car(body));
	if (!if_node->ast_if.body_true) {
		return NULL;
	}
	// Now false body branch
	body = type_cons_cdr(body);
	if (!body) {
		// There is no false branch
		return if_node;
	}
	if_node->ast_if.body_false = parser_ast_build(ctx, type_cons_car(body));
	if(!if_node->ast_if.body_false) {
		return NULL;
	}
	return if_node;
}

/** Build and allocate function expression
 */
STATIC struct AstNode* parser_ast_build_func(
		struct context* ctx,
		struct ExpressionT* expr) {
	if (!type_iscons(expr) || !type_cons_car(expr)) {
		global_error_syntax(ctx, expr);
		return NULL;
	}
	// TODO: verify name is symbol and that each cons have car

	u32 arg_count = 0;
	{
		struct ExpressionT* arg = expr; // Point to name
		while (arg) {
			arg = type_cons_cdr(arg);
			if (!arg) {
				break;
			}
			arg_count++;
		}
	}

	struct AstNode* func_node = type_ast_alloc_func(ctx, expr);
	if (!func_node) {
		global_set_error_parser_oom(ctx);
		return NULL;
	}
	func_node->ast_func.name = type_cons_car(expr);

	struct ExpressionT* curr_cons = type_cons_cdr(expr);
	struct ExpressionT* last_argument = NULL; // Points inside AstNode
	while (curr_cons) {
		struct ExpressionT* raw_argument = type_cons_car(curr_cons);
		struct AstNode *argument = parser_ast_build(ctx, raw_argument);
		struct ExpressionT* cons_cell = type_cons_alloc_stack(ctx);
		if (!argument || !cons_cell) { // Subtree failed
			return NULL;
		}
		type_cons_set_car(cons_cell, argument);
		if (!last_argument) { // This is first argument
			func_node->ast_func.args = cons_cell;
			last_argument = cons_cell;
		} else { // Just append argument to last one
			type_cons_set_cdr(last_argument, cons_cell);
			last_argument = cons_cell;
		}
		curr_cons = type_cons_cdr(curr_cons);
	}
	return func_node;
}

STATIC struct ExpressionT* _parser_ast_build_progn_internal(struct context* ctx, struct ExpressionT* expr, u32 num_expressions) {
	struct ExpressionT* array_ptr =
		type_array_heapalloc(ctx, num_expressions, sizeof(void*));
	if (!array_ptr) {
		global_set_error_parser_oom(ctx);
		return NULL;
	}
	for (u32 i = 0; i < num_expressions; i++) {
		struct ExpressionT* line = type_cons_car(expr);
		// TODO: This should be parsed here
		if (!line) {
			global_set_error_ast(ctx, expr);
			return NULL;
		}
		struct AstNode* parsed_node = parser_ast_build(ctx, line);
		if (!parsed_node) { // Error is set by parser_ast_build
			return NULL;
		}
		type_array_push_back(array_ptr, &parsed_node);
		expr = type_cons_cdr(expr);
	}
	return array_ptr;
}

/** Build Ast for progn, given pointer to first element after "progn".
 *  Currently progn is supported only in form where all members are
 *  s-expressions, and there must be at least one.
 *  Minimal progn:
 *  (progn
 *    (f-call))
 */
STATIC struct AstNode* parser_ast_build_progn(struct context* ctx, struct ExpressionT* expr) {
	if (!type_iscons(expr)) {
		global_set_error_ast(ctx, expr);
		return NULL;
	}

        // First count number of s-exps in body, so we can allocate big enough
        // array
        u32 num_expressions = 0;
	struct ExpressionT* tmp_expr = expr;
	while (1) {
		if (!type_iscons(tmp_expr)) {
			if (tmp_expr == NULL) {
				// We are at the end
				break;
			} else {
				global_set_error_ast(ctx, tmp_expr);
			}
		}
		// Now check if progn line is another s-exp. If not, it is
		// probably just value, and it is error
		if (!type_iscons(type_cons_car(tmp_expr))) {
			global_set_error_unsupported_ast(ctx, tmp_expr);
			return NULL;
		}
		tmp_expr = type_cons_cdr(tmp_expr);
		num_expressions++;
	}

	// Progn must have at least one member
	if (num_expressions == 0) {
		global_set_error_unsupported_ast(ctx, expr);
		return NULL;
	}
	// Now call function that builds array
	struct ExpressionT* functions_array =
		_parser_ast_build_progn_internal(ctx, expr, num_expressions);
	if (!functions_array) {
		// Error will be set by _parser_ast_build_progn_internal
		return NULL;
	}
	struct AstNode* result = type_ast_alloc_progn(ctx);
	if (!result) {
		global_set_error_parser_oom(ctx);
		return NULL;
	}
	result->ast_progn.functions = functions_array;
	return result;
}

/* Builder of absctract syntax tree.
 *
 * Ast build is second phase of parsing, which recognizes in parsed
 * s-expressions important structures.  Structures like ifs gotos and variable
 * declarations.
 *
 * This is top function, which then calls structure specific parsers.  These
 * parsers are also usually split in two functions, verifier - which checks
 * correct form and builder, which  acutally cretes AST structure.  This is
 * done to keep functions simpler.
 *
 */
struct AstNode* parser_ast_build(struct context* ctx, struct ExpressionT* expr) {
	if (type_isstring(expr) || type_is_symbol(expr) || type_isnumber(expr)) {
		struct AstNode* ret = type_ast_alloc_value(ctx, expr);
		if (!ret) {
			global_set_error_parser_oom(ctx);
			return NULL;
		}
		return ret;
	}

	if (!type_iscons(expr)) {
		goto error;
	}
	struct ExpressionT* t = type_cons_car(expr);
	if (!type_is_symbol(t)) {
		goto error;
	}
	u32 symbol_size = type_symbol_get_size(t);
	const char* symbol_str = type_symbol_get_str(t);

	const char keyword_if[] = {'i', 'f'};
	const char keyword_let[] = {'l', 'e', 't'};
	const char keyword_progn[] = {'p', 'r', 'o', 'g', 'n'};
	const char keyword_goto[] = {'g', 'o', 't', 'o'};


	// AST_IF
	if (symbol_size == sizeof(keyword_if)
			&& c_memcmp(keyword_if, symbol_str, 2) == 0) {
		struct AstNode* node = parser_ast_build_if(ctx, type_cons_cdr(expr));
		if (!node) {
			DEBUG_ERROR("If is wrong");
			// TODO: Build error
			return NULL;
		}
		return node;
	// AST_LET
	} else if (symbol_size == sizeof(keyword_let)
			&& c_memcmp(keyword_let, symbol_str, sizeof(keyword_let)) == 0) {
		struct AstNode* node = parser_ast_build_let(ctx, type_cons_cdr(expr));
		if (!node) {
			DEBUG_ERROR("Let is wrong");
			return NULL;
		}
		return node;
	// AST_PROGN
	} else if (symbol_size == sizeof(keyword_progn)
			&& c_memcmp(keyword_progn, symbol_str, sizeof(keyword_progn)) == 0) {
		struct AstNode* node = parser_ast_build_progn(ctx, type_cons_cdr(expr));
		if (!node) {
			// parser_ast_build_progn sets error
			return NULL;
		}
		return node;
	// AST_GOTO
	} else if (symbol_size == sizeof(keyword_goto)
			&& c_memcmp(keyword_goto, symbol_str, sizeof(keyword_goto)) == 0) {
		struct AstNode* node = parser_ast_build_goto(ctx, type_cons_cdr(expr));
		if (!node) {
			// parser_ast_build_goto sets error
			return NULL;
		}
		return node;
	} else { // Just regular function call
		 struct AstNode* node = parser_ast_build_func(ctx, expr);
		 if (!node) {
			 // TODO: Build error
			 DEBUG_ERROR("cannot allocate ast node");
			 return NULL;
		 } else {
			 return node;
		 }
	}
error:
	return NULL;
}

// ============================
//   END PARSERS
// ============================

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

/** Get address for return value. This is very hacky and will be replaced.
 *
 */
struct ExpressionT** interpreter_get_ret_addr(struct context* ctx) {
	u64 count = interpreter_get_arg_count(ctx);
	if (count == 0) {
		DEBUG_ERROR("Abort, stack return NULL");
		return NULL; // TMP
	}
	u64 position = (count + 1) *8;
	struct ExpressionT** exp =
		(struct ExpressionT**)stack_get_ptr(ctx, position);
	if (!exp) {
		DEBUG_ERROR("Abort! stack get returns NULL");
		return NULL; // Temporary
	}
	return exp;
}

/** Check if given expression is function call.
 *  Returns either TRUE or FALSE.
 */
i64 interpreter_is_expr_function_call(struct ExpressionT* expr) {
	if (!type_iscons(expr)) {
		return FALSE;
	}
	struct ExpressionT* first = type_cons_car(expr);
	if (first && first->expr_type == SYMBOL) {
		return TRUE;
	}
	return FALSE;
}

/** This function goes through parent tree and finds nth node pointing to expr
 *  Returns index of nth child on success or 0 on error. 0 is never valid as
 *  index of child, because 0 doesn't have any parent and points to itself.
 *
 */
u64 interpreter_find_nth_child_func_index(
		struct context* ctx, struct ExpressionT* expr, u64 n) {

	struct ExpressionT* parents = ctx->parent_table;
	if (!type_ismem(parents)) {
		DEBUG_ERROR("Nth child index bad parent_table");
		return 0;
	}
	u32 parent_len = type_mem_get_len(parents) / sizeof(u64);
	u64 found_children = 0;

	for (u32 i = 0; i < parent_len; i++) {
		u64* value = type_mem_get_u64(parents, i);
		// Value can't be NULL
		if (((struct ExpressionT*)*value) == expr)
			found_children++;

		if (found_children == n)
			return i;
	}
	return 0;
}

/** Push arguments on stack and resolve function return values.
 *  expr is pointer to function call in syntax tree.
 */
i64 interpreter_push_args(struct context* ctx, struct ExpressionT* expr) {
	u64 arg_count = 0;
	u64 function_calls = 0;

	struct ExpressionT* rest = type_cons_cdr(expr);

	if(!type_iscons(expr) || !type_is_symbol(type_cons_car(expr))) {
		DEBUG_ERROR("Invalid function push args");
		return -1;

	}

	while (rest) {
		struct ExpressionT * val = type_cons_car(rest);
		if (!val) {
			DEBUG_ERROR("Invalid function call, (parser error)");
			return -1;
		}

		// If this is function call, we have to replace the literal
		// function call with return value of func call.
		if (interpreter_is_expr_function_call(val)) {
			function_calls++;
			u64 child_index = interpreter_find_nth_child_func_index(
					ctx, expr, function_calls);
			if (child_index == 0) {
				DEBUG_ERROR("unresolved deps in call graph");
				return -1;
			}
			// Pull real return value
			val = (struct ExpressionT*)
				(*type_mem_get_u64(ctx->return_values, child_index));
		}

		void *stackp = stack_push_u64(ctx, (u64)val);
		if (!stackp) {
			DEBUG_ERROR("Stack push failed");
			return -1;
		}
		rest = type_cons_cdr(rest);
		arg_count++;
	}
	stack_push_u64(ctx, arg_count);
	return 0;
}

/** Get nested function call pointer.
 * Positions are indexed from 1. Position 2 means - try to find second function
 * call and return its pointer.
 *
 * Returns pointer to function call or NULL
 */
struct ExpressionT* interpreter_get_nested_func(
		struct ExpressionT* expr, u64 position) {
	if (!type_iscons(expr)) {
		DEBUG_ERROR("Expression is not a list (get nested func)");
		return NULL;
	}
	u64 found_count = 0;
	struct ExpressionT* current = (expr);
	while (current) {
		struct ExpressionT* value = type_cons_car(current);
		if (interpreter_is_expr_function_call(value)) {
			found_count++;
			if (found_count == position) {
				return value;
			}
		}
		current = type_cons_cdr(current);
	}
	return NULL;
}

void builtin_concat(struct context* ctx);
void builtin_stdout(struct context* ctx);
void builtin_read_file(struct context* ctx);

u64 _interpreter_count_expr_nodes_internal(struct ExpressionT* expr) {
	if (!type_iscons(expr)) {
		return 0;
	}
	struct ExpressionT* car = type_cons_car(expr);
	if (!type_is_symbol(car)) {
		DEBUG_ERROR("Parsing some list - unsupported yet");
		return 0;
	}

	struct ExpressionT* cons_expr = type_cons_cdr(expr);
	u64 total_children = 0;
	while (cons_expr) {
		total_children += _interpreter_count_expr_nodes_internal(
			type_cons_car(cons_expr));
		cons_expr = type_cons_cdr(cons_expr);
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

/** Retrieve function by index.
 * Currently this causes recursive search, but I don't care about speed rn.
 */
struct ExpressionT* interpreter_get_function_by_index(
		struct context* ctx, u64 order) {

	struct ExpressionT* node = ctx->program;
	u64 index = 0;
	u64 counted = 0;

	while (order != index) {
		int i = 0;
		while (1) {
			i++; // nested functions start at 1
			struct ExpressionT* tmp =
				interpreter_get_nested_func(node, i);

			if (!tmp) { // Order arg must be out of range
				DEBUG_ERROR("Cannot find by index");
				return NULL;
			}

			u64 current_node_count =
				_interpreter_count_expr_nodes_internal(tmp);

			if ((current_node_count + counted) >= order) {
				// This is subtree we want to dive into
				index = counted + 1;
				node = tmp;
				break;

			} else {
				counted += current_node_count;
			}
		}
	}
	return node;
}

struct ExpressionT* interpreter_call_function(
		struct context* ctx, struct ExpressionT* expr) {
	struct ExpressionT* ret = NULL;
	struct ExpressionT* func = type_cons_car(expr);
	if (!func) {
		DEBUG_ERROR("Func error");
		return ret;
	}
	if (func->expr_type == SYMBOL) {
		sys_write(1, "===", 3);
		sys_write(1, func->value_symbol.content, func->value_symbol.size);
		sys_write(1, "===\n", 4);
	}

	u64 stack_pointer = stack_get_sp(ctx);
	void* ret_val_pointer = stack_push_u64(ctx, 0); // Space for return value

	if (interpreter_push_args(ctx, expr) != 0) {
		goto end;
	}

	void (*builtin_function)(struct context*) = NULL;

	builtin_function = type_assoca_get(ctx->builtins,
			func->value_symbol.content, func->value_symbol.size);

	if (!builtin_function) {
		// TODO: Panic
		DEBUG_ERROR("Error unknown function");
		goto end;
	}

	// Let's assume its concat
	builtin_function(ctx);

	// Now get return value
	ret = *((struct ExpressionT**)ret_val_pointer);
end:
	stack_set_sp(ctx, stack_pointer);
	return ret;
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

u64 interpreter_load_builtins(struct context* ctx) {
	struct ExpressionT* assoca = type_assoca_alloc(ctx, 6);
	if (!assoca) {
		return 1;
	}

	{
		char concat[] = {'c', 'o', 'n', 'c', 'a', 't'};
		u64 ret = type_assoca_insert(
				assoca, concat, 6, (u64)builtin_concat);
		if (ret != 0) {
			return 1;
		}
	}

	{
		char write[] = {'w', 'r', 'i', 't', 'e'};
		u64 ret = type_assoca_insert(
				assoca, write, 5, (u64)builtin_stdout);
		if (ret != 0) {
			return 1;
		}
	}

	{
		char rf[] = {'r', 'e', 'a', 'd', 'f', 'i', 'l', 'e'};
		u64 ret = type_assoca_insert(
				assoca, rf, 8, (u64)builtin_read_file);
		if (ret != 0) {
			return 1;
		}
	}

	ctx->builtins = assoca;
	return 0;
}

int execute(struct context* ctx) {
	if(!ctx->program) {
		return -1;
	}

	if (interpreter_load_builtins(ctx)) {
		DEBUG_ERROR("Error loading builtins");
		return-1;
	}

	struct ExpressionT* parents = interpreter_build_parent_graph(ctx);
	struct ExpressionT* ret_vals = type_mem_alloc(ctx, type_mem_get_len(parents));
	if (!parents || !ret_vals) {
		return -1;
	}

	if (type_mem_memset(ret_vals, 0, type_mem_get_len(parents)) == -1) {
		// There is some problem with memory
		return -1;
	}

	ctx->parent_table = parents;
	ctx->return_values = ret_vals;

	// Start with last function
	u64 current_function = type_mem_get_len(parents) / sizeof(u64) - 1;

	// Retrieve function
	while (1) {
		struct ExpressionT* current_f =
			interpreter_get_function_by_index(ctx, current_function);
		struct ExpressionT* fret = NULL;
		fret = interpreter_call_function(ctx, current_f);
		if (fret == NULL) {
			DEBUG_ERROR("Error calling function");
			return -1;
		}
		*type_mem_get_u64(ret_vals, current_function) = (u64)fret;
		// Save return value
		if (current_function == 0) { // This was the last one
			return 0;
		}
		current_function--;
	}
}

void interpreter_set_variable();
void interpreter_create_variable();
void interpreter_delete_variable();

// ============================
//   END INTERPRETER
// ============================


// ============================
//   RUNTIME FUNCTIONS
// ============================

/** Return file size in bytes of file FILENAME
 * On error returns -1
 */
i64 runtime_get_file_size(const char* filename) {
	char buffer[sys_stat_stat_struct_len()];
	if (sys_stat(filename, buffer)) {
		DEBUG_ERROR("Sys stat failed");
		return -1;
	}
	return sys_stat_stat_get_size(buffer);
}

void runtime_panic() {
	DEBUG_ERROR("panic");
	sys_exit(1);
}

/** Get the length of linked list.
 * Returns length on success, -1 on failure.
 */
i64 runtime_list_length(struct ExpressionT* first) {
	i64 list_length = 0;
	while ((first)) {
		if (!type_iscons(first)) { // This list is bad ... error
			DEBUG_ERROR("Bad list to count");
			return -1;
		}
		if (type_cons_car(first) == NULL) { // Weird but ok probably
			DEBUG_ERROR("Empty cons cell ll");
			return list_length;
		}
		list_length++;
		first = type_cons_cdr(first);

	}
	return list_length;
}

/** Read file content to BUFFER up to BUF_SIZE length.
 *  Returns number of bytes read, or -1 on error
 */
i64 runtime_read_file(const char* filename, char* buffer, u32 buf_size) {
	i64 fsize = runtime_get_file_size(filename);
	if (fsize < 0) {
		return -1;
	}
	fsize = fsize > buf_size ? buf_size : fsize;
	i64 fd = sys_open(filename, sys_open_flag_read(), 0);
	if (fd < 1) {
		return -1;
	}
	i64 len_read = sys_read(fd, buffer, fsize);
	if (len_read < 0) { // Set error to -1
		len_read = -1;
	}
	if (sys_close(fd) < 0) {
		DEBUG_ERROR("Sys close failed");
	}
	return len_read;
}

/** Print string to destination and return size of printed string.
 * On Success returns count of written bytes up to max_size.
 * On failiure - if size of string is larger than max size, returns -1
 */
i64 _runtime_format_str(char* destination, u32 max_size, const char* source) {
	const char* src = source;
	for (u32 i = 0; i < max_size; i++) {
		if (*src) {
			*destination = *src;
			destination++;
			src++;
		} else {
			return i;
		}
	}
	if (!(*src)) { // Length was exactly the size of buffer
		return max_size;
	}
	return -1;
}

i64 _runtime_format_int(char* destination, u32 max_size, i64 source) {
	// Handle 0
	if (max_size < 1)
		return -1;
	if (source == 0) {
		*destination = '0';
		return 1;
	}
	i64 written = 0;
	if (source < 0) { // Handle negative
		destination[written++] = '-';
		source *= -1;
	}
	while (source) {
		if (written >= max_size) {
			return -1;
		}
		char modulo  = source % 10;
		source /= 10;
		destination[written++] = modulo + '0';
	}

	// Reverse written number
	u32 destination_shift = 0;
	if (*destination == '-')
		destination_shift = 1;
	for (char *last = destination + written - 1,
			*dest = destination + destination_shift;
			last > dest; last--,dest++) {
		char tmp = *dest;
		*dest = *last;
		*last = tmp;
	}
	return written;
}

/** Classic formatting function. Prints formatted function to destination.
 * On success, returns number of written bytes.
 * On failure retuns negative error code. Currently possible errors are:
 * ERROR_ARGUMENT
 * ERROR_BUFFER_SIZE
 */
i64 runtime_format(const char* format,
		char* buffer,
		u64 buffer_size,
		const u64* argv) {
	const char* fmt = format;
	u32 argv_index = 0;
	u32 buffer_left = buffer_size;
	char is_formatting = FALSE;  // Is next symbol formatter?
	char* destination = buffer;
loop:
	// Error handling
	if (buffer_left == 0) {
		return -ERROR_ARGUMENT;
	}
	if (!*fmt) { // Finish
		if (buffer_left > 0) {
			*destination = 0;
			buffer_left--;
			return buffer_size - buffer_left;

		}
		return -ERROR_ARGUMENT; // Cannot fit ending 0
	}
	// String copying
	if ((*fmt) == '%') {
		is_formatting = TRUE;
		fmt++;
		goto loop;
	}

	if (is_formatting) {
		if (*fmt == 's') {
			i64 format_ret =
				_runtime_format_str(destination,
						buffer_left, (const char*) argv[argv_index]);
			if (format_ret < 0) {
				DEBUG_ERROR("Small buffer (format)");
				return -ERROR_BUFFER_SIZE;
			}
			destination += format_ret;
			buffer_left -= format_ret;
		}
		else if (*fmt == 'd') {
			i64 format_ret =
				_runtime_format_int(destination,
						buffer_left, (i64)argv[argv_index]);
			if (format_ret < 0) {
				DEBUG_ERROR("Small buffer (format)");
				return -ERROR_BUFFER_SIZE;
			}
			destination += format_ret;
			buffer_left -= format_ret;
		} else {
			DEBUG_ERROR("Unsupported format");
			return -ERROR_ARGUMENT; // Unsupported
		}
		argv_index++;
		is_formatting = FALSE;
		fmt++;
	}

	// Just a regular copy
	*destination = *fmt;
	destination++;
	fmt++;
	buffer_left--;
	goto loop;
}

// ============================
//   END RUNTIME FUNCTIONS
// ============================

// ============================
//   DEBUG FUNCTIONS
// ============================

void c_printf0(const char* format_string) {
	char buffer[1000];
	i64 ret = runtime_format(format_string, buffer, sizeof(buffer), NULL);
	if (ret >= 0) {
		sys_write(1, buffer, ret);
	} else {
		sys_write(1, "Printf fail\n", 12);
	}
}

void c_printf1(const char* format_string, void* arg1) {
	char buffer[1000];

	i64 ret = runtime_format(format_string, buffer, sizeof(buffer), (u64*)&arg1);
	if (ret >= 0) {
		sys_write(1, buffer, ret);
	} else {
		sys_write(1, "Printf fail\n", 12);
	}
}

void debug_to_cstring(struct ExpressionT* src, char* dest, u32 max_size) {
	c_memset(dest, 0, max_size);
	if (type_isstring(src)) {
		u32 str_size = type_string_get_length(src);
		c_memcpy(dest,type_string_getp(src),
				str_size > max_size? max_size : str_size);
	} else if (type_is_symbol(src)) {
		u32 symbol_size = type_symbol_get_size(src);
		c_memcpy(dest, type_symbol_get_str(src),
				max_size > symbol_size ? symbol_size : max_size);
	} else if (type_isnumber(src)) {
		runtime_format("%d",dest, max_size, (void*)&(src->value_number));
	} else {
		if (max_size > 1)
			*dest = '?';
	}
}

void debug_print_error(struct context* ctx) {
	switch (ctx->error_code) {
		case 0:
			c_printf0("No Error\n");
			break;
		case ERROR_ARGUMENT:
			c_printf0("Bad argument\n");
			break;
		case ERROR_SYNTAX:
			c_printf0("Bad syntax\n");
			break;
		default:
			c_printf0("Unknown error\n");
	}

}

void _debug_print_ast_space(u32 depth) {
	for (u32 i = 0; i < depth; i++) {
		c_printf0(" ");
	}
}

void debug_print_ast(struct AstNode* ast, u32 depth) {
	_debug_print_ast_space(depth);
	if (!ast) {
		c_printf0("NULL\n");
	} else if (type_ast_isfunc(ast)) {
		struct ExpressionT* fexpr = type_ast_func_expr(ast);
		struct ExpressionT* fname = type_cons_car(fexpr);
		const char* fname_str = type_symbol_get_str(fname);
		u32 fname_size = type_symbol_get_size(fname);
		char fname_buf[fname_size+1];
		fname_buf[fname_size] = 0;
		if (fname_str)
			c_memcpy(fname_buf, fname_str, fname_size);
		c_printf1("FUNC: %s\n", fname_buf);
		struct ExpressionT* argument = ast->ast_func.args;
		while(argument) {
			debug_print_ast(type_cons_car_ast(argument), depth+2);
			argument = type_cons_cdr(argument);
		}
	} else if (type_ast_isval(ast)) {
		char buffer[300];
		debug_to_cstring(ast->ast_value.value, buffer, 300);

		if (type_isstring(ast->ast_value.value)) {
			c_printf1("str: \"%s\"\n", buffer);
		} else {
			c_printf1("val: %s\n", buffer);
		}
	} else if (type_ast_isif(ast)) {
		c_printf0("IF\n");
		debug_print_ast(ast->ast_if.condition, depth+2);
		debug_print_ast(ast->ast_if.body_true, depth+2);
		debug_print_ast(ast->ast_if.body_false, depth+2);
	} else if (type_ast_islet(ast)) {
		c_printf0("LET\n");
		char buffer[100];
		_debug_print_ast_space(depth+2);
		debug_to_cstring(ast->ast_let.name, buffer, 100);
		c_printf1("name: %s\n", buffer);
		_debug_print_ast_space(depth+2);
		debug_to_cstring(ast->ast_let.initial_value, buffer, 100);
		c_printf1("value: %s\n", buffer);
		debug_print_ast(ast->ast_let.function, depth+2);
	} else if (type_ast_isprogn(ast)) {
		c_printf0("PROGN\n");
		for (i32 i = 0, array_len = type_array_len(
					ast->ast_progn.functions);
				i < array_len; i++) {
			debug_print_ast(*((struct AstNode**)
						type_array_get(ast->ast_progn.functions, i)),
					depth+2);
		}
	} else if (type_ast_isgoto(ast)) {
		char buffer[100];
		struct ExpressionT* symbol = ast->ast_goto.symbol;
		u32 symbol_size = type_symbol_get_size(symbol);
		const char* symbol_str = type_symbol_get_str(symbol);
		u32 max_len = lib_min2_u32(symbol_size, 99);
		c_memcpy(buffer, symbol_str, max_len);
		buffer[max_len] = 0;
		c_printf1("GOTO %s\n", buffer);
	}
}

// ============================
//   END DEBUG FUNCTIONS
// ============================

// ============================
//   BUILTINS
// ============================

// Builtins are functions, that are called directly from lisp, e.g. + - concat.
// These functions, cannot be called from C directly, because they work with
// interpreter's stack

void builtin_concat(struct context* ctx) {
	u64 argcount = interpreter_get_arg_count(ctx);
	if (argcount != 2) {
		DEBUG_ERROR("Wrong arg count, fixme message");
		return;
	}
	struct ExpressionT *arg1, *arg2;
	arg1 = interpreter_get_arg(ctx, 1);
	arg2 = interpreter_get_arg(ctx, 2);
	if (!type_isstring(arg1) || !type_isstring(arg2)) {
		DEBUG_ERROR("Fail concat, bad argument types");
		return;
	}
	u64 final_size =
		type_string_get_length(arg1) + type_string_get_length(arg2);
	struct ExpressionT* result = type_string_alloc(ctx, final_size);
	if (!result) {
		DEBUG_ERROR("Fail here, cannot allocate fixme");
		return;
	}
	char *destp = type_string_getp(result);
	c_strcpy_s(destp, type_string_get_length(arg1), type_string_getp(arg1));

	c_strcpy_s(destp + type_string_get_length(arg1),
			type_string_get_length(arg2), type_string_getp(arg2));

	struct ExpressionT** ret_place = interpreter_get_ret_addr(ctx);
	*ret_place = result;
}

void builtin_stdout(struct context* ctx) {
	u64 argcount = interpreter_get_arg_count(ctx);
	if (argcount != 1) {
		DEBUG_ERROR("Wrong arg count");
		return;
	}
	struct ExpressionT *arg1;
	arg1 = interpreter_get_arg(ctx, 1);
	if (!type_isstring(arg1)) {
		DEBUG_ERROR("Fail stdout, bad argument type, str expected");
		return;
	}
	sys_write(1, type_string_getp(arg1), type_string_get_length(arg1));
	struct ExpressionT** ret_place = interpreter_get_ret_addr(ctx);
	*ret_place = (void*)1;
}

void builtin_read_file(struct context* ctx) {
	u64 argcount = interpreter_get_arg_count(ctx);
	if (argcount != 1) {
		DEBUG_ERROR("Wrong arg count");
		return;
	}
	struct ExpressionT *arg1;
	arg1 = interpreter_get_arg(ctx, 1);
	if (!type_isstring(arg1)) {
		DEBUG_ERROR("Fail here, bad argument types");
		return;
	}

	const char* filename = "/etc/pam.conf";
	i64 fsize = runtime_get_file_size(filename);
	if (fsize < 0) {
		// TODO: Better error handling
		DEBUG_ERROR("Cannot determine file size");
		return;
	}
	struct ExpressionT* result_string = type_string_alloc(ctx, fsize);
	if (!result_string) {
		DEBUG_ERROR("Cannot allocate space for file");
		return;
	}
	type_string_set_length(result_string, fsize);
	i64 runtime_ret =
		runtime_read_file(filename, type_string_getp(result_string), fsize);
	if (runtime_ret < 0) {
		DEBUG_ERROR("Runtime cannot read file");
		return;
	}

	struct ExpressionT** ret_place = interpreter_get_ret_addr(ctx);
	*ret_place = (void*)result_string;
}

// ============================
//   END BUILTINS
// ============================

#ifdef TEST
#include "src/tests.c"
#endif

void debug_ast(struct context* ctx) {
	struct AstNode* node = parser_ast_build(ctx, ctx->program);
	debug_print_ast(node, 0);
	char error_buffer[100];
	if (global_format_error(ctx, error_buffer, 100) != 0 ) {
		c_printf0("Critical error\n");
	} else {
		c_printf1("Error %s\n", error_buffer);
	}
}

// ============================
//   REPL
// ============================

// Group of functions, that are used to run language as standalone interpreter,
// instead of embedded thing. This doesn't mean only for repl mode.

void repl_main() {
	u32 argc = platform_get_argc();
	if (argc != 2) { // Repl mode is not currently supported
		c_printf0(REPL_HELP);
		return;
	}
	char* argv = platform_get_argv(1);
	i64 script_size = runtime_get_file_size(argv);
	if (script_size < 0) {
		c_printf0(REPL_FILE_ERROR);
		return;
	}
	char content[script_size];
	i64 result = runtime_read_file(argv, content, script_size);
	if (result < 0) {
		c_printf0(REPL_FILE_ERROR);
		return;
	}
	struct context ctx;
	init_context(&ctx);
	if (parse_program(&ctx, content, c_strlen(content)) < 0) {
		if (global_is_error(&ctx)) {
			char text[100];
			text[99] = 0;
			global_format_error(&ctx, text, 100);
			c_printf1("%s\n",text);
		}

		global_set_error(&ctx, ERROR_PARSER);
		debug_print_error(&ctx);
		sys_exit(1);
	}

	debug_ast(&ctx);
}

// ============================
//   END REPL
// ============================
void _start() {
#ifdef TEST
	run_tests();
#endif
	repl_main();
	sys_exit(0);
}

