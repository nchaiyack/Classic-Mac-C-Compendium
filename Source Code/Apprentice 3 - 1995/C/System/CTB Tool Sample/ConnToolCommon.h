// ===========================================================================
// "Connection Tool Skeleton in C" for the Communications Toolbox
// 
//	Copyright � 1994 Peter J. Creath
//	All Rights Reserved Worldwide
// ===========================================================================

#include <CommResources.h>
#include <ConnectionTools.h>
#include <Serial.h>								// This is implementation-dependent

#define	myFirstItem		1
#define	mySecondItem	2
#define localDITLID		1

// ===========================================================================
// PrivateRecord {struct}
// 	This structure should be designed to handle any run-time data needed throughout the connection
// 	tool.
// ===========================================================================
typedef struct
{
	long 		foobar;
	long 		status;
	Boolean	breakPending;
	short	inRefNum;
	short	outRefNum;
	struct
		{
			IOParam	theParamBlk;
		} myRBlk;
	struct
		{
			IOParam	theParamBlk;
		} myWBlk;
} PrivateRecord, *PrivatePtr, **PrivateHandle;

// ===========================================================================
// ConfigRecord {struct}
// 	This structure should be designed to handle whatever saved configuration data may be needed.
// ===========================================================================
typedef struct
{
	long		foobar;
	Str255	portName;
	SerShk	shaker;
	short	dataBits;
	short	baudRate;
} ConfigRecord, *ConfigPtr, **ConfigHandle;
