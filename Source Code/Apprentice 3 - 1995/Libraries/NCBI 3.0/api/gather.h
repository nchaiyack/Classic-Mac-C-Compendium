/*   gather.h
* ===========================================================================
*
*                            PUBLIC DOMAIN NOTICE
*            National Center for Biotechnology Information (NCBI)
*
*  This software/database is a "United States Government Work" under the
*  terms of the United States Copyright Act.  It was written as part of
*  the author's official duties as a United States Government employee and
*  thus cannot be copyrighted.  This software/database is freely available
*  to the public for use. The National Library of Medicine and the U.S.
*  Government do not place any restriction on its use or reproduction.
*  We would, however, appreciate having the NCBI and the author cited in
*  any work or product based on this material
*
*  Although all reasonable efforts have been taken to ensure the accuracy
*  and reliability of the software and data, the NLM and the U.S.
*  Government do not and cannot warrant the performance or results that
*  may be obtained by using this software or data. The NLM and the U.S.
*  Government disclaim all warranties, express or implied, including
*  warranties of performance, merchantability or fitness for any particular
*  purpose.
*
* ===========================================================================
*
* File Name:  gather.h
*
* Author:  Jim Ostell
*
* Version Creation Date:   10/7/94
*
* $Revision: 1.14 $
*
* File Description: 
*
* Modifications:  
* --------------------------------------------------------------------------
* Date     Name        Description of modification
* -------  ----------  -----------------------------------------------------
*
* $Log: gather.h,v $
 * Revision 1.14  1995/06/01  21:53:55  kans
 * support for Seq-align (zjing)
 *
 * Revision 1.13  1995/05/15  21:46:05  ostell
 * added Log line
 *
*
*
*
* ==========================================================================
*/

#ifndef _GATHER_
#define _GATHER_

#include <sequtil.h>


typedef struct gatherrange {
  Int4     left;
  Int4     right;
  Boolean  l_trunc;
  Boolean  r_trunc;
  Uint1    strand;
} GatherRange, PNTR GatherRangePtr;

typedef struct seqrange {
  Int4		start;
  Int4		stop;
  Uint1		strand;
 } SeqRange, PNTR SeqRangePtr;
 
typedef struct alignrange{
  Uint1    segtype;  /* codes for segments of alignments, _SEG below */
  GatherRange gr;
  SeqRange sr;
  struct alignrange PNTR next;
 } AlignRange, PNTR AlignRangePtr;
  
 
 typedef struct aligndata {
 	SeqIdPtr sip;
 	GatherRange extremes;	/*two ends on the graph*/
 	SeqRange	seqends;	/*two ends on the sequence*/
 	AlignRangePtr arp;
  	struct aligndata PNTR next;
 }AlignData, PNTR AlignDataPtr;
 	

#define GAP_SEG 1	    /* a gap */
#define REG_SEG 2		 /* a segment */
#define INS_SEG 3		 /* an insertion */
#define STD_SEG	4		/*a segment from StdSeg*/
#define DIAG_SEG 5		/*a diagnol segment*/

/*****************************************************************************
*
*   GatherSeqEntry() and GatherItem() return a pointer to the GatherContext
*   below. entityID, itemID, and thistype are the 3 minimum elements required
*   for GatherItem to relocate the item.
*
*   The fields seglevel and propagated will NOT be set by GatherItem(). They
*   are ONLY set by GatherSeqEntry().
*
*****************************************************************************/


typedef struct gathercontext {
	Uint2  entityID ,
		  itemID;
	Pointer thisitem,
		  previtem,
		  parentitem;
	Uint2 thistype,
		  prevtype,
		  parenttype;
	Pointer PNTR prevlink;    /* used to remove data item from chains */
	SeqEntryPtr sep;          /* sep that points to Bioseq or BioseqSet in thisitem */
	Pointer userdata;
	Boolean tempload;		  /* this data temporarily loaded */
	GatherRangePtr rdp;		  /* mapped intervals to global coordinates */
	AlignDataPtr adp;			/*for aligned data*/
	Int2 num_interval;		  /* number of mapped intervals */
	GatherRange extremes;       /* extreme left and right locations */
	Boolean propagated;       /* is this item propagated from out of scope? */
	Int2 seglevel;            /* how many segment levels are we? */
	Boolean product;          /* TRUE if match by SeqId on product */
	Int2 indent;              /* indent level in containing Seq-entry */
} GatherContext, PNTR GatherContextPtr;

typedef Boolean (*GatherItemProc) (GatherContextPtr gcp);

typedef struct gatherscope {
	SeqEntryPtr scope;        /* NULL, or SeqEntry to limit data items to */
	Boolean get_feats_location,   /* get all features if location on Bioseq */
		get_feats_product;    /* get all features if product on Bioseq */
	SeqLocPtr target;            /* limit features to a range on the Bioseq */
	Int4 offset;              /* offset features this much */
	Boolean ignore[OBJ_MAX];	/* if TRUE, this GITEM does not call userfunc */
	Boolean nointervals;        /* if TRUE, don't populate GatherIntervals */
	Int2 seglevels,             /* number of segment levels to traverse */
		currlevel;              /* current segment depth */
	Boolean split_packed_pnt;   /* if TRUE, give itemID to each point */
} GatherScope, PNTR GatherScopePtr;

typedef struct internalgcc {
	GatherContext gc;
	GatherScope scope;
	GatherItemProc userfunc;
	Boolean rev;                /* TRUE if scope->target is on complement */
	Int2 max_interval;
	BioseqPtr bsp;              /* for checking a range on a Bioseq */
	Int2 itemIDs[OBJ_MAX];
	SeqLocPtr segloc;           /* if target is segmented and seglevel > 0 */
	Int2 first_seg, last_seg;   /* see SeqLocCopyPart() in edutil.h */
	Uint1 locatetype;           /* itemtype to locate */
	Int2 locateID;              /* itemID to locate */
} InternalGCC, PNTR InternalGCCPtr;

Boolean SeqLocOffset PROTO((SeqLocPtr seq_loc, SeqLocPtr sfp_loc, GatherRangePtr range, Int4 offset));

AlignDataPtr FreeAlignData PROTO((AlignDataPtr adp));
AlignDataPtr gather_align_data PROTO((SeqLocPtr m_slp, SeqAlignPtr align, Int4 offset, Boolean ck_interval, Boolean map));

/*****************************************************************************
*
*   GatherEntity (entityID, userdata, userproc, scope)
*
*****************************************************************************/
Boolean LIBCALL GatherEntity PROTO((Uint2 entityID, Pointer userdata, GatherItemProc userfunc, GatherScopePtr scope));

/*****************************************************************************
*
*   GatherSeqEntry (sep, userdata, userproc, scope)
*
*****************************************************************************/
Boolean LIBCALL GatherSeqEntry PROTO((SeqEntryPtr sep, Pointer userdata, GatherItemProc userfunc, GatherScopePtr scope));

/*****************************************************************************
*
*   GatherItem (entityID, itemID, itemtype, userdata, userproc)
*      Get an item by entityID, itemID, itemtype
*      GatherContext.seglevel and GatherContext.propagated will not be
*        set on the callback.
*      itemtype is as defined in objmgr.h for OBJ_
*        only those included in GITEM_ defined above will be found
*
*****************************************************************************/
Boolean LIBCALL GatherItem PROTO((Uint2 entityID, Uint2 itemID, Uint2 itemtype,
                                   Pointer userdata, GatherItemProc userfunc));

/****************************************************************************
*
*   GatherDataForProc(ompcp, sel)
*
*       fills in data, choice, and choictype in OMProcControlPtr
*         sets ompcp->whole_entity TRUE if appropriate
*       returns TRUE if it did it
*       if (sel == TRUE), fills in ompcp with data from ObjMgrGetSelect first.
*          returns FALSE if nothing selected.. Does ErrPostEx() for it
*
****************************************************************************/
Boolean LIBCALL GatherDataForProc PROTO((OMProcControlPtr ompcp, Boolean sel));

/****************************************************************************
*
*   DetachDataForProc(ompcp, sel)
*
*       fills in data, choice, and choictype in OMProcControlPtr
*         sets ompcp->whole_entity TRUE if appropriate
*       returns TRUE if it did it
*       if (sel == TRUE), fills in ompcp with data from ObjMgrGetSelect first.
*          returns FALSE if nothing selected.. Does ErrPostEx() for it
*       Detaches data item from surrounding data if necessary
*
****************************************************************************/
Boolean LIBCALL DetachDataForProc PROTO((OMProcControlPtr ompcp, Boolean sel));

/****************************************************************************
*
*   AttachDataForProc(ompcp, sel)
*
*       fills in data, choice, and choictype in OMProcControlPtr
*         sets ompcp->whole_entity TRUE if appropriate
*       returns TRUE if it did it
*       if (sel == TRUE), fills in ompcp with data from ObjMgrGetSelect first.
*          returns FALSE if nothing selected.. Does ErrPostEx() for it
*       Attaches data in output section of ompcp into the input section
*
****************************************************************************/
Boolean LIBCALL AttachDataForProc PROTO((OMProcControlPtr ompcp, Boolean sel));

/****************************************************************************
*
*   CopyDataForProc(ompcp, sel)
*
*       fills in data, choice, and choictype in OMProcControlPtr
*         sets ompcp->whole_entity TRUE if appropriate
*       returns TRUE if it did it
*       if (sel == TRUE), fills in ompcp with data from ObjMgrGetSelect first.
*          returns FALSE if nothing selected.. Does ErrPostEx() for it
*       Attaches copy of data in output section of ompcp
*       adds to objmgr as necessary, but does NOT register copy
*
****************************************************************************/
Boolean LIBCALL CopyDataForProc PROTO((OMProcControlPtr ompcp, Boolean sel));

/****************************************************************************
*
*   ReplaceDataForProc(ompcp, sel)
*
*       fills in data, choice, and choictype in OMProcControlPtr
*         sets ompcp->whole_entity TRUE if appropriate
*       returns TRUE if it did it
*       if (sel == TRUE), fills in ompcp with data from ObjMgrGetSelect first.
*          returns FALSE if nothing selected.. Does ErrPostEx() for it
*
*       Replaces data in input section of ompcp with the output section
*       Data in input section is deleted
*
*       This replaces all contained data!! Replacing a Bioseq replaces any
*         attached descriptors or annots or history as well, for example.
*
*       This does NOT send update messages to the ObjMgr for you.
*
****************************************************************************/
Boolean LIBCALL ReplaceDataForProc PROTO((OMProcControlPtr ompcp, Boolean sel));


/*****************************************************************************
*
*   GatherProcLaunch(proctype, sel, entityID, itemID, itemtype,
*        inputtype, subinputtype, outputtype, suboutputtype)
*
*   	looks for proctype of highest priority that
*   		matches inputtype and outputtype
*     if (sel) then fills in entityID,itemID,itemtype with currently selected
*        item
*     else
*        uses the function arguments
*     locates the data pointer, determines the subtype (if any)
*     then finds the highest priority process
*   	0 on outputtype, inputsubtype, outputsubtype matches any
*     if subtype can be matched on input/output, takes in preference over
*        more general proc
*
*     USAGE:
*     1) To launch an editor for the currently selected item
*      GatherProcLaunch(OMPROC_EDIT,TRUE,0,0,0,0,0,0,0);
*     2) To launch an editor to create a new seq_descr of type pub
*      GatherProcLaunch(OMPROC_EDIT,FALSE,0,0,0,OBJ_SEQDESC,Seq_descr_pub,0,0);
*     3) To launch an editor for a specific seq_descr
*      GatherProcLaunch(OMPROC_EDIT,FALSE,2,1,4,0,0,0,0);
*        (where the 3 numbers identify the seq_descr)
*     4)To launch an editor which will create a new seq_descr and attach it
*        to the currently selected Bioseq
*      GatherProcLaunch(OMPROC_EDIT,TRUE,0,0,0,OBJ_SEQDESC,Seq_descr_pub,0,0)
*        Note in this case ompcp->input_entityid, .input_itemid, input_itemtype
*          well refer to a Bioseq. The editor should check the input_itemtype
*          and decide if it can attach it's output to it, or if it is an
*          input type mismatch error.
*     5) To launch an editor which will create a new seq_descr and attach to
*        a specific Bioseq
*         (Same as (4) but sel=FALSE, and entitid,itemid,itemtype filled in
*          for the Bioseq).
*
*     GENERAL RULES:
*
*     All this means the function will be called with OMProcControlPtr (ompcp)
*      fields filled in (input_entityid, input_itemid, input_itemtype) as:
*       1) Create new one, place in desktop = 0,0,0
*       2) Edit old one, non-zero values, with input_itemtype matching the type
*          of the editor.
*       3) Create a new one, attach it to something else, non-zero values,
*          with input_itemtype not matching the type of the editor.
*
*     Functions to install the returned values are for the cases above:
*       1) ObjMgrRegister()
*       2) GatherReplaceDataForProc()
*       3) GatherAttachDataForProc()
*
*     returns the return from the proc, or OM_MSG_RET_NOPROC if not found
*
*****************************************************************************/
Int2 GatherProcLaunch PROTO((Uint2 proctype, Boolean sel, Uint2 entityID, Uint2 itemID,
                    Uint2 itemtype, Uint2 inputtype, Uint2 subinputtype, Uint2 outputtype, Uint2 suboutputtype));

#endif
