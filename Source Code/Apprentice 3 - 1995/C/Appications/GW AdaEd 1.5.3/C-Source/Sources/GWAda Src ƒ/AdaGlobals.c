/***
 *	AdaGlobals.c
 *
 *	Global variables used in the GWAda application.
 *	We tried to keep these to a minimum. All of these
 *	variables can be made part of the CAdaApp class, but
 *	having them as globals makes the code a bit faster.
 *
 ****/
/*
 * Copyright (C) 1985-1992  New York University
 * Copyright (C) 1994 George Washington University
 * 
 * This file is part of the GWAdaEd system, an extension of the Ada/Ed-C
 * system.  See the Ada/Ed README file for warranty (none) and distribution
 * info and also the GNU General Public License for more details.
 */



#include "AdaGlobals.h"

CAdaApp		*gAdaApp;		// this is a copy of gApplication
CLibrary	*gLibrary;		// copy of instance variable in gAdaApp
CFileMgr	*gFileMgr;		// file manager, copy of instance var from gAdaApp
CSourceDocument	*gSource;	// source file that is being compiled... used
							// for opening the listing file.

