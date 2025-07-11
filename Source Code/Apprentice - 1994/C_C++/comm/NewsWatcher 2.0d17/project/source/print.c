/*----------------------------------------------------------------------------

	print.c

	This module handles printing.
	
	Portions copyright � 1990, Apple Computer.
	Portions copyright � 1993, Northwestern University.

----------------------------------------------------------------------------*/

#include <Printing.h>

#include "dlgutil.h"
#include "glob.h"
#include "print.h"
#include "save.h"
#include "util.h"


#define odd(theInt) ( (theInt % 2) == 1 )

static THPrint myHPrint;		/* printing handle */
static TEHandle printTE;		/* textedit handle to print */
static Handle txt;				/* text to print */
static short linesPerPage;		/* # of lines per page */
static long numLines;			/* total # lines */

static TPPrPort		prPort;		/* printing port */
static Boolean		prIsOpen,docIsOpen,pageIsOpen;	/* printing flags */

/* Globals for printing segmented article windows. */

static Boolean segmented;			/* true if segmented */
static short numSections;			/* number of sections */
static short curSection;			/* current section in printTE, or -1 if none */
static long **sectionBreaks;		/* handle to array of section breaks */
static long **firstLines;			/* handle to array of section first line numbers */


static void CleanUp (void)
{
	MyIOCheck(PrError());
	if (pageIsOpen)
		PrClosePage(prPort);
	if (docIsOpen)
		PrCloseDoc(prPort);
	if (prIsOpen)
		PrClose();
}

/*	PrepSegmented prepares for printing segmented article windows. It calculates
	the firstLines array and the total number of lines.
*/

static void PrepSegmented (void)
{
	long linesInSection,offset,length;
	short i;

	HLock(txt);
	for (i = numSections-1; i >= 0; i--) {
		offset = (*sectionBreaks)[i];
		length = (*sectionBreaks)[i+1] - offset;
		TESetText(*txt+offset,length,printTE);
		(*firstLines)[i] = (**printTE).nLines;
	}
	HUnlock(txt);
	
	numLines = 0;
	for (i = 0; i < numSections; i++) {
		linesInSection = (*firstLines)[i];
		(*firstLines)[i] = numLines;
		numLines += linesInSection;
	}
	(*firstLines)[numSections] = numLines;
	curSection = 0;
}

/*	MyPrepProc prepares for printing.
*/

static Boolean MyPrepProc (THPrint theHPrint)
{
	short numPages;

	linesPerPage = ((**theHPrint).prInfo.rPage.bottom - (**theHPrint).prInfo.rPage.top) / (**printTE).lineHeight;

	if (segmented) {
		PrepSegmented();
	} else {
		numLines = (**printTE).nLines;
	}
	numPages = (numLines + linesPerPage - 1) / linesPerPage;
	
	if ((**theHPrint).prJob.iLstPage > numPages)
		(**theHPrint).prJob.iLstPage = numPages;
	if ( (**theHPrint).prJob.iLstPage >= (**theHPrint).prJob.iFstPage ) {
		return(true);
	} else {
		ErrorMessage("There are no pages in the range you specified.");
		return(false);
	}
}

/*	MyPageProc prints one page.
*/

static Boolean MyPageProc (THPrint theHPrint,Rect *drawRect,short pageNum)
{
	short vCoord = drawRect->top + (**printTE).fontAscent;
	long line, lastLine, length;
	long offset = 0;
	long firstLineThisSection = 0;
	long firstLineNextSection;
	short start,next;
	short i;
	char *sectionStart;
	
	HLock(txt);
	TextFont((**printTE).txFont);
	TextFace((**printTE).txFace);
	TextMode((**printTE).txMode);
	TextSize((**printTE).txSize);
		
	line = ((pageNum - 1) * linesPerPage);	/*	LSR: starting line #	*/
	
	if (line + linesPerPage > numLines)
		lastLine = numLines;
	else
		lastLine = line + linesPerPage;
	
	if (segmented) {
		for (i = 0; i < numSections && line >= (*firstLines)[i]; i++);
		i--;
		offset = (*sectionBreaks)[i];
		sectionStart = *txt + offset;
		firstLineThisSection = (*firstLines)[i];
		firstLineNextSection = (*firstLines)[i+1];
		if (i != curSection) {
			length = (*sectionBreaks)[i+1] - offset;
			TESetText(sectionStart,length,printTE);
			curSection = i;
		}
	} else {
		sectionStart = *txt;
	}
	
	start = (**printTE).lineStarts[line-firstLineThisSection];
	
	while (true) {
		if (line-firstLineThisSection < (**printTE).nLines-1)
			next = (**printTE).lineStarts[line+1-firstLineThisSection];
		else
			next = (**printTE).teLength;
		/*	LSR: next is offset to next char to print	*/
		
		MoveTo(drawRect->left, vCoord);
		DrawText(sectionStart, start, next-start);
		line++;
		if (line == lastLine) break;
		
		start = next;
		if (segmented && line >= firstLineNextSection) {
			curSection++;
			offset = (*sectionBreaks)[curSection];
			sectionStart = *txt + offset;
			length = (*sectionBreaks)[curSection+1] - offset;
			TESetText(sectionStart,length,printTE);
			firstLineThisSection = firstLineNextSection;
			firstLineNextSection = (*firstLines)[curSection+1];
			start = 0;
		}

		vCoord += (**printTE).lineHeight;
	}

	HUnlock(txt);
	return(true);
}

/*	ExecutePrint is the main print control routine. */


static OSErr ExecutePrint (THPrint hPrint)
{
	short theFirst,theLast;
	Boolean wasnil,scratch;
	short nCopies;
	short prDevice;
	Boolean draftMode;
	TPrStatus prStatus;
	GrafPtr savePort;
	OSErr curPrError;
	Rect pageRect;
	short i,p;

	prIsOpen = docIsOpen = pageIsOpen = false;
	GetPort(&savePort);
	wasnil = (hPrint == nil);
	PrOpen();
	if (PrError() != noErr) {
		SetPort(savePort);
		CleanUp();
		return(PrError());
	}
	prIsOpen = true;
	scratch = PrValidate(hPrint);
	if (!PrJobDialog(hPrint)) {
		SetPort(savePort);
		CleanUp();
		return(PrError());
	}
	if (!MyPrepProc(hPrint)) {
		SetPort(savePort);
		CleanUp();
		return(PrError());
	}
	theFirst = (**hPrint).prJob.iFstPage;
	theLast = (**hPrint).prJob.iLstPage;
	(**hPrint).prJob.iFstPage = 1;
	(**hPrint).prJob.iLstPage = 9999;
	prDevice = ( (**hPrint).prStl.wDev >> 8 );
	draftMode = !(odd((**hPrint).prJob.bJDocLoop));
	if ((draftMode) && (prDevice == 1))
		nCopies = (**hPrint).prJob.iCopies;
	else
		nCopies = 1;
	
	prPort = PrOpenDoc(hPrint,nil,nil);
	docIsOpen = true;
	curPrError = PrError();
	if (curPrError != noErr) {
		SetPort(savePort);
		return(curPrError);
	}
	SetPort(&prPort->gPort);
	for (i=1; i<=nCopies; i++) {
		for (p=theFirst; p<=theLast; p++) {
			PrOpenPage(prPort,nil);
			pageIsOpen = true;
			HLock((Handle)hPrint);
			pageRect = (**hPrint).prInfo.rPage;
			scratch = MyPageProc(hPrint,&pageRect,p);
			HUnlock((Handle)hPrint);
			PrClosePage(prPort);
			if (!scratch) {
				SetPort(savePort);
				curPrError = iPrAbort;
				CleanUp();
				return(PrError());
			}
		}
	}
	PrCloseDoc(prPort);
	docIsOpen = false;
	curPrError = PrError();
	
	if ( !draftMode && (curPrError != noErr))
		PrPicFile(hPrint,nil,nil,nil,&prStatus);
	curPrError = PrError();
	PrClose();
	prIsOpen = false;
	
	SetPort(savePort);
	return(noErr);
}

static void InitPrint (void)
{
	static Boolean InitDone = false;

	if (InitDone) return;
	InitDone = true;
	PrOpen();
	MyIOCheck(PrError());
	myHPrint = (THPrint) MyNewHandle(sizeof(TPrint));
	if (MyMemErr() != noErr)
		return;
	PrintDefault(myHPrint);
	MyIOCheck(PrError());
	PrClose();
}

void DoPageSetup (void)
{
	InitPrint();
	InitCursor();
	PrOpen();
	if (PrError() == noErr) {
		PrValidate(myHPrint);
		if (PrError() == noErr)
			PrStlDialog(myHPrint);
		else MyIOCheck(PrError());
	}
	else
		MyIOCheck(PrError());
	PrClose();
	MyIOCheck(PrError());
}

/*	DoPrint handles printing.
*/

void DoPrint (WindowPtr wind)
{
	Handle oldText;
	TWindow **info;
	EWindowKind kind;
	Boolean isArticle, mustDispose=false;

	InitPrint();

	info = (TWindow**)GetWRefCon(wind);
	kind = (**info).kind;
	isArticle = kind == kArticle || kind == kMiscArticle;
	segmented = isArticle && (**info).numSections != 0;
	
	InitCursor();
	
	printTE = TENew(&(**myHPrint).prInfo.rPage, &(**myHPrint).prInfo.rPage);
	
	if (isArticle) {
		txt = (**info).fullText;
	} else {
		GetFullMessageText(wind, &txt, &mustDispose);
	}
	
	if (segmented) {
		numSections = (**info).numSections;
		curSection = -1;
		sectionBreaks = (**info).sectionBreaks;
		firstLines = (long**)MyNewHandle(sizeof(long)*(numSections+1));
	} else {
		oldText = (**printTE).hText;
		(**printTE).hText = txt;
		TECalText(printTE);
	}
	
	MyIOCheck( ExecutePrint(myHPrint) );
	
	if (segmented) {
		MyDisposHandle((Handle)firstLines);
	} else {
		(**printTE).hText = oldText;
	}
	TEDispose(printTE);
	if (mustDispose) MyDisposHandle(txt);
	printTE = NULL;
}
