
// PROGRAM: Pict File Reader Demo
// VERSION: 1.0
// AUTHOR : Hiep Dam, 3G Software
// STATUS : Public domain, may be freely distributed
// DATE   : December 28, 1993

// -------------------------------------------------------------------

// PURPOSE:
// This is a very simple program that demonstrates how to draw a picture
// from a picture data file. Most of the time, you would usually just read
// in a picture as a "PICT" resource in a file's resource fork, via
// _GetResource('PICT', picID).

// However, there may be a time when you would want to read in a picture
// from a pict file. It's relatively easy and hassle-free to do so, BUT
// it's not obvious at first. There are some small little points here and
// there when reading in a pict file (such as it's header data).

// I didn't know how to do this at first, but after a little rummaging around
// I found an article in a *very* old issue of MacTutor describing Pict files
// and such. This was where I discovered how to read in pict files correctly.
// If you're interested, the reference is:
// Joel West, "Comments About Picts", MacTutor June 1988 Vol 4, No 6
// (See, I told you it was an old article...)

// This code should be System 6.0.x and System 7.x compatible, though I
// wrote it on System 7.1 and haven't tested it anywhere else.
// This code was written in Symantec C++ 6.0 (wonderful!), but you
// should be able to modify it to work with THINK C 6.0.

// Hope you find this code useful. Drop me a line anytime—I design games
// in my spare time, if you're interested.
// America Online:	StarLabs
// Snail-mail:		Hiep Dam
//					2226 Parkside Ave #302
//					Los Angeles,  CA  90031

// Enjoy!


// -------------------------------------------------------------------
// -------------------------------------------------------------------

// A cheap inline that just waits for the user to click the mouse...
inline void WaitMousey() { while (!Button()) {} }

// -------------------------------------------------------------------

// Some prototypes...
PicHandle GetPictFile(Rect& picRect);
void CenterRect(Rect& insideR, const Rect& outsideR);

// -------------------------------------------------------------------

void main() {
	// Doo dah, initialize the program. Too bad there isn't an option
	// in Symantec C++ to automatically intialize the ToolBox upon
	// startup, much like Think Pascal 4.0 (are they ever going to
	// upgrade that program?!?)
	InitGraf(&thePort);
	InitFonts();
	FlushEvents(everyEvent,0);
	InitWindows();
	InitMenus();
	TEInit();
	InitDialogs(0L);
	InitCursor();

	// -----------------------------------
	
	WindowPtr bkgndWind = NewWindow(nil, &screenBits.bounds, nil, true, plainDBox, (WindowPtr)-1, false, 0);
	Rect picR;
	PicHandle thePic;
	do {
		// Get rid of that distracting desktop!!
		SetPort(bkgndWind);
		FillRect(&screenBits.bounds, gray);

		// Call the actual function that reads in the pict file...
		thePic = GetPictFile(picR);
		if (thePic == nil) {
			// The function returns nil either if the file was too large
			// and it was unable to allocate enough heap space to read in
			// the file, OR the user clicked "Cancel" in the SFGetFile dialog.
			// So exit either way...
			ExitToShell();
		}

		// Okay, pict file was successfully read in, so center
		// the picture on the screen...
		CenterRect(picR, screenBits.bounds);
		
		// Get rid of that distracting desktop!!
		SetPort(bkgndWind);
		FillRect(&screenBits.bounds, gray);

		// Create our window to hold the picture...
		WindowPtr mainWindow = NewWindow(nil, &picR, nil, true, plainDBox, (WindowPtr)-1, false, 0);
		SetPort(mainWindow);
		
		// Get the picture's frame rect
		picR = (**thePic).picFrame;
		// Alright, draw it!
		DrawPicture(thePic, &picR);

		// Dispose of our picture (since GetPictFile created a handle
		// from which to load in the pict file data)
		DisposeHandle(Handle(thePic));
		
		// Allow the user to look at the picture, and wait until
		// they click the mouse...
		WaitMousey();
		
		// Get rid of the window...
		DisposeWindow(mainWindow);
		
		// Flush everything, so mouse click doesn't register
		FlushEvents(everyEvent, 0);
	} while (1);	// Loop forever, admittedly a lazy programming technique!  :)
}

// -------------------------------------------------------------------

void CenterRect(Rect& insideR, const Rect& outsideR) {
	// Basically just centers the rect insideR within rect outsideR.
	short insideWidth = insideR.right - insideR.left;
	short insideHeight = insideR.bottom - insideR.top;
	short hDiff = ((outsideR.right - outsideR.left) - (insideWidth))/2;
	short vDiff = ((outsideR.bottom - outsideR.top) - (insideHeight))/2;
	insideR.left = outsideR.left + hDiff;
	insideR.right = insideR.left + insideWidth;
	insideR.top = outsideR.top + vDiff;
	insideR.bottom = insideR.top + insideHeight;
} // END CenterRect

// -------------------------------------------------------------------

// Alright, this is where the action is!
// GetPictFile loads in a user-specified PICT file and returns the data
// in that file as a standard PicHandle picture. It's the same as any
// picture: you can _DrawPicture it, etc.

// A PICT file is identical to a pict resource except that it lies in the
// data fork of a file. There's one additional difference: the first
// 512 bytes of the file contains "header" data; we don't need the header,
// so the header is skipped over. I wouldn't call this information common
// knowledge (hey, I didn't know about it, either!)

// Note that this function reads in the pict file in one single gulp; there
// could be problems if the user specifies a really BIG pict file and there
// isn't enough memory (in which case the function beeps and returns nil).

PicHandle GetPictFile(Rect& picRect) {
	const short kPictHeaderSize = 512;	// Size of header, in bytes

	PicHandle thePic = nil;		// Handle to a picture (which we're going to load in)
	long pictFileLen;			// Length of the pict file
	short pictFileRef;			// File reference of the pict file
	SFReply	tr;
	short rc;					// Error return code
	Point where;
	where.h=100; where.v=50;
	SFTypeList typeList;
	typeList[0] = 'PICT';		// Only load in "PICT" files...


	SFGetFile(where, "\pSelect a PICT file:", nil, 1, typeList, nil, &tr);
	if (tr.good) {
		// User selected a file, now open the file with _FSOpen
		rc = FSOpen(tr.fName, tr.vRefNum, &pictFileRef);
		if (rc) {
			// Hmm. File Manager had problems opening this file.
			SysBeep(1);
			return(nil);
		}

		// Now get size of pict file
		rc = GetEOF(pictFileRef, &pictFileLen);

		// This is the non-obvious part: skip over the first 512
		// bytes of the pict file, since this contains header data
		// and pretty much doesn't apply to us...
		rc = SetFPos(pictFileRef, fsFromStart, kPictHeaderSize);

		// Adjust the length of the pict file (shorten by 512) to
		// account for our skipping the first 512 bytes
		pictFileLen -= kPictHeaderSize;

		// Now, allocate some memory with which to load our pict
		// file in.
		thePic = (PicHandle)NewHandleClear(pictFileLen);
		if (thePic == nil) {
			// Oops, unable to allocate in the heap!
			// Probably the file's too big.
			SysBeep(1);
			return(nil);
		}

		// Lock the buffer
		HLock(Handle(thePic));

		// Okay, read in the pict file
		rc = FSRead(pictFileRef, &pictFileLen, (Ptr)*thePic);

		// Unlock our pic buffer and close the file.
		HUnlock(Handle(thePic));
		FSClose(pictFileRef);

		// Update the picture frame rect
		picRect = (**thePic).picFrame;
	}
	else
		return(nil);

	// If we get all the way to here, no errors have occurred—GREAT!
	// Return the pic buffer we allocated.
	// Note that it's up to the caller to dispose of the pic buffer
	// via DisposeHandle, since there's no way for GetPictFile to do it...
	return(thePic);
} // END GetPictFile

// END Main.c++