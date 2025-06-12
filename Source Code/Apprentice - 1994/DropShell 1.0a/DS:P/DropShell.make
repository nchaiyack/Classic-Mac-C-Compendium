##*****************************************************************************
##
##  Project Name:	DropShell
##     File Name:	DropShell.make
##
##   Description:	Makefile for DropShell
##						This makefile was created with & must be built by
##						MPW's Build Menu.  Simply use the BuildÉ option,
##						specifying DropShell as the thing to build.
##						You can also use the BuildProgram command
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
##	11/24/91			LDR		Cleaned up with some new vars
##								Added stuff for new DSUtils file
##	10/30/91			LDR		Added new POptions flag which includes $IFC setup
##	10/28/91			LDR		Officially renamed DropShell (from QuickShell)
##								And added some comments
##	04/08/91	23:57	LDR		Original Version
##
##*****************************************************************************

#   File:       DropShell.make
#   Target:     DropShell
#   Sources:    DSGlobals.p
#               DSAppleEvents.p
#               DropShell.p
#               DropShell.r
#               DSUserProcs.p
#				DSUtils.p

ObjectDir = ":Objects:"
POptions  = -o {ObjectDir} -d THINK_Pascal=FALSE

LIBS	= ¶
		"{Libraries}"Runtime.o ¶
		"{Libraries}"Interface.o ¶
		"{PLibraries}"SANELib.o ¶
		"{PLibraries}"PasLib.o

OBJECTS = ¶
		{ObjectDir}DSGlobals.p.o ¶
		{ObjectDir}DSUtils.p.o ¶
		{ObjectDir}DSAppleEvents.p.o ¶
		{ObjectDir}DSUserProcs.p.o ¶
		{ObjectDir}DropShell.p.o


"{ObjectDir}"DSGlobals.p.o Ä DropShell.make DSGlobals.p
	 Pascal  {POptions} DSGlobals.p
"{ObjectDir}"DSUtils.p.o Ä DropShell.make DSGlobals.p DSUtils.p 
	 Pascal  {POptions} DSUtils.p
"{ObjectDir}"DSUserProcs.p.o Ä DropShell.make DSGlobals.p DSUtils.p DSUserProcs.p
	 Pascal  {POptions} DSUserProcs.p
"{ObjectDir}"DSAppleEvents.p.o Ä DropShell.make DSGlobals.p DSUtils.p DSUserProcs.p DSAppleEvents.p
	 Pascal  {POptions} DSAppleEvents.p
"{ObjectDir}"DropShell.p.o Ä DropShell.make DSGlobals.p DSUtils.p DSUserProcs.p DSAppleEvents.p DropShell.p
	 Pascal  {POptions} DropShell.p

DropShell ÄÄ DropShell.make {OBJECTS}
	Link -w -t APPL -c '????' ¶
		{OBJECTS} {LIBS} ¶
		-o DropShell

DropShell ÄÄ DropShell.make DropShell.r
	Rez DropShell.r -append -o DropShell

