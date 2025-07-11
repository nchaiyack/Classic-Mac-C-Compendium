/*
 * butil.c
 *
 * This is a quick module i whipped up to provide the unix binary (b*) routines.
 * They work largely on brute force principles, and aren't optomized at all.  Any
 * suggestions for better code will be greatly appreciated.
 *
 * Mike Trent 8/94
 */

#include "butil.h"

/* bcopy
 * - Copy binary data from b2 to b1
 */
void bcopy (char *b1, char *b2, long length)
{
	long x;
	for (x =0; x<length; x++) {
		b2[x] = b1[x];
	}
}

int bcmp (char *b1, char *b2, long length)
{
	long x;
	for (x=0; x<length; x++) {
		if (b1[x] != b2[x]) return -1;
	}
	return 0;
}

void bzero (char *data, long size)
{
	long x;
	char zero = 0;

	for (x = 0; x<size; x++) {
		data[x]= zero;
	}
}

long ffs (long i)
{
	long x=0;
	
	if (i == 0) return 0;
	
	while (1) {
		if (i & (1 << x)) return (x+1);
		x++;
	}
}	