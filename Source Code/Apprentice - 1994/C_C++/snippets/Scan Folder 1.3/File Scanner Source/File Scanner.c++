#include "Scan.h"

WindowPtr bkgndWind = nil;
void SetupApp();

void main() {
	SetupApp();

	// --------------------------------------------------------------------------
	// The following code is basically all you have to do to use
	// the Scan routines:

	// Allocate 10 FSSpec records
	FSSpec *fileArray = new FSSpec[10];
	
	short defaultVRefNum;
	long defaultDirID;
	long dummy;
	short numFiles;
	
	// Get the directory of the folder that we're located in; this
	// is where we'll start scanning from...
	HGetVol(nil, &defaultVRefNum, &defaultDirID);

	// Alright! Let's look for our plug-in files:
	 numFiles = ScanFolderSpecific("\pPlug-Ins", defaultDirID, 'TEXT', 'KAHL', fileArray, 10);

	// Or you can...
	// numFiles = ScanFolderByName("\pPlug-Ins", defaultDirID, "\pTeachtext dummy file", fileArray, 10);
	// numFiles = ScanFolderByType("\pPlug-Ins", defaultDirID, 'sfil', fileArray, 10);
	// numFiles = ScanFolderByCreator("\pPlug-Ins", defaultDirID, 'ttxt', fileArray, 10);
	// numFiles = ScanFolder("\pPlug-Ins", defaultDirID, fileArray, 10);
	// Or you can start searching in app's folder, not in Plug-Ins folder:
	// numFiles = ScanFolderByType(nil, defaultDirID, 'TEXT', fileArray, 10);

	// Cool!! All of our plug-in files (if we had any) have been placed
	// in the FSSpec array, fileArray. All we have to do now is loop through them
	// and do our thing (in this example, just opening them and closing them).
	
	// --------------------------------------------------------------------------
	// The following is application-specific stuff

	// Do some "quick 'n dirty" quickdrawing...
	Str255 tempStr;
	NumToString(numFiles, tempStr);
	MoveTo(20, 20);
	DrawString("\pNum of files found of type 'TEXT' by 'KAHL' in Plug-Ins folder: ");
	DrawString(tempStr);
	MoveTo(20, 40); DrawString("\pClick to continue...");
	
	while (!Button()) {} Delay(10, &dummy); FlushEvents(everyEvent, 0);
	ForeColor(redColor);
	MoveTo(20, 70); DrawString("\pName of files in Plug-Ins folder:");
	// Not necessary:
	//PrefixColons(fileArray, numFiles);
	for (short i = 0; i < numFiles; i++) {
		MoveTo(40, 90 + (i * 20));
		DrawString(fileArray[i].name);
	}
	
	// ------------------------------------------------------------------------
	// Example of how to use FSSpec's to open data forks (text) or open
	// resource filesÑit's pretty simple once you get the hang of it...
	
	short fsErr;
	short fileRefNum;
	Boolean allOkay = true;
	
	ForeColor(greenColor);
	MoveTo(20, bkgndWind->portRect.bottom - 10);
	DrawString("\pOpening file # ");
	while (!Button()) {} Delay(10, &dummy); FlushEvents(everyEvent, 0);
	for (i = 0; i < numFiles; i++) {
		// FSpOpenDF has no problems open files with or without
		// data forks OR resource forks.
		fsErr = FSpOpenDF(&fileArray[i], fsRdPerm, &fileRefNum);
		if (fsErr != noErr) {
			allOkay = false;
			break;
		}
		FSClose(dummy);
		
		// FSpOpenResFile will return an error if the file
		// doesn't have a resource fork... Since there's no
		// resource fork, error -39 will be returned, end
		// of file [EOF] error... (text files created
		// by Think Project Manager have resources; text files
		// by Teachtext don't)
		fileRefNum = FSpOpenResFile(&fileArray[i], fsRdPerm);
		if (fileRefNum == -1) {
			allOkay = false;
			fsErr = ResError();
			break;
		}
		CloseResFile(fileRefNum);

		NumToString(i +1, tempStr);
		DrawString(tempStr);
	}

	MoveTo(20, 90 + ((numFiles + 1) * 20));
	if (allOkay)
		DrawString("\pWas able to open all files in FSSpec array.");
	else {
		DrawString("\pAargh! Had problems opening a file! Error: ");
		NumToString(fsErr, tempStr);
		DrawString(tempStr);
	}
	
	while (!Button()) {}
} // END main

void SetupApp() {
	InitGraf(&thePort);
	InitFonts();
	FlushEvents(everyEvent,0);
	InitWindows();
	InitMenus();
	TEInit();
	InitDialogs(0L);
	InitCursor();

	Rect tRect = screenBits.bounds;
	InsetRect(&tRect, 80, 80);
	bkgndWind = NewWindow(nil, &tRect, nil, true, plainDBox, (WindowPtr)-1, false, 0);
	SetPort(bkgndWind);
	
	TextFont(geneva);
	TextSize(9);
	ForeColor(blueColor);
} // END SetupApp


// END File Scanner.c++