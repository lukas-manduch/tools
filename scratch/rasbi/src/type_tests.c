void run_test_types_mem() {
	struct context _ctx;
	struct context *ctx = &_ctx;
	init_context(ctx);
	struct ExpressionT* mem = type_mem_alloc(ctx, 4*sizeof(u64));
	TEST_ASSERT(mem != NULL);
	TEST_ASSERT(mem->value_memory.taken == 0);
	TEST_ASSERT(type_mem_get_len(mem) == 0);

	TEST_ASSERT(type_mem_push_u64(mem, 23) == 0);
	TEST_ASSERT(type_mem_push_u64(mem, 123456789) == 1);
	TEST_ASSERT(type_mem_push_u64(mem, 0) == 2);

	TEST_ASSERT(type_mem_get_len(mem) == 3*8);

	TEST_ASSERT(*type_mem_get_u64(mem, 0) == 23);
	TEST_ASSERT(*type_mem_get_u64(mem, 0) == 23);
	TEST_ASSERT(*type_mem_get_u64(mem, 1) == 123456789);
	TEST_ASSERT(*type_mem_get_u64(mem, 2) == 0);
	*type_mem_get_u64(mem, 2) = 1;
	TEST_ASSERT(*type_mem_get_u64(mem, 2) == 1);
	// fail
	TEST_ASSERT(type_mem_get_u64(mem, 3) == NULL);
	TEST_ASSERT(type_mem_get_u64(mem, 4) == NULL);

	/// TEST
}
void run_tests_types() {
	run_test_types_mem();
}
