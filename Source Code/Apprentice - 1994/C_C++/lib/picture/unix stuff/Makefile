##	Makefile for picture library.
##	Don Snow 2/92.  Modified R. Gonzalez 4/92.
##	Requires g++ and XWindow system.
##	Options: simpict/ringpict/anringpi/dycubpic options will
##	produce the named executable, provided that main.cc agrees.
##	The clean/kill options remove objects and/or executables.
##
##	-g = Debugging info
##	-O = Optimize 
##
OPTFLAGS= 	-O\
##
##
CCFLAGS= 	$(OPTFLAGS)\
		-I/usr/local/X11/include
##
CCC=	g++
##
##	Libraries
##	X11 	X11 graphics library
##
LIBS=	-L/usr/local/X11/lib -lX11 -lm
##
##
2D_OBJECTS = 	class.o screen.o coord.o frame.o error.o\
		pict.o project.o backdrop.o xscrn.o
##
3D_OBJECTS = 	camera.o segment.o trans.o line.o cube.o ring.o
##
AN_OBJECTS = 	animate.o atring.o anring.o
##
DY_OBJECTS =	dynamic.o dycube.o
##
EXEC1=		simpict
##
EXEC1_OBJECTS=	$(2D_OBJECTS) simpict.o main.o
##
EXEC2=		ringpict
##
EXEC2_OBJECTS= $(2D_OBJECTS) $(3D_OBJECTS) ringpict.o main.o
##
EXEC3=		anringpi
##
EXEC3_OBJECTS=	$(2D_OBJECTS) $(3D_OBJECTS) $(AN_OBJECTS)\
		anringpi.o main.o
##
EXEC4=		dycubpic
##
EXEC4_OBJECTS=	$(2D_OBJECTS) $(3D_OBJECTS) $(AN_OBJECTS)\
		$(DY_OBJECTS) dycubpic.o main.o
##
##
.KEEP_STATE:
##
##
## Commands to compile and link objects together
##
$(EXEC1):	$(EXEC1_OBJECTS) 
		g++ -o $(EXEC1) $(EXEC1_OBJECTS) $(LIBS)
##
$(EXEC2):	$(EXEC2_OBJECTS) 
		g++ -o $(EXEC2) $(EXEC2_OBJECTS) $(LIBS)
##
$(EXEC3):	$(EXEC3_OBJECTS) 
		g++ -o $(EXEC3) $(EXEC3_OBJECTS) $(LIBS)
##
$(EXEC4):	$(EXEC4_OBJECTS) 
		g++ -o $(EXEC4) $(EXEC4_OBJECTS) $(LIBS)
##
clean:		
		rm -f $(2D_OBJECTS) $(3D_OBJECTS) $(AN_OBJECTS)\
		$(DY_OBJECTS) $(EXEC1_OBJECTS) $(EXEC2_OBJECTS)\
		$(EXEC3_OBJECTS) $(EXEC4_OBJECTS)
##
kill:		
		rm -f $(2D_OBJECTS) $(3D_OBJECTS) $(AN_OBJECTS)\
		$(DY_OBJECTS) $(EXEC1_OBJECTS) $(EXEC2_OBJECTS)\
		$(EXEC3_OBJECTS) $(EXEC4_OBJECTS) $(EXEC1) $(EXEC2)\
		$(EXEC3) $(EXEC4)





