//-- Print.c

// Handle printing this window.
 
#include <PrintTraps.h>

#include <stdio.h>
#include "struct.h"


THPrint hPrint = nil;
WindowPtr statusWindow;
char printMsg[] = "Now printing.  To halt, hit control-'.'; this seems to do the trick...";


//-- OpenWarning

// Open a warning window in the center of the screen.

OpenWarning()
{
	GrafPtr foo;
	int x,y;
	Rect r;

	GetWMgrPort(&foo);
	r = foo->portRect;
	x = (r.right - r.left - 200) / 2;
	y = (r.bottom - r.top - 100) / 2;
	r.top = y;
	r.bottom = r.top + 100;
	r.left = x;
	r.right = r.left + 200;
	statusWindow = NewWindow(nil,&r,"",1,1,(char *)-1,0,0L);
	SetPort(statusWindow);
	TextFont(0);
	TextSize(12);
	r.bottom -= r.top;
	r.top = 0;
	r.right -= r.left;
	r.left = 0;
	InsetRect(&r,8,8);
	TextBox(printMsg,(long)strlen(printMsg),&r,0);
}


//-- CloseWarning

// Close print warning.

CloseWarning()
{
	DisposeWindow(statusWindow);
}



//-- PrintStl

// Print style dialog.

PrintStl()
{
	PrOpen();
	if (hPrint == nil) 
	{
		hPrint = (THPrint)NewHandle(sizeof(TPrint));
		if (hPrint == nil) return;
		PrintDefault(hPrint);
	}
	
	PrStlDialog(hPrint);
	PrClose();
}



//-- PrintJob

// Print job dialog.

PrintJob()
{
	struct DrawWindow *w;
	long len;
	long x;
	struct DirectData *ptr;
	long pagenum;
	TPPrPort pPrPort;
	TPrStatus myPrStatus;
	short ind;
	short xx;
	Rect r;
	short pageNum;
	char buffer[64];

	if (nil == (w = (struct DrawWindow *)FrontWindow())) return;
	if (w->w.windowKind != WK_PLAN) return;
	
	PrOpen();
	if (hPrint == nil) {
		hPrint = (THPrint)NewHandle(sizeof(TPrint));
		if (hPrint == nil) return;
		PrintDefault(hPrint);
	}
	
	if (PrJobDialog(hPrint))
	{
		OpenWarning();
		
		HLock(w->data);
		ptr = *(w->data);
		len = GetHandleSize(w->data) / sizeof(struct DirectData);
		pPrPort = PrOpenDoc(hPrint,nil,nil);
		SetPort(pPrPort);
		r = (**hPrint).prInfo.rPage;				/* Page rectangle */
		r.bottom -= 15;
		x = 0;
		pageNum = 1;
		while (x < len) {
			if (PrError() == noErr) {
				PrOpenPage(pPrPort,nil);
				
				TextFont(4);
				TextSize(9);
				sprintf(buffer,"Page %d",pageNum);
				CtoPstr(buffer);
				MoveTo(r.right - 20 - StringWidth(buffer),25);
				DrawString(buffer);
				
				sprintf(buffer,"Directory listing of %s",w->vName);
				CtoPstr(buffer);
				MoveTo(r.left + 20, 25);
				DrawString(buffer);
				
				MoveTo(r.left + 10, 28);
				LineTo(r.right - 10, 28);
				MoveTo(r.left + 10, 30);
				LineTo(r.right - 10, 30);

				xx = 40 + r.top;
				if (PrError() == noErr) {
					while ((xx < r.bottom) && (x < len)) {
						for (ind = 0; ind < ptr[x].indent; ind++) {
							MoveTo(ind*12+25,xx-10);
							Line(0,12);
						}
	
						MoveTo(ptr[x].indent*12+20,xx);
						DrawString(ptr[x].data);
	
						if (ptr[x].auxdata[0] != '\0') {
							MoveTo(ptr[x].indent*12+25+StringWidth(ptr[x].data),xx);
							LineTo(305,xx);
							MoveTo(370 - StringWidth(ptr[x].auxdata),xx);
							DrawString(ptr[x].auxdata);
							
							MoveTo(390,xx);
							DrawString(ptr[x].auxdata2);
						}
						x++;
						xx += 12;
					}
				}
				PrClosePage(pPrPort);
				pageNum++;
			} else break;
		}
		PrCloseDoc(pPrPort);
		if (((**hPrint).prJob.bJDocLoop == bSpoolLoop) && (PrError() == noErr)) {
			PrPicFile(hPrint,nil,nil,nil,&myPrStatus);
		}

		CloseWarning();
	}
	PrClose();
}
