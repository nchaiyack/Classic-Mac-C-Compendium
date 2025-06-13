// This may look like C code, but it is really -*- C++ -*-
/*
 ************************************************************************
 *
 *			   Grayscale Image
 * 		Implementation of the Primitive Operations
 *		     on the rectangular area of the image
 *
 *   The image is represented as a Pixmap, i.e. a matrix of pixels
 *    each of them specifies the gray level at a particular point
 *
 ************************************************************************
 */

#include "image.h"

#pragma implementation


/*
 *------------------------------------------------------------------------
 *    Construct a new image from the rectangular area of another image
 */

IMAGE::IMAGE(const Rectangle& ra)
{
  allocate(ra.nrows,ra.ncols,ra.image.bits_per_pixel);
  register GRAY * ps = ra.ptr;
  register GRAY * pi = pixels;
  register int i,j;

  for(i=0; i < ra.nrows; i++, ps += ra.inc_to_nextrow)
    for(j=0; j < ra.ncols; j++)		// Proceed scanline by scanline
      *pi++ = *ps++;

  assert( pi == pixels + npixels );
}

/*
 *------------------------------------------------------------------------
 *		Modify all the pixels in the rectangular area
 *		  according to a particular operation
 */

				// Assign a value to all the pixels
Rectangle& Rectangle::operator = (const int val)
{
  register GRAY * pp = ptr;
  register int i,j;

  for(i=0; i < nrows; i++, pp += inc_to_nextrow)
    for(j=0; j < ncols; j++)		// Proceed scanline by scanline
      *pp++ = val;

  return *this;
}

				// Increment the pixel value over the
				// rectangular area
void operator += (const Rectangle& ra, const int val)
{
  register GRAY * pp = ra.ptr;
  register int i,j;

  for(i=0; i < ra.nrows; i++, pp += ra.inc_to_nextrow)
    for(j=0; j < ra.ncols; j++)		// Proceed scanline by scanline
      *pp++ += val;
}

				// Decrement the pixel value over the
				// rectangular area
void operator -= (const Rectangle& ra, const int val)
{
  register GRAY * pp = ra.ptr;
  register int i,j;

  for(i=0; i < ra.nrows; i++, pp += ra.inc_to_nextrow)
    for(j=0; j < ra.ncols; j++)		// Proceed scanline by scanline
      *pp++ -= val;
}

				// Multiply the pixel value over the
				// rectangular area by the const
void operator *= (const Rectangle& ra, const int val)
{
  register GRAY * pp = ra.ptr;
  register int i,j;

  for(i=0; i < ra.nrows; i++, pp += ra.inc_to_nextrow)
    for(j=0; j < ra.ncols; j++)		// Proceed scanline by scanline
      *pp++ *= val;
}

				// Perform the logical OR on the pixel values 
				// over the rectangular area
void operator |= (const Rectangle& ra, const int val)
{
  register GRAY * pp = ra.ptr;
  register int i,j;

  for(i=0; i < ra.nrows; i++, pp += ra.inc_to_nextrow)
    for(j=0; j < ra.ncols; j++)		// Proceed scanline by scanline
      *pp++ |= val;
}

				// Perform the logical AND on the pixel values 
				// over the rectangular area
void operator &= (const Rectangle& ra, const int val)
{
  register GRAY * pp = ra.ptr;
  register int i,j;

  for(i=0; i < ra.nrows; i++, pp += ra.inc_to_nextrow)
    for(j=0; j < ra.ncols; j++)		// Proceed scanline by scanline
      *pp++ &= val;
}

				// Perform the logical XOR on the pixel values 
				// over the rectangular area
void operator ^= (const Rectangle& ra, const int val)
{
  register GRAY * pp = ra.ptr;
  register int i,j;

  for(i=0; i < ra.nrows; i++, pp += ra.inc_to_nextrow)
    for(j=0; j < ra.ncols; j++)		// Proceed scanline by scanline
      *pp++ ^= val;
}

				// Shift the pixel values 
				// over the rectangular area to the left
void operator <<= (const Rectangle& ra, const int val)
{
  register GRAY * pp = ra.ptr;
  register int i,j;

  if( abs(val) >= GRAY_MAXBIT )
    _error("Very fishy shift factor: %d",val);

  for(i=0; i < ra.nrows; i++, pp += ra.inc_to_nextrow)
    for(j=0; j < ra.ncols; j++)		// Proceed scanline by scanline
      *pp++ <<= val;
}

				// Shift the pixel values 
				// over the rectangular area to the right
void operator >>= (const Rectangle& ra, const int val)
{
  register GRAY * pp = ra.ptr;
  register int i,j;

  if( abs(val) >= GRAY_MAXBIT )
    _error("Very fishy shift factor: %d",val);

  for(i=0; i < ra.nrows; i++, pp += ra.inc_to_nextrow)
    for(j=0; j < ra.ncols; j++)		// Proceed scanline by scanline
      *pp++ >>= val;
}


					// Get a total sum of all the pixels
					// over the rectangular area
double sum_over(const Rectangle& ra)
{
  register GRAY_SIGNED * pp = (GRAY_SIGNED *)ra.ptr;
  register double sum = 0;
  register int i,j;

  for(i=0; i < ra.nrows; i++, pp += ra.inc_to_nextrow)
    for(j=0; j < ra.ncols; j++)		// Proceed scanline by scanline
      sum += *pp++;

  return sum;
}

/*
 *------------------------------------------------------------------------
 *		Operations on the rectangle area as a whole
 */

				// Copy a rectangle a_rect into the
				// rectangular area of the image
Rectangle& Rectangle::operator = (const Rectangle& a_rect)
{

  if( a_rect.nrows != nrows || a_rect.ncols != ncols )
    _error("Rectangles %dx%d and %dx%d are incompatible",
	   a_rect.nrows,a_rect.ncols,nrows,ncols);

  register GRAY * sp = a_rect.ptr;		// Source and
  register GRAY * dp = ptr;			// destination ptrs
  register int i;

  for(i=0; i < nrows; i++)
    memcpy(dp,sp,ncols*sizeof(GRAY)),
    sp += a_rect.inc_to_nextrow + ncols, 
    dp += inc_to_nextrow + ncols;

  return *this;
}
