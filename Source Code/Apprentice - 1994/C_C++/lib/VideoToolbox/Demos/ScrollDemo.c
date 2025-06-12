/* ScrollDemo.c
ScrollDemo works ONLY on the original Apple color video cards, which are no
longer sold by Apple but are available for $90 from Shreve Systems
(800-227-3971). The supported video cards are called "Toby frame buffer card" or
"Mac II High-Resolution Video Card".  ScrollDemo will refuse to run on any other
video card. The scroll speed naturally depends on the video depth, i.e.
bits/pixel.
10/29/88	dgp	wrote it
10/17/90	dgp	removed unused variables
2/16/91		dgp	added check for fpu and color quickdraw
8/24/91	dgp	Made compatible with THINK C 5.0.
3/10/92	dgp	include mc68881.h
8/27/92	dgp	replace SysEnvirons() by Gestalt()
*/
#include "VideoToolbox.h"

void ScrollDemo(void);

void main(void)
{
	Require(gestalt8BitQD);
	ScrollDemo();
}

void ScrollDemo(void)
{
	long int x,y;
	int bits=8;
	int slot;
	GDHandle device;

	printf("Welcome to ScrollDemo.\n");
	device = GetDeviceList();
	while(device!=NULL){
		if(TestDeviceAttribute(device,screenDevice) &&
			EqualString("\p.Display_Video_Apple_TFB",GDName(device),1,1))break;
		device=GetNextDevice(device);
	}
	if(device==NULL){
		PrintfExit("Sorry, this program only works with the original Apple Macintosh video cards:\n"
			"“Toby frame buffer card” and “Display_Video_Apple_TFB”\n" );
	}
	slot=GetDeviceSlot(device);

	/* these calls are just for fun, to show that the calls don't crash */
	SetUpTFB(slot);
	
	SetPriority(7);	/* disable interrupt for smooth scrolling */

	/* demonstrate horizontal panning */
	y=0;
	for(x=0; x<=1028; x+=4){
		NewBlankingTFB(slot);
		PanTFB(slot,x);
	}
	ScrollTFB(slot,bits,0,0);
	for(x=0;x<=24;x++)NewBlankingTFB(slot);
	
	/* demonstrate horizontal and vertical panning */
	for(y=0,x=0; y<=256; x+=4, y++){
		NewBlankingTFB(slot);
		ScrollTFB(slot,bits,x,y);
	}

	/* restore default values */
	ScrollTFB(slot,bits,0,0);

	SetPriority(0);	/* re-enable interrupt to unfreeze mouse */
}

