/*______________________________________________________________________

	login.h - Login Movable Modal Dialog Interface
_____________________________________________________________________*/

#ifndef __login__
#define __login__

/*_____________________________________________________________________

	Functions.
_____________________________________________________________________*/

extern void login_DoPeriodic (WindowPtr w);
extern void login_DoKey (WindowPtr w, char key, short modifiers);
extern Boolean login_DoCommand (WindowPtr w, short theMenu, short theItem);
extern Boolean login_DoDialog (Str255 server, Str255 user, Str255 pswd);

#endif