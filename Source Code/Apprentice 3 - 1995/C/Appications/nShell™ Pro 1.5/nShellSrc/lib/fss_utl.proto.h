/* ==========================================

	fss_utl.proto.h
	
	Copyright (c) 1993,1994,1995 Newport Software Development
	
   ========================================== */

OSErr fss_GetFInfo(int gotFSSpec, const FSSpec *spec, FInfo *fndrInfo);
OSErr fss_OpenDF(int gotFSSpec, const FSSpec *spec, char permission, short *refNum);
OSErr fss_OpenRF(int gotFSSpec, const FSSpec *spec, char permission, short *refNum);
OSErr fss_SetFInfo(int gotFSSpec, const FSSpec *spec, const FInfo *fndrInfo);
int   fss_test( void );
OSErr fss_to_DirID(const FSSpec *spec, long *theDirID, Boolean *isDirectory);
OSErr fss_wake_parent(const FSSpec *spec);