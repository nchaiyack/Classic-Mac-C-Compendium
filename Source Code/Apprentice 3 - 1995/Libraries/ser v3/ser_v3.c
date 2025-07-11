// ser_v3.c
// Darrell Anderson 6/95
// based on 5/94 work by Andrew Forsberg and DA and the 1/95 update by DA

#include "ser_v3.h"
#include <Files.h>

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
OSErr Ser_Open( SerRefPtr *ref, short whichPort, short config, short timeout ) {
	OSErr err;

	// sanity check!
	if( ref == nil ) return(1);

	// allocate a SerRef
	*ref = (SerRefPtr) NewPtr( sizeof(SerRef) );
	err = MemError();
	if(err) return(err);

	// copy in the timeout value
	(*ref)->timeout = timeout;
	
	// open the appropriate port for reading and writing
	switch( whichPort ) {
		case modemPort:
			err = OpenDriver( "\p.AIn", &((*ref)->inRef) );
			if(err) return(err);
			err = OpenDriver( "\p.AOut", &((*ref)->outRef) );
			if(err) return(err);
			break;
		case printerPort:
			err = OpenDriver( "\p.BIn", &((*ref)->inRef) );
			if(err) return(err);
			err = OpenDriver( "\p.BOut", &((*ref)->outRef) );
			if(err) return(err);
			break;
		case quadraLink1:
			err = OpenDriver( "\p.QuadraLink Port 1InD", &((*ref)->inRef) );
			if(err) return(err);
			err = OpenDriver( "\p.QuadraLink Port 1OutD", &((*ref)->outRef) );
			if(err) return(err);
			break;
		case quadraLink2:
			err = OpenDriver( "\p.QuadraLink Port 2InD", &((*ref)->inRef) );
			if(err) return(err);
			err = OpenDriver( "\p.QuadraLink Port 2OutD", &((*ref)->outRef) );
			if(err) return(err);
			break;
		case quadraLink3:
			err = OpenDriver( "\p.QuadraLink Port 3InD", &((*ref)->inRef) );
			if(err) return(err);
			err = OpenDriver( "\p.QuadraLink Port 3OutD", &((*ref)->outRef) );
			if(err) return(err);
			break;
		case quadraLink4:
			err = OpenDriver( "\p.QuadraLink Port 4InD", &((*ref)->inRef) );
			if(err) return(err);
			err = OpenDriver( "\p.QuadraLink Port 4OutD", &((*ref)->outRef) );
			if(err) return(err);
			break;
		default:
			return( 1 );
			break;
	}

	// configure the port
	err = SerReset( (*ref)->inRef, config );	
	if(err) return(err);
	err = SerReset( (*ref)->outRef, config );	
	if(err) return(err);
		
	// flush the input buffer
	err = Ser_Flush( *ref );
	if(err) return(err);

	return( noErr );	
}

//----------------------------------------------------------------
// Ser_Close closes and deallocates the SerRef structure opened/created
// by Ser_Open.  Flushes any data that may still be in the port buffer.
//----------------------------------------------------------------
// -> ref :: the SerRefPtr for the port.
// returns noErr if all went well
//----------------------------------------------------------------
OSErr Ser_Close( SerRefPtr ref ) {
	OSErr err;

	// sanity check!
	if( ref == nil ) return(1);

	// close the input and output drivers
	err = CloseDriver( ref->inRef );
	if(err) return(err);
	err = CloseDriver( ref->outRef);
	if(err) return(err);
	
	// deallocate the SerRef
	DisposePtr( (Ptr)ref );
	
	return( noErr );	
}

//----------------------------------------------------------------
// Ser_BytesAvailable reports the number of bytes waiting to be read
// in the port's incoming buffer.
//----------------------------------------------------------------
// -> ref :: the SerRefPtr for the port.
// returns the number of bytes in the port's buffer, or -1 on error.
//----------------------------------------------------------------
short Ser_BytesAvailable( SerRefPtr ref )  {
	OSErr err;
	long howMuch;
	
	// query the driver
	err = SerGetBuf( ref->inRef, &howMuch );
	if(err)	return( -1 );
	else return( howMuch );
}

//----------------------------------------------------------------
// Ser_Read reads 'numBytes' bytes from the proper port into the 
// memory pointed to by 'data'.  In the event of a timeout, it 
// reads what's available before returning with an ioErr.
//----------------------------------------------------------------
// -> ref :: the SerRefPtr for the port.
// -> numBytes :: how many bytes we expect to read.
// -> data :: memory location to read the bytes into.
// returns noErr if all went well, ioErr (-36) on timeout.
//----------------------------------------------------------------
OSErr Ser_Read( SerRefPtr ref, long numBytes, Ptr data ) {
	OSErr err;
	long scratch;
	long bytesAvailable;
	short numDelays = 0;
	
	// busy wait up to the SerRef's timeout value for enough data
	// to be available on the port
	while( Ser_BytesAvailable(ref) < numBytes ) {
		Delay( 1L, &scratch );
		if( numDelays++ > ref->timeout ) {
			// timeout!
		
			// read what we can..
			bytesAvailable = Ser_BytesAvailable(ref);
			if( bytesAvailable == -1 ) return( ioErr );
		
			err = FSRead( ref->inRef, &bytesAvailable, data );
			if(err) return(err);
		
			// zero out the rest
			for( ; bytesAvailable < numBytes ; bytesAvailable++ ) {
				*((char *)data + bytesAvailable) = '\0';
			}
		
			return( ioErr );	
		}
	}
	
	// otherwise the data is here, read it
	err = FSRead( ref->inRef, &numBytes, data );
	if(err) return(err);

	return( noErr );	
}

//----------------------------------------------------------------
// Ser_Write writes 'numBytes' from the memory pointed to by 'data' to
// the proper port.
//----------------------------------------------------------------
// -> ref :: the SerRefPtr for the port.
// -> numBytes :: how many bytes we wish to write.
// -> data :: memory location to fetch the bytes from.
// returns noErr if all went well
//----------------------------------------------------------------
OSErr Ser_Write( SerRefPtr ref, long numBytes, Ptr data ) {
	OSErr err;

	err = FSWrite( ref->outRef, &numBytes, data );
	if(err) return(err);

	return( noErr );	
}

//----------------------------------------------------------------
// Ser_Flush flushes the input buffer for a port.
//----------------------------------------------------------------
// -> ref :: the SerRefPtr for the port.
// returns noErr if all went well
//----------------------------------------------------------------
OSErr Ser_Flush( SerRefPtr ref ) {
	OSErr err;
	char scratch;
	short numBytes;
	
	numBytes = Ser_BytesAvailable( ref );

	while( numBytes-- ) {
		err = Ser_Read( ref, 1L, &scratch );
		if(err) return(err);
	}

	return( noErr );	
}