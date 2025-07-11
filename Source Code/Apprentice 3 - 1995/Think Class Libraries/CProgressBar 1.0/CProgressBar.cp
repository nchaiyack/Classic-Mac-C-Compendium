/******************************************************************************
 CProgressBar.cp

		
	SUPERCLASS = CPane
	
	Copyright � 1994 Johns Hopkins University. All rights reserved.
	
	Original Author: 	Martin R. Wachter		email:	mrw@welchgate.welch.jhu.edu
	
	Modified:			4/27/94					by:		mrw			TCL Version:	1.1.3
	Created:			3/25/94					by:		mrw			TCL Version:	1.1.3

	CProgressBar is a subclass of CPane which emulates the Finder's progress bar
	when you copy files.  Use it like any other CPane subclass.
	
	Call UpdateProgress with a percentage complete to "animate" the progress
	fill area.
	
	You can specify any RGB colors that you want for the background and the fill
	bar areas of CProgressBar.  Call UseFinderProgressColros to use the same
	colors that the Finder uses, or call UseSystemTinges for the System's color 
	tinges as set by the user in the Color CDEV.

	Version change history:
	
	1.0		Initial release.
	
 ******************************************************************************/
 
#include "Global.h"
#include "CProgressBar.h"
#include <CPaneBorder.h>

static RGBColor	FinderFillColor = {17476,17476,17476};
static RGBColor	FinderBackColor = {52428,52428,65535};

/******************************************************************************
 IScrollPane

	Initialize a ScrollPane object
	
******************************************************************************/

void	CProgressBar::IProgressBar(
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
	RGBColor 		rgbBColor)
{		
	CPaneBorder *paneBorder;

	CPane::IPane(anEnclosure, aSupervisor,aWidth, aHeight, aHEncl, aVEncl, aHSizing, aVSizing);
	
	paneBorder = new( CPaneBorder);
	paneBorder->IPaneBorder( kBorderFrame);
	SetBorder(paneBorder);
	
	useColor = aColor && gSystem.hasColorQD;
	isVertical = aVertical;
	useShadow = aShadow;
	itsRGBFillColor = rgbFColor;
	itsRGBBackColor = rgbBColor;
	itsPercent = 0;
}

/******************************************************************************
UseFinderProgressColors

	Use the same colors that the Finder's  progress bar uses.
******************************************************************************/

void CProgressBar::UseFinderProgressColors(void)
{
	itsRGBFillColor = FinderFillColor;
	itsRGBBackColor = FinderBackColor;
}

/******************************************************************************
UseSystemTinges

	Use the System's Highlight and Window colors. for the fill and back colors.
******************************************************************************/

void CProgressBar::UseSystemTinges(void)
{
	GetWindowTinges(&itsRGBBackColor,&itsRGBFillColor);
}

/******************************************************************************
GetWindowTinges

	Returns the RGB values which the user has set in the Color CDEV
	for the System�s Highlight and Window colors.
******************************************************************************/

void CProgressBar::GetWindowTinges(RGBColor *lightTinge, RGBColor *darkTinge)
{
 CTabHandle windowCTable;
 
	lightTinge->red = lightTinge->green = lightTinge->blue = 0xffff;
	darkTinge->red = darkTinge->green = darkTinge->blue = 0x0000;

	if ((windowCTable = (CTabHandle)GetResource('wctb', 0)) != nil){
		*lightTinge = (**windowCTable).ctTable[11].rgb;
		*darkTinge = (**windowCTable).ctTable[12].rgb;
	}

	// case for black and white window defs under system 7, both return black!
	
	if ((lightTinge->red == 0x0000) && (lightTinge->green == 0x0000) &&
		(lightTinge->blue == 0x0000))
		lightTinge->red = lightTinge->green = lightTinge->blue = 0xffff;

}

/******************************************************************************
 Draw {OVERRIDE}

		Draw the Progress bar
		
******************************************************************************/

void CProgressBar::Draw( void )
{
	PenState	pen;
	Rect		r;
	RGBColor	BlackColor = {0x0000, 0x0000, 0x0000},
				WhiteColor = {0xFFFF, 0xFFFF, 0xFFFF};


	LongToQDRect(&frame,&r);

	Prepare();
	GetPenState( &pen );
	PenNormal();
	
	// paint the background
	if ( useColor ){
		RGBForeColor(&itsRGBBackColor);
		PaintRect( &r );		
	}
	else{
		PenPat ( white );
		PaintRect( &r );
	}

	// paint the fill area
	if ( useColor ){
		RGBForeColor(&itsRGBFillColor);
	}
	else{
		PenPat ( gray );
	}
	PaintRect( &itsFillRect );
	
	// draw the shadow
	if (shadow) {
	
		if ( useColor ){
			RGBForeColor(&BlackColor);
		}
		else{
			PenPat ( black );
		}

		MoveTo( r.left + SHADOW_DEPTH, r.bottom );
		LineTo( r.right, r.bottom );
		LineTo( r.right, r.top + SHADOW_DEPTH );
	}
	
	SetPenState( &pen );
	
}

/******************************************************************************
UpdateProgress

	Given a percentage of completion, UpdateProgress will set itsFillRect
	to the appropriate size.  The Draw method actually draws the fill bar.

	Repeated calls to Update with a larger percentage is how the bar is animated

******************************************************************************/
void CProgressBar::UpdateProgress( short percent )
{
	Rect		tempRect;
	short		rectLength, barFill;
		
	LongToQDRect(&frame,&tempRect);
	
	if ( ! isVertical ) {
		rectLength = tempRect.right - tempRect.left;
	
		if (percent == 100)
			barFill = rectLength;
		else
			barFill = rectLength * .01 * percent;

		SetRect(&itsFillRect,tempRect.left, tempRect.top,
				barFill, tempRect.bottom);
	} 
	else{
		rectLength = tempRect.bottom - tempRect.top;
		if (percent == 100)
			barFill = rectLength;
		else
			barFill = rectLength * .01 * percent;

		SetRect(&itsFillRect,tempRect.left, barFill,
				tempRect.right, tempRect.bottom);
	}
	
	CProgressBar::Draw();
}