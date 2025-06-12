#   File:       ARTAbrot.make
#   Target:     ARTAbrot
#   Sources:    Main_ARTAbrot.c
#               AB_Alert.c
#               ComUtil_ARTAbrot.c
#               About_ARTAbrot.c
#               Enter_Coordinates.c
#               Menu_ARTAbrot.c
#               ARTAbrot.c
#   Created:    Wednesday, August. 18, 1993 2:09:19 PM


OPTS = -r -sym on -mc68020

OBJECTS = ¶
		Main_ARTAbrot.c.o ¶
		AB_Alert.c.o ¶
		ComUtil_ARTAbrot.c.o ¶
		About_ARTAbrot.c.o ¶
		Enter_Coordinates.c.o ¶
		Menu_ARTAbrot.c.o ¶
		ARTAbrot.c.o ¶
		BrotCode.c.o


ARTAbrot ÄÄ ARTAbrot.make {OBJECTS}
	Link -t APPL -c ATT3 -sym on -mf ¶
		{OBJECTS} ¶
		#"{CLibraries}"Complex.o ¶
		"{CLibraries}"StdCLib.o ¶
		"{Libraries}"Runtime.o ¶
		"{Libraries}"Interface.o ¶
		-o ARTAbrot
Main_ARTAbrot.c.o Ä ARTAbrot.make Main_ARTAbrot.c About_ARTAbrot.h AB_Alert.h ARTAbrot.h BrotCode.h ComUtil_ARTAbrot.h Enter_Coordinates.h Menu_ARTAbrot.h
	 C {OPTS} Main_ARTAbrot.c
AB_Alert.c.o Ä ARTAbrot.make AB_Alert.c AB_Alert.h ARTAbrot.h ComUtil_ARTAbrot.h
	 C {OPTS} AB_Alert.c
ComUtil_ARTAbrot.c.o Ä ARTAbrot.make ComUtil_ARTAbrot.c ARTAbrot.h ComUtil_ARTAbrot.h
	 C {OPTS} ComUtil_ARTAbrot.c
About_ARTAbrot.c.o Ä ARTAbrot.make About_ARTAbrot.c About_ARTAbrot.h ARTAbrot.h ComUtil_ARTAbrot.h
	 C {OPTS} About_ARTAbrot.c
Enter_Coordinates.c.o Ä ARTAbrot.make Enter_Coordinates.c AB_Alert.h ARTAbrot.h ComUtil_ARTAbrot.h Enter_Coordinates.h
	 C {OPTS} Enter_Coordinates.c
Menu_ARTAbrot.c.o Ä ARTAbrot.make Menu_ARTAbrot.c About_ARTAbrot.h AB_Alert.h ARTAbrot.h BrotCode.h ComUtil_ARTAbrot.h Enter_Coordinates.h Menu_ARTAbrot.h
	 C {OPTS} Menu_ARTAbrot.c
ARTAbrot.c.o Ä ARTAbrot.make ARTAbrot.c AB_Alert.h ARTAbrot.h ComUtil_ARTAbrot.h BrotCode.h
	 C {OPTS} ARTAbrot.c
BrotCode.c.o Ä ARTAbrot.make BrotCode.c BrotCode.h
	 C {OPTS} BrotCode.c
mbrot Ä ARTAbrot.make mbrot.s
	 d32asm mbrot.s mbrot

ARTAbrot ÄÄ ARTAbrot.make ARTAbrot.RSRC ARTAbrot.r mbrot
# This is the main resource file 
	Delete -i ARTAbrot.R
	DeRez ARTAbrot.RSRC > ARTAbrot.R
	Rez -append ARTAbrot.R -o ARTAbrot  
	Delete -i mbrot.R
	DeRez mbrot > mbrot.R
	Rez -append mbrot.R -o ARTAbrot  
