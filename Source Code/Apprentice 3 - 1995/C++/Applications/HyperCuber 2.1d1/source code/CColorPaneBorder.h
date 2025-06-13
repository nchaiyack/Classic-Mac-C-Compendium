//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CColorPaneBorder.h
//|
//| This is the interface to a colored pane border
//|______________________________________________________________________

#include <CPaneBorder.h>

class CColorPaneBorder : public CPaneBorder
	{
	
	RGBColor color;
	
  public:

	void	IColorPaneBorder(short borderFlags, RGBColor *color);

	void	DrawBorder(Rect* paneFrame);

	};
