/*
CopyQuickDrawGlobals.c
This is a workaround for a silly limitation of THINK C. THINK C allows you to
implicitly initialize QuickDraw by calling a console function, e.g. printf(),
rather than explicitly initialize QuickDraw by calling InitGraf etc. In the
first case THINK C then supports full input and output through the console and
also handles events, e.g. Command-. will cause an abort. (If you explicitly
initialize QuickDraw you can only use the console for output, not input.)
However, when THINK C does its own initialization it uses a private QuickDraw
"globals" area instead of the one at qd. This happened because Symantec didn't
want to force users of the console in the ANSI library to have to load the
MacTraps library (where qd is located). The consequence of this is that if your
program lets the console implicitly initialize QuickDraw then all the QuickDraw
globals, e.g. the patterns white and black, are all zero.

This program is a quick fix. It copies the real QuickDraw globals, referenced
off of register A5 to the public qd. It is totally safe. You can call it at any
time and as many times as you want. Of course there is little point in copying
the QuickDraw globals before they have been initialized, typically by your first
call to printf.

If MATLAB is true then "qd" will have been redefined as "mex_qd".

HISTORY:
1/17/92	dgp	Wrote it based on a suggestion of Mike Kahl.
1/22/92 dgp	Eliminated asm.
3/10/92 dgp	Eliminated erroneous assumption that sizeof(qd) was a multiple of four.
			Only copy the standard public quickdraw globals.
1/25/93 dgp removed obsolete support for THINK C 4.
1/25/93	dgp Replaced SysEqu.h by LoMem.h and changed program accordingly.
2/1/93	dgp	Corrected the capitalization of QuickDraw throughout this file.
*/
#include "VideoToolbox.h"
#if THINK_C
	#include <LoMem.h>
#else
	Ptr CurrentA5 : 0x904;
#endif

void CopyQuickDrawGlobals(void)
{
	register short *s,*d;
	
	s=*(short **)CurrentA5;
    d=(short *)&qd.thePort;
    s+=sizeof(qd.thePort)/sizeof(short);
    d+=sizeof(qd.thePort)/sizeof(short);
    while(d!=(short *)&qd.randSeed) *--d = *--s;
}