/* ========== the commmand file: ==========

	beep.c
	
	Copyright (c) 1993,1994 Newport Software Development
	
	You may distribute unmodified copies of this file for
	noncommercial purposes.  You may use this file as a
	reference when writing your own nShell(tm) commands.
	
	All other rights are reserved.
	
   ========== the commmand file: ========== */

#ifdef __MWERKS__			// CodeWarrior requires an A4 setup
#include <A4Stuff.h>
#endif

#include "nshc.h"
					
#include "nshc_utl.proto.h"
					
void main(t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls)
{
#ifdef __MWERKS__
	long oldA4  = SetCurrentA4();
#endif
	
	if ( !nshc_bad_version( nshc_parms, nshc_calls, NSHC_VERSION ) ) {

		SysBeep(1);
	
		nshc_parms->result = NSHC_NO_ERR;
		nshc_parms->action = nsh_idle;
		
		}
	
#ifdef __MWERKS__
	SetA4(oldA4);
#endif
}
