# 
# ==============================================================================
# Project:	POV-Ray
#
# Version:	2.2
# 
# File:		POV-Ray.make
# 
# Contents:
# 	MPW Make file, used to generate the POV-Ray application.
#	Now works with MPW 3.0, 3.1, and 3.2.
# 
# Related Files:
# 	POV.c: Main Program source
# 	POV-Ray.¹.rsrc: what rsrcs get appended..
# ------------------------------------------------------------------------------
# Author:
#	Eduard [esp] Schwan
# ------------------------------------------------------------------------------
#	from Persistence of Vision Raytracer
#	Copyright 1993 Persistence of Vision Team
# ------------------------------------------------------------------------------
#	NOTICE: This source code file is provided so that users may experiment
#	with enhancements to POV-Ray and to port the software to platforms other 
#	than those supported by the POV-Ray Team.  There are strict rules under
#	which you are permitted to use this file.  The rules are in the file
#	named POVLEGAL.DOC which should be distributed with this file. If 
#	POVLEGAL.DOC is not available or for more info please contact the POV-Ray
#	Team Coordinator by leaving a message in CompuServe's Graphics Developer's
#	Forum.  The latest version of POV-Ray may be found there as well.
#
#	This program is based on the popular DKB raytracer version 2.12.
#	DKBTrace was originally written by David K. Buck.
#	DKBTrace Ver 2.0-2.12 were written by David K. Buck & Aaron A. Collins.
# ------------------------------------------------------------------------------
#More Info:
#	This Macintosh version of POV-Ray was created and compiled by Jim Nitchals
#	(Think 5.0) and Eduard Schwan (MPW 3.2), based (loosely) on the original
#	port by Thomas Okken and David Lichtman, with some help from Glenn Sugden.
#
#	For bug reports regarding the Macintosh version, you should contact:
#	Eduard [esp] Schwan
#		CompuServe: 71513,2161
#		Internet: jl.tech@applelink.apple.com
#		AppleLink: jl.tech
#	Jim Nitchals
#		Compuserve: 73117,3020
#		America Online: JIMN8
#		Internet: jimn8@aol.com -or- jimn8@applelink.apple.com
#		AppleLink: JIMN8
# ------------------------------------------------------------------------------
# Change History:
#	920308	[esp]	Created for POV-Ray
#	920315	[esp]	Added -b compile option
#	920330	[esp]	Did the file dependencies for real (by hand!)
#	920415	[esp]	Added QuickTime and StdCompression files
#	920429	[esp]	Changed QT & SC file placement, and changed app name to POV-Ray
#	920718	[esp]	Updated to 3.2 only
#	920902	[esp]	Tested under ETO#8, MPW 3.2.3, added TemplateMenu.c
#	920923	[esp]	Collected generic routines into a POVLib.o library for others to use
#	921202	[esp]	Removed -mf option from C, removed -FAR from C & Link
#	930425	[esp]	Broke image window stuff out to ImageWindow.c/h
#	930522	[esp]	Changed app signature from PVtm(1.0) to PvRy(2.0)
#	930605	[esp]	Changed srcpath to incpath on all .h files
#	930829	[esp]	Added the split resource files included
#	931001	[esp]	version 2.0 finished (Released on 10/4/93)
#	940204	[esp]	version 2.2 finished (altered mem alloc defines)
# ==============================================================================
# 

#------------------------------------------------------------------------------
# AppName sets the main application name for the make.
# AppSignature sets the Creator signature for the application.
#------------------------------------------------------------------------------
AppName			=	'POV-Ray'
AppSignature	=	'PvRy'


#------------------------------------------------------------------------------
# Use variables for the compiler/linker tools
#------------------------------------------------------------------------------

C		= C
Asm		= Asm
Rez		= Rez
Lib		= Lib
Link	= Link

#------------------------------------------------------------------------------
# These are the paths to prefix source and object files, & additional folders with.
#------------------------------------------------------------------------------
srcpath			=	:
corepath		=	:
rezpath			=	:
incpath			=	:
objpath			= 	:obj:

#------------------------------------------------------------------------------
# These are the include file hierarchy shorthand variables for POVLib
#------------------------------------------------------------------------------

# The POV Library file name
POVLib			= POVLib

# The headers used
h_stdiop2w = ¶
			{incpath}Stdio_p2w.h
h_FileQueue = ¶
			{incpath}FileQueue.h
h_PopupMenu = ¶
			{incpath}PopupMenu.h
h_POVMalloc = ¶
			{incpath}POVMalloc.h
h_Printf2Window = ¶
			{h_stdiop2w} ¶
			{incpath}Printf2Window.h
h_ProgressDialog = ¶
			{incpath}ProgressDialog.h
h_ScreenUtils = ¶
			{incpath}ScreenUtils.h
h_POVLib = ¶
			{h_FileQueue} ¶
			{h_PopupMenu} ¶
			{h_POVMalloc} ¶
			{h_Printf2Window} ¶
			{h_ProgressDialog} ¶
			{h_ScreenUtils}

# the objects used
o_FileQueue = ¶
			{objpath}FileQueue.o
o_PopupMenu = ¶
			{objpath}PopupMenu.o
o_POVMalloc = ¶
			{objpath}POVMalloc.o
o_Printf2Window = ¶
			{objpath}Printf2Window.o
o_ProgressDialog = ¶
			{objpath}ProgressDialog.o
o_ScreenUtils = ¶
			{objpath}ScreenUtils.o
o_POVLib = ¶
			{o_FileQueue} ¶
			{o_PopupMenu} ¶
			{o_POVMalloc} ¶
			{o_Printf2Window} ¶
			{o_ProgressDialog} ¶
			{o_ScreenUtils}

h_TextEditor = ¶
			{incpath}TextEditor.h

#------------------------------------------------------------------------------
# These are the include file hierarchy shorthand variables for the Application
#------------------------------------------------------------------------------

h_ImageWindow = ¶
		{incpath}ImageWindow.h

h_TemplateMenu = ¶
		{incpath}TemplateMenu.h

h_config = ¶
		{incpath}Config.h ¶
		{h_POVLib}

h_povmac = ¶
		{incpath}PovMac.h ¶
		{incpath}Animate.h ¶
		{h_POVLib}

h_frame = ¶
		{h_Config}

h_fp = ¶
		{h_frame} ¶
		{incpath}povproto.h ¶

h_fvp = ¶
		{h_frame} ¶
		{incpath}vector.h ¶
		{incpath}povproto.h ¶

h_fvpt = ¶
		{h_fvp} ¶
		{incpath}texture.h ¶


#------------------------------------------------------------------------------
# These are the compiled resource files that we want to link with.
# If any one of these changes, then we invoke the Rez command.
#------------------------------------------------------------------------------

rsrcFiles = ¶
		{rezpath}POV.¹.rsrc

#------------------------------------------------------------------------------
# These are the objects that we want to link with. If any one of these
# changes, then we invoke the Link command.
#------------------------------------------------------------------------------

MacObjects = ¶
		{objpath}Animate.c.o ¶
		{objpath}ImageWindow.c.o ¶
		{objpath}Pov.c.o ¶
		{objpath}SaveCmpPict.c.o ¶
		{objpath}TemplateMenu.c.o ¶
		{objpath}TextEditor.C.o ¶
		{objpath}{POVLib}.o ¶
		{libraries}StdCompressionGlue.o	# QuickTime StdCmp Dialog
		# Note that in MPW 3.3 and later, StdCompressionGlue.o is in {libraries}

POVCoreObjects = ¶
		{objpath}BEZIER.C.o ¶
		{objpath}BLOB.C.o ¶
		{objpath}BOUND.C.o ¶
		{objpath}BOXES.C.o ¶
		{objpath}CAMERA.C.o ¶
		{objpath}COLOUR.C.o ¶
		{objpath}CONES.C.o ¶
		{objpath}CSG.C.o ¶
		{objpath}DISCS.C.o ¶
		{objpath}DUMP.C.o ¶
		{objpath}EXPRESS.C.o ¶
		{objpath}GIF.C.o ¶
		{objpath}GIFDECOD.C.o ¶
		{objpath}HFIELD.C.o ¶
		{objpath}IFF.C.o ¶
		{objpath}IMAGE.C.o ¶
		{objpath}LIGHTING.C.o ¶
		{objpath}MATRICES.C.o ¶
		{objpath}NORMAL.C.o ¶
		{objpath}OBJECTS.C.o ¶
		{objpath}PARSE.C.o ¶
		{objpath}PIGMENT.C.o ¶
		{objpath}PLANES.C.o ¶
		{objpath}POINT.C.o ¶
		{objpath}POLY.C.o ¶
		{objpath}POVRAY.C.o ¶
		{objpath}QUADRICS.C.o ¶
		{objpath}RAW.C.o ¶
		{objpath}RAY.C.o ¶
		{objpath}RENDER.C.o ¶
		{objpath}SPHERES.C.o ¶
		{objpath}TARGA.C.o ¶
		{objpath}TEXTURE.C.o ¶
		{objpath}TOKENIZE.C.o ¶
		{objpath}TRIANGLE.C.o ¶
		{objpath}TXTTEST.C.o ¶
		{objpath}VECT.C.o

AppObjects = ¶
		{MacObjects} ¶
		{POVCoreObjects}


#------------------------------------------------------------------------------
# Options for our compilers:
#	-sym on: tells the compilers and linker to emit symbol information for
#		a source level debugger, such as SADE.
#	-i {ExtraIncludes}: means to look for any #include files in the specified
#		directory, as well as the normal set.
#	-r: tells the C compiler to require function prototypes.
#	-mbg off: tells the compilers to not emit low-level debugger names. This
#		saves on file space, but you may wish to remove this option if you
#		need to debug with something like Macsbug.
#	-rd: for Rez means to suppress warnings for redeclared types (we redeclare
#		'RECT' because itÕs not included in MPW 3.0).
#	-append: means to add the resources to the target file, rather than
#		deleting all the ones that are there first.
#	-d Signature...: is a way of passing our application's signature to Rez.
#		With this mechanism, we can define our signature here, and export
#		it to Rez, so that we don't have to declare it there, too.
#	-sn STDCLIB=Main: puts all the routines that would normally go into the
#		STDCLIB segment into the Main segment. This is done so that when we
#		call upon any low-level utilities, we don't potentially move memory
#		by loading in a segment.
#------------------------------------------------------------------------------

DebugStmts			=	#-d NEEDS_DEBUG					# turn on to add internal debug stmts
CAltOptions			=	-r -b2 -d SystemSixOrLater=1
Cmp020Options		=	-d NEEDS_68020 -mc68020			# turn on to force 68020 or better CPU code
CmpColorQD			=	-d NEEDS_COLORQD				# turn on to force Color Quickdraw ROM Support
Cmp32BitQD			=	-d NEEDS_32BITQD				# turn on to force 32Bit Quickdraw ROM Support
CmpFPUOptions		=	#-d NEEDS_FPU -mc68881 -elems881 # turn on to force 68881 FPU calls
USE_NATIVE_MALLOC	=	#-d USE_NATIVE_MALLOC		# turn off to use C's malloc(), on for Mac's NewPtr()
MALLOC_TRACE		=	#-d	# Add file/line info to mallocs for garbcoll info
USE_MEM_HANDLES		=	#-d	# Use Mac's NewHandle() instead of NewPtr()
USE_MEMTAGS			=	#-d	# turn on to check free calls for bad ptrs
MemFlags			= ¶
	{USE_NATIVE_MALLOC} {MALLOC_TRACE} {USE_MEM_HANDLES} {USE_MEMTAGS}
CUSTOM_VERSION		=	#-d	XYZ_VERSION					# turn ON for any "special" versions :-)
NEEDS_PROF			=	#-d	NEEDS_PROF					# turn on to do profiling
LoadDumpOptions		=	#-k {objpath}					# turn on to do load/dump files
# Far model support (fardata was needed in 1.0, farcode for non-fpu version in 2.0...)
CModel				=	-model farcode
LinkModel			=	-model far

PerfCmpOptions		=	#-d NEEDS_PERF=1 -trace on		# turn on to enable performance code
PerfLinkOptions		=	#"{Libraries}"Proff.o -sn ProffSeg=Main	# turn on to use performance tools

# non-FPU libs
LinkMathLibs			=	"{CLibraries}"CSANELib.o "{CLibraries}"Math.o
# FPU libs
#LinkMathLibs		=	"{CLibraries}"Clib881.o "{CLibraries}"CSANELib881.o "{CLibraries}"Math881.o

MapOptions			=	-l -map > {AppName}.Map			# or use this for human-readable map
SymOptions			=	#-sym full						# turn on to debug with SADE
MbgOptions		=	-mbg on							# turn on to insert MacsBug labels
CInc2Folders	=	#-i {incpath}
RInc2Folders	=	#-i {rezpath}

ShowProgress	=  Echo "# `date -s` ----- Building {Targ}"

#----------------
# Good ol' MPW 3.2 std libraries are now in their own segments, for reasons known only to
# the MPW team..  Put them back into the main segment so we don't get totally fragmented..
SegmentMappings	=	-sn INTENV=Main2 ¶
					-sn PASLIB=Main2 ¶
					-sn SADEV=Main2 ¶
					-sn STDCLIB=Main2 ¶
					-sn STDIO=Main2 ¶
					-sn CSANELib=Main2 ¶
					-sn SANELIB=Main2
#----------------
MyCOptions		=	{COptions} {CAltOptions} {CModel} {CInc2Folders} {SymOptions} {LoadDumpOptions} ¶
					{MemFlags} {DebugStmts} {CUSTOM_VERSION} {MbgOptions} ¶
					{Cmp020Options} {CmpFPUOptions} {CmpColorQD} {Cmp32bitQD}

RezOptions		=	-rd -append {RInc2Folders} ¶
					-d AppSignature={AppSignature} -d AppName={AppName}

# -d suppress dups
LinkOptions		=	-d {LinkModel} {SymOptions} {PerfLinkOptions} ¶
					{SegmentMappings} {MapOptions}


#------------------------------------------------------------------------------
# These help define the libraries that we want to link with. {AppObjects} holds
# the names of the application units we want to link together. {CLibs} holds
# the libraries we want to link with under MPW 3.0 or MPW 3.1. Under MPW 3.2
# and later, ÒCInterface.oÓ and ÒCRuntime.oÓ are merged with ÒInterface.oÓ and
# ÒRuntime.oÓ. So, under 3.2 and later, we link with the files in {CLibs32}
# instead.
#------------------------------------------------------------------------------
CLibs			=	¶
					{LinkMathLibs} ¶
					"{CLibraries}"StdCLib.o ¶
					"{CLibraries}CRuntime.o" ¶
					"{CLibraries}CInterface.o" ¶
					"{Libraries}Interface.o"

CLibs32			=	¶
					{LinkMathLibs} ¶
					"{CLibraries}"StdCLib.o ¶
					"{Libraries}Runtime.o" ¶
					"{Libraries}Interface.o"


#------------------------------------------------------------------------------
# This is a dummy dependency rule.  This will always be executed.  This dummy
# rule must be the first for {AppName} so that it will be executed first.
# This is necessary to make evaluations that are beyond the scope of
# Make.  These evaluations will be performed by the CShell at execution time,
# and they must execute first because compile and link command lines depend
# on variables set up by these evaluations.  This has the unfortunate side
# effect that Make will always consider {AppName} to be out of date.  It will
# always, at a minimum, execute the commands for the target ShellForce.
#------------------------------------------------------------------------------

{AppName}			ÄÄ ShellForce

# With the above rule, {AppName} will always be out of date with respect to
# the non-existent file ShellForce.  This will force the following commands to
# be executed.
ShellForce			Ä
	BEGIN
		IF "{ShellVersion}" == ""
			( EVALUATE "`Version`" =~ /MPW ShellÅ ([0-9]+(.[ab0-9]+)+)¨1Å/ ) · Dev:Null
			SET ShellVersion "{¨1}"
		END
		IF "{ShellVersion}" =~ /3.[01]Å/
			SET CAltOptions "-d MPW31"
			SET CSysObjects "`QUOTE {CLibs}`"
		ELSE
			SET CAltOptions "-d MPW32"
			SET CSysObjects "`QUOTE {CLibs32}`"
		END
	END · Dev:Null # Output to bit bucket so we donÕt see above calculations


#------------------------------------------------------------------------------
# Build rule that links our application together. If any of our objects 
# change, or this makefile changes, then we relink.  The dummy prerequisite
# ShellForce must come before any other prerequisites for {AppName}
#------------------------------------------------------------------------------

{AppName}		ÄÄ	{AppName}.make  {AppObjects}
	{ShowProgress} ; Echo "# Linking..."
	{Link} {LinkOptions} -o {Targ} {AppObjects} {CSysObjects}
	# Set file's creator type, turn on bundle bit, turn off inited bit
	SetFile {Targ} -t APPL -c {AppSignature} -a Bi

#------------------------------------------------------------------------------
# Build rule that creates our resources and adds them to the application
#------------------------------------------------------------------------------

{AppName}		ÄÄ	{AppName}.make ¶
					{rezpath}{AppName}.r ¶
					{rsrcFiles}
	{ShowProgress} ; Echo "# Rezzing..."
	{Rez} {RezOptions} {rezpath}{AppName}.r -o {Targ}


#------------------------------------------------------------------------------
# Segment Names for the code segments
#------------------------------------------------------------------------------

segAnimate		= -s 'povAnim'
segBezier		= -s 'povBezier'
segBlob			= -s 'povBlob'
segBound		= -s 'povBound'
segBoxes		= -s 'povBoxes'
segCamera		= -s 'povCamera'
segCones		= -s 'povCones'
segCSG			= -s 'povCSG'
segDiscs		= -s 'povDiscs'
segHF			= -s 'povHF'
segImage		= -s 'povImage'
segLighting		= -s 'povLighting'
segNormal		= -s 'povNormal'
segParse		= -s 'povParse'
segParse2		= -s 'povParse2'
segPigment		= -s 'povPigment'
segPix			= -s 'povPix'
segPlanes		= -s 'povPlanes'
segPoly			= -s 'povPoly'
segPOVLib		= -s 'povLib'
segPOV			= -s 'povMac'
segPOVRay		= -s 'povRay'
segPrim1		= -s 'povPrim1'
segPrim2		= -s 'povPrim2'
segRender		= -s 'povRender'
segTex			= -s 'povTex'
segTokenize		= -s 'povToke'
segTri			= -s 'povTri'
segSphere		= -s 'povSphere'
segWindows		= -s 'povWindows'
segColour		= -s 'povColour'
segTarga		= -s 'povTarga'
segTemplate		= -s 'povTemplate'

#------------------------------------------------------------------------------
# Build rules that compile our POV Library from its source files
#------------------------------------------------------------------------------

{o_FileQueue}			Ä	{h_FileQueue} {srcpath}FileQueue.c
	{ShowProgress}
	{C} {srcpath}FileQueue.c {segPOVLib} {MyCOptions} {CAltOptions} -o {o_FileQueue}

{o_PopupMenu}			Ä  {h_PopupMenu} {srcpath}PopupMenu.c
	{ShowProgress}
	{C} {srcpath}PopupMenu.c {segPOVLib} {MyCOptions} {CAltOptions} -o {o_PopupMenu}

{o_POVMalloc}			Ä  {h_POVMalloc} {srcpath}POVMalloc.c
	{ShowProgress}
	{C} {srcpath}POVMalloc.c {segPOVLib} {MyCOptions} {CAltOptions} -o {o_POVMalloc}

{o_Printf2Window}		Ä	{h_Printf2Window}  {srcpath}Printf2Window.c
	{ShowProgress}
	{C} {srcpath}Printf2Window.c {segPOVLib} {MyCOptions} {CAltOptions} -o {o_Printf2Window}

{o_ProgressDialog}		Ä	{h_ProgressDialog}  {srcpath}ProgressDialog.c
	{ShowProgress}
	{C} {srcpath}ProgressDialog.c {segPOVLib} {MyCOptions} {CAltOptions} -o {o_ProgressDialog}

{o_ScreenUtils}			Ä	{h_ScreenUtils}  {srcpath}ScreenUtils.c
	{ShowProgress}
	{C} {srcpath}ScreenUtils.c {segPOVLib} {MyCOptions} {CAltOptions} -o {o_ScreenUtils}

{objpath}{POVLib}.o		Ä	{o_POVLib}
	{ShowProgress}
	{Lib} {o_POVLib} -o {objpath}{POVLib}.o

#------------------------------------------------------------------------------
# Build rules that compile our Mac-specific source files
#------------------------------------------------------------------------------

{objpath}Animate.c.o		ÄÄ	{h_POVLib} {incpath}Animate.h {srcpath}Animate.c
	{ShowProgress}
	{C} {srcpath}Animate.c {segAnimate} {MyCOptions} {CAltOptions} {PerfCmpOptions} -o {objpath}Animate.c.o

{objpath}ImageWindow.c.o		ÄÄ	{h_fp} {srcpath}ImageWindow.c {h_ImageWindow}
	{ShowProgress}
	{C} {srcpath}ImageWindow.c {segWindows} {MyCOptions} {CAltOptions} -o {objpath}ImageWindow.c.o

{objpath}Pov.C.o		ÄÄ	{h_fp} {h_ImageWindow} {h_TextEditor} {h_povmac} {h_POVLib} {h_TemplateMenu} {incpath}SaveCmpPict.h {srcpath}Pov.C
	{ShowProgress}
	{C} {srcpath}Pov.C {segPOV} {MyCOptions} {CAltOptions} -o {objpath}Pov.C.o

{objpath}SaveCmpPict.c.o	ÄÄ	{incpath}SaveCmpPict.h {srcpath}SaveCmpPict.c
	{ShowProgress}
	{C} {srcpath}SaveCmpPict.c {segPix} {MyCOptions} {CAltOptions} -o {objpath}SaveCmpPict.c.o

{objpath}TemplateMenu.c.o	ÄÄ	{h_TemplateMenu}  {h_povmac}  {srcpath}TemplateMenu.c
	{ShowProgress}
	{C} {srcpath}TemplateMenu.c {segTemplate} {MyCOptions} {CAltOptions} -o {objpath}TemplateMenu.c.o

{objpath}TextEditor.C.o	ÄÄ	{h_TextEditor} {h_povmac} {srcpath}TextEditor.C
	{ShowProgress}
	{C} {srcpath}TextEditor.C {segWindows} {MyCOptions} {CAltOptions} -o {objpath}TextEditor.C.o


#------------------------------------------------------------------------------
# Build rules that compile the POV-Ray Core source files
#------------------------------------------------------------------------------

{objpath}BEZIER.C.o		ÄÄ	{h_fvp} {corepath}BEZIER.C
	{ShowProgress}
	{C} {corepath}BEZIER.C {segBezier} {MyCOptions} {CAltOptions} {PerfCmpOptions} -o {objpath}BEZIER.C.o

{objpath}BLOB.C.o		ÄÄ	{h_fvp} {corepath}BLOB.C
	{ShowProgress}
	{C} {corepath}BLOB.C {segBlob} {MyCOptions} {CAltOptions} {PerfCmpOptions} -o {objpath}BLOB.C.o

{objpath}BOUND.C.o		ÄÄ	{h_fvp} {corepath}BOUND.C
	{ShowProgress}
	{C} {corepath}BOUND.C {segBound} {MyCOptions} {CAltOptions} {PerfCmpOptions} -o {objpath}BOUND.C.o

{objpath}BOXES.C.o		ÄÄ	{h_fvp} {corepath}BOXES.C
	{ShowProgress}
	{C} {corepath}BOXES.C {segBoxes} {MyCOptions} {CAltOptions} {PerfCmpOptions} -o {objpath}BOXES.C.o

{objpath}CAMERA.C.o		ÄÄ	{h_fvp} {corepath}CAMERA.C
	{ShowProgress}
	{C} {corepath}CAMERA.C {segCamera} {MyCOptions} {CAltOptions} {PerfCmpOptions} -o {objpath}CAMERA.C.o

{objpath}COLOUR.C.o		ÄÄ	{h_fvp} {corepath}COLOUR.C
	{ShowProgress}
	{C} {corepath}COLOUR.C {segColour} {MyCOptions} {CAltOptions} {PerfCmpOptions} -o {objpath}COLOUR.C.o

{objpath}CONES.C.o		ÄÄ	{h_fvp} {corepath}CONES.C
	{ShowProgress}
	{C} {corepath}CONES.C {segCones} {MyCOptions} {CAltOptions} {PerfCmpOptions} -o {objpath}CONES.C.o

{objpath}CSG.C.o		ÄÄ	{h_fvp} {corepath}CSG.C
	{ShowProgress}
	{C} {corepath}CSG.C {segCSG} {MyCOptions} {CAltOptions} {PerfCmpOptions} -o {objpath}CSG.C.o

{objpath}DISCS.C.o		ÄÄ	{h_fvp} {corepath}DISCS.C
	{ShowProgress}
	{C} {corepath}DISCS.C {segDiscs} {MyCOptions} {CAltOptions} {PerfCmpOptions} -o {objpath}DISCS.C.o

{objpath}DUMP.C.o		ÄÄ	{h_fp} {corepath}DUMP.C
	{ShowProgress}
	{C} {corepath}DUMP.C {segPix} {MyCOptions} {CAltOptions} {PerfCmpOptions} -o {objpath}DUMP.C.o

{objpath}EXPRESS.C.o		ÄÄ	{h_fp} {corepath}EXPRESS.C
	{ShowProgress}
	{C} {corepath}EXPRESS.C {segParse2} {MyCOptions} {CAltOptions} {PerfCmpOptions} -o {objpath}EXPRESS.C.o

{objpath}GIF.C.o		ÄÄ	{h_fp} {corepath}GIF.C
	{ShowProgress}
	{C} {corepath}GIF.C {segPix} {MyCOptions} {CAltOptions} {PerfCmpOptions} -o {objpath}GIF.C.o

{objpath}GIFDECOD.C.o	ÄÄ	{h_fp} {corepath}GIFDECOD.C
	{ShowProgress}
	{C} {corepath}GIFDECOD.C {segPix} {MyCOptions} {CAltOptions} {PerfCmpOptions} -o {objpath}GIFDECOD.C.o

{objpath}HFIELD.C.o		ÄÄ	{h_fvp} {corepath}HFIELD.C
	{ShowProgress}
	{C} {corepath}HFIELD.C {segHF} {MyCOptions} {CAltOptions} {PerfCmpOptions} -o {objpath}HFIELD.C.o

{objpath}IFF.C.o		ÄÄ	{h_fp} {corepath}IFF.C
	{ShowProgress}
	{C} {corepath}IFF.C {segPix} {MyCOptions} {CAltOptions} {PerfCmpOptions} -o {objpath}IFF.C.o

{objpath}IMAGE.C.o		ÄÄ	{h_fp} {corepath}IMAGE.C
	{ShowProgress}
	{C} {corepath}IMAGE.C {segImage} {MyCOptions} {CAltOptions} {PerfCmpOptions} -o {objpath}IMAGE.C.o

{objpath}LIGHTING.C.o	ÄÄ	{h_fvp} {corepath}LIGHTING.C
	{ShowProgress}
	{C} {corepath}LIGHTING.C {segLighting} {MyCOptions} {CAltOptions} {PerfCmpOptions} -o {objpath}LIGHTING.C.o

{objpath}MATRICES.C.o	ÄÄ	{h_fvp} {corepath}MATRICES.C
	{ShowProgress}
	{C} {corepath}MATRICES.C {segPrim2} {MyCOptions} {CAltOptions} {PerfCmpOptions} -o {objpath}MATRICES.C.o

{objpath}NORMAL.C.o	ÄÄ	{h_fvp} {corepath}NORMAL.C
	{ShowProgress}
	{C} {corepath}NORMAL.C {segNormal} {MyCOptions} {CAltOptions} {PerfCmpOptions} -o {objpath}NORMAL.C.o

{objpath}OBJECTS.C.o	ÄÄ	{h_fvp} {corepath}OBJECTS.C
	{ShowProgress}
	{C} {corepath}OBJECTS.C {segPrim1} {MyCOptions} {CAltOptions} {PerfCmpOptions} -o {objpath}OBJECTS.C.o

{objpath}PARSE.C.o		ÄÄ	{h_fvp} {corepath}PARSE.C {incpath}PARSE.H
	{ShowProgress}
	{C} {corepath}PARSE.C {segParse} {MyCOptions} {CAltOptions} {PerfCmpOptions} -o {objpath}PARSE.C.o

{objpath}PIGMENT.C.o		ÄÄ	{h_fvp} {corepath}PIGMENT.C
	{ShowProgress}
	{C} {corepath}PIGMENT.C {segPigment} {MyCOptions} {CAltOptions} {PerfCmpOptions} -o {objpath}PIGMENT.C.o

{objpath}PLANES.C.o		ÄÄ	{h_fvp} {corepath}PLANES.C
	{ShowProgress}
	{C} {corepath}PLANES.C {segPlanes} {MyCOptions} {CAltOptions} {PerfCmpOptions} -o {objpath}PLANES.C.o

{objpath}POINT.C.o		ÄÄ	{h_fvp} {corepath}POINT.C
	{ShowProgress}
	{C} {corepath}POINT.C {segPrim1} {MyCOptions} {CAltOptions} {PerfCmpOptions} -o {objpath}POINT.C.o

{objpath}POLY.C.o		ÄÄ	{h_fvp} {corepath}POLY.C
	{ShowProgress}
	{C} {corepath}POLY.C {segPoly} {MyCOptions} {CAltOptions} {PerfCmpOptions} -o {objpath}POLY.C.o

{objpath}POVRAY.C.o		ÄÄ	{h_fvp} {corepath}POVRAY.C
	{ShowProgress}
	{C} {corepath}POVRAY.C {segPOVRay} {MyCOptions} {CAltOptions} {PerfCmpOptions} -o {objpath}POVRAY.C.o

{objpath}QUADRICS.C.o	ÄÄ	{h_fvp} {corepath}QUADRICS.C
	{ShowProgress}
	{C} {corepath}QUADRICS.C {segPoly} {MyCOptions} {CAltOptions} {PerfCmpOptions} -o {objpath}QUADRICS.C.o

{objpath}RAW.C.o		ÄÄ	{h_fp} {corepath}RAW.C
	{ShowProgress}
	{C} {corepath}RAW.C {segPix} {MyCOptions} {CAltOptions} {PerfCmpOptions} -o {objpath}RAW.C.o

{objpath}RAY.C.o		ÄÄ	{h_fvp} {corepath}RAY.C
	{ShowProgress}
	{C} {corepath}RAY.C {segRender} {MyCOptions} {CAltOptions} {PerfCmpOptions} -o {objpath}RAY.C.o

{objpath}RENDER.C.o		ÄÄ	{h_fvp} {corepath}RENDER.C
	{ShowProgress}
	{C} {corepath}RENDER.C {segRender} {MyCOptions} {CAltOptions} {PerfCmpOptions} -o {objpath}RENDER.C.o

{objpath}SPHERES.C.o	ÄÄ	{h_fvp} {corepath}SPHERES.C
	{ShowProgress}
	{C} {corepath}SPHERES.C {segSphere} {MyCOptions} {CAltOptions} {PerfCmpOptions} -o {objpath}SPHERES.C.o

{objpath}TARGA.C.o		ÄÄ	{h_fp} {corepath}TARGA.C
	{ShowProgress}
	{C} {corepath}TARGA.C {segTarga} {MyCOptions} {CAltOptions} {PerfCmpOptions} -o {objpath}TARGA.C.o

{objpath}TEXTURE.C.o	ÄÄ	{h_fvpt} {corepath}TEXTURE.C
	{ShowProgress}
	{C} {corepath}TEXTURE.C {segTex} {MyCOptions} {CAltOptions} {PerfCmpOptions} -o {objpath}TEXTURE.C.o

{objpath}TOKENIZE.C.o	ÄÄ	{h_fp} {corepath}TOKENIZE.C
	{ShowProgress}
	{C} {corepath}TOKENIZE.C {segTokenize} {MyCOptions} {CAltOptions} {PerfCmpOptions} -o {objpath}TOKENIZE.C.o

{objpath}TRIANGLE.C.o	ÄÄ	{h_fvp} {corepath}TRIANGLE.C
	{ShowProgress}
	{C} {corepath}TRIANGLE.C {segTri} {MyCOptions} {CAltOptions} {PerfCmpOptions} -o {objpath}TRIANGLE.C.o

{objpath}TXTTEST.C.o	ÄÄ	{h_fvpt} {corepath}TXTTEST.C
	{ShowProgress}
	{C} {corepath}TXTTEST.C {segPrim2} {MyCOptions} {CAltOptions} {PerfCmpOptions} -o {objpath}TXTTEST.C.o

{objpath}VECT.C.o		ÄÄ	{h_fvp} {corepath}VECT.C
	{ShowProgress}
	{C} {corepath}VECT.C {segPrim1} {MyCOptions} {CAltOptions} {PerfCmpOptions} -o {objpath}VECT.C.o
