/*   gather.c
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
* File Name:  gather.c
*
* Author:  Jim Ostell, Jinghui Zhang, Jonathan Kans
*
* Version Creation Date:   10/7/94
*
* $Revision: 1.36 $
*
* File Description: 
*
* Modifications:  
* --------------------------------------------------------------------------
* Date     Name        Description of modification
* -------  ----------  -----------------------------------------------------
*
* $Log: gather.c,v $
 * Revision 1.36  1995/06/02  17:53:17  kans
 * add gather range to gather bioseq
 *
 * Revision 1.35  1995/06/01  21:53:55  kans
 * support for Seq-align (zjing)
 *
 * Revision 1.34  1995/05/19  15:49:37  kans
 * fixed bug in mapping minus strand intervals
 *
 * Revision 1.33  1995/05/15  21:46:05  ostell
 * added Log line
 *
*
*
*
* ==========================================================================
*/

#include <gather.h>
#include <edutil.h>
#include <subutil.h>

static Boolean NEAR GatherSeqEntryFunc PROTO((SeqEntryPtr sep, InternalGCCPtr igccp, Pointer parent, Uint2 parenttype, SeqEntryPtr prev, Boolean in_scope, Pointer PNTR prevlink));

/*****************************************************************************
*
*   SeqLocOffset(seq_loc, sfp_loc, range, offset)
*   	returns FALSE if seq_loc does not overlap sfp_loc
*   	else fills in range structure mapping sfp_loc to seq_loc
*       adds offset to final values
*       if (ends) will assure that left is always <= right
*
*****************************************************************************/
Boolean SeqLocOffset (SeqLocPtr seq_loc, SeqLocPtr sfp_loc, GatherRangePtr range, Int4 offset)
{
  Uint1    strand_loc, strand_sfp;

  if (seq_loc == NULL || sfp_loc == NULL || range == NULL) {
    return FALSE;
  }

  if (! SeqLocCompare(seq_loc, sfp_loc))
	return FALSE;

  strand_loc = SeqLocStrand(seq_loc);
  strand_sfp = SeqLocStrand(sfp_loc);

  range->l_trunc = FALSE;
  range->r_trunc = FALSE;
  range->left = GetOffsetInLoc (sfp_loc, seq_loc, SEQLOC_LEFT_END);
  if (range->left == -1) {
    range->l_trunc = TRUE;
    if (strand_loc == Seq_strand_minus) {
      range->left = SeqLocLen (seq_loc) - 1;
    } else {
      range->left = 0;
    }
  }
  range->right = GetOffsetInLoc (sfp_loc, seq_loc, SEQLOC_RIGHT_END);
  if (range->right == -1) {
    range->r_trunc = TRUE;
    if (strand_loc == Seq_strand_minus) {
      range->right = 0;
    } else {
      range->right = SeqLocLen (seq_loc) - 1;
    }
  }

  if (strand_loc == Seq_strand_minus)
	range->strand = StrandCmp(strand_sfp);
  else
	range->strand = strand_sfp;
  range->left += offset;
  range->right += offset;

  return TRUE;
}


	/** citttype is currently 0, or 1=OBJ_SEQFEAT_CIT **/

static Boolean NEAR GatherPub(InternalGCCPtr gccp, ValNodePtr vnp,
        Uint1 cittype, Uint1 ttype, Pointer tparent, Pointer PNTR prevlink, Boolean in_scope)
{
	GatherContextPtr gcp;
	GatherScopePtr gsp;
	Boolean takeit;
	Int2 LocateItem = 0;
	Uint1 thistype;

	if (vnp == NULL) return TRUE;

	gcp = &(gccp->gc);
	gsp = &(gccp->scope);

	if (cittype)
		thistype = OBJ_SEQFEAT_CIT;
	else
		thistype = OBJ_PUB;

	if (gsp->ignore[thistype])
		return TRUE;

	takeit = in_scope;

	if (gccp->locatetype == thistype)
		LocateItem = gccp->locateID;

	gcp->parentitem = tparent;
	gcp->parenttype = ttype;

	gccp->itemIDs[thistype]++;

	if (LocateItem == gccp->itemIDs[thistype])
		takeit = TRUE;
	if (takeit)
	{
		gcp->itemID = gccp->itemIDs[thistype];
		gcp->thisitem = (Pointer)vnp;
		gcp->thistype = thistype;
		gcp->prevlink = prevlink;
		if (! (*(gccp->userfunc))(gcp))
			return FALSE;
		if (LocateItem) return FALSE;
	}

	return TRUE;
}

static Boolean NEAR GatherPubSet(InternalGCCPtr gccp, ValNodePtr vnp,
        Uint1 cittype, Uint1 ttype, Pointer tparent, Pointer PNTR prevlink, Boolean in_scope)
{
	GatherContextPtr gcp;
	GatherScopePtr gsp;
	Boolean takeit;
	Int2 LocateItem = 0;
	Uint1 thistype;
	ValNodePtr vnp2;

	if (vnp == NULL) return TRUE;

	gcp = &(gccp->gc);
	gsp = &(gccp->scope);

	if (gsp->ignore[OBJ_PUB_SET])
		return TRUE;

	takeit = in_scope;

	if (gccp->locatetype == OBJ_PUB_SET)
		LocateItem = gccp->locateID;

	gcp->previtem = NULL;
	gcp->prevtype = OBJ_PUB_SET;
	gcp->parentitem = tparent;
	gcp->parenttype = ttype;
	thistype = OBJ_PUB_SET;

	gccp->itemIDs[OBJ_PUB_SET]++;
	if (LocateItem == gccp->itemIDs[OBJ_PUB_SET])
		takeit = TRUE;
	if (takeit)
	{
		gcp->itemID = gccp->itemIDs[OBJ_PUB_SET];
		gcp->thisitem = (Pointer)vnp;
		gcp->thistype = thistype;
		gcp->prevlink = prevlink;
		if (! (*(gccp->userfunc))(gcp))
			return FALSE;
		if (LocateItem) return FALSE;
	}

	gcp->indent++;
	prevlink = &(vnp->data.ptrvalue);
	gcp->previtem = NULL;
	gcp->prevtype = OBJ_PUB;

	for (vnp2 = (ValNodePtr)(vnp->data.ptrvalue); vnp2 != NULL; vnp2 = vnp2->next)
	{
		if (! GatherPub(gccp, vnp2, cittype, thistype, (Pointer)vnp,
						    prevlink, in_scope))
			return FALSE;
		prevlink = (Pointer PNTR)&(vnp2->next);
		gcp->previtem = (Pointer)vnp2;
	
	}
	gcp->indent--;

	return TRUE;
}

static Boolean NEAR GatherSeqIds(InternalGCCPtr gccp, SeqIdPtr sip,
        Uint1 ttype, Pointer tparent, Pointer PNTR prevlink)
{
	GatherContextPtr gcp;
	GatherScopePtr gsp;
	Boolean takeit;
	Int2 LocateItem = 0;
	Uint1 thistype;

	if (sip == NULL) return TRUE;

	gcp = &(gccp->gc);
	gsp = &(gccp->scope);

	if (gsp->ignore[OBJ_SEQID])
		return TRUE;

	if (gccp->locatetype == OBJ_SEQID)
		LocateItem = gccp->locateID;
	else
		takeit = TRUE;

	gcp->previtem = NULL;
	gcp->prevtype = OBJ_SEQID;
	gcp->parentitem = tparent;
	gcp->parenttype = ttype;
	thistype = OBJ_SEQID;

	while (sip != NULL)
	{
		gccp->itemIDs[OBJ_SEQID]++;
		if (LocateItem == gccp->itemIDs[OBJ_SEQID])
			takeit = TRUE;
		if (takeit)
		{
			gcp->itemID = gccp->itemIDs[OBJ_SEQID];
			gcp->thisitem = (Pointer)sip;
			gcp->thistype = thistype;
			gcp->prevlink = prevlink;
			if (! (*(gccp->userfunc))(gcp))
				return FALSE;
			if (LocateItem) return FALSE;
		}

		gcp->previtem = (Pointer)sip;
		prevlink = (Pointer PNTR)&(sip->next);
		sip = sip->next;
	}

	return TRUE;
}

static Boolean NEAR GatherSeqDescr(InternalGCCPtr gccp, ValNodePtr vnp,
       Uint1 ttype, Pointer tparent, Pointer PNTR prevlink, Boolean in_scope)
{
	GatherContextPtr gcp;
	GatherScopePtr gsp;
	Boolean takeit;
	Int2 LocateItem = 0;
	Uint1 thistype;

	if (vnp == NULL) return TRUE;

	gcp = &(gccp->gc);
	gsp = &(gccp->scope);

	if (gsp->ignore[OBJ_SEQDESC])
		return TRUE;

	if (gccp->locatetype == OBJ_SEQDESC)
		LocateItem = gccp->locateID;

	if (LocateItem)   /* fetching an item */
	{
		takeit = FALSE;
	}
	else
	{
		takeit = in_scope;       /* if ! in_scope don't take it */
		if (gccp->bsp != NULL)   /* gsp->target set a Bioseq */
		{
			if (tparent != (Pointer)(gccp->bsp))
			{
				if (! ObjMgrIsChild(tparent, (Pointer)(gccp->bsp)))
					takeit = FALSE;   /* not in propagation path */
				else
					gcp->propagated = TRUE;
			}
		}
	}

	gcp->previtem = NULL;
	gcp->prevtype = OBJ_SEQDESC;
	gcp->parentitem = tparent;
	gcp->parenttype = ttype;
	thistype = OBJ_SEQDESC;

	while (vnp != NULL)
	{
		gccp->itemIDs[OBJ_SEQDESC]++;
		if (LocateItem == gccp->itemIDs[OBJ_SEQDESC])
			takeit = TRUE;
		if (takeit)
		{
			gcp->itemID = gccp->itemIDs[OBJ_SEQDESC];
			gcp->thisitem = (Pointer)vnp;
			gcp->thistype = thistype;
			gcp->prevlink = prevlink;
			if (! (*(gccp->userfunc))(gcp))
				return FALSE;
			if (LocateItem) return FALSE;
		}

		gcp->previtem = (Pointer)vnp;
		prevlink = (Pointer PNTR)&(vnp->next);
		vnp = vnp->next;
	}
	gcp->propagated = FALSE;   /* reset propagated flag */

	return TRUE;
}


static Int4 get_site_offset(SeqLocPtr slp, SeqLocPtr head, Int4 r_len)
{
	Uint1 m_strand, s_strand;

	m_strand = SeqLocStrand(slp);
	s_strand = SeqLocStrand(slp);

	if(m_strand == 0 || s_strand ==0)
		return 0;
	if(m_strand == 3 || s_strand == 3)
		return 0;

	if(m_strand == s_strand)
		return 0;

	if(m_strand == Seq_strand_plus && s_strand == Seq_strand_minus)
		return (-r_len);
	else
		return r_len;
}


static Boolean process_packed_pnt(SeqLocPtr slp, SeqLocPtr head, Int4 r_len, Int4 offset, GatherContextPtr gcp, Int2Ptr max_interval)
{
        PackSeqPntPtr pspp;
	Int4 site;
	Int4 site_offset;	/*for treating restriction site as an interval*/
	Int2 index;
	Int4 m_start, m_stop;
	Uint1 m_strand;
	Int4 min, max;
	Boolean rev;
	Int4 pos, ctr, i;
	GatherRangePtr trdp, lrdp;
	GatherRange trange;
	Boolean is_end = FALSE;

	if(head->choice !=SEQLOC_PACKED_PNT)
		return FALSE;

	if(!SeqIdForSameBioseq(SeqLocId(slp), SeqLocId(head)))
		return FALSE;
	m_strand = SeqLocStrand(slp);
	site_offset = get_site_offset(slp, head, (r_len-1));
	rev = (SeqLocStrand(slp) == Seq_strand_minus);
	lrdp = gcp->rdp;


	m_start = SeqLocStart(slp);
	m_stop = SeqLocStop(slp);
	pspp = head->data.ptrvalue;
        site =0;
        index =0;
	min = -1;
	max = -1;
	ctr = 0;
        while( !is_end && ((site = PackSeqPntGet(pspp, index))!= -1))
        {
                ++index;
		if (ctr >= (*max_interval))
		{
			trdp = lrdp;
			lrdp = (GatherRangePtr)MemNew((size_t)((*max_interval + 20) * sizeof(GatherRange)));
			MemCopy(lrdp, trdp, (size_t)(*max_interval * sizeof(GatherRange)));
			MemFree(trdp);
			*max_interval += 20;
			gcp->rdp = lrdp;
		}
		is_end = (site > m_stop);
                if(site >= m_start && site <=m_stop)
                {
			site += site_offset;
                        if(rev)
				pos = offset + (m_stop - site);
			else
				pos = offset + (site - m_start);
			if(max == -1)
			{
				max = pos;
				min = pos;
			}
			else
			{
				max = MAX(pos, max);
				min = MIN(pos, min);
			}
			lrdp[ctr].left = pos;
			lrdp[ctr].right = pos;
			lrdp[ctr].l_trunc = FALSE;
			lrdp[ctr].r_trunc = FALSE;
			lrdp[ctr].strand = m_strand;
			++ctr;
		}
	}

	if (ctr)     /* got some */
	{
		gcp->extremes.left = min;
		gcp->extremes.right = max;
		gcp->extremes.l_trunc = FALSE;
		gcp->extremes.r_trunc = FALSE;
		gcp->extremes.strand = m_strand;

		gcp->num_interval = (Int2)ctr;
		if (rev)    /* reverse order on rev location */
		{
			i = 0;
			ctr--;
			while (i < ctr)
			{
				MemCopy(&trange, &(lrdp[i]), sizeof(GatherRange));
				MemCopy(&(lrdp[i]), &(lrdp[ctr]), sizeof(GatherRange));
				MemCopy(&(lrdp[ctr]), &trange, sizeof(GatherRange));
				i++; ctr++;
			}
		}
		return TRUE;
	}
	else
		return FALSE;
}


static Boolean NEAR GatherSeqFeat(InternalGCCPtr gccp, SeqFeatPtr sfp,
           Uint1 ttype, Pointer tparent, Pointer PNTR prevlink, Boolean in_scope, Uint1 sfptype)
{
	GatherContextPtr gcp;
	GatherScopePtr gsp;
	Boolean takeit=TRUE,
		takecit, checkseq=FALSE;
	SeqLocPtr slp, head, tslp, target[2];
	GatherRangePtr rdp, trdp, lrdp;
	Int4 offset;
	Boolean rev, revs[2];
	Int2 ctr, max_interval, i, numcheck, j;
	GatherRange trange;
	Int2 LocateItem = 0;
	Uint1 thistype;
	Boolean is_packed_pnt = FALSE;	/*is the seq-loc a packed point?*/
	Boolean stop_now;


	SeqFeatPtr prevsfp = NULL;

	if (sfp == NULL) return TRUE;

	gcp = &(gccp->gc);
	gsp = &(gccp->scope);

	if (gsp->ignore[sfptype])
		return TRUE;

	if (gccp->locatetype == sfptype)
		LocateItem = gccp->locateID;
	else
		LocateItem = 0;

	if (gsp->target != NULL)
	{
		checkseq = TRUE;
		numcheck = 1;
		target[0] = gsp->target;
		revs[0] = gccp->rev;
		if (gccp->segloc != NULL)
		{
			numcheck = 2;
			target[1] = gccp->segloc;
			revs[1] = FALSE;
		}
		rdp = &(gcp->extremes);
		offset = gsp->offset;
		max_interval = gccp->max_interval;
		lrdp = gcp->rdp;
	}

	if (gsp->ignore[OBJ_SEQFEAT_CIT])
		takecit = FALSE;
	else
		takecit = TRUE;

	gcp->prevtype = sfptype;
	gcp->parentitem = tparent;
	gcp->parenttype = ttype;
	gcp->num_interval = 0;
	thistype = sfptype;

	while (sfp != NULL)
	{
		gcp->previtem = (Pointer) prevsfp;
		gccp->itemIDs[sfptype]++;
		if (LocateItem == gccp->itemIDs[sfptype])
			in_scope = TRUE;

		gcp->itemID = gccp->itemIDs[sfptype];
		takeit = TRUE;
		stop_now = FALSE;

		if (in_scope)
		{
			gcp->thisitem = (Pointer)sfp;
			gcp->thistype = thistype;
			gcp->prevlink = prevlink;
			gcp->product = FALSE;
			head = sfp->location;
			if (checkseq)    /* find by SeqLoc overlap */
			{
				takeit = FALSE;
				is_packed_pnt = (head->choice == SEQLOC_PACKED_PNT);
				for (j = 0; ((j < numcheck) && (! takeit) && (!stop_now)); j++)
				{
					slp = target[j];
					rev = revs[j];
					if (gsp->get_feats_location)
					{
						if(is_packed_pnt)
						{
							if(process_packed_pnt(slp, head, 0, offset, gcp, &(gccp->max_interval)))
							{
								takeit = TRUE;
								stop_now= TRUE;
							}
						}
						else
							takeit = SeqLocOffset(slp, head, rdp, offset);
					}

					if ((! takeit) && (gsp->get_feats_product))
					{
						head = sfp->product;
						takeit = SeqLocOffset(slp, head, rdp, offset);
						if (takeit)
							gcp->product = TRUE;
					}

					if ((takeit) && (! gsp->nointervals) && (!stop_now))  /* map intervals in loc */
					{
						tslp = NULL;
						ctr = 0;
						while ((tslp = SeqLocFindNext(head, tslp)) != NULL)
						{
							if (ctr >= max_interval)
							{
								trdp = lrdp;
								lrdp = (GatherRangePtr)MemNew((size_t)((max_interval + 20) * sizeof(GatherRange)));
								MemCopy(lrdp, trdp, (size_t)(max_interval * sizeof(GatherRange)));
								MemFree(trdp);
								max_interval += 20;
								gccp->max_interval = max_interval;
								gcp->rdp = lrdp;
							}
							if (SeqLocOffset(slp, tslp, &(lrdp[ctr]), offset))
								ctr++;
						}
						if (ctr)     /* got some */
						{
							gcp->num_interval = ctr;
							if (rev)    /* reverse order on rev location */
							{
								i = 0;
								ctr--;
								while (i < ctr)
								{
									MemCopy(&trange, &(lrdp[i]), sizeof(GatherRange));
									MemCopy(&(lrdp[i]), &(lrdp[ctr]), sizeof(GatherRange));
									MemCopy(&(lrdp[ctr]), &trange, sizeof(GatherRange));
									i++; ctr--;
								}
							}
						}
					}
				}
			}

			if (takeit)
			{
				if (! (*(gccp->userfunc))(gcp))
					return FALSE;
				if (LocateItem) return FALSE;

				if ((sfp->cit != NULL) && (takecit))
				{
					if (! GatherPubSet(gccp, sfp->cit, 1, thistype, (Pointer)sfp,
						                                (Pointer PNTR)&(sfp->cit), in_scope))
						return FALSE;

					gcp->prevtype = thistype;
					gcp->parentitem = tparent;
					gcp->parenttype = ttype;
				}
			}
		}
		else /* out of scope */
		{

			if (sfp->cit != NULL)  /* just run the counter */
			{
				if (! GatherPubSet(gccp, sfp->cit, 1, thistype, (Pointer)sfp,
					                                (Pointer PNTR)&(sfp->cit), in_scope))
					return FALSE;

				gcp->prevtype = thistype;
				gcp->parentitem = tparent;
				gcp->parenttype = ttype;
			}
		}

		prevsfp = sfp;
		prevlink = (Pointer PNTR)&(sfp->next);
		sfp = sfp->next;
	}
	return TRUE;
}

static Uint1 align_strand_get(Uint1Ptr strands, Int2 order)
{
        if(strands == NULL)
                return 0;
        else
                return strands[order];
}

static Boolean check_reverse_strand(Uint1 loc_strand, Uint1 a_strand)
{
        if(loc_strand == 0 || a_strand ==0)
                return FALSE;
        if(loc_strand ==3 || a_strand ==3)
                return FALSE;

        return (loc_strand !=a_strand);
}

/****************************************************************************
***
*       get_align_ends(): map the two ends of the alignment
*
*****************************************************************************
***/

static void load_start_stop(Int4Ptr start, Int4Ptr stop, Int4 c_start, Int4 c_stop)
{
	if(*start == -1)
	{
		*start = c_start;
		*stop = c_stop;
	}
	else
	{
		*start = MIN(*start, c_start);
		*stop = MAX(*stop, c_stop);
	}
}

static Int2 get_master_order(SeqIdPtr ids, SeqIdPtr sip)
{
	Int2 i;
	
	for(i =0; ids!=NULL; ids = ids->next, ++i)
	{
		if(SeqIdForSameBioseq(ids, sip))
			return i;
	}
	return -1;
}

static Boolean get_align_ends(SeqAlignPtr align, SeqIdPtr id, Int4Ptr start, Int4Ptr stop, Uint1Ptr strand)
{
	Int2 i, n;
	Boolean is_found;
	Int4 c_start, c_stop;
	DenseSegPtr dsp;
	DenseDiagPtr ddp;
	StdSegPtr ssp;
	SeqLocPtr loc;
 
	*start = -1;
	*stop = -1;
	switch(align->segtype)
	{
		case 2:         /*DenseSeg*/
			dsp = (DenseSegPtr)(align->segs);
			i = get_master_order(dsp->ids, id);
			if( i == -1)
				return FALSE;
			for(n =0; n<dsp->numseg; ++n)
			{
				c_start = dsp->starts[n*(dsp->dim) +i];
				if(c_start != -1) /*check for a non-gapped region*/ 
				{
					c_stop = c_start + dsp->lens[n] -1;
					load_start_stop(start, stop, c_start, c_stop);
				}
			}
			*strand = align_strand_get(dsp->strands, i);
			return (*start != -1);
			
		case 3:
			ssp = (StdSegPtr)(align->segs);
			while(ssp)
			{
				is_found = FALSE;
				for (loc = ssp->loc; loc!=NULL && !is_found; loc=loc->next)
				{
					if(SeqIdForSameBioseq(SeqLocId(loc), id))
					{
						is_found = TRUE;
						load_start_stop(start, stop, SeqLocStart(loc), SeqLocStop(loc));
						*strand = SeqLocStrand(loc);
					}
				}
				ssp = ssp->next;
			}
			return (*start != -1);
			
		case 1:
			ddp = (DenseDiagPtr)(align->segs);
			while(ddp)
			{
				i = get_master_order(dsp->ids, id);
				if(i != -1)
				{
					c_start = ddp->starts[i];
					c_stop = ddp->starts[i] + ddp->len -1;
					*strand = align_strand_get(ddp->strands, i);
					load_start_stop(start, stop, c_start, c_stop);
				}
				ddp = ddp->next;
			}			
			return (*start != -1);
			
		default:
			return FALSE;
	}
}
	
				

/***********************************************************************
***
*       make_seq_loc(): make SeqLoc for the features.
*
************************************************************************
***/
static SeqLocPtr make_seq_loc(Int4 start, Int4 stop, Uint1 strand, SeqIdPtr sip)
{
   SeqLocPtr location;
   SeqIntPtr sint;

                sint = SeqIntNew();
                sint->id = SeqIdDup(sip);
                sint->from = start;
                sint->to = stop;
                sint->strand = strand;

                location = (SeqLocPtr)ValNodeNew(NULL);
                location->choice = SEQLOC_INT;
                location->data.ptrvalue = sint;
                location->next = NULL;

                return location;

}

static SeqLocPtr update_seq_loc(Int4 start, Int4 stop, Uint1 strand, SeqLocPtr loc)
{
   SeqIntPtr sint;
   SeqPntPtr spp;

        if(loc->choice == SEQLOC_INT)
        {
                sint = loc->data.ptrvalue;
                if(start != -1)
                        sint->from = start;
                if(stop != -1)
                        sint->to = stop;
                if(strand != 0)
                        sint->strand = strand;
                loc->data.ptrvalue = sint;
        }
        else if(loc->choice == SEQLOC_PNT)
        {
                spp = (SeqPntPtr)(loc->data.ptrvalue);
                spp->point = start;
                spp->strand = strand;
                loc->data.ptrvalue = spp;
        }

        return loc;



}

static void LinkAlignData(AlignDataPtr PNTR head, AlignDataPtr adp)
{
	AlignDataPtr curr;
	
	if(*head == NULL)
		*head = adp;
	else
	{
		curr = *head;
		while(curr->next != NULL)
			curr = curr->next;
		curr->next = adp;
	}
}

static void LinkAlignRange(AlignRangePtr PNTR head, AlignRangePtr arp)
{
	AlignRangePtr curr;
	
	if(*head == NULL)
		*head = arp;
	else
	{
		curr = *head;
		while(curr->next != NULL)
			curr = curr->next;
		curr->next = arp;
	}
}

static AlignDataPtr get_adp_node(AlignDataPtr head, SeqIdPtr sip)
{
	if(head == NULL)
		return NULL;
	while(head)
	{
		if(SeqIdMatch(sip, head->sip))
			return head;
		head = head->next;
	}
	return NULL;
}



static void load_align_data(AlignDataPtr PNTR head, Int4 start, Int4 stop, Uint1 strand, SeqIdPtr sip, Boolean rev, Int2 seg_type, GatherRange t_range, GatherRange gr, Boolean ck_interval)
{
	AlignDataPtr adp;
	AlignRangePtr arp;
	
	adp = get_adp_node(*head, sip);
	if(rev && strand != 0)
		strand = 3 - strand;
	if(adp == NULL)
	{
		adp = MemNew(sizeof(AlignData));
		adp->sip = sip;
		MemCopy((&adp->extremes), &gr, sizeof(GatherRange));
		adp->extremes.strand = strand;
		adp->seqends.start = start;
		adp->seqends.stop = stop;
		adp->seqends.strand = strand;
		adp->arp = NULL;
		adp->next = NULL;
		LinkAlignData(head, adp);
	}
	else
	{
		adp->seqends.start = MIN(start, adp->seqends.start);
		adp->seqends.stop = MAX(stop, adp->seqends.stop);
	}	
		
	if(ck_interval)
	{
		arp = MemNew(sizeof(AlignRange));
		arp->segtype = seg_type;
		MemCopy(&(arp->gr), &t_range, sizeof(GatherRange));
		arp->sr.start = start;
		arp->sr.stop = stop;
		arp->sr.strand = strand;
		LinkAlignRange(&(adp->arp), arp);
	}
}


AlignDataPtr gather_align_data(SeqLocPtr m_slp, SeqAlignPtr align, Int4 offset, Boolean ck_interval, Boolean map)
{
	SeqIdPtr m_sip, sip;
	Uint1 m_strand, strand, c_strand;
	DenseSegPtr dsp;
	StdSegPtr ssp;
	DenseDiagPtr ddp;
	Int2 i, m_order, k, numseg;
	Int2 s1;
	Uint1 seg_type;
	Boolean is_found = FALSE;
	Int4 start, stop, c_start, c_stop;
	Int4 m_start, m_stop;
	Int4 off_start, off_stop;
	Boolean rev;
	GatherRange gr, t_range;
	SeqLocPtr a_slp, loc;
	AlignDataPtr head = NULL;


 	m_sip = SeqLocId(m_slp);
 	if(!get_align_ends(align, m_sip, &start, &stop, &c_strand))
 		return NULL;
 		
	a_slp = make_seq_loc(start, stop, c_strand, m_sip);
	if(!SeqLocOffset(m_slp, a_slp, &gr, offset))
	{
		SeqLocFree(a_slp);
		return NULL;
	}

	m_strand = SeqLocStrand(m_slp);
	m_start = SeqLocStart(m_slp);
	m_stop = SeqLocStop(m_slp);
	switch(align->segtype)
 	{
		case 2: /*for DenseSegs*/
			dsp = (DenseSegPtr)(align->segs);
			m_order = get_master_order(dsp->ids, m_sip);
			rev = check_reverse_strand(m_strand, c_strand);
			if(rev)
				k = dsp->numseg -1;
			else
				k =0;

			for(numseg = 0; numseg<dsp->numseg; ++numseg)
			{
				s1 = k*(dsp->dim)+m_order;
				if(dsp->starts[s1] != -1)
				{
					c_start = dsp->starts[s1];
					c_stop = c_start + dsp->lens[k] -1;
					update_seq_loc(c_start, c_stop, 0, a_slp);
				}
				else
				{
					if(strand == Seq_strand_minus)
						update_seq_loc(c_start, c_start, 0, a_slp);
					else
						update_seq_loc(c_stop, c_stop, 0, a_slp);
				}
				if(SeqLocOffset(m_slp, a_slp, &t_range, offset))
				{
					off_start = MAX(0, (m_start - SeqLocStart(a_slp)));
					off_stop = MAX(0, (SeqLocStop(a_slp) - m_stop));
					for(sip = dsp->ids, i=0; sip!=NULL; sip = sip->next, ++i)
					{
						if(!map|| (i != m_order))
						{
							strand = align_strand_get(dsp->strands, i);
							start = dsp->starts[k*(dsp->dim) +i];
							if(start != -1)
							{
								stop = start + dsp->lens[k] -1;
								if(dsp->starts[s1] == -1)
								{
									seg_type = INS_SEG;
									t_range.right = dsp->lens[k];
								}
								else
								{
									if(check_reverse_strand(c_strand, strand))
									{
										start += off_stop;
										stop -= off_start;
									}
									else
									{
										start += off_start;
										stop -= off_stop;
									}
									seg_type = REG_SEG;
								}
							}
							else
							{
								seg_type = GAP_SEG;
								start = -1;
								stop = dsp->lens[k];
							} 
							load_align_data(&head, start, stop, strand, sip, rev, seg_type, t_range, gr, ck_interval);
						}
					}
				}
				if(rev)
					--k;
				else
					++k;
			}
			break;
		
		case 3:
			ssp = (StdSegPtr)(align->segs);
			while(ssp)
			{
				is_found = FALSE;
				for(loc= ssp->loc, i=0; loc!=NULL && !is_found;)
				{
					if(SeqLocOffset(m_slp, loc, &t_range, offset))
						is_found = TRUE;
					else
					{
						++i;
						loc = loc->next;
					}
				}
				if(is_found)
				{
					off_start = MAX(0, (m_start - SeqLocStart(loc)));
					off_stop = MAX(0, (SeqLocStop(loc) - m_stop));
					c_strand = SeqLocStrand(loc);
					rev = check_reverse_strand(c_strand, m_strand);
					for(loc = ssp->loc; loc != NULL; loc = loc->next)
					{
						sip = SeqLocId(loc);
						if(!map|| !SeqIdForSameBioseq(sip, m_sip))
						{
							start = SeqLocStart(loc);
							stop = SeqLocStop(loc);
							strand = SeqLocStrand(loc);
							if(check_reverse_strand(strand, c_strand))
							{
								start += off_stop;
								stop -= off_start;
							}
							else
							{
								start += off_start;
								stop -= off_stop;
							}
							load_align_data(&head, start, stop, strand, sip, rev, STD_SEG, t_range, gr, ck_interval);
						}
					}
				}
				ssp = ssp->next;
			}
			break;
						
		case 1:
			ddp = (DenseDiagPtr)(align->segs);
			while(ddp)
			{
				m_order = get_master_order(ddp->id, m_sip);
				if(m_order != -1)
				{
					c_strand = align_strand_get(ddp->strands, i);
					c_start = ddp->starts[m_order];
					c_stop = start + ddp->len -1;
					update_seq_loc(c_start, c_stop, 0, a_slp);
					if(SeqLocOffset(m_slp, a_slp, &t_range, offset))
					{
						rev = check_reverse_strand(m_strand, c_strand);
						off_start = MAX(0, (m_start-c_start));
						off_stop = MAX(0, (c_stop - m_stop));
						for(sip = ddp->id, i=0; sip!=NULL; sip = sip->next, ++i)
						{
							if(!map|| i != m_order)
							{
								start = ddp->starts[i];
								stop = start + ddp->len -1;
								strand = align_strand_get(ddp->strands, i);
								if(check_reverse_strand(strand, c_strand))
								{
									start += off_stop;
									stop -= off_start;
								}
								else
								{
									start += off_start;
									stop -= off_stop;
								}
								load_align_data(&head, start, stop, strand, sip, rev, DIAG_SEG, t_range, gr, ck_interval);
							}
						}
					}
				}
				ddp = ddp->next;
			}
			break;
		default:
			break;
	}
	
	SeqLocFree(a_slp);
	return head;
			

}

					

static Boolean NEAR GatherSeqAlign(InternalGCCPtr gccp, SeqAlignPtr sap,
        Uint1 ttype, Pointer tparent, Pointer PNTR prevlink, Boolean in_scope)
{
	GatherContextPtr gcp;
	GatherScopePtr gsp;
	Int2 LocateItem = 0;
	Uint1 thistype;
        SeqLocPtr slp, target[2];
        Boolean takeit=TRUE, checkseq=FALSE;
	Int2 j, numcheck=0;
        Int4 offset = 0;
	Boolean check_interval;
	AlignDataPtr adp;


	if (sap == NULL) return TRUE;

	gcp = &(gccp->gc);
	gsp = &(gccp->scope);

	if (gsp->ignore[OBJ_SEQALIGN])
		return TRUE;

	if (gccp->locatetype == OBJ_SEQALIGN)
		LocateItem = gccp->locateID;

	thistype = OBJ_SEQALIGN;
	gcp->previtem = NULL;
	gcp->prevtype = thistype;
	gcp->parentitem = tparent;
	gcp->parenttype = ttype;


        if (gsp->target != NULL)
        {
                checkseq = TRUE;
                numcheck = 1;
                target[0] = gsp->target;
                if (gccp->segloc != NULL)
                {
                        numcheck = 2;
                        target[1] = gccp->segloc;
                }
                offset = gsp->offset;
	}

	check_interval = (gsp->nointervals == FALSE);
	while (sap != NULL)
	{
		gccp->itemIDs[OBJ_SEQALIGN]++;
		if (LocateItem == gccp->itemIDs[OBJ_SEQALIGN])
			in_scope = TRUE;

		if (in_scope)
		{
			gcp->itemID = gccp->itemIDs[OBJ_SEQALIGN];
			gcp->prevlink = prevlink;
			gcp->thisitem = (Pointer)sap;
			gcp->thistype = thistype;
			if (checkseq)
			{
				takeit = FALSE;
				for (j =0; ((j<numcheck) && (!takeit)); j++)
				{
					slp = target[j];
					FreeAlignData(gcp->adp);
					gcp->adp = NULL;
					adp = gather_align_data(slp, sap, offset, check_interval, TRUE);
					if(adp == NULL)
						takeit = FALSE;
					else
					{
						takeit = TRUE;
						gcp->adp = adp;
					}
					
				}
			}
			if(takeit)
				if (! (*(gccp->userfunc))(gcp))
					return FALSE;
			if (LocateItem) return FALSE;
		}

		gcp->previtem = (Pointer)sap;
		prevlink = (Pointer PNTR)&(sap->next);
		sap = sap->next;
	}
	return TRUE;
}

static Boolean NEAR GatherSeqGraph(InternalGCCPtr gccp, SeqGraphPtr sgp,
        Uint1 ttype, Pointer tparent, Pointer PNTR prevlink, Boolean in_scope)
{
	GatherContextPtr gcp;
	GatherScopePtr gsp;
	Int2 LocateItem = 0;
	Uint1 thistype;

	if (sgp == NULL) return TRUE;

	gcp = &(gccp->gc);
	gsp = &(gccp->scope);

	if (gsp->ignore[OBJ_SEQGRAPH])
		return TRUE;

	LocateItem = gccp->locateID;

	thistype = OBJ_SEQGRAPH;
	gcp->previtem = NULL;
	gcp->prevtype = thistype;
	gcp->parentitem = tparent;
	gcp->parenttype = ttype;

	while (sgp != NULL)
	{
		gccp->itemIDs[OBJ_SEQGRAPH]++;
		if (LocateItem == gccp->itemIDs[OBJ_SEQGRAPH])
			in_scope = TRUE;

		if (in_scope)
		{
			gcp->itemID = gccp->itemIDs[OBJ_SEQGRAPH];
			gcp->thisitem = (Pointer)sgp;
			gcp->thistype = thistype;
			gcp->prevlink = prevlink;
			if (! (*(gccp->userfunc))(gcp))
				return FALSE;
			if (LocateItem) return FALSE;
		}

		gcp->previtem = (Pointer)sgp;
		prevlink = (Pointer PNTR)&(sgp->next);
		sgp = sgp->next;
	}
	return TRUE;
}

static Boolean NEAR GatherSeqAnnot(InternalGCCPtr gccp, SeqAnnotPtr sap,
        Uint1 ttype, Pointer tparent, Pointer PNTR prevlink, Boolean in_scope)
{
	GatherContextPtr gcp;
	GatherScopePtr gsp;
	SeqAnnotPtr prevsap = NULL;
	Int2 LocateItem = 0;
	Uint1 thistype;

	if (sap == NULL) return TRUE;

	gcp = &(gccp->gc);
	gsp = &(gccp->scope);

	if (gsp->ignore[OBJ_SEQANNOT])
		return TRUE;

	if (gccp->locatetype == OBJ_SEQANNOT)
		LocateItem = gccp->locateID;
	else
		LocateItem = 0;

	while (sap != NULL)
	{
		thistype = OBJ_SEQANNOT;
		gccp->itemIDs[OBJ_SEQANNOT]++;
		if (LocateItem == gccp->itemIDs[OBJ_SEQANNOT])
			in_scope = TRUE;

		if (in_scope)
		{
			gcp->previtem = (Pointer) prevsap;
			gcp->prevtype = thistype;
			gcp->parentitem = tparent;
			gcp->parenttype = ttype;
			gcp->thisitem = (Pointer)sap;
			gcp->thistype = thistype;
			gcp->prevlink = prevlink;
			gcp->itemID = gccp->itemIDs[OBJ_SEQANNOT];
			if (! (*(gccp->userfunc))(gcp))
				return FALSE;
			if (LocateItem) return FALSE;
		}

		gcp->indent++;
		switch (sap->type)
		{
			case 1:     /* feature table */
				if (! GatherSeqFeat(gccp, (SeqFeatPtr)(sap->data), thistype, (Pointer)sap, &(sap->data), in_scope, OBJ_SEQFEAT))
					return FALSE;
				break;
			case 2:     /* alignments */
				if (! GatherSeqAlign(gccp, (SeqAlignPtr)(sap->data), thistype, (Pointer)sap, &(sap->data), in_scope))
					return FALSE;
				break;
			case 3:     /* graphs */
				if (! GatherSeqGraph(gccp, (SeqGraphPtr)(sap->data), thistype, (Pointer)sap, &(sap->data), in_scope))
					return FALSE;
				break;

		}
		gcp->indent--;

		prevsap = sap;
		prevlink = (Pointer PNTR)&(sap->next);
		sap = sap->next;
	}
	return TRUE;
}

static Boolean NEAR GatherBioseqFunc (InternalGCCPtr gccp, BioseqPtr bsp,
         Pointer parent, Uint2 parenttype, SeqEntryPtr prev,
	     Pointer PNTR prevlink, SeqEntryPtr curr, Boolean in_scope)
{
	GatherContextPtr gcp;
	GatherScopePtr gsp;
	Boolean takeit, in_range=TRUE, rev, free_seg, trunc_l, trunc_r;
	Int2 LocateItem = 0, segctr, first_seg, last_seg;
	ValNode vn;
	SeqLocPtr head, slp, target=NULL, tslp, segloc;
	SeqLocPtr targets[2];
	Int4 offset, toffset, seglen, tlen;
	GatherRangePtr rdp;
	Uint1 thistype;
	Int2 j, numcheck;

	if (bsp == NULL)
		return TRUE;

	gcp = &(gccp->gc);
	gsp = &(gccp->scope);

	takeit = in_scope;
	head = NULL;      
	if (gsp->ignore[OBJ_BIOSEQ])
		takeit = FALSE;
	else if ((takeit) && (gsp->target != NULL))
	{
		if (bsp != gccp->bsp)
			takeit = FALSE;
	}

	gccp->itemIDs[OBJ_BIOSEQ]++;
	gcp->itemID = gccp->itemIDs[OBJ_BIOSEQ];
	if (gccp->locatetype == OBJ_BIOSEQ)
	{
		LocateItem = gccp->locateID;
		if (LocateItem == gccp->itemIDs[OBJ_BIOSEQ])
			takeit = TRUE;
	}

	thistype = OBJ_BIOSEQ;
	if (takeit)
	{
		gcp->sep = curr;
		gcp->previtem = prev;
		gcp->prevtype = OBJ_SEQENTRY;
		gcp->parentitem = parent;
		gcp->parenttype = parenttype;
		gcp->thistype = thistype;
		gcp->thisitem = (Pointer)bsp;
		gcp->prevlink = prevlink;

        	if (gsp->target != NULL)
        	{
			vn.choice = SEQLOC_WHOLE;
			vn.data.ptrvalue = bsp->id;
                	numcheck = 1;
                	targets[0] = gsp->target;
                	if (gccp->segloc != NULL)
                	{
                        	numcheck = 2;
                        	targets[1] = gccp->segloc;
                	}
			for (j =0; j<numcheck; j++)
			{
				if(SeqLocOffset(targets[j], &vn, &(gcp->extremes), gsp->offset))
					break;
			}
		}

		if (! (*(gccp->userfunc))(gcp))
			return FALSE;
		if (LocateItem) return FALSE;
	}

	gcp->indent++;
	if ((bsp->repr == Seq_repr_map) && (! gsp->ignore[OBJ_BIOSEQ_MAPFEAT]))
	{
		if (! GatherSeqFeat(gccp, (SeqFeatPtr)(bsp->seq_ext), thistype, (Pointer)bsp, &(bsp->seq_ext), in_scope, OBJ_BIOSEQ_MAPFEAT))
			return FALSE;
	}


	if (! gsp->ignore[OBJ_BIOSEQ_SEG])
	{
		if (bsp->repr == Seq_repr_seg)
		{
			vn.next = NULL;
			vn.choice = SEQLOC_MIX;
			vn.data.ptrvalue = bsp->seq_ext;
			head = &vn;
		}
		else if (bsp->repr == Seq_repr_ref)
			head = (SeqLocPtr)(bsp->seq_ext);
		else
			head = NULL;

		if (head != NULL)
		{
			segloc = NULL;
			segctr = 0;
			rev = gccp->rev;
			free_seg = FALSE;
			rdp = &(gcp->extremes);

			if (gsp->target != NULL)   /* may have to map */
			{
				if (gccp->segloc != NULL)
				{
					segloc = gccp->segloc;
					first_seg = gccp->first_seg;
					last_seg = gccp->last_seg;
				}
				else
				{
					segloc = SeqLocCopyPart(head, SeqLocStart(gsp->target),
					SeqLocStop(gsp->target), SeqLocStrand(gsp->target),
					TRUE, &(first_seg), &(last_seg));
				    free_seg = TRUE;
				}

				toffset = SeqLocStart(gsp->target); /* partial first seg */
				if (toffset)
					trunc_l = TRUE;
				else
					trunc_l = FALSE;
				if ((toffset + SeqLocLen(gsp->target)) < BioseqGetLen(bsp))
					trunc_r = TRUE;
				else
					trunc_r = FALSE;

				tlen = SeqLocLen(segloc);

				if (rev)
				{
					segctr = first_seg;
					last_seg = first_seg;
					first_seg = segctr;
					segctr = 0;
				}

			}

			if (gccp->locatetype == OBJ_BIOSEQ_SEG)
				LocateItem = gccp->locateID;

			gcp->previtem = NULL;
			gcp->prevtype = 0;
			gcp->parentitem = (Pointer)bsp;
			gcp->parenttype = thistype;
			gcp->thistype = OBJ_BIOSEQ_SEG;
			gcp->prevlink = (Pointer PNTR)&(bsp->seq_ext);
			offset = gsp->offset;

			slp = NULL;
			tslp = NULL;
			rdp->l_trunc = FALSE;
			rdp->r_trunc = FALSE;
			while ((slp = SeqLocFindNext(head, slp)) != NULL)
			{
				seglen = SeqLocLen (slp);
				segctr++;

				gccp->itemIDs[OBJ_BIOSEQ_SEG]++;
				if (gccp->locatetype == OBJ_BIOSEQ_SEG)
				{
					if (LocateItem == gccp->itemIDs[OBJ_BIOSEQ_SEG])
						takeit = TRUE;
				}

				if (takeit)
				{
					if (segloc != NULL)	   /* adjust to target? */
					{
						if ((first_seg > segctr) || (last_seg < segctr))
							in_range = FALSE;
						else
						{
							in_range = TRUE;
							tslp = SeqLocFindNext(segloc, tslp);

							rdp->l_trunc = FALSE;
							rdp->r_trunc = FALSE;

							if (segctr == first_seg)
							{
								offset = gsp->offset; /* allow for partial */
								if (rev)
								{
								   if (trunc_r)
									rdp->l_trunc = TRUE;
								}
								else
								{
									if (trunc_l)
										rdp->l_trunc = TRUE;
								}
							}
							else if (segctr == last_seg)
							{
								if (rev)
								{
								   if (trunc_l)
									rdp->r_trunc = TRUE;
								}
								else
								{
									if (trunc_r)
										rdp->r_trunc = TRUE;
								}
							}

							seglen = SeqLocLen(tslp);

							if (rev)
							{
								rdp->right = offset + tlen - 1;
								rdp->left = rdp->right - seglen + 1;
							}
							else
							{
								rdp->left = offset;
								rdp->right = offset + seglen - 1;
							}
							rdp->strand = SeqLocStrand(tslp);
							tlen -= seglen;
						}
					}
					else
					{
						rdp->left = offset;
						rdp->right = offset + seglen - 1;
						rdp->strand = SeqLocStrand(slp);
					}

					if (in_range)  /* always in_range if no target */
					{
						gcp->thisitem = (Pointer)(slp);
						gcp->itemID = gccp->itemIDs[OBJ_BIOSEQ_SEG];
		
						if ((! (*(gccp->userfunc))(gcp)) || (LocateItem))
						{
							if (free_seg)
								SeqLocFree(segloc);
							return FALSE;
						}
					}
				}
				gcp->prevlink = (Pointer PNTR)&(slp->next);
				gcp->previtem = (Pointer)slp;
				gcp->prevtype = OBJ_BIOSEQ_SEG;
				offset += seglen;
			}

			if (free_seg)
				SeqLocFree(segloc);
		}
	}

	if ((bsp->hist != NULL) && (! gsp->ignore[OBJ_SEQHIST]))
	{
		gccp->itemIDs[OBJ_SEQHIST]++;
		if (gccp->locatetype == OBJ_SEQHIST)
		{
			LocateItem = gccp->locateID;
			if (LocateItem == gccp->itemIDs[OBJ_SEQHIST])
				in_scope = TRUE;
		}

		if (in_scope)
		{
			gcp->previtem = NULL;
			gcp->prevtype = 0;
			gcp->parentitem = (Pointer)bsp;
			gcp->parenttype = thistype;
			gcp->thisitem = (Pointer)(bsp->hist);
			gcp->thistype = OBJ_SEQHIST;
			gcp->prevlink = (Pointer PNTR)&(bsp->hist);
			gcp->itemID = gccp->itemIDs[OBJ_SEQHIST];

			if (! (*(gccp->userfunc))(gcp))
				return FALSE;
			if (LocateItem) return FALSE;
			if((bsp->hist->assembly!=NULL) && (!gsp->ignore[OBJ_SEQALIGN]))
				GatherSeqAlign(gccp,(SeqAlignPtr)(bsp->hist->assembly), thistype, (Pointer)bsp,(Pointer PNTR)&(bsp->hist->assembly), in_scope);
		}
	}

	if (! GatherSeqDescr(gccp, bsp->descr, thistype, (Pointer)bsp,
		                   (Pointer PNTR)&(bsp->descr), in_scope))
		return FALSE;

	if (! GatherSeqAnnot(gccp, bsp->annot, thistype, (Pointer)bsp,
		                   (Pointer PNTR)&(bsp->annot), in_scope))
		return FALSE;

	gcp->indent--;

	return TRUE;
}

static Boolean NEAR GatherBioseqSetFunc (InternalGCCPtr gccp, BioseqSetPtr bsp,
           Pointer parent, Uint2 parenttype, SeqEntryPtr prev, Boolean in_scope,
		   Pointer PNTR prevlink, SeqEntryPtr curr)
{
	GatherContextPtr gcp;
	GatherScopePtr gsp;
	Boolean takeit=TRUE, tscope, checkscope;
	SeqEntryPtr sep, prevsep = NULL, scope = NULL;
	Int2 LocateItem = 0;
	Uint1 thistype;

	if (bsp == NULL)
		return TRUE;

	gcp = &(gccp->gc);
	gsp = &(gccp->scope);

	if (gsp->ignore[OBJ_BIOSEQSET])
		takeit = FALSE;
	else if (! in_scope)
		takeit = FALSE;

	scope = gsp->scope;
	if ((scope != NULL) && (! in_scope))
		checkscope = TRUE;
	else
		checkscope = FALSE;
		
	gccp->itemIDs[OBJ_BIOSEQSET]++;
	gcp->itemID = gccp->itemIDs[OBJ_BIOSEQSET];

	if (gccp->locatetype == OBJ_BIOSEQSET)
	{
		if (gccp->locateID == gccp->itemIDs[OBJ_BIOSEQSET])
		{
			takeit = TRUE;
			LocateItem = gccp->locateID;
		}
	}

	thistype = OBJ_BIOSEQSET;
	if (takeit)
	{
		gcp->sep = curr;
		gcp->previtem = prev;
		gcp->prevtype = OBJ_SEQENTRY;
		gcp->parentitem = parent;
		gcp->parenttype = parenttype;
		gcp->thisitem = (Pointer)bsp;
		gcp->thistype = thistype;
		gcp->prevlink = prevlink;


		if (! (*(gccp->userfunc))(gcp))
			return FALSE;
		if (LocateItem) return FALSE;
	}
	gcp->indent++;
	if (! GatherSeqDescr(gccp, bsp->descr, thistype, (Pointer)bsp,
		               (Pointer PNTR)&(bsp->descr), in_scope))
		return FALSE;

	if (! GatherSeqAnnot(gccp, bsp->annot, thistype, (Pointer)bsp,
	               (Pointer PNTR)&(bsp->annot), in_scope))
		return FALSE;

	tscope = in_scope;
	prevlink = (Pointer PNTR)&(bsp->seq_set);
	for (sep = bsp->seq_set; sep != NULL; sep = sep->next)
	{
		gcp->previtem = prevsep;
		gcp->prevtype = OBJ_SEQENTRY;
		gcp->parentitem = (Pointer)bsp;
		gcp->parenttype = thistype;
		gcp->prevlink = prevlink;

		if (checkscope)
		{
			if (sep == scope)
				tscope = TRUE;
			else
				tscope = FALSE;
		}

		if (! GatherSeqEntryFunc(sep, gccp, (Pointer)bsp, OBJ_BIOSEQSET, prevsep, tscope, prevlink))
			return FALSE;

		if (checkscope)
		{
			if (tscope == TRUE)  /* just found it */
			{
				checkscope = FALSE;   /* don't look anymore */
				tscope = FALSE;       /* the siblings not in scope */
				gsp->scope = NULL;    /* no more to look */
			}
			else if (gsp->scope == NULL)   /* found lower down */
			{
				checkscope = FALSE;
				tscope = FALSE;
			}
		}

		prevsep = sep;
		prevlink = (Pointer PNTR)&(sep->next);
	}

	gcp->indent--;  /* reset to original indent level */
	return TRUE;
}

static Boolean NEAR GatherSeqEntryFunc (SeqEntryPtr sep, InternalGCCPtr igccp,
           Pointer parent, Uint2 parenttype, SeqEntryPtr prev, Boolean in_scope,
		   Pointer PNTR prevlink)
{
	if (sep == NULL)
		return TRUE;

	if (IS_Bioseq(sep))
	{
		if (! GatherBioseqFunc(igccp, (BioseqPtr)(sep->data.ptrvalue), parent, parenttype, prev, prevlink, sep, in_scope))
			return FALSE;
	}
	else
	{
		if (! GatherBioseqSetFunc(igccp, (BioseqSetPtr)(sep->data.ptrvalue), parent, parenttype, prev, in_scope, prevlink, sep))
			return FALSE;
	}

	return TRUE;
}

static Boolean NEAR GatherSeqSubCit(InternalGCCPtr gccp, CitSubPtr csp,
        Uint1 ttype, Pointer tparent, Pointer PNTR prevlink)
{
	GatherContextPtr gcp;
	GatherScopePtr gsp;
	Boolean doit = TRUE;
	Int2 LocateItem = 0;

	if (csp == NULL) return TRUE;

	gcp = &(gccp->gc);
	gsp = &(gccp->scope);

	if (gsp->ignore[OBJ_SEQSUB_CIT])
		return TRUE;

	gccp->itemIDs[OBJ_SEQSUB_CIT]++;

	if (gccp->locatetype == OBJ_SEQSUB_CIT)
	{
		LocateItem = gccp->locateID;
		if (gccp->itemIDs[OBJ_SEQSUB_CIT] != LocateItem)
			doit = FALSE;
	}

	gcp->previtem = NULL;
	gcp->prevtype = 0;
	gcp->parentitem = tparent;
	gcp->parenttype = ttype;
	gcp->thisitem = (Pointer)csp;
	gcp->thistype = OBJ_SEQSUB_CIT;
	gcp->prevlink = prevlink;
	gcp->itemID = gccp->itemIDs[OBJ_SEQSUB_CIT];

	if (doit)
	{
		if (! (*(gccp->userfunc))(gcp))
			return FALSE;
		if (LocateItem) return FALSE;
	}

	return TRUE;
}

static Boolean NEAR GatherSeqSubContact(InternalGCCPtr gccp, ContactInfoPtr cip,
        Uint1 ttype, Pointer tparent, Pointer PNTR prevlink)
{
	GatherContextPtr gcp;
	GatherScopePtr gsp;
	Int2 LocateItem = 0;
	Boolean doit = TRUE;

	if (cip == NULL) return TRUE;

	gcp = &(gccp->gc);
	gsp = &(gccp->scope);

	if (gsp->ignore[OBJ_SEQSUB_CONTACT])
		return TRUE;

	gccp->itemIDs[OBJ_SEQSUB_CONTACT]++;

	if (gccp->locatetype == OBJ_SEQSUB_CONTACT)
	{
		LocateItem = gccp->locateID;
		if (gccp->itemIDs[OBJ_SEQSUB_CONTACT] != LocateItem)
			doit = FALSE;
	}

	gcp->previtem = NULL;
	gcp->prevtype = 0;
	gcp->parentitem = tparent;
	gcp->parenttype = ttype;
	gcp->thisitem = (Pointer)cip;
	gcp->thistype = OBJ_SEQSUB_CONTACT;
	gcp->prevlink = prevlink;
	gcp->itemID = gccp->itemIDs[OBJ_SEQSUB_CONTACT];

	if (doit)
	{
		if (! (*(gccp->userfunc))(gcp))
			return FALSE;
		if (LocateItem) return FALSE;
	}

	return TRUE;
}

static Boolean NEAR GatherSubBlock(InternalGCCPtr gccp, SubmitBlockPtr sbp,
        Uint1 ttype, Pointer tparent, Pointer PNTR prevlink)
{
	GatherContextPtr gcp;
	GatherScopePtr gsp;
	Int2 LocateItem = 0;
	Boolean doit = TRUE;

	if (sbp == NULL) return TRUE;

	gcp = &(gccp->gc);
	gsp = &(gccp->scope);

	if (gsp->ignore[OBJ_SUBMIT_BLOCK])
		return TRUE;

	gccp->itemIDs[OBJ_SUBMIT_BLOCK]++;

	if (gccp->locatetype != 0) {
		doit = FALSE;
		if (gccp->locatetype == OBJ_SUBMIT_BLOCK)
		{
			LocateItem = gccp->locateID;
			if (gccp->itemIDs[OBJ_SUBMIT_BLOCK] == LocateItem)
				doit = TRUE;
		}
	}

	gcp->previtem = NULL;
	gcp->prevtype = 0;
	gcp->parentitem = tparent;
	gcp->parenttype = ttype;
	gcp->thisitem = (Pointer)sbp;
	gcp->thistype = OBJ_SUBMIT_BLOCK;
	gcp->prevlink = prevlink;
	gcp->itemID = gccp->itemIDs[OBJ_SUBMIT_BLOCK];

	if (doit)
	{
		if (! (*(gccp->userfunc))(gcp))
			return FALSE;
		if (LocateItem) return FALSE;
	}

	gcp->indent++;

	if (! GatherSeqSubContact(gccp, sbp->contact, OBJ_SUBMIT_BLOCK,
										(Pointer)sbp, (Pointer PNTR)&(sbp->contact)))
		return FALSE;

	if (! GatherSeqSubCit(gccp, sbp->cit, OBJ_SUBMIT_BLOCK,
										(Pointer)sbp, (Pointer PNTR)&(sbp->cit)))
		return FALSE;

    gcp->indent--;
	return TRUE;
}


static Boolean NEAR GatherSeqSubmit (InternalGCCPtr gccp, SeqSubmitPtr ssp, Boolean in_scope)
{
	GatherContextPtr gcp;
	GatherScopePtr gsp;
	Boolean takeit=TRUE, tscope, checkscope;
	SeqEntryPtr sep, prevsep = NULL, scope = NULL;
	Int2 LocateItem = 0;
	Uint1 thistype;
	Pointer PNTR prevlink;

	if (ssp == NULL)
		return TRUE;

	gcp = &(gccp->gc);
	gsp = &(gccp->scope);

	if (gsp->ignore[OBJ_SEQSUB])
		takeit = FALSE;
	else if (! in_scope)
		takeit = FALSE;

	scope = gsp->scope;
	if ((scope != NULL) && (! in_scope))
		checkscope = TRUE;
	else
		checkscope = FALSE;
		
	gccp->itemIDs[OBJ_SEQSUB]++;
	gcp->itemID = gccp->itemIDs[OBJ_SEQSUB];

	if (gccp->locatetype == OBJ_SEQSUB)
	{
		if (gccp->locateID == gccp->itemIDs[OBJ_SEQSUB])
		{
			takeit = TRUE;
			LocateItem = gccp->locateID;
		}
	}

	thistype = OBJ_SEQSUB;
	if (takeit)
	{
		gcp->sep = NULL;
		gcp->previtem = NULL;
		gcp->prevtype = 0;
		gcp->parentitem = NULL;
		gcp->parenttype = 0;
		gcp->thisitem = (Pointer)ssp;
		gcp->thistype = thistype;
		gcp->prevlink = NULL;


		if (! (*(gccp->userfunc))(gcp))
			return FALSE;
		if (LocateItem) return FALSE;
	}
    
    gcp->indent++;
	if (! GatherSubBlock(gccp, ssp->sub, thistype, (Pointer)ssp,
		               (Pointer PNTR)&(ssp->sub)))
		return FALSE;
    
	tscope = in_scope;
	prevlink = (Pointer PNTR)&(ssp->data);

	switch(ssp->datatype)
	{
		case 1:    /* Seq-entrys */
			prevsep = NULL;
			for (sep = (SeqEntryPtr)(ssp->data); sep != NULL; sep = sep->next)
			{
				gcp->previtem = prevsep;
				gcp->prevtype = OBJ_SEQENTRY;
				gcp->parentitem = (Pointer)ssp;
				gcp->parenttype = thistype;
				gcp->prevlink = prevlink;

				if (checkscope)
				{
					if (sep == scope)
						tscope = TRUE;
					else
						tscope = FALSE;
				}

				if (! GatherSeqEntryFunc(sep, gccp, (Pointer)ssp, OBJ_SEQSUB, prevsep, tscope, prevlink))
					return FALSE;

				if (checkscope)
				{
					if (tscope == TRUE)  /* just found it */
					{
						checkscope = FALSE;   /* don't look anymore */
						tscope = FALSE;       /* the siblings not in scope */
						gsp->scope = NULL;    /* no more to look */
					}
					else if (gsp->scope == NULL)   /* found lower down */
					{
						checkscope = FALSE;
						tscope = FALSE;
					}
				}

				prevsep = sep;
				prevlink = (Pointer PNTR)&(sep->next);
			}
			break;
		case 2:    /* Seq-annots */
			if (! GatherSeqAnnot(gccp, (SeqAnnotPtr)(ssp->data), thistype,
						(Pointer)ssp,(Pointer PNTR)&(ssp->data), in_scope))
				return FALSE;

			break;
		case 3:    /* SeqIds */
			if (! GatherSeqIds(gccp, (SeqIdPtr)(ssp->data), thistype,
						(Pointer)ssp,(Pointer PNTR)&(ssp->data)))
				return FALSE;

			break;
		default:
			break;
	}
	gcp->indent--;  /* reset to original indent level */
	return TRUE;
}

/*****************************************************************************
*
*   GatherBioseqPartsFunc(gccp, top)
*      gets parts not contained in "top" for segmented entry
*
*****************************************************************************/
static Boolean NEAR GatherBioseqPartsFunc (InternalGCCPtr gccp, Pointer top)
{
	GatherContextPtr gcp;
	GatherScopePtr gsp, tgsp;
	GatherScope scopebuf;
	SeqLocPtr slp, head;
	SeqIdPtr sip;
	Boolean retval;
	BioseqPtr tbsp;
	SeqEntryPtr sep;
	Int4 len;

	gcp = &(gccp->gc);
	gsp = &(gccp->scope);

	if (gsp->seglevels <= gsp->currlevel)
		return TRUE;


	head = gccp->segloc;

	tgsp = &scopebuf;
	MemCopy(tgsp, gsp, sizeof(GatherScope));
	tgsp->seglevels--;
	tgsp->scope = NULL;
	tgsp->currlevel++;
	tgsp->ignore [OBJ_BIOSEQ] = TRUE;

	slp = NULL;
	
	while ((slp = SeqLocFindNext(head, slp)) != NULL)
	{
		tgsp->target = slp;
		sip = SeqLocId(slp);
		tbsp = BioseqLockById(sip);
		len = SeqLocLen(slp);

		if (len >= 0 && tbsp != NULL)
		{
			if (! ObjMgrIsChild(top, (Pointer)tbsp)) /* in set we just did? */
			{
				sep = SeqEntryFind(sip);
				retval = GatherSeqEntry(sep, gcp->userdata, gccp->userfunc, tgsp);
			}
			BioseqUnlock(tbsp);
			if (! retval)
				return FALSE;
		}
		
		tgsp->offset += len;

	}

	return TRUE;
}

static Boolean NEAR IGCCBuild (InternalGCCPtr ip, ObjMgrDataPtr omdp, Pointer userdata, GatherItemProc userfunc, GatherScopePtr scope)
{
	Boolean in_scope = TRUE;
	SeqIdPtr sip;
	BioseqPtr bsp;
	ValNode fake;
	SeqLocPtr slp;

	if ((omdp == NULL) || (userfunc == NULL)) return FALSE;

	MemSet((Pointer)(ip), 0, sizeof(InternalGCC));
	ip->gc.userdata = userdata;
	ip->userfunc = userfunc;
	if (scope != NULL)
		MemCopy(&(ip->scope), scope, sizeof(GatherScope));
	ip->gc.entityID = omdp->EntityID;
	ip->gc.tempload = omdp->tempload;
	ip->gc.seglevel = ip->scope.currlevel;

	if (ip->scope.target != NULL)
	{
		sip = SeqLocId(ip->scope.target);
		bsp = BioseqLockById(sip);
		ip->bsp = bsp;
		if (SeqLocStrand(ip->scope.target) == Seq_strand_minus)
			ip->rev = TRUE;
		if ((ip->scope.seglevels > ip->scope.currlevel)
			&& (bsp != NULL))     /* get seg parts? */
		{
			slp = NULL;
			if (bsp->repr == Seq_repr_seg)
			{
				fake.choice = SEQLOC_MIX;
				fake.next = NULL;
				fake.data.ptrvalue = bsp->seq_ext;
				slp = &fake;
			}
			else if (bsp->repr == Seq_repr_ref)
				slp = (SeqLocPtr)(bsp->seq_ext);

			ip->segloc = SeqLocCopyPart(slp, SeqLocStart(ip->scope.target),
				SeqLocStop(ip->scope.target), SeqLocStrand(ip->scope.target),
				TRUE, &(ip->first_seg), &(ip->last_seg));
				
		}
	}

	return TRUE;
}

AlignDataPtr FreeAlignData(AlignDataPtr adp)
{
	AlignDataPtr next;
	AlignRangePtr arp, arp_next;
	
	while(adp)
	{
		arp = adp->arp;
		while(arp)
		{
			arp_next = arp->next;
			arp->next = NULL;
			MemFree(arp);
			arp = arp_next;
		}
		next = adp->next;
		adp->next = NULL;
		MemFree(adp);
		adp = next;
	}
	return NULL;
}
	
	
static Boolean IGCCclear(InternalGCCPtr ip)
{
	if (ip == NULL) return TRUE;

	BioseqUnlock(ip->bsp);

	MemFree(ip->gc.rdp);
	FreeAlignData(ip->gc.adp);
	ip->gc.adp = NULL;
	SeqLocFree(ip->segloc);

	return TRUE;
}

static void GatherEntityFunc (ObjMgrDataPtr omdp, InternalGCCPtr gccp, Boolean in_scope)
{
	ValNodePtr vnp;
	Pointer ptr;

	vnp = omdp->choice;
	ptr = omdp->dataptr;

	switch (omdp->choicetype)
	{
		case OBJ_SEQENTRY:
			GatherSeqEntryFunc(vnp, gccp, NULL, 0, NULL, in_scope, NULL);
			break;

		default:
			switch (omdp->datatype)
			{
				case OBJ_BIOSEQ:
					GatherBioseqFunc(gccp, (BioseqPtr)ptr, NULL, 0, NULL, NULL,
						NULL, in_scope);
					break;

				case OBJ_BIOSEQSET:
					GatherBioseqSetFunc(gccp, (BioseqSetPtr)ptr, NULL, 0, NULL,
						in_scope, NULL, NULL);
					break;

				case OBJ_SEQDESC:
					GatherSeqDescr(gccp, (ValNodePtr)ptr, 0, NULL, NULL,in_scope);
					break;

				case OBJ_SEQANNOT:
					GatherSeqAnnot(gccp, (SeqAnnotPtr)ptr, 0,NULL,NULL, in_scope);
					break;

				case OBJ_ANNOTDESC:              /* NOT SUPPORTED YET */
					break;

				case OBJ_SEQFEAT:
					GatherSeqFeat(gccp, (SeqFeatPtr)ptr, 0, NULL, NULL, in_scope, OBJ_SEQFEAT);
					break;

				case OBJ_SEQALIGN:
					GatherSeqAlign(gccp, (SeqAlignPtr)ptr, 0, NULL, NULL, in_scope);
					break;

				case OBJ_SEQGRAPH:
					GatherSeqGraph(gccp, (SeqGraphPtr)ptr, 0, NULL, NULL, in_scope);
					break;

				case OBJ_SEQSUB:
					GatherSeqSubmit (gccp, (SeqSubmitPtr) ptr, in_scope);
					break;

				case OBJ_SUBMIT_BLOCK:
					GatherSubBlock(gccp, (SubmitBlockPtr)ptr,0,NULL,NULL);
					break;

				case OBJ_SEQSUB_CONTACT:
					GatherSeqSubContact(gccp, (ContactInfoPtr)ptr,0, NULL, NULL);
					break;

				case OBJ_BIOSEQ_MAPFEAT:        /* NOT SUPPORTED YET */
					break;

				case OBJ_BIOSEQ_SEG:			/* NOT SEPARATELY SUPPORTED */
					break;

				case OBJ_SEQHIST:    			/* NOT SEPARATELY SUPPORTED */
					break;

				case OBJ_PUB:
					GatherPub(gccp, (ValNodePtr)ptr, 0, 0, NULL,NULL, in_scope);
					break;

				case OBJ_SEQFEAT_CIT:			/* NOT SEPARATELY SUPPORTED */
					break;

				case OBJ_SEQSUB_CIT:
					GatherSeqSubCit(gccp, (CitSubPtr)ptr,0, NULL, NULL);
					break;

				case OBJ_PUB_SET:
					GatherPubSet(gccp, (ValNodePtr)ptr, 0, 0, NULL,NULL, in_scope);
					break;

				case OBJ_SEQID:
					GatherSeqIds(gccp, (SeqIdPtr)ptr,0,NULL,NULL);
					break;

				default:
					break;
			}
	}


}

/*****************************************************************************
*
*   GatherSeqEntry (sep, userdata, userproc, scope)
*
*****************************************************************************/
Boolean LIBCALL GatherSeqEntry (SeqEntryPtr sep, Pointer userdata, GatherItemProc userfunc, GatherScopePtr scope)
{
	Uint2 entityID;

	if ((sep == NULL) || (userfunc == NULL)) return FALSE;

	entityID = ObjMgrGetEntityIDForChoice(sep);
	
	return GatherEntity (entityID, userdata, userfunc, scope);
}

/*****************************************************************************
*
*   GatherEntity (entityID, userdata, userproc, scope)
*
*****************************************************************************/
Boolean LIBCALL GatherEntity (Uint2 entityID, Pointer userdata, GatherItemProc userfunc, GatherScopePtr scope)
{
	InternalGCC igcc;
	Boolean in_scope;
	ObjMgrDataPtr omdp;

	if ((! entityID) || (userfunc == NULL)) return FALSE;

	omdp = ObjMgrGetData (entityID);
	if (omdp == NULL) return FALSE;

	if (! IGCCBuild(&igcc, omdp, userdata, userfunc, scope))
		return FALSE;

	if (igcc.scope.scope != NULL)
	{
		in_scope = ObjMgrIsChild((igcc.scope.scope->data.ptrvalue), (omdp->dataptr));
	}
	else                   /* no scope set.. all in scope */
		in_scope = TRUE;

	                      /*** note: if you need to remotely fetch segments
						   ***   it is good to do it first, so that if you
						   ***   are also getting OBJ_BIOSEQ_SEG, the entry
						   ***   will already in (possibly) cached in memory
						   ***   when you look up length and ids
						   ***/

	if ((igcc.segloc != NULL) && (igcc.scope.scope == NULL)) /* get segs first */
		GatherBioseqPartsFunc(&igcc, omdp->dataptr);

	GatherEntityFunc(omdp, &igcc, in_scope);

	return IGCCclear(&igcc);
}

/*****************************************************************************
*
*   GatherItem (entityID, itemID, itemtype, userdata, userproc)
*      Get an item by entityID, itemID, itemtype
*      GatherContext.seglevel and GatherContext.propagated will not be
*        set on the callback.
*   
*      Sets in_scope to FALSE so that the callback is not called
*         Scope is NULL, so in_scope is never TRUE
*      Sets ignore TRUE for everything not needed to find item
*      Sets locatetype and locateID, which are checked in the traversal
*
*****************************************************************************/
Boolean LIBCALL GatherItem (Uint2 entityID, Uint2 itemID, Uint2 itemtype,
                                   Pointer userdata, GatherItemProc userfunc)
{
	InternalGCC igcc;
	ObjMgrDataPtr omdp;

	if (userfunc == NULL) return FALSE;

	if (itemtype >= OBJ_MAX) return FALSE;

	omdp = ObjMgrGetData(entityID);
	if (omdp == NULL) return FALSE;

	if (! IGCCBuild(&igcc, omdp, userdata, userfunc, NULL))
		return FALSE;

	igcc.locatetype = (Uint1)itemtype;
	igcc.locateID = itemID;
	MemSet((Pointer)(igcc.scope.ignore), (int)(TRUE), (size_t)((OBJ_MAX+1)*sizeof(Boolean)));
	igcc.scope.ignore[itemtype] = FALSE;
	switch (itemtype)		  /* add higher levels if necessary */
	{
		case OBJ_SEQFEAT_CIT:
			igcc.scope.ignore[OBJ_SEQFEAT] = FALSE;
			igcc.scope.ignore[OBJ_PUB_SET] = FALSE;
		case OBJ_SEQFEAT:
		case OBJ_SEQALIGN:
		case OBJ_SEQGRAPH:
			igcc.scope.ignore[OBJ_SEQANNOT] = FALSE;
			break;
		case OBJ_SEQSUB_CONTACT:
		case OBJ_SEQSUB_CIT:
			igcc.scope.ignore[OBJ_SUBMIT_BLOCK] = FALSE;
		case OBJ_SUBMIT_BLOCK:
			igcc.scope.ignore[OBJ_SEQSUB] = FALSE;
			break;
	}
    

	GatherEntityFunc(omdp, &igcc, FALSE);

	return IGCCclear(&igcc);
}

typedef struct gatherdatastruct {
	OMProcControlPtr ompcp;
	Boolean succeeded;
} GDS, PNTR GDSPtr;

static Boolean GatherDataProc (GatherContextPtr gcp)
{
	OMProcControlPtr ompcp;
	GDSPtr gdsp;

	gdsp = (GDSPtr)(gcp->userdata);
	ompcp = gdsp->ompcp;
    
    ompcp->input_data = gcp->thisitem;
    
	switch (gcp->thistype)
	{
		case OBJ_BIOSEQSET:
		case OBJ_BIOSEQ:
			if (gcp->sep != NULL)
			{
				ompcp->input_choice = gcp->sep;
				ompcp->input_choicetype = OBJ_SEQENTRY;
			}
			break;
		default:
			break;
	}
	gdsp->succeeded = TRUE;
	return TRUE;
}

static Boolean ReplaceDataProc (GatherContextPtr gcp)
{
	ObjMgrPtr omp;
	ObjMgrTypePtr omtp;
	ObjMgrDataPtr omdp;
	OMProcControlPtr ompcp;
	Pointer next = NULL, ptr, newptr, dest=NULL, oldsrc;
	GDSPtr gdsp;
	size_t size = 0;
	ValNodePtr vnp;
	SeqEntryPtr sep=NULL;


	gdsp = (GDSPtr)(gcp->userdata);
	ompcp = gdsp->ompcp;
	newptr = ompcp->output_data;

    ptr = gcp->thisitem;
	oldsrc = ptr;

	omp = ObjMgrGet();
	omtp = ObjMgrTypeFind(omp, gcp->thistype, NULL, NULL);
	if (omtp == NULL)
	{
		ErrPostEx(SEV_ERROR,0,0,"ReplaceDataProc: can't find type [%d]", (int)gcp->thistype);
		return TRUE;
	}

	dest = MemNew(1000);   /* temporary buffer for copies */
	if (dest == NULL)
	{
		ErrPostEx(SEV_ERROR,0,0,"ReplaceDataProc: can't allocate buffer");
		return TRUE;
	}
    
	switch (gcp->thistype)
	{
		case OBJ_SEQSUB:
			size = sizeof(SeqSubmit);
			break;
		case OBJ_SUBMIT_BLOCK:
			size = sizeof(SubmitBlock);
			break;
		case OBJ_SEQSUB_CONTACT:
			size = sizeof(ContactInfo);
			break;
		case OBJ_SEQSUB_CIT:
			size = sizeof(CitSub);
			break;
		case OBJ_SEQHIST:
			size = sizeof(SeqHist);
			break;
		case OBJ_BIOSEQSET:
			size = sizeof(BioseqSet);
			break;
		case OBJ_BIOSEQ:
			size = sizeof(Bioseq);
			break;
		case OBJ_SEQDESC:
		case OBJ_BIOSEQ_SEG:
		case OBJ_PUB:
		case OBJ_SEQFEAT_CIT:
		case OBJ_PUB_SET:
		case OBJ_SEQLOC:
		case OBJ_SEQID:
		case OBJ_SEQENTRY:
			vnp = (ValNodePtr)ptr;
			oldsrc = &(vnp->data);
			size = sizeof(DataVal);
			vnp = (ValNodePtr)newptr;
			newptr = &(vnp->data);
			break;
		case OBJ_BIOSEQ_MAPFEAT:
		case OBJ_SEQFEAT:
			size = sizeof(SeqFeat);
			next = (Pointer)(((SeqFeatPtr)(ptr))->next);
			(((SeqFeatPtr)(ptr))->next) = NULL;
			(((SeqFeatPtr)(newptr))->next) = (SeqFeatPtr)next;
			break;
		case OBJ_SEQANNOT:
			size = sizeof(SeqAnnot);
			next = (Pointer)(((SeqAnnotPtr)(ptr))->next);
			(((SeqAnnotPtr)(ptr))->next) = NULL;
			(((SeqAnnotPtr)(newptr))->next) = (SeqAnnotPtr)next;
			break;
		case OBJ_SEQALIGN:
			size = sizeof(SeqAlign);
			next = (Pointer)(((SeqAlignPtr)(ptr))->next);
			(((SeqAlignPtr)(ptr))->next) = NULL;
			(((SeqAlignPtr)(newptr))->next) = (SeqAlignPtr)next;
			break;
		case OBJ_SEQGRAPH:
			size = sizeof(SeqGraph);
			next = (Pointer)(((SeqGraphPtr)(ptr))->next);
			(((SeqGraphPtr)(ptr))->next) = NULL;
			(((SeqGraphPtr)(newptr))->next) = (SeqGraphPtr)next;
			break;
		default:
			ErrPostEx(SEV_ERROR,0,0,"ReplaceDataProc: unsupported type %d",
				(int)(gcp->thistype));
			return TRUE;
			break;
	}

	dest = MemNew(1000);   /* temporary buffer for copies */
	if (dest == NULL)
	{
		ErrPostEx(SEV_ERROR,0,0,"ReplaceDataProc: can't allocate buffer");
		return TRUE;
	}
    
	MemCopy(dest, oldsrc, size);    /* replace the contents */
	MemCopy(oldsrc, newptr, size);
	MemCopy(newptr, dest, size);

	MemFree(dest);

	if (ompcp->output_entityID)   /* remove with objmgr? */
	{
		omdp = ObjMgrGetData (ompcp->output_entityID);
		if (omdp != NULL)
		{
			if (ObjMgrWholeEntity(omdp, ompcp->output_itemID, ompcp->output_itemtype))
				ObjMgrDelete(ompcp->output_itemtype, ompcp->output_data);
			if ((gcp->thistype == OBJ_BIOSEQ) || (gcp->thistype == OBJ_BIOSEQSET))
				sep = omdp->choice;
		}
	}

	if (sep != NULL)
		SeqEntryFree(sep);
	else
		(*(omtp->freefunc))(ompcp->output_data);

	gdsp->succeeded = TRUE;

	return TRUE;
}

static Boolean DetachDataProc (GatherContextPtr gcp)
{
	OMProcControlPtr ompcp;
	Pointer next = NULL, ptr;
	GDSPtr gdsp;

	gdsp = (GDSPtr)(gcp->userdata);
	ompcp = gdsp->ompcp;

    ptr = gcp->thisitem;
    ompcp->input_data = ptr;
    
	switch (gcp->thistype)
	{
		case OBJ_BIOSEQSET:
		case OBJ_BIOSEQ:
			if (gcp->sep != NULL)
			{
				ompcp->input_choice = gcp->sep;
				ompcp->input_choicetype = OBJ_SEQENTRY;
				next = gcp->sep->next;
			}
			break;
		case OBJ_SEQDESC:
		case OBJ_BIOSEQ_SEG:
		case OBJ_PUB:
		case OBJ_SEQFEAT_CIT:
		case OBJ_PUB_SET:
		case OBJ_SEQLOC:
		case OBJ_SEQID:
		case OBJ_SEQENTRY:
			next = (Pointer)(((ValNodePtr)(ptr))->next);
			(((ValNodePtr)(ptr))->next) = NULL;
			break;
		case OBJ_BIOSEQ_MAPFEAT:
		case OBJ_SEQFEAT:
			next = (Pointer)(((SeqFeatPtr)(ptr))->next);
			(((SeqFeatPtr)(ptr))->next) = NULL;
			break;
		case OBJ_SEQANNOT:
			next = (Pointer)(((SeqAnnotPtr)(ptr))->next);
			(((SeqAnnotPtr)(ptr))->next) = NULL;
			break;
		case OBJ_SEQALIGN:
			next = (Pointer)(((SeqAlignPtr)(ptr))->next);
			(((SeqAlignPtr)(ptr))->next) = NULL;
			break;
		case OBJ_SEQGRAPH:
			next = (Pointer)(((SeqGraphPtr)(ptr))->next);
			(((SeqGraphPtr)(ptr))->next) = NULL;
			break;
		default:
			break;
	}
	if (ompcp->whole_entity)  /* nothing to detach from */
		return TRUE;

	if (gcp->prevlink != NULL)
		*(gcp->prevlink) = next;

	ObjMgrDetach(gcp->thistype, ptr);

	gdsp->succeeded = TRUE;

	return TRUE;
}

static void AddAnAnnot(SeqAnnotPtr PNTR head, Pointer ptr)
{
	SeqAnnotPtr prev, sap;

	if (head == NULL) return;
	sap = (SeqAnnotPtr)ptr;

	if (*head == NULL)
	{
		*head = sap;
		return;
	}

	for (prev = (*head); prev->next != NULL; prev = prev->next)
		continue;

	prev->next = sap;
	return;
}

static Boolean AddToSeqAnnot (SeqAnnotPtr sap, Int2 the_type, Pointer addptr)
{
	SeqFeatPtr sfp;
	SeqAlignPtr salp;
	SeqGraphPtr sgp;
	Pointer PNTR prevlink = NULL;

	if (sap == NULL) return FALSE;

	if (sap->type == 0)
		sap->type = (Uint1)the_type;

	if (sap->type != (Uint1)the_type)
		return FALSE;

	if (sap->data == NULL)
		prevlink = &(sap->data);
	else
	{
		switch (the_type)
		{
			case 1:   /* feature table */
				for (sfp = (SeqFeatPtr)(sap->data); sfp->next != NULL; sfp = sfp->next)
					continue;
				prevlink = (Pointer PNTR)&(sfp->next);
				break;
			case 2:   /* alignments */
				for (salp = (SeqAlignPtr)(sap->data); salp->next != NULL; salp = salp->next)
					continue;
				prevlink = (Pointer PNTR)&(salp->next);
				break;
			case 3:   /* Graph */
				for (sgp = (SeqGraphPtr)(sap->data); sgp->next != NULL; sgp = sgp->next)
					continue;
				prevlink = (Pointer PNTR)&(sgp->next);
				break;
		}
	}
	if (prevlink != NULL)
		*prevlink = addptr;

	return TRUE;
}

static void AddToAnnot(SeqAnnotPtr PNTR head, Int2 the_type, Pointer addptr)
{
	SeqAnnotPtr sap, prev=NULL;

	if (head == NULL) return;

	for (sap = *head; sap != NULL; sap = sap->next)
	{
		if (sap->type == the_type)
			break;
		prev = sap;
	}

	if (sap == NULL)
	{
		sap = SeqAnnotNew();
		sap->type = (Uint1)the_type;
		if (prev != NULL)
			prev->next = sap;
		else
			*head = sap;
	}

	AddToSeqAnnot(sap, the_type, addptr);

	return;
}

static ValNodePtr PubFromDescr(ValNodePtr desc)
{
	ValNodePtr vnp2, vnp;
	PubdescPtr pdp;

	if (desc->choice != Seq_descr_pub)
		return NULL;

	pdp = (PubdescPtr)(desc->data.ptrvalue);
	vnp = pdp->pub;
	pdp->pub = NULL;

	SeqDescFree(desc);

	if (vnp == NULL) return vnp;

	if (vnp->next != NULL)
	{
		vnp2 = vnp;
		vnp = ValNodeNew(NULL);
		vnp->choice = PUB_Equiv;
		vnp->data.ptrvalue = vnp2;
	}

	return vnp;
}

static ValNodePtr DescrFromPub(ValNodePtr pub)
{
	ValNodePtr vnp;
	PubdescPtr pdp;

	pdp = PubdescNew();
	if (pub->choice == PUB_Equiv)
	{
		vnp = (ValNodePtr)(pub->data.ptrvalue);
		MemFree(pub);
		pub = vnp;
	}
	pdp->pub = pub;
	vnp = ValNodeNew(NULL);
	vnp->choice = Seq_descr_pub;
	vnp->data.ptrvalue = (Pointer)pdp;
	return vnp;
}

static Boolean AttachDataProc (GatherContextPtr gcp)
{
	OMProcControlPtr ompcp;
	Pointer ptr, newptr;
	Uint2 into, newtype;
	Boolean no_good = FALSE, into_seqentry = FALSE;
	ValNodePtr vnp;
	ObjMgrDataPtr omdp=NULL;
	ObjMgrPtr omp;
	BioseqPtr bsp;
	BioseqSetPtr bssp;
	SeqAnnotPtr sap;
	SeqFeatPtr sfp;
	SeqAlignPtr salp;
	SeqGraphPtr sgp;
	SeqSubmitPtr ssp;
	SubmitBlockPtr sbp;
	GDSPtr gdsp;

	gdsp = (GDSPtr)(gcp->userdata);
	ompcp = gdsp->ompcp;

    ptr = gcp->thisitem;   /* insert before or into this */
    ompcp->input_data = ptr;

    into = gcp->thistype;
	newtype = ompcp->output_itemtype;
	newptr = ompcp->output_data;
	omp = ObjMgrGet();
	omdp = ObjMgrFindByData(omp, newptr);

	switch (into)
	{
		case OBJ_BIOSEQSET:
		case OBJ_BIOSEQ:
			if (gcp->sep != NULL)
			{
				ompcp->input_choice = gcp->sep;
				ompcp->input_choicetype = OBJ_SEQENTRY;
				into_seqentry = TRUE;
			}
			break;
		default:
			break;
	}

	switch (into)
	{
		case OBJ_SEQSUB:
			ssp = (SeqSubmitPtr)(ptr);
			switch (newtype)
			{
				case OBJ_BIOSEQSET:
				case OBJ_BIOSEQ:
					if (omdp == NULL)
					{
						ErrPostEx(SEV_ERROR,0,0,"AttachDataProc: Not entity for Bioseq(Set)");
						return TRUE;
					}
					if (ssp->datatype == 0)
						ssp->datatype = 1;
					if (ssp->datatype != 1)
					{
						no_good = TRUE;
						break;
					}
					if (omdp->choice == NULL)
					{
						omdp->choice = ValNodeNew(NULL);
						omdp->choicetype = OBJ_SEQENTRY;
						omdp->choice->data.ptrvalue = newptr;
						if (newtype == OBJ_BIOSEQ)
							omdp->choice->choice = 1;
						else
							omdp->choice->choice = 2;
					}
					ValNodeLink((ValNodePtr PNTR)&(ssp->data), omdp->choice);
					ObjMgrConnect(newtype, newptr, into, ptr);
					break;
				case OBJ_PUB:
					vnp = (ValNodePtr)newptr;
					no_good = TRUE;
					if (vnp->choice == PUB_Sub)
					{
						if (ssp->sub == NULL)
							ssp->sub = SubmitBlockNew();
						if (ssp->sub->cit == NULL)
						{
							ssp->sub->cit = (CitSubPtr)(vnp->data.ptrvalue);
							ValNodeFree(vnp);
							no_good = FALSE;
						}
					}
					break;
				case OBJ_SEQENTRY:
					if (ssp->datatype == 0)
						ssp->datatype = 1;
					if (ssp->datatype != 1)
					{
						no_good = TRUE;
						break;
					}
					ValNodeLink((ValNodePtr PNTR)&(ssp->data), omdp->choice);
					ObjMgrConnect(newtype, newptr, into, ptr);
					break;
				case OBJ_SEQANNOT:
					if (ssp->datatype == 0)
						ssp->datatype = 2;
					if (ssp->datatype != 2)
					{
						no_good = TRUE;
						break;
					}
					AddAnAnnot((SeqAnnotPtr PNTR)&(ssp->data), newptr);
					break;
				case OBJ_SEQFEAT:
					if (ssp->datatype == 0)
						ssp->datatype = 2;
					if (ssp->datatype != 2)
					{
						no_good = TRUE;
						break;
					}
					AddToAnnot((SeqAnnotPtr PNTR)&(ssp->data), 1, newptr);
					break;
				case OBJ_SEQALIGN:
					if (ssp->datatype == 0)
						ssp->datatype = 2;
					if (ssp->datatype != 2)
					{
						no_good = TRUE;
						break;
					}
					AddToAnnot((SeqAnnotPtr PNTR)&(ssp->data), 2, newptr);
					break;
				case OBJ_SEQGRAPH:
					if (ssp->datatype == 0)
						ssp->datatype = 2;
					if (ssp->datatype != 2)
					{
						no_good = TRUE;
						break;
					}
					AddToAnnot((SeqAnnotPtr PNTR)&(ssp->data), 3, newptr);
					break;
				case OBJ_SUBMIT_BLOCK:
					if (ssp->sub == NULL)
						ssp->sub = (SubmitBlockPtr)newptr;
					else
						no_good = TRUE;
					break;
				case OBJ_SEQSUB_CONTACT:
					if (ssp->sub == NULL)
						ssp->sub = SubmitBlockNew();
					if (ssp->sub->contact == NULL)
						ssp->sub->contact = (ContactInfoPtr)newptr;
					else
						no_good = TRUE;
					break;
				default:
					no_good = TRUE;
					break;
			}
			break;
		case OBJ_SUBMIT_BLOCK:
			sbp = (SubmitBlockPtr)(ptr);
			switch (newtype)
			{
				case OBJ_PUB:
					vnp = (ValNodePtr)newptr;
					no_good = TRUE;
					if (vnp->choice == PUB_Sub)
					{
						if (sbp->cit == NULL)
						{
							sbp->cit = (CitSubPtr)(vnp->data.ptrvalue);
							ValNodeFree(vnp);
							no_good = FALSE;
						}
					}
					break;
				case OBJ_SEQSUB_CONTACT:
					if (sbp->contact == NULL)
						sbp->contact = (ContactInfoPtr)newptr;
					else
						no_good = TRUE;
					break;
				default:
					no_good = TRUE;
					break;
			}
			break;
		case OBJ_BIOSEQSET:
			bssp = (BioseqSetPtr)(ptr);
			switch (newtype)
			{
				case OBJ_BIOSEQSET:
				case OBJ_BIOSEQ:
					if (omdp == NULL)
					{
						ErrPostEx(SEV_ERROR,0,0,"AttachDataProc: Not entity for Bioseq(Set)");
						return TRUE;
					}
					if (omdp->choice == NULL)
					{
						omdp->choice = ValNodeNew(NULL);
						omdp->choicetype = OBJ_SEQENTRY;
						omdp->choice->data.ptrvalue = newptr;
						if (newtype == OBJ_BIOSEQ)
							omdp->choice->choice = 1;
						else
							omdp->choice->choice = 2;
					}
					ValNodeLink(&(bssp->seq_set), omdp->choice);
					ObjMgrConnect(newtype, newptr, into, ptr);
					break;
				case OBJ_SEQDESC:
					ValNodeLink(&(bssp->descr), (ValNodePtr)newptr);
					break;
				case OBJ_PUB:			   /* make a pubdesc */
				case OBJ_SEQFEAT_CIT:
					vnp = DescrFromPub((ValNodePtr)newptr);
					ValNodeLink(&(bssp->descr), vnp);
					break;
				case OBJ_SEQENTRY:
					ValNodeLink(&(bssp->seq_set), newptr);
					break;
				case OBJ_SEQANNOT:
					AddAnAnnot(&(bssp->annot), newptr);
					break;
				case OBJ_SEQFEAT:
					AddToAnnot(&(bssp->annot), 1, newptr);
					break;
				case OBJ_SEQALIGN:
					AddToAnnot(&(bssp->annot), 2, newptr);
					break;
				case OBJ_SEQGRAPH:
					AddToAnnot(&(bssp->annot), 3, newptr);
					break;
				default:
					no_good = TRUE;
					break;
			}
			break;
		case OBJ_BIOSEQ:
			bsp = (BioseqPtr)(ptr);
			switch (newtype)
			{
				case OBJ_BIOSEQSET:
				case OBJ_BIOSEQ:
					if (omdp == NULL)
					{
						ErrPostEx(SEV_ERROR,0,0,"AttachDataProc: Not entity for Bioseq(Set)");
						return TRUE;
					}
					if (omdp->choice == NULL)
					{
						omdp->choice = ValNodeNew(NULL);
						omdp->choicetype = OBJ_SEQENTRY;
						omdp->choice->data.ptrvalue = newptr;
						if (newtype == OBJ_BIOSEQ)
							omdp->choice->choice = 1;
						else
							omdp->choice->choice = 2;
					}
					if ((gcp->parentitem != NULL) && (gcp->prevlink != NULL))
					{
						omdp->choice->next = *(gcp->prevlink);
						*(gcp->prevlink) = omdp->choice;
						ObjMgrConnect(newtype, newptr, gcp->parenttype, gcp->parentitem);
					}
					else
						no_good = TRUE;
					break;
				case OBJ_SEQDESC:
					ValNodeLink(&(bsp->descr), (ValNodePtr)newptr);
					break;
				case OBJ_PUB:			   /* make a pubdesc */
				case OBJ_SEQFEAT_CIT:
					vnp = DescrFromPub((ValNodePtr)newptr);
					ValNodeLink(&(bsp->descr), vnp);
					break;
				case OBJ_SEQENTRY:
					if ((gcp->parentitem != NULL) && (gcp->prevlink != NULL))
					{
						omdp->choice->next = *(gcp->prevlink);
						*(gcp->prevlink) = omdp->choice;
						ObjMgrConnect(newtype, newptr, gcp->parenttype, gcp->parentitem);
					}
					else
						no_good = TRUE;
					break;
				case OBJ_SEQANNOT:
					AddAnAnnot(&(bsp->annot), newptr);
					break;
				case OBJ_SEQFEAT:
					AddToAnnot(&(bsp->annot), 1, newptr);
					break;
				case OBJ_SEQALIGN:
					AddToAnnot(&(bsp->annot), 2, newptr);
					break;
				case OBJ_SEQGRAPH:
					AddToAnnot(&(bsp->annot), 3, newptr);
					break;
				default:
					no_good = TRUE;
					break;
			}
			break;
		case OBJ_SEQDESC:
			vnp = (ValNodePtr)newptr;
			switch (newtype)
			{
				case OBJ_PUB:
					vnp = DescrFromPub(vnp);
				case OBJ_SEQDESC:
					if (gcp->prevlink != NULL)
					{
						vnp->next = *(gcp->prevlink);
						*(gcp->prevlink) = vnp;
					}
					else
						no_good = TRUE;
					break;
				default:
					no_good = TRUE;
					break;
			}
			break;
		case OBJ_SEQFEAT_CIT:
		case OBJ_PUB_SET:
			vnp = (ValNodePtr)newptr;
			switch (newtype)
			{
				case OBJ_SEQDESC:
					vnp = PubFromDescr(vnp);
					if (vnp == NULL)
					{
						no_good = TRUE;
						break;
					}
				case OBJ_PUB:
					if (gcp->prevlink != NULL)
					{
						vnp->next = *(gcp->prevlink);
						*(gcp->prevlink) = vnp;
					}
					else
						no_good = TRUE;
					break;
				default:
					no_good = TRUE;
					break;
			}
			break;
		case OBJ_SEQANNOT:
			sap = (SeqAnnotPtr)ptr;
			switch (newtype)
			{
				case OBJ_SEQFEAT:
					if (! AddToSeqAnnot(sap, 1, newptr))
						no_good = TRUE;
					break;
				case OBJ_SEQALIGN:
					if (! AddToSeqAnnot(sap, 2, newptr))
						no_good = TRUE;
					break;
				case OBJ_SEQGRAPH:
					if (! AddToSeqAnnot(sap, 3, newptr))
						no_good = TRUE;
					break;
				case OBJ_SEQANNOT:
					sap = (SeqAnnotPtr)newptr;
					if (gcp->prevlink != NULL)
					{
						sap->next = *(gcp->prevlink);
						*(gcp->prevlink) = sap;
					}
					else
						no_good = TRUE;
					break;
				default:
					no_good = TRUE;
					break;
			}
			break;
		case OBJ_BIOSEQ_MAPFEAT:
		case OBJ_SEQFEAT:
			sfp = (SeqFeatPtr)ptr;
			switch (newtype)
			{
				case OBJ_SEQDESC:
					newptr = (Pointer)PubFromDescr((ValNodePtr)newptr);
				case OBJ_PUB:
					if (sfp->cit == NULL)
					{
						sfp->cit = ValNodeNew(NULL);
						sfp->cit->choice = 1;
					}
					ValNodeLink((ValNodePtr PNTR)&(sfp->cit->data.ptrvalue), (ValNodePtr)newptr);
					break;
				case OBJ_SEQFEAT:
					sfp = (SeqFeatPtr)newptr;
					if (gcp->prevlink != NULL)
					{
						sfp->next = *(gcp->prevlink);
						*(gcp->prevlink) = sfp;
					}
					else
						no_good = TRUE;
					break;
				default:
					no_good = TRUE;
					break;
			}
			break;
		case OBJ_SEQALIGN:
			switch (newtype)
			{
				case OBJ_SEQALIGN:
					salp = (SeqAlignPtr)newptr;
					if (gcp->prevlink != NULL)
					{
						salp->next = *(gcp->prevlink);
						*(gcp->prevlink) = salp;
					}
					else
						no_good = TRUE;
					break;
				default:
					no_good = TRUE;
					break;
			}
			break;
		case OBJ_SEQGRAPH:
			switch (newtype)
			{
				case OBJ_SEQGRAPH:
					sgp = (SeqGraphPtr)newptr;
					if (gcp->prevlink != NULL)
					{
						sgp->next = *(gcp->prevlink);
						*(gcp->prevlink) = sgp;
					}
					else
						no_good = TRUE;
					break;
				default:
					no_good = TRUE;
					break;
			}
			break;
		case OBJ_SEQLOC:
		case OBJ_SEQID:
		case OBJ_SEQENTRY:
		default:
			no_good = TRUE;
			break;
	}

	if (no_good)
	{
		ErrPostEx(SEV_ERROR,0,0,"AttachDataProc: [%d] into [%d]", (int)newtype, (int)into);
	}
	else
	{
		gdsp->succeeded = TRUE;
		switch (newtype)
		{
			case OBJ_BIOSEQ:
			case OBJ_BIOSEQSET:
				break;
			default:
				if (omdp != NULL)
					ObjMgrDelete(omdp->datatype, omdp->dataptr);
		}
	}

	return TRUE;
}

static Boolean CopyDataProc (GatherContextPtr gcp)
{
	OMProcControlPtr ompcp;
	ObjMgrTypePtr omtp;
	ObjMgrPtr omp;
	Uint2 type;
	Pointer ptr, ptr2;
	Boolean was_choice = FALSE;
	GDSPtr gdsp;

	gdsp = (GDSPtr)(gcp->userdata);
	ompcp = gdsp->ompcp;

    ptr = gcp->thisitem;
	type = gcp->thistype;
    ompcp->input_data = ptr;
    
	switch (gcp->thistype)
	{
		case OBJ_BIOSEQSET:
		case OBJ_BIOSEQ:
			if (gcp->sep != NULL)
			{
				ompcp->input_choice = gcp->sep;
				ptr = (Pointer)(gcp->sep);
				ompcp->input_choicetype = OBJ_SEQENTRY;
				type = OBJ_SEQENTRY;
				was_choice = TRUE;
			}
			break;
		default:
			break;
	}

	omp = ObjMgrGet();
	omtp = ObjMgrTypeFind(omp, type, NULL, NULL);
	if (omtp == NULL)
	{
		ErrPostEx(SEV_ERROR,0,0,"CopyDataProc: can't find type [%d]", (int)type);
		return TRUE;
	}

	ptr2 = AsnIoMemCopy(ptr, omtp->asnread, omtp->asnwrite);
	if (ptr2 != NULL)
	{
		gdsp->succeeded = TRUE;

		if (! was_choice)
		{
			ompcp->output_data = ptr2;
			ompcp->output_itemtype = type;
		}
		else
		{
			ompcp->output_choice = ptr2;
			ompcp->output_choicetype = type;
			ompcp->output_data = ((ValNodePtr)(ptr2))->data.ptrvalue;
			ompcp->output_itemtype = gcp->thistype;
		}
		switch (ompcp->output_itemtype)
		{
			case OBJ_BIOSEQSET:    /* these types Add to the ObjMgr themselves */
			case OBJ_BIOSEQ:
			case OBJ_SEQSUB:
				break;
			default:
				ObjMgrAdd(ompcp->output_itemtype, ompcp->output_data);
				break;
		}
	}
	return TRUE;
}

static Boolean NEAR GenericGatherDataForProc (OMProcControlPtr ompcp, Boolean sel, Int2 func)
{
	ObjMgrDataPtr omdp;
	SelStructPtr ssp;
	Boolean retval = FALSE;
	static CharPtr funcs [5] = {
		"GatherDataForProc",
		"DetachDataForProc",
		"AttachDataForProc",
		"CopyDataForProc",
		"ReplaceDataForProc" };
	GatherItemProc gip;
	GDS gds;

	gds.succeeded = retval;
	gds.ompcp = ompcp;

	if (sel)
	{
		ssp = ObjMgrGetSelected();
		if (ssp == NULL)
		{
			/*
			ErrPostEx(SEV_ERROR,0,0,"%s: Nothing was selected", funcs[func]);
			*/
			return retval;
		}
		ompcp->input_entityID = ssp->entityID;
		ompcp->input_itemID = ssp->itemID;
		ompcp->input_itemtype = ssp->itemtype;
	}

	if (func == 1) /* detach: changes selection */
		ObjMgrDeSelect(ompcp->input_entityID, ompcp->input_itemID, ompcp->input_itemtype);

	omdp = ObjMgrGetData (ompcp->input_entityID);
	if (omdp == NULL)
	{
		ErrPostEx(SEV_ERROR,0,0,"%s: can't match entityID", funcs[func]);
	 	return retval;
	}
	
	if ((ompcp->input_itemID <= 1) &&
	    ((ompcp->input_itemtype == 0) || (ompcp->input_itemtype == omdp->datatype)))  /* top level */
	{
		ompcp->input_choice = omdp->choice;
		ompcp->input_choicetype = omdp->choicetype;
		ompcp->input_itemtype = omdp->datatype;
		ompcp->input_data = omdp->dataptr;
		ompcp->whole_entity = TRUE;
		if ((func == 0) || (func == 1))  /* gather or detach */
			return TRUE;
	}

	if (func == 4)    /* replace */
	{
		if (ompcp->input_itemtype != ompcp->output_itemtype)
		{
			ErrPostEx(SEV_ERROR,0,0,"%s: input type %d != output type %d",
				funcs[func], (int)(ompcp->input_itemtype), (int)(ompcp->output_itemtype));
			return FALSE;
		}

		if (ompcp->output_data == NULL)
		{
			ErrPostEx(SEV_ERROR,0,0,"%s: no output_data", funcs[func]);
			return FALSE;
		}
	}

	switch(func)
	{
		case 0:
			gip = GatherDataProc;
			break;
		case 1:
			gip = DetachDataProc;
			break;
		case 2:
			gip = AttachDataProc;
			break;
		case 3:
			gip = CopyDataProc;
			break;
		case 4:
			gip = ReplaceDataProc;
			break;

	}

	retval = GatherItem (ompcp->input_entityID, ompcp->input_itemID,
                  ompcp->input_itemtype,(Pointer)(&gds), gip);

	if (! retval)
		ErrPostEx(SEV_ERROR,0,0,"%s: can't do the gather", funcs[func]);
	else
		retval = gds.succeeded;
	return retval;
}

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
Boolean LIBCALL GatherDataForProc (OMProcControlPtr ompcp, Boolean sel)
{
	return GenericGatherDataForProc(ompcp, sel, 0);
}

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
Boolean LIBCALL DetachDataForProc (OMProcControlPtr ompcp, Boolean sel)
{
	return GenericGatherDataForProc(ompcp, sel, 1);
}

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
Boolean LIBCALL AttachDataForProc (OMProcControlPtr ompcp, Boolean sel)
{
	return GenericGatherDataForProc(ompcp, sel, 2);
}

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
*
****************************************************************************/
Boolean LIBCALL CopyDataForProc (OMProcControlPtr ompcp, Boolean sel)
{
	return GenericGatherDataForProc(ompcp, sel, 3);
}

/****************************************************************************
*
*   ReplaceDataForProc(ompcp, sel)
*
*       fills in data, choice, and choictype in OMProcControlPtr
*         sets ompcp->whole_entity TRUE if appropriate
*       returns TRUE if it did it
*       if (sel == TRUE), fills in ompcp with data from ObjMgrGetSelect first.
*          returns FALSE if nothing selected.. Does ErrPostEx() for it
*       Replaces data in input section of ompcp with the output section
*       Data in input section is deleted
*
****************************************************************************/
Boolean LIBCALL ReplaceDataForProc (OMProcControlPtr ompcp, Boolean sel)
{
	return GenericGatherDataForProc(ompcp, sel, 4);
}

/*****************************************************************************
*
*   GatherProcLaunch(proctype, sel, entityID, itemID, itemtype,
*        inputtype, subinputtype, outputtype, suboutputtype)
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
Int2 GatherProcLaunch (Uint2 proctype, Boolean sel, Uint2 entityID, Uint2 itemID,
                    Uint2 itemtype, Uint2 inputtype, Uint2 subinputtype, Uint2 outputtype, Uint2 suboutputtype)
{
	ObjMgrPtr omp;
	OMProcControl ompc;
	ObjMgrProcPtr ompp=NULL, genproc=NULL;
	ObjMgrTypePtr omtp;
	Boolean retval;
	Uint2 subtype = 0;

	MemSet(&ompc, 0, sizeof(OMProcControl));

	ompc.input_entityID = entityID;
	ompc.input_itemID = itemID;
	ompc.input_itemtype = itemtype;

	retval = GatherDataForProc(&ompc, sel);

	if (sel && (! retval))
		return OM_MSG_RET_ERROR;

	if (entityID && (! retval))
		return OM_MSG_RET_ERROR;

	if (! inputtype)   /* not set on input */
		inputtype = ompc.input_itemtype;   /* could now be filled in */

	omp = ObjMgrGet();
	if ((! subinputtype) && (inputtype == ompc.input_itemtype)
		               && (ompc.input_data != NULL))
	{
		omtp = ObjMgrTypeFind(omp, inputtype, NULL, NULL);
		if (omtp != NULL)
			subinputtype = (*(omtp->subtypefunc))(ompc.input_data);
	}

	while ((ompp = ObjMgrProcFindNext(omp, proctype, inputtype, outputtype, ompp)) != NULL)
	{
		if (ompp->subinputtype == subinputtype)
			break;
		else if ((genproc == NULL) && (! (ompp->subinputtype)))
			genproc = ompp;
	}

	if (ompp == NULL)
		ompp = genproc;

	if (ompp == NULL)
		return OM_MSG_RET_NOPROC;

	ompc.proc = ompp;

	return (*(ompp->func))((Pointer)&ompc);

}

