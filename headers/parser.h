#ifndef PARSER_H
#define PARSER_H

#include "pe_common.h"

extern int init_pipe(pipe_s* pipe, char* write_pipe, char* read_pipe);
// initialises pipe and opens pipe files (doesn't create fifo)
// returns -1 if pipes could not be opened

extern int my_atoi(char* string);
// returns -1 on error, only accepts strings of format "%d\0"

extern int get_message(pipe_s* pipe, char* buffer);
// 'buffer' to store array of strings, and 'array' for string pointers
// returns length of array on success,
// -1 if the message is not ';' delimited or if message size > BUFLEN
// -2 on read error

extern int parse_trader_message(char* buffer, order_s* command);
// parses message and stores information in 'command'
// returns 0 on success and -1 if message is invalid:
// product longer than 16 characters
// quantity or price < 1
// if order id < 0

extern int parse_exchange_message(char* buffer, order_s* command);
// parses message and stores information in 'command'
// returns 0 on success and -1 if message is invalid:
// product longer than 16 characters
// quantity or price < 1
// if order id < 0

extern int send_message(int pid, pipe_s* pipe, char* message);
// sends message to write pipe adding ';' delimiter and SIGUSR1 to process id
// returns 0 on success, 1 if read end is closed, 2 for invalid parameters
// returns -1 on write error -2 on kill error

#endif
