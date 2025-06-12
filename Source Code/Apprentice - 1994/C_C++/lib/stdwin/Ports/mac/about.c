/* MAC STDWIN -- "ABOUT STDWIN" MESSAGE. */

#include "macwin.h"
#ifdef MPW
#include <Events.h>
#include <TextEdit.h>
#endif
#ifdef THINK_C_PRE_5_0
#include <EventMgr.h>
#include <TextEdit.h>
#endif

/* Default About... message; applications may assign to this.
   You may also assign to about_item in "menu.c", *before*
   calling winit() or winitargs().
   Also see your THINK C licence.
*/
char *about_message=
	"STDWIN version 0.9.7 (using THINK C 4.0)\r\r\
Copyright \251 1988, 1989, 1990, 1991 Stichting Mathematisch Centrum, \
Amsterdam\r\
Written by Guido van Rossum (guido@cwi.nl)\r\
CWI, dept. AA, P.O.B. 4079\r1009 AB  Amsterdam, The Netherlands\r\r\
[Option-click in window scrolls as in MacPaint,\r\
Option-click in title sends behind]";
	/* \251 is the (c) Copyright symbol.
	   I don't want non-ASCII characters in my source. */


/* "About ..." procedure.
   This is self-contained -- if you have a better idea, change it.
*/

void
do_about()
{
	Rect r;
	WindowPtr w;
	EventRecord e;
	
	SetRect(&r, 0, 0, 340, 180); /* XXX Shouldn't be hardcoded */
	OffsetRect(&r, (screen->portRect.right - r.right)/2, 40);
	
	w = NewWindow(
		(Ptr) NULL,	/* No storage */
		&r,		/* Bounds rect */
		"",		/* No title */
		true, 		/* Visible */
		altDBoxProc,	/* Plain box with shadow */
		(WindowPtr) -1,	/* In front position */
		false,		/* No go-away box */
		0L);		/* RefCon */
	SetPort(w);
	r = w->portRect;
	InsetRect(&r, 10, 10);
	TextBox(about_message, strlen(about_message), &r, teJustCenter);
	while (!GetNextEvent(mDownMask|keyDownMask, &e))
		;
	DisposeWindow(w);
}
