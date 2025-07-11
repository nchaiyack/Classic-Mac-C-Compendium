/*****************
** savepict.c
**
** routines to handle saving the image
** generated as a pict file.
******************/

#include <MacHeaders>
#include "globals.h"
#include "constants.h"

static int Write512b( int refNum);

/*****************
** SavePict()
**
** This routine saves the generated image as a PICT file.
******************/
void SavePict(void)
{
	PicHandle	thePict;
	GrafPtr		savedPort;
	Point		where= {100,100};
	Str255		prompt, name;
	SFReply		reply;
	int			fRefNum;
	long int	size;
	int			result;
	
	// get file info to save document as
	GetIndString( prompt, kErrStrList, kSavePrompt);
	GetIndString( name,   kErrStrList, kUntitledName);
	SFPutFile( where, prompt, name, NULL, &reply);
	
	if (!reply.good)	// if user cancels it, return.
		return;
	
	// open file, setting type/creator to 'PICT' and 'MORF'
	result = Create( reply.fName, reply.vRefNum,
					kFileCreator, kPictFileType);
					
	switch (result) {
	case noErr:
		// file was created OK -- open it
		if (FSOpen( reply.fName, reply.vRefNum, &fRefNum) != noErr)
		{
			Error(kFileOpenErr, 0, 0, 0, noteIcon);
			return;
		}
		break;

	case dupFNErr:
		// file already existed.  Overwrite it.
		if (FSOpen( reply.fName, reply.vRefNum, &fRefNum) != noErr)
		{
			Error(kFileOpenErr, 0, 0, 0, noteIcon);
			return;
		}
		if (SetEOF( fRefNum, 0L) != noErr)
		{
			Error(kFileOpenErr, 0, 0, 0, noteIcon);
			FSClose( fRefNum);
			return;
		}
		break;
	default:
		Error(kFileOpenErr, 0, 0, 0, noteIcon);
		return;
		break;
	} // switch

	
	// write out the initial 512 bytes of stuff.  Nothing important.
	if (Write512b( fRefNum) != noErr)
	{
		Error( kFileSaveErr, 0, 0, 0, cautionIcon);
		FSClose( fRefNum);
		return;
	}
	
	GetPort( &savedPort);
	SetPort( gMainWindow);
	ClipRect( &gMainWindow->portRect);
	
	thePict = OpenPicture( &gMainWindow->portRect);
	
	CopyBits( &gOffBM, &gMainWindow->portBits,
			  &gOffGP.portRect, &gMainWindow->portRect, srcCopy, NULL);
	
	ClosePicture();
	
	// write out the data at **thePict

	size = GetHandleSize( (Handle)thePict);
	if ( FSWrite( fRefNum, &size, *thePict) != noErr)
		Error(kFileSaveErr, 0, 0, 0, cautionIcon);
	
	FSClose( fRefNum);
	
	KillPicture( thePict);
} // SavePict()


static int Write512b( int refNum)
{
	Ptr p;
	int code;
	long int size=512;
	
	if ( (p = NewPtrClear( size)) == NULL)
		return !noErr;	// write failed
	code = FSWrite( refNum, &size, p);
	if (code==noErr && size!=512)
		code = !noErr;
	DisposPtr(p);
	return code;
} // Write512b

