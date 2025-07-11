/*______________________________________________________________________

	utl.h - Utilities Interface.
	
	Copyright � 1988-1991 Northwestern University.
_____________________________________________________________________*/

#ifndef __utl__
#define __utl__

/* ASCII codes. */

#define homeKey				0x01		/* ascii code for home key */
#define enterKey				0x03		/* ascii code for enter key */
#define endKey					0x04		/* ascii code for end key */
#define helpKey				0x05		/* ascii code for help key */
#define deleteKey				0x08		/* ascii code for delete key */
#define tabKey					0x09		/* ascii code for tab key */
#define pageUpKey				0x0B		/* ascii code for page up key */
#define pageDownKey			0x0C		/* ascii code for page down key */
#define returnKey				0x0D		/* ascii code for return key */
#define escapeKey				0x1B		/* ascii code for escape key */
#define leftArrow				0x1C		/* ascii code for left arrow key */
#define rightArrow			0x1D		/* ascii code for right arrow key */
#define upArrow				0x1E		/* ascii code for up arrow key */
#define downArrow				0x1F		/* ascii code for down arrow key */

/* System constants. */
													
#define	FSFCBLen				0x3F6		/* address of low-memory global telling
													whether or not we have HFS 
													(see TN 66) */
#define	FCBSPtr				0x34E		/* pointer to file control blocks */	
#define	MenuList				0xA1C		/* handle to menu list */

#define	sfNoPrivs			-3993		/* rsrc id of standard file alert */
#define	sfBadChar			-3994		/* rsrc id of standard file alert */
#define	sfSysErr				-3995		/* rsrc id of standard file alert */
#define	sfReplaceExisting	-3996		/* rsrc id of standard file alert */
#define	sfDiskLocked		-3997		/* rsrc id of standard file alert */

#define	titleBarHeight		19			/* height of std doc window title bar */

#define	fInited				0x0100	/* finder flag set if file inited
													(bit 8 - see TN 40) */
#define	fAlias				0x8000	/* finder flag set if file is an alias */

/* Type definitions. */

typedef void (*utl_ComputeStdStatePtr)(WindowPtr theWindow);

typedef void (*utl_ComputeDefStatePtr)(WindowPtr theWindow,
	Rect *userState);
	
typedef pascal Boolean (*utl_AEIdleProcPtr)(EventRecord *theEventRecord,
	long *sleepTime, RgnHandle *mouseRgn);
	
typedef struct popupPrivateData {		/* see IM VI p. 3-19 */
	MenuHandle			mHandle;
	short					mID;
	char					mPrivate[0];
} popupPrivateData;

/* Function definitions. */

extern short utl_AppendDITL (DialogPtr theDialog, short theDITLID);
extern RgnHandle utl_BuildDialogIBeamRgn (DialogPtr theDialog);
extern void utl_CenterDlogRect (Rect *rect, Boolean centerMain);
extern void utl_CenterRect (Rect *rect);
extern OSErr utl_CheckAERequiredParams (AppleEvent *event);
extern Boolean utl_CheckPack (short packNum, Boolean preload);
extern short utl_CheckSpecialDialogKey (DialogPtr theDialog, char theKey,
	short modifiers, short defaultItem, short cancelItem);
extern void utl_CopyPString (Str255 dest, Str255 source);
extern Boolean utl_CouldDrag (Rect *windRect, short offset);
extern OSErr utl_CountProcesses (short *count);
extern void utl_DILoad (void);
extern OSErr utl_DoDiskInsert (long message, short *vRefNum);
extern void utl_DrawGrowIcon (WindowPtr theWindow);
extern Boolean utl_Ejectable (short vRefNum);
extern void utl_ErrorAlert (Str255 tmpl, short dlogID, 
	ModalFilterProcPtr filterProc, Str255 p0, Str255 p1,
	Str255 p2, Str255 p3);
extern void utl_ErrorAlertRez (short rezID, short index, short dlogID,
	ModalFilterProcPtr filterProc, 
	Str255 p0, Str255 p1, Str255 p2, Str255 p3);
extern void utl_FixStdFile (void);
extern void utl_FlashButton (ControlHandle theButton);
extern void utl_FlashDialogButton (DialogPtr theDialog, short itemNo);
extern pascal void utl_FrameItem (WindowPtr theWindow, short itemNo);
extern Boolean utl_GestaltFlag (OSType selector, short bitNum);
extern short utl_GetApplVol (void);
extern short utl_GetBlessedWDRefNum (void);
extern short utl_GetDialogControlValue (DialogPtr theDialog, short itemNo);
extern short utl_GetDialogEditFieldNum (DialogPtr theDialog);
extern long utl_GetDialogNumericValue (DialogPtr theDialog, short itemNo);
extern void utl_GetDialogPopupValue (DialogPtr theDialog, short itemNo, 
	Str255 val);
extern void utl_GetDialogText (DialogPtr theDialog, short itemNo, Str255 theText);
extern Boolean utl_GetFontNumber (Str255 fontName, short *fontNum);
extern long utl_GetLongSleep (void);
extern short utl_GetMBarHeight (void);
extern ControlHandle utl_GetNewControl (short controlID, 
	WindowPtr theWindow);
extern DialogPtr utl_GetNewDialog (short dialogID, Ptr dStorage, 
	WindowPtr behind);
extern WindowPtr utl_GetNewWindow (short windowID, Ptr wStorage, 
	WindowPtr behind);
extern long utl_GetSysDirID (void);
extern short utl_GetSysVol (void);
extern short utl_GetSysWD (void);
extern void utl_GetWindGD (WindowPtr theWindow, GDHandle *gd, 
	Rect *screenRect, Rect *windRect, Boolean *hasMB);
extern long utl_GetVolFilCnt (short volRefNum);
extern Boolean utl_HaveColor (void);
extern Boolean utl_HaveSound (void);
extern char utl_HGetState (Handle h);
extern void utl_HSetState (Handle h, char flags);
extern void utl_InitSpinCursor (CursHandle cursArray[], short numCurs, 
	short tickInterval);
extern Boolean utl_InteractionPermitted (void);
extern void utl_InvalGrow (WindowPtr theWindow);
extern Boolean utl_IsDAWindow (WindowPtr theWindow);
extern Boolean utl_IsLaser (THPrint hPrint);
extern OSErr utl_KillAllProcesses (utl_AEIdleProcPtr idleProc, Boolean *allKilled);
extern OSErr utl_KillOneProcess (utl_AEIdleProcPtr idleProc,
	ProcessSerialNumber *psn);
extern OSErr utl_LocateFinder (Boolean *finderFound, FSSpec *finderFSpec);
extern void utl_LockControls (WindowPtr theWindow);
extern void utl_ModalDialog (short dlogID, ModalFilterProcPtr filterProc, 
	short defaultItem, short cancelItem, 
	short *itemHit, DialogPtr *theDialog, ...);
extern void utl_ModalDialogContinue (ModalFilterProcPtr filterProc, 
	short defaultItem, short cancelItem, short *itemHit);
extern pascal void utl_OutlineDefaultButton (WindowPtr theWindow, short itemNo);
extern void utl_PlotSmallIcon (Rect *theRect, Handle theHandle);
extern void utl_PlugParams (Str255 line1, Str255 line2, Str255 p0, 
	Str255 p1, Str255 p2, Str255 p3);
extern void utl_RedrawMenuBar (Boolean mBarState[], short nMenus);
extern OSErr utl_Restart (void);
extern void utl_RestoreWindowPos (WindowPtr theWindow, Rect *userState, 
	Boolean zoomed, short offset,
	utl_ComputeStdStatePtr computeStdState,
	utl_ComputeDefStatePtr computeDefState);
extern OSErr utl_ReviveFinder (FSSpec *fSpec);
extern Boolean utl_Rom64 (void);
extern short utl_SaveChangesAlert (Str255 tmpl, short dlogID, 
	ModalFilterProcPtr filterProc, Str255 p0, Str255 p1,
	Str255 p2, Str255 p3);
extern short utl_SaveChangesAlertRez (short rezID, short index, short dlogID,
	ModalFilterProcPtr filterProc, 
	Str255 p0, Str255 p1, Str255 p2, Str255 p3);
extern void utl_SaveWindowPos (WindowPtr theWindow, Rect *userState, 
	Boolean *zoomed);
extern short utl_ScaleFontSize (short theFontNum, short theFontSize, 
	short percent, Boolean laser);
extern void utl_SetDialogControlValue (DialogPtr theDialog, short itemNo, 
	short val);
extern void utl_SetDialogNumericText (DialogPtr theDialog, short itemNo, 
	short theNum);
extern void utl_SetDialogPopupValue (DialogPtr theDialog, short itemNo,
	Str255 val, short defaultVal);
extern void utl_SetDialogText (DialogPtr theDialog, short itemNo, 
	Str255 theText);
extern void utl_SetDialogUserItem (DialogPtr theDialog, short itemNo, 
	Handle userItem);
extern void utl_SpinCursor (void);
extern void utl_StaggerWindow (Rect *windRect, short initialOffset, short offset, 
	Point *pos);
extern Boolean utl_StandardAsciiChar (char c);
extern short utl_StopAlert (short alertID, 
	ModalFilterProcPtr filterProc, short cancelItem);
extern void utl_SwitchDialogRadioButton (DialogPtr theDialog, short itemNo,
	short first, short last);
extern Boolean utl_SysHasNotMgr (void);
extern Boolean utl_SysHasPopUp (void);
extern short utl_TellMeTwiceAlert (Str255 tmpl, short dlogID, 
	ModalFilterProcPtr filterProc, Str255 p0, Str255 p1,
	Str255 p2, Str255 p3);
extern short utl_TellMeTwiceAlertRez (short rezID, short index, short dlogID,
	ModalFilterProcPtr filterProc, 
	Str255 p0, Str255 p1, Str255 p2, Str255 p3);
extern short utl_ToggleDialogCheckBox (DialogPtr theDialog, short itemNo);
extern Boolean utl_VolIsMFS (short vRefNum);
extern Boolean utl_WaitNextEvent (short eventMask, EventRecord *theEvent,
	long sleep, RgnHandle mouseRgn);

#endif