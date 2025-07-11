/* ========== the commmand file: ==========

	eq.c
	
	Copyright (c) 1993,1994 Newport Software Development
	
	You may distribute unmodified copies of this file for
	noncommercial purposes.  You may use this file as a
	reference when writing your own nShell(tm) commands.
	
	All other rights are reserved.
	
   ========== the commmand file: ========== */

/* ======================================== */

// possible choices: DO_EQ, DO_NE, DO_GT, DO_LT, DO_GE, DO_LE

#define DO_LE

/* ======================================== */

#ifdef __MWERKS__			// CodeWarrior requires an A4 setup
#include <A4Stuff.h>
#endif

#include "nshc.h"

#include "arg_utl.proto.h"
#include "nshc_utl.proto.h"
#include "str_utl.proto.h"

/* ======================================== */

void eq_name( t_nshc_calls *nshc_calls );

void eq_name( t_nshc_calls *nshc_calls )
{
#if defined DO_EQ
	nshc_calls->NSH_putStr_err( "\p.eq." );
#elif defined DO_NE
	nshc_calls->NSH_putStr_err( "\p.ne." );
#elif defined DO_GT
	nshc_calls->NSH_putStr_err( "\p.gt." );
#elif defined DO_LT
	nshc_calls->NSH_putStr_err( "\p.lt." );
#elif defined DO_LE
	nshc_calls->NSH_putStr_err( "\p.le." );
#elif defined DO_GE
	nshc_calls->NSH_putStr_err( "\p.ge." );
#endif
}
			
/* ========================================== */

int	eq_str_less( char *p, char *q );

int	eq_str_less( char *p, char *q )
{
	char jim,bob;
	
	do {
		jim = *p++;
		bob = *q++;
		if ( jim != bob )
			return( jim < bob );
	} while ( jim );
			
	return(0);
}

/* ========================================== */

int	eq_str_more( char *p, char *q );

int	eq_str_more( char *p, char *q )
{
	char jim,bob;
	
	do {
		jim = *p++;
		bob = *q++;
		if ( jim != bob )
			return( jim > bob );
	} while ( jim );
			
	return(0);
}
/* ======================================== */

void main(t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls)
{
	int		numeric;
	char	*str1;
	char	*str2;
	long	value1;
	long	value2;
	int		result;
	
#ifdef __MWERKS__
	long oldA4  = SetCurrentA4();
#endif
	
	result = -1;
	
	// exit if the version of our nshc.h does not match the one the application used.

	if (nshc_bad_version( nshc_parms, nshc_calls, NSHC_VERSION )) goto Exit;
	
	if ( nshc_parms->argc != 3 ) {
		nshc_calls->NSH_putStr_err( "\pUsage: " );
		eq_name( nshc_calls );
		nshc_calls->NSH_putStr_err( "\p Value1 Value2\r" );
		nshc_parms->result = NSHC_ERR_PARMS;
		nshc_parms->action = nsh_idle;
		goto Exit;
		}
	
	numeric = nshc_is_numeric_operand(nshc_parms,1) &&
			  nshc_is_numeric_operand(nshc_parms,2);

	if (numeric) {
		value1 =  arg_to_num(nshc_parms,1);
		value2 =  arg_to_num(nshc_parms,2);
		#if defined DO_EQ
			result = !( value1 == value2 );
		#elif defined DO_NE
			result = !( value1 != value2 );
		#elif defined DO_GT
			result = !( value1 > value2 );
		#elif defined DO_LT
			result = !( value1 < value2 );
		#elif defined DO_LE
			result = !( value1 <= value2 );
		#elif defined DO_GE
			result = !( value1 >= value2 );
		#endif
		}
	else {
		str1 = &nshc_parms->arg_buf[ nshc_parms->argv[1] ];
		str2 = &nshc_parms->arg_buf[ nshc_parms->argv[2] ];
		#if defined DO_EQ
			result = !( cStrEqual( str1, str2 ) );
		#elif defined DO_NE
			result =  ( cStrEqual( str1, str2 ) );
		#elif defined DO_GT
			result = !( eq_str_more( str1, str2 ) );
		#elif defined DO_LT
			result = !( eq_str_less( str1, str2 ) );
		#elif defined DO_LE
			if ( cStrEqual( str1, str2 ) )
				result = 0;
			else
				if ( eq_str_less( str1, str2 ) )
					result = 0;
				else
					result = 1;
		#elif defined DO_GE
			if ( cStrEqual( str1, str2 ) )
				result = 0;
			else
				if ( eq_str_more( str1, str2 ) )
					result = 0;
				else
					result = 1;
		#endif
		}
		
  	nshc_parms->result = result;
		
	// tell the application we are done - for any requested action

	nshc_parms->action = nsh_idle;

Exit:

#ifdef __MWERKS__
	SetA4(oldA4);		// CodeWarrior needs to restore A4
#else
	;					// Think needs a ; to go with the Exit label
#endif
}

