/*****************************************************************************
*
*	Header file for asn2gb files.
*
****************************************************************************/
/* $Revision: 1.10 $ */ 
/*************************************
*
* $Log: asn2ff6.h,v $
 * Revision 1.10  1995/05/15  21:46:05  ostell
 * added Log line
 *
*
**************************************/
#ifndef _ASN2FF6_
#define _ASN2FF6_

#include <asn.h>
#include <objall.h>
#include <objpubd.h>
#include <objsub.h>
#include "utilpars.h"
#include "gbfeat.h"
#include "gbftdef.h"
#include "asnprt.h"
#include "a2ferrdf.h"
#include "ftusrstr.h"
#include "a2fstruc.h"


#define REPORT_ON_NA ( (Uint1)0)
#define REPORT_ON_EVERYTHING ( (Uint1)1)
#define REPORT_ON_AA ( (Uint1)2)

#define NOT_LAST ( (Uint1)0)
#define LAST ( (Uint1)1)
#define NO_PRINTXX ( (Uint1)0)
#define PRINTXX ( (Uint1)1)

#define ASN2FLAT_NOT_ALLOC ( (Uint1)0)
#define ASN2FLAT_ALLOC ( (Uint1)1)
#define ALL ( (Uint1)0)
#define LIMITED ( (Uint1)1)
#define RELEASE ( (Uint1)0)
#define DUMP ( (Uint1)1)
#define FLATLOC_CONTEXT_LOC 1
#define FLATLOC_CONTEXT_NOTE 2
#define GENPEPT_MIN 6
#define MAX_BTP_BUF 80
/* THere is a *bad* effect if MAX_TO_RT_SIDE is 60 or more.  For a /translation
the first character is a space and CheckBufferState returns a buffer with
79 characters.  This results in an ABR. */
#define MAX_TO_RT_SIDE 59
#define ASN2FF_EMBL_MAX 78
#define ASN2FF_GB_MAX 79
#define NEWLINE '\n'
#define ASN2FF_STD_BUF 35
#define MAX_LOCUS_NAME_LEN 10
#define MAX_ACCESSION_LEN 25
#define REP_NUM_OF_SEQ_DESC 22

#define SEQ_BLK_SIZE (60*NUM_SEQ_LINES)

#define GeneStringCmp(s, q)     StringCmp(s, q)


/* --------The SeqEntryExplore Structure-------------*/

typedef struct segment {
	SeqEntryPtr sep;
	SeqEntryPtr PNTR sepp;
	Uint1 PNTR quality;
        Int4 count_set;
	Int4 count;
	Int2 range;
	BioseqPtr PNTR bsp;
} Segment, PNTR SegmentPtr;

/*------------ the structure used for Pubs ---------------*/

typedef struct pubstruct {
	ValNodePtr pub;
	ValNodePtr SeqDescr;
	ValNodePtr Pub;
	SeqFeatPtr sfp;
	PubdescPtr descr;
	ValNode vn;
	Int2 number;
    BioseqPtr PNTR bsp;
    Int2 bspcount;
	SeqFeatPtr PNTR citfeat;
	Int2 citcount;
	SeqFeatPtr PNTR pubfeat;
	Int2 pubcount;
	Int2 start;		/* type of ref: 1-descr, 2- pubfeat, 3 - citfeat */
	DatePtr date;		/* need go CompareByDate */
	Int2	choice; /* 1 - PUB_Gen (unpublished), 2 - PUB_Sub, -1 - other */
} PubStruct, PNTR PubStructPtr;

/*--------------the structure used for HeapSort------------*/

typedef struct sortstruct {
	BioseqPtr bsp;
	SeqFeatPtr sfp;
	BioseqPtr table;
	BioseqPtr other;
} SortStruct, PNTR SortStructPtr;

SegmentPtr SegmentNew PROTO ((Int2 range));
void SegmentFree PROTO ((SegmentPtr segp));
BiotablePtr BiotableNew PROTO ((Int2 range));
void BiotableFree PROTO ((BiotablePtr btp));
GeneStructPtr GeneStructNew PROTO ((GeneStructPtr gsp));
void GeneStructFree PROTO ((GeneStructPtr gsp));
void GeneStructReset PROTO ((BiotablePtr btp, Int2 count, Int2 index));
NoteStructPtr NoteStructNew PROTO ((NoteStructPtr gsp));
void NoteStructFree PROTO ((NoteStructPtr gsp));
void NoteStructReset PROTO ((NoteStructPtr nsp));
int LIBCALL CompareSfpForHeap PROTO ((VoidPtr vp1, VoidPtr vp2));
void AddCitFeat PROTO ((PubStructPtr psp, SeqFeatPtr sfp));
void AddPubBsp PROTO ((PubStructPtr psp, BioseqPtr bsp));
void AddPubFeat PROTO ((PubStructPtr psp, SeqFeatPtr sfp));
void FreePubStruct PROTO ((PubStructPtr psp));
ValNodePtr StorePubFeat PROTO ((ValNodePtr PubOnFeat, SeqFeatPtr sfp));
ValNodePtr StorePubCit PROTO ((ValNodePtr PubOnCit, SeqFeatPtr sfp));
ValNodePtr StoreNAPubCit PROTO ((ValNodePtr PubOnCit, BioseqPtr bsp, SeqFeatPtr sfp));
ValNodePtr StorePubDescr PROTO ((BioseqPtr bsp, ValNodePtr PubOnDescr, PubdescPtr pdp, ValNodePtr SeqDescr));
Int2 GetSerialNumber PROTO ((ValNodePtr pub));
DatePtr GetPubDate PROTO((ValNodePtr pub));
Int2 GetPubChoice PROTO((ValNodePtr pub));
int LIBCALLBACK  PROTO(CompareByDate (VoidPtr vp1, VoidPtr vp2));
int LIBCALL CompareSnForHeap PROTO ((VoidPtr vp1, VoidPtr vp2));
int LIBCALL CompareStartForHeap PROTO ((VoidPtr vp1, VoidPtr vp2));
SeqFeatPtr PNTR CatSfpStacks PROTO
((SeqFeatPtr PNTR sfpp1, Int2 *count1, SeqFeatPtr PNTR sfpp2, Int2 count2));
BioseqPtr PNTR CatBspStacks PROTO
((BioseqPtr PNTR bspp1, Int2 *count1, BioseqPtr PNTR bspp2, Int2 count2));
Int2 SelectBestPub PROTO ((ValNodePtr pub1, ValNodePtr pub2));
ValNodePtr FlatRefBest PROTO ((ValNodePtr equiv, Boolean error_msgs, Boolean accept_muid));
void CpListToBtp PROTO ((SortStruct PNTR List, SeqFeatPtr PNTR PNTR sfpList, Int2 currentsize, Int2Ptr sfpListsize, Int2 count));
Int2 CpSfpToList PROTO ((SortStruct PNTR List, SeqFeatPtr sfp, BiotablePtr btp, Int2 currentsize, Int2 count));
Int2 StartPrint PROTO ((BiotablePtr btp, Int2 init_indent, Int2 cont_indent, Int2 line_max, CharPtr line_prefix));
Int2 AddString PROTO ((BiotablePtr btp, CharPtr string));
Int2 AddChar PROTO ((BiotablePtr btp, Char character));
CharPtr CheckBufferState PROTO ((BiotablePtr btp, Int2Ptr increment_string, Char next_char));
Int2 TabToColumn PROTO ((BiotablePtr btp, Int2 column));
void EndPrint PROTO ((BiotablePtr btp));
void PrintXX PROTO ((BiotablePtr btp));
void FlushBuffer PROTO ((BiotablePtr btp));
Int2 AddInteger PROTO ((BiotablePtr btp, CharPtr fmt, long integer));
void ListFree PROTO ((SeqFeatPtr PNTR PNTR List, Int2 range));
Int2 NewContLine PROTO ((BiotablePtr btp));
CharPtr CheckEndPunctuation PROTO ((CharPtr string, Char end));
void VnpHeapSort PROTO ((ValNodePtr PNTR vnp, int (LIBCALL *compar )PROTO ((Nlm_VoidPtr, Nlm_VoidPtr ))));
CharPtr Cat2Strings PROTO ((CharPtr string1, CharPtr string2, CharPtr separator, Int2 num));
void EnlargeCharPtrStack PROTO ((NoteStructPtr nsp, Int2 enlarge));
SortStruct PNTR EnlargeSortList PROTO ((SortStruct PNTR List, Int2 *totalsize, Int2 increment));
void NoteToCharPtrStack PROTO ((NoteStructPtr nsp, CharPtr annot, CharPtr string, Uint1 alloc));
void CpNoteToCharPtrStack PROTO ((NoteStructPtr nsp, CharPtr annot, CharPtr string));
void SaveNoteToCharPtrStack PROTO ((NoteStructPtr nsp, CharPtr annot, CharPtr string));
BioseqContextPtr FindBioseqContextPtr PROTO ((BiotablePtr btp, BioseqPtr bsp, Int2 *count));
GBQualPtr AddGBQual PROTO ((GBQualPtr gbqual, CharPtr qual, CharPtr val));
 Int2 OrganizePubs PROTO ((BiotablePtr btp, Int2 count));
Int4 check_range PROTO ((Pointer data, SeqIdPtr seq_id));
void do_loc_errors PROTO ((CharPtr front, CharPtr details));
void do_no_loc_errors PROTO ((CharPtr front, CharPtr details));
Boolean GBQualPresent PROTO ((CharPtr ptr, GBQualPtr gbqual));
CharPtr MakeAnAccession PROTO ((CharPtr new_buf, BioseqPtr bsp));
Boolean GetNAFeatKey PROTO ((CharPtr buffer, SeqFeatPtr sfp));
CharPtr GetGBSourceLine PROTO ((GBBlockPtr gb));
CharPtr FlatOrganelle PROTO ((BiotablePtr btp, Int2 count));
Int2 RemoveRedundantFeats PROTO ((BiotablePtr btp, Int2 count, SeqFeatPtr sfp));
Int4 GetNumOfSeqBlks PROTO ((BiotablePtr btp, Int2 count));

SeqIdPtr GetProductSeqId PROTO ((ValNodePtr product));
Int4 GetGINumFromSip PROTO ((SeqIdPtr sip));
SeqIdPtr CheckXrefFeat PROTO ((BioseqPtr bsp, SeqFeatPtr sfp));
Boolean DoSpecialLineBreak PROTO ((BiotablePtr btp, CharPtr string, Int2 indent));


void UniquePubs PROTO ((ValNodePtr PNTR vnpp));
void GetPubDescr PROTO ((BioseqContextPtr bcp, BioseqPtr bsp, ValNodePtr PNTR vnpp));
void GetPubFeat PROTO ((BioseqContextPtr bcp, ValNodePtr PNTR vnpp));
void GetNAPubFeat PROTO ((BiotablePtr btp, Int2 count, ValNodePtr PNTR vnpp, SortStructPtr PNTR List, Int2Ptr currentsites));

void CatPspInfo PROTO ((PubStructPtr psp1, PubStructPtr psp2));
void CountTheSeqEntrys PROTO ((SeqEntryPtr sep, Pointer data, Int4 index, Int2 indent));
void FillSegmentArrays PROTO ((BiotablePtr btp_na, BiotablePtr btp_aa));
void GetBCPs PROTO ((BiotablePtr btp_na, BiotablePtr btp_aa));
void GetGIs PROTO ((BiotablePtr btp_na, BiotablePtr btp_aa));
void GetGINumber PROTO ((BiotablePtr btp, Int2 count));


#endif
