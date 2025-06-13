/*
	CmdKey - DA to test whether cmd-key equivalents for Edit menu get
	passed twice to DA's.  (This seemed to happen in older versions of
	TransSkel.  Build this DA, install it, then run it while a TransSkel
	program is running.

	(Only relevant on pre-MultiFinder systems without WaitNextEvent().)
	
	Paul DuBois
	Wisconsin Regional Primate Research Center
	1220 Capitol Court
	University of Wisconsin-Madison
	Madison, WI  53715-1299  USA
	
	dubois@primate.wisc.edu

	01/31/89 Created.
	10/26/93 Updated for THINK C 6.0.1.
*/


# define	bufSiz		512


typedef enum	/*  menu item numbers  */
{
	quit = 1
};


/*  global variables  */

Boolean		isOpen = false;		/* true if DA already open */
WindowPtr	theWind;
DCtlPtr		dce;
short		dCtlRefNum;


/*
	Redraw the window.  It is assumed that the port is set correctly.
	The ValidRect is done because sometimes this is called before any
	update event is obtained.  The ValidRect cancels any that might be
	pending, preventing two redraws.
*/

Update ()
{
	ValidRect (&theWind->portRect);
}


DrawNum (c)
char	c;
{
static short	h = 10;
static short	v = 12;

	MoveTo (h, v);
	DrawChar (c);
	if ((h += 10) > 120)
	{
		h = 10;
		v += 12;
	}
}


int
main(p, d, n)
cntrlParam	*p;	/*  ==> parameter block  */
DCtlPtr		d;	/*  ==> device control entry  */
short		n;	/*  entry point selector  */
{
Point			thePt;
Rect			r;
EventRecord		*theEvent;

	dCtlRefNum = (dce = d)->dCtlRefNum;

	/*  check to make sure data area was allocated  */
		
	if (d->dCtlStorage == 0)
	{
		if (n == 0)						/*  open  */
			CloseDriver(dCtlRefNum);
	}
	else switch (n)	/*  dispatch  */
	{

	case 0:		/*  open  */

		/*
			Need to set these values each time, because the OS resets
			them before every call...
		*/

		d->dCtlFlags |= dNeedLock | dNeedTime | dNeedGoodBye;
		d->dCtlDelay = 0;
		d->dCtlEMask = updateEvt | activateEvt | mouseDown
					| keyDown | autoKey ;

/*
	Check to see whether already open or not.  If not, set up window,
	window growing limits, etc.  The SelectWindow is for the case where
	the DA is already open but hidden behind some other window.
	Selecting CmdKey from the DA menu when already open simply has the
	effect of bringing it to the front.
*/
	
		if (!isOpen)
		{
			isOpen = true;

			SetRect (&r, 25, 80, 490, 320);
			theWind = NewWindow (nil, &r, "\pCmdKey", true,
									noGrowDocProc, (WindowPtr) -1L, true, 0L);
			((WindowPeek) theWind)->windowKind = dCtlRefNum;
			SetPort (theWind);
			TextFont (monaco);
			TextSize (9);
		}

		SelectWindow (theWind);

		break;

	case 2:		/*  control  */

		SetPort (theWind);
		switch (p->csCode)
		{

		case accEvent:	/* need glasses for next statement */

			theEvent = (EventRecord *) * (long *) &p->csParam;
			switch (theEvent->what)
			{

			case activateEvt:
				SysBeep (2);
				break;
			
			case updateEvt:
			
				BeginUpdate (theWind);
				Update ();
				EndUpdate (theWind);
				break;

			case mouseDown:
				SysBeep (2);
				break;
				
			case autoKey:
			case keyDown:
					DrawNum ('X');
					if (theEvent->modifiers & cmdKey)
					{
					short	i = theEvent->message & charCodeMask;
					
						do {
							DrawNum ((i%10) + '0');
							i /= 10;
						}
						while (i != 0);
					}
			}
			break;

		case accUndo:
			DrawNum ('0');
			break;
		case accCut:
			DrawNum ('2');
			break;
		case accCopy:
			DrawNum ('3');
			break;
		case accPaste:
			DrawNum ('4');
			break;
		case accClear:
			DrawNum ('5');
			break;

		/*case goodBye:
			break;*/

		}
		break;

	case 4:		/*  close  */
		DisposeWindow (theWind);	/* get rid of window and controls */
		break;
	}
	
	/*  done  */
	
	return (0);
}
