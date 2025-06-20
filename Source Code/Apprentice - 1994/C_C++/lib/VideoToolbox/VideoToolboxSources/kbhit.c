/*
kbhit.c
kbhit() returns 1 if a keypress awaits processing, 0 otherwise.

6/13/90	dgp wrote kbhit, based on suggestion from Michael Kahl and an earlier version by
		Evan Relkin.
2/16/93	dgp	added YesOrNo().
5/24/93 dgp Moved YesOrNo() to MultipleChoice.c
12/25/93 dgp cosmetic editing
*/
#ifndef THINK_C
	#error "Sorry, kbhit.c requires THINK C."
#endif
#include "VideoToolbox.h"
#include <console.h>

int kbhit(void)
{
#if 1
	// This works fine.
	int c;

	c=getcharUnbuffered();
	if(c==EOF)return 0;
	ungetc(c,stdin);
	return 1;
#else
	// This doesn't work. It always returns false. I don't understand why.
	EventRecord event;
	
	return EventAvail(keyDown,&event);
#endif
}

int getcharUnbuffered(void)
{
	int c;

	csetmode(C_RAW,stdin);		/* unbuffered: no echo, no editing */
	c=getchar();
	csetmode(C_ECHO,stdin);		/* default mode: line-buffered, echo, full editing */
	return c;
}

