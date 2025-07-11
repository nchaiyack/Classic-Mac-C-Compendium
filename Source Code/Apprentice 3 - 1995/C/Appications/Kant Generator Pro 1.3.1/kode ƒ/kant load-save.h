#ifndef __MY_LOAD_SAVE_H__
#define __MY_LOAD_SAVE_H__

#include "program globals.h"

#ifdef __cplusplus
extern "C" {
#endif

extern	ErrorTypes LoadSaveDispatch(FSSpec *fs, Boolean isLoad, Boolean useOldFile);
extern	ErrorTypes SaveTheFile(FSSpec saveFile, Boolean alreadyExists);
extern	ErrorTypes GetTheFile(FSSpec *saveFile);

#ifdef __cplusplus
}
#endif

#endif
