/* SerialPort.c */
/*****************************************************************************/
/*                                                                           */
/*    System Dependency Library for Building Portable Software               */
/*    Macintosh Version                                                      */
/*    Written by Thomas R. Lawrence, 1993 - 1994.                            */
/*                                                                           */
/*    This file is Public Domain; it may be used for any purpose whatsoever  */
/*    without restriction.                                                   */
/*                                                                           */
/*    This package is distributed in the hope that it will be useful,        */
/*    but WITHOUT ANY WARRANTY; without even the implied warranty of         */
/*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.                   */
/*                                                                           */
/*    Thomas R. Lawrence can be reached at tomlaw@world.std.com.             */
/*                                                                           */
/*****************************************************************************/

#include "MiscInfo.h"
#include "Audit.h"
#include "Debug.h"
#include "Definitions.h"

#ifdef THINK_C
	#pragma options(pack_enums)
#endif
#include <Serial.h>
#include <Devices.h>
#include <Events.h>
#ifdef THINK_C
	#pragma options(!pack_enums)
#endif

#include "SerialPort.h"
#include "Memory.h"
#include "EventLoop.h"
#include "Array.h"


#define DC1 (17) /* XOn character */
#define DC3 (19) /* XOff character */

/* this value is the length of time we'll wait for a write cell to drain.  if */
/* it doesn't drain in this much time, we return a timeout error. */
/* time units are in 1/60ths of a second */
#define IOTIMEOUT (6*60)  /* 6 seconds to write a byte to the output buffer */

/* this defines the size of the input buffer */
#define INPUTBUFFERSIZE (16384)

/* this defines how many bytes a single asynchronous write operation can handle. */
#define WRITECELLSIZE (384)

/* this defines how many write cells are allocated */
#define NUMWRITECELLS (16)

/* this structure is one write cell buffer, and can be used for one asynchronous */
/* write operation */
typedef struct
	{
		ParamBlockRec			MyPB;
		char							Buffer[WRITECELLSIZE];
		MyBoolean					InUseFlag;
		long							NumBytes;
	} WriteCell;


struct SerialPortRec
	{
		short						InputPortRefNum;
		short						OutputPortRefNum;
		MyBoolean				GracePeriodInEffect;
		char						InputBuffer[INPUTBUFFERSIZE];
		WriteCell				WriteCellArray[NUMWRITECELLS];
	};


struct SerialRefRec
	{
		long						PortIndex;
	};


EXECUTE(static MyBoolean				Initialized = False;)

EXECUTE(static long							RefCount = 0;)

EXECUTE(static ArrayRec*				ListOfRefs;)


/* prototype for callback routine */
static pascal void	Callback(void);


/* initialize serial port subsystem.  the user calls this.  this is not called */
/* from the normal Level 0 initialization since this module is optional. */
MyBoolean						InitializeSerialPorts(void)
	{
		ERROR(Initialized,PRERR(ForceAbort,"InitializeSerialPorts:  already initialized"));
		EXECUTE(RefCount = 0;)
#if DEBUG
		ListOfRefs = NewArray();
		if (ListOfRefs == NIL)
			{
				return False;
			}
#endif
		EXECUTE(Initialized = True;)
		return True;
	}


/* shut down serial ports */
void								ShutdownSerialPorts(void)
	{
		ERROR(!Initialized,PRERR(ForceAbort,"ShutdownSerialPorts:  not initialized"));
		ERROR(RefCount != 0,PRERR(AllowResume,
			"ShutdownSerialPorts:  some connections still open"));
#if DEBUG
		ERROR(ArrayGetLength(ListOfRefs) != 0,PRERR(AllowResume,
			"ShutdownSerialPorts:  some references have not been disposed"));
		DisposeArray(ListOfRefs);
#endif
		EXECUTE(Initialized = False;)
	}


/* request a port.  if the port can not be allocated, it will return NIL. */
SerialPortRec*			RequestSerialPort(long BitsRate, SerialRefRec* PortIdentifier,
											ParityTypes Parity, HandShakeTypes HandShake,
											DataBitTypes NumDataBits, StopBitTypes NumStopBits)
	{
		SerialPortRec*		SerialPort;
		long							Scan;
		OSErr							Error;
		long							Config;
		short							BaudTemp;
		SerShk						Shaker;

		ERROR(!Initialized,PRERR(ForceAbort,"RequestSerialPort:  not initialized"));
		/* create the memory block.  it must never relocate after this. */
		ERROR(ArrayFindElement(ListOfRefs,PortIdentifier) == -1,PRERR(ForceAbort,
			"RequestSerialPort:  invalid reference"));
		CheckPtrExistence(PortIdentifier);
		SerialPort = (SerialPortRec*)AllocPtrCanFail(sizeof(SerialPortRec),"SerialPortRec");
		if (SerialPort == NIL)
			{
			 FailurePoint1:
				return NIL;
			}
		/* open the serial port */
		switch (PortIdentifier->PortIndex)
			{
				default: /* bad serial port ID number */
					EXECUTE(PRERR(ForceAbort,"RequestSerialPort:  bad serial port ID number"));
					break;
				case 0: /* modem port */
					Error = OpenDriver("\p.AOut",&(SerialPort->OutputPortRefNum));
					if (Error != noErr)
						{
							ReleasePtr((char*)SerialPort);
							goto FailurePoint1;
						}
					Error = OpenDriver("\p.AIn",&(SerialPort->InputPortRefNum));
					if (Error != noErr)
						{
							CloseDriver(SerialPort->OutputPortRefNum);
							ReleasePtr((char*)SerialPort);
							goto FailurePoint1;
						}
					break;
				case 1: /* printer port */
					Error = OpenDriver("\p.BOut",&(SerialPort->OutputPortRefNum));
					if (Error != noErr)
						{
							ReleasePtr((char*)SerialPort);
							goto FailurePoint1;
						}
					Error = OpenDriver("\p.BIn",&(SerialPort->InputPortRefNum));
					if (Error != noErr)
						{
							CloseDriver(SerialPort->OutputPortRefNum);
							ReleasePtr((char*)SerialPort);
							goto FailurePoint1;
						}
					break;
			}
		/* clear the in-use fields of the write cells */
		for (Scan = 0; Scan < NUMWRITECELLS; Scan += 1)
			{
				SerialPort->WriteCellArray[Scan].InUseFlag = False;
			}
		/* initialize the various important parameters */
		Config = 0;
		switch (Parity)
			{
				case eParityNone:
					Config |= noParity;
					break;
				case eParityEven:
					Config |= evenParity;
					break;
				case eParityOdd:
					Config |= oddParity;
					break;
				default:
					EXECUTE(PRERR(ForceAbort,"Unsupported parity"));
					break;
			}
		switch (NumDataBits)
			{
				case e8DataBits:
					Config |= data8;
					break;
				case e7DataBits:
					Config |= data7;
					break;
				case e6DataBits:
					Config |= data6;
					break;
				case e5DataBits:
					Config |= data5;
					break;
				default:
					EXECUTE(PRERR(ForceAbort,"Unsupported number of data bits"));
					break;
			}
		switch (NumStopBits)
			{
				case eOneStopBit:
					Config |= stop10;
					break;
				case eOneAndAHalfStopBits:
					Config |= stop15;
					break;
				case eTwoStopBits:
					Config |= stop20;
					break;
				default:
					EXECUTE(PRERR(ForceAbort,"Unsupported number of stop bits"));
					break;
			}
		SerReset(SerialPort->InputPortRefNum,Config);
		SerReset(SerialPort->OutputPortRefNum,Config);
		/* set the baud rate */
		BaudTemp = GetClosestAvailableBaudRate(BitsRate,PortIdentifier);
		Control(SerialPort->OutputPortRefNum,13,&BaudTemp);
		BaudTemp = GetClosestAvailableBaudRate(BitsRate,PortIdentifier);
		Control(SerialPort->InputPortRefNum,13,&BaudTemp);
		/* setting the buffer */
		SerSetBuf(SerialPort->InputPortRefNum,&(SerialPort->InputBuffer[0]),INPUTBUFFERSIZE);
		/* setting handshake parameters */
		Shaker.xOn = DC1;
		Shaker.xOff = DC3;
		Shaker.fXOn = (HandShake == eHandShakeXonXoff);
		Shaker.fCTS = (HandShake == eHandShakeDtrCts) || (HandShake == eHandShakeCtsOnly);
		Shaker.errs = 0;
		Shaker.evts = 0;
		Shaker.fInX = (HandShake == eHandShakeXonXoff);
		Shaker.fDTR = (HandShake == eHandShakeDtrCts) || (HandShake == eHandShakeDtrOnly);
		SerHShake(SerialPort->InputPortRefNum,&Shaker);
		SerHShake(SerialPort->OutputPortRefNum,&Shaker);
		/* connection now established */
		SerialPort->GracePeriodInEffect = False;
		EXECUTE(RefCount += 1;)
		return SerialPort;
	}


/* close a port */
void								CloseSerialPort(SerialPortRec* SerialPort)
	{
		ERROR(!Initialized,PRERR(ForceAbort,"CloseSerialPort:  not initialized"));
		CheckPtrExistence(SerialPort);
		/* make sure no callbacks are waiting to be called */
		KillIO(SerialPort->OutputPortRefNum);
		KillIO(SerialPort->InputPortRefNum);
		/* dump the drivers */
		CloseDriver(SerialPort->InputPortRefNum);
		CloseDriver(SerialPort->OutputPortRefNum);
		/* release the memory */
		ReleasePtr((char*)SerialPort);
		EXECUTE(RefCount -= 1;)
	}


/* get how many serial ports there are on the system */
long								GetNumSerialPorts(void)
	{
		ERROR(!Initialized,PRERR(ForceAbort,"GetNumSerialPorts:  not initialized"));
		return 2;
	}


/* get the ID of a serial port from the list */
SerialRefRec*				GetIndexedSerialPort(long Index)
	{
		SerialRefRec*			Ref;

		ERROR(!Initialized,PRERR(ForceAbort,"GetIndexedSerialPort:  not initialized"));
		ERROR((Index < 0) || (Index >= GetNumSerialPorts()),PRERR(ForceAbort,
			"GetIndexedSerialPort:  port index is out of range"));
		Ref = (SerialRefRec*)AllocPtrCanFail(sizeof(SerialRefRec),"SerialRefRec");
		if (Ref == NIL)
			{
			 FailurePoint1:
				return NIL;
			}
		Ref->PortIndex = Index;
		EXECUTE(if (!ArrayAppendElement(ListOfRefs,Ref)) {ReleasePtr((char*)Ref); Ref = NIL;});
		return Ref;
	}


/* get the name associated with a serial port identifier */
char*								GetSerialPortName(SerialRefRec* TheIdentifier)
	{
		char*							Name;

		ERROR(!Initialized,PRERR(ForceAbort,"GetSerialPortName:  not initialized"));
		ERROR(ArrayFindElement(ListOfRefs,TheIdentifier) == -1,PRERR(ForceAbort,
			"GetSerialPortName:  invalid reference"));
		CheckPtrExistence(TheIdentifier);
		switch (TheIdentifier->PortIndex)
			{
				case 0:  /* modem port */
					Name = AllocPtrCanFail(10,"SerialPortName");
					if (Name != NIL)
						{
							CopyData("Modem Port",Name,10);
						}
					break;
				case 1:  /* printer port */
					Name = AllocPtrCanFail(12,"SerialPortName");
					if (Name != NIL)
						{
							CopyData("Printer Port",Name,12);
						}
					break;
				default:
					EXECUTE(PRERR(ForceAbort,"GetSerialPortName:  bad port number"));
					break;
			}
		return Name;
	}


/* dispose of a serial port reference */
void								DisposeSerialRef(SerialRefRec* TheIdentifier)
	{
		CheckPtrExistence(TheIdentifier);
		ERROR(ArrayFindElement(ListOfRefs,TheIdentifier) == -1,PRERR(ForceAbort,
			"DisposeSerialRef:  invalid reference"));
		EXECUTE(ArrayDeleteElement(ListOfRefs,ArrayFindElement(ListOfRefs,TheIdentifier)));
		ReleasePtr((char*)TheIdentifier);
	}


/* find out the closest available baud rate to the one requested.  if the */
/* requested baud rate is supported, it is returned.  if not, then the closest */
/* available rate is returned. */
long								GetClosestAvailableBaudRate(long RequestedRate,
											SerialRefRec* PortIdentifier)
	{
		ERROR(!Initialized,PRERR(ForceAbort,
			"GetClosestAvailableBaudRate:  not initialized"));
		ERROR(ArrayFindElement(ListOfRefs,PortIdentifier) == -1,PRERR(ForceAbort,
			"GetClosestAvailableBaudRate:  invalid reference"));
		ERROR((PortIdentifier->PortIndex < 0) || (PortIdentifier->PortIndex >= 2),
			PRERR(ForceAbort,"GetClosestAvailableBaudRate:  bad port number"));
		CheckPtrExistence(PortIdentifier);
		/* for Macintosh, this doesn't actually depend on the port kind, but on */
		/* some systems it might */
		if (RequestedRate < 300)
			{
				return 300;
			}
		else if (RequestedRate > 57600)
			{
				return 57600;
			}
		else
			{
				return RequestedRate;
			}
	}


/* find out how much data is waiting to be read */
long								NumSerialPortBytesWaitingToRead(SerialPortRec* SerialPort)
	{
		long							NumBytes;

		ERROR(!Initialized,PRERR(ForceAbort,
			"NumSerialPortBytesWaitingToRead:  not initialized"));
		CheckPtrExistence(SerialPort);
		SerGetBuf(SerialPort->InputPortRefNum,&NumBytes);
		return NumBytes;
	}


/* find out how much data is waiting to leave the local buffers */
long								NumSerialPortBytesWaitingToWrite(SerialPortRec* SerialPort)
	{
		long							Scan;
		long							Count;

		ERROR(!Initialized,PRERR(ForceAbort,
			"NumSerialPortBytesWaitingToWrite:  not initialized"));
		CheckPtrExistence(SerialPort);
		Count = 0;
		for (Scan = 0; Scan < NUMWRITECELLS; Scan += 1)
			{
				if (SerialPort->WriteCellArray[Scan].InUseFlag)
					{
						/* if the flag gets cleared right before we read it, well, too bad... */
						Count += SerialPort->WriteCellArray[Scan].NumBytes;
					}
			}
		return Count;
	}


/* read some bytes from the port buffer into the specified buffer.  it is an */
/* error to read more bytes than there are waiting. */
void								ReadSerialPort(SerialPortRec* SerialPort, long NumBytesToRead,
											char* Buffer)
	{
		OSErr							Error;
		long							OldNumBytes;

		ERROR(!Initialized,PRERR(ForceAbort,"ReadSerialPort:  not initialized"));
		ERROR(NumSerialPortBytesWaitingToRead(SerialPort) < NumBytesToRead,
			PRERR(AllowResume,"ReadSerialPort:  reading too many bytes"));
		EXECUTE(OldNumBytes = NumBytesToRead;)
		Error = FSRead(SerialPort->InputPortRefNum,&NumBytesToRead,&(Buffer[0]));
		ERROR(OldNumBytes != NumBytesToRead,PRERR(ForceAbort,
			"ReadSerialPort: [FSRead] read error -- didn't read all the bytes even though they exist"));
	}


/* submit bytes to be written.  it returns True if successful, or False if */
/* the operation timed out or another error occurred */
MyBoolean						WriteSerialPort(SerialPortRec* SerialPort, long Length, char* Data)
	{
		long						StartTime;
		short						ChosenWriteCell;
		short						Scan;

		ERROR(!Initialized,PRERR(ForceAbort,"WriteSerialPort:  not initialized"));
		CheckPtrExistence(SerialPort);

		if (Length == 0)
			{
				return True;
			}

		/* entry point for writing chunks of data */
	 ReEntryPoint:
		ChosenWriteCell = -1;
		StartTime = TickCount();
		/* here we loop until we can find a free write cell */
		while (ChosenWriteCell == -1)
			{
				/* check timeout */
				if (TickCount() - StartTime > IOTIMEOUT)
					{
						/* a timeout occurred because we looped for a while and were unable */
						/* to write any data. */
						return False;
					}
				/* look for available write cells */
				for (Scan = 0; (Scan < NUMWRITECELLS) && (ChosenWriteCell == -1); Scan += 1)
					{
						if (!SerialPort->WriteCellArray[Scan].InUseFlag)
							{
								ChosenWriteCell = Scan;
							}
					}
				/* relinquish CPU so that user can do something productive while waiting. */
				if (ChosenWriteCell == -1)
					{
						RelinquishCPUCheckCancel();
					}
			}

		/* we found a buffer, so we can set it up for the async. write */
		SerialPort->WriteCellArray[ChosenWriteCell].InUseFlag = True;
		SerialPort->WriteCellArray[ChosenWriteCell].MyPB.ioParam.ioCompletion
			= (ProcPtr)&Callback;
		SerialPort->WriteCellArray[ChosenWriteCell].MyPB.ioParam.ioVRefNum = 0;
		SerialPort->WriteCellArray[ChosenWriteCell].MyPB.ioParam.ioRefNum
			= SerialPort->OutputPortRefNum;
		SerialPort->WriteCellArray[ChosenWriteCell].MyPB.ioParam.ioBuffer
			= SerialPort->WriteCellArray[ChosenWriteCell].Buffer;
		SerialPort->WriteCellArray[ChosenWriteCell].MyPB.ioParam.ioPosMode = fsAtMark;

		/* how much to write? */
		if (Length > WRITECELLSIZE)
			{
				/* if the pending amount of data is larger than the size of a */
				/* write cell, then we only write the first writecell's worth of data */
				SerialPort->WriteCellArray[ChosenWriteCell].MyPB.ioParam.ioReqCount
					= WRITECELLSIZE;
				SerialPort->WriteCellArray[ChosenWriteCell].NumBytes = WRITECELLSIZE;
				CopyData(Data,SerialPort->WriteCellArray[ChosenWriteCell].Buffer,WRITECELLSIZE);
				PBWrite((ParamBlockRec*)&(SerialPort->WriteCellArray[ChosenWriteCell]),
					True/*async*/);
				Length -= WRITECELLSIZE;
				Data += WRITECELLSIZE;
				goto ReEntryPoint;
			}
		 else
			{
				SerialPort->WriteCellArray[ChosenWriteCell].MyPB.ioParam.ioReqCount = Length;
				SerialPort->WriteCellArray[ChosenWriteCell].NumBytes = Length;
				CopyData(Data,SerialPort->WriteCellArray[ChosenWriteCell].Buffer,Length);
				PBWrite((ParamBlockRec*)&(SerialPort->WriteCellArray[ChosenWriteCell]),
					True/*async*/);
			}
		if (TickCount() - StartTime > 1)
			{
				APRINT(("WriteBlock took %l ticks",(long)(TickCount() - StartTime)));
			}

		/* all done */
		return True;
	}


/* the callback routine must NOT be profiled */
#ifdef THINK_C
	#if __option(profile)
		#define ProfilingEnabled (True)
	#else
		#define ProfilingEnabled (False)
	#endif

	#pragma options(!profile)
#endif

static pascal void	Callback(void)
	{
#ifndef __cplusplus
		register WriteCell*		HiddenParameter;

		asm{move.l A0,HiddenParameter}
		HiddenParameter->InUseFlag = 0;
		HiddenParameter->NumBytes = 0;
#else
		/* 00000000: 2F0C               MOVE.L    A4,-(A7) */
		/* 00000002: 2848               MOVEA.L   A0,A4 */
		/* 00000004: 426C 01D0          CLR.W     $01D0(A4) */
		/* 00000008: 42AC 01D2          CLR.L     $01D2(A4) */
		/* 0000000C: 285F               MOVEA.L   (A7)+,A4 */
		asm(0x2f0c,0x2848,0x426c,0x01d0,0x42ac,0x01d2,0x285f);
#endif
	}

#ifdef THINK_C
	#if ProfilingEnabled
		#pragma options(profile)
	#endif
#endif
