/*______________________________________________________________________

	rep.h - Report Module Interface.
	
	Copyright � 1988, 1989, 1990 Northwestern University.  Permission is granted
	to use this code in your own projects, provided you give credit to both
	John Norstad and Northwestern University in your about box or document.
_____________________________________________________________________*/

#ifndef __rep__
#define __rep__

extern void rep_Init (Rect *repRect, WindowPtr theWindow, 
	short repType, short firstID, 
	short lDefRezID, short cellRezID, Handle *repHandle);
extern void rep_Append (Handle repHandle, Str255 repLine, 
	Boolean drawIt, Boolean scroll);
extern void rep_Fill (Handle repHandle, short id, 
	Boolean scroll);
extern void rep_Scroll (Handle repHandle, Point where);
extern short rep_Click (Handle repHandle, Point where);
extern void rep_Update (Handle repHandle);
extern void rep_Activate (Handle repHandle, Boolean active);
extern OSErr rep_Save (Handle repHandle, Str255 prompt, 
	Str255 defName, OSType creator, Boolean *good, Boolean menuPick);
extern OSErr rep_Key (Handle repHandle, short key, short modifiers);
extern void rep_Jump (Handle repHandle, short lineNum, Boolean doDraw);
extern void rep_Clear (Handle repHandle);
extern Boolean rep_Full (Handle repHandle, long slop);
extern short rep_GetSize (Handle repHandle);
extern void rep_Dispose (Handle repHandle, short cellOption);
extern short rep_Tag (short id, short tag);
extern void rep_Height (Handle repHandle, short height);
extern void rep_GetRect (Handle repHandle, Rect *rect);
extern short rep_GetPos (Handle repHandle);

#endif