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

#include <MacHeaders>
#include <Palettes.h>
#include <QDOffscreen.h>

#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>

#include "sys.stuff.h"
#include "utils.h"

static void do_sys_init_graphics(void);
static void do_sys_end_graphics(void);
static void do_sys_create_window(void);

static void do_update_screen8(void);
static void do_update_screen24(void);
static void do_clear_frame_buffer8(void);
static void do_clear_frame_buffer24(void);

static long default_size = 200;
static long default_depth = 8;
static long default_pixel = 1;

static CWindowPtr win;
static PaletteHandle palette;
static void *image_mem;
static void *screen_mem;
static long screen_width;
static long screen_height;

static PIXEL8 color8;
static PIXEL24 color24;

static short mode = 1;

static EVENT_HANDLER event_handler;

void
do_set_window_size(size)
short size;
{
	default_size = size;
}

void
do_set_depth(depth)
short depth;
{
	default_depth = depth;

	if ((default_depth != 8) && (default_depth != 24))
		fatal_error ("Invalid display size %d", default_depth);

	switch (default_depth)
	{
	case 8:
		default_pixel = 1;
		break;
	case 24:
		default_pixel = 4;
		break;
	}
}

void
do_set_mode (m)
short m;
{
	mode = m;
}

void
do_set_event_handler(handler)
EVENT_HANDLER handler;
{
	event_handler = handler;
}

void
do_sys_init()
{
	do_sys_init_graphics();
	do_sys_create_window();
}

void
do_sys_exit()
{
	do_sys_end_graphics();
}

static void
do_sys_init_graphics()
{
	MaxApplZone();

	InitGraf((Ptr) &qd.thePort);
	InitFonts();
	InitWindows();
	InitMenus();
	FlushEvents(everyEvent, 0);
	TEInit();
	InitDialogs(0L);
	InitCursor();
}

static void
do_sys_end_graphics()
{
	DisposeWindow((WindowPtr) win);
	FlushEvents(everyEvent, 0);
}

static void
do_sys_create_window()
{
	GDHandle gdev;
	Rect r, *bounds;
	char *screen_addr;
	CTabHandle ctab;

	SetRect(&r, 0, 20, 0 + default_size, 20 + default_size);
	win = (CWindowPtr) NewCWindow(nil, &r, "\p", false, plainDBox, (WindowPtr) - 1, 0, 0);
	ShowWindow((WindowPtr) win);
	SetPort((WindowPtr) win);

	ctab = GetCTable(128);
	assert(ctab != NULL);

	palette = NewPalette((**ctab).ctSize + 1, ctab, pmAnimated + pmExplicit, 0);
	NSetPalette((WindowPtr) win, palette, pmFgUpdates);

	DisposeCTable(ctab);

	PmForeColor(1);
	PaintRect(&win->portRect);

	bounds = &(*(win->portPixMap))->bounds;
	screen_width = (*(win->portPixMap))->rowBytes & 0x7FFF;
	screen_height = 0;
	screen_addr = GetPixBaseAddr(win->portPixMap) + 
		(screen_width * -bounds->top) - bounds->left;
	screen_mem = screen_addr;

	image_mem = ALLOC(default_pixel * default_size * default_size);
	assert(image_mem != NULL);
}

void
do_sys_event()
{
	static char key_down[6] = { 
		0, 0, 0, 0, 0 	};
	static char key_part[6] = { 
		1, 1, 3, 3, 3, 3 	};
	static long key_val[6] = { 
		ESC_KEY, TAB_KEY, LEFT_KEY, RIGHT_KEY, UP_KEY, DOWN_KEY 	};
	static long key_mask[6] = 
		{ 
		0x00002000, 0x00000100, 0x00000008, 0x00000010, 0x00000040, 0x00000020 	};

	KeyMap keys;
	long event_type;
	long val;
	short i;

	GetKeys(keys);

	for (i = 0; i < 6; i++)
	{
		if (keys[key_part[i]] & key_mask[i])
		{
			val = key_val[i];
			event_type = KEY_PRESS_EVENT;
			if (!key_down[i] && event_handler)
			{
				key_down[i] = 1;
				(*event_handler) (event_type, val);
			}
		}
		else if (key_down[i])
		{
			key_down[i] = 0;
			val = key_val[i];
			event_type = KEY_RELEASE_EVENT;
			if (event_handler)
				(*event_handler) (event_type, val);
		}
	}
}

void*
do_sys_read_graphic_file(file_name, width, height)
char *file_name;
long *width;
long *height;
{
	return NULL;
}

void
do_update_screen()
{
	switch (default_depth)
	{
	case 8:
		do_update_screen8();
		break;
	case 24:
		do_update_screen24();
		break;
	}
}

void 
do_clear_frame_buffer()
{
	switch (default_depth)
	{
	case 8:
		do_clear_frame_buffer8();
		break;
	case 24:
		do_clear_frame_buffer24();
		break;
	}
}

void
do_set_color(c)
long c;
{
	switch (default_depth)
	{
	case 8:
		color8 = c;
		break;
	case 24:
		color24 = c;
		break;
	}
}

void*
get_frame_buffer_address()
{
	return image_mem;
}

long
get_frame_buffer_width()
{
	return default_size;
}

long
get_frame_buffer_height()
{
	if (mode)
		return default_size >> 1;
	else
		return default_size;
}

long
get_frame_buffer_depth()
{
	return default_depth;
}

long
get_frame_buffer_pixel()
{
	return default_pixel;
}

static void 
do_update_screen8()
{
#define COPY_PIXEL(s, d)	*d++ = *s++

	register long rows, cols;
	register PIXEL24 *s, *d;
	register long diff_d;
	register long extra;
	register long size;

	s = (PIXEL24*) get_frame_buffer_address();
	d = (PIXEL24*) screen_mem;

	if (mode)
		diff_d = ((screen_width - get_frame_buffer_width()) >> 2) + (screen_width >> 2);
	else
		diff_d = ((screen_width - get_frame_buffer_width()) >> 2);

	size = get_frame_buffer_width() >> 2;
	extra = size & 0xF;
	size >>= 4;

	rows = get_frame_buffer_height();
	while (rows--)
	{
		cols = size;

		switch (extra)
		{
			do {
				COPY_PIXEL(s, d);
			case 15:
				COPY_PIXEL(s, d);
			case 14:
				COPY_PIXEL(s, d);
			case 13:
				COPY_PIXEL(s, d);
			case 12:
				COPY_PIXEL(s, d);
			case 11:
				COPY_PIXEL(s, d);
			case 10:
				COPY_PIXEL(s, d);
			case 9:
				COPY_PIXEL(s, d);
			case 8:
				COPY_PIXEL(s, d);
			case 7:
				COPY_PIXEL(s, d);
			case 6:
				COPY_PIXEL(s, d);
			case 5:
				COPY_PIXEL(s, d);
			case 4:
				COPY_PIXEL(s, d);
			case 3:
				COPY_PIXEL(s, d);
			case 2:
				COPY_PIXEL(s, d);
			case 1:
				COPY_PIXEL(s, d);
			case 0:
				;
			}
			while (cols--);
		}

		d += diff_d;
	}
}

static void 
do_update_screen24()
{
}

static void
do_clear_frame_buffer8()
{
	register long rows, cols;
	register PIXEL24 *p, c;
	register long size;
	register long extra;

	p = (PIXEL24*) get_frame_buffer_address();
	c = color8;
	c += c << 8;
	c += c << 16;

	size = get_frame_buffer_width() >> 2;
	extra = size & 0x1F;
	size >>= 5;

	rows = get_frame_buffer_height();
	while (rows--)
	{
		cols = size;

		switch (extra)
		{
			do {
				*p++ = c;
			case 31:
				*p++ = c;
			case 30:
				*p++ = c;
			case 29:
				*p++ = c;
			case 28:
				*p++ = c;
			case 27:
				*p++ = c;
			case 26:
				*p++ = c;
			case 25:
				*p++ = c;
			case 24:
				*p++ = c;
			case 23:
				*p++ = c;
			case 22:
				*p++ = c;
			case 21:
				*p++ = c;
			case 20:
				*p++ = c;
			case 19:
				*p++ = c;
			case 18:
				*p++ = c;
			case 17:
				*p++ = c;
			case 16:
				*p++ = c;
			case 15:
				*p++ = c;
			case 14:
				*p++ = c;
			case 13:
				*p++ = c;
			case 12:
				*p++ = c;
			case 11:
				*p++ = c;
			case 10:
				*p++ = c;
			case 9:
				*p++ = c;
			case 8:
				*p++ = c;
			case 7:
				*p++ = c;
			case 6:
				*p++ = c;
			case 5:
				*p++ = c;
			case 4:
				*p++ = c;
			case 3:
				*p++ = c;
			case 2:
				*p++ = c;
			case 1:
				*p++ = c;
			case 0:
				;
			}
			while (cols--);
		}
	}
}

static void
do_clear_frame_buffer24()
{
}
