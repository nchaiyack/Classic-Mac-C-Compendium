/*______________________________________________________________________

	open.h - Open Ph Record Movable Modal Dialog Interface.
_____________________________________________________________________*/

#ifndef __open__
#define __open__

/*_____________________________________________________________________

	Functions.
_____________________________________________________________________*/

extern void open_DoKey (WindowPtr w, char key, short modifiers);
extern Boolean open_DoCommand (WindowPtr w,short theMenu, short theItem);
extern Boolean open_DoDialog (Str255 user);

#endif