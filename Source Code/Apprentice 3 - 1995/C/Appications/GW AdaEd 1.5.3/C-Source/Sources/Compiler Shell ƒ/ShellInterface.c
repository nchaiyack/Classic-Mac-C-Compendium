/*****
 * ShellInterface.c
 *
 *	Routines for the window in the compiler-parts
 *
 *****/
/*
 * Copyright (C) 1985-1992  New York University
 * Copyright (C) 1994 George Washington University
 * 
 * This file is part of the GWAdaEd system, an extension of the Ada/Ed-C
 * system.  See the Ada/Ed README file for warranty (none) and distribution
 * info and also the GNU General Public License for more details.
 */

#include <string.h>
#include <stdio.h>


#include "ShellInterface.h"
#include "ShellGlobals.h"
#include "DrawTextColumn.h"
#include "AdaFileTypes.h"
#include "SpinCursor.h"
#include "VersStr.h"
#include "PStrUtilities.h"

#include "FileMgr.h"

extern	FILE 	*errfile;

static void DrawProcess(short lh, ConstStr255Param label, ConstStr255Param path);

enum	{
	kRes_Menu_Apple = 128,
	kRes_Menu_File,
	kRes_Menu_Edit
	};

enum	{
	quitItem = 1
	};


#define colTwo 82
#define border 5
//#define progressBarH (lineHeight-8)
#define kWindowWidInches	6

// Controls what lines will be drawn in the progress dlog
// (negative means don't draw).

// modified to use constants instead of variables
// when possible [Fabrizio Oddone]

enum {
lnProcName = 1,
lnVersion,
lnHeap,
lnOptions,
kTotalLines,
lineHeight = 14,
progressBarH = lineHeight - 8
};

//static	short lnEnvName = -1;

static	short lnNumLines;		// number of lines to use
static	short lnProgress;		// this is always the last one

//static	short lineHeight;		// ascent + descent + leading
static	short lnOffsetTop;		// offset from baseline to top of line
static	short lnOffsetBottom;	// offset from baseline to bottom of line


/****
 * SetUpMenus()
 *
 *	Set up the menus. Normally, we�d use a resource file, but
 *	for this example we�ll supply �hardwired� strings.
 *
 ****/

// no, no! we use resources now! :-) [Fabrizio Oddone]

void SetUpMenus(void)
{
	{
	register Handle menuBar;
	
	SetMenuBar(menuBar = GetNewMBar(kRes_Menu_Apple));
	DisposeHandle(menuBar);
	}

AppendResMenu(gAppleMenu = GetMenuHandle(kRes_Menu_Apple),'DRVR');
gFileMenu = GetMenuHandle(kRes_Menu_File);
gEditMenu = GetMenuHandle(kRes_Menu_Edit);
DrawMenuBar();
/*
InsertMenu(gAppleMenu = NewMenu(appleID, "\p\024"), 0);
InsertMenu(gFileMenu = NewMenu(fileID, "\pFile"), 0);
AppendResMenu(gAppleMenu, 'DRVR');
AppendMenu(gFileMenu, "\pQuit/Q");
DrawMenuBar();
*/
}

/****
 *  AdjustMenus()
 ****/

static void enable(MenuRef menu, short item, short ok)
{
if (ok)
	EnableItem(menu, item);
else
	DisableItem(menu, item);
}

void AdjustMenus(void)
{
enable(gFileMenu, quitItem, 1);
}

/*****
 * HandleMenu(mSelect)
 *
 *	Handle the menu selection. mSelect is what MenuSelect() and
 *	MenuKey() return: the high word is the menu ID, the low word
 *	is the menu item
 *
 *****/

void HandleMenu (long mSelect)
{
Str255		name;
GrafPtr		savePort;
short		menuID = HiWord(mSelect);
short		menuItem = LoWord(mSelect);

switch (menuID) {
	case	kRes_Menu_Apple:
		if (menuItem != 1) {
			GetPort(&savePort);
			GetMenuItemText(gAppleMenu, menuItem, name);
			OpenDeskAcc(name);
			SetPort(savePort);
			}
		break;
	case	kRes_Menu_File:
		switch (menuItem) {
			case	quitItem:
				gShellQuitting = true;
				break;
			}
		break;
	case	kRes_Menu_Edit:
		(void) SystemEdit(menuItem - 1);
		break;
	}
HiliteMenu(0);
}
/* end HandleMenu */


/*****
 * The window routines
 *****/


/****
 * SetUpWindow()
 *
 *	Create the progress window, and open it.
 *
 ****/

void SetUpWindow(Boolean useProgress)

{
Str255  shellName;
FontInfo	finfo;
Rect	windowBounds;
short	v;
short	wid;
short	hppi, vppi;
short	lineDots;

	if (useProgress) {
		lnNumLines = kTotalLines;
		lnProgress = kTotalLines;
	}
	else {
		lnNumLines = kTotalLines - 1;
		lnProgress = -1;
	}

	ScreenRes(&hppi, &vppi);
	wid = (qd.screenBits.bounds.right - qd.screenBits.bounds.left);


	if (wid * hppi > kWindowWidInches)	// keep a limit for window width
		wid = kWindowWidInches * hppi;

//	lineHeight = 14;		// guessing
	SetRect(&windowBounds, 20, 40, wid, 40 + (lineHeight * (lnNumLines+1)));

	GetPartName(shellName);
	gShellWindow = NewWindow(0L, &windowBounds, shellName, false, // invisible at first
		 noGrowDocProc, (WindowPtr) -1L, false, 0);

	SetPort(gShellWindow);		// set font to use�
	TextFont(geneva);
	TextSize(9);

	GetFontInfo(&finfo);		// and calculate line info
	lineDots = finfo.ascent + finfo.descent + finfo.leading;
	lnOffsetTop = finfo.ascent;
	lnOffsetBottom = finfo.descent + finfo.leading;
	SizeWindow(gShellWindow, wid, (lineHeight * (lnNumLines+1)), false);
	ShowWindow(gShellWindow);

	if (lnProgress > 0) {
		v = (lineHeight * lnProgress) - progressBarH;
		SetRect(&gProgressBar, colTwo + 4, v, colTwo + 4 + kProgressBarWidth, v + progressBarH);

		gProgressArea = gProgressBar;
		InsetRect(&gProgressArea, -2, -2);
	}
}


/*****
 * DrawWContents()
 *
 *	Draws the window content
 *
 *****/

static void CalcLineBox(short lh, Rect *r)
{
r->left = border;
r->right = gShellWindow->portRect.right;
r->top = lh - lnOffsetTop;
r->bottom = lh + lnOffsetBottom;
}

static void EraseLineBox(short lh)
{
Rect r;

CalcLineBox(lh, &r);
EraseRect(&r);
}

static void DrawMemory(void)
{
// modified so that we make a single FreeMem() call
// use static variables only when needed
// and show temporary memory free [Fabrizio Oddone]
Str255	num1;
Str63	tempFreeS;
Rect	r;
Point	pt;
long	temp;
unsigned short	columnWidth;

if (lnHeap > 0) {
	Size	newFreeMem;
	Size	newTempFreeMem;
	static Size	oldFreeMem = -1;
	static Size	oldTempFreeMem = -1;
	
	newFreeMem = FreeMem();
	newTempFreeMem = TempFreeMem();
	if (newFreeMem != oldFreeMem || newTempFreeMem != oldTempFreeMem) {
		oldFreeMem = newFreeMem;
		oldTempFreeMem = newTempFreeMem;
		NumToString(newFreeMem, num1);
		(void) PLstrcat(num1, "\p/");
		NumToString(newTempFreeMem, tempFreeS);
		(void) PLstrcat(num1, tempFreeS);
		temp = lnHeap * lineHeight;
 		CalcLineBox(temp, &r);
 		r.left = colTwo;
 		EraseRect(&r);
		MoveTo(colTwo, temp);
		DrawString("\p: ");

		GetPen(&pt);
		columnWidth = gShellWindow->portRect.right - pt.h - border;
		DrawStringColumn(num1, columnWidth);
		}
	}
}

static void DrawProcess(short lh, ConstStr255Param label, ConstStr255Param path)
{
Point pt;
unsigned short columnWidth;

if (lh >= 0) {	// negative means don't draw this one
	MoveTo(border, lh);
	TextFace(bold);
	DrawString(label);
	TextFace(normal);
	GetPen(&pt);
	MoveTo(colTwo, pt.v);
	DrawString("\p: ");
	GetPen(&pt);
	columnWidth = gShellWindow->portRect.right - pt.h - border;
	DrawStringColumn(path, columnWidth);
	}
}

void DrawWContents(short /*active*/)
{
// modified in order to use less stack space [Fabrizio Oddone]

//short i;
//Point pt;
Str255 tempS;

SetPort(gShellWindow);

//GetCompilerPath(partPath);
GetPartName(tempS);
DrawProcess(lnProcName * lineHeight, "\pProcess", tempS);
GetVersStr(tempS);
DrawProcess(lnVersion * lineHeight, "\pVersion", tempS);
//DrawProcess(lnEnvName * lineHeight, "\pCalled by", partName);
DrawProcess(lnOptions * lineHeight, "\pOptions", (ConstStr255Param)gCmdLineOptions);
DrawProcess(lnHeap * lineHeight, "\pFree Heap", "\p");

// if processing then draw the grow bar stuff
if (lnProgress > 0) {
	if (gProcessing) {
		MoveTo(border, lnProgress * lineHeight);
		TextFace(bold);
		DrawString("\pProgress");
		TextFace(normal);
		FrameRect(&gProgressArea);
		}
	}
}	/* end DrawWContents */

/****
 * ProgressBar()
 *
 *		This routine updates the progress bar.
 *
 *****/

void ProgressBar(unsigned short progress)
{
// If we are still doing some processing, then show some
// kind of progress indicator.
// changed not to waste CPU time [Fabrizio Oddone]

static short	previousValue = 0;

if (previousValue != progress) {
	if ((lnProgress > 0) && (gProcessing)) {
		Rect area = gProgressBar;
	
		// update indicator here
		SetPort(gShellWindow);
		if (progress < kProgressBarWidth) {
			unsigned long	templong = (area.right - area.left) * progress;
			templong /= kProgressBarWidth;	// this mess in order to shift instead of divide [Fabrizio Oddone]
			area.right = area.left + templong;
			}
		FillRect(&area, &qd.gray);
		}
	DrawMemory();
	previousValue = progress;
	}

// if we want to show memory information, then draw free mem and
// free stack space
}



/****
 *	InternalError()
 *
 * Display alert message, write error results
 * to results file, and call the longjmp.  This routine never
 * returns!
 *
 ****/

void InternalError(ErrMsgs msgs, int errCode, char *file, int line)
{
Str255 str[4];
short i;

// Setup parameters
for (i = 0; i < 4; i++) {
	if (msgs[i].id > 0)
		GetIndString(str[i], 20000, msgs[i].id);
	else if (msgs[i].msg == NULL)
		StrLength(str[i]) = 0;
	else {
		short len = strlen(msgs[i].msg);

		BlockMoveData(msgs[i].msg, &str[i][1], len);
		StrLength(str[i]) = len;
		}			
	}

// write error message to file
if (errfile) {
	Str255 partName;
	Str31 version;

	GetVersStr(version);
	GetPartName(partName);
// \n changed to \r which is the Macintosh return character (CR, not LF)
	fprintf(errfile, "In %#s version %#s\n", partName, version);
	fprintf(errfile, "Chaos: in %s, line %d\n", file, line);
	fprintf(errfile, "Error: %#s%#s%#s%#s\n\n", str[0], str[1], str[2], str[3]);
//		fflush(errfile);	// not necessary [Fabrizio Oddone]
	fclose(errfile);
	}

// Show error message.
ParamText(str[0], str[1], str[2], str[3]);
StopSpinning();
StopAlert(20000, NULL);

// jump back to the main program passing whatever return
// code we have here!
longjmp(gJumpEnv, errCode+10);		// add 10, see note in ShellMain.c
}

/****
 *	ShowMessage
 *
 *	Display a message using an alert box
 *
 ****/

void ShowMessage(char *msg)
{
// write error message to file
if (errfile) {
	fprintf(errfile, "%s\n", msg);
	fflush(errfile);
	}

// Show error message.
ParamText(CtoPstr(msg), "\p", "\p", "\p");
StopSpinning();
StopAlert(20000, NULL);
PtoCstr((unsigned char *)msg);
}

/****
 *	AbortMessage
 *
 *	Display a message using ShowMessage and then quit.
 *	This routine doesn't return to its caller.
 *
 ****/

void AbortMessage(char *msg, int errCode)
{
// write error message to file
if (errfile) {
	fprintf(errfile, "Aborting: \n");
//	fflush(errfile);	// not necessary [Fabrizio Oddone]
	fclose(errfile);
	}


ShowMessage(msg);

// jump back to the main program passing whatever return
// code we have here!
longjmp(gJumpEnv, errCode+10);		// add 10, see note in ShellMain.c
}

