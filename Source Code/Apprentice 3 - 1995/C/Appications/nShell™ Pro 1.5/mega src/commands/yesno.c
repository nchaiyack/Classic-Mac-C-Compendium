/* ========== the commmand file: ==========

	yesno.c
	
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
#include "str_utl.proto.h"
#include "nshc_utl.proto.h"

/* ======================================== */

// our data record, NewHandled and attached to nshc_parms->data

typedef struct {

	Str255	theString;		// theString we are receiving fromthe keyboard
	int		overrun;		// non-zero if theString has exceeded 255 chars
	int		stdin;			// non-zero if we input from stdin

} t_yesno_data;

typedef t_yesno_data **t_yesno_handle;

/* ======================================== */

// prototypes - for local use only

void yesno_by_parameters( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls );
void yesno_collect( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls );
void yesno_continue( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls );
void yesno_getchar( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls );
void yesno_print( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls );
void yesno_start( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls );
void yesno_stop( t_nshc_parms *nshc_parms );

/* ======================================== */

// if the user put his text on the command line, do it all in one operation

void yesno_print( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls )
{
	int				size;
	int				lastchar;
	t_yesno_handle	ndata;
	
	ndata = (t_yesno_handle)(nshc_parms->data);
		
	HLock( (Handle)ndata );
	
	size = (**ndata).theString[0];
	
	if (size) {
		nshc_calls->NSH_putStr( (**ndata).theString );
		lastchar = (**ndata).theString[size];
		if ((lastchar == '\r') || (lastchar == ' '))
			nshc_calls->NSH_puts("(y/n): ");
		else
			nshc_calls->NSH_puts(" (y/n): ");
		}
	else
		nshc_calls->NSH_puts("(y/n): ");
		
	HUnlock( (Handle)ndata );
}

/* ======================================== */

// if the user put his text on the command line, do it all in one operation

void yesno_by_parameters( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls )
{
	int				arg;
	int				got_one;
	int				length;
	char			*p;
	Str255			localString;
	t_yesno_handle	ndata;
	
	nshc_parms->result = NSHC_NO_ERR;
	nshc_parms->action = nsh_continue;
			
	ndata = (t_yesno_handle)(nshc_parms->data);
			
	arg = 1;
	got_one = 0;
	length = 0;
	localString[0] = 0;
	
	while ( arg < nshc_parms->argc ) {
		if ( nshc_is_operand( nshc_parms, arg) ) {
			p = &nshc_parms->arg_buf[nshc_parms->argv[arg]];
			length += cStrLen(p);
			if (length < 255) {
				if (got_one)
					localString[++localString[0]] = ' ';
				pStrAppendC( localString, p );
				}
			got_one = 1;
			}
		arg++;
		}
		
	if (length > 255) {
			nshc_calls->NSH_putStr_err("\pyesno: stdin exceeds 255 chars.\r");
			nshc_parms->result = NSHC_ERR_GENERAL;
			nshc_parms->action = nsh_stop;
			}
		else
			if (length) {
				HLock( (Handle)ndata );
				pStrCopy( (**ndata).theString, localString );
				HUnlock( (Handle)ndata );
				yesno_print( nshc_parms, nshc_calls );
				}
			else
				nshc_calls->NSH_puts("(y/n): ");
		
}

/* ======================================== */

// this _continue routine is used to pick up input from stdin, it loops until
// an end-of-input is found. It posts the input text if it does not exceed 255
// characters, otherwise it posts an error message.  If input exceeds 255 chars,
// all extra chars are eaten.

void yesno_collect( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls )
{
	int		value;
	int		size;
	Str255	localString;
	
	t_yesno_handle	ndata;
	
	ndata = (t_yesno_handle)(nshc_parms->data);
		
	HLock( (Handle)ndata );
	
	value = nshc_calls->NSH_getStr( localString );
	
	if (value) {
		size = localString[0] + (**ndata).theString[0];
		if (size > 255) (**ndata).overrun = 1;
		if (!(**ndata).overrun) pStrAppend( (**ndata).theString, localString );
		}
		
	HUnlock( (Handle)ndata );
		
	if (value == -1) {
	
		if ((**ndata).overrun) {
			nshc_calls->NSH_putStr_err("\pyesno: stdin exceeds 255 chars.\r");
			nshc_parms->result = NSHC_ERR_GENERAL;
			}
		else {
			yesno_print( nshc_parms, nshc_calls );
			nshc_parms->result = NSHC_NO_ERR;
			}
			
		(**ndata).stdin = 0;	
		}
}

/* ======================================== */

void yesno_getchar( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls )
{
	switch ( nshc_calls->NSH_getchar() ) {
		case 'y':
		case 'Y':									// affirmative
			nshc_calls->NSH_putchar('\r');
			nshc_parms->action = nsh_stop;
			nshc_parms->result = 0;
			break;
		case 'n':
		case 'N':									// negative
			nshc_calls->NSH_putchar('\r');
			nshc_parms->action = nsh_stop;
			nshc_parms->result = 1;
			break;
		case '\0':									// null when no keys pressed
			break;
		case -1:									// bail if end of input is found
			nshc_parms->action = nsh_stop;
			nshc_parms->result = -1;
			break;
		default:
			nshc_calls->NSH_putchar('\r');
			yesno_print( nshc_parms, nshc_calls ); // huh?
			break;
		}
}

/* ======================================== */

// state machine start

void yesno_start( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls )
{
	t_yesno_handle	ourData;

	ourData = (t_yesno_handle)NewHandle( sizeof(t_yesno_data) );

	if (ourData) {
		(**ourData).theString[0] = 0;
		(**ourData).overrun = 0;
		(**ourData).stdin = nshc_got_option( nshc_parms, 'i' );
		nshc_parms->data = (Handle)ourData;
		nshc_parms->action = nsh_continue;
		if (!(**ourData).stdin)
			yesno_by_parameters( nshc_parms, nshc_calls );
		}
	else {
		nshc_calls->NSH_putStr_err("\pyesno: Could not allocate storage.\r");
		nshc_parms->result = NSHC_ERR_MEMORY;
		nshc_parms->action = nsh_idle;
		}
}

/* ======================================== */

// state machine continue

void yesno_continue( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls )
{
	if ((**(t_yesno_handle)(nshc_parms->data)).stdin)
		yesno_collect(nshc_parms,nshc_calls);
	else
		yesno_getchar(nshc_parms,nshc_calls);
}

/* ======================================== */

// state machine stop

void yesno_stop( t_nshc_parms *nshc_parms )
{		
	if (nshc_parms->data)
		DisposeHandle(nshc_parms->data);

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
				yesno_start(nshc_parms,nshc_calls);
				break;
			case nsh_continue:
				yesno_continue(nshc_parms,nshc_calls);
				break;
			default:
				yesno_stop(nshc_parms);
				break;
			}
			
		}

#ifdef __MWERKS__
	SetA4(oldA4);
#endif
}
