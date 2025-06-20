// This may look like C code, but it is really -*- C++ -*-
/*
 ************************************************************************
 *
 *			   Grayscale Image
 *
 *	           Read an image from the X Window dump file
 *
 * $Id: read_xwd.cc,v 1.4 1994/01/13 14:53:03 oleg Exp oleg $
 *
 ************************************************************************
 */

#include "image.h"
#include "endian_io.h"
#include "x11wd.h"

#include <ostream.h>

/*
 *------------------------------------------------------------------------
 *			Create an image from the file
 * This function attempt to determine the image file format by looking at
 * the first two bytes of the file. They are
 * 'Px' - for a Portable GrayMap
 * 'MM' or 'II' - for a TIFF file
 * otherwise, it _might_ be an X window dump.
 * After the file format is guessed, the program calls an appropriate
 * reader to actually load the file.
 */

IMAGE::IMAGE(const char * file_name, const char print_header_info)
{
  message("\nReading the file '%s' ",file_name);
  EndianIO file(file_name,ios::in);
  assert( file.good() );

  char byte1 = file.peek();
  if( byte1 == 'P' )
    read_pgm(file,print_header_info);
  else if( byte1 == 'M' || byte1 == 'I' )
    read_tiff(file,print_header_info);
  else
    read_xwd(file,print_header_info);
}

/*
 *------------------------------------------------------------------------
 *			   Class XWDump
 *	 designed to contain the control info about the image
 *		as defined in the X Window Dump file 
 */

class XWDump			// All the info pertaining to the XWD file
   : public X11WDFileHeader, public EndianIO {

 friend class IMAGE;

     void set_pixmap_type(void);	// Set the pixmap type, see below

public:
     char * win_name;		      	// Name of the image in the file
     enum { PT_8bitGray, PT_BlackWhite, PT_General } pixmap_type;

     XWDump(EndianIO& file);			// Constructor
     ~XWDump() {}				// Destructor
     void info(void) const;			// Dump the header information
					// Reading the pixmap into image
     void read_8bit_pixmap(GRAY& pixel_matrix, const int no_pixels);
     void read_any_pixmap(IMAGE &image);	// Handle the general case
};

/*
 *------------------------------------------------------------------------
 *			Construct the XWDump
 *	   by reading the header of the X Window dump file
 */

XWDump::XWDump(EndianIO& file) : EndianIO(file)
{

  streampos beg_of_file = EndianIO::tellg();	// Remember the curr position
  set_bigendian();		// Try reading a magic byte in one order
  header_size = read_long("Reading the header_size");
  file_version = read_long("Reading the XWD file version");

  if( file_version != X11WD_FILE_VERSION )
  {
    set_littlendian();		// Try now another byte order
    EndianIO::seekg(beg_of_file);
    header_size = read_long("Reading the header_size");
    file_version = read_long("Reading the XWD file version");
  }

  assure( file_version == X11WD_FILE_VERSION,
	 "I have tried different byte orders but failed to read\nthe XWD "
	 "file version correctly" );
  assure( header_size >= sizeof(X11WDFileHeader),"XWD header is too small" );

  				// Read the rest of the header
  pixmap_format    = read_long("Reading the pixmap_format");
  pixmap_depth     = read_long("Reading the pixmap_depth");
  pixmap_width     = read_long("Reading the pixmap_width");
  pixmap_height    = read_long("Reading the pixmap_height");
  xoffset          = read_long("Reading the xoffset");
  xbyte_order      = read_long("Reading the byte_order");
  bitmap_unit      = read_long("Reading the bitmap_unit");
  bitmap_bit_order = read_long("Reading the bitmap_bit_order");
  bitmap_pad       = read_long("Reading the bitmap_pad");
  bits_per_pixel   = read_long("Reading the bits_per_pixel");
  bytes_per_line   = read_long("Reading the bytes_per_line");
  visual_class     = read_long("Reading the visual_class");
  red_mask         = read_long("Reading the red_mask");
  green_mask       = read_long("Reading the green_mask");
  blue_mask        = read_long("Reading the blue_mask");
  bits_per_rgb     = read_long("Reading the bits_per_rgb");
  colormap_entries = read_long("Reading the colormap_entries");
  ncolors          = read_long("Reading the ncolors");
  window_width     = read_long("Reading the window_width");
  window_height    = read_long("Reading the window_height");
  window_x         = read_long("Reading the window_x");
  window_y         = read_long("Reading the window_y");
  window_bdrwidth  = read_long("Reading the window_bdrwidth");

  					// Read the window name
  register int len = header_size - sizeof(X11WDFileHeader);
  win_name = new char[len];
  if( !EndianIO::read(win_name, len) )
    error("Error reading the window name");

  if( ncolors > 0 )			// Skip the Xcolors
    EndianIO::seekg(ncolors*sizeof(X11XColor),ios::cur);

  set_pixmap_type();
}

				// Analyse the header and figure out if
				// we can handle the XWdump we've read.
				// Set the pixmap type.
void XWDump::set_pixmap_type(void)
{
  const int bits_per_item = bitmap_unit;

  if( pixmap_depth > 24 )
    _error("can't handle X11 pixmap_depth %d greater than 24",pixmap_depth);
  if( bits_per_rgb > 24 )
    _error("can't handle X11 bits_per_rgb > 24");
  if( pixmap_format != ZPixmap && pixmap_depth != 1 )
    _error("can't handle X11 pixmap_format %d with depth != 1",pixmap_format);
  if( bits_per_item != 8 && bits_per_item != 16 && bits_per_item != 32 )
    _error("X11 bitmap_unit (%d) is non-standard - can't handle",
	   bits_per_item);
  if( bits_per_item == 8 && bits_per_pixel == 8 && 
      visual_class != TrueColor && visual_class != DirectColor )
    pixmap_type = PT_8bitGray;
  else if( bits_per_item == 8 && bits_per_pixel == 1 )
    pixmap_type = PT_BlackWhite;
  else
    pixmap_type = PT_General;
}

/*
 *------------------------------------------------------------------------
 *	Print out all the control information pertaining to the
 *		 	X Window Image read
 */

void XWDump::info(void) const
{
  cout << "\n\n=====>The following X Window dump has been read";
  cout << "\nVersion:        " << file_version;
  cout << "\nPixmap format:  ";
  switch(pixmap_format)
  {
    case XYBitmap:
         cout << "XYBitmap";
         break;
    case XYPixmap:
         cout << "XYPixmap";
         break;
    case ZPixmap:
         cout << "ZPixmap";
         break;
    default:
	 _error("Unknown pixel format: %d",pixmap_format);
  }
  cout << "\nPixmap depth:   " << pixmap_depth;
  cout << "\nPixmap width:   " << pixmap_width;
  cout << "\nPixmap height:  " << pixmap_height;
  cout << "\nBitmap x offs:  " << xoffset;
  cout << "\nByte order:     " << ( xbyte_order == MSBFirst ?
				    "MSBFirst" : "LSBFirst" );
  cout << "\nBitmap unit:    " << bitmap_unit;
  cout << "\nBitmap bitOrder:" << ( bitmap_bit_order == MSBFirst ?
				  "MSBFirst" : "LSBFirst" );
  cout << "\nScanline pad:   " << bitmap_pad;
  cout << "\nBits per pixel: " << bits_per_pixel;
  cout << "\nBytes per line: " << bytes_per_line;
  cout << "\nVisual class:   ";
  switch(visual_class)
  {
    case StaticGray:
         cout << "StaticGray";
         break;
    case GrayScale:
         cout << "GrayScale";
         break;
    case StaticColor:
         cout << "StaticColor";
         break;
    case PseudoColor:
         cout << "PseudoColor";
         break;
    case TrueColor:
         cout << "TrueColor";
         break;
    case DirectColor:
         cout << "DirectColor";
         break;
    default:
	 _error("Unknown visual class: %d",visual_class);
  }

  cout << "\nZ red mask:     0x";  cout.form("%x",red_mask);
  cout << "\nZ green mask:   0x";  cout.form("%x",green_mask);
  cout << "\nZ blue mask:    0x";  cout.form("%x",blue_mask);
  cout << "\nBits per rgb:   " << bits_per_rgb;
  cout << "\nColormap items: " << colormap_entries;
  cout << "\nNo. colors:     " << ncolors;
  cout << "\nWindow width:   " << window_width;
  cout << "\nWindow height:  " << window_height;
  cout << "\nWindow left  X: " << window_x;
  cout << "\nWindow upper Y: " << window_y;
  cout << "\nWindow border:  " << window_bdrwidth;
  cout << "\nWindow name:    " << win_name;

  cout << "\n-----End of X Window dump information\n";
}

/*
 *------------------------------------------------------------------------
 *		Read the image pixmap and write it into the image
 */

				// Special case of the Gray scale pixelmap of
				// depth 8
void XWDump::read_8bit_pixmap(GRAY& pixel_matrix, const int no_pixels)
{
  assert( pixmap_type == PT_8bitGray );
  message("\nReading the gray scale pixel map of depth 8...");
  register GRAY * pixelp;
  for(pixelp = &pixel_matrix; pixelp < &pixel_matrix+no_pixels;)
    *pixelp++ = read_byte("Reading the pixel map");
}

				// General case of any pixelmap
void XWDump::read_any_pixmap(IMAGE& image)
{
  const int ncols = pixmap_width;
  const int nrows = pixmap_height;
  const int bits_per_item = bitmap_unit;
  const int bits_per_pixel = XWDump::bits_per_pixel;
  const int bit_order = bitmap_bit_order;
  const int pixel_mask = ( 1 << bits_per_pixel ) - 1;
  int convert_rgb = 0;
  register int row;

  if( visual_class == TrueColor || visual_class == DirectColor )
  {
    convert_rgb = 1;
    message("\nA colored image with direct RGB specification\n"
	    "is to be converted to gray scale\n");
  }

  for(row=0; row < nrows; row++)
  {
    char scanline[bytes_per_line];
    register int col;

    switch( bits_per_item )		// Read a scanline
    {
      case 8:
      {
	register char *p = (char *)scanline;
	for(col=0; col < (signed)sizeof(scanline); col++)	
	  *p++ = read_byte("Reading the scanline");
      }
      break;

      case 16:
      {
	register short int *p = (short int *)scanline;
	for(col=0; col < (signed)sizeof(scanline); col+=sizeof(short int))
	  *p++ = read_short("Reading the scanline");
      }
      break;

      case 32:
      {
	register long int *p = (long int *)scanline;
	for(col=0; col < (signed)sizeof(scanline); col+=sizeof(long int))
	  *p++ = read_long("Reading the scanline");
      }
      break;

      default:
         assure(0,"can't happen");
    }

    register char *cp = &scanline[0];
    register long int bits_storage = 0;
    register int bit_shift = 0;
    register int remains_bit = 0;

    for(col=0; col<ncols; col++)	// Extract pixels and put to image
    {
      register unsigned int pixel;
      if( remains_bit == 0 )
      {
	switch(bits_per_item)
	{
	  case 8:
	  bits_storage = *cp;
	  break;

	  case 16:
	  bits_storage = *((short int *)cp);
	  break;

	  case 32:
	  bits_storage = *((long int *)cp);
	  break;
	}
	cp += bits_per_item/8;
	remains_bit = bits_per_item;
	bit_shift = ( bit_order == MSBFirst ? bits_per_item - bits_per_pixel :
		      0 );
      }
      assert( bit_shift >= 0 );
      pixel = ( bits_storage >> bit_shift ) & pixel_mask;
      bit_shift += ( bit_order == MSBFirst ? - bits_per_pixel : 
		     bits_per_pixel );
      remains_bit -= bits_per_pixel;
      assure(remains_bit >= 0,"Something goes wrong with bits conversion");

      if(convert_rgb)
	pixel = ((pixel & 0xff0000) << (20-16)) +	/* Red part	*/
	        ((pixel & 0xff00) << (10-8)) +		/* Green part	*/
		((pixel & 0xff));			/* Blue part	*/
      image(row,col) = pixel;
    }
    assert( cp - scanline == sizeof(scanline)/sizeof(*cp) );
  }
}

/*
 *========================================================================
 *		     Root module - actual IMAGE constructor
 */

void IMAGE::read_xwd(EndianIO& file, const char print_header_info)
{
  message("Reading the X window dump image\n");
  
  XWDump xwdump(file);

  if( print_header_info )
    xwdump.info();

  allocate(xwdump.pixmap_height,xwdump.pixmap_width,xwdump.bits_per_pixel);

  name = xwdump.win_name;

  switch( xwdump.pixmap_type )
  {				// Select the most efficient procedure
    case XWDump::PT_8bitGray:
         xwdump.read_8bit_pixmap(*pixels,npixels);
	 break;

    case XWDump::PT_BlackWhite:		// Not implemented yet
    default:
	 xwdump.read_any_pixmap(*this);
  }

  cout << form("\n%dx%dx%d image '%s' has been read\n",ncols,nrows,
	       bits_per_pixel,name);
}

