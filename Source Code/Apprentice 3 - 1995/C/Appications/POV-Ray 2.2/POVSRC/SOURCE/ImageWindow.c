/*==============================================================================
Project:	POV-Ray

Version:	2.2

File:	ImageWindow.c

Description:
	This file contains the Macintosh Image window routines for POV-Ray.
------------------------------------------------------------------------------
Authors:
	Jim Nitchals, David Harr, Eduard [esp] Schwan
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
	930610	[esp]	Created
	930610	[esp]	Added Size2Window code
	930620	[esp]	Changed brighten/darken image slightly to fix saturation bug
	930710	[esp]	Added WindowValid check
	930903	[esp]	Major bug fixes to Virtual Image Buffer code
	931001	[esp]	version 2.0 finished (Released on 10/4/93)
	931119	[djh]	2.0.1 source conditionally compiles for PPC machines, keyword __powerc
==============================================================================*/

/*==== our header ====*/
#include "ImageWindow.h"


/*==== defs ====*/

// temp virtual image file file
#define	POVRAY_IMAGETEMP_FNAME	"POV-Ray.vib"

// Windows
#define	kWindID_Image			1001	// image window


#define	PICTF_HEADER_SIZE		512		// old MacDraw PICTF header

/* Pict file header structure (after the 512 byte header) */
typedef struct
{
	short		picSize;		// low word of size
	Rect		picFrame;		// picture bounds
} PictFHeader, *PictFHeaderPtr;


/*==== globals ====*/

WindowPtr		gImageWindowPtr = NULL;		// the image window
Boolean			gImageWindIsValid = false;	// true if valid stuff in window
// Str63			gImageWindName;				// image window's name
Boolean			use_custom_palette = false;	// does the user desire to use color q?
int				gColorQuantMethod = -1;		// what kind of color quantization?
Boolean			gDoingVirtualFile = false;	// true if doing virtual image buffering

#if defined(__powerc)
extern RoutineDescriptor gPutPICTRD;
#endif

/*==== globals (local) ====*/

/* image window stuff */
static PixMapHandle	gOffScreenPixMapH = NULL;
static Ptr			gImagePixmapPtr = NULL;
static CTabHandle	gCTabHdl = NULL;

static PaletteHandle	gCustomPalette = NULL;
static PaletteHandle	gSystemPalette = NULL;

// used by Paint_to_Picture() and MyPutPicProc()
PicHandle			gMyPicHandle;
long				gMyPicSize;

static FILE			*gImagePictFile = NULL; // used to save image to PICT
static Rect			gImageBounds,		// source (offscreen image) bounds
					gDisplayBounds;		// destination image bounds
static short		gImageWidth,
					gImageHeight;
static int			gLastYPos = 0;		// last vertical position during previous refresh
static short		gCurrYPos;
static long			gPixMapBuffSize;
static long			gDitherTicks;		// # of ticks to wait between image refresh
static long			gRefreshTick = 0;	// Time of last Quickdraw dither of graphic port

/* Undo stuff */
static Ptr			gImageUndoBuffer = NULL;
static Ptr			gImageUndoBuffer2 = NULL;
static Ptr			gImageRevertBuffer = NULL;

/* virtual image buffer globals */
static FILE			*gVirtualImageFile = NULL;
static short		virtual_pixHeight;
static short		virtual_buffer_dirty;
static short		virtual_currentSegment = -1;
static short		virtual_minY;
static short		virtual_maxY;
static long			virtual_segSize = SWAP_SIZE;


// ==============================================
void InitImageWindow(void)
{
	// allocate the image window (it is shown/hidden later)
	gImageWindowPtr = GetNewCWindow(kWindID_Image, NULL, (WindowPtr)NULL);
} // InitImageWindow


// ==============================================
void KillImageWindow(void)
{
	if (gImageWindowPtr)
		DisposeWindow(gImageWindowPtr);
	gImageWindowPtr = NULL;
} // KillImageWindow



// ==============================================
// Create system/custom palette variables for use by image window
void SetupPalettes(void)
{
	int		k;

	// Get default System palette for non-custom mode, (IM VI, Pg. 20-17)
	gSystemPalette = NewPalette(256, NULL, pmTolerant, 0x0000);
	if (gSystemPalette)
	{
		CopyPalette(GetPalette((WindowPtr)-1), gSystemPalette, 0, 0, 256);
		for (k=0; k<256; k++)
			SetEntryUsage(gSystemPalette, k, pmTolerant, 0x0000);
	}

	// allocate a palette for the Image window, fill to default system palette
	gCustomPalette = NewPalette(256, NULL, pmTolerant, 0x0000); // exact match only
	if (gCustomPalette)
	{
		CopyPalette(gSystemPalette, gCustomPalette, 0, 0, 256);
		for (k=0; k<256; k++)
			SetEntryUsage(gCustomPalette, k, pmTolerant, 0x0000);
	}
} // SetupPalettes


// ==============================================
// Build offscreen pixmap
void SetupOffscreen(void)
{
	// Jim, since we REQUIRE 32 bit Color QuickDraw, we should change this
	// to use the much easier and better supported 32bQD call NewGWorld()? [esp]
	gCTabHdl				= (CTabHandle) NewHandle(sizeof(ColorTable));
	(**gCTabHdl).ctSeed		= 24;
	(**gCTabHdl).ctFlags	= 0;
	(**gCTabHdl).ctSize		= 0;
	gOffScreenPixMapH		= (PixMapHandle) NewHandle(sizeof(PixMap));
	if (gOffScreenPixMapH != NULL)
	{
		(**gOffScreenPixMapH).pmVersion		= 0;
		(**gOffScreenPixMapH).packType		= 0;
		(**gOffScreenPixMapH).packSize		= 0;
		(**gOffScreenPixMapH).hRes			= 0x480000;	// 72.0 x 72.0 dpi
		(**gOffScreenPixMapH).vRes			= 0x480000;
		(**gOffScreenPixMapH).pixelType		= RGBDirect;
		(**gOffScreenPixMapH).pixelSize		= 32;
		(**gOffScreenPixMapH).cmpCount		= 3;
		(**gOffScreenPixMapH).cmpSize		= 8;
		(**gOffScreenPixMapH).planeBytes	= 0;
		(**gOffScreenPixMapH).pmTable		= gCTabHdl;
		(**gOffScreenPixMapH).pmReserved	= 0;
		(**gOffScreenPixMapH).baseAddr		= NULL;
	}
	
	if (gOffScreenPixMapH == NULL)
	{
		// fatal error
		displayDialog(kdlog_GenericFatalErr, "Cannot allocate memory for PixMap",
					MemError(), ewcDoCentering, eMainDevice);
		exit_handler();
	}
} // SetupOffscreen


// ==============================================
// Build offscreen pixmap
void KillOffscreen(void)
{
	if (gCTabHdl)
		DisposeHandle((Handle)gCTabHdl);
	if (gOffScreenPixMapH)
		DisposeHandle((Handle)gOffScreenPixMapH);
	gCTabHdl = NULL;
	gOffScreenPixMapH = NULL;
} // KillOffscreen


// ==============================================
// Close the image window
void CloseImageWindow()
{
	if (gImageWindowPtr)
		HideWindow(gImageWindowPtr);
	(**gFilePrefs_h).imageMagFactor = viewmn_hidden;
} // CloseImageWindow


// ==============================================
// Force the entire image window to be updated
void InvalRect_ImageWindow(Boolean DoWholeWindow)
{
	int		magFactor;
	Rect	myInvalRect;

	if (gImageWindowPtr && ((WindowPeek)gImageWindowPtr)->visible)
	{
		SetPort(gImageWindowPtr);

		myInvalRect = gImageWindowPtr->portRect;
		ClipRect(&myInvalRect);

		// if doing partial window in a regular magnification mode...
		if (!DoWholeWindow && ((**gPrefs2Use_h).imageMagFactor != viewmn_Size2Window))
		{
			magFactor = (**gPrefs2Use_h).imageMagFactor-viewmn_normal+1; // 1,2,3,4
			myInvalRect.top = magFactor * gLastYPos;
			myInvalRect.bottom = magFactor * (gCurrYPos + 1);
		}

		InvalRect(&myInvalRect);
	}
} // InvalRect_ImageWindow


// ==============================================
// Display the image window grow box
static void DrawImageGrowBox(void)
{
	int			looper;
	Rect		box;
	PenState	pstate;

	// Save current port state
	SetPort(gImageWindowPtr);
	GetPenState(&pstate);

	// find window size
	box = gImageWindowPtr->portRect;

	// make boxes in lower left corner
	box.top = box.bottom-7;
	box.left = box.right-7;
	OffsetRect(&box, -1, -1);

	// do two overlapping boxes
	for (looper=0; looper<2; looper++)
	{
		// draw white box
		ForeColor(whiteColor);
		FrameRect(&box);

		// draw black box
		OffsetRect(&box, -1, -1);
		ForeColor(blackColor);
		FrameRect(&box);

		// move up & draw 2nd smaller box
		OffsetRect(&box, -2, -2);
		box.right	-= 1;
		box.bottom	-= 1;
	}

	// restore world
	SetPenState(&pstate);
} // DrawImageGrowBox


// ==============================================
void DoResizeImageWindow(WindowPtr w, short h, short v)
{
//	Rect	oldHorizBar;
	Rect 	r;
	
	SetPort(w);

//	oldHorizBar = w->portRect;
//	oldHorizBar.top = oldHorizBar.bottom - (SBarWidth+1);

	SizeWindow(w, h, v, false);

//	EraseRect(&oldHorizBar);
	
//	MoveControl(gSrcWind_VScroll, w->portRect.right - SBarWidth, w->portRect.top-1);
//	SizeControl(gSrcWind_VScroll, SBarWidth+1, w->portRect.bottom - w->portRect.top-(SBarWidth-2));
//	r = (**gSrcWind_VScroll).contrlRect;
//	ValidRect(&r);

	GetGlobalWindowRect(w, &r);
	(**gFilePrefs_h).imageWind_pos = r;
	if ((**gPrefs2Use_h).imageMagFactor == viewmn_Size2Window)
		gDisplayBounds = w->portRect;

} // DoResizeImageWindow


// ==============================================
void DoGrowImageWindow(WindowPtr w, Point p)
{
	GrafPtr		savePort;
	long		theResult;
	Rect		r;
	
	GetPort(&savePort);
	SetPort(w);

	GetMaxGrowRect(w, &r);	
	theResult = GrowWindow(w, p, &r);
	if (theResult != 0)
	{
		DoResizeImageWindow(w, LoWord(theResult), HiWord(theResult));
		InvalRect_ImageWindow(true);
	}

	SetPort(savePort);
} // DoGrowImageWindow


// ==============================================
void UpdateImageWindow(void)
{
	if (gImageWindowPtr)
	{
		SetPort(gImageWindowPtr);
		BeginUpdate(gImageWindowPtr);
		DrawImageWindow(false);
		EndUpdate(gImageWindowPtr);
	}
} // UpdateImageWindow


// ==============================================
void SetImageWindowMag(short magMenuItem)
{
	int			magFactor;
	Rect		r,screenRect;
	GDHandle	theGD;

	if (magMenuItem == viewmn_hidden)
		HideWindow(gImageWindowPtr);
	else
	{
		if (magMenuItem == viewmn_Size2Window)
		{ // set image buffer to 1xsize, leave window size alone
			gDisplayBounds = gImageWindowPtr->portRect;
		}
		else
		{ // set image buffer rect to N*Size, resize window
			magFactor = magMenuItem-viewmn_normal+1; // 1,2,3,4
			// get new zero-based rect
			SetRect(&gDisplayBounds, 0, 0,
						gImageWidth * magFactor,
						gImageHeight * magFactor);
			// convert to potential global window rect
			GetGlobalWindowRect(gImageWindowPtr, &r);
			r.right = r.left + gDisplayBounds.right;
			r.bottom = r.top + gDisplayBounds.bottom;
			// move it to appropriate screen if needed
			ForceRectOnScreen(&r);
// [esp] hmm, shouldn't this next useful chunk of code be moved into ScreenUtils.c?
			// clip any overhang off bottom/right
			theGD = GetClosestGDevice(&r);
			screenRect = (**theGD).gdRect;
			if (r.right > screenRect.right)
				r.right = screenRect.right;
			if (r.bottom > screenRect.bottom)
				r.bottom = screenRect.bottom;
			MoveWindow(gImageWindowPtr, r.left, r.top, false);
			DoResizeImageWindow(gImageWindowPtr,
						r.right-r.left,
						r.bottom-r.top);
// [esp] need code here to handle scroll bars... oh yah, we need scroll bars first!
		}

		// if window is valid, show it now.  This check lets the user
		// set the window size even when there's no valid window content,
		// and doesn't actually show the window until it is valid.
		if (gImageWindIsValid)
		{
			ShowWindow(gImageWindowPtr);
			SelectWindow(gImageWindowPtr);
			InvalRect_ImageWindow(true);
		}
	} // else visible
} // SetImageWindowMag


// ==============================================
// Create undo buffers for Image window changes
void make_undo(void)
{
	if (gImageUndoBuffer == 0)
	{
		gImageUndoBuffer = NewPtr (gPixMapBuffSize);	/* room for image */
		gImageUndoBuffer2 = NewPtr (gPixMapBuffSize);	/* room for image */
		gImageRevertBuffer = NewPtr (gPixMapBuffSize);
		if ( (gImageRevertBuffer) && (gImagePixmapPtr))
			memcpy(gImageRevertBuffer, gImagePixmapPtr, gPixMapBuffSize);
	}

	/* if any of the memory allocations failed, or available mem is low, */
	/* fail the whole undo system */
	if ( (gImageUndoBuffer == NULL) || (gImageUndoBuffer2 == NULL) || (gImageRevertBuffer == NULL)
	|| (FreeMem() < 50000L) )
	{
		if (gImageUndoBuffer)
			DisposePtr (gImageUndoBuffer);
		if (gImageUndoBuffer2)
			DisposePtr (gImageUndoBuffer2);
		if (gImageRevertBuffer)
			DisposePtr (gImageRevertBuffer);
		gImageUndoBuffer = NULL;
		gImageUndoBuffer2 = NULL;
		gImageRevertBuffer = NULL;
	}

	if ((gImageUndoBuffer) && (gImagePixmapPtr))
	{
		memcpy(gImageUndoBuffer, gImagePixmapPtr, gPixMapBuffSize);
		gCanUndo = TRUE;
	}
} // make_undo


// ==============================================
// Actually do the undo operation on the image window
void undo_image(void)
{
	if ((gImageUndoBuffer) && (gCanUndo) && (gImagePixmapPtr))
	{
		memcpy(gImageUndoBuffer2, gImagePixmapPtr, gPixMapBuffSize);
		memcpy(gImagePixmapPtr, gImageUndoBuffer, gPixMapBuffSize);
		memcpy(gImageUndoBuffer, gImageUndoBuffer2, gPixMapBuffSize);
		InvalRect_ImageWindow(true);
	}
} // undo_image


// ==============================================
// restore the original image to the image window
void revert_image(void)
{
	if ((gImageRevertBuffer) && (gImagePixmapPtr))
	{
		memcpy(gImageUndoBuffer, gImagePixmapPtr, gPixMapBuffSize);
		memcpy(gImagePixmapPtr, gImageRevertBuffer, gPixMapBuffSize);
		gCanUndo = TRUE;
		InvalRect_ImageWindow(true);
	}
} // revert_image


// ==============================================
// return the value "v", insuring it is between 0 and 255
static short ClipToByteRange(short v)
{
	if ( v < 0)
		return (0);
	if (v > 255)
		return (255);
	return (v);
} // ClipToByteRange


// ==============================================
// Darken every pixel value in image window by 7/8ths original
void darken_image(void)
{
	unsigned char *pptr;
	short		v;
	long		i,j;
	
	make_undo();
	if (gImagePixmapPtr == 0) return;

	for (i=0; i < gImageHeight; i++)
	{
		for (j=0; j < gImageWidth; j++)
		{
			pptr = (unsigned char *) 
					(gImagePixmapPtr + 4L * ((gImageWidth * i) + j));
			*pptr = 0;
			pptr++;
			v = (*pptr * 7 / 8)-1;
			*pptr = ClipToByteRange (v);
			pptr++;
			v = (*pptr * 7 / 8)-1;
			*pptr = ClipToByteRange (v);
			pptr++;
			v = (*pptr * 7 / 8)-1;
			*pptr = ClipToByteRange (v);
			pptr++;
		}
	}
	InvalRect_ImageWindow(true);
} // darken_image


// ==============================================
// Lighten every pixel value in image window by 8/7ths original
void lighten_image(void)
{
	unsigned char *pptr;
	long i,j;
	short v;

	make_undo();
	if (gImagePixmapPtr == 0) return;

	for (i=0; i < gImageHeight; i++)
	{
		for (j=0; j < gImageWidth; j++)
		{
			pptr = (unsigned char *) 
					(gImagePixmapPtr + 4L * ((gImageWidth * i) + j));
			*pptr = 0;
			pptr++;
			v = (*pptr * 8 / 7)+1;
			*pptr = ClipToByteRange (v);
			pptr++;
			v = (*pptr * 8 / 7)+1;
			*pptr = ClipToByteRange (v);
			pptr++;
			v = (*pptr * 8 / 7)+1;
			*pptr = ClipToByteRange (v);
			pptr++;
		}
	}
	InvalRect_ImageWindow(true);
} // lighten_image


// ==============================================
// Invert every pixel value in image window
void invert_image(void)
{
	unsigned char *pptr;
	long i,j;
	
	make_undo();
	if (gImagePixmapPtr == 0) return;

	for (i=0; i < gImageHeight; i++)
	{
		for (j=0; j < gImageWidth; j++)
		{
			pptr = (unsigned char *) 
					(gImagePixmapPtr + 4L * ((gImageWidth * i) + j));
			*pptr = 0;
			pptr++;
			*pptr = 255 - *pptr;
			pptr++;
			*pptr = 255 - *pptr;
			pptr++;
			*pptr = 255 - *pptr;
			pptr++;
		}
	}
	InvalRect_ImageWindow(true);
} // invert_image


// ==============================================
// Reduce the contrast of every pixel value in image window a bit
void reduce_contrast(void)
{
	unsigned char *pptr;
	long i,j;
	short v;
	
	make_undo();
	if (gImagePixmapPtr == 0) return;

	for (i=0; i < gImageHeight; i++)
	{
		for (j=0; j < gImageWidth; j++)
		{
			pptr = (unsigned char *) 
					(gImagePixmapPtr + 4L * ((gImageWidth * i) + j));
			*pptr++ = 0;

			v = *pptr;
			v = v - ((v - 128) >> 2);
			*pptr++ = v;

			v = *pptr;
			v = v - ((v - 128) >> 2);
			*pptr++ = v;

			v = *pptr;
			v = v - ((v - 128) >> 2);
			*pptr++ = v;
		}
	}
	InvalRect_ImageWindow(true);
} // reduce_contrast


// ==============================================
// Increase the contrast of every pixel value in image window a bit
void increase_contrast(void)
{
	unsigned char *pptr;
	long i,j;
	short v;

	make_undo();
	if (gImagePixmapPtr == 0) return;

	for (i=0; i < gImageHeight; i++)
	{
		for (j=0; j < gImageWidth; j++)
		{
			pptr = (unsigned char *) 
					(gImagePixmapPtr + 4L * ((gImageWidth * i) + j));
			*pptr++ = 0;

			v = *pptr;
			v = v + ((v - 128) >> 2);
			*pptr++ = ClipToByteRange(v);

			v = *pptr;
			v = v + ((v - 128) >> 2);
			*pptr++ = ClipToByteRange(v);

			v = *pptr;
			v = v + ((v - 128) >> 2);
			*pptr++ = ClipToByteRange(v);
		}
	}
	InvalRect_ImageWindow(true);
} // increase_contrast


// ==============================================
// draw a 1 pixel black border around the edge of the image
void draw_border(void)
{
	unsigned char *pptr;
	long i;
	
	make_undo();
	if (gImagePixmapPtr == 0) return;

	for (i=0; i < gImageHeight; i++)
	{
/* left border edge */
			pptr = (unsigned char *) 
					(gImagePixmapPtr + (gImageWidth * 4 * i) );
			*pptr++ = 0;
			*pptr++ = 0;
			*pptr++ = 0;
			*pptr++ = 0;

/* right border edge */
			pptr = (unsigned char *) 
					(gImagePixmapPtr + (gImageWidth * 4 * i) + ((gImageWidth-1) * 4) );
			*pptr++ = 0;
			*pptr++ = 0;
			*pptr++ = 0;
			*pptr++ = 0;
	}
	for (i=0; i < gImageWidth; i++)
	{
/* top border edge */
			pptr = (unsigned char *) 
					(gImagePixmapPtr + (4 * i));
			*pptr++ = 0;
			*pptr++ = 0;
			*pptr++ = 0;
			*pptr++ = 0;

/* bottom border edge */
			pptr = (unsigned char *) 
					( gImagePixmapPtr + 4L*gImageWidth*(gImageHeight-1) + 4*i );
			*pptr++ = 0;
			*pptr++ = 0;
			*pptr++ = 0;
			*pptr++ = 0;

	}
	InvalRect_ImageWindow(true);
} // draw_border


// ==============================================
// Create and open the virtual image buffer file for large images
void open_virtual(void)
{
	int err;

	delete_virtual();
	if (!gDoingVirtualFile)
	{
		gVirtualImageFile = fopen(POVRAY_IMAGETEMP_FNAME,"wb+");
		err = ferror(gVirtualImageFile);
		if (!gVirtualImageFile)
			printf("## Error #%d opening file '%s'!\n", err, POVRAY_IMAGETEMP_FNAME);
	}
	virtual_currentSegment = -1;
	virtual_buffer_dirty = false;
	virtual_minY = 32767;
	virtual_maxY = 0;
	virtual_pixHeight = virtual_segSize / (gImageWidth * 4L);
	gDoingVirtualFile = true;
} // open_virtual


// ==============================================
// Close and delete the virtual image buffer file
void delete_virtual(void)
{
	int k;
	if (gDoingVirtualFile)
	{
		fclose (gVirtualImageFile);
		gVirtualImageFile = NULL;
	}
	k = remove(POVRAY_IMAGETEMP_FNAME);
	gDoingVirtualFile = false;
} // delete_virtual


// ==============================================
// Dispose of virtual image buffer memory, and close/delete the file
void dispose_virtual(void)
{
	Handle h;

	if (gImagePixmapPtr)
	{
		h = RecoverHandle(gImagePixmapPtr);
		HUnlock(h);
		DisposHandle(h);
		gImagePixmapPtr = NULL;
		gCanUndo = 0;
	}

	if (gImageUndoBuffer)
		DisposePtr(gImageUndoBuffer);
	if (gImageUndoBuffer2)
		DisposePtr(gImageUndoBuffer2);
	if (gImageRevertBuffer)
		DisposePtr(gImageRevertBuffer);
	gImageUndoBuffer = NULL;
	gImageUndoBuffer2 = NULL;
	gImageRevertBuffer = NULL;

	delete_virtual();
	virtual_minY = 32767;
} // dispose_virtual


// ==============================================
// swap in the "y"th virtual image buffer segment from disk
void swap_virtual_segment (short y)
{
	int i;
	int err;
		
	// is Y outside currently loaded segment? need to load new one
	if (gDoingVirtualFile && ((y < virtual_minY) || (y >= virtual_maxY)) )
	{
		// is current segment dirty? save to disk first if so
		if (virtual_buffer_dirty && (virtual_currentSegment >= 0))
		{
			fseek(gVirtualImageFile, (virtual_currentSegment * virtual_segSize), SEEK_SET);
			if ( fwrite((char *) gImagePixmapPtr, virtual_segSize, 1, gVirtualImageFile) != 1)
			{	// error
				err = ferror(gVirtualImageFile);
				if (err)
					printf("## Error #%d writing position #%ld in file '%s'!\n",
							err, (virtual_currentSegment * virtual_segSize),
							POVRAY_IMAGETEMP_FNAME);
				dispose_virtual();
			}
		}

		// Now calculate and read needed segment into memory
		if (gDoingVirtualFile)
		{
			// calculate new current segment stuff
			virtual_currentSegment = (int) (y / virtual_pixHeight);
			virtual_minY = virtual_currentSegment * virtual_pixHeight;
			virtual_maxY = (virtual_currentSegment + 1) * virtual_pixHeight;

			// read it in off disk
			fseek(gVirtualImageFile, (virtual_currentSegment * virtual_segSize), SEEK_SET);
			i = fread((char *) gImagePixmapPtr, virtual_segSize, 1, gVirtualImageFile);
			err = ferror(gVirtualImageFile);
			if (err)
				printf("## Error #%d reading position #%ld in file '%s'!\n",
						err, (virtual_currentSegment * virtual_segSize),
						POVRAY_IMAGETEMP_FNAME);
			virtual_buffer_dirty = false;
		}

		// error.. couldn't keep swap file open
		if (!gDoingVirtualFile)
			(void)displayDialog(139, NULL, 0, ewcDoCentering, eSameAsPassedWindow);
	}

} // swap_virtual_segment


// ==============================================
// Initialize the display, set up buffers (VIB if needed)
void display_init(int width, int height)
{
	long		*p, *q, pmsize, i;
	Handle		h;
	short		error_dlogID = 0;
	DialogPtr	progressDialogPtr = NULL;

	if ((**gPrefs2Use_h).progress >= kProgMinimal)
	{
		printf("-- [Memory]  FreeMemory=%ldK\n", FreeMem()/1024L);
	}

	SetCursor(&gWaitCursor); // could take a little while..

	gImageWidth = width;
	gImageHeight = height;
	gPixMapBuffSize = 4L * (long) width * (long) height;
	gCurrYPos = 0;
	gLastYPos = 0;

	SetRect(&gImageBounds, 0, 0, width, height);
	pmsize = gPixMapBuffSize;

	virtual_buffer_dirty = false;

	h = NULL;
	// if there is enough room for a pixmap, plus a little free space (100KB),
	// then go ahead and allocate it..
	if	(FreeMem() > pmsize + 100000L)
		h = NewHandleClear(pmsize);

	// ..otherwise play virtual buffer games..
	if (h == NULL)
	{
		// can't allocate, do it in segments, set up progress dialog
		progressDialogPtr = GetNewProgressDialog(153, 3);
		if (progressDialogPtr)
		{
			PositionWindow(progressDialogPtr, ewcDoCentering, eSameAsPassedWindow, (WindowPtr)gp2wWindow);
			ShowWindow(progressDialogPtr);
			SelectWindow(progressDialogPtr);
			DrawDialog(progressDialogPtr);
			// make sure events get through so dlog is updated
			Cooperate(true);
		}
	}

	// let user know some info
	if ((**gPrefs2Use_h).progress >= kProgMinimal)
	{
		if ( (h == NULL) && ((**gPrefs2Use_h).progress >= kProgDebug) )
			printf("-d VIBNumSegments=%d, VIBSegmentSize=%dK\n",
					(int)1+(gImageHeight / (virtual_segSize / (gImageWidth * 4L))),
					(int)(virtual_segSize/1024L));
		printf("-- ImageBufferPixWidth=%d, ImageBufferPixHeight=%d, ImageBufferSize=%ldK\n",
			gImageWidth, gImageHeight, pmsize/1024L);
	}

	if (h)
	{ // got whole thing, initialize it
		MoveHHi(h);
		HLock(h);
		gImagePixmapPtr = *h;
		(*gOffScreenPixMapH)->bounds = gImageBounds;
		(*gOffScreenPixMapH)->rowBytes = (width * 4) | 0x8000;
		(*gOffScreenPixMapH)->baseAddr = gImagePixmapPtr;
		// fill to white
		q = (long *) ((long) gImagePixmapPtr + pmsize);
		for (p = (long *) gImagePixmapPtr; p < q; *p++ = -1L)
			;
		gDitherTicks = 20*60;	/* 20 seconds between re-dithers */
	}
	else
	{ // set up the virtual buffer

		open_virtual();
		SetRect(&gImageBounds, 0, 0, width, virtual_pixHeight);

		if (gDoingVirtualFile)
			h = NewHandleClear(virtual_segSize);

		if (!h)
		{
			dispose_virtual();
			error_dlogID = 133; // no display memory
		}
		else
		{
			if (progressDialogPtr)
				updateProgressDialog(progressDialogPtr, 0, gImageHeight-1, 1);
			MoveHHi(h);
			HLock(h);
			gImagePixmapPtr = *h;
			(*gOffScreenPixMapH)->bounds = gImageBounds;
			(*gOffScreenPixMapH)->rowBytes = (width * 4) | 0x8000;
			(*gOffScreenPixMapH)->baseAddr = gImagePixmapPtr;
			// fill to white
			q = (long *) ((long) gImagePixmapPtr + virtual_segSize);
			for (p = (long *) gImagePixmapPtr; p < q; *p++ = -1)
				;
			gDitherTicks = 4L*60L*60L;	/* 4 minutes between re-dithers */

			// We go through this for loop with i stepping down through the image
			// scanlines every 1/2 buffer size.  The only reason for not going
			// every buffer size (virtual_pixHeight) is to make the progress bar
			// a little less jerky, doesn't slow things down much.
			for (i=0; (i<gImageHeight)&&gDoingVirtualFile; i+=(virtual_pixHeight>>1))
			{
				// Update the progress bar
				if (progressDialogPtr)
				{
					Cooperate(true);
					updateProgressDialog(progressDialogPtr, 0, gImageHeight-1, i);
				}
				// force it to write each segment to disk to build the file
				virtual_buffer_dirty = true;
				swap_virtual_segment (i);
			}

			/* swap back in segment 0 */
			if (gDoingVirtualFile)
			{
				if (progressDialogPtr)
					updateProgressDialog(progressDialogPtr, 0, gImageHeight, gImageHeight);
				virtual_buffer_dirty = true;
				swap_virtual_segment(0);
			}

			if (!gDoingVirtualFile)
			{
				error_dlogID = 139; // no disk space for swap file
				dispose_virtual();
			}
		}

		SetCursor(&qd.arrow);

		// get rid of progress dialog
		if (progressDialogPtr)
			disposeProgressDialog(progressDialogPtr);

	} // else virtual

	// display any error messages now
	if (error_dlogID)
		(void)displayDialog(error_dlogID, NULL, 0, ewcDoCentering, eSameAsPassedWindow);
	else
	{
		// image window now has content
		// turn on now, so SetImageWindMag will show window
		gImageWindIsValid = true;

		// size and show the window
		SetImageWindowMag((**gPrefs2Use_h).imageMagFactor);

		if (gHasPictUtils)
			SetCustomPalette(false);
	}

	SetCursor(&qd.arrow); // all done initializing..
} // display_init


// ==============================================
// Plot a single pixel at (x,y)
void display_plot(int x, int y, 
					 unsigned char Red,
					 unsigned char Green,
					 unsigned char Blue)
{
	RGBColor c;
	unsigned char *pptr;
	Rect	magniRect;
	long	pport_y;
	int		mr;
	int		magFactor;

	gCurrYPos = y;

	if (gImagePixmapPtr)
	{
		if (gDoingVirtualFile)
		{
			swap_virtual_segment(y);
			pport_y = y - virtual_minY;		/* Y offset within current virtual segment */
			virtual_buffer_dirty = true;
		}
		else
			pport_y = y;

		// slam the RGB value into the pixmap
		pptr = (unsigned char *) 
				((long) gImagePixmapPtr + 4L * ((long) gImageBounds.right * pport_y + (long) x));
		*pptr++	= 0;
		*pptr++	= Red;
		*pptr++	= Green;
		*pptr	= Blue;
	}

	magFactor = (**gPrefs2Use_h).imageMagFactor;
	if (magFactor != viewmn_hidden)
	{ // it is visible
		SetPort(gImageWindowPtr);
		c.red = ((short)Red)<<8;
		c.green = ((short) Green)<<8;
		c.blue = ((short) Blue)<<8;
		if (magFactor == viewmn_normal)
			SetCPixel(x, y, &c);
		else
		{ // some magnification at play here
			if (magFactor != viewmn_Size2Window)
			{ // go to straight magnification N (don't display
				magFactor = magFactor-viewmn_normal+1; // 2,3,4
				mr = gCurrYPos * magFactor;
				magniRect.top		= mr;
				magniRect.bottom	= mr + magFactor;
				magniRect.left		= x * magFactor;
				magniRect.right		= (x+1) * magFactor;
				RGBForeColor(&c);
				PaintRect(&magniRect);
			}
		}

		// time to update the image?
		if (TICKS > (gRefreshTick + gDitherTicks))
		{
			if (y > (gLastYPos + 1))
			{
				InvalRect_ImageWindow(false);
				Cooperate(true);
				gLastYPos = y;
				gRefreshTick = TICKS;
			}
		}

	}
} // display_plot


// ==============================================
// Do any cleanup needed when the display is complete
void display_finished(void)
{
	// Reset last Y pos so that screen refreshes do the whole screen
	gLastYPos = 0;
} // display_finished


// ==============================================
// Close any files used after display is complete
void display_close(void)
{
} // display_close


// ==============================================
// Find and assign the appropriate custom palette to the image window
void SetCustomPalette(Boolean doScreenUpdate)
{
	PictInfo		plot_info;
	short			k;
	OSErr			anError = noErr;
	PaletteHandle	thePalette = NULL;

	if (gImageWindowPtr)
		SetPort(gImageWindowPtr);

	if (gHasPictUtils && use_custom_palette && gOffScreenPixMapH)
	{
		switch(gColorQuantMethod)
		{
			case systemMethod:
			case popularMethod:
			case medianMethod:
				anError = GetPixMapInfo(gOffScreenPixMapH,
								&plot_info,
								returnPalette + suppressBlackAndWhite,
								256,
								gColorQuantMethod,
								0);

				// copy the new palette entries into custom palette & use it
				if (plot_info.thePalette && gCustomPalette)
				{
					CopyPalette(plot_info.thePalette, gCustomPalette, 0, 0, 256);
					for (k=0; k<256; k++)
						SetEntryUsage(gCustomPalette, k, pmTolerant, 0x0000);
					thePalette = gCustomPalette;
				}
				// done with retrieved palette. toolbox created it, so we must destroy it!
				if (plot_info.thePalette)
					DisposePalette(plot_info.thePalette);
				break;
			otherwise:
				SysBeep(4);
				break;
		}
	}
	else
	{
		// Revert to system palette
		thePalette = gSystemPalette;
	}

	if (gImageWindowPtr)
	{
		// reset all screens, this is a little severe though!
		if (doScreenUpdate)
			RestoreDeviceClut(NULL);
		// tell window to use the new palette
		if (thePalette)
			NSetPalette(gImageWindowPtr, thePalette, pmAllUpdates);
		ActivatePalette(gImageWindowPtr);
	}

} // SetCustomPalette


// ==============================================
// Display the image to the window
void DrawImageWindow(Boolean DoWholeWindow)
{
	BitMap		bmap;
	short 		myMode, i;
	int			magFactor;
	static short counter = 0;
	Rect		my_bounds;
	RGBColor	myRGBWhite = {-1,-1,-1};
	RGBColor	myRGBBlack = {0,0,0};


	SetCursor(&gWaitCursor); // could take a little while..

	magFactor = (**gPrefs2Use_h).imageMagFactor;
	if (magFactor != viewmn_hidden)
	{ // it is visible
		// set these to defaults so copybits behaves predictably
		RGBBackColor(&myRGBWhite);
		RGBForeColor(&myRGBBlack);

		if (DoWholeWindow)
		{
			// clear whole window
			EraseRect(&gImageWindowPtr->portRect);
		}
		else
		{
		}

		// something to display?
		if (gImagePixmapPtr)
		{
			bmap.baseAddr = (Ptr) gOffScreenPixMapH;
			bmap.rowBytes = 0xC000;
			bmap.bounds = gImageBounds;

//			if (gHasPictUtils)
//				SetCustomPalette(false);

			if ((**gPrefs2Use_h).doDither)
				myMode = ditherCopy;
			else
				myMode = srcCopy;

			if (gDoingVirtualFile)
			{
				magFactor = magFactor-viewmn_normal+1; // 1,2,3,4
				for (i=0; i < gImageHeight; i = i + virtual_pixHeight)
				{
					swap_virtual_segment (i);
					my_bounds = gImageBounds;
// <esp> need to fix up for magFactor in virtual mode
					my_bounds.top = (i * magFactor);
					my_bounds.bottom = my_bounds.top + (virtual_pixHeight * magFactor);
					/* don't worry about the bounds, the clipping region
					of the window will stop us from drawing past the bottom */					
					my_bounds.left = 0;
					my_bounds.right = gImageWidth * magFactor;
					CopyBits(&bmap, &gImageWindowPtr->portBits,
									&gImageBounds, &my_bounds,
									myMode, NULL);
				}
			}
			else
			{
				CopyBits(&bmap, &gImageWindowPtr->portBits,
								&gImageBounds, &gDisplayBounds,
								myMode, NULL);
			}

			// Draw our custom size box if in Size-2-Window mode
			if (magFactor == viewmn_Size2Window)
				DrawImageGrowBox();

		} // offscreen exists
 	} // it is visible

	SetCursor(&qd.arrow);

} // DrawImageWindow


// ==============================================
// Write Picture bottleneck routine for paint_to_picture()
pascal void MyPutPicProc( char *dataPtr, short byteCount)
{
	int myByteCount;

	myByteCount = byteCount;
	gMyPicSize += byteCount;
	if (gImagePictFile)
	{
		if (fwrite( dataPtr, 1, myByteCount, gImagePictFile) != myByteCount)
		{
			fclose (gImagePictFile);
			gImagePictFile = 0;
		}
	}
	// we have to keep the picture handle header updated for Quickdraw!
	if (gMyPicHandle)
		(**gMyPicHandle).picSize = gMyPicSize;
} // MyPutPicProc


// ==============================================
// Write the Pixmap out to either a disk file, or the Clipboard
void paint_to_picture(short do_disk_buffer)
{
	BitMap		bmap;
	short 		myMode, i;
	OSErr		anError;
	Rect		my_bounds, myPicRect;	
	CGrafPort	myCGrafPtr;
	GrafPtr		oldGrafPtr;
	PictFHeader	myPictFHeader;
	CQDProcs	myQDProcs;

	anError = noErr;
	if (gImagePixmapPtr)
	{
		SetCursor(&gWaitCursor); // could take a little while..

		GetPort(&oldGrafPtr);
		OpenCPort (&myCGrafPtr);
		SetPort ((GrafPtr)&myCGrafPtr);

		PortSize (gImageWidth, gImageHeight);
		myPicRect.top = 0;
		myPicRect.bottom = gImageHeight;
		myPicRect.left = 0;
		myPicRect.right = gImageWidth;

		ClipRect(&myPicRect);

		if (do_disk_buffer)
		{
			SetStdCProcs ((CQDProcsPtr) &myQDProcs);
#if defined(__powerc)
			myQDProcs.putPicProc = (QDPutPicUPP)&gPutPICTRD;
#else
			myQDProcs.putPicProc = (Ptr) MyPutPicProc;
#endif // __powerc
			myCGrafPtr.grafProcs = (CQDProcsPtr) &myQDProcs;
			// write the PICT header to the file
			gMyPicSize = sizeof(myPictFHeader);
			myPictFHeader.picSize = gMyPicSize; // accumulated later in MyPutPicProc()..
			myPictFHeader.picFrame = myPicRect;
			i = fwrite (&myPictFHeader, 1, sizeof(myPictFHeader), gImagePictFile);
			anError = ferror(gImagePictFile);
		}

		if (!anError)
		{
			/* insure gMyPicHandle null, 'cause MyPutPicProc will be called in OpenPicture! */
			gMyPicHandle = NULL;
			gMyPicHandle = OpenPicture (&myPicRect);
			anError = QDError();
		}

		bmap.baseAddr = (Ptr) gOffScreenPixMapH;
		bmap.rowBytes = 0xC000;	// flag for PixMap
		if ((**gPrefs2Use_h).doDither)
			myMode = ditherCopy;
		else
			myMode = srcCopy;

		if (!anError)
		{
			if (gDoingVirtualFile)
				for (i=0; (i <= gImageHeight) && !anError; i = i + virtual_pixHeight)
				{
					swap_virtual_segment (i);
					my_bounds.top = i;
// NOTE, JIM! (next source line)
// By doing a CopyBits of the last rect in full, the whole rect of pixels _does_
// get written to the PICT file, even though not all the scan lines are valid..
// this doesn't hurt anything, just makes the file a little bigger than it needs to be,
// since the extra lines will get clipped on playback.  However, since gImageBounds is
// global, it would be icky to temporarily change its bottom along with my_bounds so
// as to only write the necessary lines.. maybe later. :-)
					my_bounds.bottom = i + virtual_pixHeight;
					my_bounds.left = 0;
					my_bounds.right = gImageWidth;
					CopyBits(&bmap, (BitMap *)&myCGrafPtr.portPixMap, &gImageBounds, &my_bounds,
							 myMode, NULL);
					anError = QDError();
				}
			else
				{
				CopyBits(&bmap, (BitMap *)&myCGrafPtr.portPixMap, &gImageBounds, &gImageBounds,
						 myMode, NULL);
				anError = QDError();
				}
		}

		/* write end-of-picture */
		if (!anError)
		{
			ClosePicture();
			anError = QDError();
		}

		if (!anError)
		{
			if (do_disk_buffer)
			{
				/* don't call us anymore! */
				SetStdCProcs((CQDProcsPtr) &myQDProcs);

				/* move back to pic header again */
				fflush(gImagePictFile);
				fseek(gImagePictFile, PICTF_HEADER_SIZE, SEEK_SET);
				anError = ferror(gImagePictFile);
				/* update the PICT header in the file (with now-true picSize field..) */
				if (!anError)
				{
					myPictFHeader.picSize = gMyPicSize;
					i = fwrite (&myPictFHeader, 1, sizeof(myPictFHeader), gImagePictFile);
					anError = ferror(gImagePictFile);
				}
			}
			else
			{
				ZeroScrap();
				HLock((Handle) gMyPicHandle);
				gMyPicSize = GetHandleSize((Handle) gMyPicHandle);
				PutScrap (gMyPicSize, 'PICT', (Ptr) *gMyPicHandle);
				gMyPicSize = UnloadScrap();
				HUnlock((Handle) gMyPicHandle);
			}
		} // if !error

		/* close up shop */
		CloseCPort(&myCGrafPtr);
		SetPort(oldGrafPtr);
		if (gMyPicHandle)
			DisposeHandle((Handle) gMyPicHandle);

		/* oh yeah, check for errors */
		if (anError)
		{
			if (gImagePictFile)
			{
				fclose (gImagePictFile);
				gImagePictFile = NULL;
			}
			SysBeep(4);
		}

		SetCursor(&qd.arrow);

	} // if gImagePixmapPtr..
} // paint_to_picture


// ==============================================
// Write the Pixmap as a PICT file
// getName - if true, prompt for output file name
// animSuffix - ignore if <0, if 0-N then use as file name frame suffix for anim
// ci - QuickTime StdCompression component if image is to be compressed
void SaveOutputFile(Boolean getName, int animSuffix, ComponentInstance ci)
{
	int 		k;
	Rect		myPicRect;	
	Point 		where;
	SFReply		reply;
	FInfo		myFileInfo;
	FSSpec		fsFile;		
	char		filler[4];
	char		cFname[256];

	for (k=0; k<4; k++)
		filler[k] = 0;

	// Find and remove the ".POV" suffix
	pStrCopy(gSrcWind_FileName, (StringPtr)cFname);
	p2cstr((StringPtr)cFname);
	k = strlen(cFname);
	if ((k>4) && (cFname[k-4] == '.'))
		cFname[k-4] = '\0';

	// add animation suffix?
	strcat (cFname, ".");
	if (animSuffix == kNoAnimSuffix)
	{
		strcat (cFname, "Pict");
	}
	else
	{
		sprintf(filler, "%03d",animSuffix);
		strcat (cFname, filler);
	}

	if (getName)
	{
		c2pstr(cFname);

		GetBestDialogPos(&where);
		SFPutFile (where, "\pSave PICT file as�", (StringPtr)cFname, NULL, &reply);
	}
	else
	{
		// copy the generated name into SF Reply..
		strcpy ((char *) reply.fName,cFname);
		c2pstr((char *)reply.fName);
		reply.vRefNum = gSrcWind_VRefNum;
		reply.good = TRUE;
	}

	if (reply.good)
	{
		SetCursor(&gWaitCursor);

		fsFile.vRefNum = reply.vRefNum;
		fsFile.parID = 0;
		BlockMove(reply.fName, fsFile.name, 1+reply.fName[0]);

		p2cstr(reply.fName);
		SetVol(NULL, reply.vRefNum);
		gImagePictFile = fopen((char *) reply.fName,"wb");
		if (gImagePictFile)
		{
			for (k=0; k<PICTF_HEADER_SIZE/4; k++)
				fwrite (&filler, 1, 4, gImagePictFile);

			// if not doing a virtual screen (which could take awhile) then
			// let the window update event get through to repaint the image
			// in case the dialog stomped on it.
			if (!gDoingVirtualFile)
			{
				Cooperate(true);
			}

			paint_to_picture(TRUE);

			if (gImagePictFile)
				{
					fclose (gImagePictFile);
					gImagePictFile = NULL;
					c2pstr((char *)reply.fName);

					/* If QuickTime is around & user wants to squish the picture, ask user how to squish it */
					if (gHasImageCompressionMgr && (**gPrefs2Use_h).doCompression)
					{
						myPicRect.top = 0;
						myPicRect.bottom = gImageHeight;
						myPicRect.left = 0;
						myPicRect.right = gImageWidth;
						k = FSMakeFSSpec(reply.vRefNum, 0, reply.fName, &fsFile);

						if (!k)
							k=CompressPictF(ci, &fsFile);
					}

					// set image file to type PICT
					k = GetFInfo(reply.fName, reply.vRefNum, &myFileInfo);
					if (k==0)
					{
						myFileInfo.fdType = 'PICT';
						myFileInfo.fdCreator = (**gPrefs2Use_h).pictFileCreator;
						k = SetFInfo (reply.fName, reply.vRefNum, &myFileInfo);
					}

// not yet tested and working, but leave in for later..
//					// stick a preview resource on it
//					if (wanted..)
//						k = AppendFilePreview2PictF(&fsFile);

					// Add System 7 custom icons of the image itself to the file
					if (gHasImageCompressionMgr)
						AppendFinderIcons2PictF(&fsFile, &myPicRect, eAFI_ShrinkWholeImage);
				}
			else
				// couldn't write PICT file
				(void)displayDialog(140, NULL, 0, ewcDoCentering, eSameAsPassedWindow);
		}
		SetCursor(&qd.arrow);
	}
} // SaveOutputFile
