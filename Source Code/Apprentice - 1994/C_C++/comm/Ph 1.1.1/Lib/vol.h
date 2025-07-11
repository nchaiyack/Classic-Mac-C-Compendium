/*______________________________________________________________________

	vol.h - Volume Selection Module Interface.
	
	Copyright � 1988-1991 Northwestern University.
_____________________________________________________________________*/

#ifndef __vol__
#define __vol__

extern OSErr vol_Init (ControlHandle driveH, ControlHandle ejectH, 
	Rect *nameRect, Rect *icnRect, Handle floppyH, 
	Handle hardH, Boolean applSel, Boolean unmountFlag);
extern OSErr vol_DoDrive(void);
extern OSErr vol_DoPopUp (Point where, short menuID);
extern OSErr vol_DoEject(void);
extern OSErr vol_DoInsert (long message);
extern OSErr vol_DoUpdate(void);
extern OSErr vol_Verify(void);
extern OSErr vol_GetSel (short *vRefNum);
extern OSErr vol_SetSel (short vRefNum, Boolean doButtons);
extern OSErr vol_GetName(Str255 vName);

#endif