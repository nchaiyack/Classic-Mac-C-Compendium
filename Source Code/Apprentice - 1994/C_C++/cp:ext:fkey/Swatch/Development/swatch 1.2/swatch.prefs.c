/**

	swatch.prefs.c
	Copyright (c) 1990, Adobe Systems, Inc.

 **/


/**-----------------------------------------------------------------------------
 **
 **	Headers
 **
 **/

#include <Folders.h>
#include "swatch.h"
#include "swatch.prefs.h"


/**-----------------------------------------------------------------------------
 **
 ** Private Variables
 **
 **/

char prefs_file_name[] = "\pSwatch Prefs";
char prefs_temp_file_name[] = "\pSwatch |____temp";
char riff_file_name[] = "\pSwatch Riff";

int16 prefs_VRefNum;
int32 prefs_DirID;


/**-----------------------------------------------------------------------------
 **
 ** Private Functions
 **
 **/

OSErr get_prefs_directory( void );

pascal OSErr FindFolder(short vRefNum,OSType folderType,Boolean createFolder,
    short *foundVRefNum,long *foundDirID)
    = {0x7000,0xA823}; 


/*******************************************************************************
 **
 **	Public Variables
 **
 **/

Preferences_t Swatch_prefs;


/*******************************************************************************
 **
 **	Public Functions
 **
 **/


/*******************************************************************************
 ***
 *** prototype
 ***
 *** summary
 ***
 *** History:
 ***
 *** To Do:
 ***
 ***/

OSErr get_prefs_directory( void )
{
	if ( Pre_system_7 ) {
		prefs_VRefNum = This_mac.sysVRefNum;
		prefs_DirID = 0;
		return noErr;
	}
	else
		return FindFolder( kOnSystemDisk, kPreferencesFolderType, kCreateFolder,
				&prefs_VRefNum, &prefs_DirID );
}


void Read_prefs( void )
{
	HParamBlockRec read_PB;
	Rect r;

	read_PB.ioParam.ioRefNum = 0;

	if ( get_prefs_directory() )
		goto use_defaults;

	read_PB.ioParam.ioNamePtr = (StringPtr) prefs_file_name;
	read_PB.ioParam.ioVRefNum = prefs_VRefNum;
	read_PB.fileParam.ioDirID = prefs_DirID;
	read_PB.ioParam.ioVersNum = 0;
	read_PB.ioParam.ioPermssn = 0;
	read_PB.ioParam.ioMisc = NULL;
	if ( PBHOpen( &read_PB, FALSE ) )
		goto use_defaults;

	read_PB.ioParam.ioBuffer = (Ptr) &Swatch_prefs;
	read_PB.ioParam.ioReqCount = sizeof( Preferences_t );
	read_PB.ioParam.ioPosMode = fsFromStart;
	read_PB.ioParam.ioPosOffset = 0;
	if ( PBRead( (ParamBlockRec *) &read_PB, FALSE ) )
		goto use_defaults;

	if ( Swatch_prefs.version != PREFS_VERSION )
		goto use_defaults;

	PBClose( (ParamBlockRec *) &read_PB, FALSE );
	read_PB.ioParam.ioRefNum = 0;

	r = Swatch_prefs.window_rect;
	r.top -= 18;
	r.bottom = r.top + 18;
	InsetRect( &r, 3, 3 );
	if ( !RectInRgn( &r, *(RgnHandle *) GrayRgn ) )
		goto use_default_window_rect;

	return;

use_defaults:
	if ( read_PB.ioParam.ioRefNum )
		PBClose( (ParamBlockRec *) &read_PB, FALSE );
	Swatch_prefs.version = PREFS_VERSION;
	Swatch_prefs.heap_scale = DEFAULT_HEAP_SCALE;
	Swatch_prefs.heap_scale_2n = DEFAULT_HEAP_SCALE_2N;
use_default_window_rect:
	SetRect( &Swatch_prefs.window_rect, DEFAULT_WINDOW_LEFT, DEFAULT_WINDOW_TOP,
			DEFAULT_WINDOW_RIGHT, DEFAULT_WINDOW_BOTTOM);
}


void Write_prefs( void )
{
	CInfoPBRec orig_cat_info_PB, new_cat_info_PB;
	HParamBlockRec write_PB;
	register Boolean previous_exists;

	write_PB.ioParam.ioRefNum = 0;

	if ( get_prefs_directory() )
		goto cant_save;

	orig_cat_info_PB.hFileInfo.ioNamePtr = (StringPtr) prefs_file_name;
	orig_cat_info_PB.hFileInfo.ioVRefNum = prefs_VRefNum;
	orig_cat_info_PB.hFileInfo.ioDirID = prefs_DirID;
	orig_cat_info_PB.hFileInfo.ioFDirIndex = 0;
	if ( PBGetCatInfo( &orig_cat_info_PB, FALSE ) )
		previous_exists = FALSE;
	else
		previous_exists = TRUE;

	write_PB.fileParam.ioNamePtr =
			previous_exists ? (StringPtr) prefs_temp_file_name : (StringPtr) prefs_file_name;
	write_PB.fileParam.ioVRefNum = prefs_VRefNum;
	write_PB.fileParam.ioDirID = prefs_DirID;
	write_PB.fileParam.ioFVersNum = 0;
	PBHCreate( &write_PB, FALSE );

	write_PB.ioParam.ioVersNum = 0;
	write_PB.ioParam.ioPermssn = 0;
	write_PB.ioParam.ioMisc = NULL;
	if ( PBHOpen( &write_PB, FALSE ) )
		goto cant_save;

	write_PB.ioParam.ioBuffer = (Ptr) &Swatch_prefs;
	write_PB.ioParam.ioReqCount = sizeof( Preferences_t );
	write_PB.ioParam.ioPosMode = fsFromStart;
	write_PB.ioParam.ioPosOffset = 0;
	if ( PBWrite( (ParamBlockRec *) &write_PB, FALSE ) )
		goto cant_save;

	PBClose( (ParamBlockRec *) &write_PB, FALSE );
	write_PB.ioParam.ioRefNum = 0;

	if ( previous_exists ) {
		write_PB.fileParam.ioNamePtr = (StringPtr) prefs_file_name;
		write_PB.fileParam.ioDirID = prefs_DirID;
		PBHDelete( &write_PB, FALSE );

		write_PB.fileParam.ioNamePtr = (StringPtr) prefs_temp_file_name;
		write_PB.ioParam.ioMisc = (Ptr) prefs_file_name;
		PBHRename( &write_PB, FALSE );
	}

	new_cat_info_PB.hFileInfo.ioNamePtr = (StringPtr) prefs_file_name;
	new_cat_info_PB.hFileInfo.ioVRefNum = prefs_VRefNum;
	new_cat_info_PB.hFileInfo.ioDirID = prefs_DirID;
	new_cat_info_PB.hFileInfo.ioFDirIndex = 0;
	PBGetCatInfo( &new_cat_info_PB, FALSE );

	if ( previous_exists ) {
		new_cat_info_PB.hFileInfo.ioFlFndrInfo = orig_cat_info_PB.hFileInfo.ioFlFndrInfo;
		new_cat_info_PB.hFileInfo.ioFlXFndrInfo = orig_cat_info_PB.hFileInfo.ioFlXFndrInfo;
		new_cat_info_PB.hFileInfo.ioFlCrDat = orig_cat_info_PB.hFileInfo.ioFlCrDat;
	}
	else {
		new_cat_info_PB.hFileInfo.ioFlFndrInfo.fdType = 'PREF';
		new_cat_info_PB.hFileInfo.ioFlFndrInfo.fdCreator = 'Peek';
	}
	new_cat_info_PB.hFileInfo.ioDirID = prefs_DirID;
	PBSetCatInfo( &new_cat_info_PB, FALSE );
	return;

cant_save:
	if ( write_PB.ioParam.ioRefNum )
		PBClose( (ParamBlockRec *) &write_PB, FALSE );
}


Handle Read_riff( void )
{
	HParamBlockRec read_PB;
	Handle riff;
	OSErr err;

	riff = 0;
	read_PB.ioParam.ioRefNum = 0;

	if ( get_prefs_directory() )
		goto no_riff;

	read_PB.ioParam.ioNamePtr = (StringPtr) riff_file_name;
	read_PB.ioParam.ioVRefNum = prefs_VRefNum;
	read_PB.fileParam.ioDirID = prefs_DirID;
	read_PB.ioParam.ioVersNum = 0;
	read_PB.ioParam.ioPermssn = 0;
	read_PB.ioParam.ioMisc = NULL;
	if ( PBHOpen( &read_PB, FALSE ) )
		goto no_riff;

	if ( PBGetEOF( (ParamBlockRec *) &read_PB, FALSE ) )
		goto no_riff;

	if ( !( riff = MFTempNewHandle( (Size) read_PB.ioParam.ioMisc, &err ) ) )
		goto no_riff;
	MFTempHLock( riff, &err );

	read_PB.ioParam.ioBuffer = (Ptr) StripAddress( *riff );
	read_PB.ioParam.ioReqCount = (int32) read_PB.ioParam.ioMisc;
	read_PB.ioParam.ioPosMode = fsFromStart;
	read_PB.ioParam.ioPosOffset = 0;
	if ( PBRead( (ParamBlockRec *) &read_PB, FALSE ) )
		goto no_riff;

	PBClose( (ParamBlockRec *) &read_PB, FALSE );
	return riff;

no_riff:
	if ( read_PB.ioParam.ioRefNum )
		PBClose( (ParamBlockRec *) &read_PB, FALSE );

	if ( riff )
		MFTempDisposHandle( riff, &err );

	return NULL;
}


void Dispose_riff( Handle riff )
{
	OSErr err;

	MFTempDisposHandle( riff, &err );
}
