/*

rgnull.c by Aaron Contorer for NCSA
Copyright 1987, board of trustees, University of Illinois

routines for "null" device -- calling these routines
	has no effect, but they are compatible with all RG calls.

*/

#ifdef MPW
#pragma segment TEKNULL
#endif

#include "rg0.proto.h"

void	TEKNULLunload(void){}

short	RG0newwin(void)
{
	return(0);
}

char *RG0devname(void)
{
	return("Null device -- do not display output");
}

void	RG0oneshort(short x)
{
	#pragma unused (x)
}

void	RG0void(void)
{}

short	RG0returnshort(short x)
{
	#pragma unused (x)
	return 0;
}

short	RG0pencolor(short w, short color)
{
	#pragma unused (w, color)
	return 0;
}

short	RG0point(short w, short x, short y)
{
	#pragma unused (w, x, y)
	return 0;
}

short	RG0drawline(short w, short a, short b, short c, short d)
{
	#pragma unused (w, a, b, c, d)
	return 0;
}

void	RG0dataline(short w, short data, short count)
{
	#pragma unused (w, data, count)
}

void	RG0charmode(short w, short rotation, short size)
{
	#pragma unused (w, rotation, size)
}

void	RG0info(short w, short a, short b, short c, short d, short v)
{
	#pragma unused (w, a, b, c, d, v)
}
