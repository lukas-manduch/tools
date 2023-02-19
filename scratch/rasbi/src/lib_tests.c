void run_tests_lib() {
	TEST_ASSERT(c_strlen("Ahoj") == 4);
	TEST_ASSERT(c_strlen("") == 0);

	TEST_ASSERT(is_alphabet('a') == 1);
	TEST_ASSERT(is_alphabet('z') == 1);
	TEST_ASSERT(is_alphabet('A') == 1);
	TEST_ASSERT(is_alphabet('Z') == 1);
	TEST_ASSERT(is_alphabet('1') == 0);
	TEST_ASSERT(is_alphabet('"') == 0);

}
