/*
** MiniTelnet.const.h
**
**	MiniTelnet application
**	Constants used by application
**
**	Copyright � 1993-94, FrostByte Design / Eric Scouten
**
*/


#pragma once


// MiniTelnet file parameters

#define kAppSignature		'dTCP'
#define kSettingsFileType		'dTSR'


// memory size information

#define kExtraMasters		4				// adjust based on observations
#define kRainyDayFund		20480
#define kCriticalBalance		20480
#define kToolboxBalance		20480


// Telnet connection parameters

#define kTelnetPort			23U
#define kTelnetRecBufferSize	16384L
#define kTelnetAutoRecSize	4
#define kTelnetAutoRecNum	4


// resource IDs

#define DLOGAboutBox		1098


// miscellaneous constants

#define kSplashScreenTicks	180				// how long to display splash screen (3 seconds)
#define telnetPort			23


// terminal emulation modes

enum TermNumber {
	termUnknown = 0,
	termMax								// highest terminal # defined
};
typedef enum TermNumber TermNumber;


// Telnet settings record

#define kTSRsignature		0x518C			// proper value for signature field
#define kTSRversion			0				// current version of settings record

#if defined(powerc) || defined (__powerc)
#pragma options align=mac68k
#endif
struct TelnetSettingsRec {
	short	signature;						// ensure we�ve got our kind of file
	short	settingsVersion;				// version of settings record
	short	settingsMinVersion;				// minimum compatible version of settings
	char		unused[10];					// align hostname on 16-byte interval
	
	char		hostName[256];				// user�s name for host & port number
	char		termEmulation[48];				// IANA name of terminal emulation

	char		backspaceChar;					// character to substitute for backspace
	Boolean	closeOnSessionEnd;				// close window when session ends
	Boolean	showDebug;					// show debugging codes
};
#if defined(powerc) || defined(__powerc)
#pragma options align=reset
#endif

typedef struct TelnetSettingsRec TelnetSettingsRec;
