/**

	drvr.csOpen.c
	Copyright (c) 1990, joe holt

 **/


/**-----------------------------------------------------------------------------
 **
 ** Headers
 **
 **/

#include "post.init.error.h"
#include "init.resources.h"
#include "drvr.patches.h"
#include "drvr.csOpen.h"


/**-----------------------------------------------------------------------------
 **
 ** Private Variables
 **
 **/

Boolean already_open = FALSE;


/*******************************************************************************
 **
 **	Public Variables
 **
 **/

SysEnvRec The_world;
int16 INIT_RefNum;


/*******************************************************************************
 **
 **	Public Functions
 **
 **/

OSErr csOpen( CntrlParam *cpb, DCtlPtr dce )
{
	register int16 i;
	register int16 init_error_strx;
	KeyMap keymap;

	INIT_RefNum = dce->dCtlRefNum;

	if ( already_open )
		return noErr;

	TheZone = SysZone;
	init_error_strx = 0;


	/**
	 **   If our global storage wasn't allocated, error
	 **/

	if ( !dce->dCtlStorage ) {
		init_error_strx = NOT_ENOUGH_MEMORY_STRx;
		goto exit;
	}


	/**------------------------------------------------------------
	 **
	 **   Initialize INIT
	 **
	 **/


	/**
	 **   works only with System 6.0.3 or greater
	 **/

	SysEnvirons( 1, &The_world );
	if ( The_world.systemVersion < 0x0603 ) {
		init_error_strx = BAD_SYSTEM_VERSION_STRx;
		goto exit;
	}


	/**
	 **   Not necessary to use the INIT with system 7
	 **/

	if ( The_world.systemVersion >= 0x0700 ) {
		init_error_strx = DONT_NEED_INIT_STRx;
		goto exit;
	}


	if ( Install_patches() ) {
		init_error_strx = NOT_ENOUGH_MEMORY_STRx;
		goto exit;
	}


	/**------------------------------------------------------------
	 **
	 **   End of initialization
	 **
	 **/

	DetachResource( dce->dCtlDriver );

exit:
	if ( init_error_strx )
		Post_init_error( init_error_strx );

	TheZone = ApplZone;
	return ( init_error_strx ? notOpenErr : noErr );
}


