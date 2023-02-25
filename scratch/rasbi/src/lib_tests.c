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

static inline void run_test_c_itoa10() {
	u32 buf_size = 100;
	u64 ret = 0;
	char buffer[buf_size];



	ret = c_itoa10(0, buffer, buf_size);
	TEST_ASSERT(ret == 1);
	TEST_ASSERT(c_strcmp(buffer, "0") == 0);

	ret = c_itoa10(5, buffer, buf_size);
	buffer[1] = 0;
	TEST_ASSERT(ret == 1);
	TEST_ASSERT(c_strcmp(buffer, "5") == 0);

	ret = c_itoa10(-1, buffer, buf_size);
	buffer[2] = 0;
	TEST_ASSERT(ret == 2);
	TEST_ASSERT(c_strcmp(buffer, "-1") == 0);

	ret = c_itoa10(9870, buffer, buf_size);
	buffer[4] = 0;
	TEST_ASSERT(ret == 4);
	TEST_ASSERT(c_strcmp(buffer, "9870") == 0);

	ret = c_itoa10(-100, buffer, buf_size);
	buffer[4] = 0;
	TEST_ASSERT(ret == 4);
	TEST_ASSERT(c_strcmp(buffer, "-100") == 0);


	// Failures
	ret = c_itoa10(-100, buffer, 3);
	TEST_ASSERT(ret == -1ull);
	ret = c_itoa10(100, buffer, 3);
	TEST_ASSERT(ret == 3);
	ret = c_itoa10(0, buffer, 0);
	TEST_ASSERT(ret == -1ull);
	ret = c_itoa10(1, buffer, 0);
	TEST_ASSERT(ret == -1ull);
}

static inline void run_test_c_memset() {
	char aa[] = {7 , 7 , 7 , 8};
	c_memset(aa+1, 4, 2);
	TEST_ASSERT(aa[0] == 7);
	TEST_ASSERT(aa[1] == 4);
	TEST_ASSERT(aa[2] == 4);
	TEST_ASSERT(aa[3] == 8);
}

void run_tests_lib() {
	TEST_ASSERT(c_strlen("Ahoj") == 4);
	TEST_ASSERT(c_strlen("") == 0);

	run_test_is_alphabet();
	run_test_c_strcmp();
	run_test_c_itoa10();
	run_test_c_memset();
}
