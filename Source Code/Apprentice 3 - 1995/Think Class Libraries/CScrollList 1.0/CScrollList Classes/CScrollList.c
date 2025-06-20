/*************************************************************************************

 CScrollList.c
	
		A scrolling list that displays information in a CArray.  Combines all the
		good qualities of CTable and CArrayPane but keeps it to one column with
		one uniform rowheight and only one selected cell.  Provides four options:
		
		SELECTABLE
		
		If this option is not selected, the currently selected cell will not be
		hilited (except when dragging).  This is useful when combined with the
		checkable option to create a check list that can't be edited or rearranged.
		
		CHECKABLE
		
		Maintains a separate CStateArray to track the state (checked or not checked)
		of each item in the CArray.  Keeps both arrays in sync with each other.
		
		DRAGABLE
		
		Allows array elements to be arranged in any order by dragging them.
		
		EDITABLE
		
		Implements the editing of the CArray elements.  This is an abstract class
		that holds the framework for editing cells, but makes no assumptions as to
		the contents of those cells.  Several methods that access the data in the
		linked array must be overriden in order to allow the array to actually be
		changed.  Subclasses should install subpanes during editing.
		
			OVERRIDE these methods to use
			{
				DoInsertCell( void)
				BeginEditing( void)
				SetupCellData( void)
				RetrieveCellData( void)
				DoneEditing( void)
			}
		
		While editing, selectedCell is the cell being edited.
		
		THINGS TO CHANGE/ADD
		
	x	Fix Panorama Coordinates & Scrolling
			Still can't have the panorama origin be at (1,1), but at least
			it's one cell to one vertical panorama unit, finally!
	x	Fix SELECTABLE option.
			Not totally complete.  Really, there should never be a selected
			cell at all, except maybe when dragging, and even then...
	x	Add FrameCell for dragging.
			It only does the same thing as hiliting, but it needed to bypass
			the standard call.  Even when SELECTABLE is off, we still want
			the cell being dragged to show itself first.
	x	Create a DrawCell and DrawCellRange CChore for updates.
			This has since been removed due to a problem in which the scroll
			list is disposed out from underneath the chore.  The motivation
			behind creating the chore has also been fixed.  6/8/92
		Automatic offscreen drawing.
			Dream on...
	
	SUPERCLASS = CPanorama
	
	REQUIREMENTS: CScrollListDragger
				  CTextEnvirons
				  CStateArray
				  gCheckCursor - expected to be loaded and locked in memory
	
		� 1992 Dave Harkness

*************************************************************************************/


#include "CScrollList.h"
#include "CScrollListDragger.h"
#include "CStateArray.h"

#include <CArray.h>
#include <CTextEnvirons.h>
#include <CScrollPane.h>
#include <CPaneBorder.h>
#include <CList.h>
#include <CApplication.h>
#include <Constants.h>
#include <Commands.h>
#include <Global.h>


#define kDefaultHIndent		3	// room for edges
#define kCheckWidth			9	// room for the check mark

#define kBorderMargin		1	// margin of active border
#define kBorderPen			2	// pen size of active border

#define kReturnKey		0x0D
#define kDeleteKey		0x08


		// Macros for testing the various list options

#define SELECTABLE		(listOptions & kSLSelectable)
#define DRAGABLE		(listOptions & kSLDragable)
#define CHECKABLE		(listOptions & kSLCheckable)
#define EDITABLE		(listOptions & kSLEditable)


extern CBureaucrat		*gGopher;
extern short			gClicks;
extern CursHandle		gCheckCursor;


/*************************************************************************************
 IScrollList
*************************************************************************************/

void
CScrollList::IScrollList( CView *anEnclosure, CBureaucrat *aSupervisor,
						  short aWidth, short aHeight,
						  short aHEncl, short aVEncl,
						  SizingOption aHSizing, SizingOption aVSizing,
						  short fOptions)

{
	CPanorama::IPanorama( anEnclosure, aSupervisor, aWidth, aHeight,
						  aHEncl, aVEncl, aHSizing, aVSizing);
			
	SetWantsClicks( TRUE);								// we want to be clicked!
	
	IScrollListX( fOptions);

}  /* CScrollList::IScrollList */


/*************************************************************************************
 IViewTemp  {OVERRRIDE}

	Extract the extra information from a 'SLst' resource.
*************************************************************************************/

void
CScrollList::IViewTemp( CView *anEnclosure, CBureaucrat *aSupervisor,
						Ptr viewData)
{
	tScrollListTempP	template = (tScrollListTempP) viewData;
	
	inherited::IViewTemp( anEnclosure, aSupervisor, viewData);
	IScrollListX( template->listOptions);
	
	SetDblClickCmd( template->dblClickCmd);
	SetDrawActiveBorder( template->drawActiveBorder != 0);
	

}  /* CScrollList::IViewTemp */


/*************************************************************************************
 IScrollListX

	Perform basic initialization that will be used for every creation method.
*************************************************************************************/

void
CScrollList::IScrollListX( short fOptions)

{
	FontInfo		fInfo;
	
			// Lists will be limited to 32767 pixels (approx. 20000 lines of text).
			// If that isn't enough, change usingLongCoord to TRUE.  CScrollList,
			// and its subclasses, will use FrameToQD for converting to/from
			// frame coordinates, instead of LongToQD.  This means that no additional
			// changes must be made.
	
	usingLongCoord = FALSE;
	
	numCells = 0;						// no cells and no selection
	selectedCell = 0;
	itsArray = NULL;
	itsChecks = NULL;
	
	SetCanBeGopher( FALSE);				// no array yet, so can't be gopher ====
	drawActiveBorder = TRUE;
	dblClickCmd = cmdNull;
	listOptions = fOptions;
	
	fEditing = FALSE;
	fStillEditing = FALSE;
	
	CreateTextEnvironment();			// setup the desired text characteristics
	Prepare();							// setup the port and restore the text
										// environment
	
	GetFontInfo( &fInfo);
	fontInfo = fInfo;					// set the instance variable
	cellHeight = fInfo.ascent + fInfo.descent + fInfo.leading;
	
			// add a pixel extra on the top and bottom of cells to look nice
	cellHeight += 2;
	
	SetScales( width, cellHeight);
	SetPt( &indent, kDefaultHIndent + (kCheckWidth * (CHECKABLE != 0)),
					fInfo.ascent + 1);

}  /* CScrollList::IScrollListX */


/*************************************************************************************
 CreateTextEnvironment
	
	Create and initialize the text environment used for drawing text.
	it will also be used for calculating the default indent and cell height.
*************************************************************************************/

void
CScrollList::CreateTextEnvironment( void)

{
	CTextEnvirons	*textEnvirons;
	TextInfoRec		textInfo;
	
	textEnvirons = new( CTextEnvirons);
	itsEnvironment = textEnvirons;
	textEnvirons->ITextEnvirons();
	
	textInfo.fontNumber = geneva;
	textInfo.theSize = 9;
	textInfo.theStyle = 0;
	textInfo.theMode = srcOr;
	
	textEnvirons->SetTextInfo( &textInfo);

}  /* CScrollList::CreateTextEnvironment */


/*************************************************************************************
 SetScrollPane  {OVERRIDE}

	Make sure the steps and overlap for our ScrollPane are set up correctly.
*************************************************************************************/

void
CScrollList::SetScrollPane( CScrollPane *aScrollPane)

{
	inherited::SetScrollPane( aScrollPane);
	
	itsScrollPane->SetSteps( 1, 1);
	itsScrollPane->SetOverlaps( 0, 1);

}  /* CScrollList::SetScrollPane */


/*************************************************************************************
 AdjustCursor  {OVERRIDE}

	Adjust the cursor to be either the check mark or the arrow apporpriately.
*************************************************************************************/

void
CScrollList::AdjustCursor( Point where, RgnHandle mouseRgn)

{
	LongPt		locWhere;
	RgnHandle	myRgn;
	Rect		globalRect;
	LongRect	frameRect;
	
	WindToFrame( where, &locWhere);
	
	if ( CHECKABLE && locWhere.h < indent.h )
	{
		SetCursor( *gCheckCursor);						// Set to check cursor
		GetAperture( &frameRect);
		frameRect.right = frameRect.left + indent.h;
	}
	else
	{
		SetCursor( &arrow);								// Set to arrow cursor
		GetAperture( &frameRect);
		frameRect.left += indent.h;
	}
	
	myRgn = NewRgn();
	if ( myRgn != NULL )
	{
		FrameToGlobalR( &frameRect, &globalRect);		// Convert to global coords
		RectRgn( myRgn, &globalRect);
		SectRgn( mouseRgn, myRgn, mouseRgn);			// and clip to old mouseRgn
		DisposeRgn( myRgn);
	}

}  /* CScrollList::AdjustCursor */


/*************************************************************************************
 MakeMouseTask

	Create a cell dragging mousetask.
	
	RETURNS the MouseTask
*************************************************************************************/

CMouseTask *
CScrollList::MakeMouseTask( short modifiers)

{
	CScrollListDragger	*theTask;
	
	theTask = new( CScrollListDragger);
	theTask->IScrollListDragger( this, modifiers, cellHeight, listOptions);
	
	return theTask;

}  /* CScrollList::MakeMouseTask */


/*************************************************************************************
 DoClick  {OVERRIDE}

	Handle clicks and drags.  A single click just selects the cell for editing.
	If the list is a check list, it might check/uncheck the cell.
	A double click sends the dblClickCmd.  A drag selects the cell and begins
	tracking the mouse to move the cell within the list.
*************************************************************************************/

void
CScrollList::DoClick( Point hitPt, short modifierKeys, long when)

{
	CMouseTask		*mouseTask;
	LongRect		pinRect;
	short			hitCell;
	LongPt			pt;
	long			hExtent, vExtent;
		
	QDToFrame( hitPt, &pt);
	
	if ( hitCell = FindHitCell( &pt) )
	{
		if ( gClicks == 2 )									// a double click?
		{
			DoDblClick( hitCell, modifierKeys, when);
		}
		else if ( CHECKABLE && (pt.h < indent.h) )			// check the cell
		{
			CheckCell( hitCell);
		}
		else if ( DRAGABLE )								// drag the cell
		{
			mouseTask = MakeMouseTask( modifierKeys);
			if ( mouseTask )
			{
				GetPixelExtent( &hExtent, &vExtent);
				SetLongRect( &pinRect, 1, 1, hExtent - 1, vExtent - 1);
				TrackMouse( mouseTask, &pt, &pinRect);
			}
		}
		else if ( SELECTABLE )								// select the cell
		{
			SelectCell( hitCell, kDoRedraw);
		}
	}
	else
		ClickOutsideBounds( hitPt, modifierKeys, when);

}  /* CScrollList::DoClick */


/*************************************************************************************
 SetDblClickCmd

	Set the command sent to this object when a cell is double-clicked..
*************************************************************************************/

void
CScrollList::SetDblClickCmd( long aCmd)

{
	dblClickCmd = aCmd;

}  /* CScrollList::SetDblClickCmd */


/*************************************************************************************
 DoDblClick

	If there is a double-click command, pass it along to this object so that
	subclasses may handle it if they wish.  Default is to send it up the
	chain of command..
*************************************************************************************/

void
CScrollList::DoDblClick( short hitCell, short modifierKeys, long when)
{
	if ( dblClickCmd != cmdNull )
		DoCommand( dblClickCmd);

}  /* CScrollList::DoDblClick */


/*************************************************************************************
 HitSamePart  {OVERRIDE}

	The TCL calls HitSamePart to determine if two clicks are in the "same part" and
	can be considered a double-click.
	
	RETURNS true if the clicks hit the same part.
*************************************************************************************/

Boolean
CScrollList::HitSamePart( Point pointA, Point pointB)
{
	short		cellA, cellB;
	LongPt		ptA, ptB;
	
			// points are in window coords, convert to local coords
	
	WindToFrame( pointA, &ptA);
	WindToFrame( pointB, &ptB);
	
			// find the cell hit by each point
	
	cellA = FindHitCell( &ptA);
	cellB = FindHitCell( &ptB);
	
			// before we test the cells for equality, we must make sure that
			// both points were in the table in the first place
	
	if ( (cellA > numCells || cellA < 1) || (cellB > numCells || cellB < 1) )
		return FALSE;
	else
		return (cellA == cellB);

}  /* CScrollList::HitSamePart */


/*************************************************************************************
 AddCell

	Add a block of cells to the list at the given position.  Shifts all cells below
	afterCell down howMany cells.  Forces a refresh of all affected cells.
	Use 0 for afterCell to insert the new cells at the top of the list.
*************************************************************************************/

void
CScrollList::AddCell( short howMany, short afterCell)
{
	if ( howMany > 0 )
	{
				// clip afterCell to actual cell count
		
		afterCell = Min( afterCell, numCells);
		afterCell = Max( afterCell, 0);
		
		numCells += howMany;
		
		AdjustBounds();	
		RefreshCellRange( afterCell+1, numCells);		// Redraw affected cells
	}

}  /* CScrollList::AddCell */


/*************************************************************************************
 DeleteCell

	Delete a block of howMany cells starting at startCell.  Shifts all cells below
	startCell up howMany cells.  Forces a refresh of all affected cells.
*************************************************************************************/

void
CScrollList::DeleteCell( short howMany, short startCell)
{
	short	newCell = selectedCell;
	short	oldNumCells = numCells;
	
	if ( howMany > 0 && numCells > 0 )
	{
				// Clip startCell and howMany to actual number of cells
		
		startCell = Min( numCells, startCell);
		startCell = Max( startCell, 1);
		howMany = Min( howMany, numCells - startCell + 1);
		
				// If selectedCell is below the block of deleted cells,
				// shift it down howMany cells.
		
		if ( newCell >= startCell + howMany )
			newCell -= howMany;
		
		numCells -= howMany;							// adjust the list
		newCell = Min( numCells, newCell);				// clip to cell range
		AdjustBounds();
		ScrollToSelection();
		RefreshCellRange( startCell, oldNumCells);
		SelectCell( newCell, kDontRedraw);
	}

}  /* CScrollList::DeleteCell */


/*************************************************************************************
 AdjustBounds

	Adjusts the bounds instance variable in response to a change in the number
	of cells in the list.
*************************************************************************************/

void
CScrollList::AdjustBounds( void)

{
	LongRect	boundsR;
	
	SetLongRect( &boundsR, 0, 0, width, numCells);
	SetBounds( &boundsR);

}  /* CScrollList::AdjustBounds */


/*************************************************************************************
 IsChecked

	Return TRUE if theCell is checked.
*************************************************************************************/

Boolean
CScrollList::IsChecked( short theCell)

{
	ASSERT( CHECKABLE);
	
	return (itsChecks->GetState( theCell) != 0);

}  /* CScrollList::IsChecked */


/*************************************************************************************
 CheckCell

	Toggle the checked state of theCell.
*************************************************************************************/

void
CScrollList::CheckCell( short theCell)

{
	ASSERT( CHECKABLE);
	
	itsChecks->ToggleState( theCell);
	RefreshCell( theCell);

}  /* CScrollList::CheckCell */


/*************************************************************************************
 CheckAllCells

	Set the check state for all cells to fCheckState.
*************************************************************************************/

void
CScrollList::CheckAllCells( Boolean fCheckState)

{
	ASSERT( CHECKABLE);
	
	itsChecks->SetAllStates( fCheckState);
	Refresh();

}  /* CScrollList::CheckAllCells */


/*************************************************************************************
 GetCheckArray

	Return a copy of the itsChecks array.
*************************************************************************************/

CStateArray *
CScrollList::GetCheckArray( void)

{
	ASSERT( CHECKABLE);
	
	return (CStateArray *)itsChecks->Copy();

}  /* CScrollList::GetCheckArray */


/*************************************************************************************
 Draw  {OVERRIDE}

	Draw the range of cells that intersect the given update area.
*************************************************************************************/

void
CScrollList::Draw( Rect *area)
{
	short		firstCell, lastCell, currCell;
	LongRect 	r, cellRect;
	Rect		qdRect;
	
	QDToFrameR( area, &r);
	
	SetLongRect( &cellRect, 0, 0, width, numCells * cellHeight);
	if ( !SectLongRect( &r, &cellRect, &r) )
		return;
	
	r.right--;						// Don't include cells not intended to be drawn
	r.bottom--;
	
	firstCell = FindHitCell( &topLeftL(r));				// get cell range to draw
	lastCell = FindHitCell( &botRightL(r));
	
	SetLongRect( &cellRect, 0, cellHeight * (firstCell - 1),
				 width, cellHeight * firstCell);
	
	for ( currCell = firstCell; currCell <= lastCell; currCell++ )
	{
		FrameToQDR( &cellRect, &qdRect);				// convert to screen rect
		DrawCell( currCell, &qdRect);
		OffsetLongRect( &cellRect, 0, cellHeight);		// move to next cell
	}
	
	if ( IsActive() && selectedCell >= firstCell && selectedCell <= lastCell )
		Hilite( selectedCell, TRUE);

}  /* CScrollList::Draw */


/*************************************************************************************
 DrawCell

	Draw theCell into the given cellRect.  Default only draws the check, if any.
	
		OVERRIDE this method.
*************************************************************************************/

void
CScrollList::DrawCell( short theCell, Rect *cellRect)

{
	if ( CHECKABLE && IsChecked( theCell) )					// Draw the check mark
	{
		MoveTo( cellRect->left + indent.h - kCheckWidth,
				cellRect->top + indent.v - 2);
		Line( 1, 1);
		Line( 4, -4);
	}

}  /* CScrollList::DrawCell */


/*************************************************************************************
 FrameCell

	Frame/unframe the specified cell in response to being dragged/released.
	Assumes the Pane is already Prepared.  Default just performs the same
	action as Hilite below.
*************************************************************************************/

void
CScrollList::FrameCell( short theCell, Boolean frameFlag)

{
	LongRect	pixels;
	Rect		visPixels;
	
	if ( !printing )
	{
		CellsToPixels( theCell, theCell, &pixels);
		if ( SectAperture( &pixels, &visPixels) )
		{
			SetHiliteMode();					// invert with user's hilite color
			InvertRect( &visPixels);
		}
	}

}  /* CScrollList::FrameCell */


/*************************************************************************************
 Hilite

	Hilite the specified cell.  Assumes the Pane is already Prepared.
	Does not hilite a cell being edited.  Only hilites if SELECTABLE.
*************************************************************************************/

void
CScrollList::Hilite( short theCell, Boolean hiliteFlag)

{
	LongRect	pixels;
	Rect		visPixels;
	
	if ( !printing && SELECTABLE && !(hiliteFlag && (fEditing || fStillEditing)) )
	{
		CellsToPixels( theCell, theCell, &pixels);
		if ( SectAperture( &pixels, &visPixels) )
		{
			SetHiliteMode();				// invert with user's hilite color
			InvertRect( &visPixels);
		}
	}

}  /* CScrollList::Hilite */


/*************************************************************************************
 RefreshCell

	Force an update event for the specified cell.
*************************************************************************************/

void
CScrollList::RefreshCell( short aCell)

{
	LongRect	r;
	
	if ( aCell > 0 && aCell <= numCells )
	{
		GetCellRect( aCell, &r);
		RefreshLongRect( &r);
	}

}  /* CScrollList::RefreshCell */


/*************************************************************************************
 RefreshCellRange

	Force an update event for the specified range of cells.
*************************************************************************************/

void
CScrollList::RefreshCellRange( short firstCell, short lastCell)

{
	LongRect	cellPixels;
	Rect		visArea;
	
	if ( CellsToPixels( firstCell, lastCell, &cellPixels) )
	{
		if ( SectAperture( &cellPixels, &visArea) )
			InvalRect( &visArea);
	}

}  /* CScrollList::RefreshCellRange */


/*************************************************************************************
 GetCellHeight

	Return the height of each cell.
*************************************************************************************/

short
CScrollList::GetCellHeight( void)

{
	return cellHeight;

}  /* CScrollList::GetCellHeight */


/*************************************************************************************
 GetNumCells

	Return the number of cells in the list.
*************************************************************************************/

short
CScrollList::GetNumCells( void)

{
	return numCells;

}  /* CScrollList::GetNumCells */


/*************************************************************************************
 GetCellRect

	Determine the pixel area spanned by theCell.
*************************************************************************************/

void
CScrollList::GetCellRect( short theCell, LongRect *cellRect)

{
	if ( theCell > 0 && theCell <= numCells )
		SetLongRect( cellRect, 0, cellHeight * (theCell - 1), width, cellHeight * theCell);
	else
		SetLongRect( cellRect, 0, 0, 0, 0);

}  /* CScrollList::GetCellRect */


/*************************************************************************************
 PixelsToCells

	Find the range of cells that a pixel area covers.
	
	RETURNS true of it covers any cells at all.
*************************************************************************************/

Boolean
CScrollList::PixelsToCells( LongRect *pixelsRect, short *firstCell, short *lastCell)

{
	short		fCell, lCell;
	
	fCell = FindHitCell( &topLeftL(*pixelsRect));
	lCell = FindHitCell( &botRightL(*pixelsRect));
	
	if ( fCell > numCells || lCell < 1 || fCell > lCell )
	{
		*firstCell = 0;
		*lastCell = 0;
		return FALSE;
	}
	else
	{
		*firstCell = Max( fCell, 1);
		*lastCell = Min( lCell, numCells);
		return TRUE;
	}

}  /* CScrollList::PixelsToCells */


/*************************************************************************************
 CellsToPixels

	Calculates the pixel area covered by a range of cells.
	
	RETURNS true if the range is valid.
*************************************************************************************/

Boolean
CScrollList::CellsToPixels( short firstCell, short lastCell, LongRect *pixelsRect)

{
	if ( lastCell >= firstCell )
	{
		SetLongRect( pixelsRect, 0, cellHeight * (firstCell - 1),
					 width, cellHeight * (lastCell));
		return TRUE;
	}
	else
	{
		SetLongRect( pixelsRect, 0, 0, 0, 0);
		return FALSE;
	}

}  /* CScrollList::CellsToPixels */


/*************************************************************************************
 FindHitCell

	Find the cell that contains hitPt.
*************************************************************************************/

short
CScrollList::FindHitCell( LongPt *hitPt)

{
	if ( (hitPt->h >= 0 && hitPt->h < width) &&
			(hitPt->v >= 0 && hitPt->v < bounds.bottom * cellHeight) )
		return (hitPt->v / cellHeight + 1);
	else
		return 0;

}  /* CScrollList::FindHitCell */


/*************************************************************************************
 ClickOutsideBounds

	Handle a click inside the frame but outside the list's bounds.
*************************************************************************************/

void
CScrollList::ClickOutsideBounds( Point hitPt, short modifierKeys, long when)

{
	SelectCell( 0, kDoRedraw);

}  /* CScrollList::ClickOutsideBounds */


/*************************************************************************************
 SelectCell

	Deselects the previous selected cell if there is one and selects the new cell.
	Doesn't reselect the same cell.  Handles all hiliting.
	Use 0 for aCell to deselect all cells.
*************************************************************************************/

void
CScrollList::SelectCell( short aCell, Boolean reDraw)

{
	tSelectionInfo		info;
	
	if ( aCell >= 0 && aCell <= numCells && aCell != selectedCell )
	{
		info.prevSelection = selectedCell;			// store change info
		info.newSelection = aCell;
		
		if ( reDraw )
		{
			Prepare();
			
			if ( selectedCell )
				Hilite( selectedCell, FALSE);		// unhilite old selected cell
			
			selectedCell = aCell;					// store new selected cell
			
			if ( aCell )
				Hilite( aCell, TRUE);				// hilite new selected cell
		}
		else										// just invalidate
		{
			if ( selectedCell )
				RefreshCell( selectedCell);
			
			selectedCell = aCell;					// store new selected cell
			
			if ( aCell )
				RefreshCell( aCell);
		}
		
		BroadcastChange( scrollListSelectionChanged, &info);
	}

}  /* CScrollList::SelectCell */


/*************************************************************************************
 GetSelectedCell

	RETURNS the selected cell or zero if none.
*************************************************************************************/

short
CScrollList::GetSelectedCell( void)

{
	return selectedCell;

}  /* CScrollList::GetSelectedCell */


/*************************************************************************************
 ScrollToSelection  {OVERRIDE}

	Scroll the selected cell into view.  Also tries to eliminate any empty space
	between the last cell in the list and the bottom of the frame.
*************************************************************************************/

void
CScrollList::ScrollToSelection( void)

{
	LongPt		selPos;
	short		hSpan, vSpan;
	
	GetFrameSpan( &hSpan, &vSpan);				// get currently visible cell range
	selPos = position;
	
	if ( selectedCell )
	{
		if ( selectedCell <= selPos.v )
			selPos.v = selectedCell - 1;
		else if ( selectedCell > selPos.v + vSpan )
			selPos.v = selectedCell - vSpan;
	}
			// try to minimize unsightly empty space at bottom of list frame
	
	if ( selPos.v + vSpan > numCells )
		selPos.v = Max( 0, numCells - vSpan);
	
	if ( selPos.v != position.v )				// only scroll if we actually moved
		ScrollTo( &selPos, TRUE);

}  /* CScrollList::ScrollToSelection */


/*************************************************************************************
 DoKeyDown  {OVERRRIDE}

	Handle moving the selected cell up or down one space.  If command key is
	down, move to top/bottom of list appropriately.
*************************************************************************************/

void
CScrollList::DoKeyDown( char theChar, Byte keyCode, EventRecord *macEvent)

{
	short		newCell = 0;
	short		currCell;
	Boolean		commandKey = ((macEvent->modifiers & cmdKey) != 0);
	
	if ( (keyCode == KeyUpCursor || keyCode == KeyDownCursor) && itsArray )
	{
		switch ( keyCode )
		{
			case  KeyUpCursor:
				newCell = commandKey ? 1 : selectedCell > 1 ? selectedCell - 1 : 1;
				break;
			
			case  KeyDownCursor:
				newCell = commandKey ? numCells : selectedCell < numCells
									 && selectedCell ? selectedCell + 1 : numCells;
				break;
		}
		
		if ( newCell != selectedCell )
		{
			SelectCell( newCell, TRUE);				// select and display new cell
			ScrollToSelection();
		}
	}
	else if ( EDITABLE && itsArray )
	{
				// Handle keystrokes to initiate adding, deleting, and editing
				// of the selected cell.
	
		switch ( theChar )
		{
			case  kEscapeOrClear:
			case  kDeleteKey:
				DoDeleteCell();
				break;
			
			case  kEnterKey:
				DoModifyCell();
				break;
			
			case  kReturnKey:
				DoAddCell();
				break;
			
			default:
				inherited::DoKeyDown( theChar, keyCode, macEvent);
				break;
		}
	}
	else
		inherited::DoKeyDown( theChar, keyCode, macEvent);

}  /* CScrollList::DoKeyDown */


/*************************************************************************************
 DoAutoKey  {OVERRRIDE}

	Bottleneck right through DoKeyDown.
*************************************************************************************/

void
CScrollList::DoAutoKey( char theChar, Byte keyCode, EventRecord *macEvent)

{
	DoKeyDown( theChar, keyCode, macEvent);

}  /* CScrollList::DoAutoKey */


/*************************************************************************************
 DoAddCell

	Add a new cell after the selected cell.  If no cell is selected, append it
	onto the end of the list.  Immediately calls DoModifyCell after adding.
*************************************************************************************/

void
CScrollList::DoAddCell( void)

{
	short		newCell;
	
	ScrollToSelection();
	
	if ( fEditing )
	{
		DoAddAnotherCell();
		return;
	}
	
	if ( selectedCell < 1 )						// choose new cell position
		newCell = numCells + 1;
	else
		newCell = selectedCell + 1;
	
	SelectCell( 0, kDoRedraw);					// get rid of hiliting on old cell
	DoInsertCell( newCell);						// create the blank new cell
	SelectCell( newCell, kDontRedraw);			// select new cell
	ScrollToSelection();
	
	DoModifyCell();

}  /* CScrollList::DoAddCell */


/*************************************************************************************
 DoAddAnotherCell

	Add a new cell after the selected cell.  Used when use does add cell while
	still editing a cell.
*************************************************************************************/

void
CScrollList::DoAddAnotherCell( void)

{
	short		newCell;
	
	newCell = selectedCell + 1;					// choose new cell position
	
	fStillEditing = TRUE;
	this->BecomeGopher( TRUE);					// finish editing
	fStillEditing = FALSE;
	
	fEditing = TRUE;
	DoInsertCell( newCell);						// create the blank new cell
	SelectCell( newCell, kDontRedraw);			// select new cell
	ScrollToSelection();
	
	BeginEditing();

}  /* CScrollList::DoAddAnotherCell */


/*************************************************************************************
 DoModifyCell

	Modify the selected cell, if there is one.  Makes the appropriate call to
	BeginEditing.
*************************************************************************************/

void
CScrollList::DoModifyCell( void)

{
	ScrollToSelection();
	
	if ( fEditing )
	{
		this->BecomeGopher( TRUE);
		return;
	}
	
	if ( selectedCell < 1 || selectedCell > numCells )
		return;
	
	Hilite( selectedCell, FALSE);
	BeginEditing();

}  /* CScrollList::DoModifyCell */


/*************************************************************************************
 DoDeleteCell

	Delete the selected cell if there is one.  Sends message to array so the
	array will send a change message to us.
*************************************************************************************/

void
CScrollList::DoDeleteCell( void)

{
	if ( fEditing )
		this->BecomeGopher( TRUE);
	
	if ( selectedCell )
		itsArray->DeleteItem( selectedCell);

}  /* CScrollList::DoDeleteCell */


/*************************************************************************************
 DoInsertCell

	Insert a blank cell before beforeCell.  Default does nothing.  Subclass method
	should create a new block and call itsArray->InsertAtIndex( &data, beforeCell).
	
		OVERRIDE this method.
*************************************************************************************/

void
CScrollList::DoInsertCell( short beforeCell)

{

}  /* CScrollList::DoInsertCell */


/*************************************************************************************
 BeginEditing

	Setup for editing the selected cell.  Default only sets the editing flag.
	Subclasses should create any subpanes needed to edit the selected cell.
	Call this method after setting up subpanes.
	
		OVERRIDE this method.
*************************************************************************************/

void
CScrollList::BeginEditing( void)

{
	SetupCellData();
	fEditing = TRUE;

}  /* CScrollList::BeginEditing */


/*************************************************************************************
 DoneEditing

	The user has finished editing the cell.  Default resets the editing flag.
	Subclasses should call the inherited method and then remove any subpanes
	that they added for editing.
	
		OVERRIDE this method.
*************************************************************************************/

void
CScrollList::DoneEditing( void)

{
	LongRect 	cellRect;
	Rect		qdRect;
	
	fEditing = FALSE;
	RetrieveCellData();
	
/*	Prepare();
	GetCellRect( selectedCell, &cellRect);					// Redraw the cell
	FrameToQDR( &cellRect, &qdRect);
	
	EraseRect( &qdRect);
	DrawCell( selectedCell, &qdRect);
	ValidRect( &qdRect);
	
	if ( !fStillEditing )
		Hilite( selectedCell, TRUE);
*/
}  /* CScrollList::DoneEditing */


/*************************************************************************************
 SetupCellData

	Get the cell data from the array and place it into editing subpanes.
	Default does nothing.
	
		OVERRIDE this method.
*************************************************************************************/

void
CScrollList::SetupCellData( void)

{

}  /* CScrollList::SetupCellData */


/*************************************************************************************
 RetrieveCellData

	Retrieve the edited cell data from the editing supanes and place it back into
	the array.  Default does nothing.
	
		OVERRIDE this method.
*************************************************************************************/

void
CScrollList::RetrieveCellData( void)

{

}  /* CScrollList::RetrieveCellData */


/*************************************************************************************
 SetDrawActiveBorder

	Specify whether or not the list has an active border.
*************************************************************************************/

void
CScrollList::SetDrawActiveBorder( Boolean fDrawActiveBorder)

{
	drawActiveBorder = fDrawActiveBorder;

}  /* CScrollList::SetDrawActiveBorder */


/*************************************************************************************
 BecomeGopher  {OVERRIDE}

	If we are becoming the gopher, and drawActiveBorder is true, draw a two pixel
	thick border around the scroll pane.
	If we are losing our gophership, remove any border around the scroll pane.
*************************************************************************************/

Boolean
CScrollList::BecomeGopher( Boolean fBecoming)

{
	if ( itsArray == NULL && fBecoming )
		return FALSE;
	
	if ( !inherited::BecomeGopher( fBecoming) )
		return FALSE;
	
	if ( drawActiveBorder && itsScrollPane )
		MakeBorder();
	
	return TRUE;

}  /* CScrollList::BecomeGopher */


/*************************************************************************************
 MakeBorder

	Create a border or take it away based on fActive.
*************************************************************************************/

void
CScrollList::MakeBorder( void)

{
	CPaneBorder 	*border;
	Rect			margin;
	
	if ( (active && gGopher == this) || fEditing )
	{
		if ( !itsScrollPane->GetBorder() )
		{
			border = new( CPaneBorder);
			border->IPaneBorder( kBorderFrame);
			SetRect( &margin, -kBorderMargin, -kBorderMargin,
						kBorderMargin, kBorderMargin);
			border->SetPenSize( kBorderPen, kBorderPen);
			border->SetMargin( &margin);
			
			itsScrollPane->SetBorder( border);
		}
	}
	else
		itsScrollPane->SetBorder( NULL);

}  /* CScrollList::MakeBorder */


/*************************************************************************************
 Activate  {OVERRRIDE}

	Hilite the selected cell, if there is one.
*************************************************************************************/

void
CScrollList::Activate( void)
{
	if ( !active )
	{
		Prepare();
		Hilite( selectedCell, TRUE);
	}
	
	if ( drawActiveBorder && itsScrollPane )
		MakeBorder();
	
	inherited::Activate();

}  /* CScrollList::Activate */


/*************************************************************************************
 Deactivate  {OVERRRIDE}

	Unhilite the selected cell, if there is one.
*************************************************************************************/

void
CScrollList::Deactivate( void)

{
	if ( active )
	{
		if ( fEditing )
			this->BecomeGopher( TRUE);
		
		Prepare();
		Hilite( selectedCell, FALSE);
	}
	
	if ( drawActiveBorder && itsScrollPane )
		MakeBorder();
	
	inherited::Deactivate();

}  /* CScrollList::Deactivate */


/*************************************************************************************
 Dispose  {OVERRRIDE}

	Dispose of the array of checks, if it exists.  ====
*************************************************************************************/

void
CScrollList::Dispose( void)

{
	ForgetObject( itsChecks);
	inherited::Dispose();

}  /* CScrollList::Dispose */


/*************************************************************************************
 SetArray

	Associate a new array with this scroll list.  If anArray is NULL, don't allow
	this list to become the gopher.
*************************************************************************************/

void
CScrollList::SetArray( CArray *anArray)

{
	short		deltaRows;
	LongPt		topLeft;
	
	if ( itsArray )
	{
		CancelDependency( itsArray);
		ForgetObject( itsChecks);
		SetLongPt( &topLeft, 0, 0);
		ScrollTo( &topLeft, kNoRedraw);
	}
	
	itsArray = anArray;
	SetCanBeGopher( anArray != NULL);
	
	if ( anArray )
	{
		DependUpon( itsArray);
		selectedCell = 0;
		
				// adjust the list to have the same cells as the array
		
		deltaRows = itsArray->GetNumItems() - numCells;
		
		if ( deltaRows > 0 )
			AddCell( deltaRows, numCells);
		else if ( deltaRows < 0 )
			DeleteCell( -deltaRows, numCells + deltaRows + 1);
	
		if ( CHECKABLE )
		{
			itsChecks = new CStateArray;
			itsChecks->IStateArray( numCells, 0);
		}
	}
	else
	{
		numCells = 0;
		selectedCell = 0;
		AdjustBounds();
	}
	
	Refresh();

}  /* CScrollList::SetArray */

/*************************************************************************************
 GetArray

	Return a reference to itsArray.
*************************************************************************************/

CArray *
CScrollList::GetArray( void)

{
	return itsArray;

}  /* CScrollList::GetArray */


/*************************************************************************************
 ProviderChanged

	Handle a change to itsArray.
*************************************************************************************/

void
CScrollList::ProviderChanged( CCollaborator *aProvider, long reason, void* info)

{
	tMovedElementInfo	*moveInfo;
	short				firstCell, lastCell;
	long				infoIndex = *(long*) info;
	short				state = 0;
	
	if ( aProvider == itsArray )
	{
		switch( reason )
		{
			case  arrayInsertElement:			// add a new cell
				if ( CHECKABLE )
					itsChecks->InsertAtIndex( &state, infoIndex);
				
				AddCell( 1, infoIndex-1);
				break;
				
				
			case  arrayDeleteElement:			// delete its cell
				if ( CHECKABLE )
					itsChecks->DeleteItem( infoIndex);
				
				DeleteCell( 1, infoIndex);
				break;
				
				
			case  arrayMoveElement:				// redraw all cells encompassing
												// its old and new positions
				moveInfo = (tMovedElementInfo *) info;
				
				if ( CHECKABLE )
					itsChecks->MoveItemToIndex( moveInfo->originalIndex,
												moveInfo->newIndex);
				
				firstCell = Min( moveInfo->originalIndex, moveInfo->newIndex);
				lastCell = Max( moveInfo->originalIndex, moveInfo->newIndex);
				RefreshCellRange( firstCell, lastCell);
				break;
				
				
			case arrayElementChanged:			// redraw its cell
				{
					LongRect	cellRect;
					Rect		visArea;
					
					Prepare();
					GetCellRect( infoIndex, &cellRect);
					if ( SectAperture( &cellRect, &visArea) )
					{
						EraseRect( &visArea);
						DrawCell( infoIndex, &visArea);
						if ( infoIndex == selectedCell )
							Hilite( selectedCell, TRUE);
						ValidRect( &visArea);
					}
				}
				
//				RefreshCell( infoIndex);
				break;	
				
				
			default:
				inherited::ProviderChanged( aProvider, reason, info);
				break;			
		}
	}
	else
		inherited::ProviderChanged( aProvider, reason, info);

}  /* CScrollList::ProviderChanged */
