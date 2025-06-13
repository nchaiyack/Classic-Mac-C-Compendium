// tcp easy.h
// Darrell Anderson

/*

A simple interface to MacTCP.

Note that these routines are semi-synchronous.  The driver calls are made asynchronously,
but the routines spin until the call completes, yielding time to other processes in the 
meantime.

General usage:

The basic unit is a stream, you need to allocate and dispose of them when finished with it.
failing to deallocate it may cause MacTCP to lock up the machine next time you try to create
a stream.

Once you have a stream, you may have at most one open connection on it.  You may close that
connection and open another on the same stream without problems.

Once you have a connection on a stream, you may send and receive data on it.

Receiving is a blocking i/o call, but you may pass a timeout value to tell it when to give
up.  Also note that if the remote end closes the connection, the receive returns immediately 
(though in error).  Note that a timeout value of 0 means to wait indefinitely.

Note: I'm not quite sure why yet, but passing 0 to wait indefinitely doesn't seem to work.
For the moment, passing 32767 and waiting 9 hours is acceptable.
*/

#ifndef __tcp_easy__
#define __tcp_easy__

#include "easy tcp structs.h"
#include "MacTCPCommonTypes.h"

// how to establish connection
#define ACTIVE 0
#define PASSIVE 1

// timout is in seconds, buffersize in bytes
#define DEFAULT_TIMEOUT 20
#define DEFAULT_BUFFERSIZE 16384

// open the MacTCP driver.  MUST be done before other routines will work.
OSErr EasyTCPInit(void);

// create a stream (necessary before opening a connection)
// arguments:  ( <- means returned, -> means passed, <> means both )
// <- stream      returns a pointer to the newly allocated stream
// -> timeout     timeout in seconds to wait, or -1 to use the default (defined above)
// -> buffersize  buffersize for this stream, or -1 to use the default (defined above)
OSErr EasyTCPCreate(StreamPtr *stream, int timeout, int buffersize);

// release a stream (must close any connection on it first!)
// -> stream      the stream to dispose of
// -> timeout     timeout in seconds to wait, or -1 to use the default (defined above)
OSErr EasyTCPRelease(StreamPtr stream, int timeout);

// open a connection.
// arguments:  ( <- means returned, -> means passed, <> means both )
// <- stream      use this for future reference to this connection (send,receive,close)
// -> hostNport   if active: which machine to connect to, and which port
//                if passive: host is ignored, but port is which port to listen at
// -> method      either ACTIVE or PASSIVE
// -> timeout     timeout in seconds to wait, or -1 to use the default (defined above)
//                a value of 0 means to wait indefinitely for the connection
OSErr EasyTCPOpen(StreamPtr stream, HostNPort *hostNport, int method, int timeout);

// close a connection
// arguments:  ( <- means returned, -> means passed, <> means both )
// -> stream      is the stream to close
// -> timeout     is the amount of time to wait before beating the connection to a pulp
//                should the remote machine not cooperate
OSErr EasyTCPClose(StreamPtr stream, int timeout);

// send data out through a tcp connection
// arguments:  ( <- means returned, -> means passed, <> means both )
// -> stream     is the connection to send the data on
// -> data       is a pointer to the data to be sent
// -> dataLength is the number of bytes to send
// -> timeout    is the amount of time to wait before giving up, pass -1 to use the default
OSErr EasyTCPSend(StreamPtr stream, Ptr data, int dataLength, int timeout);

// read data from a connection, possibly time out if it all can't be read in time
// arguments:  ( <- means returned, -> means passed, <> means both )
// -> stream     is the connection to read the data from
// -> data       is a pointer to where in memory the incoming data should be stuffed
//               note that this memory must be preallocated, EasyTCPReceive does not 
//               do that for you.
// -> bytesToRead is the number of bytes we would like to read
// <- bytesRead  is the actual number of bytes received before time ran out
// -> timeout    is the number of seconds to wait before returning with what we've read
//               pass -1 to use the default.
//               a value of 0 means to wait indefinitely for the connection
OSErr EasyTCPReceive(StreamPtr stream, Ptr data, int bytesToRead, int *bytesRead, int timeout);

#endif