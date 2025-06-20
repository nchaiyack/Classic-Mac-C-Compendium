/**********************************************************************\

File:		demo.c

Purpose:	This module handles demo initialization/shutdown and a
			dispatch for the graphic effects and fades.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program in a file named "GNU General Public License".
If not, write to the Free Software Foundation, 675 Mass Ave,
Cambridge, MA 02139, USA.

\**********************************************************************/

#include "demo.h"
#include "demo crash.h"
#include "graphics.h"
#include "demo graphics.h"
#include "wipe headers.h"
#include "reversed wipe headers.h"
#include "fade headers.h"
#include "reversed fade headers.h"
#include "program globals.h"
#include "prefs.h"
#include "menus.h"
#include "file management.h"

typedef pascal int (*wipeProcPtr)(GrafPtr sourceGrafPtr, GrafPtr destGrafPtr, Rect boundsRect);
typedef pascal int (*fadeProcPtr)(Rect boundsRect, Pattern *thePattern);

unsigned char	gIsReversed;
unsigned char	gWhichPict;
int				gWipeStatus;
int				gWhichWipe;
int				gLastWipe;
wipeProcPtr		gTheWipe[NUM_WIPES+1], gTheReverseWipe[NUM_WIPES+1];
fadeProcPtr		gTheFade[NUM_WIPES+1], gTheReverseFade[NUM_WIPES+1];

void InitTheProgram(void)
{
	int				index;
	
	InitFiles();
	
	if (gIsReversed)
		ReverseAllWipes();
	
	gWhichWipe=0;
	
	(**(gTheWindowData[kMainWindow])).dispatchProc=MainWindowDispatch;		/* demo graphics.c */
	((**(gTheWindowData[kMainWindow])).dispatchProc)(gTheWindowData[kMainWindow], kStartup, 0L);
	
	index=1;
	gTheFade[index++]=BoxOutFade;
	gTheFade[index++]=RandomFade;
	gTheFade[index++]=SpiralGyraFade;
	gTheFade[index++]=CircularFade;
	gTheFade[index++]=0L;					/* CasteWipeRL can't be a fade */
	gTheFade[index++]=FourCornerFade;
	gTheFade[index++]=BoxInFade;
	gTheFade[index++]=0L;					/* RippleWipe can't be a fade */
	gTheFade[index++]=0L;					/* DissolveWipe can't easily be a fade */
	gTheFade[index++]=DiagonalFade;
	gTheFade[index++]=0L;				/* CasteWipe can't be a fade */
	gTheFade[index++]=HilbertFade;
	gTheFade[index++]=SlideFade;
	gTheFade[index++]=SkipalineFade;
	gTheFade[index++]=SkipalineLRFade;
	gTheFade[index++]=ScissorsFade;
	gTheFade[index++]=RescueRaidersFade;
	gTheFade[index++]=FourCornerCenteredFade;
	gTheFade[index++]=TwoCornerFade;
	gTheFade[index++]=CircleOutFade;
	gTheFade[index++]=CircleInFade;
	gTheFade[index++]=CircleSerendipityFade;
	gTheFade[index++]=CircleBulgeFade;
	gTheFade[index++]=CornerCircleFade;
	gTheFade[index++]=QuadrantFade;
	gTheFade[index++]=QuadrantFade2;
	gTheFade[index++]=Skipaline2PassFade;
	gTheFade[index++]=SkipalineLR2PassFade;
	gTheFade[index++]=HGRFade;
	gTheFade[index++]=HGR2Fade;
	gTheFade[index++]=FullScrollLRFade;
	gTheFade[index++]=FullScrollUDFade;
	gTheFade[index++]=MrDoFade;
	gTheFade[index++]=MrDoOutdoneFade;
	gTheFade[index++]=SplitScrollUDFade;
	gTheFade[index++]=FourCornerScrollFade;
	gTheFade[index++]=HalvesScrollFade;
	gTheFade[index++]=PourScrollFade;
	gTheFade[index++]=QuadrantScrollFade;
	gTheFade[index++]=QuadrantScroll2Fade;
	if (index!=NUM_WIPES+1)
		Debugger();
	
	index=1;
	gTheReverseFade[index++]=BoxInFade;
	gTheReverseFade[index++]=RandomFade;
	gTheReverseFade[index++]=0L;				/* Spiral Gyra can't be easily reversed. */
	gTheReverseFade[index++]=CircularFadeReversed;
	gTheReverseFade[index++]=0L;				/* CasteWipeRight can't be a fade */
	gTheReverseFade[index++]=FourCornerFadeReversed;
	gTheReverseFade[index++]=BoxOutFade;
	gTheReverseFade[index++]=0L;				/* RippleWipe Reversed can't be a fade */
	gTheReverseFade[index++]=0L;				/* DissolveWipe can't easily be a fade */
	gTheReverseFade[index++]=DiagonalFadeDownRight;
	gTheReverseFade[index++]=0L;				/* CasteWipeDown can't be a fade */
	gTheReverseFade[index++]=HilbertFadeReversed;
	gTheReverseFade[index++]=SlideFadeReversed;
	gTheReverseFade[index++]=SkipalineFade;
	gTheReverseFade[index++]=SkipalineLRFade;
	gTheReverseFade[index++]=ScissorsFadeReversed;
	gTheReverseFade[index++]=RescueRaidersFadeReversed;
	gTheReverseFade[index++]=FourCenteredFadeReversed;
	gTheReverseFade[index++]=TwoCornerFadeReversed;
	gTheReverseFade[index++]=CircleInFade;
	gTheReverseFade[index++]=CircleOutFade;
	gTheReverseFade[index++]=0L;				/* CircleSerendipity can't be easily reversed */
	gTheReverseFade[index++]=0L;				/* CircleBulge can't be easily reversed */
	gTheReverseFade[index++]=CornerCircleFadeReversed;
	gTheReverseFade[index++]=QuadrantFadeReversed;
	gTheReverseFade[index++]=QuadrantFade2Reversed;
	gTheReverseFade[index++]=Skipaline2PassFadeReversed;
	gTheReverseFade[index++]=SkipalineLR2PassFadeReversed;
	gTheReverseFade[index++]=HGRFadeReversed;
	gTheReverseFade[index++]=HGR2FadeReversed;
	gTheReverseFade[index++]=FullScrollLeftFade;
	gTheReverseFade[index++]=FullScrollUpFade;
	gTheReverseFade[index++]=MrDoFadeReversed;
	gTheReverseFade[index++]=MrDoOutdoneFadeReversed;
	gTheReverseFade[index++]=0L;				/* SplitScrollUD can't be reversed */
	gTheReverseFade[index++]=0L;				/* FourCornerScroll can't be reversed */
	gTheReverseFade[index++]=HalvesScrollFadeReversed;
	gTheReverseFade[index++]=PourScrollFadeReversed;
	gTheReverseFade[index++]=QuadrantScrollFadeReversed;
	gTheReverseFade[index++]=QuadrantScroll2FadeReversed;
	if (index!=NUM_WIPES+1)
		Debugger();
	
	index=1;
	gTheWipe[index++]=BoxOutWipe;
	gTheWipe[index++]=RandomWipe;
	gTheWipe[index++]=SpiralGyra;
	gTheWipe[index++]=CircularWipe;
	gTheWipe[index++]=CasteWipeRL;
	gTheWipe[index++]=FourCorner;
	gTheWipe[index++]=BoxInWipe;
	gTheWipe[index++]=RippleWipe;
	gTheWipe[index++]=DissolveWipe;
	gTheWipe[index++]=DiagonalWipe;
	gTheWipe[index++]=CasteWipe;
	gTheWipe[index++]=HilbertWipe;
	gTheWipe[index++]=SlideWipe;
	gTheWipe[index++]=Skipaline;
	gTheWipe[index++]=SkipalineLR;
	gTheWipe[index++]=Scissors;
	gTheWipe[index++]=RescueRaiders;
	gTheWipe[index++]=FourCornerCentered;
	gTheWipe[index++]=TwoCorner;
	gTheWipe[index++]=CircleOut;
	gTheWipe[index++]=CircleIn;
	gTheWipe[index++]=CircleSerendipity;
	gTheWipe[index++]=CircleBulge;
	gTheWipe[index++]=CornerCircle;
	gTheWipe[index++]=QuadrantWipe;
	gTheWipe[index++]=QuadrantWipe2;
	gTheWipe[index++]=Skipaline2Pass;
	gTheWipe[index++]=SkipalineLR2Pass;
	gTheWipe[index++]=HGR;
	gTheWipe[index++]=HGR2;
	gTheWipe[index++]=FullScrollLR;
	gTheWipe[index++]=FullScrollUD;
	gTheWipe[index++]=MrDo;
	gTheWipe[index++]=MrDoOutdone;
	gTheWipe[index++]=SplitScrollUD;
	gTheWipe[index++]=FourCornerScroll;
	gTheWipe[index++]=HalvesScroll;
	gTheWipe[index++]=PourScroll;
	gTheWipe[index++]=QuadrantScroll;
	gTheWipe[index++]=QuadrantScroll2;
	if (index!=NUM_WIPES+1)
		Debugger();
	
	index=1;
	gTheReverseWipe[index++]=BoxInWipe;
	gTheReverseWipe[index++]=RandomWipe;
	gTheReverseWipe[index++]=0L;			/* Spiral Gyra can't be easily reversed */
	gTheReverseWipe[index++]=CircularWipeReversed;
	gTheReverseWipe[index++]=CasteWipeRight;
	gTheReverseWipe[index++]=FourCornerReversed;
	gTheReverseWipe[index++]=BoxOutWipe;
	gTheReverseWipe[index++]=RippleWipeReversed;
	gTheReverseWipe[index++]=DissolveWipe;
	gTheReverseWipe[index++]=DiagonalWipeDownRight;
	gTheReverseWipe[index++]=CasteWipeDown;
	gTheReverseWipe[index++]=HilbertWipeReversed;
	gTheReverseWipe[index++]=SlideWipeReversed;
	gTheReverseWipe[index++]=Skipaline;
	gTheReverseWipe[index++]=SkipalineLR;
	gTheReverseWipe[index++]=ScissorsReversed;
	gTheReverseWipe[index++]=RescueRaidersReversed;
	gTheReverseWipe[index++]=FourCornerCenteredReversed;
	gTheReverseWipe[index++]=TwoCornerReversed;
	gTheReverseWipe[index++]=CircleIn;
	gTheReverseWipe[index++]=CircleOut;
	gTheReverseWipe[index++]=0L;			/* CircleSerendipity can't be easily reversed */
	gTheReverseWipe[index++]=0L;			/* CircleBulge can't be easily reversed */
	gTheReverseWipe[index++]=CornerCircleReversed;
	gTheReverseWipe[index++]=QuadrantWipeReversed;
	gTheReverseWipe[index++]=QuadrantWipe2Reversed;
	gTheReverseWipe[index++]=Skipaline2PassReversed;
	gTheReverseWipe[index++]=SkipalineLR2PassReversed;
	gTheReverseWipe[index++]=HGRReversed;
	gTheReverseWipe[index++]=HGR2Reversed;
	gTheReverseWipe[index++]=FullScrollLeft;
	gTheReverseWipe[index++]=FullScrollUp;
	gTheReverseWipe[index++]=MrDoReversed;
	gTheReverseWipe[index++]=MrDoOutdoneReversed;
	gTheReverseWipe[index++]=0L;			/* SplitScrollUD can't be reversed */
	gTheReverseWipe[index++]=0L;			/* FourCornerScroll can't be reversed */
	gTheReverseWipe[index++]=HalvesScrollReversed;
	gTheReverseWipe[index++]=PourScrollReversed;
	gTheReverseWipe[index++]=QuadrantScrollReversed;
	gTheReverseWipe[index++]=QuadrantScroll2Reversed;
	if (index!=NUM_WIPES+1)
		Debugger();
}

void ShutDownTheProgram(void)
{
	((**(gTheWindowData[kMainWindow])).dispatchProc)(gTheWindowData[kMainWindow], kShutdown, 0L);
	SaveThePrefs();
}

void DoFullScreenFade(int whichWipe)
{
	Rect			theRect;
	int				oldMenuBarHeight;

	ObscureCursor();
	oldMenuBarHeight=GetMBarHeight();
	MBarHeight=0;
	DrawMenuBar();
	SetPort(WMgrPort);
	theRect=WMgrPort->portRect;
	
	gWhichWipe=whichWipe;
	FadeDispatch(theRect);
	RestoreScreen(oldMenuBarHeight);	
	gLastWipe=gWhichWipe;
	gWhichWipe=0;
}

void FadeDispatch(Rect theRect)
{
	if (gIsReversed)
	{
		if (gTheReverseFade[gWhichWipe]!=0L)
			gTheReverseFade[gWhichWipe](theRect, &black);
	}
	else
	{
		if (gTheFade[gWhichWipe]!=0L)
			gTheFade[gWhichWipe](theRect, &black);
	}
}

void WipeDispatch(GrafPtr sourceGrafPtr, GrafPtr destGrafPtr, Rect theRect)
{
	ObscureCursor();

	gWhichPict = !gWhichPict;
	
	if ((gWipeStatus==kFadesPlusEffects) || (gWipeStatus==kFadesPlusReverseEffects))
		FadeDispatch(theRect);
	if (gWipeStatus==kFadesPlusReverseEffects)
		gIsReversed=!gIsReversed;
	
	if (gIsReversed)
	{
		if (gTheReverseWipe[gWhichWipe]!=0L)
			gTheReverseWipe[gWhichWipe](sourceGrafPtr, destGrafPtr, theRect);
		else
			CopyBits(&(sourceGrafPtr->portBits), &(destGrafPtr->portBits), &theRect, &theRect, 0, 0L);
	}
	else
	{
		if (gTheWipe[gWhichWipe]!=0L)
			gTheWipe[gWhichWipe](sourceGrafPtr, destGrafPtr, theRect);
		else
			CopyBits(&(sourceGrafPtr->portBits), &(destGrafPtr->portBits), &theRect, &theRect, 0, 0L);
	}
	
	if (gWipeStatus==kFadesPlusReverseEffects)
		gIsReversed=!gIsReversed;
	
	gLastWipe=gWhichWipe;
	gWhichWipe=0;
}
