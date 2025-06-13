// ser_v3.h
// Darrell Anderson 6/95
// based on 5/94 work by Andrew Forsberg and DA and the 1/95 update by DA

#ifndef __ser_v3__
#define __ser_v3__

#include <Serial.h>

// serial port reference parameter should be one of:
enum {
	modemPort = 0,
	printerPort = 1,
	quadraLink1 = 2,
	quadraLink2 = 3,
	quadraLink3 = 4,
	quadraLink4 = 5
};

// configuration parameter should be one of: (or variant, see <Serial.h>)
enum {
	standard_9600 = ( baud9600 | data8 | stop10 | noParity ),
	standard_19200 = ( baud19200 | data8 | stop10 | noParity ),
	standard_57600 = ( baud57600 | data8 | stop10 | noParity )
};

#define DEFAULT_TIMEOUT 120 // 2 second default timeout

// the serial reference structure.  you don't need to worry about it's internals,
// the access functions should provide all the necessary functionality
// (with the possible exception of changing the timeout value on the fly)
typedef struct SerRef {
	short inRef, outRef; // driver reference numbers
	short timeout; // how many 1/60ths of a second before a timeout occurs
} SerRef, *SerRefPtr;

//----------------------------------------------------------------
// Ser_Open allocates and fills in a SerRef structure, opens and configures
// the appropriate serial port
//----------------------------------------------------------------
// <- ref :: a pointer to a SerRefPtr (NOT a SerRef, b/c it allocates it's own)
//           use this SerRefPtr for later serial-related calls on the port.
// -> whichPort :: which serial port we're dealing with (ie modemPort, etc, from above)
// -> config :: the configuration parameter (ie standard_9600, etc, from above)
// -> timeout :: how many 1/60ths of a second before a timeout occurs, only
//               applies when reading.  (DEFAULT_TIMEOUT is available in the header)
// returns noErr if all went well
//----------------------------------------------------------------
OSErr Ser_Open( SerRefPtr *ref, short whichPort, short config, short timeout );

//----------------------------------------------------------------
// Ser_Close closes and deallocates the SerRef structure opened/created
// by Ser_Open.  Flushes any data that may still be in the port buffer.
//----------------------------------------------------------------
// -> ref :: the SerRefPtr for the port.
// returns noErr if all went well
//----------------------------------------------------------------
OSErr Ser_Close( SerRefPtr ref );

//----------------------------------------------------------------
// Ser_BytesAvailable reports the number of bytes waiting to be read
// in the port's incoming buffer.
//----------------------------------------------------------------
// -> ref :: the SerRefPtr for the port.
// returns the number of bytes in the port's buffer, or -1 on error.
//----------------------------------------------------------------
short Ser_BytesAvailable( SerRefPtr ref );

//----------------------------------------------------------------
// Ser_Read reads 'numBytes' bytes from the proper port into the 
// memory pointed to by 'data'.  
// In the event of a timeout, it reads what's available before 
// returning with an ioErr.
//----------------------------------------------------------------
// -> ref :: the SerRefPtr for the port.
// -> numBytes :: how many bytes we expect to read.
// -> data :: memory location to read the bytes into.
// returns noErr if all went well, ioErr (-36) on timeout.
//----------------------------------------------------------------
OSErr Ser_Read( SerRefPtr ref, long numBytes, Ptr data );

//----------------------------------------------------------------
// Ser_Write writes 'numBytes' from the memory pointed to by 'data' to
// the proper port.
//----------------------------------------------------------------
// -> ref :: the SerRefPtr for the port.
// -> numBytes :: how many bytes we wish to write.
// -> data :: memory location to fetch the bytes from.
// returns noErr if all went well
//----------------------------------------------------------------
OSErr Ser_Write( SerRefPtr ref, long numBytes, Ptr data );

//----------------------------------------------------------------
// Ser_Flush flushes the input buffer for a port.
//----------------------------------------------------------------
// -> ref :: the SerRefPtr for the port.
// returns noErr if all went well
//----------------------------------------------------------------
OSErr Ser_Flush( SerRefPtr ref );

#endif