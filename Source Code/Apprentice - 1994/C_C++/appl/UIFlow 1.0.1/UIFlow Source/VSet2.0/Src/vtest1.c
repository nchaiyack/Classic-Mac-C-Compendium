#include "vg.h"
#define FS "vtest1.hdf"
main(ac,av) int ac; char**av; {
 int num;
 
  if(ac!=2) num = 10; else sscanf(av[1],"%d",&num);
	printf("%s: creates %d vgroups and %d vdatas in %s\n",av[0],num,num,FS);
	createm(FS,num);
}
createm(fs,n) char*fs; int n;{
  VGROUP * vg;
  VDATA * vs;
	DF * f;
	char ss[30];
	int i;

	if( NULL==(f=DFopen(fs,DFACC_ALL,0))) { printf("open err%s\n",fs); exit(0);}
	for(i=0;i<n;i++) {
	  vg = (VGROUP*) Vattach(f,-1,"w");
	  sprintf(ss,"gp#%d",i); Vsetname(vg,ss);
	  Vdetach(vg);
	  }
	for(i=0;i<n;i++) {
	  vs = (VDATA*) VSattach(f,-1,"w");
	  sprintf(ss,"vdata#%d",i); VSsetname(vs,ss); 
	  VSsetfields(vs,"IX");
	  VSwrite(vs,ss,1,FULL_INTERLACE);
	  VSdetach(vs);
	  }
	DFclose(f);
}

