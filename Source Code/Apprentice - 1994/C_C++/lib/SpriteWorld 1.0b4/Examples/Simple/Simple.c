///--------------------------------------------------------------------------------------
// Simple.c
//
// By: Tony Myles
//
// Copyright © 1993 Tony Myles, All rights reserved worldwide.
///--------------------------------------------------------------------------------------


#ifndef __DESK__
#include <Desk.h>
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

#ifndef __WINDOWS__
#include <Windows.h>
#endif

#ifndef __SPRITEWORLD__
#include <SpriteWorld.h>
#endif

#ifndef __SPRITELAYER__
#include <SpriteLayer.h>
#endif

#ifndef __SPRITE__
#include <Sprite.h>
#endif

#ifndef __FRAME__
#include <Frame.h>
#endif

#ifndef __SPRITEWORLDUTILS__
#include <SpriteWorldUtils.h>
#endif

#ifndef __APPLICATION__
#include "Application.h"
#endif

#ifndef __SIMPLE__
#include "Simple.h"
#endif


///--------------------------------------------------------------------------------------
// PerformSimpleAnimation
///--------------------------------------------------------------------------------------

void PerformSimpleAnimation(CWindowPtr srcWindowP)
{
	OSErr err;
	GrafPtr savePort;
	PixPatHandle pixPatH;
	SpriteWorldPtr spriteWorldP;
	SpriteLayerPtr spriteLayerP;
	SpritePtr simpleSpriteArray[kNumberOfSprites];
	SpritePtr simpleSpriteP;
	Rect moveBoundsRect;
	short spriteNum;

	GetPort(&savePort);
	SetPort((GrafPtr)srcWindowP);
	SetCursor(*GetCursor(watchCursor));


	//
	// STEP #1: initialize the sprite world package
	//

	err = SWEnterSpriteWorld();
	FatalError(err);


	//
	// STEP #2: create the various pieces that we need
	//

		// create the sprite world
	err = SWCreateSpriteWorldFromWindow(&spriteWorldP, srcWindowP, NULL);
	FatalError(err);

		// create the sprite layer
	err = SWCreateSpriteLayer(&spriteLayerP);
	FatalError(err);

		// create the first sprite
	err = SWCreateSpriteFromCIconResource(&simpleSpriteP, NULL, kSpriteCIconResID,
			kNumberOfSpriteFrames, kRegionMask);
	FatalError(err);
	simpleSpriteArray[0] = simpleSpriteP;

		// clone the rest of the sprites off the first one
	for (spriteNum = 1; spriteNum < kNumberOfSprites; spriteNum++)
	{
		err = SWCloneSprite(simpleSpriteP, simpleSpriteArray + spriteNum, NULL);
		FatalError(err);
	}


	//
	// STEP #3: put the pieces together (must be done BEFORE the sprite world is locked!)
	//

	for (spriteNum = 0; spriteNum < kNumberOfSprites; spriteNum++)
	{
			// add the sprite to the layer
		SWAddSprite(spriteLayerP, simpleSpriteArray[spriteNum]);
	}

		// add the layer to the world
	SWAddSpriteLayer(spriteWorldP, spriteLayerP);


	//
	// STEP #4: set things up for the animation
	//

		// calculate the movement boundary rectangle
	moveBoundsRect = spriteWorldP->windowFrameP->frameRect;

	for (spriteNum = 0; spriteNum < kNumberOfSprites; spriteNum++)
	{
		simpleSpriteP = simpleSpriteArray[spriteNum];

			// set up the sprite
		SWSetSpriteMoveBounds(simpleSpriteP, &moveBoundsRect);
		SWSetSpriteMoveTime(simpleSpriteP, kSpriteMoveTime);
		SWSetSpriteMoveProc(simpleSpriteP, SWBounceSpriteMoveProc);
		SWSetSpriteMoveDelta(simpleSpriteP, kHorizMoveDelta, kVertMoveDelta);

			// set the spriteÕs initial location
		SWSetSpriteLocation(simpleSpriteP, spriteNum * 60, spriteNum * 60);
	}


		// draw a nice background

	SetPort(spriteWorldP->backFrameP->framePort.monoGrafP);

	if (SWHasColorQuickDraw())
	{
		pixPatH = GetPixPat(kPixPatResID);

		if (pixPatH != NULL)
		{
			FillCRect(&moveBoundsRect, pixPatH);
			DisposePixPat(pixPatH);
		}
		else
		{
			FillRect(&moveBoundsRect, qd.ltGray);
		}
	}
	else
	{
		FillRect(&moveBoundsRect, qd.ltGray);
	}

	SetPort((GrafPtr)srcWindowP);


	//
	// STEP #5: lock the sprite world		!!! VERY IMPORTANT !!!
	//

	SWLockSpriteWorld(spriteWorldP);


	//
	// STEP #6: run the animation
	//

		// update the window
	SWUpdateSpriteWorld(spriteWorldP);
	HideCursor();

	while (!Button())
	{
		SWProcessSpriteWorld(spriteWorldP);
		SWAnimateSpriteWorld(spriteWorldP);

			// be friendly to drivers
		SystemTask();
	}


	//
	// STEP #7: unlock the sprite world
	//

	SWUnlockSpriteWorld(spriteWorldP);


	//
	// STEP #8: dispose of the pieces we created
	//

	for (spriteNum = 0; spriteNum < kNumberOfSprites; spriteNum++)
	{
		SWDisposeSprite(simpleSpriteArray[spriteNum]);
	}

	SWDisposeSpriteLayer(spriteLayerP);
	SWDisposeSpriteWorld(spriteWorldP);


	//
	// STEP #9: shut down the sprite world package
	//

	SWExitSpriteWorld();


	FlushEvents(everyEvent, 0);
	SetPort(savePort);
	ShowCursor();
	SetCursor(&qd.arrow);
}



