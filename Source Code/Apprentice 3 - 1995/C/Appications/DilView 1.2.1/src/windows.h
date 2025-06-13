/* windows.h */

#include <QDOffScreen.h>

#ifndef __my_constants__
 #include "constants.h"
#endif

#define k_window_id 128

// if the last position is offscreen, use this default position:
#define k_default_win_h 20
#define k_default_win_v 40

// how many port stores can we do before overflowing?
#define k_port_stack_size 10

// a magic number for how much beyond the scaled dil height a window needs to be.
// formula 3 scrollbar widths + 2 for the list frame.. sorry for the magic numbers!!
// + 3 more so 2 rows will fit nicely
#define k_min_win_height_additive (((3 * k_scrollbar_width) + 2) + 3) 

WindowPtr GetGWinPort( void );
void myInitWindows( void );
void myDisposeWindows( void );
pascal void myContentClick( Point where, long when, short modifiers );
pascal void myUpdate( Boolean resized );
pascal void myCloseBoxClick( void );
pascal void myZoomBoxClick( WindowPtr w, short zoomDir );
void ScaleWindow( void );
Rect *GetWinRect( Rect *rect );
void myShowWindow( void );
void CopyGWorldToWindow( GWorldPtr theGWorld, Rect *srcRect, Rect *destRect, short mode );
void SetPortToWindow( void );
void SetPortToGWorld( GWorldPtr theGWorld );
void RestorePort( void );