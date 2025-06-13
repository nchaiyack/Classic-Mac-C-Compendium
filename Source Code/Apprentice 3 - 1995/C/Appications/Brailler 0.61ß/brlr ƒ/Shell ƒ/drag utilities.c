/* Note: dragging will not work properly if you compile and run this for native PowerMac.  I
   am working on figuring out what I'm doing wrong.  -MP 1/23/95 */

#include "drag utilities.h"
#include "graphics.h"
#include "graphics dispatch.h"
#include "window layer.h"
#include "text twiddling.h"
#include <Folders.h>
#include <GestaltEQU.h>

static	long		caretTime;
static	short		caretOffset, caretShow, lastOffset, insertPosition, canAcceptItems;
static	short		cursorInContent;
static	Boolean		gDragManagerInstalled;

/*
 *	The following #define statement provides access to TextEdit's caretTime.
 */

#define	gCaretTime		((short) *((long *) 0x02F4))

void InitTheDragManager(void)
{
	long			gestaltResponse;
	
	gDragManagerInstalled=TRUE;
	if ((Gestalt(gestaltDragMgrAttr, &gestaltResponse) != noErr) ||
		(!(gestaltResponse & (1 << gestaltDragMgrPresent))))
	{
		gDragManagerInstalled=FALSE;
	}
}

Boolean DragManagerAvailableQQ(void)
{
	return gDragManagerInstalled;
}

/*
 *	TEIsFrontOfLine, given an offset and a TextEdit handle, returns true if
 *	the given offset is at the beginning of a line start.
 */

short TEIsFrontOfLine(short offset, TEHandle theTE)
{
	short		line = 0;

	if ((**theTE).teLength == 0)
		return(true);

	if (offset >= (**theTE).teLength)
		return( (*((**theTE).hText))[(**theTE).teLength - 1] == 0x0d );

	while ((**theTE).lineStarts[line] < offset)
		line++;

	return( (**theTE).lineStarts[line] == offset );
}

/*
 *	TEGetLine, given an offset and a TextEdit handle, returns the line number
 *	of the line that contains the offset.
 */

short TEGetLine(short offset, TEHandle theTE)
{
	short		line = 0;

	if (offset > (**theTE).teLength)
		return((**theTE).nLines);

	while ((**theTE).lineStarts[line] < offset)
		line++;
	
	return(line);
}

/*
 *	Given a point in global coordinates, HitTest returns a pointer to a
 *	document structure if the point is inside a document window on the screen.
 *	If the point is not inside a document window, HitTest return NULL in
 *	theDoc. If the point is in a doument window and also in the viewRect of
 *	the document's TextEdit field, HitTest also returns the offset into
 *	the text that corresponds to that point. If the point is not in the text,
 *	HitTest returns the current selection end.
 */

short HitTest(Point theLoc, WindowPtr *theWindow)
{
	short			offset;
	TEHandle		hTE;
	
	*theWindow=0L;
	offset = -1;

	if (FindWindow(theLoc, theWindow) == inContent)
	{
		SetPort(*theWindow);
		GlobalToLocal(&theLoc);
		hTE=GetWindowTE(*theWindow);
		
		if (PtInRect(theLoc, &((**hTE).viewRect)))
		{
			offset=TEGetOffset(theLoc, hTE);
			if ((TEIsFrontOfLine(offset, hTE)) && (offset) &&			
					((*((**hTE).hText))[offset - 1] != 0x0D) &&
					(TEGetPoint(offset - 1, hTE).h < theLoc.h)) {
				offset--;
			}
		}
		else offset=(**hTE).selEnd;
	}

	return offset;
}


/*
 *	DrawCaret draws a caret in a TextEdit field at the given offset. DrawCaret
 *	expects the port to be set to the port that the TextEdit field is in.
 *	DrawCaret inverts the image of the caret onto the screen.
 */

void DrawCaret(short offset, TEHandle theTE)
{
	Point		theLoc;
	short		theLine, lineHeight;

	/*
	 *	Get the coordinates and the line of the offset to draw the caret.
	 */

	theLoc  = TEGetPoint(offset, theTE);
	theLine = TEGetLine(offset, theTE);

	/*
	 *	For some reason, TextEdit dosen't return the proper coordinates
	 *	of the last offset in the field if the last character in the record
	 *	is a carriage return. TEGetPoint returns a point that is one line
	 *	higher than expected. The following code fixes this problem.
	 */

	if ((offset == (**theTE).teLength) &&
			(*((**theTE).hText))[(**theTE).teLength - 1] == 0x0D) {
		theLoc.v += TEGetHeight(theLine, theLine, theTE);
	}

	/*
	 *	Always invert the caret when drawing.
	 */

	PenMode(patXor);

	/*
	 *	Get the height of the line that the offset points to.
	 */

	lineHeight = TEGetHeight(theLine, theLine, theTE);

	/*
	 *	Draw the appropriate caret image.
	 */

	MoveTo(theLoc.h - 1, theLoc.v - 1);
	Line(0, 1 - lineHeight);

	PenNormal();
}

#if USE_STYLED_TEXT
void InsertTextAtOffset(short offset, char *theBuf, long size, StScrpHandle theStyl, TEHandle theTE)
#else
void InsertTextAtOffset(short offset, char *theBuf, long size, TEHandle theTE)
#endif
{
	if (size == 0)
		return;

	TESetSelect(offset, offset, theTE);
#if USE_STYLED_TEXT
	TEStylInsert(theBuf, size, theStyl, theTE);
#else
	TEInsert(theBuf, size, theTE);
#endif
	TESetSelect(offset, offset + size, theTE);
}

short GetSelectionSize(WindowPtr theWindow)
{
	TEHandle		hTE;
	
	hTE=GetWindowTE(theWindow);
	return((**hTE).selEnd - (**hTE).selStart);
}

Ptr GetSelectedTextPtr(WindowPtr theWindow)
{
	TEHandle		hTE;
	
	hTE=GetWindowTE(theWindow);
	return((*(**hTE).hText) + (**hTE).selStart);
}

#if USE_STYLED_TEXT
/*
 *	MySendDataProc
 *
 *	Will provide 'styl' data for the drag when requested.
 */

pascal OSErr MySendDataProc(FlavorType theType, void *refCon,
							ItemReference theItem, DragReference theDrag)
{
	WindowPtr		theWindow=(WindowPtr)refCon;
	StScrpHandle	theStyl;
	TEHandle		hTE;
	
	hTE=GetWindowTE(theWindow);
	if (theType == 'styl')
	{
		theStyl = GetStylScrap(hTE);

		//
		//	Call SetDragItemFlavorData to provide the requested data.
		//

		HLock((Handle) theStyl);
		SetDragItemFlavorData(theDrag, theItem, 'styl', (Ptr) *theStyl,
							  GetHandleSize((Handle) theStyl), 0L);
		HUnlock((Handle) theStyl);
		DisposeHandle((Handle) theStyl);

	} else {

		return(badDragFlavorErr);

	}

	return(noErr);
}
#endif

/*
 *	MyReceiveDropHandler
 *
 *	Called by the Drag Manager when a drop occurs over one of the document windows.
 */

pascal OSErr MyReceiveDropHandler(WindowPtr duhWindow, unsigned long handlerRefCon,
								  DragReference theDrag)
{	OSErr			result;
	TEHandle		tempTE;
	Rect			theRect;
	unsigned short	items, index;
	ItemReference	theItem;
	DragAttributes	attributes;
	Ptr				textData;
	Size			textSize;
#if USE_STYLED_TEXT
	Size			stylSize;
	StScrpHandle	stylHandle;
#endif
	short			offset, selStart, selEnd, mouseDownModifiers, mouseUpModifiers, moveText;
	WindowPtr		theWindow;
	TEHandle		hTE;
	
	if ((!canAcceptItems) || (insertPosition == -1))
		return(dragNotAcceptedErr);

	SetPort(duhWindow);
	theWindow=(WindowPtr)handlerRefCon;
	hTE=GetWindowTE(theWindow);
	
	GetDragAttributes(theDrag, &attributes);
	GetDragModifiers(theDrag, 0L, &mouseDownModifiers, &mouseUpModifiers);

	moveText = (attributes & dragInsideSenderWindow) &&
			   (!((mouseDownModifiers & optionKey) | (mouseUpModifiers & optionKey)));

	//
	//	Loop through all of the drag items contained in this drag and collect the text
	//	into the tempTE record.
	//

	SetRect(&theRect, 0, 0, 0, 0);
#if USE_STYLED_TEXT
	tempTE = TEStylNew(&theRect, &theRect);
#else
	tempTE = TENew(&theRect, &theRect);
#endif

	CountDragItems(theDrag, &items);

	for (index = 1; index <= items; index++)
	{
		//
		//	Get the item's reference number, so we can refer to it.
		//

		GetDragItemReferenceNumber(theDrag, index, &theItem);

		//
		//	Try to get the flags for a 'TEXT' flavor. If this returns noErr,
		//	then we know that a 'TEXT' flavor exists in the item.
		//

		result = GetFlavorDataSize(theDrag, theItem, 'TEXT', &textSize);

		if (result == noErr)
		{

			textData = NewPtr(textSize);
			if (textData == 0L)
			{
				TEDispose(tempTE);
				return(memFullErr);
			}

			GetFlavorData(theDrag, theItem, 'TEXT', textData, &textSize, 0L);

#if USE_STYLED_TEXT
			//
			//	Check for optional styl data for the TEXT.
			//

			stylHandle = 0L;
			result = GetFlavorDataSize(theDrag, theItem, 'styl', &stylSize);
			if (result == noErr) {

				stylHandle = (StScrpHandle) NewHandle(stylSize);
				if (stylHandle == 0L) {
					TEDispose(tempTE);
					DisposePtr(textData);
					return(memFullErr);
				}

				HLock((Handle) stylHandle);
				GetFlavorData(theDrag, theItem, 'styl', *stylHandle, &stylSize, 0L);
				HUnlock((Handle) stylHandle);

			}
#endif

			//
			//	Insert this drag item's text into the tempTE.
			//

			TESetSelect(32767, 32767, tempTE);
#if USE_STYLED_TEXT
			TEStylInsert(textData, textSize, stylHandle, tempTE);
#else
			TEInsert(textData, textSize, tempTE);
#endif
			DisposePtr(textData);
#if USE_STYLED_TEXT
			if (stylHandle)
				DisposeHandle((Handle) stylHandle);
#endif
		}
	}

	//
	//	Pull the TEXT and styl data out of the tempTE handle.
	//

	textData = NewPtr(textSize = (**tempTE).teLength);
	if (textData == 0L) {
		TEDispose(tempTE);
		return(memFullErr);
	}
	BlockMove(*(**tempTE).hText, textData, textSize);

#if USE_STYLED_TEXT
	TESetSelect(0, 32767, tempTE);
	stylHandle = GetStylScrap(tempTE);
#endif

	TEDispose(tempTE);

	/* if we got any text, insert it */
	if (textSize != 0)
	{
		/* remove caret and/or highlighting if necessary */
		offset = caretOffset;
		if (caretOffset != -1)
		{
			DrawCaret(caretOffset, hTE);
			caretOffset = -1;
		}
		if (attributes & dragHasLeftSenderWindow)
		{
			HideDragHilite(theDrag);
		}

		/* if drag occurred completely within one window which is not frontmost, bring the
		   window to the front and update its contents */
		if ((attributes & dragInsideSenderWindow) && (theWindow != GetFrontDocumentWindow()))
		{
			MySelectWindow(theWindow);
			UpdateTheWindow(theWindow);
			TEActivate(hTE);
		}

		/* activate TE if not already active; otherwise hiliting will screw up */
		if (!((WindowPeek)theWindow)->hilited)
		{
			TEActivate(hTE);
		}

		/* if this window is also the sender, delete source selection if option key isn't down */
		if (moveText)
		{
			selStart = (**hTE).selStart;
			selEnd   = (**hTE).selEnd;
			if (insertPosition > selStart)
			{
				insertPosition -= ((**hTE).selEnd - (**hTE).selStart);
			}
			
			TEDelete(hTE);
		}

		/* finally actually insert text */
#if USE_STYLED_TEXT
		InsertTextAtOffset(insertPosition, textData, textSize, stylHandle, hTE);
#else
		InsertTextAtOffset(insertPosition, textData, textSize, hTE);
#endif

		/* update internal bits of window data */
		ResetHiliteRgn(theWindow);
		if (GetWindowVScrollBar(theWindow)!=0L)
			AdjustVScrollBar(GetWindowVScrollBar(theWindow), hTE);
		SetWindowIsModified(theWindow, TRUE);
		ReceiveDragDispatch(GetWindowIndex(theWindow));
	}

	DisposePtr(textData);

#if USE_STYLED_TEXT
	if (stylHandle)
		DisposeHandle((Handle) stylHandle);
#endif

	/* undo the forced TE activation, if necessary */
	if (!((WindowPeek)theWindow)->hilited)
	{
		TEDeactivate(hTE);
	}

	return noErr;
}


/*
 *	MyTrackingHandler
 *
 *	This is the drag tracking handler for windows.
 */

pascal OSErr MyTrackingHandler(short message, WindowPtr duhWindow,
							   void *handlerRefCon, DragReference theDrag)
{	short				result, offset;
	long				theTime = TickCount();
	unsigned short		count, index;
	unsigned long		flavorFlags, attributes;
	ItemReference		theItem;
	RgnHandle			theRgn;
	WindowPtr			theWindow=(WindowPtr)handlerRefCon;
	WindowPtr			hitWindow;
	Point				theMouse, localMouse;
	TEHandle			hTE;
	
	if ((message != dragTrackingEnterHandler) && (!canAcceptItems))
		return(noErr);

	hTE=GetWindowTE(theWindow);
	
	GetDragAttributes(theDrag, &attributes);

	switch (message) {

		case dragTrackingEnterHandler:

			//
			//	We get called with this message the first time that a drag enters ANY
			//	window in our application. Check to see if all of the drag items contain
			//	TEXT. We only accept a drag if all of the items in the drag can be accepted.
			//

			canAcceptItems = true;

			CountDragItems(theDrag, &count);

			for (index = 1; index <= count; index++) {
				GetDragItemReferenceNumber(theDrag, index, &theItem);

				result = GetFlavorFlags(theDrag, theItem, 'TEXT', &flavorFlags);

				if (result != noErr) {
					canAcceptItems = false;
					break;
				}
			}

			break;

		case dragTrackingEnterWindow:

			//
			//	We receive an EnterWindow message each time a drag enters one of our
			//	application's windows. We initialize our global variables for tracking
			//	the drag through the window.
			//

			caretTime = theTime;
			caretOffset = lastOffset = -1;
			caretShow = true;

			cursorInContent = false;

			break;

		case dragTrackingInWindow:

			//
			//	We receive InWindow messages as long as the mouse is in one of our windows
			//	during a drag. We draw the window highlighting and blink the insertion caret
			//	when we get these messages.
			//

			GetDragMouse(theDrag, &theMouse, 0L);
			localMouse = theMouse;
			GlobalToLocal(&localMouse);

			//
			//	Show or hide the window highlighting when the mouse enters or leaves the
			//	TextEdit field in our window (we don't want to show the highlighting when
			//	the mouse is over the window title bar or over the scroll bars).
			//

			if (attributes & dragHasLeftSenderWindow)
			{
				if (PtInRect(localMouse, &(theWindow->portRect)))
				{
					if (!cursorInContent)
					{
						Rect			temp;
						
						temp=GetWindowBounds(theWindow);
						OffsetRect(&temp, -temp.left, -temp.top);
						if (GetWindowVScrollBar(theWindow)!=0L)
							temp.right-=15;
						if (GetWindowHScrollBar(theWindow)!=0L)
							temp.bottom-=15;
						RectRgn(theRgn=NewRgn(), &temp);
						ShowDragHilite(theDrag, theRgn, true);
						DisposeRgn(theRgn);
					}
					cursorInContent = true;

				} else
				{
					if (cursorInContent)
					{
						HideDragHilite(theDrag);
					}
					cursorInContent = false;

				}
			}

			offset = HitTest(theMouse, &hitWindow);
			
			//
			//	If this application is the sender, do not allow tracking through
			//	the selection in the window that sourced the drag.
			//

			if (attributes & dragInsideSenderWindow) {
				if ((offset >= (**hTE).selStart) &&
					(offset <= (**hTE).selEnd)) {
						offset = -1;
				}
			}

			if (hitWindow == theWindow) {

				insertPosition = offset;

				//
				//	Reset flashing counter if the offset has moved. This makes the
				//	caret blink only after the caret has stopped moving long enough.
				//

				if (offset != lastOffset) {
					caretTime = theTime;
					caretShow = true;
				}
				lastOffset = offset;

				//
				//	Flash caret.
				//

				if (theTime - caretTime > gCaretTime) {
					caretShow = !caretShow;
					caretTime = theTime;
				}
				if (! caretShow) {
					offset = -1;
				}

				//
				//	If caret offset has changed, move caret on screen.
				//

				if (offset != caretOffset) {
					if (caretOffset != -1) {
						DrawCaret(caretOffset, hTE);
					}
					if (offset != -1) {
						DrawCaret(offset, hTE);
					}
				}

				caretOffset = offset;

			} else {

				lastOffset = offset;
				insertPosition = -1;

			}

			break;

		case dragTrackingLeaveWindow:

			//
			//	If the caret is on the screen, remove it.
			//

			if (caretOffset != -1) {
				DrawCaret(caretOffset, hTE);
				caretOffset = -1;
			}

			//
			//	Remove window highlighting, if showing.
			//

			if ((cursorInContent) && (attributes & dragHasLeftSenderWindow))
				HideDragHilite(theDrag);

			break;

		case dragTrackingLeaveHandler:
			break;

	}

	return(noErr);
}


/*
 *	DropLocationIsFinderTrash
 *
 *	Returns true if the given dropLocation AEDesc is a descriptor of the Finder's Trash.
 */

Boolean DropLocationIsFinderTrash(AEDesc *dropLocation)
{
	OSErr			result;
	AEDesc			dropSpec;
	FSSpec			*theSpec;
	CInfoPBRec		thePB;
	short			trashVRefNum;
	long			trashDirID;

	//
	//	Coerce the dropLocation descriptor to an FSSpec. If there's no dropLocation or
	//	it can't be coerced into an FSSpec, then it couldn't have been the Trash.
	//

	if ((dropLocation->descriptorType != typeNull) &&
		(AECoerceDesc(dropLocation, typeFSS, &dropSpec) == noErr)) {

		HLock(dropSpec.dataHandle);
		theSpec = (FSSpec *) *dropSpec.dataHandle;

		//
		//	Get the directory ID of the given dropLocation object.
		//

		thePB.dirInfo.ioCompletion = 0L;
		thePB.dirInfo.ioNamePtr = (StringPtr) &theSpec->name;
		thePB.dirInfo.ioVRefNum = theSpec->vRefNum;
		thePB.dirInfo.ioFDirIndex = 0;
		thePB.dirInfo.ioDrDirID = theSpec->parID;

		result = PBGetCatInfo(&thePB, false);

		HUnlock(dropSpec.dataHandle);
		AEDisposeDesc(&dropSpec);

		if (result != noErr)
			return(false);

		//
		//	If the result is not a directory, it must not be the Trash.
		//

		if (!(thePB.dirInfo.ioFlAttrib & (1 << 4)))
			return(false);

		//
		//	Get information about the Trash folder.
		//

		FindFolder(theSpec->vRefNum, kTrashFolderType, kCreateFolder, &trashVRefNum, &trashDirID);

		//
		//	If the directory ID of the dropLocation object is the same as the directory ID
		//	returned by FindFolder, then the drop must have occurred into the Trash.
		//

		if (thePB.dirInfo.ioDrDirID == trashDirID)
			return(true);
	}

	return(false);
}


/*
 *	Drag the selected text in the given document.
 */

Boolean DragText(WindowPtr theWindow, EventRecord *theEvent, RgnHandle hiliteRgn)
{
	short			result;
	RgnHandle		dragRegion, tempRgn;
	Point			theLoc;
	DragReference	theDrag;
#if USE_STYLED_TEXT
	StScrpHandle	theStyl;
#endif
	AEDesc			dropLocation;
	DragAttributes	attributes;
	short			mouseDownModifiers, mouseUpModifiers, copyText;
	TEHandle		hTE;
	

	if (!WindowHasLayer(theWindow))
		return FALSE;
	hTE=GetWindowTE(theWindow);
	
	//
	//	Copy the hilite region into dragRegion and offset it into global coordinates.
	//

	CopyRgn(hiliteRgn, dragRegion = NewRgn());
	SetPt(&theLoc, 0, 0);
	LocalToGlobal(&theLoc);
	OffsetRgn(dragRegion, theLoc.h, theLoc.v);

	//
	//	Wait for the mouse to move to the mouse button to be released. If the mouse button was
	//	released before the mouse moves, return false. Returing false from DragText means that
	//	a drag operation did not occur.
	//

	if (! WaitMouseMoved(theEvent->where))
	{
		return FALSE;
	}

	NewDrag(&theDrag);

	//
	//	For purposes of demonstration, we insert the 'TEXT' data and promise 'styl'
	//	data. If a receiver requests 'TEXT', the Drag Manager will give them the text
	//	without needing us to intervene. If a receiver requests 'styl', the Drag Manager
	//	will call our MySendDataProc to provide the data at drop time. The MySendDataProc
	//	is specified by calling SetDragSendProc.
	//

	AddDragItemFlavor(theDrag, 1, 'TEXT', GetSelectedTextPtr(theWindow),
					  GetSelectionSize(theWindow), 0);

#if USE_STYLED_TEXT
	theStyl = GetStylScrap(hTE);
	HLock((Handle) theStyl);
	AddDragItemFlavor(theDrag, 1, 'styl', (Ptr) *theStyl, GetHandleSize((Handle) theStyl), 0);
	HUnlock((Handle) theStyl);
	DisposeHandle((Handle) theStyl);

	SetDragSendProc(theDrag, MySendDataProc, (void *)theWindow);
#endif

	//
	//	Set the item's bounding rectangle in global coordinates.
	//

	SetDragItemBounds(theDrag, 1, &(**dragRegion).rgnBBox);

	//
	//	Prepare the drag region.
	//

	tempRgn = NewRgn();
	CopyRgn(dragRegion, tempRgn);
	InsetRgn(tempRgn, 1, 1);
	DiffRgn(dragRegion, tempRgn, dragRegion);
	DisposeRgn(tempRgn);

	//
	//	Drag the text. TrackDrag will return userCanceledErr if the drop zoomed-back
	//	for any reason.
	//

	result = TrackDrag(theDrag, theEvent, dragRegion);

	if (result != noErr && result != userCanceledErr)
	{
		return TRUE;
	}

	//
	//	Check to see if the drop occurred in the Finder's Trash. If the drop occurred
	//	in the Finder's Trash and a copy operation wasn't specified, delete the
	//	source selection. Note that we can continute to get the attributes, drop location
	//	modifiers, etc. of the drag until we dispose of it using DisposeDrag.
	//

	GetDragAttributes(theDrag, &attributes);
	if (!(attributes & dragInsideSenderApplication))
	{
		GetDropLocation(theDrag, &dropLocation);

		GetDragModifiers(theDrag, 0L, &mouseDownModifiers, &mouseUpModifiers);
		copyText = (mouseDownModifiers | mouseUpModifiers) & optionKey;

		if ((!copyText) && (DropLocationIsFinderTrash(&dropLocation)))
		{
			TEDelete(hTE);
			SetWindowIsModified(theWindow, TRUE);
		}

		AEDisposeDesc(&dropLocation);
	}

	//
	//	Dispose of the drag.
	//

	DisposeDrag(theDrag);

	DisposeRgn(dragRegion);

	return TRUE;
}

Boolean DragInWindow(WindowPtr theWindow, EventRecord *theEvent)
/* returns TRUE if drag occurred in window, FALSE otherwise */
{
	Point			thePoint;
	RgnHandle		hiliteRgn;
	
	if ((!DragManagerAvailableQQ()) || (theWindow==0L))
		return FALSE;
	if (!WindowIsDraggableQQ(theWindow))
		return FALSE;
	
	hiliteRgn=GetWindowHiliteRgn(theWindow);
	
	SetPort(theWindow);
	thePoint = theEvent->where;
	GlobalToLocal(&thePoint);

	if (PtInRgn(thePoint, hiliteRgn))
	{
		return DragText(theWindow, theEvent, hiliteRgn);
	}
	
	return FALSE;
}

void ResetHiliteRgn(WindowPtr theWindow)
{
	TEHandle		hTE;
	RgnHandle		hiliteRgn;
	
	if ((!DragManagerAvailableQQ()) || (theWindow==0L))
		return;
	if ((!WindowIsDraggableQQ(theWindow)) || ((hTE=GetWindowTE(theWindow))==0L))
		return;
	if ((hiliteRgn=GetWindowHiliteRgn(theWindow))==0L)
		return;
	
	if (hiliteRgn==0L)
		hiliteRgn=NewRgn();
	TEGetHiliteRgn(hiliteRgn, hTE);
	SetWindowHiliteRgn(theWindow, hiliteRgn);
}

Boolean CursorInDraggableRgn(Point thePoint, WindowPtr theWindow)
{
	RgnHandle		hiliteRgn;
	
	if ((!DragManagerAvailableQQ()) || (theWindow==0L))
		return FALSE;
	if (!WindowIsDraggableQQ(theWindow))
		return FALSE;
	
	hiliteRgn=GetWindowHiliteRgn(theWindow);
	return (PtInRgn(thePoint, hiliteRgn));
}
