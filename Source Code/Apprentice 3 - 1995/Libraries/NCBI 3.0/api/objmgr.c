/*  objmgr.c
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
* File Name:  objmgr.c
*
* Author:  James Ostell
*   
* Version Creation Date: 9/94
*
* $Revision: 1.23 $
*
* File Description:  Manager for Bioseqs and BioseqSets
*
* Modifications:  
* --------------------------------------------------------------------------
* Date	   Name        Description of modification
* -------  ----------  -----------------------------------------------------
*
*
* $Log: objmgr.c,v $
 * Revision 1.23  1995/05/15  21:46:05  ostell
 * added Log line
 *
*
*
* ==========================================================================
*/

/** for ErrPostEx() ****/

static char *this_module = "ncbiapi";
#define THIS_MODULE this_module
static char *this_file = __FILE__;
#define THIS_FILE this_file

/**********************/

#include <objmgr.h>		   /* the interface */
#include <objsset.h>       /* temporary for caching functions */

/***
#define DEBUG_OBJMGR
***/


#ifdef DEBUG_OBJMGR
void ObjMgrDump PROTO((FILE * fp, CharPtr title));
#include <sequtil.h>
#endif

/*****************************************************************************
*
*   Data Object local functions
*
*****************************************************************************/
static Boolean NEAR ObjMgrExtend PROTO((ObjMgrPtr omp));
static ObjMgrDataPtr NEAR ObjMgrFindByEntityID PROTO((ObjMgrPtr omp, Uint2 entityID, ObjMgrDataPtr PNTR prev));


/*****************************************************************************
*
*   Procedure Management local Functions
*
*****************************************************************************/
static Boolean NEAR ObjMgrProcExtend PROTO((ObjMgrPtr omp));

/*****************************************************************************
*
*   Type Management Local functions
*
*****************************************************************************/
static Boolean NEAR ObjMgrTypeExtend PROTO((ObjMgrPtr omp));

/*****************************************************************************
*
*   Messaging/Selection local functions
*
*****************************************************************************/
static Boolean NEAR ObjMgrSendMsgFunc PROTO((ObjMgrPtr omp, ObjMgrDataPtr omdp,
				Int2 msg, Uint2 entityID, Uint2 itemID,	Uint2 itemtype));
static SelStructPtr NEAR ObjMgrAddSelStruct PROTO((ObjMgrPtr omp, Uint2 entityID, Uint2 itemID, Uint2 itemtype));
static Boolean NEAR ObjMgrDeSelectFunc PROTO((ObjMgrPtr omp, SelStructPtr ssp));
static Boolean NEAR ObjMgrDeSelectAllFunc PROTO((ObjMgrPtr omp));


/*****************************************************************************
*
*   ObjMgr Functions
*
*****************************************************************************/
ObjMgrDataPtr LIBCALL ObjMgrGetData (Uint2 entityID)
{
	ObjMgrPtr omp;
	ObjMgrDataPtr prev;

	omp = ObjMgrGet();

	return ObjMgrFindByEntityID (omp, entityID, &prev);
}

static ObjMgrDataPtr NEAR ObjMgrFindByEntityID (ObjMgrPtr omp, Uint2 entityID, ObjMgrDataPtr PNTR prev)
{
	ObjMgrDataPtr omdp, prevptr=NULL;
	ObjMgrDataPtr PNTR omdpp;
	Int2 i, imax;

	imax = omp->currobj;
	omdpp = omp->datalist;
	for (i = 0; i < imax; i++)
	{
		omdp = omdpp[i];    /* emptys always at end */
		if (omdp->parentptr == NULL)
		{
			if (omdp->EntityID == entityID)
			{
				if (prev != NULL)
					*prev = prevptr;
				return omdp;
			}
		}
	}
	return NULL;
}

ObjMgrDataPtr LIBCALL ObjMgrFindByData (ObjMgrPtr omp, Pointer ptr)
{
	ObjMgrDataPtr omdp;
	ObjMgrDataPtr PNTR omdpp;
	Int2 i, imax;
	
	if (ptr == NULL) return NULL;

	imax = omp->currobj;
	omdpp = omp->datalist;

	i = ObjMgrLookup(omp, ptr);   /* find by binary search on dataptr? */
	if (i >= 0)
		return omdpp[i];          /* found it */

	for (i = 0; i < imax; i++)
	{
		omdp = omdpp[i];    /* emptys always at end */
		if ((Pointer)(omdp->choice) == ptr)
		{
			return omdp;
		}
	}
	return NULL;
}

Uint2 LIBCALL ObjMgrAddEntityID (ObjMgrPtr omp, ObjMgrDataPtr omdp)
{
	if (omdp == NULL) return 0;

	if (omdp->EntityID)
	   return omdp->EntityID;

	omdp->EntityID = ++(omp->HighestEntityID);

#ifdef DEBUG_OBJMGR
	ObjMgrDump(NULL, "ObjMgrAddEntityID-A");
#endif

	ObjMgrSendMsgFunc(omp, omdp, OM_MSG_CREATE, omdp->EntityID, 0, 0);

#ifdef DEBUG_OBJMGR
	ObjMgrDump(NULL, "ObjMgrAddEntityID-B");
#endif

	return omdp->EntityID;
}

static Boolean NEAR ObjMgrSendMsgFunc (ObjMgrPtr omp, ObjMgrDataPtr omdp,
				Int2 msg, Uint2 entityID, Uint2 itemID,	Uint2 itemtype)
{
	OMUserDataPtr prev=NULL, curr, next, PNTR root;
	Int2 retval;
	Int2 ctr = 2;

	curr = omp->userdata;
	root = &(omp->userdata);
	while (ctr)
	{
		prev = NULL;
		while (curr != NULL)
		{
			next = curr->next;
			if (curr->messagefunc != NULL)
			{
				retval = (* (curr->messagefunc))(msg, entityID, itemID, itemtype, (Pointer)curr);
				if (retval == OM_MSG_RET_ERROR)
					ErrShow();
				else if (retval == OM_MSG_RET_DEL)
				{
					if (prev == NULL)
						*root = next;
					else
						prev->next = next;
					if (curr->freefunc != NULL)
						(* (curr->freefunc))(curr->userdata.ptrvalue);
					MemFree(curr);
				}
				else if (retval == OM_MSG_RET_DONE)
					return TRUE;
			}
			curr = next;
		}

		if (omdp == NULL)
			ctr -= 2;
		else if (ctr == 2)
		{
			ctr--;
			curr = omdp->userdata;
			root = &(omdp->userdata);
		}
		else
			ctr--;
	}
	return TRUE;

}

/*****************************************************************************
*
*   Return the current ObjMgr
*   	Initialize if not done already
*
*****************************************************************************/
ObjMgrPtr LIBCALL ObjMgrGet (void)
{
	static char * ObjMgrStr = "NCBIObjMgr";
	ObjMgrPtr omp=NULL;

	omp = (ObjMgrPtr) GetAppProperty(ObjMgrStr);
	if (omp != NULL)
		return omp;

	                             /*** have to initialize it **/
	omp = (ObjMgrPtr) MemNew (sizeof(ObjMgr));
	omp->maxtemp = DEFAULT_MAXTEMP;

	SetAppProperty (ObjMgrStr, (void *) omp);
	return omp;
}


/*****************************************************************************
*
*   Data Management Functions
*
*****************************************************************************/


/*****************************************************************************
*
*   ObjMgrExtend(omp)
*
*****************************************************************************/
static Boolean NEAR ObjMgrExtend (ObjMgrPtr omp)
{
	Boolean result = FALSE;
	OMDataPtr omdp, prev=NULL;
	ObjMgrDataPtr PNTR tmp;
	Int2 i, j;

	for (omdp = omp->ncbidata; omdp != NULL; omdp = omdp->next)
		prev = omdp;

	omdp = (OMDataPtr)MemNew(sizeof(OMData));
	if (omdp == NULL) return result;
	tmp = (ObjMgrDataPtr PNTR)MemNew((size_t)(sizeof(ObjMgrDataPtr) * (omp->totobj + NUM_OMD)));
	if (tmp == NULL)
	{
		MemFree(omdp);
		return result;
	}

	if (prev != NULL)
	{
		prev->next = omdp;
		MemMove(tmp, omp->datalist, (size_t)(sizeof(ObjMgrDataPtr) * omp->totobj));
		MemFree(omp->datalist);
	}
	else
		omp->ncbidata = omdp;

	j = omp->totobj;

	for (i = 0; i < NUM_OMD; i++, j++)
		tmp[j] = &(omdp->data[i]);

	omp->totobj += NUM_OMD;
	omp->datalist = tmp;

	result = TRUE;
	return result;
}

/*****************************************************************************
*
*   ObjMgrAdd(type, data)
*   	adds a pointer (data) of type (type) to the sequence manager
*
*****************************************************************************/
Boolean LIBCALL ObjMgrAdd (Uint2 type, Pointer data)
{
	ObjMgrPtr omp;
	ObjMgrDataPtr omdp;
	ObjMgrDataPtr PNTR omdpp;
	Int2 i, imin, imax;
	unsigned long tmp, datai;
#ifdef DEBUG_OBJMGR
	FILE * fp;

	fp = FileOpen("ObjMgr.log", "a");
#endif

	omp = ObjMgrGet();

	if (omp->currobj >= omp->totobj)
	{
		if (! ObjMgrExtend(omp))
			return FALSE;
	}

	i = omp->currobj;
	omdpp = omp->datalist;
	omdp = omdpp[i];    /* emptys always at end */

	imin = 0;                   /* find where it goes */
	imax = omp->currobj-1;

	datai = (unsigned long)data;

	if ((i) && (datai < (unsigned long)(omdpp[imax]->dataptr)))
	{
		i = (imax + imin) / 2;
		while (imax > imin)
		{
			tmp = (unsigned long)(omdpp[i]->dataptr);
#ifdef DEBUG_OBJMGR
			fprintf(fp, "Sort: i=%d tmp=%ld data=%ld imax=%d imin=%d\n",
				(int)i, (long)tmp, (long)data, (int)imax, (int)imin);
#endif
			if (tmp > datai)
				imax = i - 1;
			else if (tmp < datai)
				imin = i + 1;
			else
				break;
			i = (imax + imin)/2;
		}

#ifdef DEBUG_OBJMGR
			fprintf(fp, "End: i=%d tmp=%ld data=%ld imax=%d imin=%d\n",
				(int)i, (long)tmp, (long)data, (int)imax, (int)imin);
#endif
		if (datai > (unsigned long)(omdpp[i]->dataptr)) /* check for off by 1 */
		{
			i++;
#ifdef DEBUG_OBJMGR
			fprintf(fp, "Adjust: i=%d\n", (int)i);
#endif
		}


		imax = omp->currobj - 1;	 /* open the array */
		while (imax >= i)
		{
			omdpp[imax+1] = omdpp[imax];
			imax--;
		}
	}

	omdpp[i] = omdp;    /* put in the pointer in order */
	omp->currobj++;     /* got one more */

	omdp->dataptr = data;  /* fill in the values */
	omdp->datatype = type;
	omdp->touch = GetSecs();   /* stamp with time */

#ifdef DEBUG_OBJMGR
	FileClose(fp);
	ObjMgrDump(NULL, "ObjMgrAdd");
#endif

	return TRUE;
}

/*****************************************************************************
*
*   ObjMgrRegister (datatype, data)
*   	datatype is the datatype of data to register
*   	if data is already registered in ObjMgr, returns entityID
*   	if not, is added to the ObjMgr, returns entityID
*
*   	if (datatype is a choice type, uses data as a ValNodePtr)
*   
*   	on failure returns 0
*
*****************************************************************************/
Uint2 LIBCALL ObjMgrRegister (Uint2 datatype, Pointer data)
{
	Uint2 dtype;
	ObjMgrPtr omp;
	ObjMgrDataPtr omdp;
	ObjMgrTypePtr omtp;
	Pointer ptr;


	if (data == NULL) return 0;

	dtype = datatype;  /* sets the default state */
	ptr = data;

	omp = ObjMgrGet();

	omdp = ObjMgrFindByData(omp, data);  /* already have it? */
	if (omdp != NULL)
	{
		if (! omdp->EntityID)
		{
			if (omdp->parentptr != NULL)
				ErrPostEx(SEV_ERROR,0,0, "ObjMgrRegister: parent != NULL");

			ObjMgrAddEntityID(omp, omdp);
		}

		return omdp->EntityID;
	}
								/* have to add it to ObjMgr */
	omtp = ObjMgrTypeFind(omp, datatype, NULL, NULL);
	if (omtp == NULL)
	{
		ErrPostEx(SEV_ERROR,0,0, "ObjMgrRegister: invalid data type [%d]", (int)datatype);
		return 0;
	}
      
      /*** this was to register things like SeqEntry.. currently we register data.ptrvalue directly **
      
	if (omtp->fromchoicefunc == NULL)
	{
		dtype = datatype;
		ptr = data;
	}
	else
	{
		vnp = (ValNodePtr)data;
		dtype = (*(omtp->fromchoicefunc))(vnp);
		ptr = vnp->data.ptrvalue;
	}

      ***********************************************************************************************/
      
	ObjMgrAdd(dtype, ptr);
	omdp = ObjMgrFindByData(omp, ptr);
	if (omdp == NULL) return 0;

	ObjMgrAddEntityID(omp, omdp);

	return omdp->EntityID;
}


/*****************************************************************************
*
*   ObjMgrAddUserData(entityID, procid, proctype, userkey)
*   	creates a new OMUserData struct attached to entityID
*   	if entityID = 0, attaches to the desktop (all objects)
*   	Caller must fill in returned structure
*   	returns NULL on failure
*
*****************************************************************************/
OMUserDataPtr LIBCALL ObjMgrAddUserData (Uint2 entityID, Uint2 procid, Uint2 proctype, Uint2 userkey)
{
	OMUserDataPtr omudp=NULL, tmp;
	ObjMgrPtr omp;
	ObjMgrDataPtr omdp;

	omp = ObjMgrGet();

	omudp = MemNew(sizeof(OMUserData));
	omudp->procid = procid;
	omudp->proctype = proctype;
	omudp->userkey = userkey;

	if (entityID == 0)
	{
		if (omp->userdata == NULL)
			omp->userdata = omudp;
		else
		{
			for (tmp = omp->userdata; tmp->next != NULL; tmp = tmp->next)
				continue;
			tmp->next = omudp;
		}
	}
	else
	{
		omdp = ObjMgrFindByEntityID(omp, entityID, NULL);
		if (omdp == NULL)
			omudp = MemFree(omudp);
		else
		{
			if (omdp->userdata == NULL)
				omdp->userdata = omudp;
			else
			{
				for (tmp = omdp->userdata; tmp->next != NULL; tmp = tmp->next)
					continue;
				tmp->next = omudp;

			}
		}
	}

	return omudp;
}

/*****************************************************************************
*
*   ObjMgrGetUserData(entityID, procid, proctype, userkey)
*   	Finds an OMUserData struct attached to entityID
*   	if entityID = 0, gets from the desktop
*   	returns NULL on failure
*
*****************************************************************************/
OMUserDataPtr LIBCALL ObjMgrGetUserData (Uint2 entityID, Uint2 procid, Uint2 proctype, Uint2 userkey)
{
	OMUserDataPtr omudp=NULL;
	ObjMgrPtr omp;
	ObjMgrDataPtr omdp;

	omp = ObjMgrGet();

	if (entityID == 0)
	{
		omudp = omp->userdata;
	}
	else
	{
		omdp = ObjMgrFindByEntityID(omp, entityID, NULL);
		if (omdp != NULL)
			omudp = omdp->userdata;
	}

	while (omudp != NULL)
	{
		if ((omudp->procid == procid) || (! procid))
		{
			if ((omudp->proctype == proctype) || (! proctype))
			{
				if ((omudp->userkey == userkey) || (! userkey))
					break;
			}
		}
		omudp = omudp->next;
	}

	return omudp;
}
/*****************************************************************************
*
*   ObjMgrFreeUserData(entityID, procid, proctype, userkey)
*   	frees OMUserData attached to entityID by procid
*       if procid ==0, frees all OMUserData of proctype
*   	if proctype ==0, matches any proctype
*       if userkey == matches any userkey
*   	returns TRUE if any freed
*
*****************************************************************************/
Boolean LIBCALL ObjMgrFreeUserData (Uint2 entityID, Uint2 procid, Uint2 proctype, Uint2 userkey)
{
	OMUserDataPtr omudp=NULL, prev, next;
	ObjMgrPtr omp;
	ObjMgrDataPtr omdp;
	Boolean got_one = FALSE;

	omp = ObjMgrGet();

	if (entityID == 0)
	{
		omudp = omp->userdata;
	}
	else
	{
		omdp = ObjMgrFindByEntityID(omp, entityID, NULL);
		if (omdp != NULL)
			omudp = omdp->userdata;
	}

	prev = NULL;
	while (omudp != NULL)
	{
		next = omudp->next;
		if ((omudp->procid == procid) || (! procid))
		{
			if ((omudp->proctype == proctype) || (! proctype))
			{
				if ((omudp->userkey == userkey) || (! userkey))
				{
					got_one = TRUE;
					if (prev == NULL)
					{
						if (entityID == 0)
							omp->userdata = next;
						else
							omdp->userdata = next;
					}
					else
						prev->next = next;
					if (omudp->freefunc != NULL)
						(* (omudp->freefunc))(omudp->userdata.ptrvalue);
					omudp = MemFree(omudp);
				}
			}
		}
		if (omudp != NULL)
			prev = omudp;
		omudp = next;
	}

	return got_one;
}


/*****************************************************************************
*
*   ObjMgrLookup(omp, data)
*   	Binary lookup of data in omp->datalist
*   	returns index (>=0) if found
*       returns -1 if not found
*
*****************************************************************************/
Int2 LIBCALL ObjMgrLookup(ObjMgrPtr omp, Pointer data)
{
	Int2 imin, imax, i;
	ObjMgrDataPtr PNTR omdpp;
	unsigned long tmp, datai;

	imin = 0;
	imax = omp->currobj - 1;
	omdpp = omp->datalist;

	datai = (unsigned long)data;

	while (imax >= imin)
	{
		i = (imax + imin)/2;
		tmp = (unsigned long)(omdpp[i]->dataptr);
		if (tmp > datai)
			imax = i - 1;
		else if (tmp < datai)
			imin = i + 1;
		else
			return i;
	}

	return (Int2)(-1);
}

/*****************************************************************************
*
*   ObjMgrDelete(type, data)
*   	deletes a pointer (data) of type (type) to the sequence manager
*
*****************************************************************************/
Boolean LIBCALL ObjMgrDelete (Uint2 type, Pointer data)
{
	ObjMgrPtr omp;
	ObjMgrDataPtr omdp;
	ObjMgrDataPtr PNTR omdpp, PNTR to, PNTR from;
	Int2 i, j;

	omp = ObjMgrGet();

	i = ObjMgrLookup(omp, data);
	if (i < 0)  /* not found */
	{
	   /***	may not be registered with objmgr ***
		ErrPostEx(SEV_ERROR, 0,0, "ObjMgrDelete: pointer [%ld] type [%d] not found",
			(long)data, (int)type);
		***/
		return FALSE;
	}

	omdpp = omp->datalist;
	omdp = omdpp[i];    /* emptys always at end */

	if (omdp->EntityID != 0)
	{
		ObjMgrSendMsgFunc(omp, omdp, OM_MSG_DEL, omdp->EntityID, 0, 0);
		ObjMgrDeSelect(omdp->EntityID,0,0);
		ObjMgrFreeUserData(omdp->EntityID, 0, 0, 0);
	}

	if (omdp->clipboard)    /* update the clipboard */
		omp->clipboard = NULL;

	if (omdp->lockcnt)
		ErrPostEx(SEV_ERROR, 0,0,"ObjMgrDelete: deleting locked element");
	else if (omdp->tempload)
	{
		if (omp->tempcnt)
			omp->tempcnt--;
		else
			ErrPostEx(SEV_ERROR, 0,0, "ObjMgrDelete: reducing tempcnt below 0");
	}

	if (omdp->EntityID != 0 && omdp->EntityID == omp->HighestEntityID)
		omp->HighestEntityID--;

	MemSet(omdp, 0, sizeof(ObjMgrData));
	omp->currobj--;
	j = omp->currobj - i;
	if (j)
	{
		to = omdpp + i;
		from = to + 1;
		MemMove(to, from, (size_t)(sizeof(ObjMgrDataPtr) * j));
	}
	omdpp[omp->currobj] = omdp;  /* put in pointer to empty data space */

#ifdef DEBUG_OBJMGR
	ObjMgrDump(NULL, "ObjMgrDelete");
#endif

	return TRUE;
}

/*****************************************************************************
*
*   ObjMgrGetClipBoard()
*     returns ObjMgrDataPtr to current clipboard object or NULL if none
*
*****************************************************************************/
ObjMgrDataPtr LIBCALL ObjMgrGetClipBoard (void)
{
	ObjMgrPtr omp;
	
	omp = ObjMgrGet();
	return omp->clipboard;
}

/*****************************************************************************
*
*   ObjMgrAddToClipBoard(entityID, ptr)
*   	if entityID > 0, then uses it.
*   	else, looks up entityID using ptr
*       adds entityID if needed
*       sends OM_MSG_TO_CLIPBOARD
*
*   	Anything in the clipboard is deleted
*
*****************************************************************************/
Boolean LIBCALL ObjMgrAddToClipBoard (Uint2 entityID, Pointer ptr)
{
	ObjMgrPtr omp;
	ObjMgrDataPtr omdp;

	omp = ObjMgrGet();
	if (! entityID)
	{
		omdp = ObjMgrFindByData(omp, ptr);
		if (omdp != NULL)
		{
			if (omdp->parentptr != NULL)
			{
				ErrPostEx(SEV_ERROR,0,0,"AddToClipBoard: ParentPtr != NULL");
				return FALSE;
			}
			if (omdp->EntityID == 0)
				omdp->EntityID = ++(omp->HighestEntityID);
		}
	}
	else
	{
		omdp = ObjMgrFindByEntityID(omp, entityID, NULL);
	}

	if (omdp == NULL)
	{
		ErrPostEx(SEV_ERROR,0,0,"AddToClipBoard: data not found");
		return FALSE;
	}

	ObjMgrFreeClipBoard();

	omdp->clipboard = TRUE;
	omp->clipboard = omdp;

	ObjMgrSendMsgFunc(omp, omdp, OM_MSG_TO_CLIPBOARD, omdp->EntityID, 0, 0);

	return TRUE;
}

/*****************************************************************************
*
*   ObjMgrFreeClipBoard()
*     clears any data from the clipboard
*
*****************************************************************************/
Boolean LIBCALL ObjMgrFreeClipBoard (void)
{
	ObjMgrPtr omp;
	ObjMgrTypePtr omtp;
	ObjMgrDataPtr omdp;
	Uint2 type;
	Pointer ptr;
	
	omp = ObjMgrGet();
	omdp = omp->clipboard;
	if (omdp == NULL) return TRUE;

	if (omdp->choice != NULL)
	{
		type = omdp->choicetype;
		ptr = omdp->choice;
	}
	else
	{
		type = omdp->datatype;
		ptr = omdp->dataptr;
	}

	omtp = ObjMgrTypeFind(omp, type, NULL, NULL);
	if (omtp == NULL)
	{
		ErrPostEx(SEV_ERROR,0,0,"ObjMgrFreeClipBoard: cant find type [%d]", (int)type);
		return FALSE;
	}
	else
		(*(omtp->freefunc))(ptr);
	ObjMgrDelete(omdp->datatype, omdp->dataptr);
	omp->clipboard = NULL;
	return TRUE;
}

/*****************************************************************************
*
*   ObjMgrConnect (type, data, parenttype, parentdata)
*   	Adds parent info to element
*       Updates EntityID
*   		if both are 0, assigns it by incrementing HighestEntityID
*   		if one is 0, assigns it the other
*           if neither is 0 and not the same ID
*                assigns parent to child (and cascades to its children)
*
*****************************************************************************/
Boolean LIBCALL ObjMgrConnect (Uint2 type, Pointer data, Uint2 parenttype, Pointer parentdata)
{
	ObjMgrPtr omp;
	Int2 i;
	ObjMgrDataPtr omdp;
	
	omp = ObjMgrGet();

	i = ObjMgrLookup(omp, data);
	if (i < 0)  /* not found */
	{
		ErrPostEx(SEV_ERROR, 0,0, "ObjMgrConnect: pointer [%ld] type [%d] not found",
			(long)data, (int)type);
		return FALSE;
	}

	omdp = omp->datalist[i];
	omdp->parenttype = parenttype;
	omdp->parentptr = parentdata;

	if ((omdp->EntityID != 0) && (parentdata != NULL))
	{                         /* registered type, inform any attached procs */

		ObjMgrSendMsg(OM_MSG_CONNECT, omdp->EntityID, 0,0);
		ObjMgrDeSelect(omdp->EntityID,0,0);
		ObjMgrFreeUserData(omdp->EntityID, 0, 0, 0);

		omdp->EntityID = 0;   /* reset.. now has a parent */
	}

	if (parentdata != NULL)
	{
		i = ObjMgrLookup(omp, parentdata);
		if (i < 0)  /* not found */
		{
			ErrPostEx(SEV_ERROR, 0,0, "ObjMgrConnect: parent pointer [%ld] type [%d] not found",
				(long)parentdata, (int)parenttype);
			return FALSE;
		}

		
	}

#ifdef DEBUG_OBJMGR
	ObjMgrDump(NULL, "ObjMgrConnect");
#endif

	return TRUE;
}

/*****************************************************************************
*
*   ObjMgrDetach (type, data)
*   	Removes parent info from element
*       Adds to objmgr if necessary
*       Does NOT register entity
*
*****************************************************************************/
Boolean LIBCALL ObjMgrDetach (Uint2 type, Pointer data)
{
	ObjMgrPtr omp;
	Int2 i;
	ObjMgrDataPtr omdp;
	
	omp = ObjMgrGet();

	i = ObjMgrLookup(omp, data);
	if (i < 0)  /* not found */
	{
		return ObjMgrAdd(type, data);
	}

	omdp = omp->datalist[i];
	if (omdp->parentptr == NULL)  /* not connected anyway */
		return TRUE;

	omdp->parenttype = 0;
	omdp->parentptr = NULL;
	omdp->EntityID = 0;   /* reset.. now has a parent */

	return TRUE;
}

/*****************************************************************************
*
*   ObjMgrSetChoice(type, choice, data)
*   	Adds the ValNodePtr pointing directly to this Bioseq or BioseqSet
*
*****************************************************************************/
Boolean LIBCALL ObjMgrSetChoice (Uint2 type, ValNodePtr choice, Pointer data)
{
	ObjMgrPtr omp;
	Int2 i;
	ObjMgrDataPtr omdp;
	
	omp = ObjMgrGet();

	i = ObjMgrLookup(omp, data);
	if (i < 0)  /* not found */
	{
		ErrPostEx(SEV_ERROR, 0,0, "ObjMgrChoice: pointer [%ld] type [%d] not found",
			(long)data, (int)type);
		return FALSE;
	}

	omdp = omp->datalist[i];
	omdp->choicetype = type;
	omdp->choice = choice;

#ifdef DEBUG_OBJMGR
	ObjMgrDump(NULL, "ObjMgrChoice");
#endif

	return TRUE;
}

/*****************************************************************************
*
*   ObjMgrFindTop(omp, omdp)
*   	finds the highest parent of omdp
*       returns a ObjMgrDataPtr to the top
*
*****************************************************************************/
ObjMgrDataPtr LIBCALL ObjMgrFindTop (ObjMgrPtr omp, ObjMgrDataPtr omdp)
{
	Int2 i;

	if ((omp == NULL) || (omdp == NULL)) return NULL;

	while (omdp->parentptr != NULL)
	{
		i = ObjMgrLookup(omp, omdp->parentptr);
		if (i < 0)  /* not found */
		{
			ErrPostEx(SEV_ERROR, 0,0, "ObjMgrFindTop: parentptr [%ld] not found",
				(long)(omdp->parentptr));
			return NULL;
		}
		omdp = omp->datalist[i];
	}

	return omdp;
}

/*****************************************************************************
*
*   ObjMgrWholeEntity(omdp, itemID, itemtype)
*		returns TRUE if itemID, itemtype identify a complete entity omdp
*       returns FALSE if these are an internal part of the entity
*
*****************************************************************************/
Boolean LIBCALL ObjMgrWholeEntity (ObjMgrDataPtr omdp, Uint2 itemID, Uint2 itemtype)
{
	if (omdp == NULL)
		return FALSE;

	if ((itemID == 0) && (itemtype == 0))
		return TRUE;

	if ((itemID == 1) && (itemtype == omdp->datatype))
		return TRUE;

	return FALSE;

}

/*****************************************************************************
*
*   ObjMgrFreeCache(type)
*   	Frees all cached objects of type and subtypes of type
*   		based on ObjMgrMatch()
*   	if type == 0, frees all cached objects
*   	returns TRUE if no errors occured
*
*****************************************************************************/
Boolean LIBCALL ObjMgrFreeCache (Uint2 type)
{
    Int2 i, num;
	ObjMgrPtr omp;
	ObjMgrDataPtr PNTR omdpp, omdp;
	Boolean result = TRUE;

	omp = ObjMgrGet();

	omdpp = omp->datalist;
	num = omp->currobj;
    for (i = 0; i < num; i++)
    {
		omdp = omdpp[i];
		if ((omdp->parentptr == NULL) &&     /* top level */
			(omdp->tempload == TL_CACHED))   /* cached */
		{
			if ((! type) ||
				(ObjMgrMatch(type, omdp->datatype)) ||
				(ObjMgrMatch(type, omdp->choicetype)))
			{
				/** here is where the freefunc should be called **/

				if (omdp->choice != NULL)
				{
					switch (omdp->choicetype)
					{
						case OBJ_SEQENTRY:
							SeqEntryFree(omdp->choice);
							break;
						default:
							ErrPostEx(SEV_ERROR,0,0, "ObjMgrFreeCache: unsupported choicetype[%d]",
								(int)(omdp->choicetype));
							result = FALSE;
							break;
					}
				}
				else
				{
					switch(omdp->datatype)
					{
						case OBJ_BIOSEQ:
							BioseqFree((BioseqPtr)(omdp->dataptr));
							break;
						case OBJ_BIOSEQSET:
							BioseqSetFree((BioseqSetPtr)(omdp->dataptr));
							break;
						default:
							ErrPostEx(SEV_ERROR,0,0,"ObjMgrFreeCache: usupported datatype [%d]",
								(int)(omdp->datatype));
							result = FALSE;
							break;
					}
				}
			}
		}
    }
    return result;
	
}

/*****************************************************************************
*
*   ObjMgrMatch(type1, type2)
*   	returns 0 if no match
*   	1 if identical
*   	2 if 2 is a subset of 1   (e.g. 1=OBJ_SEQENTRY, 2=BIOSEQ)
*       current type1 that can have subtypes are:
*   		OBJ_SEQENTRY
*   		OBJ_PUB
*   		OBJ_SEQANNOT
*   		OBJ_SEQCODE_SET
*   		OBJ_GENETIC_CODE_SET
*
*****************************************************************************/
Int2 LIBCALL ObjMgrMatch (Uint2 type1, Uint2 type2)
{
	if (type1 == type2)
		return 1;

	switch (type1)
	{
		case OBJ_SEQENTRY:
			switch (type2)
			{
				case OBJ_BIOSEQ:
				case OBJ_BIOSEQSET:
					return 2;
			}
			break;
		case OBJ_SEQANNOT:
			switch (type2)
			{
				case OBJ_SEQFEAT:
				case OBJ_SEQALIGN:
				case OBJ_SEQGRAPH:
					return 2;
			}
			break;
		case OBJ_SEQCODE_SET:
			switch (type2)
			{
				case OBJ_SEQCODE:
					return 2;
			}
			break;
		case OBJ_GENETIC_CODE_SET:
			switch (type2)
			{
				case OBJ_GENETIC_CODE:
					return 2;
			}
			break;
		default:
			break;
	}

	return 0;
}


/*****************************************************************************
*
*   ObjMgrGetChoiceForData(data)
*   	returns ValNodePtr for a BioseqPtr or BioseqSetPtr
*       choice must have been put in ObjMgr using ObjMgrChoice
*       the Bioseq/BioseqSets it is a part of must also be in ObjMgr
*       returns NULL on failure.
*
*****************************************************************************/
ValNodePtr LIBCALL ObjMgrGetChoiceForData (Pointer data)
{
	ObjMgrPtr omp;
	Int2 i;
	ValNodePtr choice = NULL;

	if (data == NULL) return choice;
	omp = ObjMgrGet();
	i = ObjMgrLookup(omp, data);
	if (i < 0)
	{
		ErrPostEx(SEV_ERROR, 0,0, "ChoiceGetChoiceForData: data not in ObjMgr");
		return choice;
	}
	return omp->datalist[i]->choice;
}

/*****************************************************************************
*
*   ObjMgrGetEntityIDForChoice(choice)
*   	returns the EntityID for a ValNodePtr
*       choice must have been put in ObjMgr using ObjMgrChoice
*       the Bioseq/BioseqSets it is a part of must also be in ObjMgr
*       This function will move up to the top of the Choice tree it may be
*          in. If top level EntityID is 0, one is assigned at this point.
*       If an element is moved under a different hierarchy, its EntityID will
*          change.
*       returns 0 on failure.
*
*****************************************************************************/
Uint2 LIBCALL ObjMgrGetEntityIDForChoice (ValNodePtr choice)
{
	Pointer data;

	if (choice == NULL) return 0;
	data = choice->data.ptrvalue;
	if (data == NULL) return 0;

	return ObjMgrGetEntityIDForPointer (data);
}

/*****************************************************************************
*
*   ObjMgrGetEntityIDForPointer(data)
*   	returns the EntityID for any pointer, Choice or Data
*       This function will move up to the top of the tree it may be
*          in. If top level EntityID is 0, one is assigned at this point.
*       If an element is moved under a different hierarchy, its EntityID will
*          change.
*
*       Either ObjMgrGetEntityIDForPointer() or ObjMgrGetEntityIDForChoice()
*   		MUST be called to have an OM_MSG_CREATE message sent to any
*           registered proceedures
*   
*       returns 0 on failure.
*
*****************************************************************************/
Uint2 LIBCALL ObjMgrGetEntityIDForPointer (Pointer ptr)
{
	ObjMgrPtr omp;
	ObjMgrDataPtr omdp;

	if (ptr == NULL)
		return 0;

	omp = ObjMgrGet();
	omdp = ObjMgrFindByData(omp, ptr);

	if (omdp == NULL) return 0;

	omdp = ObjMgrFindTop(omp, omdp);
	if (omdp == NULL) return 0;

	if (omdp->EntityID == 0)     /* need to assign it */
		ObjMgrAddEntityID(omp, omdp);

	return omdp->EntityID;
}

/*****************************************************************************
*
*   ObjMgrGetChoiceForEntityID (id)
*
*****************************************************************************/
ValNodePtr LIBCALL ObjMgrGetChoiceForEntityID (Uint2 id)
{
	ObjMgrPtr omp;
	ObjMgrDataPtr omdp;

	if (id <= 0) return NULL;

	omp = ObjMgrGet();
	omdp = ObjMgrFindByEntityID(omp, id, NULL);
	if (omdp != NULL)
		return omdp->choice;
	return NULL;
}

/*****************************************************************************
*
*   ObjMgrLock(type, data, lockit)
*   	if lockit=TRUE, locks the element
*       else unlocks it
*       returns the current lock count or -1 on failure
*
*****************************************************************************/
Int2 LIBCALL ObjMgrLock (Uint2 type, Pointer data, Boolean lockit)
{
	ObjMgrPtr omp;
	
	omp = ObjMgrGet();
	if (omp != NULL)
		return ObjMgrLockFunc(omp, type, data, lockit);
	else
		return -1;
}

Int2 LIBCALL ObjMgrLockFunc (ObjMgrPtr omp, Uint2 type, Pointer data, Boolean lockit)
{
	Int2 i, lockcnt = -1;
	ObjMgrDataPtr omdp;

#ifdef DEBUG_OBJMGR
	Char buf[80];
#endif

	i = ObjMgrLookup(omp, data);
	if (i < 0)  /* not found */
	{
		ErrPostEx(SEV_ERROR, 0,0, "ObjMgrLock: pointer [%ld] type [%d] not found",
			(long)data, (int)type);
		return lockcnt;
	}

	omdp = ObjMgrFindTop(omp, omp->datalist[i]);

	if (lockit) {
		omdp->lockcnt++;
	}
	else
	{
		if (omdp->lockcnt)
			omdp->lockcnt--;
		else
		{
			ErrPostEx(SEV_ERROR, 0,0,"ObjMgrLock: unlocking 0 lockcnt");
			return lockcnt;
		}
	}

	lockcnt = omdp->lockcnt;

	if (! lockit)     /* check for automatic delete */
	{
		if ((omdp->tempload != TL_NOT_TEMP) && (! omdp->lockcnt))
		{
			omdp->touch = GetSecs();   /* stamp with time */
			/*
			omp->tempcnt++;
			*/
			ObjMgrReap(omp);
		}
	}


#ifdef DEBUG_OBJMGR
	if (lockit)
		sprintf(buf, "ObjMgrLock   Lock [%d]", (int)i);
	else
		sprintf(buf, "ObjMgrLock   Unlock [%d]", (int)i);
	ObjMgrDump(NULL, buf);
#endif

	return lockcnt;
}

/*****************************************************************************
*
*   Boolean ObjMgrSetTempLoad (ObjMgrPtr omp, Pointer ptr)
*   Sets a Loaded element to "temporary"
*
*****************************************************************************/
Boolean LIBCALL ObjMgrSetTempLoad (ObjMgrPtr omp, Pointer ptr)
{
	Int2 index;
	ObjMgrDataPtr omdp;

	index = ObjMgrLookup(omp, ptr);
	if (index <= 0) return FALSE;

	omdp = ObjMgrFindTop(omp, omp->datalist[index]);
	if (omdp == NULL) return FALSE;

	if (omdp->tempload == TL_NOT_TEMP)
	{
		omdp->tempload = TL_LOADED;
		omp->tempcnt++;
	}
	omdp->touch = GetSecs();
	ObjMgrReap (omp);   /* check to see if we need to reap */
	return TRUE;
}

/*****************************************************************************
*
*   ObjMgrReap(omp)
*   	Checks to see if memory needs to be cleared, and does it
*
*****************************************************************************/
Boolean LIBCALL ObjMgrReap (ObjMgrPtr omp)
{
	time_t lowest;
	Int2 num, j;
	ObjMgrDataPtr tmp, ditch, PNTR omdpp;

	while (omp->tempcnt > omp->maxtemp)   /* time to reap */
	{
		lowest = (time_t) UINT4_MAX;
		num = omp->currobj;
		omdpp = omp->datalist;
		ditch = NULL;
		for (j = 0; j < num; j++, omdpp++)
		{
			tmp = *omdpp;
			if ((tmp->tempload == TL_LOADED) && (! tmp->lockcnt))
			{
				if (lowest > tmp->touch)
				{
					lowest = tmp->touch;
					ditch = tmp;
				}
			}
		}
		if (ditch == NULL)    /* nothing to free */
			return FALSE;

		ditch->tempload = TL_CACHED;
		ObjMgrSendMsgFunc(omp, ditch, OM_MSG_CACHED, ditch->EntityID, 0, 0);
		omp->tempcnt--;

		if (ditch->choice != NULL)
		{
			switch (ditch->choicetype)
			{
				case OBJ_SEQENTRY:
					SeqEntryFreeComponents(ditch->choice);
					break;
			}
		}
		else
		{
			switch (ditch->datatype)
			{
				case OBJ_BIOSEQ:
					BioseqFreeComponents((BioseqPtr)(ditch->dataptr));
					break;
				case OBJ_BIOSEQSET:
					BioseqSetFreeComponents((BioseqSetPtr)(ditch->dataptr), FALSE);
					break;
				default:
					ErrPostEx(SEV_ERROR,0,0,"ObjMgrUnlock: ditching unknown type");
					break;
			}
		}
		

#ifdef DEBUG_OBJMGR
	ObjMgrDump(NULL, "ObjMgrReap");
#endif

	}

	return TRUE;
}

/*****************************************************************************
*
*   Boolean ObjMgrIsTemp(data)
*   	returns TRUE if data is a temporarily loaded item
*       data must be BioseqPtr or BioseqSetPtr
*
*****************************************************************************/
Boolean LIBCALL ObjMgrIsTemp (Pointer data)
{
	ObjMgrPtr omp;
	Int2 i;
	ObjMgrDataPtr omdp;
	
	omp = ObjMgrGet();

	i = ObjMgrLookup(omp, data);
	if (i < 0)  /* not found */
	{
		ErrPostEx(SEV_ERROR, 0,0, "ObjMgrIsTemp: pointer [%ld] not found",
			(long)data);
		return FALSE;
	}

	omdp = ObjMgrFindTop(omp, omp->datalist[i]);
	if (omdp == NULL) return FALSE;

	if (omdp->tempload == TL_NOT_TEMP)
		return FALSE;
	else
		return TRUE;

}

/*****************************************************************************
*
*   Boolean ObjMgrIsParent(parent, child)
*   	returns TRUE if child is a child of parent
*       if parent = NULL, returns TRUE if child has no parent
*       child must never be NULL
*       returns TRUE if they are the equal
*       data must be BioseqPtr or BioseqSetPtr
*
*****************************************************************************/
Boolean LIBCALL ObjMgrIsChild (Pointer parent, Pointer child)
{
	ObjMgrPtr omp;
	Int2 i;
	ObjMgrDataPtr omdp, PNTR omdpp;

	if (child == NULL) return FALSE;
	if (parent == child) return TRUE;

	omp = ObjMgrGet();
	omdpp = omp->datalist;

	i = ObjMgrLookup(omp, child);
	if (i < 0)  /* not found */
	{
		ErrPostEx(SEV_ERROR, 0,0, "ObjMgrIsChild: pointer [%ld] not found",
			(long)child);
		return FALSE;
	}

	omdp = omdpp[i];
	if (parent == NULL)
	{
		if (omdp->parentptr == NULL)
			return TRUE;
		else
			return FALSE;
	}

	while (omdp->parentptr != NULL)
	{
		if (omdp->parentptr == parent)
			return TRUE;
		i = ObjMgrLookup(omp, omdp->parentptr);
		if (i < 0)
			return FALSE;
		omdp = omdpp[i];
	}

	return FALSE;
}

#ifdef DEBUG_OBJMGR
/*****************************************************************************
*
*   ObjMgrDump(fp)
*
*****************************************************************************/

static void ObjMgrDump (FILE * fp, CharPtr title)
{
	ObjMgrPtr omp;
	ObjMgrDataPtr omdp;
	Int2 i;
	Char buf[80];
	BioseqPtr bsp;
	Boolean close_it = FALSE;
	
	if (fp == NULL)
	{
		fp = FileOpen("ObjMgr.log", "a");
		close_it = TRUE;
	}

	omp = ObjMgrGet();
	fprintf(fp, "\n%s currobj=%d tempcnt=%d\n", title, (int)(omp->currobj),
		(int)(omp->tempcnt));
	for (i = 0; i < (Int2)(omp->currobj); i++)
	{
		omdp = omp->datalist[i];
		fprintf(fp, "[%d] [%d %d %ld] [%d %ld] %ld (%d) %ld\n", (int)i,
		    (int)omdp->EntityID, (int)(omdp->datatype),
			(long)(omdp->dataptr), (int)(omdp->parenttype),
			(long)(omdp->parentptr), (long)(omdp->choice), (int)(omdp->lockcnt),
			(long)(omdp->touch));
		if ((omdp->datatype == OBJ_BIOSEQ) && (omdp->dataptr != NULL))
		{
			bsp = (BioseqPtr)(omdp->dataptr);
			SeqIdPrint(bsp->id, buf, PRINTID_FASTA_LONG);
			fprintf(fp, "[%s %ld]\n", buf, (long)(bsp));
		}
	}
	
	if (close_it)
		FileClose(fp);
	return;
}
#endif

/*****************************************************************************
*
*   Procedure Management Functions
*
*****************************************************************************/


/*****************************************************************************
*
*   ObjMgrProcExtend(omp)
*
*****************************************************************************/
static Boolean NEAR ObjMgrProcExtend (ObjMgrPtr omp)
{
	Boolean result = FALSE;
	OMProcPtr omdp, prev=NULL;
	ObjMgrProcPtr PNTR tmp;
	Int2 i, j;

	for (omdp = omp->ncbiproc; omdp != NULL; omdp = omdp->next)
		prev = omdp;

	omdp = (OMProcPtr)MemNew(sizeof(OMProc));
	if (omdp == NULL) return result;
	tmp = (ObjMgrProcPtr PNTR)MemNew((size_t)(sizeof(ObjMgrProcPtr) * (omp->totproc + NUM_OMD)));
	if (tmp == NULL)
	{
		MemFree(omdp);
		return result;
	}

	if (prev != NULL)
	{
		prev->next = omdp;
		MemMove(tmp, omp->proclist, (size_t)(sizeof(ObjMgrProcPtr) *omp->totproc));
		MemFree(omp->proclist);
	}
	else
		omp->ncbiproc = omdp;

	j = omp->totproc;

	for (i = 0; i < NUM_OMD; i++, j++)
		tmp[j] = &(omdp->data[i]);

	omp->totproc += NUM_OMD;
	omp->proclist = tmp;

	result = TRUE;
	return result;
}

/*****************************************************************************
*
*   ObjMgrProcAdd(data, priority)
*   	adds an ObjMgrProc at given priority
*		priority must be > 0
*       highest priority function is called first.
*       if priority == 0 (default)
*   		gets the next highest priority over previous procs of same type
*       if priority == PROC_PRIORITY_HIGHEST
*           is always the highest priority (first one wins)
*   	if priority == PROC_PRIORITY_LOWEST
*           is always the lowest priority
*
*****************************************************************************/
Uint2  LIBCALL ObjMgrProcAdd (ObjMgrProcPtr data, Int2 priority)
{
	ObjMgrPtr omp;
	ObjMgrProcPtr omdp;
	ObjMgrProcPtr PNTR omdpp;
	Int2 i;
	Uint2 procID;

	omp = ObjMgrGet();

	if (priority == 0)   /* set to next highest */
	{
		omdp = ObjMgrProcFindNext(omp, data->proctype, data->inputtype,
								data->outputtype, NULL);
		if (omdp != NULL)
			priority = omdp->priority + 10;
	}


	if (omp->currproc >= omp->totproc)
	{
		if (! ObjMgrProcExtend(omp))
			return FALSE;
	}

	i = omp->currproc;
	omdpp = omp->proclist;
	omdp = omdpp[i];    /* emptys always at end */

	omp->currproc++;     /* got one more */
	procID = ++(omp->HighestProcID);

	MemMove(omdp, data, sizeof(ObjMgrProc));
	omdp->priority = priority;
	omdp->procid = procID;  /* fill in the values */
	omdp->proclabel = StringSave(data->proclabel);
	omdp->procname = StringSave(data->procname);

	return procID;
}

/*****************************************************************************
*
*   ObjMgrProcLoad(proctype, procname, proclabel, inputtype, subinputtype,
*              outputtype, suboutputtype,
*   				data, func, priority)
*   	adds a new proceedure with these parameters
*   	returns the procid
*   	if a procedure of the same name and type are already loaded
*   		returns the procid of the loaded proc.. does not reload
*
*****************************************************************************/
Uint2 ObjMgrProcLoad (Uint2 proctype, CharPtr procname, CharPtr proclabel,
							Uint2 inputtype, Uint2 subinputtype,
							Uint2 outputtype, Uint2 suboutputtype, Pointer userdata,
							ObjMgrGenFunc func, Int2 priority)
{
	ObjMgrPtr omp;
	ObjMgrProcPtr ompp;
	ObjMgrProc ompd;

	omp = ObjMgrGet();
	ompp = ObjMgrProcFind(omp, 0, procname, proctype);
	if (ompp != NULL)  /* already enabled */
		return ompp->procid;

	ompp = &ompd;
	MemSet(ompp, 0, sizeof(ObjMgrProc));
	
	ompp->proctype = proctype;
	ompp->procname = procname;
	ompp->proclabel = proclabel;
	ompp->inputtype = inputtype;
	ompp->subinputtype = subinputtype;
	ompp->outputtype = outputtype;
	ompp->suboutputtype = suboutputtype;
	ompp->procdata = userdata;
	ompp->func = func;

	return ObjMgrProcAdd(ompp, priority); /* order determines priority */
	
}

/*****************************************************************************
*
*   ObjMgrProcLookup(omp, procid)
*   	these are currently just stored in order
*   	returns index (>=0) if found
*       returns -1 if not found
*
*****************************************************************************/
Int2 LIBCALL ObjMgrProcLookup(ObjMgrPtr omp, Uint2 procid)
{
	if (omp == NULL)
		return (Int2)(-1);

	if (procid)
		return (Int2)(procid - 1);
	else
		return (Int2)(-1);
}

/*****************************************************************************
*
*   ObjMgrProcFind(omp, procid, procname, proctype)
*   	if procid != NULL looks for it
*   	else matches on procname and proctype
*   		proctype = 0, matches all proctypes
*
*****************************************************************************/
ObjMgrProcPtr LIBCALL ObjMgrProcFind(ObjMgrPtr omp, Uint2 procid,
										CharPtr procname, Uint2 proctype)
{
	ObjMgrProcPtr ompp=NULL, PNTR omppp, tmp;
	Int2 i, imax;

	omppp = omp->proclist;
	imax = omp->currproc;

	if (procid)   /* procid lookup is different */
	{
		i = ObjMgrProcLookup(omp, procid);
		if (i >= 0)
			ompp = omppp[i];
		return ompp;
	}

	for (i = 0; i < imax; i++)
	{
		tmp = omppp[i];
		if ((! proctype) || (proctype == tmp->proctype))
		{
			if (! StringCmp(procname, tmp->procname))
				return tmp;
		}
	}
	return ompp;
}

/*****************************************************************************
*
*   ObjMgrProcFindNext(omp, proctype, inputtype, outputtype, last)
*   	looks for proctype of highest priority that
*   		matches inputtype and outputtype
*   	0 on proctype or inputtype or outputtype matches any
*   	if last != NULL, then gets next after last
*
*****************************************************************************/
ObjMgrProcPtr LIBCALL ObjMgrProcFindNext (ObjMgrPtr omp, Uint2 proctype,
						Uint2 inputtype, Uint2 outputtype, ObjMgrProcPtr last)
{
	ObjMgrProcPtr best = NULL, tmp;
	Int2 i, bestpriority=-32766, imax, maxpriority=32767;
	ObjMgrProcPtr PNTR omppp;

	omppp = omp->proclist;
	imax = omp->currproc;

	if (last != NULL)
	{
		maxpriority = last->priority;
		i = ObjMgrProcLookup(omp, last->procid);
		i++;
		while (i < imax)  /* find next of equal priority */
		{
			tmp = omppp[i];
			if ((! proctype) || (tmp->proctype == proctype))
			{
				if ((! inputtype) || (tmp->inputtype == inputtype))
				{
					if ((! outputtype) || (tmp->outputtype == outputtype))
					{
						if (tmp->priority == maxpriority)
							return tmp;
					}
				}
			}
			i++;
		}
	}

	for (i = 0; i < imax; i++)  /* find the highest priority less than any previous */
	{
		tmp = omppp[i];
		if ((! proctype) || (tmp->proctype == proctype))
		{
			if ((! inputtype) || (tmp->inputtype == inputtype))
			{
				if ((! outputtype) || (tmp->outputtype == outputtype))
				{
					if ((tmp->priority > bestpriority) &&
						(tmp->priority < maxpriority))
					{
						best = tmp;
						bestpriority = tmp->priority;
					}
				}
			}
		}

	}
	return best;
}

Int2 LIBCALL ObjMgrProcOpen (ObjMgrPtr omp, Uint2 outputtype)
{
	ObjMgrProcPtr ompp=NULL, currp=NULL;
	Int2 retval;
	Boolean did_one = FALSE;
	OMProcControl ompc;

	MemSet(&ompc, 0, sizeof(OMProcControl));
	ompc.output_itemtype = outputtype;

	while ((currp = ObjMgrProcFindNext(omp, OMPROC_OPEN, 0, outputtype, currp)) != NULL)
	{
		ompc.proc = currp;
		retval = (*(currp->func)) (&ompc);
		did_one = TRUE;
		if (retval == OM_MSG_RET_DONE)
			break;
	}

	if (! did_one)
	{
		ErrPostEx(SEV_ERROR,0,0, "No OPEN function found");
		retval = OM_MSG_RET_ERROR;
	}
	return retval;
}

Uint2 LIBCALL OMGetNextUserKey (void)

{
  ObjMgrPtr  omp;

  omp = ObjMgrGet ();
  if (omp == NULL) return 0;
  (omp->HighestUserKey)++;
  if (omp->HighestUserKey == 0) {
    (omp->HighestUserKey)++;
  }
  return omp->HighestUserKey;
}

/*****************************************************************************
*
*   Data Type Functions
*
*****************************************************************************/

static Int2 LIBCALLBACK ObjMgrDefaultLabelFunc ( Pointer data, CharPtr buffer, Int2 buflen, Uint1 content)
{
	ObjMgrPtr omp;
	ObjMgrDataPtr omdp;
	ObjMgrTypePtr omtp;
	CharPtr label=NULL;
	static CharPtr defaultlabel="NoLabel";

	label = defaultlabel;
	omp = ObjMgrGet();
	omdp = ObjMgrFindByData(omp, data);
	if (omdp != NULL)
	{
		omtp = ObjMgrTypeFind(omp, omdp->datatype, NULL, NULL);
		if (omtp != NULL)
		{
			if (omtp->label != NULL)
				label = omtp->label;
			else if (omtp->name != NULL)
				label = omtp->name;
			else
				label = omtp->asnname;
		}
	}

	return LabelCopy(buffer, label, buflen);
}

static Uint2 LIBCALLBACK ObjMgrDefaultSubTypeFunc (Pointer ptr)
{
	return 0;
}

/*****************************************************************************
*
*   ObjMgrTypeExtend(omp)
*
*****************************************************************************/
static Boolean NEAR ObjMgrTypeExtend (ObjMgrPtr omp)
{
	Boolean result = FALSE;
	OMTypePtr omdp, prev=NULL;
	ObjMgrTypePtr PNTR tmp;
	Int2 i, j;

	for (omdp = omp->ncbitype; omdp != NULL; omdp = omdp->next)
		prev = omdp;

	omdp = (OMTypePtr)MemNew(sizeof(OMType));
	if (omdp == NULL) return result;
	tmp = (ObjMgrTypePtr PNTR)MemNew((size_t)(sizeof(ObjMgrTypePtr) * (omp->tottype + NUM_OMD)));
	if (tmp == NULL)
	{
		MemFree(omdp);
		return result;
	}

	if (prev != NULL)
	{
		prev->next = omdp;
		MemMove(tmp, omp->typelist, (size_t)(sizeof(ObjMgrTypePtr) * omp->tottype));
		MemFree(omp->typelist);
	}
	else
		omp->ncbitype = omdp;

	j = omp->tottype;

	for (i = 0; i < NUM_OMD; i++, j++)
		tmp[j] = &(omdp->data[i]);

	omp->tottype += NUM_OMD;
	omp->typelist = tmp;

	result = TRUE;
	return result;
}

/*****************************************************************************
*
*   ObjMgrTypeAdd(type, data)
*   	adds a pointer (data) of type (type) to the sequence manager
*       Currently this MUST be predefined and < OBJ_MAX
*
*****************************************************************************/
Uint2 LIBCALL ObjMgrTypeAdd (ObjMgrTypePtr data)
{
	ObjMgrPtr omp;
	ObjMgrTypePtr omdp;
	ObjMgrTypePtr PNTR omdpp;
	Int2 i, imin, imax;
	Uint2 tmp, type;

	if (data == NULL) return 0;

	omp = ObjMgrGet();

	type = data->datatype;
	if ((! type) || (type >= OBJ_MAX))
	{
		/***
		type = ++(omp->HighestObjMgrType);
	    ***/

		ErrPostEx(SEV_ERROR, 0,0, "ObjMgrTypeAdd: Can't register new object types yet");
		return 0;
	}

	if (omp->currtype >= omp->tottype)
	{
		if (! ObjMgrTypeExtend(omp))
			return 0;
	}

	i = omp->currtype;
	omdpp = omp->typelist;
	omdp = omdpp[i];    /* emptys always at end */

	imin = 0;                   /* find where it goes */
	imax = omp->currtype-1;

	if ((i) && (type < omdpp[imax]->datatype))
	{
		i = (imax + imin) / 2;
		while (imax > imin)
		{
			tmp = omdpp[i]->datatype;
			if (tmp > type)
				imax = i - 1;
			else if (tmp < type)
				imin = i + 1;
			else
				break;
			i = (imax + imin)/2;
		}

		if (type > omdpp[i]->datatype)     /* check for off by 1 */
			i++;

		imax = omp->currtype - 1;	 /* open the array */
		while (imax >= i)
		{
			omdpp[imax+1] = omdpp[imax];
			imax--;
		}
	}

	omdpp[i] = omdp;    /* put in the pointer in order */
	omp->currtype++;     /* got one more */

	MemMove(omdp, data, sizeof(ObjMgrType));
	omdp->datatype = type;  /* fill in the values */
	omdp->asnname = StringSave(data->asnname);
	omdp->label = StringSave(data->label);
	omdp->name = StringSave(data->name);
	if (omdp->labelfunc == NULL)
		omdp->labelfunc = ObjMgrDefaultLabelFunc;
	if (omdp->subtypefunc == NULL)
		omdp->subtypefunc = ObjMgrDefaultSubTypeFunc;

	return type;
}

/*****************************************************************************
*
*   ObjMgrTypeLoad(args)
*
*****************************************************************************/
Uint2 LIBCALL ObjMgrTypeLoad ( Uint2 type, CharPtr asnname,
		CharPtr label, CharPtr name, AsnTypePtr atp, OMNewFunc newfunc,
		AsnReadFunc asnread, AsnWriteFunc asnwrite, OMFreeFunc freefunc,
		OMLabelFunc labelfunc, OMSubTypeFunc subtypefunc)
{
	ObjMgrType omt;
	Uint2 newtype;

	MemSet(&omt, 0, sizeof(ObjMgrType));
	omt.datatype = type;
	omt.asnname = asnname;
	omt.label = label;
	omt.name = name;
	omt.atp = atp;
	omt.newfunc = newfunc;
	omt.asnread = asnread;
	omt.asnwrite = asnwrite;
	omt.freefunc = freefunc;
	omt.labelfunc = labelfunc;
	omt.subtypefunc = subtypefunc;

	newtype = ObjMgrTypeAdd(&omt);

	return newtype;
}

/*****************************************************************************
*
*   ObjMgrTypeLookup(omp, data)
*   	Binary lookup of data in omp->typelist
*   	returns index (>=0) if found
*       returns -1 if not found
*
*****************************************************************************/
Int2 LIBCALL ObjMgrTypeLookup(ObjMgrPtr omp, Uint2 data)
{
	Int2 imin, imax, i;
	ObjMgrTypePtr PNTR omdpp;
	Uint2 tmp;

	imin = 0;
	imax = omp->currtype - 1;
	omdpp = omp->typelist;

	while (imax >= imin)
	{
		i = (imax + imin)/2;
		tmp = omdpp[i]->datatype;
		if (tmp > data)
			imax = i - 1;
		else if (tmp < data)
			imin = i + 1;
		else
			return i;
	}

	return (Int2)(-1);
}

/*****************************************************************************
*
*   ObjMgrTypeFind(omp, type, asnname, name)
*   	returns the objmgrptr by looking for
*   		type: if type != 0
*   		asnname: if asnname != NULL
*   		name: if name != NULL
*       in that order of preference.
*   	returns NULL if can't match on highest priority key
*
*****************************************************************************/
ObjMgrTypePtr LIBCALL ObjMgrTypeFind (ObjMgrPtr omp, Uint2 type, CharPtr asnname, CharPtr name)
{
	ObjMgrTypePtr omtp = NULL;
	ObjMgrTypePtr PNTR omdpp;
	Int2 i, imax, result;

	omdpp = omp->typelist;
	imax = omp->currtype;
	if (type)
	{
		i = ObjMgrTypeLookup(omp, type);
		if (i >= 0)
			omtp = omdpp[i];
	}
	else
	{
		for (i = 0; i < imax; i++)
		{
			if (asnname != NULL)
				result = StringCmp(omdpp[i]->asnname, asnname);
			else
				result = StringCmp(omdpp[i]->name, name);
			if (! result)
			{
				omtp = omdpp[i];
				break;
			}
		}
	}
	return omtp;
}

/*****************************************************************************
*
*   ObjMgrTypeSetLabelFunc(type, labelfunc)
*   	replaces the labelfunc for type with a new one
*   	can also set it for the first time
*
*****************************************************************************/
Boolean LIBCALL ObjMgrTypeSetLabelFunc(Uint2 type, OMLabelFunc labelfunc)
{
	ObjMgrPtr omp;
	ObjMgrTypePtr omtp;

	omp = ObjMgrGet();
	omtp = ObjMgrTypeFind(omp, type, NULL, NULL);
	if (omtp == NULL)
		return FALSE;
	omtp->labelfunc = labelfunc;
	return TRUE;
}

/**************************************************************************
*
* ObjMgrTypeFindNext(omp, omtp)
*    returns next ObjMgrType after omtp
*    Exhaustively traverses registered types if omtp starts as NULL
*
***************************************************************************/		
ObjMgrTypePtr LIBCALL ObjMgrTypeFindNext (ObjMgrPtr omp, ObjMgrTypePtr last)
{
	ObjMgrTypePtr PNTR omdpp;
	Int2 i, imax;
	Boolean got_it = FALSE;

	omdpp = omp->typelist;
	imax = omp->currtype;
	if (last == NULL)   /* take the first one */
		got_it = TRUE;
	for (i = 0; i < imax; i++)
	{
		if (got_it)
			return omdpp[i];
			
		if (omdpp[i] == last)
			got_it = TRUE;
	}
	
	return NULL;
}

/*****************************************************************************
*
*   Selection Functions for data objects
*
*****************************************************************************/

static SelStructPtr NEAR ObjMgrAddSelStruct(ObjMgrPtr omp, Uint2 entityID, Uint2 itemID, Uint2 itemtype)
{
	SelStructPtr ssp, tmp;
	ObjMgrDataPtr omdp;

	omdp = ObjMgrFindByEntityID(omp, entityID, NULL);
	if (omdp == NULL)
		return NULL;

	tmp = omp->sel;

	if (tmp != NULL)
	{
		while (tmp->next != NULL)
			tmp = tmp->next;
	}

	ssp = MemNew(sizeof(SelStruct));
	if (tmp != NULL)
	{
		tmp->next = ssp;
		ssp->prev = tmp;
	}
	else
		omp->sel = ssp;

	ssp->entityID = entityID;
	ssp->itemID = itemID;
	ssp->itemtype = itemtype;

	ObjMgrSendMsgFunc(omp, omdp, OM_MSG_SELECT, entityID, itemID, itemtype);

	return ssp;
}

static Boolean NEAR ObjMgrDeSelectFunc (ObjMgrPtr omp, SelStructPtr ssp)
{
	SelStructPtr next, prev;
	ObjMgrDataPtr omdp;

	if (ssp == NULL) return FALSE;

	omdp = ObjMgrFindByEntityID(omp, ssp->entityID, NULL);
	ObjMgrSendMsgFunc(omp, omdp, OM_MSG_DESELECT, ssp->entityID, ssp->itemID, ssp->itemtype);

	next = ssp->next;
	prev = ssp->prev;

	MemFree(ssp);
	if (prev != NULL)
		prev->next = next;
	else
		omp->sel = next;

	if (next != NULL)
		next->prev = prev;

	return TRUE;
}

static Boolean NEAR ObjMgrDeSelectAllFunc (ObjMgrPtr omp)
{
	SelStructPtr tmp, prev;

	tmp = omp->sel;
	if (tmp == NULL)
		return TRUE;

	prev = NULL;

	while (tmp->next != NULL)
	{
		prev = tmp;
		tmp = tmp->next;
	}

	while (tmp != NULL)
	{
		ObjMgrDeSelectFunc(omp, tmp);
		tmp = prev;
		if (tmp != NULL)
			prev = tmp->prev;
	}

	return TRUE;
}

/*****************************************************************************
*
*   ObjMgrSelect(entityID, itemID, itemtype)
*      if entityID == 0, just deselects everything
*      if entityID,itemID, itemtype already selected, deselects it
*      if something else selected, deselects it first, then selects requested
*        item
*      returns TRUE if item is now currently selected, FALSE if not
*
*****************************************************************************/
Boolean LIBCALL ObjMgrSelect (Uint2 entityID, Uint2 itemID, Uint2 itemtype)
{
	ObjMgrPtr omp;
	SelStructPtr ssp;
	Boolean was_selected = FALSE;

	omp = ObjMgrGet();

	if (entityID == 0)    /* desktop */
		was_selected = TRUE;
	else
	{
		ssp = ObjMgrGetSelected();
		while (ssp != NULL)
		{
			if (ssp->entityID == entityID)
			{
				if ((ssp->itemID == itemID) || (! itemID))
				{
					if ((ssp->itemtype == itemtype) || (! itemtype))
					{
						was_selected = TRUE;
						break;
					}
				}
			}
			ssp = ssp->next;
		}
	}

	if (! ObjMgrDeSelectAllFunc(omp))
		return FALSE;

	if (was_selected)  /* just deselecting something */
		return FALSE;

	ssp = ObjMgrAddSelStruct(omp, entityID, itemID, itemtype);

	if (ssp == NULL)
		return FALSE;
	else
		return TRUE;

}

/*****************************************************************************
*
*   ObjMgrDeSelect(entityID, itemID, itemtype)
*   	if this item was selected, then deselects and returns TRUE
*   	else returns FALSE
*
*****************************************************************************/

Boolean LIBCALL ObjMgrDeSelect (Uint2 entityID, Uint2 itemID, Uint2 itemtype)
{
	ObjMgrPtr omp;
	SelStructPtr tmp, next;
	Boolean retval=FALSE, tret;

	omp = ObjMgrGet();

	if (entityID == 0)
		return ObjMgrDeSelectAllFunc(omp);

	tmp = omp->sel;
	while (tmp != NULL)
	{
		next = tmp->next;
		if (tmp->entityID == entityID)
		{
			if ((! itemtype) || (itemtype == tmp->itemtype))
			{
				if ((! itemID) || (itemID == tmp->itemID))
				{
					tret = ObjMgrDeSelectFunc(omp, tmp);
					if (tret)
						retval = TRUE;
				}
			}
		}
		tmp = next;
	}
	return retval;
}

Boolean LIBCALL ObjMgrAlsoSelect (Uint2 entityID, Uint2 itemID, Uint2 itemtype)
{
	ObjMgrPtr omp;
	SelStructPtr ssp;

							/* if already selected, just deselect */
	if (ObjMgrDeSelect(entityID, itemID, itemtype))
		return FALSE;

	omp = ObjMgrGet();

	ssp = ObjMgrAddSelStruct(omp, entityID, itemID, itemtype);

	if (ssp == NULL)
		return FALSE;
	else
		return TRUE;

}

Boolean LIBCALL ObjMgrDeSelectAll (void)
{
	ObjMgrPtr omp;

	omp = ObjMgrGet();
	return ObjMgrDeSelectAllFunc(omp);
}

SelStructPtr LIBCALL ObjMgrGetSelected (void)
{
	ObjMgrPtr omp;

	omp = ObjMgrGet();
	return omp->sel;
}

/*****************************************************************************
*
*   ObjMgrSendMsg(msg, entityID, itemID, itemtype)
*       Directly invokes the objmgr messaging system
*       should be used cautiously
*
*****************************************************************************/
Boolean LIBCALL ObjMgrSendMsg(Uint2 msg, Uint2 entityID, Uint2 itemID, Uint2 itemtype)
{
	ObjMgrPtr omp;
	ObjMgrDataPtr omdp;

	omp = ObjMgrGet();
	omdp = ObjMgrFindByEntityID(omp, entityID, NULL);
	if (omdp == NULL)
		return FALSE;
	ObjMgrSendMsgFunc(omp, omdp, msg, entityID, itemID, itemtype);
	return TRUE;
}

 GBPARSE_INT_COMMA :
			case  GBPARSE_INT_SINGLE_DOT :
					
					Nlm_gbparse_error("illegal initial token", 
						head_token, current_token);
						* keep_rawPt = TRUE;  (* num_errsPt) ++;
				current_token = current_token -> next;
				break;

			case  GBPARSE_INT_ACCESION :
/*--- no warn, but strange ---*/
/*-- no break on purpose ---*/

			case  GBPARSE_INT_CARET : case  GBPARSE_INT_GT :
			case  GBPARSE_INT_LT : case  GBPARSE_INT_NUMBER :
			case  GBPARSE_INT_LEFT :

			case GBPARSE_INT_ONE_OF_NUM:

			retval = Nlm_gbint(keep_rawPt,  & current_token,
				head_token, (num_errsPt), seq_id);
/* need to check that out of tokens here */
			retval = Nlm_gbparse_better_be_done(num_errsPt, current_token, 
					head_token, retval, keep_rawPt,  paren_count);
				break;

			case  GBPARSE_INT_REPLACE :
			retval = Nlm_gbreplace(keep_rawPt,  & paren_count, sitesPt, & current_token,  
				head_token, (num_errsPt), seq_id);
				* keep_rawPt = TRUE;
/*---all errors handled within this function ---*/
				break;
			case GBPARSE_INT_SITES :
				* sitesPt = TRUE;
				go_again = TRUE;
				current_token = current_token -> next;
				break;
		}
	}while (go_again && current_token);
	}else{
		* keep_rawPt = TRUE;
	}
	
	if ( head_token)
		ValNodeFreeData(head_token);

	if ( (*num_errsPt)){
		SeqLocFree(retval);
		retval = NULL;
	}
	return retval;
}

/*---------- Nlm_gbloc()-----*/

SeqLocPtr
Nlm_gbloc(Boolean PNTR keep_rawPt, int PNTR parenPt, Boolean PNTR sitesPt, ValNodePtr PNTR currentPt, ValNodePtr head_token, int PNTR num_errPt, SeqIdPtr seq_id)
{
	SeqLocPtr retval =NULL;
	Boolean add_nulls=FALSE;
	ValNodePtr current_token = * currentPt;
	Boolean did_complement= FALSE;
	Boolean go_again ;

	do {
		go_again= FALSE;
		switch ( current_token -> choice){
			case  GBPARSE_INT_COMPL :
			*currentPt = (* currentPt) -> next;
			if ( (*currentPt) == NULL){
				Nlm_gbparse_error("unexpected end of usable tokens",
					head_token, *currentPt);
				* keep_rawPt = TRUE;  (* num_errPt) ++;
				goto FATAL;
			}
			if ( (* currentPt) -> choice != GBPARSE_INT_LEFT){
					Nlm_gbparse_error("Missing \'(\'", /* paran match  ) */
						head_token, * currentPt);
					* keep_rawPt = TRUE;  (* num_errPt) ++;
					goto FATAL;
			}else{
				(*parenPt) ++; *currentPt = (* currentPt) -> next;
				if ( ! * currentPt){
						Nlm_gbparse_error("illegal null contents",
							head_token, *currentPt);
						* keep_rawPt = TRUE;  (* num_errPt) ++;
					goto FATAL;
				}else{
					if (  (* currentPt) -> choice == GBPARSE_INT_RIGHT){ /* paran match ( */
						Nlm_gbparse_error("Premature \')\'",
							head_token, *currentPt);
						* keep_rawPt = TRUE;  (* num_errPt) ++;
						goto FATAL;
					}else{
						retval =  Nlm_gbloc (keep_rawPt, parenPt, sitesPt, currentPt, 
							head_token, num_errPt,seq_id) ;
						SeqLocRevCmp ( retval);
						did_complement= TRUE;
						if ( * currentPt){
							if ( (* currentPt) -> choice != GBPARSE_INT_RIGHT){
									Nlm_gbparse_error("Missing \')\'",
										head_token, *currentPt);
									* keep_rawPt = TRUE;  (* num_errPt) ++;
									goto FATAL;
							}else{
								(*parenPt) --; *currentPt = (* currentPt) -> next;
							}
						}else{
							Nlm_gbparse_error("Missing \')\'",
								head_token, *currentPt);
							* keep_rawPt = TRUE;  (* num_errPt) ++;
							goto FATAL;
						}
					}
				}
			}
				break;
/* REAL LOCS */
			case GBPARSE_INT_JOIN : retval =ValNodeNew(NULL); retval -> choice = SEQLOC_MIX; break;
			case  GBPARSE_INT_ORDER : retval =ValNodeNew(NULL); retval -> choice = SEQLOC_MIX; add_nulls=TRUE;break;
			case  GBPARSE_INT_GROUP : * keep_rawPt = TRUE; retval =ValNodeNew(NULL); retval -> choice = SEQLOC_MIX;
				break;
			case  GBPARSE_INT_ONE_OF : retval =ValNodeNew(NULL); retval -> choice = SEQLOC_EQUIV; break;

/* ERROR */
			case GBPARSE_INT_STRING:
				Nlm_gbparse_error("string in loc", 
					head_token, current_token);
					* keep_rawPt = TRUE;  (* num_errPt) ++;
					goto FATAL;
/*--- no break on purpose---*/
			case  GBPARSE_INT_UNKNOWN : default: 
			case  GBPARSE_INT_RIGHT : case  GBPARSE_INT_DOT_DOT:case  GBPARSE_INT_COMMA :
			case  GBPARSE_INT_SINGLE_DOT :
				Nlm_gbparse_error("illegal initial loc token",
					head_token, *currentPt);
				* keep_rawPt = TRUE;  (* num_errPt) ++;
					goto FATAL;

/* Interval, occurs on recursion */
			case  GBPARSE_INT_ACCESION :
			case  GBPARSE_INT_CARET : case  GBPARSE_INT_GT :
			case  GBPARSE_INT_LT : case  GBPARSE_INT_NUMBER :
			case  GBPARSE_INT_LEFT :

			case GBPARSE_INT_ONE_OF_NUM:

			retval = Nlm_gbint(keep_rawPt, currentPt, 
				head_token, num_errPt, seq_id);
				break;

			case  GBPARSE_INT_REPLACE :
/*-------illegal at this level --*/
				Nlm_gbparse_error("illegal replace",
					head_token, *currentPt);
				* keep_rawPt = TRUE;  (* num_errPt) ++;
					goto FATAL;
			case GBPARSE_INT_SITES :
				* sitesPt = TRUE;
				go_again = TRUE;
				(*currentPt) = (*currentPt) -> next;
				break;
		}
	} while (go_again && *currentPt);

	if ( !  (* num_errPt)) if (retval) 
	if ( retval -> choice != SEQLOC_INT && retval -> choice != SEQLOC_PNT
			&& ! did_complement){ 
/*--------
 * ONLY THE CHOICE has been set. the "join", etc. only has been noted
 *----*/
		*currentPt = (* currentPt) -> next;
		if ( ! * currentPt){
				Nlm_gbparse_error("unexpected end of interval tokens",
					head_token, *currentPt);
						* keep_rawPt = TRUE;  (* num_errPt) ++;
					goto FATAL;
		}else{
			if ( (* currentPt) -> choice != GBPARSE_INT_LEFT){
					Nlm_gbparse_error("Missing \'(\'",
						head_token, *currentPt); /* paran match  ) */
					* keep_rawPt = TRUE;  (* num_errPt) ++;
					goto FATAL;
			}else{
				(*parenPt) ++; *currentPt = (* currentPt) -> next;
				if ( ! * currentPt){
						Nlm_gbparse_error("illegal null contents",
							head_token, *currentPt);
						* keep_rawPt = TRUE;  (* num_errPt) ++;
						goto FATAL;
				}else{
					if (  (* currentPt) -> choice == GBPARSE_INT_RIGHT){ /* paran match ( */
						Nlm_gbparse_error("Premature \')\'" ,
							head_token, *currentPt);
						* keep_rawPt = TRUE;  (* num_errPt) ++;
					goto FATAL;
					}else{

						ValNodePtr last= NULL, next_loc = NULL;

						while ( ! *num_errPt && * currentPt){
							if ( (* currentPt) -> choice == GBPARSE_INT_RIGHT){
								while  ((* currentPt) -> choice == GBPARSE_INT_RIGHT){ 
									(*parenPt) --;
									*currentPt = (* currentPt) -> next;
									if ( ! *currentPt)
										break;
								}
								break;
							}
							if ( ! * currentPt){
								break;
							}
							next_loc = Nlm_gbloc(keep_rawPt, parenPt,sitesPt,  
								currentPt, head_token, num_errPt,
								seq_id);
							if( retval -> data.ptrvalue == NULL)
								retval -> data.ptrvalue = next_loc;
							if ( last)
								last -> next = next_loc;
							last = next_loc;
							if ( ! * currentPt){
								break;
							}
							if ( ! * currentPt){
								break;
							}
							if ((* currentPt) -> choice == GBPARSE_INT_RIGHT){
								break;
							}
							if (  (* currentPt) -> choice == GBPARSE_INT_COMMA){
								*currentPt = (* currentPt) -> next;
								if(ad