OSErr	myOpenFile(FSSpec *theFile, Ptr, Size);
OSErr	mySaveFile(Ptr where, Size howmuch, OSType fType, short prompt, short defName);
OSErr	OpenProcessorState(FSSpecPtr theFile);
OSErr	SaveProcessorState(void);
