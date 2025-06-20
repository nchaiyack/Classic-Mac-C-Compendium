/*****************************************************************************
*
*	This file ("utilities.c") contains functions, for use in "asn2flat",
*	that are likely to be generally useful.
*
*****************************************************************************/
/* $Revision: 1.54 $ */ 
#include "asn2ff6.h"
#include "asn2ffp.h"


Boolean asn2ff_flags[11];

static SeqFeatPtr MakeSyntheticSeqFeat PROTO ((void));
static void CpPubDescr PROTO ((PubdescPtr new_desc, PubdescPtr old_desc));
Int2 GetPubScore PROTO ((ValNodePtr pub));
ValNodePtr LookForPubsOnFeat PROTO ((SeqFeatPtr sfp, ValNodePtr PubOnFeat));



SegmentPtr SegmentNew(Int2 range)

{
	Int2 index;
	SegmentPtr new_segp;

	new_segp = (SegmentPtr) MemNew(sizeof(Segment));
	new_segp->quality = (Uint1 PNTR) MemNew(range*sizeof(Uint1));;
	new_segp->count = 0;
	new_segp->count_set = 0;
	new_segp->range = range;
	new_segp->bsp = (BioseqPtr PNTR) MemNew(range * sizeof(BioseqPtr));
	for (index=0; index<range; index++)
		new_segp->quality[index] = DUMP;

	return new_segp;
}

void SegmentFree (SegmentPtr segp)

{
	MemFree(segp->bsp);
	MemFree(segp->quality);
	MemFree(segp);
}

BiotablePtr BiotableNew (Int2 range)

{
	BiotablePtr new_btp;
	Int2 count;

	new_btp = (BiotablePtr) MemNew(sizeof(Biotable));
	new_btp->bsp = (BioseqPtr PNTR) MemNew((range+1) * sizeof(BioseqPtr));
	new_btp->table = (BioseqPtr PNTR) MemNew((range+1) * sizeof(BioseqPtr));
	new_btp->bcp = 
	  (BioseqContextPtr PNTR) MemNew((range+1) * sizeof(BioseqContextPtr));
	new_btp->bcp_na = 
	  (BioseqContextPtr PNTR) MemNew((range+1) * sizeof(BioseqContextPtr));
	new_btp->bcp_na_alloc = (Uint1 PNTR) MemNew((range+1)*sizeof(Uint1));
	new_btp->cds = (SeqFeatPtr PNTR) MemNew((range+1) * sizeof(SeqFeatPtr));
	new_btp->FlatLoc_context = FLATLOC_CONTEXT_LOC;
	new_btp->sfp_out = MakeSyntheticSeqFeat();
	new_btp->sfpCommsize = (Int2Ptr) MemNew((range+1) * sizeof(Int2));
	new_btp->sfpCommlist = 
	   (SeqFeatPtr PNTR PNTR) MemNew((range+1) * sizeof(SeqFeatPtr PNTR));
	new_btp->sfpGenesize = (Int2Ptr) MemNew((range+1) * sizeof(Int2));
	new_btp->sfpGenelist = 
	   (SeqFeatPtr PNTR PNTR) MemNew((range+1) * sizeof(SeqFeatPtr PNTR));
	new_btp->sfpListsize = (Int2Ptr) MemNew((range+1) * sizeof(Int2));
	new_btp->sfpList = 
	   (SeqFeatPtr PNTR PNTR) MemNew((range+1) * sizeof(SeqFeatPtr PNTR));
	new_btp->sfpOrgsize = (Int2Ptr) MemNew((range+1) * sizeof(Int2));
	new_btp->sfpOrglist = 
	   (SeqFeatPtr PNTR PNTR) MemNew((range+1) * sizeof(SeqFeatPtr PNTR));
	new_btp->sfpSitesize = (Int2Ptr) MemNew((range+1) * sizeof(Int2));
	new_btp->sfpSitelist = 
	   (SeqFeatPtr PNTR PNTR) MemNew((range+1) * sizeof(SeqFeatPtr PNTR));
	new_btp->sfpSourcesize = (Int2Ptr) MemNew((range+1) * sizeof(Int2));
	new_btp->sfpSourcelist = 
	   (SeqFeatPtr PNTR PNTR) MemNew((range+1) * sizeof(SeqFeatPtr PNTR));
	new_btp->sfpXrefsize = (Int2Ptr) MemNew((range+1) * sizeof(Int2));
	new_btp->sfpXreflist = 
	   (SeqFeatPtr PNTR PNTR) MemNew((range+1) * sizeof(SeqFeatPtr PNTR));
	new_btp->base_cnt_line = (CharPtr PNTR) MemNew((range+1) * sizeof(CharPtr));
	for (count=0; count<=range; count++)
	{
		new_btp->cds[count] = NULL;
		new_btp->table[count] = NULL;
		new_btp->bcp_na[count] = NULL;
		new_btp->base_cnt_line[count] = NULL;
		new_btp->sfpGenesize[count] = 0;
		new_btp->sfpListsize[count] = 0;
		new_btp->sfpOrgsize[count] = 0;
		new_btp->sfpSitesize[count] = 0;
		new_btp->sfpSourcesize[count] = 0;
		new_btp->sfpXrefsize[count] = 0;
		new_btp->bcp_na_alloc[count] = ASN2FLAT_NOT_ALLOC;
	}	
	new_btp->buffer = (CharPtr) MemNew((MAX_BTP_BUF+2)*sizeof(Char));
	new_btp->line_prefix = (CharPtr) MemNew(3*sizeof(Char));
	new_btp->newline = NEWLINE;
	new_btp->access = (CharPtr PNTR) MemNew((range+1) * sizeof(CharPtr));
	new_btp->locus = (CharPtr PNTR) MemNew((range+1) * sizeof(CharPtr));
	new_btp->division = (CharPtr PNTR) MemNew((range+1) * sizeof(CharPtr));
	new_btp->date = (CharPtr PNTR) MemNew((range+1) * sizeof(CharPtr));
	new_btp->create_date = (CharPtr PNTR) MemNew((range+1) * sizeof(CharPtr));
	new_btp->update_date = (CharPtr PNTR) MemNew((range+1) * sizeof(CharPtr));
	new_btp->embl_rel = (CharPtr PNTR) MemNew((range+1) * sizeof(CharPtr));
	new_btp->embl_ver = (Int2Ptr) MemNew((range+1)*sizeof(Int4));
	new_btp->gi = (Int4Ptr) MemNew((range+1)*sizeof(Int4));
	new_btp->show_gi = TRUE;
	new_btp->Pub = 
	   (ValNodePtr PNTR) MemNew((range+1)*sizeof(ValNodePtr));
	new_btp->seg_num = (Int2Ptr) MemNew((range+1) * sizeof(Int2));
	new_btp->seg_total = (Int2Ptr) MemNew((range+1) * sizeof(Int2));
	for (count=0; count<=range; count++)
	{
		new_btp->access[count] = (CharPtr) MemNew((MAX_ACCESSION_LEN+1) * sizeof(Char));
		new_btp->locus[count] = (CharPtr) MemNew(20 * sizeof(Char));
		new_btp->date[count] = (CharPtr) MemNew(12 * sizeof(Char));
		new_btp->division[count] = NULL;
		new_btp->create_date[count]=NULL;
		new_btp->update_date[count]=NULL;
		new_btp->embl_rel[count]=NULL;
		new_btp->embl_ver[count] = -1;
		new_btp->Pub[count] = NULL;
		new_btp->seg_num[count] = 0;
		new_btp->seg_total[count] = 0;
		new_btp->gi[count] = -1;
	}
	new_btp->count = range;
	new_btp->gspp = (GeneStructPtr PNTR PNTR) MemNew((range+1)*sizeof(GeneStructPtr PNTR));
	new_btp->nspp = (NoteStructPtr PNTR PNTR) MemNew((range+1)*sizeof(NoteStructPtr PNTR));
	new_btp->source_notes = NoteStructNew(new_btp->source_notes);
	new_btp->spp = NULL;

	return new_btp;
}

void BiotableFree (BiotablePtr btp)

{

	Int2 count, index, range=btp->count, size;
	ValNodePtr vnp;

	MemFree(btp->bsp);
	for (count=0; count<=range; count++)
	{
		BioseqContextFree(btp->bcp[count]);
		if (btp->bcp_na_alloc[count] == ASN2FLAT_ALLOC)
			BioseqContextFree(btp->bcp_na[count]);
	}
	MemFree(btp->bcp);
	MemFree(btp->bcp_na);
	MemFree(btp->bcp_na_alloc);
	MemFree(btp->table);
	MemFree(btp->cds);
	NoteStructFree(btp->source_notes);
	for (count=0; count<=range; count++)
	{
		size=btp->sfpListsize[count];
		size++;
		if (btp->gspp[count])
		{
			for (index=0; index<size; index++)
				GeneStructFree(btp->gspp[count][index]);
			btp->gspp[count] = MemFree(btp->gspp[count]);
		}
		if (btp->nspp[count])
		{
			for (index=0; index<size; index++)
				NoteStructFree(btp->nspp[count][index]);
			btp->nspp[count] = MemFree(btp->nspp[count]);
		}
		if (btp->base_cnt_line[count])
			btp->base_cnt_line[count] = MemFree(btp->base_cnt_line[count]);
	}
	btp->gspp = MemFree(btp->gspp);
	btp->nspp = MemFree(btp->nspp);
	btp->base_cnt_line = MemFree(btp->base_cnt_line);
	btp->sfp_out->comment=NULL;
	btp->sfp_out->location=NULL;
	btp->sfp_out->product=NULL;
	SeqFeatFree(btp->sfp_out);
	ListFree(btp->sfpList, range);
	ListFree(btp->sfpCommlist, range);
	ListFree(btp->sfpGenelist, range);
	ListFree(btp->sfpOrglist, range);
	ListFree(btp->sfpSitelist, range);
	ListFree(btp->sfpSourcelist, range);
	ListFree(btp->sfpXreflist, range);
	MemFree(btp->sfpListsize);
	MemFree(btp->sfpCommsize);
	MemFree(btp->sfpGenesize);
	MemFree(btp->sfpOrgsize);
	MemFree(btp->sfpSourcesize);
	MemFree(btp->sfpSitesize);
	MemFree(btp->sfpXrefsize);
	MemFree(btp->buffer);
	MemFree(btp->line_prefix);
	for (count=0; count<=range; count++)
	{
		MemFree(btp->access[count]);
		MemFree(btp->locus[count]);
		MemFree(btp->division[count]);
		MemFree(btp->date[count]);
		MemFree(btp->update_date[count]);
		MemFree(btp->create_date[count]);
	 	btp->embl_rel[count] = NULL;
		for (vnp=btp->Pub[count]; vnp; vnp=vnp->next)
			FreePubStruct(vnp->data.ptrvalue);
		ValNodeFree(btp->Pub[count]);
	}
	MemFree(btp->Pub);
	MemFree(btp->access);
	MemFree(btp->locus);
	MemFree(btp->division);
	MemFree(btp->date);
	MemFree(btp->create_date);
	MemFree(btp->update_date);
	MemFree(btp->embl_rel);
	MemFree(btp->embl_ver);
	MemFree(btp->gi);
	MemFree(btp->seg_num);
	MemFree(btp->seg_total);
	MemFree(btp->line_return);
	SeqPortFree(btp->spp);
	MemFree(btp->sepp);
	MemFree(btp);
}

GeneStructPtr GeneStructNew (GeneStructPtr gsp)

{
	gsp = (GeneStructPtr) MemNew(sizeof(GeneStruct));
	gsp->gene = NULL;
	gsp->product = NULL;
	gsp->standard_name = NULL;
	gsp->map = (CharPtr PNTR) MemNew(sizeof(CharPtr));
	gsp->map_index = 0;
	gsp->map_size = 1;
	gsp->ECNum = NULL;
	gsp->matchlist_size = 0;

	return gsp;
}

NoteStructPtr NoteStructNew (NoteStructPtr nsp)

{
	nsp = (NoteStructPtr) MemNew(sizeof(NoteStruct));

	nsp->note = (CharPtr PNTR) MemNew(5*sizeof(CharPtr));
	nsp->note_annot = (CharPtr PNTR) MemNew(5*sizeof(CharPtr));
	nsp->note_alloc = (Uint1 PNTR) MemNew(5*sizeof(Uint1));
	nsp->note_index = 0;
	nsp->note_size = 5;

	return nsp;
}

void GeneStructFree (GeneStructPtr gsp)

{
	if (gsp->gene)
		gsp->gene = ValNodeFree(gsp->gene);
	if (gsp->product)
		gsp->product = ValNodeFree(gsp->product);
	if (gsp->standard_name)
		gsp->standard_name = ValNodeFree(gsp->standard_name);
	gsp->map = MemFree(gsp->map);
	if (gsp->ECNum)
		gsp->ECNum = ValNodeFree(gsp->ECNum);
	gsp->genes_matched = MemFree(gsp->genes_matched);
	MemFree(gsp);
}

void NoteStructFree (NoteStructPtr nsp)

{
	Int2 index;

	if (nsp->note_index > 0)
	{
		for (index=0; index<nsp->note_index; index++)
			if (nsp->note_alloc[index] == ASN2FLAT_ALLOC)
				nsp->note[index] = MemFree(nsp->note[index]);
	}
	nsp->note = MemFree(nsp->note);
	nsp->note_annot = MemFree(nsp->note_annot);
	nsp->note_alloc = MemFree(nsp->note_alloc);
	MemFree(nsp);
}

void GeneStructReset (BiotablePtr btp, Int2 count, Int2 ext_index)

{
	GeneStructPtr gsp=btp->gspp[count][ext_index];
	Int2 index, largest=0;

	if (gsp->matchlist_size == 0)
	{
		largest = btp->sfpGenesize[count];
		gsp->genes_matched = (Int2Ptr) MemNew(largest*sizeof(Int2));
		gsp->matchlist_size = largest;

		for (index=0; index<largest; index++)
				gsp->genes_matched[index] = -1;
	}

	if (gsp->gene)
		gsp->gene = ValNodeFree(gsp->gene);
	if (gsp->product)
		gsp->product = ValNodeFree(gsp->product);
	if (gsp->standard_name)
		gsp->standard_name = ValNodeFree(gsp->standard_name);
	for (index=0; index<gsp->map_size; index++)
		gsp->map[index] = NULL;
	if (gsp->ECNum)
		gsp->ECNum = ValNodeFree(gsp->ECNum);
}

void NoteStructReset (NoteStructPtr nsp)

{
	Int2 index;

	if (nsp->note_index > 0)
	{
		for (index=0; index<nsp->note_index; index++)
		{
			if (nsp->note_alloc[index] == ASN2FLAT_ALLOC)
				nsp->note[index] = MemFree(nsp->note[index]);
			nsp->note[index] = NULL;
			nsp->note_annot[index] = NULL;
		}
	}
	nsp->note_index = 0;
}


void ListFree (SeqFeatPtr PNTR PNTR List, Int2 range)

{
	Int2 index;

	for (index=0; index<range; index++)
		MemFree(List[index]);

	MemFree(List);
}

/***********************************************************************
*static SeqFeatPtr MakeSyntheticSeqFeat(void) 
*
*	This function allocates a "synthetic" SeqFeatPtr, which is
*	used to print the SeqFeats out.  To print out SeqFeats, they
*	are copied to this "synthetic" sfp, which is an ImpFeat, 
*	adjusted, validated, and then a function prints out this ImpFeat.
*************************************************************************/

static SeqFeatPtr MakeSyntheticSeqFeat(void) 

{
	ImpFeatPtr ifp;
	SeqFeatPtr sfp_out;

	sfp_out = SeqFeatNew();
	if (sfp_out)
	{
		sfp_out->data.choice = 8;
		sfp_out->qual = NULL;
		ifp = sfp_out->data.value.ptrvalue = ImpFeatNew();
		ifp->key = (CharPtr) MemNew(20*sizeof(Char));
		ifp->loc = NULL;
		sfp_out->comment = NULL;
		sfp_out->location = NULL;
		sfp_out->product = NULL;
	}

	return sfp_out;
}

void CpNoteToCharPtrStack (NoteStructPtr nsp, CharPtr annot, CharPtr string)

{
	NoteToCharPtrStack(nsp, annot, string, ASN2FLAT_NOT_ALLOC);
	return;
}

void SaveNoteToCharPtrStack (NoteStructPtr nsp, CharPtr annot, CharPtr string)

{
	NoteToCharPtrStack(nsp, annot, string, ASN2FLAT_ALLOC);
	return;
}


void NoteToCharPtrStack (NoteStructPtr nsp, CharPtr annot, CharPtr string, Uint1 alloc)

{
	Int2 index, note_size;

	if (nsp)
	{
		note_size = nsp->note_size;
		index = nsp->note_index;
	}
	else
		return;

	if (index == note_size)
		EnlargeCharPtrStack(nsp, 5);

	nsp->note_annot[index] = annot;

	if (alloc == ASN2FLAT_NOT_ALLOC)
	{
		nsp->note_alloc[index] = ASN2FLAT_NOT_ALLOC;
		nsp->note[index] = string;
	}
	else if (alloc == ASN2FLAT_ALLOC)
	{
		nsp->note_alloc[index] = ASN2FLAT_ALLOC;
		nsp->note[index] = StringSave(string);
	}

	nsp->note_index++;

	return;
}

void EnlargeCharPtrStack (NoteStructPtr nsp, Int2 enlarge)

{
	CharPtr PNTR new;
	CharPtr PNTR new_annot;
	Int2 index;
	Uint1 PNTR new_alloc;

	new = (CharPtr PNTR) MemNew((size_t) ((enlarge+(nsp->note_size))*sizeof(CharPtr)));
	new_annot = (CharPtr PNTR) MemNew((size_t) ((enlarge+(nsp->note_size))*sizeof(CharPtr)));
	new_alloc = (Uint1 PNTR) MemNew((size_t) ((enlarge+(nsp->note_size))*sizeof(Uint1)));

	for (index=0; index<(nsp->note_size); index++)
	{
		new[index] = nsp->note[index];
		new_annot[index] = nsp->note_annot[index];
		new_alloc[index] = nsp->note_alloc[index];
	}

	nsp->note_size += enlarge;

	nsp->note = MemFree(nsp->note);
	nsp->note_annot = MemFree(nsp->note_annot);
	nsp->note_alloc = MemFree(nsp->note_alloc);
	nsp->note = new;
	nsp->note_annot = new_annot;
	nsp->note_alloc = new_alloc;
}

SortStruct PNTR EnlargeSortList(SortStruct PNTR List, Int2 *totalsize, Int2 increment)

{
	SortStruct PNTR NewList;

	if (List == NULL)
	{
		NewList = (SortStruct PNTR) MemNew(increment*sizeof(SortStruct));
		(*totalsize) += increment;
	}
	else
	{
		NewList = (SortStruct PNTR) MemNew(((*totalsize)+increment) * sizeof(SortStruct));
		MemCopy(NewList, List, ((*totalsize) * sizeof(SortStruct)));
		(*totalsize) += increment;
	}
	MemFree(List);
	return NewList;

}	/* EnlargeSortList */

void VnpHeapSort (ValNodePtr PNTR vnp, int (LIBCALLBACK *compar )PROTO ((Nlm_VoidPtr, Nlm_VoidPtr )))	

{
	Int2 index, total;
	ValNodePtr vnp1;
	ValNodePtr PNTR temp;

	total=0;
	for (vnp1 = *vnp; vnp1; vnp1=vnp1->next)
		total++;

	temp = (ValNodePtr PNTR) MemNew(total*sizeof(ValNodePtr));

	index=0;
	for (vnp1 = *vnp; vnp1; vnp1=vnp1->next)
	{
		temp[index] = vnp1;
		index++;
	}

	HeapSort ((VoidPtr) temp, (size_t) index, sizeof(ValNodePtr), compar);

	*vnp = temp[0];
	for (vnp1 = *vnp, index=0; index<(total-1); vnp1=vnp1->next, index++)
	{
		vnp1->next = temp[index+1];
	}
	vnp1 = temp[total-1];
	vnp1->next = NULL;

	temp = MemFree(temp);
}

static ValNodePtr extract_node(ValNodePtr PNTR head, ValNodePtr x)
{
	ValNodePtr	v, p;
	
	if (*head == NULL) {
		return NULL;
	}
	if (x == *head) {
		*head = x->next;
		x->next = NULL;
		return x;
	}
	for (v = *head; v != NULL && v != x; v = v->next) {
		p = v;
	}
	if (v == NULL) {
		return NULL;
	}
	p->next = x->next;
	x->next = NULL;
	return x;
}
static ValNodePtr tie_next(ValNodePtr head, ValNodePtr next)
{
	ValNodePtr v;

	if (head == NULL) {
		return next;
	}
	for (v = head; v->next != NULL; v = v->next) {
		v = v;
	}
	v->next = next;
	return head;
}
static void PubHeapSort (ValNodePtr PNTR vnp, int (LIBCALLBACK *compar )PROTO ((Nlm_VoidPtr, Nlm_VoidPtr )))	

{
	PubStructPtr psp;
	ValNodePtr v, ex_v, v_sub=NULL, v_unp=NULL, vnext;

	for (v = *vnp; v != NULL; v = vnext) {
		vnext = v->next;
		psp = (PubStructPtr) v->data.ptrvalue;
		if(psp->choice == PUB_Sub) {
			ex_v = extract_node(vnp, v);
			v_sub = tie_next(v_sub, ex_v);
		} else if (psp->choice == PUB_Gen) {
			ex_v = extract_node(vnp, v);
			v_unp = tie_next(v_unp, ex_v);
		}
	}		
	if (*vnp) {
		VnpHeapSort(vnp, compar);
	}
	if (v_unp) {
		VnpHeapSort(&(v_unp), compar);
	}
	if (v_sub) {
		VnpHeapSort(&(v_sub), compar);
	}
	
	*vnp = tie_next(*vnp, v_unp);
	*vnp = tie_next(*vnp, v_sub);
	
}

int LIBCALLBACK CompareSfpForHeap (VoidPtr vp1, VoidPtr vp2)

{

	SortStructPtr sp1, sp2;
	BioseqPtr bsp;
	SeqFeatPtr sfp1, sfp2;
	Int2 status;

	sp1 = (SortStructPtr) vp1; 
	sp2 = (SortStructPtr) vp2;

	bsp = sp1->bsp;
	sfp1 = sp1->sfp;
	sfp2 = sp2->sfp;

	status = SeqLocOrder(sfp1->location, sfp2->location, bsp);

	if (ABS(status) >= 2 && sp1->table)
	{
		bsp = sp1->table;
		if (bsp)
		    status = SeqLocOrder(sfp1->location, sfp2->location, bsp);
	}
	else if (ABS(status) >= 2)
	{
		if (sp1->other)
		{
			bsp = sp1->other;
			status = SeqLocOrder(sfp1->location, sfp2->location, bsp);
		}
		else if (sp2->other)
		{
			bsp = sp2->other;
			status = SeqLocOrder(sfp1->location, sfp2->location, bsp);
		}
	}
	return status;
}
int LIBCALLBACK CompareByDate (VoidPtr vp1, VoidPtr vp2)
{

	PubStructPtr psp1, psp2;
	Int2 status;
	ValNodePtr vnp1, vnp2;
	ValNodePtr PNTR vnpp1;
	ValNodePtr PNTR vnpp2;

	vnpp1 = (ValNodePtr PNTR) vp1;
	vnpp2 = (ValNodePtr PNTR) vp2;
	vnp1 = *vnpp1;
	vnp2 = *vnpp2;

	psp1 = vnp1->data.ptrvalue;
	psp2 = vnp2->data.ptrvalue;

/* First, three cases that should not be, need to be checked for. */
	if (psp1 == NULL && psp2 != NULL)
		return 1;
	if (psp1 != NULL && psp2 == NULL)
		return -1;
	if (psp1 == NULL && psp2 == NULL)
		return 0;
	status = DateMatch(psp1->date, psp2->date, FALSE);
	if (status == -2) {
		status = 2;
	}

	return status;
}

/***************************************************************************
*
*	"CompareSnForHeap" compares serial numbers, if they exist,
*	to sort pubs in that manner.
*
***************************************************************************/

int LIBCALLBACK CompareSnForHeap (VoidPtr vp1, VoidPtr vp2)

{
	PubStructPtr psp1, psp2;
	ValNodePtr vnp1, vnp2;
	ValNodePtr PNTR vnpp1;
	ValNodePtr PNTR vnpp2;

	vnpp1 = (ValNodePtr PNTR) vp1;
	vnpp2 = (ValNodePtr PNTR) vp2;
	vnp1 = *vnpp1;
	vnp2 = *vnpp2;

	psp1 = vnp1->data.ptrvalue;
	psp2 = vnp2->data.ptrvalue;

/* First, three cases that should not be, need to be checked for. */
	if (psp1 == NULL && psp2 != NULL)
		return 1;
	if (psp1 != NULL && psp2 == NULL)
		return -1;
	if (psp1 == NULL && psp2 == NULL)
		return 0;

	if (psp1->number > psp2->number)
		return 1;
	else if (psp1->number < psp2->number)
		return -1;
	else 
		return 0;
/* Two pubs should not have the same serial number, but that case is covered 
anyway by the line above.*/
}

/***************************************************************************
*
*	"CompareStartForHeap" compares "start" numbers, found in
*	the PubStruct, so that pubs are sorted into the order
*	descrip, cit, and feat.
*
***************************************************************************/
int LIBCALLBACK CompareStartForHeap (VoidPtr vp1, VoidPtr vp2)

{
	PubStructPtr psp1, psp2;
	ValNodePtr vnp1, vnp2;
	ValNodePtr PNTR vnpp1;
	ValNodePtr PNTR vnpp2;

	vnpp1 = (ValNodePtr PNTR) vp1;
	vnpp2 = (ValNodePtr PNTR) vp2;
	vnp1 = *vnpp1;
	vnp2 = *vnpp2;

	psp1 = vnp1->data.ptrvalue;
	psp2 = vnp2->data.ptrvalue;


/* First, three cases that should not be, need to be checked for. */
	if (psp1 == NULL && psp2 != NULL)
		return 1;
	if (psp1 != NULL && psp2 == NULL)
		return -1;
	if (psp1 == NULL && psp2 == NULL)
		return 0;

	if (psp1->start > psp2->start)
		return 1;
	else if (psp1->start < psp2->start)
		return -1;
	else 	/* If the two start values are equal */
		return 0;

}

/***********************************************************************
*
*	FillSegmentArrays populates the fields seg_num and seg_total,
*	specifying the segment number and the total number of segments,
*	in the BiotablePtr.
*
************************************************************************/

void FillSegmentArrays (BiotablePtr btp_na, BiotablePtr btp_aa)

{
	Int2 count, total;

	if (btp_na->count > 0)
	{
		for (count=0, total=0; count<(btp_na->count); count++)
		{
			if (btp_na->table[count] == NULL)
				total = 0;
			else if (btp_na->table[count] != NULL && total == 0)
				total++;
			else if (btp_na->table[count] == btp_na->table[count-1])
				total++;
			btp_na->seg_num[count] = total;
		}
	
		for (count=((btp_na->count)-1), total=0; count>=0; count--)
		{
			if (btp_na->seg_num[count] == 0)
				total = 0;
			else if (btp_na->seg_num[count] > total)
				total = btp_na->seg_num[count];
			btp_na->seg_total[count] = total;

			if (total == 1)
			{
				btp_na->seg_total[count] = 0;
				btp_na->seg_num[count] = 0;
			}
			if (btp_na->seg_num[count] == 1)
				total=0;
		}
	}

	if (btp_aa->count > 0)
	{
		for (count=0, total=0; count<(btp_aa->count); count++)
		{
			if (btp_aa->table[count] == NULL)
				total = 0;
			else if (btp_aa->table[count] != NULL && total == 0)
				total++;
			else if (btp_aa->table[count] == btp_aa->table[count-1])
				total++;
			btp_aa->seg_num[count] = total;
		}
	
		for (count=((btp_aa->count)-1), total=0; count >= 0; count--)
		{
			if (btp_aa->seg_num[count] == 0)
				total = 0;
			else if (btp_aa->seg_num[count] > total)
				total = btp_aa->seg_num[count];
			btp_aa->seg_total[count] = total;

			if (total == 1)
			{
				btp_aa->seg_total[count] = 0;
				btp_aa->seg_num[count] = 0;
			}
			if (btp_aa->seg_num[count] == 1)
				total=0;
		}
	}
	return;
}	/* FillSegmentArrays */

/***********************************************************************
*
*	GetBCPs gets the BioseqContextPtrs
*
************************************************************************/

void GetBCPs (BiotablePtr btp_na, BiotablePtr btp_aa)

{
	BioseqPtr bsp;
	Int2 count;
	
	for (count=0; count<(btp_na->count); count++)
	{
		bsp = btp_na->bsp[count];
		btp_na->bcp[count] = BioseqContextNew(bsp);
	}

	for (count=0; count<(btp_aa->count); count++)
	{
		bsp = btp_aa->bsp[count];
		btp_aa->bcp[count] = BioseqContextNew(bsp);
	}

	return;
}	/* GetBCPs */

/**************************************************************************
*	void GetGINumber (BiotablePtr btp, Int2 count)
*
*	btp: BiotablePtr,
*	count: index of the entry in the btp.
*
*	This function places the gi number in the "gi" slot in the btp.
*	If no gi number is found, -1 is places there, and a warning is
*	issued.
**************************************************************************/

void GetGINumber (BiotablePtr btp, Int2 count)

{
	BioseqPtr bsp;
	Boolean found;
	ValNodePtr vnp;

	found = FALSE;
	bsp = btp->bsp[count];
	for (vnp=bsp->id; vnp; vnp=vnp->next)
	{
		if (vnp->choice == SEQID_GI)
		{
			btp->gi[count] = vnp->data.intvalue;
			found = TRUE;
			break;
		}
	}
	if (! found)
	{
		btp->gi[count] = -1;
		if (btp->error_msgs == TRUE)
		{
			ErrPostEx(SEV_WARNING, ERR_GI_No_GI_Number, "");
		}
	}
}

/***********************************************************************
*
*	GetGIs gets the GI's.
*
************************************************************************/

void GetGIs (BiotablePtr btp_na, BiotablePtr btp_aa)

{
	BioseqPtr bsp;
	Int2 count;
	
	if (btp_na->show_gi == TRUE)
		for (count=0; count<(btp_na->count); count++)
			GetGINumber(btp_na, count);

	if (btp_aa->show_gi == TRUE)
		for (count=0; count<(btp_aa->count); count++)
			GetGINumber(btp_aa, count);

	return;
}	/* GetGIs */

/*********************************************************************
* CountTheSeqEntrys (SeqEntryPtr sep)
*
*******************************************************************/

void CountTheSeqEntrys (SeqEntryPtr sep, Pointer data, Int4 index, Int2 indent)
{
	BioseqPtr bsp;
	SegmentPtr segp;
	SeqLocPtr slp;

	bsp = (BioseqPtr) sep->data.ptrvalue;
	segp = (SegmentPtr) data;
	if (bsp->seq_ext_type == 1 && bsp->repr == Seq_repr_seg)
	{
		slp = bsp->seq_ext;
		while (slp)
		{
			bsp = BioseqFind(SeqLocId(slp));		
			if (bsp != NULL)
				segp->count++;
			slp = slp->next;
		}
		segp->count++;
	}
	else
	{
		segp->count++;
	}
	return;
}

/****************************************************************************
*
*	"CatSfpStacks" concatenates two SeqFeatPtr PNTR's and returns
*	the result to replace sfpp1.
*
****************************************************************************/

SeqFeatPtr PNTR CatSfpStacks 
(SeqFeatPtr PNTR sfpp1, Int2 *count1, SeqFeatPtr PNTR sfpp2, Int2 count2)

{

	Int2 i, j, count=(*count1);
	SeqFeatPtr PNTR tmp;
	Boolean redundant;

	if (count2 == 0)
		return sfpp1;

	tmp = (SeqFeatPtr PNTR) MemNew((*count1+count2)*sizeof(SeqFeatPtr));

	for (i=0; i<count; i++)
		tmp[i] = sfpp1[i];

	for (i=0; i<count2; i++)
	{
		redundant = FALSE;
		for (j=0; j<count; j++)
		{
			if (sfpp1[j] == sfpp2[i])
			{
				redundant = TRUE;
				break;
			}
		}
		if (redundant == FALSE)
		{
			tmp[(*count1)] = sfpp2[i];
			(*count1)++;
		}
	}

	if (count != 0)
		MemFree(sfpp1);

	return tmp;
}

/****************************************************************************
*
*	"CatBspStacks" concatenates two BioseqPtr PNTR's and returns
*	the result to replace bspp1.
*
****************************************************************************/

BioseqPtr PNTR CatBspStacks 
(BioseqPtr PNTR bspp1, Int2 *count1, BioseqPtr PNTR bspp2, Int2 count2)

{

	Int2 i, j, count=(*count1);
	BioseqPtr PNTR tmp;
	Boolean redundant;

	if (count2 == 0)
		return bspp1;

	tmp = (BioseqPtr PNTR) MemNew((*count1+count2)*sizeof(BioseqPtr));

	for (i=0; i<count; i++)
		tmp[i] = bspp1[i];

	for (i=0; i<count2; i++)
	{
		redundant = FALSE;
		for (j=0; j<count; j++)
		{
			if (bspp1[j] == bspp2[i])
			{
				redundant = TRUE;
				break;
			}
		}
		if (redundant == FALSE)
		{
			tmp[(*count1)] = bspp2[i];
			(*count1)++;
		}
	}

	if (count != 0)
		MemFree(bspp1);

	return tmp;
}

void AddPubBsp (PubStructPtr psp, BioseqPtr bsp)

{
	Boolean match=FALSE;
	Int2 i, count=psp->bspcount;
	BioseqPtr PNTR tmp;

	if (count != 0)
	{ /* Look to see if this BioseqPtr is already in the list. */
		for (i=0; i<count; i++)
			if (bsp == psp->bsp[i])
			{
				match = TRUE;
				break;
			}
	}
				
	if (match == FALSE)
	{
		if (count != 0)
		{
			tmp = (BioseqPtr PNTR) MemNew(count*sizeof(BioseqPtr));
			for (i=0; i<count; i++)
				tmp[i] = psp->bsp[i];
	
			MemFree(psp->bsp);
		}
	
		psp->bsp = 
			(BioseqPtr PNTR) MemNew((count+1)*sizeof(BioseqPtr));
	
		if (count != 0)
		{
			for (i=0; i<count; i++)
				psp->bsp[i] = tmp[i];
	
			MemFree(tmp);
		}
	
		psp->bsp[count] = bsp;
		(psp->bspcount)++;
	}

	return;
}

void AddCitFeat (PubStructPtr psp, SeqFeatPtr sfp)

{
	Int2 i, count=psp->citcount;
	SeqFeatPtr PNTR tmp;


	if (count != 0)
	{
		tmp = (SeqFeatPtr PNTR) MemNew(count*sizeof(SeqFeatPtr));
		for (i=0; i<count; i++)
			tmp[i] = psp->citfeat[i];

		MemFree(psp->citfeat);
	}

	psp->citfeat = 
		(SeqFeatPtr PNTR) MemNew((count+1)*sizeof(SeqFeatPtr));

	if (count != 0)
	{
		for (i=0; i<count; i++)
			psp->citfeat[i] = tmp[i];

		MemFree(tmp);
	}

	psp->citfeat[count] = sfp;
	(psp->citcount)++;

	return;

}

void AddPubFeat (PubStructPtr psp, SeqFeatPtr sfp)

{
	Boolean match=FALSE;
	Int2 i, count=psp->pubcount;
	SeqFeatPtr PNTR tmp;

	if (count != 0)
	{ /* Look to see if this SeqFeatPtr is already in the list. */
		for (i=0; i<count; i++)
			if (sfp == psp->pubfeat[i])
			{
				match = TRUE;
				break;
			}
	}
				
	if (match == FALSE)
	{
		if (count != 0)
		{
			tmp = (SeqFeatPtr PNTR) MemNew(count*sizeof(SeqFeatPtr));
			for (i=0; i<count; i++)
				tmp[i] = psp->pubfeat[i];
	
			MemFree(psp->pubfeat);
		}
	
		psp->pubfeat = 
			(SeqFeatPtr PNTR) MemNew((count+1)*sizeof(SeqFeatPtr));
	
		if (count != 0)
		{
			for (i=0; i<count; i++)
				psp->pubfeat[i] = tmp[i];
	
			MemFree(tmp);
		}
	
		psp->pubfeat[count] = sfp;
		(psp->pubcount)++;
	}

	return;

}

void FreePubStruct(PubStructPtr psp)

{
	if (psp)
	{
		if (psp->bspcount)
			MemFree(psp->bsp);
		if (psp->citcount)
			MemFree(psp->citfeat);
		if (psp->pubcount)
			MemFree(psp->pubfeat);
		if (psp->descr)
		{
			psp->descr->comment = NULL;
			psp->descr->fig = NULL;
			psp->descr->maploc = NULL;
			psp->descr->name = NULL;
			psp->descr->num = NULL;
			psp->descr->pub = NULL;
			psp->descr->seq_raw = NULL;
			psp->descr = PubdescFree(psp->descr);
		}
		if (psp->date)
			DateFree(psp->date);
		psp = MemFree(psp);
	}
	return;
}

SeqIdPtr GetProductSeqId(ValNodePtr product)

{
	SeqIdPtr sip=NULL;
	SeqIntPtr seq_int;

	if (product)
	{
		if (product->choice == SEQLOC_WHOLE)
		{
			sip = (SeqLocPtr) product->data.ptrvalue;
		} 
		else if (product->choice == SEQLOC_INT)
		{
			seq_int = (SeqIntPtr) product->data.ptrvalue;
			sip = seq_int->id;
		}
	}
	return sip;
}

/*****************************************************************************
*check_range
*
*	This function is called by the gbparse functions of Karl Sirotkin 
*	and determines if the length of a BioSeqPtr is sensible.
*
*	Tom Madden
*****************************************************************************/

Int4 check_range(Pointer data, SeqIdPtr seq_id)

{
	BioseqPtr bsp;

	bsp = BioseqFind(seq_id);

	if (bsp)
		return bsp->length;
	else
		return 0;
}	/* check_range */

/****************************************************************************
*do_loc_errors
*
*	This function is called both by the gbparse functions of Karl Sirotkin
*	and by asn2ff.  If called by gbparse, error messages are stored in 
*	buffers and a flag is set; if called by asn2ff, the error messages
*	are retrieved and the flag reset.
*	
***************************************************************************/

void do_loc_errors(CharPtr front, CharPtr details)
{
	ErrPostEx(SEV_INFO, ERR_FEATURE_Bad_location, "%s: %s\n", front, details);
}

/***************************************************************************
*do_no_loc_errors
*
*	Is used when no error messages are wanted.
****************************************************************************/

void do_no_loc_errors(CharPtr front, CharPtr details)
{
	return;
}

/***************************************************************************
*Boolean GBQualPresent(CharPtr ptr, GBQualPtr gbqual)
*
*This function check that a qual, that is to be added to the list of qual
*isn't already present.
***************************************************************************/
Boolean GBQualPresent(CharPtr ptr, GBQualPtr gbqual)

{
	Boolean present=FALSE;
	GBQualPtr qual;

	for (qual=gbqual; qual; qual=qual->next)
		if (StringCmp(ptr, qual->qual) == 0)
		{
			present = TRUE;
			break;
		}

	return present;
}	/* GBQualPresent */

/**********************************************************************
*Boolean GetNAFeatKey(CharPtr buffer, SeqFeatPtr sfp)
*
*	This function places the sfp "key" in buffer and returns TRUE
*	if successful, it returns FALSE if not successful.
*	This function only works for nucleic acid sequences, as the
*	keys (for corresponding numbers) are different for peptides.
***********************************************************************/

Boolean GetNAFeatKey(CharPtr buffer, SeqFeatPtr sfp)

{

	Boolean retval=TRUE;
	ImpFeatPtr ifp;
	RnaRefPtr rrp;


	switch (sfp->data.choice)
	{
	case SEQFEAT_GENE:	/* gene becomes misc_feat for purposes of CheckNAFeat */
		buffer = StringCpy(buffer, "misc_feature");
		break;
	case SEQFEAT_CDREGION:
		buffer = StringCpy(buffer, "CDS");
		break;
	case SEQFEAT_RNA:
		rrp = sfp->data.value.ptrvalue;
		/* the following code was taken (almost) directly from Karl
		Sirotkin's code.					*/
		switch ( rrp -> type){ /* order of case n: matches tests in
                                is_RNA_type() of genasn.c in
                                GenBankConversion directory */
			case 2:
				buffer = StringCpy(buffer, "mRNA");
				break;
			case 255:
				if (rrp ->ext.choice == 1){
					buffer = StringCpy(buffer, rrp->ext.value.ptrvalue);
				}else{
					buffer = StringCpy(buffer, "misc_RNA");
				}
				break;
			case 3:
				buffer = StringCpy(buffer, "tRNA");
				break;
			case 4:
				buffer = StringCpy(buffer, "rRNA");
				break;
			case 1:
				buffer = StringCpy(buffer, "prim_transcript");
				break;
			case 5:
				buffer = StringCpy(buffer, "snRNA");
				break;
			case 6:
				buffer = StringCpy(buffer, "scRNA");
				break;
		}
		break;
	case SEQFEAT_IMP:
		ifp = (ImpFeatPtr) sfp->data.value.ptrvalue;
		buffer = StringCpy(buffer, ifp->key); 
		break;
	case SEQFEAT_SEQ:
	case SEQFEAT_REGION:
	case SEQFEAT_COMMENT:
	case SEQFEAT_RSITE:
		buffer = StringCpy(buffer, "misc_feature");
		break;
	case SEQFEAT_BIOSRC:
		buffer = StringCpy(buffer, "source");
		break;
	default:
		retval = FALSE;	
		break;
	}

	return retval;
}	/* GetNAFeatKey */

/**************************************************************************
*SeqIdPtr CheckXrefFeat (BioseqPtr bsp, SeqFeatPtr sfp)
*
* 	First the location of the xref is checked to see if it overlaps 
*	the sequence.  If this feature has a xref that is NOT of type 
*	genbank, embl, or ddbj, it is put out as a misc_feat.  If it's 
*	one of genbank, embl, or ddbj, it has been put out as a second 
*	accession.  If the feature should be put out as a misc_feat, then
*	the SeqIdPtr (xid) is returned, otherwise NULL.
**************************************************************************/

SeqIdPtr CheckXrefFeat (BioseqPtr bsp, SeqFeatPtr sfp)

{

	SeqIdPtr xid=NULL;
	SeqIntPtr si;
	SeqLocPtr xref;
	ValNodePtr location;
			
	location = ValNodeNew(NULL);
	si = SeqIntNew();
	location->choice = SEQLOC_INT;
	location->data.ptrvalue = si;
	si->from = 0;
	si->to = bsp->length - 1;
	si->id = bsp->id;	/* Don't delete id!! */
	if (SeqLocCompare(sfp->location, location) != 0)
	{
		xref = (SeqLocPtr) sfp->data.value.ptrvalue;
		xid = (SeqIdPtr) xref->data.ptrvalue;
		if (xid->choice != 5 && xid->choice != 6 && xid->choice != 13)
			;
		else
			xid = NULL;
	}
	si->id = NULL;
	SeqIntFree(si);
	ValNodeFree(location);
	return xid;
}

Int4 GetGINumFromSip (SeqIdPtr sip)

{
	Int4 gi = -1;
	ValNodePtr vnp;

	for (vnp=sip; vnp; vnp=vnp->next)
		if (vnp->choice == SEQID_GI)
			gi = vnp->data.intvalue;

	return gi;
}


ValNodePtr StorePubFeat(ValNodePtr PubOnFeat, SeqFeatPtr sfp)

{
	ImpFeatPtr ifp;
	PubdescPtr descr;
	PubStructPtr psp;
	ValNodePtr vnp, PubSet, pub;

	PubSet = (ValNodePtr) sfp->cit;
	pub = (ValNodePtr) PubSet->data.ptrvalue;

	for (vnp=pub; vnp; vnp=vnp->next)
	{
		psp = (PubStructPtr) MemNew(sizeof(PubStruct));
		psp->Pub = pub;
		ValNodeAddPointer(&(PubOnFeat), 0, psp);
		psp->vn.choice = vnp->choice;
		psp->vn.next = NULL;
		psp->vn.data = vnp->data;
		psp->pub = &(psp->vn);
		psp->start = 3;
		AddPubFeat(psp, sfp);
		psp->number = GetSerialNumber(vnp);
		psp->date = GetPubDate(psp->pub);
		psp->choice = GetPubChoice(psp->pub);
		if (sfp->data.choice == 8)
		{
			ifp = (ImpFeatPtr) sfp->data.value.ptrvalue;
			if (StringCmp(ifp->key, "Site-ref") == 0
				&& sfp->comment)
			{
				descr = PubdescNew();
				descr->comment = sfp->comment;
				psp->descr = descr;
			}
		}


	}

	return PubOnFeat;
}

ValNodePtr StorePubCit(ValNodePtr PubOnCit, SeqFeatPtr sfp)

{
	PubdescPtr pdp, descr;
	PubStructPtr psp;

	psp = (PubStructPtr) MemNew(sizeof(PubStruct));
	ValNodeAddPointer(&(PubOnCit), 0, psp);

	pdp = (PubdescPtr) (sfp->data.value.ptrvalue);
	descr = PubdescNew();
	CpPubDescr(descr, pdp);
	psp->descr = descr;
	psp->vn.choice = PUB_Equiv;
	psp->vn.data.ptrvalue = pdp->pub;
	psp->vn.next = NULL;
	psp->pub = &(psp->vn);
	psp->start = 2;
	psp->number = GetSerialNumber(psp->pub);
	psp->date = GetPubDate(psp->pub);
	psp->choice = GetPubChoice(psp->pub);

	AddCitFeat(psp, sfp);

	return PubOnCit;
}

/***************************************************************
*ValNodePtr StoreNAPubCit(ValNodePtr PubOnCit, BioseqPtr bsp, SeqFeatPtr sfp)
*
*	major KLUDGE warning!  This function stores the pub
*	from an sfp as though it were a pub that covered the
*	entire entry.  This is acceptable, as the Pub-feat 
*	(on a nucleotide entry) WILL cover the entire PROTEIN entry.
*
*	To ensure clickability of the GenPept flat file in Sequin,
*	the original SeqFeatPtr is also stored.  This will allow
*	Sequin to identify the pub.  To avoid "misuse", psp->SeqDescr 
*	is set equal to NULL.  These variables are used in GetPapRefPtr.
*	
***************************************************************/

ValNodePtr StoreNAPubCit(ValNodePtr PubOnCit, BioseqPtr bsp, SeqFeatPtr sfp)

{
	PubdescPtr pdp, descr;
	PubStructPtr psp;

	psp = (PubStructPtr) MemNew(sizeof(PubStruct));
	ValNodeAddPointer(&(PubOnCit), 0, psp);

	pdp = (PubdescPtr) (sfp->data.value.ptrvalue);
	descr = PubdescNew();
	CpPubDescr(descr, pdp);
	psp->descr = descr;
	psp->sfp = sfp;
	psp->SeqDescr = NULL;
	psp->vn.choice = PUB_Equiv;
	psp->vn.data.ptrvalue = pdp->pub;
	psp->vn.next = NULL;
	psp->pub = &(psp->vn);
	psp->start = 1;
	psp->number = GetSerialNumber(psp->pub);
	psp->date = GetPubDate(psp->pub);
	psp->choice = GetPubChoice(psp->pub);
	AddPubBsp (psp, bsp);

	return PubOnCit;
}

ValNodePtr StorePubDescr(BioseqPtr bsp, ValNodePtr pubOnDescr, PubdescPtr pdp, ValNodePtr seqDescr)

{
	PubdescPtr descr=NULL;
	PubStructPtr psp;

	psp = (PubStructPtr) MemNew(sizeof(PubStruct));
	ValNodeAddPointer(&(pubOnDescr), 0, psp);
	descr = PubdescNew();
	CpPubDescr(descr, pdp);
	psp->SeqDescr = seqDescr;
	psp->descr = descr;
	psp->vn.choice = PUB_Equiv;
	psp->vn.data.ptrvalue = pdp->pub;
	psp->vn.next = NULL;
	psp->pub = &(psp->vn);
	psp->start = 1;
	psp->number = GetSerialNumber(psp->pub);
	psp->date = GetPubDate(psp->pub);
	psp->choice = GetPubChoice(psp->pub);
	AddPubBsp (psp, bsp);

	return pubOnDescr;
}

/***************************************************************
*static void CpPubDescr (PubdescPtr new_desc, PubdescPtr old_desc)
*
*Copy the old_desc to the new_desc.  No fields are allocated!!
***************************************************************/

static void CpPubDescr (PubdescPtr new_desc, PubdescPtr old_desc)

{
	if (new_desc && old_desc)
	{
		new_desc->pub = old_desc->pub;
		new_desc->name = old_desc->name;
		new_desc->fig = old_desc->fig;
		new_desc->num = old_desc->num;
		new_desc->numexc = old_desc->numexc;
		new_desc->poly_a = old_desc->poly_a;
		new_desc->align_group = old_desc->align_group;
		new_desc->maploc = old_desc->maploc;
		new_desc->seq_raw = old_desc->seq_raw;
		new_desc->comment = old_desc->comment;
	}

	return;
}

Int2 GetSerialNumber (ValNodePtr pub)
{
	CitGenPtr cgp;
	ValNodePtr vnp;

	if (pub == NULL)
		return -1;

	if (pub->choice == PUB_Equiv)
	{
		for (vnp=(pub->data.ptrvalue); vnp != NULL; vnp = vnp->next)
		{
			if (vnp->choice == PUB_Gen)
			{
				cgp = vnp->data.ptrvalue;
				return cgp->serial_number;
			}
		}
	}
	else if (pub->choice == PUB_Gen)
	{
		cgp = pub->data.ptrvalue;
		return cgp->serial_number;
	}
			
	return -1; /* -1 if no serial number available */
}	/* GetSerialNumber*/

Int2 GetPubChoice (ValNodePtr pub)
{
	CitGenPtr cgp;
	ValNodePtr vnp;

	if (pub == NULL)
		return -1;

	if (pub->choice == PUB_Equiv)
	{
		for (vnp=(pub->data.ptrvalue); vnp != NULL; vnp = vnp->next)
		{
			if (vnp->choice == PUB_Gen) {
				if ((cgp = vnp->data.ptrvalue) != NULL) {
					if (cgp->cit != NULL) {
						if (ASN2FF_SHOW_ALL_PUBS) {
							return PUB_Gen;  /* 1 */
						} else if (StringICmp(cgp->cit, "Unpublished") == 0) {
							return PUB_Gen;  /* 1 */
						}
					}
				}
			} else if (vnp->choice == PUB_Sub) {
				return PUB_Sub;  /* 2 */
			}
		}
	}
	else if (pub->choice == PUB_Gen)
	{
		if ((cgp = pub->data.ptrvalue) != NULL) {
			if (cgp->cit != NULL) {
				if (ASN2FF_SHOW_ALL_PUBS) {
					return PUB_Gen;  /* 1 */
				} else if (StringICmp(cgp->cit, "Unpublished") == 0) {
					return PUB_Gen;  /* 1 */
				}
			}
		}
	}
	else if (pub->choice == PUB_Sub)
	{
		return PUB_Sub;  /* 2 */
	}		
	return -1; /* -1 not PUB_Sub and not unpublis */
}	/* GetPubChoice*/

DatePtr GetPubDate (ValNodePtr pub)
{
	ValNodePtr vnp;
	CitGenPtr cgp;
	CitArtPtr cap;
	CitBookPtr cbp;
	CitJourPtr cjp;
	CitSubPtr csp;
	CitPatPtr	cpp;
	ImprintPtr	imp;
	DatePtr		dp = NULL;

	if (pub == NULL) {
		return NULL;
	}
	if (pub->choice == PUB_Equiv)
	{
		for (vnp=pub->data.ptrvalue; vnp != NULL; vnp = vnp->next) {
			switch (vnp->choice) 
			{
				case (PUB_Gen):
					cgp = (CitGenPtr) vnp->data.ptrvalue;
					dp = (DatePtr) cgp->date;
					break;
				case (PUB_Sub):
					csp = (CitSubPtr) vnp->data.ptrvalue;
					imp = (ImprintPtr) csp->imp;
					dp = (DatePtr) imp->date;
					break;
				case (PUB_Article):
					cap = (CitArtPtr) vnp->data.ptrvalue;
					if (cap->from == 1)
					{
							cjp = (CitJourPtr) (cap->fromptr);
							dp = cjp->imp->date;
					} else if (cap->from == 2) {
							cbp = (CitBookPtr) (cap->fromptr);
							dp = cbp->imp->date;
					}
					break;
				case (PUB_Patent):
					cpp = (CitPatPtr) vnp->data.ptrvalue;
					dp = (DatePtr) cpp->date_issue;
					break;
				case (PUB_Equiv):
				case (PUB_Muid):
				case (PUB_Man):
				case (PUB_Proc):
				default:
					break;
			}
			if (dp != NULL) {
				return DateDup(dp);
			}
		}
	} else {
		switch (pub->choice) 
		{
			case (PUB_Gen):
				cgp = (CitGenPtr) pub->data.ptrvalue;
				dp = (DatePtr) cgp->date;
				break;
			case (PUB_Sub):
				csp = (CitSubPtr) pub->data.ptrvalue;
				imp = (ImprintPtr) csp->imp;
				dp = (DatePtr) imp->date;
				break;
			case (PUB_Article):
				cap = (CitArtPtr) pub->data.ptrvalue;
				if (cap->from == 1)
				{
						cjp = (CitJourPtr) (cap->fromptr);
						dp = cjp->imp->date;
				} else if (cap->from == 2) {
						cbp = (CitBookPtr) (cap->fromptr);
						dp = cbp->imp->date;
				}
				break;
			case (PUB_Patent):
				cpp = (CitPatPtr) pub->data.ptrvalue;
				dp = (DatePtr) cpp->date_issue;
				break;
			case (PUB_Equiv):
			case (PUB_Muid):
			case (PUB_Man):
			case (PUB_Proc):
			default:
				break;
		}
	}
	if (dp != NULL) {
		return DateDup(dp);
	}
	return NULL;
}

/**************************************************************************
*
*	"SelectBestPub" selects the best of two publications in terms
*	of the most complete record (e.g., title etc.).  To be used
*	with "UniquePubs"
*	return values are:
*	>0: pub1 is better than pub2
*	 0: pub1 and pub2 are equivalent (i.e., pub1->choice = pub2->choice)
*	<0: pub2 is better than pub1
*
***************************************************************************/

Int2 SelectBestPub (ValNodePtr pub1, ValNodePtr pub2)

{
	CitArtPtr ca=NULL;
	CitJourPtr jp;
	ImprintPtr imp;
	Int2 score1, score2, temp;
	MedlineEntryPtr ml;
	ValNodePtr vnp, int_pub1;

	score1=0;
	if (pub1->choice == PUB_Equiv)
	{
		for (vnp=pub1->data.ptrvalue; vnp; vnp=vnp->next)
		{
			temp = GetPubScore(vnp);
			if (temp > score1)
			{
				score1 = temp;
				int_pub1 = vnp;
			}
		}
	}
	else
	{
		score1 = GetPubScore(pub1);
		int_pub1 = pub1;
	}

	score2=0;
	if (pub2->choice == PUB_Equiv)
	{
		for (vnp=pub2->data.ptrvalue; vnp; vnp=vnp->next)
		{
			temp = GetPubScore(vnp);
			if (temp > score2)
			{
				score2 = temp;
			}
		}
	}
	else
	{
		score2 = GetPubScore(pub2);
	}

/* The following should be improved to encompass other prepub situations
 (i.e., books etc.) as well as (possibly) check for page numbering etc. ????*/

	if (score1 == score2)
	{ /* If the scores are the same, see if number one is a prepub */
		if ( int_pub1 -> choice == 3){
			ml = (MedlineEntryPtr) int_pub1 -> data.ptrvalue;
			ca = (CitArtPtr) ml -> cit;
		}else if (int_pub1->choice == 5) {
			ca = (CitArtPtr) int_pub1 -> data.ptrvalue;
		}
		if(ca && ca ->fromptr && ca ->from ==1)
		{
			jp = (CitJourPtr) ca -> fromptr;
			if ( jp && (imp=jp->imp) != NULL)
				if (imp->prepub == 1 || imp->prepub == 2)
					score1 = score2-1;
		}
	}


	return score1-score2;

}

Int2 GetPubScore (ValNodePtr pub)

{
/* Below are the scores assigned to each pub following (roughly) 
Sirotkin's FlatRefBest.  Here 0 (not set) is given a score of 0, 1
(gen) a score of 6, 2 (sub) a score of 11 etc. The higher the score
the better the pub. */
	static Int2 scores[12] = 
	{0, 6, 11, 7, 1, 8, 4, 3, 5, 9, 2, 10};

	if (pub == NULL)
		return 0;

	return scores[pub->choice];

}

/**********************************************************************
*void GetPubDescr (BioseqContextPtr bcp, BioseqPtr bsp, ValNodePtr PNTR vnpp)
*
*	"GetPubDescr" gets the pubs from the BioseqPtr's.
*
************************************************************************/

void GetPubDescr (BioseqContextPtr bcp, BioseqPtr bsp, ValNodePtr PNTR vnpp)

{
	PubdescPtr pdp;
	ValNodePtr descr=NULL, vnp = *vnpp;

	while ((descr=BioseqContextGetSeqDescr(bcp, Seq_descr_pub, descr, NULL)) != NULL) 
	{
		pdp = descr->data.ptrvalue;
		vnp = StorePubDescr(bsp, vnp, pdp, descr);
	}
	*vnpp = vnp;
}

/**************************************************************************
* void GetPubFeat (BioseqContextPtr bcp, ValNodePtr PNTR vnpp)
*
*	Get the Pub's that are on features (as a feature and as "cit
*	on a feature).  
*
****************************************************************************/

void GetPubFeat (BioseqContextPtr bcp, ValNodePtr PNTR vnpp)
{
	SeqFeatPtr sfp=NULL;
	ValNodePtr vnp = *vnpp;

	sfp = BioseqContextGetSeqFeat(bcp, 0, sfp, NULL, 2);

	while (sfp != NULL)
	{
		vnp = LookForPubsOnFeat(sfp, vnp);
		if (sfp->data.choice == 6)
			vnp = StorePubCit(vnp, sfp);
		
		sfp = BioseqContextGetSeqFeat(bcp, 0, sfp, NULL, 2);
	}
	*vnpp = vnp;
}


/**************************************************************************
* void GetNAPubFeat (BiotablePtr btp_aa, Int2 count, ValNodePtr PNTR vnpp)
*
*	Get the Pub's that are on Nucleic Acid features and cover all or
*	part of the protein entry.
*
*	First sfp->cit's and sfp's that are pubs (i.e., sfp->data.choice == 6)
*	are used.  If none are applicable, then sfp's that are ImpFeats
*	of type "sites" are used.
*
****************************************************************************/

void GetNAPubFeat (BiotablePtr btp_aa, Int2 count, ValNodePtr PNTR vnpp, SortStructPtr PNTR List, Int2Ptr currentsites)
{
	BioseqContextPtr bcp=btp_aa->bcp_na[count];
	BioseqPtr bsp=btp_aa->bsp[count];
	ImpFeatPtr ifp;
	Int2 status, totalsites=0;
	SeqFeatPtr sfp=NULL, prot_sfp=btp_aa->cds[count];
	SortStructPtr Siteslist = *List;
	ValNodePtr vnp = *vnpp;

	sfp = BioseqContextGetSeqFeat(bcp, 0, sfp, NULL, 2);

	while (sfp != NULL)
	{
		if (sfp->data.choice == 6)
		{
			status = SeqLocCompare(prot_sfp->location, sfp->location);
			if (status != 0)
				vnp = StoreNAPubCit(vnp, bsp, sfp);
		}
		else if (sfp->cit != NULL)
		{
			if (sfp->data.choice == 8)
				ifp = (ImpFeatPtr) sfp->data.value.ptrvalue;
			if (sfp->data.choice == 8 && 
				StringCmp(ifp->key, "Site-ref") == 0)
			{
				if (*currentsites == totalsites)
				{
			   	if (totalsites == 0)
		    	   	Siteslist = 
				   EnlargeSortList(NULL, &(totalsites), 3);
			  	 else
		    	  	 Siteslist = 
				   EnlargeSortList(Siteslist, &(totalsites), 3);
				}
				Siteslist[*currentsites].sfp = sfp;
				(*currentsites)++;	
			}
			else
			{
				status = SeqLocCompare(prot_sfp->location, sfp->location);
				if (status != 0)
					vnp = StorePubFeat(vnp, sfp);
			}
		}
		sfp = BioseqContextGetSeqFeat(bcp, 0, sfp, NULL, 2);
	}
	*List=Siteslist;
	*vnpp = vnp;
}	/* GetNAPubFeat */


ValNodePtr LookForPubsOnFeat(SeqFeatPtr sfp, ValNodePtr PubOnFeat)

{

	if (sfp->cit != NULL)
		PubOnFeat = StorePubFeat(PubOnFeat, sfp);

	return PubOnFeat;

}


/**************************************************************************
*
*	"OrganizePubs" to unique and sort the pubs.  First a check is
*	done for serial numbers and, if they exist, the pubs are output
*	in order without further work.  If not, they are uniqued and
*	sorted.  The return value is the number of pubs (>= 0).
*
**************************************************************************/

Int2 OrganizePubs (BiotablePtr btp, Int2 count)

{

	Int2 index;
	PubStructPtr psp;
	ValNodePtr vnp, vnp1;
	Boolean numbered=TRUE, ordered=TRUE;

	if (btp->Pub[count] == NULL)
		return 0;
	else
		vnp=btp->Pub[count];

	for (vnp1=vnp; vnp1; vnp1=vnp1->next)
	{
		psp = vnp1->data.ptrvalue;
		if (psp->number == -1)
		{
			numbered = FALSE;
			break;
		}
	}
	
	if (numbered == TRUE)
	{
		for (vnp1=vnp, index=0; vnp1; vnp1=vnp1->next)
		{
			index++;
			psp = vnp1->data.ptrvalue;
			if (psp->number != index)
			{
				ordered = FALSE;
				break;
			}
		}

		if (ordered != TRUE)
		{
			VnpHeapSort(&(vnp), CompareSnForHeap);
		}
	}
	else
	{
	/*	VnpHeapSort(&(vnp), CompareStartForHeap); */
		PubHeapSort(&(vnp), CompareByDate); 
	}

/* New 	serial numbers are assigned for both non-ordered and non-numbered,
as an numbered set of pubs could all have redundant serial numbers. 
Could this cause problems if serial numbers are referenced in the feature
table?????????*/
	if (!ordered || !numbered)
	{
		for (vnp1=vnp, index=0; vnp1; vnp1=vnp1->next)
		{
			index++;
			psp = vnp1->data.ptrvalue;
			psp->number = index;
		}
		btp->Pub[count] = vnp;
	}

/* How many pubs are there? */
	for (vnp1=btp->Pub[count], index=0; vnp1; vnp1=vnp1->next)
		index++;

	return index;
}

/************************************************************************
*
*	"Unique Pubs" to unique the pubs in the PubStructPtr PNTR
*	and then concatenate the information (seqfeats etc.) in 
*	two elements that share a pub.
*
***********************************************************************/

void UniquePubs (ValNodePtr PNTR vnpp)
{
	Int2 status, status1;
	PubStructPtr psp1, psp2;
	ValNodePtr last1, last2, vnp1, vnp2;

	vnp1 = *vnpp;
	last1 = NULL;
	while (vnp1)
	{
		for (vnp2=vnp1->next, last2=vnp1; vnp2; vnp2=vnp2->next)
		{
			psp1 = vnp1->data.ptrvalue;
			psp2 = vnp2->data.ptrvalue;
			status = PubMatch(psp1->pub, psp2->pub);
			if (! ABS(status))
			{
				status1 = SelectBestPub(psp1->pub, psp2->pub);
				if (status1 < 0)
				{ 
				    CatPspInfo(psp1, psp2);
				    if (last1 == NULL)
				    {
					*vnpp = vnp1->next;
					vnp1->next = NULL;
					FreePubStruct(vnp1->data.ptrvalue);
					ValNodeFree(vnp1);
					vnp1 = *vnpp;
				    }
				    else
				    {
					last1->next = vnp1->next;
					vnp1->next = NULL;
					FreePubStruct(vnp1->data.ptrvalue);
					ValNodeFree(vnp1);
					vnp1 = last1->next;
				    }
				    break;
				}
				else
				{
				    CatPspInfo(psp2, psp1);
				    last2->next = vnp2->next;
				    vnp2->next = NULL;
				    FreePubStruct(vnp2->data.ptrvalue);
				    ValNodeFree(vnp2);
				    vnp2 = last2;
				}
			}
			else
				last2 = vnp2;
		}
		last1 = vnp1;
		if (vnp1 != *vnpp || vnp2 == NULL)
			vnp1 = vnp1->next;
	}

}	/* UniquePubs */

/***************************************************************************
*void CatPspInfo (PubStructPtr psp1, PubStructPtr psp2)
*
*	Concatenate some of the information from two different PubStructs
*	that refer to the same publication.
**************************************************************************/

void CatPspInfo (PubStructPtr psp1, PubStructPtr psp2)

{
  psp2->citfeat = 
    CatSfpStacks(psp2->citfeat, &(psp2->citcount), psp1->citfeat, psp1->citcount); 
  psp2->pubfeat = 
    CatSfpStacks(psp2->pubfeat, &(psp2->pubcount), psp1->pubfeat, psp1->pubcount); 
  psp2->bsp = 
    CatBspStacks(psp2->bsp, &(psp2->bspcount), psp1->bsp, psp1->bspcount); 
  if (psp2->start > psp1->start)
	psp2->start = psp1->start;
   if (psp1->descr && psp2->descr)
   {
	if (psp2->descr->fig == NULL && psp1->descr->fig)
		psp2->descr->fig = psp1->descr->fig;
	if (psp2->descr->poly_a == FALSE && psp1->descr->poly_a)
		psp2->descr->poly_a = psp1->descr->poly_a;
	if (psp2->descr->comment == NULL && psp1->descr->comment)
		psp2->descr->comment = psp1->descr->comment;
	if (psp2->descr->maploc == NULL && psp1->descr->maploc)
		psp2->descr->maploc = psp1->descr->maploc;
   }
   return;
}

/*****************************************************************************
*FlatRefBest
*
*	taken (with minor modifications) from Karl Sirotkin's code 
*	by Tom Madden.
*
*****************************************************************************/

/*--------FlatRefBest()----*/

/*
	returns ValNodePtr to best (for FlatFile production)
	pub in a equiv set.
*/
ValNodePtr FlatRefBest 
(ValNodePtr equiv, Boolean error_msgs, Boolean anything)
{
	ValNodePtr the_pub, retval = NULL, newpub;
	CitBookPtr cb;
	CitSubPtr cs;
	CitGenPtr cg;	
	CitArtPtr ca;
	MedlineEntryPtr ml;
	CitJourPtr jp;
	ImprintPtr ip;
	Boolean good_one;
	Int1 bad_one= 0;
	CharPtr str_ret;

	if (equiv->choice == PUB_Equiv)
		newpub = equiv->data.ptrvalue;
	else
		newpub = equiv;

	for (the_pub = newpub, good_one = FALSE; the_pub && ! good_one
			; the_pub = the_pub -> next){

		switch ( the_pub -> choice){

      case 2:
         cs = (CitSubPtr) the_pub -> data.ptrvalue;
			if ( cs -> imp){
				ip = cs -> imp;
				if ( ip -> date){
					retval = the_pub;
					good_one = TRUE; /* good for submitted */
				}
			}
         break;
	/*  thesis */
		case 11:
			cb = (CitBookPtr) the_pub -> data.ptrvalue;
			if ( cb -> imp){
				ip = cb -> imp;
				if ( ip -> date){
					retval = the_pub;
					good_one = TRUE; /* good for thesis */
				}
			}
		break;
		case 9:
			retval = the_pub;
			good_one = TRUE; /* might exclude later...*/
		break;

/*  CitArts  or CitBook */
		case 5:
		case 3:
			if ( the_pub -> choice == 3){
				ml = (MedlineEntryPtr) the_pub -> data.ptrvalue;
				ca = (CitArtPtr) ml -> cit;

			}else{
				ca = (CitArtPtr) the_pub -> data.ptrvalue;
			}
		if( ca -> fromptr){
			if ( ca -> from ==1){
				jp = (CitJourPtr) ca -> fromptr;
				if ( jp -> imp){
					ip = jp -> imp;
					if ( ip -> date){
						retval = the_pub;
						good_one = TRUE; /* good as it gets */
					}
				}
			}else {
				CitBookPtr book = (CitBookPtr) ca -> fromptr;
					if ( book -> imp){
						ip = book -> imp;
						if ( ip -> date){
							retval = the_pub;
							good_one = TRUE; /* good for book */
						}
					}
				
			}
		}
			break;
		case 1: 
			cg = (CitGenPtr) the_pub -> data.ptrvalue;
			if (cg -> cit){

					str_ret = NULL;
					str_ret = StrStr(cg -> cit ,"Journal=\"");
					if ((str_ret) || (cg->title) || (cg->journal) || (cg->date))
					{
						retval = the_pub;  /*unless something better */
					} else {
						if (StringNICmp("unpublished", cg->cit, 11) == 0)
							retval = the_pub;
						else if (StringNICmp("to be published", cg->cit, 15) == 0)
							retval = the_pub;
						else if (StringNICmp("in press", cg->cit, 8) == 0)
							retval = the_pub;
						else if (StringNICmp("submitted", cg->cit, 8) == 0)
							retval = the_pub;
					}
				}else if (cg -> journal){
						retval = the_pub;  /*unless something better */
				}

			break;
		case 8:  /* proc */
			bad_one = the_pub -> choice;
			break;
		}
	}

	if (! retval && anything)
	{
	   for (the_pub = newpub; the_pub; the_pub = the_pub -> next)
	   {		/* Look for an muid */
		if (the_pub->choice == 4)
			retval = the_pub;
	   }
	   if (! retval) /* Take anything left over now and hope for the best */
		retval = newpub;
	}

	if ( ! retval && bad_one != 0){
		if (error_msgs == TRUE)
			ErrPostEx(SEV_WARNING, ERR_REFERENCE_Illegalreference,
			"FlatRefBest: Unimplemented pub type = %d", bad_one);
	}
	
	return retval;
}	/* FlatRefBest */


void CpListToBtp(SortStruct PNTR List, SeqFeatPtr PNTR PNTR sfpList, Int2 currentsize, Int2Ptr sfpListsize, Int2 count)

{
	Int2 ilist;
	sfpList[count] = (SeqFeatPtr PNTR) MemNew((currentsize+1) * sizeof(SeqFeatPtr));

	if (currentsize == 0)
		return;

	for (ilist=0; ilist<currentsize; ilist++)
		sfpList[count][ilist] = List[ilist].sfp;

	sfpListsize[count] = currentsize;
}

Int2 CpSfpToList(SortStruct PNTR List, SeqFeatPtr sfp, BiotablePtr btp, Int2 currentsize, Int2 count)

{
	List[currentsize].sfp = sfp;
	List[currentsize].bsp = btp->bsp[count];
	List[currentsize].table = btp->table[count];

	currentsize++;

	return currentsize;
}
/****************************************************************************
*CharPtr Cat2Strings (CharPtr string1, CharPtr string2, CharPtr separator, Int2 num)
*
* Concatenates two strings (string1 and string2) and separates them by a
* "separator".  If num>0, takes num spaces off the end of string1 on
* concatenation; if num<0 takes all spaces off the end of the complete 
* string.
*****************************************************************************/
CharPtr Cat2Strings (CharPtr string1, CharPtr string2, CharPtr separator, Int2 num)

{
	Boolean no_space=FALSE;
	Int2 length1=0, length2=0, length_sep=0, length_total;
	CharPtr newstring=NULL;

	if (num < 0)
	{
		num=0;
		no_space=TRUE;
	}

	if (string1 != NULL)
		length1 = StringLen(string1);
	if (string2 != NULL)
		length2 = StringLen(string2);
	if (separator != NULL)
		length_sep = StringLen(separator);

	length_total = length1+length2+length_sep-num+1;

	newstring = (CharPtr) MemNew(length_total*sizeof(Char));

	if (string1 != NULL)	
		newstring = StringCat(newstring, string1);
	if ((length1-num) >= 0)
		newstring[length1-num] = '\0';
	if (no_space && length1 > 0)
		while (length1 > 0 && newstring[length1-1] == ' ')
		{
			newstring[length1-1] = '\0';
			length1--;
		}
	if (separator != NULL)	
		newstring = StringCat(newstring, separator);
	if (string2 != NULL)	
		newstring = StringCat(newstring, string2);


	return newstring;
}

/*****************************************************************************
*Boolean DoSpecialLineBreak (BiotablePtr btp, CharPtr string, Int2 indent)
*
*	Look for line breaks in special cases, such as KEYWORDS, when
*	line breaks are only allowed after semi-colons.
*
*	"length" is reset to zero if string is NULL, "indent" tells how
*	many spaces the line is indented from the left side.
*****************************************************************************/
 
Boolean DoSpecialLineBreak (BiotablePtr btp, CharPtr string, Int2 indent)

{
	Boolean retval;
	static Int2 length=0;

	if (string == NULL)
	{
		length = 0;
		retval = FALSE;
	}
	else
	{
		length += StringLen(string);
		length += 2;
		if (length < (ASN2FF_GB_MAX-indent))
		{
			retval = FALSE;
		}
		else
		{
			length = StringLen(string);
			length += 2;
			retval = TRUE;
		}
	}
	return retval;

}	/* DoSpecialLineBreaks */


/******************************************************************************
*
*	print utilities to handle the printing of the asn2ff system.
*	
*	by Tom Madden
*
*****************************************************************************/

Int2 StartPrint (BiotablePtr btp, Int2 init_indent, Int2 cont_indent, Int2 line_max, CharPtr line_prefix)

{

	CharPtr buffer; 
	Int2 indent_space;

	buffer = btp->buffer;
	if (buffer == NULL)
		return -1;

	btp->init_indent = init_indent;
	btp->cont_indent = cont_indent;

	if (line_max > 0)
		btp->line_max = line_max;
	else
		btp->line_max = MAX_BTP_BUF;

	if (line_prefix)
	{
		btp->line_prefix = StringCpy(btp->line_prefix, line_prefix);
		*buffer = *line_prefix;
		buffer++;
		line_prefix++;
		*buffer = *line_prefix;
		buffer++;
		line_prefix++;
		indent_space = init_indent - StringLen(btp->line_prefix);
	}
	else
	{
		btp->line_prefix[0] = '\0'; 
		indent_space = init_indent;
	}

	if (indent_space > 0)
		MemSet((VoidPtr) buffer, ' ', indent_space);

	return init_indent;
}	/* StartPrint */
		
Int2 AddString (BiotablePtr btp, CharPtr string)

{
	Char newline = btp->newline;
	CharPtr buffer;
	Int2 increment_string=0, index, length_b, length_s, line_max=btp->line_max;

	if (string == NULL)
		return -1;

	buffer = CheckBufferState(btp, &increment_string, string[0]);

	if (buffer == NULL)
		return -1;

	length_b = StringLen(btp->buffer);

	string += increment_string;
	index = length_b;
	while ((*buffer = *string) != '\0')
	{	
		if (*string == newline)
		{
			*buffer = '\0';
			*string = ' ';
			NewContLine(btp);
			string++;
			AddString(btp, string);
			break;
		}
		else
		{
			buffer++;
			string++;
			index++;
			if (index == line_max)
			{ /* return value for AddString??????? */
				AddString(btp, string);
				break;
			}
		}
	}

	length_s = index - length_b;

	return length_s;
}	/* AddString */	

Int2 AddChar (BiotablePtr btp, Char character)

{
	CharPtr buffer;
	Int2 increment_string=0;

	if (character == NULLB)
		return -1;

	if (character == btp->newline)
	{
		NewContLine(btp);
	}
	else
	{
		buffer = CheckBufferState(btp, &increment_string, character);
		*buffer = character;
	}

	return 1;
}	/* AddChar */

Int2 AddInteger (BiotablePtr btp, CharPtr fmt, long integer)

{
	Char buffer[10];

	/* Only one integer read in	*/
	sprintf(buffer, fmt, integer);
	AddString(btp, buffer);
	return 1;
}	/* AddInteger */
		
		
CharPtr CheckBufferState(BiotablePtr btp, Int2Ptr increment_string, Char next_char)

{
	CharPtr buffer, line_prefix;
	CharPtr buf_ptr_comma=NULL, buf_ptr_dash=NULL, buf_ptr_space=NULL, 
	buf_ptr_start, ptr_index;
	CharPtr temp_ptr, temp_ptr_start;
	Char temp[MAX_TO_RT_SIDE+1];
	Int2 length, cont_indent=btp->cont_indent, indent_space; 
	Int2 line_max = btp->line_max, line_index;

	*increment_string = 0;
	temp_ptr = temp_ptr_start = temp;

	buffer = btp->buffer;
	length=0;
	while (*(buffer+length) != '\0')
		length++;

	if (length < btp->line_max)
	{
		return buffer+length;
	}
	else if (length == btp->line_max && next_char == '\0')
	{
		return buffer+length;
	}
	else
	{
		if (StringLen(btp->line_prefix) > 0)
		{
			line_prefix = btp->line_prefix;
			indent_space = cont_indent - StringLen(line_prefix);
		}
		else
		{
			line_prefix = NULL;
			indent_space = cont_indent;
		}

		if (next_char == ' ')	
		{
			buf_ptr_space = buffer+line_max+1;
		}
		else
		{
			for (ptr_index=buffer+line_max, line_index=line_max-cont_indent; 
				ptr_index>buffer+line_max-MAX_TO_RT_SIDE &&
					line_index > 0;
						ptr_index--, line_index--)
			{
				if (ptr_index[0] == ' ')
				{
					buf_ptr_space = ptr_index;
					break;
				}
			}
		
			for (ptr_index=buffer+line_max, line_index=line_max-cont_indent; 
				ptr_index>buffer+line_max-MAX_TO_RT_SIDE &&
					line_index > 0;
						ptr_index--, line_index--)
			{
				if (ptr_index[0] == ',')
				{
					buf_ptr_comma = ptr_index;
/* Add one on the next line, otherwise the comma is lost, as space is. */
					buf_ptr_comma++;
					break;
				}
			}
			for (ptr_index=buffer+line_max, line_index=line_max-cont_indent; 
				ptr_index>buffer+line_max-MAX_TO_RT_SIDE &&
					line_index > 0;
						ptr_index--, line_index--)
			{
				if (ptr_index[0] == '-')
				{  /* Don't put "-" on next line! */
					buf_ptr_dash = ptr_index + 1;
					break;
				}
			}
		}

		if (next_char != ' ' &&
			(buf_ptr_space || buf_ptr_comma || buf_ptr_dash))
		{
			if (buf_ptr_space)
			{
				buf_ptr_start = buf_ptr_space;
				buf_ptr_space++;
				while((*temp_ptr = *buf_ptr_space) != '\0')
				{
					temp_ptr++;
					buf_ptr_space++;
				}
				buf_ptr_start[0] = '\0';
				if (*(buf_ptr_start-1) == ' ')
				{ /* If there are two spaces in a row */
					buf_ptr_start--;
					buf_ptr_start[0] = '\0';
				}
			}
			else if (buf_ptr_comma)
			{
				buf_ptr_start = buf_ptr_comma;
/* Check if a space follows the comma; if so, delete it; otherwise it shows
up on the next line and really looks stupid.			*/
				if (*buf_ptr_comma == ' ')
					buf_ptr_comma++;
				while((*temp_ptr = *buf_ptr_comma) != '\0')
				{
					temp_ptr++;
					buf_ptr_comma++;
				}
				buf_ptr_start[0] = '\0';
			}
			else if (buf_ptr_dash)
			{
				buf_ptr_start = buf_ptr_dash;
				while((*temp_ptr = *buf_ptr_dash) != '\0')
				{
					temp_ptr++;
					buf_ptr_dash++;
				}
				buf_ptr_start[0] = '\0';
			}

			FlushBuffer(btp);
			if (line_prefix != NULL)
			{
				*buffer = *line_prefix;
				buffer++;
				line_prefix++;
				*buffer = *line_prefix;
				buffer++;
				line_prefix++;
			}
			if (indent_space > 0)
				MemSet((VoidPtr) buffer, ' ', indent_space);
			buffer += indent_space;
			temp_ptr = temp_ptr_start;
			while((*buffer = *temp_ptr) != '\0')
			{
				temp_ptr++;
				buffer++;
			}
		
			return buffer;
		}
		else if (next_char == ' ')
		{
			FlushBuffer(btp);
			if (line_prefix)
			{
				*buffer = *line_prefix;
				buffer++;
				line_prefix++;
				*buffer = *line_prefix;
				buffer++;
				line_prefix++;
			}
			if (indent_space-1 > 0)
			{ 
				MemSet((VoidPtr) buffer, ' ', indent_space-1);
				return buffer+indent_space-1;
			}
			else if (indent_space-1 == 0)
			{ /* if there is one space indentation! */
				return buffer;
			}
			else if (indent_space-1 < 0)
			{ /* if there is zero space indentation! */
				*increment_string = 1;
				return buffer;
			}
		}
		else 
		{
			FlushBuffer(btp);
			if (line_prefix)
			{
				*buffer = *line_prefix;
				buffer++;
				line_prefix++;
				*buffer = *line_prefix;
				buffer++;
				line_prefix++;
			}
			if (indent_space > 0)
				MemSet((VoidPtr) buffer, ' ', indent_space);
			return buffer+indent_space;
		}
	}
	return buffer;	/* never used!  Only put in to make CodeWarrior happy*/
}	/* CheckBufferState */

Int2 NewContLine (BiotablePtr btp)

{
	Int2 cont_indent=btp->cont_indent, indent_space;
	CharPtr buffer, line_prefix=NULL;

	FlushBuffer(btp);
	buffer=btp->buffer;

	if (StringLen(btp->line_prefix) > 0)
	{
		line_prefix = btp->line_prefix;
		indent_space = cont_indent - StringLen(line_prefix);
	}
	else
	{
		indent_space = cont_indent;
	}

	if (line_prefix)
	{
		*buffer = *line_prefix;
		buffer++;
		line_prefix++;
		*buffer = *line_prefix;
		buffer++;
		line_prefix++;
	}

	if (indent_space > 0)
		MemSet((VoidPtr) buffer, ' ', indent_space);
	return cont_indent;
}	/* NewContLine */


Int2 TabToColumn (BiotablePtr btp, Int2 column)

{
	CharPtr start_buffer, buffer;
	Int2 increment_string=0, length;
	start_buffer = btp->buffer;

	if ((length=StringLen(start_buffer)) > column)
		return -1;

	buffer = CheckBufferState(btp, &increment_string, '\0');

	if ((column-length-1) > 0)
		MemSet((VoidPtr) buffer, ' ', (column-length-1));
		
	return length;
}

void EndPrint (BiotablePtr btp)

{
	FlushBuffer(btp);
}

void FlushBuffer (BiotablePtr btp)

{
	ByteStorePtr byte_sp=NULL;
	CharPtr line_ret=btp->line_return;

	if (btp->fp)
	{
		fflush(btp->fp);
		fprintf(btp->fp, "%s\n", btp->buffer);
	}
	else
	{
		byte_sp=btp->byte_sp;
		if (! byte_sp)
			btp->byte_sp = byte_sp = BSNew(MAX_BTP_BUF);
		BSWrite(byte_sp, btp->buffer, StringLen(btp->buffer));
		BSWrite(byte_sp, line_ret, StringLen(line_ret));
	}
	/* Do I need to do the next line ??????????????*/
	MemSet((VoidPtr) btp->buffer, '\0', btp->line_max);
}


/**************************************************************************
*CheckEndPunctuation
*
*	This code checks to ensure that the last character of a string
*	is the character passed.  Any extra spaces or tabs at the end of
*	the string are eliminated.
*
**************************************************************************/

CharPtr CheckEndPunctuation (CharPtr string, Char end)

{
	CharPtr stringptr, newstring;
	Int2 length;

	if (string == NULL)
		return NULL;

	length = StringLen(string);
	newstring = (CharPtr) MemNew((length+2)*sizeof(Char));

	if (length > 0)
	{
		newstring = StringCpy(newstring, string);
		for (stringptr=newstring+length-1; stringptr > newstring; stringptr--)
		{
			if (*stringptr == ' ' || *stringptr == '\t' || *stringptr == '~')
			{
				*stringptr = '\0';
			}
			else
			{
				break;
			}
		}
	
		if (*stringptr != end)
		{
			stringptr++;
			*stringptr = end;
			stringptr++;
			*stringptr = '\0';
		}	
	} 
	else
	{
		newstring[0] = end;
		newstring[1] = '\0';
	}

	return newstring;
}
	
/**************************************************************************
*BioseqContextPtr FindBioseqContextPtr (BiotablePtr btp, BioseqPtr bsp, Int2 *index)
*	
*	btp: BiotablePtr, which has the address of a BioContextPtr (bcp) one 
*		wishes to know.
*	bsp: BioseqPtr corresponding to a given BioContextPtr.
*	*index: entry numbers of the sequences in the btp.
*
*	For a given bsp one may know that the bcp (found earlier in the 
*	program) is listed on a btp, but not the index of the bcp.
*	This program compares the input bsp to all bsp's on the btp
*	and finds the bcp.  The bcp is returned and *index is changed
*	to the relevant value.
***************************************************************************/

BioseqContextPtr FindBioseqContextPtr (BiotablePtr btp, BioseqPtr bsp, Int2 *index)

{
	BioseqContextPtr bcp=NULL;
	Int2 range=btp->count;

	range = btp->count;

	for (*index=0; *index<range; (*index)++)
		if (bsp == btp->bsp[*index])
		{
			bcp = btp->bcp[*index];
			break;
		}

	return bcp;
}


/************************************************************************
*AddGBQual
*
*	This function makes a new GBQual and adds a "val" and a
*	a "qual".
*   doesn't add qual if it's already there /tatiana/
***********************************************************************/

GBQualPtr AddGBQual (GBQualPtr gbqual, CharPtr qual, CharPtr val)

{
	GBQualPtr curq;

	if (gbqual)
	{
		for (curq=gbqual; curq->next != NULL; curq=curq->next) {
			if (StringCmp(curq->qual, qual) == 0 && 
						StringCmp(curq->val, val) == 0)
				return gbqual;
		}

		curq->next = GBQualNew();
		curq = curq->next;
		if (val)
			curq->val = StringSave(val);
		curq->qual = StringSave(qual);
	}
	else
	{
		gbqual = GBQualNew();
		gbqual->next = NULL;
		if (val)
			gbqual->val = StringSave(val);
		gbqual->qual = StringSave(qual);
	}

	return gbqual;
}


/****************************************************************************
*
*	MakeAnAccession is for last ditch efforts to get an accession
*	after all the normal things have failed.
*
****************************************************************************/

CharPtr MakeAnAccession (CharPtr new_buf, BioseqPtr bsp)

{
	SeqIdPtr seq_id, new_id;

	seq_id = bsp->id;
	new_id = SeqIdFindBest(seq_id, SEQID_GENBANK);
	SeqIdPrint(new_id, new_buf, PRINTID_FASTA_SHORT);
	return new_buf;

}

/****************************************************************************
*GetGBSourceLine
*
*	Gets the source from the GBBlock.
*
****************************************************************************/

CharPtr GetGBSourceLine (GBBlockPtr gb)

{
	CharPtr source = NULL;

	if(gb && gb->source)
		source = StringSave(gb->source);

	return source;
}

/*  from ASN.1 defs:
    plasmid (3) ,
    mitochondrial (4) ,
    chloroplast (5) ,
    kinetoplast (6) ,
    cyanelle (7) ,
    synthetic (8) ,
    recombinant (9) ,
    partial (10) ,
    transposon (14) ,
    insertion-seq (15) ,
   macronuclear (18) ,
   proviral (19) ,
---------------------------*/

/*--------FlatOrganelle()---------*/

CharPtr
FlatOrganelle(BiotablePtr btp, Int2 count)
{
	CharPtr retval = NULL;
	ValNodePtr man, vnp=NULL;
	static char * organelle_names [] = {
		 "Mitochondrion " ,
    "Chloroplast " ,
    "Kinetoplast ",
    "Cyanelle "};
	BioSourcePtr biosp=NULL;
	
	static CharPtr genome[] = {
	NULL, NULL, "Cloroplast ", "Chromoplast ", "Kinetoplast ", "Mitochondrion ", "Plastid", "Macronuclear", "Extrachrom", "Plasmid", NULL, NULL, "Cyanelle ", "Proviral"};
	
/* try new first */
	if ((vnp=BioseqContextGetSeqDescr(btp->bcp[count], Seq_descr_source, vnp, NULL)) != NULL) 
	{
		biosp = vnp->data.ptrvalue;
		if (biosp->genome < 6 || biosp->genome == 12)
			retval = StringSave(genome[biosp->genome]);
	}
/* old next */
	if (biosp == NULL) {
		if ((vnp=BioseqContextGetSeqDescr(btp->bcp[count], Seq_descr_modif, vnp, NULL)) != NULL) 
		{
			for (man = (ValNodePtr) vnp-> data.ptrvalue; man; man = man -> next)
			{
				switch (man -> data.intvalue){
					case 4: case 5: case 6: case 7:
					if (! retval )
						retval = StringSave(organelle_names
								[man->data.intvalue-4]);
						break;
					default:
						break;
					}
			}
		}
	}
	return retval;
}

/*************************************************************************
*	Look for the same sfp in two different places.  Return 1
*	if a match is found; return 0 if none found.
*************************************************************************/

Int2 RemoveRedundantFeats(BiotablePtr btp, Int2 count, SeqFeatPtr sfp)

{
	Int2 index, retval=0, int_count;

	for (int_count=count+1; int_count < (btp->count); int_count++)
	{
		index=0;
		while (btp->sfpList[int_count][index])
		{
			if (btp->sfpList[int_count][index] == sfp)
				return 1;
			index++;
		}
	}
	return retval;
}

Int4 GetNumOfSeqBlks (BiotablePtr btp, Int2 count)

{
        Int4 length, num_of_seqblks;

        length = BioseqGetLen(btp->bsp[count]);
        num_of_seqblks = ROUNDUP(length, SEQ_BLK_SIZE)/SEQ_BLK_SIZE;

        return num_of_seqblks;
}

