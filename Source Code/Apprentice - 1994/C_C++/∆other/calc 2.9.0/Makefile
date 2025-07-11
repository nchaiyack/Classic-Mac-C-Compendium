#
# Copyright (c) 1993 David I. Bell and Landon Curt Noll
# Permission is granted to use, distribute, or modify this source,
# provided that this copyright notice remains intact.
#
# Arbitrary precision calculator.
#
# calculator by David I. Bell
# makefile by Landon Curt Noll

##############################################################################
#-=-=-=-=-=-=-=-=- You may want to change some values below -=-=-=-=-=-=-=-=-#
##############################################################################

# Determine the type of terminal controls that you want to use
#
#	VARARG value	  meaning
#	------------	  -------
#	(nothing)	  let the makefile guess at what you need
#	-DUSE_TERMIOS	  use struct termios from <termios.h>
#	-DUSE_TERMIO 	  use struct termios from <termio.h>
#	-DUSE_SGTTY    	  use struct sgttyb from <sys/ioctl.h>
#
TERMCONTROL=
#TERMCONTROL= -DUSE_TERMIOS
#TERMCONTROL= -DUSE_TERMIO
#TERMCONTROL= -DUSE_SGTTY

# Determine the type of varargs that you want to use
#
#	VARARG value	  meaning
#	------------	  -------
#	(nothing)	  let the makefile guess at what you need
#	STDARG	    	  you have ANSI C and /usr/include/stdarg.h
#	VARARGS	    	  you have /usr/include/varargs.h
#	SIMULATE_STDARG   use simulated ./stdarg.h
#
# Try defining VARARG to be nothing.  The makefile will look for the
# needed .h files, trying for stdarg.h first.
#
VARARG=
#VARARG= STDARG
#VARARG= VARARGS
#VARARG= SIMULATE_STDARG

# If your system does not have a vsprintf() function, you could be in trouble.
#
#	vsprintf(stream, format, ap)
#
# This function works like sprintf except that the 3rd arg is a va_list
# strarg (or varargs) list.
#
# If you have vsprintf, then define DONT_HAVE_VSPRINTF to be an empty string.
# Some old systems do not have vsprintf().  If you do not have vsprintf()
# then define DONT_HAVE_VSPRINTF to be -DDONT_HAVE_VSPRINTF and hope for
# the best.
#
DONT_HAVE_VSPRINTF=
#DONT_HAVE_VSPRINTF= -DDONT_HAVE_VSPRINTF

# Determine the byte order of your machine
#
#    Big Endian:	Amdahl, 68k, Pyramid, Mips, Sparc, ...
#    Little Endian:	Vax, 32k, Spim (Dec Mips), i386, i486, ...
#
# If in doubt, leave BYTE_ORDER empty.  This makefile will attempt to
# use BYTE_ORDER in <machine/endian.h> or it will attempt to run
# the endian program.  If you get syntax errors when you compile,
# try forcing the value to be BIG_ENDIAN and run the calc regression
# tests. (see the README file)  If the calc regression tests fail, do
# a make clobber and try LITTLE_ENDIAN.  If that fails, ask a wizard
# for help.
#
BYTE_ORDER=
#BYTE_ORDER= BIG_ENDIAN
#BYTE_ORDER= LITTLE_ENDIAN

# Determine whether to use the standard malloc or the alternative one
# included with the calculator.  On some machines, the alternative malloc
# may be faster.  It also can help to debug malloc problems.
#
# Define MALLOC to be -DCALC_MALLOC to use the internal malloc routines.
#
# If in doubt, leave MALLOC empty.
#
MALLOC=
#MALLOC= -DCALC_MALLOC

# where to install binary files
#
#BINDIR= /usr/bin
BINDIR= /usr/local/bin

# where to install the lib/*.cal files
#
#LIBDIR= /usr/lib/calc
LIBDIR= /usr/local/lib/calc

# where to install help files
#
# The ${LIBDIR}/help is where the help files will be installed.
#
HELPDIR= ${LIBDIR}/help

# where man pages are installed
#
#MANDIR=/usr/man/man1
MANDIR=/usr/local/man/man1
#MANDIR=/usr/share/man/man1
#MANDIR=/usr/man/u_man/man1

# If the $CALCPATH environment variable is not defined, then the following
# path will be search for calc lib routines.
#
CALCPATH= .:./lib:~/lib:${LIBDIR}

# If the $CALCRC environment variable is not defined, then the following
# path will be search for calc lib routines.
#
CALCRC= ${LIBDIR}/startup:~/.calcrc

# If the $CALCBINDINGS environment variable is not defined, then the following
# file will be used for the command line and edit history key bindings.
#
CALCBINDINGS= ${LIBDIR}/bindings

# If $PAGER is not set, use this program to display a help file
#
#CALCPAGER= more
CALCPAGER= less
#CALCPAGER= pg
#CALCPAGER= cat

# Compile debug options
#
# Select -O, or empty string, if you don't want to debug
DEBUG= -O
#DEBUG= -O3
#DEBUG= -g
#DEBUG= -gx
#DEBUG= -WM,-g
#DEBUG=

# On systems that have dynamic shared libs, you want want to disable them
# for faster calc startup.
#
NO_SHARED=
#NO_SHARED= -dn

# Some systems allow 'mkdir -p' to make a directory and any needed parent
# directories.
#
# If you system has 'mkdir -p', then:	MKDIR=mkdir -p
# otherwise:				MKDIR=mkdir
#
# If you do not have 'mkdir -p', then you must ensure that ${LIBDIR},
# ${BINDIR} and ${HELPDIR} exist before you do an install.
#
#MKDIR=mkdir -p
MKDIR=mkdir

# If you are running an an old BSD system, then you may not have
# the following functions:
#
#	memcpy()	strchr()	memset()
#
# If you do not have these functions, define OLD_BSD to be -DOLD_BSD,
# otherwise define OLD_BSD to be an empty string.
#
# Modern BSD and BSD-like systems have these functions and thus don't
# need OLD_BSD.  If you don't know, try using the empty string and if
# you get complaints, try -DOLD_BSD.
#
OLD_BSD=
#OLD_BSD= -DOLD_BSD

# Some old systems don't know what a uid_t is.  Define UID_T if you get
# an error regarding 'uid_t' when compiling files such as calc.c
#
UID_T=
#UID_T= -DUID_T

# Some systems require one to use ranlib to add a symbol table to
# a *.a library.  Set RANLIB to the utility that performs this action.
# Set RANLIB to : if your system does not need such a utility.
#
RANLIB=ranlib
#RANLIB=:

# Some systems are able to form lint libs.  How it is formed depends
# on your system.  If you do not care about lint, use : as the
# LINTLIB value.
#
#    System type    LINTLIB recomendation
#
#	BSD	    ${LINT} ${LCFLAGS} ${LINTFLAGS} -u -Ccalc
#	SYSV	    ${LINT} ${LCFLAGS} ${LINTFLAGS} -u -o calc
#	disable     :
#
LINTLIB= ${LINT} ${LCFLAGS} ${LINTFLAGS} -u -Ccalc
#LINTLIB= ${LINT} ${LCFLAGS} ${LINTFLAGS} -u -o calc
#LINTLIB= :

# The lint flags vary from system to system.  Some systems have the
# opposite meaning for the flags below.  Other systems change flag
# meaning altogether.
#
#       System    LINTFLAGS recomendation
#
#	SunOs	  -a -h -v -z
#
LINTFLAGS= -a -h -v -z
#LINTFLAGS=

##############################################################################
#-=-=-=-=-=-=-=-=- Be careful if you change something below -=-=-=-=-=-=-=-=-#
##############################################################################

# standard utilities used during make
#
SHELL= /bin/sh
MAKE= make
SED= sed
TEE= tee
LINT= lint
CTAGS= ctags
#CC= cc
CC= gcc


#
# the source files which are built into a math library
#
LIBSRC = alloc.c comfunc.c commath.c qfunc.c qio.c qmath.c qmod.c qtrans.c \
	zfunc.c zio.c zmath.c zmod.c zmul.c


#
# the object files which are built into a math library
#
LIBOBJS = alloc.o comfunc.o commath.o qfunc.o qio.o qmath.o qmod.o qtrans.o \
	zfunc.o zio.o zmath.o zmod.o zmul.o

#
# the calculator source files
#
CALCSRC = addop.c assocfunc.c calc.c codegen.c config.c const.c	file.c \
	func.c hist.c input.c label.c listfunc.c matfunc.c obj.c opcodes.c \
	string.c symbol.c token.c value.c version.c


#
# we build these .o files for calc
#
CALCOBJS = addop.o assocfunc.o calc.o codegen.o config.o const.o file.o \
	func.o hist.o input.o label.o listfunc.o matfunc.o obj.o opcodes.o \
	string.o symbol.o token.o value.o version.o


# we build these .h files during the make
#
BUILD_H_SRC= config.h have_malloc.h have_stdlib.h have_string.h args.h \
	args.h terminal.h endian.h


# these .h files are needed by programs that use libcalc.a
#
LIB_H_SRC= qmath.h zmath.h cmath.h alloc.h have_stdlib.h have_malloc.h \
	have_string.h endian.h


# The code program is not part of the calc distribution, don't worry
# if you do not have it.
#
CODEOBJS= code.o


# LCFLAGS are flags that both ${CC} and ${LINT} can use
# CFLAGS are the default flags given to ${CC}
#
LCFLAGS= ${MALLOC} ${OLD_BSD} ${DONT_HAVE_VSPRINTF} ${UID_T}
CFLAGS= ${DEBUG} ${LCFLAGS} -ansi

all: calc calc.1

calc: libcalc.a ${CALCOBJS}
	${CC} ${CFLAGS} ${CALCOBJS} libcalc.a -o calc ${NO_SHARED}

hist.o: hist.c Makefile
	${CC} ${CFLAGS} ${DEBUG} ${TERMCONTROL} -c hist.c

libcalc.a: ${LIBOBJS} Makefile
	rm -f libcalc.a
	ar qc libcalc.a ${LIBOBJS}
	${RANLIB} libcalc.a

endian: endian.c
	-@rm -f endian.o endian
	${CC} ${CFLAGS} endian.c -o endian ${NO_SHARED}

config.h: Makefile
	rm -f config.h
	@echo 'forming config.h'
	@echo '/*' > config.h
	@echo ' * DO NOT EDIT -- generated by the Makefile' >> config.h
	@echo ' */' >> config.h
	@echo '' >> config.h
	@echo '/* the default :-separated search path */' >> config.h
	@echo '#ifndef DEFAULTCALCPATH' >> config.h
	@echo '#define DEFAULTCALCPATH "${CALCPATH}"' >> config.h
	@echo '#endif /* DEFAULTCALCPATH */' >> config.h
	@echo '' >> config.h
	@echo '/* the default :-separated startup file list */' >> config.h
	@echo '#ifndef DEFAULTCALCRC' >> config.h
	@echo '#define DEFAULTCALCRC "${CALCRC}"' >> config.h
	@echo '#endif /* DEFAULTCALCRC */' >> config.h
	@echo '' >> config.h
	@echo '/* the default key bindings file */' >> config.h
	@echo '#ifndef DEFAULTCALCBINDINGS' >> config.h
	@echo '#define DEFAULTCALCBINDINGS "${CALCBINDINGS}"' >> config.h
	@echo '#endif /* DEFAULTCALCBINDINGS */' >> config.h
	@echo '' >> config.h
	@echo '/* the location of the help directory */' >> config.h
	@echo '#ifndef HELPDIR' >> config.h
	@echo '#define HELPDIR "${HELPDIR}"' >> config.h
	@echo '#endif /* HELPDIR */' >> config.h
	@echo '' >> config.h
	@echo '/* the default pager to use */' >> config.h
	@echo '#ifndef DEFAULTCALCPAGER' >> config.h
	@echo '#define DEFAULTCALCPAGER "${CALCPAGER}"' >> config.h
	@echo '#endif /* DEFAULTCALCPAGER */' >> config.h
	@echo 'config.h formed'

endian.h: endian
	rm -f endian.h
	@echo 'forming endian.h'
	@echo '/*' > endian.h
	@echo ' * DO NOT EDIT -- generated by the Makefile' >> endian.h
	@echo ' */' >> endian.h
	@echo '' >> endian.h
	-@if [ X"${BYTE_ORDER}" = X ]; then \
	    if [ -f /usr/include/machine/endian.h ]; then \
		echo '#include <machine/endian.h>' >> endian.h; \
	    else \
		./endian >> endian.h; \
	    fi; \
	else \
	    echo "#define BYTE_ORDER ${BYTE_ORDER}" >> endian.h; \
	fi
	@echo 'endian.h formed'

have_malloc.h: Makefile
	rm -f have_malloc.h
	@echo 'forming have_malloc.h'
	@echo '/*' > have_malloc.h
	@echo ' * DO NOT EDIT -- generated by the Makefile' >> have_malloc.h
	@echo ' */' >> have_malloc.h
	@echo '' >> have_malloc.h
	@echo '/* do we have /usr/include/malloc.h? */' > have_malloc.h
	-@if [ -f /usr/include/malloc.h ]; then \
		echo '#define HAVE_MALLOC_H  /* yes */' >> have_malloc.h; \
	else \
		echo '#undef HAVE_MALLOC_H   /* no */' >> have_malloc.h; \
	fi
	@echo 'have_malloc.h formed'

have_stdlib.h: Makefile
	rm -f have_stdlib.h
	@echo 'forming have_stdlib.h'
	@echo '/*' > have_stdlib.h
	@echo ' * DO NOT EDIT -- generated by the Makefile' >> have_stdlib.h
	@echo ' */' >> have_stdlib.h
	@echo '' >> have_stdlib.h
	@echo '/* do we have /usr/include/stdlib.h? */' > have_stdlib.h
	-@if [ -f /usr/include/stdlib.h ]; then \
		echo '#define HAVE_STDLIB_H  /* yes */' >> have_stdlib.h; \
	else \
		echo '#undef HAVE_STDLIB_H   /* no */' >> have_stdlib.h; \
	fi
	@echo 'have_stdlib.h formed'

have_string.h: Makefile
	rm -f have_string.h
	@echo 'forming have_string.h'
	@echo '/*' > have_string.h
	@echo ' * DO NOT EDIT -- generated by the Makefile' >> have_string.h
	@echo ' */' >> have_string.h
	@echo '' >> have_string.h
	@echo '/* do we have /usr/include/string.h? */' > have_string.h
	-@if [ -f /usr/include/string.h ]; then \
		echo '#define HAVE_STRING_H  /* yes */' >> have_string.h; \
	else \
		echo '#undef HAVE_STRING_H   /* no */' >> have_string.h; \
	fi
	@echo 'have_string.h formed'

terminal.h: Makefile
	rm -f terminal.h
	@echo 'forming terminal.h'
	@echo '/*' > terminal.h
	@echo ' * DO NOT EDIT -- generated by the Makefile' >> terminal.h
	@echo ' */' >> terminal.h
	@echo '' >> terminal.h
	@echo '#if !defined(USE_TERMIOS)' >> terminal.h
	@echo '#if !defined(USE_TERMIO)' >> terminal.h
	@echo '#if !defined(USE_SGTTY)' >> terminal.h
	-@if [ -f /usr/include/termios.h ]; then \
		echo '#define USE_TERMIOS  /* <termios.h> */' >> terminal.h; \
		echo '#undef USE_TERMIO    /* <termio.h> */' >> terminal.h; \
		echo '#undef USE_SGTTY     /* <sys/ioctl.h> */' >> terminal.h; \
	elif [ -f /usr/include/termio.h ]; then \
		echo '#undef USE_TERMIOS   /* <termios.h> */' >> terminal.h; \
		echo '#define USE_TERMIO   /* <termio.h> */' >> terminal.h; \
		echo '#undef USE_SGTTY     /* <sys/ioctl.h> */' >> terminal.h; \
	else \
		echo '#undef USE_TERMIOS   /* <termios.h> */' >> terminal.h; \
		echo '#undef USE_TERMIO    /* <termio.h> */' >> terminal.h; \
		echo '#define USE_SGTTY    /* <sys/ioctl.h> */' >> terminal.h; \
	fi
	@echo '#endif' >> terminal.h
	@echo '#endif' >> terminal.h
	@echo '#endif' >> terminal.h
	@echo 'terminal.h formed'

args.h: Makefile
	rm -f args.h
	@echo 'forming args.h'
	@echo '/*' > args.h
	@echo ' * DO NOT EDIT -- generated by the Makefile' >> args.h
	@echo ' */' >> args.h
	@echo '' >> args.h
	@echo '/* what sort of variable args do we have? */' > args.h
	-@if [ ! -z "${VARARG}" ]; then \
		echo '#define ${VARARG}' >> args.h; \
	elif [ -f /usr/include/stdarg.h ]; then \
		echo '#define STDARG' >> args.h; \
	elif [ -f /usr/include/varargs.h ]; then \
		echo '#define VARARGS' >> args.h; \
	else \
		echo '#define SIMULATE_STDARG' >> args.h; \
	fi
	@echo 'args.h formed'

calc.1: calc.man
	rm -f calc.1
	${SED} -e 's:$${LIBDIR}:${LIBDIR}:g' < calc.man > calc.1

llib-lcalc.ln: ${BUILD_H_SRC} ${LIBSRC} Makefile
	rm -f llib-lcalc.ln llib.out
	-touch llib-lcalc.ln
	${LINTLIB} ${LIBSRC} 2>&1 | ${SED} -f lint.sed | ${TEE} llib.out

lint: ${BUILD_H_SRC} ${CALCSRC} llib-lcalc.ln lint.sed Makefile
	rm -f lint.out
	${LINT} ${LINTFLAGS} ${LCFLAGS} llib-lcalc.ln ${CALCSRC} 2>&1 | \
	    ${SED} -f lint.sed | ${TEE} lint.out

tags: ${CALCSRC}
	${CTAGS} ${CALCSRC}

lintclean:
	rm -f llib-lcalc.ln llib.out lint.out

clean:
	rm -f ${LIBOBJS} ${CALCOBJS} ${CODEOBJS} endian.o
	cd help; ${MAKE} -f Makefile LIBDIR=${LIBDIR} HELPDIR=${HELPDIR} clean

clobber: lintclean
	rm -f ${LIBOBJS} ${CALCOBJS} ${CODEOBJS}
	rm -f tags calc code libcalc.a
	rm -f ${BUILD_H_SRC} calc.1 endian
	cd help; ${MAKE} -f Makefile LIBDIR=${LIBDIR} HELPDIR=${HELPDIR} clobber

install: calc libcalc.a ${LIB_H_SRC} calc.1
	-@if [ ! -d ${LIBDIR} ]; then \
		echo ${MKDIR} ${LIBDIR}; \
		${MKDIR} ${LIBDIR}; \
	fi
	-chmod 0755 ${LIBDIR}
	-@if [ ! -d ${HELPDIR} ]; then \
		echo ${MKDIR} ${HELPDIR}; \
		${MKDIR} ${HELPDIR}; \
	fi
	-chmod 0755 ${HELPDIR}
	-@if [ ! -d ${BINDIR} ]; then \
		echo ${MKDIR} ${BINDIR}; \
		${MKDIR} ${BINDIR}; \
	fi
	-chmod 0755 ${BINDIR}
	rm -f ${BINDIR}/calc
	cp calc ${BINDIR}
	-chmod 0555 ${BINDIR}/calc
	cd help; ${MAKE} -f Makefile LIBDIR=${LIBDIR} HELPDIR=${HELPDIR} install
	cd lib; ${MAKE} -f Makefile LIBDIR=${LIBDIR} install
	rm -f ${LIBDIR}/libcalc.a
	cp libcalc.a ${LIBDIR}/libcalc.a
	-chmod 0644 ${LIBDIR}/libcalc.a
	${RANLIB} ${LIBDIR}/libcalc.a
	@for i in ${LIB_H_SRC}; do \
		echo rm -f ${LIBDIR}/$$i; \
		rm -f ${LIBDIR}/$$i; \
		echo cp $$i ${LIBDIR}; \
		cp $$i ${LIBDIR}; \
		echo chmod 0444 ${LIBDIR}/$$i; \
		chmod 0444 ${LIBDIR}/$$i; \
	done
	@# If lint was made, install the lint library.
	-@if [ -f llib-lcalc.ln ]; then \
		echo rm -f ${LIBDIR}/llib-lcalc.ln; \
		rm -f ${LIBDIR}/llib-lcalc.ln; \
		echo cp llib-lcalc.ln ${LIBDIR}; \
		cp llib-lcalc.ln ${LIBDIR}; \
		echo chmod 0444 ${LIBDIR}/llib-lcalc.ln; \
		chmod 0444 ${LIBDIR}/llib-lcalc.ln; \
	fi
	rm -f ${MANDIR}/calc.1
	-cp calc.1 ${MANDIR}
	-chmod 0444 ${MANDIR}/calc.1
	@# The code program is not part of the calc distribution, don't worry
	@# if you do not have it.
	-@if [ -f code ]; then \
		echo chmod +x code; \
		chmod +x code; \
		echo rm -f ${BINDIR}/code; \
		rm -f ${BINDIR}/code; \
		echo cp code ${BINDIR}; \
		cp code ${BINDIR}; \
		echo chmod 0555 ${BINDIR}/code; \
		chmod 0555 ${BINDIR}/code; \
	fi

# The code program is not part of the calc distribution, don't worry
# if you do not have it.
#
code: ${CODEOBJS} ${LIBOBJS}
	${CC} ${CFLAGS} ${CODEOBJS} libcalc.a -o code ${NO_SHARED}

code.o:		code.c stdarg.h args.h qmath.h zmath.h alloc.h \
		have_malloc.h have_string.h have_stdlib.h endian.h
	${CC} ${CFLAGS} code.c -c


# make depend stuff
#
addop.o:	addop.c calc.h value.h cmath.h qmath.h zmath.h alloc.h \
		have_malloc.h have_string.h have_stdlib.h opcodes.h string.h \
		func.h label.h token.h symbol.h endian.h
alloc.o:	alloc.c alloc.h have_malloc.h have_string.h have_stdlib.h
assocfunc.o:	assocfunc.c value.h cmath.h qmath.h zmath.h alloc.h \
		have_malloc.h have_string.h have_stdlib.h endian.h
calc.o:		calc.c calc.h value.h cmath.h qmath.h zmath.h alloc.h \
		have_malloc.h have_string.h have_stdlib.h hist.h func.h \
		label.h opcodes.h config.h token.h symbol.h endian.h
codegen.o:	codegen.c calc.h value.h cmath.h qmath.h zmath.h alloc.h \
		have_malloc.h have_string.h have_stdlib.h token.h symbol.h \
		label.h opcodes.h string.h func.h config.h endian.h
comfunc.o:	comfunc.c cmath.h qmath.h zmath.h alloc.h have_malloc.h \
		have_string.h have_stdlib.h endian.h
commath.o:	commath.c cmath.h qmath.h zmath.h alloc.h have_malloc.h \
		have_string.h have_stdlib.h endian.h
config.o:	config.c calc.h value.h cmath.h qmath.h zmath.h alloc.h \
		have_malloc.h have_string.h have_stdlib.h endian.h
const.o:	const.c calc.h value.h cmath.h qmath.h zmath.h alloc.h \
		have_malloc.h have_string.h have_stdlib.h endian.h
file.o:		file.c stdarg.h args.h calc.h value.h cmath.h qmath.h \
		zmath.h alloc.h have_malloc.h have_string.h have_stdlib.h \
		endian.h
func.o:		func.c calc.h value.h cmath.h qmath.h zmath.h alloc.h \
		have_malloc.h have_string.h have_stdlib.h opcodes.h token.h \
		func.h label.h string.h symbol.h endian.h
hist.o:		hist.c hist.h terminal.h
input.o:	input.c calc.h value.h cmath.h qmath.h zmath.h alloc.h \
		have_malloc.h have_string.h have_stdlib.h config.h hist.h \
		endian.h
label.o:	label.c calc.h value.h cmath.h qmath.h zmath.h alloc.h \
		have_malloc.h have_string.h have_stdlib.h token.h label.h \
		string.h opcodes.h func.h endian.h
listfunc.o:	listfunc.c value.h cmath.h qmath.h zmath.h alloc.h \
		have_malloc.h have_string.h have_stdlib.h endian.h
matfunc.o:	matfunc.c value.h cmath.h qmath.h zmath.h alloc.h \
		have_malloc.h have_string.h have_stdlib.h endian.h
obj.o:		obj.c calc.h value.h cmath.h qmath.h zmath.h alloc.h \
		have_malloc.h have_string.h have_stdlib.h opcodes.h func.h \
		label.h symbol.h string.h endian.h
opcodes.o:	opcodes.c stdarg.h args.h calc.h value.h cmath.h qmath.h \
		zmath.h alloc.h have_malloc.h have_string.h have_stdlib.h \
		opcodes.h func.h label.h symbol.h hist.h endian.h
qfunc.o:	qfunc.c qmath.h zmath.h alloc.h have_malloc.h have_string.h \
		have_stdlib.h endian.h
qio.o:		qio.c stdarg.h args.h qmath.h zmath.h alloc.h have_malloc.h \
		have_string.h have_stdlib.h endian.h
qmath.o:	qmath.c qmath.h zmath.h alloc.h have_malloc.h have_string.h \
		have_stdlib.h endian.h
qmod.o:		qmod.c qmath.h zmath.h alloc.h have_malloc.h have_string.h \
		have_stdlib.h endian.h
qtrans.o:	qtrans.c qmath.h zmath.h alloc.h have_malloc.h have_string.h \
		have_stdlib.h endian.h
string.o:	string.c calc.h value.h cmath.h qmath.h zmath.h alloc.h \
		have_malloc.h have_string.h have_stdlib.h string.h endian.h
symbol.o:	symbol.c calc.h value.h cmath.h qmath.h zmath.h alloc.h \
		have_malloc.h have_string.h have_stdlib.h token.h symbol.h \
		string.h opcodes.h func.h label.h endian.h
token.o:	token.c stdarg.h args.h calc.h value.h cmath.h qmath.h \
		zmath.h alloc.h have_malloc.h have_string.h have_stdlib.h \
		token.h string.h endian.h
value.o:	value.c value.h cmath.h qmath.h zmath.h alloc.h \
		have_malloc.h have_string.h have_stdlib.h opcodes.h func.h \
		calc.h label.h symbol.h string.h endian.h
version.o:	version.c calc.h value.h cmath.h qmath.h zmath.h alloc.h \
		have_malloc.h have_string.h have_stdlib.h endian.h
zfunc.o:	zfunc.c zmath.h alloc.h have_malloc.h have_string.h \
		have_stdlib.h endian.h
zio.o:		zio.c stdarg.h args.h zmath.h alloc.h have_malloc.h \
		have_string.h have_stdlib.h endian.h
zmath.o:	zmath.c zmath.h alloc.h have_malloc.h have_string.h \
		have_stdlib.h endian.h
zmod.o:		zmod.c zmath.h alloc.h have_malloc.h have_string.h \
		have_stdlib.h endian.h
zmul.o:		zmul.c zmath.h alloc.h have_malloc.h have_string.h \
		have_stdlib.h endian.h
