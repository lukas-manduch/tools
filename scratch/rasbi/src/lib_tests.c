static inline void run_test_is_alphabet() {
	TEST_ASSERT(is_alphabet('a') == 1);
	TEST_ASSERT(is_alphabet('z') == 1);
	TEST_ASSERT(is_alphabet('A') == 1);
	TEST_ASSERT(is_alphabet('Z') == 1);
	TEST_ASSERT(is_alphabet('1') == 0);
	TEST_ASSERT(is_alphabet('"') == 0);
}

static inline void run_test_c_strcmp() {
	TEST_ASSERT(c_strcmp("", "") == 0);
	TEST_ASSERT(c_strcmp("aaa", "aaa") == 0);
	TEST_ASSERT(c_strcmp("Hello World!", "Hello World!") == 0);

	TEST_ASSERT(c_strcmp("Hello World!", "Hello!") < 0);
	TEST_ASSERT(c_strcmp("abCdef", "abcdef") < 0);

	TEST_ASSERT(c_strcmp("abcdef", "ABCDEF") > 0);
	TEST_ASSERT(c_strcmp("Hello World!", "Hello") > 0);
	TEST_ASSERT(c_strcmp("Hello there", "Hello World!") > 0);
}

void run_tests_lib() {
	TEST_ASSERT(c_strlen("Ahoj") == 4);
	TEST_ASSERT(c_strlen("") == 0);

	run_test_is_alphabet();
	run_test_c_strcmp();
}
