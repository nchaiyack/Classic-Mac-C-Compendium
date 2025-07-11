/*
	FileUtils.h
*/

#pragma once

#include	<Files.h>
#include	<Aliases.h>
#include	<Folders.h>

#define	kInvalidRefNum	-1

enum {
		kInvalidFork,
		kResFork,
		kDataFork,
		kBothForks
};

typedef union {
	ParamBlockRec	p;
	HParamBlockRec	h;
	CInfoPBRec		c;
	CMovePBRec		cmove;
	WDPBRec		wd;
	FCBPBRec		fcb;
} PBRecUnion;

// Tired of twisting and turning through unions and structs in the param block? � use these macros
// Remember � garbage in, garbage out � so use them with discretion, only after calling one of the PB routines
//	that return valid data, and only after checking for an error
// The numbers given in the comments on the right are the offsets to the pertinent fields

// Macros that are valid ONLY for CInfoPBRec's and must follow a call to PBGetCatInfo �

	#define	PBParID(pb)			(((CInfoPBRec *) pb)->hFileInfo.ioFlParID)						// 100
	#define	PBIsVolume(pb)		(((CInfoPBRec *) pb)->hFileInfo.ioFlParID == fsRtParID)			// 100
	#define	PBIsAliasFile(pb)		(!(((HParmBlkPtr) pb)->fileParam.ioFlAttrib & ioDirMask) \
								&& (((CInfoPBRec *) pb)->hFileInfo.ioFlFndrInfo.fdFlags & 0x8000))	// 30, 32 + 8

// Valid for CInfoPBRec's and HParamBlockRec's �

	// Must follow a call to PBGetCatInfo or PB(H)GetFileInfo �
	
		// Valid for files and directories �
		
			#define	PBCreated(pb)		(((HParmBlkPtr) pb)->fileParam.ioFlCrDat)				// 72
			#define	PBModified(pb)		(((HParmBlkPtr) pb)->fileParam.ioFlMdDat)				// 76
			#define	PBIsDirectory(pb)		(((HParmBlkPtr) pb)->fileParam.ioFlAttrib & ioDirMask)		// 30
			#define	PBIsFile(pb)			!(((HParmBlkPtr) pb)->fileParam.ioFlAttrib & ioDirMask)	// 30
		
		// Valid only for files �
		
			#define	PBFileType(pb)		(((HParmBlkPtr) pb)->fileParam.ioFlFndrInfo.fdType)		// 32 + 0
			#define	PBFileCreator(pb)		(((HParmBlkPtr) pb)->fileParam.ioFlFndrInfo.fdCreator)	// 32 + 4
			#define	PBFileFlags(pb)		(((HParmBlkPtr) pb)->fileParam.ioFlFndrInfo.fdFlags)		// 32 + 8
			#define	PBDataForkSize(pb)	(((HParmBlkPtr) pb)->fileParam.ioFlLgLen)				// 54
			#define	PBResForkSize(pb)	(((HParmBlkPtr) pb)->fileParam.ioFlRLgLen)				// 64

	// Do not need to follow PBGetCatInfo or PB(H)GetFileInfo �
	
		#define	PBVolRefNum(pb)		(((HParmBlkPtr) pb)->fileParam.ioVRefNum)					// 22
		#define	PBName(pb)			(((HParmBlkPtr) pb)->fileParam.ioNamePtr)					// 18

typedef struct {
	short	numTypes;
	OSType	type[];
} *TypeListPtr, **TypeListHndl;

void FSpRefreshFinderDisplay (FSSpec *fss);

OSErr FSpOpenDataFork (FSSpec *fss, short *refNum, char *perm);
OSErr FSpOpenResFork (FSSpec *fss, short *refNum, char *perm);

OSErr FSpFindFolder (OSType folderType, FSSpec *fss);

OSErr FSpToPB (PBRecUnion *pb, FSSpec *fss, Boolean resolveAFs, Boolean followAFChain, Boolean *wasAF);
OSErr FSpToPBCatInfo (PBRecUnion *pb, FSSpec *fss, Boolean resolveAFs, Boolean followAFChain, Boolean *wasAF);
OSErr PBToFSpCatInfo (PBRecUnion *pb, FSSpec *fss, Boolean resolveAFs, Boolean followAFChain, Boolean *wasAF);

TypeListHndl FREFTypes (short resFork);
Boolean OpenableType (OSType fileType, TypeListHndl openableTypesHndl);
OSErr RefNumToFSSpec (short refNum, FSSpec *fss, Boolean *isResFork);

typedef Boolean (*DCLActionProc) (CInfoPBPtr pb, long refCon);
OSErr DirectoryContentsLoop (short vRefNum, long dirID, DCLActionProc actionProc, Boolean skipOnError, long refCon);

