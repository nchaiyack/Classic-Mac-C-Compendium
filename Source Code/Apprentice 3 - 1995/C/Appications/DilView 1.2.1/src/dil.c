/* dil.c */

#include <QuickDraw.h>
#include <QDOffScreen.h>
#include <Values.h>
#include "dil.h"
#include "globals.h"
#include "constants.h"
#include "error.h"
#include "structs.h"
#include "windows.h"
#include "resources.h"
#include "GIF.h"
#include "status_bar.h"
#include "nyi.h"

GWorldPtr g_dil_buffer, g_scaled_dil_buffer;
OSErr gError;
short gMessage;

void myInitDil( void )
{
	Rect rect;
	QDErr err;

	// the full size offscreen buffer
	SetRect( &rect, 0, 0, k_dil_width, k_dil_height );
	err = NewGWorld( &g_dil_buffer, k_dil_buffer_depth, &rect, nil, nil, 0 );
	if( err != noErr ) myError( "Couldn't create offscreen buffer.  Try increasing memory allocation", true );
	
	// blank it
	SetPortToGWorld( g_dil_buffer );
	EraseRect( &(g_dil_buffer->portRect) );
	RestorePort();
	
	// the scaled buffer (for faster updates)
	SetRect( &rect, 0, 0, k_dil_width * GetScalePercent(), k_dil_height * GetScalePercent() );
	err = NewGWorld( &g_scaled_dil_buffer, k_scaled_dil_buffer_depth, &rect, nil, nil, 0 );
	if( err != noErr ) myError( "Couldn't create scaled offscreen buffer.  Try increasing memory allocation", true );
}

void myDisposeDil( void )
{
	DisposeGWorld( g_dil_buffer );
	DisposeGWorld( g_scaled_dil_buffer );
}

void DrawDil( void )
//
// draw the current dil to the window, possibly loading/scaling it if things have changed
// since the last time we drew it, otherwise just blast it onscreen from our buffer
//
{
	static dil_rec *last_dil = nil;
	static short last_scale = 0;
	Boolean reDither = false;
	dil_rec *cur_dil;
	short cur_scale;
	Rect rect;
	
	cur_dil = GetCurDil();
	cur_scale = GetScale();
	
	// the the dil being displayed has changed, we need to load the new one into the offscreen
	// buffer(s) before copying it onscreen!
	if( last_dil != cur_dil )
	{
		reDither = true;
		last_dil = cur_dil;
		LoadDilIntoDilBuffer( cur_dil );
	}
	
	// if the scale has changed we need to resize the scaled buffer, and redither
	if( last_scale != cur_scale )
	{
		reDither = true;
		last_scale = cur_scale;
		SetRect( &rect, 0, 0, k_dil_width * GetScalePercent(), k_dil_height * GetScalePercent() );
		UpdateGWorld( &g_scaled_dil_buffer, k_scaled_dil_buffer_depth, &rect, nil, nil, 0 );
	}

	// if we need to re-dither, dither to the window first to make it feel like the program
	// is responding to the user, then dither to the scaled buffer (I used to just srcCopy
	// the window to the scaled buffer, but that has the 'feature' of clipping if the window
	// was partially offscreen at the time..)  This is slower but is more flexible..

	if( reDither )
	{
		CopyGWorldToWindow( g_dil_buffer, nil, GetDilRect(&rect), ditherCopy );
		SetPortToGWorld( g_scaled_dil_buffer );

#if slower_but_better
		CopyBits( (BitMap *) *(g_dil_buffer->portPixMap), 
			(BitMap *) *(g_scaled_dil_buffer->portPixMap),
			&(g_dil_buffer->portRect), &(g_scaled_dil_buffer->portRect), ditherCopy, nil );
#else
		EraseRect( &(g_scaled_dil_buffer->portRect) );
		// only copy from the window if it's visible!
		if( ((WindowPeek)GetGWinPort())->visible )
		{
			CopyBits( &(((GrafPtr)GetGWinPort())->portBits), 
				(BitMap *) *(g_scaled_dil_buffer->portPixMap),
				GetDilRect(&rect), &(g_scaled_dil_buffer->portRect), srcCopy, nil );
		}
#endif
		RestorePort();
	}
	else
	{
		// if we don't need to re-dither, then just blast the scaled buffer onscreen
		CopyGWorldToWindow( g_scaled_dil_buffer, nil, GetDilRect(&rect), srcCopy );
	}
}

void LoadDilIntoDilBuffer( dil_rec *dil )
{
	if( dil != nil )
	{
		// reload the picture if its been purged from memory
		if( *(dil->pict_handle) == nil )
		{
			dil->pict_handle = GetPicture( dil->pict_id );
			if( *(dil->pict_handle) == nil ) 
				myError( "Unable to retrieve PICT from scratch file!", false );
		}
		
		// lock down the handle 
		HLock( dil->pict_handle );
		
		// set the port, draw the picture, restore the port
		SetPortToGWorld( g_dil_buffer );
		DrawPicture( dil->pict_handle, &(g_dil_buffer->portRect) );
		RestorePort();
		
		// unlock the handle and make sure its purgable
		HUnlock( dil->pict_handle );
		HPurge( dil->pict_handle );
	}
}

Rect *GetDilRect( Rect *rect )
{
	SetRect( rect, 0, 0, k_dil_width * GetScalePercent(), 
				k_dil_height * GetScalePercent() );
	return( rect );
}

void DrawDilInRect( dil_rec *dil, Rect *rect )
//
// draw the specified dil in the current port in the specified rect.  used for printing.
//
{
	GrafPtr print_port;
	GrafPtr oldPort;

	// get a pointer to the current port (probably when printing!)
	GetPort( &print_port );

	// load the dil into our full size buffer (the port stuff is nec b/c port vs gworld
	// incompatibilities..)
	GetPort( &oldPort );
	StatusBarMsg( k_printing, dil->fname );
	LoadDilIntoDilBuffer( dil );
	SetPort( oldPort );
	
	// draw it into the port
	CopyBits( (BitMap *) *(g_dil_buffer->portPixMap), 
			&(print_port->portBits),
			&(g_dil_buffer->portRect), rect, ditherCopy, nil );
}

dil_rec *NewDilRec( void )
//
// allocate a new dil_rec, return a pointer to it
//
{
	dil_rec *dil;
	
	dil = (dil_rec *) NewPtr( sizeof(dil_rec) );
	
	if( dil == nil )
		myError( "Out of memory error", true );
	
	dil->marked = false;
	dil->next = nil;
	dil->pict_id = 0;

	return( dil );
}
	
dil_rec *GifToDil( FSSpec *spec )
//
// given a file spec, return a dil_rec, with the gif converted to a pict, stored in the scratch file
//
{
	dil_rec *dil;
	long mem_used;
	GWorldPtr tempGWorld;
	OpenCPicParams picParam;
	short i;
		
	// allocate a new dil_rec
	dil = NewDilRec();
		
	// make sure the current GWorld is ours.. (otherwise DrawGIF sometimes crashes?)
	SetGWorld( (CGrafPtr) GetGWinPort(), GetMainDevice() );
		
	// create a GWorld and draw the Gif into it
	DrawGIF( spec, MAXLONG, 0L, &tempGWorld, &mem_used );
	
// NO! KEEP 8 BIT! (larger scratch file, better picture)	
//	// change to 4 bit
//	UpdateGWorld( &tempGWorld, 4, &(tempGWorld->portRect), nil, nil,
//		 (GetMMUMode() == false32b ? keepLocal : 0) );
	
	// any problems?
	if( gMessage != noErr )
	{
		if( gMessage == 129 )
			myError( "Not enough memory to decode GIF.  Please increase my memory allocation!", true );
		else
		{
			myError( "An error occured trying to extract the gif, sorry!", false );
			// reset gMessage for future use
			gMessage = noErr;	
			return( nil );
		}
	}
	
	// set the port to be the GIF's
//	SetPortToGWorld( tempGWorld );
	SetGWorld( tempGWorld, GetMainDevice() );
	ClipRect( &(tempGWorld->portRect) );
				
	// open a picture (blank)
	picParam.srcRect = tempGWorld->portRect;
	picParam.hRes = picParam.vRes = 0x00480000;
	picParam.version = -2;
#if 1
	dil->pict_handle = OpenCPicture( &picParam ); 
#else	
	dil->pict_handle = OpenPicture( &(tempGWorld->portRect) );
#endif

	// check for problems
	if( dil->pict_handle == nil )
		myError( "Probably out of memory..", true );
	
	// copy the port to itself, thereby making its contents the picture
	CopyBits( (BitMap *)(*(tempGWorld->portPixMap)),
			(BitMap *)(*(tempGWorld->portPixMap)), 
			&(tempGWorld->portRect), &(tempGWorld->portRect), srcCopy, 0L );

	// close the picture.. now it should be a valid pichandle
	ClosePicture();
		
//	RestorePort();

	// lose the memory for the temp world
	DisposeGWorld( tempGWorld );
	
	// get the id for a PICT resource in our scratch file
	dil->pict_id = PicHandleToPICT( dil->pict_handle );
	
	// store the filename
	for( i=0 ; i<64 ; i++ )
		((unsigned char *)dil->fname)[i] = ((unsigned char *)spec->name)[i];
	
	// return a pointer to the newly created dil_rec
	return( dil );
}

