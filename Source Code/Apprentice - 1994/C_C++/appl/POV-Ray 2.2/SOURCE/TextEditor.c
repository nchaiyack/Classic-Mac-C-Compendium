/*==============================================================================
Project:	POV-Ray

Version:	2.2

File:	TextEditor.c

Description:
	This file contains Macintosh-specific routines for the
	source code text editor.
------------------------------------------------------------------------------
Author:
	Symantec, mods by Jim Nitchals, more mods & Think 6 updates by Eduard [esp] Schwan
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
	920815	[jln]	version 1.0 Mac released.
	920908	[esp]	version 1.1 alpha Mac
	920912	[esp]	changed window id to 1000 & moved #define here from POVMac.h
	921110	[esp]	Added TextEditor.h file & pulled in defs to there
	921207	[esp]	Added code to handle zoom box
	921211	[esp]	Fixed bug in Open: if open failed, left New & Open items dimmed
	930710	[esp]	fixed "cancelling of save-as dialog" logic
	930817	[esp]	Added FlushVol calls in SaveFile and SaveAs to force disk update
	931001	[esp]	version 2.0 finished (Released on 10/4/93)
	931119	[djh]	2.0.1 conditionally compiles for PPC machines, keyword __powerc
==============================================================================*/


#include "TextEditor.h"

/*==== Standard C headers ====*/
#include <stdio.h>

/*==== Mac toolbox headers ====*/
// NOTE: _H_MacHeaders_ is defined by Think C if it is using
// precompiled headers.  This is only for compilation speed improvement.
#if !defined(_H_MacHeaders_)
#include <types.h>		// basic types
#include <desk.h>		// SystemEdit, etc.
#include <errors.h>		// noErr, etc.
#include <files.h>		// FSOpen, etc. 
#include <fonts.h>		// monaco, etc. 
#include <memory.h>		// NewHandle, etc. 
#include <menus.h>		// MenuHandle, etc. 
#include <packages.h>	// SFGetFile, etc. 
#include <scrap.h>		// ZeroScrap, etc. 
#include <toolutils.h>	// watchCursor, etc. 
#include <windows.h>	// SizeWindow, etc. 
#endif // _H_MacHeaders_


/*==== POV Mac Library routines =====*/

#include "POVLib.h"
#include "ImageWindow.h" // CloseImageWindow(), gImageWindIsValid


/*==== globals (external scope) ====*/

#if defined(__powerc)
extern RoutineDescriptor gScrollRD;
#endif

WindowPtr			gSrcWind_Window;
TEHandle			gSrcWind_TEH;
Boolean				gSrcWind_dirty;
Boolean				gSrcWind_visible;
Str255 				gSrcWind_FileName;
short				gSrcWind_VRefNum;
ControlHandle 		gSrcWind_VScroll;


/*==== globals (local scope) ====*/

static WindowRecord	wRecord;
static int			linesInFolder;


// ==============================================
#define SrcWind_WindID 	1000
#define ErrorAlert		256
#define	AdviseAlert		257

#define fmNew			1
#define fmRevert		7
#define fmPageSetUp		9
#define fmPrint			10
#define fmQuit			13

#define aaSave			1
#define aaDiscard		2
#define aaCancel		3

#define SBarWidth		15

#define	ours(w)		((gSrcWind_Window != NULL) && (w == gSrcWind_Window))

#define	kUntitledFName	"\pUntitled.POV"


// ==============================================
int SetUpFiles(void)
{
	pStrCopy(kUntitledFName, gSrcWind_FileName);
	gSrcWind_VRefNum = 0;
	return 0; // should be void fn!?
}



// ==============================================
int DoFile(int item)
{
	short 	vRef, refNum;
	Str255	fn;

	switch (item) {

		case fmn_open:
			HideWindow(gSrcWind_Window);	/* gives a sense of closing the old file. */
			if (OldFile(fn, &vRef))
			{
				gSrcWind_VRefNum = 0;
				gSrcWind_dirty = 0;
				if (FSOpen(fn, vRef, &refNum)==noErr) {
					if (ReadFile(refNum, gSrcWind_TEH)) {
						pStrCopy(fn, gSrcWind_FileName);
						gSrcWind_VRefNum = vRef;
						SetWTitle(gSrcWind_Window, gSrcWind_FileName);
					}
					if (FSClose(refNum)==noErr) ;
					ShowWindow(gSrcWind_Window);
					gSrcWind_visible = TRUE;
					TESetSelect(0, 0, gSrcWind_TEH);
					ShowSelect();
				}
				else
					FileError("\pError opening ", fn);
			}
			break;

		case fmn_close:
			if (DoFile(render_file))	// recurse.
				CloseMyWindow();
			break;

		case render_file:
			if (gSrcWind_dirty)
			{
				ParamText("\pSave changes to Ò",
						gSrcWind_FileName,
						"\pÓ?", "\p");
				switch (Alert(AdviseAlert, 0L))
				{
				case aaSave:
					if (gSrcWind_VRefNum == 0)
					{ // create new file
						pStrCopy(gSrcWind_FileName, fn);
						if (!SaveAs(fn, &vRef))
							return(0);
					}
		 			else // save existing file
						if (!SaveFile(gSrcWind_FileName, gSrcWind_VRefNum))
							return(0);
		 			break;
		 		case aaCancel:
					return(0);
		 		case aaDiscard:
					gSrcWind_dirty = 0;
		 		}
		 	}
			break;

		case fmn_save:
			// if file already open
			if (gSrcWind_VRefNum != 0)
			{
				if (!SaveFile(gSrcWind_FileName, gSrcWind_VRefNum))
					return(0);
				break;
			}
			// else drop through to here..

		case fmn_saveas:
			pStrCopy(gSrcWind_FileName, fn);
			if (SaveAs(fn, &vRef))
			{
				pStrCopy(fn, gSrcWind_FileName);
				gSrcWind_VRefNum = vRef;
				SetWTitle(gSrcWind_Window, gSrcWind_FileName);
			}
			else
				return(0);
			break;

		case fmRevert:
			ParamText("\pRevert to last saved version of Ò",
					gSrcWind_FileName, "\pÓ?", "\p");
			switch (Alert(AdviseAlert, 0L)) {
			case aaSave:
				HidePen();
				TESetSelect(0, (**gSrcWind_TEH).teLength, gSrcWind_TEH);
				ShowPen();
				TEDelete(gSrcWind_TEH);
				if ((gSrcWind_VRefNum != 0) &&
					(FSOpen(gSrcWind_FileName, gSrcWind_VRefNum, &refNum)==noErr))
				{
					gSrcWind_dirty = !ReadFile(refNum, gSrcWind_TEH); 
					if (FSClose(refNum)==noErr) ;
				}
				ShowWindow(gSrcWind_Window);
				gSrcWind_visible = TRUE;
				UpdateWindow(gSrcWind_Window);
	 		case aaCancel:
	 		case aaDiscard:
				return(0);
	 		}
	
			break;

#if defined(_DO_PRINTING_SOMEDAY_)
		case fmPageSetUp:
			DoPageSetUp();
			break;
		case fmPrint:
			PrintText( (**gSrcWind_TEH).hText, (long)(**gSrcWind_TEH).teLength, (GrafPtr)gSrcWind_Window,
							StringWidth("\pmmmm"));
			break;
#endif // DO_PRINTING_SOMEDAY
		case fmQuit: 
			if (DoFile(fmn_close))
				ExitToShell();
	}
	return(1);
}



// ==============================================
static Point SFwhere;
static SFReply reply;


int SaveAs(Str255 fn, short *vRef)
{
	short refNum;
	
	if (NewFile(fn, vRef)) 
		if (!CreateFile(fn, vRef, &refNum))
		{
			FileError("\pError creating file ", fn);
			return (0);
		}
		else
		{
			char hstate;
			
			hstate = HGetState((**gSrcWind_TEH).hText);
			HLock((**gSrcWind_TEH).hText);
			if (WriteFile(refNum, (*(**gSrcWind_TEH).hText), (long)(**gSrcWind_TEH).teLength))
				FileError("\pError writing file ", fn);
			HSetState((**gSrcWind_TEH).hText, hstate);
			FSClose(refNum);
			gSrcWind_dirty = 0;
			// flush any buffers to disk
			FlushVol(NULL, *vRef);
			return(1);
		}
}



// ==============================================
int SaveFile(Str255 fn, short vRef)
{
	short refNum;
	if (FSOpen(fn, vRef, &refNum) != noErr)
	{
		FileError("\pError opening file ", fn);
		return (1);
	}
	else
	{
		char hstate;
		
		hstate = HGetState((**gSrcWind_TEH).hText);
		HLock((**gSrcWind_TEH).hText);
		if (WriteFile(refNum, (*(**gSrcWind_TEH).hText), (long)(**gSrcWind_TEH).teLength))
			FileError("\pError writing file ", fn);
		HSetState((**gSrcWind_TEH).hText, hstate);
		gSrcWind_dirty = 0;
		FSClose(refNum);
		// flush any buffers to disk
		FlushVol(NULL, vRef);
		return(1);
	}
}



// ==============================================
int NewFile(Str255 fn, short *vRef)
{
	GetBestDialogPos(&SFwhere);
	SFPutFile(SFwhere, "\pSave POV-Ray text file as", fn, 0L, &reply);
	if (!reply.good)
		return (0);
	else {
		pStrCopy(reply.fName, fn);
		*vRef = reply.vRefNum;
		return(1);
	}
}



// ==============================================
int OldFile(Str255 fn, short *vRef)
{
	SFTypeList	myTypes;
	
	myTypes[0]='TEXT';

	GetBestDialogPos(&SFwhere);
	SFGetFile(SFwhere, "\p", 0L, 1, myTypes, 0L, &reply );

	if (!reply.good)
		return (0);
	else {
		pStrCopy(reply.fName, fn);
		*vRef = reply.vRefNum;
		return(1);
	}
}



// ==============================================
int CreateFile(Str255 fn, short *vRef, short *theRef)
{
	OSErr io;
	
	io=Create(fn, *vRef, kAppSignature, 'TEXT');
	if ((io == noErr) || (io == dupFNErr))
		io = FSOpen(fn, *vRef, theRef );

	return ((io == noErr) || (io == dupFNErr));
}



// ==============================================
int WriteFile(short refNum, char *p, long num)
{
	OSErr io;			
	/* gee,	somebody should check the return code for errors */
	io=FSWrite(refNum, &num, p);
	if (io)
		return(io);
	io=SetEOF(refNum, num);
	return(io);
}



// ==============================================
int ReadFile(short refNum, TEHandle textH)
{
	char	buffer[256];
	long	count;
	OSErr		io;
	
	(**textH).selStart = 0;
	(**textH).selEnd = (**textH).teLength;
	TEDelete(textH);
	GetEOF(refNum, &count);
	if (count > 32767L)
		return (999);
	do {
		count = 256;
		io = FSRead(refNum, &count, &buffer);
		TEInsert(&buffer, count, textH);
	} while (io==noErr);
	return (io == eofErr);
}



// ==============================================
/* copies a pascal string from p1 to p2 */
int pStrCopy(StringPtr p1, StringPtr p2)
{
	register int len = *p1;	// added asignment
	
	*(p2++) = *(p1++);			// added parens
	while (--len>=0) *(p2++)=*(p1++);	// added parens
	return 0; // should be void fn!?
}



// ==============================================
int FileError(Str255 s, Str255 f)
{
	ParamText(s, f,"\p", "\p");
	Alert(ErrorAlert, 0L);
	return 0; // should be void fn!?
}



// ==============================================
void PreInitWindows(void)
{
	if (gSrcWind_Window == 0)
	{
		SetUpWindows();
		HideWindow(gSrcWind_Window);	/* just in case it's already visible */
	}
}



// ==============================================
int SetUpWindows(void)
{
	Rect	viewRect;
	Rect	vScrollRect;

	gSrcWind_Window = GetNewWindow(SrcWind_WindID, &wRecord, (WindowPtr)NULL);
	if (!gSrcWind_Window)
		return 1;
	SetPort(gSrcWind_Window);
	TextFont(monaco);
	TextSize(9);
	vScrollRect = (*gSrcWind_Window).portRect;
	vScrollRect.left = vScrollRect.right-SBarWidth;
	vScrollRect.right += 1;
	vScrollRect.bottom -= SBarWidth-1;
	vScrollRect.top -= 1;
	gSrcWind_VScroll = NewControl( gSrcWind_Window, &vScrollRect, "\pSrcVScroll", 1, 0, 0, 0,
		scrollBarProc, 0L);

	viewRect = qd.thePort->portRect;
	viewRect.right -= SBarWidth;
	viewRect.bottom -= SBarWidth;
	InsetRect(&viewRect, 4, 4);
	gSrcWind_TEH = TENew(&viewRect, &viewRect);
	SetView((WindowPtr) qd.thePort);
	gSrcWind_dirty = 0;
	return 0; // should be void fn!?
}



// ==============================================
int AdjustText(void)
{
	int		oldScroll, newScroll, delta;
	
	oldScroll = (**gSrcWind_TEH).viewRect.top - (**gSrcWind_TEH).destRect.top;
	newScroll = GetCtlValue(gSrcWind_VScroll) * (**gSrcWind_TEH).lineHeight;
	delta = oldScroll - newScroll;
	if (delta != 0)
	  TEScroll(0, delta, gSrcWind_TEH);
	SetVScroll();
	return 0; // should be void fn!?
}



// ==============================================
int SetVScroll(void)
{
	register int	n;
	
	n = (**gSrcWind_TEH).nLines-linesInFolder;

	if ((**gSrcWind_TEH).teLength > 0 && (*((**gSrcWind_TEH).hText))[(**gSrcWind_TEH).teLength-1]=='\r')
		n++;

	SetCtlMax(gSrcWind_VScroll, n > 0 ? n : 0);
	ShowControl(gSrcWind_VScroll);
	return 0; // should be void fn!?
}



// ==============================================
int ShowSelect(void)
{
	register	int		topLine, bottomLine, theLine;
	
	SetVScroll();
	AdjustText();
	
	topLine = GetCtlValue(gSrcWind_VScroll);
	bottomLine = topLine + linesInFolder;
	
	// count up the lines to current
	if ((**gSrcWind_TEH).selStart < (**gSrcWind_TEH).lineStarts[topLine] ||
			(**gSrcWind_TEH).selStart >= (**gSrcWind_TEH).lineStarts[bottomLine]) {
		for (	theLine = 0;
				(**gSrcWind_TEH).selStart >= (**gSrcWind_TEH).lineStarts[theLine];
				theLine++)
			;
		SetCtlValue(gSrcWind_VScroll, theLine - linesInFolder / 2);
		AdjustText();
	}
	return 0; // should be void fn!?
}



// ==============================================
void SelectAllText(void)
{	
	TESetSelect(0, -1, gSrcWind_TEH);
	SetVScroll();
	AdjustText();	
}



// ==============================================
int SetView(WindowPtr w)
{
	(**gSrcWind_TEH).viewRect = w->portRect;
	(**gSrcWind_TEH).viewRect.right -= SBarWidth;
	(**gSrcWind_TEH).viewRect.bottom -= SBarWidth;
	InsetRect(&(**gSrcWind_TEH).viewRect, 4, 4);

	linesInFolder = ((**gSrcWind_TEH).viewRect.bottom-(**gSrcWind_TEH).viewRect.top)/(**gSrcWind_TEH).lineHeight;
	(**gSrcWind_TEH).viewRect.bottom = (**gSrcWind_TEH).viewRect.top + (**gSrcWind_TEH).lineHeight*linesInFolder;
	(**gSrcWind_TEH).destRect.right = (**gSrcWind_TEH).viewRect.right;
	TECalText(gSrcWind_TEH);
	return 0; // should be void fn!?
}



// ==============================================
int UpdateWindow(WindowPtr theWindow)
{
	GrafPtr	savePort;
	
	GetPort(&savePort);
	SetPort(theWindow);

	BeginUpdate(theWindow);
	EraseRect(&theWindow->portRect);
	DrawControls(theWindow);
	DrawGrowIcon(theWindow);
	TEUpdate(&theWindow->portRect, gSrcWind_TEH);
	EndUpdate(theWindow);

	SetPort(savePort);
	return 0; // should be void fn!?
}



// ==============================================
pascal void ScrollProc(ControlHandle theControl, short theCode)
{
	int	pageSize;
	int	scrollAmt;
	int oldCtl;
	
	if (theCode == 0)
		return ;
	
	pageSize = ((**gSrcWind_TEH).viewRect.bottom-(**gSrcWind_TEH).viewRect.top) / 
			(**gSrcWind_TEH).lineHeight - 1;
			
	switch (theCode) {
		case inUpButton: 
			scrollAmt = -1;
			break;
		case inDownButton: 
			scrollAmt = 1;
			break;
		case inPageUp: 
			scrollAmt = -pageSize;
			break;
		case inPageDown: 
			scrollAmt = pageSize;
			break;
	}

	oldCtl = GetCtlValue(theControl);
	SetCtlValue(theControl, oldCtl+scrollAmt);

	AdjustText();
}



// ==============================================
int DoContent(WindowPtr theWindow, EventRecord *theEvent)
{
	short			cntlCode;
	ControlHandle 	theControl;
	GrafPtr			savePort;
	
	GetPort(&savePort);
	SetPort(theWindow);

	GlobalToLocal(&theEvent->where);
	if ((cntlCode = FindControl(theEvent->where, theWindow, &theControl)) == 0) {
		if (PtInRect(theEvent->where, &(**gSrcWind_TEH).viewRect))
			TEClick(theEvent->where, (theEvent->modifiers & shiftKey)!=0, gSrcWind_TEH);
	} else if (cntlCode == inThumb) {
		TrackControl(theControl, theEvent->where, 0L);
		AdjustText();
	} else
#if defined(__powerc)
		TrackControl(theControl, theEvent->where, (ControlActionUPP)&gScrollRD);
#else
		TrackControl(theControl, theEvent->where, (ProcPtr)&ScrollProc);
#endif

	SetPort(savePort);
	return 0; // should be void fn!?
}



// ==============================================
void MyResizeWindow(WindowPtr w, short h, short v)
{
	Rect	oldHorizBar;
	Rect 	r;
	
	SetPort(w);

	oldHorizBar = w->portRect;
	oldHorizBar.top = oldHorizBar.bottom - (SBarWidth+1);

	SizeWindow(w, h, v, false);

	// remember this new size in prefs
	GetGlobalWindowRect(w, &(**gFilePrefs_h).srcWind_pos);

	InvalRect(&w->portRect);
	
	SetView(w);

	EraseRect(&oldHorizBar);
	
	MoveControl(gSrcWind_VScroll, w->portRect.right - SBarWidth, w->portRect.top-1);
	SizeControl(gSrcWind_VScroll, SBarWidth+1, w->portRect.bottom - w->portRect.top-(SBarWidth-2));
	r = (**gSrcWind_VScroll).contrlRect;
	ValidRect(&r);

	SetVScroll();
	AdjustText();
	
} // MyResizeWindow



// ==============================================
void MyGrowWindow(WindowPtr w, Point p)
{
	GrafPtr		savePort;
	long		theResult;
	Rect		r;
	
	GetPort(&savePort);
	SetPort(w);

	GetMaxGrowRect(w, &r);	
	theResult = GrowWindow(w, p, &r);
	if (theResult != 0)
		MyResizeWindow(w, LoWord(theResult), HiWord(theResult));

	SetPort(savePort);
} // MyGrowWindow



// ==============================================
int CloseMyWindow(void)
{

	if (gSrcWind_VRefNum != 0)
	{
		WriteFilePrefs();	// Save window info
		// OK, now we're done with this file, release the pesky Working Directory
//		CloseWD(gSrcWind_VRefNum);
		gSrcWind_VRefNum = 0;
	}

	// if there's a window (always) then hide it
	if (gSrcWind_Window)
		HideWindow(gSrcWind_Window);
	gSrcWind_visible = false;

	// Image window also no longer valid!
	CloseImageWindow();
	gImageWindIsValid = false;

	// get rid of TE text (select all & delete)
	if (gSrcWind_TEH)
	{
		TESetSelect(0, (**gSrcWind_TEH).teLength, gSrcWind_TEH);
		TEDelete(gSrcWind_TEH);
	}

	SetVScroll();

	SetUpFiles();

	return 0; // should be void fn!?
}



// ==============================================
main_init() 
{
	gSrcWind_Window = NULL;
	SetUpFiles();
	PreInitWindows();
}


// ==============================================
int DoEditMouseDown(int windowPart, WindowPtr whichWindow, EventRecord *myEvent)
{
	switch (windowPart) {
		case inGoAway:
			if (ours(whichWindow) && (gDoingRender == 0))
				if (TrackGoAway(gSrcWind_Window, myEvent->where))
					DoFile(fmn_close);
			break;

		case inDrag:
			if (ours(whichWindow))
			{
				SelectWindow(whichWindow);
				DragWindow(whichWindow, myEvent->where, &gDragBounds);
				GetGlobalWindowRect(whichWindow, &(**gFilePrefs_h).srcWind_pos);
			}
			break;

		case inGrow:
			if (ours(whichWindow))
				MyGrowWindow(whichWindow, myEvent->where);
			break;

		case inZoomIn:
		case inZoomOut:
			SelectWindow(whichWindow);
			if (TrackBox(whichWindow, myEvent->where, windowPart))
			{
				EraseRect(&((WindowPtr)whichWindow)->portRect);
				ZoomWindow((WindowPtr)whichWindow, windowPart, (WindowPtr)whichWindow==FrontWindow());
				MyResizeWindow(whichWindow,
						whichWindow->portRect.right - whichWindow->portRect.left,
						whichWindow->portRect.bottom - whichWindow->portRect.top);
			}
			break;

		case inContent:
			if (whichWindow != FrontWindow())
				SelectWindow(whichWindow);
			else if (ours(whichWindow))
				DoContent(whichWindow, myEvent);
			break;
	}
	return 0; // should be void fn!?
}



// ==============================================
int MaintainCursor(void)
{
	Point		pt;
	WindowPeek	wPtr;
	GrafPtr		savePort;
	
	if (ours((WindowPtr)(wPtr=(WindowPeek)FrontWindow()))) {
		GetPort(&savePort);
		SetPort((GrafPtr)wPtr);
		GetMouse(&pt);
		if (PtInRect(pt, &(**gSrcWind_TEH).viewRect ) )
			SetCursor( &gEditCursor);
		else
			SetCursor(&qd.arrow);
		SetPort(savePort);
	}
	return 0; // should be void fn!?
}


// ==============================================
#if defined(_DO_PRINTING_SOMEDAY_)

#define topMargin 20
#define leftMargin 20
#define bottomMargin 20
#define tabChar	((char)'\t')


static	THPrint	hPrint = NULL;
static	int		tabWidth;


	/**
	 **		Prototypes for private functions.
	 **		(They really should be static.)
	 **
	 **/
 
int	CheckPrintHandle(void);
int MyDrawText(char *p, int count);
int PrDoc(char **hText, long count, THPrint hPrint, int font, int size);
int HowMany(void);


// ==============================================
PleaseWait()
{
	SetCursor(&gWaitCursor);
}


// ==============================================
CheckPrintHandle()
{
	if (hPrint==NULL) 
		PrintDefault(hPrint = (TPrint **) NewHandle(sizeof( TPrint)));
}


// ==============================================
DoPageSetUp()
{
	PrOpen();
	CheckPrintHandle();
	if (PrStlDialog(hPrint)) ;
	PrClose();
}


// ==============================================
MyDrawText(char	*p, int count)
{
	register char	*p1, *p2;
	int				len;
	Point			pt;

	p1 = p;
	p2 = p+count;
	while (p<p2) {
		while ((p1<p2) && (*p1 !=tabChar)) *p1++;
		if ((len=p1-p)>0) DrawText(p, 0, p1-p);
		if (*p1==tabChar) {
			GetPen(&pt);
			Move((tabWidth-(pt.h-leftMargin)%tabWidth), 0);
			*p1++;
		}
		p = p1;
	}
}


// ==============================================
PrDoc(char **hText, long count, THPrint hPrint, int font, int size)
{
	register int 	line = 0;
	register int 	lastLineOnPage = 0;
	int				length;
	Rect 			printRect;
	int 			linesPerPage;
	int 			lineBase;
	int 			lineHeight;
	register char 	*ptr, *p1;
	FontInfo		info;
	TPPrPort		printPort;

	printPort = PrOpenDoc(hPrint, 0L, 0L);
	SetPort((GrafPtr)printPort);
	TextFont(font);
	TextSize(size);
	printRect = (**hPrint).prInfo.rPage;
	GetFontInfo(&info);
	lineHeight = info.leading+info.ascent+info.descent;
	linesPerPage = 
		(printRect.bottom-printRect.top-topMargin-bottomMargin)/lineHeight;
	HLock(hText);
	ptr = p1 = (*hText);
	do {
		PrOpenPage(printPort, 0L);
		lastLineOnPage += linesPerPage;
		MoveTo( printRect.left+leftMargin, 
			(lineBase = printRect.top+lineHeight) );
		do {
			/* PrintLine: */
			while ((ptr<=(*hText)+count) && (*ptr++ != (char)'\r')) ;
			if ((length=(int)(ptr-p1)-1)>0)
				MyDrawText(p1, length);
			MoveTo( printRect.left+leftMargin, (lineBase += lineHeight));
			p1 = ptr;
		} while ((++line != lastLineOnPage) && (ptr<(*hText)+count));
		PrClosePage(printPort);
	} while (ptr<(*hText)+count);
	HUnlock(hText);
	PrCloseDoc(printPort);
}


// ==============================================
PrintText(char	**hText, long length, GrafPtr gp, int tabPixels)
{
	GrafPtr		savePort;
	TPrStatus	prStatus;
	int			copies;
	
    PrOpen();
	CheckPrintHandle();
	tabWidth = tabPixels;
	SetCursor(&qd.arrow);
	if (PrJobDialog(hPrint) != 0) {
		PleaseWait();
		GetPort(&savePort);
		for (copies=HowMany(); copies>0; copies--) {
			PrDoc (hText, length, hPrint, (*gp).txFont, (*gp).txSize);
			PrPicFile(hPrint, 0L, 0L, 0L, &prStatus);
		}
		SetPort(savePort);
	}
	PrClose();
}


// ==============================================
int HowMany(void)
{
	return( ((**hPrint).prJob.bJDocLoop==bDraftLoop) ? 
				(**hPrint).prJob.iCopies : 1 );
}

#endif // DO_PRINTING_SOMEDAY
