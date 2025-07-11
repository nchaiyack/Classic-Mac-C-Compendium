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
* vg.c
* Part of the HDF VSet interface
* This file contains routine to handle VDATAs.
*
* Most routines return -1 (FAIL) on error. 
* VSattach returns NULL on error.
*
* PRIVATE functions manipulate vsdir and are used only within this file.
* PRIVATE data structures in here pertain to vdata in vsdir only.
*
**************************************************************************/

#include <ctype.h> 
#include "vg.h"

PRIVATE int newref_count = -1; /* used by Vnewref only */

/* ------------------------------------------------------------------
*	Vnewref
*	utility routine. returns a unique reference number.
*
*	NOTE: should be modified to call DFnewref!!!
*  currently looks once in vsdir and vgdir, and find the highest ref.
*	also, the parameter f isn't used right now.
*
*	RETURNS a unique ref (+ve integer) ,
*	RETURNS -1 if error
*
*  undocumented
*
*/

int vnewref (f)			/*@@*/

DF  *f;
{
#if 0
	int t;


	if (f==NULL) return(FAIL);
	if (newref_count == -1) {
		for(t=1;t<65300;t++) {
			if      ( -1 != vexistvg(f,t))   continue;
			else if ( -1 != vexistvs(f,t))   continue;
			else break;
		}
		newref_count = t;
	}

	newref_count++;
	if (newref_count > 65300) return(FAIL);

	return( newref_count - 1 );
#endif

#if 1
	return( DFnewref(f) );
#endif

} /* vnewref */


/* ------------------------------------------------------------------
* VSelts
* returns the number of elements in the VDATA vs 
* returns -1  on error.
*
* undocumented
* 28-MAR-91 Jason NG NCSA
*
*/

PUBLIC int32 VSelts (vs)                       /*@-@*/

VDATA *vs;
{
	return( (vs->otag==VSDESCTAG) ?  vs->nvertices : -1);

} /* VSelts */



/* ------------------------------------------------------------------
*	VSgetinterlace 
*  returns the interlace (in the file) of the vdata vs.
*  returns -1 on error.
*
*  undocumented
*
*/

PUBLIC int VSgetinterlace (vs)              /*@-@*/

VDATA *vs;
{
	return( (vs==NULL) ? FAIL : vs->interlace );

} /* VSgetinterlace */



/* ------------------------------------------------------------------
*	VSsetinterlace 
* 	sets the vdata's interlace to full or none.
*	returns -1 on error.
*/

PUBLIC int VSsetinterlace (vs, interlace)      /*@-@*/

VDATA *vs;
int interlace;
{
	if (vs==NULL)
		return(FAIL);
	if ( vs->access == 'r' ||			/* only for write-only files */
	vs->nvertices > 0) 			/*may not change interlace if data exists*/
		return(FAIL);

	/* currently only 2 kinds allowed */

	if ( interlace==FULL_INTERLACE || 
	    interlace==NO_INTERLACE ) {
		vs->interlace = interlace;
		return(1); /* ok */
	}
	else 
		return(FAIL);  	  	    			/* bad interlace type */

} /* VSsetinterlace */


/* ------------------------------------------------------------------
*	VSgetfields 
*  returns the fieldnames in a vdata.
*  RETURNS  -1 on error, else the no of fields in the vdata.
*
*	undocumented
*
*/

PUBLIC int VSgetfields (vs,fields)		/*@-@*/

VDATA *vs;
char  *fields;							/* fieldnames are returned in this var */
{
	int i;
	if (vs==NULL) return(FAIL);

	fields[0] = '\0';
	for (i=0;i<vs->wlist.n;i++) { /* build the comma-separated string */
		strcat(fields,vs->wlist.name[i]);
		if ( i < vs->wlist.n - 1 )
			strcat(fields,",");
	}

	return(vs->wlist.n);

} /* VSgetfields */

/* ------------------------------------------------------------------
*	VSfexist
* 	tests for existence of 1 or more fields in a vdata.
*
*	RETURNS -1 if flase, or error
*	RETURNS 1 if true
*/

PUBLIC int VSfexist (vs, fields)          /*@-@*/

VDATA *vs;
char	*fields;
{
	char   		**av, *s;
	int			ac,i,j,found;
	VWRITELIST	*w;

	if (vs==NULL) return(FAIL);

	/* call scanattrs to parse the string */

	if (scanattrs(fields,&ac,&av) < 0) return(FAIL); /* bad attrs string */
	if (ac<1) return(FAIL); 							    /* no attrs */

	/* now check in vs's field table */

	w = &vs->wlist;
	for (i=0;i<ac;i++) {
		for (found=0,s=av[i],j=0;j<w->n;j++) {
			if ( matchnocase(s,w->name[j]) ) {
				found=1;
				break;
			}
		}
		if (!found) return(FAIL);
	}

	return(1);

} /* VSfexist */


/* ================================================================== */
/*
*	VSsizeof - computes the byte size of the field(s) of a vdata.
*	         - Note that the size is the actual size for the local machine.
*
*				- RETURNS -1 on error, else the field(s) size (+ve integer).
*/

PUBLIC int VSsizeof (vs, fields)           /*@-@*/

VDATA *vs;
char  *fields;
{
	int 	totalsize, ac, i,j,found;
	char   **av;

	if (vs==NULL) 						     return(FAIL);

	if (scanattrs(fields,&ac,&av) < 0) return(FAIL); 	/* bad attrs string */
	if (ac<1) 		   					  return(FAIL); 	/* no attrs */

	if (vjv) { 
		sprintf(sjs,"#VSsizeof: fields are [%s]\n",fields);
		zj; 
	}

	totalsize=0;
	for (i=0;i<ac;i++) {
		for (found=0,j=0;j<vs->wlist.n;j++)
			if (!strcmp(av[i], vs->wlist.name[j])) {  /* check fields in vs */
				totalsize += vs->wlist.esize[j];
				found=1;
				break;
			}

		if (!found) {
			sprintf(sjs,"@VSsizeof:[%s] not in vs\n",av[i]);
			zj;
			return(FAIL);
		}
	}

	return(totalsize);

} /* VSsizeof */

/* ================================================================== */

/* matchnocase -  (PRIVATE) compares 2 strings, ignoring case 
*	               returns 1 if match, else 0
*/

PRIVATE int matchnocase (strx,stry)     /*@@*/

char *strx,*stry;
{
	int 	i,nx,ny,tx,ty;
	char 	*sx, *sy;

	nx = strlen(strx);
	ny = strlen(stry);
	if ( nx != ny) return(0);  /* different lengths */

	for (sx=strx, sy=stry, i=0;i<nx;i++,sx++,sy++) {
		tx= *sx; 
		ty= *sy;
		if (islower(tx)) tx=toupper(tx);
		if (islower(ty)) ty=toupper(ty);
		if (tx != ty) 	  return(0);
	}

	return(1);

} /* matchnocase */


/* ================================================================== */

/*
*	VSdump - prints contents of a vdata (for debugging) 
*				no return codes.
*/

void VSdump (vs)                           /*@@*/

VDATA *vs;
{
	VWRITELIST	*w;
	int 			i;

	if (vs==NULL) {
		sprintf(sjs,"@VSdump: vs is null\n");
		zj;
		return;
	}

	sprintf(sjs,"@tag=%d ref=%d i=%d ",vs->otag, vs->oref,vs->interlace);
	zj;
	sprintf(sjs,"@nv=%d\n ",vs->nvertices);
	zj;

	w = (VWRITELIST*) &vs->wlist;
	sprintf(sjs,"@vsize(HDF)=%d fields=%d [%s]\n",w->ivsize,w->n,vs->vsname);
	zj;

	for (i=0;i<w->n;i++)
	{ 
		sprintf(sjs,"@<%s>      type:%d esize=%d isize=%d off=%d\n",
		    w->name[i], w->type[i], w->esize[i],w->isize[i],w->off[i]);
		zj;
	}

	sprintf(sjs,"\n");
	zj;

} /* VSdump */


/* ======================================================= */
/*
*	VSsetname - give a name to a vdata.
*	          - NO RETURN VALUES
*	          - truncates to max length of VSNAMELENMAX
*      
*/

PUBLIC void VSsetname (vs, vsname)           /*@-@*/

VDATA *vs;
char	*vsname;

{
	if (vs == NULL) return;
	if ( strlen(vsname) > VSNAMELENMAX) {
		strncpy(vs->vsname, vsname,VSNAMELENMAX);
		vs->vsname[VSNAMELENMAX]='\0';
	}
	else 
		strcpy(vs->vsname, vsname);
	vs->marked = 1;
	return;

} /* VSsetname */

/* ======================================================= */
/*
*	VSsetclass- assigns a class name to a vdata.
*	          - NO RETURN VALUES
*	          - truncates to max length of VSNAMELENMAX
*      
*/

PUBLIC void VSsetclass (vs, vsclass)           /*@-@*/

VDATA *vs;
char	*vsclass;

{
	if (vs == NULL) return;
	if ( strlen(vsclass) > VSNAMELENMAX) {
		strncpy(vs->vsclass, vsclass,VSNAMELENMAX);
		vs->vsclass[VSNAMELENMAX]='\0';
	}
	else 
		strcpy(vs->vsclass, vsclass);
	vs->marked = 1;
	return;

} /* VSsetclass*/

/* ======================================================= */

/*
*	VSgetname - gets the vdata's name.
*				 - NO RETURN VALUES
*/

PUBLIC void VSgetname (vs, vsname)           /*@-@*/

VDATA *vs;
char	*vsname;
{
	if (vs != NULL) strcpy(vsname, vs->vsname);
	return;

} /* VSgetname */

/* ======================================================= */

/*
*	VSgetclass - gets the vdata's class name.
*				 - NO RETURN VALUES
*/

PUBLIC void VSgetclass (vs, vsclass)           /*@-@*/

VDATA *vs;
char	*vsclass;
{
	if (vs != NULL) strcpy(vsclass, vs->vsclass);
	return;

} /* VSgetclass */


/* ================================================================== */
/*
*	VSinquire - gets info about a vdata vs:
*
*				  nvertices: 	no of vertices in it.
*				  interlace: 	its interlcae
*				  fields : 		a comma separated string listing the field(s). 
*									(eg "PX,PY")
*				  eltsize: 	   size of element (all field(s)) on local machine.
*				  vsname: 		vdata's name, if any.
*
*	RETURNS -1 if error
*	RETURNS 1 if ok
*
*/


PUBLIC int VSinquire (vs, nelt, interlace, fields, eltsize, vsname)   /*@-@*/

VDATA 	*vs;
char  	*fields, *vsname;
int 		*nelt, *interlace, *eltsize;

{
	if (vs==NULL) return(FAIL);
	VSgetfields (vs,fields);

	*nelt       = vs->nvertices;
	*interlace  = vs->interlace;
	*eltsize    =  VSsizeof (vs,fields);
	strcpy(vsname,vs->vsname);

	return(1); /* ok */


} /* VSinquire */

/* ================================================================== */
/*
* VSlone - returns an array of refs of all lone vdatas in the file.
* 	      - returns -1 if error
*	      - otherwise returns the total number of lone vdatas in the file 
*
*			If idarray is too small, routine will only fill idarray with up
*			 to asize worth of refs.
*
*			INPUT idarray: user supplies  an int array.
*		   INPUT asize: integer specifying how many ints in idarray[];
*			INPUT f: HDF file pointer.
*
*/

PUBLIC int VSlone(f, idarray, asize) 

DF * f;
int idarray[];			 /* array to contain the refs */
int asize;            /* input: size of idarray */
{
int 	 * lonevdata; /* local working area: stores flags of vdatas */
int 		i,vgid, vsid, vstag;
VGROUP 	* vg;
int 		dum;
int 		nlone; /* total number of lone vdatas */


/* -- allocate space for vdata refs, init to zeroes -- */
if (NULL == (lonevdata = (int*) DFIgetspace( 65000 * sizeof(int)))) 
   RTNEG ("VSlone: space too low. VSlone not executed\n");
for(i=0;i<65000;i++) lonevdata[i] = 0;

/* -- look for all vdatas in the file, and flag (1) each -- */
vsid = -1;
dum=0;
while(1) {
	if ( -1 == (vsid = VSgetid (f, vsid))) break; /* no more vdatas */
	lonevdata[vsid ] = 1;
	}

/* -- Look through all vgs, searching for vdatas -- */
/* -- increment its index in lonevdata if found -- */
dum=0;
vgid = -1;
while(1) {
	if ( -1 == (vgid = Vgetid (f, vgid))) break; /* no more vgroups */
	vg = (VGROUP*) Vattach(f,vgid,"r");
	for (i=0; i< Vntagrefs(vg); i++) {
		Vgettagref (vg, i, &vstag, &vsid);
		if (vstag==VSDESCTAG) 
			{ lonevdata[vsid]++; dum++;  }
		}
	Vdetach(vg);
	}

/* -- check in lonevdata: it's a lone vdata if its flag is still 1 -- */
nlone = 0;
for(i=0;i<65000;i++) {
	if (1 == lonevdata[i]) {
	 	if (nlone < asize) { /* insert into idarray up till asize */
			idarray[nlone] = i;
			}
		nlone ++;
		}
   }
DFIfreespace(lonevdata);

return(nlone); /* return the TOTAL # of lone vdatas */

} /* VSlone */

/* ================================================================== */
/*
* Vlone  - returns an array of refs of all lone vgroups in the file.
* 	      - returns -1 if error
*	      - otherwise returns the total number of lone vgroups in the file 
*
*			If idarray is too small, routine will only fill idarray with up
*			 to asize worth of refs.
*
*			INPUT idarray: user supplies  an int array.
*		   INPUT asize: integer specifying how many ints in idarray[];
*			INPUT f: HDF file pointer.
*
*/

PUBLIC int Vlone(f, idarray, asize) 

DF * f;
int idarray[];			 /* array to contain the refs */
int asize;            /* input: size of idarray */
{
int 		* lonevg; /* local working area: stores flags of vgroups */
int		i;
int 		vgid, vstag, id;
VGROUP 	* vg;
int 		dum;
int 		nlone; /* total number of lone vgroups */


/* -- allocate space for vgroup refs, init to zeroes -- */
if (NULL == (lonevg = (int*) DFIgetspace( 65000 * sizeof(int)))) 
   RTNEG ("Vlone: space too low. Vlone not executed\n");
for(i=0;i<65000;i++) lonevg[i] = 0;

/* -- look for all vgroups in the file, and flag (1) each -- */
id = -1;
dum=0;
while(1) {
	if ( -1 == (id = Vgetid (f, id))) break; /* no more vgroups */
	lonevg[ id ] = 1;
	}

/* -- Look through all vgs, searching for vgroups -- */
/* -- increment its index in lonevg if found -- */
dum=0;
vgid = -1;
while(1) {
	if ( -1 == (vgid = Vgetid (f, vgid))) break; /* no more vgroups */
   printf("Vlone: vgid=%d..attach",vgid);
	vg = (VGROUP*) Vattach(f,vgid,"r");
   printf("..attach done\n");
	id = -1;
	for (i=0; i< Vntagrefs(vg); i++) {
		Vgettagref (vg, i, &vstag, &id);
		if (vstag==VGDESCTAG) { lonevg[id]++; dum++;  }
		}
	Vdetach(vg);
	}

/* -- check in lonevg: it's a lone vgroup if its flag is still 1 -- */
nlone = 0;
for(i=0;i<65000;i++) {
	if (1 == lonevg[i]) {
	 	if (nlone < asize) { /* insert into idarray up till asize */
			idarray[nlone] = i;
			}
		nlone ++;
		}
   }
DFIfreespace(lonevg);

return(nlone); /* return the TOTAL # of lone vgroups */

} /* Vlone */


/* ================================================================== */
/* new jan 3 1991 */
/* looks in the file and returns the ref of the vgroup with name vgname */
/* 
* returns -1 if not found, or error.
* otherwise, returns the vgroup's ref (a +ve integer).
*/

int Vfind (f, vgname)
DF * f;
char * vgname;
{
  int vgid = -1;
  VGROUP* vg;
  char name[512];

	while ( -1 != (vgid=Vgetid(f, vgid)) ) {
		vg = (VGROUP*) Vattach(f,vgid,"r");
		if (vg==NULL) return(-1); 			/* error */
		Vgetname(vg, name);
		Vdetach (vg);
		if (!strcmp(vgname,name)) return (vg->oref);  /* found the vgroup */
  	}
  	return(-1); /* not found */

} /* Vfind */

/* ================================================================== */
/* new jan 3 1991 */
/* looks in the file and returns the ref of the vdata with name vsname */
/* 
* returns -1 if not found, or error.
* otherwise, returns the vdata's ref (a +ve integer).
*/

int VSfind (f, vsname)
DF * f;
char * vsname;
{
  int vsid = -1;
  VDATA * vs;
  char name[512];

	while ( -1 != (vsid=VSgetid(f, vsid)) ) {
		vs = (VDATA*) VSattach(f,vsid,"r");
		if (vs==NULL) return(-1); 			/* error */
		VSgetname(vs, name);
		VSdetach (vs);
		if (!strcmp(vsname, name)) return (vs->oref);  /* found the vdata */
  	}
  	return(-1); /* not found */

} /* VSfind */

/* ================================================================== */

/*
* Vsetzap: Useless now. Maintained for back compatibility.
*/

Vsetzap() {
	if (vjv) { sprintf(sjs,"Vsetzap: defunct\n"); zj; }
}
/* ================================================================== */
