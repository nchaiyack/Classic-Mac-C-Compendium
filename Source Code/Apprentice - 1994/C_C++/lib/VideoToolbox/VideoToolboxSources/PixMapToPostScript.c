/*
PixMapToPostScript.c
Copyright © 1991,1992,1993 Denis G. Pelli

	PixMapToPostScript(filename,pm,rectPtr,pageRectPtr,cellsPerInch,grayLevels);

	This is a simple but reasonably general routine to convert a grayscale
PixMap to a PostScript file that may be transmitted to a LaserWriter or Linotype
to produce a halftone image on paper. We use it a lot; it works well.
	The filename, by convention, should end in ".ps" to indicate that it is
a postscript file, but this is not enforced. The file's type is set to 'TEXT'
with creator '4PSU', so that it can be double-clicked to open my favorite
PostScript downloading program, PostHaste. However, it's a plain text file so
any downloader will work.
	The PixMap must have 8 bits per pixel. No color tables are used. The raw
pixel value (Apple calls it an "index") is sent directly to the printer with no
transformation. PostScript assumes that the number, from 0 to 255 is
proportional to desired reflectance, from zero to 1. The *rectPtr indicates what
part of your PixMap is to be used.
	The *pageRectPtr is subtle. It describes, in typographers points
(1/72"), the rectangle that your image will be mapped onto on the printed page.
It is essential that you keep in mind that Apple and Adobe use different
coordinate systems. Both Apple and Adobe increase x from left to right. However,
Apple has y increasing from top to bottom, whereas Adobe increases y from bottom
to top. Adobe's origin is the lower left corner of the page, even though that
point is usually not printable, since most printers can only print to within
about a half inch of the edge. The pageRect, though supplied in Apple's Rect
data structure, must be in Adobe's coordinates, respecting the names of the Rect
structure's fields: left, top, right, bottom. So, for an image to fill most of
an 8.5x11 page, with 0.5" margins, you might use the following:

		SetRect(&pageRect,0.5*72,10.5*72,8*72,0.5*72);

	In printing PostScript halftones the halftone cell size determines both
the spatial and graylevel resolutions of the resulting image. For the convenience
of the user this can be specified by setting either the cellsPerInch or the
grayLevels argument to the desired value; the other one should be zero. If both
are zero then the printer will be left at its default cell size, which is
usually a good choice. Note that there need not be any particular correspondence
between pixels in your image and cells in the halftone; the printer
automatically resamples your image to produce the halftone.
	If you set cellsPerInch to a nonzero value then the printer will be
asked to print its halftone with that many halftone cells per inch. E.g. to
produce a halftone original for subsequent one-to-one reproduction in a journal,
you'll want the cells to be coarse enough for them to reproduce without
re-screening, e.g. 100 cells per inch.
	Alternatively, if you set grayLevels to a nonzero value then the printer
will be asked to print its halftone with cells containing grayLevels-1 printer
pixels, yielding the specified number of gray levels. E.g. you might want to
force your 300 dpi LaserWriter to use big cells yielding 256 gray levels.
	You should remove() any pre-existing old file of the same name, unless
you want to append to it. PixMapToPostScript always appends to an existing file
with the same name, if one exists, to allow you to place several images onto a
page. (You should offset their pageRects unless you want the images to
superimpose.)
	You will need to add a "showpage\n" command at the end of the postscript
file. This is the command that tells the printer to go ahead and print
the page. Use the subroutine AppendToFile(filename,"showpage\n"), supplied for this
purpose. Naturally, you could also use to this to add your own postscript
commands before or after the imaging code inserted by PixMapToPostScript.
	Here's a minimal example of the three commands:

		remove("test.ps");
		PixMapToPostScript("test.ps",pixMapHandle,&rect,&pageRect,0.0,0);
		AppendToFile("test.ps","showpage\n");

	To print a screen to disk, preserving the size and scale of the image,
try this:

		remove("test.ps");
		pageRect=window->portRect;
		pageRect.top*=-1;		// convert from Apple to Adobe coordinates
		pageRect.bottom*=-1;	// convert from Apple to Adobe coordinates
		SetRect(&paperRect,0,11*72,8.5*72,0);
		CenterRectInRect(&pageRect,&paperRect);
		PixMapToPostScript("test.ps",window->portPixMap,&window->portRect,&pageRect,0,0);
		AppendToFile("test.ps","showpage\n");

	Tiling is something we often want to do, creating a huge image by taping
many pages together. You accomplish this by repeatedly printing the huge
image--only one pageful appears each time--shifting the image so that eventually
every bit has been printed. (It's slow, since the whole image is
transmitted to the printer each time.) Here's an example that creates a big "width"
by "height" image. We tile onto multiple 8.5"x11" pages, using 7.5"x10" of each
page, allowing for 0.5" nonprinting margins:

		remove("test.ps");
		SetRect(&mosaicRect,0,height,width,0);	// In units of "points". 72 points/inch.
		OffsetRect(&mosaicRect,0.5*72,0.5*72);	// allow for nonprinting margin
		for(i=0;i<width;i+=7.5*72)for(j=0;j<height;j+=10*72){
			pageRect=mosaicRect;
			OffsetRect(&pageRect,-i,-j);
			PixMapToPostScript("test.ps",pixMapHandle,&rect,&pageRect,0.0,0);
			AppendToFile("test.ps","showpage\n");
		}

	In case your PostScript manual isn't handy, the easy way to obtain
multiple copies of each page is to use the #copies variable that's built into
Postscript. Anywhere in your file before "showpage", set #copies to the desired
value. E.g.

		AppendToFile("test.ps","/#copies 3 def\n");
		
	There are many free PostScript downloaders, but I find them clumsy to
use, especially the ones that don't inform you of errors, should they occur. I
bought and use
PostHaste
from:
Micro Dynamics, Ltd.
8555 16th St., Suite 802
Silver Spring, MD 20910
(301)-589-6300
and
LaserStatus, a desk accessory included in the 
MockPackage Plus Utilities
from:
CE Software
1854 Fuller Road
PO Box 65580
West Des Moines, Iowa 50265
(515)-224-1995

You may also want to read:

Adobe Systems (1985) PostScript Language Reference Manual, Second Edition. 
Reading, MA: Addison-Wesley.

Pelli, D. G. (1987) Programming in PostScript: Imaging on paper from a mathematical
description. BYTE, 12 (5), 185-202.

HISTORY:
4/21/91	dgp	wrote it
7/24/91 dgp added comment about shifting pageRect
8/24/91	dgp	Made compatible with THINK C 5.0
12/7/91	dgp	minor editing of comments
10/10/92 dgp Added support for Pixmap's that require 32-bit addressing.
			Much faster now, using table-lookup instead of sprintf for 
			the hex encoding.
			Deleted obsolete support for THINK C 4.
4/29/93	dgp	& jas Added explanation of how to do tiling.
5/27/93	dgp	minor editing for speed and clarity
6/15/93	dgp	Fixed silly bug introduced 5/27/93 that suppressed all hex data.
			Call StripAddress.
6/29/93	dgp	Changed call interface to accept a pixmap handle instead of a pointer.
			The problem with accepting a pointer is that the user must remember
			to lock the handle before dereferencing it to get the pointer, and
			lots of people forget, leading to a mysterious crash. Now the locking
			is handled internally, automatically.
6/30/93	dgp	Added example, above, showing how to print screen to disk.
7/9/93	dgp check for 32-bit addressing capability.
12/15/93 dgp added filename argument to the diagnostic messages.
			Corrected grayLevels to grayLevels-1 in computing
			the required number of pixels in the halftone cell.
*/
#include "VideoToolbox.h"
#include <assert.h>
#include <math.h>
#define CREATOR '4PSU'	/* identifies the postscript file as a PostHaste document */

void PixMapToPostScript(char *filename,PixMap **pm,Rect *rectPtr
	,Rect *pageRectPtr,double cellsPerInch,int grayLevels)
{
	FILE *file;
	unsigned char *addr;
	long y,bytes,width;
	register long i;
	unsigned short *buffer,*buffer32;
	register unsigned short *word,hex[256];
	register unsigned char *byte;
	short pixelSize;
	short rowBytes;
	time_t ANSITime;
	char string[100];
	char mode,pmState;
	Boolean can32;
	long value=0,error;

	error=Gestalt(gestaltAddressingModeAttr,&value);
	can32=!error && (value&(1<<gestalt32BitCapable));
	if(cellsPerInch!=0.0 && grayLevels!=0.0)PrintfExit("PixMapToPostScript(%s): "
		"you may not specify BOTH cellsPerInch & grayLevels.\n"
		"Set one to zero.\n",filename);
	file=fopen(filename,"a");
	if(file==NULL)PrintfExit("PixMapToPostScript: Error in opening file Ò%sÓ.\n"
		,filename);
	pmState=HGetState((Handle)pm);
	HLock((Handle)pm);
	addr=RectToAddress(*pm,rectPtr,&rowBytes,&pixelSize,NULL);
	if(addr==NULL)PrintfExit("PixMapToPostScript(%s): Bad PixMap.\n",filename);
	if(pixelSize!=8)PrintfExit("PixMapToPostScript(%s): "
		"Sorry, pixelSize must be 8, not %d.\n",filename,pixelSize);
	time(&ANSITime);
	strftime(string,sizeof(string),"%I:%M %p %A, %B %d, %Y",localtime(&ANSITime));
	fprintf(file,
		"\nsave					%% %s, %s\n",filename,string);
	fprintf(file,
		"/nx %d def				%% pixels per raster line\n",rectPtr->right-rectPtr->left);
	fprintf(file,
		"/ny %d def				%% lines in image\n",rectPtr->bottom-rectPtr->top);
	fprintf(file,
		"%d %d translate			%% locate lower left of image\n",pageRectPtr->left
		,pageRectPtr->bottom);
	fprintf(file,
		"%d %d scale			%% print image with these dimensions on page\n"
		,pageRectPtr->right-pageRectPtr->left,pageRectPtr->top-pageRectPtr->bottom);
	fprintf(file,
		"/hypotenuse {dup mul exch dup mul add sqrt} bind def\n"
		"/pixelsPerInch gsave initmatrix 72 0 dtransform hypotenuse grestore def\n");
	if(cellsPerInch!=0.0){
		fprintf(file,
			"/cellsPerInch %.2f def	%% halftone dot frequency\n",cellsPerInch);
		fprintf(file,
			"cellsPerInch currentscreen 4 -2 roll pop 3 1 roll setscreen\n");
	}
	if(grayLevels!=0){
		fprintf(file,
			"/cellsPerInch pixelsPerInch %.2f div def	%% halftone dot frequency\n"
			,sqrt(grayLevels-1));
		fprintf(file,
			"cellsPerInch currentscreen 4 -2 roll pop 3 1 roll setscreen\n");
	}
	fprintf(file,
		"/s nx string def		%% string to hold one raster line\n"
		"nx ny 8					%% dimensions and bits/pixel of source image\n"
		"[nx 0 0 ny neg 0 ny]	%% map unit square to PixMap data\n"
		"{currentfile s readhexstring pop}	%% read data\n"
		"bind					%% speed up reading\n"
		"image\n");
	assert(sizeof(*byte)==1);	// required by our algorithm
	assert(sizeof(*word)==2);	// required by our algorithm
	for(i=0;i<256;i++){
		sprintf(string,"%02x",(int)i);
		hex[i]=*(unsigned short *)string;
	}
	width=rectPtr->right-rectPtr->left;
	bytes=(width+1)*sizeof(*word);
	buffer=(unsigned short *)NewPtr(bytes);
	if(buffer==NULL)PrintfExit("PixMapToPostScript(%s): "
		"no room for %ld byte buffer.\n\007",filename,bytes);
	buffer32=(unsigned short *)StripAddress(buffer);
	for(y=rectPtr->top;y<rectPtr->bottom;y++){
		mode=true32b;
		if(can32)SwapMMUMode(&mode);
		byte=addr;
		word=buffer32;
		for(i=0;i<width;i++) *word++ = hex[*byte++];
		*word=0;
		if(can32)SwapMMUMode(&mode);
		fprintf(file,"%s\n",(char *)buffer);
		addr+=rowBytes;
	}
	HSetState((Handle)pm,pmState);
	DisposPtr((Ptr)buffer);
	fprintf(file,"restore\n");
	fclose(file);
	SetFileInfo(filename,'TEXT',CREATOR);
}

void AppendToFile(char *filename,char *string)
{
	FILE *file;
	
	file=fopen(filename,"a");
	if(file==NULL)PrintfExit("AppendToFile: Error in opening file \"%s\"\n",filename);
	fprintf(file,"%s",string);
	fclose(file);
}