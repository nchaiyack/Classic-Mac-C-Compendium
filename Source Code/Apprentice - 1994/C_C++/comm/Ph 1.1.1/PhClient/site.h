/*______________________________________________________________________

	site.h - Default Site Movable Modal Dialog Interface.
_____________________________________________________________________*/

#ifndef __site__
#define __site__

/*_____________________________________________________________________

	Functions.
_____________________________________________________________________*/

extern void site_DoKey (WindowPtr w, char key, short modifiers);
extern Boolean site_DoDialog (Str255 server, short label1, short label2);

#endif