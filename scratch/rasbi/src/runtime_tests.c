

static inline void run_test_runtime_format_str() {
	char buffer[20];
	const char* copy_me1 = "Hello";
	const char* copy_me2 = "Mi nombre es Juan";
	const char* copy_me3 = "Help";

	buffer[5] = 'x';
	TEST_ASSERT(_runtime_format_str(buffer, 5, copy_me1) == 5);
	TEST_ASSERT(buffer[5] == 'x');
	TEST_ASSERT(_runtime_format_str(buffer, 5, copy_me2) == -1);
	TEST_ASSERT(_runtime_format_str(buffer, 5, copy_me3) == 4);
}

static inline void run_test_runtime_format() {
	char destination[100];
	const char* str1 = "Peter";
	const char* args1[] = {str1};
	TEST_ASSERT(runtime_format("Hello %s!", destination, 100, (const u64* )args1) == 13);
	TEST_ASSERT(c_strcmp("Hello Peter!", destination) == 0);
}

void run_tests_runtime() {
	run_test_runtime_format_str();
	run_test_runtime_format();
}
