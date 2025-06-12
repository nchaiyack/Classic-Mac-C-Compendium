#   File:       xdiff.make
#   Target:     xdiff
#   Sources:    xdiff.c xdiffapp.c
#   Created:    Monday, July 10, 1989 8:57:14 PM

XDversion.h Ä xdiff.make xdiffapp.c 
	echo "char *VERSION = ¶"Version 1.0 `date -d -s`¶";" >XDversion.h
xdiff.c.o Ä xdiff.make xdiff.c
	 C xdiff.c
xdiffapp.c.o Ä xdiff.make xdiffapp.c XDversion.h
	 C xdiffapp.c

SOURCES = xdiff.c xdiffapp.c
OBJECTS = xdiff.c.o xdiffapp.c.o

xdiff ÄÄ xdiff.make {OBJECTS}
	Link -w -t APPL -c 'xdif' ¶
		{OBJECTS} ¶
		"{CLibraries}"CRuntime.o ¶
		"{Libraries}"Interface.o ¶
		"{CLibraries}"StdCLib.o ¶
		"{CLibraries}"CSANELib.o ¶
		"{CLibraries}"Math.o ¶
		"{CLibraries}"CInterface.o ¶
		-o xdiff
