#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#ifdef CMLmake
	#include "CML.h"
	#include "CMLglobs.h"
#else
	#include "MDL.h"
	#include "MDLglobs.h"
#endif

#ifdef MAC
	#include <types.h>
	#include <memory.h>
	#include <OSUtils.h>
	#include <quickdraw.h>
	#include <math.h>
	#include <files.h>
	#include <strings.h>
#endif

#ifdef DOS
	#include <search.h>
#endif

/* macros specific to here */
#define getNextPlace(li) (struct nameRec*)nextFromLList(li)+(li->current&BlockLo)


/* functions in CMLmem.c */
struct sList *sListAlloc(int,size_t,int);
struct lList *lListAlloc(int,int,int,int,int);
void tempRead(void *,long,int);
long tempWrite(void *,long,int);
void *pageSList(struct sList *);
void *storeList(struct sList *);
void *nextFromLList(struct lList *);
void *getLListBuf(int, struct lList *);
void freeSL(struct sList *);
void _freeLL(int, struct lList *);
void memsetLL(struct lList *,char);
int newSegmentRec(int, char *,unsigned short *);
int findSegment(int, char *);
struct segmentRec *lookUpSR(int,char *);
struct fieldRec *newField(int Field);
void findLit(struct parseRec *);
void lookUpLit(struct parseRec *);
int addToFieldList(struct segmentRec *);
int lowerList(int);
int raiseList(int);
void trimMemory(void);
void *emalloc(size_t);
void *ecalloc(size_t,size_t);
void *erealloc(void *,size_t);
void *hemalloc(size_t,Handle *);
void *hecalloc(size_t, size_t,Handle *);
void *hcalloc(size_t, size_t,Handle *);
void *hmalloc(size_t,Handle *);
int findName(char *);
struct locnRec *setUpDataSpace(int,indexType);
struct locnRec *_getMainData(struct segmentRec *,int,struct subSegRec **);
struct dimData *findDims(struct locnRec *);
void clearMem(void);
struct segmentRec *nextSegment(struct segmentRec *,struct fieldRec **);
#ifndef MAC
void DisposeHandle(Handle);
#endif
#ifdef MDLView
Handle makeHandle(void *,size_t);
Handle makeRecon(struct reconstruction *);
int disposeLList(Handle);
int fetchRecon(Handle recon);
int MDLLastError(int);

int MDLDispose(Handle recon);
/*int MDLDraw(Handle recon, extents,struct transform *trans); */
/*int MDLWhichSegment(Handle recon, struct cartData *point); */
struct segmentRec *MDLGetSegment(Handle recon, int segmentIndex, struct segmentRec *seg);
/*======================= Minimum routines for drawing =====================*/
int MDLSegmentsInRecon(Handle recon);
int MDLDrawSegment(Handle recon,int segmentIndex,struct transform *trans);
Point MDLTransformPoint(struct transform *trans,struct c_locn *data);
Point MDLTransformSPoint(struct transform *trans,struct cartData *data);
int MDLFillMatrix(struct transform *trans);
int MDLRotate(struct transform *trans,c_float x,c_float y,c_float z);
int MDLRotateRelative(struct transform *trans,c_float x,c_float y,c_float z);
int MDLTranslate(struct transform *trans,c_float dx,c_float dy,c_float dz);
int MDLTranslateRelative(struct transform *trans,c_float dx,c_float dy,c_float dz);
int MDLScale(struct transform *trans,c_float mag);
int MDLScaleRelative(struct transform *trans,c_float mag);

/*=========================================================================*/
/*int MDLDrawPoint(Handle recon,int segmentIndex,int point,struct transform *trans); */
/*int MDLNextPoint(Handle recon, int segmentIndex); */
/*int MDLNextNode(Handle recon, int segmentIndex); */
/*int MDLNextSegment(Handle recon); */
/*int MDLNextField(Handle recon); */
/*int MDLUnLockSegment(Handle recon,int segmentIndex); */
/*int MDLUnLockField(Handle recon, int fieldIndex); */
/*int MDLUnLockRecon(Handle recon); */
/*int MDLLockSegment(Handle recon,int segmentIndex); */
/*int MDLLockField(Handle recon, int fieldIndex); */
/*int MDLLockRecon(Handle recon); */
/*int MDLReportSegment(Handle recon, int segmentIndex, int how); */
/*int MDLDrawLine(point1,point2); */
#endif
void saveSnapShot(int a, char *b[]);


/*--------------------------- The List Routines -----------------------------

	Memory management is through lists of indexes into blocks of data.
	
	Blocks are paged to disk when necessary, and retrieved again when
	needed. There are two kinds of lists that establish make a kind of 
	virtual memory: sLists for sequential (write-only) data, and lLists 
	for random-access data. 
	
	There is a special list, the FieldList, that has nothing to do with
	memory management, but is a hash into the segment records.
	
	The routines are:
		sListAlloc and lListAlloc allocate data and list space for both 
			types of lists.
		storeList pages an sList data block to disk.
		tempRead and tempWrite are for the paging I/O.
		getLListBuf delivers a lList datablock, returning it from disk,
			if necessary.
		nextFromLList delivers the lList datablock for a new data element.
		(the corresponding routine for sLists is the macro nextFromList.)
		freeSL and freeLL page all data blocks to disk and free all data
			block memory.
		lowerList and raiseList resize the FieldList.
		
----------------------------------------------------------------------------*/

struct sList *sListAlloc(int nItemsInBlock,size_t structSize,int initialListLen)
{
	struct sList *theList;
	theList = emalloc(sizeof(struct sList));
	theList->current = 0;
	theList->other = 0;
	theList->max = initialListLen;
	theList->size=nItemsInBlock*structSize;
	theList->buffer = ecalloc(nItemsInBlock,structSize);
	theList->position = emalloc(initialListLen*sizeof(long));
	return(theList);
}


struct lList *lListAlloc(int numBuffers,int nItemsInBlock,int structSize,
	int initialListLen,int listID)
{
	struct lList *theList;
	int i;
	
	numBuffers=(numBuffers>3)? 3:numBuffers;
	numBuffers=0;
	initialListLen=(numBuffers>initialListLen)? numBuffers:initialListLen;

	theList = ecalloc(1,sizeof(struct lList));
	theList->nbuffers = numBuffers;
	theList->code = (numBuffers)?numBuffers:3;
	/* Note: nbuffers is the number of buffers actually present in memory.
		It is constantly maintained. The lower 2 bits of code is 1, 2, or
		3: the minimum number of buffers that should be kept in memory.
		This number is my best estimate of how each data structure is used
		and is selected to best avoid thrashing. The digits above these are
		flag bits.
	*/
	theList->thisHit = 0;
	theList->faults = 0;
	theList->currentList = 0;
	theList->size = nItemsInBlock * structSize;
	theList->nItems = nItemsInBlock;
	theList->current = 0;
	theList->other = 0;
	theList->master = listID;
	theList->nLE = initialListLen;
	theList->maxE = initialListLen * nItemsInBlock;
	theList->list = ecalloc(initialListLen,sizeof(struct listElement));
	if(listID>=0) {
		MastersArray[listID].p=theList;
		MastersArray[listID].h=0;
		MastersArray[listID].other=0;
	}
	if(numBuffers==0)
		return(theList);
	for(i=0;i<numBuffers;i++) 
		theList->list[i].ptr = hecalloc(1,theList->size,&(theList->list[i].handle));
	return (theList);
}

void tempRead(void *buf,long pos,int size)
{
/*	fprintf(OutFile,"pos,size:\t%d\t%d\n",pos,size);	====*/
	if(pos>=0) {		/* -1 is a flag: read at present position. */
		if(fseek(Tf,pos,SEEK_SET))
			errorAlert(DiskErr);
	}
	if(fread(buf,size,1,Tf) != 1)
		errorAlert(DiskErr);
	NrTempReads++;
	return;
}

long tempWrite(void *buf,long pos,int size)
{
	int e,d;
	char *p;
#ifdef MAC
	OSErr err;		
	FInfo fndrInfo;
#endif

	
/*	fprintf(OutFile,"TempWrite, pos,size:\t%d\t%d\n",pos,size);	====*/
	NrTempWrites++;
	if(!Tf) {			/* If tempfile is not open, open it. */
		if(SnapShot) {
			char scratch[256];
			strcpy(scratch,IFileList[0]);
			if(p=strrchr(scratch,'.'))
				*p='\0';
			strcat(scratch,".bin");
			Tf=fopen(scratch,"w+");
#ifdef MAC
			err=GetFInfo(c2pstr(scratch),0,&fndrInfo);
			if(err==noErr) {
				fndrInfo.fdType=0x434D4C42;		/* 'CMLB' */
  				fndrInfo.fdCreator=0x4D525047;	/* 'MRPG' */
				SetFInfo(scratch,0,&fndrInfo);
			}
			p2cstr(scratch);
#endif
		} else
			Tf=tmpfile();
		if(!Tf)
			errorAlert(NoTempFile);
		rewind(Tf);
		fwrite(&e,64*sizeof(long),1,Tf);	/* So position not zero and room to
												write a header */
	}
	if(pos==-1) {		/* -1 is a flag: no existing image on disk. Put this */
		e=fseek(Tf,0L,SEEK_END);	/* at end of tempfile. */
		pos=ftell(Tf);
	} else 
		e=fseek(Tf,pos,SEEK_SET);	/* or put it at pos. */
	if (e)
		errorAlert(DiskErr);
	d=fwrite(buf,size,1,Tf);	/* Do the write. */
	if(d!=1)
		errorAlert(DiskErr);
	return(pos);
}

void *pageSList(struct sList *theList)	/* sList buffer paged to disk */
{
	if (theList->current>>BlockHi >= theList->max) {
			/* Extend the array of longs containing file positions */
		theList->max += ListSizeIncr;
		theList->position=erealloc(theList->position,theList->max*sizeof(long));
	}
	
	/*---------------------------------------------------------------------------
		This is called exclusively from the nextFromList macro, where the value of 
		->current. is a multiple of 64 (BlockLo+1). We are saving the buffer just
		behind the one pointed to by ->current.
	----------------------------------------------------------------------------*/

	theList->position[(theList->current >> BlockHi)-1]=tempWrite(theList->buffer,-1L,theList->size);
	memset(theList->buffer,0,theList->size);	/* Null the buffer. Locn's depend on this. 
													It's cheap, during a disk access. */
	return(theList->buffer);					/* The index is 0, so the 1st element of 
													block will be accessed. */
}		

void *storeList(struct sList *theList)	/* sList store */
{
	if ((theList->current >> BlockHi) >= theList->max) {
		theList->max += ListSizeIncr;
		theList->position=erealloc(theList->position,theList->max*sizeof(long));
	}
	
	theList->position[theList->current >> BlockHi]=tempWrite(theList->buffer,-1L,theList->size);
	return(theList->buffer);					/* The index is 0, so the 1st element of 
													block will be accessed. */
}		

/*--------------------------- nextFromLList ------------------------------------

 	This returns a pointer to the BEGINNING of the data block that contains
	the next-indexed data structure. It does not add the offset into that block
	so as to avoid the overhead having to calculate offset and BlockLo mask from
	what it deducts about the sizes of the particular list's data elements. It
	is more than just a call to getLListBuf() since it will extend the data into
	memory or disk space, depending upon whether the former is available.
	
------------------------------------------------------------------------------*/

void *nextFromLList(struct lList *theList)
{
	theList->current++;
	return(getLListBuf(theList->current,theList));
}

#if 0
void *nextFromLList(struct lList *theList)
{
	struct listElement *le,*li;
	unsigned long oldHit;
	int i,oldest;
	if((++theList->current) % theList->nItems) 	/* data block is somewhere */
		return(getLListBuf(theList->current,theList));
	/* else need a new data block */
	if(theList->current >= theList->maxE) {	/* list too small */
		theList->nLE += ListSizeIncr;
		theList->maxE += ListSizeIncr * theList->nItems;
		theList->list=erealloc(theList->list,theList->nLE*sizeof(struct listElement));
		for(i=1;i<=ListSizeIncr;i++) {
			le=theList->list+theList->nLE-i;
			le->ptr=0;
			le->handle=0;
			le->position=0;
			le->hit=0;
		}
	}
	/* initialize the next list element */
	theList->currentList=theList->current/theList->nItems;
	le=theList->list+theList->currentList;
	if(le->ptr=hcalloc(1,theList->size,&(le->handle))) {	/* space available  */
	
		theList->nbuffers++;
		le->position=0;	/* No space on disk yet for new block. */
	} else {					/* not enough room, must swap */
		oldHit = theList->thisHit;
		theList->faults++;
		for (i=0,oldest=-1;i<theList->nLE;i++) {
			li=theList->list+i;
			if (li->ptr && (le->hit < oldHit)) {
				oldest=i;
				oldHit=li->hit;
			}
		}
		if(oldest<0)
			errorAlert(OutOfMemory);
		li=theList->list+oldest;
		li->position=(li->position)? tempWrite(li->ptr,li->position,theList->size)
			: tempWrite(li->ptr,-1L,SegList->size);  
		le->ptr=li->ptr;			/* Take its space */
		le->handle=li->handle;
		li->ptr=0;				/* and show that it's not in heap. */
		li->handle=0;
		le->position=0;			/* No space on disk yet for new block. */
	}
	le->hit = ++theList->thisHit;
	return(le->ptr);
}
#endif

/*--------------------------------------------------------------------
	getLListBuf() returns a pointer to the requested long list
	buffer. It will create a new buffer if the requested n is less
	than theList->current (i.e., reserved, unallocated space).
----------------------------------------------------------------------*/

void *getLListBuf(int n,struct lList *theList)
{
	struct listElement *le,*li;
	unsigned long oldValue;
	int i,oldest;
	
	if(!theList) {
		errorAlert(UnexpectedError);
		return 0;
	}

	if(n>theList->current || n<1) {
/* #ifdef DEBUG		*/
		fflush(OutFile);
		fprintf(ErrFile,"Invalid index: %d in %s list\n",n,*(LListNames+theList->master));
/*		clearMem();	*/
/* #endif	*/
		errorAlert(PagingError);	/* Fatal */
	}
	if((theList->currentList=n/theList->nItems)>=theList->nLE) {
		/* 	Added late (11-may-91) so that an incompletely allocated 
			list will extend itself under all conditions. Changed 8 May 92.*/
		theList->list=erealloc(theList->list,(theList->currentList+1)*sizeof(struct listElement));
		for(i=theList->nLE;i<=theList->currentList;i++) {
			le=theList->list+i;
			le->ptr=0;
			le->handle=0;
			le->position=0;
			le->hit=0;
			theList->maxE+=theList->nItems;
		}
		theList->nLE=theList->currentList+1;
	}
	le=theList->list+theList->currentList;
	if (le->ptr) {		/* Exists in heap. */
		le->hit = ++theList->thisHit;
		return(le->ptr);
	}
	if (le->position) {	/* != NULL means it exists in temp file */
		if(le->ptr=hmalloc(theList->size,&(le->handle))) {	/* space available */
			tempRead(le->ptr,le->position,theList->size);
			le->hit = ++theList->thisHit;
			theList->nbuffers++;
			return(le->ptr);
		}		/* not enough room, must swap */
		oldValue = theList->thisHit;
		theList->faults++;
		for (i=0,oldest=-1;i<theList->nLE;i++) {
			li=theList->list+i;
			if (li->ptr && (li->hit < oldValue)) {
				oldest=i;
				oldValue=li->hit;
			}
		}
		if(oldest>=0) {
			li=theList->list+oldest;
			li->position=tempWrite(li->ptr,(li->position)?li->position:-1L,theList->size);
			le->ptr=li->ptr;	/* take its space */
			le->handle=li->handle;
			li->handle=0;
			li->ptr=0;			/* and show that it's not in heap. */
			tempRead(le->ptr,le->position,theList->size);
			le->hit = ++theList->thisHit;
			return(le->ptr);
		}
		/* Can't swap. Must try to trim memory. */
		if(le->ptr=hemalloc(theList->size,&(le->handle))) { /* There is space, after trimming. */
			tempRead(le->ptr,le->position,theList->size);
			le->hit = ++theList->thisHit;
			theList->nbuffers++;
			return(le->ptr);
		}
		errorAlert(OutOfMemory);
		return 0;
	}
	
	/* Gets here if neither in heap on in tempFile.  */
	if(le->ptr=hcalloc(1,theList->size,&(le->handle))) { /* space available */
		le->hit = ++theList->thisHit;
		theList->nbuffers++;
		return(le->ptr);
	}
	oldValue = theList->thisHit;
	theList->faults++;
	for (i=0,oldest=-1;i<theList->nLE;i++) {
		li=theList->list+i;
		if (li->ptr && (li->hit < oldValue)) {
			oldest=i;
			oldValue=li->hit;
		}
	}	/* must take an existing buffer */
	if(oldest>=0) {	/* Success. There is space to swap into. */
		li=theList->list+oldest;
		li->position=(li->position)? tempWrite(li->ptr,li->position,theList->size)
			: tempWrite(li->ptr,-1L,theList->size);
		le->ptr=li->ptr;	/* take its space */
		le->handle=li->handle;
		li->handle=0;
		li->ptr=0;			/* and show that it's not in heap. */
		strncpy(le->ptr,0,theList->size);	/* null it */
		le->hit = ++theList->thisHit;
		return(le->ptr);
	}
	/* Can't swap. Must try to trim memory. */
	if(le->ptr=hecalloc(1,theList->size,&(le->handle))) { /* There is space, after trimming. */
		le->hit = ++theList->thisHit;
		theList->nbuffers++;
		return(le->ptr);
	}
	errorAlert(OutOfMemory);
	return 0;
}

void freeSL(struct sList *theList)
{
	storeList(theList);
	free(theList->buffer);
	theList->buffer = NULL;
	return;
}

/*--------------------------------------------------------------------
	_freeLL() either purges, frees, or flushes a longList. Macro-ized 
	calls of freeLL, flushLL, and purgeLL work through this routine.
	Flush, flushes all buffers to disk, leaving the list and buffer intact.
	Free, flushes and also frees the buffer memory. The list remains intact.
	Purge, discards all data and frees all memory allocated for the list.
	_freeLL() works with pointers. If handles are being used, they must
	be locked before calling _freeLL() and the pointer fields properly
	filled. It will check for handles and disposes them rather than 
	freeing pointers.
	
	memsetLL(struct lList *theList,char val) does just that. It calls
	memset for each buffer in the lList.
----------------------------------------------------------------------*/

void _freeLL(int freeFlag,struct lList *theList)
{
	int i,numLE;
	struct listElement *le;
	numLE=(theList->current/theList->nItems)+1;
	for(i=0;i<numLE;i++) {
		le=theList->list+i;
		if(le->ptr) {
			if(freeFlag!=-1) { /* free (1) or Flush (0) */
				if (le->position)
					tempWrite(le->ptr,le->position,theList->size);
				else
					le->position=tempWrite(le->ptr,-1L,theList->size);
			}
			if(freeFlag) {	/* Purge (-1) or Free (1) */
				if(le->handle)
					DisposeHandle(le->handle);
				else
					free(le->ptr);
			}
			le->ptr=0;
			le->handle=0;
		}
	}
	if(freeFlag == -1) {	/* Purge (-1) */
		if(theList->hList)
			DisposeHandle(theList->hList);
		else
			free(theList->list);
		if(theList->master<0) 
			free(theList);
		else {
			if(MastersArray[theList->master].h) {
				DisposeHandle(MastersArray[theList->master].h);
				
			} else
				free(theList);
			MastersArray[theList->master].p=0;
			MastersArray[theList->master].h=0;
		}
	}
	return;
}

void memsetLL(struct lList *theList,char val)
{
	int i;
	char *p;
	
	if(theList->current) {
		p=(char *)getLListBuf(1,theList);
		memset(p,val,theList->size);
		for(i=theList->nItems;i<=theList->current;i+=theList->nItems) {
			p=(char *)getLListBuf(i,theList);
			memset(p,val,theList->size);
		}
	}
}

#ifndef MAC
void DisposeHandle(Handle h)
{
	return;
}
#endif

/*------------------------ The segmentRec routines ------------------------
	These service p1Segment Records, which are held in an lList and are
	also indexed by the FieldList and the Unresolved Segment List.

		newSegmentRec(int Field, char desig, unsigned short *from)
		findSegment(int Field, char *desig)
		lookUpSR(int Field, char *desig)
		lowerList(int Field)
		raiseList(int Field) 
		addToFieldList(struct segmentRec *theSegment)

-------------------------------------------------------------------------*/

int newSegmentRec(int Field,char *desig,unsigned short *from)
{
	struct segmentRec *seg;
	
	seg=(struct segmentRec *) nextFromLList(SegList)+(SegList->current & BlockLo);
	memset(seg,0,sizeof(struct segmentRec));
	seg->segment = SegList->current;
	seg->Field = Field;
	desigCpy(seg->desig,desig);

	if(from) {		/* Insert into linked list. */ 
		seg->next= *from;
		*from=seg->segment;
	} else
		seg->next = 0;		/* Negative Field, -1 if a lit, < -1 if unresolved. */
	return(seg->segment);
}

/*----------------------------------------------------------------------
	findSegment returns a segment #, given an Field # and desig. It creates 
	a new segmentRec if necessary. It interacts with newSegmentRec to
	set up the FieldList for new segments.
------------------------------------------------------------------------*/

int findSegment(int Field,char *desig)
{
	unsigned short n,*last;
	int cmp;
	struct segmentRec *seg;
	
	if (Field<MinField) 
		MinField=lowerList(Field);
	else if (Field>MaxField)
		MaxField=raiseList(Field);
		
	/*----------------------------------------------------------------------------------	
		n points to the last index into linked list of segments in a field. It is passed
		to newSegmentRec(), which uses it to insert the new survey into the linked list.
	------------------------------------------------------------------------------------*/
	if (*(last=FieldList+Field-MinField)) {
		n=*last;
		do {
			seg = getSR(n);
			if(!(cmp=strncmp(seg->desig,desig,DesigLen)))
				return(seg->segment);
			else if(cmp>0) {	/* seg->desig>desig: Gone past, so seg doesn't exist. Insert it. */
				return(newSegmentRec(Field,desig,last));
			}
			last=&seg->next;
		} while (n=seg->next);
	} else if(FieldLList)
		newField(Field);
	/* not found, add to list, inserting at top */
	return(*last=newSegmentRec(Field,desig,0));
}

struct fieldRec *newField(int Field)
{
	struct fieldRec *se=0;
	if(Field>FieldLList->current)
		FieldLList->current=Field;
	se=getField(Field);
	se->plane=se->ThisField=Field;
	se->dimension=ThisContext->dimension;
	se->z=se->plane*ThisContext->dimension;
	return se;
}

/*----------------------------------------------------------------------
	lookUpSegment does much the same as findSegment. If the segment is 
	not found, it returns NULL instead of creating a new segmentRec. It 
	uses but does no maintenance of the FieldList.
------------------------------------------------------------------------*/

struct segmentRec *lookUpSR(int Field,char *desig)
{
	int i;
	struct segmentRec *seg;
	if (Field<MinField || Field>MaxField) {
		errorAlert(NoSegment );		
	}
	if (i=FieldList[Field-MinField]) {
		do {
			seg = getSR(i);
			if(desigCmp(seg->desig,desig))
				return(seg);
		} while (i = seg->next);
	}
	errorAlert(NoSegment);
}

/*------------------------------- findLit -------------------------------
	Analogous to findSegment, findLit always suceeds. It files the parseRec
	passed to it with a segment number and station number. If there is no 
	existing match, then findLit makes a new entry.
	1990.11.29: Integrate into the findName mechanism.
	====> Searching is linear and
	crude, as it is expected that there will never be many literal station 
	names. Should their number reach some threshold, say 100, then a sort 
	could be done at p1.5 and bsearch used from then on. 
		
	LookupLit is the p2 routine for searching the established list.
--------------------------------------------------------------------------*/

void findLit(struct parseRec *pR)
{
	int i;
	char lit[NameLength];
	struct segmentRec *seg;
	/* Retrieve the lit string. */
	for(i=0,s++;i<NameLength;i++,s++) {
		if(*s=='\'') {
			lit[i]=0;
			break;
		}
		*(lit+i)=tolower(*s);
	}
	if(!i) { 
		errorAlert(NoLit);
		return; 
	}
	if(*s!='\'') {
		errorAlert(NameTooLong);
		do { s++; } 
			while(*s && (*s!='\'') && (*s!='\n'));
	}
	if(*s=='\'')
		s++;
	if(!NameList) {
		NameHashArray=ecalloc(37,sizeof(long));
		NameList=lListAlloc(1,BlockLen,sizeof(struct nameRec),4,nNameList);
		NameList->current=36;
	}
	if(Literals)
		seg=getSR(Literals);
	else {
		seg=(struct segmentRec *) nextFromLList(SegList)+(SegList->current & BlockLo);
		Literals=seg->segment=SegList->current;
		seg->Field=-1;	/* a flag for THE lit segment. */
		*seg->desig=0;
		seg->age=0;
		seg->nextSubSeg=0;
		seg->nStations=seg->nLines=seg->first=seg->other=seg->flags=seg->last=0;
		seg->nTies=seg->nLinks=seg->nLocns=0;
		seg->links=0;
	}
	seg->nStations++;
	pR->this.segment=Literals;	/* a global */
	pR->this.n=findName(lit);
	pR->desig[0]=0;
	pR->Field=-1;
	return;
}

void lookUpLit(struct parseRec *pR)
{
	int i;
	char lit[NameLength];
	/* Retrieve the lit string. */
	for(i=0,s++;i<NameLength;i++,s++) {
		if(*s=='\'') {
			lit[i]=0;
			break;
		}
		*(lit+i)=tolower(*s);
	}
	if(!i) { 
		errorAlert(NoLit);
		return; 
	}
	if(*s!='\'') {
		errorAlert(NameTooLong);
		do { s++; } 
			while(*s && (*s!='\'') && (*s!='\n'));
	}
	if(*s=='\'')
		s++;
	pR->this.segment=Literals;	/* a global */
	pR->this.n=findName(lit);
	pR->desig[0]=0;
	pR->Field=-1;
	return;
}

/*---------------------------------------------------------------------
	The FieldList is an array of indexType (shorts) that are segment 
	numbers and that match Field numbers with the segments in those
	fields. sR->next in each segment record completes a linked list of
	segments for a given Field.
-----------------------------------------------------------------------*/

int addToFieldList(struct segmentRec *theSegment)
{
	int Field=theSegment->Field;
	if (Field<MinField) 
		MinField=lowerList(Field);
	else if (Field>MaxField)
		MaxField=raiseList(Field);
	theSegment->next = FieldList[Field-MinField]; 
	FieldList[Field-MinField] = theSegment->segment;
	return 1;
}

int lowerList(int Field)
{
	int m;
	unsigned short *new;
	m=(Field-20>=0)? Field-20 : 0;
	new=ecalloc(MaxField-m+1,sizeof(short));
	memcpy(new+MinField-m,FieldList,(MaxField-MinField+1)*sizeof(short));
	free(FieldList);
	FieldList=new;
	return(m);
}

int raiseList(int Field) /* Specific to FieldList */
{
	int i;
	unsigned short *new;
	new=(unsigned short *)erealloc(FieldList,(Field-MinField+21)*sizeof(unsigned short));
	for (i=MaxField-MinField+1;i<=Field-MinField+20;i++)
		new[i]=0;
	FieldList=new;
	return(Field+20);
}

void trimMemory(void)
{
	int i,j,oldest; 
	unsigned long oldValue;
	struct lList *li;
	struct listElement *le;
	fprintf(OutFile,"Trimming...\n");
	
	for(i=0;i<nMasters;i++) {
		if(li=MastersArray[i].p) {
			if(li->code&MayPurge)
				purgeLL(li);
			else if(li->code&MayFree)
				freeLL(li);
			else if(li->nbuffers>(li->code&3)) {	/* Trim number of buffers to preset value (1,2, or 3). */
#ifdef DEBUG
				fprintf(OutFile,"%s buffers: %d limit: %d ",LListNames[i],li->nbuffers,li->code&3);
#endif
				li->code&=3;
				li->code|=(li->nbuffers>3)? 3:li->nbuffers;
				oldest=0;
				while(li->nbuffers>(li->code&3) && oldest>=0) {
					for(j=0,oldValue=li->thisHit,oldest=-1;j<li->nLE;j++) {
						le=li->list+j;
						if(le->ptr && (le->hit<oldValue)) {
							oldest=j;
							oldValue=le->hit;
						}
					} 
					if(oldest>=0) {
						le=li->list+oldest;
						le->position=(le->position)? tempWrite(le->ptr,le->position,li->size)
							:tempWrite(le->ptr,-1L,li->size);
						if(le->handle)
							DisposeHandle(le->handle);
						else
							free(le->ptr);
						le->ptr=NULL;
						le->handle=NULL;
						li->nbuffers--;
					}
				}
#ifdef DEBUG
				fprintf(OutFile,"result: %d\n",li->nbuffers);
#endif
			}
		}
	}
	return;
}

void *emalloc(size_t size)
{
	void *ptr;

	if(!(ptr=malloc(size))) {
		trimMemory();
		if(!(ptr=malloc(size)))
			errorAlert(OutOfMemory);
	}
	return ptr;
} 

void *ecalloc(size_t nmemb, size_t size)
{
	void *ptr;
	if(!(ptr=calloc(nmemb,size))) {
		trimMemory();
		if(!(ptr=calloc(nmemb,size))) 
			errorAlert(OutOfMemory);
	}
	return ptr;
}

void *erealloc(void *ptr, size_t size)
{
	void *pptr;

	if(!(pptr=realloc(ptr,size))) {
		trimMemory();
		if(!(pptr=realloc(ptr,size)))
			errorAlert(OutOfMemory);
	}
	return pptr;
}

void *hemalloc(size_t size,Handle *h)
{
	void *ptr;
#ifdef MAC
	if(!(*h=NewHandle(size))) {
		trimMemory();
		if(!(*h=NewHandle(size)))
			errorAlert(OutOfMemory);
	}
	HLock(*h);
	ptr=**h;
#else
	if(!(ptr=malloc(size))) {
		trimMemory();
		if(!(ptr=malloc(size)))
			errorAlert(OutOfMemory);
	}
#endif
	return ptr;
} 

void *hecalloc(size_t nmemb, size_t size,Handle *h)
{
	void *ptr;
#ifdef MAC
	if(!(*h=NewHandle(nmemb*size))) {
		trimMemory();
		if(!(*h=NewHandle(nmemb*size)))
			errorAlert(OutOfMemory);
	}
	HLock(*h);
	ptr=**h;
	memset(ptr,0,nmemb*size);
#else
	if(!(ptr=calloc(nmemb,size))) {
		trimMemory();
		if(!(ptr=calloc(nmemb,size))) 
			errorAlert(OutOfMemory);
	}
#endif
	return ptr;
}

void *hcalloc(size_t nmemb, size_t size,Handle *h)
{
#ifdef MAC
	void *ptr;
	if(!(*h=NewHandle(nmemb*size))) 
		return 0;
	HLock(*h);
	ptr=**h;
	memset(ptr,0,nmemb*size);
	return ptr;
#else
	return (calloc(nmemb,size));
#endif
}

void *hmalloc(size_t size,Handle *h)
{
#ifdef MAC
	void *ptr;
	if(!(*h=NewHandle(size)))
		return 0;
	HLock(*h);
	ptr=**h;
	return ptr;
#else
	return (malloc(size)) ;
#endif
} 

/*---------------------- Handling proper nouns -----------------------
	For now (1990.11.29) all proper names are to be put into one
	list. This includes place names (i.e. #location), people
	names passed out of personnel(), plus literal names. Literal
	names are anything enclosed in single quotes and include
	literal station names and literal (non-numerical) identifiers
	of such things as compass and inclinometer.

	Names are in a hashed linked list. Hashing is by [a-z,0-9]+other.
	The name to be found is the strings s, which hides the global s.
	
----------------------------------------------------------------------*/
/*---------------------------------------------------------------------
dBRecs

dBRecs are a multipurpose device for handling database-like functions.
Its metaphor is that of a flat memory heap within which linked lists,
multidimensional arrays, contiguous lists, etc. are intermeshed. The
atomic (i.e. memory cell) size, though, is sizeof(struct dBRec),
currently 12 bytes. I have attempted to keep the usage of dBRec fields
somehow logical and related to how they are named. Most db uses work
hand-in-hand with the NameList, where stings are stored in a hashed
list. Retrievals in the NameList return a unique index that can then
be used as a dBRec field (what, where, etc.).

	Attributes recorded by #start....#finish directives could create a
multidimensional structure in the dBRec space. #begin xxx directives
create an attribute class named xxx. Classes are stored as contiguous
records in the lList blocks (which are power-of-2-sized; dBRecs are
currently 64 large). The first block of dBRecs are reserved for class
records. If the block size is 64, dB[63], the last dBRec of the first
block, is reserved to be a pointer (index type) to the next block of
classes, should that be necessary. Subsequent blocks of classes may
not occupy all of a 64-record-long dBRec block, as their may be useful
space at the tail of a partially-occupped block, but that block's last
member is always reserved for a linking class record.

	However, as first implemented (v. 0.98db) attributes are a simple
linked list, in numerical order by the .what field, for each header
rec of segments. Much simpler. Attributes are bounded by segments.

---------------------------------------------------------------------*/

int findName(char *s)
{
	int m;
	long *nextPtr;
	struct nameRec *place;

	/*	The string has been conditioned. The essential operation is to make
		the string lower case (an asthetic choice). Canonization must occur
		before so long as there is a unified list for all proper nouns. */
	
	/*	The list is alphabetical within each linked list. The base hashing
		is like a telephone book: 
			(1) all non-alphanumeric
			(2) numeric
			(3) alpha			*/
	
	if(!NameList || !*s)
		return 0;	/* No error. This is a way we deal with !-h. */

	if(isalpha(*s))
		m = 11 + tolower(*s) - 'a';
	else if(isdigit(*s))
		m = 1 + *s - '0';
	else 
		m = 0;
	nextPtr=NameHashArray+m;
	while(*nextPtr) {
		place = getPlace(*nextPtr,NameList);
		if((m=strcmp(place->name,s))==0)  /* match */
			return *nextPtr;
		if(m>0)  	/* no match possible, insert here. */
			break;		
		/* m > 0 so will loop again. */
		nextPtr=&place->next;
	} 
	/* No match; make a new record. */
	
	place = getNextPlace(NameList);

	/*	The only danger in this routine is if the last place gets swapped 
		out of memory by place=getNextPlace(...). This will never happen
		so long as there are at least two buffers for the NameList. */

	if((NameList->current>BlockLen) && (NameList->nbuffers<2)) {
		errorAlert(OutOfNameListMem);
		return 0;
	}
	strcpy(place->name,s);
	place->next = *nextPtr;
	return (*nextPtr=NameList->current);	
}

struct locnRec *setUpDataSpace(int thisStation,indexType segment)
{
	struct locnRec *theData;
	struct subSegRec *ss;
	struct segmentRec *sr;
	
	sr = getSR(segment);
#if 1
	if(segment!=ThisContext->segment)
		fprintf(OutFile,"Line %d: %d ­ %d\n",LineCnt,segment,ThisContext->segment);

#endif
	for(ss=(struct subSegRec *)sr,theData=0;!theData;) {
		if(ss->first<=thisStation && thisStation<=ss->last && ss->data) 
			theData=getData(ss->data+thisStation-ss->first);
		else if (ss->nextSubSeg)
			ss=getSS(ss->nextSubSeg);
		else {
			theData=&TempLocn;
#if 0
			fprintf(OutFile,"Line %d ### Filling (3) TempLocn, Length: %d\n",LineCnt,TempLocn.length);
#endif
			if(theData->code) {
				theData->code=0;
				errorAlert(DoubleData);
				/* TempLocn has been used but not cleared. This
					means that two shots map onto the same
					location. */
			}
		}
	}
#if 0
	if(UsingStrings) {
		for(i=0;i<sr->nStrings;i++) {
			str=getStr(sr->strings+i);
			if(str->code&UnMapped) continue;
			if(str->first<=thisStation && thisStation<=str->last)
				break;
		} 
		if(i==sr->nStrings) {
			errorAlert(NoString);		/*=== this will fail.==== A10=[..] fatal */
		}
	}
#endif
	return(theData);
}

struct locnRec *_getMainData(struct segmentRec *seg,int sta,struct subSegRec **subS)
{
	struct subSegRec *ss;
	for(ss = (struct subSegRec *)seg;ss;ss=(ss->nextSubSeg)?getSS(ss->nextSubSeg):0) {
		if(ss->first<=sta && sta<=ss->last) {
			if(subS) 
				*subS=ss;
			return getData(ss->data+sta-ss->first);
		}
	}
	if(subS) 
		*subS=0;
	return 0;
}

struct dimData *findDims(struct locnRec *locn)
{
	int n;
	struct locnRec *buf;
	
	if(ThisContext->pass == 1) {
		/* In an explicit tie the dimensions are for the from-station. */
		DimList->current=P1LocnList->current;
		return getDims(P1LocnList->current-1);
	}
	else if(locn == &TempLocn)
		return &TempDims;
	else {
	/* 	There is one list for dimensions. The main data dims are just placed on top of
		the LocnList->current number of records from P1. */
		buf=(struct locnRec *)DataList->list[DataList->currentList].ptr;
		if(buf && locn>=buf && locn<=(buf+DataList->nItems-1)) {
			n=LocnList->current
				+DataList->currentList*DataList->nItems
				+(locn-buf);
			if(LineOrder!=ToVector) n--;
			if(n>DimList->current)
				DimList->current=n;
			return getDims(n);
		}
		buf=(struct locnRec *)LocnList->list[LocnList->currentList].ptr;
		if(buf && locn>=buf && locn<=(buf+LocnList->nItems-1)) {
			n=LocnList->currentList*LocnList->nItems+(locn-buf);
			return getDims(n);
		}
	}
	getDims(0);		/* Force a fatal error. */
	return 0;
}

void clearMem(void)
{
	int i;
	
	for(i=0;i<nMasters;i++) {
		if(MastersArray[i].p) 
			purgeLL((struct lList *)(MastersArray+i)->p);
	}
	
	/*	Structures needed for just this run. */
	if(FixArray) { free(FixArray); FixArray=0; }
	if(FieldList) { free(FieldList); FieldList=0; }
	if(FieldsToIgnore) { free(FieldsToIgnore); FieldsToIgnore=0; }
	if(LogList) { free(LogList); LogList=0; }
	if(IFileList) { free(IFileList); IFileList=0; }
	if(NameHashArray) { free(NameHashArray); NameHashArray=0; }
}

struct segmentRec *nextSegment(struct segmentRec *seg,struct fieldRec **fr)
{
	int n;
	
	if(seg && seg->next)			/*  Within linked list of segmentRecs */
		return getSR(seg->next);	/* 	within a section/FSB. */
	/*	Need to go to next field. If there are fieldRecs, we will use them. */
	if(*fr) {
		*fr=(n=(*fr)->nextField)?getField(n):0;
		return (n=FieldList[(*fr)->ThisField-MinField])? getSR(n):0;
	}
	/* No fr, so we are stepping by FieldList. Start looking for a
		non-null entry at the next FieldList */
	for(n=(seg)? seg->Field-MinField+1:0;n<=MaxField-MinField;n++) 
		if(FieldList[n])
			return getSR(FieldList[n]);
	return 0;
}

#if defined(MDLView) || defined(VECTORS)

Handle makeHandle(void *p,size_t size)
{
	Handle h;
	fprintf(OutFile,"makeHandle %d, size: %d\n",(int)p,size);
	if(PtrToHand(p,&h,size))
		errorAlert(OutOfMemory);
	return h;
}

Handle makeRecon(struct reconstruction *recon)
{
	int i,j,numLE;
	Handle r;
	struct lList *theList;
	struct listElement *le;
	
	fprintf(OutFile,"makeRecon\n");
	for(i=0;i<nMasters;i++) {
		if((recon->masters+i)->p) {
			theList=(recon->masters+i)->p;
			theList->nLE=numLE=(theList->current/theList->nItems)+1;
			for(j=0;j<numLE;j++) {
				/* Unlock all of the buffer handles. */
				le=theList->list+j;
				if(le->ptr) {
					if(!le->handle) {
						/* No handle, as with the LinkList. */ 
						le->handle=makeHandle(le->ptr,theList->size);
						free(le->ptr); 
					}
					HUnlock(le->handle);
					le->ptr=0;
				}
			}
			/*	Now the list's list becomes an unlocked Handle. */
			theList->hList=makeHandle(theList->list,numLE*sizeof(struct listElement));
			free(theList->list);
			theList->list=0;
			/*	As does the list itself. */
			(recon->masters+i)->h=makeHandle(recon->masters[i].p,sizeof(struct lList));
			free(recon->masters[i].p);
			(recon->masters+i)->p=0;
		}
	}

	recon->literals=Literals;
	if(NameHashArray) {
		recon->names=(int **)makeHandle(NameHashArray,37*sizeof(int));
	} else
		recon->names=0;
	recon->tf=Tf;
	Tf=0;				/* Prevents it from being closed at end of MDL. */
	if(!(r=NewHandle(sizeof(struct reconstruction))))
		errorAlert(OutOfMemory);
	HLock(r);
	**(struct reconstruction **)r=*recon;
	HUnlock(r);
	fprintf(OutFile,"Handle: %d\n",r);
	return r;
}
#endif

#ifdef MDLView
int disposeLList(Handle hLi)
{
	/* 	
		Disposes or frees all memory allocated within a lList plus
		the lList itself.
	*/
	struct lList *li;
	struct listElement *le;
	int i;
	HLock(hLi);
	li=(struct lList *) *hLi;
	if(li->hList) {
		HLock(li->hList);
		li->list=(struct listElement *) *li->hList;
	} 
	for(le=li->list,i=0;i<li->nLE;le++,i++) {
		if(le->handle)
			DisposeHandle(le->handle);
		else if(le->ptr)
			free(le->ptr);
	}
	if(li->hList)
		DisposeHandle(li->hList);
	else
		free(li->list);
	DisposeHandle(hLi);
	return 0;
}


int fetchRecon(Handle recon)
{
	struct lList *li;
	struct listElement *le;
	int i,j;
	
	/* fprintf(OutFile,"fetchRecon with: %d\n",recon); */
	/* CJW removed caused called from DrawSegment */
	HLock(recon);
	CurrentRecon=recon;
	Literals=(**(struct reconstruction **)recon).literals;
	Tf=(**(struct reconstruction **)recon).tf;
	MastersArray=(**(struct reconstruction **)recon).masters;
	for(i=0;i<nMasters;i++) {
		if((MastersArray+i)->h) {
/*			fprintf(OutFile,"List %d: ",i); // CJW removed caused called from DrawSegment	*/
			HLock((MastersArray+i)->h);
			li = (MastersArray+i)->p = *((MastersArray+i)->h);
			if(li->hList) {
				HLock(li->hList);
				li->list=(struct listElement *) *li->hList;
			} 
		/*	fprintf(OutFile,"Current: %d list: %d nLE: %d\n",li->current,li->list,li->nLE);	*/
		/* CJW removed caused called from DrawSegment	*/
			for(le=li->list,j=0;j<li->nLE;le++,j++) {
				if(le->handle) {
					HLock(le->handle);
					le->ptr = *le->handle;
				}
			}
		}
	}
	DataList=(MastersArray+nDataList)->p;			/* 0 */
	FieldLList=(MastersArray+nFieldLList)->p;		/* 1 */
	LinkList=(MastersArray+nLinkList)->p;			/* 2 */
	LocnList=(MastersArray+nLocnList)->p;			/* 3 */
	StringList=(MastersArray+nStringList)->p;		/* 4 */
	SubSegList=(MastersArray+nSubSegList)->p;		/* 5 */
	SegList=(MastersArray+nSegList)->p;				/* 6 */
	UnResSegList=(MastersArray+nUnResSegList)->p;	/* 7 */
	ColList=(MastersArray+nColList)->p;				/* 8 */
	DimList=(MastersArray+nDimList)->p;				/* 9 */
	LineList=(MastersArray+nLineList)->p;			/* 10 */
	LongTraList=(MastersArray+nLongTraList)->p;		/* 11 */
	NameList=(MastersArray+nNameList)->p;			/* 12 */
	DbList=(MastersArray+nDbList)->p;				/* 13 */
	RowList=(MastersArray+nRowList)->p;				/* 14 */
	HeaderList=(MastersArray+nHeaderList)->p;		/* 16 */

	return 0;
}

int MDLDispose(Handle hRecon)
{
	struct reconstruction *recon;
	int i;
	
	HLock(hRecon);
	recon = (struct reconstruction *) *hRecon;
	for(i=0;i<nMasters;i++) {
		if((recon->masters+i)->h) {
			disposeLList((recon->masters+i)->h);
		} else if((recon->masters+i)->p) {
			freeLL((recon->masters+i)->p);
		}
	}
	if(recon->names)
		DisposeHandle((Handle)recon->names);
	if(recon->tf)
		fclose(recon->tf);
	DisposeHandle(hRecon);
	return 0;
}

int MDLLastError(int how)
{
	int err;
	if(ThisContext) {
		err=ThisContext->errNum;
		if(how) free(ThisContext);
		return err;
	}
	/*	If there is no ThisContext struct, then either this routine has been 
		mistakingly called after a normal return or the severest kind of 
		out-of-memory environment exists, i.e. the first malloc() failed!
		Instead of returning -19 (OutOfMemory) I'll return the non-MDL
		error code of -1. */
	return -1;
}

struct segmentRec *MDLGetSegment(Handle recon, int segmentIndex, struct segmentRec *seg)
/*	Copies the indicated segment record into the space pointed to by seg.
	This is a pointer to space that has been allocated by the caller.
	Returns seg. */
{
	struct segmentRec *seg0;
	if(CurrentRecon!=recon)
		fetchRecon(recon);
	if(seg0=getSR(segmentIndex))
		*seg=*seg0;
	return seg0;
}

int MDLDrawSegment(Handle recon,int segmentIndex,struct transform *trans)
{
	
	int sta,j;
	Point pt;
	struct segmentRec *seg;
	struct subSegRec *ss;
	struct locnRec *theData,*theData2;
	LinkRecPtr theLink;
	
	if(CurrentRecon!=recon)
		fetchRecon(recon);

	seg=getSR(segmentIndex);
/*	fprintf(OutFile,"Drawing segment %d (%s) with %d points and %d links\n", */
/*		segmentIndex,seg->desig,seg->nStations,seg->nTies+seg->nLinks); */

	for(ss=(struct subSegRec *)seg
		;ss
		;ss=(ss->nextSubSeg)?ss=getSS(ss->nextSubSeg):0) {
		if(ss->data) {
			for(theData=getData(ss->data),sta=ss->first;
					sta<=ss->last;theData++,sta++) {
				/* penwidth */
				if((theData->code&(_Point|_Rel))==Move_to) {
					/* transform */
					pt=MDLTransformSPoint(trans,&theData->data.cart);
					MoveTo(pt.h,pt.v);
				} else if((theData->code&(_Point|_Rel))==Line_to) {
					/* transform */
					pt=MDLTransformSPoint(trans,&theData->data.cart);
					LineTo(pt.h,pt.v);
				}
			}
		}
		for(j=seg->links;j<seg->links+seg->nLocns+seg->nLinks+seg->nTies;j++) {
			theLink = getLink(j);
			if(theLink->to.segment>0 && theLink->data && !(theLink->from.segment&Link)) {
				theData2 = getLocn(theLink->data);
				if(!(theData2->code&Virtual)) {
					if(theData2->code&CloseShape) {
						/* transform */
						pt=MDLTransformSPoint(trans,&theData2->data.cart);
						LineTo(pt.h,pt.v);
					} else if((theData2->code&(_Point|_Rel))==Line_to) {
						theData = theData2-1;
						/* transform */
						pt=MDLTransformSPoint(trans,&theData->data.cart);
						MoveTo(pt.h,pt.v);
						/* transform */
						pt=MDLTransformSPoint(trans,&theData2->data.cart);
						LineTo(pt.h,pt.v);
					}
				}
			}
		}
	}
	return 0;
}

int MDLSegmentsInRecon(Handle recon)
{
	/*	Returns the number of segments in the reconstruction recon. */
	return (**(struct lList**)((**(struct reconstruction**)recon).masters[nSegList].h)).current;
}

Point MDLTransformPoint(struct transform *trans,struct c_locn *data)
{
	Point pt;
	c_float X,Y;
	X=trans->r[0]*data->x+trans->r[1]*data->y+trans->r[2]*data->z+trans->t[0];
	Y=trans->r[3]*data->x+trans->r[4]*data->y+trans->r[5]*data->z+trans->t[1];
	pt.h=X;
	pt.v=Y;
	return pt;
}

Point MDLTransformSPoint(struct transform *trans,struct cartData *data)
/*	Performs a limited transform (only X' and Y' are calculated) on *data, 
	according to the trans matrix and returns the 2D result as a Point. */
{
	Point pt;
	c_float X,Y;
	X=trans->r[0]*data->x+trans->r[1]*data->y+trans->r[2]*data->z+trans->t[0];
	Y=trans->r[3]*data->x+trans->r[4]*data->y+trans->r[5]*data->z+trans->t[1];
	pt.h=X;
	pt.v=Y;
	return pt;
}


int MDLRotate(struct transform *trans,c_float x,c_float y,c_float z)
/*	The rotation fields of trans are replaced by x,y, and z and the matrix
{
	trans->x=x;
	trans->y=y;
	trans->z=z;
	return MDLFillMatrix(trans);
}

int MDLRotateRelative(struct transform *trans,c_float x,c_float y,c_float z)
/*	The trans matrix is rotated by x,y, and z. */
{
	trans->x+=x;
	trans->y+=y;
	trans->z+=z;
	return MDLFillMatrix(trans);
}
	
int MDLTranslate(struct transform *trans,c_float dx,c_float dy,c_float dz)
/*	The translate fields of trans are replaced by x,y, and z and the matrix
	recalculated. */
{
	trans->dx=dx;
	trans->dy=dy;
	trans->dz=dz;
	return MDLFillMatrix(trans);
}

int MDLTranslateRelative(struct transform *trans,c_float dx,c_float dy,c_float dz)
/*	The trans matrix is translated by dx,dy, and dz. */
{
	trans->dx+=dx;
	trans->dy+=dy;
	trans->dz+=dz;
	return MDLFillMatrix(trans);
}

int MDLScale(struct transform *trans,c_float mag)
/*	The mag field of trans is replaced by mag and the matrix recalulated. */
{
	trans->mag=mag;
	return MDLFillMatrix(trans);
}

int MDLScaleRelative(struct transform *trans,c_float mag)
/*	The trans matrix is maginified by mag. */
{
	trans->mag*=mag;
	return MDLFillMatrix(trans);
}

int MDLFillMatrix(struct transform *trans)
/*-----------------------------------------------------------------
	Calculated the transform matrix (r[] and t[] terms) based on the values
	contained in the x,y,z,ax,ay,az, and mag fields. Returns 0 if no error.

	Transform expansion:

	M is Magnification
	Dx,Dy,Dz are translation terms
	x,y,z are rotation angles
	X,Y,Z are coordinate values

C'=((M*cos(y)*cos(z)*X
	+(M*cos(z)*sin(y)*sin(x)-M*cos(x)*sin(z))*Y
	+(M*cos(x)*cos(z)*sin(y)+M*sin(x)*sin(z))*Z
	+Dx*M*cos(y)*cos(z)
	+Dy*M*cos(z)*sin(y)*sin(x)-Dy*M*cos(x)*sin(z)
	+Dz*M*cos(x)*cos(z)*sin(y)+Dz*M*sin(x)*sin(z),

	M*cos(y)*sin(z)*X
	+(M*cos(x)*cos(z)+M*sin(x)*sin(y)*sin(z))*Y
	+(-M*cos(z)*sin(x)+M*cos(x)*sin(y)*sin(z))*Z
	+Dx*M*cos(y)*sin(z)
	+Dy*M*cos(x)*cos(z)+Dy*M*sin(x)*sin(y)*sin(z)
	-Dz*M*cos(z)*sin(x)+Dz*M*cos(x)*sin(y)*sin(z),

	-M*sin(y)*X
	+M*cos(y)*sin(x)*Y
	+M*cos(x)*cos(y)*Z
	+Dz*M*cos(x)*cos(y)
	+Dy*M*cos(y)*sin(x)
	-Dx*M*sin(y),
	
	1))
	-------------------------------------------------------------*/
	
{
	c_float cx,cy,cz,sx,sy,sz;
	
	cx=cos(trans->x);
	cy=cos(trans->y);
	cz=cos(trans->z);
	sx=sin(trans->x);
	sy=sin(trans->y);
	sz=sin(trans->z);
	/*	First column is r[0-2] and t[0] */
	trans->r[0]=trans->t[0]=cy*cz*trans->mag;
	
	trans->t[0]*=trans->dx;		/* added by charlie 5/11/92 */
	
	trans->r[1]=trans->r[2]=cz*sy;
	trans->r[1]*=sx;
	trans->r[1]-=cx*sz;
	trans->r[1]*=-trans->mag;	/* inverted by charlie 5/24/92 for top-down screen */
	
	trans->r[2]*=cx;
	trans->r[2]+=sx*sz;
	trans->r[2]*=trans->mag;
	
	trans->t[0]+=trans->dy*trans->r[1];
	trans->t[0]+=trans->dz*trans->r[2];
	
	/*	Second column is r[3-5] and t[1] */
	
	trans->r[3]=trans->t[1]=cy*sz*trans->mag;
	
	trans->t[1]*=trans->dx;	/* added by charlie 5/11/92 */
	
	trans->r[4]=trans->r[5]=sy*sz;
	trans->r[4]*=sx;
	trans->r[4]+=cx*cz;
	trans->r[4]*=-trans->mag;	/* inverted by charlie 5/24/92 for top-down screen */
	
	trans->r[5]*=cx;
	trans->r[5]-=sx*cz;
	trans->r[5]*=trans->mag;
	
	trans->t[1]+=trans->dy*trans->r[4];
	trans->t[1]+=trans->dz*trans->r[5];
	
	return 0;
}

#endif

void saveSnapShot(int a, char *b[])
{
	long i,n,pos;
	struct lList *li;
	Str31 version;
	
	/*	Save all of the lList data to disk, plus the lists themselves. The position
		of each lList is saved in the MastersArray. */
	for(i=0;i<nMasters;i++) {
		if(MastersArray[i].p) {
			freeLL(li=(MastersArray+i)->p);
			li->other=tempWrite(li->list,-1L,li->nLE*sizeof(struct listElement));
			MastersArray[i].other=tempWrite(li,-1L,sizeof(struct lList));
		}
	}
	/*	Now write the header data that unlocks the positions of all of the data. */
	/*	(a) First are the command line tokens (argv,argc) that have been passed as a, b. */
	pos=tempWrite(&a,-1L,sizeof(long));
	for(i=0;i<a;i++) {
		n=strlen(b[i]);
		tempWrite(&n,-1L,sizeof(long));
		tempWrite(b[i],-1L,n*sizeof(char));
	}
	/*	(b) Next comes the version string, which is fixed length. */
	strcpy(version,VersionString);
	tempWrite(version,-1L,sizeof(Str31));
		/* 	Note that the seg->center (12 bytes), replaced by seg->stations (8 bytes)
			with version 1.8 */
	
	/*	(c) Now we save the FieldList and its range. */
	tempWrite(&MinField,-1L,sizeof(long));
	tempWrite(&MaxField,-1L,sizeof(long));
	tempWrite(FieldList,-1L,(MaxField-MinField+1)*sizeof(short));
	
	/*	(d) If there is a NameHashArray, we save a flag and then it (fixed length) */
	tempWrite(&NameHashArray,-1L,sizeof(long *));
	if(NameHashArray)
		tempWrite(NameHashArray,-1L,37*sizeof(long));
	
	/*	(e) Next comes the single Literals variable. */
	tempWrite(&Literals,-1L,sizeof(long));
	
	/*	(f) Now comes the key to unlocking all of the lList data: the MastersArray. */
	n=nMasters;
	tempWrite(&n,-1L,sizeof(long));
	tempWrite(MastersArray,-1L,nMasters*sizeof(struct masterRec));
	
	/*	Finally, the position where the header began is stored as the first 4 bytes of the file. */
	tempWrite(&pos,0L,sizeof(long));
}
