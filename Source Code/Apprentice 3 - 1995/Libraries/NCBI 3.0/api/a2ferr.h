/* $Revision: 1.18 $ */
/***********************************
*
* $Log: a2ferr.h,v $
 * Revision 1.18  1995/05/15  21:46:05  ostell
 * added Log line
 *
*
***********************************/

#ifndef __MODULE_a2ferr__
#define __MODULE_a2ferr__

#define ERR_DATE  1,0
#define ERR_DATE_IllegalDate  1,1
#define ERR_DATE_Create_after_update  1,2
#define ERR_SEGMENT  2,0
#define ERR_SEGMENT_LocalSegEntry  2,1
#define ERR_SEGMENT_MoreThan1000Segs  2,2
#define ERR_ACCESSION  3,0
#define ERR_ACCESSION_BadAccessNum  3,1
#define ERR_ACCESSION_NoAccessNum  3,2
#define ERR_LOCUS  4,0
#define ERR_LOCUS_ChangedLocusName  4,1
#define ERR_LOCUS_BadLocusName  4,2
#define ERR_LOCUS_LocusNameCollision  4,3
#define ERR_LOCUS_NoLocusName  4,8
#define ERR_REFERENCE  5,0
#define ERR_REFERENCE_NoPageNumbering  5,1
#define ERR_REFERENCE_IllegalPageRange  5,2
#define ERR_REFERENCE_YearEquZero  5,3
#define ERR_REFERENCE_IllegalDate  5,4
#define ERR_REFERENCE_Patent  5,5
#define ERR_REFERENCE_Journal  5,6
#define ERR_REFERENCE_Thesis  5,7
#define ERR_REFERENCE_Book  5,8
#define ERR_REFERENCE_DirectSubmission  5,9
#define ERR_REFERENCE_Illegalreference  5,10
#define ERR_REFERENCE_NoAuthorName  5,11
#define ERR_REFERENCE_NoJournalName  5,12
#define ERR_REFERENCE_NoValidRefs  5,13
#define ERR_REFERENCE_NoDateOnRef  5,14
#define ERR_REFERENCE_DirSubInCitGen  5,15
#define ERR_REFERENCE_MuidZeroOnlyInfo  5,16
#define ERR_REFERENCE_VolHasSupplInfo  5,17
#define ERR_REFERENCE_ParanInSuppInfo  5,18
#define ERR_FEATURE  6,0
#define ERR_FEATURE_UnknownFeatureKey  6,1
#define ERR_FEATURE_non_pseudo  6,2
#define ERR_FEATURE_Bad_location  6,3
#define ERR_FEATURE_GcodeAndTTableClash  6,4
#define ERR_FEATURE_NoQualOnMiscFeat  6,5
#define ERR_ENTRY  7,0
#define ERR_ENTRY_Partial_peptide  7,1
#define ERR_ENTRY_DroppedFromFlatfile  7,2
#define ERR_ENTRY_No_source_line  7,3
#define ERR_ENTRY_OriginTooLong  7,4
#define ERR_GI  8,0
#define ERR_GI_No_GI_Number  8,1

#endif
