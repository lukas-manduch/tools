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

	TEST_ASSERT(*((u64*)type_mem_get_loc(mem, 8)) == 123456789);

	TEST_ASSERT(*type_mem_get_u64(mem, 0) == 23);
	TEST_ASSERT(*type_mem_get_u64(mem, 0) == 23);
	TEST_ASSERT(*type_mem_get_u64(mem, 1) == 123456789);
	TEST_ASSERT(*type_mem_get_u64(mem, 2) == 0);
	*type_mem_get_u64(mem, 2) = 1;
	TEST_ASSERT(*type_mem_get_u64(mem, 2) == 1);

	// fail
	TEST_ASSERT(type_mem_get_u64(mem, 3) == NULL);
	TEST_ASSERT(type_mem_get_u64(mem, 4) == NULL);
	TEST_ASSERT(type_mem_memset(mem, 1, sizeof(u64)*5) == -1);
	// end fail

	TEST_ASSERT(type_mem_memset(mem, 1, sizeof(u64)*4) == 0);
	TEST_ASSERT(((*type_mem_get_u64(mem, 1))&0xFF) == 1);
}

void run_test_types_varchar() {
	char space[30];

	type_varchar_create(space, "Ahojj", 6);
	TEST_ASSERT(type_varchar_get_size((struct Varchar*)space) == 8);
	type_varchar_create(space, "A", 1);
	TEST_ASSERT(type_varchar_get_size((struct Varchar*)space) == 8);
	type_varchar_create(space, "AbcdAbcdA", 9);
	TEST_ASSERT(type_varchar_get_size((struct Varchar*)space) == 16);
}


void run_test_types_assoca() {
	struct context _ctx;
	struct context *ctx = &_ctx;
	init_context(ctx);

	struct ExpressionT* ass = type_assoca_alloc(ctx);
	TEST_ASSERT(ass != NULL);
	TEST_ASSERT(type_isassoca(ass));
	TEST_ASSERT(type_ismem(ass));

	// Test alignments
	u64 space1 = _type_assoca_count_free_space(ass);
	TEST_ASSERT((space1 % 8) == 0);
	type_assoca_insert(ass, "asd", 3, 11);
	u64 space2 = _type_assoca_count_free_space(ass);
	TEST_ASSERT((space2 % 8) == 0);
	TEST_ASSERT((space1 - space2) == 16);

	type_assoca_insert(ass, "concat", 6, 123456);
	u64 space3 = _type_assoca_count_free_space(ass);
	TEST_ASSERT((space3 % 8) == 0);
	TEST_ASSERT((space2 - space3) == 16);

	// Test value retrieval
	u64 val1 = (u64)type_assoca_get(ass, "asd", 3);
	u64 val2 = (u64)type_assoca_get(ass, "concat", 6);
	TEST_ASSERT(val1 == 11);
	TEST_ASSERT(val2 == 123456);

	// Test reinsertion
	TEST_ASSERT(type_assoca_insert(ass, "asd", 3, 9876) == 0);
	val1 = (u64)type_assoca_get(ass, "asd", 3);
	TEST_ASSERT(val1 == 9876);
	TEST_ASSERT(type_assoca_insert(ass, "concat", 6, 9875) == 0);
	val2 = (u64)type_assoca_get(ass, "concat", 6);
	TEST_ASSERT(val2 == 9875);
}

void run_tests_types() {
	run_test_types_mem();
	run_test_types_varchar();
	run_test_types_assoca();
}
