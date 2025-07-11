/*  lsqfetch.h
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
* File Name:  lsqfetch.h
*
* Author:  Jinghui Zhang
*
* Version Creation Date: 5/25/95
*
* $Revision: 1.1 $
*
* File Description:  Utilities for fetching local sequences
*
* Modifications:
* --------------------------------------------------------------------------
* Date     Name        Description of modification
* -------  ----------  -----------------------------------------------------
*
*
* $Log: lsqfetch.h,v $
 * Revision 1.1  1995/05/25  19:50:52  zjing
 * Initial revision
 *
*
*
* ==========================================================================
*/

#ifndef _LSQFETCH_
#define _LSQFETCH_


#ifndef _NCBI_SeqMgr_
#include <seqmgr.h>
#endif

#ifndef _NCBI_Seqloc_
#include <objloc.h>
#endif

#ifndef _NCBI_Seqset_
#include <objsset.h>    
#endif

#ifndef _NCBI_SeqUtil_
#include <sequtil.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*********************************************************************
*
*       seqid_to_string(sip, name, use_locus)
*       print the most important field in Seqid to a string stored in
*       name.
*
**********************************************************************/
Boolean seqid_to_string PROTO((SeqIdPtr sip, CharPtr name, Boolean use_locus));

/******************************************************************* 
* 
*       get_path_delimiter() 
*       return the path delimiter for different platforms 
* 
********************************************************************/ 
Char get_path_delimiter PROTO((void));


/********************************************************************** 
* 
*       FastaLibBioseqFetchEnable(libs, now)
*       Initiate the function for fetch a Bioseq from a Fasta Library 
*       file. libs is a list of library file names. 
*       If now = TRUE, open the library files and set the state to  
*       FASTALIB_OPEN. return TRUE for success.  
* 
***********************************************************************/ 
Boolean FastaLibBioseqFetchEnable PROTO((ValNodePtr libs, Boolean now));

/********************************************************************** 
* 
*       FastaLibBioseqFetchEnable(libs, now)
*       Initiate the function for fetch a Bioseq from a Fasta Library 
*       file. libs is a list of library file names. 
*       If now = TRUE, open the library files and set the state to  
*       FASTALIB_OPEN. return TRUE for success.  
* 
***********************************************************************/ 
void FastaLibBioseqFetchDisable PROTO((void));


#define BIN_ASN 1
#define TEXT_ASN 2
#define FASTA_FILE 3

/*********************************************************************
*
*       FileBioseqFetchEnable(path, ext)
*       Initiate a BioseqFetch function by either reading an ASN.1 
*       Seq-entry file or FASTA file. path->choice determines the 
*       type of the file, such as text ASN, binary ASN and FASTA file 
*       ext is the extension that is needed to add to the end of the 
*       sequence name to make the sequence file
*
*********************************************************************/
Boolean FileBioseqFetchEnable PROTO((ValNodePtr path, ValNodePtr ext));





/********************************************************************** 
* 
*       FileBioseqFetchDisable() 
*       Remove the proc associated with FileBioseqFetch and free all the  
*       sequence names in userdata 
*
***********************************************************************/ 
void FileBioseqFetchDisable PROTO((void));




/*********************************************************************
*
*       BioseqFetchInit()
*       Initiate BioseqFetch functions from local data and Entrez.
*       Local data files are stored in a config file.
*       If non is successful, return FALSE
*
*********************************************************************/
Boolean BioseqFetchInit PROTO((Boolean now));



/***********************************************************************
* 
*       BioseqFetchDisable(): Remove all the functions associated with 
*       BioseqFetch 
* 
**********************************************************************/ 
void BioseqFetchDisable PROTO((void));


/*********************************************************************
*
*	find_big_bioseq(sep)
*	return the longest Bioseq found in Seq-entry sep
*
*
**********************************************************************/
BioseqPtr find_big_bioseq PROTO((SeqEntryPtr sep));


/********************************************************************
*
*	names of config files in different platforms
*
*********************************************************************/
#ifdef WIN_MSWIN
	static CharPtr seqinfo_file = "seqinfo.dat";
#else
	static CharPtr seqinfo_file = ".seqinfo";
#endif

#ifdef __cplusplus
}
#endif

#endif
		no_good = TRUE;
					break;
			}
			break;
		case OBJ_SEQANNOT:
			sap = (SeqAnnotPtr)ptr;
			switch (newtype)
			{
				case OBJ_SEQFEAT:
					if (! AddToSeqAnnot(sap, 1, newptr))
						no_good = TRUE;
					break;
				case OBJ_SEQALIGN:
					if (! AddToSeqAnnot(sap, 2, newptr))
						no_good = TRUE;
					break;
				case OBJ_SEQGRAPH:
					if (! AddToSeqAnnot(sap, 3, newptr))
						no_good = TRUE;
					break;
				case OBJ_SEQANNOT:
					sap = (SeqAnnotPtr)newptr;
					if (gcp->prevlink != NULL)
					{
						sap->next = *(gcp->prevlink);
						*(gcp->prevlink) = sap;
					}
					else
						no_good = TRUE;
					break;
				default:
					no_good = TRUE;
					break;
			}
			break;
		case OBJ_BIOSEQ_MAPFEAT:
		case OBJ_SEQFEAT:
			sfp = (SeqFeatPtr)ptr;
			switch (newtype)
			{
				case OBJ_SEQDESC:
					newptr = (Pointer)PubFromDescr((ValNodePtr)newptr);
				case OBJ_PUB:
					if (sfp->cit == NULL)
					{
						sfp->cit = ValNodeNew(NULL);
						sfp->cit->choice = 1;
					}
					ValNodeLink((ValNodePtr PNTR)&(sfp->cit->data.ptrvalue), (ValNodePtr)newptr);
					break;
				case OBJ_SEQFEAT:
					sfp = (SeqFeatPtr)newptr;
					if (gcp->prevlink != NULL)
					{
						sfp->next = *(gcp->prevlink);
						*(gcp->prevlink) = sfp;
					}
					else
						no_good = TRUE;
					break;
				default:
					no_good = TRUE;
					break;
			}
			break;
		case OBJ_SEQALIGN:
			switch (newtype)
			{
				case OBJ_SEQALIGN:
					salp = (SeqAlignPtr)newptr;
					if (gcp->prevlink != NULL)
					{
						salp->next = *(gcp->prevlink);
						*(gcp->prevlink) = salp;
					}
					else
						no_good = TRUE;
					break;
				default:
					no_good = TRUE;
					break;
			}
			break;
		case OBJ_SEQGRAPH:
			switch (newtype)
			{
				case OBJ_SEQGRAPH:
					sgp = (SeqGraphPtr)newptr;
					if (gcp->prevlink != NULL)
					{
						sgp->next = *(gcp->prevlink);
						*(gcp->prevlink) = sgp;
					}
					else
						no_good = TRUE;
					break;
				default:
					no_good = TRUE;
					break;
			}
			break;
		case OBJ_SEQLOC:
		case OBJ_SEQID:
		case OBJ_SEQENTRY:
		default:
			no_good = TRUE;
			break;
	}

	if (no_good)
	{
		ErrPostEx(SEV_ERROR,0,0,"AttachDataProc: [%d] into [%d]", (int)newtype, (int)into);
	}
	else
	{
		gdsp->succeeded = TRUE;
		switch (newtype)
		{
			case OBJ_BIOSEQ:
			case OBJ_BIOSEQSET:
				break;
			default:
				if (omdp != NULL)
					ObjMgrDelete(omdp->datatype, omdp->dataptr);
		}
	}

	return TRUE;
}

static Boolean CopyDataProc (GatherContextPtr gcp)
{
	OMProcControlPtr ompcp;
	ObjMgrTypePtr omtp;
	ObjMgrPtr omp;
	Uint2 type;
	Pointer ptr, ptr2;
	Boolean was_choice = FALSE;
	GDSPtr gdsp;

	gdsp = (GDSPtr)(gcp->userdata);
	ompcp = gdsp->ompcp;

    ptr = gcp->thisitem;
	type = gcp->thistype;
    ompcp->input_data = ptr;
    
	switch (gcp->thistype)
	{
		case OBJ_BIOSEQSET:
		case OBJ_BIOSEQ:
			if (gcp->sep != NULL)
			{
				ompcp->input_choice = gcp->sep;
				ptr = (Pointer)(gcp->sep);
				ompcp->input_choicetype = OBJ_SEQENTRY;
				type = OBJ_SEQENTRY;
				was_choice = TRUE;
			}
			break;
		default:
			break;
	}

	omp = ObjMgrGet();
	omtp = ObjMgrTypeFind(omp, type, NULL, NULL);
	if (omtp == NULL)
	{
		ErrPostEx(SEV_ERROR,0,0,"CopyDataProc: can't find type [%d]", (int)type);
		return TRUE;
	}

	ptr2 = AsnIoMemCopy(ptr, omtp->asnread, omtp->asnwrite);
	if (ptr2 != NULL)
	{
		gdsp->succeeded = TRUE;

		if (! was_choice)
		{
			ompcp->output_data = ptr2;
			ompcp->output_itemtype = type;
		}
		else
		{
			ompcp->output_choice = ptr2;
			ompcp->output_choicetype = type;
			ompcp->output_data = ((ValNodePtr)(ptr2))->data.ptrvalue;
			ompcp->output_itemtype = gcp->thistype;
		}
		switch (ompcp->output_itemtype)
		{
			case OBJ_BIOSEQSET:    /* these types Add to the ObjMgr themselves */
			case OBJ_BIOSEQ:
			case OBJ_SEQSUB:
				break;
			default:
				ObjMgrAdd(ompcp->output_itemtype, ompcp->output_data);
				break;
		}
	}
	return TRUE;
}

static Boolean NEAR GenericGatherDataForProc (OMProcControlPtr ompcp, Boolean sel, Int2 func)
{
	ObjMgrDataPtr omdp;
	SelStructPtr ssp;
	Boolean retval = FALSE;
	static CharPtr funcs [5] = {
		"GatherDataForProc",
		"DetachDataForProc",
		"AttachDataForProc",
		"CopyDataForProc",
		"ReplaceDataForProc" };
	GatherItemProc gip;
	GDS gds;

	gds.succeeded = retval;
	gds.ompcp = ompcp;

	if (sel)
	{
		ssp = ObjMgrGetSelected();
		if (ssp == NULL)
		{
			/*
			ErrPostEx(SEV_ERROR,0,0,"%s: Nothing was selected", funcs[func]);
			*/
			return retval;
		}
		ompcp->input_entityID = ssp->entityID;
		ompcp->input_itemID = ssp->itemID;
		ompcp->input_itemtype = ssp->itemtype;
	}

	if (func == 1) /* detach: changes selection */
		ObjMgrDeSelect(ompcp->input_entityID, ompcp->input_itemID, ompcp->input_itemtype);

	omdp = ObjMgrGetData (ompcp->input_entityID);
	if (omdp == NULL)
	{
		ErrPostEx(SEV_ERROR,0,0,"%s: can't match entityID", funcs[func]);
	 	return retval;
	}
	
	if ((ompcp->input_itemID <= 1) &&
	    ((ompcp->input_itemtype == 0) || (ompcp->input_itemtype == omdp->datatype)))  /* top level */
	{
		ompcp->input_choice = omdp->choice;
		ompcp->input_choicetype = omdp->choicetype;
		ompcp->input_itemtype = omdp->datatype;
		ompcp->input_data = omdp->dataptr;
		ompcp->whole_entity = TRUE;
		if ((func == 0) || (func == 1))  /* gather or detach */
			return TRUE;
	}

	if (func == 4)    /* replace */
	{
		if (ompcp->input_itemtype != ompcp->output_itemtype)
		{
			ErrPostEx(SEV_ERROR,0,0,"%s: input type %d != output type %d",
				funcs[func], (int)(ompcp->input_itemtype), (int)(ompcp->output_itemtype));
			return FALSE;
		}

		if (ompcp->output_data == NULL)
		{
			ErrPostEx(SEV_ERROR,0,0,"%s: no output_data", funcs[func]);
			return FALSE;
		}
	}

	switch(func)
	{
		case 0:
			gip = GatherDataProc;
			break;
		case 1:
			gip = DetachDataProc;
			break;
		case 2:
			gip = AttachDataProc;
			break;
		case 3:
			gip = CopyDataProc;
			break;
		case 4:
			gip = ReplaceDataProc;
			break;

	}

	retval = GatherItem (ompcp->input_entityID, ompcp->input_itemID,
                  ompcp->input_itemtype,(Pointer)(&gds), gip);

	if (! retval)
		ErrPostEx(SEV_ERROR,0,0,"%s: can't do the gather", funcs[func]);
	else
		retval = gds.succeeded;
	return retval;
}

/****************************************************************************
*
*   GatherDataForProc(ompcp, sel)
*
*       fills in data, choice, and choictype in OMProcControlPtr
*         sets ompcp->whole_entity TRUE if appropriate
*       returns TRUE if it did it
*       if (sel == TRUE), fills in ompcp with data from ObjMgrGetSelect first.
*          returns FALSE if nothing selected.. Does ErrPostEx() for it
*
****************************************************************************/
Boolean LIBCALL GatherDataForProc (OMProcControlPtr ompcp, Boolean sel)
{
	return GenericGatherDataForProc(ompcp, sel, 0);
}

/****************************************************************************
*
*   DetachDataForProc(ompcp, sel)
*
*       fills in data, choice, and choictype in OMProcControlPtr
*         sets ompcp->whole_entity TRUE if appropriate
*       returns TRUE if it did it
*       if (sel == TRUE), fills in ompcp with data from ObjMgrGetSelect first.
*          returns FALSE if nothing selected.. Does ErrPostEx() for it
*       Detaches data item from surrounding data if necessary
*
****************************************************************************/
Boolean LIBCALL DetachDataForProc (OMProcControlPtr ompcp, Boolean sel)
{
	return GenericGatherDataForProc(ompcp, sel, 1);
}

/****************************************************************************
*
*   AttachDataForProc(ompcp, sel)
*
*       fills in data, choice, and choictype in OMProcControlPtr
*         sets ompcp->whole_entity TRUE if appropriate
*       returns TRUE if it did it
*       if (sel == TRUE), fills in ompcp with data from ObjMgrGetSelect first.
*          returns FALSE if nothing selected.. Does ErrPostEx() for it
*       Attaches data in output section of ompcp into the input section
*
****************************************************************************/
Boolean LIBCALL AttachDataForProc (OMProcControlPtr ompcp, Boolean sel)
{
	return GenericGatherDataForProc(ompcp, sel, 2);
}

/****************************************************************************
*
*   CopyDataForProc(ompcp, sel)
*
*       fills in data, choice, and choictype in OMProcControlPtr
*         sets ompcp->whole_entity TRUE if appropriate
*       returns TRUE if it did it
*       if (sel == TRUE), fills in ompcp with data from ObjMgrGetSelect first.
*          returns FALSE if nothing selected.. Does ErrPostEx() for it
*       Attaches copy of data in output section of ompcp
*
****************************************************************************/
Boolean LIBCALL CopyDataForProc (OMProcControlPtr ompcp, Boolean sel)
{
	return GenericGatherDataForProc(ompcp, sel, 3);
}

/****************************************************************************
*
*   ReplaceDataForProc(ompcp, sel)
*
*       fills in data, choice, and choictype in OMProcControlPtr
*         sets ompcp->whole_entity TRUE if appropriate
*       returns TRUE if it did it
*       if (sel == TRUE), fills in ompcp with data from ObjMgrGetSelect first.
*          returns FALSE if nothing selected.. Does ErrPostEx() for it
*       Replaces data in input section of ompcp with the output section
*       Data in input section is deleted
*
****************************************************************************/
Boolean LIBCALL ReplaceDataForProc (OMProcControlPtr ompcp, Boolean sel)
{
	return GenericGatherDataForProc(ompcp, sel, 4);
}

/*****************************************************************************
*
*   GatherProcLaunch(proctype, sel, entityID, itemID, itemtype,
*        inputtype, subinputtype, outputtype, suboutputtype)
*   	looks for proctype of highest priority that
*   		matches inputtype and outputtype
*     if (sel) then fills in entityID,itemID,itemtype with currently selected
*        item
*     else
*        uses the function arguments
*     locates the data pointer, determines the subtype (if any)
*     then finds the highest priority process
*   	0 on outputtype, inputsubtype, outputsubtype matches any
*     if subtype can be matched on input/output, takes in preference over
*        more general proc
*
*     USAGE:
*     1) To launch an editor for t