/* Network.c */
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
#include <GestaltEqu.h>
#include <PPCToolbox.h>
#include <Script.h>
#include <Errors.h>
#ifdef THINK_C
	#pragma options(!pack_enums)
#endif

#include "Network.h"
#include "Memory.h"
#include "EventLoop.h"
#include "Array.h"


/* this is the AppleTalk PPCToolBox version of the Network module */


/* since this system can only handle one outstanding requested connection at */
/* a time, we might have to try several times to establish a connection.  these */
/* specify how many times to try and how long to wait between each one. */
#define MAXNUMTRIES (10)
#define TRYDELAY (1.0) /* in seconds */

#define DEFAULTTYPE "PPCToolBox"
#define DEFAULTTYPELENGTH (10)

/* size of a single buffer record */
#define BUFFERSIZE (256)

typedef enum
	{
		eAwaitingConnection EXECUTE(= -8982),
		eConnectionPending
	} PortStates;

struct PortIDType
	{
		PPCPortRefNum					ThePortRefnum;
		PortStates						PortState;
		PPCInformPBRec				ThePPCInformPBRec;
		PPCPortRec						ThePPCPortRec;
		PPCPortRec						RemotePortName; /* temporary buffer for it */
		LocationNameRec				RemoteLocationName; /* temporary buffer */
	};

typedef struct BufferRec
	{
		struct BufferRec*			Next;
		long									NumBytes;
		char									Buffer[BUFFERSIZE];
	} BufferRec;

typedef enum
	{
		eWriteIdle EXECUTE(= -4152),
		eWriteInProgress,
		eWriteFinished
	} SessionWriteStates;

typedef enum
	{
		eReadIdle EXECUTE(= -28874),
		eReadInProgress,
		eReadFinished
	} SessionReadStates;

typedef enum
	{
		eSessionNormal EXECUTE(= -9887),
		eSessionDataArrived,
		eSessionClosed
	} SessionStates;

struct SessionIDType
	{
		PPCSessRefNum					SessionRefnum;
		PortIDType*						Port;
		SessionWriteStates		WriteState;
		SessionReadStates			ReadState;
		SessionStates					OverallState;
		BufferRec*						SendHead;
		BufferRec*						SendTail;
		BufferRec*						SendInProgress; /* valid if WriteState != eWriteIdle */
		BufferRec*						ReceiveHead;
		BufferRec*						ReceiveTail;
		BufferRec*						ReceiveInProgress; /* valid if ReadState != eReadIdle */
		PPCReadPBRec					ThePPCReadPBRec;
		PPCWritePBRec					ThePPCWritePBRec;
		PPCPortRec						RemotePortName; /* probably not needed */
		LocationNameRec				RemoteLocationName; /* NBP name of remote machine */
	};


/* maintains the status of this module for debugging */
EXECUTE(static MyBoolean					Initialized = False;)

/* list of network ports.  NIL means the port slot is available.  the length */
/* is determined by the heap block size. */
static ArrayRec*									PortArray;

/* list of sessions.  NIL means session slot is available... */
static ArrayRec*									SessionArray;

/* This system allows you to make an outgoing connection without a port.  The */
/* Macintosh requires all connections to be made through ports, so we provide */
/* a "system port" through which all outgoing connections are made. */
static PortIDType*								AppleTalkSystemPort;

/* session scan counter.  this is used so that all of the sessions are checked */
/* fairly instead of favoring ones that are near the beginning of the list */
static long												SessionScan;


/* utility function prototypes */
static void					ReinstallPPCInform(PortIDType* Port);
static MyBoolean		DecodeMachineName(char* MachineStr, char ObjStr[32],
											char TypeStr[32], char ZoneStr[32]);
static void					UpdateRead(SessionIDType* Session);
static void					UpdateWrite(SessionIDType* Session);
static pascal void	MyPPCInformCompletionRoutine(PPCInformPBRec* PB);
static pascal void	MyPPCReadCompletionRoutine(PPCParamBlockRec* PB);
static pascal void	MyPPCWriteCompletionRoutine(PPCParamBlockRec* PB);


/* initialize the network interface.  the user calls this.  it is not called from */
/* the standard Level 0 initialization routine because not all programs need */
/* networking.  Could return eNetNoError, eNetNoMemory, eNetNetworkNotAvailable, */
/* eNetCouldntInitNet, or eNetUnknownError */
NetErrors						InitializeNetwork(void)
	{
		OSErr							Error;
		long							Result;
		NetErrors					ReturnValue;
		unsigned long			PortIDScan;

		ERROR(Initialized,PRERR(ForceAbort,"InitializeNetwork:  already initialized"));
		EXECUTE(Initialized = True;)
		Error = Gestalt(gestaltPPCToolboxAttr,&Result);
		if (Error != noErr)
			{
				ReturnValue = eNetNetworkNotAvailable;
			 FailurePoint1:
				EXECUTE(Initialized = False;)
				return ReturnValue;
			}
		if ((Result & gestaltPPCSupportsRealTime) == 0)
			{
				/* not initialized.  try initializing */
				Error = PPCInit();
				if (Error != noErr)
					{
						ReturnValue = eNetCouldntInitNet;
						goto FailurePoint1;
					}
				Error = Gestalt(gestaltPPCToolboxAttr,&Result);
			}
		/* don't know what to do in these cases */
		if ((Result & gestaltPPCSupportsOutGoing) == 0)
			{
			}
		if ((Result & gestaltPPCSupportsIncoming) == 0)
			{
			}
		/* allocate arrays for holding session and port records */
		PortArray = NewArray();
		if (PortArray == NIL)
			{
				ReturnValue = eNetNoMemory;
			 FailurePoint2:
				goto FailurePoint1;
			}
		SessionArray = NewArray();
		if (SessionArray == NIL)
			{
				ReturnValue = eNetNoMemory;
			 FailurePoint3:
				DisposeArray(PortArray);
				goto FailurePoint2;
			}
		/* initialize the session counter for NetUpdate */
		SessionScan = 0;
		/* trying to obtain a port for outgoing connections.  port names must be unique */
		/* and since there may be several programs using this package running on the */
		/* same computer, we need to search for an available port to work out of. */
		for (PortIDScan = 0xffffffff; PortIDScan > 0; PortIDScan -= 1)
			{
				char*							PortNameTemp;
				int								Scan;

				PortNameTemp = AllocPtrCanFail(8,"SysPortName");
				if (PortNameTemp == NIL)
					{
						ReturnValue = eNetNoMemory;
						goto FailurePoint3;
					}
				for (Scan = 0; Scan < 8; Scan += 1)
					{
						PortNameTemp[Scan] = 'a' + ((PortIDScan >> (Scan * 4)) & 15);
					}
				/* try to open the port */
				ReturnValue = NetListenAtPort(PortNameTemp,&AppleTalkSystemPort,eNetAppleTalk);
				ReleasePtr(PortNameTemp);
				/* see if the port worked */
				switch (ReturnValue)
					{
						default:
							EXECUTE(PRERR(ForceAbort,
								"InitializeNetwork:  bad value from NetListenAtPort"));
							break;
						case eNetNoError:
							goto SystemPortAllocatedPoint;
						case eNetNoMemory:
							/* value of ReturnValue stays the same */
							/* ReturnValue = eNetNoMemory; */
						 FailurePoint4:
							DisposeArray(SessionArray);
							goto FailurePoint3;
						case eNetPortInUse:
							break; /* loop again */
						case eNetBadPortString:
							EXECUTE(PRERR(ForceAbort,"InitializeNetwork:  bad port string"));
							break;
						case eNetUnknownError:
							/* value of ReturnValue stays the same */
							/* ReturnValue = eNetUnknownError; */
							goto FailurePoint4;
							break;
					}
			}
		/* fall through:  couldn't allocate any system ports */
		ReturnValue = eNetUnknownError;
		goto FailurePoint4;
		/* jump here if all is good */
	 SystemPortAllocatedPoint:
		return eNetNoError;
	}


/* discard any pending data, close any open connections, and clean internal data */
void								ShutdownNetwork(void)
	{
		long							Scan;
		long							Limit;

		ERROR(!Initialized,PRERR(ForceAbort,"ShutdownNetwork:  not initialized"));
		/* first, close all sessions */
		Limit = ArrayGetLength(SessionArray);
		for (Scan = 0; Scan < Limit; Scan += 1)
			{
				SessionIDType*		Session;

				Session = (SessionIDType*)ArrayGetElement(SessionArray,Scan);
				EXECUTE(PRERR(AllowResume,"ShutdownNetwork:  session still open"));
				NetCloseSession(Session);
			}
		/* now close all open ports, including the system port */
		NetTerminatePortAndSessions(AppleTalkSystemPort); /* technically not needed */
		Limit = ArrayGetLength(PortArray);
		for (Scan = 0; Scan < Limit; Scan += 1)
			{
				PortIDType*				Port;

				Port = (PortIDType*)ArrayGetElement(PortArray,Scan);
				EXECUTE(PRERR(AllowResume,"ShutdownNetwork:  found a port that's still open"));
				NetTerminatePortAndSessions(Port);
			}
		/* now release memory occupied by arrays */
		DisposeArray(PortArray);
		DisposeArray(SessionArray);
		EXECUTE(Initialized = False;)
	}


/* Network update routine.  It handles periodic update tasks and should be called */
/* frequently (preferably in the main event loop next to GetAnEvent). */
NetEvents						NetUpdate(SessionIDType** SessionNumber)
	{
		OSErr							Error;
		long							Scan;
		long							Limit;
		long							OldSessionScan;
		NetEvents					ReturnValue;

		ERROR(SessionNumber == NIL,PRERR(ForceAbort,"NetUpdate:  SessionNumber == NIL"));
		ERROR(!Initialized,PRERR(ForceAbort,"NetUpdate:  not initialized"));
		/* look for PPCInforms that completed */
		Limit = ArrayGetLength(PortArray);
		for (Scan = 0; Scan < Limit; Scan += 1)
			{
				PortIDType*				Port;

				Port = (PortIDType*)ArrayGetElement(PortArray,Scan);
				if (Port->PortState == eConnectionPending)
					{
						SessionIDType*		Session;

						/* return an event indicating the new session and reset the PPCInform */
						Session = (SessionIDType*)AllocPtrCanFail(sizeof(SessionIDType),"Session");
						if ((Session == NIL) || (Port == AppleTalkSystemPort))
							{
								PPCEndPBRec				ThePPCEndPBRec;

								/* if we are out of memory, then kill the session */
								/* we also do this if the connection was established on the */
								/* system port */
							 NewSessionFailurePoint1:
								ThePPCEndPBRec.sessRefNum = Port->ThePPCInformPBRec.sessRefNum;
								Error = PPCEnd(&ThePPCEndPBRec,False/*sync*/);
								ERROR(Error != noErr,PRERR(AllowResume,"NetUpdate:  PPCEnd != noErr"));
								ReinstallPPCInform(Port); /* this resets PortState */
								goto NextPortPoint;
							}
						if (!ArrayAppendElement(SessionArray,Session))
							{
								goto NewSessionFailurePoint1;
							}
						/* initialize all of the fields of the session record */
						Session->SessionRefnum = Port->ThePPCInformPBRec.sessRefNum;
						Session->Port = Port;
						Session->WriteState = eWriteIdle;
						Session->ReadState = eReadIdle;
						Session->OverallState = eSessionNormal;
						Session->SendHead = NIL;
						Session->SendTail = NIL;
						Session->SendInProgress = NIL;
						Session->ReceiveHead = NIL;
						Session->ReceiveTail = NIL;
						Session->ReceiveInProgress = NIL;
						/* initiate the first PPC read operation */
						UpdateRead(Session);
						/* copy over the port and location names of the remote session */
						Session->RemotePortName = Port->RemotePortName;
						Session->RemoteLocationName = Port->RemoteLocationName;
						ReinstallPPCInform(Port); /* this resets PortState */
						/* return session number for caller */
						*SessionNumber = Session;
						return eNetEventNewSession;
					}
				/* jump here after killing a session that we couldn't handle so that */
				/* we can check the next port. */
			 NextPortPoint:
				;
			}
		/* now do session scans */
		ReturnValue = eNetEventNone;
		Limit = ArrayGetLength(SessionArray);
		if (Limit > 0)
			{
				OldSessionScan = SessionScan;
				if (OldSessionScan > Limit - 1)
					{
						/* this prevents us from getting stuck in an infinite loop if someone */
						/* removed an array element and left SessionScan beyond the end of the array */
						OldSessionScan = 0;
					}
				do
					{
						SessionIDType*			Session;

						/* we increment first so that we don't dwell on hot sessions */
						SessionScan += 1;
						if (SessionScan >= Limit)
							{
								SessionScan = 0;
							}
						Session = (SessionIDType*)ArrayGetElement(SessionArray,SessionScan);
						/* check to see if port is closed */
						if (Session->OverallState == eSessionClosed)
							{
								*SessionNumber = Session;
								return eNetEventSessionClosed;
							}
						/* check to see if write needs to be retriggered */
						if (Session->WriteState != eWriteInProgress)
							{
								if (Session->WriteState == eWriteFinished)
									{
										ReturnValue = eNetEventInternal;
									}
								UpdateWrite(Session);
							}
						/* check to see if read needs to be retriggered */
						if (Session->ReadState != eReadInProgress)
							{
								if (Session->ReadState == eReadFinished)
									{
										ReturnValue = eNetEventInternal;
									}
								UpdateRead(Session);
							}
						/* check to see if there is new data */
						if (Session->OverallState == eSessionDataArrived)
							{
								Session->OverallState = eSessionNormal;
								*SessionNumber = Session;
								return eNetEventDataIncoming;
							}
					} while (SessionScan != OldSessionScan);
			}
		return ReturnValue;
	}


/* local utility routine for installing an asynchronous PPC callback */
static void					ReinstallPPCInform(PortIDType* Port)
	{
		OSErr							Error;

		Port->ThePPCInformPBRec.ioCompletion = (ProcPtr)&MyPPCInformCompletionRoutine;
		Port->ThePPCInformPBRec.portRefNum = Port->ThePortRefnum;
		Port->ThePPCInformPBRec.autoAccept = True;
		/* provide a place for identifying the remote port */
		Port->ThePPCInformPBRec.portName = &(Port->RemotePortName);
		/* provide a place for identifying the remote machine */
		Port->ThePPCInformPBRec.locationName = &(Port->RemoteLocationName);
		/* we don't support user names */
		Port->ThePPCInformPBRec.userName = NIL;
		Port->PortState = eAwaitingConnection; /* BEFORE the call */
		Error = PPCInform(&(Port->ThePPCInformPBRec),True/*async*/);
		ERROR(Error != noErr,PRERR(AllowResume,"ReinstallPPCInform:  PPCInform != noErr"));
	}


/* Listen at the specified port.  Returns the reference number of the port being */
/* listened at.  PortString is a non-null-terminated heap block containing the string */
/* identifying the port.  The port string format depends on the underlying network */
/* scheme.  For instance, TCP/IP would be an integer between 128 and 9999. */
/* for AppleTalk (NBP), it is a valid NBP name.  Could return eNetNoError, */
/* eNetNoMemory, eNetPortInUse, eNetBadPortString, eNetUnknownError, or */
/* eNetProtocolNotSupported. */
NetErrors						NetListenAtPort(char* PortString, PortIDType** PortOut,
											NetworkTypes WhichNetwork)
	{
		PPCOpenPBRec			ThePPCOpenPBRec;
		long							PortStringLength;
		PortIDType*				Port;
		OSErr							Error;
		NetErrors					ReturnValue;

		ERROR(!Initialized,PRERR(ForceAbort,"NetListenAtPort:  not initialized"));
		CheckPtrExistence(PortString);
		ERROR(PortOut == NIL,PRERR(ForceAbort,"NetListenAtPort:  PortOut is NIL"));
		/* verify network connection requested */
		switch (WhichNetwork)
			{
				case eNetDefault:
				case eNetAppleTalk:
					break;
				case eNetTCP:
					return eNetProtocolNotSupported;
				default:
					EXECUTE(PRERR(ForceAbort,"NetListenAtPort:  bad protocol specified"));
					break;
			}
		/* make a place for it */
		Port = (PortIDType*)AllocPtrCanFail(sizeof(PortIDType),"PortIDType");
		if (Port == NIL)
			{
				ReturnValue = eNetNoMemory;
			 FailurePoint1:
				return ReturnValue;
			}
		if (!ArrayAppendElement(PortArray,Port))
			{
				ReturnValue = eNetNoMemory;
			 FailurePoint2:
				ReleasePtr((char*)Port);
				goto FailurePoint1;
			}
		/* initialize the port record */
		Port->ThePPCPortRec.nameScript = smRoman;
		PortStringLength = PtrSize(PortString);
		if (PortStringLength > 31)
			{
				ReturnValue = eNetBadPortString;
			 FailurePoint3:
				ArrayDeleteElement(PortArray,ArrayFindElement(PortArray,Port));
				goto FailurePoint2;
			}
		CopyData(PortString,(char*)&(Port->ThePPCPortRec.name[1]),PortStringLength);
		Port->ThePPCPortRec.name[0] = PortStringLength; /* stupid pascal strings */
		Port->ThePPCPortRec.portKindSelector = ppcByString;
		CopyData(DEFAULTTYPE,(char*)&(Port->ThePPCPortRec.u.portTypeStr[1]),
			DEFAULTTYPELENGTH);
		Port->ThePPCPortRec.u.portTypeStr[0] = DEFAULTTYPELENGTH;
		/* initialize the Openrec */
		ThePPCOpenPBRec.serviceType = ppcServiceRealTime;
		ThePPCOpenPBRec.resFlag = 0;
		ThePPCOpenPBRec.portName = &(Port->ThePPCPortRec);
		ThePPCOpenPBRec.locationName = NIL;
		ThePPCOpenPBRec.networkVisible = True;
		/* try to open it */
		Error = PPCOpen(&ThePPCOpenPBRec,False/*synchronous*/);
		switch (Error)
			{
				case noErr:
					Port->ThePortRefnum = ThePPCOpenPBRec.portRefNum;
					break; /* continue on to the next phase */
				case badLocNameErr:
					ReturnValue = eNetBadPortString;
					goto FailurePoint3;
				case portNameExistsErr:
				case nbpDuplicate:
					ReturnValue = eNetPortInUse;
					goto FailurePoint3;
				default:
					ReturnValue = eNetUnknownError;
					goto FailurePoint3;
			}
		/* indicate that the callback is out there */
		Port->PortState = eAwaitingConnection;
		/* enable listen callback */
		ReinstallPPCInform(Port);
		/* put reference number out for caller */
		*PortOut = Port;
		return eNetNoError;
	}


/* Stop listening at a port and let the OS use it for someone else.  Sessions */
/* established through this port are terminated. */
void								NetTerminatePortAndSessions(PortIDType* Port)
	{
		OSErr							Error;
		PPCClosePBRec			TheCloseRec;
		long							Scan;
		long							Limit;

		ERROR(!Initialized,PRERR(ForceAbort,
			"NetTerminatePortAndSessions:  not initialized"));
		CheckPtrExistence(Port);
		/* close any sessions registered through this port */
		Limit = ArrayGetLength(SessionArray);
		Scan = 0;
		while (Scan < Limit)
			{
				SessionIDType*			Session;

				Session = (SessionIDType*)ArrayGetElement(SessionArray,Scan);
				if (Session->Port == Port)
					{
						/* clean up our local data structures */
						NetCloseSession(Session);
						/* decrement limit, since we just dropped current element from array */
						Limit -= 1;
					}
				 else
					{
						/* only increment this if we didn't delete the element */
						Scan += 1;
					}
			}
		/* close the port itself */
		TheCloseRec.portRefNum = Port->ThePortRefnum;
		Error = PPCClose(&TheCloseRec,False/*synchronous*/);
		ERROR(Error != noErr,PRERR(AllowResume,
			"NetTerminatePortAndSessions:  return from PPCClose != noErr"));
		ArrayDeleteElement(PortArray,ArrayFindElement(PortArray,Port));
		ReleasePtr((char*)Port);
	}


/* Open a session to another (or the same) machine.  Returns a session number in */
/* *SessionOut.  PortString is the remote port to connect to, as described above, */
/* and MachineString is a machine string determined by the network stack.  For */
/* instance, TCP/IP would support standard a.b.c.d or machine.zone.domain format. */
/* The Macintosh uses machine:type@zone.  Could return eNetNoError, eNetNoMemory, */
/* eNetBadPortString, eNetBadMachineString, eNetMachineUnknown, eNetUnknownError, */
/* eNetConnectRefused, or eNetProtocolNotSupported. */
NetErrors						NetOpenSession(char* PortString, char* MachineString,
											SessionIDType** SessionOut, NetworkTypes WhichNetwork)
	{
		PPCStartPBRec			ThePPCStartPBRec;
		OSErr							Error;
		long							PortNameLength;
		SessionIDType*		Session;
		NetErrors					ReturnValue;
		long							TryCounter;
		unsigned char			UserNamePlace[32];

		ERROR(!Initialized,PRERR(ForceAbort,"NetOpenSession:  not initialized"));
		CheckPtrExistence(PortString);
		CheckPtrExistence(MachineString);
		ERROR(SessionOut == NIL,PRERR(ForceAbort,"NetOpenSession:  SessionOut == NIL"));
		/* verify network connection requested */
		switch (WhichNetwork)
			{
				case eNetDefault:
				case eNetAppleTalk:
					break;
				case eNetTCP:
					return eNetProtocolNotSupported;
				default:
					EXECUTE(PRERR(ForceAbort,"NetListenAtPort:  bad protocol specified"));
					break;
			}
		/* allocate a place for it */
		Session = (SessionIDType*)AllocPtrCanFail(sizeof(SessionIDType),"SessionIDType");
		if (Session == NIL)
			{
				ReturnValue = eNetNoMemory;
			 FailurePoint1:
				return ReturnValue;
			}
		if (!ArrayAppendElement(SessionArray,Session))
			{
				ReturnValue = eNetNoMemory;
			 FailurePoint2:
				ReleasePtr((char*)Session);
				goto FailurePoint1;
			}
		/* initialize the ppc record */
		EXECUTE(ThePPCStartPBRec.ioCompletion = (ProcPtr)0x81818181;)
		ThePPCStartPBRec.portRefNum = AppleTalkSystemPort->ThePortRefnum;
		ThePPCStartPBRec.serviceType = ppcServiceRealTime;
		ThePPCStartPBRec.resFlag = 0;
		/* see if this connection is for this machine or another */
		PortNameLength = PtrSize(PortString);
		if (PortNameLength > 31)
			{
				ReturnValue = eNetBadPortString;
			 FailurePoint3:
				ArrayDeleteElement(SessionArray,ArrayFindElement(SessionArray,Session));
				goto FailurePoint2;
			}
		CopyData(PortString,(char*)&(Session->RemotePortName.name[1]),PortNameLength);
		Session->RemotePortName.name[0] = PortNameLength;
		Session->RemotePortName.nameScript = smRoman;
		Session->RemotePortName.portKindSelector = ppcByString;
		CopyData(DEFAULTTYPE,(char*)&(Session->RemotePortName.u.portTypeStr[1]),
			DEFAULTTYPELENGTH);
		Session->RemotePortName.u.portTypeStr[0] = DEFAULTTYPELENGTH;
		ThePPCStartPBRec.portName = &(Session->RemotePortName);
		if ((PtrSize(MachineString) != 9) && (PtrSize(MachineString) != 0))
			{
				char							MyNBPObjectStr[32];
				char							MyNBPTypeStr[32];
				char							MyNBPZoneStr[32];

				/* remote machine specified */
			 RemoteName:
				if (!DecodeMachineName(MachineString,MyNBPObjectStr,MyNBPTypeStr,MyNBPZoneStr))
					{
						ReturnValue = eNetBadMachineString;
						goto FailurePoint2;
					}
				CopyData(&(MyNBPObjectStr[0]),(char*)&(Session->RemoteLocationName
					.u.nbpEntity.objStr),MyNBPObjectStr[0] + 1);
				CopyData(&(MyNBPTypeStr[0]),(char*)&(Session->RemoteLocationName
					.u.nbpEntity.typeStr),MyNBPTypeStr[0] + 1);
				CopyData(&(MyNBPZoneStr[0]),(char*)&(Session->RemoteLocationName
					.u.nbpEntity.zoneStr),MyNBPZoneStr[0] + 1);
				Session->RemoteLocationName.locationKindSelector = ppcNBPLocation;
				ThePPCStartPBRec.locationName = &(Session->RemoteLocationName);
			}
		 else
			{
				if (PtrSize(MachineString) == 9)
					{
						long							Scan;

						for (Scan = 0; Scan < 9; Scan += 1)
							{
								if ("localhost"[Scan] != MachineString[Scan])
									{
										goto RemoteName;
									}
							}
					}
				/* it's the local machine */
				Session->RemoteLocationName.locationKindSelector = ppcNoLocation;
				ThePPCStartPBRec.locationName = &(Session->RemoteLocationName);
			}
		ThePPCStartPBRec.userData = 0; /* users are not used */
		if (noErr != GetDefaultUser(&(ThePPCStartPBRec.userRefNum),UserNamePlace))
			{
				ThePPCStartPBRec.userRefNum = 0;
			}
		TryCounter = MAXNUMTRIES;
	 TryAgainPoint:
		Error = PPCStart(&ThePPCStartPBRec,False/*sync*/);
		switch (Error)
			{
				default:
					ReturnValue = eNetUnknownError;
					goto FailurePoint3;
				case destPortErr:
				case noResponseErr:
				case userRejectErr:
				case localOnlyErr:
				case guestNotAllowedErr:
					ReturnValue = eNetConnectRefused;
					goto FailurePoint3;
				case noErr:
					break; /* continue */
				case noInformErr:
					TryCounter -= 1;
					/* sleep a little bit */
					{
						double			Now;

						Now = ReadTimer();
						while (TimerDifference(ReadTimer(),Now) < TRYDELAY)
							{
								RelinquishCPUCheckCancel();
							}
					}
					/* see if we should try again */
					if (TryCounter > 0)
						{
							goto TryAgainPoint;
						}
					ReturnValue = eNetConnectRefused;
					goto FailurePoint3;
				case portClosedErr:
				case badPortNameErr:
				case noUserRecErr:
				case noPortErr:
				case badServiceMethodErr:
				case nameTypeErr:
					EXECUTE(PRERR(ForceAbort,"NetOpenSession:  bad error code"));
					break;
			}
		/* initialize all of the structure's parameters */
		Session->SessionRefnum = ThePPCStartPBRec.sessRefNum;
		Session->Port = AppleTalkSystemPort;
		Session->WriteState = eWriteIdle;
		Session->ReadState = eReadIdle;
		Session->OverallState = eSessionNormal;
		Session->SendHead = NIL;
		Session->SendTail = NIL;
		Session->SendInProgress = NIL;
		Session->ReceiveHead = NIL;
		Session->ReceiveTail = NIL;
		Session->ReceiveInProgress = NIL;
		*SessionOut = Session;
		return eNetNoError;
	}


/* utility routine to decode string of format <name>:<type>@<zone> */
/* we allow the omission of <type> and automatically substitute DEFAULTTYPE for it */
/* you can omit the zone name as well. */
static MyBoolean		DecodeMachineName(char* MachineStr, char ObjStr[32],
											char TypeStr[32], char ZoneStr[32])
	{
		long							Limit;
		long							Scan;

		Limit = PtrSize(MachineStr);
		Scan = 0;
		while ((Scan < Limit) && (MachineStr[Scan] != ':') && (MachineStr[Scan] != '@'))
			{
				if (Scan >= 31)
					{
						return False;
					}
				ObjStr[Scan + 1] = MachineStr[Scan];
				Scan += 1;
			}
		ObjStr[0] = Scan;
		if ((Scan == Limit) || (MachineStr[Scan] == '@'))
			{
				/* type name omitted */
				CopyData(DEFAULTTYPE,&(TypeStr[1]),DEFAULTTYPELENGTH);
				TypeStr[0] = DEFAULTTYPELENGTH;
				goto ElidedTypePoint;
			}
		MachineStr += Scan + 1;
		Limit -= Scan + 1;
		Scan = 0;
		while ((Scan < Limit) && (MachineStr[Scan] != '@'))
			{
				if (Scan >= 31)
					{
						return False;
					}
				TypeStr[Scan + 1] = MachineStr[Scan];
				Scan += 1;
			}
		TypeStr[0] = Scan;
	 ElidedTypePoint:
		if (Scan == Limit)
			{
				/* no zone name */
				ZoneStr[0] = 0;
				return True;
			}
		MachineStr += Scan + 1;
		Limit -= Scan + 1;
		Scan = 0;
		while (Scan < Limit)
			{
				if (Scan >= 31)
					{
						return False;
					}
				ZoneStr[Scan + 1] = MachineStr[Scan];
				Scan += 1;
			}
		ZoneStr[0] = Scan;
		return True;
	}


/* Close a session.  Any waiting data in either direction is discarded. */
void								NetCloseSession(SessionIDType* Session)
	{
		OSErr							Error;
		PPCEndPBRec				EndRec;
		BufferRec*				BuffScan;

		ERROR(!Initialized,PRERR(ForceAbort,"NetCloseSession:  not initialized"));
		CheckPtrExistence(Session);
		/* first, kill the session to complete any reads/writes in progress. */
		/* this might fail if the session is already dead. */
		EndRec.sessRefNum = Session->SessionRefnum;
		Error = PPCEnd(&EndRec,False/*sync*/);
		/* now that that's finished, we can dump the buffers */
		if (Session->ReceiveInProgress != NIL)
			{
				ReleasePtr((char*)(Session->ReceiveInProgress));
			}
		if (Session->SendInProgress != NIL)
			{
				ReleasePtr((char*)(Session->SendInProgress));
			}
		BuffScan = Session->SendHead;
		while (BuffScan != NIL)
			{
				BufferRec*				BuffTemp;

				BuffTemp = BuffScan;
				BuffScan = BuffScan->Next;
				ReleasePtr((char*)BuffTemp);
			}
		BuffScan = Session->ReceiveHead;
		while (BuffScan != NIL)
			{
				BufferRec*				BuffTemp;

				BuffTemp = BuffScan;
				BuffScan = BuffScan->Next;
				ReleasePtr((char*)BuffTemp);
			}
		/* now dump the session record */
		ArrayDeleteElement(SessionArray,ArrayFindElement(SessionArray,Session));
		ReleasePtr((char*)Session);
	}


/* utility routine to submit a new read operation if one has completed */
static void					UpdateRead(SessionIDType* Session)
	{
		OSErr							Error;

		CheckPtrExistence(Session);
		ERROR(Session->ReadState == eReadInProgress,PRERR(ForceAbort,
			"UpdateRead:  read is already in progress"));
		/* handle the data that comes back from a read operation. */
		if (Session->ReadState == eReadFinished)
			{
				CheckPtrExistence(Session->ReceiveInProgress);
				/* check for session closed notification */
				if (Session->ThePPCReadPBRec.ioResult == sessClosedErr)
					{
						Session->OverallState = eSessionClosed;
					}
				ERROR((Session->ThePPCReadPBRec.ioResult != sessClosedErr)
					&& (Session->ThePPCReadPBRec.ioResult != noErr),PRERR(AllowResume,
					"UpdateRead:  error returned from PPCRead"));
				/* get read length */
				Session->ReceiveInProgress->NumBytes = Session->ThePPCReadPBRec.actualLength;
				ERROR((Session->ReceiveInProgress->NumBytes < 0)
					|| (Session->ReceiveInProgress->NumBytes > BUFFERSIZE),PRERR(ForceAbort,
					"UpdateRead:  too many bytes returned from PPCRead"));
				if (Session->ReceiveInProgress->NumBytes != 0)
					{
						/* move this buffer onto the list */
						Session->ReceiveInProgress->Next = NIL;
						if (Session->ReceiveTail != NIL)
							{
								CheckPtrExistence(Session->ReceiveTail);
								Session->ReceiveTail->Next = Session->ReceiveInProgress;
							}
						 else
							{
								Session->ReceiveHead = Session->ReceiveInProgress;
							}
						Session->ReceiveTail = Session->ReceiveInProgress;
					}
				 else
					{
						ReleasePtr((char*)(Session->ReceiveInProgress));
					}
				/* clear buffer register */
				Session->ReceiveInProgress = NIL;
				/* indicate that data has arrived */
				if (Session->OverallState == eSessionNormal)
					{
						/* don't change this if it contains eSessionClosed! */
						Session->OverallState = eSessionDataArrived;
					}
				/* reset read state */
				Session->ReadState = eReadIdle;
			}
		/* if the session has closed with the previous read, don't retrigger the read */
		/* but instead just exit */
		if (Session->OverallState == eSessionClosed)
			{
				return;
			}
		/* retrigger the read operation on a new block */
		Session->ReceiveInProgress = (BufferRec*)AllocPtrCanFail(
			sizeof(BufferRec),"ReadBufferRec");
		if (Session->ReceiveInProgress == NIL)
			{
				return;
			}
		Session->ThePPCReadPBRec.ioCompletion = (ProcPtr)&MyPPCReadCompletionRoutine;
		Session->ThePPCReadPBRec.sessRefNum = Session->SessionRefnum;
		Session->ThePPCReadPBRec.bufferLength = BUFFERSIZE;
		Session->ThePPCReadPBRec.bufferPtr = &(Session->ReceiveInProgress->Buffer[0]);
		Session->ReadState = eReadInProgress; /* BEFORE the call! */
		Error = PPCRead(&(Session->ThePPCReadPBRec),True/*async*/);
	}


/* utility routine to submit another write if one has completed */
static void					UpdateWrite(SessionIDType* Session)
	{
		OSErr							Error;

		CheckPtrExistence(Session);
		ERROR((Session->WriteState == eWriteInProgress),PRERR(ForceAbort,
			"UpdateWrite:  write is already in progress"));
		/* if a write has completed, then handle it */
		if (Session->WriteState == eWriteFinished)
			{
				/* check for session closed notification */
				if (Session->ThePPCWritePBRec.ioResult == sessClosedErr)
					{
						Session->OverallState = eSessionClosed;
					}
				ERROR((Session->ThePPCWritePBRec.ioResult != sessClosedErr)
					&& (Session->ThePPCWritePBRec.ioResult != noErr),PRERR(AllowResume,
					"UpdateWrite:  error returned from PPCWrite"));
				/* reset write state stuff */
				Session->WriteState = eWriteIdle;
				/* dispose of the write buffer block */
				ReleasePtr((char*)(Session->SendInProgress));
				Session->SendInProgress = NIL;
			}
		/* if there is more data queued to be sent, then send it */
		if (Session->SendHead != NIL)
			{
				CheckPtrExistence(Session->SendHead);
				/* move the first block to the in progress register */
				Session->SendInProgress = Session->SendHead;
				/* pop the block off the queue */
				Session->SendHead = Session->SendHead->Next;
				if (Session->SendHead == NIL)
					{
						Session->SendTail = NIL;
					}
				/* construct the parameter record */
				Session->ThePPCWritePBRec.ioCompletion = (ProcPtr)&MyPPCWriteCompletionRoutine;
				Session->ThePPCWritePBRec.sessRefNum = Session->SessionRefnum;
				Session->ThePPCWritePBRec.bufferLength = Session->SendInProgress->NumBytes;
				Session->ThePPCWritePBRec.bufferPtr = &(Session->SendInProgress->Buffer[0]);
				Session->ThePPCWritePBRec.more = False; /* always false */
				Session->ThePPCWritePBRec.userData = 0;
				Session->ThePPCWritePBRec.blockCreator = 0;
				Session->ThePPCWritePBRec.blockType = 0;
				Session->WriteState = eWriteInProgress; /* BEFORE the call! */
				Error = PPCWrite(&(Session->ThePPCWritePBRec),True/*async*/);
			}
	}


/* Verify that a session is still usable.  Returns True if the session is still */
/* available, or False if the remote system disconnected it.  If it returns False, */
/* then you should call NetCloseSession to dispose of the session record. */
MyBoolean						NetIsSessionStillAlive(SessionIDType* Session)
	{
		ERROR(!Initialized,PRERR(ForceAbort,"NetIsSessionStillAlive:  not initialized"));
		CheckPtrExistence(Session);
		return (Session->OverallState != eSessionClosed);
	}


/* Obtain a string identifying the machine from which a session has been */
/* established.  The string is a non-null-terminated heap block. */
char*								NetSessionGetRemoteMachineName(SessionIDType* Session)
	{
		char							LocalBuffer[32 + 32 + 32 + 1 + 1];
		long							Index;
		long							Scan;
		char*							Pointer;

		ERROR(!Initialized,PRERR(ForceAbort,
			"NetSessionGetRemoteMachineName:  not initialized"));
		CheckPtrExistence(Session);
		switch (Session->RemoteLocationName.locationKindSelector)
			{
				default:
					EXECUTE(PRERR(ForceAbort,
						"NetSessionGetRemoteMachineName:  unknown locationKindSelector"));
					break;
				case ppcNoLocation:
					Index = 9;
					CopyData("localhost",LocalBuffer,9);
					break;
				case ppcNBPLocation:
					Index = 0;
					for (Scan = 0; Scan < Session->RemoteLocationName
						.u.nbpEntity.objStr[0]; Scan += 1)
						{
							LocalBuffer[Index++] = Session->RemoteLocationName
								.u.nbpEntity.objStr[Scan + 1];
						}
					if (Session->RemoteLocationName.u.nbpEntity.typeStr[0] != 0)
						{
							LocalBuffer[Index++] = ':';
						}
					for (Scan = 0; Scan < Session->RemoteLocationName
						.u.nbpEntity.typeStr[0]; Scan += 1)
						{
							LocalBuffer[Index++] = Session->RemoteLocationName
								.u.nbpEntity.typeStr[Scan + 1];
						}
					if (Session->RemoteLocationName.u.nbpEntity.zoneStr[0] != 0)
						{
							LocalBuffer[Index++] = '@';
						}
					for (Scan = 0; Scan < Session->RemoteLocationName
						.u.nbpEntity.zoneStr[0]; Scan += 1)
						{
							LocalBuffer[Index++] = Session->RemoteLocationName
								.u.nbpEntity.zoneStr[Scan + 1];
						}
					break;
			}
		Pointer = AllocPtrCanFail(Index,"NBPNameString");
		if (Pointer != NIL)
			{
				CopyData(&(LocalBuffer[0]),&(Pointer[0]),Index);
			}
		return Pointer;
	}


/* Find out how much data is waiting to be read from the port. */
long								NetHowMuchDataToRead(SessionIDType* Session)
	{
		long							DataCount;
		BufferRec*				BufferScan;

		ERROR(!Initialized,PRERR(ForceAbort,"NetHowMuchDataToRead:  not initialized"));
		CheckPtrExistence(Session);
		BufferScan = Session->ReceiveHead;
		DataCount = 0;
		while (BufferScan != NIL)
			{
				DataCount += BufferScan->NumBytes;
				BufferScan = BufferScan->Next;
			}
		return DataCount;
	}


/* Find out how much data is waiting in local buffers to be written to a port. */
long								NetHowMuchDataToWrite(SessionIDType* Session)
	{
		long							DataCount;
		BufferRec*				BufferScan;

		ERROR(!Initialized,PRERR(ForceAbort,"NetHowMuchDataToWrite:  not initialized"));
		CheckPtrExistence(Session);
		/* initiate another read if there isn't one pending */
		if (Session->ReadState == eReadIdle)
			{
				UpdateRead(Session);
			}
		/* count data in buffers waiting to be eaten */
		BufferScan = Session->SendHead;
		DataCount = 0;
		while (BufferScan != NIL)
			{
				DataCount += BufferScan->NumBytes;
				BufferScan = BufferScan->Next;
			}
		return DataCount;
	}


/* Read data from a session.  It is an error to read more data than there is waiting. */
void								NetReadData(SessionIDType* Session, char* Buffer, long NumBytes)
	{
		ERROR(!Initialized,PRERR(ForceAbort,"NetReadData:  not initialized"));
		CheckPtrExistence(Session);
		ERROR((NumBytes < 0) || (NumBytes > NetHowMuchDataToRead(Session)),
			PRERR(ForceAbort,"NetReadData:  number of bytes is out of range"));
		/* extract data from blocks */
		while (NumBytes > 0)
			{
				long							TempNumBytes;
				BufferRec*				TempBuffer;

				TempBuffer = Session->ReceiveHead;
				CheckPtrExistence(TempBuffer);
				TempNumBytes = TempBuffer->NumBytes;
				CopyData(&(TempBuffer->Buffer[0]),&(Buffer[0]),TempNumBytes);
				Session->ReceiveHead = TempBuffer->Next;
				NumBytes -= TempNumBytes;
				Buffer += TempNumBytes;
				ReleasePtr((char*)TempBuffer);
			}
		if (Session->ReceiveHead == NIL)
			{
				Session->ReceiveTail = NIL;
			}
		/* initiate another read if there isn't one pending */
		if (Session->ReadState == eReadIdle)
			{
				UpdateRead(Session);
			}
	}


/* Write data to a session.  If data could not be sent without blocking, then */
/* it is locally buffered until it can be sent (that's what NetUpdate is for) */
/* returns True if successful, or False if there isn't enough memory.  if it fails, */
/* then NO data is written (i.e. data is never partially written) */
MyBoolean						NetWriteData(SessionIDType* Session, char* Buffer, long NumBytes)
	{
		BufferRec*				LocalHead;
		BufferRec*				LocalTail;
		BufferRec*				BuffScan;
		long							TempByteCount;

		ERROR(!Initialized,PRERR(ForceAbort,"NetWriteData:  not initialized"));
		CheckPtrExistence(Session);
		/* we create the buffers totally first, and only submit them if we could */
		/* allocate ALL of them */
		LocalHead = NIL;
		LocalTail = NIL;
		TempByteCount = NumBytes;
		while (TempByteCount > 0)
			{
				BufferRec*				TempBuff;

				if (TempByteCount > BUFFERSIZE)
					{
						TempByteCount -= BUFFERSIZE;
					}
				 else
					{
						TempByteCount -= TempByteCount;
					}
				TempBuff = (BufferRec*)AllocPtrCanFail(sizeof(BufferRec),"BufferRec");
				if (TempBuff == NIL)
					{
						while (LocalHead != NIL)
							{
								/* dump ones that we already allocated */
								CheckPtrExistence(LocalHead);
								TempBuff = LocalHead;
								LocalHead = LocalHead->Next;
								ReleasePtr((char*)TempBuff);
							}
						return False; /* oops */
					}
				TempBuff->Next = NIL;
				if (LocalTail != NIL)
					{
						LocalTail->Next = TempBuff;
					}
				 else
					{
						LocalHead = TempBuff;
					}
				LocalTail = TempBuff;
			}
		/* buffers allocated, now fill them */
		TempByteCount = NumBytes;
		BuffScan = LocalHead;
		while (TempByteCount > 0)
			{
				long							NumBytesThisTime;

				if (TempByteCount > BUFFERSIZE)
					{
						NumBytesThisTime = BUFFERSIZE;
					}
				 else
					{
						NumBytesThisTime = TempByteCount;
					}
				CheckPtrExistence(BuffScan);
				BuffScan->NumBytes = NumBytesThisTime;
				CopyData(&(Buffer[0]),&(BuffScan->Buffer[0]),NumBytesThisTime);
				Buffer += NumBytesThisTime;
				TempByteCount -= NumBytesThisTime;
				BuffScan = BuffScan->Next;
			}
		ERROR(BuffScan != NIL,PRERR(ForceAbort,
			"NetWriteData:  BuffScan not NIL after buffer fill loop"));
		/* now, tack it on the end of the write buffer list */
		if (Session->SendTail != NIL)
			{
				Session->SendTail->Next = LocalHead;
			}
		 else
			{
				Session->SendHead = LocalHead;
			}
		Session->SendTail = LocalTail;
		/* if there is no outstanding write, then make a write */
		if (Session->WriteState == eWriteIdle)
			{
				UpdateWrite(Session);
			}
		/* successful */
		return True;
	}


#ifdef THINK_C
	#if __option(profile)
		#define Profiling (True)
	#else
		#define Profiling (False)
	#endif

	#pragma options(!profile)
#endif

static pascal void	MyPPCInformCompletionRoutine(PPCInformPBRec* PB)
	{
		PortIDType*				Port;

		/* some goofy pointer arithmetic to get from a member of the struct */
		/* back to the beginning of the struct. */
		Port = (PortIDType*)((char*)PB - (long)&(((PortIDType*)NIL)->ThePPCInformPBRec));
		/* now we can access the other members */
		Port->PortState = eConnectionPending; /* indicate that PPCInform is done. */
	}

static pascal void	MyPPCReadCompletionRoutine(PPCParamBlockRec* PB)
	{
		SessionIDType*		Session;

		/* some goofy pointer arithmetic to get from a member of the struct */
		/* back to the beginning of the struct. */
		Session = (SessionIDType*)((char*)PB
			- (long)&(((SessionIDType*)NIL)->ThePPCReadPBRec));
		/* now we can access the other members */
		Session->ReadState = eReadFinished;
	}

static pascal void	MyPPCWriteCompletionRoutine(PPCParamBlockRec* PB)
	{
		SessionIDType*		Session;

		/* some goofy pointer arithmetic to get from a member of the struct */
		/* back to the beginning of the struct. */
		Session = (SessionIDType*)((char*)PB
			- (long)&(((SessionIDType*)NIL)->ThePPCWritePBRec));
		/* now we can access the other members */
		Session->WriteState = eWriteFinished;
	}

#ifdef THINK_C
	#if Profiling
		#pragma options(profile)
	#endif
#endif
