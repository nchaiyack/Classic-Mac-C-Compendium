/**

	prefs.c
	Copyright (c) 1990-1992, joe holt

 **/


/**-----------------------------------------------------------------------------
 **
 **	Headers
 **
 **/

#include <Folders.h>

#ifndef __ctypes__
#include "ctypes.h"
#endif
#ifndef __prefs__
#include "prefs.h"
#endif
#ifndef __resources__
#include "resources.h"
#endif
#ifndef __swatch__
#include "swatch.h"
#endif


/**-----------------------------------------------------------------------------
 **
 **	Private Constants
 **
 **/

enum {
	DEFAULT_HEAP_SCALE = 8192,
	DEFAULT_HEAP_SCALE_2N = 13,

	DEFAULT_WINDOW_LEFT = 50,
	DEFAULT_WINDOW_TOP = 50,
	DEFAULT_WINDOW_RIGHT = 408,
	DEFAULT_WINDOW_BOTTOM = 210,

	DEFAULT_WNE_IN_FOREGROUND = 2,
	DEFAULT_WNE_IN_BACKGROUND = 15
};


enum {
	PREFS_VERSION = 3
};


/*******************************************************************************
 **
 **	Public Variables
 **
 **/

Preferences_t Prefs;


/*******************************************************************************
 **
 **	Public Functions
 **
 **/

void Prefs_init( void )
{
	short prefs_VRefNum;
	int32 prefs_DirID;
	OSErr err;
	HParamBlockRec read_PB;
	Rect r;

	read_PB.ioParam.ioRefNum = 0;

	err = FindFolder( kOnSystemDisk, kPreferencesFolderType, kDontCreateFolder,
			&prefs_VRefNum, &prefs_DirID );
	if ( err ) goto use_defaults;

	read_PB.ioParam.ioNamePtr = (StringPtr) pstr(Prefs_filename_STR_x);
	read_PB.ioParam.ioVRefNum = prefs_VRefNum;
	read_PB.fileParam.ioDirID = prefs_DirID;
	read_PB.ioParam.ioVersNum = 0;
	read_PB.ioParam.ioPermssn = 0;
	read_PB.ioParam.ioMisc = NULL;
	if ( PBHOpen( &read_PB, FALSE ) )
		goto use_defaults;

	read_PB.ioParam.ioBuffer = (Ptr) &Prefs;
	read_PB.ioParam.ioReqCount = sizeof( Prefs );
	read_PB.ioParam.ioPosMode = fsFromStart;
	read_PB.ioParam.ioPosOffset = 0;
	if ( PBRead( (ParamBlockRec *) &read_PB, FALSE ) )
		goto use_defaults;

	if ( Prefs.version != PREFS_VERSION )
		goto use_defaults;

	PBClose( (ParamBlockRec *) &read_PB, FALSE );
	read_PB.ioParam.ioRefNum = 0;

	r = Prefs.window_rect;
	r.top -= 18;
	r.bottom = r.top + 18;
	InsetRect( &r, 3, 3 );
	if ( !RectInRgn( &r, GrayRgn ) )
		goto use_default_window_rect;

	Prefs.dirty = false;
	return;

use_defaults:
	if ( read_PB.ioParam.ioRefNum )
		PBClose( (ParamBlockRec *) &read_PB, FALSE );
	Prefs.version = PREFS_VERSION;
	Prefs.heap_scale = DEFAULT_HEAP_SCALE;
	Prefs.heap_scale_2n = DEFAULT_HEAP_SCALE_2N;
	Prefs.wne_in_foreground = DEFAULT_WNE_IN_FOREGROUND;
	Prefs.wne_in_background = DEFAULT_WNE_IN_BACKGROUND;
use_default_window_rect:
	SetRect( &Prefs.window_rect, DEFAULT_WINDOW_LEFT, DEFAULT_WINDOW_TOP,
			DEFAULT_WINDOW_RIGHT, DEFAULT_WINDOW_BOTTOM);

	Prefs.dirty = false;	// setting prefs to default doesn't count
}


void Prefs_save( void )
{
	short prefs_VRefNum;
	int32 prefs_DirID;
	OSErr err;
	CInfoPBRec orig_cat_info_PB, new_cat_info_PB;
	HParamBlockRec write_PB;
	Boolean previous_exists;

	if ( !Prefs.dirty ) return;
	Prefs.dirty = false;

	write_PB.ioParam.ioRefNum = 0;

	err = FindFolder( kOnSystemDisk, kPreferencesFolderType, kCreateFolder,
			&prefs_VRefNum, &prefs_DirID );
	if ( err ) goto cant_save;

	orig_cat_info_PB.hFileInfo.ioNamePtr = (StringPtr) pstr(Prefs_filename_STR_x);
	orig_cat_info_PB.hFileInfo.ioVRefNum = prefs_VRefNum;
	orig_cat_info_PB.hFileInfo.ioDirID = prefs_DirID;
	orig_cat_info_PB.hFileInfo.ioFDirIndex = 0;
	if ( PBGetCatInfo( &orig_cat_info_PB, FALSE ) )
		previous_exists = FALSE;
	else
		previous_exists = TRUE;

	write_PB.fileParam.ioNamePtr = (StringPtr) pstr(
			previous_exists ? Prefs_tempname_STR_x : Prefs_filename_STR_x );
	write_PB.fileParam.ioVRefNum = prefs_VRefNum;
	write_PB.fileParam.ioDirID = prefs_DirID;
	write_PB.fileParam.ioFVersNum = 0;
	PBHCreate( &write_PB, FALSE );

	write_PB.ioParam.ioVersNum = 0;
	write_PB.ioParam.ioPermssn = 0;
	write_PB.ioParam.ioMisc = NULL;
	if ( PBHOpen( &write_PB, FALSE ) )
		goto cant_save;

	write_PB.ioParam.ioBuffer = (Ptr) &Prefs;
	write_PB.ioParam.ioReqCount = sizeof( Prefs );
	write_PB.ioParam.ioPosMode = fsFromStart;
	write_PB.ioParam.ioPosOffset = 0;
	if ( PBWrite( (ParamBlockRec *) &write_PB, FALSE ) )
		goto cant_save;

	PBClose( (ParamBlockRec *) &write_PB, FALSE );
	write_PB.ioParam.ioRefNum = 0;

	if ( previous_exists ) {
		write_PB.fileParam.ioNamePtr = (StringPtr) pstr(Prefs_filename_STR_x);
		write_PB.fileParam.ioDirID = prefs_DirID;
		PBHDelete( &write_PB, FALSE );

		write_PB.fileParam.ioNamePtr = (StringPtr) pstr(Prefs_tempname_STR_x);
		write_PB.ioParam.ioMisc = (Ptr) pstr(Prefs_filename_STR_x);
		PBHRename( &write_PB, FALSE );
	}

	new_cat_info_PB.hFileInfo.ioNamePtr = (StringPtr) pstr(Prefs_filename_STR_x);
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
