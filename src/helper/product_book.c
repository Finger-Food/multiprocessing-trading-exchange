#include "product_book.h"

node* create_order_node(order_s* command) {
	node* order_node = Malloc(sizeof(node));
	order_node->prev = NULL;
	order_node->next = NULL;

	order_node->value = Malloc(sizeof(order_s));
	memcpy(order_node->value, command, sizeof(order_s));

	return order_node;
}

void delete_order_node(node* order_node) {
	free(order_node->value);
	order_node->value = NULL;
	free(order_node);
}


void init_product_book(prod_book_s* book, int num_trad) {
	// initialise positions
	book->positions = Malloc(sizeof(pos_s) * num_trad);
	memset(book->positions, 0, sizeof(pos_s) * num_trad);

	// initialise new price_time struct, and update dictionary
	book->buy_orders = Malloc(sizeof(llist));
	llist_init(book->buy_orders);

	book->sell_orders = Malloc(sizeof(llist));
	llist_init(book->sell_orders);
}

static void free_orders(llist* list) {
	node* next;
	node* cursor = list->head;
	while (cursor != NULL) {
		next = cursor->next;
		delete_order_node(cursor);
		cursor = next;
	}
}

void free_product_book(prod_book_s* book) {
	if (book->positions == NULL)
		return;

	free(book->positions);
	book->positions = NULL;

	free_orders(book->buy_orders);
	free(book->buy_orders);
	book->buy_orders = NULL;

	free_orders(book->sell_orders);
	free(book->sell_orders);
	book->sell_orders = NULL;
}


void orderbook_add(prod_book_s* book, node* new) {
	llist* order_list;
	node* cursor;

	// find  higher price-time priority, then insert before
	if (TYPE(new) == BUY) {
		order_list = book->buy_orders;
		cursor = order_list->head;

		while (cursor != NULL) {
			if (PRICE(cursor) >= PRICE(new)) {
				llist_insert_before(order_list, cursor, new);
				break;
			}
			cursor = cursor->next;
		}
		if (cursor == NULL)
			llist_append(order_list, new);
	}

	else { // SELL order
		order_list = book->sell_orders;
		cursor = order_list->tail;

		while (cursor != NULL) {
			if (PRICE(cursor) <= PRICE(new)) {
				llist_insert_after(order_list, cursor, new);
				break;
			}
			cursor = cursor->prev;
		}
		if (cursor == NULL)
			llist_insert_first(order_list, new);
	}
}

void orderbook_remove(prod_book_s* book, node* order) {
	if (book == NULL)
		return;

	if (TYPE(order) == SELL)
		llist_remove(book->sell_orders, order);

	else
		llist_remove(book->buy_orders, order);
}


static int get_levels(llist* order_list) {
	node* order = order_list->head;
	if (order == NULL)
		return 0;
	
	int price = PRICE(order);
	int levels = 1;
	order = order->next;

	while (order != NULL) {
		if (PRICE(order) != price) {
			price = PRICE(order);
			levels += 1;
		}
		order = order->next;
	}

	return levels;
}

static void output_level(char* type, int qty, int price, int orders) {
	if (orders == 1)
		printf(PEX_ORDER, type, qty, price, orders);
	else
		printf(PEX_LEVEL, type, qty, price, orders);
}

static void print_orders(llist* order_list, char* type) {
	node* order = order_list->tail;
	if (order == NULL)
		return;

	int qty = QTY(order);
	int price = PRICE(order);
	int same_level = 1;
	order = order->prev;

	while (order != NULL) {
		if (PRICE(order) == price) {
			same_level += 1;
			qty += QTY(order);
		}

		else {
			output_level(type, qty, price, same_level);

			same_level = 1;
			price = PRICE(order);
			qty = QTY(order);
		}
		order = order->prev;
	}
	
	output_level(type, qty, price, same_level);
}

void output_orderbook(char* product, prod_book_s* book) {
	int buy_levels = get_levels(book->buy_orders);
	int sell_levels = get_levels(book->sell_orders);

	printf(PEX_IND "Product: %s; Buy levels: %d; Sell levels: %d\n",
			product, buy_levels, sell_levels);

	print_orders(book->sell_orders, "SELL");
	print_orders(book->buy_orders, "BUY");
}


void output_positions(exchange_s* pex) {
	pos_s pos;
	char* product;

	for (int tid = 0; tid < pex->num_trad; tid++) {
		printf(PEX_IND "Trader %d:", tid);

		for (int p = 0; p < pex->num_prod; p++) {	
			product = pex->products[p];
			pos = ((prod_book_s*)get(pex->prod_map, product))->positions[tid];

			if (p != 0)
				putchar(',');
			printf(" %s %ld ($%ld)", product, pos.amount, pos.balance);
		}
		putchar('\n');
	}
}
