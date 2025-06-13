/* ========== the commmand file: ==========

	echo.c
	
	Copyright (c) 1993,1994 Newport Software Development
	
	You may distribute unmodified copies of this file for
	noncommercial purposes.  You may use this file as a
	reference when writing your own nShell(tm) commands.
	
	All other rights are reserved.
	
   ========== the commmand file: ========== */

#ifdef __MWERKS__			// Get the appropriate A4 stuff
#include <A4Stuff.h>
#else
#include <SetUpA4.h>
#endif

#include "nshc.h"

#include "buf_utl.proto.h"
#include "nshc_utl.proto.h"

/* ======================================== */

void main(t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls)
{
	int arg, argc;
	
#ifdef __MWERKS__
	long oldA4  = SetCurrentA4();
#else
	RememberA0();
	SetUpA4();
#endif
	
	if ( !nshc_bad_version( nshc_parms, nshc_calls, NSHC_VERSION ) ) {
	
		buf_init( nshc_calls );
		
		nshc_parms->action = nsh_idle;
		nshc_parms->result = NSHC_NO_ERR;
		
		arg = 1;
		argc = nshc_parms->argc;
		
		while ( arg < argc ) {
			if ( arg > 1 )
				buf_putchar( ' ' );
			buf_puts( &nshc_parms->arg_buf[nshc_parms->argv[arg]] );
			arg++;
			}
			
		if ( argc > 1 ) buf_putchar( '\r' );
		
		buf_flush();
				
		}

#ifdef __MWERKS__
	SetA4(oldA4);
#else
	RestoreA4();
#endif
}
