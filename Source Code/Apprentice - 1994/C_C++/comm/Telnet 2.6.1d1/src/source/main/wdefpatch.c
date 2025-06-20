/*******************************************

	WDEF Patcher
	Steve Falkenburg MacDTS
	�1991 Apple Computer
	
	This snippet shows how you can add a simple extra part to a WDEF without
	writing an entire WDEF.  It also shows how to access the new part via
	FindWindow().
	
	Roberto Avanzi (independent programmer), June 18, 1992
	Added support for tracking the extra part, in a way similar to the one used
	by the system.
	given back to Apple as an enhanced snippet (mmmh, sounds quite absurd)
	
	6/1/92	SJF		fixed a5 problem in WDEF patch (StripAddress is glue, and a5 wasn't set up)
	6/1/92	SJF		fixed varCode bug that made zoom boxes not work (masked out high 8 bits)
	
*******************************************/

#ifdef MPW
#pragma segment 22
#endif
 
#include "TelnetHeader.h"
#include <GestaltEqu.h>
#ifdef THINK_C
#include <SysEqu.h>
#endif
#include "wind.h"
#include "wdefpatch.proto.h"

static void drawicon(short id, Rect *dest);

/* 931112, ragge, NADA, KTH */
static void drawSize(Rect *wSize, WindowPtr window);

/* add 2 to this when checking with FindWindow() ! */

#define kOurHit	32


/* 
 * this struct allows us to insert a WDEF patch safely.  It contains a jump instruction
 * and stores the old handle to the WDEF
 */

typedef struct {
	short jmpInst;
	ProcPtr patchAddr;
	Handle oldAddr;
	Boolean partState;	/* roberto avanzi jun 18 1992 */
	long ourA5;
	struct WindRec *tw;
} WDEFPatch, *WDEFPatchPtr, **WDEFPatchHndl;


/* 
 * this installs the WDEF patch into a window 
 */
void PatchWindowWDEF (WindowPtr window, struct WindRec *tw)
{
#ifdef	__powerpc__
	return;
#else
	WDEFPatchHndl wdefHndl;
	WDEFPatchPtr wdefPatch;
	Handle oldAddr;
	unsigned long wdefEntry;
	
	wdefHndl = (WDEFPatchHndl)NewHandle(sizeof(WDEFPatch));
#ifdef notdef
	if (MemError() != noErr)
		/* ... */
#endif

	oldAddr = ((WindowPeek)window)->windowDefProc;
	if (GetMMUMode()) // 32-bit
		wdefEntry = (unsigned long)wdefHndl;
	else
		wdefEntry = (unsigned long)StripAddress(wdefHndl) | ((unsigned long)oldAddr&0xff000000);

	HLock((Handle)wdefHndl);
	wdefPatch = *wdefHndl;
	wdefPatch->oldAddr = oldAddr;
	wdefPatch->jmpInst = 0x4ef9; /*JMP*/
	wdefPatch->patchAddr = (ProcPtr)MyWDEFPatch;
	wdefPatch->ourA5 = *(long *)CurrentA5;
    wdefPatch->tw = tw;

	HUnlock((Handle)wdefHndl);

	((WindowPeek)window)->windowDefProc = (Handle)wdefEntry;
#endif
}


/* 
 * RePatchWindowWDEF
 * this adjusts the tw pointer for a patched window
 * We have to do this since the tw for a window can change when other
 * windows are killed.
 */
void RePatchWindowWDEF (WindowPtr window, struct WindRec *tw)
{
#ifdef __powerpc__
	return;
#else
	WDEFPatchHndl wdPatch;

	wdPatch = (WDEFPatchHndl) ((WindowPeek)window)->windowDefProc;
	(**wdPatch).tw = tw;
#endif
}


/*
 * GetPatchStuffHandle
 * This returns the handle to our patch block so we can release it
 * when killing windows. The tw is verified to insure that this
 * window is really patched.
 */
Handle GetPatchStuffHandle (WindowPtr window, struct WindRec *tw)
{
#ifdef __powerpc__
	return (NULL);
#else
	WDEFPatchHndl wdPatch;

	wdPatch = (WDEFPatchHndl) ((WindowPeek)window)->windowDefProc;
	if ((**wdPatch).tw == tw)
		return ((Handle)wdPatch);
	else
		return ((Handle)0);
#endif
}


pascal long MyWDEFPatch (short varCode, WindowPtr window, short message, long param)
{
#ifdef __powerpc__
	return 0;
#else
	WDEFPatchHndl wdPatch;
	pascal long (*wdefProc)(short varCode,WindowPtr window,short message,long param);
	Handle oldWDEF;
	long result;
	Rect ourRect,ourElementRect;
	GrafPtr	savePort;
	GrafPtr aPort;
	RgnHandle aRgn;
	Rect aRect;
	struct WindRec *tw;
	long appA5, saveA5;
	
	wdPatch = (WDEFPatchHndl) ((WindowPeek)window)->windowDefProc;
	appA5 = (**wdPatch).ourA5;
	saveA5 = SetA5(appA5);

	ourRect = (**((WindowPeek)window)->strucRgn).rgnBBox;
	/* our 16x16 rectangle */
	SetRect(&ourElementRect,ourRect.right-42,ourRect.top+1,ourRect.right-26,ourRect.top+17);
	
	tw = (**wdPatch).tw;

	oldWDEF = (**wdPatch).oldAddr;
	HLock(oldWDEF);
	wdefProc = (void *)*oldWDEF;
	wdefProc = (void *)StripAddress(wdefProc);

	/* 
	 * now, folks, WHY do I check it, u'll ask me ? Heh, it's a funny quirk in
	 * the sys WDEF (at least, sys 7's, dunno whattabout older ones) .
	 * Suppose You click once in the grow icon and DO NOT resize the window.
	 * (remember, just click and do not move the mouse in the meantime).
	 * Then you click on the added part. That part is tracked in the right way,
	 * 							*	BUT   *
	 * ALSO the zoom box gets hilited. Dunno why should happen, but It's a lot
	 * of FUN. Sadly, cannot find its place in any useful app. Therefore we
	 * do this check.
	 * APPLE says: if you write your own WDEF and receive unknown messages, do not
	 * 			do anything. pass along and do NOTHING.
	 * APPLE does: we get something new ? therefore we process it anyway, just to
	 * 			keep developers writing workarounds and keep their minds afresh.
	 * What else can we say ? Thanks !!!!! (Roberto Avanzi june 19, 1992)
	 */
	if ( (message == wDraw) ? (((short)param) != kOurHit ) : true )
		result = (wdefProc)(varCode,window,message,param);

	if (((WindowPeek)window)->visible)
	if (((WindowPeek)window)->hilited)
	{
		switch (message) {
			case wDraw:
				GetPort(&savePort);
				GetWMgrPort(&aPort);
				SetPort(aPort);
				aRgn = NewRgn();
				GetClip(aRgn);
				SetRect(&aRect,-32000,-32000,32000,32000);
				ClipRect(&aRect);
				switch ( (short) param ) {	// Roberto Avanzi 18-06-1992: support for 
											// tracking of the new part
					case 0:
						(**wdPatch).partState = false;

					case kOurHit:
						PenNormal();							// draw our part
	
 						if (tw->edata != NULL) {
 							if (tw->edata->encrypt_output || tw->edata->decrypt_input) {
 								/* 
 								 * erase 18 x 11. This gives us a 1 pixel margin
 								 * on the left and right, and matches the mask that
 								 * we're using in our crsr resources.
 								 */
 								InsetRect(&ourElementRect, -1, 0);
 								ourElementRect.top += 3;
 								ourElementRect.bottom -= 2;
 								EraseRect(&ourElementRect);
 								ourElementRect.top -= 3;
 								ourElementRect.bottom += 2;
 								InsetRect(&ourElementRect, 1, 0);
 							}						
 							if (tw->edata->encrypt_output && tw->edata->decrypt_input)
 								drawicon(lockcrsr, &ourElementRect);
 							else if (tw->edata->encrypt_output)
 								drawicon(rightcrsr, &ourElementRect);
 							else if (tw->edata->decrypt_input)
 								drawicon(leftcrsr, &ourElementRect);
 						}
						break;
						
					default:
						break;
				}
				SetClip(aRgn);
				DisposeRgn(aRgn);
				SetPort(savePort);
				break;

				// removed this test so that one can move the window
				// also when clicking on the icon area.
				// 931112, ragge, NADA, KTH
#ifdef NOTDEF
			case wHit:
				hitPt = (Point *)&param;					// hit test our part
				if (PtInRect(*hitPt,&ourElementRect))
				{
					//result =  kOurHit;
				}
				break;
#endif
			
			case wGrow:		/* 931112, ragge, NADA, KTH */
				drawSize((Rect *) param, window);
				break;

			default:
				break;
		}	// switch
	}	//	if hilited (otherwise we dont see the new box, addition by Roberto Avanzi)
	HUnlock(oldWDEF);
	
	SetA5(saveA5);
	
	return result;
#endif
}


/*
 * drawicon
 */
void drawicon (short id, Rect *dest)
{
	long qdv;
    Handle ih = 0;
    Rect source_rect;
    BitMap mask_bitmap;
    GrafPtr local_port;
	PixMap *pm;
	Ptr colormap;
	CCrsr *ccrsr;
	BitMap src_bitmap;

	GetPort(&local_port);

	ih = GetResource ('crsr', id);				/* color cursor */
	if (!ih)
		return;
	DetachResource(ih);		/* ... need to save handle somewhere ... */
	HLock(ih);				/* ... to avoid reloading the resource all the time */

	/* 
	 * Set source Rect and intialize source BitMaps. 
	 * A few PixMap fields must be munged;
	 */
	SetRect (&source_rect, 0, 0, 16, 16);
	
	ccrsr = (CCrsr *)(*ih);

	mask_bitmap.bounds = source_rect;
	mask_bitmap.rowBytes = 2;
	mask_bitmap.baseAddr = (Ptr)&ccrsr->crsrMask; /* (Ptr)(((Byte *)(*ih)) + 52); */
	
	/*
	 * if gestalt fails or no color quickdraw, just use the b/w bitmap.
	 */
	if (Gestalt(gestaltQuickdrawVersion, &qdv) || ((qdv & gestalt32BitQD) == 0)) {
		src_bitmap.bounds = source_rect;
		src_bitmap.rowBytes = 2;
		src_bitmap.baseAddr = (Ptr)&ccrsr->crsr1Data;
		CopyBits(&src_bitmap, &(local_port->portBits), &source_rect, dest,
				 srcCopy, nil);	
	} else {
		pm = (PixMap *) ((unsigned char *)ccrsr + (long)ccrsr->crsrMap);
		pm->baseAddr = (Ptr) ((unsigned char *)ccrsr + (long)ccrsr->crsrData);
			colormap = (Ptr) ((unsigned char *)ccrsr + (long)pm->pmTable);
		pm->pmTable = (CTabHandle) &colormap;		/* handle to colormap */

		/* 
		 * Draw the crsr using its mask. 
		 * Do we need the mask ??? ...
		 */
		CopyMask((BitMap *)pm, &mask_bitmap, &(local_port->portBits),
				 &source_rect, &source_rect, dest);
	}

	HUnlock(ih);
	ReleaseResource((Handle)ih);
	DisposHandle((Handle)ih);
}

/* 931112, ragge, NADA, KTH */
#define	HOFFSET	2
#define VOFFSET	2
static Rect gGrowTextBox;
static Rect gGrowTextBoxInset;
Boolean gDoGrowSize = false;
static struct growSavedStruct {
    Point charSize;
    Point charInset;
	Boolean eraseIt;

	PenState	savedPen;
    short txFont;
    Style txFace;
    short txMode;
    short txSize;
} gGrowSaved;

/* 931112, ragge, NADA, KTH */
void setupForGrow(WindowPtr window, short hCharInset, short vCharInset, short hCharSize, short vCharSize)
{
#ifdef __powerpc__
	return;
#else
	GrafPtr	savedPort;
	FontInfo fInfo;

	GetPort(&savedPort);
	SetPort(window);
		
	gGrowSaved.charSize.h = hCharSize;
	gGrowSaved.charSize.v = vCharSize;
	gGrowSaved.charInset.h = hCharInset;
	gGrowSaved.charInset.v = vCharInset;

	if(gGrowSaved.charSize.h == 0)	// don't want zero-div
		gGrowSaved.charSize.h = 1;
	if(gGrowSaved.charSize.v == 0)
		gGrowSaved.charSize.v = 1;
		
	gGrowSaved.eraseIt = false;
	
	GetPenState(&gGrowSaved.savedPen);
	
	gGrowSaved.txFont = window->txFont;
	gGrowSaved.txFace = window->txFace;
	gGrowSaved.txMode = window->txMode;
	gGrowSaved.txSize = window->txSize;

	PenNormal();
	TextFont(1);
	TextSize(9);
	TextFace(0);
	TextMode(srcCopy);
	
	GetFontInfo(&fInfo);

	gGrowTextBox.top = VOFFSET;
	gGrowTextBox.left = HOFFSET;
	gGrowTextBox.bottom = VOFFSET + fInfo.ascent + fInfo.descent + fInfo.leading + 3;	// Yes, 3!
	gGrowTextBox.right = HOFFSET + StringWidth("\p000 * 000") + 6;
	gGrowTextBoxInset = gGrowTextBox;
	InsetRect(&gGrowTextBoxInset, 1, 1);
	
	gDoGrowSize = true;
	
	SetPort(savedPort);
#endif
}

/* 931112, ragge, NADA, KTH */
void cleanupForGrow(WindowPtr window)
{
#ifdef __powerpc__
	return;
#else

	GrafPtr	savedPort;
	GetPort(&savedPort);
	SetPort(window);
	
	gDoGrowSize = false;
	
	InvalRect(&gGrowTextBox);
	
	SetPenState(&gGrowSaved.savedPen);
	
	window->txFont = gGrowSaved.txFont;
	window->txFace = gGrowSaved.txFace;
	window->txMode = gGrowSaved.txMode;
	window->txSize = gGrowSaved.txSize;
	
	SetPort(savedPort);
#endif
}

/* 931112, ragge, NADA, KTH */
void drawSize(Rect *wSize, WindowPtr window)
{
	unsigned char string[50], yValLen;
	GrafPtr savedPort;
	
	if(!gDoGrowSize)
		return;

	GetPort(&savedPort);
	SetPort(window);
	
	if(!gGrowSaved.eraseIt) {
		NumToString((wSize->right - wSize->left - 15 - gGrowSaved.charInset.h) / gGrowSaved.charSize.h, string);
		string[++string[0]] = ' ';
		string[++string[0]] = '*';
		NumToString((wSize->bottom - wSize->top - 15 - gGrowSaved.charInset.v) / gGrowSaved.charSize.v, string + string[0] + 1);
		yValLen = string[string[0] + 1];
		string[++string[0]] = ' ';
		string[0] += yValLen;
		TextBox(string + 1, string[0], &gGrowTextBoxInset, 1);
		FrameRect(&gGrowTextBox);
	} else {
		Rect rGlob = gGrowTextBox;
		LocalToGlobal((Point *) &(rGlob.top));
		LocalToGlobal((Point *) &(rGlob.bottom));
	}
	
	gGrowSaved.eraseIt = !gGrowSaved.eraseIt;

	SetPort(savedPort);
}
