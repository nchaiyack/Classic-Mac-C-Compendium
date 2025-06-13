//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| This file contains the C++ versions of the antialiasing routines used by HyperCuber
//|________________________________________________________________________________


#include <FixMath.h>
#include <math.h>


//============================ Types ============================\\

typedef void (*IntensifyProc)(long, long, double);



//============================ Prototypes ============================\\

void DrawAntialiasedLine(short x1, short y1, short x2, short y2, RGBColor *color);

extern "C" void DrawAntialiasedLineO1asm(long x1, long y1, long x2, long y2, RGBColor *color);
extern "C" void DrawAntialiasedLineO2asm(long x1, long y1, long x2, long y2, RGBColor *color);
extern "C" void DrawAntialiasedLineO3asm(long x1, long y1, long x2, long y2, RGBColor *color);
extern "C" void DrawAntialiasedLineO4asm(long x1, long y1, long x2, long y2, RGBColor *color);

void DrawAntialiasedLineO1(long x1, long y1, long x2, long y2, RGBColor *color);
void DrawAntialiasedLineO2(long x1, long y1, long x2, long y2, RGBColor *color);
void DrawAntialiasedLineO3(long x1, long y1, long x2, long y2, RGBColor *color);
void DrawAntialiasedLineO4(long x1, long y1, long x2, long y2, RGBColor *color);

void DrawAntialiasedLineO4o(long x1, long y1, long x2, long y2, RGBColor *color);

void IntensifyPixel(long x, long y, Fract distance, RGBColor *color);

extern Boolean clip_line(long *p1h, long *p1v, long *p2h, long *p2v, Rect *clip_rect);




//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| Procedure: DrawAntialiasedLine
//|
//| Purpose: draw an antialiased line using Gupta-Sproull's algorithm
//|
//| Parameters: x1,y1,x2,y2:  endpoints of the line
//|             perspective:  TRUE if perspective should be used
//|             left_eye:     TRUE if the left eye view should be drawn;
//|                           FALSE if the right eye view should be drawn.
//|             hyperpane:    the hypercuber pane
//|             pane:         the graphics pane to draw in
//|             color:      color to draw the line
//|____________________________________________________________________

void DrawAntialiasedLine(long x1, long y1, long x2, long y2, RGBColor *color,
								Rect *clip_rect)
{

	if (!clip_line(&x1, &y1, &x2, &y2,			//  Clip line to clip rect
					clip_rect))
		return;									//  Return if line is entirely outside of cliprect
	
	register long xmin;
	register long xmax;
	register long ymin;
	register long ymax;

	long dx = (x2 - x1);
	long dy = (y2 - y1);

	if (x1 == x2)						//  Handle vertical lines as special case
		{
		RGBForeColor(color);
		MoveTo(x1, y1);
		LineTo(x2, y2);
		return;
		}

	if (x1 < x2)
		{  xmin = x1; xmax = x2;  }	//  Find min and max of x and y
	else
		{  xmin = x2; xmax = x1;  }

	if (y1 < y2)
		{  ymin = y1; ymax = y2;  }
	else
		{  ymin = y2; ymax = y1;  }

	if ((dx & 0x80000000) == (dy & 0x80000000))	//  If slope > 0, signs are the same
		{
		if (((dx > 0) && (dy > dx)) || ((dx < 0) && (dy < dx)))
			DrawAntialiasedLineO2asm(ymin, xmin, ymax, xmax, color);	//  slope > 1; octant 2
		else
			DrawAntialiasedLineO1asm(xmin, ymin, xmax, ymax, color);	//  slope < 1; octant 1
		}
	else
		{
		if (((dx > 0) && (dy > -dx)) || ((dx < 0) && (dy < -dx)))
			{
			DrawAntialiasedLineO4asm(xmin, -ymax, xmax, -ymin, color);	//  slope > -1; octant 4
			}
		else
			DrawAntialiasedLineO3asm(-ymax, xmin, -ymin, xmax, color);	//  slope < -1; octant 3
		}

}	//==== CGraphic::DrawAntialiasedLine () ====\\


//=============================================================================\\
//=============================================================================\\
//== Below are the C++ versions of the low-level graphics routines.  These   ==\\
//== are provided to make it easier to port these routines.  The routines    ==\\
//== actually used by HyperCuber are written in 68020 Assembly Language, are ==\\
//== in the file "HyperCuber Assembly.asm," and are much faster.  Note that  ==\\
//== the routines below use only integer arithmetic, through the use of the  ==\\
//== Macintosh-specific Fixed and Fract types.  The routines will work on    ==\\
//== other machines (which do not support these types) by using the double   ==\\
//== type.  To make this happen, you have to uncomment the double statements ==\\
//== and delete the Fract statements.  In general, however, floating point   ==\\
//== math is slower than integer math.                                       ==\\
//=============================================================================\\
//=============================================================================\\


Fract intensity_table2[16] = {	0x40000000, 0x3EF8D2E5, 0x3BEFE0CE, 0x370C77DE,
								0x30965F52, 0x29086BDF, 0x214BD33D, 0x1A03A323,
								0x1371971C, 0x0DBFD2E9, 0x09086BDF, 0x055831F0,
								0x02AF5C0E, 0x0100BE01, 0x002EA30B, 0x00000000   };

#if 0


void IntensifyPixel(long x, long y, Fract distance, RGBColor *color)
{

//	double intensity_table[16] = {	1., 0.983937, 0.936516, 0.860136, 0.759178, 0.641139,
// 									0.520253, 0.406472, 0.303808, 0.214833, 
//									0.141139, 0.083508, 0.0419531, 0.0156703, 0.00284649, 0 };

	register long red;
	register long green;
	register long blue;

//	double intensity = intensity_table[					//  Get intensity of this point from table
//					(short) floor(
//						(fabs(distance)*10 + .5))];

	long abs_distance = (distance < 0) ? -distance : distance;		//  Do above without FP arith.
	short index = (((abs_distance / 0x4000) * 10) + 0x8000) >> 16;
	
	Fract fract_intensity = intensity_table[index];
	Fixed intensity = fract_intensity >> 15;			//  This is not really a Fixed; decimal
														//    point is between bits 14 and 15.

	RGBColor old_color;
	GetCPixel(x, y, &old_color);						//  Get previous value of this pixel
	
	red = old_color.red + ((color->red * intensity) >> 15);		//  Merge old and drawing colors
	green = old_color.green + ((color->green * intensity) >> 15);
	blue = old_color.blue + ((color->blue * intensity) >> 15);

	RGBColor new_color;
	new_color.red = (red > 0xFFFF) ? 0xFFFF : red;		//  Clip color to 16-bit
	new_color.green = (green > 0xFFFF) ? 0xFFFF : green;
	new_color.blue = (blue > 0xFFFF) ? 0xFFFF : blue;

	SetCPixel(x, y, &new_color);						//  Draw the pixel

}	//==== IntensifyPixel() ====\\



void DrawAntialiasedLineO1(long x1, long y1, long x2, long y2, RGBColor *color)
{

	long dx, dy;
	register long x;
	register long y;
	register long d;
	long incrE, incrNE, two_v_dx;
	
	dx = x2 - x1;								//  Find horizontal and vertical distances
	dy = y2 - y1;
	
	d = 2*dy - dx;								//  Intialize decision variable
	
	incrE = 2*dy;								//  Increment for d used when moving E
	incrNE = 2*(dy-dx);							//  Increment for d used when moving NE
	
	two_v_dx = 0;								//  Numerator of distance from pixel; starts at 0

//	invDenom = 1/(2 * sqrt(dx*dx + dy*dy));		//  Denominator of distance from pixel

	Fract invDenom =							//  Do the above command using only int arithmetic
			FracDiv(0x10000,
				(FracSqrt((dx*dx + dy*dy) * 0x1000) / 16));
	
	Fract two_dx_invDenom = 2*dx*invDenom;		//  Precomputed constant

	x = x1;										//  Initialize "current point" to start of line
	y = y1;
	
	IntensifyPixel(x, y, 0, color);
	IntensifyPixel(x, y + 1, two_dx_invDenom, color);
	IntensifyPixel(x, y - 1, two_dx_invDenom, color);
		
	while (x < x2)
		{
		
		if (d < 0)
			{
			two_v_dx = d + dx;					//  Move East
			d += incrE;
			x++;
			}
		else
			{
			two_v_dx = d - dx;					//  Move NorthEast
			d += incrNE;
			x++;
			y++;
			}
		
		IntensifyPixel(x, y, two_v_dx * invDenom, color);
		IntensifyPixel(x, y + 1, two_dx_invDenom - two_v_dx * invDenom, color);
		IntensifyPixel(x, y - 1, two_dx_invDenom + two_v_dx * invDenom, color);
		
		}	//  end while

}	//==== DrawAntialiasedLineO1() ====\\



void DrawAntialiasedLineO2(long x1, long y1, long x2, long y2, RGBColor *color)
{

	long dx, dy;
	register long x;
	register long y;
	register long d;
	long incrE, incrNE, two_v_dx;
	
	dx = x2 - x1;								//  Find horizontal and vertical distances
	dy = y2 - y1;
	
	d = 2*dy - dx;								//  Intialize decision variable
	
	incrE = 2*dy;								//  Increment for d used when moving E
	incrNE = 2*(dy-dx);							//  Increment for d used when moving NE
	
	two_v_dx = 0;								//  Numerator of distance from pixel; starts at 0

//	invDenom = 1/(2 * sqrt(dx*dx + dy*dy));		//  Denominator of distance from pixel

	Fract invDenom =							//  Do the above command using only int arithmetic
			FracDiv(0x10000,
				(FracSqrt((dx*dx + dy*dy) * 0x1000) / 16));
	
	Fract two_dx_invDenom = 2*dx*invDenom;		//  Precomputed constant

	x = x1;										//  Initialize "current point" to start of line
	y = y1;
	
	IntensifyPixel(y, x, 0, color);
	IntensifyPixel(y + 1, x, two_dx_invDenom, color);
	IntensifyPixel(y - 1, x, two_dx_invDenom, color);
		
	while (x < x2)
		{
		
		if (d < 0)
			{
			two_v_dx = d + dx;					//  Move East
			d += incrE;
			x++;
			}
		else
			{
			two_v_dx = d - dx;					//  Move NorthEast
			d += incrNE;
			x++;
			y++;
			}

		IntensifyPixel(y, x, two_v_dx * invDenom, color);
		IntensifyPixel(y + 1, x, two_dx_invDenom - two_v_dx * invDenom, color);
		IntensifyPixel(y - 1, x, two_dx_invDenom + two_v_dx * invDenom, color);
		
		}	//  end while

}	//==== DrawAntialiasedLineO2() ====\\



void DrawAntialiasedLineO3(long x1, long y1, long x2, long y2, RGBColor *color)
{

	long dx, dy;
	register long x;
	register long y;
	register long d;
	long incrE, incrNE, two_v_dx;
	
	dx = x2 - x1;								//  Find horizontal and vertical distances
	dy = y2 - y1;
	
	d = 2*dy - dx;								//  Intialize decision variable
	
	incrE = 2*dy;								//  Increment for d used when moving E
	incrNE = 2*(dy-dx);							//  Increment for d used when moving NE
	
	two_v_dx = 0;								//  Numerator of distance from pixel; starts at 0

//	invDenom = 1/(2 * sqrt(dx*dx + dy*dy));		//  Denominator of distance from pixel

	Fract invDenom =							//  Do the above command using only int arithmetic
			FracDiv(0x10000,
				(FracSqrt((dx*dx + dy*dy) * 0x1000) / 16));
	
	Fract two_dx_invDenom = 2*dx*invDenom;		//  Precomputed constant

	x = x1;										//  Initialize "current point" to start of line
	y = y1;

	IntensifyPixel(y, -x, 0, color);
	IntensifyPixel(y + 1, -x, two_dx_invDenom, color);
	IntensifyPixel(y - 1, -x, two_dx_invDenom, color);
		
	while (x < x2)
		{
		
		if (d < 0)
			{
			two_v_dx = d + dx;					//  Move East
			d += incrE;
			x++;
			}
		else
			{
			two_v_dx = d - dx;					//  Move NorthEast
			d += incrNE;
			x++;
			y++;
			}
		
		IntensifyPixel(y, -x, two_v_dx * invDenom, color);
		IntensifyPixel(y + 1, -x, two_dx_invDenom - two_v_dx * invDenom, color);
		IntensifyPixel(y - 1, -x, two_dx_invDenom + two_v_dx * invDenom, color);

		}	//  end while

}	//==== DrawAntialiasedLineO3() ====\\



void DrawAntialiasedLineO4(long x1, long y1, long x2, long y2, RGBColor *color)
{

	long dx, dy;
	register long x;
	register long y;
	register long d;
	long incrE, incrNE, two_v_dx;
	
	dx = x2 - x1;								//  Find horizontal and vertical distances
	dy = y2 - y1;
	
	d = 2*dy - dx;								//  Intialize decision variable
	
	incrE = 2*dy;								//  Increment for d used when moving E
	incrNE = 2*(dy-dx);							//  Increment for d used when moving NE
	
	two_v_dx = 0;								//  Numerator of distance from pixel; starts at 0

//	invDenom = 1/(2 * sqrt(dx*dx + dy*dy));		//  Denominator of distance from pixel

	Fract invDenom =							//  Do the above command using only int arithmetic
			FracDiv(0x10000,
				(FracSqrt((dx*dx + dy*dy) * 0x1000) / 16));
	
	Fract two_dx_invDenom = 2*dx*invDenom;		//  Precomputed constant

	x = x1;										//  Initialize "current point" to start of line
	y = y1;
	
	IntensifyPixel(x, -y, 0, color);
	IntensifyPixel(x, -y - 1, two_dx_invDenom, color);
	IntensifyPixel(x, -y + 1, two_dx_invDenom, color);
		
	while (x < x2)
		{
		
		if (d < 0)
			{
			two_v_dx = d + dx;					//  Move East
			d += incrE;
			x++;
			}
		else
			{
			two_v_dx = d - dx;					//  Move NorthEast
			d += incrNE;
			x++;
			y++;
			}
		
		IntensifyPixel(x, -y, two_v_dx * invDenom, color);
		IntensifyPixel(x, -y - 1, two_dx_invDenom - two_v_dx * invDenom, color);
		IntensifyPixel(x, -y + 1, two_dx_invDenom + two_v_dx * invDenom, color);
		
		}	//  end while

}	//==== DrawAntialiasedLineO4() ====\\




void DrawAntialiasedLineO4o(long x1, long y1, long x2, long y2, RGBColor *color)
{

	long dx, dy;
	register long x;
	register long y;
	register long d;
	long incrE, incrNE, two_v_dx;
	
	PixMapHandle pixmap = ((CGrafPtr) thePort)->portPixMap;
	short line_width = (*pixmap)->rowBytes & 0x7FFF;
	unsigned char *base_address = (unsigned char *) (*pixmap)->baseAddr - (*pixmap)->bounds.top*line_width -
							(*pixmap)->bounds.left;
	register unsigned char *this_pixel = base_address - line_width*y1 + x1;
	
	dx = x2 - x1;								//  Find horizontal and vertical distances
	dy = y2 - y1;
	
	d = 2*dy - dx;								//  Intialize decision variable
	
	incrE = 2*dy;								//  Increment for d used when moving E
	incrNE = 2*(dy-dx);							//  Increment for d used when moving NE
	
	two_v_dx = 0;								//  Numerator of distance from pixel; starts at 0

//	invDenom = 1/(2 * sqrt(dx*dx + dy*dy));		//  Denominator of distance from pixel

	Fract invDenom =							//  Do the above command using only int arithmetic
			FracDiv(0x10000,
				(FracSqrt((dx*dx + dy*dy) * 0x1000) / 16));
	
	Fract two_dx_invDenom = 2*dx*invDenom;		//  Precomputed constant

	x = x1;										//  Initialize "current point" to start of line
	y = y1;
	
	IntensifyPixel(x, -y, 0, color);
	IntensifyPixel(x, -y - 1, two_dx_invDenom, color);
	IntensifyPixel(x, -y + 1, two_dx_invDenom, color);
		
	while (x < x2)
		{
		
		if (d < 0)
			{
			two_v_dx = d + dx;					//  Move East
			d += incrE;
			this_pixel++;
			}
		else
			{
			two_v_dx = d - dx;					//  Move NorthEast
			d += incrNE;
			this_pixel -= line_width;
			this_pixel++;
			}
				
		
//======== Draw the first pixel
		
//		IntensifyPixel(x, -y, two_v_dx * invDenom, color);

		RGBColor rgb_black = {0, 0, 0};
		Fract distance, fract_intensity;
		long abs_distance;
		Fixed intensity;									//  This is not really a Fixed; decimal
															//    point is between bits 14 and 15.
		RGBColor old_color, new_color;
		long red, green, blue;
		long index;
		
		distance = two_v_dx * invDenom;
		
		abs_distance = (distance < 0) ? -distance : distance;
		index = (((abs_distance / 0x4000) * 10) + 0x8000) >> 16;
	
		fract_intensity = intensity_table[index];
		intensity = fract_intensity >> 15;			

		Index2Color(*this_pixel, &old_color);				//  Get previous value of this pixel
		
		old_color = rgb_black;
		
		red = old_color.red + ((color->red * intensity) >> 15);	//  Merge old and drawing colors
		green = old_color.green + ((color->green * intensity) >> 15);
		blue = old_color.blue + ((color->blue * intensity) >> 15);
	
		new_color.red = (red > 0xFFFF) ? 0xFFFF : red;		//  Clip color to 16-bit
		new_color.green = (green > 0xFFFF) ? 0xFFFF : green;
		new_color.blue = (blue > 0xFFFF) ? 0xFFFF : blue;
	
		*this_pixel = Color2Index(&new_color);				//  Draw the pixel

//======== Draw the second pixel
		
//		IntensifyPixel(x, -y - 1, two_dx_invDenom - two_v_dx * invDenom, color);

		this_pixel -= line_width;

		distance = two_dx_invDenom - two_v_dx * invDenom;
		
		abs_distance = (distance < 0) ? -distance : distance;
		index = (((abs_distance / 0x4000) * 10) + 0x8000) >> 16;
	
		fract_intensity = intensity_table[index];
		intensity = fract_intensity >> 15;			

		Index2Color(*this_pixel, &old_color);				//  Get previous value of this pixel
		
		old_color = rgb_black;

		red = old_color.red + ((color->red * intensity) >> 15);	//  Merge old and drawing colors
		green = old_color.green + ((color->green * intensity) >> 15);
		blue = old_color.blue + ((color->blue * intensity) >> 15);
	
		new_color.red = (red > 0xFFFF) ? 0xFFFF : red;		//  Clip color to 16-bit
		new_color.green = (green > 0xFFFF) ? 0xFFFF : green;
		new_color.blue = (blue > 0xFFFF) ? 0xFFFF : blue;
	
		*this_pixel = Color2Index(&new_color);				//  Draw the pixel

		this_pixel += line_width;

//======== Draw the third pixel
		
		IntensifyPixel(x, -y + 1, two_dx_invDenom + two_v_dx * invDenom, color);

		this_pixel += line_width;
		
		distance = two_dx_invDenom + two_v_dx * invDenom;
		
		abs_distance = (distance < 0) ? -distance : distance;
		index = (((abs_distance / 0x4000) * 10) + 0x8000) >> 16;
	
		fract_intensity = intensity_table[index];
		intensity = fract_intensity >> 15;			

		Index2Color(*this_pixel, &old_color);				//  Get previous value of this pixel

		old_color = rgb_black;
		
		red = old_color.red + ((color->red * intensity) >> 15);	//  Merge old and drawing colors
		green = old_color.green + ((color->green * intensity) >> 15);
		blue = old_color.blue + ((color->blue * intensity) >> 15);
	
		new_color.red = (red > 0xFFFF) ? 0xFFFF : red;		//  Clip color to 16-bit
		new_color.green = (green > 0xFFFF) ? 0xFFFF : green;
		new_color.blue = (blue > 0xFFFF) ? 0xFFFF : blue;
	
		*this_pixel = Color2Index(&new_color);				//  Draw the pixel

		this_pixel -= line_width;

		
		}	//  end while

}	//==== DrawAntialiasedLineO4o() ====\\

#endif
