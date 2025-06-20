///--------------------------------------------------------------------------------------
// SpriteTest.c
//
// Created: 8/14/91 at 1:53 AM
// By: Tony Myles
//
// Copyright � 1991-94 Tony Myles, All rights reserved worldwide.
///--------------------------------------------------------------------------------------


#ifndef __SWCOMMON__
#include "SWCommonHeaders.h"
#endif

#ifndef __DESK__
#include <Desk.h>
#endif

#ifndef __PACKAGES__
#include <Packages.h>
#endif

#ifndef __ERRORS__
#include <Errors.h>
#endif

#ifndef __DIALOGS__
#include <Dialogs.h>
#endif

#ifndef __RESOURCES__
#include <Resources.h>
#endif

#ifndef __MEMORY__
#include <Memory.h>
#endif

#ifndef __EVENTS__
#include <Events.h>
#endif

#ifndef __OSEVENTS__
#include <OSEvents.h>
#endif

#ifndef __TOOLUTILS__
#include <ToolUtils.h>
#endif

#ifndef __SEGLOAD__
#include <SegLoad.h>
#endif

#ifndef __STANDARDFILE__
#include <StandardFile.h>
#endif

#ifndef __SPRITECOMPILER__
#include "SpriteCompiler.h"
#endif

#ifndef __SPRITEWORLD__
#include "SpriteWorld.h"
#endif

#ifndef __SPRITELAYER__
#include "SpriteLayer.h"
#endif

#ifndef __SPRITE__
#include "Sprite.h"
#endif

#ifndef __FRAME__
#include "Frame.h"
#endif

#ifndef __SPRITEWORLDUTILS__
#include "SpriteWorldUtils.h"
#endif

#include "GameUtils.h"
#include "DebugUtils.h"

#include "BlitPixie.h"
#include "SpriteTest.h"

RgnHandle gWorkRgn = NULL;


///--------------------------------------------------------------------------------------
// CreateSpriteTest
///--------------------------------------------------------------------------------------

OSErr CreateSpriteTest(
	SpriteTestPtr* spriteTestP,
	CWindowPtr srcWindowP)
{
	OSErr err = noErr;
	SpriteTestPtr tempSpriteTestP;
	SpriteWorldPtr spriteWorldP;
	SpriteLayerPtr spriteLayerP;
	SpritePtr testSpriteArray[kNumberOfTestSprites];
	SpritePtr titleSpriteP;
	FramePtr titleFrameP;
	PixPatHandle pixPatH;
	long spriteNum;
	short oldResRefNum, curResRefNum;
	Rect worldRect;

	*spriteTestP = NULL;

	gWorkRgn = NewRgn();

	SetPort((GrafPtr)srcWindowP);
	worldRect = srcWindowP->portRect;

	tempSpriteTestP = (SpriteTestPtr)NewPtrClear((Size)sizeof(SpriteTestRec));

	if (tempSpriteTestP == NULL)
	{
		err = MemError();
	}

	if (err == noErr)
	{
		tempSpriteTestP->isTestRunning = true;

		err = SWEnterSpriteWorld();		
	}

	if (err == noErr)
	{
			// create the sprite world
		err = SWCreateSpriteWorldFromWindow(&spriteWorldP, srcWindowP, NULL);
	}

	if (err == noErr)
	{
		tempSpriteTestP->spriteWorldP = spriteWorldP;

			// create the sprite layer
		err = SWCreateSpriteLayer(&spriteLayerP);
	}

	if (err == noErr)
	{
		oldResRefNum = CurResFile();
		curResRefNum = OpenResFile("\pSpriteTest Frames");
		err = ResError();
	}

	if (err == noErr)
	{
		UseResFile(curResRefNum);

		tempSpriteTestP->spriteLayerP = spriteLayerP;

		err = SWCreateSpriteFromCIconResource(
			testSpriteArray,
			NULL,
			kTestCIconID,
			kNumberOfTestFrames,
			kCompiledFatMask);

		UseResFile(oldResRefNum);
		CloseResFile(curResRefNum);
	}

	if (err == noErr)
	{
		tempSpriteTestP->testSpriteArray[0] = testSpriteArray[0];

			// create our sprites
		for (spriteNum = 1; spriteNum < kNumberOfTestSprites; spriteNum++)
		{
			err = SWCloneSprite(testSpriteArray[0], testSpriteArray + spriteNum, NULL);

			if (err == noErr)
			{
				tempSpriteTestP->testSpriteArray[spriteNum] = testSpriteArray[spriteNum];
			}
			else
			{
				break;
			}
		}
	}

	if (err == noErr)
	{
		err = SWCreateSprite(&titleSpriteP, NULL, 1);
	}

	if (err == noErr)
	{
		tempSpriteTestP->titleSpriteP = titleSpriteP;

			// create a frame for our sprite
		err = SWCreateFrameFromPictResource(&titleFrameP, 128, 129, kCompiledFatMask);
	}

	if (err == noErr)
	{
		tempSpriteTestP->titleFrameP = titleFrameP;
		*spriteTestP = tempSpriteTestP;

		SWAddFrame(titleSpriteP, titleFrameP);

		for (spriteNum = 0; spriteNum < kNumberOfTestSprites; spriteNum++)
		{
			if (spriteNum == (kNumberOfTestSprites / 2))
			{
				SWAddSprite(spriteLayerP, titleSpriteP);
			}

				// add the sprite in the layer
			SWAddSprite(spriteLayerP, testSpriteArray[spriteNum]);
		}

			// add the layer to the world
		SWAddSpriteLayer(spriteWorldP, spriteLayerP);

		SWLockSpriteWorld(spriteWorldP);

		SetupSpriteTest(tempSpriteTestP);

		SWSetPortToBackGround(spriteWorldP);

		if (SWHasColorQuickDraw() && ((**srcWindowP->portPixMap).pixelSize > 1))
		{
				// fill the sprite world with a pretty pattern
			pixPatH = GetPixPat(kBackDropPixPatID);

			if (pixPatH != NULL)
			{
				FillCRect(&worldRect, pixPatH);
				DisposePixPat(pixPatH);
			}
			else
			{
				#ifdef dangerousPattern
				FillRect(&worldRect, qd.ltGray);
				#else
				FillRect(&worldRect, &qd.ltGray);
				#endif
			}
		}
		else
		{
			#ifdef dangerousPattern
			FillRect(&worldRect, qd.ltGray);
			#else
			FillRect(&worldRect, &qd.ltGray);
			#endif
		}

		SetPort((GrafPtr)srcWindowP);
	}

	if (err != noErr)
	{
		DisposeSpriteTest(tempSpriteTestP);
	}
	
	return err;
}


void DisposeSpriteTest(
	SpriteTestPtr spriteTestP)
{
	SpriteLayerPtr curLayerP;
	SpritePtr curSpriteP;

	if (gWorkRgn != NULL)
		DisposeRgn(gWorkRgn);

	if (spriteTestP != NULL)
	{
		if (spriteTestP->spriteWorldP != NULL)
		{
			SWUnlockSpriteWorld(spriteTestP->spriteWorldP);

				// dispose of each sprite in the layer
			while ((curLayerP = SWGetNextSpriteLayer(spriteTestP->spriteWorldP, NULL)) != NULL)
			{
				while ((curSpriteP = SWGetNextSprite(curLayerP, NULL)) != NULL)
				{
					SWRemoveSprite(curLayerP, curSpriteP);
					SWDisposeSprite(curSpriteP);
				}
	
				SWRemoveSpriteLayer(spriteTestP->spriteWorldP, curLayerP);
				SWDisposeSpriteLayer(curLayerP);
			}
		}

		if (spriteTestP->spriteLayerP != NULL)
		{
			SWDisposeSpriteLayer(spriteTestP->spriteLayerP);
		}
	
		if (spriteTestP->spriteWorldP != NULL)
		{
			SWDisposeSpriteWorld(spriteTestP->spriteWorldP);
		}

		DisposePtr((Ptr)spriteTestP);
	}

	SWExitSpriteWorld();
}


void SetupSpriteTest(
	SpriteTestPtr spriteTestP)
{
	register long spriteNum;
	Rect moveBoundsRect;

	moveBoundsRect = spriteTestP->spriteWorldP->windowFrameP->frameRect;

		// set up the test sprites
	for (spriteNum = 0; spriteNum < kNumberOfTestSprites; spriteNum++)
	{
		SetupTestSprite(spriteTestP->testSpriteArray[spriteNum], &moveBoundsRect,
				GetRandom(0, moveBoundsRect.right),
				GetRandom(0, moveBoundsRect.bottom));
	}

		// set up the title sprite
	SWSetSpriteLocation(spriteTestP->titleSpriteP,
			(moveBoundsRect.right / 2) - (spriteTestP->titleFrameP->frameRect.right / 2),
			(moveBoundsRect.bottom / 2) - (spriteTestP->titleFrameP->frameRect.bottom / 2));
	SWSetSpriteMoveTime(spriteTestP->titleSpriteP, -1);

	spriteTestP->isCommandActive[kSpriteTestTitleCommand] = true;
	spriteTestP->isCommandActive[kBouncingBallsCommand] = true;
	spriteTestP->isCommandActive[kCollisionDetectionCommand] = false;
	spriteTestP->isCommandActive[kCopyBitsTestCommand] = true;
}


///--------------------------------------------------------------------------------------
// SetupTestSprite
///--------------------------------------------------------------------------------------

short gMoveDelta = 0;

void SetupTestSprite(
	SpritePtr testSpriteP,
	Rect *moveBoundsRect,
	short horizLocation,
	short vertLocation)
{
	Rect tempBoundsRect;
	short horizMoveDelta;
	short vertMoveDelta;

	gMoveDelta += 1;

	tempBoundsRect = *moveBoundsRect;
	horizMoveDelta = gMoveDelta;		//GetRandom(2, 6);
	vertMoveDelta =	gMoveDelta;		//GetRandom(2, 6);

		// calculate the movement boundary rectangle
	InsetRect(&tempBoundsRect, horizMoveDelta, vertMoveDelta);

	if (GetRandom(0, 1) == 1)
	{
		horizMoveDelta = -horizMoveDelta;
	}

	if (GetRandom(0, 1) == 1)
	{
		vertMoveDelta = -vertMoveDelta;
	}

		// set the sprite�s movement characteristics
	SWSetSpriteMoveBounds(testSpriteP, &tempBoundsRect);
	SWSetSpriteMoveDelta(testSpriteP, horizMoveDelta, vertMoveDelta);
	SWSetSpriteMoveProc(testSpriteP, SWBounceSpriteMoveProc);
	SWSetSpriteMoveTime(testSpriteP, kTestSpriteMoveTime);

	SWSetSpriteFrameTime(testSpriteP, kTestSpriteFrameTime);
	SWSetSpriteFrameRange(testSpriteP, 0, kNumberOfTestFrames - 1);
	SWSetSpriteFrameAdvance(testSpriteP, GetRandom(0, 1) ? -1 : 1);

	SWSetSpriteCollideProc(testSpriteP, SWBounceSpriteCollideProc);

		// set the sprite�s initial location
	SWSetSpriteLocation(testSpriteP, horizLocation, vertLocation);
}


///--------------------------------------------------------------------------------------
// SWBounceSpriteCollideProc
///--------------------------------------------------------------------------------------
 
void SWBounceSpriteCollideProc(
	SpritePtr srcSpriteP,
	SpritePtr dstSpriteP,
	Rect* sectRect)
{
	register short tempDelta;
	Rect rgnRect;
 
		// If both sprites use the same collision routine (this one),ignore the second collision.
	if ((!srcSpriteP->isVisible || !dstSpriteP->isVisible) ||
		((srcSpriteP->spriteCollideProc == dstSpriteP->spriteCollideProc) &&
		(srcSpriteP > dstSpriteP)))
		return;

	rgnRect = (**srcSpriteP->curFrameP->maskRgn).rgnBBox;

		// move the mask region to the new sprite location
	OffsetRgn(srcSpriteP->curFrameP->maskRgn,
				(srcSpriteP->destFrameRect.left - rgnRect.left) +
				srcSpriteP->curFrameP->offsetPoint.h,
				(srcSpriteP->destFrameRect.top - rgnRect.top) +
				srcSpriteP->curFrameP->offsetPoint.v);

	rgnRect = (**dstSpriteP->curFrameP->maskRgn).rgnBBox;

		// move the mask region to the new sprite location
	OffsetRgn(dstSpriteP->curFrameP->maskRgn,
				(dstSpriteP->destFrameRect.left - rgnRect.left) +
				dstSpriteP->curFrameP->offsetPoint.h,
				(dstSpriteP->destFrameRect.top - rgnRect.top) +
				dstSpriteP->curFrameP->offsetPoint.v);

	SectRgn(srcSpriteP->curFrameP->maskRgn, dstSpriteP->curFrameP->maskRgn, gWorkRgn);

	if (!EmptyRgn(gWorkRgn))
	{
		if ((dstSpriteP->horizMoveDelta == 0) && (dstSpriteP->vertMoveDelta == 0))
		{
			if (!((srcSpriteP->destFrameRect.left == sectRect->left) &&
				(srcSpriteP->destFrameRect.right == sectRect->right)))
			{
				srcSpriteP->horizMoveDelta = -srcSpriteP->horizMoveDelta;

				SWOffsetSprite(srcSpriteP, srcSpriteP->horizMoveDelta, 0);
			}

			if (!((srcSpriteP->destFrameRect.top == sectRect->top) &&
				(srcSpriteP->destFrameRect.bottom == sectRect->bottom)))
			{
				srcSpriteP->vertMoveDelta = -srcSpriteP->vertMoveDelta;

				SWOffsetSprite(srcSpriteP, 0, srcSpriteP->vertMoveDelta);
			}

			return;
		}

			// swap movement delta's
		if (srcSpriteP < dstSpriteP)		// Only one time!
		{
				// reverse spins.
			srcSpriteP->frameAdvance = -srcSpriteP->frameAdvance;
			dstSpriteP->frameAdvance = -dstSpriteP->frameAdvance;

				// swap h/v delta's
			tempDelta = srcSpriteP->horizMoveDelta;
			srcSpriteP->horizMoveDelta = dstSpriteP->horizMoveDelta;
			dstSpriteP->horizMoveDelta = tempDelta;

			tempDelta = srcSpriteP->vertMoveDelta;
			srcSpriteP->vertMoveDelta = dstSpriteP->vertMoveDelta;
			dstSpriteP->vertMoveDelta = tempDelta;
		}
	}
}
 
 
void RunSpriteTest(
	SpriteTestPtr spriteTestP)
{
	if (spriteTestP->isTestRunning)
	{
		SWLockSpriteWorld(spriteTestP->spriteWorldP);

		if (spriteTestP->isCommandActive[kCollisionDetectionCommand])
		{
			SWCollideSpriteLayer(spriteTestP->spriteLayerP, spriteTestP->spriteLayerP);
		}

		SWProcessSpriteWorld(spriteTestP->spriteWorldP);
		SWAnimateSpriteWorld(spriteTestP->spriteWorldP);

		SWUnlockSpriteWorld(spriteTestP->spriteWorldP);
	}
}


void UpdateSpriteTest(
	SpriteTestPtr spriteTestP)
{
	SWLockSpriteWorld(spriteTestP->spriteWorldP);
	SWUpdateSpriteWorld(spriteTestP->spriteWorldP);
	SWUnlockSpriteWorld(spriteTestP->spriteWorldP);
}


void HandleCreateSpriteCommand(
	SpriteTestPtr spriteTestP)
{
	OSErr err;
	SpritePtr testSpriteP;
	Rect moveBoundsRect;
	Point mouseLocation;

	err = SWCloneSprite(spriteTestP->testSpriteArray[0], &testSpriteP, NULL);

	if (err == noErr)
	{
		moveBoundsRect = spriteTestP->spriteWorldP->windowFrameP->frameRect;
		GetMouse(&mouseLocation);

		SetupTestSprite(testSpriteP, &moveBoundsRect, mouseLocation.h, mouseLocation.v);

		SWAddSprite(spriteTestP->spriteLayerP, testSpriteP);
	}
}


void HandleGetSpriteInfoCommand(
	SpriteTestPtr spriteTestP)
{
#if MPW
#pragma unused(spriteTestP)
#endif
}


void HandleRunPauseCommand(
	SpriteTestPtr spriteTestP)
{
	spriteTestP->isTestRunning = !spriteTestP->isTestRunning;
}


void HandleSpriteTestTitleCommand(
	SpriteTestPtr spriteTestP)
{
	spriteTestP->isCommandActive[kSpriteTestTitleCommand] =
		!spriteTestP->isCommandActive[kSpriteTestTitleCommand];

	if (spriteTestP->isCommandActive[kSpriteTestTitleCommand])
	{
		//SWAddSprite(spriteTestP->spriteLayerP, spriteTestP->titleSpriteP);
		SWSetSpriteVisible(spriteTestP->titleSpriteP, true);
	}
	else
	{
			// erase the sprite (theres got to be an easier way!)
		SWSetSpriteVisible(spriteTestP->titleSpriteP, false);
		//RunSpriteTest(spriteTestP);

			// remove the sprite
		//SWRemoveSprite(spriteTestP->spriteLayerP, spriteTestP->titleSpriteP);
	}
}


void HandleBouncingBallsCommand(
	SpriteTestPtr spriteTestP)
{
	SpritePtr ballSpriteP;
	Boolean isCommandActive;
	long ballNum;
	
	isCommandActive = spriteTestP->isCommandActive[kBouncingBallsCommand] =
		!spriteTestP->isCommandActive[kBouncingBallsCommand];

	for (ballNum = 0; ballNum < kNumberOfTestSprites; ballNum++)
	{
		ballSpriteP = spriteTestP->testSpriteArray[ballNum];

		SWSetSpriteVisible(ballSpriteP, isCommandActive);
	}
}


void CopyBitsSpeedTestCommand(
	SpriteTestPtr spriteTestP)
{
	WindowPtr testWindowP = FrontWindow();
	SpritePtr curSpriteP;
	unsigned long frames, seconds;
	long ticks;

	HideCursor();
	HideMenuBar(testWindowP);

	SWLockSpriteWorld(spriteTestP->spriteWorldP);
	SWUpdateSpriteWorld(spriteTestP->spriteWorldP);

	curSpriteP = NULL;

	while ((curSpriteP = SWGetNextSprite(spriteTestP->spriteLayerP, curSpriteP)) != NULL)
	{
		if (curSpriteP != spriteTestP->titleSpriteP)
		{
			SWSetSpriteMoveTime(curSpriteP, 0);
			SWSetSpriteFrameTime(curSpriteP, 0);
		}
	}

	ticks = TickCount();

	for (frames = 0; ((TickCount() - ticks) < kTestTime) && (!Button()); frames++)
	{
		SWProcessSpriteWorld(spriteTestP->spriteWorldP);
		SWAnimateSpriteWorld(spriteTestP->spriteWorldP);
	}

	seconds = ((TickCount() - ticks) / 60);

	curSpriteP = NULL;

	while ((curSpriteP = SWGetNextSprite(spriteTestP->spriteLayerP, curSpriteP)) != NULL)
	{
		if (curSpriteP != spriteTestP->titleSpriteP)
		{
			SWSetSpriteMoveTime(curSpriteP, kTestSpriteMoveTime);
			SWSetSpriteFrameTime(curSpriteP, kTestSpriteFrameTime);
		}
	}

	SWUnlockSpriteWorld(spriteTestP->spriteWorldP);

	ShowCursor();
	ShowMenuBar(testWindowP);

	DisplayPerformance(frames, seconds);
}


void BlitPixieSpeedTestCommand(
	SpriteTestPtr spriteTestP)
{
	WindowPtr testWindowP = FrontWindow();
	SpritePtr curSpriteP;
	unsigned long frames, seconds;
	long ticks;

	HideCursor();
	HideMenuBar(testWindowP);

	SWLockSpriteWorld(spriteTestP->spriteWorldP);
	SWUpdateSpriteWorld(spriteTestP->spriteWorldP);

	curSpriteP = NULL;

	while ((curSpriteP = SWGetNextSprite(spriteTestP->spriteLayerP, curSpriteP)) != NULL)
	{
		if (curSpriteP != spriteTestP->titleSpriteP)
		{
			SWSetSpriteMoveTime(curSpriteP, 0);
			SWSetSpriteFrameTime(curSpriteP, 0);
		}

		SWSetSpriteDrawProc(curSpriteP, BlitPixieMaskDrawProc);
	}

	ticks = TickCount();

#if SW_PPC
	for (frames = 0; (((*(long*)0x16A) - ticks) < kTestTime); frames++)
#else
	for (frames = 0; ((TickCount() - ticks) < kTestTime) && (!Button()); frames++)
#endif
	{
		SWProcessSpriteWorld(spriteTestP->spriteWorldP);
		SWBlastAnimateSpriteWorld(spriteTestP->spriteWorldP);
	}

	seconds = ((TickCount() - ticks) / 60);

	curSpriteP = NULL;

	while ((curSpriteP = SWGetNextSprite(spriteTestP->spriteLayerP, curSpriteP)) != NULL)
	{
		if (curSpriteP != spriteTestP->titleSpriteP)
		{
			SWSetSpriteMoveTime(curSpriteP, kTestSpriteMoveTime);
			SWSetSpriteFrameTime(curSpriteP, kTestSpriteFrameTime);
		}

		SWSetSpriteDrawProc(curSpriteP, SWStdDrawProc);
	}

	SWUnlockSpriteWorld(spriteTestP->spriteWorldP);

	ShowCursor();
	ShowMenuBar(testWindowP);

	DisplayPerformance(frames, seconds);
}


void SpriteTestCommand(
	SpriteTestPtr spriteTestP,
	CWindowPtr srcWindowP)
{
	short x, i;

	SetPort((GrafPtr)srcWindowP);
	HideMenuBar((WindowPtr)srcWindowP);

	for (x = 0; (x < 1000) && !Button(); x++)
	{
		#ifdef dangerousPattern
		FillRect(&srcWindowP->portRect, qd.black);
		#else
		FillRect(&srcWindowP->portRect, &qd.black);
		#endif

		DisposeSpriteTest(spriteTestP);
		CreateSpriteTest(&spriteTestP, srcWindowP);
		SetupSpriteTest(spriteTestP);
		UpdateSpriteTest(spriteTestP);

		for (i = 0; (i < 100) && !Button(); i++)
		{
			SystemTask();
			RunSpriteTest(spriteTestP);
		}
	}

	ShowMenuBar((WindowPtr)srcWindowP);
}


void CompileSpriteCommand(
	SpriteTestPtr spriteTestP)
{
#if MPW
#pragma unused(spriteTestP)
#endif
	OSErr err;
	StandardFileReply sfReply;
	SFTypeList sfTypes;
	short curResRefNum, oldResRefNum;
	PixelCodeHdl newPixCodeH;
	CWindowPtr testWindowP = (CWindowPtr)FrontWindow();
	Boolean isFileOpen = false;
	short pixCodeResID;
	short colorIconIndex, numberOfColorIcons;
	Str255 resName;
	ResType tempType;
	Handle tempIconH;

	StandardGetFile(NULL, -1, sfTypes, &sfReply);

	if (sfReply.sfGood)
	{
		oldResRefNum = CurResFile();
		err = ResError();

		if (err == noErr)
		{
			curResRefNum = FSpOpenResFile(&sfReply.sfFile, fsRdWrPerm);
			err = ResError();
		}

		if (err == noErr)
		{
			isFileOpen = true;
			UseResFile(curResRefNum);
			err = ResError();
		}

		if (err == noErr)
		{
			numberOfColorIcons = Count1Resources(kColorIconResType);
			err = ResError();
		}

		for (colorIconIndex = 1; (err == noErr) && (colorIconIndex <= numberOfColorIcons); colorIconIndex++)
		{
			tempIconH = Get1IndResource(kColorIconResType, colorIconIndex);

			if (tempIconH != NULL)
			{
				GetResInfo(tempIconH, &pixCodeResID, &tempType, resName);
				err = ResError();
			}
			else
			{
				err = ResError();

				if (err == noErr)
					err = resNotFound;
			}

			if (err == noErr)
			{
				err = SWCompileColorIconResource(pixCodeResID, &newPixCodeH);
			}

			if (err == noErr)
			{
				err = SWSavePixelCodeResource(newPixCodeH, pixCodeResID);

				DisposeHandle((Handle)newPixCodeH);
			}
			else
			{
				DebugStrNum("\pSWCompileColorIconResource", err);
			}
		}

		if (isFileOpen)
		{
			UseResFile(oldResRefNum);
			CloseResFile(curResRefNum);
		}

		if (err != noErr)
			DebugStrNum("\pCompileSpriteCommand", err);
	}
}


void RunCompiledSpriteCommand(
	SpriteTestPtr spriteTestP,
	Boolean useQuickDraw)
{
#if MPW
#pragma unused(useQuickDraw)
#endif
	WindowPtr testWindowP = FrontWindow();
	SpritePtr curSpriteP;
	unsigned long frames, seconds;
	long ticks;

	HideCursor();
	HideMenuBar(testWindowP);

	SWLockSpriteWorld(spriteTestP->spriteWorldP);
	SWUpdateSpriteWorld(spriteTestP->spriteWorldP);

	curSpriteP = NULL;

	while ((curSpriteP = SWGetNextSprite(spriteTestP->spriteLayerP, curSpriteP)) != NULL)
	{
		if (curSpriteP != spriteTestP->titleSpriteP)
		{
			SWSetSpriteMoveTime(curSpriteP, 0);
			SWSetSpriteFrameTime(curSpriteP, 0);
			SWSetSpriteDrawProc(curSpriteP, CompiledSpriteDrawProc);
		}
		else
			SWSetSpriteDrawProc(spriteTestP->titleSpriteP, BlitPixieMaskDrawProc);
	}


	#if !SW_PPC
	FlushInstructionCache();
	#endif

	ticks = TickCount();

	for (frames = 0; ((TickCount() - ticks) < kTestTime) && (!Button()); frames++)
	{
		SWProcessSpriteWorld(spriteTestP->spriteWorldP);
		SWBlastAnimateSpriteWorld(spriteTestP->spriteWorldP);
	}

	seconds = ((TickCount() - ticks) / 60);


	curSpriteP = NULL;

	while ((curSpriteP = SWGetNextSprite(spriteTestP->spriteLayerP, curSpriteP)) != NULL)
	{
		if (curSpriteP != spriteTestP->titleSpriteP)
		{
			SWSetSpriteMoveTime(curSpriteP, kTestSpriteMoveTime);
			SWSetSpriteFrameTime(curSpriteP, kTestSpriteFrameTime);
			SWSetSpriteDrawProc(curSpriteP, SWStdDrawProc);
		}
		else
			SWSetSpriteDrawProc(spriteTestP->titleSpriteP, SWStdDrawProc);
	}


	SWUnlockSpriteWorld(spriteTestP->spriteWorldP);

	ShowCursor();
	ShowMenuBar(testWindowP);

	DisplayPerformance(frames, seconds);
}


void DisplayPerformance(
	long frames,
	long seconds)
{
	Str255 framesString, secondsString, fpsString;
	long fps;
	
	NumToString(frames, framesString);
	NumToString(seconds, secondsString);

	fps = (seconds > 0) ? frames / seconds : frames;
	NumToString(fps, fpsString);

	ParamText(framesString, secondsString, fpsString, "\p");
	NoteAlert(kPerformanceAlertID, NULL);
}


