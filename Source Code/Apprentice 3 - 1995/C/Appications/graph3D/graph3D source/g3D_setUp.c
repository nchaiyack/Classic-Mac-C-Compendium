/*
	Copyright '89	Christopher Moll
	all rights reserved
*/


#include	"graph3D.h"
#ifdef	_LSC3_
#	include <ColorToolbox.h>
#	include <WindowMgr.h>
#	include <EventMgr.h>
#	include <MenuMgr.h>
#	include <FileMgr.h>
#	include <TextEdit.h>
#	include <ScrapMgr.h>
#endif



/* windows */
extern	GrafPtr			windManPort;

extern	WindowPtr		graphWind;
WindowRecord	fwindRecord;
WindowRecord	gwindRecord;


extern	Point		*graphPoints;	/* same elements as funcResults */

extern	Real	startX, startY;
extern	Real	endX, endY;
extern	Real	deltaX, deltaY;
extern	int		numX, numY;

extern	Boolean		vectrsCurrent;

extern	Vector		maxVect, minVect;
extern	Real		rotMatrx[3][3];
extern	Boolean		functCurrent;

extern	Real		*funcResults;
extern	Vector		*scalVectResults;



/* menus */
extern	MenuHandle		appleMenu, fileMenu, editMenu, plotMenu,
						gTypeMenu, windwMenu;

extern	int			scrapCompare;	/* status of the deskscrap (clipboard) */
extern	Boolean		scrapDirty;		/* deskscrap has been changed */

extern	Boolean			inMultiFndr;
extern	Boolean			colorQD, scrnColor;

extern	PatternList	*greyList;
extern	int		numGrays;

extern	long    MoveToAddr, LineToAddr, FixMulAddr;

OSErr	SendPort(...);


DoSetup()
{
	Init();
	OpenSerPort();
	AddrsSet();

	scrapCompare = ScrapInfo.scrapCount + 1;
	scrapDirty = FALSE;
	ReadDeskScrap();

	InitColor();
	MakeWindows();
	MakeMenus();
	InitPts();
	CheckMultiFndr();
}

static
InitColor()
{
	PattListHand	greyHndl;
	SysEnvRec		theWorld;
	GDHandle		grefDev;
	int				depth;
	Boolean		ScrnHasColor();

	greyHndl = (PattListHand)GetResource('PAT#', 256);
	if (greyHndl)
	{
		MoveHHi(greyHndl);
		HLock(greyHndl);
		greyList = *greyHndl;
	}

	SysEnvirons(1, &theWorld);
#ifdef	_MC68881_
	if (theWorld.processor < env68020)
	{
		Alert(ALERT_020, NIL);
		ExitToShell();
	}
	else if (NOT(theWorld.hasFPU))
	{
		Alert(ALERT_881, NIL);
		ExitToShell();
	}
#endif
	colorQD = theWorld.hasColorQD;
	scrnColor = ScrnHasColor();
}

Boolean
ScrnHasColor()
{
	int				depth;
	GDHandle		grefDev;

	if (colorQD)
	{
		grefDev = GetGDevice();
		depth = (**((**grefDev).gdPMap)).pixelSize;
		return(depth > 1);
	}
	else
		return(FALSE);
}

static
CheckMultiFndr()
{
	long	WaitAdrs, unimpAdrs;

	WaitAdrs = GetTrapAddress(0x60);
	unimpAdrs = GetTrapAddress(0x9f);
	if (WaitAdrs EQ unimpAdrs)
		inMultiFndr = FALSE;
	else
		inMultiFndr = TRUE;
}


Init()
{
	InitGraf(&thePort);
	InitFonts();
	InitWindows();
	InitMenus();
	TEInit();
	InitDialogs(0L);
	InitCursor();

	FlushEvents(everyEvent, 0);

	GetWMgrPort(&windManPort);	/* get whole screen port that window mgr
				   uses */
	SetPort(windManPort);	/* and start off with it */

	SetEventMask(everyEvent);
}

MakeWindows()
{
	Rect editRect;

	ShowSpecs();
	if (colorQD)
		graphWind = GetNewCWindow(WIND_GRAPH, &gwindRecord, (WindowPtr)-1L);
	else
		graphWind = GetNewWindow(WIND_GRAPH, &gwindRecord, (WindowPtr)-1L);
	ShowWindow(graphWind);
	SetPort(graphWind);
	SetGraphRect();
	ShowLimits();
	ShowFunctDial();
}

MakeMenus()
{
	appleMenu = GetMenu(MENU_APPLE);
	AddResMenu(appleMenu, 'DRVR');
	InsertMenu(appleMenu, 0);

	fileMenu = GetMenu(MENU_FILE);
	InsertMenu(fileMenu, 0);

	editMenu = GetMenu(MENU_EDIT);
	InsertMenu(editMenu, 0);

	plotMenu = GetMenu(MENU_PLOT);
	InsertMenu(plotMenu, 0);

	gTypeMenu = GetMenu(MENU_GTYP);
	InsertMenu(gTypeMenu, 0);
	CheckItem(gTypeMenu, 1, TRUE);

	windwMenu = GetMenu(MENU_WIND);
	InsertMenu(windwMenu, 0);
	
	CheckItem(plotMenu, PLOT_WIREFRAM, TRUE);

	DrawMenuBar();
}

InitPts()
{
	register	long	i, numElems;
	register	Real	*resltsPtr;

	MkRotMat(0.0, -.3, -.3, rotMatrx);

	GetSpecDCont();
	GetLimtDCont();

	numX = numY = 20;

	deltaX = (endX - startX) / numX;
	deltaY = (endY - startY) / numY;

	funcResults = (Real *)NewPtr((long)sizeof(Real) * (long)numX * numY);
	if (MemErr)
		goto Fail;
	scalVectResults = (Vector *)NewPtr((long)sizeof(Vector) * (long)numX * numY);
	if (MemErr)
		goto Fail;
	graphPoints = (Point *)NewPtr((long)sizeof(Point) * (long)numX * numY);
	if (MemErr)
		goto Fail;

	resltsPtr = funcResults;
	numElems = (long)numX * numY;
	for (i = 0; i < numElems; i++)
		*resltsPtr++ = 0.0;

	maxVect.x = 0.0;
	maxVect.y = 0.0;
	maxVect.z = 0.0;
	minVect.x = 0.0;
	minVect.y = 0.0;
	minVect.z = 0.0;

	vectrsCurrent = FALSE;
	functCurrent = TRUE;
	return;

Fail:
	GenralAlert("\PToo little memory to run; returning to Finder");
	ExitToShell();
}



static	long	rtrnAdrs;
static	char	buffer[256];

tprintf()
{
	asm	{
		move.l	(sp)+,rtrnAdrs
		}
	sprintf(buffer);
	SendPort(buffer, strlen(buffer));
	asm	{
		move.l	rtrnAdrs,a0
		jmp		(a0)
		}
}

scprintf()
{
	asm	{
		move.l	(sp)+,rtrnAdrs
		}
	sprintf(buffer);
	My_printf(buffer);
	asm	{
		move.l	rtrnAdrs,a0
		jmp		(a0)
		}
}

/*flprintf()
{
	asm	{
		move.l	(sp)+,rtrnAdrs
		}
	sprintf(buffer);
	AppendTempFile(buffer);
	asm	{
		move.l	rtrnAdrs,a0
		jmp		(a0)
		}
}*/

static
_My_printf(shwStr)
char	*shwStr;
{
	char	buffer[256];
static	Rect	dispRect = {67, 54, 87, 166};

	ClipRect(&dispRect);
	EraseRect(&dispRect);
	MoveTo(dispRect.left + 10, dispRect.bottom - 4);
	strcpy(buffer, shwStr);
	ctop(buffer);
	DrawString(buffer);
}

static	IOParam		portParam;

/*** open printer port ***/
OpenSerPort()
{
	portParam.ioCompletion = 0L;
	portParam.ioNamePtr = (StringPtr)("\p.BOut");
	portParam.ioRefNum = 0;
	portParam.ioPermssn = fsWrPerm;
	portParam.ioMisc = 0L;

	PBOpen(&portParam, FALSE);
	if (portParam.ioResult);
		/*if (portParam.ioResult EQ -98)
			GenralAlert("\PAppleTalk connected; won't be able to print");
		
		else
			GenralAlert("\PError opening printer port");*/
}

static
OSErr
SendPort(buffer, length)
char	*buffer;
int		length;
{
	portParam.ioBuffer = buffer;
	portParam.ioReqCount = length;
	portParam.ioPosMode = fsAtMark;

	PBWrite(&portParam, FALSE);
	buffer[portParam.ioActCount] = '\0';
	return(portParam.ioResult);
}




_SetGray(theColor, greyShd)
register	RGBColor	*theColor;
register	int		greyShd;
{
	int		setColor;

	setColor = greyShd << 8;

	theColor->red = setColor;
	theColor->green = setColor;
	theColor->blue = setColor;
}

SetGray(greyShd)
register	int		greyShd;
{
	if (scrnColor)
	{
		if (greyShd EQ 0)
			PmForeColor(1);
		else if (greyShd EQ 255)
			PmForeColor(0);
		else
			PmForeColor(256 - greyShd);
	}
	else
		PenPat(greyList->patList + (63 - greyShd));
}


__SetGray(greyShd)
register	int		greyShd;
{
	RGBColor	theColor;
register	int		setColor;

	if (greyShd < 0)
		setColor = 0;
	else if (greyShd < 12)
		setColor = greyShd * 5462;
	else
		setColor = 0xffff;

	theColor.red = setColor;
	theColor.green = setColor;
	theColor.blue = setColor;
	RGBForeColor(&theColor);
}

static
AddrsSet()
{
	MoveToAddr = GetTrapAddress(MoveToTrapNum);
	LineToAddr = GetTrapAddress(LineToTrapNum);
	FixMulAddr = GetTrapAddress(FixMulTrapNum);
}

