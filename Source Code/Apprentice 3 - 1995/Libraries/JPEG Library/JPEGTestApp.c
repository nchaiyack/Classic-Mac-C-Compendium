//=====================================================================================
// JPEGUtilities.c -- written by Aaron Giles
// Last update: 7/7/94
//=====================================================================================
// A very simple, poorly-coded demo application which opens JPEG and PICT images in a
// window, then converts from one format to the other, displaying the result in a 
// second window.  No real user interface is presented.  After displaying the first
// image, click the mouse button to perform the JPEG<->PICT conversion and view the
// resulting image.  Click the mouse button again to quit.
//=====================================================================================
// This code has been successfully compiled under Metrowerks C/C++ 1.0a4, under
// THINK C 7.0, and under MPW C 3.3.
//=====================================================================================
// If you find any bugs in this source code, please email me and I will attempt to fix
// them.  If you have any additions/modifications that you think would be generally
// useful, email those to me as well, and I will consider incorporating them into the
// next release.  My email address is giles@med.cornell.edu.
//=====================================================================================
// This source code is copyright © 1994, Aaron Giles.  Permission to use this code in
// your product is freely granted, provided that you credit me appropriately in your
// application's About box/credits *and* documentation.  If you ship an application
// which uses this code, I would also like to request that you provide me with one
// complimentary copy of the application.
//=====================================================================================

#include <Errors.h>
#include <Fonts.h>
#include <Memory.h>
#include <QuickDraw.h>
#include <StandardFile.h>
#include <Types.h>

#include "JPEGUtilities.h"

void main(void);
OSErr ReadData(StandardFileReply *reply, Handle *theData);
OSErr DrawImageInWindow(Handle theData, OSType theType);

void main(void)
{
	OSType theTypes[] = { 'PICT', 'JPEG' };
	StandardFileReply reply;
	Handle theData, newData;
	OSErr theErr;

	InitGraf(&qd.thePort);
	InitFonts();
	InitWindows();
	InitMenus();
	TEInit();
	InitDialogs(nil);
	InitCursor();

	StandardGetFile(nil, 2, theTypes, &reply);
	if (reply.sfGood) {
		theErr = ReadData(&reply, &theData);
		if (theErr == noErr) theErr = DrawImageInWindow(theData, reply.sfType);
		if (theErr == noErr) {
			while (!Button());
			if (reply.sfType == 'PICT') {
				newData = UnwrapJPEG((PicHandle)theData);
				reply.sfType = 'JPEG';
			} else {
				newData = (Handle)WrapJPEG(theData);
				reply.sfType = 'PICT';
			}
			if (newData) {
				theErr = DrawImageInWindow(newData, reply.sfType);
				while (!Button());
			} else SysBeep(1);
		}
	}
}
		
OSErr ReadData(StandardFileReply *reply, Handle *theData)
{
	short refNum;
	OSErr theErr;
	long size;
	
	theErr = FSpOpenDF(&reply->sfFile, fsRdPerm, &refNum);
	if (theErr == noErr) {
		theErr = GetEOF(refNum, &size);
		if (theErr == noErr && reply->sfType == 'PICT') {
			size -= 512;
			theErr = SetFPos(refNum, fsFromStart, 512);
		}
		if (theErr == noErr) {
			if (*theData = NewHandle(size)) {
				HLock(*theData);
				theErr = FSRead(refNum, &size, **theData);
				HUnlock(*theData);
			} else theErr = memFullErr;
		}
		FSClose(refNum);
	}
	return theErr;
}

OSErr DrawImageInWindow(Handle theData, OSType theType)
{
	OSErr theErr = noErr;
	WindowPtr theWindow;
	Rect bounds;
	
	if (theType == 'PICT') bounds = (*(PicHandle)theData)->picFrame;
	else theErr = GetJPEGBounds(theData, &bounds);
	if (theErr == noErr) {
		OffsetRect(&bounds, 0, 39);
		theWindow = NewCWindow(nil, &bounds,
					(theType == 'PICT') ? (StringPtr)"\pPICT Image" : (StringPtr)"\pJPEG Image",
					true, noGrowDocProc, (WindowPtr)-1, false, 0);
		if (theWindow) {
			SetPort(theWindow);
			OffsetRect(&bounds, 0, -39);
			ClipRect(&bounds);
			if (theType == 'PICT') DrawPicture((PicHandle)theData, &bounds);
			else theErr = DrawJPEG(theData, &bounds, &bounds, ditherCopy);
		}
	}
	return theErr;
}
