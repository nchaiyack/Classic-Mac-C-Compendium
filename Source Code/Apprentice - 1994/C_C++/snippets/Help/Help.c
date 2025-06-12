typedef struct
{
	ResType	hlpType;
	short	hlpID;
}	SHelpInfo, *SHelpInfoPtr, **SHelpInfoHandle;



typedef struct
{
	ListHandle	myList;
	TEHandle	myTEh;
	PicHandle	myPic;
	Rect		myPicRect;
	SHelpInfo	mSHelpInfo[1];
}	HelpInfo, *HelpInfoPtr, **HelpInfoHandle;




// constants

#define	kInFront		(WindowPtr)-1L
#define	kListItem		2
#define	kTextItem		3
#define	kMargin			2

#define	kCR				0x0d
#define	kEnter			0x03
#define	kDelete			0x08
#define	kEscape			0x1B
#define	kDownArrow		0x1F
#define	kUpArrow		0x1E

#define	SysEnvironsTrap	0xA090
#define	UnknownTrap		0xA89F


// prototypes

void SetupDialog(DialogPtr theDialog);
void PrepareFreeDialog(DialogPtr theDialog);

Boolean InMyList(DialogPtr theDialog, Point p, short modifiers);
pascal void ListDrawProc(DialogPtr theDialog,short theItem);
pascal void TextDrawProc(DialogPtr theDialog,short theItem);
pascal Boolean MyDialogFilter(DialogPtr theDialog,EventRecord *ev,short *itemHit);
void HandleKeyPress(DialogPtr theDialog,char theChar);
void HandleActivate(DialogPtr theDialog);

void DrawHelpText(short TextIndex, HelpInfoHandle myHelpInfoHandle);


void DrawAHelpPICT(TEHandle theTE, SHelpInfoPtr mySHelpInfoPtr, short TextIndex, HelpInfoHandle myHelpInfoHandle);
void DrawAHelpTEXT(TEHandle theTE, SHelpInfoPtr mySHelpInfoPtr, short TextIndex);

void SetSelection(DialogPtr theDialog, short Offset);

ControlHandle GetScrollBar(DialogPtr theDialog);
TEHandle GetListInfoHandle(DialogPtr theDialog);




short TestForColor()
{
  short		hasColor;							/* Color Flag			*/
  GrafPtr	aPort;								/* Quickdraw port		*/
  SysEnvRec	sysEnv;
  hasColor = false;								/* Test for color		*/
  if ((long)NGetTrapAddress(SysEnvironsTrap, OSTrap) !=
  (long)NGetTrapAddress(UnknownTrap, ToolTrap))
  {
     SysEnvirons(1, &sysEnv);
     GetPort(&aPort);
     if (sysEnv.hasColorQD)
     {
       if ((*((*((GDHandle)GetGDevice()))->gdPMap))->pixelSize > 1)		/* Check graphic's device depth	*/
       {
         hasColor = BitTst(&aPort->portBits.rowBytes, 0L);			/* Set color			*/
       }
     }
   }
   return(hasColor);
}



void DoHelp(void)
{
	DialogPtr	theDialog;
	short		item;
	
	theDialog = GetNewDialog(128, 0L, kInFront);
	SetupDialog(theDialog);
		
	do
	{
		ModalDialog(MyDialogFilter, &item);
	}while(item != ok);
	
	PrepareFreeDialog(theDialog);
	DisposeDialog(theDialog);
}


void SetupDialog(DialogPtr theDialog)
{
	short			iType, iCnt, i;
	Handle			iHndl;
	Rect			iRect, rView, rBounds;
	Point			pCellSz;
	TEHandle		theTE;
	ListHandle		theList;
	HelpInfoHandle	myHelpInfoHandle;
	SHelpInfoHandle	hRsrc;
	FontInfo		fInfo;
	
	SetPort(theDialog);
	GetDItem(theDialog,kListItem,&iType,&iHndl,&iRect);
	
	TextFont(geneva);
	TextSize(9);
	TextFace(bold);
	
	GetFontInfo(&fInfo);
	pCellSz.h = 0;
	pCellSz.v = fInfo.ascent + fInfo.descent + fInfo.leading;
	iRect.bottom = iRect.top + ((iRect.bottom - iRect.top) / pCellSz.v) * pCellSz.v;
	InsetRect(&iRect, -1, -1);
	SetDItem(theDialog,kListItem,iType,ListDrawProc,&iRect);
	
	rView = iRect;
    rView.right -= 14;
    InsetRect(&rView, 1, 1);

	SetRect(&rBounds,0,0,1,0);
	 
	
	
	theList = LNew(&rView,&rBounds,pCellSz,0,theDialog,TRUE,TRUE,FALSE,TRUE);
	LDoDraw(FALSE, theList);
	iCnt = Count1Resources('MHLP');
	myHelpInfoHandle = NewHandle(sizeof(HelpInfo) + sizeof(SHelpInfo) * (iCnt - 1));
	pCellSz.h = 0;
	for(i = 1; i <= iCnt; i++)
	{
		short	rID;
		ResType	rType;
		Str255	rName;
		
		pCellSz.v = LAddRow(1, 999, theList);
		hRsrc = Get1IndResource('MHLP', i);
		GetResInfo(hRsrc, &rID, &rType, rName);
		LSetCell(rName + 1, *rName, pCellSz, theList);
		
		(*myHelpInfoHandle)->mSHelpInfo[i - 1].hlpID = (*hRsrc)->hlpID;
		(*myHelpInfoHandle)->mSHelpInfo[i - 1].hlpType = (*hRsrc)->hlpType;
		ReleaseResource(hRsrc);
	}
	(*myHelpInfoHandle)->myList = theList;
	pCellSz.h = 0; pCellSz.v = 0;
	LSetSelect(TRUE, pCellSz, theList);

	

	GetDItem(theDialog,kTextItem,&iType,&iHndl,&iRect);
	SetDItem(theDialog,kTextItem,iType,TextDrawProc,&iRect);
	InsetRect(&iRect, kMargin, kMargin);
	(*myHelpInfoHandle)->myTEh = TEStylNew(&iRect, &iRect);
	(*myHelpInfoHandle)->myPic = 0L;
	
	TextFont(0);
	TextSize(0);
	TextFace(0);
	
	DrawHelpText(0, myHelpInfoHandle);
	SetWRefCon(theDialog,(long)myHelpInfoHandle);
	LDoDraw(TRUE, theList);
}




void PrepareFreeDialog(DialogPtr theDialog)
{
	HelpInfoHandle	myHelpInfoHandle;
	
	myHelpInfoHandle = GetListInfoHandle(theDialog);
	if((*myHelpInfoHandle)->myPic)
		ReleaseResource((*myHelpInfoHandle)->myPic);
	LDispose((*myHelpInfoHandle)->myList);
	TEDispose((*myHelpInfoHandle)->myTEh);
	DisposHandle(myHelpInfoHandle);
}



pascal void ListDrawProc(DialogPtr theDialog,short theItem)
{
	short		iType, hasColor;
	Handle		iHndl;
	Rect		iRect;
	GrafPtr		savePort;
	RGBColor	SaveColor;
	HelpInfoHandle	myHelpInfoHandle;
	
	GetPort(&savePort);
	SetPort(theDialog);
	TextFont(geneva);
	TextSize(9);
	TextFace(bold);
	myHelpInfoHandle = GetListInfoHandle(theDialog);
	hasColor = TestForColor();
	if(hasColor)
		GetBackColor(&SaveColor);
	BackColor(whiteColor);
	GetDItem(theDialog,theItem,&iType,&iHndl,&iRect);
	EraseRect(&iRect);
	FrameRect(&iRect);
	LUpdate(theDialog->visRgn, (*myHelpInfoHandle)->myList);
	if(hasColor)
		RGBBackColor(&SaveColor);
	TextFont(0);
	TextSize(0);
	TextFace(0);
	SetPort(savePort);
}



pascal void TextDrawProc(DialogPtr theDialog,short theItem)
{
	short		iType, hasColor;
	Handle		iHndl;
	Rect		iRect;
	GrafPtr		savePort;
	RGBColor	SaveColor;
	HelpInfoHandle	myHelpInfoHandle;
	
	GetPort(&savePort);
	SetPort(theDialog);
	myHelpInfoHandle = GetListInfoHandle(theDialog);
	hasColor = TestForColor();
	if(hasColor)
		GetBackColor(&SaveColor);
	GetDItem(theDialog,theItem,&iType,&iHndl,&iRect);
	EraseRect(&iRect);
	if((*myHelpInfoHandle)->myPic)
		DrawPicture((*myHelpInfoHandle)->myPic, &(*myHelpInfoHandle)->myPicRect);
	else
		TEUpdate(&iRect, (*myHelpInfoHandle)->myTEh);
	if(hasColor)
		RGBBackColor(&SaveColor);
	SetPort(savePort);
}




pascal Boolean MyDialogFilter(DialogPtr theDialog, EventRecord *ev, short *itemHit)
{
	char theChar;
	
	switch (ev->what)
	{
		case keyDown:
		case autoKey:
			theChar = (ev->message & charCodeMask);
			switch(theChar)
			{
				case kCR:
				case kEnter:
				case kEscape:
					*itemHit = ok;
					return TRUE;
				case kDownArrow:
					SetSelection(theDialog, +1);
					break;
				case kUpArrow:
					SetSelection(theDialog, -1);
					break;
				default:
					return FALSE;
			}
		case activateEvt:
			HandleActivate(theDialog);
			return false;
		case mouseDown:
		{
			if(InMyList(theDialog, ev->where, ev->modifiers))
				return(TRUE);
			else
				return(FALSE);
				
		}
		default:
			return false;
	}
}


void SetSelection(DialogPtr theDialog, short Offset)
{
	HelpInfoHandle	myHelpInfoHandle;
	ListHandle		theList;
	GrafPtr			savePort;
	Cell			theCell;
	short			hasColor, NewSel;
	RGBColor		SaveColor;
	
	GetPort(&savePort);
	SetPort(theDialog);
	TextFont(geneva);
	TextSize(9);
	TextFace(bold);
	hasColor = TestForColor();
	if(hasColor)
		GetBackColor(&SaveColor);
	BackColor(whiteColor);
	myHelpInfoHandle = GetListInfoHandle(theDialog);
	theList = (*myHelpInfoHandle)->myList;
	
	theCell.h = theCell.v = 0;
	LGetSelect(TRUE, &theCell, theList);
	NewSel = theCell.v + Offset;
	if((NewSel >= (*theList)->dataBounds.top) && (NewSel < (*theList)->dataBounds.bottom))
	{
		LSetSelect(FALSE, theCell, theList);
		theCell.v = NewSel;
	
		LSetSelect(TRUE, theCell, theList);
		LAutoScroll(theList);
	}
	if(hasColor)
		RGBBackColor(&SaveColor);
	DrawHelpText(theCell.v, myHelpInfoHandle);
	TextFont(0);
	TextSize(0);
	TextFace(0);
	SetPort(savePort);
}


Boolean InMyList(DialogPtr theDialog, Point p, short modifiers)
{
	Rect			iRect;
	short			type, hasColor;
	Handle			iHndle;
	GrafPtr			savePort;
	HelpInfoHandle	myHelpInfoHandle;
	RGBColor		SaveColor;
	Cell			theOriginalCell, theCell;
	ListHandle		theList;
	
	GetPort(&savePort);
	SetPort(theDialog);
	myHelpInfoHandle = GetListInfoHandle(theDialog);
	theList = (*myHelpInfoHandle)->myList;
	HLock(theList);
	GlobalToLocal(&p);
	GetDItem(theDialog, kListItem, &type, &iHndle, &iRect);
	if(PtInRect(p, &iRect))
	{
		theOriginalCell.h = theOriginalCell.v = 0;
		LGetSelect(TRUE, &theOriginalCell, theList);
		
		TextFont(geneva);
		TextSize(9);
		TextFace(bold);
		hasColor = TestForColor();
		if(hasColor)
			GetBackColor(&SaveColor);
		BackColor(whiteColor);
		LClick(p, modifiers, theList);
		if(hasColor)
			RGBBackColor(&SaveColor);
		TextFont(0);
		TextSize(0);
		TextFace(0);
		
		theCell.h = theCell.v = 0;
		LGetSelect(TRUE, &theCell, theList);
		if(theCell.v != theOriginalCell.v)
			DrawHelpText(theCell.v, myHelpInfoHandle);
		HUnlock(theList);
		SetPort(savePort);
		return(TRUE);
	}
	SetPort(savePort);
	return(FALSE);
}


void DrawHelpText(short TextIndex, HelpInfoHandle myHelpInfoHandle)
{
	TEHandle		theTE;
	SHelpInfoPtr	mySHelpInfoPtr;
	
	theTE = (*myHelpInfoHandle)->myTEh;
	HLock(myHelpInfoHandle);
	mySHelpInfoPtr = (*myHelpInfoHandle)->mSHelpInfo;
	
	if((*myHelpInfoHandle)->myPic)
	{
		EraseRect(&(*myHelpInfoHandle)->myPicRect);
		ReleaseResource((*myHelpInfoHandle)->myPic);
	}
	switch(mySHelpInfoPtr[TextIndex].hlpType)
	{
		case 'TEXT':
			DrawAHelpTEXT(theTE, mySHelpInfoPtr, TextIndex);
			(*myHelpInfoHandle)->myPic = 0L;
			break;
		case 'PICT':
			DrawAHelpPICT(theTE, mySHelpInfoPtr, TextIndex, myHelpInfoHandle);
			break;
		default:
			SysBeep(1);
	}
	HUnlock(myHelpInfoHandle);
}


void DrawAHelpPICT(TEHandle theTE, SHelpInfoPtr mySHelpInfoPtr, short TextIndex, HelpInfoHandle myHelpInfoHandle)
{
	PicHandle	thePic;
	Rect		r;
	long		wide, high;
	
	(*theTE)->selStart = 0;
	(*theTE)->selEnd = 32000;
	TEDelete(theTE);
	thePic = Get1Resource('PICT', mySHelpInfoPtr[TextIndex].hlpID);
	HLock(thePic);
	r = (*thePic)->picFrame;
	wide = r.right - r.left;
	high = r.bottom - r.top;
	SetRect(&r, (*theTE)->destRect.left, (*theTE)->destRect.top, (*theTE)->destRect.left + wide, (*theTE)->destRect.top + high);
	DrawPicture(thePic, &r);
	(*myHelpInfoHandle)->myPicRect = r;
	(*myHelpInfoHandle)->myPic = thePic;
	HUnlock(thePic);
	
}



void DrawAHelpTEXT(TEHandle theTE, SHelpInfoPtr mySHelpInfoPtr, short TextIndex)
{
	Handle			myText;
	StScrpHandle	SavedStyle;
	
	myText = Get1Resource('TEXT', mySHelpInfoPtr[TextIndex].hlpID);
	HLock(myText);
	SavedStyle = Get1Resource('styl', mySHelpInfoPtr[TextIndex].hlpID);
	HLock(SavedStyle);
	
	(*theTE)->selStart = 0;
	(*theTE)->selEnd = 32000;
	TEDelete(theTE);
	TEActivate(theTE);
	if((*SavedStyle)->scrpNStyles > 0)
		TEStylInsert(*myText, GetHandleSize(myText), SavedStyle, theTE);
	else
		TEInsert(*myText, GetHandleSize(myText), theTE);
	TEDeactivate(theTE);
	HUnlock(SavedStyle);
	HUnlock(myText);
	ReleaseResource(myText);
	ReleaseResource(SavedStyle);
}




void HandleActivate(DialogPtr theDialog)
{
	HelpInfoHandle	myHelpInfoHandle;

	myHelpInfoHandle = GetListInfoHandle(theDialog);
	TextFont(geneva);
	TextSize(9);
	TextFace(bold);
	LActivate(TRUE, (*myHelpInfoHandle)->myList);
	TextFont(0);
	TextSize(0);
	TextFace(0);
}



TEHandle GetListInfoHandle(DialogPtr theDialog)
{
	return( (HelpInfoHandle)GetWRefCon((WindowPtr)theDialog) );
}