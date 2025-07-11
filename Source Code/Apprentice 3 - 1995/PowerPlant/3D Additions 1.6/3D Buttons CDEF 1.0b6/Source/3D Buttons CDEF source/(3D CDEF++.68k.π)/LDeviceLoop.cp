/*
	Public domain by Zig Zichterman.
*/
/*
	LDeviceLoop
	
	A class that iterates through all the graphic devices (aka screens)
	that intersect with a given local rectangle.  Each time through the
	loop, you get back the depth of one device that you're clipped to,
	so you can suit your drawing to that device
	
	If the system lacks color QuickDraw, LDeviceLoop will return one
	device (not 3D, not greyish capable).
	
	12/27/94	zz	h	initial write
*/
#include "LDeviceLoop.h"

#include "IsColorQDPresent.h"

/**************************************************************************
	LDeviceLoop()									[public]
	
	Create a new device iterator for the given local rect. 
**************************************************************************/
LDeviceLoop::LDeviceLoop(
	const Rect &	inLocalRect)
{
	mCurrentDevice	= NULL;
	mLacksColorQD	= (::IsColorQDPresent() == false);
	mReturnedOneNonColorQDDevice	= false;
	
	// convert the given local rect to global coords
	mGlobalRect.Copy(inLocalRect);
	mGlobalRect.LocalToGlobal();
}
	
/**************************************************************************
	Next()											[public]
	
	Is there another device that intersects with the original rect?
	Return true if so, false if not.
	
	Also tell you whether that device is something worth drawing
	3D on (4bits + greyscale, or 8+ bits), and whether it's worth
	drawing the dimmed stuff (text, frame) in a true greyscale
	(2bits + greyscale, or 4+ bits).
**************************************************************************/
Boolean
LDeviceLoop::Next(
	Boolean &	outIs3D,				// should caller use 3D appearance?
	Boolean &	outIsGreyishCapable,	// should caller use true grey for dimmed stuff?
	RectT &		outClippedRect)			// bounds of intersected clip region
{
	// if we lack color quickdraw
	if (mLacksColorQD) {
		// if we have already returned a device
		if (mReturnedOneNonColorQDDevice) {
			// there are no more devices
			return false;
		} else {
			// remember that we're about to return a device
			mReturnedOneNonColorQDDevice	= true;
			
			// return one very shallow device
			outIs3D				= false;
			outIsGreyishCapable	= false;
			return true;
		}
	}
	
	// safe to use color Quickdraw from here on
	
	// if we haven't done so already
	if (mCurrentDevice == NULL) {
		// get the device list
		mCurrentDevice = ::GetDeviceList();
	} else {
		// get the next device
		mCurrentDevice = ::GetNextDevice(mCurrentDevice);
	}
	
	// keep going until we run out of devices or find an intersecting one
	RectT	intersection;
	RectT	deviceRect;
	for (;(mCurrentDevice != NULL) && (*mCurrentDevice != NULL);
			mCurrentDevice = ::GetNextDevice(mCurrentDevice)) {
		
		// does this device intersect with our global rect?
		deviceRect.Copy((**mCurrentDevice).gdRect);
		if (::SectRect(&mGlobalRect,&deviceRect,&intersection) == false) {
			// nope. Keep searching
			continue;
		}
		
		// is the device an active monitor?
		if ((::TestDeviceAttribute(mCurrentDevice,screenDevice) == 0)
			|| (::TestDeviceAttribute(mCurrentDevice,screenActive) == 0)) {
			continue;	// not a screen. Keep looking			
		}
		
		// we've got one. Clip to intersection of original clip
		// and device bounds
		ClipToDeviceIntersection(outClippedRect);
		
		// how deep is the device?
		short			depth			= 0;
		PixMapHandle	pixMapHandle	= (**mCurrentDevice).gdPMap;
		if ((pixMapHandle != NULL) && (*pixMapHandle != NULL)) {
			depth = (**pixMapHandle).pixelSize;
		}

		// is the device showing greyscales instead of colors?
		// !!! I haven't seen this flag documented, so this might
		// !!! be a dangerous test. It's worked so far though...
		Boolean			isGreyscale		= false;
		if (((**mCurrentDevice).gdFlags & 0x0001) == 0) {
			isGreyscale = true;
		}
		
		// is the device 3D capable?
		if (8 <= depth) {
			outIs3D	= true;
		} else if ((4 <= depth) && (isGreyscale)) {
			outIs3D = true;
		} else {
			outIs3D	= false;
		}
		
		// is the device capable of drawing true grey for dimmed stuff?
		if (4 <= depth) {
			outIsGreyishCapable	= true;
		} else if ((2 <= depth) && (isGreyscale)) {
			outIsGreyishCapable = true;
		} else {
			outIsGreyishCapable	= false;
		}
		
		// tell the caller we've got a device
		return true;
	}
	
	// we fell through the for-loop without finding an intersecting device
	return false;	
}

/**************************************************************************
	ClipToDeviceIntersection()						[protected]
	
	Clip to the intersection of the original clip and the 
	current device's bounds
**************************************************************************/
void
LDeviceLoop::ClipToDeviceIntersection(
	RectT &	outLocalClippedRect) const
{
	// get the original clip region
	RgnHandleT	originalClip;
	originalClip.Copy(mSaveClip.mRgn);
	
	// get the device bounds as a rect
	RectT	deviceRect;
	deviceRect.Copy((**mCurrentDevice).gdRect);
	deviceRect.GlobalToLocal();
	
	// convert the rect to a region
	RgnHandleT	deviceBounds;
	deviceBounds.SetRect(deviceRect);
	
	// calculate the intersection of the two regions
	RgnHandleT	intersection(originalClip);
	intersection.SectWith(deviceBounds);
	
	// clip to the intersection
	intersection.CopyToClip();
	
	// get the bounding box for the intersection
	intersection.GetBoundingBox(outLocalClippedRect);
}

