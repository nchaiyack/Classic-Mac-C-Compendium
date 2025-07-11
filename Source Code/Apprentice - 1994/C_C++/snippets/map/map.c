#include <Quickdraw.h>
# include "trigtab.h"
# include <math.h>
# ifndef M_PI
# define M_PI 3.1415926536
# endif

extern trigtab  *trig;
extern int xmax,ymax;

# define degrad (180.0/M_PI)

double org_lat = -4.5;
double org_lng = 55.5;
double ref_lat,ref_lng,plotsize;
static long end;

static double sqr(double x)
	{
	return(x*x);
	}

void setcolor(int color)
    {
    }

void plot(double ref_lat, double ref_lng, double plotsize)
	{ 
	trigtab  *tr = trig; 
	double reflat = ref_lat/degrad;
	double reflng = ref_lng/degrad;
	double zr = cos(reflat);
	double yr = sin(reflat);
	double cosreflng = cos(reflng);
	double sinreflng = sin(reflng);
	int xmax2 = xmax/2;
	int ymax2 = ymax/2;
	int x2 = xmax2;
	int y2 = ymax2;
	double yscale = ymax2/sin(plotsize/degrad);
	double xscale = yscale;
	MoveTo(xmax2, ymax2);
	while ((tr->color >= 0))
		{
		double rc = tr->cosnewlat;
		double xc = rc*(tr->sinnewlng*cosreflng-tr->cosnewlng*sinreflng);
		double yc = tr->sinnewlat;
		double zc = rc*(tr->cosnewlng*cosreflng+tr->sinnewlng*sinreflng);
		double yrot = yc*zr-zc*yr;
		double sqdist = sqr(xc)+sqr(yc-yr)+sqr(zc-zr);
/*
		int x1 = x2;
		int y1 = y2;
*/
		x2 = xmax2+(int)(xc*xscale);
		y2 = ymax2-(int)(yrot*yscale);
		if (tr->color)
			{
   			MoveTo(x2,y2);
			setcolor(tr->color);
			}
		else
			{
			if ((sqdist > 0.0) && (sqdist <= 1.9) && ((xmax2 != x2)||(ymax2 != y2)))
				{
   				LineTo(x2,y2);
    			}
			else 
				{
   				MoveTo(x2,y2);
    			}
			}
		tr++;
		}
	}
