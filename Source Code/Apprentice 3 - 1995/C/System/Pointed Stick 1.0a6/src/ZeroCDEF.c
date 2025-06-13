#include "MetaGlobal.h"

typedef pascal long (*CDEFProc)(short varCode, ControlHandle theControl,
									short msg, long param);

typedef struct AuxControlRec {
	struct AuxControlRec**	nextAux;
	ControlHandle			theControl;
	Handle					realCDEF;
} AuxControlRec, *AuxControlRecPtr, **AuxControlRecHdl;

Handle	GetRealCDEF(ControlHandle theControl, StdHeaderHdl us);

Ptr		GetA0(void)
	= { 0x2008 };

pascal long main(short varCode, ControlHandle theControl, short message, long param)
{
	long			result;
	GrafPtr			port;
	short			oldTop;
	Handle			realCDEF;
	StdHeaderHdl	us;
	Handle			origCDEF;

	us = (StdHeaderHdl) RecoverHandle(GetA0());
	realCDEF = GetRealCDEF(theControl, us);
	if (realCDEF != NIL) {
		if (*realCDEF == NIL)  {
			LoadResource(realCDEF);
		}
		if (*realCDEF != NIL) {
		
			GetPort(&port);
			oldTop = port->portRect.top;
			port->portRect.top = 0;
			
			origCDEF = (**theControl).contrlDefProc;
			(**theControl).contrlDefProc = realCDEF;
			HLock(realCDEF);
			result = ((CDEFProc) *realCDEF)(varCode, theControl, message, param);
			HUnlock(realCDEF);
			(**theControl).contrlDefProc = origCDEF;
			
			port->portRect.top = oldTop;
			
			return result;
		}
	}

	SysError(dsCDEFNotFound);
}

Handle	GetRealCDEF(ControlHandle theControl, StdHeaderHdl us)
{
	AuxControlRecHdl	auxRecHandle;
	AuxControlRecPtr	auxRecPtr;
	
	auxRecHandle = (AuxControlRecHdl) (**us).refCon;
	while (auxRecHandle != NIL) {	
		auxRecPtr = *auxRecHandle;
		if ((*auxRecPtr).theControl == theControl)
			return (*auxRecPtr).realCDEF;
		auxRecHandle = (*auxRecPtr).nextAux;
	}
	return NIL;
}
