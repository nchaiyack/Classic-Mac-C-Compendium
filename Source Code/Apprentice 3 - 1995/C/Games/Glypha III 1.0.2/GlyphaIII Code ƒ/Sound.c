
//============================================================================
//----------------------------------------------------------------------------
//									Sound.c
//----------------------------------------------------------------------------
//============================================================================

// This file handles all sound routines.  It handles 2 concurrent sound�
// channels allowing 2 sounds to be played simultaneously.  It also handles�
// a system of priorites whereby you can ensure that "important" sounds don't�
// get cut off by "lesser" sounds.  In that there are 2 channels however,�
// "lesser" sounds are not discounted outright - both channels are considered�
// to determine if one of the channels is not playing at all (priority = 0) or�
// playing a sound of an even lesser priority.  Make sense?

#include <Sound.h>
#include "Externs.h"


#define kMaxSounds				17			// Number of sounds to load.
#define	kBaseBufferSoundID		1000		// ID of first sound (assumed sequential).
#define kSoundDone				913			// Just a number I chose.
#define kSoundDone2				749			// Just a number I chose.


void PlaySound1 (short, short);
void PlaySound2 (short, short);
pascal void ExternalCallBack (SndChannelPtr, SndCommand);
pascal void ExternalCallBack2 (SndChannelPtr, SndCommand);
void LoadAllSounds (void);
OSErr LoadBufferSounds (void);
OSErr DumpBufferSounds (void);
OSErr OpenSoundChannel (void);
OSErr CloseSoundChannel (void);


SndCallBackUPP		externalCallBackUPP, externalCallBackUPP2;
SndChannelPtr		externalChannel, externalChannel2;
Ptr					theSoundData[kMaxSounds];
short				externalPriority, externalPriority2;
Boolean				channelOpen, soundOn;


//==============================================================  Functions
//--------------------------------------------------------------  PlaySound1

// This function takes a sound ID and a priority, and forces that sound to 
// play through channel 1 - and saves the priority globally.  As well, a 
// callback command is queues up in channel 1.

void PlaySound1 (short soundID, short priority)
{
	SndCommand	theCommand;
	OSErr		theErr;
	
	theCommand.cmd = flushCmd;			// Send 1st a flushCmd to clear the sound queue.
	theCommand.param1 = 0;
	theCommand.param2 = 0L;
	theErr = SndDoImmediate(externalChannel, &theCommand);
	
	theCommand.cmd = quietCmd;			// Send quietCmd to stop any current sound.
	theCommand.param1 = 0;
	theCommand.param2 = 0L;
	theErr = SndDoImmediate(externalChannel, &theCommand);
	
	externalPriority = priority;		// Copy priority to global variable.
	
	theCommand.cmd = bufferCmd;			// Then, send a bufferCmd to channel 1.
	theCommand.param1 = 0;				// The sound played will be soundID.
	theCommand.param2 = (long)(theSoundData[soundID]);
	theErr = SndDoImmediate(externalChannel, &theCommand);
	
	theCommand.cmd = callBackCmd;		// Lastly, queue up a callBackCmd to notify us�
	theCommand.param1 = kSoundDone;		// when the sound has finished playing.
	theCommand.param2 = SetCurrentA5();
	theErr = SndDoCommand(externalChannel, &theCommand, TRUE);
}

//--------------------------------------------------------------  PlaySound2

// This function is identical to the above function except that it handles�
// playing sounds through channel 2.

void PlaySound2 (short soundID, short priority)
{
	SndCommand	theCommand;
	OSErr		theErr;
	
	theCommand.cmd = flushCmd;			// Send 1st a flushCmd to clear the sound queue.
	theCommand.param1 = 0;
	theCommand.param2 = 0L;
	theErr = SndDoImmediate(externalChannel2, &theCommand);
	
	theCommand.cmd = quietCmd;			// Send quietCmd to stop any current sound.
	theCommand.param1 = 0;
	theCommand.param2 = 0L;
	theErr = SndDoImmediate(externalChannel2, &theCommand);
	
	externalPriority2 = priority;		// Copy priority to global variable.
	
	theCommand.cmd = bufferCmd;			// Then, send a bufferCmd to channel 1.
	theCommand.param1 = 0;				// The sound played will be soundID.
	theCommand.param2 = (long)(theSoundData[soundID]);
	theErr = SndDoImmediate(externalChannel2, &theCommand);
	
	theCommand.cmd = callBackCmd;		// Lastly, queue up a callBackCmd to notify us�
	theCommand.param1 = kSoundDone2;	// when the sound has finished playing.
	theCommand.param2 = SetCurrentA5();
	theErr = SndDoCommand(externalChannel2, &theCommand, TRUE);
}

//--------------------------------------------------------  PlayExternalSound

// This function is probably poorly named for this application.  I lifted this�
// whole library from one of my games and chopped it down for purposes of Glypha.
// The original game treated "external" and "cockpit" sounds as seperate channels�
// (such that cockpit sounds could only "override" other cockpit sounds and�
// external sounds could only override other external sounds.
// In any event, this is the primary function called from throughout Glypha.
// This function is called with a sound ID and a priority (just some number) and�
// the function then determines if one of the two sound channels is free to play�
// the sound.  It determines this by way of priorities.  If a sound channel is�
// idle and playing no sound, its channel priority is 0.  Since the priority of�
// the sound you want to play is assumed to be greater than 0, it will, without�
// a doubt, be allowed to play on an idle channel.  If however there is already�
// a sound playing (the channel's priority is not equal to 0), the sound with the�
// largest priority wins.  Mind you though that there are two channels to choose�
// between.  Therefore, the function compares the priority passed in with the�
// sound channel with the lowest priority.

void PlayExternalSound (short soundID, short priority)
{							// A little error-checking.
	if ((soundID >= 0) && (soundID < kMaxSounds))
	{
		if (soundOn)		// More error-checking.
		{					// Find channel with lowest priority.
			if (externalPriority < externalPriority2)
			{				// Compare priority with that of channel 1.
				if (priority >= externalPriority)
					PlaySound1(soundID, priority);
			}
			else
			{				// Compare priority with that of channel 2.
				if (priority >= externalPriority2)
					PlaySound2(soundID, priority);
			}
		}
	}
}

//--------------------------------------------------------  ExternalCallBack

// Callback routine.  If this looks ugly, blame Apple's Universal Headers.
// The callback routine is called after a sound finishes playing.  The�
// callback routine is extremely useful in that it enables us to know when�
// to set the sound channels priority back to 0 (meaning no sound playing).
// Keep in mind (by the way) that this funciton is called at interrupt time�
// and thus may not cause memory to be moved.  Also, note that also because�
// of the interupt situation, we need to handle setting A5 to point to our�
// app's A5 and then set it back again.

RoutineDescriptor ExternalCallBackRD = 
		BUILD_ROUTINE_DESCRIPTOR(uppFilePlayCompletionProcInfo, ExternalCallBack);

pascal void ExternalCallBack (SndChannelPtr theChannel, SndCommand theCommand)
{
	long		thisA5, gameA5;
	
	if (theCommand.param1 == kSoundDone)	// See if it's OUR callback.
	{
		gameA5 = theCommand.param2;			// Extract our A5 from sound command.
		thisA5 = SetA5(gameA5);				// Point A5 to our app (save off current A5).
		
		externalPriority = 0;				// Set global to reflect no sound playing.
		
		thisA5 = SetA5(thisA5);				// Restire A5.
	}
}

//--------------------------------------------------------  ExternalCallBack2

// This function is identical to the above function but handles sound channel 2.

RoutineDescriptor ExternalCallBackRD2 = 
		BUILD_ROUTINE_DESCRIPTOR(uppFilePlayCompletionProcInfo, ExternalCallBack2);

pascal void ExternalCallBack2 (SndChannelPtr theChannel, SndCommand theCommand)
{
	long		thisA5, gameA5;
	
	if (theCommand.param1 == kSoundDone2)	// See if it's OUR callback.
	{
		gameA5 = theCommand.param2;			// Extract our A5 from sound command.
		thisA5 = SetA5(gameA5);				// Point A5 to our app (save off current A5).
		
		externalPriority2 = 0;				// Set global to reflect no sound playing.
		
		thisA5 = SetA5(thisA5);				// Restire A5.
	}
}

//--------------------------------------------------------  LoadBufferSounds

// This function loads up all the sounds we'll need in the game and then�
// strips off their header so that we can pass them as buffer commands.
// Sounds are stored in our resource fork as 'snd ' resources.  There is a�
// 20 byte header that we need to remove in order to use bufferCmd's.
// This function is called only once, when the game loads up.

OSErr LoadBufferSounds (void)
{
	Handle		theSound;
	long		soundDataSize;
	OSErr		theErr;
	short		i;
	
	theErr = noErr;						// Assume no errors.
	
	for (i = 0; i < kMaxSounds; i++)	// Walk through all sounds.
	{									// Load 'snd ' from resource.
		theSound = GetResource('snd ', i + kBaseBufferSoundID);
		if (theSound == 0L)				// Make sure it loaded okay.
			return (ResError());		// Return reason it failed (if it did).
		
		HLock(theSound);				// If we got this far, lock sound down.
										// Calculate size of sound minus header.
		soundDataSize = GetHandleSize(theSound) - 20L;
		HUnlock(theSound);				// Okay, unlock.
										// Create pointer the size calculated above.
		theSoundData[i] = NewPtr(soundDataSize);
		if (theSoundData[i] == 0L)		// See if we created it okay.
			return (MemError());		// If failed, return the reason why.
		HLock(theSound);				// Okay, lock the sound handle again.
										// Copy sound data (minus header) to our pointer.
		BlockMove((Ptr)(*theSound + 20L), theSoundData[i], soundDataSize);
		HUnlock(theSound);				// Unlock sound handle again.
		ReleaseResource(theSound);		// And toss it from memory.
	}
	
	return (theErr);
}

//--------------------------------------------------------  DumpBufferSounds

// This function is called when Glypha exits (quits).  All those nasty pointers�
// we created in the above function are reclaimed.

OSErr DumpBufferSounds (void)
{
	OSErr		theErr;
	short		i;
	
	theErr = noErr;
	
	for (i = 0; i < kMaxSounds; i++)		// Go through all sound pointers.
	{
		if (theSoundData[i] != 0L)			// Make sure it exists.
			DisposPtr(theSoundData[i]);		// Dispose of it.
		theSoundData[i] = 0L;				// Make sure it reflects its "nonexistence".
	}
	
	return (theErr);
}

//--------------------------------------------------------  OpenSoundChannel

// This should perhaps be called OpenSoundChannels() since it opens two.
// It is called once (at initialization) to set up the two sound channels�
// we will use throughout Glypha.  For purposes of speed, 8-bit sound channels�
// with no interpolation and monophonic are opened.  They'll use the sampled�
// synthesizer (digitized sound) and be assigned their respective callback�
// routines.

OSErr OpenSoundChannel (void)
{
	OSErr		theErr;
	
	#if USESROUTINEDESCRIPTORS
		externalCallBackUPP = &ExternalCallBackRD;	// Handle Universal Header ugliness.
		externalCallBackUPP2 = &ExternalCallBackRD2;
	#else
		externalCallBackUPP = (SndCallBackUPP) &ExternalCallBack;
		externalCallBackUPP2 = (SndCallBackUPP) &ExternalCallBack2;
	#endif
	
	theErr = noErr;									// Assume no errors.
	
	if (channelOpen)								// Error checking.
		return (theErr);
	
	externalChannel = 0L;
	theErr = SndNewChannel(&externalChannel, 		// Open channel 1.
			sampledSynth, initNoInterp + initMono, 
			(SndCallBackUPP)externalCallBackUPP);
	if (theErr == noErr)							// See if it worked.
		channelOpen = TRUE;
	
	externalChannel2 = 0L;
	theErr = SndNewChannel(&externalChannel2, 		// Open channel 2.
			sampledSynth, initNoInterp + initMono, 
			(SndCallBackUPP)externalCallBackUPP2);
	if (theErr == noErr)							// See if it worked.
		channelOpen = TRUE;
	
	return (theErr);
}

//--------------------------------------------------------  CloseSoundChannel

// This function is called only upon quitting Glypha.  Both sound channels�
// we created above are closed down.

OSErr CloseSoundChannel (void)
{
	OSErr		theErr;
	
	theErr = noErr;
	
	if (!channelOpen)			// Error checking.
		return (theErr);
	
	if (externalChannel != 0L)	// Dispose of channel 1 (if open).
		theErr = SndDisposeChannel(externalChannel, TRUE);
	externalChannel = 0L;		// Flag it closed.
	
	if (externalChannel2 != 0L)	// Dispose of channel 2 (if open).
		theErr = SndDisposeChannel(externalChannel2, TRUE);
	externalChannel2 = 0L;		// Flag it closed.
	
	if (theErr == noErr)
		channelOpen = FALSE;
	
	return (theErr);
}

//--------------------------------------------------------  InitSound

// All the above initialization routines are handled by this one function.
// This single function is the only one that needs to be called - it handles�
// calling the functions that load the sounds and create the sound channels.
// It is called from main() when Glypha is loading up and going through its�
// initialization phase.

void InitSound (void)
{
	OSErr		theErr;
	
	soundOn = TRUE;			// Note that initialization of sounds has occurred�
							// (or rather is just about to this instant!).
	externalChannel = 0L;	// Flag channels as nonexistant.
	externalChannel2 = 0L;
	externalPriority = 0;	// Set priorities to 0 (no sound playing).
	externalPriority2 = 0;
							// Load up all sounds (see above function).
	theErr = LoadBufferSounds();
	if (theErr != noErr)	// If it fails, we'll quit Glypha.
		RedAlert("\pFailed Loading Sounds");
							// Open up the two sound channels.
	theErr = OpenSoundChannel();
	if (theErr != noErr)	// If that fails we'll quit Glypha as well.
		RedAlert("\pFailed To Open Sound Channels");
}

//--------------------------------------------------------  KillSound

// Complementary to the above function, this one is called only when Glypha�
// quits and it handles all the "shut-down" routines.  It also is called from�
// main(), but it is called last - just as Glypha is quitting.

void KillSound (void)
{
	OSErr		theErr;
	
	theErr = DumpBufferSounds();	// Kill all sound pointers.
	theErr = CloseSoundChannel();	// Close down the sound channels.
}

