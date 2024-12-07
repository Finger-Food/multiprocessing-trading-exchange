#include "pe_trader.h"
#include "parser.h"

void clean_up(pipe_s* pipe, int epoll_fd) {
	close(pipe->write_fd);
	close(pipe->read_fd);
	close(epoll_fd);
}

volatile sig_atomic_t signals_received;

void sigusr_handler(int sig, siginfo_t* info, void* ucontext) {
	signals_received = 1;
}

int main(int argc, char ** argv) {
	// validate command line arguments
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
	int next_order_id = 0;

	// register signal handler
	struct sigaction sig;
	memset(&sig, 0, sizeof(struct sigaction));
	sig.sa_sigaction = sigusr_handler;
	sig.sa_flags = SA_SIGINFO | SA_RESTART;
	if (sigaction(SIGUSR1, &sig, NULL) == -1) {
		perror("Autotrader aborting, sigaction failed");
		return 1;
	}

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


	int to_accept = 0;	// to keep track of orders made, that have yet to be accepted
	order_s order;
	while (1) {
		if (pipe.curr_len == 0)
			// epoll won't fail after the first success (unless a signal is received)
			// in which case the following code will account for an empty pipe
			epoll_wait(epoll_fd, events, 1, 1000);

		if (get_message(&pipe, buffer) == -1) {
			if (to_accept)
				kill(ppid, SIGUSR1);
			continue;
		}

/*		if (!signals_received)
			fprintf(stderr, "Signal not received for messge: %s\n", buffer);
		else
			signals_received = 0;
*/
		// invalid messages are ignored in this implementation
		if (parse_exchange_message(buffer, &order) == -1) {
			//fprintf(stderr, "Invalid message received: %s\n", buffer);
			continue;
		}

		// to account for signal not being received by exchange when making order
		if (to_accept) {
			if (order.type != ACCEPTED)
				kill(ppid, SIGUSR1);
			else
				to_accept--;
		}
			
		if (order.type != SELL || order.qty == 0)	// to account for amend and cancels
			continue;

		if (order.qty >= 1000)	// end condition
			break;
		
		// make and send order (including signal)
		sprintf(buffer, "BUY %d %s %d %d", next_order_id++, 
					order.product, order.qty, order.price);
		if (send_message(ppid, &pipe, buffer)) {
			perror("Autotrader aborting, failed to send message to exchange");
			break;
		}
	
		to_accept++;
	}

	clean_up(&pipe, epoll_fd);
	return 0;
}
