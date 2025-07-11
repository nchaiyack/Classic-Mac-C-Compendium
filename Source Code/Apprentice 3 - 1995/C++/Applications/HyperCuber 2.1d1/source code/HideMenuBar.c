//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| HideMenuBar.c
//|
//| This contains a collection of routines which hide or display the menu bar.
//| This code was adapted from Pascal code by Earle R. Horton.  The original
//| Pascal source contained the following paragraph:
//|
//|   This file is part of Earle R. Horton's private source code library.
//|   Earle R. Horton assumes no responsibility for any damages arising
//|   out of use of this source code for any purpose.  Earle R. Horton
//|   places no restrictions on use of all or any part of this source code,
//|   except that this paragraph may not be altered or removed.
//|____________________________________________________________________________



//=============================== Globals ===============================\\

RgnHandle	save_region;
short		menubar_height;
Rect		menubar_rect;
Boolean		menubar_hidden;

//=============================== Prototypes ===============================\\

void init_menubar(void);
void show_menubar(void);
void hide_menubar(void);
void set_menubar_height(short new_height);


//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| Procedure set_menubar_height
//|
//| Purpose: Set the height of the menubar.  This modifies a low-memory global
//|
//| Parameters: new_height: desired height of the menubar
//|____________________________________________________________________________

void set_menubar_height(short new_height)
{

asm	{
	
	move.w	new_height, 0x0BAA		//  Update menubarheight global
	
	}	

}	//==== set_menubar_height() ====\\



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| Procedure init_menubar
//|
//| Purpose: Initialize globals in preparation for an eventual call to hide_menubar
//|
//| Parameters: none
//|_________________________________________________________________________________

void init_menubar(void)
{

	menubar_hidden = FALSE;				//  Menubar starts visible
	menubar_height = GetMBarHeight();	//  Get the height of the menu bar
	SetRect(&menubar_rect,				//  Compute menu bar's rectangle
				screenBits.bounds.left,
				screenBits.bounds.top,
				screenBits.bounds.right,
				screenBits.bounds.top + menubar_height);

}	//==== init_menubar() ====\\



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| Procedure hide_menubar
//|
//| Purpose: Hide the menu bar
//|
//| Parameters: none
//|_________________________________________________________________________________

void hide_menubar(void)
{

	RgnHandle	menubar_region;
	WindowPeek	window;

	if (!menubar_hidden)
		{
		
		save_region = NewRgn();					//  Initialize Region where we'll save GrayRgn
		
		menubar_region = NewRgn();				//  Initialize menu bar region
		
		set_menubar_height(0);					//  Set menubar to zero height
		
		CopyRgn(GetGrayRgn(), save_region);		//  Get a copy of GrayRgn
		
		RectRgn(menubar_region, &menubar_rect);	//  Set GrayRgn to original GrayRgn plus menu bar
		UnionRgn(GetGrayRgn(), menubar_region,
					GetGrayRgn());
		
		window = (WindowPeek) FrontWindow();	//  Fix any windows which were behind menubar
		PaintOne(window, menubar_region);
		CalcVis(window);
		CalcVisBehind(window, menubar_region);
		
		DisposeRgn(menubar_region);				//  Get rid of menubar region
		
		menubar_hidden = TRUE;					//  Menubar is now hidden
		
		}

}	//==== hide_menubar() ====\\



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| Procedure show_menubar
//|
//| Purpose: Show the menu bar
//|
//| Parameters: none
//|_________________________________________________________________________________

void show_menubar()
{

	WindowPeek	window;
	
	if (menubar_hidden)
		{
		menubar_hidden = FALSE;					//  Menu bar is no longer hidden
		
		CopyRgn(save_region, GetGrayRgn());		//  Restore old GrayRgn
		
		set_menubar_height(menubar_height);		//  Restore menubar height
		
		RectRgn(save_region, &menubar_rect);	//  Update any covered windows
		window = (WindowPeek) FrontWindow();
		CalcVis(window);
		CalcVisBehind(window, save_region);
		
		DisposeRgn(save_region);				//  Get rid of the region used to save GrayRgn
		
		HiliteMenu(0);							//  Draw the menubar
		DrawMenuBar();
		}

}	//==== show_menubar() ====\\



