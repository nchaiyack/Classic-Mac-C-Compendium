/* ----------------------------------------------------------------------

	PICTButton CDEF
	version 1.0.6
	19 March 1994
	
	Written by: Paul Celestin
	
	This CDEF displays a picture whose resource ID is derived from
	the value field of the CNTL. The min, max, and refcon fields are
	not used and should therefore not be set.
	
	940319 - 1.0.6 - bug fixes
	
	931012 - 1.0.5 - bug fixes
	
	931012 - 1.0.4 - bug fixes
	
	930922 - 1.0.3 - bug fixes
	
	930915 - 1.0.2 - bug fixes
	
	930829 - 1.0.1 - added variations 1 and 2
	
	930822 - 1.0.0 - initial release

---------------------------------------------------------------------- */

# include	<Controls.h>
# include	<QuickDraw.h>
# include	<Resources.h>

# define	DEFAULTPICT		0
# define	INVERTEDPICT	1
# define	BW_OFFSET		2

/* ----------------------------------------------------------------------
prototypes
---------------------------------------------------------------------- */

pascal 	long		main(short, ControlHandle, short, long);
		int		 	hasColor();
	 	void 		drawIt(ControlHandle, short);
		long 		testIt(ControlHandle, Point);

/* ----------------------------------------------------------------------
main - here is where it all began...
---------------------------------------------------------------------- */
pascal long main
	(
	short			variation,
	ControlHandle	theControl,
	short			message,
	long			param
	)

{
	long			returnValue = 0L;
	char			state = HGetState((Handle)theControl);

	switch(message)
	{
		case drawCntl:
			drawIt(theControl,variation);
		case testCntl:
			returnValue = testIt(theControl, *(Point *) &param);
		case calcCRgns:
			break;
	  	case initCntl:
	  		break;
		case dispCntl:
			break;
		case posCntl:
			break;
		case thumbCntl:
			break;
		case dragCntl:
			break;
		case autoTrack:
			break;
		case calcCntlRgn:
			break;
		case calcThumbRgn:
			break;
		default:
			break;
	}

	HSetState((Handle)theControl,state);

	return(returnValue);				/* tell them what happened */
}


/* ----------------------------------------------------------------------
hasColor
---------------------------------------------------------------------- */
int hasColor()

{
	SysEnvRec		myComputer;
	GDHandle		curDev;
	PixMapHandle	myPixMap;
	
	SysEnvirons(2,&myComputer);
	if (myComputer.hasColorQD)
	{
		curDev = GetMainDevice();
		myPixMap = (**curDev).gdPMap;
		if ((**myPixMap).pixelSize > 1)
			return(1);
		else
			return(0);
	}
	return(0);
}


/* ----------------------------------------------------------------------
drawIt - here is where we actually draw the control
---------------------------------------------------------------------- */
static void drawIt
	(
	ControlHandle	control,
	short			variation
	)

{
	Rect 			myRect;
	short 			myPictID;
	GrafPtr			myPort;
	PicHandle		myPicture;
	Pattern			myGray;
	PenState		oldPenState;
	Str255			myTitle;
	int				savedFont,
					savedSize,
					savedMode;

	GetPort(&myPort);						/* save off the current port */

	if (!(*control)->contrlVis)				/* if not visible, do nothing */
		return;

	myPictID = GetCtlValue(control);		/* base ID is stored in the value field */
		
	if (!hasColor())							/* use black and white PICTs */
		myPictID = myPictID + BW_OFFSET;
	
	if ((*control)->contrlHilite == inButton)
		myPictID = myPictID + INVERTEDPICT;	/* invert while tracking */
	
	myPicture = (PicHandle)GetResource('PICT', myPictID);
	
	if ( myPicture == 0L )					/* could not find the PICT */
		return;

	LoadResource((Handle)myPicture);
	HNoPurge((Handle)myPicture);

	myRect = (*control)->contrlRect;
	
	DrawPicture(myPicture, &myRect);		/* draw the picture */
	
	switch (variation)
	{
	case 1:									/* display title of control in geneva 9 */
		{
			savedFont = myPort->txFont;		/* save off current values */
			savedSize = myPort->txSize;
			savedMode = myPort->txMode;
			
			ForeColor(blueColor);
			TextFont(geneva);
			TextSize(9);
			TextMode(srcOr);
			
			BlockMove(((*control)->contrlTitle),myTitle,((*control)->contrlTitle)[0] + 1);
			
			MoveTo((myRect.right+myRect.left) / 2 - StringWidth(myTitle) / 2
				,myRect.bottom - 6);
			DrawString(myTitle);
			
			TextFont(savedFont);			/* restore saved values */
			TextSize(savedSize);
			TextMode(savedMode);
			ForeColor(blackColor);
			
			break;
		}
	case 2:									/* simple 2-pixel wide black border */
		{
			PenSize(2,2);
			InsetRect(&myRect,-3,-3);
			FrameRect(&myRect);
			InsetRect(&myRect,3,3);
			PenSize(1,1);
			break;
		}
	}

	if ((*control)->contrlHilite == 255)	/* gray out the picture */
	{
		GetPenState(&oldPenState);
		StuffHex(&myGray, "\pAA55AA55AA55AA55");
		PenPat(myGray);
		PenMode(srcBic);
		PaintRect(&myRect);
		SetPenState(&oldPenState);
	}
	
	HPurge((Handle)myPicture);
	
	return;									/* we are done drawing */
}


/* ----------------------------------------------------------------------
testIt - test for mouse hits within control
---------------------------------------------------------------------- */
static long testIt
	(
	ControlHandle		control,
	Point				myPoint
	)

{
	Rect myRect;

	myRect = (*control)->contrlRect;
	
	if	(
		((*control)->contrlVis != 0) &&
		((*control)->contrlHilite != 255) &&
		(PtInRect(myPoint,&myRect))
		)
		return(inButton);
	else
		return(0);
}
