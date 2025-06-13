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

OBJS=	long.c.o getopt.c.o memory.c.o time.c.o filepart.c.o identify.c.o strtbl.c.o rtdb.c.o ¶
	mlocal.c.o literals.c.o rswitch.c.o alloc.c.o long.c.o getopt.c.o memory.c.o time.c.o ¶
	save.c.o rswitch.c.o redirerr.c.o dlrgint.c.o ipp.c.o

.c.o Ä .c
	{C} {DepDir}{Default}.c -o {TargDir}{Default}.c.o -s {Default} {COptions}

commonÄ		doincl.c.o patchstr.c.o
		{CC} {LinkOpts} -o doincl doincl.c.o
		{CC} {LinkOpts} -o patchstr patchstr.c.o
		:-.:doincl -o :::bin:rt.h ::h:rt.h
		cp patchstr :::bin
			
xpmÄ
		cd ::xpm; {MAKE} libXpm.a
		cp ::xpm:libXpm.a :::bin
		-(test -f :::NoRanlib) || (ranlib :::bin:libXpm.a)

{OBJS}Ä ::h:define.h ::h:config.h ::h:cstructs.h ::h:mproto.h  ::h:path.h ¶
	  ::h:typedefs.h ::h:proto.h ::h:cpuconf.h

identify.c.oÄ ::h:version.h

ipp.c.oÄ ::h:features.h

literals.c.oÄ ::h:esctab.h

rtdb.c.oÄ ::h:version.h icontype.h

dlrgint.c.oÄ ::h:rproto.h ::h:rexterns.h ::h:rmacros.h ::h:rstructs.h

#xwindow.c.oÄ ::h:xicon.h ::h:xwin.h

rswitch.c.oÄ {RSWITCH}
	{CC} -c {OFLAGS} {RSWITCH}


#  The following section is needed if changes are made to the Icon grammar,
#  but it is not run as part of the normal installation process.  If it is
#  needed, it is run by changing ::icont:Makefile :and:or ::iconc:Makefile;
#  see the comments there for details.  icont must be in the search path
#  for this section to work.
 
gfilesÄ			lextab.h yacctok.h fixgram pscript
 
lextab.h yacctok.hÄ	tokens.txt op.txt mktoktab
			:mktoktab 
 
mktoktabÄ		mktoktab.icn
			icont -s mktoktab.icn
 
fixgramÄ		fixgram.icn
			icont -s fixgram.icn
 
pscriptÄ		pscript.icn
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
#icontype.hÄ typespec.txt typespec
#	typespec <typespec.txt >icontype.h
#
#typespecÄ typespec.icn
#	icont typespec
