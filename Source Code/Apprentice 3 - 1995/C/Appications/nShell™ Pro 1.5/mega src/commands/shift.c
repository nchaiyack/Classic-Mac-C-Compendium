/* ========== the commmand file: ==========

	shift.c - an nShell(tm) command.
	
	Copyright (c) 1993,1994 Newport Software Development
	
	You may distribute unmodified copies of this file for
	noncommercial purposes.  You may use this file as a
	reference when writing your own nShell(tm) commands.
	
	All other rights are reserved.
	
   ========== the commmand file: ========== */
   
#ifdef __MWERKS__			// CodeWarrior requires an A4 setup
#include <A4Stuff.h>
#endif

#include "Folders.h"

#include "nshc.h"

#include "arg_utl.proto.h"
#include "str_utl.proto.h"
#include "nshc_utl.proto.h"

/* ======================================== */

// We do this all in one pass to try to speed things up.
// It's still a slow process, what with creating and searching
// for all the variables based upon their names (strings).
// Perhaps in future versions I might save numeric variables
// with numeric (binary) keys.  That should speed things up.

int shift(t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls);

int shift(t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls)
{
	OSErr	result;
	long	distance;
	int		count;
	int		to_num;
	Str32	to_name;
	int		from_num;
	Str32	from_name;
	Str255	value;
	
	result = 0;
	
	// bail if bad parameters
	
	if ( nshc_parms->argc > 2 ) {
		nshc_calls->NSH_putStr_err( "\pUsage: shift [distance]\r" );
		return( NSHC_ERR_PARMS );
		}

	if ( nshc_parms->argc == 2 ) {
		if ( nshc_is_numeric_operand( nshc_parms, 1) )
			distance = arg_to_num( nshc_parms, 1 );
		else {
			nshc_calls->NSH_putStr_err( "\pshift: Distance parameter must be an integer.\r" );
			return( NSHC_ERR_PARMS );
			}
		}
	else
		distance = 1;
		
	// this loop shifts down variables the specified distance
		
	to_num = 0;
	from_num = distance;
	count = 0;
		
	while ( !result ) {
	
		NumToString( from_num, from_name );
		result = nshc_calls->NSH_var_env( from_name, value );
		
		if ( !result ) {
			NumToString( to_num, to_name );
			result = nshc_calls->NSH_var_set( to_name, value );
			count++;
			}
			
		to_num++;
		from_num++;
		}
		
	// set $# to be the number of numeric parameters remaining
	
	NumToString( count, value );
	nshc_calls->NSH_var_set( "\p#", value );

	// delete extra variables from the end of the list
	
	for ( to_num = count ; to_num < count + distance ; to_num++ ) {
		NumToString( to_num, to_name );
		nshc_calls->NSH_var_unset( to_name );
		}
		
	return( !count );
}
	
/* ======================================== */

void main(t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls)
{
	OSErr	result;
	
#ifdef __MWERKS__
	long oldA4  = SetCurrentA4();
#endif
	
	if (!nshc_bad_version( nshc_parms, nshc_calls, NSHC_VERSION )) {
	
		// otherwise, handle requests from the application
	
	  	switch (nshc_parms->action) {
			case nsh_start:
				result = shift( nshc_parms, nshc_calls );
				break;
			case nsh_continue:
			case nsh_idle:
			case nsh_stop:
				result = NSHC_NO_ERR;
				break;
			}
		
		// tell the application we are done

 		nshc_parms->action = nsh_idle;
		nshc_parms->result = result;

		}
		
#ifdef __MWERKS__
	SetA4(oldA4);		// CodeWarrior needs to restore A4
#endif
}
