// WASTE Object public routines.
//
//		maintained by Michael F. Kamprath
//		kamprath@earthlink.net
//
#ifndef __SOUND__
	#include <Sound.h>
#endif

#ifndef _WASTE_
	#include "WASTE.h"
#endif

#include <Icons.h>
#include <SoundInput.h>
#if !defined(GENERATINGPOWERPC) && !defined(GENERATING68K)
typedef Handle SndListHandle;
#endif

#ifdef __cplusplus
extern "C" {
#endif

// Initialization

OSErr	InstallAllWASTEObjHandlers( void );  // Installs all object handlers

OSErr	InstallPICTObject( WEHandle theWE );
OSErr	InstallSoundObject( WEHandle theWE );
OSErr	InstallHFSObject( WEHandle theWE );

// PICT Object public routines

// Sound object public routines

void	CheckSoundStatus( void );
OSErr	PlaySelectedSound( WEHandle theWE );
OSErr	StopCurrentSound( void );
Boolean	SoundIsPlaying( void );

OSErr	CreateNewSoundObject( WEHandle theWE );
OSErr	PlaySoundHandle( SndListHandle theSound );

// File object public routines

OSErr	InsertFileRefFromFSSpec( FSSpec *theFile, WEHandle theWE );

#ifdef __cplusplus
}
#endif
