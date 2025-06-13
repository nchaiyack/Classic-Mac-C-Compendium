#ifndef __MY_GENERIC_OPEN_H__
#define __MY_GENERIC_OPEN_H__

#ifdef __cplusplus
extern "C" {
#endif

extern	void OpenTheFile(FSSpec *myFSS);
extern	void PrintTheFile(FSSpec *myFSS);
extern	OSType GetFileType(FSSpec *myFSS);

#ifdef __cplusplus
}
#endif

#endif
