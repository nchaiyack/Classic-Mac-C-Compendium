/*______________________________________________________________________

	wstm.h - Window State Module Interface.
	
	Copyright � 1988-1991 Northwestern University.
_____________________________________________________________________*/


#ifndef __wstm__
#define __wstm__

#include "glob.h"

extern void wstm_Save (WindowPtr theWindow, WindState *windState);
extern void wstm_ComputeStd (WindowPtr theWindow);
extern WindowPtr wstm_Restore (Boolean dlog, short windID, Ptr wStorage,
	WindState *windState);

#endif


