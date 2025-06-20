/******************************************************************************
*
*	"location.c" prints out the location in the Genbank FlatFile
*	format.  Most of this code was taken from Karl Sirotkin's
*	code and modified by Tom Madden.
*
******************************************************************************/
/* $Revision: 1.13 $ */ 
/*************************************
*
* $Log: asn2ff4.c,v $
 * Revision 1.13  1995/05/15  21:46:05  ostell
 * added Log line
 *
*
**************************************/
#include "asn2ffp.h"
#define CTX_2GB_LOCATION_TROUBLE 0
#define CTX_2GB_NOT_IMPLEMENTED 1

/******************** Function Prototypes *********************************/

CharPtr FlatLocHalf PROTO ((CharPtr buf, Int4 base, IntFuzzPtr fuzz));
CharPtr FlatLocHalfCaret PROTO ((CharPtr buf, Int4 base, IntFuzzPtr fuzz));
Boolean FlatLocPoint PROTO ((BiotablePtr btp, SeqIdPtr pointIdPtr, SeqIdPtr this_sidp, CharPtr piecebuf, Int4 point, IntFuzzPtr pointfuzzPtr));
Boolean FlatLocCaret PROTO ((BiotablePtr btp, SeqIdPtr pointIdPtr, SeqIdPtr this_sidp, CharPtr piecebuf, Int4 point, IntFuzzPtr pointfuzzPtr));
Boolean FlatVirtLoc PROTO ((BiotablePtr btp, Int2 count, ValNodePtr location));
Boolean FlatLocElement PROTO ((BiotablePtr btp, Int2 count, ValNodePtr location, CharPtr buf));
CharPtr complement_FlatLoc PROTO ((SeqIdPtr this_sidp, Boolean PNTR is_okPt, CharPtr total_buf, CharPtr temp, Int4Ptr lengthPt, BiotablePtr btp, Int2 count, ValNodePtr location));
CharPtr FlatSmartStringMove PROTO ((CharPtr total_buf, Int4Ptr lengthPt, CharPtr temp, CharPtr string));
Boolean FlatNullAhead PROTO ((BiotablePtr btp, Int2 count, ValNodePtr location));
CharPtr FlatPackedPoint PROTO ((BiotablePtr btp, CharPtr total_buf, CharPtr temp, Int4Ptr lengthPt, PackSeqPntPtr pspp, SeqIdPtr this_sidp));
CharPtr do_FlatLoc PROTO ((Boolean PNTR is_okPt, Boolean ok_to_complement, SeqIdPtr this_sidp, CharPtr total_buf, CharPtr temp, Int4Ptr lengthPt, BiotablePtr btp, Int2 count, ValNodePtr location));
CharPtr group_FlatLoc PROTO ((SeqIdPtr this_sidp, Boolean PNTR is_okPt, int which, CharPtr total_buf, CharPtr temp, Int4Ptr lengthPt, BiotablePtr btp, Int2 count, ValNodePtr location));
Boolean is_real_id PROTO ((BiotablePtr btp, SeqIdPtr pointIdPtr, SeqIdPtr this_sidp));
void Bond PROTO ((BiotablePtr btp, SeqBondPtr bondp, SeqIdPtr this_sidp, CharPtr buf));
Boolean LookForFuzz PROTO ((SeqLocPtr head));

/*--- the number of characters per location element is less
      than 130.  The maximum would be for a bond with both accessions
      and both with (n.m) locations.
----*/
#define MAX_CHAR_LOCATION 135

	static char * lim_str [5] = {"", ">","<", ">", "<"};

/*----------- FlatLocHalf ()------*/
CharPtr FlatLocHalf 
(CharPtr buf, Int4 base, IntFuzzPtr fuzz)
{
	char localbuf [30];
	Uint1 index;

/*------
typedef struct intfuzz {
   Uint1 choice;        1=p-m, 2=range, 3=pct, 4=lim 
   Int4 a, b;           a=p-m,max,pct,orlim, b=min 
} IntFuzz, PNTR IntFuzzPtr;

Int-fuzz ::= CHOICE {
    p-m INTEGER ,                    -- plus or minus fixed amount
    range SEQUENCE {                 -- max to min
        max INTEGER ,
        min INTEGER } ,
    pct INTEGER ,                    -- % plus or minus (x10) 0-1000
    lim ENUMERATED {                 -- some limit value
        unk (0) ,                    -- unknown
        gt (1) ,                     -- greater than
        lt (2) ,                     -- less than
        tr (3) ,                     -- space to right of position
        tl (4) ,                     -- space to left of position
        other (255) } }              -- something else
-------*/

	localbuf[0] = '\0';
	buf[0] = '\0';

	if (fuzz){
		/* Fuzz_found = TRUE; */
		switch (fuzz -> choice){
			case 1:
				sprintf(localbuf,"(%ld.%ld)", (long) (base - fuzz -> a),
					(long) (base + fuzz -> a));
				break;
			case 2:
				sprintf(localbuf,"(%ld.%ld)", (long) (1+fuzz -> b),
					(long) (1+fuzz -> a));
				break;
			case 3:
				sprintf(localbuf,"(%ld.%ld)", 
					(long) (base - base* ((double) fuzz -> a/1000.0 )),
					(long) (base +base*( (double) fuzz -> a/1000.0 )));
				break;
			case 4:
				index = (Uint1) fuzz -> a;
				if (index > 4) index = 0;
				sprintf(localbuf,"%s%ld", lim_str[index], (long) base);
				break;
			default:
			sprintf(localbuf,"%ld", (long) base);
		}
	}else{
		sprintf(localbuf,"%ld", (long) base);
	}

	StringMove(buf, localbuf);

	return buf;
}

/*----------- FlatLocHalfCaret ()------*/

CharPtr FlatLocHalfCaret
(CharPtr buf, Int4 base, IntFuzzPtr fuzz)
{
	char localbuf [30];
	Uint1 index;

	if (fuzz){
		/* Fuzz_found = TRUE; */
		switch (fuzz -> choice){
			case 1:
				break;
			case 2:
				sprintf(localbuf,"%ld^%ld", (long) (1+fuzz -> b),
					(long) (1+fuzz -> a));
				break;
			case 3:
				break;
			case 4:
				if (base > 1){
					sprintf(localbuf,"%ld^%ld", (long) (base-1), (long) base);
				}else{
					index = (Uint1) fuzz -> a;
					if (index > 4) index = 0;
					sprintf(localbuf,"%s%ld", 
						lim_str[index], (long) base);
				}
				break;
			default:
			sprintf(localbuf,"%ld", (long) base);
		}
	}else{
		sprintf(localbuf,"%ld", (long) base);
	}

	StringMove(buf, localbuf);

	return buf;
}

/*----------- FlatLocPoint  ()------*/

Boolean 
FlatLocPoint 
(BiotablePtr btp, SeqIdPtr pointIdPtr, SeqIdPtr this_sidp, CharPtr piecebuf, Int4 point, IntFuzzPtr pointfuzzPtr)
{
	BioseqPtr bs;
	SeqIdPtr use_id;
	Char buf_space[MAX_CHAR_LOCATION +1], halfbuf_space[MAX_CHAR_LOCATION +1];
	CharPtr buf, halfbuf, temp;
	Boolean OK = FALSE;
	static Boolean order_initialized = FALSE;
	static Uint1 order[18];
	ObjectIdPtr ob;
	
if ( ! order_initialized){
	int dex;
	for (dex=0; dex < 18; dex ++)
		order[dex] = 255;
	order_initialized = TRUE;
		order[SEQID_GENBANK ] = 1;
		order[SEQID_EMBL ] = 2;
		order[SEQID_DDBJ ] = 3;
		order[SEQID_LOCAL ] =4;
		order[SEQID_GI ] =5;
		order[SEQID_GIBBSQ ] =6;
		order[SEQID_GIBBMT ] =7;
		order[SEQID_PRF ] =8;
		order[SEQID_PDB ] =9;
		order[SEQID_PIR ] =10;
		order[SEQID_SWISSPROT ] =11;
		order[SEQID_PATENT ] =12;
		order[SEQID_OTHER ] =13;
		order[SEQID_GENERAL ] =14;
		order[SEQID_GIIM ] =15;
}

	buf = &(buf_space[0]);
	halfbuf = &(halfbuf_space[0]);
	piecebuf[0] = '\0';
	buf[0] = '\0';
	temp = buf;

		if (pointIdPtr)
		if ( ! SeqIdIn ( pointIdPtr, this_sidp)){
			use_id = pointIdPtr;
                        bs = BioseqFind(use_id);
			if ( bs ){
				use_id = SeqIdSelect ( bs -> id, order,18);
			}
			SeqIdPrint( use_id, buf, PRINTID_TEXTID_ACCESSION);
			if (buf) if (*buf)
				OK = TRUE;
			if (! OK){
				if (btp->FlatLoc_context == FLATLOC_CONTEXT_LOC ){
					if (use_id -> choice == SEQID_LOCAL){
						ob = (ObjectIdPtr) use_id -> data.ptrvalue;
						if (ob ->str){
							if (*ob -> str){
								StringNCpy(buf, ob ->str,12);
								OK = TRUE;
							}
						}
					}
					if (!OK)
						StringCpy(buf,"?00000");
				}else{
					SeqIdPrint( use_id, buf, PRINTID_TEXTID_LOCUS);
					if (buf) if (*buf)
						OK = TRUE;
					if (! OK){
						StringCpy(buf,"??????");
					}
				}
			}
			temp = StringMove (temp, buf);
			temp = StringMove(temp,":");
		}
		FlatLocHalf(halfbuf, point+1, pointfuzzPtr);
    temp = StringMove(temp, halfbuf);
		StringMove(piecebuf, buf);

	return TRUE;
}

/*----------- FlatLocCaret  ()------*/

Boolean 
FlatLocCaret 
(BiotablePtr btp, SeqIdPtr pointIdPtr, SeqIdPtr this_sidp, CharPtr piecebuf, Int4 point, IntFuzzPtr pointfuzzPtr)
{
	BioseqPtr bs;
	Char buf_space[MAX_CHAR_LOCATION +1], halfbuf_space[MAX_CHAR_LOCATION +1];
	CharPtr buf, halfbuf, temp;
	SeqIdPtr use_id;
	static Boolean order_initialized = FALSE;
	static Uint1 order[18];
	
if ( ! order_initialized){
	int dex;
	for (dex=0; dex < 18; dex ++)
		order[dex] = 255;
	order_initialized = TRUE;
		order[SEQID_GENBANK ] = 1;
		order[SEQID_EMBL ] = 2;
		order[SEQID_DDBJ ] = 3;
		order[SEQID_LOCAL ] =4;
		order[SEQID_GI ] =5;
		order[SEQID_GIBBSQ ] =6;
		order[SEQID_GIBBMT ] =7;
		order[SEQID_PRF ] =8;
		order[SEQID_PDB ] =9;
		order[SEQID_PIR ] =10;
		order[SEQID_SWISSPROT ] =11;
		order[SEQID_PATENT ] =12;
		order[SEQID_OTHER ] =13;
		order[SEQID_GENERAL ] =14;
		order[SEQID_GIIM ] =15;
}

	buf = &(buf_space[0]);
	halfbuf = &(halfbuf_space[0]);
	piecebuf[0] = '\0';
	buf[0] = '\0';
	temp = buf;

		if (pointIdPtr)
		if ( ! SeqIdIn ( pointIdPtr, this_sidp)){
			use_id = pointIdPtr;
                        bs = BioseqFind(use_id);
			if ( bs ){
				use_id = SeqIdSelect ( bs -> id, order,18);
			}
			SeqIdPrint( use_id, buf, PRINTID_TEXTID_ACCESSION);
			temp = StringMove (temp, buf);
			temp = StringMove(temp,":");
		}
		FlatLocHalfCaret(halfbuf, point+1, pointfuzzPtr);
    temp = StringMove(temp, halfbuf);
		StringMove(piecebuf, buf);

	return TRUE;
}

/*----------- FlatVirtLoc()------*/

Boolean 
FlatVirtLoc
(BiotablePtr btp, Int2 count, ValNodePtr location)
{
	Boolean retval = FALSE;
	SeqIntPtr sintp;
	BioseqPtr this_bsp=NULL;
	SeqIdPtr this_sidp=NULL, sidp = NULL;
	SeqPntPtr spp;

	this_bsp = btp->bsp[count];
	this_sidp = this_bsp -> id;


	switch ( location -> choice){
		case SEQLOC_MIX:
		case SEQLOC_EQUIV:
		case SEQLOC_PACKED_INT:
		case  SEQLOC_PACKED_PNT:
		case  SEQLOC_NULL:
		break;
		case  SEQLOC_EMPTY:
			break;
		case  SEQLOC_WHOLE:
			      sidp = (SeqIdPtr) location -> data.ptrvalue;
      if (! sidp){
         /* if ( !  (Flat_Be_quiet&1)) */
if (btp->error_msgs == TRUE)
ErrPostEx(SEV_INFO, CTX_NCBI2GB,CTX_2GB_LOCATION_TROUBLE,
"FlatLocElement: whole location without ID:");
            retval = TRUE;
            break;
      }
/*--- no break on purpose ---*/
		case  SEQLOC_INT:
			if ( location -> choice == SEQLOC_INT){
				sintp = (SeqIntPtr) location -> data.ptrvalue;
				sidp = sintp-> id;
			}
		
			if ( ! is_real_id(btp, sidp, this_sidp)){
				retval = TRUE;
			}

			break;
		case  SEQLOC_PNT:
			spp = (SeqPntPtr) ( location -> data.ptrvalue);
			if ( ! is_real_id(btp, spp-> id, this_sidp)){
				retval = TRUE;
			}
			break;
		case SEQLOC_BOND:
			break;
		case SEQLOC_FEAT:
		/*if ( !  (Flat_Be_quiet&1))*/
		if (btp->error_msgs == TRUE)
				ErrPostEx(SEV_INFO, CTX_NCBI2GB, CTX_2GB_NOT_IMPLEMENTED,
					"FlatVirtLoc:SEQLOC_FEAT not implemented");
			break;
	}


	return retval;
}

/*----------- FlatLocElement ()------*/

Boolean 
FlatLocElement 
(BiotablePtr btp, Int2 count, ValNodePtr location, CharPtr buf)
{
	Char localbuf_space[MAX_CHAR_LOCATION +1], piecebuf_space[MAX_CHAR_LOCATION +1];
	CharPtr localbuf , piecebuf ;
	CharPtr temp ;
	SeqIntPtr sintp;
	SeqPntPtr spp;
	BioseqPtr this_bsp=NULL, bsp=NULL;
	SeqIdPtr this_sidp=NULL, sidp=NULL;
	SeqIntPtr whole_intPtr =NULL;
	Boolean retval=TRUE;
	Boolean whole_trouble;

	localbuf = &(localbuf_space[0]);
	piecebuf = &(piecebuf_space[0]);
	temp = localbuf;

	this_bsp = btp->bsp[count];
	this_sidp = this_bsp -> id;
	bsp = this_bsp;
	sidp = this_sidp;

	localbuf[0] = '\0';
	buf[0] = '\0';

	switch ( location -> choice){
		case SEQLOC_MIX:
		case SEQLOC_EQUIV:
		case SEQLOC_PACKED_INT:
		case  SEQLOC_PACKED_PNT:
		case  SEQLOC_NULL:
			if (btp->error_msgs == TRUE)
				ErrPostEx(SEV_INFO, CTX_NCBI2GB,CTX_2GB_LOCATION_TROUBLE,
				"Unexpected internal complex type");
			retval = FALSE;
		break;
		case  SEQLOC_EMPTY:
			break;
		case  SEQLOC_WHOLE:
		whole_trouble=TRUE;
		sidp = (SeqIdPtr) location -> data.ptrvalue;
		if (sidp){
                        bsp = BioseqFind(sidp);
			if (bsp){
				sintp = whole_intPtr = MemNew( sizeof(SeqInt) );
				whole_intPtr -> id = sidp;
				whole_intPtr -> from = 0;
				whole_intPtr -> to = -1;
				if ( bsp -> length > 0)
				{
					whole_intPtr -> to = bsp -> length -1 ;
					whole_trouble=FALSE;
				}
			}
		}
		if (whole_trouble && btp->FlatLoc_context == FLATLOC_CONTEXT_LOC ) {
			/*if ( !  (Flat_Be_quiet&1))*/
if (btp->error_msgs == TRUE)
ErrPostEx(SEV_INFO, CTX_NCBI2GB,CTX_2GB_LOCATION_TROUBLE,
"FlatLocElement: whole location without being able to look up limits: %s", sidp?SeqIdPrint (sidp, localbuf, PRINTID_FASTA_LONG):"No Id");
				retval = FALSE;
				break;
		}else if (whole_trouble) {
				SeqIdPrint (sidp, localbuf, PRINTID_FASTA_LONG);
			break;
		}
/*--- no break on purpose ---*/
		case  SEQLOC_INT:
			if ( location -> choice == SEQLOC_INT){
				sintp = (SeqIntPtr) location -> data.ptrvalue;
			}
		
			if (is_real_id(btp, sintp-> id, this_sidp)){
				if (sintp -> strand == 2) /* minus strand */
					temp = StringMove(temp, "complement("); /* ) vi match */
				FlatLocPoint (btp, sintp -> id, this_sidp, piecebuf, 
						sintp -> from, sintp -> if_from );
				temp = StringMove(temp,piecebuf);
				if ( sintp -> to >0 && (sintp -> to != sintp -> from 
						|| sintp -> if_from ||  sintp -> if_to )){
					temp = StringMove(temp,"..");
					FlatLocPoint(btp, NULL, this_sidp, piecebuf, 
						sintp -> to, sintp -> if_to);
					temp = StringMove(temp, piecebuf);
				}
				if (sintp -> strand == 2) /* minus strand */
					/* ( vi match */ temp = StringMove(temp, ")");
			}else{
#ifdef VIRTUALS_NOT_TREATED_AS_NULLS
				StringCpy(localbuf,"No id");
				if (sintp -> id){
					SeqIdPrint (sintp -> id, localbuf, PRINTID_FASTA_LONG);
					if (btp->error_msgs == TRUE)
						ErrPostEx(SEV_INFO, CTX_NCBI2GB,CTX_2GB_LOCATION_TROUBLE,
					"FlatLocElement: interval without being able to use id: %s", 
						localbuf);
				}
				retval = FALSE;
#endif
			}

			break;
		case  SEQLOC_PNT:
			spp = (SeqPntPtr) ( location -> data.ptrvalue);
			if (is_real_id(btp, spp-> id, this_sidp)){
				if (spp -> strand == 2) /* minus strand */
					temp = StringMove(temp, "complement("); /* ) vi match */
				if ( spp -> fuzz){
/*--------
 *  points with fuzz treated as if always come from '^': 
 *  not best, perhaps, but pretty close
 *-------*/
					FlatLocCaret (btp, spp -> id, this_sidp, piecebuf, 
							spp -> point, spp -> fuzz );
				}else{
					FlatLocPoint ( btp, spp -> id, this_sidp, piecebuf, 
							spp -> point, spp -> fuzz );
				}
				temp = StringMove(temp, piecebuf);
				if (spp -> strand == 2) /* minus strand */
					/* ( vi match */ temp = StringMove(temp, ")");
			}else{
#ifdef VIRTUALS_NOT_TREATED_AS_NULLS
ErrPostEx(SEV_INFO, CTX_NCBI2GB,CTX_2GB_LOCATION_TROUBLE,
"FlatLocElement: point without being able to use id: %s", sidp?SeqIdPrint (sidp, localbuf, PRINTID_FASTA_LONG):"No Id");
				retval = FALSE;
#endif
			}
			break;
		case SEQLOC_BOND:
		/*
		bondp = (SeqBondPtr) location -> data.ptrvalue;
		spp = bondp -> a;
		FlatLocPoint ( btp, spp -> id, this_sidp, piecebuf, 
				spp -> point, spp -> fuzz );
		temp = StringMove(temp, piecebuf);
		temp = StringMove(temp,",");
		spp = bondp -> b;
		FlatLocPoint ( btp, NULL, this_sidp, piecebuf, 
				spp -> point, spp -> fuzz );
		temp = StringMove(temp, piecebuf);
		*/
		Bond(btp, (SeqBondPtr)location->data.ptrvalue, this_sidp,  localbuf);
			break;
		case SEQLOC_FEAT:
	/*	if ( !  (Flat_Be_quiet&1)) */
		if (btp->error_msgs == TRUE)
			ErrPostEx(SEV_INFO, CTX_NCBI2GB, CTX_2GB_NOT_IMPLEMENTED,
				"FlatLocElement:SEQLOC_FEAT not implemented");
			break;
	}

	if (whole_intPtr)
		MemFree(whole_intPtr);  /* NOT object free ! ! ! */
	
	if (! retval){
		*buf = '\0';
	}else{
		StringMove(buf, localbuf);
	}

	return retval;
}

/****************************************************************************
*Bond
*
*	This function takes a SeqBondPtr and a CharPtr, in buf, and returns a
*	string, in buf.  If both ends of the bond exist, the output
*	is bond(a, b); if only one end exists, the output is bond(a).
*
*	Tom Madden
*
**************************************************************************/

void Bond(BiotablePtr btp, SeqBondPtr bondp, SeqIdPtr this_sidp, CharPtr buf)

{
	Char piecebuf1[MAX_CHAR_LOCATION+1];
	Char piecebuf2[MAX_CHAR_LOCATION+1];
	SeqPntPtr spp;

	spp = bondp -> a;
	FlatLocPoint ( btp, spp -> id, this_sidp, &(piecebuf1[0]), 
		spp -> point, spp -> fuzz );
	if (bondp->b)
	{
		spp = bondp -> b;
		FlatLocPoint ( btp, NULL, this_sidp, &(piecebuf2[0]), 
			spp -> point, spp -> fuzz );
		sprintf(buf, "bond(%s,%s)", piecebuf1, piecebuf2);
	}
	else
	{
		sprintf(buf, "bond(%s)", piecebuf1);

	}
}

/*----------- FlatLoc  ()------*/

CharPtr FlatLoc 
(BiotablePtr btp, Int2 count, ValNodePtr location)
{
	CharPtr retval = NULL;
	Int4 max_length, len_used;
	Boolean is_ok = TRUE;
	BioseqPtr this_bsp;
	SeqIdPtr this_sidp;

	btp->FlatLoc_context = FLATLOC_CONTEXT_LOC ;

	if (location){
		this_bsp = btp->bsp[count];
		this_sidp = this_bsp -> id;
		max_length = 0;
		do_FlatLoc(& is_ok, TRUE, this_sidp, NULL, NULL, 
			& max_length, btp, count, location);
		if (is_ok){
			retval = (CharPtr) MemNew((size_t) (max_length + 10));
			len_used = 0;
			do_FlatLoc(&is_ok, TRUE, this_sidp,  retval, retval, 
					&len_used, btp, count, location);
		}
	}

	return retval;
}



/*-------------complement_FlatLoc ()---------------*/

CharPtr
complement_FlatLoc (SeqIdPtr this_sidp, Boolean PNTR is_okPt, CharPtr total_buf, CharPtr temp, Int4Ptr lengthPt, BiotablePtr btp, Int2 count, ValNodePtr location)
{
	SeqLocRevCmp(location);

	temp = FlatSmartStringMove(total_buf, lengthPt, temp,"complement(");
	temp = do_FlatLoc (is_okPt, FALSE, this_sidp, total_buf, 
		temp, lengthPt, btp, count, location);
	temp = FlatSmartStringMove(total_buf, lengthPt, temp,")");

	SeqLocRevCmp(location);

	return temp;
}
/*-------- FlatSmartStringMove()-------*/

CharPtr 
FlatSmartStringMove(CharPtr total_buf, Int4Ptr lengthPt, CharPtr temp, CharPtr string)
{
	CharPtr retval = temp;

	if ( lengthPt){
		(*lengthPt) += StringLen(string);
	}
	if (total_buf){
		retval = StringMove(temp,string);
	}

	return retval;
}
#define FLAT_EQUIV 0
#define FLAT_JOIN 1
#define FLAT_ORDER 2
static CharPtr group_names []= {
"one-of","join","order"};

/*--------- FlatNullAhead()---------*/

Boolean
FlatNullAhead(BiotablePtr btp, Int2 count, ValNodePtr location)
{
	Boolean retval = FALSE;
	SeqLocPtr next;
	if (location){
		next = location -> next;
		if (next){
			if ( next -> choice == SEQLOC_NULL)
				retval = TRUE;
			if (FlatVirtLoc(btp, count, next))
				retval = TRUE;
		}else{
			retval = TRUE;  /* last one always true */
		}
	}

	return retval;
}

								
/*---------- FlatPackedPoint() -----------*/

CharPtr
FlatPackedPoint (BiotablePtr btp, CharPtr total_buf, CharPtr temp, Int4Ptr lengthPt, PackSeqPntPtr pspp, SeqIdPtr this_sidp)
{
	Char buf_space[MAX_CHAR_LOCATION +1];
	CharPtr buf;
	int dex;

	buf = &(buf_space[0]);
	for (dex=0; dex < (int) pspp -> used; dex ++){
		FlatLocPoint ( btp, pspp -> id, 
			this_sidp, buf, (pspp->pnts)[dex], pspp -> fuzz);
		 temp = FlatSmartStringMove(total_buf, 
					lengthPt, temp,buf);
	}

	return temp;
}
/*--------- group_FlatLoc ()-----------*/

CharPtr 
group_FlatLoc (SeqIdPtr this_sidp, Boolean PNTR is_okPt, int which, CharPtr total_buf, CharPtr temp, Int4Ptr lengthPt, BiotablePtr btp, Int2 count, ValNodePtr location)
{
	Char buf_space[MAX_CHAR_LOCATION +1];
	CharPtr buf;
	SeqLocPtr this_loc;
	Boolean special_mode = FALSE; /* join in order */
	int parens = 1;
	Boolean found_non_virt = FALSE;

	buf = &(buf_space[0]);
	
	 temp = FlatSmartStringMove(total_buf, lengthPt,
		temp,group_names[which]);
	 temp = FlatSmartStringMove(total_buf, lengthPt,
		temp,"(");
	
	for (this_loc = (SeqLocPtr) location -> data.ptrvalue;
			this_loc && *is_okPt; this_loc = this_loc -> next){
		if ( FlatVirtLoc(btp, count, this_loc)) {
			if ( this_loc != location && this_loc -> next){
				if (special_mode ){
					special_mode = FALSE;
					temp = FlatSmartStringMove(total_buf, lengthPt,
						temp,")");
					parens --;
				}
			}
			continue;
		}
		if ( found_non_virt &&
				this_loc -> choice !=  SEQLOC_EMPTY &&
				this_loc -> choice !=  SEQLOC_NULL ){
			temp = FlatSmartStringMove(total_buf, lengthPt,
				temp,",");
		}
		switch ( this_loc -> choice ){
		case SEQLOC_NULL : 
			if ( this_loc != location && this_loc -> next){
				if (special_mode ){
					special_mode = FALSE;
					temp = FlatSmartStringMove(total_buf, lengthPt,
						temp,")");
					parens --;
				}
			}
			break;
		case SEQLOC_EMPTY :
			break;
		case SEQLOC_WHOLE : 
		case SEQLOC_PNT : 
		case SEQLOC_BOND :
		case SEQLOC_FEAT :
		 found_non_virt = TRUE;
			if (FlatVirtLoc(btp, count,this_loc)){
				if ( this_loc != location && this_loc -> next){
					if (special_mode ){
						special_mode = FALSE;
						temp = FlatSmartStringMove(total_buf, lengthPt,
							temp,"),");
						parens --;
					}
				}
			}else{
				if( FlatLocElement(btp, count,this_loc, buf)){
					temp = FlatSmartStringMove(total_buf, lengthPt, 
						temp,buf);
				}else{
					temp = NULL;
					* is_okPt = FALSE;
				}
			}
			break;
		case SEQLOC_INT :
		 found_non_virt = TRUE;
			if ( which == FLAT_ORDER 
					&& ! FlatNullAhead(btp, count, this_loc)){
				special_mode = TRUE;
				 temp = FlatSmartStringMove(total_buf, lengthPt,
					temp,group_names[FLAT_JOIN]);
				 temp = FlatSmartStringMove(total_buf, lengthPt,
					temp,"(");
				parens ++;
			}
		
			if( FlatLocElement(btp, count, this_loc, buf)){
				temp = FlatSmartStringMove(total_buf, lengthPt, 
					temp,buf);
			}else{
				temp = NULL;
				* is_okPt = FALSE;
			}
			break;

		case SEQLOC_PACKED_PNT :
		 found_non_virt = TRUE;
				temp = FlatPackedPoint
					(btp, total_buf, temp, lengthPt,
					(PackSeqPntPtr)this_loc->data.ptrvalue,
					this_sidp);
			break;
		case SEQLOC_PACKED_INT :
		case SEQLOC_MIX :
		case SEQLOC_EQUIV :
		{
			ValNodePtr hold_next = this_loc -> next;
			 found_non_virt = TRUE;
			this_loc -> next = NULL;
			temp = do_FlatLoc(is_okPt, FALSE, this_sidp, total_buf, temp, lengthPt, 
				btp, count, this_loc);
			this_loc-> next = hold_next;
		}
			break;
		}
	}

	if (* is_okPt){
		while ( parens > 0){
		 temp = FlatSmartStringMove(total_buf, lengthPt,
			temp,")");
			parens --;
		}
	}

	return temp;
}

CharPtr 
do_FlatLoc (Boolean PNTR is_okPt, Boolean ok_to_complement, SeqIdPtr this_sidp, CharPtr total_buf, CharPtr temp, Int4Ptr lengthPt, BiotablePtr btp, Int2 count, ValNodePtr location)
{
	Char buf_space[MAX_CHAR_LOCATION +1];
	CharPtr buf;
	SeqLocPtr slp;
	SeqLocPtr next_loc = NULL;
	Boolean did_complement = FALSE;

	buf = &(buf_space[0]);

	if (location){
		if ( ok_to_complement && 
				SeqLocStrand(location) == Seq_strand_minus ){
			temp = complement_FlatLoc (this_sidp, is_okPt, total_buf, 
				temp, lengthPt, btp, count, location);
			did_complement = TRUE;
		}
		if ( ! did_complement)
		for ( slp = location; slp && *is_okPt; slp = slp -> next){
			if ( slp -> choice == SEQLOC_NULL ||
					FlatVirtLoc(btp, count, slp))
				continue;
			if ( slp != location){
				 temp = FlatSmartStringMove(total_buf, lengthPt,
					temp,",");
			}
			switch( slp -> choice){
				case  SEQLOC_NULL:
					break;
				case SEQLOC_MIX:
				case SEQLOC_PACKED_INT:
					{
						Boolean found_null = FALSE;
						for (next_loc = (SeqLocPtr) slp -> 
								data.ptrvalue; next_loc;
								next_loc = next_loc -> next){
							if ( next_loc -> choice == SEQLOC_NULL
									|| FlatVirtLoc( btp, count, next_loc)){
								found_null = TRUE;
								temp=group_FlatLoc(this_sidp, is_okPt,
											FLAT_ORDER, 
									total_buf, temp, lengthPt, btp, count, 
									slp);
								break;
							}
						}
						if ( ! found_null){
							temp=group_FlatLoc(this_sidp, is_okPt,
											FLAT_JOIN, total_buf, 
								temp, lengthPt, btp, count, 
								slp);
						}
					}
					break;
				case SEQLOC_EQUIV:
					temp=group_FlatLoc(this_sidp, is_okPt,
											FLAT_EQUIV, total_buf, temp,
						lengthPt, btp, count, slp);
					break;
				case  SEQLOC_PACKED_PNT:
				temp = FlatPackedPoint
					(btp, total_buf, temp, lengthPt,
					(PackSeqPntPtr)slp->data.ptrvalue,
					this_sidp);
				break;
				default:
				if ( ! FlatVirtLoc( btp, count, slp))
				if( FlatLocElement(btp, count,slp, buf)){
					temp = FlatSmartStringMove(total_buf, lengthPt, 
						temp,buf);
				}else{
					temp = NULL;
					*is_okPt = FALSE;
				}
			}
		}
	}


	return temp;
}


/*-----is_real_id()---*/

Boolean is_real_id
(BiotablePtr btp, SeqIdPtr pointIdPtr, SeqIdPtr this_sidp)
{
	Boolean retval = TRUE;
	BioseqPtr bs;
	SeqIdPtr use_id;

		if ( ! SeqIdIn ( pointIdPtr, this_sidp)){
			use_id = pointIdPtr;
                        bs = BioseqFind(use_id);
			if ( bs ){
			if (bs -> repr == Seq_repr_virtual)
				retval = FALSE;
			}
		}
	return retval;
}

/*----------------------- FlatAnnotPartial()-----------*/
Boolean
FlatAnnotPartial (SeqFeatPtr sfp, Boolean use_product)
{
	Boolean fuzz=FALSE, retval = TRUE;
	CharPtr str;
	ImpFeatPtr imp;

	if ( sfp -> data.choice == 8) /*  Imp-feat    */
	{
		imp = (ImpFeatPtr) (sfp -> data.value.ptrvalue);
		if (imp ->loc != NULL)
		for( str = imp -> loc; *str; str ++){
			if ( *str == '<' || *str == '>'){
				retval = FALSE;
				break;
			}else if (*str == 'r' && *(str +1) == 'e'){
				if (StringNCmp ("replace",str, (size_t) 7) == 0){
					retval = FALSE;
					break;
				}
			}
		}
		if (retval == TRUE) /* Look for fuzz in the ASN.1 location */
		{
			if (use_product)
				fuzz = LookForFuzz (sfp->product);
			else
				fuzz = LookForFuzz (sfp->location);
			if (fuzz == TRUE)
				retval = FALSE;
		}
	}

	return retval;
}

/************************************************************************
*Boolean LookForFuzz (SeqLocPtr slp)
*
*	Look for fuzz in the location.  For use in FlatAnnotPartial.
************************************************************************/

Boolean
LookForFuzz (SeqLocPtr head)

{
	Boolean retval=FALSE;
	IntFuzzPtr ifp;
	PackSeqPntPtr pspp;
	SeqIntPtr sip;
	SeqLocPtr slp;
	SeqPntPtr spp;

	if (head == NULL)
		return retval;

	slp=NULL;
	while ((slp = SeqLocFindNext(head, slp)) != NULL)
	{
		switch (slp->choice)
		{
			case SEQLOC_INT:
				sip = (SeqIntPtr)(slp->data.ptrvalue);
				ifp = sip->if_from;
				if (ifp != NULL)
				{
					if (ifp->choice == 4)
					{
						if (ifp->a != 0)
							retval=TRUE;
					}
					else
						retval = TRUE;	
				}
				ifp = sip->if_to;
				if (ifp != NULL)
				{
					if (ifp->choice == 4)
					{
						if (ifp->a != 0)
							retval=TRUE;
					}
					else
						retval = TRUE;	
				}
				break;
			case SEQLOC_PNT:
				spp = (SeqPntPtr)(slp->data.ptrvalue);
				ifp = spp->fuzz;
				if (ifp != NULL)
				{
					if (ifp->choice == 4)
					{
						if (ifp->a != 0)
							retval=TRUE;
					}
					else
						retval = TRUE;	
				}
				break;
			case SEQLOC_PACKED_PNT:
				pspp = (PackSeqPntPtr)(slp->data.ptrvalue);
				ifp = pspp->fuzz;
				if (ifp != NULL)
				{
					if (ifp->choice == 4)
					{
						if (ifp->a != 0)
							retval=TRUE;
					}
					else
						retval = TRUE;	
				}
				break;
			default:
				break;
		}
		if (retval == TRUE)
			break;
	}
	return retval;
} /* LookForFuzz */
