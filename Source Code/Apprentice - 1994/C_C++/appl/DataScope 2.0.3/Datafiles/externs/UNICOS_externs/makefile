#Remember that DS_serve must be in your home directory
DS_serve	: dsfn.h DScope.h dsfns.c DS_serve.c dsfnF.o
	cc -DUNICOS DS_serve.c dsfns.c dsfnF.o -lf -lu -lm -lnet -o DS_serve

dsfnF.o	: dsfnF.f
	cf77 -c dsfnF.f
