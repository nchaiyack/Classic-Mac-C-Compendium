/*------------------------------------------------------------------------
				Copyright (c) 1993-1994 by Melburn R. Park

This code is free. It may be freely distributed and incorporated into other
programs. This code may not be subsequently copyrighted, sold, or assigned 
in any way that restricts its use by any others, including its authors.

-------------------------------------------------------------------------*/


/*----------------------------- Includes ---------------------------------*/
	
#include "PlainText.h"
#include "PTGlobs.h"

/*---------------------------- Declarations ------------------------------*/

void StraightToCurly(WindowPtr);
void CurlyToStraight(WindowPtr);
void AddLinefeeds(WindowPtr);
void StripLinefeeds(WindowPtr);
void StripControls(WindowPtr);
void LinefeedstoReturns(WindowPtr);
void ReturnstoLinefeeds(WindowPtr);
void WordWrap(WindowPtr,Boolean);
void TabsToSpaces(WindowPtr);
void SpacesToTabs(WindowPtr);
void FormParagraphs(WindowPtr);
void EnTabAndShift(WindowPtr,int);

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

void StraightToCurly(WindowPtr wind)
{
	TE32KHandle tH;
	long selStart,selEnd,i,dirty=0;
	register char *p;
	Rect box;
	Handle undoH;
	
	if(tH=(TE32KHandle)((DocumentPeek)wind)->docData) {
		selStart=(**tH).selStart;
		selEnd=(**tH).selEnd;
		if(selStart>=selEnd) {
			selStart=0;
			selEnd=(**tH).teLength;
		}
		
		undoH=FillTempHandle(tH,selStart,selEnd);
		
		HLock((**tH).hText);
		p=*(**tH).hText+selStart;
		for(i=selStart;i<selEnd;i++,p++) {
			if(*p=='\'') {
				if(!i || isspace(*(p-1)))
					*p='Ô';
				else
					*p='Õ';
				dirty=1;
			} else if(*p=='\"') {
				if(!i || isspace(*(p-1)))
					*p='Ò';
				else
					*p='Ó';
				dirty=1;
			}
		}
		HUnlock((**tH).hText);
		if(dirty) {
			if((**tH).undoBuf)
				DisposeHandle((**tH).undoBuf);
			(**tH).undoBuf=undoH;
			(**tH).undoStart = selStart;
			(**tH).undoEnd = selEnd;
			(**tH).resetUndo = false;
			(**tH).undoDelta = 0;
			((DocumentPeek)wind)->fNeedtoSave=true;
			SetPort(wind);
			GetContentRect(wind,&box);
			InvalRect(&box);
			EraseRect(&box);
		} else if(undoH)
			DisposeHandle(undoH);
	}
}

void CurlyToStraight(WindowPtr wind)
{
	TE32KHandle tH;
	long selStart,selEnd,i,dirty=0;
	register char *p;
	Rect box;
	Handle undoH;
	
	if(tH=(TE32KHandle)((DocumentPeek)wind)->docData) {
		selStart=(**tH).selStart;
		selEnd=(**tH).selEnd;
		if(selStart>=selEnd) {
			selStart=0;
			selEnd=(**tH).teLength;
		}
		
		undoH=FillTempHandle(tH,selStart,selEnd);
		
		HLock((**tH).hText);
		p=*(**tH).hText+selStart;
		for(i=selStart;i<selEnd;i++,p++) {
			if(*p=='Õ' || *p=='Ô') {
				*p='\'';
				dirty=1;
			} else if(*p=='Ó' || *p=='Ò') {
				*p='\"';
				dirty=1;
			}
		}
		HUnlock((**tH).hText);
		if(dirty) {
			if((**tH).undoBuf)
				DisposeHandle((**tH).undoBuf);
			(**tH).undoBuf=undoH;
			(**tH).undoStart = selStart;
			(**tH).undoEnd = selEnd;
			(**tH).resetUndo = false;
			(**tH).undoDelta = 0;
			((DocumentPeek)wind)->fNeedtoSave=true;
			SetPort(wind);
			GetContentRect(wind,&box);
			InvalRect(&box);
			EraseRect(&box);
		} else if(undoH)
			DisposeHandle(undoH);
	}

}

void AddLinefeeds(WindowPtr wind)
{
	TE32KHandle tH;
	long selStart,selEnd,i,len,teBufLen;
	register char *p;
	Rect box;
	Handle undoH,newText;
	
	if(tH=(TE32KHandle)((DocumentPeek)wind)->docData) {
		selStart=(**tH).selStart;
		selEnd=(**tH).selEnd;
		len=(**tH).teLength;
		if(selStart>=selEnd) {
			selStart=0;
			selEnd=(**tH).teLength;
		}
		
		undoH=FillTempHandle(tH,selStart,selEnd);
		teBufLen=GetHandleSize((**tH).hText);
		HLock((**tH).hText);
		p=*(**tH).hText+selStart;
		for(i=selStart;i<selEnd;i++,p++) {
			if(*p=='\n') {
				if (teBufLen < ++len) {
					HUnlock((**tH).hText);
					SetHandleSize((**tH).hText,len + 256);
					teBufLen=GetHandleSize((**tH).hText);
					if (MemError() || teBufLen < len) {
						newText=NewHandle(len + 256);
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
					HLock((**tH).hText);
					p=*(**tH).hText+i;
				}
				BlockMove(p+1,p+2,len-i-1);
				*(p+1)='\r';
				UpdateMarks((DocumentPeek)wind,i,i,1,(**tH).teLength);
				selEnd++;
			}
		}
		HUnlock((**tH).hText);
		if(len != (**tH).teLength) {
			if((**tH).undoBuf)
				DisposeHandle((**tH).undoBuf);
			(**tH).undoBuf=undoH;
			(**tH).undoStart = selStart;
			(**tH).selEnd=(**tH).undoEnd = selEnd;
			if((**tH).selStart>selEnd);
				(**tH).selStart=selEnd;
			(**tH).resetUndo = false;
			(**tH).undoDelta = 0;
			((DocumentPeek)wind)->fNeedtoSave = true;
			(**tH).teLength=len;
			TE32KCalText(tH);
			TE32KSetSelect((**tH).selStart,(**tH).selEnd,tH);
			SetPort(wind);
			GetContentRect(wind,&box);
			InvalRect(&box);
			EraseRect(&box);
		} else if(undoH)
			DisposeHandle(undoH);
	}
}

void StripControls(WindowPtr wind)
{
	TE32KHandle tH;
	long selStart,selEnd,i,len;
	register char *p;
	Rect box;
	Handle undoH;
	
	if(tH=(TE32KHandle)((DocumentPeek)wind)->docData) {
		selStart=(**tH).selStart;
		selEnd=(**tH).selEnd;
		len=(**tH).teLength;
		if(selStart>=selEnd) {
			selStart=0;
			selEnd=(**tH).teLength;
		}
		
		undoH=FillTempHandle(tH,selStart,selEnd);

		HLock((**tH).hText);
		p=*(**tH).hText+selStart;
		for(i=selStart;i<selEnd;) {
			if(*p<0x20 && *p != '\t' && *p != '\n') {
				BlockMove(p+1,p,len-i);
				UpdateMarks((DocumentPeek)wind,i,i,-1,(**tH).teLength);
				len--;
				selEnd--;
			} else {
				p++;
				i++;
			}
		}
		HUnlock((**tH).hText);
		if(len != (**tH).teLength) {
			if((**tH).undoBuf)
				DisposeHandle((**tH).undoBuf);
			(**tH).undoBuf=undoH;
			(**tH).undoStart = selStart;
			(**tH).selEnd=(**tH).undoEnd = selEnd;
			if((**tH).selStart>selEnd);
				(**tH).selStart=selEnd;
			(**tH).resetUndo = false;
			(**tH).undoDelta = 0;
			((DocumentPeek)wind)->fNeedtoSave = true;
			(**tH).teLength=len;
			TE32KCalText(tH);
			TE32KSetSelect((**tH).selStart,(**tH).selEnd,tH);
			SetPort(wind);
			GetContentRect(wind,&box);
			InvalRect(&box);
			EraseRect(&box);
		} else if(undoH)
			DisposeHandle(undoH);
	}
}


void StripLinefeeds(WindowPtr wind)
{
	TE32KHandle tH;
	long selStart,selEnd,i,len;
	register char *p;
	Rect box;
	Handle undoH;
	
	if(tH=(TE32KHandle)((DocumentPeek)wind)->docData) {
		selStart=(**tH).selStart;
		selEnd=(**tH).selEnd;
		len=(**tH).teLength-1;
		if(selStart>=selEnd) {
			selStart=0;
			selEnd=(**tH).teLength;
		}
		
		undoH=FillTempHandle(tH,selStart,selEnd);
		
		HLock((**tH).hText);
		p=*(**tH).hText+selStart;
		for(i=selStart;i<selEnd;i++,p++) {
			if(*p=='\r') {
				BlockMove(p+1,p,len-i);
				UpdateMarks((DocumentPeek)wind,i,i,-1,(**tH).teLength);
				len--;
				selEnd--;
			}
		}
		HUnlock((**tH).hText);
		if(++len != (**tH).teLength) {
			if((**tH).undoBuf)
				DisposeHandle((**tH).undoBuf);
			(**tH).undoBuf=undoH;
			(**tH).undoStart = selStart;
			(**tH).selEnd=(**tH).undoEnd = selEnd;
			if((**tH).selStart>selEnd);
				(**tH).selStart=selEnd;
			(**tH).resetUndo = false;
			(**tH).undoDelta = 0;
			((DocumentPeek)wind)->fNeedtoSave = true;
			(**tH).teLength=len;
			TE32KCalText(tH);
			TE32KSetSelect((**tH).selStart,(**tH).selEnd,tH);
			SetPort(wind);
			GetContentRect(wind,&box);
			InvalRect(&box);
			EraseRect(&box);
		} else if(undoH)
			DisposeHandle(undoH);
	}
}

void LinefeedstoReturns(WindowPtr wind)
{

	TE32KHandle tH;
	long selStart,selEnd,i,dirty=0;
	register char *p;
	Rect box;
	Handle undoH;
	
	if(tH=(TE32KHandle)((DocumentPeek)wind)->docData) {
		selStart=(**tH).selStart;
		selEnd=(**tH).selEnd;
		if(selStart>=selEnd) {
			selStart=0;
			selEnd=(**tH).teLength;
		}
		
		undoH=FillTempHandle(tH,selStart,selEnd);
		
		HLock((**tH).hText);
		p=*(**tH).hText+selStart;
		for(i=selStart;i<selEnd;i++,p++) {
			if(*p=='\r') {
				*p='\n';
				dirty=1;
			} 
		}
		HUnlock((**tH).hText);
		if(dirty) {
			if((**tH).undoBuf)
				DisposeHandle((**tH).undoBuf);
			(**tH).undoBuf=undoH;
			(**tH).undoStart = selStart;
			(**tH).undoEnd = selEnd;
			(**tH).resetUndo = false;
			(**tH).undoDelta = 0;
			((DocumentPeek)wind)->fNeedtoSave=true;
			TE32KCalText(tH);
			TE32KSetSelect((**tH).selStart,(**tH).selEnd,tH);
			AdjustScrollBars(wind);
			SetScrollBarValues(wind);
			SetPort(wind);
			GetContentRect(wind,&box);
			InvalRect(&box);
			EraseRect(&box);
		} else if(undoH)
			DisposeHandle(undoH);
	}
}

void ReturnstoLinefeeds(WindowPtr wind)
{
	TE32KHandle tH;
	long selStart,selEnd,i,dirty=0;
	register char *p;
	Rect box;
	Handle undoH;
	
	if(tH=(TE32KHandle)((DocumentPeek)wind)->docData) {
		selStart=(**tH).selStart;
		selEnd=(**tH).selEnd;
		if(selStart>=selEnd) {
			selStart=0;
			selEnd=(**tH).teLength;
		}
		
		undoH=FillTempHandle(tH,selStart,selEnd);
		
		HLock((**tH).hText);
		p=*(**tH).hText+selStart;
		for(i=selStart;i<selEnd;i++,p++) {
			if(*p=='\n') {
				*p='\r';
				dirty=1;
			} 
		}
		HUnlock((**tH).hText);
		if(dirty) {
			if((**tH).undoBuf)
				DisposeHandle((**tH).undoBuf);
			(**tH).undoBuf=undoH;
			(**tH).undoStart = selStart;
			(**tH).undoEnd = selEnd;
			(**tH).resetUndo = false;
			(**tH).undoDelta = 0;
			((DocumentPeek)wind)->fNeedtoSave=true;
			TE32KCalText(tH);
			TE32KSetSelect((**tH).selStart,(**tH).selEnd,tH);
			AdjustScrollBars(wind);
			SetScrollBarValues(wind);
			SetPort(wind);
			GetContentRect(wind,&box);
			InvalRect(&box);
			EraseRect(&box);
		} else if(undoH)
			DisposeHandle(undoH);
	}
}

void FormParagraphs(WindowPtr wind)
{
	TE32KHandle tH;
	long selStart,selEnd,i,len,dirty=0,karet;
	register char *p;
	Rect box;
	Handle undoH;
	Boolean haveSelection;
	
	if(tH=(TE32KHandle)((DocumentPeek)wind)->docData) {
		selStart=(**tH).selStart;
		selEnd=(**tH).selEnd;
		if(!(haveSelection=(selStart<selEnd))) {
			selStart=0;
			selEnd=(**tH).teLength;
			karet=(**tH).selStart;
		}
		
		undoH=FillTempHandle(tH,selStart,selEnd);
		
		HLock((**tH).hText);
		p=*(**tH).hText+selStart;
		len=(**tH).teLength;
		for(i=selStart;i<selEnd;i++,p++) {
			if(*p=='\n') {
				if(*(p-1)==' ') {
					BlockMove(p,p-1,len-i);
					UpdateMarks((DocumentPeek)wind,i,i,-1,(**tH).teLength);
					if(karet>i--)
						karet--;
					len--;
					selEnd--;
					p--;
				}
				if(*(p+1)==' ' || *(p+1)=='\t' || *(p+1)=='\n' || *(p-1)=='\n' || *(p-1)=='\r')
					;
				else {
					*p=' ';
					dirty=1;
				}
			} 
		}
		HUnlock((**tH).hText);
		if(dirty) {
			if((**tH).undoBuf)
				DisposeHandle((**tH).undoBuf);
			(**tH).undoBuf=undoH;
			(**tH).undoStart = selStart;
			(**tH).undoEnd = selEnd;
			(**tH).resetUndo = false;
			(**tH).undoDelta = 0;
			(**tH).teLength=len;
			SetTERect(wind);
			TE32KCalText(tH);
			if(haveSelection)
				TE32KSetSelect(selStart,selEnd,tH);
			else if(karet!=(**tH).selStart)
				TE32KSetSelect(karet,karet,tH);
			AdjustScrollBars(wind);
			SetScrollBarValues(wind);
			((DocumentPeek)wind)->fNeedtoSave=true;
			SetPort(wind);
			GetContentRect(wind,&box);
			InvalRect(&box);
			EraseRect(&box);
		} else if(undoH)
			DisposeHandle(undoH);
	}
}

void WordWrap(WindowPtr wind, Boolean how)
{
	TE32KHandle tH;
	long selStart,selEnd,i,len,teBufLen,nNewLines,karet,position;
	register char *p;
	Rect box;
	Handle newText;
	Boolean crOnly,wrapToLength;
	Boolean haveSelection;

	if(tH=(TE32KHandle)((DocumentPeek)wind)->docData) {
		selStart=(**tH).selStart;
		selEnd=(**tH).selEnd;
		len=(**tH).teLength;
		if(!(haveSelection=(selStart<selEnd))) {
			selStart=0;
			selEnd=(**tH).teLength;
			karet=(**tH).selStart;
		}
		
		if((**tH).undoBuf)
			DisposeHandle((**tH).undoBuf);
		(**tH).undoBuf=FillTempHandle(tH,selStart,selEnd);
		
		wrapToLength=(**tH).wrapToLength;
		crOnly=(**tH).crOnly;
		
		if(crOnly || (how && !wrapToLength)) {
			(**tH).crOnly=0;
			(**tH).wrapToLength=how;
			SetTERect(wind);
			TE32KCalText(tH);
		}
		
		teBufLen=GetHandleSize((**tH).hText);
		
		HLock((Handle)tH);
		HLock((**tH).hText);
		i=1;
		while(selStart>(**tH).lineStarts[i]) i++;
		position=(**tH).lineStarts[i];
		for(nNewLines=0;selEnd>(position=(**tH).lineStarts[i]+nNewLines);i++) {
			p=*(**tH).hText+position;
			if(*(p-1)!='\n') {
				if(*(p-1)==' ')
					*(p-1)='\n';
				else {
					if (teBufLen < ++len) {
						HUnlock((**tH).hText);
						HUnlock((Handle)tH);
						SetHandleSize((**tH).hText,len + 256);
						teBufLen=GetHandleSize((**tH).hText);
						if (MemError() || teBufLen < len) {
							newText=NewHandle(len + 256);
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
						p=*(**tH).hText+position;
					}
					BlockMove(p,p+1,len-position-1);
					UpdateMarks((DocumentPeek)wind,position,position,1,(**tH).teLength);
					if(karet>position)
						karet++;
					*(p)='\n';
					selEnd++;
					nNewLines++;
				}
			}
		}
		
		HUnlock((**tH).hText);
		HUnlock((Handle)tH);
		
		(**tH).wrapToLength=wrapToLength;
		(**tH).crOnly=crOnly;
		
		(**tH).undoStart = selStart;
		(**tH).undoEnd = selEnd;
		(**tH).resetUndo = false;
		(**tH).undoDelta = 0;
		((DocumentPeek)wind)->fNeedtoSave = true;
		(**tH).teLength=len;
		
		SetTERect(wind);
		TE32KCalText(tH);
		if(haveSelection)
			TE32KSetSelect(selStart,selEnd,tH);
		else if(karet!=(**tH).selStart)
			TE32KSetSelect(karet,karet,tH);
		AdjustScrollBars(wind);
		SetScrollBarValues(wind);
		SetPort(wind);
		GetContentRect(wind,&box);
		InvalRect(&box);
		EraseRect(&box);
	}
}

void SpacesToTabs(WindowPtr wind)
{
	TE32KHandle tH;
	long selStart,selEnd,i,len;
	long tabChars;
	register char *p;
	Rect box;
	Handle undoH;
	Str255 spaces;
	
	if(tH=(TE32KHandle)((DocumentPeek)wind)->docData) {
		tabChars=(**tH).tabChars;
		selStart=(**tH).selStart;
		selEnd=(**tH).selEnd;
		len=(**tH).teLength;
		if(selStart>=selEnd) {
			selStart=0;
			selEnd=(**tH).teLength;
		}
		
		memset(spaces,' ',tabChars);
		*(spaces+tabChars)=0;
		
		undoH=FillTempHandle(tH,selStart,selEnd);
		
		HLock((**tH).hText);
		p=*(**tH).hText+selStart;
		for(i=selStart;i<selEnd;i++,p++) {
			if(*p==' ' && !strncmp(p,spaces,tabChars)) {
				BlockMove(p+tabChars,p+1,len-i-tabChars);
				*p=TAB;
				UpdateMarks((DocumentPeek)wind,i,i,-tabChars,(**tH).teLength);
				len-=(tabChars-1);
				selEnd-=(tabChars-1);
			}
		}
		HUnlock((**tH).hText);
		if(len != (**tH).teLength) {
			if((**tH).undoBuf)
				DisposeHandle((**tH).undoBuf);
			(**tH).undoBuf=undoH;
			(**tH).undoStart = selStart;
			(**tH).selEnd=(**tH).undoEnd = selEnd;
			if((**tH).selStart>selEnd);
				(**tH).selStart=selEnd;
			(**tH).resetUndo = false;
			(**tH).undoDelta = 0;
			((DocumentPeek)wind)->fNeedtoSave = true;
			(**tH).teLength=len;
			TE32KCalText(tH);
			TE32KSetSelect((**tH).selStart,(**tH).selEnd,tH);
			SetPort(wind);
			GetContentRect(wind,&box);
			InvalRect(&box);
			EraseRect(&box);
		} else if(undoH)
			DisposeHandle(undoH);
	}
}

void TabsToSpaces(WindowPtr wind)
{
	TE32KHandle tH;
	long selStart,selEnd,i,len,teBufLen;
	long tabChars,j;
	register char *p;
	Rect box;
	Handle undoH,newText;
	
	if(tH=(TE32KHandle)((DocumentPeek)wind)->docData) {
		selStart=(**tH).selStart;
		selEnd=(**tH).selEnd;
		len=(**tH).teLength;
		if(selStart>=selEnd) {
			selStart=0;
			selEnd=(**tH).teLength;
		}
		
		undoH=FillTempHandle(tH,selStart,selEnd);
		teBufLen=GetHandleSize((**tH).hText);
		tabChars=(**tH).tabChars;
		
		HLock((**tH).hText);
		p=*(**tH).hText+selStart;
		for(i=selStart;i<selEnd;i++,p++) {
			if(*p==TAB) {
				if (teBufLen < (len+=(tabChars-1))) {
					HUnlock((**tH).hText);
					SetHandleSize((**tH).hText,len + 256);
					teBufLen=GetHandleSize((**tH).hText);
					if (MemError() || teBufLen < len) {
						newText=NewHandle(len + 256);
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
					HLock((**tH).hText);
					p=*(**tH).hText+i;
				}
				BlockMove(p+1,p+tabChars,len-i-tabChars);
				for(j=0;j<tabChars;j++) {
					*(p++)=' ';
				}
				UpdateMarks((DocumentPeek)wind,i,i,tabChars,(**tH).teLength);
				selEnd+=(tabChars-1);
				i+=(tabChars-1);
			}
		}
		HUnlock((**tH).hText);
		if(len != (**tH).teLength) {
			if((**tH).undoBuf)
				DisposeHandle((**tH).undoBuf);
			(**tH).undoBuf=undoH;
			(**tH).undoStart = selStart;
			(**tH).selEnd=(**tH).undoEnd = selEnd;
			if((**tH).selStart>selEnd);
				(**tH).selStart=selEnd;
			(**tH).resetUndo = false;
			(**tH).undoDelta = 0;
			((DocumentPeek)wind)->fNeedtoSave = true;
			(**tH).teLength=len;
			TE32KCalText(tH);
			TE32KSetSelect((**tH).selStart,(**tH).selEnd,tH);
			SetPort(wind);
			GetContentRect(wind,&box);
			InvalRect(&box);
			EraseRect(&box);
		} else if(undoH)
			DisposeHandle(undoH);
	}
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
	
