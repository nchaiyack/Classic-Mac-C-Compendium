###
#
# Makefile - For Byacc 1.9, Macintosh System 7.5, MPW 3.4
#			 by Christopher E. Hyde, 95-06-30
#
###

Dump		=	{O}Byacc.Dump

CDefs		=	-d PROGRAM=¶"BYacc¶" -d SIG_TYPE=void
COpts		=	-ansi on -mc68020 -b2 {CDebugOpts} -i "{S}" {CDefs}
COptions	=	{COpts} -load "{Dump}"
StdLibs		=	"{Libraries}"Stubs.o "{CLibraries}"StdCLib.o "{Libraries}"MacRuntime.o ¶
				"{Libraries}"IntEnv.o "{Libraries}"Interface.o
Link		=	Link -d -t MPST -c 'MPS ' -sg Main=STDCLIB,STDIO,SANELIB,INTENV

Objs1		=	{O}main.c.o {O}closure.c.o {O}error.c.o {O}lalr.c.o {O}lr0.c.o
Objs2		=	{O}mkpar.c.o {O}output.c.o {O}reader.c.o {O}skeleton.c.o ¶
				{O}symtab.c.o {O}verbose.c.o {O}warshall.c.o

AllObjs		=	{Objs1} {Objs2}


{O}			Ä	{S}
{O}.c.o		Ä	{S}.c
	{C} {COptions} {DepDir}{Default}.c -o {TargDir}{Default}.c.o

{AllObjs}	ÄÄ	{Dump}

{Dump}		Ä	{S}defs.h #{S}Makefile
	SC {S}defs.h -o {O}temp-dump.o {COpts} -dumpc {Dump}

{O}ByObj2	Ä	{Objs2}
	Lib -d -sg Byacc2 -o {Targ} {Deps}

{Prog}		Ä	{Objs1} {O}ByObj2
	{Link} -o {Targ} {Deps} {StdLibs} #-map
	SetVersion {Targ} -t 'vers' -i 1 -country Britain -version '1.9' ¶
				-verstring '^ © The Regents of the University of California 1993.'
	SetVersion {Targ} -t 'vers' -i 2 -country Britain -version '1.0' ¶
				-verstring 'ChrisÕ Tools ^'

Byacc		Ä	{AllObjs} {Prog}
