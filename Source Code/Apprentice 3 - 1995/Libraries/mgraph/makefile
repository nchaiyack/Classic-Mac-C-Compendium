##	Makefile for graphics routines.
##	R. Gonzalez 4/93. (Based on example by Don Snow.)
##	Requires g++ (though only ANSI C is used) and XWindow system.
##
##	Substitute your filename(s) for pendulum.o, below. Be sure to
##	name source files: *.cc
##
##	Options: 'make graph' to compile, 'make clean' to
##	remove object files.
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
GRAPH_OBJECTS = 	xgraph.o trans.o
##
EXEC1=		graph
##
EXEC1_OBJECTS=	$(GRAPH_OBJECTS) pendulum.o
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
clean:
		rm -f $(GRAPH_OBJECTS) $(EXEC1_OBJECTS)
