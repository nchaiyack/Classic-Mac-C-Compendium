/* ========== the commmand file: ==========

	hack.c - I link this into the main app and set the HACKING
	flag in "nsh.h" when I want to step through a command.
	
	Copyright (c) 1993,1994,1995 Newport Software Development
	
   ========== the commmand file: ========== */

#include "multi.h"

void cmain(t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls);

void cmain(t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls)
{
	nshc_parms->action = nsh_idle;
}
