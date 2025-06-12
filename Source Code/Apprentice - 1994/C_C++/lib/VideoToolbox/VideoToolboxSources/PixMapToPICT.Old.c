/*
PixMapToPICT.c
Saves a section of a PixMap as a PICT file.

HISTORY:
9/28/93	mike schechter wrote it, based in part on PixMapToPostScript.c
9/29/93	dgp polished it, substituting a GWorld for a CPort, so that a
			custom GDevice will be used, instead of whatever is current,
			so we'll preserve our current pixel size and color table.
*/

#include "VideoToolbox.h"

void PixMapToPICT(char *filename,PixMap **pm,Rect *rectPtr
	,int pixelSize,ColorTable **cTable)
{
	FILE *file;
	long buffer[128],n,value;
	short error;
	PicHandle pic;
	CGrafPtr oldPort;
	GDHandle oldDevice;
	int i;
	GWorldPtr world;
	
	Gestalt(gestaltQuickdrawVersion,&value);
	if(value<gestalt32BitQD)
		PrintfExit("Sorry. PixMapToPICT requires 32-bit QuickDraw.\n");

/* draw pixmap into a Picture */
	GetGWorld(&oldPort,&oldDevice);
	error=NewGWorld(&world,(**pm).pixelSize,rectPtr,(**pm).pmTable,NULL,0);
	if(error)PrintfExit("PixMapToPICT: NewGWorld error %d.\n",error);
	SetGWorld(world,NULL);
	LockPixels(world->portPixMap);
	ClipRect(rectPtr);
	pic=OpenPicture(rectPtr);
	EraseRect(rectPtr);
	CopyBits((BitMap *)*pm,(BitMap *)*world->portPixMap
		,rectPtr,rectPtr,srcCopy,NULL);
	ClosePicture();
	SetGWorld(oldPort,oldDevice);
	DisposeGWorld(world);
	if(EmptyRect(&(*pic)->picFrame))PrintfExit("PixMapToPICT: out of memory.\n");
	
/* save Picture to a file */
	file=fopen(filename,"wb");
	if(file==NULL)PrintfExit("PixMapToPICT: Error in opening file \"%s\".\n"
		,filename);
	/* zero 512-byte header */
	for(i=0;i<128;i++) buffer[i]=0;
	if(128!=fwrite(buffer,4,128,file))
		PrintfExit("PixMapToPICT: Error writing header of file \"%s\".\n",filename);
	n=GetHandleSize((Handle)pic);
	if(n&1 !=0)n++;	/* pad to even length */
	if(n!=fwrite(*pic,1,n,file))
		PrintfExit("PixMapToPICT: Error writing file \"%s\"\n",filename);
	fclose(file);
	SetFileInfo(filename,'PICT','ttxt');
	KillPicture(pic);
}

