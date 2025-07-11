/*
	MacTCPExtras.h
	
	Header file for all of the little "extras" which are included in the library,
	but are scattered throughout the source.
	
	01/18/94 dn - Created
	01/30/94 dn - Added PBControl calls.
*/

#pragma once

#ifndef __H_MacTCPExtras__
#define __H_MacTCPExtras__

#include <OSUtils.h>
#include <Files.h>

// error codes
enum {
	extrasBaseErr=23200,			// the base for the extras
	nilParmBlkPtr					// the ParmBlkPtr was nil.
	
};

#define __H_LibTraps__

#define TrapMask 0x0800

/*
	The Prototypes
*/

#ifdef __cplusplus
extern "C" {
#endif

// String/Memory Stuff
short StrLen(char* string);							// return a string's len
void MemSet(char* mem,char ch,long size);				// set memory to a value

// PBControl Stuff
OSErr SyncAsync(ParmBlkPtr pb,Boolean async);			// do a sync/async call
OSErr Sync(ParmBlkPtr pb);							// do a synchronous call
OSErr Async(ParmBlkPtr pb);							// do an asynchronous call

// DNR Folder extras
OSErr GetSystemFolder(short* vref,long* dirID);			// find the system folder
OSErr GetCPanelFolder(short* vref,long* dirID);			// find the control panels folder

// Trap extras
short NumToolboxTraps(void);							// returns the number of traps in the system
TrapType GetTrapType(short theTrap);					// returns the trap type of a given trap
Boolean TrapAvailable(short theTrap);					// determines whether a trap is available

#ifdef __cplusplus
}
#endif

#endif /* __H_MacTCPExtras__ */
