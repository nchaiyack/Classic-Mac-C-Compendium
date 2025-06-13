#include "TestBed.h"
#include "TestBedAnim.h"
#include "GameUtils.h"
#include "QDUtils.h"
#include "KeyUtils.h"
#include "TilePict.h"
#include "SoundUtils.h"

// ---------------------------------------------------------------------------

#define MAXFRAMES 3000

// ---------------------------------------------------------------------------

void SetupBuffers();
void OutputSpeed(long elapsedTime, long frames);

// ---------------------------------------------------------------------------

void Animate() {
	EventRecord theEvent;
	Rect demoBounds;
	BasicSprite **sprites;
	BasicSprite *spriteList;
	Handle theSound;
	long frames, dummy;
	long time;
	short diff;
	short i;
	short demoWidth, demoHeight;

	if (Convert2GraphicsBuffer(&gTestBed.outputBuffer, gTestBed.demoWindow,
		&gTestBed.windowBounds, true) != noErr) {
		SysBeep(10); ExitToShell();
	}

	if (gTestBed.primaryDevice == GetMainDevice())
		HideMenuBar(false);
	SetupBuffers();

	/* Allocate space for our sprite data */
	sprites = (BasicSprite**)NewHandleClear(sizeof(BasicSprite) * gTestBed.numSpritesInDemo);
	if (sprites == NULL) {
		SysBeep(10); ExitToShell();
	}
	MoveHHi((Handle)sprites);
	HLock((Handle)sprites);
	spriteList = *sprites;
	
	/* Setup "master sprite" properties */
	/* 3 properties will already be set for us: offBounds, buffer, region. */
	switch(gTestBed.animationMethod) {
		case kCopyMaskMethod:
			gTestBed.masterSprite.maskBuffer = gTestBed.spriteMaskBuffer;
			gTestBed.masterSprite.mask = (Ptr)gTestBed.masterSprite.maskBuffer;
		break;

		case kCopyBitsRegionMethod:
			gTestBed.masterSprite.maskBuffer = NULL;
			gTestBed.masterSprite.mask = (Ptr)gTestBed.masterSprite.region;
		break;

		case kCopyBitsTransparentMethod:
		case kBlitterTransparentMethod:
			gTestBed.masterSprite.maskBuffer = NULL;
			gTestBed.masterSprite.mask = NULL;
		break;
		
		case kBlitterDeepMaskMethod:
			gTestBed.masterSprite.maskBuffer = gTestBed.spriteMaskDeepBuffer;
			gTestBed.masterSprite.mask = (Ptr)gTestBed.masterSprite.maskBuffer;
		break;
	}
	
	demoBounds = gTestBed.windowBounds;
	OffsetRect(&demoBounds, -demoBounds.left, -demoBounds.top);
	demoWidth = demoBounds.right - demoBounds.left;
	demoHeight = demoBounds.bottom - demoBounds.top;

	for (i = 0; i < gTestBed.numSpritesInDemo; i++) {
		BlockMove(&gTestBed.masterSprite, &spriteList[i], sizeof(BasicSprite));

		spriteList[i].oldRect = gTestBed.masterSprite.offBounds;
		spriteList[i].curRect = gTestBed.masterSprite.offBounds;
		spriteList[i].unionRect = gTestBed.masterSprite.offBounds;

		OffsetRect(&spriteList[i].curRect, GetRandom(2, (demoWidth-128)),
			GetRandom(2, (demoHeight-128)));
		/* Make sure sprite sits on an even location
		   if we're using the 4-bit blitters */
		if (gTestBed.bufferDepth == 4 && spriteList[i].curRect.left % 2 != 0)
			OffsetRect(&spriteList[i].curRect, -1, 0);

		if (gTestBed.bufferDepth == 16) {
			spriteList[i].hVel = GetRandom(1, 6) * 2;	// Even only
			spriteList[i].vVel = GetRandom(1, 10);
		}
		else if (gTestBed.bufferDepth == 8) {
			spriteList[i].hVel = GetRandom(1, 4) * 4;	// Multiples of 4 only
			spriteList[i].vVel = GetRandom(1, 10);
		}
		else if (gTestBed.bufferDepth == 4) {
			spriteList[i].hVel = GetRandom(1, 3) * 8;	// Multiples of 8 only
			spriteList[i].vVel = GetRandom(1, 6);
		}
		else {
			spriteList[i].hVel = GetRandom(1, 10);
			spriteList[i].vVel = GetRandom(1, 10);
		}
	}

	/* Do the bkgnd music/sound */
	theSound = NULL;
	if (gTestBed.useBkgndSound) {
		if (CreateSndChannel(0) == noErr) {
			theSound = GetResource('MUSE', 128);
			if (theSound != NULL) {
				MoveHHi(theSound);
				HLock(theSound);
			}
		}
	}

	SetPort(gTestBed.demoWindow);
	CopyGraphicsBuffer(gTestBed.dockBuffer, gTestBed.outputBuffer,
		&gTestBed.bufferBounds, &gTestBed.bufferBounds);
	HideCursor();
	frames = 0;
	if (gTestBed.useBkgndSound && theSound != NULL)
		PlayAsynch(theSound, 0);
	time = TickCount();
	while (!GetOSEvent(mDownMask + keyDownMask, &theEvent)) {
		frames++;

		for (i = 0; i < gTestBed.numSpritesInDemo; i++) {
			spriteList[i].oldRect = spriteList[i].curRect;
			FastOffsetRect(spriteList[i].curRect, spriteList[i].hVel, spriteList[i].vVel);

			diff = spriteList[i].curRect.right - demoBounds.right;
			if (diff > 0) {
				FastOffsetRect(spriteList[i].curRect, -diff, 0);
				spriteList[i].hVel = -spriteList[i].hVel;
			}
			else {
				diff = demoBounds.left - spriteList[i].curRect.left;
				if (diff > 0) {
					FastOffsetRect(spriteList[i].curRect, diff, 0);
					spriteList[i].hVel = -spriteList[i].hVel;
				}
			}
			diff = spriteList[i].curRect.bottom - demoBounds.bottom;
			if (diff > 0) {
				FastOffsetRect(spriteList[i].curRect, 0, -diff);
				spriteList[i].vVel = -spriteList[i].vVel;
			}
			else {
				diff = demoBounds.top - spriteList[i].curRect.top;
				if (diff > 0) {
					FastOffsetRect(spriteList[i].curRect, 0, diff);
					spriteList[i].vVel = -spriteList[i].vVel;
				}
			}

			//MoveRgnToRect(&spriteList[i].curRect, spriteList[i].region);
			FastUnionRect(spriteList[i].oldRect, spriteList[i].curRect, spriteList[i].unionRect);
		}

		for (i = 0; i < gTestBed.numSpritesInDemo; i++) {
			// Copy patch of bkgnd
			(*gTestBed.eraser)(gTestBed.bkgndBuffer, gTestBed.dockBuffer, &spriteList[i].oldRect, &spriteList[i].oldRect);
		}

		for (i = 0; i < gTestBed.numSpritesInDemo; i++) {
			// Copy sprite to dock
			if (gTestBed.animationMethod == kCopyBitsRegionMethod)
				MoveRgnToRect(&spriteList[i].curRect, spriteList[i].region);
			(*gTestBed.drawer)(spriteList[i].buffer, gTestBed.dockBuffer, &spriteList[i].offBounds, &spriteList[i].curRect, spriteList[i].mask);
		}

		for (i = 0; i < gTestBed.numSpritesInDemo; i++) {
			// Copy dock to window
			(*gTestBed.refresher)(gTestBed.dockBuffer, gTestBed.outputBuffer, &spriteList[i].unionRect, &spriteList[i].unionRect);
		}

		/* Make sure bkgnd sound is continuous */
		if (gTestBed.useBkgndSound && theSound != NULL) {
			if (SndDone(0))
				PlayAsynch(theSound, 0);
		}

		if (CapsKeyDown())
			Delay(30, &dummy);
		
		if (frames == MAXFRAMES) break;
	}
	time = TickCount() - time;
	if (gTestBed.useBkgndSound && theSound != NULL) {
		SndStopSoftly(0);
		HUnlock(theSound);
		ReleaseResource(theSound);
		(void)DisposeSndChannel(0);
	}

	if (gTestBed.primaryDevice == GetMainDevice())
		ShowMenuBar();
	ShowCursor();

	HUnlock((Handle)sprites);
	DisposeHandle((Handle)sprites);
	DisposeGraphicsBuffer(gTestBed.outputBuffer);

	OutputSpeed(time, frames);

	while (!GetOSEvent(mDownMask + keyDownMask, &theEvent)) {}
	FillRect(&gTestBed.windowBounds, &qd.black);
} // END Animate

// ---------------------------------------------------------------------------

void SetupBuffers()
{
	GWorldPtr saveWorld; GDHandle saveDev;
	Handle bkgndPic;
	short id;

	GetGWorld(&saveWorld, &saveDev);

	switch(gTestBed.bufferDepth) {
		case 16: id = 128; break;
		default: id = 129; break;
	}
	bkgndPic = Get1Resource('PTRN', id);
	if (bkgndPic == NULL) {
		SysBeep(10); ExitToShell();
	}
	TilePict((PicHandle)bkgndPic, gTestBed.bkgndBuffer->gworld, &gTestBed.bufferBounds);
	CopyGraphicsBuffer(gTestBed.bkgndBuffer, gTestBed.dockBuffer,
		&gTestBed.bufferBounds, &gTestBed.bufferBounds);
	
	SetGWorld(saveWorld, saveDev);
} // END SetupBuffers

// ---------------------------------------------------------------------------

void OutputSpeed(long elapsedTime, long frames) {
	Str15 tempStr;
	long timeTicks;

	timeTicks = elapsedTime;
	if (elapsedTime >= 60)
		elapsedTime = ((float)frames / (float)((float)elapsedTime / (float)60.0));
	else
		elapsedTime = 0;

	SetPort(gTestBed.demoWindow);
	TextFont(geneva); TextSize(9); TextFace(bold); TextMode(srcCopy);

	NumToString(elapsedTime, tempStr);
	MoveTo(50, 50);
	DrawString("\pFrames per second: "); DrawString(tempStr);
	NumToString(frames, tempStr);
	MoveTo(50, 70);
	DrawString("\pFrames drawn: "); DrawString(tempStr);
	NumToString(timeTicks, tempStr);
	MoveTo(50, 90);
	DrawString("\pTicks total: "); DrawString(tempStr);
} // END OutputSpeed