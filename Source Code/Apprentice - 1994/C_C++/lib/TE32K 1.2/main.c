#include <stdio.h>
#include <string.h>
#include "TE32K.h"


#define EQSTR	0


#define	yesNoCancelDLOG	132
#define	errorDLOG		256

#define	numMenus	4

#define	appleMenu	0
#define	fileMenu	1
#define	editMenu	2
#define fontMenu	3

#define	appleID		128
#define	fileID		129
#define	editID		130
#define fontID		131

#define	newCommand		1
#define	openCommand		2
#define insertCommand	3
#define	closeCommand	4
#define	saveCommand		5
#define	saveAsCommand	6
#define makeBigCommand	8
#define	quitCommand		10

#define	undoCommand		1
#define	cutCommand		3
#define	copyCommand		4
#define pasteCommand	5
#define	clearCommand	6
#define	selectCommand	7
#define	wrapCommand		9



char			alive;
MenuHandle		myMenus[numMenus];
char			tempString[256];
WindowPtr		theWPtr = 0L;
TE32KHandle		theTEH32K = 0L;
ControlHandle	theVScroll = 0L, theHScroll = 0L;
Cursor			editCursor,waitCursor;
SFReply			mySFReply;
char			changed;
int				defaultFont,defaultFontSize;
char			*theTestString = "This is terminated by a \\n character\n";

RestartProc()
{
	ExitToShell();
}



main()
{
EventRecord		theEvent;
WindowPtr		whichWindow;
unsigned char	theChar;

	InitGraf(&thePort);
	InitFonts();
	FlushEvents(everyEvent,0);
	InitWindows();
	InitMenus();
	TEInit();
	InitDialogs(RestartProc);
	InitCursor();
	
	TE32KInit();
	
	SetUpCursors();
	
	SetUpMenus();
	
	defaultFont = monaco;
	defaultFontSize = 9;
	MaintainFontMenu();
	
	FlushEvents(everyEvent,0);
	
	alive = TRUE;
	
	while (alive)
	{
		SystemTask();
		MaintainCursor();
		MaintainMenus();
		DoIdle();
		
		if (GetNextEvent(everyEvent,&theEvent))
		{
			switch (theEvent.what)
			{
				case mouseDown:
					DoMouseDown(&theEvent);
					break;
				
				case keyDown:
				case autoKey: 
					
					theChar = theEvent.message & charCodeMask;
					
					if (theEvent.modifiers & cmdKey)
						DoCommand(MenuKey((unsigned char) theChar));
					
					else
						DoKey((unsigned char) theChar);
					
					break;
					
				case activateEvt:
					if (IsOurWindow((WindowPtr) theEvent.message)) 
						DoActivateDeactivate((WindowPtr) theEvent.message,(char) (theEvent.modifiers & activeFlag));
					
					break;
					
				case updateEvt: 
					if (IsOurWindow((WindowPtr) theEvent.message)) 
						UpdateWindow((WindowPtr) theEvent.message);
					
					break;
			}
		}
	}
}





pascal void HiliteDefaultButton(theDPtr, whichItem)
DialogPtr theDPtr; int whichItem;
{
int			type;
Handle		theItem;
Rect		theRect;

	GetDItem(theDPtr, ((DialogPeek) theDPtr)->aDefItem, &type, &theItem, &theRect);
	PenNormal();
	PenSize(3,3);
	InsetRect(&theRect,-4,-4);
	FrameRoundRect(&theRect,16,16);
	PenSize(1,1);
}



ErrorAlert(p0)
char	*p0;
{
GrafPtr			oldPort;
DialogPtr		theDPtr,tempDPtr;
int				itemHit, type;
Handle			theItem;
Rect			theRect;
EventRecord		theEvent;

	GetPort(&oldPort);
	
	InitCursor();
	
	if (!(theDPtr = GetNewDialog(errorDLOG, NULL,(WindowPtr) -1L)))
	{
		SysBeep(1);
		ExitToShell();
	}
	
	SetPort(theDPtr);
	
	CenterWindow(theDPtr);
	ShowWindow(theDPtr);
	
	((DialogPeek) theDPtr)->aDefItem = 1;
	
	GetDItem(theDPtr, 3, &type, &theItem, &theRect);
	SetDItem(theDPtr, 3, type, HiliteDefaultButton, &theRect);
	
	GetDItem(theDPtr, 2, &type, &theItem, &theRect);
	CtoPstr(p0);
	SetIText(theItem, p0);
	PtoCstr(p0);
	
	while (!GetNextEvent(updateMask,&theEvent));
	
	if (theEvent.message == (long) theDPtr)
		DialogSelect(&theEvent,&tempDPtr,&itemHit);
	
	SysBeep(1);
	
	do 
	{
		itemHit = 0;
		
		while (!GetNextEvent(everyEvent,&theEvent));
		
		if (theEvent.what==keyDown || theEvent.what==autoKey)
		{
			if ((theEvent.message & charCodeMask)=='\r' || (theEvent.message & charCodeMask)==0x03)
				itemHit = ((DialogPeek) theDPtr)->aDefItem;
			else
				SysBeep(1);
		}
		
		else
		{
			tempDPtr = (DialogPtr) 0L;
			
			if (!IsDialogEvent(&theEvent) || !DialogSelect(&theEvent,&tempDPtr,&itemHit) || tempDPtr!=theDPtr)
				itemHit = 0;
		}
		
	} while (itemHit!=1);
	
	DisposDialog(theDPtr);
	SetPort(oldPort);
}





SetUpMenus()
{
int			i,numDAs;

	myMenus[appleMenu] = GetMenu(appleID);
	AddResMenu(myMenus[appleMenu],'DRVR');
	myMenus[fileMenu] = GetMenu(fileID);
	myMenus[editMenu] = GetMenu(editID);
	myMenus[fontMenu] = GetMenu(fontID);
	InsertResMenu(myMenus[fontMenu],'FONT',0);
	
	InsertMenu(myMenus[appleMenu],0);
	InsertMenu(myMenus[fileMenu],0);
	InsertMenu(myMenus[editMenu],0);
	InsertMenu(myMenus[fontMenu],0);
	
	DrawMenuBar();
	
	numDAs = CountMItems(myMenus[appleMenu]);
	
	for (i=1;i<=numDAs;i++)
		EnableItem(myMenus[appleMenu],i);
}



MaintainMenus()
{
int		i,numItems;

	if (theWPtr && FrontWindow() == theWPtr)
	{
		DisableItem(myMenus[fileMenu],newCommand);
		DisableItem(myMenus[fileMenu],openCommand);
		EnableItem(myMenus[fileMenu],insertCommand);
		EnableItem(myMenus[fileMenu],closeCommand);
		
		if (changed)
			EnableItem(myMenus[fileMenu],saveCommand);
		else
			DisableItem(myMenus[fileMenu],saveCommand);
		
		EnableItem(myMenus[fileMenu],saveAsCommand);
		
		
		DisableItem(myMenus[editMenu],undoCommand);
		
		if (theTEH32K)
		{
			if ((**theTEH32K).selStart == (**theTEH32K).selEnd)
			{
				DisableItem(myMenus[editMenu],cutCommand);
				DisableItem(myMenus[editMenu],copyCommand);
				DisableItem(myMenus[editMenu],clearCommand);
			}
			
			else
			{
				EnableItem(myMenus[editMenu],cutCommand);
				EnableItem(myMenus[editMenu],copyCommand);
				EnableItem(myMenus[editMenu],clearCommand);
			}
			
			if (TE32KGetScrapLen() > 0L)
				EnableItem(myMenus[editMenu],pasteCommand);
			else
				DisableItem(myMenus[editMenu],pasteCommand);
			
			EnableItem(myMenus[editMenu],selectCommand);
			EnableItem(myMenus[editMenu],wrapCommand);
			
			if (theTEH32K && !(**theTEH32K).crOnly)
			{
				CheckItem(myMenus[editMenu],wrapCommand,TRUE);
				SetItemMark(myMenus[editMenu],wrapCommand,checkMark);
			}
				
			else
				CheckItem(myMenus[editMenu],wrapCommand,FALSE);
		}
		
		
		numItems = CountMItems(myMenus[fontMenu]);
		
		for (i=1;i<=numItems;i++)
			EnableItem(myMenus[fontMenu],i);
		
		MaintainFontMenu();
	}
	
	else
	{
		EnableItem(myMenus[fileMenu],newCommand);
		EnableItem(myMenus[fileMenu],openCommand);
		DisableItem(myMenus[fileMenu],insertCommand);
		DisableItem(myMenus[fileMenu],closeCommand);
		DisableItem(myMenus[fileMenu],saveCommand);
		DisableItem(myMenus[fileMenu],saveAsCommand);
		
		
		EnableItem(myMenus[editMenu],undoCommand);
		EnableItem(myMenus[editMenu],cutCommand);
		EnableItem(myMenus[editMenu],copyCommand);
		EnableItem(myMenus[editMenu],pasteCommand);
		EnableItem(myMenus[editMenu],clearCommand);
		DisableItem(myMenus[editMenu],selectCommand);
		DisableItem(myMenus[editMenu],wrapCommand);
		CheckItem(myMenus[editMenu],wrapCommand,FALSE);
		
		numItems = CountMItems(myMenus[fontMenu]);
		
		for (i=1;i<=numItems;i++)
		{
			DisableItem(myMenus[fontMenu],i);
			CheckItem(myMenus[fontMenu],i,FALSE);
		}
	}

}


DoCommand(mResult)
long mResult;
{
int			theItem,numItems,userChoice;
WindowPeek 	wPtr;
GrafPtr		oldPort;
SFReply		oldSFReply;
Rect		tempRect;


	theItem = LoWord(mResult);
	
	switch (HiWord(mResult)) 
	{
		case appleID:
			if (theItem == 1)
			{
				DoAboutBox(0);
			}
			
			else
			{
				GetPort(&oldPort);
				GetItem(myMenus[appleMenu], theItem, &tempString);
				OpenDeskAcc(&tempString);
				SetPort(oldPort);
			}
			
			break;
			
		case fileID:
			switch (theItem)
			{
				case newCommand:
					if (!theTEH32K)
						DoShowWindow();
					else
						SysBeep(1);
					
					break;
				
				case openCommand:
					if (theTEH32K && DoCloseWindow(theWPtr))
					{
						DoShowWindow();
						
						if (OpenTextFile(theTEH32K,&mySFReply))
						{
							SetWTitle(theWPtr,mySFReply.fName);
							AdjustScrollBar();
							changed = FALSE;
						}
					}
					
					else if (!theTEH32K)
					{
						DoShowWindow();
						
						if (OpenTextFile(theTEH32K,&mySFReply))
						{
							SetWTitle(theWPtr,mySFReply.fName);
							AdjustScrollBar();
							changed = FALSE;
						}
					}
					
					break;
				
				case insertCommand:
					if (theTEH32K)
					{
						changed = InsertTextFile(theTEH32K);
						
						AdjustScrollBar();
					}
					
					break;
					
				case closeCommand:
					if (theTEH32K)
						DoCloseWindow(theWPtr);
					else
						SysBeep(1);
						
					break;
				
				case saveCommand:
					if (theTEH32K)
					{
						if (SaveTextFile(theTEH32K,&mySFReply))
						{
							SetWTitle(theWPtr,mySFReply.fName);
							changed = FALSE;
						}
					}
						
					else
						SysBeep(1);
					
					break;
				
				case saveAsCommand:
					if (theTEH32K)
					{
						oldSFReply = mySFReply;
						mySFReply.good = FALSE;
						
						if (SaveTextFile(theTEH32K,&mySFReply))
						{
							changed = FALSE;
							SetWTitle(theWPtr,mySFReply.fName);
						}
						else
							mySFReply = oldSFReply;
					}
						
					else
						SysBeep(1);
					
					break;
				
				case makeBigCommand:
					CreateBigFile();
					break;
				
				case quitCommand:
					if (theTEH32K)
					{
						if (DoCloseWindow(theWPtr))
							alive = FALSE;
					}
					else
						alive = FALSE;
					
					break;
			}
			
			break;
		
		case editID:
			switch(theItem)
			{
				case cutCommand:
					if (!DoCut())
						SystemEdit(theItem - 1);
					break;
				
				case copyCommand:
					if (!DoCopy())
						SystemEdit(theItem - 1);
					break;
				
				case pasteCommand:
					if (!DoPaste())
						SystemEdit(theItem - 1);
					
					break;
				
				case clearCommand:
					if (!DoClear())
						SystemEdit(theItem - 1);
					
					break;
				
				case selectCommand:
					if (!DoSelectAll())
						SystemEdit(theItem - 1);
					
					break;
				
				case wrapCommand:
					if (theWPtr && theWPtr == FrontWindow() && theTEH32K)
					{
						(**theTEH32K).crOnly = !(**theTEH32K).crOnly;
						
						TE32KCalText(theTEH32K);
						
						tempRect = theWPtr->portRect;
						GetPort(&oldPort);
						SetPort(theWPtr);
						InvalRect(&tempRect);
						SetPort(oldPort);
						
						AdjustForResizedWindow();
					}
					
					break;
				
				
				default:
					SystemEdit(theItem - 1);
					break;
			}
			
			break;
			
		case fontID:
			numItems = CountMItems(myMenus[fontMenu]);
			
			if (theItem <= numItems-7)
			{
				GetItem(myMenus[fontMenu], theItem, &tempString);
				PtoCstr(tempString);
				DoFontSize(tempString,-1);
			}
			
			else
			{
				GetItem(myMenus[fontMenu], theItem, &tempString);
				PtoCstr(tempString);
				
				if (sscanf(tempString,"%d",&numItems)==1)
				{
					tempString[0] = 0;
					DoFontSize(tempString,numItems);
				}
			}
			
			break;
	}
	
	HiliteMenu(0);
}




OpenTextFile(theTEH,mySFReply)
TE32KHandle	theTEH;SFReply	*mySFReply;
{
SFTypeList	mySFTypes;
Point		theTopLeft;
int			errCode,fRefNum;
long		byteCount;
Ptr			theText;


	if (theTEH)
	{
		theTopLeft.h = screenBits.bounds.left + (screenBits.bounds.right - screenBits.bounds.left - 348)/2;;
		theTopLeft.v = screenBits.bounds.top + 50;
		
		mySFTypes[0] = 'TEXT';
		
		InitCursor();
		
		SFGetFile(theTopLeft,"\Select Text File To Open:",0L,1,&mySFTypes,0L,mySFReply);
		
		if (mySFReply->good)
		{
			if ((errCode = FSOpen(mySFReply->fName,mySFReply->vRefNum,&fRefNum))!=noErr)
			{
				mySFReply->good = FALSE;
				PtoCstr((char *) mySFReply->fName);
				sprintf(tempString,"OpenTextFile: FSOpen('%s') Error %d",mySFReply->fName,errCode);
				ErrorAlert(tempString);
				return(FALSE);
			}
			
			if ((errCode = GetEOF(fRefNum,&byteCount))!=noErr)
			{
				mySFReply->good = FALSE;
				FSClose(fRefNum);
				PtoCstr((char *) mySFReply->fName);
				sprintf(tempString,"OpenTextFile: GetEOF('%s',0) Error %d",mySFReply->fName,errCode);
				ErrorAlert(tempString);
				return(FALSE);
			}
			
			if ((errCode = SetFPos(fRefNum,fsFromStart,0L))!=noErr)
			{
				mySFReply->good = FALSE;
				FSClose(fRefNum);
				PtoCstr((char *) mySFReply->fName);
				sprintf(tempString,"OpenTextFile: SetFPos('%s',0) Error %d",mySFReply->fName,errCode);
				ErrorAlert(tempString);
				return(FALSE);
			}
			
			theText = NewPtr(byteCount);
			
			if (StripAddress(theText)==0L || MemError())
			{
				mySFReply->good = FALSE;
				FSClose(fRefNum);
				sprintf(tempString,"OpenTextFile: NewPtr(%ld) Error %d",byteCount,MemError());
				ErrorAlert(tempString);
				return(FALSE);
			}
			
			SetCursor(&waitCursor);
			
			if ((errCode = FSRead(fRefNum,&byteCount,theText))!=noErr)
			{
				InitCursor();
				mySFReply->good = FALSE;
				DisposPtr(theText);
				FSClose(fRefNum);
				PtoCstr((char *) mySFReply->fName);
				sprintf(tempString,"OpenTextFile: FSRead('%s') Error %d",mySFReply->fName,errCode);
				ErrorAlert(tempString);
				return(FALSE);
			}
			
			if ((errCode = FSClose(fRefNum))!=noErr)
			{
				InitCursor();
				mySFReply->good = FALSE;
				DisposPtr(theText);
				PtoCstr((char *) mySFReply->fName);
				sprintf(tempString,"OpenTextFile: FSClose('%s',0) Error %d",mySFReply->fName,errCode);
				ErrorAlert(tempString);
				return(FALSE);
			}
			
			TE32KSetText(theText,byteCount,theTEH);
			
			DisposPtr(theText);
			
			InitCursor();
			
			return(TRUE);
		}
		
		else
			return(FALSE);
	}
	
	else
		return(FALSE);
}



SaveTextFile(theTEH,mySFReply)
TE32KHandle	theTEH;SFReply	*mySFReply;
{
Point		theTopLeft;
int			errCode,fRefNum;
long		byteCount;
Handle		theText;

	if (theTEH)
	{
		theTopLeft.h = screenBits.bounds.left + (screenBits.bounds.right - screenBits.bounds.left - 304)/2;;
		theTopLeft.v = screenBits.bounds.top + 50;
		
		InitCursor();
		
		if (!(mySFReply->good))
			SFPutFile(theTopLeft,"\pSave Text File As:","\p",0L,mySFReply);
		
		if (mySFReply->good)
		{
			if ((errCode = FSOpen(mySFReply->fName,mySFReply->vRefNum,&fRefNum))!=noErr)
			{
				if (errCode==fnfErr)
				{
					if ((errCode = Create(mySFReply->fName,mySFReply->vRefNum,'????','TEXT'))!=noErr)
					{
						mySFReply->good = FALSE;
						PtoCstr((char *) mySFReply->fName);
						sprintf(tempString,"SaveTextFile: Create('%s') Error %d",mySFReply->fName,errCode);
						ErrorAlert(tempString);
						return(FALSE);
					}
					
					errCode = FSOpen(mySFReply->fName,mySFReply->vRefNum,&fRefNum);
				}
				
				if (errCode != noErr)
				{
					mySFReply->good = FALSE;
					PtoCstr((char *) mySFReply->fName);
					sprintf(tempString,"SaveTextFile: FSOpen('%s') Error %d",mySFReply->fName,errCode);
					ErrorAlert(tempString);
					return(FALSE);
				}
			}
			
			if ((errCode = SetEOF(fRefNum,0L))!=noErr)
			{
				mySFReply->good = FALSE;
				FSClose(fRefNum);
				PtoCstr((char *) mySFReply->fName);
				sprintf(tempString,"SaveTextFile: SetEOF('%s',0) Error %d",mySFReply->fName,errCode);
				ErrorAlert(tempString);
				return(FALSE);
			}
			
			if ((errCode = SetFPos(fRefNum,fsFromStart,0L))!=noErr)
			{
				mySFReply->good = FALSE;
				FSClose(fRefNum);
				PtoCstr((char *) mySFReply->fName);
				sprintf(tempString,"SaveTextFile: SetFPos('%s',0) Error %d",mySFReply->fName,errCode);
				ErrorAlert(tempString);
				return(FALSE);
			}
			
			theText = (**theTEH).hText;
			byteCount = (**theTEH).teLength;
			
			HLock(theText);
			
			SetCursor(&waitCursor);
			
			if ((errCode = FSWrite(fRefNum,&byteCount,*theText))!=noErr)
			{
				InitCursor();
				mySFReply->good = FALSE;
				HUnlock(theText);
				FSClose(fRefNum);
				PtoCstr((char *) mySFReply->fName);
				sprintf(tempString,"SaveTextFile: FSWrite('%s') Error %d",mySFReply->fName,errCode);
				ErrorAlert(tempString);
				return(FALSE);
			}
			
			HUnlock(theText);
			
			if ((errCode = FSClose(fRefNum))!=noErr)
			{
				InitCursor();
				mySFReply->good = FALSE;
				PtoCstr((char *) mySFReply->fName);
				sprintf(tempString,"SaveTextFile: FSClose('%s',0) Error %d",mySFReply->fName,errCode);
				ErrorAlert(tempString);
				return(FALSE);
			}
			
			FlushVol("\p",mySFReply->vRefNum);
			
			InitCursor();
			
			return(TRUE);
		}
		
		else
			return(FALSE);
	}
	
	else
		return(FALSE);
}




YesNoCancel(thePrompt,theDefault)
char	*thePrompt;int	theDefault;
{
DialogPtr		theDPtr,tempDPtr;
int				itemHit, type;
Handle			theItem;
Rect			tempRect;
GrafPtr			oldPort;
char			tempString[256];
EventRecord		theEvent;

	GetPort(&oldPort);
	
	InitCursor();
	
	theDPtr = GetNewDialog(yesNoCancelDLOG, 0L,(WindowPtr) -1L);
	
	CenterWindow(theDPtr);
	ShowWindow(theDPtr);
	
	SetPort(theDPtr);
	
	((DialogPeek) theDPtr)->aDefItem = theDefault;
	
	GetDItem(theDPtr, 4, &type, &theItem, &tempRect);
	SetDItem(theDPtr, 4, type, HiliteDefaultButton, &tempRect);
	
	strcpy(tempString,thePrompt);
	CtoPstr(tempString);
	GetDItem(theDPtr, 5, &type, &theItem, &tempRect);
	SetIText(theItem, tempString);
	
	if (GetNextEvent(updateMask,&theEvent) && theEvent.message == (long) theDPtr)
		DialogSelect(&theEvent,&tempDPtr,&itemHit);
	
	SysBeep(1);
	
	do 
	{
		ModalDialog(0L,&itemHit);
		
	} while (!(itemHit>=1 && itemHit<=3));
	
	DisposDialog(theDPtr);
	SetPort(oldPort);
	
	return(itemHit);
}




CenterWindow(wPtr)
WindowPtr	wPtr;
{
int		screenWidth,screenHeight,windowWidth,windowHeight,left,top;

	if (wPtr == 0L)
		return;
	
	screenWidth = screenBits.bounds.right - screenBits.bounds.left;
	screenHeight = screenBits.bounds.bottom - screenBits.bounds.top - 20;
	windowWidth = wPtr->portRect.right - wPtr->portRect.left;
	windowHeight = wPtr->portRect.bottom - wPtr->portRect.top;
	
	left = screenBits.bounds.left + (screenWidth - windowWidth)/2;
	top = screenBits.bounds.top + 20 + (screenHeight - windowHeight)/2;
	
	if (left < 0)
		left = 0;
	
	if (top < 20)
		top = 20;
	
	MoveWindow(wPtr,left,top,FALSE);
}




InsertTextFile(theTEH)
TE32KHandle	theTEH;
{
SFReply		mySFReply;
SFTypeList	mySFTypes;
Point		theTopLeft;
int			errCode,fRefNum;
long		byteCount;
Ptr			theText;
char		tempString[256];

	if (theTEH)
	{
		theTopLeft.h = screenBits.bounds.left + (screenBits.bounds.right - screenBits.bounds.left - 348)/2;;
		theTopLeft.v = screenBits.bounds.top + 50;
		
		mySFTypes[0] = 'TEXT';
		
		InitCursor();
		
		SFGetFile(theTopLeft,"\Select Text File To Insert:",0L,1,&mySFTypes,0L,&mySFReply);
		
		if (mySFReply.good)
		{
			if ((errCode = FSOpen(mySFReply.fName,mySFReply.vRefNum,&fRefNum))!=noErr)
			{
				PtoCstr((char *) mySFReply.fName);
				sprintf(tempString,"InsertTextFile: FSOpen('%s') Error %d",mySFReply.fName,errCode);
				ErrorAlert(tempString);
				return(TRUE);
			}
			
			if ((errCode = GetEOF(fRefNum,&byteCount))!=noErr)
			{
				FSClose(fRefNum);
				PtoCstr((char *) mySFReply.fName);
				sprintf(tempString,"InsertTextFile: GetEOF('%s',0) Error %d",mySFReply.fName,errCode);
				ErrorAlert(tempString);
				return(TRUE);
			}
			
			if ((errCode = SetFPos(fRefNum,fsFromStart,0L))!=noErr)
			{
				FSClose(fRefNum);
				PtoCstr((char *) mySFReply.fName);
				sprintf(tempString,"InsertTextFile: SetFPos('%s',0) Error %d",mySFReply.fName,errCode);
				ErrorAlert(tempString);
				return(TRUE);
			}
			
			theText = NewPtr(byteCount);
			
			if (StripAddress(theText)==0L || MemError())
			{
				FSClose(fRefNum);
				sprintf(tempString,"InsertTextFile: NewPtr(%ld) Error %d",byteCount,MemError());
				ErrorAlert(tempString);
				return(TRUE);
			}
			
			if ((errCode = FSRead(fRefNum,&byteCount,theText))!=noErr)
			{
				DisposPtr(theText);
				FSClose(fRefNum);
				PtoCstr((char *) mySFReply.fName);
				sprintf(tempString,"InsertTextFile: FSRead('%s') Error %d",mySFReply.fName,errCode);
				ErrorAlert(tempString);
				return(TRUE);
			}
			
			if ((errCode = FSClose(fRefNum))!=noErr)
			{
				DisposPtr(theText);
				PtoCstr((char *) mySFReply.fName);
				sprintf(tempString,"InsertTextFile: FSClose('%s',0) Error %d",mySFReply.fName,errCode);
				ErrorAlert(tempString);
				return(TRUE);
			}
			
			SetCursor(&waitCursor);
			
			TE32KInsert(theText,byteCount,theTEH);
			
			InitCursor();
			
			DisposPtr(theText);
		}
		
		return(TRUE);
	}
	
	else
		return(FALSE);
}




DoFontSize(theFontName,theFontSize)
char *theFontName;int	theFontSize;
{
WindowPtr	theWPtr;
int			theFontNum;
Rect		tempRect;
GrafPtr		oldPort;
	
	if (theTEH32K)
	{
		if (theFontName[0])
		{
			CtoPstr(theFontName);
			GetFNum(theFontName,&theFontNum);
		}
		else
			theFontNum = -1;
		
		
		if (theFontNum < 0)
			theFontNum = defaultFont;
		
		if (theFontSize <= 0)
			theFontSize = defaultFontSize;
			
		defaultFont = theFontNum;
		defaultFontSize = theFontSize;
		
		TE32KSetFontStuff(theFontNum,0,srcCopy,theFontSize,theTEH32K);
		
		AdjustForResizedWindow();
		
		GetPort(&oldPort);
		SetPort((**theTEH32K).inPort);
		tempRect = (*(**theTEH32K).inPort).portRect;
		InvalRect(&tempRect);
		SetPort(oldPort);
		
		MaintainFontMenu();
	}
}





MaintainFontMenu()
{
int		i,numItems,tempSize;
char	theFontString[128];
char	tempString[256];

	numItems = CountMItems(myMenus[fontMenu]);
	
	GetFontName(defaultFont,theFontString);
	PtoCstr(theFontString);
	
	for (i=1;i<=numItems-7;i++)
	{
		GetItem(myMenus[fontMenu],i,&tempString);
		PtoCstr(tempString);
		
		if (strcmp(tempString,theFontString) == EQSTR)
		{
			CheckItem(myMenus[fontMenu],i,TRUE);
			SetItemMark(myMenus[fontMenu],i,checkMark);
		}
		else
			CheckItem(myMenus[fontMenu],i,FALSE);
	}
	
	
	sprintf(theFontString,"%d",defaultFontSize);
	
	for (i=numItems-6;i<=numItems;i++)
	{
		GetItem(myMenus[fontMenu],i,&tempString);
		PtoCstr(tempString);
		
		if (sscanf(tempString,"%d",&tempSize) != 1)
			tempSize = -1;
		
		if (strcmp(tempString,theFontString) == EQSTR)
		{
			CheckItem(myMenus[fontMenu],i,TRUE);
			SetItemMark(myMenus[fontMenu],i,checkMark);
		}
		else
			CheckItem(myMenus[fontMenu],i,FALSE);
		
		if (RealFont(defaultFont,tempSize))
			SetItemStyle(myMenus[fontMenu],i,outline);
		else
			SetItemStyle(myMenus[fontMenu],i,0);
	}
}




CreateBigFile()
{
int			errCode,fRefNum;
long		byteCount,i;
char		tempString[256];
SFReply		mySFReply;
Point		theTopLeft;

	theTopLeft.h = screenBits.bounds.left + (screenBits.bounds.right - screenBits.bounds.left - 304)/2;;
	theTopLeft.v = screenBits.bounds.top + 50;
	
	InitCursor();
	
	strcpy((char *) mySFReply.fName,"Big Text File");
	CtoPstr((char *) mySFReply.fName);
	
	SFPutFile(theTopLeft,"\pSave 10,000 line file as:",mySFReply.fName,0L,&mySFReply);
	
	if (!(mySFReply.good))
		return;
	
	if ((errCode = FSOpen(mySFReply.fName,mySFReply.vRefNum,&fRefNum))!=noErr)
	{
		if (errCode==fnfErr)
		{
			if ((errCode = Create(mySFReply.fName,mySFReply.vRefNum,'????','TEXT'))!=noErr)
			{
				PtoCstr((char *) mySFReply.fName);
				sprintf(tempString,"createBigFile: Create('%s') Error %d",(char *) mySFReply.fName,errCode);
				ErrorAlert(tempString);
				return(FALSE);
			}
			
			errCode = FSOpen(mySFReply.fName,mySFReply.vRefNum,&fRefNum);
		}
		
		if (errCode != noErr)
		{
			PtoCstr((char *) mySFReply.fName);
			sprintf(tempString,"createBigFile: FSOpen('%s') Error %d",(char *) mySFReply.fName,errCode);
			ErrorAlert(tempString);
			return(FALSE);
		}
	}
			
	if ((errCode = SetEOF(fRefNum,0L))!=noErr)
	{
		FSClose(fRefNum);
		PtoCstr((char *) mySFReply.fName);
		sprintf(tempString,"createBigFile: SetEOF('%s',0L) Error %d",(char *) mySFReply.fName,errCode);
		ErrorAlert(tempString);
		return(FALSE);
	}
	
	if ((errCode = SetFPos(fRefNum,fsFromStart,0L))!=noErr)
	{
		FSClose(fRefNum);
		PtoCstr((char *) mySFReply.fName);
		sprintf(tempString,"createBigFile: SetFPos('%s',0) Error %d",(char *) mySFReply.fName,errCode);
		ErrorAlert(tempString);
		return(FALSE);
	}
	
	SetCursor(&waitCursor);
	
	for (i=0;i<10000;i++)
	{
		sprintf(tempString,"line %ld\r",i);
		byteCount = strlen(tempString);
		
		if ((errCode = FSWrite(fRefNum,&byteCount,tempString))!=noErr)
		{
			InitCursor();
			FSClose(fRefNum);
			PtoCstr((char *) mySFReply.fName);
			sprintf(tempString,"createBigFile: FSWrite('%s') Error %d",(char *) mySFReply.fName,errCode);
			ErrorAlert(tempString);
			return(FALSE);
		}
	}
		
	if ((errCode = FSClose(fRefNum))!=noErr)
	{
		InitCursor();
		PtoCstr((char *) mySFReply.fName);
		sprintf(tempString,"createBigFile: FSClose('%s') Error %d",(char *) mySFReply.fName,errCode);
		ErrorAlert(tempString);
		return(FALSE);
	}
	
	FlushVol("\p",mySFReply.vRefNum);
	
	InitCursor();
	
	return(TRUE);
}