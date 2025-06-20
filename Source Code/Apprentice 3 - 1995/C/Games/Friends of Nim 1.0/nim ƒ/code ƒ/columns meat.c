#include "columns meat.h"
#include "nim endgame.h"
#include "nim message.h"
#include "nim grid.h"
#include "nim globals.h"
#include "shadow box.h"
#include "sound layer.h"
#include "window layer.h"

#define ThePlayerIndex		(gPlayer==kFirstPlayer ? 0 : 1)

static	short			gThisGameNumRows, gThisGameNumColumns, gThisGameDistribution;
static	short			gPlayerPosition[2][kColumnsNumColumnsMax];
static	Rect			gAvailableRect;

static	GameError ValidMove(Point pos);
static	void ShowColumnsMove(WindowRef theWindow, Point pos);
static	void CheckColumnsEndGame(WindowRef theWindow);
static	short CalculateNimSum(void);
static	Boolean CheckMisereEndGame(void);

void InitColumns(void)
{
}

void InitColumnsOneGame(void)
{
	short			i, j, r;
	
	gThisGameNumRows=gColumnsNumRows;
	gThisGameNumColumns=gColumnsNumColumns;
	gThisGameDistribution=gColumnsDistribution;
	
	switch (gThisGameDistribution)
	{
		case kNimDistRandom:
			for (i=0; i<gThisGameNumColumns; i++)
			{
				r=(Random()&0x7fff)%(gThisGameNumRows-1);
				gPlayerPosition[0][i]=r;
				r=((Random()&0x7fff)%(gThisGameNumRows-r-1))+r+1;
				gPlayerPosition[1][i]=r;
			}
			break;
		case kNimDistUniform:
			for (i=0; i<gThisGameNumColumns; i++)
			{
				gPlayerPosition[0][i]=0;
				gPlayerPosition[1][i]=gThisGameNumRows-1;
			}
			break;
		case kNimDistLinear:
			j=0;
			for (i=0; i<gThisGameNumColumns; i++)
			{
				gPlayerPosition[0][i]=j;
				gPlayerPosition[1][i]=gThisGameNumRows-1-j;
				j++;
				if (gThisGameNumRows-1-j<=j)
					j=0;
			}
			break;
	}
}

void ColumnsDrawWindow(WindowRef theWindow, short theDepth)
{
	Rect			boundsRect;
	short			i;
	Point			pos;
	
	SetRect(&boundsRect, 0, 0, gThisGameNumColumns*kGridColumnWidth+28,
		gThisGameNumRows*kGridRowHeight+28);
	OffsetRect(&boundsRect, (theWindow->portRect.right-theWindow->portRect.left-boundsRect.right)/2,
		(theWindow->portRect.bottom-theWindow->portRect.top-boundsRect.bottom)/2);
	Draw3DShadowBox(&boundsRect, theDepth, &gAvailableRect);
	DrawGrid(FALSE, theDepth, &gAvailableRect, gThisGameNumRows, gThisGameNumColumns);
	for (i=0; i<gThisGameNumColumns; i++)
	{
		pos.h=i;
		pos.v=gPlayerPosition[0][i];
		DrawGridPosition(FALSE, &gAvailableRect, pos, kFirstPlayer, gThisGameNumRows);
		pos.v=gPlayerPosition[1][i];
		DrawGridPosition(FALSE, &gAvailableRect, pos, kSecondPlayer, gThisGameNumRows);
	}
}

void ColumnsClick(WindowRef theWindow, Point thePoint)
{
	Point			pos;
	GameError		e;
	
	pos=PositionFromPoint(FALSE, thePoint, &gAvailableRect, gThisGameNumRows);
	if ((e=ValidMove(pos))==eNoErr)
	{
		ShowColumnsMove(theWindow, pos);
		gPlayer=!gPlayer;
		CheckColumnsEndGame(theWindow);
	}
	else
	{
		DisplayMessage(e);
	}
}

static	GameError ValidMove(Point pos)
{
	if (pos.v==gPlayerPosition[ThePlayerIndex][pos.h])
		return eSpotTaken;
	if (pos.v==gPlayerPosition[1-ThePlayerIndex][pos.h])
		return eSpotTaken;
	if ((pos.v>=gThisGameNumRows) || (pos.v<0) || (pos.h>=gThisGameNumColumns) || (pos.h<0))
		return eOffBoard;
	
	if (gPlayer==kFirstPlayer)
	{
		if (pos.v<gPlayerPosition[0][pos.h])
			return eCantMoveBackwards;
		if (pos.v<gPlayerPosition[1][pos.h])
			return eNoErr;
	}
	else
	{
		if (pos.v>gPlayerPosition[1][pos.h])
			return eCantMoveBackwards;
		if (pos.v>gPlayerPosition[0][pos.h])
			return eNoErr;
	}
	
	return eCantJumpOverOpponent;
}

static	void ShowColumnsMove(WindowRef theWindow, Point pos)
{
	gPlayerPosition[ThePlayerIndex][pos.h]=pos.v;
	SetWindowOffscreenNeedsUpdate(theWindow, TRUE);
	UpdateTheWindow(theWindow);
	DoSound(sound_gridplay, TRUE);
}

static	void CheckColumnsEndGame(WindowRef theWindow)
{
	short			i;
	
	for (i=0; i<gThisGameNumColumns; i++)
	{
		if ((gPlayerPosition[1][i]-gPlayerPosition[0][i]-1)>=1)
			return;
	}
	
	EndGame(theWindow);
}

void ColumnsIdle(WindowRef theWindow)
{
	short			i, nimSum, row, oldPosition, magic, magicRow, magicSum, min, max;
	Boolean			foundMagic;
	Point			pos;
	
	magicSum=(gMisere && CheckMisereEndGame()) ? 1 : 0;
	nimSum=CalculateNimSum();
	if (nimSum==magicSum)
	{	/* do random move */
		do
		{
			pos.h=(Random()&0x7fff)%gThisGameNumColumns;
			pos.v=(Random()&0x7fff)%gThisGameNumRows;
		}
		while (ValidMove(pos)!=eNoErr);
		ShowColumnsMove(theWindow, pos);
	}
	else
	{	/* do calculated move */
		foundMagic=FALSE;
		for (row=0; ((row<gThisGameNumColumns) && (!foundMagic)); row++)
		{
			oldPosition=gPlayerPosition[ThePlayerIndex][row];
			pos.h=row;
			min=(magicSum==0) ? (gPlayer==kFirstPlayer) ? oldPosition+1 : gPlayerPosition[0][row]+1 :
								0;
			max=(magicSum==0) ? (gPlayer==kFirstPlayer) ? gPlayerPosition[1][row]-1 : oldPosition-1 :
								gThisGameNumRows-1;
			for (i=min; ((i<=max) && (!foundMagic)); i++)
			{
				pos.v=i;
				if (ValidMove(pos)==eNoErr)
				{
					gPlayerPosition[ThePlayerIndex][row]=i;
					foundMagic=(CalculateNimSum()==magicSum);
					if (foundMagic)
					{
						magic=i;
						magicRow=row;
					}
					gPlayerPosition[ThePlayerIndex][row]=oldPosition;
				}
			}
		}
		pos.h=magicRow;
		pos.v=magic;
		ShowColumnsMove(theWindow, pos);
	}
	
	gPlayer=!gPlayer;
	CheckColumnsEndGame(theWindow);
}

static	short CalculateNimSum(void)
{
	short			i;
	short			sum;
	
	sum=0;
	for (i=0; i<gThisGameNumColumns; i++)
		sum^=(gPlayerPosition[1][i]-gPlayerPosition[0][i]-1);
	
	return sum;
}

static	Boolean CheckMisereEndGame(void)
{
	short			i;
	short			numOverOne;
	
	numOverOne=0;
	for (i=0; i<gThisGameNumColumns; i++)
	{
		if ((gPlayerPosition[1][i]-gPlayerPosition[0][i]-1)>1)
			numOverOne++;
	}
	
	return (numOverOne<=1);
}
