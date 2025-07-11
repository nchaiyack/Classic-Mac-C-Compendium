/* GDOpenWindow.c
Copyright � 1989-1994 Denis G. Pelli

SUMMARY:

AddExplicitPalette(window) adds a palette to a color window or GWorld with all the
colors marked as pmExplicit. This allows you to use PmForeColor() and
PmBackColor() to specify the value you want poked into each pixel by QuickDraw
operations, e.g. EraseRect() and DrawString().

RemovePalette(window) disposes of the palette created by AddExplicitPalette.

window=GDOpenWindow1(device) opens a full-screen window on the specified screen,
and calls AddExplicitPalette. The window truly fills the screen: any rounded corners 
are made square and if the window is on the main screen then the Menu Bar is
hidden.

GDDisposeWindow1(window) closes and discards a color window, disposing of any palette
or color table, generally undoing whatever GDOpenWindow1 did.

These routines return and accept a WindowPtr (even though it's actually
a color window), which seems to be the standard way of doing things. 
The older routines, without the "1" suffix, return and accept a CWindowPtr,
which ends up forcing you to do a lot of tedious casting, e.g. when you call
SetPort(), which wants a WindowPtr.

COMMENTS:

QuickDraw likes to pick a bunch of good colors and stuff them in the clut, in
essentially random order, except that white is first and black is last. It wants
you to specify any desired color as an RGB triplet and then it picks the mysterious
clut index that would provide the closest match. If you're processing grayscale
images, then QuickDraw's approach involves a lot of overhead involving inverse
color tables, and makes the numbers stored in your pixels meaningless (unless
you look them up in the associated color table or palette).

The philosophy of the VideoToolbox is to bypass QuickDraw's color model, and
work explicitly with the numbers that are stored in your pixels.
SetPixelsQuickly will efficiently poke (or peek) numbers in your PixMap.
However, if you want to use QuickDraw's drawing operations, especially
EraseRect() and DrawString() then you need a way to specify the foreground and
background colors. AddExplicitPalette() gives your window a palette in which all
the colors are marked as pmExplicit. This tells the palette manager not to
meddle, and to use your arguments to PmForeColor() and PmBackColor() literally.

I suggest that you control the clut by calling GDUncorrectedGamma() and
GDSetEntries(), since these calls directly control the video device driver,
bypassing the Color Manager. Since the Color and Palette Managers don't know
that you've changed the color environment they can't react to it, and will
passively let you continue to specify colors by their clut index. For example,
the Palette Manager religiously believes that the first clut entry should be
white and the last one black, and it will change them back to those values if
you change them and it finds out about it. If you use the Color Manager
SetEntries call then the Paletter Manger WILL find out because a record is made
in the ColorTable. Calling GDSetEntries() bypasses the Color Manager. Instead,
the video device driver writes directly to the clut and the ColorTable is not
modified. Of course, this means that you should ignore the ColorTable since it
will no longer reflect the contents of the clut.

The Inside Mac books suggest that a new color window should be given its own
color table. This will happen if you set MAKE_COLOR_TABLE true. However, for my
purposes I think of the window and the screen as essentially the same thing, so
I give the window a handle to the screen's color table, so they share the same
table.

Every time you access the stdio package, e.g. printf or getch(), THINK C will
move the Console window to the front, which may obscure your window. You can
bring your window back to the front by calling BringToFront().

EXAMPLE:

Open your window by saying:
	window=GDOpenWindow1(device);
When you're through with the window, get rid of it by calling:
	GDDisposeWindow1(window);
Besides closing the window and disposing of the allocated memory structures it
restores the device's clut to whatever is in the color table associated with device.

HISTORY:

12/88		dgp	wrote it
8/5/89			added call to GDUncorrectedGamma, so I couldn't forget.
8/15/89 	dgp trivia
3/20/90		dgp	make compatible with MPW C.
3/29/90		dgp	changed declared returned type from WindowPtr to CWindowPtr, which
				is what it's really been all along. Same change to argument of
				GDDisposeWindow(). The new offscreen GWorld calls for the first time
				make it easier to honestly declare one's windows as color rather
				than pretending they're not.
8/24/91		dgp	Made compatible with THINK C 5.0.
2/1/92		dgp Made optional the device argument to GDDisposeWindow(). If it's
				NULL, then it will be determined automatically from window.
2/3/93		dhb	Extracted AddExplicitPalette from GDOpenWindow.
2/21/93		dgp	HideMenuBar if window is on main screen.
2/23/93		dgp AddExplicitPalette() returns immediately unless it receives a color 
				window.
				Added GDOpenWindow1() and GDDisposeWindow1(), which both use
				a WindowPtr, instead of a less convenient CWindowPtr.
3/5/93		dgp	Added calls to UnclipScreen() and RestoreScreenClipping(), so the window
				now truly fills the whole screen. Edited GDOpenWindow() for clarity.
3/7/83		dgp	Added calls to GDSaveGamma(device) and GDRestoreGamma(device).
4/16/93		dgp	Cosmetic editing.
5/22/93		dgp	Added RemovePalette(), but didn't test it.
1/27/94		dgp	Cosmetic editing.
3/5/94		dgp	Decided that RemovePalette() works fine now.
*/
#include "VideoToolbox.h"

#define MAKE_COLOR_TABLE 0

CWindowPtr GDOpenWindow(GDHandle device)
// Obsolete. Use GDOpenWindow1 instead.
{
	return (CWindowPtr)GDOpenWindow1(device);
}

void GDDisposeWindow(GDHandle device,CWindowPtr window)
// Obsolete. Use GDDisposeWindow1 instead.
{
	GDDisposeWindow1((WindowPtr) window);
}


WindowPtr GDOpenWindow1(GDHandle device)
{
	WindowPtr window;
	Rect r;
	GDHandle oldDevice;

	if(device==NULL)return NULL;
	r=(*device)->gdRect;		// rect of desired screen in global coordinates
	GDSaveGamma(device);
	GDUncorrectedGamma(device);
	UnclipScreen(device);
	oldDevice=GetGDevice();
	SetGDevice(GetMainDevice());
	window=(WindowPtr)NewCWindow(NULL,&r,"\pHi",TRUE,plainDBox,(WindowPtr) -1L,0,123L);
	SetGDevice(oldDevice);
	AddExplicitPalette(window);
	return window;
}

void GDDisposeWindow1(WindowPtr window)
// Dispose of window and palette and restore the clut.
{
	GDHandle device;
	
	if(window==NULL)return;
	device=GetWindowDevice(window);
	DisposePalette(GetPalette(window));
	#if MAKE_COLOR_TABLE
		DisposHandle((Handle)(*((CGrafPtr)window)->portPixMap)->pmTable);
	#endif
	DisposeWindow(window);
	if(device==NULL)return;
	GDRestoreGamma(device);
	GDRestoreDeviceClut(device);
	RestoreScreenClipping(device);
}

void AddExplicitPalette(WindowPtr window)
// Create a palette for the color window and mark all the entries as explicit.
// Copy the entries from the window's device.
{
	GDHandle device;
	CTabHandle cTabHandle;
	PaletteHandle palette;
	int colors,i;
	OSErr error;

	if(window==NULL)return;
	if(((CGrafPtr)window)->portVersion>=0) return;	// Not a color window, return.
	device=GetWindowDevice(window);
	if(device==NULL)PrintfExit("AddExplicitPalette: window has no device!\n");
	cTabHandle=(**(**device).gdPMap).pmTable;
	colors=(*cTabHandle)->ctSize+1;
	#if MAKE_COLOR_TABLE
		error=HandToHand((Handle*)&cTabHandle);
		if(error)PrintfExit("AddExplicitPalette: error %d in copying color table\n",error);
		for(i=0;i<colors;i++) (*cTabHandle)->ctTable[i].value=i;
		(*cTabHandle)->ctFlags &= 0x7fff;
		(*cTabHandle)->ctSeed=GetCTSeed();
		(*((CGrafPtr)window)->portPixMap)->pmTable=cTabHandle;
	#endif
	palette=NewPalette(colors,cTabHandle,pmExplicit,0);
	SetPalette(window,palette,0);
}

void RemovePalette(WindowPtr window)
{
	DisposePalette(GetPalette(window));
}
/*
RemovePalette may look dangerous. I worried that the Window or
Palette Manager might become confused if it later tries to access the
window's palette, which no longer exists. However, by trial and error
I've found that this works fine, and that my attempts to make explicit the
fact that the palette is gone, e.g. by calling SetPalette(window,NULL,0),
all produced crashes. I conclude that the Palette or Window Manager monitors
the calls to DisposePalette and thus knows that the palette is gone.
*/