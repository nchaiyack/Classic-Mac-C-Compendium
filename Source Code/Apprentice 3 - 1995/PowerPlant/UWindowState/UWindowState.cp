// ===========================================================================
//	UWindowState.cp						   �1994 J. Rodden, DD/MF & Associates
// ===========================================================================
// Utilities for saving and restoring a window's state (position, size, and
// zoom state) in a file. Typically usefull for document windows as per
// Apple's User Interface Guidelines. Use when opening and saving a document.
//
// All rights reserved. You may use this code in any application, recognition
// would be appreciated but is not required.

#include <UWindowState.h>

#include <LFile.h>
#include <LWindow.h>

const ResType kWindowStateRsrc = 'WSET';

struct WindowStateData {
	Rect	userBounds;
	Boolean	isZoomed;
};

// ===========================================================================

void
UWindowState::SaveWindowState( LFile* inFile, LWindow* inWindow, short inResID)
{
	// Make file a resource fork if it doesn't already have one
	FSSpec	theFileSpec;
	FInfo	theFileInfo;
	
	inFile->GetSpecifier(theFileSpec);
	
	::FSpGetFInfo( &theFileSpec, &theFileInfo);
	::FSpCreateResFile( &theFileSpec, theFileInfo.fdCreator, theFileInfo.fdType, nil);


	// -----------------------------------------
	// Get the window state
	Rect			dummyRect;
	WindowStateData	theWindowState;
	
	inWindow->CalcPortFrameRect(theWindowState.userBounds);
	inWindow->PortToGlobalPoint(topLeft(theWindowState.userBounds));
	inWindow->PortToGlobalPoint(botRight(theWindowState.userBounds));
	
	theWindowState.isZoomed = inWindow->CalcStandardBounds(dummyRect);


	// -----------------------------------------
	// Store the window state
	inFile->OpenResourceFork(fsWrPerm);
	
	WindowStateData	**theHandle = 
					(WindowStateData**) ::Get1Resource( kWindowStateRsrc, inResID);
	
	if ( theHandle != nil )	{
		// a state data resource already exists -- update it
		**theHandle = theWindowState;
	  ::ChangedResource(Handle(theHandle));
	} else {
		// no state data has yet been saved -- add state data resource
		theHandle = (WindowStateData**) (NewHandle(sizeof(theWindowState)));
		if ( theHandle != nil ) {
			**theHandle = theWindowState;
		  ::AddResource( Handle(theHandle), kWindowStateRsrc, inResID, nil);
		}
	}
	
	if ( theHandle != nil ) {
	  ::UpdateResFile(inFile->GetResourceForkRefNum());
	  ::ReleaseResource(Handle(theHandle));
	}
	
	inFile->CloseResourceFork();
}


// ===========================================================================

void
UWindowState::RestoreWindowState( LFile* inFile, LWindow* inWindow, short inResID)
{
	//WStateData	theWindowState;
	WindowStateData	theWindowState;
	WindowStateData	**theHandle;
	
	
	// -----------------------------------------
	// Get saved window state
	inFile->OpenResourceFork(fsRdPerm);
		
	theHandle = (WindowStateData**) ::Get1Resource( kWindowStateRsrc, inResID);
		
	// -----------------------------------------
	// Restore window state
	if ( theHandle != nil )	{	// handle to data succeeded -- retrieve saved user state
		theWindowState = **theHandle;
		
		inWindow->DoSetBounds(theWindowState.userBounds);
		inWindow->DoSetZoom(theWindowState.isZoomed);
	}	
	
  ::ReleaseResource(Handle(theHandle));
  
	inFile->CloseResourceFork();
}

