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
*  Likkai Ng MAY 91  NCSA
* 
* vg.h
* Part of HDF VSet interface
*
* defines symbols and structures used in all v*.c files    
*
* NOTES:
* This include file depends on the basic HDF *.h files dfi.h and df.h.
* An 'S' in the comment means that that data field is saved in the HDF file.
*
******************************************************************************/


#include <df.h>

#define PUBLIC		
#define PRIVATE	static



/* 
* interlacing supported by the vset. 
*/

#define FULL_INTERLACE	0
#define NO_INTERLACE		1

/* 
* some max lengths 
*
* Except for FIELDNAMELENMAX, change these as you please, they 
* affect memory only, not the file. 
*
*/

#define FIELDNAMELENMAX	16				/* fieldname   : 16 chars max */

#define VSFIELDMAX		20  			/* max no of fields per vdata */
#define VSNAMELENMAX		64				/* vdata name  : 64 chars max */	
#define VGNAMELENMAX		64				/* vgroup name : 64 chars max */	
#define VFILEMAX 			40 			/* max no of files that may be opened */
/* actually, for now, this is the max # of file units that can be used */


/*
* definition of the 2 data elements of the vset.
*/

typedef struct vgroup_desc     	VGROUP;
typedef struct vdata_desc			VDATA;

typedef VDATA VSUBGROUP;


/*
* -----------------------------------------------------------------
* structures that are part of the VDATA structure
* -----------------------------------------------------------------
*/

typedef struct symdef_struct 
{
	char* name;						/* symbol name */
	int	type;						/* whether int, char, float etc */
	int	isize;					/* field size as stored in vdata */
	int	order;					/* order of field */

	} SYMDEF;

typedef struct write_struct	
{
	int32	n;							/* S actual # fields in element */
	int	ivsize;					/* S size of element as stored in vdata */
 	char 	name[VSFIELDMAX][FIELDNAMELENMAX+1];
										/* S name of each field */
	int	len[VSFIELDMAX]; 		/* S length of each fieldname */
	int	type[VSFIELDMAX];		/* S field type */
  	int	off[VSFIELDMAX];		/* S field offset in element in vdata */
  	int 	isize[VSFIELDMAX];	/* S internal (HDF) size [incl order] */
  	int	order[VSFIELDMAX];	/* S order of field */
  	int	esize[VSFIELDMAX];	/*  external (local machine) size [incl order] */
	int	(*toIEEEfn  [VSFIELDMAX] )();
	int	(*fromIEEEfn[VSFIELDMAX] )();

  	}		 VWRITELIST;

typedef struct read_struct
{
	int	n; 						/* # fields to read */
	int	item[VSFIELDMAX]; 	/* index into vftable_struct */

	} VREADLIST;

/* 
*  ----------------------------------------------- 
        V G R O U P     definition     
*  ----------------------------------------------- 
*/

#define MAXNVELT  36				/* max no of elements in a vgroup */

struct vgroup_desc
{ 
	int    otag, oref;			/* tag-ref of this vgroup */
  	DF		 *f;	 	     			/* HDF file ptr */
	int16	 nvelt; 					/* S no of elements */
  	int	 access;					/* 'r' or 'w' */
	int	 tag[MAXNVELT];		/* S tag of element */
	int	 ref[MAXNVELT];		/* S ref of element */
 	char	 vgname[VGNAMELENMAX+1];/* S name of this vgroup */
 	char	 vgclass[VGNAMELENMAX+1];/*  class name of this vgroup */
	VDATA* velt[MAXNVELT];	 	/* pts to a element if opened; or else NULL */
	int	marked;					/* =1 if new info has been added to vgroup */
	int16		extag, exref;			/* expansion tag-ref */
	int16		version, more;			/* version and "more" field */	

	};									/* VGROUP */

/*
*  ----------------------------------------------- 
*         V D A T A      definition   
*  ----------------------------------------------- 
*/

#define USYMMAX 36				/* max user-defined symbols allowed */

struct vdata_desc{ 
  	int	otag, oref; 			/* tag,ref of this vdata */
  	DF		*f;	 	     			/* HDF file ptr */
  	int	access;					/* 'r' or 'w' */
 	char	vsname[VSNAMELENMAX+1];/* S name of this vdata */
 	char	vsclass[VSNAMELENMAX+1];/* class name of this vdata */
  	int	interlace;				/* S  interlace as in file */
  	int32	nvertices;				/* S  #vertices in this vdata */
	int	vpos;    				/* cur position (in terms of elements) */
	VWRITELIST	wlist;
  	VREADLIST	rlist;
	int  			nusym;
	SYMDEF 		usym[USYMMAX];
	int	marked;					/* =1 if new info has been added to vdata */

	int16		extag, exref;			/* expansion tag-ref */
	int16		version, more;			/* version and "more" field */	

  	};  								/* VDATA */ 

/* MACROS - Use these for accessing field components of vdata. */

#define VFnfields(vdata) 		(vdata->wlist.n)
#define VFfieldname(vdata,t) 	(vdata->wlist.name[t])
#define VFfieldtype(vdata,t) 	(vdata->wlist.type[t])
#define VFfieldisize(vdata,t) 	(vdata->wlist.isize[t])
#define VFfieldesize(vdata,t) 	(vdata->wlist.esize[t])
#define VFfieldorder(vdata,t) 	(vdata->wlist.order[t])


/* --------------  H D F    V S E T   tags  ---------------------------- */

#define OLD_VGDESCTAG  	61820		/* tag for a vgroup d*/ 
#define OLD_VSDESCTAG 	61821		/* tag for a vdata descriptor */
#define OLD_VSDATATAG 	61822		/* tag for actual raw data of a vdata */ 

#define NEW_VGDESCTAG    1965
#define NEW_VSDESCTAG    1962
#define NEW_VSDATATAG    1963

#define VGDESCTAG 		NEW_VGDESCTAG 
#define VSDESCTAG 		NEW_VSDESCTAG 
#define VSDATATAG  		NEW_VSDATATAG 

/*
* Actual sizes of data types stored in HDF file, and are IEEE-defined. 
*/

#define IEEE_UNTYPEDSIZE   0
#define IEEE_CHARSIZE      1
#define IEEE_INT16SIZE     2
#define IEEE_INT32SIZE     4
#define IEEE_FLOATSIZE     4

/*
* types used in defining a new field via a call to VSfdefine
*/

#define LOCAL_NOTYPE			0
#define LOCAL_CHARTYPE  	1
#define LOCAL_INTTYPE 	 	2
#define LOCAL_FLOATTYPE		3
#define LOCAL_LONGTYPE 	 	4

/*
* actual LOCAL MACHINE sizes of the above types
*/

#define LOCAL_UNTYPEDSIZE  0
#define LOCAL_CHARSIZE  	sizeof(unsigned char)
#define LOCAL_INTSIZE  		sizeof(int)
#define LOCAL_LONGSIZE 		sizeof(long)
#define LOCAL_FLOATSIZE 	sizeof(float)

/* ------------------------------------------------------------------ */
/* 2 GLOBAL VARIABLES (int vjv and char sj[]) provide a simple
* debugging scheme. Debugging is turned on and off via calls to 
* setjj and setnojj. These globals and functions are found in vgp.c.
* 
* If the debug feature is no longer needed, delete all these, and
* all statements that refer to xzj,sjs, and vjv from the source
*
*/

extern int 		vjv; 			/* debugger switch */
extern char 	sjs[]; 		/* contains the debug/error message */

/* zj just prints out the contents of the text buffer sjs. */
/* sjs contains debugging messages when debug is on */
/* for the Mac, replace define zj to be any print msg routine */
/* for printf and fprintf - there should be some Mac equivalent */

#ifdef MAC
#define printf
#define fprintf
#define zj
#else
#define zj 		fprintf(stderr,"%s",sjs) 
#endif

/* Macros for returning null, -1 or no value with a message */

#define FAIL -1
#define RTNULL(ss) { sprintf(sjs,"@%s\n",ss); zj; return(NULL); }
#define RTNEG(ss) { sprintf(sjs,"@%s\n",ss); zj; return(FAIL); }
#define RT(ss) { sprintf(sjs,"@%s\n",ss); zj; return; }

/* .................................................................. */
/* Private data structures. Unlikely to be of interest to applications */
/* 
* These are just typedefs. Actual vfile_ts are declared PRIVATE and
* are not accessible by applications. However, you may change VFILEMAX
* to allow however many files to be opened.
*
* These are memory-resident copies of the tag-refs of the vgroups
* and vdatas for each file that is opened.
* 
*/

/* this is a memory copy of a vs tag/ref found in the file */
typedef struct vg_instance_struct {
	int 	ref;			/* ref # of this vgroup in the file */
	int 	nattach;		/* # of current attachs to this vgroup */
	int 	nentries;	/* # of entries in that vgroup initially */
	VGROUP *vg;			/* points to the vg when it is attached */
struct vg_instance_struct * next;
}	vginstance_t; 

/* this is a memory copy of a vs tag/ref found in the file */
typedef struct vs_instance_struct {
	int	ref;			/* ref # of this vdata in the file */
	int	nattach;		/* # of current attachs to this vdata */
	int	nvertices;	/* # of elements in that vdata initially */
	VDATA	* vs; 		/* points to the vdata when it is attached */
	struct vs_instance_struct * next;
}	vsinstance_t; 

/* each vfile_t maintains 2 linked lists: one of vgs and one of vdatas
* that already exist or are just created for a given file.  */

typedef struct vfiledir_struct {
	DF			* f;

   int				vgtabn;				/* # of vg entries in vgtab so far */
	vginstance_t	vgtab;				/* start of vg linked list */
   vginstance_t	*vgtabtail;	 		/* its tail end */

   int				vstabn;				/* # of vs entries in vstab so far */
	vsinstance_t	vstab;				/* start of vs linked list */
   vsinstance_t	*vstabtail;			/* its tail end */
} vfile_t;

/* .................................................................. */
#define VSET_VERSION 2					/* DO NOT CHANGE!! */

#ifdef MAC
#include "vset-prototypes.h"
#include <String.h>
#include <ctype.h>
#endif

/* 
* DFopen and DFclose are redefined to perform additional work, 
* ie Vset data structure initialization and clean-up.
* Does not affect usage by other HDF interfaces.
*/

#define DFopen  DFvsetopen
#define DFclose DFvsetclose

