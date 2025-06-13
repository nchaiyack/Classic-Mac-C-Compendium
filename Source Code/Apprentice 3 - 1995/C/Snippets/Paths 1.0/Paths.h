/*******************************************************************************
 * Paths.h
 *
 * Miscellaneous routines for Think C to convert volume and directory
 * information from ids to names, names to ids, find folders/files, etc.
 *
 * Append_File_To_Path		Insert directory/file name at end of path name
 * DirId_To_Path			Returns a path name given a volume and directory id
 * Find_File				Searches a volume for a file or directory name
 * Get_Preferences_Folder	Returns a volume and directory id for the active
 *							Preferences folder
 * Get_Root_Vol_Info		Returns ref. number and name of default root volume
 * Get_SystemFolder_Path	Returns a path name to the active System Folder
 * Last_File_In_Path		Returns the last file or directory name in a path
 * Path_To_DirId			Returns a volume and directory id for a path name
 * Prepend_Dir_To_Path		Insert directory/volume name in front of path name
 * SFGetDirectory			A standard file dialog to select a directory
 *
 * Version 1.0 ¥ June 1994
 * ©1994 T&T Software ¥ by Tad Woods ¥ 70312,3552 on CompuServe
 *
 * You may use these routines in your own applications. They have been
 * tested but the author will not be responsible for any unforseen damage
 * caused by using these routines. Send problem reports or comments to
 * the address above.
 * Include MacTraps and MacTraps2 in your project.
 *
 *******************************************************************************/

#pragma once



/*******************************************************************************
 * Append_File_To_Path
 *
 *  -> fileName		pascal string to append to pathName
 *  -> pathName		pascal string
 * <-  fullFilePath	pascal string containing pathName + : + fileName
 *					(fullFilePath may be the same string as pathName)
 *
 * Appends a : and fileName to the end of pathName. If pathName is empty,
 * the : is not inserted.
 *
 *******************************************************************************/
void Append_File_To_Path(Str32 fileName, Str255 pathName, Str255 fullFilePath);



/*******************************************************************************
 * DirId_To_Path
 *
 *  -> vRefNum		volume reference number
 *  -> dirId		directory id (0 if vRefNum is a working directory id)
 * <-  rootVolName	on exit, name of the root volume
 * <-  pathToDir	on exit, path name to the directory w/o the root volume name
 *
 * Returns 0 or an error code.
 *
 * Converts vRefNum and dirId to a corresponding path name.
 * After DirId_To_Path, call Prepend_Dir_To_Path(rootVolName, pathToDir,
 * pathToDir) to build the full path name.
 *
 *******************************************************************************/
short DirId_To_Path(short vRefNum, long dirId, Str32 rootVolName, Str255 pathToDir);



/*******************************************************************************
 * Find_File
 *
 *  -> findFile		Pascal string of file or directory name to find
 * <-> vRefNum		on entry, volume ref. number to search (normally 0)
 *					on exit, volume ref. number where file/directory was found
 * <-> dirId		on entry, directory id to start search (normally 2)
 *					on exit, directory id where file was found or directory id
 *					of directory found
 * <-> index		on entry, index in directory to start search (normally 1)
 *					on exit, index in directory where file/directory was found
 *  -> searchDepth	number of sub-folders deep to search; -1 to search all
 *					sub-folders and sub-folders within folder; 0 to search
 *					no sub-folders
 * <-> findDirs		on entry, TRUE to comapre directory names otherwise FALSE
 *					on exit, TRUE if file found was a directory otherwise FALSE
 * <-> findFiles	on entry, TRUE to compare file names otherwise FALSE
 *					on exit, TRUE if file found was a file otherwise FALSE
 *
 * Returns an error code or 0 if file or directory was found.
 *
 *******************************************************************************/
short Find_File(Str32 findFile, short *vRefNum, long *dirId, short *index, short searchDepth,
				Boolean *findDirs, Boolean *findFiles);



/*******************************************************************************
 * Get_Preferences_Folder
 *
 * <-  vRefNum		on exit, volume reference number where Preferences folder
 *					was found
 * <-  dirId		on exit, directory id where Preferences folder was found
 *
 * Returns 0 or an error code.
 *
 * For System 7 and later vRefNum and dirId refer to the Preferences folder
 * within the System Folder. For System 6 and eariler, they refer to the
 * System Folder itself.
 *
 *******************************************************************************/
short Get_Preferences_Folder(short *vRefNum, long *dirId);



/*******************************************************************************
 * Get_Root_Vol_Info
 *
 * <-  rootVolNum	on entry, pointer to short or NULL
 *					on exit the volume ref. of the default root volume
 * <-  rootVolName	on entry, string pointer or NULL
 *					on exit the name of the default root volume
 *
 * Returns 0 or an error code.
 *
 *******************************************************************************/
short Get_Root_Vol_Info(short *rootVolNum, Str32 rootVolName);



/*******************************************************************************
 * Get_SystemFolder_Path
 *
 * <-  rootVolName	on exit, name of the root volume
 * <-  pathToSysDir	on exit, path name to the system folder
 *
 * Returns 0 or an error code.
 *
 *******************************************************************************/
short Get_SystemFolder_Path(Str32 rootVolName, Str255 pathToSysDir);



/*******************************************************************************
 * Last_File_In_Path
 *
 *  -> fullFilePath	pascal string containing full path to a file name
 * <-  fileName		pascal string containing just fileName
 *					(fileName may be the same string as fullFilePath)
 *
 * Extracts the last directory/file name from a full path name.
 *
 *******************************************************************************/
void Last_File_In_Path(Str255 fullFilePath, Str32 fileName);



/*******************************************************************************
 * Path_To_DirId
 *
 *  -> rootVolName	name of the root volume,
 *					or "\p" if full path name is already in pathToDir
 *  -> pathToDir	path name to the directory without the root volume name
 * <-  vRefNum		on exit, volume reference number
 * <-  dirId		on exit, directory id
 *
 * Returns 0 or an error code.
 *
 * Converts rootVolName and pathToDir to corresponding vRefNum and dirId.
 *
 *******************************************************************************/
short Path_To_DirId(Str32 rootVolName, Str255 pathToDir, short *vRefNum, long *dirId);



/*******************************************************************************
 * Prepend_Dir_To_Path
 *
 *  -> dirName		pascal string to insert before pathName
 *  -> pathName		pascal string
 * <-  fullDirPath	pascal string containing dirName + : + pathName
 *					(fullFilePath may be the same string as pathName)
 *
 * Inserts dirName plus a : in front of pathName. If pathName is empty,
 * the : is not inserted.
 *
 *******************************************************************************/
void Prepend_Dir_To_Path(Str32 dirName, Str255 pathName, Str255 fullDirPath);



/*******************************************************************************
 * SFGetDirectory
 *
 *  -> where		global point to display dialog
 *  -> dlogID		resource ID of dialog box to use (this custom resource
 *					must be in your application's resource file)
 * <-  vRefNum		on exit, volume reference number of folder in view
 * <-  dirId		on exit, directory id of folder in view
 *
 * Returns TRUE if the user selected a directory or FALSE if the user
 * pressed Cancel.
 *
 * Calls SFPGetFile with a custom dialog resource to get the user to
 * select a directory.
 *
 *******************************************************************************/
Boolean SFGetDirectory(Point where, short dlogID, short *vRefNum, long *dirId);



