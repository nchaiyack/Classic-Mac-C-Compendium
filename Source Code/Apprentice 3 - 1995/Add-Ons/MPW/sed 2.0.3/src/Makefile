# File:		Makefile
# Target:	sed
# Sources:	sed.c utils.c rx.c getopt.c getopt1.c sed.r
# Created:	Fri, Feb 5, 1993 2:15 PM
# Author:	Franklin Chen

# GNU sed
# Wed, Jan 19, 1994	Version 2.03

#### Start of system configuration section. ####

C = gC

# Things you might add to DEFS:
# -DSTDC_HEADERS	If you have ANSI C headers and libraries.
# -DUSG			If you have System V/ANSI C string
#			and memory functions and headers.
# -D__CHAR_UNSIGNED__	If type `char' is unsigned.
#			gcc defines this automatically.
# -DNO_VFPRINTF		If you lack vprintf function (but have _doprnt).

DEFS = �
	-d MPW �
#	-d __STDC__ �
#	-d HAVE_ALLOCA_H �
	-d HAVE_STRING_H	# for sed.c �
	-d STDC_HEADERS �
	-d USG �
	-d HAVE_MEMCPY # sed.c �
	-d HAVE_VPRINTF # utils.c

COptions = �
	-mc68020 �
	-warnings full �
	-opt full �
#	-finline-functions �
	-fomit-frame-pointer �
	-fstrength-reduce

LOptions = -d -c 'MPS ' -t MPST
extra_objs =

LIBS =	"{CLibraries}"StdClib.o �
	"{Libraries}"Stubs.o �
	"{Libraries}"Runtime.o �
	"{Libraries}"Interface.o �
	"{Libraries}"ToolLibs.o

# Where to install the executable.
bindir = "{MPW}Tools:"

#### End of system configuration section. ####

objs = sed.c.o utils.c.o rx.c.o getopt.c.o getopt1.c.o
srcs = sed.c utils.c rx.c getopt.c getopt1.c alloca.c

all_objs = {objs} {extra_objs}
all �	sed

.c.o � .c
	{C} {DEFS} {COptions} -s {default} {depDir}{default}.c -o {targDir}{default}.c.o 

sed ��	{all_objs}
	Link {LOptions} {all_objs} {LIBS} �
		-sg Main=sed,utils,getopt,getopt1 �
		-sn Rx=rx �
		-o sed
sed �� sed.r
	Rez sed.r -append -o sed

sed.c.o rx.c.o � rx.h
sed.c.o getopt1.c.o � getopt.h

install �	all install_already_built

install_already_built �
	Duplicate -y sed "{bindir}"
	Duplicate -y sed.Help "{MPW}"sed.Help

clean �
	Delete -i sed �.c.o �.makeout

alloca.o � alloca.c
