#   File:       MCaller.make
#   Target:     MCaller
#   Sources:    MCaller.c MCaller.r XPINGLUE.c


# Uncomment this line to turn debugging on...
DebugOn = -sym on

MCaller �� MCaller.c.o XPINGLUE.c.o
	Link -t APPL -c 'rp&A' {DebugOn} -mf �
		MCaller.c.o XPINGLUE.c.o �
		"{Libraries}"Runtime.o �
		"{Libraries}"Interface.o �
		"{CLibraries}"StdCLib.o �
		-o MCaller

XPINGLUE.c.o � XPINGLUE.c MCaller.h
	 C -r {DebugOn} -warnings off XPINGLUE.c

MCaller.c.o � MCaller.c MCaller.h
	 C -r {DebugOn} -warnings off MCaller.c

MCaller		�� MCaller.r MCaller.h
		Rez -rd -o MCaller MCaller.r -append

