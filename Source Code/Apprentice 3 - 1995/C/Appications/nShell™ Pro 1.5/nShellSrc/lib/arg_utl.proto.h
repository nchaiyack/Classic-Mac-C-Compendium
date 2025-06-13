/* ========================================

	arg_utl.proto.h
	
	Copyright (c) 1993,1994,1995 Newport Software Development
		
   ======================================== */

int  arg_fss_utl( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls, int arg, FSSpec *spec );
int  arg_to_fss( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls, int arg, FSSpec *spec );
long arg_to_num(t_nshc_parms *nshc_parms, int arg);
int  arg_to_real_fss( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls, int arg, FSSpec *spec );
int  arg_to_str( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls, int arg, Str255 string );
