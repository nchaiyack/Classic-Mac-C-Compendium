#######################################################################
#	Project	:	MacPerl			-
#	File		:	Makefile			-	Do it
#	Author	:	Matthias Neeracher
#	Started	:	15May93								Language	:	MPW Shell/Make
#	Modified	:	29May93	MN	Compiles correctly
#					29May93	MN	Add more conservative dependencies
#					09Aug93	MN	MPHelp
#					16Aug93	MN	MPScript
#					08Sep93	MN	4.1.0b3
#					14Sep93	MN	Runtime
#					02Oct93	MN	Droplet
#					13Oct93	MN	4.1.0b5
#					24Oct93	MN	4.1.0
#					08Dec93	MN	Preferences
#					12Jan94	MN	4.1.1
#					08Jul94	MN	4.1.4 : PowerPC
#	Last		:	08Jul94
#######################################################################

#######################################################################
# You might not need to change anything below this line 
#

#
# Specify one of:
#  68K			Build 68K version only
#	PPC			Build PPC version only
#	Both			Build both 68K and PPC versions
#

Focus		=	Both

#
# 68K debugging options
#

Debug68K				=	-sym on -mbg on

#
# PowerPC debugging options. Note that -sym on disables optimization!
#

DebugPPC				=	-sym on

#
# PowerPC optimization options. Note that -sym on above disables optimization!
#

PPCCOpt				=	-opt local

#
# Tool options

MacPerlInc			=	-i : -i ::Perl
COptions 			= 	-r {MacPerlInc} {Debug68K} {IsRuntime} 
CPlusOptions 		= 	-mf {MacPerlInc} {Debug68K} {IsRuntime} 
PPCCExt				=	-d macintosh -appleext on -d __useAppleExts__
PPCCOptions 		=   {PPCCOpt} {PPCCExt} {MacPerlInc} {DebugPPC}
LOptions				= 	-sym 3.2 -mf -w
ROptions 			= 	-i :

MacPerlSources	=		�
	MPAEUtils.c			�
	MPAppleEvents.c	�
	MPGlobals.c			�
	MPEditions.c		�
	MPUtils.c			�
	MPFile.c				�
	MPMain.c				�
	MPWindow.c			�
	MPConsole.cp		�
	MPAEVTStream.cp	�
	MPHelp.c				�
	MPScript.c			�
	MPPseudoFile.cp	�
	MPSave.c				�
	Mercutio.c
	
MacPerlHeaders	=		�
	MPAEUtils.h			�
	MPAppleEvents.h	�
	MPGlobals.h			�
	MPEditions.h		�
	MPUtils.h			�
	MPFile.h				�
	MPWindow.h			�
	MPConsole.h			�
	MPHelp.h				�
	MPScript.h			�
	MPSave.h				�
	Mercutio.h

MacPerlObjects	=				�
	:MP:MPAEUtils.c.o			�
	:MP:MPAppleEvents.c.o	�
	:MP:MPGlobals.c.o			�
	:MP:MPEditions.c.o		�
	:MP:MPUtils.c.o			�
	:MP:MPFile.c.o				�
	:MP:MPMain.c.o				�
	:MP:MPWindow.c.o			�
	:MP:MPConsole.cp.o		�
	:MP:MPAEVTStream.cp.o	�
	:MP:MPHelp.c.o				�
	:MP:MPScript.c.o			�
	:MP:MPPreferences.c.o	�
	:MP:MPPseudoFile.cp.o	�
	:MP:MPSave.c.o				�
	:MP:Mercutio.c.o			�
	::Perl:Perl.o				�
	"{CLibraries}GUSI.o"		�
	"{CLibraries}libdb.o"	�
	"{CLibraries}XL.o"		�
	:MP:MPForceLoad.cp.o

MPPPCObjects	=					�
	:MP:MPAEUtils.c.o.PPC		�
	:MP:MPAppleEvents.c.o.PPC	�
	:MP:MPGlobals.c.o.PPC		�
	:MP:MPEditions.c.o.PPC		�
	:MP:MPUtils.c.o.PPC			�
	:MP:MPFile.c.o.PPC			�
	:MP:MPMain.c.o.PPC			�
	:MP:MPWindow.c.o.PPC			�
	:MP:MPConsole.cp.o.PPC		�
	:MP:MPAEVTStream.cp.o.PPC	�
	:MP:MPHelp.c.o.PPC			�
	:MP:MPScript.c.o.PPC			�
	:MP:MPPreferences.c.o.PPC	�
	:MP:MPPseudoFile.cp.o.PPC	�
	:MP:MPSave.c.o.PPC			�
	:MP:Mercutio.c.o.PPC

MPRuntimeObjects	=			�
	:RT:MPAppleEvents.c.o	�
	:RT:MPGlobals.c.o			�
	:RT:MPEditions.c.o		�
	:RT:MPUtils.c.o			�
	:RT:MPFile.c.o				�
	:RT:MPMain.c.o				�
	:RT:MPWindow.c.o			�
	:RT:MPConsole.cp.o		�
	:RT:MPAEVTStream.cp.o	�
	:RT:MPScript.c.o			�
	:RT:MPPreferences.c.o	�
	:RT:MPPseudoFile.cp.o	�
	:RT:MPSave.c.o				�
	:RT:Mercutio.c.o			�
	::Perl:Perl.o				�
	"{CLibraries}GUSI.o"		�
	"{CLibraries}libdb.o"	�
	"{CLibraries}XL.o"		�
	:RT:MPForceLoad.cp.o

MacPerlSegments	=																	�
	-sg Main=main																		�
	-sg AppleEvents=AECommandHandlers,AECommandIssuers						�
	-sg AEObjects=ObjectAccessors,AEObjSuppt,AEObjGlue,AEObjPacking	�
	-sg MacPerl=Editions,Utils,File,Window,MPConsole,MPPseudo			�
	-sg Perl1=array,cmd,cons,consarg												�
	-sg Perl2=doarg,doio																�
	-sg Perl3=dolist,dump,form,hash,missing									�
	-sg Perl4=eval																		�
	-sg Perl5=regcomp,regexec,stab,str											�
	-sg Perl6=toke																		�
	-sg Perl7=util,SubLaunch,usersub,macperl,reentrant,icemalloc		�
	-sg Perl8=perly,perl																�
	-sg GUSI1=GUSI																		�
	-sg GUSI2=GUSIUnix,GUSIINET,GUSIPAP											�
	-sg Lib=GUSILib,INTENV,STDCLIB,STDIO,SADEV,SANELIB,CSANELib			�
	-sg DB=::hash:
	
LibFiles		= 									�
	"{CLibraries}GUSI_Everything.cfg"	�
	"{CLibraries}StdCLib.o" 				�
	"{Libraries}RTLib.o" 					�
	"{Libraries}Runtime.o" 					�
	"{Libraries}Interface.o" 				�
	"{Libraries}ToolLibs.o"					�
	"{CLibraries}Math.o" 					�
	"{CLibraries}CSANELib.o"				�
	"{Libraries}MathLib.o" 					�
	"{CLibraries}AEGizmos.lib"				�
	"{Libraries}AEObjectSupportLib.o"

PPCLibs = 											�
 	::Perl:Common:Perl.xcoff					�
 	::Perl:Common:AEGizmos.xcoff				�
	"{PPCLibraries}"XL.o							�
	"{PPCLibraries}"GUSI.xcoff					�
	"{PPCLibraries}"LibDB.xcoff				�
	"{PPCLibraries}"AppleScriptLib.xcoff	�
	"{PPCLibraries}"ObjectSupportLib.xcoff	�
	"{PPCLibraries}"InterfaceLib.xcoff		�
	"{PPCLibraries}"StdCLib.xcoff				�
	"{PPCLibraries}"StdCRuntime.o				�
	"{PPCLibraries}"CPlusLib.o					�
	"{PPCLibraries}"PPCCRuntime.o				�
	"{PPCLibraries}"PPCToolLibs.o				�
	"{PPCLibraries}"MathLib.xcoff

PPCPEFs =											�
	{PPCLibraries}GUSI 							�
	{PPCLibraries}LibDB 							�
	::Perl:Common:Perl

PPCLibEqu = 												�
	-l Perl.xcoff=Perl									�
	-l "AEGizmos.xcoff=AEGizmosPPC#0x01-0x01"		�
	-l "GUSI.xcoff=GUSI#0x01508000-0x01508000" 	�
	-l "LibDB.xcoff=LibDB#0x01738000-0x01738000" �
	-l AppleScriptLib.xcoff=AppleScriptLib 		�
	-l ObjectSupportLib.xcoff=ObjectSupportLib 	�
	-l InterfaceLib.xcoff=InterfaceLib 				�
	-l "StdCLib.xcoff=StdCLib" 						�
	-l MathLib.xcoff=MathLib

:MP:	�	:
:RT:	�	:

.c.o.PPC	�	.c
	PPCC {PPCCOptions} {DepDir}{Default}.c -o {TargDir}{Default}.c.o.PPC
.cp.o.PPC	�	.cp
	PPCC {PPCCOptions} {DepDir}{Default}.cp -o {TargDir}{Default}.cp.o.PPC

All	�	MacPerl MPRuntime MacPerlTest.Script "MacPerl Help" MPDroplet

MacPerl.68K �� {MacPerlObjects}
		Link -t APPL -c McPL {LOptions}			�
			-model far -p 								�
			{MacPerlSegments}							�
			{MacPerlObjects} 							�
			{LibFiles}									�
			-o MacPerl.68K
	SetFile -a B MacPerl.68K

MacPerl.68K	��	MacPerl.r MacPerl.rsrc MPTerminology.r :MP:FontLDEF.rsrc
	Rez -a -t APPL -c McPL MacPerl.r -o MacPerl.68K
	SetFile -a B MacPerl.68K

MacPerl.xcoff � {MPPPCObjects} ::Perl:Common:Perl.xcoff
	PPCLink 										�
		{MPPPCObjects}  {PPCLibs}			�
		-main __cplusstart					�
		-sym on				 			 		�
		-o :MacPerl.xcoff
	MakeSYM -i ::XL: -cttes :MacPerl.xcoff

MacPerl.PPC	��	MacPerl.xcoff
	MakePEF -ft 'APPL' -fc 'McPL' �
	  MacPerl.xcoff 	�
     {PPCLibEqu} �
   -o MacPerl.PPC				

MacPerl.PPC	��	MacPerl.r MacPerl.rsrc MPTerminology.r :MP:FontLDEF.rsrc
	Rez -a -t APPL -c McPL MacPerl.r 					�
		-d APPNAME=�"MacPerl�" 								�
		-o MacPerl.PPC
	SetFile -a B MacPerl.PPC
	
MacPerl �� {MacPerlObjects}
		Link -t APPL -c McPL {LOptions}			�
			-model far -p 								�
			{MacPerlSegments}							�
			{MacPerlObjects} 							�
			{LibFiles}									�
			-o MacPerl
	SetFile -a B MacPerl

MacPerl	��	MacPerl.xcoff MacPerl.r MacPerl.rsrc MPTerminology.r MPBalloons.r :MP:FontLDEF.rsrc {PPCPEFs}
	Rez -a -t APPL -c McPL MacPerl.r 					�
		-d OSL=�""{PPCLibraries}"ObjectSupportLib�"	�
		-d APPNAME=�"MacPerl�" 								�
		-o MacPerl
	MakePEF -ft 'APPL' -fc 'McPL' �
		MacPerl.xcoff 	�
     {PPCLibEqu} �
   -o MacPerl				
	MergeFragment 								�
		{PPCLibraries}AEGizmosPPC 			�
		{PPCLibraries}ObjectSupportLib 	�
		{PPCLibraries}AppleScriptLib 		�
		{PPCPEFs}								�
		MacPerl
	SetFile -a B MacPerl

:MP:FontLDEF.rsrc	�	:MP:MPPreferences.c.o
	Link -t rsrc -c RSED -rt LDEF=128 -m FONTLDEF -o :MP:FontLDEF.rsrc 	�
		:MP:MPPreferences.c.o "{Libraries}"Interface.o

MPRuntime � ForceRuntime "MacPerl Runtime"
	
"MacPerl Runtime" ��	{MPRuntimeObjects}
 		Link -t APPL -c MrPL {LOptions}		�
 			-model far								�
			{MPRuntimeObjects} 					�
			{LibFiles}								�
			-o "MacPerl Runtime"
	SetFile -a B "MacPerl Runtime"

"MacPerl Runtime"	��	MacPerl.r MacPerl.rsrc :RT:FontLDEF.rsrc
	Rez -a -t APPL -c MrPL -d RUNTIME MacPerl.r -o "MacPerl Runtime"
	SetFile -a B "MacPerl Runtime"

:RT:FontLDEF.rsrc	�	:RT:MPPreferences.c.o
	Link -t rsrc -c RSED -rt LDEF=128 -m FONTLDEF -o :RT:FontLDEF.rsrc 	�
		:RT:MPPreferences.c.o "{Libraries}"Interface.o
	
ForceRuntime �
	Set IsRuntime "-d RUNTIME"

"MacPerl Runtime".SYM � "MacPerl Runtime" "MacPerl Runtime".NJ
	ILinkToSym "MacPerl Runtime".NJ
	
"MacPerl Help" � ::Perl:MacScripts:4112Balloon ::Perl:PerlHelp
	::Perl:MacScripts:4112Balloon ::Perl:PerlHelp
	SetFile -t HELP -c McPL "MacPerl Help"

MPDroplet.code � MPDrop.c.o
	Link -t 'rsrc' -c 'RSED' -sym on					�
		MPDrop.c.o											�
		"{Libraries}Runtime.o" 							�
		"{Libraries}Interface.o"						�
		 -o MPDroplet.code	

MPDroplet � ":MacPerl Extensions:Droplet"	

":MacPerl Extensions:Droplet" � MPDroplet.code MPDroplet.r MPExtension.r MacPerl.rsrc
	Rez -t McPp -c McPL -o ":MacPerl Extensions:Droplet" MPDroplet.r

MPTerminology.r	�	MPTerminology.aete
	:macscripts:Aete2Rez MPTerminology.aete > MPTerminology.r

MPBalloons.r	�	MPBalloons.ball
	:macscripts:Balloon2Rez MPBalloons.ball

:RT:MPGlobals.c.o	�	MPGlobals.h
:MP:MPGlobals.c.o	�	MPGlobals.h

MacPerlTest.Script	�	:t:MakeMacPerlTest
	Set OldDir `Directory`
	Directory t
		MakeMacPerlTest :�:�.t > {OldDir}MacPerlTest.Script
	Directory {OldDir}
	
Distr � All
	Distribute MacPerl.distr Mac_Perl_418_appl.sit

SrcDistr �	
	Distribute PerlSrc.distr Mac_Perl_418_src.sit
