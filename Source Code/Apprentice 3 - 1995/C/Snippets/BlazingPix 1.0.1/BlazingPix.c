/************************************************************************/
/*																		*/
/* 	BlazingPix.c -- Written by Jay Riley to illustrate crude animation.	*/
/*																		*/
/*	Copyright © 1991 James N Riley Jr, All rights reserved.				*/
/*																		*/
/*	11/22/91 10.28 PM	JNR		Created.								*/
/*	11/23/91 12.44 PM	JNR		Last Change.							*/
/*																		*/
/************************************************************************/

/********************************************************************
 Ported to Metrowerks CodeWarrior by Paul Celestin on 4 November 1994
 ********************************************************************/

/*#include "MacTypes.h"
#include "QuickDraw.h"
#include "WindowMgr.h"*/

pascal void main()
{
	long						oldTCount;
	Handle						iconHand;
	Point						mouseIsHere;
	Rect						bgRect		=	{0, 0, 240, 384},
								wRect		=	{60, 64, 300, 448},
								drawSpriteHere;
	BitMap						background,
								icon,
								mask;
	GrafPort					bgPort;
	WindowPtr					ourWindow;
	
	/******************************/
	/* Bare Bones Initialization. */
	/******************************/
	
	InitGraf(&qd.thePort);
	InitFonts();
	InitWindows();
	
	InitCursor();
	
	HideCursor();
	
	/**********************/
	/* Create Our Window. */
	/**********************/
	
	if (ourWindow = NewWindow(0L, &wRect, "\p\"Blazing Pixels\" Sample Program",
		true, 0, (WindowPtr)-1L, false, 0L))
	{
		/*********************************/
		/* Allocate an Offscreen BitMap. */
		/*********************************/
		
		background.bounds = bgRect;
		background.rowBytes = (((bgRect.right - bgRect.left) + 31) / 32) *4;
		
		if (background.baseAddr = NewPtr((long)background.rowBytes *
			(long)(bgRect.bottom - bgRect.top)))
		{
			/*************************************************/
			/* Allocate a GrafPort for the Offscreen BitMap. */
			/*************************************************/
			
			OpenPort(&bgPort);
			
			TextFace(outline);
					
			/***************************************************/
			/* Make sure GrafPort uses background as portBits. */
			/***************************************************/
			
			SetPortBits(&background);
			ClipRect(&bgRect);

			/********************************/
			/* Setup Icon and Mask BitMaps. */
			/********************************/
			
			if (iconHand = GetResource('ICN#', 3))
			{
				HLock(iconHand);
				
				SetRect(&icon.bounds, 0, 0, 32, 32);
				icon.rowBytes = 4;
				icon.baseAddr = *iconHand;
				
				mask = icon;
				mask.baseAddr += 128;
				
				/***************************************************/
				/* No Real Event Loop -- Just Check For mouseDown. */
				/***************************************************/
				
				while(! Button())
				{
					/*********************/
					/* Clear Background. */
					/*********************/
					
					SetPort(&bgPort);
					
					FillRect(&bgRect, &qd.ltGray);
					
					/****************************************/
					/* Make ourWindow the current GrafPort. */
					/****************************************/
					
					SetPort(ourWindow);
					
					/*********************************************/
					/* Calculate Sprite Position based on Mouse. */
					/*********************************************/
					
					GetMouse(&mouseIsHere);
					
					if (mouseIsHere.h < 0)
						mouseIsHere.h = 0;
					else if (mouseIsHere.h > bgRect.right)
						mouseIsHere.h = bgRect.right;
					
					if (mouseIsHere.v < 0)
						mouseIsHere.v = 0;
					else if (mouseIsHere.v > bgRect.bottom)
						mouseIsHere.v = bgRect.bottom;
					
					SetRect(&drawSpriteHere, mouseIsHere.h - 16,
						mouseIsHere.v - 16, mouseIsHere.h + 16,
						mouseIsHere.v + 16);
						 
					/******************************/
					/* Draw Sprite on background. */
					/******************************/
					
					SetPort(&bgPort);
					
					CopyBits(&mask, &background, &mask.bounds, &drawSpriteHere,
						srcBic, 0L);
						
					CopyBits(&icon, &background, &icon.bounds, &drawSpriteHere,
						srcOr, 0L);
					
					MoveTo(90, 200);
					DrawString("\pPress Mouse Button To Quit.");
	
					/**********************************************/
					/* Synch with VBL Interupt to reduce flicker. */
					/**********************************************/
					
					oldTCount = TickCount();
					while (oldTCount == TickCount());
					
					/*******************************************/
					/* Copy Offscreen background to ourWindow. */
					/*******************************************/
					
					CopyBits(&background, &ourWindow->portBits, &bgRect, &bgRect,
						srcCopy, 0L);
				}	/* END: while(! Button()) */
					
				/********************************/
				/* Clean House Before Quitting. */
				/********************************/
			
				HUnlock(iconHand);
				ReleaseResource(iconHand);
			}	/* END: if (iconHand = GetResource('ICN#', 3)) */
			
			ClosePort(&bgPort);
			
			DisposPtr(background.baseAddr);
		}
		
		DisposeWindow(ourWindow);
	}
	ShowCursor();
}