// This may look like C code, but it is really -*- C++ -*-
/*
 ************************************************************************
 *
 *			   Grayscale Image
 *
 *   The image is represented as a Pixmap, i.e. a matrix of pixels
 *    each of them specifies the gray level at a particular point
 *
 *	The header file defines arithmetical and all other operations
 *		  permitted on the grayscale image
 *
 * $Id: image.h,v 1.14 1994/03/24 20:40:21 oleg Exp oleg $
 *
 ************************************************************************
 */

//#pragma once
#ifndef _image_h
#define _image_h

#pragma interface

#include "myenv.h"
#include <std.h>

typedef unsigned short GRAY;		// Pixel type
typedef signed short GRAY_SIGNED;	// Pixel type, signed
const int GRAY_MAXBIT = 8*sizeof(GRAY);	// Max no of bits per pixel
const int GRAY_MAXVAL = ((1<<GRAY_MAXBIT)-1);


class Rectangle;
class rowcol;
class Extrema;
class IMAGE_to_PASS;
class EndianIO;

typedef int USER_F(int);		// User function that can be applied
					// to the image

class IMAGE
{
  friend class Rectangle;
  friend class Extrema;
  
private:			// Private part
  int valid_code;			// Validation code
  enum { IMAGE_val_code = 577767 };	// Image validation code
  int ncols;				// Image width in pixels
  int nrows;				// Image height in pixels
  int npixels;				// Total no of pixels in the image
  char * name;				// Image name
  int bits_per_pixel;			// Image depth
  GRAY ** scanrows;			// scanrows[i] = &pixels[i,0]
  GRAY * pixels;			// pixels[i,j] is the
  					// pixel value at point (i,j)
					// Pixels are ordered in rows

  void allocate(const int nrows, const int ncols, const int depth);

  void _expand(const IMAGE& prototype);	// Expand the prototype twice
  void _shrink(const IMAGE& prototype);	// Shrink the prototype twice

					// The following functions create an
					// image from a particular file
					// format. They call allocate()
					// to allocate the image data and
					// then read the data. The functions
					// are private to an IMAGE(file_name)
					// constructor and should be used
					// only by it
  void read_xwd(EndianIO& file, const char verbose); // read an X Window dump
  void read_pgm(EndianIO& file, const char verbose);// read a Portable GrayMap
  void read_tiff(EndianIO& file, const char verbose);	// read TIFF

public:			// Public interface
  
				// Constructors and destructors
  					// Make a blank image
  IMAGE(const int nrows, const int ncols, const int depth);
  IMAGE(const IMAGE &image);		// Make a new blank image like 
					// Read an image from the file
					// Attempts to detect the file format
  IMAGE(const char * file_name,const char print_header_info = 0);
  IMAGE(const Rectangle& area);	// Make an image from a square area
					// of another image

					// Construct an image applying a spec
					// operation to the prototype
  enum IMAGE_CREATORS_1op { Expand, Shrink };
  IMAGE(const IMAGE_CREATORS_1op op, const IMAGE& prototype);

  ~IMAGE();

  void is_valid() const
  { assure(valid_code == IMAGE_val_code,"Invalid image"); }

				// Status
  int q_nrows() const			{ return nrows; }
  int q_ncols()	const			{ return ncols; }
  int q_depth()	const			{ return bits_per_pixel; }
  int q_npixels() const			{ return npixels; }
  const char * q_name() const  		{ return name; } 
  void set_name(const char * new_name);	

				// Individual pixel manipulations
  GRAY& operator () (const int row, const int col) const;
  GRAY& operator () (const rowcol pos) const;
  void  sure_within(const rowcol pos) const;	// Make sure the pos
						// is within the image

// Row Column, Square operations

				// Image-scalar operations

				// Find out if the predicate
				// "(signed)pixel op val" is true for ALL
				// pixels of the image?
  int operator ==  (const int val) const;	// ? (signed)pixels == val
  int operator !=  (const int val) const;	// ? (signed)pixels != val
  int operator <   (const int val) const;	// ? (signed)pixels <  val
  int operator <=  (const int val) const;	// ? (signed)pixels <= val
  int operator >   (const int val) const;	// ? (signed)pixels >  val
  int operator >=  (const int val) const;	// ? (signed)pixels >= val

				// Modify every element of the
				// image according to the operation
  IMAGE& operator =   (const int val);		// Assignment to all the pixels
  IMAGE& operator -=  (const int val);		// Diminish the brightness
  IMAGE& operator +=  (const int val);		// Increase the brightness
  IMAGE& operator *=  (const int val);
  IMAGE& operator |=  (const int val);		// OR
  IMAGE& operator &=  (const int val);		// AND
  IMAGE& operator ^=  (const int val);		// XOR
  IMAGE& operator <<= (const int val);		// Shift all the pixels
  IMAGE& operator >>= (const int val);		// Shift all the pixels


				// Single image operations
  IMAGE& clear(void);			// Clear the image
  IMAGE& invert(void);			// Invert the image
  IMAGE& abs(void);			// pixel = |(signed)pixel|
  IMAGE& clip_to_intensity_range(void); // Clip pixel values to
					// [0,1<<bits_per_pixel-1]
  IMAGE& normalize_for_display(void);	// Normalize pixel values to be
					// in range 0..1<<bits_per_pixel-1
  IMAGE& equalize(const int no_grays);	// Perform the histogram equalization
  IMAGE& apply(USER_F * fp);		// Apply any function defined for
					// a pixel to all pixels in the image

				// Estimate the norm of the (signed) image
  double norm_1(void)     const;	// SUM{ |(signed)pixel[i,j]| }
  double norm_2_sqr(void) const;	// SUM{ (signed)pixel[i,j]^2 }
  int    norm_inf(void)   const;	// MAX{ |(signed)pixel[i,j]| }

				// Two images operations
  IMAGE& operator = (const IMAGE& source);	// Assignment

  friend int  identical(const IMAGE& im1, const IMAGE& im2);
  friend int  operator == (const IMAGE& im1, const IMAGE& im2);	// Alias
  friend void compare(const IMAGE& im1, const IMAGE& im2, 
		      const char * title);
  friend void are_compatible(const IMAGE& im1, const IMAGE& im2);

  						// Arithmetics
  friend IMAGE& operator += (IMAGE& target, const IMAGE& source);
  friend IMAGE& operator -= (IMAGE& target, const IMAGE& source);
  friend IMAGE& add(IMAGE& target, const int scalar,const IMAGE& source);
						// Shift the source to 'pos'
						// clip if necessary
						// multiply by scalar
						// and add
  IMAGE& shift_clip_add(rowcol pos, const int scalar, const IMAGE& source);

						// Logic
  friend IMAGE& operator |= (IMAGE& target, const IMAGE& source);
  friend IMAGE& operator &= (IMAGE& target, const IMAGE& source);
  friend IMAGE& operator ^= (IMAGE& target, const IMAGE& source);

						// Scalar product
  friend double operator * (const IMAGE& im1, const IMAGE& im2);

				// Estimate the norm of the difference 
				// between two (signed) image
					// SUM{ |(signed)pixel[i,j]| }
  friend double norm_1(const IMAGE& im1, const IMAGE& im2);
					// SUM{ (signed)pixel[i,j]^2 }
  friend double norm_2_sqr(const IMAGE& im1, const IMAGE& im2);
					// MAX{ |(signed)pixel[i,j]| }
  friend int    norm_inf(const IMAGE& im1, const IMAGE& im2);


				// I/O: write, read, display, print info
  					// Write to a file
					// "| command name" is OK as a file
					// name
  void write(const char * file_name,const char * title = "") const
  		{ write_xwd(file_name,title); }
					// Write an X window dump
  void write_xwd(const char * file_name,const char * title = "") const;
					// Write a Portable GrayMap
  void write_pgm(const char * file_name,const char * title = "") const;
					// Write a TIFF file
  void write_tiff(const char * file_name,const char * title = "") const;
  void display(const char * title) const;
  void info(void) const;		// Print the info about the image
  void print(const char * title) const;	// Print the image as a table

				// Clip a square area of the image
  Rectangle	square_of (const int size, const rowcol pos) const;
				// Clip a rectangular area of the image
				// specified by the coordinates of
				// the upper-left and lower-right corners
  Rectangle	rectangle (const rowcol uppleft, const rowcol lowright) const;

				// misc
  volatile void      error(const char* msg) const;
};

/*
 *------------------------------------------------------------------------
 *		Position specification and operations on it
 */

class rowcol {			// Specifying the row/col position
  friend class IMAGE;
  friend class Extrema;

protected:
  short int row_val;			// both row, col start from 0
  short int col_val;

public:
					// Constructors
  rowcol(const int row, const int col) 	: row_val(row), col_val(col) {}
  rowcol(void)				: row_val(-1), col_val(-1) {}

 ~rowcol() {}

  int row(void) const			{ return row_val; }
  int col(void) const			{ return col_val; }

					// Assignments
					// Note, that the implementation takes
					// advantage and is dependent of the
					// fact the entire rowcol structure
					// fits into one long word
  rowcol& operator = (const rowcol& pos) 
  	{ *((long int *)this) = *((long int *)&pos); return *this; }
  rowcol(const rowcol& pos)
  	{ *((long int *)this) = *((long int *)&pos); }

  int operator == (const rowcol& pos)
  	{ return *((long int *)this) == *((long int *)&pos); }
//	{ return memcmp(this,&pos,sizeof(rowcol)); }

					// Offset the current position
  rowcol& operator += (const rowcol& pos)
  	{ row_val += pos.row_val; col_val += pos.col_val; return *this; }
  rowcol& operator -= (const rowcol& pos)
  	{ row_val -= pos.row_val; col_val -= pos.col_val; return *this; }

  friend rowcol operator + (const rowcol& pos1, const rowcol& pos2);
  friend rowcol operator - (const rowcol& pos1, const rowcol& pos2);

					// Scale the current position
  rowcol& operator *= (const int scalef)
  	{ row_val *= scalef; col_val *= scalef; return *this; }

  friend rowcol operator *  (const rowcol& pos, const int scalef);
  friend rowcol operator << (const rowcol& pos, const int shiftf);
  friend rowcol operator >> (const rowcol& pos, const int shiftf);
};

inline rowcol operator + (const rowcol& pos1, const rowcol& pos2) return
        rc(pos1.row_val+pos2.row_val, pos1.col_val+pos2.col_val) {}
inline rowcol operator - (const rowcol& pos1, const rowcol& pos2) return
        rc(pos1.row_val-pos2.row_val, pos1.col_val-pos2.col_val) {}

inline rowcol operator *  (const rowcol& pos, const int scalef) return
	rc(pos.row_val*scalef, pos.col_val*scalef) {}
inline rowcol operator >> (const rowcol& pos, const int shiftf) return
	rc(pos.row_val >> shiftf, pos.col_val >> shiftf) {}
inline rowcol operator << (const rowcol& pos, const int shiftf) return
	rc(pos.row_val << shiftf, pos.col_val << shiftf) {}

class Extrema			// Find min/max values of the (signed) image
{
  GRAY_SIGNED max_value;		// Max pixel value in the image
  GRAY_SIGNED min_value;		// Min pixel value in the image

public:
  rowcol      max_pixel;		// Position of the largest pixel
  rowcol      min_pixel;		// Position of the smallest pixel

  Extrema(const IMAGE& image);		// Find extremum pixels of the image
  ~Extrema() {}

  GRAY_SIGNED max(void) const		{ return max_value; }
  GRAY_SIGNED min(void) const		{ return min_value; }
};

/*
 *------------------------------------------------------------------------
 *		 Dealing with the rectangular area of the image
 */

class Rectangle
{
  friend class IMAGE;

  IMAGE&	image;		// The image I'm a rectangle of
  short int	nrows;		// Dimension of the rectangle
  short int	ncols;
  GRAY *	ptr;		// Pointer to the upper left corner of 
				// the rectangle
  int		inc_to_nextrow;	// inc_to_nextrow = p' - p =
  				// 		  = image.ncols - ncols
				// where
				// p = ptr + ncols points to the pixel
				// just beyond the current scanline of
				// the Rectangle
				// p' = (ptr - col) + image.ncols + col
				// points to the first pixel of the next
				// scanline of the rectangtle
				// col tells the column of the upper left
				// corner of the rectangle within the image
 
				// Private constructor
  				// Note these are private constructors to
				// be used with IMAGE::square_of/rectangle
				// functions
  Rectangle
    (IMAGE& im, const int size, const rowcol pos);
  Rectangle
    (IMAGE& im, const rowcol uppleft, const rowcol lowright);

  public:
				// Note how to construct square area of
				// the image 'im':
				//	im.square_of(10,rowcol(1,5))
				// and rectangular area
				// 	im.rectangle(rowcol(0,1),rowcol(4,5))

  Rectangle (IMAGE& im);		// Treat entire image as a rectangle
  ~Rectangle()	{}
					// Assign a value to all the pixels
					// in the rectangle area
  Rectangle& operator  = (const int val);
  					// Modify the pixels in the rectangle
					// area
  friend void operator +=  (const Rectangle& sa, const int val);
  friend void operator -=  (const Rectangle& sa, const int val);
  friend void operator *=  (const Rectangle& sa, const int val);
  friend void operator |=  (const Rectangle& sa, const int val);
  friend void operator &=  (const Rectangle& sa, const int val);
  friend void operator ^=  (const Rectangle& sa, const int val);
  friend void operator <<= (const Rectangle& sa, const int val);
  friend void operator >>= (const Rectangle& sa, const int val);

					// Copy a rectangle a_rect into the
					// rectangular area of the image
  Rectangle& operator = (const Rectangle& a_rect);

  					// Get a total sum of all the pixels
  friend double sum_over(const Rectangle& sa);
};

/*
 *------------------------------------------------------------------------
 *			Some service procedures
 */

inline
int log2(			// Find a binary logarithm of n
	const int n )		// and check that n is a power of two
{
  register int i,k;

  assure(n > 0, "log2: the argument's got to be positive!");

  for(k=0,i=1; i < n; k++, i*=2)
    ;
  if( i != n )
    _error("log2: the argument %d has got to be an exact power of two",n);

  return k;
}

inline
int exp2(const int k)			// Compute 2^k, k>=0
{
  assure(k >= 0, "exp2: the argument may not be negative!");
  if( k > (signed)sizeof(int)*8-1 )
    _error("exp2: the exponent %d is too big",k);

  return 1<<k;
}

inline					// return b * round( a/b )
int round_to_even_multiple(const int a, const int b)
{
  assert( b > 0 );
  register int rem = a % b;
  if( 2*rem < b )
    return a - rem;
  else
    return a + b - rem;
}

/*
 *------------------------------------------------------------------------
 *			Inline Image Procedures
 */

inline IMAGE::IMAGE(const int no_rows, const int no_cols, const int depth)
{
  allocate(no_rows,no_cols,depth);
}

					// Make a new image like the
inline IMAGE::IMAGE(const IMAGE& old)	// old one
{
  allocate(old.nrows,old.ncols,old.bits_per_pixel);
}

inline GRAY& IMAGE::operator () (const int row, const int col) const
{
  is_valid();
  if( row >= nrows || row < 0 )
    _error("Row index %d is out of image boundaries [0,%d]",row,nrows-1);
  if( col >= ncols || col < 0 )
    _error("Col index %d is out of image boundaries [0,%d]",col,ncols-1);
  
  return (scanrows[row])[col];
}

inline GRAY& IMAGE::operator () (const rowcol pos) const
{
  return operator()(pos.row(),pos.col());
}

					// Make sure the pos is within the
					// image
inline void  IMAGE::sure_within(const rowcol pos) const
{
  is_valid();
  if( pos.row() >= nrows || pos.row() < 0 )
    _error("Row index %d is out of image boundaries [0,%d]",pos.row(),nrows-1);
  if( pos.col() >= ncols || pos.col() < 0 )
    _error("Col index %d is out of image boundaries [0,%d]",pos.col(),ncols-1);
}

inline IMAGE& IMAGE::clear(void)	// Clean the image
{
  is_valid();
  memset(pixels,0,npixels*sizeof(GRAY));
  return *this;
}

inline IMAGE& IMAGE::invert(void)	// Invert the image
{
  is_valid();
  return (*this ^= ((1<<bits_per_pixel)-1));
}

inline void are_compatible(const IMAGE& im1, const IMAGE& im2)
{
  im1.is_valid();
  im2.is_valid();
  
  if( im1.ncols != im2.ncols || im1.nrows != im2.nrows )
    _error("The image %dx%d and the image %dx%d have different sizes",
	   im1.nrows,im1.ncols,im2.nrows,im2.ncols);
}

inline int  operator == (const IMAGE& im1, const IMAGE& im2)
{
  return identical(im1,im2);
}


				// Construct a square rectangle
inline Rectangle::Rectangle
    (IMAGE& im, const int sq_size, const rowcol pos)
    : image(im), nrows(sq_size), ncols(sq_size)
{
  image.is_valid();

  if( pos.row() >= image.nrows || pos.row() < 0 ||
      pos.row() + nrows > image.nrows ||
      pos.col() >= image.ncols || pos.col() < 0 ||
      pos.col() + ncols > image.ncols )
    _error("Square area (left upper point [%d,%d], size %d)\n"
	   "is not within the image %dx%d",
	   pos.row(),pos.col(),sq_size,image.nrows,image.ncols);

  ptr  = &(image.scanrows[pos.row()][pos.col()]);
  inc_to_nextrow = image.ncols - ncols;
}

				// Clip a square area from the image
inline Rectangle IMAGE::square_of(const int size, const rowcol pos) const
return c(*this, size, pos)
{}


				// Make a rectangle with upper left corner
				// at uppleft position and lower right corner
				// at lowright position
inline Rectangle::Rectangle
    (IMAGE& im, const rowcol uppleft, const rowcol lowright)
    : image(im), nrows(lowright.row()-uppleft.row()+1), 
                 ncols(lowright.col()-uppleft.col()+1)
{
  image.is_valid();

  image.sure_within(uppleft);
  image.sure_within(lowright);

  ptr  = &(image.scanrows[uppleft.row()][uppleft.col()]);
  inc_to_nextrow = image.ncols - ncols;
}


				// Clip a rectangular area from the image
inline Rectangle IMAGE::rectangle(const rowcol uppleft,const rowcol lowright)
                 const
return c(*this, uppleft, lowright)
{}


				// Treat the entire image as a rectangular area
inline Rectangle::Rectangle (IMAGE& im)
    : image(im), nrows(im.nrows), ncols(im.ncols)
{
  image.is_valid();

  ptr  = image.pixels;
  inc_to_nextrow = 0;
}

#endif
