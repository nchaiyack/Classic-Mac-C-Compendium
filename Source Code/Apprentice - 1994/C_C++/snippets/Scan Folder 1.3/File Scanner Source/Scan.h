
// FILE    : Scan.h
// NAME    : Scan Folder Utilities
// VERSION : 1.3
// DATE    : Dec. 29, 1993
// UPDATE  : Feb. 21, 1994
// AUTHOR  : Hiep Dam, 3G Software
// CONTACT : Via America Online at StarLabs, or
//			 2226 Parkside Ave. #302, Los Angeles, CA 90031
// STATUS  : This code is freeware and is in the public domain. Feel free to use it
//			 if you find it useful (it would be nice if you gave me some credit,
//			 though  ;)

// ----------------------------------------------------------------

// ScanFolder.
// Scan for the given folder, starting at startDirID, and then scan the contents
// of the folder for any files. Any files found will be appended to the FSSpec array
// you pass to it. Note that the FSSpec records ScanFolder creates (and all the other
// Scan functions as well) do not prefix the file's name with a ":". This is so
// that you can display the file name, do other things with it first, etc. If you
// wish, you can have the colons prefixed. Luckily you don't have to do this yourself;
// just call PrefixColon for one FSSpec, or PrefixColons for an array of FSSpec's.
// I've found that FSpOpenDF and FSpOpenResFile work fine either with or without
// a colon. I'm not sure if it's required before a filename (not pathname). If you
// know, please tell me! (Thanks!)
// If you want ScanFolder to start the scan in the same folder as the application,
// pass nil in the folderName argument, still passing the same startDirID (assuming
// this dirID is the default dir id).
// ScanFolder returns the number of files found.
// Vers 1.3 addendum: scanning routine now works with aliases...

short ScanFolder(Str63 folderName, long startDirID, FSSpec *fileArray, short arraySize,
			Boolean resolveAlias = true, Boolean resolveFolderAlias = true);


// ScanFolderByType.
// Much like ScanFolder, but restrict the scan to files of a particular type, i.e.
// scan only for "TEXT" files, "PICT" files, "sfil" files, etc. Pass the file
// type in "restriction".
// ScanFolderByType returns the number of files found with the given type located
// in the folder folderName.
short ScanFolderByType(Str63 folderName, long startDirID, OSType restriction, FSSpec *fileArray,
			short arrraySize, Boolean resolveAlias = true, Boolean resolveFolderAlias = true);


// ScanFolderByCreator.
// Exactly like ScanFolderByType, except do it by creator, i.e. Think's "KAHL" or
// Teachtext's "ttxt", etc.
short ScanFolderByCreator(Str63 folderName, long startDirID, OSType restriction, FSSpec *fileArray,
			short arraySize, Boolean resolveAlias = true, Boolean resolveFolderAlias = true);


// ScanFolderSpecific.
// A combination of ScanFolderByCreator and ScanFolderByType.
short ScanFolderSpecific(Str63 folderName, long startDirID, OSType fType, OSType fCreator, FSSpec *fileArray,
			short arraySize, Boolean resolveAlias = true, Boolean resolveFolderAlias = true);


// ScanFolderByName.
// Actually a search routine.
// Search for the file with the given filename within the folder folderName.
// The file found will be placed in fileArray[0]. ScanFolderByName will only search
// for the file once; once it finds the file, it exits, so any files with duplicate
// names that may be in nested folders within folder folderName won't be found.
short ScanFolderByName(Str63 folderName, long startDirID, Str63 fileName, FSSpec *fileArray,
			short arraySize, Boolean resolveAlias = true, Boolean resolveFolderAlias = true);


// GetFolderDirID.
// If for any reason you need the directory id of a folder, you can use this function.
// Else, you'll probably have no use for it; it's used primarily by the above Scan
// routines (they use it to search for the folder you specify to them).
long GetFolderDirID(Str63 folderName, long startDirID);


// PrefixColon(s).
// Prefix file names in a FSSpec record with a colon ":". Note that
// these routines do no error-checking, so if the file already has a
// colon prefixed to it, these routines will prefix anyway.
void PrefixColon(FSSpec *fileSpec);
void PrefixColons(FSSpec *fileArray, short arraySize);