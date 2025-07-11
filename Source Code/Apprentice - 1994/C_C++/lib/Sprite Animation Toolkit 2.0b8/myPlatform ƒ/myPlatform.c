#include "SAT.h"

/**************/
/* myPlatform demo*/
/**************/

/* This demo is a hack I made, testing if we can use faceless sprites to make stationary*/
/* obstacles. That worked pretty nicely, so I went on and made some moving platforms too.*/
/* Take it for what it is: a test hack that suggests one way to make this kind of games.*/
/* There are many other ways. The controls can be improved a lot, but it is a start.*/

/* Translated to C by Mike Zimmerman */

#include "myPlatform.h"

SATPatHandle	thepat;
long			 L;
SpritePtr		ignoreSp;
Point			p;

void DrawInfo()
{
	Rect	r;
	
	SetPort(gSAT.backScreen);
	SetRect(&r, 100, 50, 300, 100);
	EraseRect(&r);
	FrameRect(&r);
	MoveTo(110, 70);
	DrawString("\pSAT Platform demo");
	MoveTo(110, 90);
	DrawString("\pMove with , . and space");
	SATBackChanged(&r);
}


main()
{
	Rect		tempRect;

	MaxApplZone ();
	FlushEvents (everyEvent - diskMask, 0 );
	InitGraf (&thePort);
	InitFonts ();
	InitWindows ();
	InitMenus ();
	TEInit ();
	InitDialogs (nil);		/* no restart proc */
	InitCursor ();

	MoreMasters ();
	MoreMasters ();


/* End of initializations */

	ConfigureSAT(true, kLayerSort, kBackwardCollision, 32);
	InitSAT(0, 0, 512, 322);  /* No PICTs */

	/* Use a background pattern (instead of PICTs - just to demo that too) */
	
	SetPort(gSAT.backScreen);  
	thepat = SATGetPat(128);
	SATPenPat(thepat);
	SetRect(&tempRect, 0, 0, gSAT.offSizeH, gSAT.offSizeV);
	PaintRect(&tempRect);
	
	CopyBits(&(gSAT.backScreen)->portBits, &gSAT.offScreen->portBits, &gSAT.offScreen->portRect, &gSAT.offScreen->portRect, srcCopy, nil);

	DrawInfo();
	
/*Initialize all sprite units*/
	InitPlayerSprite();
	InitPlatform();
	InitMovPlatform();
	InitHMovPlatform();
	
/*Show the game window and update it. (SATwind and gameWind are the same.)*/
	ShowWindow(gSAT.wind);
	SelectWindow(gSAT.wind);
	PeekOffscreen();
	
	SetRect(&tempRect, 0, 0, gSAT.offSizeH, gSAT.offSizeV);
  	CopyBits(&(gSAT.offScreen->portBits), &(gSAT.wind->portBits), &tempRect, &tempRect, srcCopy, nil);

	GetMouse(&p);
	ignoreSp = NewSprite(1, p.h, p.v, HandlePlayerSprite, SetupPlayerSprite, HitPlayerSprite);
	ignoreSp = NewSprite(0, 50, 300, HandlePlatform, SetupPlatform, HitPlatform);
	ignoreSp = NewSprite(0, 150, 200, HandlePlatform, SetupPlatform, HitPlatform);
	ignoreSp = NewSprite(0, 250, 100, HandlePlatform, SetupPlatform, HitPlatform);
	ignoreSp = NewSprite(0, 350, 50, HandlePlatform, SetupPlatform, HitPlatform);
	ignoreSp = NewSprite(0, 350, 300, HandleMovPlatform, SetupMovPlatform, HitMovPlatform);
	ignoreSp = NewSprite(0, 50, 200, HandleMovPlatform, SetupMovPlatform, HitMovPlatform);
	ignoreSp = NewSprite(0, 200, 150, HandleHMovPlatform, SetupHMovPlatform, HitHMovPlatform);

	HideCursor();
	while (!Button())
	{
		L = TickCount();
		RunSAT(true);
		while (L > TickCount() - 2L)
			;
	}
	ShowCursor();
	SATSoundShutup();
}
