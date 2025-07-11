/*****************************************************************************
* 
*			  NCSA HDF Vset release 2.1
*					May 1991
*
* NCSA HDF Vset release 2.1 source code and documentation are in the public
* domain.  Specifically, we give to the public domain all rights for future
* licensing of the source code, all resale rights, and all publishing rights.
* 
* We ask, but do not require, that the following message be included in all
* derived works:
* 
* Portions developed at the National Center for Supercomputing Applications at
* the University of Illinois at Urbana-Champaign.
* 
* THE UNIVERSITY OF ILLINOIS GIVES NO WARRANTY, EXPRESSED OR IMPLIED, FOR THE
* SOFTWARE AND/OR DOCUMENTATION PROVIDED, INCLUDING, WITHOUT LIMITATION,
* WARRANTY OF MERCHANTABILITY AND WARRANTY OF FITNESS FOR A PARTICULAR PURPOSE
* 
*****************************************************************************
* Likkai Ng NCSA May 1991
*
* vio.c
* Part of the HDF Vset interface.
*
* VDATAs are handled by routines in here.
* PRIVATE functions manipulate vsdir and are used only within this file.
* PRIVATE data structures in here pertain to vdatas in vsdir only.
*
*************************************************************************/

#include <stdio.h>
#include "vg.h"

/* ------------------------------------------------------------------ */
/*
DFvsetopen and DFvsetclose
*/
#undef DFopen
#undef DFclose

PUBLIC DF * DFvsetopen (fname, access, defDDs )
char * fname;
int access, defDDs;
{
	DF * f;
	f = DFopen(fname, access, defDDs);
	Vinitialize(f);
	/* sprintf(sjs,"DFvsetopen called\n"); zj; */
	return (f);
}
/* ------------------------------------------------------------------ */
PUBLIC int DFvsetclose (dfile)
DF * dfile;
{
	int s;

	s = DFclose(dfile);
	return (s);
}

/* ------------------------------------------------------------------ */
/*
* Looks thru vstab for vsid and return the addr of the vdata instance
* where vsid is found.
* RETURNS NULL if error or not found.
* RETURNS vsinstance_t pointer if ok.
*
*/

vsinstance_t * vsinstance (f,vsid)             /*@@*/
DF  *f;
int vsid;

{
	vsinstance_t * w;
	vfile_t      * vf;

	if (NULL== (vf = (vfile_t*) Get_vfile(f)))
		RTNULL ("vsinstance: no such file\n");

	w = vf->vstab.next;
	while (NULL != w) {
		if (w->ref == vsid) return(w);
		w = w->next;
	}
	return(NULL);

} /* vsinstance */

/* ------------------------------------------------------------------ */
/*
* Tests if a vdata with id vsid is in the file's vstab.
* returns -1 if not found,
* returns 1 if found.
*/
int vexistvs (f,vsid)             /*@@*/
DF  *f;
int vsid;
{
	if (NULL== (vsinstance_t *) vsinstance(f,vsid))
		return(-1);
	else
		return (1);

} /* vexistvs */

/* ------------------------------------------------------------------ */
/*
The following 2 routines, vpackvs and vunpackvs, packs and unpacks
a VDATA structure into a compact form suitable for storing in the HDF file.
*/

/****
CONTENTS of VS stored in HDF file with tag VSDESCTAG:
	int16		interlace
	int32		nvertices
	int16		vsize
	int16		nfields

	int16		isize[1..nfields] (internal size of each field)
	int16		off[1..nfields] (internal offset of each field)
	char		fname[1..nfields][FIELDNAMELENMAX]
	char		vsname[VSNAMELENMAX]
****/


#define INT16SIZE 2
#define INT32SIZE 4

/* ------------------------------------------------------------------ */
/*
The following 2 PRIVATE routines, vpackvs and vunpackvs, packs and unpacks
a VDATA structure into a compact form suitable for storing in the HDF file.
*/

/****
CONTENTS of VS stored in HDF file with tag VSDESCTAG:
	int16		interlace
	int32		nvertices
	int16		vsize
	int16		nfields

	int16		isize[1..nfields] (internal size of each field)
	int16		off[1..nfields] (internal offset of each field)
	char		fname[1..nfields][FIELDNAMELENMAX]
	char		vsname[VSNAMELENMAX]
****/

/* 
convert a vs struct to a vspack suitable for storage in a HDF file 
*/

void vpackvs (vs,buf,size)         /*@@*/

VDATA *vs;
int   *size;
unsigned char buf[];
{
	int           i;
	unsigned char *b, *bb;
	int16         int16var;


	bb = &buf[0];

	/* save the interlace */
	b = bb;
	INT16WRITE(b,vs->interlace);
	bb+=INT16SIZE;

	/* save nvertices */
	b = bb;
	INT32WRITE(b,vs->nvertices);
	bb+=INT32SIZE;

	/* save ivsize */
	b = bb;
	INT16WRITE(b,vs->wlist.ivsize);
	bb+=INT16SIZE;

	/* save nfields */
	b = bb;
	INT16WRITE(b,vs->wlist.n);
	bb+=INT16SIZE;

	for (i=0;i<vs->wlist.n;i++) { /* save the type */
		b = bb;
		INT16WRITE(b,vs->wlist.type[i]);
		bb+=INT16SIZE;
	}
	for (i=0;i<vs->wlist.n;i++) { /* save the isize */
		b = bb;
		INT16WRITE(b,vs->wlist.isize[i]);
		bb+=INT16SIZE;
	}
	for (i=0;i<vs->wlist.n;i++) { /* save the offset	*/
		b = bb;
		INT16WRITE(b,vs->wlist.off[i]);
		bb+=INT16SIZE;
	}
	for (i=0;i<vs->wlist.n;i++)  { /* save the order */
		b = bb;
		INT16WRITE(b,vs->wlist.order[i]);
		bb+=INT16SIZE;
	}

	/* save each field length and name - omit the null */
	for (i=0;i<vs->wlist.n;i++) {
		b = bb;
		int16var = strlen(vs->wlist.name[i]);
		INT16WRITE(b,int16var);
		bb+=INT16SIZE;

		strcpy(bb, vs->wlist.name[i]);
		bb += strlen(vs->wlist.name[i]);
	}

	/* save the vsnamelen and vsname - omit the null */
	b = bb;
	int16var = strlen(vs->vsname);
	INT16WRITE(b,int16var);
	bb+=INT16SIZE;

	strcpy(bb,vs->vsname);
	bb += strlen(vs->vsname);

	/* save the vsclasslen and vsclass- omit the null */
	b = bb;
	int16var = strlen(vs->vsclass);
	INT16WRITE(b,int16var);
	bb+=INT16SIZE;

	strcpy(bb,vs->vsclass);
	bb += strlen(vs->vsclass);

	/* save the expansion tag/ref pair */
	b= bb;
	INT16WRITE(b,vs->extag);
	bb +=INT16SIZE;

	b= bb;
	INT16WRITE(b,vs->exref);
	bb +=INT16SIZE;

	/* save the version field - init to version_2 now */
	b= bb;
	INT16WRITE(b,vs->version);
	bb +=INT16SIZE;

	/* save the 'more' field - NONE now */
	b= bb;
	INT16WRITE(b,vs->more);
	bb +=INT16SIZE;

	*size = bb - buf + 1;

} /* vpackvs */

/* ------------------------------------------------------------------ */
/* 
Convert a packed form(from HDF file)  to a VDATA structure.
This routine will also initalize the VDATA structure as much as it can.
*/

void vunpackvs (vs,buf,size)       /*@@*/

VDATA *vs;
int   *size;			/* UNUSED, but retained for compatibility with vpackvs */
unsigned char buf[];
{
	unsigned char *b, *bb;
	int            i;
	int				int16var;

	i = *size; /* dum */

	bb = &buf[0];

	/* retrieve interlace */
	b = bb;
	INT16READ(b,vs->interlace);
	bb += INT16SIZE;

	/* retrieve nvertices */
	b = bb;
	INT32READ(b,vs->nvertices);
	bb += INT32SIZE;

	/* retrieve tore ivsize */
	b = bb;
	INT16READ(b,vs->wlist.ivsize);
	bb += INT16SIZE;

	/* retrieve nfields */
	b = bb;
	INT16READ(b,vs->wlist.n);
	bb += INT16SIZE;

	for (i=0;i<vs->wlist.n;i++)  { /* retrieve the type */
		b = bb;
		INT16READ(b,vs->wlist.type[i]);
		bb += INT16SIZE;
	}

	for (i=0;i<vs->wlist.n;i++)  { /* retrieve the isize */
		b = bb;
		INT16READ(b,vs->wlist.isize[i]);
		bb += INT16SIZE;
	}

	for (i=0;i<vs->wlist.n;i++)  { /* retrieve the offset */
		b = bb;
		INT16READ(b,vs->wlist.off[i]);
		bb += INT16SIZE;
	}

	for (i=0;i<vs->wlist.n;i++)  { /* retrieve the order */
		b = bb;
		INT16READ(b,vs->wlist.order[i]);
		bb += INT16SIZE;
	}

	/* retrieve the field names (and each field name's length)  */
	for (i=0;i<vs->wlist.n;i++) {
		b = bb;
		INT16READ(b,int16var); /* this gives the length */
		bb += INT16SIZE;

		strncpy(vs->wlist.name[i], bb, int16var);
		vs->wlist.name[i][int16var] = '\0';
		bb += int16var;
	}

	/* retrieve the vsname (and vsnamelen)  */
	b = bb;
	INT16READ(b,int16var); /* this gives the length */
	bb += INT16SIZE;

	strncpy(vs->vsname, bb, int16var);
	vs->vsname[int16var] = '\0';
	bb += int16var;

	/* retrieve the vsclass (and vsclasslen)  */
	b = bb;
	INT16READ(b,int16var); /* this gives the length */
	bb += INT16SIZE;

	strncpy(vs->vsclass, bb, int16var);
	vs->vsclass[int16var] = '\0';
	bb += int16var;

	/* retrieve the expansion tag and ref */
	b = bb;
	INT16READ(b,vs->extag);
	bb += INT16SIZE;

	b = bb;
	INT16READ(b,vs->exref);
	bb += INT16SIZE;

	/* retrieve the version field */
	b = bb;
	INT16READ(b,vs->version);
	bb += INT16SIZE;

	/* retrieve the 'more' field */
	b = bb;
	INT16READ(b,vs->more);
	bb += INT16SIZE;

	/* **EXTRA**  fill in the machine-dependent size fields */
	for (i=0;i<vs->wlist.n;i++) {
		vs->wlist.esize[i] = vs->wlist.order[i] * SIZEOF(vs->wlist.type[i]);
	}

} /* vunpackvs */

/* ------------------------------------------------------------------ */



/* ***************************************************************
   VSattach: 
	if vsid == -1, then
	(a) if vg is "w", create a new vs in vg and attach it.
							add to vsdir, set nattach= 1, nvertices = 0.
	(b) if vg is "r", forbidden.
   if vsid is +ve, then
	(a) if vg is "w"  => new data may be added BUT must be same format
								as existing vdata.
								(ie. VSsetfields must match old format exactly!!)

	(b) if vg is "r"  => look in vsdir
								if not found,
									fetch  vs from file, add to vsdir,  
									set nattach= 1, nvertices = val from file.
								if found,
									check access of found vs
									if "w" => being written, unstable! forbidden
									if "r" => ok. incr nattach.
	
	in all cases, set the marked flag to 0.
	returns NULL if error.
   *************************************************************** */

PUBLIC VDATA * VSattach (f,vsid,accesstype)       /*@-@*/

DF		*f;
int 	vsid;
char *accesstype;
{
	VDATA 			*vs;  			 /* new vdata to be returned */
	int 				vspacksize;
	unsigned char 	vspack[sizeof(VWRITELIST)];
	int				access;
	vsinstance_t	* w;
	vfile_t			* vf;

	if (f==NULL)
		RTNULL("vsattach:bad f");

	if (NULL==(vf = (vfile_t*) Get_vfile(f)))
		RTNULL ("Vattach: no such file\n");

	if      ( accesstype[0]=='R' || accesstype[0]=='r') { 
		access = 'r'; 
	}
	else if ( accesstype[0]=='W' || accesstype[0]=='w') { 
		access = 'w'; 
	}
	else RTNULL("VSattach: bad access type");

	if (vjv) {
		sprintf(sjs,"#VSATTACH:vsid=%d access=%s\n",vsid, accesstype);
		zj;
	}

	if (vsid < -1)	{			/* --------- ILLEGAL VSID ---------------------- */
		RTNULL("VSattach:bad vsid");
	}

	else if (vsid == -1) {  /* ---------- VSID IS -1 ----------------------- */
		if (access == 'r') RTNULL("VSattach: may not read vsid of -1");
		/* otherwise 'w' */

		/* allocate space for vs,  & zero it out  */
		if ( (vs=(VDATA*) DFIgetspace(sizeof(VDATA))) == NULL)
			RTNULL("VSattach: cannot getspace");
		zerofill ((unsigned char*) vs,sizeof(VDATA));

		vs->oref				= vnewref(f);
		if ( vs->oref < 0) RTNULL("VSattach: no more refs");
		vs->otag				= VSDESCTAG;
		vs->vsname[0] 		= '\0';
		vs->interlace		= FULL_INTERLACE; /* DEFAULT */
		vs->access			= 'w';
		vs->f					= f;
		vs->marked			= 0;

		vs->vsclass[0]		= '\0';
		vs->extag			= 0;
		vs->exref			= 0;
		vs->more				= 0;
		vs->version			= VSET_VERSION;

		/* attach new vs to file's vstab */
		if ( NULL == (w = (vsinstance_t*) DFIgetspace(sizeof(vsinstance_t))))
			RTNULL("VSattach: cannot getspace for vstab\n");

		vf->vstabtail->next = w;
		vf->vstabtail      = w;
		vf->vstabn++;
		w->next        = NULL;

		w->ref       = vs->oref;
		w->vs        = vs;
		w->nattach   = 1;
		w->nvertices = 0;

		return (vs);
	}


	else {                  /*  --------  VSID IS NON_NEGATIVE ------------- */

		if (access == 'w') {
			RTNULL("VSattach: may not write to existing vs");
		}
		/* otherwise 'r' */

		if (NULL == (w = (vsinstance_t*) vsinstance (f,vsid)) )
			RTNULL("VSattach: vsid not in vstab");

		if (w->vs != NULL) { /* already attached */
			if ( (w->vs)->access == 'w') {
				RTNULL("VSattach: existing vs already attached for write");
			}
			else { /* existing vs as attached for 'r' */
				w->nattach ++;
				return (w->vs);
			}
		}
		else { /* need to fetch from file */

			if (-1 ==  DFgetelement(f,VSDESCTAG,vsid,vspack) )
				RTNULL("VSattach: vs  not in file");

			/* allocate space for vs,  & zero it out  */
			if ( (vs=(VDATA*) DFIgetspace(sizeof(VDATA))) == NULL)
				RTNULL("VSattach: cannot getspace");
			zerofill((unsigned char*) vs,sizeof(VDATA));

			/* unpack the vs, then init all other fields in it */
			vunpackvs (vs,vspack,&vspacksize);
			vs->otag				= VSDESCTAG;
			vs->oref				= vsid;
			vs->access			= 'r';
			vs->f					= f;
			vs->vpos 			= 0;
			vs->marked			= 0;

			/* attach vs to vsdir  at the vdata instance w */
			w->vs        = vs;
			w->nattach   = 1;
			w->nvertices = vs->nvertices;

			return(vs);
		}
	}

} /* VSattach */

/* ------------------------------------------------------------------ */

/* *************************************************************** 
 	Detach vs from vstab. 

	if vs has "w" access,   ( <=> only attached ONCE! )
		decr nattach.
		if (nattach is not  0)  => bad error in code.
		if nvertices (in vs) is 0) just free vs from vstab.

		if marked flag is 1
			write out vs to file and set marked flag to 0.
		   free vs from vsdir.

	if vs has "r" access,   ( <=> only attached ONCE! )
		decr nattach.
		if (nattach is 0)   just free vs from vstab.
			
   *************************************************************** */

PUBLIC void VSdetach (vs)        /*@-@*/

VDATA  *vs;
{
	int				i, vspacksize;
	unsigned char  vspack[sizeof(VWRITELIST)];
	vsinstance_t	 * w;
	char 				ss[80];

	if (vs==NULL) RT("VSdetach: bad vs");
	if (vs->otag != VSDESCTAG) return;

	/* locate vs's entry in vstab */
	if (NULL ==( w = (vsinstance_t*) vsinstance (vs->f,vs->oref))) {
		sprintf(sjs,"@Vdetach: vg not found\n");
		zj;
		return;
	}

	w->nattach--;

	if (vs->access =='w') {
		if (w->nattach != 0) {
			sprintf(ss,"VSdetach: nattach serious error. [%d] not 0", w->nattach);
			RT(ss);
		}

		if (vs->marked)  { /* if marked , write out vdata's VSDESC to file */
			if (vs->nvertices == 0) {
			} /* if vdata is empty, don't write to file */
			else {
				vpackvs(vs,vspack,&vspacksize);
				if ( DFputelement(vs->f,VSDESCTAG,vs->oref,vspack,vspacksize) == -1)
					RT("VSdetach: cannot write out vs");
			}
			vs->marked = 0;
		}

		if (vjv && vs->nvertices<=0){
			sprintf(sjs,"#VSdetach:vs->nver=%d!\n",vs->nvertices);
			zj;
		}

		/* remove all defined symbols */
		for (i=0;i<vs->nusym;i++) DFIfreespace(vs->usym[i].name);
		vs->nusym = 0;

		w->vs = NULL; /* detach vs from vsdir */
		DFIfreespace(vs);
	}
	else { /* access was 'r' */
		if (w->nattach == 0) {
			w->vs = NULL; /* detach vs from vsdir */
			DFIfreespace(vs);
		}
	}
	return;

} /* VSdetach */

/* ------------------------------------------------------------------ */


/* Visvs
*  checks if an id in a vgroup refers to a VDATA
*  RETURNS 1 if so
*  RETURNS 0 if not, or if error.
*/

PUBLIC int Visvs (vg,id)           /*@-@*/

VGROUP  *vg;
int id;
{
	int i;
	for (i=0;i<vg->nvelt;i++)
		if (vg->ref[i] == id && vg->tag[i]==VSDESCTAG) return(1);

	return(0);

} /* Visvs */

/* ======================================================= */

/* 
returns the id of the next  VDATA from the file f .
(vsid = -1 gets the 1st vDATA). 
RETURNS -1 on error.
RETURNS vdata id (0 or +ve integer) 
*/

PUBLIC int VSgetid(f,vsid)           /*@-@*/

int vsid;
DF  *f;
{
	vsinstance_t * w;
	vfile_t		 * vf;

	if (vsid < -1) return(FAIL);

	if (NULL==(vf = (vfile_t*) Get_vfile(f)))
		RTNEG ("VSgetid: no such file\n");


	if (vjv) {
		sprintf(sjs,"#VSgetid:vstabn is %d\n",vf->vstabn);
		zj;
	}

	if (vsid == -1) {
		if (NULL == vf->vstab.next)
			return (-1);
		else
			return((vf->vstab.next)->ref); /* rets 1st vdata's ref */
	}

	/* look in vstab  for vsid */
	w = (vf->vstab).next;
	while(NULL != w) {
		if (w->ref == vsid) break;
		w = w->next;
	}

	if (w==NULL)
		return(FAIL);			/* none found */
	else if (w->next == NULL)
		return(FAIL);			/* this is the last vdata, no more after it */
	else
		return( (w->next)->ref);  /* success, return the next vdata's ref */

} /* VSgetid */

/* ------------------------------------------------------------------ */
/*
zero out n bytes in array x.
*/
zerofill(x,n)           /*@@*/
unsigned char* x;
int n;
{
	register int i;
	for(i=0;i<n;i++) *x++ = 0;
} /* zerofill */
/* ------------------------------------------------------------------ */
