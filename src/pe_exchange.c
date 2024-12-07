/**
 * comp2017 - assignment 3
 * Kris Parekh
 * kpar9682
 */

#include "pe_exchange.h"
#include "pex_helper.h"
#include "parser.h"
#include "product_book.h"

static exchange_s pex; // global just for error function free
static queue signal_queue;
queue discon_queue;

// Error handling function
void pe_error(int err_num, const char* message) {
	free_exchange(&pex);

	char* error_type = NULL;
	switch(err_num) {
		case ARG_ERR:
			error_type = "Exchange could not be initialised";
			break;

		case PRODUCT_PARSE_ERR:
			error_type = "Product file could not be parsed";
			break;

		case SIGACT_ERR:
			error_type = "Sigaction could not be initialised";
			break;

		case MKFIFO_ERR:
			error_type = "Failed to create FIFO";
			break;

		case FORK_ERR:
			error_type = "Exchange failed to fork child process";
			break;

		case TRADER_EXEC_ERR:
			error_type = "Exchange failed to execute trader";
			break;

		case OPEN_PIPE_ERR:
			error_type = "Failed to connect to FIFO";
			break;

		default:
			fprintf(stderr, "This code shouldn't be reachable (pe_error)\n");
			exit(err_num);
	}

	fprintf(stderr, PEX);
	if (message == NULL)
		perror(error_type);
	else
		fprintf(stderr, "%s: %s\n", error_type, message);
	printf(PEX "Aborting Exchange\n");
	exit(err_num);
}

// Signal handlers
void sigusr_handler(int sig, siginfo_t* info, void* ucontext) {
	queue_add(&signal_queue, info->si_pid);
}

void sigchld_handler(int sig, siginfo_t* info, void* ucontext) {
	int status;
	int pid = wait(&status);
	if (WIFEXITED(status) && WEXITSTATUS(status) == TRADER_EXEC_ERR)
		queue_add(&discon_queue, -1);
	queue_add(&discon_queue, pid);
}

int main(int argc, char** argv) {
	/* Initialise exchange */
	memset(&pex, 0, sizeof(exchange_s));
	printf(PEX "Starting\n");
	
	/* Validate enough arguments have been given*/
	if (argc == 1)
		pe_error(ARG_ERR, "No product file given");
	
	if (argc <= 2)
		pe_error(ARG_ERR, "No traders given");
	
	pex.num_trad = argc - 2;


	/* Open and parse product file */
	FILE* product_file = fopen(argv[1], "r");
	if (product_file == NULL)
		pe_error(PRODUCT_PARSE_ERR, NULL);

	int ret = parse_products(product_file, &pex);
	fclose(product_file);
	if (ret == -1)
		pe_error(PRODUCT_PARSE_ERR, "Invalid format");

	// Output products to exchange
	printf(PEX "Trading %d products: ", pex.num_prod);
	for (int p = 0; p < pex.num_prod; p++)
		if (p == pex.num_prod - 1)
			printf("%s\n", pex.products[p]);
		else
			printf("%s ", pex.products[p]);

	pex.prod_map = Malloc(sizeof(hash_map));
	init_map(pex.prod_map, pex.num_prod, string_hash, string_comp);

	// initialise product books for each product in product map
	prod_book_s* book;
	for (int p = 0; p < pex.num_prod; p++) {
		book = Malloc(sizeof(prod_book_s));
		init_product_book(book, pex.num_trad);
		put(pex.prod_map, pex.products[p], book);
	}


	// register signal handlers
	struct sigaction sigusr;
	memset(&sigusr, 0, sizeof(struct sigaction));
	sigusr.sa_sigaction = sigusr_handler;
	sigusr.sa_flags = SA_SIGINFO | SA_RESTART;

	if (sigaction(SIGUSR1, &sigusr, NULL) == -1)
		pe_error(SIGACT_ERR, NULL);

	struct sigaction sigchld;
	memset(&sigchld, 0, sizeof(struct sigaction));
	sigchld.sa_sigaction = sigchld_handler;
	sigchld.sa_flags = SA_SIGINFO | SA_RESTART;

	if (sigaction(SIGCHLD, &sigchld, NULL) == -1)
		pe_error(SIGACT_ERR, NULL);

	// signal blocker
	sigset_t block, unblock;
	sigemptyset(&block);
	sigaddset(&block, SIGCHLD);
	sigaddset(&block, SIGUSR1);
	sigprocmask(SIG_BLOCK, &block, &unblock);

	// initialise queues
	init_queue(&signal_queue);
	init_queue(&discon_queue);
	/* Exchange initialised */

	/* Start processing traders */
	// initialise trader array and pid to trader hash map
	pex.pid_to_trader = Malloc(sizeof(hash_map));
	init_map(pex.pid_to_trader, pex.num_trad, int_hash, int_comp);
	pex.traders = Malloc(sizeof(trader_s) * pex.num_trad);
	memset(pex.traders, 0, sizeof(trader_s) * pex.num_trad);	

	int pid, tid;
	trader_s* trader;
	char exchange_fifo[BUFLEN], trader_fifo[BUFLEN];

	for (tid = 0; tid < pex.num_trad; tid++) {
		sprintf(exchange_fifo, FIFO_EXCHANGE, tid);
		sprintf(trader_fifo, FIFO_TRADER, tid);
		
		// make fifos
		if (mkfifo(exchange_fifo, FIFO_FLAG))
			pe_error(MKFIFO_ERR, NULL);
		printf(PEX "Created FIFO %s\n", exchange_fifo);

		if (mkfifo(trader_fifo, FIFO_FLAG)) {
			unlink(exchange_fifo);
			pe_error(MKFIFO_ERR, NULL);
		}
		printf(PEX "Created FIFO %s\n", trader_fifo);

		// fork and exec traders
		printf(PEX "Starting trader %d (%s)\n", tid, argv[tid + 2]);
		if ((pid = fork()) == 0) {
			sigprocmask(SIG_SETMASK, &unblock, NULL);
			for (int id = 0; id < tid; id++) {
				close(pex.traders[id].pipe->write_fd);
				close(pex.traders[id].pipe->read_fd);
			}
			if (execl(argv[tid + 2], argv[tid + 2], stringf("%d", tid), NULL))
				return TRADER_EXEC_ERR;	// exec failure that's processed later
		}

		// fork failure
		else if (pid == -1) {
			unlink(exchange_fifo);
			unlink(trader_fifo);
			pe_error(FORK_ERR, NULL);
		}

		// initialise trader fields
		trader = &pex.traders[tid];
		trader->id = tid;
		trader->pid = pid;
		trader->next_order = 0;
		trader->orders = Malloc(sizeof(hash_map));
		init_map(trader->orders, pex.num_prod, int_hash, int_comp);
		// add to pid -> trader hash map
		put(pex.pid_to_trader, VOID(pid), &pex.traders[tid]);

		// set up trader pipes
		trader->pipe = Malloc(sizeof(pipe_s));
		if (init_pipe(trader->pipe, exchange_fifo, trader_fifo)) {
			free(trader->pipe);
			trader->pipe = NULL;
			unlink(exchange_fifo);
			unlink(trader_fifo);
			pe_error(OPEN_PIPE_ERR, NULL);
		}

		// print pipe connection messages
		printf(PEX "Connected to %s\n", exchange_fifo);
		printf(PEX "Connected to %s\n", trader_fifo);
	}
	/* Finish trader processing */



	/* Exchange main loop */
	node* order;
	char message[BUFLEN + 1] = { 0 };
	order_s command;
	int invalid = 0;
	int existing_traders = pex.num_trad;

	char* response;
	sleep(1);	// very shockingly necessary -- drops my test cases to 1/4 without
	
	for (tid = 0; tid < pex.num_trad; tid++)
		message_trader(&pex.traders[tid], "MARKET OPEN");

	// main loop
	while (existing_traders > 0) {

		// Wait and block for signals if none received
		sigprocmask(SIG_BLOCK, &block, NULL);	//
		if (signal_queue.size == 0 && discon_queue.size == 0) {
			sigsuspend(&unblock);
		}
		sigprocmask(SIG_SETMASK, &unblock, NULL);	//
		
		// Deal with trader disconnections
		if (discon_queue.size > 0) {
			pid = queue_remove(&discon_queue);
			if (pid == -1) {	// trader could not be executed
				pid = queue_remove(&discon_queue);
				trader = get(pex.pid_to_trader, VOID(pid));
				tid = trader->id;
				sprintf(message, "Trader %d binary (%s) invalid",
						 tid, argv[tid + 2]); 
				pe_error(TRADER_EXEC_ERR, message);
			}
			
			// standard disconnection (trader return or pipe closure)
			trader = get(pex.pid_to_trader, VOID(pid));
			if (trader->orders != NULL) {
				free_trader(trader);
				printf(PEX "Trader %d disconnected\n", trader->id);
				existing_traders--;
			}

			continue;
		}

		if (signal_queue.size == 0)
			continue;
			
		// Get and process message from signal received
		/* Validate Signal */
		invalid = 0;
		pid = queue_remove(&signal_queue);
		trader = get(pex.pid_to_trader, VOID(pid));
		tid = trader->id;

		if (trader->orders == NULL) // disconnected trader
			continue;

		if (get_message(trader->pipe, message)) {	// no message, or non-terminated message
			message_trader(trader, "INVALID");
			continue;
		}


		/* Parse message */
		printf(PEX "[T%d] Parsing command: <%s>\n", tid, message);
		
		// correct format, price and qty in range, product <= 16 characters
		if (parse_trader_message(message, &command) == -1)
			invalid = 1;

		// check if order id is valid (next order), and product is valid
		else if (command.type == BUY || command.type == SELL) {
			if (command.id != trader->next_order ||
				(book = get(pex.prod_map, command.product)) == NULL)
				invalid = 1;
		}

		// amend and cancel, check if order id is valid (already in system)
		else if ((order = get(trader->orders, VOID(command.id))) == NULL)
			invalid = 1;

		if (invalid) {
			message_trader(trader, "INVALID");
			continue;
		}

		
		/* Process command */
		if (command.type == CANCEL || command.type == AMEND) {

			book = get(pex.prod_map, ORDER(order)->product);
			del(trader->orders, VOID(command.id));
			orderbook_remove(book, order);

			strcpy(command.product, ORDER(order)->product);

			if (command.type == CANCEL) {
				command.qty = 0;
				command.price = 0;
				command.type = TYPE(order);
				response = "CANCELLED";
				
				delete_order_node(order);
				order = NULL;
			}
			else {
				command.type = TYPE(order);
				response = "AMENDED";

				QTY(order) = command.qty;
				PRICE(order) = command.price;
			}
		}

		else {
			trader->next_order++;

			order = create_order_node(&command);
			ORDER(order)->trader_id = tid;
			response = "ACCEPTED";
		}
		/* Command processed */

		sprintf(message, "%s %d", response, command.id);
		message_trader(trader, message);
		market_order(&pex, tid, command);


		if (order != NULL)
			match_and_insert(&pex, book, order);

		printf(PEX_IND "--ORDERBOOK--\n");
		for (int p = 0; p < pex.num_prod; p++) {
			book = get(pex.prod_map, pex.products[p]);
			output_orderbook(pex.products[p], book);
		}
		
		printf(PEX_IND "--POSITIONS--\n");
		output_positions(&pex);
	}
	/* End main loop */

	printf(PEX "Trading completed\n");
	printf(PEX "Exchange fees collected: $%ld\n", pex.income);

	free_exchange(&pex);

	return 0;
}
