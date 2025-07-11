/*
	File:		TEditMainProgram.c

	Written by:	Mark Gross

	Copyright:	� 1992 by Applied Technical Software, all rights reserved.
	Use at your own risk.

*/
//
// this is the model main function modual for the typical DinkClass
// application
//

#include "DTEditApp.h"
#include "DTEditDoc.h"
#include "DinkUtils.h"

//
//Core apple event prototypes
//

void DoAEInstallation(void);

pascal OSErr HandleQUIT( AppleEvent *theAppleEvent, AppleEvent *reply,
						 long myRefCon);
pascal OSErr HandleOAPP( AppleEvent *theAppleEvent, AppleEvent *reply, 
						 long myRefCon);
pascal OSErr HandlePDOC( AppleEvent *theAppleEvent, AppleEvent *reply,
						 long myRefCon);
pascal OSErr HandleODOC( AppleEvent *theAppleEvent, AppleEvent *reply,
						 long myRefCon);


main()
{
	DApplication	*theApp;
	
	InitToolBox(5);

	if(!System7Available() )
		DebugStr( "\p No System 7!!" );

	DEventHandler::gApplication = NULL;// to make sure no messages get sent to gApplication 
									// before it is instanicated.  otherwise the Application
									// object will be sending install handler message to itself
									// befor it is fully instaniated.
	
	theApp = new DTEditApp;
	
	DEventHandler::gApplication = theApp;
	DEventHandler::gPassItOn = TRUE;

	theApp->fCreator = 'MkGr';
	theApp->fClipType = 'TEXT';
	theApp->fMainFileType = 'TEXT';

	DoAEInstallation();// the AE-handler procs refrence gApplication 
						// so I'm initializing the AEvents after I have
						// a valid gApplication
	
	
	if(theApp->InitApp() )
	{			
		theApp->MakeDDoc(FALSE);
		do
		{
			theApp->EventLoop();
		} while(!theApp->CleanUp());
	}
	delete theApp;
	ExitToShell();	//last good bye kiss
}// the end




void DoAEInstallation(void)
{

	AEInstallEventHandler( kCoreEventClass, kAEOpenDocuments,
							(EventHandlerProcPtr)HandleODOC, 0, false);

	AEInstallEventHandler( kCoreEventClass, kAEQuitApplication,
							(EventHandlerProcPtr)HandleQUIT, 0, false);

	AEInstallEventHandler( kCoreEventClass, kAEPrintDocuments,
							(EventHandlerProcPtr)HandlePDOC, 0, false);

	AEInstallEventHandler( kCoreEventClass, kAEOpenApplication,
							(EventHandlerProcPtr)HandleOAPP, 0, false);

}							



pascal OSErr HandleQUIT( AppleEvent *theAppleEvent, AppleEvent *reply, long myRefCon)
{
	OSErr myErr;
	
	myErr = RequiredCheck( theAppleEvent);
	if (myErr) 
		return myErr;
	
	DEventHandler::gApplication->fDone = true;
	return noErr;
}



pascal OSErr HandleOAPP( AppleEvent *theAppleEvent, AppleEvent *reply, long myRefCon)
{
	OSErr myErr;

	myErr = RequiredCheck( theAppleEvent);
	if (myErr) 
		return myErr;
	
	return noErr;
}



pascal OSErr HandlePDOC( AppleEvent *theAppleEvent, AppleEvent *reply, long myRefCon)
{
	OSErr myErr;

	myErr = RequiredCheck( theAppleEvent);
	if (myErr) 
		return myErr;
	
	return errAEEventNotHandled;
}


pascal OSErr HandleODOC( AppleEvent *theAppleEvent, AppleEvent *reply, long myRefCon)
{
	OSErr myErr;
	AEDescList  docList;
	FSSpec		myFSS;
	long		i;
	long		itemsInList;
	AEKeyword	theKeyWord;
	DescType	typeCode;
	Size		actualSize;
	WindowPtr	docWindow;
	DDocument* newDoc;

	myErr = AEGetParamDesc(theAppleEvent, keyDirectObject, typeAEList, &docList);
	if ( myErr)
		return(myErr);

	myErr = RequiredCheck( theAppleEvent);
	if (myErr) 
		return myErr;


	myErr = AECountItems(&docList, &itemsInList);
	if (myErr) 
		return myErr;
		
	for (i = 1; ((i <= itemsInList) && (!myErr)); ++i) 
	{
		myErr = AEGetNthPtr( &docList, i, typeFSS, &theKeyWord,	&typeCode, 
			(Ptr)&myFSS, sizeof(FSSpec), &actualSize );
		
		if (myErr) 
			return myErr;
		
		newDoc = new DTEditDoc;
		if (newDoc)
			newDoc->AEInitDoc(&myFSS); 
	}
	return noErr;
}

