/* ========== the commmand file: ==========

	fss.c
	
	Copyright (c) 1994 Newport Software Development
	
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
#include "fss_utl2.proto.h"
#include "nshc_utl.proto.h"
#include "str_utl.proto.h"

// data definition - this struct is the root of all data

typedef struct {

	int		got_fss;		// 0 if FSSpec calls are not available
	int		arg;			// position in arg list

} t_fss_data;

typedef	t_fss_data	**fss_hndl;

/* ======================================== */

// prototypes - utility

void fss_bad( t_nshc_parms *nshc_parms, int code );
void fss_bad_file( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls, StringPtr msg );
void fss_good( t_nshc_parms *nshc_parms );

// prototypes - file routines

void  fss_one( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls, t_fss_data **hData );

// prototypes - state machine

void fss_start( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls  );
void fss_continue( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls );
void fss_stop( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls );

/* ======================================== */

// utility routines

/* ======================================== */

void fss_bad(  t_nshc_parms *nshc_parms, int code )
{
	nshc_parms->action = nsh_stop;
	nshc_parms->result = code;
}

void fss_bad_file(  t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls, StringPtr msg )
{

	nshc_calls->NSH_putStr_err("\pfss: Access error (");
	nshc_calls->NSH_putStr_err(msg);
	nshc_calls->NSH_putStr_err("\p)\r");
}

void fss_good(  t_nshc_parms *nshc_parms )
{
	nshc_parms->action = nsh_stop;
	nshc_parms->result = 0;
}

/* ======================================== */

// file access routines

/* ======================================== */

void fss_one( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls, t_fss_data **hData )
{
	CInfoPBRec pb;
	OSErr result;
	FSSpec	fsspec;
		
	// =====> convert argument to fsspec
	
	result = arg_to_real_fss( nshc_parms, nshc_calls, (**hData).arg, &fsspec );

	(**hData).arg++;
	
	if (result)
		return;
	
	pb.hFileInfo.ioNamePtr = (StringPtr)&fsspec.name;
	pb.hFileInfo.ioVRefNum = fsspec.vRefNum;
	pb.hFileInfo.ioDirID = fsspec.parID;
	pb.hFileInfo.ioFDirIndex = 0;

	result = PBGetCatInfoSync(&pb);
	
	if (result)
		fss_bad_file( nshc_parms, nshc_calls, (StringPtr)fsspec.name );
	else {
		nshc_calls->NSH_printf( " %6d %6ld ", fsspec.vRefNum, fsspec.parID );
		nshc_calls->NSH_putStr( fsspec.name );
		nshc_calls->NSH_putchar( '\r' );
		}	
}

/* ======================================== */

// state machine - core routines

/* ======================================== */

void fss_start( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls  )
{
	fss_hndl	hData;	// handle to hold our data
	
	if (nshc_parms->argc < 2) {
		nshc_calls->NSH_putStr_err( "\pUsage: fss pathname [pathname...]\r" );
		fss_bad( nshc_parms, NSHC_ERR_PARMS );
		return;
		}
		
	nshc_parms->action = nsh_continue;

	hData = (fss_hndl)NewHandleClear(sizeof(t_fss_data));
	
	if (hData) {
		(**hData).arg = 1;					// start at the arg = 1 position
		(**hData).got_fss = fss_test();		// test if we can use FSSpec calls
		nshc_parms->data = (Handle)hData;
		}
	else {
		nshc_calls->NSH_putStr_err( "\pfss: Could not allocate storage.\r" );
		fss_bad( nshc_parms, NSHC_ERR_MEMORY );
		}
}

/* ======================================== */

void fss_continue( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls )
{
	fss_hndl	hData;
	
	if (hData = (fss_hndl)nshc_parms->data)
		if ((**hData).arg >= nshc_parms->argc)
			fss_good( nshc_parms );
		else
			fss_one( nshc_parms, nshc_calls, hData );
}

/* ======================================== */

void fss_stop( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls )
{
	fss_hndl	hData;
	
	if (hData = (fss_hndl)nshc_parms->data)
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
				fss_start(nshc_parms, nshc_calls);
				break;
			case nsh_continue:
				fss_continue(nshc_parms, nshc_calls);
				break;
			case nsh_stop:
				fss_stop(nshc_parms, nshc_calls);
				break;
			}
			
		}
	
#ifdef __MWERKS__
	SetA4(oldA4);
#endif
}

/* ======================================== */
