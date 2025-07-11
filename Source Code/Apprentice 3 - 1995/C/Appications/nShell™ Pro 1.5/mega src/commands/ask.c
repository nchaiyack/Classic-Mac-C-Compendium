/* ========== the commmand file: ==========

	ask.c
	
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

} t_ask_data;

typedef t_ask_data **t_ask_handle;

/* ======================================== */

// prototypes - for local use only

int ask_by_parameters( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls );
void ask_continue( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls );
void ask_display( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls, Str255 theString );
void ask_start( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls );
void ask_stop( t_nshc_parms *nshc_parms );

// utility

void ask_display( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls, Str255 theString )
{
	int		size;

	size = 1;
	
	if (nshc_got_option( nshc_parms, 's')) size = 0;
	if (nshc_got_option( nshc_parms, 'l')) size = 2;
	
	nshc_parms->result = nshc_calls->NSH_ask( theString, size );
}

/* ======================================== */

// if the user put his text on the command line, do it all in one operation

int ask_by_parameters( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls )
{
	int				arg;
	int				got_one;
	int				length;
	char			*p;
	Str255			theString;
	
	arg = 1;
	got_one = 0;
	length = 0;
	theString[0] = 0;
	
	while ( arg < nshc_parms->argc ) {
		if ( nshc_is_operand( nshc_parms, arg) ) {
			p = &nshc_parms->arg_buf[nshc_parms->argv[arg]];
			length += cStrLen(p);
			if (length < 255) {
				if (got_one)
					theString[++theString[0]] = ' ';
				pStrAppendC( theString, p );
				}
			got_one = 1;
			}
		arg++;
		}
		
	if (got_one)
		if (length <= 255)
			ask_display( nshc_parms, nshc_calls, theString );
		else{
			nshc_calls->NSH_putStr_err("\pask: stdin exceeds 255 chars.\r");
			nshc_parms->result = NSHC_ERR_GENERAL;
			}

	return( got_one );
}

/* ======================================== */

// state machine

void ask_start( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls )
{
	t_ask_handle	ourData;

	nshc_parms->action = nsh_idle;
	nshc_parms->result = NSHC_NO_ERR;
	
	if ( ask_by_parameters( nshc_parms, nshc_calls ) )
		return;

	ourData = (t_ask_handle)NewHandle( sizeof(t_ask_data) );

	if (ourData) {
		(**ourData).theString[0] = 0;
		(**ourData).overrun = 0;
		nshc_parms->data = (Handle)ourData;
		nshc_parms->action = nsh_continue;
		}
	else {
		nshc_calls->NSH_putStr_err("\pask: Could not allocate storage.\r");
		nshc_parms->result = NSHC_ERR_MEMORY;
		}
}

/* ======================================== */

// this _continue routine is used to pick up input from stdin, it loops until
// an end-of-input is found. It posts the input text if it does not exceed 255
// characters, otherwise it posts an error message.  If input exceeds 255 chars,
// all extra chars are eaten.

void ask_continue( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls )
{
	int		value;
	int		size;
	Str255	localString;
	
	t_ask_handle	ndata;
	
	ndata = (t_ask_handle)(nshc_parms->data);
	
	HLock( (Handle)ndata );
	
	value = nshc_calls->NSH_getStr( localString );
	
	if (value) {
		size = localString[0] + (**ndata).theString[0];
		if (size > 255) (**ndata).overrun = 1;
		if (!(**ndata).overrun) pStrAppend( (**ndata).theString, localString );
		}
		
	if (value == -1) {
	
		if ((**ndata).overrun) {
			nshc_calls->NSH_putStr_err("\pask: stdin exceeds 255 chars.\r");
			nshc_parms->result = NSHC_ERR_GENERAL;
			}
		else
			ask_display( nshc_parms, nshc_calls, (**ndata).theString );
			
		nshc_parms->action = nsh_stop;
		}
		
	HUnlock( (Handle)ndata );
}

/* ======================================== */

void ask_stop( t_nshc_parms *nshc_parms )
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
				ask_start(nshc_parms,nshc_calls);
				break;
			case nsh_continue:
				ask_continue(nshc_parms,nshc_calls);
				break;
			default:
				ask_stop(nshc_parms);
				break;
			}
		}
	
#ifdef __MWERKS__
	SetA4(oldA4);
#endif
}
