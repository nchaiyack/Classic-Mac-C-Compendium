Boolean GetSourceFile(FSSpec *sourceFS, ResType theType);
Boolean GetDestFile(FSSpec *destFS, Boolean *deleteTheThing, Str255 theTitle);
pascal OSErr MyMakeFSSpec(short vRefNum, long parID, Str255 fileName,
	FSSpecPtr myFSS);
