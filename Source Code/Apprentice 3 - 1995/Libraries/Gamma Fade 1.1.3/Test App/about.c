// File "about.c" - Source for the Cool About Box

#include "about.h"
#include "gamma.h"

// * ****************************************************************************** *

void doAboutBox() {
	short done=0, theItem=0, i;
	DialogPtr theDialog;	

	SetCursor(&arrow);

	if (IsGammaAvailable() == 0) {
		DebugStr("\pTester is unable to do fades on this machine.");
		ExitToShell();
		}
	
	SetupGammaTools();
	for(i=100; i >= 0; i-=5) DoGammaFade(i);

	theDialog = GetNewDialog(kAboutBoxDLOG, 0, (WindowPtr) -1);
	ShowWindow(theDialog);
	UpdtDialog(theDialog,theDialog->visRgn);

	for(i=0; i <= 100; i+=2) DoOneGammaFade(GetMainDevice(), i);

	while(done == 0) {
		ModalDialog(aboutFilterProc, &theItem);
		if (theItem == 1) done = -1;
		}

	for(i=100; i >= 0; i-=2) DoOneGammaFade(GetMainDevice(), i);

	HideWindow(theDialog);
	DisposeDialog(theDialog);

	for(i=0; i <= 100; i+=5) DoGammaFade(i);
	DisposeGammaTools();
	}

// * ****************************************************************************** *
// * ****************************************************************************** *

pascal Boolean aboutFilterProc(DialogPtr theDialog, EventRecord *theEvent, short *theItem) {

	switch(theEvent->what) {
		case mouseDown:
		case keyDown:
		case autoKey:
			*theItem = 1;
			return(kDontPassIt);
			break;
			
		case nullEvent:
		default:
			if (theEvent->modifiers & (cmdKey + optionKey + shiftKey + controlKey)) {
				*theItem = 1;
				return(kDontPassIt);
				}
			break;
		}

	return(kPassItOn);
	}
