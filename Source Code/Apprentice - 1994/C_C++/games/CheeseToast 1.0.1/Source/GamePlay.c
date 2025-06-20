/****************************************************************************
 * GamePlay.c
 *
 *		Overall Game routines
 *
 * 2/7/94
 *		Updated to work on 12 inch (512x384) monitors and 
 *		to center properly on monitors larger than 640 x 480
 *
 * 4/28/93 - 5/93
 *		Original Code Written
 ****************************************************************************/
 
#define SPRITEMAIN	1
#include "CToast.h"
#include "math.h"

// Game Initialization
//
void InitializeGame(WindowPtr theWin)
{
	short			i,j,n,b;
	GDHandle		gDevice;
	Ptr				sp,dp,cp;
	Handle			h;
	OSErr			oe;
	GDHandle		curDevice;

	// Fade out the screen (comment this out if you want to use debugger)
	GammaFadeOut();
	curDevice = GetGDevice();

	// Get Video Ptr and Dimensions
	gVideoRowBytes = (*(*curDevice)->gdPMap)->rowBytes & 0x3FFF;
	gVideoMem = (*(*curDevice)->gdPMap)->baseAddr;

	// Choose between 512x384 and 640x480 modes
	//
	if (screenBits.bounds.bottom - screenBits.bounds.top < 480 ||
		screenBits.bounds.right - screenBits.bounds.left < 640) 
	{
		g12InchMode = true;
		SetRect(&gOffscreenRect,0,0,512+IconWidth*2,384+IconWidth*2);
		gVidOffset.h = ((screenBits.bounds.right - screenBits.bounds.left) - 512)/2;
		gVidOffset.v = ((screenBits.bounds.bottom - screenBits.bounds.top) - 384)/2;
	}
	else {
		SetRect(&gOffscreenRect,0,0,640+IconWidth*2,480+IconWidth*2);
		gVidOffset.h = ((screenBits.bounds.right - screenBits.bounds.left) - 640)/2;
		gVidOffset.v = ((screenBits.bounds.bottom - screenBits.bounds.top) - 480)/2;
	}
	
	// Calc the center point
	gCenterP.h = (gOffscreenRect.right - gOffscreenRect.left) / 2;
	gCenterP.v = (gOffscreenRect.bottom - gOffscreenRect.top) / 2;

	// Allocate an offscreen pixel map
	//
	if ((oe = NewGWorld(&gOffScreen,0,&gOffscreenRect,NULL,NULL,0)) != noErr) {
		DebugStr("\pOy!");
		return;
	}
	gPixMap = GetGWorldPixMap(gOffScreen);
	LockPixels(gPixMap);

	gScreenRowBytes = (*gPixMap)->rowBytes & 0x3FFF;
	gScreenMem = (*gPixMap)->baseAddr;

	gPlayRect = gOffscreenRect;
	gPlayRect.right -= IconWidth;
	gPlayRect.bottom -= IconHeight;

	// Load User Preferences - Sound Volume, Keys
	LoadPreferences();	// Do this before loading sounds - has volume setting

	// Load the Sounds
	InitSounds();

	// Hide Menubar
	oldMBarHeight = MBarHeight;
	MBarHeight = 0;
	RectRgn(theWin->visRgn,&screenBits.bounds);

	// Paint Startup Screen, Fade In
	PaintRect(&theWin->portRect);
	StartupScreen();
	GammaFadeIn();

	// Draw Status Bar - used to show initialization progress
	DrawStatusBar(0,100);

	// Randomize the Random Number Generator
	GetDateTime((unsigned long *) &randSeed);

	// Play the startup sound - note: i'm getting a sound glitch
	// when sound manager 3.0 is used
	PlaySound(S_Startup, 4);

	// Calc dimensions of play field
	gPlayWidth = gPlayRect.right - gPlayRect.left;
	gPlayHeight = gPlayRect.bottom - gPlayRect.top;

	// Allocate tables
	smTable = (SaveMapRecord *) NewPtrClear(sizeof(SaveMapRecord) * MaxSaveMaps);
	if (smTable == NULL)
		DebugStr("\pOut of Mem");

	sTable = (SpriteInstance *) NewPtrClear(sizeof(SpriteInstance) * MaxSprites);
	if (sTable == NULL)
		DebugStr("\pOut of Mem");

	// Load in Sprites
	//
	for (i = 0; i < NbrSprites; ++i) {
		DrawStatusBar(i,NbrSprites+1);

		if (sDef[i].firstIconID) {
			// Allocate Pix Maps
			sDef[i].colorMaps = NewPtrClear(ColorMapSize * sDef[i].nbrIcons);
			if (sDef[i].colorMaps == NULL)
				DebugStr("\pOut of Mem");
			sDef[i].maskMaps = NewPtrClear(ColorMapSize *  sDef[i].nbrIcons);
			if (sDef[i].maskMaps == NULL)
				DebugStr("\pOut of Mem");
	
			// Load Icons
			for (j = 0; j < sDef[i].nbrIcons; ++j) {
	
				// Get Color Sprite Picture
				h = GetResource('icl8',sDef[i].firstIconID+j);
				BlockMove(*h,sDef[i].colorMaps + AniFrameIndex(j), ColorMapSize);
				ReleaseResource(h);
	
				// Get Mask, convert to 8 bit, clear unmasked colors in
				// sprite
				h = GetResource('ICN#',sDef[i].firstIconID+j);
				sp = *h + MaskMapSize;
				dp = sDef[i].maskMaps + AniFrameIndex(j);
				cp = sDef[i].colorMaps + AniFrameIndex(j);
				b = 0x80;
				for (n = 0; n < ColorMapSize; ++n) {
					if ((*sp & b) > 0) {
						*dp = 0;
					}
					else {
						*dp = 0xFF;
						*cp = 0;
					}
					++dp;
					++cp;
					b >>= 1;
					if (b == 0) {
						b = 0x80;
						++sp;
					}
				}
				ReleaseResource(h);
			}
		}
	}
	DrawStatusBar(NbrSprites,NbrSprites+1);

	// Load Top Scores
	LoadTopScores();
	DrawStatusBar(NbrSprites+1,NbrSprites+1);
	
	SetPort(theWin);
}

// CleanUp before quitting
//
void CleanUp(void)
{
	EndSounds();
	ShowCursor();
	MBarHeight = oldMBarHeight;
	CloseResFile(gResFile);
}

void DrawStatusBar(short curLevel, short maxLevel)
{
	CGrafPtr		curPort;
	GDHandle		curDevice;
	Rect			r,r2;
	RGBColor		saveColor,foreColor;

	GetGWorld(&curPort,&curDevice);
	SetGWorld(gOffScreen,NULL);

	SetRect(&r,0,0,412,2);

	// Position within 512x384 Rectangle
	OffsetRect(&r,(512-412)/2,384-8);

	// Position within offscreen video area (which is scaled to monitor size)
	// But max 640 x 480
	OffsetRect(&r,(gOffScreen->portRect.right - 512)/2,
				  (gOffScreen->portRect.bottom - 384)/2);

	GetForeColor(&saveColor);
	foreColor.red = 0x0000;
	foreColor.green = 0x9999;
	foreColor.blue = 0xCCCC;
	RGBForeColor(&foreColor);
	PaintRect(&r);
	r2 = r;
	r2.right -= (r2.right-r2.left)-(((r2.right-r2.left)*curLevel)/maxLevel);
	foreColor.green = 0;
	RGBForeColor(&foreColor);
	PaintRect(&r2);
	RGBForeColor(&saveColor);

	SetGWorld(curPort,curDevice);

	MyCopyRect(&r);
}

// Display Attract Mode Screen
void BeginAttract(void)
{
	CGrafPtr	curPort;
	GDHandle	curDevice;
	static 		StringPtr promptStr1 = "\pHit P to Play";
	static 		StringPtr promptStr2 = "\p0-7 to change volume";
	static 		StringPtr promptStr3 = "\pK to redefine keys";


	GetGWorld(&curPort,&curDevice);

	SetGWorld(gOffScreen,NULL);
	PaintRect(&gOffScreen->portRect);

	DisplayPicture(BackgroundPICT,-1,-1);
	if (g12InchMode)
		DisplayPicture(SmallLogoPICT,42,42);
	else
		DisplayPicture(SmallLogoPICT,64,64);

	DisplayTopScores();

	TextFont(geneva);
	TextSize(12);
	TextFace(bold);
	TextMode(srcBic);

	CenterString(promptStr1);	Move(0,g12InchMode? -16 : -32);
	DrawString(promptStr1);

	CenterString(promptStr2);	Move(0,0);
	DrawString(promptStr2);

	CenterString(promptStr3);	Move(0,g12InchMode? 16 : 32);
	DrawString(promptStr3);

	TextMode(srcCopy);


	SetGWorld(curPort,curDevice);

	GammaFadeOut();
	MyCopyBits();
	GammaFadeIn();
}


void BeginGame(void)
{
	short	i;
	CGrafPtr		curPort;
	GDHandle		curDevice;
	short			eMask;

	register		SpriteInstance	*sp;

	gMaxSprite = 0;
	gNbrSaveMaps = 0;
	gSpriteCnt = 0;
	gSparkCnt = 0;
	gAsteroidCnt = 0;
	gRemainingShips = 3;
	gGameLevel = 0;
	gGameScore = 0L;
	gShieldPower = MaxShieldPower;

	GetGWorld(&curPort,&curDevice);

	SetGWorld(gOffScreen,NULL);

	TextFont(geneva);
	TextSize(9);
	TextFace(0);
	TextMode(notSrcCopy);
	PaintRect(&gOffScreen->portRect);

	DisplayPicture(BackgroundPICT,-1,-1);

	SetGWorld(curPort,curDevice);

	InitStatusDisplay();	// Display sprites should be at the lowest level

	// NewDebugDisplay();		// Debug Sprites Next

	InitLevel();			// Asteroids

	NewShip();				// Ship

	GammaFadeOut();
	MyCopyBits();
	GammaFadeIn();

	// Draw New Sprites
	for (i = 0,sp = sTable; i < gMaxSprite; ++i,++sp)
		(*sDef[sp->type].drawFunc)(sp);

	eMask = everyEvent;
	eMask &= ~keyUpMask;
	eMask &= ~keyDownMask;
	eMask &= ~autoKeyMask;
	SetEventMask(eMask);

	gGameState = GS_Play;
}

void EndGame(void)
{
	short			eMask;
	CGrafPtr		curPort;
	GDHandle		curDevice;
	static StringPtr promptStr = "\pGame Over";
	SpriteInstance	*sp;
	short			i;
	long			t;
	EventRecord		dmyEvent;

	i = gMaxSprite;
	sp = &sTable[i-1];
	while (i--) {
		if (sp->active)
			(*sDef[sp->type].eraseFunc)(sp);
		--sp;
	}

	GetGWorld(&curPort,&curDevice);

	SetGWorld(gOffScreen,NULL);

	TextFont(geneva);
	TextSize(12);
	TextFace(bold);
	TextMode(srcBic);
	CenterString(promptStr);
	DrawString(promptStr);

	SetGWorld(curPort,curDevice);
	MyCopyBits();


	eMask = everyEvent;
	eMask &= ~keyUpMask;
	SetEventMask(eMask);
	FlushEvents(everyEvent, 0);

	t = TickCount();

	IntegrateScore(gGameScore, gGameLevel);

	gGameState = GS_Attract;
	while (TickCount() - t < 180L &&
			!OSEventAvail(everyEvent, &dmyEvent));
			;
}

// Create a new batch of asteroids
void InitLevel(void)
{
	short	i,n;

	n = gGameLevel/2 + 3;
	if (n > 20)
		n = 20;

	while (n--)
		NewAsteroid((n+gGameLevel) % NbrAsteroids);

	gScoreMultiply = 1;

	gBadGuyChance = 1200 - 100*(gGameLevel/3);

	if (gBadGuyChance < 300)
		gBadGuyChance = 300;

	gYummyChance = 1000;
}

// Level was completed successfully
void EndLevel(void)
{
	++gGameLevel;
	AddScore(LevelBonusScore*gGameLevel);
	PlaySound(S_LevelCompletion, 4);
}

// Main Game Loop
//
void MainGameLoop(WindowPtr theWin)
{
	register SpriteInstance	*sp;
	register short	i,n;
	register long	t;
	static EventRecord	dmyEvent;
	Rect	r;
#if DEBUGGING
	void	IdleTest();				// Debugging Function used to determine CPU load for profiler
#endif

#if __option(profile)			// 6/15 Optional profiling support
	_profile = 1;
#endif

	SetPort(theWin);

	do {
		t = Ticks;
	
		// If we're ready for next frame of animation
		//
		if (t - gLastDispTime >= gGameClockTicks) {
			//  Update the game clock
			++gGameClock;
			gLastDispTime = t;
	
			// Compact Sprite Table if necessary
			if (gMaxSprite > 20 && gMaxSprite > (gSpriteCnt<<1)) {
				for (i = n = 0; i < gMaxSprite; ++i) {
					if (sTable[i].active) {
						sTable[n] = sTable[i];
						if (sTable[n].saveMapPtr)
							sTable[n].saveMapPtr->sp = &sTable[n];
						if (gShip == &sTable[i])
							gShip = &sTable[n];
						++n;
					}
				}
				gMaxSprite = n;
				for (i = n = 0; i < gNbrSaveMaps; ++i) {
					if (smTable[i].active) {
						smTable[n] = smTable[i];
						smTable[n].sp->saveMapPtr = &smTable[n];
						++n;
					}
				}
				gNbrSaveMaps = n;
			}
	

			// Erase all Sprites from offscreen gworld
			// Each sprite has a pointer to an erase function
			//
			i = gMaxSprite;
			sp = &sTable[i-1];
			while (i--) {
				if (sp->active)
					(*sDef[sp->type].eraseFunc)(sp);
				--sp;
			}
	

			// Advance to next play level if necessary
			if (gAsteroidCnt == 0) {
				EndLevel();
				InitLevel();
			}

			// Check Keyboard - respond to keys
			CheckKeys();

			// Random Events - bad guys, yummies
			if (MyRandom(gBadGuyChance) == 0) {	// 1200 Random Bad Guys
				if (MyRandom(30) == 0)			// Every 30 minutes
					n = MyRandom(7);			// Lucky 7!
				else
					n = 1;
				while (n--) {
					switch (MyRandom(5)) {
					case 0:
					case 1:
						NewSaucer();
						break;
					case 2:
					case 3:
						NewBarbell();
						break;
					case 4:
						NewCube();
						break;
						
					}
				}
			}

			// Random Yummies
			if (gYummyCnt == 0 && MyRandom(gYummyChance) == 0)	// Random Yummys
				NewYummy();

			// Move Sprites - using Sprite's moveFunction
			//
			n = gMaxSprite;
			for (i = 0,sp=sTable; i < n; ++i,++sp) {
				// Update Sprite Position
				if (sp->active)
					(*sDef[sp->type].moveFunc)(sp);
			}

			// Draw Sprites in Offscreen GWorld
			//
			n = gMaxSprite;
			for (i = 0,sp=sTable; i < n; ++i,++sp) {
				// Render Sprite in New Position
				if (sp->active)
					(*sDef[sp->type].drawFunc)(sp);
			}
	
			// Render Sprites onto Onscreen video
			//
			n = gMaxSprite;
			for (i = 0,sp=sTable; i < n; ++i,++sp) {
				// Render Sprite in New Position, if it needs updating
				//
				if (sp->update) {
					register Point	p1,p2;

					sp->update = false;

					p1 = sp->oldPos;
					p2 = sp->pos;
					// If distance between old and new positions is large
					// than sprite has crossed boundaries from one side of screen
					// to the other, in this case the part of the sprite that was
					// erased is on the other side of the screen and needs to
					// be drawn separately
					//
					if (abs(p2.h - p1.h) > 320 ||
						abs(p2.v - p1.v) > 240)
					{
						// Object has crossed boundaries, draw two separate rects
						r.left = p1.h;
						r.top =  p1.v;
						r.right = p1.h+sp->width;
						r.bottom = p1.v+sp->width;
						// Clip
						if (r.left < IconWidth)
							r.left = IconWidth;
						if (r.top < IconHeight)
							r.top = IconHeight;
						if (r.right >= gPlayRect.right)
							r.right = gPlayRect.right-1;
						if (r.bottom >= gPlayRect.bottom)
							r.bottom = gPlayRect.bottom-1;
						if (r.right > r.left && r.bottom > r.top)
							MyCopyRect(&r);
						r.left = p2.h;
						r.top =  p2.v;
						r.right = p2.h+sp->width;
						r.bottom = p2.v+sp->width;
						// Clip
						if (r.left < IconWidth)
							r.left = IconWidth;
						if (r.top < IconHeight)
							r.top = IconHeight;
						if (r.right >= gPlayRect.right)
							r.right = gPlayRect.right-1;
						if (r.bottom >= gPlayRect.bottom)
							r.bottom = gPlayRect.bottom-1;
						if (r.right > r.left && r.bottom > r.top)
							MyCopyRect(&r);
					}
					// Otherwise, the part that was erased is nearby
					// we merge those two rects into one rect and
					// copy the whole area in one pass
					//
					else {
						// Merge rects into composite rect
						r.left = (p1.h < p2.h)? p1.h : p2.h;
						r.top = (p1.v < p2.v)? p1.v : p2.v;
						r.right = (p1.h > p2.h)? p1.h : p2.h;
						r.right += sp->width;
						r.bottom = (p1.v > p2.v)? p1.v : p2.v;
						r.bottom += sp->width;
						// Clip
						if (r.left < IconWidth)
							r.left = IconWidth;
						if (r.top < IconHeight)
							r.top = IconHeight;
						if (r.right >= gPlayRect.right)
							r.right = gPlayRect.right-1;
						if (r.bottom >= gPlayRect.bottom)
							r.bottom = gPlayRect.bottom-1;
						if (r.right > r.left && r.bottom > r.top)
							MyCopyRect(&r);
					}
					sp->oldPos = sp->pos;
				}
			}
		}
#if DEBUGGING
		// This debugging function is used with the profiler to determine
		// how much idle time I am getting on a slow machine
		// this was used to determine how fast I could make the game clock
		// it is currently set at 20 frames a second
		else {
			IdleTest();
		}
#endif
	// Keep looping as long as game is in play
	// If game is over, keep looping while there are visible sparks from the
	// last explosion
	} while (gGameState == GS_Play || (gGameState == GS_GameOver && gSparkCnt));
#if __option(profile)			// 6/15 Optional profiling support
	_profile = 0;
#endif
	FlushEvents(everyEvent,0);
}

void InitStatusDisplay(void)
{
	register SpriteInstance *sp;
	if ((sp = NewSprite(false)) == NULL)
		return;
	sp->type = ST_StatusDisplay;
	sp->param1 = 0;
	sp->pos.h = 40;
	sp->pos.v = 64;
	sp->width = 2;
	gLastScore = -1;
	gLastRemainingShips = -1;
	gLastLevel = -1;
	gLastScoreMultiply = -1;
	gLastShieldPower = -1;
}

// Two different sets of X coordinates are used for the status bar
// elements depending on whether I'm on a 12 inch monitor or not

enum {XScore, XLevel, XPots, XMultiply, XPause, XShields};

static short	xP[2][6] = {{40,122,202,286,352,520},
							{40,122-32,202-32,286-32,352-32,520-128}};

// Draw the game status bar on top of the screen
//
void StatusDraw(register SpriteInstance *sp)
{
	static char		scoreText[32];
	Rect			r;
	register short	cw;

	if (gLastScore != gGameScore) {
		gLastScore = gGameScore;
		StatusPrintf(xP[g12InchMode][XScore],44,"%-10ld",gGameScore);
	}
	if (gLastLevel != gGameLevel) {
		gLastLevel = gGameLevel;
		StatusPrintf(xP[g12InchMode][XLevel],44,"Level: %d", gGameLevel);
	}
	if (gLastRemainingShips != gRemainingShips) {
		gLastRemainingShips = gRemainingShips;
		StatusPrintf(xP[g12InchMode][XPots],44,"Pots: %d", gRemainingShips);
	}
	if (gLastScoreMultiply != gScoreMultiply) {
		gLastScoreMultiply = gScoreMultiply;
		if (gScoreMultiply > 1)
			StatusPrintf(xP[g12InchMode][XMultiply], 44,"x%d     ", gScoreMultiply);
		else
			StatusPrintf(xP[g12InchMode][XMultiply], 44,"        ", gScoreMultiply);
	}
	if (gLastShieldPower != gShieldPower) {
		RGBColor	fc;
		Rect		r;
		CGrafPtr		curPort;
		GDHandle		curDevice;

		GetGWorld(&curPort,&curDevice);
		SetGWorld(gOffScreen,NULL);

		gLastShieldPower = gShieldPower;
		MoveTo(xP[g12InchMode][XShields],40);

		fc.red = 0x0000;	fc.green = fc.blue = 0x8080;
		RGBForeColor(&fc);
		Line(gShieldPower,0);

		fc.red = 0xFFFF;	fc.green = fc.blue = 0x0000;
		RGBForeColor(&fc);
		Line(MaxShieldPower-gShieldPower,0);

		fc.red = fc.green = fc.blue = 0x0000;
		RGBForeColor(&fc);
		SetRect(&r,xP[g12InchMode][XShields],40,xP[g12InchMode][XShields]+100,41);
		MyCopyRect(&r);

		SetGWorld(curPort,curDevice);
	}
}

// Keep track of game score
//
void AddScore(short amt)
{
	gGameScore += amt * (long) gScoreMultiply;
	if (gGameScore/50000L != gLastScore/50000) {
		++gRemainingShips;
		PlaySound(S_ExtraShip, 4);
	}
}

// Print part of top score display
//
void PrintfXY(short x, short y, char *template, ...)
{
	CGrafPtr		curPort;
	GDHandle		curDevice;
	char 			tbuf[128];
	va_list 		args;
	Rect			r;
	register		short cw;

	va_start(args,template);
	vsprintf(tbuf,template,args);
	va_end(args);

	CtoPstr(tbuf);

	cw = StringWidth((StringPtr) tbuf);
	MoveTo(x,y);
	DrawString((StringPtr) tbuf);
}

// Move to the right position to center a string
//
void CenterString(StringPtr str)
{
	MoveTo(((gOffscreenRect.right - gOffscreenRect.left)-StringWidth(str))/2,
			(gOffscreenRect.bottom - gOffscreenRect.top)/2);
}

// Print part of status bar display
//
void StatusPrintf(short x, short y, char *template, ...)
{
	CGrafPtr		curPort;
	GDHandle		curDevice;
	char 			tbuf[128];
	va_list 		args;
	Rect			r;
	register		short cw;

	va_start(args,template);
	vsprintf(tbuf,template,args);
	va_end(args);

	CtoPstr(tbuf);

	GetGWorld(&curPort,&curDevice);
	SetGWorld(gOffScreen,NULL);
	cw = StringWidth((StringPtr) tbuf);
	MoveTo(x,y);
	DrawString((StringPtr) tbuf);
	SetGWorld(curPort,curDevice);

	r.left = x;
	r.top = y - 10;
	r.right = x + cw;
	r.bottom = y;

	MyCopyRect(&r);
}

// Keyboard handling
// for speed, we use GetKeys function which polls entire keyboard
// bypassing normal WaitNextEvent handlng
//
// DeBounce is used to prevent rapid firing
// in "Uzi" mode, debounce is less to allow rapid firing
//
#define Debounce		10
#define UziDebounce		1

short	fireCtr=0;

void CheckKeys(void)
{
	static KeyMap	km;
	register short	flags;
	extern SpriteInstance	*gShip;

	GetKeys(km);


	// Non Ship Related Stuff
	//
	if ((((char *) km)[1] & 0x10) ||		// 0x0C Q
		(((char *) km)[6] & 0x20)) {		// 0x35 ESC
		gGameState = GS_GameOver;
	}

#if DEBUGGING
	if (((char *) km)[5] & 0x04) {			// 0x2A \		trigger random event
		switch (MyRandom(4)) {
		case 0:	NewYummy();		break;
		case 1:	NewSaucer(); 	break;
		case 2: NewBarbell();	break;
		case 3: NewCube();		break;
		}
	}
	if (((char *) km)[6] & 0x04) {			// 0x32 ~		trigger random event
		switch (MyRandom(2)) {
		case 0:	NewYummy();				break;
		case 1: NewAsteroid(ST_Jim);	break;
		}
	}
#endif

	if (((char *) km)[4] & 0x08) {			// 0x23 P
		do {
			GetKeys(km);
		} while (((char *) km)[4] & 0x08);	// Wait for Key Up

		// Print Pause Message
		StatusPrintf(xP[g12InchMode][XPause],44,"PAUSE");

		do {
			GetKeys(km);
		} while (!(((char *) km)[4] & 0x08));	// Wait for Key Down
		do {
			GetKeys(km);
		} while (((char *) km)[4] & 0x08);	// Wait for Key Up

		// Print UnPause Message
		StatusPrintf(xP[g12InchMode][XPause],44,"             ");
	}

	// Ship Related Stuff
	//

	if (!gShip)
		return;

	flags = false;

	if (((char *) km)[gPrefs.thrustKeyByte] & gPrefs.thrustKeyBit) {
		flags |= SF_Thrust;
	}

	if (((char *) km)[gPrefs.fireKeyByte] & gPrefs.fireKeyBit) {
		if (fireCtr == 0) {
			flags |= SF_Fire;
			fireCtr = ((gShipMode & SM_Uzi)? UziDebounce : Debounce);
		}
		else
			--fireCtr;
	}
	else
		fireCtr = 0;

	if (((char *) km)[gPrefs.leftKeyByte] & gPrefs.leftKeyBit) {
		flags |= SF_Left;
	}

	if (((char *) km)[gPrefs.rightKeyByte] & gPrefs.rightKeyBit) {
		flags |= SF_Right;
	}

	if (((char *) km)[gPrefs.shieldKeyByte] & gPrefs.shieldKeyBit) {
		if (gShieldPower > 0) {
			gShieldPower--;				// Use up shield Power
			flags |= SF_Shield;
		}
	}
	else
		if ((gGameClock & 0xFF) == 0)		// Increase Shield Power every 16 secs
			if (gShieldPower < MaxShieldPower)
				gShieldPower += 5;

	if (gShipMode & SM_AutoShield) {
		if (gShip && gShip->tickCtr > 40)
			gShipMode &= ~SM_AutoShield;
		else
			flags |= SF_Shield;
	}

	gShip->param1 = flags;

}

#if DEBUGGING
void IdleTest()
{
	short	x;
	x = 0;
}
#endif
