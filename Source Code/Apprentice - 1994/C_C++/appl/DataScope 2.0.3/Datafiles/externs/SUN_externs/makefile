#Remember that DS_serve must be in your home directory
DS_serve:  DS_serve.c dsfns.c dsfn.h DScope.h dsfnF.o
	cc -D SUN -g DS_serve.c dsfns.c dsfnF.o -lF77 -lI77 -lm -fswitch -o DS_serve

dsfnF.o: dsfnF.f
	f77 dsfnF.f -c -o dsfnF.o

