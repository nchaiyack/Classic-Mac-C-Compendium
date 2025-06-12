//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CColorPane
//|
//| This implements a colored pane
//|_________________________________________________________

#include "CColorPane.h"
#include <Picker.h>



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CColorPane::IColorPane
//|
//| Purpose: Initialize the colored pane.
//|
//| Parameters: none
//|_________________________________________________________

void CColorPane::IColorPane(CView *anEnclosure, CBureaucrat *aSupervisor, short aWidth,
							short aHeight, short aHEncl, short aVEncl, SizingOption	aHSizing,
							SizingOption aVSizing, RGBColor *color)
{

	IPane(anEnclosure, aSupervisor, aWidth, aHeight, aHEncl, aVEncl, aHSizing, aVSizing);
													//  Call superclass' initialization routine

	pane_color = *color;							//  Set the fill color

}	//==== CColorPane::IColorPane() ====\\


//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CColorPane::Draw
//|
//| Purpose: Draw the colored pane.
//|
//| Parameters: ignored
//|_________________________________________________________

void CColorPane::Draw (Rect *area)
{

	RGBColor	rgb_black =		{0x0000, 0x0000, 0x0000};

	Prepare();
	RGBForeColor(&pane_color);				//  Change color or pattern to desired

	PaintRect(area);
	
	RGBForeColor(&rgb_black);				//  Change back to black

}	//==== CColorPane::Draw ====\\



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CColorPane::DoClick
//|
//| Purpose: Handle a click in the colored pane.  This method opens the color picker
//|          and lets the user select a new color.
//|
//| Parameters: ignored
//|___________________________________________________________________________________

void CColorPane::DoClick(Point hit, short modifierKeys, long when)
{

	Point where_point = {0, 0};

	RGBColor color;
	color = pane_color;
	Boolean good = GetColor(where_point,				//  Let user select a new color
					"\pPlease choose a new color",
						&color, &color);
	pane_color = color;

	Rect area;
	LongToQDRect(&frame, &area);
	Draw(&area);

}	//==== CColorPane::DoClick ====\\



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CColorPane::GetPaneColor
//|
//| Purpose: Get the current color of this pane.
//|
//| Parameters: color: receives the color of the pane
//|___________________________________________________________________________________

void CColorPane::GetPaneColor(RGBColor *color)
{

	*color = pane_color;

}	//==== CColorPane::GetPaneColor() ====\\



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CColorPane::SetPaneColor
//|
//| Purpose: Set the current color of this pane.
//|
//| Parameters: color: the new color of the pane
//|___________________________________________________________________________________

void CColorPane::SetPaneColor(RGBColor *color)
{

	pane_color = *color;								//  Change the color

	Rect area;
	LongToQDRect(&frame, &area);						//  Redraw the pane
	Draw(&area);

}	//==== CColorPane::SetPaneColor() ====\\



