/*______________________________________________________________________

	edit.h - Edit Window Manager Interface.
_____________________________________________________________________*/

#ifndef __edit__
#define __edit__

/*_____________________________________________________________________

	Defines.
_____________________________________________________________________*/

typedef enum EditLoginStatus {
	editNotLoggedIn,
	editLoggedIn,
	editHero
} EditLoginStatus;

/*_____________________________________________________________________

	Functions.
_____________________________________________________________________*/

extern void edit_DoPeriodic (WindowPtr w);
extern void edit_DoClick (WindowPtr w, Point where, short modifiers);
extern void edit_DoKey (WindowPtr w, char key, short modifiers);
extern void edit_DoUpdate (WindowPtr w);
extern void edit_DoActivate (WindowPtr w);
extern void edit_DoDeactivate (WindowPtr w);
extern void edit_DoGrow (WindowPtr w, Point where);
extern void edit_DoZoom (WindowPtr w, Point where, short partCode);
extern void edit_DoClose (WindowPtr w);
extern Boolean edit_DoCommand (WindowPtr top, short theMenu, short theItem);
extern void edit_Init (void);
extern EditLoginStatus edit_GetStatus (Str255 server, Str255 alias);
extern Boolean edit_IsDirty (WindowPtr w);
extern Boolean edit_Terminate (Boolean interactionPermitted);

#endif