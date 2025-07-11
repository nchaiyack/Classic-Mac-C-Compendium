/*-----------------------------------------------------------------------
	Vectors
	by Mel Park
	mpark@utmem1.utmem.edu
	
	This code is in the Public Domain and is freely distributable
	
	The style in which Vectors is a little unusual. It is a pure C 
	implimentation of ideas contained in the object class library 
	published in "Elements of C++ Macintosh Programming" by Dan Weston.
	Objects are implemented as structures, member functions as pointers
	to subroutines within the object struct.

------------------------------------------------------------------------*/

#include "CVectors.h"
#include "CVGlobs.h"

#ifdef __powerc
QDGlobals qd;
#endif

/*---------------------------- Declarartions ----------------------------*/
void CorrectDialog(StringPtr,StringPtr);

extern void _DataInit();			/* this is the C initialization code */
extern void nullWindowMethod(WindowPtr);
extern void nullEventMethod(WindowPtr,EventRecord *);
extern int nullIntMethod(WindowPtr);
extern void DoPageScroll(ControlHandle,short);
extern void Scroll(ControlHandle,short);
extern void ScrollClick(WindowPtr,EventRecord *);
extern void textDestructor(WindowPtr wind);
extern int ReadDefaultResourceFork(TE32KHandle);

void FatalError(void);

int SetUpWorld(void);
void QuitTime(void);
void ActivateEvt(EventRecord *);   
void OSEvent(EventRecord *);
void CheckForDASwitch(WindowPtr);
void DoSuspend(EventRecord *,Boolean);
void DoResume(EventRecord *,Boolean);
void UpdateEvt(EventRecord *);
static void DoKey(EventRecord *);
static void DoMouse(EventRecord *);
int CloseADoc(WindowPtr);
WindowPtr FindDoc(WindowPtr);
void AdjustMenus(WindowPtr wind);

static void DoAboutDialog(void);

void OpenOldDoc(int);
WindowPtr InitOldDoc(OSType, FSSpecPtr);
Boolean OpenDocFromFinder(void);
Boolean AcceptableFileType(OSType);
void OpenNewDoc(void);
void GiveClipToSystem(void);
void GetClipFromSystem(void);
Boolean doProcessFilesDialog(StringPtr);
void doOptionsDialog(void);
void TempToDoc(DocumentPeek);
void ProcessWindow(WindowPtr,short);
void WindowMenuSelect(short);
char *QuoteName(char *);
char *ConcatOptions(char *);
void MakeWorksheetDoc(FSSpec *spec);
void sortMarks(DocumentPeek, Boolean);
void FillMarkMenu(DocumentPeek doc);
void InitAEStuff(void);
void DoHighLevel(EventRecord *);

pascal OSErr AEOpenHandler(AppleEvent *, AppleEvent *, long);
pascal OSErr AEOpenDocHandler(AppleEvent *, AppleEvent *, long);
pascal OSErr AEPrintHandler(AppleEvent *, AppleEvent *, long);
pascal OSErr AEQuitHandler(AppleEvent *, AppleEvent *, long);
OSErr processOpenPrint(AppleEvent *, Boolean);

void SizeDocWindow(WindowPtr, Rect *);
void PositionDocWindow(WindowPtr, Rect *,Boolean);
void TileWindows(void);
void StackWindows(void);

MarkRecPtr GetIndMark(Ptr, short);

static DocumentPeek OpenClipboardWindow(void);

/*----------------------------- RestartProc -----------------------------*/

#ifdef MC68000
#pragma segment Main
#endif

restartProc()
{
	QuitTime();
}

/*----------------------------- doMessage --------------------------------*/

pascal Boolean SimpleDialogFilter(DialogPtr dialog, EventRecord *theEvent, short *itemHit)
{
#ifdef MC68000
#pragma unused(dialog)
#endif
	char key;
	
	if(theEvent->what==keyDown) {
		key=theEvent->message&charCodeMask;
		switch(key) {
			case 3: case 13:		/* Enter or CR */
				*itemHit=1;
				return true;
			case 46:				/* Period */
				if(theEvent->modifiers&0x100) {
			case 27:				/* Escape */
					*itemHit=2;
					return true;
				}
			default:
				return false;
		}
	}
	return false;
}

pascal void doMessage(short message)	/* Uses Str resource 130 */ 
{
	DialogPtr dialogP;
	short itemType;
	Rect box;
	Handle itemHand;
	Str255 buf;
	
	GetIndString(buf,130,message);
	ParamText(buf,(StringPtr)"",(StringPtr)"",(StringPtr)"");
	dialogP=GetNewDialog(MessageDialog,(Ptr)NIL,(WindowPtr)-1);
	if (dialogP==NIL)
		FatalError();
	else {
		GetDItem(dialogP,4,&itemType,&itemHand,&box);
		SetDItem(dialogP,4,itemType,(Handle)doButton,&box);
		InitCursor();
		ModalDialog(SimpleDialogFilter,&itemType);
		DisposDialog(dialogP);
	}
}

void CorrectDialog(StringPtr buf,StringPtr message)
{
	Str255 nullStr;
	DialogPtr dialog;
	short item;
	Handle itemHand;
	Rect box;
	
	*nullStr = 0;
	ParamText(message,nullStr,nullStr,nullStr);
	dialog=GetNewDialog(rCorrectDialog,(Ptr)NIL,(WindowPtr)-1);
	SysBeep(5);
	if (dialog==NIL)
		FatalError();
	else {
		InitCursor();
		GetDItem(dialog,3,&item,&itemHand,&box);
		SetIText(itemHand,buf);
		SelIText(dialog,3,0,32767);
		do {
			ModalDialog(SimpleDialogFilter,&item);
		} while(item!=1);
		GetDItem(dialog,3,&item,&itemHand,&box);
		GetIText(itemHand,buf);
		DisposDialog(dialog);
	}	
}

void Correct2Dialog(StringPtr buf1,StringPtr buf2,StringPtr message1,StringPtr label1,StringPtr label2)
{
	Str255 nullStr;
	DialogPtr dialog;
	short item;
	Handle itemHand;
	Rect box;
	
	*nullStr = 0;
	ParamText(message1,label1,label2,nullStr);
	dialog=GetNewDialog(rCorrect2Dialog,(Ptr)NIL,(WindowPtr)-1);
	SysBeep(5);
	if (dialog==NIL)
		FatalError();
	else {
		InitCursor();
		GetDItem(dialog,3,&item,&itemHand,&box);
		SetIText(itemHand,buf1);
		GetDItem(dialog,4,&item,&itemHand,&box);
		SetIText(itemHand,buf2);
		SelIText(dialog,3,0,32767);
		do {
			ModalDialog(SimpleDialogFilter,&item);
		} while(item!=1);
		GetDItem(dialog,3,&item,&itemHand,&box);
		GetIText(itemHand,buf1);
		GetDItem(dialog,4,&item,&itemHand,&box);
		GetIText(itemHand,buf2);
		DisposDialog(dialog);
	}	
}

void doDiagnosticMessage(short message, short errNum)
{
	DialogPtr dialogP;
	short item;
	Str255 buf,num;
	
	GetIndString(buf,130,message);
	NumToString(errNum,num);
	ParamText(buf,num,(StringPtr)"",(StringPtr)"");
	dialogP=GetNewDialog(MessageDialog,(Ptr)NIL,(WindowPtr)-1);
	if (dialogP==NIL)
		FatalError();
	else {
		InitCursor();
		ModalDialog(SimpleDialogFilter,&item);
		DisposDialog(dialogP);
	}
}

/*------------------------------- SetUpWorld -----------------------------*/

#ifdef MC68000
#pragma segment setup
#endif

int SetUpWorld(void)
{
	OSErr err;
	short MBarHeight;
	int i,response=0;
	GDHandle maingd;
	
	err=Gestalt(gestaltSystemVersion,&response);
	response&=0xFFFF;
	if(err!=noErr || response<0x604) {
		SysBeep(7);
		doMessage(15);
		return false;
	}
	InitGraf(&qd.thePort);
	InitFonts();
	FlushEvents(everyEvent,nullStopMask);
	InitWindows();
	InitMenus();
	TE32KInit();
	InitDialogs((ResumeProcPtr)restartProc);

	InitACurs(160);			/* Init the beachball cursor */
	InitCursor();
	MaxApplZone();
	
    MoreMasters();        /* Make sure all the master   */
    MoreMasters();        /* pointers I need will be at */
    MoreMasters();        /*   the bottom of the heap   */
			
	response=0;
	err=Gestalt(gestaltKeyboardType,&response);
	gMacPlusKBD=(err!=noErr && response<=3);
	
	response=0;
	err=Gestalt(gestaltQuickdrawVersion,&response);
	gHasColorQD=(err==noErr && (response&0xFF00));
	
	for (i=0;i<MAXWINDS;i++) {
		Documents[i].fDocWindow=NULL;
	}
	
	MBarHeight=GetMBarHeight();
	SetRect(&DragBoundsRect,0,0,512,342);
	if(gHasColorQD) {
		maingd=GetMainDevice();
		DragBoundsRect=(**maingd).gdRect;
	}
	InsetRect(&DragBoundsRect,4,4);
	DragBoundsRect.top+=(MBarHeight-4);
	
	gInBackground = false;
	gDAonTop = false;
	gLastScrapCount = 0;
	
	GetClipFromSystem();
	
	if(gHasColorQD) {
		if(!(DefaultPalette=GetNewPalette(130))) {
			return 0;
		}
		if(!(MyCMHandle=(CTabHandle)NewHandle(sizeof(ColorTable)))) {
			doMessage(14);
			return 0;
		}
		Palette2CTab(DefaultPalette,MyCMHandle);  
	}
	
	AppScratch = NewHandle(1024);
	if(!AppScratch)
		return 0;
		
	gAppResRefNum = CurResFile();

	PenNormal();
	return true;
}
	
/*----------------------------- setUpMenus ------------------------------*/

setUpMenus()
{	
	DeskMenu=GetMenu(DESK_ID);
	AddResMenu(DeskMenu,'DRVR');
	InsertMenu(DeskMenu,0);
	FileMenu=GetMenu(FILE_ID);
	InsertMenu(FileMenu,0);
	EditMenu=GetMenu(EDIT_ID);
	InsertMenu(EditMenu,0);
	FindMenu=GetMenu(FIND_ID);
	InsertMenu(FindMenu,0);
	MarkMenu=GetMenu(MARK_ID);
	InsertMenu(MarkMenu,0);
	WindowMenu=GetMenu(WINDOW_ID);
	InsertMenu(WindowMenu,0);
	ViewMenu=GetMenu(VIEW_ID);
	InsertMenu(ViewMenu,0);
	ReportsMenu=GetMenu(REPORTS_ID);
	InsertMenu(ReportsMenu,0);
	RotateMenu=GetMenu(ROTATE_ID);
	InsertMenu(RotateMenu,-1);
	ScaleMenu=GetMenu(SCALE_ID);
	InsertMenu(ScaleMenu,-1);
	DataMenu=GetMenu(DATA_ID);
	InsertMenu(DataMenu,0);
	SelectTemplateMenu=GetMenu(SELECTTEMPLATE_ID);
	AddResMenu(SelectTemplateMenu,'MRPd');
	InsertMenu(SelectTemplateMenu,-1);
	gCurrentTemplate=CountMItems(SelectTemplateMenu);
	if(gCurrentTemplate) {
		CheckItem(SelectTemplateMenu,1,true);
		gCurrentTemplate=1;
	}
	AdjustMenus((WindowPtr)nil);
	DrawMenuBar();
	return;
}

#ifdef MC68000
#pragma segment Main
#endif

void FatalError(void)
{
	SysBeep(5);
	ExitToShell();
}

void errorAlertDialog(int errFlag)
{
	DialogPtr dialogP;
	short itemType;
	Rect box;
	Handle itemHand;
	
	if(errFlag>0)
		ParamText("\pError",c2pstr(ErrorTable[errFlag]),(StringPtr)"",(StringPtr)"");
	else
		ParamText("\pFatal Error",c2pstr(ErrorTable[errFlag]),(StringPtr)"",(StringPtr)"");
	dialogP=GetNewDialog(MessageDialog,(Ptr)NIL,(WindowPtr)-1);
	if (dialogP==NIL)
		FatalError();
	else {
		GetDItem(dialogP,4,&itemType,&itemHand,&box);
		SetDItem(dialogP,4,itemType,(Handle)doButton,&box);
		InitCursor();
		ModalDialog(SimpleDialogFilter,&itemType);
		DisposDialog(dialogP);
	}
	p2cstr((StringPtr)ErrorTable[errFlag]);
}

void ActivateEvt(EventRecord *EvtPtr)           
{
	WindowPtr targetWindow;
    
	targetWindow=(WindowPtr)EvtPtr->message; 		/* What window is it in?  */
	
	if(FindDoc(targetWindow)) 			/* Is it a Documents window? */
		DoActivate(targetWindow,EvtPtr);
}

/*	FrontLayer() substitutes for FrontWindow(). It never returns a pane
	but rather its parent window. */
	
WindowPtr FrontLayer(void)
{
	WindowPtr wind;
	
	if(wind = FrontWindow()) {
		if(((DocumentPeek)wind)->homeWindow)
			return ((DocumentPeek)wind)->homeWindow;
	}
	return wind;
}

static void DoAboutDialog(void)
{
	short itemHit,dWidth;
	DialogPtr abtDlg;
	DialogTHndl dT;

	dT = (DialogTHndl)GetResource('DLOG',1001);
	LoadResource((Handle)dT);
	
	dWidth=(**dT).boundsRect.right-(**dT).boundsRect.left;
	(**dT).boundsRect.left=(DragBoundsRect.right+DragBoundsRect.left-dWidth)/2;
	(**dT).boundsRect.right=(**dT).boundsRect.left + dWidth;

	abtDlg=GetNewDialog(1001,NULL,(WindowPtr)(-1));
		
	itemHit=30;	/* just any big number */
	while (itemHit>10) 
		ModalDialog(SimpleDialogFilter,&itemHit);

	DisposDialog(abtDlg);
}

/*------------------------------- doMenu ---------------------------------*/

DoMenu(long menuResult,WindowPtr wind,short modifiers)
{
	int menuID,itemNumber;
	short itemMark;
	TE32KHandle tH;
	GrafPtr theCurrentPort;
	Str255 daName;
	
	menuID=HiWord(menuResult);
	itemNumber=LoWord(menuResult);
	InitCursor();
	
	/* Give current doc a chance to handle the item. */
	if(wind && (*((DocumentPeek)wind)->doDocMenuCommand)(wind,menuID,itemNumber,modifiers)) {
		HiliteMenu(0);
		return;
	}
	
	switch(menuID) {
		case DESK_ID:
			if (itemNumber==1) 
				DoAboutDialog();
			else if (itemNumber==0) 	/* nested menus */
				;
			else {						/* a desk accessory */
				GetItem(DeskMenu,itemNumber,&daName);
				GetPort(&theCurrentPort);
				OpenDeskAcc(&daName);
				SetPort(theCurrentPort);
			}
			break;
		case FILE_ID:	/* the file menu */
			switch(itemNumber) {
				case iNew:
					OpenNewDoc();
					break;
				case iOpen:
					OpenOldDoc(modifiers&optionKey);
				    break;
				case iClose:
					if (wind) 
						CloseADoc(wind);
					break;
				case iSave:
					if(wind)
						DoSave(wind);
					break;
				case iSaveAs:
					if(wind)
						DoSaveAs(wind);
					break;
				case iSaveACopy:
					if(wind)
						DoSaveACopy(wind);
					break;
				case iRevert:
					if(wind)
						DoRevert(wind);
					break;
				case iPageSetup:
					if(wind)
						(*((DocumentPeek)wind)->doPageSetup)(wind);
					break;
				case iPrint:
					if(wind)
						(*((DocumentPeek)wind)->doPrint)(wind);
					break;
				case iQuit:
					QuitTime();
					break;
				default:
					break;
			}
			break;
		case EDIT_ID:	/* the edit menu */
			switch(itemNumber) {
				case iUndo:
					(*((DocumentPeek)wind)->doUndo)(wind);
					break;
				case iCut:
					(*((DocumentPeek)wind)->doCut)(wind);
					break;
				case iCopy:
					(*((DocumentPeek)wind)->doCopy)(wind);
					break;
				case iPaste:
					(*((DocumentPeek)wind)->doPaste)(wind);
					break;
				case iClear:
					(*((DocumentPeek)wind)->doClear)(wind);
					break;
				case iSelectAll:
					(*((DocumentPeek)wind)->doSelectAll)(wind);
					break;
				case iShowClipboard:
					if(gClipboardDoc) 
						CloseADoc((WindowPtr)gClipboardDoc);
					else 
						gClipboardDoc=OpenClipboardWindow();
					break;
				default:
					break;
			}
			break;
		case FIND_ID:		/* the Find menu */
			switch(itemNumber) {
				case iSelectSurvey:
					doSelectDialog(FrontLayer());
					break;
				case iDisplaySelection:
					DisplaySelection(FrontLayer());
					break;
				case iReplace:
					break;
				case iReplaceSame:
					break;
				default:
					break;
			}
			break;
		case MARK_ID:
			if(itemNumber==iAlphabetical) {	
				GetItemMark(MarkMenu,iAlphabetical,&itemMark);
				sortMarks((DocumentPeek)wind,(Boolean)itemMark);
			} 
			break;
		case WINDOW_ID:
			if(itemNumber==iTileWindows) {
				TileWindows();
			} else if(itemNumber==iStackWindows) {
				StackWindows();
			} else {
				WindowMenuSelect(itemNumber-3);
			}
			break;
		case DATA_ID:
			switch (itemNumber) {
				case iProcess:
					HiliteMenu(0);
					ProcessWindow(wind,modifiers&shiftKey);
					return;
				case iProcessFiles:
					if(doProcessFilesDialog(daName)) {
						ConcatOptions((StringPtr)daName);
						HiliteMenu(0);
						if(modifiers&shiftKey) {
							strcat((char*)gCommandLine,"\n");
							PrintBuf(gCommandLine,strlen((char*)gCommandLine),0);
						} else {
							Main(0,0,(char*)gCommandLine);
							UnloadSeg(Main);
						}
						if(gWorkSheetDoc) {
							tH = (TE32KHandle)gWorkSheetDoc->docData;
							TE32KSelView(tH);
							AdjustScrollBars((WindowPtr)gWorkSheetDoc);
							SetScrollBarValues((WindowPtr)gWorkSheetDoc);
						}
						return;
					}
					break;
				case iOptions:
					doOptionsDialog();
					break;
				default:
					break;
			}
			break;
		case SELECTTEMPLATE_ID:
			if(gCurrentTemplate) 
				CheckItem(SelectTemplateMenu,gCurrentTemplate,false);
			gCurrentTemplate=itemNumber;
			CheckItem(SelectTemplateMenu,gCurrentTemplate,true);
			break;
		case ROTATE_ID:
			switch(itemNumber) {
				case 1:		/* Plan */
				case 2:		/* Profile */
				case 4:		/* Other */
				default:
					break;
			}
			break;
		case SCALE_ID:
			if(itemNumber==7) {	/* Other� */
				;
			} else if(itemNumber<6) {
#if 0
				FillOffscreen((DocumentPeek)wind,2-itemNumber-fStuff->gain);
				SetPort(wind);
				EraseRect(&wind->portRect);
				InvalRect(&wind->portRect);
#endif
			}
		default:
			break;
	}	
	return;
}

void QuitTime(void)
{
	int i;
	
	for(i=MAXWINDS-1;i>=0;i--) {
		if (Documents[i].docData) {
			if(!CloseADoc((WindowPtr)&Documents[i]))
				return;		/* Couldn't close a doc. Abort Quit. */
		}
	}
	GiveClipToSystem();
	gAppIsRunning=false;
}

WindowPtr FindDoc(WindowPtr wind)
{
	int i;
	
	for(i=0;i<MAXWINDS;i++) {
		if(wind == (WindowPtr) &Documents[i])
			return wind;
	}
	return 0;
}

void AppIdle(WindowPtr curDoc)
{
	if(curDoc)
		(*((DocumentPeek)curDoc)->doIdle)(curDoc);
}
	
/*------------------------------- EventLoop -----------------------------*/

static void eventLoop(void)
{
	EventRecord theEvent;
	int gotEvent;
	WindowPtr theFrontWindow;
	
	while(gAppIsRunning) {
		theFrontWindow = FrontWindow();
		CheckForDASwitch(theFrontWindow);
		
		gotEvent=WaitNextEvent(everyEvent,&theEvent,(long)NIL,(RgnHandle)NIL);
		
		if(gotEvent) {
			switch(theEvent.what) {
				case osEvt:
					OSEvent(&theEvent);
					break;
				case mouseDown:
					DoMouse(&theEvent);
					break;
				case keyDown:
				case autoKey:
					DoKey(&theEvent);
					break;
				case activateEvt:
					ActivateEvt(&theEvent);
					break;
				case updateEvt:
					UpdateEvt(&theEvent);
					break;
				case nullEvent:
					AppIdle(FindDoc(FrontLayer()));
					break;
				case kHighLevelEvent:
                	DoHighLevel(&theEvent);
				default:
					break;
			}
		} else 
			AppIdle(FindDoc(FrontLayer()));
	}
}

void DoHighLevel(EventRecord *AERecord)
{
	OSErr myErr;
    myErr=AEProcessAppleEvent(AERecord);
}

void UpdateEvt(EventRecord *event)
{
	WindowPtr wind;
	
	wind = (WindowPtr) event->message;
	if(FindDoc(wind))
		DoTheUpdate(wind,event);
}

Boolean isEditKey(EventRecord *theEvent)
{
	int keyCode,ch;
	
/*	This routine returns true if the key is an arrow key. */

	if(gMacPlusKBD) {
		switch(keyCode = (int)(theEvent->message&keyCodeMask)>>8) {
			case 0x42:
				theEvent->message=(theEvent->message&~charCodeMask)|RIGHTARROW;
				break;
			case 0x46:
				theEvent->message=(theEvent->message&~charCodeMask)|LEFTARROW;
				break;
			case 0x48:
				theEvent->message=(theEvent->message&~charCodeMask)|DOWNARROW;
				break;
			case 0x4D:
				theEvent->message=(theEvent->message&~charCodeMask)|UPARROW;
				break;
			default:
				break;
		}
	}
	ch=theEvent->message&charCodeMask;
	return ch==LEFTARROW || ch==RIGHTARROW || ch==DOWNARROW || ch==UPARROW;
}

static void DoKey(EventRecord *theEvent)
{
	long ch;
	WindowPtr wind;
	
/*
	TE32K recognizes these char codes (decimal):
		LeftArrow 	28 
		RightArrow 	29
		UpArrow 	30
		DownArrow 	31
	MacPlus Keyboard virtual codes are (hex):
		LeftArrow 	46
		RightArrow 	42
		UpArrow 	4D
		DownArrow 	48
	MacII codes are:
		LeftArrow 	7B
		RightArrow 	7C
		UpArrow 	7E
		DownArrow 	7D
	Extended codes are:
		63	copy			10 (DLE)
		72	help/insert		05 (ENQ)
		73	home			01 (SOH)
		74	pg up			0B (VT)
		75	del fwrd		7F (DEL)
		76	paste			10
		77	end				04 (EOT)
		78	cut				10
		79	pg down			0C (FF)
		7A	undo			10
*/
	
	wind=FrontLayer();	
	if (theEvent->modifiers&cmdKey) {
		AdjustMenus(wind);	/* Have to adjust before calling MenuKey */
		ch=MenuKey((short)theEvent->message&charCodeMask);
		if(ch&0xFFFF0000) {	/* There is an enabled menu item for this command-key equivalent. */
			DoMenu(ch,wind,theEvent->modifiers);
			while(TickCount() < theEvent->when+6) ;
			HiliteMenu(0);
			return;
		} else if(!isEditKey(theEvent))	/* only cmd-arrow keys get further processing */
			return;
	}
	/* The cut, copy, paste, and undo keys are processed here. */
	
	if((ch=theEvent->message&charCodeMask) == 0x10) {
		switch (theEvent->message&keyCodeMask) {
			case 0x6300:
				if(((DocumentPeek)wind)->fHaveSelection) {
					FlashMenuBar(EDIT_ID);
					(*((DocumentPeek)wind)->doCopy)(wind);
					while(TickCount() < theEvent->when+6) ;
					FlashMenuBar(EDIT_ID);
				}
				return;
			case 0x7600:
				FlashMenuBar(EDIT_ID);
				(*((DocumentPeek)wind)->doPaste)(wind);
				while(TickCount() < theEvent->when+6) ;
				FlashMenuBar(EDIT_ID);
				return;
			case 0x7800:
				if(((DocumentPeek)wind)->fHaveSelection) {
					FlashMenuBar(EDIT_ID);
					(*((DocumentPeek)wind)->doCut)(wind);
					while(TickCount() < theEvent->when+6) ;
					FlashMenuBar(EDIT_ID);
				}
				return;
			case 0x7A00:
				if(((DocumentPeek)wind)->fNeedtoSave) {
					FlashMenuBar(EDIT_ID);
					(*((DocumentPeek)wind)->doUndo)(wind);
					while(TickCount() < theEvent->when+6) ;
					FlashMenuBar(EDIT_ID);
				}
				return;
			default:
				break;
		}
	}
	if(ch==0x0B) 		/* Page up */
		DoPageScroll(((DocumentPeek)wind)->vScroll,-inPageUp);
	else if(ch==0x0C)	/* Page down */
		DoPageScroll(((DocumentPeek)wind)->vScroll,-inPageDown);
	else if(ch==0x01)	/* home */
		Scroll(((DocumentPeek)wind)->vScroll,-32768);
	else if(ch==0x04)	/* end */
		Scroll(((DocumentPeek)wind)->vScroll,32767);
	else {
		if (gMacPlusKBD && theEvent->modifiers&shiftKey) 
			isEditKey(theEvent);
		(*((DocumentPeek)wind)->doKeyDown)(wind,theEvent);
	}
}

void ClearVariableMenu(MenuHandle menuH, int topItem)
{
	int i;
	
	for(i=CountMItems(menuH);i>topItem;i--)
		DelMenuItem(menuH,i);
}

WindowPtr TargetWindow(void)
{
	WindowPeek wind;
	
	if(wind = (WindowPeek)FrontLayer()) {
		for(wind=wind->nextWindow;wind;wind=wind->nextWindow) {
			if(wind->windowKind==9 || wind->windowKind==13)
				return (WindowPtr)wind;
		}
	}
	return 0;
}

void AdjustMenus(WindowPtr wind)
{
	int i,cnt,k;
	DocumentPeek target;
	Str255 itemText;
	
	ClearVariableMenu(WindowMenu,3);
	
	target = (DocumentPeek)TargetWindow();
	for(i=cnt=0;i<MAXWINDS && cnt<NumWindows;i++) {
		if(Documents[i].fDocWindow) {
			cnt++;
			if(((WindowPeek)(Documents+i))->windowKind!=10) {	/* Don't bother with panes */
				k=1;
				if(wind == (WindowPtr)(Documents+i)) {
					*(itemText+k++)='!';
					*(itemText+k++)=checkMark;
				}
				if((Documents+i) == target) {
					*(itemText+k++)='!';
					*(itemText+k++)=0xA5;
				}
				if(Documents[i].fNeedtoSave) {
					*(itemText+k++)='<';
					*(itemText+k++)='U';
				}
				GetWTitle((WindowPtr)(Documents+i),itemText+k);
				p2cstr(itemText+k);
				*itemText=strlen((char*)itemText+1);
				AppendMenu(WindowMenu,itemText);
			}
		}
	}
	CalcMenuSize(WindowMenu);
	
	if(wind) {
		if(((DocumentPeek)wind)->marks) {
			EnableItem(MarkMenu,iMark);
			EnableItem(MarkMenu,iUnmark);
			EnableItem(MarkMenu,iAlphabetical);
		} else {
			EnableItem(MarkMenu,iMark);
			DisableItem(MarkMenu,iUnmark);
			DisableItem(MarkMenu,iAlphabetical);
		}
		EnableItem(MarkMenu,0);
		(*((DocumentPeek)wind)->adjustDocMenus)(wind);
	} else {
		DisableItem(FileMenu,iClose);
		DisableItem(FileMenu,iSave);
		DisableItem(FileMenu,iSaveAs);
		DisableItem(FileMenu,iRevert);
		DisableItem(FileMenu,iPageSetup);
		DisableItem(FileMenu,iPrint);
		EnableItem(FileMenu,0);
		DisableItem(EditMenu,0);
		DisableItem(FindMenu,0);
		DisableItem(MarkMenu,0);
		DisableItem(WindowMenu,0);
		DisableItem(ViewMenu,0);
		DisableItem(ReportsMenu,0);
		EnableItem(DataMenu,iProcess);
		EnableItem(DataMenu,iOptions);
	}
}

static void DoMouse(EventRecord *theEvent)
{
	Point eventPt;
	short windowCode;
	WindowPtr wind;
	
	eventPt=theEvent->where;
	
	windowCode=FindWindow(eventPt,&wind);
	
	switch(windowCode) {
		case inSysWindow:
			SystemClick(theEvent,wind);
			break;
		case inMenuBar:
			wind=FrontLayer();
			AdjustMenus(wind);
			DoMenu(MenuSelect(theEvent->where),wind,theEvent->modifiers);
			HiliteMenu(0);
			break;
		case inGoAway:
			if (TrackGoAway(wind,theEvent->where)) 
				CloseADoc(wind);
			break;
		case inDrag:
			DragWindow(wind,eventPt,&DragBoundsRect);
			break;
		case inGrow:
			(*((DocumentPeek)wind)->doGrow)(wind,theEvent);
			break;
		case inZoomIn:
		case inZoomOut:
			if (TrackBox(wind,theEvent->where,windowCode))	/* point passed by value */
				(*((DocumentPeek)wind)->doZoom)(wind,windowCode);
			break;
		case inContent:
			if(wind!=FrontLayer()) {
				SelectWindow(wind);
				break;
			} else 
				DoContent(wind,theEvent);
			break;
		case inDesk:
			break;
		default:
			break;
	}
	return;
}

void clipboardDestructor(WindowPtr wind)
{
	Str255 clipName;
	
	textDestructor(wind);
	gClipboardDoc = 0;
	GetIndString(clipName,129,3);
	SetItem(EditMenu,iShowClipboard,clipName);
}

void clipboardAdjustDocMenus(WindowPtr)
{
	
	/* File Menu */
	EnableItem(FileMenu,iClose);
	
	DisableItem(FileMenu,iSave);
	DisableItem(FileMenu,iSaveAs);
	DisableItem(FileMenu,iSaveACopy);
	DisableItem(FileMenu,iRevert);
	DisableItem(FileMenu,iPageSetup);
	DisableItem(FileMenu,iPrint);
		
	/* Edit Menu */
	DisableItem(EditMenu,iUndo);
	DisableItem(EditMenu,iCut);
	DisableItem(EditMenu,iCopy);
	DisableItem(EditMenu,iPaste);	
	DisableItem(EditMenu,iClear);
	DisableItem(EditMenu,iSelectAll);
	EnableItem(EditMenu,iShowClipboard);
	DisableItem(EditMenu,iFormat);
	
	DisableItem(EditMenu,iAlign);
	DisableItem(EditMenu,iShiftRight);
	DisableItem(EditMenu,iShiftLeft);
	
	EnableItem(FileMenu,0);
	EnableItem(EditMenu,0);
	DisableItem(FindMenu,0);
	DisableItem(MarkMenu,0);
	EnableItem(WindowMenu,0);
}

void clipbooardDoContent(WindowPtr wind,EventRecord *event)
{
	Rect contents;

	FocusOnWindow(wind);
	GlobalToLocal(&event->where);
	(*((DocumentPeek)wind)->getContentRect)(wind,&contents);
	if(!PtInRect(event->where,&contents))
		ScrollClick(wind,event);
}

void clipboardDoIdle(WindowPtr wind)
{
#ifdef MC68000
#pragma unused(wind)
#endif

	TE32KHandle tH;
		
	if(tH = (TE32KHandle)gClipboardDoc->docData)
		if((**tH).caretState)
			xorCaret(tH);
}

static DocumentPeek OpenClipboardWindow(void)
{
	DocumentPeek doc;
	TE32KHandle tH;
	Rect wRect;
	Str255 clipTitle;
	
	GetIndString(clipTitle,129,5);			/* "Clipboard" */
	if(doc = MakeDoc('TEXT',0)) {
		if(((*doc->makeWindow)((WindowPtr)doc)) && ((*doc->initDoc)((WindowPtr)doc))) {
			/* Read only methods */
			doc->doKeyDown=nullEventMethod;
			doc->doCut=nullWindowMethod;
			doc->doCopy=nullWindowMethod;
			doc->doPaste=nullWindowMethod;
			doc->doClear=nullWindowMethod;
			doc->doUndo=nullWindowMethod;
			doc->writeDocFile=nullIntMethod;
			doc->destructor=clipboardDestructor;
			doc->adjustDocMenus=clipboardAdjustDocMenus;
			doc->doContent=clipbooardDoContent;
			doc->doIdle=clipboardDoIdle;

			/* Make the default a non-wrapping window. */
			tH=(TE32KHandle)doc->docData;
			(**tH).crOnly=true;
			(**tH).wrapToLength=false;
			/* Make it a low, wide window on the bottom of the screen. */
			wRect = DragBoundsRect;	
			InsetRect(&wRect,10,10);
			wRect.top=wRect.bottom-130;
			PositionDocWindow((WindowPtr)doc,&wRect,true);
			
			TE32KPaste(tH);
			SetDocWindowTitle((WindowPtr)doc, clipTitle);
			ShowDocWindow((WindowPtr)doc);
			GetIndString(clipTitle,129,4);
			SetItem(EditMenu,iShowClipboard,clipTitle);
			InitCursor();
		} else {
			DeleteDoc(doc);
			doc=0;
		}
	}
	return doc;
}

void OpenWorksheetDoc(void)
{
	Str255 VolandWSNames;
	OSErr err;
	short vRefNum;
	long response=0;
	
	err = GetVol(VolandWSNames, &vRefNum);
	GetIndString(VolandWSNames,131,1);
	if(Gestalt(gestaltFSAttr,&response)==noErr)
		err = FSMakeFSSpec(vRefNum,0,VolandWSNames,&HomeSpecs);
	else
		err = glueFSMakeFSSpec(vRefNum,0,VolandWSNames,&HomeSpecs);
	err=CloseWD(vRefNum);
	
	DefaultSpecs=HomeSpecs;
	if(gUseWorksheet) {
		if(!(gWorkSheetDoc=(DocumentPeek)InitOldDoc('TEXT',&HomeSpecs)))
			MakeWorksheetDoc(&HomeSpecs);
	}
}

/*------------------------------- Preferences ----------------------------*/

OSErr DoCopyResource(ResType rType, short rID, short source, short dest)
{
	Handle h;
	short attr,ID,current;
	ResType type;
	OSErr resErr;
	Str255 rName;
	
	current = CurResFile();
	UseResFile(source);
	h = Get1Resource(rType,rID);
	if(h) {
		GetResInfo(h,&ID,&type,rName);
		attr = GetResAttrs(h);
		DetachResource(h);
		UseResFile(dest);
		if(ResError() == noErr)
			AddResource(h,rType,rID,rName);
		if(ResError() == noErr)
			SetResAttrs(h,attr);
		if(ResError() == noErr)
			ChangedResource(h);
		if(ResError() == noErr)
			WriteResource(h);
	}
	resErr = ResError();
	ReleaseResource(h);
	UseResFile(current);
	return resErr;
}

short DoCreatePrefsFile(short aVRefNum, long aDirID, StringPtr rName)
{
	Handle h;
	short resNum=-1,ID;
	OSErr resErr;
	ResType rType;
	FInfo fndrInfo;
	
/*	resErr = HCreate(aVRefNum,aDirID,rName,'MRPH','pref'); */
	HCreateResFile(aVRefNum,aDirID,rName);
	if(ResError() == noErr) {
/*/	if(resErr == noErr) { */
		resErr = HGetFInfo(aVRefNum,aDirID,rName,&fndrInfo);
		if(resErr == noErr) {
			fndrInfo.fdType = 'pref';
			fndrInfo.fdCreator = 'MRPH';
			resErr = HSetFInfo(aVRefNum,aDirID,rName,&fndrInfo);
		}
		resNum = HOpenResFile(aVRefNum,aDirID,rName,fsCurPerm);
		if(resNum != -1) {
			UseResFile(gAppResRefNum);
			h = Get1Resource('MPSR',1005);
			if(ResError() == noErr) {
				GetResInfo(h,&ID,&rType,rName);
				resErr = DoCopyResource('MPSR',ID,gAppResRefNum,resNum);
			} else {
				CloseResFile(resNum);
				resErr = HDelete(aVRefNum,aDirID,rName);
				resNum = -1;
			}
		}
	}
	return resNum;
}

void DoReadPrefs(void)
{
	Handle myPrefs;				/* Handle to actual preferences data */
	long myDirID;
	short myVRefNum;
	short myResNum;				/* reference number of opened resource file */
	OSErr myResult;
	Str255 myName;				/* Name of this application */
	
	GetIndString(myName,131,6);
	
	myResult = FindFolder(kOnSystemDisk,kPreferencesFolderType,kDontCreateFolder,&myVRefNum,&myDirID);
	if(myResult != noErr) {
		/*	If there is no Preferences folder, then use the System Folder. If that doesn't work (!!!) 
			use the default volume and directory. */
		myResult = FindFolder(kOnSystemDisk,kSystemFolderType,kDontCreateFolder,&myVRefNum,&myDirID);
		if(myResult != noErr) {
			myVRefNum=0;
			myDirID=0;
		}
	}
	myResNum = HOpenResFile(myVRefNum,myDirID,myName,fsCurPerm);
	if(myResNum == -1)
		myResNum = DoCreatePrefsFile(myVRefNum,myDirID,myName);
	
	gPrefsResRefNum = myResNum;

	if(myResNum != -1) {	/* Successfully opened the file */
		UseResFile(myResNum);
		myPrefs = Get1Resource('MPSR',1005);
		if(!myPrefs)
			return;
		 /* copy prefs from handle here */
		 
		 UseResFile(gAppResRefNum);
	}
}

void DoSavePrefs(TE32KHandle tH)
{
	Handle myPrefsData;		/* handle to new resource */
	Handle myHandle;		/* handle to resource to replace */
	MPSRPtr state;
	short myAttr,current;
	ResType myType;
	short myID;
	Str255 myName;
	
	if(gPrefsResRefNum == -1)
		return;
		
	current = CurResFile();
	
	myPrefsData = NewHandleClear(sizeof(MPSRWindowResource));
	HLock(myPrefsData);
	
	state = (MPSRPtr)*myPrefsData;
	
	state->showInvisibles=(**tH).showInvisibles;
	state->wordWrap=(((**tH).autoIndent)?1:0) | (((**tH).crOnly)?0:2) |(((**tH).wrapToLength)?4:0);
	state->tabWidth=(**tH).tabChars;
	state->fontWidth=(**tH).theCharWidths[' '];
	state->fontSize=(**tH).txSize;

	GetFontName((**tH).txFont,myName);
	strcpy(state->fontName,p2cstr(myName));

	UseResFile(gPrefsResRefNum);
	myHandle = Get1Resource('MPSR',1005);
	if(myHandle) {
		GetResInfo(myHandle,&myID,&myType,myName);
		myAttr = GetResAttrs(myHandle);
		RmveResource(myHandle);
		if(ResError() == noErr) 
			AddResource(myPrefsData,'MPSR',1005,myName);
		if(ResError() == noErr)
			WriteResource(myPrefsData);
	}
	HUnlock(myPrefsData);
	ReleaseResource(myPrefsData);
	
	UseResFile(current);
}

static void getAppPreferences(void)
{	
	DoReadPrefs();
}

/*---------------------------------- main --------------------------------*/

main()
{
	long response=0;
	
	UnloadSeg((Ptr)_DataInit);				/* throw out setup code */

	if(!SetUpWorld())
		ExitToShell();

	getAppPreferences();
	setUpMenus();

    UnloadSeg((Ptr)SetUpWorld);				/* get rid of my initialization code */

	if(Gestalt(gestaltAppleEventsAttr,&response)==noErr) 
		InitAEStuff();
	else {
		OpenWorksheetDoc();
		OpenDocFromFinder();
	}
	eventLoop();
	
}

void minimain(void)
{
	EventRecord minievent;
	short code;	
	
	while (WaitNextEvent(activMask+updateMask+app4Mask+mDownMask,&minievent,gSleepTime,NIL)) {	
		switch (minievent.what) {
			case updateEvt:
				UpdateEvt(&minievent);
				break;
			case activateEvt:
				ActivateEvt(&minievent);
				break;
			case osEvt:
				code=(minievent.message>>24)&0x0FF;
				if (code==suspendResumeMessage) {
					if (minievent.message&resumeFlag)	/* resume event */
						DoResume(&minievent,0);		
					else	/* suspend event */
						DoSuspend(&minievent,0);		/* suspend event*/
				}
				break;
			default:
				SysBeep(2);
		}
	}
}

void OSEvent(EventRecord *event) 
{
	Boolean doConvert;
	unsigned char evType;
	
	evType = (unsigned char)(event->message >>24)&0x0FF;
	
	switch(evType) {
		case mouseMovedMessage:
			AppIdle(FindDoc(FrontLayer()));
			break;
		case suspendResumeMessage:
			doConvert=event->message&2;
			if (event->message&resumeFlag) {
				gInBackground=false;
				DoResume(event,doConvert);
			} else {
				gInBackground=true;
				DoSuspend(event,doConvert);
			}
			break;
		default:
			break;
	}
}

void GetClipFromSystem(void)
{
	OSErr err;
	
	err = TE32KFromScrap();
/*	gClipType = 'TEXT';	*/
}

void GiveClipToSystem(void)
{
	PScrapStuff scrapInfo;
	OSErr err;
	
	err = TE32KToScrap();

	scrapInfo = InfoScrap();
	gLastScrapCount = scrapInfo->scrapCount;
}

Boolean ClipHasChanged(void)
{
	PScrapStuff scrapInfo = InfoScrap();
	return (scrapInfo->scrapCount != gLastScrapCount);
}

void CheckForDASwitch(WindowPtr theFrontWindow)
{
	Boolean DAWindowOnTop;
	
	if(theFrontWindow) {
		DAWindowOnTop = ((WindowPeek)theFrontWindow)->windowKind<0;
		if(DAWindowOnTop != gDAonTop) {
			if(gDAonTop = DAWindowOnTop) 
				GiveClipToSystem();
			else if(ClipHasChanged())
				GetClipFromSystem();
		}
	}
}

/*------------------------------ DoSuspend--------------------------------*/

void DoSuspend(EventRecord *theEvent,Boolean convertClip)
{
	WindowPtr targetWindow;
    
	if(convertClip)
		GiveClipToSystem();
	targetWindow=FrontLayer();
	if(FindDoc(targetWindow)) {	/* Is it a Documents window? */
		theEvent->modifiers &= (!activeFlag);
		DoActivate(targetWindow,theEvent);
	}
}	
		
/*------------------------------ DoResume --------------------------------*/

void DoResume(EventRecord *theEvent, Boolean convertClip) 
{
	WindowPtr targetWindow;
    
	targetWindow=FrontLayer(); 		/* What window is it in?  */
	
	if(convertClip)
		GetClipFromSystem();
	
	if(FindDoc(targetWindow)) {	/* Is it a Documents window? */
		theEvent->modifiers |= activeFlag;
		DoActivate(targetWindow,theEvent);
	}
}

int CloseADoc(WindowPtr wind)
{
	if(((DocumentPeek)wind)->fDocWindow) {	/* Document is active */
		if(DoClose(wind)) {
			(*((DocumentPeek)wind)->destructor)(wind);
			
			AdjustMenus(FrontLayer());
			DrawMenuBar();
			return 1;
		}
		return 0;
	}
	return 1;
}

WindowPtr OpenNamedFile(StringPtr fileName)
{
	FSSpec spec;
	char *p;
	HFileInfo pB;
	OSErr err;
	long response=0;
	
	p=ConditionFileName((char*)fileName);
	c2pstr(p);
	
	if(Gestalt(gestaltFSAttr,&response)==noErr)
		err=FSMakeFSSpec(DefaultSpecs.vRefNum,DefaultSpecs.parID,(StringPtr)p,&spec);
	else
		err=glueFSMakeFSSpec(DefaultSpecs.vRefNum,DefaultSpecs.parID,(StringPtr)p,&spec);

	memset(&pB,0,sizeof(HFileInfo));
	pB.ioVRefNum=spec.vRefNum;
	pB.ioDirID=spec.parID;
	pB.ioNamePtr=spec.name;
	err=PBGetCatInfoSync((CInfoPBPtr)&pB);
	if(err!=noErr)
		return 0;
	if(AcceptableFileType(pB.ioFlFndrInfo.fdType))
		return InitOldDoc(pB.ioFlFndrInfo.fdType,&spec);
	return 0;
}

/*------------------------------------------------------------------------
	Open a file, given an FSSpec. First get and check the type. Called 
	from Open and Print Apple Events.
-------------------------------------------------------------------------*/

WindowPtr OpenOldFile(FSSpec *spec) {
	HFileInfo pB;
	OSErr err;

	memset(&pB,0,sizeof(HFileInfo));
	pB.ioVRefNum=spec->vRefNum;
	pB.ioDirID=spec->parID;
	pB.ioNamePtr=spec->name;
	err=PBGetCatInfoSync((CInfoPBPtr)&pB);
	if(err!=noErr)
		return 0;

	if(AcceptableFileType(pB.ioFlFndrInfo.fdType))
		return InitOldDoc(pB.ioFlFndrInfo.fdType,spec);
	return 0;
}

void OpenOldDoc(int how)
{
	StandardFileReply reply;
	long response=0;
	DocumentPeek doc;

	InitCursor();
	if(Gestalt(gestaltStandardFileAttr,&response)==noErr)
		StandardGetFile(NIL,(how)?-1:NumFileTypes,MyFileTypes,&reply);
	else
		glueStandardGetFile(NIL,(how)?-1:NumFileTypes,MyFileTypes,&reply);
	if(reply.sfGood) {
		if(doc=(DocumentPeek)InitOldDoc((how)?'TEXT':reply.sfType,&reply.sfFile)) {
			if(how) {
				/* Read only if option key held down */
				doc->doKeyDown=nullEventMethod;
				doc->doCut=nullWindowMethod;
				doc->doCopy=nullWindowMethod;
				doc->doPaste=nullWindowMethod;
				doc->doClear=nullWindowMethod;
				doc->doUndo=nullWindowMethod;
				doc->writeDocFile=nullIntMethod;
			}
		}
	}
}

WindowPtr InitOldDoc(OSType type, FSSpecPtr fileInfo)
{
	DocumentPeek doc;
	int i,n;
	Rect r;
	
	for(i=n=0;n<NumWindows;i++) {
		doc=Documents+i;
		if(doc->fDocWindow) {
			n++;
			if(doc->fDocType==type && doc->fileSpecs.vRefNum==fileInfo->vRefNum
					&& doc->fileSpecs.parID==fileInfo->parID
					&& *doc->fileSpecs.name==*fileInfo->name
					&& !strncmp(doc->fileSpecs.name+1,fileInfo->name+1,*fileInfo->name))
				return (WindowPtr)doc;
		}
	}
	if(doc = (DocumentPeek)MakeDoc(type,fileInfo)) {
		if(((*doc->makeWindow)((WindowPtr)doc)) && ((*doc->initDoc)((WindowPtr)doc))) {
			if(OpenDocFile((WindowPtr)doc)) {
				if((*doc->readDocFile)((WindowPtr)doc)) {
					if(doc->resourcePathRefNum) {
						if(doc->windowState) {
							r = ((MPSRPtr)*doc->windowState)->userState;
							r.top-=20;
							PositionDocWindow((WindowPtr)doc,&r,true);
						}
					}
					ShowDocWindow((WindowPtr)doc);
					if(doc->marks) 
						FillMarkMenu(doc);
				} else {
					CloseDocFile(doc);
					DeleteDoc(doc);
					return 0;
				}
			} else {
				DeleteDoc(doc);
				return 0;
			}
		} else {
			DeleteDoc(doc);
			doc=0;
		}
	}
	return (WindowPtr)doc;
}

Boolean OpenDocFromFinder(void)
{
	Boolean fileOpened = false;
#ifdef MC68000
	short message;
	short count,i;
	AppFile theApp;
	FSSpec spec;
	Boolean hasFSSpec;
	OSErr err;
	long response=0;
	
	CountAppFiles(&message,&count);
	if(!count)
		return false;
		
	hasFSSpec=(Gestalt(gestaltFSAttr,&response)==noErr);
	for(i=count;i;i--) {
		GetAppFiles(i,&theApp);
		if(hasFSSpec)
			err=FSMakeFSSpec(theApp.vRefNum,0,theApp.fName,&spec);
		else
			err=glueFSMakeFSSpec(theApp.vRefNum,0,theApp.fName,&spec);
		
		if(AcceptableFileType(theApp.fType))
			if(InitOldDoc(theApp.fType,&spec))
				fileOpened = true;
	}
#endif
	return fileOpened;
}

Boolean AcceptableFileType(OSType theType)
{
	OSType *theTypeList;
	int i;
	
	theTypeList = MyFileTypes;
	
	if((NumFileTypes == 0) || (theTypeList == nil))
		return true;
	
	for(i=0; i<NumFileTypes; i++) {
		if(theType == *theTypeList++)
			return true;
	}
	return false;
}


void OpenNewDoc(void)
{
	DocumentPeek newDoc;
	FSSpec spec;
		
	spec=DefaultSpecs;
	GetIndString(spec.name,131,3);

	if(newDoc = MakeDoc('TEXT',&spec)) {
		if(((*newDoc->makeWindow)((WindowPtr)newDoc)) && ((*newDoc->initDoc)((WindowPtr)newDoc))) {
			ReadDefaultResourceFork((TE32KHandle)newDoc->docData);
			/* Now make the data entry doc */
			AdjustScrollBars((WindowPtr)newDoc);
			SetScrollBarValues((WindowPtr)newDoc);
			newDoc->fNeedtoSaveAs=true;
/*========= (*newDoc->newState)((WindowPtr)newDoc);	*/
			ShowDocWindow((WindowPtr)newDoc);
		} else 
			DeleteDoc(newDoc);
	}
}

void MakeWorksheetDoc(FSSpecPtr specs)
{
	if(!gWorkSheetDoc) {
		if(gWorkSheetDoc = MakeDoc('TEXT',specs)) {
			if( (*gWorkSheetDoc->makeWindow)((WindowPtr)gWorkSheetDoc) &&
					(*gWorkSheetDoc->initDoc)((WindowPtr)gWorkSheetDoc)) {
				ShowDocWindow((WindowPtr)gWorkSheetDoc);
				gWorkSheetDoc->fNeedtoSaveAs=false;
				gWorkSheetDoc->fNeedtoSave=false;
			} else {
				DeleteDoc(gWorkSheetDoc);
				gWorkSheetDoc=0;
			}
		}
	}
}

#ifdef MC68000
#pragma segment process
#endif

void TempToDoc(DocumentPeek doc)
{
	TE32KHandle tH;
	Handle bufH;
	int len,got;
	
	/* OutputFlag is 0, -10, -20, ... when output is stdout. */
	if(!(OutputFlag%10) && OutFile!=stdout) {
		fflush(OutFile);
		if(len=ftell(OutFile)) {
			if(!doc)
				doc=gWorkSheetDoc;
			if(doc) {
				tH = (TE32KHandle)doc->docData;
				bufH=NewHandle((len>1024)?1024:len);
				if(bufH) {
					HLock(bufH);
					rewind(OutFile);
					while(len>1024) {
						if((got=fread(*bufH,1024,1,OutFile))==1) {
							len-=1024;
							TE32KInsert((char*)*bufH,1024,tH);
							AdjustScrollBars((WindowPtr)doc);
							SetScrollBarValues((WindowPtr)doc);
							TE32KSelView(tH);

						} else
							break;
					}
					if(len<1024 && (got=fread(*bufH,len,1,OutFile))==1) {
						TE32KInsert(*bufH,len,tH);
						AdjustScrollBars((WindowPtr)doc);
						SetScrollBarValues((WindowPtr)doc);
						TE32KSelView(tH);
					}
					
					doc->fNeedtoSave = true;
					HUnlock(bufH);
					DisposeHandle(bufH);
				}
			}
		}
		rewind(OutFile);
	}
}

Boolean doProcessFilesDialog(StringPtr commandLine)
{
	DialogPtr pfDialog;
	short itemHit;
	short itemType;
	Handle itemHand;
	int pau=false,k=0;
	Rect box;
	Boolean res=false;
	
	pfDialog=GetNewDialog(rProcessFilesDialog,(Ptr)&DlogStor,(WindowPtr)(-1));

	if(!pfDialog) 
		FatalError();

	while(!pau) {
		
		GetDItem(pfDialog,5,&itemType,&itemHand,&box);
		SetDItem(pfDialog,5,itemType,(Handle)doButton,&box);
		
		ModalDialog(SimpleDialogFilter,&itemHit);

		switch(itemHit) {
			case 1:
				pau=true;
				GetDItem(pfDialog,3,&itemType,&itemHand,&box);
				GetIText(itemHand,commandLine);
				p2cstr(commandLine);
				res=true;
				break;
			case 2:
				*commandLine='\0';
				pau=true;
				break;
			default:
				break;
		}
	}
	CloseDialog(pfDialog);
	return res;
}

void doOptionsDialog(void)
{
	DialogPtr optionsDialog;
	short itemHit,itemType;
	Handle itemHand;
	int pau=false;
	Rect box;
	char *p;

	short hiliteLevel;
	short processingLevel,processingFlagBits,logMethod,logBits,graphicsMethod;
	short textOutputFlagBits;
	
	optionsDialog=GetNewDialog(rOptionsDialog,(Ptr)&DlogStor,(WindowPtr)(-1));

	if(!optionsDialog) 
		FatalError();
	
	processingFlagBits = gProcessingFlagBits;
	processingLevel = gProcessingFlagBits&ProcessingLevelMask;
	
	textOutputFlagBits = gTextOutputFlagBits;
	logBits = logMethod = gTextOutputFlagBits&LogMethodsMask;
	graphicsMethod = gTextOutputFlagBits&GraphicsMethodsMask;
	
	while(!pau) {
		/* Initialize the radio buttons */
		GetDItem(optionsDialog,4,&itemType,&itemHand,&box);
		SetCtlValue((ControlHandle)itemHand,gSelectedBooks==0);
		GetDItem(optionsDialog,5,&itemType,&itemHand,&box);
		SetCtlValue((ControlHandle)itemHand,gSelectedBooks==1);
		
		if(!gSelectedBooks) {
			GetDItem(optionsDialog,6,&itemType,&itemHand,&box);
			SetIText(itemHand,(StringPtr)"\p");
		}
		
		GetDItem(optionsDialog,7,&itemType,&itemHand,&box);
		SetCtlValue((ControlHandle)itemHand,processingLevel==0);
		GetDItem(optionsDialog,8,&itemType,&itemHand,&box);
		SetCtlValue((ControlHandle)itemHand,processingLevel==1);
		GetDItem(optionsDialog,9,&itemType,&itemHand,&box);
		SetCtlValue((ControlHandle)itemHand,processingLevel==2);
		GetDItem(optionsDialog,10,&itemType,&itemHand,&box);
		SetCtlValue((ControlHandle)itemHand,processingLevel==3);

		/* Do the check boxes */
		GetDItem(optionsDialog,11,&itemType,&itemHand,&box);
		SetCtlValue((ControlHandle)itemHand,processingFlagBits&WarningsFlagBit);
		GetDItem(optionsDialog,12,&itemType,&itemHand,&box);
		SetCtlValue((ControlHandle)itemHand,processingFlagBits&ErrorAbortFlagBit);
		
		if(!(processingFlagBits&ErrorAbortFlagBit)) {
			GetDItem(optionsDialog,13,&itemType,&itemHand,&box);
			SetIText(itemHand,(StringPtr)"\p");
		}
		
		GetDItem(optionsDialog,14,&itemType,&itemHand,&box);
		SetCtlValue((ControlHandle)itemHand,!(processingFlagBits&ProcessPeople));

		GetDItem(optionsDialog,18,&itemType,&itemHand,&box);	/* Write reports to disk */
		
		hiliteLevel = (processingLevel==3)? 255:0;
		
		GetDItem(optionsDialog,18,&itemType,&itemHand,&box);		/* Write Reports to disk */
		HiliteControl((ControlHandle)itemHand,hiliteLevel);
		SetCtlValue((ControlHandle)itemHand,textOutputFlagBits&DiskOutputFlagBit);
		
		hiliteLevel = (hiliteLevel || !(textOutputFlagBits&DiskOutputFlagBit))? 255:0;
		
		GetDItem(optionsDialog,19,&itemType,&itemHand,&box);		/* Schematics */
		HiliteControl((ControlHandle)itemHand,hiliteLevel);
		SetCtlValue((ControlHandle)itemHand,logMethod);

		GetDItem(optionsDialog,20,&itemType,&itemHand,&box);		/* Coordinates */
		HiliteControl((ControlHandle)itemHand,hiliteLevel);
		SetCtlValue((ControlHandle)itemHand,textOutputFlagBits&OutputCoordinates);

		GetDItem(optionsDialog,21,&itemType,&itemHand,&box);		/* Closure Stats */
		HiliteControl((ControlHandle)itemHand,hiliteLevel);
		SetCtlValue((ControlHandle)itemHand,textOutputFlagBits&OutputClosureStats);

		GetDItem(optionsDialog,17,&itemType,&itemHand,&box);		/* Full listing */
		HiliteControl((ControlHandle)itemHand,hiliteLevel);
		SetCtlValue((ControlHandle)itemHand,textOutputFlagBits&OutputPrettyPrint);

		GetDItem(optionsDialog,22,&itemType,&itemHand,&box);		/* Attributes */
		HiliteControl((ControlHandle)itemHand,hiliteLevel);
		SetCtlValue((ControlHandle)itemHand,textOutputFlagBits&OutputAttributes);

		GetDItem(optionsDialog,23,&itemType,&itemHand,&box);		/* Postscript */
		HiliteControl((ControlHandle)itemHand,hiliteLevel);
		SetCtlValue((ControlHandle)itemHand,(textOutputFlagBits&GraphicsMethodsMask)==PostscriptFlag);

		GetDItem(optionsDialog,24,&itemType,&itemHand,&box);		/* Super 3d */
		HiliteControl((ControlHandle)itemHand,hiliteLevel);
		SetCtlValue((ControlHandle)itemHand,(textOutputFlagBits&GraphicsMethodsMask)==Super3DFlag);

		GetDItem(optionsDialog,25,&itemType,&itemHand,&box);		/* DXF */
		HiliteControl((ControlHandle)itemHand,hiliteLevel);
		SetCtlValue((ControlHandle)itemHand,(textOutputFlagBits&GraphicsMethodsMask)==DXFFlag);
		
		GetDItem(optionsDialog,28,&itemType,&itemHand,&box);
		SetDItem(optionsDialog,28,itemType,(Handle)doLine,&box);
		
		GetDItem(optionsDialog,29,&itemType,&itemHand,&box);
		SetDItem(optionsDialog,29,itemType,(Handle)doFrame,&box);
		
		GetDItem(optionsDialog,30,&itemType,&itemHand,&box);
		SetDItem(optionsDialog,30,itemType,(Handle)doButton,&box);
		
		ModalDialog(SimpleDialogFilter,&itemHit);

		switch(itemHit) {
			case 1:
				pau=true;
				gProcessingFlagBits=processingFlagBits&0xFFF8;
				gProcessingFlagBits|=processingLevel;
				gTextOutputFlagBits=textOutputFlagBits&0xFFE0;
				gTextOutputFlagBits|=(logMethod|graphicsMethod);
				p=(char*)gCommandLine+4;	/* Global begins with "CML" */
				*p='\0';			/* truncate previous commands */
				if(gSelectedBooks) {
					strcpy(p,"-i ");
					GetDItem(optionsDialog,6,&itemType,&itemHand,&box);
					GetIText(itemHand,(StringPtr)p+3);
					p2cstr((StringPtr)p+3);
					strcat((char*)gCommandLine," ");
				}
				if(processingFlagBits&ErrorAbortFlagBit) {
					strcat(p,"-er ");
					itemHit = strlen(p);		/* temp use of this short */
					GetDItem(optionsDialog,13,&itemType,&itemHand,&box);
					GetIText(itemHand,(StringPtr)p+itemHit);
					p2cstr((StringPtr)p+itemHit);
					if(strcmp(p+itemHit,"10")) {
						strcat((char*)gCommandLine," ");
					} else
						*(p+itemHit-4) = '\0';
				} else
					strcat(p,"-er 0 ");
				break;
			case 2:			/* Cancel */
				pau=true;
				break;
			case 3:			/* More Options� */
				break;
			case 4:			/* All or */
				gSelectedBooks=0;
				break;
			case 5:			/* Selected books: */
			case 6:			/* Edit text for books expression */
				gSelectedBooks=1;
				break;
			case 7:			/* No coordinates */
				processingLevel=0;
				break;
			case 8:			/* Calculate coordinates */
				processingLevel=1;
				break;
			case 9:			/* Close loops */
				processingLevel=2;
				break;
			case 10:		/* Special processing */
				processingLevel=3;
				break;
			case 11:		/* Report warnings */
				processingFlagBits ^= WarningsFlagBit;
				break;
			case 12:		/* Abort after */
				processingFlagBits ^= ErrorAbortFlagBit;
				break;
			case 13:
				processingFlagBits |= ErrorAbortFlagBit;
				break;
			case 14:		/* No log */
				processingFlagBits ^= ProcessPeople;
				break;
			case 15:		/* SEF */
			case 16:		/* SMAPS Ascii */
				break;
			case 17:		/* Full Listing */
				textOutputFlagBits ^= OutputPrettyPrint;
				break;
			case 18:		/* Write Reports to disk */
				textOutputFlagBits ^= DiskOutputFlagBit;
				break;
			case 19:		/* Schematics */
				logBits = (logBits)? logBits:1;		/* Minimum to be output */
				logMethod = (logMethod^logBits)&logBits;
				break;
			case 20:		/* Coordinates */
				textOutputFlagBits ^= OutputCoordinates;
				break;
			case 21:		/* Closure stats */
				textOutputFlagBits ^= OutputClosureStats;
				break;
			case 22:		/* Attributes */
				textOutputFlagBits ^= OutputAttributes;
				break;
			case 23:		/* Postscript */
			case 24:		/* Super 3D */
			case 25:		/* DXF */
				break;			
			default:
				break;
		}
	}
	CloseDialog(optionsDialog);
}

void ProcessWindow(WindowPtr wind, short how)
{
	TE32KHandle tH;
	Str255 title;
	
	if(wind) {
		if(tH = (TE32KHandle)((DocumentPeek)wind)->docData) {
			GetDocWindowTitle(wind,title);
			QuoteName(p2cstr(title));
			ConcatOptions((char*)title);
			if(how) {
				strcat((char*)gCommandLine,"\n");
				PrintBuf(gCommandLine,strlen((char*)gCommandLine),0);
			} else {
				HLock((**tH).hText);
				Main((char *)*(**tH).hText,(**tH).teLength,(char*)gCommandLine);
				HUnlock((**tH).hText);
				UnloadSeg(Main);
			}
			if(gWorkSheetDoc) {
				tH = (TE32KHandle)gWorkSheetDoc->docData;
				TE32KSelView(tH);
				AdjustScrollBars((WindowPtr)gWorkSheetDoc);
				SetScrollBarValues((WindowPtr)gWorkSheetDoc);
			}
		}
	}
}

char *QuoteName(char *name)
{
	char *s;
	short quote;
	char buf[256];
	
	s=name;
	for(quote=0;!quote && *s;s++)
		quote = isspace(*s);
	if(quote) {
		strcpy(buf,name);
		strcpy(name,"\"");
		strcat(name,buf);
		strcat(name,"\"");
	}
	return name;
}

void WindowMenuSelect(short docNumber)
{
	short i,cnt;
	
	for(i=cnt=0;i<MAXWINDS;i++) {
		if(Documents[i].fDocWindow) {
			if(++cnt == docNumber) {
				if((WindowPtr)(Documents+i) != FrontLayer())
					SelectWindow((WindowPtr)(Documents+i));
				return;
			}
		}
	}
}

char *ConcatOptions(char *inputList)
{
	strcpy((char *)gCommandLine,"CML ");
	
	switch(gProcessingFlagBits&ProcessingLevelMask) {
		case 0:
			strcat((char*)gCommandLine,"-nop2 ");
			break;
		case 2:
			strcat((char*)gCommandLine,"-c ");
			break;
		case 3:
			strcat((char*)gCommandLine,"-sef ");
			return strcat((char*)gCommandLine,inputList);
			break;
		default:
			break;
	}
	if(gProcessingFlagBits&ProcessDimensions)
		strcat((char*)gCommandLine,"-d ");
	if(gProcessingFlagBits&(ProcessPeople|ProcessAttributes))
		strcat((char*)gCommandLine,"-p ");
	else if(gProcessingFlagBits&ProcessHeadersFlagBit)
		strcat((char*)gCommandLine,"-h ");
	if(!(gProcessingFlagBits&WarningsFlagBit))
		strcat((char*)gCommandLine,"-now ");
	if(gProcessingFlagBits&SnapShotFlagBit)
		strcat((char*)gCommandLine,"-sb0 ");
	if(gTextOutputFlagBits&DiskOutputFlagBit) {
		strcat((char*)gCommandLine,"-o ");
		if(!(gTextOutputFlagBits&LogMethodsMask))
			strcat((char*)gCommandLine,"-nolog ");
		if(!(gTextOutputFlagBits&OutputCoordinates))
			strcat((char*)gCommandLine,"-nocart ");
		if(gTextOutputFlagBits&OutputPrettyPrint)
			strcat((char*)gCommandLine,"-pp ");
	} else
		strcat((char*)gCommandLine,"-0 -s ");
	return strcat((char*)gCommandLine,inputList);
}

/*	For apps having non-text windows, PositionDocWindow should be a member function
	and share code with doGrow. */
void PositionDocWindow(WindowPtr wind, Rect *newSize,Boolean makeFront)
{
	if(((DocumentPeek)wind)->docData) {
		MoveWindow(wind,newSize->left,newSize->top+20,makeFront);
		SizeDocWindow(wind,newSize);
	}
}

void SizeDocWindow(WindowPtr wind, Rect *newSize)
{
	Rect r;
	TE32KHandle tH;

	if(((DocumentPeek)wind)->docData) {
				
		FocusOnWindow(wind);
		r = wind->portRect;
		r.left = r.right-15;
		InvalRect(&r);
		
		r = wind->portRect;	
		r.top = r.bottom-15;
		InvalRect(&r);
		
		if(newSize->right>((DocumentPeek)wind)->limitRect.right)
			newSize->right=((DocumentPeek)wind)->limitRect.right;
		if(newSize->right>DragBoundsRect.right)
			newSize->right=DragBoundsRect.right;
		
		if(newSize->bottom>((DocumentPeek)wind)->limitRect.bottom)
			newSize->bottom=((DocumentPeek)wind)->limitRect.bottom;
		if(newSize->bottom>DragBoundsRect.bottom)
			newSize->bottom=DragBoundsRect.bottom;
		
		SizeWindow(wind,newSize->right-newSize->left,newSize->bottom-newSize->top-20,false);

		r = wind->portRect;
		r.left = r.right-15;
		EraseRect(&r);
		InvalRect(&r);
		
		r = wind->portRect;
		r.top = r.bottom-15;
		EraseRect(&r);
		InvalRect(&r);
		
		SizeScrollBars(wind);
#if 0
		if(!makeFront) {
			if(((DocumentPeek)wind)->vScroll)
				HideControl(((DocumentPeek)wind)->vScroll);
			if(((DocumentPeek)wind)->hScroll)
				HideControl(((DocumentPeek)wind)->hScroll);
		}
#endif
		if(SetTERect(wind)) {
			TE32KCalText(tH=(TE32KHandle)((DocumentPeek)wind)->docData);
			TE32KSetSelect((**tH).selStart,(**tH).selEnd,tH);
		}
		AdjustScrollBars(wind);
		SetScrollBarValues(wind);
		DrawGrowIcon(wind);
		GetContentRect(wind,&r);
		InvalRect(&r);
		EraseRect(&r);
	}
	return;
}

void TileWindows(void)
{
	int i,j,nWinds,nRows,nCols;
	short wWidth,wHeight;
	Rect base,wRect;
	DocumentPeek doc;
	WindowPtr front;
	
	front=0;
	for(nWinds=i=0;i<MAXWINDS;i++) {
		if((Documents+i)!=gWorkSheetDoc && Documents[i].docData) {
			if(!front)
				front=(WindowPtr)&Documents[i];
			nWinds++;
		}
	}
	if(nWinds) {
		nCols=0;
		do {
			nRows = nWinds/++nCols;
		} while(nRows>nCols+2);
		
		wWidth=(DragBoundsRect.right-DragBoundsRect.left-(nCols-1)*4)/nCols;
		wHeight=(DragBoundsRect.bottom-DragBoundsRect.top-4-(nRows-1)*4)/nRows;
		base=DragBoundsRect;
		base.top+=4;
		base.right=base.left+wWidth;
		base.bottom=base.top+wHeight;
		doc=&Documents[0];
		for(i=0;i<nCols;i++) {
			for(j=0;j<nRows;j++) {
				wRect=base;
				OffsetRect(&wRect,(wWidth+4)*i,(wHeight+4)*j);
				while(doc<&Documents[MAXWINDS-1] && (!doc->docData || doc==gWorkSheetDoc))
					doc++;
				PositionDocWindow((WindowPtr)doc,&wRect,!front);
				doc++;
			}
		}
		if(front)
			SelectWindow(front);
	}
}

void StackWindows(void)
{
	int i,nWinds;
	short shift;
	Rect base,wRect;
	WindowPtr wind;
	
	for(nWinds=0,i=1;i<MAXWINDS;i++) {
		if(Documents[i].docData) 
			nWinds++;
	}
	if(gWorkSheetDoc)
		nWinds--;
	base=DragBoundsRect;
	base.top+=2;
	shift=10*nWinds;
	base.right-=shift;
	base.bottom-=shift;
	
	for(wind=FrontLayer();wind;
		wind=(WindowPtr)((WindowPeek)wind)->nextWindow) {
		if(wind!=(WindowPtr)gWorkSheetDoc) {
			shift=10*nWinds;
			wRect=base;
			OffsetRect(&wRect,shift,shift);
			wRect.top=base.top+20*nWinds;
			PositionDocWindow(wind,&wRect,false);
			nWinds--;
		}
	}
}

/*----------------------- File I/O glues -----------------------------*/

#ifdef Sys6Glues

#ifdef MC68000
#pragma segment glues
#endif

pascal OSErr glueFSMakeFSSpec(short volRefNum,long dirID,
						ConstStr255Param fileName,FSSpecPtr spec)
{
	OSErr err;
	short newVolRefNum;
	long newDirID;
	HFileInfo pB;
	Str255 temp;
	char *p;

	err=GetWDInfo(volRefNum,&newVolRefNum,&newDirID,0);
	if(err==noErr) {
		dirID=(dirID)?dirID:newDirID;
		volRefNum=newVolRefNum;
	}
	
	if(*fileName) {
		BlockMove(fileName,temp,*fileName+1);
		*(temp+*fileName+1)='\0';
		if(isfullpath(temp+1)) {
			err=GetVolRefNum(temp,&volRefNum);
			if(err!=noErr) {
				BlockMove(temp,spec->name,*temp+1);
				return err;
			}
			BlockMove(fileName,temp,*fileName+1);
			*(temp+*fileName+1)='\0';
		}
	} else
		*temp=0;
	
	memset(&pB,0,sizeof(HFileInfo));
	pB.ioNamePtr=temp;
	pB.ioDirID=dirID;
	pB.ioVRefNum=volRefNum;

	/*  PBGetCatInfo with pB.ioFDirIndex==0 doesn't work if there
		is no file name. */
	if(!*temp) 
		pB.ioFDirIndex=-1;

	err=PBGetCatInfoSync((CInfoPBPtr)&pB);
	spec->vRefNum=pB.ioVRefNum;
	spec->parID=pB.ioFlParID;
	BlockMove(temp,spec->name,*temp+1);
	if(err==noErr) {
		if(*temp) {
			/*	Whether a file or a directory, the name is incorect */
			if(pB.ioFlAttrib&16) { /* We have a directory but not a directory name */
				pB.ioFDirIndex=-1;
				*temp=0;
				err=PBGetCatInfoSync((CInfoPBPtr)&pB);
				BlockMove(temp,spec->name,*temp+1);
			} else {
				p=strrchr(temp+1,':');
				p=(p)? p+1:temp+1;
				*spec->name=strlen(p);
				BlockMove(p,spec->name+1,*spec->name);
			}
		}
	}
	return err;
}

pascal void glueStandardGetFile(FileFilterProcPtr ffpp, 
		short numTypes, SFTypeList typeList, StandardFileReply *reply)
{
#ifdef MC68000
#pragma unused (ffpp)
#endif
	SFReply oldReply;
	FInfo finderInfo;
	Point p;
	OSErr err;
	
	p.h = (DragBoundsRect.right-DragBoundsRect.left-344)/2; 
	p.v = DragBoundsRect.top+60;
	oldReply.good = FALSE;
	oldReply.fName[0]= 0;
	
	SFGetFile(p, NIL, NIL, numTypes, typeList, NIL, &oldReply);
	reply->sfGood=oldReply.good;
	reply->sfReplacing=false;
	reply->sfType=oldReply.fType;
	reply->sfScript=-1;
	reply->sfIsFolder=false;
	reply->sfIsVolume=false;
	reply->sfReserved1=0;
	reply->sfReserved2=0;
	if(oldReply.good) {
		err=GetWDInfo(oldReply.vRefNum,&reply->sfFile.vRefNum,&reply->sfFile.parID,0);
		if(err==noErr) {
			strncpy(reply->sfFile.name+1,oldReply.fName+1,oldReply.fName[0]);
			reply->sfFile.name[0]=oldReply.fName[0];
		}
		err=GetFInfo(oldReply.fName,oldReply.vRefNum,&finderInfo);
		reply->sfFlags=finderInfo.fdFlags;
		err=CloseWD(oldReply.vRefNum);
	}
}

pascal void glueStandardPutFile(ConstStr255Param prompt,
          ConstStr255Param defaultName, StandardFileReply *reply)
{
	SFReply oldReply;
	Point p;
	OSErr err;
		
	p.h = (DragBoundsRect.right-DragBoundsRect.left-344)/2; 
	p.v = DragBoundsRect.top+60;
	SFPutFile(p,prompt,defaultName,(DlgHookProcPtr)nil,&oldReply);
	reply->sfGood=oldReply.good;
	if(!oldReply.good) {
		/* the user canceled the SaveAs */
		return;
	}
	reply->sfReplacing=false;
	reply->sfScript=-1;
	reply->sfIsFolder=false;
	reply->sfIsVolume=false;
	reply->sfReserved1=false;
	reply->sfReserved2=false;
	err=GetWDInfo(oldReply.vRefNum,&reply->sfFile.vRefNum,&reply->sfFile.parID,0);
	if(err==noErr) {
		strncpy(reply->sfFile.name+1,oldReply.fName+1,oldReply.fName[0]);
		reply->sfFile.name[0]=oldReply.fName[0];
	}
	err=CloseWD(oldReply.vRefNum);
}

#endif

#ifdef MC68000
#pragma segment marks
#endif

MarkRecPtr GetIndMark(Ptr mark, short index)
{
	short nMarks,i;
	
	nMarks = *((short *) mark);
	mark += 2;
	for(i=0;i<index && i<nMarks; i++) 
		 mark+=sizeofMark(mark);
	return (i==index)? (MarkRecPtr)mark : 0;
}

Ptr switchMarks(MarkRecPtr m0,MarkRecPtr m1)
{
	Str255 temp;
	Ptr m;
	
	m=(Ptr)m1;
	m+=m1->label-m0->label;
	BlockMove(m0,temp,m0->label+9);
	BlockMove(m1,m0,m1->label+9);
	BlockMove(temp,m,9+((MarkRecPtr)temp)->label);
	return m;
}

int ConfirmDialog(short strID,StringPtr message)
{
	DialogPtr confirmDialog;
	short itemHit,itemType;
	Handle itemHand;
	Rect box;
	Str255 buf,a;
	
	GetIndString(buf,128,strID);
	*(buf+*buf+1)=0;
	*a=sprintf((char*)a+1,(char*)buf+1,message+1);

	confirmDialog=GetNewDialog(rConfirmDialog,(Ptr)&AuxDlogStor,(WindowPtr)(-1));

	if(!confirmDialog) 
		FatalError();
	GetDItem(confirmDialog,3,&itemType,&itemHand,&box);
	SetIText(itemHand,a);
	GetDItem(confirmDialog,4,&itemType,&itemHand,&box);
	SetDItem(confirmDialog,4,itemType,(Handle)doButton,&box);
		
	for(;;) {
		ModalDialog(DialogStandardFilter,&itemHit);
		
		if(itemHit==1) {				/* Okay */
			CloseDialog(confirmDialog);
			return 1;
		} else if(itemHit==2) {
			CloseDialog(confirmDialog);
			return 0;
		}
	}
}

/* nameRequested is a c string */

int RequestDialog(short strID,char *nameRequested)
{
	DialogPtr requestDialog;
	short itemHit,itemType;
	Handle itemHand;
	Rect box;
	char *p;
	Str255 message;

	requestDialog=GetNewDialog(rRequestDialog,(Ptr)&DlogStor,(WindowPtr)(-1));

	if(!requestDialog) 
		FatalError();
		
	/*	Truncate the selection to one line and make it less than 63 chars. */
	for(p=nameRequested;*p && *p!='\n';p++) ;
	*p='\0';
	c2pstr(nameRequested);
	if(*nameRequested>63)
		*nameRequested=63;
	*(nameRequested+*nameRequested+1)=0;
	
	GetDItem(requestDialog,3,&itemType,&itemHand,&box);
	GetIndString(message,128,strID);	
	SetIText(itemHand,message);
	GetDItem(requestDialog,4,&itemType,&itemHand,&box);
	SetIText(itemHand,nameRequested);
	SelIText(requestDialog,4,0,32767);
	GetDItem(requestDialog,5,&itemType,&itemHand,&box);
	SetDItem(requestDialog,5,itemType,(Handle)doButton,&box);
	
	for(;;) {
		
		ModalDialog(DialogStandardFilter,&itemHit);

		if(itemHit==1) {					/* Okay */
			GetDItem(requestDialog,4,&itemType,&itemHand,&box);
			GetIText(itemHand,nameRequested);
			if(*nameRequested>63)
				*nameRequested=63;
			CloseDialog(requestDialog);
			return 1;
		} else if(itemHit==2) {				/* Cancel */
			CloseDialog(requestDialog);
			return 0;
		}
	}
}

void sortMarks(DocumentPeek doc, Boolean how)
{
	Ptr mark,firstMark;
	short i,nMarks;
	Boolean changed;
	MarkRecPtr m0,m1;
	
	if(doc->marks) {
		HLock(doc->marks);
		mark = *doc->marks;
		nMarks = *((short *)mark);
		firstMark=mark+2;
		/* This is a unidirectional bubble sort. */
		do {
			mark=firstMark;
			m1=(MarkRecPtr)mark;
			for(nMarks--,changed=i=0;i<nMarks;i++) {
				m0=m1;
				mark+=9+m1->label;
				m1=(MarkRecPtr)mark;
				if((how)?	m0->selStart>m1->selStart :
							IUCompString((StringPtr)&m0->label,(StringPtr)&m1->label)>0) {
					mark=switchMarks(m0,m1);
					m1=(MarkRecPtr)mark;
					changed=true;
				}
			}
		} while(changed);
		HUnlock(doc->marks);
		FillMarkMenu(doc);
	}
}

void InsertMark(DocumentPeek doc,int selStart,int selEnd,char *p)
{
	Ptr mark,firstMark;
	short i,nMarks,itemType;
	OSErr err;
	MarkRecPtr m0;
	int whereInMarks,oldSize,newSize;
	
	/*	p is the tentative mark name and is a c string when passed,
		but is turned into a pstring by RequestDialog. */
		
	if(RequestDialog(1,p)) {
		if(doc->marks) {
			HLock(doc->marks);
			mark = *doc->marks;
			nMarks = *((short *)mark);
			firstMark=mark+2;
			i=*p;
			if(!(i%2)) {
				(*p)++;
				p[*p]='\0';
			}
			whereInMarks=Munger(doc->marks,2,p,*p+1,0,0);
			if(whereInMarks>=0) {		/* This mark already exists */
				*p=i;
				if(ConfirmDialog(2,p)) {
					*p+=(i%2)? 0:1;
					m0 = (MarkRecPtr)(mark+whereInMarks-8);
					m0->selStart=selStart;
					m0->selEnd=selEnd;
				}
				return;
			}
			/* Insert a new mark. Put it at beginning of new Handle so the bubble sort will
				position it in one iteration. */
			HUnlock(doc->marks);
			oldSize=GetHandleSize(doc->marks);
			newSize=*p+(((*p)%2)? 9:10);
			SetHandleSize(doc->marks,oldSize+newSize);
			err=MemError();
			if(err==noErr) {
				HLock(doc->marks);
				BlockMove(*doc->marks+2,*doc->marks+newSize+2,oldSize-2);
				*((short *)*doc->marks)=nMarks+1;
				m0 = (MarkRecPtr)(*doc->marks + 2);
				m0->selStart=selStart;
				m0->selEnd=selEnd;
				BlockMove(p,&m0->label,*p+1);
				HUnlock(doc->marks);
#if 0
				if(doc->resourcePathRefNum)
					ChangedResource(doc->marks);
#endif
				GetItemMark(MarkMenu,iAlphabetical,&itemType);
				sortMarks(doc,!itemType);	/* Calls FillMarkMenu */
			} else {
				/* A mem error */ 
			
			}
		} else {
			newSize=*p+(((*p)%2)? 11:12);
			doc->marks=NewHandle(newSize);
			if(doc->marks) {
				HLock(doc->marks);
				*((short *)*doc->marks)=1;
				m0 = (MarkRecPtr)(*doc->marks + 2);
				m0->selStart=selStart;
				m0->selEnd=selEnd;
				BlockMove(p,&m0->label,*p+1);
				if(!(m0->label%2)) {
					m0->label++;
					p=&m0->label;
					p[m0->label]='\0';
				}
				HUnlock(doc->marks);
				FillMarkMenu(doc);
			}
		}
	}
}

ListHandle FillMarkList(DocumentPeek doc,WindowPtr unMarkDialog, int selStart, int selEnd)
{
	short i,nMarks;
	Ptr mark;
	MarkRecPtr m0;
	ListHandle theList=0;
	Rect box,dataBounds;
	Point cellSize,cell;
	short itemType;
	Handle itemHand;
	
	nMarks = *((short *)*doc->marks);
	
	GetDItem(unMarkDialog,4,&itemType,&itemHand,&box);
	InsetRect(&box,1,1);

	cellSize.v=15;
	cellSize.h=box.right-box.left;

	box.bottom=box.top+cellSize.v*((box.bottom-box.top)/cellSize.v);
	box.right-=15;

	SetRect(&dataBounds,0,0,1,nMarks);
	theList=LNew(&box,&dataBounds,cellSize,0,unMarkDialog,FALSE,FALSE,FALSE,TRUE);
	if(!theList) {
		SysBeep(2);
		return 0;
	}

	HLock(doc->marks);

	cell.h=0;
	cell.v=0;
	mark=*doc->marks+2;
	for(i=0;i<nMarks;i++) {
		m0=(MarkRecPtr)mark;
		LSetCell(mark+9,m0->label,cell,theList);
		if(m0->selStart==selStart && m0->selEnd==selEnd)
			LSetSelect(true,cell,theList);
		cell.v++;
		mark+= 9+m0->label;
	}
	HUnlock(doc->marks);
	return theList;
}

short DeleteMark(DocumentPeek doc,short markIndex)
{
	short nMarks,i;
	Ptr mark;
	long len,delta,newSize;
	
	if(doc->marks) {
		nMarks=*((short *)*doc->marks);
		if(markIndex>nMarks)
			return nMarks;
		if(--nMarks) {
			len=GetHandleSize(doc->marks);
			HLock(doc->marks);
			*((short *)*doc->marks)=nMarks;
			mark=*doc->marks + 2;
			for(i=0;i<markIndex;i++)
				mark+=9+((MarkRecPtr)mark)->label;
			delta = 9 + ((MarkRecPtr)mark)->label;
			newSize=(long)StripAddress(*doc->marks);
			newSize+=len;
			newSize-=(long)StripAddress(mark);
			newSize-=delta;

			if(markIndex<nMarks)	/* not last mark to be deleted */
				BlockMove(mark+delta,mark,newSize);
			HUnlock(doc->marks);
			newSize=len-delta;
			SetHandleSize(doc->marks,newSize);
			i=(short)MemError();
			if(i!=noErr) {
				DisposeHandle(doc->marks);
				doc->marks=0;
			}
		} else {
			SetHandleSize(doc->marks,2);
			*((short *)*doc->marks)=nMarks;
		}
		FillMarkMenu(doc);
		return nMarks;
	}
}

/*---------------------------------------------------------------------------
	This is a universal filter for modal dialogs that use List Manager lists.
	It does the conventional checking for double clicks and return or enter
	key hits.
-----------------------------------------------------------------------------*/

pascal Boolean listFilter(DialogPtr dialog,EventRecord *theEvent,short *itemHit)
{
	ListHandle list;
	Point mouseLoc,cell;
	short modifiers;
	Rect box;
	short itemType;
	Handle itemHand;
	Boolean selected;
	
	SetPort(dialog);
	GetDItem(dialog,1,&itemType,&itemHand,&box);

	cell.h=cell.v=0;
	list=(ListHandle)GetWRefCon(dialog);
	selected=LGetSelect(true,&cell,list);
	
	if(theEvent->what==mouseDown) {
		mouseLoc=theEvent->where;
		modifiers=theEvent->modifiers;
		GlobalToLocal(&mouseLoc);
		if(LClick(mouseLoc,modifiers,list)) {	/* Double Click */
			*itemHit=1;
			return true;
		}
	} else if(theEvent->what==keyDown) {
		if(DialogStandardKeyDown(dialog,theEvent,itemHit))
			return true;
	}
	HiliteControl((ControlHandle)itemHand,(selected)? 0:255);
	return false;
}

void UpdateList (ListHandle TheListHandle)
{
	Rect ViewRect;
	RgnHandle ListUpdateRgn;

	SetPort((**TheListHandle).port);
	/*	Get the List manager to update the list. */
	ViewRect = (**TheListHandle).rView;
	LDoDraw(true, TheListHandle);
	ListUpdateRgn = NewRgn();
	RectRgn(ListUpdateRgn,&ViewRect);
	LUpdate(ListUpdateRgn, TheListHandle);
	/*	Draw the border */
	InsetRect(&ViewRect, -1, -1);
	FrameRect(&ViewRect);
	/*	Clean up after ourselves */
  	DisposeRgn(ListUpdateRgn);
}

void DoUnmark(DocumentPeek doc,int selStart,int selEnd)
{
	DialogPtr unmarkDialog;
	Boolean pau=false;
	short itemHit,itemType;
	Handle itemHand;
	ListHandle list;
	Rect box;
	Point cell;
	Str255 buf;
	
	unmarkDialog=GetNewDialog(rUnmarkDialog,(Ptr)&DlogStor,(WindowPtr)(-1));

	if(!unmarkDialog) 
		FatalError();
	
	if(list=FillMarkList(doc,unmarkDialog,selStart,selEnd)) {
		SetWRefCon(unmarkDialog,(long)list);
		LAutoScroll(list);
		UpdateList(list);
				
		GetIndString(buf,128,5);
		GetDItem(unmarkDialog,3,&itemType,&itemHand,&box);
		SetIText(itemHand,buf);
		
		GetDItem(unmarkDialog,5,&itemType,&itemHand,&box);
		SetDItem(unmarkDialog,5,itemType,(Handle)doButton,&box);

		while(!pau) {
			ModalDialog(listFilter,&itemHit);
				if(itemHit==1) {			/* Okay */
				pau=true;
				cell.h=cell.v=0;
				for(itemType=0;LGetSelect(true,&cell,list);itemType++) {
					DeleteMark(doc,cell.v-itemType);
					cell.v++;
				}
			} else if(itemHit==2) {		/* Cancel */
				pau=true;
			}
		}
	}
	LDispose(list);
	CloseDialog(unmarkDialog);
}

/*	
	This works okay. It operates directly on the MENU resource in memory.
 	This is not illegal.
*/
void FillMarkMenu(DocumentPeek doc)
{
	register Ptr mark,dest;
	register int i,j,nMarks;
	Boolean order=false,putUp;
	Size baseLength,markLength;
	
	OSErr err;
	CursHandle cH;
	
	if(doc->markMenu) {
		cH=GetCursor(watchCursor);
		if(cH)
			SetCursor(*cH);

		if(putUp=(StripAddress(MarkMenu)==StripAddress(doc->markMenu)))
			DeleteMenu(MARK_ID);
			
		DisposeHandle((Handle)doc->markMenu);
		doc->markMenu=(MenuHandle)GetResource('MENU',MARK_ID);
		err=HandToHand((Handle *)&doc->markMenu);
		if(err!=noErr)
			return;
			
		if(doc->marks) {
			nMarks=*(short *)*doc->marks;
			
			/* Later on, only do the following if nMarks>50 */
			
			baseLength=GetHandleSize((Handle)doc->markMenu);
			markLength=GetHandleSize(doc->marks)-nMarks*4-2;
			
			SetHandleSize((Handle)doc->markMenu,baseLength+markLength);
			err=MemError();
			if(err!=noErr)
				return;
					
			HLock(doc->marks);
	
			/* 1. Check for order */
			mark=*doc->marks+2;
			for(j=i=0;i<nMarks;i++,mark+=sizeofMark(mark)) {
				if(j>((MarkRecPtr)mark)->selStart) {
					CheckItem(doc->markMenu,iAlphabetical,true);
					break;
				}
				j=((MarkRecPtr)mark)->selStart;
			}
			mark=*doc->marks+10;
			
			/* 2. Move the mark labels into the MENU resource structure */
			HLock((Handle)doc->markMenu);
			memset((*((Handle)doc->markMenu))+baseLength,0,markLength);
			
			mark=*doc->marks+10;
			dest=(*((Handle)doc->markMenu))+baseLength-1;
						
			while(nMarks) {
				for(j=*mark+1,i=0;i<j;i++) 
					*dest++ = *mark++;
				mark+=8;
				dest+=4;
				nMarks--;
			}
				
			HUnlock(doc->marks);
			HUnlock((Handle)doc->markMenu);
			
			CalcMenuSize(doc->markMenu);
			
			if(putUp) {
				MarkMenu=doc->markMenu;
				InsertMenu(MarkMenu,WINDOW_ID);
			}
		}
	}
	InitCursor();
}

/*----------------------------------------------------------------------

	Marks whose selBegin is greater than a value are moved by delta, 
	whether delta is positive or negative.

	Marks whose selEnd is less than a value are not changed.

	If delta is negative then marks completely enclosed in a range are 
	deleted.

	Marks that are intersected by a range are extended or decreased.

	The range and delta can usually be determined from the undo 
	parameters. In continuous typing, however, this is not possible as 
	an event that triggers a need to update the mark (e.g. a mouse-down 
	in the menu bar) may not trigger the formation of a new undo buffer.
	
-----------------------------------------------------------------------*/

void UpdateMarks(DocumentPeek doc,long selStart,long selEnd,long delta, long docLength)
{
	Ptr mark;
	short nMarks,i,mSize;

#if 0
/*=============== Debug Code ===========*/
	long offset;
	TE32KHandle tH;
	MarkRecPtr theMark;
	char *p,*q;
/*=============== Debug Code ===========*/
#endif
	if(doc->marks && delta) {
		HLock(doc->marks);
		nMarks = *((short *)*doc->marks);
		mark=*doc->marks+2;
		
		/*	
			If a mark is deleted, mSize is set to zero so that we won't increment 
			past the newly current mark. 
		*/
			
		for(i=0;i<nMarks;i++,mark+=mSize) {
			mSize = sizeofMark(mark);
			
			if(selStart <= selEnd) {
				if(selEnd < ((MarkRecPtr)mark)->selStart) {	
					/* 	
						Most common case: 
							No overlap with mark and mark is after the edited region. 
					*/
					
					((MarkRecPtr)mark)->selStart += delta;
					((MarkRecPtr)mark)->selEnd += delta;
					
				} else if(selStart<((MarkRecPtr)mark)->selStart 
							&& selEnd>((MarkRecPtr)mark)->selEnd) {
					/* 
						Extreme case: 
							Change overlaps both ends of mark. Either delete or move mark.
					*/
					
					if(delta<0) {					/* Negative delta means text has been deleted. */
						nMarks=DeleteMark(doc,i);	/* N.B. DeleteMark unlocks doc->marks handle. */
						if(nMarks) {
							HLock(doc->marks);
							mark=(Ptr)GetIndMark(*doc->marks,i--);
							if(!mark) {
								HUnlock(doc->marks);
								return;
							}
						}  
						mSize=0;
					} else {						/* Text added. Move mark. */
						((MarkRecPtr)mark)->selStart += delta;
						((MarkRecPtr)mark)->selEnd += delta;
					}
				} else if(selStart <= ((MarkRecPtr)mark)->selEnd 
						|| selEnd <= ((MarkRecPtr)mark)->selEnd) {
					/*
						Mark partially overlapped by edited region.
					*/
					
					((MarkRecPtr)mark)->selEnd += delta;
				}
			} else {
				if(selStart < ((MarkRecPtr)mark)->selStart) {	
					/* Most common case: No overlap. */
					((MarkRecPtr)mark)->selStart += delta;
					((MarkRecPtr)mark)->selEnd += delta;
				} else if(selEnd<((MarkRecPtr)mark)->selStart && selStart>((MarkRecPtr)mark)->selEnd) {
					/* Extreme case, change overlaps both end. */
					if(delta<0) {
						((MarkRecPtr)mark)->selStart = selEnd;
						((MarkRecPtr)mark)->selEnd = selEnd;
					} else {
						((MarkRecPtr)mark)->selStart += delta;
						((MarkRecPtr)mark)->selEnd += delta;
					}
				} else if(selEnd <= ((MarkRecPtr)mark)->selStart) {		/* deleting back through beginning of mark */
					if(delta<0) {
						((MarkRecPtr)mark)->selStart = selEnd;
						((MarkRecPtr)mark)->selEnd = selEnd;
					} else {
						((MarkRecPtr)mark)->selStart += (selStart-selEnd);
						((MarkRecPtr)mark)->selEnd += delta;
					}
					
				} else if(selEnd <= ((MarkRecPtr)mark)->selEnd) {		/* deleting back and encountering mark	*/
					if(delta<0)
						((MarkRecPtr)mark)->selEnd = selEnd;
					else
						((MarkRecPtr)mark)->selEnd += (selStart-selEnd);
				}
			}
			/*
				If a mark is beyond the current document. Delete it. 
			*/
			if(mSize && docLength>=0 && (((MarkRecPtr)mark)->selEnd>docLength 
						|| ((MarkRecPtr)mark)->selStart>docLength)) { 
				if(nMarks=DeleteMark(doc,i)) {
					HLock(doc->marks);
					mark=(Ptr)GetIndMark(*doc->marks,i--);
					if(!mark) {
						HUnlock(doc->marks);
						return;
					}
				}
				mSize=0;
			}  
		}
		if(doc->marks)		/* It could have been disposed of. */
			HUnlock(doc->marks);
	}
	doc->fNeedtoUpdateMarks = false;
	
#if 0
/*=============== Debug Code ===========*/

	tH = (TE32KHandle)doc->docData;
	if(tH && doc->marks) {
		theMark = (MarkRecPtr)(*doc->marks+2);
		offset = theMark->selStart;
		p=*(**tH).hText+offset;
		q=&theMark->label;
		if(*p != q[1])
			SysBeep(7);
		offset=*q;
		if(!*(q+offset))
			offset--;
		if(*(p+offset-1) != *(q+offset))
			SysBeep(7);
	}
#endif
}

#ifdef MC68000
#pragma segment appleevents
#endif

/*--------------InitAEStuff installs AppleEvent handlers ----------------*/

void InitAEStuff(void)
{
    AEinstalls HandlersToInstall[] =  
		{ 	{ kCoreEventClass, kAEOpenApplication, AEOpenHandler	},
			{ kCoreEventClass, kAEOpenDocuments, AEOpenDocHandler	},
			{ kCoreEventClass, kAEQuitApplication, AEQuitHandler	},
			{ kCoreEventClass, kAEPrintDocuments, AEPrintHandler	}, 
			/* The above are the four required AppleEvents. */
    	};
    OSErr aevtErr = noErr;
	register qq;

    /*	
		The following series of calls installs all our AppleEvent Handlers.
		These handlers are added to the application event handler list that 
		the AppleEvent manager maintains.  So, whenever an AppleEvent happens
		and we call AEProcessEvent, the AppleEvent manager will check our
		list of handlers and dispatch to it if there is one.
    */
	for (qq = 0; qq < ((sizeof(HandlersToInstall) / sizeof(AEinstalls))); qq++) {
		aevtErr = AEInstallEventHandler(HandlersToInstall[qq].theClass, HandlersToInstall[qq].theEvent,
                                            HandlersToInstall[qq].theProc, 0, false);
		if (aevtErr) {
			ExitToShell();			/* just fail, baby */
		}
    }
}

/* This is the standard Open Application event.  */
pascal OSErr AEOpenHandler(AppleEvent *messagein, AppleEvent *reply, long refIn)
{

#ifdef MC68000
#pragma unused (messagein,reply,refIn)
#endif
	
	OpenWorksheetDoc();	
    return noErr;
}

/* end AEOpenHandler */

/* Open Doc, opens our documents.  Remember, this can happen at application start AND */
/* anytime else.  If your app is up and running and the user goes to the desktop, hilites one */
/* of your files, and double-clicks or selects Open from the finder File menu this event */
/* handler will get called. Which means you don't do any initialization of globals here, or */
/* anything else except open then doc.  */
/* SO-- Do NOT assume that you are at app start time in this */
/* routine, or bad things will surely happen to you. */

pascal OSErr AEOpenDocHandler(AppleEvent *messagein, AppleEvent *reply, long refIn)
{
	OSErr err;
	
#ifdef MC68000
#pragma unused (refIn,reply)
#endif
	if(!gWorkSheetDoc) 
		OpenWorksheetDoc();
	err = processOpenPrint(messagein, false);
	return err;
}

pascal OSErr AEPrintHandler(AppleEvent *messagein, AppleEvent *reply, long refIn)
{                                                           /* no printing handler in yet, so we'll ignore this */
	OSErr err;
	
#ifdef MC68000
#pragma unused (refIn,reply)
#endif
   
	err = processOpenPrint(messagein, true);
	return err;
}

pascal OSErr AEQuitHandler(AppleEvent *messagein, AppleEvent *reply, long refIn)
{
#ifdef MC68000
#pragma unused (messagein,refIn,reply)
#endif
    QuitTime();
    return noErr;
}

/*-------------------------------------------------------------------------------------

	MissedAnyParameters

	Used to check for any unread required parameters. Returns true if we missed at
	least one.

--------------------------------------------------------------------------------------*/

Boolean MissedAnyParameters(AppleEvent *message)
{
    OSErr err;
    DescType ignoredActualType;
    AEKeyword missedKeyword;
    Size ignoredActualSize;
    EventRecord event;
    
    err = AEGetAttributePtr(message, keyMissedKeywordAttr, typeKeyword, &ignoredActualType, (Ptr)&missedKeyword,
                            sizeof(missedKeyword), &ignoredActualSize);
    
    /* no error means that we found some more.*/
    
    if (!err) {
        event.message = *(long *)&ignoredActualType;
        event.where = *(Point *)&missedKeyword;
        doMessage(7);
        err = errAEEventNotHandled;
    }
    
    /* errAEDescNotFound means that there are no more parameters. If we get */
    /* an error code other than that, flag it. */
    
    else if (err != errAEDescNotFound) {
        doMessage(8);
        
    }
    return(err != errAEDescNotFound);
}

/*-------------------------------------------------------------------------------------

 	processOpenPrint handles ODOC and PDOC events.  Both events open a document, 
 	one prints it 
--------------------------------------------------------------------------------------*/

OSErr processOpenPrint(AppleEvent *messagein, Boolean printIt)
{
    OSErr err;
    OSErr err2;
    AEDesc theDesc;
    FSSpec theFSS;
    register qq;
    long numFilesToOpen;
    AEKeyword ignoredKeyWord;
    DescType ignoredType;
    Size ignoredSize;
    WindowPtr wind;
	
	err = AEGetParamDesc(messagein, keyDirectObject, typeAEList, &theDesc);
	if (err) 
		doMessage(9);

	if (!MissedAnyParameters(messagein)) {
        
		/* Got all the parameters we need. Now, go through the direct object, */
		/* see what type it is, and parse it up. */
        
		if (err = AECountItems(&theDesc, &numFilesToOpen)) {
			doMessage(10);
		} else {
			for (qq = 1; ((qq <= numFilesToOpen) && (!err)); ++qq) {
				if (err = AEGetNthPtr(&theDesc, qq, typeFSS, &ignoredKeyWord, &ignoredType, (Ptr)&theFSS, sizeof(theFSS),
						&ignoredSize)) {
					doMessage(11);
                    
				} else {
					wind=OpenOldFile(&theFSS);
					if(wind) {
						DefaultSpecs=theFSS;
						/*======= TN 80 technique for setting SF Default Vol =====*/
						*(short *)SFSaveDisk = -1 * theFSS.vRefNum;
						*(long *)CurDirStore = theFSS.parID; /*==== Global Variable fill =====*/
					}
				}
				if (printIt && wind) {
                    (*((DocumentPeek)wind)->doPrint)(wind);
					CloseADoc(wind);
				}
			}												/* for qq = ... */
		}													/* AECountItems OK */
	}														/* Got all necessary parameters */
    
	if (err2 = AEDisposeDesc(&theDesc)) 
		doMessage(12);

	return(err ? err : err2);
}
