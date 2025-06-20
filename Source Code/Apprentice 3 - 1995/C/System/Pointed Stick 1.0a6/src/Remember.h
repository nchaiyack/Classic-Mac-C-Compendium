#ifndef __REMEMBER__
#define __REMEMBER__

#include "List.h"

#define kSortByName				0x00
#define kSortByAge				0x01
#define kSortByMask				0x07
#define kSeparatePermObjects	0x08

extern DynamicArrayHdl	gFileList;
extern DynamicArrayHdl	gDirectoryList;
extern DynamicArrayHdl	gVolumeList;
extern DynamicArrayHdl	gReboundList;
extern short			gSortType;
extern short			gMaxFileObjectsToRemember;

typedef struct {
	short			volIndex;
	long			parID;
	union {
		OSType		type;
		long		dirID;
	} u;
	short			finderFlags;
	Boolean			permanent;
	Boolean			openableByCurrentApp;
	unsigned long	timeAdded;
	StringHandle	menuText;
	StringHandle	fName;
} FileObject, *FileObjectPtr, **FileObjectHdl;


typedef struct {
	short			vRefNum;
	unsigned long	creationDate;
	char			name[2];		// Leave no doubt about struct size
} VolumeEntry, *VolumeEntryPtr, **VolumeEntryHdl;


extern UniversalProcPtr	gOldMountVol;
extern UniversalProcPtr	gOldUnmountVol;

pascal void				MyMountVol(void);
pascal void				MyUnmountVol(void);

void					RememberFileAndParent(FSSpecPtr);
void					RememberParentDirectory(FSSpecPtr);
void					RememberSelectionForRebound(void);
void					RememberFileObject(FSSpecPtr filePtr, DynamicArrayHdl list);
short					RememberVolume(short vRefNum);
FileObjectHdl			GetReboundItem(short vRefNum, long parID);
short					GetVolRefNum(FileObjectHdl);
short					AddVolume(Str255 name, unsigned long crDate);
Boolean					FileHandleExists(FileObjectHdl);
Boolean					FileSpecExists(FSSpecPtr);
void					DeleteFileObject(DynamicArrayHdl array, FileObjectHdl record);
void					CullOldest(DynamicArrayHdl array);
void					SetSortProcedure(short sortType);

#endif