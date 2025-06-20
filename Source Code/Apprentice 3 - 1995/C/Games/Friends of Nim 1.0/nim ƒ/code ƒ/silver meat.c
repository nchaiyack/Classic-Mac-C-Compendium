#include "silver meat.h"
#include "nim endgame.h"
#include "nim message.h"
#include "nim globals.h"
#include "nim grid.h"
#include "shadow box.h"
#include "sound layer.h"
#include "window layer.h"

static	short			gThisGameNumPlaces, gThisGameNumTiles, gThisGameDistribution;
static	Rect			gAvailableRect;
static	short			gSelectedIndex;
static	short			gTilePosition[kSilverNumTilesMax];

static	GameError ValidMove(short tileIndex, short newPosition);
static	void DrawNewSelection(WindowRef theWindow, short tileIndex, short oldIndex);
static	void DrawMove(WindowRef theWindow, short selectedIndex, short newPosition);
static	void CheckSilverEndGame(WindowRef theWindow);
static	short SilverSum(void);
static	Boolean CheckMisereEndGame(void);

void InitSilver(void)
{
}

void InitSilverOneGame(void)
{
	short			i, j, r, index;
	Boolean			used[kSilverNumPlacesMax];
	
	gThisGameNumPlaces=gSilverNumPlaces;
	gThisGameNumTiles=gSilverNumTiles;
	gThisGameDistribution=gSilverDistribution;
	
	gSelectedIndex=-1;
	for (i=0; i<gThisGameNumPlaces; i++)
		used[i]=FALSE;
	
	switch (gThisGameDistribution)
	{
		case kNimDistRandom:
			for (i=0; i<gThisGameNumTiles; i++)
			{
				do
				{
					r=(Random()&0x7fff)%gThisGameNumPlaces;
				}
				while (used[r]);
				used[r]=TRUE;
			}
			index=0;
			for (i=0; i<gThisGameNumPlaces; i++)
			{
				if (used[i])
					gTilePosition[index++]=i;
			}
			break;
		case kNimDistUniform:
			j=gThisGameNumPlaces/gThisGameNumTiles;
			index=gThisGameNumPlaces-1;
			for (i=gThisGameNumTiles-1; i>=0; i--)
			{
				gTilePosition[i]=index;
				index-=j;
			}
			break;
	}
}

void SilverDrawWindow(WindowRef theWindow, short theDepth)
{
	Rect			boundsRect;
	Point			pos;
	short			i;
	
	SetRect(&boundsRect, 0, 0, gThisGameNumPlaces*kGridColumnWidth+28, kGridRowHeight+28);
	OffsetRect(&boundsRect, (theWindow->portRect.right-theWindow->portRect.left-boundsRect.right)/2,
		(theWindow->portRect.bottom-theWindow->portRect.top-boundsRect.bottom)/2);
	Draw3DShadowBox(&boundsRect, theDepth, &gAvailableRect);
	DrawGrid(FALSE, theDepth, &gAvailableRect, 1, gThisGameNumPlaces);
	pos.v=0;
	for (i=0; i<gThisGameNumTiles; i++)
	{
		pos.h=gTilePosition[i];
		if (i==gSelectedIndex)
			DrawGridPosition(FALSE, &gAvailableRect, pos, gPlayer, 1);
		else
			DrawNeutralGridPosition(FALSE, &gAvailableRect, pos, 1);
	}
}

void SilverClick(WindowRef theWindow, Point thePoint)
{
	Point			pos;
	short			i, tileIndex;
	Boolean			gotone;
	GameError		e;
	
	pos=PositionFromPoint(FALSE, thePoint, &gAvailableRect, 1);
	gotone=FALSE;
	for (i=0; ((i<gThisGameNumTiles) && (!gotone)); i++)
	{
		if (pos.h==gTilePosition[i])
		{
			gotone=TRUE;
			tileIndex=i;
		}
	}
	if (gotone)
	{	/* select a coin */
		if (gSelectedIndex==tileIndex)
			return;
		DrawNewSelection(theWindow, tileIndex, gSelectedIndex);
	}
	else
	{	/* move a coin */
		if (gSelectedIndex==-1)
		{
			DisplayMessage(eMustSelectFirst);
			return;
		}
		
		if ((e=ValidMove(gSelectedIndex, pos.h))==eNoErr)
		{
			DrawMove(theWindow, gSelectedIndex, pos.h);
			gPlayer=!gPlayer;
			CheckSilverEndGame(theWindow);
		}
		else
		{
			DisplayMessage(e);
			return;
		}
	}
}

static	GameError ValidMove(short tileIndex, short newPosition)
{
	if (newPosition<0)
		return eOffBoard;
	if (newPosition>=gTilePosition[tileIndex])
		return eMustMoveLeft;
	if (tileIndex==0)
		return eNoErr;
	if (newPosition<=gTilePosition[tileIndex-1])
		return eCantJumpOtherCoins;
	return eNoErr;
}

static	void DrawNewSelection(WindowRef theWindow, short tileIndex, short oldIndex)
{
	Point			pos;
	MyOffscreenPtr	offscreenWorldPtr;
	
	offscreenWorldPtr=GetWindowPermanentOffscreenWorld(theWindow);
	SetPortToOffscreenWindow(offscreenWorldPtr);
	if (oldIndex!=-1)
	{
		pos.v=0;
		pos.h=gTilePosition[oldIndex];
		DrawNeutralGridPosition(FALSE, &gAvailableRect, pos, 1);
	}
	pos.v=0;
	pos.h=gTilePosition[tileIndex];
	DrawGridPosition(FALSE, &gAvailableRect, pos, gPlayer, 1);
	gSelectedIndex=tileIndex;
	SetPortToOnscreenWindow(theWindow, offscreenWorldPtr);
	UpdateTheWindow(theWindow);
}

static	void DrawMove(WindowRef theWindow, short selectedIndex, short newPosition)
{
	gTilePosition[selectedIndex]=newPosition;
	gSelectedIndex=-1;
	SetWindowOffscreenNeedsUpdate(theWindow, TRUE);
	UpdateTheWindow(theWindow);
	DoSound(sound_gridplay, TRUE);
}

static	void CheckSilverEndGame(WindowRef theWindow)
{
	short			i;
	
	for (i=0; i<gThisGameNumTiles; i++)
		if (gTilePosition[i]!=i)
			return;
	
	EndGame(theWindow);
}

void SilverIdle(WindowRef theWindow)
{
	short			nimSum, magicSum, magicTileIndex, magicOffset, space, oldTilePosition;
	short			i, startIndex;
	Boolean			foundMagic;
	long			dummy;
	
	magicSum=(gMisere && CheckMisereEndGame()) ? 1 : 0;
	nimSum=SilverSum();
	if (nimSum==magicSum)
	{	/* do random move */
		do
		{
			magicTileIndex=(Random()&0x7fff)%gThisGameNumTiles;
			space=(magicTileIndex==0) ? gTilePosition[magicTileIndex] :
					gTilePosition[magicTileIndex]-gTilePosition[magicTileIndex-1]-1;
		}
		while (space==0);
		magicOffset=((Random()&0x7fff)%space)+1;
	}
	else
	{	/* do calculated move */
		foundMagic=FALSE;
		for (startIndex=1; ((startIndex<gThisGameNumTiles) && (!foundMagic)); startIndex++)
		{
			oldTilePosition=gTilePosition[startIndex];
			space=gTilePosition[startIndex]-gTilePosition[startIndex-1]-1;
			for (i=1; i<=space; i++)
			{
				gTilePosition[startIndex]=oldTilePosition-i;
				if ((nimSum=SilverSum())==magicSum)
				{
					foundMagic=TRUE;
					magicOffset=i;
					magicTileIndex=startIndex;
				}
			}
			gTilePosition[startIndex]=oldTilePosition;
		}
	}
	
	DrawNewSelection(theWindow, magicTileIndex, -1);
	Delay(30, &dummy);
	DrawMove(theWindow, magicTileIndex, gTilePosition[magicTileIndex]-magicOffset);
	gPlayer=!gPlayer;
	CheckSilverEndGame(theWindow);
}

static	Boolean CheckMisereEndGame(void)
{
	short			i;
	short			numOverOne;
	
	numOverOne=0;
	for (i=1; i<gThisGameNumTiles; i+=2)
	{
		if (gTilePosition[i]-gTilePosition[i-1]>2)
			numOverOne++;
	}
	
	return (numOverOne<=1);
}

static	short SilverSum(void)
{
	short			i;
	short			sum;
	
	sum=0;
	for (i=0; i<gThisGameNumTiles; i+=2)
		sum^=(gTilePosition[i+1]-gTilePosition[i]-1);
	
	return sum;
}
