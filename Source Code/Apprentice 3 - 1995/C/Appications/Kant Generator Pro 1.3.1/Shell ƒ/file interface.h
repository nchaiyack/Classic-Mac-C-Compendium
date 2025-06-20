#ifndef __MY_FILE_INTERFACE_H__
#define __MY_FILE_INTERFACE_H__

#ifdef __cplusplus
extern "C" {
#endif

extern	Boolean GetSourceFile(FSSpec *sourceFS, ResType theType);
extern	Boolean GetDestFile(FSSpec *destFS, Boolean *deleteTheThing, Str255 theTitle);
extern	pascal OSErr MyMakeFSSpec(short vRefNum, long parID, Str255 fileName,
	FSSpecPtr myFSS);

#ifdef __cplusplus
}
#endif

#endif
