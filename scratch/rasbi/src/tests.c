void _test_color_red() {
	sys_write(1, "\033[1;31m", 7);
}

void _test_color_green() {
	sys_write(1, "\033[0;32m", 7);
}

void _test_color_reset() {
	sys_write(1, "\033[0m", 4);
}

void test_assert(char bool_value, const char* some_string, const char* content) {
	if (bool_value) {
		_test_color_green();
		sys_write(1, " OK\t", 4);
		_test_color_reset();
		sys_write(1, ".....\t", 6);
	} else {
		_test_color_red();
		sys_write(1, "FAIL\t", 5);
		sys_write(1, content, c_strlen(content));
		_test_color_reset();
		sys_write(1, "  FILE: ", 8);
	}
	sys_write(1, some_string, c_strlen(some_string));
	sys_write(1, "\n", 1);
	if (!bool_value)
		sys_exit(1);
}

#define _TEST_FORM_STR(a, b) a ## b

#define __LINE_STR(x) #x
#define _LINE_STR(x)  __LINE_STR(x)
#define  TEST_LINE _LINE_STR(__LINE__)


#define TEST_ASSERT(x) test_assert(x,  __FILE__ ":" TEST_LINE , __LINE_STR(x))
#include <src/parser_tests.c>
#include <src/lib_tests.c>
#include <src/type_tests.c>
#include <src/runtime_tests.c>

void run_tests() {
	sys_write(1, "LIB TESTS:\n", 11);
	run_tests_lib();
	sys_write(1, "TYPE TESTS:\n", 12);
	run_tests_types();
	run_tests_runtime();
	run_tests_parser();
}

