# MPW Makefile for Mac Xconq.

src = :src:

mac = :mac:

obj = :Objects:

"{obj}" � "{src}" "{mac}"

.c.o � .c
   {C} {DepDir}{Default}.c {COptions} -s {Default} -o {TargDir}{Default}.c.o

# MPW C cannot compile Xconq!

C = gC

# 68881 compilation unneeded because very little floating point is done.

COptions = -i "{src}","{mac}" -u USE_CONSOLE -d MAC -d MPW -d dangerousPattern -m -w -r -mc68020

MACOBJS = �
		"{obj}"macconq.c.o �
		"{obj}"macdesign.c.o �
		"{obj}"macdraw.c.o �
		"{obj}"macimf.c.o �
		"{obj}"macinit.c.o �
		"{obj}"maclist.c.o �
		"{obj}"macmap.c.o �
		"{obj}"macmap2.c.o �
		"{obj}"macmenus.c.o �
		"{obj}"macwins.c.o

SKELOBJS = �
		"{obj}"skelconq.c.o �
		"{obj}"cmdline.c.o �
		"{obj}"compile.c.o �
		"{obj}"mac.c.o

OBJECTS = �
		"{obj}"actions.c.o �
		"{obj}"ai.c.o �
		"{obj}"combat.c.o �
		"{obj}"generic.c.o �
		"{obj}"help.c.o �
		"{obj}"history.c.o �
		"{obj}"init.c.o �
		"{obj}"lisp.c.o �
		"{obj}"mknames.c.o �
		"{obj}"mkrivers.c.o �
		"{obj}"mkroads.c.o �
		"{obj}"mkterr.c.o �
		"{obj}"mkunits.c.o �
		"{obj}"module.c.o �
		"{obj}"mplay.c.o �
		"{obj}"mplay2.c.o �
		"{obj}"nlang.c.o �
		"{obj}"plan.c.o �
		"{obj}"plan2.c.o �
		"{obj}"read.c.o �
		"{obj}"run.c.o �
		"{obj}"score.c.o �
		"{obj}"side.c.o �
		"{obj}"tables.c.o �
		"{obj}"task.c.o �
		"{obj}"types.c.o �
		"{obj}"unit.c.o �
		"{obj}"util.c.o �
		"{obj}"version.c.o �
		"{obj}"world.c.o �
		"{obj}"write.c.o

# The program itself.

Xconq �� {MACOBJS} {OBJECTS}
	Link -t APPL -c XCNQ �
		{MACOBJS} {OBJECTS} �
		"{CLibraries}"CSANELib.o �
		"{CLibraries}"Math.o �
		"{CLibraries}"StdClib.o �
		"{Libraries}"Runtime.o �
		"{Libraries}"Interface.o �
		-o Xconq

Skelconq �� {SKELOBJS} {OBJECTS}
	Link -t MPST -c 'MPS ' �
		{SKELOBJS} {OBJECTS} �
		"{CLibraries}"CSANELib.o �
		"{CLibraries}"Math.o �
		"{CLibraries}"StdClib.o �
		"{Libraries}"Runtime.o �
		"{Libraries}"Interface.o �
		-o Skelconq

"{obj}"macconq.c.o � "{mac}"macconq.c "{mac}"mac.h "{mac}"macdefs.h
"{obj}"macdesign.c.o � "{mac}"macdesign.c "{mac}"mac.h "{mac}"macdefs.h
"{obj}"macdraw.c.o � "{mac}"macdraw.c "{mac}"mac.h "{mac}"macdefs.h
"{obj}"macimf.c.o � "{mac}"macimf.c "{mac}"mac.h "{mac}"macdefs.h
"{obj}"macinit.c.o � "{mac}"macinit.c "{mac}"mac.h "{mac}"macdefs.h
"{obj}"maclist.c.o � "{mac}"maclist.c "{mac}"mac.h "{mac}"macdefs.h
"{obj}"macmap.c.o � "{mac}"macmap.c "{mac}"mac.h "{mac}"macdefs.h
"{obj}"macmenus.c.o � "{mac}"macmenus.c "{mac}"mac.h "{mac}"macdefs.h
"{obj}"macwins.c.o � "{mac}"macwins.c "{mac}"mac.h "{mac}"macdefs.h

"{obj}"skelconq.c.o � "{src}"skelconq.c
"{obj}"cmdline.c.o � "{src}"cmdline.c
"{obj}"compile.c.o � "{src}"compile.c
"{obj}"mac.c.o � "{src}"mac.c

"{obj}"actions.c.o � "{src}"actions.c
"{obj}"ai.c.o � "{src}"ai.c
"{obj}"combat.c.o � "{src}"combat.c
"{obj}"generic.c.o � "{src}"generic.c
"{obj}"help.c.o � "{src}"help.c
"{obj}"history.c.o � "{src}"history.c
"{obj}"init.c.o � "{src}"init.c
"{obj}"lisp.c.o � "{src}"lisp.c
"{obj}"mknames.c.o � "{src}"mknames.c
"{obj}"mkrivers.c.o � "{src}"mkrivers.c
"{obj}"mkroads.c.o � "{src}"mkroads.c
"{obj}"mkterr.c.o � "{src}"mkterr.c
"{obj}"module.c.o � "{src}"module.c
"{obj}"mplay.c.o � "{src}"mplay.c
"{obj}"mplay2.c.o � "{src}"mplay2.c
"{obj}"nlang.c.o � "{src}"nlang.c
"{obj}"plan.c.o � "{src}"plan.c
"{obj}"plan2.c.o � "{src}"plan2.c
"{obj}"read.c.o � "{src}"read.c
"{obj}"run.c.o � "{src}"run.c
"{obj}"score.c.o � "{src}"score.c
"{obj}"side.c.o � "{src}"side.c
"{obj}"tables.c.o � "{src}"tables.c
"{obj}"task.c.o � "{src}"task.c
"{obj}"types.c.o � "{src}"types.c
"{obj}"unit.c.o � "{src}"unit.c
"{obj}"util.c.o � "{src}"util.c
"{obj}"version.c.o � "{src}"version.c
"{obj}"world.c.o � "{src}"world.c
"{obj}"write.c.o � "{src}"write.c

