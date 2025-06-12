///--------------------------------------------------------------------------------------
//	SpriteWorld.c
//
//	Created:	Wednesday, May 29, 1991 at 10:43:28 PM
//	By:		Tony Myles
//
//	Copyright: © 1991-94 Tony Myles, All rights reserved worldwide.
//
//	Description:	implementation of the sprite world architecture
///--------------------------------------------------------------------------------------


#ifndef __SWCOMMON__
#include "SWCommonHeaders.h"
#endif

#ifndef __QUICKDRAW__
#include <QuickDraw.h>
#endif

#ifndef __MEMORY__
#include <Memory.h>
#endif

#ifndef __GESTALTEQU__
#include <GestaltEqu.h>
#endif

#ifndef __SPRITEWORLDUTILS__
#include "SpriteWorldUtils.h"
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

#ifndef __BLITPIXIE__
#include "BlitPixie.h"
#endif

#if MPW
#pragma segment SpriteWorld
#endif

char gSWmmuMode;


///--------------------------------------------------------------------------------------
//	SWEnterSpriteWorld
///--------------------------------------------------------------------------------------

SW_FUNC OSErr SWEnterSpriteWorld(void)
{
	OSErr err;
	long	versionNumber;

		// make sure we can run in this environment
	err = Gestalt(gestaltTimeMgrVersion, &versionNumber);

	if ((err != noErr) || (versionNumber < gestaltStandardTimeMgr))
	{
		err = kTimeMgrNotPresentErr;
	}

	if (err == noErr)
	{
		gSWmmuMode = GetMMUMode();
	}

	return err;
}


///--------------------------------------------------------------------------------------
//	SWExitSpriteWorld
///--------------------------------------------------------------------------------------

SW_FUNC void SWExitSpriteWorld(void)
{
	// nothing happens here right now, but that might change later
}


///--------------------------------------------------------------------------------------
//	SWCreateSpriteWorld
///--------------------------------------------------------------------------------------

SW_FUNC OSErr SWCreateSpriteWorld(
	SpriteWorldPtr *spriteWorldP,
	FramePtr windowFrameP,
	FramePtr backFrameP,
	FramePtr loadFrameP)
{
	OSErr err;
	SpriteWorldPtr tempSpriteWorldP;

	err = noErr;
	*spriteWorldP = NULL;

	tempSpriteWorldP = (SpriteWorldPtr)NewPtrClear((Size)sizeof(SpriteWorldRec));

	if (tempSpriteWorldP != NULL)
	{
		tempSpriteWorldP->windowFrameP = windowFrameP;
		tempSpriteWorldP->backFrameP = backFrameP;
		tempSpriteWorldP->loadFrameP = loadFrameP;
		tempSpriteWorldP->eraseDrawProc = SWStdWorldDrawProc;
		tempSpriteWorldP->screenDrawProc = SWStdWorldDrawProc;

		*spriteWorldP = tempSpriteWorldP;
	}
	else
	{
		err = MemError();
	}

	return err;
}


///--------------------------------------------------------------------------------------
//	SWCreateSpriteWorldFromWindow
///--------------------------------------------------------------------------------------

SW_FUNC OSErr SWCreateSpriteWorldFromWindow(
	SpriteWorldPtr* spriteWorldP,
	CWindowPtr srcWindowP,
	Rect* worldRect)
{
	OSErr err;
	CGrafPtr savePort;
	FramePtr windowFrameP, backFrameP, loadFrameP;
	Rect tempRect;

	*spriteWorldP = NULL;
	windowFrameP = backFrameP = loadFrameP = NULL;

	tempRect = (worldRect == NULL) ? srcWindowP->portRect : *worldRect;

	GetPort((GrafPtr*)&savePort);
	SetPort((GrafPtr)srcWindowP);

		// create window frame
	err = SWCreateFrame(&windowFrameP, srcWindowP, &tempRect);

	if (err == noErr)
	{
			// create back drop frame
		err = SWCreateFrame(&backFrameP, NULL, &tempRect);
	}

	if (err == noErr)
	{
			// create loader frame
		err = SWCreateFrame(&loadFrameP, NULL, &tempRect);
	}

	if (err == noErr)
	{
			// create sprite world
		err = SWCreateSpriteWorld(spriteWorldP, windowFrameP, backFrameP, loadFrameP);
	}

	if (err != noErr)
	{
			// an error occurred so dispose of anything we managed to create

		if (windowFrameP != NULL)
		{
			SWDisposeFrame(windowFrameP);
		}

		if (backFrameP != NULL)
		{
			SWDisposeFrame(backFrameP);
		}

		if (loadFrameP != NULL)
		{
			SWDisposeFrame(loadFrameP);
		}
	}

	SetPort((GrafPtr)savePort);

	return err;
}


///--------------------------------------------------------------------------------------
//	SWDisposeSpriteWorld
///--------------------------------------------------------------------------------------

SW_FUNC void SWDisposeSpriteWorld(
	SpriteWorldPtr spriteWorldP)
{
	if (spriteWorldP != NULL)
	{
		SWDisposeFrame(spriteWorldP->backFrameP);

		SWDisposeFrame(spriteWorldP->loadFrameP);

		spriteWorldP->windowFrameP->framePort.colorGrafP = NULL;
		SWDisposeFrame(spriteWorldP->windowFrameP);

		DisposePtr((Ptr)spriteWorldP);
	}
}


///--------------------------------------------------------------------------------------
//	SWAddSpriteLayer
///--------------------------------------------------------------------------------------

SW_FUNC void SWAddSpriteLayer(
	SpriteWorldPtr spriteWorldP,
	SpriteLayerPtr newSpriteLayerP)
{
	SpriteLayerPtr tailSpriteLayerP = spriteWorldP->tailSpriteLayerP;

	if (tailSpriteLayerP != NULL)
	{
			// doubly link the new layer
		tailSpriteLayerP->nextSpriteLayerP = newSpriteLayerP;
		newSpriteLayerP->prevSpriteLayerP = tailSpriteLayerP;
		newSpriteLayerP->nextSpriteLayerP = NULL;
	}	
	else
	{
		newSpriteLayerP->prevSpriteLayerP = NULL;
		newSpriteLayerP->nextSpriteLayerP = NULL;

			// make the new layer the head
		spriteWorldP->headSpriteLayerP = newSpriteLayerP;
	}

		// make the new layer the tail
	spriteWorldP->tailSpriteLayerP = newSpriteLayerP;
}


///--------------------------------------------------------------------------------------
//	SWRemoveSpriteLayer
///--------------------------------------------------------------------------------------

SW_FUNC void SWRemoveSpriteLayer(
	SpriteWorldPtr spriteWorldP,
	SpriteLayerPtr oldSpriteLayerP)
{
		// is there a next layer?
	if (oldSpriteLayerP->nextSpriteLayerP != NULL)
	{
			// link the next layer to the prev layer
		oldSpriteLayerP->nextSpriteLayerP->prevSpriteLayerP = oldSpriteLayerP->prevSpriteLayerP;
	}
	else
	{
			// make the prev layer the tail
		spriteWorldP->tailSpriteLayerP = oldSpriteLayerP->prevSpriteLayerP;
	}

		// is there a prev layer?
	if (oldSpriteLayerP->prevSpriteLayerP != NULL)
	{
			// link the prev layer to the next layer
		oldSpriteLayerP->prevSpriteLayerP->nextSpriteLayerP = oldSpriteLayerP->nextSpriteLayerP;
	}
	else
	{
			// make the next layer the head
		spriteWorldP->headSpriteLayerP = oldSpriteLayerP->nextSpriteLayerP;
	}
}


///--------------------------------------------------------------------------------------
//	SWSwapSpriteLayer
///--------------------------------------------------------------------------------------

SW_FUNC void SWSwapSpriteLayer(
	SpriteWorldPtr spriteWorldP,
	SpriteLayerPtr srcSpriteLayerP,
	SpriteLayerPtr dstSpriteLayerP)
{
	register SpriteLayerPtr swapSpriteLayerP;
	
	swapSpriteLayerP = srcSpriteLayerP->nextSpriteLayerP;
	srcSpriteLayerP->nextSpriteLayerP = dstSpriteLayerP->nextSpriteLayerP;
	dstSpriteLayerP->nextSpriteLayerP = swapSpriteLayerP;

	swapSpriteLayerP = srcSpriteLayerP->prevSpriteLayerP;
	srcSpriteLayerP->prevSpriteLayerP = dstSpriteLayerP->prevSpriteLayerP;
	dstSpriteLayerP->prevSpriteLayerP = swapSpriteLayerP;

	if (srcSpriteLayerP->nextSpriteLayerP == NULL)
	{
		spriteWorldP->tailSpriteLayerP = srcSpriteLayerP;
	}
	else if (srcSpriteLayerP->prevSpriteLayerP == NULL)
	{
		spriteWorldP->headSpriteLayerP = srcSpriteLayerP;
	}

	if (dstSpriteLayerP->nextSpriteLayerP == NULL)
	{
		spriteWorldP->tailSpriteLayerP = dstSpriteLayerP;
	}
	else if (dstSpriteLayerP->prevSpriteLayerP == NULL)
	{
		spriteWorldP->headSpriteLayerP = dstSpriteLayerP;
	}
}


///--------------------------------------------------------------------------------------
//	SWGetNextSpriteLayer
///--------------------------------------------------------------------------------------

SW_FUNC SpriteLayerPtr SWGetNextSpriteLayer(
	SpriteWorldPtr spriteWorldP,
	SpriteLayerPtr curSpriteLayerP)
{
	return (curSpriteLayerP == NULL) ?
			spriteWorldP->headSpriteLayerP :
			curSpriteLayerP->nextSpriteLayerP;
}


///--------------------------------------------------------------------------------------
//	SWLockSpriteWorld
///--------------------------------------------------------------------------------------

SW_FUNC void SWLockSpriteWorld(
	SpriteWorldPtr spriteWorldP)
{
	SpriteLayerPtr curSpriteLayerP;

	SWLockFrame(spriteWorldP->windowFrameP);
	SWLockFrame(spriteWorldP->backFrameP);
	SWLockFrame(spriteWorldP->loadFrameP);

	curSpriteLayerP = spriteWorldP->headSpriteLayerP;

	while (curSpriteLayerP != NULL)
	{
		SWLockSpriteLayer(curSpriteLayerP);

		curSpriteLayerP = curSpriteLayerP->nextSpriteLayerP;
	}
}


///--------------------------------------------------------------------------------------
//	SWUnlockSpriteWorld
///--------------------------------------------------------------------------------------

SW_FUNC void SWUnlockSpriteWorld(
	SpriteWorldPtr spriteWorldP)
{
	SpriteLayerPtr curSpriteLayerP;

	SWUnlockFrame(spriteWorldP->windowFrameP);
	SWUnlockFrame(spriteWorldP->backFrameP);
	SWUnlockFrame(spriteWorldP->loadFrameP);

	curSpriteLayerP = spriteWorldP->headSpriteLayerP;

	while (curSpriteLayerP != NULL)
	{
		SWUnlockSpriteLayer(curSpriteLayerP);

		curSpriteLayerP = curSpriteLayerP->nextSpriteLayerP;
	}
}


///--------------------------------------------------------------------------------------
//	SWSetPortToBackGround
///--------------------------------------------------------------------------------------

SW_FUNC void SWSetPortToBackGround(
	SpriteWorldPtr spriteWorldP)
{
	SetPort(spriteWorldP->backFrameP->framePort.monoGrafP);
}


///--------------------------------------------------------------------------------------
//	SWSetPortToWindow
///--------------------------------------------------------------------------------------

SW_FUNC void SWSetPortToWindow(
	SpriteWorldPtr spriteWorldP)
{
	SetPort(spriteWorldP->windowFrameP->framePort.monoGrafP);
}


///--------------------------------------------------------------------------------------
//	SWSetSpriteWorldEraseProc
///--------------------------------------------------------------------------------------

SW_FUNC void SWSetSpriteWorldEraseProc(
	SpriteWorldPtr spriteWorldP,
	WorldDrawProcPtr eraseProc)
{
	spriteWorldP->eraseDrawProc = eraseProc;
}


///--------------------------------------------------------------------------------------
//	SWSetSpriteWorldDrawProc
///--------------------------------------------------------------------------------------

SW_FUNC void SWSetSpriteWorldDrawProc(
	SpriteWorldPtr spriteWorldP,
	WorldDrawProcPtr drawProc)
{
	spriteWorldP->screenDrawProc = drawProc;
}


///--------------------------------------------------------------------------------------
//	SWStdWorldDrawProc
///--------------------------------------------------------------------------------------

SW_FUNC void SWStdWorldDrawProc(
	FramePtr srcFrameP,
	FramePtr dstFrameP,
	Rect* drawRect)
{
	CopyBits((BitMapPtr)srcFrameP->framePix.pixMapP, (BitMapPtr)dstFrameP->framePix.pixMapP,
				drawRect, drawRect, srcCopy, NULL);
}


///--------------------------------------------------------------------------------------
//	SWUpdateSpriteWorld
///--------------------------------------------------------------------------------------

SW_FUNC void SWUpdateSpriteWorld(
	SpriteWorldPtr spriteWorldP)
{
	register SpriteLayerPtr curSpriteLayerP;
	register SpritePtr curSpriteP;
	Rect rgnRect;

		// the current port should the one in which we are drawing    
	SetPort(spriteWorldP->loadFrameP->framePort.monoGrafP);

	(*spriteWorldP->eraseDrawProc)(spriteWorldP->backFrameP,
								spriteWorldP->loadFrameP,
								&spriteWorldP->loadFrameP->frameRect);

	curSpriteLayerP = spriteWorldP->headSpriteLayerP;

		// iterate through the layers in this world
	while (curSpriteLayerP != NULL)
	{
		curSpriteP = curSpriteLayerP->headSpriteP;

			// iterate through the sprites in this layer
		while (curSpriteP != NULL)
		{
			if (curSpriteP->isVisible)
			{
				if (curSpriteP->curFrameP->maskRgn != NULL)
				{
					rgnRect = (**curSpriteP->curFrameP->maskRgn).rgnBBox;

						// move the mask region to the new sprite location
					OffsetRgn(
						curSpriteP->curFrameP->maskRgn,
						(curSpriteP->destFrameRect.left - rgnRect.left) +
						curSpriteP->curFrameP->offsetPoint.h,
						(curSpriteP->destFrameRect.top - rgnRect.top) +
						curSpriteP->curFrameP->offsetPoint.v);
				}

					// copy the sprite image onto the back drop piece
				(*curSpriteP->frameDrawProc)(curSpriteP->curFrameP,
											spriteWorldP->loadFrameP,
											&curSpriteP->curFrameP->frameRect,
											&curSpriteP->destFrameRect);
			}

			curSpriteP = curSpriteP->nextSpriteP;
		}

		curSpriteLayerP = curSpriteLayerP->nextSpriteLayerP;
	}

			// the current port should the one in which we are drawing    
	SetPort(spriteWorldP->windowFrameP->framePort.monoGrafP);

	(*spriteWorldP->screenDrawProc)(spriteWorldP->loadFrameP,
									spriteWorldP->windowFrameP,
									&spriteWorldP->windowFrameP->frameRect);
}


///--------------------------------------------------------------------------------------
//	SWProcessSpriteWorld
///--------------------------------------------------------------------------------------

SW_FUNC void SWProcessSpriteWorld(
	SpriteWorldPtr spriteWorldP)
{
	register SpriteLayerPtr curSpriteLayerP;
	SpriteLayerPtr nextSpriteLayerP;
	register SpritePtr curSpriteP;
	SpritePtr nextSpriteP;
	register FramePtr oldFrameP, newFrameP;
	Point spritePoint, oldPoint;
	short horizOffset, vertOffset;

	curSpriteLayerP = spriteWorldP->headSpriteLayerP;	

		// iterate through the layers in this world
	while (curSpriteLayerP != NULL)
	{
		nextSpriteLayerP = curSpriteLayerP->nextSpriteLayerP;
		curSpriteP = curSpriteLayerP->headSpriteP;

			// iterate through the sprites in this layer
		while (curSpriteP != NULL)
		{
			nextSpriteP = curSpriteP->nextSpriteP;

				// is it time to advance the spriteÕs frame?
			if (curSpriteP->frameTimeTask.hasTaskFired)
			{
				curSpriteP->curFrameIndex += curSpriteP->frameAdvance;

				if (curSpriteP->curFrameIndex < curSpriteP->firstFrameIndex)
				{
						// wrap to the last frame
					curSpriteP->curFrameIndex = curSpriteP->lastFrameIndex;
				}
				else if (curSpriteP->curFrameIndex > curSpriteP->lastFrameIndex)
				{
						// wrap to the first frame
					curSpriteP->curFrameIndex = curSpriteP->firstFrameIndex;
				}

					// get new frame
				newFrameP =	curSpriteP->frameArray[curSpriteP->curFrameIndex];

					// is there a frame callback?
				if (curSpriteP->frameChangeProc != NULL)
				{
						// call it
					(*curSpriteP->frameChangeProc)(curSpriteP, newFrameP,
							&curSpriteP->curFrameIndex);

						// make sure the new frame index is in range
					if (curSpriteP->curFrameIndex < 0)
					{
						curSpriteP->curFrameIndex = 0;
					}
					else if (curSpriteP->curFrameIndex >= curSpriteP->maxFrames)
					{
						curSpriteP->curFrameIndex = curSpriteP->maxFrames - 1;
					}
				}

					// change the frame
				newFrameP =	curSpriteP->frameArray[curSpriteP->curFrameIndex];

					// has the frame actually changed?
				if (curSpriteP->curFrameP != newFrameP)
				{
					oldFrameP = curSpriteP->curFrameP;
					curSpriteP->curFrameP = newFrameP;

					horizOffset = (curSpriteP->destFrameRect.left - newFrameP->frameRect.left);
					vertOffset = (curSpriteP->destFrameRect.top - newFrameP->frameRect.top);

					curSpriteP->destFrameRect = newFrameP->frameRect;

					curSpriteP->destFrameRect.left += horizOffset;
					curSpriteP->destFrameRect.right += horizOffset;
					curSpriteP->destFrameRect.top += vertOffset;
					curSpriteP->destFrameRect.bottom += vertOffset;

					curSpriteP->needsToBeDrawn = true;
				}

				if (curSpriteP->frameTimeInterval > 0)
				{
					curSpriteP->frameTimeTask.hasTaskFired = false;

						// reset the time till next frame
					PrimeTime((QElemPtr)&curSpriteP->frameTimeTask, curSpriteP->frameTimeInterval);
				}
			}

				// is it time to move the sprite?
			if (curSpriteP->moveTimeTask.hasTaskFired)
			{
					// is there a movement callback?
				if (curSpriteP->spriteMoveProc != NULL)
				{
					oldPoint = *(Point*)&(curSpriteP->destFrameRect);
					spritePoint = oldPoint;
					spritePoint.h += curSpriteP->horizMoveDelta;
					spritePoint.v += curSpriteP->vertMoveDelta;

						// call it
					(*curSpriteP->spriteMoveProc)(curSpriteP, &spritePoint);

					if ((spritePoint.h != oldPoint.h) || (spritePoint.v != oldPoint.v))
					{
						curSpriteP->destFrameRect.right  = (curSpriteP->destFrameRect.right -
															curSpriteP->destFrameRect.left) +
															spritePoint.h;
						curSpriteP->destFrameRect.bottom = (curSpriteP->destFrameRect.bottom -
															curSpriteP->destFrameRect.top) +
															spritePoint.v;
						curSpriteP->destFrameRect.left = spritePoint.h;
						curSpriteP->destFrameRect.top = spritePoint.v;

						curSpriteP->needsToBeDrawn = true;
					}
				}
				else if ((curSpriteP->horizMoveDelta != 0) || (curSpriteP->vertMoveDelta != 0))
				{
						// offset destination rect (way faster than OffsetRect)
					curSpriteP->destFrameRect.top += curSpriteP->vertMoveDelta;
					curSpriteP->destFrameRect.left += curSpriteP->horizMoveDelta;
					curSpriteP->destFrameRect.bottom += curSpriteP->vertMoveDelta;
					curSpriteP->destFrameRect.right += curSpriteP->horizMoveDelta;

					curSpriteP->needsToBeDrawn = true;
				}

				if (curSpriteP->moveTimeInterval > 0)
				{
					curSpriteP->moveTimeTask.hasTaskFired = false;

						// reset the time till next move
					PrimeTime((QElemPtr)&curSpriteP->moveTimeTask,
							curSpriteP->moveTimeInterval);
				}
			}

			if (nextSpriteP != NULL)
			{
				curSpriteP = nextSpriteP;
			}
			else
			{
				curSpriteP = curSpriteP->nextSpriteP;
			}
		}

		if (nextSpriteLayerP != NULL)
		{
			curSpriteLayerP = nextSpriteLayerP;	
		}
		else
		{
			curSpriteLayerP = curSpriteLayerP->nextSpriteLayerP;
		}
	}
}


///--------------------------------------------------------------------------------------
//	SWAnimateSpriteWorld
///--------------------------------------------------------------------------------------

SW_FUNC void SWAnimateSpriteWorld(
	SpriteWorldPtr spriteWorldP)
{
	register SpriteLayerPtr curSpriteLayerP;
	register SpritePtr curSpriteP;
	SpritePtr headActiveSpriteP = NULL;
	SpritePtr curActiveSpriteP = NULL;


	//-----------------erase the sprites--------------------

		// the current port should the one in which we are drawing    
	SetPort(spriteWorldP->loadFrameP->framePort.monoGrafP);

	curSpriteLayerP = spriteWorldP->headSpriteLayerP;	

		// iterate through the layers in this world
	while (curSpriteLayerP != NULL)
	{
		curSpriteP = curSpriteLayerP->headSpriteP;

			// iterate through the sprites in this layer
		while (curSpriteP != NULL)
		{
			if ((curSpriteP->needsToBeDrawn && curSpriteP->isVisible) ||
				(curSpriteP->needsToBeErased && !curSpriteP->isVisible))
			{
				if (headActiveSpriteP == NULL)
					headActiveSpriteP = curSpriteP;

				if (curActiveSpriteP != NULL)
						curActiveSpriteP->nextActiveSpriteP = curSpriteP;

				curActiveSpriteP = curSpriteP;

					// union last rect and current rect
					// this way is much faster than UnionRect
				curSpriteP->deltaFrameRect.top =
					SW_MIN(curSpriteP->oldFrameRect.top, curSpriteP->destFrameRect.top);
				curSpriteP->deltaFrameRect.left =
					SW_MIN(curSpriteP->oldFrameRect.left, curSpriteP->destFrameRect.left);
				curSpriteP->deltaFrameRect.bottom =
					SW_MAX(curSpriteP->oldFrameRect.bottom, curSpriteP->destFrameRect.bottom);
				curSpriteP->deltaFrameRect.right =
					SW_MAX(curSpriteP->oldFrameRect.right, curSpriteP->destFrameRect.right);

				{
					short temp;

						// align the left edge to long word boundary
					curSpriteP->deltaFrameRect.left &=
							(spriteWorldP->loadFrameP->leftAlignFactor);

						// align the right edge to long word boundary
					temp = curSpriteP->deltaFrameRect.right &
						spriteWorldP->loadFrameP->rightAlignFactor;
					if (temp != 0)
					{
						curSpriteP->deltaFrameRect.right +=
								(spriteWorldP->loadFrameP->rightAlignFactor + 1) - temp;
					}

						// align the left edge to long word boundary
					curSpriteP->oldFrameRect.left &=
						(spriteWorldP->loadFrameP->leftAlignFactor);

						// align the right edge to long word boundary
					temp = curSpriteP->oldFrameRect.right &
						spriteWorldP->loadFrameP->rightAlignFactor;
					if (temp != 0)
					{
						curSpriteP->oldFrameRect.right +=
							(spriteWorldP->loadFrameP->rightAlignFactor + 1) - temp;
					}
				}

					// copy the back drop piece
				(*spriteWorldP->eraseDrawProc)(
						spriteWorldP->backFrameP,
						spriteWorldP->loadFrameP,
						&curSpriteP->oldFrameRect);
			}

			curSpriteP = curSpriteP->nextSpriteP;
		}

		curSpriteLayerP = curSpriteLayerP->nextSpriteLayerP;	
	}

	if (curActiveSpriteP != NULL)
			curActiveSpriteP->nextActiveSpriteP = NULL;


	//-----------------draw the sprites--------------------

	curSpriteLayerP = spriteWorldP->headSpriteLayerP;

		// iterate through the layers in this world
	while (curSpriteLayerP != NULL)
	{
		curSpriteP = curSpriteLayerP->headSpriteP;

			// iterate through the sprites in this layer
		while (curSpriteP != NULL)
		{
			if (curSpriteP->isVisible)
			{
				if (curSpriteP->needsToBeDrawn)
				{
					if (curSpriteP->curFrameP->maskRgn != NULL)
					{
						Rect rgnRect = (**curSpriteP->curFrameP->maskRgn).rgnBBox;
	
							// move the mask region to the new sprite location
						OffsetRgn(
							curSpriteP->curFrameP->maskRgn,
							(curSpriteP->destFrameRect.left - rgnRect.left) +
							curSpriteP->curFrameP->offsetPoint.h,
							(curSpriteP->destFrameRect.top - rgnRect.top) +
							curSpriteP->curFrameP->offsetPoint.v);
					}

						// copy the sprite image onto the back drop piece
					(*curSpriteP->frameDrawProc)(
							curSpriteP->curFrameP,
							spriteWorldP->loadFrameP,
							&curSpriteP->curFrameP->frameRect,
							&curSpriteP->destFrameRect);
				}
				else
				{
					SWCheckIdleSpriteOverlap(spriteWorldP, curSpriteP, headActiveSpriteP);
				}
			}

			curSpriteP = curSpriteP->nextSpriteP;
		}

		curSpriteLayerP = curSpriteLayerP->nextSpriteLayerP;
	}


	//-----------------update the screen--------------------

		// the current port should the one in which we are drawing    
	SetPort(spriteWorldP->windowFrameP->framePort.monoGrafP);

	curSpriteP = headActiveSpriteP;

		// iterate through the sprites in this layer
	while (curSpriteP != NULL)
	{
			// copy the backdrop+sprite piece from the loader to the screen
		(*spriteWorldP->screenDrawProc)(
				spriteWorldP->loadFrameP,
				spriteWorldP->windowFrameP,
				&curSpriteP->deltaFrameRect);

			// set the delta and last rect to the current rect
		curSpriteP->deltaFrameRect = curSpriteP->destFrameRect;
		curSpriteP->oldFrameRect = curSpriteP->destFrameRect;

			// this sprite no longer needs to be drawn
		curSpriteP->needsToBeDrawn = false;
		curSpriteP->needsToBeErased = false;

		curSpriteP = curSpriteP->nextActiveSpriteP;
	}
}


///--------------------------------------------------------------------------------------
//	SWFastAnimateSpriteWorld
///--------------------------------------------------------------------------------------

SW_FUNC void SWFastAnimateSpriteWorld(
	SpriteWorldPtr spriteWorldP)
{
	register SpriteLayerPtr curSpriteLayerP;
	register SpritePtr curSpriteP;
	SpritePtr headActiveSpriteP = NULL;
	SpritePtr curActiveSpriteP = NULL;
#if !SW_PPC
	char mmuMode;

		// if we are already in 32 bit mode, then we don't have to incur the
		// overhead of calling SwapMMUMode. if we are in 24 bit mode, we incur
		// a neglible hit by checking before the swap.
	if (gSWmmuMode != true32b)
	{
			// change to 32-bit addressing mode to access video memory
			// the previous addressing mode is returned in mmuMode for restoring later
		mmuMode = true32b;
		SwapMMUMode(&mmuMode);
	}
#endif

	//-----------------erase the sprites--------------------

	curSpriteLayerP = spriteWorldP->headSpriteLayerP;	

		// iterate through the layers in this world
	while (curSpriteLayerP != NULL)
	{
		curSpriteP = curSpriteLayerP->headSpriteP;

			// iterate through the sprites in this layer
		while (curSpriteP != NULL)
		{
			if ((curSpriteP->needsToBeDrawn && curSpriteP->isVisible) ||
				(curSpriteP->needsToBeErased && !curSpriteP->isVisible))
			{
				if (headActiveSpriteP == NULL)
					headActiveSpriteP = curSpriteP;

				if (curActiveSpriteP != NULL)
						curActiveSpriteP->nextActiveSpriteP = curSpriteP;

				curActiveSpriteP = curSpriteP;

					// union last rect and current rect
					// this way is much faster than UnionRect
				curSpriteP->deltaFrameRect.top =
					SW_MIN(curSpriteP->oldFrameRect.top, curSpriteP->destFrameRect.top);
				curSpriteP->deltaFrameRect.left =
					SW_MIN(curSpriteP->oldFrameRect.left, curSpriteP->destFrameRect.left);
				curSpriteP->deltaFrameRect.bottom =
					SW_MAX(curSpriteP->oldFrameRect.bottom, curSpriteP->destFrameRect.bottom);
				curSpriteP->deltaFrameRect.right =
					SW_MAX(curSpriteP->oldFrameRect.right, curSpriteP->destFrameRect.right);

				{
					short temp;

						// align the left edge to long word boundary
					curSpriteP->deltaFrameRect.left &= kLeftAlignFactor;

						// align the right edge to long word boundary
					temp = curSpriteP->deltaFrameRect.right & kRightAlignFactor;
					if (temp != 0)
					{
						curSpriteP->deltaFrameRect.right += (kRightAlignFactor+1) - temp;
					}

						// align the left edge to long word boundary
					curSpriteP->oldFrameRect.left &= kLeftAlignFactor;

						// align the right edge to long word boundary
					temp = curSpriteP->oldFrameRect.right & kRightAlignFactor;
					if (temp != 0)
					{
						curSpriteP->oldFrameRect.right += (kRightAlignFactor+1) - temp;
					}
				}

					// copy the back drop piece
				(*spriteWorldP->eraseDrawProc)(
						spriteWorldP->backFrameP,
						spriteWorldP->loadFrameP,
						&curSpriteP->oldFrameRect);
			}

			curSpriteP = curSpriteP->nextSpriteP;
		}

		curSpriteLayerP = curSpriteLayerP->nextSpriteLayerP;	
	}

	if (curActiveSpriteP != NULL)
			curActiveSpriteP->nextActiveSpriteP = NULL;


	//-----------------draw the sprites--------------------

	curSpriteLayerP = spriteWorldP->headSpriteLayerP;

		// iterate through the layers in this world
	while (curSpriteLayerP != NULL)
	{
		curSpriteP = curSpriteLayerP->headSpriteP;

			// iterate through the sprites in this layer
		while (curSpriteP != NULL)
		{
			if (curSpriteP->isVisible)
			{
				if (curSpriteP->needsToBeDrawn)
				{
						// copy the sprite image onto the back drop piece
					(*curSpriteP->frameDrawProc)(
							curSpriteP->curFrameP,
							spriteWorldP->loadFrameP,
							&curSpriteP->curFrameP->frameRect,
							&curSpriteP->destFrameRect);
				}
				else
				{
					SWFastCheckIdleSpriteOverlap(spriteWorldP, curSpriteP, headActiveSpriteP);
				}
			}

			curSpriteP = curSpriteP->nextSpriteP;
		}

		curSpriteLayerP = curSpriteLayerP->nextSpriteLayerP;
	}


	//-----------------update the screen--------------------

	curSpriteP = headActiveSpriteP;

		// iterate through the sprites in this layer
	while (curSpriteP != NULL)
	{
			// copy the backdrop+sprite piece from the loader to the screen
		(*spriteWorldP->screenDrawProc)(
				spriteWorldP->loadFrameP,
				spriteWorldP->windowFrameP,
				&curSpriteP->deltaFrameRect);

			// set the delta and last rect to the current rect
		curSpriteP->deltaFrameRect = curSpriteP->destFrameRect;
		curSpriteP->oldFrameRect = curSpriteP->destFrameRect;

			// this sprite no longer needs to be drawn
		curSpriteP->needsToBeDrawn = false;
		curSpriteP->needsToBeErased = false;

		curSpriteP = curSpriteP->nextActiveSpriteP;
	}

#if !SW_PPC
	if (gSWmmuMode != true32b)
	{
		SwapMMUMode(&mmuMode);
	}
#endif
}


///--------------------------------------------------------------------------------------
//	SWBlastAnimateSpriteWorld
///--------------------------------------------------------------------------------------

SW_FUNC void SWBlastAnimateSpriteWorld(
	SpriteWorldPtr spriteWorldP)
{
	register SpriteLayerPtr curSpriteLayerP;
	register SpritePtr curSpriteP;
	SpritePtr headActiveSpriteP = NULL;
	SpritePtr curActiveSpriteP = NULL;
#if !SW_PPC
	char mmuMode;

		// if we are already in 32 bit mode, then we don't have to incur the
		// overhead of calling SwapMMUMode. if we are in 24 bit mode, we incur
		// a neglible hit by checking before the swap.
	if (gSWmmuMode != true32b)
	{
			// change to 32-bit addressing mode to access video memory
			// the previous addressing mode is returned in mmuMode for restoring later
		mmuMode = true32b;
		SwapMMUMode(&mmuMode);
	}
#endif

	//-----------------erase the sprites--------------------

	curSpriteLayerP = spriteWorldP->headSpriteLayerP;	

		// iterate through the layers in this world
	while (curSpriteLayerP != NULL)
	{
		curSpriteP = curSpriteLayerP->headSpriteP;

			// iterate through the sprites in this layer
		while (curSpriteP != NULL)
		{
			if ((curSpriteP->needsToBeDrawn && curSpriteP->isVisible) ||
				(curSpriteP->needsToBeErased && !curSpriteP->isVisible))
			{
				if (headActiveSpriteP == NULL)
					headActiveSpriteP = curSpriteP;

				if (curActiveSpriteP != NULL)
						curActiveSpriteP->nextActiveSpriteP = curSpriteP;

				curActiveSpriteP = curSpriteP;

					// union last rect and current rect
					// this way is much faster than UnionRect
				curSpriteP->deltaFrameRect.top =
					SW_MIN(curSpriteP->oldFrameRect.top, curSpriteP->destFrameRect.top);
				curSpriteP->deltaFrameRect.left =
					SW_MIN(curSpriteP->oldFrameRect.left, curSpriteP->destFrameRect.left);
				curSpriteP->deltaFrameRect.bottom =
					SW_MAX(curSpriteP->oldFrameRect.bottom, curSpriteP->destFrameRect.bottom);
				curSpriteP->deltaFrameRect.right =
					SW_MAX(curSpriteP->oldFrameRect.right, curSpriteP->destFrameRect.right);

				{
					short temp;

						// align the left edge to long word boundary
					curSpriteP->deltaFrameRect.left &= kLeftAlignFactor;

						// align the right edge to long word boundary
					temp = curSpriteP->deltaFrameRect.right & kRightAlignFactor;
					if (temp != 0)
					{
						curSpriteP->deltaFrameRect.right += (kRightAlignFactor+1) - temp;
					}

						// align the left edge to long word boundary
					curSpriteP->oldFrameRect.left &= kLeftAlignFactor;

						// align the right edge to long word boundary
					temp = curSpriteP->oldFrameRect.right & kRightAlignFactor;
					if (temp != 0)
					{
						curSpriteP->oldFrameRect.right += (kRightAlignFactor+1) - temp;
					}
				}

					// copy the back drop piece
				BlitPixieWorldDrawProc(
						spriteWorldP->backFrameP,
						spriteWorldP->loadFrameP,
						&curSpriteP->oldFrameRect);
			}

			curSpriteP = curSpriteP->nextSpriteP;
		}

		curSpriteLayerP = curSpriteLayerP->nextSpriteLayerP;	
	}

	if (curActiveSpriteP != NULL)
			curActiveSpriteP->nextActiveSpriteP = NULL;


	//-----------------draw the sprites--------------------

	curSpriteLayerP = spriteWorldP->headSpriteLayerP;

		// iterate through the layers in this world
	while (curSpriteLayerP != NULL)
	{
		curSpriteP = curSpriteLayerP->headSpriteP;

			// iterate through the sprites in this layer
		while (curSpriteP != NULL)
		{
			if (curSpriteP->isVisible)
			{
				if (curSpriteP->needsToBeDrawn)
				{
							// jump into the compiled mask code
					(*curSpriteP->frameDrawProc)(
							curSpriteP->curFrameP,
							spriteWorldP->loadFrameP,
							&curSpriteP->curFrameP->frameRect,
							&curSpriteP->destFrameRect);
				}
				else
				{
					SWFastCheckIdleSpriteOverlap(spriteWorldP, curSpriteP, headActiveSpriteP);
				}
			}

			curSpriteP = curSpriteP->nextSpriteP;
		}

		curSpriteLayerP = curSpriteLayerP->nextSpriteLayerP;
	}


	//-----------------update the screen--------------------

	curSpriteP = headActiveSpriteP;

		// iterate through the sprites in this layer
	while (curSpriteP != NULL)
	{
			// copy the backdrop+sprite piece from the loader to the screen
		BlitPixieWorldDrawProc(
				spriteWorldP->loadFrameP,
				spriteWorldP->windowFrameP,
				&curSpriteP->deltaFrameRect);

			// set the delta and last rect to the current rect
		curSpriteP->deltaFrameRect = curSpriteP->destFrameRect;
		curSpriteP->oldFrameRect = curSpriteP->destFrameRect;

			// this sprite no longer needs to be drawn
		curSpriteP->needsToBeDrawn = false;
		curSpriteP->needsToBeErased = false;

		curSpriteP = curSpriteP->nextActiveSpriteP;
	}

#if !SW_PPC
	if (gSWmmuMode != true32b)
	{
		SwapMMUMode(&mmuMode);
	}
#endif
}


///--------------------------------------------------------------------------------------
//	SWCheckIdleSpriteOverlap
///--------------------------------------------------------------------------------------

static void SWCheckIdleSpriteOverlap(
	SpriteWorldPtr spriteWorldP,
	register SpritePtr idleSpriteP,
	SpritePtr headActiveSpriteP)
{
	register SpritePtr activeSpriteP;
	RgnHandle maskRgn = idleSpriteP->curFrameP->maskRgn;

	activeSpriteP = headActiveSpriteP;

		// iterate through the active sprites
	while (activeSpriteP != NULL)
	{
			// do the sprites overlap?
		if ((idleSpriteP->destFrameRect.top < activeSpriteP->deltaFrameRect.bottom) &&
			 (idleSpriteP->destFrameRect.bottom > activeSpriteP->deltaFrameRect.top) &&
			 (idleSpriteP->destFrameRect.left < activeSpriteP->deltaFrameRect.right) &&
			 (idleSpriteP->destFrameRect.right > activeSpriteP->deltaFrameRect.left))
		{
			Rect srcSectRect, dstSectRect;

			if (maskRgn != NULL)
			{
					// move the mask region to the new sprite location
				OffsetRgn(
					maskRgn,
					(idleSpriteP->destFrameRect.left - (**maskRgn).rgnBBox.left) +
					idleSpriteP->curFrameP->offsetPoint.h,
					(idleSpriteP->destFrameRect.top - (**maskRgn).rgnBBox.top) +
					idleSpriteP->curFrameP->offsetPoint.v);

				maskRgn = NULL;
			}

				// calculate the intersection between the idle sprite's destination
				// rect, and the active sprite's old rect
			dstSectRect.left =
					SW_MAX(idleSpriteP->destFrameRect.left, activeSpriteP->deltaFrameRect.left);
			dstSectRect.top =
					SW_MAX(idleSpriteP->destFrameRect.top, activeSpriteP->deltaFrameRect.top);
			dstSectRect.right =
					SW_MIN(idleSpriteP->destFrameRect.right, activeSpriteP->deltaFrameRect.right);
			dstSectRect.bottom =
					SW_MIN(idleSpriteP->destFrameRect.bottom, activeSpriteP->deltaFrameRect.bottom);

			srcSectRect = dstSectRect;

			srcSectRect.left -= idleSpriteP->destFrameRect.left;
			srcSectRect.right -= idleSpriteP->destFrameRect.left;
			srcSectRect.top -= idleSpriteP->destFrameRect.top;
			srcSectRect.bottom -= idleSpriteP->destFrameRect.top;

				// copy a piece of the sprite image onto the back drop piece
			(*idleSpriteP->frameDrawProc)(
					idleSpriteP->curFrameP,
					spriteWorldP->loadFrameP,
					&srcSectRect,
					&dstSectRect);
		}

		activeSpriteP = activeSpriteP->nextActiveSpriteP;
	}
}


///--------------------------------------------------------------------------------------
//	SWFastCheckIdleSpriteOverlap
///--------------------------------------------------------------------------------------

static void SWFastCheckIdleSpriteOverlap(
	SpriteWorldPtr spriteWorldP,
	register SpritePtr idleSpriteP,
	SpritePtr headActiveSpriteP)
{
	register SpritePtr activeSpriteP = headActiveSpriteP;

		// iterate through the active sprites
	while (activeSpriteP != NULL)
	{
			// do the sprites overlap?
		if ((idleSpriteP->destFrameRect.top < activeSpriteP->deltaFrameRect.bottom) &&
			 (idleSpriteP->destFrameRect.bottom > activeSpriteP->deltaFrameRect.top) &&
			 (idleSpriteP->destFrameRect.left < activeSpriteP->deltaFrameRect.right) &&
			 (idleSpriteP->destFrameRect.right > activeSpriteP->deltaFrameRect.left))
		{
			Rect srcSectRect, dstSectRect;

				// calculate the intersection between the idle sprite's destination
				// rect, and the active sprite's old rect
			dstSectRect.left =
					SW_MAX(idleSpriteP->destFrameRect.left, activeSpriteP->deltaFrameRect.left);
			dstSectRect.top =
					SW_MAX(idleSpriteP->destFrameRect.top, activeSpriteP->deltaFrameRect.top);
			dstSectRect.right =
					SW_MIN(idleSpriteP->destFrameRect.right, activeSpriteP->deltaFrameRect.right);
			dstSectRect.bottom =
					SW_MIN(idleSpriteP->destFrameRect.bottom, activeSpriteP->deltaFrameRect.bottom);

			srcSectRect = dstSectRect;

			srcSectRect.left -= idleSpriteP->destFrameRect.left;
			srcSectRect.right -= idleSpriteP->destFrameRect.left;
			srcSectRect.top -= idleSpriteP->destFrameRect.top;
			srcSectRect.bottom -= idleSpriteP->destFrameRect.top;

				// copy a piece of the sprite image onto the back drop piece
			(*idleSpriteP->frameDrawProc)(
					idleSpriteP->curFrameP,
					spriteWorldP->loadFrameP,
					&srcSectRect,
					&dstSectRect);
		}

		activeSpriteP = activeSpriteP->nextActiveSpriteP;
	}
}


