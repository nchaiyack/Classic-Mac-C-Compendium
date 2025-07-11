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
* Likkai Ng May 1991 NCSA
*
* vgF.c
* Part of the HDF VSet interface.
*
* C routines (short names) to be called from fortran 
*
*
************************************************************************/


#include "vg.h"

/* 
remove trailing blanks from a string. input argument is a  string
and *MUST* be a variable and not a constant!! For internal use only!! 
Used only on Crays where the Fortran compiler will pad strings to the 
nearest 8-byte boundary.
*/

trimendblanks(ss) char *ss;
{
#ifdef UNICOS

	int i,n;
	n = strlen(ss);
	for(i=n-1;i>=0;i--) {
		if(ss[i]!=' ') {
			ss[i+1]='\0';
			break;
		}
	}
#endif
}

/* ================================================== */
/*  VGROUP routines                                   */
/* ================================================== */

/* 
attach from a vgroup
related: Vattach--vatchc--VFATCH
*/


#ifdef MAC
pascal
#endif 

#ifdef DF_CAPFNAMES
int * VATCHC (f, vgid, accesstype)

#else 
int * vatchc_ (f, vgid, accesstype)
#endif

DF		**f;
int	*vgid;
_fcd 	accesstype;						/* one character only */
{
	VGROUP *vg;
	char	 *acc;

	acc = (char *) DFIf2cstring (accesstype,1); /* ie 'r' or 'w' only */

	vg = (VGROUP*) Vattach(*f, *vgid, acc);
	(void) DFIfreespace(acc);

	if(vg==NULL)
		return((int*) -1);
	else 
		return( (int*) vg);
}

/* ------------------------------------------------------------------ */

/* 
detach from a vgroup
related: Vdetach--vdtchc--VFDTCH
*/

#ifdef MAC
pascal
#endif

#ifdef DF_CAPFNAMES
void VDTCHC (vg)

#else
void  vdtchc_ (vg)
#endif

VGROUP **vg;
{
	Vdetach(*vg);
}
/* ------------------------------------------------------------------ */

/* 
get the name of a vgroup
related: Vgetname--vgnamc--VFGNAM
*/

#ifdef MAC
pascal
#endif

#ifdef DF_CAPFNAMES
void VGNAMC (vg, vgname)

#else
void vgnamc_ (vg, vgname)
#endif

VGROUP **vg;
_fcd 	vgname;

{
	char  *name;
	name = _fcdtocp(vgname);
	Vgetname (*vg, name);
}	/* VGNAMC */

/* ------------------------------------------------------------------ */
/* 
get the class name of a vgroup
related: Vgetclass--vgclsc--VFGCLS
*/

#ifdef MAC
	pascal
#endif

#ifdef DF_CAPFNAMES
	    void VGCLSC (vg, vgclass)

#else
	    void vgclsc_ (vg, vgclass)
#endif

	    VGROUP **vg;
	_fcd 	vgclass;

	{
		char  *class;
		class = _fcdtocp(vgclass);
		Vgetclass(*vg, class);
	}	/* VGCLSC */

/* ------------------------------------------------------------------ */
/* 
general inquiry on a vgroup 
related: Vinquire--vinqc--VFINQ
*/

#ifdef MAC
pascal
#endif

#ifdef DF_CAPFNAMES
int VINQC (vg, nentries, vgname)

#else
int vinqc_ (vg, nentries, vgname)
#endif

VGROUP **vg;
int	 *nentries;
_fcd   vgname;

{
	char  *name;
	name =  _fcdtocp(vgname);
	return( Vinquire(*vg, nentries, name) );
} /* VINQC */


/* ------------------------------------------------------------------ */
/* 
gets the id of the next vgroup in the file
related: Vgetid--vgidc--VFGID
*/

#ifdef MAC
pascal
#endif

#ifdef DF_CAPFNAMES
int VGIDC (f,vgid)

#else
int vgidc_ (f,vgid)
#endif

DF			**f;
int 		*vgid;
{
	return( Vgetid (*f, *vgid) );
}

/* ------------------------------------------------------------------ */
/* 
gets the id of the next entry in the vgroup
related: Vgetnext--vgnxtc--VFGNXT
*/

#ifdef MAC
pascal
#endif

#ifdef DF_CAPFNAMES
int VGNXTC (vg,id)

#else
int vgnxtc_ (vg,id)
#endif

VGROUP	**vg;
int		*id;
{
	return( Vgetnext(*vg, *id) );
}

/* ------------------------------------------------------------------ */
/* 
sets the name of the vgroup
related: Vsetname--vsnamc--VFSNAM
*/

#ifdef MAC
pascal
#endif

#ifdef DF_CAPFNAMES
void VSNAMC (vg, vgname)

#else
void vsnamc_ (vg, vgname)
#endif

VGROUP	**vg;
_fcd		vgname;

{
	char *name;
	name = (char*) DFIf2cstring (vgname,VSNAMELENMAX);
	trimendblanks(name);
	Vsetname (*vg, name);
	(void) DFIfreespace(name);
}
/* ------------------------------------------------------------------ */
/* 
sets the class name of the vgroup
related: Vsetclass--vsclsc--VFSCLS

*/

#ifdef MAC
	pascal
#endif

#ifdef DF_CAPFNAMES
	    void VSCLSC (vg, vgclass)

#else
	    void vsclsc_ (vg, vgclass)
#endif

	    VGROUP	**vg;
	_fcd		vgclass;

	{
		char *class;
		class = (char*) DFIf2cstring (vgclass,VGNAMELENMAX);
		trimendblanks(class);
		Vsetclass (*vg, class);
		(void) DFIfreespace(class);
	}

/* ------------------------------------------------------------------ */
/* 
inserts a vset entity (ie vgroup or vdata) into the given vgroup
related: Vinsert--vinsrtc--VFINSRT
*/

#ifdef MAC
pascal
#endif

#ifdef DF_CAPFNAMES
int VINSRTC (vg,ventity)

#else
int vinsrtc_ (vg,ventity)
#endif

VGROUP	**vg;
int		**ventity; /* actually can be either VDATA * or VGROUP * */
{
	return( Vinsert(*vg, (VDATA*) *ventity) );
}

/* ------------------------------------------------------------------ */
/* 
tests if an id in a vgroup is a vgroup
related: Visvg--visvgc--VFISVG
*/

#ifdef MAC
pascal
#endif

#ifdef DF_CAPFNAMES
int VISVGC (vg, id)

#else
int visvgc_ (vg, id)
#endif


VGROUP	**vg;
int		*id;
{
	return( Visvg(*vg, *id) );
}

/* ------------------------------------------------------------------ */
/* 
tests if an id in a vgroup is a vdata
related: Visvs--visvsc--VFISVS
*/

#ifdef MAC
pascal
#endif

#ifdef DF_CAPFNAMES
int VISVSC (vg, id)

#else
int visvsc_ (vg, id)
#endif


VGROUP	**vg;
int		*id;
{
	return( Visvs(*vg, *id) );
}
/* ================================================== */
/*  VDATA routines                                    */
/* ================================================== */

/* 
attach to a vdata
related: VSattach--vsatchc--VFATCH
*/

#ifdef MAC
pascal
#endif

#ifdef DF_CAPFNAMES
int * VSATCHC (f, vsid, accesstype)

#else
int * vsatchc_ (f, vsid, accesstype)

#endif

DF		**f;
int	*vsid;
_fcd	accesstype;
{
	VDATA *vs;
	char *acc;

	acc = (char*) DFIf2cstring (accesstype,1);   /* ie 'r' or 'w' only */

	vs =  (VDATA*) VSattach(*f, *vsid, acc);
	(void) DFIfreespace(acc);
	if ( vs == NULL)
		return((int*) -1);
	else 
		return( (int*) vs);
}

/* ------------------------------------------------------------------ */
/*  
detach from a vdata
related: VSdetach--vsdtchc--VFDTCH
*/

#ifdef MAC
pascal
#endif

#ifdef DF_CAPFNAMES
void VSDTCHC (vs)

#else
void vsdtchc_ (vs)
#endif

VDATA **vs;
{
	VSdetach(*vs);
}

/* ------------------------------------------------------------------ */
/* 
seeks to a given element position in a vadata
related: VSseek--vsseekc--VSFSEEK
*/

#ifdef MAC
pascal
#endif

#ifdef DF_CAPFNAMES
int VSSEEKC (vs,eltpos)

#else
int vsseekc_ (vs,eltpos)
#endif


VDATA **vs;
int	*eltpos;
{
	return( VSseek(*vs, *eltpos) );
}

/* ------------------------------------------------------------------ */
/* 
gets the name of a vdata
related: VSgetname--vsgnamc--VSFGNAM
*/

#ifdef MAC
pascal
#endif

#ifdef DF_CAPFNAMES
void VSGNAMC (vs, vsname)

#else
void vsgnamc_ (vs, vsname)
#endif

VDATA **vs;
_fcd vsname;

{
	char  *name;

	name =  _fcdtocp(vsname);
	VSgetname (*vs, name);
}	/* VSGNAMC */

/* ------------------------------------------------------------------ */
/* 
get the class name of a vdata
related: VSgetclass--vsgclsc--VSFGCLS
*/

#ifdef MAC
	pascal
#endif

#ifdef DF_CAPFNAMES
	    void VSGCLSC (vs, vsclass)

#else
	    void vsgclsc_ (vs, vsclass)
#endif

	    VDATA **vs;
	_fcd 	vsclass;

	{
		char  *class;
		class = _fcdtocp(vsclass);
		VSgetclass(*vs, class);
	}	/* VSGCLSC */

/* ------------------------------------------------------------------ */
/*
general inquiry on a vdata
related: VSinquire--vsinqc--VSFINQ
*/

#ifdef MAC
pascal
#endif

#ifdef DF_CAPFNAMES
int VSINQC (vs, nelt ,interlace, fields, eltsize, vsname)

#else
int vsinqc_ (vs, nelt ,interlace, fields, eltsize, vsname)
#endif


VDATA **vs;
int	*nelt, *interlace, *eltsize;
_fcd  fields, vsname;

{

	char  *name,  *flds;
	name =  _fcdtocp(vsname);
	flds =  _fcdtocp(fields);
	trimendblanks(name);
	trimendblanks(flds);
	return( VSinquire (*vs, nelt, interlace, flds, eltsize, name) );
} 	/* VSINQC */




/* ------------------------------------------------------------------ */
/* 
tests if given fields exist in the vdata
related: VSfexist--vsfexc--VSFEX
*/

#ifdef MAC
pascal
#endif

#ifdef DF_CAPFNAMES
int VSFEXC (vs, fields)

#else
int vsfexc_ (vs, fields)
#endif

VDATA **vs;
_fcd	fields;
{
	int 	stat;
	char	*flds;

	flds = (char*) DFIf2cstring(fields, 55);
	trimendblanks(flds);
	stat =  VSfexist(*vs, flds);
	(void) DFIfreespace(flds);

	return (stat);
}

/* ------------------------------------------------------------------ */
/* 
gets the id of the next vdata from the file
related: VSgetid--vsgidc--VSFGID
*/

#ifdef MAC
pascal
#endif

#ifdef DF_CAPFNAMES
int VSGIDC (f,vsid)

#else
int vsgidc_ (f,vsid)
#endif

DF		**f;
int	*vsid;
{
	return( VSgetid( *f, *vsid) );
}

/* ------------------------------------------------------------------ */
/* 
sets the name of a vdata
related: VSsetname--vssnamc--VSFSNAM
*/

#ifdef MAC
pascal
#endif

#ifdef DF_CAPFNAMES
void VSSNAMC (vs, vsname)

#else
void vssnamc_ (vs, vsname)
#endif

VDATA **vs;
_fcd	vsname;

{
	char	*name;

	name = (char*) DFIf2cstring(vsname, VSNAMELENMAX);
	trimendblanks(name);
	VSsetname(*vs, name);
	(void) DFIfreespace(name);
}

/* ------------------------------------------------------------------ */
/* 
sets the class name of the vdata
related: VSsetclass--vssclsc--VSFSCLS
*/

#ifdef MAC
	pascal
#endif

#ifdef DF_CAPFNAMES
	    void VSSCLSC (vs, vsclass)

#else
	    void vssclsc_ (vs, vsclass)
#endif

	    VDATA		**vs;
	_fcd		vsclass;

	{
		char *class;
		class = (char*) DFIf2cstring (vsclass,VSNAMELENMAX);
		trimendblanks(class);
		VSsetclass (*vs, class);
		(void) DFIfreespace(class);
	}

/* ------------------------------------------------------------------ */
/* 
sets the fields in a vdata for reading or writing
related: VSsetfields--vssfldc--VSFSFLD
*/

#ifdef MAC
pascal
#endif

#ifdef DF_CAPFNAMES
int VSSFLDC (vs,fields)

#else
int vssfldc_ (vs,fields)
#endif

VDATA **vs;
_fcd	fields;
{
	char *ss;
	int ret;

	ss = (char*) DFIf2cstring (fields,55);
	trimendblanks(ss);
	ret =	VSsetfields (*vs,ss);

	(void) DFIfreespace(ss);
	return( ret);
}


/* ------------------------------------------------------------------ */
/* 
sets the file interlace of a vdata
related: VSsetinterlace--vssintc--VSFSINT
*/

#ifdef MAC
pascal
#endif

#ifdef DF_CAPFNAMES
int VSSINTC (vs, interlace)

#else
int vssintc_ (vs, interlace)
#endif

VDATA  **vs;
int	 *interlace;
{
	return( VSsetinterlace (*vs, *interlace) );
}

/* ------------------------------------------------------------------ */
/* 
defines a new field to be used in the vdata
related: VSfdefine--vsfdefc--VSFFDEF
*/

#ifdef MAC
pascal
#endif

#ifdef DF_CAPFNAMES
int VSFDEFC (vs,field,localtype,order)

#else
int vsfdefc_ (vs,field,localtype,order)
#endif

VDATA **vs;
_fcd	field;
int 	*localtype, *order;
{
	int ret;
	char *ss;

	ss  = (char*) DFIf2cstring (field, FIELDNAMELENMAX);
	trimendblanks(ss);
	ret =  VSfdefine(*vs, ss, *localtype, *order );
	(void) DFIfreespace(ss);
	return (ret);
}

/* ------------------------------------------------------------------ */
/* 
reads from a vdata
related: VSread--vsreadc--VSFREAD
*/

#ifdef MAC
pascal
#endif

#ifdef DF_CAPFNAMES
int VSREADC (vs, buf, nelt, interlace)

#else
int vsreadc_ (vs, buf, nelt, interlace)
#endif

VDATA		      **vs;
int				*nelt, *interlace;
unsigned char 	*buf;
{
	return( VSread(*vs, buf, *nelt, *interlace) );
}

/* ------------------------------------------------------------------ */
/* 
writes to a vdata
related: VSwrite--vswritc--VSFWRIT
*/

#ifdef MAC
pascal
#endif

#ifdef DF_CAPFNAMES
int VSWRITC (vs, buf, nelt, interlace)

#else
int vswritc_ (vs, buf, nelt, interlace)
#endif


VDATA	 		   **vs;
int				*nelt, *interlace;
unsigned char 	*buf;
{

	return( VSwrite(*vs, buf, *nelt, *interlace) );
}


/* ======================================== */
/* miscellaneous VDATA inquiry routines */
/* ======================================== */
/* undocumented */

/* ------------------------------------------------------------------ */
/* 
gets the interlace of the vdata
related: VSgetinterlace--vsgintc--VSFGINT
*/

#ifdef MAC
pascal
#endif

#ifdef DF_CAPFNAMES
int VSGINTC (vs)

#else
int vsgintc_ (vs)
#endif

VDATA **vs;
{
	return( VSgetinterlace(*vs) );
}

/* ------------------------------------------------------------------ */
/* 
gets the number of elements in the vdata
related: VSelts--vseltsc--VSFELTS
*/

#ifdef MAC
pascal
#endif

#ifdef DF_CAPFNAMES
int VSELTSC (vs)

#else
int vseltsc_ (vs)
#endif

VDATA **vs;
{
	return( VSelts (*vs) );
}

/* ------------------------------------------------------------------ */
/* 
gets the fields in the vdata
related: VSgetfields--vsgfldc--VSFGFLD
*/

#ifdef MAC
pascal
#endif

#ifdef DF_CAPFNAMES
int VSGFLDC (vs,fields)

#else
int vsgfldc_ (vs,fields)
#endif

VDATA **vs;
_fcd  fields;
{
	char  *flds;
	flds =  _fcdtocp(fields);

	return( VSgetfields(*vs,flds) );
}	/* VSGFLDC */

/* ------------------------------------------------------------------ */
/* 
determines the (machine) size of the given fields
related: VSsizeof--vssizc--VSFSIZ
*/

#ifdef MAC
pascal
#endif

#ifdef DF_CAPFNAMES
int VSSIZC (vs,fields)

#else
int vssizc_ (vs,fields)
#endif

VDATA **vs;
_fcd	fields;
{
	int 	stat;
	char *flds;

	flds = (char*) DFIf2cstring (fields,55);
	trimendblanks(flds);
	stat =  VSsizeof(*vs, flds);
	(void) DFIfreespace(flds);
	return (stat);
}

/* ------------------------------------------------------------------ */
/*
determines the no of entries in a vgroup
related: Ventries--ventsc--VFENTS
*/

#ifdef MAC
pascal
#endif

#ifdef DF_CAPFNAMES
int VENTSC (f,vgid) 
#else
	int ventsc_ (f,vgid) 
#endif
DF * f;
int vgid;
{

		return( Ventries (f,vgid) );
	}

/* ======================================================= */
/* DEBUGGING ROUTINES                                      */
/* ======================================================= */
/*
enable debug
related: setjj--setjjc--SETFJJ
*/

#ifdef MAC
	pascal
#endif

#ifdef DF_CAPFNAMES
	    void SETJJC ()
#else
	    void setjjc_ ()
#endif
	    { setjj(); }

/* ------------------------------------------------------------------ */
/*
disable debug
related: setnojj--setnojjc--SETFNJJ
*/

#ifdef MAC
	pascal
#endif

#ifdef DF_CAPFNAMES
	    void SETNOJJC ()
#else
	    void  setnojjc_ ()
#endif

	    { setnojj(); }

/* ------------------------------------------------------------------ */
/* 
gets the refs of all lone vgroups in the file
related: Vlone--vlonec--VFLONE
*/

#ifdef MAC
	pascal
#endif

#ifdef DF_CAPFNAMES
	    int VLONEC (f, idarray, asize)

#else
		int vlonec_ (f, idarray, asize)
#endif

			DF		**f;
	int	**idarray; /* an integer array */
	int	*asize;
	{
		return( Vlone( *f, *idarray, *asize) );
	}

/* ------------------------------------------------------------------ */
/*
gets the refs of all lone vdatas in the file
related: VSlone--vslonec--VSFLONE
*/

#ifdef MAC
	pascal
#endif

#ifdef DF_CAPFNAMES
	    int VSLONEC (f, idarray, asize)

#else
		int vslonec_ (f, idarray, asize)
#endif

			DF		**f;
	int	**idarray; /* an integer array */
	int	*asize;
	{
		return( VSlone( *f, *idarray, *asize) );
	}

/*
==================================================================
HIGH-LEVEL VSET ROUTINES --- VHxxxxx()
==================================================================
*/

/*
store a simple dataset in a vdata 
related: VHstoredata--vhsdc--vhfsd
*/
#ifdef MAC
pascal
#endif

#ifdef DF_CAPFNAMES
int VHSDC (f, field, buf, n, datatype, vsname, vsclass)

#else
int vhsdc_ (f, field, buf, n, datatype, vsname, vsclass)
#endif

DF		**f;
_fcd   field, vsname, vsclass;
int	*n, *datatype;
unsigned char *buf;

{
   char  *name,  *class, *fld;
	name  =  _fcdtocp(vsname);
	class =  _fcdtocp(vsclass);
	fld   =  _fcdtocp(field);
	trimendblanks(name);
	trimendblanks(class);
	trimendblanks(fld);
	 return( VHstoredata (*f, fld , buf, *n, *datatype, name, class));
}

/* ------------------------------------------------------------------ */
/*
store an aggregate dataset in a vdata
related: VHstoredatam--vhsdmc--vhfsdm
*/
#ifdef MAC
pascal
#endif

#ifdef DF_CAPFNAMES
int VHSDMC (f, field, buf, n, datatype, vsname, vsclass, order)

#else
int vhsdmc_ (f, field, buf, n, datatype, vsname, vsclass, order)
#endif

DF		**f;
_fcd   field, vsname, vsclass;
int	*n, *datatype, *order;
unsigned char *buf;

{
   char  *name,  *fld, *class;
	name  =  _fcdtocp(vsname);
	class =  _fcdtocp(vsclass);
	fld   =  _fcdtocp(field);
	trimendblanks (name);
	trimendblanks (class);
	trimendblanks (fld);

	 return( VHstoredatam (*f, fld , buf, *n, *datatype, name, class, *order));
}



/* ------------------------------------------------------------------ */
/*
make a new vgroup given several tag/ref pairs 
related: VHmakegroup--vhmkgpc--vhfmkgp
*/
#ifdef MAC
pascal
#endif

#ifdef DF_CAPFNAMES
int VHMKGPC  (f, tagarray, refarray , n, vgname, vgclass)

#else
int vhmkgpc_  (f, tagarray, refarray , n, vgname, vgclass)
#endif

DF		**f;
_fcd   vgname, vgclass;
int	*n, *tagarray, *refarray;

{
   char  *name, *class;
	name  =  _fcdtocp(vgname);
	class =  _fcdtocp(vgclass);
	trimendblanks (name);
	trimendblanks (class);
	return ( VHmakegroup (*f, tagarray, refarray , *n, name, class));

}

/* ================================================================== */
/* ------- May 1991 new routines ---- */
#ifdef MAC
pascal
#endif

#ifdef DF_CAPFNAMES
int VFLOCC (vg, field)

#else
int vflocc_ (vg, field)
#endif

VGROUP	**vg;
_fcd   field;
{
   char  *fld;
	fld =  _fcdtocp(field);
	trimendblanks(fld);


	return( Vflocate (*vg, fld));
}
/* ------------------------------------------------------------------ */
/* 
tests if a tag/ref pair is in a vgroup.
related: Vinqtagref--vinqtrc--vfinqtr
*/

#ifdef MAC
pascal
#endif

#ifdef DF_CAPFNAMES
int VINQTRC (vg, tag, ref)

#else
int vinqtrc_ (vg, tag, ref)
#endif

VGROUP	**vg;
int	 *tag, *ref;

{
	return ( Vinqtagref ( *vg, *tag, *ref) );
}
/* ------------------------------------------------------------------ */

/* 
gets the number of tag/refs stored in a vgroup
related: Vntagrefs--vntrc--VFNTR
*/

#ifdef MAC
pascal
#endif

#ifdef DF_CAPFNAMES
int VNTRC (vg)

#else
int vntrc_ (vg)
#endif

VGROUP	**vg;

{
	return (  Vntagrefs (*vg) );
}
/* ------------------------------------------------------------------ */

/*
returns all the tag/ref pairs in a vgroup 
related: Vgettagrefs--vgttrsc--vfgttrs
*/

#ifdef MAC
pascal
#endif

#ifdef DF_CAPFNAMES
int VGTTRSC (vg, tagarray, refarray, n)

#else
int vgttrsc_ (vg, tagarray, refarray, n)
#endif

VGROUP	**vg;
int *tagarray, *refarray, *n;
{
	return (  Vgettagrefs (*vg, tagarray, refarray, *n) );
}

/*
returns a specified tag/ref pair in a vgroup 
related: Vgettagref--vgttrc--vfgttr
*/

#ifdef MAC
pascal
#endif

#ifdef DF_CAPFNAMES
int VGTTRC (vg, which, tag, ref)

#else
int vgttrc_ (vg, which, tag, ref)
#endif

VGROUP	**vg;
int *which;
int *tag, *ref;
{
	return (  Vgettagref (*vg, *which, tag, ref) );
}
/* ------------------------------------------------------------------ */

/*
add a tag/ref pair to a vgroup 
related: Vaddtagref--vadtrc--VFADTR 
*/
#ifdef MAC
pascal
#endif

#ifdef DF_CAPFNAMES
int  VADTRC ( vg, tag, ref)

#else
int  vadtrc_ ( vg, tag, ref)
#endif
VGROUP	**vg;
int		*tag, *ref;
{
	return ( Vaddtagref ( *vg, *tag, *ref) );
}
/* ------------------------------------------------------------------ */

