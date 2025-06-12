/*______________________________________________________________________

	fsu.h - File System Utilities Interface.
	
	Copyright © 1988-1991 Northwestern University.
_____________________________________________________________________*/

#ifndef __fsu__
#define __fsu__

extern OSErr fsu_CheckRFSanity (FSSpecPtr fSpec, Boolean *sane);
extern OSErr fsu_FindFolder (short vRefNum, OSType folderType, 
	Boolean createFolder, short *foundVRefNum, long *foundDirID);
extern OSErr fsu_FSpCreate (FSSpecPtr spec, OSType creator,
	OSType fileType);
extern OSErr fsu_FSpCreateResFile (FSSpecPtr spec, OSType creator, 
	OSType fileType);
extern OSErr fsu_FSpDelete (FSSpecPtr spec);
extern OSErr fsu_FSpGetFInfo (FSSpecPtr spec, FInfo *fndrInfo);
extern OSErr fsu_FSpOpenDF (FSSpecPtr spec, char permission, 
	short *refNum);
extern OSErr fsu_FSpOpenResFile (FSSpecPtr spec, char permission, 
	short *refNum);
extern OSErr fsu_FSpOpenRF (FSSpecPtr spec, char permission, 
	short *refNum);
extern OSErr fsu_FSpRstFLock (FSSpecPtr spec);
extern OSErr fsu_FSpSetFInfo (FSSpecPtr spec, FInfo *fndrInfo);
extern OSErr fsu_FSpSetFLock (FSSpecPtr spec);
extern OSErr fsu_HCheckRFSanity (short vRefNum, long dirID, Str255 fileName, 
	Boolean *sane);
extern OSErr fsu_HCreate (short vRefNum, long dirID, Str255 fileName, 
	OSType creator, OSType fileType);
extern OSErr fsu_HCreateResFile (short vRefNum, long dirID, Str255 fileName, 
	OSType creator, OSType fileType);
extern OSErr fsu_HDelete (short vRefNum, long dirID, Str255 fileName);
extern OSErr fsu_HGetFInfo (short vRefNum, long dirID, Str255 fileName, 
	FInfo *fndrInfo);
extern OSErr fsu_HOpenDF (short vRefNum, long dirID, Str255 fileName, 
	char permission, short *refNum);
extern OSErr fsu_HOpenResFile (short vRefNum, long dirID, Str255 fileName, 
	char permission, short *refNum);
extern OSErr fsu_HOpenRF (short vRefNum, long dirID, Str255 fileName, 
	char permission, short *refNum);
extern OSErr fsu_HRstFLock (short vRefNum, long dirID, Str255 fileName);
extern OSErr fsu_HSetFInfo (short vRefNum, long dirID, Str255 fileName, 
	FInfo *fndrInfo);
extern OSErr fsu_HSetFLock (short vRefNum, long dirID, Str255 fileName);

#endif