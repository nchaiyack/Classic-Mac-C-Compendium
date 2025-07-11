/* serial_lib.c */

/* a collection of routines to simplify serial communication, including via a
 * quadralink card.
 *
 * 5/2/94 by Darrell Anderson and Andy Forsberg
 */
 
#include <stdio.h>
#include <string.h>
#include <Serial.h>
#include <stdlib.h>
#include <OSUtils.h>
#include <MacHeaders.h>
#include "error_dlog.h"
 #define __IN_SERIAL_LIB__
#include "serial_lib.h"
 #undef __IN_SERIAL_LIB__

// use our error window routine (see error_dlog.c & .h )
#define use_report_error 1

// PROTOTYPES for internal use only functions
void HandleError( char *errMsg, short errNum, int whichSerialPort );

// GLOBALS
short ddRef_SerialPort[ (k_NumSerialPorts*2) ];
// this keeps track of the device driver reference numbers. the user merely tells which
// port to use (with a clearly labelled #define), as a reference into this array (no need
// to force the user to keep track of these numbers, we do it internally..)
// note: we need twice as many placeholders to keep track of both an _in and an _out device
// driver reference number for each port.

// Macros for interpreting our enumerated indices (from serial_lib.h) to indices into the arrays
// ddRef_SerialPort and k_DeviceDriverNames, which hold device driver reference numbers and
// the character string names for serial ports (both _in and _out for each port)
#define InRef(which) ((short)(which*2))
#define OutRef(which) ((short)((which*2)+1))

void InitSerial( int whichSerialPort, short configurationParameter )
//
// open the serial port for reading and writing
// configure the serial port to configurationParameter
// report any problems
//
{
	OSErr theErr;
	long numBytes;
	char flushed_byte;
			
	// open the port for reading	
	theErr = OpenDriver( (char *) k_DeviceDriverNames[ InRef(whichSerialPort) ], 
							(short *) &(ddRef_SerialPort[ InRef(whichSerialPort) ]) );
							
	// look for and report any errors. (better error handling is on the to do list)
	if( theErr != 0 )
		HandleError( "can't open serial port for reading", theErr, whichSerialPort );

	
	// open the port for writing
	theErr = OpenDriver( k_DeviceDriverNames[ OutRef(whichSerialPort) ], 
							&(ddRef_SerialPort[ OutRef(whichSerialPort) ]) );
	
	// look for and report any errors. (better error handling is on the to do list)
	if( theErr != 0 )
		HandleError( "can't open serial port for writing", theErr, whichSerialPort );
	
	// configure the _out port
	theErr = SerReset( ddRef_SerialPort[ OutRef(whichSerialPort) ], configurationParameter );
	
	// look for and report any errors. (better error handling is on the to do list)
	if( theErr != 0 )
		HandleError( "can't configure serial port _out", theErr, whichSerialPort );
	
	// configure the _in port
	theErr = SerReset( (short) ddRef_SerialPort[ InRef(whichSerialPort) ], (short) configurationParameter );

	// look for and report any errors. (better error handling is on the to do list)
	if( theErr != 0 )
		HandleError( "can't configure serial port _in", theErr, whichSerialPort );
		
	// clear the input buffer
	theErr = SerGetBuf( ddRef_SerialPort[ InRef(whichSerialPort) ], &numBytes );

	// look for and report any errors
	if( theErr != 0 )
		HandleError( "can't get number of bytes in input buffer", theErr, whichSerialPort );
	
	// Flush the buffer one byte at a time until empty
	while( numBytes > 0 ) {
		ReadFromSerial(whichSerialPort,1,&flushed_byte);
		numBytes--;
	}
}

void CloseSerial( int whichSerialPort )
{
	OSErr theErr;
	
	// close the _in driver
	theErr = CloseDriver( ddRef_SerialPort[ InRef(whichSerialPort) ] );
	
	// look for and report any errors. (better error handling is on the to do list)
	if( theErr != 0 )
		HandleError( "can't close serial port", theErr, whichSerialPort );
	
	// close the _out driver
	theErr = CloseDriver( ddRef_SerialPort[ OutRef(whichSerialPort) ] );

	// look for and report any errors. (better error handling is on the to do list)
	if( theErr != 0 )
		HandleError( "can't close serial port", theErr, whichSerialPort );
}

void ReadFromSerial( int whichSerialPort, long howManyBytes, Ptr dataPtr )
//
// read 'howManyBytes' from 'whichSerialPort', reading into memory pointed to by dataPtr.
//
{
	OSErr 	theErr;
	long 	bytesRead = 0;
	short 	ddRef;	// reference to the device we're reading from
	long 	startTime;
	
	ddRef = ddRef_SerialPort[ InRef(whichSerialPort) ]; // compute this once, use many.
	
	GetDateTime( &startTime );
	
	// note: use SerGetBuf so we can timeout instead of hang..
	while( bytesRead < howManyBytes )
	{
		SerGetBuf( ddRef, &bytesRead );
		
		// timeout code: (we're using the low memory global Time.. use 'GetDateTime' to
		// be more toolbox compliant, but this is faster.
		if( Time - startTime > k_serialTimeout )
			HandleError( "serial port timed out!", 0, whichSerialPort );
	}
	
	// SerGetBytes is telling us its there for the picking.. go get it.
	theErr = FSRead( ddRef, &howManyBytes, dataPtr );
}

void WriteToSerial( int whichSerialPort, long howManyBytes, Ptr dataPtr )
//
// write 'howManyBytes' to 'whichSerialPort', reading from memory pointed to by dataPtr.
//
{
	OSErr theErr;	

	// send it out..	
	theErr = FSWrite( ddRef_SerialPort[ OutRef(whichSerialPort) ], &howManyBytes, dataPtr );
}

void HandleError( char *errMsg, short errNum, int whichSerialPort )
{
	char ErrorString[ 256 ];
	
	// we always reference ports so far, but you _could_ pass -1 (#define me please!)
	// to ignore that feature.
	if( whichSerialPort == -1 )
		sprintf( ErrorString, "%s", errMsg );
	else
		sprintf( ErrorString, "%s [Port: %s]", errMsg, 
					k_UnderstandablePortNames[ whichSerialPort ] );

	// use ReportError or just print to a console window. (a la use_report_error being set)
#if use_report_error
	ReportError( ErrorString, errNum );
#else
	printf( "%s\nError number: %d\nClick the mouse to exit.\n", ErrorString, errNum );
	while( !Button() );
	ExitToShell();
#endif
}