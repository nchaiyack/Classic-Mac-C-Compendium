///--------------------------------------------------------------------------------------
//	Sprite.c
//
//	Created:	Tuesday, October, 1992 at 10:10:06 PM
//	By:		Tony Myles
//
//	Copyright: © 1991-94 Tony Myles, All rights reserved worldwide
//
//	Description:	implementation of the sprites
///--------------------------------------------------------------------------------------


#ifndef __SWCOMMON__
#include "SWCommonHeaders.h"
#endif

#ifndef __MEMORY__
#include <Memory.h>
#endif

#ifndef __SPRITEWORLDUTILS__
#include "SpriteWorldUtils.h"
#endif

#ifndef __SPRITEWORLD__
#include "SpriteWorld.h"
#endif

#ifndef __SPRITE__
#include "Sprite.h"
#endif

#ifndef __FRAME__
#include "Frame.h"
#endif

#if MPW
#pragma segment SpriteWorld
#endif


///--------------------------------------------------------------------------------------
//	SWCreateSprite
///--------------------------------------------------------------------------------------

SW_FUNC OSErr SWCreateSprite(
	SpritePtr* newSpriteP,
	void* spriteStorageP,
	short maxFrames)
{
	OSErr err = noErr;
	SpritePtr tempSpriteP;
	Size frameArraySize;

	*newSpriteP = NULL;

	tempSpriteP = (SpritePtr) (spriteStorageP == NULL) ? NewPtr(sizeof(SpriteRec)) : spriteStorageP;

	if (tempSpriteP != NULL)
	{
		frameArraySize = (Size)(maxFrames * sizeof(FramePtr));

		tempSpriteP->frameArray = (FramePtr *)NewPtrClear(frameArraySize);

		if (tempSpriteP->frameArray != NULL)
		{
			tempSpriteP->nextSpriteP = NULL;
			tempSpriteP->prevSpriteP = NULL;

				// initialize drawing fields
			tempSpriteP->isVisible = true;
			tempSpriteP->needsToBeDrawn = true;
			tempSpriteP->needsToBeErased = false;
			tempSpriteP->destFrameRect.left = 0;
			tempSpriteP->destFrameRect.top = 0;
			tempSpriteP->destFrameRect.right = 0;
			tempSpriteP->destFrameRect.bottom = 0;
			tempSpriteP->oldFrameRect.left = 0;
			tempSpriteP->oldFrameRect.top = 0;
			tempSpriteP->oldFrameRect.right = 0;
			tempSpriteP->oldFrameRect.bottom = 0;
			tempSpriteP->deltaFrameRect.left = 0;
			tempSpriteP->deltaFrameRect.top = 0;
			tempSpriteP->deltaFrameRect.right = 0;
			tempSpriteP->deltaFrameRect.bottom = 0;
			tempSpriteP->screenMaskRgn = NULL;
			tempSpriteP->frameDrawProc = SWStdDrawProc;

				// initialize frame fields
			tempSpriteP->frameTimeTask.timeTask.tmAddr = NewTimerProc(SWTimeTask);
			tempSpriteP->frameTimeTask.timeTask.qLink = NULL;
			tempSpriteP->frameTimeTask.timeTask.qType = 0;
			tempSpriteP->frameTimeTask.timeTask.tmCount = 0;
			tempSpriteP->frameTimeTask.timeTask.tmWakeUp = 0;
			tempSpriteP->frameTimeTask.timeTask.tmReserved = 0;
			tempSpriteP->frameTimeTask.hasTaskFired = false;
			tempSpriteP->curFrameP = NULL;
			tempSpriteP->numFrames = 0;
			tempSpriteP->maxFrames = maxFrames;
			tempSpriteP->frameTimeInterval = -1;
			tempSpriteP->frameAdvance = 1;
			tempSpriteP->curFrameIndex = 0;
			tempSpriteP->firstFrameIndex = 0;
			tempSpriteP->lastFrameIndex = 0;
			tempSpriteP->frameChangeProc = NULL;

				// initialize movement fields
			tempSpriteP->moveTimeTask.timeTask.tmAddr = NewTimerProc(SWTimeTask);
			tempSpriteP->moveTimeTask.timeTask.qLink = NULL;
			tempSpriteP->moveTimeTask.timeTask.qType = 0;
			tempSpriteP->moveTimeTask.timeTask.tmCount = 0;
			tempSpriteP->moveTimeTask.timeTask.tmWakeUp = 0;
			tempSpriteP->moveTimeTask.timeTask.tmReserved = 0;
			tempSpriteP->moveTimeTask.hasTaskFired = true;
			tempSpriteP->moveTimeInterval = 0;
			tempSpriteP->horizMoveDelta = 0;
			tempSpriteP->vertMoveDelta = 0;
			tempSpriteP->moveBoundsRect.left = 0;
			tempSpriteP->moveBoundsRect.top = 0;
			tempSpriteP->moveBoundsRect.right = 0;
			tempSpriteP->moveBoundsRect.bottom = 0;
			tempSpriteP->spriteMoveProc = NULL;

				// collision detection fields
			tempSpriteP->collideRect.left = 0;
			tempSpriteP->collideRect.top = 0;
			tempSpriteP->collideRect.right = 0;
			tempSpriteP->collideRect.bottom = 0;
			tempSpriteP->spriteCollideProc = NULL;

			tempSpriteP->userData = 0;

				// insert the sprite time tasks into the time manager queue
			InsTime((QElemPtr)&tempSpriteP->moveTimeTask);
			InsTime((QElemPtr)&tempSpriteP->frameTimeTask);

				// the sprite has been successfully created
			*newSpriteP = tempSpriteP;
		}
		else
		{
			err = MemError();

			DisposePtr((Ptr)tempSpriteP);
		}
	}
	else
	{
		err = MemError();
	}

	return err;
}


///--------------------------------------------------------------------------------------
//	SWCreateSpriteFromCIconResource
///--------------------------------------------------------------------------------------

SW_FUNC OSErr SWCreateSpriteFromCIconResource(
	SpritePtr* newSpriteP,
	void* spriteStorageP,
	short cIconID,
	short maxFrames,
	MaskType maskType)
{
	OSErr err;
	SpritePtr tempSpriteP;
	FramePtr newFrameP;
	short frame;

	*newSpriteP = NULL;

	err = SWCreateSprite(&tempSpriteP, spriteStorageP, maxFrames);

	if (err == noErr)
	{
		for (frame = 0; frame < maxFrames; frame++)
		{
			err = SWCreateFrameFromCIconResource(&newFrameP, cIconID + frame, maskType);

			if (err == noErr)
			{
				err = SWAddFrame(tempSpriteP, newFrameP);
			}

			if (err != noErr)
			{
				SWDisposeFrame(newFrameP);
				SWDisposeSprite(tempSpriteP);
				break;
			}
		}

		if (err == noErr)
		{
			SWSetSpriteFrameRange(tempSpriteP, 0, maxFrames - 1);

			*newSpriteP = tempSpriteP;
		}
	}

	return err;
}


///--------------------------------------------------------------------------------------
//	SWCreateSpriteFromPictResource
///--------------------------------------------------------------------------------------

SW_FUNC OSErr SWCreateSpriteFromPictResource(
	SpritePtr* newSpriteP,
	void* spriteStorageP,
	short pictResID,
	short maskResID,
	short maxFrames,
	MaskType maskType)
{
	OSErr err;
	SpritePtr tempSpriteP;
	FramePtr newFrameP;
	short frame;

	*newSpriteP = NULL;

	err = SWCreateSprite(&tempSpriteP, spriteStorageP, maxFrames);

	if (err == noErr)
	{
		for (frame = 0; frame < maxFrames; frame++)
		{
			err = SWCreateFrameFromPictResource(&newFrameP, pictResID + frame, maskResID + frame, maskType);

			if (err == noErr)
			{
				err = SWAddFrame(tempSpriteP, newFrameP);
			}

			if (err != noErr)
			{
				SWDisposeFrame(newFrameP);
				SWDisposeSprite(tempSpriteP);
				break;
			}
		}

		if (err == noErr)
		{
			SWSetSpriteFrameRange(tempSpriteP, 0, maxFrames - 1);

			*newSpriteP = tempSpriteP;
		}
	}
	
	return err;
}


///--------------------------------------------------------------------------------------
//	SWCloneSprite
///--------------------------------------------------------------------------------------

SW_FUNC OSErr SWCloneSprite(
	SpritePtr cloneSpriteP,
	SpritePtr* newSpriteP,
	void* spriteStorageP)
{
	OSErr err;
	SpritePtr tempSpriteP;
	TimeTaskRec moveTimeTask, frameTimeTask;
	short frame;
	FramePtr* tempFrameArray;

	err = SWCreateSprite(&tempSpriteP, spriteStorageP, cloneSpriteP->maxFrames);

	if (err == noErr)
	{
			// save off the fields that are unique to a particular sprite
		moveTimeTask = tempSpriteP->moveTimeTask;
		frameTimeTask = tempSpriteP->frameTimeTask;
		tempFrameArray = tempSpriteP->frameArray;

			// copy the clone sprite into the temp sprite
		*tempSpriteP = *cloneSpriteP;

			// restore the unique fields
		tempSpriteP->moveTimeTask = moveTimeTask;
		tempSpriteP->frameTimeTask = frameTimeTask;
		tempSpriteP->frameArray = tempFrameArray;

			// clear the next and prev fields, just in case
		tempSpriteP->nextSpriteP = NULL;
		tempSpriteP->prevSpriteP = NULL;

			// copy the frame array
		for (frame = 0; frame < tempSpriteP->maxFrames; frame++)
		{
			tempSpriteP->frameArray[frame] = cloneSpriteP->frameArray[frame];
			tempSpriteP->frameArray[frame]->useCount++;
		}

		*newSpriteP = tempSpriteP;
	}

	return err;
}


///--------------------------------------------------------------------------------------
//	SWDisposeSprite
///--------------------------------------------------------------------------------------

SW_FUNC void SWDisposeSprite(
	SpritePtr deadSpriteP)
{
	SWCloseSprite(deadSpriteP);

	DisposePtr((Ptr)deadSpriteP);
}


///--------------------------------------------------------------------------------------
//	SWCloseSprite
///--------------------------------------------------------------------------------------

SW_FUNC void SWCloseSprite(
	SpritePtr deadSpriteP)
{
	short frame;

	if (deadSpriteP != NULL)
	{
			// remove the sprite time tasks from the time manager queue
		RmvTime((QElemPtr)&deadSpriteP->moveTimeTask);
		RmvTime((QElemPtr)&deadSpriteP->frameTimeTask);

#if SW_PPC
		DisposeRoutineDescriptor(deadSpriteP->moveTimeTask.timeTask.tmAddr);
		DisposeRoutineDescriptor(deadSpriteP->frameTimeTask.timeTask.tmAddr);
#endif


		for (frame = 0; frame < deadSpriteP->numFrames; frame++)
		{
			SWDisposeFrame(deadSpriteP->frameArray[frame]);
		}

		DisposePtr((Ptr)deadSpriteP->frameArray);
	}
}


///--------------------------------------------------------------------------------------
//	SWLockSprite
///--------------------------------------------------------------------------------------

SW_FUNC void SWLockSprite(
	SpritePtr srcSpriteP)
{
	register long numFrames;

		// is the sprite not locked?
	numFrames = srcSpriteP->numFrames;

	while (numFrames--)
	{
		SWLockFrame(srcSpriteP->frameArray[numFrames]);
	}
}


///--------------------------------------------------------------------------------------
//	SWUnlockSprite
///--------------------------------------------------------------------------------------

SW_FUNC void SWUnlockSprite(
	SpritePtr srcSpriteP)
{
	register long numFrames;

		// is the sprite locked?
	numFrames = srcSpriteP->numFrames;

	while (numFrames--)
	{
		SWUnlockFrame(srcSpriteP->frameArray[numFrames]);
	}
}


///--------------------------------------------------------------------------------------
//	SWSetSpriteDrawProc
///--------------------------------------------------------------------------------------

SW_FUNC void SWSetSpriteDrawProc(
	SpritePtr srcSpriteP,
	DrawProcPtr drawProc)
{
	srcSpriteP->frameDrawProc = drawProc;
}


///--------------------------------------------------------------------------------------
//	SWStdDrawProc
///--------------------------------------------------------------------------------------

SW_FUNC void SWStdDrawProc(
	FramePtr srcFrameP,
	FramePtr dstFrameP,
	Rect* srcRect,
	Rect* dstRect)
{
		// by the way, CopyBits with a mask region is 60% faster than CopyMask!
	CopyBits((BitMapPtr)srcFrameP->framePix.pixMapP, (BitMapPtr)dstFrameP->framePix.pixMapP,
				srcRect, dstRect, srcCopy, srcFrameP->maskRgn);
}


///--------------------------------------------------------------------------------------
//	SWAddFrame
///--------------------------------------------------------------------------------------

SW_FUNC OSErr SWAddFrame(
	SpritePtr srcSpriteP,
	FramePtr newFrameP)
{
	OSErr err = noErr;

		// donÕt exceed maximum number of frames
	if (srcSpriteP->numFrames < srcSpriteP->maxFrames)
	{
		srcSpriteP->frameArray[srcSpriteP->numFrames] = newFrameP;

			// increment the number of frames
		srcSpriteP->numFrames++;
		newFrameP->useCount++;

		SWSetCurrentFrame(srcSpriteP, srcSpriteP->frameArray[0]);
	}
	else
	{
		err = kMaxFramesErr;
	}

	return err;
}


///--------------------------------------------------------------------------------------
//	SWRemoveFrame
///--------------------------------------------------------------------------------------

SW_FUNC void SWRemoveFrame(
	SpritePtr srcSpriteP,
	FramePtr oldFrameP)
{
	register long numFrames;
	register FramePtr *frameArray;

	numFrames = srcSpriteP->numFrames;
	frameArray = srcSpriteP->frameArray;

	oldFrameP->useCount--;

		// find the frame to be removed
	while (numFrames--)
	{
		if (frameArray[numFrames] == oldFrameP)
		{
				// move the rest of the frames down
			while (numFrames < (srcSpriteP->numFrames - 1L))
			{
				frameArray[numFrames] = frameArray[numFrames + 1L];

				numFrames++;
			}

			break;
		}
	}
}


///--------------------------------------------------------------------------------------
//	SWSetCurrentFrame
///--------------------------------------------------------------------------------------

SW_FUNC void SWSetCurrentFrame(
	SpritePtr srcSpriteP,
	FramePtr newFrameP)
{
	Rect lastRect;
	short horizOffset, vertOffset;
	short frameIndex = 0;

	if (srcSpriteP->curFrameP != newFrameP)
	{
		srcSpriteP->curFrameP = newFrameP;

		lastRect = srcSpriteP->destFrameRect;

		srcSpriteP->destFrameRect = newFrameP->frameRect;

		horizOffset = (lastRect.left - srcSpriteP->destFrameRect.left);
		vertOffset = (lastRect.top - srcSpriteP->destFrameRect.top);

		srcSpriteP->destFrameRect.left += horizOffset;
		srcSpriteP->destFrameRect.right += horizOffset;
		srcSpriteP->destFrameRect.top += vertOffset;
		srcSpriteP->destFrameRect.bottom += vertOffset;

			// this really lame search is performed so we set the
			// frame index of the sprite properly, which could
			// prevent strange behavior later on. The lesson to be learned
			// here is to call SWSetCurrentFrameIndex instead.
		for (frameIndex = 0; frameIndex < srcSpriteP->numFrames; frameIndex++)
		{
			if (srcSpriteP->frameArray[frameIndex] == newFrameP)
			{
				srcSpriteP->curFrameIndex = frameIndex;
				break;
			}
		}

		srcSpriteP->needsToBeDrawn = true;
	}
}


///--------------------------------------------------------------------------------------
//	SWSetCurrentFrameIndex
///--------------------------------------------------------------------------------------

SW_FUNC void SWSetCurrentFrameIndex(
	SpritePtr srcSpriteP,
	short frameIndex)
{
	register FramePtr newFrameP;
	Rect lastRect;
	short horizOffset, vertOffset;

	if (frameIndex < srcSpriteP->numFrames)
	{
		newFrameP = srcSpriteP->frameArray[frameIndex];

		srcSpriteP->curFrameP = newFrameP;

		lastRect = srcSpriteP->destFrameRect;

		srcSpriteP->destFrameRect = newFrameP->frameRect;

		horizOffset = (lastRect.left - srcSpriteP->destFrameRect.left);
		vertOffset = (lastRect.top - srcSpriteP->destFrameRect.top);

		srcSpriteP->destFrameRect.left += horizOffset;
		srcSpriteP->destFrameRect.right += horizOffset;
		srcSpriteP->destFrameRect.top += vertOffset;
		srcSpriteP->destFrameRect.bottom += vertOffset;

		srcSpriteP->curFrameIndex = frameIndex;

		srcSpriteP->needsToBeDrawn = true;
	}
}


///--------------------------------------------------------------------------------------
//	SWSetSpriteFrameAdvance
///--------------------------------------------------------------------------------------

SW_FUNC void SWSetSpriteFrameAdvance(
	SpritePtr srcSpriteP,
	short frameAdvance)
{
	srcSpriteP->frameAdvance = frameAdvance;
}


///--------------------------------------------------------------------------------------
//	SWSetSpriteFrameRange
///--------------------------------------------------------------------------------------

SW_FUNC void SWSetSpriteFrameRange(
	SpritePtr srcSpriteP,
	short firstFrameIndex,
	short lastFrameIndex)
{
	srcSpriteP->firstFrameIndex = firstFrameIndex;
	srcSpriteP->lastFrameIndex = lastFrameIndex;
}


///--------------------------------------------------------------------------------------
//	SWSetSpriteFrameTime
///--------------------------------------------------------------------------------------

SW_FUNC void SWSetSpriteFrameTime(
	SpritePtr srcSpriteP,
	long timeInterval)
{
	srcSpriteP->frameTimeInterval = timeInterval;

		// is the time interval positive?
	if (timeInterval > 0)
	{
			// if the task is not already primedÉ
		if (!SWIsTaskPrimed(&srcSpriteP->frameTimeTask.timeTask))
		{
				// Éprime it
			srcSpriteP->frameTimeTask.hasTaskFired = true;
		}
	}
		// is the time interval negative?
	else if (timeInterval < 0)
	{
			// if the task is primedÉ
		if (SWIsTaskPrimed(&srcSpriteP->frameTimeTask.timeTask))
		{
				// Éremove it
			RmvTime((QElemPtr)&srcSpriteP->frameTimeTask);
			InsTime((QElemPtr)&srcSpriteP->frameTimeTask);
		}

			// the timeInterval is negative, which means never change the frame
		srcSpriteP->frameTimeTask.hasTaskFired = false;
	}
		// is the time interval zero?
	else
	{
			// this means change frames as quickly as possible
		srcSpriteP->frameTimeTask.hasTaskFired = true;
	}
}


///--------------------------------------------------------------------------------------
//	SWSetSpriteFrameProc
///--------------------------------------------------------------------------------------

SW_FUNC void SWSetSpriteFrameProc(
	SpritePtr srcSpriteP,
	FrameProcPtr frameProc)
{
	srcSpriteP->frameChangeProc = frameProc;
}


///--------------------------------------------------------------------------------------
//	SWMoveSprite
///--------------------------------------------------------------------------------------

SW_FUNC void SWMoveSprite(
	SpritePtr srcSpriteP,
	short horizLoc,
	short vertLoc)
{
	if ((horizLoc != 0) || (vertLoc != 0))
	{
		srcSpriteP->destFrameRect.bottom = vertLoc + (srcSpriteP->destFrameRect.bottom -
													srcSpriteP->destFrameRect.top);
		srcSpriteP->destFrameRect.right = horizLoc + (srcSpriteP->destFrameRect.right -
													srcSpriteP->destFrameRect.left);
		srcSpriteP->destFrameRect.top = vertLoc;
		srcSpriteP->destFrameRect.left = horizLoc;

		srcSpriteP->needsToBeDrawn = true;
	}
}


///--------------------------------------------------------------------------------------
//	SWOffsetSprite
///--------------------------------------------------------------------------------------

SW_FUNC void SWOffsetSprite(
	SpritePtr srcSpriteP,
	short horizDelta,
	short vertDelta)
{
	if ((horizDelta != 0) || (vertDelta != 0))
	{
		srcSpriteP->destFrameRect.right += horizDelta;
		srcSpriteP->destFrameRect.bottom += vertDelta;
		srcSpriteP->destFrameRect.left += horizDelta;
		srcSpriteP->destFrameRect.top += vertDelta;

		srcSpriteP->needsToBeDrawn = true;
	}
}


///--------------------------------------------------------------------------------------
//	SWSetSpriteLocation
///--------------------------------------------------------------------------------------

SW_FUNC void SWSetSpriteLocation(
	SpritePtr srcSpriteP,
	short horizLoc,
	short vertLoc)
{
	srcSpriteP->destFrameRect.bottom = vertLoc + (srcSpriteP->destFrameRect.bottom -
												srcSpriteP->destFrameRect.top);
	srcSpriteP->destFrameRect.right = horizLoc + (srcSpriteP->destFrameRect.right -
												srcSpriteP->destFrameRect.left);
	srcSpriteP->destFrameRect.top = vertLoc;
	srcSpriteP->destFrameRect.left = horizLoc;

	srcSpriteP->deltaFrameRect = srcSpriteP->destFrameRect;
	srcSpriteP->oldFrameRect = srcSpriteP->destFrameRect;
}


///--------------------------------------------------------------------------------------
//	SWSetSpriteMoveBounds
///--------------------------------------------------------------------------------------

SW_FUNC void SWSetSpriteMoveBounds(
	SpritePtr srcSpriteP,
	Rect* moveBoundsRect)
{
	srcSpriteP->moveBoundsRect = *moveBoundsRect;
}


///--------------------------------------------------------------------------------------
//	SWSetSpriteMoveDelta
///--------------------------------------------------------------------------------------

SW_FUNC void SWSetSpriteMoveDelta(
	SpritePtr srcSpriteP,
	short horizDelta,
	short vertDelta)
{
		// set spriteÕs velocity
	srcSpriteP->horizMoveDelta = horizDelta;
	srcSpriteP->vertMoveDelta = vertDelta;
}


///--------------------------------------------------------------------------------------
//	SWSetSpriteMoveTime
///--------------------------------------------------------------------------------------

SW_FUNC void SWSetSpriteMoveTime(
	SpritePtr srcSpriteP,
	long timeInterval)
{
	srcSpriteP->moveTimeInterval = timeInterval;

		// is the time interval positive?
	if (timeInterval > 0)
	{
			// if the task is not already primedÉ
		if (!SWIsTaskPrimed(&srcSpriteP->moveTimeTask.timeTask))
		{
				// Éprime it
			srcSpriteP->moveTimeTask.hasTaskFired = true;
		}
	}
		// is the time interval negative?
	else if (timeInterval < 0)
	{
			// if the task is primedÉ
		if (SWIsTaskPrimed(&srcSpriteP->moveTimeTask.timeTask))
		{
				// Éremove it
			RmvTime((QElemPtr)&srcSpriteP->moveTimeTask);
			InsTime((QElemPtr)&srcSpriteP->moveTimeTask);
		}

			// the timeInterval is negative, which means never change the frame
		srcSpriteP->moveTimeTask.hasTaskFired = false;
	}
		// is the time interval zero?
	else
	{
			// this means change frames as quickly as possible
		srcSpriteP->moveTimeTask.hasTaskFired = true;
	}
}


///--------------------------------------------------------------------------------------
//	SWSetSpriteMoveProc
///--------------------------------------------------------------------------------------

SW_FUNC void SWSetSpriteMoveProc(
	SpritePtr srcSpriteP,
	MoveProcPtr moveProc)
{
	srcSpriteP->spriteMoveProc = moveProc;
}


///--------------------------------------------------------------------------------------
//	SWBounceSpriteMoveProc
///--------------------------------------------------------------------------------------

SW_FUNC void SWBounceSpriteMoveProc(
	SpritePtr srcSpriteP,
	Point* spritePoint)
{
	if (srcSpriteP->destFrameRect.left < srcSpriteP->moveBoundsRect.left)
	{
		srcSpriteP->horizMoveDelta = -srcSpriteP->horizMoveDelta;
		spritePoint->h = srcSpriteP->moveBoundsRect.left;
	}
	else if (srcSpriteP->destFrameRect.right > srcSpriteP->moveBoundsRect.right)
	{
		srcSpriteP->horizMoveDelta = -srcSpriteP->horizMoveDelta;
		spritePoint->h = srcSpriteP->moveBoundsRect.right -
			(srcSpriteP->curFrameP->frameRect.right - srcSpriteP->curFrameP->frameRect.left);
	}

	if (srcSpriteP->destFrameRect.top < srcSpriteP->moveBoundsRect.top)
	{
		srcSpriteP->vertMoveDelta = -srcSpriteP->vertMoveDelta;
		spritePoint->v = srcSpriteP->moveBoundsRect.top;
	}
	else if (srcSpriteP->destFrameRect.bottom > srcSpriteP->moveBoundsRect.bottom)
	{
		srcSpriteP->vertMoveDelta = -srcSpriteP->vertMoveDelta;
		spritePoint->v = srcSpriteP->moveBoundsRect.bottom -
			(srcSpriteP->curFrameP->frameRect.bottom - srcSpriteP->curFrameP->frameRect.top);
	}
}


///--------------------------------------------------------------------------------------
//	SWWrapSpriteMoveProc
///--------------------------------------------------------------------------------------

SW_FUNC void SWWrapSpriteMoveProc(
	SpritePtr srcSpriteP,
	Point* spritePoint)
{
	Point oldSpritePoint = *spritePoint;

	if (srcSpriteP->destFrameRect.left > srcSpriteP->moveBoundsRect.right)
	{
		spritePoint->h = srcSpriteP->moveBoundsRect.left -
								(srcSpriteP->destFrameRect.right - 
								srcSpriteP->destFrameRect.left);
	}
	else if (srcSpriteP->destFrameRect.right < srcSpriteP->moveBoundsRect.left)
	{
		spritePoint->h = srcSpriteP->moveBoundsRect.right;
	}

	if (srcSpriteP->destFrameRect.top > srcSpriteP->moveBoundsRect.bottom)
	{
		spritePoint->v = srcSpriteP->moveBoundsRect.top -
								(srcSpriteP->destFrameRect.bottom - 
								srcSpriteP->destFrameRect.top);
	}
	else if (srcSpriteP->destFrameRect.bottom < srcSpriteP->moveBoundsRect.top)
	{
		spritePoint->v = srcSpriteP->moveBoundsRect.bottom;
	}

	if ((spritePoint->h != oldSpritePoint.h) || (spritePoint->v != oldSpritePoint.v))
	{
		SWSetSpriteLocation(srcSpriteP, spritePoint->h, spritePoint->v);
	}	
}


///--------------------------------------------------------------------------------------
//	SWSetSpriteCollideProc
///--------------------------------------------------------------------------------------

SW_FUNC void SWSetSpriteCollideProc(
	SpritePtr srcSpriteP,
	CollideProcPtr collideProc)
{
	srcSpriteP->spriteCollideProc = collideProc;
}


///--------------------------------------------------------------------------------------
//	SWSetSpriteVisible
///--------------------------------------------------------------------------------------

SW_FUNC void SWSetSpriteVisible(
	SpritePtr srcSpriteP,
	Boolean isVisible)
{
	srcSpriteP->isVisible = isVisible;
	srcSpriteP->needsToBeDrawn = true;
	srcSpriteP->needsToBeErased = !isVisible;
}


///--------------------------------------------------------------------------------------
//	SWIsSpriteInRect
///--------------------------------------------------------------------------------------

SW_FUNC Boolean SWIsSpriteInRect(
	SpritePtr srcSpriteP,
	Rect* testRect)
{
	return ((srcSpriteP->destFrameRect.top < testRect->bottom) &&
			(srcSpriteP->destFrameRect.bottom > testRect->top) &&
		 	(srcSpriteP->destFrameRect.left < testRect->right) &&
		 	(srcSpriteP->destFrameRect.right > testRect->left));
}


///--------------------------------------------------------------------------------------
//	SWIsPointInSprite
///--------------------------------------------------------------------------------------

SW_FUNC Boolean SWIsPointInSprite(
	SpritePtr srcSpriteP,
	Point testPoint)
{
	return	(testPoint.h >= srcSpriteP->destFrameRect.left) &&
			(testPoint.h <= srcSpriteP->destFrameRect.right) &&
			(testPoint.v >= srcSpriteP->destFrameRect.top) &&
			(testPoint.v <= srcSpriteP->destFrameRect.bottom);
}


///--------------------------------------------------------------------------------------
//	SWTimeTask
///--------------------------------------------------------------------------------------

#if defined(powerc) || defined(__powerc)

pascal void SWTimeTask(
	TMTaskPtr tmTaskPtr)
{
	(*(TimeTaskPtr)tmTaskPtr).hasTaskFired = true;
}

#else	// 68k
#if THINK_C

pascal void SWTimeTask(void)
{
	asm
	{
		move.b #0x01, TimeTaskRec.hasTaskFired(A1)
	}
}

#else	// MetroWerks or MPW

	// %%% this inline needs to be updated if the TimeTaskRec structure ever changes
	// move.b #0x01, TimeTaskRec.hasTaskFired(A1)

void SWSetTaskHasFired() = { 0x137C, 0x0001, 0x0016 }; 

pascal void SWTimeTask(void)
{
	SWSetTaskHasFired();
}

#endif
#endif



///--------------------------------------------------------------------------------------
//	SWIsTaskPrimed
///--------------------------------------------------------------------------------------

SW_FUNC Boolean SWIsTaskPrimed(
	TMTask* tmTaskP)
{
	return (tmTaskP->qType & 0x8000) != 0;
}

