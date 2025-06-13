/* ========================================

	nshc_utl.proto.h
	
	Copyright (c) 1993,1994,1995 Newport Software Development
	
   ======================================== */

int  nshc_bad_version(t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls, int version);
int  nshc_got_option(t_nshc_parms *nshc_parms, char option);
int  nshc_is_operand(t_nshc_parms *nshc_parms, int arg);
int  nshc_next_operand(t_nshc_parms *nshc_parms, int start);
int  nshc_is_numeric_operand(t_nshc_parms *nshc_parms, int arg);
