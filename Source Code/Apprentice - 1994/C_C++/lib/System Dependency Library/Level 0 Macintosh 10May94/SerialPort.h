/* SerialPort.h */

#ifndef Included_SerialPort_h
#define Included_SerialPort_h

/* SerialPort module depends on */
/* MiscInfo.h */
/* Audit */
/* Debug */
/* Definitions */
/* Memory */
/* EventLoop */
/* Array */

/* definitions for serial communcation attributes */
typedef enum
	{
		eParityNone EXECUTE(= -24321),
		eParityOdd,
		eParityEven
	} ParityTypes;

typedef enum
	{
		e8DataBits EXECUTE(= -1656),
		e7DataBits,
		e6DataBits,
		e5DataBits
	} DataBitTypes;

typedef enum
	{
		eOneStopBit EXECUTE(= -4341),
		eOneAndAHalfStopBits,
		eTwoStopBits
	} StopBitTypes;

typedef enum
	{
		eHandShakeNone EXECUTE(= -341),
		eHandShakeXonXoff,
		eHandShakeDtrCts,
		eHandShakeDtrOnly,
		eHandShakeCtsOnly
	} HandShakeTypes;

struct SerialPortRec;
typedef struct SerialPortRec SerialPortRec;

struct SerialRefRec;
typedef struct SerialRefRec SerialRefRec;

/* initialize serial port subsystem.  the user calls this.  this is not called */
/* from the normal Level 0 initialization since this module is optional. */
MyBoolean						InitializeSerialPorts(void);

/* shut down serial ports */
void								ShutdownSerialPorts(void);

/* request a port.  if the port can not be allocated, it will return NIL. */
SerialPortRec*			RequestSerialPort(long BitsRate, SerialRefRec* PortIdentifier,
											ParityTypes Parity, HandShakeTypes HandShake,
											DataBitTypes NumDataBits, StopBitTypes NumStopBits);

/* close a port */
void								CloseSerialPort(SerialPortRec* SerialPort);

/* get how many serial ports there are on the system */
long								GetNumSerialPorts(void);

/* get the ID of a serial port from the list */
SerialRefRec*				GetIndexedSerialPort(long Index);

/* get the name associated with a serial port identifier (not null terminated) */
char*								GetSerialPortName(SerialRefRec* TheIdentifier);

/* dispose of a serial port reference */
void								DisposeSerialRef(SerialRefRec* TheIdentifier);

/* find out the closest available baud rate to the one requested.  if the */
/* requested baud rate is supported, it is returned.  if not, then the closest */
/* available rate is returned. */
long								GetClosestAvailableBaudRate(long RequestedRate,
											SerialRefRec* PortIdentifier);

/* find out how much data is waiting to be read */
long								NumSerialPortBytesWaitingToRead(SerialPortRec* SerialPort);

/* find out how much data is waiting to leave the local buffers */
long								NumSerialPortBytesWaitingToWrite(SerialPortRec* SerialPort);

/* read some bytes from the port buffer into the specified buffer.  it is an */
/* error to read more bytes than there are waiting. */
void								ReadSerialPort(SerialPortRec* SerialPort, long NumBytesToRead,
											char* Buffer);

/* submit bytes to be written.  it returns True if successful, or False if */
/* the operation timed out or another error occurred */
MyBoolean						WriteSerialPort(SerialPortRec* SerialPort, long Length, char* Data);

#endif
