// FILE    : Scan.c++
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

// P.S.
//	READ THE HEADER!!! (Scan.h)

// ----------------------------------------------------------------

// NOTES   :
//		So what are these routines, anyway? Well, I'm developing a game right
//	now, and I'm trying to make it as extensible as possible. That's why
//	I'm implementing a "Plug-Ins" type of scheme: within the same folder as
//	the application, I plan to have a folder named "Plug-Ins" or "Data"
//	or what-have-you, with the requisite plug-in files inside. This scheme is
//	very much like Photoshop's Plug-Ins folder, Firefall's Data folder, etc.
//	"Enough!" you say. OK. Anyway, this is all fine and dandy, but how does
//	one *actually* implement something like this??? I consider myself an
//	intermediate programmer (when I can write a pixel-blitting routine by myself
//	or understand how the hell to use cluts and pltts, then I'll consider
//	myself an advanced programmer) so programming something like this is a
//	tad tricky, especially if you're dealing with the File Manager�too many
//	ioThisPtrs, vRefThats, and (Funky)TypeCasting->ioCrazy.thisFileDirIDs!!!!

//		OK, down to the code. Luckily I have Think Reference, and I was able
//	to find some information on searching a volume. By the way, if you don't
//	have Think Reference, I suggest you stop reading right now, get dressed,
//	get some money, and go to the local computer store and get your copy
//	right now. Right now, I say! (or call MacConnection)...

//		Think Reference had an example routine on searching all the files in a
//	volume (File Manager, "Searching All Files On A Volume"). I basically
//	copied off of that, but made some modifications here and there where it
//	was necessary. I had to dig up information on HFileInfo's, volume id's,
//	directory id's, "hard" numbers, ioNamePtrs, and on and on and on...
//	Too many reference numbers! Sigh. Oh well. Thank goodness I don't design
//	scsi hard disks. Read the accompanying "HFS Illuminated" file for a layman's
//	explanation of what all those numbers mean...

//		Boy, am I getting off track! Down to business: remembering my goal
//	mentioned earlier, my initial scanning code scanned *everything* within
//	the same folder as the application. This is not that good. It should scan
//	only in the specified folder (i.e. "Plug-Ins"), and not in any other folders.
//	We might not want to use a plug-in; to do that we should be able to place
//	the plug-in *anywhere* except the Plug-Ins folder. So I changed the scanning
//	code to only start scanning in the folder passed to it. This is much better!
//	Note that if you still want to start scanning in the same folder as the 
//	application, just pass nil in the folderName argument.

//		After that, I added specific search criteria: scan folder by file type,
//	by file creator, by both type and creator, or by file name. Or you can just
//	scan everything in the folder. This code hasn't been really extensively tested,
//	but it should work. It definitely works on a Mac IIsi w/ 5 megs of ram running
//	System 7.1... It was written in Symantec C++ 6.0, but *should* work on THINK C
//	5.0 (just change .c++ to .c).

//		Some final notes: Macintosh Tech Note #68 (Searching Volumes) states that
//	that you should search for files via _PBCatSearch, if it's available. It's
//	much faster than the recursive _PBGetCatInfo method I use below, but to be
//	honest, I think the method below (albeit a tad messy and recursive) is way
//	easier to understand than _PBCatSearch. I've read the section on it in Inside
//	Mac VI many times, but I still don't know how to fill in the necessary fields
//	for PBCatSearch. Is it me or is that book way too technical (and not enough
//	clear and "meaty" examples, to boot!) Anyway, the intended use for the Scan
//	functions is just to scan for a moderate amount of files located in a folder
//	within the same directory as the application running it, not as a file
//	searcher or cataloguer, so they should work just fine as they were intended.

//		I hope this code helps you in your programming efforts or further your
//	understanding of Macintosh programming. I can't say how much I learned just
//	from downloading source code from other programmers. This is the best way of
//	learning Mac programming, if you ask me (but only if the source code is
//	sufficiently commented, else it's a disaster!) This code is free, and you
//	can use it any way you want. Any comments, suggestions, queries, death-threats,
//	bug reports, etc. can be made to me at America Online, StarLabs. Enjoy!!!

//	Hiep Dam
//	3G Software


//	Version History:
//	1.0: Worked fine with my demo application...
//	1.1: Fixed a bug when passing nil as folderName. It seems gCPB.ioNamePtr wasn't
//		 set when passing a nil folderName; added code in InitScan to fix that.
//  1.2: Fixed a bug in GetFolderDirID. Same bug as above!! I forgot to set
//		 gCPB.ioNamePtr and gCPB.ioVRefNum. The routine worked fine if called by
//		 Scan routines (since they set ioNamePtr & ioVRefNum) but calling GetFolderDirID
//		 directly caused it to return -1, since it can't find any folder with an
//		 empty name. Oops!! (1/5/94)
//	1.3: Added support for aliases. Now you can tell the scanning routines to
//		 resolve the aliases for you. You can also tell the routines to resolve folder
//		 aliases as well. Whew!! Also fixed and changed the code here and
//		 there. You know, the little stuff. (2/20/94)

// ----------------------------------------------------------------

#include <Packages.h>	// Needed to use _IUEqualString
#include <Aliases.h>	// Needed for _ResolveAliasFile
#include "Scan.h"

// ----------------------------------------------------------------

// Some globals

// ** Version 1.3 Change **: Made globals below static...

// Used by ScanContents and ScanDirectory4Folder only...
static HFileInfo gCPB;				// Low-level volume information used by _PBGetCatInfo
static Str255 gCurFileName;			// Name of current file/folder we're looking at
static long gDirScratch = -1;		// Used to temporarily  hold a directory id

// Used by all of the routines...
static FSSpec *gCurFileArray = nil;	// Array to hold list of FSSpec's
static short gCurFileIndex;			// Current index into above-mentioned array
static short gMaxFileLimit;			// Size of above-mentioned array
static Boolean gRestrictByType;		// Look for file based on file type?
static Boolean gRestrictByCreator;	// Look for file based on file creator?
static Boolean gRestrictByName;		// Look for file based on name?
static Boolean gResolveAlias;		// Do we resolve aliases?	Vers 1.3 addition
static Boolean gResolveFolderAlias;	// Do we resolve folder aliases & look in there? Vers 1.3 addition
static OSType gFileType;			// File type, i.e. "TEXT", "PICT", etc.
static OSType gCreatorType;			// File creator, i.e. "SPNT", "KAHL", etc.
static Str63 gFileName;				// File name.
static FSSpec gTempSpec;			// Used to temporarily make FSSpec's. Vers 1.3 addition

// ----------------------------------------------------------------

// Pascal string functions below courtesy of Alex. D. Chaffee,
// from his CreatePath.c source code...

// ** Version 1.3 Change **: Change name of Length to PLength to avoid clash
// with THINK C 6.0 syntax...

//	return length of string
#define PLength(s) (int)(*(s))

// copy part of one pascal string to another
void Copy(unsigned char *dest, unsigned char *src, int start, int end)
{
	int length = end-start+1;
	if (length > 0) {
		*dest = length;
		BlockMove(src+start, dest+1, length);
	} else
		*dest = 0;
}

//	concatenate a CHAR onto a string
void Concat(unsigned char *dest, unsigned char *src, char c)
{
	Copy(dest, src, 1, PLength(src));
	dest[0]++;
	dest[dest[0]]=c;
}

// Prefix a CHAR onto a string; this is my own routine, modeled
// after Concat
void Precat(unsigned char *dest, unsigned char *src, char c) {
	Copy(dest, src, 1, PLength(src));
	// Shift everything down
	short strEnd = PLength(src);
	if (strEnd == 63)
		strEnd--;
	for (short i = PLength(src)+1; i > 0; i--)
		dest[i] = dest[i-1];
	if (dest[0] < 63)
		dest[0]++; // Update Pascal string length byte
	dest[1] = c;
}

// ----------------------------------------------------------------

// ScanContents.
// Scans starting at dirIDToSearch, based on parameters set in the globals
// above. Admittedly a messy and lazy implementation, but it's recursive and
// I didn't want to pass identical parameters to it every time.
// This is the "core" of all the scanning routines.

void ScanContents(long dirIDToSearch)
{
	short index = 1;
	OSErr err;
	Boolean matchType = false;
	Boolean matchCreator = false;
	Boolean matchName = false;
	Boolean addFSSpec = false;
	Boolean wasFolder, wasAlias;
	
	do {
		gCPB.ioFDirIndex = index;	// set up index
		// do this every time since PBGetCatInfo returns ioFlNum
		// in this field
		gCPB.ioDirID = dirIDToSearch;
		err = PBGetCatInfo((CInfoPBRec*)&gCPB, false);
		if(err == noErr) {
			// check to see if the file is a folder
			if ((gCPB.ioFlAttrib & ioDirMask) != 0) {	// Version 1.3 Change: made more elegant
				// found a directory, so search on.
				// Make recursive call
				ScanContents(gCPB.ioDirID);
				err = 0;
			}
			else if ((gCPB.ioFlFndrInfo.fdType == 'fdrp') && gResolveFolderAlias) {
				// 'fdrp' == kContainerFolderAliasType
				// First, make a FSSpec of the alias file
				(void)FSMakeFSSpec(gCPB.ioVRefNum, gCPB.ioFlParID, gCurFileName, &gTempSpec);

				// Now turn that FSSpec of an alias file (folder in our case)
				// into the actual file (resolve the reference)
				(void)ResolveAliasFile(&gTempSpec, true, &wasFolder, &wasAlias);

				gCPB.ioCompletion = nil;
				BlockMove((Ptr)&gTempSpec.name[0], (Ptr)&gCurFileName[0], gTempSpec.name[0]);
				gCPB.ioVRefNum = gTempSpec.vRefNum;
				((CInfoPBRec)gCPB).dirInfo.ioDrDirID = gTempSpec.parID;
				gCPB.ioFDirIndex = 0;	// Use name and dirID
				(void)PBGetCatInfo((CInfoPBRec*)&gCPB, false);

				ScanContents(((CInfoPBRec)gCPB).dirInfo.ioDrDirID);
				err = 0;
			}
			else {
				// Found a file, now check to see if it matches
				// the search criteria...
				addFSSpec = false;	// Don't add file to FSSpec array � yet.

				if (gRestrictByName) {
					// Ok, search criteria is by name, so see if our
					// file's name matches:
					if (IUEqualString(gCPB.ioNamePtr, gFileName) == 0)
						matchName = true;	// Yeah, matched!
					else
						matchName = false;	// Nope...
				}
				if (gRestrictByType) {
					// Ok, one of the search criteria is by type,
					// so check the file's type to see if it's what
					// we're looking for:
					if (gCPB.ioFlFndrInfo.fdType == gFileType)
						matchType = true;
					else
						matchType = false;
				}
				if (gRestrictByCreator) {
					// Same as above, but look at the file's creator type...
					if (gCPB.ioFlFndrInfo.fdCreator == gCreatorType)
						matchCreator = true;
					else
						matchCreator = false;
				}

				// Now we have to determine if we add the file to the
				// FSSpec array. There's several different combinations
				// of type/creator search criteria, so we'll have to
				// do a if-then-else run-through:
				if (gRestrictByName) {
					if (matchName)
						addFSSpec = true;
				}
				else if (gRestrictByType && gRestrictByCreator) {
					if (matchType && matchCreator)
						// Add file to array only if BOTH type and creator match...
						addFSSpec = true;
				}
				else if (gRestrictByType && !gRestrictByCreator) {
					if (matchType)
						addFSSpec = true;
				}
				else if (!gRestrictByType && gRestrictByCreator) {
					if (matchCreator)
						addFSSpec = true;
				}
				else if (!gRestrictByType && !gRestrictByCreator)
					// This means we'll add the file, no matter it's
					// type, creator, or name...
					addFSSpec = true;

				// Is it time to add to the array yet?
				if (addFSSpec) {
					// Alright, add the file by making an FSSpec...
					short fsErr = FSMakeFSSpec(gCPB.ioVRefNum, gCPB.ioFlParID,
								  gCurFileName, &gCurFileArray[gCurFileIndex++]);

					if (gResolveAlias)
						(void)ResolveAliasFile(&gCurFileArray[gCurFileIndex - 1], true,
								&wasFolder, &wasAlias);

					// Let's do some bounds checking: it's possible there are
					// more files than there are FSSpec's in the array, so:
					if (gCurFileIndex == gMaxFileLimit)
						err = 1; // stop scanning, reached max file limit...
					
					// Searching by name, instead of by type or creator, is
					// slightly different in that once we find the matching file,
					// we exit the search. Scanning by type or creator, however,
					// continues until there are no more files or until the
					// FSSpec array is exhausted.
					if (gRestrictByName)
						err = 1;
				}
			}
			index++;
		}
	} while(err == noErr);
} // END ScanContents

// ----------------------------------------------------------------

// ScanDirectory4Folder.
// Scans a directory (read=hard disk, etc.) for the specified folder, starting
// at dirIDToSearch. Returns the directory id of the folder in gDirScratch,
// if folder found. You won't have any need to use this function, though.
// The code is pretty much duplicated from ScanContents(). It's simpler, so
// you can use this to write your own scanning routines.

void ScanDirectory4Folder(Str63 folderName, long dirIDToSearch) {
	short index = 1;
	OSErr err;

	do {
		gCPB.ioFDirIndex = index;
		gCPB.ioDirID = dirIDToSearch;
		err = PBGetCatInfo((CInfoPBRec*)&gCPB, false);
		if(err == noErr) {
			// check to see if the file is a folder
			if ((gCPB.ioFlAttrib & ioDirMask) != 0) {
				// found a directory
				if (IUEqualString(gCPB.ioNamePtr, folderName) == 0) {
					gDirScratch = gCPB.ioDirID;
					err = 1; // stop scanning
				}
				else {
					ScanDirectory4Folder(folderName, gCPB.ioDirID);
					err = 0;
				}
			}
			else {
				// found a file. Do nothing.
			}
			index++;
		}
	} while(err == noErr);
}	// END ScanDirectory4Folder


// GetFolderDirID.
// A much more useful function. Returns the directory id
// of the specified folder (you'll have to know the name of the
// folder beforehand, thus users should not change the folder's
// name). Makes a call to ScanDirectory4Folder.
// Returns -1 if folder not found.

long GetFolderDirID(Str63 folderName, long startDirID) {
	gDirScratch = -1;	// Initialize global variable

	gCPB.ioNamePtr = gCurFileName;	// Version 1.2 Fix
	gCPB.ioVRefNum = 0;				// Ditto...

	ScanDirectory4Folder(folderName, startDirID);
	return gDirScratch;
} // END GetFolderDirID

// ----------------------------------------------------------------

// InitScan.
// Initializes some global variables...

void InitScan(FSSpec *fileArray, short arraySize) {
	gCurFileArray = fileArray;	// Array of FSSpec's passed to us
	gCurFileIndex = 0;			// Index into the array; start at 0
	gMaxFileLimit = arraySize;	// Size of array (so we don't overstep bounds)
	gCPB.ioNamePtr = gCurFileName;
	gCPB.ioVRefNum = 0;
} // END InitScan

// ----------------------------------------------------------------

// ScanFolder.
// See header file for comments on using this function.
// Returns number of files found and placed in the FSSpec array, 0 otherwise.

short ScanFolder(Str63 folderName, long startDirID, FSSpec *fileArray, short arraySize,
		Boolean resolveAlias, Boolean resolveFolderAlias) {
	InitScan(fileArray, arraySize);
	// Scan *everything*, so turn off all search criteria...
	gRestrictByType = false;
	gRestrictByCreator = false;
	gRestrictByName = false;
	gResolveAlias = resolveAlias;
	gResolveFolderAlias = resolveFolderAlias;

	long startDir;

	if (folderName != nil)
		startDir = GetFolderDirID(folderName, startDirID);
	else
		startDir = startDirID;
	ScanContents(startDir);
	if (gCurFileIndex > 0)
		return gCurFileIndex;
	else
		return 0;
} // END ScanFolder

// ----------------------------------------------------------------

short ScanFolderByType(Str63 folderName, long startDirID, OSType restriction, FSSpec *fileArray,
		short arraySize, Boolean resolveAlias, Boolean resolveFolderAlias) {
	InitScan(fileArray, arraySize);
	gFileType = restriction;	// OSType to filter through
	gRestrictByType = true;
	gRestrictByCreator = false;
	gRestrictByName = false;
	gResolveAlias = resolveAlias;
	gResolveFolderAlias = resolveFolderAlias;

	long startDir;
	if (folderName != nil)
		startDir = GetFolderDirID(folderName, startDirID);
	else
		startDir = startDirID;
	ScanContents(startDir);
	if (gCurFileIndex > 0)
		return gCurFileIndex;
	else
		return 0;
} // END ScanFolderByType

// ----------------------------------------------------------------

short ScanFolderByCreator(Str63 folderName, long startDirID, OSType restriction, FSSpec *fileArray,
		short arraySize, Boolean resolveAlias, Boolean resolveFolderAlias) {
	InitScan(fileArray, arraySize);
	gCreatorType = restriction;
	gRestrictByType = false;
	gRestrictByCreator = true;
	gRestrictByName = false;
	gResolveAlias = resolveAlias;
	gResolveFolderAlias = resolveFolderAlias;

	long startDir;

	if (folderName != nil)
		startDir = GetFolderDirID(folderName, startDirID);
	else
		startDir = startDirID;
	ScanContents(startDir);
	if (gCurFileIndex > 0)
		return gCurFileIndex;
	else
		return 0;
} // END ScanFolderByCreator

// ----------------------------------------------------------------

short ScanFolderSpecific(Str63 folderName, long startDirID,
		OSType fType, OSType fCreator, FSSpec *fileArray, short arraySize,
		Boolean resolveAlias, Boolean resolveFolderAlias) {
	InitScan(fileArray, arraySize);
	gCreatorType = fCreator;
	gFileType = fType;
	gRestrictByType = true;
	gRestrictByCreator = true;
	gRestrictByName = false;
	gResolveAlias = resolveAlias;
	gResolveFolderAlias = resolveFolderAlias;

	long startDir;
	if (folderName != nil)
		startDir = GetFolderDirID(folderName, startDirID);
	else
		startDir = startDirID;
	ScanContents(startDir);
	if (gCurFileIndex > 0)
		return gCurFileIndex;
	else
		return 0;
} // END ScanFolderSpecific

// ----------------------------------------------------------------

short ScanFolderByName(Str63 folderName, long startDirID, Str63 fileName,
					FSSpec *fileArray, short arraySize, Boolean resolveAlias, Boolean resolveFolderAlias) {
	InitScan(fileArray, arraySize);
	Copy(gFileName, fileName, 1, Length(fileName));
	gRestrictByType = false;
	gRestrictByCreator = false;
	gRestrictByName = true;
	gResolveAlias = resolveAlias;
	gResolveFolderAlias = resolveFolderAlias;

	long startDir;
	if (folderName != nil)
		startDir = GetFolderDirID(folderName, startDirID);
	else
		startDir = startDirID;
	ScanContents(startDir);
	if (gCurFileIndex > 0)
		return gCurFileIndex;	// Should be one, since we're scanning by a file's name
	else
		return 0;
} // END ScanFolderByName

// ----------------------------------------------------------------

void PrefixColon(FSSpec *fileSpec) {
	Precat(fileSpec->name, fileSpec->name, ':');
}

void PrefixColons(FSSpec *fileArray, short arraySize) {
	for (short i = 0; i < arraySize; i++)
		PrefixColon(&fileArray[i]);
}