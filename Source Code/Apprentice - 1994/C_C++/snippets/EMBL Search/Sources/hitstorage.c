/*
*********************************************************************
*	
*	HitStorage.c
*	Handling of results storage and hitlist management
*
*	Rainer Fuchs
*	EMBL Data Library
*	Postfach 10.2209
*	D-6900 Heidelberg, FRG
*	E-mail: fuchs@embl-heidelberg.de
*
*	Copyright � 1992 EMBL Data Library
*		
**********************************************************************
*	
*/ 

#include <stdio.h>

#include "EMBL-Search.h"
#include "EMBL-Search.rsrc.h"

/*
******************************* Prototypes ***************************
*/

#include "hitstorage.h"
#include "util.h"
#include "pstr.h"
#include "window.h"

static Boolean NewDEBlock(CString80Hdl *newBlockPtr);
static Boolean NewResultRec(ResultHdl *new);


/*
********************************* Globals *****************************
*/

extern IndexFiles	gFileList;
extern DBInfo		gDBInfo[DB_NUM];
extern char			gError[256];


/**************************************
*	Allocates movable memory block for description line buffer
*	Return value:	TRUE, if successful
*						FALSE, if error occurred
*	Side-effect:	Handle to this block
*/

static Boolean	NewDEBlock(CString80Hdl *newBlockPtr)
{
	*newBlockPtr = (CString80Hdl)NewHandleClear((Size)MAXBUFLINES * sizeof (CString80));
	return( *newBlockPtr != NULL );
}


/**************************************
*	Fill description line buffer from disk so that "from" is in buffer.
*	To avoid continous loading when a user browses through the results,
*	the buffer is filled (if possible) such that "from" is in the middle of
*	the buffer.
*	Return value:	TRUE, if successful
*						FALSE, if error occurred
*/

Boolean FillDEBuffer(ResultHdl resHdl,short first, Boolean bCache)
{
	short				fd;
	SignedByte		oldDEState,oldHlState;
	short				i,j;
	u_long			pos;
	long				size,count;
	CString80Hdl	descBufHdl = (**resHdl).descBufHdl;
	HitlistHdl 		hlHdl = (**resHdl).hlHdl;
	Str255			indexName,lenStr;
	DescRec			rec;
	char				*descPtr;
	OSErr				err = noErr;
	DialogPtr		myDialog;
	
	/* calculate first hit to be loaded. The size of the buffer guarantees
		that we always load at least two screen fulls */
	if(bCache) {
		if (first >= MAXBUFLINES/2)
			first -= MAXBUFLINES/2;
		else
			first = 0;
	}
	
	/* open short description index */
	pstrcpy(indexName,gFileList.briefIdxFName);
	if(OpenMacFileReadOnly(indexName,gDBInfo[(**resHdl).dbcode].InxWDRefNum, &fd, TRUE) != noErr )
		return(FALSE);
	
	/* Lock blocks */
	oldDEState = LockHandleHigh((Handle)descBufHdl);
	oldHlState = LockHandleHigh((Handle)hlHdl);

	CenterDA('DLOG',FETCHTITLES_DLG,50);
	myDialog = GetNewDialog(FETCHTITLES_DLG,NULL,(WindowPtr)-1);
	if(myDialog) {
		ShowWindow(myDialog);
		DrawDialog(myDialog);
	}
	
	size = sizeof(DescRec);
	for(	i=first,j=0;
			i < first+MAXBUFLINES && i < (**resHdl).nhits;
			++i,++j) {
		count = size;
		pos = ( (*hlHdl + i)->ename_rec );
		
		if( (err = SetFPos(fd,fsFromStart,pos*size+sizeof(Header))) != noErr) {
			sprintf(gError,LoadErrorStr(ERR_READFILE,FALSE),
						PtoCstr(indexName),err );
			ErrorMsg(0);
			break;
		}
		
		RotateWaitCursor();
		if( (err=ReadMacFile(fd,&count,&rec,indexName,TRUE)) != noErr )
			break;
		
		descPtr = (char *)(*descBufHdl + j);
		NumToString((long)ConvertLong(&rec.seqlen),lenStr);
		sprintf(descPtr,"%10.10s %7.7s %6.6s %.54s",
					rec.entryname,rec.accno,PtoCstr(lenStr),rec.description);
		CtoPstr(descPtr);
	}

	if (myDialog) DisposeDialog(myDialog);

	FSClose(fd);
	HSetState((Handle)descBufHdl,oldDEState);
	HSetState((Handle)hlHdl,oldHlState);
	(**resHdl).buftop = first;

	return(err == noErr);
}


/**************************************
*	Allocates a hitlist
*	Return value:	TRUE, if successful
*						FALSE, if error occurred
*	Side-effect:	Handle to the new block,
*/

Boolean NewHitlist(HitmapHdl hitmapHdl,short dbcode, HitlistHdl *new, short *nhits)
{
	register long	i,j,size;
	HitlistHdl		hlHdl;
	SignedByte		oldState;
	register long	*ptr;
	
	/* allocate empty hitlist */
	*new = (HitlistHdl)NewHandle(0L);
	if(*new != NULL) {
		hlHdl = *new;
		*nhits = 0;

/*		for (i=0; i<gDBInfo[dbcode].ename_nrec;++i) {*/
/*			if(BitTst(*hitmapHdl,i)) {*/
/*				++(*nhits);*/
/*				*/
/*				SetHandleSize( *new, *nhits * (long)sizeof(HitlistRec) );*/
/*				if( MemError() != noErr ) {*/
/*					DisposHandle((Handle)*new);*/
/*					return( ErrorMsg(ERR_HITLISTEXT,NULL) );*/
/*				}*/
/*				*/
/*				(**new)[*nhits-1].ename_rec = i;*/
/*				SetSelectState(*new,*nhits-1,FALSE);*/
/*			}*/
/*		}*/

		/* look through hitmap */
		oldState = MyHLock((Handle)hitmapHdl);
		ptr = *hitmapHdl;
		
		/* Assuming that hits are rare we do not test bit by bit, but check each long
			word in the hitmap, eg 32 bits in one go. Only if a long word is not zero,
			we check individual bits */
		size = GetHandleSize((Handle)hitmapHdl)/4;
		for (i=0; i<size;++i,++ptr) {
			if(*ptr) {
				for(j=0;j<32;++j) {
					if(BitTst(ptr,j)) {
						/* for each hit we add a new record to the hitlist */
						++(*nhits);
						
						SetHandleSize( (Handle)*new, *nhits * sizeof(HitlistRec) );
						if( MemError() != noErr ) {
							DisposHandle((Handle)*new);
							HSetState((Handle)hitmapHdl,oldState);
							return( ErrorMsg(ERR_HITLISTEXT) );
						}
						
						(**new)[*nhits-1].ename_rec = 32L * i + j;
						SetSelectState(*new,*nhits-1,FALSE);
					}
				}
			}
		}
		HSetState((Handle)hitmapHdl,oldState);
	}
	
	return( *new != NULL );
}


/**************************************
*	Allocates relocatable memory block for result record
*	Return value:	TRUE, if successful
*						FALSE, if error occurred
*	Side-effect:	Handle to this block
*/

static Boolean NewResultRec(ResultHdl *new)
{
	*new = (ResultHdl)NewHandleClear(sizeof(ResultRec));
	return( *new != NULL );
}


/**************************************
*	Gets a new result record, incl. description line buffer, and initialises it
*	Return values:
*	Return value:	TRUE, if successful
*						FALSE, if error occurred
*	Side effect:	Handle to new result record
*/

Boolean InitResultRec(ResultHdl *new, short dbcode, HitmapHdl hitmapHdl,
					QueryHdl queryHdl)
{
	ResultPtr	resPtr;
	HitlistHdl	hlHdl;
	short			nhits;
	
	if( new == NULL )
		return(FALSE);
		
	/* Allocate memory for result record */
	if ( !NewResultRec(new) )
		return( FALSE );
	
	/* Get a pointer to it */
	LockHandleHigh((Handle)*new);
	resPtr = (ResultPtr)**new;
	
	/* Allocate memory for description line buffer */
	if ( !NewDEBlock(&resPtr->descBufHdl) ) {
		DisposHandle((Handle)*new);
		*new = NULL;
		return(FALSE);
	}
	
	/* Allocate memory for hitlist */
	if ( !NewHitlist(hitmapHdl,dbcode,&hlHdl,&nhits) ) {
		DisposHandle((Handle)resPtr->descBufHdl);
		DisposHandle((Handle)*new);
		*new = NULL;
		return(FALSE);
	}
	
	/* Anchor hitlist to this result record */
	resPtr->hlHdl = hlHdl;
	resPtr->nhits = nhits;
	
	/* Anchor hitmap to result record */
	resPtr->hitmapHdl = hitmapHdl;
	
	/* Anchor query record to result record */
	resPtr->queryHdl = queryHdl;
	
	/* Init other fields */
	resPtr->buftop = 0;
	resPtr->nsel = 0;
	resPtr->dbcode = dbcode;
	HUnlock((Handle)*new);
	return(TRUE);
}


/**************************************
*	Dispose of contents of an existing result record
*	Return value:	none
*/

void DisposeResRec(ResultHdl resRecHdl)
{
	ResultPtr resPtr;
	
	if(resRecHdl == NULL)
		return;
	
	/* Get a pointer to the result record */
	LockHandleHigh((Handle)resRecHdl);
	resPtr = *resRecHdl;
	
	/* Free memory occupied by hitlist and description line buffers */
	if( resPtr->hlHdl != NULL)
		DisposHandle((Handle)resPtr->hlHdl);
	
	if( resPtr->descBufHdl != NULL)
		DisposHandle((Handle)resPtr->descBufHdl);
		
	if( resPtr->hitmapHdl != NULL )
		DisposHandle((Handle)resPtr->hitmapHdl);

	if( resPtr->queryHdl != NULL )
		DisposHandle((Handle)resPtr->queryHdl);
	
	HUnlock((Handle)resRecHdl);
}


/**************************************
*	See whether a result record is selected
*	Return value:	TRUE, if selected
*						FALSE, if not
*/

Boolean GetSelectState(HitlistHdl hlHdl, short pos)
{
	return((*hlHdl)[pos].flags.selected);
}


/**************************************
*	Select/deselect a result record
*	Return value:	none
*/

void SetSelectState(HitlistHdl hlHdl, short pos, Boolean state)
{
	(*hlHdl)[pos].flags.selected = state;
}


/**************************************
*	Allocate new hitmap. We always create multiples of "long", ie 32 bits.
*	Return value:	TRUE, if selected
*						FALSE, if not
*	Side-effect:	Handle to new block
*/

Boolean NewHitmap(HitmapHdl *new, short dbcode)
{
	register long	i;
	long				nlong;
	
	/* calculate number of long words necessary to hold hitmap. */
	nlong = (gDBInfo[dbcode].ename_nrec-1)/32 + 1;
	
	*new = (HitmapHdl) NewHandleClear((Size)nlong * 4L);
/*	if(*new) {
		for(i=0;i<nlong; ++i)
			(**new)[i] = 0L;
	}
*/
	
	return( *new != NULL );
}


/**************************************
*	Logical Or hitmap 1 with hitmap 2
*	Return value:	none
*	Side-effect:	Result of logical Or in hmHdl1
*/

void OrHitmaps(HitmapHdl hmHdl1, HitmapHdl hmHdl2)
{
	register long	i;
	long				size = GetHandleSize((Handle)hmHdl1)/4;
	
	if(hmHdl1 == hmHdl2)
		return;
		
	for(i=0;i<size;++i)
		(*hmHdl1)[i] |= (*hmHdl2)[i];
}


/**************************************
*	Logical And hitmap 1 with hitmap 2
*	Return value:	none
*	Side-effect:	Result of logical And in hmHdl1
*/

void AndHitmaps(HitmapHdl hmHdl1, HitmapHdl hmHdl2)
{
	register long	i;
	long				size = GetHandleSize((Handle)hmHdl1)/4;
	
	if(hmHdl1 == hmHdl2)
		return;

	for(i=0;i<size;++i)
		(*hmHdl1)[i] &= (*hmHdl2)[i];
}

/**************************************
*	Logical Not hitmap
*	Return value:	none
*	Side-effect:	Result of logical Not in hmHdl
*/

void NotHitmap(HitmapHdl hmHdl)
{
	register long	i;
	long				size = GetHandleSize((Handle)hmHdl)/4;
	
	if(hmHdl == NULL)
		return;

	for(i=0;i<size;++i)
		(*hmHdl)[i] = ~(*hmHdl)[i];
}