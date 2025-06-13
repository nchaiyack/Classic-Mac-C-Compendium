#include <stdio.h>
#ifdef THINK_C
#include "plot.h"
#include "tout_protos.h"
#endif

int chdir(char *fn)
{
	fprintf(stderr,"Sorry, I cannot chdir to %s.\n", fn);
	return((int) -1);
 }
 
 char *getcwd(char *fn,int len){
	fprintf(stderr,"Sorry, I cannot getcwd.\n");
 	return ("");
 }
