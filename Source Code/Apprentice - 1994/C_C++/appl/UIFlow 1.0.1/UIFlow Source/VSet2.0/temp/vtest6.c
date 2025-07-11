#include "vg.h"
main(ac,av) int ac; char**av; {
DF * f;
char * fname;
char name[80];
int id;

printf("%s: tests the routines VSfind() and Vfind()\n",av[0]);
printf(" Enter a vgroup or vdata name, and the program will look in the\n");
printf("given hdf-file and return the id (ref) of the vgroup or vdata\n\n");

if (ac!=2) { printf("usage: %s hdffile\n",av[0]); exit(0); }

fname = av[1];
if (NULL == (f=DFopen(fname,DFACC_ALL,0))) {
	printf("error open %s\n", fname); exit(0);	
	}
while(1) {
  printf("find what? (or 'quit') ");
  scanf("%s",name); if (!strcmp(name,"quit")) break;
  id = VSfind (f,name);	
  printf("VSfind [%s] rets id= %d\n", name, id);
  id = Vfind (f,name);	
  printf("Vfind [%s] rets id= %d\n", name, id);
  }
DFclose(f);
}

