/* $Revision: 1.60 $ */ 
/*************************************
*
* $Log: asn2ff3.c,v $
 * Revision 1.60  1995/05/19  21:25:06  kans
 * no longer fetches CDS protein product causing Entrez disc swap
 *
 * Revision 1.59  1995/05/15  21:46:05  ostell
 * added Log line
 *
*
**************************************/
#include "asn2ffp.h"
#include "parsegb.h"


Int2 ConvertToNAImpFeat PROTO ((BiotablePtr btp, Int2 count, SeqFeatPtr sfp_in, SeqFeatPtr PNTR sfp_out, Int2 ext_index));
Int2 ConvertToAAImpFeat PROTO ((BiotablePtr btp, Int2 count, SeqFeatPtr sfp_in, SeqFeatPtr PNTR sfp_out, Int2 ext_index));
void ComposetRNANote PROTO ((BiotablePtr btp, tRNAPtr trna, Int2 count, Int2 ext_index));
void ComposeCodeBreakQuals PROTO ((BiotablePtr btp, Int2 count, Int2 ext_index, SeqFeatPtr sfp_out));
CharPtr Get3LetterSymbol PROTO ((Uint1 seq_code, SeqCodeTablePtr table, Uint1 residue, Boolean error_msgs));
CharPtr SeqCodeNameGet PROTO ((SeqCodeTablePtr table, Uint1 residue, Boolean error_msgs));
Int2 ValidateAAImpFeat PROTO ((BiotablePtr btp, Int2 count, SeqFeatPtr sfp, Boolean use_product));
Int2 ValidateNAImpFeat PROTO ((BiotablePtr btp, Int2 count, SeqFeatPtr sfp));
void LookForPartialImpFeat PROTO ((SeqFeatPtr sfp, Boolean use_product));
void AddProteinQuals PROTO ((SeqFeatPtr sfp, SeqFeatPtr sfp_out, NoteStructPtr nsp));
void GetGeneticCode PROTO ((CharPtr ptr, SeqFeatPtr sfp));
void GetNonGeneQuals PROTO ((SeqFeatPtr sfp_in, SeqFeatPtr sfp_out, BiotablePtr btp, Int2 count, Int2 index));
void ComposeGBQuals PROTO ((SeqFeatPtr sfp_out, BiotablePtr btp, Int2 count, Int2 index));
CharPtr ComposeNoteFromNoteStruct PROTO ((NoteStructPtr nsp, GeneStructPtr gsp));
void AddGINumToNote PROTO ((BiotablePtr btp_na, Int2 count, SeqFeatPtr sfp_out, Int2 index));
Int2 MakeGBSelectNote PROTO ((CharPtr ptr, SeqFeatPtr sfp));
void GetProtRefInfo PROTO ((GeneStructPtr gsp, NoteStructPtr nsp, ProtRefPtr prp));
void GetProtRefComment PROTO ((SeqFeatPtr sfp, BiotablePtr btp, Int2 count, NoteStructPtr nsp));
void GetProductGeneInfo PROTO ((SeqFeatPtr sfp_in, BiotablePtr btp, Int2 count, Int2 ext_index));
Int2 MiscFeatOrphanGenes PROTO ((BiotablePtr btp, Int2 count, SeqFeatPtr sfp, Int2 index));
Int2 CheckForQual PROTO ((GBQualPtr gbqual, CharPtr string_q, CharPtr string_v));
GBQualPtr AddModifsToGBQual PROTO ((BiotablePtr btp, Int2 count, GBQualPtr gbqual));
GBQualPtr AddOrgRefModToGBQual PROTO ((OrgRefPtr orp, GBQualPtr gbqual));
Int2 CheckForEqualSign PROTO ((CharPtr qual));
CharPtr GetProductFromCDS PROTO ((ValNodePtr product, ValNodePtr location, Int4 length));
void PrepareSourceFeatQuals PROTO ((SeqFeatPtr sfp_in, SeqFeatPtr sfp_out, BiotablePtr btp, Int2 count, Boolean add_modif));
void PutTranslationLast PROTO ((SeqFeatPtr sfp));
static Int2 CheckForExtraChars PROTO ((CharPtr note));
CharPtr GettRNAaa PROTO ((tRNAPtr trna, Boolean error_messages));
void DotRNAQuals PROTO ((BiotablePtr btp, SeqFeatPtr sfp_in, SeqFeatPtr sfp_out, Int2 count, Int2 ext_index));
GBQualPtr AddBioSourceToGBQual PROTO((BioSourcePtr biosp, GBQualPtr gbqual));

typedef struct {
	CharPtr name;
	Uint1   num;
} ORGMOD;

#define num_subtype 22
CharPtr subtype[num_subtype] = {
"chromosome", "map", "clone", "sub_clone", "haplotype", "genotype", "sex",
"cell_line", "cell_type", "tissue_type", "clone_lib", "dev_stage", 
"frequency", "germline", "rearranged", "lab_host", "pop_variant",
"tissue_lib", "plasmid", "transposon", "insertion_seq", "plastid"};

#define num_genome 14
static CharPtr genome[num_genome] = {"unknown", "genomic", "cloroplast", "chromoplast", "kinetoplast", "mitochondrion", "plastid", "macronuclear",
"extrachrom", "plasmid", "transposon", "insertion_seq", "cyanelle", "proviral"};

#define num_biomol 7
static CharPtr biomol[num_biomol] = {"genomic", "RNA", "mRNA", "rRNA",
"tRNA", "snRNA", "scRNA"};

ORGMOD orgmod_subtype[8] = {
	{ "strain", 2 }, {"sub_strain", 3}, {"variety", 6}, {"cultivar", 10},
	{"isolate", 17}, {"specific_host", 21}, {"sub_species", 255}, { NULL, 0 }
};


void PrintSourceFeat(BiotablePtr btp, Int2 count)

{
	BioseqPtr bsp=btp->bsp[count];
	Char location[10];
	ImpFeatPtr ifp;
	Int2 status = -1, mol = -1, seqmol = -1;
	NoteStructPtr nsp=btp->source_notes;
	OrgRefPtr orp=NULL;
	SeqFeatPtr sfp_in, sfp_out=NULL;
	SeqIntPtr sip;
	SeqLocPtr slp;
	ValNodePtr vnp=NULL;
	BioSourcePtr biosp = NULL;
	MolInfoPtr mfp;
	

	sprintf(location, "1..%ld", (long) (bsp->length));

        sfp_out = SeqFeatNew();
	sfp_out->data.choice = 8;
	sfp_out->qual = NULL;   
	ifp = sfp_out->data.value.ptrvalue = ImpFeatNew();
	ifp->key = StringSave("source");
	ifp->loc = StringSave(location);
	slp = (SeqLocPtr) ValNodeNew(NULL);
	slp->choice = SEQLOC_INT;
	sip = SeqIntNew();
	sip->from = 0;
	sip->to = (bsp->length)-1;
	sip->id = SeqIdDup(bsp->id);
	slp->data.ptrvalue = sip;
	sfp_out->location = slp;

	flat2asn_install_feature_user_string("source", ifp->loc);
	if (btp->sfpSourcesize[count] != 0)
	{	/* ImpFeat with key "source" was present, use. */
		sfp_in = btp->sfpSourcelist[count][0];
		if (sfp_out->qual != NULL)
			sfp_out->qual = GBQualFree(sfp_out->qual);
		NoteStructReset(nsp);
		PrepareSourceFeatQuals(sfp_in, sfp_out, btp, count, FALSE);
		status = ValidateNAImpFeat(btp, count, sfp_out);
		if (status < 0)
		{ /* source feat is probably missing organism name, add
		and try again.  Don't delete old quals! */
			if ((vnp=BioseqContextGetSeqDescr(btp->bcp[count], Seq_descr_org, NULL, NULL)) != NULL) 
				orp = (OrgRefPtr) vnp->data.ptrvalue;
			else if (btp->sfpOrgsize[count] != 0)
				orp = (OrgRefPtr) (btp->sfpOrglist[count][0])->data.value.ptrvalue;
			if (orp)
			{
				if (orp->orgname) {
					/*	ErrPostEx() */
				}
				if (orp->taxname)
				{
					sfp_out->qual = AddGBQual(sfp_out->qual, "organism", orp->taxname);
					if (orp->common)
						CpNoteToCharPtrStack(nsp, NULL, orp->common);
				}
				else if (orp->common)
				{
					if (StrStr(orp->common, "virus") ||
					    StrStr(orp->common, "Virus") ||
					    StrStr(orp->common, "phage") ||
					    StrStr(orp->common, "Phage") ||
					    StrStr(orp->common, "viroid") ||
					    StrStr(orp->common, "Viroid"))
					{
						sfp_out->qual = AddGBQual(sfp_out->qual, "organism", orp->common);
					}
				}
			}
			status = ValidateNAImpFeat(btp, count, sfp_out);
		}
	}
/* keep both new and old style for now */
/* look for Biosource first */
	if (status < 0)
	{
		if ((vnp=BioseqContextGetSeqDescr(btp->bcp[count], Seq_descr_source, 
														NULL, NULL)) != NULL) {
			biosp = vnp->data.ptrvalue;
			orp = (OrgRefPtr) biosp->org;	
		} else {
			orp = NULL;
		}
		if (orp) {
			NoteStructReset(nsp);
			if (sfp_out->qual != NULL)
				sfp_out->qual = GBQualFree(sfp_out->qual);
			if (orp->taxname) {
				sfp_out->qual = AddGBQual(sfp_out->qual, "organism", orp->taxname);
				if (orp->common)
					CpNoteToCharPtrStack(nsp, NULL, orp->common);
			} else if (orp->common) {
				if (StrStr(orp->common, "virus") ||
				    StrStr(orp->common, "Virus") ||
				    StrStr(orp->common, "phage") ||
				    StrStr(orp->common, "Phage") ||
				    StrStr(orp->common, "viroid") ||
				    StrStr(orp->common, "Viroid")) {
					sfp_out->qual = AddGBQual(sfp_out->qual, "organism",
															 orp->common);
				}
			}
			sfp_out->qual = AddBioSourceToGBQual(biosp, sfp_out->qual);
		}
		if ((vnp=BioseqContextGetSeqDescr(btp->bcp[count], 
					Seq_descr_molinfo, NULL, NULL)) != NULL) { 
			mfp = vnp->data.ptrvalue;
			if (mfp) {
				mol = mfp->biomol;
			}
		}
		switch(bsp->mol) {
			case Seq_mol_rna:
				if (mol == 1) {  /* genomic */
					sfp_out->qual = AddGBQual(sfp_out->qual,
						 "sequenced_mol", "cDNA to genomic RNA");
				} else if (mol == 3) {
					sfp_out->qual = AddGBQual(sfp_out->qual,
						 "sequenced_mol", "cDNA to mRNA");
				}
				break;
			case Seq_mol_dna:
				if (mol == 1) {  /* genomic */
					sfp_out->qual = AddGBQual(sfp_out->qual,
						 "sequenced_mol", "DNA");
				}
			default:
				break;
		}
		PrepareSourceFeatQuals(NULL, sfp_out, btp, count, TRUE);
		status = ValidateNAImpFeat(btp, count, sfp_out);
	}
/* old style: look for org-ref */
	if (status < 0)
	{
		if ((vnp=BioseqContextGetSeqDescr(btp->bcp[count], Seq_descr_org, NULL, NULL)) != NULL) 
			orp = (OrgRefPtr) vnp->data.ptrvalue;
		else if (btp->sfpOrgsize[count] != 0)
			orp = (OrgRefPtr) (btp->sfpOrglist[count][0])->data.value.ptrvalue;
		if (orp)
		{
			NoteStructReset(nsp);
			if (sfp_out->qual != NULL)
				sfp_out->qual = GBQualFree(sfp_out->qual);
			if (orp->taxname)
			{
				sfp_out->qual = AddGBQual(sfp_out->qual, "organism", orp->taxname);
				if (orp->common)
					CpNoteToCharPtrStack(nsp, NULL, orp->common);
			}
			else if (orp->common)
			{
				if (StrStr(orp->common, "virus") ||
				    StrStr(orp->common, "Virus") ||
				    StrStr(orp->common, "phage") ||
				    StrStr(orp->common, "Phage") ||
				    StrStr(orp->common, "viroid") ||
				    StrStr(orp->common, "Viroid"))
				{
					sfp_out->qual = AddGBQual(sfp_out->qual, "organism", orp->common);
				}
			}
			sfp_out->qual = AddOrgRefModToGBQual(orp, sfp_out->qual);
			PrepareSourceFeatQuals(NULL, sfp_out, btp, count, TRUE);
			status = ValidateNAImpFeat(btp, count, sfp_out);
		}
	}
/* ----------Organism not found -------------*/
	if (status < 0)
	{
		if (sfp_out->qual)
			sfp_out->qual = GBQualFree(sfp_out->qual);
		sfp_out->qual = AddGBQual(sfp_out->qual, "organism", "unknown");
		NoteStructReset(nsp);
		if (orp && orp->common)
			CpNoteToCharPtrStack(nsp, NULL, orp->common);
/*try new first */
		if (biosp) {
			sfp_out->qual = AddBioSourceToGBQual(biosp, sfp_out->qual);
		}
/* try old  then */
		sfp_out->qual = AddOrgRefModToGBQual(orp, sfp_out->qual);
		PrepareSourceFeatQuals(NULL, sfp_out, btp, count, TRUE);
		status = ValidateNAImpFeat(btp, count, sfp_out);
	}	
	flat2asn_delete_feature_user_string();

	if (status >= 0 || ASN2FF_VALIDATE_FEATURES == FALSE)
	{
		PrintImpFeat(btp, count, sfp_out);
		sfp_out->comment = NULL;
		sfp_out->product = NULL;
		SeqFeatFree(sfp_out);
		return;
	}
	else
	{
		sfp_out->comment = NULL;
		sfp_out->product = NULL;
		SeqFeatFree(sfp_out);
		return;
	}
}	/* PrintSourceFeat */

void PrintNAFeatByNumber (BiotablePtr btp, Int2 count)

{
	
	Boolean loc_ok;
	ByteStorePtr byte_sp;
	Char genetic_code[3];
	CharPtr protein_seq, ptr=NULL;
	ImpFeatPtr ifp;
	SeqFeatPtr sfp_in, sfp_out=NULL, sfp_cds;
	Int2 status, total_feats=btp->sfpListsize[count];
	Int2 feat_index = (Int2) btp->pap_index;
	Int4 len_cds, len_prot;

	sfp_out=btp->sfp_out;
	if (sfp_out->qual)
		sfp_out->qual = GBQualFree(sfp_out->qual);
	ifp = sfp_out->data.value.ptrvalue;
	if (ifp->loc)
		ifp->loc = MemFree(ifp->loc);
	if (feat_index < total_feats)
	{
		sfp_in = btp->sfpList[count][feat_index];
		switch (sfp_in->data.choice)
		{ /* CDS's are done separately so that the GenBankSelect note
		comes immediately before the gi number in the note. */
			case SEQFEAT_CDREGION:
				status = ConvertToNAImpFeat(btp, count, sfp_in, &sfp_out, feat_index);
				if (status < 1)
					break;
				GetNonGeneQuals(sfp_in, sfp_out, btp, count, feat_index);
				ifp = sfp_out->data.value.ptrvalue;
				flat2asn_install_feature_user_string(ifp->key, NULL);
				loc_ok=CheckAndGetNAFeatLoc(btp, count, &ptr, sfp_out, TRUE);
				flat2asn_delete_feature_user_string();
				flat2asn_install_feature_user_string(ifp->key, ptr);
				if (loc_ok == TRUE || ASN2FF_VALIDATE_FEATURES == FALSE)
					ifp->loc = ptr;
				else
					break;
				LookForPartialImpFeat(sfp_out, FALSE);
				ComposeCodeBreakQuals(btp, count, feat_index, sfp_out);
				genetic_code[0]='\0';
				if (ASN2FF_TRANSL_TABLE == TRUE)
				{
					GetGeneticCode(genetic_code, sfp_in);
					if (genetic_code[0] != '\0')
						sfp_out->qual =
							AddGBQual(sfp_out->qual, "transl_table", genetic_code);
				}
				if (btp->format == GENBANK || btp->format == EMBL)
				{ 
					if ((GBQualPresent("translation", sfp_out->qual)) == FALSE)
					{ 
						sfp_cds = SeqFeatNew();
/* ProteinFromCdRegion needs a CDS (sfp->data.choice == 3) sfp to work */
						sfp_cds->data.choice = 3;
						sfp_cds->data.value.ptrvalue = sfp_in->data.value.ptrvalue;
						sfp_cds->qual = sfp_out->qual;
						sfp_cds->partial = sfp_out->partial;
						sfp_cds->comment = sfp_out->comment;
						sfp_cds->exp_ev = sfp_out->exp_ev;
						sfp_cds->location = sfp_out->location;
						sfp_cds->product = sfp_out->product;
						byte_sp = ProteinFromCdRegion(sfp_cds, FALSE);
						protein_seq = BSMerge(byte_sp, NULL);
						if ( protein_seq && protein_seq[0] != '-')
						{
							len_prot = StringLen(protein_seq);
							len_cds = SeqLocLen(sfp_in->location);
							if (len_prot >= 6)
								if (3*len_prot == len_cds || 
									3*(len_prot+1) == len_cds)
									sfp_out->qual = AddGBQual(sfp_out->qual, "translation", protein_seq);
						}
						byte_sp = BSFree(byte_sp);
						protein_seq = MemFree(protein_seq);
						sfp_cds->qual = NULL;
						sfp_cds->comment = NULL;
						sfp_cds->location = NULL;
						sfp_cds->product = NULL;
						sfp_cds->data.value.ptrvalue = NULL;
						SeqFeatFree(sfp_cds);
					}
				}
				ComposeGBQuals(sfp_out, btp, count, feat_index);
				PutTranslationLast(sfp_out);
				status = ValidateNAImpFeat(btp, count, sfp_out);
				if (status >= 0 || ASN2FF_VALIDATE_FEATURES == FALSE)
					PrintImpFeat(btp, count, sfp_out);
				break;
			case SEQFEAT_BIOSRC:
			case SEQFEAT_RNA:
			case SEQFEAT_SEQ:
			case SEQFEAT_REGION:
			case SEQFEAT_COMMENT:
			case SEQFEAT_RSITE:
				status = ConvertToNAImpFeat(btp, count, sfp_in, &sfp_out, feat_index);
				if (status < 1)
					break;
				GetNonGeneQuals(sfp_in, sfp_out, btp, count, feat_index);
				ifp = sfp_out->data.value.ptrvalue;
				flat2asn_install_feature_user_string(ifp->key, NULL);
				loc_ok=CheckAndGetNAFeatLoc(btp, count, &ptr, sfp_out, TRUE);
				flat2asn_delete_feature_user_string();
				flat2asn_install_feature_user_string(ifp->key, ptr);
				if (loc_ok == TRUE || ASN2FF_VALIDATE_FEATURES == FALSE)
					ifp->loc = ptr;
				else
					break;
				LookForPartialImpFeat(sfp_out, FALSE);
				ComposeGBQuals(sfp_out, btp, count, feat_index);
				status = ValidateNAImpFeat(btp, count, sfp_out);
				if (status >= 0 || ASN2FF_VALIDATE_FEATURES == FALSE)
					PrintImpFeat(btp, count, sfp_out);
				break;
			case SEQFEAT_IMP:
				status = ConvertToNAImpFeat(btp, count, sfp_in, &sfp_out, feat_index);
				if (status < 1)
					break;
				GetNonGeneQuals(sfp_in, sfp_out, btp, count, feat_index);
				ifp = sfp_out->data.value.ptrvalue;
				flat2asn_install_feature_user_string(ifp->key, NULL);
				loc_ok=CheckAndGetNAFeatLoc(btp, count, &ptr, sfp_out, TRUE);
				flat2asn_delete_feature_user_string();
				flat2asn_install_feature_user_string(ifp->key, ptr);
				if (loc_ok == TRUE || ASN2FF_VALIDATE_FEATURES == FALSE)
					ifp->loc = ptr;
				else
					break;
				LookForPartialImpFeat(sfp_out, FALSE);
				ComposeGBQuals(sfp_out, btp, count, feat_index);
				status = ValidateNAImpFeat(btp, count, sfp_out);
				if (status >= 0 || ASN2FF_VALIDATE_FEATURES == FALSE)
					PrintImpFeat(btp, count, sfp_out);
				break;
			default:
				break;
		}
		flat2asn_delete_feature_user_string();
	}
	else
		PrintOrphanFeatByNumber(btp, count, sfp_out, feat_index-total_feats);

	sfp_out->comment = NULL;
	sfp_out->location = NULL;
	sfp_out->product = NULL;
	if (sfp_out->qual)
		sfp_out->qual = GBQualFree(sfp_out->qual);
}	/* PrintNAFeatByNumber */

void PrintOrphanFeatByNumber (BiotablePtr btp, Int2 count, SeqFeatPtr sfp_out, Int2 feat_index)

{
	Boolean found_orphan=FALSE, loc_ok;
	CharPtr ptr=NULL;
	GeneStructPtr gsp=btp->gspp[count][0];
	ImpFeatPtr ifp;
	Int2 orphan_index, status;

	ifp = (ImpFeatPtr) sfp_out->data.value.ptrvalue;

	for (orphan_index=0; orphan_index<btp->sfpGenesize[count]; orphan_index++)
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
	{
		orphan_index = MiscFeatOrphanGenes (btp, count, sfp_out, orphan_index);
		if (orphan_index != -1)
		{
			if (ifp->loc)
				ifp->loc = MemFree(ifp->loc);
			if (btp->format == GENPEPT)
			{ /* the functions that CheckAndGetFeatLoc use for
			checking fails on protein locations sometimes. */
				GetAAFeatLoc (btp, count, &ptr, sfp_out, FALSE);
				ifp->loc = ptr;
				status = ValidateAAImpFeat(btp, count, sfp_out, FALSE);
				if (status >= 0 || ASN2FF_VALIDATE_FEATURES == FALSE)
					PrintImpFeat(btp, count, sfp_out);
			} 
			else 
			{ 
				loc_ok=CheckAndGetNAFeatLoc(btp, count, &ptr, sfp_out, TRUE);
				if (loc_ok == TRUE || ASN2FF_VALIDATE_FEATURES == FALSE)
				{
					ifp->loc = ptr;
					flat2asn_install_feature_user_string("misc_feature", ifp->loc);
					status = ValidateNAImpFeat(btp, count, sfp_out);
					if (status >= 0 || ASN2FF_VALIDATE_FEATURES == FALSE)
						PrintImpFeat(btp, count, sfp_out);
					flat2asn_delete_feature_user_string();
				}
			}
		}
	}
	return;
}	/* PrintOrphanFeatByNumber */

/**************************************************************************
*Int2 ConvertToNAImpFeat
*
*	This code copies a SeqFeat into an ImpFeat format for use in
*	producing GenBank format.  Two SeqFeatPtr's should be passed
*	in as arguments (sfp_in, &sfp_out).  
*	An Int2 is returned, telling the status of the function:
*		1: conversion successful
*		0: no conversion, also no error (data in ASN.1 that is lost
*			or put out otherwise).
*		-1: an error
*
*	Written by Tom Madden 
*	new Biosource feature converted to ImpFeat /tatiana 2-28-95/
**************************************************************************/

Int2 ConvertToNAImpFeat (BiotablePtr btp, Int2 count, SeqFeatPtr sfp_in, SeqFeatPtr PNTR sfpp_out, Int2 ext_index)

{
	BioseqPtr bsp;
	Boolean found_key;
	CdRegionPtr cdr;
	Char buffer[2], printbuf[41], temp[65];
	CharPtr buf_ptr = &(buffer[0]), protein_seq=NULL, ptr = &(temp[0]);
	NoteStructPtr nsp;
	ImpFeatPtr ifp, ifp_in;
	Int2 retval=1;
	Int4 length=0;
	RnaRefPtr rrp;
	SeqFeatPtr sfp_out;
	SeqIdPtr xid;
	ValNodePtr product;
	BioSourcePtr biosp;
	OrgRefPtr orp;
	CharPtr loc = NULL;

	sfp_out = *sfpp_out;

	if (sfp_out->data.choice != SEQFEAT_IMP)
		return -1;

	ifp = (ImpFeatPtr) sfp_out->data.value.ptrvalue;

	sfp_out->partial = sfp_in->partial;
	sfp_out->comment = sfp_in->comment;
	sfp_out->exp_ev = sfp_in->exp_ev;
	sfp_out->location = sfp_in->location;
	sfp_out->product = sfp_in->product;

/* These lines are being commented out for purposes of comparison of the
new output with the old output.  They should probably be reinstated 
after QA is finished.  Tom Madden (5/10/94).
	if (sfp_out->comment)
	{
		nsp = btp->nspp[count][ext_index];
		CpNoteToCharPtrStack(nsp, NULL, (CharPtr) sfp_out->comment);
	}
	*/

	found_key = GetNAFeatKey(ifp->key, sfp_in) ;
	if (! found_key)
		return -1;
	switch (sfp_in->data.choice)
	{
	case SEQFEAT_BIOSRC:            /* add by Tatiana 2-28-95 */
		biosp = sfp_in->data.value.ptrvalue;
		orp = (OrgRefPtr) biosp->org;	
		if (orp) {
			if (orp->taxname) {
				sfp_out->qual = AddGBQual(sfp_out->qual, 
									"organism", orp->taxname);
			} else if (orp->common) {
				if (StrStr(orp->common, "virus") ||
				    StrStr(orp->common, "Virus") ||
				    StrStr(orp->common, "phage") ||
				    StrStr(orp->common, "Phage") ||
				    StrStr(orp->common, "viroid") ||
				    StrStr(orp->common, "Viroid"))
				{
					sfp_out->qual = AddGBQual(sfp_out->qual, "organism",
															 orp->common);
				}
			}
		} else {
			sfp_out->qual = AddGBQual(sfp_out->qual, "organism",
															 "unknown");
		}
		sfp_out->qual = AddBioSourceToGBQual(biosp, sfp_out->qual);
		break;
	case SEQFEAT_CDREGION:
		product = sfp_in->product;
		cdr = (CdRegionPtr) sfp_in->data.value.ptrvalue;
		if ((GBQualPresent("codon_start", sfp_in->qual)) == FALSE)
		{ /* Above checks if codon_start is already present. */
			if (cdr->frame)
				sprintf(buf_ptr, "%ld", (long) (cdr->frame)); 
			else 
				sprintf(buf_ptr, "1"); 
			sfp_out->qual = AddGBQual(sfp_out->qual, "codon_start", buf_ptr);
		}
		if (product)
		{
			GetProductGeneInfo(sfp_in, btp, count, ext_index);
			bsp = btp->bsp[count];
			length = bsp->length;
			protein_seq = GetProductFromCDS(product, sfp_in->location, length);
			if (protein_seq)
			{
				sfp_out->qual = AddGBQual(sfp_out->qual, "translation", protein_seq);
				MemFree(protein_seq);
			}
		}
		break;
	case SEQFEAT_RNA:
		rrp = sfp_in->data.value.ptrvalue;
		/* the following code was taken (almost) directly from Karl
		Sirotkin's code.					*/
		switch ( rrp -> type){ /* order of case n: matches tests in
                                is_RNA_type() of genasn.c in
                                GenBankConversion directory */
			case 2:
				break;
			case 255:
				break;
			case 3:
				nsp = btp->nspp[count][ext_index];
				if (rrp->ext.choice == 1)
					CpNoteToCharPtrStack(nsp, NULL, (CharPtr) rrp->ext.value.ptrvalue);
				else if (rrp->ext.choice == 0 ||
					rrp->ext.choice == 2)
					DotRNAQuals(btp, sfp_in, sfp_out, count, ext_index);
				break;
			case 4:
				break;
			case 1:
				break;
			case 5:
				break;
			case 6:
				break;
		}

		if (rrp->pseudo && rrp->pseudo == TRUE)
		{
			if ((GBQualPresent("pseudo", sfp_in->qual)) == FALSE)
				sfp_out->qual = AddGBQual(sfp_out->qual, "pseudo", NULL);
		}

			
		break;
	case SEQFEAT_SEQ:	
		if ((xid=CheckXrefFeat(btp->bsp[count], sfp_in)) != NULL)
		{
			ptr = &(temp[0]);
			SeqIdPrint(xid, printbuf, PRINTID_FASTA_SHORT);
			sprintf(ptr, "Cross-reference: %s", printbuf);
			nsp = btp->nspp[count][ext_index];
			SaveNoteToCharPtrStack(nsp, NULL, ptr);
		}
		else
			retval = 0;
		break;
	case SEQFEAT_IMP:
		ifp_in = (ImpFeatPtr) sfp_in->data.value.ptrvalue;
		if (ifp_in->loc != NULL)
			ifp->loc = ifp_in->loc;
		if (StringCmp(ifp_in->key, "CDS") == 0)
		{
			if ((GBQualPresent("pseudo", sfp_in->qual)) == FALSE &&
				btp->error_msgs == TRUE)
				ErrPostEx(SEV_INFO, ERR_FEATURE_non_pseudo, 
				    "ConvertToNAImpFeat: Non-pseudo ImpFeat CDS found\n");
		}
		break;
	case SEQFEAT_REGION:
	case SEQFEAT_COMMENT:
	case SEQFEAT_RSITE:
		break;
	default:
		if (btp->error_msgs == TRUE)
			ErrPostEx(SEV_WARNING, ERR_FEATURE_UnknownFeatureKey, 
				"Unimplemented type of feat in ConvertToNAImpFeat\n");
			retval = -1;	
		break;
	}

	return retval;
}	/* ConvertToNAImpFeat */

/***************************************************************************
*PrintAAFeatByNumber
*
*	This function prints out the genpept SeqFeats.
*
*	written by Tom Madden
**************************************************************************/

void PrintAAFeatByNumber (BiotablePtr btp, Int2 count)

{
	CharPtr ptr=NULL;
	Char genetic_code[3];
	ImpFeatPtr ifp;
	Int2 status, total_feats=btp->sfpListsize[count];
	Int2 feat_index = (Int2) btp->pap_index;
	NoteStructPtr nsp;
	SeqFeatPtr sfp_in, sfp_out=NULL;

	sfp_out=btp->sfp_out;
	if (sfp_out->qual)
		sfp_out->qual = GBQualFree(sfp_out->qual);
	ifp = sfp_out->data.value.ptrvalue;
	if (ifp->loc)
		ifp->loc = MemFree(ifp->loc);

	if (feat_index < total_feats)
	{
		sfp_in = btp->sfpList[count][feat_index];
		switch (sfp_in->data.choice)
		{ /* Note: the functions that CheckAndGetFeatLoc use for
		checking fails on protein locations sometimes. */
			case 3:
				GetNonGeneQuals(sfp_in, sfp_out, btp, count, feat_index);
				status = ConvertToAAImpFeat(btp, count, sfp_in, &sfp_out, feat_index);
				if (status < 0)
					break;
				ComposeGBQuals(sfp_out, btp, count, feat_index);
				GetAAFeatLoc(btp, count, &ptr, sfp_in, TRUE);
				ifp->loc = ptr;
				ptr = FlatLoc(btp, count, sfp_in->location);
				sfp_out->qual =
				     AddGBQual(sfp_out->qual, "coded_by", ptr);
				ptr = MemFree(ptr);
				genetic_code[0]='\0';
				if (ASN2FF_TRANSL_TABLE == TRUE)
				{
					GetGeneticCode(genetic_code, sfp_in);
					if (genetic_code[0] != '\0')
						sfp_out->qual =
							AddGBQual(sfp_out->qual, "transl_table", genetic_code);
				}
				status = ValidateAAImpFeat(btp, count, sfp_out, TRUE);
				if (status >= 0)
					PrintImpFeat(btp, count, sfp_out);
				break;
			case 4:
				GetNonGeneQuals(sfp_in, sfp_out, btp, count, feat_index);
				nsp = btp->nspp[count][feat_index];
				AddProteinQuals(sfp_in, sfp_out, nsp);
				status = ConvertToAAImpFeat(btp, count, sfp_in, &sfp_out, feat_index);
				if (status < 0)
					break;
				ComposeGBQuals(sfp_out, btp, count, feat_index);
				GetAAFeatLoc(btp, count, &ptr, sfp_out, FALSE);
				ifp->loc = ptr;
				status = ValidateAAImpFeat(btp, count, sfp_out, FALSE);
				if (status >= 0)
					PrintImpFeat(btp, count, sfp_out);
				break;
			case 7:
			case 8:
			case 9:
			case 10:
			case 11:
			case 12:
			case 17:
			case 18:
			case 19:
				GetNonGeneQuals(sfp_in, sfp_out, btp, count, feat_index);
				status = ConvertToAAImpFeat(btp, count, sfp_in, &sfp_out, feat_index);
				if (status < 0)
					break;
				ComposeGBQuals(sfp_out, btp, count, feat_index);
				GetAAFeatLoc(btp, count, &ptr, sfp_out, FALSE);
				ifp->loc = ptr;
				status = ValidateAAImpFeat(btp, count, sfp_out, FALSE);
				if (status >= 0)
					PrintImpFeat(btp, count, sfp_out);
				break;
			default:
				break;
		}
	}
	else
		PrintOrphanFeatByNumber(btp, count, sfp_out, feat_index-total_feats);

	sfp_out->comment = NULL;
	sfp_out->location = NULL;
	sfp_out->product = NULL;
	if (sfp_out->qual)
		sfp_out->qual = GBQualFree(sfp_out->qual);
}	/* PrintAAFeatByNumber */


/**************************************************************************
*ConvertToAAImpFeat
*
*	This code copies a SeqFeat into an ImpFeat format for use in
*	producing GenBank format.  Two SeqFeatPtr's should be passed
*	in as arguments (sfp_in, sfp_out).  On the first call, of a
*	number of calls, sfp_out should be NULL so that memory for 
*	ImpFeat can be allocated.  On subsequent calls, sfp_out->data.choice
*	should be "8" (for ImpFeats).
*
*	Written by Tom Madden 
*
**************************************************************************/

Int2 ConvertToAAImpFeat (BiotablePtr btp, Int2 count, SeqFeatPtr sfp_in, SeqFeatPtr PNTR sfpp_out, Int2 ext_index)

{
	BioseqPtr bsp=NULL;
	Char printbuf[41], temp[65];
	CharPtr ptr;
	ImpFeatPtr ifp, ifp_in;
	Int2 retval=1;
	NoteStructPtr nsp;
	SeqFeatPtr sfp_out;
	SeqIdPtr sip=NULL, xid;
	ValNodePtr vnp, vnp1;

	sfp_out = *sfpp_out;

	if (sfp_out->data.choice != 8)
		return -1;

	ifp = (ImpFeatPtr) sfp_out->data.value.ptrvalue;

	sfp_out->partial = sfp_in->partial;
	sfp_out->comment = sfp_in->comment;
	sfp_out->exp_ev = sfp_in->exp_ev;
	sfp_out->location = sfp_in->location;

	if (sfp_out->comment)
	{
		nsp = btp->nspp[count][ext_index];
		CpNoteToCharPtrStack(nsp, NULL, (CharPtr) sfp_out->comment);
	}

	switch (sfp_in->data.choice)
	{
	case 3:
		ifp->key = StringCpy(ifp->key, "CDS");
		break;
	case 4:
		ifp->key = StringCpy(ifp->key, "Protein");
		if (sfp_in->location)
		{
			sip = SeqLocId(sfp_in->location);
			if (sip)
				bsp = BioseqFind(sip);
			if (bsp)
			{
				vnp = bsp->descr;
				while (vnp)
				{
					if (vnp->choice == Seq_descr_modif)
					{
					    vnp1 = vnp->data.ptrvalue;
					    while (vnp1)
					    {
					         if (vnp1->data.intvalue == 10)
					         {
					    		sfp_out->partial = TRUE;
					    		break;
					         }
					         vnp1=vnp1->next;
					    }
					}
					vnp = vnp->next;
				}
			}
		}
		break;
	case 7:	
		ifp->key = StringCpy(ifp->key, "misc_feature");
		if ((xid=CheckXrefFeat(btp->bsp[count], sfp_in)) != NULL)
		{
			SeqIdPrint(xid, printbuf, PRINTID_FASTA_SHORT);
			ptr = &(temp[0]);
			sprintf(ptr, "Cross-reference: %s", printbuf);
			nsp = btp->nspp[count][ext_index];
			SaveNoteToCharPtrStack(nsp, NULL, ptr);
		}
		else
			retval = 0;
		break;
	case 8:
		ifp_in = (ImpFeatPtr) sfp_in->data.value.ptrvalue;
		ifp->key = StringCpy(ifp->key, ifp_in->key);
		break;
	case 9:
		sfp_out->qual = 
		     AddGBQual(sfp_out->qual, "region_name", sfp_in->data.value.ptrvalue);
		ifp->key = StringCpy(ifp->key, "Region");
		break;
	case 10:
		ifp->key = StringCpy(ifp->key, "misc_feature");
		break;
	case 11:
		ptr = AsnEnumStr("SeqFeatData.bond", (Int2) (sfp_in->data.value.intvalue));
		sfp_out->qual = AddGBQual(sfp_in->qual, "bond_type", ptr);
		ifp->key = StringCpy(ifp->key, "Bond");
		break;
	case 12:
		ptr = AsnEnumStr("SeqFeatData.site", (Int2) (sfp_in->data.value.intvalue));
		sfp_out->qual = AddGBQual(sfp_out->qual, "site_type", ptr);
		ifp->key = StringCpy(ifp->key, "Site");
		break;
	case 17:
		ptr = AsnEnumStr("SeqFeatData.psec-str", (Int2) (sfp_in->data.value.intvalue));
		sfp_out->qual = AddGBQual(sfp_out->qual, "sec_str_type", ptr);
		ifp->key = StringCpy(ifp->key, "SecStr");
		break;
	case 18:
		sfp_out->qual = 
			AddGBQual(sfp_out->qual, "non-std-residue", sfp_in->data.value.ptrvalue);
		ifp->key = StringCpy(ifp->key, "NonStdResidue");
		break;
	case 19:
		sfp_out->qual = 
		     AddGBQual(sfp_out->qual, "heterogen", sfp_in->data.value.ptrvalue);
		ifp->key = StringCpy(ifp->key, "Het");
		break;
	default:
		if (btp->error_msgs == TRUE)
			ErrPostEx(SEV_WARNING, ERR_FEATURE_UnknownFeatureKey, 
				"Unimplemented type of feat in ConvertToAAImpFeat\n");
		retval = 0;
		break;
	}

	return retval;

}	/* ConvertToAAImpFeat */

/************************************************************************
*GetProductFromCDS(ValNodePtr product, ValNodePtr location, Int4 length)
*
*	Gets the CDS product, using SeqPortNewByLoc
*	The bsp is that of the protein, and comes from the location.  The bsp
*	is found in the calling program anyway, as it's used to get
*	the EC_NUM.
*	The protein sequence comes back in allocated memory.  The user
*	is responsible for deallocating that.
*
*  A check is made (BioseqFind()) that the protein Bioseq is in memory.
*  This guarantees that a fetch is NOT made if it is not memory, to accomodate
*  the splitting of DNA and protein in Entrez. In this case, it's just
*  translated.
*
*************************************************************************/

CharPtr GetProductFromCDS(ValNodePtr product, ValNodePtr location, Int4 bsp_length)

{
	Boolean at_end=FALSE;
	CharPtr protein_seq=NULL, start_ptr=NULL;
	Int4 length;
	SeqPortPtr spp;
	Uint1 residue, code;
	BioseqPtr bsp;
	SeqIdPtr sip;

	if (ASN2FF_IUPACAA_ONLY == TRUE)
		code = Seq_code_iupacaa;
	else
		code = Seq_code_ncbieaa;

	if (product)
	{
		sip = SeqLocId(product);
		bsp = BioseqFindCore(sip);
		if (bsp != NULL)    /* Bioseq is (or has been) in memory */
		{
			length = SeqLocLen(product);
			if (length > 0)
			{
				if (SeqLocStart(location) == 0 ||
					SeqLocStop(location) == bsp_length-1)
					at_end = TRUE;
				start_ptr = protein_seq = (CharPtr) MemNew((size_t) (length*sizeof(CharPtr)));
				spp = SeqPortNewByLoc(product, code);
				while ((residue=SeqPortGetResidue(spp)) != SEQPORT_EOF)
				{
					if ( !IS_residue(residue) && residue != INVALID_RESIDUE )
						continue;
					if (residue == INVALID_RESIDUE)
						residue = (Uint1) 'X';
					*protein_seq = residue;
					protein_seq++;
				}
				SeqPortFree(spp);
				if (at_end)
				{
					if (StringLen(start_ptr) < GENPEPT_MIN)
						start_ptr = MemFree(start_ptr);
				}
			}
		}
	}
	return start_ptr;
}

/************************************************************************
*CharPtr DotRNAQuals (BiotablePtr btp, SeqFeatPtr sfp, Int2 count, Int2 ext_index)
*
*	Make the anticodon qualifier and (possible) note to the tRNA
*	with the following paradigm:
*
*	1.) Look at SeqFeat.ext for a UserObject using the fct. QualLocWrite,
*	if result is not NULL, use this location in anticodon qualifier;
*
*	2.) Look for an anticodon qualifier, use if no QualLocWrite's 
*	result was not zero;
*
*	3.) Make note if neither 1.) or 2.) was true, or there are
*	multiple codons.
*************************************************************************/

void DotRNAQuals (BiotablePtr btp, SeqFeatPtr sfp_in, SeqFeatPtr sfp_out, Int2 count, Int2 ext_index)

{
	Boolean found_anticodon=FALSE, found_qual=FALSE;
	Char buffer[40];
	CharPtr aa_ptr, newptr=NULL, ptr = &(buffer[0]), tmp=NULL;
	GBQualPtr curq;
	RnaRefPtr rrp;
	tRNAPtr trna=NULL;

	if (sfp_in->ext)
	{	/* Look for UserObject */
		tmp = QualLocWrite(sfp_in->ext, ptr);
		if (tmp)
		{
			newptr = MemNew(50*sizeof(Char));
			rrp = sfp_in->data.value.ptrvalue;
			trna = rrp->ext.value.ptrvalue;
			aa_ptr = GettRNAaa(trna, btp->error_msgs);
			if (aa_ptr)
			{
			     sprintf(newptr, "(pos:%s,aa:%s)", ptr, aa_ptr);
			     sfp_out->qual = 
			        AddGBQual(sfp_out->qual, "anticodon", newptr); 
			     found_anticodon=TRUE;
			}
			newptr = MemFree(newptr);
		}
	}

	if (! found_anticodon)
	{	/* Look for anticodon qual if no UserObject found */
		for (curq=sfp_in->qual; curq; curq=curq->next)
			if (StringCmp("anticodon", curq->qual) == 0)
			{
			    sfp_out->qual = 
			       AddGBQual(sfp_out->qual, "anticodon", curq->val); 
			    found_qual=TRUE;
			    break;
			}
	}

	if (!found_anticodon && !found_qual)
	{	/* If neither was true, make note */
		rrp = sfp_in->data.value.ptrvalue;
		if (rrp->ext.choice == 2)
		{
			trna = rrp->ext.value.ptrvalue;
			ComposetRNANote(btp, trna, count, ext_index);
		}
	}

	if (trna && (trna->codon)[0] != 255 && (trna->codon)[1] != 255)
	{	/* if multiple codons recognized, make note */
		rrp = sfp_in->data.value.ptrvalue;
		trna = rrp->ext.value.ptrvalue;
		ComposetRNANote(btp, trna, count, ext_index);
	}

	
}	/* DotRNAQuals */


/*************************************************************************
*ComposetRNANote (BiotablePtr btp, tRNAPtr trna, Int2 count, Int2 ext_index)
*
*	Add info from Trna-ext to Note stack in the GeneStructPtr.
**************************************************************************/

void ComposetRNANote(BiotablePtr btp, tRNAPtr trna, Int2 count, Int2 ext_index)

{
	Char buffer[25];
	CharPtr ptr = &(buffer[0]);
	NoteStructPtr nsp=btp->nspp[count][ext_index];
	Int2 index;
	Uint1 codon[4];

	if (! trna) 
		return;

	if ((trna->codon)[0] != 255)
	{
		codon[3] = '\0';
		for (index=0; index<6; index++)
		{
			if ((trna->codon)[index] == 255)
				break;
			if (CodonForIndex((trna->codon)[index], Seq_code_iupacna, codon))
			{
				StringCpy(ptr, (CharPtr) codon);
				ptr += 3;
			}
			else
			{
				*ptr = '?';	ptr++;
			}
			if (index<5 && (trna->codon)[index+1] != 255)
			{
				*ptr = ',';	ptr++;
				*ptr = ' ';	ptr++;
			}
		}
		if ((trna->codon)[1] == 255)
		{
			ptr = &buffer[0];
			SaveNoteToCharPtrStack(nsp, "codon recognized:", ptr);
		}
		else
		{
			ptr = &buffer[0];
			SaveNoteToCharPtrStack(nsp, "codons recognized:", ptr);
		}
	}

	if (trna -> aatype)
	{
		ptr = GettRNAaa(trna, btp->error_msgs);
		SaveNoteToCharPtrStack(nsp, "aa:", ptr);
	}
	
	return;

}	/* ComposetRNANote */


/**************************************************************************
*CharPtr GettRNAaa (tRNAPtr trna, Boolean error_messages)
*
*	Return a pointer containing the amino acid type.
**************************************************************************/

CharPtr GettRNAaa (tRNAPtr trna, Boolean error_msgs)

{
	CharPtr ptr=NULL;
	SeqCodeTablePtr table;
	Uint1 seq_code;
/*
  The choice values used in the tRNA structure do NOT corresond to
  the choice(==ENUMs) of Seq-code_type, and the latter are used
  by all the utility functions, so we map them...
*/
	if ( trna && trna -> aatype)
	{
		switch (trna -> aatype)
		{
			case 1:
				seq_code = 2;
				break;
			case 2:
				seq_code = 8;
				break;
			case 3:
				seq_code = 7;
				break;
			case 4:
				seq_code = 11;
				break;
		}

		if ((table=SeqCodeTableFind (seq_code)) != NULL)
		  ptr = Get3LetterSymbol(seq_code, table, trna->aa, error_msgs);
/*
			ptr = SeqCodeNameGet(table, trna->aa, error_msgs);
*/
	}
	
	return ptr;
}	/* GettRNAaa */

/*****************************************************************************
*void ComposeCodeBreakQuals (BiotablePtr btp, Int2 count, Int2 ext_index, SeqFeatPtr sfp_out)
*
*	Add the quals of the form "/transl_except=(pos: ,aa: )" to the
*	SeqFeatPtr sfp_out.  
*	
*****************************************************************************/	

void ComposeCodeBreakQuals (BiotablePtr btp, Int2 count, Int2 ext_index, SeqFeatPtr sfp_out)

{
	Boolean minus=FALSE;
	CdRegionPtr crp;
	Char buffer[35];
	CharPtr ptr;
	Choice aa;
	CodeBreakPtr cbp;
	Int4 pos1, pos2;
	SeqCodeTablePtr table;
	SeqFeatPtr sfp_in = btp->sfpList[count][ext_index];
	SeqIntPtr sip;
	SeqLocPtr slp;
	Uint1 seq_code=0, the_residue;

	if ((sfp_in == NULL) || (sfp_in->data.choice != 3))
		return;

        crp = (CdRegionPtr) sfp_in->data.value.ptrvalue;

        if (crp->code_break != NULL)
        {
                cbp = crp->code_break;
                while (cbp != NULL)
                {
			aa = cbp->aa;
			switch (aa.choice)
			{
				case 1:
					seq_code = 8;
					break;
				case 2:
					seq_code = 7;
					break;
				case 3:
					seq_code = 11;
					break;
			}
			table = NULL;
			if (seq_code != 0)
				table=SeqCodeTableFind (seq_code);
			if (table != NULL)
			{
       	               		pos1 = INT4_MAX;
       	               		pos2 = -10;
       	               		slp = NULL;
       	               		while ((slp = SeqLocFindNext(cbp->loc, slp)) != NULL)
				{
					if (slp->choice == SEQLOC_INT)
					{
						sip = (SeqIntPtr)slp->data.ptrvalue;
						pos2 = sip->to;
						pos1 = sip->from;
					}
							
       	                	}
       	               		if ((pos2 - pos1) == 2)   /*  a codon */
       	                	{
					the_residue = (Uint1) cbp->aa.value.intvalue;
					ptr = Get3LetterSymbol(seq_code, table, the_residue, btp->error_msgs);
					pos1++;	/*increment by 1 for flat file*/
					pos2++;
					sprintf(buffer, "(pos:%ld..%ld,aa:%s)", (long) pos1, (long) pos2, ptr);
					AddGBQual(sfp_out->qual, "transl_except", buffer); 
       	                	}
       	                	else
       	                	{
       	               	 		ErrPost(CTX_NCBIOBJ, 1, "Invalid Code-break.loc");
       	                	}
			}
                        cbp = cbp->next;
                }
        }

	return;

}	/* ComposeCodeBreakQuals */

/***************************************************************************
*void Get3LetterSymbol (Uint1 seq_code, SeqCodeTablePtr table, Uint1 residue, Boolean error_msgs)
*
*	if (ASN2FF_IUPACAA_ONLY == TRUE) then
*	Check if the residue is legal in iupacaa; if not, return 'X', if so,
*	return the three letter code from iupacaa3.
*
*	if (ASN2FF_IUPACAA_ONLY != TRUE) then
*	Then do a translation, if necessary, then get th three letter code
*	from iupacaa3.
*
***************************************************************************/

CharPtr Get3LetterSymbol (Uint1 seq_code, SeqCodeTablePtr table, Uint1 residue, Boolean error_msgs)

{
	static CharPtr bad_symbol= "OTHER";
	CharPtr ptr, retval=NULL;
	Int2 index;
	SeqCodeTablePtr table_3aa;
	SeqMapTablePtr smtp;
	Uint1 code, new_residue;

	if (ASN2FF_IUPACAA_ONLY == TRUE)
		code = Seq_code_iupacaa;
	else
		code = Seq_code_ncbieaa;

	if (code != seq_code)
	{/* if code and seq_code are identical, then smtp is NULL?? */
		smtp = SeqMapTableFind(seq_code, code);
		new_residue = SeqMapTableConvert(smtp, residue);
	}
	else
		new_residue = residue;

/* The following looks for non-symbols (255) and "Undetermined" (88) */
	if ((int) new_residue == 255 || (int) new_residue == 88)
		retval = bad_symbol;
	else
	{
		ptr = SeqCodeNameGet(table, residue, error_msgs);
		
		table_3aa=SeqCodeTableFind (Seq_code_iupacaa3);
		if (ptr != NULL && table_3aa != NULL)
		{
			for (index=0; index < (int) table->num; index++)
			{
				if (StringCmp(ptr, (table_3aa->names) [index]) == 0)
				{
					retval = (table_3aa->symbols) [index];
					break;
				}
			}
		}
	}
	
	return retval;

}	/* Get3LetterSymbol */

/*------------ SeqCodeNameGet () ---------------*/
/* Karl Sirotkin's function.				*/

/* ---- Jim, don't these need some sanity range chaecks???*/

CharPtr SeqCodeNameGet (SeqCodeTablePtr table, Uint1 residue, Boolean error_msgs)
{
	int index=residue - table -> start_at;
	static CharPtr oops = "?";

	if (index >= 0 && index < (int) table -> num){
		return (table -> names) [index];
	}else {
		if (error_msgs == TRUE) 
			ErrPostEx(SEV_WARNING, CTX_NCBI2GB, 1,
			"asn2gnbk: %c(%d) > max in SeqCode table=%d",
			(char) residue, (int) residue, (int) table -> num);
		return oops;
	}
}

/*****************************************************************************
*ValidateNAImpFeat
*
*	This code validates an ImpFeat using some functions from
*	the GenBank Parser.
*
*	If a feat is bad and can't be corrected, -1 is returned.
*
*****************************************************************************/

Int2 ValidateNAImpFeat (BiotablePtr btp, Int2 count, SeqFeatPtr sfp)

{
	CharPtr key;
	GBQualPtr gbqual;
	ImpFeatPtr ifp;
	Int2 index, retval=0, status=0;

	if (sfp->data.choice != 8)
		return -1;
	else
	{

		ifp = sfp->data.value.ptrvalue;
		key = ifp->key;
		index = GBFeatKeyNameValid(&key, btp->error_msgs);
		if (key != ifp->key)
		{
			StringCpy(ifp->key, key);
			key = MemFree(key);
		}
	
		if (index == -1)
		{
			retval = -2;
		}
		else
		{
			gbqual = sfp->qual;
			if (ASN2FF_VALIDATE_FEATURES == TRUE)
				status = GBFeatKeyQualValid(index, &gbqual, FALSE, TRUE);
			else
				status = GBFeatKeyQualValid(index, &gbqual, FALSE, FALSE);
			if (status == GB_FEAT_ERR_NONE)
				retval = 1;
			else if (status == GB_FEAT_ERR_REPAIRABLE)
			{
				retval = 0;
			}
			else if (status == GB_FEAT_ERR_DROP)
			{
				retval = -1;
			}
			sfp->qual = gbqual;
		}

	}

	return retval;
}	/* ValidateNAImpFeat */

/*****************************************************************************
*ValidateAAImpFeat
*
*	This code will validate an ImpFeat using some functions from
*	the GenBank Parser.  Right now it just checks to see that the
*	sfp is an ImpFeat and checks for a partial qualifier.
*
*	If a feat is bad and can't be corrected, -1 is returned.
*
*****************************************************************************/

Int2 ValidateAAImpFeat (BiotablePtr btp, Int2 count, SeqFeatPtr sfp, Boolean use_product)

{

	if (sfp->data.choice != 8)
		return -1;

	LookForPartialImpFeat(sfp, use_product);

	return 0;
}	/* ValidateAAImpFeat */

/*****************************************************************************
*LookForPartialImpFeat
*
*	This function first looks for the sfp->qual of type "partial".
*	If found the qual is deleted and the variable "partial" is
*	set equal to TRUE.  If "partial" is TRUE or if sfp->partial
*	is TRUE, FlatAnnotPartial is called (modified version of Karl Sirotkin's
*	program) to see if sfp->partial should really be TRUE.
*	WARNING: sfp should be an ImpFeatPtr
*
*	written by Tom Madden (12/7/93)
*****************************************************************************/

void LookForPartialImpFeat(SeqFeatPtr sfp, Boolean use_product)

{
	Boolean partial=FALSE;
	GBQualPtr curq, gbqual, lastq=NULL, tmpqual;

	gbqual = sfp->qual;

	while (gbqual && (StringCmp(gbqual->qual, "partial")==0))
	{
		partial = TRUE;
		tmpqual = gbqual->next;
		gbqual->next = NULL;
		gbqual = GBQualFree(gbqual);
		gbqual = tmpqual;
	}

	if (gbqual)
	{
		for (lastq=gbqual, curq=gbqual->next; curq; curq=curq->next)
		{
			if (StringCmp(curq->qual, "partial") == 0)
			{
				partial = TRUE;
				lastq->next = curq->next;
				curq->next = NULL;
				curq = GBQualFree(curq);
				curq = lastq;
			}
			else
				lastq = curq;
		}
	}

	sfp->qual = gbqual;

	if (partial == TRUE || sfp->partial == TRUE)
		sfp->partial = FlatAnnotPartial(sfp, use_product);
}	/* LookForPartialImpFeat */


/*****************************************************************************
*void PrepareSourceFeatQuals(SeqFeatPtr sfp_in, SeqFeatPtr sfp_out, BiotablePtr btp, Int2 count, Boolean add_modifs)
*
*Normally called from PrintSourceFeat, collects all notes etc. together.
*Note: sfp_out may already have quals when it comes here, they should not
*be deleted!
* 	add_modifs: allows the addition of modifs to be specified, don't add
*		modifs if the source feature is a ImpFeat.
*
*For many cases there is no sfp_in, so that must be checked for.
*****************************************************************************/

void PrepareSourceFeatQuals(SeqFeatPtr sfp_in, SeqFeatPtr sfp_out, BiotablePtr btp, Int2 count, Boolean add_modif)

{
	CharPtr note=NULL;
	GBQualPtr qual1;
	NoteStructPtr nsp=btp->source_notes;

	if (sfp_in)
		for (qual1=sfp_in->qual; qual1; qual1=qual1->next)
		{
			if (StringCmp(qual1->qual, "note") == 0)
				CpNoteToCharPtrStack(nsp, NULL, qual1->val);
			else 	
				sfp_out->qual = 
					AddGBQual(sfp_out->qual, qual1->qual, qual1->val);
		}

/* not used in new style  */
	if (add_modif == TRUE)
		sfp_out->qual = AddModifsToGBQual(btp, count, sfp_out->qual);
/*---------------------    tatiana */
	if (sfp_in && sfp_in->comment)
	{
		CpNoteToCharPtrStack(nsp, NULL, (CharPtr) sfp_in->comment);
	}

	if (nsp->note[0])
	{
		note = ComposeNoteFromNoteStruct(nsp, NULL);
		if (note)
		{
			sfp_out->qual = AddGBQual(sfp_out->qual, "note", note);
			note = MemFree(note);
		}
	}

	return;
}


/*************************************************************************
*AddProteinQuals
*
*************************************************************************/

void AddProteinQuals (SeqFeatPtr sfp, SeqFeatPtr sfp_out, NoteStructPtr nsp)

{

	ProtRefPtr prp=sfp->data.value.ptrvalue;
	ValNodePtr vnp;

	if (prp->name != NULL)
	{
		for (vnp=prp->name; vnp; vnp=vnp->next)
			if (GBQualPresent("product", sfp_out->qual) == FALSE)
				sfp_out->qual = 
				     AddGBQual(sfp_out->qual, "product", vnp->data.ptrvalue);
			else 
				CpNoteToCharPtrStack(nsp, NULL, vnp->data.ptrvalue);
	}
	else if (prp->desc)
	{
		sfp_out->qual = 
		     AddGBQual(sfp_out->qual, "name", prp->desc);
	}

	for (vnp=prp->ec; vnp; vnp=vnp->next)
		if ((CheckForQual(sfp_out->qual, "EC_number", vnp->data.ptrvalue)) == 0)
			sfp_out->qual = 
		    		AddGBQual(sfp_out->qual, "EC_number", vnp->data.ptrvalue);

	return;
}

/***********************************************************************
*void GetGeneticCode(CharPtr ptr, SeqFeatPtr sfp)
*
*	returns ONLY non-standard (i.e., id not 0 or 1)
*	genetic codes.
***********************************************************************/

void GetGeneticCode(CharPtr ptr, SeqFeatPtr sfp)

{
	Boolean code_is_one=FALSE;
	CdRegionPtr cdr;
	GBQualPtr qual;
	ValNodePtr gcp, var;

	cdr = sfp->data.value.ptrvalue;
	gcp = cdr->genetic_code;

	if (gcp != NULL)
	{
		for (var=gcp->data.ptrvalue; var != NULL; var=var->next)
		{
			if (var->choice == 2)
			{
				if (var->data.intvalue != 0 )
				{
					if (var->data.intvalue == 1)
						code_is_one = TRUE;
					else
						sprintf(ptr, "%ld", (long) (var->data.intvalue));
				}
				break;	
			}
		}
		if (*ptr != '\0')
		{
			for (qual=sfp->qual; qual; qual=qual->next)
			{
				if (StringCmp("transl_table", qual->qual) == 0 &&
				      	  StringCmp(ptr, qual->val) != 0)
				{
				      	  ErrPostEx(SEV_WARNING, 
				 		ERR_FEATURE_GcodeAndTTableClash, "");
				          break;
				}
			}
		}	
		else if (code_is_one == TRUE)
		{
			for (qual=sfp->qual; qual; qual=qual->next)
			{
				if (StringCmp("transl_table", qual->qual) == 0 &&
				      	  StringCmp("1", qual->val) != 0)
				{
				      	  ErrPostEx(SEV_WARNING, 
				 		ERR_FEATURE_GcodeAndTTableClash, "");
				          break;
				}
			}
		}
	}
	else
	{
		for (qual=sfp->qual; qual; qual=qual->next)
			if (StringCmp("transl_table", qual->qual) == 0)
			{
				StringCpy(ptr, qual->val);
				break;
			}
	}

	return;
}	/* GetGeneticCode */

void GetGeneQuals (SeqFeatPtr sfp_in, BiotablePtr btp, Int2 count, Int2 index)

{
	GBQualPtr qual1;
	GeneStructPtr gsp=btp->gspp[count][index];

	for (qual1=sfp_in->qual; qual1; qual1=qual1->next)
	{
		if (StringCmp(qual1->qual, "gene") == 0)
		{
			if (gsp->gene)
				ValNodeAddStr(&(gsp->gene), 0, qual1->val);
			else
				gsp->gene = ValNodeAddStr(&(gsp->gene), 0, qual1->val);
		}
		else if (StringCmp(qual1->qual, "product") == 0)
		{
			if (gsp->product)
				ValNodeAddStr(&(gsp->product), 0, qual1->val);
			else
				gsp->product = ValNodeAddStr(&(gsp->product), 0, qual1->val);
		}
		else if (StringCmp(qual1->qual, "standard_name") == 0)
		{
			if (gsp->standard_name)
				ValNodeAddStr(&(gsp->standard_name), 0, qual1->val);
			else
				gsp->standard_name = ValNodeAddStr(&(gsp->standard_name), 0, qual1->val);
		}
		else if (gsp->map[0] == NULL 
			&& StringCmp(qual1->qual, "map") == 0)
			gsp->map[0] = qual1->val;
		else if (StringCmp(qual1->qual, "EC_number") == 0)
		{
			if (gsp->ECNum)
				ValNodeAddStr(&(gsp->ECNum), 0, qual1->val);
			else
				gsp->ECNum = ValNodeAddStr(&(gsp->ECNum), 0, qual1->val);
		}
	}

	return;
}	/* GetGeneQuals */

void GetNonGeneQuals (SeqFeatPtr sfp_in, SeqFeatPtr sfp_out, BiotablePtr btp, Int2 count, Int2 index)

{
	GBQualPtr qual1;
	NoteStructPtr nsp=btp->nspp[count][index];

	for (qual1=sfp_in->qual; qual1; qual1=qual1->next)
	{
		if (StringCmp(qual1->qual, "gene") == 0)
			;
		else if (StringCmp(qual1->qual, "product") == 0)
			;
		else if (StringCmp(qual1->qual, "standard_name") == 0)
			;
		else if (StringCmp(qual1->qual, "map") == 0)
			;
		else if (StringCmp(qual1->qual, "EC_number") == 0)
			;
		else if (StringCmp(qual1->qual, "anticodon") == 0)
			;	/* This is done by DotRNAQuals */
		else if (StringCmp(qual1->qual, "note") == 0)
			CpNoteToCharPtrStack(nsp, NULL, qual1->val);
		else if (StringCmp(qual1->qual, "transl_table") == 0)
			;	/* This is captured by GetGeneticCode */
		else 	
			sfp_out->qual = 
				AddGBQual(sfp_out->qual, qual1->qual, qual1->val);
	}

	return;
}	/* GetNonGeneQuals */

/****************************************************************************
*  void ComposeGBQuals (SeqFeatPtr sfp_out, BiotablePtr btp, Int2 count, Int2 index)
*
*	Composes the GBQuals for sfp_out using the information in the
*	GeneStructPtr (gsp), and then the quals already on sfp_out.
*
*	sfp_out: ImpFeat sfp (type 8).
*	btp: BiotablePtr containing all info.
**************************************************************************/
void ComposeGBQuals (SeqFeatPtr sfp_out, BiotablePtr btp, Int2 count, Int2 index)

{
	Char temp[65];
	Char buffer[10];
	CharPtr ascii, start, note=NULL, ptr=NULL;
	GBQualPtr gbqp=NULL, qual_temp, qual1;
	GeneStructPtr gsp=btp->gspp[count][index];
	Int2 int_index, status;
	NoteStructPtr nsp=btp->nspp[count][index];
	PubStructPtr psp;
	SeqFeatPtr sfp=btp->sfpList[count][index];
	Int2 ascii_len;
	ValNodePtr vnp, vnp1;

	if (gsp->gene) 
	{
		for (vnp=gsp->gene; vnp; vnp=vnp->next)
		{
			ascii_len = Sgml2AsciiLen(vnp->data.ptrvalue);
			start = ascii = MemNew((size_t) (10+ascii_len));
			ascii = Sgml2Ascii(vnp->data.ptrvalue, ascii, ascii_len+1);
			if ((CheckForQual(gbqp, "gene", start)) == 0)
				if ((CheckForQual(sfp_out->qual, "gene", start)) == 0)
					gbqp=AddGBQual(gbqp, "gene", start);
			start = MemFree(start);
		}
	}
	if (gsp->product) 
	{
		for (vnp=gsp->product; vnp; vnp=vnp->next)
		{
			if (GBQualPresent("product", gbqp) == FALSE &&
				GBQualPresent("product", sfp_out->qual) == FALSE)
					sfp_out->qual = 
					     AddGBQual(sfp_out->qual, "product", vnp->data.ptrvalue);
			else 
				CpNoteToCharPtrStack(nsp, NULL, vnp->data.ptrvalue);
		}
	}
	if (gsp->standard_name) 
	{
		for (vnp=gsp->standard_name; vnp; vnp=vnp->next)
		{
			if ((CheckForQual(gbqp, "standard_name", vnp->data.ptrvalue)) == 0)
				if ((CheckForQual(sfp_out->qual, "standard_name", vnp->data.ptrvalue)) == 0)
					gbqp=AddGBQual(gbqp, "standard_name", vnp->data.ptrvalue);
		}
	}
	if (gsp->map[0])
		gbqp = AddGBQual(gbqp, "map", gsp->map[0]);
	if (gsp->ECNum) 
	{
		for (vnp=gsp->ECNum; vnp; vnp=vnp->next)
		{
			if ((CheckForQual(gbqp, "EC_number", vnp->data.ptrvalue)) == 0)
				if ((CheckForQual(sfp_out->qual, "EC_number", vnp->data.ptrvalue)) == 0)
					gbqp=AddGBQual(gbqp, "EC_number", vnp->data.ptrvalue);
		}
	}
	if (gsp->pseudo == TRUE) 
	{
		gbqp = AddGBQual(gbqp, "pseudo", NULL);
	}
/* Add Experimental note */
	if (sfp != NULL && sfp->data.choice == 3)
	{
		ptr = &(temp[0]);
		status = MakeGBSelectNote(ptr, sfp);
		if (status > 0)
			SaveNoteToCharPtrStack(nsp, NULL, ptr);
		ptr=NULL;
	}
/* Right before compostion of note, add GI number. */
	AddGINumToNote (btp, count, sfp_out, index);
	if (nsp->note[0])
	{
		note = ComposeNoteFromNoteStruct(nsp, gsp);
		if (note)
		{
			gbqp = AddGBQual(gbqp, "note", note);
			note = MemFree(note);
		}
	}

	vnp = btp->Pub[count];
	for (vnp1=vnp; vnp1; vnp1=vnp1->next)
	{
		psp = vnp1->data.ptrvalue;
		for (int_index=0; int_index<psp->pubcount; int_index++)
			if (sfp == psp->pubfeat[int_index])
			{
				sprintf(buffer, "[%ld]", (long) (psp->number));
				AddGBQual(gbqp, "citation", buffer);
			}
	}

	if (gbqp)	/* any gene or note related quals added above? */
	{
		qual_temp = sfp_out->qual;
		sfp_out->qual = gbqp;

		for (qual1=gbqp; qual1->next; qual1=qual1->next)
			;
		qual1->next = qual_temp;
	}

	return;
}	/* ComposeGBQuals */

/*************************************************************************
*void PutTranslationLast(SeqFeatPtr sfp)
*
*	Check if there is a /translation and, if so, make it the last qualifier.
*	The sfp should be an ImpFeat with key CDS.
*
*	Below we search for the translation qualifier and store it as
*	qual_temp, until we get to the end of the linked list, then we
*	stick it back on.
**************************************************************************/

void PutTranslationLast(SeqFeatPtr sfp)

{
	Boolean still_looking=TRUE;
	GBQualPtr gbqual, qual, qual_temp=NULL, qual_last;
	ImpFeatPtr ifp=NULL;

	if ((sfp == NULL) || (sfp->data.choice != 8))
		return;

	ifp = sfp->data.value.ptrvalue;
	if (StringCmp(ifp->key, "CDS") == 0)
	{
		gbqual = sfp->qual;
		qual_last = NULL;
		for (qual=gbqual; qual->next; qual=qual->next)
		{ /* We need to go to the end of the linked list */
			if (still_looking == TRUE &&
				StringCmp("translation", qual->qual) == 0)
			{
				still_looking = FALSE;
				if (qual->next != NULL)
				{ /* if it's not the last qual anyway */
					if (qual_last == NULL) /*first*/
						gbqual = qual->next;
					else
						qual_last->next = qual->next;
					qual_temp = qual;
					qual=qual->next;
					qual_temp->next = NULL;
				}
			}
			qual_last = qual;
			if (qual->next == NULL)
				break;
		}
		qual->next = qual_temp;
		sfp->qual = gbqual;
	}
	return;
}	/* PutTranslationLast */

/****************************************************************************
* CharPtr ComposeNoteFromNoteStruct (NoteStructPtr nsp, GeneStrunctPtr gsp)
*
*	This function composes a "/note" for a SeqFeatPtr from the information
*	in the GeneStructPtr (gsp).
*	The first "for" loop initializes the first CharPtr and a check 
*	is done that the information in gsp->note is *not* redundant.  If 
*	it is not, first gsp->note_annot is copied onto a CharPtr (this 
*	field contains words describing the origin of the info in note, i.e., 
*	"Description"); then the actual note is copied onto the CharPtr.  
*	The second "for" loop does the same checking as the first and the
*	concatenation of more "note" strings is performed.
*
*n.b.: the caller is responsible for deallocating the final returned "note".
***************************************************************************/
CharPtr ComposeNoteFromNoteStruct (NoteStructPtr nsp, GeneStructPtr gsp)

{
	Boolean status;
	CharPtr note1=NULL, note2, note3;
	Int2 index, index1, index2, len;

	for (index=0; index<nsp->note_index; index++)
	{
		if (gsp)
		{
			if (CompareStringWithGsp(gsp, nsp->note[index]) != 0)
			{
				if (nsp->note_annot[index])
					note1 = Cat2Strings(nsp->note_annot[index], nsp->note[index], " ", 0);
				else
					note1 = StringSave(nsp->note[index]);
				len = CheckForExtraChars(note1);
				if (len == 0)
					note1 = MemFree(note1);
				else
					break;
			}
		}
		else
		{
			if (nsp->note_annot[index])
				note1 = Cat2Strings(nsp->note_annot[index], nsp->note[index], " ", 0);
			else
				note1 = StringSave(nsp->note[index]);
			len = CheckForExtraChars(note1);
			if (len == 0)
				note1 = MemFree(note1);
			else
				break;
		}
	}
	index++;

	for (index1=index; index1<nsp->note_index; index1++)
	{
		status = TRUE;
		note2 = nsp->note[index1];
		if (gsp && CompareStringWithGsp(gsp, note2) == 0)
			continue;

		for (index2=0; index2<index1; index2++)
		{
			if (gsp)
				if (GeneStringCmp(note2, nsp->note[index2]) == 0)
					status = FALSE;
		}
		if (status == TRUE)
		{
			if (nsp->note_annot[index1])
				note2 = Cat2Strings(nsp->note_annot[index1], nsp->note[index1], " ", 0);
			else /* rewrite to not always allocate note2 if no annot?????*/
				note2 = StringSave(nsp->note[index1]);
			len = CheckForExtraChars(note1);
			if (len > 0)
			{
				if (note1[len-1] == '.')
			        	note3 = Cat2Strings(note1, note2, "  ", -1);
				else 
					note3 = Cat2Strings(note1, note2, ";  ", -1);
				note1 = MemFree(note1);
				note2 = MemFree(note2);
				note1 = note3;
				note3 = NULL;
			}
			else
			{
				if (note1)
					note1 = MemFree(note1);
				note1 = note2;
			}
		}
	}

	return note1;
}	/* ComposeNoteFromNoteStruct */

/*************************************************************************
*static Int2 CheckForExtraChars(CharPtr note)
*
*	Check for spaces or semi-colons on the ends of notes.
************************************************************************/

static Int2 CheckForExtraChars(CharPtr note)

{
	Int2 len=0;

	if (note != NULL)
	{
		len = StringLen(note);
		while (len > 0)
		{
			if (note[len-1] == ' ' || note[len-1] == ';')
				note[len-1] = '\0';
			else
				break;
			len--;
		}
	}

	return len;

}	/* CheckForExtraChars */

/*************************************************************************
*	void AddGINumToNote (BiotablePtr btp, Int2 count, SeqFeatPtr sfp_in, SeqFeatPtr sfp_out, Int2 index)
*
*	btp: BiotablePtr,
*	sfp_out: synthetic SeqFeatPtr of type ImpFeat for use in printing.
*
*	This function puts the GI number on a SeqFeatPtr (sfp) of type CDS.
*	Checking is first done to see if this sfp is indeed a CDS, then
*	the GI number is gotten from the twin BiotablePtr for amino acids
*	(see note on GetProductGeneInfo).
*****************************************************************************/

void AddGINumToNote (BiotablePtr btp_na, Int2 count, SeqFeatPtr sfp_out, Int2 ext_index)

{
	BioseqPtr bsp;
	BiotablePtr btp_aa = btp_na->btp_other;
	Char buffer[20];
	NoteStructPtr nsp;
	ImpFeatPtr ifp;
	Int2 index;
	Int4 gi = -1;
	SeqIdPtr sip;
	ValNodePtr product;

	ifp = sfp_out->data.value.ptrvalue;
	if (StringCmp(ifp->key, "CDS") == 0)
	{
		product = sfp_out->product; 
		if (product)
		{
			sip = GetProductSeqId(product);
			bsp = BioseqFind(sip);
			if (bsp)
			{
				for (index=0; index<btp_aa->count; index++)
					if (bsp == btp_aa->bsp[index])
					{
						nsp=btp_na->nspp[count][ext_index];
						gi = btp_aa->gi[index];
						break;
					}
			}
			else 
			{	/* For GenBankSelect the product is missing */
				gi = GetGINumFromSip(sip);
				nsp=btp_na->nspp[count][ext_index];
			}
			if (gi != -1)
			{
				sprintf(buffer, "NCBI gi: %ld", (long) gi);
				SaveNoteToCharPtrStack(nsp, NULL, buffer);
			}
		}
	}
	return;
}	/* AddGINumToNote */

/***************************************************************************
*Int2 MakeGBSelectNote (CharPtr ptr, SeqFeatPtr sfp)
*
*Adds note to CDS GenBankSelect 
***************************************************************************/
Int2 MakeGBSelectNote (CharPtr ptr, SeqFeatPtr sfp)

{
	Boolean found_select=FALSE, found_match=FALSE;
	CharPtr acc=NULL;
	Int2 number = -1;
	ObjectIdPtr oip=NULL, type;
	UserFieldPtr ufp;
	UserObjectPtr uop=NULL;

	if (sfp && (uop=sfp->ext) != NULL)
	{
		if (uop->_class && (type=uop->type) != NULL)
		{
			if (StringCmp(uop->_class, "GB-Select") == 0)
				found_select = TRUE;
			if (type->str) 
				if (StringCmp(type->str, "SPmatch") == 0)
					found_match = TRUE;
			if (found_match && found_select)
			{
				for (ufp=uop->data; ufp; ufp=ufp->next)
				{
					oip = ufp->label;
					if (oip->id == 2)
					{
						if (ufp->choice == 1)
							acc = ufp->data.ptrvalue;
					}
					else if (oip->id == 3)
					{
						if (ufp->choice == 2)
						{
							number = (Int2) (ufp->data.intvalue);
						}
					}
						
				}
				if (number == 1)
					sprintf(ptr, 
						"Identical to Swiss-Prot Accession Number %s", acc);
				else if (number == 2 || number == 3)
					sprintf(ptr, 
						"Similar to Swiss-Prot Accession Number %s", acc);
			}
		}
	}
	return number;
}

/**************************************************************************
* void GetProductGeneInfo (SeqFeatPtr sfp_in, BiotablePtr btp_na, Int2 count, Int2 ext_index)
*
*	sfp_in: any type of SeqFeatPtr (sfp).
*	btp: BiotablePtr containing all info.
*	count: entry number of sequence in btp.
*
*	This function finds, for a given sfp, whether it has a product
*	and whether this product has relevant "gene" information
*	associated with it.  
*
*	This function is complicated by the nature of the BiotablePtr's
*	(btp's): a btp applies either to the nucleic acid entries of a seq-set
*	or to the amino acid entries of a seq-set.  sfp_in is a nucleic acid
*	sfp and hence on btp_na, but it's product (a protein) is on btp_aa.  
*	The address of btp_aa is given by btp_na->btp_other (conversely that
*	of btp_na is given in btp_aa->btp_other).  Using BioseqFind and 
*	BioseqContextGetSeqFeat the protein product(s) is found and 
*	the largest one is selected.  "GetProtRefInfo" then extracts
*	any relevant "gene" information on the product.
*************************************************************************/

void GetProductGeneInfo (SeqFeatPtr sfp_in, BiotablePtr btp_na, Int2 count, Int2 ext_index)

{
	BioseqPtr bsp;
	BioseqContextPtr bcp_aa=NULL;
	BiotablePtr btp_aa=btp_na->btp_other;
	GeneStructPtr gsp=btp_na->gspp[count][ext_index];
	Int2 currentprot=0, count_other, index, status;
	Int4 length, longest_length=0;
	NoteStructPtr nsp=btp_na->nspp[count][ext_index];
	ProtRefPtr prot=NULL;
	SeqFeatPtr sfp=NULL;
	SeqIdPtr sip;
	ValNodePtr product=NULL, vnp;
	static SortStructPtr Protlist;
	static Int2 totalprot=0;

	if (sfp_in->product)
		product = sfp_in->product;
	else 
		return;
	
	sip = GetProductSeqId(product);
	if (sip)
	{	/* Get protein bsp	*/
		bsp = BioseqFind(sip);
		if (bsp)	/* Get the bcp for the protein bsp */
			bcp_aa = FindBioseqContextPtr(btp_aa, bsp, &count_other);
		if (bcp_aa)
		{
/* The following  lines collect protein pub info, needed for the note.  */
			if (btp_aa->Pub[count_other] == NULL)
			{
				vnp=NULL;
				status = StorePubInfo(bcp_aa, bsp, &vnp, btp_na->error_msgs);
				if (status >= 0)
					btp_aa->Pub[count_other] = vnp;
			}
			sfp=BioseqContextGetSeqFeat(bcp_aa, 4, sfp, NULL, 0);
			while (sfp)
			{
				if (currentprot == totalprot)
				{
					if (totalprot == 0)
					Protlist = EnlargeSortList(NULL, &(totalprot), 5);
					else
					Protlist = EnlargeSortList(Protlist, &(totalprot), 5);
				}
				currentprot = CpSfpToList(Protlist, sfp, btp_na->btp_other, currentprot, count_other);
				sfp=BioseqContextGetSeqFeat(bcp_aa, 4, sfp, NULL, 0);
			}
			if (currentprot)	/* not zero if one found. */
			{
				prot = NULL;
				for (index=0; index<currentprot; index++)
				{
					sfp = Protlist[index].sfp;
					if ((length=SeqLocLen(sfp->location)) == -1)
						continue;
					if (length > longest_length)
					{
						prot = sfp->data.value.ptrvalue;
						longest_length = length;
					}
				}
			}
			GetProtRefInfo(gsp, nsp, prot);
			GetProtRefComment(sfp_in, btp_na->btp_other, count_other, nsp);
		}
	}
	return;
	/* What about deallocating "Protlist"???????????????*/
}	/* GetProductGeneInfo */


/**************************************************************************
*	void GetCdregionGeneXrefInfo (SeqFeatPtr sfp, BiotablePtr btp, Int2 count, Int2 index)
*
*	sfp: SeqFeatPtr on which one wishes to find "gene: cross-reference
*		information.
*	btp: BiotablePtr for nucleic acids.
*
*	Look on a sfp for sfp->xref and, if it exists, get information
*	about genes or proteins from it; put this information on
*	the GeneStructPtr (gsp), the address of which is given in btp.
**************************************************************************/

void GetCdregionGeneXrefInfo (SeqFeatPtr sfp, BiotablePtr btp, Int2 count, Int2 index)

{
	GeneRefPtr grp, grp1;
	GeneStructPtr gsp=btp->gspp[count][index], first_gsp=btp->gspp[count][0];
	Int2 index1;
	NoteStructPtr nsp=btp->nspp[count][index];
	ProtRefPtr prp;
	SeqFeatPtr gene_sfp;
	SeqFeatXrefPtr xrp, xrp1;

	if (sfp->xref)
		xrp = sfp->xref;
	else
		return;

	for (xrp1=xrp; xrp1; xrp1=xrp1->next)
	{
		if (xrp1->data.choice == 1)
		{
			grp = (GeneRefPtr) xrp1->data.value.ptrvalue;
			GetGeneRefInfo(gsp, nsp, grp);
			for (index1=0; index1<btp->sfpGenesize[count]; index1++)
			{
			    gene_sfp = btp->sfpGenelist[count][index1];
			    grp1 = (GeneRefPtr) (gene_sfp->data.value.ptrvalue);
			    if (grp->locus && grp1->locus)  
				if (StringCmp(grp->locus, grp1->locus) == 0 )
			    {
			    	first_gsp->genes_matched[index1]++;
			    }
			    if (grp->allele && grp1->allele)  
				if (StringCmp(grp->allele, grp1->allele) == 0 )
			    {
			    	first_gsp->genes_matched[index1]++;
			    }
			    if (grp->desc && grp1->desc)  
				if (StringCmp(grp->desc, grp1->desc) == 0 )
			    {
			    	first_gsp->genes_matched[index1]++;
			    }
			    if (grp->maploc && grp1->maploc)  
				if (StringCmp(grp->maploc, grp1->maploc) == 0 )
			    {
			    	first_gsp->genes_matched[index1]++;
			    }
			}
		}
	}


	for (xrp1=xrp; xrp1; xrp1=xrp1->next)
	{
		if (xrp1->data.choice == 4)
		{
			prp = (ProtRefPtr) xrp1->data.value.ptrvalue;
			GetProtRefInfo(gsp, nsp, prp);
		}
	}

	return;
}	/* GetCdregionGeneXrefInfo */


/********************************************************************
*	Int2 CompareStringWithGsp (GeneStructPtr gsp, CharPtr string)
*
*	gsp: GeneStructPtr containing the gene information,
*	string: a CharPtr with (possibly) relevant gene information
*		(i.e., gene name, allele, product etc.).
*
*	A comparison is made between string and the information already
*	stored in the gsp.  Following the convention for StringCmp,
*	"0" is returned if a match is found, otherwise "1" is returned.
*	At present (2/7/94) GeneStringCmp is a #define for StringCmp.
************************************************************************/

Int2 CompareStringWithGsp (GeneStructPtr gsp, CharPtr string)

{
	CharPtr ascii, start;
	Int2 ascii_len;
	ValNodePtr vnp;	

	for (vnp=gsp->gene; vnp; vnp=vnp->next)
	{
		ascii_len = Sgml2AsciiLen(vnp->data.ptrvalue);
		start = ascii = MemNew((size_t) (10+ascii_len));
		ascii = Sgml2Ascii(vnp->data.ptrvalue, ascii, ascii_len+1);
		if (GeneStringCmp(start, string) == 0)
		{
			start = MemFree(start);
			return 0;
		}
		start = MemFree(start);
	}
	vnp=gsp->product;
	if (vnp != NULL)
	{
		if (GeneStringCmp(vnp->data.ptrvalue, string) == 0)
			return 0;
	}
	for (vnp=gsp->standard_name; vnp; vnp=vnp->next)
	{
		if (GeneStringCmp(vnp->data.ptrvalue, string) == 0)
			return 0;
	}
	if (gsp->map[0] && GeneStringCmp(gsp->map[0], string) == 0)
		return 0;
	if (gsp->ECNum)
	for (vnp=gsp->ECNum; vnp; vnp=vnp->next)
	{
		if (GeneStringCmp(vnp->data.ptrvalue, string) == 0)
			return 0;
	}

	return 1;
}	/* CompareStringWithGsp */

/****************************************************************************
*	void GetGeneRefInfo (GeneStructPtr gsp, NoteStructPtr nsp, GeneRefPtr grp)
*
*	gsp: GeneStructPtr containing gene information
*	grp: GeneRefPtr from a sfp of type gene or a sfp xref.
*
*	If fields are empty on the gsp, and the relevant information
*	is given by the grp, that field is filled on the gsp
****************************************************************************/

void GetGeneRefInfo (GeneStructPtr gsp, NoteStructPtr nsp, GeneRefPtr grp)

{
	CharPtr ascii, start;
	DbtagPtr dbtp;
	Int2 ascii_len;
	
	if (grp)
	{
		if (grp->locus != NULL)
		{
			if (gsp->gene)
				ValNodeAddStr(&(gsp->gene), 0, grp->locus);
			else
				gsp->gene = ValNodeAddStr(&(gsp->gene), 0, grp->locus);
		}
		if (gsp->map[0] == NULL && grp->maploc)
			gsp->map[0] = grp->maploc;
		if (grp->desc)
			CpNoteToCharPtrStack(nsp, "Description:", grp->desc);
		if (grp->allele)
		{
			ascii_len = Sgml2AsciiLen(grp->allele);
			start = ascii = MemNew((size_t) (10+ascii_len));
			ascii = Sgml2Ascii(grp->allele, ascii, ascii_len+1);
			SaveNoteToCharPtrStack(nsp, "Allele:", start);
			start = MemFree(start);
		}
		if (grp->db)
		{
		    dbtp = grp->db->data.ptrvalue;
		    if (dbtp)
		    {
		    	if (dbtp->db && StringCmp(dbtp->db, "GDB") == 0)
			    if (dbtp->tag && dbtp->tag->str)
			    	SaveNoteToCharPtrStack(nsp, "gdb_xref:", dbtp->tag->str);
		    }
		}
		gsp->pseudo = FALSE;
		if (grp->pseudo)
		{
			gsp->pseudo = TRUE;
		}
	}

	return;
}

/****************************************************************************
*	void GetProtRefInfo (GeneStructPtr gsp, NoteStructPtr nsp, ProtRefPtr prp)
*
*	gsp: GeneStructPtr containing gene information
*	prp: ProtRefPtr from a sfp of type protein or a sfp xref.
*
*	If fields are empty on the gsp, and the relevant information
*	is given by the prp, that field is filled on the gsp
****************************************************************************/
void GetProtRefInfo (GeneStructPtr gsp, NoteStructPtr nsp, ProtRefPtr prp)

{
	CharPtr string=NULL;
	ValNodePtr name, vnp;

	if (prp)
	{
		name = prp->name;
		if (name)
		{
			for (vnp=name; vnp; vnp=vnp->next)
			{ 
				string = (CharPtr)vnp->data.ptrvalue;
				if (gsp->product)
					ValNodeAddStr(&(gsp->product), 0, string);
				else
					gsp->product = ValNodeAddStr(&(gsp->product), 0, string);
			}
		}
		for (vnp=prp->ec; vnp; vnp=vnp->next)
		{
			if (gsp->ECNum)
				ValNodeAddStr(&(gsp->ECNum), 0, vnp->data.ptrvalue);
			else
				gsp->ECNum = ValNodeAddStr(&(gsp->ECNum), 0, vnp->data.ptrvalue);
		}
		if (prp->desc)
		{
			SaveNoteToCharPtrStack(nsp, NULL, prp->desc);
		}
	}

	return;
}

/****************************************************************************
*	void GetProtRefComment (SeqFeatPtr sfp, BiotablePtr btp, Int2 count, NoteStructPtr nsp)
*
*	sfp: SeqFeatPtr for CDS
*	btp: BiotablePtr for the PROTEIN!!!!
*	count: index for the PROTEIN!!!!
*	nsp: NoteStructPtr 
*
* Used to get comments from the Protein for use in a CDS /note.
*
* Will find the Protein Pubs, as they are needed and (presumably) haven't
* been found yet, so as to save "upfront" time when the formatter is 
* running in Entrez.
****************************************************************************/
void GetProtRefComment (SeqFeatPtr sfp, BiotablePtr btp, Int2 count, NoteStructPtr nsp)

{
	BioseqContextPtr bcp_aa;
	BioseqPtr bsp=btp->bsp[count];
	Boolean first_done=FALSE, protein=FALSE;
	Char buffer[60];
	CharPtr ptr = &buffer[0], string=NULL, string1=NULL, newstring=NULL, temp;
	CharPtr conflict_msg_no_protein="Author-provided coding region translates with internal stops. ";
	CharPtr except_msg_no_protein="Author-provided coding region translates with internal stops for reasons explained in citation. ";
	CharPtr conflict_msg="Author-given protein sequence is in conflict with the conceptual translation. ";
	CharPtr except_msg="Author-given protein sequence differs from the conceptual translation for reasons explained in citation. ";
	CdRegionPtr cdr=NULL;
	Int2 total=0;
	PubdescPtr pdp;
	PubStructPtr psp;
	SeqFeatPtr sfp_local=NULL;
	ValNodePtr descr, vnp, vnp1, product;

	bcp_aa = btp->bcp[count];
	if (bcp_aa)
	{
		sfp_local=BioseqContextGetSeqFeat(bcp_aa, 10, sfp_local, NULL, 0);
		while (sfp_local)
		{ /* Get protein sfp's that are comments */
			if (first_done)
			{
				if (StringLen(sfp_local->comment))
				{
					string1 = CheckEndPunctuation(sfp_local->comment, '.');
					newstring = Cat2Strings(string, string1, " ", 0);
					string = MemFree(string);
					string1 = MemFree(string1);
					string = newstring;
				}
			}
			else
			{
				if (StringLen(sfp_local->comment))
				{
					string = CheckEndPunctuation(sfp_local->comment, '.');
					first_done = TRUE;
				}
			}
			sfp_local=BioseqContextGetSeqFeat(bcp_aa, 10, sfp_local, NULL, 0);
		}
	}
	/* Get comments ONLY in the protein Bioseq */

	if (bsp && (descr=bsp->descr) != NULL)
	{
		for (vnp=descr; vnp; vnp=vnp->next)
		{
			if (vnp->choice == Seq_descr_comment)
			{
				if (first_done)
				{
					if (StringLen(vnp->data.ptrvalue))
					{
						string1 = CheckEndPunctuation(vnp->data.ptrvalue, '.');
						newstring = Cat2Strings(string, string1, " ", 0);
						string = MemFree(string);
						string1 = MemFree(string1);
						string = newstring;
					}
				}
				else
				{
					if (StringLen(vnp->data.ptrvalue))
					{
						string = CheckEndPunctuation(vnp->data.ptrvalue, '.');
						first_done = TRUE;
					}
				}
			}
			else if (vnp->choice == Seq_descr_method)
			{
				if (vnp->data.intvalue > 1)
				{
					sprintf(ptr, "Method: %s.", StringForSeqMethod( (Int2) vnp->data.intvalue)); 
		
					if (first_done)
					{
						newstring = Cat2Strings(string, ptr, " ", 0);
						string = MemFree(string);
						string = newstring;
					}
					else
					{
						string = StringSave(ptr);
						first_done = TRUE;
					}
				}
			}
		}
	}


	vnp = btp->Pub[count];
	for (vnp1=vnp; vnp1; vnp1=vnp1->next)
	{
		psp = vnp1->data.ptrvalue;
		if ((pdp=psp->descr) != NULL)
		{
			if (pdp->fig)
			{
				total += 30;
				total += StringLen(pdp->fig);
			}
			if (pdp->maploc)
			{
				total += 15;
				total += StringLen(pdp->maploc);
			}
		}
	}

	if (sfp)
	{
		cdr = (CdRegionPtr) sfp->data.value.ptrvalue;
		product = sfp->product;
		if (product && SeqLocLen(product)) 
			protein = TRUE;
		if (sfp->excpt)
			total += 110;
		if (cdr && cdr->conflict && (protein || ! sfp->excpt))
			total += 110;
	}

	string1 = (CharPtr) MemNew(total*sizeof(Char));

	vnp = btp->Pub[count];
	for (vnp1=vnp; vnp1; vnp1=vnp1->next)
	{
		psp = vnp1->data.ptrvalue;
		if ((pdp=psp->descr) != NULL)
		{
			if (pdp->fig)
			{
				temp = CheckEndPunctuation(pdp->fig, '.');
				total = StringLen(string1);

				sprintf(string1+total, "This sequence comes from %s  ", temp);
				temp = MemFree(temp);
			}
			if (pdp->maploc)
			{
				total = StringLen(string1);
				sprintf(string1+total, "Map location %s.  ", pdp->maploc);
			}
		}
	}

	if (sfp)
	{
		if (sfp->excpt)
		{
			total = StringLen(string1);
			if (protein)
				sprintf(string1+total, except_msg);	
			else
				sprintf(string1+total, except_msg_no_protein);	
		}
	
		if (cdr && cdr->conflict && (protein || ! sfp->excpt))
		{
			total = StringLen(string1);
			sprintf(string1+total, protein?conflict_msg:conflict_msg_no_protein);
		}
	}
	
	if (string && string1)
	{
		newstring = Cat2Strings(string, string1, " ", 0);
		string = MemFree(string);
		string1 = MemFree(string1);
	}
	else if (string)
		newstring = string;
	else if (string1)
		newstring = string1;

	if (newstring)
	{
		SaveNoteToCharPtrStack(nsp, NULL, newstring);
		newstring = MemFree(newstring);
	}

	return;
}	/* GetProtRefComment */

/****************************************************************************
*	Int2 MiscFeatOrphanGenes (BiotablePtr btp, Int2 count, SeqFeatPtr sfp, Int2 index)
*
*	btp: BiotablePtr,
*	count: index of bsp in btp,
*	sfp: "synthetic" SeqFeatPtr, which was built in order to output
*		a misc_feat ImpFeatPtr to cover an orphaned gene.
*	index: index in the array gsp->genes_matched, index should
*	be zero on the first call of this function, which looks for orphaned
*	genes in the "genes_matched" list.  If one is found, a "misc_feat"
*	is built and index, incremented by "1", is returned.  This index
*	should be used on the next call of this function.  If no more
*	"orphan" genes are found, "-1" is returned.
***************************************************************************/

Int2 MiscFeatOrphanGenes (BiotablePtr btp, Int2 count, SeqFeatPtr sfp, Int2 index)

{
	Boolean assigned=FALSE;
	GeneRefPtr grp;
	GeneStructPtr first_gsp=btp->gspp[count][0], gsp;
	ImpFeatPtr ifp = sfp->data.value.ptrvalue;
	Int2 index1, size=btp->sfpListsize[count];
	NoteStructPtr nsp;
	SeqFeatPtr gene_sfp;

	gsp = btp->gspp[count][size];

	for (index1=index; index1<btp->sfpGenesize[count]; index1++)
	{
		if (first_gsp->genes_matched[index1] == -1)
		{
			if (sfp->qual)
				sfp->qual = GBQualFree(sfp->qual);
			ifp->key = StringCpy(ifp->key, "misc_feature");
			gene_sfp = btp->sfpGenelist[count][index1];
			grp = (GeneRefPtr) (gene_sfp->data.value.ptrvalue);
			GeneStructReset(btp, count, size);
			nsp = btp->nspp[count][size];
			NoteStructReset(nsp);
			sfp->comment = gene_sfp->comment;
			if (sfp->comment)
				CpNoteToCharPtrStack(nsp, NULL, sfp->comment);
			GetGeneRefInfo(gsp, nsp, grp);
			ComposeGBQuals(sfp, btp, count, size);
			sfp->exp_ev = gene_sfp->exp_ev;
			sfp->location = gene_sfp->location;
			sfp->partial = gene_sfp->partial;
			sfp->product = gene_sfp->product;
			assigned = TRUE;
			break;
		}
	}
	index1++;

	if (assigned == FALSE)
		return -1;
	else
		return index1;
}	/* MiscFeatOrphanGenes */

/****************************************************************************
*	Int2 CheckForQual(GBQualPtr gbqual, CharPtr string_q, CharPtr string_v)
*
*	Compares string (a potential gbqual->val) against all gbquals.
*	If a match is found, "1" is returned; if not "0".
****************************************************************************/

Int2 CheckForQual (GBQualPtr gbqual, CharPtr string_q, CharPtr string_v)

{
	Boolean match=FALSE;
	GBQualPtr curq;

	if (gbqual)
	{
		for (curq=gbqual; curq; curq=curq->next)
			if (StringCmp(string_q, curq->qual) == 0 && 
				StringCmp(string_v, curq->val) == 0)
			{
				match = TRUE;
				break;
			}
	}
	if (match == TRUE)
		return 1;
	else
		return 0;
}

GBQualPtr AddModifsToGBQual (BiotablePtr btp, Int2 count, GBQualPtr gbqual)
{
	CharPtr ptr;
	ValNodePtr descr, man;

	descr=BioseqContextGetSeqDescr(btp->bcp[count], Seq_descr_modif, NULL, NULL); 
	if (descr) {
		for (man = (ValNodePtr) descr-> data.ptrvalue; man != NULL; man = man -> next){
			switch (man -> data.intvalue){
			case 3: case 14: 
				ptr = AsnEnumStr("GIBB-mod", 
		  		   (Int2) man->data.intvalue);
				if (GBQualPresent(ptr, gbqual) == FALSE)
					gbqual = AddGBQual(gbqual, ptr, " ");
				break;
			case 4:
				if (GBQualPresent("mitochondrion", gbqual) == FALSE)
					gbqual = AddGBQual(gbqual, "mitochondrion", NULL);
				break;
			case 15:
				if (GBQualPresent("insertion_seq", gbqual) == FALSE)
					gbqual = AddGBQual(gbqual, "insertion_seq", " ");
				break;
			case 5: case 6: case 7: case 18: case 19:
				ptr = AsnEnumStr("GIBB-mod", 
		  		   (Int2) man->data.intvalue);
				if (GBQualPresent(ptr, gbqual) == FALSE)
					gbqual = AddGBQual(gbqual, ptr, NULL);
				break;
			default:
				break;
			}
		}
	}
	return gbqual;
}	/* AddModifsToGBQual */

/*************************************************************************
*GBQualPtr AddOrgRefModToGBQual (OrgRefPtr orp, GBQualPtr gbqual);
*
*Add the OrgRef.mod to a source feat.  Note: a few of the quals added
*may be illegal for a source feature, but the validator will catch them 
*in the end.
***************************************************************************/
GBQualPtr AddOrgRefModToGBQual (OrgRefPtr orp, GBQualPtr gbqual)

{
	CharPtr mod, ptr, temp_ptr;
	Char temp[ASN2FF_STD_BUF]; /* ASN2FF_STD_BUF (now 35) is longer than 
any qual. */
	Int2 index;
	ValNodePtr vnp;

	if (orp && orp->mod)
	{
		for (vnp=orp->mod; vnp; vnp=vnp->next)
		{
			mod = vnp->data.ptrvalue;
			if (StringNCmp(mod, "citation", 8) == 0)
				continue;
			index=0;
			for (ptr=mod; *ptr != '\0'; ptr++)
			{
				index++;
				if (*ptr == ' ' || *ptr == '=')
				{
					ptr++;
					index--;
					break;
				}
			}
			if (index > ASN2FF_STD_BUF-1)
				continue;

			temp_ptr = &(temp[0]);
			StringNCpy(temp_ptr, mod, index);
			temp[index] = '\0';
			if ((GBQualNameValid(temp_ptr)) == -1)
				continue;
			if (ptr)
				gbqual = AddGBQual(gbqual, temp_ptr, ptr);
			else
				gbqual = AddGBQual(gbqual, temp_ptr, NULL);
		}
	}
	return gbqual;
}	/* AddOrgRefModToGBQual */

/*************************************************************************
*GBQualPtr AddBioSourceToGBQual (BioSourcePtr biosp, GBQualPtr gbqual);
*
*Add the OrgMod.subtypes and SubSource.subtypes to a source feat.
*Add BioSource.genome to a source feat.  
*Note: a few of the quals added may be illegal for a source feature, 
*but the validator will catch them in the end.
***************************************************************************/
GBQualPtr AddBioSourceToGBQual (BioSourcePtr biosp, GBQualPtr gbqual)

{
	CharPtr qual, val = NULL;
	OrgModPtr vnp;
	OrgNamePtr onp;
	SubSourcePtr ssp;
	Int2 i;

	if (biosp == NULL)
		return NULL;
	if (biosp->genome) {
		i = biosp->genome;
		if (i > 1) {
			qual = genome[i];
			if (qual && (GBQualNameValid(qual)) != -1) {
				gbqual = AddGBQual(gbqual, qual, val);
			}
		}
	}
	if (biosp->org)
	{
		onp = (OrgNamePtr) biosp->org->orgname;
		if (onp) 
		{
			for (vnp=onp->mod; vnp != NULL; vnp=vnp->next)
			{
				for (i=0; orgmod_subtype[i].name != NULL; i++) {
					if (vnp->subtype == orgmod_subtype[i].num)
						break;
				}
				qual = orgmod_subtype[i].name;
				if ((val = vnp->subname) == NULL)
					val = "";
				
				if ((GBQualNameValid(qual)) == -1)
					continue;
				gbqual = AddGBQual(gbqual, qual, val);
			}
		}
	}
	for (ssp = biosp->subtype; ssp != NULL; ssp=ssp->next) {
		qual = subtype[ssp->subtype - 1];
		if (ssp->subtype != 14 && ssp->subtype != 15) {
			if ((val = ssp->name) == NULL)
				val = "";
			}
		if ((GBQualNameValid(qual)) == -1)
			continue;
		gbqual = AddGBQual(gbqual, qual, val);
		
	}
	return gbqual;
}	/* AddBioSourceToGBQual */

/****************************************************************************
*PrintImpFeat
*
*	This code prints out an ImpFeat in GenBank format.
*		written by Tom Madden
*
*
****************************************************************************/

Int2 PrintImpFeat (BiotablePtr btp, Int2 count, SeqFeatPtr sfp)

{
	Boolean evidence_present;
	CharPtr flatloc_ptr, key, loc;
	GBQualPtr gbqp;
	ImpFeatPtr ifp;
	Uint1 class_qual, format=btp->format;
	Int2 class_equal, gbqual_index;
	extern Uint1 ParFlat_GBQual_class[ParFlat_TOTAL_GBQUAL];


	if (sfp == NULL)
		return -1;
	if (sfp->data.choice != 8)
		return -1;
	ifp = (ImpFeatPtr) sfp->data.value.ptrvalue;
	key = ifp->key;
	loc = ifp->loc;

	if (format == EMBL)
		StartPrint(btp, 5, 21, ASN2FF_EMBL_MAX, "FT");
	else
		StartPrint(btp, 5, 21, ASN2FF_GB_MAX, NULL);
	AddString(btp, key);
	TabToColumn(btp, 22);
	if (loc == NULL)
	{
		flatloc_ptr = FlatLoc(btp, count, sfp->location);
		AddString(btp, flatloc_ptr);
		MemFree(flatloc_ptr);	/* FlatLoc allocates this */
	}
	else
	{
		AddString(btp, loc);
	}

	if (sfp->partial == TRUE)
	{
		NewContLine(btp);
		AddString(btp, "/partial");
	}

	evidence_present = GBQualPresent("evidence", sfp->qual);
	if (sfp->exp_ev)
	{
		if (evidence_present == FALSE)
		{
			NewContLine(btp);
			AddString(btp, "/evidence");
			AddChar(btp, '=');
			if (sfp->exp_ev == 1)
				AddString(btp, "experimental");
			if (sfp->exp_ev == 2)
				AddString(btp, "not_experimental");
		}
		else
		{
			for (gbqp=sfp->qual; gbqp; gbqp=gbqp->next)
				if (StringCmp(gbqp->qual, "evidence") == 0)
				{
					gbqp->val = MemFree(gbqp->val);
					if (sfp->exp_ev == 1)
						gbqp->val = StringSave("experimental");
					if (sfp->exp_ev == 2)
						gbqp->val = StringSave("not_experimental");
					break;
				}
		}
	}
	else if (evidence_present == TRUE)
	{
		for (gbqp=sfp->qual; gbqp; gbqp=gbqp->next)
			if (StringCmp(gbqp->qual, "evidence") == 0)
			{
				if (StringCmp(gbqp->val, "EXPERIMENTAL") == 0)
				{
					StringCpy(gbqp->val, "experimental");
				}
				else if (StringCmp(gbqp->val, "NOT_EXPERIMENTAL") == 0)
				{
					StringCpy(gbqp->val, "not_experimental");
				}
				break;
			}
	}


	for (gbqp=sfp->qual; gbqp; gbqp=gbqp->next)
	{
		gbqual_index = GBQualNameValid(gbqp->qual);
		if (gbqual_index != -1)
		{
			NewContLine(btp);
			AddChar(btp, '/');
			AddString(btp, gbqp->qual);
			class_qual = ParFlat_GBQual_class[gbqual_index];
			if (class_qual == Class_none)
			{
				class_equal=CheckForEqualSign(gbqp->qual);
				if (class_equal == 1)
					continue;
			}
			AddChar(btp, '=');
			if (class_qual == Class_text && 
				StringCmp(gbqp->val, "\"\"") == 0)
			{ /* an empty string is considered legal */ 
				AddString(btp, gbqp->val);
				continue;
			}
			if (class_qual == Class_text || class_qual == Class_none
				|| class_qual == Class_ecnum || class_qual == Class_note)
				AddString(btp, "\"");
			if (class_qual != Class_none)
				AddString(btp, gbqp->val);
			if (class_qual == Class_text || class_qual == Class_none
				|| class_qual == Class_ecnum || class_qual == Class_note)
				AddString(btp, "\"");
		}
		else if (format == GENPEPT)
		{
			if (StringCmp(gbqp->qual, "site_type") == 0)
			{
				NewContLine(btp);
				AddChar(btp, '/');
				AddString(btp, gbqp->qual);
				AddChar(btp, '=');
				AddString(btp, "\"");
				AddString(btp, gbqp->val);
				AddString(btp, "\"");
			}
			else if (StringCmp(gbqp->qual, "bond_type") == 0)
			{
				NewContLine(btp);
				AddChar(btp, '/');
				AddString(btp, gbqp->qual);
				AddChar(btp, '=');
				AddString(btp, "\"");
				AddString(btp, gbqp->val);
				AddString(btp, "\"");
			}
			else if (StringCmp(gbqp->qual, "region_name") == 0)
			{
				NewContLine(btp);
				AddChar(btp, '/');
				AddString(btp, gbqp->qual);
				AddChar(btp, '=');
				AddString(btp, "\"");
				AddString(btp, gbqp->val);
				AddString(btp, "\"");
			}
			else if (StringCmp(gbqp->qual, "sec_str_type") == 0)
			{
				NewContLine(btp);
				AddChar(btp, '/');
				AddString(btp, gbqp->qual);
				AddChar(btp, '=');
				AddString(btp, "\"");
				AddString(btp, gbqp->val);
				AddString(btp, "\"");
			}
			else if (StringCmp(gbqp->qual, "non-std-residue") == 0)
			{
				NewContLine(btp);
				AddChar(btp, '/');
				AddString(btp, gbqp->qual);
				AddChar(btp, '=');
				AddString(btp, "\"");
				AddString(btp, gbqp->val);
				AddString(btp, "\"");
			}
			else if (StringCmp(gbqp->qual, "heterogen") == 0)
			{
				NewContLine(btp);
				AddChar(btp, '/');
				AddString(btp, gbqp->qual);
				AddChar(btp, '=');
				AddString(btp, "\"");
				AddString(btp, gbqp->val);
				AddString(btp, "\"");
			}
			else if (StringCmp(gbqp->qual, "name") == 0)
			{
				NewContLine(btp);
				AddChar(btp, '/');
				AddString(btp, gbqp->qual);
				AddChar(btp, '=');
				AddString(btp, "\"");
				AddString(btp, gbqp->val);
				AddString(btp, "\"");
			}
			else if (StringCmp(gbqp->qual, "coded_by") == 0)
			{
				NewContLine(btp);
				AddChar(btp, '/');
				AddString(btp, gbqp->qual);
				AddChar(btp, '=');
				AddString(btp, "\"");
				AddString(btp, gbqp->val);
				AddString(btp, "\"");
			}
		}
		else if (ASN2FF_VALIDATE_FEATURES == FALSE)
		{
			NewContLine(btp);
			AddChar(btp, '/');
			AddString(btp, gbqp->qual);
			if (gbqp->val != NULL && StringLen(gbqp->val) != 0)
			{
				AddChar(btp, '=');
				AddString(btp, "\"");
				AddString(btp, gbqp->val);
				AddString(btp, "\"");
			}
		}
	}

	EndPrint(btp);

	return 1;
}	/* PrintImpFeat */
		
#define NOEQUALTOTAL 11
Int2 CheckForEqualSign(CharPtr qual)
				/* this have to be changed. Tatiana 02.28.95 */
{
	Int2 i;
	static CharPtr NoEqualSign[NOEQUALTOTAL] = {
	"chloroplast",
	"chromoplast",
	"cyanelle", 
	"germline",
	"kinetoplast",
	"macronuclear", 
	"mitochondrion",
	"partial",
	"proviral",
	"pseudo",
	"rearranged"
	};

	if (qual == NULL)
		return -1;

	for (i=0; i < NOEQUALTOTAL; i++)
		if (StringICmp(qual, NoEqualSign[i]) == 0)
			return 1;

	return 0;

}

