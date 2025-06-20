/*
*********************************************************************
*	
*	XRef.c
*	Handling of cross-references
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
#include <string.h>
#include <ctype.h>

#include "EMBL-Search.h"
#include "EMBL-Search.rsrc.h"

/*
******************************* Prototypes ***************************
*/

#include "xref.h"
#include "util.h"
#include "hitstorage.h"
#include "search.h"
#include "sequence.h"

static Boolean NewXRefBuf(XRefHdl *new);
static void RemoveXRefsFromMenu(void);



/*
******************************** Global variables *****************
*/

extern MenuHandle	gMenu[MENUNR];
extern WDRec		gWindows[MAXWIN];
extern IndexFiles	gFileList;
extern DBInfo		gDBInfo[DB_NUM];


/**************************************
*	When user selected an item from the XREF menu we do an acc# search for
*	this entry and open a new window.
*	Return value:	TRUE, if successful
*						FALSE, if an error occurred
*/

Boolean OpenXRefs(char *name,short dbcode)
{
	char			key[ACCNOLEN+1];
	HitmapHdl	hitmapHdl;
	Boolean		ret;
	ResultHdl	resHdl;
	register short i;
	
	strncpy(key,name,ACCNOLEN);
	key[ACCNOLEN] = EOS;
	
	/* start new hitmap */
	if( !NewHitmap(&hitmapHdl,dbcode) )
		return( ErrorMsg(ERR_MEMORY) );

	StartWaitCursor();
	
	/* search for key, populate hitlist with hits */
	ret=FindHits(key,dbcode,hitmapHdl,
					gDBInfo[dbcode].actrg_recsize,gFileList.acnumTrgFName,
					gDBInfo[dbcode].actrg_nrec,
					general_compare,gFileList.acnumHitFName,
					gDBInfo[dbcode].actrg_recsize-2*sizeof(long));

	if(!ret) { /* not found or error */
		DisposHandle((Handle)hitmapHdl);
		return( ErrorMsg(ERR_NOTFOUND) );
	}
	else {	/* found! */
		/* get a new result record and initialise it */
		if( !InitResultRec(&resHdl,dbcode,hitmapHdl,NULL) ) {
			DisposHandle((Handle)hitmapHdl);
			return( ErrorMsg(ERR_MEMORY) );
		}
		else {
			/* fill short description buffer */
			StartWaitCursor();
			ret = FillDEBuffer(resHdl,0,FALSE);
			InitCursor();
			if(ret) {
				for(i=0;i < (**resHdl).nhits; ++i) {
					if( (ret=NewSequenceWindow(resHdl,i)) == FALSE )
						break;
				}
			}
			DisposeResRec(resHdl);
			InitCursor();
			return(ret);
		}
	}
}


/**************************************
*	Every DR line passed to this routine is checked for EMBL or SWIS-PROT cros-references.
*	Information from each xref is extracted and stored into an XRefRec.
*	Return value:	TRUE, if successful
*						FALSE, if an error occurred
*/

Boolean ParseDRLine(SeqRecHdl seqRecHdl, char *line)
{
	char			*pos1,*pos2,*pos;
	XRefPtr		xrefPtr;
	Boolean		ret;
	SignedByte	oldState;
	Size			newSize;
	char			buf[ACCNOLEN+1];
	short			i,n;
	
	pos1=pos2=NULL;
	
	if( (pos1=strstr(line,"SWISS-PROT")) == NULL &&
		 (pos2 = strstr(line,"EMBL")) == NULL )
		 return(TRUE);
	else {
		oldState = MyHLock((Handle)seqRecHdl);
		
		/* create a new xref block, if necessary */
		if( (**seqRecHdl).xrefHdl == NULL) {
			if ( !NewXRefBuf(&(**seqRecHdl).xrefHdl) ) {
				HSetState((Handle)seqRecHdl,oldState);
				return(ErrorMsg(ERR_MEMORY));
			}
			
			n = 1;
		}
		else {	/* grow xref block */
			newSize = GetHandleSize((Handle)(**seqRecHdl).xrefHdl) + sizeof(XRef);
			SetHandleSize((Handle)(**seqRecHdl).xrefHdl, newSize);
			if(MemError() != noErr)  {
				HSetState((Handle)seqRecHdl,oldState);
				return(ErrorMsg(ERR_MEMORY));
			}
			
			n = newSize / sizeof(XRef);
		}
		
		--n;	/* change to C array offset */
		
		HLock((Handle)(**seqRecHdl).xrefHdl);
		xrefPtr = *(**seqRecHdl).xrefHdl;
		
		if (pos1 != NULL) {
			pos = pos1;
			xrefPtr[n].dbcode = DB_SWISS;
		}
		else {
			pos = pos2;
			xrefPtr[n].dbcode = DB_EMBL;
		}
				
		while(*pos++ != ';') ;	/* skip db name */
		while(isspace(*pos)) ++pos;	/* skip white space */
		for( i=0; *pos != ';' && i < ACCNOLEN; ++pos,++i )
			xrefPtr[n].accno[i] = *pos;
		xrefPtr[n].accno[i] = EOS;
		while(*pos++ != ';') ; /* go to entryname */
		while(isspace(*pos)) ++pos;
		for( i=0; *pos != '.' && i < ENTRYNAMELEN; ++pos,++i)
			xrefPtr[n].ename[i] = *pos;
		xrefPtr[n].ename[i] = EOS;
		
		HUnlock((Handle)(**seqRecHdl).xrefHdl);
		HSetState((Handle)seqRecHdl,oldState);
		
		return(TRUE);
	}
}

/**************************************
*	Allocate memory for new xref buffer
*	Return value:	TRUE, if successful
*						FALSE, if an error occurred
*/

static Boolean NewXRefBuf(XRefHdl *new)
{
	*new = (XRefHdl)NewHandleClear(sizeof(XRef));
	return( *new != NULL );
}


/**************************************
*	Add one menu item for each cross-reference to XREF hierarchical menu
*/

void AddXRefsToMenu(WDPtr wdp)
{
	XRefHdl		xrefHdl;
	SeqRecHdl	seqRecHdl;
	short			nxref;
	short			i;
	char			temp[256];
	SignedByte	oldState;
	
	if(wdp == NULL || ((WindowPeek)wdp)->windowKind != seqW)
		return;
		
	/* remove old xrefs */
	RemoveXRefsFromMenu();
	
	if( (seqRecHdl = (SeqRecHdl)(wdp->userHandle)) == NULL )
		return;
		
	if( (xrefHdl = (**seqRecHdl).xrefHdl) == NULL )
		return;
		
	oldState = MyHLock((Handle)xrefHdl);
	
	nxref = (short)(GetHandleSize((Handle)xrefHdl)/sizeof(XRef));
	for( i = 0; i < nxref; ++i ) {
		strcpy(temp,(*xrefHdl)[i].accno);
		strcat(temp," [");
		strcat(temp,(*xrefHdl)[i].ename);
		strcat(temp,"]");
		CtoPstr(temp);
		AppendMenu(gMenu[XREF],(StringPtr)temp);
		EnableItem(gMenu[XREF],i+2);
	}
	
	HSetState((Handle)xrefHdl,oldState);
}

/**************************************
*	Remove all cross-references from XREF menu
*/

static void RemoveXRefsFromMenu()
{
	short i;
	
	for(i = CountMItems(gMenu[XREF]); i > 2 ; --i)
		DelMenuItem(gMenu[XREF],i);
}


