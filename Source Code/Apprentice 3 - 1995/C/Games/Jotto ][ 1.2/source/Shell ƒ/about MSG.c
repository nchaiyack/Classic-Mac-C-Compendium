#include "about MSG.h"
#include "styled text.h"
#include "pict utilities.h"
#include "window layer.h"
#include "graphics.h"

/*-----------------------------------------------------------------------------------*/
/* internal stuff for about MSG.c                                                    */

static	void DoTheMSGThing(WindowPtr theWindow);
static	void AboutMSGEventLoop(WindowPtr theWindow);

static	PicHandle	gAboutPict=0L;
static	CharHandle	gAboutText=0L;
static	StylHandle	gAboutStyle=0L;

static	Ptr			gOffscreenBitMap=0L;
static	GrafPort	gOffscreenGrafPort;
static	GrafPtr		gOffscreenGrafPtr=0L;

static	Boolean		gSetupDone=FALSE;

#define MAX_OFFSCREEN_LINES	5
#define kOffscreenWidth		130
#define kOffscreenHeight	((MAX_OFFSCREEN_LINES*12)+1)
#define RIGHT_SLOP			13
#define TOP_SLOP			29

#define	kAboutTextID		150

void SetupTheAboutMSGWindow(WindowPtr theWindow)
{
	long			offRowBytes, sizeOfOff;
	unsigned char	*titleStr="\p";
	
	SetWindowWidth(theWindow, 324);
	SetWindowHeight(theWindow, 199);
	SetWindowType(theWindow, plainDBox);
	SetWindowTitle(theWindow, titleStr);
	SetWindowHasCloseBox(theWindow, FALSE);
	SetWindowMaxDepth(theWindow, 8);
	SetWindowDepth(theWindow, 8);
	SetWindowIsFloat(theWindow, FALSE);
	SetWindowAutoCenter(theWindow, TRUE);
	
	if (gSetupDone)
		return;
	
	gAboutText=(CharHandle)GetResource('TEXT', kAboutTextID);
	gAboutStyle=(StylHandle)GetResource('styl', kAboutTextID);

	gOffscreenGrafPtr=&gOffscreenGrafPort;
	OpenPort(&gOffscreenGrafPort);	/* make a new port */
	
	/* calculate the size of the offscreen bitmap from the boundsrect */
	offRowBytes=((GetWindowWidth(theWindow)+15)>>4)<<1;
	sizeOfOff=(long)(GetWindowHeight(theWindow)*offRowBytes);
	
	gOffscreenBitMap=NewPtr(sizeOfOff);		/* allocate space for bitmap */
	
	gOffscreenGrafPort.portBits.baseAddr=gOffscreenBitMap;	/* --> our bitmap */
	gOffscreenGrafPort.portBits.rowBytes=offRowBytes;		/* bitmap size */
	SetRect(&gOffscreenGrafPort.portBits.bounds, 0, 0, kOffscreenWidth, kOffscreenHeight);
	gOffscreenGrafPort.portRect=gOffscreenGrafPort.portBits.bounds;
	
	SetPort(gOffscreenGrafPtr);
	
	gSetupDone=TRUE;
}

void ShutDownTheAboutMSGWindow(void)
{
	gAboutPict=ReleaseThePict(gAboutPict);
	if (gOffscreenBitMap!=0L)
	{
		DisposePtr(gOffscreenBitMap);
		ClosePort(&gOffscreenGrafPort);
	}
}

void OpenTheMSGWindow(WindowPtr theWindow)
{
	UpdateTheWindow(theWindow);
	AboutMSGEventLoop(theWindow);
}

void AboutMSGEventLoop(WindowPtr theWindow)
{
	GrafPtr			oldPort;
	GrafPtr			curPort;
	short			startLine, numLines;
	short			i;
	Rect			sourceRect, destRect;
	Rect			mirrorSourceRect, mirrorDestRect;
	Rect			newSourceRect, newDestRect;
	long			dummy;
	RgnHandle		scrollRgn;
	Rect			ovalRect;
	Boolean			notDoneYet;
	
	GetPort(&oldPort);
	SetPort(curPort=(GrafPtr)theWindow);
	startLine=0;
	numLines=CountLines(gAboutText, gAboutStyle, kCenter, srcXor, gOffscreenGrafPort.portRect);
	SetRect(&sourceRect, curPort->portRect.right-RIGHT_SLOP-kOffscreenWidth,
		TOP_SLOP, curPort->portRect.right-RIGHT_SLOP,
		(curPort->portRect.bottom-curPort->portRect.top)/2);
	OffsetRect(&sourceRect, 0, 1);
	destRect=sourceRect;
	OffsetRect(&destRect, 0, -1);
	mirrorSourceRect=sourceRect;
	mirrorSourceRect.top=sourceRect.bottom-1;
	mirrorSourceRect.bottom=sourceRect.bottom-sourceRect.top+mirrorSourceRect.top;
	mirrorDestRect=mirrorSourceRect;
	OffsetRect(&mirrorDestRect, 0, 1);
	newDestRect=sourceRect;
	newDestRect.top=newDestRect.bottom-1;
	SetRect(&ovalRect, 173, 28, 315, 171);
	scrollRgn=NewRgn();
	OpenRgn();
		FrameOval(&ovalRect);
	CloseRgn(scrollRgn);
	notDoneYet=TRUE;
	
	while (notDoneYet)
	{
		SetPort(gOffscreenGrafPtr);
		FillRect(&(gOffscreenGrafPtr->portRect), &qd.black);
		DrawThePartialText(gAboutText, gAboutStyle, kCenter, srcXor, gOffscreenGrafPort.portRect,
			startLine, startLine+MAX_OFFSCREEN_LINES+1);
		SetPort(curPort);
		
		newSourceRect.top=newSourceRect.left=0;
		newSourceRect.right=kOffscreenWidth;
		newSourceRect.bottom=1;
		for (i=0; ((i<kOffscreenHeight) && ((notDoneYet=(!Button()))==TRUE)); i++)
		{
			CopyBits(&(curPort->portBits), &(curPort->portBits), &sourceRect, &destRect, 0,
				scrollRgn);
			CopyBits(&(curPort->portBits), &(curPort->portBits), &mirrorSourceRect,
				&mirrorDestRect, 0, scrollRgn);
			CopyBits(&(gOffscreenGrafPort.portBits), &(curPort->portBits), &newSourceRect,
				&newDestRect, 0, 0L);
			newSourceRect.top++;
			newSourceRect.bottom++;
			Delay(7, &dummy);
		}
		
		startLine+=MAX_OFFSCREEN_LINES;
		if (startLine>numLines)
			startLine=0;
	}
	
	DisposeRgn(scrollRgn);
	SetPort(oldPort);
	FlushEvents(mouseDown+mouseUp+keyDown+keyUp+autoKey, 0);
	
	CloseTheWindow(theWindow);
}

void DrawTheAboutMSGWindow(WindowPtr theWindow, short theDepth)
{
	short			h, w, i;
	Rect			sourceRect, destRect;
	GrafPtr			curPort;
	
	gAboutPict=DrawThePicture(gAboutPict, (theDepth>2) ? 128 : 129, 0, 0);
	if (gAboutPict==0L)
		return;
	
	h=(**gAboutPict).picFrame.bottom-(**gAboutPict).picFrame.top;
	w=(**gAboutPict).picFrame.right-(**gAboutPict).picFrame.left;
	GetPort(&curPort);
	
	SetRect(&sourceRect, 0, 0, w, 1);
	SetRect(&destRect, 0, 2*h-2, w, 2*h-1);
	for (i=0; i<h; i++)
	{
		CopyBits(&(curPort->portBits), &(curPort->portBits), &sourceRect, &destRect, 0, 0L);
		sourceRect.top++;
		sourceRect.bottom++;
		destRect.top--;
		destRect.bottom--;
	}
}

void ChangeDepthTheAboutMSGWindow(WindowPtr theWindow, short newDepth)
{
	gAboutPict=ReleaseThePict(gAboutPict);
}
