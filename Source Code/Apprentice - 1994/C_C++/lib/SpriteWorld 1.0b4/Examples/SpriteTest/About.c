///--------------------------------------------------------------------------------------
//	About.c
//
//	By:		Tony Myles
//
//	Copyright: © 1993-94 Tony Myles, All rights reserved worldwide.
///--------------------------------------------------------------------------------------


#ifndef __RESOURCES__
#include <Resources.h>
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

#ifndef __DIALOGUTILS__
#include "DialogUtils.h"
#endif

#ifndef __SPRITETEST__
#include "SpriteTest.h"
#endif

#ifndef __APPLICATION__
#include "Application.h"
#endif

#ifndef __ABOUT__
#include "About.h"
#endif


extern SpriteTestPtr gSpriteTestP;



///--------------------------------------------------------------------------------------
// DisplayAboutBox
///--------------------------------------------------------------------------------------

void DisplayAboutBox(void)
{
	OSErr err;
	GrafPtr savePort;
	DialogPtr aboutDialogP;
	SpriteWorldPtr spriteWorldP = NULL;
	SpriteLayerPtr spriteLayerP = NULL;
	SpritePtr earthSpriteP = NULL, shadowSpriteP = NULL;
	Rect itemRect, moveBoundsRect;
	short itemHit;
#if SW_PPC
	ModalFilterUPP spriteWorldDialogFilter;
#else
	ModalFilterProcPtr spriteWorldDialogFilter;
#endif

	aboutDialogP = GetNewDialog(kAboutDialogID, NULL, (WindowPtr)-1L);
	
	if (aboutDialogP != NULL)
	{
		GetPort(&savePort);
		SetPort(aboutDialogP);

			// create the sprite world
		err = SWCreateSpriteWorldFromWindow(&spriteWorldP, (CWindowPtr)aboutDialogP, NULL);
	
		if (err == noErr)
		{	
				// create the sprite layer
			err = SWCreateSpriteLayer(&spriteLayerP);
		}

		if (err == noErr)
		{
			err = SWCreateSpriteFromCIconResource(&earthSpriteP, NULL, kEarthCIconID, 1, kRegionMask);
		}

		if (err == noErr)
		{
			err = SWCreateSpriteFromCIconResource(&shadowSpriteP, NULL, kShadowCIconID,
															kNumberOfShadowFrames, kRegionMask);
		}

		if (err == noErr)
		{
			shadowSpriteP->userData = (long)earthSpriteP;

			SWAddSprite(spriteLayerP, shadowSpriteP);
			SWAddSprite(spriteLayerP, earthSpriteP);
			SWAddSpriteLayer(spriteWorldP, spriteLayerP);

			GetDItemRect(aboutDialogP, 4, &itemRect);
			moveBoundsRect = aboutDialogP->portRect;
			moveBoundsRect.bottom = itemRect.top - (kMaxSpeed / 2);

				// set the spriteÕs movement characteristics
			SWSetSpriteMoveBounds(earthSpriteP, &moveBoundsRect);
			SWSetSpriteMoveDelta(earthSpriteP, 0, kInitialSpeed);
			SWSetSpriteMoveProc(earthSpriteP, EarthMoveProc);
			SWSetSpriteMoveTime(earthSpriteP, 55);
			SWSetSpriteMoveTime(shadowSpriteP, -1);		// never move
			SWSetSpriteFrameTime(shadowSpriteP, 0);
			SWSetSpriteFrameRange(shadowSpriteP, 0, 2);
			SWSetSpriteFrameProc(shadowSpriteP, ShadowFrameProc);
			SWSetCurrentFrameIndex(shadowSpriteP, 2);
			SWSetSpriteMoveBounds(shadowSpriteP, &moveBoundsRect);

				// set the spriteÕs initial location
			SWSetSpriteLocation(shadowSpriteP, itemRect.left, itemRect.top);
			GetDItemRect(aboutDialogP, 5, &itemRect);
			SWSetSpriteLocation(earthSpriteP, itemRect.left, itemRect.top);

			((WindowPeek)aboutDialogP)->refCon = (long)spriteWorldP;

			ShowWindow(aboutDialogP);
			SetPort(aboutDialogP);
			DrawDialog(aboutDialogP);
			EraseRect(&itemRect);
			OutlineDefaultButton(aboutDialogP, ok);
			ValidRect(&aboutDialogP->portRect);

			SWLockSpriteWorld(spriteWorldP);

			CopyBits(&aboutDialogP->portBits,
						spriteWorldP->backFrameP->framePix.bitMapP,
						&aboutDialogP->portRect,
						&aboutDialogP->portRect,
						srcCopy, NULL);


			spriteWorldDialogFilter = NewModalFilterProc(AboutDialogFilter);

			do
			{
				ModalDialog(spriteWorldDialogFilter, &itemHit);
			} while (itemHit != ok);

#if SW_PPC
			DisposeRoutineDescriptor(spriteWorldDialogFilter);
#endif

			SWUnlockSpriteWorld(spriteWorldP);
		}

		DisposeDialog(aboutDialogP);

		if (earthSpriteP != NULL)
			SWDisposeSprite(earthSpriteP);

		if (shadowSpriteP != NULL)
			SWDisposeSprite(shadowSpriteP);

		if (spriteLayerP != NULL)
			SWDisposeSpriteLayer(spriteLayerP);

		if (spriteWorldP != NULL)
			SWDisposeSpriteWorld(spriteWorldP);

		SetPort(savePort);

		if (err != noErr)
		{
			ErrorAlert(err, kUnknownErrorStringIndex);
		}
	}
	else
	{
		ErrorAlert(ResError(), kCantFindResourceStringIndex);
	}
}


///--------------------------------------------------------------------------------------
// EarthMoveProc
///--------------------------------------------------------------------------------------

SW_FUNC void EarthMoveProc(
	SpritePtr srcSpriteP,
	Point* spritePoint)
{
		// gravity
	if (srcSpriteP->vertMoveDelta < (short)kMaxSpeed)
		srcSpriteP->vertMoveDelta++;

		// bounce
	if (srcSpriteP->destFrameRect.bottom > srcSpriteP->moveBoundsRect.bottom)
	{
		srcSpriteP->vertMoveDelta = -srcSpriteP->vertMoveDelta;
		
		spritePoint->v = srcSpriteP->moveBoundsRect.bottom -
			(srcSpriteP->curFrameP->frameRect.bottom - srcSpriteP->curFrameP->frameRect.top);
	}
}


///--------------------------------------------------------------------------------------
// ShadowFrameProc
///--------------------------------------------------------------------------------------

SW_FUNC void ShadowFrameProc(
	SpritePtr srcSpriteP,
	FramePtr curFrameP,
	long* curFrameIndex)
{
#if MPW
#pragma unused(curFrameP)
#endif

	SpritePtr earthSpriteP = (SpritePtr)srcSpriteP->userData;
	short distanceFromTop = earthSpriteP->destFrameRect.top - srcSpriteP->moveBoundsRect.top;
	short height = srcSpriteP->moveBoundsRect.bottom - srcSpriteP->moveBoundsRect.top;

	*curFrameIndex = (long)((distanceFromTop * 3) / height);

		// lets not index past the last frame!
		// (even though SpriteWorld will not allow it to happen)
	if (*curFrameIndex > 2L) *curFrameIndex = 2L;
}


///--------------------------------------------------------------------------------------
// AboutDialogFilter
///--------------------------------------------------------------------------------------

pascal Boolean AboutDialogFilter(
	DialogPtr aboutDialogP,
	EventRecord *event,
	short *itemHit)
{
	Boolean eventHandled = false;
	SpriteWorldPtr spriteWorldP = (SpriteWorldPtr)((WindowPeek)aboutDialogP)->refCon;

	switch (event->what)
	{
		case nullEvent:
		{
				// run the about box animation
			SWProcessSpriteWorld(spriteWorldP);
			SWAnimateSpriteWorld(spriteWorldP);

				// keep the title animation going!
			RunSpriteTest(gSpriteTestP);
			break;
		}

		case keyDown:
		case autoKey:
		{
			char key = (char)(event->message & charCodeMask);

			if (key == kReturnChar || (key == kEnterChar))
			{
				*itemHit = ok;
				eventHandled = true;
				ClickDialogButton(aboutDialogP, ok);
			}

			break;
		}

		case updateEvt:
		{
			if ((DialogPtr)event->message == aboutDialogP)
			{
				SetPort(aboutDialogP);
				BeginUpdate(aboutDialogP);

				SWUpdateSpriteWorld(spriteWorldP);
				
				EndUpdate(aboutDialogP);
				eventHandled = true;
			}
			else
			{
				HandleUpdateEvent((WindowPtr)event->message);
			}

			break;
		}
	}

	return eventHandled;
}


