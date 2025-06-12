/*

A note from Ken Long:

I downloaded this file off America Online, and it did not run.  It had
an added library called "TSkel.lib" and, within it, was a reference to 
"FlushEvents" which the Think C Linker called "undefined."  So, I had
to rebuild that library (I thought) from the TransSkel source project, 
which I also got two versions of off AOL.

But the TransSkel (any version) I had did not compile and build, either.
So I had to get that to happen first.  Since EarthPlot only wanted part
of the whole TransSkel package, I merely added TransSkel.c and its
#include to the EarthPlot project and it worked like a champ.

There was one call to cWindowUpdate which had too few arguments, all the
window behinds needed updated, but that was about it.

Since the merged file of all the Earth Plot source and headers only came
to 30k, I decided to pool them.  Hence only 3 files added to the project.

I've included the original file for comparison.

Try settings: 34N, 118W - Los Angeles (from "Map" Desk Accessory).
Watch for earthquakes!

*/

#include <stdio.h>
#include <Math.h>

#define AboutAlert	100
#define EarthIcon	512
#define iconID		512

#define altScale	1000

#define	Options		1
#define	drawEarth	2

#define diam  7926.0
#define pi    3.141592654
#define twopi 6.283185308
#define conv  0.017453293	/* (3.14159 / 180.0) */

#define pi100 0.03141592654	/* pi / 100 */
#define fudge 10000

/* real fudge so we can shift to divide rather than use expensive ldiv */
#define fudge2 20078

 
/*double		coslat, coslon, sinlon, x, y, z;*/
double			coslat, alat, alon, coslon, sinlon, x, y, z, a_real;
long		ix, iy, iz;
double		alatd, alond, height; 
double		cos1, cos2, sin1, sin2, xpos, xmax, scaler;
long		icos1, icos2, isin1, isin2, ixmax, ixpos, iscaler;
int			lat, lon, alt;
int			ihalf_xsize, ihalf_ysize;
char		s[255];
extern		long	oldBackground, oldFore;

Boolean hasColorQD;

WindowPtr	cWindow, eWindow;

PicHandle ePict, ebmPict;

int	draw;	/* != 0 if we want to draw the line.  ==0 if move to point */
int	over;	/* != 0 when point is outside visible area */

CursHandle watchCursorHand;

Handle iconHandle;

double	xsize, ysize;
double	half_xsize, half_ysize;
int		ixsize, iysize;

DialogPtr	optionsDialogPtr;

int npictButton, nbitmapButton, nintButton, nfpButton, nhiddenlinesBox, 
	nlnlBox, nsquareBox;
	
long backgroundC, earthbackgroundC, earthoutlineC, 
	 latC, longC, landC;

long	oldBackground	= whiteColor;
long	oldFore			= blackColor;

Rect latSBRect, lonSBRect, altSBRect, latDataRect, lonDataRect, altDataRect, 
	 northRect, southRect, eastRect, westRect, mileRect, kmRect, 
	 earthWindowRect, controlWindowRect, iconRect;

ControlHandle latSB, lonSB, altSB, northCheck, southCheck, eastCheck, 
			  westCheck, mileCheck, kmCheck;
