#
#  This is the Makefile for xdizzy, a digital simulator for Motif/X.
#
#  Copyright 1990 Juri Munkki, all rights reserved
#
#  Please read the included file called "DizzyDoc" for information on
#  what your rights are concerning this product.
#
#  Some Macintosh-only files are included in the make process even
#  though they do not produce any object code. You might want to
#  remove macdizzy.c and util.c, if compiles are taking too long.
#

CC       = gcc
CFLAGS   = -O	# -DSYSV for HP machines.
LIBS     = -lXm -lXr3t -lX11	# -lXm -lXt -lX11 -lPW for most of you out there.
LIBSRC   = -L/p/motif/lib	# This is where our motif library is kept.
INCLUDES = -I/p/motif/include	# This is where our motif includes are kept.

OBJS = arrow.o dizzyx.o edit.o filesel.o macdizzy.o \
       macutil.o menu.o pntonline.o quickraw.o simdisplay.o \
       simfile.o simmem.o simsim.o tool.o zapper.o

SRCS = arrow.c dizzyx.c edit.c filesel.c macdizzy.c \
       macutil.c menu.c pntonline.c quickraw.c simdisplay.c \
       simfile.c simmem.c simsim.c tool.c zapper.c

INCS = dizzy.h protos.h macstuff.h xstuff.h elementdefs.h graphicsgems.h

BITS = tool.xbm menu.xbm misc.xbm splash.xbm

#
#  Dependencies begin here:
#
xdizzy:	$(OBJS)
	-/bin/rm -f xdizzy
	$(CC) $(OBJS) $(LIBSRC) $(LIBS)	-o $@

$(OBJS): $(INCS) $(BITS)

.c.o:
	$(CC) $(CFLAGS) -c $(INCLUDES) -o $@ $*.c 

clean:
	-/bin/rm -f xdizzy
	-/bin/rm -f *.o
	-/bin/rm -f core
