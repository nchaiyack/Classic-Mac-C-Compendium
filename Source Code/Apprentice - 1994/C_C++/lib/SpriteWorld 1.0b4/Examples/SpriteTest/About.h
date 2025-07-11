//--------------------------------------------------------------------------------------
//	About.h
//
//	By:		Tony Myles
//
//	Copyright: � 1993-94 Tony Myles, All rights reserved worldwide.
///--------------------------------------------------------------------------------------


#ifndef __ABOUT__
#define __ABOUT__

#ifndef __DIALOGS__
#include <Dialogs.h>
#endif

#ifndef __EVENTS__
#include <Events.h>
#endif

#ifndef __SPRITE__
#include <Sprite.h>
#endif


enum
{
	kAboutDialogID = 1000,
	kEarthCIconID = 300,
	kShadowCIconID = 301,
	kNumberOfShadowFrames = 3,
	kInitialSpeed = 14,
	kMaxSpeed = 15,
	kReturnChar = 0x0D,
	kEnterChar = 0x03
};


#ifndef NewModalFilterProc
#define NewModalFilterProc(x) (x)
#endif


#ifdef __cplusplus
extern "C" {
#endif


void DisplayAboutBox(void);

SW_FUNC void ShadowFrameProc(
	SpritePtr srcSpriteP,
	FramePtr curFrameP,
	long* curFrameIndex);

SW_FUNC void EarthMoveProc(
	SpritePtr srcSpriteP,
	Point* spritePoint);

pascal Boolean AboutDialogFilter(
	DialogPtr aboutDialogP,
	EventRecord *event,
	short *itemHit);


#ifdef __cplusplus
};
#endif
#endif
