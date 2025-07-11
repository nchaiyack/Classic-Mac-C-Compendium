/* ========== the commmand file: ==========

	ps.c
	
	Copyright (c) 1993,1994 Newport Software Development
	
	You may distribute unmodified copies of this file for
	noncommercial purposes.  You may use this file as a
	reference when writing your own nShell(tm) commands.
	
	All other rights are reserved.
	
   ========== the commmand file: ========== */

#ifdef __MWERKS__			// CodeWarrior requires an A4 setup
#include <A4Stuff.h>
#endif

#include <GestaltEqu.h>
#include <Processes.h>

#include "nshc.h"
					
#include "nshc_utl.proto.h"

int	Pre7( void );

int	Pre7( void )
{
	OSErr	error;
	long	response;
	
	if ( error = Gestalt( 'sysv', &response ) )
		return(true);
		
	if ( response < 0x700 )
		return(true);
	else
		return(false);
}

					
void main(t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls)
{
	char					got_one;
	Str255					name;
	ProcessSerialNumber		psn_p;
    ProcessInfoRec  		pi;
	OSErr					error;
	long					response;
	
#ifdef __MWERKS__
	long oldA4  = SetCurrentA4();
#endif
	
	if (nshc_bad_version( nshc_parms, nshc_calls, NSHC_VERSION )) goto Exit;
	
	if ( Pre7() ) {
		nshc_calls->NSH_putStr_err("\pps: This command requires System 7.\r");
		nshc_parms->result = NSHC_ERR_GENERAL;
		nshc_parms->action = nsh_idle;
		goto Exit;
		}
	    
    pi.processName       = name;
    pi.processInfoLength = sizeof(pi);
    pi.processAppSpec    = NULL;
    
	psn_p.highLongOfPSN = 0;
	psn_p.lowLongOfPSN  = kNoProcess;
	
	got_one = 0;
    
	while(!GetNextProcess(&psn_p))
		if(!GetProcessInformation(&psn_p,&pi)) {
			if (!got_one) {
				nshc_calls->NSH_puts("\r Process Crea Type  Mem Size   Mem Free  Name\r");
				  nshc_calls->NSH_puts(" ------- ---- ---- ---------- ---------- ----\r");
				got_one = 1;
				}
			nshc_calls->NSH_printf( " %7ld", psn_p.lowLongOfPSN );
			nshc_calls->NSH_printf( " %.4s %.4s", &pi.processSignature, &pi.processType );
			nshc_calls->NSH_printf( " %10ld", pi.processSize );
			nshc_calls->NSH_printf( " %10ld ", pi.processFreeMem );
			name[++name[0]] = '\r';
			nshc_calls->NSH_putStr( name );
			}

	if (got_one) nshc_calls->NSH_putchar('\r');
	
	nshc_parms->result = !got_one;	//	success if we found any processes
	nshc_parms->action = nsh_idle;

Exit:

#ifdef __MWERKS__
	SetA4(oldA4);		// CodeWarrior needs to restore A4
#else
	;					// Think needs a ; to go with the Exit label
#endif
}
