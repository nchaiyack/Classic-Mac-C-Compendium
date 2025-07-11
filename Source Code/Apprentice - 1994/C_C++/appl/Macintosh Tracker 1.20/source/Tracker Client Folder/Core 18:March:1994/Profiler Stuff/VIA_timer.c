#if __option(profile)

/*
 *  VIA_timer.c
 *
 *  Copyright (c) 1991 Symantec Corporation.  All rights reserved.
 *
 */

#include "stdlib.h"
#include "Profiler.h"

#pragma options(!profile)

#define vT1C		(512*4)
#define vT1CH		(512*5)
#define vIER		(512*14)

struct words { short hi, lo; };
struct bytes { char hi, lo; };

/*  tick count  */
static union {
	long			dword;
	struct words	word;
} elapsed;

/*  VIA countdown timer #1  */
static union {
	short			word;
	struct bytes	byte;
} timer;

static struct {
	ProcPtr			vector;
	char			enable;
} save;

static Boolean timer_running, exithook_installed;


/*
 *  VIA_ticks - return tick count
 *
 *  Each tick represents approximately 1.28 micro-seconds.
 *
 */

long
VIA_ticks(void)
{
	if (!timer_running)
		start_VIA_timer();
	do {
		timer.byte.hi = VIA[vT1CH];
		timer.byte.lo = VIA[vT1C];
	} while (timer.byte.hi != VIA[vT1CH]);
	elapsed.word.lo = ~timer.word;
	return(elapsed.dword);
}


/*
 *  start_VIA_timer
 *
 *  It is not necessary to call this routine, as it is automatically
 *  invoked by "VIA_ticks".
 *
 */

void
start_VIA_timer(void)
{	
		/*  install rollover interrupt  */

	VIA[vT1C] = VIA[vT1CH] = ~0;
	save.vector = Lvl1DT[6];
	save.enable = VIA[vIER];
	asm {
		lea		@A5,a0
		move.l	a5,(a0)
		lea		@rollover,a0
		move.l	a0,Lvl1DT[6]
	}
	VIA[vIER] = 0xC0;
	
		/*  arrange for timer to be stopped at exit  */
		
	timer_running = true;
	if (!exithook_installed) {
		_atexit(stop_VIA_timer);
		exithook_installed = true;
	}
	return;
		
		/*  interrupt handler  */

rollover:
	asm {
		move.l	a5,-(sp)
		movea.l	@A5,a5
	}
	++elapsed.word.hi;
	VIA[vT1C] = VIA[vT1CH] = ~0;
	asm {
		movea.l	(sp)+,a5
		rts
A5:		dc.l	0				;  store app's A5 here
	}
}


/*
 *  stop_VIA_timer
 *
 *  It is not necessary to call this routine, as it will be called
 *  automatically at program exit.
 *
 */

void
stop_VIA_timer(void)
{
	if (timer_running) {
		if (!(save.enable & 0x40))
			VIA[vIER] = 0x40;
		Lvl1DT[6] = save.vector;
		timer_running = false;
	}
}

#endif
