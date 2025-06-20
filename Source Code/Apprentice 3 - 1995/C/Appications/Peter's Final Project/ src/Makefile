HDRS =				\
	camera.h		\
	clip.h 			\
	collision.h		\
	computer.h		\
	engine.h		\
	face.h			\
	list.h			\
	matrix.vector.h		\
	maze.h			\
	object.h		\
	point.h			\
	render.h		\
	scan.h			\
	sector.h		\
	sys.stuff.h		\
	sys.types.h		\
	texture.h		\
	timer.h			\
	type.defs.h		\
	utils.h			\
	world.h

SRCS =				\
	camera.c		\
	clip.c			\
	collision.c		\
	computer.c		\
	engine.c		\
	face.c			\
	list.c			\
	main.c			\
	matrix.vector.c		\
	maze.c			\
	object.c		\
	point.c			\
	render.c		\
	scan.c			\
	sector.c		\
	sys.stuff.c		\
	texture.c		\
	timer.c			\
	utils.c			\
	world.c

OBJS =				\
	camera.o		\
	clip.o			\
	collision.o		\
	computer.o		\
	engine.o		\
	face.o			\
	list.o			\
	main.o			\
	matrix.vector.o		\
	maze.o			\
	object.o		\
	point.o			\
	render.o		\
	scan.o			\
	sector.o		\
	sys.stuff.o		\
	texture.o		\
	timer.o			\
	utils.o			\
	world.o

PROGRAM	= final

CC        =	gcc
OPTIMIZE =	-O3 -finline-functions
#PROFILE  =	-pg
#DEBUG	  =	-g
CFLAGS    =	$(PROFILE) $(DEBUG) $(OPTIMIZE)
CLIBS     =			\
	  	-lX11		\
	  	-lXext		\
		-lXpm		\
	  	-lm		\
		-ljpeg

INCLUDE		= -I/usr/sww/X11/include -I/usr/sww/include

LINCLUDE	= -L/usr/sww/X11/lib -L/usr/sww/lib

LINKER  	= $(CC)

#----------------------------------------------------------------------
# You probably will not need to modify anything below this line.
#----------------------------------------------------------------------

# Add all the flags to CFLAGS

CFLAGS	+= $(INCLUDE)

default:    $(PROGRAM)

program:    $(PROGRAM)

server: server.c clientserver.h
	gcc -o server -g server.c

all:	    $(PROGRAM)

.c:
	$(CC) $(CFLAGS) -o $@ $< $(CLIBS) $(INCLUDE)
	
$(PROGRAM): $(OBJS) 
	@echo -n "Loading $(PROGRAM) ... "
	$(LINKER) $(PROFILE) $(OBJS) $(CLIBS) $(LINCLUDE) -o $(PROGRAM)
	@echo "done."

depend:
ifeq ($(OSTYPE), hpux)
	@echo "Making dependencies for HP..."
	/usr/sww/X11/bin/makedepend $(INCLUDE) $(CFLAGS) $(SRCS) 
else
	@echo "Making dependencies for SGI..."
	/usr/bin/X11/makedepend $(INCLUDE) $(CFLAGS) $(SRCS) 
endif


clean:
	rm -f *.o *~ gmon.out profile

# DO NOT DELETE THIS LINE -- make depend depends on it

camera.o: /usr/sww/include/assert.h camera.h matrix.vector.h type.defs.h
camera.o: sys.types.h sys.stuff.h /usr/include/stdlib.h
camera.o: /usr/include/sys/stdsyms.h
clip.o: clip.h point.h list.h type.defs.h sys.types.h matrix.vector.h
clip.o: sys.stuff.h /usr/include/stdlib.h /usr/include/sys/stdsyms.h
collision.o: collision.h type.defs.h sys.types.h face.h list.h point.h
collision.o: matrix.vector.h texture.h object.h camera.h sector.h
engine.o: /usr/sww/include/assert.h /usr/include/stdio.h
engine.o: /usr/include/sys/stdsyms.h /usr/include/stdlib.h collision.h
engine.o: type.defs.h sys.types.h engine.h matrix.vector.h maze.h object.h
engine.o: camera.h list.h render.h scan.h point.h texture.h sector.h face.h
engine.o: sys.stuff.h timer.h world.h
face.o: /usr/sww/include/assert.h face.h list.h type.defs.h sys.types.h
face.o: point.h matrix.vector.h texture.h sys.stuff.h /usr/include/stdlib.h
face.o: /usr/include/sys/stdsyms.h
list.o: /usr/sww/include/assert.h list.h type.defs.h sys.types.h sys.stuff.h
list.o: /usr/include/stdlib.h /usr/include/sys/stdsyms.h
main.o: /usr/include/stdio.h /usr/include/sys/stdsyms.h /usr/include/stdlib.h
main.o: /usr/include/string.h engine.h utils.h sys.stuff.h sys.types.h
matrix.vector.o: /usr/include/math.h /usr/include/sys/stdsyms.h
matrix.vector.o: /usr/include/stdio.h /usr/include/stdlib.h matrix.vector.h
matrix.vector.o: type.defs.h sys.types.h
maze.o: /usr/sww/include/assert.h /usr/include/stdlib.h
maze.o: /usr/include/sys/stdsyms.h /usr/include/stdio.h list.h type.defs.h
maze.o: sys.types.h maze.h sys.stuff.h
object.o: /usr/sww/include/assert.h face.h list.h type.defs.h sys.types.h
object.o: point.h matrix.vector.h texture.h object.h camera.h sys.stuff.h
object.o: /usr/include/stdlib.h /usr/include/sys/stdsyms.h
point.o: /usr/sww/include/assert.h point.h list.h type.defs.h sys.types.h
point.o: matrix.vector.h sys.stuff.h /usr/include/stdlib.h
point.o: /usr/include/sys/stdsyms.h
render.o: /usr/include/math.h /usr/include/sys/stdsyms.h /usr/include/stdio.h
render.o: /usr/include/stdlib.h clip.h point.h list.h type.defs.h sys.types.h
render.o: matrix.vector.h face.h texture.h object.h camera.h render.h scan.h
render.o: sector.h sys.stuff.h
scan.o: /usr/sww/include/assert.h /usr/include/math.h
scan.o: /usr/include/sys/stdsyms.h /usr/include/stdio.h list.h type.defs.h
scan.o: sys.types.h sys.stuff.h /usr/include/stdlib.h scan.h matrix.vector.h
scan.o: point.h texture.h utils.h
sector.o: /usr/sww/include/assert.h sector.h face.h list.h type.defs.h
sector.o: sys.types.h point.h matrix.vector.h texture.h sys.stuff.h
sector.o: /usr/include/stdlib.h /usr/include/sys/stdsyms.h
sys.stuff.o: /usr/sww/X11/include/X11/Xlib.h /usr/include/sys/types.h
sys.stuff.o: /usr/include/sys/stdsyms.h /usr/sww/X11/include/X11/X.h
sys.stuff.o: /usr/sww/X11/include/X11/Xfuncproto.h
sys.stuff.o: /usr/sww/X11/include/X11/Xosdefs.h /usr/include/stddef.h
sys.stuff.o: /usr/sww/X11/include/X11/Xutil.h
sys.stuff.o: /usr/sww/X11/include/X11/keysym.h
sys.stuff.o: /usr/sww/X11/include/X11/keysymdef.h
sys.stuff.o: /usr/sww/X11/include/X11/extensions/XShm.h
sys.stuff.o: /usr/sww/X11/include/X11/xpm.h /usr/include/sys/ipc.h
sys.stuff.o: /usr/include/sys/shm.h /usr/sww/include/assert.h
sys.stuff.o: /usr/include/signal.h /usr/include/sys/signal.h sys.stuff.h
sys.stuff.o: /usr/include/stdlib.h sys.types.h utils.h
texture.o: /usr/sww/include/assert.h /usr/include/math.h
texture.o: /usr/include/sys/stdsyms.h /usr/include/stdio.h
texture.o: /usr/sww/include/jpeglib.h /usr/sww/include/jconfig.h
texture.o: /usr/sww/include/jmorecfg.h sys.stuff.h /usr/include/stdlib.h
texture.o: sys.types.h texture.h type.defs.h utils.h
timer.o: /usr/include/stdlib.h /usr/include/sys/stdsyms.h timer.h
utils.o: /usr/include/stdio.h /usr/include/sys/stdsyms.h
utils.o: /usr/include/stdlib.h utils.h
world.o: /usr/sww/include/assert.h /usr/include/stdio.h
world.o: /usr/include/sys/stdsyms.h face.h list.h type.defs.h sys.types.h
world.o: point.h matrix.vector.h texture.h maze.h object.h camera.h sector.h
world.o: sys.stuff.h /usr/include/stdlib.h world.h
