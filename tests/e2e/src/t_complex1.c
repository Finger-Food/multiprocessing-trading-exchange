#include "pe_trader.h"
#include "parser.h"

void clean_up(pipe_s* pipe, int epoll_fd) {
	close(pipe->write_fd);
	close(pipe->read_fd);
	close(epoll_fd);
}

volatile sig_atomic_t signals_received;

int next_order_id;

void sigusr_handler(int sig, siginfo_t* info, void* ucontext) {
	signals_received = 1;
}

void make_order(order_s* order, enum command_type type, char* product, int qty, int price) {
	order->type = type;
	strcpy(order->product, product);
	order->qty = qty;
	order->price = price;
}

void make_message(char* buffer, order_s* order) {
	// signal blocker
	sigset_t block;
	sigemptyset(&block);
	sigaddset(&block, SIGUSR1);
	sigprocmask(SIG_BLOCK, &block, NULL);

	if (order == NULL)
		return;
	
	if (order->type == CANCEL)
		sprintf(buffer, "CANCEL %d", order->id);

	else if (order->type == AMEND)
		sprintf(buffer, "AMEND %d %d %d",
			order->id, order->qty, order->price);

	else {
		char* action;
		if (order->type == BUY)
			action = "BUY";
		
		else if (order->type == SELL)
			action = "SELL";

		sprintf(buffer, "%s %d %s %d %d", action,
			next_order_id++, order->product, order->qty, order->price);
	}
}

int main(int argc, char ** argv) {
	int trader_id;
    if (argc < 2) {
        printf("Trader error: Not enough arguments\n");
        return 1;
    }

	if (argc > 2) {
		printf("Trader error: Too many arguments\n");
		return 1;
	}

	if ((trader_id = my_atoi(argv[1])) < 0) {
		printf("Trader error: Invalid ID\n");
		return 1;
	}

	// initialise trader variables
	pid_t ppid = getppid();
	signals_received = 0;
	next_order_id = 0;

	// register signal handler
	struct sigaction sig;
	memset(&sig, 0, sizeof(struct sigaction));
	sig.sa_sigaction = sigusr_handler;
	sig.sa_flags = SA_SIGINFO | SA_RESTART;
	if (sigaction(SIGUSR1, &sig, NULL) == -1) {
		perror("Autotrader aborting, sigaction failed");
		return 1;
	}

	// signal blocker
	sigset_t block;
	sigemptyset(&block);
	sigaddset(&block, SIGUSR1);
	sigprocmask(SIG_BLOCK, &block, NULL);

	// open epoll
	int epoll_fd = epoll_create(1);
	if (epoll_fd < 0) {
		perror("Autotrader aborting, failed to setup epoll");
		return 1;
	}


    // connect to named pipes
	char exchange_fifo[BUFLEN], trader_fifo[BUFLEN];
	sprintf(exchange_fifo, FIFO_EXCHANGE, trader_id);
	sprintf(trader_fifo, FIFO_TRADER, trader_id);

	pipe_s pipe;
	if (init_pipe(&pipe, trader_fifo, exchange_fifo) == -1) {
		perror("Autotrader aborting, failed to open pipes");
		return 1;
	}

	// add read pipe to epoll
	struct epoll_event fifo_read;
	fifo_read.events = EPOLLIN;
	fifo_read.data.fd = pipe.read_fd;

	if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, pipe.read_fd, &fifo_read)) {
		perror("Autotrader aborting, failed to setup epoll");
		clean_up(&pipe, epoll_fd);
		return 1;
	}

	// poll first message
	char buffer[BUFLEN + 1] = { 0 };
	struct epoll_event events[1];
	if (epoll_wait(epoll_fd, events, 1, -1) != 1) {
		perror("Autotrader aborting, failed to setup epoll");
		clean_up(&pipe, epoll_fd);
		return 1;
	}

	// assert first message is MARKET OPEN
	get_message(&pipe, buffer);
	if (strcmp("MARKET OPEN", buffer)) {
		fprintf(stderr, "Autotrader aborting: MARKET OPEN not received\n");
		clean_up(&pipe, epoll_fd);
		return 1;
	}

	order_s order;

	// Send orders
	sleep(1);
	make_order(&order, SELL, "GPU", 99, 511);
	make_message(buffer, &order);
	send_message(ppid, &pipe, buffer);

	sleep(1);
	make_order(&order, SELL, "GPU", 99, 402);
	make_message(buffer, &order);
	send_message(ppid, &pipe, buffer);

	sleep(8);
		

	clean_up(&pipe, epoll_fd);
	return 0;
}
