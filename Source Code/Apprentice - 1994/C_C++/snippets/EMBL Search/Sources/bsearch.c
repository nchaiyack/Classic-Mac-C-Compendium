/*
*********************************************************************
*	
*	bsearch.c
*	Disked-based binary search in index file on CD
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

#include "EMBL-Search.h"
#include "EMBL-Search.rsrc.h"

/*
******************************* Prototypes ***************************
*/

#include "bsearch.h"
#include "util.h"


/*
******************************** Global variables ********************
*/

extern char gError[256];


/**************************************
*	General binary search routine
*	This routine looks for "key" in file "fName" (assuming this file is in the index
*	directory). Record size is specified by "size" and "nrec" gives the number of
*	records. Comparison is done by "*compare".
*	Return value:	TRUE, if key found
*					FALSE, if not 
*	Side-effect: 	*rec contains the hit record and *hitrec the record number
*/

Boolean CDIndex_BSearch(void *key,StringPtr fName, short fd,
						u_long size, u_long nrec,
						short (*compare)(void *,void *), void *rec,u_long *hitrec)
{
	register long	max;
	register long	min;
	register long	pos;
	long			count;
	OSErr			err;
	short			comp;
		
	max = (long)nrec - 1;
	min = 0L;
	comp = 0;
	
	/* binary search */
	
	do {
		RotateWaitCursor();
		pos=(min+max)/2;
		count=size;
		if( (err=SetFPos(fd,fsFromStart,pos*size+sizeof(Header))) != noErr) {
			sprintf(gError,LoadErrorStr(ERR_READFILE,FALSE),
						PtoCstr(fName),err );
			CtoPstr((char *)fName);
			return(ErrorMsg(0));
		}

		if( ReadMacFile(fd,&count,rec,fName, TRUE) ) {
			return(FALSE);
		}
		
		if( (comp = (*compare)(key,rec)) < 0)
			max=pos-1;
		else
			min=pos+1;
	} while(comp && min <= max);
	
	*hitrec = (u_long)pos;
	return(!comp);
}
