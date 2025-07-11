// PolyMaze Color 1.0.1
// ported to CodeWarrior by Ken Long (kenlong@netcom.com)
// updated for CodeWarrior 6 on 950712

//�  ---------- "Source to 3D maze display" ---------- �//

//� B/W version written Jul 16, 1985 by Steve Hawley (sdh@joevax.UUCP)
//� This is the source code for the maze display program. 
//� It was done in Aztec C version 1.06D. 

//� Colorization by Kenneth A. Long, at "itty bitty bytes(tm)"
//� Made to run in Think C and Metrowerks C.


#include <stdio.h>
#include <GestaltEqu.h>

#define kSleep				0L

#define mDevice				131

#define kNULLFilterProc		NULL
Boolean		gDone;

WindowRecord	wRecord;
WindowPtr	myWindow;
EventRecord myEvent;

//� the maze. 1's = blocks, 0's = space.
static char maze [22] [17] = { 
	{1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1}, 
	{1, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 1}, 
	{1, 0, 1, 1, 1, 0, 1, 1, 0, 1, 0, 1, 1, 0, 1, 1, 1}, 
	{1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 1}, 
	{1, 1, 0, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 1, 1, 0, 1}, 
	{1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1}, 
	{1, 0, 1, 1, 0, 1, 0, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1}, 
	{1, 0, 0, 1, 0, 1, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 1}, 
	{1, 1, 0, 0, 0, 1, 1, 1, 0, 1, 0, 1, 1, 1, 0, 1, 1}, 
	{1, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 1}, 
	{1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 0, 1, 1, 1, 1}, 
	{1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 1, 0, 0, 1}, 
	{1, 0, 1, 1, 1, 1, 1, 1, 0, 1, 0, 1, 0, 0, 0, 1, 1}, 
	{1, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 1}, 
	{1, 0, 1, 1, 0, 1, 1, 1, 0, 1, 0, 1, 1, 1, 1, 0, 1}, 
	{1, 0, 0, 1, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 1}, 
	{1, 0, 1, 1, 0, 1, 0, 1, 0, 1, 1, 1, 1, 1, 1, 0, 1}, 
	{1, 0, 1, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1}, 
	{1, 0, 1, 1, 0, 1, 0, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1}, 
	{1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 1}, 
	{1, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1}, 
	{1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}
};		

//� Prototypes.

SetForeColor (short red, short green,short  blue);
void		Maze_Window_Set(GDHandle device);
Boolean		HasColorQD(void);
GDHandle	How_Deep(void);
short		Get_The_Depth(GDHandle device);
void		Scramble_Color(RGBColor *colorPtr);
int			Square_3_D(int column, int row);
int			Draw_Maze(void);
void		main(void);

//� Set the RGB forecolor with a single line.
SetForeColor (short red, short green,short  blue)
{
	RGBColor theColor;

	theColor.red = red;
	theColor.green = green;
	theColor.blue = blue;
	RGBForeColor(&theColor);
}

//� Set the RGB backcolor with a single line.
SetBackColor (short red, short green,short  blue)
{
	RGBColor theColor;

	theColor.red = red;
	theColor.green = green;
	theColor.blue = blue;
	RGBBackColor(&theColor);
}

void Maze_Window_Set (GDHandle device)
{
	WindowPtr	amazing;
	Rect		globalRect, localRect;
	
	//� Set up the window.  Give it some bounds.
	SetRect (&globalRect, 4, 40, 508, 370);
	
	amazing = NewCWindow (0L, &globalRect, "\pSomething Amazing", true, 0,
							 (WindowPtr)-1L, false, 60L);
	ShowWindow (amazing);
	SetPort (amazing);

	//� Make a draw rect.  Since we set the port th the window, 
	//� topLeft is now 0, 0 of the window (local).
	//� This rect has the sole purpose of installing a background color.
	//� Since we die on mouseDown, this is only done here.  No need to
	//� insert preservation or update provisions.
	SetRect (&localRect, 0, 0, 504, 330);
	
	//� So we set a back color.  Mac default is already set at white.
	SetBackColor (0xdddd, 0xffff, 0xffff);
	
	//� Since it's white, we must erase it to reveal our RGBBackColor.
	EraseRect (&localRect);
}

Boolean	HasColorQD (void)
{
	unsigned char	version[ 4 ];
	OSErr			err;
	
	err = Gestalt (gestaltQuickdrawVersion, (long *)version);
	
	if (err != noErr)
	{
		SysBeep (10);	//� Error.
		ExitToShell ();
	}
	
	if (version[ 2 ] > 0)
		return (true);
	else
		return (false);
}

GDHandle How_Deep (void)
{
	GDHandle	curDevice, maxDevice = NULL;
	short		curDepth, maxDepth = 0;
	
	curDevice = GetDeviceList ();
	
	while (curDevice != NULL)
	{
		curDepth = Get_The_Depth (curDevice);
		
		if (curDepth > maxDepth)
		{
			maxDepth = curDepth;
			maxDevice = curDevice;
		}

		curDevice = GetNextDevice (curDevice);
	}
	
	return (maxDevice);
}

short Get_The_Depth (GDHandle device)
{
	PixMapHandle	screenPixMapH;
	
	screenPixMapH = (**device).gdPMap;
	
	return ((**screenPixMapH).pixelSize);
}

//� This scramble will give a random RGB for each poly that calls it.
//� I set specifics instead, but left this here for data purposes.
void Scramble_Color (RGBColor *colorPtr)
{
	colorPtr->red =  Random () + 32767;
	colorPtr->blue = Random () + 32767;
	colorPtr->green = Random () + 32767;
}

//� Displays a given block from the maze. Decides on which side of 
//� center the block is found, and draws the visible faces, which 
//� are defined as polygons. The faces are projected using the
//� formulae: 
//� 		  'x' = x / z; 
//� 		  'y' = y / z; 		
//� to achieve one point perspective.

Square_3_D (int column, int row)
{
	RGBColor color, myColor;
	int x1, x2, y1, y2, z1, z2;
	PolyHandle left_side, right_side, top_side, front_side;
	
	x1 = (column - 8) * 100;		//� 100 is width of side.
	x2 = x1 + 100;
	z1 = (23 - row);
	z2 = z1 -1;
	y1 = 150;
	y2 = 250;
	
	left_side = OpenPoly ();
	MoveTo ((x1 / z1) + 256, (y1 / z1) + 60);
	LineTo ((x1 / z1) + 256, (y2 / z1) + 60);
	LineTo ((x1 / z2) + 256, (y2 / z2) + 60);
	LineTo ((x1 / z2) + 256, (y1 / z2) + 60);
	LineTo ((x1 / z1) + 256, (y1 / z1) + 60);
	ClosePoly ();
	
	right_side = OpenPoly ();
	MoveTo ((x2 / z1) + 256, (y1 / z1) + 60);
	LineTo ((x2 / z1) + 256, (y2 / z1) + 60);
	LineTo ((x2 / z2) + 256, (y2 / z2) + 60);
	LineTo ((x2 / z2) + 256, (y1 / z2) + 60);
	LineTo ((x2 / z1) + 256, (y1 / z1) + 60);
	ClosePoly ();
	
	top_side = OpenPoly ();
	MoveTo ((x1 / z1) + 256, (y1 / z1) + 60);
	LineTo ((x2 / z1) + 256, (y1 / z1) + 60);
	LineTo ((x2 / z2) + 256, (y1 / z2) + 60);
	LineTo ((x1 / z2) + 256, (y1 / z2) + 60);
	LineTo ((x1 / z1) + 256, (y1 / z1) + 60);
	ClosePoly ();
	
	front_side = OpenPoly ();
	MoveTo ((x1 / z2) + 256, (y1 / z2) + 60);
	LineTo ((x2 / z2) + 256, (y1 / z2) + 60);
	LineTo ((x2 / z2) + 256, (y2 / z2) + 60);
	LineTo ((x1 / z2) + 256, (y2 / z2) + 60);
	LineTo ((x1 / z2) + 256, (y1 / z2) + 60);
	ClosePoly ();
	
	//� Decide to draw right side (dark gray).
	if (x2 < 0 && maze [row] [column + 1] != 1)
	{
//		Scramble_Color (&color);
//		RGBForeColor (&color);

		//� Uncomment the above two lines 
		//� and comment out the below line,
		//� and do the same for similar sets below for UGLY random.

		//� 4444 is 5/16 of the range.
		SetForeColor (0x4444, 0x4444, 0x4444);
		ErasePoly (right_side);
		PaintPoly (right_side);
		SetForeColor (0, 0, 0);		//� Our line is "off" or black.
		FramePoly (right_side);
	}
	
	//� Decide to draw left face (light gray).
	if (x1 > 0 && maze [row] [column - 1] != 1) 
	{
//		Scramble_Color (&color);
//		RGBForeColor (&color);
		SetForeColor (0xbbbb, 0xbbbb, 0xbbbb);	//� 11/16 toward full on.
		ErasePoly (left_side);
		PaintPoly (left_side);
		
		SetForeColor (0, 0, 0);
		FramePoly (left_side);
	}

	//� Draw the top_side (white).
//	Scramble_Color (&color);
//	RGBForeColor (&color);
	SetForeColor (0xeeee, 0xeeee, 0xeeee);	//� 15/16 full on.
	ErasePoly (top_side);
	PaintPoly (top_side);
	SetForeColor (0, 0, 0);
	FramePoly (top_side);
	
	//� Draw the front_side (gray).
//	Scramble_Color (&color);
//	RGBForeColor (&color);
	SetForeColor (0x7777, 0x7777, 0x7777);	//� 7/16 full on.
	ErasePoly (front_side);
	PaintPoly (front_side);
	
	SetForeColor (0, 0, 0);
	FramePoly (front_side);
	
	//� Restore pen characteristics.
	PenNormal (); 
	
	//� Dispose of all the polygons to free up memory.
	KillPoly (top_side);		
	KillPoly (front_side);
	KillPoly (left_side);
	KillPoly (right_side);
}

//� Draws out all the blocks, starting from the back, moving left 
//� to center, then right to center.

Draw_Maze ()
{
	Rect trect;
	int row, column;
	
	for (row = 0; row < 22; row++)
	{
		for (column = 0; column < 8; column++)
			if (maze [row] [column] == 1) 
				Square_3_D (column, row);
		for (column = 16; column > 7; column--)
			if (maze [row] [column] == 1) 
				Square_3_D (column, row);
	}
}

void	main (void)
{
	InitGraf (&qd.thePort);
	InitFonts ();
	InitWindows ();
	InitMenus ();
	TEInit ();
	InitDialogs (nil);
	InitCursor ();

	//� No color?  Then you don't get to play!
	if (! HasColorQD ())
		ExitToShell ();
			
	//� HowDeep is not really needed, since all our colors are set
	//� at divisions of 16.  But in case you need it, it was left in.
	Maze_Window_Set (How_Deep ());	
	
	Draw_Maze ();
	
	while (!Button ()) 	//� Click and exit.
		GetNextEvent (everyEvent, &myEvent);
}
