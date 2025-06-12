/*______________________________________________________________________

	query.h - Query Window Manager Interface.
_____________________________________________________________________*/

#ifndef __query__
#define __query__

/*_____________________________________________________________________

	Functions.
_____________________________________________________________________*/

extern void query_DoPeriodic (WindowPtr w);
extern void query_DoClick (WindowPtr w, Point where, short modifiers);
extern void query_DoKey (WindowPtr w, char key, short modifiers);
extern void query_DoUpdate (WindowPtr w);
extern void query_DoActivate (WindowPtr w);
extern void query_DoDeactivate (WindowPtr w);
extern void query_DoGrow (WindowPtr w, Point where);
extern void query_DoZoom (WindowPtr w, Point where, short partCode);
extern void query_DoClose (WindowPtr w);
extern Boolean query_DoCommand (WindowPtr top, short theMenu, short theItem);
extern void query_Init (void);
extern Boolean query_CanEditField (WindowPtr w);
extern void query_NewSiteList (void);
extern void query_NewDefaultSite (void);
extern void query_Terminate (void);

#endif