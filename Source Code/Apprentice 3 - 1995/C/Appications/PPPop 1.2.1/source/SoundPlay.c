/* PPPop, verison 1.2  June 6, 1995  Rob Friefeld  */

#include "SoundPlay.h"
#include <Sound.h>
#include <Files.h>



short	gSoundFileUp = -1;		// Res ID for sounds file
short	gSoundFileDown = -1;

void DoOpenSoundFiles()
{
	gSoundFileUp = DoOpenSoundFile(sSndUp);
	gSoundFileDown = DoOpenSoundFile(sSndDown);
}

short DoOpenSoundFile (short sndNameIndex)
{
	short		myVRefNum = 0;
	long		myDirID = 0;
	Str255		fileName;
	short		myResNum;
	FSSpec		theFile;
	OSErr		error;
	MenuHandle	menu;

	GetIndString(fileName, rSndName, sndNameIndex);		// The sounds file name is a STR#
	if (fileName[0] == 0)
		return(-1);

	error = FSMakeFSSpec(myVRefNum, myDirID, fileName, &theFile);
	if ((error == fnfErr) || (error != noErr))
		return(-1);

	myResNum = FSpOpenResFile(&theFile, fsCurPerm);	// -1 indicates error
	return(myResNum);
}

void PlayUp()
{
	PlaySound(gSoundFileUp, rUpSnd);
}

void PlayDown()
{
	PlaySound(gSoundFileDown, rDownSnd);
}

void Click(short resID)
{
	PlaySound(CurResFile(), resID);
}


void PlaySound(short soundResNum, short soundID)
{
	Handle			mySndHandle;
	SndChannelPtr	mySndChan;
	OSErr			error;
	short			theResFile;

	if (soundResNum == -1)
		return;					// sounds file never opened
		
	CollectGarbage();			// SndPlay will not free up mem itself
	
	theResFile = CurResFile();
	UseResFile(soundResNum);
	mySndChan = nil;

	mySndHandle = GetResource('snd ', soundID);
	if (mySndHandle != nil) {
		error = SndPlay(mySndChan, mySndHandle, kAsync);
		ReleaseResource(mySndHandle);
	}
	UseResFile(theResFile);
}
