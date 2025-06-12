/**************************************************************************
** Meter.h
**
** Header file for the analog meter-style dial control.
**
** Copyright © 1992, Brent Burton
**
** Permission is granted to freely copy, use, and distribute this
** code for commercial or non-commercial purposes.  If this code
** is used, it is used with the understanding that a credit message
** must appear in the software that uses this code.  I.e., give credit
** where credit is due.  Thanks!
***************************************************************************/


/*************
** Note: inTurnUp and inTurnDown are oppositely named what the
** standard inPageUp/inPageDown values are. (I.e., a scroll
** inPageUp is 22, and inPageDown is 23.)
** The numbering difference is because a meter works in an
** opposite manner than a scroll bar.
**************/
#ifndef METER_H
#define METER_H

#define inNeedle	129		/* same number as scroll bars' thumb */
#define inTurnUp	23		/* turn meter's value up (clockwise) */
#define inTurnDown	22		/* turn meter's value down (CCW) */

#define kMeterPict	-10176	/* Owned resource ID for dial's pict. */

#endif