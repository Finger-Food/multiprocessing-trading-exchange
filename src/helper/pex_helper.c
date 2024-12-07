#include "pex_helper.h"
#include "pe_exchange.h"
#include "parser.h"
#include "product_book.h"

/* dictionary hash and comparators functions */
int string_hash(void* data) {
	char* c = (char*)data;
	int result = 0;
	while (*c != '\0') {
		result += *c;
		c++;
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
/* dictionary hash and comparator functions */


// if the file doesn't end with a newline it's invalid (by POSIX)
int parse_products(FILE* product_file, exchange_s* pex) {

	pex->num_prod = -1;
	char c;

	// validate first number.
	if (fscanf(product_file, "%d%c", &pex->num_prod, &c) < 2 || c != '\n')
		pex->num_prod = -1;

	if (pex->num_prod <= 0)
		return -1;

	// get and store all products
	pex->products = (char (*)[PRODUCT_LEN + 1])
			Malloc(sizeof(char) * pex->num_prod * (PRODUCT_LEN + 1));

	for (int p = 0; p < pex->num_prod; p++)
		if (fscanf(product_file, "%" STR(PRODUCT_LEN) "s%c",
					pex->products[p], &c) < 2)
			return -1;

		else if (c != '\n')
			return -1;

	return 0;
}

void message_trader(trader_s* trader, char* message) {
	if (trader->pipe == NULL)
		return;

	int ret = send_message(trader->pid, trader->pipe, message);

	if (ret == 1 || ret < 0) {	// if pipe empty, or unexpected write/kill fail
		delete_pipe(trader->id, trader->pipe);
		trader->pipe = NULL;
		queue_add(&discon_queue, trader->pid);
	}
}

void market_order(exchange_s* pex, int source, order_s order) {
	char* type = order.type == SELL ? "SELL" : "BUY";

	char* message = stringf("MARKET %s %s %d %d", type,
			order.product, order.qty, order.price);

	for (int id = 0; id < pex->num_trad; id++)
		if (id != source)
			message_trader(&pex->traders[id], message);
}


// Order matching and calculation
static long rounded_fee(long value) {
	double fee = value * FEE_PERCENTAGE / 100.0;
	if (fee - (long)fee >= 0.5)
		return (long)fee + 1;
	else
		return (long)fee;
}

void match_and_insert(exchange_s* pex, prod_book_s* book, node* new) {
	trader_s* traders = pex->traders;
	int sell = TYPE(new) == SELL;
	int tid = ORDER(new)->trader_id;

	node *match, *matched;
	int match_tid;
	int price, amount;
	long total, fee;

	char fill_message[BUFLEN];
	
	match = sell ? book->buy_orders->tail : book->sell_orders->head;
	while (match != NULL) {
		if ((sell && PRICE(new) > PRICE(match)) ||
			(!sell && PRICE(new) < PRICE(match)))
			break;

		match_tid = ORDER(match)->trader_id;

		price = PRICE(match);	
		amount = QTY(new) < QTY(match) ? QTY(new) : QTY(match);
		QTY(new) -= amount;
		QTY(match) -= amount;

		total = (long)price * (long)amount;
		fee = rounded_fee(total);

		if (sell) {
			book->positions[tid].amount -= amount;
			book->positions[tid].balance += (total - fee);

			book->positions[match_tid].amount += amount;
			book->positions[match_tid].balance -= total;
		}
		else {	// buy
			book->positions[tid].amount += amount;
			book->positions[tid].balance -= (total + fee); 

			book->positions[match_tid].amount -= amount;
			book->positions[match_tid].balance += total;
		}
		pex->income += fee;

		if (sell) {
			sprintf(fill_message, "FILL %d %d", ID(match), amount);
			message_trader(&pex->traders[match_tid], fill_message);
			sprintf(fill_message, "FILL %d %d", ID(new), amount);
			message_trader(&pex->traders[tid], fill_message);
		}
		else {
			sprintf(fill_message, "FILL %d %d", ID(new), amount);
			message_trader(&pex->traders[tid], fill_message);
			sprintf(fill_message, "FILL %d %d", ID(match), amount);
			message_trader(&pex->traders[match_tid], fill_message);
		}	
		printf(PEX_MATCH, ID(match), match_tid, ID(new), tid, total, fee);
		
		matched = match;
		match = sell ? match->prev : match->next;
		
		if (QTY(matched) == 0) {
			del(pex->traders[match_tid].orders, VOID(ID(matched)));
			orderbook_remove(book, matched);
			delete_order_node(matched);
		}

		if (QTY(new) == 0)
			break;
	}

	if (QTY(new) != 0)
		put(traders[tid].orders, VOID(ID(new)), new),
		orderbook_add(book, new);
	
	else
		delete_order_node(new);
}


void delete_pipe(int id, pipe_s* pipe) {
	close(pipe->write_fd);
	close(pipe->read_fd);
	unlink(stringf(FIFO_EXCHANGE, id));
	unlink(stringf(FIFO_TRADER, id));
	
	free(pipe);
}

// free trader and its pipe
void free_trader(trader_s* trader) {
	if (trader->orders == NULL)
		return;
	
	if (trader->pipe != NULL) {
		delete_pipe(trader->id, trader->pipe);
		trader->pipe = NULL;
	}

	free_map(trader->orders);
	free(trader->orders);
	trader->orders = NULL;
}

// free exchange
void free_exchange(exchange_s* pex) {
	if (pex == NULL)
		return;

	// free product to product book map and corresponding product books
	if (pex->prod_map != NULL) {

		// free product book
		prod_book_s* book;
		for (int p = 0; p < pex->num_prod; p++) {
			book = get(pex->prod_map, pex->products[p]);
			free_product_book(book);
			free(book);
		}

		// free product book map
		free_map(pex->prod_map),
		free(pex->prod_map);
		pex->prod_map = NULL;
	}

	// free trader and pipe data
	if (pex->traders != NULL) {

		// free trader if not already free
		for (int id = 0; id < pex->num_trad; id++)
			free_trader(&pex->traders[id]);
			
		// free trader array
		free(pex->traders);
		pex->traders = NULL;
	}

	// free process id to trader id map
	if (pex->pid_to_trader != NULL) {
		free_map(pex->pid_to_trader);
		free(pex->pid_to_trader);
		pex->pid_to_trader = NULL;
	}

	// free product string array
	if (pex->products != NULL) {
		free(pex->products);
		pex->products = NULL;
	}
}
