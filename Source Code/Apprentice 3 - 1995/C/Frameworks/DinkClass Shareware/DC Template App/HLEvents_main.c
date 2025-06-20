/*
	This is the main function for the DHLEvents application.  This
	file also has the requiered apple event handlers in it.
	Mark Gross 10/10/92
*/

#include "DHLApp.h"
#include "DHLDoc.h"
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
	DHLApp	*theApp;
	
	InitToolBox(5);

	if(!System7Available() )
		DebugStr( "\p No System 7!!" );

	DEventHandler::gApplication = NULL;// to make sure no messages get sent to gApplication 
									// before it is instanicated.  otherwise the Application
									// object will be sending install handler message to itself
									// befor it is fully instaniated.
	
	theApp = new DHLApp;
	
	DEventHandler::gApplication = theApp;
	DEventHandler::gPassItOn = TRUE;

	theApp->fCreator = '????';// use your faverate initials
	theApp->fClipType = '????';
	theApp->fMainFileType = '????';

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
// this application has no file so ODOC AE will not be of
// much use here.

	OSErr myErr;

	myErr = RequiredCheck( theAppleEvent);
	if (myErr) 
		return myErr;
	
	return errAEEventNotHandled;
}
