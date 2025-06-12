/*
PixMapToPICT.c
Saves a section of a PixMap (or BitMap) as a PICT file. You may substitute a
BitMap handle for the PixMap handle. You are allowed to specify the PICT's pixel
size and the color table used in translating the PixMap to a PICT. If you
request zero pixelSize, the PICT's pixelSize will be the same as that of your
PixMap. If the supplied color table handle is NULL then the Macintosh operating
system default color table for that pixel size will be used. Note that PICT
files do not include a color table, and typically are displayed with a default
color table. If you want to preserve the numerical value of each pixel then set
pixelSize to zero (i.e. unchanged) and supply the color table of your PixMap,
i.e. (**pm).pmTable.

EXAMPLE:
	PixMapToPICT("image.pict",((CWindowPtr)window)->portPixMap,&window->portRect,0,NULL);

BUGS:
It works if you use the current device (NEW_DEVICE set to 0), but produces a bad
Picture if you ask NewGWorld to create a new device and make that the current
device (NEW_DEVICE set to 1). The bad Picture seems to be compressed
horizontally, and to have the wrong row length, resulting in skew due to
wraparound, as though it were accessing a pixmap with the wrong pixelsize or
rowlength. (The bad Picture can be saved to a file and opened, e.g., in
TeachText, but still looks just as bad.) I haven't read Apple's documentation of
OpenPicture(), but, at first sight, this would appear to be a bug in QuickDraw.

QUESTIONS:
We're using OpenPicture() to capture a CopyBits command, nothing else. Is the
resulting Picture affected by the pixelSize of the destination pixmap? Is the
resulting Picture affected by the color table of the current device? These
things affect the color translation that CopyBits normally performs, but it is
possible that all of the CopyBits transformations are deferred until the Picture
is actually drawn by DrawPicture. In that case there would be no need to worry
about the destination port's pixelSize and current device used here. However,
things can't be that rosy, as in that case it shouldn't matter that I've set the
current device to be the new device created by NewGWorld. In that case I've
found that the picSize is approximately proportional to the pixelSize of the
destination GWorld. By the way, I've confirmed that all the GWorld stuff is
correct by commenting out the Picture stuff and confirming that I can copy the
source to destination and back faithfully.

HISTORY:
9/28/93	mike schechter wrote it, based in part on PixMapToPostScript.c
9/29/93	dgp polished it, substituting a GWorld for a CPort, 
			so we can specify the pixel size and color table.
10/2/93	dgp	added pixelSize and colorTable arguments.
*/
#define NEW_DEVICE 0
#define DEBUG 1

#include "VideoToolbox.h"
#include <Errors.h>

void PixMapToPICT(char *filename,PixMap **pm,Rect *rectPtr
	,int pixelSize,ColorTable **cTable)
{
	FILE *file;
	long buffer[128],n,value;
	short error;
	PicHandle pic;
	CGrafPtr oldPort;
	GDHandle oldDevice,device;
	int i;
	GWorldPtr world;
	Rect r;
	GWorldFlags flags;
	
	Gestalt(gestaltQuickdrawVersion,&value);
	if(value<gestalt32BitQD)
		PrintfExit("Sorry. PixMapToPICT requires 32-bit QuickDraw.\n");
	if(pixelSize==0)if((**pm).rowBytes & 0x8000)	// Pixmap or Bitmap?
		pixelSize=(**pm).pixelSize;		// Pixmap
	else pixelSize=1;					// Bitmap

	// draw pixmap into a Picture
	GetGWorld(&oldPort,&oldDevice);
	if(NEW_DEVICE){
		// Ask NewGWorld to create a device, using supplied pixelSize and cTable.
		flags=0;
		device=NULL;
	}else{
		// Use the current device. (Ignores supplied pixelSize and cTable.)
		flags=noNewDevice;
		device=oldDevice;
	}
	error=NewGWorld(&world,pixelSize,rectPtr,cTable,device,flags);
	if(error==cNoMemErr || error==memFullErr)
		error=NewGWorld(&world,pixelSize,rectPtr,cTable,device,flags|useTempMem);
	if(error==cTempMemErr)
		PrintfExit("PixMapToPICT: not enough memory; reduce rect or pixelSize.\n");
	if(error)PrintfExit("PixMapToPICT: NewGWorld error %d.\n",error);
	LockPixels(world->portPixMap);
	SetGWorld(world,NULL);
	ClipRect(rectPtr);
	pic=OpenPicture(rectPtr);
	CopyBits((BitMap *)*pm,(BitMap *)*world->portPixMap
		,rectPtr,rectPtr,srcCopy,NULL);
	ClosePicture();
	SetGWorld(oldPort,oldDevice);
	if(DEBUG){
		// Confirm that Picture is ok. Just for debugging.
		printf("pixelSize %d\n",(**world->portPixMap).pixelSize);
		printf("Picture size %d\n",(**pic).picSize);
		EraseRect(rectPtr);
		DrawPicture(pic,rectPtr);
	}
	DisposeGWorld(world);	
	error=QDError();
	if(error)PrintfExit("PixMapToPICT: QDError %d.\n",QDError());
	if(EmptyRect(&(*pic)->picFrame) && !EmptyRect(rectPtr))
		PrintfExit("PixMapToPICT: out of memory. Reduce rect or pixelSize.\n");
	
	// save Picture to a file
	file=fopen(filename,"wb");
	if(file==NULL)PrintfExit("PixMapToPICT: Error in opening file \"%s\".\n"
		,filename);
	// zero 512-byte header
	for(i=0;i<128;i++)buffer[i]=0;
	if(128!=fwrite(buffer,4,128,file))
		PrintfExit("PixMapToPICT: Error writing header of file \"%s\".\n",filename);
	n=GetHandleSize((Handle)pic);
	n=(n+1)&~1L; /* round up to multiple of 2 */
	HLock((Handle)pic);
	if(n!=fwrite(*pic,1,n,file))
		PrintfExit("PixMapToPICT: Error writing file \"%s\"\n",filename);
	fclose(file);
	SetFileInfo(filename,'PICT','ttxt');
	KillPicture(pic);
}

