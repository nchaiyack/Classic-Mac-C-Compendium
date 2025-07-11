#ifndef _H_gmacprefs
#define _H_gmacprefs

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


#include "gmacres.h"


typedef struct
{
	char	flags;
	char	_fill1;

	short	winWidth;
	short	winHeight;

	short	pageWidth;
	short	pageHeight;

	short	consoleRows;
	short	consoleCols;

	OSType	fCreator;

	short	delScroll;

} **MacPrefsH;


enum _macPrefs
{
	iUseCLI		= 0x80,
	iUseWinRes  = 0x40,
	iUsePageRes = 0x20,
	iUseConsRes	= 0x10,
	iExitPause	= 0x08,

	_iLastMacPref_
};


#define MACPREFS_DEFAULT_FCREATOR	(ResType) '????'
#define MACPREFS_DEFAULT_DELSCROLL	5


#endif /* _H_gmacprefs */
