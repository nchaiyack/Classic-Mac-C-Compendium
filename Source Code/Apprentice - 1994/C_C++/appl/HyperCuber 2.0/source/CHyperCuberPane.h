/****
 * CHyperCuberPane.h
 *
 *	The main pane for the graphics window.
 *
 ****/

#pragma once

#include <CPane.h>
#include <CEditText.h>

class __pasobj CCheckBox;
class __pasobj CRadioControl;
class __pasobj CGraphic;
class __pasobj CGraphic3D;
class __pasobj CGraphic4D;
class __pasobj CScrollBar;

enum {mono, two_image_stereo, two_color_stereo};

class CHyperCuberPane : public CPane {



 public:

	CGraphic	*graphic;					//  Graphic associated with this pane

	Boolean fAddOver;						//  TRUE if this graphic should be drawn in AddOver mode.
	Boolean	fUseNativeColors;				//  TRUE if the graphic's declared colors should be used
											//    when drawing (otherwise, all parts of the graphic
											//    are drawn in GraphicColor).
	Boolean	fDrawOffscreen;					//  TRUE if actual drawing should be done off-screen
	Boolean	fAntialias;						//  TRUE if the object should be antialiased
	short	StereoMode;						//  Drawing mode to use: mono, two_image_stereo, or
											//    two_color_stereo

	RGBColor GraphicColor;					//  The color to draw the graphic if fUseNativeColors
											//    is FALSE.

	CGrafPort	OffscreenPort;				//  The offscreen grafport


									/** Contruction/Destruction **/
	void		IHyperCuberPane(CView *anEnclosure, CBureaucrat *aSupervisor,
							short aWidth, short aHeight,
							short aHEncl, short aVEncl,
							SizingOption aHSizing, SizingOption aVSizing);

	void		Dispose(void);
									/** Drawing **/
	void		Draw(Rect *area);

	void		UpdateGraphicsPanes(void);
	
									/** Mouse **/
	void		DoClick(Point hitPt, short modifierKeys, long when);
	
									/** Cursor **/

	void		AdjustToEnclosure(Rect *deltaEncl);
		
};
