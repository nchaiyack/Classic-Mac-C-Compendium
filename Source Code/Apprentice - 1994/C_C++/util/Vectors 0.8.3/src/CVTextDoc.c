#if 0

/*------------------------ The MPSR structures ------------------------

struct zoomRects {		// MPSR 1008
	Rect userState;
	Rect otherState;
	long vScrollValue;
	long nothing;
	short a,b,c;	// More added here for multiple panes
}

struct stateRec {		// MPSR 1005
	short fontSize;
	char fontName[32];
	short fontWidth;	// 0006
	short tabWidth;		// 0004
	Rect userState;
	Rect otherState;
	long modifiedDate;
	long selStart;
	long selEnd;
	long vScrollValue;
	short flag;	// 0100 autoindent
				// 0001 show invisibles
}

/*
	Resource MPSR 1007 begins with a *short*, the number of marks, followed
	by that many markRecs. MarkRecs are of even length. The markName
	is a pString.
	
	There is no flag to indicate if order is alphabetical or not. It
	must sort upon loading and if the sort has no effect, Marks that
	state.
	
*/

struct markRec {
	long selStart;
	long selEnd;
	char *markName;
}
	

data 'MPSR' (1005) {
	$"0009 4D6F 6E61 636F 006F 0020 2831 3030"            /* .�Monaco.o. (100 */
	$"3529 207B 0D09 2422 3030 3030 2030 3030"            /* 5) {��$"0000 000 */
	$"3020 0006 0004 00C1 00F4 0227 02F1 00C1"            /* 0 .....�...'...� */
	$"00F4 0227 02F1 A868 8ED6 0000 01A2 0000"            /* ...'..�h��...�.. */
	$"01A9 0000 0070 0100"                                /* .�...p.. */
};

data 'MPSR' (1007) {
	$"0004 0000 0008 0000 000D 0541 6375 7273"            /* .........�.Acurs */
	$"0000 007B 0000 0082 0743 4D4C 6D61 696E"            /* ...{...�.CMLmain */
	$"0000 0187 0000 018F 0948 6973 746F 7269"            /* ...�...��Histori */
	$"6300 0000 01A2 0000 01A9 0743 5652 6F75"            /* c....�...�.CVRou */
	$"7473"                                               /* ts */
};

data 'MPSR' (1008) {
	$"00C1 00F4 0227 02F1 00C1 00F4 0227 02F1"            /* .�...'...�...'.. */
	$"0000 0070 0000 0000 0000 0000 0000"                 /* ...p.......... */
};

#endif
	
/*----------------------------- Includes ---------------------------------*/

#include "CVectors.h"
#include "CVGlobs.h"

#ifdef MC68000
#pragma segment TextViewDoc
#endif

#define isneol(c) (c!='\n')
#define iseol(c) (c=='\n')

/*--------------------------- Declarations -------------------------------*/

char *matchExpressionRight(char *,char *,int *,int *);

void AddText(WindowPtr,Ptr,long);

void textDraw(WindowPtr, Rect *,short);
void textActivate(WindowPtr);
void textDeactivate(WindowPtr);
void textDoIdle(WindowPtr wind);
void textAdjustCursor(WindowPtr,Point);
void textDoKeyDown(WindowPtr,EventRecord *);
void textDestructor(WindowPtr);
void textSetScrollBarValues(WindowPtr);
short textGetHorizSize(WindowPtr);
short textGetVertSize(WindowPtr);
void textGetContentRect(WindowPtr,Rect *);
void textScrollContents(WindowPtr,short,short);
short textGetVertLineScrollAmount(WindowPtr);
short textGetHorizLineScrollAmount(WindowPtr);
void textDoZoom(WindowPtr,short );
void textDoGrow(WindowPtr,EventRecord *);
int textWriteDocFile(WindowPtr);
void textAdjustDocMenus(WindowPtr);
void AddTextMemberFunctions(DocumentPeek);
OSErr GetDirectoryFSSpec(short,long,ConstStr255Param,FSSpecPtr);
void FindExpression(WindowPtr,char *);
Boolean findSearchString(WindowPtr, short );
void EnTabAndShift(WindowPtr,int);

extern void DoSavePrefs(TE32KHandle);
extern int nullIntMethod(WindowPtr);

/*---------------------------- TextWindow Object ----------------------------*/

void DrawTick(WindowPtr wind)
{
	Rect r;
	short pageWidth;
	RgnHandle oldClip;
	TE32KHandle tH;
	
	oldClip = NewRgn();
	GetClip(oldClip);
	r=wind->portRect;
	ClipRect(&r);
	
	r=wind->portRect;
	r.bottom=4;
	r.right-=15;
	EraseRect(&r);
	
//	PenPat(qd.ltGray);
	
	tH=(TE32KHandle)((DocumentPeek)wind)->docData;
	pageWidth=(**((DocumentPeek)wind)->fPrintRecord).prInfo.rPage.right;
	pageWidth += ((**tH).destRect.left - (**tH).viewRect.left);
	MoveTo(pageWidth-2*LeftMargin,0);

	Line(0,3);

	SetClip(oldClip);
	DisposeRgn(oldClip);

}

void textDestructor(WindowPtr wind)
{
	WindowPtr sister;
	
	if((TE32KHandle)((DocumentPeek)wind)->docData) {
		TE32KDispose((TE32KHandle)((DocumentPeek)wind)->docData);
		((DocumentPeek)wind)->docData=0;
		if(((DocumentPeek)wind)->associatedWindow) {
			sister = ((DocumentPeek)wind)->associatedWindow;
			if(StripAddress(((DocumentPeek)sister)->associatedWindow)==StripAddress(wind))
				((DocumentPeek)sister)->associatedWindow=0;
			((DocumentPeek)wind)->associatedWindow=0;
		}
	}
	destructor(wind);
}

void textDoPageSetup(WindowPtr wind)
{
	Rect r;
	
	PrOpen();
	PrStlDialog(((DocumentPeek)wind)->fPrintRecord);
	PrClose();
	if(wind) {
		r = wind->portRect;
		r.bottom=4;
		r.right-=15;
		InvalRect(&r);
		EraseRect(&r);
	}
}

void textSetScrollBarValues(WindowPtr wind)
{
	TE32KHandle tH;
	LongRect visible,dest;
	long hPos=0,vPos=0,lineHeight;
	
	if(tH = (TE32KHandle)((DocumentPeek)wind)->docData) {
		visible = (**tH).viewRect;
		dest = (**tH).destRect;
		lineHeight = (**tH).lineHeight;
		vPos = (visible.top - dest.top)/lineHeight;
		hPos = (visible.left - dest.left);
	}
	FocusOnWindow(wind);
	SetCtlValue(((DocumentPeek)wind)->hScroll,(short)hPos);
	SetCtlValue(((DocumentPeek)wind)->vScroll,(short)vPos);
}

void textScrollContents(WindowPtr wind,short dh,short dv) 
{
	TE32KHandle tH;
	int lineHeight;
	
	if(tH=(TE32KHandle)((DocumentPeek)wind)->docData) {
		lineHeight = (**tH).lineHeight;
		TE32KScroll((long)dh,(long)dv*lineHeight,tH);
	}
	DrawTick(wind);
}

short textGetVertLineScrollAmount(WindowPtr wind)
{
	if(((DocumentPeek)wind)->docData)
		return 1;
	else
		return 0;
}

short textGetHorizLineScrollAmount(WindowPtr wind)
{
	return (**((TE32KHandle)((DocumentPeek)wind)->docData)).theCharWidths['M'];
}

void textGetContentRect(WindowPtr wind,Rect *r)
{
	getContentRect(wind,r);
	r->left += 4;
	r->top	+= 4;
}

short textGetVertSize(WindowPtr wind)
{
	return((**((TE32KHandle)((DocumentPeek)wind)->docData)).nLines);
}

short textGetHorizSize(WindowPtr wind)
{
	TE32KHandle tH;
	
	tH = (TE32KHandle)((DocumentPeek)wind)->docData;
		return (**tH).destRect.right - (**tH).destRect.left ;
		
	/* 	Note that the destRect is the same as the viewRect when word
		wrapping to window. */
}

#if 0
void textPrintDraw(WindowPtr wind,Rect *r,short)
{
	TE32KHandle tH;
	short i,firstLine,lastLine;
	
	tH = (TE32KHandle)((DocumentPeek)wind)->docData;
	TextFont((**tH).txFont);
	TextSize((**tH).txSize);
	firstLine=r->top;
	lastLine=r->bottom;
	
	HLock((**tH).hText);
	for(i=firstLine;i<lastLine;i++) {
		MoveTo(LeftMargin,(**tH).lineHeight*(i-firstLine)+TopMargin+(**tH).fontAscent);
		DrawText(*(**tH).hText+(**tH).lineStarts[i],0,(**tH).lineStarts[i+1]-(**tH).lineStarts[i]);
	}
	HUnlock((**tH).hText);
}

#else

void textPrintDraw(WindowPtr wind,Rect *r,short page)
{
	long i,firstChar,lastChar;
	short firstLine,lastLine,vPos;
	long tabWidth;
	short pageWidth;
	unsigned char *textPtr;
	GrafPtr currentPort;
	TE32KHandle tH;
	Str31 numStr;

	tH = (TE32KHandle)((DocumentPeek)wind)->docData;
	
	GetPort(&currentPort);
	
	TextFont((**tH).txFont);
	TextFace((**tH).txFace);
	TextSize((**tH).txSize);
	TextMode((**tH).txMode);
				
	firstLine=r->top;		/* zero indexed */
	lastLine=r->bottom;
	
	HLock((**tH).hText);
	textPtr = (unsigned char *) *((**tH).hText);
	tabWidth = (long)(**tH).tabWidth;
	
	vPos = TopMargin;
	
	while(firstLine<lastLine) {
		i=firstChar = (**tH).lineStarts[firstLine];
		lastChar = (**tH).lineStarts[firstLine+1];
		if (lastChar > firstChar && iseol(textPtr[lastChar-1]))
			lastChar--;
		MoveTo(LeftMargin,vPos);
		if((**tH).showInvisibles) {
			while (firstChar < lastChar) {
				while (i<lastChar && textPtr[i]>=0x20 && !isspace(textPtr[i]))
					i++;

				if (i > firstChar)
					DrawText(&(textPtr[firstChar]),0,(short) (i - firstChar));
				
				if(i<lastChar) {
					if (textPtr[i]==TAB) {
						DrawChar('�');
						MoveTo(LeftMargin + ((currentPort->pnLoc.h - LeftMargin + tabWidth)/tabWidth)*tabWidth,currentPort->pnLoc.v);
					} else if(textPtr[i]==' ')
						DrawChar('�');
					else if(textPtr[i]=='\n')
						DrawChar('�');
					else 
						DrawChar('�');
					i++;
				}
				
				firstChar = i;
				
			}
			if(i<(**tH).teLength && textPtr[i]=='\n')
				DrawChar('�');
		} else {
			while (firstChar < lastChar) {
				while (i<lastChar && textPtr[i]!=TAB)
					i++;
				
				if (i > firstChar)
					DrawText(&(textPtr[firstChar]),0,(short) (i - firstChar));
				
				if (i<lastChar && textPtr[i]==TAB) {
					MoveTo(LeftMargin + ((currentPort->pnLoc.h - LeftMargin + tabWidth)/tabWidth)*tabWidth,currentPort->pnLoc.v);
					i++;
				}
				
				firstChar = i;
				
			}
		}
		vPos += (**tH).lineHeight;
		firstLine++;
	}
	pageWidth=(**((DocumentPeek)wind)->fPrintRecord).prInfo.rPage.right;
	NumToString(page,numStr);
	pageWidth -= StringWidth(numStr);
	MoveTo(pageWidth/2,(**((DocumentPeek)wind)->fPrintRecord).prInfo.rPage.bottom);
	DrawString(numStr);

	HUnlock((**tH).hText);
}
#endif

void textDoTheUpdate(WindowPtr wind, EventRecord *x)
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
#if 0
		PenPat(qd.ltGray);
		Line(0,4);
#else
		DrawTick(wind);
#endif
		DrawControls(wind);
		DrawGrowIcon(wind);
		EndUpdate(wind);
	}
}

void textDraw(WindowPtr wind, Rect *r,short page)
{
	LongRect lr;
	
	if(((DocumentPeek)wind)->docData) {
		if(page) 							/* Printing method */
			textPrintDraw(wind,r,page);
		else {
			EraseRect(r);
			RectToLongRect(r,&lr);
			TE32KUpdate(&lr,(TE32KHandle)((DocumentPeek)wind)->docData);
		}
	}
}

void textDoPrint(WindowPtr wind)
{
	TPPrPort printPort;
	TPrStatus status;
	TE32KHandle tH;
	short page,nPages;
	Rect r;
	short pageHeight,linesPerPage;
	short firstLine,lastLine;
	
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
	tH = (TE32KHandle)((DocumentPeek)wind)->docData;
	
	pageHeight =(**((DocumentPeek)wind)->fPrintRecord).prInfo.rPage.bottom;
	linesPerPage=(pageHeight-TopMargin-BottomMargin)/(**tH).lineHeight;
	nPages=1+(**tH).nLines/linesPerPage;
		
	for(page=1;page<=nPages;page++) {
		firstLine=(page-1)*linesPerPage;
		if(firstLine<0) 
			firstLine=0;
		lastLine=firstLine+linesPerPage;
		if(lastLine>(**tH).nLines)
			lastLine=(**tH).nLines;
		r.top=firstLine;
		r.bottom=lastLine;
		PrOpenPage(printPort,nil);
		(*((DocumentPeek)wind)->draw)(wind,&r,page);
		PrClosePage(printPort);
	}
	PrCloseDoc(printPort);
	if((**((DocumentPeek)wind)->fPrintRecord).prJob.bJDocLoop != 0) 
		PrPicFile(((DocumentPeek)wind)->fPrintRecord,nil,nil,nil,&status);
	PrClose();
}

void textActivate(WindowPtr wind)
{
	activate(wind);
	if(((DocumentPeek)wind)->docData)
		TE32KActivate((TE32KHandle)((DocumentPeek)wind)->docData);
}

void textDeactivate(WindowPtr wind)
{
	deactivate(wind);
	if(((DocumentPeek)wind)->docData)
		TE32KDeactivate((TE32KHandle)((DocumentPeek)wind)->docData);
}

void textDoIdle(WindowPtr wind)
{
	GrafPtr oldPort;
	Point pt;
	
#ifdef IDLE_DEBUG
/*=============== Debug Code ===========*/
	TE32KHandle tH;
	DocumentPeek doc;
	MarkRecPtr theMark;
	long offset;
	char *p,*q;

	doc = (DocumentPeek)FrontLayer();
	tH = (TE32KHandle)((DocumentPeek)wind)->docData;
	if(tH && doc->marks) {
		theMark = (MarkRecPtr)(*doc->marks+2);
		offset = theMark->selStart;
		p=*(**tH).hText+offset;
		q=&theMark->label;
		if(*p != q[1])
			SysBeep(7);
	}
#endif
	
	if(((DocumentPeek)wind)->docData) 
		TE32KIdle((TE32KHandle)((DocumentPeek)wind)->docData);
	GetPort(&oldPort);
	SetPort(wind);
	GetMouse(&pt);
	(*((DocumentPeek)wind)->adjustCursor)(wind,pt);
	SetPort(oldPort);
}

void textAdjustCursor(WindowPtr wind,Point where)
{
	Rect r;
	CursHandle IBeam;
	
	GetContentRect(wind,&r);
	if(PtInRect(where,&r)) {
		IBeam = GetCursor(iBeamCursor);
		if(IBeam)
			SetCursor(*IBeam);
	} else
		InitCursor();
}

short SetSelectionFlag(TE32KHandle tH)
{
	long cr;
	
	if((**tH).selStart < (**tH).selEnd) {
		cr=Munger((**tH).hText,(**tH).selStart,"\n",1L,0,0);
		if(cr<0 || cr>=(**tH).selEnd)
			return 1;	/* no '\n' in selection range */
		return -1;		/* \n in selection range. */
	}
	return 0;			/* no selection range */
}

/*------------------------------- DoCommandLine -----------------------------------
	Here is the command parser for Vectors.
	
	At this time (v.0.7) command lines are passed in the global gCommandLine. This
	is not a bad idea as one might want to Unload most Vectors segments when doing
	a command.
	
	My list of essential commands to be implemented is short.
		1.	CML (always -o ????)
		2.	dir/ls
		3.	open
		4.	target (open then BringToFront(Worksheet)
		5.	line (in target)
		6.	search
		7.	find
		8.	cd
		9.	close
		
----------------------------------------------------------------------------------*/

void SelectLine(WindowPtr wind,long line)
{
	DocumentPeek doc;
	TE32KHandle tH;
	long selStart,selEnd,lastLine;
	GrafPtr oldPort;
	
	if(doc=(DocumentPeek)wind) {
		if((tH=(TE32KHandle)doc->docData) && 
				(((WindowPeek)wind)->windowKind==9 || 
					((WindowPeek)wind)->windowKind==13)) {
			GetPort(&oldPort);
			SetPort(wind);
			lastLine=(**tH).nLines-1;
			/* Since nLines isn�t right if the last character is a return, check for that case.
					(hint from TESample example. */
			if ( *(*(**tH).hText + (**tH).teLength - 1) == '\n' )
				lastLine++;
			line=(line>lastLine)? lastLine: line;
			line=(line<=0)? 1 : line;
			selEnd = (**tH).lineStarts[line];
			selStart = (**tH).lineStarts[line-1];
			TE32KSetSelect(selStart,selEnd,tH);
			positionView(tH,1);
			AdjustScrollBars(wind);
			SetScrollBarValues(wind);
			doc->fHaveSelection = 1;
			SetPort(oldPort);
		}
	}
}

void FindExpression(WindowPtr wind,char *exp)
{
	DocumentPeek doc;
	TE32KHandle tH;
	GrafPtr oldPort;
	short oldSearchMethod,oldSearchBackwards,oldWrapSearch;
	Str255 oldFindBuffer;
	
	if(doc=(DocumentPeek)wind) {
		if((tH=(TE32KHandle)doc->docData) && 
				(((WindowPeek)wind)->windowKind==9 || 
					((WindowPeek)wind)->windowKind==13)) {
			GetPort(&oldPort);
			
			oldSearchMethod=gSearchMethod;
			oldSearchBackwards = gSearchBackwards;
			oldWrapSearch = gWrapSearch;

			gSearchMethod = strpbrk(exp,"?*[�")? 2:0;
			
			BlockMove(gFindBuffer,oldFindBuffer,*gFindBuffer+1);
			exp=ConditionFileName(exp);
			strcpy(gFindBuffer,exp);
			c2pstr(gFindBuffer);
			gSearchBackwards = false;
			gWrapSearch = false;
			if(!findSearchString(wind,0)) {
				gSearchMethod=oldSearchMethod;
				gSearchBackwards = oldSearchBackwards;
				gWrapSearch = oldWrapSearch;
				BlockMove(oldFindBuffer,gFindBuffer,*oldFindBuffer+1);
			}
			SetPort(oldPort);
		}
	}
}

void MakeTargetWindow(WindowPtr wind)
{
	WindowPtr front;
	
	if(wind!=TargetWindow()) {
		front = FrontLayer();
		if(wind!=front) {								/* wind has to be brought closer to the front */
			PaintOne((WindowPeek)wind,((WindowPeek)wind)->strucRgn);
			CalcVis((WindowPeek)wind);
			SendBehind(wind,front);
		} else if(front=(WindowPtr)((WindowPeek)wind)->nextWindow) /* it is the front window and there is more than one window */
			SelectWindow(front);
	}
}

Boolean WildCardMatch(char *tok,char *exp)
{
	int foundLen=0,n2=0;
	
	*(tok+*tok+1) = '\0';
	/* 	
		MatchExpressionRight is a search routine that returns a match
		anywhere in tok. We want a perfect match, where tok is identical
		to the found string.
	*/
	return (matchExpressionRight(tok+1,exp,&foundLen,&n2) && foundLen==*tok);
}

Boolean CheckForFlag(char *str,char *flag,long len)
{
	char *ret;
	ret = strstr(str,flag);
	return (ret)? isspace(*(ret+len)) || !*(ret+len) : false;
}

char *ConditionFileName(char *str)
{	
	char *p,*ret;
	
	p=str;
	while(*p) {
		while(isspace(*p)) p++;
		if(*p=='-') {
			p++;
			while(isalnum(*p)) p++;
		} else if(*p) {
			if(*p=='"' || *p=='\'') {
				ret=p++;
				while(*p && *p!=*ret) p++;
				if(*p) {
					*p='\0';
					return ret+1;
				} else
					return 0;
			} else {
				ret=p;
				while(isgraph(*p)) p++;
				*p='\0';
				return ret;
			}
		}
	}
	return 0;
}

void PrintBuf(StringPtr buf,long length,DocumentPeek doc)
{
	TE32KHandle tH;
	
	if(!doc)
		doc=gWorkSheetDoc;
	if(doc) {
		tH = (TE32KHandle)doc->docData;
		TE32KInsert(buf,length,tH);
		doc->fNeedtoSave = true;
	}
}

void PrintFileName(WindowPtr wind,HFileInfo *pB,char *path)
{
	int k=0,quote;
	Str255 buf;
	
	p2cstr(pB->ioNamePtr);
	
	quote=*path || strchr(pB->ioNamePtr,' ');
	if(quote) {	/* Need to quote */
		*buf='\'';
		k++;
	}
	if(!*path && pB->ioFlAttrib&16)		/* It's a directory	*/
		*(buf+k++)=':';
	if(*path)
		k+=sprintf(buf+k,"%s",path);
	k+=sprintf(buf+k,"%s",pB->ioNamePtr);
	if(pB->ioFlAttrib&16)				/* It's a directory	*/
		*(buf+k++)=':';
	if(quote)
		*(buf+k++)='\'';
	*(buf+k++)='\n';
	*(buf+k)='\0';
	PrintBuf(buf,k,(DocumentPeek)wind);
}

PrintLongFormat(WindowPtr wind,HFileInfo *pB)
{
	int k,hour;
	DateTimeRec date;
	char dateStr[12];
	Str255 buf;
	
	p2cstr(pB->ioNamePtr);
	memset(buf,' ',21);
	if(strchr(pB->ioNamePtr,' ')) 
		k=sprintf(buf,"'%s'",pB->ioNamePtr);
	else
		k=sprintf(buf,"%s",pB->ioNamePtr);
	if(k<21)
		*(buf+k)=' ';
	else
		*(buf+20)='�';
	k=21;
	if(pB->ioFlAttrib&16)		/* It's a directory */
		k+=sprintf(buf+k," Fldr Fldr");
	else
		k+=sprintf(buf+k," %4.4s %4.4s",&pB->ioFlFndrInfo.fdType,&pB->ioFlFndrInfo.fdCreator);
		
	k+=sprintf(buf+k,"%6dK",(pB->ioFlPyLen+pB->ioFlRPyLen)/1024);
	Secs2Date(pB->ioFlMdDat,&date);
	sprintf(dateStr,"%d/%d/%02d",date.month,date.day,date.year%100);
	k+=sprintf(buf+k,"%11s",dateStr);
	hour=(date.hour>12)? date.hour-12:date.hour;
	if(hour==0)
		hour=12;
	k+=sprintf(buf+k,"%3d:%02d %s",hour,date.minute,(date.hour>12)?"PM":"AM");

	Secs2Date(pB->ioFlCrDat,&date);
	sprintf(dateStr,"%d/%d/%02d",date.month,date.day,date.year&100);
	k+=sprintf(buf+k,"%11s",dateStr);
	hour=(date.hour>12)? date.hour-12:date.hour;
	k+=sprintf(buf+k,"%3d:%02d %s",hour,date.minute,(date.hour>12)?"PM":"AM");
	k+=sprintf(buf+k,"\n");
	PrintBuf(buf,k,(DocumentPeek)wind);
}

void CommandLineError(short errNum)
{
	if(errNum) {
		;
	} else
		SysBeep(7);
}


void ChangeDirectory(char *dirName)
{
	OSErr err;
	FSSpec spec;
	char *p;
	
	p=ConditionFileName(dirName);	
	if(!p || !*p)
		return;

	c2pstr(p);
	err=GetDirectoryFSSpec(DefaultSpecs.vRefNum,DefaultSpecs.parID,p,&spec);
	/*	The problem here is that the parent directory was returned. For
		directories, this is one level too high. */
	if(err==noErr) 
		DefaultSpecs=spec;
	else
		CommandLineError(0);
}

OSErr GetVolRefNum(char *name,short *volID)
{	
	OSErr err;
	HVolumeParam pB;
	Str255 temp;
	
	memset(&pB,0,sizeof(HVolumeParam));
	if(*name)
		BlockMove(name,temp,*name+1);
	else
		*temp='\0';
	pB.ioNamePtr=temp;
	pB.ioVolIndex=-1;
	err=PBHGetVInfoSync((HParmBlkPtr)&pB);
	*volID=pB.ioVRefNum;
	return err;
}
	
int AscendTheTree(short vRefNum,long dirID,char *buf,int k)
{
	OSErr err;
	Str63 name;
	HFileInfo pB;

	if(dirID==1)
		return k;
	memset(&pB,0,sizeof(HFileInfo));
	pB.ioDirID=dirID;
	pB.ioVRefNum=vRefNum;
	pB.ioNamePtr=name;
	pB.ioFDirIndex=-1;
	err=PBGetCatInfoSync((CInfoPBPtr)&pB);
	if(err!=noErr)
		return 0;
	if(dirID==2)
		return sprintf(buf,"%s:",p2cstr(name));
	k=AscendTheTree(vRefNum,pB.ioFlParID,buf+k,k);
	k+=sprintf(buf+k,"%s:",p2cstr(name));
	return k;
}

OSErr GetDirectoryFSSpec(short volRefNum,long dirID,
						ConstStr255Param fileName,FSSpecPtr spec)
{
	OSErr err;
	short newVolRefNum;
	long newDirID;
	HFileInfo pB;
	Str255 temp;

	err=GetWDInfo(volRefNum,&newVolRefNum,&newDirID,0);
	if(err==noErr) {
		dirID=(dirID)?dirID:newDirID;
		volRefNum=newVolRefNum;
	}
	
	if(*fileName) {
		/*	There is a pathname. */
		BlockMove(fileName,temp,*fileName+1);
		*(temp+*fileName+1)='\0';
		if(isfullpath(temp+1)) {
			err=GetVolRefNum(temp,&volRefNum);
			if(err!=noErr) {
				BlockMove(temp,spec->name,*temp+1);
				return err;
			}
			BlockMove(fileName,temp,*fileName+1);
			*(temp+*fileName+1)='\0';
		}
	} else
		*temp=0;
	
	memset(&pB,0,sizeof(HFileInfo));
	pB.ioNamePtr=temp;
	pB.ioDirID=dirID;
	pB.ioVRefNum=volRefNum;
	
	if(*temp) {
		/*  PBGetCatInfo with pB.ioFDirIndex==0 doesn't work if there
			is no file name. */
		err=PBGetCatInfoSync((CInfoPBPtr)&pB);
		if(err!=noErr)
			return err;
		if(!(pB.ioFlAttrib&16)) {
			/*	Whether we have returned a directory or not, we still need
				the correct directory name. The dirID is in the parID if we
				have returned a file. */
			pB.ioDirID=pB.ioFlParID;
		}
	}
	pB.ioFDirIndex=-1;
	*temp=0;
	err=PBGetCatInfoSync((CInfoPBPtr)&pB);
	spec->vRefNum=pB.ioVRefNum;
	spec->parID=pB.ioDirID;
	BlockMove(temp,spec->name,*temp+1);
	return err;
}

void FillPathName(char *buf,char *name)
{
	FSSpec spec;
	OSErr err;
	Str255 path;
	
	if(name) {
		strcpy(path,name);
		c2pstr(path);
	} else
		*path='\0';
		
	*buf='\0';
	err=GetDirectoryFSSpec(DefaultSpecs.vRefNum,DefaultSpecs.parID,path,&spec);
	if(err==noErr)
		AscendTheTree(spec.vRefNum,spec.parID,buf,0);
}

void Statistics(WindowPtr target, WindowPtr wind)
{
	TE32KHandle tH;
	Ptr p;
	long k,wordCnt,line,column,selStart;
	char endChar;
	Str255 data;
	
	if(target && wind) {
		tH = (TE32KHandle)((DocumentPeek)target)->docData;
		HLock((**tH).hText);
		p = *(**tH).hText;
		endChar = *(p+(**tH).teLength);
		*(p+(**tH).teLength)=0;
		wordCnt=0;
		while (*p) {
			while(*p && !isalnum(*p)) p++;
			if(isalnum(*p)) {
				wordCnt++;
				while(isalnum(*p)) p++;
			}
		}
		*(p+(**tH).teLength) = endChar;
		if(isalnum(endChar) && !isalnum(*(p+(**tH).teLength-1)))
			wordCnt++;
		HUnlock((**tH).hText);
		
		k=sprintf(data,"Number of characters: %d\nNumber of words: %d\n",(**tH).teLength,wordCnt);
		k+=sprintf(data+k,"Number of lines: %d\n",(**tH).nLines);
		PrintBuf(data,k,(DocumentPeek)wind);
		selStart = (**tH).selStart;
		line = indexToLine(selStart,tH);
		column = selStart-(**tH).lineStarts[line];
		k=sprintf(data,"Selection start: %d (line: %d, column: %d)\n",selStart,line+1,column+1);
		k+=sprintf(data+k,"Selection length: %d\n",(**tH).selEnd-selStart);
		PrintBuf(data,k,(DocumentPeek)wind);
	}
}

void Directory(WindowPtr wind,char *args)
{
	short i,volRefNum;
	long dirID,k;
	Boolean fFullPath,fLongFormat;
	char *fWildCards;
	HFileInfo pB;
	OSErr err;
	Str63 exp;
	Str255 name,path;
	char *fileName;
	
	fFullPath=CheckForFlag(args,"-f",2);
	fLongFormat=CheckForFlag(args,"-l",2);
	fileName=ConditionFileName(args);
	
	if(fWildCards=(fileName)?strpbrk(fileName,"?*[�"):false) {
		fWildCards=strrchr(fileName,':');
		fWildCards=(fWildCards)? fWildCards+1:fileName;
		strcpy(exp,fWildCards);
		*fWildCards='\0';			/* Remove wildcard part from fileName */
	}

	if(fLongFormat) {
		k=sprintf(name,"Name                  Type Crtr  Size       Last-Mod-Date       Creation-Date \n");
		PrintBuf(name,k,(DocumentPeek)wind);
		k=sprintf(name,"--------------------  ---- ---- ------ ------------------- -------------------\n");
		PrintBuf(name,k,(DocumentPeek)wind);
	}
		
	if(fFullPath)
		FillPathName(path,fileName);
	else
		*path='\0';
		
	memset(&pB,0,sizeof(HFileInfo));
	
	dirID=DefaultSpecs.parID;
	volRefNum=DefaultSpecs.vRefNum;
	if(fileName && *fileName) {
		c2pstr(fileName);
		*(fileName+*fileName+1)='\0';
		if(isfullpath(fileName+1)) {
			err=GetVolRefNum(fileName,&volRefNum);
			if(err!=noErr)
				return;
		}
		pB.ioDirID=dirID;
		pB.ioVRefNum=volRefNum;
		pB.ioNamePtr=fileName;
		if((err=PBGetCatInfoSync((CInfoPBPtr)&pB))==noErr) {
			if(pB.ioFlAttrib&16) {		/* It's a directory */
				dirID=pB.ioDirID;
				volRefNum=pB.ioVRefNum;
			} else if(!fWildCards || WildCardMatch(pB.ioNamePtr,exp)) {	/* It's a file */
				if(fLongFormat)
					PrintLongFormat(wind,&pB);
				else
					PrintFileName(wind,&pB,path);
				return;
			}
		} else
			return;
	}
	/* list an entire directory */
	fileName=name;
	for(i=1,err=0;err==noErr;i++) {
		*fileName=0;
		pB.ioDirID=dirID;
		pB.ioVRefNum=volRefNum;
		pB.ioFDirIndex = i;
		pB.ioNamePtr=fileName;
		if((err=PBGetCatInfoSync((CInfoPBPtr)&pB))==noErr) {
			if(!fWildCards || WildCardMatch(pB.ioNamePtr,exp)) {
				if(fLongFormat)
					PrintLongFormat(wind,&pB);
				else
					PrintFileName(wind,&pB,path);
			}
		}
	}
}

void ChangeTypeCreator(Str255 args, int changeWhat)
{
	char *p,ch=0,type[4],*fileName;
	int i;
	OSErr err;
	FInfo fndrInfo;

	p = args;
	if(*p == '\'' || *p == "\"") 
		ch = *p++;
	for(i=0;i<4;i++)
		type[i] = (*p==ch)? 0 : *p++ ;
	if(ch && *p==ch) 
		p++;
	fileName = ConditionFileName(p);
	c2pstr(fileName);
	
	err = HGetFInfo(DefaultSpecs.vRefNum,DefaultSpecs.parID,(StringPtr)fileName,&fndrInfo);
	if(err == noErr) {
		if(changeWhat)
			fndrInfo.fdType = *((OSType *)&type[0]);
		else
			fndrInfo.fdCreator = *((OSType *)&type[0]);
		err = HSetFInfo(DefaultSpecs.vRefNum,DefaultSpecs.parID,(StringPtr)fileName,&fndrInfo);
	}

}

void DoCommandLine(WindowPtr wind)
{
	char *s,*arg,*p;
	Boolean noAbort = true;
	long value;
	WindowPtr newWind;
	
	s=gCommandLine;
	do {
		while(isspace(*s)) s++;						/* s points to command */
		
		for(arg=s;isgraph(*arg);arg++) ;			/* arg point to argument(s) */
		while(isspace(*arg)) arg++;
		
		for(p=arg;*p && *p!=';';p++) ;				/* p points to end of command */
		if(*p==';')
			*p++='\0';
		
		switch (tolower(*s)) {
			case 'c':
				if(tolower(s[1])=='m' && tolower(s[2])=='l' && isspace(s[3])) {
					Main(0,0,gCommandLine);
					UnloadSeg(Main);
				} else if(s[1]=='d' && isspace(s[2])) 
					ChangeDirectory(arg);
				else if(!strncmp(s+1,"htyp",4) && isspace(s[5])) 
					ChangeTypeCreator(arg,1);
				else if(!strncmp(s+1,"hcre",4) && isspace(s[5]))
					ChangeTypeCreator(arg,0);
				break;
			case 'f':
				if(!strncmp(s+1,"ile",3) && isspace(s[4])) {
					gTextWindowPosition=wind;
					if(newWind=OpenNamedFile((StringPtr)arg))
						textDeactivate(newWind);
					else
						noAbort = false;
				} else if(!strncmp(s+1,"ind",3) && isspace(s[4])) 
					FindExpression(TargetWindow(),arg);
				break;
			case 'l':
				if(tolower(s[1])=='s' && (!s[2] || isspace(s[2]))) 
					Directory(wind,arg);
				else if(!strncmp(s+1,"ine",3) && (!s[4] || isspace(s[4]))) {
					value = atoi(arg);
					SelectLine(TargetWindow(),value);
				}
				break;
			case 'o':
				if(!strncmp(s+1,"pen",3) && (!s[4] || isspace(s[4]))) {
					if(newWind=OpenNamedFile((StringPtr)arg))
						SelectWindow(newWind);
					else
						noAbort = false;
				}
				break;
			case 's':
				if(!strncmp(s+1,"tats",4) && isspace(s[5])) 
					Statistics(TargetWindow(),wind);
				break;
			case ';':
				p++;
				break;
			default:
				break;
		}
	} while(noAbort && *(s=p));
	
	TE32KSelView((TE32KHandle)((DocumentPeek)wind)->docData);
	AdjustScrollBars(wind);
	SetScrollBarValues(wind);
}

void textDoKeyDown(WindowPtr wind,EventRecord *theEvent)
{
	int commandLen=0,tokenStart,tokenEnd,teLength;
	Boolean fCommandIsSelection=false,result;
	unsigned char *s, ch;
	TE32KHandle tH;
	long selStart,selEnd,delta;
	
	if(tH=(TE32KHandle)((DocumentPeek)wind)->docData) {
		ch=theEvent->message&charCodeMask;
		if(ch==3) {		/* ENTER character */
		
			/*
			If there is a selection, pass it to the command parser and make no 
			change to selection range. If there is no selection (selEnd==selStart), 
			pass the whole line to the command parser and add a newline to the end 
			of the line.
			*/
			
		/*	xorCaret(tH);	*/
		
			HLock((**tH).hText);
			tokenEnd = (**tH).selEnd;
			tokenStart = (**tH).selStart;
			commandLen = tokenEnd - tokenStart;
			if(tokenStart >= tokenEnd) {	
				/* 	
				No selection range, or selection range is screwed up (i.e. reversed). 
				Note: The TE32K routines are always checking for reversed (**tH).selStart, 
				(**tH).selEnd. I don't know of any cases where this happens, but I tread 
				softly.
				*/
				tokenEnd=tokenStart;
				s=(unsigned char *) *(**tH).hText;
				teLength = (**tH).teLength;
				for(;tokenStart>0 && *(s+tokenStart-1)!='\n' && *(s+tokenStart-1)!='\r';tokenStart--) ;
				for(;tokenEnd<teLength && *(s+tokenEnd)!='\n' && *(s+tokenEnd)!='\r';tokenEnd++) ;
				tokenEnd++;
				tokenStart = (tokenStart<0)? 0 : tokenStart;
				if(tokenEnd>teLength) {
					tokenEnd=teLength;
					TE32KSetSelect(tokenEnd,tokenEnd,tH);
				} else
					TE32KSetSelect(tokenEnd-1,tokenEnd-1,tH);
				commandLen = tokenEnd - tokenStart;
			} else
				fCommandIsSelection=true;
			if(commandLen<255) {
				BlockMove(*((**tH).hText) + tokenStart, gCommandLine,tokenEnd - tokenStart);
				*(gCommandLine+commandLen) = '\0';
			} else 
				commandLen = 0;
			HUnlock((**tH).hText);
		} 
		if(!fCommandIsSelection) {
			/* 
				There is not need to update marks during normal typing as the undoStart and undoEnd
				range provide an adequate record of how the buffer (hText) has changed. Two case
				where updating are necessary are flagged here:
					1. When auto-indenting and spaces follow the cursor
					2. A selection range made by shift-arrow key combinations is deleted, after 
						some previous typing.
				Number 2 is too complex, so lets just update marks whenever a shifted character is 
				typed. This catches all shift-arrow combinations. Updating with every new sentence
				is not an important penalty.
			*/
			if(ch==RETURN && (**tH).autoIndent 
					&& ((DocumentPeek)wind)->fNeedtoUpdateMarks
					&& ((*(**tH).hText + (**tH).selEnd)==' ' || (*(**tH).hText + (**tH).selEnd)==TAB)) {
				UpdateMarks((DocumentPeek)wind,(**tH).undoStart,(**tH).undoEnd,(**tH).undoStart-(**tH).undoEnd,(**tH).teLength);
			} else if(((DocumentPeek)wind)->fNeedtoUpdateMarks &&
				(ch==LEFTARROW || ch==RIGHTARROW || ch==UPARROW || ch==DOWNARROW)) {
				selStart = (**tH).undoStart;
				selEnd = (**tH).undoEnd;
				if(selEnd > selStart)
					delta = selEnd - selStart;
				else 
					delta=0;
				if((**tH).undoBuf)
					delta -= GetHandleSize((**tH).undoBuf);
				/*
					This is the second use of (**tH).undoDelta for input. The other is in 
					ContentClick, which is the other way of moving the insertion point.
				*/
				delta -= (**tH).undoDelta;
				(**tH).undoDelta = delta;
				if(selEnd > selStart)
					selEnd-=delta;
				UpdateMarks((DocumentPeek)wind,selStart,selEnd,delta,(**tH).teLength);
			}
				
			result = TE32KKey(ch,tH,theEvent->modifiers);
			
			((DocumentPeek)wind)->fNeedtoUpdateMarks |= result;
			((DocumentPeek)wind)->fNeedtoSave |= result;
					
			/* SynchScrollBars */
			AdjustScrollBars(wind);
			TE32KSelView(tH);
			SetScrollBarValues(wind);
			((DocumentPeek)wind)->fHaveSelection = SetSelectionFlag(tH);
		}
		if(commandLen)
			DoCommandLine(wind);
	}
}

void textWriteOnlyDoKeyDown(WindowPtr wind,EventRecord *theEvent)
{
	unsigned ch;
	TE32KHandle tH;
	
	if(tH=(TE32KHandle)((DocumentPeek)wind)->docData) {
		ch=theEvent->message&charCodeMask;

		if(ch==LEFTARROW || ch==RIGHTARROW || ch==UPARROW || ch==DOWNARROW) {
			
			TE32KKey(ch,tH,theEvent->modifiers);

			/* SynchScrollBars */
			AdjustScrollBars(wind);
			TE32KSelView(tH);
			SetScrollBarValues(wind);
			((DocumentPeek)wind)->fHaveSelection = SetSelectionFlag(tH);

		} else 
			doMessage(18);
	}
}

pascal void MyClicker(void)
{
	short		lineHeight;
	Rect		viewRect;
	Point		mousePoint;
	RgnHandle	saveClip;
	long		hDelta,vDelta;

	if (ClickedTE32KH) {
		LongRectToRect(&((**ClickedTE32KH).viewRect),&viewRect);
		lineHeight = (**ClickedTE32KH).lineHeight;
	
		hDelta = 0L;
		vDelta = 0L;
		
		GetMouse(&mousePoint);
		
		if (!PtInRect(mousePoint,&viewRect)) {
			if (mousePoint.v > viewRect.bottom && (**ClickedTE32KH).viewRect.bottom < (**ClickedTE32KH).destRect.top + (long) lineHeight * (**ClickedTE32KH).nLines)
				vDelta = -lineHeight;
			
			else if (mousePoint.v < viewRect.top && (**ClickedTE32KH).viewRect.top > (**ClickedTE32KH).destRect.top)
				vDelta = lineHeight;
			
			
			if (mousePoint.h > viewRect.right && (**ClickedTE32KH).viewRect.right < (**ClickedTE32KH).destRect.right)
				hDelta = -lineHeight;
			
			else if (mousePoint.h<viewRect.left && (**ClickedTE32KH).viewRect.left > (**ClickedTE32KH).destRect.left)
				hDelta = lineHeight;
		}
		
		if (hDelta || vDelta) {
			saveClip = NewRgn();
			GetClip(saveClip);
			viewRect = (*((**ClickedTE32KH).inPort)).portRect;
			ClipRect(&viewRect);
			
			TE32KScroll(hDelta,vDelta,ClickedTE32KH);
			AdjustScrollBars((**ClickedTE32KH).inPort);
			SetScrollBarValues((**ClickedTE32KH).inPort);
			
			SetClip(saveClip);
			DisposeRgn(saveClip);
		}
	}
}

void AddText(WindowPtr wind,Ptr text, long len)
{
	TE32KHandle tH;
	
	if(tH=(TE32KHandle)((DocumentPeek)wind)->docData) {
		TE32KInsert(text,len,tH);
		((DocumentPeek)wind)->fNeedtoUpdateMarks = ((DocumentPeek)wind)->fNeedtoSave = true;
		/* SynchScrollBars */
		TE32KSelView(tH);
		AdjustScrollBars(wind);
		SetScrollBarValues(wind);
	}
}

Boolean SetTERect(WindowPtr wind)
{
	Rect r;
	TE32KHandle tH;
	
	if(((DocumentPeek)wind)->docData) {
		tH = (TE32KHandle)((DocumentPeek)wind)->docData;
		GetContentRect(wind,&r);
		RectToLongRect(&r,&(**tH).viewRect);
		(**tH).viewRect.bottom=(**tH).viewRect.top+((r.bottom-r.top)/(**tH).lineHeight)*(**tH).lineHeight;
		if((**tH).crOnly)
			(**tH).destRect.right=(**tH).destRect.left+8192;
		else if((**tH).wrapToLength) {
			(**tH).destRect.right=(**tH).destRect.left+(**tH).maxLineWidth*(**tH).theCharWidths['M'];
		} else {
			(**tH).destRect.right=(**tH).viewRect.right;
			(**tH).destRect.left=(**tH).viewRect.left;
		}
		return !(**tH).crOnly;
	}
	return false;
}

void textDoGrow(WindowPtr wind,EventRecord *event)
{	
	Rect r;
	TE32KHandle tH;
	
	if(doGrow(wind,event)) {
		SizeScrollBars(wind);
		if(SetTERect(wind)) {
			TE32KCalText(tH=(TE32KHandle)((DocumentPeek)wind)->docData);
			TE32KSetSelect((**tH).selStart,(**tH).selEnd,tH);
		}
		AdjustScrollBars(wind);
		SetScrollBarValues(wind);
		GetContentRect(wind,&r);
		InvalRect(&r);
		EraseRect(&r);
	}
}

void textDoZoom(WindowPtr wind,short partCode)
{
	TE32KHandle tH;
	Rect r;
	
	scrollDoZoom(wind,partCode);
	if(SetTERect(wind)) {	/* If wordwrapping, CalText. */
		TE32KCalText(tH=(TE32KHandle)((DocumentPeek)wind)->docData);
		TE32KSetSelect((**tH).selStart,(**tH).selEnd,tH);
		AdjustScrollBars(wind);
		SetScrollBarValues(wind);
	}
	GetContentRect(wind,&r);
	InvalRect(&r);
	EraseRect(&r);
}

void textDoContent(WindowPtr wind,EventRecord *event)
{
	Rect contents;
	Boolean shiftKeyDown;
	TE32KHandle tH;
	long selStart,selEnd,delta;
	
	FocusOnWindow(wind);
	GlobalToLocal(&event->where);
	(*((DocumentPeek)wind)->getContentRect)(wind,&contents);
	if(PtInRect(event->where,&contents)) {
		shiftKeyDown =((event->modifiers & shiftKey) != 0);
		/*
			This is the first use of (**tH).undoDelta as input. UndoDelta
			keeps a record of previous UpdateMarks. Obviously, one can click 
			many times and there be no change in the undoBuffer and UpdateMarks 
			based on the undoBuffer values shouldn't be redone.
			
			The other use is for arrow key cursor movement.
		*/
		if(tH=(TE32KHandle)((DocumentPeek)wind)->docData) {
			if(((DocumentPeek)wind)->fNeedtoUpdateMarks) {
				selStart = (**tH).undoStart;
				selEnd = (**tH).undoEnd;
				if(selEnd > selStart)
					delta = selEnd - selStart;
				else
					delta = 0;
				if((**tH).undoBuf)
					delta -= GetHandleSize((**tH).undoBuf);
				delta -= (**tH).undoDelta;
				(**tH).undoDelta = delta;
				if(selEnd > selStart)
					selEnd-=delta;
				UpdateMarks((DocumentPeek)wind,selStart,selEnd,delta,(**tH).teLength);
			}
			
			TE32KClick(event->where,shiftKeyDown,tH);
			
			((DocumentPeek)wind)->fHaveSelection = SetSelectionFlag(tH);
		}
	} else
		ScrollClick(wind,event);
}

RectLocalToGlobal(Rect *r)
{
	Point pt;
	
	pt.h=r->left;
	pt.v=r->top;
	LocalToGlobal(&pt);
	r->left=pt.h;
	r->top=pt.v;

	pt.h=r->right;
	pt.v=r->bottom;
	LocalToGlobal(&pt);
	r->right=pt.h;
	r->bottom=pt.v;
}

Boolean inStdState(WindowPtr wind,Rect *r)
{
	Handle h;
	WStateData *ws;
	Boolean ret;
	GrafPtr oldPort;
	
	if(!((WindowPeek)wind)->spareFlag)
		return false;
	h=((WindowPeek)wind)->dataHandle;
	if(!h)
		return false;
	HLock(h);
	ws = (WStateData *)(*h);
	*r=wind->portRect;
	GetPort(&oldPort);
	SetPort(wind);
	RectLocalToGlobal(r);
	SetPort(oldPort);
	ret=EqualRect(r,&ws->stdState);
	HUnlock(h);
	return ret;
}

Boolean textMakeWindow(WindowPtr wind)
{
	Rect r;
	short hasGoAway;
	char *title;
	
	title=((DocumentPeek)wind)->fileSpecs.name;

	SetRect(&r,DragBoundsRect.left+5+25*NumWindows,
						DragBoundsRect.top+25+25*NumWindows,
						DragBoundsRect.right-40,
						DragBoundsRect.bottom);
	if(!((r.right-r.left)%2)) r.right--;
	if(!((r.bottom-r.top)%2)) r.bottom--;
	
	/* 	If we are supposed to be using a worksheet and there isn't one yet, 
		then this is the worksheet. */
		
	hasGoAway=(gUseWorksheet)? gWorkSheetDoc!=0 : 1;

	wind=NewWindow((Ptr)wind,&r,"",false,zoomDocProc,gTextWindowPosition,hasGoAway,0);
	gTextWindowPosition=(WindowPtr)-1;
	
	((DocumentPeek)wind)->fDocWindow=true;
	((WindowPeek)wind)->windowKind = (hasGoAway)? 9:13;
	NumWindows++;
	if(*title) {
		SetDocWindowTitle(wind,title);
	}
	return true;
}

Boolean textInitDoc(WindowPtr wind)
{
	DocumentPeek doc;
	LongRect view,dest;
	TE32KHandle tH;

	doc = (DocumentPeek)wind;
	
	SetRect(&doc->limitRect,100,100,1000,1000);
	
	if(!(doc->fPrintRecord = (THPrint)NewHandle(sizeof(TPrint))))
		return false;
	PrOpen();
	PrintDefault(doc->fPrintRecord);
	PrClose();

	SetPort(wind);
	TextFont(monaco);
	TextSize(9);
	RectToLongRect(&wind->portRect,&dest);
	view=dest;
	dest.right=2000;
	dest.bottom=2000;
	dest.left+=4;
	dest.top+=4;
	tH=TE32KNew(&dest,&view);
	if(tH==NIL) {
		DisposeWindow(wind);
		doc->docData=NIL;
		doc->fDocWindow=false;
		InitCursor();
		doMessage(5);	
		return false;
	}
	(**tH).crOnly=false;
	doc->docData=(Handle)tH;
		
	if(!InitScrollDoc(wind))
		return false;
		
	SetTERect(wind);

	TE32KAutoView(true,tH);
		
	DrawControls(wind);
	DrawGrowIcon(wind);
	return true;
}

unsigned long GetModDate(FSSpec *specs)
{
	HFileParam pB;
	
	memset(&pB,0,sizeof(HFileParam));
	pB.ioCompletion=0;
	pB.ioNamePtr=specs->name;
	pB.ioVRefNum=specs->vRefNum;
	pB.ioFDirIndex=0;
	pB.ioDirID=specs->parID;
		
	PBHGetFInfoSync((HParmBlkPtr)&pB);
	
	return pB.ioFlMdDat;
}

int ReadTextDocResourceFork(WindowPtr wind)
{
	Handle res;
	short err,refNum,nMarks;

	/*	
		Text files have two resources. Both follow formats used by MPW.
	
		MPSR 1005 is obligatory and encodes all format information, window
		size, scroll bar position and selection range.
		
		MPSR 1007 encodes marks.
	*/
	
	refNum=((DocumentPeek)wind)->resourcePathRefNum;
			
	if(!refNum)
		return false;
		
	UseResFile(refNum);
	res=Get1Resource('MPSR',1005);
	
	((DocumentPeek)wind)->windowState=res;
	
	/* 	Retrieve the file's modified date so that it can be retained should 
		file's resources be changed but the file not dirtied.  Changes in
		marks, scroll position, window size, and selection range do not
		dirty a text file. Neither does show invisibles, or any font changes. */
	
	if(res) 
		((MPSRPtr)*res)->modifiedDate=GetModDate(&((DocumentPeek)wind)->fileSpecs);

	/* Now see if there are already marks for this file. */
	
	res=Get1Resource('MPSR',1007);
	((DocumentPeek)wind)->marks=(ResError()==noErr)? res:0;
	if(res=((DocumentPeek)wind)->marks) {
		nMarks = *((short *)*res);
		if(nMarks == 0)	{	/* 	There are no marks. This can only 
										happen if a foreign file, i.e. MPW,
										has opened this file and left an
										MPSR 1007 resource that should have
										been removed. */
			RmveResource(res);
			err=ResError();
			DisposeHandle(res);
			((DocumentPeek)wind)->marks=0;
		}
	}
	
	return true;
}

void readOnlyAdjustDocMenus(WindowPtr wind)
{
	
	/* File Menu */
	EnableItem(FileMenu,iClose);
	
	DisableItem(FileMenu,iSave);
	EnableItem(FileMenu,iSaveAs);
	EnableItem(FileMenu,iSaveACopy);
	DisableItem(FileMenu,iRevert);
	EnableItem(FileMenu,iPageSetup);
	EnableItem(FileMenu,iPrint);
		
	/* Edit Menu */
	DisableItem(EditMenu,iUndo);
	DisableItem(EditMenu,iCut);
	SetMenuAbility(EditMenu,iCopy,((DocumentPeek)wind)->fHaveSelection);
	DisableItem(EditMenu,iPaste);	
	DisableItem(EditMenu,iClear);
	EnableItem(EditMenu,iSelectAll);
	EnableItem(EditMenu,iShowClipboard);
	DisableItem(EditMenu,iFormat);
	
	DisableItem(EditMenu,iAlign);
	DisableItem(EditMenu,iShiftRight);
	DisableItem(EditMenu,iShiftLeft);
	
	/* Find Menu */
	EnableItem(FindMenu,iFind);
	SetMenuAbility(FindMenu,iFindSame,*gFindBuffer);
	SetMenuAbility(FindMenu,iFindSelection,((DocumentPeek)wind)->fHaveSelection>0);
	EnableItem(FindMenu,iDisplaySelection);	
	DisableItem(FindMenu,iReplace);
	DisableItem(FindMenu,iReplaceSame);

	EnableItem(FileMenu,0);
	EnableItem(EditMenu,0);
	EnableItem(FindMenu,0);
	EnableItem(MarkMenu,0);
	EnableItem(WindowMenu,0);

}

Boolean textOpenDocFile(WindowPtr wind)
{
	DocumentPeek doc;
	
	if(openDocFile(wind)) {
		doc = (DocumentPeek)wind;
		if(doc->fReadOnly) {
			doc->doKeyDown=textWriteOnlyDoKeyDown;
			doc->writeDocFile=nullIntMethod;
			doc->adjustDocMenus=readOnlyAdjustDocMenus;
		} else {
			doc->doKeyDown=textDoKeyDown;
			doc->writeDocFile=textWriteDocFile;
			doc->adjustDocMenus=textAdjustDocMenus;
		}
		return true;
	}
	return false;
}

int ReadDefaultResourceFork(TE32KHandle tH)
{
	Handle h;
	short refNum,curRefNum,fontNum;
	
	refNum = gPrefsResRefNum;
	if(!refNum)
		return 0;
	
	curRefNum = CurResFile();
	UseResFile(refNum);
		
	h = Get1Resource('MPSR',1005);

	if(h==-1)
		return 0;
		
	HLock(h);
	(**tH).showInvisibles=((MPSRPtr)*h)->showInvisibles;
	(**tH).crOnly = !(((MPSRPtr)*h)->wordWrap&2);
	(**tH).wrapToLength = ((MPSRPtr)*h)->wordWrap&4;
	(**tH).autoIndent = ((MPSRPtr)*h)->wordWrap&1;
	(**tH).tabChars=((MPSRPtr)*h)->tabWidth;
	GetFNum(c2pstr(((MPSRPtr)*h)->fontName),&fontNum);
	p2cstr(((MPSRPtr)*h)->fontName);
	TE32KSetFontStuff(fontNum,(**tH).txFace,(**tH).txMode,((MPSRPtr)*h)->fontSize,tH);
	HUnlock(h);
	
	UseResFile(curRefNum);
	
	return 1;
}

int textReadDocFile(WindowPtr wind)
{
	DocumentPeek doc;
	short refNum,fontNum;
	long len,selEnd,selStart;
	OSErr err;
	Handle theText;
	TE32KHandle tH;	
	
	doc = (DocumentPeek)wind;
	
	if((doc->fDocWindow)&&(doc->docData)) {
		refNum=doc->dataPathRefNum;
		err = GetEOF(refNum,&len);

		theText=NewHandle(len);
		if(MemError() || StripAddress(theText)==nil) {
			doMessage(1);
			return false;
		}
		
		HLock(theText);
		err = SetFPos(refNum,fsFromStart,0);
		err = FSRead(refNum,&len,(Ptr)*theText);
		HUnlock(theText);
		if(err == noErr) {
			tH = (TE32KHandle)doc->docData;
			if((**tH).hText)
				DisposeHandle((**tH).hText);
			(**tH).hText=theText;
			(**tH).teLength=len;
			(**tH).selStart=len;
			(**tH).selEnd=len;
			
			ReadTextDocResourceFork(wind);
						
			if(doc->windowState) {
				HLock(doc->windowState);
				(**tH).showInvisibles=((MPSRPtr)*doc->windowState)->showInvisibles;
				(**tH).crOnly = !(((MPSRPtr)*doc->windowState)->wordWrap&2);
				(**tH).wrapToLength = ((MPSRPtr)*doc->windowState)->wordWrap&4;
				(**tH).autoIndent = ((MPSRPtr)*doc->windowState)->wordWrap&1;
				(**tH).tabChars=((MPSRPtr)*doc->windowState)->tabWidth;
				GetFNum(c2pstr(((MPSRPtr)*doc->windowState)->fontName),&fontNum);
				p2cstr(((MPSRPtr)*doc->windowState)->fontName);
				TE32KSetFontStuff(fontNum,(**tH).txFace,(**tH).txMode,((MPSRPtr)*doc->windowState)->fontSize,tH);
				HUnlock(doc->windowState);
			} else ReadDefaultResourceFork(tH);
		
			SetTERect(wind);
			TE32KCalText(tH);

			if(doc->windowState) {
				selStart=((MPSRPtr)*doc->windowState)->vScrollValue;
				selEnd=indexToLine(selStart,tH);
				ScrollContents(wind,(short)0,(short)-selEnd);
				
				selStart=((MPSRPtr)*doc->windowState)->selStart;
				selEnd=((MPSRPtr)*doc->windowState)->selEnd;
				if(selEnd>selStart)
					doc->fHaveSelection=true;
				TE32KSetSelect(selStart,selEnd,tH);
			}
			AdjustScrollBars(wind);
			SetScrollBarValues(wind);
			
			return true;
		} else {
			DisposeHandle(theText);
			return false;
		}
	}
	return false;
}

int textWriteDocResourceFork(WindowPtr wind)
{
	DocumentPeek doc;
	short val,nMarks;
	Handle h,res;
	Rect r;
	WStateData *ws;
	MPSRPtr state;
	TE32KHandle tH;
	Str255 fontName;
	
	doc = (DocumentPeek) wind;
	if(!doc->resourcePathRefNum)
		return 0;
		
	UseResFile(doc->resourcePathRefNum);
	
	res=doc->windowState;
	if(!doc->windowState) {
		/* Resource is missing. Make one. */
		res=NewHandle(sizeof(MPSRWindowResource));
		if(!res)
			return 0;	/* Not enough memory */
		HLock(res);
		state=(MPSRPtr)*res;
		state->modifiedDate = GetModDate(&doc->fileSpecs);
	}
	
	HLock(res);
	state=(MPSRPtr)*res;
		
	if(inStdState((WindowPtr)doc,&r)) {
		h=((WindowPeek)doc)->dataHandle;
		HLock(h);
		ws = (WStateData *)(*h);
		state->userState=ws->stdState;
		state->stdState=ws->userState;
		HUnlock(h);
	} else {
		state->userState=r;
		state->stdState=r;
	}
	
	tH=(TE32KHandle)doc->docData;
	state->selStart=(**tH).selStart;
	state->selEnd=(**tH).selEnd;
	val=GetCtlValue(doc->vScroll);
	state->vScrollValue = (**tH).lineStarts[val];

	state->showInvisibles=(**tH).showInvisibles;
	state->wordWrap=(((**tH).autoIndent)?1:0) | (((**tH).crOnly)?0:2) |(((**tH).wrapToLength)?4:0);
	state->tabWidth=(**tH).tabChars;
	state->fontWidth=(**tH).theCharWidths[' '];
	state->fontSize=(**tH).txSize;
	
	GetFontName((**tH).txFont,fontName);

	strcpy(state->fontName,p2cstr(fontName));

	HUnlock(res);
		
	if(!doc->windowState) {
		doc->windowState=res;
		AddResource(res,'MPSR',1005,"");
	} else
		ChangedResource(doc->windowState);
		
	WriteResource(doc->windowState);
	
	if(doc->marks) {
		nMarks = *(short *)*doc->marks ;
		if(nMarks) {
			if(HomeResFile(doc->marks)!=-1) {
				ChangedResource(doc->marks);
				WriteResource(doc->marks);
			} else  {
				AddResource(doc->marks,'MPSR',1007,"");
				WriteResource(doc->marks);
			}
		} else {
			if(res) {
				RmveResource(res);
				DisposeHandle(res);
			}
		}
	}		
}

int textWriteDocFile(WindowPtr wind)
{
	DocumentPeek doc;
	short refNum;
	OSErr err;
	long len;
	Handle theText;
	TE32KHandle tH;
	unsigned long modDate;
	
	doc = (DocumentPeek)wind;
	
	if((doc->fDocWindow)&&(doc->docData)) {
		textWriteDocResourceFork(wind);
		refNum=doc->dataPathRefNum;
		tH = (TE32KHandle)doc->docData;
		len = (**tH).teLength;
		theText = (**tH).hText;
		HLock(theText);
		err = SetFPos(refNum,fsFromStart,0);
		err |= FSWrite(refNum,&len,*theText);
		HUnlock(theText);
		err |= SetEOF(refNum,len);
		err |= FlushVol("",doc->fileSpecs.vRefNum);
		if(err==noErr) {
			if(doc->windowState) {
				GetDateTime(&modDate);
				((MPSRPtr)*doc->windowState)->modifiedDate = modDate;
			}
			return true;
		}
	}
	return false;
}

void textAdjustDocMenus(WindowPtr wind)
{
	DocumentPeek doc;
	long selStart,selEnd,delta=0;
	TE32KHandle tH;

	
	doc=(DocumentPeek)wind;
	
	if(doc->fNeedtoUpdateMarks && (tH=(TE32KHandle)doc->docData)) {
		selStart = (**tH).undoStart;
		selEnd = (**tH).undoEnd;
		if(selEnd > selStart)
			delta = selEnd - selStart;
		if((**tH).undoBuf) 
			delta -= GetHandleSize((**tH).undoBuf);
		delta -= (**tH).undoDelta;
		(**tH).undoDelta = delta;
		if(selEnd > selStart)
			selEnd-=delta;
		UpdateMarks((DocumentPeek)wind,selStart,selEnd,delta,(**tH).teLength);
	}
	
	// File Menu
	SetMenuAbility(FileMenu,iClose,((WindowPeek)doc)->windowKind!=13);
		// Can't close Worksheet
	SetMenuAbility(FileMenu,iSave,doc->fNeedtoSave);
	SetMenuAbility(FileMenu,iSaveAs,((WindowPeek)doc)->windowKind!=13);
	EnableItem(FileMenu,iSaveACopy);
	SetMenuAbility(FileMenu,iRevert,doc->fNeedtoSave);
	EnableItem(FileMenu,iPageSetup);
	EnableItem(FileMenu,iPrint);
		
	// Edit Menu
	SetMenuAbility(EditMenu,iUndo,doc->fNeedtoSave);
	SetMenuAbility(EditMenu,iCut,doc->fHaveSelection);
	SetMenuAbility(EditMenu,iCopy,doc->fHaveSelection);
	EnableItem(EditMenu,iPaste);	
	SetMenuAbility(EditMenu,iClear,doc->fHaveSelection);
	EnableItem(EditMenu,iSelectAll);
	EnableItem(EditMenu,iShowClipboard);
	EnableItem(EditMenu,iFormat);
	
	EnableItem(EditMenu,iAlign);
	EnableItem(EditMenu,iShiftRight);
	EnableItem(EditMenu,iShiftLeft);
	
	// Find Menu
	DisableItem(FindMenu,iSelectSurvey);
	EnableItem(FindMenu,iFind);
	SetMenuAbility(FindMenu,iFindSame,*gFindBuffer);
	SetMenuAbility(FindMenu,iFindSelection,doc->fHaveSelection>0);
	EnableItem(FindMenu,iDisplaySelection);	
	EnableItem(FindMenu,iReplace);
	SetMenuAbility(FindMenu,iReplaceSame,*gFindBuffer);
	
	// View Menu
	/* 	Items initialized as enabled and should never
		be disabled by another document. */

	// Data Menu
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

int forSearch(char *buf, char *end,const char *what, int len)
{
	int i,offset;
	
	for(offset=0;buf<end;buf++,offset++) {
		if(tolower(*buf) == tolower(*what)) {
			for(i=1;i<len;i++) {
				if(tolower(*(buf+i)) != tolower(*(what+i)))
					goto NoMatch;
			}
			return offset;
		}
NoMatch:	;
	}
	return -1;
}

int forCSSearch(char *buf, char *end, const char *what, int len)
{
	int i,offset;
	
	for(offset=0;buf<end;buf++,offset++) {
		if(*buf == *what) {
			for(i=1;i<len;i++) {
				if(*(buf+i) != *(what+i))
					goto NoMatch;
			}
			return offset;
		}
NoMatch:	;
	}
	return -1;
}

int revSearch(char *buf, char *start, const char *what, int len)
{
	int i,offset;
	
	for(offset=-1;buf>=start;buf--,offset--) {
		if(toupper(*buf) == toupper(*what)) {
			for(i=1;i<len;i++) {
				if(toupper(*(buf+i)) != toupper(*(what+i)))
					goto NoMatch;
			}
			return offset;
		}
NoMatch:	;
	}
	return 0;
}

int revCSSearch(char *buf, char *start,const char *what, int len)
{
	int i,offset;
	
	for(offset=-1;buf>=start;buf--,offset--) {
		if(*buf == *what) {
			for(i=1;i<len;i++) {
				if(*(buf+i) != *(what+i))
					goto NoMatch;
			}
			return offset;
		}
NoMatch:	;
	}
	return 0;
}

int RSearch(Handle h,int offset,const char *what,int len)
{
	int where,notFound;
	
	HLock(h);
	notFound = (gSearchMethod==1);
	do {
		where = (gCaseSensitive)? 
					revCSSearch(*h+offset-1,*h,what,len) : 
					revSearch(*h+offset-1,*h,what,len) ;
		offset+=where;
		if(where) {
			if(gSearchMethod==1) {		/* Whole word */
				if(isalnum(*(*h+offset+len)) || (offset && isalnum(*(*h+offset-1)))) 
					;
				else
					notFound = 0;	/* Trip out */
			}
		} else
			notFound = 0;
	} while (notFound) ;
	HUnlock(h);
	return (where)? offset : -1;
}

char *matchLiteral(char *buf,char *lit,int *foundLen,int *litLen)
{
	char *p,*match;
	int len1=0,len2=0;
	Str255 searchStr;
	
	/* fill the search string */
	p=searchStr;
	while(*lit && *lit!='*' && *lit!='�' && *lit!='?' && *lit!='[') {
		*p++ = *lit ++;
		len1++;
		len2++;
	}
	*p='\0';
	*foundLen+=len1;
	*litLen+=len2;
	if(gCaseSensitive) {
		match=strstr(buf,searchStr);
		buf+=len1;
	} else {
		for(p=searchStr;*p;p++)
			*p=toupper(*p);
		p=searchStr;
		while(*buf) {
			while(*buf && *p!=toupper(*buf)) buf++;
			if(*(match=buf)) {
				p++;
				buf++;
				while(*p) {
					if(*p!=toupper(*buf)) {
						p=searchStr;
						buf++;
						match=0;
						break;
					}
					p++;
					buf++;
				}
				if(!*p)
					break;
			}
		}
	}
	if(match) {
		len1=len2=0;
		if(matchExpressionRight(buf,lit+len2,&len1,&len2)) {
			*foundLen+=len1;
			*litLen+=len2;
			return match;
		}
	}
	return 0;
}

char *matchRange(char *buf,char *lit,int *foundLen,int *rangeLen)
{
	char *p,*match;
	int len1=0,len2=0,repeats=false;
	Str255 searchStr;
	
	/* fill the search string */
	p=searchStr;
	while(*lit && *lit!=']') {
		*p++ = *lit ++;
		len2++;
	}
	if(!*lit) {
		CommandLineError(0);
		return 0;
	}
	len2++;
	if(*(p-1)=='*') {
		repeats=true;
		p--;
	}
	*p='\0';
	
	if(match=strpbrk(buf,searchStr)) {
		len1=(repeats)?strspn(match,searchStr):1;
		*foundLen+=len1;
		len1=len2=0;
		if(matchExpressionRight(buf+len1,lit+len2,&len1,&len2)) {
			*foundLen+=len1;
			*rangeLen+=len2;
			return match;
		}
	}
	return 0;
}

char *matchExpressionRight(char *buf,char *exp,int *foundLen,int *expLen)
{
	char *anchor,*start,*match;
	Boolean doLeft=false;
	
	anchor=exp;
	start=buf;
	for(;;) {
		if(*anchor=='?') {
			start++;
			doLeft=true;
		} else if(*anchor=='*' || anchor=='�') 
			doLeft=true;
		else
			break;
		anchor++;
	}
	if(!*anchor)
		match=start;
	else if(*anchor=='[') 
		match=matchRange(start,anchor+1,foundLen,expLen);
	else
		match=matchLiteral(start,anchor,foundLen,expLen);
	if(match && doLeft) {
		while(anchor>exp) {
			anchor--;
			if(*anchor=='?') {
				match--;
				(*foundLen)++;
				(*expLen)++;
			} else if(*anchor=='*' || *anchor=='�') {
				while(match>buf && *match!='\n') {
					match--;
					(*foundLen)++;
				}
				if(*match=='\n') {
					match++;
					(*foundLen)--;
				}
			}
			(*expLen)++;
		}
	}
	return match;
}

int ExpressionMatch(char *buf,char *end,char *exp,int *expLen)
{
	
	char storage,*found;
	int len1=0,len2=0;
	
	storage = *(end-1);
	*(end-1)='\0';
	found=matchExpressionRight(buf,exp,&len1,&len2);
	*expLen=len1;
	*(end-1)=storage;
	if(found)
		return found-buf+1;
	else
		return -1;
}

int ExpressionSearch(Handle h,int offset,const char *what,int *len)
{
	int where,hLen,foundLen;
	Str255 expressionString;
	
	BlockMove(what,expressionString,*len);
	*(expressionString+*len)='\0';
	hLen = GetHandleSize(h);
	HLock(h);
	where = ExpressionMatch(*h+offset+1,*h+hLen,expressionString,&foundLen);
	offset+=where;
	*len=foundLen;
	if(where && gSearchMethod==1) {
		if(isalnum(*(*h+offset+*len-1)) || (offset<hLen && isalnum(*(*h+offset+1)))) {
			HUnlock(h);
			return -1;
		}
	}	
	HUnlock(h);
	return (where)? offset : -1;
}

int FSearch(Handle h,int offset,const char *what,int len)
{
	int where,hLen;
	
	hLen = GetHandleSize(h);
	HLock(h);
	
	do {
		where = (gCaseSensitive)? 
					forCSSearch(*h+offset,*h+hLen,what,len) : 
					forSearch(*h+offset,*h+hLen,what,len) ;
		if(where>=0) {
			offset+=where;
			if(where && gSearchMethod==1) {
				if(isalnum(*(*h+offset-1)) || 
					((offset+len)<hLen && isalnum(*(*h+offset+len)))) 
						offset+=len;
				else
					where = -1; /* Trip out */
			} else 
				where = -1;		/* Found string. Trip out. */
		} else /* where == -1 means string not found. Exit. */
			offset = -1;
	} while (where>=0) ;
	HUnlock(h);
	return offset;
}

Boolean findSearchString(WindowPtr wind, short direction)
{
	int where,end,lastLine,foundLength;
	TE32KHandle tH;

	if(tH=(TE32KHandle)((DocumentPeek)wind)->docData) {
		if(gSearchMethod==2) {	/* Selection expression */
			if(isdigit(*(gFindBuffer+1))) {
				lastLine=(**tH).nLines-1;
				/* Since nLines isn�t right if the last character is a return, check for that case.
					(hint from TESample example. */
				if ( *(*(**tH).hText + (**tH).teLength - 1) == '\n' )
					lastLine++;
				
				*(gFindBuffer + *gFindBuffer + 1) = '\0';
				where=atoi(gFindBuffer+1);
				where=(where>lastLine)? lastLine : where ;
				where=(where<=0)? 1 : where;
				end = (**tH).lineStarts[where];
				where = (**tH).lineStarts[where-1];
			} else {
				foundLength=*gFindBuffer;
				where=ExpressionSearch((**tH).hText,(**tH).selEnd,gFindBuffer+1,&foundLength);
				if(where<0 && gWrapSearch) {	/* not found, try one more time, from start */
					foundLength=*gFindBuffer;
					where=ExpressionSearch((**tH).hText,0,gFindBuffer+1,&foundLength);
					if(where==(**tH).selStart && foundLength==((**tH).selEnd-(**tH).selStart))
						where=-1;
				}
				end = where + foundLength;
			}
		} else {
			if((direction)? !gSearchBackwards : gSearchBackwards)
				where=RSearch((**tH).hText,(**tH).selStart,gFindBuffer+1,(long)*gFindBuffer);
			else
				where=FSearch((**tH).hText,(**tH).selEnd,gFindBuffer+1,(long)*gFindBuffer);
			if(where<0 && gWrapSearch) {	/* not found, try one more time, from start */
				if((direction)? !gSearchBackwards : gSearchBackwards)
					where=RSearch((**tH).hText,(**tH).teLength,gFindBuffer+1,(long)*gFindBuffer);
				else
					where=FSearch((**tH).hText,0,gFindBuffer+1,(long)*gFindBuffer);
				if(where==(**tH).selStart && *gFindBuffer==((**tH).selEnd-(**tH).selStart))
					where=-1;
			}
			end = where + *gFindBuffer;
		}
		if(where>=0) {
			TE32KSetSelect(where,end,tH);
			TE32KSelView(tH);			
			AdjustScrollBars(wind);
			SetScrollBarValues(wind);
			((DocumentPeek)wind)->fHaveSelection = 1;
			return true;
		}
		SysBeep(7);
	}
	return false;
}

pascal Boolean DialogStandardKeyDown(DialogPtr dialog, EventRecord *theEvent, short *itemHit)
{
	Rect box;
	Handle itemHand;
	short itemType;
	char key;
	
	if(theEvent->what==keyDown) {
		key=theEvent->message&charCodeMask;
		switch(key) {
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
			default:
				return false;
		}
	}
	return false;
}

Boolean doFindDialog(short which)
{
	DialogPtr findDialog;
	short itemHit;
	short itemType;
	Handle itemHand;
	int pau=false,result=false;
	Rect box;
	WindowPtr targetWindow;
	
	targetWindow=FrontLayer();
	findDialog=GetNewDialog(which,(Ptr)&DlogStor,(WindowPtr)(-1));
	/* Note: map Dialog is 137 */
	if(!findDialog) 
		FatalError();

	GetDItem(findDialog,11,&itemType,&itemHand,&box);
	SetDItem(findDialog,11,itemType,(Handle)doLine,&box);
	GetDItem(findDialog,12,&itemType,&itemHand,&box);
	SetDItem(findDialog,12,itemType,(Handle)doLine,&box);
	GetDItem(findDialog,13,&itemType,&itemHand,&box);
	SetDItem(findDialog,13,itemType,(Handle)doButton,&box);
	GetDItem(findDialog,4,&itemType,&itemHand,&box);
	SetIText(itemHand,gFindBuffer);
	SelIText(findDialog,4,0,32767);
	while(!pau) {
		/* Initialize the three radio buttons */
		GetDItem(findDialog,5,&itemType,&itemHand,&box);
		SetCtlValue((ControlHandle)itemHand,gSearchMethod==0);
		GetDItem(findDialog,6,&itemType,&itemHand,&box);
		SetCtlValue((ControlHandle)itemHand,gSearchMethod==1);
		GetDItem(findDialog,7,&itemType,&itemHand,&box);
		SetCtlValue((ControlHandle)itemHand,gSearchMethod==2);
		/* Do the check boxes */
		GetDItem(findDialog,8,&itemType,&itemHand,&box);
		SetCtlValue((ControlHandle)itemHand,gCaseSensitive);
		GetDItem(findDialog,9,&itemType,&itemHand,&box);
		SetCtlValue((ControlHandle)itemHand,gSearchBackwards);
		GetDItem(findDialog,10,&itemType,&itemHand,&box);
		SetCtlValue((ControlHandle)itemHand,gWrapSearch);
		
		ModalDialog(DialogStandardFilter,&itemHit);

		switch(itemHit) {
			case 1:
				pau=true;
				GetDItem(findDialog,4,&itemType,&itemHand,&box);
				GetIText(itemHand,gFindBuffer);
				result=true;;
				break;
			case 2:
				pau=true;
				break;
			case 5:			/* Survey or Literal Radio Button */
				gSearchMethod=0;
				break;
			case 6:			/* Station or Entire Word */
				gSearchMethod=1;
				break;
			case 7:			/* Selection Expression */
				gSearchMethod=2;
				break;
			case 8:			/* Select All or Case Sensitive */
				gCaseSensitive = !gCaseSensitive;
				break;
			case 9:			/* Center Selection or Search Backwards */
				gSearchBackwards = !gSearchBackwards;
				break;
			case 10:		/* Scale to fit or Wrap-Around Search */
				gWrapSearch = !gWrapSearch;
				break;
			default:
				break;
		}
	}
	CloseDialog(findDialog);
	return result;
}

short doReplaceDialog(void)
{
	DialogPtr replaceDialog;
	short itemHit;
	short itemType;
	Handle itemHand;
	int pau=false;
	Rect box;
	
	replaceDialog=GetNewDialog(133,(Ptr)&DlogStor,(WindowPtr)(-1));
	if(!replaceDialog) 
		FatalError();

	GetDItem(replaceDialog,15,&itemType,&itemHand,&box);
	SetDItem(replaceDialog,15,itemType,(Handle)doLine,&box);
	GetDItem(replaceDialog,16,&itemType,&itemHand,&box);
	SetDItem(replaceDialog,16,itemType,(Handle)doLine,&box);
	GetDItem(replaceDialog,17,&itemType,&itemHand,&box);
	SetDItem(replaceDialog,17,itemType,(Handle)doButton,&box);

	GetDItem(replaceDialog,4,&itemType,&itemHand,&box);
	SetIText(itemHand,gFindBuffer);
	SelIText(replaceDialog,4,0,32767);

	GetDItem(replaceDialog,6,&itemType,&itemHand,&box);
	SetIText(itemHand,gReplaceBuffer);

	while(!pau) {
		/* Initialize the three radio buttons */
		GetDItem(replaceDialog,9,&itemType,&itemHand,&box);
		SetCtlValue((ControlHandle)itemHand,gSearchMethod==0);
		GetDItem(replaceDialog,10,&itemType,&itemHand,&box);
		SetCtlValue((ControlHandle)itemHand,gSearchMethod==1);
		GetDItem(replaceDialog,11,&itemType,&itemHand,&box);
		SetCtlValue((ControlHandle)itemHand,gSearchMethod==2);
		/* Do the check boxes */
		GetDItem(replaceDialog,12,&itemType,&itemHand,&box);
		SetCtlValue((ControlHandle)itemHand,gCaseSensitive);
		GetDItem(replaceDialog,13,&itemType,&itemHand,&box);
		SetCtlValue((ControlHandle)itemHand,gSearchBackwards);
		GetDItem(replaceDialog,14,&itemType,&itemHand,&box);
		SetCtlValue((ControlHandle)itemHand,gWrapSearch);

		ModalDialog(DialogStandardFilter,&itemHit);

		switch(itemHit) {
			case 1: case 7: case 8:	/* Replace, Replace All, Find */
				pau=true;
				GetDItem(replaceDialog,4,&itemType,&itemHand,&box);
				GetIText(itemHand,gFindBuffer);
				GetDItem(replaceDialog,6,&itemType,&itemHand,&box);
				GetIText(itemHand,gReplaceBuffer);
				break;
			case 2:
				pau=true;
				break;
			case 9:			/* Literal Radio Button */
				gSearchMethod=0;
				break;
			case 10:		/* Entire Word */
				gSearchMethod=1;
				break;
			case 11:		/*  Selection Expression */
				gSearchMethod=2;
				break;
			case 12:		/* Case Sensitive */
				gCaseSensitive = !gCaseSensitive;
				break;
			case 13:		/* Search Backwards */
				gSearchBackwards = !gSearchBackwards;
				break;
			case 14:		/* Wrap-Around Search */
				gWrapSearch = !gWrapSearch;
				break;
			default:
				SysBeep(7);
				break;
		}
	}
	CloseDialog(replaceDialog);
	return itemHit;
}

char *GetSelection(WindowPtr wind,char *buf,int len)
{
	/* Returns a c string. */
	TE32KHandle tH;
	int selLen;
	
	*buf=0;
	if((tH=(TE32KHandle)((DocumentPeek)wind)->docData) && (**tH).selStart<(**tH).selEnd) {
		selLen = (**tH).selEnd-(**tH).selStart;
		HLock((**tH).hText);
		selLen=(selLen<len)?selLen:len;
		strncpy(buf,*(**tH).hText+(**tH).selStart,selLen);
		HUnlock((**tH).hText);
		*(buf+selLen)=0;
		return buf;
	}
	return 0;
}

void textSelectMark(WindowPtr wind,short item)
{
	TE32KHandle tH;
	short *mark;
	short i;
	
	if(tH=(TE32KHandle)((DocumentPeek)wind)->docData) {
		HLock(((DocumentPeek)wind)->marks);
		mark = (short *)*((DocumentPeek)wind)->marks;
		for(mark++,i=1;i<item;i++)
			mark+= 4+(((MarkRecPtr)mark)->label+1)/2;
		TE32KSetSelect(((MarkRecPtr)mark)->selStart,((MarkRecPtr)mark)->selEnd,tH);
		HUnlock(((DocumentPeek)wind)->marks);
		positionView(tH,1);			
		AdjustScrollBars(wind);
		SetScrollBarValues(wind);
		((DocumentPeek)wind)->fHaveSelection = 1;		
	}
}

void textNewMark(WindowPtr wind)
{
	TE32KHandle tH;
	long selStart,selEnd;
	Str255 selection;
	
	if(tH=(TE32KHandle)((DocumentPeek)wind)->docData) {
		selStart=(**tH).selStart;
		selEnd=(**tH).selEnd;
		GetSelection(wind,selection,63);
		InsertMark((DocumentPeek)wind,selStart,selEnd,selection);
	}
}

void textUnMark(DocumentPeek doc)
{	
	TE32KHandle tH;
	long selStart,selEnd;
	
	if(doc->marks && (tH=(TE32KHandle)doc->docData)) {
		selStart=(**tH).selStart;
		selEnd=(**tH).selEnd;
		DoUnmark(doc,selStart,selEnd);
	}
}
void UpdateSizeList(WindowPtr formatDialog,short fontNum,ListHandle *sizeList) 
{
	ListHandle theList=0;
	long textSize;
	short i,nRows,lastSize;
	short itemType;
	Handle itemHand;
	Point cellSize,cell;
	Rect box,dataBounds;
	Str255 data;
	
	if(*sizeList) 
		LDispose(*sizeList);
		
	GetDItem(formatDialog,9,&itemType,&itemHand,&box);
	GetIText(itemHand,data);
	StringToNum(data,&textSize);
	
	GetDItem(formatDialog,11,&itemType,&itemHand,&box);
	InsetRect(&box,1,1);
	EraseRect(&box);
	InvalRect(&box);

	cellSize.v=15;
	cellSize.h=box.right-box.left;

	box.bottom=box.top+cellSize.v*((box.bottom-box.top)/cellSize.v);
	box.right-=15;

	SetRect(&dataBounds,0,0,1,0);
	*sizeList=LNew(&box,&dataBounds,cellSize,0,formatDialog,FALSE,FALSE,FALSE,TRUE);
	if(!*sizeList) {
		SysBeep(7);
		return;
	}
	
	cell.h=cell.v=0;
	nRows=0;
	i=(textSize<9)? textSize:9;
	for(;i<25;i++) {
		if(i==textSize || RealFont((short)fontNum,i)) {
			NumToString(i,data);
			LAddRow(1,nRows++,*sizeList);
			LSetCell(data+1,*data,cell,*sizeList);
			if(textSize==i) {
				LSetSelect(true,cell,*sizeList);
				LAutoScroll(*sizeList);
			}
			cell.v++;
		}
		if(i<8)
			i=8;
	}
	lastSize=i=(textSize>24 && textSize<30)? textSize:30;
	for(;i<97;i+=6) {
		if(i==textSize || RealFont((short)fontNum,i)) {
			NumToString(i,data);
			LAddRow(1,nRows++,*sizeList);
			LSetCell(data+1,*data,cell,*sizeList);
			if(textSize==i) {
				LSetSelect(true,cell,*sizeList);
				LAutoScroll(*sizeList);
			}
			cell.v++;
		} else if(textSize<i && textSize>lastSize) {
			NumToString(textSize,data);
			LAddRow(1,nRows++,*sizeList);
			LSetCell(data+1,*data,cell,*sizeList);
			LSetSelect(true,cell,*sizeList);
			LAutoScroll(*sizeList);
			cell.v++;
		}
	}
}

ListHandle FillFontList(WindowPtr formatDialog,short fontNum,Point *fontCell)
{
	short i,nFonts;
	ListHandle theList=0;
	MenuHandle fMenu;
	char *fonts;
	Rect box,dataBounds;
	Point cellSize,cell;
	short itemType;
	Handle itemHand;
	Str255 fontName;
	
	if(fMenu=NewMenu(2202,"")) {
	
		/* Pending more study, the MenuManager's method for building a
			list of fonts, seems most efficient. Questions: are the font
			resources left in memory? Are they in alphabetical order? */
			
		AddResMenu(fMenu,'FONT');
		nFonts = CountMItems(fMenu);
		
		GetDItem(formatDialog,10,&itemType,&itemHand,&box);
		InsetRect(&box,1,1);
	
		cellSize.v=15;
		cellSize.h=box.right-box.left;
	
		box.bottom=box.top+cellSize.v*((box.bottom-box.top)/cellSize.v);
		box.right-=15;
	
		SetRect(&dataBounds,0,0,1,nFonts);
		theList=LNew(&box,&dataBounds,cellSize,0,formatDialog,FALSE,FALSE,FALSE,TRUE);

		if(!theList) {
			SysBeep(7);
			DisposeMenu(fMenu);
			return 0;
		}
		
		GetFontName(fontNum,fontName);
		cell.h=0;
		cell.v=0;
		HLock((Handle)fMenu);
		fonts=(**fMenu).menuData+1;
		for(i=1;i<=nFonts;i++) {
			LSetCell(fonts+1,*fonts,cell,theList);
			if(*fontName=*fonts && !strncmp(fontName+1,fonts+1,*fonts)) {
				*fontCell=cell;
				LSetSelect(true,cell,theList);
				LAutoScroll(theList);
			}
			cell.v++;
			fonts+=*fonts+5;
		}
		HUnlock((Handle)fMenu);
		DisposeMenu(fMenu);
		return theList;
	}
	return 0;
}

pascal Boolean formatFilter(DialogPtr dialog,EventRecord *theEvent,short *itemHit)
{
	ListHandle *lists;
	Point mouseLoc,cell;
	short modifiers;
	Rect box;
	short itemType;
	char key;
	Handle itemHand;
	Boolean selected;
	
	SetPort(dialog);

	cell.h=cell.v=0;
	lists=(ListHandle *)GetWRefCon(dialog);
	selected=LGetSelect(true,&cell,lists[0]);
	
	if(theEvent->what==mouseDown) {
		mouseLoc=theEvent->where;
		modifiers=theEvent->modifiers;
		GlobalToLocal(&mouseLoc);
		GetDItem(dialog,10,&itemType,&itemHand,&box);
		if(PtInRect(mouseLoc,&box)) {
			if(LClick(mouseLoc,modifiers,lists[0])) {	/* Double Click */
				*itemHit=1;
				return true;
			}
		}
		GetDItem(dialog,11,&itemType,&itemHand,&box);
		if(PtInRect(mouseLoc,&box)) {
			if(LClick(mouseLoc,modifiers,lists[1])) {	/* Double Click */
				*itemHit=1;
				return true;
			}
		}
	} else if(theEvent->what==keyDown) {
		if(DialogStandardKeyDown(dialog,theEvent,itemHit))
			return true;
		key=theEvent->message&charCodeMask;
		if(isdigit(key) || key==TAB || key==DELETE) {
			return false;
		} else
			return true;
	}
	GetDItem(dialog,1,&itemType,&itemHand,&box);
	HiliteControl((ControlHandle)itemHand,(selected)? 0:255);
	return false;
}

static void doFormatDialog(WindowPtr wind)
{
	TE32KHandle tH;
	DialogPtr formatDialog;
	short itemHit,itemType,dataLen,fontNum,i;
	Boolean noWordWrap,calText=false,pau=false,showInvisibles;
	Boolean autoIndent,wrapToLength,wrapText;
	Handle itemHand;
	ListHandle lists[2];
	Point fontCell,tempCell,sizeCell;
	long textSize,tabChars,lineLength;
	Rect box;
	Str255 numStr;
	
	if(tH=(TE32KHandle)((DocumentPeek)wind)->docData) {
		formatDialog=GetNewDialog(144,(Ptr)&DlogStor,(WindowPtr)(-1));
		if(!formatDialog) 
			FatalError();
	
		if(lists[0]=FillFontList(formatDialog,(**tH).txFont,&fontCell)) {
	
			noWordWrap=(**tH).crOnly;
			wrapToLength=(**tH).wrapToLength;
			autoIndent=(**tH).autoIndent;
			lineLength=((**tH).maxLineWidth==32767)? 75:(**tH).maxLineWidth;
			showInvisibles=(**tH).showInvisibles;

			SetWRefCon(formatDialog,(long)lists);
			LAutoScroll(lists[0]);
			UpdateList(lists[0]);

			GetDItem(formatDialog,20,&itemType,&itemHand,&box);
			SetDItem(formatDialog,20,itemType,(Handle)doButton,&box);
			
			GetDItem(formatDialog,18,&itemType,&itemHand,&box);
			SetDItem(formatDialog,18,itemType,(Handle)doFrame,&box);
			
			GetDItem(formatDialog,4,&itemType,&itemHand,&box);
			NumToString((**tH).tabChars,numStr);
			SetIText(itemHand,numStr);
			
			GetDItem(formatDialog,9,&itemType,&itemHand,&box);
			NumToString((**tH).txSize,numStr);
			SetIText(itemHand,numStr);
			
			GetDItem(formatDialog,15,&itemType,&itemHand,&box);
			NumToString(lineLength,numStr);
			SetIText(itemHand,numStr);
			
			lists[1]=0;
#if 0
			UpdateSizeList(formatDialog,(**tH).txFont,&lists[1]);
			LAutoScroll(lists[1]);
			UpdateList(lists[1]);
#endif
			sizeCell.v=-1;
						
			while(!pau) {
			
				/* Do the check boxes */
				
				GetDItem(formatDialog,5,&itemType,&itemHand,&box);
				SetCtlValue((ControlHandle)itemHand,showInvisibles);
				
				GetDItem(formatDialog,6,&itemType,&itemHand,&box);
				SetCtlValue((ControlHandle)itemHand,!noWordWrap);
				
				GetDItem(formatDialog,16,&itemType,&itemHand,&box);
				SetCtlValue((ControlHandle)itemHand,autoIndent);
				
				/* Do the radio buttons */
				
				GetDItem(formatDialog,12,&itemType,&itemHand,&box);
				SetCtlValue((ControlHandle)itemHand,!wrapToLength);
				HiliteControl((ControlHandle)itemHand,(noWordWrap)?255:0);
				
				GetDItem(formatDialog,13,&itemType,&itemHand,&box);
				SetCtlValue((ControlHandle)itemHand,wrapToLength);
				HiliteControl((ControlHandle)itemHand,(noWordWrap)?255:0);

				tempCell=LLastClick(lists[0]);
				if(tempCell.v!=fontCell.v) {
					fontCell=tempCell;
					dataLen=255;
					LGetCell(numStr+1,&dataLen,fontCell,lists[0]);
					*numStr=dataLen;
					GetFNum(numStr,&fontNum);
					UpdateSizeList(formatDialog,fontNum,&lists[1]);
					LAutoScroll(lists[1]);
					UpdateList(lists[1]);
				}
				ModalDialog(formatFilter,&itemHit);
		
				switch(itemHit) {
					case 19:
					case 1:
						GetDItem(formatDialog,4,&itemType,&itemHand,&box);
						GetIText(itemHand,numStr);
						StringToNum(numStr,&tabChars);
						GetDItem(formatDialog,15,&itemType,&itemHand,&box);
						GetIText(itemHand,numStr);
						StringToNum(numStr,&lineLength);
						
						if(tabChars>0 && lineLength>0) {
							pau=true;
							(**tH).tabChars=tabChars;
							(**tH).tabWidth=tabChars * (**tH).theCharWidths[' '];
						} else {
							SelIText(formatDialog,4,0,32767);
							SysBeep(7);
						}
						break;
					case 2:
						LDispose(lists[0]);
						LDispose(lists[1]);
						CloseDialog(formatDialog);
						return;
					case 5:			/* Show Invisibles */
						showInvisibles=!showInvisibles;
						break;
					case 6:			/* Word Wrap */
						noWordWrap=!noWordWrap;
						break;
					case 11:
						tempCell.h=tempCell.v=0;
						if(LGetSelect(true,&tempCell,lists[1])) {
							sizeCell=tempCell;
							dataLen=255;
							LGetCell(numStr+1,&dataLen,sizeCell,lists[1]);
							*numStr=dataLen;
							GetDItem(formatDialog,9,&itemType,&itemHand,&box);
							SetIText(itemHand,numStr);
							SelIText(formatDialog,9,0,32767);
						}
						break;
					case 12:
						wrapToLength=0;
						break;
					case 13:
						wrapToLength=1;
						break;
					case 16:
						autoIndent=!autoIndent;
						break;
					default:
						break;
				}
			}
			
			tempCell.h=tempCell.v=0;
			fontNum=(**tH).txFont;
			if(LGetSelect(true,&tempCell,lists[0])) {
				dataLen=255;
				LGetCell(numStr+1,&dataLen,tempCell,lists[0]);
				*numStr=dataLen;
				GetFNum(numStr,&fontNum);
			}
			
			GetDItem(formatDialog,9,&itemType,&itemHand,&box);
			GetIText(itemHand,numStr);
			StringToNum(numStr,&textSize);			

			LDispose(lists[0]);
			LDispose(lists[1]);
			CloseDialog(formatDialog);
			
			if((**tH).txFont!=fontNum || (**tH).txSize!=textSize) {
				calText=true;
				TE32KSetFontStuff(fontNum,(**tH).txFace,(**tH).txMode,(short)textSize,tH);
			}
			
			(**tH).autoIndent=autoIndent;
			wrapText = noWordWrap!=(**tH).crOnly 
							|| wrapToLength!=(**tH).wrapToLength
							|| (wrapToLength && (**tH).maxLineWidth!=lineLength);
							
			(**tH).maxLineWidth=lineLength;
			
			if(calText || (**tH).showInvisibles!=showInvisibles || wrapText) {
				SetPort(formatDialog);
				TextFont((**tH).txFont);
				TextFace((**tH).txFace);
				TextSize((**tH).txSize);
				TextMode((**tH).txMode);
				(**tH).showInvisibles=showInvisibles;
				if(showInvisibles) {
					for(i=0;i<0x20;i++)
						(**tH).theCharWidths[i] = CharWidth((unsigned char) '�');
					(**tH).theCharWidths[' '] = CharWidth((unsigned char) '�');
				} else {
					for(i=0;i<0x21;i++)
						(**tH).theCharWidths[i] = CharWidth((unsigned char) i);
				}

				if(!noWordWrap || noWordWrap!=(**tH).crOnly) 
					SetCtlValue(((DocumentPeek)wind)->hScroll,0);
					
				(**tH).crOnly=noWordWrap;
				(**tH).wrapToLength=wrapToLength;
				SetTERect(wind);
				TE32KCalText(tH);
				TE32KSetSelect((**tH).selStart,(**tH).selEnd,tH);
				AdjustScrollBars(wind);
				SetScrollBarValues(wind);
				((DocumentPeek)wind)->fNeedtoSave=true;

				SetPort(wind);
				GetContentRect(wind,&box);
				InvalRect(&box);
				EraseRect(&box);
			}
			if(itemHit==19) 
				DoSavePrefs(tH);
		}
	}
}

Boolean textDoDocMenuCommand(WindowPtr wind, short menuID, short menuItem, short modifiers)
{
	DocumentPeek entryDoc,textDoc;
	short dialogItem;
	
	if(doDocMenuCommand(wind,menuID,menuItem,0))
		return true;
	
	switch(menuID) {
		case EDIT_ID:
			switch(menuItem) {
				case iFormat: 
					doFormatDialog(wind);
					break;
				case iAlign:
					EnTabAndShift(wind,0);
					break;
				case iShiftRight:
					EnTabAndShift(wind,1);
					break;
				case iShiftLeft:
					EnTabAndShift(wind,-1);
					break;
			}
			break;
		case FIND_ID:
			switch(menuItem) {
				case iFind:
					if(doFindDialog(138))
						findSearchString(wind,modifiers&shiftKey);
					break;
				case iFindSame:
					findSearchString(wind,modifiers&shiftKey);
					break;
				case iFindSelection:
					GetSelection(wind,gFindBuffer,255);
					c2pstr(gFindBuffer);
					if(gSearchMethod==2)
						gSearchMethod=0;
					findSearchString(wind,modifiers&shiftKey);
					break;
				case iDisplaySelection:
					positionView((TE32KHandle)((DocumentPeek)wind)->docData,1);
					AdjustScrollBars(wind);
					SetScrollBarValues(wind);
					break;
				case iReplace:
					switch (dialogItem=doReplaceDialog()) {
						case 1:			/* Replace */
							if(findSearchString(wind,modifiers&shiftKey)) {
								TE32KDelete((TE32KHandle)((DocumentPeek)wind)->docData);
								TE32KInsert(gReplaceBuffer+1,(long)*gReplaceBuffer+1,(TE32KHandle)((DocumentPeek)wind)->docData);
								((DocumentPeek)wind)->fNeedtoSave=true;
							}
							break;
						case 7:			/* Replace All */
							while(findSearchString(wind,modifiers&shiftKey)) {
								TE32KDelete((TE32KHandle)((DocumentPeek)wind)->docData);
								TE32KInsert(gReplaceBuffer+1,(long)*gReplaceBuffer+1,(TE32KHandle)((DocumentPeek)wind)->docData);
								((DocumentPeek)wind)->fNeedtoSave=true;
							}
							break;
						case 8:			/* Find */
							findSearchString(wind,modifiers&shiftKey);
							break;
						default:
							break;
					}
					return true;
				case iReplaceSame:
					if(findSearchString(wind,modifiers&shiftKey)) {
						TE32KDelete((TE32KHandle)((DocumentPeek)wind)->docData);
						TE32KInsert(gReplaceBuffer+1,(long)*gReplaceBuffer,(TE32KHandle)((DocumentPeek)wind)->docData);
						((DocumentPeek)wind)->fNeedtoSave=true;
					}
					return true;
				default:
					break;
			}
			break;
		case MARK_ID:
			if(menuItem==iMark) {	
				textNewMark(wind);
			} else if(menuItem==iUnmark) {
				textUnMark((DocumentPeek)wind);
			} else if(menuItem==iAlphabetical) 
				return false;
			else 
				textSelectMark(wind,menuItem-4);
			return true;
		case DATA_ID:
			textDoc = (DocumentPeek)wind;
			switch(menuItem) {
				case iTitlePage:
					if(textDoc->associatedWindow) {
						SelectWindow(textDoc->associatedWindow);
						ToggleDialog(textDoc->associatedWindow,140);
					} else if(entryDoc=MakeModelessDialog('titl')) {
						textDoc->associatedWindow=(WindowPtr)entryDoc;
						entryDoc->associatedWindow=(WindowPtr)textDoc;
					}
					return true;
				case iDataPage:
					if(textDoc->associatedWindow) {
						SelectWindow(textDoc->associatedWindow);
						ToggleDialog(textDoc->associatedWindow,139);
					} else if(entryDoc=MakeModelessDialog('entr')) {
						textDoc->associatedWindow=(WindowPtr)entryDoc;
						entryDoc->associatedWindow=(WindowPtr)textDoc;
					}
					return true;
				case iConfigure:
					doConfigureDialog(wind);
					break;
				case iDeleteTemplate:
					break;
				default:
					break;
			}
			break;
		default:
			break;
	}
	return false;
}

short textGetVertPageScrollAmount(WindowPtr wind)
{
	Rect r;
	short lineHeight;
	
	lineHeight = (**(TE32KHandle)((DocumentPeek)wind)->docData).lineHeight;
	GetContentRect(wind,&r);
	return ((r.bottom - r.top)/lineHeight - 1);
}
	
void textAdjustScrollBars(WindowPtr wind)
{
	DocumentPeek doc;
	Rect r;
	short dh,dv,lineHeight;
	short currentValue;
	short newMax;
	RgnHandle oldClip;
	
	doc = (DocumentPeek)wind;
	GetContentRect(wind,&r);
	dh=dv=0;
	if(doc->vScroll) {
		currentValue=GetCtlValue(doc->vScroll);
		lineHeight = (**(TE32KHandle)doc->docData).lineHeight;
		newMax = GetVertSize(wind) - 2;
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
	FocusOnContent(wind);
	if(dh | dv) {
		InvalRect(&r);
		/* Shut down clip region to empty rectangle before calling
			ScrollContents. */
		oldClip = NewRgn();
		if(MemError()) {
			doMessage(1);
			return;
		}
		GetClip(oldClip);
		SetRect(&r,0,0,0,0);
		ClipRect(&r);
		ScrollContents(wind,dh,dv);
		SetClip(oldClip);
		DisposeRgn(oldClip);
	}
}

void textDoUndo(WindowPtr wind)
{
	long selStart,selEnd,delta=0;
	TE32KHandle tH;
	
	if(((DocumentPeek)wind)->docData) {
		tH = (TE32KHandle)((DocumentPeek)wind)->docData;
		selStart = (**tH).undoStart;
		selEnd = (**tH).undoEnd;
		if(selStart<selEnd) 
			delta=selStart-selEnd;
		if((**tH).undoBuf)
			delta+=GetHandleSize((**tH).undoBuf);
		TE32KUndo(tH);
		UpdateMarks((DocumentPeek)wind,selStart,selEnd,delta,(**tH).teLength);
		((DocumentPeek)wind)->fHaveSelection = SetSelectionFlag(tH);
		AdjustScrollBars(wind);
		SetScrollBarValues(wind);
	}
}

void textDoCut(WindowPtr wind)
{
	long selStart,selEnd;
	TE32KHandle tH;

	tH = (TE32KHandle)((DocumentPeek)wind)->docData;
	selStart = (**tH).selStart;
	selEnd = (**tH).selEnd;
	TE32KCut(tH);
	((DocumentPeek)wind)->fNeedtoSave=true;
	UpdateMarks((DocumentPeek)wind,selStart,selEnd,selStart-selEnd,(**tH).teLength);
	(**tH).undoDelta = selStart-selEnd;
	AdjustScrollBars(wind);
	SetScrollBarValues(wind);
	if(gClipboardDoc) {
		tH=(TE32KHandle)gClipboardDoc->docData;
		TE32KSetSelect(0L,(**tH).teLength,tH);
		TE32KDelete(tH);
		TE32KPaste((TE32KHandle)gClipboardDoc->docData);
		AdjustScrollBars((WindowPtr)gClipboardDoc);
		SetScrollBarValues((WindowPtr)gClipboardDoc);
	}
}

void textDoPaste(WindowPtr wind)
{
	long selStart,selEnd,delta;
	TE32KHandle tH;
	
	tH = (TE32KHandle)((DocumentPeek)wind)->docData;
	selStart = (**tH).selStart;
	selEnd = (**tH).selEnd;
	TE32KPaste(tH);
	delta=selStart-selEnd;	/* -length of cut string */
	/* plus length of replacement string */
	delta+=TE32KScrpLength;
	UpdateMarks((DocumentPeek)wind,selStart,selEnd,delta,(**tH).teLength);
	(**tH).undoDelta=delta;
	((DocumentPeek)wind)->fNeedtoSave=true;
	((DocumentPeek)wind)->fHaveSelection = SetSelectionFlag(tH);
	TE32KSelView(tH);
	AdjustScrollBars(wind);
	SetScrollBarValues(wind);
}

void textDoClear(WindowPtr wind)
{
	long selStart,selEnd;
	TE32KHandle tH;
	
	tH = (TE32KHandle)((DocumentPeek)wind)->docData;
	selStart = (**tH).selStart;
	selEnd = (**tH).selEnd;
	TE32KDelete(tH);
	
	((DocumentPeek)wind)->fNeedtoSave=true;
	UpdateMarks((DocumentPeek)wind,selStart,selEnd,selStart-selEnd,(**tH).teLength);
	TE32KSelView(tH);
	AdjustScrollBars(wind);
	SetScrollBarValues(wind);
}

void textDoCopy(WindowPtr wind)
{
	TE32KHandle tH;
	
	TE32KCopy((TE32KHandle)((DocumentPeek)wind)->docData);
	if(gClipboardDoc) {
		tH=(TE32KHandle)gClipboardDoc->docData;
		TE32KSetSelect(0L,(**tH).teLength,tH);
		TE32KDelete(tH);
		TE32KPaste((TE32KHandle)gClipboardDoc->docData);
		if((**tH).caretState)
			xorCaret(tH);
		AdjustScrollBars((WindowPtr)gClipboardDoc);
		SetScrollBarValues((WindowPtr)gClipboardDoc);
	}
}

void textSelectAll(WindowPtr wind)
{
	TE32KHandle tH;
	if(tH=(TE32KHandle)((DocumentPeek)wind)->docData) {
		TE32KSetSelect(0L,(**tH).teLength,tH);
		((DocumentPeek)wind)->fHaveSelection = SetSelectionFlag(tH);
	}
}

void AddTextMemberFunctions(DocumentPeek doc)
{
	doc->makeWindow=textMakeWindow;
	doc->initDoc=textInitDoc;
	doc->destructor=textDestructor;
	doc->openDocFile=textOpenDocFile;
	doc->readDocFile=textReadDocFile;
	doc->writeDocFile=textWriteDocFile;
	doc->writeDocResourceFork=textWriteDocResourceFork;
	doc->fDocType='TEXT';
	doc->doTheUpdate=textDoTheUpdate;
	doc->draw=textDraw;
	doc->doPageSetup=textDoPageSetup;
	doc->doPrint=textDoPrint;
	doc->activate=textActivate;
	doc->deactivate=textDeactivate;
	doc->doContent=textDoContent;
	doc->doKeyDown=textDoKeyDown;
	doc->doIdle=textDoIdle;
	doc->doGrow=textDoGrow;
	doc->doZoom=textDoZoom;
	doc->doUndo=textDoUndo;
	doc->doCut=textDoCut;
	doc->doPaste=textDoPaste;
	doc->doClear=textDoClear;
	doc->doCopy=textDoCopy;
	doc->doSelectAll=textSelectAll;
	doc->adjustCursor=textAdjustCursor;
	doc->adjustScrollBars=textAdjustScrollBars;
	doc->setScrollBarValues=textSetScrollBarValues;
	doc->getContentRect=textGetContentRect;
	doc->getHorizSize=textGetHorizSize;
	doc->getVertSize=textGetVertSize;
	doc->getVertPageScrollAmount=textGetVertPageScrollAmount;
/*	doc->getHorizPageScrollAmount=textGetHorizPageScrollAmount;	*/
	doc->scrollContents=textScrollContents;
	doc->getVertLineScrollAmount=textGetVertLineScrollAmount;
	doc->getHorizLineScrollAmount=textGetHorizLineScrollAmount;
	doc->adjustDocMenus=textAdjustDocMenus;
	doc->doDocMenuCommand=textDoDocMenuCommand;
}

Handle FillTempHandle(TE32KHandle tH,long selStart,long selEnd)
{
	Handle undoH;
	
	undoH=NewHandle(selEnd-selStart);
	if (MemError() || StripAddress(undoH)==nil) {
		doMessage(6);
		undoH=0;
	} else {
		HLock(undoH);
		HLock((**tH).hText);
		BlockMove(*((**tH).hText) + selStart,*undoH,selEnd-selStart);
		HUnlock(undoH);
		HUnlock((**tH).hText);
	}
	return undoH;
}

static int VirtualEntabALine(StringPtr p,int *nTabs,int *nSpaces,int tabChars)
{
	int nT=0,nS=0,tabCnt;
	StringPtr q;
	
	q=p;
	while(isspace(*p)) {
		if(*p==TAB) {
			tabCnt=tabChars;
			nS=0;
			nT++;
		} else if(*p==' ') {
			if(!--tabCnt) {
				tabCnt=tabChars;
				nT++;
				nS=0;
			} else
				nS++;
		}
		p++;
	}
	*nTabs=nT;
	*nSpaces=nS;
	return p-q;	/* return count to first nonspace */
}

void EnTabAndShift(WindowPtr wind,int direction)
{
	StringPtr q;
	int line,lastLine,tabChars,nMoved,position;
	int selStart,selEnd,len,whiteSpace,delta,newLen;
	int teBufLen;
	int nAlignTabs,nAlignSpaces,nTabs,nSpaces;
	TE32KHandle tH;
	Handle newText;
	Rect box;
	
	if(tH=(TE32KHandle)((DocumentPeek)wind)->docData) {
		selStart=(**tH).selStart;
		selEnd=(**tH).selEnd;
		len=(**tH).teLength;
		
		if((**tH).undoBuf)
			DisposeHandle((**tH).undoBuf);
		(**tH).undoBuf=FillTempHandle(tH,selStart,selEnd);
		
		teBufLen=GetHandleSize((**tH).hText);
		tabChars=(**tH).tabChars;

		HLock((Handle)tH);
		HLock((**tH).hText);
		
		line = indexToParagraph(selStart,tH);
		selStart = (**tH).lineStarts[line];

		lastLine = indexToLine(selEnd,tH);
		if(selEnd > (**tH).lineStarts[lastLine])
			selEnd = (**tH).lineStarts[++lastLine];
		
		nMoved=0;

		if(direction==0) {
			position=(**tH).lineStarts[line++];
			q=*(**tH).hText + position;
			
 			whiteSpace=VirtualEntabALine(q,&nAlignTabs,&nAlignSpaces,tabChars);		
		}
		
		for(;line<lastLine;line++) {
			position=(**tH).lineStarts[line]+nMoved;
			q=*(**tH).hText + position;
			if((**tH).crOnly || *(q-1)==RETURN || *(q-1)==LINEFEED) {
 				whiteSpace=VirtualEntabALine(q,&nTabs,&nSpaces,tabChars);		
			
				if(direction==1)
					nTabs++;
				else if(direction==-1) {
					if(nTabs)
						nTabs--;
					if(!nTabs)			/* number of spaces after last tab set to zero if no tab */
						nSpaces = 0;
				} else {
					nTabs = nAlignTabs;
					nSpaces = nAlignSpaces;
				}
				
				delta = nTabs + nSpaces - whiteSpace; /* if (nTabs + nSpaces - whiteSpace) < 0 then Block move left */
			
				newLen = len + delta;
			
				nMoved += delta; 

				selEnd += delta;
			
				if (teBufLen < newLen) {
					HUnlock((**tH).hText);
					HUnlock((Handle)tH);
					SetHandleSize((**tH).hText,newLen + 256);
					teBufLen=GetHandleSize((**tH).hText);
					if (MemError() || teBufLen < newLen) {
						newText=NewHandle(newLen + 256);
						if(!newText) {
							doMessage(1);
							return;
						}
						teBufLen=GetHandleSize((**tH).hText);
						HLock((**tH).hText);
						HLock(newText);
					
						BlockMove(*((**tH).hText),*newText,teBufLen);
					
						HUnlock((**tH).hText);
						HUnlock(newText);
						DisposeHandle((**tH).hText);
						(**tH).hText=newText;
					}
					HLock((Handle)tH);
					HLock((**tH).hText);
					q=*(**tH).hText + position;			
				}
			
				if(delta) { 
					BlockMove(q+whiteSpace,q+nTabs+nSpaces,len-position-whiteSpace);
					UpdateMarks((DocumentPeek)wind,position+nTabs+nSpaces,position+whiteSpace,delta,(**tH).teLength);
				}
							
				while(nTabs-- > 0)
					*q++ = TAB;
				while(nSpaces-- > 0)
					*q++ = ' ';
			
				len = newLen;
			}
		}

		HUnlock((**tH).hText);
		HUnlock((Handle)tH);
		(**tH).undoStart = selStart;
		(**tH).undoEnd = selEnd;
		(**tH).resetUndo = false;
		(**tH).undoDelta = 0;
		((DocumentPeek)wind)->fNeedtoSave = true;
		(**tH).teLength=len;
		
		SetTERect(wind);
		TE32KCalText(tH);
		TE32KSetSelect(selStart,selEnd,tH);
		AdjustScrollBars(wind);
		SetScrollBarValues(wind);
		SetPort(wind);
		GetContentRect(wind,&box);
		InvalRect(&box);
		EraseRect(&box);
	}
}
	
