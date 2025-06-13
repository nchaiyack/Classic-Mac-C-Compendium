/******************************************************************************
*
*	files that go with "asn2ff".
*		
*
*
******************************************************************************/
/* $Revision: 1.45 $ */ 
/*************************************
*
* $Log: asn2ff1.c,v $
 * Revision 1.45  1995/06/14  21:04:46  kans
 * moved asn2ff_flags to avoid link error
 *
 * Revision 1.44  1995/05/22  14:51:08  tatiana
 * remove old MatchNAFeatToGene function and add ASN2FF_SHOW_ALL_PUBS
 *
 * Revision 1.43  1995/05/19  21:25:06  kans
 * gene match code moved to sequtil (ostell)
 *
 * Revision 1.42  1995/05/15  21:46:05  ostell
 * added Log line
 *
*
**************************************/
#include "asn2ffp.h"
#include "parsegb.h"
#include "gbfeat.h"
#define NUM_ORDER 16
/* The following corresponds to NUM_SEQ_LINES lines each with 60 
residues/basepairs */
#define SEQ_BLK_SIZE (60*NUM_SEQ_LINES)
#define A2F_OTHER ( (Uint1)0)
#define A2F_SOURCE_FEATURE ( (Uint1)1)
#define A2F_FEATURE ( (Uint1)2)
#define A2F_REFERENCE ( (Uint1)3)




/* ---------------Function Prototypes ---------------*/
Int4 asn2gb_setup PROTO ((BiotablePtr btp_na, FFPrintArrayPtr PNTR papp));
Int4 asn2embl_setup PROTO ((BiotablePtr btp_na, FFPrintArrayPtr PNTR papp));
Int4 asn2gp_setup PROTO ((BiotablePtr btp_aa, FFPrintArrayPtr PNTR papp));
Int4 asn2ep_setup PROTO ((BiotablePtr btp_aa, FFPrintArrayPtr PNTR papp));
void LoadPap PROTO ((FFPrintArrayPtr pap, FFPapFct fct, BiotablePtr btp, Int2 count, Int4 index, Uint1 last, Uint1 printxx, Int2 estimate, Uint1 element_type)); 
static void SeqToArrays PROTO ((SegmentPtr segp, BiotablePtr btp_na, BiotablePtr btp_aa));
void GetSegNum PROTO ((SeqEntryPtr sep, Pointer data, Int4 index, Int2 indent));
Int2 GetNumAA PROTO ((SegmentPtr segp, Uint1 format, Boolean error_msgs));
Int2 GetNumNA PROTO ((SegmentPtr segp, Uint1 format, Boolean error_msgs));
static void GetAARefs PROTO ((BiotablePtr btp_na, BiotablePtr btp_aa));
static void GetNARefs PROTO ((BiotablePtr btp_na, BiotablePtr btp_aa));
static void GetLocusParts PROTO ((BiotablePtr btp));
static void UseGIforLocus PROTO ((BiotablePtr btp));
static void FindRawSeq PROTO ((BioseqPtr bsp, BiotablePtr btp_na, BiotablePtr btp_aa));
static Boolean CompareToSegpList PROTO ((BioseqPtr bsp, SegmentPtr segp));
Int2 ValidateAccession PROTO((CharPtr new_buf, CharPtr orig_buf));
CharPtr ValidateLocus PROTO((BiotablePtr btp, Int2 count, CharPtr buf_rlocus, Int2 total_segs, Int2 num_seg, CharPtr new_buf, CharPtr buf_locus, CharPtr orig_buf));
CharPtr MakeBaseAccession PROTO ((BiotablePtr btp));
CharPtr MakeBaseLocus PROTO((CharPtr base_locus, BiotablePtr btp, Int2 count));
CharPtr CheckLocusLength PROTO((BiotablePtr btp, CharPtr locus, Int2 locus_max, Int2 total_segs));
void PrintSequence PROTO ((BiotablePtr btp, Int2 count, Int4 start, Int4 stop));
void PrintEPSequence PROTO ((BiotablePtr btp, Int2 count, Int4 start, Int4 stop));
void CheckSeqPort PROTO ((BiotablePtr btp, Int2 count, Int4 start));
void PrintBaseCount PROTO ((BiotablePtr btp, Int2 count));
void GetMolInfo PROTO ((CharPtr buffer, Int2 count, BiotablePtr btp));
void PrintLocusLine PROTO((BiotablePtr btp, Int2 count));
void PrintEPLocusLine PROTO((BiotablePtr btp, Int2 count));
void PrintAccessLine PROTO((BiotablePtr btp, Int2 count));
void PrintSegmentLine PROTO((BiotablePtr btp, Int2 count));
void PrintKeywordLine PROTO((BiotablePtr btp, Int2 count));
void PrintDefinitionLine PROTO((BiotablePtr btp, Int2 count));
void PrintOriginLine PROTO ((BiotablePtr btp, Int2 count));
void PrintSourceLine PROTO ((BiotablePtr btp, Int2 count));
void PrintOrganismLine PROTO ((BiotablePtr btp, Int2 count));
void GetAASeqFeat PROTO((BiotablePtr btp));
void GetNASeqFeat PROTO((BiotablePtr btp));
void GetSeqFeat PROTO ((BiotablePtr btp));
void OrganizeSeqFeat PROTO ((BiotablePtr btp, Int2 count));
void MarkRedundantGeneFeats PROTO ((BiotablePtr btp));
void PrintPubsByNumber PROTO ((BiotablePtr btp, Int2 count));
CharPtr GetPDBSourceLine PROTO ((PdbBlockPtr pdb));
void PrintFeatHeader PROTO ((BiotablePtr btp, Int2 count));
void PrintTerminator PROTO ((BiotablePtr btp));
void PrintDateLines PROTO ((BiotablePtr btp, Int2 count));
void PrintXrefLine PROTO ((BiotablePtr btp, Int2 count));
Boolean CheckXrefLine PROTO ((BiotablePtr btp, Int2 count));
Int2 GetNumOfFeats PROTO ((BiotablePtr btp, Int2 count));
void CompareGenesToQuals PROTO ((BiotablePtr btp, Int2 count, Int2 ext_index));
Int2 MatchAAGeneToFeat PROTO ((BiotablePtr btp, Int2 count, SeqFeatPtr sfp, Int2 index));
Int2 MatchNAGeneToFeat PROTO ((BiotablePtr btp, Int2 count, SeqFeatPtr sfp, Int2 index));
void MatchNAFeatToGene PROTO ((BiotablePtr btp, Int2 count));
Int2 CheckGeneToFeatOverlap PROTO ((BiotablePtr btp, Int2 count, SeqLocPtr gene_loc, Int2 PNTR best_feat));
void CompareGenesToSourceQuals PROTO ((BiotablePtr btp, Int2 count));
void GBComAndFH PROTO ((BiotablePtr btp, Int2 count));
void GPComAndFH PROTO ((BiotablePtr btp, Int2 count));
void GBComAndXref PROTO ((BiotablePtr btp, Int2 count));
void EstablishProteinLink PROTO ((BiotablePtr btp_na, BiotablePtr btp_aa, Int2 count));
void set_flags PROTO ((Boolean show_seq, Boolean show_gi, Uint1 mode, Boolean error_msgs, Boolean bind_all, Uint1 format));
void GetPapRefPtr PROTO ((BiotablePtr btp, Int2 count, Int4 ext_index, Int4 pap_index, FFPrintArrayPtr pap));
void GetPapSeqFeatPtr PROTO ((BiotablePtr btp, Int2 count, Int4 ext_index, Int4 pap_index, FFPrintArrayPtr pap));
void PrintSeqBlk PROTO ((BiotablePtr btp, Int2 count));
void PrintEPSeqBlk PROTO ((BiotablePtr btp, Int2 count));
Int2 ConvertToNAImpFeat();




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

static void PrintTheString (FILE *fp, CharPtr string, CharPtr line_return)

{
	CharPtr return_ptr = line_return, buffer_ptr;
	Char buffer[100];

	buffer_ptr = &(buffer[0]);

	if (! fp)
		return;

	if (! string || *string == *return_ptr)
	{
		ErrPostEx(SEV_WARNING, 1, 1, 
			"CAUTION: NULL String in PrintTheString\n\n");
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
*	asn2ff is a stand-alone function for saving a flat file to a file.
**************************************************************************/

Boolean asn2ff (SeqEntryPtr sep, FILE *fp, Boolean show_seq, Boolean show_gi, Uint1 mode, Boolean error_msgs, Boolean bind_all, Uint1 format, StdPrintOptionsPtr spop)

{
  BiotablePtr      btp_aa;
  BiotablePtr      btp_na;
  Int4             index;
  FFPrintArrayPtr  pap;
  Int4             pap_size;
  Boolean          rsult;
  CharPtr          string;

  rsult = FALSE;
  if (sep != NULL && fp != NULL && spop != NULL) {
    pap_size = asn2ff_setup (sep, show_seq, show_gi, mode, error_msgs, bind_all,
                             format, spop, &pap, &btp_aa, &btp_na);
    if (pap_size > 0) {
      asn2ff_set_output (NULL, btp_aa, btp_na, "\n");
      for (index = 0; index < pap_size; index++) {
        string = FFPrint (pap, index, pap_size);
        if (string != NULL && *string != '\0') {
          PrintTheString (fp, string, "\n");
          string = MemFree (string);
        }
      }
      asn2ff_cleanup (btp_aa, btp_na, pap);
      rsult = TRUE;
    }
  }
  return rsult;
}


/***********************************************************************
*
*	asn2ff_entrez is a stand-alone function for writing output
*	to a file; arguments are the standard ones for Entrez.
**************************************************************************/

Boolean asn2ff_entrez (SeqEntryPtr sep, FILE *fp, Uint1 format)

{
  Boolean          rsult;
	
  rsult = SeqEntryToFlat(sep, fp, format, RELEASE_MODE);
  return rsult;
}

/***********************************************************************
*
*	SeqEntryToFlat is a stand-alone function that takes a SeqEntryPtr
*	and writes a flat file to a disk file.  If the formatting is
*	successful, TRUE is returned; otherwise FALSE is returned.
*
Choices for the Uint1's format and mode are defined in asn2ff.h.

For format they are:

GENBANK_FMT 	standard GenBank flat file for nucleotides
EMBL_FMT	standard EMBL flat file  for nucleotides
GENPEPT_FMT 	standard GenBank flat file for proteins
PSEUDOEMBL_FMT  a flavor of the EMBL flat file used by the "Authorin" program

The modes are:

RELEASE_MODE	this mode assures that all the requirements (e.g., identifiers
		features, references as described in the GenBank release notes
		and the feature table) are met.
		are met 
DUMP_MODE 	dump out the ASN.1 to a flat file
SEQUIN_MODE 	mode used by sequin
CHROMO_MODE 	mode used by Chromoscope
DIRSUB_MODE 	mode used by NCBI indexers during the "dirsub" process.
REVISE_MODE 	mode used by the "revise" program at NCBI (for in-house
		editing of entries).
*
**************************************************************************/

Boolean SeqEntryToFlat (SeqEntryPtr sep, FILE *fp, Uint1 format, Uint1 mode)

{
  BiotablePtr      btp_aa;
  BiotablePtr      btp_na;
  Boolean          rsult;
  Boolean 	   show_seq=TRUE, show_gi=TRUE,error_msgs=FALSE, bind_all=TRUE;
  CharPtr          string;
  FFPrintArrayPtr  pap;
  Int4             index, pap_size;
  StdPrintOptionsPtr Spop = NULL;

  rsult = FALSE;
  if (format == GENPEPT_FMT)
  {
       if (AllObjLoad () && SubmitAsnLoad () && SeqCodeSetLoad () &&
               PrintTemplateSetLoad ("asn2ff.prt"))
       {
               ErrShow();
       }
       Spop = StdPrintOptionsNew(NULL);
       if (Spop)
       {
               Spop->newline = "~";
               Spop->indent = "";
       }
       else
       {
               Message (MSG_FATAL, "StdPrintOptionsNew failed");
  	       return rsult;
       }
   }

  if (sep != NULL && fp != NULL) {
    pap_size = asn2ff_setup (sep, show_seq, show_gi, mode, error_msgs, 
			bind_all, format, Spop, &pap, &btp_aa, &btp_na);
    if (pap_size > 0) {
      asn2ff_set_output (NULL, btp_aa, btp_na, "\n");
      for (index = 0; index < pap_size; index++) {
        string = FFPrint (pap, index, pap_size);
        if (string != NULL && *string != '\0') {
          PrintTheString (fp, string, "\n");
          string = MemFree (string);
        }
      }
      asn2ff_cleanup (btp_aa, btp_na, pap);
      rsult = TRUE;
    }
  }
  return rsult;
}

CharPtr FFPrint (FFPrintArrayPtr pap, Int4 index, Int4 pap_size)

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

	if (pa.printxx == PRINTXX)
		PrintXX(btp);

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

Int4 asn2ff_setup (SeqEntryPtr sep, Boolean show_seq, Boolean show_gi, Uint1 mode, Boolean error_msgs, Boolean bind_all, Uint1 format, StdPrintOptionsPtr Spop, FFPrintArrayPtr PNTR papp, BiotablePtr PNTR btp_aa_p, BiotablePtr PNTR btp_na_p)

{
	BiotablePtr btp_aa, btp_na;
	SegmentPtr segp;
	Int2 bioseq_count, num_of_aa, num_of_na;
	Int4 pap_size = -1;

	if (format == EMBLPEPT) /* Turn off Validators for EMBLPEPT */
		mode = DUMP_MODE;

	set_flags(show_seq, show_gi, mode, error_msgs, bind_all, format);

	flat2asn_install_accession_user_string("SET-UP");
	flat2asn_install_locus_user_string("SET-UP");

	if (ASN2FF_LOOK_FOR_SEQ == FALSE)
	{
	      bioseq_count = (Int2) SeqEntryCount(sep);	
	}
	else
	{
	      segp = SegmentNew((Int2) 1);
	      SeqEntryExplore(sep, (Pointer) segp, CountTheSeqEntrys);
	      bioseq_count = (Int2) (segp->count);
	      SegmentFree(segp);
	}

	segp = SegmentNew(bioseq_count);
	segp->sep = sep;

	SeqEntryExplore (sep, (Pointer) segp, GetSegNum);

	if (format == EMBLPEPT)
	{	/* Look for sequences as if GenPept, an EMBLPEPT Kludge. */
		num_of_na = GetNumNA(segp, GENPEPT, error_msgs);
		num_of_aa = GetNumAA(segp, GENPEPT, error_msgs);
	}
	else
	{
		num_of_na = GetNumNA(segp, format, error_msgs);
		num_of_aa = GetNumAA(segp, format, error_msgs);
	}

	if ((num_of_aa+num_of_na) > 0)
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

		if (show_gi == TRUE)
		{
			btp_na->show_gi = TRUE;
			btp_aa->show_gi = TRUE;
		}
		else
		{
			btp_na->show_gi = FALSE;
			btp_aa->show_gi = FALSE;
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
 
		if (bind_all == TRUE)
		{
			btp_na->gene_binding = ALL;
			btp_aa->gene_binding = ALL;
		}
		else
		{
			btp_na->gene_binding = LIMITED;
			btp_aa->gene_binding = LIMITED;
		}

		btp_na->pseudo = FALSE;
		btp_aa->pseudo = FALSE;

		if (format == GENBANK)
		{
			btp_na->format = GENBANK;
			btp_aa->format = GENBANK;
		}
		else if (format == GENPEPT)
		{
			btp_na->format = GENPEPT;
			btp_aa->format = GENPEPT;
			btp_aa->Spop = Spop;
		}
		else if (format == EMBL)
		{
			btp_na->format = EMBL;
			btp_aa->format = EMBL;
		}
		else if (format == PSEUDOEMBL)
		{
			btp_na->format = EMBL;
			btp_aa->format = EMBL;
			btp_na->pseudo = TRUE;
			btp_aa->pseudo = TRUE;
		}
		else if (format == SELECT)
		{ 
			btp_na->format = GENBANK;
			btp_aa->format = GENBANK;
			btp_na->pseudo = TRUE;
			btp_aa->pseudo = TRUE;
		}
		else if (format == EMBLPEPT)
		{
			btp_na->format = EMBL;
			btp_aa->format = EMBL;
		}
	
		SeqToArrays(segp, btp_na, btp_aa);
        	btp_na->btp_other = btp_aa;
        	btp_aa->btp_other = btp_na;
		GetBCPs(btp_na, btp_aa);
		GetGIs(btp_na, btp_aa);
		GetNARefs(btp_na, btp_aa);
		GetAARefs(btp_na, btp_aa);
		FillSegmentArrays(btp_na, btp_aa);
		SegmentFree(segp);

		if (btp_na->format == GENBANK) 
			pap_size = asn2gb_setup(btp_na, papp);
		else if (btp_na->format == EMBL)
		{
			if (format == EMBL)
				pap_size = asn2embl_setup(btp_na, papp);
			else
				pap_size = asn2ep_setup(btp_aa, papp);
		}
		else if (btp_aa->format == GENPEPT)
			pap_size = asn2gp_setup(btp_aa, papp);
	
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
}	/* asn2ff_setup */

/****************************************************************************
*void set_flags (Boolean show_seq, Boolean show_gi, Uint1 mode, Boolean error_msgs, Boolean bind_all, Uint1 format)
*
*	set_flags to determine which tasks to perform.
*****************************************************************************/
void set_flags (Boolean show_seq, Boolean show_gi, Uint1 mode, Boolean error_msgs, Boolean bind_all, Uint1 format)

{

/* The defines are:
ASN2FF_LOCAL_ID                 asn2ff_flags[0]
	If FALSE then entries with "local" id's are NOT formatted 
ASN2FF_LOOK_FOR_SEQ             asn2ff_flags[1]
	If TRUE BioseqFind is run in an attempt to "find" entries that
	have been loaded into memory and are referenced by an entry 
ASN2FF_VALIDATE_FEATURES        asn2ff_flags[2]
	If TRUE then validation is run on features.  If they are invalid
	they are dropped.
ASN2FF_IGNORE_PATENT_PUBS            asn2ff_flags[3]
	This flag only applies to patent pubs.  If FlatIgnoreThisPatentPub
	is true and this flag is TRUE, that pub is dropped.  ALL OTHER
	PUBS are validated all the time.
ASN2FF_DROP_SHORT_AA            asn2ff_flags[4]
	Drop amino acid sequences that are too short.  Only applies to 
	GenPept (i.e., protein) format  
ASN2FF_AVOID_LOCUS_COLL         asn2ff_flags[5]
	If TRUE Check for LOCUS collisions with Karl's algorithm
	Otherwise Use the LOCUS in the id field.
ASN2FF_DATE_ERROR_MSG		asn2ff_flags[6]
	If TRUE report a missing date.  SHould be FALSE for indexing
	work when no date for a record has been set.
ASN2FF_IUPACAA_ONLY		asn2ff_flags[7]
	Use only iupaca characters if TRUE.  Only iupacaa is the flat
	file standard.
ASN2FF_TRANSL_TABLE		asn2ff_flags[8]
	If TRUE print the transl_table qualifiers.  Set to FALSE until
	the database correctly reflects transl_tables.
ASN2FF_REPORT_LOCUS_COLL     	asn2ff_flags[9]
	If TRUE, report locus collisions via ErrPostEx
ASN2FF_SHOW_ALL_PUBS	        asn2ff_flags[10]
	if TRUE don't drop CitGen reference or replace CitGen->cit with
	"Unpublished"
	
*/

	if (mode == RELEASE_MODE)
	{
		asn2ff_flags[0] = FALSE; 
		asn2ff_flags[1] = FALSE; 
		asn2ff_flags[2] = TRUE; 
		asn2ff_flags[3] = TRUE; 
		asn2ff_flags[4] = TRUE; 
		asn2ff_flags[5] = TRUE; 
		asn2ff_flags[6] = TRUE; 
		asn2ff_flags[7] = TRUE; 
		asn2ff_flags[8] = FALSE; 
		asn2ff_flags[9] = FALSE; 
		asn2ff_flags[10] = FALSE; 
	}
	else if (mode == DIRSUB_MODE)
	{
		asn2ff_flags[0] = FALSE; 
		asn2ff_flags[1] = FALSE; 
		asn2ff_flags[2] = TRUE; 
		asn2ff_flags[3] = TRUE; 
		asn2ff_flags[4] = TRUE; 
		asn2ff_flags[5] = TRUE; 
		asn2ff_flags[6] = FALSE; 
		asn2ff_flags[7] = FALSE; 
		asn2ff_flags[8] = TRUE; 
		asn2ff_flags[9] = FALSE; 
		asn2ff_flags[10] = FALSE; 
	}
	else if (mode == REVISE_MODE)
	{
		asn2ff_flags[0] = TRUE; 
		asn2ff_flags[1] = FALSE; 
		asn2ff_flags[2] = FALSE; 
		asn2ff_flags[3] = FALSE; 
		asn2ff_flags[4] = FALSE; 
		asn2ff_flags[5] = FALSE; 
		asn2ff_flags[6] = TRUE; 
		asn2ff_flags[7] = FALSE; 
		asn2ff_flags[8] = TRUE; 
		asn2ff_flags[9] = TRUE; 
		asn2ff_flags[10] = FALSE; 
	}
	else if (mode == DUMP_MODE)
	{
		asn2ff_flags[0] = TRUE; 
		asn2ff_flags[1] = FALSE; 
		asn2ff_flags[2] = FALSE; 
		asn2ff_flags[3] = FALSE; 
		asn2ff_flags[4] = FALSE; 
		asn2ff_flags[5] = FALSE; 
		asn2ff_flags[6] = TRUE; 
		asn2ff_flags[7] = FALSE; 
		asn2ff_flags[8] = TRUE; 
		asn2ff_flags[9] = FALSE; 
		asn2ff_flags[10] = TRUE; 
	}
	else if (mode == SEQUIN_MODE)
	{
		asn2ff_flags[0] = TRUE; 
		asn2ff_flags[1] = FALSE; 
		asn2ff_flags[2] = FALSE; 
		asn2ff_flags[3] = TRUE; 
		asn2ff_flags[4] = TRUE; 
		asn2ff_flags[5] = TRUE; 
		asn2ff_flags[6] = FALSE; 
		asn2ff_flags[7] = FALSE; 
		asn2ff_flags[8] = TRUE; 
		asn2ff_flags[9] = FALSE; 
		asn2ff_flags[10] = FALSE; 
	}
	else if (mode == CHROMO_MODE)
	{
		asn2ff_flags[0] = TRUE; 
		asn2ff_flags[1] = TRUE; 
		asn2ff_flags[2] = FALSE; 
		asn2ff_flags[3] = TRUE; 
		asn2ff_flags[4] = FALSE; 
		asn2ff_flags[5] = FALSE; 
		asn2ff_flags[6] = FALSE; 
		asn2ff_flags[7] = FALSE; 
		asn2ff_flags[8] = TRUE; 
		asn2ff_flags[9] = FALSE; 
		asn2ff_flags[10] = FALSE; 
	}
}

/***********************************************************************
*void asn2ff_set_output (FILE * fp, BiotablePtr btp_aa, BiotablePtr btp_na, CharPtr line_return)
*
*	This function sets the appropriate fields in the BiotablePtrs
*	for the output file or the line return.
**************************************************************************/

void asn2ff_set_output (FILE *fp, BiotablePtr btp_aa, BiotablePtr btp_na, CharPtr line_return)
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


/*************************************************************************
*asn2gb_setup
*
*	This code setups up the FFPrintArray for GenBank format.
*
**************************************************************************/

Int4 asn2gb_setup(BiotablePtr btp_na, FFPrintArrayPtr PNTR papp)

{
	FFPrintArrayPtr pap;
	Int2 count;
	Int2Ptr num_of_feats, num_of_pubs;
	Int4 index, max, total;
	Int4Ptr num_of_seqblks;

	num_of_feats = (Int2Ptr) MemNew((btp_na->count)*sizeof(Int2));
	num_of_pubs = (Int2Ptr) MemNew((btp_na->count)*sizeof(Int2));
	num_of_seqblks = (Int4Ptr) MemNew((btp_na->count)*sizeof(Int4));
	
	GetLocusParts(btp_na);
	GetNASeqFeat(btp_na);

	total=0;
	for (count=0; count<btp_na->count; count++)
	{
		total += 9;
		if (btp_na->seg_total[count] > 0)
			total++;
		num_of_feats[count] = GetNumOfFeats(btp_na, count);
		total += num_of_feats[count];
		num_of_seqblks[count] = GetNumOfSeqBlks(btp_na, count);
		total += num_of_seqblks[count];
		num_of_pubs[count] = OrganizePubs(btp_na, count);
		total += num_of_pubs[count];

		GetGBDate(btp_na, count);
	}

	*papp = (FFPrintArrayPtr) MemNew((size_t) total*sizeof(FFPrintArray));
	pap = *papp;

/* The next line sets the static Int4, pap_index, in LoadPap	*/ 
	LoadPap(NULL, NULL, NULL, 0, 0, (Uint1)0, (Uint1)0, 0, A2F_OTHER);
	for (count=0; count<btp_na->count; count++)
	{
		LoadPap(pap, PrintLocusLine, btp_na, count, 0, (Uint1)0, (Uint1)0, line_estimate[0], A2F_OTHER);
		flat2asn_delete_locus_user_string();
		flat2asn_install_locus_user_string(btp_na->locus[count]);
		LoadPap(pap, PrintDefinitionLine, btp_na, count, 0, (Uint1)0, (Uint1)0, line_estimate[1], A2F_OTHER);
		LoadPap(pap, PrintAccessLine, btp_na, count, 0, (Uint1)0, (Uint1)0, line_estimate[2], A2F_OTHER);
		flat2asn_delete_accession_user_string();
		flat2asn_install_accession_user_string(btp_na->access[count]);
		LoadPap(pap, PrintKeywordLine, btp_na, count, 0, (Uint1)0, (Uint1)0, line_estimate[3], A2F_OTHER);
		if (btp_na->seg_total[count] > 0)
			LoadPap(pap, PrintSegmentLine, btp_na, count, 0, (Uint1)0, (Uint1)0, line_estimate[0], A2F_OTHER);
		LoadPap(pap, PrintSourceLine, btp_na, count, 0, (Uint1)0, (Uint1)0, line_estimate[4], A2F_OTHER);
		max = (Int4) num_of_pubs[count];
		for (index=0; index<max; index++)
			LoadPap(pap, PrintPubsByNumber, btp_na, count, index, (Uint1)0, (Uint1)0, line_estimate[5], A2F_REFERENCE);
		LoadPap(pap, GBComAndFH, btp_na, count, 0, (Uint1)0, (Uint1)0, line_estimate[6], A2F_OTHER);
		LoadPap(pap, PrintSourceFeat, btp_na, count, 0, (Uint1)0, (Uint1)0, line_estimate[8], A2F_SOURCE_FEATURE);
		max = (Int4) num_of_feats[count];
		for (index=0; index<max; index++)
			LoadPap(pap, PrintNAFeatByNumber, btp_na, count, index, (Uint1)0, (Uint1)0, line_estimate[8], A2F_FEATURE);
		LoadPap(pap, PrintBaseCount, btp_na, count, 0, (Uint1)0, (Uint1)0, line_estimate[0], A2F_OTHER);
		LoadPap(pap, PrintOriginLine, btp_na, count, 0, (Uint1)0, (Uint1)0, line_estimate[0], A2F_OTHER);
		for (index=0; index<num_of_seqblks[count]; index++)
		{
			if (num_of_seqblks[count] == index+1)
				LoadPap(pap, PrintSeqBlk, btp_na, count, index, (Uint1)1, (Uint1)0, line_estimate[9], A2F_OTHER);
			else
				LoadPap(pap, PrintSeqBlk, btp_na, count, index, (Uint1)0, (Uint1)0, line_estimate[9], A2F_OTHER);
		}
	}

	num_of_feats = MemFree(num_of_feats);
	num_of_pubs = MemFree(num_of_pubs);
	num_of_seqblks = MemFree(num_of_seqblks);

	return total;
}	/* asn2gb_setup */

/*************************************************************************
*asn2embl_setup
*
*	This code calls the routines to output an EMBL Flat File
*
**************************************************************************/

Int4 asn2embl_setup(BiotablePtr btp_na, FFPrintArrayPtr PNTR papp)

{
	Boolean PNTR xref_present;
	FFPrintArrayPtr pap;
	Int2 count;
	Int2Ptr num_of_feats, num_of_pubs;
	Int4 index, max, total;
	Int4Ptr num_of_seqblks;

	num_of_feats = (Int2Ptr) MemNew((btp_na->count)*sizeof(Int2));
	num_of_pubs = (Int2Ptr) MemNew((btp_na->count)*sizeof(Int2));
	num_of_seqblks = (Int4Ptr) MemNew((btp_na->count)*sizeof(Int4));
	xref_present = (Boolean PNTR) MemNew((btp_na->count)*sizeof(Boolean));

	GetLocusParts(btp_na);
	GetNASeqFeat(btp_na);

	total=0;
	for (count=0; count<btp_na->count; count++)
	{
		total += 9;
		if (CheckXrefLine(btp_na, count) == TRUE)
		{
			total++;
			xref_present[count] = TRUE;
		}
		else
		{
			xref_present[count] = FALSE;
		}
		num_of_feats[count] = GetNumOfFeats(btp_na, count);
		total += num_of_feats[count];
		num_of_seqblks[count] = GetNumOfSeqBlks(btp_na, count);
		total += num_of_seqblks[count];
		num_of_pubs[count] = OrganizePubs(btp_na, count);
		total += num_of_pubs[count];

		GetEMBLDate(btp_na, count);
		GetVersion(btp_na, count);
	}

	*papp = (FFPrintArrayPtr) MemNew((size_t) total*sizeof(FFPrintArray));
	pap = *papp;

/* The next line sets the static Int4, pap_index, in LoadPap	*/ 
	LoadPap(NULL, NULL, NULL, 0, 0, (Uint1)0, (Uint1)0, 0, A2F_OTHER);
	for (count=0; count<btp_na->count; count++)
	{
		LoadPap(pap, PrintLocusLine, btp_na, count, 0, (Uint1)0, (Uint1)1, line_estimate[0], A2F_OTHER);
		flat2asn_delete_locus_user_string();
		flat2asn_install_locus_user_string(btp_na->locus[count]);
		LoadPap(pap, PrintAccessLine, btp_na, count, 0, (Uint1)0, (Uint1)1, line_estimate[2], A2F_OTHER);
		flat2asn_delete_accession_user_string();
		flat2asn_install_accession_user_string(btp_na->access[count]);
		LoadPap(pap, PrintDateLines, btp_na, count, 0, (Uint1)0, (Uint1)1, line_estimate[10], A2F_OTHER);
		LoadPap(pap, PrintDefinitionLine, btp_na, count, 0, (Uint1)0, (Uint1)1, line_estimate[1], A2F_OTHER);
		LoadPap(pap, PrintKeywordLine, btp_na, count, 0, (Uint1)0, (Uint1)1, line_estimate[3], A2F_OTHER);
		LoadPap(pap, PrintOrganismLine, btp_na, count, 0, (Uint1)0, (Uint1)0, line_estimate[11], A2F_OTHER);
		max = (Int4) num_of_pubs[count];
		for (index=0; index<max; index++)
			LoadPap(pap, PrintPubsByNumber, btp_na, count, index, (Uint1)0, (Uint1)0, line_estimate[5], A2F_REFERENCE);
		if (xref_present[count] == TRUE)
		{
			LoadPap(pap, GBComAndXref, btp_na, count, 0, (Uint1)0, (Uint1)0, line_estimate[7], A2F_OTHER);
			LoadPap(pap, PrintFeatHeader, btp_na, count, 0, (Uint1)0, (Uint1)0, line_estimate[0], A2F_OTHER);
		}
		else
		{
			LoadPap(pap, GBComAndFH, btp_na, count, 0, (Uint1)0, (Uint1)0, line_estimate[6], A2F_OTHER);
		}
		max = (Int4) num_of_feats[count];
		if (max == 0) /* No feats separate source feat from sequence */
			LoadPap(pap, PrintSourceFeat, btp_na, count, 0, (Uint1)0, (Uint1)1, line_estimate[8], A2F_SOURCE_FEATURE);
		else
			LoadPap(pap, PrintSourceFeat, btp_na, count, 0, (Uint1)0, (Uint1)0, line_estimate[8], A2F_SOURCE_FEATURE);
		for (index=0; index<max; index++)
		{
			if (max == index+1)
				LoadPap(pap, PrintNAFeatByNumber, btp_na, count, index, (Uint1)0, (Uint1)1, line_estimate[8], A2F_FEATURE);
			else
				LoadPap(pap, PrintNAFeatByNumber, btp_na, count, index, (Uint1)0, (Uint1)0, line_estimate[8], A2F_FEATURE);
		}
		LoadPap(pap, PrintBaseCount, btp_na, count, 0, (Uint1)0, (Uint1)0, line_estimate[0], A2F_OTHER);
		for (index=0; index<num_of_seqblks[count]; index++)
		{
			if (num_of_seqblks[count] == index+1)
				LoadPap(pap, PrintSeqBlk, btp_na, count, index, (Uint1)1, (Uint1)0, line_estimate[9], A2F_OTHER);
			else
				LoadPap(pap, PrintSeqBlk, btp_na, count, index, (Uint1)0, (Uint1)0, line_estimate[9], A2F_OTHER);
		}
	}

	num_of_feats = MemFree(num_of_feats);
	num_of_pubs = MemFree(num_of_pubs);
	num_of_seqblks = MemFree(num_of_seqblks);
	xref_present = MemFree(xref_present);

	return total;
}	/* asn2embl_setup */

/*************************************************************************
*asn2gp_setup
*
*	This code calls the routines to output a GenPept Flat File
*
**************************************************************************/

Int4 asn2gp_setup(BiotablePtr btp_aa, FFPrintArrayPtr PNTR papp)
{

	BioseqPtr bsp;
	FFPrintArrayPtr pap;
	Int2 count;
	Int2Ptr num_of_feats, num_of_pubs;
	Int4 index, max, total;
	Int4Ptr num_of_seqblks;

	num_of_feats = (Int2Ptr) MemNew((btp_aa->count)*sizeof(Int2));
	num_of_pubs = (Int2Ptr) MemNew((btp_aa->count)*sizeof(Int2));
	num_of_seqblks = (Int4Ptr) MemNew((btp_aa->count)*sizeof(Int4));

	UseGIforLocus(btp_aa);
	GetAASeqFeat(btp_aa);

	total=0;
	for (count=0; count<btp_aa->count; count++)
	{ /* The next lines decide if a GenPept entry should be dropped due to length. */
		bsp = btp_aa->bsp[count];
		if (ASN2FF_DROP_SHORT_AA == TRUE &&
			btp_aa->seg_total[count] == 0 && 
				bsp->length < GENPEPT_MIN)
		{
			flat2asn_delete_accession_user_string();
			flat2asn_delete_locus_user_string();
			flat2asn_install_accession_user_string(btp_aa->access[count]);
			flat2asn_install_locus_user_string(btp_aa->locus[count]);
			if (btp_aa->error_msgs == TRUE)
				ErrPostEx(SEV_INFO, ERR_ENTRY_Partial_peptide, 
					"Entry dropped due to length.");
			continue;
		}
		total += 9;
		if (btp_aa->seg_total[count] > 0)
			total++;
		num_of_feats[count] = GetNumOfFeats(btp_aa, count);
		total += num_of_feats[count];
		num_of_seqblks[count] = GetNumOfSeqBlks(btp_aa, count);
		total += num_of_seqblks[count];
		num_of_pubs[count] = OrganizePubs(btp_aa, count);
		total += num_of_pubs[count];

		GetGPDate(btp_aa, count);
	}

	*papp = (FFPrintArrayPtr) MemNew((size_t) total*sizeof(FFPrintArray));
	pap = *papp;

/* The next line sets the static Int4, pap_index, in LoadPap	*/ 
	LoadPap(NULL, NULL, NULL, 0, 0, (Uint1)0, (Uint1)0, 0, A2F_OTHER);
	for (count=0; count<btp_aa->count; count++)
	{
		bsp = btp_aa->bsp[count];
		if (ASN2FF_DROP_SHORT_AA == TRUE &&
			btp_aa->seg_total[count] == 0 && 
				bsp->length < GENPEPT_MIN)
		{
			continue;
		}
		LoadPap(pap, PrintLocusLine, btp_aa, count, 0, (Uint1)0, (Uint1)0, line_estimate[0], A2F_OTHER);
		flat2asn_delete_locus_user_string();
		flat2asn_install_locus_user_string(btp_aa->locus[count]);
		LoadPap(pap, PrintDefinitionLine, btp_aa, count, 0, (Uint1)0, (Uint1)0, line_estimate[1], A2F_OTHER);
		LoadPap(pap, PrintAccessLine, btp_aa, count, 0, (Uint1)0, (Uint1)0, line_estimate[2], A2F_OTHER);
		flat2asn_delete_accession_user_string();
		flat2asn_install_accession_user_string(btp_aa->access[count]);
		LoadPap(pap, PrintDBSourceLine, btp_aa, count, 0, (Uint1)0, (Uint1)0, line_estimate[12], A2F_OTHER);
		LoadPap(pap, PrintKeywordLine, btp_aa, count, 0, (Uint1)0, (Uint1)0, line_estimate[3], A2F_OTHER);
		if (btp_aa->seg_total[count] > 0)
			LoadPap(pap, PrintSegmentLine, btp_aa, count, 0, (Uint1)0, (Uint1)0, line_estimate[0], A2F_OTHER);
		LoadPap(pap, PrintSourceLine, btp_aa, count, 0, (Uint1)0, (Uint1)0, line_estimate[4], A2F_OTHER);
		max = (Int4) num_of_pubs[count];
		for (index=0; index<max; index++)
			LoadPap(pap, PrintPubsByNumber, btp_aa, count, index, (Uint1)0, (Uint1)0, line_estimate[5], A2F_REFERENCE);
		LoadPap(pap, GPComAndFH, btp_aa, count, 0, (Uint1)0, (Uint1)0, line_estimate[6], A2F_OTHER);
		LoadPap(pap, PrintSourceFeat, btp_aa, count, 0, (Uint1)0, (Uint1)0, line_estimate[8], A2F_SOURCE_FEATURE);
		max = (Int4) num_of_feats[count];
		for (index=0; index<max; index++)
			LoadPap(pap, PrintAAFeatByNumber, btp_aa, count, index, (Uint1)0, (Uint1)0, line_estimate[8], A2F_FEATURE);
		LoadPap(pap, PrintOriginLine, btp_aa, count, 0, (Uint1)0, (Uint1)0, line_estimate[0], A2F_OTHER);
		for (index=0; index<num_of_seqblks[count]; index++)
		{
			if (num_of_seqblks[count] == index+1)
				LoadPap(pap, PrintSeqBlk, btp_aa, count, index, (Uint1)1, (Uint1)0, line_estimate[9], A2F_OTHER);
			else
				LoadPap(pap, PrintSeqBlk, btp_aa, count, index, (Uint1)0, (Uint1)0, line_estimate[9], A2F_OTHER);
		}
	}

	num_of_feats = MemFree(num_of_feats);
	num_of_pubs = MemFree(num_of_pubs);
	num_of_seqblks = MemFree(num_of_seqblks);

	return total;
}	/* asn2gp_setup */

/*************************************************************************
*asn2ep_setup
*
*	This code calls the routines to output an "EMBLPept" Flat File
*
**************************************************************************/

Int4 asn2ep_setup(BiotablePtr btp_aa, FFPrintArrayPtr PNTR papp)
{

	BioseqPtr bsp;
	FFPrintArrayPtr pap;
	Int2 count;
	Int2Ptr num_of_feats, num_of_pubs;
	Int4 index, max, total;
	Int4Ptr num_of_seqblks;

	num_of_feats = (Int2Ptr) MemNew((btp_aa->count)*sizeof(Int2));
	num_of_pubs = (Int2Ptr) MemNew((btp_aa->count)*sizeof(Int2));
	num_of_seqblks = (Int4Ptr) MemNew((btp_aa->count)*sizeof(Int4));

	btp_aa->format = GENPEPT;
	GetLocusParts(btp_aa);
	GetAASeqFeat(btp_aa);
	btp_aa->format = EMBL;

	total=0;
	for (count=0; count<btp_aa->count; count++)
	{ /* The next lines decide if a GenPept entry should be dropped due to length. */
		bsp = btp_aa->bsp[count];
		if (ASN2FF_DROP_SHORT_AA == TRUE &&
			btp_aa->seg_total[count] == 0 && 
				bsp->length < GENPEPT_MIN)
		{
			flat2asn_delete_accession_user_string();
			flat2asn_delete_locus_user_string();
			flat2asn_install_accession_user_string(btp_aa->access[count]);
			flat2asn_install_locus_user_string(btp_aa->locus[count]);
			if (btp_aa->error_msgs == TRUE)
				ErrPostEx(SEV_INFO, ERR_ENTRY_Partial_peptide, 
					"Entry dropped due to length.");
			continue;
		}
		total += 8;
		if (btp_aa->seg_total[count] > 0)
			total++;
		num_of_pubs[count] = OrganizePubs(btp_aa, count);
		total += num_of_pubs[count];

		num_of_feats[count] = GetNumOfFeats(btp_aa, count);
		total += num_of_feats[count];
		num_of_seqblks[count] = GetNumOfSeqBlks(btp_aa, count);
		total += num_of_seqblks[count];

		GetEMBLDate(btp_aa, count);
		GetVersion(btp_aa, count);
	}

	*papp = (FFPrintArrayPtr) MemNew((size_t) total*sizeof(FFPrintArray));
	pap = *papp;

/* The next line sets the static Int4, pap_index, in LoadPap	*/ 
	LoadPap(NULL, NULL, NULL, 0, 0, (Uint1)0, (Uint1)0, 0, A2F_OTHER);
	for (count=0; count<btp_aa->count; count++)
	{
		bsp = btp_aa->bsp[count];
		if (ASN2FF_DROP_SHORT_AA == TRUE &&
			btp_aa->seg_total[count] == 0 && 
				bsp->length < GENPEPT_MIN)
		{
			continue;
		}

		LoadPap(pap, PrintEPLocusLine, btp_aa, count, 0, (Uint1)0, (Uint1)1, line_estimate[0], A2F_OTHER);
		flat2asn_delete_locus_user_string();
		flat2asn_install_locus_user_string(btp_aa->locus[count]);
		LoadPap(pap, PrintAccessLine, btp_aa, count, 0, (Uint1)0, (Uint1)1, line_estimate[2], A2F_OTHER);
		flat2asn_delete_accession_user_string();
		flat2asn_install_accession_user_string(btp_aa->access[count]);
		LoadPap(pap, PrintDateLines, btp_aa, count, 0, (Uint1)0, (Uint1)1, line_estimate[10], A2F_OTHER);
		LoadPap(pap, PrintDefinitionLine, btp_aa, count, 0, (Uint1)0, (Uint1)1, line_estimate[1], A2F_OTHER);
		LoadPap(pap, PrintKeywordLine, btp_aa, count, 0, (Uint1)0, (Uint1)1, line_estimate[3], A2F_OTHER);
		LoadPap(pap, PrintOrganismLine, btp_aa, count, 0, (Uint1)0, (Uint1)0, line_estimate[11], A2F_OTHER);
		max = (Int4) num_of_pubs[count];
		for (index=0; index<max; index++)
			LoadPap(pap, PrintPubsByNumber, btp_aa, count, index, (Uint1)0, (Uint1)0, line_estimate[5], A2F_REFERENCE);
		LoadPap(pap, GBComAndFH, btp_aa, count, 0, (Uint1)0, (Uint1)0, line_estimate[6], A2F_OTHER);
		LoadPap(pap, PrintSourceFeat, btp_aa, count, 0, (Uint1)0, (Uint1)0, line_estimate[8], A2F_SOURCE_FEATURE);
		max = (Int4) num_of_feats[count];
		for (index=0; index<max; index++)
			LoadPap(pap, PrintAAFeatByNumber, btp_aa, count, index, (Uint1)0, (Uint1)0, line_estimate[8], A2F_FEATURE);
		for (index=0; index<num_of_seqblks[count]; index++)
		{
			if (num_of_seqblks[count] == index+1)
				LoadPap(pap, PrintEPSeqBlk, btp_aa, count, index, (Uint1)1, (Uint1)0, line_estimate[9], A2F_OTHER);
			else
				LoadPap(pap, PrintEPSeqBlk, btp_aa, count, index, (Uint1)0, (Uint1)0, line_estimate[9], A2F_OTHER);
		}
	}

	num_of_feats = MemFree(num_of_feats);
	num_of_pubs = MemFree(num_of_pubs);
	num_of_seqblks = MemFree(num_of_seqblks);

	return total;
}	/* asn2ep_setup */

void asn2ff_cleanup (BiotablePtr btp_aa, BiotablePtr btp_na, FFPrintArrayPtr pap)

{
	if (btp_aa)
		BiotableFree(btp_aa);
	if (btp_na)
		BiotableFree(btp_na);
	if (pap)
		MemFree(pap);

/* Delete these strings so they don't interfere with others */
        flat2asn_delete_locus_user_string();
        flat2asn_delete_accession_user_string();

	return;
}


/*****************************************************************************
*void LoadPap(FFPrintArrayPtr pap, FFPapFct fct, BiotablePtr btp, Int2 count, Int4 index, Uint1 last, Uint1 printxx, Uint1 element_type) 
*
*	This function places the parameters in the correct spaces in the
*	FFPrintArrayPtr.
*
****************************************************************************/

void LoadPap(FFPrintArrayPtr pap, FFPapFct fct, BiotablePtr btp, Int2 count, Int4 index, Uint1 last, Uint1 printxx, Int2 estimate, Uint1 element_type) 

{
	static Int4 pap_index;

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
		if (element_type == A2F_SOURCE_FEATURE)
		{
			if (btp->sfpSourcesize[count] != 0)
				pap[pap_index].sfp = btp->sfpSourcelist[count][0];
		}
		else if (element_type == A2F_FEATURE)
			GetPapSeqFeatPtr (btp, count, index, pap_index, pap);
		else if (element_type == A2F_REFERENCE)
			GetPapRefPtr (btp, count, index, pap_index, pap);
		pap_index++;
	}

	return;
}

/**************************************************************************
*void GetPapSeqFeatPtr (BiotablePtr btp, Int2 count, Int4 ext_index, Int4 pap_index, FFPrintArrayPtr pap)
*
*	Find the SeqFeatPtr that is associated with this entry in the 
*	FFPrintArrayPtr.
*************************************************************************/

void GetPapSeqFeatPtr (BiotablePtr btp, Int2 count, Int4 ext_index, Int4 pap_index, FFPrintArrayPtr pap)

{
	Boolean found_orphan=FALSE;
	GeneStructPtr gsp;
	Int2 feat_index, genesize, index, listsize=btp->sfpListsize[count], orphan_index;
	SeqFeatPtr sfp=NULL;

	index = (Int2) ext_index;

	feat_index = index - listsize;
	if (feat_index >= 0)
	{
		gsp=btp->gspp[count][0];
		genesize=btp->sfpGenesize[count];
		for (orphan_index=0; orphan_index<genesize; orphan_index++)
			if (gsp->genes_matched[orphan_index] == -1)
			{
				if (feat_index == 0)
				{
                           	 	found_orphan = TRUE;
                             	  	break;
                       		}
                       		feat_index--;
                	}
		if (found_orphan == TRUE)
			sfp = btp->sfpGenelist[count][orphan_index];
	}
	else
	{
		sfp = btp->sfpList[count][index];
	}

	pap[pap_index].sfp = sfp;

	return;
}

/**************************************************************************
*void GetPapRefPtr (BiotablePtr btp, Int2 count, Int4 ext_index, Int4 pap_index, FFPrintArrayPtr pap)
*
*	Get the ValNodePtr associated with a certain reference.
**************************************************************************/

void GetPapRefPtr (BiotablePtr btp, Int2 count, Int4 ext_index, Int4 pap_index, FFPrintArrayPtr pap)

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
/* The next two lines are a KLUDGE for GenPept (see StoreNAPubCit comment) */
			if (btp->format == GENPEPT && psp->sfp)
				pap[pap_index].sfp = psp->sfp;
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
*	The following three functions (GBComAndFH, GPComAndFH, and 
*	GBComAndXref) call either DoGBComment or GPComment, and
*	another function.  This is because the comment block is 
*	optional, which is a problem when the program checks at the
*	beginning as to the number of calls necessary.  Rather than 
*	checking whether it exists, these functions print the comment 
*	and another block out at once. 
************************************************************************/

void GBComAndFH (BiotablePtr btp, Int2 count)

{
	DoGBComment(btp, count);
	if (btp->format == EMBL)
		PrintXX(btp);
	PrintFeatHeader(btp, count);

	return;
}

void GPComAndFH (BiotablePtr btp, Int2 count)

{
	DoGPComment(btp, count);
	PrintFeatHeader(btp, count);

	return;
}

void GBComAndXref (BiotablePtr btp, Int2 count)

{
	DoGBComment(btp, count);
	if (btp->format == EMBL)
		PrintXX(btp);
	PrintXrefLine(btp, count);

	return;
}

/**************************************************************************
*void PrintTerminator (BiotablePtr btp)
*
*	Prints the double slash (//) at the end of an entry.
**************************************************************************/

void PrintTerminator (BiotablePtr btp)

{
	StartPrint(btp, 0, 0, 0, NULL);
	AddChar(btp, '/');
	AddChar(btp, '/');
	EndPrint(btp);
}

/**************************************************************************
*GetNumAA
*	GetNumAA to get the number of amino acid sequence entries.
*
**************************************************************************/

Int2 GetNumAA (SegmentPtr segp, Uint1 format, Boolean error_msgs)

{
	BioseqPtr bsp;
	Int2 index, count=0;

	for (index=0; index<segp->count; index++)
	{
		bsp = segp->bsp[index];
		if (ISA_aa(bsp->mol) &&
		   (bsp->repr == Seq_repr_raw || bsp->repr == Seq_repr_const))
		{
			count++;
		}
	}

	return count;
}	/* GetNumAA */

/**************************************************************************
*GetNumNA
*	GetNumNA to get the number of nucleic acid sequence entries.
*
**************************************************************************/

Int2 GetNumNA (SegmentPtr segp, Uint1 format, Boolean error_msgs)

{
	BioseqPtr bsp;
	Int2 index, count=0;
	SeqIdPtr isip, sip;

	for (index=0; index<segp->count; index++)
	{
		bsp = segp->bsp[index];
		if (ISA_na(bsp->mol) &&
		   (bsp->repr == Seq_repr_raw || bsp->repr == Seq_repr_const))
		{
			if (ASN2FF_LOCAL_ID == FALSE)
			{
				isip = bsp->id;
				sip = SeqIdSelect(isip, fasta_order, NUM_ORDER);
				if (sip->choice != SEQID_LOCAL)
				{
					segp->quality[index] = RELEASE;
					count++;
				}
				else if (sip->choice == SEQID_LOCAL && 
					format == GENPEPT)
				{
					segp->quality[index] = RELEASE;
					count++;
				}
			}
			else
			{
				count++;
			}
		} 
	}

	return count;
}	/* GetNumNA */

/**************************************************************************
*
*	SeqToArrays to place the raw BioseqPtr's in arrays.
*
**************************************************************************/

static void SeqToArrays (SegmentPtr segp, BiotablePtr btp_na, BiotablePtr btp_aa)

{
	BioseqPtr bsp;
	SeqIdPtr isip, sip;
	Int2 index, count_aa=0, count_na=0;
	ValNodePtr vnp=NULL;

	for (index=0; index<segp->count; index++)
	{
		bsp = segp->bsp[index];
		if(bsp->repr == Seq_repr_raw || bsp->repr == Seq_repr_const)
			if (ISA_na(bsp->mol))
			{
				if (ASN2FF_LOCAL_ID == FALSE)
				{
					isip = bsp->id;
					sip = SeqIdSelect(isip, fasta_order, NUM_ORDER);
					if (sip->choice != SEQID_LOCAL ||
						btp_na->format == GENPEPT)
					{
						if (segp->quality[index] == RELEASE)
                                                {
							btp_na->bsp[count_na] = bsp;
							count_na++;
						}
					}
				}
				else
				{
					btp_na->bsp[count_na] = bsp;
					count_na++;
				}
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
			FindRawSeq(bsp, btp_na, btp_aa);
	}

}

/**************************************************************************
*GetAARefs
*	GetAARefs to get the amino acid refs.
*
*NOTE: Pubs are only collected and sorted if they will be used (i.e.,
*GenPept format).  This is due to the enormous amount of time that is
*used to collect Pubs from references on some entries.
**************************************************************************/

static void GetAARefs (BiotablePtr btp_na, BiotablePtr btp_aa)

{
	BioseqContextPtr bcp;
	BioseqPtr bsp;
	Boolean nucl_db=FALSE;
	Boolean error_msgs=btp_na->error_msgs;
	Char buffer[30];
	Int2 index, status;
	Uint1 format=btp_na->format;
	ValNodePtr vnp;

	if (format == GENPEPT)
	{
		for (index=0; index<btp_na->count; index++)
		{
			bsp = btp_na->bsp[index];
			if (bsp->repr == Seq_repr_raw || 
				bsp->repr == Seq_repr_const)
				if (OneOfThree(bsp) == TRUE)	
				{
					nucl_db = TRUE;
					break;
				}
		}
	}

	for (index=0; index<btp_aa->count; index++)
	{
		vnp=NULL;
		bsp = btp_aa->bsp[index];
		if (bsp->repr == Seq_repr_raw || bsp->repr == Seq_repr_const)
		{
			if (format == GENPEPT)
			{
				bcp = btp_aa->bcp[index];
				if (nucl_db == TRUE)
					EstablishProteinLink(btp_na, btp_aa, index);
				status=StorePubInfo(bcp, bsp, &vnp, error_msgs);
				if (status <= 0 && nucl_db == TRUE)
					status = StoreNAPubInfo(btp_aa, index, &vnp, error_msgs);
				if (status <= 0)
				{
					if (error_msgs == TRUE)
					{
						MakeAnAccession(buffer, bsp);
						flat2asn_delete_locus_user_string();
						flat2asn_install_locus_user_string(buffer);
						flat2asn_delete_accession_user_string();
						flat2asn_install_accession_user_string(buffer);
						ErrPostEx(SEV_ERROR, ERR_REFERENCE_NoValidRefs, 
						"No refs found that would result in legal flatfile format");
					}
					if (status == 0)
						btp_aa->Pub[index] = vnp;
				}
				else
					btp_aa->Pub[index] = vnp;
			}
		}
	}

	return;
}	/* GetAARefs */

/**************************************************************************
*GetNARefs
*	Get the references for the Nucleic acids.
*
**************************************************************************/

static void GetNARefs (BiotablePtr btp_na, BiotablePtr btp_aa)

{
	BioseqContextPtr bcp;
	BioseqPtr bsp;
	Boolean error_msgs=btp_na->error_msgs;
	Char buffer[30];
	Int2 index, status;
	Uint1 format=btp_na->format;
	ValNodePtr vnp;

	for (index=0; index<btp_na->count; index++)
	{
		vnp=NULL;
		bsp = btp_na->bsp[index];
		bcp = btp_na->bcp[index];
		if ((status=StorePubInfo(bcp, bsp, &vnp, error_msgs)) <= 0)
		{
			if (error_msgs == TRUE)
			{
				MakeAnAccession(buffer, bsp);
				flat2asn_delete_locus_user_string();
				flat2asn_install_locus_user_string(buffer);
				flat2asn_delete_accession_user_string();
				flat2asn_install_accession_user_string(buffer);
				ErrPostEx(SEV_ERROR, ERR_REFERENCE_NoValidRefs, 
				"No refs found that would result in legal flatfile format");
			}
			/* found something. */
			if (status == 0)
				btp_na->Pub[index] = vnp;
		}
		else
			btp_na->Pub[index] = vnp;
	}

	return;
}	/* GetNARefs */

/************************************************************************
*
*	GetSegNum()
*		SeqEntryExplore function to make a list of all segmented
*		bioseqs in a seqentry.
*
*************************************************************************/

void GetSegNum (SeqEntryPtr sep, Pointer data, Int4 index, Int2 indent)

{
	SegmentPtr segp;
	BioseqPtr bsp;
	BioseqSetPtr bssp;
	SeqLocPtr slp;

	if (IS_Bioseq(sep))
	{
		bsp = (BioseqPtr) sep->data.ptrvalue;
		if (ASN2FF_LOOK_FOR_SEQ == FALSE)
		{
			segp = (SegmentPtr) data;
			segp->bsp[segp->count] = bsp;
			segp->count++;
		}
		else
		{
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
						if (CompareToSegpList(bsp, segp) == FALSE && bsp != NULL)
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
				if (CompareToSegpList(bsp, segp) == FALSE && bsp != NULL)
				{
					segp->bsp[segp->count] = bsp;
					segp->count++;
				}
			}
		}
	}
	else
	{
		bssp = (BioseqSetPtr) sep->data.ptrvalue;
	}


	return;

}	/* GetSegNum */

/**************************************************************************
* static Boolean CompareToSegpList (BioseqPtr bsp, SegmentPtr segp)
*
*	Compare the bsp with the bsp's already in segp.
**************************************************************************/
static Boolean CompareToSegpList (BioseqPtr bsp, SegmentPtr segp)

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

/***************************************************************************
*
*	GetLocusParts populates the locus and accession fields in
*	the BiotablePtr.
*
***************************************************************************/

static void GetLocusParts (BiotablePtr btp)
{
	BioseqPtr bsp;
	SeqIdPtr sip, isip;
	Int2 count, num_seg=0, total_segs=0;
	TextSeqIdPtr tsip;
	Char buffer[4], buf_a[MAX_ACCESSION_LEN+1], buf_l[20], base_l[20];
	CharPtr buf_acc=buf_a, buf_locus=buf_l, base_locus=base_l, base_a;

	base_a = MakeBaseAccession(btp);

	for (count=0; count<btp->count; count++)
	{
		buffer[0] = '\0';
		GetDivision(buffer, count, btp);
		if (buffer[0] != NULLB)
			btp->division[count] = StringSave(buffer);
		num_seg = btp->seg_num[count]; 
		total_segs = btp->seg_total[count]; 
		if (num_seg == 1)
			base_locus = MakeBaseLocus(base_locus, btp, count);

		bsp = btp->bsp[count];
		isip = bsp->id;
		sip = SeqIdSelect(isip, fasta_order, NUM_ORDER);
		tsip = (TextSeqIdPtr) sip->data.ptrvalue;
		switch (sip->choice)
		{		/* These three should be easy.		*/
		    case SEQID_GENBANK:
	    	    case SEQID_EMBL:
	    	    case SEQID_DDBJ:
			if ((ValidateAccession(buf_acc, tsip->accession)) < 0)
			{
				if (base_a != NULL)
					StringNCpy(buf_acc, base_a, MAX_ACCESSION_LEN);
				else
					buf_acc = MakeAnAccession(buf_acc, btp->bsp[count]);
			}
	/*  Should a warning be issued if Accession is made??????*/
			buf_locus = ValidateLocus(btp, count, base_locus, total_segs, num_seg, buf_locus, tsip->name, buf_acc); 
			btp->access[count] = StringNCpy(btp->access[count], buf_acc, MAX_ACCESSION_LEN);
			sprintf(btp->locus[count], "%-10s", buf_locus); 
			num_seg--;
			break;
		    case SEQID_LOCAL:
			if ((((ObjectIdPtr)sip->data.ptrvalue)->str) == NULL)
			{
				buf_acc[0] = 'X';
				sprintf(buf_acc+1, "%ld", 
					(long)((ObjectIdPtr)sip->data.ptrvalue)->id);
			}
			else 
				buf_acc = StringNCpy(buf_acc, ((ObjectIdPtr)sip->data.ptrvalue)->str, MAX_ACCESSION_LEN);
			buf_locus = ValidateLocus(btp, count, base_locus, total_segs, num_seg,buf_locus,  buf_acc, buf_acc); 
			btp->access[count] = StringNCpy(btp->access[count], buf_acc, MAX_ACCESSION_LEN);
			sprintf(btp->locus[count], "%-10s", buf_locus); 
			num_seg--;
			break;

		   case SEQID_GI:
			sprintf(buf_acc, "%ld", (long) (sip->data.intvalue));
			buf_locus = ValidateLocus(btp, count, base_locus, total_segs, num_seg, buf_locus, buf_acc, buf_acc); 
			btp->access[count] = StringNCpy(btp->access[count], buf_acc, MAX_ACCESSION_LEN);
			sprintf(btp->locus[count], "%-10s", buf_locus); 
			num_seg--;
			break;

		   case SEQID_PIR:
		   case SEQID_SWISSPROT:
			if ((ValidateAccession(buf_acc, tsip->accession)) < 0)
			{
				if (base_a != NULL)
					StringNCpy(buf_acc, base_a, MAX_ACCESSION_LEN);
				else
					buf_acc = MakeAnAccession(buf_acc, btp->bsp[count]);
			}
	/*  Should a warning be issued if Accession is made??????*/
			buf_locus = ValidateLocus(btp, count, base_locus, total_segs, num_seg, buf_locus, tsip->name, buf_acc); 
			btp->access[count] = StringNCpy(btp->access[count], buf_acc, MAX_ACCESSION_LEN);
			sprintf(btp->locus[count], "%-10s", buf_locus); 
			num_seg--;
			
			break;
		   default:
			buf_acc = MakeAnAccession(buf_acc, btp->bsp[count]);
			buf_locus = ValidateLocus(btp, count, base_locus, total_segs, num_seg, buf_locus, buf_acc, buf_acc); 
			btp->access[count] = StringNCpy(btp->access[count], buf_acc, MAX_ACCESSION_LEN);
			sprintf(btp->locus[count], "%-10s", buf_locus); 
			num_seg--;
			break;
		}
	}
	if (base_a != NULL)
		base_a = MemFree(base_a);
	
}
/*******************************************************************
*Boolean OneOfThree (BioseqPtr bsp)
*
*	This function checks if the database is one of the three 
*	nucleotide databases.  If so, it's permissible (as a KLUDGE)
*	to use the na info for aa.
*
*	This function should actually be called OneOfThreePlusOne
*	as it looks for the three nucleotide databases as well as
*	"LOCAL" entries.
******************************************************************/

Boolean OneOfThree (BioseqPtr bsp)

{
	Boolean retval=FALSE;
	SeqIdPtr sip, isip;

	if (bsp != NULL)
	{
		isip = bsp->id;
		sip = SeqIdSelect(isip, fasta_order, NUM_ORDER);
		if (sip->choice == SEQID_GENBANK || sip->choice == SEQID_EMBL ||
			sip->choice == SEQID_DDBJ || sip->choice == SEQID_LOCAL)
			retval = TRUE;

	}
	return retval;
}

/***************************************************************************
*
*	UseGIforLocus to get the GI number for the locus and accession numbers.
*
***************************************************************************/

static void UseGIforLocus (BiotablePtr btp)
{
	BiotablePtr btp_other;
	Char buffer[4];
	Int2 count;

	for (count=0; count<btp->count; count++)
	{
		if (btp->gi[count] != -1)
		{	
			sprintf(btp->access[count], "%ld", (long) (btp->gi[count]));
			sprintf(btp->locus[count], "%-10ld", (long) (btp->gi[count]));
		}
		else
		{
			MemSet((VoidPtr) btp->access[count], ' ', 10);
			MemSet((VoidPtr) btp->locus[count], ' ', 10);
		}
		buffer[0] = '\0';
		GetDivision(buffer, count, btp);
		btp_other = btp->btp_other;
		if (buffer[0] == NULLB)
		{
			if (OneOfThree(btp_other->bsp[0]) == TRUE)
			{
				GetDivision(buffer, 0, btp->btp_other);
				if (buffer != '\0')
					btp->division[count] = StringSave(buffer);
			}
			else
				btp->division[count] = StringSave("   ");
		}
		else
			btp->division[count] = StringSave(buffer);
	}
}

/**************************************************************************
*GetBaseAccession
*
*	GetBaseAccession takes a BiotablePtr and returns an 
*	accession if 1.) the set is segmented, and 2.) there is
*	an accession at a higher level.  Otherwise NULL is returned.
*	The user should deallocate the CharPtr.
**************************************************************************/

CharPtr MakeBaseAccession (BiotablePtr btp)

{
	BioseqPtr bsp;
	Char buffer[MAX_ACCESSION_LEN+1];
	CharPtr return_buf, buf_acc=buffer;
	Int2 status = -1;
	SeqIdPtr sip, isip;
	TextSeqIdPtr tsip;


	if (btp->table[0] == NULL)
		return NULL;
	bsp = btp->table[0];
	isip = bsp->id;
	sip = SeqIdSelect(isip, fasta_order, NUM_ORDER);
	if (sip && ((sip->choice == SEQID_GENBANK) || 
		(sip->choice == SEQID_EMBL) || 
		(sip->choice == SEQID_PIR) || 
		(sip->choice == SEQID_SWISSPROT) || 
		(sip->choice == SEQID_DDBJ) || 
		(sip->choice == SEQID_PRF)))
	{
		tsip = (TextSeqIdPtr) sip->data.ptrvalue;
		switch (sip->choice)
		{		/* These three should be easy.		*/
			case SEQID_GENBANK:
			case SEQID_EMBL:
			case SEQID_DDBJ:
			case SEQID_PIR:
			case SEQID_SWISSPROT:
				status = ValidateAccession(buf_acc, tsip->accession);
		}
	}
	else 
		status = -1;

	if (status < 0)
		return NULL;
	else
	{
		return_buf = StringSave(buf_acc);
		return return_buf;
	}
}


/***************************************************************************
*
*	MakeBaseLocus takes a BiotablePtr and a CharPtr (base_locus)
*	and returns a CharPtr which is the new base_locus.  Checking is 
*	done to assure suitability of the new base locus name (i.e., 
*	no more than 9 characters for less than 10 segments and no more
*	than 8 characters for 10 or more segments).
*
***************************************************************************/

CharPtr MakeBaseLocus (CharPtr base_locus, BiotablePtr btp, Int2 count)

{
	BioseqPtr bsp, bbsp;
	Int2  index, local_count, length, base_locus_max, name_len, num_of_digits, num_seg;
	SeqIdPtr sip, bsip=NULL, isip=NULL, bisip=NULL;
	TextSeqIdPtr tsip, btsip=NULL;
	Char buffer[21], temp_buf[21];
	CharPtr localbuf=buffer, name, ptr=temp_buf;
	CharPtr tmp = "SEG_";

	base_locus[0] = '\0'; 

			/* How many segments (num_seg) in this segmented set? */
	num_seg = btp->seg_total[count];
	if (num_seg < 10)
		num_of_digits = 1;
	else if (num_seg < 100)
		num_of_digits = 2;
	else if (num_seg < 1000)
		num_of_digits = 3;
	else 
	{
		ErrPostEx(SEV_INFO, ERR_SEGMENT_MoreThan1000Segs, "");
	}
	base_locus_max = MAX_LOCUS_NAME_LEN - num_of_digits;
			/* Is there already a base locus?*/
	bbsp = btp->table[count];
	bisip = bbsp->id;
	bsip = SeqIdSelect(bisip, fasta_order, NUM_ORDER);
	if (bsip)
		btsip = (TextSeqIdPtr) bsip->data.ptrvalue;
	if (btsip && bsip && (bsip->choice == SEQID_GENBANK || 
			bsip->choice == SEQID_EMBL ||
			bsip->choice == SEQID_DDBJ ||
			bsip->choice == SEQID_SWISSPROT ||
			bsip->choice == SEQID_PIR ) &&
			StringLen(btsip->name) > 0)
	{
		localbuf = StringCpy(localbuf, btsip->name);
		if (StringNCmp(localbuf, tmp, 4) == 0)
		{
		/* check if name starts with "SEG_", remove if it does. */
		    StringCpy(ptr, localbuf+4);
		    length = StringLen(ptr);
		    ptr[length] = '\0';
		    if (ptr[length-1] == '1')
		    {
		       bsp = btp->bsp[count];
		       isip = bsp->id;
		       sip = SeqIdSelect(isip, fasta_order, NUM_ORDER);
		       if (sip && (name=((TextSeqIdPtr)sip->data.ptrvalue)->name) != NULL)
		       {
		          name_len = StringLen(name);
		          if (name_len == length)
		          {
		             if (name[length-1] == ptr[length-1])
		    	       for (index=2; index >= num_of_digits; index++)
		               {/* The following is *really* '0'! */
		                  if (ptr[length-index] == '0')
		                  {
		                     if (ptr[length-index] == name[length-index])
		                     {
		                        StringNCpy(base_locus, ptr, length-index);
		                        base_locus[length-index] = '\0';
		                     }
		                     else
		                     {
		                        StringNCpy(base_locus, ptr, length-index+1);
		                        base_locus[length-index+1] = '\0';
		                     }
		                  }
		                  else
		                  {
		                     StringNCpy(base_locus, ptr, length-index+1);
		                     base_locus[length-index+1] = '\0';
				     break;
		                  }
		               }
		            }
		         }
		    } 
		    /* If nothing else worked, use base locus anyway. */
		    if (base_locus[0] == '\0')
		    	StringCpy(base_locus, ptr);
		}
		if (base_locus[0] == '\0')
			StringCpy(base_locus, btsip->name);

		/*check for length, truncate if necessary.	*/
		base_locus = CheckLocusLength (btp, base_locus, base_locus_max, num_seg);
		return base_locus;
	}

			/* Look for one sensible locus in all segments. */
	local_count = count;
	while (btp->table[local_count] != NULL)
	{
		bsp = btp->bsp[local_count];
		isip = bsp->id;
		sip = SeqIdSelect(isip, fasta_order, NUM_ORDER);
		if (sip)
			tsip = (TextSeqIdPtr) sip->data.ptrvalue;
		if (tsip && sip && (sip->choice == SEQID_GENBANK || 
				sip->choice == SEQID_EMBL ||
				sip->choice == SEQID_DDBJ ||
				sip->choice == SEQID_SWISSPROT ||
				sip->choice == SEQID_PIR ) &&
				StringLen(tsip->name) > 0)
		{
		    if (tsip->name)
		    {
		        base_locus = StringCpy(base_locus, tsip->name);
			length = StringLen(base_locus);
			base_locus[length-num_of_digits] = '\0';
			base_locus = CheckLocusLength (btp, base_locus, base_locus_max, num_seg);
			return base_locus;
		    }
		}
		local_count++;
	}

		/* No option left but to take the first locus name.*/
	bsp = btp->bsp[count];
	isip = bsp->id;
	sip = SeqIdSelect(isip, fasta_order, NUM_ORDER);
	if (sip && (((TextSeqIdPtr)sip->data.ptrvalue)->name))
		base_locus = StringCpy(base_locus, ((TextSeqIdPtr)sip->data.ptrvalue)->name);

	else if ((((ObjectIdPtr)sip->data.ptrvalue)->str) == NULL)
		sprintf(base_locus, "%ld", (long)((ObjectIdPtr)sip->data.ptrvalue)->id);
	else 
		base_locus = StringCpy(base_locus, ((ObjectIdPtr)sip->data.ptrvalue)->str);
	base_locus = CheckLocusLength (btp, base_locus, base_locus_max, num_seg);
	return base_locus;
	
}	/* MakeBaseLocus */	

/****************************************************************************
*
*	ValidateAccession takes an accession number and makes sure it is
*	in the proper format (starts with a capital letter that is followed
*	by five numbers).
*
*	Return values are:
*	 0: no problem
*	-1: Accession did not start with a letter
*	-2: Accession did not contain five numbers
*	-3: the original Accession number to be validated was NULL
*
****************************************************************************/

Int2 ValidateAccession(CharPtr new_buf, CharPtr orig_buf)

{
	Int2 count;
	Boolean FirstCheck=FALSE, SecondCheck=FALSE;

	if (orig_buf == NULL || orig_buf[0] == '\0')
		return -3;
	
	if (orig_buf[0] < 'A' || orig_buf[0] > 'Z')
		return -1;
	else
		FirstCheck = TRUE;

	for (count=1; count<5; count++)
	{
		if(! IS_DIGIT(orig_buf[count]))
			break;
		else if (count == 4)
			SecondCheck = TRUE;
	}

	if (FirstCheck == TRUE && SecondCheck == TRUE)
	{
		StringCpy(new_buf, orig_buf);
		return 0;
	}
	else
		return -2;
}

/**************************************************************************
*	CharPtr GetExtraAccessions(BiotablePtr btp, Int2 count)
*
*	Looks in the descriptor for any extra-accessions.
**************************************************************************/

CharPtr GetExtraAccessions(BiotablePtr btp, Int2 count)

{
	BioseqPtr bsp;
	Char buffer[10];
	CharPtr ptr=buffer, ptr1=NULL, ptr2;
	EMBLBlockPtr eb;
	GBBlockPtr gb;
	Int2 index, status;
	SeqFeatPtr sfp;
	SeqIdPtr xid;
	SeqIntPtr si;
	SeqLocPtr xref;
	TextSeqIdPtr text;
	ValNodePtr extra_access=NULL, location=NULL, vnp;

	if ((vnp=BioseqContextGetSeqDescr(btp->bcp[count], Seq_descr_genbank, NULL, NULL)) != NULL) 
	{
		gb = (GBBlockPtr) vnp->data.ptrvalue;
		extra_access = gb->extra_accessions;
		if (extra_access != NULL)
		{
			for (vnp=extra_access; vnp != NULL; vnp=vnp->next)
			{
				status = ValidateAccession(ptr, vnp->data.ptrvalue);
				if (status == 0)
					if (ptr1 != NULL)
					{
						if (btp->format == EMBL)
							ptr2 = Cat2Strings(ptr1, ptr, "; ", 0); 
						else
							ptr2 = Cat2Strings(ptr1, ptr, " ", 0); 
						ptr1 = MemFree(ptr1);
						ptr1 = ptr2;
					}
					else
						ptr1 = StringSave(ptr);
			}
		}
	}

	if ((vnp=BioseqContextGetSeqDescr(btp->bcp[count], Seq_descr_embl, NULL, NULL)) != NULL) 
	{
		eb = (EMBLBlockPtr) vnp->data.ptrvalue;
		extra_access = eb->extra_acc;
		if (extra_access != NULL)
		{
			for (vnp=extra_access; vnp != NULL; vnp=vnp->next)
			{
				status = ValidateAccession(ptr, vnp->data.ptrvalue);
				if (status == 0)
					if (ptr1 != NULL)
					{
						if (btp->format == EMBL)
							ptr2 = Cat2Strings(ptr1, ptr, "; ", 0); 
						else
							ptr2 = Cat2Strings(ptr1, ptr, " ", 0); 
						ptr1 = MemFree(ptr1);
						ptr1 = ptr2;
					}
					else
						ptr1 = StringSave(ptr);
			}
		}
	}

	for (index=0; index<btp->sfpXrefsize[count]; index++)
	{
		if (location == NULL)
		{
			location = ValNodeNew(NULL);
			si = SeqIntNew();
			location->choice = SEQLOC_INT;
			location->data.ptrvalue = si;
		}
		si->from = 0;
		bsp = btp->bsp[count];
		si->to = bsp->length - 1;
		si->id = bsp->id;	/* Don't delete id!! */
		sfp = btp->sfpXreflist[count][index];
		if (SeqLocCompare(sfp->location, location) != 0)
		{
			xref = (SeqLocPtr) sfp->data.value.ptrvalue;
			xid = (SeqIdPtr) xref->data.ptrvalue;
			if (xid->choice == 5 || xid->choice == 6 ||
				xid->choice == 13)
			{
				text = (TextSeqIdPtr) xid->data.ptrvalue;
				status = ValidateAccession(ptr, text->accession);
				if (status == 0)
					if (ptr1 != NULL)
					{
						if (btp->format == EMBL)
							ptr2 = Cat2Strings(ptr1, ptr, "; ", 0); 
						else
							ptr2 = Cat2Strings(ptr1, ptr, " ", 0); 
						ptr1 = MemFree(ptr1);
						ptr1 = ptr2;
					}
					else
						ptr1 = StringSave(ptr);
			}
		}
	}

	if (location)
	{
		si->id = NULL;
		SeqIntFree(si);
		ValNodeFree(location);
	}

	return ptr1;
}

#define BUF_EXT_LENGTH 4
/*****************************************************************************
*
*	ValidateLocus takes a locus name and assures that the format is 
*	proper.
*
*****************************************************************************/
CharPtr ValidateLocus(BiotablePtr btp, Int2 count, CharPtr base_locus, Int2 total_segs, Int2 num_seg, CharPtr new_buf, CharPtr buf_locus, CharPtr orig_buf)

{
	BioseqPtr bsp=btp->bsp[count];
	Boolean collision=FALSE;
	static Boolean order_init=FALSE;
	Char buf_ext[BUF_EXT_LENGTH], buffer[30];
	CharPtr tmp="0";
	DbtagPtr db;
	int dex;
	Int2 base_locus_max, buf_index, exp, length, num_of_digits;
	ObjectIdPtr ob;
	SeqIdPtr best_id, id;
	static Uint1 rel_order[18];

	if (! order_init)
	{
		for (dex=0; dex<18; dex++)
			rel_order[dex] = 255;
		rel_order[SEQID_GENERAL ] = 14;
	}
	order_init = TRUE;

	if (ASN2FF_AVOID_LOCUS_COLL || ASN2FF_REPORT_LOCUS_COLL)
	{	/* Check for LOCUS collisions with Karl's algorithm */
		id = bsp->id;
		best_id = SeqIdSelect( id, rel_order,18);
		if (best_id != NULL) {
			if (best_id -> choice == SEQID_GENERAL){ /* always! */

			    db = (DbtagPtr) best_id -> data.ptrvalue;
			    if (StringCmp(db -> db, LOCUS_COLLISION_DB_NAME) == 0){
				ob = db -> tag;
				    if ( ob != NULL)
				    {
					if (ASN2FF_REPORT_LOCUS_COLL)
					{
					    MakeAnAccession(buffer, bsp);
					    flat2asn_delete_locus_user_string();
					    flat2asn_install_locus_user_string(buffer);
					    flat2asn_delete_accession_user_string();
					    flat2asn_install_accession_user_string(buffer);
					     ErrPostEx(SEV_WARNING, ERR_LOCUS_LocusNameCollision, "");
					}
					if (ASN2FF_AVOID_LOCUS_COLL)
					{
					    collision=TRUE;
					    StringNCpy(new_buf, ob -> str, MAX_LOCUS_NAME_LEN);
					}
				    }
				}
			}
		}
	}

	if (! collision)
	{
		if (total_segs == 0)
		{	/* Not a segmented set. */
			if ((length=StringLen(buf_locus)) <= 0)
				new_buf = StringCpy(new_buf, orig_buf);
			else
				new_buf = StringCpy(new_buf, buf_locus);
				 
			new_buf = CheckLocusLength (btp, new_buf, MAX_LOCUS_NAME_LEN, 0);
		}
		else
		{
			if (total_segs < 10)
				num_of_digits = 1;
			else if (total_segs < 100)
				num_of_digits = 2;
			else if (total_segs < 1000)
				num_of_digits = 3;
			else 
			{
				ErrPostEx(SEV_INFO, ERR_SEGMENT_MoreThan1000Segs, "");
			}
			if (num_seg < 10)
				exp = 1;
			else if (num_seg < 100)
				exp = 2;
			else if (num_seg < 1000)
				exp = 3;
			base_locus_max = MAX_LOCUS_NAME_LEN - num_of_digits;
			length = StringLen(base_locus);
			StringCpy(new_buf, base_locus);
			MemSet((VoidPtr) buf_ext, '\0', BUF_EXT_LENGTH);	
			MemSet((VoidPtr) buf_ext, '0', num_of_digits);	
			sprintf(buf_ext+num_of_digits-exp, "%ld", (long) num_seg);
			buf_index = 0;
			while (buf_ext[buf_index] != '\0')
			{
			     new_buf[length+buf_index] = buf_ext[buf_index];
			     buf_index++;
			 }
			 new_buf[length+buf_index] = '\0';
		}
	}
	
	return new_buf;
}	/* ValidateLocus */


/***************************************************************************
*CharPtr CheckLocusLength (BiotablePtr btp, CharPtr locus, Int2 locus_max, Int2 total_segs)
*
*	Calculate the length of the locus; if it's too long, take characters
*	off the front.  If it's part of a segmented set and the locus ends 
*	in a number, add an "S".  If it appears to be an NCBI locus of the
*	form HSU00001, then take two letters off the front.
***************************************************************************/ 
CharPtr CheckLocusLength (BiotablePtr btp, CharPtr locus, Int2 locus_max, Int2 total_segs)

{
	Boolean cut_two=FALSE;
	CharPtr buffer;
	Int2 length, surplus;

	length = StringLen(locus);
	buffer = MemNew((length+2)*sizeof(Char));
	buffer = StringCpy(buffer, locus);
	
	if (total_segs > 0 && IS_DIGIT(locus[length-1]) != 0)
		if (locus[length-1] != '0' || IS_DIGIT(locus[length-2]) != 0)
		{
			length++;
			buffer[length-1] = 'S';
			buffer[length] = '\0';
		}

	surplus = length - locus_max;

	if (surplus > 0)
	{
		if (surplus <= 2)
		{	/* Check if this is of the form HSU00001S */
			if (IS_ALPHA(buffer[0]) != 0 &&
				IS_ALPHA(buffer[1]) != 0 &&
				IS_ALPHA(buffer[2]) != 0 &&
	    		        IS_DIGIT(buffer[3]) != 0 &&
	        	        IS_DIGIT(buffer[4]) != 0 &&
	       	          	IS_DIGIT(buffer[5]) != 0 &&
	       	             	IS_DIGIT(buffer[6]) != 0 &&
	       	                IS_DIGIT(buffer[7]) != 0 &&
	       	                buffer[8] == 'S' &&
				buffer[9] == '\0')
					cut_two = TRUE;
		}
	
		if (cut_two == TRUE)
			locus = StringCpy(locus, buffer+2);
		else
			locus = StringCpy(locus, buffer+surplus);
		if (btp->error_msgs == TRUE)
		{
			flat2asn_delete_locus_user_string();
			flat2asn_install_locus_user_string(buffer);
			ErrPostEx(SEV_INFO, ERR_LOCUS_ChangedLocusName, "");
		}
	}

	buffer = MemFree(buffer);

	return locus;
}	/* CheckLocusLength */

/******************************************************************************
*
*	FindRawSeq to find the raw sequences recursively from the
*	segmented sequences.
*
******************************************************************************/

static void FindRawSeq(BioseqPtr bsp, BiotablePtr btp_na, BiotablePtr btp_aa)
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
					FindRawSeq(bsp2, btp_na, btp_aa);
			}
		}
	}
}

/****************************************************************************
*
*	"PrintBaseCount" counts and prints the number of a, c, g, t, and 
*	other in a sequence.
*
****************************************************************************/

void PrintBaseCount (BiotablePtr btp, Int2 count)

{
	CharPtr buffer;
	Int4 base_count[5], total=0;
	SeqPortPtr spp;
	Uint1 residue;

	if (btp->base_cnt_line[count])
	{	/* Been there (at least once), done that.	*/
		buffer = btp->base_cnt_line[count];
	}
	else
	{
		base_count[0]=0;
		base_count[1]=0;
		base_count[2]=0;
		base_count[3]=0;
		base_count[4]=0;

		CheckSeqPort(btp, count, 0);
		spp = btp->spp;
		while ((residue=SeqPortGetResidue(spp)) != SEQPORT_EOF)
		{
			if ( !IS_residue(residue) && residue != INVALID_RESIDUE )
				continue;
	
			total++;
			switch (residue) {
				case 'A':
					base_count[0]++;
					break;
				case 'C':
					base_count[1]++;
					break;
				case 'G':
					base_count[2]++;
					break;
				case 'T':
					base_count[3]++;
					break;
				default:
						base_count[4]++;
						break;
			}
		}
		buffer = (CharPtr) MemNew(80*sizeof(Char));
		if (btp->format == EMBL)
		{
			sprintf(buffer, 
			"%ld BP; %ld A; %ld C; %ld G; %ld T; %ld other;", 
			(long) total, (long) base_count[0], (long) base_count[1], (long) base_count[2], (long) base_count[3], (long) base_count[4]);
		}
		else 	/* GENBANK format */
		{
			if (base_count[4] == 0)
			{
			sprintf(buffer, 
			"%7ld a%7ld c%7ld g%7ld t", 
			(long) base_count[0], (long) base_count[1], (long) base_count[2], (long) base_count[3]);
			}
			else
			{
			sprintf(buffer, 
			"%7ld a%7ld c%7ld g%7ld t%7ld others", 
			(long) base_count[0], (long) base_count[1], (long) base_count[2], (long) base_count[3], (long) base_count[4]);
			}
		}
		btp->base_cnt_line[count] = buffer;
	}

	if (btp->format == EMBL)
	{
		StartPrint(btp, 5, 5, ASN2FF_EMBL_MAX, "SQ");
		AddString(btp, "Sequence ");
		AddString(btp, buffer);
	}
	else
	{
		StartPrint(btp, 0, 0, ASN2FF_GB_MAX, NULL);
		AddString(btp, "BASE COUNT");
		TabToColumn(btp, 13);
		AddString(btp, buffer);
	}

	EndPrint(btp);
}	/* PrintBaseCount */

/****************************************************************************
* void CheckSeqPort (BiotablePtr btp, Int2 count, Int4 start)
*
*	This function checks a SeqPortPtr, maintained on the Biotable Ptr, 
*	and compares it's BioseqPtr to that of the BioseqPtr associated
*	with segment count of the btp.  At present, used only for nucleic
*	acids (4/14/94).
****************************************************************************/

void CheckSeqPort (BiotablePtr btp, Int2 count, Int4 start)
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
*	"PrintSequence" to get the biological sequence (in iupacna or
*	iupacaa format)	and put it in a buffer suitable for Genbank 
*	or EMBL format.
*
*	The variables "start" and "stop" allow one to read from a point 
*	not at the beginning of the sequence to a point not at the end
*	of the sequence.
*
*	Rewrite to store in a buffer and print out more at once????????
*****************************************************************************/

void PrintSequence (BiotablePtr btp, Int2 count, Int4 start, Int4 stop)

{
	BioseqPtr bsp=btp->bsp[count];
	Char buffer[MAX_BTP_BUF], num_buffer[10];
	CharPtr ptr = &(buffer[0]), num_ptr;
	Int4 index, inner_index, inner_stop, total=start;
	SeqPortPtr spp;
	Uint1 residue;


	if (btp->format == GENBANK)
	{
		StartPrint(btp, 0, 0, ASN2FF_GB_MAX, NULL);
		num_ptr = &(num_buffer[0]);
		sprintf(num_ptr, "%9ld", (long) (total+1));
		while ((*ptr = *num_ptr) != '\0')
		{
			ptr++; num_ptr++;
		}
		*ptr = ' '; ptr++;
		CheckSeqPort(btp, count, start);
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
					while ((*ptr = *num_ptr) != '\0')
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
	else if (btp->format == GENPEPT)
	{
		total++;

		StartPrint(btp, 0, 0, ASN2FF_GB_MAX, NULL);
		num_ptr = &(num_buffer[0]);
		sprintf(num_ptr, "%9ld", (long) total);
		while ((*ptr = *num_ptr) != '\0')
		{
			ptr++; num_ptr++;
		}
		*ptr = ' '; ptr++;
		if (ASN2FF_IUPACAA_ONLY == TRUE)
			spp = SeqPortNew(bsp, start, stop, 0, Seq_code_iupacaa);
		else
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
						while ((*ptr = *num_ptr) != '\0')
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
	else if (btp->format == EMBL)
	{	/* numbers at far right, let line go to MAX_BTP_BUF */

		StartPrint(btp, 5, 5, 0, NULL);
		CheckSeqPort(btp, count, start);
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
			total = inner_index;
			if (ROUNDUP(total, 10) == total) 
			{
				if (ROUNDUP(total, 60) == total)
				{
					*ptr = '\0';
					ptr = &buffer[0];
					AddString(btp, ptr);
					TabToColumn(btp, 73);
					AddInteger(btp, "%8ld", (long) total);
					if (ROUNDUP(total, SEQ_BLK_SIZE) != total)
						NewContLine(btp);
				}
				else
				{
					*ptr = ' '; ptr++;
				}
			}
		}
		total = stop+1;
		if (ROUNDUP(total, 60) != total)
		{
			*ptr = '\0';
			ptr = &buffer[0];
			AddString(btp, ptr);
			TabToColumn(btp, 73);
			AddInteger(btp, "%8ld", (long) total);
		}
	}

	EndPrint(btp);


}	/* PrintSequence */

/*****************************************************************************
*
*	"PrintEPSequence" to get the biological sequence (in iupacna or
*	iupacaa format)	and put it in a buffer suitable for Genbank 
*	or EMBL format.
*
*	The variables "start" and "stop" allow one to read from a point 
*	not at the beginning of the sequence to a point not at the end
*	of the sequence.
*
*	Rewrite to store in a buffer and print out more at once????????
*****************************************************************************/

void PrintEPSequence (BiotablePtr btp, Int2 count, Int4 start, Int4 stop)

{
	BioseqPtr bsp=btp->bsp[count];
	Char buffer[MAX_BTP_BUF];
	CharPtr ptr = &(buffer[0]);
	Int4 index, inner_index, inner_stop, total=start;
	SeqPortPtr spp;
	Uint1 residue;


	/* numbers at far right, let line go to MAX_BTP_BUF */

	StartPrint(btp, 5, 5, 0, NULL);
	if (ASN2FF_IUPACAA_ONLY == TRUE)
		spp = SeqPortNew(bsp, start, stop, 0, Seq_code_iupacaa);
	else
		spp = SeqPortNew(bsp, start, stop, 0, Seq_code_ncbieaa);
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
	
			*ptr = residue; ptr++;
		}
		total = inner_index;
		if (ROUNDUP(total, 10) == total) 
		{
			if (ROUNDUP(total, 60) == total)
			{
				*ptr = '\0';
				ptr = &buffer[0];
				AddString(btp, ptr);
				TabToColumn(btp, 73);
				AddInteger(btp, "%8ld", (long) total);
				if (ROUNDUP(total, SEQ_BLK_SIZE) != total)
					NewContLine(btp);
			}
			else
			{
				*ptr = ' '; ptr++;
			}
		}
	}
	total = stop+1;
	if (ROUNDUP(total, 60) != total)
	{
		*ptr = '\0';
		ptr = &buffer[0];
		AddString(btp, ptr);
		TabToColumn(btp, 73);
		AddInteger(btp, "%8ld", (long) total);
	}

	EndPrint(btp);

	SeqPortFree(spp);


}	/* PrintEPSequence */

/***************************************************************************
*
*	"GetMolInfo" gets information about the molecule for the locus
*	line.  The formatted information is in "buffer".
*
***************************************************************************/

void GetMolInfo (CharPtr buffer, Int2 count, BiotablePtr btp)

{
	static CharPtr strand [4]= { "   ", "ss-", "ds-","ms-"};
	static CharPtr mol [9] = {"    ", "DNA ", "RNA ", " AA ", "mRNA", "rRNA", "tRNA", "uRNA", "scRNA"};
	static CharPtr pseudo_mol [9] = {"    ", "DNA", "RNA", "AA", "mRNA", "rRNA", "tRNA", "uRNA", "scRNA"};
	static CharPtr embl_mol [8] = {"xxx", "DNA", "RNA", "AA ", "RNA", "RNA", "RNA", "RNA"};

	BioseqPtr bsp;
	Int2 istrand, imol;
	ValNodePtr vnp;
	MolInfoPtr mfp;

	bsp = btp->bsp[count];
	istrand = bsp->strand;
	if (istrand > 3) 
		istrand = 0;

	imol = bsp->mol;
	if (imol > 3)
		imol = 0;

/*keep both old and new style */
/* get new first */
	if ((vnp=BioseqContextGetSeqDescr(btp->bcp[count], Seq_descr_molinfo, NULL, NULL)) != NULL) 
	{
		mfp = vnp->data.ptrvalue;
		if (mfp->biomol >=3 && mfp->biomol <= 6)
			imol = (Int2) (mfp->biomol + 1);
	}
	else if ((vnp=BioseqContextGetSeqDescr(btp->bcp[count], Seq_descr_mol_type, NULL, NULL)) != NULL) 
	{
		if (vnp->data.intvalue >=3 && vnp->data.intvalue <= 6)
			imol = (Int2) (vnp->data.intvalue + 1);
	}

/* if ds-DNA don't show ds */
	if (imol == 1 && istrand == 2) { 
		istrand = 0;
	} 
/* ss-any RNA don't show ss */
	if (imol > 3 && istrand == 1) { 
		istrand = 0;
	} 
	
	
	if (btp->format == GENBANK)
	{
		if (bsp->topology == 2)
			sprintf(buffer,
			"%7ld bp %s%-4s  circular", 
			bsp->length, strand[istrand], mol[imol]);
		else
			sprintf(buffer, "%7ld bp %s%-4s          ", 
			bsp->length, strand[istrand], mol[imol]);
	}
	else if (btp->format == GENPEPT)
	{
			sprintf(buffer, "%7ld aa", bsp->length);
	}
	else if (btp->format == EMBL)
	{
		if (btp->pseudo == FALSE) /* do authentic EMBL */
		{
			if (imol < 8)
			{
				if (bsp->topology == 2)
					sprintf(buffer, "circular %s", embl_mol[imol]);
				else
					sprintf(buffer, "%s", embl_mol[imol]);
			}
		}
		else 		/* Use GenBank molecule names */
		{
			if (bsp->topology == 2)
				sprintf(buffer, "circular %s", pseudo_mol[imol]);
			else
				sprintf(buffer, "%s", pseudo_mol[imol]);
		}
	}
	return;
}

/**************************************************************************
*void EstablishProteinLink (BiotablePtr btp_na, BiotablePtr btp_aa, Int2 count)
*
*	This function has a two-fold function: one is to find the 
*	BioseqContextPtr (bcp) that applies to the nucleotide
*	entry which relates to the protein entry we're studying; 
*	the other is to discover if more than one cds is on the
*	nucleotide sequence, in order to determine if the comment
*	should be printed out.
*
*	count as an argument above refers to the amino acid BiotablePtr
****************************************************************************/

void EstablishProteinLink (BiotablePtr btp_na, BiotablePtr btp_aa, Int2 count)

{
	BioseqContextPtr bcp;
	BioseqPtr bsp;
	Boolean found=FALSE;
	Int2 index, number_of_cds, status;
	SeqFeatPtr sfp=NULL;
	SeqIntPtr sip=NULL;
	ValNodePtr prot_location=NULL;

	number_of_cds = btp_aa->number_of_cds;

	for (index=0; index<btp_na->count; index++)
	{
		bsp = btp_na->bsp[index];
		if ((OneOfThree(bsp) == TRUE) &&
			(bsp->repr == Seq_repr_raw || 
				bsp->repr == Seq_repr_const))
		{
			bcp = btp_na->bcp[index];
			sfp = BioseqContextGetSeqFeat(bcp, 3, NULL, NULL, 2);
			while (sfp != NULL)
			{
				number_of_cds++;
				if (prot_location == NULL)
				{
					bsp = btp_aa->bsp[count];
					prot_location = (SeqLocPtr) ValNodeNew(NULL);
					prot_location->choice = SEQLOC_INT;
					sip = SeqIntNew();
					sip->from = 0;
					sip->to = (bsp->length)-1;
					sip->id = bsp->id; /* Don't delete! */
					prot_location->data.ptrvalue = sip;
				}
				if (found != TRUE)
				{ 
/* Only do compare if we're still looking for a cds that matches protein */
					status = SeqLocCompare(sfp->product, prot_location);
					if (status != 0)
					{
						btp_aa->bcp_na[count] = bcp;
						btp_aa->bcp_na_alloc[count] = 
							btp_na->bcp_na_alloc[index];
						btp_aa->cds[count] = sfp;
						found = TRUE;
					}
				}

				if (found == TRUE && number_of_cds > 1)
					sfp = NULL;
				else
					sfp = BioseqContextGetSeqFeat(bcp, 3, sfp, NULL, 2);
			}
			if (found == TRUE && number_of_cds > 1)
				break;
		}
	}
/* Only store this on the first pass through. */
	if (btp_aa->number_of_cds == 0)	
		btp_aa->number_of_cds = number_of_cds;

	if (prot_location != NULL)
	{
		sip->id = NULL;
		SeqIntFree(sip);
		ValNodeFree(prot_location);
	}

	return;
}	/* EstablishProteinLink */

void GetDivision(CharPtr buffer, Int2 count, BiotablePtr btp) 

{
	
	GBBlockPtr gb=NULL;
	EMBLBlockPtr eb=NULL;
	static CharPtr embl_embl_divs [] = {
	"FUN",
	"INV",
	"MAM",
	"ORG",
	"PHG",
	"PLN",
	"PRI",
	"PRO",
	"ROD",
	"SYN",
	"UNA",
	"VRL",
	"VRT",
	"PAT",
	"EST",
	"STS"
	};
	static CharPtr genbank_embl_divs [] = {
	"PLN",
	"INV",
	"MAM",
	"ORG",
	"PHG",
	"PLN",
	"PRI",
	"BCT",
	"ROD",
	"SYN",
	"UNA",
	"VRL",
	"VRT",
	"PAT",
	"EST",
	"STS"
	};
	BioseqContextPtr bcp=btp->bcp[count];
	ValNodePtr descr;
	CharPtr gb_div=NULL, embl_div=NULL;
	CharPtr PNTR div_use=NULL;

	descr=NULL;
	while ((descr=BioseqContextGetSeqDescr(bcp, Seq_descr_genbank, descr, NULL)) != NULL) 
	{
		gb = (GBBlockPtr) descr->data.ptrvalue;
		if ((gb_div=gb->div) != NULL)
			break;
	}

	descr=NULL;
	while ((descr=BioseqContextGetSeqDescr(bcp, Seq_descr_embl, descr, NULL)) != NULL) 
	{
		eb = (EMBLBlockPtr) descr->data.ptrvalue;
		if (eb->div)
			break;
	}

	if (eb && eb->div)
	{
		if (eb->div != 255 && eb->div <= 15)
			if (btp->format == EMBL)
			{
				div_use = embl_embl_divs;
			}
			else
			{
				div_use = genbank_embl_divs;
			}
	
		if (div_use)
			embl_div = div_use[eb->div];
	}

	if (btp->format == EMBL)
	{
		if (embl_div != NULL)
		{
			buffer = StringCpy(buffer, embl_div);
		}
		else if (gb_div != NULL) 
		{
			if (StringCmp(gb_div, "BCT") == 0)
				buffer = StringCpy(buffer, embl_embl_divs[7]);
			else
				buffer = StringCpy(buffer, gb_div);
		}
		else
			buffer = StringCpy(buffer, "   ");
	}
	else
	{
		if (gb_div != NULL)
			buffer = StringCpy(buffer, gb_div);
		else if (embl_div != NULL)
			buffer = StringCpy(buffer, embl_div);
		else if (btp->format != GENPEPT)
			buffer = StringCpy(buffer, "   ");
	}

	return;
}

/*************************************************************************
*void PrintXrefLine (BiotablePtr btp, Int2 count)
*
*	Prints out the cross-refs from the EMBL block, in the descriptor.
*	Used ONLY to make EMBL output.
**************************************************************************/

void PrintXrefLine (BiotablePtr btp, Int2 count)

{
	Boolean done_once=FALSE;
	Char buffer[20], buffer1[20], buffer2[20];
	CharPtr name, string;
	EMBLBlockPtr eb=NULL;
	EMBLXrefPtr xref=NULL;
	ObjectIdPtr oip;
	Uint1 class;
	ValNodePtr descr=NULL, id;

	while ((descr=BioseqContextGetSeqDescr(btp->bcp[count], Seq_descr_embl, descr, NULL)) != NULL) 
	{
		eb = (EMBLBlockPtr) descr->data.ptrvalue;
		for (xref=eb->xref; xref; xref=xref->next)
		{
			name=NULL;
			class=xref->_class;
			if (class)
			{
				if (class == 5)
					StringCpy(buffer, "SWISS-PROT");
				else if (class == 8)
					StringCpy(buffer, "EPD");
				else if (class == 10)
					StringCpy(buffer, "TFD");
				else if (class == 11)
					StringCpy(buffer, "FLYBASE");
				name = &(buffer[0]);
			}
			else if (xref->name)
				name = xref->name;
			if (name && xref->id)
			{
				id=xref->id;
			
				oip = id->data.ptrvalue;
				if (oip->str)
					StringCpy(buffer1, oip->str);
				else if (oip->id)
					sprintf(buffer1, "%ld", (long) (oip->id));
				id = id->next;
				if (id)
				{
					oip = id->data.ptrvalue;
					if (oip->str)
						StringCpy(buffer2, oip->str);
					else if (oip->id)
						sprintf(buffer2, "%ld", (long) (oip->id));
				}
				if (done_once == FALSE)
				{
					StartPrint(btp, 5, 5, ASN2FF_EMBL_MAX, "DR");
					done_once=TRUE;
				}
				else
					NewContLine(btp);
				AddString(btp, name);
				AddString(btp, "; ");
				AddString(btp, buffer1);
				AddString(btp, "; ");
				string = CheckEndPunctuation(buffer2, '.');
				AddString(btp, string);
				string = MemFree(string);
			}
		}
	}
	if (done_once)
	{
		EndPrint(btp);
		PrintXX(btp);
	}
}

/*************************************************************************
*Boolean CheckXrefLine (BiotablePtr btp, Int2 count)
*
*	Checks if there is a Xref in EMBL format.
*	Used ONLY to make EMBL output.
*This could probably be done more efficiently???????????????????
**************************************************************************/

Boolean CheckXrefLine (BiotablePtr btp, Int2 count)

{
	Boolean ret_val=FALSE;
	Char buffer[20];
	CharPtr name;
	EMBLBlockPtr eb=NULL;
	EMBLXrefPtr xref=NULL;
	Uint1 class;
	ValNodePtr descr=NULL;

	while ((descr=BioseqContextGetSeqDescr(btp->bcp[count], Seq_descr_embl, descr, NULL)) != NULL) 
	{
		eb = (EMBLBlockPtr) descr->data.ptrvalue;
		for (xref=eb->xref; xref; xref=xref->next)
		{
			name=NULL;
			class=xref->_class;
			if (class)
			{
				if (class == 5)
					StringCpy(buffer, "SWISS-PROT");
				else if (class == 8)
					StringCpy(buffer, "EPD");
				else if (class == 10)
					StringCpy(buffer, "TFD");
				else if (class == 11)
					StringCpy(buffer, "FLYBASE");
				name = &(buffer[0]);
			}
			else if (xref->name)
				name = xref->name;
			if (name && xref->id)
				ret_val = TRUE;
			else
				ret_val = FALSE;
		}
	}
	return ret_val;
}
	
	
void PrintLocusLine(BiotablePtr btp, Int2 count)

{
	BioseqPtr bsp=btp->bsp[count];
	Char buffer[30];

	if (btp->format == EMBL)
	{
		StartPrint(btp, 5, 0, ASN2FF_EMBL_MAX, "ID");
		AddString(btp, btp->locus[count]);
		AddString(btp, " standard; ");
		GetMolInfo(buffer, count, btp);
		AddString(btp, buffer);
		AddString(btp, "; ");
		AddString(btp, btp->division[count]);
		AddString(btp, "; ");
		AddInteger(btp, "%ld", (long) bsp->length);
		AddString(btp, " BP.");
		EndPrint(btp);
	}
	else
	{
		StartPrint(btp, 0, 0, ASN2FF_GB_MAX, NULL);
		AddString(btp, "LOCUS");
		TabToColumn(btp, 13);
		AddString(btp, btp->locus[count]);
		GetMolInfo(buffer, count, btp);
		AddString(btp, buffer);
		TabToColumn(btp, 53);
		AddString(btp, btp->division[count]);
		TabToColumn(btp, 63);
		AddString(btp, btp->date[count]);
		EndPrint(btp);
	}
}

void PrintEPLocusLine(BiotablePtr btp, Int2 count)

{
	BioseqPtr bsp=btp->bsp[count];
	Char buffer[30];

	StartPrint(btp, 5, 0, ASN2FF_EMBL_MAX, "ID");
	AddString(btp, btp->locus[count]);
	AddString(btp, " standard; ");
	GetMolInfo(buffer, count, btp);
	AddString(btp, buffer);
	AddString(btp, "; ");
	AddString(btp, btp->division[count]);
	AddString(btp, "; ");
	AddInteger(btp, "%ld", (long) bsp->length);
	AddString(btp, " RS.");
	EndPrint(btp);
}

void PrintXX(BiotablePtr btp)

{
	StartPrint(btp, 0, 0, ASN2FF_EMBL_MAX, "XX");
	EndPrint(btp);
}


void PrintAccessLine(BiotablePtr btp, Int2 count)

{
	CharPtr extra_access=NULL;

	if (btp->format == EMBL)
	{
		StartPrint(btp, 5, 5, ASN2FF_EMBL_MAX, "AC");
	}
	else
	{
		StartPrint(btp, 0, 12, ASN2FF_GB_MAX, NULL);
		AddString(btp, "ACCESSION");
		TabToColumn(btp, 13);
	}
	AddString(btp, btp->access[count]);
	if (btp->format == EMBL)
		AddChar(btp, ';');

	extra_access = GetExtraAccessions(btp, count);
	if (extra_access)
	{
		AddChar(btp, ' ');
		AddString(btp, extra_access);
		if (btp->format == EMBL)
			AddChar(btp, ';');
		extra_access = MemFree(extra_access);
	}
	EndPrint(btp);
	return;
}

/*************************************************************************
*void PrintDateLines (BiotablePtr btp, Int2 count)
*
*	prints the date lines for the EMBL flatfile.
**************************************************************************/
void PrintDateLines (BiotablePtr btp, Int2 count)

{
	StartPrint(btp, 5, 5, ASN2FF_EMBL_MAX, "DT");
	if (btp->update_date[count])
	{	/* both create and update date exist.	*/
		if (btp->pseudo == FALSE)
		{ /* In pseudo-EMBL mode only one date line */
			AddString(btp, btp->create_date[count]);
			NewContLine(btp);
		}
		AddString(btp, btp->update_date[count]);
		if (btp->embl_rel[count])
		{
			AddString(btp, " (Rel. ");
			AddString(btp, btp->embl_rel[count]);
			AddString(btp, ", Last updated, Version ");
			AddInteger(btp, "%ld", (long) btp->embl_ver[count]);
			AddChar(btp, ')');
		}
	}
	else
	{	/* only create date exists.	*/
		AddString(btp, btp->create_date[count]);
		if (btp->embl_rel[count])
		{
			AddString(btp, " (Rel. ");
			AddString(btp, btp->embl_rel[count]);
			AddString(btp, ", Last updated, Version ");
			AddInteger(btp, "%ld", (long) btp->embl_ver[count]);
			AddChar(btp, ')');
		}
		if (btp->pseudo == FALSE)
		{ /* In pseudo-EMBL only one date line. */
			NewContLine(btp);
			AddString(btp, btp->create_date[count]);
			if (btp->embl_rel[count])
			{
				AddString(btp, " (Rel. ");
				AddString(btp, btp->embl_rel[count]);
				AddString(btp, ", Last updated, Version ");
				AddInteger(btp, "%ld", (long) btp->embl_ver[count]);
				AddChar(btp, ')');
			}
		}
	}
	EndPrint(btp);
}	/* PrintDateLines */

void PrintSegmentLine(BiotablePtr btp, Int2 count)

{

	if (btp->seg_total[count] > 0)
	{
		StartPrint(btp, 0, 0, ASN2FF_GB_MAX, NULL);
		AddString(btp, "SEGMENT");
		TabToColumn(btp, 13);
		AddInteger(btp, "%ld", (long) btp->seg_num[count]);
		AddString(btp, " of ");
		AddInteger(btp, "%ld", (long) btp->seg_total[count]);
		EndPrint(btp);
	}
}

ValNodePtr GetKeywordLine(BiotablePtr btp, Int2 count)

{
	ValNodePtr block, keyword=NULL;
	GBBlockPtr gbp;
	EMBLBlockPtr ebp;
	PirBlockPtr pbp;
	PrfBlockPtr prfp;
	SPBlockPtr spbp;

	if ((block=BioseqContextGetSeqDescr(btp->bcp[count], Seq_descr_genbank, NULL, NULL)) != NULL) 
	{
		gbp = (GBBlockPtr) block->data.ptrvalue;
		keyword = gbp->keywords;
	}

	if (keyword == NULL && (block=BioseqContextGetSeqDescr(btp->bcp[count], Seq_descr_embl, NULL, NULL)) != NULL) 
	{
		ebp = (EMBLBlockPtr) block->data.ptrvalue;
		keyword = ebp->keywords;
	}

	if (keyword == NULL && (block=BioseqContextGetSeqDescr(btp->bcp[count], Seq_descr_pir, NULL, NULL)) != NULL) 
	{
		pbp = (PirBlockPtr) block->data.ptrvalue;
		keyword = pbp->keywords;
	}

	if (keyword == NULL && (block=BioseqContextGetSeqDescr(btp->bcp[count], Seq_descr_prf, NULL, NULL)) != NULL) 
	{
		prfp = (PrfBlockPtr) block->data.ptrvalue;
		keyword = prfp->keywords;
	}

	if (keyword == NULL && (block=BioseqContextGetSeqDescr(btp->bcp[count], Seq_descr_sp, NULL, NULL)) != NULL) 
	{
		spbp = (SPBlockPtr) block->data.ptrvalue;
		keyword = spbp->keywords;
	}

	return keyword;

}	/* GetKeywordLine */


void PrintKeywordLine(BiotablePtr btp, Int2 count)

{
	Boolean line_return;
	CharPtr string;
	Int2 tab_length=12;
	ValNodePtr keyword, vnp;

	keyword = GetKeywordLine(btp, count);

	if (btp->format == EMBL)
	{
		StartPrint(btp, 5, 5, ASN2FF_EMBL_MAX, "KW");
	}
	else
	{
		StartPrint(btp, 0, tab_length, ASN2FF_GB_MAX, NULL);
		AddString(btp, "KEYWORDS");
		TabToColumn(btp, (tab_length+1));
	}
	if (keyword != NULL)
	{	/* the next line initializes the length */
		line_return = DoSpecialLineBreak(btp, NULL, tab_length);
		for (vnp=keyword; vnp != NULL; vnp=vnp->next)
		{
			string = vnp->data.ptrvalue;
			line_return = DoSpecialLineBreak(btp, string, tab_length);
			if (line_return)
				NewContLine(btp);
			AddString(btp, string);
			if (vnp->next != NULL)
			{
				AddChar(btp, ';');
				AddChar(btp, ' ');
			}
		}
	}
	AddChar(btp, '.');
	EndPrint(btp);

}	/* PrintKeywordLine */

void PrintDefinitionLine(BiotablePtr btp, Int2 count)

{
	BioseqContextPtr bcp=btp->bcp[count];
	BioseqPtr bsp=btp->bsp[count];
	Boolean found_title=FALSE;
	Char tmp[200];
	CharPtr string, string_start, title=NULL;
	Int2 length=0;
	PatentSeqIdPtr psip;
	PdbBlockPtr pbp;
	ValNodePtr vnp;

	if ((title = BioseqContextGetTitle(bcp)) != NULL)
		; 
	else if ((vnp = BioseqContextGetSeqDescr(bcp, Seq_descr_pdb, NULL, NULL)) != NULL)
	{
		pbp = (PdbBlockPtr)(vnp->data.ptrvalue);
		title = (CharPtr) (pbp->compound->data.ptrvalue);
	}

	if (title)	/* Some titles are of length zero. */
		length = StringLen(title);
	if (length > 0)
		found_title=TRUE;

	if (!found_title)
	{
		title = tmp;
		vnp = bsp->id;
		while (vnp != NULL)
		{
			if (vnp->choice == SEQID_PATENT)
			{
				psip = (PatentSeqIdPtr)(vnp->data.ptrvalue);
				sprintf(title, "Sequence %d from patent %s %s",
					(int)psip->seqid, psip->cit->country, psip->cit->number);
				vnp = NULL;
				found_title = TRUE;
			}
			else
				vnp = vnp->next;
		}
		if (! found_title)
			StringCpy(title, "No definition line found");
	}


	string_start = string = CheckEndPunctuation(title, '.');

	while (*string != '\0')
	{
		if (*string == '\"')
			*string = '\'';
		string++;
	}

	string = string_start;

	if (btp->format == EMBL)
	{
		StartPrint(btp, 5, 5, ASN2FF_EMBL_MAX, "DE");
	}
	else
	{
		StartPrint(btp, 0, 12, ASN2FF_GB_MAX, NULL);
		AddString(btp, "DEFINITION");
		TabToColumn(btp, 13);
	}
	AddString(btp, string);
	EndPrint(btp);

	MemFree(string);
}

void PrintOriginLine(BiotablePtr btp, Int2 count)

{
	Char buffer[68];
	CharPtr origin=NULL;
	GBBlockPtr gb;
	Int2 length=0;
	ValNodePtr vnp=NULL;

	StartPrint(btp, 0, 12, ASN2FF_GB_MAX, NULL);
	AddString(btp, "ORIGIN");
	TabToColumn(btp, 13);
	if ((vnp=BioseqContextGetSeqDescr(btp->bcp[count], Seq_descr_genbank, NULL, NULL)) != NULL)
	{
		gb = (GBBlockPtr) vnp->data.ptrvalue;
		if (gb)
		{
			if (gb->origin && (length=StringLen(gb->origin)) > 0)
			{ /*???? What if gb->origin is longer than 68 chars. */
				StringNCpy(buffer, gb->origin, 66);
				if (length < 66)
					buffer[length] = '\0';
				else
					buffer[66] = '\0';
				origin = CheckEndPunctuation(buffer, '.');
				AddString(btp, origin);
			}
			if (length > 66)
				ErrPostEx(SEV_WARNING, ERR_ENTRY_OriginTooLong, "");
		}
	}
	if (origin != NULL)
		MemFree(origin);
	EndPrint(btp);

}

/***************************************************************************
*PrintSourceLine
*
*	"PrintSourceLine" to print the source and organism entries for 
*	genbank and genpept FlatFiles.
*
*Rewrite for better logic???? (11/30/93 & 12/13/93)
*To take care of pdb, pir cases???? (12/13/93)
****************************************************************************/

void PrintSourceLine (BiotablePtr btp, Int2 count)

{
	BiotablePtr btp_na;
	Boolean found_tax;
	CharPtr organelle, organism, source=NULL, newsource, taxonomy=NULL;
	GBBlockPtr gb=NULL;
	OrgRefPtr orp=NULL;
	BioSourcePtr biosp;
/*	See note below, remove when PDB source better.
	PdbBlockPtr pbp=NULL;
*/
	ValNodePtr vnp=NULL;

	
	if ((vnp=BioseqContextGetSeqDescr(btp->bcp[count], Seq_descr_genbank, NULL, NULL)) != NULL) 
	{
		gb = (GBBlockPtr) vnp->data.ptrvalue;
		if (gb)
			source = GetGBSourceLine(gb);
	}

/* PDB source not in parsable form, not being used now.
	if (source == NULL && (vnp=BioseqContextGetSeqDescr(btp->bcp[count], Seq_descr_pdb, NULL, NULL)) != NULL) 
	{ 
		pbp = (PdbBlockPtr) vnp->data.ptrvalue;
		if (pbp)
			source = GetPDBSourceLine(pbp);
	}
*/
/*keep both old and new style */
/* try new first */
	if ((vnp=BioseqContextGetSeqDescr(btp->bcp[count], Seq_descr_source, NULL, NULL)) != NULL) 
	{
		biosp = vnp->data.ptrvalue;
		orp = (OrgRefPtr) biosp->org;
	} else {
	}
/* try old */
	if (!orp) {
		if ((vnp=BioseqContextGetSeqDescr(btp->bcp[count], Seq_descr_org, NULL, NULL)) != NULL) 
		{
			orp = (OrgRefPtr) vnp->data.ptrvalue;
		}
		else if (btp->sfpOrgsize[count] != 0)
		{
			orp = (OrgRefPtr) (btp->sfpOrglist[count][0])->data.value.ptrvalue;
		}
	}
	if (!source && orp)
	{
		source = FlatStringGroup(orp->common?orp->common:orp->taxname, orp->mod, " ", NULL);
	}

	StartPrint(btp, 0, 12, ASN2FF_GB_MAX, NULL);
	AddString(btp, "SOURCE");
	TabToColumn(btp, 13);
	if (source)
	{
		newsource = CheckEndPunctuation(source, '.');
		AddString(btp, newsource);
		MemFree(source);
		MemFree(newsource);
	}
	else
	{
		AddString(btp, "Unknown.");
		if (btp->error_msgs == TRUE)
			ErrPostEx(SEV_WARNING, ERR_ENTRY_No_source_line, "");
	}
	EndPrint(btp);

	if (orp)
		if(orp->common && !orp->taxname)
			orp->taxname = TaxNameFromCommon(orp->common);

	organelle = FlatOrganelle(btp, count);
	StartPrint(btp, 2, 12, ASN2FF_GB_MAX, NULL);
	AddString(btp, "ORGANISM");
	TabToColumn(btp, 13);
	if (orp && orp->taxname)
	{
		organism = FlatStringGroup(organelle, NULL, NULL, orp->taxname);
		AddString(btp, organism);
		MemFree(organism);
	}
	else
	{
		AddString(btp, "Unknown.");
	}
	MemFree(organelle);
	EndPrint(btp);

/* major KLUDGE coming up.  The next lines find a GBBlockPtr from the 
nucleotide sequences if gb->taxonomy wasn't found in GenPept and the
database in question is one of the three nucleotide databases.	*/
	btp_na = btp->btp_other;
	if (btp->format == GENPEPT && (OneOfThree(btp_na->bsp[0]) == TRUE))
	{
		found_tax = FALSE;
		if (gb && gb->taxonomy)
			found_tax = TRUE;
		if (found_tax == FALSE)
		{
			if ((vnp=BioseqContextGetSeqDescr(btp->bcp_na[count], Seq_descr_genbank, NULL, NULL)) != NULL) 
				gb = (GBBlockPtr) vnp->data.ptrvalue;
		}
	}
/* End of KLUDGE */

	StartPrint(btp, 12, 12, ASN2FF_GB_MAX, NULL);
	if (gb && gb->taxonomy)
	{
		taxonomy = CheckEndPunctuation(gb->taxonomy, '.');
		AddString(btp, taxonomy);
		MemFree(taxonomy);
	}
	else
	{
		AddString(btp, "Unclassified.");
	}
	EndPrint(btp);
}	/* PrintSourceLine */

/***************************************************************************
*PrintOrganismLine
*
*	"PrintOrganismLine" to print the source and organism entries for 
*	EMBL FlatFiles.
*
*Rewrite for better logic???? (11/30/93 & 12/13/93)
*To take care of pdb, pir cases???? (12/13/93)
*Note: two organism lines are searched for!!
****************************************************************************/

void PrintOrganismLine (BiotablePtr btp, Int2 count)

{
	ValNodePtr vnp=NULL;
	OrgRefPtr orp=NULL, orp1=NULL;
	CharPtr organelle, taxonomy=NULL;
	BioSourcePtr	biosp;
	GBBlockPtr gb=NULL;

	
	if ((vnp=BioseqContextGetSeqDescr(btp->bcp[count], Seq_descr_genbank, NULL, NULL)) != NULL) 
		gb = (GBBlockPtr) vnp->data.ptrvalue;
/* new first */
	if ((vnp=BioseqContextGetSeqDescr(btp->bcp[count], Seq_descr_source, NULL, NULL)) != NULL) {
		biosp = (BioSourcePtr) vnp->data.ptrvalue;
		orp = (OrgRefPtr) biosp->org;
		if ((vnp=BioseqContextGetSeqDescr(btp->bcp[count], Seq_descr_source, NULL, NULL)) != NULL) {
			biosp = (BioSourcePtr) vnp->data.ptrvalue;
			orp1 = (OrgRefPtr) biosp->org;
		}
	}
	if (!orp) {
		if ((vnp=BioseqContextGetSeqDescr(btp->bcp[count], Seq_descr_org, NULL, NULL)) != NULL) 
		{	
			orp = (OrgRefPtr) vnp->data.ptrvalue;
			if ((vnp=BioseqContextGetSeqDescr(btp->bcp[count], Seq_descr_org, vnp, NULL)) != NULL)
			orp1 = (OrgRefPtr) vnp->data.ptrvalue;
		}
		else if (btp->sfpOrgsize[count] != 0)
		{
			orp = (OrgRefPtr) (btp->sfpOrglist[count][0])->data.value.ptrvalue;
			if (btp->sfpOrglist[count][1])
				orp1 = (OrgRefPtr) (btp->sfpOrglist[count]
											[1])->data.value.ptrvalue;
		}
	}

	if (orp)
		if(orp->common && !orp->taxname)
			orp->taxname = TaxNameFromCommon(orp->common);

	StartPrint(btp, 5, 5, ASN2FF_EMBL_MAX, "OS");
	if (orp && orp->taxname)
	{
		AddString(btp, orp->taxname);
		if (orp->common)
		{
			AddString(btp, " (");
			AddString(btp, orp->common);
			AddChar(btp, ')');
		}
	}
	else
		AddString(btp, "Unclassified.");

	EndPrint(btp);

	StartPrint(btp, 5, 5, ASN2FF_EMBL_MAX, "OC");
	if (gb && gb->taxonomy)
	{
		taxonomy = CheckEndPunctuation(gb->taxonomy, '.');
		AddString(btp, taxonomy);
		MemFree(taxonomy);
	}
	else
	{
		AddString(btp, "Unclassified.");
	}
	EndPrint(btp);
	
	if (orp1)	/* second organism */
	{
		if (orp1 && orp1->taxname)
		{
			PrintXX(btp);
			StartPrint(btp, 5, 5, ASN2FF_EMBL_MAX, "OS");
			AddString(btp, orp1->taxname);
			if (orp1->common)
			{
				AddString(btp, " (");
				AddString(btp, orp1->common);
				AddChar(btp, ')');
			}
			EndPrint(btp);
		}
	}

/* What about plasmids on the OG line???????????????*/
/* Get this info from a qual of the SourceFeat that has qual "plasmid"??*/
	organelle = FlatOrganelle(btp, count);
	if (organelle)
	{
		PrintXX(btp);
		StartPrint(btp, 5, 5, ASN2FF_EMBL_MAX, "OG");
		AddString(btp, organelle);
		EndPrint(btp);
		MemFree(organelle);
	}

}	/* PrintOrganismLine */

/****************************************************************************
*GetPDBSourceLine
*
*	Gets the source from the PDBBlock.
*
****************************************************************************/

CharPtr GetPDBSourceLine (PdbBlockPtr pdb)

{
	CharPtr source = NULL;
	ValNodePtr vnp;

	if(pdb && pdb->source)
	{
		vnp = pdb->source;
		source = StringSave(vnp->data.ptrvalue);
	}

	return source;
}

void GetAASeqFeat(BiotablePtr btp)

{
	Int2 count, index=0, size;
	SeqFeatPtr sfp;

	GetSeqFeat(btp);

	for (count=0; count<(btp->count); count++)
	{

		size = btp->sfpListsize[count];
		for (index=0; index<size; index++)
		{
			sfp = btp->sfpList[count][index];
			GetGeneQuals(sfp, btp, count, index);
			MatchAAGeneToFeat(btp, count, sfp, index);
		}
	}
}

void GetNASeqFeat(BiotablePtr btp)

{
	Int2 count, index=0, size;
	NoteStructPtr nsp;
	SeqFeatPtr sfp;

	GetSeqFeat(btp);

	for (count=0; count<(btp->count); count++)
	{
		size = btp->sfpListsize[count];
		for (index=0; index<size; index++)
		{
			sfp = btp->sfpList[count][index];
/* These line are here to make QA easier (see note in ConvertToNAImpFeat).
This activity can probably be moved back to ConvertToNAImpFeat after QA. */
		        if (sfp->comment)
			{
				nsp = btp->nspp[count][index];
				CpNoteToCharPtrStack(nsp, NULL, sfp->comment);
			}

			GetCdregionGeneXrefInfo(sfp, btp, count, index);
			GetGeneQuals(sfp, btp, count, index);
			CompareGenesToQuals(btp, count, index);
			MatchNAGeneToFeat(btp, count, sfp, index);
		}
		CompareGenesToSourceQuals (btp, count);
/*		MatchNAFeatToGene(btp, count); */
	}

	MarkRedundantGeneFeats(btp);
}

void GetSeqFeat(BiotablePtr btp)

{
	GeneStructPtr gsp;
	Int2 count, genesize, index=0, size;

	for (count=(btp->count-1); count>=0; count--)
	{
		flat2asn_delete_locus_user_string();
		flat2asn_install_locus_user_string(btp->locus[count]);
		flat2asn_delete_accession_user_string();
		flat2asn_install_accession_user_string(btp->access[count]);
		OrganizeSeqFeat(btp, count);
	}
	
	OrganizeSeqFeat(btp, -1);

	for (count=0; count<(btp->count); count++)
	{
		size = btp->sfpListsize[count];
		 /* Need at least one allocated for source and misc feats */
		size++;
		btp->gspp[count] = (GeneStructPtr PNTR) MemNew(size*sizeof(GeneStructPtr));
		btp->nspp[count] = (NoteStructPtr PNTR) MemNew(size*sizeof(NoteStructPtr));
		for (index=0; index<size; index++)
		{
			btp->gspp[count][index] =
				GeneStructNew(btp->gspp[count][index]);
			btp->nspp[count][index] =
				NoteStructNew(btp->nspp[count][index]);
		}
		gsp = btp->gspp[count][0]; 
		genesize = btp->sfpGenesize[count];
		gsp->genes_matched = (Int2Ptr) MemNew(genesize*sizeof(Int2));
		for (index=0; index<genesize; index++)
			gsp->genes_matched[index] = -1;
		gsp->matchlist_size = genesize;
	}
}

Int2 GetNumOfFeats(BiotablePtr btp, Int2 count)

{
	GeneStructPtr gsp = btp->gspp[count][0];
	Int2 gene_size, index, total=btp->sfpListsize[count];

	gene_size=gsp->matchlist_size;

	for (index=0; index<gene_size; index++)
	{/* How many orphan genes will go out as misc_feats? */
		if (gsp->genes_matched[index] == -1)
			total++;
	}

	return total;
}

/***********************************************************************
*void PrintSeqBlk (BiotablePtr btp, Int2 count)
*
*       This function prints out a block of the sequence (at most
*       of size SEQ_BLK_SIZE).
*       After the last sequence block, the terminator is printed also.
***********************************************************************/
 
void PrintSeqBlk (BiotablePtr btp, Int2 count)
 
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
        PrintSequence(btp, count, start, stop);
        if (stop == -1)
                PrintTerminator(btp);
}

/***********************************************************************
*void PrintEPSeqBlk (BiotablePtr btp, Int2 count)
*
*       This function prints out a block of the sequence (at most
*       of size SEQ_BLK_SIZE).
*       After the last sequence block, the terminator is printed also.
***********************************************************************/
 
void PrintEPSeqBlk (BiotablePtr btp, Int2 count)
 
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
        PrintEPSequence(btp, count, start, stop);
        if (stop == -1)
                PrintTerminator(btp);
}

void OrganizeSeqFeat(BiotablePtr btp, Int2 count)

{
	BioseqContextPtr bcp;
	BioseqPtr bsp;
	Boolean valid_feat;
	ImpFeatPtr ifp;
	SeqFeatPtr sfp = NULL;
	SeqIdPtr xid;
	static SortStructPtr Commlist;
	static SortStructPtr Genelist;
	static SortStructPtr List;
	static SortStructPtr Orglist;
	static SortStructPtr Siteslist;
	static SortStructPtr Sourcelist;
	static SortStructPtr Xreflist;
	Int2 currentsize=0, currentcomm=0, currentgene=0, currentorg=0, status;
	Int2 currentsource=0, currentsites=0, currentxref=0;
	static Int2 totalsize, totalcomm, totalgene, totalorg, totalsites, 
	totalsource, totalxref;
	
	if (count == -1)	/* perform cleanup, deallocate memory */
	{
		if (totalcomm > 0)
			Commlist = MemFree(Commlist);
		if (totalgene > 0)
			Genelist = MemFree(Genelist);
		if (totalsize > 0)
			List = MemFree(List);
		if (totalorg > 0)
			Orglist = MemFree(Orglist);
		if (totalsites > 0)
			Siteslist = MemFree(Siteslist);
		if (totalsource > 0)
			Sourcelist = MemFree(Sourcelist);
		if (totalxref > 0)
			Xreflist = MemFree(Xreflist);
		return;
	}

/* Change the next line to more standardized form????????? 	*/
	if (count == (btp->count-1))
	{
		totalcomm=0;
		totalgene=0;
		totalorg=0;
		totalsize=0;
		totalsites=0;
		totalsource=0;
		totalxref=0;
	}

	bcp = btp->bcp[count];

	sfp=BioseqContextGetSeqFeat(bcp, 0, sfp, NULL, 2);

	while (sfp !=NULL)
	{
		switch (sfp->data.choice) {
		case SEQFEAT_GENE: 
			if (btp->format != GENPEPT)
			{/* Do intital check only for nucleic acids */
				valid_feat = CheckNAFeat(btp, count, sfp);
				if (! valid_feat && ASN2FF_VALIDATE_FEATURES == TRUE)
					break;
			}
			if (currentgene == totalgene)
			{
			   if (totalgene == 0)
		    	   Genelist = EnlargeSortList(NULL, &(totalgene), 5);
			   else
		    	   Genelist = EnlargeSortList(Genelist, &(totalgene), 5);
			}
			currentgene = CpSfpToList(Genelist, sfp, btp, currentgene, count);
			break;
		case SEQFEAT_ORG: 
			if (currentorg == totalorg)
			{
			   if (totalorg == 0)
		    	   Orglist = EnlargeSortList(NULL, &(totalorg), 2);
			   else
		    	   Orglist = EnlargeSortList(Orglist, &(totalorg), 2);
			}
			currentorg = CpSfpToList(Orglist, sfp, btp, currentorg, count);
			break;
		case SEQFEAT_PUB: /* Pubs are already captured by "StorePubInfo". */
			break;
		case SEQFEAT_IMP: /* This case must be before the generic case */
			ifp = (ImpFeatPtr) sfp->data.value.ptrvalue;
			if (StringCmp(ifp->key, "source") == 0)
			{ /* Capture only the first source feat that covers the
			entire entry, the others go among the generic features.*/
				if (currentsource == 0)
				{
					bsp = btp->bsp[count];
					if (bsp->length != -1 &&
						bsp->length == SeqLocLen(sfp->location))
					{
						if (currentsource == totalsource)
						{
		    	   				Sourcelist = 
							   EnlargeSortList(NULL, &(totalsource), 1);
					 	}
						currentsource =
						CpSfpToList(Sourcelist, sfp, btp, currentsource, count);
						break;
					}
				}
			} /* if "Site-ref" has a pub, put out info as a pub */
			else if (StringCmp(ifp->key, "Site-ref") == 0 && 
				sfp->cit == NULL)
			{
				if (currentsites == totalsites)
				{
			   	if (totalsites == 0)
		    	   	Siteslist = 
				   EnlargeSortList(NULL, &(totalsites), 5);
			  	 else
		    	  	 Siteslist = 
				   EnlargeSortList(Siteslist, &(totalsites), 5);
				}
				currentsites = 
				CpSfpToList(Siteslist, sfp, btp, currentsites, count);
				break;
			} /* Check to see if this was already put out as a pub*/
			else if (StringCmp(ifp->key, "Site-ref") == 0 && 
				sfp->cit != NULL)
			{
				break;
			} /* If none of the above is true, execute generic. */
		default:
	/* Look to see if an Xref goes out as a SeqFeat or as a 2nd accession */
			if (sfp->data.choice == SEQFEAT_SEQ)
			{
				xid=CheckXrefFeat(btp->bsp[count], sfp);
				if (xid == NULL)
				{
					if (currentxref == totalxref)
					{
			   			if (totalxref == 0)
		    	  			Xreflist = EnlargeSortList(NULL, &(totalxref), (Int2) 5);
			  			else
		    	  			Xreflist = EnlargeSortList(Xreflist, &(totalxref), (Int2) 5);
					}
					currentxref = CpSfpToList(Xreflist, sfp, btp, currentxref, count);
					break;

				}
			}
			else if (sfp->data.choice == SEQFEAT_COMMENT)
			{
				bsp = btp->bsp[count];
				if (bsp->length != -1 &&
					bsp->length == SeqLocLen(sfp->location))
				{ /* The following assures a valid comment */
					if (sfp->comment == NULL ||
						StringLen(sfp->comment) == 0)
						break;
					if (currentcomm == totalcomm)
					{
			   			if (totalcomm == 0)
		    	  			Commlist = EnlargeSortList(NULL, &(totalcomm), (Int2) 1);
			  			else
		    	  			Commlist = EnlargeSortList(Commlist, &(totalcomm), (Int2) 1);
				 	}
					currentcomm =
					CpSfpToList(Commlist, sfp, btp, currentcomm, count);
					break;
				}
			}
			if (btp->format != GENPEPT)
			{/* Do intital check only for nucleic acids */
				valid_feat = CheckNAFeat(btp, count, sfp);
				if (! valid_feat && ASN2FF_VALIDATE_FEATURES == TRUE)
					break;
			}
			if (currentsize == totalsize)
			{
			   if (totalsize == 0)
		    	   List = EnlargeSortList(NULL, &(totalsize), (Int2) 5);
			   else
		    	   List = EnlargeSortList(List, &(totalsize), (Int2) 5);
			}
			if (count != (btp->count-1))
			{
			   status = RemoveRedundantFeats(btp, count, sfp);
			   if (status == 1)
					break;
			}
			currentsize = CpSfpToList(List, sfp, btp, currentsize, count);
			break;
		}
		sfp=BioseqContextGetSeqFeat(bcp, 0, sfp, NULL, 2);
	}
	if (currentsize > 0)
		HeapSort((VoidPtr) List, (size_t) currentsize, sizeof(SortStruct), CompareSfpForHeap);
	if (currentcomm > 0)
		HeapSort((VoidPtr) Commlist, (size_t) currentcomm, sizeof(SortStruct), CompareSfpForHeap);
	if (currentgene > 0)
		HeapSort((VoidPtr) Genelist, (size_t) currentgene, sizeof(SortStruct), CompareSfpForHeap);
		/* Not sorted so results will agree with Karl's
	if (currentorg > 0)
		HeapSort((VoidPtr) Orglist, (size_t) currentorg, sizeof(SortStruct), CompareSfpForHeap);
	*/
	if (currentsites > 0)
		HeapSort((VoidPtr) Siteslist, (size_t) currentsites, sizeof(SortStruct), CompareSfpForHeap);
	if (currentsource > 0)
		HeapSort((VoidPtr) Sourcelist, (size_t) currentsource, sizeof(SortStruct), CompareSfpForHeap);
		/* Xref's are intentionally not sorted!! */
	CpListToBtp(List, btp->sfpList, currentsize, btp->sfpListsize, count);
	CpListToBtp(Commlist, btp->sfpCommlist, currentcomm, btp->sfpCommsize, count);
	CpListToBtp(Genelist, btp->sfpGenelist, currentgene, btp->sfpGenesize, count);
	CpListToBtp(Orglist, btp->sfpOrglist, currentorg, btp->sfpOrgsize, count);
	CpListToBtp(Siteslist, btp->sfpSitelist, currentsites, btp->sfpSitesize, count);
	CpListToBtp(Sourcelist, btp->sfpSourcelist, currentsource, btp->sfpSourcesize, count);
	CpListToBtp(Xreflist, btp->sfpXreflist, currentxref, btp->sfpXrefsize, count);
}	/* OrganizeSeqFeat */

/***************************************************************************
*	Int2 MatchNAGeneToFeat (BiotablePtr btp, Int2 count, SeqFeatPtr sfp, Int2 ext_index)
*
*	btp: the BiotablePtr,
*	count: sequence number of the raw sequence in the btp,
*	sfp: SeqFeatPtr that is to be compared with the list of genes.
*
*	This function assigns genes to the given sfp by comparing locations.
*	First, the location of the sfp is found (slp), then this location is
*	compared with the locations ("gene_loc" locally designates 
*	a single gene location) of the list of genes (gene_list) using 
*	"SeqLocCompare".  If SeqLocCompare (gene_loc, slp) returns "3", there 
*	is an exact match and this gene is judged "the best" match to the sfp.`
*	If SeqLocCompare(gene_loc, slp) returns "2", slp is contained within 
*	gene_loc and the difference between the two features (in bp) is 
*	measured.  The index of the best gene is stored as "best_gene"
*	and that information is stored in the GeneStructPtr (gsp) by
*	GetGeneRefInfo.  The return value is the index of the best gene,
*	unless none is found, then -1 is returned.
***************************************************************************/	
				
Int2 MatchAAGeneToFeat (BiotablePtr btp, Int2 count, SeqFeatPtr sfp, Int2 ext_index)

{

	GeneStructPtr gsp=btp->gspp[count][ext_index], gene_gsp=btp->gspp[count][0];
	GeneRefPtr grp;
	Int2 best_gene = -1, index, listsize=btp->sfpGenesize[count], status;
	Int4 feat_length;
	NoteStructPtr nsp;
	SeqFeatPtr PNTR gene_list=btp->sfpGenelist[count];
	SeqLocPtr gene_loc, slp=sfp->location;

	feat_length = SeqLocLen(slp);
	if (feat_length == -1)
		return best_gene;	 /*SeqLocLen failed on CDS location */

/* Only look on the protein or on the CDS for genpept.		*/
	if (sfp->data.choice != 3 && sfp->data.choice != 4)
		return best_gene;

	for (index=0; index<listsize; index++)
	{
		if (gene_list[index] == NULL)
			break;	/* Only NULL pointers follow	*/ 

		gene_loc = gene_list[index]->location;
		status = SeqLocCompare(gene_loc, slp);
		if (status == 3)
		{
			best_gene = index;
			break;
		}
	}

	if (best_gene == -1)	/*no gene found that completely contains CDS*/
		return best_gene;
	else
	{ /*	The next line differs from MatchNAGeneToFeat.	*/
		gene_gsp->genes_matched[best_gene]++;
		grp = gene_list[best_gene]->data.value.ptrvalue;
	}

	nsp = btp->nspp[count][ext_index];
	GetGeneRefInfo(gsp, nsp, grp);

	return best_gene;
}	/* MatchAAGeneToFeat */

				
Int2 MatchNAGeneToFeat (BiotablePtr btp, Int2 count, SeqFeatPtr sfp, Int2 ext_index)

{

	Boolean bind_to_feat=FALSE;
	GeneStructPtr gsp=btp->gspp[count][ext_index], gene_gsp=btp->gspp[count][0];
	GeneRefPtr grp;
	ImpFeatPtr ifp;
	Int2 best_gene = -1, index, listsize=btp->sfpGenesize[count];
	Int4 diff_lowest, diff_current;
	NoteStructPtr nsp;
	SeqFeatPtr PNTR gene_list=btp->sfpGenelist[count];
	SeqLocPtr slp=sfp->location;

	if (btp->gene_binding == LIMITED)
	{
		if (sfp->data.choice == 3 || sfp->data.choice == 5)
			bind_to_feat = TRUE;	
		else if (sfp->data.choice == 8)
		{
			ifp = (ImpFeatPtr) sfp->data.value.ptrvalue;
			if (StringCmp(ifp->key, "CDS") == 0)
				bind_to_feat = TRUE;	
		}
	}
	else
		bind_to_feat = TRUE;	

	if (bind_to_feat == FALSE)
		return best_gene;	/* Don't bind to this feature */

	/* not used by new function
	if ((feat_length = SeqLocLen(slp)) == -1)
		return best_gene; */	 /*SeqLocLen failed on CDS location */
	diff_lowest = -1;

	for (index=0; index<listsize; index++)
	{
		if (gene_list[index] == NULL)
			break;	/* Only NULL pointers follow	*/ 
		
		if (btp->gene_binding == LIMITED &&
			gene_gsp->genes_matched[index] != -1)
			continue;

		/***** old version of gene match ******

		gene_loc = gene_list[index]->location;
		status = SeqLocCompare(gene_loc, slp);
		if (status == 3)
		{
			best_gene = index;
			diff_lowest = 0;
			break;
		}
		else if (status == 2)
		{
			gene_length = SeqLocLen(gene_loc);
			feat_length = SeqLocLen(slp);
			diff_current = gene_length - feat_length; 
			if (diff_lowest == -1 || diff_current < diff_lowest)
			{
				diff_lowest = diff_current;
				best_gene = index;
			}
		}

		***************************************/

		/***** new version of gene match J.O. ******/

		diff_current = SeqLocAinB(slp, gene_list[index]->location);
		if (! diff_current)   /* perfect match */
		{
			best_gene = index;
			break;
		}
		else if (diff_current > 0)
		{
			if ((diff_lowest == -1) || (diff_current < diff_lowest))
			{
				diff_lowest = diff_current;
				best_gene = index;
			}
		}

		/***************************************/
	}

	if (best_gene == -1)	/*no gene found that completely contains CDS*/
		return best_gene;
	else
	{
		grp = gene_list[best_gene]->data.value.ptrvalue;
		gene_gsp->genes_matched[best_gene]++;
	}

	nsp = btp->nspp[count][ext_index];
	GetGeneRefInfo(gsp, nsp, grp);

	return best_gene;
}	/* MatchNAGeneToFeat */

Int2 CheckGeneToFeatOverlap (BiotablePtr btp, Int2 count, SeqLocPtr gene_loc, Int2 PNTR best_feat)

{
	Boolean bind_to_feat=FALSE;
	ImpFeatPtr ifp;
	Int2 index_f, listsize = btp->sfpListsize[count], num_of_matches=0, start_index, status;
	static Int4 diff_current, diff_lowest;
	Int4 gene_length, feat_length;
	SeqFeatPtr sfp, PNTR sfp_list=btp->sfpList[count];
	SeqLocPtr slp;

	if (*best_feat == -1)
	{
		start_index=0;
		diff_lowest = -1;
	}
	else 
		start_index=(*best_feat)+1;

	for (index_f=start_index; index_f<listsize; index_f++)
	{
		/* Gene Struct already has gene info for this feature. */
		if (sfp_list[index_f] == NULL)
			break;	/* Only NULL pointers follow	*/ 
	
		sfp=sfp_list[index_f];
		if ((GBQualPresent("gene", sfp->qual)) == TRUE)
			continue;     /* Only one gene qualifier per feature */
		slp = sfp->location;
		if (btp->gene_binding == LIMITED)
		{	/* only bind certain to certain feats if LIMITED */
			bind_to_feat = FALSE;
			if (sfp->data.choice == 3 || sfp->data.choice == 5)
				bind_to_feat = TRUE;	
			else if (sfp->data.choice == 8)
			{
				ifp = (ImpFeatPtr) sfp->data.value.ptrvalue;
				if (StringCmp(ifp->key, "CDS") == 0)
					bind_to_feat = TRUE;	
			}
		}
		else
		{	/* Check that a gene is legal here	*/
			if (GBQualNameValid("gene") != -1)
				bind_to_feat = TRUE;
		}
	
		if (bind_to_feat == FALSE)
			continue;	/* Don't bind to this feature */

		if ((feat_length = SeqLocLen(slp)) == -1)
			continue;	 /*SeqLocLen failed on CDS location */
		status = SeqLocCompare(gene_loc, slp);
		if (status == 2 || status == 3)
		{
			gene_length = SeqLocLen(gene_loc);
			feat_length = SeqLocLen(slp);
			diff_current = gene_length - feat_length; 
			if (diff_lowest == -1 || diff_current < diff_lowest)
			{
				*best_feat = index_f;
				diff_lowest = diff_current;
				num_of_matches=1;
			}
			else if (diff_current == diff_lowest)
			{
				if (num_of_matches == 0)
					*best_feat = index_f;
				num_of_matches++;
			}
		}
	}
	if (num_of_matches == 0)
		*best_feat = -1;
	return num_of_matches;
}

/****************************************************************************
*	void CompareGenesToQuals (BiotablePtr btp, Int2 count, Int2 ext_index)
*
*	btp: BiotablePtr of interest,
*	count: index of sequence among the btp entries.
*	
*	Compares the contents of the GeneStructPtr (gsp), to the 
*	unassigned ("orphan") genes.  At present (2/7/94), a very loose
*	criteria is used to determine if a gene is already "covered";
*	only a check is done as to whether the gene name has been mentioned
*	in a qualifier somewhere and no location comparison is done.
*	If a gene is "covered" (i.e., not "orphaned") the genes_matched list
*	for that gene, is increased from "-1".
*****************************************************************************/

void CompareGenesToQuals (BiotablePtr btp, Int2 count, Int2 ext_index)

{
	GeneRefPtr grp;
	GeneStructPtr gsp=btp->gspp[count][ext_index], gene_gsp=btp->gspp[count][0];
	Int2 index, index1, listsize=btp->sfpGenesize[count];
	NoteStructPtr nsp=btp->nspp[count][ext_index];
	SeqFeatPtr PNTR gene_list=btp->sfpGenelist[count];

	for (index=0; index<listsize; index++)
	{
		if (gene_gsp->genes_matched[index] == -1)
		{
			grp = (GeneRefPtr) gene_list[index]->data.value.ptrvalue;
			if (grp->locus && 
				CompareStringWithGsp (gsp, grp->locus) == 0)
			{
				gene_gsp->genes_matched[index]++;
			}
			else if (grp->maploc && 
				CompareStringWithGsp (gsp, grp->maploc) == 0)
			{
				gene_gsp->genes_matched[index]++;
			}
			else 
			{
				for (index1=0; index1<nsp->note_index; index1++)
					if (grp->locus &&
						GeneStringCmp(grp->locus, nsp->note[index1]) == 0)
						gene_gsp->genes_matched[index]++;
			}
		}
	}
}	/* CompareGenesToQuals */

/***************************************************************************
* void CompareGenesToSourceQuals (BiotablePtr btp, Int2 count)
*
*	This function compares the genes to the quals of the 
*	ImpFeat of type "source".
**************************************************************************/
void CompareGenesToSourceQuals (BiotablePtr btp, Int2 count)

{
	GBQualPtr qual;
	GeneRefPtr grp;
	GeneStructPtr gene_gsp=btp->gspp[count][0];
	Int2 index, listsize=btp->sfpGenesize[count];
	SeqFeatPtr sfp, PNTR gene_list=btp->sfpGenelist[count];

	if (btp->sfpSourcesize[count] != 0)
	{
		sfp = btp->sfpSourcelist[count][0];
		for (index=0; index<listsize; index++)
		{
			if (gene_gsp->genes_matched[index] == -1)
			{
				grp = (GeneRefPtr) gene_list[index]->data.value.ptrvalue;
				for (qual=sfp->qual; qual; qual=qual->next)
				{
					if (grp->locus && 
						GeneStringCmp(grp->locus, qual->val) == 0)
					{
						gene_gsp->genes_matched[index]++;
						break;
					}
					if (grp->allele &&
						GeneStringCmp(grp->allele, qual->val) == 0)
					{
						gene_gsp->genes_matched[index]++;
						break;
					}
					if (grp->maploc &&
						GeneStringCmp(grp->maploc, qual->val) == 0)
					{
						gene_gsp->genes_matched[index]++;
						break;
					}
					if (grp->desc &&
						GeneStringCmp(grp->desc, qual->val) == 0)
					{
						gene_gsp->genes_matched[index]++;
						break;
					}
				}
			}
		}
	}
}

void MarkRedundantGeneFeats(BiotablePtr btp)

{
	GeneStructPtr gsp_mid, gsp_out;
	Int2 index=0, outer_index, mid_index, inner_index;
	SeqFeatPtr sfp_last, sfp;

	for (outer_index=(btp->count)-1; outer_index>=0; outer_index--)
	{
		for (mid_index=outer_index-1; mid_index>=0; mid_index--)
		{
			gsp_mid=btp->gspp[mid_index][0];
			gsp_out=btp->gspp[outer_index][0];
			for (inner_index=0; inner_index<gsp_mid->matchlist_size; inner_index++)
			{
				sfp=btp->sfpGenelist[mid_index][inner_index];
				index=0;
				while ((sfp_last=btp->sfpGenelist[outer_index][index]) != NULL)
				{
					if (sfp_last == sfp)
					{
						if (gsp_mid->genes_matched[inner_index] != -1 &&
							gsp_out->genes_matched[index] == -1)
							gsp_out->genes_matched[index]++;
						else if (gsp_mid->genes_matched[inner_index] == -1 &&
							gsp_out->genes_matched[index] != -1)
							gsp_mid->genes_matched[inner_index]++;
						else if (gsp_mid->genes_matched[inner_index] == -1 &&
							gsp_out->genes_matched[index] == -1)
						{
							gsp_mid->genes_matched[inner_index]++;
						}
					}
					index++;
				}
			}
		}
	}

	return;
}

void PrintPubsByNumber (BiotablePtr btp, Int2 count)

{
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
		psp = vnp1->data.ptrvalue;
		if (btp->format == EMBL)
			EMBL_PrintPubs(btp, count, psp);
		else
			GB_PrintPubs(btp, count, psp);
	}
}

void PrintFeatHeader (BiotablePtr btp, Int2 count)

{
	if (btp->format == EMBL)
	{
		StartPrint(btp, 5, 0, ASN2FF_EMBL_MAX, "FH");
		AddString(btp, "Key");
		TabToColumn(btp, 22);
		AddString(btp, "Location/Qualifiers");
		NewContLine(btp);
	}
	else
	{
		StartPrint(btp, 0, 0, ASN2FF_GB_MAX, NULL);
		AddString(btp, "FEATURES");
		TabToColumn(btp, 22);
		AddString(btp, "Location/Qualifiers");
	}
		EndPrint(btp);
}


