/*______________________________________________________________________

	pswd.h - Change Password Movable Modal Dialog Interface
_____________________________________________________________________*/

#ifndef __pswd__
#define __pswd__

/*_____________________________________________________________________

	Functions.
_____________________________________________________________________*/

extern void pswd_DoPeriodic (WindowPtr w);
extern void pswd_DoKey (WindowPtr w, char key, short modifiers);
extern Boolean pswd_DoCommand (WindowPtr w, short theMenu, short theItem);
extern Boolean pswd_DoDialog (Str255 pswd1, Str255 pswd2);

#endif