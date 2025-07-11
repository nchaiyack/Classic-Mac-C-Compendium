/*
 *  Peter's Final Project -- A texture mapping demonstration
 *  � 1995, Peter Mattis
 *
 *  E-mail:
 *  petm@soda.csua.berkeley.edu
 *
 *  Snail-mail:
 *   Peter Mattis
 *   557 Fort Laramie Dr.
 *   Sunnyvale, CA 94087
 *
 *  Avaible from:
 *  http://www.csua.berkeley.edu/~petm/final.html
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef __SYS_STUFF_H__
#define __SYS_STUFF_H__

#include <stdlib.h>
#include "sys.types.h"

#define ALLOC(x)     (malloc(x))
#define FREE(x)      (free(x))

#ifndef NULL
#define NULL 0L
#endif

typedef void (*EVENT_HANDLER) (long, long);

void do_set_window_size (short);
void do_set_depth (short);
void do_set_mode (short);
void do_set_event_handler (EVENT_HANDLER);

void do_sys_init (void);
void do_sys_exit (void);
void do_sys_event (void);
void *do_sys_read_graphic_file (char *, long *, long *);

void do_update_screen (void);
void do_clear_frame_buffer (void);

void do_set_color (long);

void *get_frame_buffer_address (void);
long get_frame_buffer_width (void);
long get_frame_buffer_height (void);
long get_frame_buffer_depth (void);
long get_frame_buffer_pixel (void);

#define NULL_EVENT            1
#define KEY_PRESS_EVENT       2
#define KEY_RELEASE_EVENT     3
#define BUTTON_PRESS_EVENT    4
#define BUTTON_RELEASE_EVENT  5

#define ESC_KEY   0xFF1B
#define TAB_KEY   0xFF09
#define LEFT_KEY  0xFF51
#define RIGHT_KEY 0xFF53
#define UP_KEY    0xFF52
#define DOWN_KEY  0xFF54

#endif /* __SYS_STUFF_H__ */
