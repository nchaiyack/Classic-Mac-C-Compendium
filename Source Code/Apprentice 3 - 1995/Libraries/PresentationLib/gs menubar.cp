// * gs Menubar utils
// * ©1995 Chris K. Thomas.  All Rights Reserved.

#include "gs menubar.h"

/*
	Strategy:
	Build a rectangular region representing the menubar
	and add or subtract it from the GrayRgn as necessary.
*/

void HideMenuBar (void)
// Hide the menubar - always balance with ShowMenuBar
{
	Rect		tempRect;
	RgnHandle	menuBarRgn;
	GDHandle	ourDevice = GetMainDevice();
	
	tempRect = (**(**ourDevice).gdPMap).bounds;
	tempRect.bottom = LMGetMBarHeight();
	
	menuBarRgn = NewRgn();
	RectRgn(menuBarRgn,&tempRect);
	UnionRgn(LMGetGrayRgn(),menuBarRgn,LMGetGrayRgn());
		
	DisposeRgn(menuBarRgn);
}


void ShowMenuBar (void)
//Show the menubar - always call HideMenuBar at some previous point in time
{
	Rect		tempRect;
	RgnHandle	menuBarRgn;
	GDHandle	ourDevice = GetMainDevice();
	
	tempRect = (**(**ourDevice).gdPMap).bounds;
	tempRect.bottom = LMGetMBarHeight();
	
	menuBarRgn = NewRgn();
	RectRgn(menuBarRgn,&tempRect);
	DiffRgn(LMGetGrayRgn(),menuBarRgn,LMGetGrayRgn());
		
	DisposeRgn(menuBarRgn);
}