/* ========== the commmand file: ==========

	chattr.c
	
	Copyright (c) 1993,1994 Newport Software Development
	
	You may distribute unmodified copies of this file for
	noncommercial purposes.  You may use this file as a
	reference when writing your own nShell(tm) commands.
	
	All other rights are reserved.
	
   ========== the commmand file: ========== */

#ifdef __MWERKS__			// CodeWarrior requires an A4 setup
#include <A4Stuff.h>
#endif

#include <script.h>
#include <string.h>

#include "nshc.h"

#include "arg_utl.proto.h"
#include "fss_utl.proto.h"
#include "nshc_utl.proto.h"
#include "str_utl.proto.h"

// data definition - this struct is the root of all data

typedef struct {

	int		got_fss;		// 0 if FSSpec calls are not available
	int		arg;			// position in arg list
	
	int		got_one;		// 0 until header is printed
	
	int		change_type;	// 0 if none, or arg pos of "-t"
	OSType	new_type;
	
	int		change_creator;	// 0 if none, or arg pos of "-c"
	OSType	new_creator;

} t_chattr_data;

typedef	t_chattr_data	**chattr_hndl;

/* ======================================== */

// prototypes - utility

int  chattr_get_OSType(t_nshc_parms *nshc_parms, int arg, OSType *selector);
void chattr_bad( t_nshc_parms *nshc_parms, int code );
void chattr_bad_file( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls, StringPtr msg );
void chattr_good( t_nshc_parms *nshc_parms );

// prototypes - file routines

void  chattr_one( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls, t_chattr_data **hData );

// prototypes - state machine

void chattr_start( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls  );
void chattr_continue( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls );
void chattr_stop( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls );

/* ======================================== */

// utility routines

int chattr_get_OSType(t_nshc_parms *nshc_parms, int arg, OSType *os_type)
{
	char	c;
	char	*p;
	int		i;
	int		success;
	
	if (arg >= nshc_parms->argc)
		return(0);
	
	success = 1;
	*os_type = 0;
	
	p = &nshc_parms->arg_buf[ nshc_parms->argv[ arg ] ];
	
	for (i = 0 ; i < 4 ; i++ )
		if ( c = *p++ ) {
			*os_type = *os_type << 8;
			*os_type = *os_type + c;
			}
		else
			success = 0;
			
	if (*p) success = 0;
	
	return(success);
}
	
/* ======================================== */

void chattr_bad(  t_nshc_parms *nshc_parms, int code )
{
	nshc_parms->action = nsh_stop;
	nshc_parms->result = code;
}
	
/* ======================================== */

void chattr_bad_file(  t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls, StringPtr msg )
{

	nshc_calls->NSH_putStr_err("\pchattr: File access error (");
	nshc_calls->NSH_putStr_err(msg);
	nshc_calls->NSH_putStr_err("\p)\r");

	nshc_parms->action = nsh_stop;
	nshc_parms->result = NSHC_ERR_GENERAL;
}
	
/* ======================================== */

void chattr_good(  t_nshc_parms *nshc_parms )
{
	nshc_parms->action = nsh_stop;
	nshc_parms->result = 0;
}

/* ======================================== */

// file access routines

/* ======================================== */

void chattr_one( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls, t_chattr_data **hData )
{
	int		result;
	FSSpec	fsspec;
	FInfo	fndrInfo;
	int		write;
	
	// =====> convert argument to fsspec
	
	result = arg_to_fss( nshc_parms, nshc_calls, (**hData).arg, &fsspec );

	(**hData).arg++;
	
	if (result) {
		chattr_bad( nshc_parms, result );
		return;
		}
	
	result = fss_GetFInfo((**hData).got_fss, &fsspec, &fndrInfo);
				
	if (result == fnfErr) {
		nshc_calls->NSH_putStr_err("\pchattr: Not a file (");
		nshc_calls->NSH_putStr_err((StringPtr)fsspec.name);
		nshc_calls->NSH_putStr_err("\p)\r");
		return;
		}
	
	if (!result) {
	
		write = 0;
	
		if ( (**hData).change_type ) {
			write = 1;
			fndrInfo.fdType = (**hData).new_type;
			}
			
		if ( (**hData).change_creator ) {
			write = 1;
			fndrInfo.fdCreator = (**hData).new_creator;
			}
			
		if (write) {
			result = fss_SetFInfo((**hData).got_fss, &fsspec, &fndrInfo);
			if (!result)
				result = fss_wake_parent( &fsspec );
			}
			
		if (!(**hData).got_one) {
			nshc_calls->NSH_puts("  Crea   Type  Name\r");
			nshc_calls->NSH_puts(" ------ ------ ----\r");
			(**hData).got_one = 1;
			}
				
		nshc_calls->NSH_printf( " '%.4s' '%.4s' ", &fndrInfo.fdCreator, &fndrInfo.fdType );
		nshc_calls->NSH_putStr( (StringPtr)fsspec.name );
		nshc_calls->NSH_putchar( '\r' );
		
		}
	
	if ( result )
		chattr_bad_file( nshc_parms, nshc_calls, (StringPtr)fsspec.name );
}

/* ======================================== */

// state machine - core routines

/* ======================================== */

void chattr_start( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls  )
{
	int			arg;
	OSType		os_type;
	chattr_hndl	hData;	// handle to hold our data
	
	if (nshc_parms->argc < 2) {
		nshc_calls->NSH_putStr_err( "\pUsage: chattr file [file...] [-t type] [-c crea]\r" );
		chattr_bad( nshc_parms, NSHC_ERR_PARMS );
		return;
		}
		
	nshc_parms->action = nsh_continue;

	hData = (chattr_hndl)NewHandleClear(sizeof(t_chattr_data));
	
	if (hData) {
	
		(**hData).arg = 1;					// start at the arg = 1 position
		(**hData).got_fss = fss_test();		// test if we can use FSSpec calls
		
		// test if we are doing a type change
		
		if (arg = nshc_got_option( nshc_parms, 't' ))
			if (chattr_get_OSType( nshc_parms, arg+1, &os_type)) {
				(**hData).change_type = arg;
				(**hData).new_type = os_type;
				}
			else {
				nshc_calls->NSH_putStr_err( "\pchattr: Bad OSType given for type.\r" );
				chattr_bad( nshc_parms, NSHC_ERR_PARMS );
				}
			
		// test if we are doing a creator change
		
		if (arg = nshc_got_option( nshc_parms, 'c' ))
			if (chattr_get_OSType( nshc_parms, arg+1, &os_type)) {
				(**hData).change_creator = arg;
				(**hData).new_creator = os_type;
				}
			else {
				nshc_calls->NSH_putStr_err( "\pchattr: Bad OSType given for creator.\r" );
				chattr_bad( nshc_parms, NSHC_ERR_PARMS );
				}
			
		nshc_parms->data = (Handle)hData;
		}
	else {
		nshc_calls->NSH_putStr_err( "\pchattr: Could not allocate storage.\r" );
		chattr_bad( nshc_parms, NSHC_ERR_MEMORY );
		}
}

/* ======================================== */

void chattr_continue( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls )
{
	int		i;
	int		arg;
	chattr_hndl	hData;
	
	if (hData = (chattr_hndl)nshc_parms->data) {
	
		arg = (**hData).arg;
		
		while ( ( arg == (**hData).change_type ) || ( arg == (**hData).change_creator ) ) {
			arg += 2;
			(**hData).arg = arg;
			}

		if (arg >= nshc_parms->argc)
			chattr_good( nshc_parms );
		else
			chattr_one( nshc_parms, nshc_calls, hData );

		}
}

/* ======================================== */

void chattr_stop( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls )
{
	chattr_hndl	hData;
	
	if (hData = (chattr_hndl)nshc_parms->data)
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
				chattr_start(nshc_parms, nshc_calls);
				break;
			case nsh_continue:
				chattr_continue(nshc_parms, nshc_calls);
				break;
			case nsh_stop:
				chattr_stop(nshc_parms, nshc_calls);
				break;
			}
		
		}
	
#ifdef __MWERKS__
	SetA4(oldA4);
#endif
}

/* ======================================== */
