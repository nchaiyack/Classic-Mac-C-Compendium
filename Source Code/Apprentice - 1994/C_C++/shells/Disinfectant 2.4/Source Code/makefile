#	makefile - Sample make file.
#
#	Targets:
#	--------
#
#	Sample - Sample program.  Default = debugging symbols.
#	wrap, cvrt - Tools.
#
#	Shell scripts:
#	--------------
#
#	rel - Build release version, with no debugging symbols.
#
#	Tools.  (r) = reusable, public:
#	-------------------------------
#	
#	wrap - Wrap paragraphs. (r)
#	cvrt - Compile document. (r)
#	
#	General-purpose C modules.  Reusable:
#	-------------------------------------
#	
#	utl - Utilities.
#	gff - Get file or folder.
#	vol - Volume selection.
#	scn - Volume scanning.
#	rep - Report manager.
#	rpp - Report printing manager.
#	ldf - Custom list definition function for reports.
#	hlp - Help window manager.
#	
#	Other C Modules.:
#	-----------------
#	
#	glob - Global definitions.
#	init - Initialization.
#	misc - Miscellany.
#	wstm - Window state manager.
#	mssg = Message manager.
#	scan - Scanning manager.
#	abou - About window manager.
#	help - Help window manager.
#	pref - Preferences window manager.
#	main - Main window manager.
#	prog - Main program.
#	
#	precompile - Precompile standard system header files.
#
#	Assembler modules:
#	------------------
#
#	unmount - UnmountVol trap patch.
#
#	Resource files:
#	---------------
#
#	picts - Pictures.
# 	sounds - Sounds.
#	rez.r - All other resources.
#
#	Other files:
#	------------
#
#	rez.h - Constant definitions for the resources.
#	doc - About window text as maintained in MS Word.
#	rephead.t - Initial text for the report field.
#
#	C Coding conventions:
#	---------------------
#
#	Public reusable c modules have three character names.
#	Private c modules have four character names.
#
#	References to exported types and functions are always qualified 
#	by the module name:
#
#		module name_type name
#		module name_function name
#
#	e.g., prog_DoEvent.
#
#	Global variable and type names have their first letter capitalized.
#	Local variable and type names have their first letter in lower case.
#	Module names are all lower case.
#	Constant names are all lower case.
#
#	With a few exceptions, all code in a module is placed in its own 
#	segment, with the segment name equal to the module name.  The segments
#	are combined in the link command.


#	Assembly, compilation, and linking options.
#
#	The debugging options should be turned off when building release versions.  
#	See the "rel" shell script.

AOptions = -wb
COptions = -mbg ch8 -sym on -r
LDebugOptions = -sym on

#	Sample object files.

ObjFiles = 	prog.c.o ¶
			main.c.o ¶
			glob.c.o ¶
			vol.c.o ¶
			scn.c.o ¶
			rep.c.o ¶
			rpp.c.o ¶
			hlp.c.o ¶
			gff.c.o ¶
			utl.c.o ¶
			abou.c.o ¶
			wstm.c.o ¶
			mssg.c.o ¶
			scan.c.o ¶
			init.c.o ¶
			misc.c.o ¶
			pref.c.o ¶
			help.c.o ¶
			unmount.a.o

#	Additional dependencies for C and assembler modules.  MPW's default
#	rules and build commands are used for these modules.

abou.c.o Ä precompile utl.h rez.h rpp.h glob.h wstm.h abou.h help.h
gff.c.o Ä precompile gff.h
glob.c.o Ä precompile rpp.h rez.h glob.h
help.c.o Ä precompile utl.h rpp.h rez.h hlp.h glob.h ¶
	wstm.h help.h misc.h main.h unmount.h vol.h
hlp.c.o Ä precompile rep.h rpp.h utl.h hlp.h doc.h
init.c.o Ä precompile utl.h rpp.h rez.h glob.h ¶
	pref.h help.h init.h misc.h main.h abou.h prog.h
main.c.o Ä precompile vol.h rep.h rpp.h scn.h utl.h rez.h glob.h ¶
	wstm.h misc.h scan.h help.h prog.h main.h unmount.h mssg.h
misc.c.o Ä precompile utl.h rep.h rpp.h scn.h vol.h rez.h glob.h ¶
	unmount.h misc.h wstm.h unmount.h prog.h main.h mssg.h
mssg.c.o Ä precompile utl.h rep.h scn.h vol.h rpp.h rez.h glob.h mssg.h
pref.c.o Ä precompile utl.h rpp.h rez.h glob.h wstm.h pref.h help.h misc.h
prog.c.o Ä precompile vol.h scn.h rep.h rpp.h hlp.h utl.h rez.h ¶
	glob.h wstm.h abou.h pref.h help.h misc.h ¶
	init.h prog.h main.h mssg.h
rep.c.o Ä precompile rep.h utl.h doc.h
rpp.c.o Ä precompile rpp.h utl.h doc.h
scn.c.o Ä precompile scn.h utl.h
utl.c.o Ä precompile utl.h
vol.c.o Ä precompile vol.h utl.h
scan.c.o Ä precompile utl.h rpp.h rep.h vol.h rez.h scn.h gff.h ¶
	glob.h misc.h main.h prog.h
wstm.c.o Ä precompile utl.h rpp.h rez.h glob.h wstm.h
unmount.a.o Ä precompile unmount.h

#	Precompiled system headers.

precompile Ä precompile.c
	C {COptions} precompile.c

#	List definition procedure.

ldf.c.o Ä ldf.c doc.h precompile
	C {COptions} ldf.c -o ldf.c.o -r -w	
	
#	Sample dependencies.

Sample ÄÄ rez.r picts sounds
	Rez rez.r -a -o Sample
	
Sample ÄÄ rephead.t cvrt
	cvrt rephead.t -o Sample -i 150
	
Sample ÄÄ doc wrap cvrt picts
	wrap -r 335 -p <doc >wraptemp 
	cvrt wraptemp -o Sample -i 200 -t 128 -g 128 -c 128 ¶
		-p picts -w 345 -h 12
	delete wraptemp

Sample ÄÄ {ObjFiles}
	Link -t APPL -c SAMP -ra =$30 {LDebugOptions} ¶
		-sg main1seg=Main,prog,utl,misc,main ¶
		-sg main2seg=wstm,vol,rep,repi,misc,scn,scan,mssg ¶
		-sg initseg=init,voli ¶
		-sg auxseg=abou,help,pref,hlp,gff ¶
		-sg rppseg=rpp ¶
		{ObjFiles} ¶
		"{Libraries}"Interface.o ¶
		"{CLibraries}"CRuntime.o ¶
		"{CLibraries}"StdCLib.o ¶
		"{CLibraries}"CInterface.o ¶
		-o Sample
	Setfile -a B Sample
		
Sample ÄÄ ldf.c.o
	Link -rt LDEF=128 -m MYLIST ¶
	 	ldf.c.o ¶
		"{Libraries}"Interface.o ¶
		-o Sample
		
#	wrap tool.		
		
wrap.c.o Ä wrap.c precompile
	C {COptions} wrap.c -w
	
wrapObjects = wrap.c.o

wrap Ä {wrapObjects}
	Link -w -c 'MPS ' -t MPST ¶
		{wrapObjects} ¶
		"{Libraries}"stubs.o ¶
		"{CLibraries}"CRuntime.o ¶
		"{Libraries}"Interface.o ¶
		"{CLibraries}"StdCLib.o ¶
		"{CLibraries}"CInterface.o ¶
		-o wrap
		
#	cvrt tool.		
		
cvrt.c.o Ä cvrt.c doc.h precompile
	C {COptions} cvrt.c
	
cvrtObjects = cvrt.c.o

cvrt Ä {cvrtObjects}
	Link -w -c 'MPS ' -t MPST ¶
		{cvrtObjects} ¶
		"{Libraries}"stubs.o ¶
		"{CLibraries}"CRuntime.o ¶
		"{Libraries}"Interface.o ¶
		"{CLibraries}"StdCLib.o ¶
		"{CLibraries}"CInterface.o ¶
		-o cvrt
