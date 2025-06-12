#ifndef _H_gmacres
#define	_H_gmacres

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


#define MACPREFS_RES_TYPE			'pref'

enum
{
	MACPREFS_RES_ID = 128,		/*	'pref'	*/

	MACSTRS_RES_ID = 128,		/*	'STR#'	*/
	MACPSSTRS_RES_ID,
	MACMENUSTRS_RES_ID,
	MACALERTSTRS_RES_ID,
	MACPAPALERTSTRS_RES_ID,

	MACICON_RES_ID = 128,		/*	'curs'	*/

	MACSPLASHDLOG = 128,		/*	'DLOG'	*/
	MACNOCOLORQDDLOG,
	MACALERTDLOG,
	MACSETDEVICEDLOG,
	MACSAVESELECTIONDLOG = 133,

	MACVMERROR = 132,			/*	'ALRT'	*/

	__LAST_MACSTRS_RES_ID
};


enum	/*	MACSTRS_RES_ID 'STR#'	*/
{
	iGSLibPathStr		 = 1,
	iGSInitFileStr,
	iConsoleWinTitleStr,
	iGraphicsWinTitleStr,
	iMacGSMenuNameStr,
	iSavePictureNameStr,
	iSFPutPromptStr,
	iPSExtensionStr,
	iPictExtensionStr,

	__iLastStrStr__
};


enum	/*	MACPSSTRS_RES_ID 'STR#'	*/
{
	iLaserPrepStr = 1,

	iMacRunFile,

	iShowpageOrig,
	iShowpageMac,
	iShowpageNoPrompt,

	iMacSetDevice,

	__iLastPSStrStr__
};


enum	/*	MACMENUSTRS_RES_ID 'STR#'	*/
{
	iAboutMenuStr = 1,
	iOpenFileMenuStr,
	iConfigureMenuStr,
	iLaserPrepMenuStr,
	iSavePictureMenuStr,
	iPrintMenuStr,
	iCommandMenuStr,

	iGraphicsWindowStr,

	__iLastMenuStrStr__
};


enum	/*	MACALERTSTRS_RES_ID 'STR#'	*/
{
	iMissingInitFile = 1,
	iUnableToLoadLaserPrep,

	/*	malloc () failures	*/

	iCouldNotAllocateFmt,

	iWindowCreationFailed,
	iClipRegionCreationFailed,
	iSaveRegionCreationFailed,
	iUpdRegionCreationFailed,
	iPictWinCreationFailed,
	iClippedPictCreationFailed,

	__iLastAlertStrStr__
};


enum	/*	MACPAPALERTSTRS_RES_ID 'STR#'	*/
{
	iLWNotFound,
	iLWCommInterrupted,
	iPrinterDriverNotFound6,
	iLWHasSpooler,
	iNoLWPrinterSelected,
	iNoPrinterSelected,
	iPSError,
	iMissingLWResources,
	iNotLWPrinter,
	iPrinterDriverNotFound7,

	__iLastPAPAlertStrStr__
};


#define rFSFType		(ResType) '^FSF'
#define rFSFID			0

#define NUM_MAC_WATCH_CURS		8


#endif /* _H_gmacres */
