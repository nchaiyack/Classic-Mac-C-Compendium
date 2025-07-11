/*
	DFileCleaver.c
	
	A Dragon subspecies developed using THINK C

	Splits text files into smaller segments.
	
	NOTE:	This sub-Dragon is in development.  I wrote it to do a very specific task: split up dragonsmith10b2.hqx
			into files of 24800 bytes for mailing to info-mac@sumex-aim.stanford.edu (my local mail system wouldn't
			send anything > 25000 bytes long).
			
			There's a lot of room for improvement � preferences handling in particular.  You might notice that it's being
			designed to split files in more than one way, but the byte-count method is all that I've implemented so far.
			
			Things that need work are marked PMH.
	
	Copyright � 1992�1994 by Paul M. Hoffman
	Send comments or suggestions to paul.hoffman@umich.edu -or- dragonsmith@umich.edu
	
	There are no restrictions on the use or distribution of applications developed using Dragonsmith.
	This source code may be freely used, altered, and distributed in any way as long as:
		1.	It is GIVEN away rather than sold (unless expressly permitted by the author)
		2.	These statements and the above copyright notice are left intact.

*/

#include	"Dragon.h"
#include	"FileUtils.h"
#include	"MenuUtils.h"
#include	"StringUtils.h"
#include	"HandleUtils.h"
#include	<Types.h>

#define	cRETURN	'\r'
#define	cNIL			'\0'
#define	cCOLON		':'
#define	cELLIPSIS	'�'		// Yes, that's one char (option-semicolon)

enum {
	mOptions = mEdit + 1
};

enum {
	iSplitAt = 1,
	iSegmentSize
};

// Structure of the DFileCleaver preferences resource ('SSiz' 128)
typedef struct {
	short	splitType;
	short	numSegs;
	long		segSize;
	Boolean	filler;
	Boolean	useAfterText;
	Boolean	useAtText;
	Boolean	useBeforeText;
} SplitStrategy;

// Possible values for splitType
enum {
	kSplitByNumSegs,
	kSplitBySegSize,
	kSplitByTextMatch
};

enum {
	prefSplitStrategy = 1,		// Split strategy preferences ID
	prefSplitAfterText,			// Text AFTER which a split may be made
	prefSplitAtText,			// Text AT which a split may be made
	prefSplitBeforeText			// Text BEFORE which a split may be made
};

#define	rAskSegSize		130		// ID of the "Segment Size�" dialog
#define	iSegSizeText		4		// Item number of the edit text item containing the segment size

class DFileCleaver: public Dragon {

	protected:
		Boolean		askForSpecs;
		SplitStrategy	**strategy;
		short		splitType;
		short		numSegments;
		long			segmentSize;
		Handle		buffer;			// Read & write buffer
		long			bufferSize;

		MenuHandle	optionsMenu;
		
	public:
						DFileCleaver (void);
	
	protected:
		virtual void		BeginProcessing (void);		// Override
		virtual void		ReadPrefs (void);			// Override
		virtual void		ProcessFile (void);

		virtual void		SetUpMenus (void);
		virtual void		DoMenu (long menuItemCode);
		virtual void		DoOptionsMenu (short itemNum);
		virtual void		AdjustMenusBusy (void);
		virtual void		AdjustMenusIdle (void);

		virtual OSErr		SplitFile (FSSpec *srcFile);
		virtual OSErr		ExtractSegFile (FSSpec *srcFile, short srcRefNum, short segNum);
		virtual OSErr		CreateSegFile (FSSpec *fss, unsigned char *sourceName, short segNum);
		virtual void		MakeSegFileName (unsigned char *sourceName, short segNum);
		virtual OSErr		CopyBetweenFiles (short source, short dest);
		virtual OSErr		CopyFileRange (short source, short dest, long rangeSize);
		virtual OSErr		CopyByTextMatch (short source, short dest);
		virtual Boolean	OKToReplace (FSSpec *fss);
		virtual Boolean	SplitSpecsInvalid (void);
		virtual Boolean	AskForSplitSpecs (void);
		virtual Boolean	AskForSegmentSize (long *num);
		virtual Boolean	SplitFailed (FSSpec *sourceFss, OSErr err);
};

Dragon *CreateGDragon (void)
{
	return (Dragon *) new DFileCleaver;
}

DFileCleaver::DFileCleaver (void)
{
	autoQuit = FALSE;			// This will be overridden by the 'DrPr' prerefence resource
	dirDepthLimit = 0;			// Ditto

	askForSpecs = TRUE;		// Should this be set in preferences->Init??
	splitType = kSplitBySegSize;
	numSegments = 0;
	segmentSize = 24800L;		// Last-ditch default
	strategy = NULL;

	buffer = NULL;
	bufferSize = 0L;
	
	optionsMenu = NULL;
}

void DFileCleaver::ReadPrefs (void)
{
	inherited::ReadPrefs ();
	
	strategy = (SplitStrategy **) preferences->GetPrefResource (prefSplitStrategy);
	if (strategy != NULL) {
		HNoPurge ((Handle) strategy);
		segmentSize = (*strategy)->segSize;		// PMH � we ignore everything else in the struct!!
	}
	askForSpecs = SplitSpecsInvalid ();		// PMH � get askForSpecs from the prefs file?
}

void DFileCleaver::SetUpMenus (void)
{
	inherited::SetUpMenus ();			// Add the Apple, File, and Edit menus

	optionsMenu = GetMenu (mOptions);
	InsertMenu (optionsMenu, 0);
	
	DrawMenuBar ();
}

void DFileCleaver::DoMenu (long menuItemCode)
{
	short	menuID, itemNum;

	menuID = menuItemCode >> 16;
	itemNum = menuItemCode & 0xFFFF;

	if (menuID == mOptions)
		DoOptionsMenu (itemNum);
	else
		inherited::DoMenu (menuItemCode);
}

void DFileCleaver::DoOptionsMenu (short itemNum)
{
	switch (itemNum) {
		case iSplitAt:
			// PMH � we need to put something here (obviously)
			break;
		case iSegmentSize:
			(void) AskForSplitSpecs ();	// PMH � need to rearrange, rename methods
			break;
		default:
			break;
	}
}

void DFileCleaver::AdjustMenusBusy (void)
{
	inherited::AdjustMenusBusy ();
	DisableItem (optionsMenu, 0);	// Disable the Options menu
}

void DFileCleaver::AdjustMenusIdle (void)
{
	inherited::AdjustMenusIdle ();
	EnableItem (optionsMenu, 0);		// Enable the options menu
}

void DFileCleaver::BeginProcessing (void)
{
	Boolean		ok = TRUE;

	// Make sure the specifications for the split(s) have been set
	if (askForSpecs || SplitSpecsInvalid ())
		ok = AskForSplitSpecs ();
	if (!ok)
		StopProcessing (userCanceledErr);
		
	inherited::BeginProcessing ();
}

void DFileCleaver::ProcessFile (void)
{
	OSErr	err;
	
	if ((err = SplitFile (curDocFSS)) != noErr && SplitFailed (curDocFSS, err))
		StopProcessing (err);
}

OSErr DFileCleaver::SplitFile (FSSpec *srcFile)
{
	short	srcRefNum, destRefNum;
	OSErr	err;
	short	segNum;
	long		actualSize;
	Boolean	ok;

	// If the file is smaller than the segment size, there's no need to split it
	if (PBDataForkSize (curDocPB) <= segmentSize)
		return noErr;		// PMH � or should we return an error?
			
	err = FSpOpenDF (srcFile, fsRdPerm, &srcRefNum);
	if (err == noErr) {
		bufferSize = segmentSize;
		if (!ok) return -1;		// PMH � impromptu error code
		
		splitType = 0;			// PMH � hard-coded garbage
		if (!ok) return -1;		// PMH � impromptu error code
		actualSize = bufferSize;
		buffer = BigHandle (&actualSize);				// Returns the actual block size in actualSize
		bufferSize = actualSize;
		segNum = 1;
		do
			err = ExtractSegFile (srcFile, srcRefNum, segNum++);
		while (err == noErr);
		FSClose (srcRefNum);
		DisposHandle (buffer);
	}
	return (err == eofErr) ? noErr : err;
}

OSErr DFileCleaver::ExtractSegFile (FSSpec *srcFile, short srcRefNum, short segNum)
{
	FSSpec	destFile;
	short	destRefNum;
	OSErr	err, copyErr;
	
	DoBusy ();							// Give other processes a little time between segments
	
	destFile.vRefNum = srcFile->vRefNum;		// PMH � leave segments in the same place
	destFile.parID = srcFile->parID;			//	as the file we're splitting
	
	// Make a file for the next segment, basing its name on that of the original
	err = CreateSegFile (&destFile, (unsigned char *) &srcFile->name, segNum);
	if (err == noErr) {
		err = FSpOpenDF (&destFile, fsWrPerm, &destRefNum);
		if (err == noErr) {
			err = copyErr = CopyBetweenFiles (srcRefNum, destRefNum);
			if (copyErr == noErr || copyErr == eofErr) {
				err = FSClose (destRefNum);
				(void) FlushVol (NULL, destFile.vRefNum);		// Ignore any errors � what would we do??
			}
		}
		if (err != noErr)
// BEGIN bug-fix 1.1.1 � Change pointer to the destination file's refnum to a pointer to its FSSpec!!
			(void) FSpDelete (&destFile);
// END bug-fix 1.1.1
	}
	return (err == noErr) ? copyErr : err;
}

OSErr DFileCleaver::CreateSegFile (FSSpec *fss, unsigned char *sourceName, short segNum)
{
	OSErr	err;
	Boolean	ok;
	
	CopyPStr (sourceName, fss->name);
	MakeSegFileName ((unsigned char *) &fss->name, segNum);
	err = FSpCreate (fss, curFileCreator, curFileType, smSystemScript);
	if (err == dupFNErr) {
		ok = OKToReplace (fss);
		if (ok) {
			err = FSpDelete (fss);
			if (err == noErr)
				err = FSpCreate (fss, curFileCreator, curFileType, smSystemScript);
		}
	}
	return err;
}

void DFileCleaver::MakeSegFileName (unsigned char *sourceName, short segNum)
{
	#define	maxShortString	10		// "\p32767" => 5 digits would be the longest
									//	in USA, but other scripts might produce
									//	a bigger string from a short, so we leave
									//	a little extra room
	#define	maxFileName		63		// Make allowances for MFS (yeah, yeah, OK)
	
	unsigned char		segNumStr [maxShortString];
	unsigned short	len;
	unsigned char		partString [] = "\p-pt";	// PMH � hard-coded string
	unsigned char		chopLen;
	
	chopLen = maxFileName - partString [0] - maxShortString;
	if (sourceName [0] > chopLen) {
		sourceName [0] = chopLen;
		sourceName [chopLen] = cELLIPSIS;
	}
	AppendPStr (sourceName, partString);
	NumToString (segNum, segNumStr);
	AppendPStr (sourceName, segNumStr);
}

OSErr DFileCleaver::CopyBetweenFiles (short source, short dest)
{
	Boolean	ok;
	
	if (splitType == -1)								// PMH � hard-coded garbage
		return CopyByTextMatch (source, dest);		// PMH � change this?
	else
		return CopyFileRange (source, dest, segmentSize);
}

OSErr DFileCleaver::CopyFileRange (short source, short dest, long rangeSize)
{
	OSErr	err = noErr, copyErr = noErr;
	long		copySize, bytesToCopy;
	
	copySize = bufferSize;
	for (bytesToCopy = rangeSize; bytesToCopy > 0L && err == noErr && copyErr != eofErr; bytesToCopy -= copySize) {
		if (copySize > bytesToCopy)
			copySize = bytesToCopy;
		err = copyErr = FSRead (source, &copySize, *buffer);
		if (err == noErr || err == eofErr)
			err = FSWrite (dest, &copySize, *buffer);
	}
	return (err == noErr) ? copyErr : err;
}

OSErr DFileCleaver::CopyByTextMatch (short source, short dest)
{
	// Not implemented yet
}

Boolean DFileCleaver::OKToReplace (FSSpec *fss)
{
	return TRUE;	// PMH � always replace
}

Boolean DFileCleaver::AskForSplitSpecs (void)
{
	OSErr	err;
	Boolean	ok;
	long		sizeReplied = segmentSize;
	
	ok = AskForSegmentSize (&sizeReplied);
	if (ok && sizeReplied != 0L && sizeReplied != segmentSize) {	// Did the user change it (to a non-zero value)?
		if (strategy!= NULL) {
			(*strategy)->segSize = segmentSize = sizeReplied;
			preferences->SavePrefResource (prefSplitStrategy);
		}
		askForSpecs = FALSE;								// Don't need to ask again
	}
	return ok;
}

Boolean DFileCleaver::AskForSegmentSize (long *num)
{
	DialogPtr		dialog;
	short		itemHit, itemType;
	Handle		itemHndl;
	Rect			itemRect;
	Str255		numString;
	Boolean		done;
	
	if (num == NULL || *num < 0)
		return FALSE;
	
	if (!InteractWithUser (60*60)) {
		StopProcessing (userCanceledErr);
		return FALSE;
	}
	
	dialog = GetNewDialog (rAskSegSize, NULL, (WindowPtr) -1);
	if (dialog == NULL)
		return FALSE;
		
	NumToString (*num, numString);
	
	GetDItem (dialog, iSegSizeText, &itemType, &itemHndl, &itemRect);
	SetIText (itemHndl, numString);
	SelIText (dialog, iSegSizeText, 0, 32767);
	
	ShowWindow ((WindowPtr) dialog);

	do {
		do
			ModalDialog (NULL, &itemHit);
		while (itemHit != ok && itemHit != cancel);
		
		done = TRUE;			// User clicked on Cancel or OK (or pressed <Return> or <Enter>)
		if (itemHit == ok) {
			GetIText (itemHndl, numString);
			done = PStrToULong (numString, num);		// PStrToULong returns FALSE if it's not a valid unsigned long
		}
	} while (!done);
	
	HideWindow ((WindowPtr) dialog);
	DisposDialog (dialog);
	
	return (itemHit != cancel);
}

Boolean DFileCleaver::SplitSpecsInvalid (void)
{
	switch (splitType) {
		case kSplitByNumSegs:
			return TRUE;
			break;
		case kSplitBySegSize:			// PMH � we only do kSplitBySegSize
			if (segmentSize < 1)
				return TRUE;
			break;
		case kSplitByTextMatch:
		default:
			return TRUE;
	}
	
	// If we reach this point, then the specs are not invalid
	return FALSE;
}

Boolean DFileCleaver::SplitFailed (FSSpec *sourceFss, OSErr err)
{
	// Display an alert telling the user that there was a problem and asking if they want to continue.  Return TRUE if
	//	they chose "Stop" or FALSE if they want to continue splitting the remaining files
	
	return FALSE;		// PMH � I'd really like to put something more sophisticated here�
}
