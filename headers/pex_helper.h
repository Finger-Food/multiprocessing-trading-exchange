#ifndef PEX_HELPER_H
#define PEX_HELPER_H

#include "pe_common.h"

#define LITERAL(X) #X
#define STR(x) LITERAL(x)

extern void delete_pipe(int id, pipe_s* pipe);
// closes pipe files, unlinks fifos and frees pipe pointer


extern int parse_products(FILE* product_file, exchange_s* data);
// only accepts files ending in a newline character
// will return -1 on invalid file and 0 on success


extern void message_trader(trader_s* trader, char* message);
// calls send_message
// if read end is closed, deletes pipe and adds pid to discon_queue
// if read end is closed or some other error occurs in sending message
// or signal, then pipe for the trader is closed

/*
#define message_traderf(trader, fmt, ...) \
	message_trader(trader, stringf(fmt "%c", ##__VA_ARGS__, TERM))
// wrapper function for message_trader, allowing it to be used like fprintf whilst also adding terminator character
*/

extern void market_order(exchange_s* pex, int source, order_s order);
// transforms order into a MARKET message, and calls message_trader for each trader except the source
// returns 0 on success, -1 on write error for any pipe


extern void match_and_insert(exchange_s* pex, prod_book_s* book, node* new);
// matches the order, and sends fill statements
// calculates fee, adjusts positions and adds fee to exchange
// prints appropriate statements to stdout


extern void free_trader(trader_s* trader);
// frees trader and pipe (if not already freed)

extern void free_exchange(exchange_s* pex);
// frees all the exchange data structures initialised


// dictionary hash and comparators
// strings cast to void*
int string_hash(void* data);

int string_comp(void* a, void* b);

// ints cast to void*
int int_hash(void* data);

int int_comp(void* a, void* b);

#endif
