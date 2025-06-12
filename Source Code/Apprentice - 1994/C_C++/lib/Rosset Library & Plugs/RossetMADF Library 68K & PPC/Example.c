//
//		Example of MADF Library
//		Copyright 1994 Antoine ROSSET
//
//
//		FREEWARE


#include "MAD.h"
#include "RDriver.h"
#include "gestaltequ.h"
#include "sound.h"

static 	Boolean		Stereo, StereoMixing, NewSoundManager, hasASC;

void MyDebugStr( Str255 theStr);

void MyDebugStr( Str255 theStr)
{
	// MusicDriver return FATAL error in this function
	
	DebugStr( theStr);
}

main()
{
long			gestaltAnswer;
int				myBit;
NumVersion		nVers;
Point			where = { -1, -1};
SFReply			reply;
SFTypeList		aType;

InitGraf( &qd.thePort);
InitFonts();
FlushEvents(everyEvent,0);
InitWindows();
TEInit();
InitMenus();
InitCursor();
MaxApplZone();

/****** HARDWARE IDENTIFICATION AND CHECK **********/

/** ASC CHIP ? **/

Gestalt( gestaltHardwareAttr, &gestaltAnswer);
myBit = gestaltHasASC;
if( BitTst( &gestaltAnswer, 31-myBit) == false) hasASC = false;
else hasASC = true;

/** STEREO ? **/

Gestalt( gestaltSoundAttr, &gestaltAnswer);
myBit = gestaltStereoCapability;
Stereo = BitTst( &gestaltAnswer, 31-myBit);

/** STEREO MIXING ? **/

myBit = gestaltStereoMixing;
StereoMixing = BitTst( &gestaltAnswer, 31-myBit);

/** SOUND MANAGER >3.0 ? **/

nVers = SndSoundManagerVersion();
if( nVers.majorRev >= 3) NewSoundManager = true;
else NewSoundManager = false;
/****************************************************/


/****** CHOOSE A MADF File ********/
aType[ 0] = 'MADF';
SFGetFile( where, "\p", 0L, 1, aType, 0L, &reply);
SetVol( 0L, reply.vRefNum);


/*** To load a MADF File *********/
if( RLoadMOD( reply.fName) != noErr) Debugger();


/*
	TO OPEN A MADF RESOURCE

	To create a MADF Resource, use Auto-Exec export from Player PRO !!!!

	And load the resource with: by example...

	RLoadMADFRsrc( 'MADF', 3124);
*/

/*********************************/

/********* MADF Library Initialisation : choose the best driver for the current hardware ******/

if( Stereo == true && StereoMixing == true)
{		
	if( NewSoundManager == true)
	{
		if( RInitMOD( SMStereo, thePartition.header->Tracks) != noErr) ExitToShell();
	}
	else if( hasASC == true)
	{
		if( RInitMOD( ASCStereo, thePartition.header->Tracks) != noErr) ExitToShell();
	}
	else if( RInitMOD( SMDSP, thePartition.header->Tracks) != noErr) ExitToShell();
}
else
{		
	if( NewSoundManager == true)
	{
		if( RInitMOD( SMMono, thePartition.header->Tracks) != noErr) ExitToShell();
	}
	else if( hasASC == true)
	{
		if( RInitMOD( ASCMono, thePartition.header->Tracks) != noErr) ExitToShell();
	}
	else DebugStr("\pNo Driver for your Mac!");
}	

RPlayMOD();
Reading = true;

while( !Button())
{
	/** Do what you want here.... **/
	
	/* Bla bla...*/
}


RStopMOD();			// Stop music
RClearMOD();		// Clear MADF Music
RQuitMOD();			// Clear MADF Driver

FlushEvents( everyEvent, 0);
ExitToShell();
}

