/* ========== the commmand file: ==========

	rm.c
	
	Copyright (c) 1993,1994 Newport Software Development
	
	You may distribute unmodified copies of this file for
	noncommercial purposes.  You may use this file as a
	reference when writing your own nShell(tm) commands.
	
	All other rights are reserved.
	
   ========== the commmand file: ========== */

//
// This code can build to be "rm" or "rmdir".
//
// To build "rm", define RM below.
//
// To build "rmdir", define RMDIR below.
//

#define  RMDIR

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

} t_rm_data;

typedef	t_rm_data	**rm_hndl;

/* ======================================== */

// prototypes - utility

void rm_bad( t_nshc_parms *nshc_parms, int code );
void rm_bad_file( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls, StringPtr msg );
void rm_good( t_nshc_parms *nshc_parms );

// prototypes - file routines

OSErr rm_info(const FSSpec *spec, long *theDirID, int *isDir, int *isEmpty);
void  rm_one( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls, t_rm_data **hData );

// prototypes - state machine

void rm_start( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls  );
void rm_continue( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls );
void rm_stop( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls );

/* ======================================== */

// utility routines

/* ======================================== */

void rm_bad(  t_nshc_parms *nshc_parms, int code )
{
	nshc_parms->action = nsh_stop;
	nshc_parms->result = code;
}

void rm_bad_file(  t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls, StringPtr msg )
{

#if defined RM
	nshc_calls->NSH_putStr_err("\prm: File access error (");
#elif defined RMDIR
	nshc_calls->NSH_putStr_err("\prmdir: Directory access error (");
#endif

	nshc_calls->NSH_putStr_err(msg);
	nshc_calls->NSH_putStr_err("\p)\r");

	nshc_parms->action = nsh_stop;
	nshc_parms->result = NSHC_ERR_GENERAL;
}

void rm_good(  t_nshc_parms *nshc_parms )
{
	nshc_parms->action = nsh_stop;
	nshc_parms->result = 0;
}

/* ======================================== */

// file access routines

/* ========================================== */

OSErr rm_info(const FSSpec *spec, long *theDirID, int *isDir, int *isEmpty)
{
	CInfoPBRec pb;
	OSErr error;

	pb.hFileInfo.ioNamePtr = (StringPtr)spec->name;
	pb.hFileInfo.ioVRefNum = spec->vRefNum;
	pb.hFileInfo.ioDirID = spec->parID;
	pb.hFileInfo.ioFDirIndex = 0;
	
	error = PBGetCatInfoSync(&pb);
	
	*theDirID = pb.hFileInfo.ioDirID;
	*isDir = (pb.hFileInfo.ioFlAttrib & 0x10) != 0;
	*isEmpty = pb.dirInfo.ioDrNmFls == 0;
		
	return (error);
}

/* ======================================== */

void rm_one( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls, t_rm_data **hData )
{
	int		result;
	FSSpec	fsspec;
	long	dirID;
	int		isDir;
	int		isEmpty;
	
	// =====> convert argument to fsspec
	
	result = arg_to_real_fss( nshc_parms, nshc_calls, (**hData).arg, &fsspec );

	(**hData).arg++;
	
	if (result) {
		rm_bad( nshc_parms, result );
		return;
		}
	
	result = rm_info( &fsspec, &dirID, &isDir, &isEmpty );
	
#if defined RM

	if ( !result && isDir ) {
		nshc_calls->NSH_putStr_err("\prm: Skiping directory = ");
		nshc_calls->NSH_putStr_err((StringPtr)fsspec.name);
		nshc_calls->NSH_putchar('\r');
		return;
		}

#elif defined RMDIR

	if (!result) {
	
		if ( !isDir ) {
			nshc_calls->NSH_putStr_err("\prmdir: Skiping file = ");
			nshc_calls->NSH_putStr_err((StringPtr)fsspec.name);
			nshc_calls->NSH_putchar('\r');
			return;
			}
	
		if ( !isEmpty ) {
			nshc_calls->NSH_putStr_err("\prmdir: Directory is not empty = ");
			nshc_calls->NSH_putStr_err((StringPtr)fsspec.name);
			nshc_calls->NSH_putchar('\r');
			return;
			}
		
		}

#endif
			
	if ( result == fnfErr ) {

		#if defined RM
		nshc_calls->NSH_putStr_err("\prm: File not found = ");
		#elif defined RMDIR
		nshc_calls->NSH_putStr_err("\prmdir: Directory not found = ");
		#endif

		nshc_calls->NSH_putStr_err((StringPtr)fsspec.name);
		nshc_calls->NSH_putchar('\r');
		return;
		}
			
	if ( result ) {
		rm_bad_file( nshc_parms, nshc_calls, (StringPtr)fsspec.name );
		nshc_calls->NSH_printf( "error = %d\r", result );
		return;
		}
			
	// =====> delete the file or directory
		
	result = fss_Delete((**hData).got_fss, &fsspec);
	
	if ( result ) {
		rm_bad_file( nshc_parms, nshc_calls, (StringPtr)fsspec.name );
		return;
		}

	result = fss_wake_parent( &fsspec );

	if (result)
		nshc_calls->NSH_putStr_err( "\pmv: Warning - could update finder info.\r" );
}

/* ======================================== */

// state machine - core routines

/* ======================================== */

void rm_start( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls  )
{
	rm_hndl	hData;	// handle to hold our data
	
	if (nshc_parms->argc < 2) {
	
		#if defined RM
		nshc_calls->NSH_putStr_err( "\pUsage: rm file [file...]\r" );
		#elif defined RMDIR
		nshc_calls->NSH_putStr_err( "\pUsage: rmdir dir [dir...]\r" );
		#endif

		rm_bad( nshc_parms, NSHC_ERR_PARMS );
		return;
		}
		
	nshc_parms->action = nsh_continue;

	hData = (rm_hndl)NewHandleClear(sizeof(t_rm_data));
	
	if (hData) {
		(**hData).arg = 1;					// start at the arg = 1 position
		(**hData).got_fss = fss_test();		// test if we can use FSSpec calls
		nshc_parms->data = (Handle)hData;
		}
	else {
		nshc_calls->NSH_putStr_err( "\prm: Could not allocate storage.\r" );
		rm_bad( nshc_parms, NSHC_ERR_MEMORY );
		}
}

/* ======================================== */

void rm_continue( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls )
{
	int		i;
	rm_hndl	hData;
	
	if (hData = (rm_hndl)nshc_parms->data) {

		if ((**hData).arg >= nshc_parms->argc)
			rm_good( nshc_parms );
		else
			rm_one( nshc_parms, nshc_calls, hData );

		}
}

/* ======================================== */

void rm_stop( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls )
{
	rm_hndl	hData;
	
	if (hData = (rm_hndl)nshc_parms->data)
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
				rm_start(nshc_parms, nshc_calls);
				break;
			case nsh_continue:
				rm_continue(nshc_parms, nshc_calls);
				break;
			case nsh_stop:
				rm_stop(nshc_parms, nshc_calls);
				break;
			}
	
		}
	
#ifdef __MWERKS__
	SetA4(oldA4);
#endif
}

/* ======================================== */
