/*
	Public domain by Zig Zichterman.
*/
/*
	LCDEFPushButton
	
	A push button
	
	12/27/94	zz	h	initial write
*/
#pragma once

#include "LCDEFPushOrIconButton.h"

class LCDEFPushButton
	: public LCDEFPushOrIconButton
{
	public :
		LCDEFPushButton(
			ControlHandle	inControl,
			ButtonKindT		inButtonKind,
			Boolean			inIndentWhenHighlighted,
			Boolean			inUseWindowFont)
			: LCDEFPushOrIconButton(inControl,inButtonKind,inIndentWhenHighlighted,
				inUseWindowFont)
			{}
	
	protected :
		enum {	InsideRadius	= 8,
				FrameRadius		= 10	};
				
		virtual short
		GetFrameRadius(void) const
		{	return FrameRadius;	}
		
		virtual short
		GetInsideRadius(void) const
		{	return InsideRadius;	}

		virtual void 
		DrawTitle(
			Is3DFlagT			inIs3D,
			IsHighlightedFlagT	inIsHighlighted,
			Rect &				outTitleRect) const;

		virtual void 
		Draw3DEffects(void) const;
		
		virtual void 
		DrawValue(
			Is3DFlagT			/*inIs3D*/,
			IsEnabledFlagT		/*inEnabled*/,
			IsHighlightedFlagT	/*inHighlight*/) const
			{}
};
