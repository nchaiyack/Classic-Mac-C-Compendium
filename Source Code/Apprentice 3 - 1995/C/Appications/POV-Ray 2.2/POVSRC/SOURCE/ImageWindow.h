/*==============================================================================
Project:	POV-Ray

Version:	2.2

File:	ImageWindow.c

Description:
	This file contains the Macintosh Image window routines for POV-Ray.
------------------------------------------------------------------------------
Authors:
	Jim Nitchals, David Harr, Eduard [esp] Schwan
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
	930610	[esp]	Created
	931001	[esp]	version 2.0 finished (Released on 10/4/93)
==============================================================================*/

#if !defined(IMAGEWINDOW_H)
#define IMAGEWINDOW_H


/*==== POV-Ray std headers ====*/
#include "PovMac.h"

#include "TextEditor.h"		// gSrcWind_...
#include "povproto.h"


/*==== Standard C headers ====*/
#include <stdlib.h>
#include <setjmp.h>

/*==== Macintosh-specific headers ====*/
// NOTE: _H_MacHeaders_ is defined by Think C if it is using
// precompiled headers.  This is only for compilation speed improvement.
#if !defined(_H_MacHeaders_)
#include <Types.h>
#include <Controls.h>
#include <Desk.h>
#include <Dialogs.h>
#include <Files.h>
#include <Memory.h>
#include <pictutil.h>
#include <QuickDraw.h>
#include <Resources.h>
#include <Windows.h>
#include <scrap.h>
#include <Folders.h>
#include <errors.h>			/* dupFNErr, etc */
#include <fonts.h>
#include <string.h>			/* strcpy/cat */
#include <toolutils.h>		/* BitTst, etc */
#endif // _H_MacHeaders_

#if defined (applec)
#include <strings.h>		/* p2cstr */
#endif // applec


/*==== POV Mac Library routines =====*/
#include "POVLib.h"


/*==== Compress PICT routines ====*/
#include "SaveCmpPict.h"


/*==== definitions ====*/
#define	kNoAnimSuffix	-1	// for saveOutputFile()

#define SWAP_SIZE 2L*65536L	// virtual buffer swap segment size


/*==== globals ====*/
extern WindowPtr		gImageWindowPtr;		// the image window
extern Boolean			gImageWindIsValid;		// true if valid stuff in window
// set image window title name to that of source window, with
// .POV suffix replaced by .PICT.  Also follow the users
// Save As... file name changes... in version 2.x!
// extern Str63			gImageWindName;			// image window's name
extern Boolean			use_custom_palette;		// does the user desire to use color q?
extern int				gColorQuantMethod;		// what kind of color quantization?
extern Boolean			gDoingVirtualFile;		// true if doing virtual image buffering


/*==== prototypes ====*/
void	InitImageWindow(void);
void	KillImageWindow(void);
void	SetupPalettes(void);
void	SetupOffscreen(void);
void	KillOffscreen(void);
void	CloseImageWindow(void);
void	InvalRect_ImageWindow(Boolean DoWholeWindow);
void	DoResizeImageWindow(WindowPtr w, short h, short v);
void	DoGrowImageWindow(WindowPtr w, Point p);
void	SetImageWindowMag(short magMenuItem);
void	UpdateImageWindow(void);

void	make_undo(void);
void	undo_image(void);

void	revert_image(void);
void	darken_image(void);
void	lighten_image(void);
void	invert_image(void);
void	reduce_contrast(void);
void	increase_contrast(void);
void	draw_border(void);

void	open_virtual(void);
void	delete_virtual(void);
void	dispose_virtual(void);
void	swap_virtual_segment(short y);

void	display_init(int width, int height);
void	display_plot(int x, int y, unsigned char Red, unsigned char Green, unsigned char Blue);
void	display_finished(void);
void	display_close(void);

void	SetCustomPalette(Boolean doScreenUpdate);
void	DrawImageWindow(Boolean DoWholeWindow);
void	paint_to_picture(short do_disk_buffer);

void	SaveOutputFile(Boolean getName, int animSuffix, ComponentInstance ci);


#endif // IMAGEWINDOW_H


