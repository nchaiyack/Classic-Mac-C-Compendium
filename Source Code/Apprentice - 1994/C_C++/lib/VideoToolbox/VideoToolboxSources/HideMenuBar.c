/*
HideMenuBar.c

Based on:

"Code gadgets: Hiding the menu bar", THINKin' CaP, 1(2):28-29, Fall 1990.
Copyright © 1991 SPLAsh Resources.

and

Symantec THINK Reference 2, "How to Hide the MenuBar".

HISTORY:
2/28/91 dgp added to VideoToolbox
8/24/91	dgp	Made compatible with THINK C 5.
1/25/93 dgp removed obsolete support for THINK C 4.
1/25/93	dgp Replaced SysEqu.h by LoMem.h and changed program accordingly.
2/23/93	dgp	Call CopyQuickDrawGlobals to make sure qd is valid.
2/27/93	dgp Edited the code and comments, partly copying from THINK Reference's
			suggestion for how to do this. This was prompted by David Brainard's
			report that these routines were crashing when called within the
			MATLAB environment. The main changes are to also call
			CalcVisBehind() after restoring, and to use DiffRgn
			to restore by cutting the menu bar back out of the desktop
			instead of restoring by copying from a saved copy of the region. 
3/3/93	dgp	Added SquareCorners and RestoreCorners, with support for 1-bit quickdraw.
*/
#include "VideoToolbox.h"
#include <Menus.h>
#if THINK_C
	#include <LoMem.h>
#else
	short MBarHeight : 0xBAA;
#endif

static short oldMBarHeight;					// Pixel height of the menu bar
static RgnHandle mBarRgn=NULL;				// Region encompassing the menu bar
static RgnHandle cornerRgn[MAX_SCREENS];

void HideMenuBar(void)
{
	Rect r;
	
	if (MBarHeight>0) {
		mBarRgn=NewRgn();
		CopyQuickDrawGlobals();				// Make sure qd is valid
		r=qd.screenBits.bounds;
		r.bottom=r.top+MBarHeight;
		RectRgn(mBarRgn,&r);
		oldMBarHeight=MBarHeight;
		MBarHeight=0;
		UnionRgn(GetGrayRgn(),mBarRgn,GetGrayRgn());
		PaintOne(NULL,mBarRgn);
		CalcVisBehind((WindowPeek)FrontWindow(),mBarRgn);
	}
}

void ShowMenuBar(void)
{
	if(MBarHeight==0 && mBarRgn!=NULL){
		MBarHeight=oldMBarHeight;
		DiffRgn(GetGrayRgn(),mBarRgn,GetGrayRgn());
		DrawMenuBar();
		CalcVisBehind((WindowPeek)FrontWindow(),mBarRgn);
		DisposeRgn(mBarRgn);
		mBarRgn=NULL;
	}
}

void SquareCorners(GDHandle device)
// Extend GrayRgn to include this screen's corners, which otherwise might be rounded off.
// If NULL then applies to all screens.
{
	int i;
	Rect r;
	long quickDraw;
	
	Gestalt(gestaltQuickdrawVersion,&quickDraw);
	if(quickDraw<gestalt8BitQD){
		i=0;
		CopyQuickDrawGlobals();				// Make sure qd is valid
		r=qd.screenBits.bounds;
		r.top+=MBarHeight;
	}else{
		if(device==NULL){
			for(i=0;GetScreenDevice(i)!=NULL;i++)SquareCorners(GetScreenDevice(i));
			return;
		}
		i=GetScreenIndex(device);
		if(i>=MAX_SCREENS)return;
		r=(*device)->gdRect;
		if(device==GetMainDevice())r.top+=MBarHeight;
	}
	cornerRgn[i]=NewRgn();
	RectRgn(cornerRgn[i],&r);
	DiffRgn(cornerRgn[i],GetGrayRgn(),cornerRgn[i]);
	if(EmptyRgn(cornerRgn[i]))return;
	UnionRgn(GetGrayRgn(),cornerRgn[i],GetGrayRgn());
	PaintBehind((WindowPeek)FrontWindow(),cornerRgn[i]);
	CalcVisBehind((WindowPeek)FrontWindow(),cornerRgn[i]);
}

void RestoreCorners(GDHandle device)
// Restore rounding to this screen.
// If NULL then applies to all screens.
{
	int i;
	long quickDraw;
	
	Gestalt(gestaltQuickdrawVersion,&quickDraw);
	if(quickDraw<gestalt8BitQD){
		i=0;
	}else{
		if(device==NULL){
			for(i=0;GetScreenDevice(i)!=NULL;i++)RestoreCorners(GetScreenDevice(i));
			return;
		}
		i=GetScreenIndex(device);
		if(i>=MAX_SCREENS)return;
	}
	if(cornerRgn[i]!=NULL){
//		CopyQuickDrawGlobals();			// Make sure qd is valid.
//		FillRgn(cornerRgn[i],qd.black);	// don't know what port it belongs to
		DiffRgn(GetGrayRgn(),cornerRgn[i],GetGrayRgn());
		DisposeRgn(cornerRgn[i]);
		cornerRgn[i]=NULL;
	}
}

void UnclipScreen(GDHandle device)
{
	long quickDraw;

	Gestalt(gestaltQuickdrawVersion,&quickDraw);
	if(quickDraw<gestalt8BitQD || device==GetMainDevice())HideMenuBar();
	SquareCorners(device);
}

void RestoreScreenClipping(GDHandle device)
{
	long quickDraw;

	RestoreCorners(device);
	Gestalt(gestaltQuickdrawVersion,&quickDraw);
	if(quickDraw<gestalt8BitQD || device==GetMainDevice())ShowMenuBar();
}