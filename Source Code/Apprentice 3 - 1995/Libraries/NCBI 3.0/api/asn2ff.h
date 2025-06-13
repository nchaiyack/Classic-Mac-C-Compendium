/*****************************************************************************
*
*	Header file for asn2gb files.
*
****************************************************************************/
/* $Revision: 1.22 $ */ 
/*************************************
*
* $Log: asn2ff.h,v $
 * Revision 1.22  1995/05/15  21:46:05  ostell
 * added Log line
 *
*
**************************************/

#ifndef _ASN2FF_
#define _ASN2FF_

#include <ncbi.h>
#include <objsset.h>
#include <prtutil.h>
#include <seqport.h>
#include <sequtil.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "a2fstruc.h"

#define GENBANK_FMT ( (Uint1)0)
#define EMBL_FMT ( (Uint1)1)
#define GENPEPT_FMT ( (Uint1)2)
#define PSEUDOEMBL_FMT ( (Uint1)3)
#define SELECT_FMT ( (Uint1)4)
#define EMBLPEPT_FMT ( (Uint1)5)
#define RELEASE_MODE ( (Uint1)6)
#define DUMP_MODE ( (Uint1)7)
#define SEQUIN_MODE ( (Uint1)8)
#define CHROMO_MODE ( (Uint1)9)
#define DIRSUB_MODE ( (Uint1)10)
#define REVISE_MODE ( (Uint1)11)

#define NUM_OF_ESTIMATES 20
#define NUM_SEQ_LINES 10

#define LOCUS_COLLISION_DB_NAME "GenBank FlatFile LOCUS"


Int4 asn2ff_setup PROTO ((SeqEntryPtr sep, Boolean show_seq, Boolean show_gi, Uint1 mode, Boolean error_msgs, Boolean bind_all, Uint1 format, StdPrintOptionsPtr Spop, FFPrintArrayPtr PNTR papp, BiotablePtr PNTR btp_aa, BiotablePtr PNTR btp_na));

void asn2ff_set_output PROTO ((FILE *fp, BiotablePtr btp_aa, BiotablePtr btp_na, CharPtr line_return));

void asn2ff_cleanup PROTO ((BiotablePtr btp_aa, BiotablePtr btp_na, FFPrintArrayPtr pap));

CharPtr FFPrint PROTO((FFPrintArrayPtr pap, Int4 index, Int4 pap_size));

Boolean asn2ff PROTO ((SeqEntryPtr sep, FILE *fp, Boolean show_seq, Boolean show_gi, Uint1 mode, Boolean error_msgs, Boolean bind_all, Uint1 format, StdPrintOptionsPtr spop));

Boolean asn2ff_entrez PROTO ((SeqEntryPtr sep, FILE *fp, Uint1 format));

Boolean SeqEntryToFlat PROTO ((SeqEntryPtr sep, FILE *fp, Uint1 format, Uint1 mode));

#ifdef __cplusplus
}
#endif

#endif
