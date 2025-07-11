////////////
//
//	CChessDoc.cp
//
//	Document methods for a Chess application.
//
//  Copyright � 1993 Steven J. Bushell. All rights reserved.
//
////////////

#include <Global.h>
#include <Commands.h>
#include <CBartender.h>
#include <CDataFile.h>
#include <CDecorator.h>
#include <CDesktop.h>
#include <CError.h>
#include <CPanorama.h>
#include <CScrollPane.h>
#include "CChessDoc.h"
#include <TBUtilities.h>
#include <CWindow.h>
#include <Packages.h>

#include "CBrain.h"
#include "ChessCommands.h"
#include "CChessOptionsDialog.h"

#define	WINDChess		500		/* Resource ID for WIND template */

extern	CApplication *gApplication;	/* The application */
extern	CBartender	*gBartender;	/* The menu handling object */
extern	CDecorator	*gDecorator;	/* Window dressing object	*/
extern	CDesktop	*gDesktop;		/* The enclosure for all windows */
extern	CBureaucrat	*gGopher;		/* The current boss in the chain of command */
extern	OSType		gSignature;		/* The application's signature */
extern	CError		*gError;		/* The global error handler */
extern	CBrain		*gBrain;

void				DoAboutChessBox(void);

/***
 * IChessDoc
 *
 *	This is your document's initialization method.
 *	If your document has its own instance variables, initialize
 *	them here.
 *
 *	The least you need to do is invoke the default method.
 *
 ***/
 

void CChessDoc::IChessDoc(CApplication *aSupervisor, Boolean printable)

{
	CDocument::IDocument(aSupervisor, printable);

	// Create the global brain, your virtual opponent
	gBrain = new CBrain;
	gBrain->IBrain(this);

	// We initially don't have a file associated with the board
	itsFile = NULL;
}


/***
 * Dispose
 *
 *	This is your document's destruction method.
 *	If you allocated memory in your initialization method
 *	or opened temporary files, this is the place to release them.
 *
 *	Be sure to call the default method!
 *
 ***/

void CChessDoc::Dispose()

{
	inherited::Dispose();
}


/***
 * DoCommand
 *
 *	This is the heart of your document.
 *	In this method, you handle all the commands your document
 *	deals with.
 *
 *	Be sure to call the default method to handle the standard
 *	document commands: cmdClose, cmdSave, cmdSaveAs, cmdRevert,
 *	cmdPageSetup, cmdPrint, and cmdUndo. To change the way these
 *	commands are handled, override the appropriate methods instead
 *	of handling them here.
 *
 ***/

void CChessDoc::DoCommand(long theCommand)

{
	switch (theCommand) {

		case cmdSwapPlayers:
			gBrain->abortMove = true;
			SwapPlayers();
			break;
		case cmdStopThinking:
			itsMainPane->BecomeGopher(true);
			gBrain->isThinking = false;
			break;
		case cmdOptions:
			DoChessOptionsDialog();
			break;
		default:
			inherited::DoCommand(theCommand);
			break;
	}
}


/***
 * UpdateMenus
 *
 *  In this method you can enable menu commands that apply when
 *  your document is active.
 *
 *  Be sure to call the inherited method to get the default behavior.
 *  The inherited method enables these commands: cmdClose, cmdSaveAs,
 *  cmdSave, cmdRevert, cmdPageSetup, cmdPrint, cmdUndo.
 *
***/

 void CChessDoc::UpdateMenus()

 {
	inherited::UpdateMenus();
	
	// If a chess board is already open, we don't want to 
	// be able to open or create a new one
	gBartender->DisableCmd(cmdNew);
	gBartender->DisableCmd(cmdOpen);
	
	gBartender->EnableCmd(cmdOptions);
}


/***
 * NewFile
 *
 *	When the user chooses New from the File menu, the CreateDocument()
 *	method in your Application class will send a newly created document
 *	this message. This method needs to create a new window, ready to
 *	work on a new document.
 *
 *	Since this method and the OpenFile() method share the code for creating
 *	the window, you should use an auxiliary window-building method.
 *
 ***/
void CChessDoc::NewFile(void)

{    
    Str255  wTitle;     /* Window title string.         */
    short   wCount;     /* Index number of new window.  */
    Str255  wNumber;    /* Index number as a string.    */
    
		/**
		 **	BuildWindow() is the method that
		 **	does the work of creating a window.
         ** Its parameter should be the data that
		 **	you want to display in the window.
		 **	Since this is a new window, there's nothing
		 **	to display.
		 **
		 **/

	BuildWindow(NULL);
    
        /**
         ** Append an index number to the
         ** default name of the window.
         **/

    itsWindow->SetTitle("\pChess++");

		/**
		 **	Send the window a Select() message to make
		 **	it the active window.
		 **/
	
	itsWindow->Select();
}


/***
 * OpenFile
 *
 *	When the user chooses Open� from the File menu, the OpenDocument()
 *	method in your Application class will let the user choose a file
 *	and then send a newly created document this message. The information
 *	about the file is in the SFReply record.
 *
 *	In this method, you need to open the file and display its contents
 *	in a window. This method uses the auxiliary window-building method.
 *
 ***/

void CChessDoc::OpenFile(SFReply *macSFReply)

{
	CDataFile	*theFile;
	Handle		theData = NULL;
	Str63		theName;
	OSErr		theError;
	
	TRY
	{
	
			/**
			 ** Create a file and send it a SFSpecify()
			 **	message to set up the name, volume, and
			 **	directory.
			 **
			 **/
	
		theFile = new(CDataFile);
		theFile->IDataFile();
		theFile->SFSpecify(macSFReply);
		
			/**
			 **	Be sure to set the instance variable
			 **	so other methods can use the file if they
			 **	need to. This is especially important if
			 **	you leave the file open in this method.
			 **	If you close the file after reading it, you
			 **	should be sure to set itsFile to NULL.
			 **
			 **/
	
		itsFile = theFile;
	
			/**
			 **	Send the file an Open() message to
			 **	open it. You can use the ReadSome() or
			 **	ReadAll() methods to get the contents of the file.
			 **
			 **/
	
		theFile->Open(fsRdWrPerm);
		
			/**
			 **	Make sure that the memory request to read
			 **	the data from the file doesn't use up any
			 **	of our rainy day fund and that the GrowMemory()
			 **	method (in the application) knows that it's OK
			 **	if we couldn't get enough memory.
			 **
			 **/
	
	
	    theData = theFile->ReadAll();     /* ReadAll() creates the handle */
	    
	 
		
		
		BuildWindow(theData);
	
			/**
			 **	In your application, you'll probably store
			 **	the data in some form as an instance variable
			 **	in your document class. For this example, there's
			 **	no need to save it, so we'll get rid of it.
			 **
			 **/
	
		DisposHandle(theData);
		theData = NULL;
	
			/**
			 **	In this implementation, we leave the file
			 **	open. You might want to close it after
			 **	you've read in all the data.
			 **
			 **/
	
		itsFile->GetName(theName);
		itsWindow->SetTitle(theName);
		itsWindow->Select();			/* Don't forget to make the window active */
	}
	
	CATCH
	{
		/*
		 * This exception handler will be executed if an exception occurs
		 * anywhere within the scope of the TRY block above.
		 * You should perform any cleanup of things that won't be needed
		 * since the document could not be opened. By convention,
		 * the creator of an object is responsible for sending it
		 * the Dispose message. This means that we should only dispose
		 * of things that would not be taken care of in Dispose.
		 * In this case, we just make sure that the Handle theData
		 * has been disposed of. The exception will propagate up to
		 * CApplications's exception handler, which handles displaying
		 * an error alert.
		 */
		 
		 if (theData) DisposHandle( theData);
		 
	}
	ENDTRY;
}



/***
 * BuildWindow
 *
 *	This is the auxiliary window-building method that the
 *	NewFile() and OpenFile() methods use to create a window.
 *
 *	In this implementation, the argument is the data to display.
 *
 ***/

void CChessDoc::BuildWindow (Handle theData)

{
	CPane		*thePane;
	CChessPane	*theMainPane;
	Rect	tempRect;
	
	itsWindow = new(CWindow);
	itsWindow->IWindow(WINDChess, FALSE, gDesktop, this);
	itsWindow->ChangeSize(256,256);

	SetRect(&tempRect,256,256,256,256);
	itsWindow->SetSizeRect(&tempRect);

	theMainPane = new(CChessPane);
    theMainPane->IChessPane(itsWindow, this, 0, 0, 0, 0,
    						sizELASTIC, sizELASTIC);
	itsMainPane = theMainPane;
	itsGopher = theMainPane;

	theMainPane->FitToEnclosure(TRUE, TRUE);
}


/***
 * DoSave
 *
 *	This method handles what happens when the user chooses Save from the
 *	File menu. This method should return TRUE if the file save was successful.
 *	If there is no file associated with the document, you should send a
 *	DoSaveFileAs() message.
 *
 ***/

Boolean CChessDoc::DoSave(void)

{
		/**
		 **	If you closed your file in your NewFile() method,
		 **	you'll need a different way than this to determine
		 **	if there's a file associated with your document.
		 **
		 **/

	if (itsFile == NULL)
		return(DoSaveFileAs());
	else {
			
		/**
		 **	In your application, this is where you'd
		 **	write out your file. if you left it open,
		 **	send the WriteSome() or WriteAll() mesages
		 **	to itsFile.
		 **
		 **/
			
		dirty = FALSE;					/* Document is no longer dirty		*/
		gBartender->DisableCmd(cmdSave);
		return(TRUE);					/* Save was successful				*/
	}
}


/***
 * DoSaveAs
 *
 *	This method handles what happens when the user chooses Save As� from
 *	File menu. The default DoCommand() method for documents sends a DoSaveFileAs()
 *	message which displays a standard put file dialog and sends this message.
 *	The SFReply record contains all the information about the file you're about
 *	to create.
 *
 ***/

Boolean CChessDoc::DoSaveAs(SFReply *macSFReply)

{
		/**
		 **	If there's a file associated with this document
		 **	already, close it. The Dispose() method for files
		 **	sends a Close() message to the file before releasing
		 **	its memory.
		 **
		 **/
		 
	if (itsFile != NULL)
		itsFile->Dispose();


		/**
		 **	Create a new file, and then save it normally.
		 **
		 **/

	itsFile = new(CDataFile);
	((CDataFile *)itsFile)->IDataFile();
	itsFile->SFSpecify(macSFReply);
	itsFile->CreateNew(gSignature, 'TEXT');
	itsFile->Open(fsRdWrPerm);
	
	itsWindow->SetTitle(macSFReply->fName);

	return( DoSave() );
}


/***
 * DoRevert
 *
 *	If your application supports the Revert command, this method
 *	should close the current file (without writing anything out)
 *	and read the last saved version of the file.
 *
 ***/

void CChessDoc::DoRevert(void)

{
}

void CChessDoc::SwapPlayers(void)
{
	CChessBoard *itsChessBoard = ((CChessPane *)itsMainPane)->itsChessBoard;

	if (gBrain->isBrainsMove)
	{
		gBrain->isBrainsMove = false;
		gBrain->isThinking = false;
	}
	
	itsChessBoard->myColor = itsChessBoard->theOtherPlayersColor;
	if (itsChessBoard->myColor == White)
		itsChessBoard->theOtherPlayersColor = Black;
	else
		itsChessBoard->theOtherPlayersColor = White;
}


void CChessDoc::DoChessOptionsDialog(void)
{
	CChessOptionsDialog *theOptionsDialog;
	
	theOptionsDialog = new CChessOptionsDialog;
	ParamText("\pSteven Bushell","\pSteven Bushell",NULL,NULL);
	theOptionsDialog->IChessOptionsDialog(this);
	theOptionsDialog->DoChessOptionsDialog();

	theOptionsDialog->Dispose();
}


void CChessDoc::ProviderChanged( CCollaborator *aProvider, long reason, void* info)
{
	if (member( aProvider, CBureaucrat) && (reason == bureaucratIsGopher))
	{	
		if (member( aProvider, CBrain))
		{	// If the Brain is thinking, we don't want the user
			// interrupting with anything but StopThinking
			gBartender->DisableMenu(MENUfile);
			gBartender->DisableMenu(MENUedit); 
		}
		else if (member( aProvider, CChessPane))
		{
			gBartender->EnableMenu(MENUfile);
			gBartender->EnableMenu(MENUedit);
		}
	}
	inherited::ProviderChanged( aProvider, reason, info);

}	/* CDirector::ProviderChanged */