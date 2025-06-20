///--------------------------------------------------------------------------------------
//	SpriteLayer.h
//
//	Created:	Tuesday, October 27, 1992 at 10:10:06 PM
//	By:		Tony Myles
//
//	Copyright: � 1991-94 Tony Myles, All rights reserved worldwide
//
//	Description:	constants, structures, and prototypes for sprite layers
///--------------------------------------------------------------------------------------


#ifndef __SPRITELAYER__
#define __SPRITELAYER__

#ifndef __SWCOMMON__
#include "SWCommonHeaders.h"
#endif

#ifndef __SPRITE__
#include "Sprite.h"
#endif

extern RgnHandle gSWWorkRgn;


///--------------------------------------------------------------------------------------
//	sprite layer type definitions
///--------------------------------------------------------------------------------------

typedef struct SpriteLayerRec SpriteLayerRec;
typedef SpriteLayerRec *SpriteLayerPtr, **SpriteLayerHdl;


///--------------------------------------------------------------------------------------
//	sprite layer data structure
///--------------------------------------------------------------------------------------

struct SpriteLayerRec
{
	SpriteLayerPtr nextSpriteLayerP;	// next sprite layer
	SpriteLayerPtr prevSpriteLayerP;	// previous sprite layer

	SpritePtr headSpriteP;				// head of sprite linked list
	SpritePtr tailSpriteP;				// tail of sprite linked list

	long userData;						// reserved for user
};


///--------------------------------------------------------------------------------------
//	sprite layer function prototypes
///--------------------------------------------------------------------------------------

#ifdef __cplusplus
extern "C" {
#endif

SW_FUNC OSErr SWCreateSpriteLayer(
	SpriteLayerPtr *spriteLayerP);

SW_FUNC void SWDisposeSpriteLayer(
	SpriteLayerPtr spriteLayerP);

SW_FUNC void SWAddSprite(
	SpriteLayerPtr spriteLayerP,
	SpritePtr newSpriteP);

SW_FUNC void SWRemoveSprite(
	SpriteLayerPtr spriteLayerP,
	SpritePtr oldSpriteP);

SW_FUNC void SWRemoveAllSpritesFromLayer(
	SpriteLayerPtr srcSpriteLayerP);

SW_FUNC void SWSwapSprite(
	SpriteLayerPtr spriteLayerP,
	SpritePtr srcSpriteP,
	SpritePtr dstSpriteP);

SW_FUNC SpritePtr SWGetNextSprite(
	SpriteLayerPtr spriteLayerP,
	SpritePtr curSpriteP);

SW_FUNC void SWLockSpriteLayer(
	SpriteLayerPtr spriteLayerP);
SW_FUNC void SWUnlockSpriteLayer(
	SpriteLayerPtr spriteLayerP);

SW_FUNC void SWMoveSpriteLayer(
	SpriteLayerPtr spriteLayerP);

SW_FUNC void SWCollideSpriteLayer(
	SpriteLayerPtr srcSpriteLayerP,
	SpriteLayerPtr dstSpriteLayerP);

SW_FUNC SpritePtr SWFindSpriteByPoint(
	SpriteLayerPtr spriteLayerP,
	SpritePtr startSpriteP,
	Point testPoint);

#ifdef __cplusplus
};
#endif

#endif /* __SPRITELAYER__ */