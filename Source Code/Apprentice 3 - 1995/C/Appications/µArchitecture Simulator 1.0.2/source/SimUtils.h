void FromStringToNum(ConstStr255Param sourceStr, long *destN, short whichFormat);
void FromNumToString(Str255 destStr, short sourceN, short whichFormat);
pascal long MyGrowZone(Size qNeeded);
void SetupVertScrollBar(WindowPtr wind, ControlHandle scrollB);
void SetupHorzScrollBar(WindowPtr wind, ControlHandle scrollB);
pascal Boolean myStdFilterProc(DialogPtr theD, EventRecord *thEv, short *iHit);
pascal Boolean myStdFilterProcNoCancel(DialogPtr theD, EventRecord *thEv, short *iHit);
void ErrorAlert(OSErr reason);

void ResetRegisters(void);
void ResetMemory(void);
