#include "asn2ffp.h"
/* $Revision: 1.35 $ */ 


Int2 GetGenDate PROTO ((BiotablePtr btp, Int2 count, CharPtr buffer));
void PrintComment PROTO ((BiotablePtr btp, CharPtr string, Boolean identifier));
CharPtr GetPDBHet PROTO ((BiotablePtr btp, Int2 count, Int2 indent));
CharPtr GetSPBlock PROTO ((BiotablePtr btp, Int2 count));
CharPtr GetPIRBlock PROTO ((BiotablePtr btp, Int2 count));
CharPtr GetPDBBlock PROTO ((BiotablePtr btp, Int2 count));
CharPtr GetPRFBlock PROTO ((BiotablePtr btp, Int2 count));
static CharPtr SPBlockPrintProc PROTO ((BiotablePtr btp, ValNodePtr vnp));
static CharPtr PIRBlockPrintProc PROTO ((BiotablePtr btp, ValNodePtr vnp));
static CharPtr PDBBlockPrintProc PROTO ((BiotablePtr btp, ValNodePtr vnp));
static CharPtr PRFBlockPrintProc PROTO ((BiotablePtr btp, ValNodePtr vnp));
static CharPtr SeqIdPrintProc PROTO ((BiotablePtr btp, SeqIdPtr sip));
static CharPtr BioseqIdPrintProc PROTO ((BiotablePtr btp, Int2 count));
static CharPtr TableIdPrintProc PROTO ((BiotablePtr btp, Int2 count));
CharPtr DateToGB PROTO ((CharPtr buf, NCBI_DatePtr ndp, Boolean error_msgs));
CharPtr GetSeqDescrComment PROTO ((BiotablePtr btp, Int2 count));
CharPtr GetSeqDescrMaploc PROTO ((BiotablePtr btp, Int2 count));
CharPtr GetSeqDescrRegion PROTO ((BiotablePtr btp, Int2 count));
CharPtr GetSeqDescrMethod PROTO ((BiotablePtr btp, Int2 count));
CharPtr GetSfpComment PROTO ((BiotablePtr btp, Int2 count));
SeqLocPtr GetBaseRangeForCitation PROTO ((SeqLocPtr loc, SeqLocPtr slp, Int4Ptr start, Int4Ptr stop));
Int4 GetMuid PROTO ((ValNodePtr equiv));
static void AddStringWithTildes PROTO ((BiotablePtr btp, CharPtr string));
static ValNodePtr AddToUniqueList PROTO ((SeqIdPtr sid, ValNodePtr list));
static ValNodePtr GetDBSourceForNuclDB PROTO ((BiotablePtr btp, Int2 count));

#define NUM_ORDER 16
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





/***********************************************************************
*DoGBComment
*
***********************************************************************/

void DoGBComment(BiotablePtr btp, Int2 count)

{
	Boolean got_comment=FALSE;
	CharPtr newstring, string;
	Char buffer[150];
	Int2 index;
	Int4 gibbsq;

	string = &buffer[0];

	if (GetSeqDescrCommentLength(btp, count) > 0)
	{
		string = GetSeqDescrComment(btp, count);
		if (string)
		{
			newstring = CheckEndPunctuation(string, '.');
			string = MemFree(string);
			if (got_comment)
				PrintComment(btp, newstring, FALSE);
			else
				PrintComment(btp, newstring, TRUE);
			newstring = MemFree(newstring);
			got_comment = TRUE;
		}
	}

	if (GetSeqDescrMaplocLength(btp, count) > 0)
	{
	string = GetSeqDescrMaploc(btp, count);
	if (string)
	{
		newstring = CheckEndPunctuation(string, '.');
		string = MemFree(string);
		if (got_comment)
			PrintComment(btp, newstring, FALSE);
		else
			PrintComment(btp, newstring, TRUE);
		newstring = MemFree(newstring);
		got_comment = TRUE;
	}
	}

	if (GetSeqDescrRegionLength(btp, count) > 0)
	{
	string = GetSeqDescrRegion(btp, count);
	if (string)
	{
		newstring = CheckEndPunctuation(string, '.');
		string = MemFree(string);
		if (got_comment)
			PrintComment(btp, newstring, FALSE);
		else
			PrintComment(btp, newstring, TRUE);
		newstring = MemFree(newstring);
		got_comment = TRUE;
	}
	}

	if (GetSeqDescrMethodLength(btp, count) > 0)
	{
	string = GetSeqDescrMethod(btp, count);
	if (string)
	{
		newstring = CheckEndPunctuation(string, '.');
		string = MemFree(string);
		if (got_comment)
			PrintComment(btp, newstring, FALSE);
		else
			PrintComment(btp, newstring, TRUE);
		newstring = MemFree(newstring);
		got_comment = TRUE;
	}
	}

	for (index=0; index < btp->sfpCommsize[count]; index++)
	{
		btp->pap_index = (Int4) index;
		string = GetSfpComment(btp, count);
		newstring = CheckEndPunctuation(string, '.');
		if (got_comment)
			PrintComment(btp, newstring, FALSE);
		else
			PrintComment(btp, newstring, TRUE);
		newstring = MemFree(newstring);
		got_comment = TRUE;
	}

	if (btp->gi[count] != -1)
	{
		if (got_comment)
		{
			sprintf(buffer, "~NCBI gi: %ld", (long) (btp->gi[count]));
			PrintComment(btp, buffer, FALSE);
		}
		else
		{
			sprintf(buffer, "NCBI gi: %ld", (long) (btp->gi[count]));
			PrintComment(btp, buffer, TRUE);
		}
	}

	return;
}

/***********************************************************************
*DoGPComment
*
***********************************************************************/

void DoGPComment(BiotablePtr btp, Int2 count)

{
	Boolean got_comment=FALSE;
	Char buffer[30];
	CharPtr newstring, string;
	Int2 index;

	if (btp->number_of_cds < 2)
	{ /* Only print out comments if there's only one CDS */
		if (GetSeqDescrCommentLength(btp, count) > 0)
		{
			string = GetSeqDescrComment(btp, count);
			if (string)
			{
				newstring = CheckEndPunctuation(string, '.');
				string = MemFree(string);
				if (got_comment)
					PrintComment(btp, newstring, FALSE);
				else
					PrintComment(btp, newstring, TRUE);
				newstring = MemFree(newstring);
				got_comment = TRUE;
			}
		}
	
		string = GetSeqDescrMaploc(btp, count);
		if (string)
		{
			newstring = CheckEndPunctuation(string, '.');
			string = MemFree(string);
			if (got_comment)
				PrintComment(btp, newstring, FALSE);
			else
				PrintComment(btp, newstring, TRUE);
			newstring = MemFree(newstring);
			got_comment = TRUE;
		}
	
		string = GetSeqDescrRegion(btp, count);
		if (string)
		{
			newstring = CheckEndPunctuation(string, '.');
			string = MemFree(string);
			if (got_comment)
				PrintComment(btp, newstring, FALSE);
			else
				PrintComment(btp, newstring, TRUE);
			newstring = MemFree(newstring);
			got_comment = TRUE;
		}

		string = GetSeqDescrMethod(btp, count);
		if (string)
		{
			newstring = CheckEndPunctuation(string, '.');
			string = MemFree(string);
			if (got_comment)
				PrintComment(btp, newstring, FALSE);
			else
				PrintComment(btp, newstring, TRUE);
			newstring = MemFree(newstring);
			got_comment = TRUE;
		}
	
		for (index=0; index < btp->sfpCommsize[count]; index++)
		{
			btp->pap_index = (Int4) index;
			string = GetSfpComment(btp, count);
			newstring = CheckEndPunctuation(string, '.');
			if (got_comment)
				PrintComment(btp, newstring, FALSE);
			else
				PrintComment(btp, newstring, TRUE);
			newstring = MemFree(newstring);
			got_comment = TRUE;
		}
	}

	string = GetPDBHet(btp, count, 12);
	if (string)
	{
		if (got_comment == TRUE)	
		{
			newstring = Cat2Strings("~", string, NULL, 0);
			PrintComment(btp, newstring, FALSE);
			newstring = MemFree(newstring);
		}
		else 
			PrintComment(btp, string, TRUE);

		string 	= MemFree(string);
		got_comment = TRUE;
	}

	if (btp->gi[count] != -1)
	{
		if (got_comment)
		{
			sprintf(buffer, "~NCBI gi: %ld", (long) (btp->gi[count]));
			PrintComment(btp, buffer, FALSE);
		}
		else
		{
			sprintf(buffer, "NCBI gi: %ld", (long) (btp->gi[count]));
			PrintComment(btp, buffer, TRUE);
		}
	}

	return;
}	/* DoGPComment */

/*************************************************************************
*CharPtr GetDBSourceLine(BiotablePtr btp, Int2 count)
*
*	Print the DBSource line using the Print Templates.
*	Only used in GenPept mode.
**************************************************************************/
void PrintDBSourceLine(BiotablePtr btp, Int2 count)

{
	Boolean first=TRUE;
	CharPtr newstring, string=NULL;
	SeqIdPtr sid;
	ValNodePtr vnp, vnp1;

	StartPrint(btp, 0, 12, ASN2FF_GB_MAX, NULL);
	AddString(btp, "DBSOURCE");
	TabToColumn(btp, 13);
	sid = SeqIdSelect(btp->bsp[count]->id, fasta_order, NUM_ORDER);
	if (sid && ((sid->choice == SEQID_PIR) ||
                (sid->choice == SEQID_SWISSPROT) ||
                (sid->choice == SEQID_PDB) ||
                (sid->choice == SEQID_PRF)))
	{
/* Use TableIdPrintProc as the higher level Bioseq may have more info
than the individual parts of a segmented set, at least in Swiss-prot. */
        	string = TableIdPrintProc(btp, count);
        	if (string)
        	{
                	AddStringWithTildes(btp, string);
                	string  = MemFree(string);
        	}
        	else if ((string = BioseqIdPrintProc(btp, count)) != NULL)
		{
			AddStringWithTildes(btp, string);
			string 	= MemFree(string);
		}
	}
	else if (sid && ((sid->choice == SEQID_EMBL) ||
                (sid->choice == SEQID_GENBANK) || 
                (sid->choice == SEQID_DDBJ) || 
                (sid->choice == SEQID_GIBBSQ) || 
                (sid->choice == SEQID_GIBBMT) || 
                (sid->choice == SEQID_GIIM))) 

	{
		vnp = GetDBSourceForNuclDB(btp, count);
		for (vnp1=vnp; vnp1; vnp1=vnp1->next)
		{
			if ((string = SeqIdPrintProc(btp, (SeqIdPtr) vnp1->data.ptrvalue)) != NULL)
			{
				if (first == FALSE)
				{
					NewContLine(btp);
					TabToColumn(btp, 13);
				}
				AddString(btp, string);
				string = MemFree(string);
				first = FALSE;
			}
		}
		vnp = ValNodeFree(vnp);
	}
	else	/* Group all others as unknown */
			AddString(btp, "UNKNOWN");
		

/* Only one of the following four should be non-NULL! */
	string = GetPRFBlock(btp, count);
	if (string != NULL)
	{
		newstring = CheckEndPunctuation(string, '.');
		NewContLine(btp);
		AddStringWithTildes(btp, string);
		string = MemFree(string);
		newstring = MemFree(newstring);
	}
	else if ((string=GetPDBBlock(btp, count)) != NULL)
	{
		newstring = CheckEndPunctuation(string, '.');
		NewContLine(btp);
		AddStringWithTildes(btp, string);
		string = MemFree(string);
		newstring = MemFree(newstring);
	}
	else if ((string=GetSPBlock(btp, count)) != NULL)
	{
		newstring = CheckEndPunctuation(string, '.');
		NewContLine(btp);
		AddStringWithTildes(btp, string);
		string = MemFree(string);
		newstring = MemFree(newstring);
	}
	else if ((string=GetPIRBlock(btp, count)) != NULL)
	{
		newstring = CheckEndPunctuation(string, '.');
		NewContLine(btp);
		AddStringWithTildes(btp, string);
		string = MemFree(string);
		newstring = MemFree(newstring);
	}
	EndPrint(btp);

}	/* PrintDBSourceLine */


/********************************************************************
*ValNodePtr GetDBSourceForNuclDB (BiotablePtr btp, Int2 count)
*
*	Look for database id's for the DBSource line.  Look first if
*	there's a CDS; if so get the nucleotide id and check if it's
*	unique. 
*
*
*******************************************************************/

static ValNodePtr GetDBSourceForNuclDB (BiotablePtr btp, Int2 count)

{
	BioseqPtr bsp;
	Int2 index, status, total_feats=btp->sfpListsize[count];
	SeqFeatPtr sfp_in;
	SeqIdPtr sid;
	SeqLocPtr location, tmp;
	ValNodePtr list=NULL, vnp;

	for (index=0; index<total_feats; index++)
	{
		sfp_in = btp->sfpList[count][index];
		if (sfp_in->data.choice == SEQFEAT_CDREGION)
		{
			location = sfp_in->location;
			tmp = NULL;
			while ((tmp = SeqLocFindNext(location, tmp)) != NULL)
			{
				sid = SeqLocId(tmp);
				list = AddToUniqueList(sid, list);
			}
		}
	}

/* If list is still NULL, then there was (probably) no CDS and this is
a "gibbsq" protein (from the backbone).  Use the bsp->id of the protein
itself. */
	if (list == NULL)
	{
		if ((bsp = btp->table[count]) != NULL)
		{
			sid = SeqIdSelect(bsp->id, fasta_order, NUM_ORDER);
			if (sid->choice == SEQID_GIBBMT) 
				ValNodeAddPointer(&list, 0, bsp->id);	
		}
		bsp = btp->bsp[count];
		ValNodeAddPointer(&list, 0, bsp->id);	
	}

/* Look for a bsp associated with this id and select the best id from
bsp->id.  If no bsp, keep the id that was found above by SeqLocId. */
	for (vnp=list; vnp; vnp=vnp->next)
	{
		bsp = BioseqFindCore(vnp->data.ptrvalue);
		if (bsp)
		{
			sid = SeqIdSelect(bsp->id, fasta_order, NUM_ORDER);
			vnp->data.ptrvalue = sid;
		}
		
	}
	return list;
}

/***********************************************************************
*ValNodePtr AddToUniqueList(SeqIdPtr sid, ValNodePtr list)
*
*
*	Checks that the SeqIdPtr is not already present in the list
*	and adds it if it is not.
***********************************************************************/

static ValNodePtr AddToUniqueList(SeqIdPtr sid, ValNodePtr list)

{
	Boolean present=FALSE;
	ValNodePtr vnp;

	if (sid == NULL)	/* A sid of NULL doesn't work here */
		return list;

	for (vnp=list; vnp; vnp=vnp->next)
	{
		if (SeqIdMatch (sid, (SeqIdPtr) vnp->data.ptrvalue) == TRUE)
		{
			present = TRUE;
			break;
		}
	}

	if (present == FALSE)
		vnp = ValNodeAddPointer(&list, 0, sid);	

	if (list == NULL)
		list = vnp;

	return list;
}


/**************************************************************************
*GetSfpComment
*
*	Get the sfp->comments from sfp's that cover the entire bsp, are
*	sfp's of type comment, and are not of length zero.  The length
*	is checked for in OrganizeSeqFeat.
*
**************************************************************************/

CharPtr GetSfpComment (BiotablePtr btp, Int2 count)

{
	Int2 index = (Int2) btp->pap_index;
	SeqFeatPtr sfp;

	sfp = btp->sfpCommlist[count][index];

	return sfp->comment;
}

Int4 GetSeqDescrCommentLength(BiotablePtr btp, Int2 count)

{
	Int4 total=0;
	ValNodePtr vnp=NULL;

	while ((vnp=BioseqContextGetSeqDescr(btp->bcp[count], Seq_descr_comment, vnp, NULL)) != NULL) 
	{
		total += StringLen(vnp->data.ptrvalue);
		total += 2;
	}

	return total;
}

CharPtr GetSeqDescrComment (BiotablePtr btp, Int2 count)

{

	Boolean first_done=FALSE;
	CharPtr string, string1, new_string;
	ValNodePtr vnp=NULL;

	string=NULL;

	while ((vnp=BioseqContextGetSeqDescr(btp->bcp[count], Seq_descr_comment, vnp, NULL)) != NULL) 
	{
		if (first_done)
		{
			string1 = vnp->data.ptrvalue;
			if (StringLen(string1))
			{
				new_string = Cat2Strings(string, string1, "~", 0);
				string = MemFree(string);
				string = new_string;
			}
		}
		else
		{
			if (StringLen(vnp->data.ptrvalue))
			{
				string = StringSave(vnp->data.ptrvalue);
				first_done = TRUE;
			}
		}
	}
	
	return string;
}

Int4 GetSeqDescrMaplocLength (BiotablePtr btp, Int2 count)

{
	CharPtr db=NULL, str=NULL;
	DbtagPtr dbtag=NULL;
	Int4 id = -1, total = 0;
	ObjectIdPtr oip;
	ValNodePtr vnp=NULL;

	if ((vnp=BioseqContextGetSeqDescr(btp->bcp[count], Seq_descr_maploc, NULL, NULL)) != NULL) 
	{
		dbtag = (DbtagPtr) vnp->data.ptrvalue;
		if (dbtag)
		{
			db=dbtag->db;
			oip=dbtag->tag;
			if (oip)
			{
				if (oip->id)
					id = oip->id;
				if (oip->str)
					str = oip->str;
			}
		}
		if (db && id != -1)
			total +=1;
		else if (str)
			total +=1;
	}

	return total;
}

CharPtr GetSeqDescrMaploc (BiotablePtr btp, Int2 count)

{

	Boolean first_done=FALSE;
	Char buffer[160];
	CharPtr db, ptr = &buffer[0], string=NULL, new_string=NULL, str=NULL;
	DbtagPtr dbtag=NULL;
	Int4 id = -1;
	ObjectIdPtr oip;
	ValNodePtr vnp=NULL;

	if ((vnp=BioseqContextGetSeqDescr(btp->bcp[count], Seq_descr_maploc, NULL, NULL)) != NULL) 
	{
		dbtag = (DbtagPtr) vnp->data.ptrvalue;
		if (dbtag)
		{
			db=dbtag->db;
			oip=dbtag->tag;
			if (oip)
			{
				if (oip->id)
					id = oip->id;
				if (oip->str)
					str = oip->str;
			}
		}
		if (db && id != -1)
			sprintf(ptr, "Map location: (Database %s; id # %ld)", db, (long) (id));
		else if (str)
			sprintf(ptr, "Map location: %s", str);

		if (first_done)
		{
			new_string = Cat2Strings(string, ptr, "~", 0);
			string = MemFree(string);
			string = new_string;
		}
		else
		{
			string = StringSave(ptr);
			first_done = TRUE;
		}
	}

	return string;
}


Int4 GetSeqDescrRegionLength (BiotablePtr btp, Int2 count)

{
	Int4 total = 0;
	ValNodePtr vnp=NULL;

	if ((vnp=BioseqContextGetSeqDescr(btp->bcp[count], Seq_descr_region, NULL, NULL)) != NULL) 
	{
		total += 1;
	}
	return total;
}

CharPtr GetSeqDescrRegion (BiotablePtr btp, Int2 count)

{
	Boolean first_done=FALSE;
	Char buffer[160];
	CharPtr ptr = &buffer[0], string=NULL, string1=NULL, new_string=NULL;
	ValNodePtr vnp=NULL;

	if ((vnp=BioseqContextGetSeqDescr(btp->bcp[count], Seq_descr_region, NULL, NULL)) != NULL) 
	{
		sprintf(ptr, "Region: ");
		new_string = Cat2Strings(ptr , vnp->data.ptrvalue, NULL, 0);
		string1 = CheckEndPunctuation(new_string, '.');
		new_string = MemFree(new_string);

		if (first_done)
		{
			new_string = Cat2Strings(string, string1, "~", 0);
			string = MemFree(string);
			string1 = MemFree(string1);
			string = new_string;
		}
		else
		{
			string = string1;
			first_done = TRUE;
		}
	}
	return string;
}

Int4 GetSeqDescrMethodLength (BiotablePtr btp, Int2 count)

{
	Int4 total = 0;
	ValNodePtr vnp=NULL;

	if ((vnp=BioseqContextGetSeqDescr(btp->bcp[count], Seq_descr_method, NULL, NULL)) != NULL) 
	{
		if (vnp->data.intvalue > 1)
			total += 1;
	}
	return total;
}

CharPtr GetSeqDescrMethod (BiotablePtr btp, Int2 count)

{
	Boolean first_done=FALSE;
	Char buffer[160];
	CharPtr ptr = &buffer[0], string=NULL, new_string=NULL;
	ValNodePtr vnp=NULL;

	if ((vnp=BioseqContextGetSeqDescr(btp->bcp[count], Seq_descr_method, NULL, NULL)) != NULL) 
	{
		if (vnp->data.intvalue > 1)
		{
			sprintf(ptr, "Method: %s.", StringForSeqMethod( (Int2) vnp->data.intvalue)); 

			if (first_done)
			{
				new_string = Cat2Strings(string, ptr, "~", 0);
				string = MemFree(string);
				string = new_string;
			}
			else
			{
				string = StringSave(ptr);
				first_done = TRUE;
			}
		}
	}
	return string;
}



/*********************************************************************
*	Int4 GetGibbsqNumber (BiotablePtr btp, Int2 count)
*
*	returns the gibbsq id if a backbone record, otherwise 0.
*
*************************************************************************/

Int4 GetGibbsqNumber (BiotablePtr btp, Int2 count)

{
	BioseqPtr bsp;
	Int4 gibbsq=0;
	SeqIdPtr sip;

	bsp = btp->bsp[count];
	for (sip=bsp->id; sip; sip=sip->next)
	{
		if (sip->choice == SEQID_GIBBSQ)
			gibbsq = sip->data.intvalue;
	}

	return gibbsq;
}

Int4 GetGibbsqCommentLength(BiotablePtr btp, Int2 count)

{
	Int4 gibbsq=0, total;
	PubdescPtr pdp;
	PubStructPtr psp;
	ValNodePtr vnp, vnp1;

	gibbsq = GetGibbsqNumber(btp, count);

	total = 0;

	if (gibbsq > 0)
	{
		total = 0;
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
				if (pdp->poly_a)
					total += 85;
				if (pdp->maploc)
				{
					total += 20;
					total += StringLen(pdp->maploc);
				}
			}
		}
	}
	return total;
}

/**************************************************************************
*	CharPtr GetGibbsqComment(BiotablePtr btp, Int2 count)
*
*	This function determines whether the entry was a backbone entry
*	and generates a message if it was.  The CharPtr needs to be
*	deleted by the caller.
**************************************************************************/

CharPtr GetGibbsqComment(BiotablePtr btp, Int2 count)

{
	CharPtr ptr, temp;
	Int4 gibbsq=0, length, total;
	PubdescPtr pdp;
	PubStructPtr psp;
	ValNodePtr vnp, vnp1;

	gibbsq = GetGibbsqNumber(btp, count);

	if (gibbsq > 0)
	{
		total = GetGibbsqCommentLength(btp, count);

		ptr = (CharPtr) MemNew((size_t) total*sizeof(Char));

		vnp = btp->Pub[count];
		for (vnp1=vnp; vnp1; vnp1=vnp1->next)
		{
			psp = vnp1->data.ptrvalue;
			if ((pdp=psp->descr) != NULL)
			{
				if (pdp->fig)
				{
					temp = CheckEndPunctuation(pdp->fig, '.');
					length = StringLen(ptr);
					sprintf(ptr+length, "~This sequence comes from %s", temp);
					temp = MemFree(temp);
				}
				if (pdp->poly_a)
				{
					length = StringLen(ptr);
					sprintf(ptr+length, "~Polyadenylate residues occurring in the figure were omitted from the sequence.");
				}
				if (pdp->maploc)
				{
					temp = CheckEndPunctuation(pdp->maploc, '.');
					length = StringLen(ptr);
					sprintf(ptr+length, "~Map location: %s", temp);
					temp = MemFree(temp);
				}
			}
		}
		return ptr;	/* gibbsq found */
	}
	return NULL;	/* gibbsq was zero */

}	/* GetGibbsqComment */

/**************************************************************************
*	CharPtr GetGibbsqStatement(BiotablePtr btp, Int2 count)
*
*	This function determines whether the entry was a backbone entry
*	and generates a message if it was.  The caller needs to 
*	include a Char array of 150 characters.
**************************************************************************/

Int4 GetGibbsqStatement(BiotablePtr btp, Int2 count, CharPtr ptr)

{
	Int4 gibbsq=0;

	gibbsq = GetGibbsqNumber(btp, count);

	if (gibbsq > 0)
	{
		sprintf(ptr, "GenBank staff at the National Library of Medicine created this entry [NCBI gibbsq %ld] from the original journal article.", (long) gibbsq);
	}
	return gibbsq;	

}	/* GetGibbsqStatement */

/***************************************************************************
*PrintComment
*
***************************************************************************/

void PrintComment (BiotablePtr btp, CharPtr string, Boolean identifier)

{
	if (string != NULL)
	{
		if (btp->format == EMBL)
		{
			if (identifier == TRUE)
				PrintXX(btp);
			StartPrint(btp, 5, 5, ASN2FF_EMBL_MAX, "CC");
		}
		else
		{
			StartPrint(btp, 0, 12, ASN2FF_GB_MAX, NULL);
			if (identifier == TRUE)
				AddString(btp, "COMMENT");
			TabToColumn(btp, 13);
		}

		AddStringWithTildes(btp, string);
	
		EndPrint(btp);
	}

	return;
}	/* PrintComment */

/***********************************************************************
*static void AddStringWithTildes (BiotablePtr btp, CharPtr string)
*
*	This function prints out a string and replaces the tiles ("~")
*	by new lines (by calling NewContLine).  Before this function is
*	called, printing must be initialized by calling StartPrint;
*	afterwards EndPrint must be called!
*
* Can't this be rewritten to use AddString????  That would be faster! 
************************************************************************/

static void AddStringWithTildes (BiotablePtr btp, CharPtr string)
{
                while (*string != '\0')
                { /* One "~" is a line return, "~~" or "~~ ~~" means 2 returns */       
                        if (*string == '~')
                        {      
                                NewContLine(btp);
                                string++;
                                if (*string == '~')
                                {
                                        NewContLine(btp);
                                        string++;
                                        if (*string == ' ' &&
                                                *(string+1) == '~' &&
                                                        *(string+2) == '~')
                                                string += 3;
                                }
                        }
                        else if (*string == '\"')
                        {   
                                *string = '\'';
                                AddChar(btp, *string);
                                string++;
                        }
                        else
                        {  
                                AddChar(btp, *string);
                                string++;
                        }
                }

}	/* AddStringWithTildes */

CharPtr GetSPBlock (BiotablePtr btp, Int2 count)

{
	ValNodePtr vnp=NULL;
	CharPtr temp, temp1, newstring=NULL, string, newline= "~", 
		identifier= "NewSPblock:~";

	while ((vnp=BioseqContextGetSeqDescr(btp->bcp[count], Seq_descr_sp, vnp, NULL)) != NULL) 
	{
		string = SPBlockPrintProc(btp, vnp);
		if (newstring == NULL)
		{
			newstring = StringSave(string);
			string = MemFree(string);
		}
		else
		{
			temp = Cat2Strings(newstring, newline, ";", 1); 
			newstring = MemFree(newstring);
			temp1 = Cat2Strings(temp, identifier, NULL, 0); 
			newstring = Cat2Strings(temp1, string, NULL, 0); 
			string = MemFree(string);
			temp = MemFree(temp);
			temp1 = MemFree(temp1);
		}

	}
	return newstring;
}

CharPtr GetPDBBlock (BiotablePtr btp, Int2 count)

{
	ValNodePtr vnp=NULL;
	CharPtr temp, temp1, newstring=NULL, string, newline= "~", 
		identifier= "NewPDBblock:~";

	while ((vnp=BioseqContextGetSeqDescr(btp->bcp[count], Seq_descr_pdb, vnp, NULL)) != NULL) 
	{
		string = PDBBlockPrintProc(btp, vnp);
		if (newstring == NULL)
		{
			newstring = StringSave(string);
			string = MemFree(string);
		}
		else
		{
			temp = Cat2Strings(newstring, newline, ";", 1); 
			newstring = MemFree(newstring);
			temp1 = Cat2Strings(temp, identifier, NULL, 0); 
			newstring = Cat2Strings(temp1, string, NULL, 0); 
			string = MemFree(string);
			temp = MemFree(temp);
			temp1 = MemFree(temp1);
		}

	}
	return newstring;
}


CharPtr GetPDBHet (BiotablePtr btp, Int2 count, Int2 indent)

{
	Boolean first_done=FALSE, line_return, paranthesis=FALSE;
	CharPtr het_string="Heterogen Groups:", start, temp, temp1, newstring=NULL, string;
	Int2 length;
	ValNodePtr vnp=NULL;

	while ((vnp=BioseqContextGetSeqDescr(btp->bcp[count], Seq_descr_het, vnp, NULL)) != NULL) 
	{
		
		string = vnp->data.ptrvalue;
		length = StringLen(string);
		temp = start = MemNew(length*sizeof(Char));
		while (*string != '\0')
		{
			if (*string == '(')
				paranthesis = TRUE;
			else if (*string == ')')
				paranthesis = FALSE;
			if (paranthesis && *string == ' ')
				string++;
			else 
			{
				*temp = *string;
				temp++; string++;
			}
		}
		*temp = '\0';

		if (first_done == FALSE)
		{	/* the next two lines initialize DoSpecialLineBreak */
			line_return = DoSpecialLineBreak(btp, NULL, indent);
			line_return = DoSpecialLineBreak(btp, het_string, indent);
			line_return = DoSpecialLineBreak(btp, start, indent);
			if (line_return)
				newstring = Cat2Strings(het_string, start, "~", 0);
			else
				newstring = Cat2Strings(het_string, start, " ", 0);
			first_done = TRUE;
		}
		else
		{
			line_return = DoSpecialLineBreak(btp, start, indent);
			if (line_return)
				temp1 = Cat2Strings(newstring, start, ";~", 0);
			else
				temp1 = Cat2Strings(newstring, start, "; ", 0);
			newstring = MemFree(newstring);
			newstring = temp1;
		}
		start = MemFree(start);
	}
	return newstring;
}

CharPtr GetPRFBlock (BiotablePtr btp, Int2 count)

{
	ValNodePtr vnp=NULL;
	CharPtr temp, temp1, newstring=NULL, string, newline= "~", 
		identifier= "NewPRFblock:~";

	while ((vnp=BioseqContextGetSeqDescr(btp->bcp[count], Seq_descr_prf, vnp, NULL)) != NULL) 
	{
		string = PRFBlockPrintProc(btp, vnp);
		if (newstring == NULL)
		{
			newstring = StringSave(string);
			string = MemFree(string);
		}
		else
		{
			temp = Cat2Strings(newstring, newline, ";", 1); 
			newstring = MemFree(newstring);
			temp1 = Cat2Strings(temp, identifier, NULL, 0); 
			newstring = Cat2Strings(temp1, string, NULL, 0); 
			string = MemFree(string);
			temp = MemFree(temp);
			temp1 = MemFree(temp1);
		}

	}
	return newstring;
}

CharPtr GetPIRBlock (BiotablePtr btp, Int2 count)

{
	ValNodePtr vnp=NULL;
	CharPtr temp, temp1, newstring=NULL, string, string1, newline= "~", 
		identifier= "NewPIRblock:~";

	while ((vnp=BioseqContextGetSeqDescr(btp->bcp[count], Seq_descr_pir, vnp, NULL)) != NULL) 
	{
		string1 = PIRBlockPrintProc(btp, vnp);
		if (StringNCmp("host:host", string1, 9) == 0)
		{ 	/* The following takes out the PIR "host". */
			string = Cat2Strings("host", string1+9, ":", 0); 
			string1 = MemFree(string1);
		}
		else
			string = string1;
		
		if (newstring == NULL)
		{
			newstring = StringSave(string);
			string = MemFree(string);
		}
		else
		{
			temp = Cat2Strings(newstring, newline, ";", 1); 
			newstring = MemFree(newstring);
			temp1 = Cat2Strings(temp, identifier, NULL, 0); 
			newstring = Cat2Strings(temp1, string, NULL, 0); 
			string = MemFree(string);
			temp = MemFree(temp);
			temp1 = MemFree(temp1);
		}

	}
	return newstring;
}

static CharPtr SeqIdPrintProc (BiotablePtr btp, SeqIdPtr sip)

{
	StdPrintOptionsPtr Spop=btp->Spop;

	if (! StdFormatPrint((Pointer)sip, 
		(AsnWriteFunc)SeqIdAsnWrite, "StdSeqId", Spop))
		    ErrPostEx (SEV_WARNING, 0, 0, "StdFormatPrint failed\n");

	if (Spop->ptr != NULL && *((CharPtr) (Spop->ptr)) != '\0') 
		return Spop->ptr;
	else
		return NULL;
}

static CharPtr BioseqIdPrintProc (BiotablePtr btp, Int2 count)

{
	BioseqPtr bsp=btp->bsp[count];
	StdPrintOptionsPtr Spop=btp->Spop;

	if (! StdFormatPrint((Pointer)bsp, 
		(AsnWriteFunc)BioseqAsnWrite, "StdBioseqId", Spop))
		    ErrPostEx (SEV_WARNING, 0, 0, "StdFormatPrint failed\n");

	if (Spop->ptr != NULL && *((CharPtr) (Spop->ptr)) != '\0') 
		return Spop->ptr;
	else
		return NULL;
}


static CharPtr TableIdPrintProc (BiotablePtr btp, Int2 count)

{
	BioseqPtr bsp=btp->table[count];
	StdPrintOptionsPtr Spop=btp->Spop;

	if (bsp == NULL)
		return NULL;

	if (! StdFormatPrint((Pointer)bsp, 
		(AsnWriteFunc)BioseqAsnWrite, "StdBioseqId", Spop))
                    ErrPostEx (SEV_WARNING, 0, 0, "StdFormatPrint failed\n");

	if (Spop->ptr != NULL && *((CharPtr) (Spop->ptr)) != '\0') 
		return Spop->ptr;
	else
		return NULL;
}

static CharPtr SPBlockPrintProc (BiotablePtr btp, ValNodePtr vnp)

{
	StdPrintOptionsPtr Spop=btp->Spop;

	if (! StdFormatPrint((Pointer)vnp->data.ptrvalue, 
		(AsnWriteFunc)SPBlockAsnWrite, "StdSPBlock", Spop))
                    ErrPostEx (SEV_WARNING, 0, 0, "StdFormatPrint failed\n");

	if (Spop->ptr != NULL && *((CharPtr) (Spop->ptr)) != '\0') 
		return Spop->ptr;
	else
		return StringSave ("Empty Data\n");
}

static CharPtr PIRBlockPrintProc (BiotablePtr btp, ValNodePtr vnp)

{
	StdPrintOptionsPtr Spop=btp->Spop;

	if (! StdFormatPrint((Pointer)vnp->data.ptrvalue, 
		(AsnWriteFunc)PirBlockAsnWrite, "StdPirBlock", Spop))
                    ErrPostEx (SEV_WARNING, 0, 0, "StdFormatPrint failed\n");

	if (Spop->ptr != NULL && *((CharPtr) (Spop->ptr)) != '\0') 
		return Spop->ptr;
	else
		return StringSave ("Empty Data\n");
}

static CharPtr PDBBlockPrintProc (BiotablePtr btp, ValNodePtr vnp)

{
	StdPrintOptionsPtr Spop=btp->Spop;

	if (! StdFormatPrint((Pointer)vnp->data.ptrvalue, 
		(AsnWriteFunc)PdbBlockAsnWrite, "StdPDBBlock", Spop))
                    ErrPostEx (SEV_WARNING, 0, 0, "StdFormatPrint failed\n");

	if (Spop->ptr != NULL && *((CharPtr) (Spop->ptr)) != '\0') 
		return Spop->ptr;
	else
		return StringSave ("Empty Data\n");
}

static CharPtr PRFBlockPrintProc (BiotablePtr btp, ValNodePtr vnp)

{
	StdPrintOptionsPtr Spop=btp->Spop;

	if (! StdFormatPrint((Pointer)vnp->data.ptrvalue, 
		(AsnWriteFunc)PrfBlockAsnWrite, "StdPRFBlock", Spop))
                    ErrPostEx (SEV_WARNING, 0, 0, "StdFormatPrint failed\n");

	if (Spop->ptr != NULL && *((CharPtr) (Spop->ptr)) != '\0') 
		return Spop->ptr;
	else
		return StringSave ("Empty Data\n");
}


void GetGBDate (BiotablePtr btp, Int2 count)

{
	Char buffer[12];
	Int2 status;

	status = GetGenDate(btp, count, buffer);
	if (status != -1)
	{
		btp->date[count] = StringNCpy(btp->date[count], buffer, 11);
	}
	else 
	{ /* Nothing work, use something */
		StringCpy(btp->date[count], "01-JAN-1900");
		if (btp->error_msgs == TRUE && ASN2FF_DATE_ERROR_MSG == TRUE)
		{
			ErrPostEx (SEV_WARNING, ERR_DATE_IllegalDate, 
				"GetGBDate: No string or std date found\n");
		}
	}
	return;
}

void GetGPDate (BiotablePtr btp, Int2 count)

{
	BiotablePtr btp_na;
	Char buffer[12];
	Int2 status;

	status = GetGenDate(btp, count, buffer);
	if (status != -1)
	{
		btp->date[count] = StringNCpy(btp->date[count], buffer, 11);
	}
	else if ((OneOfThree(btp->btp_other->bsp[0])) == TRUE)
	{
		btp_na = btp->btp_other;
		status = GetGenDate(btp_na, 0, buffer);
		if (status != -1)
		{
			btp->date[count] = StringNCpy(btp->date[count], buffer, 11);
		}
	}

	if (status == -1)
	{ /* Nothing work, use something */
		StringCpy(btp->date[count], "01-JAN-1900");
		if (btp->error_msgs == TRUE && ASN2FF_DATE_ERROR_MSG == TRUE)
		{
			ErrPostEx (SEV_WARNING, ERR_DATE_IllegalDate, 
				"GetGPDate: No string or std date found\n");
		}
	}
	return;
}

Int2 GetGenDate(BiotablePtr btp, Int2 count, CharPtr buffer)

{

	EMBLBlockPtr ebp=NULL;
	GBBlockPtr gbp=NULL;
	Int2 date_choice, status;
	NCBI_DatePtr date=NULL, best_update_date=NULL, best_create_date=NULL;
	SPBlockPtr spp=NULL;
	PdbBlockPtr pbp=NULL;
	PdbRepPtr prp=NULL;
	ValNodePtr vnp;

	date_choice = -1;

	
	if ((vnp=BioseqContextGetSeqDescr(btp->bcp[count], Seq_descr_update_date, NULL, NULL)) != NULL) 
	{
		date = (NCBI_DatePtr) vnp->data.ptrvalue;
		if (date->data[0] == 1)
		{
			if (best_update_date) {
		  	     status = DateMatch(date, best_update_date, FALSE);
			     if (status == 1) {
				best_update_date = date; }
			} else {
				best_update_date = date; }
		}
	}

	vnp=NULL;
	while ((vnp=BioseqContextGetSeqDescr(btp->bcp[count], Seq_descr_embl, vnp, NULL)) != NULL) 
	{ /*	ebp possibly used again below. */
		ebp = (EMBLBlockPtr) vnp->data.ptrvalue;
		if (ebp->update_date != NULL)
		{
			if (ebp->update_date->data[0] == 1)
			{
				date = (NCBI_DatePtr) ebp->update_date;
				if (best_update_date) {
				     status = DateMatch(date, best_update_date, FALSE);
				     if (status == 1) {
					best_update_date = date; }
				} else {
					best_update_date = date; }
			}
			break;
		}
	}

	if ((vnp=BioseqContextGetSeqDescr(btp->bcp[count], Seq_descr_sp, NULL, NULL)) != NULL) 
	{ /*	spp possibly used again below. */
		spp = (SPBlockPtr) vnp->data.ptrvalue;
		if (spp->sequpd != NULL && spp->sequpd->data[0] == 1)
		{
			date = (NCBI_DatePtr) spp->sequpd;
			if (best_update_date) {
			     status = DateMatch(date, best_update_date, FALSE);
			     if (status == 1) {
				best_update_date = date; }
			} else {
				best_update_date = date; }
		}
		if (spp->annotupd != NULL && spp->annotupd->data[0] == 1)
		{
			date = (NCBI_DatePtr) spp->annotupd;
			if (best_update_date) {
			     status = DateMatch(date, best_update_date, FALSE);
			     if (status == 1) {
				best_update_date = date; }
			} else {
				best_update_date = date; }
		}
	}

	if ((vnp=BioseqContextGetSeqDescr(btp->bcp[count], Seq_descr_pdb, NULL, NULL)) != NULL) 
	{ /* pbp possibly used again below. */
		pbp = (PdbBlockPtr) vnp->data.ptrvalue;
		if ((prp = pbp->replace) != NULL && prp->date != NULL && prp->date->data[0] == 1)
		{
			date = (NCBI_DatePtr) prp->date;
			if (best_update_date) {
			     status = DateMatch(date, best_update_date, FALSE);
			     if (status == 1) {
				best_update_date = date; }
			} else {
				best_update_date = date; }
		}
	}

	if ((vnp=BioseqContextGetSeqDescr(btp->bcp[count], Seq_descr_create_date, NULL, NULL)) != NULL) 
	{
		date = (NCBI_DatePtr) vnp->data.ptrvalue;
		if (date->data[0] == 1)
		{
			if (best_create_date) {
		  	     status = DateMatch(date, best_create_date, FALSE);
			     if (status == 1) {
				best_create_date = date; }
			} else {
				best_create_date = date; }
		}
	}

	vnp=NULL;
	while ((vnp=BioseqContextGetSeqDescr(btp->bcp[count], Seq_descr_genbank, vnp, NULL)) != NULL) 
	{
		gbp = (GBBlockPtr) vnp->data.ptrvalue;
		if (gbp->entry_date != NULL )
		{
			if (gbp->entry_date->data[0] == 1)
			{
				date = gbp->entry_date;
				if (best_create_date) {
		  		     status = DateMatch(date, best_create_date, FALSE);
				     if (status == 1) {
					best_create_date = date; }
				} else {
					best_create_date = date; }
			}
			break;
		}
	}

	if (ebp) /* EMBLBlockPtr found above */
	{
		if (ebp->creation_date != NULL && ebp->creation_date->data[0] == 1)
		{
			date = ebp->creation_date;
			if (best_create_date) {
		  	     status = DateMatch(date, best_create_date, FALSE);
			     if (status == 1) {
				best_create_date = date; }
			} else {
				best_create_date = date; }
		}
	}

	if (spp) /* SPBlockPtr found above */
	{
		if (spp->created != NULL && spp->created->data[0] == 1)
		{
			date = spp->created;
			if (best_create_date) {
		  	     status = DateMatch(date, best_create_date, FALSE);
			     if (status == 1) {
				best_create_date = date; }
			} else {
				best_create_date = date; }
		}
	}

	if (pbp) /* PDB Block found above */
	{
		if (pbp->deposition != NULL && pbp->deposition->data[0] == 1)
		{
			date = pbp->deposition;
			if (best_create_date) {
		  	     status = DateMatch(date, best_create_date, FALSE);
			     if (status == 1) {
				best_create_date = date; }
			} else {
				best_create_date = date; }
		}
	}

	if (best_update_date || best_create_date)
	{
		date_choice = 1;
	}
	else
	{
		date = NULL;
		if ((vnp=BioseqContextGetSeqDescr(btp->bcp[count], Seq_descr_update_date, NULL, NULL)) != NULL) 
		{
			date = (NCBI_DatePtr) vnp->data.ptrvalue;
			if (date->data[0] == 0)
				best_update_date = date; 
		}

		if (!best_update_date && ebp)
			if (ebp->update_date != NULL && ebp->update_date->data[0] == 0)
				best_update_date = (NCBI_DatePtr) ebp->update_date;

		if (!best_update_date && spp)
		{
			if (spp->sequpd != NULL && spp->sequpd->data[0] == 0)
			{
				best_update_date = (NCBI_DatePtr) spp->sequpd;
			}
			else if (spp->annotupd != NULL && spp->annotupd->data[0] == 0)
			{
				best_update_date = (NCBI_DatePtr) spp->annotupd;
			}
		}

		if (!best_update_date && prp)
			if (prp->date && prp->date->data[0] == 0)
				best_update_date = (NCBI_DatePtr) prp->date;

		if (!best_update_date && (vnp=BioseqContextGetSeqDescr(btp->bcp[count], Seq_descr_create_date, NULL, NULL)) != NULL) 
		{
			date = (NCBI_DatePtr) vnp->data.ptrvalue;
			if (date->data[0] == 0)
				best_create_date = date;
		}

		if (!best_update_date && gbp)
			if (gbp->entry_date && gbp->entry_date->data[0] == 0)
				best_update_date = gbp->entry_date;

		if (!best_create_date && ebp) /* EMBLBlockPtr found above */
		{
			if (ebp->creation_date && ebp->creation_date->data[0] == 0)
				best_create_date = ebp->creation_date;
		}

		if (!best_create_date && spp) /* SPBlockPtr found above */
		{
			if (spp->created && spp->created->data[0] == 0)
				best_create_date = spp->created;
		}

		if (!best_create_date && pbp) /* PDB Block found above */
		{
			if (pbp->deposition && pbp->deposition->data[0] == 0)
				best_create_date = pbp->deposition;
		}
		if (best_update_date || best_create_date)
			date_choice = 0;
	}

	if (date_choice == 1)
	{
		if (best_update_date && best_create_date)
		{
			status = DateMatch(best_update_date, best_create_date, FALSE);
			if (status == 0 || status == 1)
			{
				buffer = DateToGB(buffer, best_update_date, btp->error_msgs);
			} else {
				buffer = DateToGB(buffer, best_create_date, btp->error_msgs);
				if (btp->error_msgs == TRUE && ASN2FF_DATE_ERROR_MSG == TRUE)
				{
					ErrPostEx(SEV_WARNING, ERR_DATE_Create_after_update, "GetGBDate: std create date after update date");
				}
			}
		} else if (best_update_date) {
			buffer = DateToGB(buffer, best_update_date, btp->error_msgs);
		} else if (best_create_date) {
			buffer = DateToGB(buffer, best_create_date, btp->error_msgs);
		}
	}
	else if (date_choice == 0) 
	{
		if (best_update_date)
		{
			buffer = StringNCpy(buffer, best_update_date->str, 11);
			btp->date[count][11]='\0';
			if (btp->error_msgs == TRUE && ASN2FF_DATE_ERROR_MSG == TRUE)
			{
				ErrPostEx(SEV_INFO, ERR_DATE_IllegalDate, 
					"GetGBDate: Only string update date found");
			}
		} 
		else if (best_create_date) 
		{
			buffer = StringNCpy(buffer, best_create_date->str, 11);
			buffer[11]='\0';
			if (btp->error_msgs == TRUE && ASN2FF_DATE_ERROR_MSG == TRUE)
			{
				ErrPostEx(SEV_INFO, ERR_DATE_IllegalDate, 
					"GetGBDate: Only string create date found");
			}
		}
	}
	
	return date_choice;
}	/* GetGenDate */

void GetEMBLDate (BiotablePtr btp, Int2 count)

{

	Char buf_update[12], buf_create[12];
	EMBLBlockPtr ebp=NULL;
	GBBlockPtr gbp=NULL;
	Int2 status;
	NCBI_DatePtr date=NULL, best_update_date=NULL, best_create_date=NULL;
	SPBlockPtr spp=NULL;
	PdbBlockPtr pbp=NULL;
	PdbRepPtr prp=NULL;
	ValNodePtr vnp;

	buf_create[0] = '\0';
	buf_update[0] = '\0';

	
	if ((vnp=BioseqContextGetSeqDescr(btp->bcp[count], Seq_descr_update_date, NULL, NULL)) != NULL) 
	{
		date = (NCBI_DatePtr) vnp->data.ptrvalue;
		if (date->data[0] == 1)
		{
			if (best_update_date) {
		  	     status = DateMatch(date, best_update_date, FALSE);
			     if (status == 1) {
				best_update_date = date; }
			} else {
				best_update_date = date; }
		}
	}

	vnp=NULL;
	while ((vnp=BioseqContextGetSeqDescr(btp->bcp[count], Seq_descr_embl, vnp, NULL)) != NULL) 
	{ /*	ebp possibly used again below. */
		ebp = (EMBLBlockPtr) vnp->data.ptrvalue;
		if (ebp->update_date != NULL)
		{
			if (ebp->update_date->data[0] == 1)
			{
				date = (NCBI_DatePtr) ebp->update_date;
				if (best_update_date) {
				     status = DateMatch(date, best_update_date, FALSE);
				     if (status == 1) {
					best_update_date = date; }
				} else {
					best_update_date = date; }
			}
			break;
		}
	}

	if ((vnp=BioseqContextGetSeqDescr(btp->bcp[count], Seq_descr_sp, NULL, NULL)) != NULL) 
	{ /*	spp possibly used again below. */
		spp = (SPBlockPtr) vnp->data.ptrvalue;
		if (spp->sequpd != NULL && spp->sequpd->data[0] == 1)
		{
			date = (NCBI_DatePtr) spp->sequpd;
			if (best_update_date) {
			     status = DateMatch(date, best_update_date, FALSE);
			     if (status == 1) {
				best_update_date = date; }
			} else {
				best_update_date = date; }
		}
		if (spp->annotupd != NULL && spp->annotupd->data[0] == 1)
		{
			date = (NCBI_DatePtr) spp->annotupd;
			if (best_update_date) {
			     status = DateMatch(date, best_update_date, FALSE);
			     if (status == 1) {
				best_update_date = date; }
			} else {
				best_update_date = date; }
		}
	}

	if ((vnp=BioseqContextGetSeqDescr(btp->bcp[count], Seq_descr_pdb, NULL, NULL)) != NULL) 
	{ /* pbp possibly used again below. */
		pbp = (PdbBlockPtr) vnp->data.ptrvalue;
		if ((prp = pbp->replace) != NULL && prp->date != NULL && prp->date->data[0] == 1)
		{
			date = (NCBI_DatePtr) prp->date;
			if (best_update_date) {
			     status = DateMatch(date, best_update_date, FALSE);
			     if (status == 1) {
				best_update_date = date; }
			} else {
				best_update_date = date; }
		}
	}

	if ((vnp=BioseqContextGetSeqDescr(btp->bcp[count], Seq_descr_create_date, NULL, NULL)) != NULL) 
	{
		date = (NCBI_DatePtr) vnp->data.ptrvalue;
		if (date->data[0] == 1)
		{
			if (best_create_date) {
		  	     status = DateMatch(date, best_create_date, FALSE);
			     if (status == 1) {
				best_create_date = date; }
			} else {
				best_create_date = date; }
		}
	}

	vnp=NULL;
	while ((vnp=BioseqContextGetSeqDescr(btp->bcp[count], Seq_descr_genbank, vnp, NULL)) != NULL) 
	{
		gbp = (GBBlockPtr) vnp->data.ptrvalue;
		if (gbp->entry_date != NULL )
		{
			if (gbp->entry_date->data[0] == 1)
			{
				date = gbp->entry_date;
				if (best_create_date) {
		  		     status = DateMatch(date, best_create_date, FALSE);
				     if (status == 1) {
					best_create_date = date; }
				} else {
					best_create_date = date; }
			}
			break;
		}
	}

	if (ebp) /* EMBLBlockPtr found above */
	{
		if (ebp->creation_date != NULL && ebp->creation_date->data[0] == 1)
		{
			date = ebp->creation_date;
			if (best_create_date) {
		  	     status = DateMatch(date, best_create_date, FALSE);
			     if (status == 1) {
				best_create_date = date; }
			} else {
				best_create_date = date; }
		}
	}

	if (spp) /* SPBlockPtr found above */
	{
		if (spp->created != NULL && spp->created->data[0] == 1)
		{
			date = spp->created;
			if (best_create_date) {
		  	     status = DateMatch(date, best_create_date, FALSE);
			     if (status == 1) {
				best_create_date = date; }
			} else {
				best_create_date = date; }
		}
	}

	if (pbp) /* PDB Block found above */
	{
		if (pbp->deposition != NULL && pbp->deposition->data[0] == 1)
		{
			date = pbp->deposition;
			if (best_create_date) {
		  	     status = DateMatch(date, best_create_date, FALSE);
			     if (status == 1) {
				best_create_date = date; }
			} else {
				best_create_date = date; }
		}
	}

	if (best_update_date)
		DateToGB(buf_update, best_update_date, btp->error_msgs);
	if (best_create_date)
		DateToGB(buf_create, best_create_date, btp->error_msgs);

	if (buf_create[0] == '\0')
	{
		if (!best_create_date && ebp) /* EMBLBlockPtr found above */
		{
			if (ebp->creation_date && ebp->creation_date->data[0] == 0)
				best_create_date = ebp->creation_date;
		}

		if (!best_create_date && spp) /* SPBlockPtr found above */
		{
			if (spp->created && spp->created->data[0] == 0)
				best_create_date = spp->created;
		}

		if (!best_create_date && pbp) /* PDB Block found above */
		{
			if (pbp->deposition && pbp->deposition->data[0] == 0)
				best_create_date = pbp->deposition;
		}
		if (best_create_date)
		{
			StringNCpy(buf_create, best_create_date->str, 11);
			buf_create[11]='\0';
			if (btp->error_msgs == TRUE && ASN2FF_DATE_ERROR_MSG == TRUE)
			{
				ErrPostEx(SEV_INFO, ERR_DATE_IllegalDate, 
					"GetEMBLDate: Only string create date found");
			}
		}
		else
		{
			StringCpy(buf_create, "01-JAN-1900");
			if (btp->error_msgs == TRUE && ASN2FF_DATE_ERROR_MSG == TRUE)
			{
				ErrPostEx(SEV_INFO, ERR_DATE_IllegalDate, 
					"GetEMBLDate: No string or std create date found");
			}
		}
	}

	if (buf_update[0] == '\0')
	{
		date = NULL;
		if ((vnp=BioseqContextGetSeqDescr(btp->bcp[count], Seq_descr_update_date, NULL, NULL)) != NULL) 
		{
			date = (NCBI_DatePtr) vnp->data.ptrvalue;
			if (date->data[0] == 0)
				best_update_date = date; 
		}

		if (!best_update_date && ebp)
			if (ebp->update_date != NULL && ebp->update_date->data[0] == 0)
				best_update_date = (NCBI_DatePtr) ebp->update_date;

		if (!best_update_date && spp)
		{
			if (spp->sequpd != NULL && spp->sequpd->data[0] == 0)
			{
				best_update_date = (NCBI_DatePtr) spp->sequpd;
			}
			else if (spp->annotupd != NULL && spp->annotupd->data[0] == 0)
			{
				best_update_date = (NCBI_DatePtr) spp->annotupd;
			}
		}

		if (!best_update_date && prp)
			if (prp->date && prp->date->data[0] == 0)
				best_update_date = (NCBI_DatePtr) prp->date;

		if (!best_update_date && (vnp=BioseqContextGetSeqDescr(btp->bcp[count], Seq_descr_create_date, NULL, NULL)) != NULL) 
		{
			date = (NCBI_DatePtr) vnp->data.ptrvalue;
			if (date->data[0] == 0)
				best_create_date = date;
		}

		if (!best_update_date && gbp)
			if (gbp->entry_date && gbp->entry_date->data[0] == 0)
				best_update_date = gbp->entry_date;

		if (best_update_date)
		{
			StringNCpy(buf_update, best_update_date->str, 11);
			buf_update[11]='\0';
			if (btp->error_msgs == TRUE && ASN2FF_DATE_ERROR_MSG == TRUE)
			{
				ErrPostEx(SEV_INFO, ERR_DATE_IllegalDate, 
					"GetEMBLDate: Only string update date found");
			}
		}
	}


	btp->create_date[count] = StringSave(buf_create);
	if (buf_update[0] != '\0')
		btp->update_date[count] = StringSave(buf_update);
	
	return;

}	/* GetEMBLDate */

void GetVersion (BiotablePtr btp, Int2 count)

{
	BioseqPtr bsp=btp->bsp[count];
	SeqIdPtr isip, sip;
	TextSeqIdPtr tsip;

	isip = bsp->id;

	for (sip=isip; sip; sip=sip->next)
	{
		if (sip->choice == 6)
		{
			tsip = (TextSeqIdPtr) sip->data.ptrvalue;
			if (tsip->release)
				btp->embl_rel[count] = tsip->release;
			if (tsip->version)
				btp->embl_ver[count] = tsip->version;
		}
	}
}	/* GetVersion */

CharPtr DateToGB (CharPtr buf, NCBI_DatePtr ndp, Boolean error_msgs)

{
	static CharPtr month_names[] = {
	"JAN",
	"FEB",
	"MAR",
	"APR",
	"MAY",
	"JUN",
	"JUL",
	"AUG",
	"SEP",
	"OCT",
	"NOV",
	"DEC"
	};

	int month = 1,
		day = 1,
		year = 1900,
		tens_place = 0;	/* in case day is less than ten.	*/
	
	if (ndp->data[0] == 0)
	{
		buf = StringCpy(buf, ndp->str);
	}
	else if (ndp->data[0] == 1)   /* std date */
	{
		year = 1900 + (int)(ndp->data[1]);
		if (ndp->data[2] != 0)
		   month = (int)(ndp->data[2]);
		if (ndp->data[3] != 0)
			day = (int)(ndp->data[3]);

		if (day < 10)
			sprintf(buf, "%ld%ld-%s-%ld", 
				(long) tens_place, (long) day, (long) month_names[month-1], (long) year);
		else
			sprintf(buf, "%ld-%s-%ld", 
				(long) day, (long) month_names[month-1], (long) year);

	}
	else
	{
		if (error_msgs == TRUE && ASN2FF_DATE_ERROR_MSG == TRUE)
			ErrPostEx(SEV_WARNING, ERR_DATE_IllegalDate, "Unknown Date type in DateToGB\n");
		*buf = '\0';
	}
		return buf;
}

/*************************************************************************
*GB_PrintPubs
*
*	"GB_PrintPubs" to dump pubs in Flat File (i.e., Genbank) format.
*
**************************************************************************/

void GB_PrintPubs (BiotablePtr btp, Int2 count, PubStructPtr psp)

{

	BioseqPtr bsp=btp->bsp[count];
	Boolean first_time, ignore_this=FALSE, submit=FALSE, tag;
	Char buffer[150];
	CharPtr authors=NULL, title=NULL, journal=NULL, string_start, string;
	Int2 i;
	Int4 gibbsq, muid, pat_seqid=0, start=0, stop=0;
	PubdescPtr pdp;
	SeqFeatPtr sfp;
	SeqLocPtr loc, slp;
	ValNodePtr pub;

	if (ASN2FF_SHOW_ALL_PUBS) {
		pub = FlatRefBest(psp->pub, btp->error_msgs, FALSE);
	} else {
		pub = FlatRefBest(psp->pub, btp->error_msgs, TRUE);
	}
	if (pub == NULL)
	{
		if (btp->error_msgs == TRUE)
			PostARefErrMessage (bsp, psp, NULL, -1, NULL);
		return;
	}
	ignore_this = FlatIgnoreThisPatentPub(bsp, pub, &pat_seqid);
	if (btp->format != GENPEPT)
	{
		if (ignore_this == TRUE)
		{
			if (btp->error_msgs == TRUE)
				PostARefErrMessage (bsp, psp, NULL, -1, NULL);
			return;
		}
	}

	StartPrint(btp, 0, 12, ASN2FF_GB_MAX, NULL);
	AddString(btp, "REFERENCE");
	TabToColumn(btp, 13);
	AddInteger(btp, "%ld", (long) psp->number);
	if (psp->start == 1)
	{
		TabToColumn(btp, 16);
		if (btp->format != GENPEPT)
			AddString(btp, "(bases 1 to ");
		else
			AddString(btp, "(residues 1 to ");
		AddInteger(btp, "%ld", (long) bsp->length);
		AddChar(btp, ')');
	}
	else if (psp->start == 2)
	{
		TabToColumn(btp, 16);
		if (btp->format != GENPEPT)
			AddString(btp, "(bases ");
		else
			AddString(btp, "(residues ");
		for (i=0; i<psp->citcount; i++)
		{
			sfp = psp->citfeat[i];
			loc = (SeqLocPtr) sfp->location;
			slp = GetBaseRangeForCitation (loc, NULL, &start, &stop);
			if (start != 0 || stop != 0)
			{
				AddInteger(btp, "%ld", (long) start);
				AddString(btp, " to ");
				AddInteger(btp, "%ld", (long) stop);
				if (slp != NULL || i+1 != psp->citcount)
					AddString(btp, "; ");
			}
			while (slp != NULL)
			{
				slp = GetBaseRangeForCitation (loc, slp, &start, &stop);
				if (start != 0 || stop != 0)
				{
					AddInteger(btp, "%ld", (long) start);
					AddString(btp, " to ");
					AddInteger(btp, "%ld", (long) stop);
					if (slp != NULL || i+1 != psp->citcount)
						AddString(btp, "; ");
				}
			}
		}
		AddChar(btp, ')');
	}
	else if (psp->start == 3) 
	{
		TabToColumn(btp, 16);
		AddString(btp, "(sites)");
	}
	else
	{
		if (btp->error_msgs == TRUE)
			ErrPostEx(SEV_WARNING, CTX_NCBI2GB, 1, 
			 "Incorrect start value (%d) in PubStruct\n", psp->start);
	}
	EndPrint(btp);

	authors = FlatAuthor(btp, pub);
	StartPrint(btp, 2, 12, ASN2FF_GB_MAX, NULL);
	AddString(btp, "AUTHORS");
	TabToColumn(btp, 13);

	if (authors && *authors != NULLB)
	{
		AddString(btp, authors);
	}
	else
	{
		AddChar(btp, '.');
	}
	EndPrint(btp);

	title = FlatPubTitle(pub);
	if (title ){
		if ( *title  != NULLB){
			StartPrint(btp, 2, 12, ASN2FF_GB_MAX, NULL);
			AddString(btp, "TITLE");
			TabToColumn(btp, 13);
			AddString(btp, title);
			EndPrint(btp);
		}
	}

	journal = FlatJournal(btp, count, pub, pat_seqid, &submit, FALSE);
	StartPrint(btp, 2, 12, ASN2FF_GB_MAX, NULL);
	AddString(btp, "JOURNAL");
	TabToColumn(btp, 13);
	if (journal )
		AddString(btp, journal);
	EndPrint(btp);

	muid = GetMuid(psp->pub);
	if (muid > 0)
	{
		StartPrint(btp, 2, 12, ASN2FF_GB_MAX, NULL);
		AddString(btp, "MEDLINE");
		TabToColumn(btp, 13);
		AddInteger(btp, "%ld", (long) muid);
		EndPrint(btp);
	}

	tag = FALSE;
	pdp = psp->descr;
	if (pdp != NULL && pdp->comment != NULL)
	{
		if (StringCmp(pdp->comment, "full automatic") != 0 &&
		  StringCmp(pdp->comment, "full staff_review") != 0 &&
		   StringCmp(pdp->comment, "full staff_entry") != 0 &&
		    StringCmp(pdp->comment, "simple staff_review") != 0 &&
		      StringCmp(pdp->comment, "simple staff_entry") != 0 &&
		       StringCmp(pdp->comment, "simple automatic") != 0 &&
		        StringCmp(pdp->comment, "unannotated automatic") != 0 &&
		         StringCmp(pdp->comment, "unannotated staff_review") != 0 &&
		          StringCmp(pdp->comment, "unannotated staff_entry") != 0)
		{
			StartPrint(btp, 2, 12, ASN2FF_GB_MAX, NULL);
			AddString(btp, "REMARK");
			TabToColumn(btp, 13);
			AddStringWithTildes(btp, pdp->comment);
			tag = TRUE;
		}
	}
	string = &buffer[0];
	gibbsq = GetGibbsqStatement(btp, count, string);
	if (gibbsq > 0)
	{
		if (tag != TRUE)
		{
			StartPrint(btp, 2, 12, ASN2FF_GB_MAX, NULL);
			AddString(btp, "REMARK");
			TabToColumn(btp, 13);
		}
		else
			NewContLine(btp);
		AddStringWithTildes(btp, string);
		tag = TRUE;
	}
	string = GetGibbsqComment(btp, count);
	if (string)
	{
		string_start = string;
		if (tag != TRUE)
		{
			StartPrint(btp, 2, 12, ASN2FF_GB_MAX, NULL);
			AddString(btp, "REMARK");
			TabToColumn(btp, 13);
		}
		else
			NewContLine(btp);
		first_time = TRUE;
/* Can't this be rewritten to use AddString????  That would be faster! */
		while (*string != '\0')
		{
			if (*string == '~')
			{
				if (first_time == FALSE)
					NewContLine(btp);
				else
					first_time = FALSE;
			}
			else if (*string == '\"')
			{
				*string = '\'';
				AddChar(btp, *string);
			}
			else 
				AddChar(btp, *string);
			string++;
		}
		string_start = MemFree(string_start);
		tag=TRUE;
	}
	if (tag == TRUE)
		EndPrint(btp);

	if (authors)
		MemFree(authors);
	MemFree(title);
	MemFree(journal);
}	/* GB_PrintPubs */


Int4
GetMuid(ValNodePtr equiv)

{
	Int4 muid=0;
	ValNodePtr newpub, the_pub;

        if (equiv->choice == PUB_Equiv)
                newpub = equiv->data.ptrvalue;
        else
                newpub = equiv;

        for (the_pub = newpub; the_pub; the_pub = the_pub -> next)
	{
		if (the_pub->choice == PUB_Muid)
		{
			muid = the_pub->data.intvalue;
			break;
		}

	}

	return muid;

}	/* GetMuid */

/*************************************************************************
*EMBL_PrintPubs
*
*	"EMBL_PrintPubs" to dump pubs in FlatFile (EMBL) format.
*
**************************************************************************/

void EMBL_PrintPubs (BiotablePtr btp, Int2 count, PubStructPtr psp)

{

	BioseqPtr bsp=btp->bsp[count];
	Boolean ignore_this=FALSE, submit=FALSE;
	CharPtr authors=NULL, title=NULL, journal=NULL, new_journal;
	Int2 i;
	Int4 pat_seqid=0;	
	Int4 start=0, stop=0, tmp_range, range;
	PubdescPtr descr=psp->descr;
	SeqFeatPtr sfp;
	SeqLocPtr loc, slp;
	ValNodePtr pub;

	pub = FlatRefBest(psp->pub, btp->error_msgs, FALSE);
	if (pub == NULL)
	{
		if (btp->error_msgs == TRUE)
			ErrPostEx(SEV_WARNING, ERR_REFERENCE_Illegalreference, "FFDumpPubs: Invalid Pub found.");
		return;
	}
	ignore_this = FlatIgnoreThisPatentPub(bsp, pub, &pat_seqid);
	if (ignore_this == TRUE && ASN2FF_IGNORE_PATENT_PUBS != FALSE)
	{
		if (btp->error_msgs == TRUE)
			ErrPostEx(SEV_WARNING, ERR_REFERENCE_Illegalreference, "FFDumpPubs: Invalid Patent Pub");
		return;
	}

	PrintXX(btp);

	StartPrint(btp, 5, 5, ASN2FF_EMBL_MAX, "RN");
	AddChar(btp, '[');
	AddInteger(btp, "%ld", (long) psp->number);
	AddChar(btp, ']');
	EndPrint(btp);
	if (psp->start == 1)
	{
		StartPrint(btp, 5, 5, ASN2FF_EMBL_MAX, "RP");
		AddString(btp, "1-");
		AddInteger(btp, "%ld", (long) bsp->length);
		EndPrint(btp);
	}
	else if (psp->start == 2)
	{
		range = 0;
		StartPrint(btp, 5, 5, ASN2FF_EMBL_MAX, "RP");
		for (i=0; i<psp->citcount; i++)
		{
			sfp = psp->citfeat[i];
			loc = (SeqLocPtr) sfp->location;
			slp = GetBaseRangeForCitation (loc, NULL, &start, &stop);
			if (start != 0 || stop != 0)
			{ /* Why do I need the tmp_range test??? */
				tmp_range = stop - start;
				if (tmp_range >= range)
				{
					range = tmp_range;
					AddInteger(btp, "%ld", (long) start);
					AddChar(btp, '-');
					AddInteger(btp, "%ld", (long) stop);
					if (slp != NULL || i+1 != psp->citcount)
						AddString(btp, ", ");
				}
			}
			while (slp != NULL)
			{
				slp = GetBaseRangeForCitation (loc, slp, &start, &stop);
				if (start != 0 || stop != 0)
				{
					AddInteger(btp, "%ld", (long) start);
					AddChar(btp, '-');
					AddInteger(btp, "%ld", (long) stop);
					if (slp != NULL || i+1 != psp->citcount)
						AddString(btp, ", ");
				}
			}
		}
		EndPrint(btp);
	}
	else if (psp->start == 3 && btp->pseudo == TRUE) 
	{ /* "sites" only for pseudo-embl.  */
		StartPrint(btp, 5, 5, ASN2FF_EMBL_MAX, "RP");
		AddString(btp, "(sites)");
		EndPrint(btp);
	}

	journal = FlatJournal(btp, count, pub, pat_seqid, &submit, FALSE);

	if (descr && descr->comment)
	{
		StartPrint(btp, 5, 5, ASN2FF_EMBL_MAX, "RC");
		AddString(btp, descr->comment);
		EndPrint(btp);
	}
	authors = FlatAuthor(btp, pub);
	StartPrint(btp, 5, 5, ASN2FF_EMBL_MAX, "RA");
	if (authors)
		AddString(btp, authors);
	AddChar(btp, ';');
	EndPrint(btp);
	
	StartPrint(btp, 5, 5, ASN2FF_EMBL_MAX, "RT");
	if (! submit)
	{
		title = FlatPubTitle(pub);
		if (title ){
			if ( *title )
			{
				AddChar(btp, '\"');
				AddString(btp, title);
				AddChar(btp, '\"');
			}
		}
	}
	AddChar(btp, ';');
	EndPrint(btp);

	StartPrint(btp, 5, 5, ASN2FF_EMBL_MAX, "RL");
	if (journal)
	{
		new_journal = CheckEndPunctuation(journal, '.');
		AddString(btp, new_journal);
		new_journal = MemFree(new_journal);
	}
	EndPrint(btp);

	if (authors)
		MemFree(authors);
	MemFree(title);
	MemFree(journal);
}	/* EMBL_PrintPubs */

/***************************************************************************
* SeqLocPtr GetBaseRangeForCitation (SeqLocPtr loc, SeqLocPtr slp, Int4Ptr start, Int4Ptr stop)
*
*
*	This function finds the start and stop Int4 values for a location.
*	If this is a cmplex location (e.g., SEQLOC_MIX), then the
*	function is called several times, with the returned slp used
*	as an argument on the next round.
*	
*	The first call should be with slp set to NULL.
*
****************************************************************************/

SeqLocPtr GetBaseRangeForCitation (SeqLocPtr loc, SeqLocPtr slp, Int4Ptr start, Int4Ptr stop)
{
	Int4 tmp_start, tmp_stop, tmp_range;
	
	*start = 0;
	*stop = 0;

	switch (loc->choice)
	{
		case SEQLOC_BOND:   
       		case SEQLOC_FEAT:   
        	case SEQLOC_NULL:    
        	case SEQLOC_EMPTY:  
			slp = NULL;
                        break;
        	case SEQLOC_WHOLE:
        	case SEQLOC_INT:
			if ((tmp_start = SeqLocStart(loc)) >= 0  &&
					(tmp_stop = SeqLocStop(loc)) >= 0)
			{
				tmp_range = tmp_stop - tmp_start;
				if (tmp_range >= 0)
				{ /* +1 for Genbank format. */
					*start = tmp_start+1;
					*stop = tmp_stop+1;
				}
			}
			slp = NULL;
			break;
        	case SEQLOC_MIX:
        	case SEQLOC_EQUIV:
        	case SEQLOC_PACKED_INT:
			if (slp == NULL)
				slp = loc->data.ptrvalue;
			if (slp != NULL)
			{
				if ((tmp_start = SeqLocStart(slp)) >= 0  &&
						(tmp_stop = SeqLocStop(slp)) >= 0)
				{
					tmp_range = tmp_stop - tmp_start;
					if (tmp_range >= 0)
					{ /* +1 for Genbank format. */
						*start = tmp_start+1;
						*stop = tmp_stop+1;
					}
				}
				slp = slp->next;
			}
			break;
        	case SEQLOC_PACKED_PNT: 
       		case SEQLOC_PNT:
			slp = NULL;
			break;
		default:
			slp = NULL;
			break;
	}
	return slp;
}	/* GetBaseRangeForCitation */

