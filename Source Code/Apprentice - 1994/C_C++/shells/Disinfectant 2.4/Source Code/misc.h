/*______________________________________________________________________

	misc.h - Miscellany Module Interface.
	
	Copyright � 1988, 1989, 1990, Northwestern University.
_____________________________________________________________________*/

#ifndef __misc__
#define __misc__

#ifndef __rpp__
#include "rpp.h"
#endif

#ifndef __glob__
#include "glob.h"
#endif

extern void misc_HiliteScan(void);
extern void misc_ClearReport (void);
extern void misc_CheckEject (short refNum);
extern void misc_ValPrint (rpp_PrtBlock *p, Boolean report);
extern void misc_ReadPref (void);
extern void misc_WritePref (void);
extern WindKind misc_GetWindKind (WindowPtr theWindow);
extern void misc_Notify (short msg, Boolean wait);
extern void misc_Update (void);
extern void misc_PrintError (OSErr rCode);
extern void misc_SetCursor (void);

#endif