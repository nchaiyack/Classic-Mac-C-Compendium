/*****
 * CBrowserApp.c
 *
 *	Application methods for a typical application.
 *
 *  Copyright � 1990 Symantec Corporation.  All rights reserved.
 *  Copyright � 1991 Manuel A. P�rez.  All rights reserved.
 *
 *****/

#include "CBrowserApp.h"
#include "CBrowserDoc.h"
#include <Commands.h>

extern	OSType	gSignature;

#define		kExtraMasters		4
#define		kRainyDayFund		20480
#define		kCriticalBalance	20480
#define		kToolboxBalance		20480


/***
 * IBrowserApp
 ***/

void CBrowserApp::IBrowserApp(void)

{
	CApplication::IApplication( kExtraMasters, kRainyDayFund, 
						kCriticalBalance, kToolboxBalance);
}

/***
 * SetUpFileParameters
 ***/

void CBrowserApp::SetUpFileParameters(void)

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

	gSignature = 'dbws';
}

/***
 * DoCommand - handle about box
 ***/
void CBrowserApp::DoCommand(long theCommand)

{
	switch (theCommand) {
		case cmdAbout:
			Alert(1000, NULL);
			break;

		default:	inherited::DoCommand(theCommand);
					break;
	}
}

/***
 * OpenDocument
 *
 *	The user chose Open� from the File menu.
 *	In this method you need to create a document
 *	and send it an OpenFile() message.
 *
 *	The macSFReply is a good SFReply record that contains
 *	the name and vRefNum of the file the user chose to
 *	open.
 *
 ***/
 
void CBrowserApp::OpenDocument(SFReply *macSFReply)
{
CBrowserDoc	*theDocument = NULL;
	
	TRY
	{
	
		theDocument = new(CBrowserDoc);
			
			/**
			 **	Send your document an initialization
			 **	message. The first argument is the
			 **	supervisor (the application). The second
			 **	argument is TRUE if the document is printable.
			 **
			 **/
		
		theDocument->IBrowserDoc(this, TRUE);
	
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
		/*
		 * This exception handler gets executed if a failure occurred 
		 * anywhere within the scope of the TRY block above. Since 
		 * this indicates that the document could not be opened, we
		 * send it a Dispose message. The exception will propagate up to
		 * CSwitchboard's exception handler, which handles displaying
		 * an error alert.
		 */
		 
		 if (theDocument) theDocument->Dispose();
	}
	ENDTRY;
}

/******************************************************************************
 InitToolbox

		Initialize the Macintosh Toolbox
 ******************************************************************************/

void	CBrowserApp::InitToolbox(void)
{
	// do nothing, the toolbox is initialized outside of this
	// object, so that we can put up a splash screen
}
