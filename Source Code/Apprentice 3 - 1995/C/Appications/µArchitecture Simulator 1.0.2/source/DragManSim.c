/*
Copyright й 1993,1994,1995 Fabrizio Oddone
еее еее еее еее еее еее еее еее еее еее
This source code is distributed as freeware:
you may copy, exchange, modify this code.
You may include this code in any kind of application: freeware,
shareware, or commercial, provided that full credits are given.
You may not sell or distribute this code for profit.
*/
#ifndef __DRAG__
#include <Drag.h>
#endif

#include	"Assembler.h"
#include	"CursorBalloon.h"
#include	"DragManSim.h"
#include	"Globals.h"
#include	"myMemory.h"

typedef struct myglobptr {
	Boolean	canAcceptDrag;
	Boolean	inContent;
	} GlobalsData, *GlobalsPtr;

GlobalsData		gmyGlobals;

static pascal OSErr MyTrackingHandler(DragTrackingMessage theMessage, WindowPtr theWindow,
						void *handlerRefCon, DragReference theDrag);
static Boolean IsMyTypeAvailable(WindowPtr theWindow, DragReference theDrag);
static pascal OSErr MyReceiveHandler(WindowPtr theWindow, void *handlerRefCon,
						DragReference theDrag);


/*
OSErr MyInitDragManager(void)
{
OSErr	err;

err = InstallTrackingHandler(MyDefaultTrackingHandler, 0L, &myGlobals);
if (err == noErr)
	err = InstallReceiveHandler(MyDefaultReceiveHandler, 0L, &myGlobals);

return err;
}
*/

OSErr MyInstallHWindow(WindowPtr theWindow)
{
OSErr       err;

((FabWindowPtr)theWindow)->trackUPP = NewDragTrackingHandlerProc(MyTrackingHandler);
((FabWindowPtr)theWindow)->recUPP = NewDragReceiveHandlerProc(MyReceiveHandler);

if ((err = InstallTrackingHandler(((FabWindowPtr)theWindow)->trackUPP, theWindow, &gmyGlobals)) == noErr) {
	err = InstallReceiveHandler(((FabWindowPtr)theWindow)->recUPP, theWindow, &gmyGlobals);
	if (err)
		RemoveTrackingHandler(((FabWindowPtr)theWindow)->trackUPP, theWindow);
	}
else
	DebugStr("\pMyInstallHWindow error on InstallTrackingHandler");

return err;
}

void MyRemoveHWindow(WindowPtr theWindow)
{
(void) RemoveTrackingHandler(((FabWindowPtr)theWindow)->trackUPP, theWindow);
(void) RemoveReceiveHandler(((FabWindowPtr)theWindow)->recUPP, theWindow);
if (((FabWindowPtr)theWindow)->trackUPP) {
	DisposeRoutineDescriptor(((FabWindowPtr)theWindow)->trackUPP);
	((FabWindowPtr)theWindow)->trackUPP = nil;
	}
if (((FabWindowPtr)theWindow)->recUPP) {
	DisposeRoutineDescriptor(((FabWindowPtr)theWindow)->recUPP);
	((FabWindowPtr)theWindow)->recUPP = nil;
	}
}

pascal OSErr MyTrackingHandler(DragTrackingMessage theMessage, WindowPtr theWindow,
						void *handlerRefCon, DragReference theDrag)
{
Rect	tempRect = {0, 0, 0, 0};
GlobalsPtr		myGlobals = (GlobalsPtr) handlerRefCon;
Point			mouse, localMouse;
void (*theProc)(WindowPtr, RectPtr);
DragAttributes	attributes;
RgnHandle		hiliteRgn;
OSErr	err;

if ((err = GetDragAttributes(theDrag, &attributes)) == noErr) {
	switch(theMessage) {
//		case dragTrackingEnterHandler:
//			break;
		case dragTrackingEnterWindow:
			myGlobals->canAcceptDrag = IsMyTypeAvailable(theWindow, theDrag);
			myGlobals->inContent = false;
			break;
		case dragTrackingInWindow:
			if (myGlobals->canAcceptDrag) {
				if ((err = GetDragMouse(theDrag, &mouse, 0L)) == noErr) {
					localMouse = mouse;
					GlobalToLocal(&localMouse);
					if (attributes & dragHasLeftSenderWindow) {
						theProc = ((FabWindowPtr)theWindow)->getDragHiliteRectProc;
						if (theProc)
							theProc(theWindow, &tempRect);
						if (PtInRect(localMouse, &tempRect)) {
							if (myGlobals->inContent == false) {
								hiliteRgn = NewRgn();
								if (hiliteRgn) {
									//InsetRect(&tempRect, 1, 1);
									RectRgn(hiliteRgn, &tempRect);
									(void) ShowDragHilite(theDrag, hiliteRgn, true);
									DisposeRgn(hiliteRgn);
									}
								myGlobals->inContent = true;
								}
							}
						else {
							if (myGlobals->inContent) {
								(void) HideDragHilite(theDrag);
								myGlobals->inContent = false;
								}
							}
						}
					}
	//			MyTrackItemUnderMouse(localMouse, theWindow);
				}
	        break;
		case dragTrackingLeaveWindow:
			if (myGlobals->canAcceptDrag && myGlobals->inContent) {
				(void) HideDragHilite(theDrag);
				}
	        myGlobals->canAcceptDrag = false;
	        break;
//	    case dragTrackingLeaveHandler:
//	        break;
		}
	}
return err;
}

Boolean IsMyTypeAvailable(WindowPtr theWindow, DragReference theDrag)
{
//HVolumeParam	mypb;
FSSpec			copySpec;
HFSFlavor		*theData;
//AliasHandle		tempFolder;
Size			dataSize;
//FlavorFlags		theFlags;
ItemReference	theItem;
unsigned short	items, index;
OSErr			err;
Boolean			targetFolder, isAnAlias;
Boolean			retVal = true;


if ((err = CountDragItems(theDrag, &items)) == noErr) {
	for (index = 1; (index <= items) && retVal; index++) {
		if ((err = GetDragItemReferenceNumber(theDrag, index, &theItem)) == noErr) {
//			if ((err = GetFlavorFlags(theDrag, theItem, flavorTypeHFS, &theFlags)) == noErr) {
				if ((err = GetFlavorDataSize(theDrag, theItem, flavorTypeHFS, &dataSize)) == noErr) {
					theData = (HFSFlavor *)NewPtr(dataSize);
					if (theData) {
						if ((err = GetFlavorData(theDrag, theItem, flavorTypeHFS, theData, &dataSize, 0L)) == noErr) {
							switch (theData->fileType) {
								case kFTY_REG:
									retVal = theWindow == gWPtr_Registers;
									break;
								case kFTY_RAM:
								case 'TEXT':
									retVal = theWindow == gWPtr_Disasm || theWindow == gWPtr_Dump;
									break;
								default:
									retVal = false;
								}
							if (retVal) {
								copySpec = theData->fileSpec;
								if (err = ResolveAliasFile(&copySpec, true, &targetFolder, &isAnAlias))
									retVal = false;
								}
							}
						else
							retVal = false;
						DisposePtr((Ptr)theData);
						}
					else
						retVal = false;
					}
				else
					retVal = false;
//				}
//			else
//				retVal = false;
			}
		else
			retVal = false;
		}
	}
else
	retVal = false;
return retVal;
}

pascal OSErr MyReceiveHandler(WindowPtr theWindow, void *handlerRefCon,
						DragReference theDrag)
{
GlobalsPtr		myGlobals = (GlobalsPtr) handlerRefCon;
//HVolumeParam	mypb;
FSSpec			copySpec;
HFSFlavor		*theData;
//AliasHandle		tempFolder;
//Point			mouse;
ItemReference	theItem;
//FlavorFlags		theFlags;
Size			dataSize;
unsigned short	items, index;
OSErr			err = noErr;
Boolean			targetFolder, isAnAlias;

//if ((err = GetDragMouse(theDrag, &mouse, 0L)) == noErr) {
if (myGlobals->canAcceptDrag && myGlobals->inContent) {
	if ((err = CountDragItems(theDrag, &items)) == noErr) {
		for (index = 1; (index <= items) && (err == noErr); index++) {
			if ((err = GetDragItemReferenceNumber(theDrag, index, &theItem)) == noErr) {
//				if ((err = GetFlavorFlags(theDrag, theItem, flavorTypeHFS, &theFlags)) == noErr) {
					if ((err = GetFlavorDataSize(theDrag, theItem, flavorTypeHFS, &dataSize)) == noErr) {
						theData = (HFSFlavor *)NewPtr(dataSize);
						if (theData) {
							if ((err = GetFlavorData(theDrag, theItem, flavorTypeHFS, theData, &dataSize, 0L)) == noErr) {
								copySpec = theData->fileSpec;
								if ((err = ResolveAliasFile(&copySpec, true, &targetFolder, &isAnAlias)) == noErr) {
									switch (theData->fileType) {
										case kFTY_REG:
											err = OpenProcessorState(&copySpec);
											UnloadSeg(OpenProcessorState);
											break;
										case kFTY_RAM:
											err = myOpenFile(&copySpec, gMMemory, kSIZE_RAM);
											UnloadSeg(myOpenFile);
											break;
										case 'TEXT':
											err = myAsmFile(&copySpec);
											UnloadSeg(myAsmFile);
											break;
										default:
											err = dragNotAcceptedErr;
										}
									}
								}
							DisposePtr((Ptr)theData);
							}
						else
							err = dragNotAcceptedErr;
						}
//					}
				}
			}
		}
	}
else
	err = dragNotAcceptedErr;

if (err)
	err = dragNotAcceptedErr;
return err;
}

