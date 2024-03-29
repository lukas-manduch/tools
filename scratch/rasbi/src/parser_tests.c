static void run_test_parse_number() {
	struct context ctx;
	init_context(&ctx);
	u64 processed = 0;
	struct ExpressionT* result = NULL;

	const char* str1 = "0";
	const char* str2 = "-0";
	const char* str3 = "-123";
	const char* str4 = "133 ";
	const char* str5 = "3)";

	const char* err1 = "3a";

	result = parser_parse_number(str1, 1, &processed, &ctx);
	TEST_ASSERT(processed == 1);
	TEST_ASSERT(type_isnumber(result) == 1);
	TEST_ASSERT(type_number_get_value(result) == 0);

	result = parser_parse_number(str2, 2, &processed, &ctx);
	TEST_ASSERT(processed == 2);
	TEST_ASSERT(type_isnumber(result) == 1);
	TEST_ASSERT(type_number_get_value(result) == 0);

	result = parser_parse_number(str3, 4, &processed, &ctx);
	TEST_ASSERT(processed == 4);
	TEST_ASSERT(type_isnumber(result) == 1);
	TEST_ASSERT(type_number_get_value(result) == -123);

	result = parser_parse_number(str4, 4, &processed, &ctx);
	TEST_ASSERT(processed == 3);
	TEST_ASSERT(type_isnumber(result) == 1);
	TEST_ASSERT(type_number_get_value(result) == 133);

	result = parser_parse_number(str5, 4, &processed, &ctx);
	TEST_ASSERT(processed == 1);
	TEST_ASSERT(type_isnumber(result) == 1);
	TEST_ASSERT(type_number_get_value(result) == 3);

	result = parser_parse_number(err1, 2, &processed, &ctx);
	TEST_ASSERT(result == NULL);
}
static void run_test_symbol(struct context* ctx) {
	const char* symbol1 = "aa";
	const char* symbol2 = "( aa )";
	const char* symbol3 = " + ";
	const char* not_symbol1 = "22";
	u64 out = 0;
	i64 cmp_result = 0;
	// symbol1
	struct ExpressionT* result = parser_parse_expression(symbol1, c_strlen(symbol1), ctx, &out);
	TEST_ASSERT(type_is_symbol(result) == 1);
	// symbol2
	result = parser_parse_expression(symbol2, c_strlen(symbol2), ctx, &out);
	TEST_ASSERT(type_iscons(result) == 1);
	TEST_ASSERT(type_is_symbol(type_cons_car(result)) == 1);
	TEST_ASSERT(type_cons_cdr(result) == NULL);
	result = type_cons_car(result);
	cmp_result = c_strncmp(symbol1, type_symbol_get_str(result), type_symbol_get_size(result));
	TEST_ASSERT(cmp_result == 0);
	// symbol3
	result = parser_parse_expression(symbol3, c_strlen(symbol3), ctx, &out);
	TEST_ASSERT(type_is_symbol(result) == 1);
	// not_symbol1
	result = parser_parse_expression(not_symbol1, c_strlen(not_symbol1), ctx, &out);
	TEST_ASSERT(type_is_symbol(result) == 0);
}

void run_tests_parser() {
	struct context ctx;
	init_context(&ctx);
	run_test_symbol(&ctx);
	run_test_parse_number();
}
