/* ========== the commmand file: ==========

	mv.c
	
	Copyright (c) 1993,1994 Newport Software Development
	
	You may distribute unmodified copies of this file for
	noncommercial purposes.  You may use this file as a
	reference when writing your own nShell(tm) commands.
	
	All other rights are reserved.
	
   ========== the commmand file: ========== */

#ifdef __MWERKS__			// CodeWarrior requires an A4 setup
#include <A4Stuff.h>
#endif

#include "nshc.h"

#include "arg_utl.proto.h"
#include "fss_utl.proto.h"
#include "fss_utl2.proto.h"
#include "nshc_utl.proto.h"
#include "str_utl.proto.h"

// data definition - this struct is the root of all data

typedef struct {

	int		arg;			// position in arg list
	
	int		got_fss;
	
	Boolean	isDir;			// if target is a directory
	long	dirID;			// id of target if it is a directory

	FSSpec	toSpec;			// fsspec of output file
	
} t_mv_data;

typedef	t_mv_data	**CDataH;

/* ======================================== */

// prototypes - utility

void mv_bad( t_nshc_parms *nshc_parms, int code );
void mv_bad_file( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls, StringPtr msg );
void mv_good( t_nshc_parms *nshc_parms );

// prototypes - file routines

void  mv_make_name( Str63 name );
OSErr mv_clear_init(const FSSpec *spec);
OSErr mv_move(FSSpec *fromSpec, FSSpec *toSpec, t_nshc_calls *nshc_calls);
void  mv_setup( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls, t_mv_data **hData );
void  mv_task( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls, t_mv_data **hData );

// prototypes - state machine

void mv_start( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls  );
void mv_continue( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls );
void mv_stop( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls );

/* ======================================== */

// utility routines

/* ======================================== */

void mv_bad(  t_nshc_parms *nshc_parms, int code )
{
	nshc_parms->action = nsh_stop;
	nshc_parms->result = code;
}

void mv_bad_file(  t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls, StringPtr msg )
{
	nshc_calls->NSH_putStr_err("\pmv: File access error (");
	nshc_calls->NSH_putStr_err(msg);
	nshc_calls->NSH_putStr_err("\p)\r");

	nshc_parms->action = nsh_stop;
	nshc_parms->result = NSHC_ERR_GENERAL;
}

void mv_good(  t_nshc_parms *nshc_parms )
{
	nshc_parms->action = nsh_stop;
	nshc_parms->result = 0;
}

/* ======================================== */

// file access routines

/* ======================================== */

OSErr mv_clear_init(const FSSpec *spec)
{
	CInfoPBRec pb;
	OSErr result;

	pb.hFileInfo.ioNamePtr = (StringPtr)spec->name;
	pb.hFileInfo.ioVRefNum = spec->vRefNum;
	pb.hFileInfo.ioDirID = spec->parID;
	pb.hFileInfo.ioFDirIndex = 0;

	result = PBGetCatInfoSync(&pb);

	if (!result) {
		pb.hFileInfo.ioFlFndrInfo.fdFlags = pb.hFileInfo.ioFlFndrInfo.fdFlags & 0xfeff;
		pb.hFileInfo.ioDirID = spec->parID;
		result = PBSetCatInfoSync(&pb);	/* now, save the new information back to disk */
		}
		
	return (result);
}

/* ======================================== */

void mv_setup( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls, t_mv_data **hData )
{
	int		argc;
	int		result;
	char	*toStr;
	
	(**hData).arg = 1;					// start at the arg = 1 position
	(**hData).got_fss = fss_test();		// remember if fsspecs are available

	// strip off target path and process it

	argc = --nshc_parms->argc;
	
	toStr = &nshc_parms->arg_buf[ nshc_parms->argv[argc] ];
	
	if ( cStrEqual( toStr, "dev:tty" ) ) {
		nshc_calls->NSH_putStr_err( "\pmv: \"dev:tty\" is not supported.\r" );
		mv_bad( nshc_parms, NSHC_ERR_PARMS );
		return;
		}

	if ( cStrEqual( toStr, "dev:null" ) ) {
		nshc_calls->NSH_putStr_err( "\pmv: \"dev:null\" is not supported.\r" );
		mv_bad( nshc_parms, NSHC_ERR_PARMS );
		return;
		}
		
	result = arg_to_fss( nshc_parms, nshc_calls, argc, &(**hData).toSpec );
	
	if (result)
		mv_bad( nshc_parms, result );
	else{
		result = fss_to_DirID( &(**hData).toSpec, &(**hData).dirID, &(**hData).isDir );
		if (argc > 2) {
			if ( (result != noErr) || !(**hData).isDir ) {
				nshc_calls->NSH_putStr_err( "\pmv: Destination must be a directory when more than one file is moved.\r" );
				mv_bad( nshc_parms, NSHC_ERR_PARMS );
				}
			}
		else {
			if ( (result == noErr) && !(**hData).isDir ) {
				nshc_calls->NSH_putStr_err("\pmv: File already exists (");
				nshc_calls->NSH_putStr_err((StringPtr)(**hData).toSpec.name);
				nshc_calls->NSH_putStr_err("\p)\r");
				mv_bad( nshc_parms, NSHC_ERR_PARMS );
				}
			}
		}
}

/* ========================================== */

void mv_make_name( Str63 name )
{
	char	temp[20];
	int		i,j;
	long	ticks;
	
	i = 0;
	ticks = TickCount();
	
	while (ticks) {
		temp[i++] = '0' + ticks % 10;
		ticks = ticks / 10;
		}
	
	name[0] = 4 + i;
	name[1] = 't';
	name[2] = 'm';
	name[3] = 'p';
	name[4] = '.';
	
	j = 5;
	while (i--)
		name[j++] = temp[i];
}
	
OSErr mv_move(FSSpec *fromSpec, FSSpec *toSpec, t_nshc_calls *nshc_calls)
{
	CMovePBRec		pb;
	HParamBlockRec	hpb;
	OSErr			result;
	int				retries;
	Str63			temp_name;
	Str63			save_name;
	
	if ( fromSpec->parID != toSpec->parID ) {
	
		retries = 0;
		result = 0;
	
		do {
		
			if (retries) {
				if (retries == 1)
					pStrCopy( save_name, (StringPtr) &(fromSpec->name) );
				mv_make_name( temp_name );
				hpb.fileParam.ioDirID = fromSpec->parID;
				hpb.ioParam.ioMisc = (Ptr)temp_name;
				hpb.ioParam.ioNamePtr = (StringPtr) &(fromSpec->name);
				hpb.ioParam.ioVersNum = 0;
				hpb.ioParam.ioVRefNum = fromSpec->vRefNum;
				result = PBHRenameSync(&hpb);
				if ( !result )
					pStrCopy( (StringPtr) &(fromSpec->name), temp_name );
				}

			if ( !result ) {
				pb.ioDirID = fromSpec->parID;
				pb.ioNamePtr = (StringPtr) &(fromSpec->name);
				pb.ioNewDirID = toSpec->parID;
				pb.ioNewName = nil;
				pb.ioVRefNum = fromSpec->vRefNum;
				result = PBCatMoveSync(&pb);
				}
			
			retries++;
	
		} while (( retries < 11) && (result == dupFNErr));
		
		if ( result ) {
			if ( !pStrEqual( (StringPtr)&(fromSpec->name), save_name ) ) {
				hpb.fileParam.ioDirID = fromSpec->parID;
				hpb.ioParam.ioMisc = (Ptr)save_name;
				hpb.ioParam.ioNamePtr = (StringPtr) &(fromSpec->name);
				hpb.ioParam.ioVersNum = 0;
				hpb.ioParam.ioVRefNum = fromSpec->vRefNum;
				PBHRenameSync(&hpb);
				}
			return( result );
			}
			
		}
	
	if ( pStrEqual( (StringPtr) &(fromSpec->name), (StringPtr) &(toSpec->name) ) )
		return( noErr );
		
	hpb.fileParam.ioDirID = toSpec->parID;
	hpb.ioParam.ioMisc = (Ptr)&(toSpec->name);
	hpb.ioParam.ioNamePtr = (StringPtr) &(fromSpec->name);
	hpb.ioParam.ioVersNum = 0;
	hpb.ioParam.ioVRefNum = toSpec->vRefNum;
	
	return( PBHRenameSync(&hpb) );
}

/* ======================================== */

void mv_task( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls, t_mv_data **hData )
{
	int		result;
	long	dirID;
	Boolean	isDir;
	FInfo	fndrInfo;
	FSSpec	fromSpec;
	
	// =====> convert "from" path to fsspec
	
	result = arg_to_fss( nshc_parms, nshc_calls, (**hData).arg, &fromSpec );
	
	(**hData).arg++;
	
	if (result) {
		mv_bad( nshc_parms, result );
		return;
		}
	
	if (fromSpec.vRefNum != (**hData).toSpec.vRefNum) {
		nshc_calls->NSH_putStr_err("\pmv: File must be on destination volume (");
		nshc_calls->NSH_putStr_err((StringPtr)fromSpec.name);
		nshc_calls->NSH_putStr_err("\p)\r");
		mv_bad( nshc_parms, NSHC_ERR_GENERAL );
		return;
		}
		
	result = fss_to_DirID( &fromSpec, &dirID, &isDir );
		
	if ( result == fnfErr ) {
		nshc_calls->NSH_putStr_err("\pmv: File not found (");
		nshc_calls->NSH_putStr_err((StringPtr)fromSpec.name);
		nshc_calls->NSH_putStr_err("\p)\r");
		mv_bad( nshc_parms, NSHC_ERR_GENERAL );
		return;
		}
			
	if ( result ) {
		mv_bad_file( nshc_parms, nshc_calls, (StringPtr)fromSpec.name );
		return;
		}
			
	// =====> if the target is a dir, construct and test full target path
		
	if ((**hData).isDir) {
	
		(**hData).toSpec.parID = (**hData).dirID;
		pStrCopy( (**hData).toSpec.name, fromSpec.name );
		
		if ( !fss_to_DirID( &(**hData).toSpec, &dirID, &isDir ) ) {
			if (isDir)
				nshc_calls->NSH_putStr_err("\pmv: Folder already exists (");
			else
				nshc_calls->NSH_putStr_err("\pmv: File already exists (");
			nshc_calls->NSH_putStr_err((StringPtr)(**hData).toSpec.name);
			nshc_calls->NSH_putStr_err("\p)\r");
			mv_bad( nshc_parms, NSHC_ERR_GENERAL );
			return;
			}
		}
		
	// =====> move it
	
	HLock( (Handle)hData );
	result =  mv_move( &fromSpec, &(**hData).toSpec, nshc_calls );
	HUnlock( (Handle)hData );
	
	if (result) {
		nshc_calls->NSH_putStr_err( "\pmv: Could not move file (");
		nshc_calls->NSH_putStr_err((StringPtr)(**hData).toSpec.name);
		nshc_calls->NSH_putStr_err("\p)\r");
		mv_bad( nshc_parms, NSHC_ERR_PARMS );
		return;
		}
		
	// =====> and tell the findir to init it
	
	result =  mv_clear_init( &(**hData).toSpec );

	if (result)
		nshc_calls->NSH_putStr_err( "\pmv: Warning - could not position icon.\r" );
		
	result = fss_wake_parent( &(**hData).toSpec );

	if (result)
		nshc_calls->NSH_putStr_err( "\pmv: Warning - could update finder info.\r" );
}

/* ======================================== */

// state machine - core routines

/* ======================================== */

void mv_start( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls  )
{
	CDataH	hData;	// handle to hold our data
	
	if (nshc_parms->argc < 3) {
		nshc_calls->NSH_putStr_err( "\pUsage: see \"man mv\" for options.\r" );
		mv_bad( nshc_parms, NSHC_ERR_PARMS );
		return;
		}
		
	nshc_parms->action = nsh_continue;

	hData = (CDataH)NewHandleClear(sizeof(t_mv_data));
	
	if (hData) {
		HLock( (Handle)hData );
		mv_setup( nshc_parms, nshc_calls, hData );
		HUnlock( (Handle)hData );
		nshc_parms->data = (Handle)hData;
		}
	else {
		nshc_calls->NSH_putStr_err( "\pmv: Could not allocate storage.\r" );
		mv_bad( nshc_parms, NSHC_ERR_MEMORY );
		}
}

/* ======================================== */

void mv_continue( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls )
{
	CDataH	hData;
	
	if (hData = (CDataH)nshc_parms->data) {
	
		if ((**hData).arg >= nshc_parms->argc)
			mv_good( nshc_parms );
		else
			mv_task( nshc_parms, nshc_calls, hData );
			
		}
	else
		nshc_parms->action = nsh_idle;
}

/* ======================================== */

void mv_stop( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls )
{
	if ( nshc_parms->data )		
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
				mv_start(nshc_parms, nshc_calls);
				break;
			case nsh_continue:
				mv_continue(nshc_parms, nshc_calls);
				break;
			case nsh_stop:
				mv_stop(nshc_parms, nshc_calls);
				break;
			}
			
		}
			
#ifdef __MWERKS__
	SetA4(oldA4);
#endif
}

/* ======================================== */
