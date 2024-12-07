#ifndef PE_TRADER_H
#define PE_TRADER_H

#include "pe_common.h"
#include <sys/epoll.h>

void clean_up(pipe_s* pipe, int epoll_fd);
// cleans up the memory associated with the program
// which is only the files

#endif
