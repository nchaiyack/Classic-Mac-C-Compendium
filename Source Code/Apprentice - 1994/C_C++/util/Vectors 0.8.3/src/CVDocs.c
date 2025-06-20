#include "CVectors.h"
#include "CVGlobs.h"

#ifdef MC68000
#pragma segment CVDocs
#endif

/*--------------------------- Declarations -------------------------------*/

void nullWindowMethod(WindowPtr);
int nullIntMethod(WindowPtr) ;

short nullShortMethod(WindowPtr);

void focusOnWindow(WindowPtr);
void focusOnContent(WindowPtr);
void getContentRect(WindowPtr, Rect *);

void InitLButton(LButton *,Rect *,char *,int,LButton *);

int StringToFloat(char *,float *);
pascal void doButton(DialogPtr,short);
pascal void doFrame(DialogPtr,short);
pascal void doLine(DialogPtr,short);
Boolean InitScrollDoc(WindowPtr);


int SizeToFit(FileStuff *,Rect);

DocumentPeek MakeDoc(OSType, FSSpecPtr);
short getVertPageScrollAmount(WindowPtr);
short getHorizPageScrollAmount(WindowPtr);

void AddMemberFunctions(DocumentPeek);
static void AddMapMemberFunctions(DocumentPeek);
static void AddRoseMemberFunctions(DocumentPeek);
extern void AddTitleMemberFunctions(DocumentPeek);
extern void AddEntryMemberFunctions(DocumentPeek);

void destructor(WindowPtr);
int DoClose(WindowPtr);
void scrollDoGrow(WindowPtr,EventRecord *);

void PtTimesGain(Point *where,int gain);
void FillOffscreen(DocumentPeek,int);
void TransformCoorinates(FileStuff *);
int doLittleButtons(WindowPtr, Point);

void LocToMap(Point *,FileStuff *);
void MapToBM(Point *,FileStuff *);
void MapToLoc(Point *,FileStuff *);
void RectMapToLoc(Rect *,FileStuff *);
void RectLocToMap(Rect *,FileStuff *);
void RectMapToBM(Rect *,FileStuff *);
void showMouseLoc(WindowPtr,Boolean);
void setOffscreenPort(FileStuff *,int);
void unlockOffscreenBits(FileStuff *fStuff);
BitMap *getOffscreenBits(FileStuff *fStuff);

int doGrow(WindowPtr,EventRecord *);
void doZoom(WindowPtr,short);

void mapActivate(WindowPtr);
void mapDeactivate(WindowPtr);
static void OpenRoseWindow(WindowPtr);

void doActivate(WindowPtr, EventRecord *);
void activate(WindowPtr);
void deactivate(WindowPtr);
void SizeScrollBars(WindowPtr);
void adjustScrollBars(WindowPtr);
void setScrollBarValues(WindowPtr);
void doContent(WindowPtr,EventRecord *);
void ScrollClick(WindowPtr,EventRecord *);
void DoButtonScroll(ControlHandle,Point);
pascal void ActionProc(ControlHandle,short);
void DoPageScroll(ControlHandle,short);
void DoThumbScroll(ControlHandle,Point);
void Scroll(ControlHandle,short);
void CombinedScroll(WindowPtr,short,short);
void doTheUpdate(WindowPtr, EventRecord *);
void nullDraw(WindowPtr,Rect *,short);

void SetLButton(LButton *aButton,int newValue);

static void MakeOnePane(void);

void focusOnWindow(WindowPtr wind)
{
	Rect r;
	
	SetPort(wind);
	SetOrigin(0,0);
	r=wind->portRect;
	ClipRect(&r);
}

void focusOnContent(WindowPtr wind)
{
	Rect r;
	
	SetPort(wind);
	SetOrigin(((DocumentPeek)wind)->hOffset,((DocumentPeek)wind)->vOffset);
	GetContentRect(wind,&r);
	ClipRect(&r);
}

void getContentRect(WindowPtr wind, Rect *r)
{
	*r = wind->portRect;
	if(((DocumentPeek)wind)->vScroll)
		r->right-=15;
	if(((DocumentPeek)wind)->hScroll)
		r->bottom-=15;
}

void doTheUpdate(WindowPtr wind, EventRecord *x)
{
#ifdef MC68000
#pragma unused (x)
#endif
	Rect r;
	if(((DocumentPeek)wind)->fDocWindow) {
		FocusOnContent(wind);
		BeginUpdate(wind);
		r=(**(wind->visRgn)).rgnBBox;
		(*((DocumentPeek)wind)->draw)(wind,&r,0);
		FocusOnWindow(wind);
		DrawControls(wind);
		DrawGrowIcon(wind);
		EndUpdate(wind);
	}
}

void nullDraw(WindowPtr pt,Rect *r,short x)
{
#ifdef MC68000
#pragma unused (pt,r,x)
#endif
	return;
}

int doGrow(WindowPtr wind,EventRecord *event)
{
	long		growResult;
	Rect r;
	
	SetPort(wind);
	growResult = GrowWindow(wind, event->where, &((DocumentPeek)wind)->limitRect);
	
	if (growResult) {							/* has it really changed size? */
		r = wind->portRect;
		r.left = r.right-15;
		InvalRect(&r);
		
		r = wind->portRect;	
		r.top = r.bottom-15;
		InvalRect(&r);

		SizeWindow(wind, LoWord(growResult), HiWord(growResult), TRUE);
		
		r = wind->portRect;
		r.left = r.right-15;
		InvalRect(&r);
		
		r = wind->portRect;
		r.top = r.bottom-15;
		InvalRect(&r);
		
		return 1;
	}
	return 0;
}

void doZoom(WindowPtr wind,short partCode)
{
	SetPort(wind);
	EraseRect(&wind->portRect);
	ZoomWindow(wind,partCode,wind==FrontWindow());
	InvalRect(&wind->portRect);
	return;
}

void doActivate(WindowPtr wind, EventRecord *event)
{
	if(((DocumentPeek)wind)->fDocWindow) {
		if ((event->modifiers & activeFlag)) 		 	/* It's an activate  */
			(*((DocumentPeek)wind)->activate)(wind);
		else
			(*((DocumentPeek)wind)->deactivate)(wind);
	}
}

void doContent(WindowPtr wind,EventRecord *event)
{
	Rect contents;
	
	FocusOnWindow(wind);
	GlobalToLocal(&event->where);
	GetContentRect(wind,&contents);
	if(!PtInRect(event->where,&contents))
		ScrollClick(wind,event);
}

void activate(WindowPtr wind)
{
	FocusOnWindow(wind);
	DrawGrowIcon(wind);
	if(((DocumentPeek)wind)->hScroll)
		ShowControl(((DocumentPeek)wind)->hScroll);
	if(((DocumentPeek)wind)->vScroll)
		ShowControl(((DocumentPeek)wind)->vScroll);
	CurrentScrollDocument=(DocumentPeek)wind;
	/* Each doc has its own copy of the markMenu */
	DeleteMenu(MARK_ID);
	MarkMenu=((DocumentPeek)wind)->markMenu;
	if(MarkMenu)
		InsertMenu(MarkMenu,WINDOW_ID);
	AdjustMenus(wind);
	DrawMenuBar();
	return;
}

void deactivate(WindowPtr wind)
{
	FocusOnWindow(wind);
	DrawGrowIcon(wind);
	if(((DocumentPeek)wind)->hScroll)
		HideControl(((DocumentPeek)wind)->hScroll);
	if(((DocumentPeek)wind)->vScroll)
		HideControl(((DocumentPeek)wind)->vScroll);
	return;
}

void SizeScrollBars(WindowPtr wind)
{
	Rect r;
	DocumentPeek doc;
	
	FocusOnWindow(wind);
	r=wind->portRect;
	doc = (DocumentPeek)wind;
	if(doc->vScroll) {
		SizeControl(doc->vScroll,16,r.bottom-r.top-15+2);
		MoveControl(doc->vScroll,r.right-15,-1);
		ValidRect(&(**doc->vScroll).contrlRect);
	}
	if(doc->hScroll) {
		SizeControl(doc->hScroll,r.right-r.left-15+2,16);
		MoveControl(doc->hScroll,-1,r.bottom-r.top-15);
		ValidRect(&(**doc->hScroll).contrlRect);
	}
}

void adjustScrollBars(WindowPtr wind)
{
	DocumentPeek doc;
	Rect r;
	short dh,dv;
	short currentValue;
	short newMax;
	RgnHandle oldClip;
	
	doc = (DocumentPeek)wind;
	GetContentRect(wind,&r);
	dh=dv=0;
	if(doc->vScroll) {
		currentValue=GetCtlValue(doc->vScroll);
		newMax = GetVertSize(wind) - (r.bottom - r.top);
		if(newMax<0)
			newMax=0;
		if(currentValue>newMax)
			dv = currentValue - newMax;
		SetCtlMax(doc->vScroll,newMax);
	}
	if(doc->hScroll) {
		currentValue=GetCtlValue(doc->hScroll);
		newMax = GetHorizSize(wind) - (r.right - r.left);
		if(newMax<0)
			newMax=0;
		if(currentValue>newMax)
			dh = currentValue - newMax;
		SetCtlMax(doc->hScroll,newMax);
	}
	if(dh | dv) {
		FocusOnContent(wind);
		InvalRect(&r);
		/* Shut down clip region to empty rectangle before calling
			ScrollContents. */
		oldClip = NewRgn();
		GetClip(oldClip);
		SetRect(&r,0,0,0,0);
		ClipRect(&r);
		ScrollContents(wind,dh,dv);
		SetClip(oldClip);
		DisposeRgn(oldClip);
	}
}

void setScrollBarValues(WindowPtr wind)
{
	DocumentPeek doc;
	
	FocusOnWindow(wind);
	doc = (DocumentPeek)wind;
	if(doc->hScroll)
		SetCtlValue(doc->hScroll,doc->hOffset);	
	if(doc->vScroll)
		SetCtlValue(doc->vScroll,doc->vOffset);	
}

void ScrollClick(WindowPtr wind,EventRecord *event)
{
	ControlHandle whichControl;
	short part;
	
	FocusOnWindow(wind);
	
	if(part = FindControl(event->where,wind,&whichControl)) {
		switch (part) {
			case inThumb:
				DoThumbScroll(whichControl,event->where);
				break;
			case inUpButton:
			case inDownButton:
				DoButtonScroll(whichControl,event->where);
				break;
			case inPageUp:
			case inPageDown:
				DoPageScroll(whichControl,part);
				while(TickCount() < event->when+6) ;
				break;
			default:
				break;
		}
	}
}

void DoPageScroll(ControlHandle theControl,short part)
{
	short scrollAmount;
	Point thePt;
	short currentPart;
	WindowPtr wind;
		
	wind = (**theControl).contrlOwner;
	if(theControl == ((DocumentPeek)wind)->vScroll)
		scrollAmount=(*((DocumentPeek)wind)->getVertPageScrollAmount)(wind);
	else
		scrollAmount=(*((DocumentPeek)wind)->getHorizPageScrollAmount)(wind);
		
	/* repeat as long as user holds down mouse button */
	do {
		GetMouse(&thePt);
		if(part<0) 
			currentPart=part=-part;
		else
			currentPart = TestControl(theControl,thePt);
		if(currentPart == part) {
			if(currentPart == inPageUp)
				Scroll(theControl,-scrollAmount);
			if(currentPart == inPageDown)
				Scroll(theControl,scrollAmount);
		}
	} while (Button());
}

void DoThumbScroll(ControlHandle theControl,Point localPt)
{
	short oldValue,trackResult,newValue,diff;
	
	WindowPtr wind;	
	oldValue = GetCtlValue(theControl);
	trackResult = TrackControl(theControl,localPt,NIL);
	if(trackResult) {
		newValue = GetCtlValue(theControl);
		diff = oldValue - newValue;
		wind = (**theControl).contrlOwner;
		FocusOnContent(wind);
		if(theControl==((DocumentPeek)wind)->hScroll)
			ScrollContents(wind,diff,0);
		if(theControl==((DocumentPeek)wind)->vScroll)
			ScrollContents(wind,0,diff);
		FocusOnWindow(wind);
	}
}
	
void Scroll(ControlHandle theControl,short change)
{
	/* Ref: p. 214 */
	/* This should not have to be overridden (cf. p. 215). More likely, 
		ScrollContents and SetScrollBarValues will be overridden. */
		
	WindowPtr wind;
	RgnHandle oldClip;
	long newValue;
	short diff=0;
	short oldValue;
	short minValue,maxValue;
	
	oldClip=NewRgn();
	GetClip(oldClip);
	
	oldValue = GetCtlValue(theControl);
	newValue = oldValue + change;
	
	if(change<0) {
		minValue = GetCtlMin(theControl);
		if(newValue < minValue)
			newValue = minValue;
	} else {
		maxValue = GetCtlMax(theControl);
		if(newValue  > maxValue)
			newValue = maxValue;
	}
	diff = oldValue - newValue;
	wind = (**theControl).contrlOwner;
	FocusOnContent(wind);
	if(theControl == ((DocumentPeek)wind)->vScroll)
		ScrollContents(wind,0,diff);
	if(theControl == ((DocumentPeek)wind)->hScroll)
		ScrollContents(wind,diff,0);
		
	FocusOnWindow(wind);
	SetScrollBarValues(wind);
	
	SetClip(oldClip);
	DisposeRgn(oldClip);

}

void CombinedScroll(WindowPtr wind,short dh,short dv)
{
	/* 
		This is Scroll for both directions simultaneously. It could
		be used for a hand tool (option-click). It is the working
		routine for Display Selection. 
	*/
		
	DocumentPeek doc;
	RgnHandle oldClip;
	long newValue;
	short diffH=0,diffV=0;
	short oldValue;
	short minValue,maxValue;
	
	oldClip=NewRgn();
	GetClip(oldClip);
	
	doc=(DocumentPeek)wind;
	
	if(dh) {
		oldValue = GetCtlValue(doc->hScroll);
		newValue = oldValue + dh;
	
		if(dh<0) {
			minValue = GetCtlMin(doc->hScroll);
			if(newValue < minValue)
				newValue = minValue;
		} else {
			maxValue = GetCtlMax(doc->hScroll);
			if(newValue  > maxValue)
				newValue = maxValue;
		}
		diffH = oldValue - newValue;
	}
	if(dv) {
		oldValue = GetCtlValue(doc->vScroll);
		newValue = oldValue + dv;
	
		if(dv<0) {
			minValue = GetCtlMin(doc->vScroll);
			if(newValue < minValue)
				newValue = minValue;
		} else {
			maxValue = GetCtlMax(doc->vScroll);
			if(newValue  > maxValue)
				newValue = maxValue;
		}
		diffV = oldValue - newValue;
	}
		
	ScrollContents(wind,diffH,diffV);
		
	FocusOnWindow(wind);
	SetScrollBarValues(wind);
	
	SetClip(oldClip);
	DisposeRgn(oldClip);
}

void DoButtonScroll(ControlHandle theControl,Point localPt)
{
	short result;
	
	result = TrackControl(theControl,localPt,(ProcPtr)ActionProc);
}

pascal void ActionProc(ControlHandle theControl,short partCode)
{
	short scrollAmount=0;
	
	if(theControl==CurrentScrollDocument->vScroll) 
		scrollAmount = (*CurrentScrollDocument->getVertLineScrollAmount)((WindowPtr)CurrentScrollDocument);
	if(theControl==CurrentScrollDocument->hScroll) 
		scrollAmount = (*CurrentScrollDocument->getHorizLineScrollAmount)((WindowPtr)CurrentScrollDocument);
	
	if(partCode == inUpButton)	
		Scroll(theControl,-scrollAmount);
	if(partCode == inDownButton)	
		Scroll(theControl,scrollAmount);
}

void scrollContents(WindowPtr wind,short dh,short dv) 
{
	RgnHandle updateRgn;
	Rect r;
	
	GetContentRect(wind,&r);
	updateRgn = NewRgn();
	ScrollRect(&r,dh,dv,updateRgn);
	
	((DocumentPeek)wind)->hOffset -= dh;
	((DocumentPeek)wind)->vOffset -= dv;
	
	InvalRgn(updateRgn);
	
	DoTheUpdate(wind,(EventRecord *)nil);
	
	DisposeRgn(updateRgn);
}

short getVertPageScrollAmount(WindowPtr wind)
{
	Rect r;
	GetContentRect(wind,&r);
	return r.bottom - r.top - 16;
}

short getHorizPageScrollAmount(WindowPtr wind)
{
	Rect r;
	GetContentRect(wind,&r);
	return r.right - r.left - 16;
}
	
short WantToSave(WindowPtr wind)
{
	Str255 title;
	Str255 nullStr;
	
	*nullStr=0;
	
	if(((DocumentPeek)wind)->fDocWindow) {
		GetWTitle(wind,title);
		ParamText(title,nullStr,nullStr,nullStr);
	} else
		ParamText(nullStr,nullStr,nullStr,nullStr);
	
	return Alert(500,(ModalFilterProcPtr)nil);
}

Boolean InitScrollDoc(WindowPtr wind)
{
	if(((DocumentPeek)wind)->fDocWindow) {
		SetPort(wind);
		((DocumentPeek)wind)->hScroll = GetNewControl(128,wind);
		((DocumentPeek)wind)->vScroll = GetNewControl(129,wind);
		SizeScrollBars(wind);
		AdjustScrollBars(wind);
		SetScrollBarValues(wind);
	}
	return(((DocumentPeek)wind)->hScroll && ((DocumentPeek)wind)->vScroll);
}

#if 0
void SelectWindowFromFile(short refnum)
{
	int i;
	
	for(i=0;i<MAXWINDS;i++) {
		if(Documents[i].dataPathRefNum == refnum) {
			if(Documents[i].fFileOpen && Documents[i].fDocWindow)
				SelectWindow((WindowPtr)(Documents+i));
			else
				doMessage(2);
			return;
		}
	}
	doMessage(3);
}
#endif

void DetachDocResources(DocumentPeek doc)
{
	doc->windowState=0;
}

Boolean openDocFile(WindowPtr wind)
{
	DocumentPeek doc;
	short refnum, resRefnum;
	OSErr err;
	FSSpec specs;
	long position;
	
	doc=(DocumentPeek)wind;
	specs=doc->fileSpecs;
	err = HOpen(specs.vRefNum,specs.parID,specs.name,fsCurPerm,&refnum); 
/*	err = HOpen(specs.vRefNum,specs.parID,specs.name,fsRdWrPerm,&refnum); */
	switch(err) {
		case fnfErr:	/* file not found, create it */
			err = HCreate(specs.vRefNum,specs.parID,specs.name,'MRPH',doc->fDocType);
			if(err==noErr) {
				err = HOpen(specs.vRefNum,specs.parID,specs.name,fsCurPerm,&refnum);
				if(err!=noErr)
					return 0;
			} else
				return 0;
				
			/* if open was successful, fall through to next case. */
			
		case noErr:
			doc->fFileOpen = true;
			doc->dataPathRefNum = refnum;
			break;
		
		case opWrErr:
			/* 
				The file cannot be written into. It is already open in this
				or another application or it is on a locked volume.
			*/
			doMessage(3);
			doc->fFileOpen = true;
			doc->fReadOnly = true;
			doc->dataPathRefNum = refnum;
			break;
		case nsvErr:
			/*	noSuchVolume error
				This error has occured in opening files from the finder in
				System 6, using multifinder. 
			*/
			doMessage(16);
			return 0;
		default:
			doDiagnosticMessage(13,err);	/* Unknown error */
			return 0;
	}
	/* Successfull open. Now open any resource fork; */
	
	err = GetEOF(refnum,&position);
	if(err==noErr) {
		err = SetEOF(refnum,position);
		switch(err) {
			case noErr:	
				break;
			case dskFulErr:	/* Disk is full */
			case extFSErr:	/* File part of external file system */
			case fnOpnErr:	/* File not open */
			case ioErr:		/* I/O Error (bummers) */
			case rfNumErr:	/* Bad reference number */
				break;
			case fLckdErr:	/* File is locked */
			case vLckdErr:	/* Software volume lock */
			case wPrErr:	/* Diskette is locked */
			case wrPermErr:	/* Write permission denied */
				doc->fReadOnly = true;
				break;
			default:
				break;
		}
		
	}
	
	resRefnum=HOpenResFile(specs.vRefNum,specs.parID,specs.name,fsCurPerm);
/*	resRefnum=HOpenResFile(specs.vRefNum,specs.parID,specs.name,fsRdWrPerm); */
	
	if(resRefnum == -1) {
		/* No resource fork for this file. */
		doc->resourcePathRefNum=0;
		if(!doc->fReadOnly) {
			refnum=0;
			err=ResError();
	/*=============
			switch (err) {
			
				default:
					break;
			}
			===============*/
		
			HCreateResFile(specs.vRefNum,specs.parID,specs.name);
			err=ResError();
			if(err!=noErr) {
				doMessage(17);
			} else {
				refnum=HOpenResFile(specs.vRefNum,specs.parID,specs.name,fsCurPerm);
				doc->resourcePathRefNum=refnum;
				if(refnum==-1) {
					doc->resourcePathRefNum=refnum=0;
					doMessage(17);
					/* Can't create it. Forget this. */
				}
			}
		}
	} else	
		doc->resourcePathRefNum=resRefnum;
		
	return refnum;
}

void MoveDocWindow(WindowPtr wind,short h,short v)
{
	if(((DocumentPeek)wind)->fDocWindow)
		MoveWindow(wind,h,v,true);
}

void SetModDate(FSSpec *specs, unsigned long modDate)
{
	OSErr err;
	HFileParam pB;
	
	if(modDate) {
		memset(&pB,0,sizeof(HFileParam));
		pB.ioCompletion=0;
		pB.ioNamePtr=specs->name;
		pB.ioVRefNum=specs->vRefNum;
		pB.ioFDirIndex=0;
		pB.ioDirID=specs->parID;
		
		err=PBHGetFInfoSync((HParmBlkPtr)&pB);
		
		pB.ioFlMdDat=modDate;
		pB.ioDirID=specs->parID;

		err=PBHSetFInfoSync((HParmBlkPtr)&pB);
	}
}

void CloseDocFile(DocumentPeek doc)
{
	OSErr err;
	unsigned long modDate=0;
	
	if(doc->windowState) 
		modDate = ((MPSRPtr)*doc->windowState)->modifiedDate;
		
	err = FSClose(doc->dataPathRefNum);
	doc->dataPathRefNum=0;
	if(doc->resourcePathRefNum) {
		CloseResFile(doc->resourcePathRefNum);
		doc->marks=0;
		doc->windowState=0;
		
	}
	doc->resourcePathRefNum=0;
	
	if(modDate) 
		SetModDate(&doc->fileSpecs,modDate);
}

void DeleteDoc(DocumentPeek doc)
{
	(*doc->destructor)((WindowPtr)doc);
}

void SetDocWindowTitle(WindowPtr wind,StringPtr title)
{
	if(((DocumentPeek)wind)->fDocWindow) 
		SetWTitle(wind,title);
}

void GetDocWindowTitle(WindowPtr wind,StringPtr title)
{
	if(((DocumentPeek)wind)->fDocWindow) 
		GetWTitle(wind,title);
}

void ShowDocWindow(WindowPtr wind)
{
	if(((DocumentPeek)wind)->fDocWindow)
		ShowWindow(wind);
}

int DoSaveAs(WindowPtr wind)
{
	DocumentPeek doc;
	StandardFileReply reply;
	long response=0;
	unsigned long modDate=0;
	short resourcePathRefNum=0;
	short dataPathRefNum=0;
	OSErr err;
	Boolean fFileOpen;
	FSSpec existingFileInfo;
	Str255 buf;
		
	doc = (DocumentPeek)wind;
	GetIndString(buf,131,4);
	if(Gestalt(gestaltStandardFileAttr,&response)==noErr)
		StandardPutFile(buf,doc->fileSpecs.name,&reply);
	else
		glueStandardPutFile(buf,doc->fileSpecs.name,&reply);
	
	if(!reply.sfGood)				/* the user canceled the SaveAs */
		return false;


	if(fFileOpen=doc->fFileOpen) {
		existingFileInfo=doc->fileSpecs;
		dataPathRefNum=doc->dataPathRefNum;
		resourcePathRefNum=doc->resourcePathRefNum;
		if(doc->windowState)
			modDate = ((MPSRPtr)*doc->windowState)->modifiedDate;
	}

	doc->fileSpecs=reply.sfFile;

	if(!OpenDocFile(wind))	{		/* file didn't open */
		doc->fileSpecs=existingFileInfo;
		return false;
	}
	
	if(resourcePathRefNum && doc->resourcePathRefNum) {
		UseResFile(doc->resourcePathRefNum);
		if(doc->windowState) {
			DetachResource(doc->windowState);
			AddResource(doc->windowState,'MPSR',1005,"");
			WriteResource(doc->windowState);
		}
		if(doc->marks) {
			DetachResource(doc->marks);
			AddResource(doc->marks,'MPSR',1007,"");
			WriteResource(doc->marks);
		}
	
	}
	
	if(!WriteDocFile(wind)) {		/* write was unsuccessful */
		doc->fileSpecs=existingFileInfo;
		doc->fFileOpen=fFileOpen;
		doc->resourcePathRefNum=resourcePathRefNum;
		doc->dataPathRefNum=dataPathRefNum;
		return false;
	}
	
	doc->fNeedtoSaveAs=false;
	doc->fNeedtoSave=false;
	SetDocWindowTitle(wind,reply.sfFile.name);
	
	if(dataPathRefNum) {
		err = FSClose(dataPathRefNum);
		if(resourcePathRefNum)
			CloseResFile(resourcePathRefNum);
		if(modDate) 
			SetModDate(&existingFileInfo,modDate);
	}

	return true;
}

int DoSaveACopy(WindowPtr wind)
{
	DocumentPeek doc;
	StandardFileReply reply;
	FSSpec existingFileInfo;
	long response=0;
	short resourcePathRefNum=0;
	short dataPathRefNum=0;
	Boolean fReadOnly=0;
	OSErr err;
	int ret=false;
	Handle res,windowState,marks;
	
	EventMethod existingDoKeyDown;
	IntMethod existingWriteDocFile;
	WindowMethod existingAdjustDocMenus;

	Str255 buf;
	
	doc = (DocumentPeek)wind;
	GetIndString(buf,131,5);
	if(Gestalt(gestaltStandardFileAttr,&response)==noErr)
		StandardPutFile(buf,doc->fileSpecs.name,&reply);
	else
		glueStandardPutFile(buf,doc->fileSpecs.name,&reply);
	
	if(!reply.sfGood) 
		return false;
		
	existingFileInfo=doc->fileSpecs;
	dataPathRefNum=doc->dataPathRefNum;
	resourcePathRefNum=doc->resourcePathRefNum;
	fReadOnly=doc->fReadOnly;
	marks=doc->marks;
	windowState=doc->windowState;
	
	existingDoKeyDown=doc->doKeyDown;
	existingWriteDocFile=doc->writeDocFile;
	existingAdjustDocMenus=doc->adjustDocMenus;

	doc->resourcePathRefNum=0;
	doc->dataPathRefNum=0;
	doc->fReadOnly=0;

	/* 	doc->marks has been removed as a resource. We need a copy of its
		data for the new file. The copy will become a resource and be
		purged from memory. The orginal (**marks) is still good. */
			
	doc->fileSpecs=reply.sfFile;

	if(OpenDocFile(wind)) {	
		if(resourcePathRefNum && doc->resourcePathRefNum) {	/* do both the original and new file have a resource fork? */
			/*
				The following is not satisfactory. We want the saved file to have the marks and
				windowState as it stands now and we want to preserve it for the FrontWindow as
				well. The technique was to save them first.
			*/
#if 0
			if(doc->marks)
				ChangedResource(doc->marks);
			if(doc->windowState)
				ChangedResource(doc->windowState);
			UpdateResFile(resourcePathRefNum);
#endif
			UseResFile(doc->resourcePathRefNum);
			if(res=windowState) {
				err=HandToHand(&res);
				if(err==noErr) {
					doc->windowState=res;
					AddResource(res,'MPSR',1005,"");
				} else
					doc->windowState=0;
			}
			if(res=marks) {
				err=HandToHand(&res);
				if(err==noErr) {
					doc->marks=res;
					AddResource(doc->marks,'MPSR',1007,"");
				} else
					doc->marks=0;
			}
		}
		ret=WriteDocFile(wind);
		CloseDocFile(doc);
	}
			
	doc->fileSpecs=existingFileInfo;
	doc->dataPathRefNum=dataPathRefNum;
	doc->marks=marks;
	doc->windowState=windowState;
	doc->resourcePathRefNum=resourcePathRefNum;
	doc->fReadOnly=fReadOnly; 
	
	doc->doKeyDown=existingDoKeyDown;
	doc->writeDocFile=existingWriteDocFile;
	doc->adjustDocMenus=existingAdjustDocMenus;

#if 0
	if(doc->resourcePathRefNum=resourcePathRefNum) {
		UseResFile(resourcePathRefNum);
		doc->windowState=Get1Resource('MPSR',1005);
		doc->marks=Get1Resource('MPSR',1007);
	}
#endif
	return ret;
}

int DoSave(WindowPtr wind)
{	
	if(((DocumentPeek)wind)->fNeedtoSaveAs) 
		return DoSaveAs(wind);
		
	if(WriteDocFile(wind)) {
		((DocumentPeek)wind)->fNeedtoSave=false;
		return true;
	} else
		return false;
}

int DoClose(WindowPtr wind)
{
	short saveit;
	
	if(((DocumentPeek)wind)->fNeedtoSave) {
		saveit = (wind==gWorkSheetDoc)? iYes : WantToSave(wind);
		if(saveit == iCancel)
			return false;
		if(saveit == iYes) {
			if(!DoSave(wind))
				return false;
		}
	} else 
		(*((DocumentPeek)wind)->writeDocResourceFork)(wind);
	/*	close the file	*/
	if(((DocumentPeek)wind)->fFileOpen) {
		CloseDocFile((DocumentPeek)wind);
		((DocumentPeek)wind)->fFileOpen=0;
	}
	return true;
}

int DoRevert(WindowPtr wind)
{
	DocumentPeek doc;
	Rect box;
	Handle mH;
	OSErr err;
	
	doc = (DocumentPeek)wind;
	if(ConfirmDialog(6,doc->fileSpecs.name)) {
		if(doc->marks) {
			DetachResource(doc->marks);
			DisposeHandle(doc->marks);
			doc->marks=0;
		}
		if(doc->windowState) {
			DetachResource(doc->windowState);
			DisposeHandle(doc->windowState);
			doc->windowState=0;
		}
		if(ReadDocFile(wind)) {
			SetPort(wind);
			GetContentRect(wind,&box);
			InvalRect(&box);
			EraseRect(&box);
			if(doc->marks)
				FillMarkMenu(doc);
			else {
				mH=GetResource('MENU',MARK_ID);
				err=HandToHand(&mH);
				if(err==noErr)
					doc->markMenu=(MenuHandle)mH;
				else
					doc->markMenu=0;
			}
			doc->fNeedtoSave=false;
			return true;
		}
	} 
	return false;
}

void ClosePane(WindowPtr wind)
{
	WindowPtr doc;
	
	/* First remove this window from pane linked list. */
	doc=((DocumentPeek)wind)->homeWindow;
	while(doc && wind!=((DocumentPeek)doc)->associatedWindow)
		doc=((DocumentPeek)doc)->associatedWindow;
	if(!doc) 
		FatalError();

	((DocumentPeek)doc)->associatedWindow=((DocumentPeek)wind)->associatedWindow;
	CloseWindow(wind);
	((DocumentPeek)wind)->docData=NIL;
	((DocumentPeek)wind)->fDocWindow=false;
	return;
}

/*----------------------------- Null Methods --------------------------------*/


void nullWindowMethod(WindowPtr x) { 
#ifdef MC68000
#pragma unused (x)
#endif
	return;
}
void nullEventMethod(WindowPtr x,EventRecord *e) {
#ifdef MC68000
#pragma unused (x,e)
#endif
	return;
}
void nullPointMethod(WindowPtr x,Point pt) {
#ifdef MC68000
#pragma unused (x,pt)
#endif
	return;
}
short getLineScrollAmount(WindowPtr x) {
#ifdef MC68000
#pragma unused (x)
#endif
	return 16;
}
int nullIntMethod(WindowPtr x) {
#ifdef MC68000
#pragma unused (x)
#endif
	return 0;
}
short nullShortMethod(WindowPtr x) {
#ifdef MC68000
#pragma unused (x)
#endif
	return 0;
}
Boolean initDoc(WindowPtr x) {
#ifdef MC68000
#pragma unused (x)
#endif
	return true;
}

void destructor(WindowPtr wind)
{
	Handle mH;
	
	if(((DocumentPeek)wind)->fDocWindow) {
		CloseWindow(wind);
		NumWindows--;
	}
	
	mH=GetResource('MENU',MARK_ID);
	if(mH) {
		MarkMenu=(MenuHandle)mH;
		DeleteMenu(MARK_ID);
		InsertMenu(MarkMenu,WINDOW_ID);
	}
	
	if(((DocumentPeek)wind)->marks)
		DisposeHandle(((DocumentPeek)wind)->marks);
	if(((DocumentPeek)wind)->markMenu)
		DisposeHandle((Handle)((DocumentPeek)wind)->markMenu);
	if(((DocumentPeek)wind)->windowState)
		DisposeHandle(((DocumentPeek)wind)->windowState);
	if(((DocumentPeek)wind)->docData)
		DisposeHandle(((DocumentPeek)wind)->docData);
	((DocumentPeek)wind)->fDocWindow=0;
	((DocumentPeek)wind)->markMenu=0;
	((DocumentPeek)wind)->windowState=0;
	((DocumentPeek)wind)->docData=0;

}

/*-------------------------------- MakeDoc ----------------------------------
	MakeDoc finds the preallocated space for the document record, clears it,
	and initilizes it with (1) the member functions and (2) filetype and 
	FSSpec information. 
----------------------------------------------------------------------------*/

DocumentPeek MakeDoc(OSType fType, FSSpecPtr fileSpecs)
{
	int i;
	DocumentPeek doc;
	Handle mH;
	OSErr err;

	for(i=0;i<MAXWINDS;i++) {
		if(Documents[i].fDocWindow==NIL) {
			doc=&Documents[i];
			memset(doc,0,sizeof(DocumentRecord));
			doc->fDocType=fType;
			if(fileSpecs)
				doc->fileSpecs=*fileSpecs;
			mH=GetResource('MENU',MARK_ID);
			err=HandToHand(&mH);
			doc->markMenu=(MenuHandle)mH;
			AddMemberFunctions(doc);
			switch (fType) {
				case 'TEXT':	/* also New */
					AddTextMemberFunctions(doc);
					break;
				case 'CMLB':
					AddMapMemberFunctions(doc);
					break;
				case 'rose':
					AddRoseMemberFunctions(doc);
					break;
				case 'dlog':
				case 'entr':
				/*=====================================
					The window resource gets loaded from the MRPd resource.
					entrMakeWindow should take care of this
				======================================*/
					setDocName(doc,"\pData Page");
					doc->windResource = 139;
					AddEntryMemberFunctions(doc);
					break;
				case 'titl':
					setDocName(doc,"\pTitle Page");
					doc->windResource = 140;
					AddTitleMemberFunctions(doc);
					break;
				case 'pane':
					doc->windResource = 1128;
					AddPaneMemberFunctions(doc);
					break;
				default:
					break;
			}
			return doc;
		}
	}
	InitCursor();
	doMessage(4);	
	return 0;

}

Boolean makeWindow(WindowPtr wind)
{
	Rect r;
	short windowID;
	unsigned char *title;
	
	windowID=((DocumentPeek)wind)->windResource;
	
	SetRect(&r,DragBoundsRect.left+5+25*NumWindows,
						DragBoundsRect.top+25+25*NumWindows,
						DragBoundsRect.right-40,
						DragBoundsRect.bottom);
	if ((r.right-r.left)>(1024+15)) r.right=r.left+1024+15;
	if ((r.bottom-r.top)>(1024+15)) r.bottom=r.top+1024+15;
	if(!((r.right-r.left)%2)) r.right--;
	if(!((r.bottom-r.top)%2)) r.bottom--;
	if(windowID) 
		wind=(gHasColorQD)?  GetNewCWindow(windowID,(void *)wind,(WindowPtr)-1L) :
								GetNewWindow(windowID,(void *)wind,(WindowPtr)-1L) ;
	else
		wind=(gHasColorQD)?  NewCWindow((void *)wind,&r,(Str255)"",FALSE,zoomDocProc,(WindowPtr)-1L,TRUE,0) :
								NewWindow((void *)wind,&r,(Str255)"",FALSE,zoomDocProc,(WindowPtr)-1L,TRUE,0);
	if(!wind)
		return 0;
	((DocumentPeek)wind)->fDocWindow=true;	
	NumWindows++;
	if(title=((DocumentPeek)wind)->fileSpecs.name)
		SetDocWindowTitle(wind,title);	
	return true;

}

void adjustDocMenus(WindowPtr wind)
{
	DocumentPeek doc;
	
	doc=(DocumentPeek)wind;
	/* File Menu */
	EnableItem(FileMenu,iClose);
	SetMenuAbility(FileMenu,iSave,doc->fNeedtoSave);
	EnableItem(FileMenu,iSaveAs);
	EnableItem(FileMenu,iSaveACopy);
	SetMenuAbility(FileMenu,iRevert,doc->fNeedtoSave);
	EnableItem(FileMenu,iPageSetup);
	EnableItem(FileMenu,iPrint);
	
	/* Edit Menu */
	SetMenuAbility(EditMenu,iUndo,doc->fNeedtoSave);
	
	SetMenuAbility(EditMenu,iCut,doc->fHaveSelection);
	SetMenuAbility(EditMenu,iCopy,doc->fHaveSelection);
	EnableItem(EditMenu,iPaste);
	SetMenuAbility(EditMenu,iClear,doc->fHaveSelection);

	/* Data Menu */
	/* items 1-5 always enabled */
	DisableItem(DataMenu,iNextPage);
	DisableItem(DataMenu,iPreviousPage);
	DisableItem(DataMenu,iFirstPage);
	DisableItem(DataMenu,iLastPage);

	EnableItem(FileMenu,0);
	EnableItem(EditMenu,0);
	EnableItem(FindMenu,0);
	EnableItem(MarkMenu,0);
	EnableItem(WindowMenu,0);
	DisableItem(ViewMenu,0);
	DisableItem(ReportsMenu,0);
	EnableItem(DataMenu,0);
}

void doPageSetup(WindowPtr wind)
{
	PrOpen();
	PrStlDialog(((DocumentPeek)wind)->fPrintRecord);
	PrClose();
}

Boolean doDocMenuCommand(WindowPtr w, short x, short y, short z)
{	
#ifdef MC68000
#pragma unused (w,x,y,z)
#endif
	return false; 
}

/*  
	This is a menu function. Each Doc supplies its own method for this
	in its doDocMenuCommand method.
*/

void AddMemberFunctions(DocumentPeek doc)
{
	doc->makeWindow=makeWindow;
	doc->initDoc=initDoc;
	doc->openDocFile=openDocFile;
	doc->destructor=destructor;
	/* Event actions */
	doc->doTheUpdate=doTheUpdate;
	doc->draw=nullDraw;
	doc->doActivate=doActivate;
	doc->activate=activate;
	doc->deactivate=deactivate;
	doc->doContent=doContent;
	doc->doKeyDown=nullEventMethod;
	doc->doIdle=nullWindowMethod;
	doc->adjustCursor=nullPointMethod;
	doc->doGrow=(EventMethod)doGrow;
	doc->doZoom=doZoom;
	
	/* Edit menu and clipboard functions */
	doc->doDocMenuCommand=doDocMenuCommand;
	doc->adjustDocMenus=adjustDocMenus;
	doc->doCut=nullWindowMethod;
	doc->doCopy=nullWindowMethod;
	doc->doPaste=nullWindowMethod;
	doc->doClear=nullWindowMethod;
	doc->doSelectAll=nullWindowMethod;
	doc->doUndo=nullWindowMethod;
	
	doc->readDocFile=nullIntMethod;
	doc->writeDocFile=nullIntMethod;
	doc->writeDocResourceFork=nullIntMethod;
	
	doc->fDocType='\?\?\?\?';

	doc->doPageSetup=nullWindowMethod;
	doc->doPrint=nullWindowMethod;
	
	/* Scrolling methods */
	doc->adjustScrollBars=adjustScrollBars;
	doc->focusOnContent=focusOnContent;
	doc->focusOnWindow=focusOnWindow;
	doc->getVertSize=nullShortMethod;
	doc->getHorizSize=nullShortMethod;
	doc->getVertLineScrollAmount=getLineScrollAmount;
	doc->getHorizLineScrollAmount=getLineScrollAmount;
	doc->getVertPageScrollAmount=getVertPageScrollAmount;
	doc->getHorizPageScrollAmount=getHorizPageScrollAmount;
	
	doc->displaySelection=nullWindowMethod;
	
	doc->scrollContents=scrollContents;
	doc->setScrollBarValues=setScrollBarValues;
	doc->getContentRect=getContentRect;
}

void scrollDoGrow(WindowPtr wind,EventRecord *event)
{
	FocusOnWindow(wind);
	if(doGrow(wind,event)) {
		SizeScrollBars(wind);
		AdjustScrollBars(wind);
		SetScrollBarValues(wind);
	}
}

	
/*-------------------------- Map Object ----------------------------*/

#ifdef MC68000
#pragma segment mapObject
#endif

short mapGetHorizSize(WindowPtr wind)
{
	Rect r;
	short gain;
	if(((DocumentPeek)wind)->docData) {
		r = (**(FileStuff **)((DocumentPeek)wind)->docData).bounds;
		gain = (**(FileStuff **)((DocumentPeek)wind)->docData).gain;
		if(gain>0)
			return (r.right - r.left<<gain) + 4;
		else if(gain<0)
			return (r.right - r.left >>-gain) + 4;
		else
			return r.right - r.left + 4;
	}
	return 0;
}

short mapGetVertSize(WindowPtr wind)
{
	Rect r;
	short gain;
	if(((DocumentPeek)wind)->docData) {
		r = (**(FileStuff **)((DocumentPeek)wind)->docData).bounds;
		gain = (**(FileStuff **)((DocumentPeek)wind)->docData).gain;
		if(gain>0)
			return (r.bottom - r.top<<gain) + 4;
		else if(gain<0)
			return (r.bottom - r.top>>-gain) + 4;
		else
			return r.bottom - r.top + 4;
	}
	return 0;
}

void mapSetScrollBarValues(WindowPtr wind)
{
	FileStuff *fStuff;
	Point center; 
	Rect r;
	short hPos,vPos;
	
	HLock(((DocumentPeek)wind)->docData);
	fStuff = (FileStuff *)*((DocumentPeek)wind)->docData;
	r = fStuff->bounds;
	center.h = fStuff->XMapCtr-r.left;
	center.v = fStuff->YMapCtr-r.top;
	PtTimesGain(&center,fStuff->gain);
	
	hPos = center.h - fStuff->xBMCtr + fStuff->DhBM;
	vPos = center.v - fStuff->yBMCtr + fStuff->DvBM;
	
	HUnlock(((DocumentPeek)wind)->docData);
	
	FocusOnWindow(wind);
	SetCtlValue(((DocumentPeek)wind)->hScroll,hPos);
	SetCtlValue(((DocumentPeek)wind)->vScroll,vPos);
}

void mapScrollContents(WindowPtr wind, short dh, short dv)
{
	FileStuff **map;
	short hBM,vBM;
	RgnHandle updateRgn;
	Rect r;
	
	if(map = (FileStuff **)((DocumentPeek)wind)->docData) {
		(**map).DhBM -= dh;
		(**map).DvBM -= dv;
		hBM=(**map).DhBM;
		vBM=(**map).DvBM;
		
		GetContentRect(wind,&r);
	
		if(hBM<0 || vBM<0) {
			FillOffscreen((DocumentPeek)wind,0);
			EraseRect(&r);
			InvalRect(&r);
			return;
		}
		if((hBM+r.right-r.left)>OffScreenH) {
			FillOffscreen((DocumentPeek)wind,0);
			EraseRect(&r);
			InvalRect(&r);
			return;
		}
		if((vBM+r.bottom-r.top)>OffScreenV) {
			FillOffscreen((DocumentPeek)wind,0);
			EraseRect(&r);
			InvalRect(&r);
			return;
		}
		
		updateRgn = NewRgn();
		ScrollRect(&r,dh,dv,updateRgn);
		InvalRgn(updateRgn);
		DoTheUpdate(wind,(EventRecord *)nil);
		DisposeRgn(updateRgn);
	}
}

void mapDeactivate(WindowPtr wind)
{
	deactivate(wind);
	if(((DocumentPeek)wind)->associatedWindow) {
		HideWindow(((DocumentPeek)wind)->associatedWindow);
		HiliteWindow(wind,false);
		DrawGrowIcon(wind);
	}
	SurveySelected=0;
	SelectedDesig[0]=0;
}

void mapActivate(WindowPtr wind)
{
	FileStuff *fStuff;
	WindowPtr pane;
	
	activate(wind);
	if(((DocumentPeek)wind)->docData) {
	
		HLock(((DocumentPeek)wind)->docData);
		fStuff=(FileStuff *)*((DocumentPeek)wind)->docData;
		if(SurveySelected=fStuff->selected.survey) {
			SelectedFSB=fStuff->selectedFSB;
			desigCpy(SelectedDesig,fStuff->selectedDesig);
		} else {
			SelectedFSB=0;
			SelectedDesig[0]=0;
		}
		HUnlock(((DocumentPeek)wind)->docData);
		if(pane=((DocumentPeek)wind)->associatedWindow) {
			ShowHide(pane,true);		/* make the pane visible w/o generating any activate events, or changing any window's highlighing */
			HiliteWindow(pane,true);	/* set its hilighting, w/o changing anything else */
			BringToFront(pane);			/* again, w/o changing anything or generating any activate events */
		}
	}
}

void mapDestructor(WindowPtr wind)
{	
	FileStuff *fStuff;
	DocumentPeek doc,pane;

	if(((DocumentPeek)wind)->docData) {
		doc=(DocumentPeek)wind;
		while(pane=(DocumentPeek)doc->associatedWindow) {
			(*pane->destructor)((WindowPtr)pane);
			/* 	The called destructor manipulates doc->associatedWindow so that
				this loop will terminate when all the panes are
				destroyed */
		}
		HLock(((DocumentPeek)wind)->docData);
		fStuff=(FileStuff *)*((DocumentPeek)wind)->docData;
		if(gHasColorQD)
			DisposeGWorld(fStuff->offscreen);
		else {
			ClosePort((GrafPtr)fStuff->offscreen);
			DisposeHandle(fStuff->bitHandle);
		}
		DisposeLists(fStuff);
		DisposeHandle(fStuff->fieldList);
		DisposeHandle(fStuff->intData);
		DisposeHandle(fStuff->boundsData);
		HUnlock(doc->docData);
		DisposeHandle(doc->docData);
		doc->docData=0;
	}
	destructor(wind);
}

Boolean mapInitDoc(WindowPtr wind)
{
	FileStuff *fStuff;
	DocumentPeek doc;
		
	if(!InitScrollDoc(wind))
		return false;
		
	doc=(DocumentPeek) wind;
	
	SetRect(&doc->limitRect,240,80,DragBoundsRect.right-DragBoundsRect.left,
				DragBoundsRect.bottom-DragBoundsRect.top);
				
	if(!(doc->fPrintRecord = (THPrint)NewHandle(sizeof(TPrint))))
		return false;
	PrOpen();
	PrintDefault(doc->fPrintRecord);
	PrClose();

	if(!(doc->docData=NewHandle(sizeof(FileStuff))))
		return false;
		
	HLock(doc->docData);
	fStuff=(FileStuff *)*doc->docData;

	memset(fStuff,0,sizeof(FileStuff));

	fStuff->doc=doc;
	
	fStuff->offscreen=prepareGWorld(fStuff->offscreen,OffScreenH,OffScreenV,0,&fStuff->bitHandle);
	if(!fStuff->offscreen) {
		DisposeWindow(wind);
		HUnlock(doc->docData);
		DisposeHandle(doc->docData);
		doc->fDocWindow=false;
		HideACurs();
		InitCursor();
		doMessage(5);	
		return false;
	}
	HUnlock(doc->docData);
	
	DrawControls(wind);
	DrawGrowIcon(wind);
	return true;
}

int mapReadDocFile(WindowPtr wind)
{
	DocumentPeek doc;
	short refNum;
	long len;
	int err = 0;
	FileStuff *fStuff;
	
	int i,cnt,nLE;
	int nLocns=0,nData=0;
	ListElement *tempListElements,*newSpace;
	int totalListElements;
	long pos;
	LongList *myLists;
	Rect mapFrame;
	
	doc = (DocumentPeek)wind;

	if((doc->fDocWindow)&&(doc->docData)) {
		refNum=doc->dataPathRefNum;
		HLock(doc->docData);
		fStuff = (FileStuff *)*doc->docData;
		err = 1;
		
		/*	Read the snapshot file:
			1. Read the position of beginning of the header data
				placed by saveSnapShot. */
			
		cvTempRead(refNum,&pos,-1L,sizeof(long));

		/*	The header data consists of (a) the command line to CML/MDL. */
	
		cvTempRead(refNum,&fStuff->argv,pos,sizeof(int));
		for(cnt=i=0;i<fStuff->argv;i++) {
			cvTempRead(refNum,&len,-1L,sizeof(int));
			if((cnt+len)<254) {
				cvTempRead(refNum,fStuff->params+cnt,-1L,len*sizeof(char));
				cnt+=len;
				*(fStuff->params+cnt++)=0;
			} else
				cnt=254;	/* Make sure no more stuffed into fStuff->params */
		}
	
		/*	(b) the version string */
		
		cvTempRead(refNum,fStuff->version,-1L,sizeof(Str31));

		/*	(c) the limits of and the FieldList. */
		
		cvTempRead(refNum,&fStuff->minField,-1L,sizeof(int));
		cvTempRead(refNum,&fStuff->maxField,-1L,sizeof(int));
		fStuff->fieldList=NewHandle((cnt=fStuff->maxField-fStuff->minField+1)*sizeof(short));
		if(fStuff->fieldList==0)
			errorAlertDialog(OutOfMemory);
		HLock(fStuff->fieldList);
		cvTempRead(refNum,*fStuff->fieldList,-1L,cnt*sizeof(short));
		HUnlock(fStuff->fieldList);
	
		/*	(d)	a flag which is nonzero if there is a NameHashArray. */
		
		cvTempRead(refNum,&cnt,-1L,sizeof(int *));
		if(cnt)
			cvTempRead(refNum,fStuff->nameHashArray,-1L,37*sizeof(int));
	
		/*	(e) the single Literals value. */
		
		cvTempRead(refNum,&fStuff->literals,-1L,sizeof(int));
		
		/*	(f) the MastersArray */
		
		cvTempRead(refNum,&fStuff->nMasters,-1L,sizeof(int));
		cvTempRead(refNum,fStuff->masters,-1L,fStuff->nMasters*sizeof(MasterRec));
	
		/*	2. Using the MastersArray, we can now retrieve the lLists. */
		
		for(cnt=i=0;i<fStuff->nMasters;i++)
			if(fStuff->masters[i].other)
				cnt++;
		fStuff->nLongLists=cnt;
		fStuff->longLists=NewHandle(cnt*sizeof(LongList));
		if(fStuff->longLists==0)
			errorAlertDialog(OutOfMemory);
		totalListElements=cnt*10;
		tempListElements=(ListElement *)NewPtr(totalListElements*sizeof(ListElement));
		if(tempListElements==0)
			errorAlertDialog(OutOfMemory);
		HLock(fStuff->longLists);
		myLists=(LongList *)*fStuff->longLists;
		for(nLE=cnt=i=0;i<fStuff->nMasters;i++) {
			if(fStuff->masters[i].other) {
				cvTempRead(refNum,myLists+cnt,fStuff->masters[i].other,sizeof(LongList));
				if((nLE+(myLists+cnt)->nLE)>totalListElements) {
					totalListElements+=20;
					newSpace=(ListElement *)NewPtr((totalListElements+20)*sizeof(ListElement));
					if(MemError()!=noErr) {
						errorAlertDialog(OutOfMemory);
						return 1;
					}
					BlockMove((Ptr)tempListElements,(Ptr)newSpace,totalListElements*sizeof(ListElement));
					totalListElements+=20;
					DisposePtr((Ptr)tempListElements);
					tempListElements=newSpace;
				}
				(myLists+cnt)->firstList=nLE;	/* Handle field in CML */
				cvTempRead(refNum,tempListElements+nLE,(myLists+cnt)->other,
					(myLists+cnt)->nLE*sizeof(ListElement));
				nLE+=(myLists+cnt)->nLE;
				/* Keep a count if masters so won't have to lock lists as often. */
				fStuff->masters[i].other=(myLists+cnt)->current;	
				(myLists+cnt)->other=0;
				(myLists+cnt)->master=i;
				if(i==nDataList)
					nData=(myLists+cnt)->current;
				else if(i==nLocnList)
					nLocns=(myLists+cnt)->current;
				cnt++;
			}
		}
		HUnlock(fStuff->longLists);
		/* There are no buffers yet. Erase any residue left in snapshot. */
		for(i=0;i<nLE;i++) {
			(tempListElements+i)->ptr=0;	
			(tempListElements+i)->handle=0;
		}
		PtrAndHand(tempListElements,fStuff->longLists,nLE*sizeof(ListElement));
		DisposePtr((Ptr)tempListElements);
	
		fStuff->intData=NewHandle((nData+nLocns+2)*sizeof(IntData));
		if(fStuff->intData==0) {
			errorAlertDialog(OutOfMemory);
			err = 0;
		}
			
		LockLists(fStuff);
		TransformCoorinates(fStuff);
		FillBounds(fStuff);
		fStuff->gain+=SizeToFit(fStuff,fStuff->bounds);
		/*	With the gain in place, we can set the BM center to its
			proper value. It is not 512. */

		mapFrame=fStuff->bounds;
		RectMapToBM(&mapFrame,fStuff);
		fStuff->xBMCtr=(mapFrame.right-mapFrame.left)/2;
		fStuff->yBMCtr=(mapFrame.bottom-mapFrame.top)/2;
		
		FillOffscreen(doc,0);
		UnlockLists(fStuff);
		
		HUnlock(doc->docData);
		
		ShowDocWindow(wind);		/* Necessary here so pane will come to front */
		MakeOnePane();
	}
	return err;
}

void mapDraw(WindowPtr wind, Rect *r,short x)
{
#ifdef MC68000
#pragma unused (r,x)
#endif
	FileStuff *fStuff;
	PixMapHandle pixBase;
	Rect seeRect,tempRect;

	HLock(((DocumentPeek)wind)->docData);
	(*((DocumentPeek)wind)->getContentRect)(wind,&seeRect);
	tempRect=seeRect;
	
	fStuff=(FileStuff *)*((DocumentPeek)wind)->docData;
	
	OffsetRect(&tempRect,fStuff->DhBM,fStuff->DvBM); 

	if(gHasColorQD) {
		pixBase=GetGWorldPixMap(fStuff->offscreen);
		LockPixels(pixBase); 
		HLock((Handle)(((CGrafPtr)wind)->portPixMap));
		CopyBits((BitMap *)*pixBase,
			&wind->portBits,
			&tempRect,&seeRect,srcCopy,NIL);
		SetOrigin(0,0);
		UnlockPixels(pixBase); 
		HUnlock((Handle)(((CGrafPtr)wind)->portPixMap));
	} else {
		HLock(fStuff->bitHandle);
		((GrafPtr)fStuff->offscreen)->portBits.baseAddr = *fStuff->bitHandle;
		CopyBits(&((GrafPtr)fStuff->offscreen)->portBits,
			&wind->portBits,
			&tempRect,&seeRect,srcCopy,NIL);
		SetOrigin(0,0);
		
		HUnlock(fStuff->bitHandle);
		((GrafPtr)fStuff->offscreen)->portBits.baseAddr = 0;
	}

	HUnlock(((DocumentPeek)wind)->docData);
} 

void scrollDoZoom(WindowPtr wind,short partCode)
{
	doZoom(wind,partCode);
	SizeScrollBars(wind);
	AdjustScrollBars(wind);
	SetScrollBarValues(wind);
}

void mapDoContent(WindowPtr wind, EventRecord *event)
{
	Rect contents;
	
	FocusOnWindow(wind);
	GlobalToLocal(&event->where);
	(*((DocumentPeek)wind)->getContentRect)(wind,&contents);
	if(PtInRect(event->where,&contents)) {
		FocusOnContent(wind);
		SelectObject(event->where,wind);
	} else
		ScrollClick(wind,event);
}

void mapDoIdle(WindowPtr wind)
{
	Point locPt;
	
	SetPort(wind);
	GetMouse(&locPt);
	(*((DocumentPeek)wind)->adjustCursor)(wind,locPt);
	showMouseLoc(wind,false);
}

void mapAdjustCursor(WindowPtr wind, Point where)
{
	Rect r;
	CursHandle MapCurs;
	
	GetContentRect(wind,&r);
	if(PtInRect(where,&r)) {
		MapCurs = GetCursor(crossCursor);
		if(MapCurs)
			SetCursor(*MapCurs);
	} else
		InitCursor();
}

void mapAdjustDocMenus(WindowPtr wind)
{
	DocumentPeek doc;
		doc=(DocumentPeek)wind;
	/* File Menu */
	EnableItem(FileMenu,iClose);
	DisableItem(FileMenu,iSave);
	DisableItem(FileMenu,iSaveAs);
	DisableItem(FileMenu,iSaveACopy);
	DisableItem(FileMenu,iRevert);
	EnableItem(FileMenu,iPageSetup);
	EnableItem(FileMenu,iPrint);

	/* Find Menu */
	EnableItem(FindMenu,iSelectSurvey);
	DisableItem(FindMenu,iFind);
	DisableItem(FindMenu,iFindSame);
	DisableItem(FindMenu,iFindSelection);
	SetMenuAbility(FindMenu,iDisplaySelection,doc->fHaveSelection);
	DisableItem(FindMenu,iReplace);
	DisableItem(FindMenu,iReplaceSame);	
	
	/* View Menu */
	/* 	Items initialized as enabled and should never
		be disabled by another document. */

	SetMenuAbility(ReportsMenu,iSchematic,doc->fHaveSelection);
	SetMenuAbility(ReportsMenu,iLog,doc->fHaveSelection);
	EnableItem(ReportsMenu,iRose);
	SetMenuAbility(ReportsMenu,iPosition,!doc->associatedWindow);
	
	/* Data Menu */
	DisableItem(DataMenu,iNextPage);
	DisableItem(DataMenu,iPreviousPage);
	DisableItem(DataMenu,iFirstPage);
	DisableItem(DataMenu,iLastPage);
	
	EnableItem(FileMenu,0);
	DisableItem(EditMenu,0);
	EnableItem(FindMenu,0);
	EnableItem(MarkMenu,0);
	EnableItem(WindowMenu,0);
	EnableItem(ViewMenu,0);
	EnableItem(ReportsMenu,0);
	EnableItem(DataMenu,0);
}

void mapNewMark(DocumentPeek doc)
{
	FileStuff *fStuff;
	long selStart,selEnd;
	char selection[256];
	
	if(doc->docData && doc->fHaveSelection) {
		selEnd=selStart=doc->fHaveSelection;
		HLock(doc->docData);
		fStuff = (FileStuff *)*doc->docData;
		sprintf(selection,"$%d%.4s",fStuff->selectedFSB,fStuff->selectedDesig);
		HUnlock(doc->docData);
		InsertMark(doc,selStart,selEnd,selection);
	}
}

void mapUnMark(DocumentPeek doc)
{	
	long selStart,selEnd;
	
	if(doc->marks && doc->docData) {
		selEnd=selStart=doc->fHaveSelection;
		DoUnmark(doc,selStart,selEnd);
	}
}

Boolean mapDoDocMenuCommand(WindowPtr wind, short menuID, short menuItem, short x)
{	
#ifdef MC68000
#pragma unused (x)
#endif
	if(doDocMenuCommand(wind,menuID,menuItem,0))
		return true;

	switch(menuID) {
		case MARK_ID:
			if(menuItem==iMark) {	
				mapNewMark((DocumentPeek)wind);
			} else if(menuItem==iUnmark) {
				mapUnMark((DocumentPeek)wind);
			} else if(menuItem==iAlphabetical) 
				return false;
/*			else  */
/*				mapSelectMark(wind,menuItem-4); */
			return true;
		case VIEW_ID:
			switch (menuItem) {
				case iEnlarge:
					FillOffscreen((DocumentPeek)FrontLayer(),1);
					SetPort(wind);
					EraseRect(&wind->portRect);
					InvalRect(&wind->portRect);
					return true;
				case iReduce:
					FillOffscreen((DocumentPeek)FrontLayer(),-1);
					SetPort(wind);
					EraseRect(&wind->portRect);
					InvalRect(&wind->portRect);
					return true;
				case iFitToWindow:
#if 0
					if(temp=SizeToFit(fStuff,
						(fStuff->selected.survey)?fStuff->selectedBounds:fStuff->bounds)) {
						FillOffscreen((DocumentPeek)FrontLayer(),temp);
						SetPort(wind);
						EraseRect(&wind->portRect);
						InvalRect(&wind->portRect);
					}
#endif
					break;
				default:
					break;
			}
			break;
		case REPORTS_ID:
			switch (menuItem) {
				case iSchematic:
					if(!wind)
						break;
					OpenTextWindow(wind,1);
					break;
				case iLog:
					OpenTextWindow(wind,2);
					break;
				case iRose:
					OpenRoseWindow(wind);
					break;
				case iPosition:
					if(!((DocumentPeek)wind)->associatedWindow) {
						ShowDocWindow(wind);		/* Necessary here so pane will come to front */
						MakeOnePane();
/*						ShowDocWindow(wind);		 */
						ShowHide(((DocumentPeek)wind)->associatedWindow,true);
						HiliteWindow(((DocumentPeek)wind)->associatedWindow,true);
						BringToFront(((DocumentPeek)wind)->associatedWindow);
					}
					break;
				default:
					break;
				}
		default:
			break;
	}
	return false;
}

void mapDisplaySelection(WindowPtr wind)
{
	FileStuff *fStuff;
	Rect frame,view;
	
	if(((DocumentPeek)wind)->fHaveSelection) {	
		fStuff=(FileStuff *)*((DocumentPeek)wind)->docData;
		if(fStuff->selected.survey) {
			frame=fStuff->selectedBounds;
			(*((DocumentPeek)wind)->getContentRect)(wind,&view);
			RectMapToLoc(&frame,fStuff);
			SetPort(wind);
			CombinedScroll(wind,-(short)((view.right+view.left)/2-(frame.right+frame.left)/2),
				-(short)((view.top+view.bottom)/2-(frame.top+frame.bottom)/2));
		}
	}
	return;
}

void mapDoPrint(WindowPtr wind)
{
	TPPrPort printPort;
	TPrStatus status;
	Rect r;
	
	PrOpen();
	if(PrValidate(((DocumentPeek)wind)->fPrintRecord)) {
		if(!PrStlDialog(((DocumentPeek)wind)->fPrintRecord)) {
			PrClose();
			return;
		} 
	}
	if(!PrJobDialog(((DocumentPeek)wind)->fPrintRecord)) {
		PrClose();
		return;
	}
	printPort = PrOpenDoc(((DocumentPeek)wind)->fPrintRecord,nil,nil);
	PrOpenPage(printPort,nil);
	SetRect(&r,0,0,0,0);
	(*((DocumentPeek)wind)->draw)(wind,&r,1);
	PrClosePage(printPort);
	PrCloseDoc(printPort);
	if((**((DocumentPeek)wind)->fPrintRecord).prJob.bJDocLoop != 0) 
		PrPicFile(((DocumentPeek)wind)->fPrintRecord,nil,nil,nil,&status);
	PrClose();
}

void mapDoKeyDown(WindowPtr wind,EventRecord *event)
{	
	char ch;
	short sequence=0;
	Handle docData;
	FileStuff *fStuff;
	Survey *su;
	Rect r;
	
	ch=event->message&charCodeMask;
	if(((DocumentPeek)wind)->fHaveSelection) {
		docData=((DocumentPeek)wind)->docData;
		HLock(docData);
		fStuff=(FileStuff *)*docData;
		if (ch==LEFTARROW) {
			sequence=fStuff->sequence-1;
			if(!sequence) 
				sequence=-1;
		} else if (ch==RIGHTARROW)
			sequence=fStuff->sequence+1;
		if(sequence) {
			setOffscreenPort(fStuff,1);
			HLock(fStuff->intData);
			LockLists(fStuff);
			su=getSurvey(fStuff->selected.survey);
			HiliteSurvey(su,fStuff,0);
			HiliteSurvey(su,fStuff,sequence);
			UnlockLists(fStuff);
			HUnlock(fStuff->intData);
			setOffscreenPort(fStuff,0);
			SetPort(wind);
			r=fStuff->selectedBounds;
			RectMapToLoc(&r,fStuff);
			InsetRect(&r,-2,-2);
			EraseRect(&r);
			InvalRect(&r);
			showMouseLoc(wind,true);
		}
		HUnlock(docData);
	}
}

static void AddMapMemberFunctions(DocumentPeek doc)
{
	doc->initDoc=mapInitDoc;
	doc->destructor=mapDestructor;
	doc->readDocFile=mapReadDocFile;
	doc->draw=mapDraw;
	doc->doPageSetup=doPageSetup;
	doc->doPrint=mapDoPrint;
	doc->activate = mapActivate;
	doc->deactivate = mapDeactivate;
	doc->doContent=mapDoContent;
	doc->doIdle=mapDoIdle;
	doc->doGrow=scrollDoGrow;
	doc->doZoom=scrollDoZoom;
	doc->adjustCursor=mapAdjustCursor;
	doc->setScrollBarValues=mapSetScrollBarValues;
	doc->getHorizSize=mapGetHorizSize;
	doc->getVertSize=mapGetVertSize;
	doc->scrollContents=mapScrollContents;
	doc->displaySelection=mapDisplaySelection;
	doc->adjustDocMenus=mapAdjustDocMenus;
	doc->doKeyDown=mapDoKeyDown;
	doc->doDocMenuCommand=mapDoDocMenuCommand;
}

void RectLocToMap(Rect *what,FileStuff *fStuff)
{
	short temp;
	
	what->top+=(fStuff->DvBM-fStuff->yBMCtr);	/* LocToBM */
	what->left+=(fStuff->DhBM-fStuff->xBMCtr);	/* LocToBM */
	what->bottom+=(fStuff->DvBM-fStuff->yBMCtr);	/* LocToBM */
	what->right+=(fStuff->DhBM-fStuff->xBMCtr);	/* LocToBM */
	if(fStuff->gain>0) {			/* Multiply by gain to get real-world coordinates */
		what->top>>=fStuff->gain;
		what->left>>=fStuff->gain;
		what->bottom>>=fStuff->gain;
		what->right>>=fStuff->gain;
	} else if(fStuff->gain<0) {
		what->top<<=-fStuff->gain;
		what->left<<=-fStuff->gain;
		what->bottom<<=-fStuff->gain;
		what->right<<=-fStuff->gain;
	}
	temp=-what->top;					/* Vertical is sign-reversed */
	what->top=-what->bottom;
	what->bottom=temp;
	what->top+=fStuff->YMapCtr;			/* Add real values of bit map center. */
	what->left+=fStuff->XMapCtr;
	what->bottom+=fStuff->YMapCtr;
	what->right+=fStuff->XMapCtr;
	return;
}
	
void LocToMap(Point *where,FileStuff *fStuff)
{
	where->v+=(fStuff->DvBM-fStuff->yBMCtr);	/* LocToBM */
	where->h+=(fStuff->DhBM-fStuff->xBMCtr);
	if(fStuff->gain>0) {			/* Multiply by gain to get real-world coordinates */
		where->v>>=fStuff->gain;
		where->h>>=fStuff->gain;
	} else if(fStuff->gain<0) {
		where->v<<=-fStuff->gain;
		where->h<<=-fStuff->gain;
	}
	where->v=-where->v;				/* Vertical is sign-reversed */
	where->h+=fStuff->XMapCtr;		/* Add real values of bit map center. */
	where->v+=fStuff->YMapCtr;
	return;
}

void RectMapToBM(Rect *what,FileStuff *fStuff)
{
	/*	Remember that on the screen and BM, + is down, where as in the Map
		world, + is up. There for Rect top and bottom must be switched. */
		
	short temp;
	
	if(fStuff->gain>=0) {
		temp=fStuff->yBMCtr-((what->top-fStuff->YMapCtr)<<fStuff->gain);
		what->left=fStuff->xBMCtr+((what->left-fStuff->XMapCtr)<<fStuff->gain);
		what->top=fStuff->yBMCtr-((what->bottom-fStuff->YMapCtr)<<fStuff->gain);
		what->right=fStuff->xBMCtr+((what->right-fStuff->XMapCtr)<<fStuff->gain);
	} else {
		temp=fStuff->yBMCtr-((what->top-fStuff->YMapCtr)>>-fStuff->gain);
		what->left=fStuff->xBMCtr+((what->left-fStuff->XMapCtr)>>-fStuff->gain);
		what->top=fStuff->yBMCtr-((what->bottom-fStuff->YMapCtr)>>-fStuff->gain);
		what->right=fStuff->xBMCtr+((what->right-fStuff->XMapCtr)>>-fStuff->gain);
	}
	what->bottom=temp;
	return;
}

void MapToBM(Point *where,FileStuff *fStuff)
{
	if(fStuff->gain>=0) {
		where->v=(fStuff->yBMCtr-((where->v-fStuff->YMapCtr)<<fStuff->gain));
		where->h=fStuff->xBMCtr+((where->h-fStuff->XMapCtr)<<fStuff->gain);
	} else {
		where->v=(fStuff->yBMCtr-((where->v-fStuff->YMapCtr)>>-fStuff->gain));
		where->h=fStuff->xBMCtr+((where->h-fStuff->XMapCtr)>>-fStuff->gain);
	}
	return;
}

void RectMapToLoc(Rect *what,FileStuff *fStuff)
{
	/*	Remember that on the screen and BM, + is down, where as in the Map
		world, + is up. There for Rect top and bottom must be switched. */
		
	short temp;
	
	if(fStuff->gain>=0) {
		temp = 			fStuff->yBMCtr-fStuff->DvBM-((what->top-fStuff->YMapCtr)<<fStuff->gain);
		what->left = 	fStuff->xBMCtr-fStuff->DhBM+((what->left-fStuff->XMapCtr)<<fStuff->gain);
		what->top = 	fStuff->yBMCtr-fStuff->DvBM-((what->bottom-fStuff->YMapCtr)<<fStuff->gain);
		what->right = 	fStuff->xBMCtr-fStuff->DhBM+((what->right-fStuff->XMapCtr)<<fStuff->gain);
	} else {
		temp = 			fStuff->yBMCtr-fStuff->DvBM-((what->top-fStuff->YMapCtr)>>-fStuff->gain);
		what->left = 	fStuff->xBMCtr-fStuff->DhBM+((what->left-fStuff->XMapCtr)>>-fStuff->gain);
		what->top = 	fStuff->yBMCtr-fStuff->DvBM-((what->bottom-fStuff->YMapCtr)>>-fStuff->gain);
		what->right = 	fStuff->xBMCtr-fStuff->DhBM+((what->right-fStuff->XMapCtr)>>-fStuff->gain);
	}
	what->bottom=temp;
	return;
}

void MapToLoc(Point *where,FileStuff *fStuff)
{
	if(fStuff->gain>=0) {
		where->v=-((where->v-fStuff->YMapCtr)<<fStuff->gain);
		where->h=(where->h-fStuff->XMapCtr)<<fStuff->gain;
	} else {
		where->v=-((where->v-fStuff->YMapCtr)>>-fStuff->gain);
		where->h=(where->h-fStuff->XMapCtr)>>-fStuff->gain;
	}
	return;
}

void showMouseLoc(WindowPtr wind, Boolean force)
{
	static short LastV=-1;
	static short LastH=-1;
	Point pt;
	WindowPtr pane;
	char xbuf[32],ybuf[32];
	Handle docData;
	FileStuff *fStuff;
	
	if(pane=((DocumentPeek)wind)->associatedWindow) {
		GetMouse(&pt);
		if(force || LastV != pt.v || LastH != pt.h) {
			LastV = pt.v;
			LastH = pt.h;
			if(((DocumentPeek)wind)->docData) {
				docData=((DocumentPeek)wind)->docData;
				HLock(docData);
				fStuff=(FileStuff *)*docData;
				
				LocToMap(&pt,fStuff);
				sprintf(xbuf,"X: %d",pt.h);
				sprintf(ybuf,"Y: %d",pt.v);
				c2pstr(xbuf);
				c2pstr(ybuf);
				SetPort(pane);
				EraseRect(&pane->portRect);
				MoveTo(10,15);
				DrawString((ConstStr255Param)xbuf);
				MoveTo(10,25);
				DrawString((ConstStr255Param)ybuf);
	
				if(SurveySelected) {
					MoveTo(10,35);
					sprintf(xbuf,"$%d%.4s%d",fStuff->selectedFSB,
							fStuff->selectedDesig,fStuff->selected.n);
					c2pstr(xbuf);
					DrawString((ConstStr255Param)xbuf);
					if(fStuff->selectedTie.survey) {
						MoveTo(10,45);
						sprintf(xbuf," -> $%d%.4s%d",fStuff->tieFSB,
							fStuff->tieDesig,fStuff->selectedTie.n);
						c2pstr(xbuf);
						DrawString((ConstStr255Param)xbuf);
					}
				}
				HUnlock(docData);
				SetPort(wind);
			}
		}
	}
}

BitMap *getOffscreenBits(FileStuff *fStuff)
{
	PixMapHandle pixBase;

	if(gHasColorQD) {
		pixBase=GetGWorldPixMap(fStuff->offscreen);
		LockPixels(pixBase); 
		return (BitMap *)*pixBase;
	} else {
		HLock(fStuff->bitHandle);
		((GrafPtr)fStuff->offscreen)->portBits.baseAddr = *fStuff->bitHandle;
		return &((GrafPtr)fStuff->offscreen)->portBits;
	}
}

void unlockOffscreenBits(FileStuff *fStuff)
{
	PixMapHandle pixBase;

	if(gHasColorQD) {
		pixBase=GetGWorldPixMap(fStuff->offscreen);
		UnlockPixels(pixBase);
	} else {
		HUnlock(fStuff->bitHandle);
		((GrafPtr)fStuff->offscreen)->portBits.baseAddr = 0;
	}
	return;
}

void setOffscreenPort(FileStuff *fStuff,int how)
{
	static CGrafPtr savedPort;
	static GDHandle savedDevice;
	
	if(how) {	/* setPort(offscreen) */
		if(gHasColorQD) {
			GetGWorld(&savedPort,&savedDevice);
			SetGWorld(fStuff->offscreen,0);
		} else {
			GetPort((GrafPtr *)&savedPort);
			HLock(fStuff->bitHandle);
			((GrafPtr)fStuff->offscreen)->portBits.baseAddr=*fStuff->bitHandle;
			SetPort((GrafPtr)fStuff->offscreen);
		}
	} else {	/* restore saved port */
		if(gHasColorQD) 
			SetGWorld(savedPort,savedDevice);
		else {
			SetPort((GrafPtr)savedPort);
			HUnlock(fStuff->bitHandle);
			((GrafPtr)fStuff->offscreen)->portBits.baseAddr = 0;
		}
	}
}

void PtTimesGain(Point *where,int gain)
{
	if(gain>0) {
		where->v<<=gain;
		where->h<<=gain;
	} else {
		where->v>>=-gain;
		where->h>>=-gain;
	}
}

void FillOffscreen(DocumentPeek doc,int dgain)
{
	int i,nData;
	short xCenter,yCenter,gain,mapX,mapY;
	short mapH,mapV,DhMap,DvMap;
	short windowH,windowV,maxH,maxV;
	IntData *proj;
	Point centerRW,mapSize,*center=0;
	Rect mapFrame,viewRect;
	FileStuff *fStuff;
	int showBackground=FALSE;
	int BMSize,newCenter,centerOffset;
	extended mag;
	
	/*---------------------------------------------------------------------
	1. If gain is so small that map<window then values predetermined. 
		There is no scrolling, offsets are zero and the Map and BM
		centers are map middle (RW) and 1/2 map width in screen units. 
		
	2. If map<BM then need to calculate a projected center. Offsets 
		are identical and calculated from that center. Scrolling limit
		bounded by map width. 
		
	3.	If map>BM, then need to calculate the projected center and
		try to center BM and window on that center. DhMap is the scaled
		difference from that center to the left of the Map. 
	----------------------------------------------------------------------*/
	
	(*doc->getContentRect)((WindowPtr)doc,&viewRect);
	
	HLock(doc->docData);
	fStuff=(FileStuff *)*doc->docData;
	
	gain=fStuff->gain+dgain;
	windowH=RectWidth(viewRect);
	windowV=RectHeight(viewRect);
	centerRW.h=windowH/2;
	centerRW.v=windowV/2;
	mapFrame=fStuff->bounds;
	InsetRect(&mapFrame,-4,-4);
	SetPt(&mapSize,RectWidth(mapFrame),RectHeight(mapFrame));
	PtTimesGain(&mapSize,gain);
	
	if(mapSize.h<windowH) {						/* Map smaller than Window */
		fStuff->XMapCtr=(mapFrame.right+mapFrame.left)/2;	/* BM center in RW units. */
		fStuff->xBMCtr=(mapSize.h+1)/2;			/* BM half-width in screen units. */
		maxH=0;									/* Max scroll value */
		fStuff->DhBM=0;					/* Offset of Window in BM */
		DhMap=0;						/* Offset of Window in Map */
		showBackground=TRUE;
	} else {
		if(!doc->hScroll) {
			center=0;
			DhMap=fStuff->DhBM=0;
		} else {
			/* 	The center is converted to real-world coordinates. This conversion 
				depends upon ->DhBM, yBMCtr (usually 512), ->gain, and ->XMapCtr 
				(the RW center of the BM). This is the old gain. We don't need 
				this indirection, but center will be used as a flag.*/
			center=&centerRW;
			LocToMap(center,fStuff);
			newCenter=centerRW.h;				/* RW x-coordinate. */
		}
			
		maxH=mapSize.h-windowH;
		
		if(mapSize.h<OffScreenH) {			/* Map smaller than BM */

			/*	Now set values reflecting the new coordinate system. */
	
			fStuff->XMapCtr=(mapFrame.right+mapFrame.left)/2;
			fStuff->xBMCtr=(mapSize.h+1)/2;
			if(center) {
				/*	From center find window offset within BM. Convert RW to BM 
					coordinate system. This uses any new gain and the new XMapCtr
					and xBMCtr. Note DhBM is not needed. */
					
				newCenter-=mapFrame.left;			/* Offset from left edge of map (RW). */
				newCenter=(gain>0)? newCenter<<gain : newCenter>>-gain;	/* Scale to screen units. */
				newCenter-=windowH/2;				/* Center should be in middle of window. */
				
				DhMap=(newCenter>maxH)? maxH:(newCenter<0)? 0:newCenter ;
				fStuff->DhBM=DhMap;
			}
			showBackground=TRUE;
		} else {							/* Large map. */
						
			fStuff->xBMCtr=OffScreenH/2;	/* 512 */
			/* Scale 1024/2 to RW units. */
			BMSize=(gain>=1)? OffScreenH>>(gain-1) : OffScreenH<<(-1-gain);
			
			if(center) {
				if((newCenter+BMSize)>mapFrame.right) {
					fStuff->XMapCtr=mapFrame.right-BMSize;
					centerOffset=(gain>=0)? (fStuff->XMapCtr-newCenter)<<gain:(fStuff->XMapCtr-newCenter)>>-gain;
				} else if((newCenter-BMSize)<mapFrame.left)  {
					fStuff->XMapCtr=mapFrame.left+BMSize;
					centerOffset=(gain>=0)? (fStuff->XMapCtr-newCenter)<<gain:(fStuff->XMapCtr-newCenter)>>-gain;
				} else {
					fStuff->XMapCtr=newCenter;		/* BM will center on it. */
					centerOffset=0;
				}
				/* From center find window offset within BM */
				
				newCenter-=mapFrame.left;			/* Offset from left edge of map (RW). */
				newCenter=(gain>0)? newCenter<<gain : newCenter>>-gain;	/* Scale to screen units. */
				newCenter-=windowH/2;				/* Center should be in middle of window. */
				
				if(newCenter>maxH) {			/* Can't center, window at right of Map */
					DhMap=maxH;
					fStuff->DhBM=OffScreenH-windowH;
				} else if(newCenter<0) {
					DhMap=fStuff->DhBM=0;
				} else {
					DhMap=newCenter;
					fStuff->DhBM=(OffScreenH-windowH)/2 - centerOffset;									
				}
			} else
				fStuff->XMapCtr=mapFrame.left+BMSize;
		}
	}
	
	/*	Note that N & S are reversed in the mapFrame & fStuff->bounds Rects. */
		
	if(mapSize.v<windowV) {						/* Map smaller than Window */
		fStuff->YMapCtr=(mapFrame.bottom+mapFrame.top)/2;	/* BM center in RW units. */
		fStuff->yBMCtr=(mapSize.v+1)/2;			/* BM half-width in screen units. */
		maxV=0;									/* Max scroll value */
		fStuff->DvBM=0;					/* Offset of Window in BM */
		DvMap=0;								/* Offset of Window in Map */
		showBackground=TRUE;
	} else {
		if(!doc->vScroll) {
			center=0;
			fStuff->DvBM=DvMap=0;
		} else {
			if(!center) {
				center=&centerRW;				/* This is window center in local coordinates. */
				LocToMap(center,fStuff);		/* Convert to Map coordinates, with + = North */
			}
			newCenter=centerRW.v;				/* RW x-coordinate. */
		}
			
		maxV=mapSize.v-windowV;
		
		if(mapSize.v<OffScreenV) {			/* Map smaller than BM */	
			fStuff->YMapCtr=(mapFrame.bottom+mapFrame.top)/2;
			fStuff->yBMCtr=(mapSize.v+1)/2;
			if(center) {
				newCenter-= mapFrame.bottom;	/* In mapFrame, bottom is North. */
				newCenter=(gain>0)? -newCenter<<gain : -newCenter>>-gain;
				newCenter-=windowV/2;
				DvMap=(newCenter>maxV)? maxV:(newCenter<0)? 0:newCenter;
				fStuff->DvBM=DvMap;
			}
			showBackground=TRUE;
		} else {							/* Large map. */
			fStuff->yBMCtr=OffScreenV/2;	/* 512 */
			BMSize=(gain>=1)? OffScreenV>>(gain-1) : OffScreenV<<(-1-gain);
			if(center) {
				if((newCenter+BMSize)>mapFrame.bottom) {	/* Too close to top? (top & bottom reversed) */
					fStuff->YMapCtr=mapFrame.bottom-BMSize;
					centerOffset=(gain>=0)? (fStuff->YMapCtr-newCenter)<<gain:(fStuff->YMapCtr-newCenter)>>-gain;
				} else if((newCenter-BMSize)<mapFrame.top)  {	/* Too close to bottom? */
					fStuff->YMapCtr=mapFrame.top+BMSize;
					centerOffset=(gain>=0)? (fStuff->YMapCtr-newCenter)<<gain:(fStuff->YMapCtr-newCenter)>>-gain;
				} else {
					fStuff->YMapCtr=newCenter;		/* BM will center on it. */
					centerOffset=0;
				}
						
				newCenter-= mapFrame.bottom;		/* Distance from top edge of map (RW). */
				newCenter=(gain>0)? -newCenter<<gain : -newCenter>>-gain;	/* Scale to screen units. */
				newCenter-=windowV/2;				/* Center should be in middle of window. */
				
				if(newCenter>maxV) {			/* Can't center, window at bottom of Map */
					DvMap=maxV;
					fStuff->DvBM=OffScreenV-windowV;
				} else if(newCenter<0) {
					DvMap=fStuff->DvBM=0;
				} else {
					DvMap=newCenter;
					fStuff->DvBM=(OffScreenV-windowV)/2 + centerOffset ;
				}
			} else
				fStuff->YMapCtr=mapFrame.top+BMSize;
		}
	}
	fStuff->gain=gain;
	
	/* With all in place, we can set up the tentative coordinate system */
			
	RectMapToBM(&mapFrame,fStuff);	/* Convert to BM coordinates, at new offsets. */
	InsetRect(&mapFrame,-4,-4);	/* Give it a margin. */

	mapH=mapFrame.right-mapFrame.left;
	mapV=mapFrame.bottom-mapFrame.top;

	SetCtlMax(doc->hScroll,maxH);
	SetCtlMax(doc->vScroll,maxV);

	SetCtlMin(doc->hScroll,0);
	SetCtlMin(doc->vScroll,0);

	SetCtlValue(doc->hScroll,DhMap);
	SetCtlValue(doc->vScroll,DvMap);
		
	/* Now get ready to draw. */

	nData=fStuff->masters[nDataList].other+fStuff->masters[nLocnList].other+2;
	
	xCenter=fStuff->xBMCtr;
	yCenter=fStuff->yBMCtr;
	mapX=fStuff->XMapCtr;
	mapY=fStuff->YMapCtr;
	
	setOffscreenPort(fStuff,1);
	EraseRect(&fStuff->offscreen->portRect);
	if(showBackground) {
		FillRect(&fStuff->offscreen->portRect,(ConstPatternParam)&qd.gray);
		EraseRect(&mapFrame);
		FrameRect(&mapFrame);
	}
	HLock(fStuff->intData);
	proj=(IntData *)*fStuff->intData;

	if(gain>=0) {
		for(i=1;i<=nData;i++,proj++) {
			if(!(proj->code&Virtual)) {
				if(proj->code&Move_to)
					MoveTo(xCenter+((proj->x-mapX)<<gain),yCenter-((proj->y-mapY)<<gain));
				else if(proj->code&Line_to) 
					LineTo(xCenter+((proj->x-mapX)<<gain),yCenter-((proj->y-mapY)<<gain));
			}
		}
	} else {
		gain=-gain;
		for(i=1;i<=nData;i++,proj++) {
			if(!(proj->code&Virtual)) {
				if(proj->code&Move_to) 
					MoveTo(xCenter+((proj->x-mapX)>>gain),yCenter-((proj->y-mapY)>>gain));
				else if(proj->code&Line_to) 
					LineTo(xCenter+((proj->x-mapX)>>gain),yCenter-((proj->y-mapY)>>gain));
			}
		}
	}

	if(fStuff->selected.survey) {
		LockLists(fStuff);
		HiliteSurvey(getSurvey(fStuff->selected.survey),fStuff,fStuff->sequence);
		UnlockLists(fStuff);
	}
	HUnlock(fStuff->intData);
	setOffscreenPort(fStuff,0);
	
	mag=1<<abs(fStuff->gain);
	if(fStuff->gain>0)
		mag=1/mag;
	sprintf(fStuff->scaleButton.name,"1:%.0f",864*mag);
	c2pstr(fStuff->scaleButton.name);
	
	gain=2-fStuff->gain;	/* turn gain (log base 2 value) into menu item number. */
	HUnlock(doc->docData);

	CheckItem(ScaleMenu,1,gain==1);
	CheckItem(ScaleMenu,2,gain==2);
	CheckItem(ScaleMenu,3,gain==3);
	CheckItem(ScaleMenu,4,gain==4);
	CheckItem(ScaleMenu,5,gain==5);
	CheckItem(ScaleMenu,7,gain>5 || gain<1);
	
	return;
}

#if 0
void FillWindow(FileStuff *fStuff)
{
	int i,j,cnt=0,nFSBs;
	RgnHandle viewRgn;
	Bounds *FSBs,*surveys,*base;
	Rect bounds;
	DocumentPeek doc;
	
	doc=fStuff->doc;
	(*doc->getContentRect)((WindowPtr)doc,&bounds);
	RectLocToMap(&bounds,fStuff);
	viewRgn=NewRgn();
	if(!viewRgn) {
		errorAlertDialog(-OutOfMemory);
		return;
	}
	RectRgn(viewRgn,&bounds);
	HLock(fStuff->boundsData);
	HLock(fStuff->intData);
	LockLists(fStuff);
	base=FSBs=(Bounds *)*fStuff->boundsData;
	nFSBs=FSBs->who;
	for(i=0;i<nFSBs;i++,FSBs++) {
		if(RectInRgn(&FSBs->bounds,viewRgn)) {
			surveys=base+FSBs->who;
			for(j=0;j<FSBs->what;j++,surveys++) {
				if(RectInRgn(&surveys->bounds,viewRgn)) {
					DrawSurvey(getSurvey(surveys->who),fStuff);
					surveys->what=1;
				} 
			}
		} 
	}
	DisposeRgn(viewRgn);
	surveys=base+base->who;
	if(fStuff->selected.survey) {
		HiliteSurvey(getSurvey(fStuff->selected.survey),fStuff,fStuff->sequence);
		(surveys+fStuff->selected.who)->what=1;/*===== won't work =====*/
	}
	j=((LongList *)fStuff->masters[nSegList].p)->current;
	surveys=base+base->who;
	for(i=0;i<j;i++,surveys++) {
		if(!surveys->what) {
			fStuff->drawMore=i;
			setOffscreenPort(fStuff,0);
			return;
		} else
			surveys->what=0;	/* Clear as you go */
	}
	/* No more drawing to do. Unlock everything. */
	fStuff->drawMore=0;
	UnlockLists(fStuff);
	HUnlock(fStuff->intData);
	HUnlock(fStuff->boundsData);
	setOffscreenPort(fStuff,0);
	return;
	
}

void DrawMore(FileStuff *fStuff)
{
	int nSurveys,cnt,i;
	Bounds *surveys,*base;

	base=(Bounds *)*fStuff->boundsData;
	surveys=base+base->who;
	nSurveys=((LongList *)fStuff->masters[nSegList].p)->current;
	for(cnt=0,i=fStuff->drawMore;i<=nSurveys;i++,cnt++) {
		if(!surveys[i].what) 
			DrawSurvey(getSurvey(surveys->who),fStuff);
		else
			surveys->what=0;
		if(cnt>9) {
			fStuff->drawMore=i;
			return;
		}
	}	
	/* No more drawing to do. Unlock everything. */
	fStuff->drawMore=0;
	UnlockLists(fStuff);
	HUnlock(fStuff->intData);
	HUnlock(fStuff->boundsData);
	return;
}
#endif

void TransformCoorinates(FileStuff *fStuff)
{
	IntData *proj;
	int i;
	Locn *data;
	short *pen;
	DimData *dims;
	LongList *dataList,*locnList,*dimList;
	
	fStuff->transform.mag=1;

	HLock(fStuff->intData);
	proj=(IntData *)*fStuff->intData;
	dataList=fStuff->masters[nDataList].p;
	locnList=fStuff->masters[nLocnList].p;
	dimList=fStuff->masters[nDimList].p;
	
	proj->code=0;
	for(proj++,i=1;i<=locnList->current;i++,proj++) {
		data=getLocn(i);
		if(proj->code=(data->code&_Rel)? 0:data->code) {
			proj->x=data->x;
			proj->y=data->y;
		}
	}
	proj->code=0;
	proj->code=(data->code&_Rel)? 0:proj->code;

	for(proj++,i=1;i<=dataList->current;i++,proj++) {
		data=getData(i);
		if(proj->code=(data->code&_Rel)? 0:data->code) {
			proj->x=data->x;
			proj->y=data->y;
		}
	}
	HUnlock(fStuff->intData);
	if(fStuff->penData) {
		HLock(fStuff->penData);
		pen=(short *)*fStuff->penData;
		
		for(pen++,i=1;i<=dimList->current;i++,pen++) {
			dims=getDims(i);
			*pen=dims->l+dims->r;
		}

		HUnlock(fStuff->penData);
	}
	return;
}

void FloatToString(float num,char *str)
{
	sprintf(str,"%.1f",num);
	C2PStr(str);
}

int StringToFloat(char *str,float *num)
{
	char *s;
	int decimal=0;
	int digit=0;
	
	*(str+*str+1)=0;
	s=str+1;
	while(isspace(*s)) s++;
	if(*s=='+' || *s=='-') s++;
	while(*s) {
		if(isdigit(*s)) 
			digit++;
		else if(*s=='.') 
			decimal++;
		else 
			return FALSE;
		s++;
	}
	if(!digit || decimal>1)
		return FALSE;
	*num=atof(str+1);
	return TRUE;
}
	
pascal void doButton(DialogPtr wind,short x)
{
#ifdef MC68000
#pragma unused (x)
#endif
	short itemType;
	Handle itemHand;
	Rect box;

	GetDItem(wind,1,&itemType,&itemHand,&box);
	PenSize(3,3);
	InsetRect(&box,-4,-4);
	FrameRoundRect(&box,16,16);
}

pascal void doFrame(DialogPtr wind,short item)
{
	short itemType;
	Handle itemHand;
	Rect box;

	GetDItem(wind,item,&itemType,&itemHand,&box);
	PenSize(1,1);
	FrameRect(&box);
}

pascal void doLine(DialogPtr wind,short item)
{
	short itemType;
	Handle itemHand;
	Rect box;

	GetDItem(wind,item,&itemType,&itemHand,&box);
	PenSize(1,1);
	MoveTo(box.left,box.top);
	LineTo(box.right,box.top);
}

int SizeToFit(FileStuff *fStuff,Rect objectBounds)
{
	extended windowH,windowV,objectH,objectV;
	Rect r;
	extended mag;
	short gain;
	
	(*fStuff->doc->getContentRect)((WindowPtr)fStuff->doc,&r);
	windowH=r.right-r.left;	/* window width */
	windowV=r.bottom-r.top; /* window height */
	objectH=objectBounds.right-objectBounds.left;
	objectV=objectBounds.bottom-objectBounds.top;
	windowH/=objectH;	
	windowV/=objectV;
	mag=(windowH<windowV)? windowH:windowV;
	gain=log(mag)*1.4426950409;
	gain--;

	if(gain>15) gain=15;
	if(gain<-15) gain=-15;

	return gain-fStuff->gain;
}

/*------------------------ Pane Object ------------------------*/

Boolean paneInitDoc(WindowPtr wind)
{
	WindowPtr parent;
	
	parent=FrontLayer();	/* note, pane not yet shown, so FrontLayer works */
	if(!parent)
		return false;

	SetPort(wind);
	TextFont(monaco);
	TextSize(9);
	
	((DocumentPeek)wind)->homeWindow = parent;
	((DocumentPeek)wind)->associatedWindow = ((DocumentPeek)parent)->associatedWindow;
	((DocumentPeek)parent)->associatedWindow = wind;
	SetPort(parent);
	return true;
}

Boolean paneMakeWindow(WindowPtr wind)
{
	wind = GetNewWindow(1128,(Ptr)wind,NULL);
	if(!wind)
		return 0;
	((DocumentPeek)wind)->fDocWindow=true;
	((WindowPeek)wind)->windowKind=10;
	NumWindows++;
	return true;
}

void paneDestructor(WindowPtr wind)
{
	DocumentPeek parent;
	
	((DocumentPeek)wind)->docData=0;
	parent = (DocumentPeek)((DocumentPeek)wind)->homeWindow;
	parent->associatedWindow = ((DocumentPeek)wind)->associatedWindow;
	destructor(wind);
}

void paneDeactivate(WindowPtr wind)
{
	/* 	
		Deactives are either ignored or passed on to the parent object.
		ignore: pane is in the front plane.
		else pass on
	*/
	WindowPtr parent;
	
	if(gInBackground || (((DocumentPeek)wind)->homeWindow != FrontLayer())) {
		parent = ((DocumentPeek)wind)->homeWindow;
		(*((DocumentPeek)parent)->deactivate)(parent);
	}
}

void AddPaneMemberFunctions(DocumentPeek doc)
{
	doc->makeWindow=paneMakeWindow;
	doc->initDoc=paneInitDoc;
	doc->destructor=paneDestructor;
	doc->docData=(Handle)1;
	doc->doGrow=nullEventMethod;
	doc->deactivate=paneDeactivate;
}

static void MakeOnePane(void)
{
	DocumentPeek doc;
		
	if(doc = MakeDoc('pane',0)) {
		if(((*doc->makeWindow)((WindowPtr)doc)) && ((*doc->initDoc)((WindowPtr)doc)))
			ShowDocWindow((WindowPtr)doc);
		else
			DeleteDoc(doc);
	}
}

#ifdef MC68000
#pragma segment rose
#endif

int SurveyFillRoseFromDims(Survey *su,FileStuff *fStuff,long *array,long binWidth)
{
	int sta,j,cnt=0,bin;
	int d,locnListOffset;
	SubSurvey *ss;
	String *st;
	Locn *theData,*theData2;
	Link *theLink;
	DimData *theDims,*theDims2;
		
	locnListOffset=((LongList *)fStuff->masters[nLocnList].p)->current;
	for(j=0;j<su->nStrings;j++) {
		st=getStr(su->strings+j);
		if((st->code&(FixedFlag|UnMapped|Virtual))==FixedFlag) { /* FixedFlag, Mapped, and not Virtual */
			sta=st->first;
			for(ss = (struct subSegRec *)su,theData=0;!theData;) {
				if(ss->first<=sta && sta<=ss->last) {
					theData=getData(ss->data+sta-ss->first);
					theDims=getDims(d=locnListOffset+ss->data+sta-ss->first);
				} else if (ss->nextSubSeg)
					ss=getSubSurvey(ss->nextSubSeg);
			}
			
			if(theDims->azi!=0xFFFF) {
				array[bin=theDims->azi/binWidth]++;
				if(bin==0)
					bin++;
				cnt++;
			}
			/*	A Dims->azi==0xFFFF is for .. stations. There is no shot
				associated with them. */
			for(++sta,++theData,++d;sta<st->last;sta++,theData++,d++) {
				if(!(theData->code&Virtual)) {
					theDims=getDims(d);
					if(theDims->azi!=0xFFFF) {
						array[bin=theDims->azi/binWidth]++;
						if(bin==0)
							bin++;
						cnt++;
					}
				}
			}
		}
	}
	for(j=su->links+su->nLocns+su->nLinks;j<su->links+su->nLocns+su->nLinks+su->nTies;j++) {
		theLink = getLink(j);
		if(theLink->to.survey>0 && theLink->data && !(theLink->from.survey&fLink)) {
			theData2 = getLocn(theLink->data);
			theDims2 = getDims(theLink->data);
			if(!(theData2->code&Virtual)) {
				if((theData2->code&(_Point|_Rel))==Line_to) {
				theData = theData2-1;
				theDims = getDims(theLink->data-1);
					if(theDims->azi!=0xFFFF) {
						array[bin=theDims->azi/binWidth]++;
						if(bin==0)
							bin++;
						cnt++;
					}
				}
			}
		}
	}
	return cnt;
}

#if 0
int SurveyCalculateRose(Survey *su,FileStuff *fStuff,long *array,long binWidth)
{
	int sta,j,cnt=0,bin;
	short dx,dy,ang;
	Fixed ratio;
	int locnListOffset;
	SubSurvey *ss;
	String *st;
	IntData *locn,*toData,*fromData;
	Link *theLink;
	LongList *locnList;
		
	locn=(IntData *)*fStuff->intData;
	locnList=(LongList *)fStuff->masters[nLocnList].p;
	locnListOffset=((LongList *)fStuff->masters[nLocnList].p)->current;
	for(j=0;j<su->nStrings;j++) {
		st=getStr(su->strings+j);
			/* 
			The only reason we go through the string records is
			to check some flags that are only set there.
			*/
		if((st->code&(FixedFlag|UnMapped|Virtual))==FixedFlag) { 
				/* 
				i.e. Fixed (no longer relative coordinates), 
				Mapped (not just a dummy string put there for a tie), 
				and not Virtual (plot wanted) 
				*/
			sta=st->first;
				/*
				Now we have to find the data and that means going 
				to the subsegment (subsurvey) records. They point
				to the main data.
				*/
			for(ss = (struct subSegRec *)su,toData=0;!toData;) {
				if(ss->first<=sta && sta<=ss->last) 
					toData=locn+locnListOffset+ss->data+sta-ss->first+1;
				else if (ss->nextSubSeg)
					ss=getSubSurvey(ss->nextSubSeg);
			}	/* Fatal if no subsurvey */

			/*----------------- Loop 1: through the Main Data ---------------
				The Main Data consists of sequential stations. Most data
				is here. They are stored in numberical station order, grouped
				by survey. X,y,z triplets are floats. Bits in the code field 
				categorize each point as a move_to, line_to, move_to_relative,
				or line_to_relative. There should be no relative shots at
				this point (they are caused by floating surveys) but we check
				anyway.
			----------------------------------------------------------------*/ 
			for(;sta<st->last;sta++) {
				fromData=toData++;
				if(!(toData->code&Virtual)) {
					dx=toData->x-fromData->x;
					dy=fromData->y-toData->y;	/* Y coordinate plane reversed in QuickDraw */
					ratio=FixRatio(dy,dx);
					ang=AngleFromSlope(ratio);
					array[bin=ang/binWidth]++;
					if(bin==0)
						bin++;
					cnt++;
				}
			}
		}
	}
	/*----------------- Loop 2: through the tie data ---------------
		Ties between surveys result in a second set of coordinates for
		each tie station. Also some stations are only represented by
		ties, such as spray shots, or screwy survey practices where 
		sequential numbering is not used. An example of the latter would
		be A1->A1'->A2->A2', etc. Tie data is stored separately from the
		main data and is accessed through link records.
	----------------------------------------------------------------*/ 
	for(j=su->links+su->nLocns+su->nLinks;j<su->links+su->nLocns+su->nLinks+su->nTies;j++) {
		theLink = getLink(j);
		if(theLink->to.survey>0 && theLink->data && !(theLink->from.survey&fLink)) {
			toData = locn+theLink->data;
			fromData = toData-1;
			if(!(toData->code&Virtual)) {
				if((fromData->code&(_Point|_Rel))==Move_to) {
					dx=toData->x-fromData->x;
					dy=fromData->y-toData->y;
					ratio=FixRatio(dy,dx);
					ang=AngleFromSlope(ratio);
					array[bin=ang/binWidth]++;
					if(bin==0)
						bin++;
					cnt++;
				}
			}
		}
	}
	return cnt;
}

#else

int SurveyCalculateRose(Survey *su,FileStuff *fStuff,long *array,long binWidth)
{
	int sta,j,cnt=0,bin;
	c_float dx,dy,ang;
	SubSurvey *ss;
	String *st;
	Locn *toData,*fromData;
	Link *theLink;
	
	for(j=0;j<su->nStrings;j++) {
		st=getStr(su->strings+j);
			/* 
			The only reason we go through the string records is
			to check some flags that are only set there.
			*/
		if((st->code&(FixedFlag|UnMapped|Virtual))==FixedFlag) { 
				/* 
				i.e. Fixed (no longer relative coordinates), 
				Mapped (not just a dummy string put there for a tie), 
				and not Virtual (plot wanted) 
				*/
			sta=st->first;
				/*
				Now we have to find the data and that means going 
				to the subsegment (subsurvey) records. They point
				to the main data.
				*/
			for(ss = (struct subSegRec *)su,toData=0;!toData;) {
				if(ss->first<=sta && sta<=ss->last) 
					toData=getData(ss->data+sta-ss->first);
				else if (ss->nextSubSeg)
					ss=getSubSurvey(ss->nextSubSeg);
			}	/* Fatal if no subsurvey */

			for(;sta<st->last;sta++) {
				fromData=toData++;
				if(!(toData->code&Virtual)) {
					dx=toData->x-fromData->x;
					dy=toData->y-fromData->y;

					ang=atan2(dx,dy)*RtoD;
					ang+=(ang<0)?360:0;
					bin=ang/binWidth;
					if(bin>=0  && bin<gNumRoseBins) {
						array[bin]++;
						if(bin==0)
							bin++;
						cnt++;
					} else
						SysBeep(7);
				}
			}
		}
	}

	for(j=su->links+su->nLocns+su->nLinks;j<su->links+su->nLocns+su->nLinks+su->nTies;j++) {
		theLink = getLink(j);
		if(theLink->to.survey>0 && theLink->data && !(theLink->from.survey&fLink)) {
			toData = getLocn(theLink->data);
			fromData = toData-1;
			if(!(toData->code&Virtual)) {
				if((fromData->code&(_Point|_Rel))==Move_to) {
					dx=toData->x-fromData->x;
					dy=toData->y-fromData->y;

					ang=atan2(dx,dy)*RtoD;
					ang+=(ang<0)?360:0;
					bin=ang/binWidth;
					if(bin>=0  && bin<gNumRoseBins) {
						array[bin]++;
						if(bin==0)
							bin++;
						cnt++;
					} else
						SysBeep(7);
				}
			}
		}
	}
	return cnt;
}
#endif

Handle FillRoseHistogram(WindowPtr target)
{
	FileStuff *fStuff;
	Handle rose;
	long *histogram,nShots=0,binWidth,i;
	short ret;
	Survey *su;
	LongList *surveyList;
	static Boolean sRoseRemind=true;
	
	if(rose=NewHandle(gNumRoseBins*sizeof(long))) {
		HLock(((DocumentPeek)target)->docData);
		fStuff=(FileStuff *)*((DocumentPeek)target)->docData;
		LockLists(fStuff);
		HLock(rose);
		histogram=(long *)*rose;
		memset(histogram,0,GetHandleSize(rose));
		surveyList=fStuff->masters[nSegList].p;
		binWidth=360/gNumRoseBins;
		if(fStuff->masters[nDimList].p) {
			binWidth*=10;
			for(i=1;i<=surveyList->current;i++) {
				su=getSurvey(i);
				nShots+=SurveyFillRoseFromDims(su,fStuff,histogram,binWidth);
			}
		} else {
			if(sRoseRemind) {
				ParamText(((DocumentPeek)target)->fileSpecs.name,(Str255)"",(Str255)"",(Str255)"");
				ret = NoteAlert(501,(ModalFilterProcPtr)nil);
				if(ret==3) {
					ret=1;
					sRoseRemind=false;
				}
			} else
				ret=1;
			if(ret==1) {
				for(i=1;i<=surveyList->current;i++) {
					su=getSurvey(i);
					nShots+=SurveyCalculateRose(su,fStuff,histogram,binWidth);
				}
			} else {
				HUnlock(rose);
				DisposeHandle(rose);
				rose=0;
			}
		}
  		UnlockLists(fStuff);
		if(rose)
			HUnlock(rose);
		HUnlock(((DocumentPeek)target)->docData);
	}
	return rose;
}

static int RoseModalDialog(WindowPtr wind)
{
	/* 
		Returns the type of rose diagram:
			0:	Cancel
			1:	Polar
			2:	Histogram
			3:	Text
		or the opposite of these if the rose diagram is to operate
		on the selected survey(s).
	*/
	
	short selS;
	
	selS=((FileStuff *)*((DocumentPeek)wind)->docData)->selected.survey;
	gNumRoseBins=36;
	
	return 1;
}

static void OpenRoseWindow(WindowPtr theWindow)
{
	DocumentPeek doc;
	FileStuff *fStuff;
	int type;
		
	if(type=RoseModalDialog(theWindow)) {
		if(type==3) {
			OpenTextWindow(theWindow,3);
		}
		if(doc = MakeDoc('rose',0)) {
			doc->associatedWindow = theWindow;		/* Map window to which this is attached */
			if(((*doc->makeWindow)((WindowPtr)doc)) && ((*doc->initDoc)((WindowPtr)doc)))
				ShowDocWindow((WindowPtr)doc);
			else {
				DeleteDoc(doc);
				return;
			}
			HLock(doc->docData);
			fStuff=(FileStuff *)*((DocumentPeek)theWindow)->docData;

			HUnlock(doc->docData);
		}
	}
}

void ScaleHistogram(Handle h,int nElems,long minOut,long maxOut)
{
	long *array;
	long i;
	long min,max,dividend,divisor;
	
	/*	By the way, don't reuse variables in most subroutines. The
		compiler does that for you. */
	
	HLock(h);
	array=(long *)*h;
	
	min=max=*array;
	for(i=1;i<nElems;i++) {
		if(min>*(array+i))
			min=*(array+i);
		else if(max<*(array+i))
			max=*(array+i);
	}
	dividend=max-min;
	divisor=maxOut-minOut;
	if(divisor) {
		for(i=0;i<nElems;i++) 
			array[i]=(array[i]-min)*divisor/dividend+minOut;
	}
	HUnlock(h);
}

Boolean roseInitDoc(WindowPtr wind)
{
	DocumentPeek doc;
	Rect pictRect,param;
	long *value;
	Handle distributionH;
	short i,cnt,center,ang,angD,dh,dv;
	GrafPtr oldPort;
	RgnHandle oldClip;
	
	doc = (DocumentPeek)wind;
	
	SetRect(&pictRect,0,0,480,480);
	SetRect(&doc->limitRect,80,80,pictRect.right,pictRect.bottom);
	if(!(doc->fPrintRecord = (THPrint)NewHandle(sizeof(TPrint))))
		return false;
		
	PrOpen();
	PrintDefault(doc->fPrintRecord);
	PrClose();
	
	if(!(distributionH=FillRoseHistogram(doc->associatedWindow)))
		return false;
		
	doc->docData=distributionH;
	
	ScaleHistogram(distributionH,gNumRoseBins,0,200);

	angD=360/gNumRoseBins;
	center = 240;
	
	GetPort(&oldPort);
	SetPort(wind);
	
	if(!(doc->docData=(Handle)OpenPicture(&pictRect))) {
		SetPort(oldPort);
		DisposeHandle(distributionH);
		return false;
	}
	
	oldClip = NewRgn();
	GetClip(oldClip);
	ClipRect(&pictRect);
	for(ang=i=0,value=(long *)*distributionH;i<gNumRoseBins;value++,i++,ang+=angD) {
		cnt = *value;
		SetRect(&param,-cnt,-cnt,cnt,cnt);
		OffsetRect(&param,center,center);
		FrameArc(&param,ang,angD);
		if(i) {
			cnt=(cnt>(short)*(value-1))? cnt : (short)*(value-1);
			dv=(short)(cnt*cos(DtoR*ang));
			dh=(short)(cnt*sin(DtoR*ang));
		} else {
			dh=0;
			dv=(cnt>(short)*(value+gNumRoseBins-1))? cnt : (short)*(value+gNumRoseBins-1);
		}
		MoveTo(center,center);
		LineTo(center+dh,center-dv);
	}
	
	ClosePicture();
	DisposeHandle(distributionH);
	SetClip(oldClip);
	DisposeRgn(oldClip);

	if(!InitScrollDoc(wind))
		return false;

	DrawGrowIcon(wind);
	FocusOnContent(wind);
	return true;
}

void roseDestructor(WindowPtr wind)
{	
	if(((DocumentPeek)wind)->docData) 
		KillPicture((PicHandle)((DocumentPeek)wind)->docData);
	((DocumentPeek)wind)->docData=0;
	destructor(wind);
}

void roseDraw(WindowPtr wind, Rect *r,short x)
{
#ifdef MC68000
#pragma unused (x)
#endif
	EraseRect(r);
	DrawPicture((PicHandle)((DocumentPeek)wind)->docData,
		&(**((PicHandle)((DocumentPeek)wind)->docData)).picFrame);
}

short roseGetHorizSize(WindowPtr wind)
{
	PicHandle pH;
	short ret=0;
	
	if(pH = (PicHandle)((DocumentPeek)wind)->docData) 
		ret=(**pH).picFrame.right-(**pH).picFrame.left;
	
	return ret;
}

short roseGetVertSize(WindowPtr wind)
{
	PicHandle pH;
	short ret=0;
	
	if(pH = (PicHandle)((DocumentPeek)wind)->docData) 
		ret=(**pH).picFrame.bottom-(**pH).picFrame.top;
	
	return ret;
}

void adjustCursor(WindowPtr, Point pt)
{	
#ifdef MC68000
#pragma unused (pt)
#endif
	InitCursor();
}

static void AddRoseMemberFunctions(DocumentPeek doc)
{
	doc->initDoc=roseInitDoc;
	doc->destructor=roseDestructor;
	doc->draw=roseDraw;
	doc->doPageSetup=doPageSetup;
/*	doc->doPrint=roseDoPrint; */
	doc->doGrow=scrollDoGrow;
	doc->doZoom=scrollDoZoom;
	doc->adjustCursor=adjustCursor;

/*	doc->setScrollBarValues=roseSetScrollBarValues; */

	doc->getHorizSize=roseGetHorizSize;
	doc->getVertSize=roseGetVertSize;
	
/*	doc->scrollContents=roseScrollContents; */
	
/*	==>doc->doDocMenuCommand=roseDoDocMenuCommand; */
/*==>	doc->adjustDocMenus=roseAdjustDocMenus; */
}
