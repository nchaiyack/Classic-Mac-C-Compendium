#include "queen meat.h"
#include "nim message.h"
#include "nim globals.h"
#include "nim grid.h"
#include "nim endgame.h"
#include "shadow box.h"
#include "sound layer.h"
#include "window layer.h"

#define	kQueenSafePoints	10
#define kKingSafePoints		144
#define kSuperqueenSafePoints	10	/* this is wild guessing */
#define kSuperkingSafePoints	144	/* this is eduated guessing */
#define kMaxSafePoints		kKingSafePoints		/* may change later */

static	short		gThisGameNumRows, gThisGameNumColumns, gThisGameStart;

static	Point		gQueenSafeArray[kQueenSafePoints]=
						{ {0,0}, {1,2}, {3,5}, {4,7}, {6,10},
						  {8,13}, {9,15}, {11,18}, {12,20}, {14,23} };
static	Point		gQueenMisereSafeArray[kQueenSafePoints]=
						{ {0,1}, {2,2}, {3,5}, {4,7}, {6,10},
						  {8,13}, {9,15}, {11,18}, {12,20}, {14,23} };
static	Point		gSuperqueenSafeArray[kSuperqueenSafePoints]=
						{ {0,0}, {1,3}, {2,6}, {4,5}, {7,10},
						  {8,13}, {9,16}, {11,19}, {12,18}, {14,23} };
static	Point		gSuperqueenMisereSafeArray[kSuperqueenSafePoints]=
						{ {0,1}, {2,4}, {3,3}, {5,8}, {6,11},
						  {7,14}, {9,13}, {10,16}, {12,20}, {0,1} };

static	short		gNumSafeArray[4]={ kQueenSafePoints, kKingSafePoints, kSuperqueenSafePoints,
										kSuperkingSafePoints };

static	Point		gSafeArray[4][kMaxSafePoints];
static	Point		gMisereSafeArray[4][kMaxSafePoints];

static	Point		gThePosition;
static	Rect		gAvailableRect;
static	Boolean		gInitted=FALSE;

static	Point DoCornerAskPosition(WindowRef theWindow);
static	GameError ValidMove(Point fromPoint, Point toPoint);
static	void CheckCornerEndGame(WindowRef theWindow);
static	void CornerRandomMove(Point *yourFinalAnswer);
static	void ShowCornerMove(WindowRef theWindow, Point magic);

void InitCorner(void)
{
	short			i, j;
	short			kingGap;
	
	if (gInitted)
		return;
	
	gInitted=TRUE;
	for (i=0; i<kQueenSafePoints; i++)
	{
		gSafeArray[0][i]=gQueenSafeArray[i];
		gMisereSafeArray[0][i]=gQueenMisereSafeArray[i];
	}
	
	for (i=0; i<kKingSafePoints; i++)
	{
		gMisereSafeArray[1][i].h=gSafeArray[1][i].h=gMisereSafeArray[3][i].h=gSafeArray[3][i].h=(i%12)*2;
		gMisereSafeArray[1][i].v=gSafeArray[1][i].v=gMisereSafeArray[3][i].v=gSafeArray[3][i].v=(i/12)*2;
	}
	kingGap=1;
	for (i=0; i<12; i++)
	{
		gMisereSafeArray[1][i].h++;
		if (i<11)
			gMisereSafeArray[1][(i+1)*12].h++;
		for (j=0; j<12; j++)
			gMisereSafeArray[3][i*12+j].h+=kingGap;
		kingGap+=2;
	}
	for (i=0; i<kSuperqueenSafePoints; i++)
	{
		gSafeArray[2][i]=gSuperqueenSafeArray[i];
		gMisereSafeArray[2][i]=gSuperqueenMisereSafeArray[i];
	}
}

void InitCornerOneGame(WindowRef theWindow)
{
	#pragma unused(theWindow)
	
	short			r;
	
	gThisGameNumRows=gCornerInfo[TheCornerIndex].numRows;
	gThisGameNumColumns=gCornerInfo[TheCornerIndex].numColumns;
	gThisGameStart=gCornerInfo[TheCornerIndex].start;
	
	gThePosition.h=gThisGameNumColumns-1;
	gThePosition.v=gThisGameNumRows-1;
	switch (gThisGameStart)
	{
		case kCornerStartTopRight:
			break;
		case kCornerStartRandom:
			r=(Random()&0x7fff)%2;
			if (r==0)
			{
				r=(Random()&0x7fff)%(gThisGameNumColumns/2)+gThisGameNumColumns/2;
				gThePosition.h=r;
				gThePosition.v=gThisGameNumRows-1;
			}
			else
			{
				r=(Random()&0x7fff)%(gThisGameNumRows/2)+gThisGameNumRows/2;
				gThePosition.v=r;
				gThePosition.h=gThisGameNumColumns-1;
			}
			break;
	}
}

void CornerDrawWindow(WindowRef theWindow, short theDepth)
{
	Rect			boundsRect;
	
	SetRect(&boundsRect, 0, 0, gThisGameNumColumns*kGridColumnWidth+28,
		gThisGameNumRows*kGridRowHeight+28);
	OffsetRect(&boundsRect, (theWindow->portRect.right-theWindow->portRect.left-boundsRect.right)/2,
		(theWindow->portRect.bottom-theWindow->portRect.top-boundsRect.bottom)/2);
	Draw3DShadowBox(&boundsRect, theDepth, &gAvailableRect);
	DrawGrid(FALSE, theDepth, &gAvailableRect, gThisGameNumRows,
		gThisGameNumColumns);
	DrawGridPosition(FALSE, &gAvailableRect, gThePosition, gPlayer, gThisGameNumRows);
}

void CornerClick(WindowRef theWindow, Point thePoint)
{
	Point			pos;
	GameError		e;
	
	pos=PositionFromPoint(FALSE, thePoint, &gAvailableRect, gThisGameNumRows);
	if ((e=ValidMove(gThePosition, pos))==eNoErr)
	{
		gPlayer=!gPlayer;
		ShowCornerMove(theWindow, pos);
		CheckCornerEndGame(theWindow);
	}
	else
	{
		DisplayMessage(e);
	}
}

static	GameError ValidMove(Point fromPoint, Point toPoint)
{
	if ((toPoint.h>fromPoint.h) || (toPoint.v>fromPoint.v))
		return eOnlyMoveLeftDown;
	if ((toPoint.h==fromPoint.h) && (toPoint.v==fromPoint.v))
		return eSpotTaken;
	if ((toPoint.h<0) || (toPoint.v<0))
		return eOffBoard;
	
	switch (gGameType)
	{
		case kCornerTheQueen:
			if (toPoint.h==fromPoint.h)
				return eNoErr;
			if (toPoint.v==fromPoint.v)
				return eNoErr;
			if ((fromPoint.v-toPoint.v)==(fromPoint.h-toPoint.h))
				return eNoErr;
			return eMustMoveLikeQueen;
			break;
		case kCornerTheKing:
			if ((fromPoint.h-toPoint.h<=1) && (fromPoint.v-toPoint.v<=1))
				return eNoErr;
			return eMustMoveLikeKing;
			break;
		case kCornerTheSuperqueen:
			if (toPoint.h==fromPoint.h)
				return eNoErr;
			if (toPoint.v==fromPoint.v)
				return eNoErr;
			if ((fromPoint.v-toPoint.v)==(fromPoint.h-toPoint.h))
				return eNoErr;
			if ((fromPoint.h-toPoint.h==1) && (fromPoint.v-toPoint.v==2))
				return eNoErr;
			if ((fromPoint.h-toPoint.h==2) && (fromPoint.v-toPoint.v==1))
				return eNoErr;
			return eMustMoveLikeQueenOrKnight;
			break;
		case kCornerTheSuperking:
			if ((fromPoint.h-toPoint.h<=1) && (fromPoint.v-toPoint.v<=1))
				return eNoErr;
			if ((fromPoint.h-toPoint.h==1) && (fromPoint.v-toPoint.v==2))
				return eNoErr;
			if ((fromPoint.h-toPoint.h==2) && (fromPoint.v-toPoint.v==1))
				return eNoErr;
			return eMustMoveLikeKingOrKnight;
			break;
	}
	
	return eNoErr;	/* should never get here, but we need to return something */
}

static	void CheckCornerEndGame(WindowRef theWindow)
{
	if ((gThePosition.h==0) && (gThePosition.v==0))
		EndGame(theWindow);
}

void CornerIdle(WindowRef theWindow)
{
	short			i, maxSafeMoves;
	Boolean			gotone;
	Point			magic, mirrorPoint, realPoint;
	
	maxSafeMoves=gNumSafeArray[TheCornerIndex];
	gotone=FALSE;
	for (i=0; ((i<maxSafeMoves) && (!gotone)); i++)
	{
		realPoint=gMisere ? gMisereSafeArray[TheCornerIndex][i] : gSafeArray[TheCornerIndex][i];
		if (ValidMove(gThePosition, realPoint)==eNoErr)
		{
			gotone=TRUE;
			magic=realPoint;
		}
	}
	
	if (!gotone)
	{
		for (i=0; ((i<maxSafeMoves) && (!gotone)); i++)
		{
			mirrorPoint.h=gMisere ? gMisereSafeArray[TheCornerIndex][i].v : gSafeArray[TheCornerIndex][i].v;
			mirrorPoint.v=gMisere ? gMisereSafeArray[TheCornerIndex][i].h : gSafeArray[TheCornerIndex][i].h;
			if (ValidMove(gThePosition, mirrorPoint)==eNoErr)
			{
				gotone=TRUE;
				magic=mirrorPoint;
			}
		}
	}
	
	gPlayer=!gPlayer;
	if (!gotone)
		CornerRandomMove(&magic);
	ShowCornerMove(theWindow, magic);
	
	CheckCornerEndGame(theWindow);
}

static	void CornerRandomMove(Point *yourFinalAnswer)
{
	short			r, i, x;
	Point			magic;
	
	do
	{
		r=(Random()&0x7fff)%3;
		magic=gThePosition;
		x=(magic.h<magic.v) ? magic.h : magic.v;
		if (x<=1)
			x=2;
		i=1+((Random()&0x7fff)%(x-1));
		switch (r)
		{
			case 0:	magic.h-=i;	break;
			case 1:	magic.v-=i;	break;
			case 2:	magic.h-=i;	magic.v-=i;	break;
		}
	}
	while (ValidMove(gThePosition, magic)!=eNoErr);
	*yourFinalAnswer=magic;
}

static	void ShowCornerMove(WindowRef theWindow, Point magic)
{
	gThePosition=magic;
	SetWindowOffscreenNeedsUpdate(theWindow, TRUE);
	UpdateTheWindow(theWindow);
	DoSound(sound_gridplay, TRUE);
}
