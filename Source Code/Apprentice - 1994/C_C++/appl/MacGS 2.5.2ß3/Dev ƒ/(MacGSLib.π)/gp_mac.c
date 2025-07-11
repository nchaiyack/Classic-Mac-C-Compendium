/* Copyright (C) 1989, 1990, 1991 Aladdin Enterprises.  All rights reserved.
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

/* gp_mac.c */
/* Macintosh-specific routines for Ghostscript */

#include <GestaltEqu.h>
#include <Traps.h>
#include "memory_.h"
#include "string_.h"
#include "gx.h"
#include "gp.h"
#include "gserrors.h"
#include "gmacprefs.h"
#include <console.h>


/* Globals */

Boolean		gWNEPresent;		//	WaitNextEvent present
Boolean		gHasColorQD;		//	32-Bit QD present
Boolean		gSys7SFDlog;		//	System 7 SF Dialog present
Boolean		gHasFindFolder;		//	System 7 FindFolder present
WindowPtr	gConsoleWinPtr;		//	console window pointer
Point		gSFWhere = {-1, -1};
char		gSFFName[32];


/* Do platform-dependent initialization */

	void
gp_init (void)

{
	extern char    *gs_lib_default_path;
	extern char    *gs_init_file;
	Str255			string;
	MacPrefsH		hPrefs = (MacPrefsH) GetResource (MACPREFS_RES_TYPE, MACPREFS_RES_ID);
	char			fFlags = 0;
#ifdef __GS_ARGC_ARGV__
	int			 	argc;
	char		  **argv;
#endif
	void			InitGlobals (void);
	void			gp_macui_open (void);


	SetCurrentA5 ();
	InitGlobals ();

	GetIndString (string, MACSTRS_RES_ID, iConsoleWinTitleStr);

	if (ResError () == noErr)
	{
		char   *pStr = gs_malloc (1, (size_t) (string[0] + 1), "gp_init");


		BlockMove ((Ptr) string, (Ptr) pStr, (Size) (string[0] + 1));

		console_options.title = (unsigned char *) pStr;
	}

	console_options.pause_atexit = 0;

	if (hPrefs /* != (MacPrefsH) NULL */)
	{
		fFlags = (*hPrefs)->flags;

		if (fFlags & iUseConsRes)
		{
			console_options.nrows = (*hPrefs)->consoleRows;
			console_options.ncols = (*hPrefs)->consoleCols;
		}

		if (fFlags & iExitPause)
			console_options.pause_atexit = 1;

		ReleaseResource ((Handle) hPrefs);
	}

	__open_std ();
	gConsoleWinPtr = FrontWindow ();

#ifdef __GS_ARGC_ARGV__
	if (fFlags & iUseCLI)
		argc = ccommand (&argv);
#endif

	/*...Initialize Ghostscript's default library paths and initialization file...*/

	{
		int			i;
		char	  **p;


		for (i = iGSLibPathStr, p = &gs_lib_default_path;
			 i <= iGSInitFileStr;
			 i++, p = &gs_init_file)
		{
			GetIndString (string, MACSTRS_RES_ID, i);
			(void) PtoCstr (string);
			*p = gs_malloc (1, (size_t) (strlen ((char *) string) + 1), "gp_init");
			strcpy (*p, (char *) string);
		}
	}

	gp_macui_open ();
}


/* Do platform-dependent cleanup. */

/* It sure would be nice if Peter changed gp_finit () to have the exit
 * code as its formal parameter and pass the code down to this routine.
 * This would allow gp_exit () to change console.pause_at_exit if
 * something untoward happened during execution; otherwise we *might*
 * lose germane error messages if the console window is summarily destroyed.
 */

	void
gp_exit (void)

{
}


/* Read the current date (in days since Jan. 1, 1980) */
/* and time (in milliseconds since midnight). */

	void
gp_get_clock (long *pdt)

{
	long				secs;
	DateTimeRec			dateRec;
	static DateTimeRec	baseDateRec = {1980, 1, 1, 0, 0, 0, 1};
	long				pdtmp[2];
	void 				do_get_clock (DateTimeRec *dateRec, long *pdt);


	GetDateTime ((unsigned long *) &secs);
	Secs2Date (secs, &dateRec);

	do_get_clock (&dateRec	  , pdt);
	do_get_clock (&baseDateRec, pdtmp);

	/* If the date is reasonable, subtract the days since Jan. 1, 1980 */

	if (pdtmp[0] < pdt[0])
		pdt[0] -= pdtmp[0];

#ifdef DEBUG_CLOCK
	printf("pdt[0] = %ld  pdt[1] = %ld\n", pdt[0], pdt[1]);
#endif
}


	static void
do_get_clock (DateTimeRec *dateRec, long *pdt)

{
	long 	idate;
	static int mstart[12] =
	   {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334};


	idate = (long) dateRec->year * 365 +
		(dateRec->year / 4 + 1 +		/* account for leap years */
		 mstart[dateRec->month - 1] +	/* month is 1-origin */
		 dateRec->day - 1);				/* day of month is 1-origin */
	if (dateRec->month <= 2 && dateRec->year % 4 == 0)		/* Jan. or Feb. of leap year */
		idate--;

	pdt[0] = idate;
	pdt[1] = (dateRec->hour * 60L + dateRec->minute) * 60000L + dateRec->second * 1000L;
}


/* ------ Screen management ------ */

/* Write a string to the console. */

	void
gp_console_puts (const char *str, uint size)

{
	extern void gp_macui_console_puts (void);


	gp_macui_console_puts ();

	fwrite (str, 1, size, stdout);
}

/* Make the console current on the screen. */

	int
gp_make_console_current (gx_device *dev)

{
	return 0;
}

/* Make the graphics current on the screen. */

	int
gp_make_graphics_current (gx_device *dev)
{
	return 0;
}


/* ------ Printer accessing ------ */

/* These should NEVER be called. */

/* Open a connection to a printer.  A null file name means use the */
/* standard printer connected to the machine, if any. */
/* "|command" opens an output pipe. */
/* Return NULL if the connection could not be opened. */

	FILE *
gp_open_printer (char *fname)

{	
	return
	  (strlen (fname) == 0
	  	  ? gp_open_scratch_file (gp_scratch_file_name_prefix, fname, "wb")
	  	  : fopen (fname, "wb"));
}

/* Close the connection to the printer. */

	void
gp_close_printer (FILE *pfile, const char *fname)

{
	fclose (pfile);
}


/* ------ File name syntax ------ */

/* Define the character used for separating file names in a list. */

const char gp_file_name_list_separator = ',';

/* Define the default scratch file name prefix. */

const char gp_scratch_file_name_prefix[] = "tempgs_";

/* Define whether case is insignificant in file names. */

const int gp_file_names_ignore_case = 1;

/* Define the string to be concatenated with the file mode */
/* for opening files without end-of-line conversion. */

const char gp_fmode_binary_suffix[] = "b";

/* Define the file modes for binary reading or writing. */

const char gp_fmode_rb[] = "rb";
const char gp_fmode_wb[] = "wb";


/* Create and open a scratch file with a given name prefix. */
/* Write the actual file name at fname. */

	FILE *
gp_open_scratch_file (const char *prefix, char *fname, const char *mode)

{
	FILE  *pFile;


	strcpy (fname, (char *) prefix);
	{
		char newName[50];


		tmpnam (newName);
		strcat (fname, newName);
	}

	if ((pFile = fopen (fname, mode)) /* != (FILE *) NULL */)
	{
#if 0
		pFile->remove = 1;
#endif
	}

	return pFile;
}


/* Answer whether a file name contains a directory/device specification, */
/* i.e. is absolute (not directory- or device-relative). */

	int
gp_file_name_is_absolute (const char *fname, register uint len)

{
	if (len /* > 0 */)
	{
		if (*fname == ':')
		{
			return 0;
		}
		else
		{
			register char  *p;
			register char	lastWasColon;
			register char	sawColon;


			for (len, p = (char *) fname, lastWasColon = 0, sawColon = 0;
				 len /* > 0 */;
				 len--, p++)
			{
				if (*p == ':')
				{
					sawColon = 1;

					if (lastWasColon /* != 0 */)
					{
						return 0;
					}
					else
					{
						lastWasColon = 1;
					}
				}
				else
				{
					lastWasColon = 0;
				}
			}

			return sawColon;
		}
	}
	else
	{
		return 0;
	}
}

/* Answer the string to be used for combining a directory/device prefix */
/* with a base file name.  The file name is known to not be absolute. */

	const char *
gp_file_name_concat_string (const char *prefix, uint plen, const char *fname, uint len)

{
	if ( plen > 0 && prefix[plen - 1] == ':' )
		return "";
	return ":";
}


/* ------ File operations ------ */

/* If the file given by fname exists, fill in its status and return 1; */
/* otherwise return 0. */

	int
gp_file_status (const char *fname, file_status *pstatus)

{
#if 0
	struct stat sbuf;


	pstatus->size_pages		 = (sbuf.st_size + 511L) >> 9;
	pstatus->size_bytes		 = sbuf.st_size;
	pstatus->time_referenced = sbuf.st_mtime;
	pstatus->time_created	 = sbuf.st_ctime;
#endif

	return 1;
}


/* ------ File enumeration ------ */

/****** THIS IS NOT SUPPORTED ON MACINTOSH SYSTEMS. ******/

struct file_enum_s {
	char *pattern;
	int first_time;
	gs_memory_procs mprocs;
};

/* Initialize an enumeration.  NEEDS WORK ON HANDLING * ? \. */

	file_enum *
gp_enumerate_files_init (const char *pat, uint patlen,
						 proc_alloc_t palloc, proc_free_t pfree)

{	file_enum *pfen = (file_enum *)(*palloc)(1, (size_t) sizeof(file_enum), "gp_enumerate_files");
	char *pattern;
	if ( pfen == 0 ) return 0;
	pattern = (*palloc)(patlen + 1, (size_t) 1,
			    "gp_enumerate_files(pattern)");
	if ( pattern == 0 ) return 0;
	memcpy(pattern, pat, patlen);
	pattern[patlen] = 0;
	pfen->pattern = pattern;
	pfen->mprocs.alloc = palloc;
	pfen->mprocs.free = pfree;
	pfen->first_time = 1;
	return pfen;
}

/* Enumerate the next file. */

	uint
gp_enumerate_files_next (file_enum *pfen, char *ptr, uint maxlen)

{	if ( pfen->first_time )
	   {	pfen->first_time = 0;
	   }
	return -1;
}

/* Clean up the file enumeration. */

	void
gp_enumerate_files_close (file_enum *pfen)

{	proc_free_t pfree = pfen->mprocs.free;
	(*pfree)(pfen->pattern, (uint) strlen(pfen->pattern) + 1, 1,
		 "gp_enumerate_files_close(pattern)");
	(*pfree)((char *)pfen, 1, (size_t)sizeof(file_enum), "gp_enumerate_files_close");
}


static short CursorNow = 0;
static short WatchVal  = 0;
static short WatchPos  = 0;
static long	 WatchTicks;
Boolean		 gDoCheckInterrupts = FALSE;


Boolean	gp_in_check_interrupts (void);


	Boolean
gp_in_check_interrupts (void)

{
	return WatchVal > 1;
}


	short
gp_check_interrupts (void)

{
	short		retVal = FALSE;
	EventRecord theEvent;
	void		SetCursorType (short cursorID);


	SystemTask ();

	if (WatchVal == 0)
	{
		ShowCursor ();
		
		if (gDoCheckInterrupts)
			SetCursorType (watchCursor);
		
		WatchTicks = TickCount ();
		WatchPos   = 0;
	}
	
	WatchVal++;

	if (WatchVal > 0 && TickCount () - WatchTicks > 15)
	{
		WatchPos = (WatchPos + 1) % NUM_MAC_WATCH_CURS;

		if (gDoCheckInterrupts)
			SetCursorType (128 + WatchPos);

		if (GetOSEvent (keyDownMask | autoKeyMask, &theEvent) &&
			(theEvent.message & charCodeMask) == '.' &&
			(theEvent.modifiers & cmdKey) /* != 0 */)
		{
			retVal = TRUE;
		}
		else
		{			
			if (gWNEPresent)
				(void) WaitNextEvent (0, &theEvent, 0L, (RgnHandle) NULL);
		}

		WatchTicks = TickCount ();
	}

	return retVal;
}


enum
{
	iOKOutline = OK + 1,
	iStaticTextItem
};


Boolean gp_mac_alert (char *msg, short lenStr, short index);


	Boolean
gp_mac_alert (char *msg, short lenStr, short index)

{
	Boolean		retVal = FALSE;
	Str255		string;
	StringPtr	pStr   = (StringPtr) NULL;


	if (msg /* != (char *) NULL */)
	{
		if (lenStr > 255)
			lenStr = 255;
		string[0] = lenStr;
		BlockMove ((Ptr) msg, (Ptr) &string[1], (Size) lenStr);
		pStr = string;
	}
	else
	{
		if (0 < index && index < __iLastAlertStrStr__)
		{
			GetIndString (string, MACALERTSTRS_RES_ID, index);
			pStr = string;
		}
	}

	if (pStr /* != (StringPtr) NULL */)
	{
		DialogPtr dialog = GetNewDialog (MACALERTDLOG, (char *) NULL, (WindowPtr) -1);
		short	  iType;
		Handle	  hItem;
		Rect	  rect;
		short	  item;


		if (dialog /* != (DialogPtr) NULL */)
		{
			/*...Prep outline field...*/

			GetDItem (dialog, iOKOutline, &iType, &hItem, &rect);

			if (iType == userItem | itemDisable)
			{
				pascal void drawOutline (WindowPtr dialog, short item);


				SetDItem (dialog, iOKOutline, iType, (Handle) drawOutline, &rect);
			}

			ParamText (pStr, (StringPtr) NULL, (StringPtr) NULL, (StringPtr) NULL);

			ShowWindow (dialog);
			InitCursor ();
			ModalDialog ((ProcPtr) NULL, &item);
			DisposDialog (dialog);

			retVal = TRUE;
		}
	}

	return retVal;
}


short gp_VMError (char *msg, short index);


	short
gp_VMError (char *msg, short index)

{
	Str255	sFormat;
	Boolean	fDoFPrintF;


	GetIndString (sFormat, MACALERTSTRS_RES_ID, iCouldNotAllocateFmt);

	if (!(fDoFPrintF = ResError ()))
	{
		Str255	sMsg;


		sFormat[sFormat[0] + 1] = '\0';

		GetIndString (sMsg, MACALERTSTRS_RES_ID, index);

		if (!(fDoFPrintF = ResError ()))
		{
			char	string[256];


			sMsg[sMsg[0] + 1] = '\0';

			fDoFPrintF = (sprintf (string, (char *) &sFormat[1], (char *) &msg[1]) == 1) &&
						 gp_mac_alert (string, strlen (string), 0);
		}
	}

	if (fDoFPrintF)
	{
		fprintf (stderr,
				 "Could not allocate %s.  Please quit and increase MacGS' memory size\n",
				 msg);
	}

	return_error (gs_error_VMerror);
}


static Boolean		gGestaltPresent;
static SysEnvRec 	gEnvRec;


	static void
InitGlobals (void)

{
	static Boolean	fBeenHere = FALSE;
	Boolean			TrapAvailable (short trapNum);
	Boolean			HasColorQD (void);
	Boolean			HasSys7SFDlog (void);
	Boolean			HasFindFolder (void);


	if (!fBeenHere)
	{
		fBeenHere = TRUE;

		gGestaltPresent = TrapAvailable (_GestaltDispatch);
		gWNEPresent		= TrapAvailable (_WaitNextEvent);

		if (!gGestaltPresent)
			(void) SysEnvirons (1, &gEnvRec);

		gHasColorQD		= HasColorQD ();
		gSys7SFDlog		= HasSys7SFDlog ();
		gHasFindFolder	= HasFindFolder ();

		if (!gSys7SFDlog)
			SetPt (&gSFWhere, 50, 50);

		GetIndString ((StringPtr) gSFFName, MACSTRS_RES_ID, iSavePictureNameStr);
		(void) PtoCstr ((StringPtr) gSFFName);
	}
}


	static Boolean
HasColorQD (void)

{
	if (gGestaltPresent)
	{
		long	version;
		OSErr	err = Gestalt (gestaltQuickdrawVersion, &version);


		return (err == noErr && version >= gestalt32BitQD);
	}
	else
	{
		//	pre-Gestalt method
		
		return gEnvRec.hasColorQD;
	}
}


	static Boolean
HasSys7SFDlog (void)

{
	if (gGestaltPresent)
	{
		long	attrBits;
		OSErr	err = Gestalt (gestaltStandardFileAttr, &attrBits);


		return (err == noErr && (attrBits & (1L << gestaltStandardFile58)));
	}
	else
	{
		return FALSE;
	}
}


	static Boolean
HasFindFolder (void)

{
	if (gGestaltPresent)
	{
		long	attrBits;
		OSErr	err = Gestalt (gestaltFindFolderAttr, &attrBits);


		return (err == noErr && (attrBits & (1L << gestaltFindFolderPresent)));
	}
	else
	{
		return FALSE;
	}
}


/*	copied from Pascal code in Inside Mac, Volume VI, p. 3-8.	*/

TrapType	GetTrapType (short trapNum);
short		NumToolboxTraps (void);


	static short
NumToolboxTraps (void)

{
	if (NGetTrapAddress (_InitGraf, ToolTrap) == NGetTrapAddress (0xAA6E, ToolTrap))
		return 0x200;
	else
		return 0x400;
}


	static TrapType
GetTrapType (short trapNum)

{
	if ((trapNum & 0x0800) > 0)
		return ToolTrap;
	else
		return OSTrap;
}


	static Boolean
TrapAvailable (short trapNum)

{
	TrapType trapType = GetTrapType (trapNum);


	if (trapType == ToolTrap)
	{
		trapNum &= 0x07FF;

		if (trapNum >= NumToolboxTraps ())
			trapNum = _Unimplemented;
	}
	
	return (NGetTrapAddress (trapNum, trapType) != NGetTrapAddress (_Unimplemented, ToolTrap));
}


void	macForceCursorInit (void);


	void
macForceCursorInit (void)

{
	void	DoCursorType (short cursorID);


	DoCursorType (0);
	
	CursorNow = 0;
	WatchVal  = 0;
}


	static void
SetCursorType (short cursorID)

{
	if (CursorNow == cursorID)
		return;
	
	DoCursorType (cursorID);
	
	CursorNow = cursorID;
}


	static void
DoCursorType (short cursorID)

{
	extern void macInitCursor (void);


	if (cursorID /* != 0 */)
	{
		CursHandle curs = GetCursor (cursorID);


		SetCursor (*curs);
	}
	else
	{
		macInitCursor ();
	}
}


	static pascal void
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


	void
macInitCursor (void)

{
#undef _SetCursor


	asm
	{
		movea.l	(a5),a0
		pea		-108(a0)			;  arrow
		_SetCursor
	}
}
