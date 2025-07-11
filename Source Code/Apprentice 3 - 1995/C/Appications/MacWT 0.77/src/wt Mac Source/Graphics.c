/*
** File:		Graphics.c
**
** Written by:	Bill Hayden
**				Nikol Software
**
** Copyright � 1995 Nikol Software
** All rights reserved.
*/



#include <stdio.h>
#include <string.h>
#include <QDOffscreen.h>
#include <Palettes.h>

#include "Failure.h"
#include "wt.h"
#include "error.h"
#include "framebuf.h"
#include "graphics.h"
#include "MacWT.h"
#include "Constants.h"
#include "StringUtils.h"
#include "Blit.h"

/**/


static	GWorldPtr	gWorld;
static	Rect		gWorldRect;
static	Rect		gDestinationRect;
static	Rect		gInformationRect;
		DialogPtr	gWindow;
		long		gStartTicks, gFrameCount;
static	PixMapHandle	srcPixHandle;
static	PixMapHandle	portPixMap;


/**/




Graphics_info *init_graphics(int width, int height, Boolean blocky)
{
	static Graphics_info ginfo;
	GDHandle	oldGD;
	GWorldPtr	oldWorld;
	OSErr		err;


	// Screen width must be multiple of 4...

	if (width & 3)
		fatal_error("Screen width must be a multiple of 4.");
	
	GetGWorld(&oldWorld, &oldGD);        	// Get main window port
		
	SetRect(&gWorldRect, 0, 0, width, height);
	err = NewGWorld(&gWorld, gTrueColor ? 16 : 8, &gWorldRect, nil, nil, 0);
	if (err)
		Fail(err, __FILE__, __LINE__, TRUE);
			
	(*GetGWorldPixMap(gWorld))->rowBytes = 0x8000 | width;

	SetGWorld(gWorld, nil);
	EraseRect(&gWorld->portRect);
	
	SetGWorld(oldWorld, oldGD);          	// Set port to main window

	srcPixHandle = GetGWorldPixMap(gWorld);
	HLock((Handle) srcPixHandle);

	portPixMap = ((CGrafPtr)qd.thePort)->portPixMap;
 	HLock((Handle) portPixMap);

	InitBlit(*srcPixHandle, *portPixMap, &gWorldRect, &gDestinationRect);

	ginfo.width = width;
	ginfo.height = height;

	if (!gTrueColor)
		{
		const short ncolors = 256;
		short x;

		ginfo.color_lookup = (long *)NewPtr(sizeof(long) * ncolors);
		for (x = 0; x < ncolors ; x++)
			ginfo.color_lookup[x] = (long)x;
		}
	else
		ginfo.color_lookup = nil;

	return &ginfo;
}


/**/


void end_graphics(void)
{
	GDHandle	theGDevice;

	if (gWorld) DisposeGWorld(gWorld);

	theGDevice = GetMainDevice();
	if ( gScreenDepth != (*(*theGDevice)->gdPMap)->pixelSize )
		SetDepth( theGDevice, gScreenDepth, 0, 0 );
}


/**/


void MacAttractMode(void)
{
	if (!gWindow)
		Fail(ResError(), __FILE__, __LINE__, TRUE);

	// This is a bit of a hack, I�m afraid...

	if (!gGameOn)
		{
		Str255	SplashString = "\pMacWT ";

		SetPort(gWindow);

		pcat(SplashString, gWTVersion);
		pcat(SplashString, "\p, executing "
#if	__powerc
		"PowerPC code.");
#else
		"MC680x0 code.");
#endif

		BackColor(blackColor);
		ForeColor(whiteColor);
		TextBox(SplashString + 1, SplashString[0], &gInformationRect, teCenter);
		}
}


/**/


void BeginGame(void)
{
	BackColor(blackColor);
	EraseRect(&gInformationRect);

	gGameOn = true;
	if (gPaused)
		TogglePause();
}


/**/


void RefreshWTWindow(void)
{
	GWorldPtr		frontWorld;
	GDHandle		frontGD;

	
	if (!gWindow)
		Fail(ResError(), __FILE__, __LINE__, TRUE);

	SetPort(gWindow);
	
	GetGWorld(&frontWorld,&frontGD);                	// be sure we're in the about box
	SetGWorld(gWorld, nil);
	if (!LockPixels(GetGWorldPixMap(gWorld)))			// make sure everything is kosher
		{
		UpdateGWorld(&gWorld, gScreenDepth, &gWorldRect, nil, nil, 0);
		if (!LockPixels(GetGWorldPixMap(gWorld)))
			Fail(ResError(), __FILE__, __LINE__, TRUE);
		}
	
	SetGWorld(frontWorld,frontGD);

	ForeColor(blackColor);								// recommended by Apple when
	BackColor(whiteColor);								// using CopyBits drawing

	if (gUseQuickdraw)
		CopyBits(&((GrafPtr)gWorld)->portBits,			// copy from GWorld
				 &gWindow->portBits,					// to window
				 &gWorldRect,							// using CopyBits
				 &gDestinationRect, srcCopy, nil);
	else
		CopyBlit(*srcPixHandle,
				 *portPixMap,
				 &gWorldRect,
				 &gDestinationRect);										// copy using CopyBlit

	UnlockPixels(GetGWorldPixMap(gWorld));				// done drawing in about box	
	DisposeGWorld(frontWorld);							// Scrap our temporary GWorldPtr

	if (gGameOn && gShowFPS)
		{
		char			aStr[256];
		long			tNow;

		ForeColor(whiteColor);
		BackColor(blackColor);
		
		tNow = (TickCount() - gStartTicks) / 60;
		if (!tNow)
			tNow = 1;

		aStr[0] = sprintf(aStr+1, "Frame: %ld, fps: %2.2f   ", gFrameCount, (float)gFrameCount/(float)tNow);
		TextBox(aStr + 1, aStr[0], &gInformationRect, teCenter);
		}

}

/**/


void update_screen(Framebuffer *fb)
{
	RefreshWTWindow();

	if (gGameOn)
		{
		++gFrameCount;
		if (!gStartTicks)
			gStartTicks = TickCount();
		}
}


/**/


Pixel *get_framebuffer_memory(int width, int height)
{
	return (Pixel *)GetPixBaseAddr( GetGWorldPixMap( gWorld ) );
}



void GetScreenSize(short *width, short *height)
{
	short	ScreenSize, iType;
	Handle	iHandle;


	ScreenSize = Alert(rScreenSize, nil);

	GetDItem( gWindow, ScreenSize, &iType, &iHandle, &gDestinationRect );
	GetDItem( gWindow, 5, &iType, &iHandle, &gInformationRect );

	*width = gDestinationRect.right - gDestinationRect.left;
	*height = gDestinationRect.bottom - gDestinationRect.top;
}




void ShowIfPaused(void)
{
	Str255		aStr = "\pGame Paused - press TAB to continue";
	PenState	savePenState;

	GetPenState( &savePenState );	/* save current state */
	ShowPen();						/* we want pen to be visible */

	ForeColor(blackColor);
	PenMode(patOr);
	PenPat(&qd.ltGray);
	PaintRect(&gDestinationRect);

	ForeColor(whiteColor);
	BackColor(blackColor);

	TextBox(aStr + 1, aStr[0], &gInformationRect, teCenter);

	SetPenState( &savePenState );	/* restore the pen's visible state */
}
