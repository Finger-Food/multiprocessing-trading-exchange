#include "pe_common.h"
#include "hash_map.h"

#include <stdio.h>
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <string.h>
#include "cmocka.h"

// dictionary hash and comparators
int string_hash(void* data) {
	char* string = (char*)data;
	int result = 0;
	while (*string != '\0') {
		result += *string;
		string++;
	}
	return result;
}

int string_comp(void* a, void* b) {
	return strcmp((char*)a, (char*)b);
}

int int_hash(void* data) {
	return INT(data) * 2;
}

int int_comp(void* a, void* b) {
	return (long)a - (long)b;
}



static int keys[] = { 10, 51, 32, 19 };
static int values[] = { 59, 22, 1004, 3933 };

static int extra_keys[] = { 490, 939, 20, 84, 89, 102, 44 };
static int extra_values[] = { 402, 492, 91, 99, 90, 3, 49 };

static int create_map(void** state) {
	*state = test_malloc(sizeof(hash_map));
	init_map(*state, 4, int_hash, int_comp);

	for (int i = 0; i < sizeof(keys) / sizeof(keys[i]); i++)
		put(*state, VOID(keys[i]), VOID(values[i]));

	return 0;
}

static int destroy_map(void **state) {
	free_map(*state);
    test_free(*state);

    return 0;
}


static void test_get_items(void **state) {
	for (int i = 0; i < sizeof(keys) / sizeof(keys[0]); i++) {
		void* value = get(*state, VOID(keys[i]));
		assert_int_equal(INT(value), values[i]);
	}
}

static void test_put_extra_items(void **state) {
	for (int i = 0; i < sizeof(extra_keys) / sizeof(extra_keys[i]); i++)
		assert_null(put(*state, VOID(extra_keys[i]), VOID(extra_values[i])));

	for (int i = 0; i < sizeof(extra_keys) / sizeof(extra_keys[i]); i++) {
		void* value = get(*state, VOID(extra_keys[i]));
		assert_int_equal(INT(value), extra_values[i]);
	}
}

static void test_replace_item(void **state) {
	int new_val = -1;
	void* old = put(*state, VOID(keys[1]), VOID(new_val));

	assert_int_equal(INT(old), values[1]);

	assert_int_equal(INT(get(*state, VOID(keys[1]))), new_val);
}


static void test_delete_item(void **state) {
	assert_null(del(*state, VOID(9999)));


	void* old = del(*state, VOID(keys[1]));

	assert_int_equal(INT(old), values[1]);

	assert_null(del(*state, VOID(keys[1])));


	assert_null(put(*state, VOID(keys[1]), VOID(-1)));

	void* val = get(*state, VOID(keys[1]));

	assert_int_equal(INT(val), -1);
}


int main(void) {
	printf("-- Testing Map Data Structure --\n");
    const struct CMUnitTest tests[] = {
        cmocka_unit_test_setup_teardown(test_get_items,
                                        create_map, destroy_map),
		cmocka_unit_test_setup_teardown(test_put_extra_items,
										create_map, destroy_map),
		cmocka_unit_test_setup_teardown(test_replace_item,
										create_map, destroy_map),
		cmocka_unit_test_setup_teardown(test_delete_item,
										create_map, destroy_map),
    };
    return cmocka_run_group_tests(tests, NULL, NULL);
}

