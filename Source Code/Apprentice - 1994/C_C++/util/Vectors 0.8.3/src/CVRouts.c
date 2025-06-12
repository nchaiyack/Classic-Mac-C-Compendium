#include "CVectors.h"
#include "CVGlobs.h"

/*--------------------------- Declarations --------------------------------*/


QDErr NewOffscreenBitMap(GrafPtr *,Rect *,Handle *);

GWorldPtr prepareGWorld(GWorldPtr,short,short,int,Handle *);

void LockLists(FileStuff *);
void UnlockLists(FileStuff *);
void DisposeLists(FileStuff *);
void cvTempRead(short, void *,long,long);
static void *hcalloc(size_t, size_t,Handle *);
static void *hmalloc(size_t,Handle *);
static void *hecalloc(size_t, size_t,Handle *);
static void *hemalloc(size_t,Handle *);
static void trimMemory(void);
void *getCVLListBuf(int,FileStuff *,int);

void SelectObject(Point,WindowPtr);
ListHandle FillList(FileStuff *,WindowPtr);
int ccompare(char *,char *);
int dcompare(char *,char *);

int FillBounds(FileStuff *);
int DrawSurvey(Survey *,FileStuff *,int);
void HiliteSurvey(Survey *,FileStuff *,int);
void OpenTextWindow(WindowPtr, int);
int reportSurvey(Survey *,char *,FileStuff *);
int reportHeader(Survey *,char *,FileStuff *);
int cvPrintTies(Survey *,char *,int,int,int *,FileStuff *);
extern char *printSta(int,char *);
char *cvPrintDesigSta(int,Survey *,char *,FileStuff *);
extern char *cvPrintFullDesig(int,Survey *,char *,FileStuff *);
char *cvPrintFullSta(int,Survey *,char *,FileStuff *);
void doSelectDialog(WindowPtr);

/*------------------------------- Some Globals -----------------------*/
int ListDisplayRule=1;
int DefaultCenterSelection=TRUE;
int DefaultScaleToFit=FALSE;

QDErr NewOffscreenBitMap(GrafPtr *newOffscreen,Rect *inBounds,Handle *bitHandle)
{

	GrafPtr savePort;
	GrafPtr newPort;
	OSErr err;
	
	GetPort(&savePort);
	
	newPort = (GrafPtr) NewPtr(sizeof(GrafPort));
	if((err=MemError()) != noErr)
		return err;
	
	OpenPort(newPort);
	newPort->portRect = *inBounds;
	newPort->portBits.bounds = *inBounds;
	RectRgn(newPort->clipRgn,inBounds);
	RectRgn(newPort->visRgn, inBounds);
	
	newPort->portBits.rowBytes = ((inBounds->right-inBounds->left+31)>>5)<<2;
	
	*bitHandle = NewHandle(newPort->portBits.rowBytes*(long)(inBounds->bottom-inBounds->top));
	if((err=MemError())!=noErr) {
		SetPort(savePort);
		ClosePort(newPort);
		DisposePtr((Ptr)newPort);
		return err;
	}
	HLock(*bitHandle);
	newPort->portBits.baseAddr = **bitHandle;
	EraseRect(inBounds);
	HUnlock(*bitHandle);
	newPort->portBits.baseAddr = 0;
	*newOffscreen = newPort;
	SetPort(savePort);
	return noErr;
}

GWorldPtr prepareGWorld(GWorldPtr where,short width,short length,int how,Handle *bitHandle)
{
	GDHandle oldGD;
	GWorldPtr oldGW;
	PixMapHandle pixBase;
	QDErr err=0;
	Rect fullRect;

	unsigned short rowBytes;
	SetRect(&fullRect,0,0,width,length);

	if(gHasColorQD) {
		GetGWorld(&oldGW,&oldGD);
	
		if(!how)
			err=NewGWorld(&where,1,&fullRect,MyCMHandle,NIL,NIL);
		else
			err=UpdateGWorld(&where,1,&fullRect,MyCMHandle,NIL,NIL);
		*bitHandle=0;
	} else 
		err=NewOffscreenBitMap((GrafPtr *)&where,&fullRect,bitHandle);

	if (err!=noErr) {
		doMessage(20);
		return 0;
	}
	if(gHasColorQD) {
		pixBase=GetGWorldPixMap(where);
		rowBytes=(**pixBase).rowBytes;
	
		LockPixels(pixBase);
		SetGWorld(where,nil); 
		EraseRect(&(where->portRect)); 
		SetGWorld(oldGW,oldGD);	
		UnlockPixels(pixBase);
		
		SetGWorld(oldGW,oldGD);	
	}
	return where;	/* No error */
}

#ifdef MC68000
#pragma segment CMLinterface
#endif

void cvTempRead(short refNum, void *buf,long pos,long size)
{
	OSErr err;
	
	if(pos>=0) {		/* -1 is a flag: read at present position. */
		err=SetFPos(refNum,fsFromStart,pos);
		if(err!=noErr) {
			errorAlertDialog(DiskErr);
			return;
		}
	}
	err=FSRead(refNum,&size,buf);
	if(err!=noErr) {
		errorAlertDialog(DiskErr);
	}
}


static void *hcalloc(size_t nmemb, size_t size,Handle *h)
{
	void *ptr;
	
	if(!(*h=NewHandle(nmemb*size))) 
		return 0;
	HLock(*h);
	ptr=**h;
	memset(ptr,0,nmemb*size);
	return ptr;
}

static void *hmalloc(size_t size,Handle *h)
{
	void *ptr;
	if(!(*h=NewHandle(size)))
		return 0;
	HLock(*h);
	ptr=**h;
	return ptr;
} 

static void *hemalloc(size_t size,Handle *h)
{
	void *ptr;

	if(!(*h=NewHandle(size))) {
		trimMemory();
		if(!(*h=NewHandle(size)))
			errorAlertDialog(OutOfMemory);
	}
	HLock(*h);
	ptr=**h;
	return ptr;
} 

static void *hecalloc(size_t nmemb, size_t size,Handle *h)
{
	void *ptr;

	if(!(*h=NewHandle(nmemb*size))) {
		trimMemory();
		if(!(*h=NewHandle(nmemb*size)))
			errorAlertDialog(OutOfMemory);
	}
	HLock(*h);
	ptr=**h;
	memset(ptr,0,nmemb*size);
	return ptr;
}

static void trimMemory(void)
{
	return;
}

/*--------------------------------------------------------------------
	getCVLListBuf() returns a pointer to the requested long list
	buffer. It will create a new buffer if the requested n is less
	than theList->current (i.e., reserved, unallocated space).
----------------------------------------------------------------------*/

void *getCVLListBuf(int n,FileStuff *fStuff,int listIndex)
{
	ListElement *le,*li;
	LongList *theList;
	unsigned long oldValue;
	int i,oldest;
	
	theList=fStuff->masters[listIndex].p;

	if(n>theList->current || n<1)
		errorAlertDialog(PagingError);	/* Fatal */

	if((theList->currentList=n/theList->nItems)>=theList->nLE) 
		errorAlertDialog(PagingError);

	le=theList->list+theList->currentList;
	if (le->ptr) {		/* Exists in heap. */
		le->hit = ++theList->thisHit;
		return(le->ptr);
	}
	if(le->handle) {	/* Exists on heap, but not locked. */
		HLock(le->handle);
		le->hit = ++theList->thisHit;
		return(le->ptr=*le->handle);
	}
	if (le->position) {	/* != NULL means it exists in temp file */
		if(le->ptr=hmalloc(theList->size,&(le->handle))) {	/* space available */
			cvTempRead(fStuff->doc->dataPathRefNum,le->ptr,le->position,theList->size);
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
/*			li->position=tempWrite(li->ptr,(li->position)?li->position:-1L,theList->size); */
			le->ptr=li->ptr;	/* take its space */
			le->handle=li->handle;
			li->handle=0;
			li->ptr=0;			/* and show that it's not in heap. */
			cvTempRead(fStuff->doc->dataPathRefNum,le->ptr,le->position,theList->size);
			le->hit = ++theList->thisHit;
			return(le->ptr);
		}
		/* Can't swap. Must try to trim memory. */
		if(le->ptr=hemalloc(theList->size,&(le->handle))) { /* There is space, after trimming. */
			cvTempRead(fStuff->doc->dataPathRefNum,le->ptr,le->position,theList->size);
			le->hit = ++theList->thisHit;
			theList->nbuffers++;
			return(le->ptr);
		}
		errorAlertDialog(OutOfMemory);
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
/*		li->position=(li->position)? tempWrite(li->ptr,li->position,theList->size) */
/*			: tempWrite(li->ptr,-1L,theList->size); */
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
	errorAlertDialog(OutOfMemory);
	return 0;
}

/*----------------------------- LockLists ----------------------------



----------------------------------------------------------------------*/
void LockLists(FileStuff *fStuff)
{
	int i,master;
	ListElement *le;
	LongList *li;
	
	HLock(fStuff->longLists);
	li=(LongList *)*fStuff->longLists;
	le=(ListElement *)(li+fStuff->nLongLists);
	for(i=0;i<fStuff->nLongLists;i++,li++) {
		master=li->master;
		fStuff->masters[master].p=li;
		li->list=le;
		le+=li->nLE;
	}
	return;
}

void UnlockLists(FileStuff *fStuff)
{
	int i,j;
	ListElement *le;
	LongList *li;
	
	HLock(fStuff->longLists);	/* Just in case not locked */
	li=(LongList *)*fStuff->longLists;
	le=(ListElement *)(li+fStuff->nLongLists);
	for(i=0;i<fStuff->nLongLists;i++,li++) {
		fStuff->masters[li->master].p=0;
		for(j=0;j<li->nLE;le++,j++) {
			if(le->ptr && le->handle) {
				HUnlock(le->handle);
				le->ptr=0;
			}
		}
	}
	HUnlock(fStuff->longLists);
}

void DisposeLists(FileStuff *fStuff)
{
	int i,j;
	ListElement *le;
	LongList *li;

	UnlockLists(fStuff);
	HLock(fStuff->longLists);	/* Just in case not locked */
	
	li=(LongList *)*fStuff->longLists;
	le=(ListElement *)(li+fStuff->nLongLists);
	for(i=0;i<fStuff->nLongLists;i++,li++) {
		for(j=0;j<li->nLE;le++,j++) {
			if(le->handle) 
				DisposeHandle(le->handle);
		}
	}
	HUnlock(fStuff->longLists);
	DisposeHandle(fStuff->longLists);
	fStuff->longLists=0;
}

void SelectObject(Point locPt,WindowPtr eventWindow)
{
	FileStuff *fStuff;

	BitMap *offscreenBitMap;
	unsigned short rowBytes;
	int vv,hh,i,j,nFSBs,surveySelected,locked=0;
	Ptr bits;
	unsigned char mask;
	Point mapPt,mapPt2;
	Station deselect;
	Bounds *FSBs,*surveys,*base;
	Survey *su,*su0;
	Rect selectRect;
	
	HLock(((DocumentPeek)eventWindow)->docData);
	fStuff=(FileStuff *)*((DocumentPeek)eventWindow)->docData;

	deselect=fStuff->selected;
	selectRect=fStuff->selectedBounds;
	if(deselect.survey) {
		setOffscreenPort(fStuff,1);
		LockLists(fStuff);
		HLock(fStuff->intData);
		locked=TRUE;
		HiliteSurvey(getSurvey(deselect.survey),fStuff,0);
	}
	fStuff->selected.survey=0;	/* There will either be a change in selection or a deselection. */
	((DocumentPeek)eventWindow)->fHaveSelection=0;
	
	offscreenBitMap=getOffscreenBits(fStuff);
	rowBytes=offscreenBitMap->rowBytes&0x1FFF;
	vv=(locPt.v+fStuff->DvBM)*rowBytes;
	hh=locPt.h+fStuff->DhBM;
	if(*(bits=(offscreenBitMap->baseAddr+vv+hh/8))) {
		mask=0x80>>(hh%8);
		if((*bits)&mask) {
			*bits-=mask;
			if(!locked) {
				setOffscreenPort(fStuff,1);
				HLock(fStuff->intData);
				LockLists(fStuff);
				locked=TRUE;
			}
			HLock(fStuff->boundsData);
			mapPt2=mapPt=locPt;
			LocToMap(&mapPt,fStuff);
			mapPt2.v--;
			mapPt2.h++;
			LocToMap(&mapPt2,fStuff);
/*				mapPt2.v=-mapPt2.v; */
/*				mapPt.v=-mapPt.v;	/* Convert back to QuickDraw convention. */
			base=FSBs=(Bounds *)*fStuff->boundsData;
			nFSBs=FSBs->who;
			for(surveySelected=i=0;!surveySelected && i<nFSBs;i++,FSBs++) {
				if(PtInRect(mapPt,&FSBs->bounds)||PtInRect(mapPt2,&FSBs->bounds)) {
					surveys=base+FSBs->who;
					for(j=0;j<FSBs->what;j++,surveys++) {
						if(PtInRect(mapPt,&surveys->bounds)||PtInRect(mapPt2,&surveys->bounds)) {
							su=getSurvey(surveys->who);
							DrawSurvey(su,fStuff,0);
							if((*bits)&mask) {
								fStuff->selected.survey=surveys->who;
								fStuff->selectedBounds=surveys->bounds;
								fStuff->selectedFSB=SelectedFSB=su->FSB;
								fStuff->sequence=1;
								desigCpy(SelectedDesig,su->desig);
								desigCpy(fStuff->selectedDesig,su->desig);
								if(fStuff->selectedTie.survey) {
									su0=getSurvey(fStuff->selectedTie.survey);
									desigCpy(fStuff->tieDesig,su0->desig);
									fStuff->tieFSB=su0->FSB;
								}
								SurveySelected=surveySelected=TRUE;
								((DocumentPeek)eventWindow)->fHaveSelection=fStuff->selected.survey;
								break;
							}
						}
					}
				}
			}
			HUnlock(fStuff->boundsData);
		}
	}
	if(fStuff->selected.survey) 
		HiliteSurvey(su,fStuff,1);
	unlockOffscreenBits(fStuff);
	if(locked)
		setOffscreenPort(fStuff,0);
	SetPort(eventWindow);
	if(deselect.survey) {
		RectMapToLoc(&selectRect,fStuff);
		InsetRect(&selectRect,-2,-2);
		EraseRect(&selectRect);
		InvalRect(&selectRect);
	}
	if(fStuff->selected.survey) {
		selectRect=fStuff->selectedBounds;
		RectMapToLoc(&selectRect,fStuff);
		InsetRect(&selectRect,-2,-2);
		EraseRect(&selectRect);
		InvalRect(&selectRect);
	} else
		DisableItem(FindMenu,3);
	if(locked) {
		HUnlock(fStuff->intData);
		UnlockLists(fStuff);
	}
	HUnlock(((DocumentPeek)eventWindow)->docData);
	SurveySelected=surveySelected;
	showMouseLoc(eventWindow,true);
		
	return;
}

/*------------------------------- FillList --------------------------------
	Creates the List Manager list of surveys used by the Select Survey
	Modal Dialog. It could be combined with FillBounds.
---------------------------------------------------------------------------*/

ListHandle FillList(FileStuff *fStuff,WindowPtr selectDialog)
{
	int i,n;
	short *FSBList;
	ListHandle theList;
	DataPtr listData;
	Rect box,dataBounds;
	Point cellSize,cell;
	Survey *su;
	LongList *surveyList;
	char name[16],*book,*desig;
	short itemType,nCells;
	Handle itemHand;

	GetDItem(selectDialog,3,&itemType,&itemHand,&box);
	InsetRect(&box,1,1);
	
	cellSize.v=15;
	cellSize.h=box.right-box.left;
	n=(box.bottom-box.top)/cellSize.v;
	box.bottom=box.top+n*cellSize.v;
	LockLists(fStuff);

	surveyList=fStuff->masters[nSegList].p;
	nCells=(fStuff->literals)? surveyList->current-1 : surveyList->current;
	SetRect(&dataBounds,0,0,1,nCells);
	theList=LNew(&box,&dataBounds,cellSize,0,selectDialog,FALSE,FALSE,FALSE,TRUE);
	if(!theList) {
		SysBeep(2);
		return 0;
	}
	
	HLock(fStuff->fieldList);
	
	FSBList=(short *)*fStuff->fieldList;
	n=fStuff->maxField-fStuff->minField;
	cell.h=0;
	cell.v=0;
	book=&name[0];
	desig=book+5;
	for(i=0;i<=n;i++) {
		if(FSBList[i]) {
			sprintf(book,"%4d ",i+fStuff->minField);
			for(su=getSurvey(FSBList[i]);su;su=(su->next)?getSurvey(su->next):0) {
				sprintf(desig,"%-8.4s",su->desig);
				LSetCell(book,10,cell,theList);
				cell.v++;
			}
		}
	}
	UnlockLists(fStuff);
	HUnlock(fStuff->fieldList);
	if(ListDisplayRule==2) {
		HLock((Handle)(**theList).cells);
		listData=*((**theList).cells);
		qsort(listData,(size_t)dataBounds.bottom,(size_t)10,dcompare);
		HUnlock((Handle)(**theList).cells);
	}
	return theList;
}

int dcompare(char *arg1,char *arg2)
{   
	int n;
	if(n=strncmp(arg1+5,arg2+5,4))
		return n;
	return (strncmp(arg1,arg2,4));
}

int ccompare(char *arg1,char *arg2)
{
	return (strncmp(arg1,arg2,4));
}

/*------------------------------- FillBounds ------------------------------
	Creates the bounds array that summarizes the bounding rectangles of 
	the FSB and Survey data types and also finds the global bounding 
	rectangle. It expects the LongLists to be locked when it is called
	and it leaves them locked. It unlocks the list it creates and the
	FSBList that it uses. 
	
	The coordinate system is the same as for the data. That means that
	the vertical dimensions are sign reversed.
---------------------------------------------------------------------------*/
	
int FillBounds(FileStuff *fStuff)
{
	int i,cnt,n,nSu;
	short ftop,fbottom,fleft,fright;
	short btop,bbottom,bleft,bright;
	short top,bottom,left,right;
	short topSurvey=1,bottomSurvey=1,leftSurvey=1,rightSurvey=1;
	short topS,bottomS,leftS,rightS;

	Bounds *FSBs,*surveys;
	Survey *su;
	LongList *surveyList;
	float extent;
	short *FSBList;
	
	surveyList=fStuff->masters[nSegList].p;
	HLock(fStuff->fieldList);
	
	FSBList=(short *)*fStuff->fieldList;
	n=fStuff->maxField-fStuff->minField;
	
	for(cnt=i=0;i<=n;i++) {
		if(FSBList[i]) 
			cnt++;
	}
	fStuff->boundsData=NewHandle(n=(cnt+surveyList->current)*sizeof(Bounds));
	if(!fStuff->boundsData) {
		HUnlock(fStuff->fieldList);
		fStuff->boundsData=0;
		errorAlertDialog(-OutOfMemory);
		return 0;
	}
	HLock(fStuff->boundsData);
	FSBs=(Bounds *)*fStuff->boundsData;
	surveys=FSBs+cnt;
	memset(FSBs,0,n);	
	FSBs->who=cnt;		/* Number of FSBs is also index into beginning of survey 
							bounds data. */
	
	/* Set up for taking reconstruction (field) extents. In this usage, top>bottom.
		When filling a Rect, top and bottom must be reversed. */
	
	su=getSurvey(1);
	fleft=su->LSW.x;
	ftop=su->UNE.y;
	fright=su->UNE.x;
	fbottom=su->LSW.y;
	
	n=fStuff->maxField-fStuff->minField;
	
	for(i=0;i<=n;i++) {
		if(FSBList[i]) {
			FSBs->who=cnt;	/* index to place in list of survey bounds */
			su=getSurvey(FSBList[i]);
			nSu=0;
			while(su && !(su->flags&FixedFlag)) {
				nSu++;
				su=(su->next)? getSurvey(su->next):0;
			}
			if(su) {
				/*	These rects are in real-world coordinates without the
					Y-axis negated. To use with screen and graphic's device 
					coorinate system, verticals have to be reversed. */
				bleft=su->LSW.x;
				btop=su->UNE.y;
				bright=su->UNE.x;
				bbottom=su->LSW.y;
				leftS=topS=rightS=bottomS=su->survey;
				while(su) {
					nSu++;
					/* There are just two fields of importance, the bounds 
						and the survey number */
					left=su->LSW.x;
					top=su->UNE.y;
					right=su->UNE.x;
					bottom=su->LSW.y;
					right++;	/* QD convention. The bounding coordinate to the */
					top++;		/* lower right of a drawn point. Rember, top will be bottom. */
					/* Note top and bottom reversed. */
					SetRect(&surveys->bounds,left,bottom,right,top);
					surveys->who=su->survey;
					if(su->flags&FixedFlag) {
						if(left<bleft) {
							bleft=left;
							leftS=su->survey;
						}
						if(top>btop) {
							btop=top;
							topS=su->survey;
						}
						if(right>bright) {
							bright=right;
							rightS=su->survey;
						}
						if(bottom<bbottom) {
							bbottom=bottom;
							bottomS=su->survey;
						}
					}
					surveys++;
					cnt++;
					su=(su->next)? getSurvey(su->next):0;
				}
				if(bleft<fleft) {
					fleft=bleft;
					leftSurvey=leftS;
				}
				if(btop>ftop) {
					ftop=btop;
					topSurvey=topS;
				}
				if(bright>fright) {
					fright=bright;
					rightSurvey=rightS;
				}
				if(bbottom<fbottom) {
					fbottom=bbottom;
					bottomSurvey=bottomS;
				}
				SetRect(&FSBs->bounds,bleft,bbottom,bright,btop);
			} else {	/* no fixed surveys in book. */
				SetRect(&FSBs->bounds,0,0,0,0);
			}
			FSBs->what=nSu;
			FSBs++;
		}
	}
		
	HUnlock(fStuff->boundsData);
	HUnlock(fStuff->fieldList);
	
	
	SetRect(&fStuff->bounds,fleft,fbottom,fright,ftop);
	InsetRect(&fStuff->bounds,-20,-20);

	/* 	The RW center of a new document is initially the center. */
	
	fStuff->XMapCtr=(short)((int)fright+(int)fleft)/2;
	fStuff->YMapCtr=(short)((int)fbottom+(int)ftop)/2;
	
	/* check extents */
	su=getSurvey(leftSurvey);
	extent=su->LSW.x;
	su=getSurvey(rightSurvey);
	if(fabs(su->UNE.x-extent)>3.2768e+4)
		return FALSE;
	su=getSurvey(bottomSurvey);
	extent=su->LSW.y;
	su=getSurvey(topSurvey);
	if(fabs(su->UNE.y-extent)>3.2768e+4)
		return FALSE;
	return TRUE;
}


void HiliteSurvey(Survey *su,FileStuff *fStuff,int how)
{
	if(how) {	/* Will Hilite */
		PenSize(1,1);
		PenMode(patBic);
		DrawSurvey(su,fStuff,0);
		PenSize(2,2);
		PenMode(patXor);
		DrawSurvey(su,fStuff,how);
	} else {	/* Unhilite */
		PenSize(2,2);
		PenMode(patXor);
		DrawSurvey(su,fStuff,fStuff->sequence);
		PenSize(1,1);
		PenMode(patCopy);
		DrawSurvey(su,fStuff,0);
	}

	PenSize(1,1);
	PenMode(patCopy);

	return;
}

#if 0
void HiliteSurvey(Survey *su,FileStuff *fStuff,int how)
{
	int i;
	short gain,mapX,mapY,bmX,bmY;
	IntData *data,*locn;
	Link *li;
	Locn *lo;

	
	/* This is a MapToBM conversion. */
	mapX=fStuff->XMapCtr;
	mapY=fStuff->YMapCtr;
	bmX=fStuff->xBMCtr;		/* Center of offscreen bitmap. Should be 512. */
	bmY=fStuff->yBMCtr;
	locn=(IntData *)*fStuff->intData;
	data=locn+fStuff->masters[nLocnList].other+su->data+1;
	gain=fStuff->gain;
	
	if(how) {	/* Will Hilite */
		PenSize(1,1);
		PenMode(patBic);
	} else {	/* Unhilite */
		PenSize(2,2);
		PenMode(patXor);
	}
	
	data=locn+fStuff->masters[nLocnList].other+su->data+1;

	if(gain>=0) {
		for(i=0;i<su->nStations;i++,data++) {
			if(data->code&Move_to)
				MoveTo(bmX+((data->x-mapX)<<gain),
						bmY-((data->y-mapY)<<gain));
			else if(data->code&Line_to)
				LineTo(bmX+((data->x-mapX)<<gain),
						bmY-((data->y-mapY)<<gain));
		}
		for(i=su->links+su->nLocns+su->nLinks;
			i<su->links+su->nLocns+su->nLinks+su->nTies;i++) {
			li=getLink(i);
			data=locn+li->data-1;
			lo=getLocn(li->data-1);
			if(lo->length==su->survey && (-li->to.survey)!=su->survey) {	/* belongs && not duplicate within same survey */
				if(!(data->code&Virtual)) {
					if((data->code&(_Point|_Rel))==Move_to) {
						MoveTo(bmX+((data->x-mapX)<<gain),
								bmY-((data->y-mapY)<<gain));
						data++;
						LineTo(bmX+((data->x-mapX)<<gain),
								bmY-((data->y-mapY)<<gain));
					}
				}
			}
			
		}
	} else {
		for(i=0;i<su->nStations;i++,data++) {
			if(data->code&Move_to)
				MoveTo(bmX+((data->x-mapX)>>-gain),
						bmY-((data->y-mapY)>>-gain));
			else if(data->code&Line_to)
				LineTo(bmX+((data->x-mapX)>>-gain),
						bmY-((data->y-mapY)>>-gain));
		}
		for(i=su->links+su->nLocns+su->nLinks;
			i<su->links+su->nLocns+su->nLinks+su->nTies;i++) {
			li=getLink(i);
			data=locn+li->data-1;
			lo=getLocn(li->data-1);
			if(lo->length==su->survey && (-li->to.survey)!=su->survey) {
				if(!(data->code&Virtual)) {
					if((data->code&(_Point|_Rel))==Move_to) {
						MoveTo(bmX+((data->x-mapX)>>-gain),
								bmY-((data->y-mapY)>>-gain));
						data++;
						LineTo(bmX+((data->x-mapX)>>-gain),
								bmY-((data->y-mapY)>>-gain));
					}
				}
			}
			
		}
	}

	if(how) {	/* Will Hilite */
		PenSize(2,2);
		PenMode(patXor);
	} else {	/* Unhilite */
		PenSize(1,1);
		PenMode(patCopy);
	}
	
	data=locn+fStuff->masters[nLocnList].other+su->data+1;

	if(gain>=0) {
		for(i=0;i<su->nStations;i++,data++) {
			if(data->code&Move_to)
				MoveTo(bmX+((data->x-mapX)<<gain),
						bmY-((data->y-mapY)<<gain));
			else if(data->code&Line_to)
				LineTo(bmX+((data->x-mapX)<<gain),
						bmY-((data->y-mapY)<<gain));
		}
		for(i=su->links+su->nLocns+su->nLinks;
			i<su->links+su->nLocns+su->nLinks+su->nTies;i++) {
			li=getLink(i);
			data=locn+li->data-1;
			lo=getLocn(li->data-1);
			if(lo->length==su->survey && (-li->to.survey)!=su->survey) {
				if(!(data->code&Virtual)) {
					if((data->code&(_Point|_Rel))==Move_to) {
						MoveTo(bmX+((data->x-mapX)<<gain),
								bmY-((data->y-mapY)<<gain));
						data++;
						LineTo(bmX+((data->x-mapX)<<gain),
								bmY-((data->y-mapY)<<gain));
					}
				}
			}
			
		}
	} else {
		gain=-gain;
		for(i=0;i<su->nStations;i++,data++) {
			if(data->code&Move_to)
				MoveTo(bmX+((data->x-mapX)>>gain),
						bmY-((data->y-mapY)>>gain));
			else if(data->code&Line_to)
				LineTo(bmX+((data->x-mapX)>>gain),
						bmY-((data->y-mapY)>>gain));
		}
		for(i=su->links+su->nLocns+su->nLinks;
			i<su->links+su->nLocns+su->nLinks+su->nTies;i++) {
			li=getLink(i);
			data=locn+li->data-1;
			lo=getLocn(li->data-1);
			if(lo->length==su->survey && (-li->to.survey)!=su->survey) {
				if(!(data->code&Virtual)) {
					if((data->code&(_Point|_Rel))==Move_to) {
						MoveTo(bmX+((data->x-mapX)>>gain),
								bmY-((data->y-mapY)>>gain));
						data++;
						LineTo(bmX+((data->x-mapX)>>gain),
								bmY-((data->y-mapY)>>gain));
					}
				}
			}
			
		}
	}
	PenSize(1,1);
	PenMode(patCopy);
	
	return;
}
#endif

void HiliteStation(short h, short v)
{
	Rect r;
	PenState pen;

	GetPenState(&pen);
	SetRect(&r,h-2,v-2,h+4,v+4);
	FrameRect(&r);
	SetPenState(&pen);
}

#if 1
int DrawSurvey(Survey *su,FileStuff *fStuff,int sequence)
{
	int i,j,cnt,ret=0;
	short gain,mapX,mapY,bmX,bmY;
	IntData *data,*locn;
	Link *li;
	Locn *lo;
	SubSurvey *ss;
	short tempx,tempy;
	
	/* This is a MapToBM conversion. */
	mapX=fStuff->XMapCtr;
	mapY=fStuff->YMapCtr;
	bmX=fStuff->xBMCtr;	/* Center of offscreen bitmap. Should be 512. */
	bmY=fStuff->yBMCtr;
	gain=fStuff->gain;
	locn=(IntData *)*fStuff->intData;
	
	for(ss=(SubSurvey *) su;ss;ss=(ss->nextSubSeg)? getSubSurvey(ss->nextSubSeg):0) {
		data=locn+fStuff->masters[nLocnList].other+ss->data+1;
		for(i=j=0,cnt=ss->last-ss->first;i<=cnt;i++,data++) {
			tempx=bmX+ScaleToBM((data->x-mapX),gain);
			tempy=bmY-ScaleToBM((data->y-mapY),gain);
			j++;
			if(j==sequence) { 
				HiliteStation(tempx,tempy);
				ret=fStuff->selected.n=ss->first+i;
				fStuff->selectedTie.survey=0;
				fStuff->sequence=sequence;
			}
			if(data->code&Move_to)
				MoveTo(tempx,tempy);
			else if(data->code&Line_to)
				LineTo(tempx,tempy);
		}
	}
	for(i=su->links+su->nLocns+su->nLinks;
		i<su->links+su->nLocns+su->nLinks+su->nTies;i++) {
		li=getLink(i);
		data=locn+li->data-1;
		lo=getLocn(li->data-1);
		if(lo->length==su->survey && (-li->to.survey)!=su->survey) {
			if(!(data->code&Virtual)) {
				if((data->code&(_Point|_Rel))==Move_to) {
					j++;
					tempx=bmX+ScaleToBM((data->x-mapX),gain);
					tempy=bmY-ScaleToBM((data->y-mapY),gain);
					if(j==sequence) {
						HiliteStation(tempx,tempy);
						ret=fStuff->selected.n=li->from.n;
						fStuff->selectedTie.n=li->to.n;
						fStuff->selectedTie.survey=abs(li->to.survey);
						fStuff->sequence=sequence;
					}
					MoveTo(tempx,tempy);
					data++;
					LineTo(tempx,tempy);
				}
			}
		}
	}
	if(sequence && !ret) {	/* sequence out of range */
		if(sequence<0) {	/* must select last. */
			fStuff->sequence=sequence=j;
			if(su->nTies) {
				data--;
				ret=fStuff->selected.n=li->from.n;
				fStuff->selectedTie=li->to;
			} else {
				ret=fStuff->selected.n=ss->first+i-1;
				fStuff->selectedTie.survey=0;
			}
		} else {		/* must select first */
			fStuff->sequence=sequence=1;
			data=0;
			if(su->data) {
				data=locn+fStuff->masters[nLocnList].other+su->data+1;
			} else if(su->nTies) {
				for(i=su->links+su->nLocns+su->nLinks;
					i<su->links+su->nLocns+su->nLinks+su->nTies;i++) {
					li=getLink(i);
					data=locn+li->data-1;
					lo=getLocn(li->data-1);
					if(lo->length==su->survey && (-li->to.survey)!=su->survey) {
						if(!(data->code&Virtual)) {
							if((data->code&(_Point|_Rel))==Move_to) {
								li=getLink(su->links+su->nLocns+su->nLinks);
								data=locn+li->data-1;
								break;
							} else
								data=0;
						} else
							data=0;
					}
				}
			}
		}
		if(data) {
			tempx=bmX+ScaleToBM((data->x-mapX),gain);
			tempy=bmY-ScaleToBM((data->y-mapY),gain);
			HiliteStation(tempx,tempy);
		}
	}
	return ret;
}

#else
int DrawSurvey(Survey *su,FileStuff *fStuff,int sequence)
{
	int i,j=1,cnt,ret=0;
	short gain,mapX,mapY,bmX,bmY;
	IntData *data,*locn;
	Link *li;
	Locn *lo;
	SubSurvey *ss;
	
	/* This is a MapToBM conversion. */
	mapX=fStuff->XMapCtr;
	mapY=fStuff->YMapCtr;
	bmX=fStuff->xBMCtr;	/* Center of offscreen bitmap. Should be 512. */
	bmY=fStuff->yBMCtr;
	gain=fStuff->gain;
	locn=(IntData *)*fStuff->intData;
	
	if(gain>=0) {
		for(ss=(SubSurvey *) su;ss;ss=(ss->nextSubSeg)? getSubSurvey(ss->nextSubSeg):0) {
			data=locn+fStuff->masters[nLocnList].other+ss->data+1;
			for(i=0,cnt=ss->last-ss->first;i<=cnt;i++,j++,data++) {
				if(j==sequence) {
					HiliteStation(bmX+((data->x-mapX)<<gain),bmY-((data->y-mapY)<<gain));
					ret=ss->first+i;
					fStuff->sequence=sequence;
				}
				if(data->code&Move_to)
					MoveTo(tempx=bmX+((data->x-mapX)<<gain),tempy=bmY-((data->y-mapY)<<gain));
				else if(data->code&Line_to)
					LineTo(tempx=bmX+((data->x-mapX)<<gain),tempy=bmY-((data->y-mapY)<<gain));
			}
		}
		for(i=su->links+su->nLocns+su->nLinks;
			i<su->links+su->nLocns+su->nLinks+su->nTies;i++) {
			li=getLink(i);
			data=locn+li->data-1;
			lo=getLocn(li->data-1);
			if(lo->length==su->survey && (-li->to.survey)!=su->survey) {
				if(!(data->code&Virtual)) {
					if((data->code&(_Point|_Rel))==Move_to) {
						j++;
						if(j==sequence) {
							HiliteStation(bmX+((data->x-mapX)<<gain),bmY-((data->y-mapY)<<gain));
							ret=li->from.n;
							fStuff->sequence=sequence;
						}
						MoveTo(bmX+((data->x-mapX)<<gain), bmY-((data->y-mapY)<<gain));
						data++;
						LineTo(bmX+((data->x-mapX)<<gain), bmY-((data->y-mapY)<<gain));
					}
				}
			}
		} 
	} else {
		gain=-gain;
		for(ss=(SubSurvey *) su;ss;ss=(ss->nextSubSeg)? getSubSurvey(ss->nextSubSeg):0) {
			data=locn+fStuff->masters[nLocnList].other+ss->data+1;
			for(i=0,cnt=ss->last-ss->first;i<=cnt;i++,j++,data++) {
				if(j==sequence) {
					HiliteStation(bmX+((data->x-mapX)>>gain),bmY-((data->y-mapY)>>gain));
					ret=ss->first+i;
					fStuff->sequence=sequence;
				}
				if(data->code&Move_to)
					MoveTo(tempx=bmX+((data->x-mapX)>>gain),tempy=bmY-((data->y-mapY)>>gain));
				else if(data->code&Line_to)
					LineTo(tempx=bmX+((data->x-mapX)>>gain),tempy=bmY-((data->y-mapY)>>gain));
			}
		}
		for(i=su->links+su->nLocns+su->nLinks;
			i<su->links+su->nLocns+su->nLinks+su->nTies;i++) {
			li=getLink(i);
			data=locn+li->data-1;
			lo=getLocn(li->data-1);
			if(lo->length==su->survey && (-li->to.survey)!=su->survey) {
				if(!(data->code&Virtual)) {
					if((data->code&(_Point|_Rel))==Move_to) {
						j++;
						if(j==sequence) {
							HiliteStation(bmX+((data->x-mapX)>>gain),bmY-((data->y-mapY)>>gain));
							ret=li->from.n;
							fStuff->sequence=sequence;
						}
						MoveTo(bmX+((data->x-mapX)>>gain), bmY-((data->y-mapY)>>gain));
						data++;
						LineTo(bmX+((data->x-mapX)>>gain), bmY-((data->y-mapY)>>gain));
					}
				}
			}
			
		}
	}
	if(sequence>j) {
	
	}
	return ret;
}
#endif

#ifdef MC68000
#pragma segment reports
#endif

void OpenTextWindow(WindowPtr theWindow, int type)
{
	FileStuff *fStuff;
	DocumentPeek textDoc;
	Survey *su;
	char *buf;
	char suName[32];
	int nChars,nLines,i,width,lineWidth;
	
	HLock(((DocumentPeek)theWindow)->docData);
	fStuff=(FileStuff *)*((DocumentPeek)theWindow)->docData;

	if(fStuff->selected.survey) {
		if(textDoc = MakeDoc('TEXT',0)) {
			if(((*textDoc->makeWindow)((WindowPtr)textDoc)) && ((*textDoc->initDoc)((WindowPtr)textDoc))) {
				buf=NewPtr(3000);
				if(!buf) {
					SysBeep(2);
					errorAlertDialog(-OutOfMemory);
					HUnlock(((DocumentPeek)theWindow)->docData);
					DeleteDoc(textDoc);
					return;
				}
				LockLists(fStuff);
				su=getSurvey(fStuff->selected.survey);
				switch (type) {
					case 1:		/* Schematic */
						nChars=reportSurvey(su,buf,fStuff);
						sprintf(suName,"Survey $%d%1.4s",su->FSB,su->desig);
						strcpy((char*)textDoc->fileSpecs.name,suName);
						SetDocWindowTitle((WindowPtr)textDoc,(StringPtr)c2pstr((char*)textDoc->fileSpecs.name));
						break;
					case 2:		/* Header */
						nChars=reportHeader(su,buf,fStuff);
						sprintf(suName,"Book $%d",su->FSB);
						strcpy((char*)textDoc->fileSpecs.name,suName);
						SetDocWindowTitle((WindowPtr)textDoc,(StringPtr)c2pstr((char*)textDoc->fileSpecs.name));
						break;
					case 3:		/* Rose text */
					default:
						nChars=0;
						break;
				}
				UnlockLists(fStuff);
				if(nChars) {
					for(nLines=lineWidth=i=0;i<nChars;i+=(width+1),nLines++) {
						width=strlen(buf+i);
						*(buf+i+width)='\n';
						lineWidth=(width>lineWidth)? width:lineWidth;
					}
					SetPort((WindowPtr)textDoc);
					SizeWindow((WindowPtr)textDoc,lineWidth*8+20,nLines*9+50,true);
					SetTERect((WindowPtr)textDoc);
					SizeScrollBars((WindowPtr)textDoc);
					AdjustScrollBars((WindowPtr)textDoc);
					AddText((WindowPtr)textDoc,buf,nChars);
					SetScrollBarValues((WindowPtr)textDoc);

					((DocumentPeek)textDoc)->fNeedtoSave = false;
					((DocumentPeek)textDoc)->associatedWindow = theWindow;		/* Map window to which this is attached */
					DisposePtr(buf);
					ShowDocWindow((WindowPtr)textDoc);			
				}
			}
		}
		HUnlock(((DocumentPeek)theWindow)->docData);
		return;
	} 

	SysBeep(2);
	HUnlock(((DocumentPeek)theWindow)->docData);
	return;
}

int reportHeader(Survey  *su,char *buf,FileStuff *fStuff)
{
	int k=0,n;
	indexType header,headerSurvey;
	short *FSBList;
	struct headerRec *head;
	Survey *su0;
	char *p;
	struct nameRec *place,*person;
	struct dbRec *party;
	
	if(su->header)
		header=su->header;
	else {
		HLock(fStuff->fieldList);
		FSBList=(short *)*fStuff->fieldList;
		if(headerSurvey=FSBList[su->FSB-fStuff->minField]) {
			su0=getSurvey(headerSurvey);
			header=su0->header;
		}
			else 
				header=0;
		HUnlock(fStuff->fieldList);
	}
	if(head=(header)? getHeader(header):0) {
		if(su->FSB>0) {
			k+=1+sprintf(buf+k,"Survey $%d%1.4s (%d)",su->FSB,su->desig,su->survey);
		}
/*		k+=1+sprintf(buf+k," "); */
/*		k+=1+sprintf(buf+k," "); */
		if(head->date) {
			p=ctime(&head->date);
			k+=1+sprintf(buf+k," #Date %.10s%.5s",p,p+19);
		}
		if(head->object) {
			place=getPlace(head->object);
			k+=1+sprintf(buf+k," #Cave %s",place->name);
		}
		if(head->location) {
			place=getPlace(head->location);
			k+=1+sprintf(buf+k," #Area %s",place->name);
		}
		if(n=head->personnel) {
			k+=sprintf(buf+k," #Personnel");
			do {
				party=getDb(n);
				n++;
				person=getName(party->what);
				k+=sprintf(buf+k," %s",person->name);
				if(party->where&Leader)
					k+=sprintf(buf+k,"*");
				if(party->where&Job) {
					k+=sprintf(buf+k," (");
					if(party->where&Book)
						k+=sprintf(buf+k,"b");
					if(party->where&Compass)
						k+=sprintf(buf+k,"c");
					if(party->where&PointPerson)
						k+=sprintf(buf+k,"p");
					if(party->where&Sketch)
						k+=sprintf(buf+k,"s");
					if(party->where&Backsights)
						k+=sprintf(buf+k," bs ");
					if(party->where&Foresights)
						k+=sprintf(buf+k," fs ");
					if(party->where&Tape)
						k+=sprintf(buf+k,"t");
					k+=sprintf(buf+k,")");
				}
			} while (!(party->where&LastInParty));
/*			k+=1+sprintf(buf+k," "); */
		}
		k+=1+sprintf(buf+k," ");
	} else
		SysBeep(2);
	return k;
}

int reportSurvey(Survey  *su,char *buf,FileStuff *fStuff)
{
	String *theString;
	Link *theLink; 
	int i,j,desLen,k=0,lastLast=-1;
	int tieLength;
	char a[32];
		
	if(su->FSB>0) {
		k+=1+sprintf(buf+k,"Survey $%d%1.4s (%d)",su->FSB,su->desig,su->survey);
	}

	for(i=su->strings;i<su->strings+su->nStrings;i++) {
		theString = getStr(i);
		if(theString->code&UnMapped) {
			k+=1+sprintf(buf+k," ");
			k+=desLen=sprintf(buf+k,"%s ",cvPrintDesigSta(theString->first,su,a,fStuff));
			k+=cvPrintTies(su,buf+k,theString->first,desLen,&tieLength,fStuff);
			lastLast = theString->first;
		} else {
			if(theString->first<su->first) {
				lastLast=theString->last;
				continue;
			}
			if(!(theString->code&ContigAtStart)) {
				/* Print any unmapped links before this string. These can only occur
					if string is not contiguous at start. */
				for(j=su->links;j<su->links+su->nLocns+su->nLinks+su->nTies;j++) {
					theLink = getLink(j);
					if(theLink->from.n>lastLast && theLink->from.n<theString->first) {
						k+=1+sprintf(buf+k," ");
						k+=desLen=sprintf(buf+k,"%1.4s%s ",su->desig,printSta(theLink->from.n,a));
						k+=cvPrintTies(su,buf+k,theLink->from.n,desLen,&tieLength,fStuff);
						lastLast = theLink->from.n;
					}
				}
				/* Print the first station of the string and any ties. There is no preceeding
					contiguous string. */
				k+=1+sprintf(buf+k," ");
				k+=desLen=sprintf(buf+k,"%1.4s%s ",su->desig,printSta(theString->first,a));
				if(theString->code & NodeAtStart) 
					k+=cvPrintTies(su,buf+k,theString->first,desLen,&tieLength,fStuff);
				else 
					k++;
			}

			/* Print the vertical bar for the string itself. It has two flavors, dead end
				and not dead end. */
			if(theString->code&DeadEnd) {
				k+=1+sprintf(buf+k," |*");
				if(theString->length) {
					k--;
					k+=1+sprintf(buf+k," %d.%d",theString->length/10,theString->length%10);
				}
			} else {
				k+=1+sprintf(buf+k," |");
				if(theString->length) {
					k--;
					k+=1+sprintf(buf+k," %d.%d",theString->length/10,theString->length%10);
				}
			}
			/*	Print the last station and any links. */
			k+=desLen=sprintf(buf+k,"%1.4s%s ",su->desig,printSta(theString->last,a));
			if(theString->code & NodeAtEnd) 
				k+=cvPrintTies(su,buf+k,theString->last,desLen,&tieLength,fStuff);
			else 
				k++;
			/*	Get ready for next string. */
			lastLast=theString->last;
		}
	}
	return k;
}

/*--------------------------------- cvPrintTies -------------------------------
	Prints all the links to a station into *buf. Returns number of characters
	printed. Comes with the desig+station (i.e."A15" already printed).
	  
	A15	= [3400,1233,444]
	 .	= $476X10
	 .  -> $1500C1
	A15 <- $1502D15
----------------------------------------------------------------------------*/

int cvPrintTies(Survey *su,char *buf,int sta,int desLen,int *tieLength,FileStuff *fStuff)
{
	int i,lim,k=0;
	int fi=-1,la;
	Link *theLink;
	Locn *theLocn;
	Survey *su0;
	int tieDataBelongs,internalTie;
	char a[32],b[32];
	
	for(lim=i=su->links,lim+=su->nLocns+su->nTies+su->nLinks;i<lim;i++) {
		if((theLink=getLink(i))->from.n==sta) {
			fi=(fi<0)? i : fi;
			la = i;
		}
	}
	if(fi<0) {
		return 0;
	}
	for(i=fi;i<=la;i++) {
		if((theLink=getLink(i))->from.n == sta) {
			if(i != fi) {
				if(i == la)
					k+=sprintf(buf+k,"%s ",cvPrintDesigSta(sta,su,a,fStuff));
				else
					k+=sprintf(buf+k," .%*s",desLen-1," ");
			}
			if(!theLink->to.survey) {	/* = [123,456,789] */
				theLocn = getLocn(theLink->data);
				/* Adding 1 includes the terminal '\0', ending the string. */
				k+=(1+sprintf(buf+k,"= [%.1f,%.1f,%.1f]",theLocn->x,
						theLocn->y,theLocn->z));
			} else if(!theLink->data || theLink->from.survey & LinkFlag) { /* Link */
				su0 = getSurvey(theLink->to.survey);
				if(su0->FSB==su->FSB)
					k+=1+sprintf(buf+k,"= %s",cvPrintFullDesig(theLink->to.n,su0,b,fStuff));
				else
					k+=1+sprintf(buf+k,"= %s",cvPrintFullSta(theLink->to.n,su0,b,fStuff));
				if(theLink->from.survey&DeadEnd)
					k+=sprintf(buf+k,"*");
			} else {	/* Tie */
				if(internalTie=abs(theLink->to.survey)==su->survey)
					su0=su;
				else
					su0 = getSurvey(abs(theLink->to.survey));
				theLocn = getLocn(theLink->data);
				tieDataBelongs=theLocn[-1].length==su->survey;
				if(theLink->to.survey > 0) {
					/* 	Original tie; the from-station is in this survey, 
						but is it part of this segment's data? */
					if(tieDataBelongs) 
						*tieLength+=theLocn->length;
					else 
						k+=sprintf(buf+k,"(");
					k+=sprintf(buf+k,"-> ");
				} else {
					/* This is the reversed tie. The orignal to-station (the
						from station of this link) in this segment. Is it in
						the segment's data? */
					if(!internalTie) {
						/*	Reversed internal ties do not contribute to length but
							they do get normal notation. */
						if(tieDataBelongs)
							*tieLength+=theLocn->length;
						else
							k+=sprintf(buf+k,"(");
					}
					k+=sprintf(buf+k,"<- ");
				}
				if(su0->FSB==su->FSB)
					k+=sprintf(buf+k,"%s",cvPrintFullDesig(theLink->to.n,su0,b,fStuff));
				else
					k+=sprintf(buf+k,"%s",cvPrintFullSta(theLink->to.n,su0,b,fStuff));

				if(!tieDataBelongs)
					k+=sprintf(buf+k,")");
				if(theLocn->code&DeadEnd)
					k+=sprintf(buf+k,"*");
				if(theLink->to.survey&&theLink->data) {	/* ties only */
					/* str lengths only calculated in building matrix. */
					if(!tieDataBelongs||theLocn->code&DeadEnd)
						k+=sprintf(buf+k," %d.%d",theLocn->length/10,theLocn->length%10);
					else
						k+=sprintf(buf+k," %d.%d",theLocn->length/10,theLocn->length%10);
				
				}
				k++;
			}
		}
	}
	return k;
}

char *cvPrintDesigSta(int sta,Survey *su,char *buf,FileStuff *fStuff)
{
	int k=0;
	struct nameRec *lit;

	if(su->FSB>0) {
		k=sprintf(buf,"%.4s",su->desig);
		printSta(sta,buf+k);
	} else if(su->FSB==-1) {	/* lit */
		lit=getName(sta);
		k=sprintf(buf,"\'%s\'",lit->name);
	} else {	/* unresolved */
		k=sprintf(buf,"$(%d)%.4s",su->FSB,su->desig);
		printSta(sta,buf+k);
	}
	return buf;
}

char *cvPrintFullSta(int sta,Survey *seg,char *buf,FileStuff *fStuff)
{
	int k=0;
	struct nameRec *lit;

	if(seg->FSB>0) {
		k=sprintf(buf,"$%d%.4s",seg->FSB,seg->desig);
		printSta(sta,buf+k);
	} else if(seg->FSB==-1) {	/* lit */
		lit=getName(sta);
		k=sprintf(buf,"\'%s\'",lit->name);
	} else {	/* unresolved */
		k=sprintf(buf,"$(%d)%.4s",seg->FSB,seg->desig);
		printSta(sta,buf+k);
	}
	return buf;
}

char *cvPrintFullDesig(int sta,Survey *seg,char *buf,FileStuff *fStuff)
{
	int k=0;
	struct nameRec *lit;

	if(seg->FSB>0) {
		k=sprintf(buf,"%.4s",seg->desig);
		printSta(sta,buf+k);
	} else if(seg->FSB==-1) {	/* lit */
		lit=getName(sta);
		k=sprintf(buf,"\'%s\'",lit->name);
	} else {	/* unresolved */
		k=sprintf(buf,"$(%d)%.4s",seg->FSB,seg->desig);
		printSta(sta,buf+k);
	}
	return buf;
}

#ifdef MC68000
#pragma segment Dialogs
#endif

void doSelectDialog(WindowPtr wind)
{
	FileStuff *fStuff;
	DialogPtr selectDialog;
	short itemHit;
	short itemType;
	short dataLen;
	Handle itemHand;
	int pau=FALSE;
	Rect box,dataBounds;
	ListHandle list;
	DataPtr listData;
	int oldCenterFlag,oldScaleFlag,oldDisplayRule,reOrderList=FALSE;
	Point theCell;
	char data[16],*desig;
	int FSB,n;
	short *FSBList;
	Survey *su;
	short deselect;
	
	oldCenterFlag=DefaultCenterSelection;
	oldScaleFlag=DefaultScaleToFit;
	oldDisplayRule=ListDisplayRule;
	
	
	selectDialog=GetNewDialog(128,(Ptr)&DlogStor,(WindowPtr)(-1));
	if(!selectDialog)
		FatalError();

	HLock(((DocumentPeek)wind)->docData);
	fStuff=(FileStuff *)*((DocumentPeek)wind)->docData;
	if(list=FillList(fStuff,selectDialog)) {
		SetWRefCon(selectDialog,(long)list);
		if(fStuff->selected.survey) {
			sprintf(data,"%4d ",fStuff->selectedFSB);
			sprintf(data+5,"%-8.4s",fStuff->selectedDesig);
			theCell.v=0;
			theCell.h=0;
			if(LSearch(data,10,0,&theCell,list)) {
				LSetSelect(TRUE,theCell,list);
				LAutoScroll(list);
			}
		
		}
		
		UpdateList(list);
		
		/* Do the cute drawing */
		GetDItem(selectDialog,10,&itemType,&itemHand,&box);
		SetDItem(selectDialog,10,itemType,(Handle)doLine,&box);
		GetDItem(selectDialog,6,&itemType,&itemHand,&box);
		SetDItem(selectDialog,6,itemType,(Handle)doButton,&box);
		while(!pau) {
			/* Initialize the two radio buttons */
			GetDItem(selectDialog,7,&itemType,&itemHand,&box);
			SetCtlValue((ControlHandle)itemHand,ListDisplayRule==1);
			GetDItem(selectDialog,8,&itemType,&itemHand,&box);
			SetCtlValue((ControlHandle)itemHand,ListDisplayRule==2);

			/* Do the two check boxes */
			GetDItem(selectDialog,4,&itemType,&itemHand,&box);
			SetCtlValue((ControlHandle)itemHand,DefaultCenterSelection);
			GetDItem(selectDialog,5,&itemType,&itemHand,&box);
			SetCtlValue((ControlHandle)itemHand,DefaultScaleToFit);
			
			if(reOrderList) {
				reOrderList=FALSE;
				LDoDraw(false, list);
				theCell.v=0;
				theCell.h=0;
				if(LGetSelect(TRUE,&theCell,list)) {
					dataLen=10;
					LGetCell(data,&dataLen,theCell,list);
					LSetSelect(FALSE,theCell,list);
				} else
					data[0]=0;
				
				dataBounds=(**list).dataBounds;
				HLock((Handle)(**list).cells);
				listData=*((**list).cells);
		
				if(ListDisplayRule==2)
					qsort(listData,(size_t)dataBounds.bottom,(size_t)10,dcompare);
				else
					qsort(listData,(size_t)dataBounds.bottom,(size_t)10,ccompare);
					
				HUnlock((Handle)(**list).cells);
				if(data[0]) {
					theCell.v=0;
					theCell.h=0;
					if(LSearch(data,10,0,&theCell,list)) {
						LSetSelect(TRUE,theCell,list);
						LAutoScroll(list);
					}
				}
				UpdateList(list);
			}
				
			ModalDialog(listFilter,&itemHit);
			switch(itemHit) {
				case 1:		/* Select */
					pau=TRUE;
					theCell=LLastClick(list);
					if(theCell.v<0) {	/* Nothing selected */
						LDispose(list);
						CloseDialog(selectDialog);
						break;
					}
					dataLen=10;
					LGetCell(data,&dataLen,theCell,list);
					FSB=atoi(data);
					desig=data+5;
					while(!isspace(*desig)) desig++;
					*desig=0;
					desig=data+5;
					LockLists(fStuff);
					HLock(fStuff->fieldList);
					FSBList=(short *)*fStuff->fieldList;
					n=FSBList[FSB-fStuff->minField];
					for(su=getSurvey(n);su;su=(su->next)? getSurvey(su->next):0) {
						if(!strncmp(su->desig,desig,4))
							break;
					}
					LDispose(list);				/* Do these late in order to get a longer Hilite */
					CloseDialog(selectDialog);
					if(su) {
						deselect=fStuff->selected.survey;
						box=fStuff->selectedBounds;
						fStuff->selected.survey=su->survey;
						((DocumentPeek)wind)->fHaveSelection=su->survey;
						SetRect(&(fStuff->selectedBounds),
							(short)su->LSW.x,(short)su->LSW.y,
							(short)su->UNE.x,(short)su->UNE.y);
						fStuff->selectedFSB=SelectedFSB=FSB;
						desigCpy(SelectedDesig,su->desig);
						desigCpy(fStuff->selectedDesig,su->desig);
						SurveySelected=TRUE;
						setOffscreenPort(fStuff,1);
						HLock(fStuff->intData);
						if(deselect)
							HiliteSurvey(getSurvey(deselect),fStuff,0);
						HiliteSurvey(su,fStuff,1);
						HUnlock(fStuff->intData);
						setOffscreenPort(fStuff,0);
						SetPort(wind);
						if(deselect) {
							RectMapToLoc(&box,fStuff);
							InsetRect(&box,-2,-2);
							EraseRect(&box);
							InvalRect(&box);
						}
						box=fStuff->selectedBounds;
						RectMapToLoc(&box,fStuff);
						InsetRect(&box,-2,-2);
						EraseRect(&box);
						InvalRect(&box);
						InsetRect(&box,2,2);
						if(DefaultScaleToFit) {
							if(SizeToFit(fStuff,fStuff->selectedBounds)) {
								FillOffscreen((DocumentPeek)wind,0);
								HLock(((DocumentPeek)wind)->docData);
								fStuff=(FileStuff *)*((DocumentPeek)wind)->docData;
								EraseRect(&wind->portRect);
								InvalRect(&wind->portRect);
							}
						}
						if(DefaultCenterSelection) 
							DisplaySelection(wind);
					} else 
						SysBeep(2);
										
					HUnlock(fStuff->fieldList);
					UnlockLists(fStuff);
					break;
				case 2:		/* Cancel */
					pau=TRUE;
					DefaultCenterSelection=oldCenterFlag;
					DefaultScaleToFit=oldScaleFlag;
					ListDisplayRule=oldDisplayRule;
					LDispose(list);
					CloseDialog(selectDialog);
					break;
				case 4:		/* Center Checkbox */
					DefaultCenterSelection = !DefaultCenterSelection;
					break;
				case 5:		/* Scale to Fit Checkbox */
					DefaultScaleToFit = !DefaultScaleToFit;
					break;
				case 7:		/* By book radio button */
					if(ListDisplayRule!=1)
						reOrderList=TRUE;
					ListDisplayRule=1;
					break;
				case 8:		/* By Designation radio button */
					if(ListDisplayRule!=2)
						reOrderList=TRUE;
					ListDisplayRule=2;
					break;
				default:
					break;
			}
		}
	}
	HUnlock(((DocumentPeek)wind)->docData);
	return;
}
