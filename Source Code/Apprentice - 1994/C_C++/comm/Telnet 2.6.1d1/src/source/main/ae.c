/*
*	ae.c
*	Code to handle the AppleEvents we recognize
*****************************************************************
*	NCSA Telnet for the Macintosh								*
*																*
*	National Center for Supercomputing Applications				*
*	Software Development Group									*
*	152 Computing Applications Building							*
*	605 E. Springfield Ave.										*
*	Champaign, IL  61820										*
*																*
*	Copyright (c) 1986-1994,									*
*	Board of Trustees of the University of Illinois				*
****************************************************************/

#include <GestaltEqu.h>
#include <AppleEvents.h>

#include "TelnetHeader.h"
#include "ae.proto.h"
#include "Sets.proto.h"			// For readconfig proto
#include "menuseg.proto.h"		// For HandleQuit proto
#include "debug.h"

SIMPLE_UPP(MyHandleODoc, AEEventHandler);
pascal OSErr  MyHandleODoc (AppleEvent *theAppleEvent, AppleEvent* reply, long
														handlerRefCon)
{
	FSSpec		myFSS;
	AEDescList	docList;
	OSErr		err;
	long		index, itemsInList;
	Size		actualSize;
	AEKeyword	keywd;
	DescType	returnedType;
	FInfo		fileInfo;
	
	if (err = AEGetParamDesc (theAppleEvent, keyDirectObject, typeAEList, &docList))
		return err;

	// check for missing parameters
	if (err = MyGotRequiredParams (theAppleEvent))
		return err;

	// count the number of descriptor records in the list
	if (err = AECountItems (&docList, &itemsInList))
		return err;

	for (index = 1; index <= itemsInList; index++) {
			err = AEGetNthPtr (&docList, index, typeFSS, &keywd, &returnedType, 
								(Ptr) &myFSS, sizeof(myFSS), &actualSize);
			if (err) return err;
			
			FSpGetFInfo(&myFSS, &fileInfo);        /* make sure it's a data file */
			if (fileInfo.fdCreator == creator && fileInfo.fdType == filetype)
				readconfig(myFSS);		// Read the actual set
	}

	err = AEDisposeDesc (&docList);
	return noErr;
}

SIMPLE_UPP(MyHandlePDoc, AEEventHandler);
pascal OSErr  MyHandlePDoc (AppleEvent *theAppleEvent, AppleEvent *reply, long
														handlerRefCon)
{
	// We don't print files.
	return (errAEEventNotHandled);
}

SIMPLE_UPP(MyHandleOApp, AEEventHandler);
pascal OSErr  MyHandleOApp (AppleEvent *theAppleEvent, AppleEvent *reply, long
														handlerRefCon)
{
	// Don't need to do anything for OApp.
	return noErr;
}

SIMPLE_UPP(MyHandleQuit, AEEventHandler);
pascal OSErr  MyHandleQuit (AppleEvent *theAppleEvent, AppleEvent *reply, long
														handlerRefCon)
{
	if (HandleQuit())
		return(-128);	// userCancelledErr
			
	return(noErr);	
}

OSErr MyGotRequiredParams (AppleEvent *theAppleEvent)
{
	DescType	returnedType;
	Size		actualSize;
	OSErr		err;

	err = AEGetAttributePtr (theAppleEvent, keyMissedKeywordAttr,
									typeWildCard, &returnedType, nil, 0,
									&actualSize);
	if (err == errAEDescNotFound)	// you got all the required parameters
			return noErr;
	else if (!err)			// you missed a required parameter
			return errAEEventNotHandled;
	else					// the call to AEGetAttributePtr failed
			return err;
}