#include "Global.h"
#include "InitGraf.h"
#include "Conversions.h"	// For ReplyToFSSpec
#include "Pack3.h"			// For gOffsetToReply
#include "Remember.h"		// For RememberFileAndParent, and RememberFileObject
#include "Utilities.h"		// For ReportError

#include <Traps.h>

typedef pascal void	(*InitGrafProc)(Ptr);

UniversalProcPtr	gOldInitGraf;

UniversalProcPtr	pOldPack3;
static long			pSavedA4;				// static because also in Pack3.c
static long			pSavedReturnAddress;	// static because also in Pack3.c
long				pReplyPtr;

void				ReadAppFiles(void);
void				InstallPack3FrontPatch(void);

pascal void			Pack3Remember(void);
void				RememberParameters(short selector);
void				RememberMFReply(void);


//--------------------------------------------------------------------------------

pascal void	MyInitGraf(Ptr grafPort)
{
	OSErr			err;
	FCBPBRec		pbFCB;
	Str63			name;
	Boolean			install;
	FSSpec			spec;
	WDPBRec			pb;
	CInfoPBRec		catpb;
	
	#ifdef BREAK_ON_ENTRY
	DebugStr("\pBreaking on MyInitGraf");
	#endif

	SetUpA4();

	((InitGrafProc) gOldInitGraf)(grafPort);

	install = FALSE;
	pbFCB.ioFCBIndx = 0;
	pbFCB.ioVRefNum = 0;
	pbFCB.ioRefNum = CurResFile();
	pbFCB.ioNamePtr = name;

	err = PBGetFCBInfoSync(&pbFCB);

	if (err == noErr) {

		catpb.hFileInfo.ioNamePtr = name;
		catpb.hFileInfo.ioVRefNum = pbFCB.ioVRefNum;
		catpb.hFileInfo.ioFDirIndex = 0;
		catpb.hFileInfo.ioDirID = pbFCB.ioFCBParID;
		err = PBGetCatInfoSync(&catpb);
		
		if (err == noErr) {
			install = (catpb.hFileInfo.ioFlFndrInfo.fdType == kApplicationSig);
		}
	}


	if (install) {
		pb.ioNamePtr = NIL;
		
		if (PBHGetVolSync(&pb) == noErr) {
			FSMakeFSSpec(pb.ioWDVRefNum, pb.ioWDDirID, NIL, &spec);
			RememberFileObject(&spec, gDirectoryList);
		}
		ReadAppFiles();
		InstallPack3FrontPatch();
	}
	
	RestoreA4();
}


//--------------------------------------------------------------------------------

void	ReadAppFiles()
{
	short		message;
	short		count;
	AppFile		theFile;
	FSSpec		spec;
	
	CountAppFiles(&message, &count);
	if (message == appOpen) {
		while (count > 0) {
			GetAppFiles(count, &theFile);
			FSMakeFSSpec(theFile.vRefNum, 0, theFile.fName, &spec);
			RememberFileAndParent(&spec);
			count--;
		}
	}
}


//--------------------------------------------------------------------------------

void	InstallPack3FrontPatch()
{
	UniversalProcPtr	currentAddress;
	
	currentAddress = GetToolTrapAddress(_Pack3);
	if (currentAddress != (UniversalProcPtr) Pack3Remember) {
		pOldPack3 = currentAddress;
		SetToolTrapAddress((UniversalProcPtr) Pack3Remember, _Pack3);
	} else {
		ReportError(kReinstallingPack3, 0);
	}
}


//--------------------------------------------------------------------------------

pascal void Pack3Remember()
{
	SetUpA4();
	asm {
		move.l	(sp)+,pSavedA4
		move.l	(sp)+,pSavedReturnAddress
		move.w	(sp),D0
		beq.s	@unknownSelector
		cmp.w	#8,D0
		bhi.s	@unknownSelector
		
		bsr		RememberParameters
		move.l	pOldPack3,A0
		jsr		(A0)
		bsr		RememberMFReply
		bra.s	@continue

	@unknownSelector
		move.l	pOldPack3,A0
		jsr		(A0)

	@continue
		move.l	pSavedReturnAddress,-(sp)
		move.l	pSavedA4,A4
	}
}


//--------------------------------------------------------------------------------

void	RememberParameters(short selector)
{
	gSelector = selector;
	pReplyPtr = *(long*) ((char*)&selector + gOffsetToReply[selector-1]);
	
	gScarfedItYet = FALSE;
}


//--------------------------------------------------------------------------------

void	RememberMFReply()
{
	FSSpec		spec;
	char*		namePtr;
	
	if (!gScarfedItYet) {
		if (((SFReply*) pReplyPtr)->good) {
			namePtr = (char*) (IsNewCall(gSelector)
				? &((StandardFileReply*) pReplyPtr)->sfFile.name
				: &((SFReply*) pReplyPtr)->fName);

			if (StrLength(namePtr) != 0) {
				ReplyToFSSpec((void*) pReplyPtr, &spec);
				RememberFileAndParent(&spec);
			} else {
				RememberParentDirectory(&spec);
			}
		}
	}
}
