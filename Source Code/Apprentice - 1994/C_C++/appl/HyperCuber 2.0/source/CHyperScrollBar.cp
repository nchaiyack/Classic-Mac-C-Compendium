//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CHyperScrollBar.cp
//|
//| This implements a scroll bar which has "live" thumb tracking, and wraps around
//|_______________________________________________________________________________________

#include "CHyperScrollBar.h"



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CHyperScrollBar::IHyperScrollBar
//|
//| Purpose: Initialize the scroll bar.
//|
//| Parameters: scroll_bar: scroll bar to track
//|             wrap:       TRUE if the bar should wrap around
//|             rest same as superclass
//|____________________________________________________________

void CHyperScrollBar::IHyperScrollBar(CView *enclosure, CBureaucrat *supervisor,
										Orientation orientation, short length,
										short horiz, short vert, Boolean wrap)
{

	CScrollBar::IScrollBar(enclosure, supervisor,		//  Initialize superclass
							orientation, length,
							horiz, vert);

	SetCRefCon(macControl, 0L);							//  This may be replaced by a handle if
														//    the bar is linked to something
	
	fWrap = wrap;										//  Remember whether to wrap
	
}	//==== CHyperScrollBar::IHyperScrollBar() ====\\



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CHyperScrollBar::Dispose
//|
//| Purpose: Dispose of the scroll bar.
//|
//| Parameters: none
//|_________________________________________________________

void CHyperScrollBar::Dispose(void)
{

	Handle handle = (Handle) GetCRefCon(macControl);	//  Get rid of the refcon handle, if any
	if (handle)
		DisposeHandle(handle);
		
	CScrollBar::Dispose();								//  Dispose of superclass
	
}	//==== CHyperScrollBar::Dispose() ====\\



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CHyperScrollBar::SetValue
//|
//| Purpose: Set the value of the scroll bar.  Unlike normal scroll bars, this
//|          wraps around.  So if the value is greater than the max, it is
//|          wrapped to the min.
//|
//| Parameters: value: the value to set
//|____________________________________________________________________________

void CHyperScrollBar::SetValue(short value)
{

	if (fWrap)
		{
		short max = GetMaxValue();
		short min = GetMinValue();
		
		if (value > max)						//  Allow wrap-around
			SetValue(value - (max - min));
	
		else if (value < min)
			SetValue(value + (max - min));
		
		else
			CScrollBar::SetValue(value);		//  Set the value
		}
	else
		CScrollBar::SetValue(value);			//  Set the value, no wrap-around

}	//==== CHyperScrollBar::SetValue() ====\\



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CHyperScrollBar::DoClick
//|
//| Purpose: This is called when the control is clicked (this code taken partly
//|          from TCL CControl::DoClick).
//|
//| Parameters: value: the value to set
//|____________________________________________________________________________

void CHyperScrollBar::DoClick(
	Point		hitPt,					/* Mouse location in Frame coords	*/
	short		modifierKeys,			/* State of modifier keys			*/
	long		when)					/* Tick time of mouse click			*/
{
	register short	whichPart;			/* Part of control clicked in		*/
	short			origValue;			/* Original control setting			*/
	short			delta;				/* Change in control setting		*/
	LongPt			tmp;
	
	QDToLongPt( hitPt, &tmp);
	FrameToWind( &tmp, &hitPt);

										/* Find part of control clicked in	*/
										/*   Note that a part code of zero	*/
										/*   means that the control is		*/
										/*   inactive, so we do nothing		*/
	whichPart = TestControl( macControl, hitPt);
	
	PenNormal();
	
	if (whichPart >= inThumb) {			/* Click inside moving indicator	*/

		TrackThumb();				//  Different from CControl: Call TrackThumb()
	
	} else if (whichPart > 0) {			/* Click inside stationary part		*/
	
		/* The parameter of -1L	means that the actionProc in the	*/
		/* Toolbox control record will be called continuously while	*/
		/* the mouse is down.  If actionProc is NULL, no additional	*/
		/* action is performed.										*/

		if (TrackControl(macControl, hitPt, (ProcPtr)-1L))
		
				/* TrackControl returns the part code if user	*/
				/* executed a good click, i.e., the mouse was	*/
				/* pressed and released in the same part. If so	*/
				/* respond to the click; otherwise, do nothing.	*/

			DoGoodClick(whichPart);
	}
	
}	//==== CHyperScrollBar::DoClick() ====\\



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CHyperScrollBar::TrackThumb
//|
//| Purpose: This is called when the thumb is clicked.  It tracks the mouse
//|          and actively rotated the hypercube according to the mouse position.
//|
//| Parameters: none
//|____________________________________________________________________________

void CHyperScrollBar::TrackThumb(void)
{

	while(StillDown())
		{
		
		Prepare();
		
		Point mouse_point;						//  Find current mouse location
		GetMouse(&mouse_point);
		
		LongRect frame;							//  Find max and min locations of thumb
		GetFrame(&frame);
		short tracking_min = hEncl + 16 + 8;
		short tracking_max = hEncl + frame.right - 16 - 8;
		
		if (mouse_point.h > tracking_max)		//  Pin mouse point to max and min locations
			mouse_point.h = tracking_max;
			
		if (mouse_point.h < tracking_min)
			mouse_point.h = tracking_min;
			
		short min = GetMinValue();				//  Find max and min scroll bar values
		short max = GetMaxValue();
	
		short value = min +						//  Compute scroll bar value correspinding to
			(max - min)*						//    current mouse location
				(mouse_point.h - tracking_min)/
					(tracking_max - tracking_min);
	
		SetValue(value);						//  Change scroll bar value (and redraw hypercube)
	
		}

}	//==== CHyperScrollBar::TrackThumb() ====\\