////////////
//
//	CAboutChessBox.cp
//
//	The Chess++ About Box Class
//
//	SUPERCLASS = CDirector
//	Copyright © 1993 Steven J. Bushell. All rights reserved.
//
////////////

#include <CBartender.h>
#include <CDecorator.h>
#include <CApplication.h>
#include "CAboutChessBox.h"
#include <CWindow.h>
#include <CBitMap.h>
#include <CBitMapPane.h>

#include <QDOffscreen.h>
#include <Stdlib.h>
#include <Sane.h>
#include <math.h>
#include <FixMath.h>

#define WINDChessSplashScreen 520

/*** Global Variables ***/

extern	CApplication	*gApplication;		/* Global Application object		*/
extern	CBartender		*gBartender;		/* Manages all menus				*/
extern	CBureaucrat		*gGopher;			/* First in line to get commands	*/
extern	CDesktop		*gDesktop;
extern	CDecorator		*gDecorator;

/**** C O N S T R U C T I O N / D E S T R U C T I O N   M E T H O D S ****/

void	CAboutChessBox::IAboutChessBox(CDirectorOwner *aSupervisor)
{	
	FlashMenuBar(1);
	CDirector::IDirector(aSupervisor);	/* Initialize superclass			*/
	itsWindow = new(CWindow);
	itsWindow->IWindow(WINDChessSplashScreen, true, gDesktop, this);

	GetDateTime((unsigned long *)&randSeed);
	switch((Random() % 2) + 1) {
		case 0:
			DoAboutChess();
			break;
		case 1:
			DoAboutChessGradualWiggle();
			break;
		case 2:
		default:
			DoAboutChessWiggle();
			break;
	}
}


static CGrafPtr	savedGWorld;
static GDHandle	savedGDevice;

static void EnterGWorldMode(GWorldPtr theNewGWorld)
{
	GetGWorld(&savedGWorld,&savedGDevice);
	SetGWorld(theNewGWorld,nil);
}

static void ExitGWorldMode(void)
{
	SetGWorld(savedGWorld,savedGDevice);
}
		
void	CAboutChessBox::DoAboutChessWiggle(void)
{
	register		i;
	PicHandle		thePict;
	Rect			picRect,blitRect,destRect,letRect[20];
	GWorldPtr		picWorld,bufWorld;
	PixMapHandle	picPmh,bufPmh;
	OSErr			qdErr;

	thePict = GetPicture(WINDChessSplashScreen);
	picRect = (*thePict)->picFrame;

	qdErr = NewGWorld(&picWorld,8,&picRect,nil,nil,0);
	qdErr = NewGWorld(&bufWorld,8,&picRect,nil,nil,0);

	itsWindow->ChangeSize(picRect.right-picRect.left,picRect.bottom-picRect.top);
	gDecorator->CenterWindow(itsWindow);
	itsWindow->Show();

	itsWindow->Prepare();
	SetOrigin(picRect.left,picRect.top);
	ClipRect(&picRect);
	DrawPicture(thePict,&picRect);	

	EnterGWorldMode(picWorld);
	SetOrigin(picRect.left,picRect.top);
	ClipRect(&picRect);
	DrawPicture(thePict,&picRect);	
	ExitGWorldMode();

	EnterGWorldMode(bufWorld);
	SetOrigin(picRect.left,picRect.top);
	ClipRect(&picRect);
	EraseRect(&picRect);
	ExitGWorldMode();

	ReleaseResource((Handle)thePict);
	TextMode(srcCopy);

	picPmh = GetGWorldPixMap(picWorld);
	bufPmh = GetGWorldPixMap(bufWorld);
	LockPixels(picPmh);
	LockPixels(bufPmh);
	
	SetRect(&letRect[0],50,100,113,216);	//	C
	SetRect(&letRect[1],116,92,183,216);	//	h
	SetRect(&letRect[2],184,126,238,216);	//	e
	SetRect(&letRect[3],236,128,285,216);	//	s
	SetRect(&letRect[4],285,128,332,216);	//	s
	SetRect(&letRect[5],332,130,395,216);	//	+
	SetRect(&letRect[6],396,130,458,216);	//	+

	SetRect(&picRect,24,88,472,226);

	while(!Button())
	{
		CopyBits((BitMap *)(*picPmh),(BitMap *)(*bufPmh),
					&picRect,&picRect,srcCopy,nil);
		EnterGWorldMode(bufWorld);
		for (i=0;i<7;i++)
		{
			destRect = letRect[i];
			OffsetRect(&destRect,Random()/3000,Random()/3000);
			CopyBits((BitMap *)(*picPmh),(BitMap *)(*bufPmh),
						&letRect[i],&destRect,adMin,nil);
		}
		ExitGWorldMode();
		CopyBits((BitMap *)(*bufPmh),&itsWindow->macPort->portBits,
					&picRect,&picRect,srcCopy,nil);					
	}

	UnlockPixels(picPmh);
	UnlockPixels(bufPmh);

	DisposeGWorld(picWorld);
	DisposeGWorld(bufWorld);

	while(!Button())
		;
}

void	CAboutChessBox::DoAboutChessGradualWiggle(void)
{
	register		i,div;
	PicHandle		thePict;
	Rect			picRect,blitRect,destRect,letRect[20];
	GWorldPtr		picWorld,bufWorld;
	PixMapHandle	picPmh,bufPmh;
	OSErr			qdErr;

	thePict = GetPicture(WINDChessSplashScreen);
	picRect = (*thePict)->picFrame;

	qdErr = NewGWorld(&picWorld,8,&picRect,nil,nil,0);
	qdErr = NewGWorld(&bufWorld,8,&picRect,nil,nil,0);

	itsWindow->ChangeSize(picRect.right-picRect.left,picRect.bottom-picRect.top);
	gDecorator->CenterWindow(itsWindow);
	itsWindow->Show();

	itsWindow->Prepare();
	SetOrigin(picRect.left,picRect.top);
	ClipRect(&picRect);
	DrawPicture(thePict,&picRect);	

	EnterGWorldMode(picWorld);
	SetOrigin(picRect.left,picRect.top);
	ClipRect(&picRect);
	DrawPicture(thePict,&picRect);	
	ExitGWorldMode();

	EnterGWorldMode(bufWorld);
	SetOrigin(picRect.left,picRect.top);
	ClipRect(&picRect);
	EraseRect(&picRect);
	ExitGWorldMode();

	ReleaseResource((Handle)thePict);
	TextMode(srcCopy);

	picPmh = GetGWorldPixMap(picWorld);
	bufPmh = GetGWorldPixMap(bufWorld);
	LockPixels(picPmh);
	LockPixels(bufPmh);
	
	SetRect(&letRect[0],50,100,113,216);	//	C
	SetRect(&letRect[1],116,92,183,216);	//	h
	SetRect(&letRect[2],184,126,238,216);	//	e
	SetRect(&letRect[3],236,128,285,216);	//	s
	SetRect(&letRect[4],285,128,332,216);	//	s
	SetRect(&letRect[5],332,130,395,216);	//	+
	SetRect(&letRect[6],396,130,458,216);	//	+

	div = 35000;
	while(!Button())
	{
		CopyBits((BitMap *)(*picPmh),(BitMap *)(*bufPmh),
					&picRect,&picRect,srcCopy,nil);
		EnterGWorldMode(bufWorld);
		for (i=0;i<7;i++)
		{
			destRect = letRect[i];
			OffsetRect(&destRect,Random()/div,Random()/div);
			if (div > 500)
				div -= 50;
			CopyBits((BitMap *)(*picPmh),(BitMap *)(*bufPmh),
						&letRect[i],&destRect,adMin,nil);
		}
		ExitGWorldMode();
		CopyBits((BitMap *)(*bufPmh),&itsWindow->macPort->portBits,
					&picRect,&picRect,srcCopy,nil);					
	}

	UnlockPixels(picPmh);
	UnlockPixels(bufPmh);

	DisposeGWorld(picWorld);
	DisposeGWorld(bufWorld);

	while(!Button())
		;
}

void	CAboutChessBox::DoAboutChess(void)
{
	register		i,div,divInc=0;
	PicHandle		thePict;
	Rect			picRect,blitRect,destRect,letRect[20];
	GWorldPtr		picWorld,bufWorld;
	PixMapHandle	picPmh,bufPmh;
	OSErr			qdErr;
	short			centerh,centerv;

	thePict = GetPicture(WINDChessSplashScreen);
	picRect = (*thePict)->picFrame;

	qdErr = NewGWorld(&picWorld,8,&picRect,nil,nil,0);
	qdErr = NewGWorld(&bufWorld,8,&picRect,nil,nil,0);

	itsWindow->ChangeSize(picRect.right-picRect.left,picRect.bottom-picRect.top);
	gDecorator->CenterWindow(itsWindow);
	itsWindow->Show();

	itsWindow->Prepare();
	SetOrigin(picRect.left,picRect.top);
	ClipRect(&picRect);

	EnterGWorldMode(picWorld);
	SetOrigin(picRect.left,picRect.top);
	ClipRect(&picRect);
	DrawPicture(thePict,&picRect);	
	ExitGWorldMode();

	EnterGWorldMode(bufWorld);
	SetOrigin(picRect.left,picRect.top);
	ClipRect(&picRect);
	EraseRect(&picRect);
	ExitGWorldMode();

	ReleaseResource((Handle)thePict);
	TextMode(srcCopy);

	picPmh = GetGWorldPixMap(picWorld);
	bufPmh = GetGWorldPixMap(bufWorld);
	LockPixels(picPmh);
	LockPixels(bufPmh);
	
	SetRect(&letRect[0],50,100,113,216);	//	C
	SetRect(&letRect[1],116,92,183,216);	//	h
	SetRect(&letRect[2],184,126,238,216);	//	e
	SetRect(&letRect[3],236,128,285,216);	//	s
	SetRect(&letRect[4],285,128,332,216);	//	s
	SetRect(&letRect[5],332,130,395,216);	//	+
	SetRect(&letRect[6],396,130,458,216);	//	+
	SetRect(&letRect[7],80,46,404,90);		//	Symantec's
	SetRect(&letRect[8],84,230,136,274);	//	by
	SetRect(&letRect[9],144,230,248,266);	//	Steve
	SetRect(&letRect[10],254,230,396,264);	//	Bushell

	centerh = (picRect.right - picRect.left) / 2 + picRect.left;
	centerv = (picRect.bottom - picRect.top) / 2 + picRect.top;
	div = 200;
	while(!Button())
	{
		EnterGWorldMode(bufWorld);
		EraseRect(&picRect);
		for (i=0;i<11;i++)
		{
			destRect = letRect[i];
			OffsetRect(&destRect,(centerh - (destRect.right - destRect.left)/2 - destRect.left)*200L/div,
								(centerv - (destRect.bottom - destRect.top)/2 - destRect.top)*200L/div);
			CopyBits((BitMap *)(*picPmh),(BitMap *)(*bufPmh),
						&letRect[i],&destRect,adMin,nil);
		}
		if (div < 30000)
			div+=divInc++;
		ExitGWorldMode();
		CopyBits((BitMap *)(*bufPmh),&itsWindow->macPort->portBits,
					&picRect,&picRect,srcCopy,nil);					
	}

	UnlockPixels(picPmh);
	UnlockPixels(bufPmh);

	DisposeGWorld(picWorld);
	DisposeGWorld(bufWorld);

	while(!Button())
		;
}