#include <Folders.h>
#include "Global.h"		// For things like kStrings and kPrefsFile


Boolean	KeyIsDown(short keyCode);
Boolean	OptionKeyIsDown(void);


//--------------------------------------------------------------------------------

short main(FSSpecPtr prefsFile, FSSpecPtr me)
{
	OSErr			err;
	FCBPBRec		pb;

	if (SysVersion < 0x0700)
		return kErrNeedSystem7;

	if (OptionKeyIsDown())
		return kOptionKeyWasDown;

	err = FindFolder(kOnSystemDisk, kPreferencesFolderType, TRUE,
							&prefsFile->vRefNum, &prefsFile->parID);
	if (err != noErr)
		return kFindFolderFailed;

	GetIndString(prefsFile->name, kStrings, kPrefsFile);

	pb.ioCompletion = nil;
	pb.ioNamePtr = me->name;
	pb.ioVRefNum = 0;
	pb.ioRefNum = CurResFile();
	pb.ioFCBIndx = 0;

	err = PBGetFCBInfoSync(&pb);

	if (err != noErr)
		return kErrGetFCBInfo;

	me->vRefNum = pb.ioFCBVRefNum;
	me->parID = pb.ioFCBParID;

	return noErr;
}


//--------------------------------------------------------------------------------

Boolean	KeyIsDown(short keyCode)
{
	union {
		KeyMap			asMap;
		unsigned char	asBytes[16];
	} myMap;

	GetKeys(myMap.asMap);
	return ((myMap.asBytes[keyCode >> 3] >> (keyCode & 0x07)) & 1) != 0;
}


//--------------------------------------------------------------------------------

Boolean	OptionKeyIsDown()
{
	const short kOptionKey = 0x3A;

	return KeyIsDown(kOptionKey);
}


