/*************************************************************************
* gbfeat.h:
*
* $Log: gbfeat.h,v $
 * Revision 1.6  1995/05/15  21:46:05  ostell
 * added Log line
 *
*
*                                                              10-11-93
*************************************************************************/
#ifndef _GBFEAT_
#define _GBFEAT_

#include <stdio.h>
#include <ncbi.h>
#include <seqport.h>
#include <sequtil.h>


/*************************************************************************/

Int2 GBQualNameValid PROTO((CharPtr qual));
/*  -- return index of the ParFlat_GBQual_names array if it is a valid
       qualifier (ignore case), qual; otherwise return (-1)
                                                                10-12-93  */

Int2 GBQualSplit PROTO((CharPtr qual));

Int2 GBFeatKeyNameValid PROTO((CharPtr PNTR keystr, Boolean error_msgs));
/* return "index" of the ParFlat_GBFeat array if it is a valid feature key (
  ignore case), keystr; otherwise, return (-1), UnknownFeatKey
                                                                  
                                                                  10-11-93 */
                                                                  
/*   --  GB_ERR returns values */

#define GB_FEAT_ERR_NONE 0
#define GB_FEAT_ERR_SILENT 1
#define GB_FEAT_ERR_REPAIRABLE 2
#define GB_FEAT_ERR_DROP 3 

int GBFeatKeyQualValid PROTO((Int2 keyindx, GBQualPtr PNTR gbqp, Boolean error_msgs, Boolean perform_corrections));
/*  -- returns GB_ERR value -Karl 1/28/94 */

int SplitMultiValQual PROTO((GBQualPtr PNTR gbqp, 
     Boolean error_msgs, Boolean perform_corrections));

int GBQualSemanticValid PROTO((GBQualPtr PNTR gbqp, Boolean error_msgs, Boolean perform_corrections));
/* -- returns GB_ERR value
                          -Karl 1/28/94
  -- routine also drops out any unknown qualifier, if 'perform corrections' is set
                                                                  10-11-93 */

/* The following routines will return a GB_ERR code 
           -Karl 1/31/94 */

int CkQualPosSeqaa PROTO((GBQualPtr PNTR head_gbqp, GBQualPtr gbqp, GBQualPtr preq,
   Boolean error_msgs, Boolean perform_corrections, CharPtr aa, CharPtr eptr));

int CkQualPosaa PROTO((GBQualPtr PNTR gbqp,  GBQualPtr curq, GBQualPtr preq, Boolean error_msgs, Boolean perform_corrections));

int CkQualNote PROTO((GBQualPtr PNTR gbqp,  GBQualPtr curq, GBQualPtr preq, Boolean error_msgs, Boolean perform_corrections));

void ConvertEmbedQual PROTO((CharPtr value));

int CkQualText PROTO((GBQualPtr PNTR gbqp,  GBQualPtr curq, GBQualPtr preq, Boolean PNTR has_embedded, Boolean from_note, Boolean error_msgs, Boolean perform_corrections));

CharPtr ScanEmbedQual PROTO((CharPtr value));
/* -- retun NULL if no embedded qualifiers found; otherwise, return the
      embedded qualifier.
                                                            6-29-93 */
int CkQualSeqaa PROTO((GBQualPtr PNTR gbqp,  GBQualPtr curq, GBQualPtr preq, Boolean error_msgs, Boolean perform_corrections));

int CkQualMatchToken PROTO((GBQualPtr PNTR gbqp,  GBQualPtr curq, GBQualPtr preq, Boolean error_msgs, Boolean perform_corrections, CharPtr array_string[],
                                                         Int2 totalstr));

int CkQualEcnum PROTO((GBQualPtr PNTR gbqp,  GBQualPtr curq, GBQualPtr preq, Boolean error_msgs, Boolean perform_corrections));

int CkQualSite PROTO((GBQualPtr PNTR gbqp,  GBQualPtr curq, GBQualPtr preq, Boolean error_msgs, Boolean perform_corrections));

int CkQualTokenType PROTO((GBQualPtr PNTR gbqp,  GBQualPtr curq, GBQualPtr preq, Boolean error_msgs, Boolean perform_corrections, Uint1 type));

/*********************************************************************/

CharPtr GBFeatErrSpec PROTO((Uint1 indx));
/*  -- return a err_specific string by a given index         10-14-93 */

void DeleteGBQualFromList PROTO((GBQualPtr PNTR gbqp, GBQualPtr curq, GBQualPtr preq));
/* does list management to delete curq qual from gbqp head of list */


CharPtr CkBracketType PROTO((CharPtr str));

CharPtr CkLabelType PROTO((CharPtr str));

GBQualPtr tie_qual PROTO((GBQualPtr head, GBQualPtr next));

#endif
