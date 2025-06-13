/*
 *  Peter's Final Project -- A texture mapping demonstration
 *  © 1995, Peter Mattis
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

#include <console.h>
#include <profile.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "engine.h"
#include "utils.h"
#include "sys.stuff.h"

/*
 * Declare the functions private to this module.
 */

void do_event_loop (void);
void do_handle_event (long, long);
void do_quit (void);
void do_parse_command_line (int, char **);
short parse_option (char *);
short parse_depth (char *);
short parse_viewsize (char *);
short parse_mazesize (char *);
short parse_wall_texture (char *);
short parse_floor_texture (char *);
short parse_ceiling_texture (char *);
short parse_mode (char *);
short parse_debug (char *);

/*
 * Need a variable to keep track of whether
 *  the program is done or not. Also need one
 *  for storing the program name.
 */

static short done = 0;
char *prog_name;

int
main (argc, argv)
	int argc;
	char *argv[];
{
/*
 * Replace with 1 to turn on profiling.
 */
#if 0
	_ftype = 'TEXT';
	_fcreator = 'R*ch';
	freopen("out", "w+", stdout);

	InitProfile(200, 200);
	_profile = 1;
	_trace = 0;
#endif

	/*
	 * Get a command line.
	 */
	argc = ccommand (&argv);

	/*
	 * Parse the command line options.
	 */
	do_parse_command_line (argc, argv);
	
	/*
	 * Initialize the system. (Create the window, set the
	 *  palette, etc)
	 */
	do_sys_init ();
	
	/*
	 * Set the event handler.
	 */
	do_set_event_handler (do_handle_event);
	
	/*
	 * Initialize the engine.
	 */
	do_engine_init ();

	/*
	 * Enter the event loop.
	 */
	do_event_loop ();

	/*
	 * Exit the engine.
	 */
	do_engine_exit ();

	/*
	 * Exit the system.
	 */
	do_sys_exit ();

	return 0;
}

/*
 * The event loop.
 */
 
void
do_event_loop ()
{
	long event, val;

	while (!done)
	{
		do_sys_event ();
		do_engine_frame ();
	}
}

/*
 * Handle an event.
 */

void
do_handle_event (event, val)
	long event, val;
{
	switch (event)
	{
	case KEY_PRESS_EVENT:
		/*
		 * Pass all key presses to the engine
		 *  except for the ESC key, which quits
		 *  the program.
		 */
		 
		if (val == ESC_KEY)
			do_quit ();
		else
			do_engine_key (val, 1);
		break;
	case KEY_RELEASE_EVENT:
		do_engine_key (val, 0);
		break;
	case BUTTON_PRESS_EVENT:
		break;
	case BUTTON_RELEASE_EVENT:
		break;
	default:
		break;
	}
}

void
do_quit ()
{
	done = 1;
}

void
do_parse_command_line (argc, argv)
	int argc;
	char *argv[];
{
	short i;
	char *token;

	prog_name = argv[0];

	for (i = 1; i < argc; i++)
	{
		parse_option (argv[i]);
	}
}

short
parse_option (token)
	char *token;
{
	if (token[0] == '-')
	{
		token = &token[1];
		if (parse_depth (token) ||
			parse_viewsize (token) ||
			parse_mazesize (token) ||
			parse_wall_texture (token) ||
			parse_floor_texture (token) ||
			parse_ceiling_texture (token) ||
			parse_mode (token) ||
			parse_debug (token))
			return;
		else
			printf ("unknown option: %s\n", token);
	}
	else
		{
		printf ("unable to parse: %s\n", token);
	}

	return 0;
}

short
parse_depth (token)
	char *token;
{
	if (token[0] == 'd')
	{
		do_set_depth (atoi (&token[1]));
		return 1;
	}

	return 0;
}

short
parse_viewsize (token)
	char *token;
{
	if (token[0] == 'v')
	{
		do_set_window_size (atoi (&token[1]));
		return 1;
	}

	return 0;
}

short
parse_mazesize (token)
	char *token;
{
	if (token[0] == 'm')
	{
		do_set_maze_size (atoi (&token[1]));
		return 1;
	}

	return 0;
}

short
parse_wall_texture (token)
	char *token;
{
	if (token[0] == 'w')
	{
		do_set_wall_texture (&token[1]);
		return 1;
	}

	return 0;
}

short
parse_floor_texture (token)
	char *token;
{
	if (token[0] == 'f')
	{
		do_set_floor_texture (&token[1]);
		return 1;
	}

	return 0;
}

short
parse_ceiling_texture (token)
	char *token;
{
	if (token[0] == 'c')
	{
		do_set_ceiling_texture (&token[1]);
		return 1;
	}

	return 0;
}

short
parse_mode (token)
	char *token;
{
	if (token[0] == 'M')
	{
		do_set_mode (atoi (&token[1]));
		return 1;
	}

	return 0;
}

short
parse_debug (token)
	char *token;
{
	if (token[0] == 'D')
	{
		do_set_debug_level (atoi (&token[1]));
		return 1;
	}

	return 0;
}
