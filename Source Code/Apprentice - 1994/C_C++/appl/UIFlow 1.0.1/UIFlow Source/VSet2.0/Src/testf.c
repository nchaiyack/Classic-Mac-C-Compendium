#include "vg.h"
#define MAXF 3

main() {
DF * f[MAXF];
char fs[MAXF][100];
VGROUP * vg[MAXF];
VDATA  * vs[MAXF];
char ss[50];
int i,j,k,s;
float bb[MAXF][500];
int nfiles;
int nvals;
int diff;
float fval;

nfiles = 2;
nvals = 210;
for(j=0;j<MAXF;j++) for(i=0;i<500;i++) bb[j][i] = -55.0;

for(i=0;i<nfiles;i++) {
  sprintf(fs[i],"f%d.hdf",i+1);
  if( NULL==(f[i]=DFopen(fs[i],DFACC_READ,0))) 
    { printf(" %s open err\n",fs[i]); exit(0); }
  setjj();

  vs[i] = (VDATA*) VSattach(f[i],2,"r"); if(vs[i]==NULL) printf("is null!\n");
  s = VSsetfields(vs[i],"PX");
  printf("stefields status: %d\n",s);
  s = VSread (vs[i],bb[i],nvals,FULL_INTERLACE);
  printf("read status: %d\n",s);

  VSdetach(vs[i]);
  DFclose(f[i]);
  }
printf("These are PX vdata values read from each of the %d files:\n",nfiles);
for(j=0;j<nvals; j++) {
	printf("%03d: ", j);
	/* check if different */
	fval = bb[0][j]; diff = 0;
	for(i=0;i<nfiles;i++) {
		if( fval!=bb[i][j]) { diff = 1; break; }
	}
	/* print data vlues */
	if (diff) 
		for(i=0;i<nfiles;i++) { printf("%f ", bb[i][j]); }
	else
		printf("%f ", fval); 
	printf("\n");
	}
}

