#include "product_book.h"

#include <stdio.h>
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <string.h>
#include "cmocka.h"

static int trader_count = 5;

static order_s buy_orders[] = {
	{ 0, BUY, 0, 10, 150, "GOODS" },
	{ 3, BUY, 0, 15, 150, "GOODS" },
	{ 2, BUY, 0, 50, 250, "GOODS" },
	{ 3, BUY, 1, 5, 75, "GOODS" },
};

static order_s buy_ordered[] = {
	{ 3, BUY, 1, 5, 75, "GOODS" },
	{ 3, BUY, 0, 15, 150, "GOODS" },
	{ 0, BUY, 0, 10, 150, "GOODS" },
	{ 2, BUY, 0, 50, 250, "GOODS" },
};

static order_s sell_orders[] = {
	{ 0, SELL, 0, 10, 150, "GOODS" },
	{ 3, SELL, 0, 15, 150, "GOODS" },
	{ 2, SELL, 0, 50, 250, "GOODS" },
	{ 3, SELL, 1, 5, 75, "GOODS" },
};

static order_s sell_ordered[] = {
	{ 3, SELL, 1, 5, 75, "GOODS" },
	{ 0, SELL, 0, 10, 150, "GOODS" },
	{ 3, SELL, 0, 15, 150, "GOODS" },
	{ 2, SELL, 0, 50, 250, "GOODS" },
};


void test_order_node_creation(void** state) {
	order_s order = { 0, BUY, 0, 10, 150, "GOODS" };

	node* order_node = create_order_node(&order);
	
	assert_non_null(order_node);
	assert_memory_equal(order_node->value, &order, sizeof(order_s));

	delete_order_node(order_node);
}


static int create_product_book(void** state) {
	*state = test_malloc(sizeof(prod_book_s));
	init_product_book(*state, trader_count);

	for (int i  = 0; i < sizeof(buy_orders) / sizeof(buy_orders[0]); i++) {
		node* order_node = create_order_node(&buy_orders[i]);
		orderbook_add(*state, order_node);
	}

	for (int i  = 0; i < sizeof(sell_orders) / sizeof(sell_orders[0]); i++) {
		node* order_node = create_order_node(&sell_orders[i]);
		orderbook_add(*state, order_node);
	}

	return 0;
}

static int teardown(void **state) {
	free_product_book(*state);
    test_free(*state);

    return 0;
}

static void test_fill_buy_book(void** state) {
	prod_book_s* book = *state;

	node* cursor = book->buy_orders->head;
	for (int i  = 0; i < sizeof(buy_orders) / sizeof(buy_orders[0]); i++) {
		assert_memory_equal(cursor->value, &buy_ordered[i], sizeof(order_s));
		cursor = cursor->next;
	}
	assert_null(cursor);

	cursor = book->buy_orders->tail;
	for (int i = sizeof(buy_orders) / sizeof(buy_orders[0]) - 1; i >= 0; i--) {
		assert_memory_equal(cursor->value, &buy_ordered[i], sizeof(order_s));
		cursor = cursor->prev;
	}
	assert_null(cursor);
	
}

static void test_fill_sell_book(void** state) {
	prod_book_s* book = *state;

	node* cursor = book->sell_orders->head;
	for (int i  = 0; i < sizeof(sell_orders) / sizeof(sell_orders[0]); i++) {
		assert_memory_equal(cursor->value, &sell_ordered[i], sizeof(order_s));
		cursor = cursor->next;
	}
	assert_null(cursor);

	cursor = book->sell_orders->tail;
	for (int i = sizeof(sell_orders) / sizeof(sell_orders[0]) - 1; i >= 0; i--) {
		assert_memory_equal(cursor->value, &sell_ordered[i], sizeof(order_s));
		cursor = cursor->prev;
	}
	assert_null(cursor);

}

static void test_remove_head(void** state) {
	prod_book_s* book = *state;

	// buy orders
	node* head = book->buy_orders->head;
	node* second = head->next;

	orderbook_remove(book, head);
	delete_order_node(head);

	assert_ptr_equal(book->buy_orders->head, second);
	assert_int_equal(book->buy_orders->size, sizeof(buy_orders) / sizeof(buy_orders[0]) - 1);
	assert_null(second->prev);

	// sell orders
	head = book->sell_orders->head;
	second = head->next;

	orderbook_remove(book, head);
	delete_order_node(head);

	assert_ptr_equal(book->sell_orders->head, second);
	assert_int_equal(book->sell_orders->size, sizeof(sell_orders) / sizeof(sell_orders[0]) - 1);
	assert_null(second->prev);
}

static void test_remove_tail(void** state) {
	prod_book_s* book = *state;

	// buy orders
	node* tail = book->buy_orders->tail;
	node* second = tail->prev;

	orderbook_remove(book, tail);
	delete_order_node(tail);

	assert_ptr_equal(book->buy_orders->tail, second);
	assert_int_equal(book->buy_orders->size, sizeof(buy_orders) / sizeof(buy_orders[0]) - 1);
	assert_null(second->next);

	// sell orders
	tail = book->sell_orders->tail;
	second = tail->prev;

	orderbook_remove(book, tail);
	delete_order_node(tail);

	assert_ptr_equal(book->sell_orders->tail, second);
	assert_int_equal(book->sell_orders->size, sizeof(sell_orders) / sizeof(sell_orders[0]) - 1);
	assert_null(second->next);
}

static void test_remove(void** state) {
	prod_book_s* book = *state;

	node* second = book->buy_orders->head->next;
	orderbook_remove(book, second);
	delete_order_node(second);

	node* cursor = book->buy_orders->head;
	for (int i  = 0; i < sizeof(buy_orders) / sizeof(buy_orders[0]); i++) {
		if (i == 1)
			continue;
		assert_memory_equal(cursor->value, &buy_ordered[i], sizeof(order_s));
		cursor = cursor->next;
	}
	assert_null(cursor);
}

/*
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
*/

int main(void) {
	printf("-- Testing Product Book --\n");
    const struct CMUnitTest tests[] = {
		cmocka_unit_test(test_order_node_creation),
		
		cmocka_unit_test_setup_teardown(test_fill_buy_book,
							create_product_book, teardown),
		cmocka_unit_test_setup_teardown(test_fill_sell_book,
							create_product_book, teardown),
		cmocka_unit_test_setup_teardown(test_remove_head,
							create_product_book, teardown),
		cmocka_unit_test_setup_teardown(test_remove_tail,
							create_product_book, teardown),
		cmocka_unit_test_setup_teardown(test_remove,
							create_product_book, teardown),
    };
    return cmocka_run_group_tests(tests, NULL, NULL);
}

