###
#
# Makefile - For GNU Patch 2.1, Macintosh System 7.5, MPW 3.4
#			 by Christopher E. Hyde, 95-06-30
#
###

I			=	{S}Include:
Dump		=	{O}Patch.Dump

CDefs		=	-d PROGRAM=�"Patch�"
COpts		=	-ansi on -mc68020 -b2 {CDebugOpts} -i "{I}" {CDefs}
COptions	=	{COpts} -load "{Dump}"
StdLibs		=	"{Libraries}"Stubs.o "{CLibraries}"StdCLib.o "{Libraries}"MacRuntime.o �
				"{Libraries}"IntEnv.o "{Libraries}"Interface.o
Link		=	Link -d -t MPST -c 'MPS ' -sg Utils=STDCLIB,STDIO,SANELIB,INTENV,GetOps

Objs		=	{O}backupfile.c.o {O}inp.c.o {O}patch.c.o {O}pch.c.o {O}util.c.o �
				{O}version.c.o {O}MacUtils.c.o {O}getopt.c.o {O}getopt1.c.o
Headers		=	{I}config.h {I}util.h {I}patchlevel.h {I}version.h �
				{I}sys/stat.h {I}sys/types.h

{O}			�	{S}
{O}.c.o		�	{S}.c
	{C} {COptions} {DepDir}{Default}.c -o {TargDir}{Default}.c.o

{Objs}		��	{Dump}

{Dump}		�	{I}Includes.h {Headers} #{S}Makefile
	SC {I}Includes.h -o {O}temp-dump.o {COpts} -dumpc {Dump}

{Prog}		�	{Objs}
	{Link} -o {Targ} {Deps} {StdLibs} #-map
	SetVersion {Targ} -t 'vers' -i 1 -country Britain -version '2.1' �
				-verstring '^ � 1991 Free Software Foundation, Inc.'
	SetVersion {Targ} -t 'vers' -i 2 -country Britain -version '1.0' �
				-verstring 'Chris� Tools ^'

Patch		�	{Objs} {Prog}
