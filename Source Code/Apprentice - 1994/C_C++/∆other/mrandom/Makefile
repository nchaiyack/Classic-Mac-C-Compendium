CFLAGS= -O

all: mrtest

clean:
	-rm *.o
	-rm mrtest

install:; @-echo "just link mrandom.o into your own programs as needed."

mrtest: mrandom.h mrandom.o mrtest.c
	cc $(CFLAGS) -o mrtest mrtest.c mrandom.o

mrandom.o: mrandom.h mrandom.c
	cc $(CFLAGS) -c mrandom.c
