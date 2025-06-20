/*
	DFilePaths.c
	
	A Dragon subspecies developed using THINK C 5.0

	Places a list of the full pathnames of the files dropped on it into the clipboard (desk scrap).
	
	Copyright � 1992�1994 by Paul M. Hoffman
	Send feedback to paul.hoffman@umich.edu -or- dragonsmith@umich.edu
	
	This code may be freely used, altered, and distributed in any way you want as long as:
		1.	It is GIVEN away rather than sold;
		2.	This statement and the above copyright notice are left intact.

	NOTE:	You should set the "Stationery aware" SIZE bit for this � since it doesn't actually
			do anything to the things dropped on it, we don't want the Finder to think it has to
			make a copy of a stationery file before we get to see it!
				
	Possible enhancements		�	Add an option to create a file with the paths/names instead of
								copying them to the clipboard.  Also append to a file?
*/

#include	"Dragon.h"
#include	"FileUtils.h"
#include	"MenuUtils.h"
#include	"HandleUtils.h"
#include	<Files.h>
#include	<Types.h>

typedef struct {
	char		left;
	char		right;
} QuotesPair;

typedef struct {
	short		flags;
	char			filler;
	char			quotesStart;	// Initial setting of the quotes option
	QuotesPair	quotes[4];	// There should be at least 4 quotes-pairs in the 'PthP' 128 resource
} FPOptions;

class DFilePaths: Dragon {

	protected:
		Handle			pathsHndl;		// Handle to the block where we accumulate the paths
		long				blockLen;		// Length of the pathsHndl block
		long				pathsLen;		// Length of the used portion of pathsHndl
		long				threshold;		// Index of the byte szMaxPathLen bytes from the end of the block
		long				limit;				// Index of the last byte in the block
		char				quotesNum;		// Number of the quotes pair to use
		QuotesPair		quotes;			// Quotes to use
		Boolean			fullPaths;			// Return the full paths?
		Boolean			showNesting;		// Give a hierarchical view of the docs?
		Boolean			textFilesOnly;		// Process text files only?
		FPOptions		**fpOptions;
		MenuHandle		optionsMenu;
		
	public:
						DFilePaths (void);
	
	protected:
		virtual void		BeginProcessing (void);
		virtual void		EndProcessing (void);
		virtual void		ProcessFile (void);
		virtual void		ProcessDirectory (void);
		virtual void		ProcessDocsInDirectory (short vRefNum, long dirID);
		virtual Boolean	CustomFilterDoc (void);
		virtual void		ReadPrefs (void);

		virtual void		SetUpMenus (void);
		virtual void		InitOptionsMenu (void);
		virtual void		SetQuotesOption (short optionItem);
		virtual void		ToggleTextFilesOnly (void);
		virtual void		ToggleFullPaths (void);
		virtual void		ToggleShowNesting (void);
		virtual void		DoMenu (long menuItemCode);
		virtual void		DoOptionsMenu (short itemNum);
		virtual void		DoAbout (void);
		virtual void		AdjustMenusBusy (void);
		virtual void		AdjustMenusIdle (void);
};

#define	cRETURN	'\r'
#define	cNIL			'\0'
#define	cCOLON		':'
#define	cTAB		'\t'

#define	szMaxNameLength		32			// FSSpec file names can be up to 64 bytes, but we're
										// 	going to pretend MFS doesn't exist � and file
										// 	names under HFS can only be 32 bytes
#define	szMaxPathLen		1000		// One can reasonably expect the length of a file's path to be less than this
#define	szExtraRoom			400			// Extra bytes to minimize # of handle resizings needed
#define	szPathLenEstimate	70			// Estimated average path length
#define	szGrowBy			szPathLenEstimate * 6	// Number of bytes to grow the pathsHndl block by
#define	nSeveralDocs			10			// Make sure there's enough memory to process at least this many docs

enum {
	mOptions = mEdit + 1
};

// Items in the Options menu
enum {
	iFullPaths = 1,
	iShowNesting,
	iTextFilesOnly,
	iLine2,
	iNoQuotes,
	iSingleQuotes,
	iDoubleQuotes,
	iAngleBrackets
	// More quotes settings may follow here � just add to 'MENU' 131 ( == mOptions) and 'PthP' 128
};

enum {
	prefFilePathOptions = prefDragonPrefs + 1
};

// Possible values for the flags field in FPOptions
enum {
	maskShowNesting = 1,		// Least significant bit
	maskTextFilesOnly = 2,
	maskFullPaths = 4
};

// Possible values for the quotesStart field in FPOptions (and hence this->quotesNum)
enum {
	qNone,
	qSingle,
	qDouble,
	qAngle
	// Other values may follow if you've added to the 'MENU' 131 and 'PthP' 128 resources
};

// Possible error codes passed to StopProcessing
enum {
	eOverranPathsBlock = eLastDragonError + 1
};

short GetFileName (FSSpec *fsspec, char *name);
long GetFullPath (FSSpec *fsspec, char *path, OSErr *err);
unsigned short ReverseCopyP2CStr (unsigned char *pas, char *c);
void ReverseCStr (char *str);
Boolean FSpIsFile (FSSpec *fss);
Boolean FSpIsVolume (FSSpec *fss);

DFilePaths::DFilePaths (void)
{
	autoQuit = FALSE;
	useCustomFilter = TRUE;
	
	// These DFilePaths-specific preferences settings will be overridden in ReadPrefs
	quotesNum = qNone;
	quotes.left = quotes.right = cNIL;
	fullPaths = TRUE;
	showNesting = FALSE;
	textFilesOnly = TRUE;
}

void DFilePaths::ReadPrefs (void)
{
	FPOptions	*fpop;
	short		fpflags;

	inherited::ReadPrefs ();
	
	fpOptions = (FPOptions **) preferences->GetPrefResource (prefFilePathOptions);
	if (fpOptions != NULL) {
		HNoPurge ((Handle) fpOptions);
		fpop = *fpOptions;
		fpflags = fpop->flags;
		fullPaths = ((fpflags & maskFullPaths) != FALSE);
		showNesting = ((fpflags & maskShowNesting) != FALSE);
		textFilesOnly = ((fpflags & maskTextFilesOnly) != FALSE);
		if (textFilesOnly)
			filesOnly = TRUE;
		quotesNum = fpop->quotesStart;
		quotes = fpop->quotes[quotesNum];
	}
}

void DFilePaths::SetUpMenus (void)
{
	// NOTE:	This method is called AFTER ReadPrefs (see Dragon::Start � InitPrefs calls ReadPrefs)
	
	inherited::SetUpMenus ();
	
	optionsMenu = GetMenu (mOptions);
	InsertMenu (optionsMenu, 0);
	InitOptionsMenu ();
	
	DrawMenuBar ();
}

void DFilePaths::InitOptionsMenu (void)
{
	// Initialize options menu (and set the options while we're at it)
	CheckItem (optionsMenu, iFullPaths, fullPaths);
	CheckItem (optionsMenu, iShowNesting, showNesting);
	CheckItem (optionsMenu, iTextFilesOnly, textFilesOnly);
	SetQuotesOption (iNoQuotes + quotesNum);
}

void DFilePaths::SetQuotesOption (short optionItem)
{
	CheckOne (optionsMenu, iNoQuotes, iLastItem, optionItem);
	quotesNum = optionItem - iNoQuotes;
	if (fpOptions) {
		(*fpOptions)->quotesStart = quotesNum;
		quotes = (*fpOptions)->quotes[quotesNum];
	}
}

void DFilePaths::ToggleFullPaths (void)
{
	fullPaths = ToggleMenuItem (optionsMenu, iFullPaths);
	if (fpOptions) {
		if (fullPaths)
			(*fpOptions)->flags |= maskFullPaths;
		else
			(*fpOptions)->flags &= ~maskFullPaths;
	}
}

void DFilePaths::ToggleShowNesting (void)
{
	showNesting = ToggleMenuItem (optionsMenu, iShowNesting);
	if (fpOptions) {
		if (showNesting)
			(*fpOptions)->flags |= maskShowNesting;
		else
			(*fpOptions)->flags &= ~maskShowNesting;
	}
}

void DFilePaths::ToggleTextFilesOnly (void)
{
	textFilesOnly = ToggleMenuItem (optionsMenu, iTextFilesOnly);
	if (textFilesOnly) {
		if (fpOptions)
			(*fpOptions)->flags |= maskTextFilesOnly;
		filesOnly = TRUE;
	} else if (fpOptions)
		(*fpOptions)->flags &= ~maskTextFilesOnly;
}

void DFilePaths::DoMenu (long menuItemCode)
{
	short	menuID, itemNum;
	long		ticks;
	
	menuID = menuItemCode >> 16;
	itemNum = menuItemCode & 0xFFFF;
	
	if (menuID == mOptions)
		DoOptionsMenu (itemNum);
	else
		inherited::DoMenu (menuItemCode);
}

void DFilePaths::DoOptionsMenu (short itemNum)
{
	switch (itemNum) {
		case iFullPaths:
			ToggleFullPaths ();
			break;
		case iShowNesting:
			ToggleShowNesting ();
			break;
		case iTextFilesOnly:
			ToggleTextFilesOnly ();
			break;
		case iNoQuotes:
		case iSingleQuotes:
		case iDoubleQuotes:
		case iAngleBrackets:
		default:
			SetQuotesOption (itemNum);
			break;
	}
	preferences->SavePrefResource (prefFilePathOptions);		// Save any changes that were made
}

void DFilePaths::DoAbout (void)
{
	// I hope to eventually put something here�
}

void DFilePaths::AdjustMenusBusy (void)
{
	inherited::AdjustMenusBusy ();
	DisableItem (optionsMenu, 0);	// Disable the Options menu
}

void DFilePaths::AdjustMenusIdle (void)
{
	inherited::AdjustMenusIdle ();
	EnableItem (optionsMenu, 0);		// Enable the options menu
}

void DFilePaths::ProcessDirectory (void)
{
	if (!filesOnly)			// If we're not just supposed to process files,
		ProcessFile ();		//	treat directories just like files
}

void DFilePaths::ProcessDocsInDirectory (short vRefNum, long dirID)
{
	if (showNesting) {		// If we're supposed to return a hierarchical list,
		curDirDepth++;	//	then pop up temporarily and
		ProcessFile ();		//	put the directory name before its contents
		curDirDepth--;		//	and push back down
	}
	inherited::ProcessDocsInDirectory (vRefNum, dirID);
}

void DFilePaths::ProcessFile (void)
{
	register char	*p = *pathsHndl + pathsLen;
	long			len;
	OSErr		err;
	register short	n;
	
	if (showNesting) {
		for (n = curDirDepth; n < 0; n++)
			*p++ = cTAB;
		pathsLen -= curDirDepth;		// Subtract a negative to advance pathsLen in a positive direction
	}
	
	// Stick a left quote in front, if appropriate
	if (quotesNum != qNone) {
		*p++ = quotes.left;
		pathsLen++;
	}
	
	// Now copy the file name or full path, depending on the setting of the "Full Paths" option
	if (fullPaths) {
		len = GetFullPath (curDocFSS, p, &err);
		if (err != noErr) {				// Just skip this one if we had any problems
			if (quotesNum != qNone)		// Back up a byte if we have written the opening quote character
				pathsLen--;
			return;
		}
	} else
		len = (long) GetFileName (curDocFSS, p);
	p += len;
	pathsLen += len;
	
	// Stick a right quote at the end, if appropriate
	if (quotesNum != qNone) {
		*p++ = quotes.right;
		pathsLen++;
	}
	
	// Add a return character and check to see if we're running out of room in the pathsHndl block
	*p++ = cRETURN;
	if (++pathsLen > threshold) {
		if (pathsLen > limit)										// Have we overrun the end of the block?
			Abort (eOverranPathsBlock);							// If so, we should crash peacefully now
		else {
			blockLen += szGrowBy;								// Enlarge the block to accomodate more paths
			err = ResizeHandle (pathsHndl, blockLen);
			if (err != noErr)
				StopProcessing (memFullErr);
			else
				threshold = (limit = blockLen - 1) - szMaxPathLen;		// Reset limit and threshold accordingly
		}
	}
}

void DFilePaths::BeginProcessing (void)
{	
	inherited::BeginProcessing ();

	// Allocate a block big enough to contain a reasonable number of paths
	blockLen = szPathLenEstimate * nSeveralDocs + szMaxPathLen + szExtraRoom;
	pathsLen = 0L;
	pathsHndl = AnyHandle (blockLen);
	if (pathsHndl == NULL)
		StopProcessing (memFullErr);
	
	limit = blockLen - 1;
	threshold = limit - szMaxPathLen;
}

void DFilePaths::EndProcessing (void)
{
	OSErr	err;
	
	if (!abortProcessing) {
// BEGIN bug-fix 1.1.1 � Don't add a zero byte to the end, just chop off the last byte
		SetHandleSize (pathsHndl, pathsLen - 1);	// Remove the last byte (which was a cRETURN) of the paths
// END bug-fix
		err = HandleToScrap (pathsHndl, 'TEXT');
	}
	
	if (pathsHndl != NULL)
		DisposHandle (pathsHndl);
		
	inherited::EndProcessing ();
}

Boolean DFilePaths::CustomFilterDoc (void)
{
	if (textFilesOnly)
		return curDocIsFile ? (curFileType == 'TEXT') : (dirDepthLimit < 0);
	else
		return TRUE;
}

Dragon *CreateGDragon (void)
{
	return (Dragon *) new DFilePaths;
}

short GetFileName (register FSSpec *fsspec, register char *name)
{
	short					len, count;
	register unsigned char		*p;
	
	p = (unsigned char *) fsspec->name;
	count = len = *p++;
	while (count-- != 0)
		*name++ = *p++;
	return len;
}

long GetFullPath (FSSpec *fsspec, char *path, OSErr *err)
{
	short		volume = fsspec->vRefNum;
	OSErr		fsErr = noErr;
	CInfoPBRec	catinfo;
	char			dirName[64];
	long			retLen = 0L, len;
	register char	*p = path;
	
	// All comments assume a  file whose full path is
	//	HD:Fonts:Garamond
	
	// If fsspec doesn't designate a file (i.e., it's a volume or directory), we write a colon
	if ( ! FSpIsFile (fsspec)) {
		*p++ = cCOLON;
		retLen++;
	}
	
	// First, copy the file name backwards from a Pascal to a C string:
	//	"\pGaramond"  becomes "dnomaraG"

	len = (long) ReverseCopyP2CStr (fsspec->name, p);
	p += len;
	retLen += len;
	
	if ( ! FSpIsVolume (fsspec)) {		// Don't do anything more if we've got a volume
	
		catinfo.dirInfo.ioVRefNum = volume;
		catinfo.dirInfo.ioNamePtr = (StringPtr) dirName;
		catinfo.dirInfo.ioDrParID = fsspec->parID;
		
		// Now copy the rest of the path, one level at a time, backwards:
		//	"dnomaraG:stnoF:DH"
		do {
			catinfo.dirInfo.ioFDirIndex = -1;
			catinfo.dirInfo.ioDrDirID = catinfo.dirInfo.ioDrParID;	// <= This is the key � go from the
														//	current folder to its parent
			fsErr = PBGetCatInfoSync (&catinfo);
			if (fsErr == noErr) {
				*p++ = cCOLON;
				retLen += 1 + (len = ReverseCopyP2CStr ((StringPtr) dirName, p));
				p += len;
			} else {
				*err = fsErr;
				return retLen;
			}
		} while (catinfo.dirInfo.ioDrDirID != fsRtDirID);
	}
	
	// Finally, reverse the string and return its length and any error that might have occurred (ReverseCopyP2CStr has
	//	already appended a null character, so we don't have to worry about it here)
	ReverseCStr (path);
	*err = fsErr;
	return retLen;
}

unsigned short ReverseCopyP2CStr (register unsigned char *pas, register char *c)
{
	register short	i, len = *pas++;
	
	for (i = len, c += len; i > 0; i--)
		*--c = *pas++;
	c += len;
	*c = cNIL;
	return len;
}

void ReverseCStr (register char *str)
{
	register short		n = 0L, i;
	register char		t, *p1, *p2;
	
	p1 = p2 = str;
	while (*p2++)
		;
	p2--;
	n = (p2 - p1) / 2;
	for (i = n; i > 0; i--) {
		t = *--p2;
		*p2 = *p1;
		*p1++ = t;
	}
}

Boolean FSpIsFile (FSSpec *fss)
{
	CInfoPBRec	catinfo;
	OSErr		err;
	
	if (FSpIsVolume (fss))
		return FALSE;
	catinfo.hFileInfo.ioNamePtr = fss->name;
	catinfo.hFileInfo.ioVRefNum = fss->vRefNum;
	catinfo.hFileInfo.ioFDirIndex = 0;
	catinfo.hFileInfo.ioDirID = fss->parID;
	err = PBGetCatInfoSync (&catinfo);
	return ! (catinfo.hFileInfo.ioFlAttrib & ioDirMask);		// Bit 4 indicates it's a folder (or volume)
}

Boolean FSpIsVolume (FSSpec *fss)
{
	return (fss->parID == fsRtParID);		// I guess this is what this constant is for �
}

