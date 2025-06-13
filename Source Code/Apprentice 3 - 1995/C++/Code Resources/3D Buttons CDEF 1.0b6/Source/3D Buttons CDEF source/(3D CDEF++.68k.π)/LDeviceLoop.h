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
#pragma once

#include <Quickdraw.h>

#include "RectT.h"
#include "StSaveClip.h"

class LDeviceLoop
{
	public :
		LDeviceLoop(
			const Rect &	inLocalRect);
			
		Boolean
		Next(
			Boolean &	outIs3D,				// should caller use 3D appearance?
			Boolean &	outIsGreyishCapable,	// should caller use true grey for dimmed stuff?
			RectT &		outClippedRect);		// bounds of intersected clip region
			
	protected :
		RectT		mGlobalRect;
		StSaveClip	mSaveClip;
		GDHandle	mCurrentDevice;
		Boolean		mLacksColorQD;
		Boolean		mReturnedOneNonColorQDDevice;
		
		void
		ClipToDeviceIntersection(
			RectT &	outClippedRect) const;
};
