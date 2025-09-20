#pragma once

#include <stdlib.h> //calloc
#include <stddef.h> //NULL

#define ALLOC(type, number) ((type *)calloc(sizeof(type), number))
#define CALLOC(type, number) ((type *)calloc(sizeof(type), number))

#define REALLOC(type, ptr, number) ((type *)realloc(ptr, (number) * sizeof(type)))

#define FREE(p)   \
	{             \
		free(p);  \
		p = NULL; \
	}

#define TRUE 1
#define FALSE 0

#ifndef MAX
#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#endif

#ifndef MIN
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#endif

#ifndef offsetof
#define offsetof(TYPE, MEMBER) ((size_t) & ((TYPE *)0)->MEMBER)
#endif

#ifndef container_of
#define container_of(ptr, type, member) ({			 \
	const typeof( ((type *)0)->member ) *__mptr = (ptr); \
	(type *)( (char *)__mptr - offsetof(type,member) ); })
#endif

#ifndef count_of
#define count_of(x) (sizeof(x) / sizeof(0 [x]))
#endif

#ifndef likely
#define likely(x) __builtin_expect((x), 1)
#endif /* likely */

#ifndef unlikely
#define unlikely(x) __builtin_expect((x), 0)
#endif /* unlikely */

#ifndef __unused
#define __unused __attribute__((__unused__))
#endif
