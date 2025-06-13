#ifndef __MENUSETUP__
#define __MENUSETUP__

#include "Remember.h"

extern short		gAppendVolumeName;
extern short		gAppendParentDirectoryName;

MenuHandle			CreateDirectoryMenu(void);
MenuHandle			CreateFileMenu(void);
void				AddDirectories(MenuHandle menu);
void				AddFiles(MenuHandle menu);
void				FilterDirectories(void);
void				FilterFilesByFREF(void);
void				FilterFilesByHookAndList(void);
void				CreateMenuText(FileObjectHdl entry, StringPtr name);
short				GetFreeHMenuID(void);
FileObjectHdl		GetNthOpenableFileObject(DynamicArrayHdl array, short index);

#endif