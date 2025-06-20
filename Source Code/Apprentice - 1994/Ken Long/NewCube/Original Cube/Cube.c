#include <QuickDraw.h>
#include <ToolUtils.h>
#include <qdoffscreen.h>
#include <OSUtils.h>
#include "cube.h"

main()
{
typedef struct QDVar *QDVar;

	QDVar qd;

    InitGraf(&thePort);
    InitFonts();
    FlushEvents(everyEvent, 0);
    InitWindows();
    TEInit();
    InitDialogs((ProcPtr) 0);
    InitCursor();
    HideCursor();
    
    show_w = GetNewWindow(WINDOW2, &w_record2, (WindowPtr) 1); /* Front */
    work_w = GetNewWindow(WINDOW1, &w_record1, (WindowPtr)0);   /* Back */
    SetRect(&show_r, 40, 70, 210, 240);

    SetPort(show_w);
    
    MoveTo(20, 15);  DrawString("\pDpt. of Telecomm. & Computer Systems");
    MoveTo(40, 30);  DrawString("\pRoyal Institute of Technology");
    MoveTo(40, 45);  DrawString("\pS-100 44 Stockholm, SWEDEN");
    MoveTo(80, 60);  DrawString("\proland@ttds");
    MoveTo(40, 260); DrawString("\pPush mouse button to exit");

    SetPort(work_w);
	BackPat(&white);
    BackPat(&black);
	PenPat(&black);
    PenPat(&white);

    fi1 = fi2 = 0;

    while(doneFlag < 30)
    {
	if(Button() || doneFlag)
	{
	    OffsetRect(&show_r, 0, 10);
	    doneFlag++;
	}

	fi1 = (fi1 + 7) m2PI;
	fi2 = (fi2 + 1) m2PI;

	b = sinus[fi2];
	c = (sinus[(64 - fi2) m2PI] * 91) / 128; /* 91/128 = sqrt(2) */

	p0v = 85 +  sinus[fi1];
	p0h = 85 +  (b * sinus[(64 - fi1) m2PI]) / 64;

	p1v = 85 +  sinus[(64 + fi1) m2PI];
	p1h = 85 +  (b * sinus[(- fi1) m2PI]) / 64;

	p2v = 85 +  sinus[(128 + fi1) m2PI];
	p2h = 85 +  (b * sinus[(- 64 - fi1) m2PI]) / 64;

	p3v = 85 +  sinus[(192 + fi1) m2PI];
	p3h = 85 +  (b * sinus[(- 128 - fi1) m2PI]) / 64;

	EraseRect(&work_w->portRect);

	if(! (((((fi2 - 192)m2PI) < 64) && (((fi1 -  32)m2PI) < 128)) ||
		((((fi2 - 128)m2PI) < 64) && (((fi1 - 160)m2PI) < 128)))){
	    MoveTo(p0h - c, p0v); LineTo(p1h - c, p1v);
	}
	if(! (((((fi2 - 192)m2PI) < 64) && (((fi1 - 224)m2PI) < 128)) ||
		((((fi2 - 128)m2PI) < 64) && (((fi1 -  96)m2PI) < 128)))){
	    MoveTo(p1h - c, p1v); LineTo(p2h - c, p2v);
	}
	if(! (((((fi2 - 192)m2PI) < 64) && (((fi1 - 160)m2PI) < 128)) ||
		((((fi2 - 128)m2PI) < 64) && (((fi1 -  32)m2PI) < 128)))){
	    MoveTo(p2h - c, p2v); LineTo(p3h - c, p3v);
	}
	if(! (((((fi2 - 192)m2PI) < 64) && (((fi1 -  96)m2PI) < 128)) ||
		((((fi2 - 128)m2PI) < 64) && (((fi1 - 224)m2PI) < 128)))){
	    MoveTo(p3h - c, p3v); LineTo(p0h - c, p0v);
	}

	if(! (((((fi2     )m2PI) < 64) && (((fi1 -  32)m2PI) < 128)) ||
		((((fi2 -  64)m2PI) < 64) && (((fi1 - 160)m2PI) < 128)))){
	    MoveTo(p0h + c, p0v); LineTo(p1h + c, p1v);
	}
	if(! (((((fi2     )m2PI) < 64) && (((fi1 - 224)m2PI) < 128)) ||
		((((fi2 -  64)m2PI) < 64) && (((fi1 -  96)m2PI) < 128)))){
	    MoveTo(p1h + c, p1v); LineTo(p2h + c, p2v);
	}
	if(! (((((fi2     )m2PI) < 64) && (((fi1 - 160)m2PI) < 128)) ||
		((((fi2 -  64)m2PI) < 64) && (((fi1 -  32)m2PI) < 128)))){
	    MoveTo(p2h + c, p2v); LineTo(p3h + c, p3v);
	}
	if(! (((((fi2     )m2PI) < 64) && (((fi1 -  96)m2PI) < 128)) ||
		((((fi2 -  64)m2PI) < 64) && (((fi1 - 224)m2PI) < 128)))){
	    MoveTo(p3h + c, p3v); LineTo(p0h + c, p0v);
	}

	if(! (((((fi2 -  64)m2PI) < 128) && (((fi1 - 224)m2PI) < 64)) ||
		((((fi2 - 192)m2PI) < 128) && (((fi1 -  96)m2PI) < 64)))){
	    MoveTo(p0h + c, p0v); Line(-2 * c, 0);
	}
	if(! (((((fi2 -  64)m2PI) < 128) && (((fi1 - 160)m2PI) < 64)) ||
		((((fi2 - 192)m2PI) < 128) && (((fi1 -  32)m2PI) < 64)))){
	    MoveTo(p1h + c, p1v); Line(-2 * c, 0);
	}
	if(! (((((fi2 -  64)m2PI) < 128) && (((fi1 -  96)m2PI) < 64)) ||
		((((fi2 - 192)m2PI) < 128) && (((fi1 - 224)m2PI) < 64)))){
	    MoveTo(p2h + c, p2v); Line(-2 * c, 0);
	}
	if(! (((((fi2 -  64)m2PI) < 128) && (((fi1 -  32)m2PI) < 64)) ||
		((((fi2 - 192)m2PI) < 128) && (((fi1 - 160)m2PI) < 64)))){
	    MoveTo(p3h + c, p3v); Line(-2 * c, 0);
	}

	CopyBits(&work_w->portBits, &show_w->portBits,
		 &work_w->portRect, &show_r,
		 notSrcCopy, (RgnHandle) 0);
    }
    ShowCursor();
    ExitToShell();
}
