// File "sample drag.c" -

#include <Drag.h>

#include "main.h"
#include "floaters.h"
#include "sample drag.h"
#include "sample win.h"

// ***********************************************************************************
// Global Declarations 

extern GlobalsRec glob;

// ***********************************************************************************
// ***********************************************************************************

void SetupDragHandlers(WindowPtr win) {
	static DragTrackingHandlerUPP trackingHandler=0;
	static DragReceiveHandlerUPP receiveHandler=0;

	if (! trackingHandler) trackingHandler = NewDragTrackingHandlerProc(SampleTrackingHandler);
	if (! receiveHandler) receiveHandler = NewDragReceiveHandlerProc(SampleReceiveHandler);
		
	if (trackingHandler && receiveHandler) {
		InstallTrackingHandler(trackingHandler, win, 0);
		InstallReceiveHandler(receiveHandler, win, 0);
		}
	}

// ***********************************************************************************
// ***********************************************************************************

short SampleWindowDrag(EventRecord *theEvent, WindowPtr win) {
	short err=0;
	Str255 textBuff;
	RgnHandle dragRgn;
	DragReference dragRef=0;
	DragAttributes attributes;
	
	// If the user just clicks and lets go, then return
	if (! WaitMouseMoved(theEvent->where)) return(0);
	
	// Setup the information we need to do a Drag
	err = NewDrag(&dragRef);
	if (err) return(err);

	dragRgn = NewDragRgn(win);
	if (!dragRgn) goto HANDLE_ERROR;

	// Add in some data - The contents of our window
	BlockMove(GetSampleWindowText(win), textBuff, sizeof(textBuff));
	err = AddDragItemFlavor(dragRef, 1, 'TEXT', textBuff+1, textBuff[0], 0);	


	// Initiate the dragging process, and handle any errors by escaping
	err = TrackDrag(dragRef, theEvent, dragRgn);
	if (err == userCanceledErr) err = 0;
	if (err) goto HANDLE_ERROR;
	
	GetDragAttributes(dragRef, &attributes);
	if (! (attributes & dragInsideSenderApplication)) {
		AEDesc dropLocation;

		GetDropLocation(dragRef, &dropLocation);

		AEDisposeDesc(&dropLocation);
		}

	
HANDLE_ERROR:
	if (dragRef) DisposeDrag(dragRef);
	if (dragRgn) DisposeRgn(dragRgn);
	return(err);
	}

// ***********************************************************************************
// ***********************************************************************************

pascal OSErr SampleTrackingHandler(DragTrackingMessage message, WindowPtr win,
		void *refcon, DragReference dragRef) {
	static Boolean acceptDrag, inContent;
	short err=0;
	unsigned short count;
	unsigned long flavorFlags;
	Size dataSize;
	Point localPt;
	GrafPtr savePort;
	RgnHandle hiliteRgn;
	ItemReference dragItem;
	DragAttributes attributes;
	
	GetDragAttributes(dragRef, &attributes);
	
	GetPort(&savePort);
	SetPort(win);
	
	switch (message) {
		case dragTrackingEnterHandler:
			acceptDrag = FALSE;
			inContent = FALSE;
			break;
			
		case dragTrackingEnterWindow:
			// Test *every* Drag Item to see if it contains text or a text file.
			//   (The sample code in the Drag Mgr dox suggested this, but I think
			//   that you could probably reorganize it for whatever your needs are.)
			if (CountDragItems(dragRef, &count)) count=0;
			for(count, acceptDrag = TRUE; count && acceptDrag; count--) {
				GetDragItemReferenceNumber(dragRef, count, &dragItem);
				
				if (! GetFlavorFlags(dragRef, dragItem, 'TEXT', &flavorFlags)) continue;
				if (! GetFlavorFlags(dragRef, dragItem, 'hfs ', &flavorFlags)) {
					HFSFlavor hfsDataBuff;
					
					dataSize = sizeof(hfsDataBuff);
					err = GetFlavorData(dragRef, dragItem, 'hfs ', (Ptr) &hfsDataBuff, &dataSize, 0L);
					if (! err && (hfsDataBuff.fileType == 'TEXT')) continue;
					}
				
				acceptDrag = FALSE;
				}
			break;

		case dragTrackingInWindow:
			if (! acceptDrag) break;
			
			GetDragMouse(dragRef, &localPt, 0L);
			GlobalToLocal(&localPt);
			
			if (attributes & dragHasLeftSenderWindow) {
				if (! inContent) {
					if (hiliteRgn = NewRgn()) {
						RectRgn(hiliteRgn, &win->portRect);
						ShowDragHilite(dragRef, hiliteRgn, TRUE);
						DisposeRgn(hiliteRgn);
						}
					inContent = TRUE;
					}
				}
			  else {
				if (inContent) {
					HideDragHilite(dragRef);
					inContent = FALSE;
					}
				}
			break;
		case dragTrackingLeaveWindow:
			if (acceptDrag && inContent) HideDragHilite(dragRef);
			acceptDrag = FALSE;
			inContent = FALSE;
			break;
		case dragTrackingLeaveHandler:
			break;
		}

	SetPort(savePort);
	}

// ***********************************************************************************
// ***********************************************************************************

pascal OSErr SampleReceiveHandler(WindowPtr win, void *refcon, DragReference dragRef) {
	Boolean handled;
	unsigned short index, count;
	unsigned long flavorFlags;
	Size dataSize;
	Str255 textBuff;
	GrafPtr savePort;
	ItemReference dragItem;
	DragAttributes attributes;

	// As above, we test every Drag Item to see if it contains text or text file. 
	//   However, we simply find and use the first match (as the new window contents).
	if (CountDragItems(dragRef, &count)) count=0;
	for(index=1, handled=FALSE; (index<=count) && !handled; index++) {
		GetDragItemReferenceNumber(dragRef, index, &dragItem);
		
		if (! GetFlavorFlags(dragRef, dragItem, 'TEXT', &flavorFlags)) {
			GetFlavorDataSize(dragRef, dragItem, 'TEXT', &dataSize);
			textBuff[0] = dataSize = (dataSize <= 255) ? dataSize : 255;
			GetFlavorData(dragRef, dragItem, 'TEXT', (Ptr) textBuff+1, &dataSize, 0L);
			handled = TRUE; continue;
			}
		
		if (! GetFlavorFlags(dragRef, dragItem, 'hfs ', &flavorFlags)) {
			short refNum, err=0;
			HFSFlavor hfsDataBuff;
			
			dataSize = sizeof(hfsDataBuff);
			err = GetFlavorData(dragRef, dragItem, 'hfs ', (Ptr) &hfsDataBuff, &dataSize, 0L);
			if (hfsDataBuff.fileType != 'TEXT') err=-1;	// Only look at text files
			if (! err) err = FSpOpenDF(&hfsDataBuff.fileSpec, fsRdPerm, &refNum);
			  else refNum = 0;
			
			dataSize = 255;
			if (! err) err = FSRead(refNum, &dataSize, textBuff+1);
			if (err == eofErr) err = 0;	// Short files are OK, ignore this error
			if (refNum) FSClose(refNum);
			
			if (! err) textBuff[0] = dataSize;
			if (! err) { handled = TRUE; continue; }
			}
		}
	
	if (handled) {
		// Update the window, then help our app handle the event quickly.
		//   (If you notice, we may be in someone else's context!)
		SetSampleWindowText(win, textBuff);
		WakeUpProcess(&glob.myPSN);
		}
	return((handled) ? 0 : -1);
	}

// ***********************************************************************************
// ***********************************************************************************

RgnHandle NewDragRgn(WindowPtr win) {
	Point convertPt;
	GrafPtr savePort;
	RgnHandle dragRgn=0, tempRgn=0;
	
	dragRgn = NewRgn();
	tempRgn = NewRgn();
	if (! dragRgn || ! tempRgn) return(0);
	
	// Build up a region that's 1 pixel thick, as per Drag Mgr dox
	RectRgn(dragRgn, &win->portRect);
	InsetRgn(dragRgn, 3, 3);
	CopyRgn(dragRgn, tempRgn);
	InsetRgn(tempRgn, 1, 1);
	DiffRgn(dragRgn, tempRgn, dragRgn);
	DisposeRgn(tempRgn);
	
	// Convert the coords of our rgn (ala PortRect) from Local to Global
	GetPort(&savePort);
	SetPort(win);
	SetPt(&convertPt, 0, 0);
	LocalToGlobal(&convertPt);
	SetPort(savePort);
	OffsetRgn(dragRgn, convertPt.h, convertPt.v);
	
	return(dragRgn);
	}
