#include "parser.h"

int init_pipe(pipe_s* pipe, char* write_pipe, char* read_pipe) {
	if ((pipe->write_fd = open(write_pipe, O_RDWR)) == -1)
		return -1;
	
	if ((pipe->read_fd = open(read_pipe, O_RDONLY | O_NONBLOCK)) == -1) {
		close(pipe->write_fd);
		return -1;
	}

	memset(pipe->buffer, 0, BUFLEN + 1);
	pipe->curr_len = 0;
	return 0;
}

static int read_message(pipe_s* pipe, char* buffer) {
	int bytes_stored, mess_len; // message length
	char* term_pos;	// terminator position
	char* p_buffer = pipe->buffer;

	int result = read(pipe->read_fd, &p_buffer[pipe->curr_len], BUFLEN - pipe->curr_len);

	if (result < 0)
		result = 0;	// errors are to just be classified as an empty pipe

	bytes_stored = pipe->curr_len + result;
	p_buffer[bytes_stored] = '\0';

	if ((term_pos = strchr(p_buffer, TERM)) == NULL) {
		pipe->curr_len = 0;

		if (bytes_stored == BUFLEN) {
			strcpy(buffer, p_buffer);
			return BUFLEN;
		}
		else
			return -1; // a non-terminated string
	}

	mess_len = term_pos - p_buffer + 1;
	*term_pos = '\0';
	strcpy(buffer, p_buffer);
	
	if (mess_len == bytes_stored)
		pipe->curr_len = 0;

	else {
		for (int i = 0; i < bytes_stored - mess_len; i++)
			p_buffer[i] = p_buffer[mess_len + i];
		pipe->curr_len = bytes_stored - mess_len;
	}
	return mess_len - 1;	// to account for removal of ;
}

int my_atoi(char* string) {
	int value;
	char c;
	if (sscanf(string, "%d%c", &value, &c) != 1)
		return -1;
	return value;
}

static int validate_num(char* string) {
	int value = my_atoi(string);
	if (value > 999999)
		return -1;
	return value;
}

int get_message(pipe_s* pipe, char* buffer) {
	int ret = read_message(pipe, buffer);
	if (ret < 0)
		return ret;	// No delimiter (-1), or read error (-2)

	if (ret == BUFLEN)
		while (1) {
			ret = read_message(pipe, buffer);
			if (ret == -1)
				return -1;	// No delimeter
			if (ret < BUFLEN)
				return -1;	// No valid message > 128 characters
		}

	return 0;
}

static int split_message(char** array, char* buffer) {
	int size = 0;
	char* token = strtok(buffer, " ");
	while (token != NULL) {
		array[size++] = token;
		token = strtok(NULL, " ");
	}
	
	return size;
}

int parse_trader_message(char* buffer, order_s* command) {
	char* array[BUFLEN + 1];
	int size = split_message(array, buffer);
	
	enum command_type type;
	int qty, price;

	if (size < 2)
		return -1;

	if (!strcmp(array[0], "BUY"))
		type = BUY;

	else if (!strcmp(array[0], "SELL"))
		type = SELL;

	else if (!strcmp(array[0], "AMEND")) {
		if (size != 4 ||
			(qty = validate_num(array[2])) < 1 ||
			(price = validate_num(array[3])) < 1)
			return -1;
		type = AMEND;
	}

	else if (!strcmp(array[0], "CANCEL")) {
		if (size != 2)
			return -1;
		type = CANCEL;
	}

	else
		return -1;

	if (type == BUY || type == SELL) {
		if (size != 5 ||
			strlen(array[2]) > PRODUCT_LEN ||
			(qty = validate_num(array[3])) < 1 ||
			(price = validate_num(array[4])) < 1)
			return -1;

		strcpy(command->product, array[2]);
	}

	if ((command->id = validate_num(array[1])) < 0)
		return -1;

	command->type = type;
	command->qty = qty;
	command->price = price;

	return 0;
}

int parse_exchange_message(char* buffer, order_s* command) {
	if (!strcmp(buffer, "INVALID")) {
		command->type = INVALID;
		return 0;
	}

	char* array[BUFLEN + 1];
	int size = split_message(array, buffer);

	if (size == 2) {
		if (!strcmp(array[0], "ACCEPTED"))
			command->type = ACCEPTED;
		else if (!strcmp(array[0], "AMMENDED"))
			command->type = AMEND;
		else if (!strcmp(array[0], "CANCELLED"))
			command->type = CANCEL;
		else
			return -1;

		if ((command->id = validate_num(array[1])) < 0)
			return -1;
		return 0;
	}

	if (size == 3) {
		if (strcmp(array[0], "FILL") ||
				(command->id = validate_num(array[1]) < 0) ||
				(command->qty = validate_num(array[2]) < 0))
			return -1;
		
		command->type = FILL;
		return 0;
	}


	if (size != 5 || strcmp(array[0], "MARKET"))
		return -1;

	if (!strcmp(array[1], "BUY"))
		command->type = BUY;
	else if (!strcmp(array[1], "SELL"))
		command->type = SELL;
	else
		return -1;


	if (strlen(array[2]) > PRODUCT_LEN ||
		(command->qty = validate_num(array[3])) < 0 ||
		(command->price = validate_num(array[4])) < 0)
		return -1;
	strcpy(command->product, array[2]);

	return 0;
}

int send_message(int pid, pipe_s* pipe, char* message) {
	if (pipe == NULL || message == NULL)
		return 2;
	
	int length = strlen(message) + 1;
	char buffer[BUFLEN];
	strcpy(buffer, message);
	buffer[length - 1] = TERM;

	if (write(pipe->write_fd, buffer, length) == -1) {
		if (errno == EPIPE)
			return 1;
		return -1;
	}

	if (kill(pid, SIGUSR1))
		return -2;
	return 0;
}
