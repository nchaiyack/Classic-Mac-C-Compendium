/******************************************************************************
	CEditDoc.c
	
	Document methods for a tiny editor.
		
	Copyright © 1989 Symantec Corporation. All rights reserved.

 ******************************************************************************/

#include "Global.h"
#include "Commands.h"
#include "CApplication.h"
#include "CBartender.h"
#include "CDataFile.h"
#include "CDecorator.h"
#include "CDesktop.h"
#include "CError.h"
#include "CPanorama.h"
#include "CScrollPane.h"
#include "TBUtilities.h"
#include "CEditDoc.h"
#include "CEditPane.h"
#include "CWindow.h"
#include <Packages.h>
#include "CResFile.h"

#define	WINDculture		500		/* Resource ID for WIND template */

extern	CApplication *gApplication;	/* The application */
extern	CBartender	*gBartender;	/* The menu handling object */
extern	CDecorator	*gDecorator;	/* Window dressing object	*/
extern	CDesktop	*gDesktop;		/* The enclosure for all windows */
extern	CBureaucrat	*gGopher;		/* The current boss in the chain of command */
extern	OSType		gSignature;		/* The application's signature */
extern	CError		*gError;		/* The error handling object */

#define kTypeSoup	'soup'

/***
 * IEditDoc
 *
 *	This is your document's initialization method.
 *	If your document has its own instance variables, initialize
 *	them here.
 *	The least you need to do is invoke the default method.
 *
 ***/
 
void CEditDoc::IEditDoc(CApplication *aSupervisor, Boolean printable)

{
	CDocument::IDocument(aSupervisor, printable);
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
void CEditDoc::NewFile(void)

{
		/**
		 **	BuildWindow() is the method that
		 **	does the work of creating a window.
		 **	It's parameter should be the data that
		 **	you want to display in the window.
		 **	Since this is a new window, there's nothing
		 **	to display.
		 **
		 **/

	BuildWindow(NULL, NULL, NULL);

		/**
		 **	Send the window a Select() message to make
		 **	it the active window.
		 **/
	
	itsWindow->Select();
}


/***
 * OpenFile
 *
 *	When the user chooses OpenÉ from the File menu, the OpenDocument()
 *	method in your Application class will let the user choose a file
 *	and then send a newly created document this message. The information
 *	about the file is in the SFReply record.
 *
 *	In this method, you need to open the file and display its contents
 *	in a window. This method uses the auxiliary window-building method.
 *
 ***/

void CEditDoc::OpenFile(SFReply *macSFReply)

{
	CDataFile	*theFile;
	Handle		theData;
	Str63		theName;
	OSErr		theError;
	// added by DWC for style/soup support
	CResFile	*resFile;
	StScrpHandle stylHandle = NULL;
	WESoupHandle soupHandle = NULL;
	short 		oldRes = CurResFile();

		/**
		 ** Create a file and send it a SFSpecify()
		 **	message to set up the name, volume, and
		 **	directory.
		 **
		 **/

	theFile = new(CDataFile);

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

	theFile->IDataFile();
	theFile->SFSpecify(macSFReply);
	

		/**
		 **	Send the file an Open() message to
		 **	open it. You can use the ReadSome() or
		 **	ReadAll() methods to get the contents of the file.
		 **
		 **/

	theFile->Open(fsRdWrPerm);
	
	/*** no longer needed with WASTE
	if (theFile->GetLength() > 10240L)
	{
		ParamText( "\pCan't open a file this big.", "\p", "\p", "\p");
		PositionDialog('ALRT', 128);
		InitCursor();
		Alert(128, NULL);
		
		delete this;
		return;
	}
	***/

    theData = theFile->ReadAll();     /* ReadAll() creates the handle */

	// ¥¥ÊCode added by DWC for style/soup support
	// check for styl, soup resources resource
	resFile = new CResFile();
	resFile->SFSpecify(macSFReply);
	if (resFile->HasResFork())
	{
		resFile->Open(fsRdPerm);
		
		resFile->MakeCurrent(); // Use the resource file.

		// ¥¥Êstyle ¥¥
		stylHandle = (StScrpHandle)Get1IndResource('styl', 1); // get the resource
		if (stylHandle)
		{
			DetachResource((Handle)stylHandle);
#ifndef WASTE11
			// fix height=0 lines
			nStyles = (*stylHandle)->scrpNStyles;	
			for(i=0; i<nStyles; i++)
				if ((*stylHandle)->scrpStyleTab[i].scrpSize==0)
					(*stylHandle)->scrpStyleTab[i].scrpSize = 12;
#endif
		}
		// ¥¥ soup ¥¥
		soupHandle = (WESoupHandle)Get1IndResource(kTypeSoup, 1); // get the resource
		if (soupHandle)
		{
			DetachResource((Handle)soupHandle);
		}
		UseResFile(oldRes); // use old resource file
		resFile->Close();
	}
	TCLForgetObject(resFile);

	BuildWindow(theData, stylHandle, soupHandle);

		/**
		 **	In your application, you'll probably store
		 **	the data in some form as an instance variable
		 **	in your document class. For this example, there's
		 **	no need to save it, so we'll get rid of it.
		 **
		 **/

	DisposHandle(theData);
	// added by DWC
	DisposeHandle((Handle)stylHandle);
	DisposeHandle((Handle)soupHandle);

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



/***
 * BuildWindow
 *
 *	This is the auxiliary window-building method that the
 *	NewFile() and OpenFile() methods use to create a window.
 *
 *	In this implementation, the argument is the data to display.
 *
 ***/

void CEditDoc::BuildWindow (Handle theData, StScrpHandle stylHandle, WESoupHandle soupHandle)

{
	CScrollPane		*theScrollPane;
	CEditPane		*theMainPane;
	Rect			margin;

		/**
		 **	First create the window and initialize
		 **	it. The first argument is the resource ID
		 **	of the window. The second argument specifies
		 **	whether the window is a floating window.
		 **	The third argument is the window's enclosure; it
		 **	should always be gDesktop. The last argument is
		 **	the window's supervisor in the Chain of Command;
		 **	it should always be the Document object.
		 **
		 **/

	itsWindow = new(CWindow);
	itsWindow->IWindow(WINDculture, FALSE, gDesktop, this);
	
		/**
		 **	After you create the window, you can use the
		 **	SetSizeRect() message to set the windowÕs maximum
		 **	and minimum size. Be sure to set the max & min
		 **	BEFORE you send a PlaceNewWindow() message to the
		 **	decorator.
		 **
		 ** The default minimum is 100 by 100 pixels. The
		 **	default maximum is the bounds of GrayRgn() (The
		 **	entire display area on all screens.)
		 **
		 **/

	theScrollPane = new(CScrollPane);
	
		/**
		 **	You can initialize a scroll pane two ways:
		 **		1. You can specify all the values
		 **		   right in your code, like this.
		 **		2. You can create a ScPn resource and
		 **		   initialize the pane from the information
		 **		   in the resource.
		 **
		 **/

	theScrollPane->IScrollPane(itsWindow, this, 10, 10, 0, 0,
								sizELASTIC, sizELASTIC,
								TRUE, TRUE, TRUE);

		/**
		 **	The FitToEnclFrame() method makes the
		 **	scroll pane be as large as its enclosure.
		 **	In this case, the enclosure is the window,
		 **	so the scroll pane will take up the entire
		 **	window.
		 **
		 **/

	theScrollPane->FitToEnclFrame(TRUE, TRUE);


		/**
		 **	itsMainPane is the document's focus
		 **	of attention. Some of the standard
		 **	classes (particularly CPrinter) rely
		 **	on itsMainPane pointing to the main
		 **	pane of your window.
		 **
		 **	itsGopher specifies which object
		 **	should become the gopher. By default
		 **	the document becomes the gopher. ItÕs
		 **	likely that your main pane handles commands
		 **	so youÕll almost want to set itsGopher
		 **	to point to the same object as itsMainPane.
		 **
		 **	Note that the main pane is the
		 **	panorama in the scroll pane and not
		 **	the scroll pane itself.
		 **
		 **/

	theMainPane = new (CEditPane);

	itsMainPane = theMainPane;
	itsGopher = theMainPane;

		/**
		 **	The IEditPane method automatically
		 **	fits the pane to the enclosure and
		 **	gives us a little margin.
		 **
		 **/

	theMainPane->IEditPane(theScrollPane, this);

		/**
		 **	Send the scroll pane an InstallPanorama()
		 **	to associate our pane with the scroll pane.
		 **
		 **/

	theScrollPane->InstallPanorama(theMainPane);

	if (theData)
	{
		// Modified by DWC
		HLock(theData);
		theMainPane->InsertWithStyleSoup(*theData, GetHandleSize(theData),
			stylHandle, soupHandle, true);
		HUnlock(theData);
	}
	
		/**
		 **	The Decorator is a global object that takes care
		 **	of placing and sizing windows on the screen.
		 **	You don't have to use it.
		 **
		 **/

	gDecorator->PlaceNewWindow(itsWindow);
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

Boolean CEditDoc::DoSave(void)

{
	Handle		theData;
	// added by DWC
	StScrpHandle stylHandle;
	WESoupHandle soupHandle;
	CResFile	*resFile;
	short 		oldRes = CurResFile();
	Handle		h;
	FSSpec		theSpec;

	if (itsFile == NULL)
		return(DoSaveFileAs());
	else {
		// modified by DWC
		// create handles for text, style and soup
		theData = NewHandle(0);
		stylHandle = (StScrpHandle)NewHandle(0);
		soupHandle = (WESoupHandle)NewHandle(0);
		
		// get text, style and soup from edit pane
		((CEditPane *)itsMainPane)->CopyRangeWithStyleSoup(0, 0x7fffffff, theData,
			stylHandle, soupHandle);
		((CDataFile *)itsFile)->WriteAll(theData);			

		// create resource fork if it doesn't exist
		resFile = new CResFile();
		itsFile->GetFSSpec(&theSpec);
		resFile->SpecifyFSSpec(&theSpec);
		if (!resFile->HasResFork())
		{
			resFile->CreateNew('xxxx', 'xxxx');
		}
		resFile->Open(fsWrPerm);
		resFile->MakeCurrent(); // Use the resource file.

		// write styl resource
		// see if resource already exists
		h = Get1IndResource('styl', 1);
		if (h==NULL) // doesn't exist
		{
			AddResource((Handle)stylHandle, 'styl', 128, "\p");
			FailResError();
		}
		else
		{
			// change what's there
			SetHandleSize(h, 0);
			HLock((Handle)stylHandle);
			HandAndHand((Handle)stylHandle, h);
			ChangedResource(h);
			DisposeHandle((Handle)stylHandle);
		}
		if (h) ReleaseResource(h);

		// write soup resource
		// see if resource already exists
		h = Get1IndResource(kTypeSoup, 1);
		if (h==NULL) // doesn't exist
		{
			AddResource((Handle)soupHandle, kTypeSoup, 128, "\p");
			FailResError();
		}
		else
		{
			// change what's there
			SetHandleSize(h, 0);
			HLock((Handle)soupHandle);
			HandAndHand((Handle)soupHandle, h);
			ChangedResource(h);
			DisposeHandle((Handle)soupHandle);
		}
		if (h) ReleaseResource(h);

		resFile->Update();
		UseResFile(oldRes);
		resFile->Close();

		dirty = FALSE;					/* Document is no longer dirty		*/
		gBartender->DisableCmd(cmdSave);
		return(TRUE);					/* Save was successful				*/
	}
}


/***
 * DoSaveAs
 *
 *	This method handles what happens when the user chooses Save AsÉ from
 *	File menu. The default DoCommand() method for documents sends a DoSaveFileAs()
 *	message which displays a standard put file dialog and sends this message.
 *	The SFReply record contains all the information about the file you're about
 *	to create.
 *
 ***/

Boolean CEditDoc::DoSaveAs(SFReply *macSFReply)

{
		/**
		 **	If there's a file associated with this document
		 **	already, close it. The Dispose() method for files
		 **	sends a Close() message to the file before releasing
		 **	its memory.
		 **
		 **/
		 
	TCLForgetObject(itsFile);


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
