

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

static inline void run_test_runtime_format_int() {
	char buffer[6];

	TEST_ASSERT(_runtime_format_int(buffer, 5, (i64)12345) == 5);
	TEST_ASSERT(c_strncmp(buffer, "12345", 5) == 0);

	TEST_ASSERT(_runtime_format_int(buffer, 5, (i64)1) == 1);
	TEST_ASSERT(c_strncmp(buffer, "1", 1) == 0);

	TEST_ASSERT(_runtime_format_int(buffer, 5, (i64)0) == 1);
	TEST_ASSERT(c_strncmp(buffer, "0", 1) == 0);

	// Negative numbers
	TEST_ASSERT(_runtime_format_int(buffer, 5, (i64)-1) == 2);
	TEST_ASSERT(c_strncmp(buffer, "-1", 2) == 0);

	TEST_ASSERT(_runtime_format_int(buffer, 6, (i64)-88881) == 6);
	TEST_ASSERT(c_strncmp(buffer, "-88881", 6) == 0);

	// Test errors
	TEST_ASSERT(_runtime_format_int(buffer, 2, (i64)-1) == 2);
	TEST_ASSERT(_runtime_format_int(buffer, 2, (i64)-11) == -1);
	TEST_ASSERT(_runtime_format_int(buffer, 1, (i64)9) == 1);
	TEST_ASSERT(_runtime_format_int(buffer, 1, (i64)0) == 1);
	TEST_ASSERT(_runtime_format_int(buffer, 0, (i64)5) == -1);
	TEST_ASSERT(_runtime_format_int(buffer, 0, (i64)0) == -1);
	TEST_ASSERT(_runtime_format_int(buffer, 1, (i64)-1) == -1);
}

static inline void run_test_runtime_format() {
	char destination[100];
	const char* str1 = "Peter";
	u64 num1 = 88;
	const u64* args1[] = {(u64*)str1, (u64*)num1};
	TEST_ASSERT(runtime_format("Hello %s! Your happy number is %d.", destination, 100, (const u64* )args1) == 38);
	TEST_ASSERT(c_strcmp("Hello Peter! Your happy number is 88.", destination) == 0);
}

void run_tests_runtime() {
	run_test_runtime_format_str();
	run_test_runtime_format();
	run_test_runtime_format_int();
}
