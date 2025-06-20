/*
 * Copyright (C) 1993  SPDsoft
 * 
 */

#include <AppleEvents.h>
#include <GestaltEqu.h>

#include <Sound.h>

#ifdef MPW
/* 22may95,ejo: allocate space for qd globals */
QDGlobals qd;
#endif

#include <string.h>

/* 22may95,ejo: added these */
#ifdef MPW
#define FALSE false
#endif
#include <Events.h>
#include <Dialogs.h>
#include <Fonts.h>
#include <SegLoad.h>
#include <DiskInit.h>
#include <ToolUtils.h>
#include <Devices.h>
#include <StandardFile.h>
#include <Resources.h>

#include "MacGzip.h"
#include "ThePrefs.h"
#include "MacErrors.h"
#include "SPDProg.h"
#include "MacAE.h"

void	InitTheMac(void);
void	main(void);
void	MainEvent(void);
void	DoCommand(long mResult);
void	DoMouseDown(short, WindowPtr, EventRecord *);
OSErr	DoOpen( FSSpec *myFSSpec );
long	RecurseDir(long dirIDToSearch);

void	CenterPicRect(PicHandle thePic, Rect *inRect, Rect *outRect);
void	about(void);
static	pascal char MyFilter ( DialogPtr, EventRecord*, short* );
void	SetUpMenus(void);
void	MyBeep(void);

extern	OSErr OpenFile( Str255 fName);


/* Globals */

Boolean			quitting = false;

MenuHandle		myMenus[4];
OSErr			theOSErr;
KeyMap			theKeys;
char			charKey;
Boolean			modKey;
HFileInfo		gpb;

/* ejo: this was added for PPC */
AEEventHandlerUPP gMyHandleODoc;
AEEventHandlerUPP gMyHandleOApp;
AEEventHandlerUPP gMyHandlePDoc;
AEEventHandlerUPP gMyHandleQuit;

/***************************************************************************/

void InitTheMac(void)
{
	long	gestAliasResponse,
			gestStdFileResponse,
			gestFSResponse,
			gestAEResponse;

	FlushEvents(everyEvent, 0);
	InitGraf(&qd.thePort);
	InitFonts();
	InitWindows();
	InitMenus();
	TEInit();
#ifdef THINK_C
/* ejo: ??? */
	InitDialogs(0);
#else
	InitDialogs(nil);
#endif
	InitCursor();
	MaxApplZone();
	
/* sys 7 ?*/

	theOSErr = Gestalt(gestaltAppleEventsAttr, &gestAEResponse);
	theOSErr = Gestalt(gestaltAliasMgrAttr, &gestAliasResponse);
	if (theOSErr == noErr) theOSErr = Gestalt(gestaltFSAttr, &gestFSResponse);
	if (theOSErr == noErr)
		theOSErr = Gestalt(gestaltStandardFileAttr, &gestStdFileResponse);
	
	if (theOSErr != noErr ||
		(gestAliasResponse & (1 << gestaltAliasMgrPresent)) == 0 ||
		(gestFSResponse & (1 << gestaltHasFSSpecCalls)) == 0 ||
		(gestStdFileResponse & (1 << gestaltStandardFile58)) == 0 ||
		((gestAEResponse >> gestaltAppleEventsPresent) & 0x0001) == 0
		)
	{
	
		theAlert( OLD_SYSTEM,GENERIC, theOSErr, true );
	}
	
	CanBreak=true;

	if (!GetOrGeneratePrefs(&currPrefs, kCurrentPrefsTypeVers))
	{
		about();
		prefsChangedFlag = DoPrefsDialog(&currPrefs);
		if (prefsChangedFlag)
		{
			if( ( theOSErr = SavePrefs(&currPrefs)) != noErr )
					theAlert(NO_SAVE_PREF,GENERIC,theOSErr, true );
		}
		else ExitToShell();
	}

	/* ejo: this was added for PPC */
#if 1
	gMyHandleODoc = NewAEEventHandlerProc (MyHandleODoc);
	gMyHandleOApp = NewAEEventHandlerProc (MyHandleOApp);
	gMyHandlePDoc = NewAEEventHandlerProc (MyHandlePDoc);
	gMyHandleQuit = NewAEEventHandlerProc (MyHandleQuit);
	AEInstallEventHandler(kCoreEventClass, kAEOpenDocuments, gMyHandleODoc,0, FALSE);
	AEInstallEventHandler(kCoreEventClass, kAEOpenApplication, gMyHandleOApp,0, FALSE);
	AEInstallEventHandler(kCoreEventClass, kAEPrintDocuments, gMyHandlePDoc,0, FALSE);
	AEInstallEventHandler(kCoreEventClass, kAEQuitApplication, gMyHandleQuit,0, FALSE);
#else
	/* ejo: this is how it was */
	AEInstallEventHandler(kCoreEventClass, kAEOpenDocuments,
													MyHandleODoc,0, FALSE);
	AEInstallEventHandler(kCoreEventClass, kAEOpenApplication,
													MyHandleOApp,0, FALSE);
	AEInstallEventHandler(kCoreEventClass, kAEPrintDocuments,
													MyHandlePDoc,0, FALSE);
	AEInstallEventHandler(kCoreEventClass, kAEQuitApplication,
													MyHandleQuit,0, FALSE);
#endif
}

void main( void )
{
	extern	Boolean		gSavedPos;
	extern	Point		gSavedPoint;
	
	extern Handle	FPrefs;
	extern Size		FPrefsSize;


	InitTheMac();
	SetUpMenus();
		
	while( !quitting )
	{
		MainEvent();
	}

/* remember window pos */
			
	currPrefs.SavedPos = gSavedPos; /* always true */
		
	if(( gSavedPoint.h != currPrefs.SavedPoint.h ) ||
		( gSavedPoint.v != currPrefs.SavedPoint.v ) )
	{
		currPrefs.SavedPoint.h = gSavedPoint.h;
		currPrefs.SavedPoint.v = gSavedPoint.v;
		prefsChangedFlag=true;
	}

	if (prefsChangedFlag)
	{
		if( ( theOSErr = SavePrefs(&currPrefs)) != noErr )
				theAlert(NO_SAVE_PREF,GENERIC,theOSErr, true );
	}
	
	if( FPrefsSize != 0 ) DisposeHandle( FPrefs );
	if( ICmappings != nil ) DisposeHandle( ICmappings );
	
	ExitToShell();
}


void MainEvent(void)
{
	EventRecord myEvent;
	WindowPtr whichWindow;
	short windowPart;

	if ( WaitNextEvent ( everyEvent, &myEvent, 0, nil ) ) {
		switch( myEvent.what ) {
			case mouseDown:
				windowPart = FindWindow(myEvent.where, &whichWindow);
				DoMouseDown(windowPart, whichWindow, &myEvent);
				break;
				
			case keyDown:
			case autoKey: 
			{
				register char theChar;
	
				theChar = myEvent.message & charCodeMask;
				if ((myEvent.modifiers & cmdKey) != 0) 
					DoCommand( MenuKey(theChar) );
									
				break;
			}
				
			case activateEvt:
				break;

			case updateEvt: 
				break;
				
			case diskEvt:
			if ((myEvent.message >> 16) != noErr)
			{
				Point myPoint={80,80};
				
				theOSErr = DIBadMount(myPoint, myEvent.message);
				theAlert(BAD_DISK,GENERIC,theOSErr,false);
			}
			break;

			case osEvt:
			switch ((myEvent.message >> 24) & 0x0ff)
			{
				case suspendResumeMessage:
					
					SetCursor(&qd.arrow);
				
					if ((myEvent.message & resumeFlag) == 0) // suspend 
					{
					}
					else
					{										// resume
					}
					break;
					
				case mouseMovedMessage:
					break;
			}
			break;
				
			case kHighLevelEvent:
				AEProcessAppleEvent (&myEvent);
				break;


		}
	}
}
void DoCommand(long mResult)
{
	short	theItem;
	Str255	name;

	theItem = LoWord(mResult);
	switch( HiWord(mResult) ) {
		case appleID:
			if( theItem > 2 ) {
				GrafPtr savePort;
				GetItem(myMenus[appleM], theItem, name);
				GetPort(&savePort);
				OpenDeskAcc(name);
				SetPort(savePort);
			}
			else
			{
				about();
			}
			break;

		case fileID:
		
			switch( theItem )
			{
				case fmOpen:
				
					if ((GzipOK==DoOpen( (FSSpec *)nil )) &&
						(currPrefs.BeepWhenDone))
					MyBeep();


					break;
		
				case fmPrefs:
				
					prefsChangedFlag = DoPrefsDialog(&currPrefs);
					if (prefsChangedFlag)
					{
						if( ( theOSErr = SavePrefs(&currPrefs)) != noErr )
								theAlert(NO_SAVE_PREF,GENERIC,theOSErr, true );
					}
					break;
		
				case fmQuit:
				
					quitting = true;
					break;
			}
			break;

		case editID:
		
			SystemEdit(theItem-1);
			break;
			
		case gzipID:
		
			switch(theItem){
			
			case gmAscii:
			
				currPrefs.ascii=true;
				CheckItem( myMenus[gzipM], gmBin, false );
				CheckItem( myMenus[gzipM], gmAscii, true);
				break;

			case gmBin:
			
				currPrefs.ascii=false;
				CheckItem( myMenus[gzipM], gmAscii , false );
				CheckItem( myMenus[gzipM], gmBin, true);
				break;
			
			case gmComp:
			
				currPrefs.compress=true;
				CheckItem( myMenus[gzipM], gmUncomp, false );
				CheckItem( myMenus[gzipM], gmComp, true);
				break;

			case gmUncomp:
			
				currPrefs.compress=false;
				CheckItem( myMenus[gzipM], gmComp , false );
				CheckItem( myMenus[gzipM], gmUncomp, true);
				break;
			
			}
			prefsChangedFlag = true;
			break;
	}

	HiliteMenu(0);
}

void DoMouseDown(short windowPart, WindowPtr whichWindow, EventRecord *myEvent)
{
	switch( windowPart ) {
		case inGoAway:
			break;

		case inMenuBar:
			DoCommand( MenuSelect(myEvent->where) );
			break;

		case inSysWindow:
			SystemClick(myEvent, whichWindow);
			break;

		case inDrag:
			break;

		case inGrow:
			break;

		case inContent:
			if( whichWindow != FrontWindow() )
				SelectWindow(whichWindow);
			break;
	}
}

/***************************************************************************
 *
 * We use OpenWD and SetVol in order to use fopen on stdio programs
 * DoOpen(nil) prompts for a file
 * RecurseDir() is recursive (what happends to the stack?)
 */


OSErr DoOpen( FSSpec *myFSSpec )
{
	StandardFileReply	mySFR;
	SFTypeList			typeList={GZIP_ID,'ZIVU','pZIP','ZIVM'};
	HFileInfo			pb;
	OSErr				error,
						result;
	short				myVRefNum,
						SaveVRefNum;
	
	if ( myFSSpec == nil )
	{
		StandardGetFile( nil, -1, nil, &mySFR );
/*		StandardGetFile( nil,  4, typeList, &mySFR );*/

		if( !mySFR.sfGood ) return 1;
		
		myFSSpec = &(mySFR.sfFile);
		
		charKey = 0x00;
		modKey = false;
	}
	else
	{
		modKey		=	(( IsOptKey(theKeys) ) ? true : false);
		
		if (IsAKey(theKeys))
									charKey = 'a';
		else if (IsBKey(theKeys))
									charKey = 'b';
		else
									charKey = 0x00;
	}

	pb.ioNamePtr	= myFSSpec->name;
	pb.ioVRefNum	= myFSSpec->vRefNum;
	pb.ioFDirIndex	= 0;					/* query 1 item */
	pb.ioDirID		= myFSSpec->parID;
	
	error = PBGetCatInfo( (CInfoPBPtr)&pb, 0 );
	if (((pb.ioFlAttrib>>4) & 0x01) == 1)
	{
		/* Is a Dir */
		gpb.ioNamePtr = myFSSpec->name;
		gpb.ioVRefNum = myFSSpec->vRefNum;

		if ( RecurseDir(pb.ioDirID) > 0 )
			result = GzipOK;
	}
	else
	{
		/* Is a File */
		OpenWD(myFSSpec->vRefNum,myFSSpec->parID,GZIP_ID, &myVRefNum );
		error=GetVol( nil, &SaveVRefNum );		
		error=SetVol( nil, myVRefNum);
		
			result = OpenFile( myFSSpec->name );
	
		error=SetVol( nil, SaveVRefNum );
		CloseWD(myVRefNum );
	}
	
	if	(
		( (StartupFiles) || ( result == GzipOK ) ) &&
		( currPrefs.QuitWhenDone )
		)
		quitting=true;
		
	
	return result;
}

long RecurseDir(long dirIDToSearch)
{
    short int	index=1;      /* for ioFDirIndex */
    OSErr		err;
	short		myVRefNum,
				SaveVRefNum;
	long		NumOfFiles;
	
						
	OpenWD(gpb.ioVRefNum,dirIDToSearch,GZIP_ID, &myVRefNum );
	err=GetVol( nil, &SaveVRefNum );		
	err=SetVol( nil, myVRefNum);
	NumOfFiles=0;
    do
    {
         gpb.ioDirID		= dirIDToSearch;     
         gpb.ioFDirIndex	= index;		/* set up the index */
											/* we need to do this every time through,
											 * since GetCatInfo returns ioFlNum
											 * in this field
											 */

		err= PBGetCatInfo((CInfoPBPtr)&gpb,false);
		if (err == noErr) 
		{
			/* check the file attributes for folderhood */
			if (((gpb.ioFlAttrib>>4) & 0x01) == 1)
 			{
				NumOfFiles += RecurseDir(gpb.ioDirID);
				err = 0;
			}
			else 
			{
				if (GzipOK==OpenFile( gpb.ioNamePtr ))
					NumOfFiles++ ;
            }
        	index += 1;	/* increment the index for GetCatInfo */
        }
    } while (err == noErr);
    
	err=SetVol( nil, SaveVRefNum );
	CloseWD(myVRefNum );
	
	return(NumOfFiles);
}


/***************************************************************************
 *
 * About, Beep and other things
 */
 
void CenterPicRect(PicHandle thePic, Rect *inRect, Rect *outRect)
{
	short		dh,dv;
	
	*inRect = (*thePic)->picFrame;
	dh = (outRect->right - (inRect->right - inRect->left)) / 2;
	dv = (outRect->bottom - (inRect->bottom - inRect->top)) / 2;
	OffsetRect(inRect,dh,dv);
}

void   about( void )
{
	DialogPtr	GetSelection;  			
	short		itemHit;   					
	short		DType; 
	Handle		DItem;
	GrafPtr		savePort;

	Rect		picRect;
	PicHandle	thePict;
	ModalFilterUPP modalUPP;	/* 21may95,ejo: added this for PPC */

	GetSelection = GetNewDialog(dlogAbout, nil, (WindowPtr)-1);
	ShowWindow(GetSelection);   		
	SelectWindow(GetSelection); 		
	GetPort(&savePort);
	SetPort(GetSelection);


	GetDItem(GetSelection, 1, &DType, &DItem, &picRect);
	thePict = (PicHandle) Get1Resource('PICT',ABOUT);
	CenterPicRect(thePict, &picRect, &GetSelection->portRect);
	DrawPicture(thePict,&picRect);

#if 1
	/* 21may95,ejo: replaced things below with this for PPC */
	ModalDialog (modalUPP = NewModalFilterProc (MyFilter), &itemHit);
#else
/*	WARNING (?)
 *	Should be
 *	ModalDialog(NewModalFilterProc((ProcPtr)MyFilter), &itemHit);
 *	for PPC ?
 */
#if defined(__MWERKS__)
	ModalDialog(NewModalFilterProc((ProcPtr)MyFilter), &itemHit);
#else
	ModalDialog( (ProcPtr) MyFilter, &itemHit);
#endif
#endif
 
	EraseRect (&GetSelection->portRect);
	ReleaseResource((Handle)thePict);
	
	thePict = (PicHandle) Get1Resource('PICT',GPL);
	CenterPicRect(thePict, &picRect, &GetSelection->portRect);
	DrawPicture(thePict,&picRect);

#if 1
	/* 21may95,ejo: replace things below with this for PPC */
	ModalDialog (modalUPP, &itemHit);
	DisposeRoutineDescriptor (modalUPP);
#else
#	
#if defined(__MWERKS__)
	ModalDialog(NewModalFilterProc((ProcPtr)MyFilter), &itemHit);
#else
	ModalDialog( (ProcPtr) MyFilter, &itemHit);
#endif
#endif
	ReleaseResource((Handle)thePict);

	DisposDialog(GetSelection);  
	SetPort(savePort);
	
}   									
static pascal char
			MyFilter (
						DialogPtr		theDialog,  	
						EventRecord   	*theEvent,    
						short			*itemHit
			)
{
	return ((theEvent->what == keyDown)||(theEvent->what == mouseDown));
}
/***************************************************************************/
void SetUpMenus(void)
{
	short index;
	
	ClearMenuBar();
	
	myMenus[appleM] = GetMenu(appleID);
	AddResMenu(myMenus[appleM], 'DRVR');
	myMenus[fileM] = GetMenu(fileID);
	myMenus[editM] = GetMenu(editID);
	myMenus[gzipM] = GetMenu(gzipID);
	for(index=appleM; index <= gzipM; index++)
		InsertMenu(myMenus[index], 0) ;

	CheckItem( myMenus[gzipM], (currPrefs.ascii?gmAscii:gmBin), true);
	CheckItem( myMenus[gzipM], (currPrefs.compress?gmComp:gmUncomp), true);
	
	DrawMenuBar();
}
/***************************************************************************/
void MyBeep(void)
{
	Handle theSound;
	
	theSound = GetResource('snd ', SOUND_ID);
	if (theSound != nil)
	{
		SndPlay(nil, (SndListHandle) theSound, false);
		ReleaseResource(theSound);
	}
}