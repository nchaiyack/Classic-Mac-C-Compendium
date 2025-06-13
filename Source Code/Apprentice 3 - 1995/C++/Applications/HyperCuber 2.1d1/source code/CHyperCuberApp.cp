//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CHyperCuberApp.h
//|
//| This file contains the implementation of the HyperCuber Application class.
//|___________________________________________________________________________

#include "CAboutDialogDirector.h"
#include "CEquationDirector.h"
#include "CKeyControlsDirector.h"
#include "CMouseControlsDirector.h"
#include "CNCubeDialogDirector.h"
#include "CHyperCuberApp.h"
#include "CHyperCuberDoc.h"
#include "CHyperCuberPane.h"
#include "CHyperCuberPrefs.h"
#include "CPrefsDialogDirector.h"
#include "HyperCuber Commands.h"
#include "HyperCuber Errors.h"

#include <CFWDesktop.h>


//============================ Constants ============================\\

#define		kExtraMasters		12
#define		kRainyDayFund		20480
#define		kCriticalBalance	20480
#define		kToolboxBalance		20480


//============================ Globals ============================\\

extern	OSType		gSignature;
extern	CBartender	*gBartender;
extern	CDesktop	*gDesktop;

CHyperCuberPrefs	*gPrefs;

extern CGrafPtr		left_offscreen_cptr;
extern CGrafPtr		right_offscreen_cptr;
extern CGrafPtr		offscreen_cptr;

extern Boolean		menubar_hidden;


//============================ Prototypes ============================\\

extern void TerminalError(short error);
extern void CreateOffscreenPixmap(CGrafPort *color_port, Rect *bounds);
extern void DisposeOffscreenPixmap(CGrafPort *color_port);
extern "C" void init_menubar(void);
extern "C" void show_menubar(void);


//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CHyperCuberApp::IHyperCuberApp
//|
//| Purpose: Initialize the application
//|
//| Parameters: none
//|____________________________________________________________

void CHyperCuberApp::IHyperCuberApp(void)
{

	Rect offscreen_bitmap_bounds = {0, 0, 1, 1};	//  Create the empty offscreen bitmaps
	CreateOffscreenPixmap(left_offscreen_cptr, 		
							&offscreen_bitmap_bounds);
	CreateOffscreenPixmap(right_offscreen_cptr, 		
							&offscreen_bitmap_bounds);

	CApplication::IApplication( kExtraMasters, kRainyDayFund, 
						kCriticalBalance, kToolboxBalance);
    
    gPrefs = new(CHyperCuberPrefs);					//  Create the preferences
    gPrefs->IHyperCuberPrefs();						//  Initialize prefs to default
	gPrefs->Load();									//  Load the preferences in from the file

	init_menubar();									//  Prepare to possibly hide the menubar

}	//==== CHyperCuberApp::IHyperCuberApp() ====\\



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CHyperCuberApp::MakeDesktop
//|
//| Purpose: Create the desktop (which supports floating windows)
//|
//| Parameters: none
//|______________________________________________________________

void CHyperCuberApp::MakeDesktop(void)
{

	gDesktop = new(CFWDesktop);						//  Create and initialize the desktop
	((CFWDesktop *) gDesktop)->IFWDesktop(this);

}	//==== CHyperCuberApp::MakeDesktop() ====\\



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CHyperCuberApp::InspectSystem
//|
//| Purpose: Gather information about the current OS
//|
//| Parameters: none
//|__________________________________________________

void CHyperCuberApp::InspectSystem( void)
{

	OSErr		err;
	SysEnvRec	environs;
	long		response;
	
	CApplication::InspectSystem();						//  Do superclass's inspection
	
	if (!gSystem.hasFPU)								//  We must have a Floating Point Unit
		TerminalError(missing_fpu);
	
	if (!gSystem.hasGestalt)							//  We must have the gestalt manager
		TerminalError(missing_gestalt);

	if (!gSystem.hasColorQD)							//  We must have Color QuickDraw
		TerminalError(missing_color_qd);

	if (Gestalt(gestaltFindFolderAttr, &response) != noErr)	//  We must have FindFolder
		TerminalError(missing_folder_manager);

	Gestalt(gestaltProcessorType, &response);			//  We must have a 68020
	if (response < gestalt68020)
		TerminalError(missing_68020);	
	
}	//==== CHyperCuberApp::InspectSystem() ====\\



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CHyperCuberApp::SetUpFileParameters
//|
//| Purpose: Specify the kinds of files we open
//|
//| Parameters: none
//|_________________________________________________________

void CHyperCuberApp::SetUpFileParameters(void)

{
	inherited::SetUpFileParameters();		//  Call superclass

	sfNumTypes = 1;							//  We know only of 'TEXT' files
	sfFileTypes[0] = 'TEXT';

	gSignature = 'HCub';					//  Initialize our signature

}	//==== CHyperCuberApp::SetUpFileParameters() ====\\



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CHyperCuberApp::DoCommand
//|
//| Purpose: Handle application commands
//|
//| Parameters: theCommand: the command to handle
//|_________________________________________________________

void CHyperCuberApp::DoCommand(long theCommand)
{

	switch (theCommand)
		{
	
		case cmdAbout:									//  Put up the "About HyperCuber..." dialog
		
			CAboutDialogDirector about_dialog_director;
			about_dialog_director.IAboutDialogDirector(this);
			about_dialog_director.TalkToUser();		//  Get input
			
			break;

		case cmdColors:									//  Put up the Colors... dialog
		
			CPrefsDialogDirector dialog_director;
			dialog_director.IPrefsDialogDirector(this);
			dialog_director.TalkToUser();
			((CHyperCuberPane *) itsDocument->itsMainPane)->Refresh();
			
			gPrefs->Update();							//  Save the preferences to the prefs file
			gPrefs->Save();

			break;

		case cmdKeyControls:							//  Put up the Key Controls... dialog
		
			CKeyControlsDirector kcdialog_director;
			kcdialog_director.IKeyControlsDirector(this);
			kcdialog_director.TalkToUser();
			
			gPrefs->Update();							//  Save the preferences to the prefs file
			gPrefs->Save();

			break;

		case cmdMouseControls:							//  Put up the Mouse Controls... dialog
		
			CMouseControlsDirector mcdialog_director;
			mcdialog_director.IMouseControlsDirector(this);
			mcdialog_director.TalkToUser();
			
			gPrefs->Update();							//  Save the preferences to the prefs file
			gPrefs->Save();
			
			break;

		case cmdNewNCube:									//  Create a new n-dimensional cube
			
			CNCubeDialogDirector ncdialog_director;
			ncdialog_director.INCubeDialogDirector(this);		//  Create and display dialog
			long dimension = ncdialog_director.TalkToUser();	//  Get dimension of n-cube to create

			if (dimension)
				{
				itsDocument = new(CHyperCuberDoc);			//  Create the n-cube document
				itsDocument->IHyperCuberDoc(this, TRUE);
				itsDocument->NewNCube(dimension);
				}
			
			break;
			
		case cmdNewFromEquation:							//  Create a new n-dimensional object from
															//   a mathematical equation
			
			CEquationDirector equation_director;
			equation_director.IEquationDirector(this);
			Boolean graph;
			equation_director.TalkToUser(graph);			//  Let user change functions or parameters

			if (graph)
				{
				itsDocument = new(CHyperCuberDoc);			//  Create the new document
				itsDocument->IHyperCuberDoc(this, TRUE);
				Boolean error = itsDocument->
										NewFromEquation();	//  Graph the equation
				if (error)
					delete(itsDocument);					//  If error, get rid of new document
				}
				
			break;

		default:	inherited::DoCommand(theCommand);	//  Handle normal commands
					break;
		}

}	//==== CHyperCuberApp::DoCommand() ====\\



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CHyperCuberApp::UpdateMenus
//|
//| Purpose: Enable the application's menus.
//|
//| Parameters: none
//|___________________________________________________________________________

void CHyperCuberApp::UpdateMenus()
{

	inherited::UpdateMenus();    				 // Enable standard commands */

	gBartender->EnableCmd(cmdOpen);				//  Enable the Open... menu item  
	gBartender->EnableCmd(cmdColors);			//  Enable Colors... menu item  
	gBartender->EnableCmd(cmdKeyControls);		//  Enable Key Controls... menu item  
	gBartender->EnableCmd(cmdMouseControls);	//  Enable Mouse Controls... menu item  
	gBartender->EnableCmd(cmdNewNCube);			//  Enable New NCube menu item  
	gBartender->EnableCmd(cmdNewFromEquation);	//  Enable New From Equation... menu item  

}	//==== CHyperCuberApp::UpdateMenus() ====\\



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CHyperCuberApp::Exit
//|
//| Purpose: clean up before quitting.  This saves the preferences.
//|
//| Parameters: none
//|___________________________________________________________________________
 
void CHyperCuberApp::Exit()
{

	show_menubar();								//  Show the menu bar, if it's hidden

	gPrefs->Save();								//  Save the preferences to the preferences file

	DisposeOffscreenPixmap(left_offscreen_cptr);	//  Get rid of the offscreen pixmaps
	DisposeOffscreenPixmap(right_offscreen_cptr);
	
}	//==== CHyperCuberApp::Exit() ====\\



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CHyperCuberApp::CreateDocument
//|
//| Purpose: Create a new CHyperCuberDoc.
//|
//| Parameters: none
//|___________________________________________________________________________

void CHyperCuberApp::CreateDocument()

{
	
	TRY
		{
		itsDocument = new(CHyperCuberDoc);			//  Create the document
		itsDocument->IHyperCuberDoc(this, TRUE);
		itsDocument->NewFile();

		AddDirector(itsDocument);					//  Add this document to the director list
		}
	
	CATCH
		{
		if (itsDocument) delete(itsDocument);	//  If we failed, throw away the document
		}
	ENDTRY;

}	//==== CHyperCuberApp::CreateDocument() ====\\



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CHyperCuberApp::OpenDocument
//|
//| Purpose: Open a graphics file
//|
//| Parameters: macSFReply: the file to open
//|___________________________________________________________________________
 
void CHyperCuberApp::OpenDocument(SFReply *macSFReply)
{

	CHyperCuberDoc	*theDocument = NULL;
	
	TRY
		{
		theDocument = new(CHyperCuberDoc);			//  Create a new document
		theDocument->IHyperCuberDoc(this, TRUE);
		theDocument->OpenFile(macSFReply);			//  Open the file
		
		AddDirector(theDocument);					//  Add this document to the director list
		}
	
	CATCH
		{
		 if (theDocument) delete(theDocument);		//  If this failed, throw away the document
		}
		
	ENDTRY;
	
}	//==== CHyperCuberApp::OpenDocument() ====\\