#ifndef DEFINITIONS_H
#define DEFINITIONS_H

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <time.h>

typedef uint8_t byte;
typedef uint16_t two_bytes;
typedef uint32_t four_bytes;
typedef uint64_t eight_bytes;
typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;
typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;

#define Log(s, i) printf("\tLogging message: \"[%s] with parameter [%i]\"\n", s, i);
#define Logerror(s, i) printf("\tLogging error message: \"error message [%s], returned from line %i\"\n", s, i);

static inline void verify(bool check, const char *message, int line_number)
{
	if (check)
		return;

	Logerror(message, line_number);
	exit(1);
}
static inline void talkative_verify(bool check, const char *message, int line_number)
{
	if (check)
	{
		Log("verify call success", check);
		return;
	}

	Logerror(message, line_number);
	exit(1);
}

#endif
