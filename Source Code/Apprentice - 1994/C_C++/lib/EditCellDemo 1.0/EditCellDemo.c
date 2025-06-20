/******************************************************************************
 EditCellDemo.c
	
	This is a simple example of editing a CTable cell inline (ie over the
	cell in the table itself). This demo is not a framework around which to
	develop an application, but rather a demo of the mechanics of using
	a "floating" editor and using BecomeGopher().
	
	To run this demo
	
	1)	Add EditCellDemo.c and EditCellDemo.rsrc files to a new segment of 
		the Showcase Application (v 1.2a or later).
		
	2)	Compile and Run.
	
	AUTHOR: Andrew_Gilmartin@Brown.Edu
	MODIFIED: 93-10-25

******************************************************************************/


#include <CDecorator.h>
#include "CSApplication.h"
#include "EditCellDemo.h"


extern CDesktop *gDesktop;
extern CDecorator *gDecorator;



/******************************************************************************
 IEditCell

	Initialize the cell editor. This is a simple subclass of CEditText with
	methods to position itself over a cell and to hide and show itself 
	approp.
******************************************************************************/

void CEditCell::IEditCell( CView *anEnclosure, CBureaucrat *aSupervisor )
{
	enum { kBorderMargin = 1, kBorderPen = 2 };
	CPaneBorder *theBorder;
	Rect theMargin;

		/* Init the edit text */

	IEditText
		( anEnclosure
		, aSupervisor
		, 0 // size will be set as needed
		, 0
		, 0 // position will be set as needed
		, 0
		, sizFIXEDSTICKY // we want the editor to scroll with the table
		, sizFIXEDSTICKY
		, 0 );

		/* Create a border for the editor */
		
	SetWholeLines( FALSE ); // Make sure the border surounds the whole cell

	theBorder = new CPaneBorder;
	theBorder->IPaneBorder( kBorderFrame );
	SetRect
		( &theMargin
		, -kBorderMargin
		, -kBorderMargin
		, kBorderMargin
		, kBorderMargin );
	theBorder->SetPenSize( kBorderPen, kBorderPen );
	theBorder->SetMargin( &theMargin );
	
	SetBorder( theBorder );

		/* Edit the editor until needed */
		
	Hide();
	
} /* IEditCell */



/******************************************************************************
 PlaceCell

	Move the pane over the cell's boarders rectangle. HACK: The + 2 and + 1
	offsets in the Place() command are just so the CEditText overlays the
	text drawn by CTable's DrawCell().
******************************************************************************/

void CEditCell::PlaceCell( LongRect *aPlace )
{
	Rect delta;
	
		/* Place the editor over the cell */
		
	Place( aPlace->left + 2, aPlace->top + 1, FALSE );
		
		/* Size the editor to cover the cell */
		
	delta.top = 0;
	delta.left = 0;
	delta.bottom = (aPlace->bottom - aPlace->top) - height;
	delta.right = (aPlace->right - aPlace->left) - width;

	ChangeSize( &delta, TRUE );

} /* PlaceCell */



/******************************************************************************
 BecomeGopher

	Show or hide the edit pane.
******************************************************************************/

Boolean CEditCell::BecomeGopher( Boolean isBecoming )
{
		/* Don't change setting unless we can become the gopher */
		
	if ( ! inherited::BecomeGopher( isBecoming ) )
		return FALSE;
		
		/* Show or hide the editor appropriatly */
		
	if ( isBecoming )
	{
		Show();
	}
	else
	{
		Hide();
	}

	return TRUE;

} /* BecomeGopher */



/******************************************************************************
 IEditTable

	Initilaize the edit table. This example uses one cell editor that is 
	shared by all the cells.
******************************************************************************/

void CEditTable::IEditTable
	( CView *anEnclosure
	, CBureaucrat *aSupervisor
	, short aWidth
	, short aHeight
	, short aHEncl
	, short aVEncl
	, SizingOption aHSizing
	, SizingOption aVSizing )
{
	ITable
		( anEnclosure, aSupervisor
		, aWidth, aHeight
		, aHEncl, aVEncl
		, aHSizing, aVSizing );
	
	itsEditor = new CEditCell;
	itsEditor->IEditCell( this, this );

} /* IExampleTable */



/******************************************************************************
 DoClick

	Change editing focus to the clicked cell. NOTE: CTable's DoClick() is 
	not called so its selection stuff is not used.
******************************************************************************/

void CEditTable::DoClick( Point hitPt, short modifierKeys, long when)
{
	LongPt pt;
	Cell hitCell;

		/* Did the user click in a cell? */
		
	QDToFrame( hitPt, &pt );
	
	if ( PtInLongRect( &pt, &bounds) )
	{
			/* Finish editing the current cell */
			
		if ( itsEditor == gGopher )
		{
			if ( ! itsEditor->BecomeGopher( FALSE ) ) // Make sure can change focus
			{
				return;
			}
		}
		
			/* Edit clicked cell */
			
		FindHitCell( &pt, &hitCell );
		EditCell( hitCell );
		
		itsEditor->BecomeGopher( TRUE );
	}
	else
	{
		ClickOutsideBounds( hitPt, modifierKeys, when );
	}

} /* DoClick */



/******************************************************************************
 EditCell

	Method to move and configure the cell editor. In this case we are using
	a shared cell editor, but this call could create a cell specific editor
	if necessary. The only requirement would be that itsEditor be set.
******************************************************************************/

void CEditTable::EditCell( Cell editCell )
{
	LongRect cellRect;
	Str255 cellText;
	
		/* Position the floating editor */
		
	GetCellRect( editCell, &cellRect );
	itsEditor->PlaceCell( &cellRect );
	
		/* Initialize the floating editor's content */

	GetCellText( editCell, cellRect.right - cellRect.left, cellText );
	itsEditor->SetTextString( cellText );

} /* EditCell */



/******************************************************************************
 INewDemo

	Build the edit table demo window.
******************************************************************************/

void CEditTableDemoDir::INewDemo( CDirectorOwner *aSupervisor )
{
	CScrollPane* theScrollPane = NULL;
	CEditTable* theTable = NULL;

	inherited::INewDemo( aSupervisor );
	
	itsWindow = new CWindow;
	itsWindow->IWindow( 128, FALSE, gDesktop, this );

	theScrollPane = new CScrollPane;
	theScrollPane->IScrollPane
		( itsWindow, this
		, 0, 0
		, 0, 0
		, sizELASTIC, sizELASTIC
		, TRUE, TRUE, TRUE );
	theScrollPane->FitToEnclFrame( TRUE, TRUE );
		
	theTable = new CEditTable;
	theTable->IEditTable
		( theScrollPane, this
		, 0, 0
		, 0, 0
		, sizELASTIC, sizELASTIC );
	theTable->FitToEnclosure( TRUE, TRUE );

	theTable->SetDefaults( 72, -1 );
	theTable->AddCol( 25, -1 );
	theTable->AddRow( 25, -1 );

	theScrollPane->InstallPanorama( theTable );
	
	gDecorator->StaggerWindow( itsWindow );
	itsWindow->Select();

} /* INewDemo */