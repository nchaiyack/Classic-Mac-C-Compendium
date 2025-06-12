/*
*********************************************************************
*	
*	print.c
*	Print routines
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

#include <PrintTraps.h>
#include <Packages.h>
#include <stdio.h>
#include <string.h>

#include "EMBL-Search.h"
#include "EMBL-Search.rsrc.h"

#define headerMargin	10
#define topMargin		40
#define bottomMargin	60
#define footerMargin	10

#define leftMargin 	40
#define rightMargin	40

/*
******************************* Prototypes ***************************
*/

#include "print.h"
#include "util.h"
#include "window.h"
#include "pstr.h"
#include "sequence.h"
#include "hitstorage.h"

static void DoPrinting(WDPtr wdp);
static void PrintHeader(Rect *printRect, FONTINFO *fCourier, StringPtr windowName, short page);
static void PrintFooter(Rect *printRect, FONTINFO *fCourier, StringPtr dateInfo);
static pascal void myIdleProc(void);
static short HowMany(void);



/*
******************************** Global variables ********************
*/

extern Str255	gAppName;
static THPrint	prRecHdl;							/* print info */


/**************************************
*	Prepare printing
*/

void PrepPrint()
{
	PrOpen();
	prRecHdl=(THPrint)NewHandle(sizeof(TPrint));	/* get new print record	*/
	if(prRecHdl)
		PrintDefault(prRecHdl);							/* validate it								*/
	else
		FatalErrorMsg(ERR_MEMORY);
	PrClose();
}


/**************************************
*	Show page setup
*	Return value:	none
*/

void PrintDialog()
{
	PrOpen();									/* open/close PrintMgr acc. to TN	*/
	PrStlDialog(prRecHdl);					/* get page setup 						*/
	PrClose();
}

/**************************************
*	Main print loop
*	Return value:	none
*/

void PrintIt(WDPtr wdp)
{
	short			copies;
	GrafPtr		savePort;
	DialogPtr	mySpoolDlg,myPrintDlg;
	TPrStatus	myPrStatus;
	
	if( wdp == NULL )
		return;
		
	GetPort(&savePort);
	PrOpen();									/* init PrintMgr */
	PrSetError(noErr);						/* just in case last print job was 
														cancelled by user */

	if(PrJobDialog(prRecHdl)) {			/* get print job details */
		StartWaitCursor();
		for(copies=HowMany();copies && !PrError();--copies) {
			/* show status information */
			if((**prRecHdl).prJob.bJDocLoop == bSpoolLoop) {
				CenterDA('DLOG',SPOOLINFO_DLG,50);
				ShowWindow(mySpoolDlg=GetNewDialog(SPOOLINFO_DLG,NULL,(WindowPtr)-1));
				DrawDialog(mySpoolDlg);
			}
			else {
				CenterDA('DLOG',PRINTINFO_DLG,50);
				ShowWindow(myPrintDlg=GetNewDialog(PRINTINFO_DLG,NULL,(WindowPtr)-1));
				DrawDialog(myPrintDlg);
			}
			
			DoPrinting(wdp);

			/* now print spool file */
			if((**prRecHdl).prJob.bJDocLoop == bSpoolLoop && !PrError()) {
				DisposDialog(mySpoolDlg);
				CenterDA('DLOG',PRINTINFO_DLG,50);
				ShowWindow(myPrintDlg=GetNewDialog(PRINTINFO_DLG,NULL,(WindowPtr)-1));
				DrawDialog(myPrintDlg);
				PrPicFile(prRecHdl,NULL,NULL,NULL,&myPrStatus);
			} /* end if */
			DisposDialog(myPrintDlg);
		} /* end for */
		
		if(PrError())
			if(PrError() == iPrAbort)
				ErrorMsg(ERR_PRINTABORT);
			else
				ErrorMsg(ERR_PRINT);
		InitCursor();
	} /* end if */
	
	PrClose();
	SetPort(savePort);
}

/**************************************
*	Actual print routines
*	Return value:	none
*/

static void DoPrinting(WDPtr wdp)
{
	TPPrPort			myPrPort;
	FONTINFO			fCourier;
	Rect				printRect;
	short 			linesPerPage;
	short 			line=0;
	short 			lineBase;
	unsigned long	dateTime;
	Str255			temp,dateInfo,windowName;
	short				page;
	ResultHdl		resHdl;
	SeqRecHdl		seqRecHdl;
	CString80Hdl	bufHdl;
	SignedByte		oldState;
	short				hitPos,bufPos;
	GrafPtr			savePort;
	
	GetDateTime(&dateTime);
	IUDateString(dateTime,longDate,dateInfo);
	IUTimeString(dateTime,FALSE,temp);
	pstrcat(dateInfo,"\p    ");
	pstrcat(dateInfo,temp);
	
	GetWTitle((WindowPtr)wdp,windowName);

	/* to have rotating wait cursor: */
	(**prRecHdl).prJob.pIdleProc=(ProcPtr)myIdleProc;

	savePort = ChangePort((GrafPtr)(myPrPort=PrOpenDoc(prRecHdl,NULL,NULL)));
			
	/* replace Monaco by Courier (looks nicer in print) */
	GetFNum("\pCourier",&fCourier.num);
	TextSize(10);
	TextFont(fCourier.num);
	GetFontInfo(&fCourier.finfo);
	fCourier.height=fCourier.finfo.ascent+ fCourier.finfo.descent+
					fCourier.finfo.leading;
	
	/* calculate lines per page */
	printRect=(**prRecHdl).prInfo.rPage;
	linesPerPage=
		(printRect.bottom-printRect.top-topMargin-bottomMargin)/
		fCourier.height;
/*	leftMargin=
		(printRect.right-printRect.left-80*fCourier.finfo.widMax)/2;
*/
									
	switch(((WindowPeek)wdp)->windowKind) {
		case queryW:
			break;
		case seqW:
			seqRecHdl = (SeqRecHdl)(wdp->userHandle);
			if(seqRecHdl == NULL)
				break;
				
			/* Lock down sequence data */
			bufHdl = (**seqRecHdl).lineBufHdl;
			oldState=LockHandleHigh((Handle)bufHdl);
			
			hitPos = bufPos = 0;
			for(page=1; !PrError() && hitPos < (**seqRecHdl).nlines; ++page) {
				PrOpenPage(myPrPort,NULL); /* create new page */
				if(!PrError()) {
					PrintHeader(&printRect,&fCourier,windowName,page);
					MoveTo(printRect.left+leftMargin,
					       lineBase = printRect.top+topMargin+fCourier.height);
					/* Now print line by line */
					for(line=0;
						line < linesPerPage && hitPos < (**seqRecHdl).nlines;
						++line) {
						if( hitPos < (**seqRecHdl).buftop ||
							 hitPos >= (**seqRecHdl).buftop + SEQBUFLINES ) {
							if(!FillLineBuffer(seqRecHdl,hitPos)) {
								/* we fake a print error */
								PrSetError(iPrAbort);
								break;
							}
							else
								bufPos = 0;
						}
						DrawString((StringPtr)(*bufHdl)[bufPos]);
						MoveTo(printRect.left+leftMargin,lineBase += fCourier.height);	
						++hitPos,++bufPos;
					}
					PrintFooter(&printRect,&fCourier,dateInfo);
				}
				PrClosePage(myPrPort);
			}
			
			HSetState((Handle)bufHdl,oldState);
			break;
			
		case resW:
			resHdl = (ResultHdl)(wdp->userHandle);
			if(resHdl == NULL)
				break;
				
			/* Lock down result data */
			bufHdl = (**resHdl).descBufHdl;
			oldState=LockHandleHigh((Handle)bufHdl);
			
			hitPos = bufPos = 0;
			for(page=1; !PrError() && hitPos < (**resHdl).nhits; ++page) {
				PrOpenPage(myPrPort,NULL); /* create new page */
				if(!PrError()) {
					PrintHeader(&printRect,&fCourier,windowName,page);
					MoveTo(printRect.left+leftMargin,
					       lineBase = printRect.top+topMargin+fCourier.height);
					/* Now print line by line */
					for(line=0;
						line < linesPerPage && hitPos < (**resHdl).nhits;
						++line) {
						if( hitPos < (**resHdl).buftop ||
							 hitPos >= (**resHdl).buftop + MAXBUFLINES ) {
							if(!FillDEBuffer(resHdl,hitPos,FALSE)) {
								/* we fake a print error */
								PrSetError(iPrAbort);
								break;
							}
							else
								bufPos = 0;
						}
						DrawString((StringPtr)(*bufHdl)[bufPos]);
						MoveTo(printRect.left+leftMargin,lineBase += fCourier.height);	
						++hitPos,++bufPos;
					}
					PrintFooter(&printRect,&fCourier,dateInfo);
				}
				PrClosePage(myPrPort);
			}
			
			HSetState((Handle)bufHdl,oldState);
			break;
	} /* end switch */
			
	PrCloseDoc(myPrPort);
	SetPort(savePort);
}

/**************************************
*	Print a nice header
*	Return value:	none
*/

static void PrintHeader(Rect *printRect, FONTINFO *fCourier, StringPtr windowName,
								short page)
{
	Str255 pageStr,numStr;
	
	TextFace(bold);
	MoveTo(printRect->left + leftMargin,
			printRect->top + headerMargin + fCourier->height);
	DrawString(windowName);

	GetIndString(pageStr,OTHERS,PAGESTR);
	NumToString((long)page,numStr);
	pstrcat(pageStr,numStr);

	MoveTo(printRect->right - rightMargin - StringWidth(pageStr),
		printRect->top + headerMargin + fCourier->height);
	DrawString(pageStr);
	TextFace(0);
}

/**************************************
*	Print a nice footer
*	Return value:	none
*/

static void PrintFooter(Rect *printRect, FONTINFO *fCourier, StringPtr dateInfo)
{
	Str255 pageStr;
	
	TextFace(bold);
	MoveTo(printRect->left + leftMargin,
			printRect->bottom - footerMargin - fCourier->height);
	DrawString(gAppName);

	MoveTo(printRect->right - rightMargin - StringWidth(dateInfo),
		printRect->bottom - footerMargin - fCourier->height);
	DrawString(dateInfo);
	TextFace(0);
}


/**************************************
*	Check for user-initiated print cancelling
*	Return value:	none
*/

static pascal void myIdleProc()
{
	EventRecord myEvent;
	GrafPtr		savePort;	/* actually no need to store GrafPort (TN294) because we
										don't draw */
	
	RotateWaitCursor();	
	if(WaitNextEvent(everyEvent,&myEvent,0L,NULL)) {
		if(myEvent.what == keyDown || myEvent.what == autoKey) {
			if( CmdPeriod(&myEvent) || ( (myEvent.message & keyCodeMask) >> 8 == ESCAPE))
				PrSetError(iPrAbort);
		}
	}
}


/**************************************
*	Determine number of copies; only required for draft printing
*	Return value:	number of copies to print
*/

static short HowMany()
{
	return(((**prRecHdl).prJob.bJDocLoop == bDraftLoop) ?
			  (**prRecHdl).prJob.iCopies : 1);
}

