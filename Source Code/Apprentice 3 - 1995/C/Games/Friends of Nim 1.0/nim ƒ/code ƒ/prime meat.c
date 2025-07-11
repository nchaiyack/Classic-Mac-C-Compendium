#include "prime meat.h"
#include "nim endgame.h"
#include "nim globals.h"
#include "nim message.h"
#include "shadow box.h"
#include "offscreen layer.h"
#include "sound layer.h"
#include "window layer.h"
#include <Icons.h>

#define kLeftMargin			15
#define kTopMargin			12

#define kNimBoxWidth		55
#define kNimBoxHeight		262
#define kNimColumnWidth		71

static	short			gThisGameNumRows, gThisGameMaxPerRow, gThisGameDistribution;
static	short			gNimRowValues[kNimNumRowsMax];
static	Rect			gNimAvailableRect[kNimNumRowsMax];
static	short			gRowSelected, gNumSelected;
static	long			gLastClickTime=0L;

static	void PlotOneRowTiles(short theDepth, short i);
static	void NimDoubleClick(WindowRef theWindow);
static	void DoNimComputerMove(WindowRef theWindow);
static	short CalculateNimSum(void);
static	GameError ValidMove(short i);
static	void ShowNimComputerMove(WindowRef theWindow, short row, short num);
static	void CheckNimEndGame(WindowRef theWindow);
static	Boolean CheckMisereEndGame(void);

void InitPrime(void)
{
}

void InitPrimeOneGame(void)
{
	short			i, val;
	
	gThisGameNumRows=gPrimeNumRows;
	gThisGameMaxPerRow=gPrimeMaxPerRow;
	gThisGameDistribution=gPrimeDistribution;
	
	switch (gThisGameDistribution)
	{
		case kNimDistRandom:
			for (i=0; i<gThisGameNumRows; i++)
			{
				gNimRowValues[i]=((Random()&0x7fff)%gThisGameMaxPerRow)+1;
			}
			break;
		case kNimDistUniform:
			for (i=0; i<gThisGameNumRows; i++)
			{
				gNimRowValues[i]=gThisGameMaxPerRow;
			}
			break;
		case kNimDistLinear:
			val=1;
			for (i=0; i<gThisGameNumRows; i++)
			{
				gNimRowValues[i]=val;
				val++;
				if (val>gThisGameMaxPerRow)
					val=1;
			}
			break;
	}
	gRowSelected=-1;
	gNumSelected=0;
}

void PrimeDrawWindow(WindowRef theWindow, short theDepth)
{
	Rect			destRect;
	short			i;
	WindowRef		curPort;
	
	GetPort(&curPort);
	SetRect(&destRect, kLeftMargin, kTopMargin, kLeftMargin+kNimBoxWidth, kTopMargin+kNimBoxHeight);
	for (i=0; i<gThisGameNumRows; i++)
	{
		Draw3DShadowBox(&destRect, theDepth, &gNimAvailableRect[i]);
		PlotOneRowTiles(theDepth, i);
		OffsetRect(&destRect, kNimColumnWidth, 0);
	}
	
	SetWindowOffscreenNeedsUpdate(theWindow, FALSE);
}

static	void PlotOneRowTiles(short theDepth, short i)
{
	short			j;
	Rect			bitRect;
	Handle			selectedIconHandle;
	
	bitRect.left=(gNimAvailableRect[i].left+(gNimAvailableRect[i].right-gNimAvailableRect[i].left)/2)-8;
	bitRect.right=bitRect.left+16;
	bitRect.top=gNimAvailableRect[i].bottom-20;
	bitRect.bottom=bitRect.top+16;
	selectedIconHandle=(theDepth>2) ?
					((gPlayer==kFirstPlayer) ? gNimBitFirstSelectedHandle : gNimBitSecondSelectedHandle ) :
					gNimBitSecondSelectedHandle;
	for (j=0; j<gNimRowValues[i]; j++)
	{
		PlotIconSuite(&bitRect, atAbsoluteCenter, ttNone,
			((i==gRowSelected) && (j<gNumSelected)) ? selectedIconHandle : gNimBitIconHandle);
		OffsetRect(&bitRect, 0, -21);
	}
}

void PrimeClick(WindowRef theWindow, Point thePoint)
{
	short			i, which, oldRowSelected;
	Boolean			gotone=FALSE;
	MyOffscreenPtr	offscreenWorldPtr;
	long			clickTime;
	GameError		e;
	short			theDepth;
	Boolean			doubleClick=FALSE;
	
	clickTime=TickCount();
	theDepth=GetWindowDepth(theWindow);
	for (i=0; ((i<gThisGameNumRows) && (!gotone)); i++)
	{
		if (PtInRect(thePoint, &gNimAvailableRect[i]))
		{
			gotone=TRUE;
			which=kNimMaxPerRowMax-1-(thePoint.v-gNimAvailableRect[i].top-4)/21;
			if (which<gNimRowValues[i])
			{
				if ((e=ValidMove(which+1))==eNoErr)
				{
					oldRowSelected=gRowSelected;
					gRowSelected=i;
					if (which==gNumSelected-1)
					{
						if (clickTime-gLastClickTime<LMGetDoubleTime())
						{
							NimDoubleClick(theWindow);
							doubleClick=TRUE;
						}
					}
					gNumSelected=which+1;
					if (!WindowOffscreenNeedsUpdateQQ(theWindow))
					{
						offscreenWorldPtr=GetWindowPermanentOffscreenWorld(theWindow);
						SetPortToOffscreenWindow(offscreenWorldPtr);
						PlotOneRowTiles(theDepth, i);
						if ((oldRowSelected!=gRowSelected) && (oldRowSelected!=-1))
							PlotOneRowTiles(theDepth, oldRowSelected);
						SetPortToOnscreenWindow(theWindow, offscreenWorldPtr);
					}
					UpdateTheWindow(theWindow);
					if (doubleClick)
						DoSound(sound_nimplay, TRUE);
				}
				else
				{
					DisplayMessage(e);
					return;
				}
			}
		}
	}
	CheckNimEndGame(theWindow);
	gLastClickTime=clickTime;
}

static	void NimDoubleClick(WindowRef theWindow)
{
	gNimRowValues[gRowSelected]-=gNumSelected;
	gPlayer=!gPlayer;
	gRowSelected=-1;
	SetWindowOffscreenNeedsUpdate(theWindow, TRUE);
}

void PrimeIdle(WindowRef theWindow)
{
	CheckNimEndGame(theWindow);
	if (gGameStatus==kGameInProgress)
		DoNimComputerMove(theWindow);
	CheckNimEndGame(theWindow);
}

static	void DoNimComputerMove(WindowRef theWindow)
{
	short			i, nimSum, row, oldRowValue, magic, magicRow, magicSum;
	Boolean			foundMagic;
	
	magicSum=(gMisere && CheckMisereEndGame()) ? 1 : 0;
	nimSum=CalculateNimSum();
	if (nimSum==magicSum)
	{	/* do random move */
		do { magicRow=(Random()&0x7fff)%gThisGameNumRows; } while (gNimRowValues[magicRow]==0);
		do { magic=1+((Random()&0x7fff)%gNimRowValues[magicRow]); } while (ValidMove(magic)!=eNoErr);
	}
	else
	{	/* do calculated move */
		foundMagic=FALSE;
		for (row=0; ((row<gThisGameNumRows) && (!foundMagic)); row++)
		{
			oldRowValue=gNimRowValues[row];
			for (i=1; ((i<=oldRowValue) && (!foundMagic)); i++)
			{
				if (ValidMove(i)==eNoErr)
				{
					gNimRowValues[row]=oldRowValue-i;
					foundMagic=(CalculateNimSum()==magicSum);
					if (foundMagic)
					{
						magic=i;
						magicRow=row;
					}
				}
			}
			gNimRowValues[row]=oldRowValue;
		}
	}
	
	ShowNimComputerMove(theWindow, magicRow, magic);
	gPlayer=!gPlayer;
}

static	short CalculateNimSum(void)
{
	short			i;
	short			sum;
	
	sum=0;
	for (i=0; i<gThisGameNumRows; i++)
	{
		sum^=(gNimRowValues[i]%4);
	}
	
	return sum;
}

static	GameError ValidMove(short i)
{
	if ((i==1) || (i==2) || (i==3) || (i==5) || (i==7) || (i==11)) return eNoErr;
	else return eMustBePrime;
}

static	void ShowNimComputerMove(WindowRef theWindow, short row, short num)
{
	MyOffscreenPtr	offscreenWorldPtr;
	long			dummy;
	
	Delay(15, &dummy);
	gRowSelected=row;
	gNumSelected=num;
	offscreenWorldPtr=GetWindowPermanentOffscreenWorld(theWindow);
	if (!WindowOffscreenNeedsUpdateQQ(theWindow))
	{
		SetPortToOffscreenWindow(offscreenWorldPtr);
		PlotOneRowTiles(GetWindowDepth(theWindow), row);
		SetPortToOnscreenWindow(theWindow, offscreenWorldPtr);
	}
	UpdateTheWindow(theWindow);
	
	Delay(15, &dummy);
	
	gRowSelected=-1;
	gNumSelected=0;
	gNimRowValues[row]-=num;
	SetWindowOffscreenNeedsUpdate(theWindow, TRUE);
	UpdateTheWindow(theWindow);
	DoSound(sound_nimplay, TRUE);
}

static	void CheckNimEndGame(WindowRef theWindow)
{
	short			i;
	
	for (i=0; i<gThisGameNumRows; i++)
		if (gNimRowValues[i]>0)
			return;
	
	EndGame(theWindow);
}

static	Boolean CheckMisereEndGame(void)
{
	short			i;
	short			numOverOne;
	
	numOverOne=0;
	for (i=0; i<gThisGameNumRows; i++)
	{
		if ((gNimRowValues[i]%4)>1)
			numOverOne++;
	}
	
	return (numOverOne<=1);
}
