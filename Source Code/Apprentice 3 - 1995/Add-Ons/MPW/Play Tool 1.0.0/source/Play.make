#   File:       Play.make
#   Target:     Play
#   Sources:    Play.c Play.r
#   Created:    Thursday, June 11, 1992 11:19:20 PM


OBJECTS = Play.c.o


Play ÄÄ Play.make Play.r
	Rez Play.r -append -o Play

Play ÄÄ Play.make {OBJECTS}
	Link -d -c 'MPS ' -t MPST ¶
		{OBJECTS} ¶
		#"{CLibraries}"Complex.o ¶
		"{CLibraries}"StdClib.o ¶
		"{Libraries}"Stubs.o ¶
		"{Libraries}"Runtime.o ¶
		"{Libraries}"Interface.o ¶
		"{Libraries}"ToolLibs.o ¶
		-o play
Play.c.o Ä play.make Play.c
	 C -r  Play.c

play ÄÄ play.make Play.r {OBJECTS}
	Duplicate -p Play "{MPW}Tools:Play"
