/* RandomDotWin.c
	by David Phillip Oster October 1994 oster@netcom.com
	for:
	Stuart Inglis singlis@waikato.ac.nz
	Department of Computer Science
	University of Waikato, Hamilton, New Zealand
 */
#include <PictUtil.h>
#include "RandomDotMain.h"
#include "RandomDotRes.h"

#include "RandomDotWin.h"
#include "RandomDotWinCommands.h"

#include "Error.h"
#include "Menu.h"
#include "pgm.h"
#include "RandomDot.h"
#include "Utils.h"
#include "RandomDotScroll.h"

#define round(X)	((int) ((X) + 0.5))



/* private prototypes
 */
static void CopyPicToGWorld(void);
static void RandomDotPalette(void);

static void NoWindowActivateMenus(void);
static void RandomDotActivateMenus(void);


CTabHandle		grayCTab = NIL;
static CTabHandle	shimmerCT;
static PaletteHandle grayPalette = NIL;
static PaletteHandle shimmerPalette = NIL;


/* SetCTGrayRamp - set a Color Table, assumed to be 256 long, to uniform grays,
	where the indices match the complement of the gray value.

	Note: there is a bug in System 7.1 with the LaserWriter driver such that if
		you CopyBits() to the printer, with an 8-bit indexed offscreen gworld
		as the source, the first and last entries of the color table of the
		offscreen gworld get set so the first is white and the last is black.
		So, we follow that scheme in this program: for an 8-bit pixmap index,
		the gray scale value is the complement of the index.
		Watch out, expressions evaluate to ints in C, so ~(unsigned char) 1
		if 0xFFFE not 0xFE
 */
static void SetCTGrayRamp(CTabHandle ct, Integer start, Integer stop){
	LongInt		seed;
	Integer		i;
	unsigned short j;
	ColorSpec	*sp;

	seed = GetCTSeed();
	(**ct).ctSeed = seed;
	sp = (**ct).ctTable;
	for(i = start ; i <= stop ; i++, sp++){
		j = ~i & 0xFF;
		sp->rgb.red = sp->rgb.green = sp->rgb.blue = j << 8 | j;
		sp->value = i;
	}
}


/* InitRandomDot - one time initializations. 
	create a colortable that maps 0..255 into the matching shades of gray.
	The palette manager wants to autmatically add white and black, so we
	create a CTabHandle for palettes that is the same as grayCTab, but excludes
	white and black.
 */
OSErr InitRandomDot(void){
	LongInt		seed;
	OSErr		errCode;
	CTabHandle paletteCTab;

	grayCTab = (CTabHandle) NewHandle(sizeof(long) + sizeof(short) + sizeof(short) + 256L*sizeof(ColorSpec));
	if(noErr == (errCode = MemError())){
		(**grayCTab).ctFlags = 0;
		(**grayCTab).ctSize = 255;
		SetCTGrayRamp(grayCTab, 0, 255);
	}
	if(noErr == errCode){
		paletteCTab = (CTabHandle) NewHandle(sizeof(long) + sizeof(short) + sizeof(short) + 254L*sizeof(ColorSpec));
		if(noErr == (errCode = MemError())){
			(**paletteCTab).ctFlags = 0;
			(**paletteCTab).ctSize = 253;
			SetCTGrayRamp(paletteCTab, 1, 254);
			grayPalette = NewPalette(254, paletteCTab, pmTolerant, 0);
			shimmerPalette = NewPalette(254, paletteCTab, pmAnimated, 0);
			DisposeHandle((Handle) paletteCTab);
			shimmerCT = grayCTab;
			if(noErr == (errCode = HandToHand((Handle *) &shimmerCT))){
				seed = GetCTSeed();
				(**shimmerCT).ctSeed = seed;
				(**shimmerCT).ctFlags |= 0x4000;	/* palette animation */
			}else{
				shimmerCT = NIL;
			}
		}
	}
	return errCode;
}


/* PICTFileToGWorld - read a pict into a gray scale gworld
 */
static OSErr PICTFileToGWorld(FSSpecPtr fs, ScriptCode code, GWorldPtr *gworld){
	OSErr				errCode;
	PicHandle			ph;
	PictInfo			pInfo;
	Integer				ref;
	LongInt				len;
	Rect				frame;
	CGrafPtr			savePort;
	GDHandle			saveGD;
	GWorldPtr			grayImage;
	PixMapHandle		pm;

	errCode = noErr;
	ph = NIL;
	if(noErr == errCode){ errCode = FSpOpenDF(fs, fsCurPerm, &ref); }
	if(noErr == errCode){ errCode = GetEOF(ref, &len);}
	if(noErr == errCode && len < 512){ errCode = eBadPict; }
	if(noErr == errCode){ len -= 512; errCode = SetFPos(ref, fsFromStart, 512L); }
	if(noErr == errCode){ ph = (PicHandle) NewHandle(len); errCode = MemError(); }
	if(noErr == errCode){
		HLock((Handle) ph);
		errCode = FSRead(ref, &len, *ph);
		HUnlock((Handle) ph);
	}
	if(-1 != ref){ FSClose(ref); }
	if(noErr != errCode){
		if(NIL != ph){
			KillPicture(ph);
		}
		return errCode;
	}
	frame = (**ph).picFrame;
/* if the picture isn't 72 dpi, we'll need to scale the frame.
 */
 	GetPictInfo(ph, &pInfo, 0, 0, systemMethod, 0);
	OffsetRect(&frame, -frame.left, -frame.top);	/* 0 base the frame */
	if(pInfo.hRes != (72L <<16) || pInfo.vRes != (72L <<16)){
		frame.right = round( frame.right*(pInfo.hRes/(65536.*72.)));
		frame.bottom = round( frame.bottom*(pInfo.hRes/(65536.*72.)));
	}

	GetGWorld(&savePort, &saveGD);
	if(noErr == (errCode = NewGWorld(&grayImage, 8, &frame, grayCTab, NIL, 0)) &&
		NIL != grayImage){

		SetGWorld(grayImage, NIL);
		ClipRect(&qd.thePort->portRect);
		pm = GetGWorldPixMap(grayImage);
		LockPixels(pm);
		DrawPicture(ph, &frame);
		UnlockPixels(pm);
		KillPicture(ph);
		ph = NIL;
		*gworld = grayImage;
	}
	SetGWorld(savePort, saveGD);
	
	return errCode;
}

/* FileToGWorld - switch based on the file type
 */
static OSErr FileToGWorld(OSType type, FSSpecPtr fs, ScriptCode code, GWorldPtr *gworld){
	switch(type){
	case 'PICT':	return PICTFileToGWorld(fs, code, gworld);
	default:		return PGMFileToGWorld(fs, code, gworld);
	}
	return eBadFileType;
}

/* InitialGray - return TRUE if window intersects a device capable of
	displaying gray.
 */
static Boolean InitialGray(void){
	Rect	r;

	r = qd.thePort->portRect;
	LocalToGlobal(&topLeft(r));
	LocalToGlobal(&botRight(r));
	return world.hasColorQD && (**(**GetMaxDevice(&r)).gdPMap).pixelSize > 1;
}

/* RandomDotOpenFile - 
	make a new window, 
	read a PICT or pgm file in its entirety in to a handle and attach it.
 */
static OSErr RandomDotOpenFile(FSSpecPtr fs, ScriptCode code){
	OSErr				errCode;
	FInfo	fInfo;
	Integer				ref;
	RandomDotWindowPtr	win;
	CGrafPtr			savePort;
	GDHandle			saveGD;
	Boolean				initialGray;

	if(noErr != (errCode = FSpGetFInfo(fs, &fInfo))){
		return errCode;
	}
	if(kPrefType == fInfo.fdType){
		return noErr;
	}
	ref = -1;
	win = (RandomDotWindowPtr) NewPtrClear(sizeof(RandomDotWindowRec));
	if(world.hasColorQD){
		SetPort(GetNewCWindow(rWin, (Ptr) win, (WindowPtr) -1L));
	}else{
		SetPort(GetNewWindow(rWin, (Ptr) win, (WindowPtr) -1L));
	}
	win->hScroll = GetNewControl(128, qd.thePort);
	InitScrollBarClass(&HProcs);
	SetCRefCon(win->hScroll, (LongInt) &HProcs);
	win->vScroll = GetNewControl(128, qd.thePort);
	InitScrollBarClass(&VProcs);
	SetCRefCon(win->vScroll, (LongInt) &VProcs);
	win->fs = *fs;
	win->code = code;
	SetWTitle(qd.thePort, fs->name);
	errCode = MemError();
	GetGWorld(&savePort, &saveGD);
	initialGray = InitialGray();
	if(noErr == errCode){ errCode = FileToGWorld(fInfo.fdType, fs, code, &win->grayImage); }
	if(noErr == errCode){ errCode = NewGWorld(&win->stereoImage, 8, &win->grayImage->portRect, grayCTab, NIL, 0); }
	if(noErr == errCode){
		SetGWorld(win->stereoImage, NIL);
		ClipRect(&qd.thePort->portRect);
		SetGWorld(savePort, saveGD);
		win->flags |= initialGray ? (bIsGray|bShowGrayStereo) : bShowBWStereo;
	}
	if(noErr == errCode){ errCode = ComputeAutoStereogram(win->grayImage, win->stereoImage, 0 != (bIsGray & win->flags)); }
	if(noErr == errCode && FreeMem() < 50000L){ errCode = memFullErr; }
	SetGWorld(savePort, saveGD);
	if(noErr != errCode){
		if(NIL != win){
			RandomDotDisposeWindow((WindowPtr) win);
		}
	}else{
		win->frame = win->grayImage->portRect;
		if(world.hasColorQD){
			RandomDotPalette();
		}
		ZoomWindow((WindowPtr) win, inZoomOut, FALSE);
		RandomDotGrow();
		ShowWindow((WindowPtr) win);
		SelectWindow((WindowPtr) win);
	}
	return errCode;
}



/* RandomDotPalette - set the palette for this window to a gray ramp.
	Inside Mac says it is ok for windows to share a palette
 */
void RandomDotPalette(void){
	NSetPalette(qd.thePort,  grayPalette, pmFgUpdates);
}

/* ShimmerDotPalette - set the palette for this window to an animating
	palette.
 */
void ShimmerDotPalette(void){
	NSetPalette(qd.thePort,  shimmerPalette, pmFgUpdates);
}


/* RandomDotOpenWindow - if fs equals an existing window, return it.
 */
static WindowPtr RandomDotOpenWindow(FSSpecPtr fs, ScriptCode code){
	WindowPtr	win;

	for(win = FrontWindow(); NIL != win; win = (WindowPtr) ((WindowPeek) win)->nextWindow){
		if(((WindowPeek) win)->visible && 
			userKind == ((WindowPeek) win)->windowKind &&
			EqualFSSpec(fs, &((RandomDotWindowPtr) win)->fs) &&
			code == ((RandomDotWindowPtr) win)->code){

			return win;
		}
	}
	return NIL;
}


/* RandomDotOpen - make a new window, and attach the pict to it.
 */
OSErr RandomDotOpen(FSSpecPtr fs, ScriptCode code){
	WindowPtr	win;

	if(NIL != (win = RandomDotOpenWindow(fs, code))){
		SelectWindow(win);
		return noErr;
	}else{
		return RandomDotOpenFile(fs, code);
	}
}

/* RandomDotDisposeWindow - we are done with the window. dump the
	data structures.

 */
void RandomDotDisposeWindow(WindowPtr xin){
	RandomDotWindowPtr	win;

	win = (RandomDotWindowPtr) xin;
	if(NIL != win->grayImage){
		DisposeGWorld(win->grayImage);
	}
	if(NIL != win->stereoImage){
		DisposeGWorld(win->stereoImage);
	}
	DisposeWindow(xin);
	if(NIL == FrontWindow()){
		NoWindowActivateMenus();
	}
}

/* RandomDotKey - handle key down in the window.
 */
void RandomDotKey(EventRecord *e){
}

/* RandomDotClick - handle mouse down in the window
 */
void RandomDotClick(EventRecord *e){
	Rect	contents;
	Point	where;

	if(qd.thePort != FrontWindow()){
		SelectWindow(qd.thePort);
		return;
	}
	where = e->where;
	GlobalToLocal(&where);
	GetContentsRect(&contents);
	if(NOT TrackScroll(where)){
		/* empty */
	}
}

/* Roll - return a number from lo to hi, inclusive
 */
static Integer Roll(Integer lo, Integer hi){
	return lo + ((unsigned) Random()) % ( hi - lo);
}

/* ShuffleCTab - shuffle the entries in a CTabHandle from lo to high
	inclusive.
 */
static void ShuffleCTab(CTabHandle ct, Integer lo, Integer hi){
	Integer i, j;
	ColorSpec	t;

	for( i = hi ; i > lo+1 ; i--){
		j = Roll(lo, i);
		t = (**ct).ctTable[j];
		(**ct).ctTable[j] = (**ct).ctTable[i];
		(**ct).ctTable[i] = t;
	}
}


/* RandomDotIdle - 
 */
void RandomDotIdle(EventRecord *e){
	static unsigned long last;
	RandomDotWindowPtr	win;
	
	if(e->when == last || e->when == last + 1){
		return;
	}
	last = e->when;
	win = (RandomDotWindowPtr) qd.thePort;
	if((bIsShimmer & win->flags) && NIL != shimmerCT){
		ShuffleCTab(shimmerCT, 1, 254);
		AnimatePalette(qd.thePort, shimmerCT, 0, 0, 256);
	}
}


/* RandomDotGWorld - return the GWorld to draw.
 */
static GWorldPtr RandomDotGWorld(RandomDotWindowPtr win){
	switch(win->flags & (bShowGray|bShowBWStereo|bShowGrayStereo)){
	case bShowGray:			return win->grayImage;	break;
	case bShowBWStereo:	
	case bShowGrayStereo:	return win->stereoImage;	break;
	}
	return NIL;
}

/* RandomDotCopybitsWin - used for Update, Print and in Save and Copy
	draws the image using the framep rect on the current grafport.
 */
void RandomDotCopybitsWin(RandomDotWindowPtr win, Rect *framep){
	PixMapHandle		pm;
	GWorldPtr			gworld;

	if(NIL != (gworld = RandomDotGWorld(win))){
		pm = GetGWorldPixMap(gworld);
		LockPixels(pm);
		CopyBits((BitMap *) *pm, &qd.thePort->portBits, &win->frame, framep, srcCopy, NIL);
		UnlockPixels(pm);
	}
}



/* RandomDotUpdate - handle redrawing the window
	partial updates in shimmer mode weren't working right, so we swap the
	palette back. but if whole thing is being updated, then we don't
	swap the palette to avoid excessive screen flicker.

	Note 1: In order to write to a window using an animated palette,
		we need to set bit 14 of the color table flags, so that CopyBits
		will use the color table indices to map values.
 */
void RandomDotUpdate(void){
	RandomDotWindowPtr	win;
	Rect				frame;
	Rect				contents;
	RgnHandle			saveRgn;
	PixMapHandle		pm;
	GWorldPtr			gworld;
	CTabHandle			ct;

	win = (RandomDotWindowPtr) qd.thePort;
	if(userKind == ((WindowPeek) win)->windowKind){
		ct = NIL;
		DrawControls(qd.thePort);
		DrawGrowIcon(qd.thePort);
		frame = win->frame;
		OffsetRect(&frame, 
			-GetControlValue(win->hScroll)-frame.left, 
			-GetControlValue(win->vScroll)-frame.top);
		GetContentsRect(&contents);
		if((bIsShimmer|bShowGrayStereo) == (kShow & win->flags) &&
			NIL != (gworld = RandomDotGWorld(win)) &&
			NIL != (pm = GetGWorldPixMap(gworld)) &&
			NIL != (ct = (**pm).pmTable)){

			(**ct).ctFlags |= 0x4000;	/* Note 1 */
		}
		saveRgn = RestrictClipRect(&contents);
		RandomDotCopybitsWin(win, &frame);
		RestoreClip(saveRgn);
		if(NIL != ct){
			(**ct).ctFlags &= ~0x4000;	/* Note 1 */
		}
	}
}

/* NoWindowActivateMenus - 
 */
static void NoWindowActivateMenus(void){
	MenuHandle	mh;

	mh = GetMHandle(kFileMenu);
	DisableItem(mh, kCloseI);
	DisableItem(mh, kSaveI);
	DisableItem(mh, kSaveAsI);
	DisableItem(mh, kPrintI);
	mh = GetMHandle(kEditMenu);
	DisableItem(mh, kCutI);
	DisableItem(mh, kCopyI);
	DisableItem(mh, kPasteI);
	DisableItem(mh, kClearI);
	mh = GetMHandle(kRandomDotMenu);
	DisableItem(mh, kGrayI);
	DisableItem(mh, kBWStereoI);
	DisableItem(mh, kGrayStereoI);
	SetRandomDotMenu(0);
}

/* RandomDotActivateMenus - 
 */
static void RandomDotActivateMenus(void){
	MenuHandle	mh;
	Rect		r;

	mh = GetMHandle(kFileMenu);
	EnableItem(mh, kCloseI);
	EnableItem(mh, kSaveI);
	EnableItem(mh, kSaveAsI);
	EnableItem(mh, kPrintI);
	mh = GetMHandle(kEditMenu);
	EnableItem(mh, kCutI);
	EnableItem(mh, kCopyI);
	EnableItem(mh, kPasteI);
	EnableItem(mh, kClearI);
	mh = GetMHandle(kRandomDotMenu);
	EnableItem(mh, kGrayI);
	EnableItem(mh, kBWStereoI);
	r = (**LMGetGrayRgn()).rgnBBox;
	if(world.hasColorQD && (**(**GetMaxDevice(&r)).gdPMap).pixelSize > 1){
		EnableItem(mh, kGrayStereoI);
		EnableItem(mh, kShimmerI);
	}
}

/* RandomDotSetRandomDotMenu - from flag to menu.
 */
static void RandomDotSetRandomDotMenu(void){
	RandomDotWindowPtr	win;

	win = (RandomDotWindowPtr) qd.thePort;
	switch(win->flags & kShow){
	case bShowGray:			SetRandomDotMenu(kGrayI);		break;
	case bShowBWStereo:		SetRandomDotMenu(kBWStereoI);	break;
	case bShowGrayStereo:	SetRandomDotMenu(kGrayStereoI);	break;
	case bShowGrayStereo|bIsShimmer:	SetRandomDotMenu(kShimmerI);	break;
	}
}

/* RandomDotActivate - on activate
 */
void RandomDotActivate(void){
	RandomDotWindowPtr	win;

	win = (RandomDotWindowPtr) qd.thePort;
	ShowControl(win->hScroll);
	ShowControl(win->vScroll);
	DrawGrowIcon(qd.thePort);
	RandomDotActivateMenus();
	RandomDotSetRandomDotMenu();
}

/* RandomDotDeactivate - on deactivate
 */
void RandomDotDeactivate(void){
	Rect	r;
	RandomDotWindowPtr	win;

	win = (RandomDotWindowPtr) qd.thePort;
	r = qd.thePort->portRect;
	r.top = r.bottom - (kScrollBarWidth-1);
	r.left = r.right - (kScrollBarWidth-1);
	EraseRect(&r);
	HideControl(win->hScroll);
	HideControl(win->vScroll);
}

/* RandomDotGrow - call this after a grow
	constrains the grow size, 
	moves the scroll bars.
	sets the new maximums.
 */
void RandomDotGrow(void){
	RandomDotWindowPtr	win;
	Point				size, maxSize, newSize;
	Rect				r;


	/* constrains the grow size, 
	 */
	win = (RandomDotWindowPtr) qd.thePort;
	size.h = qd.thePort->portRect.right - qd.thePort->portRect.left;
	size.v = qd.thePort->portRect.bottom - qd.thePort->portRect.top;
	maxSize.h = kScrollBarWidth + win->frame.right - win->frame.left;
	maxSize.v = kScrollBarWidth + win->frame.bottom - win->frame.top;
	newSize.h = Min(size.h, maxSize.h);
	newSize.v = Min(size.v, maxSize.v);
	if(size.h != newSize.h || size.v != newSize.v){
		SizeWindow(qd.thePort, newSize.h, newSize.v, FALSE);
	}

	/* moves the scroll bars.
	 */
	r = qd.thePort->portRect;
	MoveControl(win->vScroll, r.right-kScrollBarWidth, -1 + r.top);
	SizeControl(win->vScroll, kScrollBarWidth+1, 2 + r.bottom - (r.top + kScrollBarWidth));
	MoveControl(win->hScroll, -1, r.bottom - kScrollBarWidth);
	SizeControl(win->hScroll, 2 + r.right - kScrollBarWidth, kScrollBarWidth+1);

	/* sets the new maximums.
	 */
	SetControlMaximum(win->hScroll, Max(0, maxSize.h - newSize.h));
	SetControlMaximum(win->vScroll, Max(0, maxSize.v - newSize.v));
}

/* RandomDotGrowBounds - tell the growBounds how big a rect to use.
 */
void RandomDotGrowBounds(Rect *rp){
	RandomDotWindowPtr	win;

	win = (RandomDotWindowPtr) qd.thePort;
	rp->left = kScrollBarWidth*4;
	rp->top = kScrollBarWidth*4;
	rp->right = Min( kScrollBarWidth + win->frame.right - win->frame.left,
		(**LMGetGrayRgn()).rgnBBox.right - (**LMGetGrayRgn()).rgnBBox.left);
	rp->bottom = Min( kScrollBarWidth + win->frame.bottom - win->frame.top,
		(**LMGetGrayRgn()).rgnBBox.bottom - (**LMGetGrayRgn()).rgnBBox.top);
}

/* RandomDotIdealSize - called by ZoomTheWindow to tell it how big a rect to use.
	this is almost what RandomDotGrowBounds, we just need to remove the minimum info
	and replace it by zeroes.
 */
void RandomDotIdealSize(WindowPtr win, Rect *rp){
	WindowPtr	savePort;

	GetPort(&savePort);
	SetPort(win);
	RandomDotGrowBounds(rp);
	rp->left = rp->top = 0;
	SetPort(savePort);
}



