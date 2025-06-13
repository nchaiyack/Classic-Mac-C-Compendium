// This may look like C code, but it is really -*- C++ -*-
/*
 ************************************************************************
 *
 *			   Grayscale Image
 *
 *		Apply various filtration techniques to the image
 *	
 * Algorithm
 *	Median Filtration
 *	For every pixel of the source image x[k,l] compute the
 *	pixel y[k,l] of the filtrated image according to the
 *	rule
 *	y[k,l] = median_of( x[k+i,l+j] ), i,j = -ws...0...ws
 *
 *	where 2*ws+1 is the window size. The median of the sequence
 *	is the middle term of the ordered sequence (in our case the
 *	sequence always contains the odd number of terms).
 *	The pixels "outside" the image are assumed to be zeros,
 *	i.e.
 *		x[k,l] = 0 if k,l < 0 or k,l >= Image_dimension
 *
 *	Quartile Filtration
 *	Pretty much the same as the median filtration except the
 *	filtrated pixel is obtained as the 3d quartile of the
 *	sequence of the source image pixels in the filter window
 *	minus the background value
 *	y[k,l] = 3d_quartile( x[k+i,l+j] - backgrnd ) + backgrnd, 
 *						i,j = -ws...0...ws
 *
 *	where 2*ws+1 is the window size. The 3d-quartile of the n-point
 *	sequence is the (3n+1)/4-th term of the sequence arranged in the
 *	ascending orderof abs values. Note, in our case the sequence 
 *	always contains the odd number of terms.
 *	The background value is obtained as the mean arithmetical value
 *	over the window of 2*(ws+1) size rounded to the closest integer.
 *
 *	Square filtration (Not Implemented yet)
 *	Given the square with size d, transform the source image
 *	(array of pixels x[k,l]) into the filtrated one according
 *	to the rule
 *	y[k,l] = int_average_of_sequence( x[k+i, l+j] )
 *
 ************************************************************************
 */

#include "image.h"
#include "builtin.h"

#include <ostream.h>

#pragma implementation


/*
 *------------------------------------------------------------------------
 *			    Service functions
 */

			// Compute round(a/b)
inline int div_round(const int a, const int b)
{
  if( 2*(a % b) >= b )		// Remainder is larger than the half of
    return a/b + 1;		// the divisor
  else
    return a/b;
}

#if 1
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

				// Partially sorting 'window' in acsending
				// order to obtain the median value
				// A customary bubble-sort algorithm is
				// made use of.
				// Every step of the algorithm percolates
				// i-th largest element to the end of the
				// 'window'. Once (wsize+1)/2 largest elements
				// have been percolated, the process
				// can be terminated - the median is found.
      for(i=1; i<=(wsize+1)/2; i++)
      {
	register int was_swap = 0;
	for(wp=&window[0]; wp<&window[wsize-i]; wp++)
	  if( *wp > *(wp+1) )
	  {
	    register GRAY t = *(wp+1);
	    *(wp+1) = *wp;
	    *wp = t;
	    was_swap = 1;
	  }
	if( !was_swap )
	  break;
      }

      dest(ri,ci) = window[(wsize-1)/2];	// Write down the median
    }
}
#endif


#if 0
			// Quartile filter the image src with the window
			// half-size whs to get the image dest
void quartile_filter(IMAGE& dest, const IMAGE& src, const int whs)
{
  are_compatible(dest,src);

  assure(whs > 0, "Window-half size should be at least 1");
  const int warea = sqr(2*whs+1);
  int window[warea];				// Filtration window
  const int bwhs = whs+1;			// Background window half-size
  const int bwarea = sqr(2*bwhs+1);		// and its area
  register int ri,ci;

  for(ri=0; ri<dest.q_nrows(); ri++)
    for(ci=0; ci<dest.q_ncols(); ci++)
    {
      register int i,j;
      register int bgvalue = 0;

      for(i=max(ri-bwhs,0); i<=min(ri+bwhs,dest.q_nrows()-1); i++)
	for(j=max(ci-bwhs,0); j<=min(ci+bwhs,dest.q_ncols()-1); j++)
	  bgvalue += src(i,j);
      bgvalue = div_round(bgvalue,bwarea);
message("\nri %d, ci %d, bgvalue = %d", ri, ci, bgvalue);

      register int * wp = &window[0];
      for(i=max(ri-whs,0); i<=min(ri+whs,dest.q_nrows()-1); i++)
	for(j=max(ci-whs,0); j<=min(ci+whs,dest.q_ncols()-1); j++)
	  *wp++ = src(i,j) - bgvalue;
      while( wp < &window[warea] )
	*wp++ = 0-bgvalue;		// where 0
      					// is the value for out-of-image pixels

				// Partially sorting 'window' in acsending
				// order to obtain the 3d quartile
				// A customary bubble-sort algorithm is
				// made use of.
				// Every step of the algorithm percolates
				// i-th largest element to the end of the
				// 'window'. Once (warea-1)/4+1 largest 
				// elements have been percolated, the process
				// can be terminated - the quartile is found.
      for(i=1; i<=(warea+3)/4; i++)
      {
	register int was_swap = 0;
	for(wp=&window[0]; wp<&window[warea-i]; wp++)
	  if( abs(*wp) > abs(*(wp+1)) )
	  {
	    register GRAY t = *(wp+1);
	    *(wp+1) = *wp;
	    *wp = t;
	    was_swap = 1;
	  }
	if( !was_swap )
	  break;
      }

for(wp=&window[0]; wp<&window[warea]; wp++)
  message(" %d",*wp);

      dest(ri,ci) = window[3*(warea-1)/4] + bgvalue; // Write down the quartile
    }
}
#endif

#if 0
void test_filtration(IMAGE& im,const int filter_scale)
{
  IMAGE imt(im);
  cout << "\n-----------------------------\n";
  cout << "Median filtration with half-window size " << filter_scale
       << "\n";

  im.print("Original");
  median_filter(imt,im,filter_scale);
  imt.print("Filtrated");
}

main()
{
  IMAGE im(7,7,8);

#if 0  
  register int i;
  for(i=0; i<im.q_ncols(); i++)
    im(3,i) = 2, im(5,i) = 3;
#endif

//  im.square_of(3,rowcol(2,2)) = 2;
//  im(2,3) = 0; im(4,3) = 0; im(4,5) = 0;

  im.square_of(2,rowcol(2,2)) = 2;
  test_filtration(im,1);

  im.square_of(3,rowcol(2,2)) = 2;
  test_filtration(im,1);

  im.square_of(3,rowcol(2,2)) = 2;
  test_filtration(im,2);

  im.square_of(4,rowcol(2,2)) = 2;
  test_filtration(im,1);

  im.square_of(4,rowcol(2,2)) = 2;
  test_filtration(im,2);
}

#endif
