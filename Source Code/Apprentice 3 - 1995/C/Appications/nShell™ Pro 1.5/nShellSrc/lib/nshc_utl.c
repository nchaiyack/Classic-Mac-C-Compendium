/* ========================================

	nshc_utl.c
	
	Copyright (c) 1993,1994,1995 Newport Software Development
		
	Revisions:
	
	10/20/94	John Jensen		Rewrite nshc_is_numeric_operand to test length
	
   ======================================== */
   
#include "nshc.h"
					
#include "nshc_utl.proto.h"

// ===== version tracking ======
				
int nshc_bad_version(t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls, int version)
{
	if ( nshc_parms->version != version ) {
		nshc_calls->NSH_putStr_err("\pThis command is not of a compatible version.\r");
		nshc_parms->result = NSHC_ERR_VERSION;
		nshc_parms->action = nsh_idle;
		return(1);
		}
	else
		return(0);
}

// ===== option exploration routines =====

int nshc_got_option(t_nshc_parms *nshc_parms, char option)
{
	char	c;
	int		pos;
	int		arg;
	int 	found;
	int		bad_format;
	
	arg = 1;
	found = 0;
	
	while (!found && (arg < nshc_parms->argc)) {
		pos = nshc_parms->argv[arg];
		if ( nshc_parms->arg_buf[pos++] == '-' ) {
			bad_format = 0;
			while (c = nshc_parms->arg_buf[pos++]) {
				if (c == option) found = arg;
				if ( ((( c < 'a' ) || ( c > 'z' )) && (( c < 'A' ) || ( c > 'Z' ))) )
					bad_format = 1;
				}
			}
		if (bad_format)
			found = 0;
		arg++;
		}
		
	return( found );
}

int nshc_is_numeric_operand(t_nshc_parms *nshc_parms, int arg)
{
	int		good;
	int		count;
	char	c;
	char	*arg_str;

	arg_str = &nshc_parms->arg_buf[nshc_parms->argv[arg]];
	
	count = 0;								// keep track of argument length

	if ( *arg_str == '-' ) {				// leading '-' is ok
		arg_str++;
		count++;
		}
		
	good = 1;

	while( good && ( c = *arg_str++ ) ) {
	
		if ((c < '0') || (c > '9'))			// everything else should be a numeral
			good = 0;
			
		count++;
		
		}
	
	if ( count > 255 )						// reject such big numbers out of hand
		good = 0;

	return( good );
}

int nshc_is_operand(t_nshc_parms *nshc_parms, int arg)
{
	int		pos;
	char	c;
	
	pos = nshc_parms->argv[arg];
	
	c = nshc_parms->arg_buf[pos++];
	
	return( c != '-' );
}

int nshc_next_operand(t_nshc_parms *nshc_parms, int start)
{
	int found;
	int arg;
	
	arg = start + 1;
	found = 0;
	
	while (!found && ( arg < nshc_parms->argc )) {
		if (nshc_is_operand( nshc_parms, arg ))
			found = arg;
		arg++;
		}
		
	return( found );
}
