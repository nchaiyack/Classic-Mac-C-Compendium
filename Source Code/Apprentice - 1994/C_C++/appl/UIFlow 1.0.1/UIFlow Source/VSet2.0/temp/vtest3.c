#include "vg.h"
#define filef "vta.hdf"
#define fileg "vtb.hdf"
main() {
DF * f, *g;
VGROUP* vg1, * vg2;
VDATA *vs1, * vs2;
char ss[50];
int i,j,k;
int bb[5000];
for(i=0;i<5000;i++) bb[i] =i;
printf("this test creates 2 files [%s] and [%s] and perform sequential ",
filef, fileg);
printf(" writes on 3 vdatas in each file simultaneously\n");

  if( NULL==(f=DFopen(filef,DFACC_ALL,0))) 
    { printf(" %s open err\n",filef); exit(0); }

  if( NULL==(g=DFopen(fileg,DFACC_ALL,0))) 
    { printf(" %s open err\n",fileg); exit(0); }
printf(" both files opned ok \n");


/*** PART 3: vgroups and vdatas  with sequential writes ***/

 vg1 = (VGROUP*) Vattach(f,-1,"w"); 
 sprintf(ss,"vgroup ONE  in %s",filef); Vsetname(vg1,ss);
 fprintf(stderr,"******vg [%s] ok\n",ss);

 vg2 = (VGROUP*) Vattach(g,-1,"w"); 
 sprintf(ss,"vgroup TWO  in %s",fileg); Vsetname(vg2,ss);
 fprintf(stderr,"******vg [%s] ok\n",ss);

for(i=0;i<2;i++) {
  vs1 = (VDATA*) VSattach(f,-1,"w");
  vs2 = (VDATA*) VSattach(g,-1,"w");
  VSsetfields(vs1,"IX");
  VSsetfields(vs2,"IY");
  for(j=0;j<10;j++) {
    VSwrite(vs1,(unsigned char*) &bb[200+j*10],3,FULL_INTERLACE);
    VSwrite(vs2,(unsigned char*)&bb[100+j*10],5,FULL_INTERLACE);
   }
  Vinsert(vg1,vs1);
  Vinsert(vg2,vs2);
  VSdetach(vs1);
  VSdetach(vs2);
  }

  Vdetach(vg1);
  Vdetach(vg2);


DFclose(f);
DFclose(g);

}

