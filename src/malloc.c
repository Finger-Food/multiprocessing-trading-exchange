#include "malloc.h"

void* Malloc(size_t bytes) {
	void* ret = malloc(bytes);
	if (ret == NULL)
		perror("Malloc error"),
		exit(1);

	return ret;
}
