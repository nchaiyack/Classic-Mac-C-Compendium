///--------------------------------------------------------------------------------------
//	Sprite.h
//
//	Created:	Tuesday, October 27, 1992 at 10:10:06 PM
//	By:		Tony Myles
//
//	Copyright: � 1991-94 Tony Myles, All rights reserved worldwide
//
//	Description:	constants, structures, and prototypes for sprites
///--------------------------------------------------------------------------------------


#ifndef __SPRITE__
#define __SPRITE__

#ifndef __SWCOMMON__
#include "SWCommonHeaders.h"
#endif

#ifndef __TIMER__
#include <Timer.h>
#endif

#ifndef __FRAME__
#include "Frame.h"
#endif


///--------------------------------------------------------------------------------------
//	time task data structure
///--------------------------------------------------------------------------------------

typedef struct TimeTaskRec TimeTaskRec;
typedef TimeTaskRec *TimeTaskPtr, **TimeTaskHdl;

struct TimeTaskRec
{
	TMTask timeTask;
	Boolean hasTaskFired;
};

#ifndef NewTimerProc
#define NewTimerProc(x) (x)
#endif


///--------------------------------------------------------------------------------------
//	sprite type definitions
///--------------------------------------------------------------------------------------

typedef struct SpriteRec SpriteRec;
typedef SpriteRec *SpritePtr, **SpriteHdl;

#ifdef __cplusplus
extern "C" {
#endif

typedef SW_FUNC void (*DrawProcPtr)(
	FramePtr srcFrameP,
	FramePtr dstFrameP,
	Rect *srcRect,
	Rect *dstRect);

typedef SW_FUNC void (*FrameProcPtr)(
	SpritePtr srcSpriteP,
	FramePtr curFrameP,
	long *frameIndex);

typedef SW_FUNC void (*MoveProcPtr)(
	SpritePtr srcSpriteP,
	Point *spriteLoc);

typedef SW_FUNC void (*CollideProcPtr)(
	SpritePtr srcSpriteP,
	SpritePtr dstSpriteP,
	Rect* sectRect);

#ifdef __cplusplus
};
#endif

///--------------------------------------------------------------------------------------
//	sprite data structure
///--------------------------------------------------------------------------------------

struct SpriteRec
{
	SpritePtr nextSpriteP;					// next sprite
	SpritePtr prevSpriteP;					// previous sprite
	SpritePtr nextActiveSpriteP;
	SpritePtr nextIdleSpriteP;

		// drawing fields
	Boolean isVisible;						// draw the sprite?
	Boolean needsToBeDrawn;					// sprite has changed, needs to be drawn
	Boolean needsToBeErased;				// sprite needs to be erased onscreen
	char pad1;
	Rect destFrameRect;						// frame destination rectangle
	Rect oldFrameRect;						// last frame destination rectangle
	Rect deltaFrameRect;						// union of the sprite�s lastRect and curRect
	RgnHandle screenMaskRgn;				// mask region for drawing sprite to screen
	DrawProcPtr frameDrawProc;				// callback to draw sprite

		// frame fields
	TimeTaskRec frameTimeTask;				// frame advance time task
	FramePtr *frameArray;					// array of frames
	FramePtr curFrameP;						// current frame
	long numFrames;							// number of frames
	long maxFrames;							// maximum number of frames
	long frameTimeInterval;					// time interval to advance frame
	long frameAdvance;						// amount the adjust frame index
	long curFrameIndex;						// current frame index
	long firstFrameIndex;					// first frame to advance
	long lastFrameIndex;						// last frame to advance
	FrameProcPtr frameChangeProc;			// callback to change frames

		// movement fields
	TimeTaskRec moveTimeTask;				// movement time task
	long moveTimeInterval;					// time interval to move sprite
	short horizMoveDelta;					// horizontal movement delta
	short vertMoveDelta;						// vertical movement delta
	Rect moveBoundsRect;						// bounds of the sprite�s movement
	MoveProcPtr spriteMoveProc;			// callback to handle movement

		// collision detection fields
	Rect collideRect;							// collision detection rectangle (not used currently)
	CollideProcPtr spriteCollideProc;	// callback to handle collisions

		// miscellaneous
	long userData;								// reserved for user
};


///--------------------------------------------------------------------------------------
//	sprite function prototypes
///--------------------------------------------------------------------------------------

#ifdef __cplusplus
extern "C" {
#endif

SW_FUNC OSErr SWCreateSprite(
	SpritePtr* newSpriteP,
	void* spriteStorageP,
	short maxFrames);

SW_FUNC OSErr SWCreateSpriteFromCIconResource(
	SpritePtr* newSpriteP,
	void* spriteStorageP,
	short cIconID,
	short maxFrames,
	MaskType maskType);

SW_FUNC OSErr SWCreateSpriteFromPictResource(
	SpritePtr* newSpriteP,
	void* spriteStorageP,
	short pictResID,
	short maskResID,
	short maxFrames,
	MaskType maskType);

SW_FUNC OSErr SWCloneSprite(
	SpritePtr cloneSpriteP,
	SpritePtr *newSpriteP,
	void* spriteStorageP);

SW_FUNC void SWDisposeSprite(
	SpritePtr oldSpriteP);

SW_FUNC void SWCloseSprite(
	SpritePtr deadSpriteP);

SW_FUNC void SWLockSprite(
	SpritePtr srcSpriteP);

SW_FUNC void SWUnlockSprite(
	SpritePtr srcSpriteP);

SW_FUNC void SWSetSpriteDrawProc(
	SpritePtr srcSpriteP,
	DrawProcPtr drawProc);

SW_FUNC OSErr SWAddFrame(
	SpritePtr srcSpriteP,
	FramePtr newFrameP);

SW_FUNC void SWRemoveFrame(
	SpritePtr srcSpriteP,
	FramePtr oldFrameP);

SW_FUNC void SWSetCurrentFrame(
	SpritePtr srcSpriteP,
	FramePtr curFrameP);

SW_FUNC void SWSetCurrentFrameIndex(
	SpritePtr srcSpriteP,
	short frameIndex);

SW_FUNC void SWSetSpriteFrameAdvance(
	SpritePtr srcSpriteP,
	short frameAdvance);

SW_FUNC void SWSetSpriteFrameRange(
	SpritePtr srcSpriteP,
	short firstFrameIndex,
	short lastFrameIndex);

SW_FUNC void SWSetSpriteFrameTime(
	SpritePtr srcSpriteP,
	long timeInterval);

SW_FUNC void SWSetSpriteFrameProc(
	SpritePtr srcSpriteP,
	FrameProcPtr frameProc);

SW_FUNC void SWMoveSprite(
	SpritePtr srcSpriteP,
	short horizLoc,
	short vertLoc);

SW_FUNC void SWOffsetSprite(
	SpritePtr srcSpriteP,
	short horizDelta,
	short vertDelta);

SW_FUNC void SWSetSpriteLocation(
	SpritePtr srcSpriteP,
	short horizLoc,
	short vertLoc);

SW_FUNC void SWSetSpriteMoveBounds(
	SpritePtr srcSpriteP,
	Rect* moveBoundsRect);

SW_FUNC void SWSetSpriteMoveDelta(
	SpritePtr srcSpriteP,
	short horizDelta,
	short vertDelta);

SW_FUNC void SWSetSpriteMoveTime(
	SpritePtr srcSpriteP,
	long timeInterval);

SW_FUNC void SWSetSpriteMoveProc(
	SpritePtr srcSpriteP, MoveProcPtr moveProc);

SW_FUNC void SWBounceSpriteMoveProc(
	SpritePtr srcSpriteP,
	Point* spritePoint);

SW_FUNC void SWWrapSpriteMoveProc(
	SpritePtr srcSpriteP,
	Point* spritePoint);

SW_FUNC void SWSetSpriteCollideProc(
	SpritePtr srcSpriteP,
	CollideProcPtr collideProc);

SW_FUNC void SWSetSpriteVisible(
	SpritePtr srcSpriteP,
	Boolean isVisible);

SW_FUNC Boolean SWIsSpriteInRect(
	SpritePtr srcSpriteP,
	Rect* testRect);

SW_FUNC Boolean SWIsPointInSprite(
	SpritePtr srcSpriteP,
	Point testPoint);

SW_FUNC void SWStdDrawProc(
	FramePtr srcFrameP,
	FramePtr dstFrameP,
	Rect* srcRect,
	Rect* dstRect);

#if defined(powerc) || defined(__powerc)
pascal void SWTimeTask(
	TMTaskPtr tmTaskPtr);
#else
pascal void SWTimeTask(void);
#endif

SW_FUNC Boolean SWIsTaskPrimed(
	TMTask* tmTaskP);

#ifdef __cplusplus
};
#endif


#endif	/* __SPRITE__ */

