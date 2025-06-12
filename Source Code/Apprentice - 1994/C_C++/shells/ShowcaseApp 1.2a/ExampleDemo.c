/******************************************************************************
 ExampleDemo.c
	
	This is a simple example of showcasing a subclass of CPane (all it does is 
	fill its content with gray).
	
	AUTHOR: Andrew_Gilmartin@Brown.Edu
	MODIFIED: 93-10-21

******************************************************************************/


#include <CDecorator.h>
#include "CSApplication.h"
#include "ExampleDemo.h"


void CExamplePane::IExamplePane
	( CView *anEnclosure
	, CBureaucrat *aSupervisor
	, short aWidth
	, short aHeight
	, short aHEncl
	, short aVEncl
	, SizingOption aHSizing
	, SizingOption aVSizing )
{
	IPane
		( anEnclosur
		, aSupervisor
		, aWidth
		, aHeight
		, aHEncl
		, aVEncl
		, aHSizing
		, aVSizing );

} /* IExamplePane */



void CExamplePane::Draw( Rect *area )
{
	FillRect( area, ltGray );

} /* Draw */



void CExampleDemoDir::INewDemo( CDirectorOwner *aSupervisor )
{
	extern CDesktop *gDesktop;
	extern CDecorator *gDecorator;

	inherited::INewDemo( aSupervisor );
	
	itsWindow = new CWindow;
	itsWindow->IWindow( 128, FALSE, gDesktop, this );

	itsPane = new CExamplePane;
	itsPane->IExamplePane
		( itsWindow
		, itsWindow->itsSupervisor
		, 0
		, 0
		, 0
		, 0
		, sizELASTIC
		, sizELASTIC );
	itsPane->FitToEnclFrame( TRUE, TRUE );
	
	gDecorator->StaggerWindow( itsWindow );
	itsWindow->Select();

} /* INewDemo */



