#  The -D{D} option to COptions makes it possible to specify a definition
#  at the Make command level without revising the Makefile, as
#  in
#
#	make D=Debug
#
D=NOTHING
CC=C
COptions= -b2 -mbg off -r -d MPW -d MPWFncs -d MacToolboxFncs -opt full
LinkOpts=
SHELL=bin:sh
RSWITCH=rswitch.a
MAKE = make

OBJS=	long.c.o getopt.c.o memory.c.o time.c.o filepart.c.o identify.c.o strtbl.c.o rtdb.c.o �
	mlocal.c.o literals.c.o rswitch.c.o alloc.c.o long.c.o getopt.c.o memory.c.o time.c.o �
	save.c.o rswitch.c.o redirerr.c.o dlrgint.c.o ipp.c.o

.c.o � .c
	{C} {DepDir}{Default}.c -o {TargDir}{Default}.c.o -s {Default} {COptions}

common�		doincl.c.o patchstr.c.o
		{CC} {LinkOpts} -o doincl doincl.c.o
		{CC} {LinkOpts} -o patchstr patchstr.c.o
		:-.:doincl -o :::bin:rt.h ::h:rt.h
		cp patchstr :::bin
			
xpm�
		cd ::xpm; {MAKE} libXpm.a
		cp ::xpm:libXpm.a :::bin
		-(test -f :::NoRanlib) || (ranlib :::bin:libXpm.a)

{OBJS}� ::h:define.h ::h:config.h ::h:cstructs.h ::h:mproto.h  ::h:path.h �
	  ::h:typedefs.h ::h:proto.h ::h:cpuconf.h

identify.c.o� ::h:version.h

ipp.c.o� ::h:features.h

literals.c.o� ::h:esctab.h

rtdb.c.o� ::h:version.h icontype.h

dlrgint.c.o� ::h:rproto.h ::h:rexterns.h ::h:rmacros.h ::h:rstructs.h

#xwindow.c.o� ::h:xicon.h ::h:xwin.h

rswitch.c.o� {RSWITCH}
	{CC} -c {OFLAGS} {RSWITCH}


#  The following section is needed if changes are made to the Icon grammar,
#  but it is not run as part of the normal installation process.  If it is
#  needed, it is run by changing ::icont:Makefile :and:or ::iconc:Makefile;
#  see the comments there for details.  icont must be in the search path
#  for this section to work.
 
gfiles�			lextab.h yacctok.h fixgram pscript
 
lextab.h yacctok.h�	tokens.txt op.txt mktoktab
			:mktoktab 
 
mktoktab�		mktoktab.icn
			icont -s mktoktab.icn
 
fixgram�		fixgram.icn
			icont -s fixgram.icn
 
pscript�		pscript.icn
			icont -s pscript.icn
 
 
 
#  The following section is commented out because it does not need to be
#  performed unless changes are made to typespec.txt. Such changes 
#  and are not part of the installation process.  However, if the
#  distribution files are unloaded in a fashion such that their dates
#  are not set properly, the following section would be attempted.
#
#  Note that if any changes are made to the file mentioned above, the
#  comment characters at the beginning of the following lines should be
#  removed.
#
#  Note that icont must be on your search path for this.
#
#
#icontype.h� typespec.txt typespec
#	typespec <typespec.txt >icontype.h
#
#typespec� typespec.icn
#	icont typespec
