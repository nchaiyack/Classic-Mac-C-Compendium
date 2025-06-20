///--------------------------------------------------------------------------------------
//	SpriteWorld.h
//
//	Created:	Wednesday, May 29, 1991 at 10:43:28 PM
//	By:				Tony Myles
//
//	Copyright: � 1991-94 Tony Myles, All rights reserved worldwide
//
//	Description:	constants, structures, and prototypes for sprite worlds
///--------------------------------------------------------------------------------------


#ifndef __SPRITEWORLD__
#define __SPRITEWORLD__

#ifndef __SWCOMMON__
#include "SWCommonHeaders.h"
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


///--------------------------------------------------------------------------------------
//	sprite world error constants
///--------------------------------------------------------------------------------------

enum
{
	kQuickDrawTooOldErr = 100,
	kTimeMgrNotPresentErr,
	kTimeMgrTooOldErr,
	kMaxFramesErr,					// attempt to exceed maximum number of frames for a sprite
	kInvalidFramesIndexErr	// frame index out of range
};




///--------------------------------------------------------------------------------------
//	sprite world type definitions
///--------------------------------------------------------------------------------------

typedef struct SpriteWorldRec SpriteWorldRec;
typedef SpriteWorldRec *SpriteWorldPtr, **SpriteWorldHdl;


#ifdef __cplusplus
extern "C" {
#endif

typedef SW_FUNC void (*WorldDrawProcPtr)(
	FramePtr srcFrameP,
	FramePtr dstFrameP,
	Rect *drawRect);

#ifdef __cplusplus
};
#endif

///--------------------------------------------------------------------------------------
//	sprite world data structure
///--------------------------------------------------------------------------------------

struct SpriteWorldRec
{
	SpriteLayerPtr headSpriteLayerP;	// head of the sprite layer linked list
	SpriteLayerPtr tailSpriteLayerP;	// tail of the sprite layer linked list

	FramePtr windowFrameP;				// frame for drawing to the screen
	FramePtr backFrameP;					// frame for drawing from the background
	FramePtr loadFrameP;					// frame for drawing to the loader

	WorldDrawProcPtr eraseDrawProc;			// callback for erasing sprites offscreen
	WorldDrawProcPtr screenDrawProc;		// callback for drawing sprite pieces onscreen

	long userData;							// reserved for user
};


///--------------------------------------------------------------------------------------
//	sprite world function prototypes
///--------------------------------------------------------------------------------------


#ifdef __cplusplus
extern "C" {
#endif

SW_FUNC OSErr SWEnterSpriteWorld(void);

SW_FUNC void SWExitSpriteWorld(void);

SW_FUNC OSErr SWCreateSpriteWorld(
	SpriteWorldPtr *spriteWorldP,
	FramePtr windowFrameP,
	FramePtr backFrameP,
	FramePtr loadFrameP);

SW_FUNC OSErr SWCreateSpriteWorldFromWindow(
	SpriteWorldPtr* spriteWorldP,
	CWindowPtr srcWindowP,
	Rect* worldRect);

SW_FUNC void SWDisposeSpriteWorld(
	SpriteWorldPtr spriteWorldP);

SW_FUNC void SWAddSpriteLayer(
	SpriteWorldPtr spriteWorldP,
	SpriteLayerPtr spriteLayerP);

SW_FUNC void SWRemoveSpriteLayer(
	SpriteWorldPtr spriteWorldP,
	SpriteLayerPtr spriteLayerP);

SW_FUNC void SWSwapSpriteLayer(
	SpriteWorldPtr spriteWorldP,
	SpriteLayerPtr srcSpriteLayerP,
	SpriteLayerPtr dstSpriteLayerP);

SW_FUNC SpriteLayerPtr SWGetNextSpriteLayer(
	SpriteWorldPtr spriteWorldP,
	SpriteLayerPtr curSpriteLayerP);

SW_FUNC void SWLockSpriteWorld(
	SpriteWorldPtr spriteWorldP);

SW_FUNC void SWUnlockSpriteWorld(
	SpriteWorldPtr spriteWorldP);

SW_FUNC void SWSetPortToBackGround(
	SpriteWorldPtr spriteWorldP);

SW_FUNC void SWSetPortToWindow(
	SpriteWorldPtr spriteWorldP);

SW_FUNC void SWSetSpriteWorldEraseProc(
	SpriteWorldPtr spriteWorldP,
	WorldDrawProcPtr eraseProc);

SW_FUNC void SWSetSpriteWorldDrawProc(
	SpriteWorldPtr spriteWorldP,
	WorldDrawProcPtr drawProc);

SW_FUNC void SWStdWorldDrawProc(
	FramePtr srcFrameP,
	FramePtr dstFrameP,
	Rect* drawRect);

SW_FUNC void SWUpdateSpriteWorld(
	SpriteWorldPtr spriteWorldP);

SW_FUNC void SWProcessSpriteWorld(
	SpriteWorldPtr spriteWorldP);

SW_FUNC void SWAnimateSpriteWorld(
	SpriteWorldPtr spriteWorldP);

SW_FUNC void SWFastAnimateSpriteWorld(
	SpriteWorldPtr spriteWorldP);

SW_FUNC void SWBlastAnimateSpriteWorld(
	SpriteWorldPtr spriteWorldP);

void SWCheckIdleSpriteOverlap(
	SpriteWorldPtr spriteWorldP,
	SpritePtr curSpriteP,
	SpritePtr headActiveSpriteP);

void SWFastCheckIdleSpriteOverlap(
	SpriteWorldPtr spriteWorldP,
	SpritePtr curSpriteP,
	SpritePtr headActiveSpriteP);

#ifdef __cplusplus
};
#endif

#endif /* __SPRITEWORLD__ */