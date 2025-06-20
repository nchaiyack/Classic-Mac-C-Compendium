/*
	Public domain by Zig Zichterman.
*/
/*
	LCDEFControl
	
	Base class for 3D Buttons controls. Most of the
	code lives in LCDEFControl, with selected stuff 
	overridden in derived classes.
	
	12/29/94	zz	h	add mButtonKind
	12/20/94	zz	h	initial write
*/
#pragma once

#include <Controls.h>

class LCDEFControl
{
	public :
		typedef enum {
			ButtonKind_PushButton	= 0,
			ButtonKind_Checkbox,
			ButtonKind_RadioButton,
			ButtonKind_IconButton,
			ButtonKind_Unknown
		} ButtonKindT;
			
		enum {	IndentWhenHighlightedVariantFlag	= 0x04	};
		
		
		static long
		Main(
			short			inVariation,
			ControlHandle	ioControl,
			short			inMsg,
			long			ioParam);
	
	protected :
		enum {
			Grey_Black				= 0x0000,
			Grey_White				= 0xFFFF,
		
			Grey_2BitInactiveFrame	= 0xAAAA,
			
			Grey_Background			= 0xEEEE
			
			
		};
		
		// flag typedef/enums
		typedef enum {
			Flag_IsNot3D			= false,
			Flag_Is3D				= true
		} Is3DFlagT;
		
		typedef enum {
			Flag_IsNotGreyCapable	= false,
			Flag_IsGreyCapable		= true
		} IsGreyCapableFlagT;
	
		typedef enum {
			Flag_IsNotEnabled		= false,
			Flag_IsEnabled			= true
		} IsEnabledFlagT;
		
		typedef enum {
			Flag_IsNotHighlighted	= false,
			Flag_IsHighlighted		= true
		} IsHighlightedFlagT;
			
		ControlHandle	mControl;
		ButtonKindT		mButtonKind;
		Boolean			mIndentWhenHighlighted;
		Boolean			mUseWindowFont;
		RGBColor		mBackgroundColor;
		RGBColor		mInactiveFrameColor;
		
		LCDEFControl(
			ControlHandle	inControl,
			ButtonKindT		inButtonKind,
			Boolean			inIndentWhenHighlighted,
			Boolean			inUseWindowFont);
		
		virtual ~LCDEFControl();
		
		long
		Dispatch(
			short	inMsg,
			long	ioParam);
			
		static void
		UnpackVariant(
			short			inVariation,
			ButtonKindT &	outButtonKind,
			Boolean &		outIndentWhenHighlighted,
			Boolean &		outUseWindowFont);
		
		void
		Draw(void);
		
		Boolean
		HitTest(
			Point	inHitPoint) const;
			
		virtual void
		GetControlRgn(
			RgnHandle	outControlRgn) const;
		
		// control handle accessors
		void
		GetControlRect(
			Rect &	outControlRect) const;
		
		short
		GetControlValue(void) const;
	
		StringPtr
		GetControlTitle(void) const;
		
		Boolean
		IsVisible(void) const;
		
		IsEnabledFlagT
		IsEnabled(void) const;
		
		IsHighlightedFlagT
		IsHighlighted(void) const;
		
		// drawing routines
		void
		CalcColors(void);
		
		virtual void
		CalcColorsSelf(void) {}
		
		void
		CalcBackgroundColor(void);
		
		void
		CalcInactiveFrameColor(void);
		
		virtual void
		CalcPartRects(void) {}
		
		void
		ClipToControlRgn(void) const;
		
		void
		DrawLoop(
			Is3DFlagT			inIs3D,					// can we draw 3D effects?
			IsGreyCapableFlagT	inIsGreyCapable) const;	// can we use true grey for dimmed stuff?

		virtual void
		SetBGColorToErase(
			Is3DFlagT			inIs3D,
			IsEnabledFlagT		inIsEnabled,
			IsHighlightedFlagT	inIsHighlighted) const = 0;
		
		virtual void 
		Erase(void) const = 0;
		
		void 
		SetFGColorToFrame(
			Is3DFlagT			inIs3D,
			IsGreyCapableFlagT	inIsGreyCapable,
			IsEnabledFlagT		inIsEnabled) const;
		
		virtual void 
		DrawTitle(
			Is3DFlagT			inIs3D,
			IsHighlightedFlagT	inIsHighlighted,
			Rect &				outTitleRect) const = 0;
		
		void
		DimTitleIfNecessary(
			IsGreyCapableFlagT	inIsGreyCapable,
			IsEnabledFlagT		inIsEnabled,
			const Rect &		inTitleRect) const;
			
		virtual void 
		DrawFrame(void) const = 0;
		
		virtual void 
		Draw3DEffects(void) const = 0;
		
		virtual void 
		DrawValue(
			Is3DFlagT			inIs3D,
			IsEnabledFlagT		inEnabled,
			IsHighlightedFlagT	inHighlight) const = 0;
		
		virtual void
		InvertHighlight(void) const
		{}
		
		// drawing utilities
		static void
		ForeGrey(
			unsigned short	inGrey);
		
		static void
		BackGrey(
			unsigned short	inGrey);
		
		static void
		Blend(
			unsigned short	inGreyLevel,	// 0x0000 to 0xFFFF
			const RGBColor	&inColor,		// color to mix with inGreyLevel
			float			inPercentGrey,	// 1.0 = pure grey, 0.0 = pure color
			RGBColor		&outBlend);
	
		static void
		StringBox(
			const StringPtr	inString,
			const Rect &	inBox,
			short			inAlign);
		
		static short
		CountLines(
			const StringPtr	inString);
		
		static short
		StringHeight(
			const StringPtr	inString);
		
		static void
		DimRect(
			const Rect &	inRect);

};
