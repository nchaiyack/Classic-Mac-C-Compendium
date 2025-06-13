/* ========== the commmand file: ==========

	cp.c
	
	Copyright (c) 1993,1994 Newport Software Development
	
	You may distribute unmodified copies of this file for
	noncommercial purposes.  You may use this file as a
	reference when writing your own nShell(tm) commands.
	
	All other rights are reserved.
	
   ========== the commmand file: ========== */

#ifdef __MWERKS__			// CodeWarrior requires an A4 setup
#include <A4Stuff.h>
#endif

#define	CP_BUF_SIZE	2048

#include <script.h>
#include <string.h>

#include "nshc.h"

#include "arg_utl.proto.h"
#include "fss_utl.proto.h"
#include "fss_utl2.proto.h"
#include "nshc_utl.proto.h"
#include "str_utl.proto.h"

// data definition - this struct is the root of all data

typedef enum { copy_none, copy_data, copy_rsrc, copy_close } c_state;

typedef struct {

	int		got_fss;		// 0 if FSSpec calls are not available

	int		arg;			// position in arg list
	
	c_state	copy_state;		// in data or resource portion
	
	Boolean	isDir;			// if target is a directory
	long	dirID;			// id of target if it is a directory

	FSSpec	toSpec;			// fsspec of output file
	FSSpec	fromSpec;		// fsspec of input file
	
	short	toData;			// file ref. number, 0 if no file open
	short	toRsrc;			// file ref. number, 0 if no file open
	short	fromData;		// file ref. number, 0 if no file open
	short	fromRsrc;		// file ref. number, 0 if no file open

} t_cp_data;

typedef	t_cp_data	**CDataH;

/* ======================================== */

// prototypes - utility

void cp_bad( t_nshc_parms *nshc_parms, int code );
void cp_bad_file( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls, StringPtr msg );
void cp_good( t_nshc_parms *nshc_parms );

// prototypes - file copy routines

void cp_setup( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls, t_cp_data **hData );
void cp_open( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls, t_cp_data **hData );
void cp_read_write( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls, t_cp_data **hData );

// prototypes - state machine

void cp_start( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls  );
void cp_continue( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls );
void cp_stop( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls );

/* ======================================== */

// utility routines

/* ======================================== */

void cp_bad(  t_nshc_parms *nshc_parms, int code )
{
	nshc_parms->action = nsh_stop;
	nshc_parms->result = code;
}

void cp_bad_file(  t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls, StringPtr msg )
{
	nshc_calls->NSH_putStr_err("\pcp: File access error (");
	nshc_calls->NSH_putStr_err(msg);
	nshc_calls->NSH_putStr_err("\p)\r");

	nshc_parms->action = nsh_stop;
	nshc_parms->result = NSHC_ERR_GENERAL;
}

void cp_good(  t_nshc_parms *nshc_parms )
{
	nshc_parms->action = nsh_stop;
	nshc_parms->result = 0;
}

/* ======================================== */

// file access routines

/* ======================================== */

void cp_setup( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls, t_cp_data **hData )
{
	int		argc;
	int		result;
	char	*toStr;
	
	// strip off target path and process it

	argc = --nshc_parms->argc;
	
	toStr = &nshc_parms->arg_buf[ nshc_parms->argv[argc] ];
	
	if ( cStrEqual( toStr, "dev:tty" ) ) {
		nshc_calls->NSH_putStr_err( "\pcp: \"dev:tty\" is not supported.\r" );
		cp_bad( nshc_parms, NSHC_ERR_PARMS );
		return;
		}

	if ( cStrEqual( toStr, "dev:null" ) ) {
		cp_good( nshc_parms );
		return;
		}
		
	result = arg_to_fss( nshc_parms, nshc_calls, argc, &(**hData).toSpec );
	
	if (result)
		cp_bad( nshc_parms, result );
	else{
		result = fss_to_DirID( &(**hData).toSpec, &(**hData).dirID, &(**hData).isDir );
		if (argc > 2)
			if ( (result != noErr) || !(**hData).isDir ) {
				nshc_calls->NSH_putStr_err( "\pcp: Destination must be a directory when more than one file is copied.\r" );
				cp_bad( nshc_parms, NSHC_ERR_PARMS );
				}
		}
}

/* ======================================== */

void cp_open( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls, t_cp_data **hData )
{
	int		result;
	long	dirID;
	Boolean	isDir;
	FInfo	fndrInfo;
	long	newEOF;
	long	oldEOF;
	
	// =====> convert "from" path to fsspec
	
	result = arg_to_fss( nshc_parms, nshc_calls, (**hData).arg, &(**hData).fromSpec );

	(**hData).arg++;
	
	if (result) {
		cp_bad( nshc_parms, result );
		return;
		}
	
	result = fss_to_DirID( &(**hData).fromSpec, &dirID, &isDir );
		
	if (( result == noErr) && isDir) {
		nshc_calls->NSH_putStr_err("\pcp: Skiping directory = ");
		nshc_calls->NSH_putStr_err((StringPtr)(**hData).fromSpec.name);
		nshc_calls->NSH_putchar('\r');
		return;
		}
			
	if ( result == fnfErr ) {
		nshc_calls->NSH_putStr_err("\pcp: File not found = ");
		nshc_calls->NSH_putStr_err((StringPtr)(**hData).fromSpec.name);
		nshc_calls->NSH_putchar('\r');
		return;
		}
			
	if ( result ) {
		cp_bad_file( nshc_parms, nshc_calls, (StringPtr)(**hData).fromSpec.name );
		return;
		}
			
	// =====> if "to" is a real file, create it and write finfo
		
	result = fss_GetFInfo((**hData).got_fss, &(**hData).fromSpec, &fndrInfo);
	
	if ( result ) {
		cp_bad_file( nshc_parms, nshc_calls, (StringPtr)(**hData).fromSpec.name );
		return;
		}

	if ((**hData).isDir) {
		(**hData).toSpec.parID = (**hData).dirID;
		pStrCopy( (**hData).toSpec.name, (**hData).fromSpec.name );
		}
		
	result =  fss_Delete((**hData).got_fss, &(**hData).toSpec);
	
	if ( ( result != noErr ) && ( result != fnfErr ) ) {
		cp_bad_file( nshc_parms, nshc_calls, (StringPtr)(**hData).toSpec.name );
		return;
		}

	result = fss_CreateResFile((**hData).got_fss, &(**hData).toSpec, fndrInfo.fdCreator, fndrInfo.fdType, smSystemScript);

	if ( result ) {
		cp_bad_file( nshc_parms, nshc_calls, (StringPtr)(**hData).toSpec.name );
		return;
		}

	// =====> open all the file devices
		
	result = fss_OpenDF((**hData).got_fss, &(**hData).fromSpec, fsRdPerm, &(**hData).fromData);
	
	if (!result)
		result = fss_OpenRF((**hData).got_fss, &(**hData).fromSpec, fsRdPerm, &(**hData).fromRsrc);

	if (result) {
		cp_bad_file( nshc_parms, nshc_calls, (StringPtr)(**hData).fromSpec.name );
		return;
		}
		
	result = fss_OpenDF((**hData).got_fss, &(**hData).toSpec, fsRdWrShPerm, &(**hData).toData);

	if (!result)
		result = fss_OpenRF((**hData).got_fss, &(**hData).toSpec, fsRdWrShPerm, &(**hData).toRsrc);

	if (result) {
		cp_bad_file( nshc_parms, nshc_calls, (StringPtr)(**hData).toSpec.name );
		return;
		}
			
	// =====> set the eofs to make sure there is disk space
	// =====> if there is an error, restore the old eofs
	
	if (!result) {		// data fork
	
		result = GetEOF((**hData).toData, &oldEOF);
		
		if (!result)
			result = GetEOF((**hData).fromData, &newEOF);
		
		if (!result)
			result = SetEOF((**hData).toData, newEOF);
	
		if (result)
			SetEOF((**hData).toData, oldEOF);
		}

	if (!result) {		// rsrc fork
	
		result = GetEOF((**hData).toRsrc, &oldEOF);
		
		if (!result)
			result = GetEOF((**hData).fromRsrc, &newEOF);
		
		if (!result)
			result = SetEOF((**hData).toRsrc, newEOF);
	
		if (result)
			SetEOF((**hData).toRsrc, oldEOF);
		}
		
	if (result) {
		cp_bad_file( nshc_parms, nshc_calls, "\pend of file" );
		return;
		}
		
	(**hData).copy_state = copy_data;
}

/* ======================================== */

void cp_read_write( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls, t_cp_data **hData )
{
	int		close;
	int		temp;
	int		result;
	long	bcount;
	char	buf[CP_BUF_SIZE+1];
	
	result = close = 0;
	
	// =====> copy the data fork
	
	if ( (**hData).copy_state == copy_data ) {
	
		bcount = CP_BUF_SIZE;
		result = FSRead( (**hData).fromData, &bcount, &buf );
		
		if (( result == noErr ) || ( result == eofErr )) {
		
			if (result == eofErr) {
				(**hData).copy_state = copy_rsrc;
				result = 0;
				}
			
			if (bcount && !result) {
				result = FSWrite( (**hData).toData, &bcount, &buf );
				if (result)
					cp_bad_file( nshc_parms, nshc_calls, "\pwrite data" );
				}
			
			}
		else
			cp_bad_file( nshc_parms, nshc_calls, "\pread data" );
		}
	
	// =====> copy the resource fork
	
	if ( (**hData).copy_state == copy_rsrc ) {
	
		bcount = CP_BUF_SIZE;
		result = FSRead( (**hData).fromRsrc, &bcount, &buf );
		
		if (( result == noErr ) || ( result == eofErr )) {
		
			if (result == eofErr) {
				(**hData).copy_state = copy_close;
				result = 0;
				}
			
			if (bcount && !result) {
				result = FSWrite( (**hData).toRsrc, &bcount, &buf );
				if (result)
					cp_bad_file( nshc_parms, nshc_calls, "\pwrite resource" );
				}
			
			}
		else
			cp_bad_file( nshc_parms, nshc_calls, "\pread resource" );

		}
	
	// =====> close the input file
	
	if ( (**hData).copy_state == copy_close ) {

		if ( (**hData).fromData ) {
			if ( temp = FSClose( (**hData).fromData ) )
				result = temp;
			(**hData).fromData = 0;
			}
	
		if ( (**hData).fromRsrc ) {
			if ( temp = FSClose( (**hData).fromRsrc ) )
				result = temp;
			(**hData).fromRsrc = 0;
			}
	
		if ( (**hData).toData ) {
			if ( temp = FSClose( (**hData).toData ) )
				result = temp;
			(**hData).toData = 0;
			}
	
		if ( (**hData).toRsrc ) {
			if ( temp = FSClose( (**hData).toRsrc ) )
				result = temp;
			(**hData).toRsrc = 0;
			}
		
		(**hData).copy_state = copy_none;
		
		if (result)
			cp_bad_file( nshc_parms, nshc_calls, "\pclose file" );
		
		}
}

/* ======================================== */

// state machine - core routines

/* ======================================== */

void cp_start( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls  )
{
	CDataH	hData;	// handle to hold our data
	
	if (nshc_parms->argc < 3) {
		nshc_calls->NSH_putStr_err( "\pUsage: see \"man cp\" for options.\r" );
		cp_bad( nshc_parms, NSHC_ERR_PARMS );
		return;
		}
		
	nshc_parms->action = nsh_continue;

	hData = (CDataH)NewHandleClear(sizeof(t_cp_data));
	
	if (hData) {
		(**hData).arg = 1;					// start at the arg = 1 position
		(**hData).got_fss = fss_test();		// test if we can use FSSpec calls
		nshc_parms->data = (Handle)hData;
		HLock( (Handle)hData );
		cp_setup( nshc_parms, nshc_calls, hData );
		HUnlock( (Handle)hData );
		}
	else {
		nshc_calls->NSH_putStr_err( "\pcp: Could not allocate storage.\r" );
		cp_bad( nshc_parms, NSHC_ERR_MEMORY );
		}
}

/* ======================================== */

void cp_continue( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls )
{
	CDataH	hData;
	
	if (hData = (CDataH)nshc_parms->data) {

		if ((**hData).copy_state == copy_none) {
		
			if ((**hData).arg >= nshc_parms->argc) {
				cp_good( nshc_parms );
				return;
				}
	
			HLock( (Handle)hData );
			cp_open( nshc_parms, nshc_calls, hData );
			HUnlock( (Handle)hData );
			}
		
		if ((**hData).copy_state != copy_none) {
		
			HLock( (Handle)hData );
			cp_read_write( nshc_parms, nshc_calls, hData );
			HUnlock( (Handle)hData );
			
			}
		}
}

/* ======================================== */

void cp_stop( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls )
{
	short	fRef;
	OSErr	error;
	CDataH	hData;
	int		temp;
	int		result;
	
	result = 0;
	
	if (hData = (CDataH)nshc_parms->data) {
	
		if ( (**hData).fromData ) {
			if ( temp = FSClose( (**hData).fromData ) )
				result = temp;
			(**hData).fromData = 0;
			}
	
		if ( (**hData).fromRsrc ) {
			if ( temp = FSClose( (**hData).fromRsrc ) )
				result = temp;
			(**hData).fromRsrc = 0;
			}
	
		if ( (**hData).toData ) {
			if ( temp = FSClose( (**hData).toData ) )
				result = temp;
			(**hData).toData = 0;
			}
	
		if ( (**hData).toRsrc ) {
			if ( temp = FSClose( (**hData).toRsrc ) )
				result = temp;
			(**hData).toRsrc = 0;
			}
		
		DisposeHandle(nshc_parms->data);
		}
		
	if (result)
		cp_bad_file( nshc_parms, nshc_calls, "\pclosing files" );
		
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
				cp_start(nshc_parms, nshc_calls);
				break;
			case nsh_continue:
				cp_continue(nshc_parms, nshc_calls);
				break;
			case nsh_stop:
				cp_stop(nshc_parms, nshc_calls);
				break;
			}
			
		}
	
#ifdef __MWERKS__
	SetA4(oldA4);
#endif
}

/* ======================================== */
