/*
GDInfo.c

GDInfo fills the user-supplied VideoInfo record with all the descriptive
information that can be obtained quickly from the driver and GDevice record
without disturbing the screen. Under 1-bit QuickDraw it ignores the card->device
field.

GDInfoTime fills the user-supplied VideoInfo record with the results of calling
most of the routines in GDTime.c. Under 1-bit QuickDraw it ignores the "card->device"
argument.

HISTORY:
4/16/93	dgp set pages=1 for 1-bit qd.
4/26/93	dgp	test HasDepth before relying on it.
4/27/93	dgp	replaced HasDepth by GDHasMode.
*/
#include <VideoToolbox.h>
#include "GDInfo.h"

OSErr GDInfo(VideoInfo *card)
{
	OSErr error;
	short pixelSize,pages,d,mode,ok,hasDepthOk;
	Rect r;
	long qD;
	ColorSpec cSpec;
	char *s;
	
	Gestalt(gestaltQuickdrawVersion,&qD);
	if(qD<gestalt8BitQD){
		card->device=NULL;
		sprintf(card->cardName,"%s","Original 1-bit QuickDraw");
		sprintf(card->driverName,"%s","Original 1-bit QuickDraw");
		for(d=1;d<6;d++)card->depth[d].pixelSize=0;
		card->slot=0;
		CopyQuickDrawGlobals();	// make sure qd is valid.
		r=qd.screenBits.bounds;
		card->width=r.right-r.left;
		card->height=r.bottom-r.top;
		card->depth[0].pixelSize=1;
		card->depth[0].clutSize=2;
		card->depth[0].pages=1;
		card->depth[0].framesPerClutUpdate=NAN;
		card->depth[0].framesPerClutUpdateHighPriority=NAN;
		card->depth[0].framesPerClutUpdateQuickly=NAN;
		card->depth[0].vblPerFrame=1.0;
		card->d=0;
		card->dacSize=1;
		card->dacMask=((1<<card->dacSize)-1)<<(16 - card->dacSize);
		card->basicTested=1;
		return 0;
	}
	if(!card->basicTested){
		// This info never changes, so get it only once
		for(d=0;d<6;d++){
			card->depth[d].pixelSize=0;
			card->depth[d].mode=0;
			card->depth[d].pages=0;
		}
		d=0;
		for(mode=0x80;mode<=0x85 && d<6;mode++){
			if(GDHasMode(card->device,mode,&pixelSize,&pages)){
				card->depth[d].mode=mode;
				card->depth[d].pixelSize=pixelSize;
				card->depth[d].pages=pages;
				d++;
			}
		}
		sprintf(card->cardName,"%s",s=GDCardName(card->device));
		DisposePtr(s);
		if(GDVersion(card->device)==0)sprintf(card->driverName,"%#s"
			,GDName(card->device));
		else sprintf(card->driverName,"%#s version %d"
			,GDName(card->device),GDVersion(card->device));
		card->slot=GetDeviceSlot(card->device);
		r=(**(**card->device).gdPMap).bounds;
		card->width=r.right-r.left;
		card->height=r.bottom-r.top;
		card->dacSize=GDDacSize(card->device);		// Takes 200 µs.
		card->dacMask=((1<<card->dacSize)-1)<<(16 - card->dacSize);
		card->setEntriesQuickly=(GetCardType(card->device)!=0);
		card->gdGetEntries=(0==GDGetEntries(card->device,0,0,&cSpec));
		card->basicTested=1;
	}
	// d changes any time you call SetDepth, so update it every time.
	for(d=0;d<6;d++)if(card->depth[d].mode==(**card->device).gdMode)card->d=d;
	
	// Collect information from the GDevice record
	card->depth[card->d].pixelSize=(**(**card->device).gdPMap).pixelSize;
	card->depth[card->d].clutSize=GDClutSize(card->device);
	return 0;
}

OSErr GDInfoTime(VideoInfo *card)
{
	OSErr error;
	short clutSize=0,d=card->d;
	double frames,s,missingFrames,frameRate;
	long qD;
	char blankLine[]="\r"	"          " "          " "          " "          "
							"          " "          " "          " "          " "\r";
	printf(blankLine);	
	printf("%d-bit pixels: timing CopyBits . . .\r",card->depth[d].pixelSize);
	card->depth[d].movieRate=GDMovieRate(card->device,0);
	printf(blankLine);	
	printf("%d-bit pixels: timing CopyBitsQuickly . . .\r",card->depth[d].pixelSize);
	card->depth[d].movieRateQuickly=GDMovieRate(card->device,1);
	Gestalt(gestaltQuickdrawVersion,&qD);
	if(qD>=gestalt8BitQD){
		printf(blankLine);	
		printf("%d-bit pixels: timing GDSetEntries . . .\r",card->depth[d].pixelSize);
		error=GDTimeClut(card->device,GDSetEntries,clutSize,&s,&frames,&missingFrames,&frameRate);
		if(error)frameRate=GDFrameRate(card->device);
		card->depth[d].frameRate=frameRate;
		card->depth[d].vblPerFrame=GDVBLRate(card->device)/frameRate;
		card->depth[d].framesPerClutUpdate=frames;
		card->depth[d].missingFramesPerClutUpdate=missingFrames;
	}else{
		card->depth[d].frameRate=GDFrameRate(NULL);
		card->depth[d].vblPerFrame=1.0;
		card->depth[d].framesPerClutUpdate=NAN;
		card->depth[d].missingFramesPerClutUpdate=NAN;
	}
	if(qD>=gestalt8BitQD){
		printf(blankLine);	
		printf("%d-bit pixels: timing GDSetEntriesByTypeHighPriority . . .\r",card->depth[d].pixelSize);
		error=GDTimeClut(card->device,GDSetEntriesByTypeHighPriority,clutSize,&s,&frames,&missingFrames,&frameRate);
		card->depth[d].framesPerClutUpdateHighPriority=frames;
		card->depth[d].missingFramesPerClutUpdateHighPriority=missingFrames;
	}else{
		card->depth[d].framesPerClutUpdateHighPriority=NAN;
		card->depth[d].missingFramesPerClutUpdateHighPriority=NAN;
	}
	if(qD>=gestalt8BitQD && GetCardType(card->device)){
		printf(blankLine);	
		printf("%d-bit pixels: timing SetEntriesQuickly . . .\r",card->depth[d].pixelSize);
		error=GDTimeClut(card->device,SetEntriesQuickly,clutSize,&s,&frames,&missingFrames,&frameRate);
		card->depth[d].framesPerClutUpdateQuickly=frames;
		card->depth[d].missingFramesPerClutUpdateQuickly=missingFrames;
	}else{
		card->depth[d].framesPerClutUpdateQuickly=NAN;
		card->depth[d].missingFramesPerClutUpdateQuickly=NAN;
	}
	printf(blankLine);	
	card->timeTested=1;
	card->depth[d].timeTested=1;
}