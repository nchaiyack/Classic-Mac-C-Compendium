/*______________________________________________________________________

	new.h - New Ph Record Movable Modal Dialog Interface
_____________________________________________________________________*/

#ifndef __new__
#define __new__

/*_____________________________________________________________________

	Functions.
_____________________________________________________________________*/

extern void new_DoPeriodic (WindowPtr w);
extern void new_DoKey (WindowPtr w, char key, short modifiers);
extern Boolean new_DoCommand (WindowPtr w, short theMenu, short theItem);
extern Boolean new_DoDialog (Str255 alias, Str255 name, Str255 type, 
	Str255 pswd1, Str255 pswd2, short whichField);

#endif