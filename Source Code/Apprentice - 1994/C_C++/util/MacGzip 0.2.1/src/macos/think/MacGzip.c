/*
 * Copyright (C) 1993  SPDsoft
 * 
 */

#include <GestaltEqu.h>
#include <Sound.h>
/*#include <EPPC.h>*/

#include "MacGzip.h"
#include "thePrefs.h"
#include "MacErrors.h"
#include "SPDProg.h"

#include <string.h>

void	about(void);
void	InitTheMac(void);
int		OpenFromFinder( void );
void	MyBeep(void);

void CenterPicRect(PicHandle thePic, Rect *inRect, Rect *outRect);


static	pascal char
		MyFilter ( DialogPtr theDialog, EventRecord *theEvent,short  *itemHit );

extern OSErr
		DoOpen( Str255 fName, long vRefNum);

/* CONST */
#define NIL 0L

static SFReply	reply;
SFTypeList		typeList =	{'Gzip','ZIVU','pZIP','ZIVM'};
static Point	myPoint =	{80,80};

Boolean			quitting = false;

MenuHandle		myMenus[4];
OSErr			theOSErr;
KeyMap			theKeys;


void InitTheMac(void)
{
long gestAliasResponse, gestStdFileResponse, gestFSResponse;

	FlushEvents(everyEvent, 0);
	InitGraf(&qd.thePort);
	InitFonts();
	InitWindows();
	InitMenus();
	TEInit();
	InitDialogs(NIL);
	InitCursor();
	MaxApplZone();

	GetKeys(&theKeys);

/* sys 7 ?*/

	theOSErr = Gestalt(gestaltAliasMgrAttr, &gestAliasResponse);
	if (theOSErr == noErr) theOSErr = Gestalt(gestaltFSAttr, &gestFSResponse);
	if (theOSErr == noErr)
		theOSErr = Gestalt(gestaltStandardFileAttr, &gestStdFileResponse);
	
	if (theOSErr != noErr ||
		(gestAliasResponse & (1 << gestaltAliasMgrPresent)) == 0 ||
		(gestFSResponse & (1 << gestaltHasFSSpecCalls)) == 0 ||
		(gestStdFileResponse & (1 << gestaltStandardFile58)) == 0) {
	
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

}

main()

{ /* main program */
	extern	Boolean		gSavedPos;
	extern	Point		gSavedPoint;

	InitTheMac();
	SetUpMenus();
	
	if ( 0!=OpenFromFinder() )
	{
		quitting=true;
	}
	
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
	
	
	ExitToShell();
}


void MainEvent(void)
{
	EventRecord myEvent;
	WindowPtr whichWindow;
	short windowPart;

	SystemTask();
	if( GetNextEvent(everyEvent, &myEvent) ) {
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
				
/*			case kHighLevelEvent:*/
/*				break;*/


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
				CGrafPtr savePort;
				GetItem(myMenus[appleM], theItem, &name);
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
			DoFile(theItem);
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

void DoFile(short item)
{
	switch( item ) {
		case fmOpen:
		
			SFGetFile(
						myPoint,
						"\p",
						0L,
						-1,
/*						currPrefs.compress ? -1:4,*/
						typeList,
						0L,
						&reply
					);
			
			if( reply.good != false )
			{
				if( DoOpen( reply.fName, reply.vRefNum )!=128)
				{
					if(currPrefs.BeepWhenDone) MyBeep();
					if(currPrefs.QuitWhenDone) quitting=true;
				}
			}
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

int OpenFromFinder( void )
{
	int	message, count, index;
	AppFile		theFile;
	Boolean PrefCSave,PrefASave, HasToBeep=false;
	
	PrefCSave =	currPrefs.compress;
	currPrefs.compress = (( IsOptKey(theKeys) ) ? true : false);
	PrefASave =	currPrefs.ascii;
	if (IsAKey(theKeys)) currPrefs.ascii = true;
	else if (IsBKey(theKeys)) currPrefs.ascii = false;
	
	CountAppFiles( &message, &count);			/*any files to open?*/
	for( index= 1 ; index <= count ; index ++)
	{
		GetAppFiles(index, &theFile);
		
		if( DoOpen(theFile.fName, theFile.vRefNum)!=128)
		{
			HasToBeep=currPrefs.BeepWhenDone;
		}
	}
	
	if(HasToBeep) MyBeep();

	ClrAppFiles(count);

	currPrefs.compress=PrefCSave;
	currPrefs.ascii = PrefASave;

	return count;
}

/***************************************************************************/
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

	GetSelection = GetNewDialog(dlogAbout, NIL, (WindowPtr)-1);
	ShowWindow(GetSelection);   		
	SelectWindow(GetSelection); 		
	GetPort(&savePort);
	SetPort(GetSelection);


	GetDItem(GetSelection, 1, &DType, &DItem, &picRect);
	thePict = (PicHandle) Get1Resource('PICT',ABOUT);
	CenterPicRect(thePict, &picRect, &GetSelection->portRect);
	DrawPicture(thePict,&picRect);

	ModalDialog(MyFilter, &itemHit);
	EraseRect (&GetSelection->portRect);
	ReleaseResource(thePict);
	
	thePict = (PicHandle) Get1Resource('PICT',GPL);
	CenterPicRect(thePict, &picRect, &GetSelection->portRect);
	DrawPicture(thePict,&picRect);
	
	ModalDialog(MyFilter, &itemHit);
	ReleaseResource(thePict);

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
				SndPlay(nil, theSound, false);
				ReleaseResource(theSound);
			}
}