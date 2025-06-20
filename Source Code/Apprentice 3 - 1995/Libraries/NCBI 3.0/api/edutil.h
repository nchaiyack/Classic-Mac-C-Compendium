/*  edutil.h
* ===========================================================================
*
*                            PUBLIC DOMAIN NOTICE                          
*               National Center for Biotechnology Information
*                                                                          
*  This software/database is a "United States Government Work" under the   
*  terms of the United States Copyright Act.  It was written as part of    
*  the author's official duties as a United States Government employee and 
*  thus cannot be copyrighted.  This software/database is freely available 
*  to the public for use. The National Library of Medicine and the U.S.    
*  Government have not placed any restriction on its use or reproduction.  
*                                                                          
*  Although all reasonable efforts have been taken to ensure the accuracy  
*  and reliability of the software and data, the NLM and the U.S.          
*  Government do not and cannot warrant the performance or results that    
*  may be obtained by using this software or data. The NLM and the U.S.    
*  Government disclaim all warranties, express or implied, including       
*  warranties of performance, merchantability or fitness for any particular
*  purpose.                                                                
*                                                                          
*  Please cite the author in any work or product based on this material.   
*
* ===========================================================================
*
* File Name:  edutil.h
*
* Author:  James Ostell
*   
* Version Creation Date: 2/2/94
*
* $Revision: 1.6 $
*
* File Description:  Sequence editing utilities
*
* Modifications:  
* --------------------------------------------------------------------------
* Date       Name        Description of modification
* -------  ----------  -----------------------------------------------------
*
* $Log: edutil.h,v $
 * Revision 1.6  1995/05/15  21:46:05  ostell
 * added Log line
 *
*
*
*
* ==========================================================================
*/
#ifndef _NCBI_EdUtil_
#define _NCBI_EdUtil_

#ifndef _NCBI_SeqPort_
#include <seqport.h>           /* other utilities */
#endif

#ifdef __cplusplus
extern "C" {
#endif


/*****************************************************************************
*
*   Sequence Editing Utilties
*       High Level Cut, Copy, Paste
*
*****************************************************************************/


/*****************************************************************************
*
*   BioseqDelete (target, from, to, do_feat)
*      Deletes the region of sequence between from-to, inclusive, on the
*        Bioseq whose SeqId is target.
*      If do_feat, the feature table is updated to reflect the deletion
*        using SeqEntryDelFeat()
*
*****************************************************************************/
Boolean BioseqDelete (SeqIdPtr target, Int4 from, Int4 to, Boolean do_feat);



/*****************************************************************************
*
*   BioseqCopy(newid, sourceid, from, to, strand, do_feat)
*      Creates a new Bioseq from sourceid in the range from-to inclusive.
*      If strand==Seq_strand_minus, reverse complements the sequence in
*        the copy and (if do_feat) corrects the feature table appropriately.
*      Names new Bioseq as newid, if not NULL
*        else Creates seqid.local = "Clipboard" if newid is NULL
*      If do_feat == TRUE copies appropriate region of feature table from
*        sourceid to new copy using SeqFeatsCopy().
*
*****************************************************************************/
BioseqPtr BioseqCopy (SeqIdPtr newid, SeqIdPtr sourceid, Int4 from, Int4 to,
                               Uint1 strand, Boolean do_feat);


/*****************************************************************************
*
*	SeqLocCopyPart (the_segs, from, to, strand, group, first_segp, last_segp)
*      cuts out from the_segs the part from offset from to offset to
*      reverse complements resulting seqloc if strand == Seq_strand_minus
*      if (group) puts resulting intervals into a new Seq-loc (of type
*        PACKED_INT).
*      Currently this always makes intervals. Is really for segmented and
*        reference sequence extensions
*      If first_segp and last_segp are not NULL, then they are filled in with the
*        ordinal number of the source segments that remain in the copy, based
*        on SeqLocFindNext, where 1 is the first one. Thus if the third and
*        fourth segments were copied, first is 3 and last is 4. If the
*        location was reverse complemented, first is 4 and last is 3.
*
*****************************************************************************/
SeqLocPtr SeqLocCopyPart PROTO((SeqLocPtr the_segs, Int4 from, Int4 to,
         Uint1 strand, Boolean group, Int2Ptr first_segp, Int2Ptr last_segp));

/*****************************************************************************
*
* BioseqInsert (from_id, from, to, strand, to_id, pos, from_feat, to_feat,
*                                                                     do_split)
*       Inserts a copy the region "from"-"to" on "strand" of the Bioseq
*          identified by "from_id" into the Bioseq identified by "to_id"
*          before "pos".
*          To append to the end of the sequence, use APPEND_RESIDUE for "pos".
*       if from_feat = TRUE, copies the feature table from "from" and updates
*          to locations to point to the proper residues in "to_id"
*       If to_feat = TRUE, updates feature table on "to_id" as well.
*          if do_split == TRUE, then splits features in "to_id" (to_feat must
*             be TRUE as well). Otherwise expands features at insertion.
*
*       All operations are copies. "frombsp" is unchanged.
*       Insert will only occur between certain Bioseq.repr classes as below
*
*   From Bioseq.repr                      To Bioseq.repr
*   
*                          virtual       raw      segmented        map
*                       +---------------------------------------------------
*             virtual   |   length       inst      SeqLoc         length
*                       +---------------------------------------------------
*                 raw   |   error        copy      SeqLoc         error
*                       +---------------------------------------------------
*           segmented   |   error        inst      SeqLoc*        error
*                       +---------------------------------------------------
*                 map   |   error        inst*     SeqLoc         copy
*                       +---------------------------------------------------
*
*   length = changes length of "to" by length of "from"
*   error  = insertion not allowed
*   inst   = "from" instantiated as residues ("N" or "X" for virtual "from")
*   inst*  = as above, but a restriction map can instantiate other bases
*            than "N" for known restriction recognition sites.
*   copy   = copy of "from" inserted into "to"
*   SeqLoc = a SeqLoc added to "to" which points to "from". No copy of residues.
*   SeqLoc* = as above, but note that "to" points to "from" directly, not
*             what "from" itself may point to.
*   
*****************************************************************************/
Boolean BioseqInsert (SeqIdPtr from_id, Int4 from, Int4 to, Uint1 strand, SeqIdPtr to_id, Int4 pos,
            Boolean from_feat, Boolean to_feat, Boolean do_split);


/*****************************************************************************
*
*       Some defined values are provided for the Int4 values passed as
*           from, to, and pos in functions above. Defined in seqport.h
*   
*       FIRST_RESIDUE   0	(first residue of sequence)
*       LAST_RESIDUE    -1  (last residue of sequence.. interpreted as
*                              length - 1)
*       APPEND_RESIDUE  -2  (interpreted as length.. off the end of the
*                              sequence. Only for "pos" in BioseqInsert
*                              to append to the end of the sequence )
*   
*
*****************************************************************************/





/*****************************************************************************
*
*   Sequence Editing Utilties
*       Lower Level Support Routines
*
*****************************************************************************/


/*****************************************************************************
*
*   SeqFeatDelete()
*     0 = no changes made to location or product
*     1 = changes made but feature still has some location
*     2 = all of sfp->location in deleted interval
*
*   if (merge)
*      1) correct numbers > to by subtraction
*      2) do not split intervals spanning the deletion
*   else
*      1) do not change numbers > to
*      2) split intervals which span the deletions
*
*****************************************************************************/
Int2 SeqFeatDelete (SeqFeatPtr sfp, SeqIdPtr target, Int4 from, Int4 to, Boolean merge);

Boolean SeqInsertByLoc (SeqIdPtr target, Int4 offset, SeqLocPtr fragment);

/*****************************************************************************
*
*   SeqDeleteByLoc (slp, do_feat)
*   	deletes regions referenced by slp
*       if do_feat, deletes features in those regions as well
*
*****************************************************************************/
Boolean SeqDeleteByLoc (SeqLocPtr slp, Boolean do_feat);

/*****************************************************************************
*
*   SeqLocAdd()
*       creates a linked list of SeqLocs.
*       deletes double NULLs or Nulls at start (application must delete at stop)
*       Makes copies of all incoming SeqLocs
*
*****************************************************************************/
SeqLocPtr SeqLocAdd (SeqLocPtr PNTR head, SeqLocPtr slp);


/*****************************************************************************
*
*   SeqLocDelete()
*       returns altered head or NULL if nothing left.
*   sets changed=TRUE if all or part of loc is deleted
*   does NOT set changed if location coordinates are only moved
*   if (merge) then corrects coordinates upstream of to
*   else
*     splits intervals covering from-to, does not correct upstream of to
*
*****************************************************************************/
SeqLocPtr SeqLocDelete (SeqLocPtr head, SeqIdPtr target, Int4 from, Int4 to, Boolean merge, BoolPtr changed);

SeqLocPtr SeqLocSubtract (SeqLocPtr head, SeqLocPtr piece);

/*****************************************************************************
*
*   SeqEntryDelFeat(sep, id, from, to)
*       Deletes or truncates features on Bioseq (id) in the range
*       from-to, inclusive
*       
*        Moves features > to left to account for decrease in length
*
*       If sep == NULL, then calls SeqEntryFind(id) to set scope to look
*       for features.
*   
*****************************************************************************/
Boolean    SeqEntryDelFeat (SeqEntryPtr sep, SeqIdPtr sip, Int4 from, Int4 to);

/*****************************************************************************
*
*   SeqFeatCopy(new, old, from, to, strand)
*
*****************************************************************************/
Int2 SeqFeatsCopy (BioseqPtr newbsp, BioseqPtr oldbsp, Int4 from, Int4 to, Uint1 strand);

SeqLocPtr SeqLocCopyRegion(SeqIdPtr newid, SeqLocPtr head, BioseqPtr oldbsp,
    Int4 from, Int4 to, Uint1 strand, BoolPtr split);

/*****************************************************************************
*
*   IntFuzzClip()
*       returns TRUE if clipped range values
*       in all cases, adjusts and/or complements IntFuzz
*       Designed for IntFuzz on SeqLocs
*
*****************************************************************************/
void IntFuzzClip(IntFuzzPtr ifp, Int4 from, Int4 to, Uint1 strand, BoolPtr split);

/*****************************************************************************
*
*   SeqLocInsert()
*       alters "head" by inserting "len" residues before "pos" in any SeqLoc
*         on the Bioseq "target"
*       all SeqLocs not on "target" are unaltered
*       for SeqLocs on "target"
*          all SeqLocs before "pos" are unaltered
*          all SeqLocs >= "pos" are incremented by "len"
*          all SeqLocs spanning "pos"
*             if "split" == TRUE, are split into two SeqLocs, one to the
*               left of the insertion, the other to right
*             if "split" != TRUE, the SeqLoc is increased in length to cover
*               the insertion
*       returns altered head or NULL if nothing left.
*       if ("newid" != NULL) replaces "target" with "newid" whether the
*          SeqLoc is altered on not.
*
*       Usage hints:
*          1) To update a feature location on "target" when 10 residues of
*               sequence have been inserted before position 5
*          SeqFeatPtr->location = SeqLocInsert ( SeqFeatPtr->location ,
*                "target", 5, 10, TRUE, NULL);  [for some feature types
*                      you may want "split" equal FALSE]
*   
*          2) To insert the complete feature table from "source" into a
*                different Bioseq "dest" before position 20 in "dest"
*          SFP->location = SeqLocInsert(SFP->location, "source", 0, 20,
*                FALSE, "dest");
*   
*
*****************************************************************************/
SeqLocPtr SeqLocInsert (SeqLocPtr head, SeqIdPtr target, Int4 pos, Int4 len,
                                               Boolean split, SeqIdPtr newid);





#ifdef __cplusplus
}
#endif

#endif
