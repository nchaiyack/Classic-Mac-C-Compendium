// Sound Object Handler for the WASTE Text Engine
// by Michael F. Kamprath, kamprath@earthlink.net
//
// v1.0, 16 March 1995
// v1.0.1, 13 May 1995, changed the sound playing code a little.  Also added gestalt
//						check for sound recording device when CreateNewSoundObject()
//						is called.
// v1.1, 5 June 95, Made some changes recomended by Chris Thomas, <THUNDERONE@delphi.com>:
//			-	Playing an object's sound is done by ultilizing the object's data
//				directly rather than making a copy.  Using PlaySoundHandle() for
//				your sounds still makes a copy.
//			-	Added the public routines StopCurrentSound() and IsSoundPlaying().
//			-	Option to have the sound sound created at the handler's initilization.
//			-	Added InstallSoundObject() to install sound object handler only.
//


#include "WASTE.h"

#include "WE_snd_Handler.h"
#include "WASTE_Objects.h"

//
// Local only routines
//

static	OSErr	PlayCurrentGlobalSound( void );
static	SndChannelPtr	CreateNewSoundChannel( void );

//
// Sound Channel and current playing sound pointers
//
static SndChannelPtr	gSoundChannel = nil;
static SndListHandle	gCurrentSound = nil;
static short			gCurSoundHandleStatus = 0;

#define		kIsWASTESound		0x0100

// 
// Routine and variables for installing sound object into WASTE
//

static WENewObjectUPP           newSndUPP = NULL;
static WEDisposeObjectUPP       disposeSndUPP = NULL;
static WEDrawObjectUPP          drawSndUPP = NULL;
static WEClickObjectUPP         clickSndUPP = NULL;

OSErr	InstallSoundObject( WEHandle theWE )
{
OSErr	iErr;

	if (newSndUPP == NULL)
	{

		newSndUPP = NewWENewObjectProc(HandleNewSound);
		disposeSndUPP = NewWEDisposeObjectProc(HandleDisposeSound);
		drawSndUPP = NewWEDrawObjectProc(HandleDrawSound);
		clickSndUPP = NewWEClickObjectProc(HandleClickSound);

		iErr = WEInstallObjectHandler('snd ', weNewHandler, (UniversalProcPtr)newSndUPP, theWE);
		if (iErr) return(iErr);
		iErr = WEInstallObjectHandler('snd ', weDisposeHandler, (UniversalProcPtr)disposeSndUPP, theWE);
		if (iErr) return(iErr);
		iErr = WEInstallObjectHandler('snd ', weDrawHandler, (UniversalProcPtr)drawSndUPP, theWE);
		if (iErr) return(iErr);
		iErr = WEInstallObjectHandler('snd ', weClickHandler, (UniversalProcPtr)clickSndUPP, theWE);
		if (iErr) return(iErr);
	
#if CREATE_CHANNEL_AT_INIT
		gSoundChannel = CreateNewSoundChannel();
#endif
	}
	
	return(noErr);
}

//
// New Object Handler for sounds
//
pascal OSErr	HandleNewSound(Point *defaultObjectSize,WEObjectReference objectRef)
{
#pragma unused (objectRef)

	(*defaultObjectSize).h = 32;
	(*defaultObjectSize).v = 32;

	return(noErr);
}
//
// Dispose Object handler for sounds
//

pascal OSErr	HandleDisposeSound(WEObjectReference objectRef )
{
SndListHandle	theSound;

	theSound = (SndListHandle)WEGetObjectDataHandle(objectRef);

	if (theSound == gCurrentSound)
		StopCurrentSound();
		
	DisposeHandle((Handle)theSound);

	return(MemError());
}
//
// Draw Object handler for sounds
//
pascal OSErr	HandleDrawSound(Rect *destRect, WEObjectReference objectRef )
{
#pragma unused (objectRef)

OSErr	iErr;
	
	iErr = PlotIconID(destRect,atNone,ttNone,kSoundIconID);
	
	return( iErr );
}
//
// Click Object Handler for sounds
//

pascal Boolean	HandleClickSound(	Point hitPt, 
							short modifiers, 
							long clickTime, 
							WEObjectReference objectRef)
{
#pragma unused ( hitPt, clickTime)

OSErr			iErr;

	if (modifiers & 0x0001)         // look for double-clicks
	{
		iErr = StopCurrentSound();
		
		gCurrentSound = (SndListHandle)WEGetObjectDataHandle( objectRef );
		iErr = PlayCurrentGlobalSound();
		gCurSoundHandleStatus += kIsWASTESound;
		
		if (!iErr)
			return(true);
		else
		{
			StopCurrentSound();
			return(false);
		}
	}
	else
		return(false);

}

//
// PlaySelectedSound()
//		If a sound object, and only a sound object, is selected,
//		PlaySelectedSound() will play it.
//
OSErr	PlaySelectedSound( WEHandle theWE )
{
WEObjectReference	objectRef;
SndListHandle		theSound;
OSErr				iErr = noErr;

	iErr = WEGetSelectedObject( &objectRef, theWE );
	if (!iErr)
	{
		theSound = (SndListHandle)WEGetObjectDataHandle( objectRef );
		iErr = PlaySoundHandle( theSound );
	}
		
	return( iErr );
}
//
// CreateNewSoundObject()
//		Uses built in sound recording to create a new sound object.
//
OSErr	CreateNewSoundObject( WEHandle theWE )
{
OSErr			iErr;
SndListHandle	sndHandle = nil;
long			gestaltResponse;
Point			corner = {32,32};

	iErr = Gestalt(gestaltSoundAttr, &gestaltResponse);
	if ( (iErr == noErr)&&(gestaltResponse&gestaltHasSoundInputDevice))
	{
		iErr = SndRecord(nil,corner,siGoodQuality,&sndHandle);
	
		if (iErr == noErr)
		{
			corner.h = 32;
			corner.v = 32;
			WEInsertObject( 'snd ', (Handle)sndHandle, corner, theWE );
		}
	}
	return( iErr );
}

//
// PlaySoundHandle()
//		Ultility routine to play a sound async.  Maintains a sound channel 
//		and the sound data to do so. 
//
//		Intended for use by external code.
//

OSErr	PlaySoundHandle( SndListHandle theSound )
{
OSErr		iErr = noErr;

	
	// If a sound is currently playing, stop it and dispose of it's buffer.
	iErr = StopCurrentSound();
	
	// create a new buffer for the selected sound.
	gCurrentSound = theSound;
	iErr = HandToHand((Handle *)&(gCurrentSound));
	
	if (!iErr)
	{
		iErr = PlayCurrentGlobalSound();
		
		if (iErr)
		{
			HUnlock( (Handle)gCurrentSound );
			DisposeHandle( (Handle)gCurrentSound );
			gCurrentSound = nil;
		}	
	}
	
	return(iErr);
}

//
// PlayCurrentGlobalSound()
//		Used locally by the object handler to play the current sound.
//

static	OSErr	PlayCurrentGlobalSound( void )
{
OSErr	iErr = noErr;

	// check for sound channel
	if (!gSoundChannel)
		gSoundChannel = CreateNewSoundChannel();

	if (gCurrentSound&&gSoundChannel)
	{
		// Lock the buffer and play the sound.
		gCurSoundHandleStatus = HGetState( (Handle)gCurrentSound );
		HLockHi( (Handle)gCurrentSound );
	
		iErr = SndPlay(gSoundChannel,gCurrentSound,true);
	}
	
	return(iErr);
}

//
// StopCurrentSound()
//		Stops the current sound and returns memory to original state.
//

OSErr	StopCurrentSound( void )
{
SndCommand	cmd;
OSErr		iErr;

	if (gCurrentSound&&gSoundChannel)
	{
		cmd.cmd = quietCmd;
		iErr = SndDoImmediate(gSoundChannel,&cmd);
		
		HSetState( (Handle)gCurrentSound, (char)(gCurSoundHandleStatus&0x00FF) );
		
		iErr = MemError();
		if (iErr) return(iErr);
		
		if (!(gCurSoundHandleStatus&kIsWASTESound))
			DisposeHandle( (Handle)gCurrentSound );
			
		gCurrentSound = nil;
		gCurSoundHandleStatus = 0;
		
		return(MemError());
	}

	return(noErr);
}
//
// CreateNewSoundChannel()
//		Used to create a new shound channel.
//
static SndChannelPtr	CreateNewSoundChannel( void )
{
OSErr			iErr;
SndChannelPtr	chan;
		
	// allocate a sound channel
	chan = (SndChannelPtr)NewPtrClear(sizeof(SndChannel));
	if ( chan != nil ) 
	{
		chan->qLength = stdQLength;	 //128 sound commands
		iErr = SndNewChannel(&chan, sampledSynth, 0, nil);
		if (iErr)
		{
			DisposePtr((Ptr)chan);
			chan = nil;
		}
	}
	return(chan);			// return SndChannelPtr
}

//
// CheckSoundStatus()
//		Should be called periodically to dispose of unneeded sound buffers.
//
void CheckSoundStatus( void )
{
	if (SoundIsPlaying())
	{
		HSetState( (Handle)gCurrentSound, (char)(gCurSoundHandleStatus&0x00FF) );
						
		if (!gCurSoundHandleStatus&kIsWASTESound)
			DisposeHandle( (Handle)gCurrentSound );
			
		gCurrentSound = nil;
		gCurSoundHandleStatus = 0;
	}		
}

//
// SoundIsPlaying()
//		Returns true if a sound is being played by the object handler, 
//		false if not.
//

Boolean	SoundIsPlaying( void )
{
OSErr		iErr;
SCStatus	theStatus;

	if (gSoundChannel)
	{
		iErr = SndChannelStatus(gSoundChannel,sizeof(SCStatus),&theStatus);
		if (( !theStatus.scChannelBusy )&&(gCurrentSound))
			return( true );
	}
	
	return( false );
}