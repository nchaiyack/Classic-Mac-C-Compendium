/* GetScreenDevice.c
PROBLEM:
How can I tell whether I've got a CGrafPtr or a GWorldPtr? (In one case I'll call 
GetGWorldDevice, in the other case I'll get max device.)
I noticed that the portVersions are 0xc001 and 0xc000. Should I use that to tell?

ANSWER FROM APPLE DEVSUPPORT (8/93):
You can use GetGWorldDevice() on GWorldPtr, GrafPort or CGrafPort. Use
TestDeviceAttibute() to test if the returned GDHandle is a "real" screen or not.

Also, you should refer to the tech note ÒRowBytes Revealed IIÓ (available on the
June Developer CD) which states:

You'll notice in Figure 1 that the portVersion field of a cGrafPort coincides
with the location of the rowBytes field of a grafPort. Remember, a cGrafPort has
the same size as a grafPort. During debugging, you can use the same information
that CopyBits uses to identify cGrafPorts.

If you use a grafPort template to display memory for an unknown grafPort, you
can tell if it is a cGrafPort because the rowBytes will be equal to 0xC000. The
0xC corresponds to the two high bits being set in the portVersion field of a
cGrafPort. Since these bits can not be set in a grafPort, you know you have a
cGrafPort. In addition, if the bottom bit of the portVersion field is set, then
it is a gWorld. Thus, if your rowBytes field has a value of 0xC001, then you
know you have a gWorld.

CONCLUSION:
GetWindowDevice() assumes that it's a GWorld iff the bits 0xC001 of the
are set. From Apple's comment above this is clearly true at present and probably
safe for the future.

Copyright © 1989-1993 Denis G. Pelli
HISTORY:
3/20/90		dgp	make compatible with MPW C.
3/22/90	dgp	changed GetDeviceSlot to use the AuxDCEHandle instead of deducing it
			from the baseAddr of the PixMap. This is a cleaner way to do it.
4/9/90	dgp	eliminated #define for Mainscrn mispelling in Color.h
10/17/90 dgp Added AddressToScreenDevice() for compatibility with built-in video on
			the Mac IIci, IIsi, and LC.
10/18/90 dgp Added LocalToGlobalRect() and GetWindowDevice().
8/24/91		dgp	Made compatible with THINK C 5.0.
2/1/92	dgp	Fixed bugs in GetWindowDevice() which resulted in returning garbage GDHandle.
3/3/92	dgp	In GetScreenDevice(), skip inactive screens.
8/20/92	dgp	expanded comments of GetDeviceSlot(), noting that it works even with
			built-in video, e.g. on Mac IIci.
8/26/92	dgp	GetDeviceSlot() now returns -1 if none, since zero is a legal slot.
			GetScreenDevice() first checks for 8-bit QuickDraw().
9/10/92	dgp	Actually implemented the 8/26 change instead of just changing the 
			documentation. Oops.
4/17/93	dgp Deleted obsolete AddressToSlot and AddressToScreenDevice.
5/21/93	dgp	Fixed GetWindowDevice() to support GWorld's.
8/14/93	dgp	Based on answer from DEVSUPPORT, I cleaned up GetWindowDevice().
4/11/94	dgp	Added GetRectDevice() based on code extracted from GetWindowDevice().

*/
#include "VideoToolbox.h"
GDHandle GetRectDevice(Rect *r);

GDHandle GetScreenDevice(int n)
// Returns a handle to the n-th screen, where the MainDevice is the zero-th screen.
// Returns NULL if request can't be satisfied.
{
	GDHandle device;
	int i,error;
	long value;

	if(n<0)return NULL;
	error=Gestalt(gestaltQuickdrawVersion,&value);
	if(error || value<gestalt8BitQD)return NULL;	// need 8-bit quickdraw
	if(n==0) return GetMainDevice();
	device=GetDeviceList();
	i=0;
	while(device!=NULL){
		if (TestDeviceAttribute(device,screenDevice)
			&& !TestDeviceAttribute(device,mainScreen)
			&& TestDeviceAttribute(device,screenActive)){
				i++;
				if(i==n)break;
			}
		device = GetNextDevice(device);
	}
	return device;
}

int GetScreenIndex(GDHandle device)
// Inverse of GetScreenDevice(). Returns -1 if request can't be satisfied.
{
	int i,error;
	long value;

	error=Gestalt(gestaltQuickdrawVersion,&value);
	if(error || value<gestalt8BitQD)return 0;
	if(device==NULL)return -1;
	for(i=0;i<16;i++)if(device==GetScreenDevice(i))return i;
	return -1;
}

short int GetDeviceSlot(GDHandle device)
// Gets the "slot" for any screen device, even if it's built-in video, e.g. on Mac
// IIci or Quadra. See 1992 Inside Mac "Processes" page 4-11. Returns -1 if none.
// Zero is a legal slot for built-in video devices.
{
	AuxDCEHandle myAuxDCEHandle;
 
	if(device == NULL) return -1;
	myAuxDCEHandle=(AuxDCEHandle) GetDCtlEntry((**device).gdRefNum);
	return ((**myAuxDCEHandle).dCtlSlot);
}

GDHandle SlotToScreenDevice(int n)
// Returns a handle to the screen device in slot n.
// Returns NULL if request can't be satisfied.
{
	GDHandle device;

	device=GetDeviceList();
	while(device!=NULL) {
		if (TestDeviceAttribute(device,screenDevice) &&
			GetDeviceSlot(device)==n)
				break;
		device=GetNextDevice(device);
	}
	return device;
}

GDHandle GetWindowDevice(WindowPtr window)
// For on-screen window, returns GDHandle of screen with largest intersection with the 
// window's content.
// For off-screen window (i.e. GWorld), it returns the GDHandle of the associated device.
{
	Rect r;
	WindowPtr oldWindow;
	long qD;

	if(window==NULL)return NULL;
	Gestalt(gestaltQuickdrawVersion,&qD);
	if(qD>=gestalt32BitQD && (((CWindowPtr)window)->portVersion&0xc001)==0xc001){
		// It's a GWorld iff the portVersion has both high bits set (cGrafPort) and
		// the low bit set (GWorld). See Tech Note ÒRowBytes Revealed IIÓ.
		return GetGWorldDevice((GWorldPtr)window);
	}
	r=window->portRect;
	GetPort(&oldWindow);
	SetPort(window);
	LocalToGlobalRect(&r);
	SetPort(oldWindow);
	return GetRectDevice(&r);
}

GDHandle GetRectDevice(Rect *r)
// Returns GDHandle of screen with largest intersection with the global rect.
{
	Rect overlap;
	GDHandle device,dominantDevice=NULL;
	long area,greatestArea;
	long qD;

	if(r==NULL)return NULL;
	Gestalt(gestaltQuickdrawVersion,&qD);
	if(qD<gestalt8BitQD)return NULL;	// need 8-bit quickdraw
	device=GetDeviceList();
	greatestArea=0;
	while(device!=NULL){
		if(TestDeviceAttribute(device,screenDevice)
			&& TestDeviceAttribute(device,screenActive)){
				SectRect(r,&(*device)->gdRect,&overlap);
				area=(long)(overlap.right-overlap.left)*(overlap.bottom-overlap.top);
				if(area>greatestArea){
					greatestArea=area;
					dominantDevice=device;
				}
			}
		device=GetNextDevice(device);
	}
	return dominantDevice;
}

void LocalToGlobalRect(Rect *r)
{
	Point pt={0,0};
	
	LocalToGlobal(&pt);
	OffsetRect(r,pt.h,pt.v);
}

void GlobalToLocalRect(Rect *r)
{
	Point pt={0,0};
	
	GlobalToLocal(&pt);
	OffsetRect(r,pt.h,pt.v);
}