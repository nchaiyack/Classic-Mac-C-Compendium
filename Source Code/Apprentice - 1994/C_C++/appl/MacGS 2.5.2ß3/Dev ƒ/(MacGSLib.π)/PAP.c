/*
   Routines to interface to PAP driver.  See MacTutor, Sep '86; also Jan '88.
   This routines are meant to replace the assembly language "glue" given in
   the Sep '86 article.  Refer to the article for info on how to use these
   routines.

   Original code by Sak Wathanasin (sw%kernel.co.uk or ....!mcvax!ukc!kernel!sw)
   Adapted 06MAR93 for MacGS by Martin Fong (mwfong@nisc.sri.com)

   This file (or a project built from it) can be included in any project that
   wants to talk to the PAP routines.

Synopsis:

   typedef struct
   {
   		long  systemStuff;
		char  statusStr[256];

   } PAPStatusRec, *PAPStatusPtr;
  
   pascal short PAPOpen (refNum, printerName, flowQuantum, statusBuf, compState)
	short		   *refNum;
	char		   *printerName;
	short			flowQuantum;
	PAPStatusPtr	statusBuf;
 
   pascal short PAPRead (refNum, buffer, length, eof, compState)
	short			refNum;
	char		   *buffer;
	short		   *length;
	short		   *eol;
	short		   *compState;

   pascal short PAPWrite (refNum, buffer, length, eof, compState)
	short			refNum;
	char		   *buffer;
	short			length;
	short			eol;
	short		   *compState;
 
   pascal short PAPClose (refNum)
	short			refNum;

	typedef struct AddrBlock
	{
		short		aNet;
		Byte		aNode;
		Byte		aSocket;

	} AddrBlock;

   pascal short PAPStatus (printerName, statusBuff, netAddr)
	char		   *printerName;
	PAPStatusPtr	statusBuff;
	AddrBlock	   *netAddr;
 
   OSErr PAPLoad   (StringPtr *pLWName, StringPtr *pLWType, StringPtr *pLWZone)

   OSErr PAPUnload (void)

*/


#include "PAP.h"
#include <Folders.h>


#define	NIL					0L


extern Boolean		gHasFindFolder;		//	System 7 FindFolder present


static Handle		hPAP = (Handle) NIL;
static Ptr			papPtr;			 	/* points to locked PAP mgr */
static StringHandle	hLWName;			/* handle to locked entity name */
static StringPtr	lwNameP;
static StringPtr	lwTypeP;
static StringPtr	lwZoneP;
static StringHandle	hDriverFName;		/* handle to printer driver name */


static OSErr	GetPAPDriver (Handle *pDriverCode,
							  StringHandle *pDriverFName, StringHandle *pLWName);


	OSErr
PAPLoad (StringPtr *pLWName, StringPtr *pLWType, StringPtr *pLWZone)

{
	OSErr	status = noErr;


	if (hPAP == (Handle) NIL)
	{
		if ((status = GetPAPDriver (&hPAP, &hDriverFName, &hLWName)) == noErr)
		{
			papPtr	= *hPAP;

			lwNameP = *hLWName;
			lwTypeP = lwNameP + (short) lwNameP[0] + 1;
			lwZoneP = lwTypeP + (short) lwTypeP[0] + 1;
		}
	}

	if (status == noErr)
	{
		*pLWName = lwNameP;
		*pLWType = lwTypeP;
		*pLWZone = lwZoneP;
	}

	return status;
}


	pascal short
PAPOpen (short *refNum, char *printerName, short flowQuantum, PAPStatusPtr statusBuf)

{
	asm
	{
		move.l  papPtr, A0
		jmp		0(A0)
	}
}


	pascal short
PAPRead (short refNum, char *buffer, short *length, short *eol, short *compState)

{
	asm
	{
		move.l  papPtr, A0
		jmp		4(A0)
	}
}


	pascal short
PAPWrite (short refNum, char *buffer, short length, short eol, short *compState)

{
	asm
	{
		move.l  papPtr, A0
		jmp		8(A0)
	}
}


	pascal short
PAPStatus (char *printerName, PAPStatusPtr statusBuff, AddrBlock *netAddr)

{
	asm
	{
		move.l  papPtr, A0
		jmp		12(A0)
	}
}


	pascal short
PAPClose (short refNum)

{
	asm
	{
		move.l  papPtr, A0
		jmp		16(A0)
	}
}


/* The PAP driver installs several VBL tasks that must be removed
   using the PAP unload call before disposing of the PAP code
*/

	OSErr
PAPUnload (void)

{
	OSErr   status = noErr;


	if (hPAP /* != (Handle) NIL */)
	{
		asm
		{
			subq.l  #2, sp
			move.l  papPtr, A0
			jsr		20(A0)
			move.w  (sp)+, status	  /* save the result from the PAP call */
		}

		/* Can PAPUnload ever return an error?  We assume not... */

		DisposHandle ((Handle) hLWName);		/* junk the LW name				*/
		DisposHandle ((Handle) hPAP);			/* junk the PAP driver			*/
		DisposHandle ((Handle) hDriverFName);	/* junk the printer driver name	*/

		hPAP = (Handle) NIL;
	}

	return status;
}


/*
 *	OSErr GetPAPDriver (Handle *pDriverCode, StringHandle *pLWName)
 *
 *	returns noErr if able to locate a LaserWriter driver, else an OS error.
 *	In the former case, the locked PAP driver code and printer name are
 *	also returned.  In the latter case, GetPAPDriver () may also return the
 *	following:
 *
 *		noPrinterChosenErr		no printer was selected in the Chooser
 *		printerNotFoundErr		printer file not found
 *		notPAPPrinterErr		printer does not support PAP
 *
 *	Adapted from DTS' October 1992 "Print Access Protocol Q&As" in the
 *	"New Technical Notes".
 *
 */

	static OSErr
GetPAPDriver (Handle *pDriverCode, StringHandle *pDriverFName, StringHandle *pLWName)

{
	Str255			driverFName;
	short			theResFile;
	StringHandle	hDriverFName;
	OSErr			status = noErr;


	/*...Obtain the Chooser printer driver filename from the system...*/

	if ((hDriverFName = ChooserPrinterName ()) == (StringHandle) NIL)
	{
		status = noPrinterChosenErr;
		goto premature_exit;
	}

	/*...Open the printer driver file...*/

	if ((theResFile = OpenPrinterFile (hDriverFName)) == -1)
	{
		status = printerNotFoundErr;
		DisposHandle ((Handle) hDriverFName);

		goto premature_exit;
	}

	if ((status = ResError ()) == noErr)
	{
		Handle			hDriverCode;
		StringHandle	LWNameH;


		/*...Get the printer driver code...*/

		if ((hDriverCode = GetResource ('PDEF', 10)) == (Handle) NULL)
		{
			status = notPAPPrinterErr;
			DisposHandle ((Handle) hDriverFName);

			goto premature_exit;
		}

		/*...Relocate the driver code to the top of the application heap
		 *	 and lock it.
		 */

		DetachResource (hDriverCode);
		MoveHHi (hDriverCode);
		HLock (hDriverCode);
		HNoPurge (hDriverCode);

		/*...Now get the printer entity name from the printer driver...*/

		if ((LWNameH = (StringHandle) GetResource ('PAPA', -8192))
				== (StringHandle) NULL)
		{
			status = ResError ();
			DisposHandle ((Handle) hDriverFName);
			DisposHandle (hDriverCode);

			goto premature_exit;
		}

		/*...Relocate the printer entity name to the top of the application heap
		 *	 and lock it.
		 */

		DetachResource ((Handle) LWNameH);
		MoveHHi ((Handle) LWNameH);
		HLock ((Handle) LWNameH);
		CloseResFile (theResFile);

		/*...Return the locked code driver and printer entity name...*/

		*pDriverCode  = hDriverCode;
		*pDriverFName = hDriverFName;
		*pLWName	  = LWNameH;
	}

premature_exit:

	return status;
}


/*
 *	StringHandle ChooserPrinterName (void)
 *
 *	Returns Chooser-selected printer name or (StringHandle) NULL if
 *	there is no current printer selection.  It is the responsibility
 *	of the caller to dispose of the returned handle.
 *
 */

	StringHandle
ChooserPrinterName (void)

{
	StringHandle	hDriverFName = (StringHandle) GetResource ('STR ', -8192);


	if (hDriverFName /* != (StringHandle) NULL */)
		DetachResource ((Handle) hDriverFName);

	return hDriverFName;
}


/*
 *	short OpenPrinterFile (StringHandle hDriverFName)
 *
 *	Given a handle to the name of the current Chooser printer, returns a reference
 *	number to the corresponding printer driver file, or -1 if it can't be opened.
 *
 */

	short
OpenPrinterFile (StringHandle hDriverFName)

{
	short	sysVRefNum;
	long	sysDirID	= 0L;
	OSErr	status		= noErr;
	short	theResFile	= -1;


	/*...The 7.x Way!...*/

	if (gHasFindFolder)
	{
		status = FindFolder (kOnSystemDisk, kExtensionFolderType,
							 kDontCreateFolder, &sysVRefNum, &sysDirID);
	}

	/*...If FindFolder was not available, use good ol' SysEnvirons (thanks Jim)...*/

	else
	{
	 	SysEnvRec	theWorld;


		if ((status = SysEnvirons (1, &theWorld)) == noErr)
			sysVRefNum = theWorld.sysVRefNum;
	}

	/*...Okay, at this point we should have found the folder where the drivers
	 *	 are.  If we are running under System 7.x, this folder will be the
	 *	 extensions folder in the System folder.  If we are pre-7.x, this will
	 *	 be the System folder.  If FindFolder was available, then we have to
	 *	 use HOpenResFile because FindFolder requires dirIDs.  If we used
	 *	 SysEnvirons (i.e., pre-7.x), then we need to use OpenRFPerm.
	 */

	if (status == noErr)
	{
		HLock ((Handle) hDriverFName);

		if (gHasFindFolder)
			theResFile = HOpenResFile (sysVRefNum, sysDirID, *hDriverFName, fsRdPerm);
		else
			theResFile = OpenRFPerm (*hDriverFName, sysVRefNum, fsRdPerm);

		HUnlock ((Handle) hDriverFName);
	}

	return theResFile;
}
