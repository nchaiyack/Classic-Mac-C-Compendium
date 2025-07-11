/* ========== the commmand file: ==========

	delay.c 
	
	Copyright (c) 1993,1994 Newport Software Development
	
	You may distribute unmodified copies of this file for
	noncommercial purposes.  You may use this file as a
	reference when writing your own nShell(tm) commands.
	
	All other rights are reserved.
	
   ========== the commmand file: ========== */

#define TICKS_PER_SEC	60
#define SECS_PER_MINUTE	60

#ifdef __MWERKS__			// CodeWarrior requires an A4 setup
#include <A4Stuff.h>
#endif

#include "nshc.h"

#include "arg_utl.proto.h"
#include "nshc_utl.proto.h"

// prototypes - for local use only

void delay_start( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls );
void delay_continue( t_nshc_parms *nshc_parms );
void delay_stop( t_nshc_parms *nshc_parms );

// state machine

void delay_start( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls )
{
	int		tArg;		// position in arg list of delay time
	long	dTime;		// delay in seconds
	Handle	hTime;		// handle to hold target time
	
	if ( tArg = nshc_next_operand( nshc_parms, 0 ) )
		if ( nshc_is_numeric_operand( nshc_parms, tArg ) ) {

			dTime = arg_to_num( nshc_parms, tArg ) * TICKS_PER_SEC;
			
			if (nshc_got_option( nshc_parms, 'm' ))
				dTime *= SECS_PER_MINUTE;

			hTime = NewHandle(sizeof(long));
	
			if (hTime) {
				**(long **)hTime = TickCount() + dTime;
				nshc_parms->data = hTime;
				nshc_parms->action = nsh_continue;
				}
			else {
				nshc_calls->NSH_putStr_err("\pdelay: Could not allocate storage.\r");
				nshc_parms->action = nsh_idle;
				nshc_parms->result = NSHC_ERR_MEMORY;
				}
				
			return;
			
			}

	nshc_calls->NSH_putStr_err("\pUsage: delay time [-m].\r");
	nshc_parms->result = NSHC_ERR_PARMS;
	nshc_parms->action = nsh_idle;
}

void delay_continue( t_nshc_parms *nshc_parms )
{
	long time;
	
	time = TickCount();
	
	if (time > **(long **)nshc_parms->data)
		delay_stop(nshc_parms);		
}

void delay_stop( t_nshc_parms *nshc_parms )
{
	if (nshc_parms->data)
		DisposeHandle(nshc_parms->data);
		
	nshc_parms->result = NSHC_NO_ERR;
	nshc_parms->action = nsh_idle;
}

void main(t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls)
{
#ifdef __MWERKS__
	long oldA4  = SetCurrentA4();
#endif
	
	if ( !nshc_bad_version( nshc_parms, nshc_calls, NSHC_VERSION ) ) {
		
		switch (nshc_parms->action) {
			case nsh_start:
				delay_start(nshc_parms,nshc_calls);
				break;
			case nsh_continue:
				delay_continue(nshc_parms);
				break;
			default:
				delay_stop(nshc_parms);
				break;
			}
			
		}

#ifdef __MWERKS__
	SetA4(oldA4);
#endif
}
