/*
	Public domain by Zig Zichterman.
*/
/*
	LCDEFCheckboxOrRadioButton
	
	Base class for checkboxes and radio buttons
	
	12/28/94	zz	h	initial write
*/
#pragma once

#include "LCDEFControl.h"

#include "RectT.h"

class RgnHandleT;

class LCDEFCheckboxOrRadioButton :
	public LCDEFControl
{
	protected :
		enum {	CheckboxOrRadioButtonSizePixels	=	12	};
		
		enum {	Value_Off				= 0,
				Value_On				= 1,
				Value_Tristate			= 2,
				Value_MicrosoftTristate	= 3				};
		
		enum {	Grey_MicrosoftTristateFore	= 0x7777,
				Grey_MicrosoftTristateBack	= 0xDDDD	};
				
	
		RectT	mRadioCheckRect;
		RectT	mTitleRect;
		
		LCDEFCheckboxOrRadioButton(
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

		virtual void
		CalcPartRects(void);
				
		virtual void 
		CalcRadioCheckRgn(
			RgnHandleT &	outRadioCheckRgn) const = 0;
		
		virtual void 
		DrawTitle(
			Is3DFlagT			inIs3D,
			IsHighlightedFlagT	inIsHighlighted,
			Rect &				outTitleRect) const;
};
