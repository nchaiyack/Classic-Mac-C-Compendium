#   File:       WordWrapXFCN.make
#   Target:     WordWrapXFCN
#   Sources:    WordWrapXFCN.c
#   Created:    February 2, 1993

fNAME	= WordWrapXFCN
OBJECTS = {fNAME}.c.o

{fNAME} �� {fNAME}.make {OBJECTS}
	Link -w -rt XFCN=1000 -m ENTRYPOINT -sg {fNAME} �
		{OBJECTS} �
		#"{CLibraries}"CSANELib.o �
		#"{CLibraries}"Math.o �
		#"{CLibraries}"Complex.o �
		"{CLibraries}"StdClib.o �
		"{Libraries}"Runtime.o �
		"{Libraries}"Interface.o �
		"{Libraries}"HyperXLib.o �
		-o "{fNAME} Test"
{fNAME}.c.o � {fNAME}.make {fNAME}.c
	 C -r -b  {fNAME}.c
