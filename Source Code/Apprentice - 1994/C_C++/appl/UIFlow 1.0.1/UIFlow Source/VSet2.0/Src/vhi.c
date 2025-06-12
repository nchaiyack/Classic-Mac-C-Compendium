/* 
* ============================================
* VHxxx  routines:
* HDF Vset high-level access routines
* 28-MAR-91 Jason Ng NCSA
* ============================================
*/

#include "vg.h"

/* ------------------------------------------------------------------ */
/*
Stores 'n' elements of data from 'buf' as a
field 'field' in a new vdata called 'vsname' into the
already opened HDF file (with pointer 'f').

The datatype variable must be specified as one of:
LOCAL_FLOATTYPE, LOCAL_INTTYPE, LOCAL_LONGTYPE, LOCAL_CHARTYPE.

NOTES:
n should not be zero or negative.
RETURN:
-1 if error.
ref of that new vdata (a +ve integer) if successful.
Need to check if field already defined
*/

PUBLIC int VHstoredata (f, field, buf, n, datatype, vsname, vsclass)
DF * f;
char * field;
unsigned char buf[];
int32 n;
char * vsname, * vsclass;
int datatype;
{
	int ref;
	int order = 1;


	ref=  VHstoredatam (f, field, buf, n, datatype,  vsname, vsclass, order);

	return(ref);
}

/* ------------------------------------------------------------------ */
/*
Same as VHstoredata but allows aggregate-typed field.
Stores 'n' elements of data from 'buf' as a
field 'field' in a new vdata called 'vsname' into the
already opened HDF file (with pointer 'f').

The datatype variable must be specified as one of:
LOCAL_FLOATTYPE, LOCAL_INTTYPE, LOCAL_LONGTYPE, LOCAL_CHARTYPE.

NOTES:
n should not be zero or negative.
RETURN:
-1 if error.
ref of that new vdata (a +ve integer) if successful.
Need to check if field already defined
*/

PUBLIC int VHstoredatam (f, field, buf, n, datatype, vsname, vsclass, order)
DF 	*f;
char 	*field;
unsigned char buf[];
int32 n;
int 	datatype;
int 	order;
char *vsname, *vsclass;
{
	int 	s;
	int ref;
	VDATA *vs;

	vs = (VDATA*) VSattach (f,-1,"w");          if (vs==NULL) return (-1);
	s = VSfdefine (vs, field, datatype, order); if (s == -1) return (-1);
	s = VSsetfields (vs,field);                 if (s == -1) return (-1);
	s = VSwrite (vs, buf, n, FULL_INTERLACE);	  if (n != s)  return (-1);
	VSsetname (vs, vsname);
	VSsetclass(vs, vsclass);
	ref = vs->oref;
	VSdetach(vs);
	return(ref);

} /* VHstoredatam */

/* ------------------------------------------------------------------ */
/* 
Takes an array of tags and and array of refs and create a vgroup to
store them. You tell it how many tag/ref pairs there are. You must 
also give the vgroup a name.
NOTE:
Does bot check if a tag/ref is valid or exist, but ALL tag/ref 
pairs MUST be unique.
Creating EMPTY vgroups is allowed.

RETURN: 
-1 if error
ref of the new vgroup (a +ve integre) if ok.
*/

PUBLIC int VHmakegroup (f, tagarray, refarray , n, vgname, vgclass)
DF * f;
int tagarray[], refarray[];
int n;
char 	*vgname, *vgclass;
{
	int ref, i, s;	
	VGROUP *vg;

	vg = (VGROUP*) Vattach (f, -1, "w"); if (vg==NULL) return (-1);
	Vsetname (vg, vgname);
	Vsetclass(vg, vgclass);

	for (i=0; i<n; i++) {
		s = Vaddtagref (vg, tagarray[i], refarray[i]);
		if (s == -1) return (-1);
		}

	ref = vg->oref;
	Vdetach (vg);
	return (ref);

} /* VHmakegroup */

/* ------------------------------------------------------------------ */
