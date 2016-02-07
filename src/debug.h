#include <stdio.h>

#ifndef PRINT_DEBUG
#define PRINT_DEBUG 0
#endif

#define DEBUG(fmt, ...)                                                        \
	do {                                                                   \
		if (PRINT_DEBUG) {                                             \
			fprintf(stderr, "%s:%d:%s(): " fmt, __FILE__,          \
				__LINE__, __func__, __VA_ARGS__);              \
		}                                                              \
	} while (0)
