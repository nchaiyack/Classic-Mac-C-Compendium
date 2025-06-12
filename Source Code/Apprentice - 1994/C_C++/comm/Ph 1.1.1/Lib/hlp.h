/*______________________________________________________________________

	hlp.h - Help Module Interface.
	
	Copyright © 1988-1991 Northwestern University.
_____________________________________________________________________*/

#ifndef __hlp__
#define __hlp__

#ifndef __rpp__
#include "rpp.h"
#endif

typedef void (*hlp_ExtraUpdate)(void);

typedef struct hlp_PBlock {
	short			firstStrID;
	short			listDefID;
	Rect			textRect;
	short			fontNum;
	short			fontSize;
	short			tabConID;
	Rect			tabConRect;
	short			tabConFNum;
	short			tabConFSize;
	short			tabConLSep;
	short			tag;
	short			tagRezID;
	short			scrollLine;
	short			cellRezID;
	short			cellOption;
	hlp_ExtraUpdate	extraUpdate;
} hlp_PBlock;

extern void hlp_Open (WindowPtr theWindow, hlp_PBlock *p);
extern void hlp_Close (void);
extern void hlp_Click (Point where);
extern void hlp_Activate(void);
extern void hlp_Deactivate(void);
extern void hlp_Update (void);
extern void hlp_Key (short key, short modifiers);
extern void hlp_Jump (short tag);
extern OSErr hlp_Print (rpp_PrtBlock *p, Boolean printOne);
extern void hlp_Grow (short height, short width);
extern void hlp_Zoom (void);
extern OSErr hlp_Save (Str255 prompt, Str255 defName, OSType creator);
extern short hlp_GetScrollPos (void);
extern void hlp_GetTconRect (Rect *tconRect);

#endif