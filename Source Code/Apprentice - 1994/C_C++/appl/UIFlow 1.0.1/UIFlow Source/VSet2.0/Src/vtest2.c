#include "vg.h"
#define filef "jj"
#define fileg "jjj"
main() {
DF * f, *g;
VGROUP* vg1, * vg2;
VDATA *vs1, * vs2;
char ss[50];
int i;
int bb[5000];
for(i=0;i<5000;i++) bb[i] =i;

  if( NULL==(f=DFopen(filef,DFACC_ALL,0))) 
    { printf(" %s open err\n",filef); exit(0); }

  if( NULL==(g=DFopen(fileg,DFACC_ALL,0))) 
    { printf(" %s open err\n",fileg); exit(0); }
printf(" both files opned ok \n");

/*** PART 1: vgroups only ***/
/**
for(i=0;i<3;i++) {
 vg1 = (VGROUP*) Vattach(f,-1,"w"); 
 sprintf(ss,"vgroup  %d   in %s",i,filef); Vsetname(vg1,ss);
 fprintf(stderr,"******vg [%s] ok\n",ss);

 vg2 = (VGROUP*) Vattach(g,-1,"w"); 
 sprintf(ss,"vgroup %d   in %s",i,fileg); Vsetname(vg2,ss);
 fprintf(stderr,"******vg [%s] ok\n",ss);
  Vdetach(vg1);
  Vdetach(vg2);
}
***/
/*** PART 2: vgroups and vdatas ***/

 vg1 = (VGROUP*) Vattach(f,-1,"w"); 
 sprintf(ss,"vgroup ONE  in %s",filef); Vsetname(vg1,ss);
 fprintf(stderr,"******vg [%s] ok\n",ss);

 vg2 = (VGROUP*) Vattach(g,-1,"w"); 
 sprintf(ss,"vgroup TWO  in %s",fileg); Vsetname(vg2,ss);
 fprintf(stderr,"******vg [%s] ok\n",ss);

for(i=0;i<3;i++) {
  vs1 = (VDATA*) VSattach(f,-1,"w");
  vs2 = (VDATA*) VSattach(g,-1,"w");
  VSsetfields(vs1,"IX");
  VSsetfields(vs2,"IY");
  VSwrite(vs1,&bb[i*100],10,FULL_INTERLACE);
  VSwrite(vs2,&bb[i*50],20,FULL_INTERLACE);
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

