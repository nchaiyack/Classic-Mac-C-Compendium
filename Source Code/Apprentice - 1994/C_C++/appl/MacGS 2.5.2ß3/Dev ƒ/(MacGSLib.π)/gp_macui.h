#ifndef _H_gp_macui_
#define _H_gp_macui_

/* Copyright (C) 1993 Aladdin Enterprises.  All rights reserved.
   Distributed by Free Software Foundation, Inc.

This file is part of Ghostscript.

Ghostscript is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY.  No author or distributor accepts responsibility
to anyone for the consequences of using it or for whether it serves any
particular purpose or works at all, unless he says so in writing.  Refer
to the Ghostscript General Public License for full details.

Everyone is granted permission to copy, modify and redistribute
Ghostscript, but only under the conditions described in the Ghostscript
General Public License.  A copy of this license is supposed to have been
given to you along with Ghostscript so you can know your rights and
responsibilities.  It should be in a file named COPYING.  Among other
things, the copyright notice and this notice must be preserved on all
copies.  */


typedef void (*GUIInit)			(MenuHandle hGUIMenu, short menuOffset);
typedef void (*GUINoArgFunc)	(void);
typedef void (*GUIMenu)			(short menuItem);
typedef void (*GUIActivate)		(Boolean fDoActive, CWindowPtr windowPtr);
typedef void (*GUIUpdate)		(CWindowPtr windowPtr);
typedef void (*GUIEvent)		(EventRecord *pEvent);
typedef char (*GUIOptionsProc)	(short actionValue);


typedef struct
{
	void (*DoInit)			(MenuHandle hGUIMenu, short menuOffset);
	void (*DoCursor)		(void);
	void (*DoCut)			(void);
	void (*DoCopy)			(void);
	void (*DoPaste)			(void);
	void (*DoClear)			(void);
	void (*DoMenu)			(short menuItem);
	void (*DoActivate)		(Boolean fDoActive, CWindowPtr windowPtr);
	void (*DoUpdate)		(CWindowPtr windowPtr);
	void (*DoMouseDown)		(EventRecord *pEvent);
	void (*DoKeyEvent)		(EventRecord *pEvent);
	void (*DoConsolePuts)	(void);
	void (*DoOpenFilePre)	(void);
	void (*DoOpenFilePost)	(void);
	char (*DoOptionsProc)	(short actionValue);
	void (*DoTerminate)		(void);

} GUIProcRec, *GUIProcPtr;


#define GUI_MAGIC_NUMBER	'mGUI'


enum OptionsProcEnum
{
	kNoOptions	   = 0x00,
	kHaveOptions   = 0x01,
	kIsIllegalDrvr = 0x02,

	__kLastOptionsProcEnum__
};


/* Define Macintosh GUI devices */

typedef struct gx_device_macui
{
	gx_device_common;

	long		magicNumber;	/*	GUI_MAGIC_NUMBER	*/
	GUIProcPtr	pGUIProcs;		/*	GUI interface procs	*/

} gx_device_macui;


extern short 		macOpenGUIDriver (const char *devName);
extern short		doRunIndString	(short index);
extern short		doRunString (char *pStr);
extern void			SetDoMacOpenFile (Boolean fFlag);
extern Boolean		GetDoMacOpenFile (void);
extern char		   *CfromPStr (char *pCstr, char *pPstr);
extern pascal void	drawOutline (WindowPtr dialog, short item);
extern GUIProcPtr	FindGUIProcs (const char *devName);


enum
{
	AppleMenuID = 1,
	FileMenuID,
	EditMenuID,
	MacGSMenuID = -128
};

#define GUIMenuID		MacGSMenuID

enum
{
	iUndo  = 1,
	__iEditLine1,
	iCut,
	iCopy,
	iPaste,
	iClear
};

enum
{
	iAboutMacGS = 1,
	_iMacUILine1,
	iOpenFile,
	_iMacUILine2,
	iConfigure,
	iLaserPrep,
	_iMacUILine3,

	__iLastMacGSUIMenuItem__
};

enum actionValue
{
	iInitDialogAction,
	iDoDialogAction,
	iCommitAction,
	iRetractAction,

	__iLastActionValue__
};


#endif /* _H_gp_macui_ */