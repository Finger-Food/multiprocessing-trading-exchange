#ifndef PE_COMMON_H
#define PE_COMMON_H

#define _POSIX_SOURCE 199309L
#define _DEFAULT_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <errno.h>

#include "malloc.h"
#include "hash_map.h"
#include "llist.h"
#include "int_queue.h"

#define FIFO_EXCHANGE "/tmp/pe_exchange_%d"
#define FIFO_TRADER "/tmp/pe_trader_%d"
#define FEE_PERCENTAGE 1

#define BUFLEN (128)
#define PRODUCT_LEN 16
#define TERM (';')

#define PEX "[PEX] "
#define PEX_IND "[PEX]\t"
#define PEX_ORDER "[PEX]\t\t%s %d @ $%d (%d order)\n"
#define PEX_LEVEL "[PEX]\t\t%s %d @ $%d (%d orders)\n"
#define PEX_MATCH "[PEX] Match: Order %d [T%d], New Order %d [T%d], value: $%ld, fee: $%ld.\n"

#define VOID(integer) ((void*)((long)integer))
#define INT(void_pointer) ((int)((long)void_pointer))

#define ORDER(order_node) ((order_s*)(order_node->value))
#define TYPE(order_node) (ORDER(order_node)->type)
#define ID(order_node) (ORDER(order_node)->id)
#define QTY(order_node) (ORDER(order_node)->qty)
#define PRICE(order_node) (ORDER(order_node)->price)

/* Structure definitions */
// pipe data structure
typedef struct {
	int write_fd;
	int read_fd;
	char buffer[BUFLEN];
	int curr_len;
} pipe_s;

// order data structure
enum command_type { BUY, SELL, AMEND, CANCEL, FILL, ACCEPTED, INVALID };
typedef struct {
	int trader_id;
	enum command_type type;
	int id;
	int qty;
	int price;
	char product[PRODUCT_LEN + 1];
} order_s;

// trader data structure
typedef struct {
	pipe_s* pipe;
	hash_map* orders; // key: (long) orderID, value: (node*) order
	int next_order;
	int id;
	int pid;
} trader_s;

// position data structure
typedef struct {
	long amount;
	long balance;
} pos_s;

// product book data structure
typedef struct {
	llist* buy_orders;
	llist* sell_orders;
	pos_s* positions;
} prod_book_s;

// exchange data structure
typedef struct {
	int num_prod;	// number of products
	char (*products)[PRODUCT_LEN + 1];	// array of product strings

	int num_trad;	// number of traders
	trader_s* traders;
	hash_map* pid_to_trader; // key: (int) process id, value: (trader_s*) trader

	hash_map* prod_map; // key: (char*) product, value: (prod_book_s*) product orders
	long income;
} exchange_s;

static char stringf_buffer[BUFLEN] __attribute__((unused)) = { 0 };
#define stringf(fmt, ...) (sprintf(stringf_buffer, fmt, ##__VA_ARGS__), stringf_buffer)
// neat self-made function that 'kind of' returns a string when given a format and arguments

#endif
