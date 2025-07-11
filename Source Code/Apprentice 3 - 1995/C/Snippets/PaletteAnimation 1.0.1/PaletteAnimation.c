// PaletteAnimation 1.0.1
// by Ken Long (kenlong@netcom.com)
// updated for CodeWarrior 6 on 960712

# include <Palettes.h>


# define BASE_RES_ID		400
# define NIL_POINTER		0L
# define MOVE_TO_FRONT		-1L
# define CLUT_ID			130
# define PALETTE_ID			1288
# define SRC_USAGE			2
# define SRC_TOLERANCE		0
# define REMOVE_ALL_EVENTS	0


WindowPtr		gHelloWindow;
RGBColor		myRGB;	
Point			*mouseLoc;
int				x,y,c;
PaletteHandle	gThePalette;
CTabHandle		gTheCLUT;



/******************** Main ********************/
main()
{
	ToolBoxInit();
	WindowInit();
}



/******************** ToolBoxInit ********************/
ToolBoxInit()

{
	InitGraf ( &qd.thePort );
	InitFonts();
	FlushEvents ( everyEvent,  0 );
	InitWindows();
	InitMenus();
	TEInit();
	InitDialogs( 0L );
	InitCursor();
}


/******************** WindowInit ********************/
WindowInit ()
{	

	gHelloWindow = GetNewCWindow ( BASE_RES_ID, 0L, (WindowPtr)-1L );
	SetPort ( gHelloWindow );
	gTheCLUT = GetCTable ( CLUT_ID );
	gThePalette = GetNewPalette ( PALETTE_ID );
	CTab2Palette ( gTheCLUT, gThePalette, 2, 0);
	SetPalette ( gHelloWindow, gThePalette, TRUE );
	ActivatePalette ( gHelloWindow );
	while (!Button())
	{
		y = 500;
		for (c = 1; c < 255; c++ )
		{
			x++;
			MoveTo ( x, y );
			GetEntryColor ( gThePalette, c, &myRGB );
			RGBForeColor ( &myRGB );
			GetMouse ( mouseLoc );
			LineTo ( x, 0 );
		}	
	}
}