// This may look like C code, but it is really -*- C++ -*-
/*
 ************************************************************************
 *
 *			   Grayscale Image
 *
 * 		  Morphological filtration of the image
 *	
 *	The program implements the Function-Set-Processing (FSP)
 *	for the gray-scale images and boolean patterns. Refer to the
 *	Sec. II D of the paper.
 *
 * Note, the program expects the binary pattern to lie entirely into the
 * I quadrant, i.e. abscissae and ordinates of every pattern point are
 * both non-negative. It makes possible to implement erosion and dilation,
 * two fundamental morphological operations, inplace.
 *
 ************************************************************************
 */

#include "morph_filter.h"
#include <builtin.h>

#pragma implementation


/*
 *------------------------------------------------------------------------
 *			Binary Pattern service operations
 *		
 */

				// Allocate and clear arrays to hold
				// the specific no. of points
void BinaryPattern::allocate(const int _npoints)
{
  npoints = _npoints;
  assure(npoints >0, "Cannot allocate pattern with non-positive cardinality");
  assure((xs = (int *)calloc(npoints,sizeof(int))) != 0,"Out of memory");
  assure((ys = (int *)calloc(npoints,sizeof(int))) != 0,"Out of memory");
}
					// Create a pattern of a predefined
					// shape
BinaryPattern::BinaryPattern(const PatternNames name,const int size)
{
  assure(size > 0,"Pattern size has got to be positive");

  if( name == Square )
  {
    allocate(sqr(size));
    register int i,j;
    register int *xp = &xs[0];
    register int *yp = &ys[0];

    for(i=0; i<size; i++)
      for(j=0; j<size; j++)
	*xp++ = i, *yp++ = j;

    assert( xp-&xs[0] == npoints && yp-&ys[0] == npoints );
  }
  else
    _error("Sorry, standard pattern %d hasn't implemented yet",name);
}

				// Destructor
BinaryPattern::~BinaryPattern(void)
{
  assert( npoints > 0 && xs != 0 && ys != 0 );
  delete xs;
  delete ys;
}

/*
 *------------------------------------------------------------------------
 *			Dilation of the image inplace
 * dest(k,l) = MAX{ src(k-i,l-j) } i=pattern.x(n), j=pattern.y(n)
 * n=0..pattern.card
 * Out-of-image pixels for src are assumed to be zeros.
 * If pattern.x(n) >= 0 and pattern.y(n) >= 0, the operation can be
 * performed inplace. Indeed, dilation of the point (k,l) of the image
 * requires the (k,l)-th point itself and some pixels to the left and
 * to the top of the current point. So, the image has to be scanned
 * from the right to the left and from the bottom to the top, and
 * the current point of the image can be substituted with its dilated
 * value
 */

void dilation(IMAGE& image, const BinaryPattern& pattern)
{
  image.is_valid();
  register int k,l;

  for(k=image.q_nrows()-1; k>=0; k--)		// From bottom to the top
    for(l=image.q_ncols()-1; l>=0; l--)		// and from right to left
    {
      register GRAY max_val = image(k,l);
      register int n,i,j;
      for(n=0; n<pattern.card(); n++)
	if( (i=k-pattern.y(n)) >= 0 && (j=l-pattern.x(n)) >= 0 )
	  max_val = max(max_val,image(i,j));
        else
	  max_val = max(max_val,0);
	  

      image(k,l) = max_val;
    }
}

/*
 *------------------------------------------------------------------------
 *			Erosion of the image inplace
 * dest(k,l) = MIN{ src(k+i,l+j) } i=pattern.x(n), j=pattern.y(n)
 * n=0..pattern.card
 * Out-of-image pixels for src are assumed to be zeros
 * If pattern.x(n) >= 0 and pattern.y(n) >= 0, the operation can be
 * performed inplace. Indeed, erosion of the point (k,l) of the image
 * requires the (k,l)-th point itself and some pixels to the right and
 * to the bottom of the current point. So, the image has to be scanned
 * from left to right and from the top to the bottom, and
 * the current point of the image can be substituted with its erosed
 * value
 */

void erosion(IMAGE& image, const BinaryPattern& pattern)
{
  image.is_valid();
  register int k,l;

  for(k=0; k<image.q_nrows(); k++)		// From bottom to the top
    for(l=0; l<image.q_ncols(); l++)		// and from right to left
    {
      register GRAY min_val = image(k,l);
      register int n,i,j;
      for(n=0; n<pattern.card(); n++)
	if( (i=k+pattern.y(n)) < image.q_nrows()  && 
	    (j=l+pattern.x(n)) < image.q_ncols() )
	  min_val = min(min_val,image(i,j));
        else
	  min_val = min(min_val,0);
	  

      image(k,l) = min_val;
    }
}

