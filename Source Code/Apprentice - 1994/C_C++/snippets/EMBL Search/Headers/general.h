#pragma once

#ifndef NULL
#define NULL		(void*)0L
#endif

#define EOS			'\0'
#define LINELEN		512
#define ESCAPE		0x35

typedef unsigned long u_long;
typedef unsigned short u_short;
typedef unsigned char u_byte;

#define TopLeft(aRect)		(* (Point *) &(aRect).top)
#define BottomRight(aRect)	(* (Point *) &(aRect).bottom)

#define SBarWidth	15			/* Scroll bar width */

#define BTNON	1				/* radio button status */
#define BTNOFF	0

#define ACTIVE 		0			/* control hilite status */
#define INACTIVE 	255
