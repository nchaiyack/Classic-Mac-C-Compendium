#include "program globals.h"

void LoadSaveDispatch(Boolean isLoad, Boolean useOldFile, FSSpec *useThisFS);
enum ErrorTypes SaveTheFile(FSSpec *saveFile, Boolean alreadyExists);
enum ErrorTypes GetTheFile(FSSpec *saveFile);
