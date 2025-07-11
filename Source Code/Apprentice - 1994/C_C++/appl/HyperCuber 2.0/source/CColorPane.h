//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CColorPane
//|
//| This is the interfact to a colored pane
//|_________________________________________________________

#pragma once

#include <CPane.h>

CLASS CDirectorOwner;

class CColorPane : public CPane
	{
	
	RGBColor pane_color;				//  Color to fill this pane with
	
  public:

	void	IColorPane(CView *anEnclosure, CBureaucrat *aSupervisor, short aWidth,
							short aHeight, short aHEncl, short aVEncl, SizingOption	aHSizing,
							SizingOption aVSizing, RGBColor *color);
	void	Draw(Rect *area);
	void	DoClick(Point hit, short modifierKeys, long when);
	void	GetPaneColor(RGBColor *color);
	void	SetPaneColor(RGBColor *color);

	};
