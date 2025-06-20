/****
 *	FileMgr.c
 *
 *	Class to manage files in the GWAda application.  This class
 *	is used to keep track of file information in one place.  All other
 *	parts of the system can query a global instance of this class to
 *	get information about file locations.
 *
 *	See header file for more notes.
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

// we pass FSSpecs by pointer instead of by value and
// use the Apple MoreFiles library [Fabrizio Oddone]

#ifndef __FSPCOMPAT__
#include "FSpCompat.h"
#endif
#include "FileMgr.h"
#include "GetFullPath.h"
#include "FileNames.h"
#include "PStrUtilities.h"


/* Global variables (only visible in this file) */
static	FSSpec	application;
static	FSSpec	compiler;
static	FSSpec	library;
static	FSSpec	source;

static	Str255	unitName;
static	Str255	partName;


void		IFileMgr(void)
{
StrLength(partName) = 0;
StrLength(unitName) = 0;
}

void	SetApplicationFolder(const FSSpecPtr appl)	// called at startup
{
application = *appl;
}

void	GetApplicationFolder(FSSpecPtr appl)
{
*appl = application;
}

void	GetApplicationPath(Str255 path)
{
AdaGetFullPath(&application, path);
}

void	SetCompilerFolder(const FSSpecPtr comp)	// called at startup
{
compiler = *comp;
}

void	GetCompilerFolder(FSSpecPtr comp)
{
*comp = compiler;
}

void	GetCompilerPath(Str255 path)
{
FSSpec spec;

(void) FSMakeFSSpecCompat(compiler.vRefNum, compiler.parID, "\p:", &spec);
AdaGetFullPath(&spec, path);
}

void	SetPartName(Str255 name)		// called with each command
{
PCopyString(name, partName);
}

void	GetPartName(Str255 name)
{
PCopyString(partName, name);
}


void	SetLibraryFolder(const FSSpecPtr lib)	// called when lib is opened
{
library = *lib;
}

void	GetLibraryFolder(FSSpecPtr lib)
{
*lib = library;
}

void	GetLibraryPath(Str255 path)
{
AdaGetFullPath(&library, path);
}

void	SetSource(const FSSpecPtr src)
{
source = *src;
PCopyString(source.name, unitName);
}

void	GetSource(FSSpecPtr src)
{
*src = source;
}

void	GetSourceName(Str255 name)
{
PCopyString(unitName, name);
}

void	GetSourceFolder(Str255 folder)
{
FSSpec spec;

(void) FSMakeFSSpecCompat(source.vRefNum, source.parID, "\p:", &spec);
AdaGetFullPath(&spec, folder);
}


void	SetUnitName(Str255 name)
{
PCopyString(name, unitName);
}

void	GetUnitName(Str255 name)
{
PCopyString(unitName, name);
}


void	GetOptionsFile(Str255 name)
	// CompFolder:PartName.options
{
Str255 part;

GetCompilerPath(name);
GetPartName(part);
PConcatStrings(name, part);
PConcatStrings(name, "\p.options");
}

void	GetReturnFile(Str255 name)
	// CompFolder:PartName.return
{
Str255 part;

GetCompilerPath(name);
GetPartName(part);
PConcatStrings(name, part);
PConcatStrings(name, "\p.return");
}

void	GetMsgFile(Str255 name)
	// LibFolder:SourceName.msg
{
Str255 source, nameOnly;

GetLibraryPath(name);
GetSourceName(source);

ParseFile(source, flName, nameOnly);
PConcatStrings(name, nameOnly);
PConcatStrings(name, "\p.msg");
}

void	GetLisFile(Str255 name)
	// UsrFolder:SourceName.lis
{
Str255 source, nameOnly;

GetSourceFolder(name);
GetSourceName(source);

ParseFile(source, flName, nameOnly);
PConcatStrings(name, nameOnly);
PConcatStrings(name, "\p.lis");
}

void	GetErrFile(Str255 name)
	// LibFolder:SourceName.err
{
Str255 source, nameOnly;

GetLibraryPath(name);
GetSourceName(source);

ParseFile(source, flName, nameOnly);
PConcatStrings(name, nameOnly);
PConcatStrings(name, "\p.err");
}

void	GetConsoleOutputFile(Str255 name)
	// LibFolder:SourceName.output
{
Str255 source, nameOnly;

GetLibraryPath(name);
GetSourceName(source);

ParseFile(source, flName, nameOnly);
PConcatStrings(name, "\p:");	// move up one level
PConcatStrings(name, nameOnly);
PConcatStrings(name, "\p.output");
}


void	GetDataFilePath(Str255 name)
	// LibFolder:
{
GetLibraryPath(name);
PConcatStrings(name, "\p:");	// move up one level
}



// end of file
