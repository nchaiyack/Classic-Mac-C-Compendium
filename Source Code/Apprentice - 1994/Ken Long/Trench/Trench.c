//¥ ----------------------------------------------------------------------¥//
//¥ Updated to run in the Think Cª environment by Kenneth A. Long on
//¥ 12 March 1992.
//¥ Add MacTraps and ANSI.
//¥ ----------------------------------------------------------------------¥//

//¥ -------------------- Source to "Trench Simulator" ------------------- ¥//
//¥ Written 12:00 pm  Jul 16, 1985 by 
//¥ sdh@joevax.UUCP (Steve Hawley)
//¥ Posted in uiucdcs:net.sources.mac.
//¥ This is the source to the trench simulator. It was written in MegaMax C.
//¥ Note the tabs should be set to 4 spaces.

//¥ Program to simulate the trench from Star Wars.
//¥ Written 12 July 85 by Steve Hawley in MegaMax C.

//#include <qd.h>		//¥ QuickDraw header file.
//#include <qd.h>		//¥ Quickdraw variables.
//#include <Event.h>  	//¥ Event manager header.
//#include <win.h>		//¥ Window manager header.

WindowRecord wRecord; 	//¥ Record for window.
WindowPtr myWindow;		//¥ Pointer to Record.
Rect screenRect;		//¥ Rectangle representing window size.

int wlines = 0;			//¥ phase of depth lines.

//¥ ----------------------------------------------------------------------¥//

//¥ ----------------------------------------------------------------------¥//
//¥ draws lines to give illusion of depth.
//¥ ----------------------------------------------------------------------¥//

Draw_Lines (int offx, int offy, int start) 
{
	int x1 = -200, y1 = -100;
	int z, index;

	//¥ Start is the phase (from 0 to 3)>
	//¥ The lines are projected by the formulae:

	//¥ 	x' = x / z; 
	//¥ 	y' = y / z;

	//¥ offx and offy are offsets for viewPoint changes.
	
	z = 10000;		//¥ KAL added this to keep from dividing by zero.
					//¥ in the built app.  Looks and acts the same.
					
	for (index = 50 - start; index > 0; index -= 4)
	{
		MoveTo (( x1 - offx) / z, 
				( y1 - offy) / z);
		LineTo (( x1 - offx) / z, 
				(-y1 - offy) / z);
				
		MoveTo ((-x1 - offx) / z, 
				( y1 - offy) / z);
		LineTo ((-x1 - offx) / z, 
				(-y1 - offy) / z);
	}
}

//¥ ----------------------------------------------------------------------¥//
//¥ Draws the frame of the trench.
//¥ offx and offy again represent the viewPoint offsets, and it is.
//¥ Projected using the same formulae as before.
//¥ ----------------------------------------------------------------------¥//

SetUp (short offx, short offy) 
{

	int x1 = -200, x2 = -100, y1 = -100;
	
	
	//¥ The 50 sets the far end of the lines.  Over 50 makes no noticable
	//¥ changes.  Less than 50 brings the ends closer.  Try 4.
	
	MoveTo 	(  x1 - offx,              
			   y1 - offy);
			   
	LineTo ((  x1 - offx)	/ 50,     
			(  y1 - offy)	/ 50);
			
	LineTo ((  x1 - offx)	/ 50,     
			(- y1 - offy)   / 50);
			
	LineTo 	(  x1 - offx, -            
			   y1 - offy);
	
	MoveTo (   x2 - offx,	- 
			   y1 - offy);
			   
	LineTo ((  x2 - offx)	/ 50,
			(- y1 - offy)   / 50);
	
	MoveTo 	(- x1 - offx,              
			   y1 - offy);
			   
	LineTo ((- x1 - offx)	/ 50,     
			(  y1 - offy)   / 50);
			
	LineTo ((- x1 - offx)	/ 50,     
			(- y1 - offy)   / 50);
	LineTo  (- x1 - offx,	- 		   
			   y1 - offy);
	
	MoveTo 	(- x2 - offx, -
			   y1 - offy);
			   
	LineTo ((- x2 - offx)	/ 50,     
			(- y1 - offy)   / 50);
}

//¥ ----------------------------------------------------------------------¥//
//¥ The objects are animated by using exclusive-or drawing. 
 
//¥ This way, the same routine to draw can be used to erase, and the new 
//¥ image can be drawn before the old image is erased to help eliminate 
//¥ flicker.  

//¥ Thus the the program needs two copies of the offset parameters, one
//¥ for the new and one for the old.
//¥ ----------------------------------------------------------------------¥//

main ()
{
	int offxo = 0, offxn = 0, offyo = 0, offyn = 0;
	Point mouse;
	
	InitGraf (&thePort);			//¥ Set up quickdraw.
	FlushEvents (everyEvent, 0); 	//¥ Kill previous Events.
	InitWindows ();					//¥ Initialize window manager.
	
	InitCursor ();					//¥ Ken Long added to get rid of
									//¥ default text edit cursor.
	//¥ Set window size.
	SetRect (&screenRect, 4, 40, 508, 338); 
	
	//¥ Draw one from scratch.
	myWindow = NewWindow (&wRecord, &screenRect, "\pTrench",1,0, (WindowPtr)-1L, 1, 0L);
	
	//¥ Set window parameters.
	SetPort (myWindow); 			//¥ Make the window the current grafport.
	ShowWindow (myWindow); 			//¥ Display window.

	//¥ Set the origin so the center of the screen in (0,0).
	SetOrigin (-252, -149);
	PenMode (patXor); 				//¥ Set exclusive-or drawing.
	
	SetUp (offxn, offyn); 			//¥ Draw Initial SetUp.
	
	Draw_Lines (offxn, offyn, wlines); //¥ Draw Initial depth lines.
	
	while (!Button ()) 				//¥ Repeat until Button is clicked.
	{
		offxo = offxn;   			//¥ Put new offsets in old variables.
		offyo = offyn;
		GetMouse (&mouse); 			//¥ get the mouses coordinates.
		
		//¥ We divide the mouse location by two so we don't go
		//¥ trough the walls or floor of the trench.
		if (mouse.h /2 < offxn)  	//¥ If the horizontal has changed,
			offxn =  mouse.h /2;	//¥ store it in the new offset.
		else 
			if (mouse.h /2 > offxn)
				offxn = mouse.h / 2;
				
		if (mouse.v /2 < offyn)		//¥ If the vertical has changed,
			offyn = mouse.v / 2;	//¥ store it in the new offset.
		else 
			if (mouse.v /2 > offyn)
				offyn = mouse.v / 2;
				
		//¥ If the old offset ain't the same as the new one...
		if ((offxo != offxn) || (offyo != offyn)) 	
		{																				//¥ differs from the new, update.
			SetUp (offxn, offyn); //¥ draw the new setup...
			SetUp (offxo, offyo); //¥ and erase the old one.
		}
		Draw_Lines (offxo, offyo, wlines); //¥ Erase the vertical lines.
		wlines++; //¥ Increment wlines.
		if (wlines > 3) 
			wlines = 0; //¥ Reset wlines if too big.
			
			//¥ Draw new set of lines.
			Draw_Lines (offxn, offyn, wlines); 
	}
}

