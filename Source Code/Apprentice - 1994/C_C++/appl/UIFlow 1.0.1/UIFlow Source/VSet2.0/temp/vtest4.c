#include "vg.h"
#define MAXF 50
main() {
DF * f[MAXF];
char fs[MAXF][100];
VGROUP * vg[MAXF];
VDATA  * vsa, * vsb;
char ss[50];
int i,j,k;
int bb[5000];
int nfiles;

printf("This test programs opens multiple files [jj*] and then writes ");
printf(" a vset (one vgroup and 2 vdatas) into each file\n\n");

printf(" how many files? (max=%d)\n",MAXF); scanf("%d",&nfiles);

if(nfiles>=MAXF) nfiles=MAXF;

for(i=0;i<5000;i++) bb[i] =i;

for(i=0;i<nfiles;i++) {
  sprintf(fs[i],"jj%d",i);
  if( NULL==(f[i]=DFopen(fs[i],DFACC_ALL,0))) 
    { printf(" %s open err\n",fs[i]); exit(0); }
  }
printf(" %d files opned ok \n",nfiles);

for(i=0;i<nfiles;i++) {
  vg[i] = (VGROUP*) Vattach(f[i],-1,"w"); 
  sprintf(ss,"MAIN vgroup in %s",fs[i]); Vsetname(vg[i],ss);
  }

for(i=0;i<nfiles;i++) {
  vsa = (VDATA*) VSattach(f[i],-1,"w");
  vsb = (VDATA*) VSattach(f[i],-1,"w");
  VSsetfields(vsa,"IX");
  VSsetfields(vsb,"IY");
    VSwrite(vsa,&bb[200+i*10],3,FULL_INTERLACE);
    VSwrite(vsb,&bb[i*10],7,FULL_INTERLACE);
  Vinsert(vg[i],vsa);
  Vinsert(vg[i],vsb);
  VSdetach(vsa);
  VSdetach(vsb);
  }

for(i=0;i<nfiles;i++)  Vdetach(vg[i]);

for(i=0;i<nfiles;i++)  DFclose(f[i]);

}

