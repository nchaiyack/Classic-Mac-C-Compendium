///--------------------------------------------------------------------------------------
//	SpriteWorldUtils.h
//
//	Created:	Monday, January 18, 1993 at 8:57:36 PM
//	By:		Tony Myles
//
//	Copyright: © 1993-94 Tony Myles, All rights reserved worldwide.
//
//	Description:	constants, structures, and prototypes for sprite utilities
///--------------------------------------------------------------------------------------


#ifndef __SPRITEWORLDUTILS__
#define __SPRITEWORLDUTILS__

#ifndef __SWCOMMON__
#include "SWCommonHeaders.h"
#endif

#ifndef __QUICKDRAW__
#include <QuickDraw.h>
#endif

#ifndef __QDOFFSCREEN__
#include <QDOffscreen.h>
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


enum
{
	kColorIconResType = 'cicn'
};


///--------------------------------------------------------------------------------------
//	sprite utilities function prototypes
///--------------------------------------------------------------------------------------

#ifdef __cplusplus
extern "C" {
#endif

SW_FUNC OSErr SWCreateBestCGrafPort(
	CGrafPtr *newCGrafPort,
	Rect *offScreenRect);

SW_FUNC OSErr SWCreateCGrafPort(
	CGrafPtr *newCGrafPort,
	Rect *bounds,
	short depth,
	CTabHandle colors,
	GDHandle useGDevice);

SW_FUNC OSErr SWCreateCGrafPortFromCIconMask(
	CGrafPtr *newCGrafPort,
	CIconHandle cIconH);

SW_FUNC OSErr SWCreateCGrafPortFromPictResource(
	CGrafPtr *newCGrafPort,
	short pictResID);

SW_FUNC OSErr SWCreateCGrafPortFromPict(
	CGrafPtr *newCGrafPort,
	PicHandle srcPictH);

SW_FUNC OSErr SWSetUpPixMap(
	PixMapHandle aPixMap,
	short depth,
	Rect *bounds,
	CTabHandle colors,
	short bytesPerRow);

SW_FUNC OSErr SWCreateGDevice(
	GDHandle *retGDevice,
	PixMapHandle basePixMap);

SW_FUNC void SWDisposeCGrafPort(
	CGrafPtr doomedPort);

SW_FUNC OSErr SWCreateGrafPort(
	GrafPtr *newPort,
	Rect *newPortRect);

SW_FUNC OSErr SWCreateGrafPortFromPict(
	GrafPtr *offScrnPort,
	PicHandle srcPictH);

SW_FUNC OSErr SWCreateGrafPortFromPictResource(
	GrafPtr *offScrnPort,
	short pictResID);

SW_FUNC OSErr SWCreateGrafPortFromCIconMask(
	GrafPtr *newGrafPort,
	CIconHandle cIconH);

SW_FUNC void SWDisposeGrafPort(
	GrafPtr doomedPort);

SW_FUNC OSErr SWCreateRegionFromCIconMask(
	RgnHandle *maskRgn,
	CIconHandle cIconH);

SW_FUNC OSErr SWCreateRegionFromPict(
	RgnHandle *pictRgnH,
	PicHandle srcPictH);

SW_FUNC OSErr SWCreateRegionFromPictResource(
	RgnHandle *pictRgnH,
	short pictResID);

SW_FUNC OSErr SWGetCIcon(
	CIconHandle* cIconH,
	short iconResID);

SW_FUNC OSErr SWPlotCIcon(
	CIconHandle cIconH,
	Rect* iconRect);

SW_FUNC void SWDisposeCIcon(
	CIconHandle cIconH);

SW_FUNC OSErr SWCreateOptimumGWorld(
	GWorldPtr *optGWorld,
	Rect *devRect);

SW_FUNC OSErr SWCreateGWorldFromPictResource(
	GWorldPtr *pictGWorldP,
	short pictResID);

SW_FUNC OSErr SWCreateGWorldFromPict(
	GWorldPtr *pictGWorld,
	PicHandle srcPictH);

SW_FUNC OSErr SWCreateGWorldFromCIconResource(
	GWorldPtr *iconGWorldP,
	short iconResID);

SW_FUNC OSErr SWCreateGWorldFromCIcon(
	GWorldPtr *iconGWorldP,
	CIconHandle cIconH);

SW_FUNC OSErr SWCreateGWorldFromCIconMask(
	GWorldPtr *maskGWorldP,
	CIconHandle cIconH);

SW_FUNC Boolean SWHasColorQuickDraw(void);

SW_FUNC Boolean SWHasGWorlds(void);

#ifndef BitMapToRegionGlue
//pascal OSErr BitMapToRegionGlue(RgnHandle rgn, BitMap* bits);
#endif

#ifdef __cplusplus
};
#endif


#endif /* __SPRITEWORLDUTILS__ */

