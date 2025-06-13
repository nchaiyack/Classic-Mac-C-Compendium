#   File:       LaunchWithDoc2.make
#   Target:     LaunchWithDoc2
#   Sources:    LaunchWithDoc2.c
#   Created:    Wednesday, July 13, 1994 08:28:46 AM


OBJECTS = LaunchWithDoc2.c.o



LaunchWithDoc2 ÄÄ LaunchWithDoc2.make  {OBJECTS}
	Rez -append LaunchWithDoc2.rsrc -o LaunchWithDoc2
	Link -t APPL -c '????' -sym on -mf ¶
		{OBJECTS} ¶
 		"{Libraries}"Runtime.o ¶
 		"{Libraries}"Interface.o ¶
		-o LaunchWithDoc2
LaunchWithDoc2.c.o Ä LaunchWithDoc2.make LaunchWithDoc2.c
	 C -r -sym on  LaunchWithDoc2.c
