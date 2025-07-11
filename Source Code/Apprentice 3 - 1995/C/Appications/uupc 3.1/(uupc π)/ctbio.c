/*
 *	This is a replacement module for sio.c in Sak Wathanasin's 1989 distribution
 *	of uupc for the Mac. It uses a CTB connection tool for "serial i/o".
 *
 *	In order to preserve the interface, some function declarations kept
 *	serial-specific parameters which are now unused.
 *
 *  Originally written by Roger Pantos.  Modified by Dave Platt to fit into
 *  uupc 3.1.
 */

#include "dcp.h"

#include <Connections.h>
#include <CommResources.h>
#include <Serial.h>
#include <GestaltEqu.h>

#include "ctbio.proto.h"

const long	kBufferSize = 2048;

ConnectionHandler ctbHandler = {
	CTBIOInit,
	CTBIOInBuffer,
	CTBIOOutBuffer,
	CTBIOSpeed,
	CTBIOHandshake,
	CTBIOSetting,
	CTBIOClose,
	CTBIOSetFlowCtl,
	CTBIOAllowInterrupts,
	CTBIOInterrupt,
	CTBIOPurge,
	CTBIOPutchar,
	CTBIOWrite,
	CTBIOAvail,
	CTBIOWStr,
	CTBIORead,
	CTBIOIdle,
	CTBIOEvent,
	CTBIOBreak,
	CTBIOSetParity
};

static int allowInterrupts = FALSE;
static int interruptOccurred = FALSE;

CMBufferSizes	buffSizes = {
	0L, 0L,
	0L, 0L,
	0L, 0L,
	0L, 0L,
};	

ConnHandle	gSerConHdl;
extern SysEnvRec SysEnv;
extern int settingsFileVRefNum;
extern long int settingsFileDirID;
extern int useHardwareFlowControl;

volatile int serialIOBusy, serialIOLock, serialIOCompleted;

static char *inbuf, *outbuf;
static long inbufSize, outbufSize;
static long writeCount, writeBase, writeOrigin;
static long foregroundWrites, backgroundWrites;
static long bytesActuallyWritten;
static CMErr serialIOError;
static int closeCompleted;
static int connectionShutDown;
static int asynchronous;

static enum {
  fully_synchronous,
  asynchronous_foreground,
  asynchronous_fore_background
} synchrony;

#define NOWAIT
#define NOWAITWRITE
#define QUIET

static void CTBIOEndAction(ConnHandle gSerConHdl)
{
	writeCount -= bytesActuallyWritten;
	writeOrigin = (writeOrigin + bytesActuallyWritten) % outbufSize;
}

pascal static void CTBIOOpenCompletion(ConnHandle gSerConHdl)
{
	long savedA5;
	savedA5 = SetCurrentA5();
	SetA5((**gSerConHdl).userData);
	serialIOError = (**gSerConHdl).errCode;
	SetA5(savedA5);
	return;
}

pascal static void CTBIOCloseCompletion(ConnHandle gSerConHdl)
{
	long savedA5;
	savedA5 = SetCurrentA5();
	SetA5((**gSerConHdl).userData);
	closeCompleted = 1;
	SetA5(savedA5);
	return;
}



pascal static void CTBIOCompletion(ConnHandle gSerConHdl)
{
	long savedA5;
	long length, toEndOfBuffer;
	CMErr cmErr;
	savedA5 = SetCurrentA5();
	SetA5((**gSerConHdl).userData);
	if (outbuf) {
		bytesActuallyWritten = (**gSerConHdl).asyncCount[cmDataOut];
		if (serialIOLock || synchrony != asynchronous_fore_background) {
			serialIOCompleted = 1;
		} else {
			CTBIOEndAction(gSerConHdl);
			if (writeCount > 0) {
				length = writeCount;
				toEndOfBuffer = outbufSize - writeOrigin;
				if (length > toEndOfBuffer) length = toEndOfBuffer;
				backgroundWrites += length;
				cmErr = CMWrite( gSerConHdl, outbuf + writeOrigin, &length, cmData, TRUE, (ProcPtr) CTBIOCompletion, -1L, 0);
				if (cmErr != noErr) {
					serialIOBusy = 0;
					serialIOError = cmErr;
				}
			} else {
				serialIOBusy = 0;
			}
		}
	} else {
		serialIOBusy = 0;
	}
	SetA5(savedA5);
	return;
}

int CTBIOPush()
{
	long int length, toEndOfBuffer;
	CMErr cmErr;
	if (!gSerConHdl) {
		return 0;
	}
	CMIdle(gSerConHdl);
	if (serialIOCompleted) {
		serialIOCompleted = 0;
		serialIOBusy = 0;
		printmsg(12, "Do endaction, %ld bytes written", bytesActuallyWritten);
		CTBIOEndAction(gSerConHdl);
	}
	if (synchrony != fully_synchronous && (!serialIOBusy && writeCount > 0)) {
		length = writeCount;
		toEndOfBuffer = outbufSize - writeOrigin;
		if (length > toEndOfBuffer) length = toEndOfBuffer;
		serialIOBusy = 1;
		printmsg(12, "Start write, %ld bytes", length);
		foregroundWrites += length;
		cmErr = CMWrite( gSerConHdl, outbuf + writeOrigin, &length, cmData, TRUE, (ProcPtr) CTBIOCompletion, -1L, 0);
		if (cmErr != noErr) {
			serialIOBusy = 0;
			connectionShutDown = TRUE;
			writeCount = 0;
			if (cmErr == cmNotOpen) {
				printmsg(0, "Connection closed");
			} else {
				printmsg(0, "CMWrite error %d", cmErr);
			}
			return cmErr;
		}
	}
	return 0;
}

static void CTBIOWaitOne()
{
	CMIdle(gSerConHdl);
	CTBIOPush();
  if (Check_Events(0)) {
    if (Main_State == Abort_Program)  {
			CMClose( gSerConHdl, FALSE, NULL, -1L, FALSE);
			CMDispose( gSerConHdl);
    	exit(-1);
    }
  }
}

void CTBIOWait()
{
	do {
		printmsg(12, "busy %d, completed %d, count %d, lock %d", serialIOBusy, serialIOCompleted,
		 writeCount, serialIOLock);
		CTBIOWaitOne();
	} while (serialIOBusy);
}

int CTBInit()
{
	static int initted = 0;
	OSErr errCode;
	if (!initted) {
		if (	(errCode = InitCTBUtilities()) || (errCode = InitCRM()) || 
				(errCode = InitCM()))
		{
			printmsg( 0, "CTBInit: Can't initialize Connection Manager, error %d", errCode );
			return 1;
		}
		initted = 1;
	}
	return 0;
}

void CTBAddToMenu(MenuHandle theMenu)
{
	int i;
	if ((i = HOpenResFile(settingsFileVRefNum, settingsFileDirID, "\pUUPC Settings", fsRdPerm)) == -1) {
		printmsg(0, "Can't open \"UUPC Settings\" to load CTB configs" );
		return;
	}
  AddResMenu(theMenu,'C�nn');
	CloseResFile(i);
}


int CTBIsNamedConfig(char *configName)
{
	Handle dataHandle;
	int i;
	if ((i = HOpenResFile(settingsFileVRefNum, settingsFileDirID, "\pUUPC Settings", fsRdPerm)) == -1) {
		printmsg(0, "Can't open \"UUPC Settings\" to read configuration" );
		return FALSE;
	}
	dataHandle =  GetNamedResource('C�nn', configName);
	if (dataHandle) {
		CloseResFile(i);
		return TRUE;
	} else {
		CloseResFile(i);
		return FALSE;
	}
}

ConnHandle CTBLoadConfig(char *configName)
{
	Handle dataHandle;
	ConnHandle connHandle;
	short cProcID;
	char *configString;
	int configChars;
	char bogusName[256];
	int i;
	Str255 defaultTool;
	OSErr err;
	if (CTBInit()) {
		return NULL;
	}
	if ((i = HOpenResFile(settingsFileVRefNum, settingsFileDirID, "\pUUPC Settings", fsRdPerm)) == -1) {
		printmsg(0, "Can't open \"UUPC Settings\" to read configuration" );
		return NULL;
	}
	dataHandle =  GetNamedResource('C�nn', configName);
	if (dataHandle) {
		DetachResource(dataHandle);
	}
	CloseResFile(i);
	if (!dataHandle) {
		if (strncmp(configName, "\pDefault", 8) != 0) {
			strncpy(bogusName, configName+1, 64);
			bogusName[configName[0]] = '\0';
			printmsg(0, "Can't load CTB configuration %s", bogusName);
			return NULL;
		}
		err = CRMGetIndToolName('cbnd', 1, defaultTool);
		if (err != noErr) {
			printmsg(0, "Can't find any Connection Tools!\n");
			return NULL;
		}
		dataHandle = NewHandle(defaultTool[0] + 3);
		if (!dataHandle) {
			return NULL;
		}
		HLock(dataHandle);
		memcpy(*dataHandle, (char *) defaultTool, defaultTool[0] + 1);
		(*dataHandle)[defaultTool[0] + 1] = 0;
		(*dataHandle)[defaultTool[0] + 2] = 0;
		HUnlock(dataHandle);
	}
	MoveHHi(dataHandle);
	HLock(dataHandle);
	if ( (cProcID = CMGetProcID( (StringPtr) *dataHandle)) == -1) {
		PtoCstr((StringPtr) *dataHandle);
		printmsg(0, "CTBLoadConfig:  can't get procid for %s", *dataHandle);
		ReleaseResource(dataHandle);
		return NULL;
	}
#ifdef QUIET
	if ( !(connHandle = CMNew( cProcID, (debuglevel == 0) ? cmQuiet : 0L, buffSizes, 0L, 0L)))
#else
	if ( !(connHandle = CMNew( cProcID, 0, buffSizes, 0L, 0L)))
#endif
	{
		PtoCstr((StringPtr) *dataHandle);
		printmsg(0, "CTBLoadConfig: Can't open a new %s connection", *dataHandle);
		ReleaseResource(dataHandle);
		return NULL;
	}
	configString = (char *) (*dataHandle + **dataHandle + 1);
	if (*configString != '\0') {
		configChars = CMSetConfig(connHandle, configString);
		if (configChars != cmNoErr) {
			if (configChars == -1) {
				printmsg(0, "CTBLoadConfig: unknown CTB error %d", configChars);
			} else if (configChars < 0) {
				printmsg(0, "CTBLoadConfig: O/S error %d", configChars);
			} else {
				printmsg(0, "CTBLoadConfig: config parse error at %s", configChars + configString);
			}
			ReleaseResource(dataHandle);
			return NULL;
		}
	}
	ReleaseResource(dataHandle);
	return connHandle;
}


void CTBPurgeConfig(char *configName)
{
	Handle dataHandle;
	int i;
	if ((i = HOpenResFile(settingsFileVRefNum, settingsFileDirID, "\pUUPC Settings", fsRdWrPerm)) == -1) {
		printmsg(0, "Can't open \"UUPC Settings\" to remove configuration" );
		return;
	}
	dataHandle =  GetNamedResource('C�nn', configName);
	if (dataHandle) {
		RmveResource(dataHandle);
	}
	CloseResFile(i);
}

void CTBSaveConfig(char *configName, ConnHandle connHandle)
{
	Str255 toolName;
	Ptr configString;
	Handle configHandle;
	int oldResFile, newID;
	OSErr osErr;
	int i;
	CMGetToolName((**connHandle).procID, toolName);
	PtrToHand(toolName, &configHandle, toolName[0]+1);
	configString = CMGetConfig(connHandle);
	if (!configString) {
		printmsg(0, "Oops!  Couldn't save configuration!");
		CMDispose( connHandle );
		DisposHandle(configHandle);
		return;
	}
	PtrAndHand(configString, configHandle, strlen(configString)+1);
	DisposPtr(configString);
	if ((i = HOpenResFile(settingsFileVRefNum, settingsFileDirID, "\pUUPC Settings", fsRdWrPerm)) == -1) {
		printmsg(0, "Can't open \"UUPC Settings\" to save configuration" );
		return;
	}
	newID = UniqueID('C�nn');
	osErr = ResError();
	AddResource((Handle) configHandle, 'C�nn', newID, configName);
	osErr = ResError();
	CloseResFile(i);
}

CTBIOInit ( char * whichport, char * speed, char * phone )
{
Point	where;
short	errCode=FALSE, cprocID;
CMBufferSizes	sizes;
CMStatFlags		flags;
int oldResFile, newID;
int incoming;
OSErr osErr;
Ptr configString;
Handle configHandle;
Str255 connString;
char command[256], *cmdStart;
long int minutes_to_sleep;
long currentA5;

	if ((connectionManagerGestalt & (1L << gestaltConnMgrPresent)) == 0) {
		printmsg( 0, "CTBIOInit: Connection Manager not available" );
		return -1;
	}
		
	if (CTBInit()) {
		return -1;
	}
	
	strcpy((char *) connString, whichport);
	
	CtoPstr((char *) connString);
	
	gSerConHdl = CTBLoadConfig( (char *) connString );
	
	if (!gSerConHdl) {
		printmsg(0, "Cannot load CTB configuration '%s'", whichport);
		return -1;
	}

	printmsg(2, "CTB configuration %s loaded", whichport);
	
	synchrony = asynchronous_fore_background;
	
	if (strlen(speed) > 0 && strcmp(speed, "\"\"") != 0 && strncmp(speed, "-", 1) != 0) {
		printmsg(2, "CTB options: %s", speed);
		if (strchr(speed, ' ') == NULL && isdigit(*speed)) {
			strcpy(command, "Baud ");
			strncat(command, speed, 128);
			cmdStart = command;
			errCode = CMSetConfig(gSerConHdl, command);
		} else {
			cmdStart = speed;
			errCode = CMSetConfig(gSerConHdl, speed);
		}
		if (errCode != cmNoErr) {
			if (errCode < 0) {
				printmsg(0, "CTB o/s error %d", errCode);
			} else if (errCode == -1) {
				printmsg(0, "CTB unknown error");
			} else {
				printmsg(0, "CTB parse error at: %s", cmdStart + errCode);
			}
			goto bailout;
		}
	}
	
	if (strcmp(speed, "-sync") == 0) {
		synchrony = fully_synchronous;
	} else if (strcmp(speed, "-async") == 0) {
		synchrony = asynchronous_foreground;
	} else if (strncmp(speed, "-background", 5) == 0) {
		synchrony = asynchronous_fore_background;
	}
	
	if (synchrony == fully_synchronous) {
		asynchronous = FALSE;
	} else {
		asynchronous = TRUE;
	}

	if (phone && strlen(phone) > 0 && strcmp(phone, "\"\"") != 0 && strcmp(phone, "-") != 0 &&
	 strcmp(phone, "@") != 0) {
		printmsg(2, "CTB phone: %s", phone);
		strcpy(command, "PhoneNumber \"");
		strncat(command, phone, 128);
		strcat(command, "\"");
		errCode = CMSetConfig(gSerConHdl, command);
		if (errCode != cmNoErr) {
			if (errCode < 0) {
				printmsg(0, "CTB o/s error %d", errCode);
			} else if (errCode == -1) {
				printmsg(0, "CTB unknown error");
			} else {
				printmsg(0, "CTB parse error at: %s", command + errCode);
			}
			goto bailout;
		}
	}

	currentA5 = SetCurrentA5();
	CMSetUserData(gSerConHdl, currentA5);
	
	serialIOBusy = 0;
	serialIOLock = 0;
	serialIOCompleted = 0;
	serialIOError = cmNoErr;
	inbuf = outbuf = NULL;
	inbufSize = outbufSize = 0;
	writeCount = writeBase = writeOrigin = 0;
	foregroundWrites = backgroundWrites = 0;
	currentConnection = &ctbHandler;
	
	if (strcmp(phone, "@") == 0) {
		incoming = TRUE;
		sscanf(sleeptime, "%ld", &minutes_to_sleep);
		if (minutes_to_sleep < 1) minutes_to_sleep = 1;
		printmsg(2, "Listening for inbound call, %ld-minute timeout", minutes_to_sleep);
		errCode = CMListen( gSerConHdl, asynchronous, (ProcPtr) CTBIOOpenCompletion, minutes_to_sleep * 3600L);
	} else {
		incoming = FALSE;
		errCode = CMOpen( gSerConHdl, asynchronous, (ProcPtr) CTBIOOpenCompletion, -1L);
	}
	
	if (errCode) {
		printmsg(0, "Connection Manager error %d", errCode);
		currentConnection = (ConnectionHandler *) NULL;
		CMDispose( gSerConHdl);
		return -1;
	}
	
	allowInterrupts = TRUE;
	interruptOccurred = FALSE;
	connectionShutDown = FALSE;
	
	while (1) {
		CMIdle(gSerConHdl);
		errCode = CMStatus(gSerConHdl, sizes, &flags);
		if (errCode != noErr) goto bailout;
		if (flags & cmStatusOpen) break;
		if (flags & cmStatusIncomingCallPresent) {
			printmsg(0, "Incoming call!");
			errCode = CMAccept(gSerConHdl, TRUE);
			if (errCode != 0) {
				printmsg(0, "Incoming call could not be accepted, error %d", errCode);
				goto bailout;
			}
			break;
		}
		if (!(flags & (cmStatusOpening | cmStatusListenPend))) {
			if (serialIOError == cmUserCancel) {
				Set_Main_State(Cancel_Call);
			} else {
				printmsg(0, "Connection failed to open, error %d", serialIOError);
			}
			CMAbort(gSerConHdl);
			CMClose(gSerConHdl, FALSE, NULL, -1L, TRUE);
			goto bailout;
		}
	  if (Check_Events(6)) {
	    if (Main_State == Abort_Program)  {
	  		CMAbort(gSerConHdl);
	  		CMClose(gSerConHdl, FALSE, NULL, -1L, FALSE);
				CMDispose( gSerConHdl);
	    	exit(-1);
	    }
	  }
		if (allowInterrupts && interruptOccurred) {
		  CMAbort(gSerConHdl);
			goto bailout;
		}
	}
	allowInterrupts = FALSE;
	return 0;
bailout:
	currentConnection = (ConnectionHandler *) NULL;
	CMDispose( gSerConHdl);
	gSerConHdl = NULL;
	return -1;

}


CTBIOInBuffer ( char * buf, int size)
{
	inbuf = buf;
	inbufSize = size;
}

CTBIOOutBuffer ( char * buf, int size)
{
	outbuf = buf;
	outbufSize = size;
}


CTBIOSpeed( char *speed )
{
	char realSpeed[256];
	short errCode;
	strcpy(realSpeed, "Baud ");
	strncat(realSpeed, speed, 20);
	errCode = CMSetConfig(gSerConHdl, realSpeed);
	if (errCode != cmNoErr) {
		if (errCode < 0) {
			printmsg(0, "CTBIOSpeed: O/S error %d", errCode);
		} else if (errCode == -1) {
			printmsg(0, "CTBIOSpeed: Unknown CTB error");
		} else {
			printmsg(0, "CTBIOSpeed: CTB parse error at: %s", speed + errCode);
		}
	}
}

CTBIOHandshake(int fInx, int fXOn, int fCTS, int xOn, int xOff)
{
}

CTBIOSetting(char *speed, int parity, int stopbits, int databits)
{
}

int CTBIOAvail(void)
{
CMBufferSizes	sizes;
CMStatFlags		flags;

		if (connectionShutDown) {
			return 0;
		}
		CTBIOPush();
		CMStatus( gSerConHdl, sizes, &flags);
		return ( sizes[ cmDataIn]) ;
}

CTBIOWStr(char *st)
{
	return CTBIOWrite ( st, strlen(st)) ;
}

int CTBIOSetFlowCtl(int software, int hardware)
{
	if (hardware) {
		(void) CMSetConfig(gSerConHdl, "Handshake DTR&CTS");
	} else if (software) {
		(void) CMSetConfig(gSerConHdl, "Handshake XON/XOFF");
	} else {
		(void) CMSetConfig(gSerConHdl, "Handshake None");
	}
}

int CTBIOPutchar(char ch)
{
	return CTBIOWrite(&ch, 1);
}

int CTBIOAllowInterrupts(int flag)
{
	int oldFlag;
	oldFlag = allowInterrupts;
	allowInterrupts = flag;
	interruptOccurred &= flag;
	return oldFlag;
}

CTBIOInterrupt(void)
{
	interruptOccurred = TRUE;
}

CTBIOClose ( dtr )
{
	CMErr cmErr;

	if ( gSerConHdl) {
		printmsg(2, "Flushing connection");
		CMIOKill( gSerConHdl, cmDataIn);
		CMIOKill( gSerConHdl, cmDataOut);
		CMReset(gSerConHdl);
		closeCompleted = 0;
		printmsg(2, "Closing connection");
		cmErr = CMClose( gSerConHdl, asynchronous, (ProcPtr) CTBIOCloseCompletion, -1L, TRUE);
		if (asynchronous) {
			while (cmErr == noErr && !closeCompleted) {
				short errCode;
				CMBufferSizes	sizes;
				CMStatFlags		flags;
				CMIdle(gSerConHdl);
				cmErr = CMStatus(gSerConHdl, sizes, &flags);
				(void) Check_Events(6);
				if (Main_State == Abort_Program) {
					CMDispose( gSerConHdl);		
					exit(-1);
				}
	#ifdef NOTDEF
				if (!(flags & (cmStatusOpen | cmStatusOpening))) {
					break;
				}
	#endif
			}
		}
		printmsg(1, "%ld bytes written mainline, %ld bytes written from interrupt handler",
			foregroundWrites, backgroundWrites);
		CMDispose( gSerConHdl);		
		gSerConHdl = NULL;
		currentConnection = (ConnectionHandler *) NULL;
	}
}


CTBIOPurge ( void)
{
	CMIOKill( gSerConHdl, cmDataIn);
	CMIOKill( gSerConHdl, cmDataOut);
}


/* timeout is in tenths of a second */
int CTBIORead ( char *byt, int mincount, int maxcount, long int tenths )
{
long			endTime, count;
CMBufferSizes	sizes;
CMStatFlags		flags;
CMFlags			eomFlag;

	endTime = TickCount() + 6 * tenths;
	
	do {
	  if (Check_Events(0)) {
	    if (Main_State == Abort_Program)  {
				CTBIOPurge();
				CMClose( gSerConHdl, FALSE, NULL, -1L, FALSE);
				CMDispose( gSerConHdl);
	    	exit(-1);
	    }
	  }
		/* check if we have enough bytes in read buffer */
		
		if (connectionShutDown) {
			return -1;
		}
		CTBIOPush(); /* cmIdle and push unwritten output */
		CMStatus( gSerConHdl, sizes, &flags);
		if ( sizes[ cmDataIn] >= mincount) {
			count = maxcount;
			CMRead(  gSerConHdl, byt, &count, cmData, FALSE, NULL, -1, &eomFlag);
			return (int) count;
		}
		if (allowInterrupts && interruptOccurred) {
			return -1;
		}
	} while ( TickCount() < endTime);
	
	return (int) sizes[ cmDataIn];		/* return max available (or -1?) */
}

int CTBIOWrite ( char *buf, int count )
{
long	length;
long avail, toWrite, toEndOfBuffer;
CMErr cmErr;

	if (connectionShutDown) {
		return 1;
	}
	if (outbuf && asynchronous) {
		while (count > 0) {
			serialIOLock = 1;
			avail = outbufSize - writeCount;
			if (avail == 0) {
				serialIOLock = 0;
				CTBIOWaitOne();
				continue;
			}
			if (count > 0) {
				toEndOfBuffer = outbufSize - writeBase;
				toWrite = count;
				if (toWrite > avail) toWrite = avail;
				if (toWrite > toEndOfBuffer) toWrite = toEndOfBuffer;
				memcpy(outbuf + writeBase, buf, toWrite);
				writeBase = (writeBase + toWrite) % outbufSize;
				buf += toWrite;
				writeCount += toWrite;
				count -= toWrite;
			}
			serialIOLock = 0;
			cmErr = CTBIOPush();
			if (cmErr) {
				return cmErr;
			}
#ifdef NOTDEF
			if (serialIOCompleted) {
				serialIOCompleted = 0;
				serialIOBusy = 0;
				CTBIOEndAction(gSerConHdl);
			}
			if (!serialIOBusy && writeCount > 0) {
				length = writeCount;
				serialIOBusy = 1;
				printmsg(12, "Start write, %ld bytes", length);
				foregroundWrites += length;
				cmErr = CMWrite( gSerConHdl, outbuf + writeOrigin, &length, cmData, TRUE, (ProcPtr) CTBIOCompletion, -1L, 0);
				if (cmErr != noErr) {
					serialIOBusy = 0;
					connectionShutDown = TRUE;
					if (cmErr == cmNotOpen) {
						printmsg(0, "Connection closed");
					} else {
						printmsg(0, "CMWrite error %d", cmErr);
					}
					return 1;
				}
			}
#endif
		} 
	} else if (asynchronous) {
		serialIOBusy = 1;
		length = count;
		foregroundWrites += length;
		cmErr = CMWrite( gSerConHdl, buf, &length, cmData, TRUE, (ProcPtr) CTBIOCompletion, -1L, 0);
		if (cmErr != noErr) {
			connectionShutDown = TRUE;
			if (cmErr == cmNotOpen) {
				printmsg(0, "Connection closed");
			} else {
				printmsg(0, "CMWrite error %d", cmErr);
			}
			return 1;
		}
		CTBIOWait();
	} else {
		length = count;
		foregroundWrites += length;
	  if (Check_Events(0)) {
	    if (Main_State == Abort_Program)  {
				CTBIOPurge();
				CMClose( gSerConHdl, FALSE, NULL, -1L, FALSE);
				CMDispose( gSerConHdl);
	    	exit(-1);
	    }
	  }
		cmErr = CMWrite( gSerConHdl, buf, &length, cmData, FALSE, (ProcPtr) NULL, -1L, 0);
		if (cmErr != noErr) {
			connectionShutDown = TRUE;
			if (cmErr == cmNotOpen) {
				printmsg(0, "Connection closed");
			} else {
				printmsg(0, "CMWrite error %d", cmErr);
			}
			return 1;
		}
	}

	return 0 /* (int) count */;
	
/*
	Memo to self - should really clean up CTBIOWrite, SIOWrite, putu, swrite, the
	gsendpkt/fsendpkt, and the send-data loop in dcpxfer, to agree on what value a
	low-level-write routine really returns and what it means.  There's some disagreement
	in the code... some routines expect a 0-for-OK-or-nonzero-error-code, some expect
	or provide a bytes-actually-written count, some don't care.  For now, it's the former.
	-- dplatt
*/
}

CTBIOIdle()
{
	long	length;
	CMErr cmErr;
	if (gSerConHdl) {
		CTBIOPush();
	}
}

int CTBIOEvent(EventRecord *anEvent)
{
  WindowPeek theWindow;
  int code;
	if (!gSerConHdl) {
		return 0;
	}
	switch (anEvent->what) {
		case activateEvt:
			theWindow = (WindowPeek) anEvent->message;
			if (gSerConHdl != (ConnHandle) theWindow->refCon) {
				return 0;
			}
			if (anEvent->modifiers & activeFlag) {
				CMActivate(gSerConHdl, TRUE);
			} else {
				CMActivate(gSerConHdl, FALSE);
			}
			return 1;
	    	    	
		case updateEvt:
			theWindow = (WindowPeek) anEvent->message;
			if (gSerConHdl != (ConnHandle) theWindow->refCon) {
				return 0;
			}
			CMEvent(gSerConHdl, anEvent);
			return 1;

		case mouseDown:
    	code = FindWindow(anEvent->where,&theWindow);
    	switch (code) {
    		case inContent:
    		case inDrag:
    		case inGrow:
    		case inGoAway:
					if (gSerConHdl == (ConnHandle) theWindow->refCon) {
						CMEvent(gSerConHdl, anEvent);
						return 1;
					}
					break;
				default:
					break;
			}
			return 0;
	    	    	
		case osEvt:
			if (((anEvent->message >> 24) & 0xFF) == 1 /* suspend */) {
				if (anEvent->message & 1 /* resume bit */) {
					CMActivate(gSerConHdl, TRUE);
				} else {
					CMActivate(gSerConHdl, FALSE);
				}
			}
			return 0;
		
		default:
			return 0;
			
	}
}

int CTBIOBreak(int tenths)
{
	CMBreak(gSerConHdl, tenths*2, FALSE, (ProcPtr) NULL);
}

int CTBIOSetParity(int dataBitsCode, int parityCode)
{
	OSErr errCode;
	switch (dataBitsCode) {
		case data7:
			errCode = CMSetConfig(gSerConHdl, "DataBits 7");
			break;
		case data8:
			errCode = CMSetConfig(gSerConHdl, "DataBits 8");
			break;
	}
	switch (parityCode) {
		case evenParity:
			errCode = CMSetConfig(gSerConHdl, "Parity Even");
			break;
		case oddParity:
			errCode = CMSetConfig(gSerConHdl, "Parity Odd");
			break;
		case noParity:
			errCode = CMSetConfig(gSerConHdl, "Parity None");
			break;
	}
	return;
}

		
	
