#include "vg.h"
#define fs "lonevs.hdf"

main(ac,av) int ac; char**av; {

VDATA * vs;
DF * f;
int bb[100];
int i,n;

printf("generates many lone vdatas in  file %s\n",fs);
if (ac!=2) { printf("use: %s nvdatas\n",av[0]); exit(0); }
sscanf(av[1],"%d",&n); printf("n is %d\n",n);

for(i=0;i<100;i++) bb[i] = i+1000; /* some data to be stored */

if (NULL == (f=DFopen(fs,DFACC_ALL,0))) {
   printf("cannot open %s\n",fs); exit(0);
   }

for(i=0;i<n;i++) {
   vs = (VDATA*) VSattach(f,-1,"w");
	VSsetfields(vs,"IX");
   VSwrite(vs,bb,(int)( rand()%30+11),FULL_INTERLACE);
   VSdetach(vs);
   }
DFclose(f);
printf("all done: file is %s\n",fs);
}


