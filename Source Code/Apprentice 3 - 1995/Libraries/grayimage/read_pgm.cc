// This may look like C code, but it is really -*- C++ -*-
/*
 ************************************************************************
 *
 *			   Grayscale Image
 *
 *	     Read an image from a Portable GrayMap (pgm) file
 * 
 * The program reads a "binary" (RAWBITS) pgm file of the following format
 *   - A "magic number" for identifying the file type.  A pgm
 *     file's RAWBITS magic number is the two characters "P5".
 *   - Whitespace (blanks, TABs, CRs, LFs).
 *   - A width, formatted as ASCII characters in decimal.
 *   - Whitespace.
 *   - A height, again in ASCII decimal.
 *   - Whitespace.
 *   - The maximum gray value, again in ASCII decimal. For RAWBITS pgm file
 *	   the maximum grayscale value cannot exceed 255.
 *   - A _single_ character of whitespace (typically a newline).
 *   - Width * height gray values, each as plain bytes, between
 *     0 and the specified maximum value, stored consecutivly, 
 *     starting at the top-left corner of the graymap, proceding in normal
 *     English reading order. 
 *     A value of 0 means black, and the maximum value means white.
 *
 * For more detail, see documentation on PBMPLUS package (specifically,
 * pgm(5)).
 *
 * $Id: read_pgm.cc,v 1.1 1994/01/12 21:07:24 oleg Exp oleg $
 *
 ************************************************************************
 */

#include "image.h"
#include "endian_io.h"

#include <iostream.h>
#include <ctype.h>

/*
 *------------------------------------------------------------------------
 *			   Class PGMFile
 *	 designed to contain the control info about the image
 *		as defined in the Portable GrayMap file 
 */

class PGMFile : public EndianIO
{
friend class IMAGE;
 
  int pixmap_width;			// Dimensions of the graymap
  int pixmap_height;
  int max_gray_value;			// Maximum value of a graymap pixel
  int pixmap_depth;			// ceil(log2(max_gray_value))

  void set_pixmap_type(void);

  EndianIO& get(int& item);		// Read an item skipping whitespaces
	
public:
  char * win_name;		      	// Name of the image in the file
  enum { PT_8bitGray, PT_BlackWhite, PT_General } pixmap_type;

  PGMFile(EndianIO& file_name);		// Constructor
  ~PGMFile() {}				// Destructor
  void info(void) const;		// Dump the header information
					// Reading the pixmap into image
  void read_8bit_pixmap(GRAY& pixel_matrix, const int no_pixels);
  void read_any_pixmap(IMAGE &image);	// Handle the general case
};

/*
 *------------------------------------------------------------------------
 *			Construct the PGMFile
 *	   by reading the header of the X Window dump file
 */

PGMFile::PGMFile(EndianIO& file) : EndianIO(file)
{
  const char * magic_str = "P5";
  char read_magic_str[3] = {0,0,0};
  read_magic_str[0] = read_byte("Reading magic string");
  read_magic_str[1] = read_byte("Reading magic string");
  if( strcmp(read_magic_str,magic_str) != 0 )
    _error("Read magic string '%s' is not what's expected '%s':"
    	   "it's not a Portable GrayMap (PGM) file",read_magic_str,magic_str);
    	   
  if( !get(pixmap_width) )
    _error("Failed to read the number of columns (graymap width)");
   
  if( !get(pixmap_height) )
    _error("Failed to read the number of rows (graymap height)");
    
  
  if( !get(max_gray_value) )
    _error("Failed to read the maximum gray value in the map");
 
  assert( pixmap_width > 0 && pixmap_height > 0 && max_gray_value > 0);

  if( max_gray_value > 255 )
    _error("Can't handle the graymap with the maximum gray value, %d, "
	   "greater than 255",max_gray_value);
   
  {
    int t;
    for(pixmap_depth=0,t=1; t < max_gray_value+1; pixmap_depth++, t *= 2)
      ;
  }

  win_name = "";

  set_pixmap_type();
}

				// Analyse the header and figure out if
				// we can handle the PGMFile we've read.
				// Set the pixmap type.
void PGMFile::set_pixmap_type(void)
{
  if( pixmap_depth > 8 || pixmap_depth < 1 )
    _error("Can't handle the pixmap depth of %d: "
	   "we can process only 1..8-bit maps",
    	   pixmap_depth);
  pixmap_type = PT_8bitGray;
}

				// Read an item skipping whitespaces
EndianIO& PGMFile::get(int& item)
{
  char c;
  char buffer[10];
  char * p = buffer;
	
  while( c=read_byte("Reading int item in ASCII"), isspace(c) )
    ;
  
  *p++ = c;
  while( c=read_byte("Reading int item in ASCII"), !isspace(c) )
    *p++ = c;
  *p++ = 0;
	
  item = strtol(buffer,&p,10);
  assert( *p == '\0' );
  
  return *this;
}

/*
 *------------------------------------------------------------------------
 *	Print out all the control information pertaining to the
 *		 	X Window Image read
 */

void PGMFile::info(void) const
{
  cout << "\n\n=====>The following Portable GrayMap has been read";
  cout << "\nPixmap depth:   " << pixmap_depth;
  cout << "\nPixmap width:   " << pixmap_width;
  cout << "\nPixmap height:  " << pixmap_height;
  cout << "\nMax gray value: " << max_gray_value;
  cout << "\nWindow name:    " << win_name;

  cout << "\n-----End of PGM information\n";
}

/*
 *------------------------------------------------------------------------
 *		Read the image pixmap and write it into the image
 */

				// Special case of the Gray scale pixelmap of
				// depth 8
void PGMFile::read_8bit_pixmap(GRAY& pixel_matrix, const int no_pixels)
{
  register GRAY * pixelp;
  for(pixelp = &pixel_matrix; pixelp < &pixel_matrix+no_pixels;)
    *pixelp++ = read_byte("Reading the pixel map");
}

					// Handle the general case
void PGMFile::read_any_pixmap(IMAGE &image)
{
  _error("PGMFile::read_any_pixmap is not implemented yet");
}

/*
 *========================================================================
 *     		Root module - actual IMAGE constructor
 */

void IMAGE::read_pgm(EndianIO& file, const char print_header_info)
{
  message("Reading the Portable GrayMap (PGM)\n");
  
  PGMFile pgmfile(file);

  if( print_header_info )
    pgmfile.info();

  allocate(pgmfile.pixmap_height,pgmfile.pixmap_width,pgmfile.pixmap_depth);

  name = pgmfile.win_name;

  switch( pgmfile.pixmap_type )
  {				// Select the most efficient procedure
    case PGMFile::PT_8bitGray:
         pgmfile.read_8bit_pixmap(*pixels,npixels);
	 break;

    case PGMFile::PT_BlackWhite:		// Not implemented yet
    default:
	 pgmfile.read_any_pixmap(*this);
  }

  cout << "\n" << ncols << "x" << nrows << "x" << bits_per_pixel << " image '"
  	   << name << "' has been read\n";
}
