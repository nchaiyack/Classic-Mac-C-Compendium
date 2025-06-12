
#include "vg.h"

main (ac,av) int ac; char**av;
{
char *srcfile, *dstfile;

if (ac!=3) {
  fprintf (stderr, "%s: appends one vset file into another\n",av[0]);
  fprintf (stderr, "usage %s srcfile dstfile\n",av[0]);
  exit(0);
  }
  srcfile = av[1];
  dstfile = av[2];
 vappendfile (srcfile, dstfile);
}
/* ------------------------------------------------------------------ */

#define VMAXSIZE 12

static int nvg = 0, nvs = 0;
static VGROUP *  vgnewptr[VMAXSIZE]; /* new attached vgroups in dst file */
static int vgoldref[VMAXSIZE], vgnewref[VMAXSIZE];
static int vsoldref[VMAXSIZE], vsnewref[VMAXSIZE];

/* ------------------------------------------------------------------ */

/* 
* Given tag and (old) ref, this function looks in either vgoldref[] or
* vsoldref[] and returns the corrsponding new ref from vgnewref[] or 
* vsnewref[]. 
* Return value is a ref (+ve integer) if found.
* If error or not found, -1 is returned.
*/

static int lookupnewref ( tag, ref) 
int tag, ref;
{
	int i;
	VGROUP * vg, * vgnew;

	if (tag == VSDESCTAG) {
		for(i=0;i<nvs;i++) if (ref == vsoldref[i]) return (vsnewref[i]);
	}
	else if (tag == VGDESCTAG) {
		for(i=0;i<nvg;i++) if (ref == vgoldref[i]) return (vgnewref[i]);
	}
	return (-1);
}

/* ------------------------------------------------------------------ */
/* kkk */
vappendfile (srcfile, dstfile) 
char *srcfile, *dstfile; 
{
   DF 		*f, *g;
	int 		ref, newref;
	DFdesc 	desc;
	VGROUP 	*vg, *vgnew;
	int		i,j;
	unsigned char* buf;

   if (NULL==(f=DFopen(srcfile,DFACC_READ,0))) {
	fprintf(stderr,"cannot open srcfile %s. DFerror=%d\n", srcfile, DFerror);
	exit(0);
	}
   if (NULL==(g=DFopen(dstfile,DFACC_ALL,0))) {
	fprintf(stderr,"cannot open dstfile %s. DFerror=%d\n", dstfile, DFerror);
	exit(0);
	}

	/* read in the id's of the vdatas in srcfile */
		ref = -1; 
		while( -1 != (ref = VSgetid (f, ref))) {
			if (nvs>=VMAXSIZE-1) { printf("Vdata work tables full\n"); exit(0);}
			vsoldref[nvs]  = ref;
			nvs++;
		}

	/* read in the id's of the vgroups in srcfile */
		ref = -1; 
		while( -1 != (ref = Vgetid (f, ref))) {
			if (nvg>=VMAXSIZE-1) { printf("Vgroup work tables full\n"); exit(0);}
			vgoldref[nvg]  = ref;
			nvg++;
		}

	/* ----------- Next, move all vdatas ----------- */
		for(i=0;i<nvs;i++) {

			/* create a new id for each vdata in dstfile */
				ref 			= vsoldref[i];
				newref 		= vnewref (g);
				vsnewref[i] = newref;

			/* using that id, copy the vdata data into dstfile */
				DFsetfind(f, VSDATATAG, ref);
				DFfind(f, &desc);
				buf = (unsigned char*) DFIgetspace ( desc.length );
				if (buf==NULL) { printf("no space for vsdata\n"); exit(0); }
				DFgetelement (f, VSDATATAG, ref, buf);
				DFputelement (g, VSDATATAG, newref, buf, desc.length);
				DFIfreespace (buf);

			/* using that id, copy the vdata descriptor into dstfile */
				DFsetfind(f, VSDESCTAG, ref);
				DFfind(f, &desc);
				buf = (unsigned char*) DFIgetspace ( desc.length );
				if (buf==NULL) { printf("no space for vsdesc\n"); exit(0); }
				DFgetelement (f, VSDESCTAG, ref, buf);
				DFputelement (g, VSDESCTAG, newref, buf, desc.length);
				DFIfreespace (buf);
		}

	/* ----------- Finally, move the vgroups ------- */

		/* create a new vgroup in dstfile for each vgroup in srcfile */
			for(i=0;i<nvg;i++) {
				if (NULL == (vg = (VGROUP*) Vattach (g,-1,"w"))) 
				 			{ printf("vgroup attach error in dstfile\n"); exit(0); }
				vgnewptr[i]	= vg;
				vgnewref[i]	= vg->oref;
			}

		/* update new vgroups with new refs, and then write out the new vgroups */
			for(i=0;i<nvg;i++) {
				vgnew = vgnewptr[i];
				vg = (VGROUP*) Vattach (f, vgoldref[i], "r");

				for (j=0;j<vg->nvelt;j++) {
					newref = lookupnewref (vg->tag[j], vg->ref[j]);
					vinsertpair (vgnew, vg->tag[j], newref);
				}
				/* === is this all??? more???? === */
				Vsetname(vgnew, vg->vgname);
				Vsetclass(vgnew, vg->vgclass);

				Vdetach (vgnew);
				Vdetach (vg);
			}

	/* ----------- all done, close all files ---- */
		DFclose(f);
		DFclose(g);
}
