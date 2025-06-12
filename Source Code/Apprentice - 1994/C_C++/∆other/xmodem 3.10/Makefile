SRCS = xmodem.c getput.c misc.c send.c receive.c batch.c
OBJECTS = xmodem.o getput.o misc.o send.o receive.o batch.o
CFLAGS = -O
BIN = /usr/local/bin
MAN = /usr/man/manl/xmodem.l

xmodem: $(OBJECTS)
	cc $(CFLAGS) $(OBJECTS) -o xmodem

$(OBJECTS): xmodem.h

clean:
	rm -f *.o errs a.out core xmodem

install: xmodem
	install -s xmodem ${BIN}
	install -c xmodem.1 ${MAN}

print: 
	lpr -p -Pvmslp version.h xmodem.h xmodem.c getput.c receive.c send.c \
	batch.c misc.c Makefile update.doc README xmodem.1 getput.sysv.c

lint:
	lint xmodem.c getput.c receive.c send.c batch.c misc.c 

shar:
	shar README Makefile xmodem.1 xmodem.h version.h > xmodem.shar.1
	shar update.doc xmodem.c receive.c > xmodem.shar.2
	shar getput.c getput.sysv.c > xmodem.shar.3
	shar misc.c batch.c send.c > xmodem.shar.4
	shar ymodem.doc > xmodem.shar.5

bigshar:
	shar README update.doc Makefile xmodem.1 xmodem.h version.h xmodem.c \
	getput.c getput.sysv.c receive.c misc.c batch.c send.c ymodem.doc \
	> xmodem.shar

dist: 
	tar cvf xmodem.tar README update.doc Makefile xmodem.1 xmodem.h \
	version.h xmodem.c getput.c getput.sysv.c receive.c misc.c batch.c \
	send.c ymodem.doc
	compress xmodem.tar

xmodem.o: xmodem.c xmodem.h version.h
getput.o: getput.c xmodem.h
misc.o: misc.c xmodem.h
send.o: send.c xmodem.h
receive.o: receive.c xmodem.h
batch.o: batch.c xmodem.h
