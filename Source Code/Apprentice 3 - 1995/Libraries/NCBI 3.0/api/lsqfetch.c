/*  lsqfetch.c
* ===========================================================================
*
*                            PUBLIC DOMAIN NOTICE
*               National Center for Biotechnology Information
*
*  This software/database is a "United States Government Work" under the
*  terms of the United States Copyright Act.  It was written as part of
*  the author's official duties as a United States Government employee and
*  thus cannot be copyrighted.  This software/database is freely available
*  to the public for use. The National Library of Medicine and the U.S.
*  Government have not placed any restriction on its use or reproduction.
*
*  Although all reasonable efforts have been taken to ensure the accuracy
*  and reliability of the software and data, the NLM and the U.S.
*  Government do not and cannot warrant the performance or results that
*  may be obtained by using this software or data. The NLM and the U.S.
*  Government disclaim all warranties, express or implied, including
*  warranties of performance, merchantability or fitness for any particular
*  purpose.
*
*  Please cite the author in any work or product based on this material.
*
* ===========================================================================
*
* File Name:  lsqfetch.c
*
* Author:  Jinghui Zhang
*
* Version Creation Date: 5/24/95
*
*
* File Description:  functions for fetching the local sequences
*
* Modifications:
* --------------------------------------------------------------------------
* Date     Name        Description of modification
*
* $Log: lsqfetch.c,v $
 * Revision 1.2  1995/06/02  20:10:11  zjing
 * fix bug
 *
*
*
* ==========================================================================
*/


#ifndef _LSQFETCH_
#include <lsqfetch.h>
#endif


#include <accentr.h>

/***********************************************************************
***
*
*	fasta_sep(): making a Seq-entry from a FASTA formatted file
*
************************************************************************
***/
static Boolean ck_DNA(CharPtr seq_ptr, Int4 ck_len)   /*is it DNA or aa??*/
{
 
        Int4 i, non_DNA=0;
 
        for(i=0; i<ck_len; ++i)
             if(StrChr("ACGTNacgtn", seq_ptr[i])==NULL)
               ++non_DNA;
 
        if(non_DNA >= ck_len/4)
          return FALSE;
        else return TRUE;
}


static SeqEntryPtr Sep_from_ByteStore(ByteStorePtr bsp, Int4 length, Boolean is_dna, SeqIdPtr sip) 
{
  	BioseqPtr biosp;
  	SeqEntryPtr sep;

	biosp = BioseqNew();
	biosp->id = SeqIdDup(sip);
        biosp->seq_data = bsp;
        biosp->repr = Seq_repr_raw;
        biosp->mol = (is_dna) ? Seq_mol_dna : Seq_mol_aa;
        biosp->length = length;
        biosp->topology = 1;                    /** linear sequence**/
        biosp->seq_data_type = (is_dna) ? Seq_code_iupacna : Seq_code_iupacaa;
        sep = SeqEntryNew();
        sep->choice = 1;
        sep->data.ptrvalue = biosp;
	SeqMgrSeqEntry(SM_BIOSEQ, (Pointer)biosp, sep);

        return sep;

}

static ByteStorePtr make_lib(FILE *ifp, CharPtr name, Int4Ptr length, BoolPtr is_DNA)
{
   Int4 n_len, pos, seq_len;
   Char temp[101];
   CharPtr p;
   ByteStorePtr bsp;
   Boolean is_found, is_end;
   Boolean check_DNA = FALSE;
 
 
	if(name != NULL)
	{
		rewind(ifp);
   		n_len = StringLen(name);
	}
   	is_found = FALSE;
   	while(!is_found && FileGets(temp, 100, ifp) != NULL)   /*find the right seq*/
   	{
		if(temp[0] == '>')
		{
			if(name!=NULL)
			{
				if(StringNCmp(temp+1, name, (size_t)n_len) ==0)
					is_found = TRUE;
				else
					is_found = FALSE;
			}
			else
				is_found = TRUE;
		}
		if(is_found)
		{
			pos = ftell (ifp);
			is_end = FALSE;
			seq_len =0;
			while(FileGets(temp, 100, ifp) != NULL && !is_end)
			{
				if(temp[0] == '>')
					is_end = TRUE;
				else
					seq_len += (StringLen(temp) -1);
			}
		}
				
	}


    if(!is_found)
	return NULL;

    bsp = BSNew(seq_len+1);
    BSSeek(bsp, 0, SEEK_SET);

    fseek(ifp, pos, SEEK_SET);
    is_end = FALSE;
    seq_len = 0;
    while(FileGets(temp, 100, ifp) != NULL && !is_end)
    {
	if(temp[0] == '>')
		is_end = TRUE;
	else
	{
	    if(!check_DNA)	/*check if it is a DNA sequence*/
	    {
		*is_DNA = ck_DNA(temp, StringLen(temp));
		check_DNA = TRUE;
	    }
	    for(p=temp; p!=NULL && *p != '\n'; ++p)
		if(IS_ALPHA(*p))
		{
			++seq_len;
			BSPutByte(bsp, (Int2)(*p));
		}
        }
    }

    *length = seq_len;
    return bsp;
}


/*************************************************************************
***
*	fasta_lib_sep(): make a Seq-entry from the FASTA library
*
**************************************************************************
***/
SeqEntryPtr fasta_lib_sep PROTO((FILE *fp, CharPtr seq_name, SeqIdPtr sip));

SeqEntryPtr fasta_lib_sep(FILE *fp, CharPtr seq_name, SeqIdPtr sip)
{
	
	ByteStorePtr bsp;
	Int4 length;
	Boolean is_dna;

	if((bsp = make_lib(fp, seq_name, &length, &is_dna)) != NULL)
		return Sep_from_ByteStore(bsp, length, is_dna, sip);
	else
		return NULL;
}

	
static void FindBigCallback(SeqEntryPtr sep, Pointer data, Int4 index, Int2 indent)
{
	ValNodePtr vnp;
	BioseqPtr bsp, curr;

        if(sep->choice != 1)
           return;

        if(data == NULL)
           return;
 
        vnp = (ValNodePtr)data;
        bsp = sep->data.ptrvalue;
	if(bsp)
	{
		if(vnp->data.ptrvalue == NULL)
			vnp->data.ptrvalue = bsp;
		else
		{
			curr = (BioseqPtr)(vnp->data.ptrvalue);
			if(bsp->length > curr->length)
				vnp->data.ptrvalue = bsp;
		}
	}
 
}
 
 
BioseqPtr find_big_bioseq(SeqEntryPtr sep)
{
	ValNode vn;

	vn.data.ptrvalue = NULL;
	if (sep != NULL)
    		BioseqExplore (sep, (Pointer) (&vn), FindBigCallback);
 
  	return (BioseqPtr)(vn.data.ptrvalue);
}


 
#define FASTALIB_ERROR	0	/*can not open fasta lib*/
#define FASTALIB_OPEN	1	/*fasta library is open*/
#define FASTALIB_CLOSE	2	/*fasta libary is closed*/
typedef struct fastalib{
	Boolean use_locus;
	Char  file_name[100];
	FILE  *fp;
	Uint1 state;
	struct fastalib PNTR next;
}FastaLib, PNTR FastaLibPtr;

/**********************************************************************
*
*	FastaLibBioseqFetchEnable(libs, now)
*	Initiate the function for fetch a Bioseq from a Fasta Library 
*	file. libs is a list of library file names. 
*	If now = TRUE, open the library files and set the state to 
*	FASTALIB_OPEN. return TRUE for success. 
*
***********************************************************************/
static CharPtr libproc = "FastaLibBioseqFetch";

static Pointer LIBCALLBACK FreeSeqName(Pointer data)
{
	MemFree(data);
	return NULL;
}
static Int2 LIBCALLBACK FastaLibBioseqFetchFunc (Pointer data)
{
	OMProcControlPtr ompcp;
	ObjMgrProcPtr ompp;
	FastaLibPtr flp;
	SeqIdPtr sip;
	OMUserDataPtr omdp;
	CharPtr seq_name = NULL;
	Char name[100];
	SeqEntryPtr sep = NULL;
	BioseqPtr bsp;

	ompcp = (OMProcControlPtr)data;
	ompp = ompcp->proc;
	flp = (FastaLibPtr)(ompp->procdata);

	if(ompcp->input_entityID)
	{
		omdp = ObjMgrGetUserData(ompcp->input_entityID, ompp->procid, OMPROC_FETCH, 0);
		if(omdp != NULL)
			seq_name = omdp->userdata.ptrvalue;
	}

	if(seq_name == NULL)
	{
		sip = (SeqIdPtr)(ompcp->input_data);
		if(sip == NULL)
			return OM_MSG_RET_ERROR;
		seqid_to_string(sip, name, flp->use_locus);
		seq_name = name;
	}

	while(flp && sep==NULL)
	{
		if(flp->state == FASTALIB_CLOSE)
		{
			flp->fp = FileOpen(flp->file_name, "r");
			if(flp->fp == NULL)
				flp->state = FASTALIB_ERROR;
			else
				flp->state = FASTALIB_OPEN;
		}
		if(flp->state == FASTALIB_OPEN)
			sep = fasta_lib_sep(flp->fp, seq_name, sip);
		if(sep == NULL)
			flp = flp->next;
	}
	
	if(sep == NULL)
		return OM_MSG_RET_OK;

	bsp = BioseqFindInSeqEntry(sip, sep);
	if(bsp == NULL)
		bsp = find_big_bioseq(sep);
	ompcp->output_data = (Pointer)bsp;
	ompcp->output_entityID = ObjMgrGetEntityIDForChoice(sep);
	omdp = ObjMgrAddUserData(ompcp->output_entityID, ompp->procid, OMPROC_FETCH, 0);
	omdp->userdata.ptrvalue = StringSave(seq_name);
	omdp->freefunc = FreeSeqName;
	return OM_MSG_RET_DONE;
}

Boolean FastaLibBioseqFetchEnable(ValNodePtr libs, Boolean now)
{
	FastaLibPtr flp = NULL, new, curr;
	Boolean ok;
	FILE *fp;
	CharPtr file_name;
	
	while(libs)
	{
		ok = TRUE;
		file_name = libs->data.ptrvalue;
		if(now)
		{
			if((fp = FileOpen(file_name, "r")) == NULL)
				ok = FALSE;
		}
		if(ok)
		{
			new = MemNew(sizeof(FastaLib));
			new->use_locus = FALSE;
			StringCpy(new->file_name, file_name);
			if(now)
			{
				new->state = FASTALIB_OPEN;
				new->fp = fp;
			}
			else
				new->state = FASTALIB_CLOSE;
			new->next = NULL;
			if(flp == NULL)
				flp = new;
			else
			{
				curr = flp;
				while(curr->next != NULL)
					curr = curr->next;
				curr->next = new;
			}
		}
		libs = libs->next;
	}
	
	if(flp == NULL)
		return FALSE;
	ObjMgrProcLoad(OMPROC_FETCH, libproc, libproc, OBJ_SEQID, 0, OBJ_BIOSEQ, 0, (Pointer)flp, FastaLibBioseqFetchFunc, PROC_PRIORITY_DEFAULT);
	return TRUE;
}


/***********************************************************************
*
*	FastaLibBioseqFetchDisable()
*	Free the data assoicated with the proc and Free the user data
*	as well.
*
***********************************************************************/
void FastaLibBioseqFetchDisable(void)
{
	ObjMgrPtr omp;
	ObjMgrProcPtr ompp;
	FastaLibPtr flp, next;

	omp = ObjMgrGet();
	ompp = ObjMgrProcFind(omp, 0, libproc, OMPROC_FETCH);
	if(ompp == NULL)
		return;
	ObjMgrFreeUserData(0, ompp->procid, OMPROC_FETCH, 0);

	flp = (FastaLibPtr)(ompp->procdata);
	while(flp)
	{
		if(flp->state == FASTALIB_OPEN)
			FileClose(flp->fp);
		next = flp->next;
		MemFree(flp);
		flp = next;
	}

	return;
}

 

/*********************************************************************
*
*	seqid_to_string(sip, name, use_locus)
*	print the most important field in Seqid to a string stored in 
*	name. 
*
**********************************************************************/
Boolean seqid_to_string(SeqIdPtr sip, CharPtr name, Boolean use_locus)
{
  DbtagPtr db_tag;
  ObjectIdPtr obj_id;
  TextSeqIdPtr tsip;
  PDBSeqIdPtr pip;
  GiimPtr gip;

        switch(sip->choice)
	{
          case 1:       /**local**/
            obj_id = sip->data.ptrvalue;
            if(obj_id->str)
                StringCpy(name, obj_id->str);
            else
                sprintf(name, "%ld", obj_id->id);
            break;

          case 5:       /**genbank**/
          case 6:       /**EMBL**/
          case 7:       /**PIR**/
          case 8:       /**SwissProt**/
          case 10:      /**Other**/
          case 13:      /**DDBJ**/
          case 14:      /**PRF**/
            tsip = sip->data.ptrvalue;
            if(tsip->accession)
                StringCpy(name, tsip->accession);
            if((tsip->name && use_locus) || tsip->accession == NULL)
                StringCpy(name, tsip->name);
 
            break;
 
          case 11:      /**general**/
            db_tag = sip->data.ptrvalue;
            obj_id = db_tag->tag;
            if(obj_id->str)
              StringCpy(name, obj_id->str);
            else
                sprintf(name, "%ld", obj_id->id);
            break;
 
          case 4:       /**giim**/
            gip = sip->data.ptrvalue;
            sprintf(name, "%ld", (long)(gip->id));
            break;

          case 2:     	/*gibbseq*/
	  case 3:	/*gibbmt*/
	  case 12:	/*gi*/
            sprintf(name, "%ld", (long)(sip->data.intvalue));
            break;

          case 15:      /*pdb*/
            pip = sip->data.ptrvalue;
            StringCpy(name, pip->mol);
            break;
	  default:
	    return FALSE;
	}

	return TRUE;
}

/*******************************************************************
*
*	get_path_delimiter()
*	return the path delimiter for different platforms
*
********************************************************************/
Char get_path_delimiter(void)
{
	#ifdef WIN_MAC
		return ':';
	#endif

	#ifdef WIN_MSWIN
		return '\\';
	#endif

	return '/';
}

/*********************************************************************
*
*	FileBioseqFetchEnable(path, ext)
*	Initiate a BioseqFetch function by either reading an ASN.1
*	Seq-entry file or FASTA file. path->choice determines the
*	type of the file, such as text ASN, binary ASN and FASTA file
*	ext is the extension that is needed to add to the end of the
*	sequence name to make the sequence file
*
*********************************************************************/
 
typedef ValNode SingleBspFile, FAR *SingleBspFilePtr; 

typedef struct filebsp{	/*structure for opening a bioseq from a file*/
	Boolean use_locus;
	CharPtr path;		/*path for the files*/
	CharPtr ext;		/*extension for files*/
}FileBsp, PNTR FileBspPtr;	


static CharPtr fileproc = "FileBioseqFetch";
static Int2 LIBCALLBACK FileBioseqFetchFunc (Pointer data)
{
	OMProcControlPtr ompcp;
	ObjMgrProcPtr ompp;
	FileBspPtr fbp;
	SingleBspFilePtr sbfp;
	SeqIdPtr sip;
	OMUserDataPtr omdp;
	CharPtr file_name = NULL;
	Char name[100], f_name[100];
	CharPtr c_name;
	FILE *fp;
	AsnIoPtr aip;
	SeqEntryPtr sep = NULL;
	BioseqPtr bsp;
	Boolean bin;

	ompcp = (OMProcControlPtr)data;
	ompp = ompcp->proc;
	sbfp = (SingleBspFilePtr)(ompp->procdata);

	if(ompcp->input_entityID)
	{
		omdp = ObjMgrGetUserData(ompcp->input_entityID, ompp->procid, OMPROC_FETCH, 0);
		if(omdp != NULL)
			file_name = omdp->userdata.ptrvalue;
	}

	if(file_name == NULL)
	{
		sip = (SeqIdPtr)(ompcp->input_data);
		if(sip == NULL)
			return OM_MSG_RET_ERROR;
	}
	while(sbfp && sep==NULL)
	{
		fbp = sbfp->data.ptrvalue;
		if(file_name == NULL)
		{
			seqid_to_string(sip, name, fbp->use_locus);
			if(fbp->path)
				sprintf(f_name, "%s%s", fbp->path, name);
			else
				StringCpy(f_name, name);
			if(fbp->ext)
				StringCat(f_name, fbp->ext);
			c_name= f_name;
		}
		else
			c_name = file_name;
		switch(sbfp->choice)
		{
			case FASTA_FILE:
				if((fp = FileOpen(c_name, "r")) != NULL)
				{
					sep = fasta_lib_sep(fp, NULL, sip);
					FileClose(fp);
				}
				break;

			case TEXT_ASN:	
			case BIN_ASN:
				bin = (sbfp->choice == BIN_ASN);
				if((aip = AsnIoOpen(c_name, bin?"rb":"r")) != NULL) 
				{
					sep = SeqEntryAsnRead(aip, NULL);
					AsnIoClose(aip);
				}
				break;

			default:
				break;
		}
		sbfp = sbfp->next;
	}

	if(sep == NULL)
		return OM_MSG_RET_OK;

	bsp = BioseqFindInSeqEntry(sip, sep);
	if(bsp == NULL)
		bsp = find_big_bioseq(sep);
	ompcp->output_data = (Pointer)bsp;
	ompcp->output_entityID = ObjMgrGetEntityIDForChoice(sep);
	omdp = ObjMgrAddUserData(ompcp->output_entityID, ompp->procid, OMPROC_FETCH, 0);
	omdp->userdata.ptrvalue = StringSave(file_name);
	omdp->freefunc = FreeSeqName;
	return OM_MSG_RET_DONE;
}

static Boolean path_is_loaded(SingleBspFilePtr head, Uint1 choice,  CharPtr path, CharPtr ext)
{
	FileBspPtr fbp;

	while(head)
	{
		if(head->choice == choice)
		{
			fbp = head->data.ptrvalue;
			if(StringCmp(path, fbp->path) ==0)
				if(StringCmp(ext, fbp->ext) ==0)
					return TRUE;
		}
		head = head->next;
	}
	return FALSE;
}
	

Boolean FileBioseqFetchEnable(ValNodePtr path, ValNodePtr ext)
{
	SingleBspFilePtr sbfp = NULL; 
	FileBspPtr new;
	Char c_path[100], c_ext[20]; 
	CharPtr str;
	Int4 len;
	Char delimiter;
	
	if(path == NULL || ext == NULL)
		return FALSE;
	
	while(path && ext)
	{
		new = MemNew(sizeof(FileBsp));
		new->use_locus = FALSE;
		new->path = NULL;
		new->ext = NULL;
		c_path[0] = '\0';
		c_ext[0] = '\0';

		str = path->data.ptrvalue;
		if(str !=NULL)
		{
			delimiter = get_path_delimiter();
			len = StringLen(str);
			if(str[len-1] != delimiter)
				sprintf(c_path, "%s%c", str, delimiter);
			else
				StringCpy(c_path, str);
		}
			
		str = ext->data.ptrvalue;
		if(str !=NULL)
		{
			if(str[0] != '.')
				sprintf(c_ext, ".%s", str);
			else
				StringCpy(c_ext, str);
		}

		if(c_path[0] != '\0')
		{
			if(!path_is_loaded(sbfp, path->choice, c_path, c_ext))
			{
				new->path = StringSave(c_path);
				if(c_ext[0] != '\0')
					new->ext = StringSave(c_ext);
				ValNodeAddPointer(&sbfp, path->choice, new);
			}
		}
		path = path->next;
		ext = ext->next;
	}

	ErrSetFatalLevel(SEV_MAX);
	
	ObjMgrProcLoad(OMPROC_FETCH, fileproc, fileproc, OBJ_SEQID, 0, OBJ_BIOSEQ, 0, (Pointer)sbfp, FileBioseqFetchFunc, PROC_PRIORITY_DEFAULT);
	return TRUE;
}


/**********************************************************************
*
*	FileBioseqFetchDisable()
*	Remove the proc associated with FileBioseqFetch and free all the 
*	sequence names in userdata
*
***********************************************************************/
void FileBioseqFetchDisable(void)
{
	ObjMgrPtr omp;
	ObjMgrProcPtr ompp;
	SingleBspFilePtr sbfp, curr;
	FileBspPtr fbp;

	omp = ObjMgrGet();
	ompp = ObjMgrProcFind(omp, 0, fileproc, OMPROC_FETCH);
	if(ompp == NULL)
		return;
	ObjMgrFreeUserData(0, ompp->procid, OMPROC_FETCH, 0);

	sbfp= (SingleBspFilePtr)(ompp->procdata);
	for(curr = sbfp; curr !=NULL; curr = curr->next)
	{
		fbp = curr->data.ptrvalue;
		MemFree(fbp->path);
		MemFree(fbp->ext);
		MemFree(fbp);
	}
	ValNodeFree(sbfp);
	return;
}

static Boolean lib_is_loaded(ValNodePtr head, CharPtr lib_name)
{
	CharPtr str;

	while(head!=NULL)
	{
		str = (CharPtr)(head->data.ptrvalue);
		if(StringCmp(str, lib_name) == 0)
			return TRUE;
		head = head->next;
	}
	return FALSE;
}


static Boolean load_seq_info(CharPtr word, CharPtr val, ValNodePtr PNTR libs, ValNodePtr PNTR path, ValNodePtr PNTR ext)
{

	if(StringICmp(word, "FASTA_LIB") ==0)
	{
		if(!lib_is_loaded(*libs, val))
			ValNodeAddStr(libs, 0, StringSave(val));
		return TRUE;
	}

	if(StringICmp(word, "FASTA_FILE") ==0)
	{
		ValNodeAddStr(path, FASTA_FILE, StringSave(val));
		return TRUE;
	}

	if(StringICmp(word, "BIN_ASN") == 0)
	{
		ValNodeAddStr(path, BIN_ASN, StringSave(val));
		return TRUE;
	}

	if(StringICmp(word, "TEXT_ASN") == 0)
	{
		ValNodeAddStr(path, TEXT_ASN, StringSave(val));
		return TRUE;
	}

	if(StringICmp(word, "EXT") == 0)
	{
		ValNodeAddStr(ext, 0, StringSave(val));
		return TRUE;
	}

	return FALSE;
}


		


/*********************************************************************
*
*	BioseqFetchInit()
*	Initiate BioseqFetch functions from local data and Entrez. 
*	Local data files are stored in a config file. 
*	If non is successful, return FALSE
*
*********************************************************************/
Boolean BioseqFetchInit(Boolean now)
{
	CharPtr seq_file;
	Boolean success = FALSE;
	ValNodePtr path = NULL, ext = NULL, libs = NULL;
	FILE *fp;
	Char temp[100], str[100];
	CharPtr word, val;

	seq_file = seqinfo_file;
	if((fp = FileOpen(seq_file, "r")) != NULL)
	{
   		while(FileGets(str, 100, fp) != NULL)   /*find the right seq*/
		{
			sscanf(str, "%s\n", temp);
			word = strtok(temp, "=");
			if(word !=NULL)
			{
				val = strtok(NULL, "=");
				load_seq_info(word, val, &libs, &path, &ext);
			}
		}
		FileClose(fp);
	}
			

	if(EntrezBioseqFetchEnable ("testseq", now));
		success = TRUE;
	if(FileBioseqFetchEnable(path, ext))
		success = TRUE;
	if(FastaLibBioseqFetchEnable (libs, now))
		success = TRUE;

	ValNodeFreeData(libs);
	ValNodeFreeData(path);
	ValNodeFreeData(ext);

	return success;
}

/***********************************************************************
*
*	BioseqFetchDisable(): Remove all the functions associated with 
*	BioseqFetch
*
**********************************************************************/
void BioseqFetchDisable(void)
{
	FastaLibBioseqFetchDisable();
	FileBioseqFetchDisable();
	EntrezBioseqFetchDisable();
}



****************************
*
*   GatherSeqEntry (sep, userdata, userproc, scope)
*
*****************************************************************************/
Boolean LIBCALL GatherSeqEntry (SeqEntryPtr sep, Pointer userdata, GatherItemProc userfunc, GatherScopePtr scope)
{
	Uint2 entityID;

	if ((sep == NULL) || (userfunc == NULL)) return FALSE;

	entityID = ObjMgrGetEntityIDForChoice(sep);
	
	return GatherEntity (entityID, userdata, userfunc, scope);
}

/*****************************************************************************
*
*   GatherEntity (entityID, userdata, userproc, scope)
*
*****************************************************************************/
Boolean LIBCALL GatherEntity (Uint2 entityID, Pointer userdata, GatherItemProc userfunc, GatherScopePtr scope)
{
	InternalGCC igcc;
	Boolean in_scope;
	ObjMgrDataPtr omdp;

	if ((! entityID) || (userfunc == NULL)) return FALSE;

	omdp = ObjMgrGetData (entityID);
	if (omdp == NULL) return FALSE;

	if (! IGCCBuild(&igcc, omdp, userdata, userfunc, scope))
		return FALSE;

	if (igcc.scope.scope != NULL)
	{
		in_scope = ObjMgrIsChild((igcc.scope.scope->data.ptrvalue), (omdp->dataptr));
	}
	else                   /* no scope set.. all in scope */
		in_scope = TRUE;

	                      /*** note: if you need to remotely fetch segments
						   ***   it is good to do it first, so that if you
						   ***   are also getting OBJ_BIOSEQ_SEG, the entry
						   ***   will already in (possibly) cached in memory
						   ***   when you look up length and ids
						   ***/

	if ((igcc.segloc != NULL) && (igcc.scope.scope == NULL)) /* get segs first */
		GatherBioseqPartsFunc(&igcc, omdp->dataptr);

	GatherEntityFunc(omdp, &igcc, in_scope);

	return IGCCclear(&igcc);
}

/*****************************************************************************
*
*   GatherItem (entityID, itemID, itemtype, userdata, userproc)
*      Get an item by entityID, itemID, itemtype
*      GatherContext.seglevel and GatherContext.propagated will not be
*        set on the callback.
*   
*      Sets in_scope to FALSE so that the callback is not called
*         Scope is NULL, so in_scope is never TRUE
*      Sets ignore TRUE for everything not needed to find item
*      Sets locatetype and locateID, which are checked in the traversal
*
*****************************************************************************/
Boolean LIBCALL GatherItem (Uint2 entityID, Uint2 itemID, Uint2 itemtype,
                                   Pointer userdata, GatherItemProc userfunc)
{
	InternalGCC igcc;
	ObjMgrDataPtr omdp;

	if (userfunc == NULL) return FALSE;

	if (itemtype >= OBJ_MAX) return FALSE;

	omdp = ObjMgrGetData(entityID);
	if (omdp == NULL) return FALSE;

	if (! IGCCBuild(&igcc, omdp, userdata, userfunc, NULL))
		return FALSE;

	igcc.locatetype = (Uint1)itemtype;
	igcc.locateID = itemID;
	MemSet((Pointer)(igcc.scope.ignore), (int)(TRUE), (size_t)((OBJ_MAX+1)*sizeof(Boolean)));
	igcc.scope.ignore[itemtype] = FALSE;
	switch (itemtype)		  /* add higher levels if necessary */
	{
		case OBJ_SEQFEAT_CIT:
			igcc.scope.ignore[OBJ_SEQFEAT] = FALSE;
			igcc.scope.ignore[OBJ_PUB_SET] = FALSE;
		case OBJ_SEQFEAT:
		case OBJ_SEQALIGN:
		case OBJ_SEQGRAPH:
			igcc.scope.ignore[OBJ_SEQANNOT] = FALSE;
			break;
		case OBJ_SEQSUB_CONTACT:
		case OBJ_SEQSUB_CIT:
			igcc.scope.ignore[OBJ_SUBMIT_BLOCK] = FALSE;
		case OBJ_SUBMIT_BLOCK:
			igcc.scope.ignore[OBJ_SEQSUB] = FALSE;
			break;
	}
    

	GatherEntityFunc(omdp, &igcc, FALSE);

	return IGCCclear(&igcc);
}

typedef struct gatherdatastruct {
	OMProcControlPtr ompcp;
	Boolean succeeded;
} GDS, PNTR GDSPtr;

static Boolean GatherDataProc (GatherContextPtr gcp)
{
	OMProcControlPtr ompcp;
	GDSPtr gdsp;

	gdsp = (GDSPtr)(gcp->userdata);
	ompcp = gdsp->ompcp;
    
    ompcp->input_data = gcp->thisitem;
    
	switch (gcp->thistype)
	{
		case OBJ_BIOSEQSET:
		case OBJ_BIOSEQ:
			if (gcp->sep != NULL)
			{
				ompcp->input_choice = gcp->sep;
				ompcp->input_choicetype = OBJ_SEQENTRY;
			}
			break;
		default:
			break;
	}
	gdsp->succeeded = TRUE;
	return TRUE;
}

static Boolean ReplaceDataProc (GatherContextPtr gcp)
{
	ObjMgrPtr omp;
	ObjMgrTypePtr omtp;
	ObjMgrDataPtr omdp;
	OMProcControlPtr ompcp;
	Pointer next = NULL, ptr, newptr, dest=NULL, oldsrc;
	GDSPtr gdsp;
	size_t size = 0;
	ValNodePtr vnp;
	SeqEntryPtr sep=NULL;


	gdsp = (GDSPtr)(gcp->userdata);
	ompcp = gdsp->ompcp;
	newptr = ompcp->output_data;

    ptr = gcp->thisitem;
	oldsrc = ptr;

	omp = ObjMgrGet();
	omtp = ObjMgrTypeFind(omp, gcp->thistype, NULL, NULL);
	if (omtp == NULL)
	{
		ErrPostEx(SEV_ERROR,0,0,"ReplaceDataProc: can't find type [%d]", (int)gcp->thistype);
		return TRUE;
	}

	dest = MemNew(1000);   /* temporary buffer for copies */
	if (dest == NULL)
	{
		ErrPostEx(SEV_ERROR,0,0,"ReplaceDataProc: can't allocate buffer");
		return TRUE;
	}
    
	switch (gcp->thistype)
	{
		case OBJ_SEQSUB:
			size = sizeof(SeqSubmit);
			break;
		case OBJ_SUBMIT_BLOCK:
			size = sizeof(SubmitBlock);
			break;
		case OBJ_SEQSUB_CONTACT:
			size = sizeof(ContactInfo);
			break;
		case OBJ_SEQSUB_CIT:
			size = sizeof(CitSub);
			break;
		case OBJ_SEQHIST:
			size = sizeof(SeqHist);
			break;
		case OBJ_BIOSEQSET:
			size = sizeof(BioseqSet);
			break;
		case OBJ_BIOSEQ:
			size = sizeof(Bioseq);
			break;
		case OBJ_SEQDESC:
		case OBJ_BIOSEQ_SEG:
		case OBJ_PUB:
		case OBJ_SEQFEAT_CIT:
		case OBJ_PUB_SET:
		case OBJ_SEQLOC:
		case OBJ_SEQID:
		case OBJ_SEQENTRY:
			vnp = (ValNodePtr)ptr;
			oldsrc = &(vnp->data);
			size = sizeof(DataVal);
			vnp = (ValNodePtr)newptr;
			newptr = &(vnp->data);
			break;
		case OBJ_BIOSEQ_MAPFEAT:
		case OBJ_SEQFEAT:
			size = sizeof(SeqFeat);
			next = (Pointer)(((SeqFeatPtr)(ptr))->next);
			(((SeqFeatPtr)(ptr))->next) = NULL;
			(((SeqFeatPtr)(newptr))->next) = (SeqFeatPtr)next;
			break;
		case OBJ_SEQANNOT:
			size = sizeof(SeqAnnot);
			next = (Pointer)(((SeqAnnotPtr)(ptr))->next);
			(((SeqAnnotPtr)(ptr))->next) = NULL;
			(((SeqAnnotPtr)(newptr))->next) = (SeqAnnotPtr)next;
			break;
		case OBJ_SEQALIGN:
			size = sizeof(SeqAlign);
			next = (Pointer)(((SeqAlignPtr)(ptr))->next);
			(((SeqAlignPtr)(ptr))->next) = NULL;
			(((SeqAlignPtr)(newptr))->next) = (SeqAlignPtr)next;
			break;
		case OBJ_SEQGRAPH:
			size = sizeof(SeqGraph);
			next = (Pointer)(((SeqGraphPtr)(ptr))->next);
			(((SeqGraphPtr)(ptr))->next) = NULL;
			(((SeqGraphPtr)(newptr))->next) = (SeqGraphPtr)next;
			break;
		default:
			ErrPostEx(SEV_ERROR,0,0,"ReplaceDataProc: unsupported type %d",
				(int)(gcp->thistype));
			return TRUE;
			break;
	}

	dest = MemNew(1000);   /* temporary buffer for copies */
	if (dest == NULL)
	{
		ErrPostEx(SEV_ERROR,0,0,"ReplaceDataProc: can't allocate buffer");
		return TRUE;
	}
    
	MemCopy(dest, oldsrc, size);    /* replace the contents */
	MemCopy(oldsrc, newptr, size);
	MemCopy(newptr, dest, size);

	MemFree(dest);

	if (ompcp->output_entityID)   /* remove with objmgr? */
	{
		omdp = ObjMgrGetData (ompcp->output_entityID);
		if (omdp != NULL)
		{
			if (ObjMgrWholeEntity(omdp, ompcp->output_itemID, ompcp->output_itemtype))
				ObjMgrDelete(ompcp->output_itemtype, ompcp->output_data);
			if ((gcp->thistype == OBJ_BIOSEQ) || (gcp->thistype == OBJ_BIOSEQSET))
				sep = omdp->choice;
		}
	}

	if (sep != NULL)
		SeqEntryFree(sep);
	else
		(*(omtp->freefunc))(ompcp->output_data);

	gdsp->succeeded = TRUE;

	return TRUE;
}

static Boolean DetachDataProc (GatherContextPtr gcp)
{
	OMProcControlPtr ompcp;
	Pointer next = NULL, ptr;
	GDSPtr gdsp;

	gdsp = (GDSPtr)(gcp->userdata);
	ompcp = gdsp->ompcp;

    ptr = gcp->thisitem;
    ompcp->input_data = ptr;
    
	switch (gcp->thistype)
	{
		case OBJ_BIOSEQSET:
		case OBJ_BIOSEQ:
			if (gcp->sep != NULL)
			{
				ompcp->input_choice = gcp->sep;
				ompcp->input_choicetype = OBJ_SEQENTRY;
				next = gcp->sep->next;
			}
			break;
		case OBJ_SEQDESC:
		case OBJ_BIOSEQ_SEG:
		case OBJ_PUB:
		case OBJ_SEQFEAT_CIT:
		case OBJ_PUB_SET:
		case OBJ_SEQLOC:
		case OBJ_SEQID:
		case OBJ_SEQENTRY:
			next = (Pointer)(((ValNodePtr)(ptr))->next);
			(((ValNodePtr)(ptr))->next) = NULL;
			break;
		case OBJ_BIOSEQ_MAPFEAT:
		case OBJ_SEQFEAT:
			next = (Pointer)(((SeqFeatPtr)(ptr))->next);
			(((SeqFeatPtr)(ptr))->next) = NULL;
			break;
		case OBJ_SEQANNOT:
			next = (Pointer)(((SeqAnnotPtr)(ptr))->next);
			(((SeqAnnotPtr)(ptr))->next) = NULL;
			break;
		case OBJ_SEQALIGN:
			next = (Pointer)(((SeqAlignPtr)(ptr))->next);
			(((SeqAlignPtr)(ptr))->next) = NULL;
			break;
		case OBJ_SEQGRAPH:
			next = (Pointer)(((SeqGraphPtr)(ptr))->next);
			(((SeqGraphPtr)(ptr))->next) = NULL;
			break;
		default:
			break;
	}
	if (ompcp->whole_entity)  /* nothing to detach from */
		return TRUE;

	if (gcp->prevlink != NULL)
		*(gcp->prevlink) = next;

	ObjMgrDetach(gcp->thistype, ptr);

	gdsp->succeeded = TRUE;

	return TRUE;
}

static void AddAnAnnot(SeqAnnotPtr PNTR head, Pointer ptr)
{
	SeqAnnotPtr prev, sap;

	if (head == NULL) return;
	sap = (SeqAnnotPtr)ptr;

	if (*head == NULL)
	{
		*head = sap;
		return;
	}

	for (prev = (*head); prev->next != NULL; prev = prev->next)
		continue;

	prev->next = sap;
	return;
}

static Boolean AddToSeqAnnot (SeqAnnotPtr sap, Int2 the_type, Pointer addptr)
{
	SeqFeatPtr sfp;
	SeqAlignPtr salp;
	SeqGraphPtr sgp;
	Pointer PNTR prevlink = NULL;

	if (sap == NULL) return FALSE;

	if (sap->type == 0)
		sap->type = (Uint1)the_type;

	if (sap->type != (Uint1)the_type)
		return FALSE;

	if (sap->data == NULL)
		prevlink = &(sap->data);
	else
	{
		switch (the_type)
		{
			case 1:   /* feature table */
				for (sfp = (SeqFeatPtr)(sap->data); sfp->next != NULL; sfp = sfp->next)
					continue;
				prevlink = (Pointer PNTR)&(sfp->next);
				break;
			case 2:   /* alignments */
				for (salp = (SeqAlignPtr)(sap->data); salp->next != NULL; salp = salp->next)
					continue;
				prevlink = (Pointer PNTR)&(salp->next);
				break;
			case 3:   /* Graph */
				for (sgp = (SeqGraphPtr)(sap->data); sgp->next != NULL; sgp = sgp->next)
					continue;
				prevlink = (Pointer PNTR)&(sgp->next);
				break;
		}
	}
	if (prevlink != NULL)
		*prevlink = addptr;

	return TRUE;
}

static void AddToAnnot(SeqAnnotPtr PNTR head, Int2 the_type, Pointer addptr)
{
	SeqAnnotPtr sap, prev=NULL;

	if (head == NULL) return;

	for (sap = *head; sap != NULL; sap = sap->next)
	{
		if (sap->type == the_type)
			break;
		prev = sap;
	}

	if (sap == NULL)
	{
		sap = SeqAnnotNew();
		sap->type = (Uint1)the_type;
		if (prev != NULL)
			prev->next = sap;
		else
			*head = sap;
	}

	AddToSeqAnnot(sap, the_type, addptr);

	return;
}

static ValNodePtr PubFromDescr(ValNodePtr desc)
{
	ValNodePtr vnp2, vnp;
	PubdescPtr pdp;

	if (desc->choice != Seq_descr_pub)
		return NULL;

	pdp = (PubdescPtr)(desc->data.ptrvalue);
	vnp = pdp->pub;
	pdp->pub = NULL;

	SeqDescFree(desc);

	if (vnp == NULL) return vnp;

	if (vnp->next != NULL)
	{
		vnp2 = vnp;
		vnp = ValNodeNew(NULL);
		vnp->choice = PUB_Equiv;
		vnp->data.ptrvalue = vnp2;
	}

	return vnp;
}

static ValNodePtr DescrFromPub(ValNodePtr pub)
{
	ValNodePtr vnp;
	PubdescPtr pdp;

	pdp = PubdescNew();
	if (pub->choice == PUB_Equiv)
	{
		vnp = (ValNodePtr)(pub->data.ptrvalue);
		MemFree(pub);
		pub = vnp;
	}
	pdp->pub = pub;
	vnp = ValNodeNew(NULL);
	vnp->choice = Seq_descr_pub;
	vnp->data.ptrvalue = (Pointer)pdp;
	return vnp;
}

static Boolean AttachDataProc (GatherContextPtr gcp)
{
	OMProcControlPtr ompcp;
	Pointer ptr, newptr;
	Uint2 into, newtype;
	Boolean no_good = FALSE, into_seqentry = FALSE;
	ValNodePtr vnp;
	ObjMgrDataPtr omdp=NULL;
	ObjMgrPtr omp;
	BioseqPtr bsp;
	BioseqSetPtr bssp;
	SeqAnnotPtr sap;
	SeqFeatPtr sfp;
	SeqAlignPtr salp;
	SeqGraphPtr sgp;
	SeqSubmitPtr ssp;
	SubmitBlockPtr sbp;
	GDSPtr gdsp;

	gdsp = (GDSPtr)(gcp->userdata);
	ompcp = gdsp->ompcp;

    ptr = gcp->thisitem;   /* insert before or into this */
    ompcp->input_data = ptr;

    into = gcp->thistype;
	newtype = ompcp->output_itemtype;
	newptr = ompcp->output_data;
	omp = ObjMgrGet();
	omdp = ObjMgrFindByData(omp, newptr);

	switch (into)
	{
		case OBJ_BIOSEQSET:
		case OBJ_BIOSEQ:
			if (gcp->sep != NULL)
			{
				ompcp->input_choice = gcp->sep;
				ompcp->input_choicetype = OBJ_SEQENTRY;
				into_seqentry = TRUE;
			}
			break;
		default:
			break;
	}

	switch (into)
	{
		case OBJ_SEQSUB:
			ssp = (SeqSubmitPtr)(ptr);
			switch (newtype)
			{
				case OBJ_BIOSEQSET:
				case OBJ_BIOSEQ:
					if (omdp == NULL)
					{
						ErrPostEx(SEV_ERROR,0,0,"AttachDataProc: Not entity for Bioseq(Set)");
						return TRUE;
					}
					if (ssp->datatype == 0)
						ssp->datatype = 1;
					if (ssp->datatype != 1)
					{
						no_good = TRUE;
						break;
					}
					if (omdp->choice == NULL)
					{
						omdp->choice = ValNodeNew(NULL);
						omdp->choicetype = OBJ_SEQENTRY;
						omdp->choice->data.ptrvalue = newptr;
						if (newtype == OBJ_BIOSEQ)
							omdp->choice->choice = 1;
						else
							omdp->choice->choice = 2;
					}
					ValNodeLink((ValNodePtr PNTR)&(ssp->data), omdp->choice);
					ObjMgrConnect(newtype, newptr, into, ptr);
					break;
				case OBJ_PUB:
					vnp = (ValNodePtr)newptr;
					no_good = TRUE;
					if (vnp->choice == PUB_Sub)
					{
						if (ssp->sub == NULL)
							ssp->sub = SubmitBlockNew();
						if (ssp->sub->cit == NULL)
						{
							ssp->sub->cit = (CitSubPtr)(vnp->data.ptrvalue);
							ValNodeFree(vnp);
							no_good = FALSE;
