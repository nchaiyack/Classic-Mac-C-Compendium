/*
**  MacWT -- a 3d game engine for the Macintosh
**  © 1995, Bill Hayden and Nikol Software
**
**  On the Internet:
**  bmoc1@aol.com (my personal address)
**  nikolsw@grove.ufl.edu (my school address)
**	MacWT anonymous FTP site: ftp.circa.ufl.edu/pub/software/ufmug/mirrors/LocalSW/Hayden/
**  http://grove.ufl.edu:80/~nikolsw (my WWW page, containing MacWT info)
**
**  This program is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*/



#include <stdlib.h>
#include <stdio.h>
#include "wt.h"
#include "error.h"
#include "wtmem.h"
#include "color.h"
#include "graphfile.h"
#include "StringUtils.h"


static PicHandle PictFromFile(short fref);


Graphic_file *LoadPICTR(FILE *fp, char *filename)
{
	Graphic_file	*gfile = nil;
	PicHandle		hPic;
	RGBColor		thePixel;
	long			index = 0;
	short			h, v;
	WindowPtr		wind;
	GrafPtr			savePort;
	Rect			WindowRect;
	short			vref, HomeRes, fref;
	OSErr			err = noErr;
	

	HomeRes = CurResFile();
	
	err = GetVol(nil, &vref);
	c2p(filename);
	fref = OpenResFile( (StringPtr)filename );

	if (err || fref == -1)
		goto bail;
	
	UseResFile( fref );
	
	if (Count1Resources( 'PICT' ))
		{
		hPic = (PicHandle)Get1IndResource( 'PICT', 1 );
		vref = -1;
		}
	else
		{
		err = FSClose( fref );
		
		err = OpenDF( (StringPtr)filename, vref, &fref );
		hPic = PictFromFile(fref);
		err = FSClose (fref);
		}
		
	if (hPic)
		{		
		gfile = new_graphic_file();
		gfile->type = gfTrueColor;
		gfile->palette = NULL;
		gfile->width = (*hPic)->picFrame.right - (*hPic)->picFrame.left;
		gfile->height = (*hPic)->picFrame.bottom - (*hPic)->picFrame.top;
		gfile->bitmap = wtmalloc(gfile->height * gfile->width * 3);
	
		SetRect (&WindowRect,	(*hPic)->picFrame.left,
								(*hPic)->picFrame.top + 20,
								(*hPic)->picFrame.right,
								(*hPic)->picFrame.bottom + 20);
	
		GetPort(&savePort);
		wind = NewCWindow(nil, &WindowRect, "\p", TRUE, 0, (WindowPtr)-1, FALSE, 0);
		SetPort(wind);
		DrawPicture(hPic, &wind->portRect);
			
		for (v = 0; v < (gfile->height); v++)
			for (h = 0; h < (gfile->width); h++)
				{
				GetCPixel(h, v, &thePixel);
				gfile->bitmap[index++] = (unsigned char)(thePixel.red >> 8);
				gfile->bitmap[index++] = (unsigned char)(thePixel.green >> 8);
				gfile->bitmap[index++] = (unsigned char)(thePixel.blue >> 8);
				}
		
		HideWindow(wind);
		DisposeWindow(wind);
		SetPort(savePort);
		
		if (vref == -1)
			ReleaseResource((Handle)hPic);
		else
			DisposHandle((Handle)hPic);
		}

	UseResFile(HomeRes);
	
bail:
	p2c((StringPtr)filename);
	
	return gfile;
}




static PicHandle PictFromFile(short fref)
{
	PicHandle	thePic;
	const short kPictHeaderSize = 512;	// Length of PICT header in bytes
	long		pictFileLen;			// Length of the PICT file
	OSErr		err;
	
	
	err = GetEOF(fref, &pictFileLen);

		// Skip over the 512 byte header which we don't need
	err = SetFPos(fref, fsFromStart, kPictHeaderSize);

		// Adjust the length of the pict file (shorten by 512) to
		// account for our skipping the first 512 bytes
	pictFileLen -= kPictHeaderSize;
		
	if (pictFileLen <= 0)
		return nil;
		
		// Now, allocate some memory with which to load our pict file in.
	thePic = (PicHandle)NewHandleClear(pictFileLen);
	if (thePic == nil)
		return(nil);

		// Lock the buffer
	HLock((Handle)thePic);

		// Okay, read in the pict file
	err = FSRead(fref, &pictFileLen, (Ptr)*thePic);
	
	HUnlock((Handle)thePic);
	
	return(thePic);	
}



