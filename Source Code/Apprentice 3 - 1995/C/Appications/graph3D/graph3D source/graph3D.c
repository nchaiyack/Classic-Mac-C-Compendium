/*
	Copyright '89	Christopher Moll
	all rights reserved
*/


#include	"graph3D.h"

#ifdef	_LSC3_
#	include <ColorToolbox.h>
#	include <MemoryMgr.h>
#	include <FileMgr.h>
#	include <WindowMgr.h>
#	include <DialogMgr.h>
#	include <EventMgr.h>
#	include <TextEdit.h>
#	include <DeskMgr.h>
#	include <MenuMgr.h>
#	include <ScrapMgr.h>
#endif


extern	Real	startR, startT;
extern	Real	endR, endT;
extern	Real	deltaR, deltaT;
extern	int		numR, numT;

extern	Real	startX, startY;
extern	Real	endX, endY;
extern	Real	deltaX, deltaY;
extern	int		numX, numY;

extern	WindowRecord	gwindRecord;

extern	Uchar		*gridFace;



/* windows */
GrafPtr			windManPort;

WindowPtr		graphWind;


/* Dialogs */
DialogRecord	sdialRecord;	/* storage for the dialog */
DialogPtr		specDial;
Boolean			specDOpen = FALSE;
DialogRecord	lmdialRecord;	/* storage for the dialog */
DialogPtr		limtDial;
Boolean			limtDOpen = FALSE;
DialogRecord	funcDialRecord;	/* storage for the dialog */
DialogPtr		functDial;
Boolean			functDOpen = FALSE;


/* menus */
MenuHandle		appleMenu, fileMenu, editMenu, plotMenu,
				gTypeMenu, windwMenu;


/* Clipboard stuff */
int			scrapCompare;	/* status of the deskscrap (clipboard) */
Boolean		scrapDirty;		/* deskscrap has been changed */
Boolean		isScrapTE;		/* there's text in the deskscrap */
Boolean		isScrapPIC;		/* there's a picture in the deskscrap */

Boolean		appActive = TRUE;


Boolean		useMainFunc;

int		mainFunct[64];
int		numMnOps;
Real	mnConsts[30];
int		numMnConsts = 0;

int		derivFunct[64];
int		numDerivOps;
Real	derivConsts[30];
int		numDerivConsts = 0;


/* graph points in various forms */
Real		*funcResults = NIL;		/* changes when function changed,
										new file opened, etc. */
Vector		*scalVectResults = NIL;		/* changes when function changed,
										new file opened, etc. */
Point		*graphPoints = NIL;		/* changes when rotation changes */

Vector		maxVect, minVect;

Point	XaxisPt, YaxisPt, ZaxisPt;
Point	Origin = {0, 0};
Rect	graphRect = {40, 40, 300, 470};



Real	rotMatrx[3][3];

Real	scale;


Boolean		functCurrent = FALSE;	/* function evaluated and stored in funcResults*/
Boolean		specsCurrent = FALSE;	/* funtion evaluated according to current specification
										dialog contents ('begin', 'end').
													If not , reevaluate function */
Boolean		vectrsCurrent = FALSE;	/* vectors have been set from the function */
Boolean		pntsCurrent = FALSE;	/* pnts have been taken from the vectors and scaled
										to fit in window */

Boolean		noRedrawGrph = FALSE;	/* graph being worked on;
									don't try to redraw it */

Boolean		hideSurface = FALSE, useHeight = FALSE;
Boolean		xAxPos, yAxPos;
int			graphType = FUNCT_GTYPE;

Boolean		drawToScreen = FALSE;


Real	zScale = 1.0;
Real	ceilgZ = 1e100, floorZ = -1e100;

Real	cosSrc = .86602540378444, sinSrc = .5;	/* 30 degrees */


Boolean		colorQD, scrnColor, colorNorms;
Boolean		grphOnScrn = FALSE;

PatternList		*greyList;


EventRecord		theEvent;
Boolean			inMultiFndr;
long			sleepPeriod = 2;

long		startRedraw = -1;

long    MoveToAddr, LineToAddr, FixMulAddr;


void	ReDrwLine(...);


main()
{
	DoSetup();
	SelectWindow(graphWind);

	while (1)
	{
		DialogIdle();
		if (inMultiFndr)
		{
			if (WaitNextEvent(everyEvent, &theEvent, sleepPeriod, NIL))
			{
				DoEvent();
				InitCursor();
			}
		}
		else
		{
			if (GetNextEvent(everyEvent, &theEvent))
			{
				DoEvent();
				InitCursor();
			}
			SystemTask();
		}
	}
}

static
DialogIdle()
{
	WindowPtr	frontW;

	frontW = FrontWindow();

	if (frontW EQ functDial)
		TEIdle(funcDialRecord.textH);
	else if (frontW EQ specDial)
		TEIdle(sdialRecord.textH);
	else if (frontW EQ limtDial)
		TEIdle(lmdialRecord.textH);
}

DoEvent()
{
	DialogPtr	theDial;
	int			theItem;
	Boolean		result, ScrnHasColor();

	scrnColor = ScrnHasColor();	/* check current screen status */

	if (theEvent.what EQ resumEvt)
		DoSuspndRsm(theEvent.message);
	
	if (IsDialogEvent(&theEvent))
		DoDialogEvent();
	else
	{
		switch (theEvent.what) {
		case nullEvent:
			break;
		case mouseDown:
			DoMouseDown();
			break;
		case mouseUp:
			break;
		case keyDown:
		case autoKey:
			DoKeyDown();
			break;
		case updateEvt:
			DoUpdate();
			break;
		case diskEvt:
			break;
		case activateEvt:
			DoActivate();
			break;
		case networkEvt:
		case driverEvt:
			break;
		}
	}
}

static
DoDialogEvent()
{
	DialogPtr	theDial;
	int			theItem;
	Boolean		ChkCmndKey(), ChkNmbrKey();

	theDial = FrontWindow();

	if ((theEvent.what EQ keyDown) OR (theEvent.what EQ autoKey))
	{
		if (ChkCmndKey(theDial))
			return;
		if ((theDial EQ specDial) OR (theDial EQ limtDial))
		{
			if (NOT(ChkNmbrKey()))
				return;
		}
		else
			functCurrent = FALSE;
	}

	if (theEvent.what EQ activateEvt)	/* if it's a DA, get or write clip */
	{
		if (theEvent.modifiers & activeFlag)
			ReadDeskScrap();
		else
			WriteDeskScrap();
	}

	DialogSelect(&theEvent, &theDial, &theItem);
	if (theItem EQ DIALI_S_REDRWB)
		DoRedraw();
}

/*** Force an update of the graph ***/
InvalidGraph()
{
	GrafPtr	savePort;

	GetPort(&savePort);
	SetPort(graphWind);
		InvalRect(&graphWind->portRect);
	SetPort(savePort);
}

/*** Called when a modeless dialog is frontmost; checks for and
		deals with command keys ***/
static
Boolean
ChkCmndKey(theDial)
DialogPtr	theDial;
{
	if (theEvent.modifiers & cmdKey)
	{
		DoKeyDown();
		return(TRUE);
	}

	return(FALSE);
}

static
Boolean
ChkNmbrKey()
{
	char	typed;

	typed = (char)(theEvent.message & charCodeMask);

	if (IS_DIGIT(typed))
		return(TRUE);

	switch (typed)
	{
	case '-':
	case '.':
	case 'e':
	case '\t':
	case '\b':
		return(TRUE);
	}

	return(FALSE);
}

static
DoMouseDown()
{
	int thePart;
	WindowPtr whichWindow;

	thePart = FindWindow( theEvent.where, &whichWindow);

	switch (thePart) {
	case inDesk:
		break;
	case inMenuBar:
		DoMenuClick();
		break;
	case inSysWindow:
		SystemClick(&theEvent, whichWindow);
		break;
	case inContent:
		DoContent(whichWindow);
		break;
	case inDrag:
		DoDrag(whichWindow);
		break;
	case inGrow:
		DoGrow(whichWindow);
		break;
	case inGoAway:
		DoGoAway(whichWindow, TRUE);
		break;
	case inZoomIn:
	case inZoomOut:
		DoZoom(whichWindow, thePart);
		break;
	}
}

static
DoMenuClick()
{
	long menuChoice;

	menuChoice = MenuSelect(theEvent.where);
	DoMenuChoice(menuChoice);
}

static
DoMenuChoice(menuChoice)
long menuChoice;
{
	Clock();
	switch(HiWord(menuChoice)) {
	case MENU_APPLE:
		DoAppleChoice( LoWord(menuChoice) );
		break;
	case MENU_FILE:
		DoFileMenu(LoWord(menuChoice));
		break;
	case MENU_EDIT:
		DoEditMenu(LoWord(menuChoice));
		break;
	case MENU_PLOT:
		DoPlotMenu(LoWord(menuChoice));
		break;
	case MENU_GTYP:
		DoGTypeMenu(LoWord(menuChoice));
		break;
	case MENU_WIND:
		DoWindwMenu(LoWord(menuChoice));
		break;
	}

	HiliteMenu(0); /* unhilight the selected menu */
}

static
DoAppleChoice(theItem)
int theItem;
{
	char	accName[256];
	int		accNum, i;

	WriteDeskScrap();

	if (theItem EQ 1)
		DoAbout();
	else
	{
		GetItem(appleMenu, theItem, accName);
		accNum = OpenDeskAcc(accName);
	}
}

static
DoAbout()
{
	InitCursor();
	Alert(ALRT_ABOUT, NIL);
}

static
DoFileMenu(theItem)
int theItem;
{
	Boolean		ReadData();

	switch(theItem) {
	case FILE_OPEN:
		if (graphType NEQ FDATA_GTYPE)
			DoGTypeMenu(FDATA_GTYPE);	/* "Data File" from gtype menu */
		else
		{
			if (ReadData())
				InvalidGraph();
		}
		break;
	case FILE_OPEN_MAND:
		OpenMandel();
		break;
	case FILE_CLOSE:
		DoGoAway(FrontWindow(), FALSE);
		break;
	case FILE_SVBITM:
		SavePICT(FALSE);
		break;
	case FILE_SVOBJ:
		SavePICT(TRUE);
		break;
	case FILE_QUIT:
		DoQuit();
		break;
	}
}

static
DoQuit()
{
	CloseWindow(graphWind);
	if (scrnColor)
		GetNewCWindow(130, &gwindRecord, (WindowPtr)-1L);
	exit();
}

static
DoEditMenu(theItem)
int theItem;
{
	WindowPtr	frontW;
	frontW = FrontWindow();

	switch(theItem) {
	case 1: /* Undo */
		SystemEdit(undoCmd);
		break;
	case 3: /* Cut */
		if (NOT(SystemEdit(cutCmd)))
			DoCut(frontW);
		break;
	case 4: /* Copy */
		if (NOT(SystemEdit(copyCmd)))
			DoCopy(frontW);
		break;
	case 5: /* Paste */
		if (NOT(SystemEdit(pasteCmd)))
			DoPaste(frontW);
		break;
	case 6: /* Clear */
		if (NOT(SystemEdit(clearCmd)))
			DoClear(frontW);
		break;
	}
}

static
DoCut(frontW)
WindowPtr	frontW;
{
	Boolean		IsDialog();
	
	if (IsDialog(frontW))
	{
		DlgCut(frontW);
		scrapDirty = TRUE;
	}
}

static
DoCopy(frontW)
WindowPtr	frontW;
{
	Boolean		IsDialog();
	
	if (IsDialog(frontW))
		DlgCopy(frontW);
	else if (frontW EQ graphWind)
		CopyGraph();
	scrapDirty = TRUE;
}

static
CopyGraph()
{
	PicHandle	GetGraphPICT();
	PicHandle	graphPict;

	graphPict = GetGraphPICT(OptionDown());
	if (NOT(graphPict))
		return;

	ZeroScrap();
	PutScrap(GetHandleSize(graphPict), 'PICT', *graphPict);
	isScrapPIC = TRUE;
	isScrapTE = FALSE;
}

static
DoPaste(frontW)
WindowPtr	frontW;
{
	Boolean		IsDialog();
	
	if (IsDialog(frontW))
		DlgPaste(frontW);
}

static
DoClear(frontW)
WindowPtr	frontW;
{
	Boolean		IsDialog();
	
	if (IsDialog(frontW))
		DlgDelete(frontW);
}

static
Boolean
IsDialog(theWind)
WindowPtr	theWind;
{
	return
			((theWind EQ specDial) OR 
			(theWind EQ limtDial) OR 
			(theWind EQ functDial));
}

static
DoPlotMenu(theItem)
int theItem;
{
	switch(theItem) {
	case PLOT_WIREFRAM: /* just lines */
		hideSurface = FALSE;
		useHeight = FALSE;
		CheckItem(plotMenu, PLOT_WIREFRAM, TRUE);
		CheckItem(plotMenu, PLOT_SHDSURF, FALSE);
		CheckItem(plotMenu, PLOT_SHDHITE, FALSE);
		InvalidGraph();
		break;
	case PLOT_SHDSURF: /* Hide surfaces */
		CheckItem(plotMenu, PLOT_WIREFRAM, FALSE);
		if (useHeight)	/* hideSurface TRUE */
		{
			useHeight = FALSE;
			pntsCurrent = FALSE;
			CheckItem(plotMenu, PLOT_SHDSURF, TRUE);
			CheckItem(plotMenu, PLOT_SHDHITE, FALSE);
			CheckHideView();
			InvalidGraph();
		}
		else if (NOT(hideSurface))
		{
			hideSurface = TRUE;
			useHeight = FALSE;
			pntsCurrent = FALSE;
			CheckItem(plotMenu, PLOT_SHDSURF, TRUE);
			CheckHideView();
			InvalidGraph();
		}
		break;
	case PLOT_SHDHITE: /* Shade by hight */
		CheckItem(plotMenu, PLOT_WIREFRAM, FALSE);
		if (NOT(useHeight))
		{
			hideSurface = TRUE;
			useHeight = TRUE;
			pntsCurrent = FALSE;
			CheckItem(plotMenu, PLOT_SHDSURF, FALSE);
			CheckItem(plotMenu, PLOT_SHDHITE, TRUE);
			CheckHideView();
			InvalidGraph();
		}
		break;
	case PLOT_DRWSCRN: /* Draw to screen or not */
		ToggleDrwScreen();
		break;
	case PLOT_SETLITE: /* Set light source */
		SetLightSrc();
		break;
	case PLOT_ZEROROT: /* Zero Rotation */
		MkIDMat(rotMatrx);
		vectrsCurrent = FALSE;
		InvalidGraph();
		break;
	case PLOT_NORMVIEW: /* Normal View */
		MkRotMat(0.0, -.3, -.3, rotMatrx);
		vectrsCurrent = FALSE;
		InvalidGraph();
		break;
	case PLOT_TOPVIEW: /* Top View */
		MkRotMat(0.0, PI/2.0, 0.0, rotMatrx);
		vectrsCurrent = FALSE;
		InvalidGraph();
		break;
	case PLOT_REDRAW: /* ReDraw */
		DoRedraw();
		break;
	}
	SetPlotMenu();
}

CheckHideView()
{
	Vector		testVect, resVect;

	testVect.x = 1;
	testVect.y = 0;
	testVect.z = 0;
	MatrxMul(rotMatrx, &testVect, &resVect);

	xAxPos = (resVect.x > 0);
	testVect.x = 0;
	testVect.y = 1;
	MatrxMul(rotMatrx, &testVect, &resVect);
	yAxPos = (resVect.x > 0);
}

ToggleDrwScreen()
{
	drawToScreen = NOT(drawToScreen);
	CheckItem(plotMenu, PLOT_DRWSCRN, drawToScreen);
	InvalidGraph();
}

static
SetLightSrc()
{
	Point	center, chosen;
	EventRecord	msEvent;
	Real	horiz, vert, len;

	InitCursor();
	SelectWindow(graphWind);
	center.h = (graphWind->portRect.right - graphWind->portRect.left) >> 1;
	center.v = (graphWind->portRect.bottom - graphWind->portRect.top) >> 1;

	while (!GetNextEvent(mDownMask, &msEvent));
	chosen = msEvent.where;
	GlobalToLocal(&chosen);

	horiz = chosen.h - ((graphWind->portRect.right - graphWind->portRect.left) >> 1);
	vert = chosen.v - ((graphWind->portRect.bottom - graphWind->portRect.top) >> 1);
	len = SQ(horiz) + SQ(vert);
	len = -sqrt(len);
	sinSrc = horiz / len;
	cosSrc = vert / len;

	if (hideSurface AND NOT(useHeight))
		InvalidGraph();
	pntsCurrent = FALSE;
}

static
DoRedraw()
{
	GetLimtDCont();

	startRedraw = Ticks;

	if (graphType NEQ FDATA_GTYPE)
	{
		GetSpecDCont();
		if (NOT(specsCurrent AND functCurrent))
			if ((graphType EQ FUNCT_GTYPE) OR (graphType EQ DERIV_GTYPE))
				EvalFunction();
			else
				EvalCylindFunct();
	}
	
	InvalidGraph();
}

DoGTypeMenu(theItem)
int theItem;
{
Rect	fWindRect;

	switch(theItem) {
	case FUNCT_GTYPE: /* Function */
		SetToFunction();
		break;
	case CYLIND_GTYPE: /* Cylindrical */
		SetToCylind();
		break;
	case DERIV_GTYPE: /* Derivative */
		SetToDeriv();
		break;
	case FDATA_GTYPE: /* File Data */
		SetToDataFile();
		break;
	}
}

static
SetToFunction()
{
	if (graphType NEQ FUNCT_GTYPE)
	{
		CheckItem(gTypeMenu, graphType, FALSE);
		SetFunctDialType(FUNCT_GTYPE);
		CheckItem(gTypeMenu, graphType, TRUE);
	}
}

static
SetToCylind()
{
	if (graphType NEQ CYLIND_GTYPE)
	{
		CheckItem(gTypeMenu, graphType, FALSE);
		SetFunctDialType(CYLIND_GTYPE);
		CheckItem(gTypeMenu, graphType, TRUE);
	}
}

static
SetToDeriv()
{
	if (graphType NEQ DERIV_GTYPE)
	{
		CheckItem(gTypeMenu, graphType, FALSE);
		SetFunctDialType(DERIV_GTYPE);
		CheckItem(gTypeMenu, graphType, TRUE);
	}
}

ARD(a)
register int a;
{
	DeBug();
}

static
SetToDataFile()
{
	Boolean		ReadData();

	if (graphType NEQ FDATA_GTYPE)
		if (ReadData())
		{
			CheckItem(gTypeMenu, graphType, FALSE);
			SetFunctDialType(FDATA_GTYPE);
			CheckItem(gTypeMenu, graphType, TRUE);
			InvalidGraph();
		}
}

static
SetFunctDialType(newType)
int	newType;
{
	int	oldType;
	
	oldType = graphType;
	graphType = newType;

	CloseSpecDial();
	ShowSpecs();

	if (newType EQ FDATA_GTYPE)
	{
		if ((oldType EQ FUNCT_GTYPE) OR (oldType EQ DERIV_GTYPE))
			CloseFunctDial();
	}
	else
	{
		CloseFunctDial();
		ShowFunctDial();
	}
}

static
CloseSpecDial()
{
	if (specDOpen)
	{
		CloseDialog(specDial);
		specDOpen = FALSE;
	}
}

static
DoWindwMenu(theItem)
int theItem;
{
	switch	(theItem)
	{
	case WINDMEN_GRAPH: /* Show graph */
		SelectWindow(graphWind);
		break;
	case WINDMEN_FUNCT: /* Show function */
		ShowFunctDial();
		break;
	case WINDMEN_SPECS: /* Show Plot Specs*/
		ShowSpecs();
		break;
	case WINDMEN_LIMITS: /* Show Limits */
		ShowLimits();
		break;
	}
}

static
DoContent(whichWindow)
WindowPtr whichWindow;
{
	if (whichWindow NEQ FrontWindow())
		SelectWindow(whichWindow);
	else if (FrontWindow() EQ graphWind)
		ClikRot();
}

Rect	hndRct[5], vertRect, horizRect, centrRect;
Point	drawWCntr;

static
ClikRot()
{
	Real	newRot[3][3], temp[3][3];
	Real	rot;
	Point	msDLoc;
	int		quad;
	int		WhatQuad();
	int		XArcShow(), PYArcShow(), NZArcShow(), PZArcShow(), NYArcShow();

	msDLoc = theEvent.where;
	GlobalToLocal(&msDLoc);
	quad = WhatQuad(&msDLoc);
	if (quad EQ -1)
		return;

	switch (quad)
	{
	case 0:
		rot = -DEGtoRAD * NYArcShow(&vertRect);
		MkYRotMat(rot, newRot);
		if (rot NEQ 0.0)
			SRotatePoints(cos(rot), sin(rot), 2);
		break;
	case 3:
		rot = -DEGtoRAD * NZArcShow(&horizRect);
		MkZRotMat(rot, newRot);
		if (rot NEQ 0.0)
			SRotatePoints(cos(rot), sin(rot), 3);
		break;
	case 1:
		rot = DEGtoRAD * PYArcShow(&vertRect);
		MkYRotMat(rot, newRot);
		if (rot NEQ 0.0)
			SRotatePoints(cos(rot), sin(rot), 2);
		break;
	case 2:
		rot = DEGtoRAD * PZArcShow(&horizRect);
		MkZRotMat(rot, newRot);
		if (rot NEQ 0.0)
			SRotatePoints(cos(rot), sin(rot), 3);
		break;
	case 4:
		rot = DEGtoRAD * XArcShow(&centrRect);
		MkXRotMat(rot, newRot);
		if (rot NEQ 0.0)
			SRotatePoints(cos(rot), sin(rot), 4);
		break;
	}
	pntsCurrent = FALSE;

	MatrxByMatrx(newRot, rotMatrx, temp);
	CopyMat(temp, rotMatrx);

	InvalidGraph();
}

static
DoDrag(whichWindow)
WindowPtr whichWindow;
{
	Rect limitRect;

	SetRect(&limitRect, 0, 0, 2000, 2000);
	DragWindow(whichWindow, theEvent.where, &limitRect);
}

DoGrow(whichWindow)
WindowPtr whichWindow;
{
	Rect limitRect;
	long	newSize;

	if (whichWindow EQ graphWind)
	{
		SetRect(&limitRect, 50, 50, 2000, 2000);
		newSize = GrowWindow(whichWindow, theEvent.where, &limitRect);
		if (newSize)
		{
			SizeWindow(whichWindow, LoWord(newSize), HiWord(newSize), TRUE);
			if (whichWindow EQ graphWind)
			{
				SetGraphRect();
				vectrsCurrent = FALSE;
				InvalidGraph();
			}
		}
	}
}

SetGraphRect()
{
	graphRect = graphWind->portRect;
	InsetRect(&graphRect, 2, 2);
	SetRotateHandles();
}

static
DoGoAway(whichWindow, needTrack)
WindowPtr	whichWindow;
Boolean		needTrack;
{
	if (needTrack)
		if (NOT(TrackGoAway(whichWindow, theEvent.where)))
			return;
	if (whichWindow EQ graphWind)
		exit();
	else if (whichWindow EQ functDial)
		CloseFunctDial();
	else if (whichWindow EQ specDial)
		CloseSpecDial();
	else if (whichWindow EQ limtDial)
	{
		GetLimtDCont();
		CloseDialog(limtDial);
		limtDOpen = FALSE;
	}
}

static
CloseFunctDial()
{
	if (functDOpen)
	{
		CloseDialog(functDial);
		functDOpen = FALSE;
	}
}

GetSpecDCont()
{
	Real	temp;
	Real	DialogField();

	if (NOT(specDOpen))
		return;

	temp = DialogField(specDial, DIALI_S_SX);
	specsCurrent = startX EQ temp;
	startX = temp;

	temp = DialogField(specDial, DIALI_S_EX);
	specsCurrent = specsCurrent AND (endX EQ temp);
	endX = temp;

	temp = DialogField(specDial, DIALI_S_SY);
	specsCurrent = specsCurrent AND (startY EQ temp);
	startY = temp;

	temp = DialogField(specDial, DIALI_S_EY);
	specsCurrent = specsCurrent AND (endY EQ temp);
	endY = temp;

	temp = (int)DialogField(specDial, 14);
	if ((numX-1) NEQ temp)
	{
		specsCurrent = FALSE;
		numX = temp;
		++numX;
	}

	temp = (int)DialogField(specDial, 15);
	if ((numY-1) NEQ temp)
	{
		specsCurrent = FALSE;
		numY = temp;
		++numY;
	}
	if (graphType EQ CYLIND_GTYPE)
	{
		startR = startX;
		endR = endX;
		numR = numX;
		startT = startY * DEGtoRAD;
		endT = endY * DEGtoRAD;
		numT = numY;
	}
}

static
Real
DialogField(theDialog, theItem)
DialogPtr	theDialog;
int	theItem;
{
	int		itemType;
	Handle	itemHndle;
	Rect	itemRect;
	char	contStr[256];
	Real	atof(), res;

	GetDItem(theDialog, theItem, &itemType, &itemHndle, &itemRect);
	GetIText(itemHndle, contStr);

	ptoc(contStr);
	res = atof(contStr);
	return(res);
}

GetLimtDCont()
{
	Real	temp;
	Real	DialogField();

	if (NOT(limtDOpen))
		return;

	temp = DialogField(limtDial, DIALI_L_ZS);
	vectrsCurrent = zScale EQ temp;
	zScale = temp;

	temp = DialogField(limtDial, DIALI_L_C);
	vectrsCurrent = vectrsCurrent AND (ceilgZ EQ temp);
	ceilgZ = temp;

	temp = DialogField(limtDial, DIALI_L_F);
	vectrsCurrent = vectrsCurrent AND (floorZ EQ temp);
	floorZ = temp;
}

/***  zoom box clicked  ***/
static
DoZoom(whichWindow, thePart)
WindowPtr	whichWindow;
int			thePart;
{
	if (TrackBox(whichWindow, theEvent.where, thePart))	/* ROM */
	{
		ZoomWindow(whichWindow, thePart, TRUE);	/* ROM */
		if (whichWindow EQ graphWind)
		{
			SetGraphRect();
			vectrsCurrent = FALSE;
		}
	}
}

static
DoKeyDown()
{
	unsigned	char typed;
	long menuChoice;

	typed = theEvent.message & charCodeMask;

	if ((theEvent.modifiers & cmdKey)
		AND (theEvent.what NEQ autoKey)) /* ignore repeats */
	{
		menuChoice = MenuKey(typed);
		if (menuChoice)
			DoMenuChoice(menuChoice);
	}
}

static
DoUpdate()
{
    GrafPtr savePort;		/* to save and restore the old port */
	WindowPtr whichWindow;

	Clock();
	whichWindow = (WindowPtr) (theEvent.message);

    BeginUpdate(whichWindow);	/* reset ClipRgn etc to only redraw what's
				 			  necessary. */

    	GetPort(&savePort);		/* don't trash the port; we might be
								   updating an inactive window */
    	SetPort(whichWindow);	/* work in the specified window */

		DrawWindow(whichWindow);

	EndUpdate(whichWindow);

	SetPort(savePort);
}

static
DrawWindow(whichWindow)
WindowPtr whichWindow;
{
	if (whichWindow EQ graphWind)
		UpdatGraphWind();
}

static
UpdatGraphWind()
{
	ClipRect(&graphWind->portRect);

	if (noRedrawGrph)
		return;

	if (NOT(vectrsCurrent))
	{
		if (graphType EQ CYLIND_GTYPE)
			SetCylindVectrs();
		else
			SetVectrs();
		pntsCurrent = FALSE;
	}
	if (hideSurface)
		if (scrnColor NEQ colorNorms)
			pntsCurrent = FALSE;
	if (NOT(pntsCurrent))
		SetPoints();

	CheckGrphWind();
	DrawGraph();
	PutGraphGrow();
	if (startRedraw NEQ -1)
		if ((Ticks - startRedraw) > 1800)
			SysBeep(1);
	startRedraw  = -1;
	DrawRotHndls();
}

PutGraphGrow()
{
	Rect	theRect;

	theRect = graphWind->portRect;
	theRect.left = theRect.right - SCBAR_WIDTH;
	theRect.top = theRect.bottom - SCBAR_WIDTH;
	ClipRect(&theRect);
	DrawGrowIcon(graphWind);
	ClipRect(&graphWind->portRect);
}

DrawGraph()
{
register	int	xCnt, yCnt;
register	Point	*grphPnts;
	Boolean		CmndPeriod();

	if (hideSurface)
	{
		DrawHiddenGraph();
		return;
	}

	EraseRect(&thePort->portRect);
	DrawAxes();
	if (drawToScreen AND (FrontWindow() EQ graphWind) AND NOT(scrnColor) AND grphOnScrn AND appActive)
	{
		HideCursor();
		grphPnts = graphPoints;
		xCnt = numX;
		while (xCnt--)
		{
//			if (CmndPeriod())
//			{
//				ShowCursor();
//				return;
//			}
			MoveTo(*grphPnts++);
			yCnt = numY;
			while (--yCnt)
				MLineTo(*grphPnts++);
		}

		for (yCnt = 0; yCnt < numY; yCnt++)
		{
			MoveTo(graphPoints[yCnt]);
//			if (CmndPeriod())
//			{
//				ShowCursor();
//				return;
//			}
			grphPnts = graphPoints + (numY + yCnt);
			for (xCnt = 1; xCnt < numX; xCnt++)
			{
				MLineTo(*grphPnts);
				grphPnts += numY;
			}
		}
		ShowCursor();
	}
	else
	{
		grphPnts = graphPoints;
		xCnt = numX;
		while (xCnt--)
		{
			if (CmndPeriod())
				return;
			MoveTo(*grphPnts++);
			yCnt = numY;
			while (--yCnt)
				LineTo(*grphPnts++);
		}
		
		for (yCnt = 0; yCnt < numY; yCnt++)
		{
			MoveTo(graphPoints[yCnt]);
			if (CmndPeriod())
				return;
			grphPnts = graphPoints + (numY + yCnt);
			for (xCnt = 1; xCnt < numX; xCnt++)
			{
				LineTo(*grphPnts);
				grphPnts += numY;
			}
		}
	}
}

static
DrawAxes()
{
	PenSize(2, 2);
	MoveTo(pass(Origin));
	LineTo(pass(XaxisPt));
	MoveTo(pass(Origin));
	LineTo(pass(YaxisPt));
	MoveTo(pass(Origin));
	LineTo(pass(ZaxisPt));
	PenSize(1, 1);
}

static
DrawHiddenGraph()
{
	PolyHandle	thePoly;

	if (xAxPos) {
		if (yAxPos)
			HideIt(0, 0, 1, 1, numX-1, numY-1);
		else
			HideIt(0, numY-2, 1, -1, numX-1, -1);
	}
	else {
		if (yAxPos)
			HideIt(numX-2, 0, -1, 1, -1, numY-1);
		else
			HideIt(numX-2, numY-2, -1, -1, -1, -1);
	}
}

static
HideIt(xStrt, yStrt, xStep, yStep, xEnd, yEnd)
int			xStrt, yStrt;
register	int	xStep, yStep;
int			xEnd, yEnd;
{
register	int	xCnt, yCnt;
register	long	offSet;
	PolyHandle 	thePoly;
	Boolean		CmndPeriod();

	EraseRect(&thePort->portRect);

	for (xCnt = xStrt; xCnt NEQ xEnd; xCnt += xStep)
	{
		for (yCnt = yStrt; yCnt NEQ yEnd; yCnt += yStep)
		{
			thePoly = OpenPoly();
			offSet = (long)xCnt * numY + (long)yCnt;
			MoveTo(graphPoints[offSet]);
			LineTo(graphPoints[(long)xCnt * numY + (long)(yCnt+1)]);
			LineTo(graphPoints[(long)(xCnt+1) * numY + (long)(yCnt+1)]);
			LineTo(graphPoints[(long)(xCnt+1) * numY + (long)yCnt]);
			LineTo(graphPoints[offSet]);
			ClosePoly();
			SetGray(gridFace[offSet]);
			PaintPoly(thePoly);
			KillPoly(thePoly);
		}
		if (CmndPeriod())
			break;  /* for */
	}
	SetBlack();
}

static
SetBlack()
{
	RGBColor	theColor;

	if (scrnColor)
	{
		theColor.red = theColor.blue = theColor.green = 0;
		RGBForeColor(&theColor);
	}
	else
		PenPat(black);
}

static
FitRect(theRect, thePoint)
register	Rect	*theRect;
register	Point	*thePoint;
{
	if (thePoint->h < theRect->left)
		theRect->left = thePoint->h;
	else if (thePoint->h > theRect->right)
		theRect->right = thePoint->h;

	if (thePoint->v < theRect->top)
		theRect->top = thePoint->v;
	else if (thePoint->v > theRect->bottom)
		theRect->bottom = thePoint->v;
}


static
DoActivate()
{
	WindowPtr whichWindow;

	whichWindow = (WindowPtr) (theEvent.message);
	if (theEvent.modifiers & activeFlag)
	{
		ReadDeskScrap();
		SetPort(whichWindow);
	}
	else
		WriteDeskScrap();

	if (whichWindow EQ graphWind)
		PutGraphGrow();
}

static
DoSuspndRsm(message)
long	message;
{
	if (HIBYTE(message) NEQ 1)
		return;	/* not a suspend/resume */

	if (IS_SUSPND(message))
	{
		if (LOAD_CLIP(message))
			WriteDeskScrap();
		appActive = FALSE;
	}
	else	/* resume event */
	{
		if (LOAD_CLIP(message))	/* read back the clipboard? */
			ReadDeskScrap();
		appActive = TRUE;
	}
}

ShowFunctDial()
{
	if (functDOpen)
		SelectWindow(functDial);
	else
	{
		if (graphType EQ FUNCT_GTYPE)
			functDial = GetNewDialog(DIAL_FUNCT, &funcDialRecord, (DialogPtr)-1L);
		else if (graphType EQ CYLIND_GTYPE)
			functDial = GetNewDialog(DIAL_CYLIND, &funcDialRecord, (DialogPtr)-1L);
		else if (graphType EQ DERIV_GTYPE)
			functDial = GetNewDialog(DIAL_DERIV, &funcDialRecord, (DialogPtr)-1L);
		functDOpen = TRUE;
	}
}

ShowSpecs()
{
	if (specDOpen)
		SelectWindow(specDial);
	else {
		switch	(graphType) {
		case FUNCT_GTYPE:
			specDial = GetNewDialog(DIAL_SCALE_XY, &sdialRecord, (DialogPtr)-1L);
			specDOpen = TRUE;
			break;
		case CYLIND_GTYPE:
			specDial = GetNewDialog(DIAL_SCALE_RP, &sdialRecord, (DialogPtr)-1L);
			specDOpen = TRUE;
			break;
		case DERIV_GTYPE:
			specDial = GetNewDialog(DIAL_SCALE_XY, &sdialRecord, (DialogPtr)-1L);
			specDOpen = TRUE;
			break;
		case FDATA_GTYPE:
			break;
		}
	}
}

ShowLimits()
{
	if (limtDOpen)
		SelectWindow(limtDial);
	else {
		limtDial = GetNewDialog(DIAL_LIMT, &lmdialRecord, (DialogPtr)-1L);
		limtDOpen = TRUE;
	}
}

static
int
PZArcShow(theRect)
Rect	*theRect;
{
	Point	currMouseLoc;
	int		degrs, oldDegrs;

	GetMouse(&currMouseLoc);
	degrs = (180.0 / (Real)(theRect->right - theRect->left))
			* currMouseLoc.h;
	oldDegrs = degrs;
	InvertArc(theRect, 270 - degrs, degrs);

	while (Button())
	{
		GetMouse(&currMouseLoc);
		degrs = (180.0 / (Real)(theRect->right - theRect->left))
				* currMouseLoc.h;
		if (degrs < 0)
			degrs = 0;
		else if (degrs > 180)
			degrs = 180;
		if (degrs NEQ oldDegrs)
		{
			InvertArc(theRect, 270 - degrs, degrs - oldDegrs);
			oldDegrs = degrs;
		}
	}
	InvertArc(theRect, 270 - degrs, degrs);
	return(degrs);
}

static
int
NZArcShow(theRect)
Rect	*theRect;
{
	Point	currMouseLoc;
	int		degrs, oldDegrs, widthRect;

	widthRect = theRect->right - theRect->left;
	GetMouse(&currMouseLoc);
	degrs = (180.0 / (Real)(widthRect))
			* (widthRect - currMouseLoc.h);
	oldDegrs = degrs;
	InvertArc(theRect, 90, degrs);

	while (Button())
	{
		GetMouse(&currMouseLoc);
		degrs = (180.0 / (Real)(widthRect))
				* (widthRect - currMouseLoc.h);
		if (degrs < 0)
			degrs = 0;
		else if (degrs > 180)
			degrs = 180;
		if (degrs NEQ oldDegrs)
		{
			if (degrs < oldDegrs)
				InvertArc(theRect, degrs + 90, oldDegrs - degrs);
			else
				InvertArc(theRect, oldDegrs + 90, degrs - oldDegrs);
			oldDegrs = degrs;
		}
	}

	InvertArc(theRect, 90, degrs);
	return(degrs);
}


static
int
NYArcShow(theRect)
Rect	*theRect;
{
	Point	currMouseLoc;
	int		degrs, oldDegrs, heightRect;

	heightRect = theRect->bottom - theRect->top;
	GetMouse(&currMouseLoc);
	degrs = (180.0 / (Real)(heightRect))
			* currMouseLoc.v;
	oldDegrs = degrs;
	InvertArc(theRect, 0, degrs);

	while (Button())
	{
		GetMouse(&currMouseLoc);
		degrs = (180.0 / (Real)(heightRect))
				* currMouseLoc.v;
		if (degrs < 0)
			degrs = 0;
		else if (degrs > 180)
			degrs = 180;
		if (degrs NEQ oldDegrs)
		{
			if (degrs < oldDegrs)
				InvertArc(theRect, degrs, oldDegrs - degrs);
			else
				InvertArc(theRect, oldDegrs, degrs - oldDegrs);
			oldDegrs = degrs;
		}
	}
	InvertArc(theRect, 0, degrs);

	return(degrs);
}

static
int
PYArcShow(theRect)
Rect	*theRect;
{
	Point	currMouseLoc;
	int		degrs, oldDegrs, heightRect;

	heightRect = theRect->bottom - theRect->top;
	GetMouse(&currMouseLoc);
	degrs = (180.0 / (Real)(heightRect))
			* (heightRect - currMouseLoc.v);
	oldDegrs = degrs;
	InvertArc(theRect, 180 - degrs, degrs);

	while (Button())
	{
		GetMouse(&currMouseLoc);
		degrs = (180.0 / (Real)(heightRect))
				* (heightRect - currMouseLoc.v);
		if (degrs < 0)
			degrs = 0;
		else if (degrs > 180)
			degrs = 180;
		if (degrs NEQ oldDegrs)
		{
			if (degrs < oldDegrs)
				InvertArc(theRect, 180 - oldDegrs, oldDegrs - degrs);
			else
				InvertArc(theRect, 180 - degrs, degrs - oldDegrs);
			oldDegrs = degrs;
		}
	}
	InvertArc(theRect, 180 - degrs, degrs);

	return(degrs);
}

static
int
XArcShow(theRect)
Rect	*theRect;
{
	int		degrs, oldDegrs;
	double	angle, MouseAngle();
	Point	currMsPos, oldMsPos, presPos, oldPos;
	int		startAngle, oldAngle, presAngle, rotAngle;

	PenMode(patXor);
	PenSize(2, 2);
	GetMouse(&oldMsPos);
	oldPos = drawWCntr;

	do
	{
		GetMouse(&currMsPos);
		if ((currMsPos.h NEQ oldMsPos.h) OR (currMsPos.v NEQ oldMsPos.v))
		{
			angle = MouseAngle(currMsPos);
			presPos.h = (int)(drawWCntr.v * sin(angle)) + drawWCntr.h;
			presPos.v = -(int)(drawWCntr.v * cos(angle)) + drawWCntr.v;
			angle = MouseAngle(currMsPos);
			presPos.h = (int)(drawWCntr.v * sin(angle)) + drawWCntr.h;
			presPos.v = -(int)(drawWCntr.v * cos(angle)) + drawWCntr.v;
			ReDrwLine(drawWCntr.v, drawWCntr.h, presPos, oldPos);
			oldPos = presPos;
			oldMsPos = currMsPos;
		}
	} while (Button());
	startAngle = round(angle * RADtoDEG);

	oldAngle = startAngle;
	do
	{
		GetMouse(&currMsPos);
		if ((currMsPos.h NEQ oldMsPos.h) OR (currMsPos.v NEQ oldMsPos.v))
		{
			angle = MouseAngle(currMsPos) * RADtoDEG;
			presAngle = round(angle);
			rotAngle = RelativAngle(startAngle, presAngle);

			InvertArc(theRect, startAngle + RelativAngle(startAngle, oldAngle),
					RelativAngle(startAngle, presAngle)
					- RelativAngle(startAngle, oldAngle));

			oldAngle = presAngle;
			oldMsPos = currMsPos;
		}
	} while (NOT(Button()));

	if (presAngle - startAngle > 0)
		return(presAngle - startAngle);
	else
		return(360 + presAngle - startAngle);
	PenMode(patCopy);
	PenSize(1, 1);
}

/*** Update the position of a line ***/
void
ReDrwLine(strt, presPos, oldPos)
Point		strt, presPos, oldPos;
{
	MoveTo(strt);
	LineTo(oldPos);
	MoveTo(strt);
	LineTo(presPos);
}


double
MouseAngle(currMsLoc)
Point	currMsLoc;
{
	Point	msCntrLoc;
	double	angle;

	msCntrLoc.h = currMsLoc.h - drawWCntr.h;
	msCntrLoc.v = currMsLoc.v - drawWCntr.v;
	angle = atan((double)msCntrLoc.h / msCntrLoc.v);
	angle = Abs(angle);

	if (msCntrLoc.h > 0)
		if (msCntrLoc.v > 0)
			angle = PI - angle;
		else
		{}
	else
	{
		if (msCntrLoc.v > 0)
			angle -= PI;
		else
			angle = -angle;
	}

	return(angle);
}


int
RelativAngle(startAngle, presAngle)
int	startAngle, presAngle;
{
	int	relAngle;

	if (presAngle - startAngle > 0)
		relAngle = (presAngle - startAngle);
	else
		relAngle = (360 + presAngle - startAngle);
	if (relAngle > 180)
		relAngle -= 360;
	return(relAngle);
}

My_printf(shwStr)
char	*shwStr;
{
	char	pbuffer[256];
	GrafPtr	savePort;
static	Rect	dispRect = {21, 320, 40, 510};
	
	GetPort(&savePort);
	SetPort(windManPort);
		ClipRect(&dispRect);
		EraseRect(&dispRect);
		MoveTo(330, 36);
		strcpy(pbuffer, shwStr);
		ctop(pbuffer);
		DrawString(pbuffer);
	SetPort(savePort);
}

Wait()
{
	EventRecord	msEvent;
	
	while(!Button());
	while(Button());
	GetNextEvent(mDownMask, &msEvent);
}

static	GrafPtr	_oldPort;

SavePort()
{
	GetPort(&_oldPort);
}

RestorePort()
{
	SetPort(_oldPort);
}


static	DialogPtr		meterDlg;
static	Real	meterAngle, lstDrwnAngle, meterDelta;
static	Rect	meterRect = { 10, 10, 93, 93 };

ShowMeter(elements)
long	elements;
{
static	DialogRecord	meterRecord;	/* storage for the dialog */

	if (inMultiFndr)
	{
		SavePort();
		SelectWindow(graphWind);
		SetPort(graphWind);
		EraseRect(&meterRect);
		FrameOval(&meterRect);
		RestorePort();
	}
	else
	{
		meterDlg = GetNewDialog(DIAL_METER, &meterRecord, (DialogPtr)-1L);
		DrawDialog(meterDlg);
		SavePort();
		SetPort(meterDlg);
		FrameOval(&meterRect);
	}

	meterDelta = 360.0 / elements;
	meterAngle = lstDrwnAngle = 0.0;
}

DisplayMeter()
{
	GrafPtr	svPort;

	meterAngle += meterDelta;
	if (meterAngle > (lstDrwnAngle + 3))
	{
		lstDrwnAngle = meterAngle;
		if (inMultiFndr)
		{
			GetPort(&svPort);
			SetPort(graphWind);
			FillArc(&meterRect, 0, (int)meterAngle, black);
			SetPort(svPort);
		}
		else
			FillArc(&meterRect, 0, (int)meterAngle, black);
	}
}

RemoveMeter()
{
	register	int	i = 4;

	if (NOT(inMultiFndr))
	{
		CloseDialog(meterDlg);
	
		noRedrawGrph = TRUE;
		RestorePort();
		while (i--)
			if (GetNextEvent(updateMask, &theEvent))
				DoEvent();
		noRedrawGrph = FALSE;
	}
}



/***  read in deskscrap (clipboard)  ***/
ReadDeskScrap()
{
	long	scrapLen, scrapOffset;

	if (scrapCompare NEQ ScrapInfo.scrapCount)
	{
		scrapLen = GetScrap(NIL, 'TEXT', &scrapOffset);
		if (scrapLen >= 0)
			if (TEFromScrap() < 0)	/* ROM */
				scrapLen = 0;	/* was an error */
		if (NOT(scrapLen))
		{
			TESetScrapLen(0);	/* ROM */
			isScrapTE = FALSE;
		}
		else
			isScrapTE = TRUE;

		scrapCompare = ScrapInfo.scrapCount;
	}
	SetEditMenu();
}

/***  write out deskscrap (clipboard)  ***/
WriteDeskScrap()
{
	if (scrapDirty AND isScrapTE)
	{
		scrapCompare = ZeroScrap();
		TEToScrap();	/* ROM */
		scrapDirty = FALSE;
	}
	SetEditMenu();
}

SetPlotMenu()
{
	if (hideSurface)
		DisableItem(plotMenu, PLOT_DRWSCRN);
	else
		EnableItem(plotMenu, PLOT_DRWSCRN);
}

/***  enable or gray out approriate items in the menu  ***/
SetEditMenu()
{
	WindowPtr frontW;
	Boolean		IsDialog(), IsList();

	frontW = FrontWindow();
	if (frontW EQ graphWind)
	{
		DisableItem(editMenu, EDIT_UNDO);	/* ROM */
		DisableItem(editMenu, EDIT_CUT);
		EnableItem(editMenu, EDIT_COPY);
		DisableItem(editMenu, EDIT_PASTE);
		DisableItem(editMenu, EDIT_CLEAR);
	}
	else if (IsDialog(frontW))
	{
		DisableItem(editMenu, EDIT_UNDO);	/* ROM */
		EnableItem(editMenu, EDIT_CUT);
		EnableItem(editMenu, EDIT_COPY);
		if (isScrapTE)
			EnableItem(editMenu, EDIT_PASTE);
		else
			DisableItem(editMenu, EDIT_PASTE);
		EnableItem(editMenu, EDIT_CLEAR);
	}
	else	/* menu for DA */
	{
		EnableItem(editMenu, EDIT_UNDO);
		EnableItem(editMenu, EDIT_CUT);
		EnableItem(editMenu, EDIT_COPY);
		EnableItem(editMenu, EDIT_PASTE);
		EnableItem(editMenu, EDIT_CLEAR);
	}
}

#define	KEYMAP	((long *)0x174)

int
ShiftDown()
{
	return(KEYMAP[1] & 1);
}

int
OptionDown()
{
	return(KEYMAP[1] & 4);
}

int
CapsDown()
{
	return(KEYMAP[1] & 2);
}


pascal void
RMoveTo(a)
long	a;
{
	asm{
		move.l  a,-(a7)
		move.l  MoveToAddr,a0
		jsr     (a0)
	}
}

pascal void
RLineTo(a)
long	a;
{
	asm{
		move.l  a,-(a7)
		move.l  LineToAddr,a0
		jsr     (a0)
	}
}

Clock()
{
	CursHandle		theCursH;

	theCursH = GetCursor(4);
	SetCursor(*theCursH);
}

GenralAlert(mesg)
char	*mesg;
{
	ParamText(mesg, "", "", "");
	InitCursor();
	Alert(ALERT_GENRAL, NIL);
}



/*********************************************/
/*****	MultiFinder "multitasking" stuff *****/


Boolean
CmndPeriod()
{
	int		evntMask;
	EventRecord	keyEvent;
	Boolean		CmndEvent();
static	long	lastTm = 0xf0000000;

	if (appActive)
		if ((Ticks - lastTm) < 15)
			return(FALSE);
	lastTm = Ticks;

	if (inMultiFndr)
	{
		evntMask = everyEvent - updateMask;
		if (WaitNextEvent(evntMask, &keyEvent, sleepPeriod, NIL))
		{
			if (keyEvent.what EQ resumEvt)
				DoSuspndRsm(keyEvent.message);
			else if (CmndEvent(&keyEvent))
				return(TRUE);
		}
	}
	else
	{
		SystemTask();
		if (EventAvail(keyDownMask, &keyEvent))
			if (CmndEvent(&keyEvent))
				return(TRUE);
	}
	return(FALSE);	
}

static
Boolean
CmndEvent(chkEvent)
EventRecord		*chkEvent;
{
	if (chkEvent->what EQ keyDown)
		if ((chkEvent->message & charCodeMask) EQ '.')
			if (chkEvent->modifiers & cmdKey)
				return(TRUE);
	return(FALSE);
}






SetRotateHandles()
{
	int		halfWid, width;
	Rect	drawRect;

	drawRect = graphWind->portRect;
	drawWCntr.h = DIV_2(drawRect.right - drawRect.left) + drawRect.left;
	drawWCntr.v = DIV_2(drawRect.bottom - drawRect.top) + drawRect.top;

	hndRct[0].top = drawRect.top - 1;
	hndRct[0].left = drawWCntr.h - 7;
	hndRct[0].bottom = hndRct[0].top + 13;
	hndRct[0].right = drawWCntr.h + 7;

	hndRct[1].top = drawRect.bottom - 12;
	hndRct[1].left = hndRct[0].left;
	hndRct[1].bottom = drawRect.bottom + 1;
	hndRct[1].right = hndRct[0].right;

	hndRct[2].top = drawWCntr.v - 7;
	hndRct[2].bottom = drawWCntr.v + 7;
	hndRct[2].left = -1;
	hndRct[2].right = 12;

	hndRct[3].top = hndRct[2].top;
	hndRct[3].left = drawRect.right - 12;
	hndRct[3].bottom = hndRct[2].bottom;
	hndRct[3].right = drawRect.right + 1;

	hndRct[4].top = drawRect.top - 1;
	hndRct[4].bottom = hndRct[4].top + 13;
	hndRct[4].left = -1;
	hndRct[4].right = 12;

	width = (MIN(drawWCntr.h, drawWCntr.v) * 100) / 152;
	halfWid = width / 2;

	vertRect.top = drawRect.top;
	vertRect.left = drawWCntr.h - halfWid;
	vertRect.bottom = drawRect.bottom;
	vertRect.right = drawWCntr.h + halfWid;

	horizRect.top = drawWCntr.v - halfWid;
	horizRect.left = 0;
	horizRect.bottom = drawWCntr.v + halfWid;
	horizRect.right = drawRect.right;

	centrRect.top = drawWCntr.v - width;
	centrRect.left = drawWCntr.h - width;
	centrRect.bottom = drawWCntr.v + width;
	centrRect.right = drawWCntr.h + width;
}

DrawRotHndls()
{
	int		i;

	PenMode(patXor);
	for (i = 0; i < 5; i++)
		FrameRect(hndRct + i);
	PenMode(patCopy);
}

int
WhatQuad(pntPtr)
Point	*pntPtr;
{
	int		i;

	for (i = 0; i < 5; i++)
		if (PtInRect(*pntPtr, hndRct + i))
			return(i);
	return(-1);
}
