/* ========== the commmand file: ==========

	date.c
	
	Copyright (c) 1993,1994 Newport Software Development
	
	You may distribute unmodified copies of this file for
	noncommercial purposes.  You may use this file as a
	reference when writing your own nShell(tm) commands.
	
	All other rights are reserved.
	
   ========== the commmand file: ========== */

#ifdef __MWERKS__			// CodeWarrior requires an A4 setup
#include <A4Stuff.h>
#endif

#include <OSUtils.h>
#include <Packages.h>

#include "nshc.h"

#include "nshc_utl.proto.h"

void main(t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls)
{
	int date_fmt;
	int seconds;
	unsigned long secs;
	Str255 resultStr;
	
#ifdef __MWERKS__
	long oldA4  = SetCurrentA4();
#endif
	
	if ( !nshc_bad_version( nshc_parms, nshc_calls, NSHC_VERSION ) ) {
	
		if (nshc_got_option( nshc_parms, 's' )) 
			date_fmt = shortDate;
		else
			if (nshc_got_option( nshc_parms, 'a' )) 
				date_fmt = abbrevDate;
			else
				date_fmt = longDate;
	
		GetDateTime( &secs );
		
		IUDateString( secs, date_fmt, resultStr );
		nshc_calls->NSH_putStr(resultStr);
		
		if ( ! nshc_got_option( nshc_parms, 'd' ) ) {
			nshc_calls->NSH_putchar(' ');
			if (nshc_got_option( nshc_parms, 'm' ))
				seconds = 0;
			else
				seconds = 1;
			IUTimeString( secs, seconds, resultStr );
			nshc_calls->NSH_putStr(resultStr);
			}
		
		nshc_calls->NSH_putchar('\r');
		
		nshc_parms->result = NSHC_NO_ERR;
		nshc_parms->action = nsh_idle;

		}

#ifdef __MWERKS__
	SetA4(oldA4);
#endif
}
