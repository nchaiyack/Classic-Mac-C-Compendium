###
#
# Makefile - For Flex 2.5.2, Macintosh System 7.5, MPW 3.4
#			 by Christopher E. Hyde, 95-06-30
#
###

Dump		=	{O}Flex.Dump

COpts		=	-ansi on -mc68020 -b2 {CDebugOpts} -i "{S}"
COptions	=	{COpts} -load "{Dump}"
StdLibs		=	"{Libraries}"Stubs.o "{CLibraries}"StdCLib.o "{Libraries}"MacRuntime.o �
				"{Libraries}"IntEnv.o "{Libraries}"Interface.o "{Libraries}"ToolLibs.o
Link		=	Link -d -t MPST -c 'MPS ' -sg Main=STDCLIB,STDIO,SANELIB,INTENV
Rez			=	Rez -a -t MPST -c 'MPS ' -s {S}

Objs1		=	{O}main.c.o {O}scan.c.o
Objs2		=	{O}nfa.c.o {O}sym.c.o {O}tblcmp.c.o {O}MacUtils.c.o {O}misc.c.o {O}yylex.c.o
Objs3		=	{O}gen.c.o {O}ccl.c.o {O}ecs.c.o {O}dfa.c.o {O}parse.c.o

FlexObjs	=	{Objs1} {O}FlexObj2 {O}FlexObj3
AllObjs		=	{Objs1} {Objs2} {Objs3}


{O}			�	{S}
{O}.c.o		�	{S}.c
	{C} {COptions} {DepDir}{Default}.c -o {TargDir}{Default}.c.o

{AllObjs}	��	{Dump}

{Dump}		�	{S}flexdef.h {S}config.h #{S}Makefile
	SC {S}flexdef.h -o {O}temp-dump.o {COpts} -dumpc {Dump}

{O}FlexObj2	�	{Objs2}
	Lib -d -sg Flex2 -o {Targ} {Deps}

{O}FlexObj3	�	{Objs3}
	Lib -sg Flex3 -o {Targ} {Deps}

#{S}parse.c	�	{S}parse.y
#	BYacc {S}parse.y

#{S}scan.c	�	{S}scan.l
#	Flex {S}scan.l

{Prog}		��	{S}flex.skl
	Echo "data 'TEXT' (128) �{ $$Read(�"{Deps}�"); �};" | {Rez} -o {Targ}
	SetVersion {Targ} -t 'vers' -i 1 -country Britain -version '2.5.2' �
				-verstring '^ � The Regents of the University of California 1990.'
	SetVersion {Targ} -t 'vers' -i 2 -country Britain -version '1.0' �
				-verstring 'Chris� Tools ^'

{Prog}		��	{FlexObjs}
	{Link} -o {Targ} {Deps} {StdLibs} -sg Flex2=FlexScanner #-map

{Prog}		��	{S}scan.r
	{Rez} -o {Targ} {Deps}
{Prog}		��	{S}parse.r
	{Rez} -o {Targ} {Deps}

Flex		�	{AllObjs} {Prog}
