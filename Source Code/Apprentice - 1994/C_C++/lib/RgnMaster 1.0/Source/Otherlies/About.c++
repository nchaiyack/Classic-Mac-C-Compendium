#include <Sound.h>
#include "About.h"

const short kAboutID = 131;	// Rsrc id for dialog, picture, and sound.
							// If you delete the picture and/or sound, that's OK.
							// This routine WON'T crash; however, DO NOT distribute
							// the modified version!!!!

void DoRgnMasterAbout() {
	Handle aboutPic;
	Handle aboutSnd;
	GrafPtr savePort;
	DialogPtr aboutDlog;
	short itemHit;

	// Do some pre-flighting...
	aboutPic = GetResource('PICT', kAboutID);
	if (aboutPic == nil) {
		SysBeep(1);
		return;
	}
	else {
		ReleaseResource(aboutPic);	// Will be loaded by _GetNewDialog, below
	}

	aboutSnd = GetResource('ASnd', kAboutID);
	if (aboutSnd == nil) {
		SysBeep(1);
		return;
	}
	else {
		HLockHi(aboutSnd);
	}

	GetPort(&savePort);

	aboutDlog = GetNewDialog(kAboutID, nil, (WindowPtr)-1);
	SetPort(aboutDlog);
	ShowWindow(aboutDlog);
	DrawDialog(aboutDlog);
	
	(void)SndPlay(nil, aboutSnd, false);	// Synchronous
	ModalDialog(nil, &itemHit);
	
	HUnlock(aboutSnd);
	HPurge(aboutSnd);

	DisposeDialog(aboutDlog);

	SetPort(savePort);
} // END DoRgnMasterAbout