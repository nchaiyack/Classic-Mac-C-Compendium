/*
	LogLibComponent.h
	
	Public Header file for user interaction with the LogLib Component.
	
*/

// ensure that this header is only included once...
#pragma once

// make doubly-sure...
#ifndef __H_LogLibComponent__
#define __H_LogLibComponent__

// include the Components.h file
#ifndef __COMPONENTS__
#include <Components.h>
#endif

/*
	In the universal headers the definition for ComponentCallNow has been changed so that the equal sign
	is no longer needed.  This kludge will allow the file to be compiled by either the old or universal headers.
*/
#if USESROUTINEDESCRIPTORS
#define EQUALS
#else
#define EQUALS =
#endif

// the LogLib Component type
#define kLogLibComponentType		('LogC')

#ifndef kComponentWildCard
#define kComponentWildCard		(0L)
#endif

#define kLogDefaultCreator		('R*ch')
#define kLogDefaultType			('TEXT')
#define kLogDefaultOpenSetting		(false)
#define kNewLine				0x0d		/* the Mac's newline character */

// the LogLib Component selector codes
#define kLogStorage				(0x10)
#define kLogDefaults				(0x11)
#define kLogSetup				(0x12)
#define kLogSetupFSp			(0x13)
#define kLogText				(0x14)
#define kLogTime				(0x15)
#define kLogWrite				(0x16)
#define kLogPText				(0x17)
#define kLogPTime				(0x18)

// the LogLib Storage Structure
typedef struct loglibstruct{
	// the basics
	FSSpec			logSpec;			// FSSpec for the log file
	short			fileRefNum;		// the file's reference number
	
	// the settings
	Boolean			inited;			// did the FSSpec get inited or not?
	Boolean			keepLogOpen;		// Should the log be kept open at all times? Default is no
	Boolean			savedValue;		// saved for when user requests direct access to the file...
	Boolean			wasOpened;		// was the file opened at all yet?
	OSType			creator;			// Creator for the log type, default is 'R*ch'
	OSType			type;			// Type for the log, default is 'TEXT'
	
	// my standard component storage stuff
	Component		self;				// the component saves itself
	ComponentInstance	kidnapper;			// when targeted, the component saves its kidnapper
	Component		delegate;			// when this component targets another, it saves the info
	ComponentInstance	delegated;
	short			resFileRefNum;		// this is the ref number for the components resource file.
	
} LogLibRec,* LogLibPtr,** LogLibHdl;

// so C++ compilers won't choke...
#ifdef __cplusplus
extern "C" {
#endif

// set the default type&creator, also indicate whether to keep log open or open/close as needed.
pascal ComponentResult LogDefaults(ComponentInstance logcomp,OSType creator,OSType type,Boolean keepOpen)\
		ComponentCallNow( kLogDefaults , 10 );
		
// set the file name & location for the log file
pascal ComponentResult LogSetup(ComponentInstance logcomp,StringPtr name,short vref, long dirid)\
		ComponentCallNow(kLogSetup,0x0a);

// same as above but use an FSSpec that has already been created.
pascal ComponentResult LogSetupFSSpec(ComponentInstance logcomp,FSSpec* fsspecptr)\
		ComponentCallNow(kLogSetupFSp,0x04);

// add the text pointed to by buffer to the log
pascal ComponentResult LogText(ComponentInstance logcomp,char* buffer)\
		ComponentCallNow(kLogText,0x04);

// add the date, time, and the text pointed to by buffer to the log
pascal ComponentResult LogTime(ComponentInstance logcomp,char* buffer)\
		ComponentCallNow(kLogTime,0x04);

// write a buffer to the log file
pascal ComponentResult LogWrite(ComponentInstance logcomp,long* size,Ptr buffer)\
		ComponentCallNow(kLogWrite,0x08);

// add the pascal string pointed to by buffer to the log
pascal ComponentResult LogPText(ComponentInstance logcomp,unsigned char* buffer)\
		ComponentCallNow(kLogPText,0x04);

// add the date, time, and the pascal string pointed to by buffer to the log
pascal ComponentResult LogPTime(ComponentInstance logcomp,unsigned char* buffer)\
		ComponentCallNow(kLogPTime,0x04);
		
#ifdef __cplusplus
}
#endif

#endif
