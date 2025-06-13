#
#	Apple Macintosh Developer Technical Support
#
#	MultiFinder-Aware Simple Sample Application
#
#	Sample
#
#	Sample.make	-	Make Source
#
#	Copyright © 1989, 1991 Apple Computer, Inc.
#	All rights reserved.
#
#	Versions:	
#				1.00				08/88
#				1.01				11/88
#				1.02				04/89
#				1.03				06/89
#				1.04				04/91   Updated for MPW 3.2
#
#	Components:
#				Sample.c			Feb.  1, 1990
#				Sample.r			Feb.  1, 1990
#				Sample.h			Feb.  1, 1990
#				Sample.make			Feb.  1, 1990
#
#	Sample is an example application that demonstrates how to
#	initialize the commonly used toolbox managers, operate 
#	successfully under MultiFinder, handle desk accessories, 
#	and create, grow, and zoom windows.
#
#	It does not by any means demonstrate all the techniques 
#	you need for a large application. In particular, Sample 
#	does not cover exception handling, multiple windows/documents, 
#	sophisticated memory management, printing, or undo. All of 
#	these are vital parts of a normal full-sized application.
#
#	This application is an example of the form of a Macintosh 
#	application; it is NOT a template. It is NOT intended to be 
#	used as a foundation for the next world-class, best-selling, 
#	600K application. A stick figure drawing of the human body may 
#	be a good example of the form for a painting, but that does not 
#	mean it should be used as the basis for the next Mona Lisa.
#
#	We recommend that you review this program or TESample before 
#	beginning a new application.
#
# 	You can define {SymOptions} as "-sym on" or "-sym off" for use with SADE
# 	We also recommend requiring prototypes for all functions
COptions = -r {SymOptions} -i "Projects:uupc 3.0:sources:Files in common:" ¶
						-i "Projects:uupc 3.0:sources:Mac specific:" ¶
						-i "Projects:uupc 3.0:sources:Mac specific:Unix lib"
							
CObjs		= ctbio.c.o ¶
			dcp.c.o ¶
			dcpapkt.c.o ¶
			dcpfpkt.c.o ¶
			dcpgpkt.c.o ¶
			dcpsys.c.o ¶
			dcpxfer.c.o ¶
			macinit.c.o ¶
			scandir.c.o ¶
			tcpglue.c.o ¶
			uuhost.c.o ¶
		"{Libraries}"Runtime.o ¶
		"{Libraries}"Interface.o

uupc		ÄÄ {CObjs} uupc.make
		Link -o {Targ} {CObjs} {SymOptions}
		SetFile {Targ} -t APPL -c 'MOOS' -a B

Sample		ÄÄ Sample.r Sample.h uupc.make
		Rez -rd -o {Targ} Sample.r -append

Sample.c.o	ÄÄ uupc.make

