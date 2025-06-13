/* Error.c *** error routines.
	by David Phillip Oster October 1994 oster@netcom.com
	for:
	Stuart Inglis singlis@waikato.ac.nz
	Department of Computer Science
	University of Waikato, Hamilton, New Zealand

 */
#include "RandomDotMain.h"
#include "RandomDotRes.h"
#include "Error.h"

/* Error strings
 */
enum{
	kGenericErrS = 1,
	kDirFulErr,		/*Directory full*/
	kDskFulErr,		/*disk full*/
	kNsvErr,		/*no such volume*/
	kIOErr,			/*I/O error*/
	kBdNamErr,		/*there may be no bad names in the final system!*/
	kFnOpnErr,		/*File not open*/
	kEofErr,		/*End of file*/
	kPosErr,		/*tried to position to before start of file (r/w)*/
	kMFulErr,		/*memory full (open) or file won't fit (load)*/
	kTmfoErr,		/*too many files open*/
	kFnfErr,		/*File not found*/
	kWPrErr,		/*diskette is write protected.*/
	kFLckdErr,		/*file is locked*/
	kVLckdErr,		/*volume is locked*/
	kFBsyErr,		/*File is busy (delete)*/
	kDupFNErr,		/*duplicate filename (rename)*/
	kOpWrErr,		/*file already open with with write permission*/
	kRfNumErr,		/*refnum error*/
	kGfpErr,		/*get file position error*/
	kVolOffLinErr,	/*volume not on line error (was Ejected)*/
	kPermErr,		/*permissions error (on file open)*/
	kVolOnLinErr,	/*drive volume already on-line at MountVol*/
	kNsDrvErr,		/*no such drive (tried to mount a bad drive num)*/
	kNoMacDskErr,	/*not a mac diskette (sig bytes are wrong)*/
	kExtFSErr,		/*volume in question belongs to an external fs*/
	kFsRnErr,		/*file system internal error:during rename the old entry was deleted but could not be restored.*/
	kBadMDBErr,		/*bad master directory block*/
	kWrPermErr,		/*write permissions error*/
	kDirNFErr,		/*Directory not found*/
	kTmwdoErr,		/*No free WDCB available*/
	kBadMovErr,		/*Move into offspring error*/
	kWrgVolTypErr,	/*Wrong volume type error [operation not supported for MFS]*/
	kVolGoneErr,	/*Server volume has been disconnected.*/
	kMemFullError,	/*Insufficent memory to complete operation.*/
	kPortInUse,		/* That port appears to be in use by another program. */
	kClosedPreferences,	/* the preferences file is not available */
	kBadPict,		/* This Pict file seems to be damaged */
	kBadFileType,	/* I don't know how to open this kind of file */
	kBadPGMMagicNumber,	/* This pgm file seems to be damaged */
	kPictOnly,		/* I can only replace a Pict file */
	kNeed7AndColor	/* This program needs Color Quickdraw and System 7 or later */
};

/* GetErrorStringCode - translate system error code to string index
 */
static Integer GetErrorStringCode(OSErr errCode){
	switch(errCode){
	case noErr:
	case eUserCancel:	return 0;
	case dirFulErr:		return kDirFulErr;		/*Directory full*/
	case dskFulErr:		return kDskFulErr;		/*disk full*/
	case nsvErr:		return kNsvErr;			/*no such volume*/
	case ioErr:			return kIOErr;			/*I/O error*/
	case bdNamErr:		return kBdNamErr;		/*there may be no bad names in the final system!*/
	case fnOpnErr:		return kFnOpnErr;		/*File not open*/
	case eofErr:		return kEofErr;			/*End of file*/
	case posErr:		return kPosErr;			/*tried to position to before start of file (r/w)*/
	case mFulErr:		return kMFulErr;		/*memory full (open) or file won't fit (load)*/
	case tmfoErr:		return kTmfoErr;		/*too many files open*/
	case fnfErr:		return kFnfErr;			/*File not found*/
	case wPrErr:		return kWPrErr;			/*diskette is write protected.*/
	case fLckdErr:		return kFLckdErr;		/*file is locked*/
	case vLckdErr:		return kVLckdErr;		/*volume is locked*/
	case fBsyErr:		return kFBsyErr;		/*File is busy (delete)*/
	case dupFNErr:		return kDupFNErr;		/*duplicate filename (rename)*/
	case opWrErr:		return kOpWrErr;		/*file already open with with write permission*/
	case rfNumErr:		return kRfNumErr;		/*refnum error*/
	case gfpErr:		return kGfpErr;			/*get file position error*/
	case volOffLinErr:	return kVolOffLinErr;	/*volume not on line error (was Ejected)*/
	case permErr:		return kPermErr;		/*permissions error (on file open)*/
	case volOnLinErr:	return kVolOnLinErr;	/*drive volume already on-line at MountVol*/
	case nsDrvErr:		return kNsDrvErr;		/*no such drive (tried to mount a bad drive num)*/
	case noMacDskErr:	return kNoMacDskErr;	/*not a mac diskette (sig bytes are wrong)*/
	case extFSErr:		return kExtFSErr;		/*volume in question belongs to an external fs*/
	case fsRnErr:		return kFsRnErr;		/*file system internal error:during rename the old entry was deleted but could not be restored.*/
	case badMDBErr:		return kBadMDBErr;		/*bad master directory block*/
	case wrPermErr:		return kWrPermErr;		/*write permissions error*/
	case dirNFErr:		return kDirNFErr;		/*Directory not found*/
	case tmwdoErr:		return kTmwdoErr;		/*No free WDCB available*/
	case badMovErr:		return kBadMovErr;		/*Move into offspring error*/
	case wrgVolTypErr:	return kWrgVolTypErr;	/*Wrong volume type error [operation not supported for MFS]*/
	case volGoneErr: 	return kVolGoneErr;		/*Server volume has been disconnected.*/
	case memFullErr:	return kMemFullError;	
	case portInUse:		return kPortInUse;		/* That port appears to be in use by another program. */
	case eClosedPreferences:return kClosedPreferences;	/* the preferences file is not available */
	case eBadPict:		return kBadPict;		/* This Pict file seems to be damaged */
	case eBadFileType:	return kBadFileType;	/* I don't know how to open this kind of file */
	case eBadPGMMagicNumber:	return kBadPGMMagicNumber;	/* This pgm file seems to be damaged */
	case ePictOnly:		return kPictOnly;		/* I can only replace a Pict file */
	case eNeed7AndColor:return kNeed7AndColor;	/* This program needs Color Quickdraw and System 7 or later */
	default:			return kGenericErrS;	
	}
}

/* TellError - if an error occurred, report it to the user.
 */
OSErr TellError(OSErr errCode){
	Str255	errS, s;
	Integer	errN;

	if(0 == (errN = GetErrorStringCode(errCode))){
		return errCode;
	}
	GetIndString(errS, kErrorStrs, errN);
	NumToString(errCode, s);
	ParamText(errS, s, emptyS, emptyS);
	Alert(rError, NIL);
	return errCode;
}
