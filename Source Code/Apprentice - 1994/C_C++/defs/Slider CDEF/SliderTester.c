#include "SliderCDEF.h"

#define rCustomCtrl			128
#define rMainWindow		128

#define kTop				53
#define kLeft				13
#define kWidth				400
#define kHeight				16
#define kTitleText			"\pSlider CDEF"
#define kTitleTextLeft		153
#define kTitleTextBottom		30
#define kMessageText		"\pPress any key to quit"
#define kMessageTextLeft		150
#define kMessageTextBottom	90


void DrawShadowText( Str255 theText, short h, short v  );

Boolean gQuit;

void main( void )
{
	WindowPtr	mainWindow, theWindow;
	EventRecord	theEvent;
	ControlHandle	sliderControl, theControl;
	short		part;
	Point			pt;
	short		itemHit;
	
	gQuit = false;
	InitGraf( &qd.thePort );
	InitFonts();
	InitWindows();
	InitMenus();
	TEInit();
	InitDialogs( nil );
	InitCursor();	

	mainWindow = GetNewCWindow( rMainWindow, nil, (WindowPtr) -1L );
	SetPort( mainWindow) ;
	ShowWindow( mainWindow );

	sliderControl = GetNewControl( rCustomCtrl,  mainWindow );
	ShowControl( sliderControl );

	while( !gQuit ) {
		if ( WaitNextEvent( everyEvent, &theEvent, 1, nil ) ) {
			switch( theEvent.what ) {
				case mouseDown:
					part = FindWindow( theEvent.where, &theWindow );
					if ( part == inDrag ) {
						DragWindow( theWindow, theEvent.where, &(**(GetGrayRgn())).rgnBBox );
					} else if ( part == inContent ) {
						pt = theEvent.where;
						GlobalToLocal( &pt );
						SetPort( mainWindow) ;
						part = FindControl( pt, theWindow, &theControl );
						if ( part ) {
							TrackControl( theControl, pt, nil );
						}
					}
					break;
				case autoKey:
				case keyDown:
					gQuit = true;
					break;
				case updateEvt:
					// Get the window from the event's message field.
					theWindow = (WindowPtr) theEvent.message;
					
					// Begin updating.
					BeginUpdate( theWindow );
					
					// Set the pen to normal and set the font attributes.
					PenNormal();
					TextFont( helvetica );
					TextFace( bold + italic);

					// Draw the title text.
					TextSize( 18 );					
					DrawShadowText( kTitleText, kTitleTextLeft,
						kTitleTextBottom );

					// Draw the message text.
					TextSize( 10 );
					DrawShadowText( kMessageText, kMessageTextLeft,
						kMessageTextBottom );

					// Update the window's control (the slider).
					DrawControls( theWindow );

					// End updating.
					EndUpdate( theWindow );
					break;
			}
		}
	}
}



// DrawShadowText
//
// Utility routine to draw 3-d appearing text.
void DrawShadowText( Str255 theText, short h, short v  )
{
	RGBColor	color;
	
	color.red = color.blue = color.green = 0xFFFF;
	RGBForeColor( &color );
	MoveTo( h+1, v+1 );
	DrawString( theText );

	color.red = color.blue = color.green = 0x0000;
	RGBForeColor( &color );
	MoveTo( h, v );
	DrawString( theText );
}