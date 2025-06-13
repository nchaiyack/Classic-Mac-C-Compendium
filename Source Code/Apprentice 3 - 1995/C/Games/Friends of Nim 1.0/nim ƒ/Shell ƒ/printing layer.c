#include "printing layer.h"
#include <Printing.h>

THPrint gPrinterRecord;

void InitThePrinting(void)
{
	gPrinterRecord = (THPrint) NewHandle(sizeof(TPrint));
	if (gPrinterRecord != nil)
	{
		/* if we got a print handle, initialize it to default values */
		PrOpen();
		PrintDefault(gPrinterRecord);
		PrClose();
	};
}

void DoThePageSetup(void)
{
	PrOpen();
	if (PrError() == noErr)
		(void) PrStlDialog(gPrinterRecord);
	PrClose();
}

void PrintText(TEHandle theText)
{
	const short kMargin = 20;		/* page margins in pixels */
	const Rect zeroRect = { 0, 0, 0, 0 };
	short totalLines;
	GrafPtr oldPort;
	Rect oldViewRect;
	Rect oldDestRect;
	Rect viewRect;
	Rect updateRect;
	Rect clipRect;
	short totalHeight;
	short currentLine;
	short scrollAmount;
	TPrStatus thePrinterStatus;
	Boolean printManagerIsOpen = false;
	Boolean userHasCancelled = false;
	short viewHeight;
	TPPrPort thePrinterPort;
	
	if (gPrinterRecord != nil)
	{
		PrOpen();
		if (PrJobDialog(gPrinterRecord))
		{
			GetPort(&oldPort);
			oldViewRect = (*theText)->viewRect;
			oldDestRect = (*theText)->destRect;
			thePrinterPort = PrOpenDoc(gPrinterRecord, nil, nil);
			printManagerIsOpen = (PrError() == noErr);
		};
	};
	
	if (printManagerIsOpen)
	{
		SetPort((GrafPtr) thePrinterPort);
		
		/* re-wrap the text to fill the entire page minus margins */
		viewRect = (*gPrinterRecord)->prInfo.rPage;
		InsetRect(&viewRect, kMargin, kMargin);
		(*theText)->inPort = (GrafPtr) thePrinterPort;
		(*theText)->destRect = viewRect;
		(*theText)->viewRect = viewRect;
		TECalText(theText);
		totalLines = (*theText)->nLines;
		totalHeight = TEGetHeight(totalLines, 0, theText);
		(*theText)->destRect.bottom = (*theText)->destRect.top + totalHeight;
		
		currentLine = 1;
		
		while ((!userHasCancelled) && (currentLine <= totalLines))
		{
			PrOpenPage(thePrinterPort, nil);
			scrollAmount = 0;
			clipRect = (*gPrinterRecord)->prInfo.rPage;
			ClipRect(&clipRect);
			
			viewHeight = (*theText)->viewRect.bottom - (*theText)->viewRect.top + 1;
			
			while (((scrollAmount + TEGetHeight(currentLine, currentLine, theText)) <= viewHeight)
						&& (currentLine <= totalLines))
			{
				scrollAmount += TEGetHeight(currentLine, currentLine, theText);
				currentLine++;
			};
			
			(*theText)->viewRect.bottom = scrollAmount + kMargin;
			TEDeactivate(theText); /* avoid printing selections */
			updateRect = (*theText)->viewRect;
			TEUpdate(&updateRect, theText);
			ClipRect(&zeroRect); /* prevent TEScroll from redrawing the text */
			TEScroll(0, -scrollAmount, theText); /* scroll so we can print the next page */
			(*theText)->viewRect.bottom = viewRect.bottom; /* reset to full page */
			
			if (PrError() == iPrAbort)
				userHasCancelled = true;
			PrClosePage(thePrinterPort);
		};
		
		PrCloseDoc(thePrinterPort);

		if ((*gPrinterRecord)->prJob.bJDocLoop == bSpoolLoop && PrError() == noErr)
			PrPicFile(gPrinterRecord, nil, nil, nil, &thePrinterStatus);
		PrClose();
		
		SetPort(oldPort);
		(*theText)->inPort = oldPort;
		(*theText)->viewRect = oldViewRect;
		(*theText)->destRect = oldDestRect;
		TECalText(theText);
		updateRect = (*theText)->viewRect;
		TEUpdate(&updateRect, theText);
	};
}
