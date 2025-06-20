/*
	Public domain by Zig Zichterman.
*/
/*
	LColorTable
	
	A color table from control's or window's auxilliary 
	color info.
	
	This class "borrows" the color table handle--it does
	not own the color table handle, it does not dispose it.
	The control or window would be very upset if that
	happened.
	
	CCTabHandle and CTabHandle share the same format,
	so this class works fine for control or window
	custom color tables.
	
	12/27/94	zz	h	rename from LGBColorTable
	11/27/94	zz	h	initial write
*/
#pragma once

#include <Controls.h>

class LColorTable
{
	public :
		// part constants for control and
		// window auxilliary color tables
		typedef enum {
			// cctb/::GetAuxCtl()
			Part_ControlFrame				=  0,
			Part_ControlBody				=  1,
			Part_ControlText				=  2,
			Part_ControlThumb				=  3,
			Part_ControlFillPatternColor	=  4,
			Part_ControlArrowScrollBGLight	=  5,
			Part_ControlArrowScrollBGDark	=  6,
			Part_ControlThumbLight			=  7,
			Part_ControlThumbDark			=  8,
			Part_ControlHighlightLight		=  9,
			Part_ControlHighlightDark		= 10,
			Part_ControlTitleBarLight		= 11,
			Part_ControlTitleBarDark		= 12,
			Part_ControlTingeLight			= 13,
			Part_ControlTingeDark			= 14,
			
			// wctb/actb/dctb/::GetAuxWin()->awCTable/::
			Part_WindowContent				=  0,
			Part_WindowFrame				=  1,
			Part_WindowText					=  2,
			Part_WindowHighlight			=  3,
			Part_WindowTitleBar				=  4,
			Part_WindowStripesTextLight		=  5,
			Part_WindowStripesTextDark		=  6,
			Part_WindowTitleBarBGLight		=  7,
			Part_WindowTitleBarBGDark		=  8,
			Part_WindowDialogBevelFrameLight=  9,
			Part_WindowDialogBevelFrameDark	= 10,
			Part_WindowTingeLight			= 11,
			Part_WindowTingeDark			= 12,

			Part_Unknown					= 0xFFFF
		} PartT;


		LColorTable(void);
		
		void
		GetControlColors(
			const ControlHandle	inControlH);
		
		void
		GetWindowColors(
			const WindowPtr	inWindow);
		
		inline Boolean
		HasColors(void) const
		{	return mColorTable != NULL;	}
		
		Boolean								// did we get a color
		GetColorForPart(
			PartT		inPart,				// content, frame, etc.
			RGBColor	&outColor) const;	// returned color
			
	

	protected :
		CTabHandle	mColorTable;
};
