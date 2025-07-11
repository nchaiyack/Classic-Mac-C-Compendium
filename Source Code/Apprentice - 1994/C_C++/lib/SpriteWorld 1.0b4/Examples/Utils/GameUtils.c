///--------------------------------------------------------------------------------------
//	GameUtils.c
//
//	Created:	9/19/90
//	By:		Tony Myles
//
//	Copyright: � 1990-94 Tony Myles, All rights reserved worldwide
//
//	Description:	some utility functions for games
///--------------------------------------------------------------------------------------


#ifndef __QUICKDRAW__
#include <QuickDraw.h>
#endif

#ifndef __MENUS__
#include <Menus.h>
#endif

#ifndef __GAMEUTILS__
#include "GameUtils.h"
#endif

#if MPW
#pragma segment Utils
#endif


///--------------------------------------------------------------------------------------
//	GetRandom
//
//	generate a random number between min and max inclusive
///--------------------------------------------------------------------------------------

unsigned short GetRandom(
	unsigned short min,
	unsigned short max)
{
	unsigned short random;
	long range, temp;

	random = Random();
	range = (max - min) + 1;
	temp = (random * range) / 65536;
	random = temp + min;

	return random;
}


void CenterRect(
	Rect* srcRect,
	Rect* dstRect)
{
	short width = (dstRect->right - dstRect->left);
	short height = (dstRect->bottom - dstRect->top);

	dstRect->left = srcRect->left + (((srcRect->right - srcRect->left) / 2) - (width / 2));
	dstRect->top = srcRect->top + (((srcRect->bottom - srcRect->top) / 2) - (height / 2));
	dstRect->right = dstRect->left + width;
	dstRect->bottom = dstRect->top + height;
}


/*
        // globals for the menubar showing/hiding stuff
*/
RgnHandle gOldVisRgn = NULL;


///--------------------------------------------------------------------------------------
//	HideMenuBar
///--------------------------------------------------------------------------------------

void HideMenuBar(
	GrafPtr grafPort)
{
	RgnHandle newVisRgn;
	GrafPtr savePort;

	GetPort(&savePort);
	SetPort(grafPort);

		// save off vis region
	gOldVisRgn = NewRgn();
	CopyRgn(grafPort->visRgn, gOldVisRgn);

		// expand the vis region to the port rect
	newVisRgn = NewRgn();
	RectRgn(newVisRgn, &grafPort->portRect);
	CopyRgn(newVisRgn, grafPort->visRgn);
	DisposeRgn(newVisRgn);

	SetPort(savePort);
}


///--------------------------------------------------------------------------------------
//	ShowMenuBar
///--------------------------------------------------------------------------------------

void ShowMenuBar(
	GrafPtr grafPort)
{
	GrafPtr savePort;
	RgnHandle junkRgn;

	GetPort(&savePort);
	SetPort(grafPort);

		// fill the rounded corners of the screen with black again
	junkRgn = NewRgn();
	CopyRgn(gOldVisRgn, junkRgn);
	DiffRgn(grafPort->visRgn, junkRgn, junkRgn);

	#ifdef dangerousPattern
	FillRgn(junkRgn, qd.black);
	#else
	FillRgn(junkRgn, &qd.black);
	#endif

	DisposeRgn(junkRgn);

		// restore the old vis region
	CopyRgn(gOldVisRgn, grafPort->visRgn);
	DisposeRgn(gOldVisRgn);
	gOldVisRgn = NULL;

	DrawMenuBar();
}

