/* RandomDotPrint.c
	by David Phillip Oster October 1994 oster@netcom.com
	for:
	Stuart Inglis singlis@waikato.ac.nz
	Department of Computer Science
	University of Waikato, Hamilton, New Zealand
 */
#include <Printing.h>
#include "RandomDotMain.h"
#include "RandomDotRes.h"

#include "Error.h"
#include "RandomDotWin.h"
#include "RandomDotPrint.h"
#include "Utils.h"

static THPrint thePrin = NIL;

/* GetGlobalPrinRec - 
	get a default print record from our resource file.
 */
static THPrint GetGlobalPrinRec(void){
	if(NIL == thePrin){
		thePrin = (THPrint) GetPreferencesHandle('Prec', 128);
	}
	if(NIL == thePrin){
		thePrin = (THPrint) NewHandleClear(sizeof(TPrint));
		PrintDefault(thePrin);
	}
	return thePrin;
}

/* DoRandomDotPageSetup - 
	fetch the inital print handle in the preferences file (if it exists)
	run the page setup dialog. 
	save the print handle in the preferences file (if it exists)
 */
OSErr DoRandomDotPageSetup(void){
	OSErr	errCode;
	THPrint	h;

	if(FreeMem() < 50000){
		TellError(memFullErr);
		return memFullErr;
	}
	PrOpen();	/* changes current res file to printer driver's */
	if(noErr == (errCode = PrError())){
		h = GetGlobalPrinRec();
		PrValidate(h);
		if(PrStlDialog(h)){
			SavePreferencesResource((Handle) h, 'Prec', 128);
		}
		PrClose();
	}
	TellError(errCode);
	return errCode;
}

/* ScaleCenter - destR will be the same proportions as srcR, 
	scaled by the ratio (hRes / srcRes), (vRes / srcRes)
	centered on destR
 */
static void ScaleCenter(const Rect *srcR, Fixed hRes, Fixed vRes, const Rect *pageR, Rect *destR){
	Point	where;
	Fixed	srcRes;

	srcRes = 72L << 16;

	destR->left = destR->top = 0;
	destR->right =  FixRound(FixMul(FixDiv(FixRatio(srcR->right - srcR->left, 1), srcRes), hRes));
	destR->bottom = FixRound(FixMul(FixDiv(FixRatio(srcR->bottom - srcR->top, 1), srcRes), vRes));

	where.h = pageR->left + ( (pageR->right - pageR->left) - 
				(destR->right - destR->left) )/2;
	where.v = pageR->top + ( (pageR->bottom - pageR->top) - 
				(destR->bottom - destR->top) )/2;
	OffsetRect(destR, where.h - destR->left, where.v - destR->top);
}

/* DoRandomDotPrint - 
 */
OSErr DoRandomDotPrint(void){
	OSErr				errCode;
	RandomDotWindowPtr	win;
	Fixed				hRes, vRes;
	Rect				paperR;
	THPrint				h;
	TPPrPort			prPort;
	WindowPtr			save2;
	Rect				frame;

	if(NIL != (win = (RandomDotWindowPtr) FrontWindow()) && userKind == ((WindowPeek) win)->windowKind){
		SetPort((WindowPtr) win);
	}else{
		return noErr;
	}
	PrOpen();	/* changes current res file to printer driver's */
	if(noErr == (errCode = PrError())){
		h = GetGlobalPrinRec();
		PrValidate(h);
		if(PrJobDialog(h)){
			hRes = FixRatio((**h).prInfo.iHRes, 1);	/* printer res in dpi */
			vRes = FixRatio((**h).prInfo.iVRes, 1);	/* printer res in dpi */
			paperR = (**h).rPaper;
			prPort = NIL;
			prPort = PrOpenDoc(h, NIL, NIL);  
			if(noErr == (errCode = PrError())){
				PrOpenPage(prPort, NIL);
				if(noErr == (errCode = PrError())){
					GetPort(&save2);
					SetPort((GrafPtr) prPort);
					ScaleCenter(&win->frame, hRes, vRes, &paperR, &frame);
					RandomDotCopybitsWin(win, &frame);
					SetPort(save2);
				}
				PrClosePage(prPort);
			}
			if(NIL != prPort){
				PrCloseDoc(prPort);
			}
			if(noErr == errCode && bSpoolLoop == (**h).prJob.bJDocLoop){
				PrPicFile(h, NIL, NIL, NIL, NIL);
			}
		}
		PrClose();
	}
	SetPort((WindowPtr) win);
	TellError(errCode);
	return errCode;
}
