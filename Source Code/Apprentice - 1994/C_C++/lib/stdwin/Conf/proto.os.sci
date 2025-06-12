
#
# Definitions pertaining to operating system sco
#

# Define the name of this O.S. here.
#
OS=		sco

# Define the name of the script used to update the dependencies in
# the Makefile.
# Choices are:
#	slowmkdep -- always works (as far as I know)
#	fastmkdep -- uses cc -M, which isn't always available
#	makedepend -- Todd Brunhoff's superfast tool (comes with X11)
#	              (This needs -Dunix because of configure.h)
# You may also place this definition in the proto.arch.* file if no
# method works for every architecture supported by this OS.
#
MKDEP=		$(CONF)/slowmkdep
#MKDEP=		$(CONF)/fastmkdep
#MKDEP=		makedepend -Dunix

# Define the system libraries to link with programs that use termcap and X11.
# (On SYSV it is often safer to link with -lcurses instead of -ltermcap.)
# (In some cases these are architecture-dependent)
#
LIBTERMCAP=	-lcurses -lsocket
LIBX11=		-lXR4sco_s -lsocket

# Similar for the math library
#
LIBMATH=	-lm

# Other OS-specific choices (-I flags, -D flags, compiler options)
#
OSINCLS=	
OSDEFS=		-DSYSV
OSOPTS=		
