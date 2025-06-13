/* StartupOpen.c */
/*****************************************************************************/
/*                                                                           */
/*    System Dependency Library for Building Portable Software               */
/*    Macintosh Version                                                      */
/*    Written by Thomas R. Lawrence, 1993 - 1994.                            */
/*                                                                           */
/*    This file is Public Domain; it may be used for any purpose whatsoever  */
/*    without restriction.                                                   */
/*                                                                           */
/*    This package is distributed in the hope that it will be useful,        */
/*    but WITHOUT ANY WARRANTY; without even the implied warranty of         */
/*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.                   */
/*                                                                           */
/*    Thomas R. Lawrence can be reached at tomlaw@world.std.com.             */
/*                                                                           */
/*****************************************************************************/

#include "MiscInfo.h"
#include "Debug.h"
#include "Audit.h"
#include "Definitions.h"

#ifdef THINK_C
	#pragma options(pack_enums)
#endif
#include <AppleEvents.h>
#ifdef THINK_C
	#pragma options(!pack_enums)
#endif

#include "StartupOpen.h"
#include "Memory.h"
#include "Array.h"
#include "Files.h"


/* this contains a list of things that have been received for opening.*/
/* if StartupList is NIL, then the open event hasn't been received.  Otherwise, */
/* it contains FileSpecs to be opened */
static ArrayRec*					StartupList = NIL;

/* debugging flag */
EXECUTE(static MyBoolean	Initialized = False;)

/* flag that gets set True when a quit signal is received */
static MyBoolean					QuitPending = False;


/* function to check to see that all required parameters have been gotten */
static OSErr		MyGotRequiredParams(AppleEvent* theAppleEvent)
	{
		DescType		ReturnedType;
		Size				ActualSize;
		OSErr				Error;

		Error = AEGetAttributePtr(theAppleEvent, keyMissedKeywordAttr, typeWildCard,
			&ReturnedType,NIL,0,&ActualSize);
		if (Error == errAEDescNotFound)
			{
				return noErr;  /* we got all the params, since no more were found */
			}
		 else
			{
				if (Error == noErr)
					{
						return errAEEventNotHandled;  /* missed some, so it failed */
					}
				 else
					{
						return Error;  /* AEGetAttributePtr failed, so we return why */
					}
			}
	}


/* handler for open application--presents an untitled document */
static pascal	OSErr		MyHandleOApp(AppleEvent* theAppleEvent,
												AppleEvent* reply, long handlerRefcon)
	{
		OSErr		Error;

		APRINT(("+MyHandleOApp"));
		Error = MyGotRequiredParams(theAppleEvent);
		ERROR(Error!=noErr,PRERR(AllowResume,"MyHandleOApp error."));
		if (Error != noErr)
			{
				APRINT(("-MyHandleOApp Error %s",Error));
				return Error;
			}
		StartupList = NewArray();
		APRINT(("-MyHandleOApp"));
		return noErr;
	}


/* handler for open documents */
static pascal	OSErr		MyHandleODoc(AppleEvent* theAppleEvent,
												AppleEvent* reply, long handlerRefcon)
	{
		OSErr						Error;
		long						Index,ItemsInList;
		AEDescList			DocList;
		Size						ActualSize;
		AEKeyword				Keywd;
		DescType				ReturnedType;

		APRINT(("+MyHandleODoc"));
		/* get the direct parameter--a descriptor list--and put it into DocList */
		Error = AEGetParamDesc(theAppleEvent, keyDirectObject, typeAEList, &DocList);
		ERROR(Error!=noErr,PRERR(AllowResume,"MyHandleODoc error."));
		if (Error != noErr) return Error;
		/* check for missing required parameters */
		Error = MyGotRequiredParams(theAppleEvent);
		if (Error != noErr) return Error;
		ERROR(Error!=noErr,PRERR(AllowResume,"MyHandleODoc error."));
		/* count the number of descriptor records in the list */
		Error = AECountItems(&DocList,&ItemsInList);
		ERROR(Error!=noErr,PRERR(AllowResume,"MyHandleODoc error."));
		/* now get each descriptor record from the list, coerce the returned data */
		/* to an FSSpec record, and open the associated file */
		if (StartupList == NIL)
			{
				StartupList = NewArray();
			}
		if (StartupList == NIL)
			{
				/* error -- not enough memory to create the list */
				goto OutOfMemPoint;
			}
		for (Index=1; Index <= ItemsInList; Index += 1)
			{
				FSSpec					MyFSS;

				Error = AEGetNthPtr(&DocList,Index,typeFSS,&Keywd,&ReturnedType,
					(void*)&MyFSS,sizeof(FSSpec),&ActualSize);
				ERROR(Error!=noErr,PRERR(AllowResume,"MyHandleODoc error."));
				if (Error == noErr)
					{
						FSSpec*					ArrayElement;
						long						CharIndex;

						ArrayElement = (FSSpec*)AllocPtrCanFail(sizeof(FSSpec),"StartupOpenFSSpec");
						if (ArrayElement == NIL)
							{
							 FailurePoint1:
								goto OutOfMemPoint;
							}
						*ArrayElement = MyFSS;
						if (!ArrayAppendElement(StartupList,ArrayElement))
							{
							 FailurePoint2:
								ReleasePtr((char*)ArrayElement);
								goto FailurePoint1;
							}
						if (!Eep_RegisterFileSpec((FileSpec*)ArrayElement))
							{
								ArrayDeleteElement(StartupList,
									ArrayFindElement(StartupList,ArrayElement));
								goto FailurePoint2;
							}
					}
			}
	 OutOfMemPoint:
		Error = AEDisposeDesc(&DocList);
		ERROR(Error!=noErr,PRERR(AllowResume,"MyHandleODoc error."));
		APRINT(("-MyHandleODoc"));
		return Error;
	}


/* handle a quit event */
static pascal	OSErr		MyHandleQuit(AppleEvent* theAppleEvent,
												AppleEvent* reply, long handlerRefcon)
	{
		OSErr			Error;

		APRINT(("+MyHandleQuit"));
		/* check for missing required parameters */
		Error = MyGotRequiredParams(theAppleEvent);
		ERROR(Error!=noErr,PRERR(AllowResume,"CApplication::MyHandleQuit error."));
		if (Error != noErr) return Error;
		QuitPending = True;
		APRINT(("-MyHandleQuit"));
		return noErr;
	}


/* compile a list of files that should be opened when the program starts up. */
/* The parameters should be exactly the ones passed into main() upon startup. */
/* It is implementation defined as to whether they will be used; the Macintosh */
/* does not use them, but uses Apple Events for opening startup documents instead. */
void					PrepareStartupDocuments(int argc, char* argv[])
	{
		APRINT(("+PrepareStartupDocuments"));
		ERROR(Initialized,PRERR(ForceAbort,
			"PrepareStartupDocuments called multiple times"));
		EXECUTE(Initialized = True;)
		StartupList = NIL;
		AEInstallEventHandler(kCoreEventClass,kAEOpenApplication,
			(EventHandlerProcPtr)&MyHandleOApp,0,False);
		AEInstallEventHandler(kCoreEventClass,kAEOpenDocuments,
			(EventHandlerProcPtr)&MyHandleODoc,0,False);
		AEInstallEventHandler(kCoreEventClass,kAEQuitApplication,
			(EventHandlerProcPtr)&MyHandleQuit,0,False);
		APRINT(("-PrepareStartupDocuments"));
	}


/* Get a startup item.  It will initially return False.  Once the open event is */
/* received, it will return True from then on.  If there is a file specification */
/* to get, it will be returned, otherwise NIL will be returned.  The file */
/* specification should be disposed of with DisposeFileSpec.  Here's how you know if */
/* you should open an untitled document:  the first time it returns True, if it */
/* also returns NIL, then do it. */
MyBoolean			GetStartupObject(struct FileSpec** ReturnStuff)
	{
		ERROR(!Initialized,PRERR(ForceAbort,"GetStartupList:  StartupOpen not initialized"));
		if (StartupList == NIL)
			{
				return False;
			}
		 else
			{
				if (ArrayGetLength(StartupList) != 0)
					{
						*ReturnStuff = (FileSpec*)ArrayGetElement(StartupList,0);
						ArrayDeleteElement(StartupList,0);
					}
				 else
					{
						*ReturnStuff = NIL;
					}
				return True;
			}
	}


/* clean up any internal structures allocated by PrepareStartupDocument. */
void					ClearStartupDocuments(void)
	{
		APRINT(("+ClearStartupDocuments"));
		ERROR(!Initialized,PRERR(ForceAbort,"GetStartupList:  StartupOpen not initialized"));
		if (StartupList != NIL)
			{
				while (ArrayGetLength(StartupList) != 0)
					{
						ReleasePtr((char*)ArrayGetElement(StartupList,0));
						ArrayDeleteElement(StartupList,0);
					}
				DisposeArray(StartupList);
				StartupList = NIL;
			}
		AERemoveEventHandler(kCoreEventClass,kAEOpenApplication,
			(EventHandlerProcPtr)&MyHandleOApp,False);
		AERemoveEventHandler(kCoreEventClass,kAEOpenDocuments,
			(EventHandlerProcPtr)&MyHandleODoc,False);
		AERemoveEventHandler(kCoreEventClass,kAEQuitApplication,
			(EventHandlerProcPtr)&MyHandleQuit,False);
		APRINT(("-ClearStartupDocuments"));
	}


/* this returns True if the system would like the program to quit.  The program */
/* should then ask the user if he wants to save all changed documents. */
/* A Quit event on the Macintosh will cause this to return True */
MyBoolean			CheckQuitPending(void)
	{
		return QuitPending;
	}


/* If some other signal besides an implementation defined system quit signal */
/* is received, this can be used to indicate such, and cause a normal shutdown */
/* of the program to occur. */
void					SetQuitPending(void)
	{
		APRINT(("+SetQuitPending"));
		QuitPending = True;
		APRINT(("-SetQuitPending"));
	}


/* If the user cancels the quit, this should be used to clear the flag and */
/* allow the program to continue running */
void					AbortQuitInProgress(void)
	{
		APRINT(("+AbortQuitInProgress"));
		QuitPending = False;
		APRINT(("-AbortQuitInProgress"));
	}
