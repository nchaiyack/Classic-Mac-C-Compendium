/*
==============================================================================
Project:	POV-Ray

Version:	2.2

File Name:	SaveCmpPict.h

Description:
	Routines for saving a PICT file in QuickTime compressed format.
	
	This source code was written with a lot of help from some examples
	off of Apple's QuickTime 1.0 Developer CD.

	Note that this source requires the as-yet non-standard header files
	and glue files for QuickTime and the Standard Compression dialog.
	These files will need to accompany this source until Apple rolls them
	into MPW/Think.  The additional files needed are listed below:
	Components.h			- MPW/Think - QuickTime Component header
	ImageCompression.h		- MPW/Think - QuickTime Image Compression header
	StdCompression.h		- MPW/Think - Std Compression dialog header
	StdCompression.rsrc		- MPW/Think - Std Compression dialog resource
	StdCompressionGlue.o	- MPW - Std Compression dialog glue routines
	StdCompressionGlue.�	- Think - Std Compression dialog glue routines

Related Files:
	SaveCmpPict.h: header file for Save Compressed Pict routines
	SaveCmpPict.c: main file for Save Compressed Pict routines
------------------------------------------------------------------------------
Author:
	Eduard [esp] Schwan
------------------------------------------------------------------------------
	from Persistence of Vision Raytracer
	Copyright 1993 Persistence of Vision Team
------------------------------------------------------------------------------
	NOTICE: This source code file is provided so that users may experiment
	with enhancements to POV-Ray and to port the software to platforms other 
	than those supported by the POV-Ray Team.  There are strict rules under
	which you are permitted to use this file.  The rules are in the file
	named POVLEGAL.DOC which should be distributed with this file. If 
	POVLEGAL.DOC is not available or for more info please contact the POV-Ray
	Team Coordinator by leaving a message in CompuServe's Graphics Developer's
	Forum.  The latest version of POV-Ray may be found there as well.

	This program is based on the popular DKB raytracer version 2.12.
	DKBTrace was originally written by David K. Buck.
	DKBTrace Ver 2.0-2.12 were written by David K. Buck & Aaron A. Collins.
------------------------------------------------------------------------------
More Info:
	This Macintosh version of POV-Ray was created and compiled by Jim Nitchals
	(Think 5.0) and Eduard Schwan (MPW 3.2), based (loosely) on the original
	port by Thomas Okken and David Lichtman, with some help from Glenn Sugden.

	For bug reports regarding the Macintosh version, you should contact:
	Eduard [esp] Schwan
		CompuServe: 71513,2161
		Internet: jl.tech@applelink.apple.com
		AppleLink: jl.tech
	Jim Nitchals
		Compuserve: 73117,3020
		America Online: JIMN8
		Internet: jimn8@aol.com -or- jimn8@applelink.apple.com
		AppleLink: JIMN8
------------------------------------------------------------------------------
Change History:
	920414	[esp]	Created.
	920419	[esp]	Embellished file header comments, renamed routines for consistency
	921221	[esp]	Updated includes for QuickTime 1.5 headers
	931001	[esp]	version 2.0 finished (Released on 10/4/93)
==============================================================================
*/

#if !defined(SAVECMPPICT_H)
#define SAVECMPPICT_H


/*==== POV headers ====*/
#include "PovMac.h"


/*==== Mac Toolbox Headers ====*/
#include <types.h>
#include <files.h>
#include <memory.h>
#include <qdoffscreen.h>


/*==== QuickTime compression headers ====*/
#if defined (THINK_C)
// THINK C has problems linking to StdCompressionGlue.o, fix it!
#define OpenStdCompression OPENSTDCOMPRESSION
#endif // THINK_C
#include	"ImageCompression.h"
#include	"QuickTimeComponents.h"


typedef enum {eAFI_ShrinkWholeImage, eAFI_UseCenter}  eAFI_ImagePrefs_t;



OSErr AppendFilePreview2PictF(FSSpec *fsFile);

OSErr AppendFinderIcons2PictF(FSSpec *fsFile,
						Rect *theOriginalPicFrame,
						eAFI_ImagePrefs_t theImagePrefs);

OSErr CompressPictF(ComponentInstance ci,
						FSSpec *theImageFile);


#endif // SAVECMPPICT_H
