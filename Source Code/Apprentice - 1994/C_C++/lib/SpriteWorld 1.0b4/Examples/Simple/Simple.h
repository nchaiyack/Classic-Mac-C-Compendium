///--------------------------------------------------------------------------------------
// Simple.h
//
// By: Tony Myles
//
// Copyright © 1993 Tony Myles, All rights reserved worldwide.
///--------------------------------------------------------------------------------------


#ifndef __SIMPLE__
#define __SIMPLE__

#ifndef __WINDOWS__
#include <Windows.h>
#endif

enum
{
	kSpriteCIconResID = 128,
	kNumberOfSpriteFrames = 1,
	kNumberOfSprites = 6,
	kSpriteMoveTime = 10,
	kHorizMoveDelta = 16,
	kVertMoveDelta = 16,
	kPixPatResID = 128
};

#ifdef __cplusplus
extern "C" {
#endif

void PerformSimpleAnimation(CWindowPtr srcWindowP);

#ifdef __cplusplus
};
#endif


#endif /* __APPLICATION__ */