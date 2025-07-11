/*************************************************************************************

 CScrollListDemoDir.c
	
		Director to create and manage the CScrollList demo window.
	
	SUPERCLASS = CShowcaseDemoDir
	
	REQUIRES : CScrollList
	
		� 1992 Dave Harkness

*************************************************************************************/


/*************************************************************************************

	To install this Showcase demo, follow these two easy steps:
	
		1)  Add all source files to the CSApplication project.
		
				CEditString
				CStringArray
				CStringEditList
			 *	CScrollList
				CScrollListDemoDir
			 *	CScrollListDragger
			 *	CStateArray
			
			[ Those marked with an * are the files necessary when using
			  the CScrollList class.  The other four are only for this demo. ]
	
		2)	Add all resources in "CScrollListDemo.rsrc" to the
			Showcase resource file "CSApplication �.rsrc".

*************************************************************************************/


#include "CScrollListDemoDir.h"
#include "CStringEditList.h"
#include "CScrollList.h"
#include "CStringArray.h"

#include <CScrollPane.h>
#include <CPaneBorder.h>
#include <CWindow.h>
#include <CDesktop.h>
#include <CDecorator.h>


extern CDesktop		*gDesktop;
extern CDecorator	*gDecorator;

CursHandle		gCheckCursor, gHandCursor;


/*************************************************************************************
 INewDemo
*************************************************************************************/

void
CScrollListDemoDir::INewDemo( CDirectorOwner *aSupervisor)

{
	CScrollPane			*scrollPane;
	CStringEditList		*editList;
	CPaneBorder			*listBorder;
	Rect				sizeRect;
	
	inherited::INewDemo( aSupervisor);
	
														// Get cursors for CScrollList
	gCheckCursor = GetCursor( kCheckCursorID);
	FailNILRes( gCheckCursor);
	HNoPurge( (Handle) gCheckCursor);
	gHandCursor = GetCursor( kHandCursorID);
	FailNILRes( gHandCursor);
	HNoPurge( (Handle) gHandCursor);
	
														// Create the window
	itsWindow = new(CWindow);
	itsWindow->IWindow( kWINDScrollListDemo, false, gDesktop, this);

														// Create the scrollPane
	scrollPane = new CScrollPane;
	scrollPane->IViewRes( 'ScPn', kScPnScrollListDemo, itsWindow, this);
	scrollPane->FitToEnclosure( true, true);
	SetRect( &sizeRect, -1, -1, 1, 1);
	scrollPane->ChangeSize( &sizeRect, false);
	
														// Create the scrollList
	editList = new CStringEditList;
	editList->IStringEditList( scrollPane, this, 0, 0, 0, 0,
							   sizELASTIC, sizELASTIC,
							   kSLSelectable | kSLDragable | kSLCheckable | kSLEditable);
	
														// Setup the scrollList
	editList->FitToEnclosure( true, true);
	editList->SetDrawActiveBorder( false);
	
														// Create the listBorder
	listBorder = new( CPaneBorder);
	listBorder->IPaneBorder( kBorderFrame);
	editList->SetBorder( listBorder);
	
														// Create the array
	itsArray = new CStringArray;
	itsArray->IRes( kSTRScrollListDemo);
	editList->SetArray( itsArray);
	editList->CheckCell( 15);
	editList->CheckCell( 16);
	editList->CheckCell( 18);
	editList->CheckCell( 19);
	
														// Install the scrollList
	scrollPane->InstallPanorama( editList);
	itsGopher = editList;
	editList->SelectCell( 1, kDontRedraw);
	
														// Show the window
	gDecorator->CenterWindow(itsWindow);
	itsWindow->Select();

}  /* CScrollListDemoDir::INewDemo */


/*************************************************************************************
 Dispose  {OVERRIDE}
*************************************************************************************/

void
CScrollListDemoDir::Dispose( void)

{
	CStringArray	*anArray = itsArray;
	
	inherited::Dispose();
	anArray->Dispose();

}  /* CScrollListDemoDir::Dispose */
