/******************************************************************************
	CEditApp.c
	
	Application methods for a tiny editor.
		
	Copyright © 1989 Symantec Corporation. All rights reserved.

 ******************************************************************************/

#include "Commands.h"
#include "CBartender.h"
#include "CEditApp.h"
#include "CEditDoc.h"
#include "Global.h"
#include "WASTE.h"
#ifndef __SCRIPT__
#include <Script.h>
#endif
#include <TextServices.h>
#include <GestaltEqu.h>
#include <FragLoad.h>
#include <Drag.h>

#include "CTSMSwitchboard.h"

static Boolean	TSMAvailable(void);
static Boolean	DragAndDropAvailable(void);

short gUsingTSM = false;
Boolean gHasDragAndDrop = false;

extern	OSType		gSignature;
extern	CBartender *gBartender;

#define		MENUcolor			135

#define		kExtraMasters		10
#define		kRainyDayFund		45000
#define		kCriticalBalance	40000
#define		kToolboxBalance		20000


/***
 * IEditApp
 *
 *	Initialize the application. Your initialization method should
 *	at least call the inherited method. If your application class
 *	defines its own instance variables or global variables, this
 *	is a good place to initialize them.
 *
 ***/

void CEditApp::IEditApp(void)

{
	gUsingTSM = TSMAvailable();
	if (gUsingTSM) gUsingTSM = (InitTSMAwareApplication()==noErr); // can we init?
	gHasDragAndDrop = DragAndDropAvailable();

	CApplication::IApplication( kExtraMasters, kRainyDayFund,
						kCriticalBalance, kToolboxBalance);

}



/***
 * SetUpFileParameters
 *
 *	In this routine, you specify the kinds of files your
 *	application opens.
 *
 *
 ***/

void CEditApp::SetUpFileParameters(void)

{
	inherited::SetUpFileParameters();	/* Be sure to call the default method */

		/**
		 **	sfNumTypes is the number of file types
		 **	your application knows about.
		 **	sfFileTypes[] is an array of file types.
		 **	You can define up to 4 file types in
		 **	sfFileTypes[].
		 **
		 **/

	sfNumTypes = 1;
	sfFileTypes[0] = 'TEXT';

		/**
		 **	Although it's not an instance variable,
		 **	this method is a good place to set the
		 **	gSignature global variable. Set this global
		 **	to your application's signature. You'll use it
		 **	to create a file (see CFile::CreateNew()).
		 **
		 **/

	gSignature = '???\?';
}


/***
 * SetUpMenus
 *
 *	In this method, you add special menu items and set the
 *	menu item dimming and checking options for your menus.
 *	The most common special menu items are the names of the
 *	fonts. For this tiny editor, you also want to set up the
 *	dimming and checking options so only the current font
 *	and size are checked.
 *
 ***/

void CEditApp::SetUpMenus(void)

{
		/**
		 ** Let the default method read the menus from
		 **	the MBAR 1 resource.
		 **
		 **/

	inherited::SetUpMenus();

		/**
		 ** Add the fonts in the  system to the
		 **	Font menu. Remember, MENUfont is one
		 **	of the reserved font numbers.
		 **
		 **/

	AddResMenu(GetMHandle(MENUfont), 'FONT');

		/**
		 **	The UpdateMenus() method sets up the dimming
		 **	for menu items. By default, the bartender dims
		 **	all the menus, and each bureaucrat is reponsible
		 **	for turning on the items that correspond to the commands
		 **	it can handle.
		 **
		 **	Set up the options here. The edit pane's UpdateMenus()
		 **	method takes care of doing the work.
		 **
		 **	For Font and Size menus, you want all the items to
		 **	be enabled all the time. In other words, you don't
		 **	want the bartender to ever dim any of the items
		 **	in these two menus.
		 **
		 **/

	gBartender->SetDimOption(MENUfont, dimNONE);
	gBartender->SetDimOption(MENUsize, dimNONE);
	gBartender->SetDimOption(MENUstyle, dimNONE);
	gBartender->SetDimOption(MENUcolor, dimNONE);

		/**
		 **	For Font and Size menus, one of the items
		 **	is always checked. Setting the unchecking option
		 **	to TRUE lets the bartender know that it should
		 **	uncheck all the menu items because an UpdateMenus()
		 **	method will check the right items.
		 **	For the Style menu, uncheck all the items and
		 **	let the edit pane's UpdateMenus() method check the
		 **	appropriate ones.
		 **
		 **/

	gBartender->SetUnchecking(MENUfont, TRUE);
	gBartender->SetUnchecking(MENUsize, TRUE);
	gBartender->SetUnchecking(MENUstyle, TRUE);
	gBartender->SetUnchecking(MENUcolor, TRUE);
}



/***
 * CreateDocument
 *
 *	The user chose New from the File menu.
 *	In this method, you need to create a document and send it
 *	a NewFile() message.
 *
 ***/

void CEditApp::CreateDocument()

{
	CEditDoc	*theDocument = NULL;
	
	// In the event that creating the document fails,
	// we setup an exception handler here. If any
	// of the methods called within the scope of this
	// TRY block fail, an exception will be raised and
	// control will be transferred to the CATCH block.
	// Here, the CATCH block takes care of disposing
	// of the partially created document.
	
	TRY
	{
		theDocument = new(CEditDoc);
			
			/**
			 **	Send your document an initialization
			 **	message. The first argument is the
			 **	supervisor (the application). The second
			 **	argument is TRUE if the document is printable.
			 **
			 **/
		
		theDocument->IEditDoc(this, TRUE);
	
			/**
			 **	Send the document a NewFile() message.
			 **	The document will open a window, and
			 **	set up the heart of the application.
			 **
			 **/
		theDocument->NewFile();
	}
	CATCH
	{
		ForgetObject( theDocument);
	}
	ENDTRY;
}

/***
 * OpenDocument
 *
 *	The user chose OpenÉ from the File menu.
 *	In this method you need to create a document
 *	and send it an OpenFile() message.
 *
 *	The macSFReply is a good SFReply record that contains
 *	the name and vRefNum of the file the user chose to
 *	open.
 *
 ***/
 
void CEditApp::OpenDocument(SFReply *macSFReply)

{
	CEditDoc	*theDocument = NULL;

	// In the event that opening the document fails,
	// we setup an exception handler here. If any
	// of the methods called within the scope of this
	// TRY block fail, an exception will be raised and
	// control will be transferred to the CATCH block.
	// Here, the CATCH block takes care of disposing
	// of the partially opened document.

	TRY
	{	
		theDocument = new(CEditDoc);
			
			/**
			 **	Send your document an initialization
			 **	message. The first argument is the
			 **	supervisor (the application). The second
			 **	argument is TRUE if the document is printable.
			 **
			 **/
		
		theDocument->IEditDoc(this, TRUE);
	
			/**
			 **	Send the document an OpenFile() message.
			 **	The document will open a window, open
			 **	the file specified in the macSFReply record,
			 **	and display it in its window.
			 **
			 **/
		theDocument->OpenFile(macSFReply);
	}
	CATCH
	{
		ForgetObject( theDocument);
	}
	ENDTRY;
}

/*** Dispose added for WASTE support ***/
void CEditApp::Dispose(void)
{
		if (gUsingTSM) CloseTSMAwareApplication();
}

/*** make TSMSwitchboard ***/
void CEditApp::MakeSwitchboard( void)
{
	if (gSystem.hasAppleEvents && gUsingTSM)
	{
		FailOSErr(WEInstallTSMHandlers());
	}
	itsSwitchboard = (CSwitchboard *)new CTSMSwitchboard();
	itsSwitchboard->InitAppleEvents(); // needed for TCL 2.0.3
}

/******************************************************************************
 TSMAvailable - returns true if the TextService Manager is available
 ******************************************************************************/

static Boolean TSMAvailable(void)
{
	long	response;
	
	return (Gestalt(gestaltTSMgrVersion, &response)==noErr);
}

/******************************************************************************
 DragAndDropAvailable - returns true if drag and drop is available
 ******************************************************************************/

static Boolean DragAndDropAvailable(void)
{
	long response;

	if (Gestalt(gestaltDragMgrAttr, &response) != noErr) return false;
	if ((response & (1 << gestaltDragMgrPresent)) == 0) return false;
	if ((Ptr)InstallTrackingHandler == kUnresolvedSymbolAddress) return false;
	return true;
}
