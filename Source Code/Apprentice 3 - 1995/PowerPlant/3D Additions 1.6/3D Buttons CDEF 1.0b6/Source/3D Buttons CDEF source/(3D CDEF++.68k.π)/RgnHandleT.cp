/*
	Public domain by Zig Zichterman.
*/
/*
	RgnHandleT
	
	A C++ encapsulation of a region handle
	
	12/27/94	zz	h	initial write
*/
#include "RgnHandleT.h"

/**************************************************************************
	AllocateNew()									[public]
	
	Allocate a new region handle. Throw out anything we had before
**************************************************************************/
void
RgnHandleT::AllocateNew(void)
{
	// throw out any previous
	Dispose();
	
	// allocate a new region handle
	mRgn = ::NewRgn();
}

/**************************************************************************
	AllocateIfNeeded()								[public]
	
	Allocate a new region handle, only if we don't currently have one.
	
	Returns true if we have a handle, false if we don't (allocation failed)
**************************************************************************/
Boolean
RgnHandleT::AllocateIfNeeded(void)
{
	// if we lack a region handle
	if (mRgn == NULL) {
		// allocate a new region handle
		mRgn = ::NewRgn();
	}
	
	// return true if we have a region handle now
	return (mRgn != NULL);
}

/**************************************************************************
	Dispose()										[public]
	
	If we have a region handle, dump it
**************************************************************************/
void
RgnHandleT::Dispose(void)
{
	// if we have a region handle
	if (mRgn != NULL) {
		// dump it
		::DisposeRgn(mRgn);
		
		// and never try to use it again, even accidentally
		mRgn = NULL;
	}
}

/**************************************************************************
	SetRect()										[public]
	
	Change this region into the given rectangle
**************************************************************************/
void
RgnHandleT::SetRect(
	const Rect &	inRect)
{
	// if we have a valid region handle
	if (AllocateIfNeeded()) {
		// change it
		::RectRgn(mRgn,&inRect);
	}
}

/**************************************************************************
	Copy()											[public]
	
	Copy a region handle to our region handle. Copies the region,
	not the handle, so you can dispose the original later without
	affecting our copy.
**************************************************************************/
void
RgnHandleT::Copy(
	const RgnHandle	inOriginal)
{
	// if we have a valid region handle
	if (AllocateIfNeeded()) {
		// make a copy of the region
		::CopyRgn(inOriginal,mRgn);
	}
}

/**************************************************************************
	CopyFromClip()									[public]
	
	Copy the clip region to our region
**************************************************************************/
void
RgnHandleT::CopyFromClip(void)
{
	// if we have a valid region handle
	if (AllocateIfNeeded()) {
		// copy the clip region into it
		::GetClip(mRgn);
	}
}

/**************************************************************************
	CopyToClip()									[public]
	
	Copy this region to the clip region
**************************************************************************/
void
RgnHandleT::CopyToClip(void) const
{
	// if we have a valid region handle
	if (mRgn != NULL) {
		// copy it into the clip region
		::SetClip(mRgn);
	}
}

/**************************************************************************
	UnionWith()									[public]
	
	add another region's area to our region
**************************************************************************/
void
RgnHandleT::UnionWith(
	const RgnHandle	inRgn)
{
	// if we have a valid region handle
	if (AllocateIfNeeded()) {
		// add the other region's turf to ours
		::UnionRgn(mRgn,inRgn,mRgn);
	}
}

/**************************************************************************
	SectWith()									[public]
	
	change our region to the intersection of our previous region
	and the given region
**************************************************************************/
void
RgnHandleT::SectWith(
	const RgnHandle	inRgn)
{
	// if we have a valid region handle
	if (mRgn != NULL) {
		// intersect the other region's turf with ours
		::SectRgn(mRgn,inRgn,mRgn);
	}
}

/**************************************************************************
	Diff()										[public]
	
	Subtract another region's area from our own
**************************************************************************/
void
RgnHandleT::Diff(
	const RgnHandle	inRgn)
{
	// if we have a valid region handle
	if (mRgn != NULL) {
		// add the other region's turf to ours
		::DiffRgn(mRgn,inRgn,mRgn);
	}
}

/**************************************************************************
	GetBoundingBox()							[public]
	
	Returns the bounding box for this region
**************************************************************************/
void
RgnHandleT::GetBoundingBox(
	Rect &	outBoundingBox) const
{
	if (mRgn != NULL) {
		outBoundingBox = (**mRgn).rgnBBox;
	}
}

