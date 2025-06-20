/* ==========================================

	print.c
	
	Copyright (c) 1993,1994,1995 Newport Software Development
	
	This code is not currently linked into the nShell app.  I had intended
	to allow printing of shell histories (or selections thereof), but never
	got around to it.
	
   ========================================== */

#define topMargin 20
#define leftMargin 20
#define bottomMargin 20
#define tabChar	((char)'\t')

#include "multi.h"

#include "print.proto.h"
#include "utility.proto.h"

short	gPrintErr;

extern	THPrint	gHPrint;

extern	CursHandle	gWaitCursor;

void print_cleanup(void)
{
	if (gHPrint)
		DisposeHandle(gHPrint);
}

int print_error( Str255 action )
{
	if (!gPrintErr)						// if things look good, it's time to worry
		if (gPrintErr = PrError())		// so look for trouble
			if (gPrintErr != iPrAbort)	// ignore "cancel" conditions
				error_note(action);		// in effect, only "notify" regarding new errors 

	return( gPrintErr );
}

int print_setup_handle(void)
{
	if (gHPrint) return(1);
		
	gHPrint = (TPrint **) NewHandle(sizeof( TPrint));
	
	if (!gHPrint) {
		error_note("\pno memory for printing");
		return(0);
		}
		
	PrintDefault(gHPrint);
	return( !print_error("\pcould not set printer defaults") );
}

void print_setup(void)
{
	gPrintErr = 0;		// clear "print_error()" state
	
	PrOpen();
	print_error("\pcould not open printer");
	
	if (print_setup_handle()) {	
		PrStlDialog(gHPrint);
		print_error("\pcould not modify printer state");
		}
	
	PrClose();
	print_error("\pcould not close printer");
}

#define MARGIN	20 // for now

int print_spooler( TEHandle shellTE )
{
	int				scrollAmount;		// height of a page (i think)
	int				cLine;				// current line
	int				cLineHeight;		// height of current line
	int				tLines;				// total lines
	int				tHeight;			// totlal height
	int				viewHeight;
	Rect 			printRect;
	Rect 			zeroRect;
	TPPrPort		printPort;
	
	SetRect( &zeroRect, 0,0,0,0 );

	printPort = PrOpenDoc(gHPrint, 0L, 0L);
	
	if (!printPort) {
		error_note("\pno memory for printer port");
		return(0);
		}

	SetPort(printPort);
	
	printRect = (**gHPrint).prInfo.rPage;
	InsetRect( &printRect, MARGIN, MARGIN );
	
	(**shellTE).inPort = (GrafPtr)printPort;
	(**shellTE).destRect = printRect;
	(**shellTE).viewRect = printRect;
	TECalText( shellTE );

	tLines = (**shellTE).nLines;
	tHeight = TEGetHeight( tLines, 0, shellTE );
	(**shellTE).destRect.bottom = (**shellTE).destRect.top + tHeight;
	
	cLine = 1;
	
	while ((!gPrintErr) && (cLine <= tLines)) {
		PrOpenPage(printPort, 0L);
		if (print_error("\pcould not open printer page")) break;
		scrollAmount = 0;
		ClipRect(&(**gHPrint).prInfo.rPage);
		viewHeight = (**shellTE).destRect.bottom - (**shellTE).destRect.top + 1;
		
		// figure lines per page
		cLineHeight = TEGetHeight( cLine, cLine, shellTE );
		while (((scrollAmount + cLineHeight) <= viewHeight) && (cLine <= tLines)) {
			scrollAmount += cLineHeight;
			cLine++;
			cLineHeight = TEGetHeight( cLine, cLine, shellTE );
			}
			
		(**shellTE).viewRect.bottom = scrollAmount + MARGIN;
		TEDeactivate(shellTE);
		TEUpdate(&(**shellTE).viewRect, shellTE);
		ClipRect(&zeroRect);
		TEScroll(0, -scrollAmount, shellTE);
		(**shellTE).viewRect.bottom = printRect.bottom;

		PrClosePage( printPort );
		print_error("\pcould not close printer page");
		}
	
	PrCloseDoc(printPort);
	print_error("\pcould not close printer port");
	
	return( !gPrintErr );
}

void print_shell( ShellH shell )
{
	TPPrPort	printPort;
	GrafPtr		savePort;
	TEHandle	shellTE;
	TPrStatus	prStatus;
	int			copies;
	Rect		oldView,oldDest;
	
	gPrintErr = 0;		// clear "print_error()" state
	
	shellTE = (**shell).Text;
	
    PrOpen();
	if (!print_error("\pcould not open printer")) {
		if (print_setup_handle()) {
			SetCursor(&arrow);
			if (PrJobDialog(gHPrint)) {
				SetCursor(*gWaitCursor);
				GetPort(&savePort);
				oldView = (**shellTE).viewRect;
				oldDest = (**shellTE).destRect;
				HLock(shellTE);
				for (copies=print_how_many(); copies>0; copies--) {
					if (!print_spooler(shellTE))
						break;
					if ((**gHPrint).prJob.bJDocLoop == bSpoolLoop)
						PrPicFile(gHPrint, 0L, 0L, 0L, &prStatus);
					}
				HUnlock(shellTE);
				SetPort(savePort);
				(**shellTE).inPort = savePort;
				(**shellTE).viewRect = oldView;
				(**shellTE).destRect = oldDest;
				TECalText( shellTE );
				TEActivate(shellTE);
				TEUpdate( &(**shell).WindPtr->portRect, shellTE );
				}
			}
		}
		
	PrClose();
	print_error("\pcould not close printer");
}

int print_how_many(void)
{
	return( ((**gHPrint).prJob.bJDocLoop==bDraftLoop) ? (**gHPrint).prJob.iCopies : 1 );
}
