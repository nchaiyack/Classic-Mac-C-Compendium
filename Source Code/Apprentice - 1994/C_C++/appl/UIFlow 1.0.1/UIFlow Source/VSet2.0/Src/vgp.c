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
* vgp.c
* Part of the vertex-set interface.
* VGROUPs are handled by routines in here.
*
*************************************************************************/

#include "vg.h"

/* ------------------------------------------------------------ */
/* 
*
* G L O B A L S 
*
* These are the *only* globals in the vset interface!!  
* They are used only for debugging.  No globals in other files. 
* The 2 functions below turn debugging on and off.
*
*/

PUBLIC	char 		sjs[200];
PUBLIC	int      vjv	=	0;

/*
turn debug on 
*/
PUBLIC void	setjj() 	         /*@-@*/
{ 
	vjv=1; 
}

/*
turn debug off 
*/
PUBLIC void setnojj()         /*@-@*/
{ 
	vjv=0; 
}


/*
* -------------------------------------------------------------------- 
* PRIVATE  data structure and routines.
* 
* Info about all vgroups in the file are loaded into vgtab  at start;
* and the vg field set to NULL until that vgroup is attached,
* and reset back to NULL when that vgroup is detached. 
* Info about all vdatas in the file are loaded into vstab  at start;
* and the vs field set to NULL until that vdata is attached,
* and reset back to NULL when that vdata is detached. 
* -------------------------------------------------------------------- 
*/


PRIVATE vfile_t  vfile [VFILEMAX];
PRIVATE int      nvfile = 0;

/*
  ***Only called by Get_vfile()***   
loads vgtab table with info of all vgroups in file f.
Will allocate a new vfile_t, then proceed to load vg instances.
RETURNS -1 if error
RETURNS INDEX into vfile[] if ok.
*/
PRIVATE int Load_vfile (f)              /*@@*/

DF *f;
{
	DFdesc   d;
	vfile_t	* vf;
	vginstance_t  * v;
	vsinstance_t  * w;
	int returnval;

	if (vjv) {
		sprintf(sjs,"@Load_vfile \n");
		zj;
	}

	/* allocate a new vfile_t structure */
	if (nvfile >= VFILEMAX-1) RTNEG ("Load_vfile: full\n");

	returnval = nvfile;
	vf = &vfile[nvfile];
	nvfile++;

	vf->f	        = f;

	/* load all the vg's  tag/refs from file */
	vf->vgtabn    = -1;
	vf->vgtabtail = &(vf->vgtab);

	vf->vgtab.ref      = -1;
	vf->vgtab.nattach  = -1;
	vf->vgtab.nentries = -1;
	vf->vgtab.vg       = NULL;
	vf->vgtab.next     = NULL;

	vf->vgtabn = 0;
	DFsetfind(f,VGDESCTAG,DFREF_WILDCARD);

	while (1) {
		if ( DFfind(f,&d) != 0) break;  		/* no more vgs */
		if (vjv) {
			sprintf(sjs,"#LOADVGDIR:FOUND (%d) <%d/%d>\n ",
			    vf->vgtabn, d.tag,d.ref);
			zj;
		}

		if (NULL == (v = (vginstance_t*) DFIgetspace(sizeof(vginstance_t))))
			RTNEG ("Load_vfile: no more space\n");

		vf->vgtabtail->next	= v;
		vf->vgtabtail			= v;
		v->next		= NULL;
		v->ref      = d.ref;
		v->vg       = (VGROUP*) NULL; /* ie not attached yet */
		v->nattach  = 0;
		v->nentries = 0;
		vf->vgtabn++;

	}

	/* load all the vs's  tag/refs from file */
	vf->vstabn    = -1;
	vf->vstabtail = &(vf->vstab);

	vf->vstab.ref      = -1;
	vf->vstab.nattach  = -1;
	vf->vstab.nvertices= -1;
	vf->vstab.vs       = NULL;
	vf->vstab.next     = NULL;

	vf->vstabn = 0;
	DFsetfind(f,VSDESCTAG,DFREF_WILDCARD);

	while (1) {
		if ( DFfind(f,&d) != 0) break;      /* no more vgs */
		if (vjv) {
			sprintf(sjs,"#LOADVSDIR:FOUND (%d) <%d/%d>\n ",
			    vf->vstabn, d.tag,d.ref);
			zj;
		}
		if (NULL == (w = (vsinstance_t*) DFIgetspace(sizeof(vsinstance_t))))
			RTNEG ("Load_vfile: no more space\n");

		vf->vstabtail->next  = w;
		vf->vstabtail        = w;
		w->next     = NULL;
		w->ref      = d.ref;
		w->vs       = (VDATA*) NULL; /* ie not attached yet */
		w->nattach  = 0;
		w->nvertices= 0;
		vf->vstabn++;

	}

	/* file may be incompatible with vset version 2.x. Need to check it */
	if ( (0 == vf->vgtabn) && (0 == vf->vstabn) )
		if ( 0 == vicheckcompat (f) ) { /* not compatible */
			sprintf(sjs,"********************************************\n\n");
			zj;
			sprintf(sjs,"%cFile incompatible with vset version 2.0\n",7); 
			zj;
			sprintf(sjs,"%cFirst use the utility vcompat on this file\n",7); 
			zj;
			sprintf(sjs,"********************************************\n\n");
			zj;
			nvfile--; 	/* delete the structure for that file */
			RTNEG ("Load_vfile: incompatible file\n");
		}

	/* otherwise, success */
	return (returnval); /* ok, return the index  */

} /* Load_vfile */

/* returns the vfile pointer for file f, else NULL */
vfile_t * Get_vfile (f)
DF * f;
{
	int i, vfileno;

	/* loop thru vfile[], if found, return ptr */
	for (i=0;i<nvfile;i++) if (vfile[i].f == f)	return(&vfile[i]);

	/* not found, try to load it */
	vfileno = Load_vfile (f);

	if (-1 == vfileno) {
		RTNULL("Get_vfile: cannot load file\n");
	}
	else
		return( &vfile[vfileno]); /* ok, return vfile pointer */

} /* Get_vfile */



PRIVATE void Remove_vfile (f)
DF * f;
{
	int i;

	/* loop thru vfile[], if found, delete it */
		for (i=0;i<nvfile;i++)
			if (vfile[i].f == f) { vfile[i].f = NULL; break; }
			}

/* should be replaced by HDFopen() or such */
/* is called by DFvsetopen() only */

PUBLIC void Vinitialize(f)
DF * f; {
  if (f!=NULL) Remove_vfile (f);
	if (vjv) {
		sprintf(sjs,"@Vinitialize: f=%x\n", f);
		zj;
	}
  }

/* ==================================================================== */

/*
* Looks thru vgtab for vgid and return the addr of the vg instance
* where vgid is found.
* RETURNS NULL if error or not found.
* RETURNS vginstance_t pointer if ok.
*
*/

vginstance_t * vginstance (f,vgid)             /*@@*/
DF  *f;
int vgid;

{
	vginstance_t * v;
	vfile_t      * vf;

	if (NULL== (vf = (vfile_t*) Get_vfile(f)))
		RTNULL ("vginstance: no such file\n");

	v = vf->vgtab.next;
	while (NULL != v) {
		if (v->ref == vgid) return(v);
		v = v->next;
	}
	return(NULL);

} /* vginstance */

/* 
* Tests if a vgroup with id vgid is in the file's vgtab.
* returns -1 if not found,
* returns 1 if found.
*/
int vexistvg (f,vgid)             /*@@*/
DF  *f;
int vgid;
{
	if (NULL== (vginstance_t *) vginstance(f,vgid))
		return(-1);
	else
		return (1);

} /* vexistvg */
/* ==================================================================== */
/*
* vpackvg() and vunpackvg() : Packing and unpacking routines.
* For use in retrieving and storing vgroups to/from the HDF file.
*
*	Fields of VGROUP  that gets stored in HDF as a VGDESCTAG data object:
*		int16		nvelt (no of entries )
*		char		vgname[MAXVGNAMELEN]
*		int16		tag[1..nvelt]		
*		int16		ref[1..nvelt]		
*/
/* ==================================================================== */

#define INT16SIZE 2

/* ==================================================================== */
/* 
*	vpackvg
*	extracts fields from  a VGROUP struct vg and pack the fields
*  into array buf in preparation for storage in the HDF file.
*
*  NO RETURN VALUES.
*/

void vpackvg (vg,buf,size)       /*@@*/

VGROUP			*vg;		/* vgroup to be saved to file */
unsigned char 	buf[]; 	/* buffer to receive the packed fields */
int           	*size;	/* the size of buf is returned here */

{
	int  			i;
	unsigned 	char*b, *bb;
	int16			int16var;

	bb = &buf[0];

	/* save nvelt */
	b= bb;
	INT16WRITE(b,vg->nvelt);
	bb +=INT16SIZE;


	/* save all tags */
	for(i=0;i<vg->nvelt;i++) {
		b= bb;
		INT16WRITE(b,vg->tag[i]);
		bb +=INT16SIZE;
	}

	/* save all refs */
	for(i=0;i<vg->nvelt;i++) {
		b= bb;
		INT16WRITE(b,vg->ref[i]);
		bb +=INT16SIZE;
	}

	/* save the vgnamelen and vgname - omit the null */
	b= bb;
	int16var = strlen(vg->vgname);
	INT16WRITE(b,int16var);
	bb +=INT16SIZE;

	strcpy((char*) bb,vg->vgname);
	bb +=  strlen(vg->vgname) ;

	/* save the vgclasslen and vgclass- omit the null */
	b= bb;
	int16var = strlen(vg->vgclass);
	INT16WRITE(b,int16var);
	bb +=INT16SIZE;

	strcpy((char*) bb,vg->vgclass);
	bb +=  strlen(vg->vgclass) ;

	/* save the expansion tag/ref pair */
	b= bb;
	INT16WRITE(b,vg->extag);   /* the vg's expansion tag */
	bb +=INT16SIZE;

	b= bb;
	INT16WRITE(b,vg->exref);   /* the vg's expansion ref */
	bb +=INT16SIZE;

	/*  save the vg's version field */
	b= bb;
	INT16WRITE(b,vg->version);
	bb +=INT16SIZE;

	/* save the vg's more field */
	b= bb;
	INT16WRITE(b,vg->more);
	bb +=INT16SIZE;

	if (vjv) {
		sprintf(sjs,"vpackvg: vgname is [%s]\n",vg->vgname);
		zj;
	}

	/* returns the size of total fields saved */
	*size = bb - buf + 1;

	if (vjv) {
		sprintf(sjs,"#vpackvg: vg->nvelt=%d\n",vg->nvelt);
		zj;
	}

} /* vpackvg */

/* ==================================================================== */
/*
*	vunpackvg:
*	Unpacks the fields from a buf (ie a VGDESCTAG data object just 
*	read in from the HDF file), into a VGROUP structure vg.
*
* 	Will first zero out vg, unpack fields, then inits as much of 
*  vg as it can.
*
*	NO RETURN VALUES
*
*/

void vunpackvg (vg,buf,size)          /*@@*/

VGROUP*    	  	vg;		/* vgroup to be loaded with file data */
unsigned char 	buf[]; 	/* must contain a VGDESCTAG data object from file */

int*          	size;  	/* ignored, but included to look like vpackvg() */

{

	unsigned char	*b, *bb;
	int 				i;
	int 				int16var;

	i = *size; /* dummy, so that compiler thinks it is used  */

	bb = &buf[0];

	/* retrieve nvelt */
	b = bb;
	INT16READ(b,vg->nvelt);
	bb+=INT16SIZE;

	/* retrieve the tags */
	for (i=0;i<vg->nvelt;i++) {
		b= bb;
		INT16READ(b,vg->tag[i]);
		bb +=INT16SIZE;
	}

	/* retrieve the refs */
	for (i=0;i<vg->nvelt;i++) {
		b= bb;
		INT16READ(b,vg->ref[i]);
		bb +=INT16SIZE;
	}

	/* retrieve vgname (and its len)  */
	b= bb;
	INT16READ(b,int16var);
	bb +=INT16SIZE;

	strncpy(vg->vgname, (char*) bb, int16var);
	vg->vgname[int16var] = '\0';
	bb += int16var;

	if (vjv) {
		sprintf(sjs,"vunpackvg: vgname is [%s]\n",vg->vgname);
		zj;
	}

	/* retrieve vgclass (and its len)  */
	b= bb;
	INT16READ(b,int16var);
	bb +=INT16SIZE;

	strncpy(vg->vgclass, (char*) bb, int16var);
	vg->vgclass[int16var] = '\0';
	bb += int16var;

	b = bb;
	INT16READ(b,vg->extag); /* retrieve the vg's expansion tag */
	bb += INT16SIZE;

	b = bb;
	INT16READ(b,vg->exref); /* retrieve the vg's expansion ref */
	bb += INT16SIZE;

	b = bb;
	INT16READ(b,vg->version); /* retrieve the vg's version field */
	bb += INT16SIZE;

	b = bb;
	INT16READ(b,vg->more); /* retrieve the vg's more field */
	bb += INT16SIZE;


} /* vunpackvg */


/* ------------------------------------------------- */


/* ==================================================================== */

/*
*	 Vattach:
*
*   attaches to an existing vgroup or creates a new vgroup.
*	 returns NULL if  error, else ptr to vgroup.
*
*	IGNORE accesstype. (but save it)  
*  if vgid == -1,
*	  create a NEW vg if vgdir is not full.
*	  Also set nattach =1, nentries=0.
*  if vgid +ve, 
*	  look in vgdir to see if already attached,
*	  if yes, incr nattach 
*	  if not, fetch from file. attach, set nattach=1, netries= val from file 
*
*	In any case, set marked flag to 0.
*/

PUBLIC VGROUP *Vattach (f, vgid, accesstype)     /*@-@*/

int 		vgid;			 	/* actual vgroup's vgid or -1 for new vgroup */
char   	*accesstype; 	/* ignored */
DF			*f;			 	/* HDF file handle */

{
	VGROUP			*vg;
	int 				access,  vgpacksize;
	unsigned char 	vgpack[sizeof(VGROUP)];
	vginstance_t	* v;
	vfile_t			* vf;


	if (f==NULL)
		RTNULL("vattach:bad f");

	if (NULL==(vf = (vfile_t*) Get_vfile(f)))
		RTNULL ("Vattach: no such file\n");

	if      ( accesstype[0]=='R' || accesstype[0]=='r') { 
		access = 'r'; 
	}
	else if ( accesstype[0]=='W' || accesstype[0]=='w') { 
		access = 'w'; 
	}
	else RTNULL("Vattach: bad access type");

	if (vgid == -1) { 			/******* create a NEW vg in vgdir ******/

		if (access=='r') RTNULL("Vattach: don't use r acces with new vg\n");

		/* allocate space for vg, & zero it out */
		if ( (vg = (VGROUP*) DFIgetspace(sizeof(VGROUP)) ) == NULL)
			RTNULL("Vattach: cannot getspace\n");
		zerofill ((unsigned char*) vg,sizeof(VGROUP));

		/* initialize new vg */
		vg->nvelt 		= 0;
		vg->vgname[0]	= '\0';
		vg->f				= f;
		vg->otag 		= VGDESCTAG;
		vg->oref			= vnewref(f);  /* create a new unique ref for it */
		vg->access  	= access;

		vg->marked		= 0;
		vg->vgclass[0]	= '\0';
		vg->extag		= 0;
		vg->exref		= 0;
		vg->more			= 0;
		vg->version		= VSET_VERSION;

		if( vg->oref < 0)
			RTNULL("Vattach: no more refs");

		/* attach new vg to file's vgtab  */
		if ( NULL == (v = (vginstance_t*) DFIgetspace(sizeof(vginstance_t))))
			RTNULL("Vattach: cannot getspace for vgtab \n");

		vf->vgtabtail->next = v;
		vf->vgtabtail		  = v;
		vf->vgtabn++;
		v->next 			     = NULL;
		v->ref	   = vg->oref;
		v->vg      	= vg;
		v->nattach	= 1;
		v->nentries	= 0;

		return(vg);
	}

	else { 							/******* access an EXISTING vg *********/

		if (NULL == (v= (vginstance_t*) vginstance (f,vgid)))
			RTNULL("Vattach: vgid not in vgtab");

		if (v->vg != NULL) {  /* vg already attached */
			v->nattach++;		 /* so, just incr nattach */
			return(v->vg);	    /* & return that vg's ptr */

		}

		/* else vg not attached, must fetch vg from file */

		if (-1 ==  DFgetelement(f,VGDESCTAG,vgid,vgpack))
			RTNULL("Vattach: vg  not in file");

		/* allocate space for vg, & zero it out */

		if (NULL== (vg =(VGROUP*) DFIgetspace(sizeof(VGROUP))) )
			RTNULL("Vattach: cannot getspace\n");
		zerofill((unsigned char*) vg,sizeof(VGROUP));

		/* unpack vgpack into structure vg, and init  */

		vunpackvg(vg,vgpack,&vgpacksize);
		vg->f				= f;
		vg->oref			= vgid;
		vg->otag 		= VGDESCTAG;
		vg->access		= access;
		vg->marked		= 0;

		/* attach vg to file's vgtab at the vg instance v */
		v->vg	    		= vg;
		v->nattach    	= 1;
		v->nentries = vg->nvelt;

		return(vg);
	}


} /* Vattach */

/* ==================================================================== */
/* 
*	Vdetach
*	Detaches access to vg.    
*	NO RETURN VALUES
*
*  if marked flag is 1, write out vg to file.
*	if vg still has velts attached to it, cannot detach vg.
*	decr  nattach. if (nattach is 0), free vg from vg instance.
*	(check that no velts are still attached to vg before freeing)
*
*  if attached with read access, just return.
*
* after detach, set marked flag to 0.	
*
*/

PUBLIC void Vdetach (vg)             /*@-@*/

VGROUP *vg;
{

	int 			 i,vgpacksize;
	unsigned char vgpack[sizeof(VGROUP)];
	vginstance_t * v;

	if (vg==NULL) return;
	if (vg->otag != VGDESCTAG) return;

	/* locate vg's index in vgtab */
	if (NULL ==( v = (vginstance_t*) vginstance (vg->f,vg->oref))) {
		sprintf(sjs,"@Vdetach: vg not found\n");
		zj;
		return;
	}

	/* update vgroup to file if it has write-access */

	/* if its marked flag is 1 */
	/* - OR - */
	/* if that vgroup is empty */
	if (vg->access == 'w') {
		if ((vg->nvelt==0) || (vg->marked == 1)) {
			if (vjv) {
				sprintf(sjs,"@VDETACH: added %d entries in vg..update vg to file\n",
				    vg->nvelt - v->nentries);
				zj;
			}
			vpackvg(vg,vgpack,&vgpacksize);
			if (DFputelement(vg->f,VGDESCTAG,vg->oref,vgpack,vgpacksize) == -1)
				sprintf(sjs,"@Vdetach: putelement error\n"); 
			zj;
			vg->marked = 0;
			return;
		}
	}

	v->nattach--;
	if (vjv) {
		sprintf(sjs,"#Vdetach: nattach is now %d\n", v->nattach);
		zj;
	}

	if (v->nattach > 0)
		return;    /* ok */


	/* else, we can detach and remove vg from file's vgtab  */

	for (i=0;i<vg->nvelt;i++)   	/* check if vg still has attached entries */
		if (vg->velt[i] != NULL) {
			sprintf(sjs,"@Vdetach: vg has vs %d undetached! continuing. \n",i);
			zj;
			break;
		}

	v->vg = NULL;  			/* detach vg from vgdir */

	DFIfreespace(vg);

	return; /* ok */

} /* Vdetach */


/* ==================================================================== */
/*
*	Vinsert
*  inserts a velt (vs or vg)  into a vg 
*	RETURNS entry position within vg (0 or +ve) or -1 on error.
*
*	checks and prevents duplicate links.
*
* Since multiple files are now possible, check that both vg and velt
* are from the same file. else error.
*/

PUBLIC int Vinsert (vg,velt)             /*@-@*/

VGROUP	*vg;
VDATA 	*velt;			/* (VGROUP*) or (VDATA*), doesn't matter */

{
	int i;
	char ss[80];

	if (vg == NULL || velt == NULL) return(FAIL);
	if (vg->otag != VGDESCTAG || vg->nvelt >= MAXNVELT) return(FAIL);

	if ( (vfile_t*) Get_vfile(vg->f) != (vfile_t*) Get_vfile(velt->f) )
		RTNEG("Vinsert: vg and velt not in same file\n");

	/* check in vstab  (kkk!!)  or vgtab that velt actually exist in file */

	switch (velt->otag) {
	case VSDESCTAG:
		if (-1 == vexistvs (vg->f,velt->oref))
			RTNEG("@Vinsert vs doesn't exist\n");
		break;

	case VGDESCTAG:
		if (-1 == vexistvg (vg->f,velt->oref))
			RTNEG("@Vinsert: vg doesn't exist\n");
		break;

	default:
		RTNEG("@Vinsert: unknown element\n");
		break;
	} /* switch */

	/* check and prevent duplicate links */
	for(i=0;i<vg->nvelt;i++)
		if ( (vg->tag[i] == velt->otag) && (vg->ref[i] == velt->oref) ) {
			sprintf(ss,"@Vinsert: duplicate link <%d/%d>\n", 
			    velt->otag,velt->oref);
			RTNEG(ss);
		}

	/* Finally, ok to insert */
	vinsertpair ( vg, velt->otag, velt->oref);

	if (vjv) {
		sprintf(sjs,"#Vinsert:inserted <%d/%d> at nvelt=%d\n",
		    velt->otag, velt->oref, vg->nvelt);
		zj;
	}

	vg->marked = 1;
	return(vg->nvelt - 1);

} /* Vinsert */

/* ------------------------------------------------------------------ */
/*
Checks to see if the given field exists in a vdata belonging to this vgroup.
If found, returns the ref of the vdata.
If not found, or error, returns -1.
28-MAR-91 Jason Ng NCSA
*/

PUBLIC int Vflocate (vg, field)
VGROUP * vg;
char * field;
{
	int 	i, s;
	VDATA *vs;

	for (i=0;i<vg->nvelt;i++)  {
		if (vg->tag[i] != VSDESCTAG) continue;
		vs = (VDATA*) VSattach (vg->f,vg->ref[i],"r"); 
		if (vs==NULL) return (-1);
		s = VSfexist (vs, field);
		VSdetach (vs);
		if (s==1) return (vg->ref[i]); /* found. return vdata's ref */
	}

	return (-1); /* field not found */

} /* Vflocate */

/* ------------------------------------------------------------------ */
/*
* Checks whether the given tag/ref pair already exists in the vgroup.
* RETURNS 1 if exist
* RETURNS 0 if not.
* 28-MAR-91 Jason Ng NCSA
*/
PUBLIC int Vinqtagref ( vg, tag, ref)
VGROUP * vg;
int tag, ref;
{
	int i;

	for (i=0; i<vg->nvelt; i++)
		if ( (tag== vg->tag[i]) && (ref== vg->ref[i]) ) return (1); /* exist */

	return (0); /* does not exist */

} /* Vinqtagref */

/* ------------------------------------------------------------------ */
/*
* Returns the number (0 or +ve integer) of tag/ref pairs in a vgroup.
* If error, returns -1.
* 28-MAR-91 Jason Ng NCSA.
*/

PUBLIC int Vntagrefs (vg)
VGROUP * vg;
{
	return ( (vg->otag==VGDESCTAG) ? vg->nvelt: -1);
} /* Vntagrefs */

/* ------------------------------------------------------------------ */
/*
* Returns n tag/ref pairs from the vgroup into the caller-supplied arrays
* tagrarray and refarray.
* n can be any +ve number, but arrays must be this big.
* RETURNS the total number of (0 or +ve #)  tag/ref pairs returned.
* RETURNS -1 if error.
* 28-MAR-91 Jason Ng NCSA.
*
* NOTE: Do not confuse with Vgettagref().
*
*/

PUBLIC int Vgettagrefs (vg, tagarray, refarray, n)
VGROUP * vg;
int n;
int tagarray[], refarray[];

{
	int i, ntagrefs;

	if ( n <= vg->nvelt) ntagrefs = n;
	else n = vg->nvelt;

	for (i=0; i<ntagrefs; i++) {
		tagarray[i] = vg->tag[i];
		refarray[i] = vg->ref[i];
	}

	return (ntagrefs);
} /* Vgettagrefs */

/* ------------------------------------------------------------------ */
/*
* Returns a specified tag/ref pair from the vgroup.
* User specifies an index. 
* RETURNS 1 if OK.
* RETURNS -1 if error.
* 12-MAY-91 Jason Ng NCSA.
*
* NOTE: Do not confuse with Vgettagrefs().
*
*/

PUBLIC int Vgettagref (vg, which, tag, ref)
VGROUP * vg;
int which;
int *tag, *ref; /* these are returned */

{

	if (vg==NULL) return (-1);
	if (which < 0 || which > vg->nvelt-1) return (-1); /* range err */

		*tag  = vg->tag[which];
		*ref  = vg->ref[which];
	return (1); /* ok */

} /* Vgettagref */
/* ------------------------------------------------------------------ */
/*
* Inserts a tag/ref pair into the attached vgroup vg.
* First checks that the tag/ref is unique.
* If error, returns -1 or tag/ref is not inserted.
* If OK, returns the total number of tag/refs in the vgroup (a +ve integer).
* 28-MAR-91 Jason Ng NCSA.
*/

PUBLIC int Vaddtagref ( vg, tag, ref)
VGROUP * vg;
int tag, ref;
{
	int  n;

	if (Vinqtagref (vg, tag, ref) == 1) return (-1); /* error, already exists */
	n = vinsertpair (vg, tag, ref);
	return (n);
} /* Vaddtagref */

/* ------------------------------------------------------------------ */
/*
* Inserts a tag/ref pair into the attached vgroup vg.
* Does not check for errors. 
* Returns the total number of tag/refs in theat vgroup.
*/

int vinsertpair ( vg, tag, ref)
VGROUP	* vg;
int		tag, ref;
{
	vg->velt[vg->nvelt]  = NULL;
	vg->tag[vg->nvelt]   = tag;
	vg->ref[vg->nvelt]   = ref;
	vg->nvelt ++;

	vg->marked = 1;
	return (vg->nvelt);
}

/* ==================================================================== */
/* 
* 	Ventries
*	returns the no of entries (+ve integer) in the vgroup vgid.
*  vgid must be an actual id
*  RETURNS -1 if error
*
*  undocumented
*
*/

int Ventries (f,vgid)                 /*@@*/

DF* f;
int vgid;
{
	unsigned char vgpack[sizeof(VGROUP)];
	VGROUP vg;
	int    vgpacksize;

	if (vgid < 1) return(FAIL);
	if ( DFgetelement(f,VGDESCTAG ,vgid,vgpack) == -1) {
		sprintf(sjs,"@Ventries: cannot get vg from file\n");
		zj;
		return(FAIL);
	}

	vunpackvg(&vg,vgpack,&vgpacksize);
	return(vg.nvelt);

} /* Ventries */


/* ==================================================================== */
/*
*	Vsetname
* 	gives a name to the VGROUP vg.
*
* NO RETURN VALUES.
*
*	truncates to max length of VGNAMELENMAX 
*/

PUBLIC void Vsetname (vg, vgname)         /*@-@*/

VGROUP	*vg;
char		*vgname;

{
	if (vg == NULL) return;
	if ( strlen(vgname) > VGNAMELENMAX) {
		strncpy(vg->vgname, vgname,VGNAMELENMAX);
		vg->vgname[VGNAMELENMAX]='\0';
	}
	else
		strcpy(vg->vgname, vgname);
	vg->marked = 1;
	return;

} /* Vsetname */

/* ==================================================================== */
/*
*	Vsetclass
* 	assigns a class name to the VGROUP vg.
*
* NO RETURN VALUES.
*
*	truncates to max length of VGNAMELENMAX 
*/

PUBLIC void Vsetclass (vg, vgclass)         /*@-@*/

VGROUP	*vg;
char		*vgclass;

{
	if (vg == NULL) return;
	if ( strlen(vgclass) > VGNAMELENMAX) {
		strncpy(vg->vgclass, vgclass,VGNAMELENMAX);
		vg->vgclass[VGNAMELENMAX]='\0';
	}
	else
		strcpy(vg->vgclass, vgclass);
	vg->marked = 1;
	return;

} /* Vsetclass*/


/* ======================================================= */
/*
* 	Visvg
*	tests if an entry in the vgroup vg is a VGROUP, given the entry's id. 
*
*	RETURNS 1 if so
*	RETURNS 0 if not, or if error
*
*/

PUBLIC int Visvg (vg,id)           /*@-@*/

VGROUP	*vg;
int 		id;		/* valid id of the entry in question */
{
	int 	i;

	for(i=0;i<vg->nvelt;i++)
		if (vg->ref[i] == id   &&  	/* if the ids match, */
		vg->tag[i]==VGDESCTAG) 	/* and it is a vgroup */
			return (1);

	return (0);

} /* Visvg */

/* ======================================================= */
/* 
*	Vgetid
*	
*	Given a vgroup's id, returns the next vgroup's id in the file f .
*	The call Vgetid(f,-1) returns the id of the FIRST vgroup in the file. 
*
*	RETURNS -1 if error
*	RETURNS the next vgroup's id (0 or +ve integer).
*
*	This id is actually the "ref" of the vgroup "tag/ref".
*/

PUBLIC int Vgetid (f,vgid)               /*@-@*/

int 	vgid;					/* current vgid */
DF* 	f;						/* HDF file handle */

{
	vginstance_t * v;
	vfile_t		* vf;

	if ( vgid < -1) return(FAIL);

	if (NULL==(vf = (vfile_t*) Get_vfile(f)))
		RTNEG ("Vgetid: no such file\n");

	if (vjv) {
		sprintf(sjs,"#Vgetid:vgtabn= %d vgid=%d\n",vf->vgtabn,vgid);
		zj;
	}

	if (vgid == -1) {
		if (NULL == vf->vgtab.next)
			return (-1);
		else
			return( (vf->vgtab.next)->ref); /* rets 1st vgroup's ref */
	}

	/* look in vgtab for vgid */
	v = (vf->vgtab).next;
	while(NULL != v) {
		if(v->ref == vgid) break;
		v = v->next;
	}
	if (v==NULL)
		return (FAIL); /* none found */
	else
		if( v->next ==NULL)
			return (FAIL); /* this is the last vg, no more after it */
		else
			return((v->next)->ref); /* success, return the next vg's ref */

} /* Vgetid */


/* ================================================================= */
/*
*	Vgetnext
*
*	Given the id of an entry from a vgroup vg, looks in vg for the next
*	entry after it, and returns its id.
*	The call Vgetnext (vg,-1) returns the id of the FIRST entry in the vgroup.
*
*  Vgetnext will look at only VSET elements in the vgroup.
*  To look at all links in a vgroup, use Vgettagrefs instead.
*
*	RETURNS -1 if error
*	RETURNS the id of the next entry( 0 or +ve integer)  in the vgroup.
*
*	This id is actually the "ref" of the entry's "tag/ref".
*
*/

PUBLIC int Vgetnext (vg,id)           /*@-@*/

VGROUP	*vg;
int 		id;					/* actual id of an entry in the vgroup vg */
{
	int 	i;

	if (id < -1) return(FAIL);
	if (vg == NULL) return(FAIL);
	if (vg->otag != VGDESCTAG) return(FAIL);

	if (vjv) {
		sprintf(sjs,"#Vgetnext:vg->nvelt is %d\n",vg->nvelt);
		zj;
	}
	if (vg->nvelt  == 0) return(FAIL); 			/* nothing in vg */

	if (id == -1) {
		if ((vg->tag[0]==VGDESCTAG) || (vg->tag[0]==VSDESCTAG)) 
				return(vg->ref[0]); 		/* id of first entry */
		}
	
	/* look in vg for id */
	for(i=0;i<vg->nvelt;i++) 
		if ((vg->tag[i]==VGDESCTAG) || (vg->tag[i]==VSDESCTAG)) {
			if(vg->ref[i] == id) {
				if (i == (vg->nvelt - 1) )
					return(FAIL);
				else  {
					if ((vg->tag[i+1]==VGDESCTAG) ||
						 (vg->tag[i+1]==VSDESCTAG)) 
						return(vg->ref[i+1]);		/* return the id of next entry */
					else  return (-1); 
					}
			}
		}

	return (FAIL);

} /* Vgetnext  */

/* ================================================================= */
/*
*	Vgetname
*	returns the vgroup's name
*
*/

PUBLIC void Vgetname (vg, vgname)          /*@-@*/

VGROUP	*vg;
char		*vgname;				/* its name is returned in this var */
{
	if(vg != NULL) strcpy(vgname, vg->vgname);
	return;

} /* Vgetname */
/* ================================================================= */
/*
*	Vgetclass
*	returns the vgroup's class name 
*
*/

PUBLIC void Vgetclass (vg, vgclass)          /*@-@*/

VGROUP	*vg;
char		*vgclass;				/* its class name is returned in this var */
{
	if(vg != NULL) strcpy(vgclass, vg->vgclass);
	return;

} /* Vgetclass*/

/* ================================================================= */
/*
*	Vinquire
*
*	General inquiry routine for VGROUP. 
*
*	output parameters:
*			nentries - no of entries in the vgroup
*			vgname	- the vgroup's name
*
*	RETURNS -1 if error
*	RETURNS 1 if ok
*
*/

PUBLIC int Vinquire (vg, nentries, vgname)     /*@-@*/

VGROUP	*vg;
char		*vgname;
int 		*nentries;
{
	if (vg == NULL) return(FAIL);
	if (vg->otag != VGDESCTAG) return(FAIL);

	strcpy(vgname, vg->vgname);
	*nentries = vg->nvelt;

	return(1); /* ok */

} /* Vinquire */

/* ================================================================= */

