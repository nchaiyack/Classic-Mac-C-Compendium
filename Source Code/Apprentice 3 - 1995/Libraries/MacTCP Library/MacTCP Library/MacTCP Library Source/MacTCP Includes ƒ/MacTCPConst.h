/*
	MacTCPConst.h
	
	Constants used in the MacTCPLib.
	
	See the file "NamingConventions.note" for a few notes concerning my naming conventions.
	
	01/17/94: 1.0 alpha - dn
	01/28/94: 1.0 complete - dn
*/

#pragma once

#ifndef __H_TCPConst__
#define __H_TCPConst__

#define kb					1024L	// kilobyte
#define kMinRecSize			4*kb		// minimum size of the receiving buffer
#define kMaxRecSize		16*kb	// maximum size of the receiving buffer

// a few macros...
#define mMin(a,b) (a<b?a:b)
#define mMax(a,b) (a>b?a:b)
#define mCurA5() (*(long*)CurrentA5)

// some memory macros
#define mNewHandle(x,y) (x)NewHandle(y)
#define mNewHandleClear(x,y) (x)NewHandleClear(y)
#define mNewPtr(x,y) (x)NewPtr(y)
#define mNewPtrClear(x,y) (x)NewPtrClear(y)

// the following are provided to overrule the strong typechecking (sometimes it's a pain...)
#define mMoveHHi(x) MoveHHi((Handle)(x))
#define mHLock(x) HLock((Handle)(x))
#define mHUnlock(x) HUnlock((Handle)(x))
#define mDisposeHandle(x) DisposeHandle((Handle)(x))
#define mDisposePtr(x) DisposePtr((Ptr)(x))

typedef Boolean boolean;

/*
	OSErr error codes.  Since MacTCP uses errors starting at -23000, the library will use error
	values starting at +23000.
	
	The errors are separated into their respective header files:
		mactcp.h
		ip.h
		dnr.h
		tcp.h
		udp.h
*/
enum {
	mactcpBaseErr=23000				// the base error number for the library.
	
};

#endif /* __H_TCPConst__ */