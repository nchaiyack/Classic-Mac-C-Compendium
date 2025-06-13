// Polygondrian 1.0.1
// by Ken Long (kenlong@netcom.com)
// updated for CodeWarrior 6 on 950712

//¥ This is a little color salad tossed together by Kenneth A. Long (me).
//¥ I wanted to do some polygon action and this seemed like a good way to
//¥ do it.  Random poly's sure do be fast!  Huh?	:-)

//¥ Enjoy!

//¥ kenlong@netcom.com		10 March 1994.
//¥-----------------------------------------------------------------------¥//

//¥  Polygondrian.c ¥//

//¥ Constance ¥//
//¥ Ain't got none.

//¥ Globules ¥//
Rect			windRect;
OSErr			error;
SysEnvRec		theWorld;
CGrafPtr		savePort;
Rect			r;
PolyHandle		poly;
Pattern			p;
WindowPtr		window;
short			horiz, vert;
Boolean			QDAvail;
RGBColor 		rgb;
Boolean			hasColorQD;

RgnHandle		mBarRgn, GrayRgn;
short			*mBarHeightPtr;
short			twenty_pixels;

//¥ Prototypes ¥//
void			Random_Poly_Points (Rect *rectPtr);
unsigned short	The_Ramdominator (unsigned short min, unsigned short max);
void			Draw_The_Poly (void);
void			Do_Init_Managers (void);
void			Hide_Menu_Bar (void);
void			Show_Menu_Bar (void);
Boolean			Check_The_Equipment (void);
void			Set_Up_Window (void);
void			main (void);
int				Main_Event_Loop (void);

void Random_Poly_Points (Rect *rectPtr)
{
	unsigned short start_H, start_V;

	window = FrontWindow ();
	
	//¥ Start recording a polygon.
	poly = OpenPoly ();
	
	//¥ Pick a random hoizontal and vertical point for our starting point.
	start_H = The_Ramdominator (0, qd.screenBits.bounds.right);
	start_V = The_Ramdominator (0, qd.screenBits.bounds.bottom);
	
	//¥ Move the pen to that location.
	MoveTo (start_H, start_V);

	//¥ Set up 5 random poly bounds lines starting from there.
	LineTo (The_Ramdominator (0, qd.screenBits.bounds.right), 
			The_Ramdominator (0, qd.screenBits.bounds.bottom));
			
	LineTo (The_Ramdominator (0, qd.screenBits.bounds.right), 
			The_Ramdominator (0, qd.screenBits.bounds.bottom));
			
	LineTo (The_Ramdominator (0, qd.screenBits.bounds.right), 
			The_Ramdominator (0, qd.screenBits.bounds.bottom));
			
	LineTo (The_Ramdominator (0, qd.screenBits.bounds.right), 
			The_Ramdominator (0, qd.screenBits.bounds.bottom));
			
	LineTo (The_Ramdominator (0, qd.screenBits.bounds.right), 
			The_Ramdominator (0, qd.screenBits.bounds.bottom));
	
	//¥ You could have as many successive LineTo's here as you wanted,
	//¥ within reason.  Also, you could put specific values in, or even get
	//¥ a control value.  You could get a previous value and add to 
	//¥ them or subtract from them for a series effect (like ZoomIdle).
	
	//¥ Finish the polygon by drawing a line back to the starting point.
	LineTo (start_H, start_V);
			
	//¥ Stop recording.
	ClosePoly ();
}

unsigned short The_Ramdominator (unsigned short min, unsigned short max)
{
	unsigned long ranger, tonto;	//¥ I knew that one would throw ya!
	unsigned short el_randomo;		//¥ No sign means pos. and neg. values.
	
	//¥ Get a random number (based on the date and the time at that call).
	el_randomo = Random ();
	
	//¥ Just to be sure...if it's less than 0 (which it shouldn't be)
	//¥ then multiply by -1 to make it positive.
	if (el_randomo < 0)
		el_randomo *= -1;
	
	//¥ The next three lines are a formula for generating a random
	//¥ number within a specific range.  Don't worry about the 
	//¥ mathematics, just know where you can look such a formula up 
	//¥ if you ever need it.
	
	//¥ The range IS the maximum value minus the minimum value.
	ranger = max - min;
	
	//¥ These two bytes hold the random number, muliplied by the range
	//¥ then that product divided by 65,535.
	tonto = (el_randomo * ranger) / 65535;
	
	//¥ Then, that product is added to minimum value and returned to caller.
	return (tonto + min);   
}

//¥ Set the forecolor with a single line.
SetForeColor (short red, short green,short  blue)
{
	RGBColor theColor;

	theColor.red = red;
	theColor.green = green;
	theColor.blue = blue;
	RGBForeColor(&theColor);
}

//¥ Initialize everything for the program, make sure we can run.
void Draw_The_Poly (void)
{
	Rect 		poly_wrecked;
	RGBColor	poly_color;
	
	poly_color.red   = Random ();		//¥ So are there 281,474,976,710,656
	poly_color.green = Random ();		//¥ possible combinations?
	poly_color.blue  = Random ();		//¥ Try 2 /*814749767106*/ 56 :)
	
	Random_Poly_Points (&poly_wrecked);	//¥ Figure the poly points.
	
	RGBForeColor (&poly_color);			//¥ Snag a random color for the RGB.
	
	//¥ Color paint it.
	PaintPoly (poly);

	//¥ Say what color the line is.
	SetForeColor (0, 0, 0);
	
	//¥ Suitable for framing!
	FramePoly (poly);			//¥ They're uglier with no frame!
	
	//¥ Bury it.
	KillPoly (poly);

}	//¥ Now, "doitallagainreallyreallyfast!!!" (while not button, remember?).

void Do_Init_Managers (void)
{
	MaxApplZone ();

	InitGraf (&qd.thePort);
	InitFonts ();
	InitWindows ();
	InitMenus ();
	TEInit ();
	InitDialogs (nil);
	InitCursor ();
}

void Hide_Menu_Bar (void) 
{
	Rect	mBarRect;

	GrayRgn = GetGrayRgn ();
	mBarHeightPtr = (short *)  0x0BAA;
	twenty_pixels = *mBarHeightPtr;
	*mBarHeightPtr = 0;
	mBarRect = qd.screenBits.bounds;
	mBarRect.bottom = mBarRect.top + twenty_pixels;
	mBarRgn = NewRgn ();
	RectRgn (mBarRgn, &mBarRect);
	UnionRgn (GrayRgn, mBarRgn, GrayRgn);
	PaintOne (0L, mBarRgn);
}

void Show_Menu_Bar (void) 
{
	*mBarHeightPtr = twenty_pixels;
	DiffRgn (GrayRgn, mBarRgn, GrayRgn);
	DisposeRgn (mBarRgn);
}

Boolean Check_The_Equipment ()
{
	SysEnvRec mySE;
	
	SysEnvirons (2,&mySE);
	
	if (!mySE.hasColorQD)
		ExitToShell ();
}

void Set_Up_Window (void)
{
	windRect = qd.screenBits.bounds;
	
	window = NewCWindow (0L, &windRect, "\p", true, plainDBox, (WindowPtr)-1L, false, 0);
	if (window == nil)
	{
		SysBeep (10);
		ExitToShell ();
	}
	//¥ Say where we'll draw.  Set window to current graf port.
	SetPort (window);

	//¥ Background color.
	FillRect (& (window->portRect), &qd.black);	//¥ We already know it's portRect!
}

void main (void)
{
	long ticks;

	Do_Init_Managers ();

	Check_The_Equipment ();
	Hide_Menu_Bar ();
	Set_Up_Window ();

	//¥ The seed for the random number generation.
	GetDateTime ( (unsigned long*) &qd.randSeed);

	HideCursor ();				//¥ Don't want that 'fly' buzzin' around.

	while (! Button ())			//¥ While we ain't clickin...
	{
		Draw_The_Poly ();		//¥ draw to the tickin'.
//¥		Delay (60L, &ticks);	//¥ Put the brakes on, a little...
								//¥ (uncomment for slow).
//¥ 	FillRect (& (window->portRect), black);
								//¥ (uncomment for single).
	}
		ShowCursor ();
	Show_Menu_Bar ();
	ExitToShell ();
}
