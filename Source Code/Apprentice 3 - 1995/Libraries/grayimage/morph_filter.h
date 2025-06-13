// This may look like C code, but it is really -*- C++ -*-
/*
 ************************************************************************
 *
 *			   Grayscale Image
 *
 * 		  Morphological filtration of the image
 *	
 * Algorithm
 *	Described in the paper
 *	P.Maragos, Pattern Spectrum and Multiscale Shape Representation,
 *	IEEE Trans. Pattern Anal. Machine Intell., vol. 11, N7,
 *	p. 701-716, July 1989.
 *
 * The program implements the Function-Set-Processing (FSP) for the 
 * gray-scale images and boolean pattern. Refer to the Sec. II D of the
 * paper.
 *
 * Specifically, a dilation operation is defined as 
 *	dest = src PLUS pattern, i.e.
 * 	dest(k,l) = MAX{ src(k-i,l-j) } i=pattern.x(n), j=pattern.y(n)
 *	n=0..pattern.card,
 * dest and src being the images before and after the operation, resp.
 * On the other hand, erosion is
 *	dest = src MINUS pattern, i.e.
 *	dest(k,l) = MIN{ src(k+i,l+j) } i=pattern.x(n), j=pattern.y(n)
 *	n=0..pattern.card
 * Out-of-image pixels for src are assumed to be zeros
 *
 * Opening is the dilation followed by the erosion, and closing,
 * in contrast, is the erosion followed by the dilation.
 * It is easy to see that both erosion and dilation introduce some
 * phase shift in the image. Moreover, the shifts have the same size 
 * but different sign. That's why they get cancelled out in opening and
 * closing.
 *
 ************************************************************************
 */

#include "image.h"

#pragma interface

/*
 *------------------------------------------------------------------------
 *		Defining a class for the Binary Pattern
 * Convention:
 *	The entire pattern has to be located within the I quadrant, i.e.
 *	x(n) >= 0, y(n) >= 0 for each of the pattern points
 */

     				// Names for some standard patterns
enum PatternNames {Square, Octagon};

class BinaryPattern {

  int npoints;			// No. of points in the pattern
  int * xs;			// Pattern point abscissae
  int * ys;			// Pattern point ordinates

  void allocate(const int npoints);	// Allocate the pattern structure

public:
  BinaryPattern(const int npoints);	// Create an empty pattern
					// Create a pattern of a predefined
					// shape
  BinaryPattern(const PatternNames name,const int size);
  ~BinaryPattern();

					// Q. cardinality of the pattern
  int card(void) const			{ return npoints; }
  int x(const int pointn) const;	// Get the coordinate of a
  int y(const int pointn) const;	// specific pattern point

				// Pattern morphological operations
  friend void dilation(IMAGE& im, const BinaryPattern& pattern);
  friend void erosion(IMAGE& im, const BinaryPattern& pattern);

  friend void opening(IMAGE& im, const BinaryPattern& pattern);
  friend void closing(IMAGE& im, const BinaryPattern& pattern);
};

/*
 *------------------------------------------------------------------------
 *			   Inline Procedures
 */

				// Create an empty pattern
inline BinaryPattern::BinaryPattern(const int npoints)
{
  allocate(npoints);
}

				// Get the x coordinate of a specific
				// pattern point
inline int BinaryPattern::x(const int pointn) const
{
  if( pointn < 0 || pointn >= npoints )
    _error("The no. for the point requested %d is out of range [0,%d)",
	   pointn, npoints);
  return xs[pointn];
}

				// Get the y coordinate of the specific
				// pattern point
inline int BinaryPattern::y(const int pointn) const
{
  if( pointn < 0 || pointn >= npoints )
    _error("The no. for the point requested %d is out of range [0,%d)",
	   pointn, npoints);
  return ys[pointn];
}


inline void opening(IMAGE& im, const BinaryPattern& pattern)
{
  dilation(im,pattern);
  erosion(im,pattern);
}

inline void closing(IMAGE& im, const BinaryPattern& pattern)
{
  erosion(im,pattern);
  dilation(im,pattern);
}
