#ifndef __MY_FILE_UTILITIES_H__
#define __MY_FILE_UTILITIES_H__

#ifdef __cplusplus
extern "C" {
#endif

extern	OSErr GetApplicationFSSpec(FSSpec *theFS);
extern	OSErr GetApplicationVRefNum(short *vRefNum);
extern	OSErr GetApplicationParID(long *parID);
extern	OSType GetFileType(FSSpec *myFSS);
extern	OSType GetFileCreator(FSSpec *myFSS);
extern	unsigned long GetModificationDate(FSSpec *myFSS);
extern	OSErr SetModificationDate(FSSpec *myFSS, unsigned long theDate);
extern	OSErr TouchFolder(FSSpec *theFS);
extern	OSErr FileExists(FSSpec *theFS);
extern	void GetVersionString(Str255 theVersion);
extern	pascal OSErr FSpLocationFromFullPath(short fullPathLength, const void *fullPath, FSSpec *spec);

#ifdef __cplusplus
}
#endif

#endif
