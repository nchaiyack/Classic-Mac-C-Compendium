/* ========== the commmand file: ==========

	battery.c
	
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
#include <power.h>
					
#include "nshc.h"

#include "nshc_utl.proto.h"
					
void main(t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls)
{
	OSErr	error;
	char	recently;
	char	lovoltage;
	char	highrate;
	char	connected;
	long	response;
	Byte	status;
	Byte	power;

#ifdef __MWERKS__
	long oldA4  = SetCurrentA4();
#endif
	
	if (nshc_bad_version( nshc_parms, nshc_calls, NSHC_VERSION )) goto Cleanup;
	
	nshc_parms->result = NSHC_ERR_GENERAL;		// preset return status to error
	nshc_parms->action = nsh_idle;

	error = Gestalt( 'powr', &response );
	
	if (error) {
		nshc_calls->NSH_printf_err("battery: The Gestalt call returned an error of %d.\r", error);
		goto Cleanup; // error status is already set
		}
		
	if ( !(response & ( 1 << gestaltPMgrExists ))) {
		nshc_calls->NSH_putStr_err("\pbattery: The Power Manager is not installed.\r");
		goto Cleanup; // error status is already set
		}

	error = BatteryStatus( &status, &power );
	
	if (error) {
		nshc_calls->NSH_printf_err("battery: The BatteryStatus call returned an error of %d.\r", error);
		goto Cleanup; // error status is already set
		}

	recently  = ( status & connChangedMask );
	lovoltage = ( status & batteryLowMask );
	highrate  = ( status & hiChargeMask );
	connected = ( status & chargerConnMask );
	
	if (recently)
		nshc_calls->NSH_puts("The charger has recently been");
	else
		nshc_calls->NSH_puts("The charger is");
	
	if (connected)
		nshc_calls->NSH_puts(" connected.\r");
	else
		nshc_calls->NSH_puts(" disconnected.\r");
	
	if (lovoltage)
		nshc_calls->NSH_puts("The low voltage flag is set.\r");		

	if (highrate)
		nshc_calls->NSH_puts("The high charging rate flag is set.\r");
	
	nshc_calls->NSH_printf("Battery voltage is now %f V.\r", ((power/100.0)+5.12));

	nshc_parms->result = NSHC_NO_ERR;
	nshc_parms->action = nsh_idle;
	
Cleanup:

#ifdef __MWERKS__
	SetA4(oldA4);		// CodeWarrior needs to restore A4
#else
	;					// Think needs a null statement for the Cleanup label
#endif
}
