void test_assert(char bool_value, const char* some_string, const char* content) {
	if (bool_value) {
		sys_write(1, "OK\t", 3);
		sys_write(1, ".....\t", 6);
	} else {
		sys_write(1, "FAIL\t", 5);
		sys_write(1, content, c_strlen(content));
	}
	sys_write(1, some_string, c_strlen(some_string));
	sys_write(1, "\n", 1);
}

#define _TEST_FORM_STR(a, b) a ## b

#define __LINE_STR(x) #x
#define _LINE_STR(x)  __LINE_STR(x)
#define  TEST_LINE _LINE_STR(__LINE__)


#define TEST_ASSERT(x) test_assert(x,  __FILE__ ":" TEST_LINE , __LINE_STR(x))
#include <src/lib_tests.c>

void run_tests() {
	sys_write(1, "LIB TESTS:\n", 11);
	run_tests_lib();
}

