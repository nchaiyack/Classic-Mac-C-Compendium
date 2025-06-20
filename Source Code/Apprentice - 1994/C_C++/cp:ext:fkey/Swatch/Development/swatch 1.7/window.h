/**

	window.h
	Copyright (c) 1990-1992, joe holt

 **/


#ifndef __window__
#define __window__

/*******************************************************************************
 **
 **	Public Macros
 **
 **/

#ifndef __ctypes__
#include "ctypes.h"
#endif


/*******************************************************************************
 **
 **	Public Variables
 **
 **/

typedef void (*MBox_proc_t)( Rect *r );
extern WindowPtr App_window;
extern ControlHandle horiz_scroll, vert_scroll;
extern int16 MBox_right;


/*******************************************************************************
 **
 **	Public Functions
 **
 **/

void Window_init( void );
void Window_idle( void );
Boolean Window_mousedown( EventRecord *the_event, int16 part );
void Window_update( EventRecord *the_event );
void Window_activate( EventRecord *the_event, Boolean active_state );
void Window_close( void );

ControlHandle Window_horiz_scroll( void );
ControlHandle Window_vert_scroll( void );
Boolean Window_reset_horiz_scroll( void );
void Window_reset_vert_scroll( void );

void Window_set_mbox( MBox_proc_t *handler );
void Window_erase_mbox( void );
void Window_update_mbox( void );

void Window_clip_with_controls( void );
void Window_clip_without_controls( void );

#endif  /* ifndef _H_SWATCH_WINDOW */
