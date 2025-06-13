/******************************************************************************
*
*
* $Log: makerpt1.c,v $
 * Revision 1.9  1995/05/15  21:46:05  ostell
 * added Log line
 *
*
*
******************************************************************************/
#include "makerpt.h"
#include "asn2ff6.h"

#define NUM_ORDER 16

/* The following corresponds to NUM_SEQ_LINES lines each with 60 
residues/basepairs */
#define SEQ_BLK_SIZE (60*NUM_SEQ_LINES)

/* The following defines are used by ReportLoadPap to identify the pointer type. */
#define REPORT_OTHER ( (Uint1)0)
#define REPORT_SOURCE_FEATURE ( (Uint1)1)
#define REPORT_FEATURE ( (Uint1)2)
#define REPORT_REFERENCE ( (Uint1)3)
#define REPORT_SET_DESCRIPTOR ( (Uint1)4)
#define REPORT_TABLE_DESCRIPTOR ( (Uint1)5)
#define REPORT_BIOSEQ_DESCRIPTOR ( (Uint1)6)
#define REPORT_SEQUENCE ( (Uint1)7)

Boolean makerpt_flags[10];

#define MAKERPT_DO_EDIT_MODE            makerpt_flags[0]
#define MAKERPT_LOOK_FOR_SEQ            makerpt_flags[1]

/* ---------------Function Prototypes ---------------*/
Int4 report_setup PROTO ((BiotablePtr btp_na, FFPrintArrayPtr PNTR papp));
static void ReportSeqToArrays PROTO ((SegmentPtr segp, BiotablePtr btp_na, BiotablePtr btp_aa));
void ReportGetSegNum PROTO ((SeqEntryPtr sep, Pointer data, Int4 index, Int2 indent));
Int2 ReportGetNumAA PROTO ((SegmentPtr segp, Uint1 format, Boolean error_msgs));
Int2 ReportGetNumNA PROTO ((SegmentPtr segp, Uint1 format, Boolean error_msgs));
static void ReportGetRefs PROTO ((BiotablePtr btp));
static void ReportFindRawSeq PROTO ((BioseqPtr bsp, BiotablePtr btp_na, BiotablePtr btp_aa));
static Boolean ReportCompareToSegpList PROTO ((BioseqPtr bsp, SegmentPtr segp));
void MovePubsToFirstSeg PROTO ((BiotablePtr btp_na, BiotablePtr btp_aa));
void DoPubCheck PROTO ((BiotablePtr btp_na, BiotablePtr btp_aa));
void ReportLoadPap PROTO ((FFPrintArrayPtr pap, FFPapFct fct, BiotablePtr btp, Int2 count, Int4 index, Uint1 last, Uint1 printxx, Int2 estimate, Uint1 element_type));
void ReportGetPapRefPtr PROTO ((BiotablePtr btp, Int2 count, Int4 ext_index, Int4 pap_index, FFPrintArrayPtr pap));
void ReportPrintPubsByNumber PROTO ((BiotablePtr btp, Int2 count));
void set_report_flags PROTO ((Uint1 format, Uint1 mode));
void ReportPrintSeqId PROTO ((BiotablePtr btp, Int2 count));
CharPtr TemplatePrintPubs PROTO ((BiotablePtr btp, Int2 count, PubStructPtr psp));
CharPtr TemplatePrintPubBioseqLoc PROTO ((BiotablePtr btp, Int2 count, Int2Ptr index, PubStructPtr psp, Boolean PNTR first_loc));
CharPtr TemplatePrintPubSeqFeatLoc PROTO ((BiotablePtr btp, Int2 count, Int2Ptr index, PubStructPtr psp, Boolean PNTR first_loc));
CharPtr TemplatePrintPubSeqFeatCitLoc PROTO ((BiotablePtr btp, Int2 count, Int2Ptr index, PubStructPtr psp, Boolean PNTR first_loc));
void ReportOrganizeSeqFeat PROTO ((BiotablePtr btp));
void ReportPrintFeatByNumber PROTO ((BiotablePtr btp, Int2 count));
Int2 ReportGetNumOfFeats PROTO ((BiotablePtr btp, Int2 count));
void ReportCheckSeqPort PROTO ((BiotablePtr btp, Int2 count, Int4 start));
void ReportPrintSequence PROTO ((BiotablePtr btp, Int2 count, Int4 start, Int4 stop));
void ReportPrintSeqBlk PROTO ((BiotablePtr btp, Int2 count));
void PrintFeatureBanner PROTO ((BiotablePtr btp, Int2 count));
void PrintPubBanner PROTO ((BiotablePtr btp, Int2 count));
void PrintSeqBanner PROTO ((BiotablePtr btp, Int2 count));
void PrintCommentBanner PROTO ((BiotablePtr btp, Int2 count));
void PrintDescrBanner PROTO ((BiotablePtr btp, Int2 count));
void ReportStorePubInfo PROTO ((BioseqContextPtr bcp, BioseqPtr bsp, ValNodePtr PNTR vnpp, Boolean error_msgs));


ValNodePtr GetADescrInBioseq PROTO ((BiotablePtr btp, Int2 count, Uint1 choice, Int2 ext_index));
Int2 CountDescrInBioseq PROTO ((BiotablePtr btp, Int2 count, Uint1 choice));
ValNodePtr GetADescrInTable PROTO ((BiotablePtr btp, Int2 count, Uint1 choice, Int2 ext_index));
Int2 CountDescrInTable PROTO ((BiotablePtr btp, Int2 count, Uint1 choice));
ValNodePtr GetADescrInSet PROTO ((BiotablePtr btp, Uint1 choice, Int2 ext_index));
Int2 CountDescrInSet PROTO ((BiotablePtr btp, Uint1 choice));

void PrintSetDescr PROTO ((BiotablePtr btp, Int2 index));
void PrintTableDescr PROTO ((BiotablePtr btp, Int2 index));
void PrintBioseqDescr PROTO ((BiotablePtr btp, Int2 index));
void CheckRelevanceOfSep PROTO ((SeqEntryPtr sep, Pointer data, Int4 index, Int2 indent));
void OrganizeGBBlockParts PROTO ((BiotablePtr btp, Uint1Ptr move_source, Uint1Ptr move_div, Uint1Ptr move_tax));
void PrintSourceDescr PROTO ((BiotablePtr btp, Int2 count));
void PrintDivDescr PROTO ((BiotablePtr btp, Int2 count));
void PrintTaxDescr PROTO ((BiotablePtr btp, Int2 count));
void LookForRedundantTitles PROTO ((BiotablePtr btp, Int2 count, Int2 descr_set_count, Uint1Ptr print_bioseq_title, Int2 descr_bsp_count, Uint1Ptr print_table_title, Int2 descr_table_count));



/*---------- order for other id FASTA_LONG (copied from SeqIdPrint) ------- */

static Uint1 fasta_order[NUM_ORDER] = {  
33, /* 0 = not set */
20, /* 1 = local Object-id */
15,  /* 2 = gibbsq */
16,  /* 3 = gibbmt */
30, /* 4 = giim Giimport-id */
10, /* 5 = genbank */
10, /* 6 = embl */
10, /* 7 = pir */
10, /* 8 = swissprot */
15,  /* 9 = patent */
20, /* 10 = other TextSeqId */
20, /* 11 = general Dbtag */
255,  /* 12 = gi */
10, /* 13 = ddbj */
10, /* 14 = prf */
12  /* 15 = pdb */
};


/* ---------------Functions--------------------------*/

static void ReportPrintTheString (FILE *fp, CharPtr string, CharPtr line_return)

{
	CharPtr return_ptr = line_return, buffer_ptr;
	Char buffer[100];

	buffer_ptr = &(buffer[0]);

	if (! fp)
		return;

	if (! string || *string == *return_ptr)
	{
		ErrPostEx(SEV_WARNING, 1, 1, 
			"CAUTION: NULL String in ReportPrintTheString\n\n");
	}
	else
	{
		while (*string != '\0')
		{
			*buffer_ptr = *string;
			if (*string == *return_ptr)
			{
				*buffer_ptr='\0';
				buffer_ptr = &(buffer[0]);
				fprintf(fp, "%s\n", buffer_ptr);
				return_ptr = line_return;
				string++;
			}
			else
			{
				string++; buffer_ptr++;
			}
		}
	}
}


/***********************************************************************
*
*	SeqEntryToNewReport is a stand-alone function for writing output
*	to a file; arguments are the standard ones for Entrez.
**************************************************************************/

Boolean SeqEntryToNewReport (SeqEntryPtr sep, Uint1 format, FILE *fp)

{
  BiotablePtr      btp_aa;
  BiotablePtr      btp_na;
  Boolean          rsult;
  Boolean 	   show_seq=TRUE, show_gi=TRUE,error_msgs=FALSE, bind_all=TRUE;
  CharPtr          string;
  FFPrintArrayPtr  pap;
  Int4             index, pap_size;
  StdPrintOptionsPtr Spop = NULL;
  Uint1 mode=REPORT_MODE;

  rsult = FALSE;

  if (sep != NULL && fp != NULL) {
    pap_size = do_initial_setup (sep, mode, format, show_seq, error_msgs, Spop, &pap, &btp_aa, &btp_na);
    if (pap_size > 0) {
      report_set_output (NULL, btp_aa, btp_na, "\n");
      for (index = 0; index < pap_size; index++) {
        string = ReportPrint (pap, index, pap_size);
        if (string != NULL && *string != '\0') {
          ReportPrintTheString (fp, string, "\n");
          string = MemFree (string);
        }
      }
      report_cleanup (btp_aa, btp_na, pap);
      rsult = TRUE;
    }
  }
  return rsult;
}

CharPtr ReportPrint (FFPrintArrayPtr pap, Int4 index, Int4 pap_size)

{
	BiotablePtr btp;
	ByteStorePtr byte_sp=NULL;
	CharPtr string=NULL;
	FFPrintArray pa=pap[index];

	btp = pa.btp;
	btp->pap_index = pa.index;
	btp->pap_last = pa.last;

	if (index == 0 || index == (pap_size-1))
	{
		flat2asn_delete_locus_user_string();
		flat2asn_install_locus_user_string(btp->locus[pa.count]);
		flat2asn_delete_accession_user_string();
		flat2asn_install_accession_user_string(btp->access[pa.count]);
	}

	pa.fct(btp, pa.count);

	if (! btp->fp)
	{
		byte_sp = btp->byte_sp;

		string = BSMerge(byte_sp, NULL);
		btp->byte_sp = BSFree(btp->byte_sp);
	}

	return string;
}

/***************************************************************************
*
*	Setup the FFPrintArrayPtr to be used by "FFPrint", the number 
*	returned is the number of entries in the array.
***************************************************************************/

Int4 do_initial_setup (SeqEntryPtr sep, Uint1 mode, Uint1 format, Boolean show_seq, Boolean error_msgs, StdPrintOptionsPtr Spop, FFPrintArrayPtr PNTR papp, BiotablePtr PNTR btp_aa_p, BiotablePtr PNTR btp_na_p)

{
	BiotablePtr btp_aa, btp_na;
	Int2 bioseq_count, num_of_aa, num_of_na;
	Int4 pap_size = -1;
	SegmentPtr segp;

	ErrSetFatalLevel(SEV_MAX);     /* don't die on me */

	flat2asn_install_accession_user_string("SET-UP");
	flat2asn_install_locus_user_string("SET-UP");

	set_report_flags(format, mode);

	if (MAKERPT_LOOK_FOR_SEQ == FALSE)
	{
	      bioseq_count = (Int2) SeqEntryCount(sep);	
	}
	else
	{
	      segp = SegmentNew((Int2) 1);
	      SeqEntryExplore(sep, (Pointer) segp, CountTheSeqEntrys);
	      bioseq_count = (Int2) segp->count;
	      SegmentFree(segp);
	}

	segp = SegmentNew(bioseq_count);
	segp->sep = sep;

	SeqEntryExplore (sep, (Pointer) segp, ReportGetSegNum);

	num_of_na = ReportGetNumNA(segp, format, error_msgs);
	num_of_aa = ReportGetNumAA(segp, format, error_msgs);

	if (num_of_aa > 0 || num_of_na > 0)
	{

		btp_aa = BiotableNew(num_of_aa);
		btp_na = BiotableNew(num_of_na);

		btp_na->sep = sep;
		btp_aa->sep = sep;

		if (show_seq == TRUE)
		{
			btp_na->show_seq = TRUE;
			btp_aa->show_seq = TRUE;
		}
		else
		{
			btp_na->show_seq = FALSE;
			btp_aa->show_seq = FALSE;
		}

		if (error_msgs == TRUE)
		{
			btp_na->error_msgs = TRUE;
			btp_aa->error_msgs = TRUE;
		}
		else
		{
			btp_na->error_msgs = FALSE;
			btp_aa->error_msgs = FALSE;
		}
 
		btp_na->format = format;   /* will this be used later?????*/
		btp_aa->format = format;
		btp_na->gene_binding = ALL;
		btp_aa->gene_binding = ALL;
		btp_na->pseudo = FALSE;
		btp_aa->pseudo = FALSE;

		btp_aa->Spop = Spop;
		btp_na->Spop = Spop;

		ReportSeqToArrays(segp, btp_na, btp_aa);
                btp_na->btp_other = btp_aa;
                btp_aa->btp_other = btp_na;
                GetBCPs(btp_na, btp_aa);
                GetGIs(btp_na, btp_aa);
                ReportGetRefs(btp_na);
                ReportGetRefs(btp_aa);
		FillSegmentArrays(btp_na, btp_aa);
		SegmentFree(segp);

		MovePubsToFirstSeg (btp_na, btp_aa);
		DoPubCheck(btp_na, btp_aa);

		if (format == REPORT_ON_AA)
		{
		   if (num_of_aa > 0)
			pap_size = report_setup(btp_aa, papp);
		}
		else
		{
		   if (num_of_na > 0)
			pap_size = report_setup(btp_na, papp);
		}
	
		if (pap_size > 0)
		{
			*btp_aa_p = btp_aa;
			*btp_na_p = btp_na;
		}
		else
		{
			BiotableFree(btp_aa);
			BiotableFree(btp_na);
		}
	}
	else
	{
		SegmentFree(segp);
	}
	return pap_size;
}	/* do_initial_setup */

/****************************************************************************
*void set_report_flags (Uint1 format, Uint1 mode)
*
*       set_report_flags to determine which tasks to perform.
*****************************************************************************/
void set_report_flags (Uint1 format, Uint1 mode)
 
{
 
/* The defines are:
        MAKERPT_DO_EDIT_MODE            makerpt_flags[0]
        MAKERPT_LOOK_FOR_SEQ            makerpt_flags[1]
*/

	if (mode == REPORT_MODE)
	{
       		makerpt_flags[0] = FALSE;
       		makerpt_flags[1] = FALSE;
	}
	else 
	{
       		makerpt_flags[0] = TRUE;
       		makerpt_flags[1] = FALSE;
	}
}

/***********************************************************************
*
*	This function sets the appropriate fields in the BiotablePtrs
*	for the output file or the line return.
**************************************************************************/

void report_set_output (FILE *fp, BiotablePtr btp_aa, BiotablePtr btp_na, CharPtr line_return)
{
	if (fp)
	{
		btp_aa->fp = fp;
		btp_na->fp = fp;
	}

	if (line_return)
	{
		btp_aa->line_return = StringSave(line_return);
		btp_na->line_return = StringSave(line_return);
	}
}

/**************************************************************************
*ReportGetNumAA
*	ReportGetNumAA to get the number of amino acid sequence entries.
*
**************************************************************************/

Int2 ReportGetNumAA (SegmentPtr segp, Uint1 format, Boolean error_msgs)

{
	BioseqPtr bsp;
	Int2 index, count=0;
	ValNodePtr vnp;

	for (index=0; index<segp->count; index++)
	{
		vnp=NULL;
		bsp = segp->bsp[index];
		if (ISA_aa(bsp->mol) &&
		   (bsp->repr == Seq_repr_raw || bsp->repr == Seq_repr_const))
		{
			count++;
		}
	}

	return count;
}	/* ReportGetNumAA */

/**************************************************************************
*ReportGetNumNA
*	ReportGetNumNA to get the number of nucleic acid sequence entries.
*
**************************************************************************/

Int2 ReportGetNumNA (SegmentPtr segp, Uint1 format, Boolean error_msgs)

{
	BioseqPtr bsp;
	Int2 index, count=0;
	ValNodePtr vnp;

	for (index=0; index<segp->count; index++)
	{
		vnp=NULL;
		bsp = segp->bsp[index];
		if (ISA_na(bsp->mol) &&
		   (bsp->repr == Seq_repr_raw || bsp->repr == Seq_repr_const))
		{
			count++;
		} 
	}

	return count;
}	/* ReportGetNumNA */

/**************************************************************************
*ReportGetRefs
*	ReportGetRefs to get the ref's for na or aa.
*
**************************************************************************/

static void ReportGetRefs (BiotablePtr btp)

{
	BioseqContextPtr bcp;
	BioseqPtr bsp;
	Int2 index, count=0;
	ValNodePtr vnp;

	for (index=0; index<btp->count; index++)
	{
		vnp=NULL;
		bsp = btp->bsp[index];
		if (bsp->repr == Seq_repr_raw || bsp->repr == Seq_repr_const)
		{
			bcp = btp->bcp[index];
			ReportStorePubInfo(bcp, bsp, &vnp, btp->error_msgs);
			btp->Pub[index] = vnp;
		} 
	}

	return;
}	/* ReportGetRefs */

/**************************************************************************
*
*	ReportSeqToArrays to place the raw BioseqPtr's in arrays.
*
**************************************************************************/

static void ReportSeqToArrays (SegmentPtr segp, BiotablePtr btp_na, BiotablePtr btp_aa)

{
	BioseqPtr bsp;
	SeqEntryPtr sep;
	Int2 index, count_aa=0, count_na=0, last;

	for (index=0; index<segp->count; index++)
	{
		bsp = segp->bsp[index];
		if(bsp->repr == Seq_repr_raw || bsp->repr == Seq_repr_const)
			if (ISA_na(bsp->mol))
			{
				btp_na->bsp[count_na] = bsp;
				count_na++;
			} 
			else if (ISA_aa(bsp->mol))
			{
				btp_aa->bsp[count_aa] = bsp;
				count_aa++;
			}
	}
	btp_na->bsp[count_na] = NULL;
	btp_aa->bsp[count_aa] = NULL;

	for (index=0; index<segp->count; index++)
	{
		bsp = segp->bsp[index];
		if (bsp->repr == Seq_repr_seg)
			ReportFindRawSeq(bsp, btp_na, btp_aa);
	}

	if (btp_na->count_set == 0)
		btp_na->count_set = segp->count_set;
	for (index=(Int2) (btp_na->count_set-1); index>=0; index--)
	{
		btp_na->found_match=FALSE;
		sep = segp->sepp[index];
		SeqEntryExplore(sep, (Pointer) btp_na, CheckRelevanceOfSep);
		if (btp_na->found_match)
		{
		   last=index+1;
		   break;
		}
	}	
	if (btp_na->found_match)
	{
	     btp_na->sepp = 
		      (SeqEntryPtr PNTR) MemNew(last*sizeof(SeqEntryPtr));
	     btp_na->count_set = last;
	     for (index=0; index<last; index++)
		btp_na->sepp[index] = segp->sepp[index];
	}

	if (btp_aa->count_set == 0)
		btp_aa->count_set = segp->count_set;
	for (index=(Int2) (btp_aa->count_set-1); index>=0; index--)
	{
		btp_aa->found_match=FALSE;
		sep = segp->sepp[index];
		SeqEntryExplore(sep, (Pointer) btp_aa, CheckRelevanceOfSep);
		if (btp_aa->found_match)
		{
		   last=index+1;
		   break;
		}
	}	
	if (btp_aa->found_match)
	{
	     btp_aa->sepp = 
		      (SeqEntryPtr PNTR) MemNew(last*sizeof(SeqEntryPtr));
	     btp_aa->count_set = last;
	     for (index=0; index<last; index++)
		btp_aa->sepp[index] = segp->sepp[index];
	}

	if (segp->sepp)
		segp->sepp = MemFree(segp->sepp);

}

/************************************************************************
*
*	ReportGetSegNum()
*		SeqEntryExplore function to make a list of all segmented
*		bioseqs in a seqentry.
*
*************************************************************************/

void ReportGetSegNum (SeqEntryPtr sep, Pointer data, Int4 index, Int2 indent)

{
	SegmentPtr segp;
	SeqEntryPtr PNTR sepp;
	BioseqPtr bsp;
	SeqLocPtr slp;

	if (IS_Bioseq(sep))
	{
		if (MAKERPT_LOOK_FOR_SEQ == FALSE)
		{
			bsp = (BioseqPtr) sep->data.ptrvalue;
			segp = (SegmentPtr) data;
			if (bsp->seq_ext_type == 1)
			{
				segp->bsp[segp->count] = bsp;
			}
			else
			{
				segp->bsp[segp->count] = bsp;
			}
			segp->count++;
		}
		else
		{
			bsp = (BioseqPtr) sep->data.ptrvalue;
			segp = (SegmentPtr) data;
			if (bsp->seq_ext_type == 1)
			{
				segp->bsp[segp->count] = bsp;
				slp = bsp->seq_ext;
				segp->count++;
				while (slp)
				{ 
					bsp = BioseqFind(SeqLocId(slp));
					if (bsp != NULL)
					{
						if (ReportCompareToSegpList(bsp, segp) == FALSE && bsp != NULL)
						{
							segp->bsp[segp->count] = bsp;
							++(segp->count);
						}
					}
					slp = slp->next;
				}
			}
			else
			{
				if (ReportCompareToSegpList(bsp, segp) == FALSE && bsp != NULL)
				{
					segp->bsp[segp->count] = bsp;
					segp->count++;
				}
			}
		}
	}
	else
	{
		segp = (SegmentPtr) data;
		if (segp->sepp == NULL)
	    	   segp->sepp = 
		    (SeqEntryPtr PNTR) MemNew((segp->range)*sizeof(SeqEntryPtr));
		sepp = segp->sepp;
		sepp[segp->count_set] = sep;
		segp->count_set++;
	}
	
	return;

}	/* GetSegNum */

void CheckRelevanceOfSep(SeqEntryPtr sep, Pointer data, Int4 index, Int2 indent)

{
	BiotablePtr btp;
	BioseqPtr bsp;

	btp = (BiotablePtr) data;

	if (btp->found_match)	/* been there, done that */
		return;

	if (IS_Bioseq(sep))
	{
		bsp = (BioseqPtr) sep->data.ptrvalue;
		for (index=0; index<btp->count; index++)
			if (bsp == btp->bsp[index])
			{
				btp->found_match = TRUE;
				break;
			}
	}

	return;
}

/**************************************************************************
* static Boolean ReportCompareToSegpList (BioseqPtr bsp, SegmentPtr segp)
*
*	Compare the bsp with the bsp's already in segp.
**************************************************************************/
static Boolean ReportCompareToSegpList (BioseqPtr bsp, SegmentPtr segp)

{
	Boolean retval=FALSE;
	Int2 index;

	if (bsp != NULL)
	{
		for (index=0; index<segp->count; index++)
		{
			if (bsp == segp->bsp[index])
			{
				retval = TRUE;
				break;
			}
		}
	}	
	return retval;
}

/******************************************************************
*void MovePubsToFirstSeg (BiotablePtr btp_na, BiotablePtr btp_aa)
*
*	Move pubs to the first entry on the BiotablePtr for
*	the report generator.
*****************************************************************/

void MovePubsToFirstSeg (BiotablePtr btp_na, BiotablePtr btp_aa)

{
	Boolean drop_psp;
	Int2 index;
	PubStructPtr psp, psp1;
	ValNodePtr vnp, vnp1, vnp_base, vnp_start, vnp_temp;

	for (index=0; index<btp_na->count; index++)
	{
	   if (index == 0)
	   {
		vnp_start = btp_na->Pub[index];
		vnp1 = btp_na->Pub[index];
		if (vnp1)
			while (vnp1->next != NULL)
				vnp1 = vnp1->next;
		vnp_base=vnp1;
	   }
	   else 
	   {
		vnp = btp_na->Pub[index];
		while (vnp)
		{
			drop_psp = FALSE;
			psp = vnp->data.ptrvalue;
			for (vnp1=vnp_start; vnp1; vnp1=vnp1->next)
			{
			    psp1 = vnp1->data.ptrvalue;
			    if (psp1->vn.data.ptrvalue == psp->vn.data.ptrvalue)
			    {
				drop_psp = TRUE;
				break;
		  	    }
			    else if (psp1->start == 3 && psp->start == 3)
			    {
				if (psp1->Pub == psp->Pub)
			    	{
					drop_psp = TRUE;
					break;
			    	} 		
			    }
			}
			vnp_temp = vnp->next;
			vnp->next = NULL;
			if (drop_psp)
			{
				CatPspInfo(psp, psp1);
				FreePubStruct(psp);
				ValNodeFree(vnp);
			}
			else
			{
				if (vnp_base)
				{
					vnp_base->next = vnp;
					vnp_base = vnp_base->next;
				}
				else
					vnp_base = vnp;
			}
			vnp = vnp_temp;
		}
		btp_na->Pub[index] = NULL;
	   }
	}

	for (index=0; index<btp_aa->count; index++)
	{
	   if (index == 0)
	   {
		vnp_start = btp_aa->Pub[index];
		vnp1 = btp_aa->Pub[index];
		if (vnp1)
			while (vnp1->next != NULL)
				vnp1 = vnp1->next;
		vnp_base=vnp1;
	   }
	   else 
	   {
		vnp = btp_aa->Pub[index];
		while (vnp)
		{
			drop_psp = FALSE;
			psp = vnp->data.ptrvalue;
			for (vnp1=vnp_start; vnp1; vnp1=vnp1->next)
			{
			    psp1 = vnp1->data.ptrvalue;
			    if (psp1->vn.data.ptrvalue == psp->vn.data.ptrvalue)
			    {
				drop_psp = TRUE;
				break;
		  	    }
			    else if (psp1->start == 3 && psp->start == 3)
			    {
				if (psp1->Pub == psp->Pub)
			    	{
					drop_psp = TRUE;
					break;
			    	} 		
			    }
			}
			vnp_temp = vnp->next;
			vnp->next = NULL;
			if (drop_psp)
			{
				CatPspInfo(psp, psp1);
				FreePubStruct(psp);
				ValNodeFree(vnp);
			}
			else
			{
				if (vnp_base)
				{
					vnp_base->next = vnp;
					vnp_base = vnp_base->next;
				}
				else
					vnp_base = vnp;
			}
			vnp = vnp_temp;
		}
		btp_aa->Pub[index] = NULL;
	   }
	}
	
	return;
}


/******************************************************************************
*
*	ReportFindRawSeq to find the raw sequences recursively from the
*	segmented sequences.
*
******************************************************************************/

static void ReportFindRawSeq(BioseqPtr bsp, BiotablePtr btp_na, BiotablePtr btp_aa)
{

	BioseqPtr bsp2;
	SeqLocPtr slp;
	SeqIdPtr sip;
	Int2 count, total;

	for (slp = (SeqLocPtr) bsp->seq_ext; slp != NULL; slp = slp->next)
	{
		sip = SeqLocId(slp);
		if (sip != NULL)
		{
			bsp2 = BioseqFind(sip);
			if (bsp2 != NULL)
			{
				if (bsp2->repr == Seq_repr_raw ||
					bsp2->repr == Seq_repr_const)
				{
					for (count=0, total=0; count<btp_na->count; count++)
					{
						if (bsp2 == btp_na->bsp[count])
						{
							total++;
							btp_na->table[count] = bsp;
							btp_na->seg_num[count] = total;
						}
						else 
						{
							total = 0;
						}
					}
					for (count=0, total=0; count<btp_aa->count; count++)
					{
						if (bsp2 == btp_aa->bsp[count])
						{
							total++;
							btp_aa->table[count] = bsp;
							btp_aa->seg_num[count] = total;
						}
						else 
						{
							total = 0;
						}
					}
				}
				else if (bsp2->repr == Seq_repr_seg ||
					bsp->repr == Seq_repr_const)
					ReportFindRawSeq(bsp2, btp_na, btp_aa);
			}
		}
	}
}

/*************************************************************************
* Int4 report_setup (BiotablePtr btp, FFPrintArrayPtr PNTR papp)
*
*	How many "paragraphs" are needed?
*************************************************************************/

Int4 report_setup (BiotablePtr btp, FFPrintArrayPtr PNTR papp)

{
	Boolean feat_banner, descr_banner, pub_banner, seq_banner;
        FFPrintArrayPtr pap;
        Int2 count, index1, num;
        Int2Ptr num_of_feats, num_of_pubs, descr_count, descr_set_count;
        Int2Ptr PNTR descr_bsp_count;
        Int2Ptr PNTR descr_table_count;
        Int4 index, max, total;
        Int4Ptr num_of_seqblks;
	Uint1 choice;
	Uint1Ptr move_source, move_div, move_tax;
	Uint1Ptr PNTR print_table_title;
	Uint1Ptr PNTR print_bioseq_title;

        num_of_feats = (Int2Ptr) MemNew((btp->count)*sizeof(Int2));
        num_of_pubs = (Int2Ptr) MemNew((btp->count)*sizeof(Int2));
        num_of_seqblks = (Int4Ptr) MemNew((btp->count)*sizeof(Int4));
        descr_count = (Int2Ptr) MemNew(REP_NUM_OF_SEQ_DESC*sizeof(Int2));
	descr_set_count = (Int2Ptr) MemNew(REP_NUM_OF_SEQ_DESC*sizeof(Int2));
	descr_table_count = (Int2Ptr PNTR) MemNew((btp->count)*sizeof(Int2Ptr));
	descr_bsp_count = (Int2Ptr PNTR) MemNew((btp->count)*sizeof(Int2Ptr));
        move_source = (Uint1Ptr) MemNew((btp->count)*sizeof(Uint1));
        move_div = (Uint1Ptr) MemNew((btp->count)*sizeof(Uint1));
        move_tax = (Uint1Ptr) MemNew((btp->count)*sizeof(Uint1));

	if (! MAKERPT_DO_EDIT_MODE)
	{
             print_bioseq_title = 
		(Uint1Ptr PNTR) MemNew((btp->count)*sizeof(Uint1Ptr));
             print_table_title = 
		(Uint1Ptr PNTR) MemNew((btp->count)*sizeof(Uint1Ptr));
	}

	for (index=0; index<REP_NUM_OF_SEQ_DESC; index++)
	{
		if (index == Seq_descr_pub)
			continue;	/* pubs done down below */
		choice = (Uint1) index;
		descr_set_count[index] = CountDescrInSet(btp, choice);
	}

        for (count=0; count<btp->count; count++)
	{
	    descr_table_count[count] = 
		(Int2Ptr) MemNew(REP_NUM_OF_SEQ_DESC*sizeof(Int2));
	    descr_bsp_count[count] = 
		(Int2Ptr) MemNew(REP_NUM_OF_SEQ_DESC*sizeof(Int2));
	    for (index=0; index<REP_NUM_OF_SEQ_DESC; index++)
	    {
	    	if (index == Seq_descr_pub)
			continue;	/* pubs done down below */
		choice = (Uint1) index;
		descr_table_count[count][index] = 
			CountDescrInTable(btp, count, choice);
		descr_bsp_count[count][index] = 
			CountDescrInBioseq(btp, count, choice);
	    }
	    if (! MAKERPT_DO_EDIT_MODE)
	    {
		num = descr_bsp_count[count][(Int2)Seq_descr_title];
		print_bioseq_title[count] = 
			(Uint1Ptr) MemNew(num*sizeof(Uint1));
		num = descr_table_count[count][(Int2)Seq_descr_title];
		print_table_title[count] = 
			(Uint1Ptr) MemNew(num*sizeof(Uint1));
	    }
	}
	if (! MAKERPT_DO_EDIT_MODE)
	{
	    OrganizeGBBlockParts (btp, move_source, move_div, move_tax);
            for (count=0; count<btp->count; count++)
		LookForRedundantTitles(btp, count, descr_set_count[Seq_descr_title], print_bioseq_title[count], descr_bsp_count[count][Seq_descr_title], print_table_title[count], descr_table_count[count][Seq_descr_title]);
	}	
        for (count=(btp->count-1); count>=0; count--)
	{
                flat2asn_delete_locus_user_string();
                flat2asn_install_locus_user_string(btp->locus[count]);
                flat2asn_delete_accession_user_string();
                flat2asn_install_accession_user_string(btp->access[count]);
	}
	ReportOrganizeSeqFeat(btp);

        total=0;
        for (count=0; count<btp->count; count++)
        {
        	total++;
		num_of_feats[count] = ReportGetNumOfFeats(btp, count);
                num_of_pubs[count] = OrganizePubs(btp, count);
		num_of_seqblks[count] = GetNumOfSeqBlks(btp, count);
                total += num_of_feats[count];
                total += num_of_pubs[count];
                total += num_of_seqblks[count];
 
        }

        for (count=0; count<btp->count; count++)
	{
	    for (index=0; index<REP_NUM_OF_SEQ_DESC; index++)
	    {
	    	if (index == Seq_descr_pub)
			continue;	/* pubs done down below */
		if (MAKERPT_DO_EDIT_MODE || index != Seq_descr_title)
		{
			total += descr_table_count[count][index];
			total += descr_bsp_count[count][index];
		}
		else
		{
			
		    for (index1=0; index1<descr_table_count[count][Seq_descr_title]; index1++)
		    {
			if (print_table_title[count][index1] == 1)
				total++;
		    }
		    for (index1=0; index1<descr_bsp_count[count][Seq_descr_title]; index1++)
		    {
			if (print_bioseq_title[count][index1] == 1)
				total++;
		    }
		}
	    }
	}

        for (count=0; count<btp->count; count++)
	{
	   if (MAKERPT_DO_EDIT_MODE)
	   {
		if (move_source[count] > 0)
			total++;
		if (move_div[count] > 0)
			total++;
		if (move_tax[count] > 0)
			total++;
	    }
	    else
	    {
		if (move_source[count] > 1)
			total++;
		if (move_div[count] > 1)
			total++;
		if (move_tax[count] > 1)
			total++;
	    }
	}

	for (index=0; index<REP_NUM_OF_SEQ_DESC; index++)
	{
		if (index == Seq_descr_pub)
			continue;	/* pubs done down below */
		total += descr_set_count[index];
	}

	descr_banner=FALSE;
	for (index=0; index<REP_NUM_OF_SEQ_DESC; index++)
	{
	    if (index == Seq_descr_pub)
		continue;	/* pubs done down below */
	    if (descr_set_count[index] > 0)
	    {
		descr_banner=TRUE;
		break;
	    }
	}

	if (! MAKERPT_DO_EDIT_MODE && ! descr_banner)
	{
        	for (count=0; count<btp->count; count++)
		{
	    		if (move_source[count] == 3 || 
				move_div[count] == 3 ||
					move_tax[count] == 3)
	    		{
				descr_banner=TRUE;
				break;
	    		}
		}
	}

	feat_banner=FALSE;
        for (count=0; count<btp->count; count++)
	{
                max = (Int4) num_of_feats[count];
		if (max > 0)
		{
			feat_banner=TRUE;
			break;
	    	}
	}

	pub_banner=FALSE;
        for (count=0; count<btp->count; count++)
        {
                max = (Int4) num_of_pubs[count];
		if (max > 0)
		{
			pub_banner=TRUE;
			break;
		}
	}
	seq_banner=FALSE;
        for (count=0; count<btp->count; count++)
        {
                for (index=0; index<num_of_seqblks[count]; index++)
                {
                    if (num_of_seqblks[count] > 0)
		    {
			seq_banner=TRUE;
			break;
		    }
		}
	}
	if (descr_banner)
		total++;
	if (feat_banner)
		total++;
	if (pub_banner)
		total++;
	if (seq_banner)
		total++;

        *papp = (FFPrintArrayPtr) MemNew((size_t) total*sizeof(FFPrintArray));
        pap = *papp;
 
/* The next line sets the static Int4, pap_index, in ReportLoadPap    */
        ReportLoadPap(NULL, NULL, NULL, 0, 0, (Uint1)0, (Uint1)0, 0, REPORT_OTHER);

	
	if (descr_banner)
            ReportLoadPap(pap, PrintDescrBanner, btp, 0, 0, (Uint1)0, (Uint1)0, line_estimate[5], REPORT_OTHER);

	for (index=0; index<REP_NUM_OF_SEQ_DESC; index++)
	{
	    if (index == Seq_descr_pub)
		continue;	/* pubs done down below */
	    for (index1=0; index1<(descr_set_count[index]); index1++)
	    {
		choice = (Uint1) index;
                ReportLoadPap(pap, PrintSetDescr, btp, index1, 0, choice, (Uint1)0, line_estimate[5], REPORT_SET_DESCRIPTOR);
	    }
	}

	if (! MAKERPT_DO_EDIT_MODE)
	{ /* Print out the first source, div, tax descriptor at top level. */
            for (count=0; count<btp->count; count++)
	    {
		if (move_source[count] == 3)
                   ReportLoadPap(pap, PrintSourceDescr, btp, count, 0, Seq_descr_genbank, (Uint1)0, line_estimate[5], REPORT_OTHER);
		if (move_div[count] == 3)
                   ReportLoadPap(pap, PrintDivDescr, btp, count, 0, Seq_descr_genbank, (Uint1)0, line_estimate[5], REPORT_OTHER);
		if (move_tax[count] == 3)
                   ReportLoadPap(pap, PrintTaxDescr, btp, count, 0, Seq_descr_genbank, (Uint1)0, line_estimate[5], REPORT_OTHER);
	    }
	}

	if (feat_banner)
        	ReportLoadPap(pap, PrintFeatureBanner, btp, 0, 0, (Uint1)0, (Uint1)0, line_estimate[5], REPORT_OTHER);

        for (count=0; count<btp->count; count++)
	{
                max = (Int4) num_of_feats[count];
                for (index=0; index<max; index++)
                    ReportLoadPap(pap, ReportPrintFeatByNumber, btp, count, index, (Uint1)0, (Uint1)0, line_estimate[8], REPORT_FEATURE);
	}

	if (pub_banner)
        	ReportLoadPap(pap, PrintPubBanner, btp, 0, 0, (Uint1)0, (Uint1)0, line_estimate[5], REPORT_OTHER);

        for (count=0; count<btp->count; count++)
        {
                flat2asn_delete_locus_user_string();
                flat2asn_install_locus_user_string(btp->locus[count]);
                flat2asn_delete_accession_user_string();
                flat2asn_install_accession_user_string(btp->access[count]);
                max = (Int4) num_of_pubs[count];
                for (index=0; index<max; index++)
                    ReportLoadPap(pap, ReportPrintPubsByNumber, btp, count, index, (Uint1)0, (Uint1)0, line_estimate[5], REPORT_REFERENCE);
	}

	if (seq_banner)
        	ReportLoadPap(pap, PrintSeqBanner, btp, 0, 0, (Uint1)0, (Uint1)0, line_estimate[5], REPORT_OTHER);

        for (count=0; count<btp->count; count++)
        {
		for (index=0; index<REP_NUM_OF_SEQ_DESC; index++)
		{
	 	   choice = (Uint1) index;	
	    	   if (index != Seq_descr_pub)
			for (index1=0; index1<descr_table_count[count][index]; index1++)
			{
			  if (MAKERPT_DO_EDIT_MODE || index != Seq_descr_title)
                	    ReportLoadPap(pap, PrintTableDescr, btp, count, index1, choice, (Uint1)0, line_estimate[5], REPORT_TABLE_DESCRIPTOR);
			  else if (print_table_title[count][index1] == 1)
                	    ReportLoadPap(pap, PrintTableDescr, btp, count, index1, choice, (Uint1)0, line_estimate[5], REPORT_TABLE_DESCRIPTOR);
			}
			
		}

                ReportLoadPap(pap, ReportPrintSeqId, btp, count, 0, (Uint1)0, (Uint1)0, line_estimate[5], REPORT_OTHER);
		for (index=0; index<REP_NUM_OF_SEQ_DESC; index++)
		{
	 	   choice = (Uint1) index;	
	    	   if (index != Seq_descr_pub)
			for (index1=0; index1<descr_bsp_count[count][index]; index1++)
			{
			  if (MAKERPT_DO_EDIT_MODE || index != Seq_descr_title)
                	    ReportLoadPap(pap, PrintBioseqDescr, btp, count, index1, choice, (Uint1)0, line_estimate[5], REPORT_BIOSEQ_DESCRIPTOR);
			  else if (print_bioseq_title[count][index1] == 1)
                	    ReportLoadPap(pap, PrintBioseqDescr, btp, count, index1, choice, (Uint1)0, line_estimate[5], REPORT_BIOSEQ_DESCRIPTOR);
			}
		}
		if (! MAKERPT_DO_EDIT_MODE)
		{
		   if (move_source[count] == 2)
                   ReportLoadPap(pap, PrintSourceDescr, btp, count, 0, Seq_descr_genbank, (Uint1)0, line_estimate[5], REPORT_OTHER);
		   if (move_div[count] == 2)
                   ReportLoadPap(pap, PrintDivDescr, btp, count, 0, Seq_descr_genbank, (Uint1)0, line_estimate[5], REPORT_OTHER);
		   if (move_tax[count] == 2)
                   ReportLoadPap(pap, PrintTaxDescr, btp, count, 0, Seq_descr_genbank, (Uint1)0, line_estimate[5], REPORT_OTHER);
		}

                for (index=0; index<num_of_seqblks[count]; index++)
                {
                   if (num_of_seqblks[count] == index+1)
                      ReportLoadPap(pap, ReportPrintSeqBlk, btp, count, index, (Uint1)1, (Uint1)0, line_estimate[9], REPORT_SEQUENCE);
                   else
                      ReportLoadPap(pap, ReportPrintSeqBlk, btp, count, index, (Uint1)0, (Uint1)0, line_estimate[9], REPORT_SEQUENCE);
                }
	}


        num_of_feats = MemFree(num_of_feats);
        num_of_pubs = MemFree(num_of_pubs);
        num_of_seqblks = MemFree(num_of_seqblks);
        descr_count = MemFree(descr_count);
        descr_set_count = MemFree(descr_set_count);
        for (count=0; count<btp->count; count++)
	{
	    descr_table_count[count] = MemFree(descr_table_count[count]);
	    descr_bsp_count[count] = MemFree(descr_bsp_count[count]);
		if (! MAKERPT_DO_EDIT_MODE)
		{
             		print_bioseq_title[count] = 
				MemFree(print_bioseq_title[count]);
             		print_table_title[count] = 
				MemFree(print_table_title[count]);
		}
	}
        descr_table_count = MemFree(descr_table_count);
        descr_bsp_count = MemFree(descr_bsp_count);
        move_source = MemFree(move_source);
        move_div = MemFree(move_div);
        move_tax = MemFree(move_tax);
	if (! MAKERPT_DO_EDIT_MODE)
	{
             	print_bioseq_title = MemFree(print_bioseq_title);
             	print_table_title = MemFree(print_table_title);
	}

	return total;
}	/* report_setup */

 
/*****************************************************************************
*void ReportLoadPap(FFPrintArrayPtr pap, FFPapFct fct, BiotablePtr btp, Int2 count, Int4 index, Uint1 last, Uint1 printxx, Uint1 element_type)
*
*       This function places the parameters in the correct spaces in the
*       FFPrintArrayPtr.
*
****************************************************************************/
 
void ReportLoadPap(FFPrintArrayPtr pap, FFPapFct fct, BiotablePtr btp, Int2 count, Int4 index, Uint1 last, Uint1 printxx, Int2 estimate, Uint1 element_type)
 
{
        static Int4 pap_index;
	ValNodePtr descr;
 
        if (! pap)
        {
                pap_index=0;
        }
        else
        {
                pap[pap_index].fct = fct;
                pap[pap_index].btp = btp;
                pap[pap_index].count = count;
                pap[pap_index].index = index;
                pap[pap_index].last = last;
                pap[pap_index].printxx = printxx;
                pap[pap_index].estimate = estimate;
                pap[pap_index].sfp = NULL;
                pap[pap_index].pub = NULL;
                pap[pap_index].descr = NULL;
	    	if (MAKERPT_DO_EDIT_MODE)
		{
		    if (element_type == REPORT_SET_DESCRIPTOR)
		    {
			descr = GetADescrInSet (btp, last, count);
                	pap[pap_index].descr = descr;
		    }
		    else if (element_type == REPORT_TABLE_DESCRIPTOR)
		    {
			descr = GetADescrInTable (btp, count, last, (Int2) index);
                	pap[pap_index].descr = descr;
		    }
		    else if (element_type == REPORT_BIOSEQ_DESCRIPTOR)
		    {
			descr = GetADescrInBioseq (btp, count, last, (Int2)index);
                	pap[pap_index].descr = descr;
		    }
		    else if (element_type == REPORT_FEATURE)
		    {
			if (btp->sfpListsize[count] != 0)
				pap[pap_index].sfp = btp->sfpList[count][index];
		    }
		    else if (element_type == REPORT_REFERENCE)
			ReportGetPapRefPtr (btp, count, index, pap_index, pap);
		}
                pap_index++;
        }
 
        return;
}



/**************************************************************************
*void ReportGetPapRefPtr (BiotablePtr btp, Int2 count, Int4 ext_index, Int4 pap_index, FFPrintArrayPtr pap)
*
*       Get the ValNodePtr associated with a certain reference.
**************************************************************************/

void ReportGetPapRefPtr (BiotablePtr btp, Int2 count, Int4 ext_index, Int4 pap_index, FFPrintArrayPtr pap)

{
        Int2 print_index, index; 
        PubStructPtr psp=NULL; 
        ValNodePtr vnp1, vnp=btp->Pub[count];
 
        print_index= (Int2) ext_index;
 
        if (vnp == NULL) 
                return;
 
        for (vnp1=vnp, index=0; index<print_index; index++)
                vnp1=vnp1->next;
 
        if (vnp1)
                psp = vnp1->data.ptrvalue;
                 
        if (psp) 
        {
                if (psp->start == 1)
                {
                        pap[pap_index].descr = psp->SeqDescr;
                }
                else if (psp->start == 2)
                {
                        if (psp->citcount > 0)
                                pap[pap_index].sfp = psp->citfeat[0];
                }
                else if (psp->start == 3)
                        pap[pap_index].pub = psp->Pub;
        }
        return;  
}        

/*************************************************************************
*void OrganizeGBBlockParts (BiotablePtr btp, Uint1Ptr move_source, Uint1Ptr move_div, Uint1Ptr move_tax)
*
*	Determine whether parts of the GenBankBlock, in a segmented set,
*	can be moved up to the top for a "neat" presentation.
*
*	In a segmented set:
*		- look for first GenBankBlock with source, div, or tax
*		info.  Mark this state 3.  Keep the CharPtr in source,
*		div, or tax.
*		- check following GenBankBlocks to see if the source, div
*		or tax is identical with that already stored.  If it is,
*		set state to 1; if not, set state to 2.
*		- if there is no div, source, or tax, set state (for
*		each one) to 0.
*
*	The results of this analysis are stored in the arrays move_source,
*	move_div, and move_tax with the following code:
*	0: not present
*	1: present and already represented at the set level,
*	2: present, but keep with entry,
*	3: present, move to set level.
************************************************************************/
void OrganizeGBBlockParts (BiotablePtr btp, Uint1Ptr move_source, Uint1Ptr move_div, Uint1Ptr move_tax)

{
	BioseqPtr bsp;
	CharPtr div, source, tax;
	GBBlockPtr gbbp, gbbp1;
	Int2 index, index1, seg_num, total;
	ValNodePtr descr;

	for (index=0; index<btp->count; index++)
	{
		move_div[index] = 0;
		move_source[index] = 0;
		move_tax[index] = 0;
	}

	for (index=0; index<btp->count; index++)
	{
		seg_num = btp->seg_num[index];
		if (seg_num == 0) /* not a segmented set */
		{
			gbbp=NULL;
			bsp=btp->bsp[index];
			descr=bsp->descr;
			while (descr)
			{
				if (descr->choice == Seq_descr_genbank)
				{
					gbbp = descr->data.ptrvalue;
					break;
				}
				descr=descr->next;
			}
			if (gbbp)
			{
			    if (gbbp->source)
				move_source[index] = 3;
			    else
				move_source[index] = 0;
			    if (gbbp->div)
				move_div[index] = 3;
			    else
				move_div[index] = 0;
			    if (gbbp->taxonomy)
				move_tax[index] = 3;
			    else
				move_tax[index] = 0;
			}
		}
		else if (seg_num == 1) /* segmented set */
		{
			gbbp=NULL;
			div = NULL;
			source = NULL;
			tax = NULL;
			bsp=btp->bsp[index];
			descr=bsp->descr;
			while (descr)
			{
				if (descr->choice == Seq_descr_genbank)
				{
					gbbp = descr->data.ptrvalue;
					break;
				}
				descr=descr->next;
			}
			if (gbbp)
			{
			    if (gbbp->source)
			    {
				move_source[index] = 3;
				source = gbbp->source;
			    }
			    else
				move_source[index] = 0;
			    if (gbbp->div)
			    {
				move_div[index] = 3;
				div = gbbp->div;
			    }
			    else
				move_div[index] = 0;
			    if (gbbp->taxonomy)
			    {
				move_tax[index] = 3;
				tax = gbbp->taxonomy;
			    }
			    else
				move_tax[index] = 0;
			}
			total = btp->seg_total[index];
			for (index1=(index+1); index1<(index+total); index1++)
			{
				gbbp1=NULL;
				bsp=btp->bsp[index1];
				descr=bsp->descr;
				while (descr)
				{
				   if (descr->choice == Seq_descr_genbank)
				   {
					gbbp1 = descr->data.ptrvalue;
					break;
				   }
				   descr=descr->next;
				}
				if (gbbp1)
				{
			    	    if (gbbp1->source)
				    {
					if (source)
					{
					    if (StringCmp(source, gbbp1->source) == 0)
						move_source[index1] = 1;
					    else 
						move_source[index1] = 2;
					}
					else
					{
						move_source[index1] = 3;
						source = gbbp1->source;
					}
				    }
			    	    else
					move_source[index1] = 0;

			    	    if (gbbp1->div)
				    {
					if (div)
					{
					    if (StringCmp(div, gbbp1->div) == 0)
						move_div[index1] = 1;
					    else 
						move_div[index1] = 2;
					}
					else
					{
						move_div[index1] = 3;
						div = gbbp1->div;
					}
				    }
			    	    else
					move_div[index1] = 0;

			    	    if (gbbp1->taxonomy)
				    {
					if (tax)
					{
					    if (StringCmp(tax, gbbp1->taxonomy) == 0)
						move_tax[index1] = 1;
					    else 
						move_tax[index1] = 2;
					}
					else
					{
						move_tax[index1] = 3;
						tax = gbbp1->taxonomy;
					}
				    }
			    	    else
					move_tax[index1] = 0;
				}
			}
		}
	}

}	/* OrganizeGBBlockParts */

void LookForRedundantTitles(BiotablePtr btp, Int2 count, Int2 descr_set_count, Uint1Ptr print_bioseq_title, Int2 descr_bsp_count, Uint1Ptr print_table_title, Int2 descr_table_count)

{
	CharPtr string;
	Int2 index, index1;
	ValNodePtr descr;

	for (index=0; index<descr_set_count; index++)
	{
		descr = GetADescrInSet (btp, Seq_descr_title, index);
		string = descr->data.ptrvalue;
		for (index1=0; index1<descr_table_count; index1++)
		{
		    descr = GetADescrInTable (btp, count, Seq_descr_title, index1);
		    if (StringCmp(descr->data.ptrvalue, string) == 0)
			print_table_title[index1] = 0;
		    else
			print_table_title[index1] = 1;
		}
		for (index1=0; index1<descr_bsp_count; index1++)
		{
		    descr = GetADescrInBioseq (btp, count, Seq_descr_title, index1);
		    if (StringCmp(descr->data.ptrvalue, string) == 0)
			print_bioseq_title[index1] = 0;
		    else
			print_bioseq_title[index1] = 1;
		}
	}

	for (index=0; index<descr_table_count; index++)
	{
		descr = GetADescrInTable (btp, count, Seq_descr_title, index);
		string = descr->data.ptrvalue;
		for (index1=0; index1<descr_bsp_count; index1++)
		{
		    if (print_bioseq_title[index1] != 0)
		    {
		    	descr = GetADescrInBioseq (btp, count, Seq_descr_title, index1);
		    	if (StringCmp(descr->data.ptrvalue, string) == 0)
				print_bioseq_title[index1] = 0;
		    	else
				print_bioseq_title[index1] = 1;
		    }
		}
	}
}

void report_cleanup (BiotablePtr btp_aa, BiotablePtr btp_na, FFPrintArrayPtr pap)

{
	if (btp_aa)
		BiotableFree(btp_aa);
	if (btp_na)
		BiotableFree(btp_na);
	if (pap)
		MemFree(pap);
	return;
}

void ReportPrintPubsByNumber (BiotablePtr btp, Int2 count)

{
	Boolean first_loc;
	Char buffer[10];
	CharPtr string;
        Int2 index, print_index;
        PubStructPtr psp;
        ValNodePtr vnp1, vnp=btp->Pub[count];

        print_index= (Int2) btp->pap_index;

        if (vnp == NULL)
                return;

        for (vnp1=vnp, index=0; index<print_index; index++)
                vnp1=vnp1->next;

        if (vnp1)
        {
		first_loc=FALSE;
		sprintf(buffer, "%ld", (long) (print_index+1));
                psp = vnp1->data.ptrvalue;
		string = TemplatePrintPubs(btp, count, psp);
                StartPrint(btp, 0, 12, ASN2FF_GB_MAX, NULL);
                AddString(btp, buffer);
                TabToColumn(btp, 13);
                AddString(btp, string);
		EndPrint (btp);
		string = MemFree(string);
		index=0;
                StartPrint(btp, 12, 12, ASN2FF_GB_MAX, NULL);
		string = TemplatePrintPubBioseqLoc(btp, count, &index, psp, &first_loc);
		while (index != -1)
		{
                   AddString(btp, string);
		   string = MemFree(string);
		   string = TemplatePrintPubBioseqLoc(btp, count, &index, psp, &first_loc);
		}
		index=0;
		string = TemplatePrintPubSeqFeatLoc(btp, count, &index, psp, &first_loc);
		while (index != -1)
		{
                   AddString(btp, string);
		   string = MemFree(string);
		   string = TemplatePrintPubSeqFeatLoc(btp, count, &index, psp, &first_loc);
		}
		index=0;
		string = TemplatePrintPubSeqFeatCitLoc(btp, count, &index, psp, &first_loc);
		while (index != -1)
		{
                   AddString(btp, string);
		   string = MemFree(string);
		   string = TemplatePrintPubSeqFeatCitLoc(btp, count, &index, psp, &first_loc);
		}
		EndPrint (btp);
        }
}

CharPtr TemplatePrintPubs (BiotablePtr btp, Int2 count, PubStructPtr psp)

{
        StdPrintOptionsPtr Spop=btp->Spop;
	ValNodePtr pub;

	pub = FlatRefBest(psp->pub, btp->error_msgs, TRUE);

        if (! StdFormatPrint((Pointer)pub,
                (AsnWriteFunc)PubAsnWrite, "StdPub", Spop))
                        Message(MSG_ERROR, "StdFormatPrint failed");

        if (Spop->ptr != NULL && *((CharPtr) (Spop->ptr)) != '\0')
                return Spop->ptr;
        else
                return StringSave ("Empty Data\n");
}

CharPtr TemplatePrintPubBioseqLoc (BiotablePtr btp, Int2 count, Int2Ptr index, PubStructPtr psp, Boolean PNTR first_loc)

{
	BioseqPtr bsp;
	CharPtr string=NULL;
        StdPrintOptionsPtr Spop=btp->Spop;
	ValNodePtr id, pub;

	pub = FlatRefBest(psp->pub, btp->error_msgs, TRUE);

	
	if (*index == psp->bspcount)
	{
		*index = -1;
	}
	else
	{
	    bsp = psp->bsp[*index];
	    id = bsp->id;
	    while (string == NULL)
	    {
	   	if (! *first_loc)
	   	{
        		if (! StdFormatPrint((Pointer)id,
             	     	(AsnWriteFunc)SeqIdAsnWrite, "StartingPubSeqId", Spop))
              		         Message(MSG_ERROR, "StdFormatPrint failed");
			if (Spop->ptr)
				*first_loc = TRUE;
		}
		else
		{
        		if (! StdFormatPrint((Pointer)id,
             	     	(AsnWriteFunc)SeqIdAsnWrite, "StdPubSeqId", Spop))
              		         Message(MSG_ERROR, "StdFormatPrint failed");
		}

       		if (Spop->ptr != NULL && *((CharPtr) (Spop->ptr)) != '\0')
                	string = Spop->ptr;
		if (id->next)
			id = id->next;
		else
			break;
	    }

	    if (string == NULL)
                string = StringSave ("Empty Data\n");

	    (*index)++;
	}
	
	return string;
}

CharPtr TemplatePrintPubSeqFeatLoc (BiotablePtr btp, Int2 count, Int2Ptr index, PubStructPtr psp, Boolean PNTR first_loc)

{
	CharPtr string=NULL;
	SeqFeatPtr sfp;
        StdPrintOptionsPtr Spop=btp->Spop;
	ValNodePtr pub;

	pub = FlatRefBest(psp->pub, btp->error_msgs, TRUE);

	
	if (*index == psp->citcount)
	{
		*index = -1;
	}
	else
	{
	   sfp = psp->citfeat[*index];
	   if (! *first_loc)
	   {
        	if (! StdFormatPrint((Pointer)sfp,
             	     (AsnWriteFunc)SeqFeatAsnWrite, "StartingPubFeatLocation", Spop))
              		         Message(MSG_ERROR, "StdFormatPrint failed");
		*first_loc = TRUE;
	   }
	   else
	   {
        	if (! StdFormatPrint((Pointer)sfp,
             	     (AsnWriteFunc)SeqFeatAsnWrite, "StdPubFeatLocation", Spop))
              		         Message(MSG_ERROR, "StdFormatPrint failed");
	   }


       	    if (Spop->ptr != NULL && *((CharPtr) (Spop->ptr)) != '\0')
                string = Spop->ptr;
            else
                string = StringSave ("Empty Data\n");
	    (*index)++;
	}
	
	return string;
}

CharPtr TemplatePrintPubSeqFeatCitLoc (BiotablePtr btp, Int2 count, Int2Ptr index, PubStructPtr psp, Boolean PNTR first_loc)

{
	CharPtr string=NULL;
	SeqFeatPtr sfp;
        StdPrintOptionsPtr Spop=btp->Spop;
	ValNodePtr pub;

	pub = FlatRefBest(psp->pub, btp->error_msgs, TRUE);

	
	if (*index == psp->pubcount)
	{
		*index = -1;
	}
	else
	{
	   sfp = psp->pubfeat[*index];
	   if (! *first_loc)
	   {
        	if (! StdFormatPrint((Pointer)sfp,
             	     (AsnWriteFunc)SeqFeatAsnWrite, "StartingPubFeatLocation", Spop))
              		         Message(MSG_ERROR, "StdFormatPrint failed");
		*first_loc = TRUE;
	   }
	   else
	   {
        	if (! StdFormatPrint((Pointer)sfp,
             	     (AsnWriteFunc)SeqFeatAsnWrite, "StdPubFeatLocation", Spop))
              		         Message(MSG_ERROR, "StdFormatPrint failed");
	   }


       	    if (Spop->ptr != NULL && *((CharPtr) (Spop->ptr)) != '\0')
                string = Spop->ptr;
            else
                string = StringSave ("Empty Data\n");
	    (*index)++;
	}
	
	return string;
}

void ReportPrintSeqId (BiotablePtr btp, Int2 count)

{
	BioseqPtr bsp=btp->bsp[count];
	CharPtr string;
        StdPrintOptionsPtr Spop=btp->Spop;

        if (! StdFormatPrint((Pointer)(bsp->id),
            (AsnWriteFunc)SeqIdAsnWrite, "StdSequenceId", Spop))
             	Message(MSG_ERROR, "StdFormatPrint failed");
       	if (Spop->ptr != NULL && *((CharPtr) (Spop->ptr)) != '\0')
              string = Spop->ptr;
        else
              string = StringSave ("Empty Data\n");
        StartPrint(btp, 0, 0, ASN2FF_GB_MAX, NULL);
        AddString(btp, string);
	EndPrint (btp);
	string = MemFree(string);
	
}

/**********************************************************************
*Int2 CountDescrInSet (BiotablePtr btp, Uint1 choice)
*
*	Count the number of descriptors in Bioseq-sets of type choice.
**********************************************************************/

Int2 CountDescrInSet (BiotablePtr btp, Uint1 choice)

{
	BioseqSetPtr bssp;
	GBBlockPtr gbb;
	Int2 count=0;
	Int4 count_set=btp->count_set, index;
	SeqEntryPtr PNTR sepp=btp->sepp;
	ValNodePtr descr;
	
	for (index=0; index<count_set; index++)
	{
		if (sepp)
		{
			bssp = sepp[index]->data.ptrvalue;
			descr = bssp->descr;
			while (descr)
			{
				if (! MAKERPT_DO_EDIT_MODE &&
				    choice == Seq_descr_genbank &&
					descr->choice == Seq_descr_genbank)
				{
					gbb = descr->data.ptrvalue;
					if (gbb->extra_accessions ||
				     	    gbb->keywords ||
						gbb->origin ||
					     	    gbb->date ||
							gbb->entry_date)
						count++;
				}
				else if (descr->choice == choice)	
					count++;
				descr = descr->next;
			}
		}
	}
	
	return count;
}

/**********************************************************************
*Int2 GetADescrInSet (BiotablePtr btp, Uint1 choice, Int2 ext_index)
*
*	Get a descriptor in Bioseq-sets of type choice.
**********************************************************************/

ValNodePtr GetADescrInSet (BiotablePtr btp, Uint1 choice, Int2 ext_index)

{
	BioseqSetPtr bssp;
	Int2 count=0;
	Int4 count_set=btp->count_set, index;
	SeqEntryPtr PNTR sepp=btp->sepp;
	ValNodePtr descr;
	
	for (index=0; index<count_set; index++)
	{
		if (sepp)
		{
			bssp = sepp[index]->data.ptrvalue;
			descr = bssp->descr;
			while (descr)
			{
				if (descr->choice == choice)	
				{
					if (count == ext_index)
						return descr;
					count++;
				}
				descr = descr->next;
			}
		}
	}
	
	return NULL;
}

/***************************************************************************
*Int2 CountDescrInTable (BiotablePtr btp, Int2 count, Uint1 choice)
*
*	count the number of descriptors, of type choice, in btp->table[count]
*
**************************************************************************/

Int2 CountDescrInTable (BiotablePtr btp, Int2 count, Uint1 choice)

{
	BioseqPtr bsp;
	GBBlockPtr gbb;
	Int2 descr_count=0;
	ValNodePtr descr;

	if (btp->seg_num[count] == 1)
	{
		bsp = btp->table[count];
		if (bsp)
		{
			descr = bsp->descr;
			while (descr)
			{
				if (! MAKERPT_DO_EDIT_MODE && 
				    choice == Seq_descr_genbank &&
					descr->choice == Seq_descr_genbank)
				{
					gbb = descr->data.ptrvalue;
					if (gbb->extra_accessions ||
				     	    gbb->keywords ||
						gbb->origin ||
					     	    gbb->date ||
							gbb->entry_date)
						descr_count++;
				}
				else if (descr->choice == choice)	
					descr_count++;
				descr = descr->next;
			}
		}
	}
	return descr_count;
}

/***************************************************************************
*Int2 GetADescrInTable (BiotablePtr btp, Int2 count, Uint1 choice)
*
*	Gets a descriptor, of type choice, from btp->table[count]
*
**************************************************************************/

ValNodePtr GetADescrInTable (BiotablePtr btp, Int2 count, Uint1 choice, Int2 ext_index)

{
	BioseqPtr bsp;
	Int2 descr_count=0;
	ValNodePtr descr;

	if (btp->seg_num[count] == 1)
	{
		bsp = btp->table[count];
		if (bsp)
		{
			descr = bsp->descr;
			while (descr)
			{
				if (descr->choice == choice)	
				{
					if (descr_count == ext_index)
						return descr;
					descr_count++;
				}
				descr = descr->next;
			}
		}
	}
	return NULL;
}

/***************************************************************************
*Int2 CountDescrInBioseq (BiotablePtr btp, Int2 count, Uint1 choice)
*
*	count the number of descriptors, of type choice, in btp->bsp[count]
*
**************************************************************************/

Int2 CountDescrInBioseq (BiotablePtr btp, Int2 count, Uint1 choice)

{
	BioseqPtr bsp;
	GBBlockPtr gbb;
	Int2 descr_count=0;
	ValNodePtr descr;

	bsp = btp->bsp[count];
	if (bsp)
	{
		descr = bsp->descr;
		while (descr)
		{
			if (! MAKERPT_DO_EDIT_MODE &&
			   choice == Seq_descr_genbank &&
				descr->choice == Seq_descr_genbank)
			{
				gbb = descr->data.ptrvalue;
				if (gbb->extra_accessions ||
				     gbb->keywords ||
					gbb->origin ||
					     gbb->date ||
						gbb->entry_date)
					descr_count++;
			}
			else if (descr->choice == choice)	
				descr_count++;
			descr = descr->next;
		}
	}
	return descr_count;
}

/***************************************************************************
*Int2 GetADescrInBioseq (BiotablePtr btp, Int2 count, Uint1 choice)
*
*	Gets a descriptor, of type choice, from btp->bsp[count]
*
**************************************************************************/

ValNodePtr GetADescrInBioseq (BiotablePtr btp, Int2 count, Uint1 choice, Int2 ext_index)

{
	BioseqPtr bsp;
	Int2 descr_count=0;
	ValNodePtr descr;

	bsp = btp->bsp[count];
	if (bsp)
	{
		descr = bsp->descr;
		while (descr)
		{
			if (descr->choice == choice)	
			{
				if (descr_count == ext_index)
					return descr;
				descr_count++;
			}
			descr = descr->next;
		}
	}
	return NULL;
}


void PrintSetDescr (BiotablePtr btp, Int2 index)

{
	CharPtr string;
        StdPrintOptionsPtr Spop=btp->Spop;
	Uint1 choice;
	ValNodePtr descr;

	choice = btp->pap_last;

	descr = GetADescrInSet (btp, choice, index);

	if (descr)
	{
	   if (MAKERPT_DO_EDIT_MODE)
	   {
        	if (! StdFormatPrint((Pointer)descr,
               	 (AsnWriteFunc)SeqDescAsnWrite, "StdSeqDesc", Spop))
               	         Message(MSG_ERROR, "StdFormatPrint failed");
	   }
	   else
	   {
        	if (! StdFormatPrint((Pointer)descr,
               	 (AsnWriteFunc)SeqDescAsnWrite, "StdReportSeqDesc", Spop))
               	         Message(MSG_ERROR, "StdFormatPrint failed");
	   }
/*
        	if (! StdFormatPrint((Pointer)descr,
               	 (AsnWriteFunc)SeqDescAsnWrite, "StdSeqDesc", Spop))
               	         Message(MSG_ERROR, "StdFormatPrint failed");
        	if (! StdFormatPrint((Pointer)descr,
               	 (AsnWriteFunc)SeqDescAsnWrite, "StdReportSeqDesc", Spop))
               	         Message(MSG_ERROR, "StdFormatPrint failed");

*/
      	 	if (Spop->ptr != NULL && *((CharPtr) (Spop->ptr)) != '\0')
               		string = Spop->ptr;
       		else
                	string = StringSave ("Empty Data\n");

        	StartPrint(btp, 0, 0, ASN2FF_GB_MAX, NULL);
        	AddString(btp, string);
		EndPrint (btp);

		string = MemFree(string);
	}
}

void PrintTableDescr (BiotablePtr btp, Int2 count)

{
	CharPtr string;
        Int2 index = (Int2) btp->pap_index;
        StdPrintOptionsPtr Spop=btp->Spop;
	Uint1 choice;
	ValNodePtr descr;

	choice = btp->pap_last;

	descr = GetADescrInTable (btp, count, choice, index);

	if (descr)
	{
	   if (MAKERPT_DO_EDIT_MODE)
	   {
        	if (! StdFormatPrint((Pointer)descr,
               	 (AsnWriteFunc)SeqDescAsnWrite, "StdSeqDesc", Spop))
               	         Message(MSG_ERROR, "StdFormatPrint failed");
	   }
	   else
	   {
        	if (! StdFormatPrint((Pointer)descr,
               	 (AsnWriteFunc)SeqDescAsnWrite, "StdReportSeqDesc", Spop))
               	         Message(MSG_ERROR, "StdFormatPrint failed");
	   }
/*
        	if (! StdFormatPrint((Pointer)descr,
               	 (AsnWriteFunc)SeqDescAsnWrite, "StdSeqDesc", Spop))
               	         Message(MSG_ERROR, "StdFormatPrint failed");
        	if (! StdFormatPrint((Pointer)descr,
               	 (AsnWriteFunc)SeqDescAsnWrite, "StdReportSeqDesc", Spop))
               	         Message(MSG_ERROR, "StdFormatPrint failed");
*/

      	 	if (Spop->ptr != NULL && *((CharPtr) (Spop->ptr)) != '\0')
               		string = Spop->ptr;
       		else
                	string = StringSave ("Empty Data\n");

        	StartPrint(btp, 0, 0, ASN2FF_GB_MAX, NULL);
        	AddString(btp, string);
		EndPrint (btp);

		string = MemFree(string);
	}
}

void PrintBioseqDescr (BiotablePtr btp, Int2 count)

{
	CharPtr string;
        Int2 index = (Int2) btp->pap_index;
        StdPrintOptionsPtr Spop=btp->Spop;
	Uint1 choice;
	ValNodePtr descr;

	choice = btp->pap_last;

	descr = GetADescrInBioseq (btp, count, choice, index);

	if (descr)
	{
	   if (MAKERPT_DO_EDIT_MODE)
	   {
        	if (! StdFormatPrint((Pointer)descr,
               	 (AsnWriteFunc)SeqDescAsnWrite, "StdSeqDesc", Spop))
               	         Message(MSG_ERROR, "StdFormatPrint failed");
	   }
	   else
	   {
        	if (! StdFormatPrint((Pointer)descr,
               	 (AsnWriteFunc)SeqDescAsnWrite, "StdReportSeqDesc", Spop))
               	         Message(MSG_ERROR, "StdFormatPrint failed");
	   }

      	 	if (Spop->ptr != NULL && *((CharPtr) (Spop->ptr)) != '\0')
               		string = Spop->ptr;
       		else
                	string = StringSave ("Empty Data\n");

        	StartPrint(btp, 0, 0, ASN2FF_GB_MAX, NULL);
        	AddString(btp, string);
		EndPrint (btp);

		string = MemFree(string);
	}
}

void PrintSourceDescr (BiotablePtr btp, Int2 count)

{
	CharPtr string;
        Int2 index = (Int2) btp->pap_index;
        StdPrintOptionsPtr Spop=btp->Spop;
	Uint1 choice;
	ValNodePtr descr;

	choice = btp->pap_last;

	descr = GetADescrInBioseq (btp, count, choice, 0);

	if (descr)
	{
        	if (! StdFormatPrint((Pointer)descr,
               	 (AsnWriteFunc)SeqDescAsnWrite, "StdSourceDesc", Spop))
               	         Message(MSG_ERROR, "StdFormatPrint failed");

      	 	if (Spop->ptr != NULL && *((CharPtr) (Spop->ptr)) != '\0')
               		string = Spop->ptr;
       		else
                	string = StringSave ("Empty Data\n");

        	StartPrint(btp, 0, 0, ASN2FF_GB_MAX, NULL);
        	AddString(btp, string);
		EndPrint (btp);

		string = MemFree(string);
	}
}

void PrintDivDescr (BiotablePtr btp, Int2 count)

{
	CharPtr string;
        Int2 index = (Int2) btp->pap_index;
        StdPrintOptionsPtr Spop=btp->Spop;
	Uint1 choice;
	ValNodePtr descr;

	choice = btp->pap_last;

	descr = GetADescrInBioseq (btp, count, choice, 0);

	if (descr)
	{
        	if (! StdFormatPrint((Pointer)descr,
               	 (AsnWriteFunc)SeqDescAsnWrite, "StdDivDesc", Spop))
               	         Message(MSG_ERROR, "StdFormatPrint failed");

      	 	if (Spop->ptr != NULL && *((CharPtr) (Spop->ptr)) != '\0')
               		string = Spop->ptr;
       		else
                	string = StringSave ("Empty Data\n");

        	StartPrint(btp, 0, 0, ASN2FF_GB_MAX, NULL);
        	AddString(btp, string);
		EndPrint (btp);

		string = MemFree(string);
	}
}

void PrintTaxDescr (BiotablePtr btp, Int2 count)

{
	CharPtr string;
        Int2 index = (Int2) btp->pap_index;
        StdPrintOptionsPtr Spop=btp->Spop;
	Uint1 choice;
	ValNodePtr descr;

	choice = btp->pap_last;

	descr = GetADescrInBioseq (btp, count, choice, 0);

	if (descr)
	{
        	if (! StdFormatPrint((Pointer)descr,
               	 (AsnWriteFunc)SeqDescAsnWrite, "StdTaxDesc", Spop))
               	         Message(MSG_ERROR, "StdFormatPrint failed");

      	 	if (Spop->ptr != NULL && *((CharPtr) (Spop->ptr)) != '\0')
               		string = Spop->ptr;
       		else
                	string = StringSave ("Empty Data\n");

        	StartPrint(btp, 0, 0, ASN2FF_GB_MAX, NULL);
        	AddString(btp, string);
		EndPrint (btp);

		string = MemFree(string);
	}
}

/*********************************************************************
*void DoPubCheck (BiotablePtr btp_na, BiotablePtr btp_aa)
*
*	Run "CollectPubs" on each bsp.
*****************************************************************/

void DoPubCheck (BiotablePtr btp_na, BiotablePtr btp_aa)

{
	BioseqPtr bsp;
	Int2 index;
	ValNodePtr vnp;

	for (index=0; index<btp_na->count; index++)
	{
		bsp = btp_na->bsp[index];
		vnp = btp_na->Pub[index];
		if (vnp)
		{
			UniquePubs(&vnp);
			btp_na->Pub[index] = vnp;
		}
	}

	for (index=0; index<btp_aa->count; index++)
	{
		bsp = btp_aa->bsp[index];
		vnp = btp_aa->Pub[index];
		if (vnp)
		{
			UniquePubs(&vnp);
			btp_aa->Pub[index] = vnp;
		}
	}
}

/************************************************************************
*	void ReportStorePubInfo (BioseqContextPtr bcp, BioseqPtr bsp, ValNodePtr PNTR vnp, Boolean error_msgs);
*
*	Get information about pubs.  Eventually a check will be done
*	to ascertain whether pubs are complete enough to warrant 
*	printing out.  If there are no printable pubs, the entry will
*	be killed.  The return value will indicate whether a problem
*	exists.  For now, "-1" indicates a problem, "0" indicates
*	at least one good pub.
************************************************************************/

void ReportStorePubInfo (BioseqContextPtr bcp, BioseqPtr bsp, ValNodePtr PNTR vnpp, Boolean error_msgs)

{
	GetPubDescr (bcp, bsp, vnpp);
	GetPubFeat (bcp, vnpp);
	UniquePubs(vnpp);

}

/*************************************************************************
*Int2 ReportGetNumOfFeats (BiotablePtr btp, Int2 count)
*
*	Get the number of features.  Should this be more than an Int2?
**************************************************************************/
Int2 ReportGetNumOfFeats (BiotablePtr btp, Int2 count)

{
	return btp->sfpListsize[count];	
}

/************************************************************************
*void ReportOrganizeSeqFeat(BiotablePtr btp)
*
*	Collects up all the features, except for those that have
*	been printed as pubs.  
************************************************************************/

void ReportOrganizeSeqFeat(BiotablePtr btp)

{
	BioseqContextPtr bcp;
	BiotablePtr btp_other=btp->btp_other;
	Boolean found_sfp;
	ImpFeatPtr ifp;
	Int2 index, index1, currentsize=0, totalsize=0;
	SeqFeatPtr sfp = NULL;
	SortStructPtr List = NULL;

	for (index=0; index<btp->count; index++)
	{
		bcp = btp->bcp[index];

		sfp=BioseqContextGetSeqFeat(bcp, 0, sfp, NULL, 2);

		while (sfp !=NULL)
		{
		    switch (sfp->data.choice) {
		    case 6: /* Pubs are already captured by "StorePubInfo". */
		    	break;
		    case 8: /* This case must be before the generic case */
		    	ifp = (ImpFeatPtr) sfp->data.value.ptrvalue;
		    	if (StringCmp(ifp->key, "Site-ref") == 0 && 
		    		sfp->cit != NULL)
		    	{
		    		break;
		    	} /* If none of the above is true, execute generic. */
		    default:
		    	if (currentsize == totalsize)
		    	{
		    	   if (totalsize == 0)
		              List = EnlargeSortList(NULL, &(totalsize), (Int2) 5);
		    	   else
		 	      List = EnlargeSortList(List, &(totalsize), (Int2) 5);
		    	}
			found_sfp=FALSE;
			for (index1=0; index1<currentsize; index1++)
			{
				if (sfp == List[index1].sfp)
				{
					found_sfp = TRUE;
					break;
				}
			}	
			if (! found_sfp)			
			{
				List[currentsize].sfp = sfp;
				List[currentsize].bsp = btp->bsp[index];
				List[currentsize].table = btp->table[index];
				if (btp_other->count == 1)
				     List[currentsize].other = btp_other->bsp[0];
				currentsize++;
			}
		    	break;
		    }
		    sfp=BioseqContextGetSeqFeat(bcp, 0, sfp, NULL, 2);
		}
	    if (currentsize > 0)
	    	HeapSort((VoidPtr) List, (size_t) currentsize, sizeof(SortStruct), CompareSfpForHeap);
	}
	CpListToBtp(List, btp->sfpList, currentsize, btp->sfpListsize, 0);
	if (totalsize > 0)
		List = MemFree(List);
}	/* ReportOrganizeSeqFeat */

/*************************************************************************
*void ReportPrintFeatByNumber (BiotablePtr btp, Int2 count)
*
*	Print the features using the PrintTemplates.
*
*************************************************************************/

void ReportPrintFeatByNumber (BiotablePtr btp, Int2 count)

{
	CharPtr string;
        Int2 feat_index = (Int2) btp->pap_index;
	SeqFeatPtr sfp;
        StdPrintOptionsPtr Spop=btp->Spop;

        sfp = btp->sfpList[count][feat_index];

        if (! StdFormatPrint((Pointer)sfp,
                (AsnWriteFunc)SeqFeatAsnWrite, "StdSeqFeat", Spop))
                        Message(MSG_ERROR, "StdFormatPrint failed");

        if (Spop->ptr != NULL && *((CharPtr) (Spop->ptr)) != '\0')
                string = Spop->ptr;
        else
                string = StringSave ("Empty Data\n");

        StartPrint(btp, 0, 0, ASN2FF_GB_MAX, NULL);
        AddString(btp, string);
	EndPrint (btp);

	string = MemFree(string);
}

/****************************************************************************
* void ReportCheckSeqPort (BiotablePtr btp, Int2 count, Int4 start)
*
*	This function checks a SeqPortPtr, maintained on the Biotable Ptr, 
*	and compares it's BioseqPtr to that of the BioseqPtr associated
*	with segment count of the btp.  At present, used only for nucleic
*	acids (4/14/94).
****************************************************************************/

void ReportCheckSeqPort (BiotablePtr btp, Int2 count, Int4 start)
{
	BioseqPtr bsp=btp->bsp[count];
	SeqPortPtr spp=btp->spp;

	if (spp)
	{
		if (bsp == spp->bsp)
		{
			if (spp->curpos != start)
				SeqPortSeek(spp, start, SEEK_SET);
		}
		else
		{
			SeqPortFree(spp);
			spp = SeqPortNew(bsp, 0, -1, 0, Seq_code_iupacna);
			if (start != 0)
				SeqPortSeek(spp, start, SEEK_SET);
		}
	}
	else
	{
		spp = SeqPortNew(bsp, 0, -1, 0, Seq_code_iupacna);
		if (start != 0)
			SeqPortSeek(spp, start, SEEK_SET);
	}

	btp->spp = spp;

	return;
}

/*****************************************************************************
*
*	"ReportPrintSequence" to get the biological sequence (in iupacna or
*	iupacaa format)	and put it in a buffer suitable for Genbank 
*	or EMBL format.
*
*	The variables "start" and "stop" allow one to read from a point 
*	not at the beginning of the sequence to a point not at the end
*	of the sequence.
*
*	Rewrite to store in a buffer and print out more at once????????
*****************************************************************************/

void ReportPrintSequence (BiotablePtr btp, Int2 count, Int4 start, Int4 stop)

{
	BioseqPtr bsp=btp->bsp[count];
	Char buffer[MAX_BTP_BUF], num_buffer[10];
	CharPtr ptr = &(buffer[0]), num_ptr;
	Int4 index, inner_index, inner_stop, total=start;
	SeqPortPtr spp;
	Uint1 residue;


	if (btp->format == REPORT_ON_NA || btp->format == REPORT_ON_EVERYTHING)
	{
		StartPrint(btp, 0, 0, ASN2FF_GB_MAX, NULL);
		num_ptr = &(num_buffer[0]);
		sprintf(num_ptr, "%9ld", (long) (total+1));
		while ((*ptr=*num_ptr) != '\0')
		{
			ptr++; num_ptr++;
		}
		*ptr = ' '; ptr++;
		ReportCheckSeqPort(btp, count, start);
		spp = btp->spp;
		if (stop == -1)
			stop = spp->stop;
		for (index=start; index<=stop; index += 10)
		{
			if (stop < (index+10))
				inner_stop = stop;
			else
				inner_stop = index+9;
			for (inner_index=index; inner_index<=inner_stop; inner_index++)
			{
				residue=SeqPortGetResidue(spp);
				if ( !IS_residue(residue) && residue != INVALID_RESIDUE )
					continue;
				if (residue == INVALID_RESIDUE)
					residue = (Uint1) 'X';
		
				*ptr = TO_LOWER(residue); ptr++;
			}
			total = inner_stop+1;
		/* Put in a space every ten, unless it's the end of a row. */
			if (ROUNDUP(total, 60) == total)
			{
				if (total != (start+1) && total != (stop+1))
				{
					*ptr = '\0';
					ptr = &buffer[0];
					AddString(btp, ptr);
					NewContLine(btp);
					num_ptr = &(num_buffer[0]);
					sprintf(num_ptr, "%9ld", (long) (total+1));
					while ((*ptr=*num_ptr) != '\0')
					{
						ptr++; num_ptr++;
					}
					*ptr = ' '; ptr++;
				}
			}
			else if (ROUNDUP(total, 10) == total)
			{
				*ptr = ' '; ptr++;
			}
		}
		*ptr = '\0';
		ptr = &buffer[0];
		AddString(btp, ptr);
	}
	else if (btp->format == REPORT_ON_AA)
	{
		total++;

		StartPrint(btp, 0, 0, ASN2FF_GB_MAX, NULL);
		num_ptr = &(num_buffer[0]);
		sprintf(num_ptr, "%9ld", (long) total);
		while ((*ptr=*num_ptr) != '\0')
		{
			ptr++; num_ptr++;
		}
		*ptr = ' '; ptr++;
		spp = SeqPortNew(bsp, start, stop, 0, Seq_code_ncbieaa);
		while ((residue=SeqPortGetResidue(spp)) != SEQPORT_EOF)
		{
			if ( !IS_residue(residue) && residue != INVALID_RESIDUE )
				continue;
			if (residue == INVALID_RESIDUE)
				residue = (Uint1) 'X';
	
			*ptr = residue; ptr++;
			if (ROUNDUP(total, 10) == total) 
			{
				if (ROUNDUP(total, 60) == total)
				{
					if (total != (start+1) && total != (stop+1))
					{
						*ptr = '\0';
						ptr = &buffer[0];
						AddString(btp, ptr);
						NewContLine(btp);
						num_ptr = &(num_buffer[0]);
						sprintf(num_ptr, "%9ld", (long) (total+1));
						while ((*ptr=*num_ptr) != '\0')
						{
							ptr++; num_ptr++;
						}
						*ptr = ' '; ptr++;
					}
				}
				else
				{
					*ptr = ' '; ptr++;
				}
			}
			total++;
		}
		*ptr = '\0';
		ptr = &buffer[0];
		AddString(btp, ptr);
		SeqPortFree(spp);
	}

	EndPrint(btp);


}	/* ReportPrintSequence */

/***********************************************************************
*void ReportPrintSeqBlk (BiotablePtr btp, Int2 count)
*
*       This function prints out a block of the sequence (at most
*       of size SEQ_BLK_SIZE).
*       After the last sequence block, the terminator is printed also.
***********************************************************************/

void ReportPrintSeqBlk (BiotablePtr btp, Int2 count)

{
        Int4 start, stop, index=btp->pap_index;
        Uint1 last=btp->pap_last;


        if (index == 0)
                start = 0;
        else
                start = index*SEQ_BLK_SIZE;
        if (last != LAST)
                stop = (index+1)*SEQ_BLK_SIZE - 1;
        else
                stop = -1;
        ReportPrintSequence(btp, count, start, stop);
}


void PrintFeatureBanner(BiotablePtr btp, Int2 count)

{
        StartPrint(btp, 0, 0, ASN2FF_GB_MAX, NULL);
        AddString(btp, "FEATURES:");
	EndPrint(btp);
}

void PrintPubBanner(BiotablePtr btp, Int2 count)

{
        StartPrint(btp, 0, 0, ASN2FF_GB_MAX, NULL);
        AddString(btp, "REFERENCES:");
	EndPrint(btp);
}

void PrintSeqBanner(BiotablePtr btp, Int2 count)

{
        StartPrint(btp, 0, 0, ASN2FF_GB_MAX, NULL);
        AddString(btp, "SEQUENCE:");
	EndPrint(btp);
}

void PrintCommentBanner(BiotablePtr btp, Int2 count)

{
        StartPrint(btp, 0, 0, ASN2FF_GB_MAX, NULL);
        AddString(btp, "COMMENT BLOCK:");
	EndPrint(btp);
}

void PrintDescrBanner(BiotablePtr btp, Int2 count)

{
        StartPrint(btp, 0, 0, ASN2FF_GB_MAX, NULL);
        AddString(btp, "DESCRIPTORS:");
	EndPrint(btp);
}
PackSeqPntNum(pspp);
				pspp2 = PackSeqPntNew();
				pspp2->strand = pspp->strand;
				intcnt = 0;	     /* use for included points */
				othercnt = 0;	 /* use for exclued points */
				for (i = 0; i < numpnt; i++)
				{
					tpos = PackSeqPntGet(pspp, i);
					if ((tpos < from) || (tpos > to))
					{
						othercnt++;
					}
					else
					{
						intcnt++;
						PackSeqPntPut(pspp2, tpos - from);
					}
				}
				if (! intcnt)  /* no points in region */
				{
					PackSeqPntFree(pspp2);
					break;
				}
				if (othercnt)
					*split = TRUE;
				if (pspp->fuzz != NULL)
				{
					ifp = IntFuzzNew();
					MemCopy((Pointer)ifp, (Pointer)(pspp->fuzz), sizeof(IntFuzz));
				}
				else
					ifp = NULL;

				if (strand == Seq_strand_minus)  /* rev comp */
				{
					IntFuzzClip(ifp, from, to, strand, split);
					pspp = pspp2;
					pspp2 = PackSeqPntNew();
					pspp2->strand = StrandCmp(pspp->strand);
					numpnt = PackSeqPntNum(pspp);
					numpnt--;
					for (i = numpnt; i >= 0; i--)	 /* reverse order */
					{
						tpos = PackSeqPntGet(pspp, i);
						PackSeqPntPut(pspp2, (len - tpos - 1));
					}
					PackSeqPntFree(pspp);
				}
				pspp2->id = SeqIdDup(newid);
				pspp2->fuzz = ifp;

				newhead = ValNodeNew(NULL);
				newhead->choice = SEQLOC_PACKED_PNT;
				newhead->data.ptrvalue = (Pointer)pspp2;

			}
            break;
        default:
            break;

	}
	return newhead;
}

/*****************************************************************************
*
*   IntFuzzClip()
*   	returns TRUE if clipped range values
*       in all cases, adjusts and/or complements IntFuzz
*       Designed for IntFuzz on SeqLocs
*
*****************************************************************************/
void IntFuzzClip(IntFuzzPtr ifp, Int4 from, Int4 to, Uint1 strand, BoolPtr split)
{
	Int4 len, tmp;

	if (ifp == NULL) return;
	len = to - from + 1;
	switch (ifp->choice)
	{
		case 1:      /* plus/minus - no changes */
		case 3:      /* percent - no changes */
			break;
		case 2:      /* range */
			if (ifp->a > to)     /* max */
			{
				*split = TRUE;
				ifp->a = to;
			}
			if (ifp->a < from) 
			{
				*split = TRUE;
				ifp->a = from;
			}
			if (ifp->b > to)     /* min */
			{
				*split = TRUE;
				ifp->b = to;
			}
			if (ifp->b < from) 
			{
				*split = TRUE;
				ifp->b = from;
			}
			ifp->a -= from;     /* adjust to window */
			ifp->b -= to;
			if (strand == Seq_strand_minus)
			{
				tmp = len - ifp->a;   /* reverse/complement */
				ifp->a = len - ifp->b;
				ifp->b = tmp;
			}
			break;
		case 4:     /* lim */
			if (strand == Seq_strand_minus)  /* reverse/complement */
			{
				switch (ifp->a)
				{
					case 1:    /* greater than */
						ifp->a = 2;
						break;
					case 2:    /* less than */
						ifp->a = 1;
						break;
					case 3:    /* to right of residue */
						ifp->a = 4;
						break;
					case 4:    /* to left of residue */
						ifp->a = 3;
						break;
					default:
						break;
				}
			}
			break;
	}
	return;
}

/*****************************************************************************
*
* BioseqInsert (from_id, from, to, strand, to_id, pos, from_feat, to_feat,
*                                                                  do_split)
*   	Inserts a copy the region "from"-"to" on "strand" of the Bioseq
*          identified by "from_id" into the Bioseq identified by "to_id" 
*          before "pos".
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
*					      virtual       raw      segmented        map
*					   +---------------------------------------------------
*	         virtual   |   length	    inst	  SeqLoc		 length
*					   +---------------------------------------------------
*				 raw   |   error        copy      SeqLoc         error
*					   +---------------------------------------------------
*		   segmented   |   error        inst      SeqLoc*        error
*					   +---------------------------------------------------
*				 map   |   error        inst*     SeqLoc         copy
*					   +---------------------------------------------------
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
			Boolean from_feat, Boolean to_feat, Boolean do_split)
{
	BioseqPtr tobsp, frombsp;
	Int4 len, i, ctr, tlen;
	Boolean from_type, to_type;
	Uint1 seqtype;
	SeqAnnotPtr sap, newsap;
	SeqFeatPtr sfp, newsfp, prevsfp, sfphead = NULL;
	BioseqContextPtr bcp;
	Boolean handled = FALSE;
	SeqPortPtr spp;
	Int2 residue;
	Boolean split, added = FALSE;
	SeqLocPtr newloc, curr, head, tloc, xloc, yloc, fake;
	SeqIntPtr sip;


	if ((from_id == NULL) || (to_id == NULL)) return FALSE;

	tobsp = BioseqFind(to_id);
	if (tobsp == NULL) return FALSE;

	len = BioseqGetLen(tobsp);

	if (pos == LAST_RESIDUE)
		pos = len - 1;
	else if (pos == APPEND_RESIDUE)
		pos = len;

	if ((pos < 0) || (pos > len)) return FALSE;

	frombsp = BioseqFind(from_id);
	if (frombsp == NULL) return FALSE;
	
	from_type = ISA_na(frombsp->mol);
	to_type = ISA_na(tobsp->mol);

	if (from_type != to_type) return FALSE;

	len = BioseqGetLen(frombsp);
	if (to == LAST_RESIDUE)
		to = len - 1;
	
	if ((from < 0) || (to >= len)) return FALSE;

	len = to - from + 1;

	if (tobsp->repr == Seq_repr_virtual)
	{
		if (frombsp->repr != Seq_repr_virtual)
			return FALSE;

		handled = TRUE;                    /* just length and features */
	}

 	if ((tobsp->repr == Seq_repr_raw) || (tobsp->repr == Seq_repr_const))
	{
		if (ISA_na(tobsp->mol))
		{
			seqtype = Seq_code_iupacna;
		}
		else
		{
			seqtype = Seq_code_ncbieaa;
		}

		if (tobsp->seq_data_type != seqtype)
			BioseqRawConvert(tobsp, seqtype);
		BSSeek(tobsp->seq_data, pos, SEEK_SET);
		Nlm_BSAdd(tobsp->seq_data, len);

		i = 0;

		spp = SeqPortNew(frombsp, from, to, strand, seqtype);
		while ((residue = SeqPortGetResidue(spp)) != SEQPORT_EOF)
		{
			if (! IS_residue(residue))
			{
				ErrPost(CTX_NCBIOBJ, 1, "Non-residue in BioseqInsert [%d]",
					(int)residue);
			}
			else
			{
				BSPutByte(tobsp->seq_data, residue);
				i++;
			}
		}
		SeqPortFree(spp);

		if (i != len)
		{
			ErrPost(CTX_NCBIOBJ, 1, "Tried to insert %ld residues but %ld went in",
				len, i);
			return FALSE;
		}

		handled = TRUE;
	}

	if ((tobsp->repr == Seq_repr_seg) || (tobsp->repr == Seq_repr_ref))
	{
		sip = SeqIntNew();
		sip->id = SeqIdDup(from_id);
		sip->from = from;
		sip->to = to;
		sip->strand = strand;
		tloc = ValNodeNew(NULL);
		tloc->choice = SEQLOC_INT;
		tloc->data.ptrvalue = (Pointer)sip;
		head = NULL;
		if (tobsp->repr == Seq_repr_seg)
		{
			fake = ValNodeNew(NULL);
			fake->choice = SEQLOC_MIX;
			fake->data.ptrvalue = (Pointer)(tobsp->seq_ext);
		}
		else
			fake = (SeqLocPtr)(tobsp->seq_ext);
		curr = NULL;
		ctr = 0;
		while ((curr = SeqLocFindNext(fake, curr)) != NULL)
		{
			if ((! added) && (ctr == pos))
			{
				newloc = SeqLocAdd(&head, tloc);
				added = TRUE;
			}
			tlen = SeqLocLen(curr);
			if ((! added) && ((ctr + tlen) > pos))  /* split interval */
			{
				yloc = NULL;
				xloc = SeqLocAdd(&yloc, curr);
				i = (pos - ctr) + SeqLocStart(curr);
			    newloc = SeqLocInsert(xloc, SeqLocId(xloc), i, 0, TRUE, NULL);
				xloc = newloc;
				yloc = newloc->next;
				SeqLocAdd(&head, xloc);
				SeqLocAdd(&head, tloc);
				SeqLocAdd(&head, yloc);
				SeqLocFree(xloc);
				SeqLocFree(yloc);
				added = TRUE;
			}
			else
				newloc = SeqLocAdd(&head, curr);
			ctr += tlen;
		}
		if ((! added) && (ctr == pos))
		{
			newloc = SeqLocAdd(&head, tloc);
			added = TRUE;
		}
		SeqLocFree(tloc);
		SeqLocFree(fake);
		if (tobsp->repr == Seq_repr_seg)
		{
			tobsp->seq_ext = (Pointer)head;
		}
		else
		{
			fake = ValNodeNew(NULL);
			fake->choice = SEQLOC_MIX;
			fake->data.ptrvalue = head;
			tobsp->seq_ext = (Pointer)fake;
		}
		handled = TRUE;
	}

	if (tobsp->repr == Seq_repr_map)
	{
		if (! ((frombsp->repr == Seq_repr_map) || (frombsp->repr == Seq_repr_virtual)))
			return FALSE;

		prevsfp = N L/*****************************************************************************
*   gbfeatdfn.h:
*   -- GenBank Feature table define file
*
* $Log: gbftdef.h,v $
 * Revision 1.2  1995/05/15  21:46:05  ostell
 * added Log line
 *
*
*                                                                  10-14-93
******************************************************************************/
#ifndef _GBFEATDFN_
#define _GBFEATDFN_


#define GBQUAL_anticodon          0
#define GBQUAL_bound_moiety       1
#define GBQUAL