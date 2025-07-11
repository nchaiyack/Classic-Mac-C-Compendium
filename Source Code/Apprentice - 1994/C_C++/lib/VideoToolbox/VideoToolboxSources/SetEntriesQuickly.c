/*
SetEntriesQuickly.c

SetEntriesQuickly loads your video card�s clut as quickly as possible.

This has been written by many people, and the �final� result has not been tested
on all the computers and video devices it�s meant to support. The best test is
simply to run TimeVideo, which gives it a thorough workout on all of your
computer�s video devices. Please send the report �TimeVideo results� to
denis_pelli@isr.syr.edu, and I�ll add your test results to �Video synch�
(section E). Naturally, we�ll try to fix bugs as they are discovered.

Some Macintosh video drivers are poorly written; they take too long (more than a
frame time) to load the clut. This is makes it impossible to do clut animation
for temporal modulation etc., for which one needs to be able to reload the clut
on each frame. At one time many of us thought that the limitation was in
hardware, in the RAMDAC, but we were wrong. Raynald Comptois disassembled
several video drivers and wrote his own programs to load the clut quickly, and
his programs manage to do it within a frame. Raynald was kind enough to share
his code with me. I passed it on to Peter Lennie and Bill Haake, who polished
it, making it compatible with the 68040 processor, and added support for more
cards. I polished their work, made the routines self contained, adding a
�device� argument to allow use in Macs that have more than one video device, and
quickly figuring out all the key parameters (mode, pixelSize, DAC size, and clut
size). There are now two alternative front ends: SetEntriesQuickly() for new
users, and macltset() for backward compatibility with programs that used
Raynald�s original routines. This modularity has increased runtime only
slightly, a fraction of a millisecond.

New drivers are hard to write, since they must directly address the registers of
the video card, which are unique to each video card and undocumented. So the
author of a driver must disassemble the original video driver and figure things
out on his or her own. A lot of work.

SetEntriesQuickly is unlike the standard video drivers in the following ways:
1. SetEntriesQuickly always takes less than 2 ms to load the whole clut. 
Some video drivers, e.g. for Apple�s 8�24 card, take several frames (>30 ms) to 
finish loading the clut.
2A. SetEntriesQuickly does not wait for VBL, so a visible glitch may appear on
the current frame, at least on some older video cards (e.g. Hi-Res, Toby, and
Mac IIci). (You can prevent this glitch by calling SetEntriesQuickly only at
blanking time. Use a VBL task or WaitForBlanking().) Newer cards and computers
seem to have dual ported video clut memory so you can write at any point in the
frame cycle without noticeable glitch, though of course you may want to synch
the update for other reasons.
2B. A flag argument �waitForNextBlanking� is provided, but at present this
option is only supported for the Toby video card.
3. SetEntriesQuickly ignores the gamma table, yielding the same result as using
the standard video driver with an uncorrected gamma table. E.g. after calling
GDUncorrectedGamma(device).
4. SetEntriesQuickly() does nothing and returns an error if the arguments
specify loading of an out-of-range index. 
5. SetEntriesQuickly() does nothing and returns an error if the �count�
specifies zero entries. This is contrary to the (bizarre) Apple convention that
a setEntries control call with a count corresponding to zero entries will result
in loading of entries specified by the �value� field of the ColorSpec.
6. SetEntriesQuickly does not have immediate access to the video driver�s
private tables. Therefore the first time you call SetEntriesQuickly() for a 
particular device there is an extra delay of about 1 ms while some key
information is ferreted out. That information is cached, so subsequent calls
for the same device will be fast, spending most of their time loading the clut.

Two front ends are provided, for compatibility with two distinct traditions:

OSErr SetEntriesQuickly(GDHandle device,short start,short count,ColorSpec *table);

SetEntriesQuickly() uses the same calling convention as the VideoToolbox routine
GDSetEntries() and, except for adding the GDHandle argument to specify the
device, is also the same as Apple�s SetEntries() Color Manager routine,
documented in Inside Macintosh V-143. Apple specifies special behavior when
count==-1, but we don�t support that here and simply return with an error. I
suggest that new users use SetEntriesQuickly rather that macltset. �start� is
the index of the first clut entry to load, and should be greater than or equal
to zero. �count� is the number of entries to load, minus 1. (Yes, �minus 1�,
that�s Apple�s convention.) �table� is a Colorspec array. (Each ColorSpec
element is a structure consisting of a two-byte �value�, which is not used, and
a 6-byte �rgb�, which, in turn is a structure of three 16-bit unsigned short
ints: red, green, and blue. Apple�s convention is that the MOST SIGNIFICANT BITS
of the 16-bit color values are used. It�s good practice in your programs to
provide full 16-bit values, so that when you upgrade to fancier video cards with
more-than-eight bit DACs your programs will benefit from the extra precision
without needing any change. Returns zero if successful, nonzero if unsuccessful,
i.e. illegal arguments.

short macltset(GDHandle device,short start
	,unsigned short* red,unsigned short* green,unsigned short* blue,short count1);

macltset() uses a calling convention established by Raynald Comtois, and
provides backward compatibility with older programs. �red�, �green�, and �blue�
are arrays of 16-bit unsigned short ints, of which the LEAST SIGNIFICANT BITS
are used. �start� is the index of the first entry to change. �count1� is the
number of entries to change (contrary to Apple�s convention).

Both front ends use the same general-purpose subroutine: LoadClut(), which in
turn calls the hardware-specific routine appropriate to the particular video 
device being used.

The useMostSignificantBits bit of the �flags� argument specifies whether to use 
Apple�s convention (for users of SetEntriesQuickly) or Raynald�s convention 
(for users of macltset).

At the moment all the supported video cards have 8-bit DACs, except the RasterOps
ProColor 32, which has 9-bit DACs. If the useMostSignificantBits flag is true
then you don�t need to worry, as the least significant bit of the 9-bit DAC
simply picks up the next lower bit from your numbers, giving you a tad more
precision. However, if useMostSignificantBits flag is false then, in order to
use the full range of the DAC you must make all your numbers twice as big, or --
cludge time! -- set the useOnly8Bits flag, to request that your 8-bit numbers be
multiplied by two, allowing you to use the whole range of the DAC without
changing the rest of your program, but wasting the DAC�s least significant bit
by setting it permanently to zero.

SUSPENDING INTERRUPTS. If you wish, the low-level routines will suspend
interrupts while loading the clut. Presumably Raynald had his reasons for
implementing this, so this �feature� is enabled when you use macltset(). Peter
Lennie writes, �The switch to uninterruptable processing during the write is, I
think, out of the original drivers (though I�m not absolutely sure).  I imagine
it�s to avoid display glitches that would result from some higher priority
interrupt suspending a clut rewrite somewhere in the middle.� However, I (dgp)
don�t see any advantage to suspending interrupts, and believe that there is a
significant downside if you are trying to keep track of the VBL interrupts on
several video cards, since suspending interrupts for 1 or 2 ms might be long
enough to miss a whole VBL interval. Thus SetEntriesQuickly disables this
�feature�. However, this is not a philosophical debate. We all agree that
interrupts should be suspended if doing otherwise would occasionally result in a
visible glitch. Does anyone know?

OSErr WaitForNextBlanking(GDHandle device);

Waits for beginning of next blanking interval. Currently this supports only the
Toby and HiRes cards (Apple�s original video cards, now obsolete).

SPEED. SetEntriesQuickly() is self contained. You simply give it the GDHandle of
your video device (as returned, e.g. by GetScreenDevice), and tell it what you
want to do to the clut. In order to do this for you it needs to figure out a
bunch of stuff about your video device. This research takes time; the first time
you call it for a particular device it takes on the order of 1 ms to look up
stuff. However, it saves this info in a cache, for each device, for quick
retrieval on subsequent occasions. The implication is that programs that use
SetEntriesQuickly ought to call it once just for practice (to get the cache
loading over with) before using it in a situation where speed matters.

The coding of the LoadClut �driver� routines is a compromise between the needs
of SetEntriesQuickly and macltset, which both use them. I decided not to write
separate clut loading loops for the two cases (use most- vs. least-significant
bits). I believe (but have not tested) that adding a register offset instead of
using an autoincrement instruction incurs essentially no time penalty because
the processor automatically overlaps the execution of such instructions. So I
think that SetEntriesQuickly is running flat out, and don�t see any prospect of
speeding it up significantly. On the other hand, I suspect that fetching the
least significant byte by doing a byte access to an odd address (for macltset)
does slow things down perhaps 30% (though I haven�t timed it) over doing a word
access to an even address, as Raynald had originally coded it. If that speed
loss is unacceptable, then one could insert an if(flags&useMostSignificantBits)
statement into the relevant subroutine and write two separate loops optimized
for the two cases. My guess is that the current compromise will be acceptable to
all users.

IMPROVEMENTS:
It is hoped that others will add to the functionality of this routine. Please
share your enhancements with others by sending them to denis_pelli@isr.syr.edu
for inclusion in the VideoToolbox.

Those wishing to support new video devices should begin by buying and reading
Apple�s Designing Cards and Drivers, 3rd Ed., Addison-Wesley, and then use the
VideoToolbox utility GetVideoDrivers to copy all your drivers into resource
files, and use ResEdit with CODE editor to peruse them. The ResEdit CODE editor
is a public domain file distributed by:
Ira L. Ruben
Apple Computer, Inc.
20525 Mariani Ave., MS: 37-A
Cupertino, Ca. 95014
Ira@Apple.Com
ftp.apple.com:/dts/mac/tools/resedit/resedit-2-1-code-editor...

By the way, assembly code is hard to write, read, and maintain,
and the speed advantage is negligible, about 10%. I suggest that
all new code be written in C.

It is logical that we identify the video card by the card name,
GDCardName(driver), but in fact getting the card name is very slow (1.5 ms)
whereas getting the driver name is fast, GDName(driver), and would be
sufficiently unique for our purposes. (E.g. the Toby and TFB video cards have
the same driver, and our code works for both cards.) 

KNOWN BUGS:
Has not been tested on all the video devices that are supposed to be supported.
Please run the demo TimeVideo, and send the results file to denis_pelli@isr.syr.edu

Does not work with the RasterOps ProColor 32. Hopefully this will be fixed soon.

The Quadra code requires that start==0. This could probably be figured out and
fixed pretty easily if someone took the time to do so.

I recommend using the standard drivers (i.e. GDSetEntries/GDDirectSetEntries)
instead of SetEntriesQuickly for the Toby and High Resolution video cards and
the Mac IIci built-in video. Those standard drivers work fine, whereas for those
devices SetEntriesQuickly produces visible dynamic black specks as it accesses
the clut.

None of these routines wait for the vertical blanking interval before loading
the clut. On older video devices--Toby, HiRes, Mac IIci--this results in visible
dynamic black specks on the screen. I (dgp) consider this a bug, but, for most
of these devices I don�t know how to wait for the end of frame, short of setting
up an interrupt. (Just about every video card has a bit that one could monitor,
but its address is usually undocumented.) Newer devices seem to be ok, because of
dual-ported RAMDAC memory. Check this out on your devices by running TimeVideo.

HISTORY:
8/24/92 Original setcardbase and macltset provided by Raynald Comtois
(raco@wjh12.harvard.edu) to Denis Pelli.

10/2/92 Bill Haake added code for the RasterOps ProColor 32, which has 9-bit
DACs and 9-bit entries in the lookup table.

10/1/92 Peter Lennie added code for Quadra internal video.  No provision for
changing the start position in the table, (I couldn't find any relevant
disassembly) so 'start' is ignored, and you should write the whole table.
	
9/30/92 Bill Haake & Peter Lennie modified the code for the 8x24 card
and the 8x24GC to make it a) work properly in 32-bit mode. b) to fix a bug
(feature?) of the original drivers that prevented the cards running on a Quadra.
The drivers exploit 'byte-smearing' on the 68020 and 68030 (Tech Note 282). This
means that one can move a byte to the lowest byte address of the data register
on the card, when one actually wants to put it at address+3 (!!). The functions
work for all the cards (except toby, which hasn't been tested) and on internal
video in both 24 and 32 bit mode on Quadra 700/950, IIfx or ci running system
7.0.1.
	
9/28/92 Peter Lennie added the function findcard.

11/23/92 Denis Pelli (dgp) eliminated all globals because they implicitly
assumed that there is only one video device. All routines now accept a GDHandle
specifying which video device. Simplified the logic of GetCardBase(), minimizing
the dependence on card type.

11/25/92 dgp When USE_MSB is true, all the routines now use the most significant
bits of the 16-bit elements of the user-supplied color tables. When it is false
the least significant bits are used. This is mostly implemented by offseting the
table pointers by one byte and only reading the desired byte. �Generalized
macltset() to work with tables that have an arbitrary element spacing. This
allows it to work with both with Raynald's convention of three arrays of shorts,
and the Apple convention of a ColorSpec array, each element of which consists of
red, green, blue, and value (which is not used). �Added alias "Toby frame buffer
card" for tobycard.

11/27/92 dgp Broke out the code for each card into separate subroutines. This
allows optimal register assignment for each routine, and makes it much easier to read
the THINK C disassembler output. The runtime overhead of loading and unloading
the stack is negligible, and could be eliminated entirely by putting all the
parameters in a structure and passing a pointer to it. �Added a flag,
suspendInterrupts, to make interrupt suspension optional since it may be
undesirable in some applications. (Blocking interrupts for 1 ms could cause you
to miss the interrupt from a video card, especially if you are trying to keep
track of interrupts on several video cards at once.)

11/30/92 dgp Wrote TestCluts, which reads back the clut and checks all
values, and used it to test SetEntriesQuickly() on Quadra 950 internal video,
Mac IIci internal video, hirescard, "Toby frame buffer card", and 8�24 card at
all depths, for both 24- and 32-bit addressing. Toby card was tested on 68020,
68030, and 68040 processors. �Wrote documentation. �Replaced compile-time constants
USE_MSB and PRO_8BITS by runtime flags passed as arguments. �Added WaitForNextBlanking()
based on code from VideoTFB.c.

12/3/92 dgp Incorporated Peter Lennie's corrections and additions to the comments above.

12/8/92 dgp Added missing "case" to switch in WaitForNextBlanking.

12/13/92 dgp Changed erroneous "&d" to "%d" in a printf. Added some comments to
the documentation above.
			
12/15/92 dgp Now get mode from device record and leave it in standard form, 
i.e. with the 0x80 bit set, and only strip off that bit when actually necessary,
e.g. in LoadClutMacIIci. 

12/30/92 dgp Make sure routines return zero when there's no error.

2/15/93	dgp Rewrote nonworking LoadClutToby in C, and made it work. Rewrote
nonworking LoadClutx824 in C, and made it work. Fixed sixteenBitMode in
LoadClutQuadra. Use new SwapPriority instead of Get/SetPriority.

2/20/93	dgp	Translated LoadClutGCx824 to C. (It was ignoring the start value.)

3/4/93	dgp	Added macIIsi to list of supported cards, since it uses the same
driver as the Mac IIci. Changed definitions of string types slightly to allow
compilation of this file as a code resource. However, the assembly code
uses more registers than are available to a code resource.

4/13/93	dgp	Removed 68020 requirement by translating an indexed add in LoadMacIIci to
C.

4/17/93 dgp Added support in GetCardBase for old Mac II computers whose ROM's only 
support 24-bit NuBus addressing. 

5/18/93 SetEntriesQuickly now respects the setting of the device's gray/color mode, 
and maps to gray if in the device is in gray mode and pixelSize<=8. Changed prototype
of macltset to specify the red, green, and blue arrays as "unsigned short" instead of 
"short".

7/7/93	dgp	Disabled some global optimizations because THINK C 6 will
crash while compiling if the Radius PowerView is present: "!gopt_induction,!gopt_loop".

7/9/93	dgp check for 32-bit addressing capability.
*/
#include "VideoToolbox.h"
#include <assert.h>
#define USE_ONLY_8_BITS_IN_MACLTSET 0	// 1 to use RasterOps ProColor32 as an 8-bit DAC.
#pragma options(assign_registers,honor_register,redundant_loads,defer_adjust)
#pragma options(global_optimizer,!gopt_induction,!gopt_loop,gopt_cse,gopt_coloring)

// These are the five user-callable routines:
OSErr WaitForNextBlanking(GDHandle device);
OSErr SetEntriesQuickly(GDHandle device,short start,short count,ColorSpec *table);
short macltset(GDHandle device,short start
	,unsigned short* red,unsigned short* green,unsigned short* blue,short count1);
short GetCardType(GDHandle device);
char *GetCardBase(GDHandle device);

/*
I suggest keeping the following information private to this file. In principle
you could publish these card types and use them in your programs. However, in
practice, I cannot see any point in doing so. If you need to identify the card
name I suggest you simply use the string returned by GDCardName(device) in
GDVideo.c of the VideoToolbox. (Don't forget to call DisposPtr() when you're
through with the string.) Or use GDName(device), which returns the name of the
card's driver, and is much quicker. If you simply want to know whether your
video card is supported by SetEntriesQuickly.c then you can simply make sure
that GetCardType() returns a nonzero cardType.
*/
struct vtype {		/* associates card name and id */
	char name[40];
	short id;
};
enum {				/* card identifiers */
	tobycard = 1,
	hirescard,
	macIIci,
	macIIsi,
	x824card,
	x824GCcard,
	quadra700,
	quadra900,
	quadra950,
	procolor32
};
static struct vtype card[] = {	// card name & id		// Original author:
	{"Toby frame buffer card",				tobycard},	// Raynald Comtois
	{"Display_Video_Apple_TFB",				tobycard},	//     "      "
	{"Mac II High-Resolution Video Card",	hirescard},	// Raynald Comtois
	{"Macintosh II Built-In Video",			macIIci},	// Raynald Comtois
	{"Macintosh A Built-In Video",			macIIsi},	//     "      "
	{"Macintosh Display Card",				x824card},	// Raynald Comtois
	{"Macintosh Display Card 8�24 GC",		x824GCcard},// Raynald Comtois
	{"Macintosh E Built-In Video",			quadra700},	// Peter Lennie
	{"Macintosh C Built-In Video",			quadra900},	//     "      "
	{"Macintosh G Built-In Video",			quadra950},	//     "      "
	{"ProColor 32",							procolor32}	// Bill Haake
};
static char driverName[][40]=		// Not used at present.
{
	"\p.Display_Video_Apple_TFB"	// Apple �Toby frame buffer card�
	,"\p.Display_Video_Apple_HRVC"	// Apple �Mac II High-Resolution Video Card�
	,"\p.Display_Video_Apple_MDC"	// Apple 8�24 �Macintosh Display Card�
	,"\p.Display_Video_Apple_MDCGC"	// Apple 8�24GC
	,"\p.Display_Video_Apple_RBV1"	// Mac IIci and IIsi built-in video
	,"\p.Display_Video_Apple_DAFB"	// Quadra 700, 900, 950 built-in video
	,"\p.RasterOps 1.0 32XL Video Driver"	// Radius ProColor 32
};
enum {							// Flags passed to LoadClut().
	suspendInterrupts=1,
	useMostSignificantBits=2,
	useOnly8Bits=4,
	waitForNextBlanking=8
};
enum{quadraNonzeroStart=111};	// value returned as error.
	
short LoadClut(GDHandle device,short start,short count
	,unsigned short* red,unsigned short* green,unsigned short* blue,long elementSpacing,short flags);
OSErr LoadClutProColor(short start,short count,char *r,char *g,char *b
	,long elementSpacing,short mode,short pixelSize,short clutSize
	,char *cardBase,short flags);
OSErr LoadClutQuadra(short start,short count,char *r,char *g,char *b
	,long elementSpacing,short mode,short pixelSize,short clutSize
	,char *cardBase,short flags);
OSErr LoadClutMacIIci(short start,short count,char *r,char *g,char *b
	,long elementSpacing,short mode,short pixelSize,short clutSize
	,char *cardBase,short flags);
OSErr LoadClutHiRes(short start,short count,char *r,char *g,char *b
	,long elementSpacing,short mode,short pixelSize,short clutSize
	,char *cardBase,short flags);
OSErr LoadClutx824(short start,short count,char *r,char *g,char *b
	,long elementSpacing,short mode,short pixelSize,short clutSize
	,char *cardBase,short flags);
OSErr LoadClutx824GC(short start,short count,char *r,char *g,char *b
	,long elementSpacing,short mode,short pixelSize,short clutSize
	,char *cardBase,short flags);
OSErr LoadClutToby(short start,short count,char *r,char *g,char *b
	,long elementSpacing,short mode,short pixelSize,short clutSize
	,char *cardBase,short flags);
/******************************************************************************/
/*
The arguments start, count, and table are the same as for the Color Manager call
SetEntries(), documented in Inside Macintosh V-143. (Except that a count==-1 is
considered illegal here.) Apple's ideosyncratic convention is that "count" is
"zero-based", meaning that it is one less than the number of clut entries that
you want to modify. "count" must be at least zero. Returns zero if successful,
nonzero if unsuccessful, i.e. illegal arguments.
*/
OSErr SetEntriesQuickly(GDHandle device,short start,short count,ColorSpec *table)
{
	short flags=useMostSignificantBits;
	//flags+=suspendInterrupts;		// Optional, no
	//flags+=waitForNextBlanking;	// Optional, no

	return LoadClut(device,start,count
		,&table[0].rgb.red,&table[0].rgb.green
		,&table[0].rgb.blue,sizeof(table[0]),flags);
}
/******************************************************************************/
short macltset(GDHandle device,register short start
	,unsigned short* red,unsigned short* green,unsigned short* blue,short count1)
{
	short flags=0;
	flags+=suspendInterrupts;		// Optional
	#if USE_ONLY_8_BITS_IN_MACLTSET
		flags+=useOnly8Bits;		// Optional
	#endif
	//flags+=waitForNextBlanking;	// Optional
	
	return LoadClut(device,start,count1-1,red,green,blue,sizeof(red[0]),flags);
}
/******************************************************************************/
/*
The first call to GetCardType for a particular device takes 1.5-3 ms, depending
on your computer's speed, because it takes Apple's Slot Manager a long time to
get the card name. However, GetCardType's answers are cached so subsequent calls
for a previously queried device will be fast <100 �s.
*/
short GetCardType(GDHandle device)	// returns card type, if known, or zero if not.
{
	register short i;
	short cardType;
	char *name;
	static GDHandle deviceCache[MAX_SCREENS];
	static short typeCache[MAX_SCREENS];
	
	// Do we already know the answer? Check the cache.
	for(i=0;i<MAX_SCREENS;i++)if(device==deviceCache[i])return typeCache[i];
	// Get card name, see if it's in our list of known cards
	name=GDCardName(device);
	cardType=0;
	for (i=0; i<sizeof(card)/sizeof(card[0]); i++){
		if(strcmp(name,card[i].name)==0){
			cardType=card[i].id;
			break;
		}
	}
	DisposePtr(name);
	// Save answer in cache.
	for(i=0;i<MAX_SCREENS;i++)if(deviceCache[i]==0){
		typeCache[i]=cardType;
		deviceCache[i]=device;
		break;
	}
	return cardType;
}
//#error "Yay"
/******************************************************************************/
long internalVideoBase:0xDD8;	// Undocumented System global

char *GetCardBase(GDHandle device)
{
	long cardBase,slot;	/* slot must be declared long */
	short cardType;
	
	slot=GetDeviceSlot(device);
	if(slot==0){
		// Built-in video, not in a NuBus slot.
		// E.g.: macIIci,macIIsi,quadra700,quadra900,quadra950
		#if 1
			// This C is equivalent to Raynald's assembly code below.
			cardBase = *(long *)(internalVideoBase + *(long *)internalVideoBase + 56);
		#else
			asm {
				move.l 0xDD8,a0		/* get card base address */
				adda.l (a0),a0
				move.l 56(a0),a1
				move.l a1,cardBase
			}
		#endif
	}else{
		// Video card in NuBus slot
		cardType=GetCardType(device);
		switch(cardType){
			case x824GCcard:
				cardBase = slot<<28;	// a superslot
				break;
			case tobycard:
			case hirescard:
				cardBase = (slot<<24) | 0xF0000000;
				cardBase+= (slot<<20);	// Support old Mac II 24-bit NuBus addressing
				break;
			case procolor32: 			// RasterOps
			case x824card:
			default:
				cardBase = (slot<<24) | 0xF0000000;
				break;
		}
	}
	return (char *)cardBase;
}
/******************************************************************************/
short LoadClut(GDHandle device,short start,short count
	,unsigned short* red,unsigned short* green,unsigned short* blue
	,long elementSpacing,short flags)
{
	char *cardBase;
	short cardType=0,pixelSize,mode;
	short clutSize; 	// entries in the lookup table
	OSErr error;
	int i,j;
	short isGray;
	unsigned short grayTable[256];
	
	if(device==NULL)return 1;
	cardType=GetCardType(device);	// takes 1.7 ms the first time for each device.
	if(cardType==0)return 1;
	cardBase=GetCardBase(device);
	clutSize=GDClutSize(device);
	pixelSize=(**(**device).gdPMap).pixelSize;
	mode=(**device).gdMode;
	
	// Check range.
	if(start>clutSize-1 || start<0 || count+start>clutSize-1 || count<0)return 1;
	
	// We're going to use these RAM addresses in 32-bit mode.
	red = (unsigned short *)StripAddress(red);
	green = (unsigned short *)StripAddress(green);
	blue = (unsigned short *)StripAddress(blue);

	if(waitForNextBlanking & flags){
		WaitForNextBlanking(device);
	}
	
	isGray=!TestDeviceAttribute(device,gdDevType);
	if(isGray && pixelSize<=8){
		j=elementSpacing/sizeof(*red);
		for(i=0;i<=count;i++){
			grayTable[i]=*red*0.30+*green*0.59+*blue*0.11;
			red+=j;
			green+=j;
			blue+=j;
		}
		elementSpacing=sizeof(grayTable[0]);
		red=green=blue=grayTable;
	}
	
	// After the above setting up, actually loading 256x3 clut entries takes <2 ms.
	switch (cardType) {
		// I packaged the code for each case into a separate subroutine
		// in order to allow the THINK C compiler to optimize each
		// one independently. An important consideration is that the THINK C 5.04
		// compiler disables most optimizations for any function that includes
		// the "asm" directive anywhere within the function. Thus mixing C and assembly
		// will result in inefficient C. No less important, the THINK C Disassemble
		// command is very handy in writing fast C code, but produces an uncommented 
		// listing, which is much easier to read if the separate routines are each 
		// named subroutines.
		case procolor32:
			error=LoadClutProColor(start,count,(char *)red,(char *)green,(char *)blue,
				elementSpacing,mode,pixelSize,clutSize,cardBase,flags);
			break;
		case quadra700:
		case quadra900:
		case quadra950:
			error=LoadClutQuadra(start,count,(char *)red,(char *)green,(char *)blue,
				elementSpacing,mode,pixelSize,clutSize,cardBase,flags);
			break;
		case macIIci:
		case macIIsi:
			error=LoadClutMacIIci(start,count,(char *)red,(char *)green,(char *)blue,
				elementSpacing,mode,pixelSize,clutSize,cardBase,flags);
			break;
		case hirescard:
			error=LoadClutHiRes(start,count,(char *)red,(char *)green,(char *)blue,
				elementSpacing,mode,pixelSize,clutSize,cardBase,flags);
			break;
		case x824card:
			error=LoadClutx824(start,count,(char *)red,(char *)green,(char *)blue,
				elementSpacing,mode,pixelSize,clutSize,cardBase,flags);
			break;
		case x824GCcard:
			error=LoadClutx824GC(start,count,(char *)red,(char *)green,(char *)blue,
				elementSpacing,mode,pixelSize,clutSize,cardBase,flags);
			break;
		case tobycard:
			error=LoadClutToby(start,count,(char *)red,(char *)green,(char *)blue,
				elementSpacing,mode,pixelSize,clutSize,cardBase,flags);
			break;
	}
	return error;
}
/******************************************************************************/
OSErr LoadClutProColor(short start,register short count
	,register char *red,register char *green,register char *blue
	,register long elementSpacing
	,short mode,short pixelSize,short clutSize,char *cardBase,short flags)
{
	char mmuMode=true32b,priority=7;
	register long bitShift;
	Boolean can32;
	long value=0,error;

	error=Gestalt(gestaltAddressingModeAttr,&value);
	can32=!error && (value&(1<<gestalt32BitCapable));
	if(useMostSignificantBits & flags){
		bitShift=9;
	}else{
		if(useOnly8Bits & flags) bitShift=1;
		else bitShift=0;
	}
	if(suspendInterrupts & flags)SwapPriority(&priority);
	if(can32)SwapMMUMode(&mmuMode);
	asm {
		move.l cardBase,a1		/* get card base address */
		adda.l	#0xf60000,a1	/* offset to control registers */		
	@9	move.w	start,2(a1)		/* Set the index on the card */
		move.w (red),d1
		add.l elementSpacing,red
		lsl.w bitShift,d1
		move.w d1,14(a1)
		move.w (green),d1
		add.l elementSpacing,green
		lsl.w bitShift,d1
		move.w d1,14(a1)
		move.w (blue),d1
		add.l elementSpacing,blue
		lsl.w bitShift,d1
		move.w d1,14(a1)
		addq.w	#1,start		/* Point to next entry in table */
		dbf count,@9
	}
	if(can32)SwapMMUMode(&mmuMode);
	if(suspendInterrupts & flags)SwapPriority(&priority);
	return 0;
}
OSErr LoadClutQuadra(short start,register short count
	,register char *red,register char *green,register char *blue
	,register long elementSpacing
	,short mode,short pixelSize,short clutSize,char *cardBase,short flags)
{
	char mmuMode=true32b,priority=7;
	Boolean can32;
	long value=0,error;

	error=Gestalt(gestaltAddressingModeAttr,&value);
	can32=!error && (value&(1<<gestalt32BitCapable));
	if(start!=0){
		//printf("LoadClutQuadra: start must be zero\n");
		return quadraNonzeroStart;
	}
	if(!(useMostSignificantBits & flags)){
		// Point to less significant byte of word.
		red++;
		green++;
		blue++;
	}
	if(suspendInterrupts & flags)SwapPriority(&priority);
	if(can32)SwapMMUMode(&mmuMode);
	if(mode!=sixteenBitMode)asm{
		move.l cardBase,a1
		lea 0x210(a1), a1
		clr.l -16(a1)
	@4	move.b (red),d1
		add.l elementSpacing,red
		move.l d1,(a1)
		move.b (green),d1
		add.l elementSpacing,green
		move.l d1,(a1)
		move.b (blue),d1
		add.l elementSpacing,blue
		move.l d1,(a1)
		dbf count,@4
	}else asm{
	// In sixteenBitMode the clut addressing is weird.
	// I arrived at the following solution by trial and error.
	// It's a kludge, but is still fast enough. dgp.
		move.l cardBase,a1
		lea 0x210(a1), a1
		clr.l -16(a1)
	@44	move.b (red),d1
		move.l d1,(a1)
		move.b (green),d1
		move.l d1,(a1)
		move.b (blue),d1
		move.l d1,(a1)

		move.b (red),d1
		move.l d1,(a1)
		move.b (green),d1
		move.l d1,(a1)
		move.b (blue),d1
		move.l d1,(a1)

		move.b (red),d1
		move.l d1,(a1)
		move.b (green),d1
		move.l d1,(a1)
		move.b (blue),d1
		move.l d1,(a1)

		move.b (red),d1
		move.l d1,(a1)
		move.b (green),d1
		move.l d1,(a1)
		move.b (blue),d1
		move.l d1,(a1)

		move.b (red),d1
		move.l d1,(a1)
		move.b (green),d1
		move.l d1,(a1)
		move.b (blue),d1
		move.l d1,(a1)

		move.b (red),d1
		move.l d1,(a1)
		move.b (green),d1
		move.l d1,(a1)
		move.b (blue),d1
		move.l d1,(a1)

		move.b (red),d1
		move.l d1,(a1)
		move.b (green),d1
		move.l d1,(a1)
		move.b (blue),d1
		move.l d1,(a1)

		move.b (red),d1
		add.l elementSpacing,red
		move.l d1,(a1)
		move.b (green),d1
		add.l elementSpacing,green
		move.l d1,(a1)
		move.b (blue),d1
		add.l elementSpacing,blue
		move.l d1,(a1)

		dbf count,@44
	}
	if(can32)SwapMMUMode(&mmuMode);
	if(suspendInterrupts & flags)SwapPriority(&priority);
	return 0;
}
OSErr LoadClutMacIIci(register short start,register short count
	,register char *red,register char *green,register char *blue
	,register long elementSpacing
	,short mode,short pixelSize,short clutSize,char *cardBase,short flags)
{
	static char realstartindex[] = {0xFE, 0xFC, 0xF0, 0x00,0,0,0};
	char priority=7;
	Boolean can32;
	long value=0,error;

	error=Gestalt(gestaltAddressingModeAttr,&value);
	can32=!error && (value&(1<<gestalt32BitCapable));	
	if(!(useMostSignificantBits & flags)){
		// Point to less significant byte of word.
		red++;
		green++;
		blue++;
	}
	if(suspendInterrupts & flags)SwapPriority(&priority);
	mode&=7;
	start+=realstartindex[mode];
	asm {
		move.w mode,d1
		move.l cardBase,a0		// get card base address
		move.l a0,a1
//		move.b #255,8(a0)		// not necessary
		move.b start,(a0)
		addq.l #4,a1
	@3	move.b (red),d1
		add.l elementSpacing,red
		move.b d1,(a1)
		move.b (green),d1
		add.l elementSpacing,green
		move.b d1,(a1)
		move.b (blue),d1
		add.l elementSpacing,blue
		move.b d1,(a1)
		dbf count,@3
	}
	if(suspendInterrupts & flags)SwapPriority(&priority);
	return 0;
}
// High resolution video card
//#define HRVCBase			0x80000
#define HRVCClutAddrReg		0x940E0
#define HRVCClutWDataReg	0x940E4
//#define HRVCClutRDataReg	0x94054
OSErr LoadClutHiRes(short start,register short count
	,register char *red,register char *green,register char *blue
	,register long elementSpacing
	,short mode,short pixelSize,short clutSize,char *cardBase,short flags)
{
	char *bytePtr;
	char mmuMode=true32b,priority=7;
	Boolean can32;
	long value=0,error;

	error=Gestalt(gestaltAddressingModeAttr,&value);
	can32=!error && (value&(1<<gestalt32BitCapable));	
	if(!(useMostSignificantBits & flags)){
		// Point to less significant byte of word.
		red++;
		green++;
		blue++;
	}
	if(suspendInterrupts & flags)SwapPriority(&priority);
	if(can32)SwapMMUMode(&mmuMode);
	red+=count*elementSpacing;
	green+=count*elementSpacing;
	blue+=count*elementSpacing;
	// We'll start with clut entry start+count, and work
	// down from there to clut entry start. The clut address
	// register counts down automatically.
	*(cardBase+HRVCClutAddrReg)=~(clutSize-1-start-count);
	bytePtr=cardBase+HRVCClutWDataReg;
	// This is the key loop. 
	// This C code is only about 10% slower than the original assembly code.
	elementSpacing= -elementSpacing;
	do{
		*bytePtr=~ *red;
		red+=elementSpacing;
		*bytePtr=~ *green;
		green+=elementSpacing;
		*bytePtr=~ *blue;
		blue+=elementSpacing;
	}while(--count>=0);
	if(can32)SwapMMUMode(&mmuMode);
	if(suspendInterrupts & flags)SwapPriority(&priority);
	return 0;
}
// Macintosh display card (8�24)
//#define MDCVideoBase		0xA00
#define MDCClutAddrReg		0x200200
#define MDCClutDataReg		0x200204
OSErr LoadClutx824(short start,register short count
	,register char *red,register char *green,register char *blue
	,register long elementSpacing
	,short mode,short pixelSize,short clutSize,char *cardBase,short flags)
{
	char mmuMode=true32b,priority=7;
	register char *clut;
	char *clutIndex;
	Boolean can32;
	long value=0,error;

	error=Gestalt(gestaltAddressingModeAttr,&value);
	can32=!error && (value&(1<<gestalt32BitCapable));
	if(!(useMostSignificantBits & flags)){
		// Point to less significant byte of word.
		red++;
		green++;
		blue++;
	}
	if(suspendInterrupts & flags)SwapPriority(&priority);
	if(can32)SwapMMUMode(&mmuMode);
	clut=cardBase+MDCClutDataReg+3;
	clutIndex=cardBase+MDCClutAddrReg;
	*clutIndex=start;
	for(;count>=0;count--){
		*clut=*red;
		red+=elementSpacing;
		*clut=*green;
		green+=elementSpacing;
		*clut=*blue;
		blue+=elementSpacing;
	}
	if(can32)SwapMMUMode(&mmuMode);
	if(suspendInterrupts & flags)SwapPriority(&priority);
	return 0;
}
// Macintosh display card 8�24 GC
#define MDCgcClutAddrReg	0x6C00000
#define MDCgcClutDataReg	0x6C00004
OSErr LoadClutx824GC(short start,register short count
	,register char *red,register char *green,register char *blue
	,register long elementSpacing
	,short mode,short pixelSize,short clutSize,char *cardBase,short flags)
{
	char mmuMode=true32b,priority=7;
	register long *clut;
	char *clutIndex;
	Boolean can32;
	long value=0,error;

	error=Gestalt(gestaltAddressingModeAttr,&value);
	can32=!error && (value&(1<<gestalt32BitCapable));
	if(!(useMostSignificantBits & flags)){
		// Point to less significant byte of word.
		red++;
		green++;
		blue++;
	}
	if(suspendInterrupts & flags)SwapPriority(&priority);
	if(can32)SwapMMUMode(&mmuMode);
	clutIndex=cardBase+MDCgcClutAddrReg;
	*clutIndex=start;
	#if 0
		clut=(long *)(cardBase+MDCgcClutDataReg);
		for(;count>=0;count--){
			*clut=(long)(*red)<<24;
			red+=elementSpacing;
			*clut=(long)(*green)<<24;
			green+=elementSpacing;
			*clut=(long)(*blue)<<24;
			blue+=elementSpacing;
		}
	#else
		asm {
			move.l cardBase,a1
			add.l #MDCgcClutDataReg,a1
		@8	move.b (red),d1
			add.l	elementSpacing,red
			ror.l	#8,d1
			move.l d1,(a1)
			move.b (green),d1
			add.l	elementSpacing,green
			ror.l	#8,d1
			move.l d1,(a1)
			move.b (blue),d1
			add.l elementSpacing,blue
			ror.l	#8,d1
			move.l d1,(a1)
			dbf count,@8
		}
	#endif
	if(can32)SwapMMUMode(&mmuMode);
	if(suspendInterrupts & flags)SwapPriority(&priority);
	return 0;
}
// Toby frame buffer
//#define	TFBBase			0x80000
//#define TFBBufMid			0x80008
//#define TFBBufLow			0x8000C
//#define	TFBIBase		0x8fffc
#define TFBClutWDataReg		0x90018
//#define TFBClutRDataReg	0x90028
#define TFBClutAddrReg		0x9001C
#define	TFBReadVSync		0xD0000
//#define	TFBReadVInt		0xD0004
//#define	TFBReadIntlc	0xD0008
//#define	TFBVIntEnable	0xA0000
//#define	TFBVIntDisable	0xA0004
OSErr LoadClutToby(short start,register short count
	,register char *red,register char *green,register char *blue
	,register long elementSpacing
	,short mode,short pixelSize,short clutSize,char *cardBase,short flags)
{
	register long index;
	char mmuMode=true32b,priority=7;
	register char *clut,*clutIndex;
	short shift;
	Boolean can32;
	long value=0,error;

	error=Gestalt(gestaltAddressingModeAttr,&value);
	can32=!error && (value&(1<<gestalt32BitCapable));	
	if(!(useMostSignificantBits & flags)){
		// Point to less significant byte of word.
		red++;
		green++;
		blue++;
	}
	index=(count+1)*elementSpacing;
	red+=index;
	green+=index;
	blue+=index;
	shift=8-pixelSize;
	index=start+count+1;
	clut=cardBase+TFBClutWDataReg;
	clutIndex=cardBase+TFBClutAddrReg;
	if(suspendInterrupts & flags)SwapPriority(&priority);
	for(;count>=0;count--,index--){
		*clutIndex=(index<<shift)-1;
		red-=elementSpacing;
		*clut=~*red;
		green-=elementSpacing;
		*clut=~*green;
		blue-=elementSpacing;
		*clut=~*blue;
	}
	if(suspendInterrupts & flags)SwapPriority(&priority);
	return 0;
}

OSErr WaitForNextBlanking(GDHandle device)
// WaitForNextBlanking waits for the beginning of the next vertical blanking interval.
// Returns 0 if successful, or 1 if device is not supported.
{
	register long *blankingPtr;

	switch(GetCardType(device)){
	case tobycard:
		blankingPtr = (long *) ((char *)GetCardBase(device) + TFBReadVSync);
		while (*blankingPtr & 1L);	// if we're already blanking, wait till end.
		while (!(*blankingPtr & 1L)); // wait until beginning of blanking interval.
		return 0;
	default:
		return 1;
	}
}
