/* 
GDVideo.c
Copyright © 1989-1993 Denis G. Pelli

Complete set of routines to control video drivers directly, bypassing
QuickDraw's Color and Palette Managers. There is a separate function call for
each of the Control and Status calls implemented by video drivers. They are
described in Designing Cards and Drivers, 3rd Ed., chapter 9.

This file also contains several other helpful routines that deal with video
device drivers. For background, read “Video synch”.
 
THE MORE-USEFUL (HIGH-LEVEL) ROUTINES: (in alphabetical order)

char *GDCardName(GDHandle device);
Returns the address of a C string containing the name of the video card. You
should call DisposePtr() on the returned string when you no longer need it.
Takes about 1.5 ms because Apple's slot routines are very slow. 

short GDClutSize(GDHandle device);
Returns the number of entries in the video driver's clut in the current video
mode (i.e. current pixel size). VideoToolbox.h defines a preprocessor macro
GDCLUTSIZE(device) that expands to equivalent inline code.

long GDColors(GDHandle device)
Number of colors, in the current mode.

short GDDacSize(GDHandle device)
Figures out how many bits in the video dac.

OSErr GDGetEntries(GDHandle device,short start,short count,ColorSpec *table);
This is much as you'd expect after reading GDSetEntries above. Note that unless
the gamma table is linear, the values returned may not be the same as those
originally passed by GDSetEntries. So call GDUncorrectedGamma first. Try the demo 
TimeVideo.

OSErr GDGetGamma(GDHandle device,GammaTbl **myGammaTblHandle);
Returns a pointer to the Gamma table in the specified video device. (I.e. you
pass it a pointer to your pointer, a handle, which it uses to load your
pointer.)

OSErr GDGetPageCnt(GDHandle device,short mode,short *pagesPtr);
Called "GetPages" in Designing Cards and Drivers, 3rd Ed. You tell it which mode
you're interested in. It tells you how many pages of video ram are available in 
that mode.

Boolean GDHasMode(GDHandle device,short mode,short *pixelSizePtr,short *pagesPtr);
Returns 0 if no such video mode, returns 1 if mode is available.
If pixelSizePtr is not NULL, then sets *pixelSizePtr to pixelSize or -1 if unknown.
If pagesPtr is not NULL, then sets *pagesPtr to pages.

unsigned char *GDName(GDHandle device);
Returns a pointer to the name of the driver (a pascal string). This is quick.
The string is part of the driver itself, so don't modify it or try to dispose of it.

ColorSpec *GDNewLinearColorTable(GDHandle device);
Creates a default table for use when gdType==directType.

OSErr GDPrintGammaTable(FILE *o,GDHandle device);

OSErr GDRestoreDeviceClut(GDHandle device);
Nominally equivalent to Apple's RestoreDeviceClut(), which is only available
since System 6.05. However, I find that Apple's routine sometimes does nothing,
whereas this routine always works. Passing a NULL argument causes restoration of
cluts of all video devices.

OSErr GDSaveGamma(GDHandle device);
OSErr GDRestoreGamma(GDHandle device);
Use an internal cache to save and restore the device's gamma-correction table. Call
GDRestoreGamma before GDRestoreDeviceClut.

OSErr GDSetEntriesByType(GDHandle device,short start,short count,ColorSpec *table);
Checks the (**device).gdType field and calls GDSetEntries, GDDirectSetEntries,
or nothing, as appropriate.

OSErr GDSetEntriesByTypeHighPriority(GDHandle device,short start,short count
	,ColorSpec *table);
Calls GDSetEntriesByType() while the processor priority has been temporarily 
raised to 7.

OSErr GDSetGamma(GDHandle device, GammaTbl *gamma);
Loads a Gamma table into the specified video device. The video driver will make
a copy of your table. You can discard your table after making this call. Note
that the video driver only uses the gamma table when performing SetEntries, i.e.
when actually loading the clut. The structure of the gamma table is (finally!)
documented in Designing Cards and Drivers, 3rd edition, pages 215-216. Beware of
a discrepancy between the documentation and the definition in QuickDraw.h:
gFormulaData is described as a byte array in the text, but is declared as a
short array in the QuickDraw.h header file.

OSErr GDSetPageDrawn(GDHandle device,short page);
Choose which page of video memory will be used by future drawing operations. 
Untested.

OSErr GDSetPageShown(GDHandle device,short page);
Choose which page of video memory we see. Untested.

OSErr GDUncorrectedGamma(GDHandle device);
Asks GDSetGamma to load a linear gamma table, i.e. no correction. (The gamma
correction implemented by table-lookup in the video driver is too crude for
experiments that want accurate luminance control.)

int GDVersion(GDHandle device)
Returns the version number of the driver. From the first word-aligned word after
the name string. This is quick.

LESS-USEFUL (LOW LEVEL) ROUTINES:

OSErr GDControl(int refNum,int csCode,Ptr csParamPtr)
Uses low-level PBControl() call to implement a "Control()" call that works! 
I don't know why this wasn't discussed in Apple Tech Note 262.

OSErr GDDirectSetEntries(GDHandle device,short start,short count,ColorSpec *table);
If your pixel depth is >8 then the setEntries call is disabled, and you must use
this instead of GDSetEntries().

VideoDriver *GDDriverAddress(GDHandle device);
Returns a pointer to the driver, whether in ROM or RAM. Neither this prototype
nor the definition of the VideoDriver structure are included in VideoToolbox.h.

OSErr GDGetDefaultMode(GDHandle device,short *modePtr)
It tells you what the default mode is. I'm not sure what this means.

OSErr GDGetGray(GDHandle device,Boolean *flagPtr);
Get gray flag. 0 for color. 1 if all colors mapped to luminance-equivalent gray tones.

OSErr GDGetInterrupt(GDHandle device,Boolean *flagPtr);
Get flag. 1 if VBL interrupts of this card are enabled. 0 if disabled. 

OSErr GDGetMode(GDHandle device,short *modePtr,short *pagePtr,Ptr *baseAddrPtr);
It tells you the current mode, page of video memory, and the base address of that
page.

OSErr GDGetPageBase(GDHandle device,short page,Ptr *baseAddrPtr);
Called "GetBaseAddr" in Designing Cards and Drivers, 3rd Ed. You tell it which
page of video memory you're interested in (in the current video mode). It tells
you the base address of that page.

OSErr GDGrayPage(GDHandle device,short page);
Called "GrayScreen" in Designing Cards and Drivers, 3rd Ed. Fills the specified
page with gray, i.e. the dithered desktop pattern. I'm not aware of any
particular advantage in using this instead of FillRect(). Designing Cards and
Drivers, 3rd Edition, Chapter 9, says that for direct devices, i.e. >8 bit
pixels, the driver will also load a linear, gamma-corrected, gray color table.

OSErr GDReset(GDHandle device, short *modePtr, short *pagePtr, Ptr *baseAddrPtr);
Initialize the video card to its startup state, usually 1 bit per pixel. Returns
the parameters of that state.

OSErr GDSetDefaultMode(GDHandle device,short mode);
Supposedly, you tell it what mode you want to start up with when you reboot.
(I've never been able to get this to work. No error and no effect. Perhaps I've
misunderstood its purpose.)

OSErr GDSetEntries(GDHandle device,short start,short count,ColorSpec *table);
Does a low-level setEntries Control call to the video card's driver, loading any
number of clut entries with arbitrary rgb triplets. (Note that the driver will
transform your rgb triplets via the gamma table before loading them into the
clut; so call GDUncorrectedGamma first.) "device" specifies which video device's
clut you want to load. "start" is either in the range 0 to clutSize-1,
indicating which clut entry to load first (in "sequence mode"), or is -1
(requesting "index mode"). "count" is the number of entries to be modified,
minus 1. "table" is a ColorSpec array (not a ColorTable) containing the rgb
triplets that you want to load into the clut. In sequence mode "start" must be
in the range 0 to clutSize-1, the i-th element of table corresponds to the
i+start entry in the clut, and the "value" field of each element of table is
ignored. In index mode "start" must be -1, and the "value" field of each element
of table indicates which clut entry to load it into. The arguments start,
count, and table are the same as for the Color Manager call SetEntries(),
documented in Inside Macintosh V-143. (Most drivers wait for blanking before
modifying the clut. For a full discussion, read the "Video synch" file.) You may
also want to look at the file SetEntriesQuickly.c, which provides the
functionality of GDSetEntries and GDDirectSetEntries, but bypasses the video
driver to access the hardware directly.

OSErr GDSetGray(GDHandle device,Boolean flag);
Set gray flag. 0 for color. 1 if all colors mapped to luminance-equivalent gray tones.

OSErr GDSetInterrupt(GDHandle device,Boolean flag);
Set flag to 1 to enable VBL interrupts of this card, or 0 to disable. 

OSErr GDSetMode(GDHandle device,short mode,short page,Ptr *baseAddrPtr);
You tell it what mode and video page you want. It sets 'em and returns the base
address of that page in video memory. Also, because Apple said so, the video
driver sets the entire clut to 50% gray. Note that this changes things behind
QuickDraw's back. For most applications life will be simpler if you overtly ask
QuickDraw to take charge by calling Apple's SetDepth() instead of GDSetMode().
WARNING: Apple now considers the mode numbers merely ordinal. E.g. on the
8•24 video card the 32-bit mode has mode number 0x84, not 0x85 as you might
have expected on the basis of old Apple documentation and other Apple video cards.

OSErr GDStatus(int refNum,int csCode,Ptr csParamPtr)
Uses low-level PBStatus() call to implement a "Status()" call that works! The
need for this is explained in Apple Tech Note 262, which was issued in response
to my bug report in summer of '89.

PatchMacIIciVideoDriver();
Fixes a bug in the Mac IIci built-in video driver that would crash GDGetEntries.
The patch persists until reboot. It is unlikely that you will need to call this
explicitly, PatchMacIIciVideoDriver() is automatically invoked the first time
GDGetEntries is called. The Mac IIci built-in video driver
(.Display_Video_Apple_RBV1 driver, version 0) has a bug that causes it to crash
if you try to do a GetEntries Status request. PatchMacIIciVideoDriver() finds
and patches the copy of the buggy driver residing in memory. (If the driver is
ROM-based it first moves it to RAM, and then patches that.) Only two
instructions are modified, to save & restore more registers. This fix persists
only until the next reboot. If the patch is successfully applied the version
number is increased by 100.

NOTES:

Several bugs in version 2 (in System ≤6.03) of the video driver for Apple's
"Toby Frame Buffer" video card that affected use of the GetGamma call were
fixed in version 3 (in System 6.04), apparently in response to my bug report to
Apple.

The control/status-call parameter in a GDControl or GDStatus call specifies what
you want done. See Designing Cards and Drivers, 3rd Edition, Chapter 9. Note
that sometime around 1990 Apple renamed some of the Control and Status calls,
giving them names that better reflect their function. I followed suit. However,
Apple neglected to update their book, Designing Cards and Drivers, now in its
3rd edition. Fortunately, they define both names in their Video.h header file.
To avoid confusion, here are the equivalences. Note that "csc" stands for 
"Control and Status Call"
Control call:
 cscGrayPage =  cscGrayScreen = 5
Status calls:
 cscGetPageCnt = cscGetPages = 4
 cscGetPageBase = cscGetBaseAddr = 5

Based on:
Inside Macintosh-II (Device Manager)
Designing Cards and Drivers, 3rd Edition, Chapter 9.
Tech Note 262: "Controlling Status Calls"
"GreyScale Information" from AppleLink "Apple Technical Info"
"Video Configuration ROM Software Specification" also from AppleLink,
in "Developer Tech Support:Macintosh:32 Bit QuickDraw"

HISTORY:
9/29/89 	dgp	added caution above that successive calls to SetEntries may be on one 
				frame.
11/21/89	dgp	corrected mode list: 0x80... as pointed out by Chuck Stein
11/30/89	dgp	added note above from Don Kelly.
				Added cautionary note above about GDSetEntries().
3/1/90		dgp	updated comments.
3/3/90		dgp	included Video.h instead of defining VDSetEntryRecord and VDPageInfo.
3/20/90		dgp	made compatible with MPW C.
3/23/90		dgp	changed char to unsigned char in VDDefModeRec
				and VDFlagRec to prevent undesirable sign extension.
4/2/90		dgp	Deleted mode argument from GDGrayScreen().
7/28/90		dgp Fixed stack overflow in GDGrayScreen, by declaring SysEnvRec static.
10/20/90	dgp	Apple has renamed the control and status calls, so I followed suit:
				•GDGetPageBase replaces GDGetBaseAddr
				•GDReset replaces GDInit
				•GDGrayPage replace GDGrayScreen
				•GDGetPageCnt replaces GDGetPages
2/12/91		dgp Discovered that the old bug in GDGrayPage is still present in System
				6.05, so I removed the conditional around the bug fix. TestGDVideo
				now works with: Mac II Video Card, Mac II Display Card (4•8), 
				Mac IIci Built-in Video, TrueVision NuVista, Mac IIsi Built-in Video.
7/22/91		dgp	Changed definition of csGTable from GammaTblPtr to Ptr, 
				to conform with MPW C.
8/24/91		dgp	Made compatible with THINK C 5.0.
10/22/91	dgp	With help from Bart Farell, converted all functions headers to
				Standard C style.
8/26/92		dgp	added a few miscellaneous comments
				In all routines, *baseAddrPtr is now set only if baseAddrPtr is not NULL.
10/10/92	dgp	Added GDRestoreDeviceClut(). Removed obsolete support for THINK C 4.
11/30/92	dgp corrected error in comment documenting values of argument to GDSetGray().
				Set flag to zero for color,1 for luminance-mapped gray.
12/9/92		dgp	Enhanced GDRestoreDeviceClut(). Passing a NULL argument now requests
				application to all devices. I only just learned that Apple's
				RestoreDeviceClut() behaves in this way.
12/15/92	dgp Renamed GDRestoreDeviceClut to GDRestoreDeviceClut to be consistent
				with Apple's capitalization of RestoreDeviceClut.
12/16/92	dgp Updated comments to be consistent with 3rd edition of Designing Cards
				and Drivers. •Renamed myGDHandle to "device", which is easier to read.
				Renamed GDLinearGamma to GDUncorrectedGamma, and generalized it
				to work with any size DAC. For compatibility with old programs
				VideoToolbox.h now has a #define statement making GDLinearGamma
				an alias for GDUncorrectedGamma.
12/30/92	dgp Updated some of the comments. Eliminated Files.h.
12/30/92	dgp Use GDClutSize() to determine clut size.
1/4/93		dgp In GDClutSize, check for fixedType.
1/5/93		dgp In GDClutSize, only set last clut entry.
				Added PatchMacIIciVideoDriver() to the end of this file, and 
				automatically invoke it the first time GDGetEntries is called.
1/6/93		dgp	Cleaned up GDClutSize. It now seems to work correctly in the direct modes.
1/18/93	dgp	Spruced up the documentation.
			•Added all the code formerly in GDIdentify.c, but omitted the obsolete 
			function GDIdentify(), which simply printed GDName() and GDVersion.
			•Enhanced GDGetEntries() to avoid calling drivers that are known
			to crash, returning a statusErr instead.
			•Recoded PatchMacIIciVideoDriver() so as not to call GetScreenDevice.c.
2/1/93	dgp	Fixed endless loop in PatchMacIIciVideoDriver. Enhanced to deal with
			ROM-based drivers as well.
2/5/93	dgp	Expanded the documentation of GDSetEntries above, and supplied sample
			code showing how to load the clut.
2/7/93	dgp Fixed endless loop in PatchMacIIciVideoDriver.
2/20/93	dgp	Fixed bug in GDUncorrectedGamma() that was causing TestCluts to fail
			for video devices that have nonstandard gamma tables. 
3/18/93	dgp	Fixed divide by zero error in GDClutSize.
4/6/93	dgp	GDGetPageCnt() now sets *pagePtr argument only if no error occurs.
			Deleted GDModeName(). Removed assumption, in all routines, that there
			is any particular correspondence between the video mode number and
			the pixel size. Added two new routines, GDPixelSize and GDType,
			that return the pixelSize and gdType (i.e. fixedType, clutType, or 
			directType) of the device. Completely rewrote GDClutSize.
4/16/93	dgp	Streamlined GDClutSize() to make it fast enough for routine use.
4/19/93	dgp	Added GDNewLinearColorTable.
4/25/93	dgp	Made CntrlParam automatic instead of static.
4/25/93	dgp	Added GDColors(device) and GDPrintGammaTable(). Alphabetized the lists
			of functions in the documentation above.
5/11/93	dgp	Changed GDPrintGammaTable() to accept a simple file pointer instead of
			an array of two file pointers.
7/7/93	dgp enhanced GDDacSize() to return 8 unless the gamma table is present and
			reasonable.
*/

#include "VideoToolbox.h"
#include <assert.h>
#include <ROMDefs.h>
#define dRAMBased 0x0040

typedef struct VDFlagRec {
	unsigned char flag;
	char pad;
} VDFlagRec;

typedef struct VDDefModeRec{
	unsigned char spID;
	char pad;
} VDDefModeRec;

typedef struct {
	short flags;
	short blanks[3];
	short open;
	short prime;
	short control;
	short status;
	short close;
	Str255 name;
} VideoDriver;
VideoDriver *GDDriverAddress(GDHandle device);

typedef struct {
	short csCode;		// control code
	short length;		// total parameter block bytes
	char param[];		// control call data
} ScrnCtl;

typedef struct {
	short spDrvrHw;		// Slot Manager ID
	short slot;			// Number of slot
	long dCtlDevBase;	// Start of device's address space
	short mode;			// screen characteristics
	short flagMask;		// Which flag bits are used
	short flags;		// device state: bit 0 = 0 = mono; bit 0 = 1 = color;
						// bit 11 =  1 = startup device; bit 15 = 1 = active
	short colorTable;	// 'clut' id, default = -1
	short gammaTable;	// Selects color intensity, default (MacII) = -1
	Rect globalRect;	// global rectangle, main device topLeft = 0,0
	short ctlCount;		// total control calls
	ScrnCtl ctl;
} Scrn;

typedef struct {
	short scrnCount;	//Total devices
	Scrn scrn;
} Scrns;				// 'scrn' resource

Scrn **GDGetScrn(GDHandle device);

OSErr GDSetPageDrawn(GDHandle device,short page)
// Select a page of video memory to draw into.
// Untested.
{
	OSErr error;
	short flags;
	Ptr baseAddr;
	static long qD=-1;

	if(qD==-1)Gestalt(gestaltQuickdrawVersion,&qD);
	error=GDGetPageBase(device,page,&baseAddr);
	if(error)return error;
	if(qD>=gestalt32BitQD){
		flags=GetPixelsState((**device).gdPMap);
		LockPixels((**device).gdPMap);
	}
	(**(**device).gdPMap).baseAddr=baseAddr;
	if(qD>=gestalt32BitQD){
		GDeviceChanged(device);
		SetPixelsState((**device).gdPMap,flags);
	}
}

OSErr GDSetPageShown(GDHandle device,short page)
// Select a page of video memory for display.
// Untested.
{
	OSErr error;
	short mode;

	error=GDGetMode(device,&mode,NULL,NULL);
	if(error)return error;
	return GDSetMode(device,mode,page,NULL);
}

short GDType(GDHandle device)
// Returns what would normally be in (**device).gdType, for occasions when
// the GDevice record might be invalid because you called GDSetMode().
{
	OSErr error;
	ColorSpec white={255,0xffff,0xffff,0xffff},black={0,0,0,0};

	error=GDSetEntries(device,0,0,&white);
	if(!error)return clutType;
	error=GDDirectSetEntries(device,0,0,&black);
	if(!error)return directType;
	return fixedType;
}

short GDPixelSize(GDHandle device)
// Returns what would normally be in (**(**device).gdPMap).pixelSize, for occasions
// when the GDevice record might be invalid because you called GDSetMode().
{
	OSErr error;
	short mode;

	error=GDGetMode(device,&mode,NULL,NULL);
	return GDModePixelSize(device,mode);
}

short GDModePixelSize(GDHandle device,short mode)
// Returns the pixelSize associated with a given video mode.
// If you've changed the mode by calling GDSetMode and you're running a System 
// older than 6.0.5 the answer may may be wrong for some of the newer video cards,
// because they have ideosynchratic associations of video mode and pixel size.
{
	short j;
	long version;

	if(mode==(**device).gdMode)return (**(**device).gdPMap).pixelSize;
	Gestalt(gestaltSystemVersion,&version);
	if(version>=0x605){			// Need new Palette Manager for reliable answer.
		for(j=5;j>=0;j--)if(mode==HasDepth(device,1<<j,0,0))return 1<<j;
	} else return 1<<(mode&7);	// Unreliable.
	return 0;
}

Boolean GDHasMode(GDHandle device,short mode,short *pixelSizePtr,short *pagesPtr);

Boolean GDHasMode(GDHandle device,short mode,short *pixelSizePtr,short *pagesPtr)
// Returns 0 if no such mode, returns 1 if mode is available.
// If pixelSizePtr is not NULL, then sets *pixelSizePtr to pixelSize or -1 if unknown.
// If pagesPtr is not NULL, then sets *pagesPtr to pages.
{
	short pixelSize,i,hasDepthWorks,error,pages;
	long system;
	
	Gestalt(gestaltSystemVersion,&system);
	// On Mac IIci, Sys 6.07, HasDepth returns "mode" of 0x100 at all legal depths.
	hasDepthWorks= system>=0x605 	// New Palette Manager.
		&& HasDepth(device,(**(**device).gdPMap).pixelSize,0,0)==(**device).gdMode;
	if(hasDepthWorks){
		for(i=0;i<6;i++){
			pixelSize=1<<i;
			if(mode!=HasDepth(device,pixelSize,0,0))continue;
			if(pixelSizePtr!=NULL)*pixelSizePtr=pixelSize;
			if(pagesPtr!=NULL){
				error=GDGetPageCnt(device,mode,&pages);
				if(error)pages=1;
				*pagesPtr=pages;
			}
			return 1;
		}
		return 0;
	}else{
		// If HasDepth doesn't work properly then we can still find out whether the
		// mode is available by asking the video driver for a page count, but
		// I don't know of any discreet way to find out the pixelSize.
		// Calling SetDepth would work, but that would irritate the user who has to
		// watch and wait. 
		error=GDGetPageCnt(device,mode,&pages);
		if(error)pages=0;
		if(pagesPtr!=NULL)*pagesPtr=pages;
		if(mode==(**device).gdMode)pixelSize=(**(**device).gdPMap).pixelSize;
		else pixelSize=-1;		// Unknown.
		if(pixelSizePtr!=NULL)*pixelSizePtr=pixelSize;
		return (pages>0);
	}
}

short gdClutSizeTable[33]={0,1<<1,1<<2,0,1<<4,0,0,0,1<<8,0,0,0,0,0,0,0,1<<5
,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1<<8};

long GDColors(GDHandle device)
// Returns the number of colors, in the current mode.
{
	long colors=0;
	short pixelSize;
	
	if(device==NULL)return 2;	// for compatibility with 1-bit QuickDraw
	pixelSize=(**(**device).gdPMap).pixelSize;
	if(pixelSize>=1 && pixelSize<=8)colors=1<<pixelSize;
	if(pixelSize==16)colors=1L<<15;
	if(pixelSize==32)colors=1L<<24;
	return colors;
}

short GDClutSize(GDHandle device)
// Returns the number of entries in the clut, in the current mode.
{
	short clutSize;
	
	// Method 1. Estimate clut size from pixel size.
	clutSize=gdClutSizeTable[(**(**device).gdPMap).pixelSize];

	#if 0
	// Method 2. Measure the clut's size by trying to load it.
	if(GDType(device)==directType){
		int error,i;
		ColorSpec *table,white={255,0xffff,0xffff,0xffff},black={0,0,0,0};
		for(clutSize=256;clutSize>1;clutSize>>=1){
			table=GDNewLinearColorTable(device);
			if(table==NULL)PrintfExit("GDClutSize: out of memory.\n");
			error=GDDirectSetEntries(device,0,clutSize-1,table);
			DisposePtr((Ptr)table);
			if(!error)break;
		}
	}
	#endif
	
	return clutSize;
}

short GDDacSize(GDHandle device)
// Figures out how many bits in the video dac. Answers for each device are cached.
{
	short dacSize,i;
	static short dacSizeCache[MAX_SCREENS];
	static GDHandle deviceCache[MAX_SCREENS];
	OSErr error;
	GammaTbl *gammaTblPtr=NULL;

	for(i=0;i<MAX_SCREENS;i++)if(device==deviceCache[i])return dacSizeCache[i];
	error=GDGetGamma(device,&gammaTblPtr);		// Takes 200 µs.
	if(error || gammaTblPtr==NULL || gammaTblPtr->gDataWidth==0)dacSize=8;							// Oops. Take a guess.
	else dacSize=gammaTblPtr->gDataWidth;
	for(i=0;i<MAX_SCREENS;i++)if(NULL==deviceCache[i]){
		deviceCache[i]=device;
		dacSizeCache[i]=dacSize;
		break;
	}
	return dacSize;
}

ColorSpec *GDNewLinearColorTable(GDHandle device)
// Creates a default table for use when gdType==directType.
{
	short clutSize,i;
	ColorSpec *table,*linearTable;
	
	clutSize=GDClutSize(device);
	table=linearTable=(ColorSpec *)NewPtr(clutSize*sizeof(linearTable[0]));
	if(linearTable!=NULL)for(i=0;i<clutSize;i++){
		table->rgb.red=table->rgb.green=table->rgb.blue
			=(0xffffL*i+clutSize/2)/(clutSize-1);
		table++;
	}
	return linearTable;
}

/*
Nominally equivalent to Apple's RestoreDeviceClut(), which is only available
since System 6.05. However, I find that Apple's routine sometimes does nothing,
whereas this routine always works. Passing a NULL argument causes restoration of
cluts of all video devices.
*/
OSErr GDRestoreDeviceClut(GDHandle device)
{
	OSErr error,lastError;
	short count;
	long i;

	// If NULL, then recursively call ourselves for each device.
	if(device==NULL){
		lastError=0;
		device=GetDeviceList();
		while(device!=NULL) {
			if(TestDeviceAttribute(device,screenDevice))
				error=GDRestoreDeviceClut(device);
			if(error)lastError=error;
			device=GetNextDevice(device);
		}
		return lastError;
	}
	if(GDType(device)!=directType){
		count=(**(**(**device).gdPMap).pmTable).ctSize;
		error=GDSetEntries(device,0,count
			,((**(**(**device).gdPMap).pmTable)).ctTable);
	} else error=GDSetGamma(device,NULL);
	return error;
}

GammaTbl **savedGammaTable[MAX_SCREENS];

OSErr GDSaveGamma(GDHandle device)
{
	GammaTbl *gamma;
	OSErr error;
	long size;
	int i;

	if(device==NULL || (*device)->gdType==fixedType)return 0;
	i=GetScreenIndex(device);
	if(i>=MAX_SCREENS)return 1;
 	error=GDGetGamma(device,&gamma);
	if(error)return error;
	size=gamma->gChanCnt*gamma->gDataCnt;
	if(gamma->gDataWidth>8)size*=2;
	size+=sizeof(GammaTbl)+gamma->gFormulaSize;
	if(savedGammaTable[i]!=NULL){
		DisposeHandle((Handle)savedGammaTable[i]);
		savedGammaTable[i]=NULL;
	}
	error=PtrToHand(gamma,(Handle *)&savedGammaTable[i],size);
	return error;
}

OSErr GDRestoreGamma(GDHandle device)
{
	int i;
	OSErr error;
	
	if((*device)->gdType==fixedType)return 0;
	i=GetScreenIndex(device);
	if(i>=MAX_SCREENS || savedGammaTable[i]==NULL)return 1;
	error=GDSetGamma(device,*savedGammaTable[i]);
	if(error)return error;
	DisposeHandle((Handle)savedGammaTable[i]);
	savedGammaTable[i]=NULL;
	return 0;
}	

OSErr GDGetDefaultGamma(GDHandle device,GammaTbl **gammaTbl)
// Looks for a default gamma table in both places that Apple says to look,
// but usually comes up empty handed.
{
	OSErr error;
	int i,j,slot;
	Scrn **scrn;
	GammaTbl **gammaHandle,*gamma;
	SpBlock spBlock;
	Ptr ptr;

	gammaHandle=NULL;
	if(CountResources('gama')>0){// Any 'gama' resources available in System file?
		// Check to see if 'scrn' resource in System file specifies a 'gama' resource.
		scrn=GDGetScrn(device);
		if(scrn!=NULL && (**scrn).gammaTable!=-1)
			gammaHandle=(GammaTbl **)GetResource('gama',(**scrn).gammaTable);
		DisposeHandle((Handle)scrn);
	}
	if(gammaHandle!=NULL){
		// Got gamma table from 'gama' resource.
		gamma=(GammaTbl *)NewPtr(GetHandleSize((Handle)gammaHandle));
		if(gamma==NULL)return MemError();
		BlockMove(*gammaHandle,gamma,GetHandleSize((Handle)gammaHandle));
		DisposeHandle((Handle)gammaHandle);
	}else{
		// Try to get this device's default gamma table from Slot Manager.
		spBlock.spSlot=slot=GetDeviceSlot(device);
		spBlock.spID=0;
		spBlock.spExtDev=0;
		spBlock.spTBMask=3;			// match only spCategory and spCType
		spBlock.spCategory=catDisplay;
		spBlock.spCType=typeVideo;	// this might be too restrictive, excludes LCD
		do{
			error=SNextTypeSRsrc(&spBlock);
			if(error)return error;
		}while(spBlock.spSlot!=slot || spBlock.spRefNum!=(**device).gdRefNum);
		
		if(0){
			// Print sResource name
			spBlock.spID=sRsrcName;
			error=SGetCString(&spBlock);
			printf("Slot resource “%s”\n",spBlock.spResult);
			if(error)return error;
			DisposePtr((Ptr)spBlock.spResult);
		}
		
		// Look for gamma directory. Unfortunately many video devices don't have one.
		spBlock.spID=sGammaDir;
		error=SFindStruct(&spBlock);
		if(error)return error;
		
		// Retrieve default gamma table
		spBlock.spID=0x80;
		error=SGetBlock(&spBlock);
		if(error)return error;
		gamma=(GammaTbl *)spBlock.spResult;
		ptr=(Ptr)spBlock.spResult+6;
		if(0)printf("Gamma table “%s”, %ld bytes\n",ptr,GetPtrSize((Ptr)gamma));
		ptr+=strlen(ptr)+1;				// table is just past string
		ptr=(Ptr)((long)(ptr+1)&~1L);	// round up to even address
		i=ptr-(Ptr)gamma;
		BlockMove(ptr,(Ptr)gamma,GetPtrSize((Ptr)gamma)-i);
		SetPtrSize((Ptr)gamma,GetPtrSize((Ptr)gamma)-i);
	}
	*gammaTbl=gamma;
	return 0;
}

Scrn **GDGetScrn(GDHandle device)
// Returns handle to a copy of the specific scrn resource for this device,
// or NULL if none.
{
	OSErr error=0;
	int i,j;
	Scrns **scrns;
	Scrn *scrn,**scrnHandle;
	ScrnCtl *ctl;
	int scrnCount;
	long size;
	short mode;

	scrns=(Scrns **)GetResource('scrn',0);
	if(ResError())return NULL;
	HLockHi((Handle)scrns);
	scrnCount=(**scrns).scrnCount;
	scrn=&(**scrns).scrn;
	for(i=0;i<scrnCount;i++){
		if(0 && scrn->slot==GetDeviceSlot(device)){
			printf("Slot %d,",scrn->slot);
			printf("mode %d,",scrn->mode);
			printf("colorTable %d,",scrn->colorTable);
			printf("gammaTable %d,",scrn->gammaTable);
			printf("%d control calls:",scrn->ctlCount);
		}
		ctl=&scrn->ctl;
		for(j=0;j<scrn->ctlCount;j++){
			if(0 && scrn->slot==GetDeviceSlot(device))printf(" %d",ctl->csCode);
			ctl=(ScrnCtl *)((long)(ctl+1)+ctl->length);
		}
		if(0 && scrn->slot==GetDeviceSlot(device))printf("\n");
		size=(long)ctl-(long)scrn;
		if(scrn->slot==GetDeviceSlot(device))break;
		scrn=(Scrn *)ctl;
	}
	if(i<scrnCount){
		if(error)return NULL;
		scrnHandle=(Scrn **)NewHandle(size);
		BlockMove(scrn,*scrnHandle,size);
	}else scrnHandle=NULL;
	ReleaseResource((Handle)scrns);
	return scrnHandle;
}

OSErr GDUncorrectedGamma(GDHandle device)
/*
Loads a linear gamma table into the specified video device, to defeat the
driver's attempt to do gamma correction.

According to Designing Cards and Drivers, 3rd edition, passing a NULL
GammaTblPtr instructs the driver to create a linear table. So you may wish to
just call GDSetGamma(device,NULL) instead of calling GDUncorrectedGamma(device).
However, I'm not sure how old that rule is, and don't know if all the older
drivers support it. Therefore the following code first examines the current
gamma table, and, if it's a plain vanilla table, as described in Designing Cards
and Drivers, then we write in the desired linear table. If it's fancy (or if the
driver doesn't support GDGetGamma) then we pass a NULL pointer, letting the driver
create the new table. Hopefully this will work with all drivers.
*/
{
	OSErr error;
	GammaTbl *gamma=NULL;
	int i;
	char *gData;

	error=GDGetGamma(device,&gamma);
	if(!error && gamma->gVersion==0 && gamma->gChanCnt==1 && gamma->gDataWidth<=8){
		// Overwrite the standard table
		gData = (char *)&gamma->gFormulaData+gamma->gFormulaSize;
		for(i=0;i<gamma->gDataCnt;i++)gData[i]=i;
	}else{
		// A fancy table implies a new driver, so let it do the work.
		gamma=NULL;
	}
	error=GDSetGamma(device,gamma);
	return error;
}

/* KillIO doesn't do anything, so I didn't bother to implement it. */

OSErr GDPrintGammaTable(FILE *o,GDHandle device)
{
	OSErr error;
	unsigned char *byte;
	unsigned short *word;
	int i,j,identity;
	GammaTbl *gamma;
	
	if((**device).gdType==fixedType)return statusErr;
	error=GDGetGamma(device,&gamma);
	if(error){
		fprintf(o,"GetGamma: GDGetGamma() error %d\n",error);
		if(error==statusErr)
			fprintf(o,"The video driver doesn't support this call.\n");
		return error;
	}
	byte=(unsigned char *)gamma->gFormulaData+gamma->gFormulaSize;
	word=(unsigned short *)byte;
	identity=1;
	if(gamma->gDataWidth<=8)
		for(i=0;i<gamma->gDataCnt;i++)identity &= (i==byte[i]);
	else
		for(i=0;i<gamma->gDataCnt;i++)identity &= (i==word[i]);
	if(identity){
		fprintf(o,"Gamma Table: identity transformation\n");
	}else{
		fprintf(o,"Gamma Table:\n");
		fprintf(o,"at 0x%lx,gDataWidth %d,gDataCnt %d,gVersion %d,gType %d,gFormulaSize %d,gChanCnt %d\n"
			,gamma,gamma->gDataWidth,gamma->gDataCnt,gamma->gVersion,gamma->gType,gamma->gFormulaSize,gamma->gChanCnt);
		for(i=0;i<gamma->gDataCnt;i+=64) {
			fprintf(o,"%3d: ",i);
			if(gamma->gDataWidth<=8)
				for(j=0;j<16;j++) fprintf(o," %3u",byte[i+j]);
			else
				for(j=0;j<16;j++) fprintf(o," %3u",word[i+j]);
			fprintf(o,"\n");
		}
	}
	return 0;
}

OSErr GDReset(GDHandle device, short *modePtr, short *pagePtr, Ptr *baseAddrPtr)
/*
Initialize the video card to its startup state, usually 1 bit per pixel. Returns
the parameters of that state.
*/
{
	VDPageInfo myVDPageInfo;
	OSErr error;

	if(device==NULL) return controlErr;
	myVDPageInfo.csMode= *modePtr;
	myVDPageInfo.csPage= *pagePtr;
	error=GDControl((*device)->gdRefNum,cscReset,(Ptr) &myVDPageInfo);
	*modePtr=myVDPageInfo.csMode;
	*pagePtr=myVDPageInfo.csPage;
	if(baseAddrPtr!=NULL) *baseAddrPtr=myVDPageInfo.csBaseAddr;
	return error;
}

OSErr GDSetMode(GDHandle device,short mode,short page,Ptr *baseAddrPtr)
{
	VDPageInfo myVDPageInfo;
	OSErr error;

	if(device==NULL) return controlErr;
	myVDPageInfo.csMode=mode;
	myVDPageInfo.csPage=page;
	error=GDControl((*device)->gdRefNum,cscSetMode,(Ptr) &myVDPageInfo);
	if(baseAddrPtr!=NULL) *baseAddrPtr=myVDPageInfo.csBaseAddr;
	return error;
}

OSErr GDSetEntriesByType(GDHandle device,short start,short count,ColorSpec *table)
// Calls GDSetEntries or GDDirectSetEntries or nothing, as appropriate.
// Assumes that the GDevice record is valid, i.e. that the user has not
// called GDSetMode.
{
	switch((*device)->gdType){
	case fixedType:
		return statusErr;
	case clutType:
		return GDSetEntries(device,start,count,table);
	case directType:
		return GDDirectSetEntries(device,start,count,table);
	}
}

OSErr GDSetEntriesByTypeHighPriority(GDHandle device,short start,short count
	,ColorSpec *table)
{
	char priority;
	OSErr error;

	priority=7;
	SwapPriority(&priority);
	error=GDSetEntriesByType(device,start,count,table);
	SwapPriority(&priority);
	return error;
}

OSErr GDSetEntries(GDHandle device,short start,short count,ColorSpec *table)
{
	VDSetEntryRecord myVDSetEntryRecord;
	OSErr error;

	if(device==NULL) return controlErr;
	myVDSetEntryRecord.csStart=start;
	myVDSetEntryRecord.csCount=count;
	myVDSetEntryRecord.csTable=table;
	error=GDControl((*device)->gdRefNum,cscSetEntries,(Ptr) &myVDSetEntryRecord);
	return error;
}

OSErr GDSetGamma(GDHandle device, GammaTbl *gamma)
{
	OSErr error;
	VDGammaRecord myVDGammaRecord;

	myVDGammaRecord.csGTable=(Ptr)gamma;
	error=GDControl((*device)->gdRefNum,cscSetGamma,(Ptr) &myVDGammaRecord);
	return error;
}

OSErr GDGrayPage(GDHandle device,short page)
/*
Called "GrayScreen" in Designing Cards and Drivers, 3rd Ed. Fills the specified
page with gray, i.e. the dithered desktop pattern. I'm not aware of any
particular advantage in using this instead of FillRect().

Designing Cards and Drivers, 3rd Edition, Chapter 9, says that for direct
devices, i.e. >8 bit pixels, the driver will also load a linear,
gamma-corrected, gray color table.

Contrary to the documentation, version 2 (in System 6.03) of the video driver
for Apple's old video card requires that one supply the current mode as well.
Supplying a garbage mode screwed up the screen and soon hung the software. So
this code first obtains the current mode, and then supplies it in the GrayPage
Control call.
*/
{
	VDPageInfo myVDPageInfo;
	OSErr error;
	/* The rest of the arguments are used soley for the bug fix */
	short mode=0;		/* should be ignored, but isn't */
	short actualPage;	/* ignored */
	Ptr baseAddr;		/* ignored */

	if(device==NULL) return controlErr;
	
	/* Work around driver bug: get the mode */
	error=GDGetMode(device,&mode,&actualPage,&baseAddr);
	if(error)return error;
	myVDPageInfo.csMode=mode;
	
	myVDPageInfo.csPage=page;
	error=GDControl((*device)->gdRefNum,cscGrayPage,(Ptr) &myVDPageInfo);
	return error;
}

OSErr GDSetGray(GDHandle device,Boolean flag)
/*
Tells the driver whether you want colors (flag==0), or prefer that all colors be 
mapped to luminance-equivalent gray tones? (flag==1).
*/
{
	VDFlagRec myVDFlagRec;
	OSErr error;

	if(device==NULL) return controlErr;
	myVDFlagRec.flag=flag;
	error=GDControl((*device)->gdRefNum,cscSetGray,(Ptr) &myVDFlagRec);
	return error;
}

OSErr GDSetInterrupt(GDHandle device,Boolean flag)
/*
Set flag to 1 to enable VBL interrupts of this card, or 0 to disable. 
I don't know when it's appropriate to call this.
*/
{
	VDFlagRec myVDFlagRec;
	OSErr error;

	if(device==NULL) return controlErr;
	myVDFlagRec.flag=flag;
	error=GDControl((*device)->gdRefNum,cscSetInterrupt,(Ptr) &myVDFlagRec);
	return error;
}

OSErr GDDirectSetEntries(GDHandle device,short start,short count,ColorSpec *table)
/*
If your pixel depth is >8 then the setEntries Control call is disabled, and you use
this instead. Except for that, GDDirectSetEntries is identical to GDSetEntries.
*/
{
	VDSetEntryRecord myVDSetEntryRecord;
	OSErr error;

	if(device==NULL) return controlErr;
	myVDSetEntryRecord.csStart=start;
	myVDSetEntryRecord.csCount=count;
	myVDSetEntryRecord.csTable=table;
	error=GDControl((*device)->gdRefNum,cscDirectSetEntries,(Ptr) &myVDSetEntryRecord);
	return error;
}

OSErr GDSetDefaultMode(GDHandle device,short mode)
/*
Supposedly, you tell it what mode you want to start up with when you reboot.
(I've never been able to get this to work. No error and no effect. Perhaps I've
misunderstood its purpose.)
*/
{
	VDDefModeRec myVDDefModeRec;
	OSErr error;

	if(device==NULL) return controlErr;
	myVDDefModeRec.spID=mode;
	error=GDControl((*device)->gdRefNum,cscSetDefaultMode,(Ptr) &myVDDefModeRec);
	return error;
}

OSErr GDGetMode(GDHandle device,short *modePtr,short *pagePtr,Ptr *baseAddrPtr)
/*
It tells you the current mode, page of video memory, and the base address of that
page.
*/
{
	VDPageInfo myVDPageInfo;
	OSErr error;

	if(device==NULL) return statusErr;
	error=GDStatus((*device)->gdRefNum,cscGetMode,(Ptr) &myVDPageInfo);
	if(modePtr!=NULL)*modePtr=myVDPageInfo.csMode;
	if(pagePtr!=NULL)*pagePtr=myVDPageInfo.csPage;
	if(baseAddrPtr!=NULL) *baseAddrPtr=myVDPageInfo.csBaseAddr;
	return error;
}

OSErr GDGetEntries(GDHandle device,short start,short count,ColorSpec *table)
/*
This is much as you'd expect after reading GDSetEntries above. Note that unless
the gamma table is linear, the values returned may not be the same as those
originally passed by GDSetEntries. So call GDUncorrectedGamma first. Note that
version 2 (in System 6.03) of the video driver for Apple's old video card had a
bug and did not support "indexed" mode, i.e. start==-1. This is fixed in System
6.05. Apple's .Display_Video_Apple_RBV1 v. 0 video driver (built-in to Mac IIci)
crashes if you attempt to make this call. However, that's a thing of the past,
because we now first patch the driver to fix the bug. Try the demo TimeVideo.
*/
{
	VDSetEntryRecord myVDSetEntryRecord;
	OSErr error;
	static int bugsPatched=0;
	unsigned char *name;
	int version;

	if(device==NULL)return statusErr;
	if(!bugsPatched){
		PatchMacIIciVideoDriver();
		bugsPatched=1;
	}

	// Contrary to Apple's rules, these drivers crash if we attempt
	// a getEntries call. So let's be polite and instead simply return
	// an error message indicating that this call is not available.
	name=GDName(device);
	version=GDVersion(device);
	if(EqualString(name,"\p.Display_Video_Apple_RBV1",1,1) && version==0)return statusErr;
	if(EqualString(name,"\p.Color_Video_Display",1,1) && version==9288)return statusErr;

	myVDSetEntryRecord.csStart=start;
	myVDSetEntryRecord.csCount=count;
	myVDSetEntryRecord.csTable=table;
	error=GDStatus((*device)->gdRefNum,cscGetEntries,(Ptr) &myVDSetEntryRecord);
	return error;
}

OSErr GDGetPageCnt(GDHandle device,short mode,short *pagesPtr)
/*
Called "GetPages" in Designing Cards and Drivers, 3rd Ed. You tell it what mode
you're interested in. It tells you how many pages of video ram are available.
*/
{
	VDPageInfo myVDPageInfo;
	OSErr error;

	if(device==NULL) return statusErr;
	myVDPageInfo.csMode=mode;
	error=GDStatus((*device)->gdRefNum,cscGetPageCnt,(Ptr) &myVDPageInfo);
	if(!error)*pagesPtr=myVDPageInfo.csPage;
	return error;
}

OSErr GDGetPageBase(GDHandle device,short page,Ptr *baseAddrPtr)
/*
Called "GetBaseAddr" in Designing Cards and Drivers, 3rd Ed. You tell it what
page of video memory you're interested in (in the current video mode). It tells
you the base address of that page.
*/
{
	VDPageInfo myVDPageInfo;
	OSErr error;

	if(device==NULL) return statusErr;
	myVDPageInfo.csPage=page;
	error=GDStatus((*device)->gdRefNum,cscGetPageBase,(Ptr) &myVDPageInfo);
	if(baseAddrPtr!=NULL) *baseAddrPtr=myVDPageInfo.csBaseAddr;
	return error;
}

OSErr GDGetGray(GDHandle device,Boolean *flagPtr)
// It tells you what the flag is set to. Do you want colors? (flag==0) Or do you
// want all colors mapped to luminance-equivalent gray tones? (flag==1).
{
	VDFlagRec myVDFlagRec;
	OSErr error;

	if(device==NULL) return statusErr;
	error=GDStatus((*device)->gdRefNum,cscGetGray,(Ptr) &myVDFlagRec);
	*flagPtr=myVDFlagRec.flag;
	return error;
}

OSErr GDGetInterrupt(GDHandle device,Boolean *flagPtr)
// Get flag. 1 if VBL interrupts of this card are enabled. 0 if disabled. 
{
	VDFlagRec myVDFlagRec;
	OSErr error;

	if(device==NULL) return statusErr;
	error=GDStatus((*device)->gdRefNum,cscGetInterrupt,(Ptr) &myVDFlagRec);
	*flagPtr=myVDFlagRec.flag;
	return error;
}

OSErr GDGetGamma(GDHandle device,GammaTbl **myGammaTblHandle)
/*
Returns a pointer to the Gamma table in the specified video device. (I.e. you
pass it a pointer to your pointer, a handle, which it uses to load your
pointer.)

Note that version 2 (in System ≤6.03) of the video driver for Apple's old video
card does not support this call due to a bug in the driver code. The later
versions of the driver (3, 4, and 5, in System 6.04 and later) work correctly.
*/
{
	OSErr error;
	VDGammaRecord myVDGammaRecord;

	if(device==NULL) return statusErr;
	myVDGammaRecord.csGTable=NULL;	// default address is NULL
	error=GDStatus((*device)->gdRefNum,cscGetGamma,(Ptr) &myVDGammaRecord);
	*myGammaTblHandle=(GammaTblPtr)myVDGammaRecord.csGTable;
	return error;
}

OSErr GDGetDefaultMode(GDHandle device,short *modePtr)
// It tells you what the default mode is. I'm not sure what this means.
{
	VDDefModeRec myVDDefModeRec;
	OSErr error;

	if(device==NULL) return statusErr;
	error=GDStatus((*device)->gdRefNum,cscGetDefaultMode,(Ptr) &myVDDefModeRec);
	*modePtr=(unsigned char)myVDDefModeRec.spID;
	return error;
}

OSErr GDControl(int refNum,int csCode,Ptr csParamPtr)
// Uses low-level PBControl() call to implement a "Control()" call that works! 
// I don't know why this wasn't discussed in Apple Tech Note 262.
{
	CntrlParam control;
	OSErr error;
	
	control.ioCompletion=NULL;
	control.ioCRefNum=refNum;
	control.csCode=csCode;
	*((Ptr *) &control.csParam[0]) = csParamPtr;
	error=PBControl((ParmBlkPtr) &control,0);
	return error;
}

OSErr GDStatus(int refNum,int csCode,Ptr csParamPtr)
// Uses low-level PBStatus() call to implement a "Status()" call that works! The
// need for this is explained in Apple Tech Note 262, which was issued in response
// to my bug report in summer of '89.
{
	CntrlParam control;
	OSErr error;
	
	control.ioCompletion=NULL;
	control.ioCRefNum=refNum;
	control.csCode=csCode;
	*((Ptr *) &control.csParam[0]) = csParamPtr;
	error=PBStatus((ParmBlkPtr) &control,0);
	return error;
}

#if 0
/*
From: absurd@apple.apple.com (Tim Dierks, software saboteur)
Date: Fri, 20 Nov 1992 00:38:14 GMT
Organization: MacDTS Marauders
 
Here's the right way to get the slot number of a monitor, given its
GDevice:  (as a bonus, this also gets the name of the card in
question; to just get the slot, chop off all the lines after the
*slot = ... line.
*/

OSErr GetSlotAndName(GDHandle gDev,short *slot,char *name)
{   OSErr       err;
    short       refNum;
    SpBlock     spb;
    
    refNum = (**gDev).gdRefNum;    // video driver refNum for this GDevice
    *slot = (**(AuxDCEHandle)GetDCtlEntry(refNum)).dCtlSlot;
                                   // slot in which this video card sits
    spb.spSlot = *slot;         // In the slot we're interested in
    spb.spID = 0;
    spb.spExtDev = 0;
    spb.spCategory = 1;         // Get the board sResource
    spb.spCType = 0;
    spb.spDrvrSW = 0;
    spb.spDrvrHW = 0;
    spb.spTBMask = 0;
    err = SNextTypeSRsrc(&spb);
    if (err)return err;
    spb.spID = 2;               // Getting the description string
                                // spSPointer was set up by SNextTypesResource
    err = SGetCString(&spb);
    if (err)return err;
    strcpy(name,(char *)spb.spResult);  // Get the returned string
    DisposPtr((Ptr)spb.spResult);    // Undocumented; we have to dispose of it
    c2pstr(name);
    return noErr;
}
#endif

char *GDCardName(GDHandle device)
/*
Returns the address of a C string containing the name of the video card. You
should call DisposPtr() on the returned address when you no longer need it.
Takes about 1.5 ms; I don't know why Apple's slot routines are so slow. This
routine sets up the flags in the SNextTypeSRsrc() call quite differently from
the above example, but I don't know if that matters, since I've been using this
routine for a year or so without any problems.
*/
{
	AuxDCE **auxDCEHandle;
	SpBlock SPB,SPB1;
	
	if(device==NULL)return "";
	auxDCEHandle = (AuxDCE **) GetDCtlEntry((*device)->gdRefNum);
	SPB.spSlot = (**auxDCEHandle).dCtlSlot;
	SPB.spCategory = 0;
	SPB.spCType = 	0;
	SPB.spDrvrSW = 	0;
	SPB.spDrvrHW = 	1;
	SPB.spTBMask = 0xf;
	SPB.spID = 0;
	SPB.spExtDev = 0;
	if(SNextTypeSRsrc(&SPB) == noErr) {
		SPB1.spsPointer = SPB.spsPointer;
		SPB1.spID = 2;
		SGetCString(&SPB1);
		return (char *)SPB1.spResult;
	}
	else return "";
}

unsigned char *GDName(GDHandle device)
// Returns a pointer to the name of the driver (a pascal string). 
{
	VideoDriver *videoDriverPtr;

	if(device==NULL)return "\p";
	videoDriverPtr=GDDriverAddress(device);
	return videoDriverPtr->name;
}

int GDVersion(GDHandle device)
// Returns the version number of the driver. From the first word-aligned word after
// the name string.
{
	int version;
	unsigned char *bytePtr;

	if(device==NULL)return 0;
	bytePtr=GDName(device);
	bytePtr += 1+bytePtr[0];	/* go to end of Pascal string */
	bytePtr = (unsigned char *)((long)(bytePtr+1) & ~1);	// round up to word boundary
	version = *(short *)bytePtr;
	return version;
}

VideoDriver *GDDriverAddress(GDHandle device)
// Returns a pointer to the driver, whether in ROM or RAM. 
{
	AuxDCE **auxDCEHandle;
	VideoDriver *videoDriverPtr;

	if(device==NULL)return NULL;
	auxDCEHandle = (AuxDCE **) GetDCtlEntry((*device)->gdRefNum);
	if((**auxDCEHandle).dCtlFlags & dRAMBased){
		/* RAM-based driver. */
		videoDriverPtr=*(VideoDriver **) (**auxDCEHandle).dCtlDriver;
	}
	else{
		/* ROM-based driver. */
		videoDriverPtr=(VideoDriver *) (**auxDCEHandle).dCtlDriver;
	}
	return videoDriverPtr;
}

/*
ROUTINE: PatchMacIIciVideoDriver
PURPOSE:
It is unlikely that you will need to call this explicitly, because it is called
automatically by GDGetEntries the first time it is invoked, and the sole purpose
of this routine is to fix a driver bug that would cause a crash when called by
GDGetEntries.

The Mac IIci built-in video driver (.Display_Video_Apple_RBV1 driver, version 0)
has a bug that causes it to crash if you try to do a getEntries Status request.
PatchMacIIciVideoDriver() will find and patch the memory-resident copy of the
buggy driver. Only two instructions are modified, to save and restore more
registers. This fix persists only until the next reboot. If the patch is
successfully applied the version number is increased from 0 to 100, to
distinguish it from versions 0 and 1.

A returned value of 1 indicates success: the needed patch was applied, either now
or previously. A return value of 0 indicates no patch was needed.

This patch is based on a comparison of the version 0 and 1 drivers used by the
Mac IIci and IIsi, respectively. The patch changes a pair of save and restore
operations (MOVEM.L to and from the stack) to save and restore registers D1 and
A1 as well as D4, A3, and A4. There are many other differences between versions
0 and 1 of the driver, but this change is enough to keep the version 0 driver
from crashing when we attempt to read the clut by calling GDGetEntries.

The only change that the Mac operating system could possibly notice is that,
when we're done patching, we set the handle to be non-purgeable, since purging
and reloading the driver would eliminate the patch.

edward_de_Jong@bmug.org and Robert Savoy (SAVOY@RISVAX.ROWLAND.ORG) reported
that their Mac IIci computers' built-in video driver is ROM-based, so
PatchMacIIciVideoDriver was enhanced to deal with a ROM-based driver, by copying
the driver into RAM, making that the active driver, and patching it. Robert
Savoy reports that it works fine.

An alternative, permanent, solution is described in the file "Video synch":
upgrading to Apple's bug-free version 1 of the driver. However, that solution
only works if the Mac IIci has more than one monitor.

*/

int PatchMacIIciVideoDriver(void)
{
	GDHandle device;
	int i;
	short *w;
	AuxDCE **auxDCEHandle;
	Handle handle;
	enum{badVersion=0};
	int error;
	long value;
	
	error=Gestalt(gestaltQuickdrawVersion,&value);
	if(error || value<gestalt8BitQD)return 0;	// need 8-bit quickdraw
	device = GetDeviceList();
	while(1) {
		if(device==NULL)return 0;
		if (!TestDeviceAttribute(device,screenDevice)
			|| !TestDeviceAttribute(device,screenActive)){
			device=GetNextDevice(device);
			continue;
		}
		if(EqualString("\p.Display_Video_Apple_RBV1",GDName(device),1,1))
			switch(GDVersion(device)){
				case badVersion:
					break;
				case badVersion+100:	// already patched
					return 1;
				default:
					return 0;
		}else{
			device=GetNextDevice(device);
			continue;
		}
		break;
	}
	auxDCEHandle = (AuxDCE **) GetDCtlEntry((*device)->gdRefNum);
	
	// Move ROM-based driver into RAM.
	if(!((**auxDCEHandle).dCtlFlags & dRAMBased)){
		long bytes;
		VideoDriver *driver;
		
		driver=(VideoDriver *)(**auxDCEHandle).dCtlDriver;
		// Sometimes the word preceding the driver in ROM seems to be the
		// driver size, but not always, e.g. the built-in driver on the Mac IIsi.
		bytes=*((short *)driver-1);
		// Driver size unknown, guessing (generously) at twice the highest offset.
		bytes=driver->open;
		if(bytes<driver->prime)bytes=driver->prime;
		if(bytes<driver->control)bytes=driver->control;
		if(bytes<driver->status)bytes=driver->status;
		if(bytes<driver->close)bytes=driver->close;
		bytes*=2;
		// We know the Mac IIci driver size to be 1896
		// when ROM version is 124 rev. 1, but who knows for later ROMs?
		//bytes=1896;
		handle=NewHandleSys(bytes);
		if(handle==NULL)return 0;	// Insufficient room on System heap.
		HLockHi(handle);
		BlockMove((Ptr)driver,*handle,bytes);
		(**auxDCEHandle).dCtlDriver=(Ptr)handle;
		(**auxDCEHandle).dCtlFlags |= dRAMBased;		
	}
	
	// Patch RAM-based driver.
	handle=(Handle)(**auxDCEHandle).dCtlDriver;
	w=*(short **)handle;
	if(w[0x51e/2]!=0x818 || w[0x590/2]!=0x1810 || w[0x2c/2]!=badVersion){
		printf("PatchMacIIciVideoDriver error.\n");
		return 0;
	}
	w[0x51e/2]=0x4858;
	w[0x590/2]=0x1a12;
	w[0x2c/2]+=100;									// Change version number.
	if(w[0x51e/2]!=0x4858 || w[0x590/2]!=0x1a12){
		printf("PatchMacIIciVideoDriver error.\n");
		return 0;
	}
	HNoPurge(handle);
	return 1;
}
