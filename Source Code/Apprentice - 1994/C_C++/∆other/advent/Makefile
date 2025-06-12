# Makefile for advent

CFLAGS	= -m -D_POSIX_SOURCE -D_MINIX
O=o

OBJS   = advent.$O database.$O english.$O initial.$O itverb.$O score.$O\
	travel.$O turn.$O utility.$O verb.$O

advent:	$(OBJS)
	cc -i -o advent $(OBJS) > /dev/null

setup: setup.c
	cc -o setup setup.c
 
advtext.h: setup advent1.txt advent2.txt advent3.txt advent4.txt
	setup
	mv advent?.dat /usr/src/data

advent.$O: advent.h advword.h advtext.h advdef.h
database.$O: advent.h advdec.h advcave.h
initial.$O english.$O itverb.$O score.$O travel.$O utility.$O\
	turn.$O verb.$O: advent.h advdec.h

clean:	
	@rm -rf *.o *.s *.bak core advent
