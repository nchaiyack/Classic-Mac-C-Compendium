/*______________________________________________________________________

				utl.h - Utilities Interface.
				
				Copyright � 1988, 1989, 1990 Northwestern University.  Permission is granted
				to use this code in your own projects, provided you give credit to both
				John Norstad and Northwestern University in your about box or document.
_____________________________________________________________________*/


#ifndef __utl__
#define __utl__


/* ASCII codes. */

#define homeKey 												0x01						/* ascii code for home key */
#define enterKey																0x03						/* ascii code for enter key */
#define endKey																	0x04						/* ascii code for end key */
#define helpKey 												0x05						/* ascii code for help key */
#define deleteKey 															0x08						/* ascii code for delete key */
#define tabKey																	0x09						/* ascii code for tab key */
#define pageUpKey 															0x0B						/* ascii code for page up key */
#define pageDownKey 										0x0C						/* ascii code for page down key */
#define returnKey 															0x0D						/* ascii code for return key */
#define escapeKey 															0x1B						/* ascii code for escape key */
#define leftArrow 															0x1C						/* ascii code for left arrow key */
#define rightArrow											0x1D						/* ascii code for right arrow key */
#define upArrow 												0x1E						/* ascii code for up arrow key */
#define downArrow 															0x1F						/* ascii code for down arrow key */


/* System constants. */
																																																				
#define FSFCBLen																0x3F6 					/* address of low-memory global telling
																																																				whether or not we have HFS
																																																				(see TN 66) */
#define FCBSPtr 												0x34E 					/* pointer to file control blocks */		
#define MenuList																0xA1C 					/* handle to menu list */

#define sfNoPrivs 											-3993 					/* rsrc id of standard file alert */
#define sfBadChar 											-3994 					/* rsrc id of standard file alert */
#define sfSysErr																-3995 					/* rsrc id of standard file alert */
#define sfReplaceExisting 			-3996 					/* rsrc id of standard file alert */
#define sfDiskLocked						-3997 					/* rsrc id of standard file alert */

#define titleBarHeight					19											/* height of std doc window title bar */

#define fInited 												0x100 					/* finder flag set if file inited
																																																				(bit 8 - see TN 40) */


/* Type definitions. */

typedef void (*utl_ComputeStdStatePtr)(WindowPtr theWindow);

typedef void (*utl_ComputeDefStatePtr)(WindowPtr theWindow,
				Rect *userState);
				

/* Function definitions. */

extern short utl_AppendDITL (DialogPtr theDialog, short theDITLID);
extern void utl_CenterDlogRect (Rect *rect, Boolean centerMain);
extern void utl_CenterRect (Rect *rect);
extern Boolean utl_CheckPack (short packNum, Boolean preload);
extern void utl_CopyPString (Str255 dest, Str255 source);
extern Boolean utl_CouldDrag (Rect *windRect, short offset);
extern void utl_DILoad (void);
extern OSErr utl_DoDiskInsert (long message, short *vRefNum);
extern void utl_DrawGrowIcon (WindowPtr theWindow);
extern Boolean utl_Ejectable (short vRefNum);
extern void utl_FixStdFile (void);
extern void utl_FlashButton (DialogPtr theDialog, short itemNo);
extern pascal void utl_FrameItem (WindowPtr theWindow, short itemNo);
extern short utl_GetApplVol (void);
extern short utl_GetBlessedWDRefNum (void);
extern Boolean utl_GetFontNumber (Str255 fontName, short *fontNum);
extern long utl_GetLongSleep (void);
extern short utl_GetMBarHeight (void);
extern ControlHandle utl_GetNewControl (short controlID,
				WindowPtr theWindow);
extern DialogPtr utl_GetNewDialog (short dialogID, Ptr dStorage,
				WindowPtr behind);
extern WindowPtr utl_GetNewWindow (short windowID, Ptr wStorage,
				WindowPtr behind);
extern short utl_GetSysVol (void);
extern short utl_GetSysWD (void);
extern void utl_GetWindGD (WindowPtr theWindow, GDHandle *gd, 
				Rect *screenRect, Rect *windRect, Boolean *hasMB);
extern long utl_GetVolFilCnt (short volRefNum);
extern Boolean utl_HaveColor (void);
extern Boolean utl_HaveSound (void);
extern void utl_InitSpinCursor (CursHandle cursArray[], short numCurs,
				short tickInterval);
extern void utl_InvalGrow (WindowPtr theWindow);
extern Boolean utl_IsDAWindow (WindowPtr theWindow);
extern Boolean utl_IsLaser (THPrint hPrint);
extern void utl_LockControls (WindowPtr theWindow);
extern void utl_PlotSmallIcon (Rect *theRect, Handle theHandle);
extern void utl_PlugParams (Str255 line1, Str255 line2, Str255 p0,
				Str255 p1, Str255 p2, Str255 p3);
extern void utl_RestoreWindowPos (WindowPtr theWindow, Rect *userState, 
				Boolean zoomed, short offset,
				utl_ComputeStdStatePtr computeStdState,
				utl_ComputeDefStatePtr computeDefState);
extern Boolean utl_Rom64 (void);
extern OSErr utl_RFSanity (Str255 fName, Boolean *sane);
extern void utl_SaveWindowPos (WindowPtr theWindow, Rect *userState,
				Boolean *zoomed);
extern short utl_ScaleFontSize (short fontNum, short fontSize,
				short percent, Boolean laser);
extern void utl_SpinCursor (void);
extern void utl_GetIndGD (short n,GDHandle *gd, Rect *screenRect, Boolean *hasMB);
extern void utl_StaggerWindow (Rect *windRect, short initialOffset, short offset, 
				Point *pos,short specificDevice);
extern short utl_StopAlert (short alertID,
				ModalFilterProcPtr filterProc, short cancelItem);
extern Boolean utl_SysHasNotMgr (void);
extern Boolean utl_SysHasPopUp (void);
extern Boolean utl_VolIsMFS (short vRefNum);
extern Boolean utl_WaitNextEvent (short eventMask, EventRecord *theEvent,
				long sleep, RgnHandle mouseRgn);

#endif
