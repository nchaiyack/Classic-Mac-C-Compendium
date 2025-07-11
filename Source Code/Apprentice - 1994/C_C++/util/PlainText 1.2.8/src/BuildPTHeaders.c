/*-------------------------- Copyright Notice ---------------------------------
BuildPTHeaders.c
by Mel Park

				Copyright (c) 1993-1994 by Melburn R. Park

This code is free. It may be freely distributed and incorporated into other
programs. This code may not be subsequently copyrighted, sold, or assigned 
in any way that restricts its use by any others, including its authors.

Melburn R. Park
Dept. Anatomy & Neurobiology
University of Tennessee
Memphis, Tennessee, USA
mpark@nb.utmem.edu

------------------------------------------------------------------------------*/
/*------------------------------------------------------------------------------
	This file demonstrates how to precompile headers in MPW C. I could never 
	get the #pragma load and #pragma dump directives to work until I saw a code
	example in which the #pragma load function was performed in a separate 
	compilation.
------------------------------------------------------------------------------*/
#define __REDUMP__
#include <Types.h>
#include <Quickdraw.h>
#include <Errors.h>
#include <Fonts.h>
#include <Events.h>
#include <Controls.h>
#include <Windows.h>
#include <Menus.h>
#include <TextEdit.h>
#include <Dialogs.h>
#include <Memory.h>
#include <Folders.h>
#include <OSUtils.h>
#include <SegLoad.h>
#include <Files.h>
#include <StandardFile.h>
#include <ToolUtils.h>
#include <OSEvents.h>
#include <Resources.h>
#include <Script.h>
#include <StdLib.h>
#include <String.h>
#include <Strings.h>
#include <Packages.h>
#include <Desk.h>
#include <QDOffscreen.h>
#include <CType.h>
#include <stdio.h>
#include <Printing.h>
#include <Palettes.h>
#include <Time.h>
#include <Math.h>
#include <Lists.h>
#include <SysEqu.h>
#include <GestaltEqu.h>
#include <Printing.h>
#include <AppleEvents.h>

#include <TE32K.h>
#include <ACurs.h>

#include "PlainText.h"

#undef __REDUMP__
#pragma dump "PTHeaders"

