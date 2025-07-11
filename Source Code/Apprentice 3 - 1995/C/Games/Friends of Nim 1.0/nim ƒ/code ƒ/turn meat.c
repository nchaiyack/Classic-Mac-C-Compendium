#include "turn meat.h"
#include "nim endgame.h"
#include "nim message.h"
#include "nim globals.h"
#include "nim grid.h"
#include "window layer.h"
#include "offscreen layer.h"
#include "shadow box.h"
#include "timing.h"
#include "sound layer.h"

#define kWhiteColor			kSecondPlayer
#define kBlackColor			kFirstPlayer
#define kComputerTime		30

static	short			gThisGameNumRows, gThisGameNumColumns, gThisGameDistribution;
static	short			gTurnNimValues[kTurnNumRowsMax][kTurnNumColumnsMax]=
						{	{1, 2, 1, 4, 1, 2, 1, 8}, {2, 3, 2, 8, 2, 3, 2, 12},
							{1, 2, 1, 4, 1, 2, 1, 8}, {4, 8, 4, 6, 4, 8, 4, 11},
							{1, 2, 1, 4, 1, 2, 1, 8}, {2, 3, 2, 8, 2, 3, 2, 12},
							{1, 2, 1, 4, 1, 2, 1, 8}, {8, 12, 8, 11, 8, 12, 8, 13}	};
static	short			gVal[kTurnNumRowsMax][kTurnNumColumnsMax];
static	Boolean			gTurnBoard[kTurnNumRowsMax][kTurnNumColumnsMax];
static	Rect			gAvailableRect;

static	void ShowTurnMove(WindowRef theWindow, short minRow, short maxRow, short minCol,
	short maxCol);
static	void CheckTurnEndGame(WindowRef theWindow);
static	GameError ValidMove(short minRow, short maxCol);
static	short TurnNimSum(void);
static	void TurnRandomMove(WindowRef theWindow);
static	void HypotheticalMove(short minRow, short maxRow, short minCol, short maxCol);
static	Boolean CheckMisereEndGame(void);

void InitTurn(void)
{
}

void InitTurnOneGame(void)
{
	short			row, col, i;
	
	gThisGameNumRows=gTurnNumRows;
	gThisGameNumColumns=gTurnNumColumns;
	gThisGameDistribution=gTurnDistribution;
	
	for (row=0; row<gThisGameNumRows; row++)
	{
		for (col=0; col<gThisGameNumColumns; col++)
		{
			gTurnBoard[row][col]=kWhiteColor;
			gVal[row][col]=gTurnNimValues[gThisGameNumRows-1-row][col];
		}
	}
	
	switch (gThisGameDistribution)
	{
		case kTurnDistRandom:
			for (i=0; i<(((gThisGameNumRows+1)/2)*gThisGameNumColumns); i++)
			{
				do
				{
					row=(Random()&0x7fff)%gThisGameNumRows;
					col=(Random()&0x7fff)%gThisGameNumColumns;
				}
				while (gTurnBoard[row][col]==kBlackColor);
				
				gTurnBoard[row][col]=kBlackColor;
			}
			break;
		case kTurnDistCheckerboard:
			for (row=0; row<gThisGameNumRows; row+=2)
				for (col=0; col<gThisGameNumColumns; col+=2)
					gTurnBoard[row][col]=kBlackColor;
			for (row=1; row<gThisGameNumRows; row+=2)
				for (col=1; col<gThisGameNumColumns; col+=2)
					gTurnBoard[row][col]=kBlackColor;
			break;
		case kTurnDistParallelLines:
			for (col=0; col<gThisGameNumColumns; col+=2)
				for (row=0; row<gThisGameNumRows; row++)
					gTurnBoard[row][col]=kBlackColor;
			break;
	}
}

void TurnDrawWindow(WindowRef theWindow, short theDepth)
{
	Rect			boundsRect;
	Point			pos;
	
	SetRect(&boundsRect, 0, 0, gThisGameNumColumns*kLargeGridColumnWidth+28,
		gThisGameNumRows*kLargeGridRowHeight+28);
	OffsetRect(&boundsRect, (theWindow->portRect.right-theWindow->portRect.left-boundsRect.right)/2,
		(theWindow->portRect.bottom-theWindow->portRect.top-boundsRect.bottom)/2);
	Draw3DShadowBox(&boundsRect, theDepth, &gAvailableRect);
	DrawGrid(TRUE, theDepth, &gAvailableRect, gThisGameNumRows, gThisGameNumColumns);
	
	for (pos.h=0; pos.h<gThisGameNumColumns; pos.h++)
		for (pos.v=0; pos.v<gThisGameNumRows; pos.v++)
			DrawGridPosition(TRUE, &gAvailableRect, pos, gTurnBoard[pos.v][pos.h], gThisGameNumRows);
}

void TurnClick(WindowRef theWindow, Point thePoint)
{
	Point			pos1, pos2, topLeft, lastTopLeft;
	short			minRow, maxRow, minCol, maxCol;
	Rect			theRect;
	GameError		e;
	
	SetPort(theWindow);
	pos1=PositionFromPoint(TRUE, thePoint, &gAvailableRect, gThisGameNumRows);
	if ((pos1.h<0) || (pos1.h>=gThisGameNumColumns) || (pos1.v<0) || (pos1.v>=gThisGameNumRows))
		return;
	
	lastTopLeft=thePoint;
	PenMode(patXor);
	SetRect(&theRect, thePoint.h, thePoint.v, thePoint.h, thePoint.v);
	FrameRect(&theRect);
	
	while (StillDown())
	{
		GetMouse(&topLeft);
		if ((topLeft.h!=lastTopLeft.h) || (topLeft.v!=lastTopLeft.v))
		{
			FrameRect(&theRect);
			theRect.bottom=(topLeft.v>thePoint.v) ? topLeft.v : thePoint.v;
			theRect.top=(topLeft.v<thePoint.v) ? topLeft.v : thePoint.v;
			theRect.left=(topLeft.h<thePoint.h) ? topLeft.h : thePoint.h;
			theRect.right=(topLeft.h>thePoint.h) ? topLeft.h : thePoint.h;
			FrameRect(&theRect);
		}
		lastTopLeft=topLeft;
	}
	FrameRect(&theRect);
	PenNormal();
	
	pos2=PositionFromPoint(TRUE, topLeft, &gAvailableRect, gThisGameNumRows);
	if ((pos2.h<0) || (pos2.h>=gThisGameNumColumns) || (pos2.v<0) || (pos2.v>=gThisGameNumRows))
	{
		DisplayMessage(eOffBoard);
		return;
	}
	
	minRow=(pos1.v<pos2.v) ? pos1.v : pos2.v;
	maxRow=(pos1.v>pos2.v) ? pos1.v : pos2.v;
	minCol=(pos1.h<pos2.h) ? pos1.h : pos2.h;
	maxCol=(pos1.h>pos2.h) ? pos1.h : pos2.h;
	if ((e=ValidMove(minRow, maxCol))!=noErr)
	{
		DisplayMessage(e);
		return;
	}
	
	ShowTurnMove(theWindow, minRow, maxRow, minCol, maxCol);
	gPlayer=!gPlayer;
	CheckTurnEndGame(theWindow);
}

static	void ShowTurnMove(WindowRef theWindow, short minRow, short maxRow, short minCol,
	short maxCol)
{
	MyOffscreenPtr	offscreenWorldPtr;
	Point			pos;
	
	offscreenWorldPtr=GetWindowPermanentOffscreenWorld(theWindow);
	SetPortToOffscreenWindow(offscreenWorldPtr);
	
	for (pos.v=minRow; pos.v<=maxRow; pos.v++)
	{
		for (pos.h=minCol; pos.h<=maxCol; pos.h++)
		{
			gTurnBoard[pos.v][pos.h]=!gTurnBoard[pos.v][pos.h];
			DrawGridPosition(TRUE, &gAvailableRect, pos, gTurnBoard[pos.v][pos.h], gThisGameNumRows);
		}
	}
	
	SetPortToOnscreenWindow(theWindow, offscreenWorldPtr);
	UpdateTheWindow(theWindow);
	DoSound(sound_turnplay, TRUE);
}

static	void CheckTurnEndGame(WindowRef theWindow)
{
	short			row, col;
	
	for (row=0; row<gThisGameNumRows; row++)
		for (col=0; col<gThisGameNumColumns; col++)
			if (gTurnBoard[row][col]==kBlackColor)
				return;
	
	EndGame(theWindow);
}

static	GameError ValidMove(short minRow, short maxCol)
{
	return (gTurnBoard[minRow][maxCol]==kBlackColor) ? eNoErr : eBottomRightIsntBlack;
}

void TurnIdle(WindowRef theWindow)
{
	Point			bottomRight, topLeft;
	Boolean			gotone;
	short			sum, magicSum;
	short			magicMinRow, magicMaxRow, magicMinCol, magicMaxCol;
	
	StartTiming();
	magicSum=(gMisere && CheckMisereEndGame()) ? 1 : 0;
	sum=TurnNimSum();
	if (sum==magicSum)
	{
		TurnRandomMove(theWindow);
	}
	else
	{
		gotone=FALSE;
		for (bottomRight.v=0; ((bottomRight.v<gThisGameNumRows) && (!gotone)); bottomRight.v++)
		{
			for (bottomRight.h=gThisGameNumColumns-1; ((bottomRight.h>=0) && (!gotone)); bottomRight.h--)
			{
				if (ValidMove(bottomRight.v, bottomRight.h)==eNoErr)
				{
					for (topLeft.h=bottomRight.h; ((topLeft.h>=0) && (!gotone)); topLeft.h--)
					{
						for (topLeft.v=bottomRight.v; ((topLeft.v<gThisGameNumRows) && (!gotone)); topLeft.v++)
						{
							HypotheticalMove(bottomRight.v, topLeft.v, topLeft.h, bottomRight.h);
							sum=TurnNimSum();
							HypotheticalMove(bottomRight.v, topLeft.v, topLeft.h, bottomRight.h);
							if (sum==magicSum)
							{
								gotone=TRUE;
								magicMinRow=bottomRight.v;
								magicMaxRow=topLeft.v;
								magicMinCol=topLeft.h;
								magicMaxCol=bottomRight.h;
							}
						}
					}
				}
			}
		}
		
		if (!gotone)	/* shouldn't have to do this!!! algorithm still needs work */
		{
			TurnRandomMove(theWindow);
		}
		else
		{
			TimeCorrection(kComputerTime);
			ShowTurnMove(theWindow, magicMinRow, magicMaxRow, magicMinCol, magicMaxCol);
		}
	}
	
	gPlayer=!gPlayer;
	CheckTurnEndGame(theWindow);
}

static	void TurnRandomMove(WindowRef theWindow)
{
	Point			bottomRight;
	
	do
	{
		bottomRight.v=(Random()&0x7fff)%gThisGameNumRows;
		bottomRight.h=(Random()&0x7fff)%gThisGameNumColumns;
	}
	while (ValidMove(bottomRight.v, bottomRight.h)!=eNoErr);
	TimeCorrection(kComputerTime);
	ShowTurnMove(theWindow, bottomRight.v, bottomRight.v, bottomRight.h, bottomRight.h);
}

static	void HypotheticalMove(short minRow, short maxRow, short minCol, short maxCol)
{
	short			row, col;
	
	for (row=minRow; row<=maxRow; row++)
		for (col=minCol; col<=maxCol; col++)
			gTurnBoard[row][col]=!gTurnBoard[row][col];
}

static	short TurnNimSum(void)
{
	short			row, col;
	short			sum;
	
	sum=0;
	for (row=0; row<gThisGameNumRows; row++)
		for (col=0; col<gThisGameNumColumns; col++)
			if (gTurnBoard[row][col]==kBlackColor)
				sum^=gVal[row][col];
	
	return sum;
}

static	Boolean CheckMisereEndGame(void)
{
	short			row, col;
	short			num;
	Boolean			bad;
	
	bad=FALSE;
	for (row=0; ((row<gThisGameNumRows) && (!bad)); row++)
	{
		num=0;
		for (col=0; ((col<gThisGameNumColumns) && (!bad)); col++)
		{
			if (gTurnBoard[row][col]==kBlackColor)
			{
				num++;
				if (num>1)
					bad=TRUE;
			}
		}
	}
	
	if (!bad)
		return TRUE;
	
	bad=FALSE;
	for (col=0; ((col<gThisGameNumColumns) && (!bad)); col++)
	{
		num=0;
		for (row=0; ((row<gThisGameNumRows) && (!bad)); row++)
		{
			if (gTurnBoard[row][col]==kBlackColor)
			{
				num++;
				if (num>1)
					bad=TRUE;
			}
		}
	}
	
	return !bad;
}
