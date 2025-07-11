/*
	SAMPLE C PROGRAM

	HDF VSET 2.1
	Jason Ng NCSA MAY 1991

	This program demonstrates the use of the high-level VSET write routines.
	It shows how data can be stored in vdatas, and how vgroups can be created
	from vdatas, vgroups and other HDF elements.

	This example creates a file "vtesthi.hdf" that may be viewed using the
	vset utility "vshow".

	DETAILS

	This example shows how pressure data  and color data can be stored as a 
	vset.  To store pressure data, a field named "PRES", is defined of 
	float type. The routine VHstoredata() stores the pressure values in a vdata.

	Color data comprises 3 components (red, green, blue). These can be stored as
	3 different fields in 3 vdatas. But this example shows that they can be
	treated as ONE field, and stored together as a compound field called "COLOR".
	The number of components of a field is called its order.  In this case, 
	"COLOR" has order=3. (whereas "PRES" above has order=1).

	The routine VHstoredatam() must be used to store values of a compound field.
	This is similar to VHstoredata() but has an extra argument for the order
	to be specified.

	Finally a vgroup is created, and the ids of the created vdatas are stored
	in the vgroup. This effectively groups the vdatas together. This example
	also shows that you can insert another vgroup, as well as a non-vset element
	(in this case, some element with tag=7777 and ref=1) into a vgroup.
*/

#include "vg.h"

#define fs "vtesthi.hdf"
#define NP 100
#define NC 60
#define ORDERC 3 /* 3 color components: rgb */

main (ac,av)   int ac; char**av; {
  int n = ac;

  printf("%s: tests high-level routines\n", av[0]);
  printf("Creates 2 vdatas, a vgroup, and a non-vset element\n");
  printf("then link them all into another vgroup\n");
  printf("Vdata 1 contains an order-1 float field PRES\n");
  printf("Vdata 2 contains an order-3  integer field COLOR\n\n");
  printf("The non-vset element has tag=7777, ref=1\n");
  doit();
  printf("all done. created file %s\n", fs);
}

/* ------------------------------------------------------------------ */

doit() {
DF * f;
float pvals[NP];
int cvals[NC][ORDERC];
int i,j;

int pid, cid; /* refs of vdatas */
int eid; /* empty vgroup's ref  */
int gid; /* vgroup's ref  */
int tags[10], refs[10];
char *CLS = "EXAMPLE";

/* --- generate data here --- */
  for(i=0;i<NP;i++)  pvals[i] = 100.0  + i * 0.001;
  for(i=0;i<NC;i++) for(j=0;j<3;j++)  cvals[i][j] =   i + j*100;


/* ---- open a new file --- */

if (NULL==(f=DFopen(fs,DFACC_ALL,0))) { printf("open %s err\n",fs); exit(0); }

/* ---- create 2 vdatas --- */
pid = VHstoredata (f, "PRES", (unsigned char*) pvals ,NP, LOCAL_FLOATTYPE, "pressure vals",CLS);
if (pid  == -1) { printf(" VHstoredata store PRES err. "); }

cid  = VHstoredatam(f, "COLOR",(unsigned char*) cvals, NC, LOCAL_INTTYPE, "rgb colors", CLS, 3);
if (cid  == -1) { printf(" VHstoredata store COLOR err. "); }

eid  = VHmakegroup (f, tags, refs, 0, "This is an EMPTY vgroup", CLS);
if (eid  == -1) { printf(" VHmakegroup err\n"); }

/* --- create a new vgroup to store the 2 vdatas and the empty vgroup -- */
tags[0] = VSDESCTAG;  refs[0] = pid;
tags[1] = VSDESCTAG;  refs[1] = cid;
tags[2] = VGDESCTAG;  refs[2] = eid;
tags[3] = 7777 ;      refs[3] = 1;

gid  = VHmakegroup (f, tags, refs, 4, "here is a vset with 4 links", CLS);
if (eid  == -1) { printf(" VHmakegroup err\n"); }

/* --- close the file --- */
DFclose (f);
}
