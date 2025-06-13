/* about.c */

#include <QDOffScreen.h>
#include <Events.h>
#include "about.h"
#include "resources.h"
#include "TransSkel.h"
#include "list.h"
#include "windows.h"

pascal void DoAboutBox( short item )
{
	SplashScreen( true );
}

void SplashScreen( Boolean wait_for_click )
{
	CWindowPtr		window;
	PicHandle		picture;
	Rect			windRect;
	
	// use the app's resource fork!
	UseApplRes();
	picture = GetPicture( 128 );
	if( picture != nil )
	{
		windRect = (**(picture)).picFrame;
		
		OffsetRect( &windRect, 30, 50 );
								
		window = (CWindowPtr) NewCWindow( nil, &windRect, "\pAbout", false, dBoxProc,  
								(WindowPtr)-1L, true, (long) nil );
										
		if( window != nil )
		{
			ShowWindow( window );
			SetGWorld( window, GetMainDevice() );
			DrawPicture( picture, &window->portRect );

			// deal with update events in queue
			SkelDoUpdates();
			
			if( wait_for_click )
			{	while( !Button() )
				{
					SkelPause( 2 );
				}
			}
			else
			{	
				// short pause, then lose the splash screen
				SkelPause( 90 );	
			}
			
			HideWindow( window );
			DisposeWindow( window );
			ReleaseResource( picture );
		}
	}
	myUpdate(false);
}



	