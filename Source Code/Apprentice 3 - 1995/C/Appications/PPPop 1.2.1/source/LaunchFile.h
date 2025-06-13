#pragma once

#include <Aliases.h>
#include <Files.h>
#include <AppleEvents.h>
#include <Folders.h>
#include <GestaltEqu.h>
#include <Processes.h>
#include "PPPop.h"

#define kFinderSig			'FNDR'
#define kAEFinderEvents		'FNDR'
#define kSystemType			'MACS'
#define	kAEOpenSelection	'sope'
#define	keySelection		'fsel'
#define kMacPPPType			'Mppp'
#define kMacTCPType			'ztcp'
#define kcdevType			'cdev'
#define kBufferSize			16384		// 16k buffer
#define kGestaltMask		1L
#define	NIL					0L


// Prototypes

void	OpenConfigPPP(void);
void	OpenMacTCP(void);
void	SetUpLaunch(void);
OSErr 	FindAProcess(OSType, OSType, ProcessSerialNumber*);
OSErr	OpenSelection(FSSpecPtr theDoc);
OSErr 	ScheduleFinder( void );
long 	FindControlPanel(FSSpec *resultSpec, OSType theType, short vRefNum, long dirID);
