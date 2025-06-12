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
* Likkai Ng May 1991  NCSA 
*
* vsetf.c
* Part of the HDF VSet interface.
*
************************************************************************/

/*
#include <ctype.h> 
*/

#include "vg.h"
/* 
** ==================================================================
** PRIVATE data areas and routines
** ==================================================================
**/

/*
stores sizes of local machine's known types 
*/

PRIVATE int local_sizetab[] = 
{
	    LOCAL_UNTYPEDSIZE,
	    LOCAL_CHARSIZE,
	    LOCAL_INTSIZE,
	    LOCAL_FLOATSIZE,
	    LOCAL_LONGSIZE,
};

PRIVATE SYMDEF rstab[] = 
{
	    "PX",LOCAL_FLOATTYPE,	IEEE_FLOATSIZE, 1,
	    "PY",LOCAL_FLOATTYPE,	IEEE_FLOATSIZE, 1,
	    "PZ",LOCAL_FLOATTYPE,	IEEE_FLOATSIZE, 1,

	    "IX",LOCAL_INTTYPE,		IEEE_INT16SIZE, 1,
	    "IY",LOCAL_INTTYPE,		IEEE_INT16SIZE, 1,
	    "IZ",LOCAL_INTTYPE,		IEEE_INT16SIZE, 1,

	    "NX",LOCAL_FLOATTYPE,	IEEE_FLOATSIZE, 1,
	    "NY",LOCAL_FLOATTYPE,	IEEE_FLOATSIZE, 1,
	    "NZ",LOCAL_FLOATTYPE,	IEEE_FLOATSIZE, 1,

};

#define NRESERVED ( sizeof(rstab)/sizeof(SYMDEF) )
#define SIZETAB_SIZE sizeof(local_sizetab)/sizeof(int)

/*
** returns the machine size of a field type
*/

int SIZEOF (x)

int x;
{

	if (x<0 || x>SIZETAB_SIZE-1) {
		return(FAIL);
		}
	else {
		return(local_sizetab[x]);
		}
} /* SIZEOF */


/* ------------------------------------------------------------------ */
/*
** sets the fields in a vdata for reading or writing
** RETURNS -1 if error, and 1 if ok.
** truncates each field to max length of  FIELDNAMELENMAX.
*/

PUBLIC int VSsetfields (vs,fields)        /*@-@*/

VDATA *vs;
char	*fields;
{
	char  **av;
	int 	ac, i,j,found, order;
	VREADLIST 	rlist;
	VWRITELIST  wlist;

	if (vs==NULL) return(FAIL);
	if (scanattrs(fields,&ac,&av) < 0) {
		sprintf(sjs,"@bad fields string [%s]\n",fields);
		zj;
		return(FAIL); /* bad fields string */
	}
	if (ac==0) return(FAIL);

	if (vs->access == 'w') { /* ===== W R I T E  access ============== */
		if ( vs->nvertices > 0) {
			sprintf(sjs,"@\t*cannot set fields - data already exists\n");
			zj;
			return(FAIL);
		}

		wlist        = vs->wlist;
		wlist.ivsize = 0;
		wlist.n      = 0;
		for(i=0;i<ac;i++) {
			/* --- first look in the reserved symbol table --- */
			for(found=0,j=0;j<NRESERVED;j++)
				if (!strcmp(av[i], rstab[j].name)) {
					found = 1;

					strcpy( wlist.name[wlist.n],rstab[j].name);
					order = rstab[j].order;
					wlist.type[wlist.n]  =  rstab[j].type;
					wlist.order[wlist.n] =  order;
					wlist.esize[wlist.n] =  order * SIZEOF(rstab[j].type);
					wlist.isize[wlist.n] =  order * rstab[j].isize;
					wlist.ivsize  += wlist.isize[wlist.n];
					wlist.n++;
					break;
				}

			/* --- now look in the user's symbol table --- */
			if(!found) {
				for(found=0,j=0;j<vs->nusym;j++)
					if (!strcmp(av[i], vs->usym[j].name)) {
						found = 1;

						strcpy (wlist.name[wlist.n],vs->usym[j].name);
						order = vs->usym[j].order;
						wlist.type[wlist.n]  =  vs->usym[j].type;
						wlist.order[wlist.n] = order;
						wlist.esize[wlist.n] = order * SIZEOF(vs->usym[j].type);
						wlist.isize[wlist.n] = order *  vs->usym[j].isize;
						wlist.ivsize+= wlist.isize[wlist.n];
						wlist.n++;
						break;
					}
			}
			if (!found) {    /* field is not a defined field - error  */
				sprintf(sjs,"@Vsetfield:field [%s] unknown\n",av[i]);
				zj;
				return(FAIL);
			}
		}

		/* *********************************************************** */
		/* ensure fields with order>1 are alone  */
		for (j=0,i=0;i<wlist.n;i++)
			if (wlist.order[i] >1 && wlist.n != 1) {
				sprintf(sjs,"@Vsetf: [%s] in [%s] has order %d. error.\n",
				    wlist.name[i], fields, wlist.order[i]);
				zj;
				return(FAIL);
			}
		/* *********************************************************** */

		/* compute and save the fields' offsets */
		for (j=0,i=0;i<wlist.n;i++) {
			wlist.off[i] =j;
			j += wlist.isize[i];
		}

		/* copy from wlist (temp) into vdata */
		movebytes((unsigned char*) &(wlist), (unsigned char*) &(vs->wlist),sizeof(wlist));

		return(1); /* ok */

	} /* "w" */

	else if (vs->access == 'r') {  /* ===== R E A D  access ============== */

		rlist   = vs->rlist;
		rlist.n = 0;
		for (i=0;i<ac;i++) {
			for (found=0,j=0;j<vs->wlist.n;j++)
				if (!strcmp(av[i], vs->wlist.name[j]) ) {/*  see if field exist */
					found = 1;

					rlist.item[rlist.n] = j; /* save as index into wlist->name */
					rlist.n++;
					break;
				}
			if (!found)  {    /* field does not exist - error */
				sprintf(sjs,"@Vsetfield: field [%s] does not exist in vs\n",
				    av[i]);
				zj;
				return(FAIL);
			}
		}

		/* copy from rlist (temp) into vdata */
		movebytes((unsigned char*) &(rlist), (unsigned char*) &(vs->rlist),sizeof(rlist));

		return(1); /* ok */

	} /* "r" */

} /* VSsetfields */

/* ------------------------------------------------------------------ */
/* 
** defines a (one) new field within the vdata 
*/

PUBLIC int VSfdefine (vs, field, localtype, order)    /*@-@*/

VDATA *vs;
char  *field;
int   localtype, order;
{
	char **av;
	int  ac;
	char *ss;
	int j, usymid, replacesym;

	if (vs==NULL) return(FAIL);

	if (scanattrs(field,&ac,&av) < 0) {
		sprintf(sjs,"@bad field string [%s] in Vfdefine\n",field);
		zj;
		return(FAIL);
	}
	if (ac!=1) return(FAIL); /* only one field item can be defined per call */

	if (order <1 || order > 1000) {
		sprintf(sjs,"@VSfdefine: error order %d\n",order);
		zj;
		return(FAIL);
	}

	/* 
	** check for any duplicates 
	*/
	/* --- first look in the reserved symbol table --- */
	for (j=0;j<NRESERVED;j++)
		if (!strcmp(av[0], rstab[j].name)) {
			sprintf(sjs,"@VSfdefine warning: predefined field [%s] redefined.\n",
			av[0]); zj;
			break;
		   }
	/* --- then look in the user's symbol table --- */
	for (replacesym = 0,j=0;j<vs->nusym;j++)
		if (!strcmp(av[0], vs->usym[j].name)) {
			sprintf(sjs,"@VSfdefine warning: field [%s] redefined.\n",av[0]); zj;
			replacesym = 1;
			break;
		   }

   if (replacesym)  
		usymid = j; /* new definition will replace old at this index */
   else 
		usymid = vs->nusym;

	switch (localtype) {

	case LOCAL_FLOATTYPE:
		vs->usym[usymid].isize  =	IEEE_FLOATSIZE;
		break;
	case LOCAL_INTTYPE:
		vs->usym[usymid].isize  =	IEEE_INT16SIZE;
		break;
	case LOCAL_CHARTYPE:
		vs->usym[usymid].isize  =	IEEE_CHARSIZE;
		break;
	case LOCAL_LONGTYPE:
		vs->usym[usymid].isize  =	IEEE_INT32SIZE;
		printf("@Vfdefine: [%s] defined as long \n",av[0]);
		break;
	default:
		sprintf(sjs,"@Vfdefine: bad type (=%d) for [%s]\n",localtype,av[0]);
		zj;
		return(FAIL);
		break;

	} /* switch */

	j  = strlen(av[0])+1;

	if( (ss = (char*) DFIgetspace(j))==NULL)
		return(FAIL);  						/* cannot alloc space */

		
	strcpy(ss,av[0]);
	vs->usym[usymid].name  = ss;
	vs->usym[usymid].type  = localtype;
	vs->usym[usymid].order = order;

   /* prevent user-symbol table overflow */
	if (vs->nusym >=USYMMAX) {
		sprintf(sjs,"@VSFDEFINE: %d too many symbols\n",vs->nusym); zj;
		for(j=0;j<vs->nusym;j++)
			{ sprintf(sjs,"@ sym: %d [%s]\n",j,vs->usym[j]); zj; }
		return(FAIL);
	}

	/* increment vs->nusym only if no user field has been redefined */
	if (!replacesym) vs->nusym++;

   return(1); /* ok */

} /* VSfdefine */

/* ------------------------------------------------------------------ */

