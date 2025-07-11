/* serial_lib.h */

// 5/2/94 by Darrell Anderson and Andy Forsberg

#ifndef __SERIAL_LIB__
#define __SERIAL_LIB__

// easy enums to reference the serial ports.  Use one of these as the parameter describing which
// port to use for each function in this lib.
//
// to expand (to account for more serial ports), just insert new entries such that all values
// 0..(the total number of ports) have been defined and that k_NumSerialPorts is the last entry.
// FOLLOW THIS GUIDE!  you'll also have to edit the device name array entries below
enum {
	modemPort = 0,
	printerPort = 1,
	quadraLink1 = 2,
	quadraLink2 = 3,
	quadraLink3 = 4,
	quadraLink4 = 5,
	k_NumSerialPorts = 6
};

// baud rates (this is a collection of magic numbers). binary OR them together to specify
// desired port features.  typical settings are illustrated below (feel free to use them or
// define your own in a similar manner). 
#ifndef __SERIAL__
enum {
    baud9600 = 10,
    baud19200 = 4,
    baud57600 = 0,
    stop10 = 16384,
    stop15 = -32768,
    stop20 = -16384,
    noParity = 0,
    oddParity = 4096,
    evenParity = 12288,
    data7 = 1024,
    data8 = 3072
};
#endif

// some sample serial port settings..
// binary OR together the features we want
#define standard_9600	( baud9600 | data8 | stop10 | noParity )			
#define standard_19200 	( baud19200 | data8 | stop10 | noParity )
#define standard_57600	( baud57600 | data8 | stop10 | noParity )

// how many seconds until we timeout (trying to read from a serial port)
#define k_serialTimeout 2

//===================================================================
// you probably won't use anything in this segment (between these '=' lines).

// these are _only_ referenced in serial_lib.c, so only that code knows about them.
#ifdef __IN_SERIAL_LIB__

// these are used internally -- don't worry about them.. change only to account
// for other ports (adding new ports, using different port names, etc)

// note: to add new ports, first change the enum at top (as per the instructions up there)
// then add the corresponding driver names ("in","out") driver names to the list below.

#define k_NoDeviceDriverNameMayHaveMoreThanThisManyCharacters 30

char k_DeviceDriverNames[(2*k_NumSerialPorts)][k_NoDeviceDriverNameMayHaveMoreThanThisManyCharacters] = 
{
	// printer port: (in,out)
		"\p.AIn","\p.AOut",
	// modem port: (in,out)
		"\p.BIn","\p.BOut",
	// quadralink ports (1 is right-most when looking at the back)
		"\p.QuadraLink Port 1InD","\p.QuadraLink Port 1OutD",
		"\p.QuadraLink Port 2InD","\p.QuadraLink Port 2OutD",
		"\p.QuadraLink Port 3InD","\p.QuadraLink Port 3OutD",
		"\p.QuadraLink Port 4InD","\p.QuadraLink Port 4OutD"
};

// these are used in error handling functions to describe which port went screwy to the user.
// proper names aren't as important here, but are nice.  they should have the same ordering as
// in the original enum.
char k_UnderstandablePortNames[k_NumSerialPorts][k_NoDeviceDriverNameMayHaveMoreThanThisManyCharacters] =
{
	"modemPort",
	"printerPort",
	"quadraLink1",
	"quadraLink2",
	"quadraLink3",
	"quadraLink4"
};

#endif

//===================================================================
// function prototypes:

void InitSerial( int whichSerialPort, short configurationParameter );

void CloseSerial( int whichSerialPort );

void ReadFromSerial( int whichSerialPort, long howManyBytes, Ptr dataPtr );

void WriteToSerial( int whichSerialPort, long howManyBytes, Ptr dataPtr );

#endif