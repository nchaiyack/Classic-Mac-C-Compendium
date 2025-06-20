// This may look like C code, but it is really -*- C++ -*-
/*
 ************************************************************************
 *
 *			   Grayscale Image
 *
 *		Read an image in the Data Exchange Format
 *
 * which is used to exchange SAT imagery and other weather products
 * within the weather community.
 * The DEF format is a tagged format described in the document
 *	Standard Formats for Weather Data Exchange Among Automated
 *	Weather Information Systems
 *	OFCM / Office of the Federal Coordinator for Meteorological
 *	Services and Supporting Research
 *	U.S. Department of Commerce / National Oceanic and Atmospheric
 *	Administration,
 *	FCM-S2-1990, Washington, D.C., May 1990
 *
 * In short, a DEF file consits of a sequence of fields. Each field is a
 * collection of two-byte words. The order of bytes within a word is
 * bigendian (the most significant byte first). A field starts with a
 * standard header that specifies the total field length and the
 * field type (mode/submode). The header is followed by data
 * which meaning depends on the field type as determined by a
 * particular mode/submode. The data may be followed by a checksum.
 * Control fields describe the product, conditions in which it was
 * acquired, and other relevant information. Data fields contain data
 * of the product, say, raster scans of a satellite image.
 *
 * As far as SAT product are concerned, the following sequence of fields
 * is expected within the DEF file
 *	Product ID field, mode 1/submode 1
 *	Product Def block, mode 6/submode 020
 *			   [mode 1/submode 5]	  (field/data widths specs)
 *			   [mode 1/submode 3]	  (Additional secrecy info)
 *			   [mode 6 submode 030]	  (pixel product def)
 *	Data descr block sets
 *			   mode 6/submode 1	  (describing raster scans)
 *	End Of Product block, mode 1/submode 2
 *
 * $Id: read_def.cc,v 1.1 1994/01/13 15:32:30 oleg Exp oleg $
 *
 ************************************************************************
 */

#include "image.h"
#include "endian_io.h"

				// A 2-byte integer with the guaranteed
				// "Most-significant-byte-first" byte
				// order
class Word
{
  unsigned char byte1, byte2;
public:
  Word(void) {}
  ~Word(void) {}
					// Convert to a regular int
  operator unsigned short (void) const
		{ return ((unsigned short)byte1 << 8) | byte2; }
  operator short (void) const { return ((short)byte1 << 8) | byte2; }
};

class DEFTag			// A DEF field tag
{
  char mode;				// a combination of mode and submode
  char submode;				// codes

public:
  DEFTag(void) { mode = submode = 0; }
  DEFTag(char _mode, char _submode) : mode(_mode), submode(_submode) {}
  ~DEFTag(void) {}
  DEFTag& operator = (const unsigned short word)
	{ mode = (word >> 8) & 0xff; submode = word & 0xff; }
  int operator == (const DEFTag& a_tag) 
	{ return *(unsigned short *)this == *(unsigned short *)&a_tag; }
  operator const char * (void) const;	// Give a string representation
};

				// Give a string representation for a class
				// Returns a ptr to a STATIC string
DEFTag::operator const char * (void) const
{
  static char buff[40];
  sprintf(buff,"mode %0o, submode %0o",mode,submode);
  return buff;
}

class StdField;			// Standard Field header
				// DEF file is a collection of tagged field
				// Here is a generic field
class Field : public DEFTag
{
  friend class StdField;			// Standard Field header

  short data_size;		// Actual size of the DATA, bytes 
  Word data[2048];		// A data part of the field

public:
  Field(void);				// Create an "empty" field
  ~Field(void);
  friend EndianIO& operator >> (EndianIO& fs, Field& field);
  int length(void) const	{ return data_size; }
};


				// Create an "empty" field
Field::Field(void)
{
  data_size = 0;
}


				// Destroy the field
Field::~Field(void)
{
}


				// Read in a field from the file
EndianIO& operator >> (EndianIO& fs, Field& field)
{
  unsigned short word = fs.read_short("Reading the 1st word of the field");
  char checksum_present;
  unsigned short checksum = 0;

  int length_words;				// Data size in words
  if( (word & 0xc000) == 0 )
    checksum_present = 1,
    length_words = (word & ~0xc000) - 1 - 1 -1;	// length, mode, and cs words
  else if( (word & 0xc000) == 0x4000 )
    checksum_present = 0,
    length_words = (word & ~0xc000) - 1 - 1;	// 1 word length, 1 word mode
  else if( (word & 0xc000) == 0xc000 )
    _error("Can't handle a field w/o a length");
  else
    _error("Invalid flag in the 1st field of a word %04X",word);

  field.data_size = length_words * sizeof(Word);

  if( checksum_present )
    checksum = word;

  word = fs.read_short("Reading mode/submode");
  if( checksum_present )
    checksum += word;
  *(DEFTag *)&field = word;

  assert( field.data_size >= 0 );
  assert( field.data_size < sizeof(field.data) );

  if( !fs.read((char *)field.data,field.data_size) )
    _error("Dying because of I/O error above",
	   (perror("Field data read error"),0));

  if( checksum_present )
  {
    register Word * dp;
    for(dp=field.data; dp < field.data + length_words;)
      checksum += (unsigned short)*dp++;
    checksum += fs.read_short("reading the checksum");
    assert( checksum == 0 );
  }

  return fs;
}


				// Standard Field header, from which
				// particular product field inherit
class StdField : public DEFTag
{
public:
  const enum Length_type { Exact_length, Min_length };
private:
  const int length_words;		// Length of the field in words
  Length_type length_type;
  char data[0];				// Data follow
protected:
  StdField(const DEFTag tag, const int _length, const Length_type _ltype)
    : DEFTag(tag), length_words(_length), length_type(_ltype) {}
  ~StdField(void) {}
  void operator = (const Field& field);
};

				// Assignment of a field
void StdField::operator = (const Field& field)
{
  assert( *(DEFTag *)this == field );	// Make sure the field assigned is 
					// really of the type we expect
  if( length_type == Exact_length )
    assert( sizeof(Word) * length_words == field.length() );
  else
    assert( sizeof(Word) * length_words <= field.length() );
  memcpy(data,field.data,field.length());
}

/*
 *------------------------------------------------------------------------
 *		      Specific blocks for SAT products
 * Constructor assignes default values to be used if the field isn't
 * specified in the input stream
 */

class ProductIDField : public StdField
{
				// Contents of the data field
  char originator[4];			// Facility that has generated product
  char classification;			// secrecy stuff
  char retention_time;			// in days, -1 or 0 means dummy
  char file_indicator;			// Code of the originating agency
  char catalog_no[4];
  char time_generated[3];		// e.g., in hrs
  char misc_id[2];			// See App. C, Table C2-1
					// Time the file was generated 
  Word year;				// full century year, in binary
  char month, day, hour, minute;

public:
  ProductIDField(const Field& field);
  ~ProductIDField(void) {}
  operator const char * (void) const;	// Give a readable description
};

				// Assign from a generic field
ProductIDField::ProductIDField(const Field& field)
	: StdField(DEFTag(1,1),11,StdField::Exact_length)
{
  *(StdField *)this = field;
}

				// Give a readable description of the Product
				// ID. Returns a ptr to a STATIC string
ProductIDField::operator const char * (void) const
{
  static char buffer[700];
  sprintf(buffer,"Originator %.4s, classification %c, retention_time %d\n"
	  "file_indicator %0o, catalog_number %.4s, time_generated %.3s\n"
	  "misc id %.2s, date %d/%d/%d %02d:%02d\n",
	  originator,classification, 
	  (retention_time == -1 ? 0 : retention_time),
	  file_indicator,catalog_no,time_generated,misc_id,
	  month,day,(unsigned short)year,hour,minute);
  return buffer;
}

				// Classification Block
class ClassifField : public StdField
{
  char title[100];
public:
  ClassifField(void)  
    : StdField(DEFTag(1,3),0,StdField::Min_length) { title[0] = '\0'; }
  ~ClassifField(void) {}
  void operator = (const Field& field);
  void print(void) const;
};

void ClassifField::operator = (const Field& field)
{
  *(StdField *)this = field;
  assert( field.length() < sizeof(title)-1 );
  title[field.length()] = '\0';
}

void ClassifField::print(void) const
{
  message("Classification %s\n",title);
}

				// Field describing bit sizes of other
				// field and their relative layouts
class DFWidthField : public StdField 
{
  char field_width	       : 7;
  char cross_byte_boundaries   : 1;
  char data_width	       : 7;	// bits for the actual data
  char left_justified	       : 1;
  DEFTag apply_to;			// Mode/submode for which applicable

public:
  DFWidthField(void)
    : StdField(DEFTag(1,5),2,StdField::Exact_length), apply_to(DEFTag(6,1))
	{ cross_byte_boundaries = left_justified = 0;
	  field_width = data_width = 8;
	}
  ~DFWidthField(void) {}
  void operator = (const Field& field);
  void print(void) const;
};

void DFWidthField::operator = (const Field& field)
{
  *(StdField *)this = field;
  assert( field_width <= 16 && field_width > 0 );
  assert( data_width <= field_width && data_width > 0 );
}

void DFWidthField::print(void) const
{
  message("%d data bits within %d-bit field %s byte/word boundaries\n",
	  data_width,field_width,
	  cross_byte_boundaries ? "crossing" : "within");
  message("%s, applied to tag %s\n",
	  left_justified ? "left_justified" : "contiguos",
	  (const char *)apply_to);
}



				// Satellite Product Definition Block
class SATDefField : public StdField
{
public:
  char pi_set;			// Background projection on which SAT is valid
  char gi_set;			// Grid indicator for which the data is valid
  char sat_id[2];
  Word longit_X;		// Longitude X for the base of the product
  char resolution_code;		// in 10s of nautical miles
  char sat_type;		// 0 - visual, 1 - IR, mixed otherwise
  Word x_max;			// no. of pixels across
  Word y_max;			// no. of pixels in height
  unsigned char enhance_max;	// Needed if pixel enhancement is used
  unsigned char enhance_min;	// (limits of the gray-scale look-up referenced
  char enhance_id;		// by the enhance_id)     
  unsigned char arc_length;	// Length of each scanline in tens of minutes
				// of arc
  char x_center, y_center;	// Center of the product in units of the grid
  Word latitude;		// LAT and LON of the center of the product in
  Word longitude;		// hundredths of degree
  unsigned char title_len;
  char title[256];
  
public:
  SATDefField(const Field& field);
  ~SATDefField(void) {}
  void print(void) const;
};

SATDefField::SATDefField(const Field& field)
	: StdField(DEFTag(6,020),13,StdField::Min_length)
{
  *(StdField *)this = field;
  title[title_len] = '\0';
}


void SATDefField::print(void) const
{
  message("\nSAT image '%s' id %.2s type %d\n",title,sat_id,sat_type);
  message("%dx%d pixels, enhancement code %d, for pixel interval %d-%d\n",
	  (unsigned short)x_max,(unsigned short)y_max, enhance_id,
	  enhance_min,enhance_max);
  message("Background projection %d, grid %d, base longitude %d\n",
	  pi_set,gi_set, (short)longit_X);
  message("scanline resolution %f nautical miles, or %d' of grid\n",
	  resolution_code/10.,10*arc_length);
  message("Image center is at (%d,%d) on the grid, or %2.2f LAT %2.2f LONG\n",
	  x_center,y_center,((short)latitude)/100.,((short)longitude)/100.);
}

				// Pixel product definition block
class PixelDefField : public StdField 
{
  char pi_set;		// Define the background projection for the product
  char matrix_code;	// Code for standard SAT dimensions
  char scan_code;	// Raster scan directions (1 - from top to bottom)
  char pack_code;	// Pixel pack code

public:
  PixelDefField(void)
    : StdField(DEFTag(6,030),2,StdField::Exact_length)
	{ pi_set = 0; matrix_code = -1; scan_code = 1; pack_code = 0; }
  ~PixelDefField(void) {}
  void operator = (const Field& field);
  void print(void) const;
};

void PixelDefField::operator = (const Field& field)
{
  *(StdField *)this = field;
  assert( matrix_code < 64 );
  assert( scan_code > 0 && scan_code <= 2 );
}

void PixelDefField::print(void) const
{
  message("\nbackground grid id %d, pixel matrix code %d\n",
	  pi_set,matrix_code);
  message("scanlines go %s packed with method labelled %d\n",
	  scan_code == 1 ? "top to bottom" : "bottom to top",
	  pack_code);
}


				// Pixel product definition block
class RasterScanField : public StdField 
{
public:
  Word xrow, ycol;	// Coordinates of the beginning of a scanline
  Word no_pixels;	// in this raster line
  unsigned char pixels [4096];	// Pixels themselves

public:
  RasterScanField(void)
    : StdField(DEFTag(6,1),3,StdField::Min_length)
	{ *(short *)&xrow = 0; *(short *)&ycol = 0; *(short *)&no_pixels = 0; }
  ~RasterScanField(void) {}
  void operator = (const Field& field);
};


void RasterScanField::operator = (const Field& field)
{
  *(StdField *)this = field;
}



main(void)
{
  EndianIO fs("/projects/r2d2/oleg/IMAGES/packed.700",ios::in);
  fs.set_bigendian();
  Field field;

  fs >> field;
  ProductIDField pid(field);
  message((const char *)pid);

  fs >> field;
  SATDefField sat_def(field);
  sat_def.print();

  RasterScanField scanline;

  PixelDefField pixel_def;	// Other fields that might show up
  ClassifField secrecy_fld;
  DFWidthField width_def;

  for(fs >> field; !(field == scanline); fs >> field)
  {
    if( field == pixel_def )
      pixel_def = field, pixel_def.print();
    else if( field == secrecy_fld )
      secrecy_fld = field, secrecy_fld.print();
    else if( field == width_def )
      width_def = field, width_def.print();
    else
      _error("Unexpected tag %s",(const char *)*(DEFTag *)&field);
  }

				// Check out that we can handle that image

  IMAGE image((unsigned short)sat_def.y_max,
	      (unsigned short)sat_def.x_max,8);

  for(; !(field == DEFTag(1,2)); fs >> field)
  {
    scanline = field;
    register int i;
//    message("xrow %d, ycol %d, %d pixels\n",(unsigned short)scanline.xrow,
//	    (unsigned short)scanline.ycol,(unsigned short)scanline.no_pixels);
    for(i=0; i<(unsigned short)scanline.no_pixels; i++)
      image((unsigned short)scanline.xrow,(unsigned short)scanline.ycol+i)
	= scanline.pixels[i];
  }
  image.display("image");
}


