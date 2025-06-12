/******************************************************************************

 CSApplication.c

	A Network group project, brought to you by the good folks at TCL-Talk and
	comp.sys.mac.oop.tcl.
	
	
	DOCUMENTATION
	
	An application from which example classes can be launched.  Designed to allow 
	simple addition of example classes and a demo area using resources.
	
	Suggestion: Create a custom Director, make a window from the Init code, make
	an instance or more of your new class. Show off!
	
		
	APPLICATION INFO

	This app is based on the generic starter application supplied with Think 6.0. 
	Current revisions as of the last compile are:
	
			THINK C 				6.0.1		
			TCL						1.1.3


	ADDING NEW DEMOS
			
	Step 1) Add all necessary source and resource files to this project in a 
			new segment.
	
	Step 2) Follow any additional instructions in the Read Me file (such as
			adding non-core TCL classes that aren't already in the project).

	Step 3) Compile the Showcase Application. During compilation, the compiler
			may complain about duplicate EXCs resources. Since the ID is not
			significant, just use ResEdit to change the EXCs to a random number
			and then try compiling again. 
			
			NOTE: THINK 5 allowed only one .rsrc file per project; you had 
			the merge all the resources of TCL and your application into this
			file. TPM 6, however, allows many resource files and it will build
			the project's resource file for you. Because of this, old Showcase
			Application demos will give other duplicate resource errors because
			they often have the demo's, Showcase's, and TCL's resources all in
			the demo's resource file. To fix this, remove from the demo's 
			resource file all the non-EXCs duplicate reosurces. When all else
			fails, build just one Showcase EXCs at a time.

	Step 4) Run the Showcase Application and enjoy the show!
	
	NOTE: If you want to build the application, see the note for the 
	ForceClassReferences method.
	
		
	CREATE NEW DEMOS
	
	To create a demo for your class, you must subclass the CShowcaseDemoDir class 
	and then override the INewDemo method.  See the end of the CSApplication.h 
	header file for the CShowcaseDemoDir class. Name your subclass something like 
	"CNewClassDemoDir" where NewClass is the name of the class you are demoing. 
	Your subclass should create a window (or whatever) in which to demo your new 
	class.
	
	Next, create a resource file and add an EXCs resource for your demo.
	It has the following format (there is a TMPL for the EXCs included in
	CSApplication.rsrc)
	
		PSTR	DemoDirector Class Name
						Name of your CShowcaseDemoDir subclass
		PSTR	Class Name
						Name of the class you are demoing
		PSTR	Class Description
						Brief description of the class you are demoing
		PSTR	Author Name
						Name of the class's author
		PSTR	Author Address
						E-Mail address of the author
	
	Finally, add any other support resources necessary to make your class work.
	Include all the classes required for your class (except those that are
	already in the THINK Class Library).
	
	Take a look at the examples already on the archive to see how it's done.
	This was made as simple as possible, so it should only take a few minutes
	to put something basic together, but don't be afraid to show off a little! :)

	NOTE: Use obscure IDs for your resources.


	WORKER BEES
	
	Brian Hamlin
	bhamlin@netcom.com
	
	David Harkness
	harkness@netcom.com

	Andrew Gilmartin
	Andrew_Gilmartin@Brown.Edu
 
	
	VERSION HISTORY
	
	1.2a	93-10-21	ajg
	-----------------------
	Modified project and source to work with THINK C 6.0 and TCL 1.1.3. I have not
		tried using it with a SC++ demo class. Very minor testing of changes!
	Updated the documentation.

	1.1a	92-06-06	djh
	-----------------------
	Fixed some bugs
	Added CShowcaseDemoDir for creating demos
	Removed demo command number method and replaced with new_by_name
	Added the Showcase menu (I'm too lazy to double-click in a list ;)
	Modified the NewClassDemo window a little
	Wrote the "Name/Author/Address Extraction" routines
		(Hey, do I still get the wonderful no-prize?)
	
	1.0a	92-05-08    bh
	----------------------
	Created the NewClassDemo window
	Layed the foundation and got the project running
	Hmm... Since I wasn't there when he wrote his code, how am I supposed to
		comment on what revisions he made??

	COPYRIGHT
	
	Copyright (C) 1992, 1993 by TCL-Talk & comp.sys.mac.oop.tcl.
	
	Permission to use, copy, modify, and distribute this software and its
	documentation for any purpose and without fee is hereby granted, provided
	that the above copyright notice appear in all copies and that both that
	copyright notice and this permission notice appear in supporting
	documentation.  This software is provided "as is" without express or
	implied warranty.

******************************************************************************/

#include <CDecorator.h>
#include <CWindow.h>
#include <CScrollPane.h>
#include <CPicture.h>
#include <CEditText.h>
#include <CPaneBorder.h>
#include <CBartender.h>
#include <Global.h>
#include <Commands.h>
#include "CSApplication.h"


/******************************************************************************
	CSHOWCASEDEMODIR SUBCLASS #INCLUDE FILES, IF NECESSARY   (see note above)
******************************************************************************/




/******************************************************************************
	GLOBALS
******************************************************************************/

extern CApplication	*gApplication;
extern CBartender	*gBartender;
extern CDesktop		*gDesktop;
extern CDecorator	*gDecorator;
extern CursHandle gWatchCursor;

Handle		*demoClasses;
short		numDemos;


/******************************************************************************
	MAIN
******************************************************************************/
void main()

{
	CSApplication	*Application;					

	Application = new CSApplication;
	
	Application->ISApplication();
	Application->Run();
	Application->Exit();
}


/******************************************************************************
	App::IApplication
******************************************************************************/
void CSApplication::ISApplication(void)
{
	CApplication::IApplication( kExtraMasters, kRainyDayFund, 
								kCriticalBalance, kToolboxBalance);
	
	itsNewDocWindow = NULL;
}


/******************************************************************************
	App::CreateDocument
******************************************************************************/
void CSApplication::CreateDocument(void)
{
	if ( itsNewDocWindow )
	{
		itsNewDocWindow->GetWindow()->Select();
		return;
	}
	
	TRY
	{
		itsNewDocWindow = new( CSDirector);
		itsNewDocWindow->ISDirector();
	}
	CATCH
	{
		ForgetObject( itsNewDocWindow);
	}
	ENDTRY
}


/******************************************************************************
	App::RemoveDirector
******************************************************************************/
void CSApplication::RemoveDirector( CDirector *aDirector)
{
	if ( aDirector == itsNewDocWindow )
		itsNewDocWindow = NULL;
	
	inherited::RemoveDirector( aDirector);
}


/******************************************************************************
	App::SetUpMenus
******************************************************************************/
void CSApplication::SetUpMenus()
{
	Handle			rH;
	short			index;
	unsigned char	*cp;
	
	inherited::SetUpMenus();
	gBartender->SetDimOption( kMENUShowcase, dimNONE);
		
	numDemos = Count1Resources( kInfoResType);
	
	if ( numDemos < 1 )
		ExitToShell();
	
	demoClasses = (Handle *)NewPtr( sizeof(Handle) * numDemos);
	
	if ( demoClasses )
		for ( index = 1; index <= numDemos; index++ )
		{
			rH = GetIndResource( kInfoResType, index);
			demoClasses[index-1] = rH;						// Store handle
			HLock( rH);
			cp = (unsigned char *)*rH;
			cp += *cp + 1;
															// Place in menu
			gBartender->InsertMenuCmd( cmdOpenShowcase+index, cp,
									   kMENUShowcase, index-1);
			HUnlock( rH);
		}
	else
		ExitToShell();
}


/******************************************************************************
	App::DoCommand
******************************************************************************/
void CSApplication::DoCommand(long theCommand)
{
	CShowcaseDemoDir	*demoDir;
	Handle				rH;
	unsigned char		*cp;
	char				cstr[256];
	short				index;
	
	if ( theCommand > cmdOpenShowcase && theCommand <= cmdLastShowcase )
	{
		SetCursor( *gWatchCursor );

		index = theCommand - cmdOpenShowcase;
		
		rH = demoClasses[index-1];
		LoadResource( rH);							// Make sure it's there!
		HLock( rH);
		
		cp = (unsigned char *)*rH;					// Reference to class name
		BlockMove( cp+1, cstr, cp[0]);
		cstr[cp[0]] = '\0';
		demoDir = new_by_name( cstr);
		if ( !demoDir )
			Failure( paramErr, excNewByNameFailed);
		
		HUnlock( rH);
		ASSERT( member( demoDir, CShowcaseDemoDir));		// Just to be sure
		
		demoDir->INewDemo( this);
	}
	else
		inherited::DoCommand( theCommand);
}


/******************************************************************************
	App::UpdateMenus
******************************************************************************/
void CSApplication::UpdateMenus()
{
	inherited::UpdateMenus();
}


/******************************************************************************
	App::ForceClassReferences
	
		You only need to add a dummy reference for each CShowcaseDemoDir
		_if_ you build an application.  The classes will be stripped by
		the Smart Linker, but not by the compiler alone.
		Of course, if you do this, don't forget to #include their respective
		header files at the top of this source file.
******************************************************************************/
void CSApplication::ForceClassReferences( void)
{
				// All CShowcaseDemoDir objects are created with new_by_name
	Boolean	alwaysFalse = FALSE;
	CObject *dummy;
	
	if (alwaysFalse)
	{
		member( dummy, CShowcaseDemoDir);				// E X A M P L E
		
		
	}
	
	inherited::ForceClassReferences();
}


/******************************************************************************
	App::Exit
******************************************************************************/
void CSApplication::Exit()
{
	
}



//*****************************************************************************
														//  D I R E C T O R


/******************************************************************************
	Dir::ISDirector
******************************************************************************/
void CSDirector::ISDirector()
{
	inherited::IDirector( gApplication);

	itsWindow = new(CWindow);
	itsWindow->IWindow( 500, FALSE, gDesktop, this);
	gDecorator->CenterWindow(itsWindow);

	itsPane = new( CSPane);
	itsPane->ISPane( itsWindow, this );
	itsWindow->Select();
}


/******************************************************************************
	Dir::Dispose
******************************************************************************/
void CSDirector::Dispose()
{
  inherited::Dispose();       
}


/******************************************************************************
	Dir::UpdateMenus
******************************************************************************/
void CSDirector::UpdateMenus()
{
  inherited::UpdateMenus();       
}


/******************************************************************************
	Dir::DoCommand
******************************************************************************/
void CSDirector::DoCommand(long theCommand)
{
	switch (theCommand) {
		
		default:	
			inherited::DoCommand(theCommand);
			break;
	}
}


//*****************************************************************************
														//  P A N E


/******************************************************************************
	Pane::ISPane
******************************************************************************/
void CSPane::ISPane( CView *anEnclosure, CBureaucrat *aSupervisor)
{
	CPicture			*aPic;
	CEditText			*anEditText;
	CPaneBorder			*aPB;
	CScrollPane			*aScrollPane;
	CSTable				*anSTable;
	StringHandle		rStr;
	Rect				margin;
	
	CPane::IPane( anEnclosure, aSupervisor, 0, 0, 0, 0,
				  sizFIXEDLEFT, sizFIXEDTOP);

	this->FitToEnclFrame( true, true);
	this->SetWantsClicks( true);

													// Make a Picture
	aPic = new( CPicture);
	aPic->IPicture( this, aSupervisor,
					this->frame.right, this->frame.bottom, 0, 0,
					sizFIXEDLEFT, sizFIXEDTOP);
	aPic->UsePICT( 1024);
	aPic->FrameToBounds();
	aPic->Place( 2, 2, false);
	
													// Make a Static Text
	anEditText = new( CEditText);
	anEditText->IEditText( this, aSupervisor,
					(this->frame.right/3)*2 - 40, this->frame.bottom/3 + 22,
					8, (this->frame.bottom/3)*2 - 24,
					sizFIXEDLEFT, sizFIXEDTOP,
					(this->frame.right/3)*2 );
	rStr = GetString( 1024);
	HLock( (Handle) rStr);
	anEditText->SetTextPtr( (Ptr)((char *)*rStr+1), *(unsigned char *)*rStr);
	anEditText->Specify( false, false, false);
	HUnlock( (Handle) rStr);
	
	aPB = new( CPaneBorder);
	aPB->IPaneBorder( kBorderFrame);
	SetRect( &margin, -2, 0, 2, 0);
	aPB->SetPenSize( 2, 2);
	aPB->SetMargin( &margin);
	anEditText->SetBorder( aPB);
	
	anEditText->SetFontSize( 9);
	
													// Make a CSTable
	aScrollPane = new( CScrollPane);
	aScrollPane->IScrollPane(  this, aSupervisor,
					this->frame.right/3 + 26, this->frame.bottom,
					(this->frame.right/3)*2 - 25, 0,
					sizFIXEDLEFT, sizFIXEDTOP,
					false, true, false);
	aScrollPane->SetWantsClicks( true);
		
	anSTable = new( CSTable);
	anSTable->ISTable( aScrollPane, aSupervisor,
					aScrollPane->width - 16, 0, 0, 0);
	anSTable->FitToEnclosure( TRUE, TRUE);

	aPB = new( CPaneBorder);						// re-use ptr, new object
	aPB->IPaneBorder( kBorderFrame);
	anSTable->SetBorder( aPB);
		
	aScrollPane->InstallPanorama( anSTable);
}


/******************************************************************************
	Pane::Dispose
******************************************************************************/
void CSPane::Dispose()
{
  inherited::Dispose();       
}


//*****************************************************************************
														//  T A B L E


/******************************************************************************
	Table::ISTable
******************************************************************************/
void CSTable::ISTable( CView *anEnclosure, CBureaucrat *aSupervisor,
				short aWidth, short aHeight,
				short aHEncl, short aVEncl)
{
	
	CTable::ITable( anEnclosure,  aSupervisor,
				aWidth, aHeight, aHEncl, aVEncl,
				sizFIXEDLEFT, sizFIXEDTOP);
	
	this->SetDefaults( aWidth, 60);
	this->AddCol( 1, 0);
		
	this->AddRow( numDemos, 0);
	
	this->SetDblClickCmd( cmdOpenShowcase);
	this->SetSelectionFlags( selOnlyOne);	
}

/******************************************************************************
	Table::Dispose
******************************************************************************/
void CSTable::Dispose()
{
	inherited::Dispose();       
}


/******************************************************************************
	Table::DoCommand
******************************************************************************/
void CSTable::DoCommand( long aCmd)
{
	Cell			aCell;
	
	switch (aCmd)
	{
		case  cmdOpenShowcase:
			SetCell( aCell, 0, 0);
			if ( GetSelect( true, &aCell) )
				DoCommand( cmdOpenShowcase + aCell.v + 1);
			break;
		
		default:
			inherited::DoCommand( aCmd);
			break;
	}
}


/******************************************************************************
	Table::DrawCell
******************************************************************************/
void CSTable::DrawCell( Cell theCell, Rect *cellRect)
{
	Rect			r;
	Handle			rH;
	unsigned char	*cp;
	
	r = *cellRect;
	InsetRect( &r, 2, 2);
	r.right -= 1;
	FrameRect( &r);
	
	TextSize( 9);
	TextFont( geneva);
	TextFace( bold);
	
	rH = demoClasses[theCell.v];
	LoadResource( rH);								// Make sure it's there!
	HLock( rH);
	cp = (unsigned char *)*rH;						// Dereference
	cp += *cp + 1;									// Jump over DemoDir name
	
	MoveTo( r.left+3, r.top+10-0);			// N A M E
	DrawString( cp);
	cp += *cp + 1;									// Jump over name
	
	TextFace( 0);
	
	MoveTo( r.left+3, r.top+21-0);			// D E S C R I P T I O N
	DrawString( cp);
	cp += *cp + 1;									// Jump over description
	
	MoveTo( r.left+3, r.top+40-1);			// A U T H O R
	DrawString( cp);
	cp += *cp + 1;									// Jump over author
	
	MoveTo( r.left+3, r.top+51);			// A D D R E S S
	DrawString( cp);
	HUnlock( rH);
}



//*****************************************************************************
												//  D E M O   D I R E C T O R


/******************************************************************************
	DemoDir::INewDemo
	
		To create your own DemoDirector, subclass CShowcaseDemoDir and
		override this method.  Your method should call this method first
		and then perform any initialization necessary (open a window,
		create your panes, whatever) and then start the demo.
******************************************************************************/
void CShowcaseDemoDir::INewDemo( CDirectorOwner *aSupervisor)
{
	inherited::IDirector( aSupervisor);
}



//*****************************************************************************
												// E N D   O F   L I S T I N G 


