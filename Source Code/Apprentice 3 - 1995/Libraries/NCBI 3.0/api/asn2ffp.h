/*****************************************************************************
*
*	Header file for asn2gb files.
*
****************************************************************************/
/* $Revision: 1.40 $ */ 
/*************************************
*
* $Log: asn2ffp.h,v $
 * Revision 1.40  1995/05/22  16:14:20  tatiana
 * add ASN2FF_SHOW_ALL_PUBS to asn2ff_flags
 *
 * Revision 1.39  1995/05/15  21:46:05  ostell
 * added Log line
 *
*
**************************************/
#ifndef _ASN2FFP_
#define _ASN2FFP_

#include <asn.h>
#include <objall.h>
#include <objpubd.h>
#include <objsub.h>
#include "asn2ff.h"
#include "asn2ff6.h"
#include "utilpars.h"
#include "gbfeat.h"
#include "gbftdef.h"
#include "a2ferrdf.h"
#include "ftusrstr.h"

#define GENBANK ( (Uint1)0)
#define EMBL ( (Uint1)1)
#define GENPEPT ( (Uint1)2)
#define PSEUDOEMBL ( (Uint1)3)
#define SELECT ( (Uint1)4)
#define EMBLPEPT ( (Uint1)5)

extern Boolean asn2ff_flags[11];

#define ASN2FF_LOCAL_ID                 asn2ff_flags[0]
#define ASN2FF_LOOK_FOR_SEQ             asn2ff_flags[1]
#define ASN2FF_VALIDATE_FEATURES        asn2ff_flags[2]
#define ASN2FF_IGNORE_PATENT_PUBS       asn2ff_flags[3]
#define ASN2FF_DROP_SHORT_AA            asn2ff_flags[4]
#define ASN2FF_AVOID_LOCUS_COLL         asn2ff_flags[5]
#define ASN2FF_DATE_ERROR_MSG           asn2ff_flags[6]
#define ASN2FF_IUPACAA_ONLY             asn2ff_flags[7]
#define ASN2FF_TRANSL_TABLE             asn2ff_flags[8]
#define ASN2FF_REPORT_LOCUS_COLL        asn2ff_flags[9]
#define ASN2FF_SHOW_ALL_PUBS	        asn2ff_flags[10]


void EMBL_PrintPubs PROTO ((BiotablePtr btp, Int2 count, PubStructPtr psp));
void GB_PrintPubs PROTO ((BiotablePtr btp, Int2 count, PubStructPtr psp));
Boolean FlatIgnoreThisPatentPub PROTO ((BioseqPtr bsp, ValNodePtr best, Int4Ptr seqidPt));
CharPtr FlatCleanEquals PROTO ((CharPtr retval));
ValNodePtr GetAuthors PROTO ((BiotablePtr btp, ValNodePtr the_pub));
CharPtr GetExtraAccessions PROTO ((BiotablePtr btp, Int2 count));
CharPtr FlatJournal PROTO ((BiotablePtr btp, Int2 count, ValNodePtr the_pub, Int4 pat_seqid, Boolean PNTR submit, Boolean make_index));
void GetDivision PROTO ((CharPtr buffer, Int2 count, BiotablePtr btp));
ValNodePtr GetKeywordLine PROTO ((BiotablePtr btp, Int2 count));
void PrintSourceFeat PROTO ((BiotablePtr btp, Int2 count));
Int2 PrintImpFeat PROTO ((BiotablePtr btp, Int2 count, SeqFeatPtr sfp));
void PrintNAFeatByNumber PROTO ((BiotablePtr btp, Int2 count));
void PrintAAFeatByNumber PROTO ((BiotablePtr btp, Int2 count));
void PrintOrphanFeatByNumber PROTO ((BiotablePtr btp, Int2 count, SeqFeatPtr sfp_out, Int2 feat_index));
CharPtr FlatLoc PROTO ((BiotablePtr btp, Int2 count, ValNodePtr location));
Int2 CpSfpToList PROTO ((SortStruct PNTR List, SeqFeatPtr sfp, BiotablePtr btp, Int2 currentsize, Int2 count));
CharPtr FlatStringGroup PROTO ((CharPtr start, ValNodePtr head, CharPtr delimit, CharPtr period));
CharPtr FlatStringParanGroup PROTO ((CharPtr start, ValNodePtr head, CharPtr start_delimit, CharPtr delimit, CharPtr period));
Boolean FlatAnnotPartial PROTO ((SeqFeatPtr sfp, Boolean use_product));
Boolean FlatIgnoreThisPatentPub PROTO ((BioseqPtr bsp, ValNodePtr best, Int4Ptr seqidPt));
void DoGBComment PROTO ((BiotablePtr btp, Int2 count));
void DoGPComment PROTO ((BiotablePtr btp, Int2 count));
CharPtr GetComment PROTO ((BiotablePtr btp, Int2 count));
CharPtr GetGibbsqComment PROTO ((BiotablePtr btp, Int2 count));
Int4 GetGibbsqStatement PROTO ((BiotablePtr btp, Int2 count, CharPtr ptr));
Int2 StorePubInfo PROTO ((BioseqContextPtr bcp, BioseqPtr bsp, ValNodePtr PNTR vnpp, Boolean error_msgs));
Int2 StoreNAPubInfo PROTO ((BiotablePtr btp, Int2 count, ValNodePtr PNTR vnp, Boolean error_msgs));
void GetGBDate PROTO ((BiotablePtr btp, Int2 count));
void GetGPDate PROTO ((BiotablePtr btp, Int2 count));
void GetEMBLDate PROTO ((BiotablePtr btp, Int2 count));
void GetVersion PROTO ((BiotablePtr btp, Int2 count));
void GetGeneQuals PROTO ((SeqFeatPtr sfp_in, BiotablePtr btp, Int2 count, Int2 index));
void GetCdregionGeneXrefInfo PROTO ((SeqFeatPtr sfp, BiotablePtr btp, Int2 count, Int2 index));
void GetGeneRefInfo PROTO ((GeneStructPtr gsp, NoteStructPtr nsp, GeneRefPtr grp));
Int2 CompareStringWithGsp PROTO ((GeneStructPtr gsp, CharPtr string));
Boolean CheckNAFeat PROTO ((BiotablePtr btp, Int2 count, SeqFeatPtr sfp));
Boolean CheckAndGetNAFeatLoc PROTO ((BiotablePtr btp, Int2 count, CharPtr PNTR buffer, SeqFeatPtr sfp, Boolean loc_return));
void GetAAFeatLoc PROTO ((BiotablePtr btp, Int2 count, CharPtr PNTR buffer, SeqFeatPtr sfp, Boolean use_product));
Boolean OneOfThree PROTO ((BioseqPtr bsp));
CharPtr GetGBSourceLine PROTO ((GBBlockPtr gb));
Int4 GetGibbsqNumber PROTO ((BiotablePtr btp, Int2 count));
Int4 GetGibbsqCommentLength PROTO ((BiotablePtr btp, Int2 count));
Int4 GetSeqDescrCommentLength PROTO ((BiotablePtr btp, Int2 count));
Int4 GetSeqDescrMaplocLength PROTO ((BiotablePtr btp, Int2 count));
Int4 GetSeqDescrMethodLength PROTO ((BiotablePtr btp, Int2 count));
Int4 GetSeqDescrRegionLength PROTO ((BiotablePtr btp, Int2 count));

Int2 CheckPubs PROTO ((BioseqPtr bsp, ValNodePtr PNTR vnpp, Boolean error_msgs));
CharPtr FlatAuthor PROTO ((BiotablePtr btp, ValNodePtr the_pub));
CharPtr FlatPubTitle PROTO ((ValNodePtr the_pub));
void PrintDBSourceLine PROTO ((BiotablePtr btp, Int2 count));

void PostARefErrMessage PROTO ((BioseqPtr bsp, PubStructPtr psp, ValNodePtr ext_pub, Int2 status, CharPtr string));

void SeparatePartSuppl PROTO (( CharPtr vol_issue, CharPtr part_sub ));


#endif
