#include <Aliases.h>
#include <AppleEvents.h>
#include <Drag.h>
#include <QuickDraw.h>
#include <QDOffscreen.h>
#include <Finder.h>
#include <StandardFile.h>

#include "rsrcIDs.h"
#include "demoapp rsrcIDs.h"

#include "memobject.h"
#include "handle.h"
#include "qdobject.h"
#include "recordable.h"
#include "rect.h"
#include "region.h"
#include "picture.h"

#include "appleeventhandler.h"
#include "draghandler.h"

#include "grafport.h"
#include "windowrecord.h"
#include "window.h"
#include "dialog.h"
#include "alert.h"
#include "application.h"

#include "demowindow.h"
#include "demoapp.h"


#include "standardgetfile.h"

demoapp::demoapp()
	: application()
	, dragreceiver()
{
	(void)new demowindow( 128, 128);
}

Boolean demoapp::HandleMenuSelection( long selection)
{
	const short theMenu = HiWord( selection);
	const short theItem = LoWord( selection);

	Boolean result = true;
	switch( theMenu)
	{
		case kAppleMenuID:
			result = HandleAppleMenu( theItem);
			break;

		case kFileMenuID:
			result = HandleFileMenu( theItem);
			break;

		case kEditMenuID:
			result = false;
			break;			
	}	
	return result;
}

OSErr demoapp::DragEnterWindow( WindowPtr theWindow, DragReference theDragRef)
{
	window *theWindowInvolved = WindowPtr2window( theWindow);
	
	const OSErr result =
				theWindowInvolved->DragEnterWindow( theDragRef);

	return result;
}

OSErr demoapp::DragInWindow( WindowPtr theWindow, DragReference theDragRef)
{
	window *theWindowInvolved = WindowPtr2window( theWindow);
	
	const OSErr result =
				theWindowInvolved->DragInWindow( theDragRef);

	return result;
}

OSErr demoapp::DragLeaveWindow( WindowPtr theWindow, DragReference theDragRef)
{
	window *theWindowInvolved = WindowPtr2window( theWindow);
	
	const OSErr result =
				theWindowInvolved->DragLeaveWindow( theDragRef);

	return result;
}

OSErr demoapp::theRealReceiveHandler( WindowPtr theWindow, DragReference theDragRef)
{
	window *theWindowInvolved = WindowPtr2window( theWindow);
	
	const OSErr result =
				theWindowInvolved->HandleDrop( theDragRef);
	return result;
}

Boolean demoapp::HandleAppleMenu( short theItem)
{
	Boolean result = true;
	if( theItem == 1)
	{
		dialog aboutDialog( 128);
		aboutDialog.ShowTillClick();
	} else {
		Str255 itemStr;	// Str63 (since FSSpec contains one) or even Str31 (current limit)?
		GetMenuItemText( GetMenuHandle( kAppleMenuID), theItem, itemStr);
		OpenDeskAcc( itemStr);
	}
	return result;
}

Boolean demoapp::HandleFileMenu( short theItem)
{
	Boolean result = true;
	switch( theItem)
	{
		case kNewWindowItem:
			(void)new demowindow( 128, 128);
			break;
		
		case kOpenItem:
			{
				OSType PICT_type = 'PICT';
				standardgetfile ask_the_boss( sfGetDialogID, &PICT_type, 1L); 
				
				ask_the_boss.doIt();
				
				if( ask_the_boss().sfGood)
				{
					(void)new demowindow( 128, ask_the_boss().sfFile);
				}
			}
			break;
			
		case kCloseItem:
			WindowPtr theMacWindow = FrontWindow();
			if( theMacWindow)
			{
				window *thewindow = WindowPtr2window( theMacWindow);
				if( thewindow)
				{
					CloseAWindow( thewindow);
				}
			}
			break;

		case kQuitItem:
			SendQuitToSelf();
			break;
	}
	return result;
}

OAPP_Handler::OAPP_Handler()
	: appleeventhandler( kCoreEventClass, kAEOpenApplication)
{
}

OSErr OAPP_Handler::theRealHandler( AppleEvent *theAppleEvent, AppleEvent *reply)
{
	//
	// This thing doesn't do anything. Instead,
	// we could temporarily show an about box.
	//
	return noErr;
}

ODOC_PDOC_Handler::ODOC_PDOC_Handler( AEEventID theAEEventID)
	: appleeventhandler( kCoreEventClass, theAEEventID)
	, theID( theAEEventID)
{
}

OSErr ODOC_PDOC_Handler::theRealHandler( AppleEvent *theAppleEvent, AppleEvent *reply)
{
	AEDescList	docList;
	long		itemsInList;

	OSErr result =
		AEGetParamDesc( theAppleEvent, keyDirectObject, typeAEList, &docList);

	if( result == noErr)
	{
		result = GotRequiredParams( theAppleEvent);
		if( result == noErr)
		{
			result = AECountItems( &docList, &itemsInList);
		}
	}
	for( int index = 1; (index <= itemsInList) && (result == noErr); index++)
	{
		FSSpec		aFSSpec;
		Size		actualSize;
		AEKeyword	keywd;
		DescType	typeCode;

		result = AEGetNthPtr( &docList, index, typeFSS, &keywd,
						&typeCode, (Ptr)&aFSSpec, sizeof( aFSSpec), &actualSize);

		if( result == noErr)
		{
			if( theID == kAEOpenDocuments)
			{
				DebugStr( "\pShould open a doc here!");
			} else {
				DebugStr( "\pShould print a doc here!");
			}
		}
	}
	const OSErr dispose_result = AEDisposeDesc( &docList);
	if( result == noErr)
	{
		result = dispose_result;
	}
	return result;
}

QUIT_Handler::QUIT_Handler()
	: appleeventhandler( kCoreEventClass, kAEQuitApplication)
{
}

OSErr QUIT_Handler::theRealHandler( AppleEvent *theAppleEvent, AppleEvent *reply)
{
	application::theapplication()->MayQuitNow();
	return noErr;
}
