//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CHyperScrollBar
//|
//| This is the interface to  a pane containing a number which reflects
//| the setting of a scroll bar
//|______________________________________________________________________

#pragma once

#include <CScrollBar.h>

class CHyperScrollBar : public CScrollBar
	{
	
	Boolean fWrap;					//  TRUE if the bar should wrap-around
	
  public:

	~CHyperScrollBar(void);			//  destructor

	void	IHyperScrollBar(CView *enclosure, CBureaucrat *supervisor,
							Orientation orientation, short length,
							short horiz, short vert, Boolean wrap);
	
	void	SetValue(short value);

	void	DoClick(Point hitPt, short modifierKeys, long when);

	void	TrackThumb(void);
	
	};
