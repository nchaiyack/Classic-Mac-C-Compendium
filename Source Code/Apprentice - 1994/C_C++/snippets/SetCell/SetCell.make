#   File:       SetCell.make
#   Target:     SetCell
#   Sources:    SetCell.c
#   Created:    Thursday, March 31, 1994 12:02:11


OBJECTS = SetCell.c.o

SetCell ÄÄ SetCell.make SetCell.r
	Rez SetCell.r -append -o SetCell


SetCell ÄÄ SetCell.make {OBJECTS}
	Link -t APPL -c '????' -model far -sym on ¶
		{OBJECTS} ¶
		#"{CLibraries}"CSANELib.o ¶
		#"{CLibraries}"Math.o ¶
		#"{CLibraries}"Complex.o ¶
		"{CLibraries}"StdClib.o ¶
		"{Libraries}"Runtime.o ¶
		"{Libraries}"Interface.o ¶
		"{Libraries}"AEObjectSupportLib.o ¶
		-o SetCell
SetCell.c.o Ä SetCell.make SetCell.c
	 C -t -p -model far -sym on SetCell.c
