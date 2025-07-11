/* ========== the commmand file: ==========

	mkdir.c
	
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

} t_mkdir_data;

typedef	t_mkdir_data	**mkdir_hndl;

/* ======================================== */

// prototypes - utility

void mkdir_bad( t_nshc_parms *nshc_parms, int code );
void mkdir_bad_file( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls, StringPtr msg );
void mkdir_good( t_nshc_parms *nshc_parms );

// prototypes - file routines

OSErr mkdir(int got_fss, const FSSpec *spec);
OSErr mkdir_info(const FSSpec *spec, int *isDir);
void  mkdir_one( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls, t_mkdir_data **hData );

// prototypes - state machine

void mkdir_start( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls  );
void mkdir_continue( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls );
void mkdir_stop( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls );

/* ======================================== */

// utility routines

/* ======================================== */

void mkdir_bad(  t_nshc_parms *nshc_parms, int code )
{
	nshc_parms->action = nsh_stop;
	nshc_parms->result = code;
}

void mkdir_bad_file(  t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls, StringPtr msg )
{

	nshc_calls->NSH_putStr_err("\pmkdir: Directory access error (");
	nshc_calls->NSH_putStr_err(msg);
	nshc_calls->NSH_putStr_err("\p)\r");

	nshc_parms->action = nsh_stop;
	nshc_parms->result = NSHC_ERR_GENERAL;
}

void mkdir_good(  t_nshc_parms *nshc_parms )
{
	nshc_parms->action = nsh_stop;
	nshc_parms->result = 0;
}

/* ======================================== */

// file access routines

/* ========================================== */

OSErr mkdir_info(const FSSpec *spec, int *isDir)
{
	CInfoPBRec pb;
	OSErr error;

	pb.hFileInfo.ioNamePtr = (StringPtr)spec->name;
	pb.hFileInfo.ioVRefNum = spec->vRefNum;
	pb.hFileInfo.ioDirID = spec->parID;
	pb.hFileInfo.ioFDirIndex = 0;
	
	error = PBGetCatInfoSync(&pb);
	
	*isDir = (pb.hFileInfo.ioFlAttrib & 0x10) != 0;
		
	return (error);
}

/* ========================================== */

OSErr mkdir(int got_fss, const FSSpec *spec)
{
	OSErr			result;
	long			dirID;
	HParamBlockRec	pb;

	if (got_fss)
		return (FSpDirCreate(spec, smSystemScript, &dirID));
	else {
		pb.fileParam.ioVRefNum = spec->vRefNum;
		pb.fileParam.ioDirID = spec->parID;
		pb.fileParam.ioNamePtr = (StringPtr) &(spec->name);
		
		result = PBDirCreateSync(&pb);
		
		return (result);
		}
}
/* ======================================== */

void mkdir_one( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls, t_mkdir_data **hData )
{
	int		result;
	FSSpec	fsspec;
	long	dirID;
	int		isDir;
	int		isEmpty;
	
	// =====> convert argument to fsspec
	
	result = arg_to_fss( nshc_parms, nshc_calls, (**hData).arg, &fsspec );

	(**hData).arg++;
	
	if (result) {
		mkdir_bad( nshc_parms, result );
		return;
		}
	
	result = mkdir_info( &fsspec, &isDir );
				
	if (!result) {
	
		if ( isDir )
			nshc_calls->NSH_putStr_err("\pmkdir: Directory already exists = ");
		else
			nshc_calls->NSH_putStr_err("\pmkdir: File already exists = ");
			
		nshc_calls->NSH_putStr_err((StringPtr)fsspec.name);
		nshc_calls->NSH_putchar('\r');
		return;
		
		}
	
	if ( result != fnfErr ) {
		mkdir_bad_file( nshc_parms, nshc_calls, (StringPtr)fsspec.name );
		return;
		}
			
	// =====> make the new directory
		
	result = mkdir((**hData).got_fss, &fsspec);
	
	if ( result ) {
		mkdir_bad_file( nshc_parms, nshc_calls, (StringPtr)fsspec.name );
		return;
		}

	result = fss_wake_parent( &fsspec );

	if (result)
		nshc_calls->NSH_putStr_err( "\pmv: Warning - could update finder info.\r" );
}

/* ======================================== */

// state machine - core routines

/* ======================================== */

void mkdir_start( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls  )
{
	mkdir_hndl	hData;	// handle to hold our data
	
	if (nshc_parms->argc < 2) {
		nshc_calls->NSH_putStr_err( "\pUsage: mkdir dir [dir...]\r" );
		mkdir_bad( nshc_parms, NSHC_ERR_PARMS );
		return;
		}
		
	nshc_parms->action = nsh_continue;

	hData = (mkdir_hndl)NewHandleClear(sizeof(t_mkdir_data));
	
	if (hData) {
		(**hData).arg = 1;					// start at the arg = 1 position
		(**hData).got_fss = fss_test();		// test if we can use FSSpec calls
		nshc_parms->data = (Handle)hData;
		}
	else {
		nshc_calls->NSH_putStr_err( "\pmkdir: Could not allocate storage.\r" );
		mkdir_bad( nshc_parms, NSHC_ERR_MEMORY );
		}
}

/* ======================================== */

void mkdir_continue( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls )
{
	int		i;
	mkdir_hndl	hData;
	
	if (hData = (mkdir_hndl)nshc_parms->data) {

		if ((**hData).arg >= nshc_parms->argc)
			mkdir_good( nshc_parms );
		else
			mkdir_one( nshc_parms, nshc_calls, hData );

		}
}

/* ======================================== */

void mkdir_stop( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls )
{
	mkdir_hndl	hData;
	
	if (hData = (mkdir_hndl)nshc_parms->data)
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
				mkdir_start(nshc_parms, nshc_calls);
				break;
			case nsh_continue:
				mkdir_continue(nshc_parms, nshc_calls);
				break;
			case nsh_stop:
				mkdir_stop(nshc_parms, nshc_calls);
				break;
			}

		}

#ifdef __MWERKS__
	SetA4(oldA4);
#endif
}

/* ======================================== */
