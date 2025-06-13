/*******************************************************************************
 * Paths.c
 *
 * See header file for more information.
 *
 *******************************************************************************/

#include "Paths.h"
#include <Folders.h>


// local prototypes

short Find_File1(CInfoPBRec *fInfo, Str32 findFile, long searchDirId, short searchIndex,
				short searchDepth, long *foundDirId, short *foundIndex, Boolean findDirs,
				Boolean findFiles, Boolean *foundDirFlag, Boolean *foundFileFlag);
pascal short SFGetDirHook(short item, DialogPtr theDialog);



/*******************************************************************************
 * Get_Root_Vol_Info
 *
 *******************************************************************************/

short Get_Root_Vol_Info(short *rootVolNum, Str32 rootVolName)
{
	ParamBlockRec	vInfo;
	OSErr			err;
	
	rootVolName[0] = '\0';
	vInfo.volumeParam.ioCompletion = NULL;
	vInfo.volumeParam.ioVolIndex = 0;
	vInfo.volumeParam.ioNamePtr = rootVolName;
	vInfo.volumeParam.ioVRefNum = 0;
	err = PBGetVInfo(&vInfo, FALSE);
	if (rootVolNum)
		*rootVolNum = vInfo.volumeParam.ioVRefNum;
	
	return err;
} // Get_Root_Vol_Info



/*******************************************************************************
 * Prepend_Dir_To_Path
 *
 *******************************************************************************/

void Prepend_Dir_To_Path(Str32 dirName, Str255 pathName, Str255 fullDirPath)
{
	short	hasPath = (pathName[0] != '\0');
	
	BlockMove((Ptr)(pathName + 1), (Ptr)(fullDirPath + 2 + dirName[0]), pathName[0]);
	BlockMove((Ptr)(dirName + 1), (Ptr)(fullDirPath + 1), dirName[0]);
	fullDirPath[dirName[0] + 1] = ':';
	fullDirPath[0] = dirName[0] + pathName[0] + hasPath;
} // Prepend_Dir_To_Path



/*******************************************************************************
 * Append_File_To_Path
 *
 *******************************************************************************/

void Append_File_To_Path(Str32 fileName, Str255 pathName, Str255 fullFilePath)
{
	short	noColon = (pathName[pathName[0]] != ':');
	
	if (pathName[0] == '\0')
		BlockMove((Ptr)fileName, (Ptr)fullFilePath, fileName[0] + 1);
	else
	{
		if (pathName != fullFilePath)
			BlockMove((Ptr)pathName, (Ptr)fullFilePath, pathName[0] + 1);
		BlockMove((Ptr)(fileName + 1), (Ptr)(fullFilePath + pathName[0] + 1 + noColon), fileName[0]);
		if (noColon)
			fullFilePath[pathName[0] + 1] = ':';
		fullFilePath[0] = pathName[0] + fileName[0] + noColon;
	}
} // Append_File_To_Path



/*******************************************************************************
 * Last_File_In_Path
 *
 *******************************************************************************/

void Last_File_In_Path(Str255 fullFilePath, Str32 fileName)
{
	short		pos = fullFilePath[0];
	
	while (pos > 0)
	{
		if (fullFilePath[pos] == ':')
			break;
		pos--;
	}
	BlockMove((Ptr)(fullFilePath + pos + 1), (Ptr)(fileName + 1), fullFilePath[0] - pos);
	fileName[0] = fullFilePath[0] - pos;
	return;
} // Last_File_In_Path



/*******************************************************************************
 * Path_To_DirId
 *
 *******************************************************************************/

short Path_To_DirId(Str32 rootVolName, Str255 pathToDir, short *vRefNum, long *dirId)
{
	Str255			fullPath;
	union
	{
		CInfoPBRec		dInfo;
		ParamBlockRec	vInfo;
	} pb;
	OSErr			err;
	
	if (rootVolName[0] != '\0')
		Prepend_Dir_To_Path(rootVolName, pathToDir, fullPath);
	else
		rootVolName = pathToDir;
	
	pb.vInfo.volumeParam.ioCompletion = NULL;
	pb.vInfo.volumeParam.ioVolIndex = -1;
	pb.vInfo.volumeParam.ioNamePtr = rootVolName;
	pb.vInfo.volumeParam.ioVRefNum = 0;
	err = PBGetVInfo(&pb.vInfo, FALSE);
	if (err != noErr)
		return err;
	*vRefNum = pb.vInfo.volumeParam.ioVRefNum;

	if (pathToDir[0] == '\0')
	{
		fullPath[0]++;
		fullPath[fullPath[0]] = ':';
	}
	
	pb.dInfo.dirInfo.ioCompletion = NULL;
	pb.dInfo.dirInfo.ioNamePtr = fullPath;
	pb.dInfo.dirInfo.ioVRefNum = 0;
	pb.dInfo.dirInfo.ioFDirIndex = 0;
	pb.dInfo.dirInfo.ioDrDirID = 0L;
	err = PBGetCatInfo(&pb.dInfo, FALSE);
	*dirId = pb.dInfo.dirInfo.ioDrDirID;
	
	return err;
} // Path_To_DirId



/*******************************************************************************
 * DirId_To_Path
 *
 *******************************************************************************/

short DirId_To_Path(short vRefNum, long dirId, Str32 rootVolName, Str255 pathToDir)
{
	Str32			dirName;
	CInfoPBRec		dInfo;
	OSErr			err;
	
	rootVolName[0] = '\0';
	pathToDir[0] = '\0';
	
	dInfo.dirInfo.ioNamePtr = dirName;
	dInfo.dirInfo.ioDrParID = dirId;
	do
	{
		dInfo.dirInfo.ioVRefNum = vRefNum;
		dInfo.dirInfo.ioFDirIndex = -1;
		dInfo.dirInfo.ioDrDirID = dInfo.dirInfo.ioDrParID;
		err = PBGetCatInfo(&dInfo, FALSE);
		if (err)
			return err;
		if (dInfo.dirInfo.ioDrDirID == fsRtDirID)
			BlockMove((Ptr)dirName, (Ptr)rootVolName, dirName[0] + 1);
		else
			Prepend_Dir_To_Path(dirName, pathToDir, pathToDir);
	} while (dInfo.dirInfo.ioDrDirID != fsRtDirID);
	return noErr;
} // DirId_To_Path



/*******************************************************************************
 * Get_SystemFolder_Path
 *
 *******************************************************************************/
 
short Get_SystemFolder_Path(Str32 rootVolName, Str255 pathToDir)
{
	SysEnvRec		theSysEnv;
	OSErr			err;

	err = SysEnvirons(1, &theSysEnv);
	if (err != noErr)
		return err;
	return DirId_To_Path(theSysEnv.sysVRefNum, 0L, rootVolName, pathToDir);
} // Get_SystemFolder_Path



/*******************************************************************************
 * Get_Preferences_Folder
 *
 *******************************************************************************/

short Get_Preferences_Folder(short *vRefNum, long *dirId)
{
	SysEnvRec		theSysEnv;
	OSErr			err;

	err = SysEnvirons(1, &theSysEnv);
	if (err != noErr)
		return err;
	if ((theSysEnv.systemVersion < 0x0700) || (FindFolder(kOnSystemDisk, 'pref' , TRUE, vRefNum, dirId) != noErr))
	{
		*vRefNum = theSysEnv.sysVRefNum;
		*dirId = 0L;
	}
	return noErr;
} // Get_Preferences_Folder



/*******************************************************************************
 * Find_File1
 *
 *******************************************************************************/

short Find_File1(CInfoPBRec *fInfo, Str32 findFile, long searchDirId, short searchIndex,
				short searchDepth, long *foundDirId, short *foundIndex, Boolean findDirs,
				Boolean findFiles, Boolean *foundDirFlag, Boolean *foundFileFlag)
{
	OSErr		err;
	
	do
	{
		fInfo->hFileInfo.ioFDirIndex = searchIndex;
		fInfo->hFileInfo.ioDirID = searchDirId;
		err = PBGetCatInfo(fInfo, FALSE);
		if (err == noErr)
		{
			*foundDirFlag = (((fInfo->hFileInfo.ioFlAttrib >> 4) & 0x01) == 1);
			*foundFileFlag = !(*foundDirFlag);
			if ((((findDirs) && (*foundDirFlag)) || ((findFiles) && (*foundFileFlag))) &&
				(RelString(fInfo->hFileInfo.ioNamePtr, findFile, FALSE, FALSE) == 0))
			{
				if (*foundDirFlag)
					*foundDirId = fInfo->hFileInfo.ioDirID;
				else
					*foundDirId = fInfo->hFileInfo.ioFlParID;
				*foundIndex = searchIndex;
				return noErr;
			}
			if ((*foundDirFlag) && (searchDepth))
			{
				if (Find_File1(fInfo, findFile, fInfo->hFileInfo.ioDirID, 1, searchDepth - 1, foundDirId,
						foundIndex, findDirs, findFiles, foundDirFlag, foundFileFlag) == noErr)
					return noErr;
			}
			searchIndex++;
		}
	} while (err == noErr);
	
	return err;
} // Find_File1



/*******************************************************************************
 * Find_File
 *
 *******************************************************************************/

short Find_File(Str32 findFile, short *vRefNum, long *dirId, short *index, short searchDepth,
				Boolean *findDirs, Boolean *findFiles)
{
	CInfoPBRec		fInfo;
	Str255			fName;
	
	if (*vRefNum == 0)
		Get_Root_Vol_Info(vRefNum, NULL);

	fInfo.hFileInfo.ioNamePtr = fName;
	fInfo.hFileInfo.ioVRefNum = *vRefNum;
	
	return Find_File1(&fInfo, findFile, *dirId, *index, searchDepth, dirId, index, *findDirs,
						*findFiles, findDirs, findFiles);
} // Find_File



/*******************************************************************************
 * SFGetDirHook
 *
 *******************************************************************************/

pascal short SFGetDirHook(short item, DialogPtr theDialog)
{
	short		iType;
	Handle		iHandle;
	Rect		iRect;
	
	switch (item)
	{
		case 11:
			return sfItemOpenButton;

		default:
			return item;
	}
} // SFGetDirHook



/*******************************************************************************
 * SFGetDirectory
 *
 *******************************************************************************/

Boolean SFGetDirectory(Point where, short dlogID, short *vRefNum, long *dirId)
{
	enum {
		SFSaveDisk = 0x214,
		CurDirStore = 0x398
	};
	SFTypeList		typeList;
	SFReply			reply;
	
	typeList[0] = '\0\0\0\0';
	SFPGetFile(where, "\p", NULL, 1, typeList, (DlgHookUPP)SFGetDirHook, &reply, dlogID, NULL);
	
	*vRefNum = -(SFSaveDisk);
	*dirId = CurDirStore;
	
	return reply.good;
} // SFGetDirectory



