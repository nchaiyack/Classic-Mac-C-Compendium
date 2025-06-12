#include <Palettes.h>

/*
		What this demo does:
			How to draw a picture using a custom "CLUT" resource;
			i.e. drawing a picture not using the system clut.
			No clut animation is done here.
		Last update: April 25 1994
		
		by: Hiep Dam, 3G Software
		Public Domain
		Contacting author: AOL:      starlabs
		                   Delphi:   starlabs
		                   Internet: starlabs@aol.com
*/


enum {
	kPicID = 516,			// Resource id of picture
	kClutID = kPicID		// Resource id of clut (color lookup table)
};

// ----------------------------------------------------------------

void InitMac();
void DoClutThing(WindowPtr theWindow);

// ----------------------------------------------------------------

void main() {
	InitMac();

	GDHandle saveDevice = GetGDevice();

	// Note: works only with a color window, not a normal b&w window...
	WindowPtr theWindow = (WindowPtr)NewCWindow(nil, &screenBits.bounds, "\pCluts", true, plainDBox, (WindowPtr)-1, false, 0);
	SetPort(theWindow);

	DoClutThing(theWindow);

	RestoreDeviceClut(saveDevice);	// Cheap but easy...
} // END main

// ----------------------------------------------------------------

void InitMac() {
	InitGraf(&thePort);
	InitFonts();
	InitWindows();
	TEInit();
	InitDialogs(nil);
	InitCursor();
	FlushEvents(everyEvent, 0);
} // END InitMac

// ----------------------------------------------------------------

void DoClutThing(WindowPtr theWindow) {
	SetPort(theWindow);
	FillRect(&theWindow->portRect, black);

	PicHandle thePic = (PicHandle)Get1Resource('PICT', kPicID);
	HLockHi((Handle)thePic);

	// **** Juicy part begins here... ****
	CTabHandle theClut = GetCTable(kClutID);

	PaletteHandle thePalette = NewPalette((**theClut).ctSize, theClut, pmTolerant, 0);
	NSetPalette(theWindow, thePalette, pmAllUpdates);
	ActivatePalette(theWindow);
	// **** End of juicy part... ****
	
	SetPort(theWindow);
	Rect destR = (**thePic).picFrame;
	OffsetRect(&destR, 100, 100);
	DrawPicture(thePic, &destR);
	HUnlock((Handle)thePic);

	do {} while (!Button());

	FillRect(&theWindow->portRect, black);
} // END DoClutThing