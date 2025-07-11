/*********************************************************************
 * EditWindow.c
 *
 * HexEdit, a simple hex editor
 * copyright 1993, Jim Bumgardner
 *********************************************************************/
#include "HexEdit.h"
#include "Folders.h"
#include <stdio.h>

// Create a new main window using a 'WIND' template from the resource fork
//

Boolean 		gCursorFlag;
Rect			gCursRect;
BitMap			gBitMap;
Preferences		gPrefs = {AM_Lo, false, true};
short			gForkMode=FM_Smart,gHighChar='~',gOverwrite;
THPrint			gHPrint;
static char		gBuffer[80];
Cursor			gWatchCursor,gIBeamCursor;
short			gMaxHeight=342;

void LoadPreferences(void)
{
	Handle	h;
	h = GetResource(PrefResType, PrefResID);
	if (h == NULL || ResError())
		return;
	BlockMove(*h, &gPrefs, sizeof(Preferences));
	ReleaseResource(h);
	if (gPrefs.asciiMode)
		gHighChar = 255;
	else
		gHighChar = '~';
}

void SavePreferences(void)
{
	Handle	h;
	while ((h = GetResource(PrefResType, PrefResID)) != NULL) {
		RmveResource(h);
		DisposHandle(h);
	}
	h = NewHandle(sizeof(Preferences));
	if (h == NULL) {
		ErrorAlert(ES_Caution,"Not enough memory");
	}
	else {
		BlockMove(&gPrefs, *h, sizeof(Preferences));
		AddResource(h,PrefResType,PrefResID,"\pPreferences");
		WriteResource(h);
		ReleaseResource(h);
	}
}

void InitializeEditor()
{
	Rect				offRect;
	OSErr				oe;
	CursHandle			cH = NULL;

	if (ScrapInfo.scrapState < 0)
		ZeroScrap();

	// Start Profiling
#if PROFILE			// 6/15 Optional profiling support
	freopen("profile.log","w",stdout);		// If console isn't wanted
	InitProfile(200,200);
	_profile = 0;
	// cecho2file("profile.log",false,stdout);	// If console is wanted
#endif
	gMaxHeight = screenBits.bounds.bottom - screenBits.bounds.top;
	if (gMaxHeight < 342)
		gMaxHeight = 342;
	SetRect(&offRect,0,0,MaxWindowWidth,gMaxHeight);
	gBitMap.rowBytes = (((MaxWindowWidth-1)/32)+1)*4;
	gBitMap.baseAddr = NewPtrClear( (long) gBitMap.rowBytes * gMaxHeight );
	if (gBitMap.baseAddr == NULL)
		ErrorAlert(ES_Stop,"Not enough memory");
	gBitMap.bounds = offRect;

	cH = GetCursor(watchCursor);
	if (cH)
		gWatchCursor = **cH;
	cH = GetCursor(iBeamCursor);
	if (cH)
		gIBeamCursor = **cH;

	PrOpen();
	gHPrint = (THPrint) NewHandle(sizeof(TPrint));
	if (gHPrint == NULL) {
		ErrorAlert(ES_Stop,"Not enough memory");
	}
	PrintDefault(gHPrint);
	PrClose();

	LoadPreferences();
}

void CleanupEditor()
{
	if (gBitMap.baseAddr) {
		DisposePtr(gBitMap.baseAddr);
		gBitMap.baseAddr = NULL;
	}
	SavePreferences();
}

void NewEditWindow(void)
{
	WindowPtr			theWindow;
	EditWindowPtr		dWin;
	OSErr				oe;
	short				refNum=0;
	Point				where={-1,-1};
	HParamBlockRec		pb;
	FSSpec				workSpec;
	Rect				r;

	// Get the Template & Create the Window, it is set up in the resource fork
	// to not be initially visible 

	dWin = (EditWindowPtr) NewPtrClear(sizeof(EditWindowRecord));
	if (dWin == NULL) {
		FSClose(refNum);
		ErrorAlert(ES_Caution, "Can't allocate new window");
		return;
	}

	dWin->fork = FT_Data;
	dWin->fileSize = 0L;
	dWin->refNum = 0;

	// Initialize WorkSpec
	workSpec = dWin->workSpec;
	oe = FindFolder(-1, kTemporaryFolderType, kCreateFolder, 
					&workSpec.vRefNum,
					&workSpec.parID);
	if (oe != noErr) {
		OSErrorAlert(ES_Caution, "FindFolder", oe);
		return;
	}
	BlockMove("\pUntitledw", workSpec.name, 10);
	InsureNameIsUnique(&workSpec);
	HCreate(workSpec.vRefNum,workSpec.parID,workSpec.name,CREATOR, '????');
	if (oe != noErr) {
		OSErrorAlert(ES_Caution, "HCreate", oe);
		return;
	}
	oe = HOpen(workSpec.vRefNum,workSpec.parID,workSpec.name,fsRdWrPerm,&refNum);
	if (oe != noErr) {
		OSErrorAlert(ES_Caution, "HOpen", oe);
		return;
	}

	dWin->workSpec = workSpec;
	dWin->workRefNum = refNum;
	dWin->workBytesWritten = 0L;

	dWin->fileType = DEFFILETYPE;
	dWin->creator = CREATOR;
	dWin->creationDate = 0L;

	theWindow = InitObjectWindow(MainWIND, (ObjectWindowPtr) dWin,false);
	if (theWindow == NULL)
		ErrorAlert(ES_Stop,"Not enough memory");

	SizeWindow(theWindow,484+SBarSize-1-1,gMaxHeight-64,true);
	SetRect(&r, 0, 0, MaxWindowWidth+SBarSize-1, gMaxHeight - 44);
	OffsetRect(&r,3,41);
	((WStateData *)*((WindowPeek) theWindow)->dataHandle)->stdState = r;

	SetWTitle(theWindow, "\pUntitled");

	((ObjectWindowPtr) theWindow)->Draw = MyDraw;
	((ObjectWindowPtr) theWindow)->Idle = MyIdle;
	((ObjectWindowPtr) theWindow)->HandleClick = MyHandleClick;
	((ObjectWindowPtr) theWindow)->Dispose = DisposeEditWindow;
	((ObjectWindowPtr) theWindow)->ProcessKey = MyProcessKey;
	((ObjectWindowPtr) theWindow)->Save = SaveContents;
	((ObjectWindowPtr) theWindow)->SaveAs = SaveAsContents;
	((ObjectWindowPtr) theWindow)->Revert = RevertContents;
	
	// Show the window
	ShowWindow(theWindow);

	// Make it the current grafport
	SetPort(theWindow);

	
	dWin->linesPerPage = ((theWindow->portRect.bottom - SBarSize) - TopMargin - BotMargin - HeaderHeight)/LineHeight;
	dWin->startSel = dWin->endSel = 0L;
	dWin->editMode = EM_Hex;

	SetupScrollBars(dWin);

	dWin->firstChunk = NewChunk(0L,0L,0L, CT_Unwritten);
	dWin->curChunk = dWin->firstChunk;
}

#define DataItem		11
#define RsrcItem		12
#define SmartItem		13

/*SFReply		*gReplyPtr;*/

pascal int SourceDLOGHook(short item, DialogPtr theDialog)
{
	switch (item) {
	case DataItem:
	case RsrcItem:
	case SmartItem:
		SetControl(theDialog,gForkMode+DataItem,false);
		gForkMode = item - DataItem;
		SetControl(theDialog,gForkMode+DataItem,true);
		return sfHookNullEvent;	/* Redraw the List */
	case sfHookFirstCall:
		SetControl(theDialog,gForkMode+DataItem,true);
		return sfHookNullEvent;
/*	case sfHookNullEvent:*/
/*		{*/
/*			Point	p;*/
/*			short	t;*/
/*			Handle	h;*/
/*			Rect	r;*/
/*			GetMouse(&p);*/
/*			GetDItem(theDialog,DataItem,&t,&h,&r);			*/
/*			if (PtInRect(p,&r))*/
/*				DebugStr("\pBoing!");*/
/*		}*/
/*		break;*/
	}
	return item;
}

void AskEditWindow(void)
{
	SFReply				macSFReply;
	FSSpec				fSpec;
	long				procID;
	Point				where = {-1,-1};

	// StandardGetFile(NULL, -1, NULL, &reply);
	if (!gSys7Flag) {
		where.h = 20;
		where.v = 90;
	}

	SFPGetFile(where, "\pFile to Open:", NULL, -1, NULL, (ProcPtr) SourceDLOGHook, 
				&macSFReply, GetFileDLOG, NULL);

	if (macSFReply.good) {
		BlockMove(macSFReply.fName,fSpec.name,macSFReply.fName[0]+1);
		GetWDInfo(macSFReply.vRefNum, &fSpec.vRefNum, &fSpec.parID, &procID);
		OpenEditWindow(&fSpec);
	}
	MyAdjustMenus();
}




void OpenEditWindow(FSSpec *fsSpec)
{
	WindowPtr			theWindow;
	EditWindowPtr		dWin;
	OSErr				oe;
	short				refNum=0;
	Point				where={-1,-1};
	HParamBlockRec		pb;
	FSSpec				workSpec;
	Rect				r;

	// Get the Template & Create the Window, it is set up in the resource fork
	// to not be initially visible 


	pb.fileParam.ioCompletion = 0l;
	pb.fileParam.ioNamePtr = fsSpec->name;
	pb.fileParam.ioVRefNum = fsSpec->vRefNum;
	pb.fileParam.ioDirID = fsSpec->parID;
	pb.fileParam.ioFDirIndex = 0;

	if ((oe = PBHGetFInfo(&pb,FALSE)) != noErr) {
		OSErrorAlert(ES_Caution, "PBHGetFInfo", oe);
		return;
	}

	// fileSize = pb.fileParam.ioFlLgLen;	// Data Fork Length!!!!
	// GetEOF(refNum,&fileSize);

	dWin = (EditWindowPtr) NewPtrClear(sizeof(EditWindowRecord));
	if (dWin == NULL) {
		FSClose(refNum);
		ErrorAlert(ES_Caution, "Can't allocate new window");
		return;
	}

	if (gForkMode == FM_Data || (pb.fileParam.ioFlLgLen > 0 && gForkMode == FM_Smart)) {
		// Open Data Fork
		dWin->fork = FT_Data;
		oe = HOpen(fsSpec->vRefNum,fsSpec->parID,fsSpec->name,fsRdPerm,&refNum);
		if (oe == fnfErr) {
			ParamText(fsSpec->name,"\pdata","\p","\p");
			if (CautionAlert(NoForkALRT,NULL) != 2)
				return;
			oe = HCreate(fsSpec->vRefNum,fsSpec->parID,fsSpec->name,
						pb.fileParam.ioFlFndrInfo.fdCreator,
						pb.fileParam.ioFlFndrInfo.fdType);
			if (oe != noErr) {
				OSErrorAlert(ES_Caution, "HCreate", oe);
				return;
			}
			oe = HOpen(fsSpec->vRefNum,fsSpec->parID,fsSpec->name,fsRdPerm,&refNum);
		}
		if (oe != noErr) {
			OSErrorAlert(ES_Caution, "HOpen", oe);
			return;
		}
		dWin->fileSize = pb.fileParam.ioFlLgLen;
	}
	else {
		// Open Resource Fork
		dWin->fork = FT_Resource;
		oe = HOpenRF(fsSpec->vRefNum,fsSpec->parID,fsSpec->name,fsRdPerm,&refNum);
		if (oe == fnfErr) {
			ParamText(fsSpec->name,"\presource","\p","\p");
			if (CautionAlert(NoForkALRT,NULL) != 2)
				return;
			HCreateResFile(fsSpec->vRefNum,fsSpec->parID,fsSpec->name);
			if ((oe = ResError()) != noErr) {
				OSErrorAlert(ES_Caution, "HCreateResFile", oe);
				return;
			}
			oe = HOpenRF(fsSpec->vRefNum,fsSpec->parID,fsSpec->name,fsRdPerm,&refNum);
		}
		if (oe != noErr) {
			OSErrorAlert(ES_Caution, "HOpenRF", oe);
			return;
		}
		dWin->fileSize = pb.fileParam.ioFlRLgLen;
	}

	dWin->refNum = refNum;

	workSpec = *fsSpec;
	oe = FindFolder(-1, kTemporaryFolderType, kCreateFolder, 
					&workSpec.vRefNum,
					&workSpec.parID);
	if (oe != noErr) {
		OSErrorAlert(ES_Caution, "FindFolder", oe);
		return;
	}
	if (workSpec.name[0] < 31) {
		workSpec.name[0]++;
		workSpec.name[workSpec.name[0]] = 'w';
	}
	else
		workSpec.name[31] ^= 0x10;
	InsureNameIsUnique(&workSpec);
	oe = HCreate(workSpec.vRefNum,workSpec.parID,workSpec.name,CREATOR, '????');
	if (oe != noErr) {
		OSErrorAlert(ES_Caution, "HCreate", oe);
		return;
	}
	oe = HOpen(workSpec.vRefNum,workSpec.parID,workSpec.name,fsRdWrPerm,&refNum);
	if (oe != noErr) {
		OSErrorAlert(ES_Caution, "HOpen", oe);
		return;
	}

	dWin->workSpec = workSpec;
	dWin->workRefNum = refNum;
	dWin->workBytesWritten = 0L;

	dWin->fileType = pb.fileParam.ioFlFndrInfo.fdType;
	dWin->creator = pb.fileParam.ioFlFndrInfo.fdCreator;
	dWin->creationDate =  pb.fileParam.ioFlCrDat;

	theWindow = InitObjectWindow(MainWIND, (ObjectWindowPtr) dWin,false);
	SetRect(&r, 0, 0, MaxWindowWidth+SBarSize-1, gMaxHeight - 64);

	dWin->linesPerPage = ((r.bottom - SBarSize) - TopMargin - BotMargin - HeaderHeight)/LineHeight;

	if ((dWin->linesPerPage-1) * 16L > dWin->fileSize)
		r.bottom -= LineHeight * (((dWin->linesPerPage-1) * 16L) - dWin->fileSize)/16;
	if (r.bottom < SBarSize+TopMargin+BotMargin+HeaderHeight+LineHeight*3)
		r.bottom = SBarSize+TopMargin+BotMargin+HeaderHeight+LineHeight*3;

	SizeWindow(theWindow,r.right-1,r.bottom,true);
	SetRect(&r, 0, 0, MaxWindowWidth+SBarSize-1, gMaxHeight - 44);
	OffsetRect(&r,3,41);
	((WStateData *) *((WindowPeek) theWindow)->dataHandle)->stdState = r;
	SetWTitle(theWindow, fsSpec->name);
	dWin->fsSpec = *fsSpec;
	dWin->destSpec = dWin->fsSpec;

	((ObjectWindowPtr) theWindow)->Draw = MyDraw;
	((ObjectWindowPtr) theWindow)->Idle = MyIdle;
	((ObjectWindowPtr) theWindow)->HandleClick = MyHandleClick;
	((ObjectWindowPtr) theWindow)->Dispose = DisposeEditWindow;
	((ObjectWindowPtr) theWindow)->ProcessKey = MyProcessKey;
	((ObjectWindowPtr) theWindow)->Save = SaveContents;
	((ObjectWindowPtr) theWindow)->SaveAs = SaveAsContents;
	((ObjectWindowPtr) theWindow)->Revert = RevertContents;
	
	// Show the window
	ShowWindow(theWindow);

	// Make it the current grafport
	SetPort(theWindow);
	
	dWin->linesPerPage = ((theWindow->portRect.bottom - SBarSize) - TopMargin - BotMargin - HeaderHeight)/LineHeight;
	dWin->startSel = dWin->endSel = 0L;
	dWin->editMode = EM_Hex;

	SetupScrollBars(dWin);

	LoadFile(dWin);
}

void DisposeEditWindow(WindowPtr theWindow)
{
	EditWindowPtr	dWin = (EditWindowPtr) theWindow;

	UnloadFile(dWin);
	if (dWin->refNum)
		FSClose(dWin->refNum);
	if (dWin->workRefNum) {
		FSClose(dWin->workRefNum);
		HDelete(dWin->workSpec.vRefNum, dWin->workSpec.parID, dWin->workSpec.name);
	}
	DefaultDispose(theWindow);
	MyAdjustMenus();
}

Boolean	CloseEditWindow(WindowPtr theWindow)
{
	Str63			fileName;
	EditWindowPtr	dWin = (EditWindowPtr) theWindow;

	MySetCursor(C_Arrow);

	if (dWin->dirtyFlag) {
		GetWTitle(theWindow, fileName);
		ParamText(fileName, "\p","\p","\p");
		switch (Alert(SaveChangesALRT, NULL)) {
		case OK:		
			SaveContents(theWindow);	
			break;
		case Cancel:
			return false;
		case 3:
			// Discard
			break;
		}
	}
	((ObjectWindowPtr) dWin)->Dispose(theWindow);
	return true;
}

Boolean CloseAllEditWindows()
{
	WindowPeek	wp;
	wp = (WindowPeek) FrontWindow();
	while (wp) {
		if (wp->windowKind < 0)
		 	CloseDeskAcc(wp->windowKind);
		else if (wp->refCon == MyWindowID) {
			if (!CloseEditWindow((WindowPtr) wp))
				return false;
		}
		else if ((WindowPtr) wp == gSearchWin) {
			DisposDialog(gSearchWin);
			gSearchWin = NULL;
		}
		else
			return false;
		wp = (WindowPeek) FrontWindow();
	}
	return true;
}

// Respond to an update event - BeginUpdate has already been called.
//

void MyDraw(WindowPtr theWindow)
{
	RgnHandle		rg1,rg2,rg3;
	Rect			r3;

	rg1 = NewRgn();
	rg2 = NewRgn();
	rg3 = NewRgn();
	RectRgn(rg1, &theWindow->portRect);
	RectRgn(rg2, &gBitMap.bounds);
	r3 = theWindow->portRect;
	r3.right -= SBarSize-1;
	r3.bottom -= SBarSize-1;
	RectRgn(rg3, &r3);
	SectRgn(rg2,rg3,rg2);	// Intersection of offscreen and valid content area
	DiffRgn(rg1,rg2,rg1);	// Subtract from whole content area
	EraseRgn(rg1);
	DisposeRgn(rg1);
	DisposeRgn(rg2);
	DisposeRgn(rg3);
	// Draw the grow icon in the corner
	// EraseRect(&theWindow->portRect);
	DrawControls(theWindow);
	DrawGrowIcon(theWindow);
	DrawPage((EditWindowPtr) theWindow);
	UpdateOnscreen(theWindow);
}

void UpdateOnscreen(WindowPtr theWindow)
{
	Rect			r1,r2,r3;
	EditWindowPtr	dWin;

	gCursorFlag = false;
	r1 = gBitMap.bounds;
	r2 = theWindow->portRect;
	r2.right -= SBarSize - 1;
	r2.bottom -= SBarSize - 1;
	SectRect(&r1,&r2,&r3);
	SetPort(theWindow);
	CopyBits(&gBitMap,&theWindow->portBits,&r3,&r3,srcCopy,0L);

	dWin = (EditWindowPtr) theWindow;
	if (dWin->endSel > dWin->startSel && 
		dWin->endSel >= dWin->editOffset &&
		dWin->startSel < dWin->editOffset+(dWin->linesPerPage<<4)) 
		InvertSelection(dWin);
}

void MyIdle(WindowPtr theWin, EventRecord *er)
{
	EditWindowPtr	dWin = (EditWindowPtr) theWin;
	Boolean			frontWindowFlag;
	Point			w;
	frontWindowFlag = (theWin == FrontWindow() &&
						dWin->oWin.active);
	if (frontWindowFlag) {
		w = er->where;
		SetPort(theWin);
		GlobalToLocal(&w);
		if (w.v >= HeaderHeight+TopMargin && 
			w.v < HeaderHeight+TopMargin+(dWin->linesPerPage*LineHeight))
		{
				if (w.h >= AddrPos+CharPos(HexStart) &&
					w.h < AddrPos+CharPos(HexStart)+(HexWidth<<4)) 
					MySetCursor(C_IBeam);
				else if (w.h >= AddrPos+CharPos(AsciiStart) &&
						 w.h < AddrPos+CharPos(AsciiStart)+(CharWidth<<4))
					MySetCursor(C_IBeam);
				else
					MySetCursor(C_Arrow);
		}
		else
			MySetCursor(C_Arrow);

		if (dWin->startSel == dWin->endSel) {
			if ((Ticks & 0x1F) < 0x10)
				CursorOn(theWin);
			else
				CursorOff(theWin);
		}
		if (gScrapCount != ScrapInfo.scrapCount) {
			EditChunk	**nc;
			Handle		h;
			long		size,offset;
			size = GetScrap(NULL, 'TEXT', &offset);
			if (size > 0) {
				nc = NewChunk(size,0,0,CT_Unwritten);
				if (nc == NULL)
					ErrorAlert(ES_Caution,"Not enough memory for desktop scrap");
				else {
					ReleaseEditScrap(dWin, &gScrapChunk);
					gScrapChunk = nc;
					GetScrap((*gScrapChunk)->data,'TEXT',&offset);
					(*gScrapChunk)->lastCtr = 1;	// Flag as external
				}
			}
			gScrapCount = ScrapInfo.scrapCount;
		}
	}
}


// Respond to a mouse-click - highlight cells until the user releases the button
//
void MyHandleClick(WindowPtr theWin, Point where, EventRecord *er)
{
	Point	w;
	long	pos;
	EditWindowPtr	dWin = (EditWindowPtr) theWin;
	long	anchorPos = -1,sPos,ePos;

	SetPort(theWin);
	w = where;
	GlobalToLocal(&w);
	if (HandleControlClick(theWin,w))
		return;
	// Else handle editing chore
	CursorOff(theWin);
	if (w.v >= HeaderHeight+TopMargin && 
		w.v < HeaderHeight+TopMargin+(dWin->linesPerPage*LineHeight))
	{
		do {
			AutoScroll(dWin, w);

			if (w.h >= AddrPos+CharPos(HexStart) &&
				w.h < AddrPos+CharPos(HexStart)+(HexWidth<<4)) 
			{

				pos = ((w.v - (HeaderHeight+TopMargin))/LineHeight)*16 +
					  (w.h - (AddrPos+CharPos(HexStart))+12) / HexWidth;
				dWin->editMode = EM_Hex;
			}
			else if (w.h >= AddrPos+CharPos(AsciiStart) &&
					 w.h < AddrPos+CharPos(AsciiStart)+(CharWidth<<4))
			{
				pos = ((w.v - (HeaderHeight+TopMargin))/LineHeight)*16 +
					  (w.h - (AddrPos+CharPos(AsciiStart))+3) / CharWidth;
				dWin->editMode = EM_Ascii;
			}
			else {
				goto GetMouseLabel;
			}
			pos += dWin->editOffset;
			if (pos < dWin->editOffset)
				pos = dWin->editOffset;
			if (pos > dWin->editOffset+(dWin->linesPerPage<<4))
				pos = dWin->editOffset+(dWin->linesPerPage<<4);
			if (pos > dWin->fileSize)
				pos = dWin->fileSize;
			if (anchorPos == -1) {
				if (er->modifiers & shiftKey)
					anchorPos = (pos < dWin->startSel)? dWin->endSel : dWin->startSel;
				else
					anchorPos = pos;
			}
			sPos = pos < anchorPos? pos : anchorPos;
			ePos = pos > anchorPos? pos : anchorPos;
			if (ePos > dWin->fileSize)
				ePos = dWin->fileSize;

			if (sPos != dWin->startSel ||
				ePos != dWin->endSel) {
				dWin->startSel = sPos;
				dWin->endSel = ePos;
				UpdateOnscreen(theWin);
			}

	GetMouseLabel:
			GetMouse(&w);
		} while (WaitMouseUp());
	}
}

void DrawHeader(EditWindowPtr dWin, Rect *r)
{
	TextFont(monaco);
	TextSize(9);
	TextFace(bold);
	TextMode(srcCopy);

	MoveTo(r->left,r->top+HeaderHeight-1);
	LineTo(r->right,r->top+HeaderHeight-1);

	MoveTo(20, r->top+HeaderHeight-DescendHeight-2);
	if (gPrefs.decimalAddr)
		sprintf(gBuffer,"Length: %7ld    Type: %4.4s    Creator: %4.4s    Fork: %s", 
			dWin->fileSize,&dWin->fileType, &dWin->creator,
			((dWin->fork == FT_Data)? "data" : "rsrc"));
	else
		sprintf(gBuffer,"Length: %6lXh    Type: %4.4s    Creator: %4.4s    Fork: %s", 
			dWin->fileSize,&dWin->fileType, &dWin->creator,
			((dWin->fork == FT_Data)? "data" : "rsrc"));
	DrawText(gBuffer,0,strlen(gBuffer));
}

void DrawFooter(EditWindowPtr dWin, Rect *r, short pageNbr, short nbrPages)
{
	unsigned long	tim;
	DateTimeRec		dat;
	Str31			fileName;

	TextFont(monaco);
	TextSize(9);
	TextFace(0);
	TextMode(srcCopy);

	GetDateTime(&tim);
	Secs2Date(tim,&dat);

	MoveTo(r->left,r->top);
	LineTo(r->right,r->top);

	sprintf(gBuffer, "%02d/%02d/%02d %02d:%02d",
		dat.month,dat.day,dat.year-1900,dat.hour,dat.minute);

	MoveTo(20, r->top+FooterHeight-DescendHeight-2);
	DrawText(gBuffer,0,strlen(gBuffer));

	GetWTitle((WindowPtr) dWin, fileName);
	sprintf(gBuffer,"File: %.*s", fileName[0],&fileName[1]);
	MoveTo((r->left+r->right)/2 - TextWidth(gBuffer,0,strlen(gBuffer))/2,
	 		r->top+FooterHeight-DescendHeight-2);
	DrawText(gBuffer,0,strlen(gBuffer));
	
	sprintf(gBuffer,"%d of %d",pageNbr,nbrPages);
	MoveTo(r->right - TextWidth(gBuffer,0,strlen(gBuffer)) - 8, 
			r->top+FooterHeight-DescendHeight-2);
	DrawText(gBuffer,0,strlen(gBuffer));
}

void DrawDump(EditWindowPtr dWin, Rect *r, long sAddr, long eAddr)
{
	short	y;
	short	hexPos;
	short	asciiPos;
	register short	i,ch,ch1,ch2;
	long	addr;
	TextFont(monaco);
	TextSize(9);
	TextFace(0);
	TextMode(srcCopy);

	addr = sAddr - (sAddr % 16);
	for (y = r->top; y < r->bottom && addr < eAddr; y += LineHeight) {
		MoveTo(AddrPos,y);
		if (gPrefs.decimalAddr)
			sprintf(gBuffer,"%7ld: ", addr);
		else
			sprintf(gBuffer,"%06lX:  ", addr);
		hexPos = HexStart;
		asciiPos = AsciiStart;
		for (i = 16; i; --i,++addr) {
			if (addr >= sAddr && addr < eAddr) {
				ch = GetByte(dWin, addr);
				ch1 = ch2 = ch;
				ch1 >>= 4;
				ch2 &= 0x0F;
				gBuffer[hexPos++] = ch1 + ((ch1 < 10)? '0' : ('A'-10));
				gBuffer[hexPos++] = ch2 + ((ch2 < 10)? '0' : ('A'-10));
				gBuffer[hexPos++] = ' ';
				gBuffer[asciiPos++] = ((ch >= ' ' && ch <= gHighChar)? ch : '.');
			}
			else {
				gBuffer[hexPos++] = ' ';
				gBuffer[hexPos++] = ' ';
				gBuffer[hexPos++] = ' ';
				gBuffer[asciiPos++] = ' ';
			}
		}
		gBuffer[57] = ' ';
		gBuffer[58] = ' ';
		MoveTo(AddrPos,y);
		DrawText(gBuffer,0,75);
	}
}

void DrawPage(EditWindowPtr dWin)
{
	GrafPtr			savePort;
	Rect			r;
	BitMap			realBits;

#if PROFILE
	_profile = 1;
#endif
	GetPort(&savePort);
	SetPort((GrafPtr) dWin);

	realBits = ((GrafPtr) dWin)->portBits;
	SetPortBits(&gBitMap);

	r = ((WindowPtr) dWin)->portRect;
	r.right -= (SBarSize - 1);
//	r.bottom -= (SBarSize - 1);

	if (r.right - r.left > gBitMap.bounds.right - gBitMap.bounds.left ||
		r.bottom - r.top > gBitMap.bounds.bottom - gBitMap.bounds.top)
		DebugStr("\pOy!");

	EraseRect(&r);

	DrawHeader(dWin, &r);

	r.top += (HeaderHeight+TopMargin+LineHeight-DescendHeight);
	r.bottom -= (SBarSize+DescendHeight+BotMargin);

	DrawDump(dWin, &r, dWin->editOffset, dWin->fileSize);

	// if (dWin->endSel > dWin->startSel && 
	//	dWin->endSel >= dWin->editOffset &&
	//	dWin->startSel < dWin->editOffset+(dWin->linesPerPage<<4)) 
	//	InvertSelection(dWin);

	SetPortBits(&realBits);
	SetPort(savePort);

#if PROFILE
	_profile = 0;
#endif
}

void InvertSelection(EditWindowPtr	dWin)
{
	// Invert Selection
	Rect	r;
	long	x;
	long	start,end;
	short	startX,endX;
	Boolean	frontFlag;

	frontFlag = ((WindowPtr) dWin == FrontWindow() &&
				 dWin->oWin.active);

	if (dWin->endSel <= dWin->startSel)
		return;

	start = dWin->startSel - dWin->editOffset;
	if (start < 0)
		start = 0;
	end = (dWin->endSel-1) - dWin->editOffset;
	if (end > (dWin->linesPerPage<<4)-1)
		end = (dWin->linesPerPage<<4)-1;

	PenMode(patXor);
	
	startX = ColNbr(start);
	endX = ColNbr(end);

	if (!frontFlag) {
		if (LineNbr(start) < LineNbr(end)) {
			// Invert Hex
			r.top = HeaderHeight+TopMargin+LineNbr(start)*LineHeight;
			r.bottom = r.top+LineHeight;
			r.left = AddrPos+CharPos(HexStart)+HexPos(startX)-3;
			r.right = AddrPos+CharPos(HexStart)+HexPos(16)-3;

			MoveTo(AddrPos+CharPos(HexStart)-3,r.bottom);
			if (gColorQDFlag)
				HiliteMode &= ~(1 << hiliteBit);	// Hilite using Hilite Color
			LineTo(r.left,r.bottom);
			if (gColorQDFlag)
				HiliteMode &= ~(1 << hiliteBit);	// Hilite using Hilite Color
			LineTo(r.left,r.top);
			if (dWin->startSel >= dWin->editOffset) {
				if (gColorQDFlag)
				HiliteMode &= ~(1 << hiliteBit);	// Hilite using Hilite Color
				LineTo(r.right,r.top);
			}
			else
				MoveTo(r.right,r.top);
			if (gColorQDFlag)
				HiliteMode &= ~(1 << hiliteBit);	// Hilite using Hilite Color
			LineTo(r.right,r.bottom);

			// Outline Box around Ascii
			r.left = AddrPos+CharPos(AsciiStart)+CharPos(startX)-1;
			r.right = AddrPos+CharPos(AsciiStart)+CharPos(16);
			
			MoveTo(AddrPos+CharPos(AsciiStart),r.bottom);
			if (gColorQDFlag)
				HiliteMode &= ~(1 << hiliteBit);	// Hilite using Hilite Color
			LineTo(r.left,r.bottom);

			if (gColorQDFlag)
				HiliteMode &= ~(1 << hiliteBit);	// Hilite using Hilite Color
			LineTo(r.left,r.top);
			if (dWin->startSel >= dWin->editOffset) {
				if (gColorQDFlag)
				HiliteMode &= ~(1 << hiliteBit);	// Hilite using Hilite Color
				LineTo(r.right,r.top);
			}
			else
				MoveTo(r.right,r.top);
			if (gColorQDFlag)
				HiliteMode &= ~(1 << hiliteBit);	// Hilite using Hilite Color
			LineTo(r.right,r.bottom);

			if (LineNbr(start) < LineNbr(end)-1) {
				r.top = HeaderHeight+TopMargin+LineNbr(start)*LineHeight+LineHeight;
				r.bottom = HeaderHeight+TopMargin+LineNbr(end)*LineHeight;
				r.left = AddrPos+CharPos(HexStart)-3;
				r.right = AddrPos+CharPos(HexStart)+HexPos(16)-3;
				MoveTo(r.left,r.top);
				if (gColorQDFlag)
				HiliteMode &= ~(1 << hiliteBit);	// Hilite using Hilite Color
				LineTo(r.left,r.bottom);
				MoveTo(r.right,r.top);
				if (gColorQDFlag)
				HiliteMode &= ~(1 << hiliteBit);	// Hilite using Hilite Color
				LineTo(r.right,r.bottom);

				r.left = AddrPos+CharPos(AsciiStart)-1;
				r.right = AddrPos+CharPos(AsciiStart)+CharPos(16);
				MoveTo(r.left,r.top);
				if (gColorQDFlag)
				HiliteMode &= ~(1 << hiliteBit);	// Hilite using Hilite Color
				LineTo(r.left,r.bottom);
				MoveTo(r.right,r.top);
				if (gColorQDFlag)
				HiliteMode &= ~(1 << hiliteBit);	// Hilite using Hilite Color
				LineTo(r.right,r.bottom);
			}
			r.top = HeaderHeight+TopMargin+LineNbr(end)*LineHeight;
			r.bottom = r.top+LineHeight;
			r.left = AddrPos+CharPos(HexStart)-3;
			r.right = AddrPos+CharPos(HexStart)+HexPos(endX)+HexWidth-3;
			MoveTo(r.left,r.top);
			if (gColorQDFlag)
				HiliteMode &= ~(1 << hiliteBit);	// Hilite using Hilite Color
			LineTo(r.left,r.bottom);
			if (dWin->endSel < dWin->editOffset+dWin->linesPerPage*16) {
				if (gColorQDFlag)
				HiliteMode &= ~(1 << hiliteBit);	// Hilite using Hilite Color
				LineTo(r.right,r.bottom);
			}
			else
				MoveTo(r.right,r.bottom);
			if (gColorQDFlag)
				HiliteMode &= ~(1 << hiliteBit);	// Hilite using Hilite Color
			LineTo(r.right,r.top);
			if (gColorQDFlag)
				HiliteMode &= ~(1 << hiliteBit);	// Hilite using Hilite Color
			LineTo(AddrPos+CharPos(HexStart)+HexPos(16)-3,r.top);

			r.left = AddrPos+CharPos(AsciiStart)-1;
			r.right = AddrPos+CharPos(AsciiStart)+CharPos(endX)+CharWidth-1;
			MoveTo(r.left,r.top);
			if (gColorQDFlag)
				HiliteMode &= ~(1 << hiliteBit);	// Hilite using Hilite Color
			LineTo(r.left,r.bottom-1);
			if (dWin->endSel < dWin->editOffset+dWin->linesPerPage*16) {
				if (gColorQDFlag)
				HiliteMode &= ~(1 << hiliteBit);	// Hilite using Hilite Color
				LineTo(r.right,r.bottom-1);
			}
			else
				MoveTo(r.right,r.bottom-1);
			if (gColorQDFlag)
				HiliteMode &= ~(1 << hiliteBit);	// Hilite using Hilite Color
			LineTo(r.right,r.top);
			if (gColorQDFlag)
				HiliteMode &= ~(1 << hiliteBit);	// Hilite using Hilite Color
			LineTo(AddrPos+CharPos(AsciiStart)+CharPos(16),r.top);
		}
		else {
			r.top = HeaderHeight+TopMargin+LineNbr(start)*LineHeight;
			r.bottom = r.top+LineHeight;
			r.left = AddrPos+CharPos(HexStart)+HexPos(startX)-3;
			r.right = AddrPos+CharPos(HexStart)+HexPos(endX)+HexWidth-3;
			MoveTo(r.left,r.top);
			if (gColorQDFlag)
				HiliteMode &= ~(1 << hiliteBit);	// Hilite using Hilite Color
			LineTo(r.left,r.bottom);
			if (dWin->endSel < dWin->editOffset+dWin->linesPerPage*16) {
				if (gColorQDFlag)
				HiliteMode &= ~(1 << hiliteBit);	// Hilite using Hilite Color
				LineTo(r.right,r.bottom);
			}
			else
				MoveTo(r.right,r.bottom);
			if (gColorQDFlag)
				HiliteMode &= ~(1 << hiliteBit);	// Hilite using Hilite Color
			LineTo(r.right,r.top);
			if (dWin->startSel >= dWin->editOffset) {
				if (gColorQDFlag)
				HiliteMode &= ~(1 << hiliteBit);	// Hilite using Hilite Color
				LineTo(r.left,r.top);
			}

			r.left = AddrPos+CharPos(AsciiStart)+CharPos(startX)-1;
			r.right = AddrPos+CharPos(AsciiStart)+CharPos(endX)+CharWidth-1;

			MoveTo(r.left,r.top);
			if (gColorQDFlag)
				HiliteMode &= ~(1 << hiliteBit);	// Hilite using Hilite Color
			LineTo(r.left,r.bottom-1);
			if (dWin->endSel < dWin->editOffset+dWin->linesPerPage*16) {
				if (gColorQDFlag)
				HiliteMode &= ~(1 << hiliteBit);	// Hilite using Hilite Color
				LineTo(r.right,r.bottom-1);
			}
			else
				MoveTo(r.right,r.bottom-1);
			if (gColorQDFlag)
				HiliteMode &= ~(1 << hiliteBit);	// Hilite using Hilite Color
			LineTo(r.right,r.top);
			if (dWin->startSel >= dWin->editOffset) {
				if (gColorQDFlag)
				HiliteMode &= ~(1 << hiliteBit);	// Hilite using Hilite Color
				LineTo(r.left,r.top);
			}
		}
	}
	else {
		if (dWin->editMode == EM_Hex) {
			if (LineNbr(start) < LineNbr(end)) {
	
				// Invert Hex
				r.top = HeaderHeight+TopMargin+LineNbr(start)*LineHeight;
				r.bottom = r.top+LineHeight;
				r.left = AddrPos+CharPos(HexStart)+HexPos(startX)-3;
				r.right = AddrPos+CharPos(HexStart)+HexPos(16)-3;
	
				if (gColorQDFlag)
				HiliteMode &= ~(1 << hiliteBit);	// Hilite using Hilite Color
				InvertRect(&r);
	
	
				// Outline Box around Ascii
				r.left = AddrPos+CharPos(AsciiStart)+CharPos(startX)-1;
				r.right = AddrPos+CharPos(AsciiStart)+CharPos(16);
				
				MoveTo(AddrPos+CharPos(AsciiStart),r.bottom);
				if (gColorQDFlag)
				HiliteMode &= ~(1 << hiliteBit);	// Hilite using Hilite Color
				LineTo(r.left,r.bottom);
	
				if (gColorQDFlag)
				HiliteMode &= ~(1 << hiliteBit);	// Hilite using Hilite Color
				LineTo(r.left,r.top);
				if (dWin->startSel >= dWin->editOffset) {
					if (gColorQDFlag)
				HiliteMode &= ~(1 << hiliteBit);	// Hilite using Hilite Color
					LineTo(r.right,r.top);
				}
				else
					MoveTo(r.right,r.top);
				if (gColorQDFlag)
				HiliteMode &= ~(1 << hiliteBit);	// Hilite using Hilite Color
				LineTo(r.right,r.bottom);
	
				if (LineNbr(start) < LineNbr(end)-1) {
					r.top = HeaderHeight+TopMargin+LineNbr(start)*LineHeight+LineHeight;
					r.bottom = HeaderHeight+TopMargin+LineNbr(end)*LineHeight;
					r.left = AddrPos+CharPos(HexStart)-3;
					r.right = AddrPos+CharPos(HexStart)+HexPos(16)-3;
					if (gColorQDFlag)
				HiliteMode &= ~(1 << hiliteBit);	// Hilite using Hilite Color
					InvertRect(&r);
	
					r.left = AddrPos+CharPos(AsciiStart)-1;
					r.right = AddrPos+CharPos(AsciiStart)+CharPos(16);
					MoveTo(r.left,r.top);
					if (gColorQDFlag)
				HiliteMode &= ~(1 << hiliteBit);	// Hilite using Hilite Color
					LineTo(r.left,r.bottom);
					MoveTo(r.right,r.top);
					if (gColorQDFlag)
				HiliteMode &= ~(1 << hiliteBit);	// Hilite using Hilite Color
					LineTo(r.right,r.bottom);
				}
				r.top = HeaderHeight+TopMargin+LineNbr(end)*LineHeight;
				r.bottom = r.top+LineHeight;
				r.left = AddrPos+CharPos(HexStart)-3;
				r.right = AddrPos+CharPos(HexStart)+HexPos(endX)+HexWidth-3;
				if (gColorQDFlag)
				HiliteMode &= ~(1 << hiliteBit);	// Hilite using Hilite Color
				InvertRect(&r);
	
				r.left = AddrPos+CharPos(AsciiStart)-1;
				r.right = AddrPos+CharPos(AsciiStart)+CharPos(endX)+CharWidth-1;
				MoveTo(r.left,r.top);
				if (gColorQDFlag)
				HiliteMode &= ~(1 << hiliteBit);	// Hilite using Hilite Color
				LineTo(r.left,r.bottom-1);
				if (dWin->endSel < dWin->editOffset+dWin->linesPerPage*16) {
					if (gColorQDFlag)
				HiliteMode &= ~(1 << hiliteBit);	// Hilite using Hilite Color
					LineTo(r.right,r.bottom-1);
				}
				else
					MoveTo(r.right,r.bottom-1);
				if (gColorQDFlag)
				HiliteMode &= ~(1 << hiliteBit);	// Hilite using Hilite Color
				LineTo(r.right,r.top);
				if (gColorQDFlag)
				HiliteMode &= ~(1 << hiliteBit);	// Hilite using Hilite Color
				LineTo(AddrPos+CharPos(AsciiStart)+CharPos(16),r.top);
			}
			else {
				r.top = HeaderHeight+TopMargin+LineNbr(start)*LineHeight;
				r.bottom = r.top+LineHeight;
				r.left = AddrPos+CharPos(HexStart)+HexPos(startX)-3;
				r.right = AddrPos+CharPos(HexStart)+HexPos(endX)+HexWidth-3;
				if (gColorQDFlag)
				HiliteMode &= ~(1 << hiliteBit);	// Hilite using Hilite Color
				InvertRect(&r);
	
				r.left = AddrPos+CharPos(AsciiStart)+CharPos(startX)-1;
				r.right = AddrPos+CharPos(AsciiStart)+CharPos(endX)+CharWidth-1;
	
				MoveTo(r.left,r.top);
				if (gColorQDFlag)
				HiliteMode &= ~(1 << hiliteBit);	// Hilite using Hilite Color
				LineTo(r.left,r.bottom-1);
				if (dWin->endSel < dWin->editOffset+dWin->linesPerPage*16) {
					if (gColorQDFlag)
				HiliteMode &= ~(1 << hiliteBit);	// Hilite using Hilite Color
					LineTo(r.right,r.bottom-1);
				}
				else
					MoveTo(r.right,r.bottom-1);
				if (gColorQDFlag)
				HiliteMode &= ~(1 << hiliteBit);	// Hilite using Hilite Color
				LineTo(r.right,r.top);
				if (dWin->startSel >= dWin->editOffset) {
					if (gColorQDFlag)
				HiliteMode &= ~(1 << hiliteBit);	// Hilite using Hilite Color
					LineTo(r.left,r.top);
				}
			}
		}
		else {
			// Ascii Mode!!
			//
			if (LineNbr(start) < LineNbr(end)) {
	
				// Invert Hex
				r.top = HeaderHeight+TopMargin+LineNbr(start)*LineHeight;
				r.bottom = r.top+LineHeight;
				r.left = AddrPos+CharPos(HexStart)+HexPos(startX)-3;
				r.right = AddrPos+CharPos(HexStart)+HexPos(16)-3;
	
				MoveTo(AddrPos+CharPos(HexStart)-3,r.bottom);
				if (gColorQDFlag)
				HiliteMode &= ~(1 << hiliteBit);	// Hilite using Hilite Color
				LineTo(r.left,r.bottom);
				if (gColorQDFlag)
				HiliteMode &= ~(1 << hiliteBit);	// Hilite using Hilite Color
				LineTo(r.left,r.top);
				if (dWin->startSel >= dWin->editOffset) {
					if (gColorQDFlag)
				HiliteMode &= ~(1 << hiliteBit);	// Hilite using Hilite Color
					LineTo(r.right,r.top);
				}
				else
					MoveTo(r.right,r.top);
				if (gColorQDFlag)
				HiliteMode &= ~(1 << hiliteBit);	// Hilite using Hilite Color
				LineTo(r.right,r.bottom);
	
				// Outline Box around Ascii
				r.left = AddrPos+CharPos(AsciiStart)+CharPos(startX)-1;
				r.right = AddrPos+CharPos(AsciiStart)+CharPos(16)-1;
				
				if (gColorQDFlag)
				HiliteMode &= ~(1 << hiliteBit);	// Hilite using Hilite Color
				InvertRect(&r);
	
				if (LineNbr(start) < LineNbr(end)-1) {
					r.top = HeaderHeight+TopMargin+LineNbr(start)*LineHeight+LineHeight;
					r.bottom = HeaderHeight+TopMargin+LineNbr(end)*LineHeight;
					r.left = AddrPos+CharPos(HexStart)-3;
					r.right = AddrPos+CharPos(HexStart)+HexPos(16)-3;
					MoveTo(r.left,r.top);
					if (gColorQDFlag)
				HiliteMode &= ~(1 << hiliteBit);	// Hilite using Hilite Color
					LineTo(r.left,r.bottom);
					MoveTo(r.right,r.top);
					if (gColorQDFlag)
				HiliteMode &= ~(1 << hiliteBit);	// Hilite using Hilite Color
					LineTo(r.right,r.bottom);
	
					r.left = AddrPos+CharPos(AsciiStart)-1;
					r.right = AddrPos+CharPos(AsciiStart)+CharPos(16)-1;
					if (gColorQDFlag)
				HiliteMode &= ~(1 << hiliteBit);	// Hilite using Hilite Color
					InvertRect(&r);
				}
				r.top = HeaderHeight+TopMargin+LineNbr(end)*LineHeight;
				r.bottom = r.top+LineHeight;
				r.left = AddrPos+CharPos(HexStart)-3;
				r.right = AddrPos+CharPos(HexStart)+HexPos(endX)+HexWidth-3;
				MoveTo(r.left,r.top);
				if (gColorQDFlag)
				HiliteMode &= ~(1 << hiliteBit);	// Hilite using Hilite Color
				LineTo(r.left,r.bottom);
				if (dWin->endSel < dWin->editOffset+dWin->linesPerPage*16) {
					if (gColorQDFlag)
				HiliteMode &= ~(1 << hiliteBit);	// Hilite using Hilite Color
					LineTo(r.right,r.bottom);
				}
				else
					MoveTo(r.right,r.bottom);
				if (gColorQDFlag)
				HiliteMode &= ~(1 << hiliteBit);	// Hilite using Hilite Color
				LineTo(r.right,r.top);
				if (gColorQDFlag)
				HiliteMode &= ~(1 << hiliteBit);	// Hilite using Hilite Color
				LineTo(AddrPos+CharPos(HexStart)+HexPos(16)-3,r.top);
	
				r.left = AddrPos+CharPos(AsciiStart)-1;
				r.right = AddrPos+CharPos(AsciiStart)+CharPos(endX)+CharWidth-1;
				if (gColorQDFlag)
				HiliteMode &= ~(1 << hiliteBit);	// Hilite using Hilite Color
				InvertRect(&r);
			}
			else {
				r.top = HeaderHeight+TopMargin+LineNbr(start)*LineHeight;
				r.bottom = r.top+LineHeight;
				r.left = AddrPos+CharPos(HexStart)+HexPos(startX)-3;
				r.right = AddrPos+CharPos(HexStart)+HexPos(endX)+HexWidth-3;
				MoveTo(r.left,r.top);
				if (gColorQDFlag)
				HiliteMode &= ~(1 << hiliteBit);	// Hilite using Hilite Color
				LineTo(r.left,r.bottom);
				if (dWin->endSel < dWin->editOffset+dWin->linesPerPage*16) {
					if (gColorQDFlag)
				HiliteMode &= ~(1 << hiliteBit);	// Hilite using Hilite Color
					LineTo(r.right,r.bottom);
				}
				else
					MoveTo(r.right,r.bottom);
				if (gColorQDFlag)
				HiliteMode &= ~(1 << hiliteBit);	// Hilite using Hilite Color
				LineTo(r.right,r.top);
				if (dWin->startSel >= dWin->editOffset) {
					if (gColorQDFlag)
				HiliteMode &= ~(1 << hiliteBit);	// Hilite using Hilite Color
					LineTo(r.left,r.top);
				}
	
				r.left = AddrPos+CharPos(AsciiStart)+CharPos(startX)-1;
				r.right = AddrPos+CharPos(AsciiStart)+CharPos(endX)+CharWidth-1;
				if (gColorQDFlag)
				HiliteMode &= ~(1 << hiliteBit);	// Hilite using Hilite Color
				InvertRect(&r);
			}
		}
	}
	PenMode(patCopy);
}

void PrintWindow(EditWindowPtr dWin)
{
	TPPrPort	printPort;
	TPrStatus	prStatus;
	GrafPtr		savePort;
	Boolean		ok;
	Rect		r;
	short		pageNbr,startPage,endPage,nbrPages;
	long		startAddr,endAddr,addr;
	short		linesPerPage;

	GetPort(&savePort);
	PrOpen();

	ok = PrValidate(gHPrint);
	ok = PrJobDialog(gHPrint);
	if (ok) {
		if (dWin->startSel == dWin->endSel) {
			startAddr = 0;
			endAddr = dWin->fileSize;
		}
		else {
			startAddr = dWin->startSel;
			endAddr = dWin->endSel;
		}

		printPort = PrOpenDoc(gHPrint,NULL,NULL);

		r = printPort->gPort.portRect;
		linesPerPage = ((r.bottom - FooterHeight) - TopMargin - BotMargin - HeaderHeight)/LineHeight;
		nbrPages = ((endAddr - startAddr)/16)/linesPerPage + 1;

		startPage = (**gHPrint).prJob.iFstPage;
		endPage = (**gHPrint).prJob.iLstPage;
		if (startPage > nbrPages) {
			PrCloseDoc(printPort);
			ErrorAlert(ES_Caution, "Selected pages are out of range 1-%d",nbrPages);
			goto ErrorExit;
		}
		addr = startAddr;

		if (endPage > nbrPages)
			endPage = nbrPages;

		for (pageNbr = 1; pageNbr <= nbrPages; ++pageNbr) {
			SetPort(&printPort->gPort);
			PrOpenPage(printPort, NULL);
	
			if (pageNbr >= startPage && pageNbr <= endPage) {
				r = printPort->gPort.portRect;
				DrawHeader(dWin, &r);
		
				r.top += (HeaderHeight+TopMargin+LineHeight-DescendHeight);
				r.bottom -= (FooterHeight + DescendHeight + BotMargin);
		
				DrawDump(dWin, &r, addr, endAddr);
	
				r.top = r.bottom + DescendHeight + BotMargin;
				r.bottom = r.top + FooterHeight;
				DrawFooter(dWin, &r, pageNbr, nbrPages);
			}

			addr += linesPerPage*16;
			addr -= (addr % 16);
			PrClosePage(printPort);
		}
		PrCloseDoc(printPort);
		if ((**gHPrint).prJob.bJDocLoop == bSpoolLoop && PrError() == noErr)
			PrPicFile(gHPrint, NULL, NULL, NULL, &prStatus);
	}
ErrorExit:
	PrClose();
	SetPort(savePort);
}

void ScrollToSelection(EditWindowPtr dWin, long pos, Boolean forceUpdate, Boolean centerFlag)
{
	long	curAddr,newAddr;
	curAddr = dWin->editOffset;
	if (pos >= curAddr && pos < curAddr+(dWin->linesPerPage<<4)) {
		if (forceUpdate) {
			DrawPage(dWin);
			UpdateOnscreen((WindowPtr) dWin);
		}
		AdjustScrollBars((WindowPtr) dWin,false);
		return;
	}
	if (centerFlag) {
		curAddr = pos - (pos % 16);
		curAddr -= 16 * (dWin->linesPerPage/2 - 1);
		// No need to adjust for limits, will be done by scroll routine
	}
	else {

		if (pos < curAddr) {
			// Scroll Up
			curAddr = pos;
			curAddr -= (curAddr % 16);
		}
		else {
			// Scroll Down
			curAddr = pos - (dWin->linesPerPage-1)*16;
			curAddr -= (curAddr % 16);
		}
	}
	HEditScrollToPosition(dWin, curAddr);
}

void OffsetSelection(EditWindowPtr dWin, short offset, Boolean shiftFlag)
{
	long	selWidth;
	Boolean	fullUpdate;
	selWidth = dWin->endSel - dWin->startSel;
	fullUpdate = shiftFlag || selWidth > 1;
	if (offset < 0) {
		if (dWin->startSel > 0) {
			dWin->startSel += offset;
			if (dWin->startSel < 0)
				dWin->startSel = 0;
			if (!shiftFlag) {
				dWin->endSel = dWin->startSel;
				CursorOff((WindowPtr) dWin);
			}
			ScrollToSelection(dWin, dWin->startSel, fullUpdate, false);
			if (!shiftFlag)
				CursorOn((WindowPtr) dWin);
		}
		else
			SysBeep(1);
	}
	else {
		if (dWin->endSel < dWin->fileSize) {
			dWin->endSel += offset;
			if (dWin->endSel > dWin->fileSize)
				dWin->endSel = dWin->fileSize;
			if (!shiftFlag) {
				dWin->startSel = dWin->endSel;
				CursorOff((WindowPtr) dWin);
			}
			ScrollToSelection(dWin, dWin->endSel, fullUpdate, false);
			if (!shiftFlag)
				CursorOn((WindowPtr) dWin);
		}
		else
			SysBeep(1);
	}
}

void MyProcessKey(WindowPtr theWin, EventRecord *er)
{
	short	charCode,keyCode;
	EditWindowPtr	dWin = (EditWindowPtr) theWin;

	keyCode = (er->message & keyCodeMask) >> 8;
	charCode = (er->message & charCodeMask);

	if (er->modifiers & cmdKey)
		return;

	switch (keyCode) {
	case  0x24:			// Return
		break;
	case  0x7B:			// Left
		OffsetSelection(dWin, -1, (er->modifiers & shiftKey)>0);
		break;
	case 0x7C:			// Right
		OffsetSelection(dWin, 1, (er->modifiers & shiftKey)>0);
		break;
	case 0x7E:			// Up
		OffsetSelection(dWin, -16, (er->modifiers & shiftKey)>0);
		break;
	case 0x7D:			// Down
		OffsetSelection(dWin, 16, (er->modifiers & shiftKey)>0);
		break;
	case 0x33:			// Delete
		if (dWin->endSel > dWin->startSel) {
			ClearSelection(dWin);
		}
		else if (dWin->startSel > 0L) {
			ObscureCursor();
			--dWin->startSel;
			ClearSelection(dWin);
		}
		else
			SysBeep(1);
		break;
	default:
		// Insert Ascii Text Into Area indicated by dWin->startSel - dWin->endSel
		// Delete Current Selection if > 0
		ObscureCursor();

		if (dWin->editMode == EM_Ascii) {
			if (dWin->endSel != dWin->lastTypePos ||
				dWin->startSel != dWin->lastTypePos)
				RememberOperation(dWin, EO_Typing, &gUndoRec);
			if (dWin->endSel > dWin->startSel)
				DeleteSelection(dWin);
			if (gOverwrite && dWin->startSel < dWin->fileSize - 1) {
				++dWin->endSel;
				DeleteSelection(dWin);
			}
			InsertCharacter(dWin, charCode);
			dWin->lastTypePos = dWin->startSel;
		}
		else {
			short	hexVal;

			if (charCode >= '0' && charCode <= '9')
				hexVal = charCode - '0';
			else if (charCode >= 'A' && charCode <= 'F')
				hexVal = 0x0A + charCode - 'A';
			else if (charCode >= 'a' && charCode <= 'f')
				hexVal = 0x0A + charCode - 'a';
			else {
				SysBeep(1);
				return;
			}

			if (dWin->endSel != dWin->lastTypePos ||
				dWin->startSel != dWin->lastTypePos) {
				dWin->loByteFlag = false;
				RememberOperation(dWin, EO_Typing, &gUndoRec);
			}
			if (dWin->endSel > dWin->startSel)
				DeleteSelection(dWin);

			if (dWin->loByteFlag) {
				--dWin->startSel;
				DeleteSelection(dWin);
				hexVal = hexVal | (dWin->lastNybble << 4);
				InsertCharacter(dWin, hexVal);
				dWin->loByteFlag = false;
			}
			else {
				if (gOverwrite && dWin->startSel < dWin->fileSize - 1) {
					++dWin->endSel;
					DeleteSelection(dWin);
				}
				InsertCharacter(dWin, hexVal);
				dWin->lastNybble = hexVal;
				dWin->loByteFlag = true;
			}
			dWin->lastTypePos = dWin->startSel;
		}
		break;
	}
}

void CursorOff(WindowPtr theWin)
{
	if (gCursorFlag) {
		gCursorFlag = false;
		SetPort(theWin);
		InvertRect(&gCursRect);
	}
}

void CursorOn(WindowPtr theWin)
{
	EditWindowPtr	dWin = (EditWindowPtr) theWin;
	long			start;
	Rect			r;

	if (!gCursorFlag &&
		 dWin->startSel >= dWin->editOffset &&
		dWin->startSel < dWin->editOffset+(dWin->linesPerPage<<4)) 
	{
		gCursorFlag = true;
		SetPort(theWin);
		start = dWin->startSel - dWin->editOffset;

		if (dWin->editMode == EM_Hex) {
			r.top = HeaderHeight+TopMargin+LineNbr(start)*LineHeight;
			r.bottom = r.top+LineHeight;
			r.left = AddrPos+CharPos(HexStart)+ColNbr(start)*HexWidth-3;
			r.right = r.left+2;
			InvertRect(&r);
			gCursRect = r;
		}
		else {
			r.top = HeaderHeight+TopMargin+LineNbr(start)*LineHeight;
			r.bottom = r.top+LineHeight;
			r.left = AddrPos+CharPos(AsciiStart)+ColNbr(start)*CharWidth-2;
			r.right = r.left+2;
			InvertRect(&r);
			gCursRect = r;
		}
	}
}

OSErr CopyFork(FSSpec *srcSpec, FSSpec *dstSpec, short forkType)
{
	short	sRefNum, dRefNum;
	OSErr	oe;
	Ptr		tBuffer;
	long	srcSize=0L,bufSize,count;

	tBuffer = NewPtr(32000);

	if (tBuffer == NULL) {
		ErrorAlert(ES_Caution, "Not Enough Memory");
		return 1;
	}
	if (forkType == FT_Resource) {
		oe = HOpenRF(srcSpec->vRefNum,srcSpec->parID,srcSpec->name,fsRdPerm,&sRefNum);
		if (oe != noErr) {
			OSErrorAlert(ES_Caution, "HOpenRF", oe);
			return oe;
		}
		oe = HOpenRF(dstSpec->vRefNum,dstSpec->parID,dstSpec->name,fsWrPerm,&dRefNum);
		if (oe != noErr) {
			OSErrorAlert(ES_Caution, "HOpenRF", oe);
			return oe;
		}
	}
	else {
		oe = HOpen(srcSpec->vRefNum,srcSpec->parID,srcSpec->name,fsRdPerm,&sRefNum);
		if (oe != noErr) {
			OSErrorAlert(ES_Caution, "HOpen", oe);
			return oe;
		}
		oe = HOpen(dstSpec->vRefNum,dstSpec->parID,dstSpec->name,fsWrPerm,&dRefNum);
		if (oe != noErr) {
			OSErrorAlert(ES_Caution, "HOpen", oe);
			return oe;
		}
	}
	GetEOF(sRefNum, &srcSize);
	SetEOF(dRefNum,0L);
	while (srcSize) {
		if (srcSize < 32000)
			bufSize = srcSize;
		else
			bufSize = 32000;
		srcSize -= bufSize;
		count = bufSize;
		oe = FSRead(sRefNum, &count, tBuffer);
		if (oe != noErr) {
			OSErrorAlert(ES_Caution,"FSRead",oe);
			goto ErrorExit;
		}
		oe = FSWrite(dRefNum, &count, tBuffer);
		if (oe != noErr) {
			OSErrorAlert(ES_Caution,"FSWrite",oe);
			goto ErrorExit;
		}
	}
	oe = noErr;
ErrorExit:
	FSClose(sRefNum);
	FSClose(dRefNum);
	DisposePtr(tBuffer);
	return oe;
}

void InsureNameIsUnique(FSSpec *tSpec)
{
	FInfo			fInfo;
	while (HGetFInfo(tSpec->vRefNum,
					 tSpec->parID,
					 tSpec->name, &fInfo) == noErr) {
		tSpec->name[1+MyRandom(tSpec->name[0])] = 0x80+MyRandom(64);
	}
}

void SaveContents(WindowPtr theWin)
{
	short			tRefNum=0;
	FSSpec			tSpec,bSpec;
	HParamBlockRec	pb;
	EditChunk		**cc;
	EditWindowPtr	dWin;
	long			count;
	OSErr			oe;

	dWin = (EditWindowPtr) theWin;
	if (dWin->destSpec.name[0] == 0) {
		SaveAsContents(theWin);
	}
	else {
		// Create temp file
		tSpec = dWin->destSpec;

		// If original file exists, write to temp folder
		if (dWin->refNum) {
			if (tSpec.name[0] < 31) {
				tSpec.name[0]++;
				tSpec.name[tSpec.name[0]] = 't';
			}
			else {
				tSpec.name[31] ^= 0x10;
			}
		}
		InsureNameIsUnique(&tSpec);

		HDelete(tSpec.vRefNum,tSpec.parID,tSpec.name);
		oe = HCreate(tSpec.vRefNum,tSpec.parID,tSpec.name,dWin->creator, dWin->fileType);
		if (oe != noErr) {
			OSErrorAlert(ES_Caution, "HCreate", oe);
			return;
		}

		// Preserve creation date of orig file if it exists
		if (dWin->creationDate) {
			pb.fileParam.ioCompletion = 0l;
			pb.fileParam.ioNamePtr = tSpec.name;
			pb.fileParam.ioVRefNum = tSpec.vRefNum;
			pb.fileParam.ioDirID = tSpec.parID;
			pb.fileParam.ioFDirIndex = 0;
		
			if ((oe = PBHGetFInfo(&pb,FALSE)) != noErr) {
				OSErrorAlert(ES_Caution, "PBHGetFInfo", oe);
				return;
			}
			// Reset dirID which PBHGeFInfo changes...
			pb.fileParam.ioFlCrDat = dWin->creationDate;
			pb.fileParam.ioDirID = tSpec.parID;
			if ((oe = PBHSetFInfo(&pb,FALSE)) != noErr) {
				OSErrorAlert(ES_Caution, "PBHSetFInfo", oe);
				return;
			}
		}
		// Preserve other fork if it exists
		if (dWin->refNum)
			if (CopyFork(&dWin->fsSpec,&tSpec, !dWin->fork) != noErr)
				return;

		// Open the temp file
		if (dWin->fork == FT_Resource) {
			oe = HOpenRF(tSpec.vRefNum,tSpec.parID,tSpec.name,fsWrPerm,&tRefNum);
			if (oe != noErr) {
				OSErrorAlert(ES_Caution, "HOpenRF", oe);
				return;
			}
		}
		else {
			oe = HOpen(tSpec.vRefNum,tSpec.parID,tSpec.name,fsWrPerm,&tRefNum);
			if (oe != noErr) {
				OSErrorAlert(ES_Caution, "HOpen", oe);
				return;
			}
		}
		
		// Save out to temp file
		cc = dWin->firstChunk;
		while (cc) {
			LoadChunk(dWin, cc);
			count = (*cc)->size;
			oe = FSWrite(tRefNum, &count, *(*cc)->data);
			if (oe != noErr) {
				// !! Error Message - write error
				FSClose(tRefNum);
				if (oe == dskFulErr)
					goto DiskFull;
				OSErrorAlert(ES_Caution, "FSWrite", oe);
				return;
			}
			cc = (*cc)->next;
		}

		// Close temp file
		FSClose(tRefNum);

		// If Original File Exists
		if (dWin->refNum) {
			// Close original file
			FSClose(dWin->refNum);

			bSpec = dWin->destSpec;

			// If it exists under current name
			if ((oe = HOpen(bSpec.vRefNum,bSpec.parID,bSpec.name,fsRdPerm,&dWin->refNum)) == noErr)
			{
				FSClose(dWin->refNum);

				if (gPrefs.backupFlag) {
					// Delete last backup file, if it exists
					bSpec = dWin->destSpec;
					bSpec.name[0]++;
					bSpec.name[bSpec.name[0]] = '~';
					HDelete(bSpec.vRefNum,bSpec.parID,bSpec.name);
		
					// Rename original file to backup name
					oe = HRename(dWin->destSpec.vRefNum,
								 dWin->destSpec.parID,
								 dWin->destSpec.name,
								  bSpec.name);
					if (oe != noErr) {
						// Backup is probably open, just delete original
						ErrorAlert(ES_Caution, "Can't backup original - backup file is open", oe);
						bSpec = dWin->destSpec;
						HDelete(bSpec.vRefNum,bSpec.parID,bSpec.name);
					}
				}
				else {
					// Delete Original if backup flag is false
					bSpec = dWin->destSpec;
					HDelete(bSpec.vRefNum,bSpec.parID,bSpec.name);
				}
			}

			// Rename temp file to correct name
			oe = HRename(tSpec.vRefNum,
						tSpec.parID,
						tSpec.name,
						dWin->destSpec.name);
			if (oe != noErr) {
				OSErrorAlert(ES_Stop, "HRename", oe);
			}
		}

		// Open newly saved file read only
		tSpec = dWin->destSpec;
		if (dWin->fork == FT_Resource) {
			oe = HOpenRF(tSpec.vRefNum,tSpec.parID,tSpec.name,fsRdPerm,&dWin->refNum);
			if (oe != noErr) {
				OSErrorAlert(ES_Stop, "HOpenRF", oe);
			}
		}
		else {
			oe = HOpen(tSpec.vRefNum,tSpec.parID,tSpec.name,fsRdPerm,&dWin->refNum);
			if (oe != noErr) {
				OSErrorAlert(ES_Stop, "HOpen", oe);
			}
		}

		// Reset Work File
		dWin->fsSpec = dWin->destSpec;
		SetWTitle((WindowPtr) dWin, dWin->fsSpec.name);

		dWin->workBytesWritten = 0L;
		SetEOF(dWin->workRefNum, 0L);

		// Flush the volume
		pb.volumeParam.ioCompletion = NULL;
		pb.volumeParam.ioNamePtr = NULL;
		pb.volumeParam.ioVRefNum = tSpec.vRefNum;
		PBFlushVol((ParmBlkPtr) &pb, FALSE);

		// Clear linked list
		UnloadFile(dWin);

		// Rebuilt linked list
		LoadFile(dWin);

		// Clear Dirty Flag
		dWin->dirtyFlag = false;
	}
	return;
DiskFull:
	ErrorAlert(ES_Caution, "Can't save - the disk is full.  Try freeing some space or using another disk.");
	HDelete(tSpec.vRefNum,tSpec.parID,tSpec.name);
}

void SaveAsContents(WindowPtr theWin)
{
	StandardFileReply	reply;
	EditWindowPtr		dWin = (EditWindowPtr) theWin;
	Str63				fileName;
	GetWTitle(theWin, fileName);
	StandardPutFile("\pFile to save:", 
					fileName, 
					&reply);
	if (reply.sfGood) {
		dWin->destSpec = reply.sfFile;
		dWin->creationDate = 0;
		SaveContents(theWin);
	}
}

void RevertContents(WindowPtr theWin)
{
	EditWindowPtr	dWin = (EditWindowPtr) theWin;
	long			newEOF;

	// Reset Work File
	dWin->workBytesWritten = 0L;
	SetEOF(dWin->workRefNum, 0L);

	// Clear linked list
	UnloadFile(dWin);

	// Reset EOF
	GetEOF(dWin->refNum, &newEOF);

	dWin->fileSize = newEOF;

	// Rebuilt linked list
	LoadFile(dWin);

	// Reset scroll offset, if necessary
	if (dWin->editOffset > dWin->fileSize -16*dWin->linesPerPage)
		dWin->editOffset = 0;
	DrawPage(dWin);
	UpdateOnscreen(theWin);
}

