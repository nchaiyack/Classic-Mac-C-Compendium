#   File:       resolve.make
#   Target:     resolve
#   Sources:    resolve.c
#   Created:    Thursday, April 4, 1991 3:14:30 PM

MacTCPCIncludes = "jlnx:MacTCP:CIncludes:"

OBJECTS = resolve.c.o rslv.c.o utl.c.o fsu.c.o

COptions = -mbg ch8 -sym off -r -i {MacTCPCIncludes}

resolve.c.o � precompile rslv.h fsu.h
rslv.c.o � precompile utl.h rslv.h fsu.h
utl.c.o � precompile utl.h
fsu.c.o � precompile fsu.h utl.h

precompile � precompile.c
	C {COptions} precompile.c

resolve �� resolve.make {OBJECTS}
	Link -d -c 'MPS ' -t MPST �
		{OBJECTS} �
		"{CLibraries}"StdClib.o �
		"{Libraries}"Stubs.o �
		"{Libraries}"Runtime.o �
		"{Libraries}"Interface.o �
		-o resolve
