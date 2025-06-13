/*************************************************************************
*
*	"pub.c" which contains files relating to publications.
*
**************************************************************************/
/* $Revision: 1.54 $ */ 
#include "asn2ffp.h"
#include "parsegb.h"
#define CTX_2GB_NOT_IMPLEMENTED 1
/***************************************************************************
*
*	PROTOTYPES, mostly for functions from Karl Sirotkin's code.
*
*************************************************************************/
static CharPtr GetSubmitterName PROTO ((BiotablePtr btp, ValNodePtr pub));
CharPtr FlatStringGroupTerm PROTO ((BiotablePtr btp, CharPtr start, ValNodePtr head, CharPtr delimit, CharPtr period, CharPtr term));
Int2 fix_pages PROTO ((CharPtr out_pages, CharPtr in_pages));
CharPtr FlatDateFromCreate PROTO ((CharPtr default_date, NCBI_DatePtr flat_date));
ValNodePtr GBGetAuthNames PROTO ((BiotablePtr btp, AuthListPtr ap));
CharPtr GetAffiliation PROTO ((AffilPtr afp));
Int2 ValidatePub PROTO ((BioseqPtr bsp, ValNodePtr the_pub));
static CharPtr format_article PROTO ((BiotablePtr btp, Int2 count, ValNodePtr the_pub, Boolean make_index));
static CharPtr format_cit_sub PROTO ((BiotablePtr btp, Int2 count, ValNodePtr the_pub, Boolean PNTR submit, Boolean make_index));
static CharPtr format_general PROTO ((BiotablePtr btp, Int2 count, ValNodePtr the_pub, Boolean make_index));
static CharPtr format_patent PROTO ((BiotablePtr btp, Int2 count, ValNodePtr the_pub, Int4 pat_seqid, Boolean make_index));
static CharPtr format_thesis PROTO ((BiotablePtr btp, Int2 count, ValNodePtr the_pub, Boolean make_index));
void PostARefErrMessage PROTO ((BioseqPtr bsp, PubStructPtr psp, ValNodePtr ext_pub, Int2 status, CharPtr string));
static GBQualPtr GBQualCopy PROTO ((GBQualPtr old_qual));
static CharPtr StripParanthesis PROTO ((CharPtr ptr_in, Boolean PNTR paranthesis));
static CharPtr makeaffil PROTO ((AffilPtr afp, CharPtr temp, Boolean PNTR first_std));
static CharPtr doSup PROTO ((CharPtr temp, CharPtr issue, CharPtr part_sup, CharPtr part_supi));
static CharPtr format_bookarticle PROTO ((BiotablePtr btp, Int2 count, ValNodePtr the_pub, Boolean make_index));
static CharPtr format_jourarticle PROTO ((BiotablePtr btp, Int2 count, ValNodePtr the_pub, Boolean make_index));

/*****************************************************************************
*GetAuthors
*
*	taken (with minor modifications) from Karl Sirotkin's code 
*	by Tom Madden.
*
*****************************************************************************/

/*----------- GetAuthors ()------*/
ValNodePtr 
GetAuthors 
(BiotablePtr btp, ValNodePtr the_pub)
{

	AuthListPtr ap;
	CharPtr tmp;
	CitArtPtr ca;
	CitBookPtr cb;
	CitGenPtr cg;	
	CitSubPtr cs;
	CitPatPtr cp;
	MedlineEntryPtr ml;
	ValNodePtr spare, this_name, namehead=NULL;

	switch ( the_pub -> choice){

	case 9:
		cp = (CitPatPtr) the_pub -> data.ptrvalue;
		ap = cp  -> authors;
		break;
	case 11:
		cb = (CitBookPtr) the_pub -> data.ptrvalue;
		ap = cb -> authors;
		break;
	case 5:
	case 3:
		if ( the_pub -> choice == 3){
			ml = (MedlineEntryPtr) the_pub -> data.ptrvalue;
			ca = (CitArtPtr) ml -> cit;
		}else{
			ca = (CitArtPtr) the_pub -> data.ptrvalue;
		}
		ap = ca -> authors;
		break;
	case 1: 
		cg = (CitGenPtr) the_pub -> data.ptrvalue;
		ap = cg -> authors;
		break;
	case 2:
		cs = (CitSubPtr)  the_pub -> data.ptrvalue;
		ap = cs -> authors;
		break;
	default:
		if (btp->error_msgs == TRUE)
			ErrPostEx(SEV_WARNING, ERR_REFERENCE_Illegalreference,
			"FlatAuthor: Unimplemented pub type=%d\n", 
			(int) the_pub -> choice);
	}

	if (ap){
		if (ap->choice != 1){   /* just strings */
			for (spare = ap->names, namehead = NULL; 
					spare; spare = spare -> next){
				this_name = ValNodeNew(namehead);
				if (namehead == NULL)
					namehead = this_name;
				this_name -> data.ptrvalue = StringSave(spare -> data.ptrvalue);
				if (btp->format == EMBL)
				{
					for (tmp=(CharPtr) this_name->data.ptrvalue; 
						*tmp != '\0'; tmp++)
					{
						if (*tmp == ',')
						{
							*tmp = ' ';
							break;
						}
					}
				}
			}
		} 
		else                  /* structured names */
		{
			namehead = GBGetAuthNames(btp, ap);
		}
	}

	return namehead;

}	/* GetAuthors */

/*----------- FlatAuthor ()------*/
CharPtr 
FlatAuthor 
(BiotablePtr btp, ValNodePtr the_pub)
{
	CharPtr ret_save, temp, retval=NULL;
	int len;
	ValNodePtr namehead;

	namehead = GetAuthors(btp, the_pub);

	if (btp->format == EMBL)
		retval = FlatStringGroupTerm(btp, NULL, namehead, ", ", NULL, ", ");
	else
		retval = FlatStringGroupTerm(btp, NULL, namehead, ", ", NULL, " and ");

	ValNodeFreeData(namehead);     /* remove name list */

	if (retval) {
		len = StringLen(retval);
		if (len > 0){
			if ( retval[len-1] != '.') {
				ret_save = retval;
				temp = retval = MemNew(len+2);
				temp = StringMove(temp,ret_save);
				temp = StringMove(temp,".");
				MemFree (ret_save);
			}
		}
	}

	return FlatCleanEquals(retval);

}	/* FlatAuthor */


/*****************************************************************************
*FlatCleanEquals
*
*	taken (with minor modifications) from Karl Sirotkin's code 
*	by Tom Madden.
*
*****************************************************************************/

	/*--------FlatCleanEquals()----------*/

CharPtr FlatCleanEquals
(CharPtr retval)
{
	CharPtr p=retval;

	if (p){
		for (;*p; p++){
			if (*p == '\"')
				*p = '\'';
		}
	}

return retval;
}

/*****************************************************************************
*FlatStringGroupTerm
*
*	taken (with minor modifications) from Karl Sirotkin's code 
*	by Tom Madden.
*
*****************************************************************************/

/*----------- FlatStringGroupTerm  ()------*/
CharPtr FlatStringGroupTerm 
(BiotablePtr btp, CharPtr start, ValNodePtr head, CharPtr delimit, CharPtr period, CharPtr term)
{
	CharPtr retval = NULL, p, ptr;
	Int4 lenmax, len = 0, lenstart = 0, lende = 0, lente=0, lenper=0;
	ValNodePtr an;

	if (delimit)
		lende = StringLen(delimit);

	if (term)
		lente = StringLen(term);

	lenmax = lende;
	if (lente > lende)
		lenmax = lente;

	if (period)
		lenper=StringLen(period);

	len =  lenmax + lenper;

	if (start)
		len += (lenstart = StringLen(start))  ;

	for (an = head; an; an = an -> next)
		if (an -> data.ptrvalue)
			len += lenmax + StringLen( (CharPtr) ( an -> data.ptrvalue));


/*--will be lende too long - who cares --- */

	p = retval = MemNew ((size_t)(len+1));
	if (start){
		StringCpy (p, start);
		p += lenstart;
		StringCpy(p, delimit);
		p += lende;
	}

	for (an = head; an; an = an -> next)
		if (an -> data.ptrvalue){
			StringCpy(p, (CharPtr) (an -> data.ptrvalue) );
			p += StringLen( (CharPtr) ( an -> data.ptrvalue));
			if (an -> next){
				if ( an -> next -> next){
					StringCpy(p, delimit);
					p += lende;
				}else {
					ptr = (CharPtr) an->next->data.ptrvalue;
					if ((StringCmp("et al.", ptr)) == 0 ||
						(StringCmp("et,al.", ptr)) == 0)
					{
						if (btp->format == EMBL)
						{
							StringCpy(p, term);
							p += lente;
						}
						else
						{
							StringCpy(p, " ");
							p++;
						}
					}
					else
					{
						StringCpy(p, term);
						p += lente;
					}
				}
			}
		}

	if (period){
		StringCpy(p, period);
		p += lenper;
	}

	return retval;
}	

/*****************************************************************************
*GBGetAuthNames
*
*	taken (with minor modifications) from Karl Sirotkin's code 
*	by Tom Madden.
*
*****************************************************************************/
/*****************************************************************************
*
*   GBGetAuthNames(BiotablePtr btp, AuthListPtr)
*   	Allocates a linked list of ValNode
*   	Each ->data.ptrvalue will have a string containing a formatted name
*   	Processes only AuthList of type "std"
*   		Returns NULL on any other type
*   	Caller must free returned linked list (call ValNodeFreeData())
*
*****************************************************************************/
ValNodePtr GBGetAuthNames (BiotablePtr btp, AuthListPtr ap)
{
	ValNodePtr namehead, cur, spare;
	CharPtr tmp;
	AuthorPtr authptr;
	PersonIdPtr pid;
	NameStdPtr nsp;

	if (ap == NULL)
		return NULL;

	namehead = NULL;   /* convert to strings */
	if (ap->choice != 1)
		return NULL;

	cur = NULL;
	for (spare = ap->names; spare; spare = spare->next)
	{
		cur = ValNodeNew(cur);
		if (namehead == NULL)
			namehead = cur;
		authptr = (AuthorPtr) spare->data.ptrvalue;
		pid = authptr->name;
		if (pid->choice == 2)  /* structured name */
		{
			nsp = (NameStdPtr) pid->data;
			if (nsp->names[0] != NULL)   /* last name */
			{
				tmp = MemNew((size_t)(StringLen(nsp->names[0]) + StringLen(nsp->names[4]) + StringLen(nsp->names[5]) + 3));
				cur->data.ptrvalue = tmp;
				tmp = StringMove(tmp, nsp->names[0]);
				if (nsp->names[4])
				{
					if (btp->format == EMBL)
						tmp = StringMove(tmp, " ");
					else
						tmp = StringMove(tmp, ",");
					tmp = StringMove(tmp, nsp->names[4]);
				}
				if (nsp->names[5]) /* suffix */
				{
					tmp = StringMove(tmp, " ");
					tmp = StringMove(tmp, nsp->names[5]);
				}
			}
			else if (nsp->names[3]) /* full name */
			{
				cur->data.ptrvalue = (Pointer)StringSave(nsp->names[3]);
			}
		}
		else if ((pid->choice == 3) || (pid->choice == 4)){
			cur->data.ptrvalue = (Pointer)StringSave((CharPtr)pid->data);
			if (btp->format == EMBL)
				for (tmp = (CharPtr) cur->data.ptrvalue; 
					*tmp != '\0'; tmp ++)
				{
					if (*tmp == ','){
						*tmp = ' ';
						break;
					}
				}
		}
	}
	return namehead;
}
/*****************************************************************************
*
*   GetSubmitterName (BiotablePtr btp, ValNodePtr pub)
*
*	Returns one author name for the submitter line on the flat file.
*	The pub should be a cit-sub.
*
*****************************************************************************/

static CharPtr GetSubmitterName (BiotablePtr btp, ValNodePtr pub)

{
	AuthListPtr ap;
	AuthorPtr authptr;
	CharPtr first, initials=NULL, last, middle, start, tmp, submitter_name=NULL;
	CitSubPtr cs;
	Int2 length;
	NameStdPtr nsp;
	PersonIdPtr pid;
	ValNodePtr spare;

	if (pub->choice != 2)
		return NULL;

	cs = (CitSubPtr) pub->data.ptrvalue;
	ap = cs->authors;

	if (ap && ap->choice != 1)
	{
		spare = ap->names;	/* take first name in list */
		if (spare)
		{
			submitter_name = StringSave(spare -> data.ptrvalue);
			if (btp->format == EMBL)
			{
				for (tmp=(CharPtr) submitter_name; 
					*tmp != '\0'; tmp++)
				{
					if (*tmp == ',')
					{
						*tmp = ' ';
						break;
					}
				}
			}
		}
	} 
	else
	{
		spare = ap->names;
		if (spare)
		{
			authptr = (AuthorPtr) spare->data.ptrvalue;
			pid = authptr->name;
			if (pid->choice == 2)  /* structured name */
			{
				nsp = (NameStdPtr) pid->data;
				if (nsp->names[3]) /* full name */
				{
					submitter_name = StringSave(nsp->names[3]);
				}
				else
				{
					if (nsp->names[0] != NULL)   /* last name */
						last = nsp->names[0];
					else
						last = NULL;
					if (nsp->names[1] != NULL)   /* first name */
						first = nsp->names[1];
					else
						first = NULL;
					if (nsp->names[4] != NULL)   /* initials*/
					{
					    tmp = nsp->names[4];
					    while (*tmp == ' ') 
						tmp++;     /* no white space */
					    if (first)
					    {
					       while (*tmp != '\0')
					       {
					          if (*tmp == *first)
					          {
						        tmp++;
						        if (*tmp == '.')
						   	   tmp++;
						        break;
						   }
						   else
							tmp++;
					       }
					    }
					    length = StringLen(tmp);
					    if (length > 0)
					    {
						 if (tmp[length-1] != '.')
						 {
							start = initials = MemNew((length+2)*sizeof(Char));
							initials = StringMove(initials, tmp);
							initials = StringMove(initials, ".");
							initials = start;
						 }
						 else
					   	 	initials = StringSave(tmp);
					    }
					    else
						 initials = NULL;
					}
					if (initials == NULL && 
						nsp->names[2] != NULL)   /* middle name */
					{
					     middle = nsp->names[2];
					     start = initials = MemNew(3*sizeof(Char));
					     *initials = middle[0]; initials++;
					     *initials = '.'; 
					     initials = start;
					}
					length = StringLen(last) + 
						StringLen(initials) + StringLen(first);
					if (last)
					{
						start = submitter_name = 
							MemNew((length+3)*sizeof(Char));
						if (first)
						{
					   	   start = StringMove(start, first);
						   start = StringMove(start, " ");
						}
						if (initials)
						{
						   start = StringMove(start, initials);
						   start = StringMove(start, " ");
						   initials = MemFree(initials);
						}
						start = StringMove(start, last);
					}
				}
			}
			else if ((pid->choice == 3) || (pid->choice == 4))
			{
				submitter_name = StringSave((CharPtr)pid->data);
				if (btp->format == EMBL)
					for (tmp = (CharPtr) submitter_name; 
						*tmp != '\0'; tmp ++)
					{
						if (*tmp == ','){
							*tmp = ' ';
							break;
						}
					}
			}
		}
	}
	return submitter_name;
}


/*****************************************************************************
*FlatPubTitle
*
*	taken (with minor modifications) from Karl Sirotkin's code 
*	by Tom Madden.
*
*****************************************************************************/

/*----------- FlatPubTitle ()------*/
CharPtr FlatPubTitle 
(ValNodePtr the_pub)
{
	CharPtr retval = NULL;
	CitArtPtr ca;
	CitBookPtr book;
	CitGenPtr cg;
	CharPtr str_ret, p;
	MedlineEntryPtr ml;
	CitPatPtr cp;

	switch ( the_pub -> choice){

	case 9:
		cp = (CitPatPtr) the_pub -> data.ptrvalue;
		retval = StringSave(cp -> title);
		break;
	case 11:
	book = (CitBookPtr) the_pub -> data.ptrvalue;
	if (book ->  title )
         if (book -> title -> data.ptrvalue)
            retval = StringSave(book -> title -> data.ptrvalue);
      break;

	case 5:
	case 3:
		if ( the_pub -> choice == 3){
			ml = (MedlineEntryPtr) the_pub -> data.ptrvalue;
			ca = (CitArtPtr) ml -> cit;
		}else{
			ca = (CitArtPtr) the_pub -> data.ptrvalue;
		}
		if (ca -> title )
			if (ca -> title -> data.ptrvalue)
				retval = StringSave(ca -> title -> data.ptrvalue);
		break;
	case 1: 
		cg = (CitGenPtr) the_pub -> data.ptrvalue;
		if (cg->title)
			retval = StringSave(cg->title);
		else
		{
			str_ret = NULL;
			if (cg -> cit)
				str_ret = StrStr(cg -> cit ,"Title=\"");
			if (str_ret){
				retval = StringSave(str_ret + 7);
				for (p=retval; *p; p++)
					if (*p == '"'){
						*p = '\0';
						break;
					}
			} 
			/* Finish off for unparsed direct subs. */
			/* else {
				if (StringNICmp("submitted, cg->cit, 8) == 0)
					retval = StringSave("Direct Submission");
					*/
		}
		break;
	case 2: /* submission */
		retval = StringSave("Direct Submission");
		break;
	default:
		break;

	}
		
	retval =  FlatCleanEquals(retval);
	if(retval) if (*retval)
	for (p=retval + StringLen(retval)-1 ; p> retval+2; p--){
		if (*p == ' '){
			*p = '\0';
		}else if (*p == '.'){
			Boolean remove_it = FALSE;
			if ( p > retval +5){
				if ( *(p -1 ) != '.' || * (p-2) != '.')
					remove_it = TRUE;
			}
			if (remove_it)
				*p = '\0';
			break;
		}else{
			break;
		}
	}
	return (retval);
}

/*****************************************************************************
*FlatJournal
*
*	taken (with minor modifications) from Karl Sirotkin's code 
*	by Tom Madden.
*
*Rewrite to handle weird "SUBMITTED..." stuff in cit_gen->cit (2/11/94).
*****************************************************************************/

/*----------- FlatJournal ()------*/
CharPtr FlatJournal 
(BiotablePtr btp, Int2 count, ValNodePtr the_pub, Int4 pat_seqid, Boolean PNTR submit, Boolean make_index)
{
	CharPtr retval=NULL;

	if (the_pub)
	{
		switch ( the_pub -> choice){
		case 2:	/* Cit-sub */
    			retval = format_cit_sub(btp, count, the_pub, submit, make_index);
    			break;
	
		case 9: /* Patent */
    			retval = format_patent(btp, count, the_pub, pat_seqid, make_index);
			break;
	
		case 11: /* thesis */
    			retval = format_thesis(btp, count, the_pub, make_index);
			break;
	
		case 5: /* article */
		case 3:
    			retval = format_article(btp, count, the_pub, make_index);
			break;
	
		case 1: /* Cit-gen */
    			retval = format_general(btp, count, the_pub, make_index);
			break;
	
		default:
		if (btp->error_msgs == TRUE)
			ErrPostEx(SEV_WARNING, ERR_REFERENCE_Illegalreference,
			"FlatJournal: Unimplemented pub type=%d\n", 
			(int) the_pub -> choice);
		}
	}

	if (retval){
		CharPtr hold, p , q;

		retval = FlatCleanEquals(retval);
		hold = p = MemNew(StringLen(retval)+1);

		for (q=retval; *q; q++){
			if (*q != '\n' && *q != '\r'){
				*p = *q;
			}else{
				*p = ' ';
			}
			p ++;
		}
		*p = '\0';
		MemFree(hold);
	}

	return retval;

}	/* FlatJournal */


static size_t fmt_cit_sub_1 (AffilPtr afp, size_t aflen)

{
  if (afp) {
    if (afp -> choice == 1) {
      if (afp -> affil) {
        aflen += StringLen (afp -> affil);
      }
    } else if (afp -> choice == 2) {
      aflen += 15 +
      StringLen (afp -> affil) + 
      StringLen (afp -> div) + 
      StringLen (afp -> city) + 
      StringLen (afp -> sub) + 
      StringLen (afp -> street) + 
      StringLen (afp -> country);
    }
  }
  return aflen;
}

static CharPtr fmt_cit_sub_2 (CharPtr start, CharPtr date_std, Uint1 format)

{
  CharPtr temp;

  temp = StringMove (start, "Submitted");
  temp = StringMove (temp, " (");
  temp = StringMove (temp, date_std);
  if (format == EMBL) {
    temp = StringMove (temp, ")");
  } else {
    temp = StringMove (temp, ")");
  }
  return temp;
}

/*************************************************************************
*static CharPtr format_cit_sub(BiotablePtr btp, ValNodePtr the_pub, Boolean PNTR submit, Boolean make_index)
*
*format pubs of type CitSub for GenBank and EMBL flat file.
**************************************************************************/

static CharPtr format_cit_sub (BiotablePtr btp, Int2 count, ValNodePtr the_pub, Boolean PNTR submit, Boolean make_index)

{
  AffilPtr afp;
  AuthListPtr alp;
  Boolean need_comma = FALSE, need_space = TRUE;
  Char ch;
  CitSubPtr cs;
  CharPtr affil=NULL, start, retval = NULL, temp, dup_pages = NULL;
  CharPtr authors, date_std = NULL, ptr;
  DatePtr dp;
  ImprintPtr ip;
  size_t aflen = 0;
  Uint1 format = btp->format;

  cs = (CitSubPtr) the_pub -> data.ptrvalue;

  ip = cs -> imp;
  if (ip -> date) {
    dp = ip -> date;
    date_std = FlatDateFromCreate (NULL, (NCBI_DatePtr) dp);
  }

  afp = NULL;
  if (ip -> pub) {
    afp = ip -> pub;
    aflen = 10;   /* "Submitted " */
  }

  if (afp == NULL) {
    alp = cs -> authors;
    if (alp && alp -> affil) {
      afp = alp -> affil;
    }
  }

  aflen = fmt_cit_sub_1 (afp, aflen);

  if (afp)
  {
  	if (afp->choice == 1)	/* author name is in unparsed string */
  		authors = NULL;
  	else
       		authors = GetSubmitterName (btp, the_pub);
  }
  else
       	authors = GetSubmitterName (btp, the_pub);

  retval = MemNew ((size_t) (60 + 2 + StringLen (authors) +
                   StringLen (date_std) + aflen));
  temp = fmt_cit_sub_2 (retval, date_std, format);

  if (format == EMBL) {
    temp = StringMove (temp, " to the EMBL/GenBank/DDBJ databases.");
    *temp = btp->newline;
    need_space = FALSE;
    temp++;
  }

  if (authors) {
      if (need_space)
      {
      	*temp = ' '; temp++;
      }
      temp = StringMove (temp, authors);
      authors = MemFree (authors);
      need_comma = TRUE;
  }
  *submit = TRUE;

  if (afp) {
    affil = GetAffiliation (afp);
    if (need_comma) {
      *temp = ','; temp++;
    }
    if (affil) {
      start = affil;
      if (format == EMBL && afp->choice == 1)
	  affil++;
      ptr = affil;
      ch = *affil;
      while (ch != '\0') {
        *temp = ch;
        temp++;
        affil++;
        ch = *affil;
      }
      affil = ptr;
      start = MemFree (start);
    }
  }

  if (date_std) {
    MemFree (date_std);
  }
  if (dup_pages) {
    MemFree (dup_pages);
  }

  return retval;
}	/* format_cit_sub */

/*************************************************************************
*static CharPtr format_patent(BiotablePtr btp, ValNodePtr the_pub, Int4 pat_seqid, Boolean make_index)
*
*format pubs of type CitSub for GenBank and EMBL flat file.
**************************************************************************/

static CharPtr format_patent (BiotablePtr btp, Int2 count, ValNodePtr the_pub, Int4 pat_seqid, Boolean make_index)

{
	AffilPtr afp;
	AuthListPtr ap=NULL;
	CitPatPtr cp;
	CharPtr retval = NULL, temp, pat_date = NULL;
	char buf[10];
	int patlen;
	Uint1 format=btp->format;

	cp = (CitPatPtr) the_pub -> data.ptrvalue;
	if (cp){
		patlen = 50;
		patlen += StringLen(cp -> country) ;
		if (cp -> number){
			patlen += StringLen(cp -> number);
		}else if (cp -> app_number){
			patlen += StringLen(cp -> app_number);
		}
		if ( cp -> doc_type){
			patlen += StringLen(cp -> doc_type);
		}
		ap = cp -> authors;
		if (ap){
			afp = ap -> affil;
			if (afp){
				patlen += StringLen(afp -> affil);
				if (afp -> choice == 2){
					patlen += StringLen(afp -> div);
					patlen += StringLen(afp -> city);
					patlen += StringLen(afp -> sub );
					patlen += StringLen(afp -> country);
					patlen += StringLen(afp -> street);
				}
			}
		}
		
		temp = retval = MemNew(patlen);
		if (format == EMBL)
			temp = StringMove(temp,"Patent number ");
		else
			temp = StringMove(temp,"Patent: ");
		temp = StringMove(temp,cp -> country);
		if (format != EMBL)
			temp = StringMove(temp," ");
		if (cp -> number){
			temp = StringMove(temp, cp -> number);
		}else if (cp -> app_number){
			temp = StringMove(temp,"(");
			temp = StringMove(temp, cp -> app_number);
			temp = StringMove(temp,")");
		}
		if ( cp -> doc_type) if ( * (cp -> doc_type)){
			temp = StringMove(temp,"-");
			temp = StringMove(temp, cp -> doc_type);
		}
		if (format == EMBL)
			sprintf(buf,"%s%ld%s", "/", (long) pat_seqid, ", ");
		else if (format == GENPEPT)
			sprintf(buf,"%s", " ? ");
		else
			sprintf(buf,"%s%ld ", " ", (long) pat_seqid);
		temp = StringMove(temp,buf);
		if (cp -> date_issue){
			pat_date = FlatDateFromCreate(NULL, cp -> date_issue);
		}else if (cp -> app_date){
			pat_date = FlatDateFromCreate(NULL, cp -> app_date);
		}
		if (pat_date){
			temp = StringMove(temp,pat_date);
		}
		if (format == EMBL)
			temp = StringMove(temp, ".");
		else
			temp = StringMove(temp, ";");
		if (make_index != TRUE)
			*temp = btp->newline;
		else
			*temp = ' ';
		temp++;
		ap = cp -> authors;
		if (ap){
			afp = ap -> affil;
			if (afp){
				temp = StringMove(temp,afp -> affil);
				if (afp -> choice == 2){
					if (afp -> affil)
					{
						temp = StringMove(temp,";");
						if (make_index != TRUE)
							*temp = btp->newline;
						else
							*temp = ' ';
						temp++;
					}
					if (afp -> street){
						temp = StringMove(temp,afp -> street);
						temp = StringMove(temp,";");
						if (make_index != TRUE)
							*temp = btp->newline;
						else
							*temp = ' ';
						temp++;
					}
					if (afp -> div){
						temp = StringMove(temp,afp -> div);
						temp = StringMove(temp,";");
						if (make_index != TRUE)
							*temp = btp->newline;
						else
							*temp = ' ';
						temp++;
					}
					if (afp -> city){
						temp = StringMove(temp,afp -> city);
						temp = StringMove(temp,", ");
					}
					if (afp -> sub){
						temp = StringMove(temp,afp -> sub );
						temp = StringMove(temp,";");
						if (make_index != TRUE)
							*temp = btp->newline;
						else
							*temp = ' ';
						temp++;
					}
					if (afp ->country){
						temp = StringMove(temp,afp -> country);
						temp = StringMove(temp,";");
					}
				}
			}
		}
		
		MemFree(pat_date);
	}

	return retval;

}	/* format_patent */

/*************************************************************************
*static CharPtr format_thesis(BiotablePtr btp, ValNodePtr the_pub, Boolean make_index)
*
*format pubs of type CitSub for GenBank and EMBL flat file.
**************************************************************************/

static CharPtr format_thesis (BiotablePtr btp, Int2 count, ValNodePtr the_pub, Boolean make_index)

{
	AffilPtr afp;
	AuthListPtr ap = NULL;
	char year[5];
	CharPtr book_authors=NULL, book_tit=NULL;
	CitBookPtr cb;
	CharPtr retval = NULL, temp;
	DatePtr dp;
	ImprintPtr ip;
	int aflen=0;
	ValNodePtr namehead;

	year[0] = '\0';

	cb = (CitBookPtr) the_pub -> data.ptrvalue;
	if ( cb -> othertype == 2 && cb -> let_type == 3){ /* match thesis */
		CharPtr p;
		ip = cb -> imp;
		dp = ip -> date;
		if ( dp -> data[0] == 1){
			sprintf(year,"%ld",(long) ( 1900+dp -> data[1]));
		}else{
			StringNCpy( (CharPtr) year, (CharPtr) dp -> str, (size_t) 4);
			year[4] = '\0';
		}
		ap = cb -> authors;
		if (ap->choice != 1)   /* just strings */
			namehead = ap->names;
		else                  /* structured names */
			namehead = GBGetAuthNames(btp, ap);
		book_authors = FlatStringGroupTerm(btp, NULL, namehead, ", ", NULL, 
		" and ");
		if (ap->choice == 1)    /* std names */
			ValNodeFreeData(namehead);
			
		book_tit = StringSave(cb -> title -> data.ptrvalue);
		if ( ip -> pub){
			afp = ip -> pub;
			aflen = 7;

			if (afp -> choice == 1){
				if (afp -> affil){
					aflen += StringLen(afp -> affil) + 6;
				}

			}else{
				aflen = 22;
			}
		}
		if (ip->prepub == 2)   /* In press */
			aflen += 10;

		temp = retval = MemNew( (size_t) (60+
			 StringLen( year) + aflen) );
/*--make book title all caps */

		for ( p = book_tit; *p; p++){
			*p = TO_UPPER(*p);
		}
		temp = StringMove(temp,"Thesis");
		temp = StringMove(temp," (");
		temp = StringMove(temp,year);
		temp = StringMove(temp,")");

		if ( ip -> pub){
			afp = ip -> pub;

			if ( afp -> choice == 1){
				if (afp -> affil){
					if (StringLen( afp -> affil) > 7){
						temp = StringMove(temp, " ");
						temp = StringMove(temp, afp -> affil);
						if (ip->prepub == 2)   /* In press */
							temp = StringMove(temp, ", In press");
					}
				}
			}

		}
	}
	if (book_authors)
		MemFree(book_authors);
	if (book_tit)
		MemFree(book_tit);

	return retval;

}	/* format_thesis */

/*************************************************************************
*static CharPtr format_article(BiotablePtr btp, ValNodePtr the_pub, Boolean make_index)
*
**************************************************************************/

static CharPtr format_article (BiotablePtr btp, Int2 count, ValNodePtr the_pub, Boolean make_index)

{
	CharPtr retval=NULL;
	CitArtPtr ca;
	MedlineEntryPtr ml;

	if ( the_pub -> choice == 3){
		ml = (MedlineEntryPtr) the_pub -> data.ptrvalue;
		ca = (CitArtPtr) ml -> cit;
	}else{
		ca = (CitArtPtr) the_pub -> data.ptrvalue;
	}


	if ( ca -> from != 1){
		retval = format_bookarticle(btp, count, the_pub, make_index);
	}else{
		retval = format_jourarticle(btp, count, the_pub, make_index);
	}

	return retval;

}	/* format_article */


/***********************************************************************
*static CharPtr format_bookarticle(BiotablePtr btp, CitBookPtr book)
*
*	format a book article
*	The caller must deallocate the space allocated.
**********************************************************************/

static CharPtr format_bookarticle(BiotablePtr btp, Int2 count, ValNodePtr the_pub, Boolean make_index)

{
	AffilPtr afp;
	AuthListPtr ap = NULL;
	Boolean first_std=TRUE, found_paran=FALSE;
	CharPtr temp, dup_pages=NULL, retval=NULL, volume=NULL;
	CharPtr issue=NULL, part_sup=NULL, part_supi=NULL;
	CharPtr p;
	CharPtr book_authors=NULL, book_tit=NULL;
	Char year[5];
	CitArtPtr ca;
	CitBookPtr book;
	DatePtr dp;
	ImprintPtr ip;
	Int2 aflen=0, length;
	Int2 fix_p_ret;
	MedlineEntryPtr ml;
	ValNodePtr namehead;
	size_t len_page = 0;

	if ( the_pub -> choice == 3){
		ml = (MedlineEntryPtr) the_pub -> data.ptrvalue;
		ca = (CitArtPtr) ml -> cit;
	}else{
		ca = (CitArtPtr) the_pub -> data.ptrvalue;
	}
	book = (CitBookPtr) ca->fromptr;
	ap = book -> authors;
	if (ap->choice != 1)   /* just strings */
		namehead = ap->names;
	else                  /* structured names */
			namehead = GBGetAuthNames(btp, ap);
	book_authors = FlatStringGroupTerm(btp, NULL, namehead, ", ", NULL, " and ");
				
	book_tit = StringSave(book -> title -> data.ptrvalue);
	ip = book -> imp;
	if (ip -> pages){
		len_page = StringLen( ip -> pages);
		dup_pages = MemNew ( 2*len_page );
		fix_p_ret = fix_pages(dup_pages, ip ->pages);
		if (fix_p_ret == -1 && btp->error_msgs == TRUE)
		{
			PostARefErrMessage (btp->bsp[count], NULL, the_pub, 1, dup_pages);
		}
	}else if (! ip->prepub && btp->error_msgs == TRUE) {
		PostARefErrMessage (btp->bsp[count], NULL, the_pub, 2, NULL);
	}
	dp = ip -> date;
	year[0] = '\0';
	if ( dp -> data[0] == 1){
		sprintf(year,"%ld",(long) ( 1900+dp -> data[1]));
	}else{
		StringNCpy( (CharPtr) year, (CharPtr) dp -> str, (size_t) 4);
		year[4] = '\0';
	}
	
	if ( ip -> pub){
		afp = ip -> pub;
		aflen = StringLen(afp -> affil)+ 5;
		if ( afp -> choice == 2){
			aflen += 3 + StringLen(afp -> div);
			aflen += 3 + StringLen(afp -> street);
			aflen += 3 + StringLen(afp -> city);
			aflen += 3 + StringLen(afp -> sub);
			aflen += 3 + StringLen(afp -> country);
		}
	}else{
		aflen = 22;
	}
	if (ip->volume)
	{
		volume = StringSave(ip->volume);
	}
	if (btp->format != EMBL) /* Don't do this for EMBL! */
	{
	   if (ip->part_sup)
		part_sup = StringSave(ip->part_sup);
	   if (ip->issue)
	   {
		issue = StripParanthesis(ip->issue, &found_paran);
	        if (btp->error_msgs == TRUE && found_paran == TRUE) 
		    PostARefErrMessage (btp->bsp[count], NULL, the_pub, 5, NULL);
	   }
	   if (ip->part_supi)
	   {
		part_supi = StripParanthesis(ip->part_supi, &found_paran);
	        if (btp->error_msgs == TRUE && found_paran == TRUE) 
		    PostARefErrMessage (btp->bsp[count], NULL, the_pub, 5, NULL);
	   }
	}

	length = 80;
	length += StringLen (book_tit)+ StringLen(book_authors);
	length += StringLen(volume);
	length += StringLen(issue);
	length += StringLen(part_sup);
	length += StringLen(part_supi);
	length += 2*len_page;
	length += StringLen( year) + aflen;

	temp = retval = MemNew( (size_t) length);

/*--mak book ttle all caps */
	for ( p = book_tit; *p; p++){
		*p = TO_UPPER(*p);
	}

	temp = StringMove(temp,"(in) ");
	if (make_index != TRUE)
	{
		temp = StringMove(temp,book_authors);
		if (namehead != NULL) {
			temp = StringMove(temp,namehead -> next? " (Eds.);":" (Ed.);");
		} 
		*temp = btp->newline;
		temp++;
	}

	if (ap->choice == 1)    /* std names */
		ValNodeFreeData(namehead);

	temp = StringMove(temp,book_tit);
	if (volume && StringCmp(volume, "0") != 0)
	{
		temp = StringMove(temp,", ");
		temp = StringMove(temp, "Vol. ");
		temp = StringMove(temp, volume);
		temp = doSup(temp, issue, part_sup, part_supi);
	}
	if (ip->pages != NULL)
	{
		temp = StringMove(temp,": ");
		temp = StringMove(temp,dup_pages);
	}
	if (book_tit)
	{
		*temp = ';'; 
		temp++;
	}
	if ( ip -> pub){
		afp = ip -> pub;
		if (make_index != TRUE)
			*temp = btp->newline;
		else
			*temp = ' ';
		temp++;
		temp = makeaffil(afp, temp, &first_std);
	}
	if (! first_std)
		temp = StringMove(temp," (");
	else
		temp = StringMove(temp,"(");
	temp = StringMove(temp,year);
	temp = StringMove(temp,")");
	if (ip->prepub == 2 && btp->format != EMBL)
		temp = StringMove(temp, " In press");

	if (volume)
		volume = MemFree(volume);
	if (book_authors)
		MemFree(book_authors);
	if (book_tit)
		MemFree(book_tit);
	if (issue)
		issue = MemFree(issue);
	if (part_sup)
		part_sup = MemFree(part_sup);
	if (part_supi)
		part_supi = MemFree(part_supi);
	if (dup_pages)
		MemFree(dup_pages);

	return retval;
}

/***********************************************************************
*static CharPtr format_jourarticle(BiotablePtr btp, CitBookPtr book)
*
*	format a book article
*	The caller must deallocate the space allocated.
**********************************************************************/

static CharPtr format_jourarticle(BiotablePtr btp, Int2 count, ValNodePtr the_pub, Boolean make_index)


{
	Boolean do_unpub=FALSE, found_paran=FALSE;
	char year[5];
	CharPtr temp, dup_pages=NULL, retval=NULL, volume=NULL;
	CharPtr issue=NULL, part_sup=NULL, part_supi=NULL;
	CitArtPtr ca;
	CitJourPtr jp;
	DatePtr dp;
	ImprintPtr ip;
	Int2 fix_p_ret, length, vol_len=0;
	MedlineEntryPtr ml;
	size_t len_page = 0;
	Uint1 format=btp->format;

	if ( the_pub -> choice == 3){
		ml = (MedlineEntryPtr) the_pub -> data.ptrvalue;
		ca = (CitArtPtr) ml -> cit;
	}else{
		ca = (CitArtPtr) the_pub -> data.ptrvalue;
	}

	jp = (CitJourPtr) ca -> fromptr;
	ip = jp -> imp;
	dp = ip -> date;
	year[0] = '\0';
	if ( dp -> data[0] == 1){
		if (dp->data[1] != '\0') {
			sprintf(year,"%ld",(long) ( 1900+dp -> data[1]));
		}
	}else{
		StringNCpy( (CharPtr) year, (CharPtr) dp -> str, (size_t) 4);
		year[4] = '\0';
	}
	if (ip -> pages){
		len_page = StringLen( ip -> pages);
		dup_pages = MemNew ( 2*len_page );
		fix_p_ret = fix_pages(dup_pages, ip ->pages);
		if (fix_p_ret == -1 && btp->error_msgs == TRUE)
		{
			PostARefErrMessage (btp->bsp[count], NULL, the_pub, 1, dup_pages);
		}
	}else if (! ip->prepub && btp->error_msgs == TRUE) {
		PostARefErrMessage (btp->bsp[count], NULL, the_pub, 2, NULL);
	}
	if (ip->prepub != 1 && ip->prepub != 255)
	{
		if (ip->volume)
		{
		    volume = StringSave(ip->volume);
		}
		if (format != EMBL) /* Don't do this for EMBL! */
		{
		   if (ip->part_sup)
			part_sup = StringSave(ip->part_sup);
		   if (ip->issue)
		   {
			issue = StripParanthesis(ip->issue, &found_paran);
		        if (btp->error_msgs == TRUE && found_paran == TRUE) 
			    PostARefErrMessage (btp->bsp[count], NULL, the_pub, 5, NULL);
		   }
		   if (ip->part_supi)
		   {
			part_supi = StripParanthesis(ip->part_supi, &found_paran);
		        if (btp->error_msgs == TRUE && found_paran == TRUE) 
			    PostARefErrMessage (btp->bsp[count], NULL, the_pub, 5, NULL);
		   }
		}

		length = 25;
		length += StringLen ((CharPtr) (jp -> title -> data.ptrvalue));
		length += StringLen(volume);
		length += StringLen(issue);
		length += StringLen(part_sup);
		length += StringLen(part_supi);
		length += 2*len_page + StringLen( year);


		/* Does the journal have a title? */
		if (jp -> title -> data.ptrvalue != NULL &&
			StringLen(jp -> title -> data.ptrvalue) > 2)
		{
		   temp = retval = MemNew( (size_t) length);
		   temp = StringMove(temp, (CharPtr) (jp -> title -> data.ptrvalue));
		   if ((volume != NULL) || (ip->pages != NULL))
		   {
			temp = StringMove(temp," ");
			if (volume != NULL)
				temp = StringMove(temp, volume);
			temp = doSup(temp, issue, part_sup, part_supi);
			if (ip->pages != NULL)
				if (format == EMBL)
					temp = StringMove(temp,":");
				else
					temp = StringMove(temp,", ");
		   }
		   if (ip->pages != NULL)
			temp = StringMove(temp, dup_pages);
		   else if (ip->prepub == 2 && format == EMBL)
			temp = StringMove(temp, " 0:0-0");
		   else if (format == EMBL && ip->pages == NULL 
			&& volume == NULL)
			temp = StringMove(temp, " 0:0-0");
		   if (format == EMBL)
			temp = StringMove(temp," (");
		   else
			temp = StringMove(temp," (");
		   temp = StringMove(temp, year);
		   temp = StringMove(temp,")");
		   if (ip->prepub == 2 && format != EMBL)
			temp = StringMove(temp, " In press");
		}
		else
		{
		   length = 2;
		   temp = retval = MemNew( (size_t) length);
		   temp = StringMove(temp,".");
		}
		
	}
	else
	{
                      temp = retval = MemNew(21*sizeof(Char));
                      temp = StringMove(temp, "Unpublished ");
                      if (year[0] != NULLB)
                      {
                              temp = StringMove(temp,"(");
                              temp = StringMove(temp, year);
                              temp = StringMove(temp,")");
                      }
	}

	if (volume)
		volume = MemFree(volume);
	if (issue)
		issue = MemFree(issue);
	if (part_sup)
		part_sup = MemFree(part_sup);
	if (part_supi)
		part_supi = MemFree(part_supi);
	if (dup_pages)
		MemFree(dup_pages);

	return retval;
}	/* format_jourarticle */





/************************************************************************
*static CharPtr makeaffil(AffilPtr afp, CharPtr temp, Boolean PNTR first_std)
*
*	formats the affiliation into temp, which should already be 
*	allocated to hold this stuff.
***********************************************************************/

static CharPtr makeaffil(AffilPtr afp, CharPtr temp, Boolean PNTR first_std)

{
	*first_std = TRUE;

	if (afp -> affil && StringLen(afp->affil) > 0)
	{
		temp = StringMove(temp, afp -> affil);
		*first_std = FALSE;
	}
	if ( afp -> choice == 2){
		if (afp -> div){
			if ( ! *first_std)
				temp = StringMove(temp,", ");
			temp =  StringMove(temp,afp -> div);
			*first_std = FALSE;
		}
		if (afp -> street){
			if ( ! *first_std)
				temp = StringMove(temp,", ");
			temp =  StringMove(temp,afp -> street);
			*first_std = FALSE;
		}
		if (afp -> city){
			if ( ! *first_std)
				temp = StringMove(temp,", ");
			temp =  StringMove(temp,afp -> city);
			*first_std = FALSE;
		}
		if (afp -> sub){
			if ( ! *first_std)
				temp = StringMove(temp,", ");
			temp =  StringMove(temp,afp -> sub);
			*first_std = FALSE;
		}
		if (afp -> country){
			if ( ! *first_std)
				temp = StringMove(temp,", ");
			temp =  StringMove(temp,afp -> country);
			*first_std = FALSE;
		}
	}
	return temp;
}

/************************************************************************
*static CharPtr doSup(CharPtr temp, CharPtr issue, CharPtr part_sup, CharPtr part_supi)
*
*	copies the characters onto temp.
*
**************************************************************************/
static CharPtr doSup(CharPtr temp, CharPtr issue, CharPtr part_sup, CharPtr part_supi)

{
	if (part_sup != NULL) {
		temp = StringMove(temp, part_sup);
	}
	if (issue || part_supi) {
		*temp = ' ';
		temp++;
		*temp = '(';
		temp++;
		if (issue) {
			temp = StringMove(temp, issue);
		}
		if (part_supi) {
			temp = StringMove(temp, part_supi);
		}
		*temp = ')';
		temp++;
	}
	return temp;
}

/*************************************************************************
*static CharPtr format_general(BiotablePtr btp, ValNodePtr the_pub, Boolean make_index)
*
*format pubs of type CitSub for GenBank and EMBL flat file.
**************************************************************************/

static CharPtr fmt_gen_1 (BiotablePtr btp, Int2 count, ValNodePtr the_pub, CharPtr unp, CharPtr affil, CitGenPtr cg,
                          Boolean used_cit, Boolean found_journal,
                          CharPtr year, CharPtr unp_long, CharPtr p,
                          size_t len_page)

{
  CharPtr  retval, volume=NULL;
  Int2 vol_len;

  retval = NULL;
  /* use cit or journal */
  if (unp) {
    if (cg->volume)
    {
	volume = StringSave(cg->volume);
    }
    retval = MemNew ((size_t) (9+
      StringLen (unp) +
      StringLen (affil) +
      StringLen(volume) +
      2*len_page + StringLen( year)) );
  } else if (cg -> cit && ! used_cit && found_journal ) {
    retval = MemNew ((size_t) (9 +
      11 + StringLen(volume) +
      1+ StringLen (cg -> cit) +
      2*len_page + StringLen( year) + StringLen (unp_long)) 
      + StringLen(p));
  } else { /* tack extra unpub entry to end of year */
    retval = MemNew ((size_t) (9+
      11 + StringLen(volume) + StringLen(p) +
      2*len_page + StringLen( year) + StringLen (unp_long)) );
  }

  if (volume)
	volume = MemFree(volume);
  return retval;
}

static CharPtr fmt_gen_2 (BiotablePtr btp, Int2 count, ValNodePtr the_pub, 
		CharPtr start, CharPtr ptr, CharPtr unp, CharPtr unp_long, 
		CitGenPtr cg, Uint1 format, CharPtr dup_pages, CharPtr year)

{
  CharPtr  temp, volume=NULL;
  Int2 vol_len;

	temp = start;
	if (ptr) {
		temp = StringMove (temp, ptr);
	} else if (unp_long) {
		if (!ASN2FF_SHOW_ALL_PUBS)
			temp = StringMove (temp, "Unpublished");
		else 
			temp = StringMove (temp, unp_long);
	} else if (unp) { 
		if (!ASN2FF_SHOW_ALL_PUBS)
			temp = StringMove (temp, "Unpublished");
		else 
			temp = StringMove (temp, unp);
	}
	if (cg->volume)
	{
	   volume = StringSave(cg->volume);
	}

  	if (volume) 
  	{
  		temp = StringMove (temp," ");
  		temp = StringMove (temp, volume);
  	}

  	if (dup_pages != NULL) 
  	{
  		if (format == EMBL) 
  			temp = StringMove (temp,":");
  		else 
  			temp = StringMove (temp,", ");
  		temp = StringMove (temp, dup_pages);
  	}

  	if (year[0] != '\0') 
  	{
  		temp = StringMove (temp," (");
  		temp = StringMove (temp, year);
  		temp = StringMove (temp,")");
  	}

  if (volume)
	volume = MemFree(volume);
  return temp;
}

static CharPtr format_general (BiotablePtr btp, Int2 count, ValNodePtr the_pub, Boolean make_index)

{
  AuthListPtr ap = NULL;
  Char ch;
  Boolean used_cit=FALSE, found_journal=FALSE;
  Char year[5];
  CharPtr affil=NULL, temp, dup_pages=NULL, retval=NULL;
  CharPtr str_ret, ptr, unp=NULL, unp_long=NULL, misc_unp=NULL;
  CitGenPtr cg;
  DatePtr dp;
  Int2 fix_p_ret;
  size_t len_page = 0;
  Uint1 format=btp->format;

  cg = (CitGenPtr) the_pub -> data.ptrvalue;
  year[0] = '\0';
  if (cg->date) {
    dp = cg -> date;
    if (dp -> data[0] == 1) {
      sprintf (year,"%ld", (long) (1900 + dp -> data[1]));
    } else {
      StringNCpy ((CharPtr) year, (CharPtr) dp -> str, (size_t) 4);
      year[4] = '\0';
    }
  }
  if (cg->pages) {
    len_page = StringLen (cg -> pages);
    dup_pages = MemNew (2 * len_page);
    fix_p_ret = fix_pages (dup_pages, cg ->pages);
    if (fix_p_ret == -1) {
      if (btp->error_msgs == TRUE)
	PostARefErrMessage (btp->bsp[count], NULL, the_pub, 1, dup_pages);
    }
  } else if (StringNICmp ("unpublished", cg->cit, 11) != 0 &&
             StringNICmp ("submitted", cg->cit, 8) != 0 &&
             StringNICmp ("in press", cg->cit, 8) != 0 &&
             StringNICmp ("to be published", cg->cit, 15) != 0) {
    if (btp->error_msgs == TRUE)
	PostARefErrMessage (btp->bsp[count], NULL, the_pub, 2, NULL);
  }
  if (cg->journal) {
    ptr = (CharPtr) (cg->journal->data.ptrvalue);
    found_journal = TRUE;
  } else {
    ptr = NULL;
  }

  str_ret = NULL;
  if (cg -> cit) {
    ap=cg->authors;
    str_ret = StrStr (cg -> cit ,"Journal=\"");
    if (str_ret) {
      retval = StringSave (str_ret + 9);
      found_journal =used_cit = TRUE;
      for (ptr=retval; *ptr; ptr++) {
        if (*ptr == '=' || *ptr == '\"') {
          *ptr = '\0';
        }
      }
    } else if (ptr == NULL) {   /* no journal field, use cit */
      used_cit = TRUE;
      if (strncmp ("Unpublished ", cg->cit, (unsigned) 12) == 0) {
        if (strlen (cg->cit) > (size_t) 12) { /* get rest of Unpub entry */
          unp_long = cg->cit;
          unp_long += 11;
        } else {
        unp = cg->cit;
        if ((ap=cg->authors) != NULL) {
          affil = GetAffiliation (ap->affil);
        }
      }
    } 
    else if (strncmp ("Unpublished; ", cg->cit, (unsigned) 13) == 0) 
    {
        if (strlen (cg->cit) > (size_t) 13) { /* get rest of Unpub entry */
          unp_long = cg->cit;
          unp_long += 12;
        } else {
        unp = cg->cit;
        if ((ap=cg->authors) != NULL) {
          affil = GetAffiliation (ap->affil);
        }
      }
    }
    else if (StringNICmp ("submitted", cg->cit, 8) != 0 ||
             StringNICmp ("in press", cg->cit, 8) != 0 ||
             StringNICmp ("to be published", cg->cit, 15) != 0)
	{
		if (ASN2FF_SHOW_ALL_PUBS) {
			misc_unp = StringSave(cg->cit);
		} else {
			misc_unp = StringSave("Unpublished");
		}
		unp = misc_unp;
        	if ((ap=cg->authors) != NULL) 
         	   affil = GetAffiliation (ap->affil);
	}
     }
  }

  if ((retval == NULL) && ( (ptr != NULL) || unp != NULL || unp_long))   {
    /* use cit or journal */
    retval = fmt_gen_1 (btp, count, the_pub, unp, affil, cg, used_cit, found_journal,
                        year, unp_long, ptr, len_page);

    temp = fmt_gen_2 (btp, count, the_pub, retval, ptr, unp, unp_long,
                      cg, format, dup_pages, year);
  }
  if (unp_long) {
    temp = StringMove (temp, unp_long);
  } else if (unp) {
    if (affil) {
      ptr = affil;
      ch = *affil;
      while (ch != '\0') {
        *temp = ch;
        temp++;
        affil++;
        ch = *affil;
      }
      affil = ptr;
      affil = MemFree (affil);
    }
  }
  if (cg -> cit && ! used_cit && found_journal) {
    temp = StringMove (temp," ");
    temp = StringMove (temp, cg -> cit);
  }

  if (dup_pages) {
    MemFree (dup_pages);
  }

  if (misc_unp != NULL)
	misc_unp = MemFree(misc_unp);

  return retval;

}  /* format_general */


/*************************************************************************
*CharPtr StripParanthesis(CharPtr ptr_in, Boolean PNTR paranthesis)
*
*	This function strips spaces and '(' from the front of
*	ptr_in, copies the rest of the string, up to a possible
*	')'.
*
*	The caller is responsible for deallocating "retval".
*	do not strip spaces!   add by Tatiana 06.01.95
************************************************************************/

static CharPtr 
StripParanthesis(CharPtr ptr_in, Boolean PNTR paranthesis)
{
	CharPtr temp, retval;
	Int2 length;

	*paranthesis=FALSE;

	while (*ptr_in == '(' )
	{
		if (*ptr_in == '(')
			*paranthesis = TRUE;
		ptr_in++;
	}

	if (*ptr_in == NULLB)
		retval = NULL;
	else
	{
		length = StringLen(ptr_in);
		retval = temp = MemNew((length+1)*sizeof(Char));
		while (*ptr_in != ')'  && *ptr_in != NULLB)
		{
			*temp = *ptr_in;
			temp++; *ptr_in++;
		}
		if (*ptr_in == ')')
			*paranthesis = TRUE;
		temp = '\0';
	}
	return retval;
}

#define MAX_PAGE_DIGITS 12

/*************************************************************************
*
*	fix_pages (CharPtr out_pages, CharPtr in_pages)
*
*	in_pages: input page numbering. 
*	out_pages: output page numbering; "out_pages" should already
*	have allocated space when fix_pages is called (twice as many 
*	characters as "in_pages").
*
*	medline type page numbering is expanded (e.g., 125-35 -> 125-135,
*	F124-34 -> F124-F134, 12a-c -> 12a-12c).
*	If only one page is given, this is output without a dash.
*	Expanded numbering is validated to ensure that the
*	first number is smaller than or equal to the second and
*	that the first letter is less than or identical to the second
*	(i.e., a < c).  If the input is all letters (i.e., roman numerals)
*	this is not validated.
*
*	Return values:
*	 0 : valid page numbering.
*	-1 : invalid page numbering.
************************************************************************/
Int2 fix_pages (CharPtr out_pages, CharPtr in_pages)

{
	Boolean dash=TRUE, first_alpha;
	Char firstbegin[MAX_PAGE_DIGITS];
	Char secondbegin[MAX_PAGE_DIGITS];
	Char firstend[MAX_PAGE_DIGITS];
	Char secondend[MAX_PAGE_DIGITS];
	Char temp[MAX_PAGE_DIGITS];
	CharPtr alphabegin, numbegin, alphaend, numend, ptr, in=in_pages;
	Int2 diff, index, retval=0;
	Int2 length_nb, length_ab, length_ne, length_ae; 
	Int4 num1=0, num2=0;

	while (*in != '\0')
	{			/* Check for digits in input*/
		if (IS_DIGIT(*in))
			break;
		in++;
	}

	if (*in == '\0')
	{		/* if all letters (i.e. roman numerals), put out. */
		out_pages = StringCpy(out_pages, in_pages);
		return retval;
	}

	in = in_pages;
	if (IS_DIGIT(*in))
	{			/* Do digits come first? */
		first_alpha = FALSE;
		index=0;
		while (IS_DIGIT(*in) || *in == ' ')
		{
			firstbegin[index] = *in;
			if (*in != ' ')
				index++;
			in++;
			if (*in == '-')
				break;

		}
		firstbegin[index] = '\0';
		index=0;
		if (*in != '-')
		{		/* After digits look for letters. */
			while (IS_ALPHA(*in)  || *in == ' ')
			{
				secondbegin[index] = *in;
				index++;
				in++;
				if (*in == '-')
					break;
			}
		}
		secondbegin[index] = '\0';
		if (*in == '-')		/* if dash is not present, note */
			in++;
		else
			dash=FALSE;
		index=0;
		while (IS_DIGIT(*in) || *in == ' ')
		{			/* Look for digits.	*/
			firstend[index] = *in;
			if (*in != ' ')
				index++;
			in++;
		}
		firstend[index] = '\0';
		index=0;
		if (*in != '\0')
		{			/* Look for letters again. */
			while (IS_ALPHA(*in)  || *in == ' ')
			{
				secondend[index] = *in;
				index++;
				in++;
			}
		}
		secondend[index] = '\0';
	}
	else
	{			/* Do letters come first? */
		first_alpha = TRUE;
		index=0;
		while (IS_ALPHA(*in) || *in == ' ')
		{
			firstbegin[index] = *in;
			index++;
			in++;
			if (*in == '-')
				break;
		}
		firstbegin[index] = '\0';
		index=0;
		if (*in != '-')
		{		/* After letters look for digits. 	*/
			while (IS_DIGIT(*in)  || *in == ' ')
			{
				secondbegin[index] = *in;
				if (*in != ' ')
					index++;
				in++;
				if (*in == '-')
					break;
			}
		}
		secondbegin[index] = '\0';
		if (*in == '-')		/* Note if dash is missing. */
			in++;
		else
			dash=FALSE;
		index=0;
		while (IS_ALPHA(*in) || *in == ' ')
		{		/* Look for letters again. */
			firstend[index] = *in;
			index++;
			in++;
		}
		firstend[index] = '\0';
		index=0;
		if (*in != '\0')
		{		/* Any digits here? */
			while (IS_DIGIT(*in)  || *in == ' ')
			{
				secondend[index] = *in;
				if (*in != ' ')
					index++;
				in++;
			}
		}
		secondend[index] = '\0';
	}

	if (first_alpha)
	{
		alphabegin = firstbegin;
		numbegin = secondbegin;
		alphaend = firstend;
		numend = secondend;
	}
	else
	{
		numbegin = firstbegin;
		alphabegin = secondbegin;
		numend = firstend;
		alphaend = secondend;
	}

	length_nb = StringLen(numbegin);
	length_ab = StringLen(alphabegin);
	length_ne = StringLen(numend);
	length_ae = StringLen(alphaend);

	/* If no dash, but second letters or numbers present, reject. */
	if (dash == FALSE)
	{
		if (length_ne != 0 || length_ae != 0)
			retval = -1;
	}
	/* Check for situations like "AAA-123" or "222-ABC". */
	if (dash == TRUE)
	{
		if (length_ne == 0 && length_ab == 0)
			retval = -1;
		else if (length_ae == 0 && length_nb == 0)
			retval = -1;
	}

	/* The following expands "F502-512" into "F502-F512" and
	checks, for entries like "12a-12c" that a > c.  "12aa-12ab",
	"125G-137A", "125-G137" would be rejected. */
	if (retval == 0)
	{
		if (length_ab > 0)
		{
			if (length_ae > 0) 	
			{ 
				if (StringCmp(alphabegin, alphaend) != 0)
				{
					if (length_ab != 1 || length_ae != 1)
						retval = -1;
					else if (*alphabegin > *alphaend)
						retval = -1;
				}
			}
			else
			{
				alphaend = alphabegin;
				length_ae = length_ab;
			}
		} 
		else if (length_ae > 0) 
			retval = -1;
	}

/* The following expands "125-37" into "125-137".	*/
	if (retval == 0)
	{
		if (length_nb > 0)
		{
			if (length_ne > 0)
			{
				diff = length_nb - length_ne;
				if (diff > 0)
				{
					index=0;
					while (numend[index] != '\0')
					{
						temp[index+diff] = numend[index];
						index++;
					}
					temp[index+diff] = numend[index];
					for (index=0; index<diff; index++)
						temp[index] = numbegin[index];
					index=0;
					while (temp[index] != '\0')
					{
						numend[index] = temp[index];
						index++;
					}
					numend[index] = temp[index];
				}
			}
			else
			{
				numend = numbegin;
				length_ne = length_nb;
			}
		
		}
		else if (length_ne > 0)
			retval = -1;
	/* Check that the first number is <= the second (expanded) number. */
		if (retval == 0)
		{
			sscanf(numbegin, "%d", &num1);
			sscanf(	numend, "%d", &num2);
			if (num2 < num1)
				retval = -1;
		}
	}

	if (retval == -1)
	{
		out_pages = StringCpy(out_pages, in_pages);
	}
	else
	{
		ptr = out_pages;
	/* Place expanded and validated page numbers into "out_pages". */
		if (first_alpha)
		{
			while (*alphabegin != '\0')
			{
				*ptr = *alphabegin;
				alphabegin++;
				ptr++;
			}
			while (*numbegin != '\0')
			{
				*ptr = *numbegin;
				numbegin++;
				ptr++;
			}
			if (dash == TRUE)
			{
				*ptr = '-';
				ptr++;
				while (*alphaend != '\0')
				{
					*ptr = *alphaend;
					alphaend++;
					ptr++;
				}
				while (*numend != '\0')
				{
					*ptr = *numend;
					numend++;
					ptr++;
				}
			}
			*ptr = '\0';
		}
		else 
		{
			while (*numbegin != '\0')
			{
				*ptr = *numbegin;
				numbegin++;
				ptr++;
			}
			while (*alphabegin != '\0')
			{
				*ptr = *alphabegin;
				alphabegin++;
				ptr++;
			}
			if (dash == TRUE)
			{
				*ptr = '-';
				ptr++;
				while (*numend != '\0')
				{
					*ptr = *numend;
					numend++;
					ptr++;
				}
				while (*alphaend != '\0')
				{
					*ptr = *alphaend;
					alphaend++;
					ptr++;
				}
			}
			*ptr = '\0';
		}
	}
	return retval;
}

/*****************************************************************************
*FlatDateFromCreate
*
*	taken (with minor modifications) from Karl Sirotkin's code 
*	by Tom Madden.
*
*****************************************************************************/

CharPtr
FlatDateFromCreate (CharPtr default_date, NCBI_DatePtr flat_date)
{
	CharPtr retval = NULL;
	char month[4], year[5]  , day[3];
	CharPtr daypt = & day[0];
	extern char * NCBI_months[12];
	char result[14];
	char localbuf[14];

	if ( flat_date -> data[0] == 0){
/*---string---*/
		if (StringICmp(flat_date -> str,"Not given") != 0){
			retval = StringSave(flat_date -> str);
		}
	}else{
/*---standard---*/
		if (! default_date  || 
				(flat_date -> data[1] &&flat_date -> data[2]
					&& flat_date -> data[3])){
			if (flat_date -> data[1]){
				sprintf(year, "%4ld", (long) (flat_date -> data[1] + 1900));
			}else{
				sprintf(year, "????");
			}
			if (flat_date -> data[3]){
				if (flat_date -> data[3] <= 9){
					*daypt = '0';
					daypt ++;
				}
				sprintf(localbuf, "%ld", (long) (flat_date -> data[3] ));
			}else{
				sprintf(localbuf, "??");
			}
			StringCpy (daypt, localbuf);
			if ( flat_date -> data[2] ){
				StringCpy(month, NCBI_months[flat_date -> data[2] -1 ]);
				month[1] = TO_UPPER(month[1]);
				month[2] = TO_UPPER(month[2]);
			}else{
				sprintf(month, "??");
			}
			sprintf(result,"%s-%s-%s",day,month, year);
			retval = StringSave(result);
		}
	}
	return retval;
}

/*****************************************************************************
*FlatIgnoreThisPatentPub
*
*	taken (with minor modifications) from Karl Sirotkin's code 
*	by Tom Madden.
*
*****************************************************************************/
/*-------------FlatIgnoreThisPatentPub()---------------*/
Boolean
FlatIgnoreThisPatentPub (BioseqPtr bsp, ValNodePtr best, Int4Ptr seqidPt)
{
	Boolean retval = FALSE;
	CitPatPtr cp;
	SeqIdPtr sip;
	PatentSeqIdPtr psip;
	IdPatPtr ip;

	if (seqidPt){
		* seqidPt = 0;
	}
	if (best -> choice == 9){
		cp = (CitPatPtr) best -> data.ptrvalue;
		retval = TRUE;
		for ( sip = bsp -> id; sip; sip = sip -> next){
			if (sip -> choice == SEQID_PATENT){
				psip = (PatentSeqIdPtr) sip -> data.ptrvalue;
				ip = psip -> cit;
				if (ip){
					if (ip -> number){
						if (StringCmp(ip -> number, cp -> number) == 0){
							retval = FALSE;
							if (seqidPt){
								*seqidPt = psip -> seqid;
							}
							break;
						}
					}else if (ip -> app_number){
						if (StringCmp(ip -> app_number, cp -> app_number) == 0){
							retval = FALSE;
							if (seqidPt){
								*seqidPt = (psip -> seqid);
							}
							break;
						}
					}
				}
			}
		}
	}

	return retval;
}



	
/*----------- FlatStringGroup  ()------*/
CharPtr FlatStringGroup 
(CharPtr start, ValNodePtr head, CharPtr delimit, CharPtr period)
{
	return FlatStringParanGroup(start , head, delimit, 
		delimit, period);
}


/*----------- FlatStringParanGroup  ()------*/
CharPtr FlatStringParanGroup 
(CharPtr start, ValNodePtr head, CharPtr start_delimit, CharPtr delimit, CharPtr period)
{
	CharPtr retval = NULL, p;
	Int4 len = 0, lenstart = 0, lenst=0, lende = 0, lenper=0;
	ValNodePtr an;

	if (delimit)
		lende = StringLen(delimit);

	if (start_delimit)
		lenst=StringLen(start_delimit);

	if (period)
		lenper=StringLen(period);

	len = lenst + lenper;

	if ( ! lenst && lende)
		len += lende;

	if (start)
		len += (lenstart = StringLen(start))  ;

	for (an = head; an; an = an -> next)
		if (an -> data.ptrvalue)
			len += lende + StringLen( (CharPtr) ( an -> data.ptrvalue));


/*--will be lende too long - who cares --- */

	p = retval = MemNew ( (size_t)(len+1));
	if (start){
		StringCpy (p, start);
		p += lenstart;
		if (head){
			StringCpy(p, start_delimit);
			p += lenst;
		}
	}

	for (an = head; an; an = an -> next)
		if (an -> data.ptrvalue){
			StringCpy(p, (CharPtr) (an -> data.ptrvalue) );
			p += StringLen( (CharPtr) ( an -> data.ptrvalue));
			if (an -> next){
				StringCpy(p, delimit);
				p += lende;
			}
		}

	if (period){
		StringCpy(p, period);
		p += lenper;
	}

	return retval;
}	

CharPtr GetAffiliation (AffilPtr afp)

{
	Boolean need_comma=FALSE;
	CharPtr string=NULL, temp, ptr;
	Int2 aflen=15;

	if (afp) {
		if (afp -> choice == 1){
			if (afp -> affil){
				aflen += StringLen(afp -> affil);
			}
		}else if (afp -> choice == 2){
			aflen += StringLen (afp -> affil) + 
			StringLen (afp -> div) + 
			StringLen (afp -> city) + 
			StringLen (afp -> sub) + 
			StringLen (afp -> street) + 
			StringLen (afp -> country);
		}

		temp = string = MemNew(aflen);

		if ( afp -> choice == 1){
			 if (afp -> affil){
				*temp = ' '; temp++;
				ptr = afp->affil;
				while ((*temp = *ptr) != '\0')
				{
					temp++; ptr++;
				}
			 }
		}else if (afp -> choice == 2){

			*temp = ' '; 
			temp++;
			if( afp -> div) { 
				if (need_comma)
				{
					*temp = ','; temp++;
					*temp = ' '; temp++;
				}
				ptr = afp->div;
				while ((*temp = *ptr) != '\0')
				{
					temp++; ptr++;
				}
				need_comma = TRUE;
			}

			if(afp -> affil) { 
				if (need_comma)
				{
					*temp = ','; temp++;
					*temp = ' '; temp++;
				}
				ptr = afp->affil;
				while ((*temp = *ptr) != '\0')
				{
					temp++; ptr++;
				}
				need_comma = TRUE;
			}

			if(afp -> street) { 
				if (need_comma)
				{
					*temp = ','; temp++;
					*temp = ' '; temp++;
				}
				ptr = afp->street;
				while ((*temp = *ptr) != '\0')
				{
					temp++; ptr++;
				}
				need_comma = TRUE;
			}

			if( afp -> city) { 
				if (need_comma)
				{
					*temp = ','; temp++;
					*temp = ' '; temp++;
				}
				ptr = afp->city;
				while ((*temp = *ptr) != '\0')
				{
					temp++; ptr++;
				}
				need_comma = TRUE;
			}

			if( afp -> sub) { 
				if (need_comma)
				{
					*temp = ','; temp++;
					*temp = ' '; temp++;
				}
				ptr = afp->sub;
				while ((*temp = *ptr) != '\0')
				{
					temp++; ptr++;
				}
				need_comma = TRUE;
			}

			if( afp -> country){
				if (need_comma)
				{
					*temp = ','; temp++;
					*temp = ' '; temp++;
				}
				ptr = afp->country;
				while ((*temp = *ptr) != '\0')
				{
					temp++; ptr++;
				}
				need_comma = TRUE;
			}	
		}
		temp++;
		*temp = '\0';
	}
	return string;
}	/* GetAffiliation */

/************************************************************************
*	Int2 StorePubInfo (BioseqPtr bsp, ValNodePtr PNTR vnp, Boolean error_msgs);
*
*	Get information about pubs.  Eventually a check will be done
*	to ascertain whether pubs are complete enough to warrant 
*	printing out.  If there are no printable pubs, the entry will
*	be killed.  The return value will indicate whether a problem
*	exists.  "-1" indicates a problem, "0" indicates at least one 
*	printable (thought not valid) pub, a positive number indicates
*	the number of valid articles.
************************************************************************/

Int2 StorePubInfo (BioseqContextPtr bcp, BioseqPtr bsp, ValNodePtr PNTR vnpp, Boolean error_msgs)

{
	Int2 status;
	GetPubDescr (bcp, bsp, vnpp);
	GetPubFeat (bcp, vnpp);
	if (ASN2FF_SHOW_ALL_PUBS) {
		status = 1;
	} else {
		status = CheckPubs(bsp, vnpp, error_msgs);
	}

	return status;
}

/************************************************************************
*	Int2 StoreNAPubInfo (BiotablePtr btp, ValNodePtr PNTR vnp, Boolean error_msgs);
*
*	Get information about pubs from the Nucleic Acid entries.  This
*	function is a KLUDGE and should only be called for the three
*	Nucleic databases and only if no pubs are found on the protein 
*	entries. 
*
*	"-1" indicates a problem, "0" indicates at least one 
*	printable (thought not valid) pub, a positive number indicates
*	the number of valid articles.
************************************************************************/

Int2 StoreNAPubInfo (BiotablePtr btp, Int2 count, ValNodePtr PNTR vnpp, Boolean error_msgs)

{
	BioseqContextPtr bcp=NULL;
	BioseqPtr bsp=btp->bsp[count];
	Int2 index, list_size=0, status=0;
	SortStructPtr List=NULL;
	ValNodePtr vnp;

	bcp = btp->bcp_na[count];
	
	if (bcp != NULL)
	{
		GetPubDescr (bcp, bsp, vnpp);
		GetNAPubFeat (btp, count, vnpp, &List, &list_size);
		status = CheckPubs(btp->bsp[count], vnpp, error_msgs);
	}

	if (status <= 0 && list_size > 0)
	{
		vnp = *vnpp;
		for (index=0; index<list_size; index++)
		{
			vnp = StorePubFeat(vnp, List[index].sfp);
		}
		*vnpp = vnp;
		status = CheckPubs(btp->bsp[count], vnpp, error_msgs);
	}
	
	if (list_size > 0)
		List = MemFree(List);

	return status;
}	/* StoreNAPubInfo */

/*****************************************************************************
*CheckPubs
*
*	Code to check that the pub contains enough info to warrant 
*	printing out.  This is checked out in ValidatePub.
*	If the pub is not valid, it is deleted from the list.
*
*	Written by Tom Madden.
*
*	Returns the number of valid pubs found.
*
*****************************************************************************/

Int2 CheckPubs (BioseqPtr bsp, ValNodePtr PNTR vnpp, Boolean error_msgs)

{
	Boolean good_one = FALSE;
	Int2 retval=0, status;
	PubStructPtr psp;
	ValNodePtr last, newpub, vnp2, vnp = *vnpp, pub;

	UniquePubs(vnpp);
	last=NULL;
	vnp = *vnpp;
	while (vnp)
	{
		psp = vnp->data.ptrvalue;
		if ((psp->pub)->choice == PUB_Equiv)
			newpub = psp->pub->data.ptrvalue;
		else
			newpub = psp->pub;

		for (vnp2=newpub; vnp2; vnp2=vnp2->next)
			if ((status=ValidatePub(bsp, vnp2)) > 0)
			{
				retval++;
				break;
			}

		if (status < 0)
		{
			vnp->choice = (Uint1) 1;
			if (error_msgs)
				PostARefErrMessage (bsp, psp, NULL, status, NULL);
		}
		else 
		{
			good_one = TRUE;
		}
		vnp = vnp->next;
	}

/* If not one good pub was found, look for something printable and take that. */
	vnp = *vnpp;
	if (good_one == FALSE)
	{
		retval = -1;
		while (vnp)
		{
			psp = vnp->data.ptrvalue;
			pub = FlatRefBest(psp->pub, FALSE, FALSE);
/* If a pub is a patent it probably failed FlatIgnoreThisPatentPub; don't
take it and keep looking. */
			if (pub != NULL && pub->choice != PUB_Patent)
			{
				vnp->choice = (Uint1) 0;
				retval = 0;
				break;
			}
			vnp = vnp->next;
		}
	}

/* Drop the bad ones. */
	vnp = *vnpp;
	last = NULL;
	while (vnp)
	{
		if (vnp->choice != 0)
		{
		    if (last == NULL)
		    {	/* If the first pub is bad. */
			*vnpp = vnp->next;
			vnp->next = NULL;
			FreePubStruct(vnp->data.ptrvalue);
			ValNodeFree(vnp);
			vnp = *vnpp;
		    }
		    else
		    {	/* If any but the first pub is bad. */
			last->next = vnp->next;
			vnp->next = NULL;
			FreePubStruct(vnp->data.ptrvalue);
			ValNodeFree(vnp);
			vnp = last->next;
		    }
		}
		else
		{
			last = vnp;
			vnp = vnp->next;
		}
	}
	
	return retval;
}		

/*****************************************************************************
*ValidatePub 
*
*	Checks whether pubs include authors, journal names etc.
*
* At present (2/10/94) this function passes most pubs w/o checks (i.e.,
* status is 0.  This should be tightened up to include more checks 
*(vol. # etc.)???????????
*
*	(8/17/94): adding more specific information on error return:
*	status:
*		-1	not specific, reference is just bad.
*		-2	no valid author names.
*		-3	no valid journal title.
*
*****************************************************************************/

Int2 ValidatePub (BioseqPtr bsp, ValNodePtr the_pub)
{
	AuthListPtr ap;
	AuthorPtr authptr;
	Boolean ignore_this=FALSE;
	CitBookPtr cb;
	CitSubPtr cs;
	CitGenPtr cg;	
	CitArtPtr ca;
	CitPatPtr cp;
	Int2 length, status = -1;
	Int4 pat;
	MedlineEntryPtr ml;
	CitJourPtr jp;
	ImprintPtr ip;
	NameStdPtr nsp;
	PersonIdPtr pid;

	switch ( the_pub -> choice){

     		 case 2:
			cs = (CitSubPtr) the_pub -> data.ptrvalue;
			if ( cs -> imp){
				ip = cs -> imp;
				if (ip->date)
					status = 1;
			}
			ap = cs->authors;
		break;
	/*  thesis */
		case 11:
			cb = (CitBookPtr) the_pub -> data.ptrvalue;
			if ( cb -> imp){
				ip = cb -> imp;
				if (ip)
					status = 1;
			}
			ap = cb->authors;
		break;

/* Patent */
		case 9:
			ignore_this = FlatIgnoreThisPatentPub(bsp, the_pub, &pat);
			if (ignore_this == FALSE || ISA_aa(bsp->mol))
				status = 1;
			cp = (CitPatPtr) the_pub -> data.ptrvalue;
			ap = cp->authors;
			break;

/*  CitArts  or CitBook */
		case 5:
		case 3:
			if ( the_pub -> choice == 3){
				ml = (MedlineEntryPtr) the_pub -> data.ptrvalue;
				ca = (CitArtPtr) ml -> cit;

			}else{
				ca = (CitArtPtr) the_pub -> data.ptrvalue;
			}
		if( ca -> fromptr){
			if ( ca -> from ==1){
				jp = (CitJourPtr) ca -> fromptr;
				if (StringLen((CharPtr)jp->title->data.ptrvalue) == 0)
				{
					ip = jp->imp;
					if (ip && ip->prepub == 0)
					{
						status = -3;
						break;
					}
				}
				if ( jp -> imp)
						status = 1;
			}else {
				CitBookPtr book = (CitBookPtr) ca -> fromptr;
					if ( book -> imp)
							status = 1;
			}
		}
			ap = ca -> authors;
			break;
		case 1: 
			cg = (CitGenPtr) the_pub -> data.ptrvalue;
			if (cg -> cit)
			{
				if (StringNICmp("unpublished", cg->cit, 11) == 0)
					status = 1;
				if (StringNICmp("submitted", cg->cit, 8) == 0)
					status = 1;
				if (StringNICmp("to be published", cg->cit, 15) == 0)
					status = 1;
				if (StringNICmp("in press", cg->cit, 8) == 0)
					status = 1;
				else if (StrStr(cg->cit, "Journal") != NULL)
					status = 1;
			}
			else 
			{
				if ((cg->journal) && (cg->date))
					status = 1;
				else if (cg->journal == NULL)
					status = -4;
				else if (cg->date == NULL)
					status = -5;
			}

			ap = cg -> authors;
			break;
		default:
			status = -1;
			break;
	}

	if (status < 0)
		return status;
	else if (ap == NULL)
		return -2;

	if (ap && the_pub -> choice != 9) {
		if (ap->choice == 1)
		{
			authptr = (AuthorPtr) (ap->names)->data.ptrvalue;
			pid = authptr->name;
			if (pid->choice == 2)
			{
				nsp = (NameStdPtr) pid->data;
				length = StringLen(nsp->names[0]);
				length += StringLen(nsp->names[3]);
				if (length == 0)
					status = -2;
			}
			else if (pid->choice == 3 || pid->choice == 4)
			{
				length = StringLen((CharPtr)pid->data);
				if (length == 0)
					status = -2;
			}
		}
		else if (ap->choice == 2 || ap->choice == 3)
		{
			if (ap->names == NULL)
				status = -2;
			else if (StringLen((CharPtr)ap->names->data.ptrvalue) == 0)
				status = -2;
		}
	}

	return status;
}

/*************************************************************************
*void PostARefErrMessage(BioseqPtr bsp, PubStructPtr psp, ValNodePtr pub, Int2 status, CharPtr string)
*
*	Given a pub, by "checkPubs", this function posts an error
*	message about the invalid reference.  
*
*	status has the following meanings:
*
*		-1	not specific, reference is just bad.
*		-2	no valid author names.
*		-3	no valid journal title.
*************************************************************************/

void PostARefErrMessage (BioseqPtr bsp, PubStructPtr psp, ValNodePtr ext_pub, Int2 status, CharPtr ext_string)

{
	Biotable bt;
	BiotablePtr btp = &bt;
	Boolean ignore_this, submit=FALSE;
	CharPtr authors=NULL, string=NULL, newstring=NULL, title=NULL, journal=NULL, ptr;
	Char buffer[30];
	Int2 length;
	Int4 pat_seqid=0;
	ValNodePtr equiv, pub;

	btp->format = GENBANK;
	btp->newline = ' ';
/*
	btp->error_msgs = TRUE;
*/
	btp->error_msgs = FALSE;
	btp->bsp = (BioseqPtr PNTR) MemNew(sizeof(BioseqPtr));
	btp->bsp[0] = bsp;


	MakeAnAccession(buffer, bsp);
	flat2asn_delete_locus_user_string();
	flat2asn_install_locus_user_string(buffer);
	flat2asn_delete_accession_user_string();
	flat2asn_install_accession_user_string(buffer);

	if (ext_pub)
		pub = ext_pub;
	else if (psp)
		pub = FlatRefBest(psp->pub, TRUE, FALSE);
	else	/* Nothing done here, as neither pub nor psp is given! */
		return;

	if (pub == NULL && ext_string == NULL)
	{	/* Look for pubs that only have muid of zero! */
		equiv = psp->pub;
		if (equiv->choice == PUB_Equiv)
			pub = equiv->data.ptrvalue;
		else
			pub = equiv;
		if (pub)
		{
			if (pub->choice == 4 && pub->next == NULL)
			   if (pub->data.intvalue == 0)
				status = -9;
		}
		pub = NULL;	/* To avoid problems down below */
	}



	if (pub != NULL)
	{
		ignore_this = FlatIgnoreThisPatentPub(bsp, pub, &pat_seqid);
/* if ext_pub is not NULL, then the call is from FlatJournal, due to 
bad pagination, and calling FlatJournal, with error_msgs set to TRUE, risks 
a loop! */
		if (ext_pub != NULL) 
			btp->error_msgs = FALSE;
		journal = FlatJournal(btp, 0, pub, pat_seqid, &submit, FALSE);
		title = FlatPubTitle(pub);
		authors = FlatAuthor(btp, pub);
	}

	MemFree(btp->bsp);

	length = StringLen(ext_string) + StringLen(authors) + StringLen(journal) + StringLen(title) + 5;

	string = newstring = MemNew(length*sizeof(Char));

	if (authors)
	{
		ptr = authors;
		while (*ptr != '\0')
		{
			*string = *ptr;
			ptr++; string++;
		}
		authors = MemFree(authors);
	}
	*string = '|';
	string++;

	if (journal)
	{
		ptr = journal;
		while (*ptr != '\0')
		{
			*string = *ptr;
			ptr++; string++;
		}
	}
	*string = '|';
	string++;
	
	if (title)
	{	/* Also may be needed below. */
		ptr = title;
		while (*ptr != '\0')
		{
			*string = *ptr;
			ptr++; string++;
		}
	}
	*string = '|';
	string++;

	if (ext_string)
	{
		ptr = ext_string;
		while (*ptr != '\0')
		{
			*string = *ptr;
			ptr++; string++;
		}
	}

/* Sometimes an error is caused by a Direct Sub in a cit-gen */
	if (status > 0 && 
		pub->choice == 1 && 
		    title != NULL &&
			StringNCmp(title, "Direct Submission", 17) == 0)
		status = 3;
	else if (status > 0 && 
		pub->choice == 1 && 
		    journal != NULL &&
			StringNCmp(journal, "Submitted", 9) == 0)
		status = 3;

	if (title)
		title = MemFree(title);
	if (journal)
		journal = MemFree(journal);

	if (status == 5)
	    ErrPostEx(SEV_INFO, ERR_REFERENCE_ParanInSuppInfo, newstring);
	if (status == 4)
	    ErrPostEx(SEV_INFO, ERR_REFERENCE_VolHasSupplInfo, newstring);
	else if (status == 3)
	    ErrPostEx(SEV_INFO, ERR_REFERENCE_DirSubInCitGen, newstring);
	else if (status == 2)
	    ErrPostEx(SEV_WARNING, ERR_REFERENCE_NoPageNumbering, newstring);
	else if (status == 1)
	    ErrPostEx(SEV_WARNING, ERR_REFERENCE_IllegalPageRange, newstring);
	else if (status == -1)
	    ErrPostEx(SEV_ERROR, ERR_REFERENCE_Illegalreference, newstring);
	if (status == -2)
	    ErrPostEx(SEV_ERROR, ERR_REFERENCE_NoAuthorName, newstring);
	else if (status == -3 || status == -4)
	    ErrPostEx(SEV_ERROR, ERR_REFERENCE_NoJournalName, newstring);
	else if (status == -5)
	    ErrPostEx(SEV_ERROR, ERR_REFERENCE_NoDateOnRef, newstring);
	else if (status == -9)
	    ErrPostEx(SEV_ERROR, ERR_REFERENCE_MuidZeroOnlyInfo, newstring);
	
	newstring = MemFree(newstring);
	
	return;
}

/*************************************************************************
*Boolean CheckNAFeat(BiotablePtr btp, Int2 count, SeqFeatPtr sfp)
*
*	This function does a check that the NAFeat will be valid at all,
*	i.e., that it has a valid key and all the mandatory qualifiers
*	are present.  ValidateNAImpFeat does a more thorough validation
*	later on.
*	New condition (7/21/94): if the feature is a misc_feature and
*	has no valid qualifiers, it is dropped.
************************************************************************/
Boolean CheckNAFeat(BiotablePtr btp, Int2 count, SeqFeatPtr sfp)

{
	Boolean allocated=FALSE, error_msgs, found_key, location=FALSE, valid=FALSE;
	Char key[20];
	CharPtr ptr=NULL, new_key;
	GBQualPtr gbqual;
	Int2 index, status;

	found_key = GetNAFeatKey(key, sfp);

	if (found_key)
	{
		flat2asn_install_feature_user_string(key, NULL);
		new_key = key;
		index = GBFeatKeyNameValid(&new_key, FALSE);
		if (new_key != key)
		{
			StringCpy(key, new_key);
			new_key = MemFree(new_key);
		}
		if (index != -1)
		{
			error_msgs=btp->error_msgs;
			location = CheckAndGetNAFeatLoc(btp, count, &ptr, sfp, TRUE);
			flat2asn_delete_feature_user_string();
			flat2asn_install_feature_user_string(key, ptr);
			gbqual=sfp->qual;
			if (StringCmp(key, "source") == 0) {  /*fake check for Biosource */
				status = GB_FEAT_ERR_NONE;
				valid = TRUE;
			} else {
				status = GBFeatKeyQualValid(index, &gbqual, error_msgs, FALSE);
				if (status == GB_FEAT_ERR_NONE)
					valid = TRUE;
				else if (status == GB_FEAT_ERR_REPAIRABLE)
				{ /* Check if bad qual is required!! */
					gbqual = GBQualCopy(sfp->qual);
					allocated = TRUE;
					status = GBFeatKeyQualValid(index, &gbqual, FALSE, TRUE);
					if (status != GB_FEAT_ERR_DROP)
						valid = TRUE;
				}
			}
			ptr = MemFree(ptr);

			if (valid == TRUE && 
				StringCmp(key, "misc_feature") == 0)
			{ /* Check for at least one valid qual on misc_feature*/
				if (gbqual == NULL)
					if (sfp->data.choice != 1 &&
						sfp->comment == NULL)
					{
						valid = FALSE;
						ErrPostEx(SEV_WARNING, ERR_FEATURE_NoQualOnMiscFeat, " "); 
					}
			} /* was gbqual allocated, or a copy of sfp->qual?*/
			if (allocated == TRUE && gbqual != NULL)
					gbqual = GBQualFree(gbqual);
		}
		else if (btp->error_msgs == TRUE)
		{
			ErrPostEx(SEV_WARNING, ERR_FEATURE_UnknownFeatureKey, " "); 
		}
		flat2asn_delete_feature_user_string();
	}

	if (location && valid)
		return TRUE;
	else
		return FALSE;
}	/* CheckNAFeat */

/*************************************************************************
*Boolean CheckAndGetNAFeatLoc(BiotablePtr btp, Int2 count, CharPtr PNTR buffer, SeqFeatPtr sfp, Boolean loc_return)
*
*	This function does a check that the feature location is valid.
*	It makes use of parser validation functions.
*	The Boolean "loc_return" specifies whether or not a location should 
*	be returned.
************************************************************************/

Boolean CheckAndGetNAFeatLoc(BiotablePtr btp, Int2 count, CharPtr PNTR buffer, SeqFeatPtr sfp, Boolean loc_return)

{
	Boolean location=FALSE;
	BioseqPtr bsp=btp->bsp[count];
	CharPtr flatloc_ptr=NULL;
	ImpFeatPtr ifp=NULL;
	int num_errs;
	Boolean keep_rawPt, sitesPt;
	SeqIdPtr seq_id=bsp->id;
	SeqLocPtr slp;

	install_gbparse_range_func((Pointer) btp, check_range);
	if (btp->error_msgs == TRUE)
		install_gbparse_error_handler(do_loc_errors);
	else
		install_gbparse_error_handler(do_no_loc_errors);

	if (sfp->data.choice == 8)
	{	/* Use ifp->loc if OK */
		ifp = sfp->data.value.ptrvalue;
		if (ifp && ifp->loc)
		{
			slp = gbparseint(ifp->loc, &keep_rawPt, &sitesPt, &num_errs, seq_id);
			if (num_errs == 0)
			{
				SeqLocFree(slp);
				location=TRUE;
			}
			else if ( ASN2FF_VALIDATE_FEATURES == FALSE && loc_return)
			{
				flat2asn_install_feature_user_string(ifp->key, ifp->loc);
				flat2asn_delete_feature_user_string();
			}
			if (loc_return)
			{
				if (*buffer)
					*buffer = MemFree(*buffer);
				*buffer = StringSave (ifp->loc);
			}
		}
	}

	if (location == FALSE)
	{	/* only called if not an ImpFeatPtr and/or ifp->loc bad */
		flatloc_ptr =  FlatLoc(btp, count, sfp->location);
		slp = gbparseint(flatloc_ptr, &keep_rawPt, &sitesPt, &num_errs, seq_id);

		if (num_errs == 0)
		{
			SeqLocFree(slp);
			location=TRUE;
		}
/*
		else if (ASN2FF_VALIDATE_FEATURES == FALSE && loc_return)
		{
			flat2asn_install_feature_user_string(ifp->key, flatloc_ptr);
			flat2asn_delete_feature_user_string();
		}
*/

		if (loc_return)
		{
			if (*buffer)
				*buffer = MemFree(*buffer);
			*buffer = flatloc_ptr;
		}
		else
			flatloc_ptr = MemFree(flatloc_ptr);
	}

	if (location)
		return TRUE;
	else
		return FALSE;
}	/* CheckAndGetNAFeatLoc */

/*****************************************************************************
*void GetAAFeatLoc(BiotablePtr btp, Int2 count, CharPtr PNTR buffer, SeqFeatPtr sfp, Boolean use_product)
*
*	"twin" to CheckAndGetNAFeatLoc, except that no checking is done
*	yet for genpept.
****************************************************************************/

void GetAAFeatLoc(BiotablePtr btp, Int2 count, CharPtr PNTR buffer, SeqFeatPtr sfp, Boolean use_product)

{
	Boolean location=FALSE;
	CharPtr flatloc_ptr=NULL;
	ImpFeatPtr ifp=NULL;

	if (sfp->data.choice == 8)
	{	/* Use ifp->loc if OK */
		ifp = sfp->data.value.ptrvalue;
		if (ifp && ifp->loc)
		{
			if (*buffer)
				*buffer = MemFree(*buffer);
			*buffer = StringSave (ifp->loc);
			location = TRUE;
		}
	}

	if (location == FALSE)
	{	/* only called if not an ImpFeatPtr and/or ifp->loc bad */
		if (use_product)  /* Used for CDS in genpept. */
			flatloc_ptr =  FlatLoc(btp, count, sfp->product);
		else
			flatloc_ptr =  FlatLoc(btp, count, sfp->location);

		if (*buffer)
			*buffer = MemFree(*buffer);
		*buffer = flatloc_ptr;
	}

}	/* GetAAFeatLoc */

static GBQualPtr GBQualCopy (GBQualPtr old_qual)

{
        GBQualPtr curq, new_qual, new_qual_start=NULL;

        if (old_qual)
        {
                new_qual_start = new_qual = GBQualNew();
                for (curq=old_qual; curq; curq=curq->next)
                {
                        if (curq->val)
                                new_qual->val = StringSave(curq->val);
                        if (curq->qual)
                                new_qual->qual = StringSave(curq->qual);
                        if (curq->next)
                        {
                                new_qual->next = GBQualNew();
                                new_qual = new_qual->next;
                        }
                        else
                                new_qual->next = NULL;
                }
        }
        return new_qual_start;
}

