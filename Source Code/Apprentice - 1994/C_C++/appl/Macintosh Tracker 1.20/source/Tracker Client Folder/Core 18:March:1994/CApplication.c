/* CApplication.c */

#define COMPILING_CAPPLICATION_C
#include "CApplication.h"
#include "CMyScrap.h"
#include "MenuController.h"
#include "EventLoop.h"
#include "CMyDocument.h"
#include "CAboutWindow.h"
#include "Compatibility.h"
#include "StandardFile.h"
#include "CSack.h"
#include "Alert.h"
#include "Memory.h"
#include "File.h"
#include "CMyApplication.h"
#include "WatchCursor.h"


#define NotEnoughMemoryID (130L*65536L + 2)

static	CApplication*	CApplication::GlobalApplication = NIL; /* so high level event handers can reach it */


CMyApplication*		Application;


/* */			CApplication::CApplication()
	{
		MyBoolean		NoErr;
		CSack*			Temp;

		/* initializing instance variables */
		Temp = new CSack;
		Temp->ISack(sizeof(CDocument*),256);
		ListOfDocuments = Temp;
		AboutWindow = NIL;

		GlobalApplication = this;
		/* installing apple event handlers */
		NoErr = True;
		NoErr = NoErr && (AEInstallEventHandler(kCoreEventClass,
			kAEOpenApplication,&MyHandleOApp,0,False) == noErr);
		NoErr = NoErr && (AEInstallEventHandler(kCoreEventClass,
			kAEOpenDocuments,&MyHandleODoc,0,False) == noErr);
		NoErr = NoErr && (AEInstallEventHandler(kCoreEventClass,
			kAEPrintDocuments,&MyHandlePDoc,0,False) == noErr);
		NoErr = NoErr && (AEInstallEventHandler(kCoreEventClass,
			kAEQuitApplication,&MyHandleQuit,0,False) == noErr);
		if (!NoErr)
			{
				PRERR(ForceAbort,"Unable to install Apple Event Handlers!");
			}

		IWindow(LongPointOf(-16383,-16383),LongPointOf(-16383 + 32,-16383 + 32),
			ModelessWindow,NoGrowable,NoZoomable);
		SelectWindow(MyGrafPtr);
	}


/* */			CApplication::~CApplication()
	{
		delete ListOfDocuments;
		ERROR(AboutWindow!=NIL,PRERR(ForceAbort,"About Window wasn't deleted."));
	}


/* function to check to see that all required parameters have been gotten */
static	OSErr		CApplication::MyGotRequiredParams(AppleEvent* theAppleEvent)
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
static	pascal	OSErr	CApplication::MyHandleOApp(AppleEvent* theAppleEvent,
												AppleEvent* reply, long handlerRefcon)
	{
		OSErr		Error;

		Error = MyGotRequiredParams(theAppleEvent);
		ERROR(Error!=noErr,PRERR(AllowResume,"CApplication::MyHandleOApp error."));
		if (Error != noErr)
			{
				return Error;
			}
		GlobalApplication->DispatchOpenApplication();
		return noErr;
	}

/* handler for open documents */
static	pascal	OSErr	CApplication::MyHandleODoc(AppleEvent* theAppleEvent,
												AppleEvent* reply, long handlerRefcon)
	{
		OSErr				Error;
		FSSpec			MyFSS;  /* place to put file info */
		long				Index,ItemsInList;
		AEDescList	DocList;
		Size				ActualSize;
		AEKeyword		Keywd;
		DescType		ReturnedType;

		/* get the direct parameter--a descriptor list--and put it into DocList */
		Error = AEGetParamDesc(theAppleEvent, keyDirectObject, typeAEList, &DocList);
		ERROR(Error!=noErr,PRERR(AllowResume,"CApplication::MyHandleODoc error."));
		if (Error != noErr) return Error;
		/* check for missing required parameters */
		Error = MyGotRequiredParams(theAppleEvent);
		if (Error != noErr) return Error;
		ERROR(Error!=noErr,PRERR(AllowResume,"CApplication::MyHandleODoc error."));
		/* count the number of descriptor records in the list */
		Error = AECountItems(&DocList,&ItemsInList);
		ERROR(Error!=noErr,PRERR(AllowResume,"CApplication::MyHandleODoc error."));
		/* now get each descriptor record from the list, coerce the returned data */
		/* to an FSSpec record, and open the associated file */
		if (WeAreActiveApplication)
			{
				StartTimeConsumingOperation();
			}
		for (Index=1; Index <= ItemsInList; Index += 1)
			{
				Error = AEGetNthPtr(&DocList,Index,typeFSS,&Keywd,&ReturnedType,
					(void*)&MyFSS,sizeof(FSSpec),&ActualSize);
				ERROR(Error!=noErr,PRERR(AllowResume,"CApplication::MyHandleODoc error."));
				if (Error == noErr)
					{
						GlobalApplication->DispatchOpenDocument(&MyFSS);
					}
				if (WeAreActiveApplication)
					{
						CheckCursor();
					}
			}
		if (WeAreActiveApplication)
			{
				EndTimeConsumingOperation();
			}
		Error = AEDisposeDesc(&DocList);
		ERROR(Error!=noErr,PRERR(AllowResume,"CApplication::MyHandleODoc error."));
		return Error;
	}

/* handler for print documents */
static	pascal	OSErr	CApplication::MyHandlePDoc(AppleEvent* theAppleEvent,
												AppleEvent* reply, long handlerRefcon)
	{
		OSErr				Error;
		FSSpec			MyFSS;  /* place to put file info */
		long				Index,ItemsInList;
		AEDescList	DocList;
		Size				ActualSize;
		AEKeyword		Keywd;
		DescType		ReturnedType;

		/* get the direct parameter--a descriptor list--and put it into DocList */
		Error = AEGetParamDesc(theAppleEvent, keyDirectObject, typeAEList, &DocList);
		ERROR(Error!=noErr,PRERR(AllowResume,"CApplication::MyHandlePDoc error."));
		if (Error != noErr) return Error;
		/* check for missing required parameters */
		Error = MyGotRequiredParams(theAppleEvent);
		ERROR(Error!=noErr,PRERR(AllowResume,"CApplication::MyHandlePDoc error."));
		if (Error != noErr) return Error;
		/* count the number of descriptor records in the list */
		Error = AECountItems(&DocList,&ItemsInList);
		ERROR(Error!=noErr,PRERR(AllowResume,"CApplication::MyHandlePDoc error."));
		/* now get each descriptor record from the list, coerce the returned data */
		/* to an FSSpec record, and open the associated file */
		for (Index=1; Index <= ItemsInList; Index += 1)
			{
				Error = AEGetNthPtr(&DocList,Index,typeFSS,&Keywd,&ReturnedType,
					(void*)&MyFSS,sizeof(FSSpec),&ActualSize);
				ERROR(Error!=noErr,PRERR(AllowResume,"CApplication::MyHandlePDoc error."));
				if (Error == noErr)
					{
						GlobalApplication->DispatchPrintDocument(&MyFSS);
					}
			}
		Error = AEDisposeDesc(&DocList);
		ERROR(Error!=noErr,PRERR(AllowResume,"CApplication::MyHandlePDoc error."));
		return Error;
	}

/* handle a quit event */
static	pascal	OSErr	CApplication::MyHandleQuit(AppleEvent* theAppleEvent,
												AppleEvent* reply, long handlerRefcon)
	{
/*	MyBoolean	UserCanceled; */
		OSErr			Error;

		/* check for missing required parameters */
		Error = MyGotRequiredParams(theAppleEvent);
		ERROR(Error!=noErr,PRERR(AllowResume,"CApplication::MyHandleQuit error."));
		if (Error != noErr) return Error;
		/* UserCanceled = */ GlobalApplication->DispatchQuit();
/*	if (UserCanceled)
			{
				return userCanceledErr;
			}
		 else
			{
				return noErr;
			}	*/
		return noErr;
	}


/* accept and process high level events */
void		CApplication::ReceiveHighLevelEvent(EventRecord* Event)
	{
		OSErr		Error;

		Error = AEProcessAppleEvent(Event);
		ERROR(Error != noErr,PRERR(AllowResume,
			"CApplication::ReceiveHighLevelEvent AEProcessAppleEvent error."));
	}


void		CApplication::RegisterDocument(CDocument* TheDocument)
	{
		ListOfDocuments->PushElement(&TheDocument);
	}


void		CApplication::DeregisterDocument(CDocument* TheDocument)
	{
		ListOfDocuments->KillElement(&TheDocument);
	}


void		CApplication::InitiateQuit(void)
	{
		if (AboutWindow != NIL)
			{
				delete AboutWindow;
				AboutWindow = NIL;
			}
		while (ListOfDocuments->NumElements() != 0)
			{
				CDocument*	TheDocument;

				ListOfDocuments->ResetScan();
				ListOfDocuments->GetNext(&TheDocument);
				if (!TheDocument->GoAway())
					{
						return; /* quit operation was aborted */
					}
			}
		/* quit operation was not aborted: all documents closed successfully */
		QuitPending = True;
	}


/* if the active window couldn't handle a command, it gets sent here */
MyBoolean	CApplication::DoMenuCommand(ushort MenuCommand)
	{
		switch (MenuCommand)
			{
				case mFileNew:
					DoMenuNew();
					return True;
				case mFileOpen:
					DoMenuOpen();
					return True;
				case mAppleAbout:
					DoAboutWindow(False);
					return True;
				case mFileQuit:
					InitiateQuit();
					return True;
				default:
					return False;
			}
	}


/* create a new document */
void		CApplication::DispatchOpenApplication(void)
	{
		DoMenuNew();
	}


/* open a document using specified file */
void		CApplication::DispatchOpenDocument(FSSpec* TheFSSpec)
	{
		CMyDocument*	ADoc;

		if (!FirstMemCacheValid())
			{
				AlertError(NotEnoughMemoryID,NIL);
				return;
			}
		ADoc = new CMyDocument;  /* create the document */
		ADoc->DoOpenFile(TheFSSpec);  /* make it link to a file */
	}


/* open the document, print it, and close it */
void		CApplication::DispatchPrintDocument(FSSpec* TheFSSpec)
	{
		CMyDocument*	ADoc;

		if (!FirstMemCacheValid())
			{
				AlertError(NotEnoughMemoryID,NIL);
				return;
			}
		ADoc = new CMyDocument;  /* create the document */
		ADoc->DoOpenFile(TheFSSpec);  /* make it link to a file */
		ADoc->DoPrint();  /* cause it to print itself */
		ADoc->GoAway();  /* cause it to go away */
	}


/* tell document tracker to close all documents.  If user aborts, it return true, otherwise */
/* it return false */
void		CApplication::DispatchQuit(void)
	{
		InitiateQuit(); /* indicate that we should quit when everything closes */
	}


void		CApplication::DoAboutWindow(MyBoolean AutoFlag)
	{
		CAboutWindow*		TempAboutWindow;

		if (AboutWindow != NIL)
			{
				AboutWindow->BecomeActiveWindow();
			}
		 else
			{
				if (!FirstMemCacheValid())
					{
						AlertError(NotEnoughMemoryID,NIL);
						return;
					}
				TempAboutWindow = new CAboutWindow;
				TempAboutWindow->IAboutWindow(AutoFlag);
				AboutWindow = TempAboutWindow;
			}
	}


/* enable items the application can handle */
void		CApplication::EnableMenuItems(void)
	{
		MyEnableItem(mFileNew);
		MyEnableItem(mFileOpen);
		MyEnableItem(mAppleAbout);
		MyEnableItem(mFileQuit);
	}


void		CApplication::InitMenuBar(void)
	{
		EXECUTE(PRERR(ForceAbort,"CApplication::InitMenuBar isn't overridden"));
	}


void		CApplication::DoMenuNew(void)
	{
		CMyDocument*	ADoc;

		if (!FirstMemCacheValid())
			{
				AlertError(NotEnoughMemoryID,NIL);
				return;
			}
		ADoc = new CMyDocument;
		ADoc->DoNewFile();
	}


/* open a file using the standard file things */
void		CApplication::DoMenuOpen(void)
	{
		FSSpec			FileInfo;
		OSType			TypeList[NUMFILETYPES];
		short				NumTypes;
		pascal Boolean (*FileFilter)(CInfoPBRec* pb);

		if (!FirstMemCacheValid())
			{
				AlertError(NotEnoughMemoryID,NIL);
				return;
			}
		ConstructFileTypeTable(TypeList,&NumTypes,&FileFilter);
		if (FGetFile(&FileInfo,FileFilter,TypeList,NumTypes))
			{
				DispatchOpenDocument(&FileInfo);
			}
	}


void		CApplication::ConstructFileTypeTable(OSType Table[NUMFILETYPES], short* NumTypes,
					pascal Boolean (**FileFilter)(CInfoPBRec* pb))
	{
		OSType		LocalList[NUMFILETYPES] = FILETYPELIST;

		MemCpy((char*)Table,(char*)LocalList,NUMFILETYPES * sizeof(OSType));
		*NumTypes = NUMFILETYPES;
		*FileFilter = NIL;
	}
