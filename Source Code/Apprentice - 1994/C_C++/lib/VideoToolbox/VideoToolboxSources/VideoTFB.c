/* VideoTFB.c
VideoTFB.c allows direct control of the original Apple color video cards, which
are no longer sold by Apple but are available for $90 from Shreve Systems
(800-227-3971). The required video card for VideoTFB is called either "Toby
frame buffer card" or "Mac II High-Resolution Video Card". DonÕt use these
routines unless youÕre prepared to deal with the resulting hardware
dependencies. Try the demo ScrollDemo.

Most people should skip this file and go to GDVideo.c instead, since those
routines will work with ALL Mac video cards.

TFB stands for "Toby Farrand board". Toby Farrand is the guy at Apple that wrote
the video driver for this card.

I wrote this based on my disassembly of the Apple video driver and the memory
diagnostics I received from Toby Farrand.

HISTORY:
10/29/88	dgp	wrote it
12/5/89		dgp	tidied it up a bit. Deleted two obsolete routines.
3/20/90		dgp	make compatible with MPW C.
3/23/90		dgp inserted a test in every routine to exit unless it's a TFB card.
8/24/91		dgp	Made compatible with THINK C 5.0.
12/17/92	dgp added cache to speed up TFBInSlot.
*/
#include "VideoToolbox.h"

/* The CardBase address depends on which slot the card is plugged into */
/* If the card is plugged into the first slot, i.e. slot 9, then */
/* the address is 0xF9900000. The slots are numbered 9 to F, with addresses */
/* 0xF9900000 to 0xFFF00000. The slot number appears twice in the address */
/* in order to be compatible with Mac II's supporting either 24 or 32 bit */
/* addressing. The macro CARDBASE(slot) generates the proper address. */

#define CARDBASE(slot) (unsigned char *)(0xF0000000+0x01100000*slot)
#define VideoBase(slot) (unsigned char *)(0xF0000020+0x01100000*slot)
#define	TFBBase		0x80000
#define TFBPan		0x8000C
#define	TFBIBase	0x8fffc
#define ClutDataReg	0x90018
#define ClutAddrReg	0x9001C
#define	ReadVSync	0xD0000
#define	ReadVInt	0xD0004
#define	ReadIntlc	0xD0008
#define	VIntEnable	0xA0000
#define	VIntDisable	0xA0004
#define GENLOCK		8
#define	GENLOCKREG	1

unsigned char TFBRegisterValues[4][16] = {
	{32,71,0,8,30,229,119,70,5,2,2,1,15,65,5,200},			/* 1 bit/pixel */
	{64,71,0,8,60,229,119,70,5,6,6,4,32,4,11,216},			/* 2 bit/pixel */
	{128,71,0,8,120,229,119,70,5,14,14,10,66,-118,22,-24},	/* 4 bit/pixel */
	{0,71,0,8,240,229,119,70,5,30,30,22,134,150,45,249}		/* 8 bit/pixel */
};

unsigned char NtscValues[16] =
	{0,183,0,8,96,229,59,25,5,8,26,14,132,14,46,249};		/* from NtscFkey */


Boolean TFBInSlot(int slot)
{
	GDHandle device;
	static Boolean tested[16]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	static Boolean ok[16];
	
	if(slot>15)return 0;
	if(!tested[slot]){
		tested[slot]=1;
		device=SlotToScreenDevice(slot);
		ok[slot]=EqualString("\p.Display_Video_Apple_TFB",GDName(device),1,1);
	}
	return ok[slot];
}

void SetUpTFB(int slot)
{
	register unsigned char *dstPtrB,*srcPtrB;
	register long int i;

	if(!TFBInSlot(slot)){
		printf("The card in slot %d is not a TFB card!\n",slot);
		return;
	}
	dstPtrB = CARDBASE(slot) + TFBIBase;
	srcPtrB = &TFBRegisterValues[0][0];
	for (i=15; i>=0; i--) {
		*dstPtrB = ~*srcPtrB++;
		dstPtrB -= 4;
	}
}

void RampClutTFB(int slot)
/* loads the clut with an identical linear ramp in R, G, and B. */
{
	register unsigned char *dstPtrB;
	register int i;

	if(!TFBInSlot(slot)){
		printf("The card in slot %d is not a TFB card!\n",slot);
		return;
	}
	*(CARDBASE(slot) + ClutAddrReg) = 255;
	dstPtrB = CARDBASE(slot) + ClutDataReg;
	for (i=255; i >= 0; i--){
		*dstPtrB = *dstPtrB = *dstPtrB = (unsigned char) i;
	}
	return;
}

void GrayClutTFB(int slot)
/* loads the entire clut with uniform gray. */
{
	register unsigned char *dstPtrB;
	register int i;

	if(!TFBInSlot(slot)){
		printf("The card in slot %d is not a TFB card!\n",slot);
		return;
	}
	*(CARDBASE(slot) + ClutAddrReg) = 255;
	dstPtrB = CARDBASE(slot) + ClutDataReg;
	*dstPtrB = *dstPtrB = *dstPtrB = (unsigned char) 255;
	for (i=254; i >= 1; i--){
		*dstPtrB = *dstPtrB = *dstPtrB = (unsigned char) 127;
	}
	*dstPtrB = *dstPtrB = *dstPtrB = (unsigned char) 0;
	return;
}

void LoadClutTFB(int slot,unsigned char rgb[256][3])
/* loads the clut with a user-supplied array. */
{
	register unsigned char *srcPtrB,*dstPtrB;
	register int i;


	if(!TFBInSlot(slot)){
		printf("The card in slot %d is not a TFB card!\n",slot);
		return;
	}
	*(CARDBASE(slot) + ClutAddrReg) = 255;
	dstPtrB = CARDBASE(slot) + ClutDataReg;
	srcPtrB = &rgb[255][2];
	srcPtrB++;
	for (i=255/4; i >= 0; i--){
		*dstPtrB = *(--srcPtrB);
		*dstPtrB = *(--srcPtrB);
		*dstPtrB = *(--srcPtrB);

		*dstPtrB = *(--srcPtrB);
		*dstPtrB = *(--srcPtrB);
		*dstPtrB = *(--srcPtrB);

		*dstPtrB = *(--srcPtrB);
		*dstPtrB = *(--srcPtrB);
		*dstPtrB = *(--srcPtrB);

		*dstPtrB = *(--srcPtrB);
		*dstPtrB = *(--srcPtrB);
		*dstPtrB = *(--srcPtrB);
	}
	return;
}

void NewBlankingTFB(int slot)
/* NewBlankingTFB waits for the beginning of the next vertical blanking interval */
/* The standard Apple video rate is 66.67 Hz */
{
	register long *blankingPtr;

	if(!TFBInSlot(slot)){
		printf("The card in slot %d is not a TFB card!\n",slot);
		return;
	}
	blankingPtr = (long *) (CARDBASE(slot) + ReadVSync);
	while (*blankingPtr & 1L); /* just in case we're in a tight loop, wait for end of blanking */
	while (!(*blankingPtr & 1L)); /* wait until beginning of blanking interval */
	return;
}

void NewFieldTFB(int slot)
/* NewFieldTFB waits for the beginning of the next video field */
/* The standard Apple video rate is 66.67 Hz */
{
	register long *blankingPtr;

	if(!TFBInSlot(slot)){
		printf("The card in slot %d is not a TFB card!\n",slot);
		return;
	}
	blankingPtr = (long *) (CARDBASE(slot) + ReadVSync);
	while (!(*blankingPtr & 1L));	/* wait until beginning of blanking interval */
	while (*blankingPtr & 1L);		/* wait for end of blanking */
	return;
}

int BlankingTFB(int slot)
/* returns true during the blanking interval */
{
	register long *blankingPtr;

	if(!TFBInSlot(slot)){
		printf("The card in slot %d is not a TFB card!\n",slot);
		return 0;
	}
	blankingPtr = (long *) (CARDBASE(slot) + ReadVSync);
	return (int) (*blankingPtr & 1L);
}


void SetDepthTFB(int slot,short bits)
/* sets up the video card for the desired number of bits/pixel: 1, 2, 4, or 8 */
{
	register unsigned char *dstPtrB, *srcPtrB;
	register long int i;
	short int log2bits;		/* = log2(bits) */

	if(!TFBInSlot(slot)){
		printf("The card in slot %d is not a TFB card!\n",slot);
		return;
	}
	if(bits > 8) return;
	log2bits = 0;
	while ( (bits /= 2) > 0 ) log2bits++;
	dstPtrB = (CARDBASE(slot) + TFBBase);
	srcPtrB = &(TFBRegisterValues[log2bits][0]);	
	NewFieldTFB(slot);
	*(dstPtrB + 15*4) = (char) 0xB7;	/* Preload register15. Put the TFB into a reset state */
	/* 0xB7 apparently requests interlace clock off and 0 bits/pixel */
	for(i=15; i>=0; i--) {
		*dstPtrB = ~ *srcPtrB++;
		dstPtrB += 4;
	}
	return;
}

void SynchSetDepthTFB(int masterSlot,int slot,short bits)
/* sets up the video card for the desired number of bits/pixel: 1, 2, 4, or 8 */
{
	register unsigned char *dstPtrB, *srcPtrB;
	register long int i;
	short int log2bits;		/* = log2(bits) */

	if(!TFBInSlot(slot)){
		printf("The card in slot %d is not a TFB card!\n",slot);
		return;
	}
	if(!TFBInSlot(masterSlot)){
		printf("The card in slot %d is not a TFB card!\n",masterSlot);
		return;
	}
	if(bits!=1 && bits!=2 && bits!=4 && bits!=8){
		printf("SynchSetDepthTFB: %d bits/pixel impossible for a TFB card!\n",bits);
		return;
	}
	log2bits = 0;
	while ( (bits /= 2) > 0 ) log2bits++;
	dstPtrB = (CARDBASE(slot) + TFBBase);
	srcPtrB = &(TFBRegisterValues[log2bits][0]);	
	NewFieldTFB(masterSlot);		/* wait for masterSlot end of frame*/
	*(dstPtrB + 15*4) = (char) 0xB7;	/* Preload register15. Put the TFB into a reset state */
	/* 0xB7 apparently requests interlace clock off and 0 bits/pixel */
	for(i=15; i>=0; i--) {
		*dstPtrB = ~ *srcPtrB++;
		dstPtrB += 4;
	}
	return;
}

void SynchToMainDeviceTFB(device)
GDHandle device;
{
	SynchSetDepthTFB(GetDeviceSlot(GetMainDevice()),GetDeviceSlot(device), (**(**device).gdPMap).pixelSize);
}

void HaltTFB(int slot)
{
	register unsigned char *dstPtrB;

	if(!TFBInSlot(slot)){
		printf("The card in slot %d is not a TFB card!\n",slot);
		return;
	}
	dstPtrB = (CARDBASE(slot) + TFBBase);
	NewFieldTFB(slot);
	*(dstPtrB + 15*4) = (char) 0xB7;	/* Preload register15. Put the TFB into a reset state */
	/* 0xB7 apparently requests interlace clock off and 0 bits/pixel */
}

void RestartTFB(int slot,short bits)
{
	register unsigned char *dstPtrB, *srcPtrB;
	register long int i;
	short int log2bits;		/* = log2(bits) */

	if(!TFBInSlot(slot)){
		printf("The card in slot %d is not a TFB card!\n",slot);
		return;
	}
	if(bits > 8) return;
	log2bits = 0;
	while ( (bits /= 2) > 0 ) log2bits++;
	dstPtrB = (CARDBASE(slot) + TFBBase);
	srcPtrB = &(TFBRegisterValues[log2bits][0]);	
	*(dstPtrB + 15*4) = (char) 0xB7;	/* Preload register15. Put the TFB into a reset state */
	/* 0xB7 apparently requests interlace clock off and 0 bits/pixel */
	for(i=15; i>=0; i--) {
		*dstPtrB = ~ *srcPtrB++;
		dstPtrB += 4;
	}
	return;
}

void HaltDeviceTFB(device)
GDHandle device;
{
	HaltTFB(GetDeviceSlot(device));
}

void RestartDeviceTFB(device)
GDHandle device;
{
	RestartTFB(GetDeviceSlot(device), (**(**device).gdPMap).pixelSize);
}

void ScrollTFB(int slot,short bits,long x,long y)
{
	register unsigned char *srcPtrB;
	long int s, RowBytes;

	if(!TFBInSlot(slot)){
		printf("The card in slot %d is not a TFB card!\n",slot);
		return;
	}
	RowBytes = 1024*bits/8;
	s = 32 + x + RowBytes*y;
	s /= 4;			/* Unfortunately the Base Address is specified in 4 byte words */
	srcPtrB = CARDBASE(slot) + TFBBase;
	*(srcPtrB + 3*4) = ~ s%256;
	s /= 256;
	*(srcPtrB + 2*4) = ~ s%256;
/*
	s /= 256;
	*(srcPtrB + 15*4) = ~ (4*(s%2) + TFBRegisterValues[3][15]);
*/
}

void PanTFB(int slot,long int x)
/*
PanTFB() pans the display horizontally.
x is the desired starting point in memory, in bytes. 
For simplicity I assume that the vertical scroll is zero.
*/
{
	register unsigned char *srcPtrB;

	if(!TFBInSlot(slot)){
		printf("The card in slot %d is not a TFB card!\n",slot);
		return;
	}
	x /= 4;			/* Unfortunately the Base Address is specified in 4 byte words */
	x += 8;			/* Apple's origin */
	srcPtrB = CARDBASE(slot) + TFBPan;
	*srcPtrB = ~ x;
	return;
}

