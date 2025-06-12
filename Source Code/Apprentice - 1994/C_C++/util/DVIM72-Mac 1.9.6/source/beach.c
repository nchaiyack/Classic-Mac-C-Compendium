Boolean setupbeachball( void );
void rollbeachball( void );

typedef struct {
	short	num_cursors;
	short	tick_interval;
	Cursor	*cursor[];	// in resource, hi word has CURS resource ID
}	acur;

/* (c) copyright 1987 Symantec Corporation.  For use with THINK's Lightspeed
C only.

You may incorporate these routines in any of your programs, for commercial
distribution or otherwise, but you may not charge for the specific use of
this code.  Symantec Corporation makes no warranty regarding the fitness of
this software for any particular purpose.

beach.c -- Simple little program that rolls a "beach ball cursor" while
something important but time-consuming is happening.  Lots of commercial
applications are doing this as a standard way to keep the user comfortable
that the machine hasn't crashed -- that it's still busy doing whatever its
supposed to be doing.

Also included is the resource source in beach.r.  There are four cursors with
id's of 256, 257, 258 and 259, representing each of the four possible beach
ball states.

To change the rate of rotation, just change the constant "beachrate".

To use the beachball in your application, just call "setupbeachball" at the
beginning of your program and add calls to "rollbeachball" in places where
your program is doing a lot of computation, or reading from disk, or
printing, or compiling or whatever...

First upload to LVTFORUM -- 1/6/88, by Dave Winer/LVT.
*/

#define mod		%


static int beachstate = 1; /*current state of the beachball, varies from 0 to 3*/
static short	beachrate;	/* ticks between frames */
static long tickalarm; /*the time the ball is scheduled to roll again*/
static short	num_cursors;
static acur		**acur_h;


Boolean setupbeachball ()
{
	/*
	call this routine before your first call to rollbeachball
	*/
	
	register short	i;
	register CursHandle cursor;
	register short	curs_id;
	
	tickalarm = TickCount (); /*roll the ball the first time rollbeachball is called*/
	if (acur_h == NIL)	// first time we rolled the ball
	{
		acur_h = (acur **) Get1Resource( 'acur', 128 );
		if (acur_h == NIL)
			return false;
		num_cursors = (**acur_h).num_cursors;
		beachrate = (**acur_h).tick_interval;
		
		for (i = 0; i < num_cursors; i++)
		{
			curs_id = ((long)(**acur_h).cursor[i]) >> 16;
			cursor = GetCursor(curs_id); /*pull it out of the resource file*/
			
			if (cursor == nil) /*error loading the cursor*/
				return (false);
			
			MoveHHi( (Handle) cursor );
			HLock( (Handle) cursor );
			(**acur_h).cursor[i] = *cursor; /*no error, copy into array*/
		} /*for*/
		
	}
	
	return (true); /*all beachballs loaded correctly*/
} /*setupbeachball*/
	
void
rollbeachball ()
{
	
	/*
	if enough time has elapsed since the last roll, reset the timer and
	roll the beachball into the next state.
	*/
	
	register long tc; 
	register Cursor *cursor;
	
	tc = TickCount ();
	
	if (tc < tickalarm) /*not enough time has elapsed since last roll*/
		return;
		
	tickalarm = tc + beachrate; /*time for the next roll*/
	
	beachstate = (beachstate + 1) mod num_cursors; /*advance to next state*/
	
	cursor = (**acur_h).cursor[beachstate]; /*copy from array*/
	
	SetCursor (cursor); /*show the next state*/
} /*rollbeachball*/
