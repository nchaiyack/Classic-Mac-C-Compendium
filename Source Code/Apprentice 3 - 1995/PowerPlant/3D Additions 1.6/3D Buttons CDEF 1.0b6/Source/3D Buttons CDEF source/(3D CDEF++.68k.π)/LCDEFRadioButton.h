/*
	Public domain by Zig Zichterman.
	Modified by J. Rodden, DD/MF & Associates
*/
/*
	LCDEFRadioButton
	
	12/28/94	zz	h	initial write
*/
#pragma once

#include "LCDEFCheckboxOrRadioButton.h"

class LCDEFRadioButton :
	public LCDEFCheckboxOrRadioButton
{
	public :
		LCDEFRadioButton(
			ControlHandle	inControl,
			ButtonKindT		inButtonKind,
			Boolean			inIndentWhenHighlighted,
			Boolean			inUseWindowFont)
			: LCDEFCheckboxOrRadioButton(inControl,inButtonKind,inIndentWhenHighlighted,
				inUseWindowFont)
			{}

	protected :
		virtual void 
		CalcRadioCheckRgn(
			RgnHandleT &	outRadioCheckRgn) const;

		virtual void 
		Draw3DEffects(void) const;
		
		virtual void 
		DrawValue(
			Is3DFlagT			inIs3D,
			IsEnabledFlagT		inEnabled,
			IsHighlightedFlagT	inHighlight) const;
		
		virtual void 
		DrawFrame(void) const;
		
		virtual void DrawConvexRadioBtn(void) const;
		virtual void DrawConcaveRadioBtn(void) const;
		virtual void DrawEmbeddedRadioBtn(void) const;
};
