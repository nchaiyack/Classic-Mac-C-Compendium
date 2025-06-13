/* gbparlex.h
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
* File Name:  objseq.h
*
* Author:  Karl Sirotkin
*
* $Log: gbparlex.h,v $
 * Revision 1.2  1995/05/15  21:46:05  ostell
 * added Log line
 *
*
*/


#ifndef _NCBI_GBParseLex_
#define _NCBI_GBParseLex_

#define GBPARSE_INT_UNKNOWN 0
#define GBPARSE_INT_JOIN 1
#define GBPARSE_INT_COMPL 2
#define GBPARSE_INT_LEFT 3
#define GBPARSE_INT_RIGHT 4
#define GBPARSE_INT_CARET 5
#define GBPARSE_INT_DOT_DOT 6
#define GBPARSE_INT_ACCESION 7
#define GBPARSE_INT_GT 8
#define GBPARSE_INT_LT 9
#define GBPARSE_INT_COMMA 10
#define GBPARSE_INT_NUMBER 11
#define GBPARSE_INT_ORDER 12
#define GBPARSE_INT_SINGLE_DOT 13
#define GBPARSE_INT_GROUP 14
#define GBPARSE_INT_ONE_OF 15
#define GBPARSE_INT_REPLACE 16
#define GBPARSE_INT_SITES 17
#define GBPARSE_INT_STRING 18
#define GBPARSE_INT_ONE_OF_NUM 19

#define ERR_NCBIGBPARSE_LEX 1
#define ERR_NCBIGBPARSE_INT 2

#endif
ation;

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
