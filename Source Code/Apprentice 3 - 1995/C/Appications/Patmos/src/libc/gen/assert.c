#include <assert.h>
#include <stdio.h>

void __assert(const char *expression, const char *file, int lineno)
	{
  	fprintf(stderr, "%s:%u: failed assertion `%s'\n", file, lineno, expression);
	}
