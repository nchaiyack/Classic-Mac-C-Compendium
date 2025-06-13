/*
==============================================================================
Project:	POV-Ray

Version:	2.2

File Name:	SaveCmpPict.c

Description:
	Routines for saving a PICT file in QuickTime compressed format.
	
	This source code was written with a lot of help from some examples
	off of Apple's QuickTime 1.0 Developer CD.

	Note that this source requires the as-yet non-standard header files
	and glue files for QuickTime and the Standard Compression dialog.
	These files will need to accompany this source until Apple rolls them
	into MPW/Think.  The files needed are listed below:
	Components.h			- MPW/Think - QuickTime Component header
	ImageCompression.h		- MPW/Think - QuickTime Image Compression header
	StdCompression.h		- MPW/Think - Std Compression dialog header
	StdCompression.rsrc		- MPW/Think - Std Compression dialog resource
	StdCompressionGlue.o	- MPW - Std Compression dialog glue routines
	StdCompressionGlue.¹	- Think - Std Compression dialog glue routines

Related Files:
	SaveCmpPict.h: header file for Save Compressed Pict routines
	SaveCmpPict.c: main file for Save Compressed Pict routines
------------------------------------------------------------------------------
Author:
	Eduard [esp] Schwan
------------------------------------------------------------------------------
	from Persistence of Vision Raytracer
	Copyright 1993 Persistence of Vision Team
------------------------------------------------------------------------------
	NOTICE: This source code file is provided so that users may experiment
	with enhancements to POV-Ray and to port the software to platforms other 
	than those supported by the POV-Ray Team.  There are strict rules under
	which you are permitted to use this file.  The rules are in the file
	named POVLEGAL.DOC which should be distributed with this file. If 
	POVLEGAL.DOC is not available or for more info please contact the POV-Ray
	Team Coordinator by leaving a message in CompuServe's Graphics Developer's
	Forum.  The latest version of POV-Ray may be found there as well.

	This program is based on the popular DKB raytracer version 2.12.
	DKBTrace was originally written by David K. Buck.
	DKBTrace Ver 2.0-2.12 were written by David K. Buck & Aaron A. Collins.
------------------------------------------------------------------------------
More Info:
	This Macintosh version of POV-Ray was created and compiled by Jim Nitchals
	(Think 5.0) and Eduard Schwan (MPW 3.2), based (loosely) on the original
	port by Thomas Okken and David Lichtman, with some help from Glenn Sugden.

	For bug reports regarding the Macintosh version, you should contact:
	Eduard [esp] Schwan
		CompuServe: 71513,2161
		Internet: jl.tech@applelink.apple.com
		AppleLink: jl.tech
	Jim Nitchals
		Compuserve: 73117,3020
		America Online: JIMN8
		Internet: jimn8@aol.com -or- jimn8@applelink.apple.com
		AppleLink: JIMN8
------------------------------------------------------------------------------
Change History:
	920414	[esp]	Created.
	920418	[esp]	Broke routines into UI and non-UI components for later factoring
	920419	[esp]	Embellished file header comments, renamed routines for consistency
	920611	[esp]	Worked on AppendFinderIcons2PictF function
	930526	[esp]	Updated SC dialog & Compression calls for QT 1.5
	931001	[esp]	version 2.0 finished (Released on 10/4/93)
==============================================================================
*/


/*==== Headers ====*/

#include "SaveCmpPict.h"

#include <errors.h>		// dupFNErr
#include <finder.h>		// for kCustomIconResource
#include <resources.h>
#include <icons.h>		// for large1BitMask etc.


/*
******************************************************************************
Name:
	AppendFilePreview2PictF
------------------------------------------------------------------------------
Purpose:
	Create a preview image for users of QuickTimeª standard File preview.
------------------------------------------------------------------------------
Description:
	** How it does it
------------------------------------------------------------------------------
Parameters:
------------------------------------------------------------------------------
When Used:
	** who calls me and why/when
******************************************************************************
*/
OSErr AppendFilePreview2PictF(FSSpec *fsFile)
{
	OSErr	anError = noErr;
	short	fileResRef;

	fileResRef = FSpOpenResFile(fsFile, fsRdWrPerm);
	anError = ResError();

	if (!anError && (fileResRef != -1))
	{
#if defined(__powerc)
		anError = MakeFilePreview(fileResRef, (ICMProgressProcRecordPtr)-1);
#else
		// ICMProgressProcRecordPtr is not in regular Mac 3.3 headers yet...
		anError = MakeFilePreview(fileResRef, (ProgressProcRecordPtr)-1);
#endif
		CloseResFile(fileResRef);
	}
	return anError;
} // AppendFilePreview2PictF


/*
******************************************************************************
Name:
	AppendFinderIcons2PictF
------------------------------------------------------------------------------
Purpose:
	Embed custom icons in file, for System 7.0 users to see from the Finder.
------------------------------------------------------------------------------
Description:
	** How it does it
------------------------------------------------------------------------------
Parameters:
------------------------------------------------------------------------------
When Used:
	** who calls me and why/when
******************************************************************************
*/
OSErr AppendFinderIcons2PictF(FSSpec *fsFile,
						Rect *theOriginalPicFrame,
						eAFI_ImagePrefs_t theImagePrefs)
{	
	OSErr		anError;
	GWorldPtr	iconGWorld = NULL,
				imageWorld = NULL;
	short		oldResFile;
	Rect		irect,
				orect,
				maxRect;
	Handle		tmpHandle;
	Handle		destIconRsrc;
	short		rowBytes,
				theWidth,
				theHeight,
				theDepth,
				iconCounter;
	char		mmumode;
	signed char theCorner;
	short		i,j;
	Ptr			srcPixPtr,
				tempDestPixPtr,
				tempSrcPixPtr;
	short		bytesPerRow;
	short		theSize;
	OSType		theRsrcType;
	FInfo		finfo;
	short		fileRef;

	oldResFile = CurResFile();	// remember old file

	// Create a GWorld to hold the 32x32 image from which to copy..
	SetRect(&orect, 0, 0, 32, 32);
	anError = NewGWorld(&imageWorld, 32, &orect, (CTabHandle)NULL, (GDHandle)NULL, (GWorldFlags)0);

	if (!anError)
	{
		SetGWorld(imageWorld, (GDHandle)NULL);
		EraseRect(&orect);
		maxRect = irect = *theOriginalPicFrame;
		theWidth = irect.right - irect.left;
		theHeight = irect.bottom - irect.top;

		// Figure out the source rectangle
		switch (theImagePrefs)
		{
		case eAFI_ShrinkWholeImage:
			/* this saves the whole image as the icon */
			irect = orect;	// shrink down to 32x32
			break;
		case eAFI_UseCenter:
			/* Questionable code.. need to scrutinize this! Not that I don't trust MacDTS..*/
			/* this takes a square from the center of the image */
			if (theWidth > theHeight) 
				maxRect.right = maxRect.left + theHeight;
			 else 
				maxRect.bottom = maxRect.top + theWidth;
			MapRect(&irect, &maxRect, &orect);
			theWidth = 32 - (irect.right - irect.left);
			theHeight = 32 - (irect.bottom - irect.top);
			OffsetRect(&irect, theWidth>>1, theHeight>>1);
			break;
		} // switch
	}

	// Draw the image into the GWorld
	if (!anError)
		anError = FSpOpenDF(fsFile, fsRdPerm, &fileRef);
	if (!anError)
	{
		anError = DrawPictureFile(fileRef, &irect, NULL);
		FSClose(fileRef);
	}

	// now going from 32x32 input image
	irect = orect;

	// Create/Open resource fork, in order to add icl resources
	fileRef = kRsrcFileClosed;
	if (!anError)
	{
		FSpCreateResFile(fsFile, 'ttxt','PICT', (ScriptCode)0);
		anError = ResError();
		if (!anError)
		{
			fileRef = FSpOpenResFile(fsFile, fsRdWrPerm);
			anError = ResError();
		}
	}

	// loop through each icon depth
	if (!anError)
	{ // if opened ok
		for (iconCounter=1; iconCounter <= 6 && !anError; iconCounter++)
		{
			switch(iconCounter)
			{
			case 1:
				theRsrcType = large1BitMask;
				theWidth = 32;
				theHeight = 32;
				SetRect(&orect, 0, 0, theWidth, theHeight);
				theDepth = 1;
				bytesPerRow = 4;
				theSize = theHeight*bytesPerRow*2;	// x2 'cause icon+mask
				break;
			case 2:
				theRsrcType = large4BitData;
				theWidth = 32;
				theHeight = 32;
				SetRect(&orect, 0, 0, theWidth, theHeight);
				theDepth = 4;
				bytesPerRow = 16;
				theSize = theHeight*bytesPerRow;
				break;
			case 3:
				theRsrcType = large8BitData;
				theWidth = 32;
				theHeight = 32;
				SetRect(&orect, 0, 0, theWidth, theHeight);
				theDepth = 8;
				bytesPerRow = 32;
				theSize = theHeight*bytesPerRow;
				break;
			case 4:
				theRsrcType = small1BitMask;
				theWidth = 16;
				theHeight = 16;
				SetRect(&orect, 0, 0, theWidth, theHeight);
				theDepth = 1;
				bytesPerRow = 2;
				theSize = theHeight*bytesPerRow*2;	// x2 'cause icon+mask
				break;
			case 5:
				theRsrcType = small4BitData;
				theWidth = 16;
				theHeight = 16;
				SetRect(&orect, 0, 0, theWidth, theHeight);
				theDepth = 4;
				bytesPerRow = 8;
				theSize = theHeight*bytesPerRow;
				break;
			case 6:
				theRsrcType = small8BitData;
				theWidth = 16;
				theHeight = 16;
				SetRect(&orect, 0, 0, theWidth, theHeight);
				theDepth = 8;
				bytesPerRow = 16;
				theSize = theHeight*bytesPerRow;
				break;
			} // switch
	
			anError = NewGWorld(&iconGWorld, theDepth, &orect, (CTabHandle)NULL, (GDHandle)NULL, (GWorldFlags)0);
			if (!anError)
			{
				SetGWorld(iconGWorld, (GDHandle)NULL);
				LockPixels(iconGWorld->portPixMap);
				// put the image into the destination (iconWorld)
				CopyBits((BitMap *)*imageWorld->portPixMap,
						(BitMap *)*iconGWorld->portPixMap,
						&irect, &orect,
						ditherCopy, NULL);

				// Draw border and folded document corner on iconWorld
				if (!anError)
				{
					// outer border
					FrameRect(&orect);
					// upper folded corner
					SetRect(&maxRect,((theWidth*3)/4)+1,-1,theWidth+1,(theHeight>>2)-1);
					// erase upper corner to white
					EraseRect(&maxRect);
					// draw lower folded corner
					FrameRect(&maxRect);
					// draw diagonal line
					MoveTo(((theWidth*3)/4)+1, 0);
					LineTo(theWidth, (theHeight>>2)-1);
				}

				if ((destIconRsrc=NewHandle(theSize)) != NULL)
				{
					HLock(destIconRsrc);
					rowBytes = (*iconGWorld->portPixMap)->rowBytes & 0x3fff;
					srcPixPtr = GetPixBaseAddr(iconGWorld->portPixMap);
					tempDestPixPtr = StripAddress(*destIconRsrc);
					mmumode = true32b;
					SwapMMUMode(&mmumode);	// into 32 bit mode
					for (i=0; i < orect.bottom; i++)
					{
						tempSrcPixPtr = srcPixPtr;
						for (j=0; j < bytesPerRow; j++) 
							*tempDestPixPtr++ = *tempSrcPixPtr++;
						srcPixPtr += rowBytes;
					}
					// in 1 bit depth, fill the icon mask too
					if (theDepth == 1)
					{
						if (bytesPerRow <= 2)
							theCorner = 0xfC;	// small corner (1111 1100)
						else
							theCorner = 0xC0;	// regular corner (1100 0000)
						for (i=0; i < orect.bottom; i++)
						{
							for (j=0; j < (bytesPerRow-1); j++) 
								*tempDestPixPtr++ = 0xff;
							*tempDestPixPtr++ = theCorner;
							theCorner >>= 1; // roll the corner mask over slowly
						}
					}
					SwapMMUMode(&mmumode);	// restore original mode

					// as long as there are old icon resources, delete them..
					do {
						tmpHandle = Get1Resource(theRsrcType, kCustomIconResource);
						if (tmpHandle != NULL)
						{
							RmveResource(tmpHandle);
							DisposeHandle(tmpHandle);
						}
					} while (tmpHandle != NULL);

					HUnlock(destIconRsrc);
					AddResource(destIconRsrc, theRsrcType, kCustomIconResource, NULL);
					anError = ResError();
					if (!anError)
					{
						WriteResource(destIconRsrc);
// huh?  What's this here for?  Delete this line when sure it's incorrect...
//						ReleaseResource(destIconRsrc);
					}
				}

				if (iconGWorld)
				{
					DisposeGWorld(iconGWorld);
					iconGWorld = NULL;
				}

			}
		} // for

		// Turn on the FinderInfo flag to display the custom icon suite
		if (!anError)
		{
			UpdateResFile(fileRef);
			anError = ResError();
			if (!anError)
				anError = FSpGetFInfo(fsFile,&finfo);
			if (!anError)
			{
				finfo.fdFlags |= 1<<10;			// turn on hasCustomIcon flag
				finfo.fdFlags &= ~(1<<8);		// turn off hasBeenInited flag
				FSpSetFInfo(fsFile, &finfo);
			}
		}

		// close our resource file now
		if (fileRef != kRsrcFileClosed)
			CloseResFile(fileRef);
	} // if opened ok

	UseResFile(oldResFile);

	if (iconGWorld)
		DisposeGWorld(iconGWorld);
	if (imageWorld)
		DisposeGWorld(imageWorld);
	
	return anError;

} // AppendFinderIcons2PictF



/*
******************************************************************************
Name:
	CompressPictF
------------------------------------------------------------------------------
Purpose:
	** What it does
------------------------------------------------------------------------------
Description:
	** How it does it
------------------------------------------------------------------------------
Parameters:
------------------------------------------------------------------------------
When Used:
	** who calls me and why/when
******************************************************************************
*/
OSErr CompressPictF(ComponentInstance ci, FSSpec *theImageFile)
{
	OSErr				anError;
	short				theFileRefNum;
//	ProgressProcRecord	*progP,progressRec;

	/* Set up a progress dialog for display during compression */
// later..
//	progressRec.progressProc = Progress;
//	progressRec.progressRefCon = 0;
//	progP = &progressRec;

	/* open the picture file, prepare to compress in place */
	anError = FSpOpenDF(theImageFile, fsRdWrPerm, &theFileRefNum);

	/* now compress the picture */
	if (!anError)
	{
		/* Call QuickTime to do its magic.. */
		anError = SCCompressPictureFile(
			ci,
			theFileRefNum,				/* fileRef of source picture file */
			theFileRefNum);				/* fileRef of destination picture file (in place if same as src) */

		FSClose(theFileRefNum);
	}

	/* force disk update */
//	FlushVol(NULL, theImageFile->vRefNum); -- 5/26/93 [esp] commented out, not needed?

	return(anError);
} // CompressPictF
	
	