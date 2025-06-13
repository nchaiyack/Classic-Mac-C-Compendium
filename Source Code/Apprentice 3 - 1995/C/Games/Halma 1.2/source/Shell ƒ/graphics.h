/**********************************************************************\

File:		graphics.h

Purpose:	This is the header file for graphics.c

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

#pragma once

#include "QDOffscreen.h"

#define MAX_TE_HANDLES 1

typedef struct
{
	WindowPtr			theWindowPtr;			/* window ptr of window */
	short				windowIndex;			/* index of window in program's window list */
	short				windowWidth;			/* width of window content, in pixels */
	short				windowHeight;			/* height of window content, in pixels */
	short				windowType;				/* type of window (see IM Essentials, 4-80) */
	short				windowDepth;			/* current pixel depth of window */
	short				maxDepth;				/* maximum pixel depth of window */
	Boolean				hasCloseBox;			/* window has a close box */
	Boolean				offscreenNeedsUpdate;	/* TRUE if offscreen bitmap needs redrawing */
	Boolean				isColor;				/* TRUE if color, FALSE if grayscale */
	Point				initialTopLeft;			/* initial window bounds when opened */
	Rect				windowBounds;			/* on screen rectangle of window content */
	TEHandle			hTE[MAX_TE_HANDLES];	/* array of textedit handles */
	Str31				windowTitle;			/* pascal string, title of window */
} WindowDataRec, *WindowDataPtr, **WindowDataHandle;

typedef short (*dispatchProcPtr)(WindowDataHandle theData, short theMessage, unsigned long misc);

typedef struct									/* exactly the same as WindowDataRec */
{												/* + dispatchProc at end */
	WindowPtr			theWindowPtr;
	short				windowIndex;
	short				windowWidth;
	short				windowHeight;
	short				windowType;
	short				windowDepth;
	short				maxDepth;
	Boolean				hasCloseBox;
	Boolean				offscreenNeedsUpdate;
	Boolean				isColor;
	Point				initialTopLeft;
	Rect				windowBounds;
	TEHandle			hTE[MAX_TE_HANDLES];
	Str31				windowTitle;
	dispatchProcPtr		dispatchProc;			/* called with message of windowish event */
} ExtendedWindowDataRec, *ExtendedWindowDataPtr, **ExtendedWindowDataHandle;

enum					/* messages passed to window's dispatch procedure */
{
	kNull=0,			/* on null event when window is active, frgrnd/bkgrnd */
	kStartup,			/* on program startup */
	kShutdown,			/* on program shutdown */
	kInitialize,		/* just before window is created & shown */
	kOpen,				/* just after window is created & shown */
	kUpdate,			/* during window update -- draw contents to current grafport */
	kClose,				/* just before window is closed -- this can cancel close */
	kDispose,			/* just after window is closed/disposed */
	kActivate,			/* on window activate event */
	kDeactivate,		/* on window deactivate event */
	kSuspend,			/* on program suspension (switched into background) */
	kResume,			/* on program resuming (switching into foreground) */
	kKeydown,			/* on keydown event when window is active & in foreground */
	kMousedown,			/* on mousedown event in window content when active & in frgrnd */
	kUndo,				/* specific to window */
	kCut,
	kCopy,
	kPaste,
	kClear,
	kSelectAll,
	kChangeDepth,		/* offscreen bitmap just created or pixel depth changed */
	kCopybits			/* actual copybits to update onscreen window from offscreen */
};

enum					/* return codes from window dispatch procedure */
{
	kSuccess=0,			/* message handled, no further processing please */
	kFailure,			/* message not handled, use default action if any */
	kCancel				/* message refused, cancel action (only good with kClose) */
};

/***************************************************************************************/

Boolean InitTheGraphics(void);
void ShutDownTheGraphics(void);
void OpenTheIndWindow(short index);
void GetMainScreenBounds(void);
short GetWindowDepth(ExtendedWindowDataHandle theData);
short GetBiggestDeviceDepth(ExtendedWindowDataHandle theData);
Boolean WindowIsColor(ExtendedWindowDataHandle theData);
void UpdateTheWindow(ExtendedWindowDataHandle theData);
Boolean CloseTheWindow(ExtendedWindowDataHandle theData);
Boolean CloseTheIndWindow(short index);
PicHandle DrawThePicture(PicHandle thePict, short whichPict, short x, short y);
PicHandle ReleaseThePict(PicHandle thePict);
Boolean SetPortToOffscreen(WindowDataHandle theData);
void RestorePortToScreen(WindowDataHandle theData);
GrafPtr GetOffscreenGrafPtr(WindowDataHandle theData);
GrafPtr GetIndOffscreenGrafPtr(short index);
GrafPtr GetWindowGrafPtr(WindowDataHandle theData);
GrafPtr GetIndWindowGrafPtr(short index);
ExtendedWindowDataHandle GetIndWindowDataHandle(short index);
void SetIndDispatchProc(short index, ProcPtr theProc);
short CallIndDispatchProc(short index, short theMessage, unsigned long misc);
short CallDispatchProc(ExtendedWindowDataHandle theData, short theMessage,
	unsigned long misc);
void SetIndWindowTitle(short index, Str255 theTitle);
void KillOffscreen(short index);
