#include "Conversions.h"
#include "Global.h"			// IsNewCall
#include "Pack3.h"			// gSelector
#include "PLStringFuncs.h"	// PStringCopy
#include "Utilities.h"		// NewStringSys
#include <Script.h>			// smSystemScript


Boolean				MacEasyOpenInstalled(Ptr entry);

//--------------------------------------------------------------------------------
/*
	Copies a FileObjectHdl to a FSSpec.
*/

OSErr	HandleToFSSpec(FileObjectHdl objHandle, FSSpec* spec)
{
	return FSMakeFSSpec(GetVolRefNum(objHandle), (**objHandle).parID,
							*(**objHandle).fName, spec);
}


//--------------------------------------------------------------------------------
/*
	Copies the vol/dir/name of a FSSpec to an existing FileObjectHdl.
	
	еее THINK C currently generates code such that the assignments to
		fields of an unlocked handle are OK. Is it OK so assume this?
*/

void	FSSpecToHandle(FSSpec* spec, FileObjectHdl objHandle)
{
	(**objHandle).volIndex = RememberVolume(spec->vRefNum);
	(**objHandle).parID = spec->parID;
	(**objHandle).fName = NewStringSys(spec->name);
}


//--------------------------------------------------------------------------------
/*
	Copies a reply record to a FSSpec.
*/

void	ReplyToFSSpec(void* reply, FSSpec* spec)
{
	if (IsNewCall(gSelector))
	{
		*spec = ((StandardFileReply*) reply)->sfFile;
	}
	else
	{
		(void) FSMakeFSSpec(((SFReply*) reply)->vRefNum, 0,
								((SFReply*) reply)->fName, spec);
	}
}


//--------------------------------------------------------------------------------
/*
	Copies the vol/dir/name of a FSSpec to a reply record (creating WD if
	necessary).
*/

void	FSSpecToReply(FSSpec* spec, void* reply)
{
	SFReply*			sfReplyPtr;

	if (IsNewCall(gSelector))
	{
		((StandardFileReply*) reply)->sfFile = *spec;
	}
	else
	{
		OSErr		err;
		WDPBRec		wdpb;

		sfReplyPtr = (SFReply*) reply;

		wdpb.ioNamePtr = NIL;
		wdpb.ioVRefNum = spec->vRefNum;
		wdpb.ioWDProcID = 'ERIK';
		wdpb.ioWDDirID = spec->parID;
		err = PBOpenWDSync(&wdpb);
		if (err == noErr)
		{
			sfReplyPtr->vRefNum = wdpb.ioVRefNum;
			PStringCopy(sfReplyPtr->fName, spec->name);
		}
		else
		{
			ReportError(kCreatingWD, err);
		}
	}
}


//--------------------------------------------------------------------------------
/*
	Copies a reply record to a FileObjectHdl
*/

void	ReplyToHandle(void* reply, FileObjectHdl objHandle)
{
	FSSpec	spec;
	
	ReplyToFSSpec(reply, &spec);
	FSSpecToHandle(&spec, objHandle);
}


//--------------------------------------------------------------------------------
/*
	Copies a FileObjectHdl to a reply record. Fills in all fields of the
	reply record with _something_.
*/

void	HandleToReply(FileObjectHdl objHandle, void* reply)
{
	OSErr			err;
	FSSpec			spec;

	StandardFileReply*	stdFileReplyPtr;
	SFReply*			sfReplyPtr;

	HandleToFSSpec(objHandle, &spec);
	FSSpecToReply(&spec, reply);

	if (IsNewCall(gSelector))
	{
		stdFileReplyPtr = (StandardFileReply*) reply;

		stdFileReplyPtr->sfReplacing = FALSE;
		stdFileReplyPtr->sfType = (**objHandle).u.type;
		stdFileReplyPtr->sfScript = smSystemScript;
		stdFileReplyPtr->sfFlags = (**objHandle).finderFlags;
		stdFileReplyPtr->sfIsFolder = FALSE;
		stdFileReplyPtr->sfIsVolume = FALSE;
		stdFileReplyPtr->sfReserved1 = 0;
		stdFileReplyPtr->sfReserved2 = 0;
	
		stdFileReplyPtr->sfGood = (err == noErr);

	}
	else
	{
		sfReplyPtr = (SFReply*) reply;
	
		sfReplyPtr->copy = FALSE;
		sfReplyPtr->fType = 'FRED';
		sfReplyPtr->fType = (**objHandle).u.type;
		sfReplyPtr->version = 0;
	
		sfReplyPtr->good = (sfReplyPtr->vRefNum != 0);
	}
}


//--------------------------------------------------------------------------------
/*
	Copies the selected entry in a list into a FSSpec. If there is no
	selection, the fields of the FSSpec are zeroed out.
*/

void	SelectionToFSSpec(ListHandle theList, FSSpec* spec)
{
	Cell	cell;
	short	offset;
	short	length;
	Ptr		itemPtr;
	
	spec->vRefNum = 0;
	spec->parID = 0;
	spec->name[0] = 0;
	*(long*) &cell = 0;
	if (LGetSelect(TRUE, &cell, theList))
	{
		LFind(&offset, &length, cell, theList);
		if (offset >= 0)
		{
			itemPtr = (*(**theList).cells + offset);
			SFEntryToFSSpec(itemPtr, spec);
		}
	}
}


//--------------------------------------------------------------------------------
/*
	Copies the selected entry in a list into a FSSpec. If there is no
	selection, the fields of the FSSpec are zeroed out.
*/

void	SFEntryToFSSpec(Ptr entry, FSSpec* spec)
{
	short				vRefNum;
	long				parID;
	Str255*				name;

	if (MacEasyOpenInstalled(entry))
	{
		StdFileListRec2*	itemPtr = (StdFileListRec2*) entry;
		vRefNum = itemPtr->vRefNum;
		parID = itemPtr->parID;
		name = (Str255*) itemPtr->name;
	}
	else
	{
		StdFileListRec*	itemPtr = (StdFileListRec*) entry;
		vRefNum = itemPtr->vRefNum;
		parID = itemPtr->parID;
		name = (Str255*) itemPtr->name;
	}

	FSMakeFSSpec(vRefNum, parID, *name, spec);
}


//--------------------------------------------------------------------------------
/*
	Determines if Macintosh Easy Open is busy running the Standard File
	dialog or not.
	
	Our current approach is to assume we are working with a pre-MEO version
	of the data structure and validate it. If we can validate it, we return
	FALSE (MEO is not installed). If we can't validate it, we return TRUE
	(MEO is installed).
	
	All we validate right now is the drive number and vRefNum. We get the
	driver number and use it in a HGetVInfo call. If the returned vRefNum
	matches what we assume to be the vRefNum, then we assume the data structure
	is valid.
	
	I abandoned the previous approach of seeing if the PACK resource being
	used came from a file with a creator type of 'xlat.' I did this because
	I wanted to protect against Apple rolling MEO into the system file.
*/

Boolean	MacEasyOpenInstalled(Ptr entry)
{
	static Boolean figuredItOut = FALSE;
	static Boolean result;
	
	OSErr			err;
	HParamBlockRec	pb;

	pb.volumeParam.ioVolIndex = 0;
	pb.volumeParam.ioNamePtr = NIL;

	if (!figuredItOut)
	{
		figuredItOut = TRUE;

		pb.volumeParam.ioVRefNum = ((StdFileListRec*) entry)->driveNumber;
		err = PBHGetVInfoSync(&pb);

		if (!err && pb.volumeParam.ioVRefNum == ((StdFileListRec*) entry)->vRefNum)
		{
			result = FALSE;
		}
		else
		{
			pb.volumeParam.ioVRefNum = ((StdFileListRec2*) entry)->driveNumber;
			err = PBHGetVInfoSync(&pb);

			if (!err && pb.volumeParam.ioVRefNum == ((StdFileListRec2*) entry)->vRefNum)
			{
				result = TRUE;
			}
			else
			{
				DebugStr("\pPointed Stick doesn't recognize the internal standard file format.");
				figuredItOut = FALSE;
			}
		}
	}
	
	return result;
}
