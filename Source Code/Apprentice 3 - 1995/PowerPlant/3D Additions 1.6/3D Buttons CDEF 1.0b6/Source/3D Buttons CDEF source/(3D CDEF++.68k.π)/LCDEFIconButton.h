/*
	Public domain by Zig Zichterman.
*/
/*
	LCDEFIconButton
	
	An icon button
	
	12/28/94	zz	h	initial write
*/
#pragma once

#include "LCDEFPushOrIconButton.h"

class LCDEFIconButton
	: public LCDEFPushOrIconButton
{
	public :
		LCDEFIconButton(
			ControlHandle	inControl,
			ButtonKindT		inButtonKind,
			Boolean			inIndentWhenHighlighted,
			Boolean			inUseWindowFont)
			: LCDEFPushOrIconButton(inControl,inButtonKind,inIndentWhenHighlighted,
				inUseWindowFont)
			{}

		virtual short
		GetFrameRadius(void) const
		{	return 4;	}
		
		virtual short
		GetInsideRadius(void) const
		{	return 0;	}
		
		virtual void 
		DrawTitle(
			Is3DFlagT			/*inIs3D*/,
			IsHighlightedFlagT	/*inIsHighlighted*/,
			Rect &				/*outTitleRect*/) const
			{}
	
		virtual void 
		Draw3DEffects(void) const;
		
		virtual void 
		DrawValue(
			Is3DFlagT			inIs3D,
			IsEnabledFlagT		inEnabled,
			IsHighlightedFlagT	inHighlighted) const;
};
