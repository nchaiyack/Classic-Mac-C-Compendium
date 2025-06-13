// This may look like C code, but it is really -*- C++ -*-
/*
 ************************************************************************
 *
 *			   Grayscale Images
 *
 *	     Read an image from a file in the TIFF format
 *
 * The present TIFF reader reads a Class G TIFF file (for gray-scale
 * images), See Appendix G of the TIFF specification. 
 *
 * The program really needs the following tags
 *	ImageWidth
 *	ImageLength
 *	RowsPerStrip
 *	StripOffsets
 * and can't work without them. Yet, if RowsPerStrip is left out, we
 * assume there is only one strip in the image
 *
 * In addition, class G TIFF files are required to have the following
 * tags with the following values
 *	SamplesPerPixel = 1
 *	BitsPerSample   = 4 or 8
 *	Compression	= 1 or 5(LZW)
 *	PhotometricInterpretation = 0 or 1
 * So, if these tags are specified, the program makes sure that their values
 * are those that are supposed to be for class G TIFFs.
 *
 * At present, no compression is supported.
 *
 * $Id: read_tiff.cc,v 1.1 1994/01/13 14:45:30 oleg Exp oleg $
 *
 ************************************************************************
 */

#include "image.h"
#include "endian_io.h"
#include "tiff.h"


				// Read a TIFF header from the file
				// and check it
TIFFHeader::TIFFHeader(EndianIO& file)
{
  magic = file.read_short("Reading a TIFF file magic byte");
  if( magic == TIFF_BIGENDIAN )
    file.set_bigendian();
  else if( magic == TIFF_LITTLEENDIAN )
    file.set_littlendian();
  else
    _error("Unknown magic word %x",magic);
  version = file.read_short("Reading TIFF file version");
  diroffset = file.read_long("Reading the TIFF dir offset");
}

/*
 *------------------------------------------------------------------------
 *			TIFF Directory
 */

class TIFFDirectory;

class TIFFDirReadItem : public TIFFDirEntry
{
  friend class TIFFDirectory;
				// Note, this is a private constructor
				// Be sure to call the load(file) function
				// to finish the initialization
  TIFFDirReadItem(void) { tag = 0; }
  void load(EndianIO& file);

public:
  ~TIFFDirReadItem(void) {}
  void print(void) const;
};

				// Load up the item from the file: finish
				// the construction of the object
void TIFFDirReadItem::load(EndianIO& file)
{
  tag   = file.read_short("Reading dir entry");
  type  = file.read_short("Reading dir entry");
  count = file.read_long("Reading dir entry");
  val_offset = file.read_long("Reading dir entry");
}

				// Print the contents of an item w/o frills
void TIFFDirReadItem::print(void) const
{
  message("tag %d, %d ",tag,count);
  switch(type)
  {
    case BYTE:
         message("bytes");
	 break;

    case ASCII:
         message("characters");
	 break;

    case SHORT:
         message("short ints");
	 break;

    case LONG:
         message("long ints");
	 break;

    case RATIONAL:
         message("rational numbers");
	 break;

    default:
	 _error("Invalid type %d",type);
  }
  message(" value %d (0x%x)\n",val_offset,val_offset);
}


class TIFFDirectory
{
  int no_entries;
  TIFFDirReadItem * entries;

public:
  TIFFDirectory(EndianIO& file);
  ~TIFFDirectory(void);
  void print(void) const;
				// Look up a scalar value in the directory
  unsigned long int look_up(const unsigned short tag,
			    const unsigned long default_value) const;
				// Look up a string info and read the string
  char * read_str(const unsigned short tag, EndianIO& file) const;

				// Look up an item in the directory by its
				// tag. Return 0 if not found
  TIFFDirReadItem * look_up(const unsigned short tag) const;
};

				// Load up the directory from the file
TIFFDirectory::TIFFDirectory(EndianIO& file)
{
  TIFFHeader header(file);
  assert( file.seekp(header.directory_offset()).good() );
  no_entries = file.read_short("no of entries");
  entries = new TIFFDirReadItem[no_entries];

  register int i;
  for(i=0; i<no_entries; i++)
    entries[i].load(file);

  const int offset_to_next_dir = file.read_long("reading dir");
  if( offset_to_next_dir != 0 )
    message("The TIFF file contains several images, only the first one "
	    "is going to be considered");
}

				// Destroy all entries in the directory
TIFFDirectory::~TIFFDirectory(void)
{
  assert( entries != 0 );
  delete entries;
}

				// Print the contents of the directory
void TIFFDirectory::print(void) const
{
  register int i;
  message("\nThere are %d entries in the TIFF file directory\nThey are\n",
	  no_entries);
  for(i=0;i<no_entries; i++)
    entries[i].print();
}

				// Look up an item in the directory by its
				// tag. Return 0 if not found
				// Note, we take advantage of the fact
				// that all items in the directory are
				// in the ascending order of their tags
TIFFDirReadItem * TIFFDirectory::look_up(const unsigned short tag) const
{
  int lo = 0;				// Using a binary search
  int hi = no_entries;
  while( hi > lo )
  {
    int middle = (hi+lo)/2;
    if( entries[middle].tag == tag )
      return &entries[middle];
    else if( entries[middle].tag > tag )
      hi = middle;
    else
      lo = middle+1;
  }
  return 0;				// Means not found
}


				// Look up a scalar value in the directory
unsigned long int TIFFDirectory::look_up(const unsigned short tag,
			    const unsigned long default_value) const
{
  TIFFDirEntry * entryp = look_up(tag);
  if( entryp == 0 )
    return default_value;

  assert( entryp->tag == tag);
  assert( entryp->count == 1 );		// Check that the value is scalar
					// Note, if the value is shorter
					// than long, it's LEFT justified
  switch( entryp->type )
  {
    case TIFFDirEntry::BYTE:
    case TIFFDirEntry::ASCII:
         return (entryp->val_offset) >> 24;

    case TIFFDirEntry::SHORT:
         return (entryp->val_offset) >> 16;

    case TIFFDirEntry::LONG:
    case TIFFDirEntry::RATIONAL:
         return (entryp->val_offset);

    default:
	 _error("Invalid type %d",entryp->type);
  }
}

				// Look up a string value in the directory
				// and return a dynamically allocated str
				// Return 0 if not found
char * TIFFDirectory::read_str(const unsigned short tag, EndianIO& file) const
{
  TIFFDirEntry * entryp = look_up(tag);
  if( entryp == 0 )
    return 0;
  assert( entryp->tag == tag);
  assert( entryp->type == TIFFDirEntry::ASCII );
  
  int size = entryp->count;
  if( size <= 0 )
    return 0;

  char * str = (char *)malloc(size);

  assert( file.seekp(entryp->val_offset).good() );

  file.read(str,size);
  if( str[size-1] != '\0' )		// Make sure the str is terminated
    str[size-1] = '\0';			// properly     

  return str;
}


class TIFFReadStrips
{
  int no_strips;
  int rows_per_strip;
  unsigned long int * strip_offsets;
public:
  TIFFReadStrips(const TIFFDirectory& directory,const int _rows_per_strip,
		 EndianIO& file);
  TIFFReadStrips();
  ~TIFFReadStrips(void);
  void read(IMAGE& image, EndianIO& file);
};

TIFFReadStrips::TIFFReadStrips(const TIFFDirectory& directory,
			       const int _rows_per_strip,
			       EndianIO& file)
	: rows_per_strip(_rows_per_strip)
{
  TIFFDirEntry * entryp = directory.look_up(TIFFTAG_STRIPOFFSETS);
  assure( entryp != 0, "STRIPOFFSETS tag must be present in the TIFF file");

  no_strips = entryp->count;
  assert( no_strips > 0 );
  strip_offsets = new long[no_strips];

  if( no_strips == 1 )			// The value is coded within the
  {					// dir entry itself
    if( entryp->type == TIFFDirEntry::SHORT )
      strip_offsets[0] = (entryp->val_offset) >> 16;
    else if( entryp->type == TIFFDirEntry::LONG )
      strip_offsets[0] = entryp->val_offset;
    else
      _error("StripOffsets must contain either LONG or SHORT values");
    return;
  }

					// Otherwise we got to read them,
					// I mean, the strip offsets
  assert( file.seekp(entryp->val_offset).good() );
  
  register int i;
  for(i=0; i<no_strips; i++)
    if( entryp->type == TIFFDirEntry::SHORT )
      strip_offsets[i] = file.read_short("Reading StripOffsets");
    else if( entryp->type == TIFFDirEntry::LONG )
      strip_offsets[i] = file.read_long("Reading StripOffsets");
    else
      _error("StripOffsets must contain either LONG or SHORT values");
}

TIFFReadStrips::~TIFFReadStrips(void)
{
  assert( strip_offsets != 0 );
  delete strip_offsets;
}

				// Read in the strips into the image
void TIFFReadStrips::read(IMAGE& image, EndianIO& file)
{
  register int strip;
  for(strip=0; strip<no_strips; strip++)
  {
    assert( file.seekp(strip_offsets[strip]).good() );

    register int i,j;			// Reading a strip
    for(i=strip*rows_per_strip; 
	i<(strip+1)*rows_per_strip && i < image.q_nrows(); i++)
      for(j=0; j<image.q_ncols(); j++)
	image(i,j) = file.read_byte("reading a strip");
  }
}

/*
 *========================================================================
 *     		Root module - actual IMAGE constructor
 */

void IMAGE::read_tiff(EndianIO& file, const char verbose)
{
  message("Reading the TIFF file\n");

  TIFFDirectory directory(file);
  
  if( verbose )
    directory.print();

  int no_cols = directory.look_up(TIFFTAG_IMAGEWIDTH,0);
  if( no_cols == 0 )
    _error("ImageWidth tag missing or improperly specified as 0");
  
  int no_rows = directory.look_up(TIFFTAG_IMAGELENGTH,0);
  if( no_rows == 0 )
    _error("ImageLength tag missing or improperly specified as 0");

					// Checking if we can handle this
  assert( directory.look_up(TIFFTAG_SAMPLESPERPIXEL,1) == 1 );
  assert( directory.look_up(TIFFTAG_BITSPERSAMPLE,1) == 8 );
  assert( directory.look_up(TIFFTAG_COMPRESSION,COMPRESSION_NONE) == 
	 COMPRESSION_NONE );

  int photometry = directory.look_up(TIFFTAG_PHOTOMETRIC,
				     PHOTOMETRIC_MINISBLACK);
  assert( photometry == PHOTOMETRIC_MINISBLACK ||
	  photometry == PHOTOMETRIC_MINISWHITE );


  int rows_per_strip = directory.look_up(TIFFTAG_ROWSPERSTRIP,no_rows);
  TIFFReadStrips strips(directory,rows_per_strip,file);

  allocate(no_rows,no_cols,8);

  {
    char * descr = directory.read_str(TIFFTAG_IMAGEDESCRIPTION,file);
    char * doc_name = directory.read_str(TIFFTAG_DOCUMENTNAME,file);
    if( descr != 0 || doc_name != 0 )
      if( doc_name == 0 )
	name = descr;
      else if( descr == 0 )
	name = doc_name;
      else
	name = strcat(
		 strcat(
		   strcpy((char *)malloc(strlen(descr)+strlen(doc_name)+2),
			  descr),"/"),doc_name),
        free(descr), free(doc_name);
  }


  strips.read(*this,file);

  if( photometry == PHOTOMETRIC_MINISWHITE )
    invert();

  cout << "\n" << no_cols << "x" << no_rows << "x" << q_depth() << " image '"
  	   << name << "' has been read\n";
}

