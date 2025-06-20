/* ========================================

	arg_utl.proto.h
	
	Copyright (c) 1993,1994 Newport Software Development
	
	You may distribute unmodified copies of this file for
	noncommercial purposes.  You may use this file as a
	reference when writing your own nShell(tm) commands.
	
	All other rights are reserved.
	
   ======================================== */

int  arg_fss_utl( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls, int arg, FSSpec *spec );
int  arg_to_fss( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls, int arg, FSSpec *spec );
long arg_to_num(t_nshc_parms *nshc_parms, int arg);
int  arg_to_real_fss( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls, int arg, FSSpec *spec );
int  arg_to_str( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls, int arg, Str255 string );
