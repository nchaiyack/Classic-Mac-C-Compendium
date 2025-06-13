/*
	Public domain by Zig Zichterman.
*/
/*
	RgnHandleT
	
	A C++ encapsulation of a region handle.
	
	12/27/94	zz	h	initial write
*/
#pragma once

#include <QuickDraw.h>

class RgnHandleT
{
	public :
		inline RgnHandleT(void)
			: mRgn(NULL)
		{	AllocateNew();	}
		
		inline RgnHandleT(
			const RgnHandleT &	inOriginal)
			: mRgn(NULL)
		{	Copy(inOriginal.mRgn);	}
		
		inline RgnHandleT(
			const Rect &	inRect)
			: mRgn(NULL)
		{	SetRect(inRect);	}
		
		~RgnHandleT()
		{	Dispose();	}
		
		void
		AllocateNew(void);
		
		Boolean
		AllocateIfNeeded(void);
		
		void
		Dispose(void);
		
		Boolean
		HasRgnHandle(void) const
		{	return (mRgn != NULL);	}
		
		void
		SetRect(
			const Rect &	inRect);
		
		void
		Copy(
			const RgnHandle	inOriginal);
		
		void
		CopyFromClip(void);
		
		void
		CopyToClip(void) const;
		
		void
		UnionWith(
			const RgnHandle	inRgn);
		
		void
		SectWith(
			const RgnHandle	inRgn);
		
		void
		Diff(
			const RgnHandle	inRgn);
		
		inline operator RgnHandle (void)
		{	return mRgn;	}
	
		inline RgnHandleT operator += (
			const RgnHandleT &	inRgn)
			{	UnionWith(inRgn.mRgn); return *this;	}
		
		inline RgnHandle operator += (
			const RgnHandle	inRgn)
			{	UnionWith(inRgn); return mRgn;	}
		
		inline RgnHandleT operator -= (
			const RgnHandleT &	inRgn)
			{	Diff(inRgn.mRgn); return *this;	}
		
		inline RgnHandle operator -= (
			const RgnHandle	inRgn)
			{	Diff(inRgn); return mRgn;	}
		
		void
		GetBoundingBox(
			Rect &	outBoundingBox) const;
			
	protected :
		RgnHandle	mRgn;
};
