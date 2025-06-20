#include	"WindUtils.h"

RGBColor		rgbblack = {0,0,0};
RGBColor		rgbwhite = {0xFFFF,0xFFFF,0xFFFF};
RGBColor		rgbdarkred = {0x7FFF,0x0000,0x0000};
RGBColor		rgblitered = {0xFFFF,0x8FFF,0x8FFF};

Boolean	HasColorQD = false;	//other code segments can check here, without checking over and over again

static WindowPtr	savePort;
static short		saveFont;
static short		saveSize;
static short		saveFace;

void	DrawShadowBox(Rect Box, Boolean Inside)	//draw a shadowed rectangle, with shadow either inside or outside
{
	if (Inside)
	{
		Box.top -= 1;
		Box.left -= 1;
	}
	PenSize(1,1);
	FrameRect(&Box);
	if (Inside)
	{
		MoveTo(Box.left + 1, Box.top + 1);
		LineTo(Box.right - 1, Box.top + 1);
		MoveTo(Box.left + 1, Box.top + 1);
		LineTo(Box.left + 1, Box.bottom - 1);
	}
	else
	{
		MoveTo(Box.left + 1, Box.bottom);
		LineTo(Box.right, Box.bottom);
		MoveTo(Box.right, Box.top + 1);
		LineTo(Box.right, Box.bottom);
	}
}

void	SaveTextSettings(void)	//save the text settings for a window, 
{
	GetPort(&savePort);
	saveFont = savePort->txFont;
	saveSize = savePort->txSize;
	saveFace = savePort->txFace;
}

void	RestoreTextSettings(void)
//resotore text settings.  Must follow a SaveTextSettings() with a RestoreTextSettings()
//since only one set of settings is saved here in static variables
{
	TextFont(saveFont);
	TextSize(saveSize);
	TextFace(saveFace);
}

short	MaxTextHeight(void)	//get maximum text height of current font and size
{
FontInfo	myFontInfo;

	GetFontInfo(&myFontInfo);
	return(myFontInfo.ascent + myFontInfo.descent + myFontInfo.leading);
}

/* ------------------------- SetMouse ------------------------- */
/* some dangerous low-memory-global equates */
extern  Point	MTemp           :   0x828;
extern  Point	RawMouse        :   0x82c;
extern  Point	Mouse		:0x830;
extern  int	CrsrNewCouple   :   0x8ce;  /* both New & Couple */
extern  Byte	CrsrNew         :   0x8ce;
extern  Byte	CrsrCouple      :   0x8cf;

#define     Couple      0xff;   /* value for CrsrCouple */
#define     Uncouple    0x00;   /* value for CrsrCouple */

void    SetMouse(Point   where)	//move mouse on-screen, in local coordinates
{
long    finaltick;
	
	HideCursor();
	LocalToGlobal(&where);          /* Get ready to store mouse position */
	RawMouse = where;               /* into RawMouse */
	MTemp = where;                  /* and MTemp */
	Mouse = where;                  /* and Mouse */
	CrsrNewCouple = 0xFFFF;       /* Hit CrsrNew & CrsrCouple */
	ShowCursor();
}   /* SetMouse */

void	SetWindColors(WindowPtr wind, Boolean NotInvertColors)
//if a color window, on a color mac, and screen depth is greater than 2,
//set the fore and back pen colors to the ones in the default WCTB window color table resource
//or the window's color table, if it has one.
//if NotInverColors is false, it sets the colors inverted.
{
AuxWinHandle	wincolors;
Boolean		UseColorQD = false;

	if (HasColorQD)
	{
		if ( ((CWindowPtr) wind)->portVersion & 0xC000)	//if a color grafport
		{
			if ((*( ((CWindowPtr) wind)->portPixMap))->pixelSize > 2) UseColorQD = true;
		}
	}
	if (UseColorQD)
	{
		GetAuxWin(wind, &wincolors);
		if (NotInvertColors)
		{
			RGBForeColor(& ( (*((*wincolors)->awCTable))->ctTable[wTextColor].rgb) );
			RGBBackColor(& ( (*((*wincolors)->awCTable))->ctTable[wContentColor].rgb) );
		}
		else
		{
			RGBBackColor(& ( (*((*wincolors)->awCTable))->ctTable[wTextColor].rgb) );
			RGBForeColor(& ( (*((*wincolors)->awCTable))->ctTable[wContentColor].rgb) );
		}
	}
	else
	{
		if (NotInvertColors)
		{
			ForeColor(blackColor);
			BackColor(whiteColor);
		}
		else
		{
			ForeColor(whiteColor);
			BackColor(blackColor);
		}	
	}
}

void	BlackNWhite(Boolean NotInvertColors)
//set forecolor and backcolor to black'n'white
//if NotInvertColors is false, sets to white'n'black instead
{
	if (NotInvertColors)
	{
		ForeColor(blackColor);
		BackColor(whiteColor);
	}
	else
	{
		ForeColor(whiteColor);
		BackColor(blackColor);
	}
}

void		ColorImplemented(void)
//call at program startup, to update the public variable HasColorQD
//then if a routine needs to know if the Mac has Color QD, it can check HasColorQD.
{
SysEnvRec	theWorld;

	HasColorQD = false; //Init to no color QuickDraw
	SysEnvirons(1, &theWorld);	//Check how old this system is
	if (theWorld.machineType >= 0)
	{ 	//Negative means really old
		HasColorQD = theWorld.hasColorQD; 	//Flag for Color QuickDraw being available
	}
}
