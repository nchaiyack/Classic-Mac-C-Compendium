///--------------------------------------------------------------------------------------
//	Frame.c
//
//	Created:	Sunday, November 15, 1992 at 2:37:37 AM
//	By:		Tony Myles
//
//	Copyright: © 1991-94 Tony Myles, All rights reserved worldwide
//
//	Description:	implementation of the frame stuff
///--------------------------------------------------------------------------------------


#ifndef __SWCOMMON__
#include "SWCommonHeaders.h"
#endif

#ifndef __TOOLUTILS__
#include <ToolUtils.h>
#endif

#ifndef __MEMORY__
#include <Memory.h>
#endif

#ifndef __RESOURCES__
#include <Resources.h>
#endif

#ifndef __ERRORS__
#include <Errors.h>
#endif

#ifndef __SPRITEWORLDUTILS__
#include "SpriteWorldUtils.h"
#endif

#ifndef __SPRITECOMPILER__
#include "SpriteCompiler.h"
#endif

#ifndef __FRAME__
#include "Frame.h"
#endif

#if MPW
#pragma segment SpriteWorld
#endif

///--------------------------------------------------------------------------------------
//	SWCreateFrame
///--------------------------------------------------------------------------------------

SW_FUNC OSErr SWCreateFrame(
	FramePtr* newFrameP,
	CGrafPtr srcGrafP,
	Rect* frameRect)
{
	OSErr err = noErr;
	FramePtr tempFrameP;
	short bitsPerPixel;
	long numScanLines;
	
	numScanLines = frameRect->bottom - frameRect->top;

	*newFrameP = NULL;

	tempFrameP = (FramePtr)NewPtrClear((Size)sizeof(FrameRec) + (sizeof(unsigned long) * numScanLines));

	if (tempFrameP != NULL)
	{
		tempFrameP->isColor = SWHasColorQuickDraw();

		if (srcGrafP == NULL)
		{
			if (tempFrameP->isColor)
			{
				err = SWCreateBestCGrafPort(&tempFrameP->framePort.colorGrafP, frameRect);
			}
			else
			{
				err = SWCreateGrafPort(&tempFrameP->framePort.monoGrafP, frameRect);
			}
		}
		else
		{
			tempFrameP->framePort.colorGrafP = srcGrafP;
		}

		if (err == noErr)
		{
			tempFrameP->frameRect = tempFrameP->framePort.colorGrafP->portRect;

				// cache frameRowBytes and frameRowLongs for use by our blitter routine
			tempFrameP->frameRowBytes = tempFrameP->isColor ?
					(**tempFrameP->framePort.colorGrafP->portPixMap).rowBytes & 0x7FFF :
					tempFrameP->framePort.monoGrafP->portBits.rowBytes;

			tempFrameP->frameRowLongs = (tempFrameP->frameRect.right -
					tempFrameP->frameRect.left) >> 2;

				// this calculation generates a mask value that we use to
				// long word align the rectangle when we draw the frame.
				// note that the expression "sizeof(long) * kBitsPerByte" gives us
				// the number of bits in a long.
			bitsPerPixel = tempFrameP->isColor ?
					(**tempFrameP->framePort.colorGrafP->portPixMap).pixelSize : 1;			
			tempFrameP->rightAlignFactor = ((sizeof(long) * kBitsPerByte) / bitsPerPixel) - 1;
			tempFrameP->leftAlignFactor = ~(tempFrameP->rightAlignFactor);

				// the useCount keeps track of the number of sprites that
				// are using this frame. we need to know this so we don't
				// dispose this frame twice when we are disposing the 
				// sprites that use it.
			tempFrameP->useCount = 0;

				// here we set up an array of offsets to the scan lines of
				// this frame. this allows us to address a particular scan line
				// without doing a costly multiply.
			tempFrameP->numScanLines = numScanLines;
			tempFrameP->scanLinePtrArray = (unsigned long*)(tempFrameP + 1);

			for (numScanLines = 0; numScanLines < tempFrameP->numScanLines; numScanLines++)
			{
				tempFrameP->scanLinePtrArray[numScanLines] = (numScanLines * tempFrameP->frameRowBytes);
			}


			*newFrameP = tempFrameP;
		}
	}
	else
	{
		err = MemError();
	}

	if (err != noErr)
	{
		if (tempFrameP != NULL)
		{
			if (tempFrameP->framePort.colorGrafP != NULL)
			{
				if (tempFrameP->isColor)
				{
					SWDisposeCGrafPort(tempFrameP->framePort.colorGrafP);
				}
				else
				{
					SWDisposeGrafPort((GrafPtr)tempFrameP->framePort.monoGrafP);
				}
			}

			DisposePtr((Ptr)tempFrameP);
		}
	}

	return err;
}


///--------------------------------------------------------------------------------------
//	SWCreateFrameFromCIconResource
///--------------------------------------------------------------------------------------

SW_FUNC OSErr SWCreateFrameFromCIconResource(
	FramePtr* newFrameP,
	short iconResID,
	MaskType maskType)
{
	OSErr err;
	GrafPtr savePort;
	FramePtr tempFrameP;
	CIconHandle cIconH;
	RgnHandle maskRgn;
	Rect frameRect;

	*newFrameP = NULL;
	tempFrameP = NULL;
	cIconH = NULL;

	GetPort(&savePort);

	err = SWGetCIcon(&cIconH, iconResID);

	if (err == noErr)
	{
		HNoPurge((Handle)cIconH);

		frameRect = (**cIconH).iconPMap.bounds;

		err = SWCreateFrame(&tempFrameP, NULL, &frameRect);
	}

	if (err == noErr)
	{
		SetPort(tempFrameP->framePort.monoGrafP);

		SWPlotCIcon(cIconH, &frameRect);
	}

	if (err == noErr)
	{
			// make a region mask
		if ((maskType & kRegionMask) != 0)
		{
			err = SWCreateRegionFromCIconMask(&maskRgn, cIconH);

			if (err == noErr)
			{
				SWSetFrameMaskRgn(tempFrameP, maskRgn);
			}
		}
	}

	if (err == noErr)
	{
			// make a pixel mask
		if ((maskType & kPixelMask) != 0)
		{
			if (tempFrameP->isColor)
			{
				err = SWCreateCGrafPortFromCIconMask(&tempFrameP->maskPort.colorGrafP, cIconH);
			}
			else
			{
				err = SWCreateGrafPortFromCIconMask(&tempFrameP->maskPort.monoGrafP, cIconH);
			}

				// %%% pre-xor mask experiment
			if (err == noErr)
			{
				SetPort(tempFrameP->maskPort.monoGrafP);
				InvertRect(&tempFrameP->maskPort.monoGrafP->portRect);
			}
		}
	}

	if (err == noErr)
	{
			// load compiled mask
		if ((maskType & kCompiledMask) != 0)
		{
			tempFrameP->pixCodeH = (PixelCodeHdl)GetResource(kPixelCodeResType, iconResID);
			err = ResError();

			if ((tempFrameP->pixCodeH == NULL) && (err == noErr))
				err = resNotFound;

			if (err == noErr)
			{
				HNoPurge((Handle)tempFrameP->pixCodeH);
				DetachResource((Handle)tempFrameP->pixCodeH);
			}
		}
	}

	if (cIconH != NULL)
	{
		SWDisposeCIcon(cIconH);
	}

	if (err == noErr)
	{
		*newFrameP = tempFrameP;
	}

	if (err != noErr)
	{
			// an error occurred so dispose of anything we managed to create
		if (tempFrameP != NULL)
		{
			SWDisposeFrame(tempFrameP);
		}
	}

	SetPort(savePort);

	return err;
}


///--------------------------------------------------------------------------------------
//	SWCreateFrameFromPictResource
///--------------------------------------------------------------------------------------

SW_FUNC OSErr SWCreateFrameFromPictResource(
	FramePtr* newFrameP,
	short pictResID,
	short maskResID,
	MaskType maskType)
{
	OSErr err;
	GrafPtr savePort;
	PicHandle newPictH;
	FramePtr tempFrameP;
	RgnHandle maskRgn;
	Rect frameRect;

	tempFrameP = NULL;
	*newFrameP = NULL;

	GetPort(&savePort);

	newPictH = GetPicture(pictResID);

	if (newPictH != NULL)
	{
		frameRect.top = 0;
		frameRect.left = 0;
		frameRect.right = (**newPictH).picFrame.right - (**newPictH).picFrame.left;
		frameRect.bottom = (**newPictH).picFrame.bottom - (**newPictH).picFrame.top;

		err = SWCreateFrame(&tempFrameP, NULL, &frameRect);

		if (err == noErr)
		{
			SetPort(tempFrameP->framePort.monoGrafP);

			DrawPicture(newPictH, &frameRect);
		}

			// make a region mask
		if (((maskType & kRegionMask) != 0) && (err == noErr))
		{
			err = SWCreateRegionFromPictResource(&maskRgn, maskResID);

			if (err == noErr)
			{
				SWSetFrameMaskRgn(tempFrameP, maskRgn);
			}
		}

			// make a pixel mask
		if (((maskType & kPixelMask) != 0) && (err == noErr))
		{
			if (tempFrameP->isColor)
			{
				err = SWCreateCGrafPortFromPictResource(&tempFrameP->maskPort.colorGrafP, maskResID);
			}
			else
			{
				err = SWCreateGrafPortFromPictResource(&tempFrameP->maskPort.monoGrafP, maskResID);
			}

				// %%% pre-xor mask experiment
			if (err == noErr)
			{
				SetPort(tempFrameP->maskPort.monoGrafP);
				InvertRect(&tempFrameP->maskPort.monoGrafP->portRect);
			}
		}

		if (err == noErr)
		{
			*newFrameP = tempFrameP;
		}
		else
		{
				// an error occurred so dispose of anything we managed to create
			if (tempFrameP != NULL)
			{
				SWDisposeFrame(tempFrameP);
			}
		}

		ReleaseResource((Handle)newPictH);
	}
	else
	{
		err = ResError();

		if (err == noErr)
		{
			err = resNotFound;
		}
	}

	SetPort(savePort);

	return err;
}


///--------------------------------------------------------------------------------------
//	SWDisposeFrame
///--------------------------------------------------------------------------------------

SW_FUNC void SWDisposeFrame(
	FramePtr oldFrameP)
{
	if (oldFrameP != NULL)
	{
			// is this frame still in use by another sprite?
		if (oldFrameP->useCount > 1)
		{
				// one less sprite is using it now!
			oldFrameP->useCount--;
		}
		else	// no more sprites are using this frame
		{
			if (oldFrameP->framePort.colorGrafP != NULL)
			{
				if (oldFrameP->isColor)
				{
					SWDisposeCGrafPort(oldFrameP->framePort.colorGrafP);
				}
				else
				{
					SWDisposeGrafPort(oldFrameP->framePort.monoGrafP);
				}
	
				oldFrameP->framePort.colorGrafP = NULL;
			}

			if (oldFrameP->maskRgn != NULL)
			{
				DisposeRgn(oldFrameP->maskRgn);
	
				oldFrameP->maskRgn = NULL;
			}

			if (oldFrameP->maskPort.colorGrafP != NULL)
			{
				if (oldFrameP->isColor)
				{
					SWDisposeCGrafPort(oldFrameP->maskPort.colorGrafP);
				}
				else
				{
					SWDisposeGrafPort(oldFrameP->maskPort.monoGrafP);
				}

				oldFrameP->maskPort.colorGrafP = NULL;
			}

			if (oldFrameP->pixCodeH != NULL)
			{
				DisposeHandle((Handle)oldFrameP->pixCodeH);
				oldFrameP->pixCodeH = NULL;
			}

			DisposePtr((Ptr)oldFrameP);
		}
	}
}


///--------------------------------------------------------------------------------------
//	SWSetFrameMaskRgn
///--------------------------------------------------------------------------------------

SW_FUNC void SWSetFrameMaskRgn(
	FramePtr srcFrameP,
	RgnHandle maskRgn)
{
	srcFrameP->maskRgn = maskRgn;

	srcFrameP->offsetPoint.h = (**maskRgn).rgnBBox.left;
	srcFrameP->offsetPoint.v = (**maskRgn).rgnBBox.top;
}


///--------------------------------------------------------------------------------------
//	SWGetFrameMaskRgn
///--------------------------------------------------------------------------------------

SW_FUNC RgnHandle SWGetFrameMaskRgn(
	FramePtr srcFrameP)
{
	return srcFrameP->maskRgn;
}


///--------------------------------------------------------------------------------------
//	SWLockFrame
///--------------------------------------------------------------------------------------

SW_FUNC void SWLockFrame(
	FramePtr srcFrameP)
{
	srcFrameP->isFrameLocked = true;

	if (srcFrameP->isColor)
	{
		PixMapHandle pixMapH;

		pixMapH = srcFrameP->framePort.colorGrafP->portPixMap;
		HLock((Handle)pixMapH);

		srcFrameP->framePix.pixMapP = *pixMapH;
		srcFrameP->frameBaseAddr = (**pixMapH).baseAddr;
		srcFrameP->frameRowBytes = (**pixMapH).rowBytes & 0x7FFF;

		if (srcFrameP->pixCodeH != NULL)
		{
			HLock((Handle)srcFrameP->pixCodeH);
			srcFrameP->frameBlitterP = (BlitFuncPtr)*srcFrameP->pixCodeH;
		}

		if (srcFrameP->maskPort.colorGrafP != NULL)
		{
			pixMapH = srcFrameP->maskPort.colorGrafP->portPixMap;
			HLock((Handle)pixMapH);

			srcFrameP->isMaskLocked = true;

			srcFrameP->maskPix.pixMapP = *pixMapH;

			srcFrameP->maskBaseAddr = (**pixMapH).baseAddr;
		}
	}
	else
	{
		srcFrameP->framePix.bitMapP = &srcFrameP->framePort.monoGrafP->portBits;
		srcFrameP->frameBaseAddr = srcFrameP->framePix.bitMapP->baseAddr;

		if (srcFrameP->maskPort.monoGrafP != NULL)
		{
			srcFrameP->maskPix.bitMapP = &srcFrameP->maskPort.monoGrafP->portBits;
			srcFrameP->maskBaseAddr = srcFrameP->maskPix.bitMapP->baseAddr;
		}
	}
}


///--------------------------------------------------------------------------------------
//	SWUnlockFrame
///--------------------------------------------------------------------------------------

SW_FUNC void SWUnlockFrame(
	FramePtr srcFrameP)
{
	if (srcFrameP->isColor)
	{
		if (srcFrameP->framePort.colorGrafP != NULL)
		{
			HUnlock((Handle)srcFrameP->framePort.colorGrafP->portPixMap);
		}

		if (srcFrameP->maskPort.colorGrafP != NULL)
		{
			HUnlock((Handle)srcFrameP->maskPort.colorGrafP->portPixMap);
		}
	}

	srcFrameP->isFrameLocked = false;
	srcFrameP->isMaskLocked = false;

	srcFrameP->framePix.pixMapP = NULL;
	srcFrameP->frameBaseAddr = NULL;
	srcFrameP->maskPix.pixMapP = NULL;
	srcFrameP->maskBaseAddr = NULL;

	if (srcFrameP->pixCodeH != NULL)
	{
		HUnlock((Handle)srcFrameP->pixCodeH);
		srcFrameP->frameBlitterP = NULL;
	}
}


///--------------------------------------------------------------------------------------
//	SWCopyFrame
///--------------------------------------------------------------------------------------

SW_FUNC void SWCopyFrame(
	FramePtr srcFrameP,
	FramePtr dstFrameP)
{
	if (srcFrameP->isFrameLocked)
	{
		CopyBits(srcFrameP->framePix.bitMapP,
					dstFrameP->framePix.bitMapP,
					&srcFrameP->frameRect,
					&srcFrameP->frameRect,
					srcCopy, srcFrameP->maskRgn);
	}
}


///--------------------------------------------------------------------------------------
//	SWSetFrameRect
///--------------------------------------------------------------------------------------

SW_FUNC void SWSetFrameRect(
	FramePtr srcFrameP,
	Rect* newFrameRect)
{
	srcFrameP->frameRect = *newFrameRect;
	srcFrameP->frameRowLongs = (newFrameRect->right - newFrameRect->left) >> 2;
}


