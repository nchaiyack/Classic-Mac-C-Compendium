/* resources.c */

#include "resources.h"
#include "error.h"
#include "windows.h"

short g_appl_res_ref, g_scratch_res_ref;

void UseApplRes( void )
{
	UseResFile( g_appl_res_ref );
}

void myInitResource( void )
{
	// store a handle to the application's resource fork
	g_appl_res_ref = CurResFile();
	
	// create the scratch file if not already there
	CreateScratchFile();
	
	// open it
	g_scratch_res_ref = OpenResFile( k_scratch_rsrc_file_name );
	
	// check for problems
	if( g_scratch_res_ref == -1 )
		myError( "An error occured trying to create/open the scratch file.", true );
}

void myDisposeResource( void )
{
	// close the scratch file
	CloseResFile( g_scratch_res_ref );
	
	// delete it if appropriate
	DeleteScratchFile();
}

void StoreWinLoc( void )
{
	Handle topleftH;
	Point *topleftPtr;
	WindowPtr window_port;
	
	SetPortToWindow();
	UseResFile( g_appl_res_ref );
	
	window_port = GetGWinPort();
	topleftH = GetResource( k_winloc_rsrc_type, k_winloc_rsrc_id );
	
	// make sure we have it..
	if( topleftH != nil )
	{
		HLock( topleftH );
	
		// point to the rsrc
		topleftPtr = (Point *) *topleftH;

		topleftPtr->h = window_port->portRect.left;
		topleftPtr->v = window_port->portRect.top;
				
		// make them global 
		LocalToGlobal( topleftPtr );
		
		// store it
		ChangedResource( topleftH );
		WriteResource( topleftH );
		
		// lose the mem.. 
		ReleaseResource( topleftH );
	}
	else
		myError( "Can�t store window coordinates!  Continuing..", false );
}

void RestoreWinLoc( void )
{	
	Handle topleftH;
	Point *topleftPtr;
	RgnHandle screenRgnH;
	WindowPtr window_port;
	
	SetPortToWindow();
	UseResFile( g_appl_res_ref );
	
	window_port = GetGWinPort();
	topleftH = GetResource( k_winloc_rsrc_type, k_winloc_rsrc_id );
	
	// make sure we have it..
	if( topleftH != nil )
	{
		HLock( topleftH );
	
		// get the coords 
		topleftPtr = (Point *) *topleftH;
		
		// validate the window will be partly onscreen
		screenRgnH = GetGrayRgn();
		if( !PtInRgn( *topleftPtr, screenRgnH ) )
		{
			topleftPtr->h = k_default_win_h;
			topleftPtr->v = k_default_win_v;
		}
		
		// move the window to them
		MoveWindow( window_port, topleftPtr->h, topleftPtr->v, false );
		
 		// lose the mem.. 
		ReleaseResource( topleftH );
	}
	else
		myError( "Can�t load window coordinates!  Continuing..", false );
}

short PicHandleToPICT( PicHandle pic )
{
	short pict_id;
	
	// make sure we have it
	if( pic == nil )
		myError( "Programmer Error: Can't make a resource from a nil PicHandle!", true );

// NOT TRUE FOR COLOR QD?	
//	// if the picSize is negative (-1), we have a problem.
//	if( (**pic).picSize < 0 )
//		myError( "Invalid picture.  Probably out of memory!  Try increasing memory allocation.", true );
	
	// lock it down
	HLock( pic );
	
	UseResFile( g_scratch_res_ref );
	
	// get an id for it
	pict_id = Unique1ID( 'PICT' );
	
	// store it
	AddResource( pic, 'PICT', pict_id, "\p" );
	ChangedResource( pic );
	WriteResource( pic );
	
	// free it up, so it may be purged if memory gets tight
	HUnlock( pic );
	HPurge( pic );
	
	return( pict_id );
}

void CreateScratchFile( void )
{
	short vRefNum;
	OSErr err;
	
	GetVol( 0, &vRefNum );
	
	// delete the file if it exists (heck, just try to delete it, and ignore errors)
	DeleteScratchFile();
	
	// try to create the file
	err = Create( k_scratch_rsrc_file_name, vRefNum, k_DilView_creator_type,
				k_DilView_scratch_file_type );
	if( err == dupFNErr ) // its ok if it already exists
		err = noErr;
	if( err != noErr )
		myError( "Couldn't create the scratch file.  Make sure the volume is unlocked", true );
	
	// add a rsrc fork			
	CreateResFile( k_scratch_rsrc_file_name );
	
	// _don't_ open the file here.. our purpose is only to create.
}

void DeleteScratchFile( void )
{
	short vRefNum;
	OSErr err;
	
	GetVol( 0, &vRefNum );

	err = FSDelete( k_scratch_rsrc_file_name, vRefNum );
	if( err == fnfErr ) // if the error is other than 'file not found' then oops!
		err = noErr;
	if( err != noErr )
		myError( "Couldn't delete old scratch file!  Make sure the volume is unlocked", false );
}

