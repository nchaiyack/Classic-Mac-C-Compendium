From: kenlong@netcom.com (Ken Long)
Subject: MyNewCard.c

Same as the one in my last post, here, except Matt Mora modified it to 
display the color icon.  I had no experience with any "IconSuite" 
headers, nor did I get any of them with any of the programming 
environments I have (bummer).  So I got the proto from a Think Reference 
example code copy and pasted it into Icon.h, which worked.  Evedently, 
those "suite" protos are in some new Icons.h, because Matt had it 
included.  I don't recall the inclusion being there before.

Anyway, I changed the zoomRects to a solid line (from a gray pat), and 
centered the origin of the zoom at the top.

By the way, ZoomRect.c and .h are part of the Mac Prog Secrets source 
(not where this came from, though).

You'll need a resource with an 'icl8' in a project with MacTraps and ANSI 
to run this without substituting the .c file in "MyCard" project.

Enjoy!

-Ken-
//� MyCard.c

#include <icons.h>
#define		ONE 		65536L
#define		ZOOMSTEPS	16

WindowPtr		aboutW;
Rect			aRect, cRect;
char			aStr[255];
EventRecord		aboutEvt;
Boolean			aboutDone;
Fixed	fract;
SysEnvRec gMac;

//� Prototypes.

void	drawabout (void);
void	NewAbout (void);
int		Blend (int i1, int i2);
int		zoomrect (Rect *smallrect, Rect *bigrect, Boolean zoomup);
int		ltog (Rect *r);
int		zoomport (WindowPtr wind, Boolean up);
int		centerwindow (WindowPtr wind, Rect *r);
int		centerrect (Rect *r1, Rect *r2);
void	InitMacintosh (void);
int		main (void);

void drawabout () 
{
	//� Draw the contents of the "About..." window.
	
	Handle ourIcon;
	Rect iconRect;
	
	
	TextFont (0);
	TextSize (18);

	ForeColor (blueColor);

	strcpy (aStr, "\pKen Long");	

	TextFace (condense);
	MoveTo (((aboutW->portRect.right) - StringWidth ((StringPtr) aStr)) /2, 22);
	DrawString ((StringPtr) aStr);

	TextSize (12);
	
	ForeColor (redColor);

	strcpy (aStr, "\pA  'C'  Source Code Collector & Distributor");
	TextFace (condense);
	MoveTo (((aboutW->portRect.right) - StringWidth ((StringPtr) aStr)) /2, 40);
	DrawString ((StringPtr) aStr);
	TextFace (0);

	TextFont (3);
	TextSize (9);

	ForeColor (blackColor);

	strcpy (aStr, "\pThink C� Advocate");
	MoveTo (((aboutW->portRect.right) - StringWidth ((StringPtr) aStr)) /2, 58);
	DrawString ((StringPtr) aStr);

	strcpy (aStr, "\p");
	MoveTo (((aboutW->portRect.right) - StringWidth ((StringPtr) aStr)) /2, 70);
	DrawString ((StringPtr) aStr);

	ForeColor (redColor);

	strcpy (aStr, "\p\"I just can't get enough C source code!\"");
	MoveTo (((aboutW->portRect.right) -StringWidth ((StringPtr) aStr)) /2, 82);
	DrawString ((StringPtr) aStr);

	ForeColor (blackColor);

	SetRect (&iconRect, 152, 90, 184, 122);

	if (gMac.systemVersion >= 0x0700) {
		PlotIconID (&iconRect, 0, 0, 128);
	} 
	else 
		{
			ourIcon = GetResource ('ICN#', 128);
			PlotIcon (&iconRect, ourIcon);
	
	}
	
	ReleaseResource (ourIcon);

	strcpy (aStr, "\pkenlong@netcom.com");
	MoveTo (aboutW->portRect.left+4 , aboutW->portRect.bottom - 4);
	DrawString ((StringPtr) aStr);

	strcpy (aStr, "\pkenlong@aol.com");
	MoveTo (aboutW->portRect.right - 
			StringWidth ((StringPtr) aStr) - 4 , 
			aboutW->portRect.bottom - 4);
			
	DrawString ((StringPtr) aStr);
}

void NewAbout () 
{
	GrafPtr		tempPort;

	//� Rather than using a dialog, just create a window to draw the
	//� "About..." stuff in.
	
	GetPort (&tempPort);
	InitCursor ();
	SetRect (&aRect, 0, 0, 340, 150);
	centerrect (&aRect, &screenBits.bounds);
	if (gMac.hasColorQD) 
		aboutW = NewCWindow (nil, &aRect, "\pWindow", FALSE, 3, (WindowPtr) -1, FALSE, 0);
	else
		aboutW = NewWindow (nil, &aRect, "\pWindow", FALSE, 3, (WindowPtr) -1, FALSE, 0);
	
	SetPort (aboutW);
	ForeColor (redColor);
	zoomport (aboutW, TRUE);

	aboutDone = FALSE;
	
	//� do our own event-handling until the user either clicks the 
	//� mouse, or presses a key on the keyboard

	do
	{
		if (GetNextEvent (everyEvent, &aboutEvt)) 
		{
			switch (aboutEvt.what) 
			{
				case updateEvt:		
				{
					BeginUpdate (aboutW);
					drawabout ();
					EndUpdate (aboutW);
				}
				break;
											
				case keyDown:
				case autoKey:
				case mouseDown:
					aboutDone = TRUE;
				break;
											
				default:
				break;
			}
		}
	}while (!aboutDone);
	HideWindow (aboutW);
	zoomport (aboutW, FALSE);
	DisposeWindow (aboutW);
	SetPort (tempPort);
}

int Blend (int i1, int i2) 
{
	Fixed	smallFix, bigFix, tempFix;

	smallFix = ONE * i1;
	bigFix = ONE * i2;
	tempFix = FixMul (fract, bigFix) +FixMul (ONE-fract, smallFix);
	return (FixRound (tempFix));
}

int zoomrect (Rect *smallrect, Rect *bigrect, Boolean zoomup) 
{
	Fixed		factor;
	Rect		rect1, rect2, rect3, rect4;
	GrafPtr		savePort, deskPort;
	int			i;
	long        tm;

	GetPort (&savePort);
	OpenPort (deskPort = (GrafPtr) NewPtr (sizeof (GrafPort)));
	InitPort (deskPort);
	SetPort (deskPort);
//� PenPat (gray);			//� Original.
	PenPat (black);
//� PenMode (notPatXor);	//� Orig. wasn't really needed for gray.
	PenMode (patXor);		//� Changed to make black show up.
	if (zoomup) 
	{
		rect1 = *smallrect;
		factor = FixRatio (6, 5);
		fract = FixRatio (541, 10000);
	}
	else
		{
			rect1 = *bigrect;
			factor = FixRatio (5, 6);
			fract = ONE;
	}
	rect2 = rect1;
	rect3 = rect1;
	FrameRect (&rect1);
	for (i=1;i<=ZOOMSTEPS;i++) 
	{
		rect4.left = Blend (smallrect->left, bigrect->left);
		rect4.right = Blend (smallrect->right, bigrect->right);
		rect4.top = Blend (smallrect->top, bigrect->top);
		rect4.bottom = Blend (smallrect->bottom, bigrect->bottom);
		FrameRect (&rect4);
		FrameRect (&rect1);
		rect1 = rect2;
		rect2 = rect3;
		rect3 = rect4;
		fract = FixMul (fract, factor);
		tm = TickCount ();
		while (tm == TickCount ());
		tm = TickCount ();
		while (tm == TickCount ());
	}
	FrameRect (&rect1);
	FrameRect (&rect2);
	FrameRect (&rect3);
	ClosePort (deskPort);
	DisposPtr ((Ptr) deskPort);
	PenNormal ();
	SetPort (savePort);
}

ltog (Rect *r) 
{
	Point	p1, p2;

	p1 = topLeft (*r);
	p2 = botRight (*r);
	LocalToGlobal (&p1);
	LocalToGlobal (&p2);
	Pt2Rect (p1, p2, r);
}

int zoomport (WindowPtr wind, Boolean up) 
{
	Rect	r1, r2, r3;
	
	SetPort (wind);
//� SetRect (&r1, 0, 20, 0, 20);	//� Original.
	SetRect (&r1, screenBits.bounds.right / 2, 
				 0,	 //� optional //� screenBits.bounds.bottom / 2, 
				 screenBits.bounds.right / 2, 
				 0); //� optional //� screenBits.bounds.bottom / 2);
	r3 = wind->portRect;
	r2 = r3;
	InsetRect (&r2, (r3.right - r3.left + 20) /2, 
				  (r3.bottom - r3.top + 20) /2);
	
	ltog (&r2);
	ltog (&r3);
	
	if (up) 
	{
		zoomrect (&r1, &r2, TRUE);
		zoomrect (&r2, &r3, TRUE);
		ShowWindow (wind);
		SetPort (wind);
	}
	else
		{
			HideWindow (wind);
			zoomrect (&r2, &r3, FALSE);
			zoomrect (&r1, &r2, FALSE);
	}
}
	
int centerwindow (WindowPtr wind, Rect *r) 
{
	Rect	r2;
	
	r2 = wind->portRect;
	MoveWindow	 (wind, 
				 ((r->right-r->left) - 
				  (r2.right-r2.left)) /2 - r->left, 
				 ((r->bottom-r->top) - 
				  (r2.bottom-r2.top)) /2 - r->top, 
				   false);
}

int centerrect (Rect *r1, Rect *r2) 
{
	OffsetRect	 (r1, 
				 ((r2->right  - r2->left) -
				 (r1->right  - r1->left)) / 2 - r1->left, 
				 ((r2->bottom - r2->top)  -
				 (r1->bottom - r1->top)) / 2 - r1->top);
}


void InitMacintosh (void) 
{
	MaxApplZone ();
	
	InitGraf (& (qd.thePort));
	InitFonts ();
	FlushEvents (everyEvent, 0);
	InitWindows ();
	InitMenus ();
	TEInit ();
	InitDialogs (0L);
	InitCursor ();
	SysEnvirons (1, &gMac);
}

main () 
{
	InitMacintosh ();
	NewAbout ();
}

-------->>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>><<<<<<<<<<<<<<<<<<<<<------------
