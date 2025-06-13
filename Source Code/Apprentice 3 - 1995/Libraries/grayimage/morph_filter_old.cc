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
 *	The program makes use of the Function-Set-Processing (FSP)
 *	for the gray-scale images and boolean pattern. Refer to the
 *	Sec. II D of the paper.
 *
 ************************************************************************
 */

#include "image.h"

#include <ostream.h>

#pragma implementation


/*
 *------------------------------------------------------------------------
 *		Defining a class for the Binary Pattern
 */

     				// Names for the some standard patterns
enum PatternNames {Square, Octagon};

class BinaryPattern {

  int npoints;			// No. of points in the pattern
  int * xs;			// Pattern point abscissae
  int * ys;			// Pattern point ordinates

  void allocate(const int npoints);	// Allocate the pattern structure

public:
  BinaryPattern(const int npoints);	// Create an empty pattern
					// Create a pattern of predefined
					// shape
  BinaryPattern(const PatternNames name,const int size);
  ~BinaryPattern();

					// Q. cardinality of the pattern
  int card(void) const			{ return npoints; }
  int x(const int pointn) const;	// Get the coordinate of the
  int y(const int pointn) const;	// specific pattern point
};

				// Allocate and clear arrays to hold
				// the specific no. of points
void BinaryPattern::allocate(const int npoints)
{
  assure(npoints >0, "Cannot allocate pattern with non-positive cardinality");
  BinaryPattern::npoints = npoints;
  assure((xs = calloc(npoints,sizeof(int))) != 0,"Out of memory");
  assure((ys = calloc(npoints,sizeof(int))) != 0,"Out of memory");
}

				// Create an empty pattern
inline BinaryPattern::BinaryPattern(const int npoints)
{
  allocate(npoints);
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

inline BinaryPattern::~BinaryPattern(void)
{
  assert( npoints > 0 && xs != 0 && ys != 0 );
  delete xs;
  delete ys;
}

				// Get the x coordinate of the specific
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

/*
 *------------------------------------------------------------------------
 *		Definition for the 2 fundamental operation
 *   dilation and erosion of the IMAGE with respect to the binary pattern
 */

				// Dilation operation
				// dest = src PLUS pattern, i.e.
				// dest(k,l) = MAX{ src(k-i,l-j) }
				//	i=pattern.x(n), j=pattern.y(n)
				//	n=0..pattern.card
				// Out-of-image pixels for src are
				// assumed to be zeros
void dilation(IMAGE& dest, const IMAGE& src, const BinaryPattern& pattern)
{
  are_compatible(dest,src);
  register int ri,ci;

  for(ri=0; ri<dest.q_nrows(); ri++)
    for(ci=0; ci<dest.q_ncols(); ci++)
    {
      register GRAY max_val = 0;
      register int n,i,j;
      for(n=0; n<pattern.card(); n++)
	if( (i=ri-pattern.y(n)) >= 0 && i < src.q_nrows() &&
	    (j=ci-pattern.x(n)) >= 0 && j < src.q_ncols() )
	  max_val = max(max_val,src(i,j));
        else
	  max_val = max(max_val,0);
	  

      dest(ri,ci) = max_val;
    }
}

				// Erosion operation
				// dest = src MINUS pattern, i.e.
				// dest(k,l) = MIN{ src(k+i,l+j) }
				//	i=pattern.x(n), j=pattern.y(n)
				//	n=0..pattern.card
				// Out-of-image pixels for src are
				// assumed to be zeros
void erosion(IMAGE& dest, const IMAGE& src, const BinaryPattern& pattern)
{
  are_compatible(dest,src);
  register int ri,ci;

  for(ri=0; ri<dest.q_nrows(); ri++)
    for(ci=0; ci<dest.q_ncols(); ci++)
    {
      register GRAY min_val = GRAY_MAXVAL;
      register int n,i,j;
      for(n=0; n<pattern.card(); n++)
	if( (i=ri+pattern.y(n)) >= 0 && i < src.q_nrows() &&
	    (j=ci+pattern.x(n)) >= 0 && j < src.q_ncols() )
	  min_val = min(min_val,src(i,j));
	else
	  min_val = min(min_val,0);

      dest(ri,ci) = min_val;
    }
}

#if 0
void test_filtration(IMAGE& im,const int filter_scale)
{
  IMAGE imt(im),imt1(im);
  cout << "\n-----------------------------\n";
  cout << "Opening/closing morphological filtration with square pattern " 
          "of size " << filter_scale << "\n";

  BinaryPattern pattern(Square,filter_scale);

  im.print("Original");
  erosion(imt,im,pattern);
  imt.print("Erosed");
  dilation(imt1,imt,pattern);
  imt1.print("Opened");
  dilation(imt,im,pattern);
  imt.print("Dilated");
  erosion(imt1,imt,pattern);
  imt1.print("Closed");
}

main()
{
  IMAGE im(7,7,8);

  im.clear();
  im.square_of(2,rowcol(2,2)) = 2;
  test_filtration(im,1);
  test_filtration(im,2);

  im.clear();
  im.square_of(3,rowcol(2,2)) = 2;
  test_filtration(im,2);
  test_filtration(im,3);

  im.clear();
  im.square_of(4,rowcol(2,2)) = 2;
  test_filtration(im,1);
  test_filtration(im,2);
  test_filtration(im,3);

  im(3,3) = 1; im(3,4) = 0;
  test_filtration(im,2);
  test_filtration(im,3);
}
#endif

void test_filtration(IMAGE& im,const int filter_scale)
{
  IMAGE imt(im),imt1(im);
  cout << "\n-----------------------------\n";
  cout << "Opening/closing morphological filtration with square pattern " 
          "of size " << filter_scale << "\n";

  BinaryPattern pattern(Square,filter_scale);

  im.display("Original");
  erosion(imt,im,pattern);
  imt.display("Erosed");
  dilation(imt1,imt,pattern);
  imt1.display("Opened");
  dilation(imt,im,pattern);
  imt.display("Dilated");
  erosion(imt1,imt,pattern);
  imt1.display("Closed");
}

main()
{
  IMAGE im("../old_images/text.xwd");
  im.invert();

  test_filtration(im,2);
  test_filtration(im,3);
//  test_filtration(im,4);
}

