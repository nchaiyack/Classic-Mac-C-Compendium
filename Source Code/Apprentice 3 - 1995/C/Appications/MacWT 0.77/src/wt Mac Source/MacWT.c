/*
** File:		MacWT.c
**
** Written by:	Bill Hayden
**				Nikol Software
**
** Copyright © 1995 Nikol Software
** All rights reserved.
*/



#include <stdio.h>
#include <string.h>
#include <QDOffscreen.h>

#include "Failure.h"
#include "wt.h"
#include "error.h"
//#include "input.h"
#include "framebuf.h"
#include "graphics.h"
#include "MacGame.h"

/**/


GWorldPtr		gWorld;
Rect			gWorldRect;
WindowPtr		gWTFTWindow;


/**/


long gStartTicks, gFrameCount;


/**/




Graphics_info *init_graphics(int width, int height, Boolean blocky)
{
	static Graphics_info ginfo;
	short		x, ncolors;
	GDHandle	oldGD;
	GWorldPtr	oldWorld;
	OSErr		err;	

	// Screen width must be multiple of 4...

	if (width & 3)
		fatal_error("Screen width must be a multiple of 4.");
	
	GetGWorld(&oldWorld, &oldGD);        	// Get about box port
		
	SetRect(&gWorldRect, 0, 0, width, height);
	err = NewGWorld(&gWorld, 8, &gWorldRect, nil, nil, 0);
	if (err)
		Fail(err, __FILE__, __LINE__, TRUE);
			
	(*GetGWorldPixMap(gWorld))->rowBytes = 0x8000 | width;
	
	SetGWorld(oldWorld, oldGD);          	// Set port to about box

	ginfo.width = width;
	ginfo.height = height;
     
	ncolors = 256;
	ginfo.palette.rgb_cube_size = 6;	// unused in this implementation
	ginfo.palette.color_lookup = (long *)NewPtr(sizeof(int) * ncolors);

	for (x = 0; x < 256 ; x++)
		ginfo.palette.color_lookup[x] = (int)x;

	return &ginfo;
}


/**/


void end_graphics(void)
{
	if (gWorld) DisposeGWorld(gWorld);
}


/**/


void MacAttractMode(void)
{
	short		oldFont, oldSize, oldFace, oldMode;

	if (!gWTFTWindow)
		Fail(ResError(), __FILE__, __LINE__, TRUE);

	// This is a bit of a hack, IÕm afraid...

	if (!gGameOn) {
		SetPort(gWTFTWindow);

		oldFont = gWTFTWindow->txFont;
		oldSize = gWTFTWindow->txSize;
		oldFace = gWTFTWindow->txFace;
		oldMode = gWTFTWindow->txMode;

		TextFont(geneva);
		TextSize(9);
		TextMode(srcOr);

		MoveTo(4, gWTFTWindow->portRect.bottom - kBottomBorder + 10);
		TextFace(bold);
		DrawString((StringPtr)"\pwt ");
		DrawString(gWTVersion);
		DrawString((StringPtr)"\p - Chris LaurelÕs 3-D game engine ");
#if	__powerc
		DrawString((StringPtr)"\p(PPC)");
#else
		DrawString((StringPtr)"\p(68K)");
#endif
		MoveTo(4, gWTFTWindow->portRect.bottom - kBottomBorder + 21);
		TextFace(normal);
		DrawString((StringPtr)"\pWorld File Ñ ");
		DrawText(gWorldFileName, 0, strlen(gWorldFileName));
		MoveTo(4, gWTFTWindow->portRect.bottom - kBottomBorder + 32);
		TextFace(italic);
		DrawString((StringPtr)"\pTo use default world file, hold down OPTION while launching");

		TextFont(oldFont);
		TextSize(oldSize);
		TextFace(oldFace);
		TextMode(oldMode);
	}
}


/**/


void BeginGame(void)
{
	Rect	r;

	if (!gWTFTWindow)
		Fail(ResError(), __FILE__, __LINE__, TRUE);
	r = gWTFTWindow->portRect;
	r.top = r.bottom - kBottomBorder;
	EraseRect(&r);

	gGameOn = true;
	if (gPaused)
		TogglePause();
}


/**/


void RefreshWTWindow(void)
{
	long			tNow;
	GWorldPtr		frontWorld;
	GDHandle		frontGD;
	Rect			dstRect;
	OSErr			err;
	
	if (!gWTFTWindow)
		Fail(ResError(), __FILE__, __LINE__, TRUE);

	dstRect = gWTFTWindow->portRect;
	dstRect.bottom -= kBottomBorder;
	
	ForeColor(blackColor);								// recommended by Apple when
	BackColor(whiteColor);								// using CopyBits drawing
	
	SetPort(gWTFTWindow);
	
	GetGWorld(&frontWorld,&frontGD);                	// be sure we're in the about box
	SetGWorld(frontWorld,frontGD);
	if (!LockPixels(GetGWorldPixMap(gWorld)))			// make sure everything kosher
		{
		UpdateGWorld(&gWorld, 0, &gWorldRect, nil, nil, 0);
		if (err = LockPixels(GetGWorldPixMap(gWorld)));
			Fail(err, __FILE__, __LINE__, TRUE);
		}
	
	if (gUseQuickDraw)
		CopyBits(&((GrafPtr)gWorld)->portBits,			// copy from GWorld
				 &gWTFTWindow->portBits,				// to window
				 &gWorldRect, &dstRect, srcCopy, nil);	// using CopyBits
	else
		CopyBits(&((GrafPtr)gWorld)->portBits,			// copy from GWorld
				 &gWTFTWindow->portBits,				// to window
				 &gWorldRect, &dstRect, srcCopy, nil);	// using Direct-Screen Write (someday)

	UnlockPixels(GetGWorldPixMap(gWorld));				// done drawing in about box
	
	DisposeGWorld(frontWorld);							// Scrap our temporary GWorldPtr


	ShowIfPaused();

	MoveTo(12, gWTFTWindow->portRect.bottom - kBottomBorder + 20);

	if (gGameOn && gShowFPS)
		{
		char			aStr[256];
		
		tNow = (TickCount() - gStartTicks) / 60;
		if (!tNow)
			tNow = 1;

		aStr[0] = sprintf(aStr+1, "Frame: %ld, fps: %2.2f  ", gFrameCount, (float)gFrameCount/(float)tNow);
		DrawString((StringPtr)aStr);
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


inline Pixel *get_framebuffer_memory(int width, int height)
{
	return (Pixel *)GetPixBaseAddr( GetGWorldPixMap( gWorld ) );
}


