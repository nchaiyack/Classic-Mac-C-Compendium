/* ========== the commmand file: ==========

	inc.c
	
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

#include "arg_utl.proto.h"
#include "nshc_utl.proto.h"
#include "str_utl.proto.h"

/* ======================================== */

int  inc_number(Str255 value);
int  inc_init(t_nshc_parms *nshc_parms,t_nshc_calls *nshc_calls,Str32 name,Str255 var,long *value,long *offset);
			 
/* ======================================== */

int inc_number(Str255 value)
{
	int		pos;
	char	c;
	
	pos = 1;
	
	if ( value[ pos ] == '-' ) pos++;

	for ( ; pos <= value[0]; pos++ ) {
		c = value[pos];
		if ((c < '0') || (c > '9'))
			return( 1 );
		}
	
	return( 0 );
}

/* ======================================== */

int inc_init( t_nshc_parms *nshc_parms,
			 t_nshc_calls *nshc_calls,
			 Str32 name,
			 Str255 var,
			 long *value,
			 long *offset )	 
{
	char	*p;
	char	*q;
	char	c;
	int		argc;
	int		i;
	int		error;
	
	// return if bad include file version
		
	if (nshc_bad_version( nshc_parms, nshc_calls, NSHC_VERSION ))
		return( 0 );
	
	// return if bad usage
	
	argc = nshc_parms->argc;
	
	if ( ( argc < 2 ) || ( argc > 3 ) ) {
		nshc_calls->NSH_putStr_err( "\pUsage: " );
		nshc_calls->NSH_putStr_err( "\pinc: variable [offset]\r" );
		nshc_parms->result = NSHC_ERR_PARMS;
		nshc_parms->action = nsh_idle;
		return( 0 );
		}
	
	// return if bad variable name
	
	p = q = &nshc_parms->arg_buf[ nshc_parms->argv[ 1 ] ];
	i = 0;
	while (c = *p++) {
		error = 1;
		if ( c == '_' ) error = 0; else
		if ( ( c >= 'a' ) && ( c <= 'z' ) ) error = 0; else
		if ( ( c >= 'A' ) && ( c <= 'Z' ) ) error = 0; else
		if ( ( c >= '0' ) && ( c <= '9' ) ) error = 0;
		if (error) {
		nshc_calls->NSH_putStr_err( "\pinc: Invalid variable name = " );
			nshc_calls->NSH_puts_err( q );
			nshc_calls->NSH_putchar_err( '\r' );
			nshc_parms->result = NSHC_ERR_PARMS;
			nshc_parms->action = nsh_idle;
			return( 0 );
			}
		name[++i] = c;
		}
		
	if ( i > 31 ) {
		nshc_calls->NSH_putStr_err( "\pinc: Variable name too long = " );
		nshc_calls->NSH_puts_err( q );
		nshc_calls->NSH_putchar_err( '\r' );
		nshc_parms->result = NSHC_ERR_PARMS;
		nshc_parms->action = nsh_idle;
		return( 0 );
		}
		
	name[0] = i;

	if ( nshc_calls->NSH_var_env( name, var ) ) {
		nshc_calls->NSH_putStr_err( "\pinc: Variable not found = " );
		nshc_calls->NSH_putStr_err( name );
		nshc_calls->NSH_putchar_err( '\r' );
		nshc_parms->result = NSHC_ERR_PARMS;
		nshc_parms->action = nsh_idle;
		return( 0 );
		}
		
	if ( inc_number( var ) ) {
		nshc_calls->NSH_putStr_err( "\pinc: variable must contain an integer.\r" );
		nshc_parms->result = NSHC_ERR_PARMS;
		nshc_parms->action = nsh_idle;
		return( 0 );
		}
		
	StringToNum( var, value );
		
	// return if bad offset
	
	*offset = 1;
	
	if ( argc == 3 )
		if ( nshc_is_numeric_operand( nshc_parms, 2 ) )
			*offset = arg_to_num( nshc_parms, 2 );
		else {
			nshc_calls->NSH_putStr_err( "\pinc: Offset must be an integer.\r" );
			nshc_parms->result = NSHC_ERR_PARMS;
			nshc_parms->action = nsh_idle;
			return( 0 );
			}
	
	return( 1 );
}

/* ======================================== */

void main(t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls)
{
	Str32	name;
	Str255	var;
	long	value;
	long	offset;
	
#ifdef __MWERKS__
	long oldA4  = SetCurrentA4();
#endif
		
	if ( inc_init( nshc_parms, nshc_calls, name, var, &value, &offset ) ) {
	
		value = value + offset;
		
		NumToString( value, var );
		
  		nshc_parms->result = nshc_calls->NSH_var_set( name, var );
  		
  		}
  		
	nshc_parms->action = nsh_idle;

#ifdef __MWERKS__
	SetA4(oldA4);		// CodeWarrior needs to restore A4
#endif
}

