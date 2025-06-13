#######################################################################
#	Project	:	XL				-	XCMDs for everyone
#	File		:	Makefile		-	Make it 
#	Author	:	Matthias Neeracher
#	Started	:	11Sep92								Language	:	MPW Shell/Make
#	Modified	:	27Jan93	MN	XLDEBUG
#	Last		:	27Jan93
#######################################################################

#
# Destinations
#

XLCIncludes			=	{CIncludes}
XL68KCLibraries	=	{CLibraries}
XLPPCCLibraries	=	{PPCLibraries}

#
# 68K debugging options
#

Debug68K				=	-d XLDEBUG -sym on -mbg on

#
# PowerPC debugging options. Note that -sym on disables optimization!
#

DebugPPC				=	-d XLDEBUG -sym on

#
# PowerPC optimization options. Note that -sym on above disables optimization!
#

PPCCOpt				=	-opt local

#
# Tool options
#

XLInc					=	-i : 
COptions 			= 	-r {XLInc} {Debug68K}
CPlusOptions 		= 	-mf {XLInc} {Debug68K}
PPCCExt				=	-d macintosh -appleext on
PPCCOptions 		=  {PPCCOpt} {PPCCExt} {XLInc} {DebugPPC}
LOptions				= 	-sym on -mf -w
ROptions 			= 	-i :

XL			Ä	XL.o.68K XL.o.PPC # XLTest

XL.o.68K		Ä	XL.c
	C {COptions} -o XL.o.68K XL.c

XL.o.PPC		Ä	XL.c
	PPCC {PPCCOptions} -o XL.o.PPC XL.c

XLTest.c.o Ä XLTest.c
	C {COptions} {Debug} -o XLTest.c.o XLTest.c
	
XLTest	Ä	XL.o XLTest.c.o
	Link -mf -t MPST -c 'MPS ' -o XLTest	¶
		XLTest.c.o									¶
		XL.o.68K										¶
		{Libraries}Interface.o					¶
		{Libraries}Runtime.o						¶
		{CLibraries}StdCLib.o

Install	Ä	XL.o
	Duplicate -y XL.o.68K "{XL68KCLibraries}"XL.o
	Duplicate -y XL.o.PPC "{XLPPCCLibraries}"XL.o
	Duplicate -y XL.h 	 "{XLCIncludes}"

Distr Ä XL
	:Distribute XL.distr XL_11.sit
