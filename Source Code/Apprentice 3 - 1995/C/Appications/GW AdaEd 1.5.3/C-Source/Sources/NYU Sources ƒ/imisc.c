/*
 * Copyright (C) 1985-1992  New York University
 * 
 * This file is part of the Ada/Ed-C system.  See the Ada/Ed README file for
 * warranty (none) and distribution info and also the GNU General Public
 * License for more details.
 */
#include "ipredef.h"
#include "_time.h"
#include "imisc.h"

/* Procedures reset_clock and itime are used to maintain the 'clock'.
 * reset_now resets the clock, itime returns the number of milliseconds
 * since the clock was reset. Code is include to make it appear as though
 * the clock was reset at the most recent midnight, in order that the
 * addition of TIME and DURATION values can account for the
 * overflow that occurs at midnight.
 */

static long since_midnight;    /* milliseconds since midnight at start */
static long start_time; /* starting time (units vary according to version) */

/* set TIME_KIND to determine which itime and reset_clock to use */

#define TIME_PC

/* start_time is in 'ticks' elapsed since midnight, where there are 
 * CLK_TCK ticks per second.
 * To avoid overflow, we use ms_per_tick, milliseconds per tick. This
 * assumes (as is currently the case) that CLK_TCK divides 1000
 * evenly.
 */
 
#ifdef MAC_GWADA
	/* � File: imisc.c
	   � Problem:
	   � CLK_TCK not defined.
	   �
	   � Solution:
	   � Defined it to CLOCKS_PER_SEC, defined in ANSI library.
	   � This might be a problem since CLOCKS_PER_SEC doesn't divide 1000 evenly,
	   � as suggested above.
	   � */
#define ms_per_tick (1000 / CLOCKS_PER_SEC)
#else
#define ms_per_tick (1000 / CLK_TCK)
#endif


void reset_clock(void)                                /*;reset_clock*/
{
        /* set start_time and since_midnight as described above. */
        start_time = clock() * (long) ms_per_tick; /* milliseconds since midnight */
        since_midnight = start_time;
}

long itime(void)                             /*;itime*/
{
    /* find elapsed seconds, convert to milliseconds, and add offset 
     *  for midnight corresponding to desired origin
     */
        long  itim;
        clock_t time_now;
        time_now = clock() * (long) ms_per_tick;

        /* adjust for passing midnight if necessary */
        if (time_now < start_time) time_now = time_now + 86400000L ;
        itim =   (long) (time_now - start_time);
        return itim + since_midnight;
}