// This may look like C code, but it is really -*- C++ -*-
/*
 ************************************************************************
 *
 *			   Grayscale Image
 * 		Implementation of the Primitive Operations
 *
 *   The image is represented as a Pixmap, i.e. a matrix of pixels
 *    each of them specifies the gray level at a particular point
 *
 ************************************************************************
 */

#pragma implementation "image.h"
#include "image.h"
#include <builtin.h>




/*
 *------------------------------------------------------------------------
 *			Constructors and destructors
 */

void IMAGE::allocate(
	const int no_rows,		// No. of rows
	const int no_cols,		// No. of cols
	const int depth			// No. of bits per pixel
)
{
  valid_code = IMAGE_val_code;

  assure((ncols=no_cols) > 0, "Zero image width unexpected");
  assure((nrows=no_rows) > 0, "Zero image height unexpected");
  assure((bits_per_pixel=depth) > 0 && depth <= GRAY_MAXBIT, 
	 "Zero or too large no. of bits per pixel");

  name    = "";
  npixels = nrows * ncols;

  assert( (scanrows = (GRAY **)calloc(nrows,sizeof(GRAY *))) != 0 );
  assert( (pixels   = (GRAY *)calloc(npixels,sizeof(GRAY))) != 0 );

  register int i;
  for(i=0; i<nrows; i++)
    scanrows[i] = &pixels[i*ncols];
}

				// Set a new image name
void IMAGE::set_name(const char * new_name)
{
  if( name != 0 && name[0] != '\0' )	// Dispose of the previous image name
    delete name;

  if( new_name == 0 || new_name[0] == '\0' )
    name = "";				// Image is anonymous now
  else
    name = new char[strlen(new_name)+1], strcpy(name,new_name);
}

					// Routing constructor module
IMAGE::IMAGE(const IMAGE_CREATORS_1op op, const IMAGE& prototype)
{
  switch(op)
  {
    case Expand:
         _expand(prototype);
	 break;

    case Shrink:
	 _shrink(prototype);
	 break;

    default:
	 _error("Operation %d is not yet implemented",op);
  }
}


IMAGE::~IMAGE(void)		// Dispose the image struct
{
  is_valid();
  if( name != 0 && name[0] != '\0' )
    delete name;

  delete scanrows;
  delete pixels;
  valid_code = 0;
}


/*
 *------------------------------------------------------------------------
 * 			Single Image operations
 */

				// Clip pixel values to
				// [0,1<<bits_per_pixel-1].
				// A pixel with the value outside that range
				// (i.e. negative or too big) is set to
				// 0 or 1<<bits_per_pixel-1, resp.
IMAGE& IMAGE::clip_to_intensity_range(void)
{
  is_valid();
  const int maxval = (1<<bits_per_pixel)-1;
  register GRAY_SIGNED * cp = (GRAY_SIGNED *)pixels;
  for(; cp < (GRAY_SIGNED *)pixels + npixels; cp++)
    if( *cp < 0 )
      *cp = 0;
    else if( *cp > maxval )
      *cp = maxval;

  return *this;
}

				// Perform a transformation
				// pixel = |(signed)pixel|
				// on all the pixels of the image
IMAGE& IMAGE::abs(void)
{
  is_valid();
  register GRAY_SIGNED * cp = (GRAY_SIGNED *)pixels;
  for(; cp < (GRAY_SIGNED *)pixels + npixels; cp++)
    if( *cp < 0 )
      *cp = -(*cp);

  return *this;
}

				// Perform a transformation
				// pixel = fp((signed)pixel)
				// on all the pixels of the image
				// where fp is a user-defined function
IMAGE& IMAGE::apply(USER_F * fp)
{
  is_valid();
  register GRAY_SIGNED * cp = (GRAY_SIGNED *)pixels;
  for(; cp < (GRAY_SIGNED *)pixels + npixels; cp++)
    *cp = fp(*cp);

  return *this;
}

				// Perform the histogram equalization
IMAGE& IMAGE::equalize(const int no_grays)
{
  is_valid();
  int orig_no_grays = 1 << bits_per_pixel;
  assert( no_grays <= orig_no_grays );

  int histogram [orig_no_grays];
  memset(histogram,0,sizeof(histogram));

				// Evaluate the histogram of an image
  register GRAY_SIGNED * cp = (GRAY_SIGNED *)pixels;
  for(; cp < (GRAY_SIGNED *)pixels + npixels; cp++)
  {
    if( *cp <= 0 )
      *cp = 0;
    else if( *cp >= orig_no_grays )
      *cp = orig_no_grays-1;
    histogram[*cp]++;
  }

  const int pixels_per_bin_optimal = (npixels + no_grays - 1)/no_grays;
  const int gray_shade_subsample_factor = 
    (orig_no_grays + no_grays -1)/no_grays;
  register int pixel;
  int new_pixel, new_pixel_old;
  int accumulated = 0;
  int optimal_accumulated = pixels_per_bin_optimal;
					// Mapping from pixels of original
  short look_up_table[orig_no_grays];	// image to [new_pixel_old,new_pixel]
					// (actually, just to the center of
					// this interval)

  					// Equalizing the histogram
  for(pixel=0,new_pixel=0,new_pixel_old=0; pixel<orig_no_grays; pixel++)
  {
    accumulated += histogram[pixel];
    while( accumulated > optimal_accumulated )
      new_pixel += gray_shade_subsample_factor, 
      optimal_accumulated += pixels_per_bin_optimal;
    assert( new_pixel < orig_no_grays );
    look_up_table[pixel] = (new_pixel > 
			    new_pixel_old+gray_shade_subsample_factor ?
			    (new_pixel + new_pixel_old)/2 :
			    new_pixel);
    new_pixel_old = new_pixel;
  }

				// Update the image according to the LUT
  for(cp = (GRAY_SIGNED *)pixels; cp < (GRAY_SIGNED *)pixels + npixels; cp++)
    *cp = look_up_table[*cp];

  return *this;
}

				// Compute the 1. norm of the entire image
				// SUM{ |(signed)pixel[i,j]| }
double IMAGE::norm_1(void) const
{
  is_valid();
  register GRAY_SIGNED * cp = (GRAY_SIGNED *)pixels;
  long long sum = 0;
  for(; cp < (GRAY_SIGNED *)pixels + npixels; )
    sum += ::abs(*cp++);

  return sum;
}

				// Compute the square of the 2. norm of 
				// the entire image
				// SUM{ |(signed)pixel[i,j]|^2 }
double IMAGE::norm_2_sqr(void) const
{
  is_valid();
  register GRAY_SIGNED * cp = (GRAY_SIGNED *)pixels;
  register double sum = 0;
  for(; cp < (GRAY_SIGNED *)pixels + npixels; )
    sum += ::sqr(*cp++);

  return sum;
}

				// Compute the infinity norm of the
				// entire image
				// MAX{ |(signed)pixel[i,j]| }
int IMAGE::norm_inf(void) const
{
  is_valid();
  register GRAY_SIGNED * cp = (GRAY_SIGNED *)pixels;
  register int maxp = 0;
  for(; cp < (GRAY_SIGNED *)pixels + npixels; )
    maxp = ::max(::abs(*cp++),maxp);

  return maxp;
}

				// Find extremum values of image pixels
Extrema::Extrema(const IMAGE& image)
	: max_pixel(0,0), min_pixel(0,0)
{
  image.is_valid();
  max_value = min_value = image(0,0);

  register GRAY_SIGNED * cp = (GRAY_SIGNED *)image.pixels;
  register int i,j;
  for(i=0; i<image.nrows; i++)
    for(j=0; j<image.ncols; j++, cp++)
      if( *cp > max_value )
	max_value = *cp, max_pixel.row_val = i, max_pixel.col_val = j;
      else if( *cp < min_value )
	min_value = *cp, min_pixel.row_val = i, min_pixel.col_val = j;
}

				// Normalize pixel values to be
				// in range 0..1<<bits_per_pixel-1
IMAGE& IMAGE::normalize_for_display(void)
{
  is_valid();
  Extrema extrema(*this);
  message("\nImage_normalization:"
	  "\n\tmin pixel value %d, max pixel value %d",
	  extrema.min(), extrema.max());
  if( extrema.max() != extrema.min() )
  {
    double factor = (double)((1<<bits_per_pixel)-1) /
		    ( extrema.max() - extrema.min() );
    message("\n\tNormalization is as follows: (pixel - %d)*%g\n",
	    extrema.min(), factor);
    register GRAY_SIGNED * cp = (GRAY_SIGNED *)pixels;
    for(; cp < (GRAY_SIGNED *)pixels + npixels; cp++)
      *cp = (int)( (*cp - extrema.min()) * factor + 0.5 );
  }

  return *this;
}

				// Print some info about the image
void IMAGE::info(void) const
{
  message("\nimage %dx%dx%d '%s' ",nrows,ncols,bits_per_pixel,name);
}
				// Print the image as a table of pixel values
				// (zeros are printed as dots)
void IMAGE::print(const char * title) const
{
  is_valid();
  message("\nImage %dx%dx%d '%s' is as follows\n\n",nrows,ncols,
	  bits_per_pixel,title);

  register int i,j;
  for(i=0; i<nrows; i++)
  {
    for(j=0; j<ncols; j++)
      if( (*this)(i,j) == 0 )
	message("   . ");
      else
	message("%4d ",(*this)(i,j));
    message("\n");
  }
  message("Done\n");
}

/*
 *------------------------------------------------------------------------
 * 			Image-scalar operations
 * 	  Check to see if the preedicate "(signed)pixel operation scalar"
 *		holds for ALL the pixels of the image
 */

				// (signed)pixel == val for all pixels?
int IMAGE::operator == (const int val) const
{
  is_valid();
  register GRAY_SIGNED * cp = (GRAY_SIGNED *)pixels;
  for(; cp < (GRAY_SIGNED *)pixels + npixels; )
    if( !(*cp++ == val) )
      return 0;

  return 1;
}

				// (signed)pixel != val for all pixels?
int IMAGE::operator != (const int val) const
{
  is_valid();
  register GRAY_SIGNED * cp = (GRAY_SIGNED *)pixels;
  for(; cp < (GRAY_SIGNED *)pixels + npixels; )
    if( !(*cp++ != val) )
      return 0;

  return 1;
}

				// (signed)pixel <  val for all pixels?
int IMAGE::operator < (const int val) const
{
  is_valid();
  register GRAY_SIGNED * cp = (GRAY_SIGNED *)pixels;
  for(; cp < (GRAY_SIGNED *)pixels + npixels; )
    if( !(*cp++ < val) )
      return 0;

  return 1;
}

				// (signed)pixel <= val for all pixels?
int IMAGE::operator <= (const int val) const
{
  is_valid();
  register GRAY_SIGNED * cp = (GRAY_SIGNED *)pixels;
  for(; cp < (GRAY_SIGNED *)pixels + npixels; )
    if( !(*cp++ <= val) )
      return 0;

  return 1;
}

				// (signed)pixel > val for all pixels?
int IMAGE::operator > (const int val) const
{
  is_valid();
  register GRAY_SIGNED * cp = (GRAY_SIGNED *)pixels;
  for(; cp < (GRAY_SIGNED *)pixels + npixels; )
    if( !(*cp++ > val) )
      return 0;

  return 1;
}

				// (signed)pixel >= val for all pixels?
int IMAGE::operator >= (const int val) const
{
  is_valid();
  register GRAY_SIGNED * cp = (GRAY_SIGNED *)pixels;
  for(; cp < (GRAY_SIGNED *)pixels + npixels; )
    if( !(*cp++ >= val) )
      return 0;

  return 1;
}

/*
 *------------------------------------------------------------------------
 * 			Image-scalar operations
 * 	     Modify every pixel according to the operation
 */

				// Assign a value to all the pixels
IMAGE& IMAGE::operator = (const int val)
{
  is_valid();
  register GRAY * cp = pixels;
  while( cp < pixels+npixels )
    *cp++ = val;

  return *this;
}

				// Add to all pixels
IMAGE& IMAGE::operator += (const int val)
{
  is_valid();
  register GRAY * cp = pixels;
  while( cp < pixels+npixels )
    *cp++ += val;

  return *this;
}

				// Subtract a value from all the pixels
IMAGE& IMAGE::operator -= (const int val)
{
  is_valid();
  register GRAY * cp = pixels;
  while( cp < pixels+npixels )
    *cp++ -= val;

  return *this;
}

				// Multiply a value by all the pixels
IMAGE& IMAGE::operator *= (const int val)
{
  is_valid();
  register GRAY * cp = pixels;
  while( cp < pixels+npixels )
    *cp++ *= val;

  return *this;
}

				// Logical OR a value by all the pixels
IMAGE& IMAGE::operator |= (const int val)
{
  is_valid();
  register GRAY * cp = pixels;
  while( cp < pixels+npixels )
    *cp++ |= val;

  return *this;
}

				// Logical AND a value by all the pixels
IMAGE& IMAGE::operator &= (const int val)
{
  is_valid();
  register GRAY * cp = pixels;
  while( cp < pixels+npixels )
    *cp++ &= val;

  return *this;
}

				// Logical XOR a value by all the pixels
IMAGE& IMAGE::operator ^= (const int val)
{
  is_valid();
  register GRAY * cp = pixels;
  while( cp < pixels+npixels )
    *cp++ ^= val;

  return *this;
}

				// Shift the value of all the pixels
IMAGE& IMAGE::operator <<= (const int val)
{
  is_valid();
  if( ::abs(val) >= GRAY_MAXBIT )
    _error("Very fishy shift factor: %d",val);

  register GRAY * cp = pixels;
  while( cp < pixels+npixels )
    *cp++ <<= val;

  return *this;
}

				// Shift the value of all the pixels
IMAGE& IMAGE::operator >>= (const int val)
{
  is_valid();
  if( ::abs(val) >= GRAY_MAXBIT )
    _error("Very fishy shift factor: %d",val);

  register GRAY * cp = pixels;
  while( cp < pixels+npixels )
    *cp++ >>= val;

  return *this;
}

/*
 *------------------------------------------------------------------------
 * 			Image-Image operations
 * 	     Modify the target image according to the operation
 */

IMAGE& IMAGE::operator = (const IMAGE& source)
{
  are_compatible(*this,source);
  memcpy(pixels,source.pixels,npixels*sizeof(GRAY));
  return *this;
}

int identical(const IMAGE& im1, const IMAGE& im2)
{
  are_compatible(im1,im2);
  return (memcmp(im1.pixels,im2.pixels,im1.npixels*sizeof(GRAY)) == 0);
}

				// Add the source to the target
IMAGE& operator += (IMAGE& target, const IMAGE& source)
{
  are_compatible(target,source);

  register GRAY * sp = source.pixels;
  register GRAY * tp = target.pixels;
  while( tp < target.pixels + target.npixels )
    *tp++ += *sp++;
  
  return target;
}
  
				// Subtract the source from the target
IMAGE& operator -= (IMAGE& target, const IMAGE& source)
{
  are_compatible(target,source);

  register GRAY * sp = source.pixels;
  register GRAY * tp = target.pixels;
  while( tp < target.pixels + target.npixels )
    *tp++ -= *sp++;
  
  return target;
}

				// Modified addition
				//	Target += scalar*Source
IMAGE& add(IMAGE& target, const int scalar,const IMAGE& source)
{
  are_compatible(target,source);

  register GRAY * sp = source.pixels;
  register GRAY * tp = target.pixels;
  while( tp < target.pixels + target.npixels )
    *tp++ += scalar * *sp++;
  
  return target;
}
				// Shift the source to 'pos', clip it 
				// if necessary, multiply by the scalar,
				// and add
IMAGE& IMAGE::shift_clip_add(rowcol pos, const int scalar, const IMAGE& source)
{
  source.is_valid();
  is_valid();

					// Find an intersection of 'source'
					// shifted to 'pos' (relative to
					// the image) with the (target) image
					// Only this rectangle will be 
					// affected by addition
  rowcol s_orig(max(0,-pos.row()),max(0,-pos.col()));
  rowcol t_orig(max(0,pos.row()),max(0,pos.col()));
					// No. of rows in the intersection
  const int irows = min(nrows - t_orig.row(),source.nrows - s_orig.row());
					// No. of cols in the intersection
  const int icols = min(ncols - t_orig.col(),source.ncols - s_orig.col());
  if( irows <= 0 || icols <= 0 )
    source.info(),
    info(),
    _error("The first image shifted by (%d,%d) does not intersect the other",
	   pos.row(),pos.col());

					// See the explanation in the class
					// Rectangle
  const int s_inc_to_nextrow = source.ncols - icols;	
  const int t_inc_to_nextrow = ncols - icols;	

  register GRAY_SIGNED * sp = 
    (GRAY_SIGNED *)&(source.scanrows[s_orig.row()])[s_orig.col()];
  register GRAY_SIGNED * tp = 
    (GRAY_SIGNED *)&(scanrows[t_orig.row()])[t_orig.col()];

  register int i,j;
  for(i=0; i<irows; i++, sp += s_inc_to_nextrow, tp += t_inc_to_nextrow)
    for(j=0; j<icols; j++)
      *tp++ += scalar * *sp++;

  return *this;
}

				// Evaluate the scalar product of two images
double operator * (const IMAGE& im1, const IMAGE& im2)
{
  are_compatible(im1,im2);

  register GRAY_SIGNED * p1 = (GRAY_SIGNED *)im1.pixels;
  register GRAY_SIGNED * p2 = (GRAY_SIGNED *)im2.pixels;
  register double sum = 0;

  while( p1 < (GRAY_SIGNED *)im1.pixels + im1.npixels )
    sum += *p1++ * *p2++;

  return sum;
}

			// Estimate the norm of the difference 
			// between two (signed) image
				// SUM{ |(signed)pixel[i,j]| }
double norm_1(const IMAGE& im1, const IMAGE& im2)
{
  are_compatible(im1,im2);

  register GRAY_SIGNED * p1 = (GRAY_SIGNED *)im1.pixels;
  register GRAY_SIGNED * p2 = (GRAY_SIGNED *)im2.pixels;
  register long long sum = 0;

  while( p1 < (GRAY_SIGNED *)im1.pixels + im1.npixels )
    sum += ::abs(*p1++ - *p2++);

  return sum;
}

				// SUM{ |(signed)pixel[i,j]|^2 }
double norm_2_sqr(const IMAGE& im1, const IMAGE& im2)
{
  are_compatible(im1,im2);

  register GRAY_SIGNED * p1 = (GRAY_SIGNED *)im1.pixels;
  register GRAY_SIGNED * p2 = (GRAY_SIGNED *)im2.pixels;
  register double sum = 0;

  while( p1 < (GRAY_SIGNED *)im1.pixels + im1.npixels )
    sum += ::sqr(*p1++ - *p2++);

  return sum;
}
				// MAX{ |(signed)pixel[i,j]| }
int norm_inf(const IMAGE& im1, const IMAGE& im2)
{
  are_compatible(im1,im2);

  register GRAY_SIGNED * p1 = (GRAY_SIGNED *)im1.pixels;
  register GRAY_SIGNED * p2 = (GRAY_SIGNED *)im2.pixels;
  register int maxp = 0;

  while( p1 < (GRAY_SIGNED *)im1.pixels + im1.npixels )
    maxp = max(maxp,::abs(*p1++ - *p2++));

  return maxp;
}

				// OR the source to the target
IMAGE& operator |= (IMAGE& target, const IMAGE& source)
{
  are_compatible(target,source);

  register GRAY * sp = source.pixels;
  register GRAY * tp = target.pixels;
  while( tp < target.pixels + target.npixels )
    *tp++ |= *sp++;
  
  return target;
}

				// AND the source to the target
IMAGE& operator &= (IMAGE& target, const IMAGE& source)
{
  are_compatible(target,source);

  register GRAY * sp = source.pixels;
  register GRAY * tp = target.pixels;
  while( tp < target.pixels + target.npixels )
    *tp++ &= *sp++;
  
  return target;
}

				// XOR the source to the target
IMAGE& operator ^= (IMAGE& target, const IMAGE& source)
{
  are_compatible(target,source);

  register GRAY * sp = source.pixels;
  register GRAY * tp = target.pixels;
  while( tp < target.pixels + target.npixels )
    *tp++ ^= *sp++;
  
  return target;
}

#include <builtin.h>
void compare(			// Compare the two images
	const IMAGE& image1,	// and print out the result of comparison
	const IMAGE& image2,
	const char * title )
{
  register int i,j;

  are_compatible(image1,image2);

  message("\n\nComparison of two images %dx%dx%d\n%s\n",image1.nrows,
	 image1.ncols,image1.bits_per_pixel,title);
 
  if( image1.bits_per_pixel != image2.bits_per_pixel )
    message("\nNote, images have different depth, %d and %d\n",
	     image1.bits_per_pixel, image2.bits_per_pixel);

  long long int norm1 = 0, norm2 = 0;	// Norm of the images
  long long int ndiff = 0;		// Norm of the difference
  int imax=0,jmax=0,difmax = -1;	// For the pixels that differ most
					// Image scanline pointer
  register GRAY_SIGNED *rowp1 = (GRAY_SIGNED *)image1.pixels;
  register GRAY_SIGNED *rowp2 = (GRAY_SIGNED *)image2.pixels;

  int im1_neg_pixel = 0;		// Flags whether negative pixels
  int im2_neg_pixel = 0;		// were encountered

  for(i=0; i < image1.nrows; i++)
    for(j=0; j < image1.ncols; j++)
    {
      int pv1 = *rowp1++;
      int pv2 = *rowp2++;
      int diff = abs(pv1-pv2);

      if( pv1 < 0 )
	im1_neg_pixel = 1, pv1 = -pv1;
      if( pv2 < 0 )
	im2_neg_pixel = 1, pv2 = -pv2;

      if( diff > difmax )
      {
	difmax = diff;
	imax = i;
	jmax = j;
      }
      norm1 += pv1;
      norm2 += pv2;
      ndiff += diff;
    }

  if( im1_neg_pixel )
    message("\n*** Warning: a pixel with negative value has been encountered "
	    "in image 1\n");
  if( im2_neg_pixel )
    message("\n*** Warning: a pixel with negative value has been encountered "
	    "in image 2\n");

  message("\nMaximal discrepancy    \t\t%d",difmax);
  message("\n   occured at the pixel\t\t(%d,%d)",imax,jmax);
  const int pv1 = image1(imax,jmax);
  const int pv2 = image2(imax,jmax);
  message("\n Image 1 pixel is    \t\t%d",pv1);
  message("\n Image 2 pixel is    \t\t%d",pv2);
  message("\n Absolute error v2[i]-v1[i]\t\t%d",pv2-pv1);
  message("\n Relative error\t\t\t\t%g\n",
	 (pv2-pv1)/max(abs(pv2+pv1)/2,1e-7) );

  message("\nL1 norm of image 1 per pixel \t\t%g",
	  				(double)norm1/image1.npixels);
  message("\nL1 norm of image 2 per pixel \t\t%g",
					(double)norm2/image2.npixels);
  message("\nL1 norm of image1-image2 per pixel\t\t\t%g",
					(double)ndiff/image1.npixels);
  message("\n||Image1-Image2||/sqrt(||Image1|| ||Image2||)\t%g\n\n",
	  ndiff/max( sqrt(norm1*norm2), 1e-7 )         );

}

/*
 *------------------------------------------------------------------------
 *	    	    Expansion/Shrinking of the image
 */

				// Expand the prototype twice in each
				// dimension
				// In other words, blow each pixel of the
				// prototype to the 2x2 square
void IMAGE::_expand(const IMAGE& prototype)
{
  prototype.is_valid();
  allocate(2*prototype.nrows,2*prototype.ncols,prototype.bits_per_pixel);

  register GRAY * tp = pixels;
  register GRAY * pp = prototype.pixels;
  register int i,j;

  for(i=0; i<prototype.nrows; i++, tp+=ncols)
    for(j=0; j<prototype.ncols; j++)
    {
      register int pixel = *pp++;
      *(tp+ncols) = pixel;		// Fill out 2x2 square of the
      *tp++       = pixel;		// blown image with a pixel value
      *(tp+ncols) = pixel;
      *tp++       = pixel;
    }

  assert( pp == prototype.pixels + prototype.npixels );
  assert( tp == pixels + npixels );
}

				// Shrink the prototype twice in each
				// dimension. The image row and column
				// dimensions are assumed to be even numbers
				// Image is shrunk by replacing 2x2 square
				// by one pixel with an average intensity
				// over the square
void IMAGE::_shrink(const IMAGE& prototype)
{
  prototype.is_valid();
  if( (prototype.nrows & 1) || (prototype.ncols & 1) )
    _error("No of rows and columns in the image to shrink should both be even",
	   (prototype.info(),0));
  allocate(prototype.nrows/2,prototype.ncols/2,prototype.bits_per_pixel);

  register GRAY * tp = pixels;
  register GRAY * pp = prototype.pixels;
  register int i,j;

  for(i=0; i<nrows; i++)
  {					// Average row-by-row
    for(j=0; j<ncols; j++)
      *tp = *pp++, *tp++ += *pp++;	// Sum of pairs of the prototype pixels
    tp -= ncols;
    for(j=0; j<ncols; j++, tp++, pp+=2)
    {
      register int av = *tp + pp[0] + pp[1];	// This is the aver of 4 pixels
      *tp = ( av & 2 ? (av >> 2) + 1 : av >> 2 );	// /4 with rounding
    }
  }
  assert( pp == prototype.pixels + prototype.npixels );
  assert( tp == pixels + npixels );
}

