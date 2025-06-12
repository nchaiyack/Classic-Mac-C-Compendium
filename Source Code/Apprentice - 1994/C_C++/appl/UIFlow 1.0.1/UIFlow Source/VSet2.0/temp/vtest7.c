#include "vg.h"
#define FS "vtest7.hdf"
main(ac,av) int ac; char**av; {
 int num;
 
  printf("This is an example of writing sequentially to several vdatas simultaneously.\n"); 
  printf("Note that the file size is far bigger than expected.\n");

  if(ac!=2) num = 2; else sscanf(av[1],"%d",&num);
	printf("%s: creates %d vgroups and %d vdatas in %s\n",av[0],num,num,FS);
	createm(FS,num);
}
createm(fs,n) char*fs; int n;{
  VGROUP * vg;
  VDATA * vs[10];
	DF * f;
	int i,j, nv;
	float bb[100];

	for(i=0;i<100;i++) bb[i] = 1000.0 + i;	

	if(n>=10) n=10;
	printf("will create and write to %d vdatas\n",n);

	if( NULL==(f=DFopen(fs,DFACC_ALL,0))) { printf("open err%s\n",fs); exit(0);}

	for(i=0;i<n;i++) {
	  vs[i] = (VDATA*) VSattach(f,-1,"w");
	  VSsetfields(vs[i],"IX");
	  }

	nv = 1;
	for(j=0;j<10;j++) {
	  for(i=0;i<n;i++)  VSwrite(vs[i],bb,nv,FULL_INTERLACE);
	  nv++;
	}

	for(i=0;i<n;i++) { VSdetach(vs[i]); }

	DFclose(f);
}

