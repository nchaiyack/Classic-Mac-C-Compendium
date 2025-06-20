/* load_files.c */

#include "load_files.h"

/* load_files.c */

#include <Files.h>
#include "load_files.h"
#include "multiopen.h"
#include "dil.h"
#include "globals.h"
#include "list.h"
#include "status_bar.h"
#include "windows.h"

extern SFReply gTheReply;

void LoadAndAddGIF( FSSpec *spec )
{
	dil_rec *dil;
	
	StatusBarMsg( k_processing, spec->name );
	
	dil = GifToDil( spec );
		
	if( dil != nil )
	{
		SetCurDil( dil );
		AddDilToList( dil );
	}
}

void OpenOne( void )
{
	StandardFileReply	mySFR;
	SFTypeList			myTypeList;
	
	myTypeList[0] = 'GIFf';
	
	StandardGetFile( nil, 1, myTypeList, &mySFR );
	
	if( mySFR.sfGood )
	{
		// cleanup after closing the DLOG
		myUpdate( false );
	
		LoadAndAddGIF( &(mySFR.sfFile) );
	}

	// restore the normal status bar
	DrawStatusBar();
}

void OpenMany( void )
{
	Point	where = {-1, -1};
			
	// note: the file filter can be omitted, if you want.
	SFPGetFile(where, "\pSelect files:", myFileFilter, -1, nil, myDlgHook, &gTheReply, 128, myFilterProc);
	
	if (gTheReply.good)
	{
		// cleanup after closing the DLOG
		myUpdate( false );
	
		processData();
	}
	
	// part of multi_open.c, presumably to fix data structs for re-use.
	cleanup();
	
	// restore the normal status bar
	DrawStatusBar();
}

