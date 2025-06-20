/*
 * MacDP2.c
 *
 * (c)1990-1994 SPDsoft <MacSPD@ivo.cps.unizar.es>
 * Part of a batch VTR Frame-By-Frame recorder
 * Version 1.1.1
 *
 * Grupo de Informatica Grafica, CPSUZ
 *
 */

#include <QuickTimeComponents.h>
#include "MacDP.h"

/*
 * Prototypes
 */
 
void InitTheMac(void);
void FlushTheMac(void);
void CatString( Str255 StrIO, Str255 Str );

extern PicHandle
ConvertFromJFIF(StringPtr name,short originalFile, CWindowPtr window);
extern PicHandle
ConvertFromPict(StringPtr name,short originalFile,CWindowPtr window);

/*
 * Constants
 */
 
#define keyOrMouse mDownMask+keyDownMask
#define NIL 0L

#define CopyString(a,b)		BlockMove(a, b, *(a) + 1 )


/*
 * Globals
 */
 
TPrint		myTPrint;
TPPrint		myTPPrint = &myTPrint;
THPrint		ghPtrRec = &myTPPrint;

WindowPtr 	gMyWindow = nil;
Str63		gTheName = "\p File";
Rect		PackORects[7];
char		LastMonitorID=0;
int			theMonitorID;
Boolean		CanOpenJFIF = false;

static short	oldMBarHeight;
static int		oldTop;

Boolean		quitting = false;
#define		update				( gMyWindow != nil )
Boolean		HiddenMenu=false;
Boolean		BlackBack=true;

MenuHandle			myMenus[4];
static PicHandle	thePic=nil;
HFileInfo	gpb;

/*
 * AE
 */
Boolean AppleEventsInstalled (void)
{
	OSErr err;
	long  result;

	err = Gestalt (gestaltAppleEventsAttr, &result);
	return (!err && ((result >> gestaltAppleEventsPresent) & 0x0001));
}

OSErr MyGotRequiredParams (AppleEvent *theAppleEvent)
{
	DescType	returnedType;
	Size	actualSize;
	OSErr	err;

	err = AEGetAttributePtr (theAppleEvent, keyMissedKeywordAttr,
									typeWildCard, &returnedType, nil, 0,
									&actualSize);
	if (err == errAEDescNotFound)	// you got all the required parameters
			return noErr;
	else if (!err)				// you missed a required parameter
			return errAEEventNotHandled;
	else						// the call to AEGetAttributePtr failed
			return err;
}
/******************************************************************************/


pascal OSErr  MyHandleODoc (AppleEvent *theAppleEvent, AppleEvent* reply, long
														handlerRefCon)
{
	FSSpec	myFSS;
	AEDescList	docList;
	OSErr	err;
	long	index,
			itemsInList;
	Size	actualSize;
	AEKeyword	keywd;
	DescType	returnedType;


	// get the direct parameter--a descriptor list--and put it into a docList
	err = AEGetParamDesc (theAppleEvent, keyDirectObject, typeAEList,
			&docList);
	if (err)
			return err;

	
	// check for missing parameters
	err = MyGotRequiredParams (theAppleEvent);
	if (err)
			return err;

	
	// count the number of descriptor records in the list
	err = AECountItems (&docList, &itemsInList);

	// now get each descriptor record from the list, coerce the returned
	// data to an FSSpec record, and open the associated file

	for (index = 1; index <= itemsInList; index++) {

			err = AEGetNthPtr (&docList, index, typeFSS, &keywd,
					&returnedType, (Ptr) &myFSS, sizeof(myFSS), &actualSize);
			if (err)
					return err;

			
	}
	DoOpen(&myFSS);
	
	err = AEDisposeDesc (&docList);
	return noErr;
}

pascal OSErr  MyHandlePDoc (AppleEvent *theAppleEvent, AppleEvent *reply, long
														handlerRefCon)
{
	FSSpec	myFSS;
	AEDescList	docList;
	OSErr	err;
	long	index,
			itemsInList;
	Size	actualSize;
	AEKeyword	keywd;
	DescType	returnedType;


	// get the direct parameter--a descriptor list--and put it into a docList
	err = AEGetParamDesc (theAppleEvent, keyDirectObject, typeAEList,
			&docList);
	if (err)
			return err;

	// check for missing parameters
	err = MyGotRequiredParams (theAppleEvent);
	if (err)
			return err;

	// count the number of descriptor records in the list
	err = AECountItems (&docList, &itemsInList);

	// now get each descriptor record from the list, coerce the returned
	// data to an FSSpec record, and open the associated file



	for (index = 1; index <= itemsInList; index++) {

			err = AEGetNthPtr (&docList, index, typeFSS, &keywd,
					&returnedType, (Ptr) &myFSS, sizeof(myFSS), &actualSize);
			if (err)
					return err;
			
			DoOpen(&myFSS);
			UpdateWindow(gMyWindow);
			DoFile(fmPrint);
			
	}
	
	err = AEDisposeDesc (&docList);
	return noErr;
}

pascal OSErr  MyHandleOApp (AppleEvent *theAppleEvent, AppleEvent *reply, long
														handlerRefCon)
{
	OSErr	err;

	if ( err = MyGotRequiredParams (theAppleEvent))
			return err;
			
	return ( noErr );
}

pascal	OSErr	MyHandleQuit (AppleEvent *theAppleEvent, AppleEvent *reply,
					long handlerRefcon)
{
	OSErr	err;
	extern Boolean quitting;
	
	if (err = MyGotRequiredParams(theAppleEvent)) {
		// an error occurred:  do the necessary error handling
		return	err;
	}
	
	
	DoFile(fmQuit);
	
	return ( noErr );
}
/******************************************************************************/


void InitTheMac(void)
{
	SysEnvRec	theWorld;
	GDHandle	GDHdl;
	long		resp;

	MaxApplZone();
	FlushEvents(everyEvent, 0);
	InitGraf(&qd.thePort);
	InitFonts();
	InitWindows();
	InitMenus();
	TEInit();
	InitDialogs(NIL);
	InitCursor();

	oldMBarHeight=LMGetMBarHeight();
	
	if (SysEnvirons(curSysEnvVers, &theWorld) != noErr)
	{
		ParamText("\pSorry, SysEnvirons failed","\p(System too old?)","\p","\p");
		StopAlert(WarningDlg ,NULL);
		ExitToShell();
	}
	
	if(theWorld.hasColorQD)
	{
		GDHdl = GetDeviceList();
		
		do
		{
			BlockMove(&(**GDHdl).gdRect, &PackORects[LastMonitorID], sizeof(Rect));
			LastMonitorID++;
		}while((GDHdl = GetNextDevice(GDHdl))!=NIL);
	}
	else
	{
		ParamText("\pSorry, Color QuickDraw not found","\p(System too old?)","\p","\p");
		StopAlert(WarningDlg ,NULL);
		ExitToShell();

	}
	if (AppleEventsInstalled())
	{
		AEInstallEventHandler (kCoreEventClass, kAEOpenDocuments,
													(AEEventHandlerUPP)MyHandleODoc,0, FALSE);
		AEInstallEventHandler (kCoreEventClass, kAEOpenApplication,
													(AEEventHandlerUPP)MyHandleOApp,0, FALSE);
		AEInstallEventHandler (kCoreEventClass, kAEPrintDocuments,
													(AEEventHandlerUPP)MyHandlePDoc,0, FALSE);
		AEInstallEventHandler (kCoreEventClass, kAEQuitApplication,
													(AEEventHandlerUPP)MyHandleQuit,0, FALSE);
	}
	else
	{
		ParamText("\pSorry, AppleEvents not found","\p(System too old?)","\p","\p");
		StopAlert(WarningDlg ,NULL);
		ExitToShell();
	}
	
	theMonitorID=LastMonitorID;
	thePic=nil;
	CanOpenJFIF =  !( Gestalt(gestaltCompressionMgr, &resp) != noErr || resp < 15 );

	PrOpen();
		PrintDefault(ghPtrRec);
	PrClose();
	
#ifdef SHRINKWINDOWFORDEBUG
	PackORects[0].top+= 100;
	PackORects[0].bottom-= 100;
#endif	
}

void FlushTheMac(void)
{

	LMSetMBarHeight(oldMBarHeight);
}

void main( void )

{ /* main program */
	InitTheMac();
	SetUpMenus();
	
	while( !quitting )
		MainEvent();

	FlushTheMac();
}


void MainEvent(void)
{
	EventRecord	myEvent;
	WindowPtr	whichWindow;
	short		windowPart;
	Rect		tmpRect;

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
				else
					if ((theChar=='\t')&&(update))
					{
						if (HiddenMenu)
						{
							(**(*gMyWindow).visRgn).rgnBBox.top=oldTop;
							
							SetRect(&tmpRect,
								qd.thePort->portRect.left,
								qd.thePort->portRect.top,
								qd.thePort->portRect.right,
								qd.thePort->portRect.top + oldMBarHeight
							);
							
							InvalRect( &tmpRect );

							HiddenMenu=false;
							LMSetMBarHeight(oldMBarHeight);
							ShowCursor();
							DrawMenuBar();

						}
						else
						{
							oldTop=(**(*gMyWindow).visRgn).rgnBBox.top;
							RectRgn( (*gMyWindow).visRgn, &(*qd.thePort).portRect );
							
							
							SetRect(&tmpRect,
								qd.thePort->portRect.left,
								qd.thePort->portRect.top,
								qd.thePort->portRect.right,
								qd.thePort->portRect.top + oldMBarHeight
							);
							
							InvalRect( &tmpRect );
							
							LMSetMBarHeight(0);
							HiddenMenu=true;
							HideCursor();

						}
					}
					else if ((theChar==' ')&&(update))
					{
						if ( BlackBack=!BlackBack )
								BackPat(&qd.black);
						else
								BackPat(&qd.white);

						InvalRect( &(*qd.thePort).portRect );
					}
					
				break;
			}
			case osEvt:
				switch ((myEvent.message >> 24) & 0x0ff)
				{
					case suspendResumeMessage:
						
						SetCursor(&qd.arrow);
					
						if ((myEvent.message & resumeFlag) == 0) // suspend 
						{
							if (HiddenMenu)
							{
								(**(*gMyWindow).visRgn).rgnBBox.top=oldTop;
								LMSetMBarHeight(oldMBarHeight);
								ShowCursor();
								DrawMenuBar();
	
							}
						}
						else
						{										// resume
							if (HiddenMenu)
							{
								oldTop=(**(*gMyWindow).visRgn).rgnBBox.top;
								RectRgn( (*gMyWindow).visRgn, &(*qd.thePort).portRect );
								InvalRect( &(*qd.thePort).portRect );
								LMSetMBarHeight(0);
								HideCursor();
							}
						}
						break;
						
					case mouseMovedMessage:
						break;
				}
				break;
			case activateEvt:
				break;

			case updateEvt: 
				UpdateWindow((WindowPtr)myEvent.message);
				break;

			case kHighLevelEvent:
				AEProcessAppleEvent (&myEvent);
				break;


		}
	}
}
void UpdateWindow(WindowPtr theWindow)
{
	GrafPtr savePort;
	Rect	destRect;	
	short	pw, ph, ww, wh, dw, dh;
	
	GetPort(&savePort);
	SetPort(theWindow);

	BeginUpdate(theWindow);
	DrawControls(theWindow);

	EraseRect(&theWindow->portRect);

	if( thePic != nil )
	{
		BlockMove(&(*thePic)->picFrame, &destRect, sizeof(Rect));
		ww = theWindow->portRect.right - theWindow->portRect.left;
		wh = theWindow->portRect.bottom - theWindow->portRect.top;
		pw = destRect.right - destRect.left;
		ph = destRect.bottom - destRect.top;
		
		dw = (ww - pw)/2;
		dh = (wh - ph)/2;
				
		OffsetRect( &destRect, dw, dh );

		if (( dw < dh ) && ( dw < 0 ))
			InsetRect( &destRect, -dw, -dw * (float) ph / (float) pw );
		else if (( dh < dw ) && ( dh < 0 ))
			InsetRect( &destRect, -dh * (float) pw / (float) ph, -dh );
			
		DrawPicture( thePic, &destRect);
	}
	
	EndUpdate(theWindow);

	SetPort(savePort);
}

void SetUpMenus(void)
{
	short index;
	Str255 stmp, snum;

	myMenus[appleM] = GetMenu(appleID);
	AddResMenu(myMenus[appleM], 'DRVR');
	myMenus[fileM] = GetMenu(fileID);
	myMenus[editM] = GetMenu(editID);
	myMenus[monitorM] = GetMenu(monitorID);

	
	for( index=(short)LastMonitorID-1;index>=0;index--)
	{
		*stmp=0x00;
		CatString(stmp,"\pSlot ");
		NumToString((long)(LastMonitorID-index),snum);
		CatString(stmp,snum);
		CatString(stmp,"\p/");
		CatString(stmp,snum);
		AppendMenu(myMenus[monitorM],stmp);
	}
	

	for(index=appleM; index <= monitorM; index++)
		InsertMenu(myMenus[index], 0) ;
	CheckItem( myMenus[monitorM], theMonitorID, true);
	
	DisableItem(myMenus[editM],emUndo);
	DisableItem(myMenus[editM],emClear);
	DisableItem(myMenus[editM],emCut);
	DisableItem(myMenus[editM],emPaste);
	
	DisableItem(myMenus[editM],emCopy);
	
	DrawMenuBar();
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
				ParamText("\p�SPDsoft 94: MacDP 1.1.1",
				"\p<Tab>=Full Screen; <Spc>=B/W",
				"\pe-mail: MacSPD@ivo.cps.unizar.es","\p");
				NoteAlert(AboutDlg,NULL);
			}

			break;

		case fileID:
			DoFile(theItem);
			break;

		case editID:
		
			if( SystemEdit(theItem-1) == FALSE )
			{
				if (( theItem == emCopy ) && (thePic != nil ))
				{
					ZeroScrap();
					HLock((Handle)thePic);
					PutScrap(
						(long) GetHandleSize((Handle)thePic),
						(ResType)'PICT',
						(Ptr) *thePic
					);
					HUnlock((Handle)thePic);
				}
			}
			break;
			
		case monitorID:
		
			CheckItem( myMenus[monitorM], theMonitorID, false);
			theMonitorID=( (theItem>LastMonitorID)?LastMonitorID:theItem);
			CheckItem( myMenus[monitorM], theMonitorID, true);
			
			
			if(gMyWindow!=nil)
			{
			MoveWindow(gMyWindow,
				PackORects[theMonitorID-1].left,
				PackORects[theMonitorID-1].top,
				TRUE);
			SizeWindow(gMyWindow,
				PackORects[theMonitorID-1].right - PackORects[theMonitorID-1].left,
				PackORects[theMonitorID-1].bottom - PackORects[theMonitorID-1].top,
				TRUE);
			InvalRect( &(*qd.thePort).portRect );
			}
/*			
			if (HiddenMenu)
			{
				MBarHeight=oldMBarHeight;
				DrawMenuBar();
				(**(*gMyWindow).visRgn).rgnBBox.top=oldTop;
				InvalRect( &(*qd.thePort).portRect );

				HiddenMenu=false;
				MBarHeight=oldMBarHeight;
				ShowCursor();
				DrawMenuBar();
				
				oldTop=(**(*gMyWindow).visRgn).rgnBBox.top;
				RectRgn( (*gMyWindow).visRgn, &(*qd.thePort).portRect );
				InvalRect( &(*qd.thePort).portRect );
				MBarHeight=0;
				HiddenMenu=true;
				HideCursor();
			}
*/				
			break;
	}

	HiliteMenu(0);
}

void DoFile(short item)
{
	OSErr				result;
	short compressedFile = 0;
	SFReply		sfr;	
	Point		pt = {100,100};
	char		zero=0;
	long		l;
	short		i;
	
	switch( item ) {

		case fmOpen:
		
			DoOpen((FSSpec *)nil);
			break;

		case fmClose:

			if (update) DoClose();
			DisableItem(myMenus[fileM],fmClose);
			break;

		case fmSaveAs:
			
			/*
			 * code for 'fmSaveAs' is from
			 * JPEGtoPICT.c, Written by:	Mark Krueger, Apple Computer, Inc.
			 *
			 */
			 
			if (thePic!=nil)
			{
				SFPutFile(pt,"\pSave PICT:",gTheName,NULL,&sfr);
				if ( !sfr.good )  {
					goto done;
				}
				
				/************************************************
				 *
				 *	Create the new file, if we can.
				 *
				 ************************************************/
				
				FSDelete(sfr.fName,sfr.vRefNum);
				if ( (result=Create(sfr.fName,sfr.vRefNum,'MCDJ','PICT')) != noErr ) {
					goto done;
				}
				if ( (result=FSOpen(sfr.fName,sfr.vRefNum,&compressedFile)) != noErr ) {
					goto done;
				}
		
				/************************************************
				 *
				 * write the silly 512 bytes of zero needed at the start of a PICT file
				 *
				 ************************************************/
					
				SetFPos(compressedFile,fsFromStart,512);
				l = 1;
				for ( i=0; i < 512; i++) {
					if ( (result=FSWrite(compressedFile,&l,&zero)) )
						break;
				}
	
				/************************************************
				 *
				 * write the QuickTime picture data.
				 *
				 ************************************************/
				
				l = GetHandleSize((Handle)thePic);
					
				HLock((Handle)thePic);
				result=FSWrite(compressedFile,&l,*thePic);
				HUnlock((Handle)thePic);
				
done:	
				if ( compressedFile )
				{
					FSClose(compressedFile);
					if ( result != noErr )
						FSDelete(sfr.fName,sfr.vRefNum);
					FlushVol(nil,sfr.vRefNum);
				}

			}
			break;

		case fmPageSetUp:
		
				PrOpen();
					PrintDefault(ghPtrRec);
					PrStlDialog(ghPtrRec);
				PrClose();

			break;

		case fmPrint:
			{
				TPPrPort	myPrtPort;
				Rect		destRect;
				TPrStatus	myPrStatus;
				GrafPtr		savePort;

				
				if( thePic == nil )
					break;
					
				GetPort(&savePort);
				
				PrOpen();
				if(PrJobDialog(ghPtrRec))
				{
					
					myPrtPort = PrOpenDoc( ghPtrRec, 0,0 );
					PrOpenPage( myPrtPort , 0 );
					if ( PrError() )
						goto done_pr;
					
							

					if( thePic != nil )
					{
						BlockMove(&(*thePic)->picFrame, &destRect, sizeof(Rect));
		
						OffsetRect(
							&destRect,
				
							((myPrtPort->gPort.portRect.right-myPrtPort->gPort.portRect.left) -
							(destRect.right - destRect.left)) / 2
							,
							
							((myPrtPort->gPort.portRect.bottom-myPrtPort->gPort.portRect.top) -
							(destRect.bottom - destRect.top)) / 2
							);

						DrawPicture( thePic, &destRect);
					}
					
					PrClosePage( myPrtPort );
					
					PrCloseDoc( myPrtPort );
					
					if ( (*ghPtrRec)->prJob.bJDocLoop == bSpoolLoop )
					{
						PrPicFile( ghPtrRec, 0,0,0, &myPrStatus );
					}
				}
done_pr:
				
				PrClose();
				SetPort(savePort);
			}
			break;


		case fmQuit:
		
			DoFile(fmClose);
			quitting = true;
			break;
	}
}

void DoMouseDown(short windowPart, WindowPtr whichWindow, EventRecord *myEvent)
{
	switch( windowPart ) {
		case inGoAway:
			if( TrackGoAway(whichWindow, myEvent->where) )
				DoFile(fmClose);
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
/********************************************************************************/

void	DoClose(void)
{
	if (HiddenMenu)
	{
		HiddenMenu=false;
		LMSetMBarHeight(oldMBarHeight);
		ShowCursor();
		DrawMenuBar();
	}

	if (( gMyWindow == FrontWindow() ) && ( gMyWindow != nil ))
	{
		DisposeWindow(gMyWindow);
		gMyWindow = nil;
	}
	
	if (thePic)
	{
		KillPicture(thePic);
		thePic = nil;
	}
	DisableItem(myMenus[editM],emCopy);
	DisableItem(myMenus[fileM],fmSaveAs);
	DisableItem(myMenus[fileM],fmPrint);
}

void	DoOpen( FSSpec *myFSSpec )
{
	StandardFileReply	mySFR;
	HFileInfo			pb;
	OSErr				error;
	
	SFTypeList		typeList={ 'PICT','JFIF','JPEG','\?\?\?\?' };



	if ( myFSSpec == nil )
	{
		if(CanOpenJFIF)
		{
			StandardGetFilePreview( nil, 3, typeList, &mySFR);
		}
		else
		{	
			StandardGetFile( nil, 1, typeList, &mySFR);
		}


		if( !mySFR.sfGood ) return;
		
		myFSSpec = &(mySFR.sfFile);
	}
		

#ifdef DO_FOLDERS
	pb.ioNamePtr	= myFSSpec->name;
	pb.ioVRefNum	= myFSSpec->vRefNum;
	pb.ioFDirIndex	= 0;					/* query 1 item */
	pb.ioDirID		= myFSSpec->parID;
	
	PBGetCatInfo( (CInfoPBPtr)&pb, 0 );
	if (((pb.ioFlAttrib>>4) & 0x01) == 1)
	{
		/* Is a Dir */
		gpb.ioNamePtr = myFSSpec->name;
		gpb.ioVRefNum = myFSSpec->vRefNum;

		RecurseDir(pb.ioDirID);
	}
	else
	{
#endif
		/* Is a File */
		
		OpenFile( myFSSpec );
	
#ifdef DO_FOLDERS
	}
#endif

}

void	OpenFile( FSSpec *myFSSpec )
{
	short			fRefNum;
	FInfo			finfo;


	if (thePic)
		KillPicture(thePic);
		
	thePic = nil;
	
	if ( FSpOpenDF(myFSSpec, fsCurPerm, &fRefNum) == noErr )
	{
		if ( gMyWindow == nil)
		{
			gMyWindow = NewCWindow(
							nil, &PackORects[theMonitorID-1],
							"\p", TRUE, 2, (WindowPtr)(-1L), FALSE, 0L
						);
		}
		else
		{
			InvalRect( &(*qd.thePort).portRect );
		}

		if ( gMyWindow == nil)
		{
			SysBeep(1);
			ExitToShell();
		}
	
		SetPort(gMyWindow);
		
		PenPat(&qd.white);
		BackPat(BlackBack ? &qd.black:&qd.white);
	
		
		FSpGetFInfo(myFSSpec, &finfo);
		
		if ( finfo.fdType == 'PICT' )
		{
			thePic=ConvertFromPict(myFSSpec->name, fRefNum, (CWindowPtr)gMyWindow);
		}
		else if ( ( finfo.fdType == 'JPEG' ) || ( finfo.fdType == 'JFIF' ))
		{
			if(CanOpenJFIF)
			thePic=ConvertFromJFIF(myFSSpec->name, fRefNum, (CWindowPtr)gMyWindow);
		}
	}
	
	if (thePic==nil)
	{
		DoClose();
	}
	else
	{	
		CopyString( myFSSpec->name, gTheName );
		EnableItem(myMenus[fileM],fmClose);
		EnableItem(myMenus[editM],emCopy);
		EnableItem(myMenus[fileM],fmSaveAs);
		EnableItem(myMenus[fileM],fmPrint);
	}
}


void RecurseDir(long dirIDToSearch)
{
#ifdef DO_FOLDERS
    short int	index=1;      /* for ioFDirIndex */
    OSErr		err;          /* the usual */
    FSSpec		myFSSpec;

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
                  RecurseDir(gpb.ioDirID);
                  err = 0;
             }
             else 
             {
             		FSMakeFSSpec(
             			0,
             			dirIDToSearch,
             			gpb.ioNamePtr,
             			&myFSSpec
             			);
             			
					OpenFile( &myFSSpec );
			
             }
             
        index += 1;	/* increment the index for GetCatInfo */
        }
    } while ((err == noErr)&&(!Button()));
#endif    
}

void CatString( Str255 StrIO, Str255 Str )
{
	BlockMove(&Str[1], (Str255 *)(StrIO+(int)*StrIO+1), (int) *Str);
	(*StrIO) += (unsigned int)*Str;
}

