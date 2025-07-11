/*______________________________________________________________________

	help.h - Help Window Manager Interface.
_____________________________________________________________________*/

#ifndef __help__
#define __help__

/*_____________________________________________________________________

	Functions.
_____________________________________________________________________*/

extern void help_DoPeriodic (WindowPtr w);
extern void help_DoClick (WindowPtr w, Point where, short modifiers);
extern void help_DoKey (WindowPtr w, char key, short modifiers);
extern void help_DoUpdate (WindowPtr w);
extern void help_DoActivate (WindowPtr w);
extern void help_DoDeactivate (WindowPtr w);
extern void help_DoGrow (WindowPtr w, Point where);
extern void help_DoZoom (WindowPtr w, Point where, short partCode);
extern void help_DoClose (WindowPtr w);
extern Boolean help_DoCommand (WindowPtr top, short theMenu, short theItem);
extern void help_Init (void);
extern void help_Terminate (void);
extern void help_BuildHelpMenu (void);

#endif