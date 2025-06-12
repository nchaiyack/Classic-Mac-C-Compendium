///--------------------------------------------------------------------------------------
//	GameUtils.h
//
//	By:	Tony Myles
//
//	Copyright: © 1990-94 Tony Myles, All rights reserved worldwide
//
//	Description:	some utility functions for games
///--------------------------------------------------------------------------------------


#ifndef __GAMEUTILS__
#define __GAMEUTILS__

#ifndef __QUICKDRAW__
#include <QuickDraw.h>
#endif


#ifdef __cplusplus
extern "C" {
#endif


unsigned short GetRandom(unsigned short min, unsigned short max);
void HideMenuBar(GrafPtr grafPort);
void ShowMenuBar(GrafPtr grafPort);
void CenterRect(
	Rect* srcRect,
	Rect* dstRect);


#ifdef __cplusplus
};
#endif
#endif
