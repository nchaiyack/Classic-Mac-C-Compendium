#   File:       MakeTable.make
#   Target:     MakeTable
#   Sources:    MakeTable.c
#   Created:    Monday, February 1, 1993 11:21:22 PM

OBJECTS = MakeTable.c.o

MakeTable ÄÄ MakeTable.make {OBJECTS}
	Link -w -rt XFCN=1012 -m ENTRYPOINT -sg MakeTable ¶
		{OBJECTS} ¶
		#"{CLibraries}"CSANELib.o ¶
		#"{CLibraries}"Math.o ¶
		#"{CLibraries}"Complex.o ¶
		"{CLibraries}"StdClib.o ¶
		"{Libraries}"Runtime.o ¶
		"{Libraries}"Interface.o ¶
		"{Libraries}"HyperXLib.o ¶
		-o "MakeTable Test"
MakeTable.c.o Ä MakeTable.make MakeTable.c
	 C -r -b  MakeTable.c
