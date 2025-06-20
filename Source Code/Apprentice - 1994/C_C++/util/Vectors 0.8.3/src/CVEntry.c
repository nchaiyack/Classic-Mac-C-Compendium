#include "CVectors.h"
#include "CVGlobs.h"

#define DO_CONFIGURE

/*	Fields[0] flag bits */
#define fStaggered 1
#define fRemarksFieldOnPage 2
#define fRemarksAreNotes 4

/*	Fields[1] flag bits */
#define fBlank 1

/*	Columns flag bits */
/* in byte[0] */
#define fSplit 128
#define fNarrow 64
#define TabMask 3
#define fTabBack 2
#define fTabDown 1

/* in byte[1] */
#define fAlt 16

#define kMaxTemplateCols 20
#define kMaxTemplateSta 21

#define StationsPerPageItem 5
#define NumberOfColumnsItem 7
#define HalfWidthCheckItem 9
#define SplitCheckItem 10
#define TabRightRadioItem 14
#define TabDownRadioItem 15
#define TabBackRadioItem 16
#define confUserFrame1 17
#define confUserFrame2 19
#define StaggeredCheckItem 23
#define TemplateTitleItem 3
#define confLastItem 26
#define TemplateItem 8
#define PopItem 12

#define confMakeButton 2
#define confDeleteButton 25
#define confCancelButton 1


#define LeftSlop 13			/* leave this much space on left of title */
#define RightSlop 10		/* this much on right */
#define BotSlop 5			/* this much below baseline */

#define MenuMask 15
#define AltBits 0x88442

#define cellHeight 32
#define cellHHeight 14
#define cellWidth 40
#define cellNWidth 30
#define cellMargin 5
#define endSize 48
#define SizeOfDItem 14

void dlogDraw(WindowPtr,Rect *,short);
void dlogDoContent(WindowPtr,EventRecord *);
void labelEntryPage(DialogPtr);
void labelTitlePage(DialogPtr);

int DoItemHit(DialogPtr,short);

Boolean entryDoDocMenuCommand(WindowPtr, short, short, short);
Boolean titleDoDocMenuCommand(WindowPtr, short, short, short);

void DoDialogEvent(WindowPtr,EventRecord *);
void dlogDoActivate(WindowPtr,EventRecord *);
void dlogDoTheUpdate(WindowPtr,EventRecord *);
static void Title2CML(WindowPtr,Handle);
static void Entry2CML(WindowPtr,Handle);

short GetTemplate(short,char *);
int CalcNTemplateCols(char *);
void RemoveDuplicates(char *);
pascal void DrawPopUp(DialogPtr, short);
void checkCDialogConsistency(char *);
Handle MakeDlogResource(char *,Handle,StringPtr);
Handle newDataDItemList(char *,Point *,DEMapHandle);
Boolean DisposeMRPd(short);
DEMapHandle AllocateDEMap(char *);
void AddTemplateMap(short,Handle);
DEMapHandle CalcDEMap(char *);


#ifdef MC68000
#pragma segment dataEntry
#endif

/*--------------------- Data Dialog Object -----------------------*/

void dlogDestructor(WindowPtr wind)
{
	WindowPtr sister;
	
	if(((DocumentPeek)wind)->fDocWindow)
		CloseDialog((DialogPtr)wind);
	((DocumentPeek)wind)->fDocWindow=0;
	((DocumentPeek)wind)->docData=0;	/*	Handle is not disposed. It remains in the 
											gMapHandle array. */
	CheckItem(DataMenu,iDataPage,false);
	CheckItem(DataMenu,iTitlePage,false);
	if(((DocumentPeek)wind)->associatedWindow) {
		sister = ((DocumentPeek)wind)->associatedWindow;
		((DocumentPeek)sister)->associatedWindow=0;
		((DocumentPeek)wind)->associatedWindow=0;
	}
	destructor(wind);
}

void dlogDoIdle(WindowPtr wind)
{
	EventRecord event;
	
	event.what = nullEvent;
	FocusOnContent(wind);
	DoDialogEvent(wind,&event);
}

void dlogDraw(WindowPtr wind,Rect *r,short page)
{	
	EraseRect(r);
	DrawDialog(wind);
	if(page) ;			/* Special drawing method for printing goes here */
}

void entryDlogDraw(WindowPtr wind,Rect *r,short page)
{
	dlogDraw(wind,r,0);
	labelEntryPage(wind);
	if(page) ;
	
}

void titleDlogDraw(WindowPtr wind,Rect *r,short page)
{
	dlogDraw(wind,r,0);
	labelTitlePage(wind);
	if(page) ;
}

void DoDialogEvent(WindowPtr wind,EventRecord *event)
{
	short itemHit;
	DialogPtr dialog;
		
	if(IsDialogEvent(event)) {
		dialog = (DialogPtr)wind;
		if(DialogSelect(event,&dialog,&itemHit))
			DoItemHit(dialog,itemHit);
	}
}

int DoItemHit(DialogPtr dialog,short itemHit)
{
	((DocumentPeek)dialog)->refCon = (long) itemHit;
	return 0;
}

void dlogDoActivate(WindowPtr wind,EventRecord *event)
{	
	FocusOnWindow(wind);
	doActivate(wind,event);
	FocusOnContent(wind);
	DoDialogEvent(wind,event); 
}

void dlogDoTheUpdate(WindowPtr wind,EventRecord *x)
{	
#ifdef MC68000
#pragma unused (x)
#endif
	Rect r,oldh,oldv,new;
	
	if(((DocumentPeek)wind)->fDocWindow) {
		SetRect(&new,0,0,0,0);
		oldh = (**((DocumentPeek)wind)->hScroll).contrlRect;
		oldv = (**((DocumentPeek)wind)->vScroll).contrlRect;
		(**((DocumentPeek)wind)->hScroll).contrlRect=new;
		(**((DocumentPeek)wind)->vScroll).contrlRect=new;
		
		FocusOnContent(wind);
		BeginUpdate(wind);
		r=(**(wind->visRgn)).rgnBBox;
		(*((DocumentPeek)wind)->draw)(wind,&r,0);
		FocusOnWindow(wind);

		(**((DocumentPeek)wind)->hScroll).contrlRect=oldh;
		(**((DocumentPeek)wind)->vScroll).contrlRect=oldv;
		if(((DocumentPeek)wind)->hScroll)
			Draw1Control(((DocumentPeek)wind)->hScroll);
		if(((DocumentPeek)wind)->vScroll)
			Draw1Control(((DocumentPeek)wind)->vScroll);
		
		DrawGrowIcon(wind);
		EndUpdate(wind);
	}
}

void dlogDoContent(WindowPtr wind,EventRecord *event)
{
	Rect contents;
	
	FocusOnWindow(wind);
	GlobalToLocal(&event->where);
	(*((DocumentPeek)wind)->getContentRect)(wind,&contents);
	if(PtInRect(event->where,&contents)) {
		LocalToGlobal(&event->where);
		FocusOnContent(wind);
		DoDialogEvent(wind,event);
	} else
		ScrollClick(wind,event);
}

/* 	
	Check philosophy: If an error is detected, the user is shown
	a dialog inwhich he has a chance to correct it. The correction
	is not checked again. The default is to accept the error. 
*/
void CheckDistance(StringPtr buf)
{
	char *s;
	int decimalCnt=0;
	
	s=(char*)buf+1;
	*(s+*buf) = '\0';
	while(isspace(*s)) s++;
	if(!isdigit(*s) && *s!='.') {
		CorrectDialog(buf,(StringPtr)"\pDistances have to begin with a digit or decimal point.");
		return;
	}
	while(isdigit(*s)) s++;
	if(*s=='\0')
		return;
	if(*s=='.') {
		decimalCnt++;
		s++;
	} else if(tolower(*s)=='f' || tolower(*s)=='m' || *s=='\'')
		s++;
	while(isdigit(*s)) s++;
	if(*s=='\0' || *s=='"')
		return;
	if(decimalCnt>1)
		CorrectDialog(buf,(StringPtr)"\pThis is not a number. It has more than one decimal point.");
	else
		CorrectDialog(buf,(StringPtr)"\pThis distance not a valid positive number.");
	return;
}

/*---------------------------------------------------------------------
	cvParseFixed parses fixed point numbers. For example, it rather than
	parseFloat is called by cvQuadtof because the 'E' of quad notation
	means that exponenants cannot be used in them.
---------------------------------------------------------------------*/

extended cvParseFixed(char **str)
{
	extended place=1;
	int n=0,m=0,neg=FALSE;
	char *p;
	
	p=*str;
	if (*p=='+') 
		p++;
	if (*p=='-') {
		p++;
		neg=TRUE;
	}
	
	if(!isdigit(*p) && *p!='.') {
		*str=p;
		CorrectDialog(*str,(StringPtr)"\pThis is not a valid number");
		return 0;
	}
	
	for(;isdigit(*p);p++) {
		n*=10;
		n+=*p-48;
	}
	
	if (*p=='.') {
		p++;
		for(place=1;isdigit(*p);p++) {
			m*=10; 
			m+=*p-48;
			place *= 0.1;
		}
	}
	*str=p;
	return (neg)? -(n+m*place) : n+m*place;
}

extended cvQuadtof(StringPtr str)
{
	extended ang;
	char ns,ew,*p;
	
	p=(char*)str;
	ns=tolower(*p);
	p++;
	if(isalpha(*p)) {
		ew=tolower(*p);
		p++;
		ang=cvParseFixed(&p);
	} else {
		ang=cvParseFixed(&p);
		ew=tolower(*p);
		p++;
	}
	if(ns=='n'&&ew=='e')
		return(ang);
	if(ns=='n'&&ew=='w')
		return(360-ang);
	if(ns=='s'&&ew=='e')
		return(180-ang);
	if(ns=='s'&&ew=='w')
		return(180+ang);
	if(ns!='n' && ns!='s')
		CorrectDialog(c2pstr((char*)str),(StringPtr)"\pQuad bearings begin with an �N� or �S�");
	else if(ew!='e' && ew!='w')
		CorrectDialog(c2pstr((char*)str),(StringPtr)"\pThis quad bearing has no �E� or �W�");
	else 
		CorrectDialog(c2pstr((char*)str),(StringPtr)"\pThis quad bearing makes no sense.");
	p2cstr(str);
	return 0;
}

void CheckBearing(StringPtr buf)
{
	char *s;
	extended ang;
	
	s=(char*)buf+1;
	*(s+*buf) = '\0';
	while(isspace(*s)) s++;
	ang = getAzi(s);
	if(ang<0 || ang>360)
		CorrectDialog(buf,(StringPtr)"\pBearings should be between 0 and 360�");
}

void CheckForesight(StringPtr buf1,StringPtr buf2)
{
	char *s;
	extended fs,bs;
	
	s=buf1+1;
	bs=(char*)getAzi(s);
	s=(char*)buf2+1;
	*(s+*buf2) = '\0';
	fs=getAzi(s);
	if(fabs((fabs(fs-bs)-180.0))>2.0)
		Correct2Dialog(buf2,buf1,(StringPtr)"\pBacksight does not agree with foresight",
				(StringPtr)"\pForesight:",(StringPtr)"\pBacksight:");
}

void CheckInclination(StringPtr buf)
{
	char *s;
	extended vang;
	
	s=(char*)buf+1;
	*(s+*buf) = '\0';
	while(isspace(*s)) s++;
	vang = atof(s);
	if(vang<-90 || vang>90)
		CorrectDialog(buf,(StringPtr)"\pInclinations should be between -90 and 90�");
}

void CheckForeIncl(StringPtr buf1, StringPtr buf2)
{
	char *s;
	extended fang,bang;
	
	s=(char*)buf1+1;
	bang=atof(s);
	s=(char*)buf2+1;
	*(s+*buf2) = '\0';
	fang=atof(s);
	if(fabs(fang+bang)>2.0)
		Correct2Dialog(buf2,buf1,(StringPtr)"\pInclinations disagree by more than 2�",
				(StringPtr)"\pForesight:",(StringPtr)"\pBacksight:");
}

void ProcessTitleItem(WindowPtr wind)
{
	short currentItem,itemType;
	Rect box;
	Handle itemHand;
	Str255 buf;

	currentItem = (short)((DocumentPeek)wind)->refCon;
	GetDItem(wind,currentItem,&itemType,&itemHand,&box);
	GetIText(itemHand,buf);
}

/*------------------------------------------------------------------
	General Solution
	Treatment requires knowing the column number. This could be done
	by brute force PtInRect searches, but that seems such a waste
	since we already have the dLog item number. Otherwise, for
	each col in the inverse MRPd array: For each column, we are
	in that column if:
	(1) factor == 1 (TabDown) and n is between colStart and 
		colStart+nSta.
	(2) factor > 1: (n-colStart) % colFactor)==0. 
	
	By the way, (n-colStart) % colFactor) != 0 gives the offset
	to the correct column, providing it is less than colFactor.

-------------------------------------------------------------------*/

#ifdef DO_CONFIGURE

void ProcessEntryItem(WindowPtr wind)
{
	short currentItem,nextItem,diff,row,itemType;
	int sta;
	Rect box;
	Handle itemHand,itemHand0;
	DEMapPtr map;
	Str255 buf,buf0;
	char *s;
	
	HLock(((DocumentPeek)wind)->docData);
	map = (DEMapPtr)*((DocumentPeek)wind)->docData;
	currentItem = ((DocumentPeek)wind)->refCon;
	if(map->fields[currentItem]<8) {
		GetDItem(wind,currentItem,&itemType,&itemHand,&box);
		GetIText(itemHand,buf);
		if(buf[0]) {
			switch(map->fields[currentItem-1]) {
				case 1:					/* from-station */
//				case 2:					/* to-station */
					if(buf[0]) {
						row = (currentItem - map->firstFields[1])/map->offsets[1];
						s=(char*)(buf+*buf);
						*(s+1)='\0';
						if(isdigit(*s)) {
							do { s--; } while(isdigit(*s));
							s++;
							sta = atoi(s);
							*s = '\0';
							sprintf((char*)buf,"%s%d",buf+1,sta+1);
							c2pstr((char*)buf);
							if(row<map->nStations) {
								nextItem=currentItem + map->offsets[1];
								GetDItem(wind,nextItem,&itemType,&itemHand,&box);
								SetIText(itemHand,buf);
							}
							if(map->firstFields[2]) {
								diff=map->firstFields[2]-map->firstFields[1];
								nextItem = currentItem+diff;
								GetDItem(wind,nextItem,&itemType,&itemHand,&box);
								SetIText(itemHand,buf);
							}
						}
					}
					break;
				case 3:						/* distance  */
					CheckDistance(buf);
					break;
				case 4:						/* fs */
					CheckBearing(buf);
					break;
				case 5:						/* bs */
					CheckBearing(buf);
					if(map->firstFields[4]) {
						row = (currentItem - map->firstFields[5])/map->offsets[5];
						currentItem = map->firstFields[4] + row*map->offsets[4];
						GetDItem(wind,currentItem,&itemType,&itemHand0,&box);
						GetIText(itemHand0,buf0);
						CheckForesight(buf,buf0);
						SetIText(itemHand0,buf0);
					}
					break;
				case 6:						/* fang */
					CheckInclination(buf);
					break;
				case 7:						/* bang */
					CheckInclination(buf);
					if(map->firstFields[6]) {
						row = (currentItem - map->firstFields[7])/map->offsets[7];
						currentItem = map->firstFields[6] + row*map->offsets[6];
						GetDItem(wind,currentItem,&itemType,&itemHand0,&box);
						GetIText(itemHand0,buf0);
						CheckForeIncl(buf,buf0);
						SetIText(itemHand0,buf0);
					}
					break;
				default:
					break;
			}
			SetIText(itemHand,buf);
		}
	}
	HUnlock(((DocumentPeek)wind)->docData);
}

#else

void ProcessEntryItem(WindowPtr wind)
{
	short currentItem,itemType;
	int sta;
	Rect box;
	Handle itemHand,itemHand0;
	Str255 buf,buf0;
	char *s;
	
	currentItem = (short)((DocumentPeek)wind)->refCon;
	/* process fields */
	if(currentItem<10) {				/* Station  */
		GetDItem(wind,currentItem,&itemType,&itemHand,&box);
		GetIText(itemHand,buf);
		if(buf[0]) {
			s=(char*)(buf+*buf);
			*(s+1)='\0';
			if(isdigit(*s)) {
				do { s--; } while(isdigit(*s));
				s++;
				sta = atoi(s);
				*s = '\0';
				sprintf((char*)buf,"%s%d",buf+1,sta+1);
				c2pstr((char*)buf);
				GetDItem(wind,currentItem+1,&itemType,&itemHand,&box);
				SetIText(itemHand,buf);
			}
		}
	} else if(currentItem<56) {			/* length, azi, or vang  */
		GetDItem(wind,currentItem,&itemType,&itemHand,&box);
		GetIText(itemHand,buf);
		if(buf[0]) {
			switch((currentItem-1)%5) {
				case 0:						/*	distance  */
					CheckDistance(buf);
					break;
				case 1:						/*	fs */
					CheckBearing(buf);
					break;
				case 2:						/*	bs */
					CheckBearing(buf);
					GetDItem(wind,currentItem-1,&itemType,&itemHand0,&box);
					GetIText(itemHand0,buf0);
					CheckForesight(buf,buf0);
					SetIText(itemHand0,buf0);
					break;
				case 3:						/*	fang */
					CheckInclination(buf);
					break;
				case 4:						/*	bang */
					CheckInclination(buf);
					GetDItem(wind,currentItem-1,&itemType,&itemHand0,&box);
					GetIText(itemHand0,buf0);
					CheckForeIncl(buf,buf0);
					SetIText(itemHand0,buf0);
					break;
				default:
					break;
			}
			SetIText(itemHand,buf);
		}
		
	} else if(currentItem<96) {			/* dimensions */
		;
	}
}
#endif

void DoTab(WindowPtr wind,short maxItem)
{
	short currentItem,nextItem,itemType;
	short dh=0,dv=0;
	Rect box,content;
	Handle itemHand;
#ifdef DO_CONFIGURE
	DEMapHandle eM;
	short fieldType;
	
	eM = (DEMapHandle) ((DocumentPeek)wind)->docData;
	currentItem = (short)((DocumentPeek)wind)->refCon;
	fieldType = (**eM).fields[currentItem];
	nextItem = currentItem + (**eM).offsets[fieldType];

#endif
	currentItem = (short)((DocumentPeek)wind)->refCon;
	nextItem = (currentItem==maxItem)? 1:currentItem+1;
	GetDItem(wind,nextItem,&itemType,&itemHand,&box);
	GetContentRect(wind,&content);
	if(box.top<content.top)
		dv = box.top - content.bottom;
	else if(box.bottom>content.bottom)
		dv = box.bottom - content.top;
	if(box.left<content.left)
		dh = content.left - box.left + content.left - content.right;
	else if(box.right>content.right)
		dh = content.right - box.right - content.left + content.right;
	if(dh||dv)
		CombinedScroll(wind,dh,dv);
}

void entryDoKeyDown(WindowPtr wind,EventRecord *event)
{	
	char ch;
	
	if((ch=event->message&charCodeMask)==3) 
		event->message += 6;		/* Make it a tab. */
	else if(ch==13) 
		event->message -= 4;		/* Make it a tab. */
	if((event->message&charCodeMask)==9) {
		ProcessEntryItem(wind);
#ifdef DO_CONFIGURE
		DoTab(wind,(**(DEMapHandle)((DocumentPeek)wind)->docData).nFields);
#else
		DoTab(wind,104);
#endif
	} else
		((DocumentPeek)wind)->fNeedtoSave = true;
	FocusOnContent(wind);
	DoDialogEvent(wind,event); 
}

void titleDoKeyDown(WindowPtr wind,EventRecord *event)
{	
	char ch;
	
	if((ch=event->message&charCodeMask)==3) 
		event->message += 6;		/* Make it a tab. */
	else if(ch==13) {
		event->message -= 4;		/* Make it a tab. */
	if((event->message&charCodeMask)==9) 
		ProcessTitleItem(wind);
		DoTab(wind,18);
	} else
		((DocumentPeek)wind)->fNeedtoSave = true;
	FocusOnContent(wind);
	DoDialogEvent(wind,event); 
}

short dlogGetVertSize(WindowPtr wind)
{
	DialogTHndl dR;
	
	dR = (DialogTHndl)GetResource('DLOG',((DocumentPeek)wind)->windResource);
	return (**dR).boundsRect.bottom - (**dR).boundsRect.top;
}

short dlogGetHorizSize(WindowPtr wind)
{
	DialogTHndl dR;
	
	dR = (DialogTHndl)GetResource('DLOG',((DocumentPeek)wind)->windResource);
	return (**dR).boundsRect.right - (**dR).boundsRect.left;
}

Boolean dlogMakeWindow(WindowPtr wind)
{
	DialogPtr dpDialog;
	short dialogID;
	unsigned char *title;
	
	SetDAFont(1);
	dialogID=((DocumentPeek)wind)->windResource;
	dpDialog=GetNewDialog(dialogID,(Ptr)wind,(WindowPtr)(-1));
	if(!dpDialog)
		return false;
	((DocumentPeek)dpDialog)->fDocWindow=true;
	NumWindows++;
	SetDAFont(0);
	if((wind->portRect.bottom+50)>DragBoundsRect.bottom)
		SizeWindow(wind,
			wind->portRect.right-wind->portRect.left,
			DragBoundsRect.bottom-DragBoundsRect.top-20,true);
	if(title=((DocumentPeek)wind)->fileSpecs.name)
		SetDocWindowTitle(wind,title);	
	return true;
}

#ifdef DO_CONFIGURE

void labelEntryPage(DialogPtr dialog)
{
	short i,nCols,j,left,fieldName,fieldType,staggeredType,staggered,top;
	short line1,line2,itemType,strNum=0;
	Handle itemHandle;
	MRPdHandle mH;
	DEMapHandle eM;
	Rect box;
	Str255 string;
	
	eM = (DEMapHandle)((DocumentPeek)dialog)->docData;
	mH = (MRPdHandle)GetResource('MRPd',(**eM).resID);
	
	nCols = (**mH).nColumns;
	TextSize(10);
	
	GetDItem(dialog,(**eM).nFields+3,&itemType,&itemHandle,&box);
	left = box.left+4;
	line1 = box.top+14;
	line2 = line1+14;
	staggeredType = (**eM).flags&fStaggered;
	
	for(i=0;i<nCols;i++) {
		fieldName = (**mH).colDescriptors[1+2*i];
		fieldType = fieldName&MenuMask;
		if(staggeredType) {
			switch (fieldName) {
				case 1: case 2: case 17:
					GetIndString(string,139,fieldName);
					MoveTo(left,line1);
					DrawString(string);
					break;
				case 3:
					GetIndString(string,139,3);
					MoveTo(left,line2);
					DrawString(string);
					break;
				case 4: case 5:
					GetIndString(string,139,fieldName);
					MoveTo(left,line1);
					DrawString(string);
				case 20:
					GetIndString(string,139,20);
					MoveTo(left,line2);
					DrawString(string);
					break;
				case 6: case 7:
					GetIndString(string,139,fieldName);
					MoveTo(left,line1);
					DrawString(string);
				case 22:
					GetIndString(string,139,22);
					MoveTo(left,line2);
					DrawString(string);
					break;
				case 8: case 9:
					GetIndString(string,139,fieldName);
					MoveTo(left,line1);
					DrawString(string);
				case 24:
					GetIndString(string,139,24);
					MoveTo(left,line2);
					DrawString(string);
					break;
				case 10: case 11: case 26: case 27:
					for(j=0;j<4;j++) {
						GetIndString(string,139,27+j);
						MoveTo(left,line1);
						DrawString(string);
						left += ((**mH).colDescriptors[2*i]&fNarrow)? cellNWidth+cellMargin : cellWidth+cellMargin ;
						if(i<nCols && ((**mH).colDescriptors[2*i]==(**mH).colDescriptors[2*(i+1)])) i++;
					}
					break;
				default:
					break;
			}
		} else {
			switch (fieldName) {
				case 1: case 2:
					GetIndString(string,139,17);
					MoveTo(left,line2);
					DrawString(string);
					GetIndString(string,139,fieldName);
					MoveTo(left,line1);
					DrawString(string);
					break;
				case 3:
					GetIndString(string,139,3);
					MoveTo(left,line2);
					DrawString(string);
					break;
				case 4: case 5:
					GetIndString(string,139,20);
					MoveTo(left,line2);
					DrawString(string);
					GetIndString(string,139,fieldName);
					MoveTo(left,line1);
					DrawString(string);
					break;
				case 6: case 7:
					GetIndString(string,139,22);
					MoveTo(left,line2);
					DrawString(string);
					GetIndString(string,139,fieldName);
					MoveTo(left,line1);
					DrawString(string);
					break;
				case 8: case 9:
					GetIndString(string,139,24);
					MoveTo(left,line2);
					DrawString(string);
					GetIndString(string,139,fieldName);
					MoveTo(left,line1);
					DrawString(string);
					break;
				case 10: case 11: case 26: case 27:
					for(j=0;j<4;j++) {
						GetIndString(string,139,27+j);
						MoveTo(left,line2);
						DrawString(string);
						left += ((**mH).colDescriptors[2*i]&fNarrow)? cellNWidth+cellMargin : cellWidth+cellMargin ;
						if(i<nCols && ((**mH).colDescriptors[2*i]==(**mH).colDescriptors[2*(i+1)])) i++;
					}
					i+=3;
					break;
				default:
					break;
			}
		}
		left += ((**mH).colDescriptors[2*(i)]&fNarrow)? cellNWidth+cellMargin : cellWidth+cellMargin ;
	}
	TextSize(12);
}

#else

void labelEntryPage(DialogPtr dialog)
{
	short itemType,strNum=0;
	Handle itemHandle;
	Rect box;
	Str255 string;
	
	TextSize(10);
	GetDItem(dialog,107,&itemType,&itemHandle,&box);
	MoveTo(box.left+5,box.top+14);
	GetIndString(string,139,++strNum);
	DrawString(string);						/* ("\pStation"); */
	MoveTo(box.left+223,box.top+14);
	GetIndString(string,139,++strNum);
	DrawString(string);						/* ("\pleft"); */
	MoveTo(box.left+256,box.top+14);
	GetIndString(string,139,++strNum);
	DrawString(string);						/* ("\pright"); */
	MoveTo(box.left+293,box.top+14);
	GetIndString(string,139,++strNum);
	DrawString(string);						/* ("\pfloor"); */
	MoveTo(box.left+328,box.top+14);
	GetIndString(string,139,++strNum);
	DrawString(string);						/* ("\pceil."); */
	MoveTo(box.left+57,box.top+33);
	GetIndString(string,139,++strNum);
	DrawString(string);						/* ("\pDist."); */
	MoveTo(box.left+104,box.top+33);
	GetIndString(string,139,++strNum);
	DrawString(string);						/* ("\pAzi."); */
	MoveTo(box.left+145,box.top+33);
	GetIndString(string,139,++strNum);
	DrawString(string);						/* ("\pIncl."); */
	TextSize(12);
}
#endif

void labelTitlePage(DialogPtr dialog)
{
	short itemType,strNum=0;
	Handle itemHandle;
	Rect box;
	Str255 string;
	
	TextSize(10);
	GetDItem(dialog,19,&itemType,&itemHandle,&box);
	MoveTo(box.left+5,box.top+8);
	GetIndString(string,140,++strNum);
	DrawString(string);						/*	("\pCave:"); */
	MoveTo(box.left+5,box.top+41);
	GetIndString(string,140,++strNum);
	DrawString(string);						/*	("\pArea:"); */
	MoveTo(box.left+5,box.top+80);
	GetIndString(string,140,++strNum);
	DrawString(string);						/*	("\pObjectives:"); */
	
	GetDItem(dialog,20,&itemType,&itemHandle,&box);
	MoveTo(box.left+5,box.top+13);
	GetIndString(string,140,++strNum);
	DrawString(string);						/*	("\pPersonnel:"); */
	
	GetDItem(dialog,22,&itemType,&itemHandle,&box);
	MoveTo(box.left+55,box.top+12);
	GetIndString(string,140,++strNum);
	DrawString(string);						/*	("\pCompass"); */
	MoveTo(box.left+8,box.top+28);
	GetIndString(string,140,++strNum);
	DrawString(string);						/*	("\pID:"); */
	MoveTo(box.left+2,box.top+61);
	GetIndString(string,140,++strNum);
	DrawString(string);						/*	("\pTest:"); */
	
	MoveTo(box.left+55,box.bottom-4);
	GetIndString(string,140,++strNum);
	DrawString(string);						/*	("\pClinometer"); */
	MoveTo(box.left+8,box.bottom+12);
	GetIndString(string,140,++strNum);
	DrawString(string);						/*	("\pID:"); */
	MoveTo(box.left+2,box.bottom+45);
	GetIndString(string,140,++strNum);
	DrawString(string);						/*	("\pTest:"); */
	
	GetDItem(dialog,21,&itemType,&itemHandle,&box);
	MoveTo(box.left+5,box.bottom-3);
	GetIndString(string,140,++strNum);
	DrawString(string);						/*	("\pDate:"); */
	TextSize(12);
	MoveTo(box.left+28,box.top+23);
	GetIndString(string,140,++strNum);
	DrawString(string);						/*	("\pFSB:"); */
}

#ifdef DO_CONFIGURE

Boolean entryMakeWindow(WindowPtr wind)
{
	short ID;
	char fields[44];
	DEMapHandle eM;
	MRPdHandle mH;
	Str255 templateName;
	
	if(gMapHandles && ((Handle *)*gMapHandles)[gCurrentTemplate]) {
		/* if there is a DEntryMap, then everything is there. */
		eM = (DEMapHandle)((Handle *)*gMapHandles)[gCurrentTemplate];
		mH = (MRPdHandle)GetResource('MRPd',(**eM).resID);
		((DocumentPeek)wind)->windResource = (**mH).DlogID;
		((DocumentPeek)wind)->docData = (Handle)eM;
	} else {
		/*  There is no DEntryMap. We either make it alone or we make it
			plus the dialog resources. */
		ID = GetTemplate(gCurrentTemplate,fields);
		mH = (MRPdHandle)GetResource('MRPd',ID);
		fields[3]=CalcNTemplateCols(fields);	/* It may need to be expanded. */
		if((**mH).DlogID) {
			((DocumentPeek)wind)->windResource = (**mH).DlogID;
			eM = CalcDEMap(fields);
			(**eM).resID = ID;
		} else {
			GetItem(SelectTemplateMenu,gCurrentTemplate,templateName);
			eM = (DEMapHandle)MakeDlogResource(fields,(Handle)mH,templateName);
			((DocumentPeek)wind)->windResource = (**mH).DlogID;
		}
		((DocumentPeek)wind)->docData = (Handle)eM;
	}
	
	
	return dlogMakeWindow(wind);
}

#endif

#if 0
Boolean titleMakeWindow(WindowPtr wind,char *)
{	
	return dlogMakeWindow(wind);
}
#endif

Boolean dlogInitDoc(DialogPtr dpDialog)
{
	DocumentPeek doc;
	short maxH,maxV;
	
	if(!InitScrollDoc(dpDialog))
		return false;
	
	doc = (DocumentPeek)dpDialog;
	
	maxH = GetHorizSize(dpDialog) + 16;
	maxV = GetVertSize(dpDialog) + 16;
	SetRect(&doc->limitRect,100,100,maxH,maxV);
	
	if(!(doc->fPrintRecord = (THPrint)NewHandle(sizeof(TPrint))))
		return false;
	PrOpen();
	PrintDefault(doc->fPrintRecord);
	PrClose();
	
	DrawGrowIcon(dpDialog);
	FocusOnContent(dpDialog);
	return true;
}

void DlogAdjustDocMenus(WindowPtr wind)
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
	
	/* Edit Menu */
	DisableItem(EditMenu,iUndo);
	EnableItem(EditMenu,iCut);
	EnableItem(EditMenu,iCopy);
	EnableItem(EditMenu,iPaste);
	EnableItem(EditMenu,iClear);
	DisableItem(EditMenu,iSelectAll);
	EnableItem(EditMenu,iShowClipboard);
	DisableItem(EditMenu,iFormat);
	
	/* Find Menu */
	DisableItem(FindMenu,iSelectSurvey);
	EnableItem(FindMenu,iFind);		
	DisableItem(FindMenu,iFindSame);
	SetMenuAbility(FindMenu,iFindSelection,doc->fHaveSelection);
	SetMenuAbility(FindMenu,iDisplaySelection,doc->fHaveSelection);
	DisableItem(FindMenu,iReplace);	
	DisableItem(FindMenu,iReplaceSame);	

	
	/* View Menu */
	/* 	Items initialized as enabled and should never
		be disabled by another document. */
	
	EnableItem(FileMenu,0);
	EnableItem(EditMenu,0);
	EnableItem(FindMenu,0);
	EnableItem(MarkMenu,0);
	EnableItem(WindowMenu,0);
	DisableItem(ViewMenu,0);
	DisableItem(ReportsMenu,0);
	EnableItem(DataMenu,0);
}

void entryAdjustDocMenus(WindowPtr wind)
{
	DlogAdjustDocMenus(wind);
	CheckItem(DataMenu,iDataPage,true);
	CheckItem(DataMenu,iTitlePage,false);
	/* Data Menu */
	SetMenuAbility(DataMenu,iNextPage,((DocumentPeek)wind)->associatedWindow);
	SetMenuAbility(DataMenu,iPreviousPage,((DocumentPeek)wind)->associatedWindow);
	SetMenuAbility(DataMenu,iFirstPage,((DocumentPeek)wind)->associatedWindow);
	SetMenuAbility(DataMenu,iLastPage,((DocumentPeek)wind)->associatedWindow);
}

void titleAdjustDocMenus(WindowPtr wind)
{
	DlogAdjustDocMenus(wind);
	CheckItem(DataMenu,iDataPage,false);
	CheckItem(DataMenu,iTitlePage,true);
	/* Data Menu */
	SetMenuAbility(DataMenu,iNextPage,((DocumentPeek)wind)->associatedWindow);
	DisableItem(DataMenu,iPreviousPage);
	DisableItem(DataMenu,iFirstPage);
	SetMenuAbility(DataMenu,iLastPage,((DocumentPeek)wind)->associatedWindow);
}

void ToggleDialog(WindowPtr wind, short id)
{
	DialogTHndl dR;
	Rect r,oldR;
	Point pt;
	
	dR = (DialogTHndl)GetResource('DLOG',139);
	if(((DocumentPeek)wind)->fDocWindow) {
		/* There is a window open. Is it the right kind? */
		if(((DocumentPeek)wind)->windResource == id) 
			return;
		((DocumentPeek)wind)->windResource = id;
		if(id==139) {
			if(((DocumentPeek)wind)->fNeedtoSave)
				Title2CML(wind,AppScratch);
			((DocumentPeek)wind)->adjustDocMenus=entryAdjustDocMenus;
			((DocumentPeek)wind)->doKeyDown=entryDoKeyDown;
/*			((DocumentPeek)wind)->makeWindow=entryMakeWindow; */
			((DocumentPeek)wind)->draw=entryDlogDraw;
			((DocumentPeek)wind)->doDocMenuCommand=entryDoDocMenuCommand;
			setDocName((DocumentPeek)wind,"\pData Page");
		} else {
			if(((DocumentPeek)wind)->fNeedtoSave)
				Entry2CML(wind,AppScratch);
			((DocumentPeek)wind)->adjustDocMenus=titleAdjustDocMenus;
			((DocumentPeek)wind)->doKeyDown=titleDoKeyDown;
/*			((DocumentPeek)wind)->makeWindow=titleMakeWindow; */
			((DocumentPeek)wind)->draw=titleDlogDraw;
			((DocumentPeek)wind)->doDocMenuCommand=titleDoDocMenuCommand;
			setDocName((DocumentPeek)wind,"\pTitle Page");
		}
		((DocumentPeek)wind)->fNeedtoSave=false;
		r = wind->portRect;
		SetPt(&pt,r.left,r.top);
		SetPort(wind);
		LocalToGlobal(&pt);
		OffsetRect(&r,pt.h,pt.v);
		oldR = (**dR).boundsRect;
		(**dR).boundsRect = r;
		ShowHide(wind,false);
		CloseDialog(wind);
		NumWindows--;
		memset(wind,0,sizeof(DialogRecord));

		if((*((DocumentPeek)wind)->makeWindow)(wind)) {
			(**dR).boundsRect = oldR;
			if((*((DocumentPeek)wind)->initDoc)(wind))
				ShowDocWindow(wind);
		}
	}
}

Boolean dlogDoDocMenuCommand(WindowPtr wind, short menuID, short menuItem, short x)
{	
#ifdef MC68000
#pragma unused (x)
#endif
	if(doDocMenuCommand(wind,menuID,menuItem,0))
		return true;
		
	switch(menuID) {
		case DATA_ID:
			switch(menuItem) {
				case iTitlePage:
					ToggleDialog(wind,140);
					return true;
				case iDataPage:
					ToggleDialog(wind,139);
					return true;
				default:
					break;
			}
			break;
		default:
			break;
	}
	return false;
}

Boolean titleDoDocMenuCommand(WindowPtr wind, short menuID, short menuItem, short x)
{
#ifdef MC68000
#pragma unused (x)
#endif
	if(dlogDoDocMenuCommand(wind,menuID,menuItem,0))
		return true;
	switch(menuID) {
		case DATA_ID:
			switch(menuItem) {
				case iNextPage:
					ToggleDialog(wind,139);
					return true;
				default:
					break;
			}
			break;
		default:
			break;
	}
	return false;
}

Boolean entryDoDocMenuCommand(WindowPtr wind, short menuID, short menuItem, short x)
{
#ifdef MC68000
#pragma unused (x)
#endif
	if(dlogDoDocMenuCommand(wind,menuID,menuItem,0))
		return true;
	switch(menuID) {
		case DATA_ID:
			switch(menuItem) {
				case iNextPage:
					if(((DocumentPeek)wind)->fNeedtoSave)
						Entry2CML(wind,AppScratch);
					return true;
				default:
					break;
			}
			break;
		default:
			break;
	}
	return false;
}

void dlogDoCut(WindowPtr dialog)
{
	Handle TEData;
	OSErr err;

	DlgCut((DialogPtr)dialog);
	((DocumentPeek)dialog)->fNeedtoSave=true;
	TEData = TEScrapHandle();
	err = HandToHand(&TEData);
	if(err==noErr) {
		if(TE32KScrpHandle)
			DisposeHandle(TE32KScrpHandle);
		TE32KScrpHandle = TEData;
		TE32KScrpLength = TEGetScrapLen();
	}
}

void dlogDoPaste(WindowPtr dialog)
{
	Handle *TEScrapHandle;
	
	if(TE32KScrpLength<256) {
		TESetScrapLen(TE32KScrpLength);
		TEScrapHandle = (Handle *)TEScrpHandle;
		*TEScrapHandle = TE32KScrpHandle;
		
		DlgPaste((DialogPtr)dialog);
		((DocumentPeek)dialog)->fNeedtoSave=true;
		((DocumentPeek)dialog)->fHaveSelection=true;
	} else
		doMessage(19);
}

void dlogDoClear(WindowPtr dialog)
{
	DlgDelete((DialogPtr)dialog);
	((DocumentPeek)dialog)->fNeedtoSave=true;
}

void dlogDoCopy(WindowPtr dialog)
{
	Handle TEData;
	OSErr err;
	
	DlgCopy((DialogPtr)dialog);
	TEData = TEScrapHandle();
	err = HandToHand(&TEData);
	if(err==noErr) {
		if(TE32KScrpHandle)
			DisposeHandle(TE32KScrpHandle);
		TE32KScrpHandle = TEData;
		TE32KScrpLength = TEGetScrapLen();
	}
}

void AddDlogMemberFunctions(DocumentPeek doc)
{
	doc->makeWindow=dlogMakeWindow;
	doc->initDoc=dlogInitDoc;
	doc->destructor=dlogDestructor;
	doc->doIdle=dlogDoIdle;
	doc->docData=0;
	doc->draw=dlogDraw;
	doc->doPageSetup=doPageSetup;
	doc->doPrint=mapDoPrint;
	doc->doGrow=scrollDoGrow;
	doc->doActivate=dlogDoActivate;
	doc->doTheUpdate=dlogDoTheUpdate;
	doc->doContent=dlogDoContent;
	doc->doCut=dlogDoCut;
	doc->doPaste=dlogDoPaste;
	doc->doClear=dlogDoClear;
	doc->doCopy=dlogDoCopy;
	doc->getVertSize=dlogGetVertSize;
	doc->getHorizSize=dlogGetHorizSize;
	doc->doDocMenuCommand=dlogDoDocMenuCommand;
}

void AddTitleMemberFunctions(DocumentPeek doc)
{
	AddDlogMemberFunctions(doc);
	doc->adjustDocMenus=titleAdjustDocMenus;
	doc->draw=titleDlogDraw;
	doc->doKeyDown=titleDoKeyDown;
	doc->doDocMenuCommand=titleDoDocMenuCommand;
}

void AddEntryMemberFunctions(DocumentPeek doc)
{
	AddDlogMemberFunctions(doc);
	doc->makeWindow=entryMakeWindow;
	doc->adjustDocMenus=entryAdjustDocMenus;
	doc->draw=entryDlogDraw;
	doc->doKeyDown=entryDoKeyDown;
	doc->doDocMenuCommand=entryDoDocMenuCommand;
}

DocumentPeek MakeModelessDialog(long dialogType)
{
	DocumentPeek doc;

	if(doc = MakeDoc(dialogType,0)) {
		if(((*doc->makeWindow)((WindowPtr)doc)) && ((*doc->initDoc)((WindowPtr)doc))) {
			ShowDocWindow((WindowPtr)doc);
			return doc;
		} else
			DeleteDoc(doc);
	}
	return nil;
}

static void Title2CML(WindowPtr dialog,Handle scratchHandle)
{
	char *s;
	int k;
	Str255 buf;
	short i,itemType;
	Handle itemHandle;
	Rect box;
	
	if(!((DocumentPeek)dialog)->associatedWindow) 
		return;

	HLock(scratchHandle);
	s = (char *)*scratchHandle;
	
		
	GetDItem(dialog,12,&itemType,&itemHandle,&box);
	GetIText(itemHandle,buf);
	if(!buf[0]) 
		CorrectDialog(buf,(StringPtr)"\pDon't you want to give an FSB number?");
	k=sprintf(s,";--------------------- %s ------------------\n",p2cstr(buf));
	k+=sprintf(s+k,"#openFSB %s\n",buf);
	GetDItem(dialog,13,&itemType,&itemHandle,&box);
	GetIText(itemHandle,buf);
	if(buf[0])
		k+=sprintf(s+k,"#date %s\n",p2cstr(buf));
	GetDItem(dialog,1,&itemType,&itemHandle,&box);
	GetIText(itemHandle,buf);
	if(buf[0])
		k+=sprintf(s+k,"#cave %s\n",p2cstr(buf));
	GetDItem(dialog,2,&itemType,&itemHandle,&box);
	GetIText(itemHandle,buf);
	if(buf[0])
		k+=sprintf(s+k,"#location %s\n",p2cstr(buf));
	GetDItem(dialog,3,&itemType,&itemHandle,&box);
	GetIText(itemHandle,buf);
	if(buf[0]) 
		k+=sprintf(s+k,"#objective #begin\n %s\n#end\n",p2cstr(buf));
	GetDItem(dialog,4,&itemType,&itemHandle,&box);
	GetIText(itemHandle,buf);
	if(buf[0]) {
		k+=sprintf(s+k,"#personnel %s",p2cstr(buf));
		for(i=5;i<=9;i++) {
			GetDItem(dialog,i,&itemType,&itemHandle,&box);
			GetIText(itemHandle,buf);
			if(buf[0]) 
				k+=sprintf(s+k," %s",p2cstr(buf));
		}
		k+=sprintf(s+k,"\n");
	}
	GetDItem(dialog,10,&itemType,&itemHandle,&box);
	GetIText(itemHandle,buf);
	if(buf[0])
		k+=sprintf(s+k,"#in %s\n",p2cstr(buf));
	GetDItem(dialog,11,&itemType,&itemHandle,&box);
	GetIText(itemHandle,buf);
	if(buf[0])
		k+=sprintf(s+k,"#out %s\n",p2cstr(buf));
	GetDItem(dialog,15,&itemType,&itemHandle,&box);
	GetIText(itemHandle,buf);
	if(buf[0])
		k+=sprintf(s+k,"#compass %s\n",p2cstr(buf));
	GetDItem(dialog,17,&itemType,&itemHandle,&box);
	GetIText(itemHandle,buf);
	if(buf[0])
		k+=sprintf(s+k,"#incl %s\n",p2cstr(buf));
	k+=sprintf(s+k,";\n(&1)\n");
	AddText(((DocumentPeek)dialog)->associatedWindow,s,k);
	HUnlock(scratchHandle);
}

/*----------------------------------------------------------------------
	General solution.
	
	The MRPd resource for any template contains sufficient information
	to generate the template dialog and allow data entered in it to be 
	translated into CML. Entry2CML requires an inverted MRPd resource
	where field types are related to data page columns. We can use the
	same sort of 2d byte array as is used in the MRPd resource, with
	column number and a factor byte as the two elements. Column number
	is the data page column for a particular field type. The factor is
	the offset between like entries of the same type: 1 for a TabDown
	data-entry column and the number of non-blank rows till the next
	TabBack column. For the CRF data template, it would look like this:
	(1,1),(11,5),(12,5),(13,5),(14,5),(15,5),(56,4). 56 is: 10+5*9+1.

----------------------------------------------------------------------*/

#ifdef DO_CONFIGURE

static void Entry2CML(WindowPtr dialog, Handle scratchHandle)
{
	char *s;
	Str63 buf,lastStation;
	short dItem;
	short row,itemType;
	short nShots;
	Handle itemHandle;
	Rect box;
	int i,k,walls;
	DEMapPtr map;
	
	if(!((DocumentPeek)dialog)->associatedWindow) 
		return;

	HLock(scratchHandle);
	s = (char *)*scratchHandle;
	lastStation[0]=0;
	HLock(((DocumentPeek)dialog)->docData);
	map = (DEMapPtr)*((DocumentPeek)dialog)->docData;
	
	nShots = map->nStations;
	if(map->flags&fStaggered)
		nShots--;
	
	for(k=0,row=0;row<nShots;row++ ) {
		/* 1. Get station field */
		dItem = map->firstFields[1] + row*map->offsets[1];
		GetDItem(dialog,dItem,&itemType,&itemHandle,&box);
		GetIText(itemHandle,buf);
		if(buf[0]) {
			GetIText(itemHandle,lastStation);
			p2cstr(buf);
			k+=sprintf(s+k,"%s",buf);
			buf[0]=0;
			SetIText(itemHandle,buf);
			if(map->firstFields[2]) {	/* There is a to-station. */
				dItem = map->firstFields[2] + row*map->offsets[2];
				GetDItem(dialog,dItem,&itemType,&itemHandle,&box);
				GetIText(itemHandle,buf);
				if(buf[0]) {
					GetIText(itemHandle,lastStation);
					p2cstr(buf);
					k+=sprintf(s+k,",%s",buf);
					buf[0]=0;
					SetIText(itemHandle,buf);
				}
			}
			k+=sprintf(s+k,":\t");
			
			/* 2. Get distance */
			dItem = map->firstFields[3] + row*map->offsets[3];
			GetDItem(dialog,dItem,&itemType,&itemHandle,&box);
			GetIText(itemHandle,buf);
			if(buf[0]) {
				p2cstr(buf);
				k+=sprintf(s+k,"%s\t",buf);
				buf[0]=0;
				SetIText(itemHandle,buf);
				
				/* 3. Get remaining shot data */
				if(map->firstFields[4]) {
					dItem = map->firstFields[4] + row*map->offsets[4];
					GetDItem(dialog,dItem,&itemType,&itemHandle,&box);
					GetIText(itemHandle,buf);
					if(buf[0]) {
						p2cstr(buf);
						k+=sprintf(s+k,"%s",buf);			/* fs */
						buf[0]=0;
						SetIText(itemHandle,buf);
					}
				}
				if(map->firstFields[5]) {
					dItem = map->firstFields[5] + row*map->offsets[5];
					GetDItem(dialog,dItem,&itemType,&itemHandle,&box);
					GetIText(itemHandle,buf);
					if(buf[0]) {
						p2cstr(buf);
						k+=sprintf(s+k,",%s\t",buf);		/* bs */
						buf[0]=0;
						SetIText(itemHandle,buf);
					} else
						k+=sprintf(s+k,"\t");
				}
				if(map->firstFields[6]) {
					dItem = map->firstFields[6] + row*map->offsets[6];
					GetDItem(dialog,dItem,&itemType,&itemHandle,&box);
					GetIText(itemHandle,buf);
					if(buf[0]) {
						p2cstr(buf);
						k+=sprintf(s+k,"%s",buf);			/* fang */
						buf[0]=0;
						SetIText(itemHandle,buf);
					}
				}
				if(map->firstFields[7]) {
					dItem = map->firstFields[7] + row*map->offsets[7];
					GetDItem(dialog,dItem,&itemType,&itemHandle,&box);
					GetIText(itemHandle,buf);
					if(buf[0]) {
						p2cstr(buf);
						k+=sprintf(s+k,",%s\t",buf);		/* bang */
						buf[0]=0;
						SetIText(itemHandle,buf);
					} else
						k+=sprintf(s+k,"\t");
				}
			} else
				k+=sprintf(s+k,"..\t\t\t");
			/* 4. Get wall dimensions */
			if(map->firstFields[10]) {
				walls=0;
				for(i=10;i<14;i++) {
					dItem = map->firstFields[i] + row*map->offsets[i];
					GetDItem(dialog,dItem,&itemType,&itemHandle,&box);
					GetIText(itemHandle,buf);
					if(buf[0]) {
						p2cstr(buf);
						k+=sprintf(s+k,"%s",buf);
						walls=1;
						buf[0]=0;
						SetIText(itemHandle,buf);
					}
					if(i<14)
						k+=sprintf(s+k,",");
				}
				if(!walls) {
					k-=4;
					while(isspace(s[k])) k--;
				} else
					k+=sprintf(s+k,"\t");
			}
			if(map->firstFields[14]) {
				walls=0;
				for(i=14;i<18;i++) {
					dItem = map->firstFields[i] + row*map->offsets[i];
					GetDItem(dialog,dItem,&itemType,&itemHandle,&box);
					GetIText(itemHandle,buf);
					if(buf[0]) {
						p2cstr(buf);
						k+=sprintf(s+k,"%s",buf);
						walls=1;
						buf[0]=0;
						SetIText(itemHandle,buf);
					}
					if(i<18)
						k+=sprintf(s+k,",");
				}
				if(!walls) {
					k-=4;
					while(isspace(s[k])) k--;
				}
			}
			s[k++]='\n';
		}
	}
	dItem = map->nFields+2;
	GetDItem(dialog,dItem,&itemType,&itemHandle,&box);
	GetIText(itemHandle,buf);
	p2cstr(buf);
	i=atoi((char*)buf);
	sprintf((char*)buf,"%d",i+1);
	k+=sprintf(s+k,"(&%s)\n",(char*)buf);
	c2pstr((char*)buf);
	SetIText(itemHandle,buf);
	AddText(((DocumentPeek)dialog)->associatedWindow,s,k);
	HUnlock(scratchHandle);
	GetDItem(dialog,1,&itemType,&itemHandle,&box);
	SetIText(itemHandle,lastStation);
	SelIText(dialog,1,0,32767);
	HUnlock(((DocumentPeek)dialog)->docData);
}

#else
static void Entry2CML(WindowPtr dialog, Handle scratchHandle)
{
	char *s;
	Str63 buf,lastStation;
	short row,col,itemType;
	Handle itemHandle;
	Rect box;
	int i,k,walls;
	
	if(!((DocumentPeek)dialog)->associatedWindow) 
		return;

	HLock(scratchHandle);
	s = (char *)*scratchHandle;
	lastStation[0]=0;
	
	for(k=0,row=0;row<9;row++) {
		/* 1. Get station field */
		walls=0;
		GetDItem(dialog,row+1,&itemType,&itemHandle,&box);
		GetIText(itemHandle,buf);
		if(buf[0]) {
			GetIText(itemHandle,lastStation);
			p2cstr(buf);
			k+=sprintf(s+k,"%s:\t",buf);
			buf[0]=0;
			SetIText(itemHandle,buf);
			/* 2. Get distance */
			col=row*5;
			GetDItem(dialog,col+11,&itemType,&itemHandle,&box);
			GetIText(itemHandle,buf);
			if(buf[0]) {
				p2cstr(buf);
				k+=sprintf(s+k,"%s\t",buf);
				buf[0]=0;
				SetIText(itemHandle,buf);
				/* 3. Get remaining shot data */
				GetDItem(dialog,col+12,&itemType,&itemHandle,&box);
				GetIText(itemHandle,buf);
				if(buf[0]) {
					p2cstr(buf);
					k+=sprintf(s+k,"%s",buf);			/* fs */
					buf[0]=0;
					SetIText(itemHandle,buf);
				}
				GetDItem(dialog,col+13,&itemType,&itemHandle,&box);
				GetIText(itemHandle,buf);
				if(buf[0]) {
					p2cstr(buf);
					k+=sprintf(s+k,",%s\t",buf);			/* bs */
					buf[0]=0;
					SetIText(itemHandle,buf);
				} else
					k+=sprintf(s+k,"\t");
				GetDItem(dialog,col+14,&itemType,&itemHandle,&box);
				GetIText(itemHandle,buf);
				if(buf[0]) {
					p2cstr(buf);
					k+=sprintf(s+k,"%s",buf);			/* fang */
					buf[0]=0;
					SetIText(itemHandle,buf);
				}
				GetDItem(dialog,col+15,&itemType,&itemHandle,&box);
				GetIText(itemHandle,buf);
				if(buf[0]) {
					p2cstr(buf);
					k+=sprintf(s+k,",%s\t",buf);			/* bang */
					buf[0]=0;
					SetIText(itemHandle,buf);
				} else
					k+=sprintf(s+k,"\t");
			} else
				k+=sprintf(s+k,"..");
			/* 4. Get wall dimensions */
			col=row*4;
			for(i=0;i<4;i++) {
				GetDItem(dialog,col+56+i,&itemType,&itemHandle,&box);
				GetIText(itemHandle,buf);
				if(buf[0]) {
					p2cstr(buf);
					k+=sprintf(s+k,"%s",buf);
					walls=1;
					buf[0]=0;
					SetIText(itemHandle,buf);
				}
				if(i<3)
					k+=sprintf(s+k,",");
			}
			if(!walls) {
				k-=3;
				while(isspace(s[k])) k--;
			}
			s[k++]='\n';
		}
	}
	GetDItem(dialog,106,&itemType,&itemHandle,&box);
	GetIText(itemHandle,buf);
	p2cstr(buf);
	i=atoi((char*)buf);
	sprintf((char*)buf,"%d",i+1);
	k+=sprintf(s+k,"(&%s)\n",(char*)buf);
	c2pstr((char*)buf);
	SetIText(itemHandle,buf);
	AddText(((DocumentPeek)dialog)->associatedWindow,s,k);
	HUnlock(scratchHandle);
	GetDItem(dialog,1,&itemType,&itemHandle,&box);
	SetIText(itemHandle,lastStation);
	SelIText(dialog,1,0,32767);
}
#endif

/*------------------------ Template Dialog Routines -----------------------*/

void DrawArrowHead(int direction)
{
	Rect r;
	Point pt;
	
	PenSize(1,1);
	
	GetPen(&pt);
	SetRect(&r,pt.h-3,pt.v-3,pt.h+3,pt.v+3);
	EraseRect(&r);
	if(direction==1) {		/* Tab Down */
		Move(0,-2);
		Line(3,0);
		Line(-3,3);
		Line(-3,-3);
		Line(3,0);
	} else if(direction==0) {
		Move(-2,0);
		Line(0,-3);
		Line(3,3);
		Line(-3,3);
		Line(0,-3);
	} else {
		Move(1,0);
		Line(0,-3);
		Line(-3,3);
		Line(3,3);
		Line(0,-3);
	}
}

pascal void drawTemplate(DialogPtr wind,short x)
{
#ifdef MC68000
#pragma unused (x)
#endif
	unsigned char *fields;
	int i,j;
	int width,narrow,top,bottom,height=12;
	Boolean staggered,inDims=0;
	short itemType,fieldType;
	Handle itemHand;
	Rect box;
	Str255 numStr;
	
	fields=(unsigned char *)GetWRefCon(wind);
	GetDItem(wind,NumberOfColumnsItem,&itemType,&itemHand,&box);
	GetIText(itemHand,numStr);
	StringToNum(numStr,&i);
	if(i<3 || i>20) {
		i=fields[3];
		NumToString(i,numStr);
		SetIText(itemHand,numStr);
	} else
		fields[3]=i;
	GetDItem(wind,TemplateItem,&itemType,&itemHand,&box);
	PenSize(1,1);
	TextFont(applFont);
	TextSize(9);
	EraseRect(&box);
	FrameRect(&box);
	width=box.right-box.left-20;
	width/=fields[3];			/* nCols */
	narrow=(width*2)/3;
	top=box.top+20;
	bottom=top+48;
	box.right=box.left+11;
	staggered=*fields&fStaggered;
	for(i=0;i<2*fields[3];i+=2) {
		box.left=box.right-1;
		box.right+=(*(fields+4+i)&fNarrow)? narrow:width;
		if(!inDims) {
			box.bottom=bottom;
			box.top=top;
			if(staggered) {
				if(*(fields+1)&fBlank) 
					j=(i<1 || (i+5)>fields[3])? 1:3;
				else
					j=*(fields+5+i)&MenuMask;
				if(!j || (j>2 && j<10)) {
					box.top+=6;
					box.bottom-=6;
				}
			}
		}
		FrameRect(&box);

		if(*(fields+4+i)&fSplit) {
			for(j=box.top+height-6;j<box.bottom;j+=height) {
				MoveTo(box.left,j);
				LineTo(box.right-1,j);
			}
		}
		for(j=box.top+height;j<box.bottom;j+=height) {
			MoveTo(box.left,j);
			LineTo(box.right-1,j);
			MoveTo((box.left+box.right)/2,j-height/2);
			DrawArrowHead((*(fields+4+i)&TabMask));
		}
		
		MoveTo((box.left+box.right)/2,j-height/2);
		DrawArrowHead((*(fields+4+i)&TabMask));
		if((i/2+1)==fields[2]) {
			InsetRect(&box,2,2);
			InvertRect(&box);
			InsetRect(&box,-2,-2);
		}
		fieldType=*(fields+5+i);
		if(inDims) {
			inDims--;
			MoveTo((box.left+box.right)/2-3,top-3);
			DrawChar(*(numStr+4-inDims));
		} else if((fieldType&MenuMask)==10 || (fieldType&MenuMask)==11) {
			inDims=3;
			GetIndString(numStr,144,27);
			MoveTo((box.left+box.right)/2-3,top-3);
			DrawChar(*(numStr+1));
		} else {
			GetIndString(numStr,144,fieldType);
			MoveTo((box.left+box.right)/2-6,top-3);
			DrawString(numStr);
		}
	}
	TextFont(systemFont);
	TextSize(12);
}

pascal Boolean configFilter(DialogPtr dialog,EventRecord *theEvent,short *itemHit)
{
	Point mouseLoc,menuPt;
	Handle itemHand,strHand;
	Rect box;
	short itemType,item,oldItem;
	Boolean staggered;
	char *fields,chr,*p;
	int i,j,temp,found;
	int width,narrow,top,bottom,height=12;
	
	/* c1 relates to the template cartoon */
	static char c1=0;
	Str31 menuLabel;

	fields=(char *)GetWRefCon(dialog);
	SetPort(dialog);
	if(theEvent->what==mouseDown) {
		c1=0;
		mouseLoc=theEvent->where;
		GlobalToLocal(&mouseLoc);
		item=1+FindDItem(dialog,mouseLoc);
		/* Every mouseDown causes the template and popup menu to be redrawn */
		GetDItem(dialog,PopItem,&itemType,&itemHand,&box);
		InvalRect(&box);
		GetDItem(dialog,TemplateItem,&itemType,&itemHand,&box);
		InvalRect(&box);
		staggered=*fields&fStaggered;
		switch(item) {
			case TemplateItem:
				width=box.right-box.left-20;
				width/=fields[3];			/* nCols */
				narrow=(width*2)/3;
				top=box.top+20;
				bottom=top+48;
				box.right=box.left+11;
				for(i=0;i<2*fields[3];i+=2) {
					box.left=box.right-1;
					box.right+=(*(fields+4+i)&fNarrow)? narrow:width;
					box.bottom=bottom;
					box.top=top;
					if(staggered) {
						if(*(fields+1)&fBlank) 
							j=(i<1 || (i+5)>fields[3])? 1:3;
						else
							j=*(fields+5+i)&MenuMask;
							
						if(!j || (j>2 && j<10)) {
							box.top+=6;
							box.bottom-=6;
						}
					}
					if(PtInRect(mouseLoc,&box)) {
						fields[2]=i/2+1;
						return true;
					}
				}
				break;
			case PopItem:
				if(fields[2]>0) {					
					if(staggered) {
						GetIndString(menuLabel,143,17);
						SetItem(DataFieldsMenu,2,menuLabel);
						GetIndString(menuLabel,143,24);
						SetItem(DataFieldsMenu,9,menuLabel);
						GetIndString(menuLabel,143,26);
						SetItem(DataFieldsMenu,11,menuLabel);
						DisableItem(DataFieldsMenu,3);
						DisableItem(DataFieldsMenu,10);
					} else {
						GetIndString(menuLabel,143,1);
						SetItem(DataFieldsMenu,2,menuLabel);
						GetIndString(menuLabel,143,10);
						SetItem(DataFieldsMenu,11,menuLabel);
						GetIndString(menuLabel,143,8);
						SetItem(DataFieldsMenu,9,menuLabel);
						
						EnableItem(DataFieldsMenu,3);
						EnableItem(DataFieldsMenu,10);
					}
					SetMenuAbility(DataFieldsMenu,11,(fields[2]+3)<=fields[3]);
					SetMenuAbility(DataFieldsMenu,12,((fields[2]+3)<=fields[3])&&!(fields[2+2*fields[2]]&fSplit));
					if(fields[2+2*fields[2]]&fSplit) {
						GetIndString(menuLabel,143,20);
						SetItem(DataFieldsMenu,5,menuLabel);
						GetIndString(menuLabel,143,22);
						SetItem(DataFieldsMenu,7,menuLabel);
						DisableItem(DataFieldsMenu,6);
						DisableItem(DataFieldsMenu,8);
					} else {
						GetIndString(menuLabel,143,4);
						SetItem(DataFieldsMenu,5,menuLabel);
						GetIndString(menuLabel,143,6);
						SetItem(DataFieldsMenu,7,menuLabel);
						EnableItem(DataFieldsMenu,6);
						EnableItem(DataFieldsMenu,8);
					}

					oldItem=1+(fields[3+2*fields[2]]&MenuMask);
					CheckItem(DataFieldsMenu,oldItem,true);
					GetDItem(dialog,PopItem,&itemType,&itemHand,&box);
					menuPt.h=box.left;
					menuPt.v=box.top;
					LocalToGlobal(&menuPt);
					GetDItem(dialog,PopItem+1,&itemType,&itemHand,&box);
					InvertRect(&box);
					CalcMenuSize(DataFieldsMenu);
					item=PopUpMenuSelect(DataFieldsMenu,menuPt.v,menuPt.h,(short)oldItem);
					InvertRect(&box);
					if (item--) {
						temp=(AltBits>>((item-1)<<1))&3;
						i=2+2*fields[2];				/* index of selected row */
						if(temp&3)
							if((staggered&&(temp&2))||((temp&1)&&(fields[i]&fSplit)))
								item|=fAlt;
						fields[i+1]=item;
						RemoveDuplicates(fields);
						checkCDialogConsistency(fields);
					}
					CheckItem(DataFieldsMenu,oldItem,false);
					GetDItem(dialog,confMakeButton,&itemType,&itemHand,&box);
					HiliteControl((ControlHandle)itemHand,0);
					/* Once a change has been made, you can no longer delete. */
					GetDItem(dialog,confDeleteButton,&itemType,&itemHand,&box);
					HiliteControl((ControlHandle)itemHand,255);
				}
				return true;
			case 5:
				fields[2]=-1;
				return false;			/* Handle event */
			case 7:
				fields[2]=-2;
				return false;
			case TemplateTitleItem:
				fields[2]=-3;
				return false;
			default:
				break;
		}
	} else if(theEvent->what==keyDown) {
		chr=theEvent->message&charCodeMask;
		switch(chr) {
			case 3: case 13:		/* Enter or CR */
				GetDItem(dialog,1,&itemType,&itemHand,&box);
				HiliteControl((ControlHandle)itemHand,1);
				*itemHit=1;
				return true;
			case 46:				/* Period */
				if(theEvent->modifiers&0x100) {
			case 27:				/* Escape */
					GetDItem(dialog,2,&itemType,&itemHand,&box);
					HiliteControl((ControlHandle)itemHand,2);
					*itemHit=2;
					return true;
				}
			case 9:						/* Tab */
				c1=0;
				temp=(theEvent->modifiers&shiftKey)? -1:1;
				fields[2]+=temp;
				if(!fields[2])			/* nil means nothing would be selected */
					fields[2]+=temp;
				if(fields[2]>fields[3])
					fields[2]=-3;
				if(fields[2]<-3)
					fields[2]=fields[3];
				if(fields[2]>0)
					SelIText(dialog,NumberOfColumnsItem,0,0);
				else if(fields[2]==-1) 
					SelIText(dialog,TemplateTitleItem,0,32767);
				else if(fields[2]==-2)
					SelIText(dialog,NumberOfColumnsItem,0,32767);
				else if(fields[2]==-3)
					SelIText(dialog,StationsPerPageItem,0,32767);

				GetDItem(dialog,TemplateItem,&itemType,&itemHand,&box);
				InsetRect(&box,1,1);
				EraseRect(&box);
				InvalRect(&box);
				GetDItem(dialog,PopItem,&itemType,&itemHand,&box);
				EraseRect(&box);
				InvalRect(&box);
				*itemHit=TemplateItem;
				return true;
			default:
				GetDItem(dialog,confMakeButton,&itemType,&itemHand,&box);
				HiliteControl((ControlHandle)itemHand,0);
				/* Once a change has been made, you can no longer delete. */
				GetDItem(dialog,confDeleteButton,&itemType,&itemHand,&box);
				HiliteControl((ControlHandle)itemHand,255);
				
				if(fields[2]>0) {		/* A field in the template cartoon is selected */
					if(isalpha(chr)) {
						if(strHand=GetResource('STR#',144)) {
							HLock(strHand);
							if(*((short *)*strHand)==27) {	/* Some RezEditor hasn't screwed up the number of strings. */
								p=(char *)*strHand+2;
								chr=tolower(chr);
								found=0;
								for(i=0;i<26;i++,p+=*p+1) {
									if(c1) {
										if(c1==tolower(*(p+1)) && chr==tolower(*(p+2))) {
											found=i+1;
											break;
										}
									} else if(chr==tolower(*(p+1))) {
										if(found) {
											c1=chr;
											HUnlock(strHand);
											return true;
										} else 
											found=i+1;
									}
								}
								if(found) {
									fields[3+2*fields[2]]=found ;
									RemoveDuplicates(fields);
									checkCDialogConsistency(fields);
									GetDItem(dialog,TemplateItem,&itemType,&itemHand,&box);
									InsetRect(&box,1,1);
									EraseRect(&box);
									InvalRect(&box);
									GetDItem(dialog,PopItem,&itemType,&itemHand,&box);
									EraseRect(&box);
									InvalRect(&box);
								} else
									c1=0;
							}
							HUnlock(strHand);
							return true;
						}
					} else if(chr==DELETE) {
						fields[3+2*fields[2]]=0;
						checkCDialogConsistency(fields);
						GetDItem(dialog,TemplateItem,&itemType,&itemHand,&box);
						InsetRect(&box,1,1);
						EraseRect(&box);
						InvalRect(&box);
						GetDItem(dialog,PopItem,&itemType,&itemHand,&box);
						EraseRect(&box);
						InvalRect(&box);
						return true;
					} else
						c1=0;
				} else {		/* One of the three EditText items has been typed into */
					c1=0;
					if(*itemHit==NumberOfColumnsItem) {
						if(isdigit(chr) || chr==DELETE)
							return false;
						else if(isgraph(chr))
							return true;
					} else if(*itemHit==StationsPerPageItem) {
						if(isdigit(chr) || chr==DELETE)
							return false;
						else if(isgraph(chr))
							return true;
					} else if(*itemHit==TemplateTitleItem) {
						return false;
					}
				}
		}
	}
	return false;
}

short GetTemplate(short item,char *fields)
{
	int len;
	MRPdHandle resH;
	short ID;
	ResType type;
	Str255 templateName;
	
	if(item) {
		GetItem(SelectTemplateMenu,item,templateName);
		resH=(MRPdHandle)GetNamedResource('MRPd',templateName);
		if(resH) {
			len=2*(**resH).nColumns;
			HLock((Handle)resH);
			BlockMove(&(**resH).flag,fields,len+4);
			HUnlock((Handle)resH);
			GetResInfo((Handle)resH,&ID,&type,templateName);
		}
	}
	return ID;
}

int CalcNTemplateCols(char *fields)
{
	int i,nCols,field;
	
	nCols=fields[3];
	if(nCols) {
		for(i=5;i<(2*nCols+4);i+=2) {
			field=fields[i]&MenuMask;
			if(field==10 || field==11) {
				if((fields[i+2]&MenuMask)!=field) {
					/* Expands if not a same kind of dims field */
					BlockMove(fields+i+1,fields+i+7,2*nCols+5-i);
					*(fields+i+1)=fields[i-1];
					*(fields+i+2)=fields[i];
					*(fields+i+3)=fields[i-1];
					*(fields+i+4)=fields[i];
					*(fields+i+5)=fields[i-1];
					*(fields+i+6)=fields[i];
					nCols+=3;
					i+=6;
				} else while((fields[i]&MenuMask)==field)
					i+=2;
			}
		}
	}
	return nCols;
}

void doConfigureDialog(WindowPtr x)
{
#ifdef MC68000
#pragma unused (x)
#endif
	DialogPtr configureDialog;
	short hiliteLevel;
	short itemHit,itemType,selectedFlags;
	Handle itemHand;
	long num;
	Boolean pau=false,dirty=false,toggleHilite=false;
	Rect box;
	char fields[44];
	Str255 numStr;

	configureDialog=GetNewDialog(143,(Ptr)&DlogStor,(WindowPtr)(-1));
	if(!configureDialog)
		FatalError();
		
	memset(fields,0,44);
	if(gCurrentTemplate) {
		GetTemplate(gCurrentTemplate,fields);
		num=fields[2];
		fields[3]=CalcNTemplateCols(fields);
	} else {
		fields[1]=1;		/* blank flag */
		fields[3]=9;		/* nCols */
		fields[4]=fTabDown;
	}
	fields[2]=-2;		/* Column selected (will be nStations); */

	DataFieldsMenu=GetMenu(DATAFIELDS_ID);
	InsertMenu(DataFieldsMenu,-1);
	
	GetDItem(configureDialog,StationsPerPageItem,&itemType,&itemHand,&box);
	
	NumToString(num,numStr);
	SetIText(itemHand,numStr);
	GetDItem(configureDialog,NumberOfColumnsItem,&itemType,&itemHand,&box);
	NumToString(fields[3],numStr);
	SetIText(itemHand,numStr);
	
	GetDItem(configureDialog,confLastItem,&itemType,&itemHand,&box);
	SetDItem(configureDialog,confLastItem,itemType,(Handle)doButton,&box);
	GetDItem(configureDialog,TemplateItem,&itemType,&itemHand,&box);
	SetDItem(configureDialog,TemplateItem,itemType,(Handle)drawTemplate,&box);
	GetDItem(configureDialog,PopItem,&itemType,&itemHand,&box);
	SetDItem(configureDialog,PopItem,itemType,(Handle)DrawPopUp,&box);
	GetDItem(configureDialog,confUserFrame1,&itemType,&itemHand,&box);
	SetDItem(configureDialog,confUserFrame1,itemType,(Handle)doFrame,&box);
	GetDItem(configureDialog,confUserFrame2,&itemType,&itemHand,&box);
	SetDItem(configureDialog,confUserFrame2,itemType,(Handle)doFrame,&box);
	SetWRefCon(configureDialog,(long)fields);
	
	GetItem(SelectTemplateMenu,gCurrentTemplate,numStr);
	GetDItem(configureDialog,TemplateTitleItem,&itemType,&itemHand,&box);
	SetIText(itemHand,numStr);
	SelIText(configureDialog,TemplateTitleItem,0,32767);
	
	/*	Make button not active until a change has been made. */
	GetDItem(configureDialog,confMakeButton,&itemType,&itemHand,&box);
	HiliteControl((ControlHandle)itemHand,255);
	
	/* The first two templates cannot be deleted. */
	if(gCurrentTemplate<3) {
		GetDItem(configureDialog,confDeleteButton,&itemType,&itemHand,&box);
		HiliteControl((ControlHandle)itemHand,255);
	}
	
	while(!pau) {
		/* Do the two check boxes */
		if(fields[2]>0) {
			hiliteLevel=0;
			selectedFlags=fields[2+2*fields[2]];
		} else {
			hiliteLevel=255;
			selectedFlags=0;
		}
		GetDItem(configureDialog,HalfWidthCheckItem,&itemType,&itemHand,&box);
		HiliteControl((ControlHandle)itemHand,hiliteLevel);
		SetCtlValue((ControlHandle)itemHand,selectedFlags&fNarrow);
		
		GetDItem(configureDialog,SplitCheckItem,&itemType,&itemHand,&box);
		HiliteControl((ControlHandle)itemHand,hiliteLevel);
		SetCtlValue((ControlHandle)itemHand,selectedFlags&fSplit);
		
		/* Do the RadioButtons */
		GetDItem(configureDialog,TabRightRadioItem,&itemType,&itemHand,&box);
		HiliteControl((ControlHandle)itemHand,hiliteLevel);
		SetCtlValue((ControlHandle)itemHand,(selectedFlags&TabMask)==0);
		
		GetDItem(configureDialog,TabDownRadioItem,&itemType,&itemHand,&box);
		HiliteControl((ControlHandle)itemHand,hiliteLevel);
		SetCtlValue((ControlHandle)itemHand,(selectedFlags&TabMask)==fTabDown);
		
		GetDItem(configureDialog,TabBackRadioItem,&itemType,&itemHand,&box);
		HiliteControl((ControlHandle)itemHand,hiliteLevel);
		SetCtlValue((ControlHandle)itemHand,(selectedFlags&TabMask)==fTabBack);

		GetDItem(configureDialog,21,&itemType,&itemHand,&box);
		SetCtlValue((ControlHandle)itemHand,*fields&fRemarksFieldOnPage);
		GetDItem(configureDialog,22,&itemType,&itemHand,&box);
		SetCtlValue((ControlHandle)itemHand,*fields&fRemarksAreNotes);
		GetDItem(configureDialog,StaggeredCheckItem,&itemType,&itemHand,&box);
		SetCtlValue((ControlHandle)itemHand,*fields&fStaggered);

		ModalDialog(configFilter,&itemHit);
		
		toggleHilite = (dirty)? false : true;
		
		switch(itemHit) {
			case confCancelButton:		/* default */
				pau=true;
				break;
			case confMakeButton:		/* Make */
				pau=true;
				
				GetDItem(configureDialog,StationsPerPageItem,&itemType,&itemHand,&box);
				GetIText(itemHand,numStr);
				StringToNum(numStr,&num);
				
				if(num>kMaxTemplateSta) {
					SysBeep(7);
					doMessage(21);
					NumToString(kMaxTemplateSta,numStr);
					SetIText(itemHand,numStr);
					pau=false;
					break;
				} 
				fields[2]=num;
				
				GetDItem(configureDialog,NumberOfColumnsItem,&itemType,&itemHand,&box);
				GetIText(itemHand,numStr);
				StringToNum(numStr,&num);
				
				if(num>kMaxTemplateCols) {
					SysBeep(7);
					doMessage(22);
					NumToString(kMaxTemplateCols,numStr);
					SetIText(itemHand,numStr);
					pau=false;
					break;
				}
				fields[3]=num;
				
				GetDItem(configureDialog,TemplateTitleItem,&itemType,&itemHand,&box);
				GetIText(itemHand,numStr);
				
				MakeDlogResource(fields,0,numStr);
				break;
			case confDeleteButton:
				pau=DisposeMRPd(gCurrentTemplate);
				break;
			case HalfWidthCheckItem:
				fields[2+2*fields[2]]^=fNarrow;
				break;
			case SplitCheckItem:
				fields[2+2*fields[2]]^=fSplit;
				checkCDialogConsistency(fields);
				break;
			case TabRightRadioItem:
				fields[2+2*fields[2]]&=~TabMask;
				break;
			case TabDownRadioItem:
				fields[2+2*fields[2]]&=~TabMask;
				fields[2+2*fields[2]]|=fTabDown;
				break;
			case TabBackRadioItem:
				fields[2+2*fields[2]]&=~TabMask;
				fields[2+2*fields[2]]|=fTabBack;
				break;
			case 21:
				*fields^=fRemarksFieldOnPage;
				break;
			case 22:
				*fields^=fRemarksAreNotes;
				break;
			case StaggeredCheckItem:
				*fields^=fStaggered;
				checkCDialogConsistency(fields);
				GetDItem(configureDialog,TemplateItem,&itemType,&itemHand,&box);
				SetPort(configureDialog);
				InvalRect(&box);
				EraseRect(&box);
				break;
			default:
				toggleHilite = false;
				break;
		}
		if(toggleHilite) {
			dirty = true;
			GetDItem(configureDialog,confMakeButton,&itemType,&itemHand,&box);
			HiliteControl((ControlHandle)itemHand,0);
			/* Once a change has been made, you can no longer delete. */
			GetDItem(configureDialog,confDeleteButton,&itemType,&itemHand,&box);
			HiliteControl((ControlHandle)itemHand,255);
		}
	}
	CloseDialog(configureDialog);
	/* In addition, we release the poput menu. */
	DeleteMenu(DATAFIELDS_ID);
	ReleaseResource((Handle)DataFieldsMenu);
}

pascal void DrawPopUp(DialogPtr dialog, short x)
{
#ifdef MC68000
#pragma unused (x)
#endif
	Handle itemHand;
	Rect box;
	short itemType;
	Str255 popLabel;
	char *fields;
	short newWid, newLen, wid;

	fields=(char *)GetWRefCon(dialog);
	if(fields[2]>0)
		GetItem(DataFieldsMenu,1+(short)fields[3+2*fields[2]]&MenuMask,popLabel);
	else
		*popLabel=0;
	
	GetDItem(dialog,PopItem,&itemType,&itemHand,&box);
	EraseRect(&box);
	InsetRect(&box,-1,-1); 							/* make it a little bigger */
		
	/* Make sure the title fits. Truncate it and add an ellipses (���) */

	wid = (box.right - box.left) - (LeftSlop + RightSlop); 	/* Available string area */
	newWid = StringWidth(popLabel); 
	
	if(newWid > wid) {

		newLen = *popLabel; 		/* current length in characters */
		wid -= CharWidth('�'); 		/* subtract width of ellipses} */
		
		do 
			newWid -= CharWidth(popLabel[newLen--]);
		 while (newWid>wid && newLen);
		
		popLabel[newLen] = '�'; 
		*popLabel = newLen; 		/* fix the length */
	}

	/* draw the box and its drop shadow */
	FrameRect(&box);
	MoveTo(box.right,box.top+2); 
	LineTo(box.right,box.bottom);
	LineTo(box.left+2,box.bottom);
	MoveTo(box.right-17,box.top+6);
	Line(10,0);
	Move(-9,1);
	Line(8,0);
	Move(-7,1);
	Line(6,0);
	Move(-5,1);
	Line(4,0);
	Move(-3,1);
	Line(2,0);
	Move(-1,1);
	Line(0,0);
	MoveTo(box.left+LeftSlop,box.bottom-BotSlop);
	DrawString(popLabel);
}

void checkCDialogConsistency(char *fields)
{
	int i,type,selected,alt;
	
	selected=3+2*fields[2];				/* fields[2] is the selected column, 1 indexed. */
	type=fields[selected]&MenuMask;
	
	/* 1. Look up if there are alternate names for this type of field. */
	alt=(AltBits>>((type-1)<<1))&3;
	
	/* 2. Set blank bit if there are no named fields. */
	fields[1]|=fBlank;
	for(i=5;i<2*fields[3]+4;i+=2) {
		if(fields[i]) {
			fields[1]&=~fBlank;
			break;
		}
	}
	/* 3. Examine if the alt condition is met. */
	if(alt) {
		fields[selected]&=~fAlt;
		if(((*fields&fStaggered)&&(alt&2))||((alt&1)&&(fields[selected-1]&fSplit)))
			fields[selected]|=alt=fAlt;
		else
			alt=0;
	}
	/* 4. Expand a dimensions field type to the following three cols. */
	if(type==10 || type==11) {
		for(i=selected+2;i<selected+8;i+=2) 
			fields[i]=type|alt;
	}

}

void RemoveDuplicates(char *fields)
{
	int i,type,selected;
	
	selected=3+2*fields[2];				/* fields[2] is the selected column, 1 indexed. */
	type=fields[selected];

	if(type>0) {
		for(i=5;i<(2*fields[3]+4);i+=2) {	/* fields[3] is the number of columns */
			if(i!=selected && fields[i]==type) {
				fields[i]=0;
				return;
			}
		}
	}
}

Handle MakeDlogResource(char *fields,Handle existingMRPd,StringPtr dlogName)
{
	/*	
		This routine does all the work required to make a new data
		entry template. It creates the MPRd resource for the template,
		the DLOG and DITL resources for it, the lookup data structure
		for the template, of type DEntryType, and maintains the global 
		array of handles to these data types. And it appends a menu item
		for all of this to the SelectTemplateMenu hierarchical menu.
	*/
	
	Ptr p;
	short ID,curResRef;
	Rect r;
	Point lowerRight;
	Handle DItemListH,DLogH;
	MRPdHandle MRPdH;
	DEMapHandle eM;
	
	if(eM=AllocateDEMap(fields)) {
		if(DItemListH=newDataDItemList(fields,&lowerRight,eM)) {
			if(DLogH=NewHandle(20+*dlogName+1)) {
				MRPdH = (existingMRPd)? (MRPdHandle) existingMRPd :
							(MRPdHandle)NewHandle(sizeof(MRPdType)-2+2*fields[3]) ;
				/* sizeof(MRPdType) is 8. The char[1] array occupies 2 bytes */
				if(MRPdH) {
					curResRef=CurResFile();
					UseResFile(gPrefsResRefNum);
					do { ID=UniqueID('DITL'); } while(ID<128);
					AddResource(DItemListH,'DITL',ID,dlogName);
					
					HLock(DLogH);
					SetRect(&r,20,40,lowerRight.h+40,lowerRight.v+70);
					p=*DLogH;
					*(Rect *)p=r;	/* boundsRect */
					p+=8;
					*(short *)p=0;	/* procID */
					p+=2;
					*p++=1;			/* visible */
					*p++=0;			/* ignored */
					*p++=1;			/* goAway  */
					*p++=0;			/* ignored */
					*(long *)p=0;	/* refCon  */
					p+=4;
					*(short *)p=ID;	/* resID of DITL */
					BlockMove(dlogName,p+2,*dlogName+1);
					HUnlock(DLogH);
					do { ID=UniqueID('DLOG'); } while(ID<128);
					AddResource(DLogH,'DLOG',ID,dlogName);
					
					(**MRPdH).DlogID = ID;		/* resourceID of DLOG */
					if(existingMRPd) 
						ChangedResource(existingMRPd);
					else {
						HLock((Handle)MRPdH);
						BlockMove(fields,&(**MRPdH).flag,4+2*fields[3]);
						HUnlock((Handle)MRPdH);
						do { ID=UniqueID('MRPd'); } while(ID<128);
						(**eM).resID = ID;
						AddResource((Handle)MRPdH,'MRPd',ID,dlogName);
					}
					WriteResource((Handle)MRPdH);
					
					AppendMenu(SelectTemplateMenu,dlogName);
					CheckItem(SelectTemplateMenu,gCurrentTemplate,false);
					gCurrentTemplate=CountMItems(SelectTemplateMenu);
					CheckItem(SelectTemplateMenu,gCurrentTemplate,true);
					
					AddTemplateMap(gCurrentTemplate,(Handle)eM);
					
					UseResFile(curResRef);
				} else {
					DisposeHandle(DLogH);
					DisposeHandle(DItemListH);
				}
			} else
				DisposeHandle(DItemListH);
		}
	}
	return (Handle)eM;
}

int SetUpGroups(char *fields,int *firstColInGroup,int *nColsInGroup)
{
	int i,k,nCols;
	int nGroups=-1;
	int nBlanks=0;
	
	nCols = fields[3];
	
	for(i=k=0;i<nCols;i++,k+=2) {
		if(fields[5+k]&MenuMask) {
			nGroups++;
			firstColInGroup[nGroups] = i;
			nColsInGroup[nGroups] = 0;
			
			while(i<nCols && !(fields[4+k]&TabMask)) {
				/* while tabRight, up the count */
				nColsInGroup[nGroups]++;
				i++;
				k+=2;
			}
			if(i<nCols) {
				if(fields[4+k]&fTabBack) {
				/*	This is the last col in this group. We don't know yet if there
					is another group */
					nColsInGroup[nGroups]++;
				
				} else if(fields[4+k]&fTabDown) {
				/*	The preceeding col was last in group, if that group is established.
					This is a one column group, in any case, but if it is a blank field
					we do not add it now. */
					if(!nColsInGroup[nGroups])		/* This is the present group. Close it off */
						nColsInGroup[nGroups]=1;
					else if(fields[5+k]&MenuMask) {	/* Non-blank, so create new group. */
						nGroups++;
						firstColInGroup[nGroups]=i;
						nColsInGroup[nGroups]=1;
					} else
						nBlanks++;
				} 
			}
		} else 
			/* Any blank beginning a group goes to the end of the array. */
			nBlanks++;
	}
	
	for(i=k=0;nBlanks && i<nCols;i++,k+=2) {
		if(!(fields[5+k]&MenuMask)) {
			nGroups++;
			firstColInGroup[nGroups] = i;
			nColsInGroup[nGroups] = 0;
			
			while(nBlanks && i<nCols && !(fields[4+k]&TabMask) && !(fields[5+k]&MenuMask)) {
				/* while tabRight and still blank, up the count */
				nColsInGroup[nGroups]++;
				i++;
				k+=2;
				nBlanks--;
			}
			if(nBlanks && i<nCols && !(fields[5+k]&MenuMask)) {
				nBlanks--;
				if(fields[4+k]&fTabBack) {
				/* This is the last col in this group. We don't know yet if there
					is another group */
					nColsInGroup[nGroups]++;			
				} else if(fields[4+k]&fTabDown) {
				/* The preceeding col was last in group, if that group is established.
					This is a one column group, in any case. */
					if(nColsInGroup[nGroups])
						nGroups++;
					nColsInGroup[nGroups]=1;
					firstColInGroup[nGroups]=i;
				}
			}
		} 
	}
	return nGroups+1;
}

Handle newDataDItemList(char *fields,Point *lr,DEMapHandle eM)
{
	Handle DItemListH;
	Ptr DItemList;
	Rect r;
	int i,j,k,nCols,nShots,cnt,field,top,itemNumber;
	int row,col;
	int nGroups;
	Boolean staggered;
	char *mapFields;
	int nColsInGroup[kMaxTemplateCols];
	int firstColInGroup[kMaxTemplateCols];
	int leftMargins[kMaxTemplateCols+1];
	char colNames[20];
	char colFlags[20];
	
	nShots=(*fields&fStaggered)? fields[2]-1 : fields[2];
	nCols=fields[3];

	/* Different code used for the dialog lookup tables. */
	
	for(k=i=0;i<nCols;i++,k+=2) {
		colFlags[i]=fields[4+k];
		colNames[i]=fields[5+k]&MenuMask;
		if(colNames[i] == 10) {		/* L */
			k+=2;
			for(j=1;j<4;j++,k+=2) {
				colNames[i+j] = 10+j;	/* R,U,D */
				colFlags[i+j] = colFlags[i];
			}
			i+=j;
		} else if(colNames[i] == 11) {
			for(j=0;j<4;j++,k+=2) {
				colNames[i+j] = 14+j;	/* L,R,U,D */
				colFlags[i+j] = colFlags[i];
			}
			i+=j;
		} else if(colNames[i] ==12)		/* Remarks */
			colNames[i] = 18;
	}
		
	/* 1. Already know how many dialog fields there will be. */
	
	cnt = (**eM).nFields;
	
	/* 2. Determine the left margins of every column. */
	
	leftMargins[0]=20;
	for(i=0;i<nCols;i++) {
		leftMargins[i+1]=leftMargins[i]+
							cellMargin+
							((colFlags[i]&fNarrow)?cellNWidth:cellWidth);
	}
	
	lr->h = leftMargins[nCols];
	lr->v = 62 + fields[2]*(cellHeight+cellMargin);
	
	/* 3. Determine how many groups there are, based on tab order. A group is one
		or more columns that are filled with data together. */
		
	nGroups = SetUpGroups(fields,firstColInGroup,nColsInGroup);

	if(DItemListH=NewHandle(2+endSize+cnt*SizeOfDItem)) {
		HLock(DItemListH);
		DItemList = *DItemListH;
		*((short *)DItemList) = cnt+2;	/* number of items - 1 */
		DItemList+=2;
		HLock((Handle)eM);
		mapFields = (**eM).fields;
		itemNumber=0;
		
		for(i=0;i<nGroups;i++) {
			for(row=0;row<nShots;row++) {
				top = (*fields&fStaggered)? 62 : 62+19;
				top += row * (cellHeight+cellMargin);
				for(j=0,col=firstColInGroup[i];j<nColsInGroup[i];j++,col++) {
					r.left=leftMargins[col];
					r.right = r.left + ((colFlags[col]&fNarrow)? cellNWidth:cellWidth);
					
					field=colNames[col];
					*mapFields++ = colNames[col];
					itemNumber++;
					if(row==0) 
						(**eM).firstFields[field]=itemNumber;
					else if(row==1) 
						(**eM).offsets[field]=itemNumber-(**eM).firstFields[field];
					
					
					staggered = (*fields&fStaggered) && (!field || (field>2 && field<10)) ;
					
					r.top = (staggered)? top+19 : top;
	
					r.bottom=r.top+((colFlags[col]&fSplit)? cellHHeight:cellHeight);
				
					(*(long *)DItemList)=0;
					DItemList+=4;
					(*(Rect *)DItemList)=r;
					DItemList+=8;
					*DItemList++=editText;	/* 16 */
					*DItemList++=0;
					
					r.top=r.bottom+cellMargin;
					if(colFlags[col]&fSplit) {
						*mapFields++ = colNames[col] + 1;
						itemNumber++;
						if(row==0) 
							(**eM).firstFields[field+1]=itemNumber;
						else if(row==1) 
							(**eM).offsets[field+1]=itemNumber-(**eM).firstFields[field+1];

						r.top=r.bottom+cellMargin;
						r.bottom=r.top+cellHHeight-1;
						(*(long *)DItemList)=0;
						DItemList+=4;
						(*(Rect *)DItemList)=r;
						DItemList+=8;
						*DItemList++=editText;	/* 16 */
						*DItemList++=0;
					}
				}
			}
			if(*fields&fStaggered) {
				top = (*fields&fStaggered)? 62 : 62+19;
				top += row * (cellHeight+cellMargin);
				for(j=0,col=firstColInGroup[i];j<nColsInGroup[i];j++,col++) {
					field = colNames[col];
					if(field==1 || field==2 || field>9) {
						r.left=leftMargins[col];
						r.right = r.left + ((colFlags[col]&fNarrow)? cellNWidth:cellWidth);
					
						*mapFields++ = colNames[col];
						itemNumber++;
						if(row==0) 
							(**eM).firstFields[field]=itemNumber;
						else if(row==1) 
							(**eM).offsets[field]=itemNumber-(**eM).firstFields[field];
						
						r.top =  top;
	
						r.bottom=r.top+((colFlags[col]&fSplit)? cellHHeight:cellHeight);
					
						(*(long *)DItemList)=0;
						DItemList+=4;
						(*(Rect *)DItemList)=r;
						DItemList+=8;
						*DItemList++=editText;	/* 16 */
						*DItemList++=0;
					
						r.top=r.bottom+cellMargin;
						if(colFlags[col]&fSplit) {
							*mapFields++ = colNames[col] + 1;
							itemNumber++;
							if(row==0) 
								(**eM).firstFields[field+1]=itemNumber;
							else if(row==1) 
								(**eM).offsets[field+1]=itemNumber-(**eM).firstFields[field+1];
							
							r.top=r.bottom+cellMargin;
							r.bottom=r.top+cellHHeight-1;
							(*(long *)DItemList)=0;
							DItemList+=4;
							(*(Rect *)DItemList)=r;
							DItemList+=8;
							*DItemList++=editText;	/* 16 */
							*DItemList++=0;
						}
					}
				}
			}
		} 

		/* 18 bytes total */
		SetRect(&r,lr->h-78,13,lr->h-40,29);			/* left,top,right,bottom */
		(*(long *)DItemList)=0;
		DItemList+=4;
		(*(Rect *)DItemList)=r;
		DItemList+=8;
		*DItemList++=statText;	
		*DItemList++=4;	
		(*(long *)DItemList)='Page';
		DItemList+=4;
		
		/* 15 bytes total */
		SetRect(&r,lr->h-36,13,lr->h-3,29);
		(*(long *)DItemList)=0;
		DItemList+=4;
		(*(Rect *)DItemList)=r;
		DItemList+=8;
		*DItemList++=statText;	
		*DItemList++=1;	
		*DItemList++='1';
		*DItemList++=0;

		/* 14 bytes total */
		SetRect(&r,17,40,lr->h,80);
		(*(long *)DItemList)=0;
		DItemList+=4;
		(*(Rect *)DItemList)=r;
		DItemList+=8;
		*DItemList++=userItem;	
		*DItemList++=0;	
	
		HUnlock(DItemListH);
		HUnlock((Handle)eM);
	}
	return DItemListH;
}

DEMapHandle CalcDEMap(char *fields)
{
	int i,j,k,nCols,nShots,cnt,field,itemNumber;
	int row,col;
	DEMapHandle eM;
	int nGroups;
	char *mapFields;
	int nColsInGroup[kMaxTemplateCols];
	int firstColInGroup[kMaxTemplateCols];
	char colNames[20];
	char colFlags[20];
	
	if(eM=AllocateDEMap(fields)) {
		nShots=(*fields&fStaggered)? fields[2]-1 : fields[2];
		nCols=fields[3];
		
		for(k=i=0;i<nCols;i++,k+=2) {
			colFlags[i]=fields[4+k];
			colNames[i]=fields[5+k]&MenuMask;
			if(colNames[i] == 10) {		/* L */
				k+=2;
				for(j=1;j<4;j++,k+=2) {
					colNames[i+j] = 10+j;	/* R,U,D */
					colFlags[i+j] = colFlags[i];
				}
				i+=j;
			} else if(colNames[i] == 11) {
				for(j=0;j<4;j++,k+=2) {
					colNames[i+j] = 14+j;	/* L,R,U,D */
					colFlags[i+j] = colFlags[i];
				}
				i+=j;
			} else if(colNames[i] ==12)		/* Remarks */
				colNames[i] = 18;
		}
		
		/* 1. Already know how many dialog fields there will be. */
		
		cnt = (**eM).nFields;
		
		nGroups = SetUpGroups(fields,firstColInGroup,nColsInGroup);
		HLock((Handle)eM);
		mapFields = (**eM).fields;
		itemNumber=0;

		for(i=0;i<nGroups;i++) {
			for(row=0;row<nShots;row++) {
				for(j=0,col=firstColInGroup[i];j<nColsInGroup[i];j++,col++) {

					field=colNames[col];
					*mapFields++ = colNames[col];
					itemNumber++;
					if(row==0) 
						(**eM).firstFields[field]=itemNumber;
					else if(row==1) 
						(**eM).offsets[field]=itemNumber-(**eM).firstFields[field];

					if(colFlags[col]&fSplit) {
						*mapFields++ = colNames[col] + 1;
						itemNumber++;
						if(row==0) 
							(**eM).firstFields[field+1]=itemNumber;
						else if(row==1) 
							(**eM).offsets[field+1]=itemNumber-(**eM).firstFields[field+1];
					}
				}
			}
			if(*fields&fStaggered) {
				for(j=0,col=firstColInGroup[i];j<nColsInGroup[i];j++,col++) {
					field = colNames[col];
					if(field==1 || field==2 || field>9) {
						*mapFields++ = colNames[col];
						itemNumber++;
						if(row==0) 
							(**eM).firstFields[field]=itemNumber;
						else if(row==1) 
							(**eM).offsets[field]=itemNumber-(**eM).firstFields[field];
						if(colFlags[col]&fSplit) {
							*mapFields++ = colNames[col] + 1;
							itemNumber++;
							if(row==0) 
								(**eM).firstFields[field+1]=itemNumber;
							else if(row==1) 
								(**eM).offsets[field+1]=itemNumber-(**eM).firstFields[field+1];
							
						}
					}
				}
			}
		}
		HUnlock((Handle)eM);
		AddTemplateMap(gCurrentTemplate,(Handle)eM);
	}
	return eM;
}

Boolean DisposeMRPd(short tNum)
{
	short oldResFile,thisResFile;
	Handle h1,h2,h3;
	Str255 templateName;

	GetItem(SelectTemplateMenu,tNum,templateName);
	templateName[*templateName+1]=0;
	if(ConfirmDialog(7,templateName)) {
		DelMenuItem(SelectTemplateMenu,tNum);
		
		if(gCurrentTemplate>CountMItems(SelectTemplateMenu))
			gCurrentTemplate--;
		CheckItem(SelectTemplateMenu,gCurrentTemplate,true);
		
		oldResFile = CurResFile();
		
		if(h1=GetNamedResource('MRPd',templateName)) {
			if(h2=GetResource('DLOG',*(short *)*h1)) {
				if(h3=GetResource('DITL',*((short *)(*h2+18)))) {
					thisResFile = HomeResFile(h3);
					UseResFile(thisResFile);
					RmveResource(h3);
					DisposeHandle(h3);
				}
				thisResFile = HomeResFile(h2);
				UseResFile(thisResFile);
				RmveResource(h2);
				DisposeHandle(h2);
			}
			thisResFile = HomeResFile(h1);
			UseResFile(thisResFile);
			RmveResource(h1);
			DisposeHandle(h1);
		}
		
		UseResFile(oldResFile);
		return true;
	}
	return false;
}

DEMapHandle AllocateDEMap(char *fields)
{
	DEMapHandle eM;
	int i,nCols,nShots,cnt,field;

	nShots=fields[2];
	nCols=fields[3];
	
	/* 1. Count how many fields there are. */
	if(*fields&fStaggered) {
		nShots--;
		for(cnt=i=0;i<2*nCols;i+=2) {
			field=fields[5+i]&MenuMask;
			cnt += (((fields[4+i]&fSplit)? 2:1)*((field==1 || field==2 || field>9)? nShots+1:nShots));
		}
	} else {
		for(cnt=i=0;i<2*nCols;i+=2) 
			if(fields[4+i]&fSplit)
				cnt+=nShots;
		cnt+=nCols*nShots;
	}

	eM = (DEMapHandle)NewHandleClear(sizeof(DEntryMap)+cnt);
	if(eM) {
		(**eM).nFields = cnt;
		(**eM).flags = fields[0];
		(**eM).spareFlags = fields[1];
		(**eM).nStations = fields[2];
	} else
		doMessage(1);
		
	return eM;
}

void AddTemplateMap(short index,Handle h)
{
	Size hSize;
	short nItems;
	Handle *hArray;
	OSErr err;
	
	nItems = CountMItems(SelectTemplateMenu);
	if(!gMapHandles) {
		gMapHandles = NewHandleClear(sizeof(Handle)*(nItems+8));
		if(!gMapHandles) {
			doMessage(1);
			return;
		}
	}
	if((hSize=GetHandleSize(gMapHandles)) < (sizeof(Handle)*(nItems+1))) {
		SetHandleSize(gMapHandles,sizeof(Handle)*(nItems+9));
		err=MemError();
		if(err!=noErr) {
			doMessage(1);
			return;
		}
		HLock(gMapHandles);
		memset(*gMapHandles + hSize,0,(sizeof(Handle)*(nItems+9))-hSize);
	}
	HLock(gMapHandles);
	hArray = (Handle *)*gMapHandles;
	if(hArray[index])
		DisposeHandle(hArray[index]);
	hArray[index] = h;
	HUnlock(gMapHandles);
}

/*	
	There are four principal data structures associated with a WYSIWBLL data
	entry window. (What! FOUR *principal* structures. You mean there's more
	than four? Isn't FOUR already too many??) It's not as complicated as it
	seems. The key is the C structure that is allocated on the heap
	and whose handle is put in doc->docData of the data entry Document. The
	handle is also kept in an array of handles, at *gMapHandles, that is
	indexed by the item number of the TemplateMenu. This index value is 
	stored in the global gCurrentTemplate. (**DEntryMap).redID references
	an 'MRPd' resource which in turn references the 'DLOG' and 'DITL'
	resources for the data entry dialog window. Simple, eh?
	
	If a DEntryMap has not been created for a template item, then it is
	made when the user calls to use that template. Similarly, if the 'DLOG'
	and 'DITL' resources have not been created, they can be as well. The
	'MRPd' resource is sufficient to do all of this.
	
	Because of its brevity, the 'MRPd' resource is the only structure
	stored with a text document. The 'DLOG' and 'DITL' resources are kept
	around in the 'Vectors Preferences' file.
*/
	