/*______________________________________________________________________

	oop.h - Reusable Object-Oriented Window and Command Manager Interface.
_____________________________________________________________________*/

#ifndef __oop__
#define __oop__

/*_____________________________________________________________________

	Defines.
_____________________________________________________________________*/
														
#define noWind						0			/* no window */
#define daWind						1			/* da window */
#define firstUserWindKind		2			/* first user window kind value */

/*_____________________________________________________________________

	Types.
_____________________________________________________________________*/

typedef void (*FPeriodic)(WindowPtr w);	
typedef void (*FClick)(WindowPtr w, Point where, short modifiers);		
typedef void (*FKey)(WindowPtr w, char key, short modifiers);			
typedef void (*FUpdate)(WindowPtr w);		
typedef void (*FActivate)(WindowPtr w);	
typedef void (*FDeactivate)(WindowPtr w);	
typedef void (*FGrow)(WindowPtr w, Point where);
typedef void (*FZoom)(WindowPtr w, Point where, short partCode);
typedef void (*FClose)(WindowPtr w);
typedef Boolean (*FCommand)(WindowPtr w, short theMenu, short theItem);
typedef void (*FDisk)(long message);

typedef struct oop_Dispatch {
	FPeriodic	doPeriodic;		/* pointer to periodic handler */
	FClick		doClick;			/* pointer to mouse down event handler */
	FKey			doKey;			/* pointer to key and autokey event handler */
	FUpdate		doUpdate;		/* pointer to update event handler */
	FActivate	doActivate;		/* pointer to activate event handler */
	FDeactivate	doDeactivate;	/* pointer to deactivate event handler */
	FGrow			doGrow;			/* pointer to window grow handler */
	FZoom			doZoom;			/* pointer to window zoom handler */
	FClose		doClose;			/* pointer to window close handler */
	FCommand		doCommand;		/* pointer to command handler */
} oop_Dispatch;

/*______________________________________________________________________

	Variables.
_____________________________________________________________________*/

extern EventRecord		Event;			/* the current event */

/*______________________________________________________________________

	Functions.
_____________________________________________________________________*/

extern short oop_GetWindKind (WindowPtr w);
extern Handle oop_GetWindInfo (WindowPtr w);
extern short oop_GetWindItemHit (WindowPtr w);
extern void oop_ClearWindItemHit (WindowPtr w);
extern RgnHandle oop_GetWindIbeamRgn (WindowPtr w);
extern Boolean oop_Moved (WindowPtr w);
extern Boolean oop_InForeground (void);
extern void oop_Init (short appleMenuID, short lastAppleMenuItem, 
	short fileMenuID, short closeMenuItem,
	short editMenuID, short selectAllItem, 
	FPeriodic doPeriodic, FDisk doDisk,
	long extraMasterPtrBlocks, long extraStackSpace);
extern void oop_RegisterCommandHandler (FCommand handler);
extern void oop_NewWindow (WindowPtr w, short kind,
	Handle info, oop_Dispatch *dispatch);
extern void oop_NewDialog (WindowPtr d, short kind,
	Handle info, oop_Dispatch *dispatch,
	Boolean modal, short defaultBtn, short cancelBtn);
extern void oop_DoPeriodic (WindowPtr w);
extern void oop_DoClick (WindowPtr w, Point where, short modifiers);
extern void oop_DoKey (WindowPtr w, char key, short modifiers);
extern void oop_DoUpdate (WindowPtr w);
extern void oop_DoActivate (WindowPtr w);
extern void oop_DoDeactivate (WindowPtr w);
extern void oop_DoGrow (WindowPtr w, Point where);
extern void oop_DoZoom (WindowPtr w, Point where, short partCode);
extern void oop_DoClose (WindowPtr w);
extern Boolean oop_DoCommand (WindowPtr w, short theMenu, short theItem);
extern void oop_DoEvent (EventRecord *event, short eventMask, long sleep, 
	RgnHandle mouseRgn);
extern void oop_UpdateAll (void);
extern pascal Boolean oop_ModalUpdate (DialogPtr theDialog,
	EventRecord *theEvent, short *itemHit);

#endif