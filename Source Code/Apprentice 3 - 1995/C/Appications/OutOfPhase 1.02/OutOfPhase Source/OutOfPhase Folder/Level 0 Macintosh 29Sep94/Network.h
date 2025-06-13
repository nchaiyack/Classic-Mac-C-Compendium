/* Network.h */

#ifndef Included_Network_h
#define Included_Network_h

/* Network module depends on */
/* MiscInfo.h */
/* Audit */
/* Debug */
/* Definitions */
/* Memory */
/* EventLoop */
/* Array */

/* this is the AppleTalk PPCToolBox version of the Network module */

/* ID number type of a network port being listened at */
struct PortIDType;
typedef struct PortIDType PortIDType;

/* ID number of a communications session */
struct SessionIDType;
typedef struct SessionIDType SessionIDType;

/* kind of network that you can ask for */
typedef enum
	{
		eNetDefault EXECUTE(= -21234),
		eNetAppleTalk,
		eNetTCP
	} NetworkTypes;

/* errors that can occur when dealing with ports */
typedef enum
	{
		eNetNoError EXECUTE(= -9343),
		eNetNoMemory, /* not enough memory to allocate a new port record */
		eNetPortInUse, /* the port is already being used by another thing on the system */
		eNetBadPortString, /* the port string is not valid for the network in use */
		eNetBadMachineString, /* the machine name is not valid for this network */
		eNetMachineUnknown, /* the machine could not be reached; connection timed out */
		eNetNetworkNotAvailable, /* the network services are not available on machine */
		eNetCouldntInitNet, /* the network could not be initialized */
		eNetConnectRefused, /* connections was refused */
		eNetProtocolNotSupported, /* they asked for an unsupported communication protocol */
		eNetUnknownError /* who knows what went wrong */
	} NetErrors;

/* events that can be returned from the network update function.  these are */
/* NOT the same as those from EventLoop and should not be used. */
typedef enum
	{
		eNetEventNone EXECUTE(= -412), /* nothing is happening right now */
		eNetEventNewSession, /* new incoming session *SessionNumber has been established */
		eNetEventSessionClosed, /* the session *SessionNumber has been closed by remote */
		eNetEventDataIncoming, /* new data has arrived on session *SessionNumber */
		eNetEventInternal /* there is more work to be done, so call back soon */
	} NetEvents;

/* initialize the network interface.  the user calls this.  it is not called from */
/* the standard Level 0 initialization routine because not all programs need */
/* networking.  Could return eNetNoError, eNetNoMemory, eNetNetworkNotAvailable, */
/* eNetCouldntInitNet, or eNetUnknownError */
NetErrors						InitializeNetwork(void);

/* discard any pending data, close any open connections, and clean internal data */
void								ShutdownNetwork(void);

/* Network update routine.  It handles periodic update tasks and should be called */
/* frequently (preferably in the main event loop next to GetAnEvent). */
NetEvents						NetUpdate(SessionIDType** SessionNumber);

/* Listen at the specified port.  Returns the reference number of the port being */
/* listened at.  PortString is a non-null-terminated heap block containing the string */
/* identifying the port.  The port string format depends on the underlying network */
/* scheme.  For instance, TCP/IP would be an integer between 128 and 9999. */
/* for AppleTalk (NBP), it is a valid NBP name.  Could return eNetNoError, */
/* eNetNoMemory, eNetPortInUse, eNetBadPortString, eNetUnknownError, or */
/* eNetProtocolNotSupported. */
NetErrors						NetListenAtPort(char* PortString, PortIDType** PortOut,
											NetworkTypes WhichNetwork);

/* Stop listening at a port and let the OS use it for someone else.  Sessions */
/* established through this port are terminated. */
void								NetTerminatePortAndSessions(PortIDType* Port);

/* Open a session to another (or the same) machine.  Returns a session number in */
/* *SessionOut.  PortString is the remote port to connect to, as described above, */
/* and MachineString is a machine string determined by the network stack.  For */
/* instance, TCP/IP would support standard a.b.c.d or machine.zone.domain format. */
/* The Macintosh uses machine:type@zone.  Could return eNetNoError, eNetNoMemory, */
/* eNetBadPortString, eNetBadMachineString, eNetMachineUnknown, eNetUnknownError, */
/* eNetConnectRefused, or eNetProtocolNotSupported. */
NetErrors						NetOpenSession(char* PortString, char* MachineString,
											SessionIDType** SessionOut, NetworkTypes WhichNetwork);

/* Close a session.  Any waiting data in either direction is discarded. */
void								NetCloseSession(SessionIDType* Session);

/* Verify that a session is still usable.  Returns True if the session is still */
/* available, or False if the remote system disconnected it.  If it returns False, */
/* then you should call NetCloseSession to dispose of the session record. */
MyBoolean						NetIsSessionStillAlive(SessionIDType* Session);

/* Obtain a string identifying the machine from which a session has been */
/* established.  The string is a non-null-terminated heap block. */
char*								NetSessionGetRemoteMachineName(SessionIDType* Session);

/* Find out how much data is waiting to be read from the port. */
long								NetHowMuchDataToRead(SessionIDType* Session);

/* Find out how much data is waiting in local buffers to be written to a port. */
long								NetHowMuchDataToWrite(SessionIDType* Session);

/* Read data from a session.  It is an error to read more data than there is waiting. */
void								NetReadData(SessionIDType* Session, char* Buffer, long NumBytes);

/* Write data to a session.  If data could not be sent without blocking, then */
/* it is locally buffered until it can be sent (that's what NetUpdate is for) */
/* returns True if successful, or False if there isn't enough memory.  if it fails, */
/* then NO data is written (i.e. data is never partially written) */
MyBoolean						NetWriteData(SessionIDType* Session, char* Buffer, long NumBytes);

#endif
