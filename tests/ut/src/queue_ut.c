#include "int_queue.h"

#include <stdio.h>
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <string.h>
#include "cmocka.h"

static int create_queue(void** state) {
	*state = test_malloc(sizeof(queue));
	init_queue(*state);

	return 0;
}

static int teardown(void **state) {
    test_free(*state);

    return 0;
}


static void test_basic(void **state) {
	int elements[] = { 4, 1, 5, 3, 10 };

	for (int i = 0; i < sizeof(elements) / sizeof(elements[0]); i++)
		queue_add(*state, elements[i]);

	for (int i = 0; i < sizeof(elements) / sizeof(elements[0]); i++)
		assert_int_equal(queue_remove(*state), elements[i]);

}

static void test_remove_empty(void **state) {
	assert_int_equal(queue_remove(*state), -1);

	int add = 5;
	queue_add(*state, add);
	assert_int_equal(queue_remove(*state), 5);

	assert_int_equal(queue_remove(*state), -1);
}

static void test_full_length (void **state) {
	for (int i = 0; i < 300; i++) {
		queue_add(*state, 2 * i);
		queue_add(*state, 2 * i + 1);
		assert_int_equal(queue_remove(*state),  2 * i);
		assert_int_equal(queue_remove(*state), 2 * i + 1);
	}
}

int main(void) {
	printf("-- Testing Queue Data Structure --\n");
    const struct CMUnitTest tests[] = {
        cmocka_unit_test_setup_teardown(test_basic,
                                        create_queue, teardown),
        cmocka_unit_test_setup_teardown(test_remove_empty,
                                        create_queue, teardown),
		cmocka_unit_test_setup_teardown(test_full_length,
										create_queue, teardown),
    };
    return cmocka_run_group_tests(tests, NULL, NULL);
}

