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

/* gp_macui.c */
/* Macintosh command user interface for Ghostscript library */

#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <Icons.h>
#include <console.h>
#include "gx.h"				/* for gx_bitmap; includes std.h */
#include "iref.h"
#include "errors.h"
#include "oper.h"
#include "stream.h"
#include "state.h"
#include "store.h"
#include "main.h"
#include "gxdevice.h"
#include "gp_mac.h"
#include "gmacres.h"
#include "gp_macui.h"
#include "drvr.h"


/* ---------- Ghostscript Macintosh Command User Interface ---------- */

DrvrRec drvr =
	{
		0x0760, 0, 0x016A, GUIMenuID,
		OFS (drvrRTS), OFS (drvrRTS), OFS (vCtl), OFS (drvrRTS), OFS (vClose),
		"\p.gsmacui",
		0x4E75,										/*	RTS		*/
		{0x4EF9 /* JMP */, (int (*) ()) NULL},		/*	vCtl	*/
		{0x4EF9 /* JMP */, (int (*) ()) NULL}		/*	vClose	*/
	};
DrvrH	drvrH = (DrvrH) NULL;


static Handle		guiMenuBar		= (Handle) NULL;
static MenuHandle	hGUIMenu		= (MenuHandle) NULL;
static short		numGUIMenuItems = 0;
static MenuHandle	hOrigGUIMenu	= (MenuHandle) NULL;
static GUIProcPtr	pGUIProcs		= (GUIProcPtr) NULL;
static short		old_mac_pause_atexit;
static char			macuiMenuKeys[32];
static char			drvrMenuKeys[32];


#define CALLBACK(procName, args)							\
{															\
	if (pGUIProcs /* != (GUIProcPtr) NULL */ &&				\
		pGUIProcs->procName /* != (void) (*) (...) NULL */)	\
	{														\
		(*pGUIProcs->procName) args;						\
	}														\
}


#define MENUAPPENDSTR(index, menuH)							\
{															\
	GetIndString (string, MACMENUSTRS_RES_ID, index);		\
	AppendMenu (menuH, string);								\
}


void	gp_macui_open (void);


	void
gp_macui_open (void)

{
	(void) macOpenGUIDriver ((char *) NULL);
}


	void
gp_macui_console_puts (void)

{
	CALLBACK (DoConsolePuts, ());
}


/*
 *  macOpenGUIDriver -- installs and initializes the graphical user interface
 *						driver (which spoofs being a DA).
 *
 */

	short
macOpenGUIDriver (const char *devName)

{
	static short	gsMacUIRefNum = 0;
	GUIProcPtr		newGUIProcs;
	Boolean			fDoInit		  = FALSE;
	void			GetUIMenuKeys (MenuHandle hMenu, char *pKeys, short offset);
	void			AppendUIMenuKey (char cmdChar, char *pKeys);
	void			patchConsoleControl (void);


	if ((newGUIProcs = FindGUIProcs (devName)) != pGUIProcs)
	{
		if (pGUIProcs /* != (GUIProcPtr) NULL */)
		{
			CALLBACK (DoTerminate, ());

			/*...Restore the MacGS menu to its original state...*/

			if (hOrigGUIMenu /* != (MenuHandle) NULL */ &&
				numGUIMenuItems /* > 0 */ &&
				numGUIMenuItems < CountMItems (hGUIMenu))
			{
				DeleteMenu ((*hGUIMenu)->menuID);
				DisposeMenu (hGUIMenu);
				hGUIMenu = hOrigGUIMenu;
				(void) HandToHand ((Handle *) &hGUIMenu);
				InsertMenu (hGUIMenu, (*hGUIMenu)->menuID);
			}
		}

		pGUIProcs = newGUIProcs;
		fDoInit	  = TRUE;
	}

	/*...If necessary, install the MacGS GUI driver...*/

	if (gsMacUIRefNum == 0)
	{
		DCtlHandle			dceH;
		register DCtlPtr	dce;


		/*...Create driver on heap...*/

		if (!drvrH)
		{
			Str255		string;
			int			doClose (void);
			int			doPBControl (void);


			/*...Create a new menu...*/

			GetIndString (string, MACSTRS_RES_ID, iMacGSMenuNameStr);
			InsertMenu (hGUIMenu = NewMenu (GUIMenuID, string), 0);

			MENUAPPENDSTR (iAboutMenuStr	, hGUIMenu);
			MENUAPPENDSTR (iOpenFileMenuStr	, hGUIMenu);
			MENUAPPENDSTR (iConfigureMenuStr, hGUIMenu);
			MENUAPPENDSTR (iLaserPrepMenuStr, hGUIMenu);

			numGUIMenuItems = CountMItems (hGUIMenu);

			GetUIMenuKeys (hGUIMenu, macuiMenuKeys, 0);
			AppendUIMenuKey ('Q', macuiMenuKeys);		/*	preempt Cmd-Q	*/

			hOrigGUIMenu = hGUIMenu;
			(void) HandToHand ((Handle *) &hOrigGUIMenu);

			guiMenuBar = GetMenuBar ();

			old_mac_pause_atexit = console_options.pause_atexit;

			DrawMenuBar ();

			/*...Install the driver...*/

			drvr.vCtl.vCode	  = doPBControl;
			drvr.vClose.vCode = doClose;

			asm
			{
				lea		drvr,a0
				move.l	#sizeof(drvr),d0
				_PtrToHand
				move.l	a0,drvrH
			}

			patchConsoleControl ();
		}

		/*...Find an available slot in the unit table...*/

		for (gsMacUIRefNum = 27; dceH = UTableBase[gsMacUIRefNum]; gsMacUIRefNum++)
		{
			if (!((*dceH)->dCtlFlags & dOpened))
				break;
		}
		gsMacUIRefNum = ~gsMacUIRefNum;

		/*...Create DCE...*/

		asm
		{
			move.w	gsMacUIRefNum,d0
			dc.w	0xA13D			;  _DrvrInstall
			movea.l	(a0),dce
		}

		dce->dCtlDriver = (Ptr) drvrH;
		dce->dCtlFlags	= drvr.drvrFlags;
		dce->dCtlEMask	= drvr.drvrEMask;
		dce->dCtlMenu	= drvr.drvrMenu;
	}

	if (fDoInit)
	{
		CALLBACK (DoInit, (hGUIMenu, numGUIMenuItems));
		GetUIMenuKeys (hGUIMenu, drvrMenuKeys, numGUIMenuItems);

		DrawMenuBar ();
	}

	return gsMacUIRefNum;
}


/*
 *  doClose -- handles _PBClose call
 *
 *  Normally, we shouldn't get a close call.  If we do reach here,
 *	we can keep from crashing (except on 64K ROMs) by refusing to close.
 *
 */

	static int
doClose (void)

{
	return closErr;
}


/*
 *  doPBControl -- handle _PBControl call
 *
 *	Note that this driver spoofs being a desk accessory because the "real" event
 *	loop is in <console.c>, which, for obvious reasons, we don't want to modify.
 *
 */

	static int
doPBControl (void)

{
	register CntrlParam	   *pb;
	DCtlPtr					dce;
	long					oldA5 = SetCurrentA5 ();


	asm
	{
		move.l	a0,pb
		move.l	a1,dce
	}

	switch (pb->csCode)
	{
		case accMenu:
		{
			void	doMenu (short menuID, short menuItem);


			doMenu (pb->csParam[0], pb->csParam[1]);

			break;
		}

		case accCursor:
		{
			CALLBACK (DoCursor, ());

			break;
		}

		case accCut:
		{
			CALLBACK (DoCut, ());

			break;
		}

		case accCopy:
		{
			CALLBACK (DoCopy, ());

			break;
		}

		case accPaste:
		{
			CALLBACK (DoPaste, ());

			break;
		}

		case accClear:
		{
			CALLBACK (DoClear, ());

			break;
		}

		case accEvent:
		{
			register EventRecord   *pEvent = *(EventRecord **) pb->csParam;
			void					doEvent (EventRecord *pEvent);


			doEvent (pEvent);

			break;
		}

		default:
		{
#if 0
			short	csCode = pb->csCode;


			DebugStr ("\pdoPBControl bad csCode");
			asm
			{
				clr.l	d0
				move.w	csCode,d0
			}
#endif

			break;
		}
	}

	HUnlock ((Handle) drvrH);
	HUnlock (RecoverHandleSys ((Ptr) dce));
	SetA5 (oldA5);

	return 0;
}


enum
{
	iOKOutline = OK + 1,
	iIconPicture,
	iVersionItem
};


/*
 *	doMenu -- handles menu events
 *
 */

	static void
doMenu (short menuID, short menuItem)

{
	switch (menuID)
	{
		case AppleMenuID:
		{
			MenuHandle	appleMenu = GetMHandle (AppleMenuID);
			Str255		buf;


			GetItem (appleMenu, menuItem, buf);
			OpenDeskAcc (buf);

			break;
		}

		case FileMenuID:
		{
			extern short old_mac_pause_atexit;
			extern void	 gs_exit (int code);


			console_options.pause_atexit = old_mac_pause_atexit;

			gs_exit (0);

			/* unreachable */
		}

		case EditMenuID:
		{
			switch (menuItem)
			{
				case iCut:
					CALLBACK (DoCut, ());
					break;

				case iCopy:
					CALLBACK (DoCopy, ());
					break;

				case iPaste:
					CALLBACK (DoPaste, ());
					break;

				case iClear:
					CALLBACK (DoClear, ());
					break;
			}

			break;
		}

		default:
		{
			switch (menuItem)
			{
				case iAboutMacGS:
				{
					DialogPtr dialog = GetNewDialog (MACSPLASHDLOG, (char *) NULL,
													 (WindowPtr) -1);


					if (dialog /* != (DialogPtr) NULL */)
					{
						short	item;
						short	iType;
						Handle	hItem;
						Rect	rect;


						/*...Prep the version field...*/

						GetDItem (dialog, iVersionItem, &iType, &hItem, &rect);

						if (hItem /* != (Handle) NULL */ &&
							iType == statText | itemDisable)
						{
							Handle	hFSF = GetResource (rFSFType, rFSFID);


							if (hFSF /* != (Handle) NULL */)
							{
								HLock (hFSF);
								SetIText (hItem, (StringPtr) *hFSF);
								ReleaseResource (hFSF);
							}
						}

						/*...Prep outline field...*/

						GetDItem (dialog, iOKOutline, &iType, &hItem, &rect);

						if (iType == userItem | itemDisable)
							SetDItem (dialog, iOKOutline, iType, (Handle) drawOutline,
									  &rect);

						ShowWindow (dialog);
						ModalDialog ((ProcPtr) NULL, &item);
						DisposDialog (dialog);
					}

					break;
				}

				case iOpenFile:
				{
					void	doOpenFile (void);


					doOpenFile ();

					break;
				}

				case iConfigure:
				{
					void	DoSetDeviceDialog (void);


					DoSetDeviceDialog ();

					break;
				}

				case iLaserPrep:
				{
					void	doLoadLaserPrep (void);


					doLoadLaserPrep ();

					break;
				}

				default:
				{
					CALLBACK (DoMenu, (menuItem));
					break;
				}
			}
		}
	}

	HiliteMenu (0);
}


/*
 *	doEvent -- handles events
 *
 */

	static void
doEvent (EventRecord *pEvent)

{
	switch (pEvent->what)
	{
		case activateEvt:
		{
			CWindowPtr	windowPtr = (CWindowPtr) pEvent->message;
			Boolean		fDoActive = pEvent->modifiers & activeFlag;


			CALLBACK (DoActivate, (fDoActive, windowPtr));

			break;
		}

		case updateEvt:
		{
			CWindowPeek	pWindowPeek = (CWindowPeek) pEvent->message;


			if (!EmptyRgn (pWindowPeek->updateRgn))
			{
				SetCursor (*GetCursor (watchCursor));

				SetPort ((GrafPtr) pWindowPeek);

				BeginUpdate ((WindowPtr) pWindowPeek);
					CALLBACK (DoUpdate, ((CWindowPtr) pWindowPeek));
				EndUpdate ((WindowPtr) pWindowPeek);

				asm
				{
					movea.l	(a5),a0
					pea		-108(a0)			;  arrow
					_SetCursor
				}
			}

			break;
		}

		case mouseDown:
		{
			CALLBACK (DoMouseDown, (pEvent));

			break;
		}

		case autoKey:
		case keyDown:
		{
			void	doKeyEvent (EventRecord *pEvent);


			doKeyEvent (pEvent);

			break;
		}
	}
}


/*
 *	doKeyEvent -- handles keyboard events
 *
 */

enum
{
	iPostDoNothing,
	iPostExitToShell
};


	static void
doKeyEvent (EventRecord *pEvent)

{
	char	aChar = pEvent->message & charCodeMask;


	if ('a' <= aChar && aChar <= 'z')
	{
		aChar += 'A' - 'a';
		pEvent->message &= ~charCodeMask;
		pEvent->message |= aChar;
	}

	CALLBACK (DoKeyEvent, (pEvent));

	if (pEvent->modifiers & cmdKey)
	{
		short	flashMenuID = 0;
		char	iPostAction = iPostDoNothing;


		switch (aChar)
		{
			case 'O':
			{
				void	doOpenFile (void);


				FlashMenuBar (MacGSMenuID);
				flashMenuID = MacGSMenuID;

				doOpenFile ();

				break;
			}

			case 'L':
			{
				void	doLoadLaserPrep (void);


				FlashMenuBar (MacGSMenuID);
				flashMenuID = MacGSMenuID;

				doLoadLaserPrep ();

				break;
			}

			case 'Q':
			{
				console_options.pause_atexit = old_mac_pause_atexit;

				flashMenuID	= FileMenuID;
				iPostAction = iPostExitToShell;
				break;
			}

			default:
			{
				break;
			}
		}

		if (flashMenuID /* != 0 */)
		{
			FlashMenuBar (flashMenuID);
			if (flashMenuID > 0)
				FlashMenuBar (flashMenuID);
			HiliteMenu (0);
		}

		switch (iPostAction)
		{
			case iPostExitToShell:
			{
				doMenu (FileMenuID, 1 /* console File menu only has one item */);

				/*	unreachable	*/
			}

			default:
				break;
		}
	}
}


	pascal void
drawOutline (WindowPtr dialog, short item)

{
	short	iType;
	Handle	hItem;
	Rect	rect;
	GrafPtr	savePort;


	GetPort (&savePort);
	SetPort ((GrafPtr) dialog);
		GetDItem (dialog, item, &iType, &hItem, &rect);
		PenSize (3, 3);
		FrameRoundRect (&rect, 16, 16);
	SetPort (savePort);
}


	GUIProcPtr
FindGUIProcs (const char *devName)

{
	GUIProcPtr			retVal = (GUIProcPtr) NULL;
	gx_device		  **pDevice;
	extern	gx_device  *gx_device_list[];


	if (devName == (char *) NULL)
		return retVal;

	for (pDevice = gx_device_list; *pDevice /* != (gx_device *) NULL */; pDevice++)
	{
		if (strcmp ((*pDevice)->dname, devName) == 0)
		{
			gx_device_macui	   *pGUIDevice = (gx_device_macui *) *pDevice;


			if (pGUIDevice->magicNumber == GUI_MAGIC_NUMBER)
				retVal = pGUIDevice->pGUIProcs;

			break;
		}
	}

	return retVal;
}


	static void
GetUIMenuKeys (MenuHandle hMenu, char *pKeys, short offset)

{
	short	numMenuItems = CountMItems (hMenu);
	short	count		 = 0;
	short	i;


	for (i = offset + 1; i <= numMenuItems; i++)
	{
		short	cmdChar;


		GetItemCmd (hMenu, i, &cmdChar);
		if (cmdChar != hMenuCmd && cmdChar /* != 0 */)
		{
			count++;
			*(pKeys + count) = cmdChar;
		}
	}

	*pKeys = count;
}


	static void
AppendUIMenuKey (char cmdChar, char *pKeys)

{
	short	count = *pKeys;


	count++;
	*(pKeys + count) = cmdChar;
	*pKeys = count;
}


	static Boolean
IsUIMenuKey (char aChar, char *pKeys)

{
	short	numKeys	= *pKeys;
	short	i;
	Boolean	retVal	= FALSE;


	for (i = 1; i <= numKeys; i++)
	{
		if (*(pKeys + i) == aChar)
		{
			retVal = TRUE;
			break;
		}
	}

	return retVal;
}


static int	(*pConsolePBControl) (void);
static int	uiConsolePBControl (void);


	static void
patchConsoleControl (void)

{
	extern WindowPtr	gConsoleWinPtr;		//	console window pointer
	short				consoleRefNum = ~((WindowPeek) gConsoleWinPtr)->windowKind;
	DCtlHandle			dceH 		  = UTableBase[consoleRefNum];
	DrvrH				hConsoleDrvr  = (DrvrH) (*dceH)->dCtlDriver;


	pConsolePBControl			= (*hConsoleDrvr)->vCtl.vCode;
	(*hConsoleDrvr)->vCtl.vCode = uiConsolePBControl;
}


	static int
uiConsolePBControl (void)

{
	register CntrlParam	   *pb;
	DCtlPtr					dce;
	long					oldA5 = SetCurrentA5 ();
	int						(*pPBControl) (void);
	int						status;


	asm
	{
		move.l	a0,pb
		move.l	a1,dce
	}

	pPBControl = pConsolePBControl;

	if  (pb->csCode == accEvent)
	{
		register EventRecord   *pEvent = *(EventRecord **) pb->csParam;


		switch (pEvent->what)
		{
			case autoKey:
			case keyDown:
			{
				if (pEvent->modifiers & cmdKey)
				{
					char	aChar = pEvent->message & charCodeMask;


					if ('a' <= aChar && aChar <= 'z')
					{
						aChar += 'A' - 'a';
						pEvent->message &= ~charCodeMask;
						pEvent->message |= aChar;
					}

					if (IsUIMenuKey (aChar, macuiMenuKeys) ||
						IsUIMenuKey (aChar,	drvrMenuKeys))
					{
						pPBControl = doPBControl;
					}
				}

				break;
			}

			default:
				break;
		}
	}

	asm
	{
		move.l	pb,a0
		move.l	dce,a1
		move.l	pPBControl,a2		;	status = (*pPBControl) ();
		jsr		(a2)
		move.w	d0,status
	}

	SetA5 (oldA5);

	return status;
}


static Boolean		fDoMacOpenFile	 = FALSE;
short				doMacOpenFile (ref *pRunFile);


	void
SetDoMacOpenFile (Boolean fFlag)

{
	fDoMacOpenFile = fFlag;
}


	Boolean
GetDoMacOpenFile (void)

{
	return fDoMacOpenFile;
}


#define DEFAULT_BUFFER_SIZE		512


#define CSTR_FROM_RES(index, pCstr)					\
{													\
	Str255	string;									\
													\
													\
	GetIndString (string, MACSTRS_RES_ID, index);	\
	if (string[0] > 31)								\
		string[0] = 31;								\
	(void) CfromPStr (pCstr, (char *) string);		\
}


	short
doMacOpenFile (ref *pRunfile)

{
	short		code = e_undefinedfilename;


	if (fDoMacOpenFile)
	{
		SFTypeList	types = {'TEXT', 'EPSF', 0L, 0L};
		SFReply		reply;


		SFGetFile (gSFWhere, "\p", (ProcPtr) NULL, 2, types, (ProcPtr) NULL, &reply);

		if (reply.good)
		{
			short	oldVRefNum;
			stream *s;


			(void) CfromPStr (gSFFName, (char *) reply.fName);
			GetVol ((StringPtr) NULL, &oldVRefNum);
			SetVol ((StringPtr) NULL, reply.vRefNum);
				code = file_open (gSFFName, (uint) strlen (gSFFName), "r",
								  DEFAULT_BUFFER_SIZE, pRunfile, &s);
			SetVol ((StringPtr) NULL, oldVRefNum);

			/*...Create a save picture name template...*/

			{
				short	lenStr;
				short	lenExt;
				char	psExtension[32];
				char	pictExtension[32];
				char   *p;


				CSTR_FROM_RES (iPSExtensionStr	, psExtension  );
				CSTR_FROM_RES (iPictExtensionStr, pictExtension);

				if ((p = strstr (gSFFName, psExtension)) /* != (char *) NULL */)
					*p = '\0';

				lenStr = strlen (gSFFName);
				lenExt = strlen (pictExtension);

				if (lenStr + lenExt > 31)
				{
					if (31 - lenExt >= 0)
					{
						lenStr = 31 - lenExt;
						gSFFName[lenStr] = '\0';
					}
					else
					{
						lenExt = 31 - lenStr;
						pictExtension[lenExt] = '\0';
					}
				}

				strcat (gSFFName, pictExtension);
			}
		}
	}

	return code;
}


/*
 *	doOpenFile -- opens an input PS file for interpretation
 *
 */

	static void
doOpenFile (void)

{
	CALLBACK (DoOpenFilePre, ());
	console_options.pause_atexit = 1;

	(void) doRunIndString (iMacRunFile);

	console_options.pause_atexit = old_mac_pause_atexit;
	CALLBACK (DoOpenFilePost, ());
}


	static void
doLoadLaserPrep (void)

{
	SetCursor (*GetCursor (watchCursor));

	(void) doRunIndString (iLaserPrepStr);
}


	short
doRunIndString (short index)

{
	Str255	string;
	short	code;


	GetIndString (string, MACPSSTRS_RES_ID, index);
	if ((code = ResError ()) /* != noErr */)
		return code;

	return doRunString (PtoCstr (string));
}


	short
doRunString (char *pStr)

{
	short	user_errors = 1;
	short	code;
	int		exit_code;
	ref		error_object;


	console_options.pause_atexit = 1;

	if ((code = gs_run_string (pStr, user_errors, &exit_code, &error_object)) < 0)
	{
		extern WindowPtr	gConsoleWinPtr;		//	console window pointer


		SelectWindow (gConsoleWinPtr);

		zflush ((ref *) NULL);	/* flush stdout */

		gs_debug_dump_stack (exit_code, &error_object);
	}

	console_options.pause_atexit = old_mac_pause_atexit;

	return code;
}


	char *
CfromPStr (char *pCstr, char *pPstr)

{
    short		lenstr = pPstr[0];


	if (lenstr /* > 0 */)
		BlockMove (&(pPstr[1]), pCstr, lenstr);

	pCstr[lenstr] = '\0';

	return pCstr;
}
