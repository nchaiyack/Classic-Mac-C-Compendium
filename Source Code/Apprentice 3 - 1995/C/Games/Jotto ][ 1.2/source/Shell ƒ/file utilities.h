OSErr GetApplicationFSSpec(FSSpec *theFS);
OSErr GetApplicationVRefNum(short *vRefNum);
OSErr GetApplicationParID(long *parID);
OSType GetFileType(FSSpec *myFSS);
unsigned long GetModificationDate(FSSpec *myFSS);
OSErr SetModificationDate(FSSpec *myFSS, unsigned long theDate);
