/*______________________________________________________________________

	wstm.h - Window State Module Interface.
	
	Copyright � 1988, 1989, 1990, Northwestern University.
_____________________________________________________________________*/


#ifndef __wstm__
#define __wstm__

#ifndef __glob__
#include "glob.h"
#endif

extern void wstm_Init (WindState *windState);
extern void wstm_Save (WindowPtr theWindow, WindState *windState);
extern void wstm_ComputeStd (WindowPtr theWindow);
extern void wstm_ComputeDef (WindowPtr theWindow, Rect *userState);
extern WindowPtr wstm_Restore (Boolean dlog, short windID, Ptr wStorage,
	WindState *windState);
extern void wstm_Mark (WindState *windState);

#endif


