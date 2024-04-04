#include <sys/types.h>

#define DEBUG 1
#define LOCAL static
#define STATIC static
#define INLINE static inline
#define NULL 0
#define FALSE 0
#define TRUE 1
#define STACK_SIZE 4096
#define HEAP_SIZE 4096
#define U32_MAX 0xffffffff
#define u64 unsigned long long
#define u32 unsigned int
#define u16 unsigned short
#define i32 int
#define i64 signed long long

#define CONST_ERROR_PARSER_BUFFER_SIZE 10

#if DEBUG == 1

#ifndef NOTEST
#define TEST
#endif

// If ran like repl, there is also .data and .rodata and we have strings
#define REPL
#endif

_Static_assert(sizeof(u64) == 8, "Bad size");
_Static_assert(sizeof(i64) == 8, "Bad size");
_Static_assert(sizeof(u32) == 4, "Bad size");
_Static_assert(sizeof(i32) == 4, "Bad size");
_Static_assert(sizeof(u16) == 2, "Bad size");

#if __STDC_VERSION__ != 201710L
#error "Bad c version, use std=c17"
#endif

#ifdef REPL
#define REPL_HEADLINE "This is rasbi. Copyright 2024 Lukas Manduch\n"
#define REPL_HELP "Error, requires one argument - path to script\n"\
	"rasbi <script_file>\n"
#define REPL_FILE_ERROR "Problem opening script to run\n"
#define FORMAT_STRING(simple, complex) complex
#else
#define FORMAT_STRING(simple, complex) simple
#endif
