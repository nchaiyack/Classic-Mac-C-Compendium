/* 같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같�

	CTermPane.cp
	
	Commtoolbox terminal emulation class.
	
	SUPERCLASS = CPanorama.
	
	Original copyright � 1992-93 Romain Vignes. All rights reserved.
	Modifications copyright � 1994-95 Ithran Einhorn. All rights reserved.
	
같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같� */
 
#include "CTermPaneDef.h"

TCL_DEFINE_CLASS_D1(CTermPane, CAbstractText);

/* Class Variables Initialization */

CPtrArray<CTermPane>	*CTermPane::cTermList = NULL;

/*
 * cTermIdle
 *
 * Idle time for each terminal object
 *
 *
 */
 
 		/* Idle routine for each terminal object */

		static void	Term_Idle(CTermPane *theTerm)
		{
			theTerm->DoIdle();
		}


void CTermPane::cTermIdle(void)
{
	if (cTermList != NULL)		/* List exists ? */
		cTermList->DoForEach(Term_Idle);
}	

/* 같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같 */

/*
 * cInitManager
 *
 * Terminal Manager Initialization
 *
 */
 
void CTermPane::cInitManager(void)
{
	InitTM();
}	

/* 같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같 */

/*
 * cGetTMVersion
 *
 * return the version of the Terminal Manager
 *
 */
 
short CTermPane::cGetTMVersion(void)
{
	return TMGetTMVersion();
}	

/* 같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같 */

/*
 * cCheckToolName
 *
 * Existence tool checking
 *
 * toolName:	name of the tool
 *
 * Return tmGenericError if the tool is not found
 *
 */
 
OSErr CTermPane::cCheckToolName(Str63 toolName)
{
	return(TMGetProcID(toolName));
}	

/* 같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같 */

/*
 * cTestToolMenu
 *
 * Test if the selected menu belongs to a terminal tool
 *
 * theMenu:	selected menu ID
 * theItem:	selected item ID
 *
 * Return TRUE if the menu is a terminal tool menu
 */
 

 		/* Test routine */

		static Boolean TermMenuTest(CTermPane *theTerm)
		{
			return theTerm->active;
		}


Boolean CTermPane::cTestToolMenu(short theMenu, short theItem)
{
	CTermPane	*current;
	
	if (cTermList == NULL)
		return FALSE;
	else
	{
		current = (CTermPane *)cTermList->FindItem(TermMenuTest);
		
		if (current == NULL)
			return FALSE;
		else
			return current->DoMenu(theMenu,theItem);
	}
}	

/* 같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같 */

/*
 * cTestToolEvent
 *
 * Test if the event is related to a terminal tool
 *
 * macEvent:	pointer on the event record
 * theWindow:	pointer on the window record
 *
 * Return TRUE if the event is a terminal event
 */

typedef struct	{
	EventRecord	*theEvent;
	WindowPtr	theWindow;
	Boolean		isToolEvent;
} TestParamRec;


 		/* Test routine */

		static void TermEvtTest(CTermPane *theTerm,long params)
		{
			TestParamRec *testRecPtr = (TestParamRec *)params;
			
			if (testRecPtr->isToolEvent == FALSE)
				testRecPtr->isToolEvent = theTerm->DoEvent(testRecPtr->theEvent,
														   testRecPtr->theWindow);
		}


Boolean CTermPane::cTestToolEvent(EventRecord *macEvent,WindowPtr theWindow)
{
	TestParamRec params;
	
	params.theEvent = macEvent;
	params.theWindow = theWindow;
	params.isToolEvent = FALSE;
	
	if (cTermList == NULL)
		return FALSE;
	else
	{
		cTermList->DoForEach1(TermEvtTest,(long)&params);
		
		return params.isToolEvent;
	}
}	

/* 같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같 */

/*
 * CTermPane
 * 
 * Default constructor.
 */
 
CTermPane::CTermPane()
{
}

/* 같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같 */

/*
 * CTermPane
 * 
 * Terminal pane object Initialization
 *
 * anEnclosure:		its enclosure view
 * aSupervisor:		its supervisor in the chain of command
 * toolName:		Terminal tool name ("" -> default tool)
 * flags(TM):		How to use the tool
 * sendProc(TM):	Pointer on the chars sending procedure
 * cacheProc(TM):	Pointer on the cach managing procedure
 * breakProc(TM):	Pointer on the BREAK signal sending procedure
 * clickLoop(TM):	Pointer on the clics handling sending procedure
 * environsProc(TM):Pointer on the environnement description procedure
 * refCon(TM):		undefined (application can use it)
 * userData(TM):	undefined (application can use it)
 *
 * Parameters followed by TM are exactly matching the ones used for the 
 * terminal record creation.
 *
 */

CTermPane::CTermPane(CView *anEnclosure, CBureaucrat *aSupervisor,
					 Str63 toolName,Str63 commName,Str63 xferName,TMFlags flags,TMSendProcPtr sendProc,
					 TMCacheProcPtr cacheProc,TMBreakProcPtr breakProc,TMClickProcPtr clickLoop,
					 TMEnvProcPtr environsProc,long refCon,long userData,Boolean isAsync,long TimeOut)
							
			: CAbstractText(anEnclosure,aSupervisor,0,0,0,0,
							sizELASTIC,sizELASTIC,-1)		/* Initialize its superclass */
{
	LongRect		lRect,newBounds;
	Rect			termRect,viewRect;
	LongRect		paneRect;
	OSErr			theErr;
	short			toolProcID;
	WindowPtr		owner;
	Boolean			savedAlloc;
	TermHandle		hTerm;
	Str63			tName;
	TermEnvironRec	theEnvirons;
	ConnHandle		theConn;
	CMBufferSizes	bufSizes;
	
	static unsigned char *szToolName = (unsigned char *)"";	// default tool name.
	
	asyncRead = isAsync;
	TimeOutLen = TimeOut;
	itsSelStart = itsSelEnd = itsSelAnchor = 0;
	reallyActive = FALSE;
	topMargin = 0;
	readCompleted = FALSE;
	
	owner = (WindowPtr) GetMacPort();		/* Window port */
	
	memset(bufSizes,0,sizeof(bufSizes));
	
	bufSizes[cmDataIn] = IOBUFSIZE;
	bufSizes[cmDataOut] = IOBUFSIZE;
	readAmount = 0L;
	numCachedLines = 0L;
	
	itsConnection = new CConnection(this,commName,0,bufSizes,0L,0L);
	
	itsFileTransfer = new CFileTransfer(this,xferName,0,nil,nil,nil,nil,nil,owner,0L,0L);
	
	theConn = itsConnection->GetConnHandle();
	ReadBuffer = NewPtr((*theConn)->bufSizes[cmDataIn]);
	FailMemError ();
	
	SetRect(&termRect,0,0,0,0);					/* visible terminal rect */
	SetRect(&viewRect,0,0,0,0);
	
	if (toolName[0] == 0)		/* Default tool ? */
	{
		theErr = CRMGetIndToolName(classTM,1,tName);
		
		if ((theErr != noErr) || (tName[0] == 0))	 	/* No tool */
			Failure(tmNoTools,SpecifyMsg(TERM_STR_RES_ID,NO_TOOL_STR_INDEX));

		toolProcID = cCheckToolName(tName);			/* Default tool ID */
	}
	else
		toolProcID = cCheckToolName(toolName);			/* Chosen tool ID */
	
	if (toolProcID == tmGenericError)			/* Bad tool */
			Failure(tmNoTools,SpecifyMsg(TERM_STR_RES_ID,BAD_TOOL_STR_INDEX));
		
	savedAlloc = SetAllocation(kAllocCanFail);
	
	if (sendProc == nil)
		sendProc = DefSendProc;
		
	if (breakProc == nil)
		breakProc = DefBreakProc;
		
	if (environsProc == nil)
		environsProc = (TMEnvProcPtr)DefEnvironsProc;
		
	if (cacheProc == nil)
		cacheProc = DefCacheProc;
		
	if (clickLoop == nil)
		clickLoop = DefClikLoop;
		
	hTerm = TMNew(&termRect,&viewRect,flags,toolProcID,owner,
					sendProc,(TerminalCacheProcPtr)cacheProc,breakProc,clickLoop,environsProc,
					(long)this,userData);
					
	SetAllocation(savedAlloc);
	
	FailNIL(hTerm);				/* Terminal record created ? */

	MoveHHi((Handle)hTerm);		/* Heap fragmentation� */
	
	itsTerm = hTerm;
			
	SetWantsClicks(TRUE);		
	
	SetCanBeGopher(TRUE);		
	SetWholeLines(TRUE);		
	
	// create the scroll-back buffer
	itsScrollBuffer = new CScrollBuffer(kNumCachedLines);
	
	if (cTermList == NULL)		/* First Terminal Object ? */
	{
		cTermList = new CPtrArray<CTermPane>;
	}
	
	cTermList->Add(this);			/* Terminal addition */
	
	changedConfig(false);		/* Configuration update */
	
	#ifdef __USETHREADS__
	if (CThread::cIsPresent())
	{
		itsIdleThread = new CThread(kCooperativeThread, (ThreadEntryProcPtr)cOneTermIdle, this);
	}
	#endif
	
	TCL_END_CONSTRUCTOR
}

/* 같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같 */

/*
 * Destructor
 *
 * Terminal pane object disposal
 *
 */
 
CTermPane::~CTermPane()
{
	TCL_START_DESTRUCTOR
	
	ASSERT(cTermList != NULL);
	
	#ifdef __USETHREADS__
	if (CThread::cIsPresent())
	{
		delete itsIdleThread;
	}
	#endif
	
	cTermList->Remove(this);		/* Dispose of the terminal */
	
	if (cTermList->IsEmpty())
		ForgetObject(cTermList);	/* Dispose of the cluster */

	TMDispose(itsTerm);				/* Terminal record removal */
	itsTerm = NULL;
	
	ForgetObject(itsFileTransfer);	/* Dispose the file transfer mechanism. */
	ForgetObject(itsConnection);	/* Dispose the connection. */
	ForgetObject(itsScrollBuffer);	/* Dispose scrollback buffer */
	
	ForgetPtr(ReadBuffer);			/* Release read buffer. */
}

/* 같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같 */

/*
 * Draw
 *
 * Terminal pane drawing
 *
 * area:	Pointer on the region to be updated
 *
 */
 
void CTermPane::Draw(Rect *area)
{
	RgnHandle		updateRgn;
	LongRect		longArea;
	long			startLine;
	long			endLine;
	long			numVisible;
	TermEnvironRec	theEnvirons;
		
	Prepare();
			
	updateRgn = NewRgn();
	FailNIL(updateRgn);
	
	RectRgn(updateRgn,area);		/* Transform rect into region */
		
	TMUpdate(itsTerm,updateRgn);
		
	DisposeRgn(updateRgn);
	
	/* Compute and draw the visible cached text lines */
	
	GetEnvirons(&theEnvirons);
	
	if (position.v < numCachedLines)
	{	
		Rect	areaRect;
		short	integralHeight;
	
		QDToFrameR(area, &longArea);
		LongToQDRect(&longArea,&areaRect);
		
		OffsetRect(&areaRect,0,-(theEnvirons.slop.v));
		
		// only draw an integral number of lines
		integralHeight = (RectHeight(&areaRect) / theEnvirons.cellSize.v) * theEnvirons.cellSize.v;
		areaRect.bottom = Min(areaRect.bottom, areaRect.top + integralHeight);
	
		startLine = areaRect.top / theEnvirons.cellSize.v;
		endLine = areaRect.bottom / theEnvirons.cellSize.v;
		
		numVisible = numCachedLines - position.v;
	
		if (startLine < numVisible)
		{	
			endLine = Min(endLine,(numVisible - 1));
			DrawLineRange(startLine, endLine, 0, kDontEraseText);
		}
	}
	
	// draw hilite area.
	if (itsSelStart != itsSelEnd)
		HiliteTextRange(itsSelStart, itsSelEnd);
}

/* 같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같 */

/*
 * DrawAllOffscreen
 *
 * Terminal pane drawing
 *
 * area:	Pointer on the region to be updated
 *
 */

void CTermPane::DrawAllOffscreen(Rect *area)
{
 LongRect	lsrcRect, ldestRect;
 
 CBitMap *paneBitMap;
 
 Prepare();
 
 GetWindow()->GetAperture(&lsrcRect);
 
 paneBitMap = new CBitMap(lsrcRect.right - lsrcRect.left, lsrcRect.bottom - lsrcRect.top, TRUE);
 
 paneBitMap->BeginDrawing();
 macPort = paneBitMap->macPort;
 
 DrawAll(area);
 
 macPort = paneBitMap->savePort; 
 paneBitMap->EndDrawing();
 
 QDToLongRect(area, &ldestRect);
 paneBitMap->CopyFrom(&lsrcRect, &lsrcRect, nil);
 
 delete paneBitMap;
}

/* 같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같 */

/*
 * RefreshOffscreen
 *
 * Terminal pane drawing
 *
 *
 */

void CTermPane::RefreshOffscreen(void)
{
	LongRect	termAperture;
	Rect		termRect;
	
	GetAperture(&termAperture);
	FrameToQDR(&termAperture,&termRect);
	
	DrawAllOffscreen(&termRect);
}

/* 같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같 */

/*
 * DoCommand
 *
 * Terminal pane related commands handling
 *
 * theCommand:	Command to be executed
 *
 */
 
void CTermPane::DoCommand(long theCommand)
{	
	Handle		selectionHdl;
	ResType		selectionType;
	short		scrapLength,i;
	EventRecord	theEvent;
	
	switch (theCommand)
	{		
		case cmdTermChoose:		/* Terminal tool setup */
			TermChoose();
			break;
	
		case cmdTermClear:		/* Screen clearing */
			TermClear();
			break;
	
		case cmdTermReset:		/* Terminal reset */
			Reset();
			break;
	
		case cmdCopy:			/* Copy */
			DoCopy();			
			break;
	
		case cmdTabCopy:		/* Table copy */
			DoTabCopy();
			break;
			
		case cmdPaste:			/* Paste */
		
			theEvent.what = keyDown;
			
			gClipboard->GetData('TEXT',&selectionHdl);
		
			scrapLength = gClipboard->DataSize('TEXT');
			
			for (i = 0;i < scrapLength;i ++)	{	/* For each char� */
				theEvent.message = (*selectionHdl)[i];
				theEvent.modifiers = 0;
				TMKey(itsTerm,&theEvent);
			}
			DisposHandle(selectionHdl);
			
			break;
			
		case cmdSelectAll:		/* Complete pane selection */
			DoSelectAll();
			break;
	
		default:
			inherited::DoCommand(theCommand);
			break;
	}
}

/* 같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같 */

/*
 * UpdateMenus
 *
 * Terminal tool menus updating
 *
 */
 
void CTermPane::UpdateMenus(void)
{
	Rect		nullRect;
	SignedByte	savedState;
		
	savedState = HGetState((Handle)itsTerm);
	HLock((Handle)itsTerm);
	
	inherited::UpdateMenus();			/* Pass message to its superclass */
	
	//gBartender->EnableCmd( cmdSelectAll);	/* Complete selection */
	
	gBartender->EnableCmd(cmdTermChoose);	/* Terminal tool setup */
	gBartender->EnableCmd(cmdTermClear);	/* Screen clearing */
	gBartender->EnableCmd(cmdTermReset);	/* Terminal reset */
	
	SetRect(&nullRect,0,0,0,0);
	
	if (!EqualRect(&nullRect,&(((*itsTerm)->selection).selRect)))
	{
		gBartender->EnableCmd(cmdCopy);	/* Copier */
		
		if (((*itsTerm)->selType == selTextNormal) ||
			((*itsTerm)->selType == selTextBoxed))
		{
			gBartender->EnableCmd(cmdTabCopy);	/* Table copy */
		}
	}
		
	if (gClipboard->DataSize('TEXT') > 0)		/* Paste */
		gBartender->EnableCmd(cmdPaste);
		
	HSetState((Handle)itsTerm,savedState);
}

/* 같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같 */

/*
 * TermChoose
 *
 * Terminal tool setup
 *
 */
 
void CTermPane::TermChoose(void)
{
	short			retCode;
	Point			where;
	TermHandle		hTerm;
	TermRecord		oldRecord;
	TermEnvironRec	theOldEnvirons;
	
	// save the old environment
	GetEnvirons(&theOldEnvirons);
	oldRecord = **itsTerm;
	
	hTerm = itsTerm;
	HUnlock((Handle)hTerm);
	
	SetPt(&where,H_CHOOSE_POS,V_CHOOSE_POS);	/* Dialog position */
												
	retCode = TMChoose(&hTerm,where,NULL);
	
	HLock((Handle)hTerm);
	itsTerm = hTerm;

	switch (retCode)
	{		
		case chooseCancel:			/* Canceled setup */
			break;
			
		case chooseOKMajor:			/* Changed tool */
			TermClear();			/* reset scrollback buffer, go on to chooseOKMinor processing */
			
			changedConfig();			
			TMActivate(itsTerm,TRUE);			
			itsSupervisor->Notify(NULL); 	/* Notify document� */
			
			break;
		
		
		case chooseOKMinor:					/* Same tool, changed config */
		
			AdjustTermPosition(&oldRecord, &theOldEnvirons);
			
			changedConfig();		
			TMActivate(itsTerm,TRUE);			
			itsSupervisor->Notify(NULL); 	/* Notify document� */
			
			break;
			
		default:				/* Unknown code (error) */
			FailOSErr(retCode);
			break;	
	}
}

/* 같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같 */

/*
 * DoIdle
 * 
 * Application idle time
 *
 */
 
void CTermPane::DoIdle(void)
{	
	Boolean fXferInProgress = (itsFileTransfer && itsFileTransfer->IsRunningOnSameCircuit ());
	
	Prepare();
	
	TMIdle(itsTerm);
	
	if (itsConnection && itsConnection->IsOpen())
	{
		if (readAmount && readCompleted == true && ! fXferInProgress)
		{
			DoStream(ReadBuffer,readAmount,readFlags);
			readAmount = 0L;
			readCompleted = false;
		}
	 
		if (! (itsFileTransfer && itsFileTransfer->NeedsConnection()))
		{
			readAmount = itsConnection->DataAvail();
	 
			if (readAmount)
			{
				itsConnection->DataRead(ReadBuffer,&readAmount,
										asyncRead,
										(CommProcPtr)DefReadCompletedProc,
										TimeOutLen,&readFlags);
				
				// for synchronous reads, set status to done, else completor routine will.							  
				if (asyncRead == false)
					readCompleted = true;
			}
		}
	}
}

/* 같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같 */

/*
 * cOneTermIdle
 * 
 * terminal idle time
 *
 */
 
pascal void	*CTermPane::cOneTermIdle (void *threadParam)
{
	CTermPane *aTerm = (CTermPane *)threadParam;
	
	#ifdef __USETHREADS__
	for (;;)
	{
		ThreadBeginCritical();
	#endif

		aTerm->DoIdle();	

	#ifdef __USETHREADS__
		ThreadEndCritical();
		YieldToAnyThread();
	}
	#endif

	return nil;
}
	
/* 같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같 */

/*
 * AdjustCursor
 *
 * Cursor shape update
 *
 * where:		cursor position
 * mouseRgn:	mouse region
 *
 */
 
void	CTermPane::AdjustCursor(Point where,RgnHandle mouseRgn)	
{
	/* The cursor shape is updated by the tool during the idle time */

}

/* 같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같 */

/*
 * Activate
 *
 * Terminal pane activating
 *
 */
 
void CTermPane::Activate(void)
{
	Boolean		wasActive = reallyActive;
	long		selStart = itsSelStart;
	long		selEnd = itsSelEnd;
	
	/* Remove the outlined selection range. */
	
	if (!wasActive)
	{
		Prepare();
		if (selStart != selEnd)
			HiliteTextRange(selStart, selEnd);
	}
	
	inherited::Activate();		/* Pass message to its superclass	*/
	
	Prepare();

	TMActivate(itsTerm,TRUE);		/* Tool activation */
	
	/* Reset our instance variables */
	
	reallyActive = TRUE;
	
	/* Hilite the now active selection range, if appropriate */
	
	if (selStart != selEnd && !wasActive)
	{
		Prepare();
		HiliteTextRange(selStart, selEnd);
	}
}	

/* 같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같 */

/*
 * Deactivate
 *
 * Terminal pane deactivation
 *
 */
 
void CTermPane::Deactivate(void)
{
	Boolean		wasActive = reallyActive;
	long		selStart = itsSelStart;
	long		selEnd = itsSelEnd;
	
	/* Unhilite the selection range. */
	
	if (wasActive)
	{
		Prepare();
		if (selStart != selEnd)
			HiliteTextRange(selStart, selEnd);
	}
	
	/* Reset our instance variables */
	
	reallyActive = FALSE;
	
	Prepare();

	TMActivate(itsTerm,FALSE);		/* tool deactivation */
	
	inherited::Deactivate();		/* Pass message to its superclass */
	
	/* Outline the now inactive selection range, if appropriate */
	
	if (wasActive)
	{
		Prepare();
		if (selStart != selEnd)
			HiliteTextRange(selStart, selEnd);
	}
}	

/* 같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같 */

/*
 * Mouse clic in the terminal pane
 *
 * hitPt:			Click position
 * modifierKeys:	Keyboard stat
 * when:			click instant
 *
 */
 
void CTermPane::DoClick(Point hitPt,short modifierKeys,long when)
{
	EventRecord		theEvent;
	LongPt			framePt;
	TermEnvironRec	theEnvirons;
	long			charOffset;
	long			selStart, selEnd;
	
	/* Get the current selection range */
	
	selStart = itsSelStart;
	selEnd = itsSelEnd;
	
	/* Determine which character was clicked on */
	GetEnvirons(&theEnvirons);
	
	QDToFrame(hitPt, &framePt);
	framePt.v += position.v * theEnvirons.cellSize.v;
	framePt.h += position.h * theEnvirons.cellSize.h;
	
	charOffset = GetCharOffset(&framePt);
	origCharOffset = lastCharOffset = charOffset;
	
	if (gClicks == 1)
	{
		// If the Shift key is down, extend the selection.
		
		if (modifierKeys & shiftKey)
			origCharOffset = lastCharOffset = itsSelAnchor;
		else
			SetSelection(charOffset, charOffset, kRedraw);	
	}
	else if (gClicks == 2)
	{
		// select current word.
		
		selStart = CalcWordBreak(charOffset, kBreakLeft);
		selEnd = CalcWordBreak(charOffset, kBreakRight);
		
		SetSelection(selStart, selEnd, kRedraw);
	}
	else	// select the entire line.
	{
		long			line;
		short			vertInset, horzInset;
		TermEnvironRec	theEnvirons;
			
		GetEnvirons(&theEnvirons);

		vertInset = theEnvirons.slop.v;
		horzInset = theEnvirons.slop.h;
		
		line = (framePt.v - vertInset) / theEnvirons.cellSize.v;
		
		if (line >= numCachedLines)
		{
			long	auxOffset = theEnvirons.auxSpace.top / theEnvirons.cellSize.v;
			
			vertInset = theEnvirons.slop.v + theEnvirons.auxSpace.top;
			horzInset = theEnvirons.slop.h + theEnvirons.auxSpace.left;
		
			line = Max(((framePt.v - vertInset) / theEnvirons.cellSize.v), numCachedLines);
		}
		
		selStart = GetLineStart(line);
		selEnd = GetLineEnd(line);
		
		if (itsSelStart != itsSelEnd && selStart < itsSelStart && selEnd > itsSelEnd)
		{
			HiliteTextRange(selStart, itsSelStart);
			HiliteTextRange(itsSelEnd, selEnd);
			SetSelection(selStart, selEnd, kNoRedraw);
		}
		else if (selStart != itsSelStart || selEnd != itsSelEnd)
			SetSelection(selStart, selEnd, kRedraw);
			
		origLine = lastLine = line;
	}

	theEvent.what = mouseDown;		
	theEvent.where = hitPt;
	LocalToGlobal(&theEvent.where);		/* Conversion */
	theEvent.modifiers = modifierKeys;
	theEvent.when = when;
	
	TMClick(itsTerm,&theEvent);		/* Tool handles clicks� */
	
	gLastMouseUp.what = mouseUp;	/* MouseUp� simulation */
	gLastMouseUp.when = TickCount();
	gLastMouseUp.where = hitPt;
	LocalToGlobal( &gLastMouseUp.where);	/* Conversion */
	gLastMouseUp.modifiers = modifierKeys;
	
	/* Determine the anchor point for the selection */
	
	if (gClicks == 1)
		itsSelAnchor = origCharOffset;
	else if (gClicks == 2)
		itsSelAnchor = CalcWordBreak(origCharOffset, charOffset < origCharOffset ? kBreakRight : kBreakLeft);
	else
		itsSelAnchor = (charOffset < origCharOffset ? GetLineEnd(origLine) : GetLineStart(origLine));
		
	/* Notify our dependents that the selection has changed */
	
	SelectionChanged();
}

/* 같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같 */

/*
 * ResizeFrame
 *
 * Handle changing of the terminal pane size
 *
 * delta:	updates rect
 *
 */
 
void CTermPane::ResizeFrame(Rect *delta)
{
	Rect		newTermRect;
	
	inherited::ResizeFrame(delta);			/* Pass message to its superclass */
	
	//FrameToWindR(&frame,&newTermRect);
	LongToQDRect(&frame,&newTermRect);		/* Visible region */
	
	TMResize(itsTerm,&newTermRect); 		/* tool handles resizing� */
	
											/* Border gets cleaned up in Draw method */
}

/* 같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같 */

/*
 * DoKeyDown
 *
 * Keyboard handling
 *
 * theChar:		Typed char
 * keyCode:		Code of the corresponding key
 * macEvent:	Pointer on the related event
 *
 */
 
void CTermPane::DoKeyDown(char theChar,Byte keyCode,EventRecord	*macEvent)
{
	TMKey(itsTerm,macEvent);		/* Tool handles keys� */
}	

/* 같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같 */

/*
 * DoAutoKey
 *
 * Key repetition
 *
 * theChar:		typed char
 * keyCode:		code of the related code
 * macEvent:	Pointer on the corresponding event
 *
 */
 
void CTermPane::DoAutoKey(char theChar,Byte keyCode,EventRecord	*macEvent)
{
	DoKeyDown(theChar,keyCode,macEvent);	
}	

/* 같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같 */

/*
 * SetConfig
 *
 * Terminal tool configuration
 *
 * theConfig:	configuration C string
 *
 * Return:		Negative value: error (-1 -> unknown)
 *				Positive value: Parser stop index
 *				tmNoErr if all is OK
 *
 */
 
short CTermPane::SetConfig(char *theConfig)
{
	short retCode;
	
	retCode = TMSetConfig(itsTerm,theConfig);
	
	changedConfig();	/* Config update */
	
	return retCode;
}	

/* 같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같 */

/*
 * GetToolName
 *
 * Return the name of the current tool
 *
 * toolName:	name of the current tool (pascal string)
 *
 */
 
void CTermPane::GetToolName(Str63 toolName)
{
	SignedByte	savedState;
	
	savedState = HGetState((Handle)itsTerm);
	HLock((Handle)itsTerm);
	
	TMGetToolName((*itsTerm)->procID,toolName);
	
	HSetState((Handle)itsTerm, savedState);
}	

/* 같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같 */

/*
 * GetConfig
 *
 * Return the configuration string of the current tool
 *
 * Reeturn a pointer on the C string
 *
 */
 
Ptr CTermPane::GetConfig(void)
{
	return(TMGetConfig(itsTerm));
}	


/* 같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같 */

/*
 * DoMenu
 *
 * Terminal tool menus handling
 *
 * theMenu:	selected menu
 * theItem:	selected item
 *
 * Return TRUE if the menu belongs to the tool
 *
 */
 
Boolean CTermPane::DoMenu(short theMenu,short theItem)
{
	return(TMMenu(itsTerm,theMenu,theItem));
}	

/* 같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같 */

/*
 * DoEvent
 *
 * Terminal tool related events
 *
 * theEvent:	Pointeur on the event
 * theWindow:	Window associated with the event
 *
 * Return TRUE if it is a tool event
 *
 */
 
Boolean CTermPane::DoEvent(EventRecord *theEvent,WindowPtr theWindow)
{
	Boolean		isToolEvent;
	TermHandle	theTerm;
	
	isToolEvent = FALSE;
	
	theTerm = (TermHandle) GetWRefCon(theWindow);
	
	if (theTerm == itsTerm)			/* Tool window ? */
	{
		TMEvent(itsTerm,theEvent);
		isToolEvent = TRUE;
	}
	
	return isToolEvent;
}	

/* 같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같 */

/*
 * GetTermHandle
 *
 * Return a Handle on the current terminal record
 *
 * Return the Handle
 *
 */
 
TermHandle CTermPane::GetTermHandle(void)
{
	return(itsTerm);
}	

/* 같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같 */

/*
 * DoStream
 *
 * Received data handling
 *
 * buffer:		Receiving buffer
 * buffSize:	buffer size
 * flags:		end of message
 *
 * Return the nb of chars handled
 *
 */
 
long CTermPane::DoStream(Ptr buffer,long buffSize,CMFlags flags)
{
	long	numLinesScrolled, numBytes;
	
	Prepare();
	
	numBytes = TMStream(itsTerm,buffer,buffSize,flags);

	// scroll to new line
	numLinesScrolled = LinesScrolledUp() + LinesScrolledDown();
	
	if (numLinesScrolled)
	{
		LongPt	curPos;
		
		GetPosition(&curPos);
		curPos.v += numLinesScrolled;
		
		ScrollTo(&curPos, false);
		
		Prepare();
	}
	
	return numBytes;
}	

/* 같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같 */

/*
 * TermClear
 *
 * Clear out the terminal pane, reset to origin.
 *
 */
 
void CTermPane::TermClear(void)
{	
	Rect		termRect, viewRect;
	LongRect	theLongArea;
	
	Prepare();

	TMClear(itsTerm);
	
	// remove cached lines.
	numCachedLines = 0;
	position.v = 0;
	
	itsScrollBuffer->Truncate();

	// scroll TM back to origin.
	viewRect = (*itsTerm)->viewRect;
	termRect = (*itsTerm)->termRect;
		
	ScrollTerminal(termRect.left - viewRect.left,(termRect.top - viewRect.top) + topMargin);
		
	AdjustBounds();
	
	// erase the whole frame
	GetFrame(&theLongArea);
	FrameToQDR(&theLongArea,&termRect);
	
	termRect.bottom = termRect.top + height;
	EraseRect(&termRect);
}	

/* 같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같 */

/*
 * Reset
 *
 * Terminal reset
 *
 */
 
void CTermPane::Reset(void)
{
	short	response;
	
	PositionDialog('ALRT', RESET_ALRT_ID);
	
	InitCursor();
	
	response = Alert(RESET_ALRT_ID, NULL);
		
	if (response == answerNO)
	{
		Prepare();
	
		TMReset(itsTerm);
		
		changedConfig();	/* Configuration update */
	}

}	

/* 같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같 */

/*
 * DoTabCopy
 *
 * Table copy
 *
 */
 
void CTermPane::DoTabCopy(void)
{
	DoCopy(kTabCopy);
}	

/* 같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같 */

/*
 * GetExtent
 *
 * Return the size (in char rows and columns) of the terminal pane
 *
 * theHExtent:	Nb of columns
 * theVExtent:	Nb of rows
 *
 */
 
void CTermPane::GetExtent(long *theHExtent,long *theVExtent)
{
	GetTermExtent(theHExtent,theVExtent);
	
	*theVExtent = GetNumLines();
}

/* 같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같 */

/*
 * GetEnvirons
 *
 * Terminal parameters
 *
 * theEnvirons:		Pointer on the environment record
 *
 * Return an error code
 *
 */
 
OSErr CTermPane::GetEnvirons(TermEnvironRec *theEnvirons)
{
	theEnvirons->version = curTermEnvRecVers;/* Record version */

	return (TMGetTermEnvirons(itsTerm,theEnvirons));	
}	

/* 같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같 */

/*
 * GetLine
 *
 * Content of a terminal screen line
 *
 * lineNo:		line number
 * theTermData:	Pointer on the content of the line
 *
 * Renturn an error code
 *
 */
 
void CTermPane::GetLine(short lineNo,TermDataBlock *theTermData)
{
	theTermData->theData = NewHandleCanFail(0);
	
	TMGetLine(itsTerm,lineNo,theTermData);	
}	

/* 같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같 */

/*
 * Scroll
 *
 * Terminal pane scrolling
 *
 * hDelta:	Horizontal scrolling
 * vDelta:	vertical scrolling
 * redraw:	flag of redrawing
 *
 */
 
void CTermPane::Scroll(long hDelta,long vDelta,Boolean redraw)
{
	long			hPixels,vPixels;
	TermEnvironRec	theEnvirons;
	
	Prepare();
	
	GetEnvirons(&theEnvirons);
	
	hPixels = hDelta * hScale;
	vPixels = vDelta * vScale;
	
	/*	even if we scroll more than a screen worth's, execute TMScroll or the viewRect
		will not be recalculated  by TM */
		
	// if going down, scroll terminal before scrollback buffer
	if (vDelta <= 0)
		ScrollTerminal(-(hPixels),-(vPixels));
	
	if ((Abs(hPixels) < width) && (Abs(vPixels) < height))
	{
		Rect		termRect, viewRect, visRect;
		RgnHandle	termRgn, visRgn, cacheRgn;
			
		GetEnvirons(&theEnvirons);
		
		// calculate the visible cache and invalidate it.
		termRgn = NewRgn();
		visRgn = NewRgn();
		cacheRgn = NewRgn();
		
		// let cache line write through 'border' of terminal region
		viewRect = (*itsTerm)->viewRect;		
		termRect = (*itsTerm)->termRect;
		
		SectRect(&termRect, &viewRect, &visRect);
		
		RectRgn(termRgn,&termRect);
		RectRgn(visRgn,&visRect);
		DiffRgn(termRgn,visRgn,cacheRgn);
		
		RgnRect(cacheRgn,&visRect);
		OffsetRect(&visRect,0,topMargin - TERMINAL_OFFSET);
		
		int numVisCacheLines = RectHeight(&visRect) / theEnvirons.cellSize.v;
		
		if (numVisCacheLines - abs(vDelta) >= 1)
		{
			short	numVisTermLines, newHeight;
						
			// don't dovetail the bottom of cache region on terminal region
			if (RectHeight(&visRect) >= topMargin)
				visRect.bottom -= topMargin;
			
			// only scroll an integral number of lines.
			newHeight = (RectHeight(&visRect) / theEnvirons.cellSize.v) * theEnvirons.cellSize.v;
			visRect.bottom = Min(visRect.bottom,visRect.top + newHeight);
			
			// check for frame off by one
			GetTermRect(&termRect);

			numVisTermLines = RectHeight(&termRect) / theEnvirons.cellSize.v;
			
			ScrollRect(&visRect,-(hPixels),-(vPixels),cacheRgn);
		}
		
		InvalRgn(cacheRgn);
		
		DisposeRgn(termRgn);
		DisposeRgn(visRgn);
		DisposeRgn(cacheRgn);
	}
	else
		Refresh();
		
	// if going up, scroll terminal after scrollback buffer
	if (vDelta > 0)
		ScrollTerminal(-(hPixels),-(vPixels));
		
	position.h += hDelta;
	position.v += vDelta;
	
	AdjustBounds();
	
	if (redraw)
    	((CWindow *) (((WindowPeek) macPort)->refCon))->Update();
}

/* 같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같 */

/*
 * changedConfig
 *
 * Terminal tool configuration update
 *
 */
 
void CTermPane::changedConfig(Boolean doBroadcast)
{
	TermEnvironRec	theEnvirons;
	Rect			viewRect;
	LongRect		newBounds;
	SignedByte		savedState;
	
	//savedState = HGetState((Handle)itsTerm);
	//HLock((Handle)itsTerm);
	
	GetEnvirons(&theEnvirons);							/* Environment */
	
	hScale = theEnvirons.cellSize.h;					/* Chars size */
	vScale = theEnvirons.cellSize.v;
	
	topMargin = (theEnvirons.cellSize.v / 3) + TERMINAL_OFFSET;

	SetTermOffset();
	
	AdjustBounds();
	
	if (doBroadcast)
		BroadcastChange(termSizeChangedInd, NULL);		/* tell the supervisor */
	
	//HSetState((Handle)itsTerm,savedState);
}	

/* 같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같 */

/*
 * DoSelectAll
 *
 * Complete selection
 *
 */
 
void CTermPane::DoSelectAll(void)
{
	Prepare();
	
	long termTextLen = TerminalTextLength(-1,-1);
	
	if (termTextLen)
	{
		TermEnvironRec	theEnvirons;
		TMSelection		newSelection;
		TMSelTypes		newSelType;
		long			numRows,numCols;
	
		GetEnvirons(&theEnvirons);		/* Environment */
		
		numCols = termTextLen % theEnvirons.textCols;
		numRows = (termTextLen / theEnvirons.textCols) + ((numCols) ? 1 : 0);
	
		SetRect(&(newSelection.selRect),1,1,numCols,numRows);
	
		newSelType = selTextNormal;
	
		TMSetSelection(itsTerm,&newSelection,newSelType);
	}
	
	SetSelection(0, GetLength(), kRedraw);
}	

/* 같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같 */

/*
 * BindConnection
 *
 * Give terminal a connection object
 *
 */
 
void CTermPane::BindConnection(CConnection *newConnection)
{
 itsConnection = newConnection;
}

/* 같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같 */

/*
 * GetConnection
 *
 * return connection object
 *
 */
 
CConnection	*CTermPane::GetConnection(void)
{
 return itsConnection;
}

/* 같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같 */

/*
 * GetFileTransfer
 *
 * return file transfer object
 *
 */
 
CFileTransfer *CTermPane::GetFileTransfer(void)
{
 return itsFileTransfer;
}

/* 같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같 */

/*
 * default environment call-back proc
 *
 * 
 *
 */
 
pascal CMErr CTermPane::DefEnvironsProc (long refCon, ConnEnvironRecPtr theEnvPtr)
{
 CTermPane *theTermPane = (CTermPane *)refCon;
 CConnection *theConnection = theTermPane->GetConnection();
 
 return theConnection->GetEnvirons(theEnvPtr);
}

/* 같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같 */

/*
 * default send call-back proc
 *
 * 
 *
 */
 
pascal long CTermPane::DefSendProc (Ptr buffPtr, long howMany, long refCon, CMFlags flags)
{
 OSErr			myErr = 0;
 CTermPane		*theTermPane = (CTermPane *)refCon;
 CConnection	*theConnection = theTermPane->GetConnection();
 CFileTransfer	*itsFileTransfer = theTermPane->GetFileTransfer();
  
 if (theConnection->GetConnHandle())
 {
	// If there's a file transfer in progress && it's 	
	// over our connection, then don't send the data.	
	
	if (itsFileTransfer && itsFileTransfer->IsRunningOnSameCircuit ())
		howMany = 0;
	else
		myErr = theConnection->DataWrite(buffPtr,&howMany,false,nil,15,flags);
 }
 else howMany = 0;
 
 return howMany;
}

/* 같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같 */

/*
 * default Break call-back proc
 *
 * 
 *
 */
 
pascal void CTermPane::DefBreakProc (long duration, long refCon)
{
 CTermPane *theTermPane = (CTermPane *)refCon;
 CConnection *theConnection = theTermPane->GetConnection();
 
 theConnection->SendBreak();
}

/*******************************************************************
*	DefReadCompletedProc	- default async read completor proc
*
*		theConn				- which connection tool found it
*
**********************************************************************/

pascal void CTermPane::DefReadCompletedProc (ConnHandle myConn)
{
 CConnection *theConnection = (CConnection *)CMGetRefCon (myConn);
  						 
 // if we belong to a terminal pane, get its file transfer object
 if (member(theConnection->itsSupervisor, CTermPane))
 {
 	CTermPane *theTermPane = (CTermPane *)theConnection->itsSupervisor;
 	
	theTermPane->asyncErr = (**myConn).errCode;
	theTermPane->readAmount = ((**myConn).asyncCount[cmDataIn] < (IOBUFSIZE-1L)) ?
 								(**myConn).asyncCount[cmDataIn] : (IOBUFSIZE-1L);
 															
	theTermPane->readCompleted = true;
 }
}	

/*******************************************************************
*	DefCacheProc	- default line cache procedure
*
*		refCon				- user defined, points to the term object
*		theTermData			- contains data to be cached.
*
**********************************************************************/

pascal long CTermPane::DefCacheProc (long refCon, TermDataBlock *theTermData)
{
 CTermPane	*theTermPane = (CTermPane *)refCon;
 long		sizeCached = 0L;

 HLock(theTermData->theData);
 
 if (theTermData->flags == tmTextTerminal)
 {
 	char	*cacheLine = (char *)*(theTermData->theData);
 	 	
 	if (theTermPane->numCachedLines < theTermPane->itsScrollBuffer->GetMaxLines())
 	{
 		theTermPane->itsScrollBuffer->AddLine(cacheLine, GetHandleSize(theTermData->theData));
 		
		theTermPane->numCachedLines++;
		theTermPane->position.v++;
		
		theTermPane->AdjustBounds();
		theTermPane->Prepare();
 	}
 	else
 	{
 		theTermPane->itsScrollBuffer->DeleteLine(0);
 		theTermPane->itsScrollBuffer->AddLine(cacheLine, GetHandleSize(theTermData->theData));
 	}
 
 }
 
 HUnlock(theTermData->theData);
 
 return sizeCached;
}

/*******************************************************************
*	DefClikLoop	- default mouse click procedure
*
*		refCon	- user defined, points to the term object
*
**********************************************************************/

pascal Boolean	CTermPane::DefClikLoop (long refCon)
{
	Boolean		inCacheRegion = true;
	Point		hitPt;
	CTermPane	*theTermPane = (CTermPane *)refCon;
	
	GetMouse(&hitPt);
	
	if (StillDown())
	{
		inCacheRegion = theTermPane->ProcessCacheClick(hitPt);
	}
	else
	{
		LongPt		framePt;
		long		charOffset;
	
		theTermPane->QDToFrame(hitPt, &framePt);
		PinInRect(&(theTermPane->frame), &framePt);
	
		charOffset = theTermPane->GetCharOffset(&framePt);
		inCacheRegion = charOffset < theTermPane->itsScrollBuffer->GetTextLength();
	}
	
	return inCacheRegion;
}
	
/* 같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같 */

/*
 * GetCursor -	Get the cursor position. 
 *
 * 
 *
 */
 
Point CTermPane::GetCursor(TMCursorTypes curType)
{
 return TMGetCursor(itsTerm, curType);
}

// member function overrides

/* 같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같 */

/*
 * Prepare -	Prepare the terminal for drawing. Clips to actual terminal
 *				area
 * 
 *
 */
 
void	CTermPane::Prepare( void)
{
	inherited::Prepare();
}

/******************************************************************************
 SetWholeLines

		Specify whether or not the frame should be adjusted so that
		lines are not cut off vertically. The Pane is NOT redrawn.
		
 ******************************************************************************/

void	CTermPane::SetWholeLines( Boolean aWholeLines)
{
	Rect		newFrame;
  			
	inherited::SetWholeLines(aWholeLines);
	
	if (wholeLines && fixedLineHeights)
	{
		LongToQDRect(&frame,&newFrame);			// get new frame as true measure of line heights.
		newFrame.bottom = newFrame.top + height;
		
		CalcTermRect(&newFrame);
		
		// use the height of the first line as the standard height
			
		short lineHeight = Get1Height(1);	
			
		if (lineHeight > 0)
		{
			frame.bottom = frame.top + (lineHeight * ((RectHeight(&newFrame) - topMargin) / lineHeight)) + topMargin;
			frame.bottom--;
		}
			
		SetRect(&newFrame,0,0,0,0);
		CalcFullRect(&newFrame);
		
		frame.bottom += newFrame.bottom;
		//frame.right += newFrame.right;
		
		CalcAperture();
	}

}

void	CTermPane::SetTextPtr(Ptr textPtr, long numChars)
{
}

Handle	CTermPane::GetTextHandle(void)
{
	return itsScrollBuffer->GetTextHandle();
}

void	CTermPane::SetFontNumber(short aFontNumber)
{
}

/*void	CTermPane::SetFontName(Str255 aFontName)
{
}*/

void	CTermPane::SetFontStyle(short aStyle)
{
}

void	CTermPane::SetFontSize(short aSize)
{
}

void	CTermPane::SetTextMode(short aMode)
{
}

void	CTermPane::SetAlignCmd(long anAlignment)
{
}

long	CTermPane::GetAlignCmd( void)
{
	return 0L;
}

void	CTermPane::SetSpacingCmd(long aSpacingCmd)
{
}

/******************************************************************************
 GetHeight (OVERRIDE)

	Return the height of the indicated lines of text
 ******************************************************************************/
 
long	CTermPane::GetHeight( long startLine, long endLine)
{
	TermEnvironRec	theEnvirons;
	
	GetEnvirons(&theEnvirons);
	
	return (long)((endLine - startLine + 1) * theEnvirons.cellSize.v);
}

/******************************************************************************
 GetCharOffset (OVERRIDE)

		Return the offset into the text buffer of the character position
		at a point in Frame coordinates.
		
 ******************************************************************************/
 
long	CTermPane::GetCharOffset( LongPt *aPt)
{
	long	line, charOffset = 0L;
	long	numTermLines;
	short	vertInset, horzInset, lineLen;

	TermEnvironRec	theEnvirons;

	GetEnvirons(&theEnvirons);

	lineLen = theEnvirons.cellSize.h * theEnvirons.textCols;
	
	// line that has the desired character.
	vertInset = theEnvirons.slop.v;
	horzInset = theEnvirons.slop.h;
	
	line = (aPt->v - vertInset) / theEnvirons.cellSize.v;
	
	if (line >= numCachedLines)
	{
		long	auxOffset = theEnvirons.auxSpace.top / theEnvirons.cellSize.v;
		
		vertInset = theEnvirons.slop.v + theEnvirons.auxSpace.top;
		horzInset = theEnvirons.slop.h + theEnvirons.auxSpace.left;
		
		line = Max(((aPt->v - vertInset) / theEnvirons.cellSize.v), numCachedLines);
	}

	if (line >= 0)
	{
		// number of lines in the actual terminal
		numTermLines = GetCursor().v - 1;

		// if beyond end of buffer, return the last character
		if (line > (numTermLines + itsScrollBuffer->GetNumLines()) - 1)
		{
			// last character is the terminal buffer and scrollback buffer.
			charOffset = GetLength();
		}
		else
		{
			long lineOffset = 0L;
			
			// calculate number of characters in line containing point
			if (aPt->h > horzInset)
			{
				long hPos = aPt->h;
				
				// adjust if clicked in right margin.
				if (hPos > (lineLen + horzInset))
					hPos = lineLen + horzInset;
					
				lineOffset = (hPos - horzInset) / theEnvirons.cellSize.h;
				
				if (((hPos - horzInset) % theEnvirons.cellSize.h) >= theEnvirons.cellSize.h / 2)
					lineOffset++;
			}
			
			// calculate offset into 'virtual' buffer.
			charOffset = GetLineStart(line) + lineOffset;
		}
	}

	return charOffset;
}

/******************************************************************************
 GetCharPoint (OVERRIDE)

 		Return the Frame coordinates of the character at the given offset
 		in the text buffer.		
 ******************************************************************************/
 
void	CTermPane::GetCharPoint( long charOffset, LongPt *aPt)
{
	long	line;
	short	vertInset, horzInset;

	TermEnvironRec	theEnvirons;

	GetEnvirons(&theEnvirons);
	
	if (charOffset > itsScrollBuffer->GetTextLength())
	{
		vertInset = theEnvirons.slop.v + theEnvirons.auxSpace.top;
		horzInset = theEnvirons.slop.h + theEnvirons.auxSpace.left;
	}
	else
	{
		vertInset = theEnvirons.slop.v;
		horzInset = theEnvirons.slop.h;
	}
	
	/* Ensure that the offset is within bounds */
	
	charOffset = Max(charOffset, 0);
	charOffset = Min(charOffset, GetLength());

	line = FindLine(charOffset);
		
	aPt->v = vertInset + (line * theEnvirons.cellSize.v);
	aPt->h = horzInset + ((charOffset - GetLineStart(line)) * theEnvirons.cellSize.h);
}

void	CTermPane::GetTextStyle( short *whichAttributes, TextStyle *aStyle)
{
}

void	CTermPane::GetCharStyle( long charOffset, TextStyle *theStyle)
{
}

/******************************************************************************
 FindLine (OVERRIDE)

		Find line in 'virtual' buffer that contains the given character.
 ******************************************************************************/
 
long	CTermPane::FindLine(long charPos)
{
	long	line;
	
	if (charPos > itsScrollBuffer->GetTextLength())
	{
		// find in terminal region
		TermEnvironRec	theEnvirons;
		TermDataBlock	theDataBlock;
		short			termLine;
		long			totLen = 0L;
	
		GetEnvirons(&theEnvirons);
		
		for (termLine = 1; termLine <= theEnvirons.textRows; termLine++)
		{
			GetLine(termLine,&theDataBlock);
			
			HLock(theDataBlock.theData);
			totLen += strlen(*theDataBlock.theData);
			HUnlock(theDataBlock.theData);
			
			DisposHandle(theDataBlock.theData);
			
			if (charPos < totLen)
				break;
		}
	
		line = (termLine - 1) + itsScrollBuffer->GetNumLines();
	}
	else
	{
		line = itsScrollBuffer->FindLine(charPos);
	}
	
	return line;
}

/******************************************************************************
 GetLength {OVERRIDE}
 
 		Return the number of characters in the 'virtual' buffer.
 ******************************************************************************/
 
long	CTermPane::GetLength( void)
{
	return TerminalTextLength(-1,-1) + itsScrollBuffer->GetTextLength();
}

/******************************************************************************
 GetLineStart
 
 		Return the offset of the character at the start of the given line. 
 		The line number parameter is zero-based.
 ******************************************************************************/

long	CTermPane::GetLineStart(long line)
{
	long charOffset = 0L;
	
	// calculate offset into 'virtual' buffer.
	if (line < itsScrollBuffer->GetNumLines())
	{
		// it's wholly contained in the scrollback buffer
		charOffset = itsScrollBuffer->GetLineStart(line);
	}
	else
	{
		long	start, len;
		
		// it's in the terminal region, so offset it by total amount in scrollback buffer.
		line -= itsScrollBuffer->GetNumLines();
		
		GetLineInfo(line, &start, &len);
		
		charOffset = itsScrollBuffer->GetTextLength() + start;
	}
	return charOffset;
}

/******************************************************************************
 GetLineEnd
 
 		Return the offset of the character at the end of the given line. 
 		The line number parameter is zero-based.
 ******************************************************************************/

long	CTermPane::GetLineEnd(long line)
{
	long charOffset = 0L;
	
	// calculate offset into 'virtual' buffer.
	if (line < itsScrollBuffer->GetNumLines())
	{
		// it's wholly contained in the scrollback buffer
		charOffset = itsScrollBuffer->GetLineEnd(line);
	}
	else
	{
		long	start, len;
		
		// it's in the terminal region, so offset it by total amount in scrollback buffer.
		line -= itsScrollBuffer->GetNumLines();
		
		GetLineInfo(line, &start, &len);
		
		charOffset = itsScrollBuffer->GetTextLength() + start + len;
	}
	return charOffset;
}

/******************************************************************************
 GetLineLength
 
 		Return the length of the given line. 
 		The line number parameter is zero-based.
 ******************************************************************************/

long	CTermPane::GetLineLength(long line)
{
	long lineLength = 0L;
	
	// calculate offset into 'virtual' buffer.
	if (line < itsScrollBuffer->GetNumLines())
	{
		// it's wholly contained in the scrollback buffer
		lineLength = itsScrollBuffer->GetLineLength(line);
	}
	else
	{
		long	start;
		
		// it's in the terminal region, so offset it by total amount in scrollback buffer.
		line -= itsScrollBuffer->GetNumLines();
		
		GetLineInfo(line, &start, &lineLength);
	}
	return lineLength;
}

/* 같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같 */

/*
 * TypeChar
 *
 * Type simulation
 *
 * theChar:		char to be typed
 *
 */
 
void	CTermPane::TypeChar(char theChar, short theModifiers)
{
	EventRecord	theEvent;
	long		finalTicks;
	
	theEvent.what = keyDown;
	theEvent.message = theChar;
	theEvent.modifiers = 0;
	
	Delay(CHAR_DELAY_TICKS,&finalTicks);
	
	DoKeyDown(theChar,0,&theEvent);
}	

long	CTermPane::GetSpacingCmd( void)
{
	return 0L;
}

/* 같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같 */

/*
 * GetNumLines -	(OVERRIDE) Get total # of lines, including cached lines. 
 *
 * 
 *
 */
 
long CTermPane::GetNumLines(void)
{
	TermEnvironRec	theEnvirons;
	
	GetEnvirons(&theEnvirons);
	
	return theEnvirons.textRows + numCachedLines;
}

Handle	CTermPane::CopyTextRange( long start, long end)
{
	return NULL;
}

void	CTermPane::PerformEditCommand( long theCommand)
{
}

void	CTermPane::InsertTextPtr( Ptr text, long length, Boolean fRedraw)
{
}

/******************************************************************************
 Dawdle {OVERRIDE}

		The user isn't doing anything, so flash the text insertion cursor
 ******************************************************************************/

void	CTermPane::Dawdle(long	*maxSleep)
{
	if (visible)
	{
		DoIdle();
		*maxSleep = GetCaretTime();
	}
}

/******************************************************************************
 HideSelection	{OVERRIDE}

		Hide/unhide the current selection and the blinking cursor.
		Does not change the active or gopher state.
 ******************************************************************************/

void	CTermPane::HideSelection(Boolean hide, Boolean redraw)
{
}

/******************************************************************************
 GetLineInfo
 
 		Get info about the given line. 
 		The line number parameter is zero-based.
 ******************************************************************************/

void	CTermPane::GetLineInfo(long line, long *start, long *len)
{
	// find in terminal region
	TermEnvironRec	theEnvirons;
	TermDataBlock	theDataBlock;
	short			termLine;
	
	GetEnvirons(&theEnvirons);
		
	*start = *len = 0L;
	
	for (termLine = 1; termLine <= theEnvirons.textRows && (termLine - 1) <= line; termLine++)
	{
		*start += *len;
		
		GetLine(termLine,&theDataBlock);
		
		HLock(theDataBlock.theData);
		*len = strlen(*theDataBlock.theData);
		HUnlock(theDataBlock.theData);
		
		DisposHandle(theDataBlock.theData);
	}
}

/******************************************************************************
 SetTermOffset
 
 		Move Terminal to its offset. 
 ******************************************************************************/

void	CTermPane::SetTermOffset(void)
{
	int				vDelta;
	Rect			viewRect;
	TermEnvironRec	theEnvirons;
	
	GetEnvirons(&theEnvirons);
	
	viewRect = (*itsTerm)->viewRect;
	vDelta = viewRect.top % theEnvirons.cellSize.v;
	
	if (vDelta < 0)
		vDelta += theEnvirons.cellSize.v;
	
	ScrollTerminal(0, topMargin - vDelta);
}

/******************************************************************************
 ScrollTerminal
 
 		Move Terminal region within the frame. 
 ******************************************************************************/

void	CTermPane::ScrollTerminal(int hDelta, int vDelta)
{
	TMScroll(itsTerm,hDelta,vDelta);
}

/******************************************************************************
 GetCharAt

	Return the character at aPosition. The character and its size
	are returned in charBuf, and aPosition is updated with the starting
	position of the character. If there is no character, then
	Length(charBuf) is 0, and aPosition is not updated.
	
 ******************************************************************************/
 
void	CTermPane::GetCharAt(long *aPosition, tCharBuf charBuf)
{
	long	scrollBufLen = itsScrollBuffer->GetTextLength();
	
	if (*aPosition >= scrollBufLen && *aPosition < scrollBufLen + TerminalTextLength(-1,-1))
	{
		// find in terminal region
		TermEnvironRec	theEnvirons;
		TermDataBlock	theDataBlock;
		short			termLine;
		long			totLen = 0L;
	
		GetEnvirons(&theEnvirons);
		
		for (termLine = 1; termLine <= theEnvirons.textRows; termLine++)
		{
			long len;
			
			GetLine(termLine,&theDataBlock);
			
			char *dataStr = (char *)*theDataBlock.theData;
			
			HLock(theDataBlock.theData);
			len = strlen(dataStr);
			HUnlock(theDataBlock.theData);
			
			if (*aPosition < totLen + len)
			{
				Length(charBuf) = 1;
				charBuf[1] = dataStr[*aPosition - totLen];
				DisposHandle(theDataBlock.theData);
				break;
			}
			
			totLen += len;
			
			DisposHandle(theDataBlock.theData);
		}
	}
	else
	{
		itsScrollBuffer->GetCharAt(aPosition, charBuf);
	}
		
}

/******************************************************************************
 GetCharBefore

	Return the character before aPosition. The character and its size
	are returned in charBuf, and aPosition is updated with the starting
	position of the character. If there is no preceding character, then
	Length(charBuf) is 0, and aPosition is not updated.
	
 ******************************************************************************/
 
void	CTermPane::GetCharBefore(long *aPosition, tCharBuf charBuf)
{
	long	totLength = GetLength();
	
	if ((totLength > 0) && (*aPosition <= totLength))
	{
		(*aPosition)--;
		GetCharAt(aPosition, charBuf);
	}
}

/******************************************************************************
 GetCharAfter

	Return the character after aPosition. The character and its size
	are returned in charBuf, and aPosition is updated with the starting
	position of the character. If there is no following character, then
	Length(charBuf) is 0, and aPosition is not updated.

 ******************************************************************************/
 
void	CTermPane::GetCharAfter(long *aPosition, tCharBuf charBuf)
{
	long	totLength = GetLength();
	
	if ((totLength > 0) && (*aPosition < totLength - 1))
	{
		(*aPosition)++;
		GetCharAt(aPosition, charBuf);
	}
}

/*
 * given a pixel count, return number of lines contained
 *
 * 
 *
 */
 
short	CTermPane::PixelsToLines(short	pixCount)
{
 short			windowSize,lineSize,ourSize;
 TermEnvironRec theEnvirons;
 
 GetEnvirons(&theEnvirons);

 lineSize = theEnvirons.cellSize.v;
 ourSize = pixCount / lineSize;
	
 if (ourSize > theEnvirons.textRows)
 {
 	windowSize = theEnvirons.textRows;
 }
 else
 {
	windowSize = ourSize;
 }

 return (windowSize);
}

/* 같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같 */

/*
 * GetTermExtent
 *
 * Return the size (in char rows and columns) of the terminal pane
 *
 * theHExtent:	Nb of columns
 * theVExtent:	Nb of rows
 *
 */
 
void CTermPane::GetTermExtent(long *theHExtent,long *theVExtent)
{
	TermEnvironRec theEnvirons;
	
	GetEnvirons(&theEnvirons);

	*theHExtent = theEnvirons.textCols;
	*theVExtent = theEnvirons.textRows;
}

/* 같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같 */

/*
 * AdjustBounds -	adjust the size of the virtual pane to reflect total # of 
 *					lines in the terminal pane, including cached ones.
 *
 * 
 *
 */
 
void	CTermPane::AdjustBounds(void)
{
	LongRect	newBounds;
	LongPt		newPoint;
	long		numTermCols, numTermRows;
	SignedByte	savedState;
	
	savedState = HGetState((Handle)itsTerm);
	HLock((Handle)itsTerm);
	
	QDToLongRect(&((*itsTerm)->viewRect),&newBounds);
	OffsetLongRect(&newBounds,-(newBounds.left),-(newBounds.top));
	
	GetExtent(&numTermCols, &numTermRows);
	newBounds.right = numTermCols;
	newBounds.bottom = numTermRows;
	
	GetPosition(&newPoint);
	
	SetBounds(&newBounds);
	SetPosition(&newPoint);
	
	HSetState((Handle)itsTerm,savedState);
}

/* 같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같 */

/*
 * AdjustTermPosition -	after terminal choose, adjust any terminal parameters.
 *
 * 
 *
 */
 
void	CTermPane::AdjustTermPosition(TermPtr oldRecord, TermEnvironRec *theOldEnvirons)
{
	TermEnvironRec	theEnvirons;

	GetEnvirons(&theEnvirons);
	
	if (theEnvirons.cellSize.v != theOldEnvirons->cellSize.v ||
		theEnvirons.cellSize.h != theOldEnvirons->cellSize.h)
	{
		long	oldVertOffset,oldHorzOffset,vertOffset,horzOffset;
		int		vDelta;
		
		oldVertOffset = oldRecord->viewRect.top / theOldEnvirons->cellSize.v;
		oldHorzOffset = oldRecord->viewRect.left / theOldEnvirons->cellSize.h;
		
		// make sure we leave don't scroll past the top margin.
		if (oldRecord->viewRect.top >= 0)
			vDelta = Max(0, oldRecord->viewRect.top - topMargin);
		else
			vDelta = Min(0, oldRecord->viewRect.top + topMargin);
	
		vertOffset = ((theEnvirons.cellSize.v * oldVertOffset) - vDelta);
		horzOffset = ((theEnvirons.cellSize.h * oldHorzOffset) - oldRecord->viewRect.left);
	
		ScrollTerminal(horzOffset,vertOffset);
	}
}
	
/* 같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같 */

/*
 * RefreshLine -	refresh the given line.
 *
 * 
 *
 */
 
void	CTermPane::RefreshLine(int line)
{
	Rect	r;
	
	if (CalcLineRect(line,&r))
	{
		Draw(&r);
	}
}

/* 같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같 */

/*
 * CalcLineRect -	calculate bounding rectangle of given line.
 *
 * 
 *
 */
 
CTermPane::CalcLineRect(int line, Rect *lineRect)
{
	LongRect		theLongArea;				/* frame coordinates of region */
	int				offsetX, offsetY, pixelsX, pixelsY;
	TermEnvironRec	theEnvirons;
	
	GetEnvirons(&theEnvirons);
	
	offsetY = theEnvirons.slop.v;
	offsetX = theEnvirons.slop.h + theEnvirons.auxSpace.left;
	
	pixelsX = theEnvirons.cellSize.h;
	pixelsY = theEnvirons.cellSize.v;
	
	GetFrame(&theLongArea);
	FrameToQDR(&theLongArea,lineRect);
	
	lineRect->left += offsetX;
	lineRect->right -= theEnvirons.slop.h + theEnvirons.auxSpace.right;
	
	lineRect->top += (line * pixelsY) + offsetY;
	lineRect->bottom = lineRect->top + pixelsY;
	
	OffsetRect(lineRect,0,CACHE_LINE_OFFSET);

	return lineRect->bottom > lineRect->top;
}

/* 같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같 */

/*
 * DeltaTermSize -	determine the difference between current frame size and
 * 					terminal frame size.
 *
 */
 
void CTermPane::DeltaTermSize(int *deltaH, int *deltaV)
{
	Rect		viewRect;
	LongRect	termFrame;
	SignedByte	savedState;
	
	savedState = HGetState((Handle)itsTerm);
	HLock((Handle)itsTerm);
	
	//QDToLongRect(&((*itsTerm)->viewRect),&viewRect);
	CalcTermViewRect(&viewRect);
	GetFrame(&termFrame);
	
	*deltaH = RectWidth(&viewRect) - RectWidth(&termFrame);
	*deltaV = (RectHeight(&viewRect) + topMargin + CACHE_LINE_OFFSET) - RectHeight(&termFrame);
	
	HSetState((Handle)itsTerm,savedState);
}
	
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// simple test to pump data to the terminal pane
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

void CTermPane::TestTerm(void)
{
 static	int	i;
 char	teststr[80];
 Point	itsPos;
 
 for (i=0; i<53; i++)
 {
	TMIdle(itsTerm);
 	itsPos = GetCursor();
 	sprintf(teststr,"This is a test, [yd] #%d (%d)... \n\r",i,itsPos.v);
 	DoStream(teststr,strlen(teststr),cmFlagsEOM);
	TMIdle(itsTerm);
 }
 	  
}

/******************************************************************************
 DrawLineRange

		Internal method to draw the specified range of text lines.  Assumes
		the pane has been prepared.
 ******************************************************************************/
	
void CTermPane::DrawLineRange(long startLine, long endLine, long startLineOffset, Boolean erase)
{
	TermDataBlock	theDataBlock;
	Rect			drawRect;
	long			dline;
	
	GetLine(1,&theDataBlock);
	DisposHandle(theDataBlock.theData);
	
	for (dline = startLine; dline <= endLine; dline++)
	{
	 int	soffset = dline + position.v;
	 char	szBuffer[128];
	 
	 itsScrollBuffer->GetLine(soffset,(unsigned char *)szBuffer,sizeof(szBuffer));
	 
	 PtrToHand(szBuffer,&theDataBlock.theData,strlen(szBuffer));
	
	 CalcLineRect(dline, &drawRect);
	 EraseRect(&drawRect);
	 
	 TMPaint(itsTerm,&theDataBlock,&drawRect);
	 DisposHandle(theDataBlock.theData);
	}
}

/******************************************************************************
 HiliteTextRange

		Internal method to highlight the given range of text.  Assumes
		the pane has been prepared.
 ******************************************************************************/

void	CTermPane::HiliteTextRange(long startChar, long endChar)
{
	long			startLine;
	long			endLine;
	LongPt			startPt;
	LongPt			endPt;
	LongRect		hiliteRect;
	PenState		penState;
	Rect			qdRect;
	Rect			termFrame;
	RgnHandle		hiliteRgn;
	RgnHandle		rectRgn;
	short			hSpan;
	short			vSpan, vTermSpan;
	short			lineHeight;
	short			charWidth;
	TermEnvironRec	theEnvirons;
	Boolean			isActive = reallyActive;
	long			scrollLen = itsScrollBuffer->GetTextLength();
	
	// if there's no scroll-buffer, there's nothing to hilight
	if (! scrollLen)
		return;
		
	// bounds check.
	startChar = Max(startChar, 0);
	startChar = Min(startChar, scrollLen);
	endChar = Max(endChar, 0);
	endChar = Min(endChar, scrollLen);
	endChar = Max(endChar, startChar);
	
	GetEnvirons(&theEnvirons);
	
	lineHeight = theEnvirons.cellSize.v;
	charWidth = theEnvirons.cellSize.h;
	
	LongToQDRect(&frame,&termFrame);
	
	vSpan = RectHeight(&termFrame) / theEnvirons.cellSize.v;
	
	CalcTermRect(&termFrame);
	
	if (isActive)
	{
		/* Outline the selection range if the pane is not active */
		
		if (!isActive)
		{
			hiliteRgn = NewRgn();
			rectRgn = NewRgn();
		}
		
		/* Get the starting and ending selection lines and the */
		/* number of lines spanned by the frame */
			
		startLine = FindLine(startChar);
		endLine = Min(FindLine(endChar),numCachedLines);
		startLine = Min(startLine,endLine);
		
		GetFrameSpan(&hSpan, &vTermSpan);
		
		/* Take a quick exit if the selection is not visible */
		/* within the frame span */
		
		if (startLine >= position.v + vSpan || endLine < position.v)
			return;
		
		/* Adjust the starting and ending selection lines if they */
		/* are outside the frame */
		
		if (startLine < position.v)
		{
			startLine = position.v;
			startChar = GetLineStart(startLine);
		}
		
		if (endLine >= position.v + vSpan)
		{
			endLine = position.v + vSpan;
			endChar = GetLineStart(endLine);
		}
		
		/* Get the frame coordinates corresponding to the */
		/* start and end of the selection range */
			
		GetCharPoint(startChar, &startPt);
		GetCharPoint(endChar, &endPt);
		
		/* Adjust the horizontal coordinates if the either of the starting */
		/* or ending characters lies at the beginning of a line */
		
		if (startChar == GetLineStart(startLine))
			startPt.h = termFrame.left;
		if (endChar == GetLineStart(endLine))
			endPt.h = termFrame.left;
		
		/* Check for and handle a multiple-line selection range */
		
		if (startPt.v != endPt.v)
		{
			/* Highlight the first line of the selection range */
			
			SetLongRect(&hiliteRect, startPt.h, startPt.v, termFrame.right, startPt.v + lineHeight);
			FrameToQDR(&hiliteRect, &qdRect);
			OffsetRect(&qdRect,-(position.h * charWidth),-(position.v * lineHeight));
			
			if (isActive)
				HiliteRect(&qdRect);
			else
			{
				qdRect.left -= 1;
				RectRgn(rectRgn, &qdRect);
				UnionRgn(rectRgn, hiliteRgn, hiliteRgn);
			}
			
			/* Highlight the middle lines of the selection range */
			
			hiliteRect.left = termFrame.left;
			
			if (isActive)
			{
				FrameToQDR(&hiliteRect, &qdRect);
				OffsetRect(&qdRect,-(position.h * charWidth),-(position.v * lineHeight));
				
				while (++startLine < endLine)
				{
					qdRect.top += lineHeight;
					qdRect.bottom += lineHeight;
					HiliteRect(&qdRect);
				}
			}
			else
			{
				hiliteRect.top += lineHeight;
				hiliteRect.left -= 1;
				hiliteRect.bottom = endPt.v + 1;
				FrameToQDR(&hiliteRect, &qdRect);
				OffsetRect(&qdRect,-(position.h),-(position.v));
				RectRgn(rectRgn, &qdRect);
				UnionRgn(rectRgn, hiliteRgn, hiliteRgn);
			}
			
			SetLongPt(&startPt, hiliteRect.left, endPt.v);
		}
		
		/* Hilite the last part of the selection range */
		
		SetLongRect(&hiliteRect, startPt.h, endPt.v, endPt.h, endPt.v + lineHeight);
		FrameToQDR(&hiliteRect, &qdRect);
		OffsetRect(&qdRect,-(position.h * charWidth),-(position.v * lineHeight));
		
		if (isActive)
			HiliteRect(&qdRect);
		else
		{
			if (qdRect.right > qdRect.left + 1)
			{
				qdRect.bottom += 1;
				RectRgn(rectRgn, &qdRect);
				UnionRgn(rectRgn, hiliteRgn, hiliteRgn);
			}
			
			/* Intersect the highlight region with the pane frame */
			
			FrameToQDR(&frame, &qdRect);
			RectRgn(rectRgn, &qdRect);
			SectRgn(rectRgn, hiliteRgn, hiliteRgn);
			DisposeRgn(rectRgn);
			
			/* Outline the highlight region */
			
			GetPenState(&penState);
			PenNormal();
			PenMode(patXor);
			FrameRgn(hiliteRgn);
			DisposeRgn(hiliteRgn);
			SetPenState(&penState);
		}
	}
}

/******************************************************************************
 HiliteRect
	
		Hilite the specified rectangle.  The rectangle is in QuickDraw
		coordinates.
 ******************************************************************************/

void	CTermPane::HiliteRect(const Rect *hiliteRect)
{
	SetHiliteMode();
	InvertRect(hiliteRect);
}

/******************************************************************************
 CalcWordBreak
	
		Return the character offset at which the next word break in the
		specified direction should occur.
 ******************************************************************************/

#define		TestBit(p,b)	(((long *)(p))[(b) >> 5] & (0x80000000U >> ((b) & 0x1F)))

long	CTermPane::CalcWordBreak(register long charPos, BreakDirection direction)
{
	tCharBuf 		charBuf;
	register long	textLength;
	register Byte	c;
	static long		wordBreaks[8] = {	/* Packed boolean flags to indicate	*/
		0x00000000, 0x0000FFC0,			/* whether or not a character is a	*/
		0x7FFFFFE1, 0x7FFFFFE0,			/* word break character				*/
		0x00000000, 0x00000000,
		0x00000000, 0x00000000
	};
	
	if (direction == kBreakLeft)
	{
		GetCharBefore(&charPos, charBuf);
		
		while (charPos && (c = charBuf[1], TestBit(wordBreaks, c)))
		{
			GetCharBefore(&charPos, charBuf);
		}
				
		return (charPos > 0 ? charPos + 1 : 0);
	}
	else
	{
		textLength = itsScrollBuffer->GetTextLength();
		
		GetCharAfter(&charPos, charBuf);
		
		while (charPos < textLength && (c = charBuf[1], TestBit(wordBreaks, c)))
		{
			GetCharAfter(&charPos, charBuf);
		}
		
		return (charPos);
	}
}

/******************************************************************************
 SetSelection {OVERRIDE}
 
 		Sets the selected text to the range corresponding to character
 		positions selStart through selEnd.
 ******************************************************************************/
 
void	CTermPane::SetSelection( long selStart, long selEnd, Boolean fRedraw)
{
		/* Ensure that the positions are valid */
	
	selStart = Max(selStart, 0);
	selStart = Min(selStart, GetLength());
	selEnd = Max(selEnd, 0);
	selEnd = Min(selEnd, GetLength());
	selEnd = Max(selEnd, selStart);
	
		/* Take a quick exit if the selection range isn't being changed */
	
	if (selStart == itsSelStart && selEnd == itsSelEnd)
		return;
	
		/* Unhilite the old selection range if redraw was specified */
		
	if (fRedraw)
	{
		Prepare();
		
		if (itsSelStart != itsSelEnd)
			HiliteTextRange(itsSelStart, itsSelEnd);
	}
	
		/* Update our instance variables */
	
	itsSelStart = selStart;
	itsSelEnd = selEnd;
	//upDownArrow = FALSE;
	
		/* Ensure that the anchor point is one of the selection endpoints */
	
	if (itsSelAnchor != selStart && itsSelAnchor != selEnd)
		itsSelAnchor = selStart;
		
		/* Hilite the old selection range if redraw was specified */
		
	if (fRedraw && selStart != selEnd)
	{
		HiliteTextRange(selStart, selEnd);
	}
}

void	CTermPane::GetSelection( long *selStart, long *selEnd)
{
	*selStart = itsSelStart;
	*selEnd = itsSelEnd;
}

/******************************************************************************
 LinesScrolledUp

		Calculate how many lines were scrolled up.
 ******************************************************************************/
 
long CTermPane::LinesScrolledUp(void)
{
	Rect			viewRect, termRect;
	TermEnvironRec	theEnvirons;
	
	GetEnvirons(&theEnvirons);
	
	viewRect = (*itsTerm)->viewRect;
	termRect = (*itsTerm)->termRect;
	
	return (viewRect.top - termRect.top) / theEnvirons.cellSize.v;
}
	
/******************************************************************************
 LinesScrolledDown

		Calculate how many lines are beneath the last visible line.
 ******************************************************************************/

long CTermPane::LinesScrolledDown(void)
{
	Rect			termRect;
	int				numVisRows;
	TermEnvironRec	theEnvirons;
	
	GetEnvirons(&theEnvirons);
	
	termRect = (*itsTerm)->termRect;
	
	CalcTermRect(&termRect);		// how many lines fit in the visible window?
	
	numVisRows = ((termRect.bottom - termRect.top) / theEnvirons.cellSize.v);
	numVisRows = theEnvirons.textRows - numVisRows;
	
	return numVisRows > 0 ? numVisRows : 0;
}

/******************************************************************************
 CalcTermRect

		reduce given rectangle by terminal margin.
 ******************************************************************************/
 
void	CTermPane::CalcTermRect(Rect *frameRect)
{
	TermEnvironRec	theEnvirons;
	
	GetEnvirons(&theEnvirons);
	
	InsetRect(frameRect, theEnvirons.slop.h, theEnvirons.slop.v);
	
	frameRect->left += theEnvirons.auxSpace.left;
	frameRect->right -= theEnvirons.auxSpace.right;
	frameRect->top += theEnvirons.auxSpace.top;
	frameRect->bottom -= theEnvirons.auxSpace.bottom;	
}

/******************************************************************************
 CalcFullRect

		expand given rectangle by terminal margin.
 ******************************************************************************/
 
void	CTermPane::CalcFullRect(Rect *frameRect)
{
	TermEnvironRec	theEnvirons;
	
	GetEnvirons(&theEnvirons);
	
	InsetRect(frameRect, -(theEnvirons.slop.h), -(theEnvirons.slop.v));
	
	frameRect->left -= theEnvirons.auxSpace.left;
	if (frameRect->left < 0)
	{
		frameRect->right -= frameRect->left;
		frameRect->left = 0;
	}
	frameRect->right += theEnvirons.auxSpace.right;
	frameRect->top -= theEnvirons.auxSpace.top;
	if (frameRect->top < 0)
	{
		frameRect->bottom -= frameRect->top;
		frameRect->top = 0;
	}
	frameRect->bottom += theEnvirons.auxSpace.bottom;	
}

/******************************************************************************
 CalcTermViewRect

		return size of original terminal pane.
 ******************************************************************************/
 
void	CTermPane::CalcTermViewRect(Rect *frameRect)
{
	TermEnvironRec	theEnvirons;
	short			termHeight, termWidth;
	
	GetEnvirons(&theEnvirons);
	
	termWidth = (theEnvirons.cellSize.h * theEnvirons.textCols);
	termHeight = (theEnvirons.cellSize.v * theEnvirons.textRows);
	
	termWidth += (theEnvirons.slop.h * 2) + theEnvirons.auxSpace.left + theEnvirons.auxSpace.right;
	termHeight += (theEnvirons.slop.v * 2) + theEnvirons.auxSpace.top + theEnvirons.auxSpace.bottom;
	
	SetRect(frameRect,0,0,termWidth,termHeight);
}

/******************************************************************************
 GetTermRect

		reduce given rectangle by terminal margin.
 ******************************************************************************/
 
void	CTermPane::GetTermRect(Rect *frameRect)
{
	LongRect	theLongArea;
	Rect		frameArea;
	
	GetFrame(&theLongArea);
	LongToQDRect(&theLongArea,&frameArea);
	
	CalcTermRect(&frameArea);
	
	*frameRect = frameArea;
}

/******************************************************************************
 EraseTermBorder

		erase the border around the terminal pane.
 ******************************************************************************/
 
void	CTermPane::EraseTermBorder(void)
{
	RgnHandle		termRgn, frameRgn, borderRgn;
	LongRect		theLongArea;
	Rect			frameRect, termRect;
	TermEnvironRec	theEnvirons;
	SignedByte		savedState;
	
	savedState = HGetState((Handle)itsTerm);
	HLock((Handle)itsTerm);
		
	Prepare();								// setup for drawing
	
	termRgn = NewRgn();
	frameRgn = NewRgn();
	borderRgn = NewRgn();
	
	GetFrame(&theLongArea);					// frame
	LongToQDRect(&theLongArea,&frameRect);
	
	frameRect.bottom = frameRect.top + height;
	
	termRect = (*itsTerm)->termRect;
	CalcTermRect(&termRect);				// terminal
	
	GetEnvirons(&theEnvirons);				// Environment
	
	if (position.v >= numCachedLines)
	{
		termRect.top += topMargin;
	}
	
	if ((position.v + theEnvirons.textRows) <= numCachedLines)
	{
		termRect.bottom -= topMargin;
	}
	
	RectRgn(frameRgn,&frameRect);
	RectRgn(termRgn,&termRect);

	DiffRgn(frameRgn,termRgn,borderRgn);	// border
	SetClip(borderRgn);
	EraseRgn(borderRgn);					// border erased
	
	DisposeRgn(termRgn);
	DisposeRgn(frameRgn);
	DisposeRgn(borderRgn);
	
	Prepare();								// restore clipping region
	
	HSetState((Handle)itsTerm,savedState);
}

/******************************************************************************
 TerminalTextLength

		calculate number of characters in TM buffer.
 ******************************************************************************/
 
long	CTermPane::TerminalTextLength(long startLine, long endLine)
{
	TermEnvironRec	theEnvirons;
	TermDataBlock	theDataBlock;
	short			lineno, firstDataLine;
	long			totLen = 0L;
	
	GetEnvirons(&theEnvirons);
	
	// get last line that has real data.
	for (firstDataLine = theEnvirons.textRows; firstDataLine >= 1; firstDataLine--)
	{
		GetLine(firstDataLine,&theDataBlock);
		
		char firstChar = *TrimRight(*theDataBlock.theData);
		
		DisposHandle(theDataBlock.theData);
		
		if (firstChar) break;
	}
	
	if (startLine < 1)
		startLine = 1;
	
	if (endLine < 1)
		endLine = theEnvirons.textRows;
	
	// limit to last line that has data.
	endLine = Min(endLine,firstDataLine);
		
	for (lineno = startLine; lineno <= endLine; lineno++)
	{
		GetLine(lineno,&theDataBlock);
		
		HLock(theDataBlock.theData);
		if (lineno < endLine)
			totLen += strlen(*theDataBlock.theData);
		else
			totLen += strlen(TrimRight(*theDataBlock.theData));
		HUnlock(theDataBlock.theData);
		
		DisposHandle(theDataBlock.theData);
	}
	
	return totLen;
}

/******************************************************************************
 ProcessCacheClick

		process a mouse click in the cache region.
		
		- Returns flag indicating if point was in cache region or not.
 ******************************************************************************/
 
Boolean CTermPane::ProcessCacheClick(Point hitPt)
{
	LongPt		autoScrollPt, framePt;
	long		line, selStart, selEnd;
	long		charOffset;
	SignedByte	savedState;
	Boolean		inCacheRegion;
	TermEnvironRec	theEnvirons;
	
	savedState = HGetState((Handle)itsTerm);
	HLock((Handle)itsTerm);
	
	GetEnvirons(&theEnvirons);
	
	QDToFrame(hitPt, &framePt);
	autoScrollPt = framePt;
	PinInRect(&frame, &framePt);
	
	framePt.v += position.v * theEnvirons.cellSize.v;
	framePt.h += position.h * theEnvirons.cellSize.h;
	
	charOffset = GetCharOffset(&framePt);
	inCacheRegion = charOffset < itsScrollBuffer->GetTextLength();

	if (charOffset != lastCharOffset)
	{
		long		newSelStart, newSelEnd;
		short		vertInset, horzInset;

		vertInset = theEnvirons.slop.v;
		horzInset = theEnvirons.slop.h + theEnvirons.auxSpace.left;
				
		/* Remember the current selection range */
			
		selStart = itsSelStart;
		selEnd = itsSelEnd;
			
		/* Determine the new selection range */
			
		if (gClicks == 1)
		{
			newSelStart = Min(origCharOffset, charOffset);
			newSelEnd = Max(origCharOffset, charOffset);
		}
		else if (gClicks == 2)
		{
			if (charOffset < origCharOffset)
			{
				newSelStart = CalcWordBreak(charOffset, kBreakLeft);
				newSelEnd = CalcWordBreak(origCharOffset, kBreakRight);
			}
			else
			{
				newSelStart = CalcWordBreak(origCharOffset, kBreakLeft);
				newSelEnd = CalcWordBreak(charOffset, kBreakRight);
			}
		}
		else
		{
			/* gClicks == 3 */
				
			// line that has the desired character.
			line = (framePt.v - vertInset) / theEnvirons.cellSize.v;
				
			if (line != lastLine)
			{
				if (line < origLine)
				{
					newSelStart = GetLineStart(line);
					newSelEnd = GetLineEnd(origLine);
				}
				else
				{
					newSelStart = GetLineStart(origLine);
					newSelEnd = GetLineEnd(line);
				}
					
				lastLine = line;
			}
			else
			{
				newSelStart = selStart;
				newSelEnd = selEnd;
			}
		}
			
		/* Adjust the hilited text */
			
		if (selStart != newSelStart)
			HiliteTextRange(Min(selStart, newSelStart), Max(selStart, newSelStart));
		if (selEnd != newSelEnd)
			HiliteTextRange(Min(selEnd, newSelEnd), Max(selEnd, newSelEnd));
			
		SetSelection(newSelStart, newSelEnd, kNoRedraw);
			
		/* Remember the current character offset */
			
		lastCharOffset = charOffset;
	}
	
	AutoScroll(&autoScrollPt);
	
	HSetState((Handle)itsTerm,savedState);
	
	return inCacheRegion;
}

/******************************************************************************
 DoCopy

		copy current selection to scrap.
		
 ******************************************************************************/
 
void CTermPane::DoCopy(short copyType)
{
	if (itsSelStart != itsSelEnd)
	{
		register long	scrollLen = itsScrollBuffer->GetTextLength();
		TermEnvironRec	theEnvirons;
		Handle			scrollHdl = nil, termHdl = nil;
		Handle			selectionHdl;
		ResType			selectionType = 'TEXT';
 
		GetEnvirons(&theEnvirons);
		
		// empty the clipboard.
		gClipboard->EmptyScrap();
		
		// put the selected contents of the scrollback buffer into a buffer.
		if (itsSelStart < scrollLen && theEnvirons.termType == tmTextTerminal)
		{
			long		selStart = Max(itsSelStart,0);
			long		selEnd = Min(itsSelEnd - 1,scrollLen);
			
			// if there really is something to copy, go ahead.
			if (selEnd > selStart)
			{
				scrollHdl = itsScrollBuffer->CopyTextRange(selStart, selEnd);
				
				// if tab copy, replace spaces w/tabs
				if (copyType == kTabCopy)
					EnTab(scrollHdl);
			}
		}
		
		// now, put the selected contents of the terminal into a buffer.
		if (itsSelEnd >= scrollLen)
		{
			termHdl = NewHandle(0);
			TMGetSelect(itsTerm,termHdl,&selectionType);
			
			// if tab copy, replace spaces w/tabs
			if (copyType == kTabCopy)
				EnTab(termHdl);
				
			HLock(termHdl);
		}
		
		// make sure we have at least one valid handle
		if (termHdl || scrollHdl)
		{
			// if terminal buffer exists, may have to concatenate to the scrollback buffer
			if (termHdl)
			{
				if (scrollHdl)
				{
					OSErr	theErr;
					
					// concatenate scrollback to terminal select buffer
					theErr = HandAndHand(termHdl,scrollHdl);
					FailOSErr(theErr);
					
					selectionHdl = scrollHdl;
					
					ForgetHandle(termHdl);
				}
				else 
				{
					selectionHdl = termHdl;
				}
			}
			else selectionHdl = scrollHdl;
			
			// put either one or both buffers to the clipboard.
			gClipboard->PutData(selectionType,selectionHdl);
		}
		
		// dispose of the handles to selected data.
		ForgetHandle(scrollHdl);
		ForgetHandle(termHdl);
	}
}

/******************************************************************************
 GetFrameSpan - OVERIDE

 ******************************************************************************/

void	CTermPane::GetFrameSpan(
	short		*theHSpan,
	short		*theVSpan)
{
	Rect		newFrame;
  			
	LongToQDRect(&frame,&newFrame);
		
	CalcTermRect(&newFrame);
		
	*theHSpan = (newFrame.right - newFrame.left) / hScale;
	*theVSpan = (newFrame.bottom - newFrame.top) / vScale;
}

/* 같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같 */

/*
 * EnTab
 *
 * Replace spaces w/tabs in buffer pointed to by given handle
 *
 */
 
long CTermPane::EnTab(Handle selectionHdl)
{
	register long			selectionLength,scrapLength;
	register short			i,scrapIndex,spacePending;
	register unsigned char	theChar;
	
	selectionLength = GetHandleSize(selectionHdl);
	scrapLength = selectionLength;
	
	spacePending = 0;
	scrapIndex = 0;
	
	for (i = 0;i < selectionLength;i ++)
	{	
		/* For each char� */
	
		theChar = (*selectionHdl)[i];
		
		if (theChar == SPACE)		/* Space char ? */
			spacePending ++;
		else
		{	
			if (spacePending > 1)
			{
				/* More than one space ? */
				(*selectionHdl)[scrapIndex] = TAB;
				
				scrapIndex ++;
				scrapLength -= (spacePending - 1);
				
				(*selectionHdl)[scrapIndex] = theChar;
				scrapIndex ++;
				spacePending = 0;
			}
			else if (spacePending == 1)
			{
				/* Exactly one space ? */
				(*selectionHdl)[scrapIndex] = TAB;
				scrapIndex ++;
				(*selectionHdl)[scrapIndex] = theChar;
				scrapIndex ++;
				spacePending = 0;
			}
			else
			{
				(*selectionHdl)[scrapIndex] = theChar;
				scrapIndex ++;
			}						
		}
	}
	
	SetHandleSize(selectionHdl,scrapLength);
	
	return scrapLength;
}

