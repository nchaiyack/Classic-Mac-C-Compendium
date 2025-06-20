/*
*********************************************************************
*	
*	Copy.c
*	Cut/copy/paste/clear
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

#include "EMBL-Search.h"
#include "EMBL-Search.rsrc.h"

/*
******************************* Prototypes ***************************
*/

#include "copy.h"
#include "util.h"
#include "hitstorage.h"
#include "sequence.h"

static void CopyResSelection(WDPtr wdp);
static void CopySequence(WDPtr wdp);



/**************************************
*	Cut dispatcher
*	Return value:	none	
*/

void CutSelection(WDPtr wdp)
{	
	if( wdp == NULL )
		return;
		
	switch( ((WindowPeek)wdp)->windowKind ) {
		case queryW:
			DlgCut((DialogPtr)wdp);
			ZeroScrap();
			TEToScrap();
			wdp->dirty = TRUE;
			break;
		case seqW:
			break;
		case resW:
			break;
	}
}


/**************************************
*	Clear dispatcher
*	Return value:	none	
*/

void ClearSelection(WDPtr wdp)
{	
	if( wdp == NULL )
		return;
		
	switch( ((WindowPeek)wdp)->windowKind ) {
		case queryW:
			DlgDelete((DialogPtr)wdp);
			wdp->dirty = TRUE;
			break;
		case seqW:
			break;
		case resW:
			break;
	}
}


/**************************************
*	Copy dispatcher
*	Return value:	none	
*/

void CopySelection(WDPtr wdp)
{	
	if( wdp == NULL )
		return;
		
	switch( ((WindowPeek)wdp)->windowKind ) {
		case queryW:
			DlgCopy((DialogPtr)wdp);
			ZeroScrap();
			TEToScrap();
			break;
		case seqW:
			CopySequence(wdp);
			break;
		case resW:
			CopyResSelection(wdp);
			break;
	}
}

/**************************************
*	Paste dispatcher
*	Return value:	none	
*/

void PasteSelection(WDPtr wdp)
{	
	if( wdp == NULL )
		return;
		
	switch( ((WindowPeek)wdp)->windowKind ) {
		case queryW:
			TEFromScrap();
			DlgPaste((DialogPtr)wdp);
			wdp->dirty = TRUE;
			break;
		case seqW:
			break;
		case resW:
			break;
	}
}


/**************************************
*	Copy selection in result window
*	Return value:	none	
*/

static void CopyResSelection(WDPtr wdp)
{
	ResultHdl		resHdl;
	CString80Hdl	bufHdl;
	HitlistHdl		hlHdl;
	SignedByte		oldState;
	short				hitPos,bufPos, found;
	long				count;
	char				*buf,*dest;
	OSErr				ret;
	Boolean			ret2;
	
	if( wdp == NULL || ((WindowPeek)wdp)->windowKind != resW )
		return;

	resHdl = (ResultHdl)(wdp->userHandle);
	bufHdl = (**resHdl).descBufHdl;
	hlHdl	 = (**resHdl).hlHdl;
	oldState=LockHandleHigh((Handle)bufHdl);
	
	/* allocate a new memory block large enough to hold all selected lines */
	count = (**resHdl).nsel * sizeof(CString80);
	if( (buf = (char *)NewPtrClear(count)) == NULL) {
		ErrorMsg(ERR_MEMORY);
		goto err;
	}
	else {	/* clear buffer */
		*buf=EOS;
		dest=buf;
	}
	 
	/* now copy hit by hit */
	for(	hitPos = 0, found = 0;
			hitPos < (**resHdl).nhits && found < (**resHdl).nsel;
			hitPos++, bufPos++) {

		/* copy to copy buffer */
		if( GetSelectState(hlHdl,hitPos) ) {
			/* refill result buffer if necessary */
			if( hitPos < (**resHdl).buftop || hitPos>= (**resHdl).buftop + MAXBUFLINES) {
				StartWaitCursor();
				ret2 = FillDEBuffer(resHdl,hitPos,FALSE);
				InitCursor();
				if( !ret )
					goto err;
			}
		
			found++;
			BlockMove((char *)((*bufHdl)[hitPos - (**resHdl).buftop])+1,dest,80L);
			dest += 80;
   		/* add newline */
			*dest++ = '\r';
		}
	}
	
	ret=ZeroScrap();
	/* Don't copy last \r */
	ret=PutScrap(--count,'TEXT',buf);

err:
	if(buf) DisposPtr((Ptr)buf);
	HSetState((Handle)bufHdl,oldState);
}


/**************************************
*	Copy sequence entry to clipboard
*	Return value:	none	
*/

static void CopySequence(WDPtr wdp)
{
	CString80Hdl	bufHdl;
	SeqRecHdl		seqRecHdl;
	SignedByte		oldState;
	short				hitPos,bufPos;
	long				count;
	char				*buf,*dest;
	unsigned short linelen;
	OSErr				ret;

	if( wdp == NULL || ((WindowPeek)wdp)->windowKind != seqW )
		return;

	seqRecHdl = (SeqRecHdl)(wdp->userHandle);
	if(seqRecHdl == NULL)
		return;
	
	/* Allocate memory block large enough to hold complete entry */
	if( (buf = (char *)NewPtrClear((**seqRecHdl).nlines * sizeof(CString80))) == NULL) {
		ErrorMsg(ERR_MEMORY);
		return;
	}
	else {	/* empty buffer */
		*buf=EOS;
		dest=buf;
	}
			
	/* Lock down sequence data */
	bufHdl = (**seqRecHdl).lineBufHdl;
	oldState=LockHandleHigh((Handle)bufHdl);
	
	bufPos = 0;
	count = 0L;
	for(hitPos = (**seqRecHdl).firstSel; hitPos <= (**seqRecHdl).lastSel; hitPos++) {
		if( hitPos < (**seqRecHdl).buftop || hitPos>= (**seqRecHdl).buftop + SEQBUFLINES ) {
			if(!FillLineBuffer(seqRecHdl,hitPos))
				break;
			else
				bufPos = 0;
		}
		
		linelen = (unsigned short)*(*bufHdl)[bufPos];
		BlockMove((char *)((*bufHdl)[bufPos])+1,dest,linelen);
		dest += linelen;
		count += linelen+1; /* +1 because of \r */
   	/* add newline */
		*dest++ = '\r';
		
		bufPos++;
	}

	ret=ZeroScrap();
	/* Don't copy last \r */
	ret=PutScrap(--count,'TEXT',buf);

err:
	if(buf) DisposPtr((Ptr)buf);
	HSetState((Handle)bufHdl,oldState);
}