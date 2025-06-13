/*
	Public domain by Zig Zichterman.
*/
/*
	LCDEFPushOrIconButton
	
	Base class for push button and icon button
	
	12/27/94	zz	h	initial write
*/
#pragma once

#include "LCDEFControl.h"

class LCDEFPushOrIconButton
	: public LCDEFControl
{
	protected :
		enum {
			Grey_ButtonContent				= 0xDDDD,
			Grey_HighlightedButtonContent	= 0x8888
		};
		
		LCDEFPushOrIconButton(
			ControlHandle	inControl,
			ButtonKindT		inButtonKind,
			Boolean			inIndentWhenHighlighted,
			Boolean			inUseWindowFont)
			: LCDEFControl(inControl,inButtonKind,inIndentWhenHighlighted,
				inUseWindowFont)
			{}

		virtual void
		SetBGColorToErase(
			Is3DFlagT			inIs3D,
			IsEnabledFlagT		inIsEnabled,
			IsHighlightedFlagT	inIsHighlighted) const;

		virtual void 
		Erase(void) const;
		
		virtual short
		GetFrameRadius(void) const = 0;
		
		virtual short
		GetInsideRadius(void) const = 0;
		
		virtual void 
		DrawFrame(void) const;
		
		virtual void 
		InvertHighlight(void) const;

		virtual void
		GetControlRgn(
			RgnHandle	outControlRgn) const;
};
