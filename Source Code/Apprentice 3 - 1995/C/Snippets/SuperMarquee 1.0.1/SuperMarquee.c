// SuperMarquee 1.0.1
// ported to CodeWarrior by Ken Long (kenlong@netcom.com)
// updated for CodeWarrior 6 on 950712

#define NIL 0L

BitMap	StringToBitMap (Str255 s, int h, int v, int descent);
void	InitManagers (void);
void MarqueeDisplay (Str255 s, int displayWidth);
void MarqueeIdle (void);
void MarqueeInit (void);

typedef struct MarqueeRec {
	struct MarqueeRec	*fNext;
	int			fType;
	Rect		fFromRect, fToRect, newRect;
	BitMap		fBits;
	int			fOffset;
	int			fLeftMaximum;
	long		fDelayTimer;
	WindowPtr	fMarqueePort;
} MarqueeRec, *MarqueePtr;

QHdr	gMarqueeQueue;

main ()
{
	Rect		r;
	WindowPtr	w;
	EventRecord e;
	Str255		s1 = "\p This string is too long. ", 
				s2 = "\p This string is also extremely way, way, too long!!! ",
				s3 = "\p If you can read this you are driving too close for proper traffic operational safety and personal well being! ",
				s4 = "\p Four score and seven years ago, our forefathers brought forth upon this continent a new nation.  Conceived in liberty............ ",
				s5 = "\p SUPERMAN!  Strange visitor from another planet, who came to Earth, with powers and abilities far beyond those of mortal men! ";
	
	InitManagers ();
	MarqueeInit ();
	SetRect (&r, 40, 40, 240, 290);
	w = NewWindow (NIL, &r, "\pMarquee Display", true, rDocProc, (WindowPtr)-1L, false, 0);
	SetPort (w);
	TextFont (newYork);
	MoveTo (100, 30);
	MarqueeDisplay (s1, 100);
	TextFont (0);
	MoveTo (100, 60);
	MarqueeDisplay (s2, 100);
	TextFont (monaco);
	TextSize (7);
	MoveTo (100, 85);
	MarqueeDisplay (s3, 100);
	TextFont (courier);
	TextSize (48);
	MoveTo (100, 138);
	MarqueeDisplay (s4, 100);
	TextFont (0);
	TextSize (48);
	MoveTo (100, 218);
	MarqueeDisplay (s5, 100);
	while (! GetNextEvent (mDownMask, &e) )
		MarqueeIdle ();
}

void	InitManagers (void)
{
	InitGraf (&qd.thePort);
	InitFonts ();
	InitWindows ();
	InitMenus ();
	TEInit ();
	InitDialogs (NIL);
	FlushEvents (everyEvent, 0);
	InitCursor ();
}

/***************************/
BitMap	StringToBitMap (Str255 s, int h, int v, int descent)
{
	GrafPtr		saved;
	GrafPort	newPort;
	Rect		r;

	GetPort (&saved);
	OpenPort (&newPort);
	SetRect (&r, 0, 0, h, v);
	BlockMove (&r, &newPort.portRect, sizeof (Rect));

	RectRgn (newPort.visRgn, &r);
	RectRgn (newPort.clipRgn, &r);
	newPort.portBits.rowBytes = ((h + 15) / 16) * 2;
	newPort.portBits.baseAddr = NewPtr (v * newPort.portBits.rowBytes );
	BlockMove (&r, &newPort.portBits.bounds, sizeof (Rect));

	EraseRect (&r);
	MoveTo (0, v - descent);
	TextFont (saved->txFont);
	TextFace (saved->txFace);
	TextSize (saved->txSize);
	DrawString (s);

	SetPort (saved);
	return newPort.portBits;
}
/***************************/
void MarqueeInit ()
{
  gMarqueeQueue.qFlags = 0;
  gMarqueeQueue.qHead = NIL;
  gMarqueeQueue.qTail = NIL;
}  
/***************************/
void MarqueeIdle (void )
{
	MarqueePtr	thisRec;
	
	thisRec = (MarqueePtr) gMarqueeQueue.qHead;
	for (; thisRec != NIL; thisRec = thisRec->fNext )
		if (thisRec->fOffset != 0 && TickCount () > thisRec->fDelayTimer)
		{
			thisRec->fDelayTimer = TickCount ();// + 2;
			
			CopyBits (&thisRec->fBits, &thisRec->fMarqueePort->portBits, &thisRec->fFromRect, &thisRec->fToRect, srcCopy, NIL);
			
			if (thisRec->fFromRect.left + thisRec->fOffset < 0 || thisRec->fFromRect.left + thisRec->fOffset > thisRec->fLeftMaximum) 
			{
				thisRec->fOffset = -thisRec->fOffset;
				thisRec->fDelayTimer = TickCount ();// + 40;
			}
			else
				OffsetRect (&thisRec->fFromRect, thisRec->fOffset, 0);
		}
}
/***************************/
void MarqueeDisplay (Str255 s, int displayWidth)
{
	int			sWidth, dH, lineHeight;
	Point		curLoc;
	FontInfo	fontstuff;
	MarqueePtr	newRec;
	
	sWidth = StringWidth (s);
	if (sWidth < displayWidth) 
	{
		Move (- (sWidth / 2), 0);
		DrawString (s);
 	}
	else
		{
			newRec = (MarqueePtr) (NewPtrClear (sizeof (MarqueeRec) + 4) + 4);
			
			GetFontInfo (&fontstuff);
			
			lineHeight = fontstuff.ascent + fontstuff.descent + fontstuff.leading;
			newRec->fBits = StringToBitMap (s, sWidth, lineHeight, fontstuff.descent);
			
			SetRect (&newRec->fFromRect, 0, 0, displayWidth, lineHeight);
			
			BlockMove (&newRec->fFromRect, &newRec->fToRect, sizeof (Rect));
			GetPen (&curLoc);
			OffsetRect (&newRec->fToRect, curLoc.h - displayWidth / 2, curLoc.v - fontstuff.ascent);
			InsetRect (&newRec->fToRect, -2, -2);
			FrameRect (&newRec->fToRect);
			InsetRect (&newRec->fToRect, 2, 2);
			newRec->fLeftMaximum = sWidth - displayWidth;
			newRec->fOffset = 1;
			newRec->fDelayTimer = TickCount ();// +1;
			GetPort (&newRec->fMarqueePort);
			
			Enqueue ((QElemPtr) newRec, &gMarqueeQueue );
 	}
}
