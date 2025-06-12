/* ----------------------------------------------------------------------

	cicnButton CDEF
	version 1.0.0
	20 March 1994
	
	Written by: Paul Celestin
	
	This CDEF displays a cicn whose resource ID is derived from the
	value field of the CNTL. The min, max, and refcon fields are not
	used and should therefore not be set.
	
	940320 - 1.0.0 - initial release
	
---------------------------------------------------------------------- */

# include	<Controls.h>
# include	<QuickDraw.h>
# include	<Resources.h>

# define	INVERTED_ICON	1

/* ----------------------------------------------------------------------
prototypes
---------------------------------------------------------------------- */

pascal 	long		main(short, ControlHandle, short, long);
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
drawIt - here is where we actually draw the control
---------------------------------------------------------------------- */
static void drawIt
	(
	ControlHandle	control,
	short			variation
	)

{
	Rect 			myRect;
	short 			myICONID;
	CIconHandle		myICON;
	GrafPtr			myPort;
	Pattern			myGray;
	PenState		oldPenState;
	Str255			myTitle;
	int				savedFont,
					savedSize,
					savedMode;
	OSErr			myErr;

	GetPort(&myPort);							/* save off the current port */

	if (!(*control)->contrlVis)					/* if not visible, do nothing */
		return;

	myICONID = GetCtlValue(control);			/* base ID is stored in the value field */
	
	if ((*control)->contrlHilite == inButton)
		myICONID = myICONID + INVERTED_ICON;	/* invert while tracking */
	
	myRect = (*control)->contrlRect;			/* get the rectangle of the control */
	
	myICON = GetCIcon(myICONID);				/* get the appropriate cicn resource */
	
	if ( myICON == 0L )							/* make sure the cicn exists */
		return;
	
	EraseRect(&myRect);							/* erase before drawing */
	PlotCIcon(&myRect,myICON);					/* draw the cicn */
	
	savedFont = myPort->txFont;					/* save of values before drawing title */
	savedSize = myPort->txSize;
	savedMode = myPort->txMode;

	TextFont(geneva);							/* change font to Geneva 9 point */
	TextSize(9);
	TextMode(srcOr);
	BlockMove(((*control)->contrlTitle),myTitle,((*control)->contrlTitle)[0] + 1);			
	MoveTo((myRect.right+myRect.left) / 2 - StringWidth(myTitle) / 2,myRect.bottom + 12);
	DrawString(myTitle);						/* draw the title */

	TextFont(savedFont);						/* restore the values that were saved */
	TextSize(savedSize);
	TextMode(savedMode);

	if ((*control)->contrlHilite == 255)		/* gray out the cicn */
	{
		GetPenState(&oldPenState);
		StuffHex(&myGray, "\pAA55AA55AA55AA55");
		PenPat(myGray);
		PenMode(srcBic);
		PaintRect(&myRect);
		SetPenState(&oldPenState);
	}
	
	return;
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
