// This may look like C code, but it is really -*- C++ -*-
/*
 ************************************************************************
 *
 *			Modified File class
 *	to read integers of various sizes taking the byte order
 *			    into account
 *			  and bit-stream IO
 *
 ************************************************************************
 */

#pragma implementation
#include "endian_io.h"
#include <unistd.h>
#include <errno.h>


/*
 *------------------------------------------------------------------------
 *			   Error handling
 */

void EndianIO::error(const char *descr)
{
  if( eof() )
    _error("'%s' failed because of the End-Of-File",descr);
  perror(descr);
  _error("Aborted due to the I/O error above");
}

/*
 *------------------------------------------------------------------------
 *			Opening/closing
 *
 * Note that when the EndianIO stream is opened by example, 
 * it shares the i/o buffer with the existing stream. Care should be taken
 * to not destroy the buffer when the attached stream is closed/destroyed.
 * The situation is similar to what happens when a file handle is
 * duplicated with the system function dup(). Note that creating a new
 * file buffer for the attached stream based on the dup()-ed file
 * handle is not enough. Indeed, the old (sample) stream could've read
 * some data ahead in it's buffer, so the system file pointer would not
 * correspond to data that were actually read (and consumed) by the
 * user. Though this situation can be remedied using a fstream.sync()
 * function (which sets the system file pointer to correspond to what
 * actually was read from the stream and discards the read-ahead data)
 * when the attached stream read from the file and advances the file
 * pointer, the old file should be able to recognized that fact that
 * the file was read by another stream. This problem cannot be solved
 * withoud sharing the file buffer.
 */
					// Open by example at the time
					// of constructing EndianIO stream
EndianIO::EndianIO(EndianIO& a_file)
	: fstream()
{
  open(a_file);
}

					// Deferred opening by example
EndianIO& EndianIO::open(EndianIO& a_file)
{
#if 0			// Because of the stupid implementation of filebuf:
  if( rdbuf() )		// filebuf is a part of the class fstream in GCC
    delete rdbuf();		// Destroy the old stream buffer
#endif
  _strbuf = a_file.rdbuf();		// Share the buffer of a_file
  byte_order = a_file.byte_order;
  setf(ios::dont_close | attached);
//  setf(attached);
  clear();
  return *this;
}

				// The sole purpose of this function to
				// leave the stream open when it is attached
void EndianIO::close(void)
{
  if( (flags() & attached) != attached )
    fstream::close();
}

				// If the stream was attached, detach it
				// from the buffer
EndianIO::~EndianIO(void)
{
  if( (flags() & attached) == attached )
    _strbuf = 0;
}

/*
 *------------------------------------------------------------------------
 *			   Reading routines
 */

				// Read a SHORT (2 bytes) datum item
				// in the specified byte_order
unsigned short int EndianIO::read_short(const char * op_description)
{
  unsigned char c1, c2;

  if( !get(c1) || !get(c2) ) // Read 2 consecutive bytes
    error(op_description);

  if( byte_order == MSBfirst )
    return (c1 << 8) | c2;
  else
    return (c2 << 8) | c1;
}

				// Read a LONG (4 bytes) datum item
				// in the specified byte_order
unsigned long int EndianIO::read_long(const char * op_description)
{
  unsigned char c1, c2, c3, c4;

  if( !get(c1) || !get(c2) || 			// Read 4 consecutive bytes
      !get(c3) || !get(c4) )
    error(op_description);

  if ( byte_order == MSBfirst )
    return (c1 << 24) | (c2 << 16) | (c3 << 8) | c4;
  else
    return (c4 << 24) | (c3 << 16) | (c2 << 8) | c1;
}


/*
 *------------------------------------------------------------------------
 *			Service writing routines
 */

void EndianIO::write_byte
	(const int item, const char * op_description = "")
{
  if( !put(item) )
    error(op_description);
}

				// Write out a short item as specified by
				// the byte_order
void EndianIO::write_short
	(const unsigned short item, const char* op_description = "")
{
#if 0					// The following does NOT work
  union {				// because it doesn't make sure
    unsigned short int short_int;	// that c2 follows c1
    struct { unsigned char c1,c2; } bytes;
  } int_bytes;
  int_bytes.short_int = item;
  write_byte(int_bytes.bytes.c2,"write_short, 2nd byte");
  write_byte(int_bytes.bytes.c1,"write_short, 1st byte");
#endif
  if( byte_order == MSBfirst )
    write_byte((item>>8) & 0xff,"write_short, hi byte"),
    write_byte(item & 0xff,"write_short, lo byte");
  else
    write_byte(item & 0xff,"write_short, lo byte"),
    write_byte((item>>8) & 0xff,"write_short, hi byte");
}


				// Write out a long item as specified by
				// the byte_order
void EndianIO::write_long
	(const unsigned long item, const char * op_description = "")
{
  register unsigned int t = item;
  register int i;

  if( byte_order == MSBfirst )
    for(i=24; i>=0; i-=8)
      write_byte((t >> i) & 0xff,"write_long");
  else
    for(i=0; i<4; i++)
      write_byte(t & 0xff,"write_long"), t >>= 8;
}

