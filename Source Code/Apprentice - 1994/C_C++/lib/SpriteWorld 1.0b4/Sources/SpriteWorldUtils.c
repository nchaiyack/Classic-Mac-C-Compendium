///--------------------------------------------------------------------------------------
//	SpriteWorldUtils.c
//
//	Created:	Monday, January 18, 1993 at 8:57:36 PM
//	By:		Tony Myles
//
//	Copyright: © 1993-94 Tony Myles, All rights reserved worldwide.
//
//	Description:	some utilities for creating worlds of sprites
///--------------------------------------------------------------------------------------


#ifndef __SWCOMMON__
#include "SWCommonHeaders.h"
#endif

#ifndef __QUICKDRAW__
#include <QuickDraw.h>
#endif

#ifndef __QDOFFSCREEN__
#include <QDOffscreen.h>
#endif

#ifndef __WINDOWS__
#include <Windows.h>
#endif

#ifndef __MEMORY__
#include <Memory.h>
#endif

#ifndef __GESTALTEQU__
#include <GestaltEqu.h>
#endif

#ifndef __TOOLUTILS__
#include <ToolUtils.h>
#endif

#ifndef __RESOURCES__
#include <Resources.h>
#endif

#ifndef __ERRORS__
#include <Errors.h>
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

#ifndef __SPRITEWORLDUTILS__
#include "SpriteWorldUtils.h"
#endif

#if MPW
#pragma segment SpriteWorld
#endif

///--------------------------------------------------------------------------------------
// SWCreateBestCGrafPort										Adapted from code by Forrest Tanaka
///--------------------------------------------------------------------------------------

SW_FUNC OSErr SWCreateBestCGrafPort(
	CGrafPtr	*newCGrafPort,					// returns a pointer to the new CGrafPort
	Rect		*offScreenRect)				// global rectangle of part of screen to save
{
	OSErr				err;						// err code
	GDHandle			baseGDevice;			// GDevice to base off-screen on
	PixMapHandle	basePixMap;				// baseGDeviceÕs PixMap
	Rect				tempOffScreenRect;	// temp rect used for adjustments

	err = noErr;

		// graphics devices manager tells us the deepest intersecting screen
	baseGDevice = GetMaxDevice(offScreenRect);

		// if no screens intersect the bounds, baseDevice is NULL
	if ((baseGDevice != NULL) && (err == noErr))
	{
			// normalize the bounds rectangle
		tempOffScreenRect = *offScreenRect;
		OffsetRect(&tempOffScreenRect, -tempOffScreenRect.left, -tempOffScreenRect.top);

			// create off-screen graphics environment w/ depth, clut of screen
		basePixMap = (**baseGDevice).gdPMap;
		err = SWCreateCGrafPort(newCGrafPort, &tempOffScreenRect, (**basePixMap).pixelSize,
								(**basePixMap).pmTable, baseGDevice);
	}

	return err;
}


///--------------------------------------------------------------------------------------
// SWCreateOffScreen												Adapted from code by Forrest Tanaka
///--------------------------------------------------------------------------------------

#define kMaxRowBytes 0x3FFE // Maximum number of bytes in a row of pixels

SW_FUNC OSErr SWCreateCGrafPort(
	CGrafPtr		*newCGrafPort,		// returns a pointer to the new CGrafPort
	Rect			*bounds,				// bounding rectangle of off-screen
	short			depth,				// desired number of bits per pixel in off-screen
	CTabHandle	colors,				// color table to assign to off-screen
	GDHandle		useGDevice)			// returns a handle to the new GDevice
{
	CGrafPtr			tempCGrafPort;	// pointer to the new off-screen CGrafPort
	PixMapHandle	newPixMap;		// handle to the new off-screen PixMap
	GDHandle			newDevice;		// handle to the new off-screen GDevice
	long				qdVersion;		// version of QuickDraw currently in use
	GrafPtr			savedPort;		// pointer to GrafPort used for save/restore
	SignedByte		savedState;		// saved state of color table handle
	short				bytesPerRow;	// number of bytes per row in the PixMap
	OSErr				err;				// returns err code

#if MPW
#pragma unused(useGDevice)
#endif

		// initialize a few things before we begin
	tempCGrafPort = NULL;
	newPixMap = NULL;
	newDevice = NULL;
	err = noErr;

		// save the color tableÕs current state and make sure it isnÕt purgeable
	if (colors != NULL)
	{
		savedState = HGetState( (Handle)colors );
		HNoPurge( (Handle)colors );
	}

		// calculate the number of bytes per row in the off-screen PixMap
	bytesPerRow = ((depth * (bounds->right - bounds->left) + 31) >> 5) << 2;

		// get the current QuickDraw version
	err = Gestalt(gestaltQuickdrawVersion, &qdVersion);

		// this will always be noErr, but I can't stand not checking anyway!
	if (err == noErr)
	{
			// make sure depth is indexed or depth is direct and 32-Bit QD installed
		if (depth == 1 || depth == 2 || depth == 4 || depth == 8 ||
			((depth == 16 || depth == 32) && qdVersion >= gestalt32BitQD))
		{
				// maximum number of bytes per row is 16,382; make sure within range
			if (bytesPerRow <= kMaxRowBytes)
			{
					// make sure a color table is provided if the depth is indexed
				if (depth <= 8)
					if (colors == NULL)
							// indexed depth and clut is NIL; is parameter err
						err = paramErr;
				}
			else
					// # of bytes per row is more than 16,382; is parameter err
				err = paramErr;
		}
		else
				// pixel depth isnÕt valid; is parameter err
			err = paramErr;
	}

		// if sanity checks succeed, then allocate a new CGrafPort
	if (err == noErr)
	{
		tempCGrafPort = (CGrafPtr)NewPtr(sizeof (CGrafPort) );
		if (tempCGrafPort != NULL)
		{
				// save the current port
			GetPort( &savedPort );

				// initialize the new CGrafPort and make it the current port
			OpenCPort( tempCGrafPort );

				// set portRect, visRgn, and clipRgn to the given bounds rect
			tempCGrafPort->portRect = *bounds;
			RectRgn( tempCGrafPort->visRgn, bounds );
			ClipRect( bounds );

				// initialize the new PixMap for off-screen drawing
			err = SWSetUpPixMap(tempCGrafPort->portPixMap, depth, bounds, colors, bytesPerRow);

			if (err == noErr)
			{
				EraseRect(bounds);

					// grab the initialized PixMap handle
				newPixMap = tempCGrafPort->portPixMap;
			}

				// restore the saved port
			SetPort(savedPort);
		}
		else
			err = MemError();
	}

		// restore the given state of the color table
	if (colors != NULL)
		HSetState((Handle)colors, savedState);

		// one last look around the house before we go...
	if (err != noErr)
	{
			// some err occurred; dispose of everything we allocated
		if (newPixMap != NULL)
		{
			DisposCTable((**newPixMap).pmTable);
			DisposPtr((**newPixMap).baseAddr);
		}

		if (tempCGrafPort != NULL)
		{
			CloseCPort(tempCGrafPort);
			DisposPtr((Ptr)tempCGrafPort);
		}
	}
	else
	{
			// everythingÕs OK; return refs to off-screen CGrafPort
		*newCGrafPort = tempCGrafPort;
	}

	return err;
}


///--------------------------------------------------------------------------------------
// SWCreateCGrafPortFromCIconMask
///--------------------------------------------------------------------------------------

SW_FUNC OSErr SWCreateCGrafPortFromCIconMask(
	CGrafPtr *newCGrafPort,
	CIconHandle cIconH)
{
	OSErr err;
	GrafPtr savePort;
	char saveState;
	CGrafPtr tempCGrafPort;
	BitMap maskBitMap;
	PixMapHandle maskPixMapH;

	*newCGrafPort = NULL;

	GetPort(&savePort);

	saveState = HGetState((Handle)cIconH);
	HLock((Handle)cIconH);

	maskBitMap.rowBytes = (**cIconH).iconMask.rowBytes;
	maskBitMap.bounds = (**cIconH).iconMask.bounds;
	maskBitMap.baseAddr = (Ptr)(**cIconH).iconMaskData;

	err = SWCreateBestCGrafPort(&tempCGrafPort, &maskBitMap.bounds);

	if (err == noErr)
	{
		maskPixMapH = tempCGrafPort->portPixMap;
		HLock((Handle)maskPixMapH);

		SetPort((GrafPtr)tempCGrafPort);

		CopyBits(&maskBitMap, (BitMapPtr)*maskPixMapH,
					&maskBitMap.bounds, &maskBitMap.bounds,
					srcCopy, NULL);

		HUnlock((Handle)maskPixMapH);

		*newCGrafPort = tempCGrafPort;
	}

	HSetState((Handle)cIconH, saveState);
	SetPort(savePort);

	return err;
}


///--------------------------------------------------------------------------------------
// SWCreateCGrafPortFromPict
///--------------------------------------------------------------------------------------

SW_FUNC OSErr SWCreateCGrafPortFromPict(
	CGrafPtr *newCGrafPort,
	PicHandle srcPictH)
{
	OSErr err;
	GrafPtr savePort;
	GWorldPtr tempCGrafPort;
	Rect pictRect;

	*newCGrafPort = NULL;

	GetPort(&savePort);

	pictRect.left = 0;
	pictRect.top = 0;
	pictRect.right = (**srcPictH).picFrame.right - (**srcPictH).picFrame.left;
	pictRect.bottom = (**srcPictH).picFrame.bottom - (**srcPictH).picFrame.top;

	err = SWCreateBestCGrafPort(&tempCGrafPort, &pictRect);

	if (err == noErr)
	{
		SetPort((GrafPtr)tempCGrafPort);

		DrawPicture(srcPictH, &pictRect);

		*newCGrafPort = tempCGrafPort;
	}

	SetPort(savePort);

	return err;
}


///--------------------------------------------------------------------------------------
// SWCreateCGrafPortFromPictResource
///--------------------------------------------------------------------------------------

SW_FUNC OSErr SWCreateCGrafPortFromPictResource(
	CGrafPtr *newCGrafPort,
	short pictResID)
{
	OSErr err = noErr;
	PicHandle newPictH;

	*newCGrafPort = NULL;

	newPictH = GetPicture(pictResID);

	if (newPictH != NULL)
	{
		err = SWCreateCGrafPortFromPict(newCGrafPort, newPictH);

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

	return err;
}


///--------------------------------------------------------------------------------------
// SWSetUpPixMap													Adapted from code by Forrest Tanaka
///--------------------------------------------------------------------------------------

#define kDefaultRes 0x00480000 // Default resolution is 72 DPI; Fixed type

SW_FUNC OSErr SWSetUpPixMap(
	PixMapHandle	aPixMap,			// Handle to the PixMap being initialized
	short				depth,			// Desired number of bits/pixel in off-screen
	Rect				*bounds,			// Bounding rectangle of off-screen
	CTabHandle		colors,			// Color table to assign to off-screen
	short				bytesPerRow)	// Number of bytes per row in the PixMap
{
	CTabHandle	newColors;		// color table used for the off-screen PixMap
	Ptr			offBaseAddr;	// pointer to the off-screen pixel image
	OSErr			err;				// returns err code

	err = noErr;
	newColors = NULL;
	offBaseAddr = NULL;

		// clone the clut if indexed color; allocate a dummy clut if direct color
	if (depth <= 8)
	{
		newColors = colors;
		err = HandToHand( (Handle *)&newColors );
	}
	else
	{
		newColors = (CTabHandle)NewHandle(sizeof(ColorTable) - sizeof(CSpecArray));
		err = MemError();
	}

	if (err == noErr)
	{
			// allocate pixel image; long integer multiplication avoids overflow
		offBaseAddr = NewPtr((Size)bytesPerRow *
								(bounds->bottom - bounds->top) );
		if (offBaseAddr != NULL)
		{
				// initialize fields common to indexed and direct PixMaps
			(**aPixMap).baseAddr = offBaseAddr;  // Point to image
			(**aPixMap).rowBytes = bytesPerRow | // MSB set for PixMap
					0x8000;
			(**aPixMap).bounds = *bounds;		// Use given bounds
			(**aPixMap).pmVersion = 0;		   // No special stuff
			(**aPixMap).packType = 0;			// Default PICT pack
			(**aPixMap).packSize = 0;			// Always zero in mem
			(**aPixMap).hRes = kDefaultRes;	  // 72 DPI default res
			(**aPixMap).vRes = kDefaultRes;	  // 72 DPI default res
			(**aPixMap).pixelSize = depth;	   // Set # bits/pixel
			(**aPixMap).planeBytes = 0;		  // Not used
			(**aPixMap).pmReserved = 0;		  // Not used

				// initialize fields specific to indexed and direct PixMaps
			if (depth <= 8)
			{
					// PixMap is indexed
				(**aPixMap).pixelType = 0;			// Indicates indexed
				(**aPixMap).cmpCount = 1;			// Have 1 component
				(**aPixMap).cmpSize = depth;		// Component size=depth
				(**aPixMap).pmTable = newColors;	// Handle to CLUT
			}
			else
			{
					// PixMap is direct
				(**aPixMap).pixelType = RGBDirect;	// Indicates direct
				(**aPixMap).cmpCount = 3;				// Have 3 components
				if (depth == 16)
					(**aPixMap).cmpSize = 5;			// 5 bits/component
				else
					(**aPixMap).cmpSize = 8;			// 8 bits/component
				(**newColors).ctSeed = 3 * (**aPixMap).cmpSize;
				(**newColors).ctFlags = 0;
				(**newColors).ctSize = 0;
				(**aPixMap).pmTable = newColors;
			}
		}
		else
			err = MemError();
	}
	else
		newColors = NULL;

		// if no errors occurred, return a handle to the new off-screen PixMap
	if (err != noErr)
	{
		if (newColors != NULL)
			DisposCTable( newColors );
	}

		// return the err code
	return err;
}



///--------------------------------------------------------------------------------------
// SWCreateGDevice												Adapted from code by Forrest Tanaka
///--------------------------------------------------------------------------------------

#define kITabRes 4 // Inverse-table resolution

SW_FUNC OSErr SWCreateGDevice(
	GDHandle			*retGDevice,	// returns a handle to the new GDevice
	PixMapHandle	basePixMap)		// handle to the PixMap to base GDevice on
{
	GDHandle			newDevice;		// handle to the new GDevice
	ITabHandle		embryoITab;		// handle to the embryonic inverse table
	Rect				deviceRect;		// rectangle of GDevice
	OSErr				err;				// error code

		// initialize a few things before we begin
	err = noErr;
	newDevice = NULL;
	embryoITab = NULL;

		// allocate memory for the new GDevice
	newDevice = (GDHandle)NewHandle( sizeof (GDevice) );
	if (newDevice != NULL)
	{
			// allocate the embryonic inverse table
		embryoITab = (ITabHandle)NewHandleClear( 2 );
		if (embryoITab != NULL)
		{
				// set rectangle of device to PixMap bounds
			deviceRect = (**basePixMap).bounds;

			// Initialize the new GDevice fields
			(**newDevice).gdRefNum = 0;				// Only used for screens
			(**newDevice).gdID = 0;						// WonÕt normally use
			if ((**basePixMap).pixelSize <= 8)
				(**newDevice).gdType = clutType;		// Depth²8; clut device
			else
				(**newDevice).gdType = directType;	// Depth>8; direct device
			(**newDevice).gdITable = embryoITab;	// 2-byte handle for now
			(**newDevice).gdResPref = kITabRes;		// Normal inv table res
			(**newDevice).gdSearchProc = NULL;		// No color-search proc
			(**newDevice).gdCompProc = NULL;			// No complement proc
			(**newDevice).gdFlags = 0;					// Will set these later
			(**newDevice).gdPMap = basePixMap;		// Reference our PixMap
			(**newDevice).gdRefCon = 0;				// WonÕt normally use
			(**newDevice).gdNextGD = NULL;			// Not in GDevice list
			(**newDevice).gdRect = deviceRect;		// Use PixMap dimensions
			(**newDevice).gdMode = -1;					// For nonscreens
			(**newDevice).gdCCBytes = 0;				// Only used for screens
			(**newDevice).gdCCDepth = 0;				// Only used for screens
			(**newDevice).gdCCXData = 0;				// Only used for screens
			(**newDevice).gdCCXMask = 0;				// Only used for screens
			(**newDevice).gdReserved = 0;				// Currently unused

				// set color-device bit if PixMap isnÕt black & white
			if ((**basePixMap).pixelSize > 1)
				SetDeviceAttribute( newDevice, gdDevType, true );

				// set bit to indicate that the GDevice has no video driver
			SetDeviceAttribute( newDevice, noDriver, true );

				// initialize the inverse table
			if ((**basePixMap).pixelSize <= 8)
			{
				MakeITable( (**basePixMap).pmTable, (**newDevice).gdITable, (**newDevice).gdResPref );
				err = QDError();
			}
		}
		else
			err = MemError();
	}
	else
		err = MemError();

		// handle any errors along the way
	if (err != noErr)
	{
		if (embryoITab != NULL)
			DisposHandle( (Handle)embryoITab );
		if (newDevice != NULL)
			DisposHandle( (Handle)newDevice );
	}
	else
		*retGDevice = newDevice;

		// return a handle to the new GDevice
	return err;
}


///--------------------------------------------------------------------------------------
// SWDisposeCGrafPort											Adapted from code by Forrest Tanaka
///--------------------------------------------------------------------------------------

SW_FUNC void SWDisposeCGrafPort(
	CGrafPtr doomedPort)		// pointer to the CGrafPort to be disposed of
{
	CGrafPtr currPort;		// pointer to the current port

		// check to see whether the doomed CGrafPort is the current port
	GetPort((GrafPtr *)&currPort);

	if (currPort == doomedPort)
	{
			// it is; set current port to Window Manager CGrafPort
		GetCWMgrPort(&currPort);
		SetPort((GrafPtr)currPort);
	}

	DisposPtr((**doomedPort->portPixMap).baseAddr);

	if ((**doomedPort->portPixMap).pmTable != NULL)
	{
		DisposCTable((**doomedPort->portPixMap).pmTable);
	}

	CloseCPort(doomedPort);
	DisposPtr((Ptr)doomedPort);
}


///--------------------------------------------------------------------------------------
// SWCreateGrafPort											Adapted from code by Forrest Tanaka.
///--------------------------------------------------------------------------------------

SW_FUNC OSErr SWCreateGrafPort(
	GrafPtr* newPort,
	Rect* newPortRect)
{
	OSErr err = noErr;
	GrafPtr savePort;		// save port for later restore
	GrafPtr localPort;	// local copy of GrafPort
	Rect localPortRect;	// local copy of bounds

	*newPort = NULL;

		// save off the current port
	GetPort(&savePort);

		// set the top-left corner of bounds to (0,0)
	localPortRect = *newPortRect;
	OffsetRect(&localPortRect, -newPortRect->left, -newPortRect->top);

		// allocate a new GrafPort
	localPort = (GrafPtr)NewPtrClear(sizeof(GrafPort));

	if (localPort != NULL)
	{
			// initialize the new port and make the current port
		OpenPort(localPort);
		SetPort(localPort);
		ForeColor(blackColor);
		BackColor(whiteColor);

			// initialize and allocate the bitmap
		localPort->portBits.bounds = localPortRect;
  		localPort->portBits.rowBytes = ((localPortRect.right + 15) >> 4) << 1;
		localPort->portBits.baseAddr = NewPtrClear(localPort->portBits.rowBytes *
																(long)localPortRect.bottom);

		if (localPort->portBits.baseAddr != NULL)
		{
				// clean up the new port
			localPort->portRect = localPortRect;
			ClipRect(&localPortRect);
			RectRgn(localPort->visRgn, &localPortRect);
			EraseRect(&localPortRect);

			*newPort = localPort;
		}
		else // allocation failed
		{
				// capture the error
			err = MemError();

				// deallocate the port
			ClosePort(localPort);
			DisposPtr((Ptr)localPort);
		}
	}
	else
	{
		err = MemError();
	}

	SetPort(savePort);

	return err;
}


///--------------------------------------------------------------------------------------
// SWCreateGrafPortFromCIconMask
///--------------------------------------------------------------------------------------

SW_FUNC OSErr SWCreateGrafPortFromCIconMask(
	GrafPtr *newGrafPort,
	CIconHandle cIconH)
{
	OSErr err;
	GrafPtr savePort;
	char saveState;
	GrafPtr tempGrafPort;
	BitMap maskBitMap;

	*newGrafPort = NULL;

	GetPort(&savePort);

	saveState = HGetState((Handle)cIconH);
	HLock((Handle)cIconH);

	maskBitMap.rowBytes = (**cIconH).iconMask.rowBytes;
	maskBitMap.bounds = (**cIconH).iconMask.bounds;
	maskBitMap.baseAddr = (Ptr)(**cIconH).iconMaskData;

	err = SWCreateGrafPort(&tempGrafPort, &maskBitMap.bounds);

	if (err == noErr)
	{
		SetPort(tempGrafPort);

		CopyBits(&maskBitMap, &tempGrafPort->portBits,
					&maskBitMap.bounds, &maskBitMap.bounds,
					srcCopy, NULL);

		*newGrafPort = tempGrafPort;
	}

	HSetState((Handle)cIconH, saveState);
	SetPort(savePort);

	return err;
}


///--------------------------------------------------------------------------------------
// SWCreateGrafPortFromPict
///--------------------------------------------------------------------------------------

SW_FUNC OSErr SWCreateGrafPortFromPict(
	GrafPtr *offScrnPort,
	PicHandle srcPictH)
{
	OSErr			err;
	GrafPtr		savePort;
	Rect			tmpRect;

	GetPort(&savePort);

	tmpRect.left = tmpRect.top = 0;
	tmpRect.right = (**srcPictH).picFrame.right - (**srcPictH).picFrame.left;
	tmpRect.bottom = (**srcPictH).picFrame.bottom - (**srcPictH).picFrame.top;

		//	create a port
	err = SWCreateGrafPort(offScrnPort, &tmpRect);

	if (err == noErr)
	{
		SetPort(*offScrnPort);

			//	draw the picture
		DrawPicture(srcPictH, &tmpRect);
	}

	SetPort(savePort);

	return err;
}

///--------------------------------------------------------------------------------------
// SWCreateGrafPortFromPictResource
//
//	this routine will set up an offscreen port, and draw a pict into it
//	the offscreen port is the exact size of the pict
///--------------------------------------------------------------------------------------

SW_FUNC OSErr SWCreateGrafPortFromPictResource(
	GrafPtr *offScrnPort,
	short pictResID)
{
	OSErr			err;
	GrafPtr		savePort;
	PicHandle	newPictH;

	GetPort(&savePort);

	newPictH = GetPicture(pictResID);

	if (newPictH != NULL)
	{
		err = SWCreateGrafPortFromPict(offScrnPort, newPictH);

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
// SWDisposeGrafPort											Adapted from code by Forrest Tanaka.
///--------------------------------------------------------------------------------------

SW_FUNC void SWDisposeGrafPort(
	GrafPtr doomedPort)
{
	ClosePort(doomedPort);
	DisposPtr(doomedPort->portBits.baseAddr);
	DisposPtr((Ptr)doomedPort);
}


///--------------------------------------------------------------------------------------
// SWCreateRegionFromCIconMask
///--------------------------------------------------------------------------------------

SW_FUNC OSErr SWCreateRegionFromCIconMask(
	RgnHandle *maskRgn,
	CIconHandle cIconH)
{
	OSErr err = noErr;
	RgnHandle tempMaskRgn;
	char saveState;
	BitMap iconMask;

	*maskRgn = NULL;

	saveState = HGetState((Handle)cIconH);
	HLock((Handle)cIconH);

	iconMask.rowBytes = (**cIconH).iconMask.rowBytes;
	iconMask.bounds = (**cIconH).iconMask.bounds;
	iconMask.baseAddr = (Ptr)(**cIconH).iconMaskData;

	tempMaskRgn = NewRgn();

	if (tempMaskRgn != NULL)
	{
		if (SWHasGWorlds())
		{
			err = BitMapToRegion(tempMaskRgn, &iconMask);
		}

		if (err == noErr)
		{
			*maskRgn = tempMaskRgn;
		}
		else
		{
			DisposeRgn(tempMaskRgn);
		}
	}
	else
	{
		err = MemError();
	}

	HSetState((Handle)cIconH, saveState);

	return err;
}


///--------------------------------------------------------------------------------------
// SWCreateRegionFromPict
///--------------------------------------------------------------------------------------

SW_FUNC OSErr SWCreateRegionFromPict(
	RgnHandle *pictRgnH,
	PicHandle srcPictH)
{
	OSErr err;
	RgnHandle tempRgnH;
	GrafPtr savePort, offScrnPort;

	*pictRgnH = NULL;

	GetPort(&savePort);

	err = SWCreateGrafPortFromPict(&offScrnPort, srcPictH);

	if (err == noErr)
	{
		SetPort(offScrnPort);

		tempRgnH = NewRgn();

		if (tempRgnH != NULL)
		{
			if (SWHasGWorlds())
			{
				err = BitMapToRegion(tempRgnH, &offScrnPort->portBits);
			}

			if (err == noErr)
			{
				*pictRgnH = tempRgnH;
			}
			else
			{
				DisposeRgn(tempRgnH);
			}
		}
		else
		{
			err = MemError();
		}

		SetPort(savePort);
		SWDisposeGrafPort(offScrnPort);
	}

	return err;
}


///--------------------------------------------------------------------------------------
// SWCreateRegionFromPictResource
///--------------------------------------------------------------------------------------

SW_FUNC OSErr SWCreateRegionFromPictResource(
	RgnHandle *pictRgnH,
	short pictResID)
{
	OSErr			err;
	PicHandle	newPictH;

	newPictH = GetPicture(pictResID);

	if (newPictH != NULL)
	{
		err = SWCreateRegionFromPict(pictRgnH, newPictH);

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

	return err;
}


///--------------------------------------------------------------------------------------
// SWGetCIcon
///--------------------------------------------------------------------------------------

SW_FUNC OSErr SWGetCIcon(
	CIconHandle* cIconH,
	short iconResID)
{
	OSErr err = noErr;
	CIconHandle tempCIconH;

	*cIconH = NULL;

	if (SWHasColorQuickDraw())
	{
		tempCIconH = GetCIcon(iconResID);

		if (tempCIconH == NULL)
		{
				// hmmm, could be a resource error...
			err = ResError();

			if (err == noErr)
			{
					// or, a memory error...
				err = MemError();
			}

			if (err == noErr)
			{
					// ...well then lets make up an error!
				err = resNotFound;
			}
		}
	}
	else
	{
			// don't ever do this if color QuickDraw is around!
		tempCIconH = (CIconHandle)GetResource(kColorIconResType, iconResID);

		if (tempCIconH != NULL)
		{
			DetachResource((Handle)tempCIconH);
		}
		else
		{
			err = ResError();

			if (err == noErr)
			{
				err = resNotFound;
			}
		}
	}

	if (err == noErr)
	{
		*cIconH = tempCIconH;
	}

	return err;
}


///--------------------------------------------------------------------------------------
// SWPlotCIcon
///--------------------------------------------------------------------------------------

SW_FUNC OSErr SWPlotCIcon(
	CIconHandle cIconH,
	Rect* iconRect)
{
	OSErr err = noErr;
	char saveState;
	BitMap iconBitMap, maskBitMap;
	GrafPtr curPort;

	if (SWHasColorQuickDraw())
	{
		PlotCIcon(iconRect, cIconH);
	}
	else
	{
		saveState = HGetState((Handle)cIconH);
		HLock((Handle)cIconH);

		maskBitMap.rowBytes = (**cIconH).iconMask.rowBytes;
		maskBitMap.bounds = (**cIconH).iconMask.bounds;
		maskBitMap.baseAddr = (Ptr)(**cIconH).iconMaskData;

		iconBitMap.rowBytes = (**cIconH).iconBMap.rowBytes;
		iconBitMap.bounds = (**cIconH).iconBMap.bounds;
		iconBitMap.baseAddr = maskBitMap.baseAddr +
				(maskBitMap.rowBytes * (maskBitMap.bounds.bottom - maskBitMap.bounds.top));

		GetPort(&curPort);

		CopyMask(&iconBitMap, &maskBitMap, &curPort->portBits,
				&iconBitMap.bounds, &maskBitMap.bounds, iconRect);

		HSetState((Handle)cIconH, saveState);
	}
	
	return err;
}


///--------------------------------------------------------------------------------------
// SWPlotCIcon
///--------------------------------------------------------------------------------------

SW_FUNC void SWDisposeCIcon(
	CIconHandle cIconH)
{
	if (SWHasColorQuickDraw())
	{
		DisposeCIcon(cIconH);
	}
	else
	{
		DisposeHandle((Handle)cIconH);
	}
}


///--------------------------------------------------------------------------------------
// SWCreateOptimumGWorld
//
//	create a new GWorld optimized for speed in copying
//	to the graphics device that intersects the given rect.
///--------------------------------------------------------------------------------------

SW_FUNC OSErr SWCreateOptimumGWorld(
	GWorldPtr *optGWorld,
	Rect *devRect)
{
	OSErr err;
	CGrafPtr saveCPort;
	GDHandle saveGDevice;
	GWorldPtr tempGWorld;
	PixMapHandle pixMapH;
	Rect tempRect = *devRect;

	*optGWorld = NULL;

	GetGWorld(&saveCPort, &saveGDevice);

	LocalToGlobal((Point *)&tempRect.top);
	LocalToGlobal((Point *)&tempRect.bottom);

	err = NewGWorld(&tempGWorld, 0, &tempRect, NULL, NULL, noNewDevice);

	if (err == noErr)
	{
		SetGWorld(tempGWorld, NULL);

		pixMapH = GetGWorldPixMap(tempGWorld);

		if (LockPixels(pixMapH))
		{
			EraseRect(&tempGWorld->portRect);

			UnlockPixels(pixMapH);
		}

		*optGWorld = tempGWorld;
	}

	SetGWorld(saveCPort, saveGDevice);

	return err;
}


///--------------------------------------------------------------------------------------
// SWCreateGWorldFromPict
//
//	creates a offScreen GWorld and draws the specified pict into it
///--------------------------------------------------------------------------------------

SW_FUNC OSErr SWCreateGWorldFromPict(
	GWorldPtr *pictGWorld,
	PicHandle pictH)
{
	OSErr err;
	CGrafPtr saveCPort;
	GDHandle saveGDevice;
	GWorldPtr tempGWorld;
	PixMapHandle tempPixHdl;
	Rect pictRect;

	*pictGWorld = NULL;

	GetGWorld(&saveCPort, &saveGDevice);

	pictRect.left = 0;
	pictRect.top = 0;
	pictRect.right = (**pictH).picFrame.right - (**pictH).picFrame.left;
	pictRect.bottom = (**pictH).picFrame.bottom - (**pictH).picFrame.top;

	err = SWCreateOptimumGWorld(&tempGWorld, &pictRect);

	if (err == noErr)
	{
		*pictGWorld = tempGWorld;

		SetGWorld(tempGWorld, NULL);

		tempPixHdl = GetGWorldPixMap(tempGWorld);

		if (LockPixels(tempPixHdl))
		{
			DrawPicture(pictH, &pictRect);

			UnlockPixels(tempPixHdl);
		}
	}

	SetGWorld(saveCPort, saveGDevice);

	return err;
}


///--------------------------------------------------------------------------------------
// SWCreateGWorldFromPictResource
///--------------------------------------------------------------------------------------

SW_FUNC OSErr SWCreateGWorldFromPictResource(
	GWorldPtr *pictGWorldP,
	short pictResID)
{
	OSErr err = noErr;
	PicHandle newPictH;

	newPictH = GetPicture(pictResID);

	if (newPictH != NULL)
	{
		err = SWCreateGWorldFromPict(pictGWorldP, newPictH);

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

	return err;
}


///--------------------------------------------------------------------------------------
// SWCreateGWorldFromCIconResource
///--------------------------------------------------------------------------------------

SW_FUNC OSErr SWCreateGWorldFromCIconResource(
	GWorldPtr *iconGWorldP,
	short iconResID)
{
	OSErr err;
	CIconHandle cIconH;

	cIconH = GetCIcon(iconResID);

	if (cIconH != NULL)
	{
		HNoPurge((Handle)cIconH);

		if (iconGWorldP != NULL)
		{
			err = SWCreateGWorldFromCIcon(iconGWorldP, cIconH);
		}

		DisposeCIcon(cIconH);
	}

	return err;
}


///--------------------------------------------------------------------------------------
// SWCreateGWorldFromCIcon
///--------------------------------------------------------------------------------------

SW_FUNC OSErr SWCreateGWorldFromCIcon(
	GWorldPtr *iconGWorldP,
	CIconHandle cIconH)
{
	OSErr err;
	CGrafPtr saveCPort;
	GDHandle saveGDevice;
	GWorldPtr tempGWorldP;
	PixMapHandle iconPixMapH;
	Rect iconRect;

	*iconGWorldP = NULL;

	GetGWorld(&saveCPort, &saveGDevice);

	iconRect = (**cIconH).iconPMap.bounds;
	
	err = SWCreateOptimumGWorld(&tempGWorldP, &iconRect);

	if (err == noErr)
	{
		*iconGWorldP = tempGWorldP;

		SetGWorld(tempGWorldP, NULL);

		iconPixMapH = GetGWorldPixMap(tempGWorldP);

		if (LockPixels(iconPixMapH))
		{
			PlotCIcon(&iconRect, cIconH);

			UnlockPixels(iconPixMapH);
		}
	}

	SetGWorld(saveCPort, saveGDevice);

	return err;
}


///--------------------------------------------------------------------------------------
// SWCreateGWorldFromCIconMask
///--------------------------------------------------------------------------------------

SW_FUNC OSErr SWCreateGWorldFromCIconMask(
	GWorldPtr *maskGWorldP,
	CIconHandle cIconH)
{
	OSErr err;
	CGrafPtr saveCPort;
	GDHandle saveGDevice;
	char saveState;
	BitMap maskBitMap;
	GWorldPtr tempGWorldP;
	PixMapHandle maskPixMapH;

	GetGWorld(&saveCPort, &saveGDevice);

	saveState = HGetState((Handle)cIconH);
	HLock((Handle)cIconH);

	maskBitMap.rowBytes = (**cIconH).iconMask.rowBytes;
	maskBitMap.bounds = (**cIconH).iconMask.bounds;
	maskBitMap.baseAddr = (Ptr)(**cIconH).iconMaskData;

	err = SWCreateOptimumGWorld(&tempGWorldP, &maskBitMap.bounds);

	if (err == noErr)
	{
		*maskGWorldP = tempGWorldP;

		SetGWorld(tempGWorldP, NULL);

		maskPixMapH = GetGWorldPixMap(tempGWorldP);

		if (LockPixels(maskPixMapH))
		{
			CopyBits(&maskBitMap, (BitMapPtr)*maskPixMapH,
						&maskBitMap.bounds, &maskBitMap.bounds,
						srcCopy, NULL);

			UnlockPixels(maskPixMapH);
		}
	}

	SetGWorld(saveCPort, saveGDevice);

	HSetState((Handle)cIconH, saveState);

	return err;
}


///--------------------------------------------------------------------------------------
// SWHasColorQuickDraw
///--------------------------------------------------------------------------------------

SW_FUNC Boolean SWHasColorQuickDraw(void)
{
	OSErr err;
	long	gestaltResult;

	err = Gestalt(gestaltQuickdrawVersion, &gestaltResult);

	return (err == noErr) && (gestaltResult >= gestalt8BitQD);
}


///--------------------------------------------------------------------------------------
// SWHasGWorlds
///--------------------------------------------------------------------------------------

SW_FUNC Boolean SWHasGWorlds(void)
{
	OSErr err;
	long	gestaltResult;

	err = Gestalt(gestaltQuickdrawVersion, &gestaltResult);

	return (err == noErr) && (((gestaltResult > gestaltOriginalQD) &&
			(gestaltResult < gestalt8BitQD)) || (gestaltResult >= gestalt32BitQD));
}
