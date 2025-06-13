/* ========== the commmand file: ==========

	gestalt.c
	
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

#include "nshc.h"
					
#include "nshc_utl.proto.h"

int is_OSType(t_nshc_parms *nshc_parms, int arg, OSType *selector);

int is_OSType(t_nshc_parms *nshc_parms, int arg, OSType *selector)
{
	char	c;
	char	*p;
	int		i;
	int		success;
	
	success = 1;
	*selector = 0;
	
	p = &nshc_parms->arg_buf[ nshc_parms->argv[ arg ] ];
	
	for (i = 0 ; i < 4 ; i++ )
		if ( c = *p++ ) {
			*selector = *selector << 8;
			*selector = *selector + c;
			}
		else
			success = 0;
			
	if (*p) success = 0;
	
	return(success);
}
	
void main(t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls)
{
	int		arg;
	int		return_status;
	OSErr	error;
	OSType	selector;
	long	response;
	char	num_option;
	
#ifdef __MWERKS__
	long oldA4  = SetCurrentA4();
#endif
	
	return_status = NSHC_NO_ERR;
	
	if (nshc_bad_version( nshc_parms, nshc_calls, NSHC_VERSION )) goto Exit;
	
	if ( nshc_parms->argc < 2 ) {
		nshc_calls->NSH_putStr_err("\pUsage: gestalt OSType [OSType...]\r");
		nshc_parms->action = nsh_idle;
		nshc_parms->result = NSHC_ERR_PARMS;
		goto Exit;
		}
	
	if ( nshc_got_option( nshc_parms, 'd' ) )
		num_option = 'd';
	else
		num_option = 'h';
	
	arg = 0;	// do the ones that work
	
	while ( arg = nshc_next_operand( nshc_parms, arg ) ) 
		if ( is_OSType( nshc_parms, arg, &selector ) )
		 {

			error = Gestalt( selector, &response );
	
			if (!error) {
				if ( num_option == 'h' )
					nshc_calls->NSH_printf(  "'%.4s' %8lX\r", &selector, response );
				else
					nshc_calls->NSH_printf(  "'%.4s' %10ld\r", &selector, response );
				}
				
			}

	arg = 0;	// report the ones that don't work
	
	while ( arg = nshc_next_operand( nshc_parms, arg ) ) 
		if ( is_OSType( nshc_parms, arg, &selector ) )
		 {

			error = Gestalt( selector, &response );
	
			if (error) {
				nshc_calls->NSH_printf_err("gestalt: The selector '%.4s' produces an error = %d\r", &selector, error );
				return_status = NSHC_ERR_GENERAL;
				}
				
			}
		else {
			nshc_calls->NSH_printf_err("gestalt: Parameter number %d is not an OSType.\r", arg );
			return_status = NSHC_ERR_GENERAL;
			}
			
	nshc_parms->action = nsh_idle;
	nshc_parms->result = return_status;

Exit:

#ifdef __MWERKS__
	SetA4(oldA4);		// CodeWarrior needs to restore A4
#else
	;					// Think needs a ; to go with the Exit label
#endif
}

