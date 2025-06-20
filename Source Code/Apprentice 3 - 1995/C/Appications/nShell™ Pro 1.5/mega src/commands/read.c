/* ========== the commmand file: ==========

	read.c
	
	Copyright (c) 1994 Newport Software Development
	
	You may distribute unmodified copies of this file for
	noncommercial purposes.  You may use this file as a
	reference when writing your own nShell(tm) commands.
	
	All other rights are reserved.
	
   ========== the commmand file: ========== */

#ifdef __MWERKS__			// CodeWarrior requires an A4 setup
#include <A4Stuff.h>
#endif

#include "nshc.h"
#include "str_utl.proto.h"
#include "nshc_utl.proto.h"

/* ======================================== */

typedef enum { ERROR_NONE, ERROR_USAGE, ERROR_QUOTES, ERROR_INPUT } t_read_error;

/* ======================================== */

// prototypes - for local use only

void read_error( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls, int error );
void read_start( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls );
void read_continue( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls );
void read_stop( t_nshc_parms *nshc_parms );

/* ======================================== */

// report errors and set return code

void read_error( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls, int error )
{

	switch (error) {
		case ERROR_USAGE:
			nshc_calls->NSH_putStr_err( "\pUsage: read variable [variable...]\r" );
			nshc_parms->result = NSHC_ERR_PARMS;
			break;
		case ERROR_QUOTES:
			nshc_calls->NSH_putStr_err( "\pread: Unbalanced quotes.\r" );
			nshc_parms->result = NSHC_ERR_GENERAL;
			break;
		case ERROR_INPUT:
			nshc_calls->NSH_putStr_err( "\pread: Input string is too long.\r" );
			nshc_parms->result = NSHC_ERR_GENERAL;
			break;
		}
		
	nshc_parms->action = nsh_idle;
}

/* ======================================== */

// state machine start

void read_start( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls )
{
	int		arg;
	int		error;
	int		i;
	char	c;
	char	*p;
	char	*q;
	
	nshc_parms->action = nsh_continue;
	nshc_parms->result = NSHC_NO_ERR;
	
	// bail if bad usage
	
	if (nshc_parms->argc < 2) {
		read_error( nshc_parms, nshc_calls, ERROR_USAGE );
		return;
		}
	
	// or bail if any parameter is not a good variable
	
	for ( arg = 1; arg < nshc_parms->argc; arg++ ) {
		p = q = &nshc_parms->arg_buf[ nshc_parms->argv[ arg ] ];
		i = 0;
		while (c = *p++) {
			error = 1;
			if ( c == '_' ) error = 0; else
			if ( ( c >= 'a' ) && ( c <= 'z' ) ) error = 0; else
			if ( ( c >= 'A' ) && ( c <= 'Z' ) ) error = 0; else
			if ( ( c >= '0' ) && ( c <= '9' ) ) error = 0;
			if (error) {
				nshc_calls->NSH_putStr_err( "\pread: Invalid variable name = " );
				nshc_calls->NSH_puts_err( q );
				nshc_calls->NSH_putchar_err( '\r' );
				nshc_parms->result = NSHC_ERR_PARMS;
				nshc_parms->action = nsh_idle;
				return;
				}
			i++;
			}
		if ( i > 31 ) {
			nshc_calls->NSH_putStr_err( "\pread: Variable name too long = " );
			nshc_calls->NSH_puts_err( q );
			nshc_calls->NSH_putchar_err( '\r' );
			nshc_parms->result = NSHC_ERR_PARMS;
			nshc_parms->action = nsh_idle;
			return;
			}
		}
	
	// otherwise, might as well take one pass at some real work
	
	read_continue( nshc_parms, nshc_calls );
}

/* ======================================== */

// state machine continue

void read_continue( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls )
{
	int		arg;
	int		more;
	int		got_one;
	int		last_one;
	char	c;
	char	quote;
	int		i,j;
	char	my_buf[LINE_MAX];
	Str32	name;
	Str255	value;
	
	// try the input buffer
	
	i = nshc_calls->NSH_gets( my_buf, LINE_MAX );
	
	// if we got a line, process it
	
	if (i) {
	
		i = 0;
		arg = 0;
		got_one = 0;
		last_one = 0;
		c = my_buf[i];
		
		// as long as characters are in the buf and args to fill
	
		while ( c && ( c != '\r') && (arg < nshc_parms->argc) ) {
		
			// on all but last one, advance variable and set up variable name
			
			if (!last_one) {
				arg++;
				last_one = arg == (nshc_parms->argc - 1);
				pStrFromC( name, &nshc_parms->arg_buf[ nshc_parms->argv[ arg ] ] );
				j = 0;
				}
				
			// eat whitespace

			while ( c && (( c == ' ' ) || ( c == '\t' )))
				c = my_buf[++i];
				
			// if the string starts with a quote, set it up
			
			if ( ( c == '\'') || ( c == '"' ) ) {
				quote = c;
				c = my_buf[++i];
				}
			else
				quote = 0;
				
			// if this is the last_one, and we have a partial string,
			// and more to add, we need to pad with a blank
			
			if (last_one && j && c && (c != '\r' )) 
				if ( j < 255 )
					value[++j] = ' ';
				else {
					read_error( nshc_parms, nshc_calls, ERROR_INPUT );
					return;
					}
			
			// put non-whitespace characters into the value string
				
			while ( c ) {
			
				// end an unquoted variable on a whitespace
			
				if ( !quote && (( c == ' ' ) || ( c == '\t' )) )
					break;
					
				// end a quoted string on that character
					
				if ( c == quote )
					break;
					
				// end any string on a Return char
					
				if ( c == '\r' )
					break;

				// it's an error to have quotes in non-quoted strings
			
				if ( !quote && ( ( c == '\'') || ( c == '"' ) ) ) {
					read_error( nshc_parms, nshc_calls, ERROR_QUOTES );
					return;
					}
					
				// add the character if there is room
				
				if ( j < 255 )
					value[++j] = c;
				else {
					read_error( nshc_parms, nshc_calls, ERROR_INPUT );
					return;
					}
					
				c = my_buf[++i];
					
				}
					
			value[0] = j;
			
			if (quote) {
			
				// if a quoted string did not end with a quote, bail
				// otherwise, advance past last quoted string
				
				if ( c != quote ) {
					read_error( nshc_parms, nshc_calls, ERROR_QUOTES );
					return;
					}
				else
					c = my_buf[++i];
	
				}
							
			// figure out if there is 'more' to add to the 'last_one'
			
			more = last_one && ((c == ' ') || (c == '\t'));
			
			// if we have some strings, and there is no 'more', store them away
			
			if ( name[0] && value[0] && !more ) {
				nshc_calls->NSH_var_set( name, value );
				got_one = 1;
				}
					
			}
			
		// consider it an error if no variables were filled
			
		if ( !got_one )
			nshc_parms->result = 1;
			
		read_stop( nshc_parms );
		
		}
}

/* ======================================== */

// state machine stop

void read_stop( t_nshc_parms *nshc_parms )
{
	nshc_parms->action = nsh_idle;
}

/* ======================================== */

void main(t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls)
{
#ifdef __MWERKS__
	long oldA4  = SetCurrentA4();
#endif
	
	if ( !nshc_bad_version( nshc_parms, nshc_calls, NSHC_VERSION ) ) {
		
		switch (nshc_parms->action) {
			case nsh_start:
				read_start(nshc_parms,nshc_calls);
				break;
			case nsh_continue:
				read_continue(nshc_parms,nshc_calls);
				break;
			default:
				read_stop(nshc_parms);
				break;
			}

		}

#ifdef __MWERKS__
	SetA4(oldA4);
#endif
}
