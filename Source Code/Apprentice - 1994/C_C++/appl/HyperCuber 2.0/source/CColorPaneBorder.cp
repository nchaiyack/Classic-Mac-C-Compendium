//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CColorPaneBorder.cp
//|
//| This implements a colored pane border
//|_______________________________________________________________________________________

#include "CColorPaneBorder.h"



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CColorPaneBorder::IColorPaneBorder
//|
//| Purpose: Initialize the colored pane border.
//|
//| Parameters: flags: border flags (for superclass)
//|             color: the color of the border
//|_________________________________________________________

void CColorPaneBorder::IColorPaneBorder(short borderFlags, RGBColor *the_color)
{

	CPaneBorder::IPaneBorder(borderFlags);			//  Initialize as PaneBorder

	color = *the_color;								//  Save the color

}	//==== CColorPaneBorder::IColorPaneBorder() ====\\



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CColorPaneBorder::DrawBorder
//|
//| Purpose: Draw the colored pane border.
//|
//| Parameters: pane_rect: the pane's rectangle
//|_________________________________________________________

void CColorPaneBorder::DrawBorder(Rect *pane_rect)
{

	RGBColor fore_color;

	GetForeColor(&fore_color);					//  Remember current foreground color
	RGBForeColor(&color);						//  Change foreground color to border color

	CPaneBorder::DrawBorder(pane_rect);			//  Draw the border

	RGBForeColor(&fore_color);					//  Restore foreground color

}	//==== CColorPaneBorder::DrawBorder() ====\\



