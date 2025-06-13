//¥ PD color star warp program sent to me (Ken Long) by a programmer 
//¥ on the net, on request.
//¥ Pretty much generic and common (there are lots of warp programs).
//¥ Hold down '+' to accelerate, '-' to decelerate, mouse turns, 'q'
//¥ or click quits.
//¥ Be sure to run this program in 1 bit mode, too!

//¥ Uses direct screen access; 
//¥ Needs Apple Macintosh with FPU (not!).  
//¥ Works without FPU but very slow.
//¥ Optimized for shipSpeed, not elegance.

//¥ TODO:
//¥ Don't assume monitor is landscape.
//¥ improve translation algorithm, use mathematically, graphically
//¥ 	correct algorithm.
//¥ Use keyboard map.
//¥ Use integer math?
//¥ Improve separation of code for different screen depths.
//¥ Make more structured, but keep code fast.

#include <math.h>

#ifndef powerc
	#include <SANE.h>
#endif

#include <Types.h>
#include <Memory.h>
#include <Quickdraw.h>
#include <Fonts.h>
#include <Events.h>
#include <Menus.h>
#include <Windows.h>
#include <TextEdit.h>
#include <Dialogs.h>
#include <OSUtils.h>

#define kNumberOfStars 90	//¥ Reduce number for more speed (try 18).
						
//¥ Apparent density of star field; number of 'close calls'.
#define density 1			

#define max_distance 255	//¥ Index of last color in 8 bit clut.
#define star_clut_ID 128

EventRecord			theEvent;
Boolean				keep_going = true;
RgnHandle			GrayRgn, mBarRgn;
short				*mBarHeightPtr;
short				oldMBarHeight;
Rect				windowBounds;
CWindowPtr			theWindow;
short				H_far, V_far;
double				star_H [kNumberOfStars] , star_V [kNumberOfStars] , distance [kNumberOfStars] ;
double				shipSpeed;

void HireTheManagers (void);
void HideMenuBar (void);
void ShowMenuBar (void);
void SetUpWindow (void);
long double Randomize (short high);
long double A_B (long double number);
void LoadStars (void);
void HandleEvent (void);
void MainlyLooping (void);
int main (void);

void HireTheManagers (void) 
{
	MaxApplZone ();
	InitGraf (&qd.thePort);
	InitFonts ();
	FlushEvents (everyEvent, 0);
	InitWindows ();
	InitMenus ();
	TEInit ();
	InitDialogs (0L);
	InitCursor ();
	MoreMasters ();
}

void HideMenuBar (void) 
{
	Rect	mBarRect;

	GrayRgn = GetGrayRgn ();
	mBarHeightPtr = (short *) 0x0BAA;
	oldMBarHeight = *mBarHeightPtr;
	*mBarHeightPtr = 0;
	mBarRect = qd.screenBits.bounds;
	mBarRect.bottom = mBarRect.top+oldMBarHeight;
	mBarRgn = NewRgn ();
	RectRgn (mBarRgn, &mBarRect);
	UnionRgn (GrayRgn, mBarRgn, GrayRgn);
	PaintOne (0L, mBarRgn);
}

//¥ Show me the way to the next Whiskey bar.
void ShowMenuBar (void)	
{
	*mBarHeightPtr = oldMBarHeight;
	DiffRgn (GrayRgn, mBarRgn, GrayRgn);
	DisposeRgn (mBarRgn);
}

void SetUpWindow (void) 
{
	short	centerx, centery;

	windowBounds = qd.screenBits.bounds;
	theWindow = (CWindowPtr)NewCWindow (0L, &windowBounds, "\pNameless", true, 
							(short)plainDBox, (WindowPtr)-1L, true, (long)0);
	SetPort ((GrafPtr)theWindow);
	InvertRect (&windowBounds);
	centerx = - (windowBounds.right/2);
	centery = - (windowBounds.bottom/2);
	SetOrigin (centerx, centery);
	H_far = windowBounds.right/2;
	V_far = windowBounds.bottom/2;
}

long double Randomize (short high)
{
	long double	rawResult;

	rawResult = Random ();
	return ((rawResult * high) / 32768);
}

long double A_B (long double number)
{
	if (number < 0) 
	return - number;
		else
	return number;
}

void LoadStars (void)		//¥ Could amount to trillions of tons!
{
	short	i;

	for (i = 0; i < kNumberOfStars; i++) 
	{
		star_H [i] = Randomize (H_far);
		star_V [i] = Randomize (V_far);
		distance [i] = Randomize (max_distance);
	}
}

void HandleEvent () 
{
	char	theChar;

	switch (theEvent.what) 
	{
		case mouseDown:
			keep_going = FALSE;
		break;
		
		case keyDown:
		case autoKey:
			theChar = (theEvent.message & charCodeMask);
			if ((theChar == '=')||(theChar == '+')) 
				shipSpeed += 0.5;
			if ((theChar == '-')||(theChar == '_')) 
				shipSpeed -= 0.5;
			if ((theChar == 'q')||(theChar == 'Q')) 
				keep_going = FALSE;
		break;
			
		default:
		break;
	}
}

void MainlyLooping (void)		//¥ Loop 'til you poop!
{
	Point mouseLoc;
	short A_B_H, A_B_V;
	short dark, gray, x_edge, y_edge, diagonal, j, k;
	double mx, my, tempx, tempy, temp, sin_rotate, cos_rotate;
	register short i;
	
	//¥ graphics vars
	GDHandle		theGDevice;
	PixMapHandle	thePixMap;
	unsigned char *drawAddr;
	unsigned char *baseAddr;
	unsigned long rowBytes;
	short			depth;
	Size			mapSize;
	unsigned char mask;
	CTabHandle		starclut;

	//¥ initial settings.
	LoadStars ();
	
	SetEventMask (mDownMask + keyDownMask + autoKeyMask);
	shipSpeed = 2;
	diagonal = (short) sqrt (((long double) H_far * H_far) + 
						 ((long double) V_far * V_far));
	x_edge = H_far - 10;
	y_edge = V_far - 10;
	
	H_far -= 2;
	V_far -= 2;

	//¥ get info about current device to be drawn to.
	theGDevice = GetGDevice ();
	thePixMap = (*theGDevice) ->gdPMap;
	baseAddr = (unsigned char *)(*thePixMap) ->baseAddr;
	rowBytes = (unsigned long)  ((*thePixMap) ->rowBytes & 0x3FFF);
	depth = (*thePixMap) ->pixelSize;
	mapSize = rowBytes*qd.screenBits.bounds.bottom;

	//¥ set color table to star grays if device is 8 bit.
	if (depth == 8) 
	{
			starclut = (CTabHandle)GetResource ('clut', star_clut_ID);
			SetEntries (-1, ((*starclut)->ctSize) +1, (*starclut)->ctTable);
	}

	if (depth != 8 && depth != 1) 
		keep_going = false;

	//¥ main loop.
	while (keep_going == true) 
	{
		//¥ handle user input.
		if (GetNextEvent (everyEvent, &theEvent)) 
			HandleEvent ();
			GetMouse (&mouseLoc);
			mx = ((double) mouseLoc.h / 420);
			if (A_B (mouseLoc.h) < 2) 
				mx = 0;
				my = ((double) mouseLoc.v / 8);
				sin_rotate = sin (mx);
				cos_rotate = cos (mx);
	
				//¥ cycle through list of stars.
				for (i = 0; i < kNumberOfStars; i++) 
				{
					A_B_H = star_H [i] + H_far;
					A_B_V = star_V [i] + V_far;
				
					//¥ erase star
					if ((A_B (star_V [i]) <V_far) && (A_B (star_H [i]) <H_far)) 
					{
						switch (depth) 
						{
							case 1:
								drawAddr = baseAddr+ (A_B_V * rowBytes + A_B_H/8L);
								mask = (unsigned char) 1 << (7- (A_B_H% 8));
								*drawAddr |= mask;
							break;
							
							case 8:
								drawAddr = baseAddr+A_B_V * 
										 rowBytes + A_B_H;
								for (j = 1; j <= 3; j++) 
								{
									for (k = 1; k <= 3; k++) 
									{
										*drawAddr = 0xFF;
										drawAddr++;
									}
									drawAddr -= 3;
									drawAddr += rowBytes;
								}
							break;
							default:
							break;
						}
					}
								
					//¥ update star's mapped projection.
					temp = (density + shipSpeed + distance [i]) / 
						   (distance [i] + density);
						 
					star_H [i] = star_H [i] *temp;
					star_V [i] = star_V [i] *temp + my;
					temp = star_H [i] ;
					
					star_H [i] = star_H [i] *cos_rotate + 
								 star_V [i] *sin_rotate;
								 
					star_V [i] = -temp * sin_rotate + star_V [i] * cos_rotate;
					distance [i] -= shipSpeed;
				
					//¥ if offscreen or past, generate new star*/
					if ((A_B (star_H [i]) > diagonal) ||
						 (A_B (star_V [i]) > diagonal) ||
						 (distance [i] < 0) ||
						 (distance [i] > max_distance)) 
					{
						if (shipSpeed > 0) 	//¥ going forwards.
						{
							star_H [i] = Randomize (x_edge);
							star_V [i] = Randomize (y_edge) - my;
							distance [i] = max_distance;
						}
						else		//¥ going backwards (not finished).
							{
								if (Randomize (1) > 0) 
								{
									star_H [i] = Randomize (H_far);
									if (Randomize (1) > 0) 
										star_V [i] = V_far;
									else
										star_V [i] = -V_far;
								}
								else
									{
										star_V [i] = Randomize (V_far);
										if (Randomize (1) < 0) 
											star_H [i] = H_far;
										else
											star_H [i] =- H_far;
								}
								distance [i] = A_B (Randomize (max_distance - 20)) +20;
							}
						}
					
						//¥ redraw star at new position
						A_B_H = star_H [i] + H_far;
						A_B_V = star_V [i] + V_far;
						if ((A_B (star_V [i]) < V_far) && (A_B (star_H [i]) < H_far)) 
						{
							switch (depth) 
							{
								case 1:
									drawAddr = baseAddr + 
											  (A_B_V * 
											   rowBytes + 
											   A_B_H / 8L);
											   
									mask = (unsigned char) 1 << 
										   (7 - (A_B_H % 8));
										   
									*drawAddr ^= mask;
								break;
								
								case 8:
									dark = max_distance - 
										 ((max_distance - 
										   distance [i]) / 6);
										   
									gray = max_distance - 
										 ((max_distance - 
										   distance [i]) / 4);
										   
									drawAddr = baseAddr + 
											   A_B_V * 
											   rowBytes + 
											   A_B_H;
											   
									*drawAddr = dark;
									drawAddr++;
									*drawAddr = gray;
									drawAddr++;
									*drawAddr = dark;
									drawAddr += rowBytes - 2;
									*drawAddr = gray;
									drawAddr++;
									*drawAddr = distance [i] ;
									drawAddr++;
									*drawAddr = gray;
									drawAddr += rowBytes - 2;
									*drawAddr = dark;
									drawAddr++;
									*drawAddr = gray;
									drawAddr++;
									*drawAddr = dark;
								break;
								
								default:
								break;
							}
						}
					}
	}
	SetEventMask (everyEvent);
}

int main (void)
{
	HireTheManagers ();		//¥ These guys Macintize.
	HideMenuBar ();			//¥ Get rid of this interference.
	SetUpWindow ();			//¥ A place to draw.
	HideCursor ();			//¥ Swat that fly!
	MainlyLooping ();		//¥ Get yer mo jo workin'.
	ShowCursor ();			//¥ Return of the Fly!
	ShowMenuBar ();			//¥ Get ready for Finder's keepers.
}

