////////////
//
//	CChessApp.cp
//
//	Application-level methods for Chess++.
//
//	Copyright � 1993 Steven J. Bushell. All rights reserved.
//
////////////

#include <CBartender.h>
#include <Commands.h>
#include "ChessCommands.h"
#include "CChessApp.h"
#include "CChessDoc.h"
#include "CBrain.h"
#include "CAboutChessBox.h"
#include "CChessSplashScreen.h"

#include <Lomem.h>

extern 	void		TimeSpeak(void);

extern	CBureaucrat	*gGopher;
extern	CBartender	*gBartender;
extern	OSType		gSignature;
extern	RgnHandle	gChessBoardRgnHandle;
extern	CBrain		*gBrain;

extern	CIconHandle	gWhitePawnCicnHandle;
extern	CIconHandle	gWhiteKnightCicnHandle;
extern	CIconHandle	gWhiteBishopCicnHandle;
extern	CIconHandle	gWhiteRookCicnHandle;
extern	CIconHandle	gWhiteQueenCicnHandle;
extern	CIconHandle	gWhiteKingCicnHandle;

extern	CIconHandle	gBlackPawnCicnHandle;
extern	CIconHandle	gBlackKnightCicnHandle;
extern	CIconHandle	gBlackBishopCicnHandle;
extern	CIconHandle	gBlackRookCicnHandle;
extern	CIconHandle	gBlackQueenCicnHandle;
extern	CIconHandle	gBlackKingCicnHandle;

extern	CursHandle	gCrossCursor;
extern	CursHandle	gPlusCursor;

#define		kExtraMasters		4
#define		kRainyDayFund		20480
#define		kCriticalBalance	20480
#define		kToolboxBalance		20480


/***
 * IChessApp
 *
 *	Initialize the Chess++ application.
 *
 ***/

void CChessApp::IChessApp(void)

{
	register		rank,file;
	Rect			rankRect;
	CChessSplashScreen	*theSplashScreen;
	long			timer = Ticks;
	
	CApplication::IApplication( kExtraMasters, kRainyDayFund, 
						kCriticalBalance, kToolboxBalance);

	//	show the splash screen
	theSplashScreen = new CChessSplashScreen;
	theSplashScreen->IChessSplashScreen(this);

	// initialize checkerboard region for use in CChessBoard::Draw()
	gChessBoardRgnHandle = NewRgn();
	OpenRgn();
	PenSize(1,1);
	SetRect(&rankRect,32,0,64,32);
	for(file=0;file<4;file++)
	{
		for(rank=0;rank<4;rank++)
		{
			FrameRect(&rankRect);
			OffsetRect(&rankRect,-32,32);
			FrameRect(&rankRect);
			OffsetRect(&rankRect,96,-32);
		}
		OffsetRect(&rankRect,-256,64);
	}
	CloseRgn(gChessBoardRgnHandle);
	
	gWhitePawnCicnHandle = GetCIcon(128);
	gWhiteKnightCicnHandle = GetCIcon(129);
	gWhiteBishopCicnHandle = GetCIcon(130);
	gWhiteRookCicnHandle = GetCIcon(131);
	gWhiteQueenCicnHandle = GetCIcon(132);
	gWhiteKingCicnHandle = GetCIcon(133);

	gBlackPawnCicnHandle = GetCIcon(134);
	gBlackKnightCicnHandle = GetCIcon(135);
	gBlackBishopCicnHandle = GetCIcon(136);
	gBlackRookCicnHandle = GetCIcon(137);
	gBlackQueenCicnHandle = GetCIcon(138);
	gBlackKingCicnHandle = GetCIcon(139);

	// Set up 'normal' idling speed for the board
	cMaxSleepTime = 2;

	//	get rid of splash screen after appropriate delay
	while (!Button())
		if (Ticks-timer>333)
			break;
	theSplashScreen->Dispose();
	FlushEvents(mDownMask+mUpMask+keyDownMask+keyUpMask+autoKeyMask, 0);
}



/***
 * SetUpFileParameters
 *
 *	Specify the kinds of files your
 *	application opens.
 *
 ***/

void CChessApp::SetUpFileParameters(void)

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
	sfFileTypes[0] = 'CBRD';

		/**
		 **	Although it's not an instance variable,
		 **	this method is a good place to set the
		 **	gSignature global variable. Set this global
		 **	to your application's signature. You'll use it
		 **	to create a file (see CFile::CreateNew()).
		 **
		 **/

	gSignature = 'SJB3';
}


/***
 * SetUpMenus 
 *
 * Set up menus which must be created at run time, such as a
 * Font menu. You can eliminate this method if your application
 * does not have any such menus.
 *
***/

 void CChessApp::SetUpMenus()
 {

	inherited::SetUpMenus();  /*  Superclass takes care of adding     
                                menus specified in a MBAR id = 1    
                                resource    
                            */                          

	gBartender->SetDimOption(MENUchess,dimALL);
	
        /* Add your code for creating run-time menus here */    
 }



/***
 * DoCommand
 *
 *	Your application will probably handle its own commands.
 *	Remember, the command numbers from 1-1023 are reserved.
 *  The file Commands.h contains all the predefined TCL
 *  commands.
 *
 *	Be sure to call the default method, so you can get
 *	the default behvior for standard commands.
 *
 ***/
void CChessApp::DoCommand(long theCommand)

{
	CAboutChessBox	*theAboutBox;

	switch (theCommand) {
	
		case cmdAbout:
			theAboutBox = new CAboutChessBox;
			theAboutBox->IAboutChessBox(this);
			theAboutBox->Dispose();
			FlushEvents(mDownMask+mUpMask+keyDownMask+keyUpMask+autoKeyMask, 0);
			break;
		case cmdTellTime:
			TimeSpeak();
			break;
		default:
			inherited::DoCommand(theCommand);
			break;
	}
}


/***
 *
 * UpdateMenus 
 *
 *   Perform menu management tasks
 *
***/

 void CChessApp::UpdateMenus()
 {
	inherited::UpdateMenus();     /* Enable standard commands */      

	gBartender->EnableCmd(cmdTellTime);
}


/***
 * Exit
 *
 *	Chances are you won't need this method.
 *	This is the last chance your application gets to clean up
 *  things like temporary files before terminating.
 *
 ***/
 
void CChessApp::Exit()

{
	/* your exit handler here */
}


/***
 * CreateDocument
 *
 *	The user chose New from the File menu.
 *	In this method, you need to create a document and send it
 *	a NewFile() message.
 *
 ***/

void CChessApp::CreateDocument()

{
	CChessDoc	*theDocument = NULL;
	
	TRY
	{
		theDocument = new(CChessDoc);
			
			/**
			 **	Send your document an initialization
			 **	message. The first argument is the
			 **	supervisor (the application). The second
			 **	argument is TRUE if the document is printable.
			 **
			 **/
		
		theDocument->IChessDoc(this, TRUE);
	
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
		/*
		 * This exception handler gets executed if a failure occurred 
		 * anywhere within the scope of the TRY block above. Since 
		 * this indicates that a new doc could not be created, we
		 * check if an object had been allocated and if it has, send 
		 * it a Dispose message. The exception will propagate up to
		 * CSwitchboard's exception handler, which handles displaying
		 * an error alert.
		 */
		 
		 if (theDocument) theDocument->Dispose();

	}
	ENDTRY;
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
 
void CChessApp::OpenDocument(SFReply *macSFReply)

{
	CChessDoc	*theDocument = NULL;
	
	TRY
	{
	
		theDocument = new(CChessDoc);
			
			/**
			 **	Send your document an initialization
			 **	message. The first argument is the
			 **	supervisor (the application). The second
			 **	argument is TRUE if the document is printable.
			 **
			 **/
		
		theDocument->IChessDoc(this, TRUE);
	
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