Boolean	SelectWorldFile(Str255	theFileName);
OSErr	PathNameFromDirID(short vRefNum, long dirID, Str255 fullPathName);
OSErr	GetFilenameFromPathname(ConstStr255Param pathname, Str255 filename);
void	QuitEverythingButMe(void);