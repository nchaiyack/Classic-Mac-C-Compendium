/* ========================================

	nshc_utl.proto.h
	
	Copyright (c) 1993,1994 Newport Software Development
	
	You may distribute unmodified copies of this file for
	noncommercial purposes.  You may use this file as a
	reference when writing your own nShell(tm) commands.
	
	All other rights are reserved.
	
   ======================================== */

int  nshc_bad_version(t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls, int version);
int  nshc_got_option(t_nshc_parms *nshc_parms, char option);
int  nshc_is_operand(t_nshc_parms *nshc_parms, int arg);
int  nshc_next_operand(t_nshc_parms *nshc_parms, int start);
int  nshc_is_numeric_operand(t_nshc_parms *nshc_parms, int arg);
