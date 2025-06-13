// This may look like C code, but it is really -*- C++ -*-
/*
 ************************************************************************
 *
 *			   Class Endian_IO
 *	to read integers of various sizes taking the byte order
 *			    into account
 *			  and bit-stream IO
 *
 *   Two different byte orders are supported
 *	bigendian	- most significant byte first
 *	littleendian	- most significant byte last
 *
 ************************************************************************
 */

//#pragma once
#ifndef endian_io_h
#define endian_io_h

#pragma interface

#include <fstream.h>
#include "myenv.h"


class EndianIO : public fstream {

private:
  enum { MSBfirst, LSBfirst } byte_order;
  enum { attached = 0x40000000 };	// Mask for ios::_flags, meaning the
					// stream is attached and the buffer
					// should not be closed/destroyed
public:
  EndianIO() : fstream() { byte_order = LSBfirst; }
  EndianIO(EndianIO& file);		// Open by example. File handle of
					// 'file' is dup-ed, so closing the
					// present file would not close 'file'
  EndianIO(const char * file_name, const int mode) : 
  		fstream(file_name,mode) { byte_order = LSBfirst; }
  ~EndianIO();

				// Deferred opening by example
  EndianIO& open(EndianIO& a_file);

  void open(const char * file_name, const int mode) 
	{ fstream::open(file_name,mode); }

  void close(void);		// Close the stream

  void set_bigendian(void)  		// Most significant byte first
  				{ byte_order = MSBfirst; }
  void set_littlendian(void)		// Least significant byte first
				{ byte_order = LSBfirst; }

				// Returns TRUE if something goes wrong
				// with the I/O
  int operator ! ()	{ return !good(); }
  void error(const char *descr);

  int readable(void) const { return !(rdbuf()->_flags & _IO_NO_READS); }
  int writable(void) const { return !(rdbuf()->_flags & _IO_NO_WRITES); }

				// The following I/O functions take
				// the char string op_description
				// that tells what this operation is for
				// On error, this string is printed along
				// with the error description
  unsigned char  read_byte(const char * op_description);
  unsigned short read_short(const char * op_description);
  unsigned long  read_long(const char * op_description);

				// Note, the first operand of write_byte
				// has to be specified as 'int' rather
				// than 'unsigned char', as bizarre as
				// it may seem. Otherwise, write_byte(0xff)
				// results in the i/o error
				// 'Inappropriate IOCTL for device'
				// after some 2000-4000 writings such a byte.
  void write_byte(const int item, const char * op_description = "");
  void write_short(const unsigned short item, const char* op_description = "");
  void write_long(const unsigned long item, const char * op_description = "");
};

inline unsigned char EndianIO::read_byte(const char * op_description) 
{
  unsigned char c;
  if( !get(c) )
    error(op_description);
  return c;
}

/*
 *------------------------------------------------------------------------
 *			Bit-stream Input-Output
 */

class BitIO : public EndianIO
{
private:
  unsigned char buffer;
  const short buffer_capacity = 8;
  short bits_in_buffer;

public:
			// This is a dummy constructor. Use open() function
			// of EndianIO class to perform the actual opening
  BitIO() : EndianIO()			{ bits_in_buffer = 0; }
  ~BitIO();

  void put_bit(const char bit);		// Write a bit into the output stream
  int  get_bit(void);			// Get a bit from the input stream
  void close(void);			// Close the stream
};

			// Put a bit (0/1) into the bit stream
inline void BitIO::put_bit(const char bit)
{
  if( bit )
    buffer = (buffer << 1) | 1;
  else
    buffer <<= 1;
  if( ++bits_in_buffer == buffer_capacity )
    write_byte(buffer), bits_in_buffer = 0;
}

			// Close the stream on destructing it
inline BitIO::~BitIO()
{
  close();
}
			// Flush the bit buffer on closing the stream      
inline void BitIO::close(void)
{
					// If the buffer contains something
  if( writable() && bits_in_buffer > 0 )
  {
    buffer <<= buffer_capacity - bits_in_buffer; // Fill the rest with zeros
    write_byte(buffer);			// Flush the buffer
    bits_in_buffer = 0;
  }
  EndianIO::close();
}


			// Read a bit from the bit stream
inline int BitIO::get_bit(void)
{
  if( bits_in_buffer == 0 )
  {
    if( eof() ) 
      error("Reading an 8-bit chunk");
    get(buffer);
    bits_in_buffer = buffer_capacity;
  }
  bits_in_buffer--;
  if( buffer & 0x80 )
  {
    buffer <<= 1;
    return 1;
  }
  else
  {
    buffer <<= 1;
    return 0;
  }
}

#endif
