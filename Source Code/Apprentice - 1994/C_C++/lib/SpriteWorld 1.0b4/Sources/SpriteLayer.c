///--------------------------------------------------------------------------------------
//	SpriteLayer.c
//
//	Created:	Tuesday, October, 1992 at 10:10:06 PM
//	By:		Tony Myles
//
//	Copyright: © 1991-94 Tony Myles, All rights reserved worldwide
//
//	Description:	implementation of the sprite layers
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

#ifndef __SPRITELAYER__
#include "SpriteLayer.h"
#endif

#ifndef __SPRITE__
#include "Sprite.h"
#endif

#if MPW
#pragma segment SpriteWorld
#endif


///--------------------------------------------------------------------------------------
//	SWCreateSpriteLayer
///--------------------------------------------------------------------------------------

SW_FUNC OSErr SWCreateSpriteLayer(
	SpriteLayerPtr *spriteLayerP)
{
	OSErr err;
	SpriteLayerPtr tempSpriteLayerP;

	err = noErr;
	*spriteLayerP = NULL;

	tempSpriteLayerP = (SpriteLayerPtr)NewPtrClear((Size)sizeof(SpriteLayerRec));

	if (tempSpriteLayerP != NULL)
	{
		*spriteLayerP = tempSpriteLayerP;
	}
	else
	{
		err = MemError();
	}

	return err;
}


///--------------------------------------------------------------------------------------
//	SWDisposeSpriteLayer
///--------------------------------------------------------------------------------------

SW_FUNC void SWDisposeSpriteLayer(
	SpriteLayerPtr spriteLayerP)
{
	if (spriteLayerP != NULL)
	{
		DisposePtr((Ptr)spriteLayerP);
	}
}


///--------------------------------------------------------------------------------------
//	SWAddSprite
///--------------------------------------------------------------------------------------

SW_FUNC void SWAddSprite(
	SpriteLayerPtr spriteLayerP,
	SpritePtr newSpriteP)
{
	SpritePtr tailSpriteP = spriteLayerP->tailSpriteP;

	if (tailSpriteP != NULL)
	{
			// doubly link the new sprite
		tailSpriteP->nextSpriteP = newSpriteP;
		newSpriteP->prevSpriteP = tailSpriteP;
		newSpriteP->nextSpriteP = NULL;
	}	
	else
	{
		newSpriteP->prevSpriteP = NULL;
		newSpriteP->nextSpriteP = NULL;

			// make the new sprite the head
		spriteLayerP->headSpriteP = newSpriteP;
	}

		// make the new sprite the tail
	spriteLayerP->tailSpriteP = newSpriteP;
}


///--------------------------------------------------------------------------------------
//	SWRemoveSprite
///--------------------------------------------------------------------------------------

SW_FUNC void SWRemoveSprite(
	SpriteLayerPtr spriteLayerP,
	SpritePtr oldSpriteP)
{
		// is this not the tail sprite?
	if (oldSpriteP->nextSpriteP != NULL)
	{
			// link the next sprite to the prev sprite
		oldSpriteP->nextSpriteP->prevSpriteP = oldSpriteP->prevSpriteP;
	}
	else
	{
			// make the prev sprite the tail
		spriteLayerP->tailSpriteP = oldSpriteP->prevSpriteP;
	}

		// is this not the head sprite?
	if (oldSpriteP->prevSpriteP != NULL)
	{
			// link the prev sprite to the next sprite
		oldSpriteP->prevSpriteP->nextSpriteP = oldSpriteP->nextSpriteP;
	}
	else
	{
			// make the next sprite the first sprite
		spriteLayerP->headSpriteP = oldSpriteP->nextSpriteP;
	}

	oldSpriteP->prevSpriteP = NULL;
	oldSpriteP->nextSpriteP = NULL;
}


///--------------------------------------------------------------------------------------
//	SWRemoveAllSpritesFromLayer
///--------------------------------------------------------------------------------------

SW_FUNC void SWRemoveAllSpritesFromLayer(
	SpriteLayerPtr srcSpriteLayerP)
{
	SpritePtr curSpriteP;

	while ((curSpriteP = SWGetNextSprite(srcSpriteLayerP, NULL)) != NULL)
	{
		SWRemoveSprite(srcSpriteLayerP, curSpriteP);
	}
}


///--------------------------------------------------------------------------------------
//	SWSwapSprite
///--------------------------------------------------------------------------------------

SW_FUNC void SWSwapSprite(
	SpriteLayerPtr spriteLayerP,
	SpritePtr srcSpriteP,
	SpritePtr dstSpriteP)
{
	register SpritePtr swapSpriteP;
	
	swapSpriteP = srcSpriteP->nextSpriteP;
	srcSpriteP->nextSpriteP = dstSpriteP->nextSpriteP;
	dstSpriteP->nextSpriteP = swapSpriteP;

	swapSpriteP = srcSpriteP->prevSpriteP;
	srcSpriteP->prevSpriteP = dstSpriteP->prevSpriteP;
	dstSpriteP->prevSpriteP = swapSpriteP;

	if (srcSpriteP->nextSpriteP == NULL)
	{
		spriteLayerP->tailSpriteP = srcSpriteP;
	}
	else if (srcSpriteP->prevSpriteP == NULL)
	{
		spriteLayerP->headSpriteP = srcSpriteP;
	}

	if (dstSpriteP->nextSpriteP == NULL)
	{
		spriteLayerP->tailSpriteP = dstSpriteP;
	}
	else if (dstSpriteP->prevSpriteP == NULL)
	{
		spriteLayerP->headSpriteP = dstSpriteP;
	}
}


///--------------------------------------------------------------------------------------
//	SWGetNextSprite
///--------------------------------------------------------------------------------------

SW_FUNC SpritePtr SWGetNextSprite(
	SpriteLayerPtr spriteLayerP,
	SpritePtr curSpriteP)
{
	return (curSpriteP == NULL) ? spriteLayerP->headSpriteP : curSpriteP->nextSpriteP;
}


///--------------------------------------------------------------------------------------
//	SWLockSpriteLayer
///--------------------------------------------------------------------------------------

SW_FUNC void SWLockSpriteLayer(
	SpriteLayerPtr spriteLayerP)
{
	SpritePtr curSpriteP;

	curSpriteP = spriteLayerP->headSpriteP;

	while (curSpriteP != NULL)
	{
		SWLockSprite(curSpriteP);

		curSpriteP = curSpriteP->nextSpriteP;
	}
}


///--------------------------------------------------------------------------------------
//	SWUnlockSpriteLayer
///--------------------------------------------------------------------------------------

SW_FUNC void SWUnlockSpriteLayer(
	SpriteLayerPtr spriteLayerP)
{
	SpritePtr curSpriteP;

	curSpriteP = spriteLayerP->headSpriteP;

	while (curSpriteP != NULL)
	{
		SWUnlockSprite(curSpriteP);

		curSpriteP = curSpriteP->nextSpriteP;
	}
}


///--------------------------------------------------------------------------------------
//	SWCollideSpriteLayer
///--------------------------------------------------------------------------------------

SW_FUNC void SWCollideSpriteLayer(
	SpriteLayerPtr srcSpriteLayerP,
	SpriteLayerPtr dstSpriteLayerP)
{
	SpritePtr srcSpriteP;
	SpritePtr dstSpriteP;
	SpritePtr nextSrcSpriteP;
	SpritePtr nextDstSpriteP;
	Rect sectRect;

	srcSpriteP = srcSpriteLayerP->headSpriteP;

	while (srcSpriteP != NULL)
	{
		nextSrcSpriteP = srcSpriteP->nextSpriteP;
		dstSpriteP = dstSpriteLayerP->headSpriteP;

		while (dstSpriteP != NULL)
		{
			nextDstSpriteP = dstSpriteP->nextSpriteP;

			if (srcSpriteP != dstSpriteP)
			{
					// are the spriteÕs rectangles overlapping?
				if ((srcSpriteP->destFrameRect.top < dstSpriteP->destFrameRect.bottom) &&
					(srcSpriteP->destFrameRect.bottom > dstSpriteP->destFrameRect.top) &&
					(srcSpriteP->destFrameRect.left < dstSpriteP->destFrameRect.right) &&
					(srcSpriteP->destFrameRect.right > dstSpriteP->destFrameRect.left))
				{
						// call the source spriteÕs collision routine
					if (srcSpriteP->spriteCollideProc != NULL)
					{
						sectRect.left =
							SW_MAX(srcSpriteP->destFrameRect.left, dstSpriteP->destFrameRect.left);
						sectRect.top =
							SW_MAX(srcSpriteP->destFrameRect.top, dstSpriteP->destFrameRect.top);
						sectRect.right =
							SW_MIN(srcSpriteP->destFrameRect.right, dstSpriteP->destFrameRect.right);
						sectRect.bottom =
							SW_MIN(srcSpriteP->destFrameRect.bottom, dstSpriteP->destFrameRect.bottom);

						(*srcSpriteP->spriteCollideProc)(srcSpriteP, dstSpriteP, &sectRect);
					}
				}
			}

			if (nextDstSpriteP != NULL)
			{
				dstSpriteP = nextDstSpriteP;
			}
			else
			{
				dstSpriteP = dstSpriteP->nextSpriteP;
			}
		}

		if (nextSrcSpriteP != NULL)
		{
			srcSpriteP = nextSrcSpriteP;
		}
		else
		{
			srcSpriteP = srcSpriteP->nextSpriteP;
		}
	}
}


///--------------------------------------------------------------------------------------
//	SWFindSpriteByPoint
///--------------------------------------------------------------------------------------

SpritePtr SWFindSpriteByPoint(
	SpriteLayerPtr spriteLayerP,
	SpritePtr startSpriteP,
	Point testPoint)
{
	SpritePtr curSpriteP;

	curSpriteP = (startSpriteP == NULL) ? spriteLayerP->tailSpriteP : startSpriteP;

		// note that we traverse the sprites in reverse order
		// since this will make the most sense if we are
		// looking for a sprite that was clicked
	while ((curSpriteP != NULL) && !SWIsPointInSprite(curSpriteP, testPoint))
	{
		curSpriteP = curSpriteP->prevSpriteP;
	}

	return curSpriteP;
}
