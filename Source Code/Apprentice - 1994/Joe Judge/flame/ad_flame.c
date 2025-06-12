
// flame attempt
//
//	- 4/30/94
//	- I think I'll use the horrible goto to help give time back to the system.
//
//
#include <Quickdraw.h>
#include <Windows.h>
#include <GraphicsModule_Types.h>
#include <Picker.h>

#include "ad_flame.h"
#include "xlock.h"

#include <math.h>

int errno;	// for the darned math.c file :)


#undef DEBUG


unsigned short RangedRdm( unsigned short min, unsigned short max );

void SetRGBColor(void);
void SetIndexedColor(void);


// flame structure ...each is associated with a "screen"
typedef struct {
    double		f[2][3][MAXLEV];	/* three non-homogeneous transforms */
    short		max_levels;
    short		cur_level;
    short		snum;
    short		anum;
    short		width, height;
    short		num_points;
    short		total_points;
	Point		pts[ MAXBATCH + 1];
	short		screenDepth;
    Rect		theRect;
    unsigned long startTime;
} flamestruct;



// globals ...
flamestruct flames[ MAXSCREENS];			// all the flame structs
short		theScreen;						// the current monitor we're working in
Boolean		gWaitForClick;					// awakening mode
extern Boolean gRainbowOption;				// multicolor option
extern short	gMaxBatch;					// how many points to plot at a time
extern short	gTotalPoints;				// how many points to plot before "done"
extern long		gTimeSlice;					// how long (seconds) for recursion to last?



/////////////////////////////////////////////////////////////////////////////////
// 
//	initialize the flame stuff
void
initflame(  Rect *bounds, char theDepth) 
{
    flamestruct *fs = &flames[ theScreen ];	// the flame structure for this screen
    
    // just my luck ... some nut would have > MAXSCREENS monitors 
    // hooked up on their Mac (you never know!)
    if (theScreen >= MAXSCREENS) 
    	return;
    
    fs->screenDepth = theDepth;
    
    fs->theRect = *bounds;
    fs->width = bounds->right - bounds->left;
    fs->height = bounds->bottom - bounds->top;
    fs->max_levels = BATCHCOUNT;
    
    BackColor( blackColor);
    EraseRect( bounds);
    
   
// set the forecolor to something
	if (fs->screenDepth > 8)		// direct rgb
		SetRGBColor();
	else if (fs->screenDepth > 1)	// CLUT
		SetIndexedColor();
	else 							// B&W
		ForeColor( whiteColor );
	
}


/////////////////////////////////////////////////////////////////////////////////
static Boolean
recurse(double x, double y, int l) {
int i;
double nx, ny;
Point mousePt1, mousePt2;
flamestruct *fs = &flames[ theScreen ];
unsigned long waste;
Boolean returnValue;
	
	
	GetMouse( &mousePt1);		
		
    if (l == fs->max_levels ) {
		fs->total_points++;
		if (fs->total_points > gTotalPoints)	/* how long each fractal runs */
		    return FALSE;						// was MAXTOTAL
		    
		if (x > -1.0 && x < 1.0 && y > -1.0 && y < 1.0) {
		    fs->pts[fs->num_points].h = (int) ((fs->width / 2)
							    * (x + 1.0));
		    fs->pts[fs->num_points].v = (int) ((fs->height / 2)
							    * (y + 1.0));
		    fs->num_points++;
		    
// was: if (fs->num_points >= MAXBATCH) 
			    if (fs->num_points >= gMaxBatch) {	/* point buffer size */
			    	// draw all the points in the  fs->pts[] array
			    	// fs->num_points tells how many
					XDrawPoints( fs->pts, fs->num_points);
	
					// this was a debug thing ... but I liked it so much, I put it in!
					if (gRainbowOption) {	// the rainbox option
						if (fs->screenDepth > 8)		// direct rgb
							SetRGBColor();
						else if (fs->screenDepth > 1)	// CLUT
							SetIndexedColor();
						else 							// B&W
							ForeColor( whiteColor );
					}
					
					fs->num_points = 0;
			    }
			}
    } else {
/////////////////////// major recursion section /////////////////////////
		i=0;
		while (i < fs->snum) {
		
		    nx = fs->f[0][0][i] * x + fs->f[0][1][i] * y + fs->f[0][2][i];
		    ny = fs->f[1][0][i] * x + fs->f[1][1][i] * y + fs->f[1][2][i];

		    if (i < fs->anum) {
		    	//SysBeep(0);
				nx = sin(nx);
				ny = cos(ny);	
	  		}
			if (!recurse( nx, ny, l + 1))
				return FALSE;
			i++;
		}
///////////////////////////////////////////////////////////////		
    }

///////// test if we need to leave
	    GetMouse(&mousePt2);
		if (gWaitForClick ) {
			if (Button()) {
				return FALSE;
			}
		} else if ( mousePt1.h != mousePt2.h || 
	 				mousePt1.v != mousePt2.v || Button())  {
	    	return FALSE;
		}
////////

// check for time limits ...
	GetDateTime( &waste );
	if ( (fs->startTime + gTimeSlice) < waste ) {
		return FALSE;
	}
	
    return TRUE;
}



void
drawflame()
{
    flamestruct *fs = &flames[theScreen];

    int         i, j, k;
	static Boolean alt;

    // just my luck ... some nut would have > MAXSCREENS monitors 
    // hooked up on their Mac (you never know!)
    if (theScreen >= MAXSCREENS) {
    	return;
    }	
		
	
	GetDateTime( &fs->startTime);
	
    if (!(fs->cur_level++ % fs->max_levels)) {
		EraseRect( &fs->theRect );
		alt = !alt;
    } else  {
		if (fs->screenDepth > 8)		// direct rgb
			SetRGBColor();
		else if (fs->screenDepth > 1)	// CLUT
			SetIndexedColor();
		else 							// B&W
			ForeColor( whiteColor );
	}


    /* number of functions */
    fs->snum = 2 + (fs->cur_level % (MAXLEV - 1));

    /* how many of them are of alternate form */
    if (alt)
		fs->anum = 0;
    else
		fs->anum = (Random() % fs->snum) + 2;		//halfrandom(fs->snum) + 2;


    /* 6 coefs per function */
    for (k = 0; k < fs->snum; k++) {
		for (i = 0; i < 2; i++)
		    for (j = 0; j < 3; j++) {
				fs->f[i][j][k] = ((double) (RangedRdm(0,1023)) / 512.0 - 1.0);
//				fs->f[i][j][k] = ((double) (Random() & 1023) / 512.0 - 1.0);
			}
    }
    fs->num_points = 0;
    fs->total_points = 0;
    
	(void) recurse(0.0, 0.0, 0);
    XDrawPoints(fs->pts, fs->num_points);
}


// draw a set of points .... 
void
XDrawPoints(Point	points[], short how_many) {
flamestruct *fs = &flames[theScreen];


	while (how_many-- > 0) {
		MoveTo( fs->theRect.left + points[how_many].h, 
				fs->theRect.top + points[how_many].v );
		LineTo( fs->theRect.left + points[how_many].h, 
				fs->theRect.top + points[how_many].v );
	}

}

/////////////////////////////////////////////////////////////////////////////////

// CHECK before calling this!! - color stuff going on here
// use HSV to set colors (to better ensure "bright" colors)
// I like this better ... I think I'll use this one
void
SetRGBColor(void) {
RGBColor rColor;
HSVColor hColor;

	// any hue/color
	hColor.hue = (SmallFract)RangedRdm(0, 65535);	
	
	// give it a bit more "punch" (top 1/2 of the range)
	hColor.saturation = (SmallFract) RangedRdm(32767, 65535); 	// was 16384, 32767, ...
	
	// any only the brighter half values of it
	hColor.value = (SmallFract) RangedRdm( 49150, 65535);		// was 32767, 49150
	
	HSV2RGB( &hColor, &rColor);
	RGBForeColor( &rColor);

}



// this isn't *really* needed ... but I just felt I needed to try something
// different with the clut mode monitors ... what the heck!
void
SetIndexedColor(void)  {
short numColors;
flamestruct *fs = &flames[ theScreen ];
RGBColor rgb;

	numColors = 1 << fs->screenDepth;
	// I do this, otherwise I'd ned to do a PaletteManager call to choose by index
	Index2Color( RangedRdm(1,numColors), &rgb );
	RGBForeColor( &rgb);

}


// this is from the Think C reference code example ...
unsigned short RangedRdm( unsigned short min, unsigned short max )
/* assume that min is less than max */
{
	// uh ... not this isn't quite right - it's between 0 and 65535, not 65536
	unsigned	qdRdm;	/* treat return value as 0-65536 */
	long	range, t;
	
	// just to be safe, I'll put this here
	if (min > max) DebugStr("\pMin greater then Max in RangedRdm");
	
	qdRdm = Random();
	range = max - min;
	// max - min gives us the the difference between max and min ... that is 
	// not inclusive. It gives us { min <= range < max }
	// so we never see that max number!!
	range++;
	t = ((long)qdRdm * range) / 65536; 	/* now 0 <= t <= range */
	return( t+min );
}
