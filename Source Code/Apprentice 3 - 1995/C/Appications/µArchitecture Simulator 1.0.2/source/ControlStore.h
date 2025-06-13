extern short	myFileRefN;
extern Boolean	DocIsOpen;

OSErr RevertFile(void);
OSErr myOpenCSFile(FSSpec *theFile, ScriptCode, Boolean StationeryDoc);
OSErr mySaveCSFile(short selector);
OSErr WriteMicroprogramData(Handle wrBuf, FSSpec *theFSpec, OSType, short theOpenedFile, ScriptCode);
void DoNew(void);
void ResetMicroprogramWindow(void);
Handle PrepareBufferFromList(void);
void DocumentIsDirty(Boolean dirtiness);
Boolean ReadyToTerminate(void);
