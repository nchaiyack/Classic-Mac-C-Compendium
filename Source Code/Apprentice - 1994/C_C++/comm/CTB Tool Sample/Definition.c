// ===========================================================================
// "Connection Tool Skeleton in C" for the Communications Toolbox
// 
//	Copyright � 1994 Peter J. Creath
//	All Rights Reserved Worldwide
// ===========================================================================

#include <CRMSerialDevices.h>
#include "ConnToolCommon.h"

// ===========================================================================
// Function prototypes
// ===========================================================================
extern pascal long	main(ConnHandle hConn, short msg, long p1, long p2, CMCompletorRecord *pCompletor);
extern CMErr	DoInit(ConnHandle hConn);
extern CMErr	DoDispose(ConnHandle hConn);
extern CMErr	DoSuspend(ConnHandle hConn);
extern CMErr	DoResume(ConnHandle hConn);
extern long		DoMenu(ConnHandle hConn, short menuID, short menuItem);
extern void		DoEvent(ConnHandle hConn, EventRecord *theEvent);
extern CMErr	DoActivate(ConnHandle hConn);
extern CMErr	DoDeactivate(ConnHandle hConn);
extern CMErr	DoIdle(ConnHandle hConn);
extern CMErr	DoAbort(ConnHandle hConn);
extern CMErr	DoRead(ConnHandle hConn, CMDataBuffer *pData, CMCompletorPtr pCompletor, long timeout);
extern CMErr	DoWrite(ConnHandle hConn, CMDataBuffer *pData, CMCompletorPtr pCompletor, long timeout);
extern CMErr	DoStatus(ConnHandle hConn, CMBufferSizes *bufferSize, long *theFlag);
extern CMErr	DoListen(ConnHandle hConn, CMCompletorPtr pCompletor, long timeout);
extern CMErr	DoAccept(ConnHandle hConn, Boolean accept);
extern CMErr	DoClose(ConnHandle hConn, CMCompletorPtr pCompletor, long timeout);
extern CMErr	DoOpen(ConnHandle hConn, CMCompletorPtr pCompletor, long timeout);
extern CMErr	DoBreak(ConnHandle hConn, long duration, CMCompletorPtr pCompletor);
extern CMErr	DoIOKill(ConnHandle hConn, short channel);
extern CMErr	DoReset(ConnHandle hConn);
extern CMErr	DoEnvirons(ConnHandle hConn, ConnEnvironRec *pEnvirons);

// ===========================================================================
// main()
// 	This function is the entry point for the 'cdef' resource.  It passes control to the appropriate
// 	subroutines, depending on the incoming message.  This can probably remain unchanged.
// ===========================================================================
pascal long main(ConnHandle hConn, short msg, long p1, long p2, CMCompletorRecord *pCompletor)
{
long			rtnValue;

	switch (msg)
		{
			case cmInitMsg:
				rtnValue = DoInit(hConn);
				break;

			case cmDisposeMsg:
				rtnValue = DoDispose(hConn);
				break;

			case cmSuspendMsg:
				rtnValue = DoSuspend(hConn);
				break;

			case cmResumeMsg:
				rtnValue = DoResume(hConn);
				break;

			case cmMenuMsg:
				rtnValue = DoMenu(hConn, p1, p2);
				break;

			case cmEventMsg:
				DoEvent(hConn, (EventRecord *)p1);
				break;

			case cmActivateMsg:
				rtnValue = DoActivate(hConn);
				break;

			case cmDeactivateMsg:
				rtnValue = DoDeactivate(hConn);
				break;

			case cmIdleMsg:
				rtnValue = DoIdle(hConn);
				break;

			case cmAbortMsg:
				rtnValue = DoAbort(hConn);
				break;

			case cmReadMsg:
				rtnValue = DoRead(hConn, (CMDataBufferPtr)p1, pCompletor, p2);
				break;

			case cmWriteMsg:
				rtnValue = DoWrite(hConn, (CMDataBufferPtr)p1, pCompletor, p2);
				break;

			case cmStatusMsg:
				rtnValue = DoStatus(hConn, (CMBufferSizes *)p1, (long *)p2);
				break;

			case cmListenMsg:
				rtnValue = DoListen(hConn, (CMCompletorPtr)p1, p2);
				break;

			case cmAcceptMsg:
				rtnValue = DoAccept(hConn, (Boolean)p1);
				break;

			case cmCloseMsg:
				rtnValue = DoClose(hConn, (CMCompletorPtr)p1, p2);
				break;

			case cmOpenMsg:
				rtnValue = DoOpen(hConn, (CMCompletorPtr)p1, p2);
				break;

			case cmBreakMsg:
				rtnValue = DoBreak(hConn, p1, (CMCompletorPtr)p2);
				break;

			case cmIOKillMsg:
				rtnValue = DoIOKill(hConn, p1);
				break;
			
			case cmResetMsg:
				rtnValue = DoReset(hConn);
				break;

			case cmEnvironsMsg:
				rtnValue = DoEnvirons(hConn, (ConnEnvironRec *)p1);
				break;
			
			default:
				rtnValue = cmNotSupported;
				break;
		}

	return (rtnValue);
}

// ===========================================================================
// DoInit()
//	This function is called in response to a cmInitMsg.  If your tool allocates space for internal
//	buffers in the .bufferArray field of the connection record, applications and the Connection
//	Manager must NOT manipulate the space.  Also, your tools is responsible for freeing the space
//	(in DoDispose).  You are not required to use the .bufferArray field.  This function should return
//	an appropriate error, if any.
// ===========================================================================
CMErr DoInit(ConnHandle hConn)
{
CMErr			rtnValue;
ConnPtr		pConn;
Ptr				tempPtr;
PrivatePtr	pPrivate;
char				handleState;

	rtnValue = noErr;
	handleState = HGetState((Handle)hConn);
	HLock((Handle)hConn);
	pConn = *hConn;
	
	pConn->flags |= cmData;											// Specify which channels this tool supports
	pConn->flags &= (~cmAttn);
	pConn->flags &= (~cmCntl);

	if (tempPtr = NewPtr(pConn->bufSizes[cmDataIn]))	// Allocate memory for the I/O buffers
		{
			pConn->bufferArray[cmDataIn] = tempPtr;

			if (tempPtr = NewPtr(pConn->bufSizes[cmDataOut]))
				{
					pConn->bufferArray[cmDataOut] = tempPtr;

					if (tempPtr = NewPtr(sizeof(PrivateRecord)))
						{
							pPrivate = (PrivatePtr)tempPtr;
							// Fill in private data here
						}
					else
						{
							DisposPtr(pConn->bufferArray[cmDataIn]);
							DisposPtr(pConn->bufferArray[cmDataOut]);
							rtnValue = MemErr;
						}
				}
			else
				{
					DisposPtr(pConn->bufferArray[cmDataIn]);
					rtnValue = MemErr;
				}
		}
	else
		{
			rtnValue = MemErr;
		}

	HSetState((Handle)hConn, handleState);

	return (rtnValue);
}

// ===========================================================================
// DoDispose()
//	This function is called in response to a cmDisposeMsg.  It should dispose of any buffers allocated
//	in response to cmInitMsg and any private data storage (referenced off of .cmPrivate in the
//	connection record).  It must NOT attempt to dispose of either .config or .oldConfig in the
//	connection record, or of the connection record itself.  Doing so WILL cause a system crash!
// ===========================================================================
CMErr DoDispose(ConnHandle hConn)
{
PrivatePtr	pPrivate;
CMErr			rtnValue;
ConnPtr		pConn;
char				handleState;

	rtnValue = noErr;
	handleState = HGetState((Handle)hConn);
	HLock((Handle)hConn);
	pConn = *hConn;
	pPrivate = (PrivatePtr)(pConn->cmPrivate);

	if (pPrivate->status & cmStatusOpen)						// If the connection is open then call CMClose on it
		{
			rtnValue = CMClose(hConn, FALSE, (ProcPtr)0L, 0, TRUE);
		}
	
	if (rtnValue == noErr)												// If the connection is happily closed, release memory
		{
			DisposPtr((Ptr)(pConn->cmPrivate));
			DisposPtr((Ptr)(pConn->bufferArray[cmDataIn]));
			DisposPtr((Ptr)(pConn->bufferArray[cmDataOut]));
		}

	HSetState((Handle)hConn, handleState);

	return (rtnValue);
}

// ===========================================================================
// DoSuspend()
//	This function is called in response to a cmSuspendMsg.  This should do anything needed (like
//	remove a menu from the menu bar) in response to a suspend message.
// ===========================================================================
CMErr	DoSuspend(ConnHandle hConn)
{
CMErr	rtnValue;
PrivatePtr	pPrivate;

	rtnValue = noErr;
	pPrivate = (PrivatePtr)((**hConn).cmPrivate);
	
	return (rtnValue);
}

// ===========================================================================
// DoResume()
//	This function is called in response to a cmResumeMsg.  This should do anything needed (like add
//	a menu to the menu bar) in response to a resume message.
// ===========================================================================
CMErr	DoResume(ConnHandle hConn)
{
CMErr	rtnValue;
PrivatePtr	pPrivate;

	rtnValue = noErr;
	pPrivate = (PrivatePtr)((**hConn).cmPrivate);
	
	return (rtnValue);
}

// ===========================================================================
// DoMenu()
// 	This function handles the any menu event passed to it.  It should return FALSE if the menu was not
//	handled and TRUE if it was.
// ===========================================================================
long		DoMenu(ConnHandle hConn, short menuID, short menuItem)
{
long		rtnValue;
Boolean	isMine;

	rtnValue = noErr;
	isMine = FALSE;								// Perform some check here to see if the menu belongs to this tool
	
	if (isMine)
		{
			rtnValue = TRUE;
			// process the menu command
		}

	return (rtnValue);
}

// ===========================================================================
// DoEvent()
//	This subroutine is called in response to a cmEventMsg.  It is called when an event occurs in a
//	window associated with the connection tool.
// ===========================================================================
void		DoEvent(ConnHandle hConn, EventRecord *theEvent)
{
DialogPtr	theDialog;
short		theItem;

#define CancelButton 2

	if (IsDialogEvent(theEvent))
		{
			if (DialogSelect(theEvent, &theDialog, &theItem))			// Find out which item was hit
				{
					switch (theItem)
						{
							case CancelButton:
								// Cancel the connection
								break;
						}
				}
		}
	else
		{
			// Handle the keyDown, updateEvt, mouseDown, and any other event here
		}
}

// ===========================================================================
// DoActivate()
//	This function is called in response to a cmActivateMsg.  This should do anything needed (like add
//	a menu to the menu bar) in response to an activate message.
// ===========================================================================
CMErr	DoActivate(ConnHandle hConn)
{
CMErr	rtnValue;
PrivatePtr	pPrivate;

	rtnValue = noErr;
	pPrivate = (PrivatePtr)((**hConn).cmPrivate);
	
	return (rtnValue);
}

// ===========================================================================
// DoDeactivate()
//	This function is called in response to a cmDeactivateMsg.  This should do anything needed (like
//	remove a menu from the menu bar) in response to a deactivate message.
// ===========================================================================
CMErr	DoDeactivate(ConnHandle hConn)
{
CMErr	rtnValue;
PrivatePtr	pPrivate;

	rtnValue = noErr;
	pPrivate = (PrivatePtr)((**hConn).cmPrivate);
	
	return (rtnValue);
}

// ===========================================================================
// DoIdle()
//	This function should check the status of any asynchronous routines and make sure all is well.
// ===========================================================================
CMErr	DoIdle(ConnHandle hConn)
{
CMErr	rtnValue;
PrivatePtr	pPrivate;

	rtnValue = noErr;
	pPrivate = (PrivatePtr)((**hConn).cmPrivate);
	
	return (rtnValue);
}

// ===========================================================================
// DoAbort()
//	This function is called in response to a cmAbortMsg.  It should abort the pending listen or open
//	process.  If no listen or open processes are pending, it should return an error.
// ===========================================================================
CMErr	DoAbort(ConnHandle hConn)
{
CMErr	rtnValue;
PrivatePtr	pPrivate;

	rtnValue = noErr;
	pPrivate = (PrivatePtr)((**hConn).cmPrivate);
	
	if (pPrivate->status & cmStatusOpening)				// If we're trying to open a connection
		{
			// Stop opening
		}
	else
		{
			if (pPrivate->status & cmStatusListenPend)	// If we're listening for a connection
				{
					// Stop listening
				}
			else
				{
					return (cmNoRequestPending);				// We weren't doing either...
				}
		}

	// Close the physical layer driver

	return (rtnValue);
}

// ===========================================================================
// DoRead()
//	This function is called in response to a cmReadMsg.  It should read data into the buffers specified
//	by pData.  If a channel is requested that is not supported by this tool, you should return a
//	cmNotSupported error.  If the tools does not complete the read within the specified time
//	(timeout, in ticks), it should pass back a timeout error.  If the timeout is -1, then there is no
//	timeout.  If the timeout is 0, this should read as many bytes, up to toRead bytes, as it can in
//	one read attempt.
// ===========================================================================
CMErr DoRead(ConnHandle hConn, CMDataBuffer *pData, CMCompletorPtr pCompletor, long timeout)
{
PrivatePtr	pPrivate;
CMErr			rtnValue;
ConnPtr		pConn;
Boolean			doAsync;
short			handleState;

	rtnValue = noErr;
	pData->flags = 0;												// Set flags to zero, this tool does not support it
	handleState = HGetState((Handle)hConn);
	HLock((Handle)hConn);
	pConn = *hConn;
	pPrivate = (PrivatePtr)(pConn->cmPrivate);

	if (pPrivate->status & cmStatusOpen)				// Is a connection open?
		{
			if (pData->channel == cmData)
				{
					doAsync = FALSE;
					if (pCompletor)	doAsync = pCompletor->async;

					// If async read then install VBL task to check timeout
					// Else check the available data to read in the driver buffer
					
					rtnValue = PBRead((ParmBlkPtr)&(pPrivate->myRBlk.theParamBlk), doAsync);
					
					if (rtnValue == noErr)
						{
							if ((pPrivate->myRBlk.theParamBlk.ioActCount == 0) && doAsync)
								{
									pData->count = 0;
									pConn->asyncCount[cmDataIn] = 0;
								}
							else
								{
									pData->count = pPrivate->myRBlk.theParamBlk.ioActCount;
									pConn->asyncCount[cmDataIn] = pPrivate->myRBlk.theParamBlk.ioActCount;
								}
						}
					else													// There was an error during the read
						{
							pData->count = 0;
							pConn->errCode = rtnValue;
						}
				}
			else															// We currently only support the data channel
				{
					rtnValue = cmNotSupported;
				}
		}
	else																	// There is no open connection
		{
			rtnValue = cmNotOpen;
		}

	HSetState((Handle)hConn, handleState);

	return (rtnValue);
}

// ===========================================================================
// DoWrite()
//	This function is called in response to a cmWriteMsg.  It should write data from the buffers
//	specified by pData.  If a channel is requested that is not supported by this tool, you should return
//	a cmNotSupported error.  If the tools does not complete the write within the specified time
//	(timeout, in ticks), it should pass back a timeout error.  If the timeout is -1, then there is no
//	timeout.  If the timeout is 0, this should write as many bytes as it can in one write attempt.
// ===========================================================================
CMErr DoWrite(ConnHandle hConn, CMDataBuffer *pData, CMCompletorPtr pCompletor, long timeout)
{
PrivatePtr	pPrivate;
CMErr			rtnValue;
Boolean			doAsync;
ConnPtr		pConn;
short			handleState;

	rtnValue = noErr;
	pData->flags = 0;												// Set flags to zero, this tool does not support it
	handleState = HGetState((Handle)hConn);
	HLock((Handle)hConn);
	pConn = *hConn;
	pPrivate = (PrivatePtr)(pConn->cmPrivate);

	if ((pPrivate->status & cmStatusOpen))			// Is a connection open?
		{
			if (pData->channel == cmData)
				{
					doAsync = FALSE;
					if (pCompletor)	doAsync = pCompletor->async;
					
					// If async write then install VBL task to check timeout
					// Else check the available data to write to the driver buffer
					
					rtnValue = PBWrite((ParmBlkPtr)&(pPrivate->myWBlk.theParamBlk), doAsync);
					
					if (rtnValue == noErr)
						{
							if ((pPrivate->myWBlk.theParamBlk.ioActCount == 0) && doAsync)
								{
									pData->count = 0;
									pConn->asyncCount[cmDataIn] = 0;
								}
							else
								{
									pData->count = pPrivate->myWBlk.theParamBlk.ioActCount;
									pConn->asyncCount[cmDataIn] = pPrivate->myWBlk.theParamBlk.ioActCount;
								}
						}
					else													// There was an error during the write
						{
							pData->count = 0;
							pConn->errCode = rtnValue;
						}
				}
			else															// We currently only support the data channel
				{
					rtnValue = cmNotSupported;
				}
		}
	else																	// There is no open connection
		{
			rtnValue = cmNotOpen;
		}

	HSetState((Handle)hConn, handleState);

	return (rtnValue);
}

// ===========================================================================
// DoStatus()
//	This function is called in response to a cmStatusMsg.  It should return the status of the
//	connection in hConn.  It should also return the sizes of the connection's buffers.  Finally, it
//	should return the appropriate OSErr or CMErr.
// ===========================================================================
CMErr DoStatus(ConnHandle hConn, CMBufferSizes *bufferSize, long *theFlag)
{
PrivatePtr	pPrivate;
CMErr			rtnValue;
long				count;

	rtnValue = noErr;
	pPrivate = (PrivatePtr)((**hConn).cmPrivate);

	*theFlag = pPrivate->status;
	if (pPrivate->status & cmStatusOpen)
		{
			rtnValue = SerGetBuf(pPrivate->outRefNum, &count);	// Check output driver buffer
			*bufferSize[cmDataOut] = count;
			rtnValue = SerGetBuf(pPrivate->inRefNum, &count);		// Check input driver buffer
			*bufferSize[cmDataIn] = count;
			if (count > 0)
				{
					*theFlag |= cmStatusDataAvail;								// Set the data available bit
					*theFlag |= cmStatusOpen;									// The connection is established
				}
		}
	else
		{
			*bufferSize[cmDataIn] = 0;
			*theFlag = 0;
		}

	return (rtnValue);
}


// ===========================================================================
// DoListen()
//	This function is called in response to a cmListenMsg.  It should wait for an incoming connection
//	request.  NOTE:  pCompletor is created in a local stack frame, so copy any of its contents that
//	will be needed later.
// ===========================================================================
CMErr	DoListen(ConnHandle hConn, CMCompletorPtr pCompletor, long timeout)
{
PrivatePtr	pPrivate;
CMErr			rtnValue;
long				count;

	rtnValue = noErr;
	pPrivate = (PrivatePtr)((**hConn).cmPrivate);

	if (pPrivate->status & cmStatusOpen)
		{
			rtnValue = cmNotClosed;						// The connection is already open. 
		}
	else
		{
			// Establish the physical layer driver (open the serial port, etc.)

			if (pCompletor->async)
				{
					// Do async listen call
					
					pPrivate->status |= cmStatusListenPend;
					
					// Issue VBL task to terminate the listen in a specified timeout
				}
			else
				{
					// Do sync listen call and return error when timeout
				}
		}
	
	return (rtnValue);
}

// ===========================================================================
// DoAccept()
//	This function is called in response to a cmAcceptMsg.  It should accept or reject an incoming
//	call based on the value of "accept" passed to it, it should clear the cmStatusIncomingCallPresent
//	bit, and it should return the error, if any.
// ===========================================================================
CMErr DoAccept(ConnHandle hConn, Boolean accept)
{
PrivatePtr	pPrivate;
CMErr			rtnValue;

	rtnValue = noErr;
	pPrivate = (PrivatePtr)((**hConn).cmPrivate);
	
	if (pPrivate->status & cmStatusOpen)							// If the connection is already open, yell at the caller
		{
			rtnValue = cmNotClosed;
		}
	else
		{
			if (accept)
				{
					pPrivate->status |= cmStatusOpen;				// Set the open status bit
				}
			else
				{
					// Terminate the logical connection listen process (hang up the modem, etc.)
					// Close the physical layer driver (close the serial port, etc.)
				}

			pPrivate->status &= ~cmStatusIncomingCallPresent;
		}

	return (rtnValue);
}

// ===========================================================================
// DoClose()
//	This function is called in response to a cmCloseMsg.  NOTE:  pCompletor is created in a local stack
//	frame, so copy any of its contents that will be needed later.
// ===========================================================================
CMErr DoClose(ConnHandle hConn, CMCompletorPtr pCompletor, long timeout)
{
PrivatePtr	pPrivate;
CMErr			rtnValue;
short			err;

	rtnValue = noErr;
	pPrivate = (PrivatePtr)((**hConn).cmPrivate);

	if (pPrivate->status & cmStatusOpen)
		{
			// If break pending, kill break VBL
			
			// If now, kill pending reads and writes
			// Else wait for pending reads and writes to clear
			
			if (err = CloseDriver(pPrivate->inRefNum))	rtnValue = err;			// Close input and output drivers
			if (err = CloseDriver(pPrivate->outRefNum))	rtnValue = err;

			// Call completor routine here if async is closed
		}
	else
		{
			rtnValue = cmNotOpen;
		}

	return (rtnValue);
}

// ===========================================================================
// DoOpen()
//	This function is called in response to a cmOpenMsg.  NOTE:  pCompletor is created in a local stack
//	frame, so copy any of its contents that will be needed later.  This routine is almost entirely
//	protocol-specific.
// ===========================================================================
CMErr DoOpen(ConnHandle hConn, CMCompletorPtr pCompletor, long timeout)
{
PrivatePtr		pPrivate;
CMErr				rtnValue;
ConfigPtr			pConfig;
char					handleState;
CRMSerialPtr	pSerial;

	rtnValue = noErr;
	pPrivate = (PrivatePtr)((**hConn).cmPrivate);
	pConfig = (ConfigPtr)((**hConn).config);
	
	/* pSerial = (CRMSerialPtr)GetSerialPtr(pConfig->portName);	// Get the CRM device info
	// You get to roll your own GetSerialPtr...use CRMSearch...*/

	// Check if drivers are already open.
	// If they're already open, warn the application
	
	// First open the output driver, then the input driver
	
	handleState = HGetState((Handle)(pSerial->outputDriverName));
	HLock((Handle)(pSerial->outputDriverName));
	rtnValue = OpenDriver(*(pSerial->outputDriverName), &(pPrivate->outRefNum));
	HSetState((Handle)(pSerial->outputDriverName), handleState);
	if (rtnValue == 0)																	// Output opened successfully
		{
			handleState = HGetState((Handle)(pSerial->inputDriverName));
			HLock((Handle)(pSerial->inputDriverName));
			rtnValue = OpenDriver(*(pSerial->inputDriverName), &(pPrivate->inRefNum));
			HSetState((Handle)(pSerial->inputDriverName), handleState);
			if (rtnValue == 0)															// Input opened successfully
				{
					pPrivate->status = cmStatusOpen;
				}
			else																				// Input open failed
				{
					CloseDriver(pPrivate->outRefNum);						// So close the output driver
				}
		}

	// Call completor routine here if async is open

	return (rtnValue);
}

// ===========================================================================
// DoBreak()
//	This function is called in response to a cmBreakMsg.  NOTE:  pCompletor is created in a local
//	stack frame, so copy any of its contents that will be needed later.
// ===========================================================================
CMErr DoBreak(ConnHandle hConn, long duration, CMCompletorPtr pCompletor)
{
PrivatePtr	pPrivate;
CMErr			rtnValue;
short			err;
long				foo;

	rtnValue = noErr;
	pPrivate = (PrivatePtr)((**hConn).cmPrivate);
	
	if (pPrivate->status & cmStatusOpen)
		{
			if (pPrivate->breakPending)
				{
					rtnValue = noErr;
				}
			else
				{
					if (pCompletor->async)
						{
							// Do it asynchronously
							// Start the break
							// Start a timer (VBL or such) when it finishes it will turn off the break and
							// then call the completion routine if necessary.
						}
					else
						{
							// Start the break;
							Delay(duration, &foo);
							// End the break;
						}
				}
		}
	else
		{
			rtnValue = cmNotOpen;
		}

	return (rtnValue);
}

// ===========================================================================
// DoIOKill()
//	This function is called in response to a cmIOKillMsg.  It should terminate a pending asynchronous
//	input or output request.
// ===========================================================================
CMErr DoIOKill(ConnHandle hConn, short channel)
{
PrivatePtr	pPrivate;
CMErr			rtnValue;
HIOParam		localBlk;

	rtnValue = noErr;
	pPrivate = (PrivatePtr)((**hConn).cmPrivate);
	
	switch (channel)
		{
			case cmDataIn:
				localBlk.ioRefNum = pPrivate->myRBlk.theParamBlk.ioRefNum;
				break;

			case cmDataOut:
				localBlk.ioRefNum = pPrivate->myWBlk.theParamBlk.ioRefNum;
				break;

			default:
				return (cmNotSupported);
		}
		
	localBlk.ioCompletion = 0L;
	rtnValue = PBKillIO((ParmBlkPtr)&localBlk, FALSE);
	if (rtnValue) (**hConn).errCode = rtnValue;

	return (rtnValue);
}

// ===========================================================================
// DoReset()
//	This function is called in response to a cmResetMsg.  It is entirely protocol-specific.
// ===========================================================================
CMErr DoReset(ConnHandle hConn)
{
PrivatePtr	pPrivate;
CMErr			rtnValue;

	rtnValue = noErr;
	pPrivate = (PrivatePtr)((**hConn).cmPrivate);
	
	// Reset your connection (protocol-dependent)
	
	return (rtnValue);
}

// ===========================================================================
// DoEnvirons()
//	This function is called in response to a cmEnvironsMsg.  It should return information about the
//	connection environment.
// ===========================================================================
CMErr DoEnvirons(ConnHandle hConn, ConnEnvironRec *pEnvirons)
{
PrivatePtr	pPrivate;
CMErr			rtnValue;
ConfigPtr		pConfig;

	rtnValue = noErr;
	pConfig = (ConfigPtr)((**hConn).config);
	
	if (pEnvirons->version < curConnEnvRecVers)
		{
			rtnValue = envBadVers;
		}
	else
		{
			if (pEnvirons->version > 1)
				{
					rtnValue = envVersTooBig;
				}

			pEnvirons->dataBits				= pConfig->dataBits;
			pEnvirons->baudRate				= pConfig->baudRate;
			pEnvirons->swFlowControl	= (pConfig->shaker.fInX && pConfig->shaker.fXOn);
			pEnvirons->hwFlowControl	= (pConfig->shaker.fDTR && pConfig->shaker.fCTS);
			pEnvirons->flags					= 0;
			pEnvirons->channels				= cmData;
		}

	return (rtnValue);
}
