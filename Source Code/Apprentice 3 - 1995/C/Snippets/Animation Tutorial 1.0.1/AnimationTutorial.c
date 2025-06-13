/****************************************************************************************
Did you ever want to learn about CopyBits() animation? Do you wonder what the heck is 
CopyBits() anyway? Well this file is for you, it gives a little information on the 
theory of CopyBits() animation and then provides some well documented source code that
creates a simple screen saver by animating icons. If you have any questions please E-mail 
me on America On-Line at Bernard256. By the way this requires 32 bit QuickDraw. I've tested this
in 1 to 16 bit color and it works though you have to increase the memory for 16 bit. 
Feel free to upload this anywhere but if you want to change it let me know so
I can make the changes. Please send me criticisms :-) I compiled this on CodeWarrior but 
there is no reason for it not to work on Think C.
*****************************************************************************************
Some info on CopyBits() animation: If you already know the theories behind CopyBits()
animation skip this and get to the code otherwise...
First of all there are two parts to animation first of all displaying the various frames
in a row and second moving the moving frames around the screen. This program animates 
some icons found in the resource fork of this program. 
Here is basically how you animate on the Macintosh. First you create an Offscreen Graphic
World which is basically a place were you can draw things but they don't show up on the
screen. Then you draw all the things you are gonna animate onto this Offscreen world. 
Then you use CopyBits(), a system routine, to Copy the frame you are up to in your 
animation from offscreen to onscreen, then you do the next frame and so on. The obvious
question is why go through all the trouble, why not just draw your icons or whatever 
directly on to the screen. Well there are two reasons. 
		1.Speed: CopyBits() is a lot faster then reading from a resource. So it's faster
		  to have all your stuff on an Offscreen G World and just copy them when needed
		  then to read them from a resource when needed. You will notice however that
		  this program doesn't take advantage of this speed gain because actually what
		  it does is write from the resource to the offscreen world and then from the 
		  offscreen to the real screen. This might seem like it would actually be slower
		  then drawing directly to the screen from the resource and in fact it might be
		  but this leads us to the next reason for using CopyBits()
		2. Flicker Free Animation: Did you ever notice how when you draw a picture to the
		   screen or just open a picture with a drawing program how it scrolls down the
		   screen. Well no matter how fast it does this the fact is it does it. If you 
		   tried to do animation with this the picture would look like it was flickering.
		   However when CopyBits() copies graphics from one port to another it just puts
		   it up on the screen in one move. BAM! it's up there. This is the main 
		   advantage to CopyBits().
So here is some code using CopyBits() what it does is read the appropriate icons out
of a bunch of Icon Suites (ICN#,icl4,or icl8) and animates them and then bounces them
around the screen. 
***************************************************************************************/

#include<QDOffscreen.h>  /*you have to include these                */
#include<Icons.h> /*headers to do offscreen and icons suites*/

#define BEG 128 /*this defines at which id number your icons begin so if you have 10
				  icons from 800 to 810 set this to 800*/
#define MAX 11 /*this is the number of icons you have*/
#define MOVE_DISTANCE 3 /*this is the distance you move the icons while animating*/

void ToolBoxInit(void); /*this just does some basic ToolBox Initilization Stuff*/
int MyRandom(int num);  /*this gets random numbers*/
void Animate(void);		/*this draws the next frame and moves it over a little*/
void InitOtherStuff(void); /*initilizes the offscreen gworld and some other stuff*/

WindowPtr myWindow; /*pointer to the main window*/
CGrafPtr origPtr;	/*CGrafPtr to main window*/
GDHandle origGDH;   /*GDHandle to main window*/
GWorldPtr offscreen; /*pointer to offscreen gWorld*/
Rect iconRect;		/*theRect the icon is in*/
PixMapHandle myMap; /*pixel map for the gWorld*/
int i, oldHeight;   /*you'll see what these are for later*/
Boolean NE,NW,SE,SW;/*one of these will always be true and the others false the one thats
					  true tells which direction the icon is going. NorthEast NorthWest etc...*/



void main(void)
{	
	ToolBoxInit();    /*Do intilizations*/
	InitOtherStuff();
	NE = NW = SE = SW = FALSE;

	switch(MyRandom(4))  /*get a random number between one and four to tell us which 
							direction we are heading when we start*/
	{
		case 0:
			NE = TRUE;
			break;
		case 1:
			SE = TRUE;
			break;
		case 2:
			SW = TRUE;
			break;
		case 3:
			NW = TRUE;
			break;
	}
	i = 0; 
	while(!Button()) /*while the user hasen't clicked*/
		Animate();   /*advance one frame*/
	LMSetMBarHeight(oldHeight); /*set the menu back to original height (see InitOtherStuff()
								to see how we messed around with the menuBar)*/
}

void ToolBoxInit(void)
{
	InitGraf(&qd.thePort);
	InitFonts();
	FlushEvents(everyEvent,0);
	InitWindows();
	InitMenus();
	TEInit();
	InitDialogs(0L);
	InitCursor();
	MaxApplZone();
}
void InitOtherStuff(void)
{
	int h,v;
	
	GetDateTime((unsigned long*)&qd.randSeed);/*get a new seed every time*/
	myWindow = NewWindow(nil,&(qd.screenBits.bounds), "\p",TRUE,plainDBox,(WindowPtr)-1L,
							FALSE,0); /*create a new window as big as the screen*/
	ShowWindow(myWindow);	/*show that window*/
	oldHeight = LMGetMBarHeight(); /*save the menu bar height*/
	LMSetMBarHeight(0);		/*the set it to zero. You should not here that on 68k Macs 
							LMGetMBarHeight() is not a function (I think) but a wierd
							kind of variable that holds the MBar's height*/
	RectRgn(myWindow->visRgn,&myWindow->portRect);/*We are gonna set the item's visRgn
													to the window's portRect so that
													it obscures the menubar.*/
	GetGWorld(&origPtr,&origGDH); /*save the parameters of myWindow so we can get
									back to it*/
	NewGWorld(&offscreen,0,&myWindow->portRect,nil,nil,0); /*make a new offscreen g world
															 as big as myWindow*/
	myMap = GetGWorldPixMap(offscreen);  /*get the pixel map for that gWorld*/
	LockPixels(myMap);					/*lock it. you have to do this otherwise it might
										  get messed up by other stuff going on*/
	SetGWorld(offscreen,0L);			/*now make this gWorld the curent port*/
	h = MyRandom(qd.screenBits.bounds.right-32); /*get a random nubmer that is at least 32
												pixels away from the right of the screen
												this is goin to be our horizantal cord.
												for our icon*/
	v = MyRandom(qd.screenBits.bounds.bottom-32);/*get the vertical coordinate*/
	SetRect(&iconRect,h,v,h+32,v+32);         /*now set the rect with the h and v coord.
												as well as two other points 32 pixels 
												away so its the size of an icon.*/
	EraseRect(&offscreen->portRect);		/*erase the offscreen graphics port if you 
											  don't do this you will have beautiful 
											  multi-colored garbage in your port*/
	HideCursor();							/*hide the mouse cursor*/
	UnlockPixels(myMap);					/*unlock the pixels otherwise weird stuff
											  will happen*/
}

int MyRandom(int num)
{
	int l;
	
	l=Random(); /*make l a random number between -3 thousand something and 3 thousand 
				  something*/
	if (l==0) /*if l is 0 just return it right away. I don't like doing math with 0*/
		return l;
	if(l<0) /*if l is negative make it positive by multiplying by -1*/
		l*=-1;
	l=l%num; /*what this does is divide l by the num parameter and tell you the remainder
			   so if l is 10 and num is 4 then you will get 2 (10/4 is 2 remainder 2). 
			   What this does is only return 'num' possible numbers so if num is 4 you
			   can only return between 0 and 3*/
	return l;  /*return it!*/
}

void Animate(void) /*now for the fun*/
{
		myMap = GetGWorldPixMap(offscreen); /*get the pixMap again*/
		LockPixels(myMap);   /*and lock it*/
		SetGWorld(offscreen,0L); /*set the offscreen gworld to the current port*/
		
		if(NE) /*if we should be going northeast*/
		{
			if(iconRect.right < qd.screenBits.bounds.right && 
								iconRect.top > qd.screenBits.bounds.top) /*and we aren't 
																		moving of the
																		right or top
																		of the screen*/
				OffsetRect(&iconRect,MOVE_DISTANCE,-MOVE_DISTANCE); /*move the rect a
																	  a little NE*/
			else /*if we are going of the top or right*/
			{
				if(iconRect.right >= qd.screenBits.bounds.right)/*were going of the right*/
				{
					NE=FALSE;   /*bounce us back so now were going NW*/
					NW=TRUE;
				}
				if(iconRect.top <= qd.screenBits.bounds.top) /*if we are hitting the bottom
															bounce us up to the NE*/
				{
					NE=FALSE;
					SE=TRUE;
				}
			}/* all the rest of these algorithims just bounce us in different directions
				so I won't go into them*/
		}
		else if(NW)
		{
			if(iconRect.left > qd.screenBits.bounds.left && 
								iconRect.top > qd.screenBits.bounds.top)
				OffsetRect(&iconRect,-MOVE_DISTANCE,-MOVE_DISTANCE);
			else
			{
				if(iconRect.left <= qd.screenBits.bounds.left)
				{
					NW=FALSE;
					NE=TRUE;
				}
				if(iconRect.top <= qd.screenBits.bounds.top)
				{
					NW=FALSE;
					SW=TRUE;
				}
			}
		}
		else if(SE)
		{
			if(iconRect.right < qd.screenBits.bounds.right && 
								iconRect.bottom < qd.screenBits.bounds.bottom)
				OffsetRect(&iconRect,MOVE_DISTANCE,MOVE_DISTANCE);
			else
			{
				if(iconRect.right >= qd.screenBits.bounds.right)
				{
					SE=FALSE;
					SW=TRUE;
				}
				if(iconRect.bottom >= qd.screenBits.bounds.bottom)
				{
					SE=FALSE;
					NE=TRUE;
				}
			}
		}
		else if(SW)
		{
			if(iconRect.left > qd.screenBits.bounds.left && 
						iconRect.bottom < qd.screenBits.bounds.bottom)
				OffsetRect(&iconRect,-MOVE_DISTANCE,MOVE_DISTANCE);
			else
			{
				if(iconRect.left <= qd.screenBits.bounds.left)
				{
					SW=FALSE;
					SE=TRUE;
				}
				if(iconRect.bottom >= qd.screenBits.bounds.bottom)
				{
					SW=FALSE;
					NW=TRUE;
				}
			}
		} /*Now we have the rect in the right place*/
		
		FillRect(&offscreen->portRect,(ConstPatternParam)&qd.black);/*make the g world black cause it looks
											   cool and will erase the last drawing
											   we did*/
		PlotIconID(&iconRect,atNone,ttNone,BEG+i);/*plot you icon in iconRect. We are
													plotting the icon that is Id BEG plus
													which ever one we are up to, which
													is determined by i, i is incremented
													at the end of the routine*/
		SetGWorld(origPtr,origGDH); /*this is important before you call CopyBits() make
									  sure to make the window you are copying TO the
									  current port. If you don't the routine is 6 times
									  slower*/
		SetPort(myWindow);
		CopyBits( (BitMap *)(&offscreen->portPixMap),&myWindow->portBits,&offscreen->portRect,
						&myWindow->portRect, srcCopy,nil);/*this is the crux of it all
															we are copying from the 
															offscreen pixMap to myWindow's
															pixmap. And copying the whole
															offscreen rect to the main
															window's rect. We are using
															the srcCopy mode which just
															copys over anything already
															there*/
		UnlockPixels(myMap); /*now unlock the gWorld's pixels*/
		
		i = (i < MAX-1) ? i+1 : 0; 
					/*now we have to increment i so next time we'll draw the next frame
					 of the animation. if i is less then the MAX-1 (it has to be 1 less
					than MAX because we call this after we do the routine so it won't
					catch i until after it's been used) we have just make it
					the next one. if i is Max then set it back to the begining*/
		
}

/****************************************************************************************
That's it!! This is the basics of CopyBits() animation. Of course I just learned this
a little while ago so I make no guarantees. There probably is a lot of bad code. Sorry ;-).
For more info on all the graphic stuff like offscreen GWorlds see Inside Macintosh: 
Imaging with QuickDraw. For more on icon suites see Inside Macintosh: More Macintosh
ToolBox.

What can you do with this or what should I have done to make it faster?

	1. Well to make it faster there are a couple things I could have done:
		a)If a I copied all the icons to the offscreen world and then just put them on
		  the window when I needed them it would have been faster then redrawing them on
		  the gWorld every time I called Animate().
		b)If I hadn't copied the whole portRect every time just the part that needed to
		  be updated then it would have been faster also.
	2. If you make it move the frame a further distance it will appear to be faster. This
	   is cheating though. Sort of like making a movie faster by taking out every other
	   frame. The quality will seriously deteriorate if you do this.
	3. There are other faster ways of doing animation. You can use CopyMask() which is
		kind of like CopyBits() but it uses a Mask to know what parts to copy. It is 
		faster but I'm not sure how to do it.
	4. Set the colors to 1 bit or 4 bit instead of 8 bit. These makes it faster. In
		fact on my computer (6100/60) the black and white is too fast.
	5. I could have used something called Pixel Blitting which gives you the fastest
		animation, I hear. But I haven't the foggiest idea what it is. For more info on
		it and all fast animation routines download the file SpriteWorld. I'm in the 
		midst of going through it and it's great.
	6. How else could it be better? Better graphics. The graphics I used are horrible;
	    sorry I'm not an artist. Nevertheless the fact is one of the keys to animation is 
		smooth frames. Take the game Maelstrom for instance. It wouldn't be the same
		if not for the great graphics. Try using icons from your favorite game with
		this program.
	7. You can fool around with this. Try doing more the one icon by making Animate() a
		more general routing that takes maybe a rect and a direction as parameters.
		Try doing it using CIcons or Picts. 
	8. This should probably have error checking, including checking to see if the
	   the system has 32 bit QuickDraw, but I was to lazy to put it in. On of the main
	   errors you get with CopyBits() is running out of memory.
	   
	   
		
Whatever you do have fun and be sure to check out the Macintosh Development Forum
on America On-Line. E-Mail me with questions AOL: Bernard256
											 Internet: Bernard256@aol.com

****************************************************************************************/
