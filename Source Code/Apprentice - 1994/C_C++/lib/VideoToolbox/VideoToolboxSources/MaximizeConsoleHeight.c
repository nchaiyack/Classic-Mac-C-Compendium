/*
MaximizeConsoleHeight.c
THINK C provides a built-in console. That console is less useful than it might
be because it opens to only a small size, presumably designed to fit on the tiny
screen of a Mac Plus. This routine requests that the console open to the full
height of your main screen. Call MaximizeConsoleHeight() BEFORE opening the
console in THINK C, i.e. before your first printf().

To change the WIDTH of your THINK C console do this:
#include <console.h>
...
console_options.ncols=100;

HISTORY:
1/92	dgp wrote it.
8/27/92	dgp	check for 8-bit quickdraw before using GDevices.
10/10/92 dgp Reduced maximum console height by one pixel, so as not to clip 
			displayed text.
2/22/93	dgp replaced GetMBarHeight() by MBarHeight.
4/16/93	dgp	enhanced to actively support 1-bit QD, rather than doing nothing.
*/

#include "VideoToolbox.h"
#if THINK_C
	#include <LoMem.h>	// MBarHeight
	#include <console.h>
#endif

void MaximizeConsoleHeight(void)
{
#if THINK_C
	long qD;
	Rect r;
	
	Gestalt(gestaltQuickdrawVersion,&qD);
	console_options.top=MBarHeight+19;
	console_options.left=1;
	if(qD>=gestalt8BitQD) r=(**(**MainDevice).gdPMap).bounds;
	else r=CrsrPin;	// Can't use qd.screenBits.bounds 'cause qd may not yet be inited.
	console_options.nrows=r.bottom-4-console_options.top;
	console_options.nrows/=console_options.txSize*4/3-1; /* estimate line spacing */
#endif
}