//�---------------------------------------------------------------------------�//
//� "This Old Hack," modernized to run on Think C� v.5.0.4, by Kenneth A. Long,
//� on 24 July 1993, without the help of Norm Abrahm.  Free, so don't sell.
//�---------------------------------------------------------------------------�//
//� BackInUse.c		Original comments were left in and I added some.
//�---------------------------------------------------------------------------�//
//� Program INUSE.C for the Megamax C compiler
//�  (C) 1986 William Woody, commercial rights reserved

//� If you think this program is cute, please send me computer mail and tell
//� me so!  I personally think it's a cute hack, myself, but what do I know.

//� Though I don't believe this program has any commercial value,  (or ANY
//� monitary value whatsoever), I have reserved commercial rights anyways.
//� Silly me...  But if you want to give it to your friends, please leave
//� my name on the darn thing; I need to feel appreciated.
//�---------------------------------------------------------------------------�//

#include <stdio.h>
#include <quickdraw.h>		//� What they don't tell you is that you need to
							//� include this for the SRAND macro to work...

//extern int Random ();
extern char *malloc ();

#define MAXX 480
#define MAXY 300
#define MAXM 400

WindowPtr theWindow;
WindowRecord wRecord;
Rect dragRect, linesRect;
Rect windowBounds = { 40, 2, 380, 508 };

struct foo {
	double a,b;			//� This is a point which gets to fly around
	struct foo *next;
};

main ()
{
	short number,n,x,done = 0;
	struct foo *top,*Ptr,*Ptr2;
	
//	struct {
//		short top, left, bottom, right;
//	} linesRect;
	
	long ticks;		//� For the delay in the "splash" screen.
	
	double mx,my,nx,ny;
	
	MaxApplZone();

#ifdef THINK_C
	InitGraf(&thePort);
#else
	InitGraf(&qd.thePort);
#endif
	InitFonts();
	FlushEvents(everyEvent, 0);
	InitWindows();
	InitMenus();
	TEInit();
	InitDialogs(0L);
	InitCursor();

	//� We can't drag because mouseDown quits us.  
	//� But it's here if we want to rig it in later.
	dragRect = qd.screenBits.bounds;	
	
	theWindow = NewWindow (&wRecord, &windowBounds, "\pIn Use", true, 0, 
			 		(WindowPtr)-1L, false, 0L);
	SetPort (theWindow);


	//� We add a Rect to the window that we can draw in and erase.
	SetRect (&linesRect,0,0,506,380);
//		40, 2, 380, 508
	//� Make sure there's a clean slate to draw in.
	EraseRect (&linesRect);
		
	MoveTo (10,25);
	TextFont (systemFont);	//� System Font is zero, folks.
	TextSize (12);
	DrawString ("\pProgram InUse.C for the Megamax C compiler");
	MoveTo (10,40);
	DrawString ("\pCopyright�1985 by William Woody, commercial rights reserved.");
	MoveTo (10,55);
	DrawString ("\pTo halt the program, press the mouse Button.");
	TextFont (courier);		//� Change font to Courier 12 bold.
	TextSize (12);	
	TextFace (bold);
	MoveTo (10,70);
	DrawString ("\pMade to run on Code Warrior on 7 November 1994");
	MoveTo (10,85);
	DrawString ("\pBy Kenneth A. Long");	
	GetDateTime ((unsigned long*) &qd.randSeed);	//� Seed the random number generator
	TextFont (0);
	TextSize (12);	//� Set all the font stuff back so our SysParams are OK.
	TextFace (0);
	Delay (240L, &ticks);
	EraseRect (&linesRect);		//� Get ready for action, after bragging.
	
	while  (!done) 
	{
		EraseRect (&linesRect);		//� Added to not stack drawings up.
		number =  (Random () & 3) + 2;	//� Number of points generated.
		x = 0;
			
		top = Ptr = (struct foo *) malloc(sizeof(struct foo));

		//� While horizontal incrementing value is not the same 
		//� as number of points...
		while  (x++ != number)
	
		//� ...this stuff is what it is (?).
		Ptr =  (Ptr->next = (struct foo *) malloc (sizeof(struct foo)));
		Ptr =  (Ptr->next = top);
		
		mx = 0;
		my = 0;
		nx = 9999999.0;		//� Large numbers for finding minimum value.
		ny = 9999999.0;
		do 
		{
			Ptr->a =  (double) Random ();
			if  (Ptr->a > mx) mx = Ptr->a;
			if  (Ptr->a < nx) nx = Ptr->a;
			Ptr->b =  (double) Random ();
			if  (Ptr->b > my) my = Ptr->b;
			if  (Ptr->b < ny) ny = Ptr->b;
			Ptr = Ptr->next;
			
		} 
		//� As long as Ptr ain't the same as top...
		while  (Ptr != top);
			do 		//� ...make up the points and create "a".
			{		//� A circularly linked list of points.
				Ptr->a =  (Ptr->a - nx) * MAXX /  (mx - nx);
				Ptr->b =  (Ptr->b - ny) * MAXY /  (my - ny);
				Ptr = Ptr->next;	//� Ptr points to next point.
			} 
		//� Again, as long as Ptr ain't the same as top...
		while  (Ptr != top);
			n = 0;		//� ..."n" has 0 value.
			MoveTo ( (int) Ptr->a, (int) Ptr->b);
			
			//� While that's true, and while incrementing "n" is not the 
			//� same as 400 times the number of points generated...
			while  (n++ != MAXM * number)	
			{	//� ...move the points around, like this:
				
				//� Give "Ptr->a" a value of ITSELF times 40, PLUS
				//� the value of "Ptr->next->a" (see above "do loop"),
				//� then divide that product by 41.  That's the H coord.
				Ptr->a =  (Ptr->a * 40 + Ptr->next->a) / 41;
				
				//� Same for be to get the V coord.
				Ptr->b =  (Ptr->b * 40 + Ptr->next->b) / 41;
				
				//� Then draw the line.
				LineTo ( (int)Ptr->a, (int)Ptr->b);
				
				Ptr = Ptr->next;
				if  (Button ())	//� Halt program when Button is pressed.
				{
					done = 1;	//� It's one DONE puppy!
					break;		//� Unconditionally get the hell out.
				}
			}	//� done with "while".
			
			Ptr = top;	//� Clean up the circular linked list.
			do 
			{
				Ptr =  (Ptr2 = Ptr)->next;
				free (Ptr2);
			} 
			while  (Ptr != top);	//� As long as Ptr aint the top.
		EraseRect (&linesRect);
	}
}