//¥ Cube.c ¥//

//¥ Originally written by some Swedish dudes, by updated and corrected (some)
//¥ by Kenneth A. Long, on 27 February 1993, to run on Symantec's THINK C,
//¥ version 5.0.4.
//¥ This program demonstrates some fancy math/line drawing "mathrobatics" to
//¥ give the illusion of a rotating cube.  Try working in a button to cause
//¥ a forward rotation per mouse-click, and you'll get an idea of what must
//¥ have gone into the writing of the "Kubik" game/puzzle!
//¥ This is a public domain project.  Have fun - I did!

#include <QuickDraw.h>
#include <ToolUtils.h>
#include <OSUtils.h>

#include "NewCube.h"

main() 
{
//¥	typedef struct QDVar *QDVar;	//¥ I don't know why the Swedes put this
									//¥ in here.  It still works without it.
//¥	QD		QDVar;					//¥ And this is another unknown, unneeded.

	long	ticksNow;	//¥ I put this one in here, for speed control.

	InitGraf(&thePort);
	InitFonts();
	FlushEvents(everyEvent, 0);

	InitWindows();
	show_w = GetNewWindow(window2, &windowRecord2, (WindowPtr)0); /* Back */

//¥ SetRect(&show_r, 20, 70, 210, 240);  //¥ This line made it not run right.
	TEInit();							 //¥ Maybe it was just in the wrong place?
	InitDialogs((ProcPtr) 0 );
	InitCursor();
	HideCursor();
	
	SetPort(show_w);
	MoveTo(10, 15);  DrawString("\pDpt. of Telecomm. & Computer Systems");
	MoveTo(40, 30);  DrawString("\pRoyal Institute of Technology");
	MoveTo(40, 45);  DrawString("\pS-100 44 Stockholm, SWEDEN");
	MoveTo(60, 75);  DrawString("\pDebugged and updated");
	MoveTo(70, 90);  DrawString("\pby Kenneth A. Long");
	MoveTo(75, 105);  DrawString("\p27 February 1993");
	MoveTo(10, 120);  DrawString("\pTo compile and run on THINK Cª v5.0.4");
	MoveTo(45, 140);  DrawString("\p'THINK C' is trademark of");
	MoveTo(50, 155);  DrawString("\pSymantec Corporation");
	MoveTo(50, 180); DrawString("\pPush mouse button to exit");

//¥ This next line was up under the other GNW, but the text and cube draws
//¥ overlaped into both windows, so I moved it.
	work_w = GetNewWindow(window1, &windowRecord1, (WindowPtr)-1L);   /* Front */
	SetPort(work_w);	
	BackPat(&black);
	PenPat(&white);

	fi1 = fi2 = 0;

	while(! Button ())
	{
		fi1 = (fi1 + 7) m2PI;
		fi2 = (fi2 + 1) m2PI;

		b = sinus[fi2];
		c = (sinus[( 64 - fi2) m2PI] * 91) / 128; /* 91/128 = sqrt(2) */

		p0v = 85 +  sinus[fi1];
		p0h = 85 +  (b * sinus[( 64 - fi1) m2PI]) / 64;

		p1v = 85 +  sinus[(64 + fi1) m2PI];
		p1h = 85 +  (b * sinus[( - fi1) m2PI]) / 64;

		p2v = 85 +  sinus[(128 + fi1) m2PI];
		p2h = 85 +  (b * sinus[(- 64 - fi1) m2PI]) / 64;

		p3v = 85 +  sinus[(192 + fi1) m2PI];
		p3h = 85 +  (b * sinus[(- 128 - fi1) m2PI]) / 64;

		EraseRect(&work_w->portRect);

//¥ line 1
		if( ! ( ((((fi2 - 192)m2PI) < 64) && (((fi1 -  32)m2PI) < 128)) ||
				((((fi2 - 128)m2PI) < 64) && (((fi1 - 160)m2PI) < 128)) ) )
		{
			MoveTo(p0h - c, p0v);
			LineTo(p1h - c, p1v);
		}
//¥ try putting a "Delay (ticksNow)" after each line draw.  Really slow!
		
//¥ line 2
		if( ! ( ((((fi2 - 192)m2PI) < 64) && (((fi1 - 224)m2PI) < 128)) ||
				((((fi2 - 128)m2PI) < 64) && (((fi1 -  96)m2PI) < 128)) ) )
		{
			MoveTo(p1h - c, p1v); 
			LineTo(p2h - c, p2v);
		}
//¥ line 3
		if( ! ( ((((fi2 - 192)m2PI) < 64) && (((fi1 - 160)m2PI) < 128)) ||
				((((fi2 - 128)m2PI) < 64) && (((fi1 -  32)m2PI) < 128)) ) )
		{
			MoveTo(p2h - c, p2v); 
			LineTo(p3h - c, p3v);
		}
//¥ line 4
		if( ! ( ((((fi2 - 192)m2PI) < 64) && (((fi1 -  96)m2PI) < 128)) ||
				((((fi2 - 128)m2PI) < 64) && (((fi1 - 224)m2PI) < 128)) ) )
		{
			MoveTo(p3h - c, p3v); 
			LineTo(p0h - c, p0v);
		}
//¥ line 5
		if( ! ( ((((fi2)m2PI) < 64) 	 && (((fi1 -  32)m2PI) < 128)) ||
				((((fi2 - 64)m2PI) < 64) && (((fi1 - 160)m2PI) < 128)) ) )
		{
			MoveTo(p0h + c, p0v); 
			LineTo(p1h + c, p1v);
		}
//¥ line 6
		if( ! ( ((((fi2)m2PI) < 64) 	  && (((fi1 - 224)m2PI) < 128)) ||
				((((fi2 -  64)m2PI) < 64) && (((fi1 -  96)m2PI) < 128)) ) )
		{
			MoveTo(p1h + c, p1v);
			LineTo(p2h + c, p2v);
		}
//¥ line 7
		if( ! ( ((((fi2)m2PI) < 64) && (((fi1 - 160)m2PI) < 128)) ||
				((((fi2 -  64)m2PI) < 64) && (((fi1 -  32)m2PI) < 128)) ) )
		{
			MoveTo(p2h + c, p2v);
			LineTo(p3h + c, p3v);
		}
//¥ line 8
		if( ! ( ((((fi2	  )m2PI) < 64) && (((fi1 -  96)m2PI) < 128)) ||
				((((fi2 -  64)m2PI) < 64) && (((fi1 - 224)m2PI) < 128)) ) )
		{
			MoveTo(p3h + c, p3v);
			LineTo(p0h + c, p0v);
		}
//¥ line 9
		if( ! ( ((((fi2 -  64)m2PI) < 128) && (((fi1 - 224)m2PI) < 64)) ||
				((((fi2 - 192)m2PI) < 128) && (((fi1 -  96)m2PI) < 64)) ) )
		{
			MoveTo(p0h + c, p0v);
			Line(-2 * c, 0);
		}
//¥ line 10
		if( ! ( ((((fi2 -  64)m2PI) < 128) && (((fi1 - 160)m2PI) < 64)) ||
				((((fi2 - 192)m2PI) < 128) && (((fi1 -  32)m2PI) < 64)) ) )
		{
			MoveTo(p1h + c, p1v);
			Line(-2 * c, 0);
		}
//¥ line 11
		if( ! ( ((((fi2 -  64)m2PI) < 128) && (((fi1 -  96)m2PI) < 64)) ||
				((((fi2 - 192)m2PI) < 128) && (((fi1 - 224)m2PI) < 64)) ) )
		{
			MoveTo(p2h + c, p2v);
			Line(-2 * c, 0);
		}
//¥ line 12
		if( ! ( ((((fi2 -  64)m2PI) < 128) && (((fi1 -  32)m2PI) < 64)) ||
				((((fi2 - 192)m2PI) < 128) && (((fi1 - 160)m2PI) < 64)) ) )
		{
			MoveTo(p3h + c, p3v);
			Line(-2 * c, 0);
		}
		Delay (5L, &ticksNow);	//¥ Change rotation speed here.
								//¥ K.A.L. added this line to slow rotation.
								//¥ "1L" is faster.  Commented out, it has too
								//¥ little time to completely draw all lines.
								
		CopyBits(&work_w->portBits, &show_w->portBits,
				 &work_w->portRect, &show_r,
				 notSrcCopy, (RgnHandle) 0);
	}
	ShowCursor();
	ExitToShell();	//¥ this exit is used after the 'while' statement above.
}
