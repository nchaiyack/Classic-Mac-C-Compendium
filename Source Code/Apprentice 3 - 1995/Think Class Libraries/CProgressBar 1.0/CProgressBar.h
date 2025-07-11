/******************************************************************************
 CProgressBar.cp

		
	SUPERCLASS = CPane
	
	Copyright � 1994 Johns Hopkins University. All rights reserved.
	

 ******************************************************************************/

#pragma once

#define	_H_CProgressBar

#include <CPane.h>

enum {				// use color or not
	kDontUseColor,
	kUseColor
};

enum {				// orientation of the bar
	kHorizontal,
	kVertical
};

enum {				// shadow the bar's frame
	KNoShadow,
	kShadow
};

#define SHADOW_DEPTH	2


class CProgressBar : CPane {

public:

	Boolean		useShadow;
	Boolean		isVertical;
	Boolean		useColor;
	RGBColor	itsRGBFillColor;
	RGBColor	itsRGBBackColor;
	short		itsPercent;
	Rect		itsFillRect;
	
	void	IProgressBar(
			CView			*anEnclosure,
			CBureaucrat		*aSupervisor,
			short			aWidth,
			short			aHeight,
			short			aHEncl,
			short			aVEncl,
			SizingOption	aHSizing,
			SizingOption	aVSizing,
			Boolean			aColor,
			Boolean			aVertical,
			Boolean			aShadow,
			RGBColor 		rgbFColor,
			RGBColor 		rgbBColor);
	
	virtual void 	Draw(void);
	virtual	void	UseFinderProgressColors(void);
	virtual void	UseSystemTinges(void);
	virtual void	UpdateProgress( short percent );
	
protected:

	void CProgressBar::GetWindowTinges(RGBColor *lightTinge, RGBColor *darkTinge);

};