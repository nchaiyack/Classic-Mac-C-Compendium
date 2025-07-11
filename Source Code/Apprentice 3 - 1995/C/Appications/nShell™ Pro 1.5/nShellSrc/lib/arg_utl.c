/* ========================================

	arg_utl.c
	
	Copyright (c) 1993,1994, 1995 Newport Software Development
	
	Revisions:
	
	10/20/94	John Jensen		Dropped arg_to_int
								Added arg_to_num
								Changed error string in arg_to_str
	
   ======================================== */
   
#include "nshc.h"
					
#include "str_utl.proto.h"
#include "arg_utl.proto.h"

// ===== argument converson routines =====

// assuming the argv has been checked by nshc_is_numeric_operand
		
long arg_to_num(t_nshc_parms *nshc_parms, int arg)
{
	char	*arg_str;
	Str255	string;
	long	value;
		
	arg_str = &nshc_parms->arg_buf[nshc_parms->argv[arg]];

	pStrFromC( string, arg_str );

	StringToNum( string, &value );
		
	return( value );
}

int arg_to_str( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls, int arg, Str255 string )
{
	char	*app_name;
	char	*arg_str;
	int		result;
	
	result = NSHC_NO_ERR;
	
	arg_str = &nshc_parms->arg_buf[nshc_parms->argv[arg]];
		
	if ( cStrLen( arg_str ) > 255 ) {
		app_name = &nshc_parms->arg_buf[nshc_parms->argv[0]];
		nshc_calls->NSH_puts_err(app_name);
		nshc_calls->NSH_putStr_err("\p: String is too long = ");
		nshc_calls->NSH_puts_err(arg_str);
		nshc_calls->NSH_putchar_err('\r');
		result = NSHC_ERR_PARMS;
		}
	else
		pStrFromC( string, arg_str );
			
	return( result );
}

int arg_fss_utl( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls, int arg, FSSpec *spec )
{
	char	*app_name;
	char	*arg_str;
	int		result;
	Str255	pathname;
		
	result = NSHC_ERR_PARMS;
	
	arg_str = &nshc_parms->arg_buf[nshc_parms->argv[arg]];
		
	if ( cStrLen( arg_str ) > 255 ) {
		nshc_calls->NSH_puts_err(&nshc_parms->arg_buf[nshc_parms->argv[0]]);
		nshc_calls->NSH_putStr_err("\p: Pathname is too long = ");
		nshc_calls->NSH_puts_err(arg_str);
		nshc_calls->NSH_putchar_err('\r');
		}
	else {
		pStrFromC( pathname, arg_str );
		if (!nshc_calls->NSH_path_expand( pathname ))
			result = nshc_calls->NSH_path_to_FSSpec( pathname, spec );
		}
			
	return( result );
}
 
int arg_to_fss( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls, int arg, FSSpec *spec )
{
	int		result;
	
	result = arg_fss_utl(nshc_parms, nshc_calls, arg, spec );
	
	if ( ( result == noErr ) || ( result == fnfErr ) )
		result = NSHC_NO_ERR;
				
	if (result) {
		nshc_calls->NSH_puts_err(&nshc_parms->arg_buf[nshc_parms->argv[0]]);
		nshc_calls->NSH_putStr_err("\p: Bad pathname = ");
		nshc_calls->NSH_puts_err(&nshc_parms->arg_buf[nshc_parms->argv[arg]]);
		nshc_calls->NSH_putchar_err('\r');
		}
		
	return( result );
}

int arg_to_real_fss( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls, int arg, FSSpec *spec )
{
	int		result;
	
	result = arg_fss_utl(nshc_parms, nshc_calls, arg, spec );
	
	if (result) {
		nshc_calls->NSH_puts_err(&nshc_parms->arg_buf[nshc_parms->argv[0]]);
		nshc_calls->NSH_putStr_err("\p: Bad pathname = ");
		nshc_calls->NSH_puts_err(&nshc_parms->arg_buf[nshc_parms->argv[arg]]);
		nshc_calls->NSH_putchar_err('\r');
		}
		
	return( result );
}


