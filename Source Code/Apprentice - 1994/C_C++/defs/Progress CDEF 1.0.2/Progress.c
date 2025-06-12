/* ----------------------------------------------------------------------

	Progress CDEF
	version 1.0.2
	20 March 1994
	
	Written by: Paul Celestin
	
	This CDEF displays a progress thermometer.
	
	940320 - 1.0.2 - more bug fixes
	
	931012 - 1.0.1 - bug fixes
	
	930927 - 1.0.0 - initial release

---------------------------------------------------------------------- */

# include	<Controls.h>
# include	<QuickDraw.h>

/* ----------------------------------------------------------------------
prototypes
---------------------------------------------------------------------- */
pascal 	long		main(short, ControlHandle, short, long);
	 	void 		drawIt(ControlHandle, short);

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
			break;
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
	PenState		oldPenState;
	int				myValue = GetCtlValue(control),
					myMax = GetCtlMax(control),
					myMin = GetCtlMin(control),
					myColor,
					width = 0,
					range = 100;

	if (!(*control)->contrlVis)				/* if not visible, do nothing */
		return;

	GetPenState(&oldPenState);				/* save off current environment */

	myRect = (*control)->contrlRect;		/* define our control rect */
	
	PenNormal();							/* make pen normal */
	
	FrameRect(&myRect);						/* draw border around the control */
	InsetRect(&myRect,1,1);

	ForeColor(whiteColor);
	BackColor(whiteColor);
	PaintRect(&myRect);

	width = myRect.right - myRect.left;		/* width in pixels of the control */
	range = myMax - myMin;
	
	if (range < 10) range = 10;				/* to deal with screwy ranges */
	
	myRect.right = myRect.left + (myValue * width) / range;
	
	switch (GetCRefCon(control))
	{
		case 1:
			myColor = redColor;
			break;
		case 2:
			myColor = greenColor;
			break;
		case 3:
			myColor = blueColor;
			break;
		case 4:
			myColor = cyanColor;
			break;
		case 5:
			myColor = magentaColor;
			break;
		case 6:
			myColor = yellowColor;
			break;
		case 7:
			myColor = whiteColor;
			break;
		default:
			myColor = blackColor;
			break;
	}
	
	ForeColor(myColor);
	BackColor(myColor);
	PaintRect(&myRect);

	ForeColor(blackColor);
	BackColor(whiteColor);
	SetPenState(&oldPenState);				/* restore the original environment */
	
	return;									/* we are done drawing */
}
