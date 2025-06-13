/* ==========================================

	wind.proto.h
	
	Copyright (c) 1993,1994,1995 Newport Software Development
	
   ========================================== */

void wind_action( ShellH shell, EventRecord *theEvent );
void wind_grow( ShellH shell, Point p );
int  wind_new( ShellH shell, int w_offset );
void wind_set_scroll( ShellH shell );
void wind_set_view( ShellH shell );
void wind_show_sel( ShellH shell );
void wind_text_adjust( ShellH shell );
void wind_update( ShellH shell );
void wind_zoom( ShellH shell, short part );

pascal void wind_scroll_proc( ControlHandle theControl, int theCode );

pascal Boolean wind_track_click( void );
