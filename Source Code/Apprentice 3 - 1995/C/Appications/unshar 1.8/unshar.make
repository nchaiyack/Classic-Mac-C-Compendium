#
#	File:		unshar.make
#
#	Contains:	Makefile to build unshar tool
#
#	Written by:	Sak Wathanasin
#				178 Wainbody Ave South
#				Coventry CV3 6BX
#				UK
#	Phone:		+44 203 419996
#	E-mail:		sw@network-analysis-ltd.co.uk
#
#	Copyright:	Public Domain
#
#	Change History (most recent first):
#
#	<2>			  3/2/91	sw		Change link libs for MPW 3.2
#	<1>			  6/7/90  	sw		Created base version
#
#	To Do:
#

OBJECTS = unshar.c.o


unshar.c.o Ä unshar.make unshar.c
	 C  -d MPW unshar.c

unshar ÄÄ unshar.make {OBJECTS}
	Link -w -c 'MPS ' -t MPST ¶
		{OBJECTS} ¶
		"{Libraries}"Stubs.o ¶
		"{Libraries}"Runtime.o ¶
		"{Libraries}"Interface.o ¶
		"{CLibraries}"StdCLib.o ¶
		"{Libraries}"ToolLibs.o ¶
		-o unshar
