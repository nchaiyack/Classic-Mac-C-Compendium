/* controls.h */

short GetListBufferOffsetFromControl( void );
Rect *GetScrollBarRect( Rect *rect );
void myInitControls( void );
void myDisposeControls( void );
void myDrawControls( void );
void mySetControlMax( void );
pascal void myScrollProc( ControlHandle theControl, short partCode );
void myZeroScrollBar( void );
