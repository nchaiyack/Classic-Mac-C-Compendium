///--------------------------------------------------------------------------------------
//	Frame.h
//
//	Created:	Wednesday, October 28, 1992 at 8:39:10 PM
//	By:		Tony Myles
//
//	Copyright: © 1991-94 Tony Myles, All rights reserved worldwide
//
//	Description:	constants, structures, and prototypes for frames
///--------------------------------------------------------------------------------------


#ifndef __FRAME__
#define __FRAME__

#ifndef __SWCOMMON__
#include "SWCommonHeaders.h"
#endif

#ifndef __QDOFFSCREEN__
#include <QDOffscreen.h>
#endif

#ifndef __SPRITECOMPILER__
#include "SpriteCompiler.h"
#endif


///--------------------------------------------------------------------------------------
//	frame type definitions
///--------------------------------------------------------------------------------------

typedef struct FrameRec FrameRec;
typedef FrameRec *FramePtr, **FrameHdl;


///--------------------------------------------------------------------------------------
//	frame data structure
///--------------------------------------------------------------------------------------

struct FrameRec
{
	union
	{
		CGrafPtr colorGrafP;			// color port for the frame image
		GrafPtr monoGrafP;			// mono port for the frame image
	} framePort;

	union
	{
		PixMapPtr pixMapP;			// pointer color pix map (valid only while locked)
		BitMapPtr bitMapP;			// pointer mono bit map (valid only while locked)
	} framePix;

	char* frameBaseAddr;				// base address of pixel data (valid only while locked)
	unsigned long frameRowBytes;	// number of bytes in a row of the frame
	unsigned long frameRowLongs;	// number of long words in a row of the frame
	short leftAlignFactor;			// used to align the rect.left to the nearest long word 
	short rightAlignFactor;			// used to align the rect.right to the nearest long word
	Boolean isFrameLocked;			// has the frame been locked?
	Boolean isColor;					// is this a color frame?

	Rect frameRect;					// source image rectangle
	Point offsetPoint;				// image offset factor relative to destination rectangle
	RgnHandle maskRgn;				// image masking region

	union
	{
		CGrafPtr colorGrafP;			// color port for the mask image
		GrafPtr monoGrafP;			// mono port for the mask image
	} maskPort;

	union
	{
		PixMapPtr pixMapP;			// pointer to color pix map (valid only while locked)
		BitMapPtr bitMapP;			// pointer to mono bit map (valid only while locked)
	} maskPix;

	char* maskBaseAddr;				// base address of mask pixel data (valid only while locked)
	Boolean isMaskLocked;			// has the mask been locked?

	unsigned short useCount;		// number of sprites using this frame

	unsigned short numScanLines;
	unsigned long* scanLinePtrArray;

	PixelCodeHdl pixCodeH;
	BlitFuncPtr frameBlitterP;
};


///--------------------------------------------------------------------------------------
//	frame flags constants
///--------------------------------------------------------------------------------------

typedef enum
{
	kNoMask = 0,
	kPixelMask = 1,
	kRegionMask = 2,
	kCompiledMask = 4,
	kFatMask = (kPixelMask + kRegionMask),
	kCompiledFatMask = (kPixelMask + kRegionMask + kCompiledMask)
} MaskType;


///--------------------------------------------------------------------------------------
//	frame function prototypes
///--------------------------------------------------------------------------------------

#ifdef __cplusplus
extern "C" {
#endif

SW_FUNC OSErr SWCreateFrame(FramePtr* newFrameP, CGrafPtr srcGrafP, Rect* frameRect);
SW_FUNC OSErr SWCreateFrameFromCIconResource(FramePtr* newFrameP, short iconResID, MaskType maskType);
SW_FUNC OSErr SWCreateFrameFromPictResource(FramePtr* newFrameP, short pictResID, short maskResID, MaskType maskType);
SW_FUNC void SWDisposeFrame(FramePtr oldFrameP);

SW_FUNC void SWSetFrameGWorld(FramePtr srcFrameP, GWorldPtr frameGWorldP);
SW_FUNC void SWSetFrameMaskRgn(FramePtr srcFrameP, RgnHandle maskRgn);
SW_FUNC void SWSetFrameMaskGWorld(FramePtr srcFrameP, GWorldPtr maskGWorldP);
SW_FUNC RgnHandle SWGetFrameMaskRgn(FramePtr srcFrameP);
SW_FUNC void SWSetFrameRect(FramePtr srcFrameP, Rect* newFrameRect);

SW_FUNC void SWLockFrame(FramePtr srcFrameP);
SW_FUNC void SWUnlockFrame(FramePtr srcFrameP);

SW_FUNC void SWCopyFrame(FramePtr srcFrameP, FramePtr dstFrameP);

#ifdef __cplusplus
};
#endif

#endif	/* __FRAME__ */