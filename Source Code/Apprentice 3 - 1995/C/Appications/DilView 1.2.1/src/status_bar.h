/* status_bar.h */

#define k_status_buffer_depth 8
#define k_standard_status_msg "\pDilbert is © Scott Adams, United Feature Syndicate, and ClariNet"
#define k_status_gray 0xEFFA

#define k_status_text_offset_h 5
#define k_status_text_offset_v 11

enum {
	k_std_message,
	k_processing,
	k_printing
};

void DrawStatusBar( void );
void myInitStatusBar( void );
void myDisposeStatusBar( void );
Rect *GetStatusBarRect( Rect *rect );
void StatusBarMsg( short type, Str63 fname );
void StatusBarControls( void );
