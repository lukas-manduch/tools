static inline void run_test_isalpha() {
	TEST_ASSERT(c_isalpha('a') == 1);
	TEST_ASSERT(c_isalpha('z') == 1);
	TEST_ASSERT(c_isalpha('A') == 1);
	TEST_ASSERT(c_isalpha('Z') == 1);
	TEST_ASSERT(c_isalpha('1') == 0);
	TEST_ASSERT(c_isalpha('"') == 0);
}

static inline void run_test_small_functions() {
	TEST_ASSERT(c_isdigit('0') == 1);
	TEST_ASSERT(c_isdigit('1') == 1);
	TEST_ASSERT(c_isdigit('9') == 1);
	TEST_ASSERT(c_isdigit('a') == 0);
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

static inline void run_test_c_strncmp() {
	TEST_ASSERT(c_strncmp("", "", 0) == 0);
	TEST_ASSERT(c_strncmp("abcd", "abc", 3) == 0);
	TEST_ASSERT(c_strncmp("abcd", "abce", 3) == 0);
	TEST_ASSERT(c_strncmp("abcd", "abce", 4) == -1);
}
static inline void run_test_c_memcmp() {
	char a1[] = {'a','b','c'};
	char a2[sizeof a1] = {'a','b','d'};

	TEST_ASSERT(c_memcmp(a1, a2, 3) < 0);
	TEST_ASSERT(c_memcmp(a2, a1, 3) > 0);
	TEST_ASSERT(c_memcmp(a1, a1, 3) == 0);
}

static inline void run_test_c_itoa10() {
	u32 buf_size = 100;
	u64 ret = 0;
	char buffer[buf_size];


	ret = c_itoa10(0, buffer, buf_size);
	TEST_ASSERT(ret == 1);
	buffer[1] = 0;
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

static i64  _cmp(const void* i1, const void* i2) {
	const u64* lhs = i1;
	const u64* rhs = i2;
	return *lhs - *rhs;
}

static inline void run_test_c_sort64() {
	i64 arr1[5];
	i64 target[5];
	arr1[0] = 1;
	c_sort64(arr1, 1, _cmp);
	TEST_ASSERT(arr1[0] == 1);
	arr1[0] = 5;
	arr1[1] = 3;
	arr1[2] = 4;
	arr1[3] = 1;
	arr1[4] = 3;
	//
	target[0] = 1;
	target[1] = 3;
	target[2] = 3;
	target[3] = 4;
	target[4] = 5;
	c_sort64(arr1, 5, _cmp);
	TEST_ASSERT(c_memcmp(arr1, target, 5) == 0);
}

static void test_round8() {
	TEST_ASSERT(round8(2) == 8);
	TEST_ASSERT(round8(0) == 0);
	TEST_ASSERT(round8(8) == 8);
	TEST_ASSERT(round8(9) == 16);
}

static void run_test_c_bsearch() {
	u64 arr1[] = { 1, 2, 3 };

	u64 lf = 3;
	TEST_ASSERT(c_bsearch(&lf, arr1, 3, _cmp) == &arr1[2]);
	lf = 2;
	TEST_ASSERT(c_bsearch(&lf, arr1, 3, _cmp) == &arr1[1]);
	lf = 1;
	TEST_ASSERT(c_bsearch(&lf, arr1, 3, _cmp) == &arr1[0]);
	lf = 0;
	TEST_ASSERT(c_bsearch(&lf, arr1, 3, _cmp) == NULL);
	lf = 4;
	TEST_ASSERT(c_bsearch(&lf, arr1, 3, _cmp) == NULL);

	u64 arr2[] = { 1, 2, 3, 4 };
	lf = 4;
	TEST_ASSERT(c_bsearch(&lf, arr2, 4, _cmp) == &arr2[3]);
	lf = 3;
	TEST_ASSERT(c_bsearch(&lf, arr2, 4, _cmp) == &arr2[2]);
	lf = 2;
	TEST_ASSERT(c_bsearch(&lf, arr2, 4, _cmp) == &arr2[1]);
	lf = 1;
	TEST_ASSERT(c_bsearch(&lf, arr2, 4, _cmp) == &arr2[0]);
	TEST_ASSERT(c_bsearch(&lf, arr2, 0, _cmp) == NULL);
	lf = 0;
	TEST_ASSERT(c_bsearch(&lf, arr2, 4, _cmp) == NULL);
	lf = 5;
	TEST_ASSERT(c_bsearch(&lf, arr2, 4, _cmp) == NULL);
}

void run_tests_lib() {
	TEST_ASSERT(c_strlen("Ahoj") == 4);
	TEST_ASSERT(c_strlen("") == 0);

	test_round8();
	run_test_isalpha();
	run_test_small_functions();
	run_test_c_strcmp();
	run_test_c_strncmp();
	run_test_c_itoa10();
	run_test_c_memset();
	run_test_c_memcmp();
	run_test_c_sort64();
	run_test_c_bsearch();
}
