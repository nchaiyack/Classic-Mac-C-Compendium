// This may look like C code, but it is really -*- C++ -*-
/*
 ************************************************************************
 *
 *			   Grayscale Image
 *
 *		Perform the median filtration of the image
 *	
 * Algorithm
 *	For every pixel of the source image x[i,j] compute the
 *	pixel y[i,j] of the filtrated image according to the
 *	rule
 *	y[i,j] = median_of( x[i+p,j+l] ), p,l = -ws...0...ws
 *
 *	where 2*ws+1 is the window size. The median of the sequence
 *	is the middle term of the ordered sequence (in our case the
 *	sequence always contains the odd number of terms).
 *	The pixels "outside" the image are considered to be zeros,
 *	i.e.
 *		x[k,l] = 0 if k,l < 0 or k,l >= Image_dimension
 *
 ************************************************************************
 */

#include "image.h"

#pragma implementation

/*
 *------------------------------------------------------------------------
 *		Finding a median of the sequence of GRAYs
 * 
 * Algorithm
 *	Described in the book "Numerical Recepies".
 * The algorithm does not require sorting the sequence. The main idea is
 * that if the median of the sequence is a "middle" term of the sequence,
 * the following relation should hold
 *
 *	SUM{ (xmed - x[i]) / |xmed - x[i]| } = 0
 *
 */

static GRAY find_median(GRAY seq[], const int no_elems)
{
}

/*
 *------------------------------------------------------------------------
 *		 Root module of the median filtration
 */

			// Median filter the image src with the window
			// half-size whs to get the image dest
void median_filter(IMAGE& dest, const IMAGE& src, const int whs)
{
  are_compatible(dest,src);
  assure(whs > 0, "Window-half size should be at least 1");
  const int wsize = sqr(2*whs+1);
  GRAY window[wsize];				// Filtration window
  register int ri,ci;

  for(ri=0; ri<dest.q_nrows(); ri++)
    for(ci=0; ci<dest.q_ncols(); ci++)
    {
      register int i,j;
      register GRAY * wp = &window[0];

      for(i=ri-whs; i<=ri+whs; i++)
	for(j=ci-whs; j<=ci+whs; j++)
	  if( i<0 || j<0 || i>=dest.q_nrows() || j>=dest.q_ncols() )
	    *wp++ = 0;
          else
	    *wp++ = src(i,j);
      dest(ri,ci) = find_median(window,wsize);	// Write down the median
    }
}
