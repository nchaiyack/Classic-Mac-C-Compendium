#
#	MacGzip Makefile
#
#	Targets:
#			all			- MacGzip.68k, MacGzip.PPC and MacGzip.fat.
#			MacGzip.fat	- same as all.
#			clean		- delete object files.
#			realclean	- clean and delete targets.
#
#	modification history
#	--------------------
#	01a,23may95,ejo  written.
#

CC			= SC
PC			= MrC
INCLUDES	= -i : -i ::Mac: -i ::GNU:
DEFINES		= -d MPW
SYMOPTS		=
CCOPTS		= -O all -r -A {SYMOPTS} {INCLUDES} {DEFINES}
PCOPTS		= -opt speed {SYMOPTS} {INCLUDES} {DEFINES}
SETFILEOPTS	= -t APPL -c Gzip
LINKOPTS	= {SYMOPTS}
APPNAME		= MacGzip

PPCLINKOPTS	= �
		{SYMOPTS} �
		-librename InterfaceLib.xcoff=InterfaceLib �
		-librename StdCLib.xcoff=StdCLib �
		-librename MathLib.xcoff=MathLib

PPCOBJECTS = �
		:obj:bits.c.x :obj:deflate.c.x :obj:gzip.c.x �
		:obj:inflate.c.x :obj:lzw.c.x :obj:trees.c.x :obj:unlzh.c.x �
		:obj:unlzw.c.x :obj:unpack.c.x :obj:unzip.c.x :obj:util.c.x �
		:obj:zip.c.x :obj:catinfo.c.x :obj:DoOpen.c.x :obj:ICMappings.c.x �
		:obj:MacAE.c.x :obj:MacErrors.c.x :obj:MacGzip.c.x :obj:PopUp.c.x �
		:obj:SPDCProg.c.x :obj:stat.c.x :obj:ThePrefs.c.x :obj:utime.c.x
		
PPCLIBS = �
		:lib:ICGlue.x

68KOBJECTS = �
		:obj:bits.c.o :obj:deflate.c.o :obj:gzip.c.o �
		:obj:inflate.c.o :obj:lzw.c.o :obj:trees.c.o :obj:unlzh.c.o �
		:obj:unlzw.c.o :obj:unpack.c.o :obj:unzip.c.o :obj:util.c.o �
		:obj:zip.c.o :obj:catinfo.c.o :obj:DoOpen.c.o :obj:ICMappings.c.o �
		:obj:MacAE.c.o :obj:MacErrors.c.o :obj:MacGzip.c.o :obj:PopUp.c.o �
		:obj:SPDCProg.c.o :obj:stat.c.o :obj:ThePrefs.c.o :obj:utime.c.o

68KLIBS = �
		:lib:ICGlue.o

MACPPCLIBS = �
		"{PPCLibraries}"StdCLib.xcoff �
		"{PPCLibraries}"InterfaceLib.xcoff �
		"{PPCLibraries}"MathLib.xcoff �
		"{PPCLibraries}"StdCRuntime.o �
		"{PPCLibraries}"PPCCRuntime.o �
		"{PPCLibraries}"PPCToolLibs.o
		
MAC68KLIBS = �
		"{CLibraries}"StdClib.o �
		"{Libraries}"MacRuntime.o �
		"{Libraries}"IntEnv.o �
		"{Libraries}"Interface.o �

RES = {APPNAME}.r

# Default PPC rule.
.c.x � .c
	{PC} {DepDir}{Default}.c {PCOPTS} -o {TargDir}{Default}.c.x

# Default 68K rule.
.c.o � .c
	{CC} {DepDir}{Default}.c {CCOPTS} -s {Default} -o {TargDir}{Default}.c.o

# Targets

all � {APPNAME}.fat

{APPNAME}.fat �� {APPNAME}.68k  {APPNAME}.PPC
	Duplicate -y {APPNAME}.PPC {Targ}
	Rez {RES} -o {APPNAME}.fat -append -d FAT

{APPNAME}.fat �� {APPNAME}.68k {APPNAME}.PPC {RES}
	Setfile {SETFILEOPTS} {Targ}
	
{APPNAME}.PPC �� {PPCOBJECTS} {RES}
	PPCLink {PPCLINKOPTS} -o {Targ} {PPCOBJECTS} {PPCLIBS} {MACPPCLIBS}

{APPNAME}.PPC �� {PPCOBJECTS} {RES}
	Rez {RES} -o {Targ} -append -d POWERPC -d APPNAME=�"{Targ}�" -d STACKSIZE=0

{APPNAME}.PPC �� {PPCOBJECTS} {RES}
	Setfile {SETFILEOPTS} {Targ}

{APPNAME}.68k �� {68KOBJECTS} {RES}
	Link {LINKOPTS} -o {Targ} {68KOBJECTS} {68KLIBS} {MAC68KLIBS}

{APPNAME}.68k �� {68KOBJECTS} {RES}
	Rez {RES} -o {Targ} -append

{APPNAME}.68k �� {68KOBJECTS} {RES}
	Setfile {SETFILEOPTS} {Targ}

{RES} � ::{APPNAME}.project.rsrc {APPNAME}.r.ini
	Duplicate -y {APPNAME}.r.ini {RES}
	DeRez ::{APPNAME}.project.rsrc -d SystemSevenOrLater=1 >> {RES}
	Echo "�#endif" >> {RES}

clean �
	Delete -i {68KOBJECTS} {PPCOBJECTS}

realclean � clean
	Delete -i {APPNAME}.68k {APPNAME}.PPC {APPNAME}.fat {RES}

:obj: � ::GNU: ::Mac:

# PPC deps
:obj:bits.c.x � ::GNU:tailor.h ::GNU:gzip.h ::GNU:crypt.h
:obj:deflate.c.x � ::GNU:tailor.h ::GNU:gzip.h ::GNU:lzw.h
:obj:gzip.c.x � ::GNU:tailor.h ::GNU:gzip.h ::GNU:lzw.h ::GNU:revision.h ::Mac:MacErrors.h ::Mac:ThePrefs.h ::Mac:SPDProg.h
:obj:inflate.c.x � ::GNU:tailor.h ::GNU:gzip.h ::Mac:MacErrors.h
:obj:lzw.c.x � ::GNU:tailor.h ::GNU:gzip.h ::GNU:lzw.h ::Mac:MacErrors.h
:obj:trees.c.x � ::GNU:tailor.h ::GNU:gzip.h
:obj:unlzh.c.x � ::GNU:tailor.h ::GNU:gzip.h ::GNU:lzw.h
:obj:unlzw.c.x � ::GNU:tailor.h ::GNU:gzip.h ::GNU:lzw.h ::Mac:MacErrors.h ::Mac:SPDProg.h
:obj:unpack.c.x � ::GNU:tailor.h ::GNU:gzip.h ::GNU:crypt.h
:obj:unzip.c.x � ::GNU:tailor.h ::GNU:gzip.h ::GNU:crypt.h ::Mac:MacErrors.h
:obj:util.c.x � ::GNU:tailor.h ::GNU:gzip.h ::GNU:crypt.h ::Mac:MacErrors.h ::Mac:SPDProg.h

# this cannot be optimized (bad code for put_long) */
:obj:zip.c.x � ::GNU:tailor.h ::GNU:gzip.h ::GNU:crypt.h ::Mac:SPDProg.h
	{PC} ::GNU:zip.c {SYMOPTS} {INCLUDES} {DEFINES} -o :obj:zip.c.x

:obj:catinfo.c.x � ::Mac:catinfo.h
:obj:DoOpen.c.x � ::Mac:ICMappings.h ::Mac:MacGzip.h ::Mac:ThePrefs.h ::Mac:MacErrors.h ::GNU:tailor.h
:obj:ICMappings.c.x � ::Mac:ICAPI.h ::Mac:ICKeys.h ::Mac:ICMappings.h
:obj:MacAE.c.x � ::Mac:MacAE.h ::Mac:ThePrefs.h ::Mac:MacErrors.h
:obj:MacErrors.c.x � ::Mac:MacErrors.h
:obj:MacGzip.c.x � ::Mac:MacGzip.h ::Mac:ThePrefs.h ::Mac:MacErrors.h ::Mac:SPDProg.h ::Mac:MacAE.h
:obj:PopUp.c.x � ::Mac:PopUp.h
:obj:SPDCProg.c.x � ::Mac:SPDProg.h
:obj:stat.c.x � ::Mac:catinfo.h ::Mac:sys/stat.h ::Mac:sys/types.h
:obj:ThePrefs.c.x � ::Mac:ThePrefs.h ::Mac:MacErrors.h ::Mac:PopUp.h
:obj:utime.c.x � ::Mac:catinfo.h ::Mac:utime.h

# 68K deps
:obj:bits.c.o � ::GNU:tailor.h ::GNU:gzip.h ::GNU:crypt.h
:obj:deflate.c.o � ::GNU:tailor.h ::GNU:gzip.h ::GNU:lzw.h
:obj:gzip.c.o � ::GNU:tailor.h ::GNU:gzip.h ::GNU:lzw.h ::GNU:revision.h ::Mac:MacErrors.h ::Mac:ThePrefs.h ::Mac:SPDProg.h
:obj:inflate.c.o � ::GNU:inflate.c ::GNU:tailor.h ::GNU:gzip.h ::Mac:MacErrors.h
:obj:lzw.c.o � ::GNU:tailor.h ::GNU:gzip.h ::GNU:lzw.h ::Mac:MacErrors.h
:obj:trees.c.o � ::GNU:tailor.h ::GNU:gzip.h
:obj:unlzh.c.o � ::GNU:tailor.h ::GNU:gzip.h ::GNU:lzw.h
:obj:unlzw.c.o � ::GNU:tailor.h ::GNU:gzip.h ::GNU:lzw.h ::Mac:MacErrors.h ::Mac:SPDProg.h
:obj:unpack.c.o � ::GNU:tailor.h ::GNU:gzip.h ::GNU:crypt.h
:obj:unzip.c.o � ::GNU:tailor.h ::GNU:gzip.h ::GNU:crypt.h ::Mac:MacErrors.h
:obj:util.c.o � ::GNU:tailor.h ::GNU:gzip.h ::GNU:crypt.h ::Mac:MacErrors.h ::Mac:SPDProg.h
:obj:zip.c.o � ::GNU:tailor.h ::GNU:gzip.h ::GNU:crypt.h ::Mac:SPDProg.h
:obj:catinfo.c.o � ::Mac:catinfo.h
:obj:DoOpen.c.o � ::Mac:ICMappings.h ::Mac:MacGzip.h ::Mac:ThePrefs.h ::Mac:MacErrors.h ::GNU:tailor.h
:obj:ICMappings.c.o � ::Mac:ICAPI.h ::Mac:ICKeys.h ::Mac:ICMappings.h
:obj:MacAE.c.o � ::Mac:MacAE.h ::Mac:ThePrefs.h ::Mac:MacErrors.h
:obj:MacErrors.c.o � ::Mac:MacErrors.h
:obj:MacGzip.c.o � ::Mac:MacGzip.h ::Mac:ThePrefs.h ::Mac:MacErrors.h ::Mac:SPDProg.h ::Mac:MacAE.h
:obj:PopUp.c.o � ::Mac:PopUp.h
:obj:SPDCProg.c.o � ::Mac:SPDProg.h
:obj:stat.c.o � ::Mac:catinfo.h ::Mac:sys/stat.h ::Mac:sys/types.h
:obj:ThePrefs.c.o � ::Mac:ThePrefs.h ::Mac:MacErrors.h ::Mac:PopUp.h
:obj:utime.c.o � ::Mac:catinfo.h ::Mac:utime.h
