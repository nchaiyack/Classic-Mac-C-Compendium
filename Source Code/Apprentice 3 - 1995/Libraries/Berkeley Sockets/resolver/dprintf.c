#include <stdio.h>

dprintf(fmt,a0,a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11,a12,a13,a14,a15)
	char *fmt;
	char *a0,*a1,*a2,*a3,*a4,*a5,*a6,*a7,*a8,*a9,*a10,*a11,*a12,*a13,*a14,*a15;
{
	(void) fprintf(stderr,fmt,a0,a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11,a12,a13,a14,a15);
}

