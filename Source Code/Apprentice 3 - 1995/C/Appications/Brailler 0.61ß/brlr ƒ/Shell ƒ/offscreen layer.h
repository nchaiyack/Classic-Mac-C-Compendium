#ifndef __MY_OFFSCREEN_LAYER_H__
#define __MY_OFFSCREEN_LAYER_H__

#include <QDOffscreen.h>

typedef struct
{
	GrafPtr			offscreenPtr;	/* cast to GWorldPtr if color */
	PixMapHandle	pixMapHandle;	/* color only */
	GWorldPtr		currentGWorld;	/* color only */
	GDHandle		currentGDHandle;/* color only */
	Ptr				bwBitMap;		/* B/W only */
	GrafPort		bwGrafPort;		/* B/W only */
	Boolean			isColor;		/* true if offscreenPtr is GWorldPtr & myPixMapHandle is valid */
} MyOffscreenRec, *MyOffscreenPtr, **MyOffscreenHandle;

typedef struct
{
	WindowPtr		destWindow;
	MyOffscreenRec	offscreenWorldRec;
	MyOffscreenRec	patternWorldRec;
	MyOffscreenRec	scratchWorldRec;
	Rect			sourceRect;
	Rect			destRect;
	short			scrollSpeed;
	short			scrollXPos;
	short			font;
	short			size;
	unsigned long	offset;
	Rect			scrollSourceRect;
	Rect			scrollDestRect;
	Handle			theText;
} MyOffscreenComboRec, *MyOffscreenComboPtr, **MyOffscreenComboHandle;

MyOffscreenComboPtr AllocateOffscreenCombo(WindowPtr theWindow, Rect *sourceRect, Rect *destRect,
	short scrollSpeed, Handle theText, short font, short size);
OSErr AllocatePartialOffscreenWorld(WindowPtr theWindow, MyOffscreenPtr offscreenWorldPtr,
	Rect *boundsRect, Boolean setPort, Boolean *depthChanged);
OSErr AllocateOffscreenWorld(WindowPtr theWindow, MyOffscreenPtr offscreenWorldPtr,
	Boolean *depthChanged);
void CopybitsCombo(MyOffscreenComboPtr offscreenComboPtr);
void ScrollTextCombo(MyOffscreenComboPtr offscreenComboPtr);
void SetPortToOnscreenWindow(WindowPtr theWindow, MyOffscreenPtr offscreenWorldPtr);
void SetPortToOffscreenWindow(MyOffscreenPtr offscreenWorldPtr);
void DisposeOffscreenWorld(MyOffscreenPtr offscreenWorldPtr);
MyOffscreenComboPtr DisposeOffscreenCombo(MyOffscreenComboPtr offscreenComboPtr);

#endif
