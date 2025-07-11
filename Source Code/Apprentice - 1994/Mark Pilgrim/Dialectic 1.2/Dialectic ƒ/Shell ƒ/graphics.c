/**********************************************************************\

File:		graphics.c

Purpose:	This module handles opening/closing/updating all windows:
			this includes manipulating offscreen GWorlds & bitmaps
			for fun and profit.

\**********************************************************************/

#include "graphics.h"
#include "about.h"
#include "about MSG.h"
#include "help.h"
#include "dialogs.h"
#include "error.h"
#include "menus.h"
#include "environment.h"
#include "prefs.h"
#include "program globals.h"

WindowPtr		gTheWindow[NUM_WINDOWS];			/* windowptr of nth window */
ExtendedWindowDataHandle
				gTheWindowData[NUM_WINDOWS];		/* handle to nth window data struct */

/* internal global variables for use by graphics.c only */
static	Rect		gBoundsRect[NUM_WINDOWS];		/* rectangle of offscreen bitmap */
static	Rect		gMainScreenBounds;				/* bounds of main monitor */
static	GWorldPtr	gTheGWorld[NUM_WINDOWS];		/* offscreen graphics world */
static	int			gOffscreenDepth[NUM_WINDOWS];	/* pixel depth of offscreen GWorld */
static	Ptr			gBWBitMap[NUM_WINDOWS];			/* offscreen bitmap for B/W machines */
static	GrafPort	gBWGrafPort[NUM_WINDOWS];		/* offscreen grafport "  "     "     */
static	GrafPtr		gBWGrafPtr[NUM_WINDOWS];		/* offscreen grafptr  "  "     "     */


Boolean InitTheGraphics(void)
{
	int				i;
	
	GetMainScreenBounds();
	
	for (i=0; i<NUM_WINDOWS; i++)
	{
		/* nothing is inited; if there's an error later on, we'll know how much to */
		/* clean up in ShutDownTheGraphics() */
		gTheWindow[i]=gTheGWorld[i]=gTheWindowData[i]=0L;
	}
	
	for (i=0; i<NUM_WINDOWS; i++)
	{
		gTheWindowData[i]=(ExtendedWindowDataHandle)NewHandle(sizeof(ExtendedWindowDataRec));
		if (gTheWindowData[i]==0L)							/* return if error */
			return FALSE;
		
		(**(gTheWindowData[i])).offscreenNeedsUpdate=TRUE;	/* offscreen not inited */
		(**(gTheWindowData[i])).windowIndex=i;				/* so we can retrieve it O(1) */
	}
	
	/* set window dispatch routines */
	(**(gTheWindowData[kAbout])).dispatchProc=AboutBoxDispatch;		/* see about.c */
	(**(gTheWindowData[kHelp])).dispatchProc=HelpWindowDispatch;	/* see help.c */
	(**(gTheWindowData[kAboutMSG])).dispatchProc=AboutMSGBoxDispatch;/* see about MSG.c */
	
	/* call window dispatch routines with "startup" message */
	((**(gTheWindowData[kAbout])).dispatchProc)(gTheWindowData[kAbout], kStartup, 0L);
	((**(gTheWindowData[kHelp])).dispatchProc)(gTheWindowData[kHelp], kStartup, 0L);
	((**(gTheWindowData[kAboutMSG])).dispatchProc)(gTheWindowData[kAboutMSG], kStartup, 0L);
	
	return TRUE;
}

void OpenTheWindow(int index)
{
	if (!gTheWindow[index])		/* if window exists, we'll just update it (see below) */
	{
		if (((**(gTheWindowData[index])).dispatchProc)	/* call window's dispatch to */
				(gTheWindowData[index], kInitialize, 0L)==kFailure)	/* initialize it */
		{		/* default is to center window on main screen */
			(**(gTheWindowData[index])).initialTopLeft.h =
				gMainScreenBounds.left + (((gMainScreenBounds.right -
				gMainScreenBounds.left) - (**(gTheWindowData[index])).windowWidth) / 2);
			(**(gTheWindowData[index])).initialTopLeft.v =
				gMainScreenBounds.top + (((gMainScreenBounds.bottom -
				gMainScreenBounds.top) - (**(gTheWindowData[index])).windowHeight) / 2);
		}
		
		(**(gTheWindowData[index])).windowBounds.left=
			(**(gTheWindowData[index])).initialTopLeft.h;
		
		(**(gTheWindowData[index])).windowBounds.top=
			(**(gTheWindowData[index])).initialTopLeft.v;
			
		if (((**(gTheWindowData[index])).windowType==noGrowDocProc) ||
			((**(gTheWindowData[index])).windowType==documentProc) ||
			((**(gTheWindowData[index])).windowType==movableDBoxProc) ||
			((**(gTheWindowData[index])).windowType==zoomDocProc) ||
			((**(gTheWindowData[index])).windowType==zoomNoGrow) ||
			((**(gTheWindowData[index])).windowType==rDocProc))
				(**(gTheWindowData[index])).windowBounds.top += 9;	/* compensate for title */
		
		/* don't put window over menu bar */
		if ((**(gTheWindowData[index])).windowBounds.top < GetMBarHeight()+1)
			(**(gTheWindowData[index])).windowBounds.top = GetMBarHeight()+1;
		
		(**(gTheWindowData[index])).windowBounds.bottom =
			(**(gTheWindowData[index])).windowBounds.top +
			(**(gTheWindowData[index])).windowHeight;
		
		(**(gTheWindowData[index])).windowBounds.right =
			(**(gTheWindowData[index])).windowBounds.left +
			(**(gTheWindowData[index])).windowWidth;
		
		if (gHasColorQD)
		{
			HLock(gTheWindowData[index]);	/* important!  NewCWindow may move memory */
			/* create the color window with our specs, see IM Essentials 4-79ff */
			gTheWindow[index] = NewCWindow(0L, &((**(gTheWindowData[index])).windowBounds),
				(**(gTheWindowData[index])).windowTitle, FALSE,
				(**(gTheWindowData[index])).windowType, (WindowPtr)-1L,
				(**(gTheWindowData[index])).hasCloseBox,
				(unsigned long)gTheWindowData[index]);
			HUnlock(gTheWindowData[index]);	/* no sense keeping it locked now */
		}
		else
		{
			HLock(gTheWindowData[index]);	/* important!  NewWindow may move memory */
			/* create the B/W window with our specs, see IM Essentials 4-82ff */
			gTheWindow[index] = NewWindow(0L, &((**(gTheWindowData[index])).windowBounds),
				(**(gTheWindowData[index])).windowTitle, FALSE,
				(**(gTheWindowData[index])).windowType, (WindowPtr)-1L,
				(**(gTheWindowData[index])).hasCloseBox,
				(unsigned long)gTheWindowData[index]);
			HUnlock(gTheWindowData[index]);	/* no sense keeping it locked now */
		}
	}
	
	if (gTheWindow[index])
	{
		ShowWindow(gTheWindow[index]);			/* immediately show this new window */
		SelectWindow(gTheWindow[index]);		/* immediately select this new window */
		/* call window's dispatch routine to alert it that it's open now */
		((**(gTheWindowData[index])).dispatchProc)(gTheWindowData[index], kOpen, 0L);
		SetPort(gTheWindow[index]);				/* important! */
		UpdateTheWindow(gTheWindowData[index]);	/* immediately update this new window */
	}
	else HandleError(kNoMemory, FALSE);			/* if unsuccessful, display error */
}

void GetMainScreenBounds(void)
{
	gMainScreenBounds = screenBits.bounds;		/* low-mem global */
	gMainScreenBounds.top += GetMBarHeight();	/* don't include menu bar */
}

int GetWindowDepth(ExtendedWindowDataHandle theData)
{
	int				index;
	
	index=(**theData).windowIndex;
	/* if Color Quickdraw is not available, the depth must be 1. */
	/* if Color Quickdraw is available and the window exists, return the window's
	   GWorld's graphics device's pixel map's pixel depth */
	/* if Color Quickdraw is available and the window does not exist, return the
	   pixel depth of the main screen */
	return (gHasColorQD) ? ((gTheWindow[index]) ?
			(**(**(GetGWorldDevice(gTheGWorld[index]))).gdPMap).pixelSize :
			(**(**GetMainDevice()).gdPMap).pixelSize) : 1;
}

void UpdateTheWindow(ExtendedWindowDataHandle theData)
{
	int				index;
	GWorldPtr		currentGWorld;
	GDHandle		currentGDHandle;
	long			offRowBytes, sizeOfOff;
	
	index=(**theData).windowIndex;
	gBoundsRect[index]=gTheWindow[index]->portRect;
	OffsetRect(&gBoundsRect[index], -gBoundsRect[index].left, -gBoundsRect[index].top);

	if (gHasColorQD)	/* w/o Color Quickdraw, GWorlds may not be supported */
	{
		if (gTheGWorld[index]==0L)		/* create new graphics world if none exists */
		{
			/* try to create new graphics world; display error if unsuccessful */
			if (NewGWorld(&gTheGWorld[index], 0, &gBoundsRect[index], 0L, 0L, 0)!=0)
			{
				HandleError(kNoMemory, FALSE);
				return;
			}
			gOffscreenDepth[index]=GetWindowDepth(theData);	/* keep track of pixel depth */
			NoPurgePixels(GetGWorldPixMap(gTheGWorld[index]));	/* never purge our pixmap! */
		}
		
		GetGWorld(&currentGWorld, &currentGDHandle);	/* get current settings */
		LockPixels(GetGWorldPixMap(gTheGWorld[index]));	/* important!  copybits may move mem */
		/* update offscreen graphics world, compensating for change in pixel depth */
		UpdateGWorld(&gTheGWorld[index], 0, &gBoundsRect[index], 0L, 0L, 0);
		SetGWorld(gTheGWorld[index], 0L);				/* set to our offscreen gworld */

		if (gOffscreenDepth[index]!=GetWindowDepth(theData))	/* if pixel depth changed */
		{
			gOffscreenDepth[index]=GetWindowDepth(theData);		/* save new depth */
			(**theData).offscreenNeedsUpdate=TRUE;				/* we'll need to redraw */
		}
	}
	else	/* deal with (guaranteed) B/W bitmaps manually */
	{
		if (gBWGrafPtr[index]==0L)	/* create new offscreen bitmap if none exists */
		{
			gBWGrafPtr[index]=&gBWGrafPort[index];
			OpenPort(gBWGrafPtr[index]);	/* make a new port */
			
			/* calculate the size of the offscreen bitmap from the boundsrect */
			offRowBytes=(((gBoundsRect[index].right-gBoundsRect[index].left)+15)>>4)<<1;
			sizeOfOff=(long)(gBoundsRect[index].bottom-gBoundsRect[index].top)*offRowBytes;
			
			gBWBitMap[index]=NewPtr(sizeOfOff);		/* allocate space for bitmap */
			if (gBWBitMap[index]==0L)				/* abort if unsuccessful */
			{
				ClosePort(gBWGrafPtr[index]);		/* cleaning up... */
				gBWGrafPtr[index]=0L;
				HandleError(kNoMemory, FALSE);		/* displaying error... */
				return;								/* and aborting... */
			}
			
			gBWGrafPort[index].portBits.baseAddr=gBWBitMap[index];	/* --> our bitmap */
			gBWGrafPort[index].portBits.rowBytes=offRowBytes;		/* bitmap size */
			gBWGrafPort[index].portBits.bounds=						/* bitmap bounds */
				gBWGrafPort[index].portRect=gBoundsRect[index];			
		}
		
		SetPort(gBWGrafPtr[index]);					/* set port for subsequent drawing */
	}	
	
	if ((**theData).offscreenNeedsUpdate)			/* if we need to redraw */
	{
		(**theData).offscreenNeedsUpdate=FALSE;		/* not anymore */
		/* call window's dispatch and tell it to redraw itself */
		((**theData).dispatchProc)(theData, kUpdate, GetWindowDepth(theData));
	}
	
	if (gHasColorQD)
		SetGWorld(currentGWorld, currentGDHandle);	/* restore old settings */
	
	SetPort(gTheWindow[index]);						/* set to window to draw to */
	/* copy offscreen bitmap from graphics world or bitmap to onscreen window */
	CopyBits(gHasColorQD ? &(((GrafPtr)gTheGWorld[index])->portBits) :
				&(gBWGrafPtr[index]->portBits), &(gTheWindow[index]->portBits),
				&gBoundsRect[index], &gBoundsRect[index], 0, 0L);
	
	if (gHasColorQD)
		UnlockPixels(GetGWorldPixMap(gTheGWorld[index]));	/* remember we locked these? */
	
	ValidRect(&(gTheWindow[index]->portRect));		/* so we don't reupdate */
}

Boolean CloseTheWindow(ExtendedWindowDataHandle theData)
{
	int				index;
	
	index=(**theData).windowIndex;
	
	/* if the window's dispatch cancels the close, abort */
	if (((**theData).dispatchProc)(theData, kClose, 0L)==kCancel)
		return FALSE;
	
	DisposeWindow(gTheWindow[index]);	/* get rid of the actual window in memory */
	gTheWindow[index]=0L;				/* so _we_ know the window doesn't exist */
	
	/* tell window's dispatch that it's disposed of now */
	((**theData).dispatchProc)(theData, kDispose, 0L);
	
	return TRUE;	/* successful close */
}

void DrawThePicture(PicHandle *thePict, int whichPict, int x, int y)
/* a standard routine for loading a picture (if necessary) and then drawing it */
{
	Rect			temp;
	
	if (*thePict==0L)		/* get it if it doesn't exist */
		*thePict=(PicHandle)GetPicture(whichPict);
	
	HLock(*thePict);		/* lock it down for dereferencing to get picture bounds */
	temp.top=y;
	temp.left=x;
	temp.bottom=temp.top+(***thePict).picFrame.bottom-(***thePict).picFrame.top;
	temp.right=temp.left+(***thePict).picFrame.right-(***thePict).picFrame.left;
	DrawPicture(*thePict, &temp);	/* draw picture at (x,y) */
	HUnlock(*thePict);		/* unlock for better memory management */
}

void ReleaseThePict(PicHandle *thePict)
{
	if (*thePict!=0L)	/* if exists, release it */
	{
		ReleaseResource(*thePict);
		*thePict=0L;	/* so _we_ know it's released */
	}
}

void ShutDownTheGraphics(void)
{
	int				i;
	
	/* send shutdown messages to the shell's windows */
	((**(gTheWindowData[kAbout])).dispatchProc)(gTheWindowData[kAbout], kShutdown, 0L);
	((**(gTheWindowData[kHelp])).dispatchProc)(gTheWindowData[kHelp], kShutdown, 0L);
	((**(gTheWindowData[kAboutMSG])).dispatchProc)(gTheWindowData[kAboutMSG], kShutdown, 0L);
	
	for (i=0; i<NUM_WINDOWS; i++)				/* technically, none of this cleanup */
	{											/* is necessary, since these are all */
		if (gTheGWorld[i]!=0L)					/* in the application heap, which is */
			DisposeGWorld(gTheGWorld[i]);		/* about to get trashed when we quit */
		if (gTheWindowData[i]!=0L)				/* anyway, but it's still good to Do */
			DisposeHandle(gTheWindowData[i]);	/* The Right Thing� even if it kills */
		if (gTheWindow[i]!=0L)					/* you in the process (:             */
			DisposeWindow(gTheWindow[i]);
	}
}
