/*
*********************************************************************
*	
*	Search.c
*	Lookup of records in target and hit file
*		
*	Rainer Fuchs
*	EMBL Data Library
*	Postfach 10.2209
*	D-6900 Heidelberg, FRG
*	E-mail: fuchs@embl-heidelberg.de
*
*	Copyright © 1992 EMBL Data Library
*		
**********************************************************************
*	
*/ 

#include <stdio.h>
#include <string.h>

#include "EMBL-Search.h"
#include "EMBL-Search.rsrc.h"

/*
******************************* Prototypes ***************************
*/

#include "search.h"
#include "pstr.h"
#include "util.h"
#include "bsearch.h"
#include "hitstorage.h"

static Boolean MarkHits(short fd, StringPtr fName, TargetRec *myTrgRec, u_long nhits, HitmapHdl hitmapHdl);




/*
********************************* Global variables *********************
*/

extern DBInfo		gDBInfo[DB_NUM];
extern WDRec		gWindows[MAXWIN];
extern short		gResNo;
extern IndexFiles	gFileList;
extern char			gError[256];

char AndOperator[] = " AND ";
char OrOperator[] = " OR ";
char NotOperator[] = "NOT ";



HitmapHdl DoSearch(short what, StringPtr key, short dbcode)
{
	HitmapHdl	masterHitmapHdl,hitmapHdl;
	Boolean		ret;
	StringPtr	targetFName,hitFName;
	u_long		target_nrec;
	u_short		target_recsize;
	short			maxkeylen;
	char			str[256],*posOr,*posAnd,*posStop,*token;
	Boolean		done;
	Boolean		bNot;
	short			nextOp,prevOp;
	long			offset;

	pstrcpy((StringPtr)str,(StringPtr)key);
	PtoCstr((StringPtr)str);
	str2upper(rtrim(str));
	
	switch(what) {
		case ENAME_QRY:
			break;
		case ACCNUM_QRY:
			targetFName 	= gFileList.acnumTrgFName;
			hitFName			= gFileList.acnumHitFName;
			target_nrec 	= gDBInfo[dbcode].actrg_nrec;
			target_recsize = gDBInfo[dbcode].actrg_recsize;
			maxkeylen		= gDBInfo[dbcode].actrg_recsize-2*sizeof(long);
			break;
		case KEYWORD_QRY:
			targetFName 	= gFileList.keywTrgFName;
			hitFName			= gFileList.keywHitFName;
			target_nrec 	= gDBInfo[dbcode].kwtrg_nrec;
			target_recsize = gDBInfo[dbcode].kwtrg_recsize;
			maxkeylen		= gDBInfo[dbcode].kwtrg_recsize-2*sizeof(long);
			break;
		case FREETEXT_QRY:
			targetFName 	= gFileList.textTrgFName;
			hitFName			= gFileList.textHitFName;
			target_nrec 	= gDBInfo[dbcode].texttrg_nrec;
			target_recsize = gDBInfo[dbcode].texttrg_recsize;
			maxkeylen		= gDBInfo[dbcode].texttrg_recsize-2*sizeof(long);
			break;
		case AUTHOR_QRY:
			targetFName 	= gFileList.authorTrgFName;
			hitFName			= gFileList.authorHitFName;
			target_nrec 	= gDBInfo[dbcode].authortrg_nrec;
			target_recsize = gDBInfo[dbcode].authortrg_recsize;
			maxkeylen		= gDBInfo[dbcode].authortrg_recsize-2*sizeof(long);
			break;
		case TAXON_QRY:
			targetFName 	= gFileList.taxonTrgFName;
			hitFName			= gFileList.taxonHitFName;
			target_nrec 	= gDBInfo[dbcode].taxontrg_nrec;
			target_recsize = gDBInfo[dbcode].taxontrg_recsize;
			maxkeylen		= gDBInfo[dbcode].taxontrg_recsize-2*sizeof(long);
			break;
	}
			
	masterHitmapHdl = NULL;
	token = str;
	done = FALSE;
	nextOp = prevOp = BOOLEAN_NOOP;
	while(!done && *token != EOS) {
		bNot = FALSE;
		posOr = posAnd = NULL;
			
		posOr = strstr(token,OrOperator);
		posAnd = strstr(token,AndOperator);
		if(posOr != posAnd) {
			if(posOr == NULL)
				nextOp = BOOLEAN_AND;
			else if(posAnd == NULL)
				nextOp = BOOLEAN_OR;
			else if(posAnd < posOr)
				nextOp = BOOLEAN_AND;
			else
				nextOp = BOOLEAN_OR;
		
			if(nextOp == BOOLEAN_AND) posStop = posAnd;
			else posStop = posOr;

			*posStop = EOS;
		}
		else nextOp = BOOLEAN_NOOP;
						
		/* allocate new hitmap */
		if( !NewHitmap(&hitmapHdl,dbcode) ) {
			ErrorMsg(ERR_MEMORY);
			return(masterHitmapHdl);
		}
		if (masterHitmapHdl == NULL )				/* make first hitmap master hitmap */
			masterHitmapHdl = hitmapHdl;
			
		ltrim(token);
		rtrim(token);
		
		if(!strncmp(token,NotOperator,strlen(NotOperator))) {
			bNot = TRUE;
			token += strlen(NotOperator);
			ltrim(token);
		}
		
		/* search for key, populate hitmap with hits */
		if (what == ENAME_QRY)
			ret=FindEntryname(token,dbcode,hitmapHdl);
		else
			ret=FindHits(token,dbcode,hitmapHdl,
						target_recsize,targetFName,target_nrec,
						general_compare,hitFName,maxkeylen);
						
		if(!ret) { /* not found or error */
			DisposHandle((Handle)hitmapHdl);
			if(masterHitmapHdl == hitmapHdl)	/* first query */
				masterHitmapHdl = NULL;
			else if(prevOp == BOOLEAN_AND) {	/* destroy previous results if
													   previous operator was AND and
													   nothing was found in new query */
				DisposHandle((Handle)masterHitmapHdl);
				masterHitmapHdl = NULL;
			}
		}
		else {
			if(bNot)
				NotHitmap(hitmapHdl);
				
			if(prevOp != BOOLEAN_NOOP) {
				if(prevOp == BOOLEAN_OR)
					OrHitmaps(masterHitmapHdl,hitmapHdl);
				else
					AndHitmaps(masterHitmapHdl,hitmapHdl);
					
				DisposHandle((Handle)hitmapHdl);
			}
		}
		
		if(nextOp == BOOLEAN_NOOP)
			done = TRUE;
		else {
			if(nextOp == BOOLEAN_AND)
				offset = (long)strlen(AndOperator);
			else
				offset = (long)strlen(OrOperator);
				
			token = posStop + offset;
			
			if(masterHitmapHdl != NULL)
				prevOp = nextOp;
			else prevOp = BOOLEAN_NOOP;	/* in case an AND operation failed, we
											reset prevOp */
		}
	}

	return(masterHitmapHdl);
}


/**************************************
*	Search index for key.
*	We perform a binary search on CD.
*	Return value:	TRUE, if successful
*						FALSE, if an error occurred
*/

Boolean FindHits(char *key, short dbcode, HitmapHdl hitmapHdl,
					  u_short target_recsize,StringPtr targetFName,u_long target_nrec,
					  short (*compare)(void *,void *),
					  StringPtr hitFName, short maxkeylen )
{
	char				mykey[256];
	TargetRec		*myTrgRec;
	u_long			pos,oldpos,n_hits;
	long				count;
	short				i;
	short				fd1,fd2;
	OSErr				err;
	Str255			fName1,fName2;
	Boolean			bWildCardSearch,bDone;
	
	/* we pad search terms to maxlen with blanks, so we can do a direct comparison to
		records we read from index. If we didn't do so we could eg find a hit of
		"ebv" with something like "ebv123" if we did a strncmp(..,strlen(key)) or we
		would have to rtrim all records read from index in order to do a strcmp()
		(trailing blanks!)
		
		For wildcard searches we can now get rid of the trailing blanks
	*/

	strcpy(mykey,key);	/* ¥ we can speed this up ¥ */
	if(mykey[strlen(mykey)-1] == WILDCARD1) {
		bWildCardSearch = TRUE;
		mykey[strlen(mykey)-1] = EOS;
	}
	else {
		bWildCardSearch = FALSE;
		rpad(mykey,' ',maxkeylen);
	}
	
	/* allocate memory for target records */
	if ( (myTrgRec=(TargetRec *)NewPtrClear((Size)target_recsize)) == NULL)
		return(ErrorMsg(ERR_MEMORY));

	/* Open index  */
	pstrcpy(fName1,targetFName);
	if( OpenMacFileReadOnly(fName1,gDBInfo[dbcode].InxWDRefNum,&fd1,TRUE) != noErr ) {
		DisposPtr((Ptr)myTrgRec);
		return(FALSE);
	}

	/* do the binary search */
	if( !CDIndex_BSearch(str2upper(mykey),
								fName1,
								fd1,
								(long)target_recsize,
								target_nrec,
								compare,
								myTrgRec, &pos) ) {
		DisposPtr((Ptr)myTrgRec);
		FSClose(fd1);
		return(FALSE);
	}
	n_hits = ConvertLong(&myTrgRec->nhits);
	
	RotateWaitCursor();
	
	/* Open hit file */
	pstrcpy(fName2,hitFName);
	if ( OpenMacFileReadOnly(fName2,gDBInfo[dbcode].InxWDRefNum,&fd2,TRUE) != noErr ) {
		FSClose(fd1);
		DisposPtr((Ptr)myTrgRec);
		return(FALSE);
	}
	
	/* Mark hits in hitmap */
	if( !MarkHits(fd2,fName2,myTrgRec,n_hits,hitmapHdl) ) {
		FSClose(fd1);
		FSClose(fd2);
		DisposPtr((Ptr)myTrgRec);
		return(FALSE);
	}
	
	/* Wildcard search:
		After we found a matching record during the binary search, we step backwards
		as long as we find matching records. Then we do the same in the other direction.
		This strategy is primitive, of course, and should be improved.
	*/
	
	if( bWildCardSearch ) {
		oldpos = pos;				/* store current position */
		
		/* Now go backwards in index, looking for matches */
		bDone = FALSE;
		while( !bDone && pos > 0 ) {
			--pos;
			if( (err=SetFPos(fd1,fsFromStart,
						pos*(long)target_recsize+sizeof(Header))) != noErr) {
				sprintf(gError,LoadErrorStr(ERR_READFILE,FALSE),
						PtoCstr(fName1),err );
				CtoPstr((char *)fName1);
				FSClose(fd1);
				FSClose(fd2);
				DisposPtr((Ptr)myTrgRec);
				return(ErrorMsg(0));
			}

			count = (long)target_recsize;
			if( ReadMacFile(fd1,&count,(void *)myTrgRec,fName1,TRUE) ) {
				FSClose(fd1);
				FSClose(fd2);
				DisposPtr((Ptr)myTrgRec);
				return(FALSE);
			}
				
			if(compare(mykey,myTrgRec))
				bDone = TRUE;
			else {
				n_hits = ConvertLong(&myTrgRec->nhits);

				if( !MarkHits(fd2,fName2,myTrgRec,n_hits,hitmapHdl) ) {
					FSClose(fd1);
					FSClose(fd2);
					DisposPtr((Ptr)myTrgRec);
					return(FALSE);
				}
			}
		}
		
		pos = oldpos;				/* jump to starting position */
		
		/* and go forwards in index, looking for matches */
		bDone = FALSE;
		while (!bDone && pos < target_nrec - 1) {
			++pos;
			if( (err=SetFPos(fd1,fsFromStart,
						pos*(long)target_recsize+sizeof(Header))) != noErr) {
				sprintf(gError,LoadErrorStr(ERR_READFILE,FALSE),
						PtoCstr(fName1),err );
				CtoPstr((char *)fName1);
				FSClose(fd1);
				FSClose(fd2);
				DisposPtr((Ptr)myTrgRec);
				return(ErrorMsg(0));
			}

			count = (long)target_recsize;
			if( ReadMacFile(fd1,&count,myTrgRec,fName1,TRUE) ) {
				FSClose(fd1);
				FSClose(fd2);
				DisposPtr((Ptr)myTrgRec);
				return(FALSE);
			}
				
			if(compare(mykey,myTrgRec))
				bDone = TRUE;
			else {
				n_hits = ConvertLong(&myTrgRec->nhits);

				if( !MarkHits(fd2,fName2,myTrgRec,n_hits,hitmapHdl) ) {
					FSClose(fd1);
					FSClose(fd2);
					DisposPtr((Ptr)myTrgRec);
					return(FALSE);
				}
			}
		} while ( !bDone );
	}
	
	FSClose(fd1);
	FSClose(fd2);
	DisposPtr((Ptr)myTrgRec);
	return(TRUE);
}

/**************************************
*	Find entryname in index.
*	We perform a binary search on CD.
*	Return value:	TRUE, if successful
*						FALSE, if an error occurred
*/

Boolean FindEntryname(char *key, short dbcode, HitmapHdl hitmapHdl)
{
	char		entryName[ENTRYNAMELEN+1];
	u_long	pos,oldpos;
	Boolean	ret;
	EnameRec	rec;
	Boolean	bWildCardSearch,bDone;
	short		fd;
	Str255	indexName;
	long		count;
	OSErr		err;
	
	/* we pad entrynames to maxlen with blanks, so we can do a direct comparison to
		records we read from index. If we didn't do so we could eg find a hit of
		"ebv" with something like "ebv123" if we did a strncmp(..,strlen(key)) or we
		would have to rtrim all records read from index to in order to do a strcmp()
		(trailing blanks!)
	
	For wildcard searches we don't need the trailing blanks
	*/
	
	strncpy(entryName,key,ENTRYNAMELEN);	/* ¥ we can speed this up ¥ */
	entryName[ENTRYNAMELEN] = EOS;
	if(entryName[strlen(entryName)-1] == WILDCARD1) {
		bWildCardSearch = TRUE;
		entryName[strlen(entryName)-1] = EOS;
	}
	else {
		bWildCardSearch = FALSE;
		rpad(entryName,' ',ENTRYNAMELEN);
	}
	
	/* Open index  */
	pstrcpy(indexName,gFileList.enameIdxFName);
	if( OpenMacFileReadOnly(indexName,gDBInfo[dbcode].InxWDRefNum,&fd,TRUE) != noErr )
		return(FALSE);

	/* do the binary search */
	if( !CDIndex_BSearch(str2upper(entryName),
								indexName,
								fd,
								sizeof(EnameRec),
								gDBInfo[dbcode].ename_nrec,
								ename_compare,
								&rec, &pos) ) {
		FSClose(fd);
		return(FALSE);
	}
	
	/* Mark hit in hitmap */
	BitSet((Ptr)*hitmapHdl,pos);
	
	/* Wildcard search:
		After we found a matching record during the binary search, we step backwards
		as long as we find matching records. Then we do the same in the other direction
	*/

	if( bWildCardSearch ) {
		oldpos = pos;				/* store current position */
		
		/* Now go backwards in index, looking for matches */
		bDone = FALSE;
		while(!bDone && pos > 0) {
			--pos;
			if( (err=SetFPos(fd,fsFromStart,
						pos*sizeof(EnameRec)+sizeof(Header))) != noErr) {
				sprintf(gError,LoadErrorStr(ERR_READFILE,FALSE),
						PtoCstr(indexName),err );
				CtoPstr((char *)indexName);
				FSClose(fd);
				return(ErrorMsg(0));
			}

			count = sizeof(EnameRec);
			if( ReadMacFile(fd,&count,&rec,indexName,TRUE) ) {
				FSClose(fd);
				return(FALSE);
			}
				
			if(ename_compare(entryName,&rec))
				bDone = TRUE;
			else
				BitSet((Ptr)*hitmapHdl,pos);
		}
		
		pos = oldpos;				/* jump to starting position */
		
		/* and go forwards in index, looking for matches */
		bDone = FALSE;
		while(!bDone && pos < gDBInfo[dbcode].ename_nrec - 1) {
			++pos;
			if( (err=SetFPos(fd,fsFromStart,
						pos*sizeof(EnameRec)+sizeof(Header))) != noErr) {
				sprintf(gError,LoadErrorStr(ERR_READFILE,FALSE),
						PtoCstr(indexName),err );
				CtoPstr((char *)indexName);
				FSClose(fd);
				return(ErrorMsg(0));
			}

			count = sizeof(EnameRec);
			if( ReadMacFile(fd,&count,&rec,indexName,TRUE) ) {
				FSClose(fd);
				return(FALSE);
			}
				
			if(ename_compare(entryName,&rec))
				bDone = TRUE;
			else
				BitSet((Ptr)*hitmapHdl,pos);
		}
	}
	
	FSClose(fd);
	return(TRUE);
}
	

/**************************************
*	Read hit file and mark hits in hitmap
*	Return value:	TRUE, if successful
*						FALSE, if an error occurred
*/

static Boolean MarkHits(short fd, StringPtr fName, TargetRec *myTrgRec, u_long nhits,
						HitmapHdl hitmapHdl)
{
	u_long	hitpos;
	OSErr		err;
	long		i;
	u_long	count,pos;
	
	/* Go to first hit in hit file */
	hitpos = ConvertLong(&myTrgRec->hitPtr) - 1;
	if( (err=SetFPos(fd,fsFromStart,
				hitpos*sizeof(u_long) + sizeof(Header))) != noErr) {
		sprintf(gError,LoadErrorStr(ERR_READFILE,FALSE),
						PtoCstr(fName),err );
		return(ErrorMsg(0));
	}

	/* Now read "nhits" hits from there */
	for(i=0,count= (u_long) sizeof(u_long);i < nhits; ++i) {
		if( ReadMacFile(fd,(long *)&count,&pos,fName,TRUE) ) 
			return(FALSE);

		/* Convert hit record to C-type array index */
		ConvertLong(&pos);
		BitSet((Ptr)*hitmapHdl,--pos);
	}
	
	return(TRUE);
}



/**************************************
*	Compare routine for entryname bsearch
*/

short ename_compare(void *key,void *rec)
{
	return( strncmp((char *)key,((EnameRec *)rec)->entry_name,strlen((char *)key)) );
}


/**************************************
*	Compare routine for other bsearches
*/

short general_compare(void *key,void *rec)
{
	return( strncmp((char *)key,((TargetRec *)rec)->value,strlen((char *)key)));
}