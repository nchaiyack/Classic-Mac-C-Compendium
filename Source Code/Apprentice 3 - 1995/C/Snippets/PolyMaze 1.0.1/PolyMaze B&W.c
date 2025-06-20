// PolyMaze B&W 1.0.1
// ported to CodeWarrior by Ken Long (kenlong@netcom.com)
// updated for CodeWarrior 6 on 950712

//�  ---------- "Source to 3D maze display" ---------- �//

//� Written Jul 16,  1985 by Steve Hawley (sdh@joevax.UUCP)

//� This is the source code for the maze display program. 
//� It was done in Aztec C version 1.06D. 

//� Revived and made to run on Think C and Metrowerks C by
//� Kenneth A. Long, at "itty bitty bytes(tm)".

WindowRecord	wRecord;
WindowPtr	myWindow;
EventRecord myEvent;

//� the maze. 1's = blocks,  0's = space.
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
int		Square_3_D(int col, int row);
int		Draw_Maze(void);
int		main(void);

//� Displays a given block from the maze. Decides on which side of 
//� center the block is found,  and draws the visible faces,  which 
//� are defined as polygons. The faces are projected using the
//� formulae: 
//� 		  'x' = x / z; 
//� 		  'y' = y / z; 		
//� to achieve one point perspective.

Square_3_D (int col,  int row)
{
	int x1, x2, y1, y2, z1, z2;
	PolyHandle left_side,  right_side,  top_side,  front_side;
	
	x1 = (col - 8) * 100;		//� 100 is width of side.
	x2 = x1 + 100;
	z1 = (23 - row);
	z2 = z1 -1;
	y1 = 150;
	y2 = 250;
	
	left_side = OpenPoly ();
	MoveTo ((x1 / z1) + 240,  (y1 / z1) + 60);
	LineTo ((x1 / z1) + 240,  (y2 / z1) + 60);
	LineTo ((x1 / z2) + 240,  (y2 / z2) + 60);
	LineTo ((x1 / z2) + 240,  (y1 / z2) + 60);
	LineTo ((x1 / z1) + 240,  (y1 / z1) + 60);
	ClosePoly ();
	
	right_side = OpenPoly ();
	MoveTo ((x2 / z1) + 240,  (y1 / z1) + 60);
	LineTo ((x2 / z1) + 240,  (y2 / z1) + 60);
	LineTo ((x2 / z2) + 240,  (y2 / z2) + 60);
	LineTo ((x2 / z2) + 240,  (y1 / z2) + 60);
	LineTo ((x2 / z1) + 240,  (y1 / z1) + 60);
	ClosePoly ();
	
	top_side = OpenPoly ();
	MoveTo ((x1 / z1) + 240,  (y1 / z1) + 60);
	LineTo ((x2 / z1) + 240,  (y1 / z1) + 60);
	LineTo ((x2 / z2) + 240,  (y1 / z2) + 60);
	LineTo ((x1 / z2) + 240,  (y1 / z2) + 60);
	LineTo ((x1 / z1) + 240,  (y1 / z1) + 60);
	ClosePoly ();
	
	front_side = OpenPoly ();
	MoveTo ((x1 / z2) + 240,  (y1 / z2) + 60);
	LineTo ((x2 / z2) + 240,  (y1 / z2) + 60);
	LineTo ((x2 / z2) + 240,  (y2 / z2) + 60);
	LineTo ((x1 / z2) + 240,  (y2 / z2) + 60);
	LineTo ((x1 / z2) + 240,  (y1 / z2) + 60);
	ClosePoly ();
	
	//� Decide to draw right side (dark gray).
	if (x2 < 0 && maze [row] [col + 1] != 1)
	{
		PenPat (&qd.dkGray);
		ErasePoly (right_side);
		PaintPoly (right_side);
		PenPat (&qd.black);
		FramePoly (right_side);
	}
	
	//� Decide to draw left face (light gray).
	if (x1 > 0 && maze [row] [col - 1] != 1) 
	{
		PenPat (&qd.ltGray);
		ErasePoly (left_side);
		PaintPoly (left_side);
		PenPat (&qd.black);
		FramePoly (left_side);
	}

	//� Draw the top_side (white).
	PenPat (&qd.white);
	ErasePoly (top_side);
	PaintPoly (top_side);
	PenPat (&qd.black);
	FramePoly (top_side);
	
	//� Draw the front_side (gray).
	PenPat (&qd.gray);
	ErasePoly (front_side);
	PaintPoly (front_side);
	PenPat (&qd.black);
	FramePoly (front_side);
	
	//� Restore pen characteristics.
	PenNormal (); 
	
	//� Dispose of all the polygons to free up memory.
	KillPoly (top_side);		
	KillPoly (front_side);
	KillPoly (left_side);
	KillPoly (right_side);
}

//� Draws out all the blocks,  starting from the back,  moving left 
//� to center,  then right to center.

Draw_Maze ()
{
	Rect trect;
	int row,  col;
	
	for (row = 0; row < 22; row++)
	{
		for (col = 0; col < 8; col++)
			if (maze [row] [col] == 1) 
				Square_3_D (col, row);
		for (col = 16; col > 7; col--)
			if (maze [row] [col] == 1) 
				Square_3_D (col,  row);
	}
}

main ()
{
	Rect screen;
	
	InitGraf (&qd.thePort);	/* do inits */
	InitFonts ();
	FlushEvents (everyEvent, 0);
	InitWindows ();
	SetCursor (&qd.arrow);
	ShowCursor ();
	
	//� Set up the window.  Give it some bounds.
	SetRect (&screen,  4,  40,  508,  370);
	
	//� Then create it from scratch.
	myWindow = NewWindow (&wRecord,  &screen,  "\pSomething Amazing", 1, 0,  (WindowPtr)-1L, 1, 0L);

	SetPort (myWindow);				//� Say we'll draw there.
	ShowWindow (myWindow);			//� Then show it.
	
	Draw_Maze ();						//� Draw the maze in the window.
	
	//� Wait for button to be pressed or handle events 
	//� like Clover-Shift-4, etc.
	while (!Button ()) 
		GetNextEvent (everyEvent,  &myEvent);
}
/* End of text from uiucdcs:net.sources.mac */
