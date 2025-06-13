#==============================================================================
#Project:	POV-Utilities
#
#File:		MacSPD.Make
#
#Description:
#	MPW Make file to create all the SPD data base applications
#	(Tabs are 4 spaces in this file)
#------------------------------------------------------------------------------
#Author:
#	Eduard [esp] Schwan
#------------------------------------------------------------------------------
#Change History:
#	921214	[esp]	Created.
#	930117	[esp]	Added resource files & cleaned up build rules a bit
#	930317	[esp]	Added ShowDXF app & made 020/881 apps
#	930418	[esp]	Added Driver App & Teapot
#	930805	[esp]	Cleaned up comments a bit
#	930901	[eah]	New file names
#	930918	[esp]	Restored special high-bit characters (Ä) that Eric's VI editor ate.
#	931012	[esp]	Added ReadNFF & ReadObj
#	940506	[esp]	broke libprm.c into libpr1.c/libpr2.c/libpr3.c
#==============================================================================


#------------------------------------------------------------------------------
C		= C
Link	= Link
Rez		= Rez
#------------------------------------------------------------------------------
srcpath			=	:
rezpath			=	:
incpath			=	:
# do this to keep pesky .o files out of your source dir, but create :obj: first!
# objpath			= 	:obj:
objpath			= 	:

#------------------------------------------------------------------------------
AppNameBalls	=	Balls
AppNameGears	=	Gears
AppNameMount	=	Mount
AppNameReadNFF	=	ReadNFF
#AppNameReadObj	=	ReadObj
AppNameRings	=	Rings
AppNameReadDXF	=	ReadDXF
AppNameTeapot	=	Teapot
AppNameTetra	=	Tetra
AppNameTree		=	Tree
AppNameLattice	=	Lattice
AppNameShells	=	Shells
AppNameSample	=	Sample

#------------------------------------------------------------------------------
SPDHDRS			=	{incpath}lib.h {incpath}libvec.h {incpath}def.h {incpath}drv.h
SPDOBJS			=	{objpath}libpr1.c.o {objpath}libpr2.c.o {objpath}libpr3.c.o ¶
					{objpath}libinf.c.o {objpath}libini.c.o ¶
					{objpath}libply.c.o {objpath}libdmp.c.o ¶
					{objpath}libvec.c.o {objpath}drv_mac.c.o

BallsObjects	=	"{objpath}{AppNameBalls}.c.o"	{SPDOBJS}
GearsObjects	=	"{objpath}{AppNameGears}.c.o"	{SPDOBJS}
MountObjects	=	"{objpath}{AppNameMount}.c.o"	{SPDOBJS}
#ReadObjObjects	=	"{objpath}{AppNameReadObj}.c.o"	{SPDOBJS}
ReadNFFObjects	=	"{objpath}{AppNameReadNFF}.c.o"	{SPDOBJS}
RingsObjects	=	"{objpath}{AppNameRings}.c.o"	{SPDOBJS}
ReadDXFObjects	=	"{objpath}{AppNameReadDXF}.c.o"	{SPDOBJS}
TeapotObjects	=	"{objpath}{AppNameTeapot}.c.o"	{SPDOBJS}
TetraObjects	=	"{objpath}{AppNameTetra}.c.o"	{SPDOBJS}
TreeObjects		=	"{objpath}{AppNameTree}.c.o"	{SPDOBJS}
SampleObjects	=	"{objpath}{AppNameSample}.c.o"	{SPDOBJS}
LatticeObjects	=	"{objpath}{AppNameLattice}.c.o"	{SPDOBJS}
ShellsObjects	=	"{objpath}{AppNameShells}.c.o"	{SPDOBJS}

MacSPDRez		=	"{rezpath}MacSPD.r"
MacSPDRsrc		=	"{rezpath}MacSPD.rsrc"
RezFiles		=	{MacSPDRez} {MacSPDRsrc}

Signature		=	'SPDm'

#------------------------------------------------------------------------------
SymOptions		=	-sym on				# turn this on to debug with SADE
MbgOptions		=	-mbg on 			# turn this on for Macsbug symbols
CMathOptions	=	#-mc68881			# -and- link with Link881MathLibs
CAltOptions		=	-r -mc68020 {MbgOptions} {SymOptions} {CMathOptions}
RezOptions		=	-rd -append -d Signature={Signature} -d AppName={AppName}
LinkOptions		=	{SymOptions}
LinkStdMathLibs	=	"{CLibraries}"StdCLib.o "{CLibraries}"CSANELib.o "{CLibraries}"Math.o
Link881MathLibs	=	"{CLibraries}"Clib881.o "{CLibraries}"CSANELib881.o "{CLibraries}"Math881.o
LinkMathLibs	=	{LinkStdMathLibs}

#------------------------------------------------------------------------------
{objpath}libinf.c.o		ÄÄ	{SPDHDRS}
{objpath}libini.c.o		ÄÄ	{SPDHDRS}
{objpath}libdmp.c.o		ÄÄ	{SPDHDRS}
{objpath}libply.c.o		ÄÄ	{SPDHDRS}
{objpath}libpr1.c.o		ÄÄ	{SPDHDRS}
{objpath}libpr2.c.o		ÄÄ	{SPDHDRS}
{objpath}libpr3.c.o		ÄÄ	{SPDHDRS}
{objpath}libvec.c.o		ÄÄ	{incpath}libvec.h
{objpath}drv_mac.c.o	ÄÄ	{incpath}drv.h

#------------------------------------------------------------------------------
CLibs32			= ¶
		{LinkMathLibs}				¶
		"{Libraries}"Runtime.o		¶
		"{Libraries}"Interface.o

#------------------------------------------------------------------------------
{objpath}		Ä	{srcpath}

#------------------------------------------------------------------------------
.c.o			Ä	.c  {SPDHDRS}
	Echo "# `date -s` ----- Compiling {Default}.c"
	{C} {DepDir}{Default}.c -o {TargDir}{Default}.c.o -s {Default} {COptions} {CAltOptions}

#------------------------------------------------------------------------------
# Good ol' MPW 3.2 std libraries are now in their own segments, for reasons known only to
# the MPW team..  Put them back into the main segment so we don't get totally fragmented..
SegmentMappings	= ¶
			-sn INTENV=Main2 -sn PASLIB=Main2 ¶
			-sn SADEV=Main2 -sn STDCLIB=Main2 ¶
			-sn STDIO=Main2 -sn CSANELib=Main2 ¶
			-sn SANELIB=Main2

#------------------------------------------------------------------------------
{AppNameBalls}	ÄÄ {BallsObjects}
	Echo "# `date -s` ----- Linking {Targ}"
	{Link} {LinkOptions} -o {Targ} ¶
		{BallsObjects} {CLibs32} {SegmentMappings} -map >{Targ}.map
	SetFile {Targ} -t APPL -c {Signature} -a Bi

{AppNameBalls}	ÄÄ {RezFiles}
	Echo "# `date -s` ----- Rezzing {Targ}"
	{Rez} {RezOptions} {MacSPDRez} -o {Targ}

#------------------------------------------------------------------------------
{AppNameGears}	ÄÄ {GearsObjects}
	Echo "# `date -s` ----- Linking {Targ}"
	{Link} {LinkOptions} -o {Targ} ¶
		{GearsObjects} {CLibs32} {SegmentMappings} -map >{Targ}.map
	SetFile {Targ} -t APPL -c {Signature} -a Bi

{AppNameGears}	ÄÄ {RezFiles}
	Echo "# `date -s` ----- Rezzing {Targ}"
	{Rez} {RezOptions} {MacSPDRez} -o {Targ}

#------------------------------------------------------------------------------
{AppNameLattice}	ÄÄ {LatticeObjects}
	Echo "# `date -s` ----- Linking {Targ}"
	{Link} {LinkOptions} -o {Targ} ¶
		{LatticeObjects} {CLibs32} {SegmentMappings} -map >{Targ}.map
	SetFile {Targ} -t APPL -c {Signature} -a Bi

{AppNameLattice}	ÄÄ {RezFiles}
	Echo "# `date -s` ----- Rezzing {Targ}"
	{Rez} {RezOptions} {MacSPDRez} -o {Targ}

#------------------------------------------------------------------------------
{AppNameMount}	ÄÄ {MountObjects}
	Echo "# `date -s` ----- Linking {Targ}"
	{Link} {LinkOptions} -o {Targ} ¶
		{MountObjects} {CLibs32} {SegmentMappings} -map >{Targ}.map
	SetFile {Targ} -t APPL -c {Signature} -a Bi

{AppNameMount}	ÄÄ {RezFiles}
	Echo "# `date -s` ----- Rezzing {Targ}"
	{Rez} {RezOptions} {MacSPDRez} -o {Targ}

#------------------------------------------------------------------------------
{AppNameReadDXF}	ÄÄ {ReadDXFObjects}
	Echo "# `date -s` ----- Linking {Targ}"
	{Link} {LinkOptions} -o {Targ} ¶
		{ReadDXFObjects} {CLibs32} {SegmentMappings} -map >{Targ}.map
	SetFile {Targ} -t APPL -c {Signature} -a Bi

{AppNameReadDXF}	ÄÄ {RezFiles}
	Echo "# `date -s` ----- Rezzing {Targ}"
	{Rez} {RezOptions} {MacSPDRez} -o {Targ}

#------------------------------------------------------------------------------
{AppNameReadNFF}	ÄÄ {ReadNFFObjects}
	Echo "# `date -s` ----- Linking {targ}"
	{Link} {LinkOptions} -o {targ} ¶
		{ReadNFFObjects} {CLibs32} {SegmentMappings} -map >{targ}.map
	SetFile {targ} -t APPL -c {Signature} -a Bi

{AppNameReadNFF}	ÄÄ {RezFiles}
	Echo "# `date -s` ----- Rezzing {Targ}"
	{Rez} {RezOptions} {MacSPDRez} -o {Targ}

#------------------------------------------------------------------------------
#{AppNameReadObj}	ÄÄ {ReadObjObjects}
#	Echo "# `date -s` ----- Linking {targ}"
#	{Link} {LinkOptions} -o {targ} ¶
#		{ReadObjObjects} {CLibs32} {SegmentMappings} -map >{targ}.map
#	SetFile {targ} -t APPL -c {Signature} -a Bi

#{AppNameReadObj}	ÄÄ {RezFiles}
#	Echo "# `date -s` ----- Rezzing {Targ}"
#	{Rez} {RezOptions} {MacSPDRez} -o {Targ}

#------------------------------------------------------------------------------
{AppNameRings}	ÄÄ {RingsObjects}
	Echo "# `date -s` ----- Linking {Targ}"
	{Link} {LinkOptions} -o {Targ} ¶
		{RingsObjects} {CLibs32} {SegmentMappings} -map >{Targ}.map
	SetFile {Targ} -t APPL -c {Signature} -a Bi

{AppNameRings}	ÄÄ {RezFiles}
	Echo "# `date -s` ----- Rezzing {Targ}"
	{Rez} {RezOptions} {MacSPDRez} -o {Targ}

#------------------------------------------------------------------------------
{AppNameSample}	ÄÄ {SampleObjects}
	Echo "# `date -s` ----- Linking {Targ}"
	{Link} {LinkOptions} -o {Targ} ¶
		{SampleObjects} {CLibs32} {SegmentMappings} -map >{Targ}.map
	SetFile {Targ} -t APPL -c {Signature} -a Bi

{AppNameSample}	ÄÄ {RezFiles}
	Echo "# `date -s` ----- Rezzing {Targ}"
	{Rez} {RezOptions} {MacSPDRez} -o {Targ}

#------------------------------------------------------------------------------
{AppNameShells}	ÄÄ {ShellsObjects}
	Echo "# `date -s` ----- Linking {Targ}"
	{Link} {LinkOptions} -o {Targ} ¶
		{ShellsObjects} {CLibs32} {SegmentMappings} -map >{Targ}.map
	SetFile {Targ} -t APPL -c {Signature} -a Bi

{AppNameShells}	ÄÄ {RezFiles}
	Echo "# `date -s` ----- Rezzing {Targ}"
	{Rez} {RezOptions} {MacSPDRez} -o {Targ}

#------------------------------------------------------------------------------
{AppNameTeapot}	ÄÄ {TeapotObjects}
	Echo "# `date -s` ----- Linking {Targ}"
	{Link} {LinkOptions} -o {Targ} ¶
		{TeapotObjects} {CLibs32} {SegmentMappings} -map >{Targ}.map
	SetFile {Targ} -t APPL -c {Signature} -a Bi

{AppNameTeapot}	ÄÄ {RezFiles}
	Echo "# `date -s` ----- Rezzing {Targ}"
	{Rez} {RezOptions} {MacSPDRez} -o {Targ}

#------------------------------------------------------------------------------
{AppNameTetra}	ÄÄ {TetraObjects}
	Echo "# `date -s` ----- Linking {Targ}"
	{Link} {LinkOptions} -o {Targ} ¶
		{TetraObjects} {CLibs32} {SegmentMappings} -map >{Targ}.map
	SetFile {Targ} -t APPL -c {Signature} -a Bi

{AppNameTetra}	ÄÄ {RezFiles}
	Echo "# `date -s` ----- Rezzing {Targ}"
	{Rez} {RezOptions} {MacSPDRez} -o {Targ}

#------------------------------------------------------------------------------
{AppNameTree}	ÄÄ {TreeObjects}
	Echo "# `date -s` ----- Linking {Targ}"
	{Link} {LinkOptions} -o {Targ} ¶
		{TreeObjects} {CLibs32} {SegmentMappings} -map >{Targ}.map
	SetFile {Targ} -t APPL -c {Signature} -a Bi

{AppNameTree}	ÄÄ {RezFiles}
	Echo "# `date -s` ----- Rezzing {Targ}"
	{Rez} {RezOptions} {MacSPDRez} -o {Targ}

#------------------------------------------------------------------------------
# Must be a dummy single destination app
MacSPD			Ä	¶
		{AppNameBalls} ¶
		{AppNameGears} ¶
		{AppNameLattice} ¶
		{AppNameMount} ¶
		{AppNameReadDXF} ¶
		{AppNameReadNFF} ¶
		#{AppNameReadObj} ¶
		{AppNameRings} ¶
		{AppNameSample} ¶
		{AppNameShells} ¶
		{AppNameTeapot} ¶
		{AppNameTetra} ¶
		{AppNameTree}
