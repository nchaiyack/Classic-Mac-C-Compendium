/*______________________________________________________________________

	tran.h - Server Transaction Movable Modal Dialog Interface
_____________________________________________________________________*/

#ifndef __tran__
#define __tran__

/*_____________________________________________________________________

	Functions.
_____________________________________________________________________*/

extern void tran_DoPeriodic (WindowPtr w);
extern void tran_BeginTransaction (short msgIndex, Str255 p0);
extern void tran_EndTransaction (void);

#endif