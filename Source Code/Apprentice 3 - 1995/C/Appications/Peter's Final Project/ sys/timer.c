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

#include <stdlib.h>
#include <Events.h>

#include "sys.stuff.h"
#include "timer.h"

/*
 * Make a timer object.
 */
 
TIMER
make_timer ()
{
	TIMER timer;
	
	timer = (TIMER) ALLOC (sizeof (struct _TIMER));
	timer->start = TickCount ();
	
	return timer;
}

/*
 * Free a timer object.
 */

void
free_timer (t)
	TIMER t;
{
	FREE (t);
}

/*
 * Start the timer.
 */

void
timer_start (t)
	TIMER t;
{
	t->activated = 1;
	t->start = TickCount ();
}

/*
 * Stop the timer.
 */

void
timer_stop (t)
	TIMER t;
{
	t->activated = 0;
	t->end = TickCount ();
}

/*
 * Reset the timer.
 */

void
timer_reset (t)
	TIMER t;
{
	timer_start (t);
}

/*
 * Return the time elapsed since the timer was started.
 */

float
timer_elapsed (t)
	TIMER t;
{
	float diff;
	
	if (t->activated)
		t->end = TickCount ();
	
	diff = t->end - t->start;
	if (diff != 0)
		return (diff / 60.0);
	else
		return 1.0;
}
