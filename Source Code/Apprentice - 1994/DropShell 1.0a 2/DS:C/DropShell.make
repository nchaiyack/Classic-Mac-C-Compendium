##*****************************************************************************
##
##  Project Name:	DropShell
##     File Name:	DropShell.make
##
##   Description:	Makefile for DropShell
##						This makefile was created with & must be built by
##						MPW's Build Menu.  Simply use the Build� option,
##						specifying DropShell as the thing to build.
##						You can also use the BuildProgram command.
##
##*****************************************************************************
##                       A U T H O R   I D E N T I T Y
##*****************************************************************************
##
##	Initials	Name
##	--------	-----------------------------------------------
##	LDR			Leonard Rosenthol
##
##*****************************************************************************
##                      R E V I S I O N   H I S T O R Y
##*****************************************************************************
##
##	  Date		Time	Author	Description
##	--------	-----	------	---------------------------------------------
##	11/24/91			LDR		Cleaned it up a bit using some more vars
##								Added the DSUtils stuff
##	10/28/91			LDR		Modified for the C version of DropShell
##	10/28/91			LDR		Officially renamed DropShell (from QuickShell)
##								And added some comments
##	04/08/91	23:57	LDR		Original Version
##
##*****************************************************************************

#   File:       DropShell.make
#   Target:     DropShell
#   Sources:    DSGlobals.h
#               DSAppleEvents.h
#				DropShell.h
#               DSAppleEvents.c
#               DropShell.c
#               DropShell.r
#               DSUserProcs.c
#				DSUtils.c

ObjectDir = ":Objects:"

LIBS	= �
		"{Libraries}"Runtime.o �
		"{Libraries}"Interface.o �
		"{CLibraries}"CSANELib.o �
		"{CLibraries}"CInterface.o

OBJECTS = �
		{ObjectDir}DSUtils.c.o �
		{ObjectDir}DSUserProcs.c.o �
		{ObjectDir}DSAppleEvents.c.o �
		{ObjectDir}DropShell.c.o


"{ObjectDir}"DSUtils.c.o � DropShell.make DSUtils.c DSUtils.h DSGlobals.h
	 C  -o {ObjectDir} DSUtils.c
"{ObjectDir}"DSUserProcs.c.o � DropShell.make DSUserProcs.c DSUserProcs.h DSGlobals.h
	 C  -o {ObjectDir} DSUserProcs.c
"{ObjectDir}"DSAppleEvents.c.o � DropShell.make DSAppleEvents.c DSAppleEvents.h DSGlobals.h
	 C  -o {ObjectDir} DSAppleEvents.c
"{ObjectDir}"DropShell.c.o � DropShell.make DropShell.c DSAppleEvents.c DSAppleEvents.h DSGlobals.h
	 C  -o {ObjectDir} DropShell.c

DropShell �� DropShell.make {OBJECTS}
	Link -w -t APPL -c '????' �
		{OBJECTS} {LIBS} �
		-o DropShell

DropShell �� DropShell.make DropShell.r
	Rez DropShell.r -append -o DropShell
