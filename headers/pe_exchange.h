#ifndef PE_EXCHANGE_C
#define PE_EXCHANGE_C

#include "pe_common.h"

#define FIFO_FLAG (0777)

extern queue discon_queue;

extern void pe_error(int err_num, const char* message);

enum errors {
	ARG_ERR,
	PRODUCT_PARSE_ERR,
	SIGACT_ERR,
	MKFIFO_ERR,
	FORK_ERR,
	TRADER_EXEC_ERR,
	OPEN_PIPE_ERR
};

#endif
