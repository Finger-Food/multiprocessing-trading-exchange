#ifndef PRODUCT_BOOK_H
#define PRODUCT_BOOK_H

#include "pe_common.h"

// creates and initialises node with order as value
extern node* create_order_node(order_s* command);

// frees dynamic memory used by node (including the node itself)
extern void delete_order_node(node* order_node);

// creates and initialises product book (orderbook and positions)
extern void init_product_book(prod_book_s* book, int num_trad);

// frees dynamic memory used by structure (not including structure)
extern void free_product_book(prod_book_s* book);

// adds order in correct position
extern void orderbook_add(prod_book_s* book, node* new);

// removes node (adjust list values), without deleting node
extern void orderbook_remove(prod_book_s* book, node* order);

// outputs orders in correct format
extern void output_orderbook(char* product, prod_book_s* book);

// outputs trader positions in correct format
extern void output_positions(exchange_s* exchange);

#endif
