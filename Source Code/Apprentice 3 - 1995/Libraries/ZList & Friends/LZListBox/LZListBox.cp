// ===========================================================================
//	LZListBox.cp				©1995 Chris K. Thomas. All rights reserved.
// ===========================================================================
//
//	A wrapper class for ZList
//	Note the amount of work involved in getting ZList to do things.
//	Compare this to the amount of work required to get the List
//	Manager to do the equivalent.

#ifdef PowerPlant_PCH
#include PowerPlant_PCH
#endif

#include "LZListBox.h"
#include <LStream.h>
#include <LFocusBox.h>
#include <LView.h>

#include <PP_KeyCodes.h>
#include <PP_Messages.h>
#include <UTextTraits.h>
#include <UKeyFilters.h>
#include <UDrawingState.h>

// ---------------------------------------------------------------------------
//		¥ LZListBox
// ---------------------------------------------------------------------------
//	Construct from input parameters
//
//	Usage Note: The current port *must* be the Window which contains
//		the ListBox (required by the Mac Control Manager)

LZListBox::LZListBox(
	const SPaneInfo	&inPaneInfo,
	ListFeature		inFeatures,
	Int16			inTextTraitsID,
	LCommander		*inSuper)
		: LPane(inPaneInfo),
		  LCommander(inSuper),
		  LDragAndDrop((WindowPtr)inPaneInfo.superView->GetMacPort(), this)
{
	InitListBox(inFeatures, 16, inTextTraitsID);
}


// ---------------------------------------------------------------------------
//		¥ ~LZListBox
// ---------------------------------------------------------------------------
//	Destructor

LZListBox::~LZListBox()
{
	if (mMacZList != nil) {			// Dispose Toolbox ListHandle
		FocusDraw();
		delete mMacZList;
	}
}

// ÐÐÐÐÐÐÐÐÐÐÐÐÐÐÐÐÐÐÐÐÐÐÐÐÐÐÐÐÐÐÐÐ
// ¥ Drag And Drop Overrides
// ÐÐÐÐÐÐÐÐÐÐÐÐÐÐÐÐÐÐÐÐÐÐÐÐÐÐÐÐÐÐÐÐ

void
LZListBox::InsideDropArea(DragReference inDragRef)
{
	mMacZList->TryDragTrack(inDragRef);
}

void
LZListBox::DoDragReceive(DragReference inDragRef)
{
	mMacZList->TryDragDrop(inDragRef);
}

Boolean
LZListBox::DragIsAcceptable(DragReference inDragRef)
{
	return mMacZList->WeCanReceiveDrag(inDragRef);
}

// ---------------------------------------------------------------------------
//		¥ InitListBox
// ---------------------------------------------------------------------------
//	Private initializer

void
LZListBox::InitListBox(
	ListFeature		inFeatures,
	Int16			inItemSize,
	Int16			inTextTraitsID)
{
	mFocusBox = nil;				// Create FocusBox if necessary
	mFocusBox = new LFocusBox;
	mFocusBox->Hide();
	mFocusBox->AttachPane(this);

	FocusDraw();
	
	mTextTraitsID = inTextTraitsID;	// Establish text characteristics
	UTextTraits::SetPortTextTraits(inTextTraitsID);
	
	Rect r;
	CalcLocalFrameRect(r);
	
	mMacZList = new ZList(r, inItemSize);
	ThrowIfNULL_(mMacZList);
	
	if(!mMacZList->DoesFeature(inFeatures))
		mMacZList->ToggleFeature(inFeatures);
}

// ---------------------------------------------------------------------------
//		¥ GetMacListH
// ---------------------------------------------------------------------------
//	Return the Toolbox ListHandle associated with a ListBox object
//
//	You may manipulate the ListHandle using the Toolbox ListManager traps,
//	as well as inspect and change certain fields. As documented in Inside
//	Mac, you must directly store into certain fields in a ListRecord in
//	order to change settings for a List. For example, the selFlags field
//	controls how the ListManger handles selections in a List.
//
//	There are some traps you should not call, since there is a LZListBox
//	function that performs the same task (in a way that's compatible
//	with PowerPlant Panes).
//
//		ListManager Trap		LZListBox function
//		----------------		-----------------
//		LDispose				~LZListBox
//		LSize					ResizeFrameBy
//		LDraw					Draw

ZList *
LZListBox::GetMacZList() const
{
	return mMacZList;
}


// ---------------------------------------------------------------------------
//		¥ GetFocusBox
// ---------------------------------------------------------------------------
//	Return the FocusBox associated with a ListBox

LFocusBox*
LZListBox::GetFocusBox()
{
	return mFocusBox;
}


// ---------------------------------------------------------------------------
//		¥ GetValue
// ---------------------------------------------------------------------------
//	Return the value of a ListBox.
//
//	The "value" of a ListBox is the row number of the first selected cell,
//	with the first row being number 1 (ZList uses one-based
//	numbering). If no cells are selected, the value is 0.
//
//	This "value" makes sense for a ListBox with one column, by far the
//	most common case.

Int32
LZListBox::GetValue() const
{
	Int32	value = -1;

	value = mMacZList->GetSelectedItemIndex(1);
		
	return value;
}


// ---------------------------------------------------------------------------
//		¥ SetValue
// ---------------------------------------------------------------------------
//	Set the value of a ListBox.
//
//	The "value" of a ListBox is the row number of the first selected cell,
//	with the first row being number 0 (the ListManager uses zero-based
//	numbering). If no cells are selected, the value is -1.
//
//	This "value" makes sense for a ListBox with one column, by far the
//	most common case.
//
//	This function selects the cell in row "inValue" and column 1, deselecting
//	any previously selected cells.

void
LZListBox::SetValue(
	Int32	inValue)
{
	FocusDraw();
	
	mMacZList->SelectItem(inValue);
}


// ---------------------------------------------------------------------------
//		¥ GetDescriptor
// ---------------------------------------------------------------------------
//	Doesn't make much sense in this case.
//
//	This function assumes that the cell data is text.

StringPtr
LZListBox::GetDescriptor(
	Str255	outDescriptor) const
{
	Str255 outStr = "\pThis Be A Lost!";
	
	BlockMoveData(&outStr[0], &outDescriptor[0], outStr[0]+1);

	return outDescriptor;
}


// ---------------------------------------------------------------------------
//		¥ SetDescriptor
// ---------------------------------------------------------------------------
//	Set the descriptor of a ListBox, which is the text of the first
//	selected cell. Nothing happens if there are no selected cells.
//
//	This function assumes that the cell data is text.

void
LZListBox::SetDescriptor(
	ConstStr255Param	/*inDescriptor*/)
{
	//ignorance;
}


// ---------------------------------------------------------------------------
//		¥ FocusDraw
// ---------------------------------------------------------------------------
//	Focus drawing. Overrides to use the default Pen state and to
//	set the TextTraits used by the ListBox

Boolean
LZListBox::FocusDraw()
{
	Boolean	focused = LPane::FocusDraw();
	if (focused) {
		StColorPenState::Normalize();
		UTextTraits::SetPortTextTraits(mTextTraitsID);
	}
	
	return focused;
}


// ---------------------------------------------------------------------------
//		¥ DrawSelf
// ---------------------------------------------------------------------------
//	Draw ListBox

void
LZListBox::DrawSelf()
{
	mMacZList->DrawSelf();
}


// ---------------------------------------------------------------------------
//		¥ ClickSelf
// ---------------------------------------------------------------------------
//	Respond to Click inside an ListBox

void
LZListBox::ClickSelf(
	const SMouseDownEvent	&inMouseDown)
{
	if (SwitchTarget(this)) {
		FocusDraw();
		
		mMacZList->ClickSelf(inMouseDown.macEvent);
	}
}


// ---------------------------------------------------------------------------
//		¥ ObeyCommand
// ---------------------------------------------------------------------------
//	Respond to Command message

Boolean
LZListBox::ObeyCommand(
	CommandT	inCommand,
	void*		ioParam)
{
	Boolean		cmdHandled = true;
	
	switch (inCommand) {
	
		case msg_TabSelect:
			if (!IsEnabled()) {
				cmdHandled = false;
			}
			break;
			
		default:
			cmdHandled = LCommander::ObeyCommand(inCommand, ioParam);
			break;
	}
	
	return cmdHandled;
}


// ---------------------------------------------------------------------------
//		¥ HandleKeyPress
// ---------------------------------------------------------------------------
//	ListBox supports keyboard navigation and type selection

Boolean
LZListBox::HandleKeyPress(
	const EventRecord	&inKeyEvent)
{
/*	Boolean	keyHandled = true;
	Char16	theKey = inKeyEvent.message & charCodeMask;
	
	FocusDraw();

	if (UKeyFilters::IsNavigationKey(theKey)) {
		DoNavigationKey(inKeyEvent);
	
	} else if (UKeyFilters::IsPrintingChar(theKey)) {
		DoTypeSelection(inKeyEvent);
									
	} else {
		keyHandled = LCommander::HandleKeyPress(inKeyEvent);
	}
			
	return keyHandled;*/
	return false;
}


// ---------------------------------------------------------------------------
//		¥ DoNavigationKey
// ---------------------------------------------------------------------------
//	Implements keyboard navigation by supporting selection change using
//	the arrow keys, page up, page down, home, and end

// ZList will eventually support this stuff itself.

void
LZListBox::DoNavigationKey(
	const EventRecord	&inKeyEvent)
{
/*	char	theKey = inKeyEvent.message & charCodeMask;
	Boolean	cmdKeyDown = (inKeyEvent.modifiers & cmdKey) != 0;
	Boolean	shiftKeyDown = (inKeyEvent.modifiers & shiftKey) != 0;
	Cell	theSelection = {0, 0};

	switch (theKey) {

		case char_LeftArrow:
			if (::LGetSelect(true, &theSelection, mMacListH)) {
				if (cmdKeyDown && shiftKeyDown) {
									// Select all cells left of the first
									//   selected cell
					while (--theSelection.h >= 0) {
						::LSetSelect(true, theSelection, mMacListH);
					}
				
				} else if (cmdKeyDown) {
									// Select leftmost cell in the same row as
									//   the first selected cell
					theSelection.h = 0;
					SelectOneCell(theSelection);
				
				} else if (shiftKeyDown) {
									// Extend selection by selecting the
									//   cell left of the first selected cell
					if (theSelection.h > 0) {
						theSelection.h -= 1;
						::LSetSelect(true, theSelection, mMacListH);
					}
				
				} else {			// Select cell to the left of the first
									//   selected cell
					if(theSelection.h > 0) {
						theSelection.h -= 1;
					}
					SelectOneCell(theSelection);
				}
				MakeCellVisible(theSelection);
			}
			break;

		case char_RightArrow:
			if (GetLastSelectedCell(theSelection)) {
				Int16	numColumns = (**mMacListH).dataBounds.right - 1;
				
				if (cmdKeyDown && shiftKeyDown) {
									// Select all cells right of the last
									//   selected cell
					while (++theSelection.h <= numColumns) {
						::LSetSelect(true, theSelection, mMacListH);
					}
										
				} else if (cmdKeyDown) {
									// Select rightmost cell in the same row as
									//   the last selected cell
					theSelection.h = numColumns;
					SelectOneCell(theSelection);
										
				} else if (shiftKeyDown) {
									// Extend selection by selecting the cell
									//   to the right of the last selected cell
					if(theSelection.h < numColumns) {
						theSelection.h += 1;
						::LSetSelect(true, theSelection, mMacListH);
					}
	
				} else {			// Select cell to the right of the last
									//   selected cell
				
					if(theSelection.h < numColumns) {
						theSelection.h += 1;
					}
					SelectOneCell(theSelection);
				}
				MakeCellVisible(theSelection);
			}
			break;

		case char_UpArrow:
			if (::LGetSelect(true, &theSelection, mMacListH)) {
				if (cmdKeyDown && shiftKeyDown) {
									// Select all cells above the first
									//   selected cell
					while (--theSelection.v >= 0) {
						::LSetSelect(true, theSelection, mMacListH);
					}
				
				} else if (cmdKeyDown) {
									// Select cell at top of column of the
									//   first selected cell
					theSelection.v = 0;
					SelectOneCell(theSelection);
				
				} else if (shiftKeyDown) {
									// Extend selection by selecting the
									//   cell above the first selected cell
					if (theSelection.v > 0) {
						theSelection.v -= 1;
						::LSetSelect(true, theSelection, mMacListH);
					}
				
				} else {			// Select cell one above the first
									//   selected cell
					if(theSelection.v > 0) {
						theSelection.v -= 1;
					}
					SelectOneCell(theSelection);
				}
				MakeCellVisible(theSelection);
			}
			break;

		case char_DownArrow:
			if (GetLastSelectedCell(theSelection)) {
				Int16	numRows = (**mMacListH).dataBounds.bottom - 1;
				
				if (cmdKeyDown && shiftKeyDown) {
									// Select all cells below the last
									//   selected cell
					while (++theSelection.v <= numRows) {
						::LSetSelect(true, theSelection, mMacListH);
					}
										
				} else if (cmdKeyDown) {
									// Select cell at bottom of column of
									//   the last selected cell
					theSelection.v = numRows;
					SelectOneCell(theSelection);
										
				} else if (shiftKeyDown) {
									// Extend selection by selecting the
									//   cell below the last selected cell
					if(theSelection.v < numRows) {
						theSelection.v += 1;
						::LSetSelect(true, theSelection, mMacListH);
					}
	
				} else {			// Select cell one below the last
									//   selected cell
				
					if(theSelection.v < numRows) {
						theSelection.v += 1;
					}
					SelectOneCell(theSelection);
				}
				MakeCellVisible(theSelection);
			}
			break;
			
		case char_Home:
			::LScroll(-16000, -16000, mMacListH);
			break;

		case char_End:
			::LScroll(16000, 16000, mMacListH);
			break;
			
		case char_PageUp:
		case char_PageDown: {
			Int16	dRows = (**mMacListH).visible.bottom -
							(**mMacListH).visible.top - 1;
			if (theKey == char_PageUp) {
				dRows = -dRows;
			}
			::LScroll(0, dRows, mMacListH);
			break;
		}
	}*/
}	


// ---------------------------------------------------------------------------
//		¥ BeTarget
// ---------------------------------------------------------------------------
//	ListBox is becoming the Target

void
LZListBox::BeTarget()
{
	if (mFocusBox != nil) {
		mFocusBox->Show();
	}
}


// ---------------------------------------------------------------------------
//		¥ DontBeTarget
// ---------------------------------------------------------------------------
//	ListBox is no longer the Target

void
LZListBox::DontBeTarget()
{
	if (mFocusBox != nil) {
		mFocusBox->Hide();
	}
}


// ---------------------------------------------------------------------------
//		¥ ActivateSelf
// ---------------------------------------------------------------------------
//	Activate ListBox. The Toolbox shows the selection and scroll bars.

void
LZListBox::ActivateSelf()
{
	if (FocusDraw()) {
		mMacZList->Activate(true);
	}
}


// ---------------------------------------------------------------------------
//		¥ DeactivateSelf
// ---------------------------------------------------------------------------
//	Deactivate ListBox. The Toolbox hides the selection and scroll bars.

void
LZListBox::DeactivateSelf()
{
	if (FocusDraw()) {
		mMacZList->Activate(false);
	}
}


// ---------------------------------------------------------------------------
//		¥ ResizeFrameBy
// ---------------------------------------------------------------------------
//	Change the Frame size by the specified amounts
//
//	Both PowerPlant and ZList store a size for the ListBox,
//	so we must make sure that both sizes are in synch.

void
LZListBox::ResizeFrameBy(
	Int16		inWidthDelta,
	Int16		inHeightDelta,
	Boolean		inRefresh)
{
	FocusDraw();
									// Prevent List and Control Managers
									// from automatically drawing by setting
									// an empty clipping region
	Rect	emptyRect = {0, 0, 0, 0};
	StClipRgnState	saveClip(emptyRect);
	
	Rect	displayRect;
	
	mMacZList->PubGetViewRect(displayRect);	// ª temp hack due to ZList design flaw
	
	displayRect.right += inWidthDelta;
	displayRect.bottom += inHeightDelta;
	
	mMacZList->SetViewRect(displayRect);
	
/*									// Get current size of ListBox
	Rect	displayRect = (**mMacListH).rView;
	Rect	bounds = (**mMacListH).dataBounds;
	
		// If the cell width is the default width, adjust the cell
		// width so that it will remain at the default width for the
		// new width of the ListBox. The default width is the width
		// of the ListBox divided by the number of columns.
	
	if (bounds.right > bounds.left) {
		Int16	defaultWidth = (displayRect.right - displayRect.left) /
									(bounds.right - bounds.left);
									
		if ((**mMacListH).cellSize.h == defaultWidth) {
			Point	cellSize = (**mMacListH).cellSize;
			cellSize.h = (displayRect.right - displayRect.left + inWidthDelta) /
									(bounds.right - bounds.left);
			::LCellSize(cellSize, mMacListH);
		}
	}
			
		// Resize viewing rectangle of Toolbox ListHandle
	
	::LSize(displayRect.right - displayRect.left + inWidthDelta,
			displayRect.bottom - displayRect.top + inHeightDelta,
			mMacListH);
		
		// Toolbox ListManager seems to have a bug. When there is only
		// one scroll bar (vertical or horizontal), the scroll bar is
		// not indented if inHasGrow is true. As a workaround, we
		// manually set the scroll bar size.

	if (mHasGrow) {
		if ((**mMacListH).vScroll != nil) {		// Vertical Scroll Bar
			(**((**mMacListH).vScroll)).contrlRect.bottom =
					(**mMacListH).rView.bottom - 14;
		}
		
		if ((**mMacListH).hScroll != nil) {		// Horizontal Scroll bar
			(**((**mMacListH).hScroll)).contrlRect.right =
					(**mMacListH).rView.right - 14;
		}
	}*/
									// Resize Pane
	LPane::ResizeFrameBy(inWidthDelta, inHeightDelta, inRefresh);
}


// ---------------------------------------------------------------------------
//		¥ MoveBy
// ---------------------------------------------------------------------------
//	Move the location of the Frame by the specified amounts
//
//	Both PowerPlant and the List Manager store a location for the ListBox,
//	so we must make sure that both locations are in synch.

void
LZListBox::MoveBy(
	Int32		inHorizDelta,
	Int32		inVertDelta,
	Boolean		inRefresh)
{
									// Move Pane
	LPane::MoveBy(inHorizDelta, inVertDelta, inRefresh);
	
	Rect	displayRect;
	
	mMacZList->PubGetViewRect(displayRect);
	
	OffsetRect(&displayRect, inHorizDelta, inVertDelta);
	
	mMacZList->SetViewRect(displayRect);
/*	
		// Unfortunately, the Toolbox does not have a call to
		// move a List, so we have to directly set the proper
		// fields of the ListHandle
		
		// Determine how far to offset ListHandle's view rectangle,
		// which we want to be the same as the Frame of the ListBox
		
	Rect	displayRect = (**mMacListH).rView;
	Rect	frame;
	CalcLocalFrameRect(frame);
	
	Int16	horizMove = frame.left - displayRect.left + 1;
	Int16	vertMove = frame.top - displayRect.top + 1;
	
	if ((horizMove == 0) && (vertMove == 0)) {
		return;						// No need to move view rectangle
	}

									// Move view rectangle
	::OffsetRect(&(**mMacListH).rView, horizMove, vertMove);
	
									// Move Scroll Bars
									// Ugh. We directly offset the rectangle
									//   inside the ControlHandle instead
									//   of using MoveControl since we don't
									//   know what coordinate system the
									//   List Manager uses.
	if ((**mMacListH).vScroll != nil) {
		::OffsetRect(&(**((**mMacListH).vScroll)).contrlRect, horizMove,
					vertMove);
	}
	if ((**mMacListH).hScroll != nil) {
		::OffsetRect(&(**((**mMacListH).hScroll)).contrlRect, horizMove,
					vertMove);
	}*/
}
