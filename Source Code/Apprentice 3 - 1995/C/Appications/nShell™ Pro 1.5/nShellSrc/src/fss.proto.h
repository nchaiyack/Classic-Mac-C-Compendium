/* ==========================================

	fss.proto.h
	
	Copyright (c) 1994,1995 Newport Software Development
	
   ========================================== */

OSErr fss_DirID(short vRefNum, StringPtr name, long *theDirID, Boolean *isDirectory);
OSErr fss_DirName(short vRefNum, long dirID, StringPtr name);
OSErr fss_FilenameFromPathname(const Str255 pathname, Str255 filename);
OSErr fss_make( Str255 pathname, FSSpec *spec );
OSErr fss_make_6( StringPtr pathname, FSSpec *spec);
OSErr fss_ParID(short vRefNum, StringPtr name, long *parID);
OSErr fss_VRefNum(StringPtr pathname, short *realVRefNum);
