#include <Types.h>
#include <Memory.h>
#include <Quickdraw.h>
#include <Fonts.h>
#include <Events.h>
#include <Menus.h>
#include <Windows.h>
#include <TextEdit.h>
#include <Dialogs.h>
#include <OSUtils.h>
#include <ToolUtils.h>
#include <SegLoad.h>
#include <OSEvents.h>
#include <Desk.h>

#include "MSBitMapLib.h"


EventRecord			gMainEvent;
Rect				gWindowRect;
WindowPtr			gWindowPtr;
Boolean				gUserDone;

void Demo_Init(void);
void Draw(void);

#pragma segment Main


void Demo_Init()
{
	InitGraf( &qd.thePort);
	InitFonts();		   		
	InitWindows();
	TEInit();
	InitDialogs( NULL );

	gUserDone = false;

	FlushEvents( everyEvent, nullEvent );

	InitCursor();
}


void Draw(void)
{
	TextSize(24);
	TextFont(bold);
	
/*************************************************************************
This is what you're looking for. This is how to use the routine that 
rotates text. The parameters are: 
	xPixel, 
	yPixel, 
	the string to be rotated.
**************************************************************************/
	MSDrawRotatedText (100,300, (MSStr255)"\pThe Sideways Text");
}   


void main()
{
	MoreMasters();
	MoreMasters();
	MoreMasters();
	MoreMasters();

	Demo_Init();
	gWindowRect = qd.screenBits.bounds;
	InsetRect( &gWindowRect, 30, 30 );

	if ( MSColorQDExists())	
	{
		gWindowPtr = NewCWindow(NULL, &gWindowRect, "\p", true, dBoxProc, (WindowPtr)(-1), true, 0L );
		if (gWindowPtr != NULL)
			SetPort( gWindowPtr );
	} 
	else
	{
		gWindowPtr = NewWindow(NULL, &gWindowRect, "\p", true, dBoxProc, (WindowPtr)(-1), false, (long)(WindowPtr)NULL );
		if (gWindowPtr != NULL)
			SetPort( gWindowPtr );
	}

	Draw();	
	
	MoveTo( 10, 20 );
	TextSize(12);
	TextFace(0);
	DrawString( "\pPress mouse button or any key to quit." );

	do
	{
		SystemTask();

		if (GetNextEvent( everyEvent, &gMainEvent )) 
		{
			switch (gMainEvent.what) 
			{
				case mouseDown:
					gUserDone = true;
					break;

				case keyDown:
				case autoKey:
					gUserDone = true;
					break;

				default:
					break;
			}
		}
	} while (!gUserDone);
}
