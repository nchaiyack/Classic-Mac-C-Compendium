#ifndef __MY_OFFSCREEN_LAYER_H__
#define __MY_OFFSCREEN_LAYER_H__

#include <QDOffscreen.h>

#if PRAGMA_ALIGN_SUPPORTED
#pragma options align=mac68k
#endif
struct MyOffscreenRec
{
	GrafPtr			offscreenPtr;	/* cast to GWorldPtr if color */
	PixMapHandle	pixMapHandle;	/* color only */
	GWorldPtr		currentGWorld;	/* color only */
	GDHandle		currentGDHandle;/* color only */
	Ptr				bwBitMap;		/* B/W only */
	GrafPort		bwGrafPort;		/* B/W only */
	Boolean			isColor;		/* true if offscreenPtr is GWorldPtr & myPixMapHandle is valid */
};

typedef struct MyOffscreenRec	MyOffscreenRec, *MyOffscreenPtr;

struct MyOffscreenComboRec
{
	WindowRef		destWindow;
	MyOffscreenPtr	offscreenWorldPtr;
	MyOffscreenPtr	patternWorldPtr;
	MyOffscreenPtr	scratchWorldPtr;
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
};
#if PRAGMA_ALIGN_SUPPORTED
#pragma options align=reset
#endif

typedef struct MyOffscreenComboRec	MyOffscreenComboRec, *MyOffscreenComboPtr;

#ifdef __cplusplus
extern "C" {
#endif

extern	MyOffscreenComboPtr AllocateOffscreenCombo(WindowRef theWindow, Rect *sourceRect,
	Rect *destRect, short scrollSpeed, Handle theText, short font, short size);
extern	MyOffscreenPtr AllocatePartialOffscreenWorld(WindowRef theWindow, Rect *boundsRect,
	Boolean setPort, Boolean *depthChanged);
extern	MyOffscreenPtr AllocateOffscreenWorld(WindowRef theWindow, Boolean *depthChanged);
extern	void CopybitsCombo(MyOffscreenComboPtr offscreenComboPtr);
extern	void ScrollTextCombo(MyOffscreenComboPtr offscreenComboPtr);
extern	void ChangeDepthOffscreenWindow(WindowPtr theWindow, MyOffscreenPtr *offscreenWorldPtr,
	Boolean *depthChanged);
extern	void SetPortToOnscreenWindow(WindowRef theWindow, MyOffscreenPtr offscreenWorldPtr);
extern	void SetPortToOffscreenWindow(MyOffscreenPtr offscreenWorldPtr);
extern	MyOffscreenPtr DisposeOffscreenWorld(MyOffscreenPtr offscreenWorldPtr);
extern	MyOffscreenComboPtr DisposeOffscreenCombo(MyOffscreenComboPtr offscreenComboPtr);

#ifdef __cplusplus
}
#endif

#endif
