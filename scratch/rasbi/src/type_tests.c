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

void test_assoca_insert1(struct ExpressionT* assoca) {
	// insert 3 values
	TEST_ASSERT(type_isassoca(assoca));

	TEST_ASSERT(type_assoca_insert(assoca, "alpha", 5, 44) == 0);
	TEST_ASSERT(type_assoca_insert(assoca, "xenix", 5, 55) == 0);
	TEST_ASSERT(type_assoca_insert(assoca, "lexa", 4, 66) == 0);
}

void test_assoca_content1(struct ExpressionT* assoca) {
	u64 ret = 0;
	ret = (u64)type_assoca_get(assoca, "alpha", 5);
	TEST_ASSERT(ret == 44);

	ret = (u64)type_assoca_get(assoca, "lexa", 4);
	TEST_ASSERT(ret == 66);

	ret = (u64)type_assoca_get(assoca, "xenix", 5);
	TEST_ASSERT(ret == 55);

	ret = (u64)type_assoca_get(assoca, "lexa", 3);
	TEST_ASSERT(ret == 0);
}

void test_assoca_copy() {
	struct context _ctx;
	struct context *ctx = &_ctx;
	init_context(ctx);
	struct ExpressionT* ass = type_assoca_alloc(ctx, 6);
	struct ExpressionT* ass2 = type_assoca_alloc(ctx, 6);
	test_assoca_insert1(ass);
	test_assoca_content1(ass);
	type_assoca_copy(ass2, ass);
	test_assoca_content1(ass2);
}

void test_assoca_get_index() {
	struct context _ctx;
	struct context *ctx = &_ctx;
	init_context(ctx);
	struct ExpressionT* ass = type_assoca_alloc(ctx, 6);
	test_assoca_insert1(ass);
	TEST_ASSERT(type_assoca_len(ass) == 3);
	char buffer[50];
	u64 value;
	TEST_ASSERT(type_assoca_get_by_index(ass, 0, buffer, 10, (void**)(&value)) == 5);
	TEST_ASSERT(c_memcmp(buffer, "alpha", 5) == 0);
	TEST_ASSERT(value == 44);

	TEST_ASSERT(type_assoca_get_by_index(ass, 1, buffer, 10, (void**)(&value)) == 4);
	TEST_ASSERT(c_memcmp(buffer, "lexa", 4) == 0);
	TEST_ASSERT(value == 66);

	TEST_ASSERT(type_assoca_get_by_index(ass, 3, buffer, 10, (void**)(&value)) < 0);
}

void test_assoca_pack() {
	struct context _ctx;
	struct context *ctx = &_ctx;
	init_context(ctx);
	struct ExpressionT* ass = type_assoca_alloc(ctx, 3);
	TEST_ASSERT(type_isassoca(ass));
	TEST_ASSERT(type_assoca_insert(ass, "aaaabbbbccccddddeeee", 12, 44) == 0);
	TEST_ASSERT(type_assoca_insert(ass, "baaabbbbccccddddeeee", 12, 54) == 0);
	TEST_ASSERT(type_assoca_insert(ass, "caaabbbbccccddddeeee", 12, 64) == 0);
	// should be full

	TEST_ASSERT(type_assoca_insert(ass, "eee", 3, 44) == 1);
	TEST_ASSERT(type_assoca_get(ass, "baaabbbbccccddddeeee", 12) != 0);
	TEST_ASSERT(type_assoca_delete(ass, "baaabbbbccccddddeeee", 12) == 0);
	// This insert will fail if pack is not working
	TEST_ASSERT(type_assoca_insert(ass, "eee", 3, 44) == 0);
}

void run_test_types_assoca() {
	struct context _ctx;
	struct context *ctx = &_ctx;
	init_context(ctx);

	struct ExpressionT* ass = type_assoca_alloc(ctx, 6);
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

	test_assoca_copy();
	test_assoca_pack();
	test_assoca_get_index();
}

static void run_test_types_array() {
	struct context _ctx;
	struct context *ctx = &_ctx;
	init_context(ctx);

	const char* element1 = "12345";
	const char* element2 = "67890";
	const char* element3 = "abcde";

	struct ExpressionT* array = type_array_heapalloc(ctx, 3, 4);
	TEST_ASSERT(type_isarray(array));

	TEST_ASSERT(type_array_push_back(array, element1) == 0);
	TEST_ASSERT(type_array_push_back(array, element2) == 0);
	TEST_ASSERT(type_array_push_back(array, element3) == 0);
	TEST_ASSERT(type_array_push_back(array, element1) == 1);

	void* data1 = type_array_get(array, 0);
	void* data2 = type_array_get(array, 1);
	void* data3 = type_array_get(array, 2);

	TEST_ASSERT(c_memcmp(data1, element1, 4) == 0);
	TEST_ASSERT(c_memcmp(data2, element2, 4) == 0);
	TEST_ASSERT(c_memcmp(data3, element3, 4) == 0);
}

void run_tests_types() {
	run_test_types_mem();
	run_test_types_varchar();
	run_test_types_assoca();
	run_test_types_array();
}
