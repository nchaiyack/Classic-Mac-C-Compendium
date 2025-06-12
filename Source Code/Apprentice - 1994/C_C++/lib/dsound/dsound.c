/* dsound.c */
/*
DS_soundLib is a small, simple sound-utilities library created so I don't have to
re-invent the wheel each time I wan't to incorporate asynchronous sound into a
program.

This library includes the ability to play asynchronous sounds, including a sound
track (background music that endlessly repeats).  Beyond the sound track, only one 
sound can be played at a time.  If this isn't enough, it shouldn't be difficult to 
hack this code to give more.

An asynchronous sound is when the mac will play a sound in tandem with other processing,
so your program could play a sound and keep working while the sound is playing.
This is especially useful for game development, where the action doesn't stop while
waiting for a sound to finish.

Though functional, this library leaves much to be desired: with the exception of the 
music, each time you play a sound (via DS_SoundPlay), it is loaded in from the resource
fork.  It works, but if a sound is used frequently it would be more efficient to load
it early and just reference it for playing (rather that loading/disposing of it each time).

In any case, hopefully this code is clear enough that you can adapt it to your needs.
*/

/* INCLUDES */
#include <sound.h>

/* DEFINES */
#define	NoSynth		0		// no specified synth for channel 
#define	InitNone	0		// no specified init parameters for channel 

/* MY MASTER DATA STRUCTURE */
struct ds_data
{
	SndChannelPtr		soundChannel;		// the standard sound channel
	SndChannelPtr		musicChannel;		// the soundtrack sound channel
	
	Handle				musicHandle;		// a handle to a loaded music snd
	
	Boolean				soundChannelActive;	// sound playing now?
	Handle				soundHandle;		// a handle to the sound being played
	
	Boolean				musicFlag;			// music on/off
	Boolean				soundFlag;			// sound on/off
};

/* GLOBALS */
struct ds_data gDS_DATA;

/* FUNCTION PROTOTYPES */
OSErr	DS_InitSound( void );
void	DS_DisposeSound( void );
void	DS_SetSoundFlag( short flag );
void	DS_SetMusicFlag( short flag );			// start music?
void	DS_LoadMusic( Str255 musicRsrcName );	// clobber existing?
void	DS_MusicPlay( void );					// called internally, don't use!!
void	DS_SoundPlay( Str255 soundRsrcName, short clobber );
Boolean	DS_SoundBusy( void );
void	DS_WaitForQuiet( void );
void	DS_ClobberSound( void );
void	DS_ClobberMusic( void );
pascal void 	DS_SoundCallBack( SndChannel *Chan, SndCommand cmd );
pascal void 	DS_MusicCallBack( SndChannel *Chan, SndCommand cmd );

/* FUNCTIONS */

//************************************************************************************************
//***
//***	DS_InitSound
//***
//***	Allocate sound channels (checking for err!), and set defaults.
//***
OSErr DS_InitSound( void )
{
	OSErr err;
		
		// allocate the sound channel.  if an err occurs, return it.
		// note the final parameter.  this is a 'callback,' a pointer
		// to a function which is called after a sound finishes.  in this
		// case, the callback releases the sound and frees the channel
		// for another sound to play on it. (see DS_SoundCallBack for
		// specifics.)
	err = SndNewChannel( &gDS_DATA.soundChannel, NoSynth, InitNone, 
								(SndCallBackProcPtr)DS_SoundCallBack );
	if( err != noErr )
		return( err );
		
		// allocate the music channel.  if an err occurs, return it.
		// this callback routing merely starts the sound again, thus
		// once the sound starts playing, it keeps looping forever
		// without any fuss.  to stop it, use DS_SetMusicFlag to set
  		// note that the sound doesn't actually get played until
  		// DS_MusicPlay starts it up.
  	err = SndNewChannel( &gDS_DATA.musicChannel, NoSynth, InitNone, 
								(SndCallBackProcPtr)DS_MusicCallBack );
	if( err != noErr )
		return( err );

	gDS_DATA.soundChannelActive = false;	// set defaults.
	gDS_DATA.musicFlag = false;		
	gDS_DATA.soundFlag = true;
	gDS_DATA.musicHandle = nil;
	
	return( noErr );						// if we got this far, everything is cool.
}
	
//************************************************************************************************
//***
//***	DS_DisposeSound
//***
//***	quiet the sound, release memory, ignore errors (not much you can do at this point!).
//***
void DS_DisposeSound( void )
{
	OSErr err;
	SndCommand	mySndCmd;
	
	mySndCmd.cmd = 3;			// quietCmd 
	
	err = SndDoCommand( gDS_DATA.soundChannel, &mySndCmd, false );
	err = SndDisposeChannel( gDS_DATA.soundChannel, true );
	
	err = SndDoCommand( gDS_DATA.musicChannel, &mySndCmd, false );
	err = SndDisposeChannel( gDS_DATA.musicChannel, true );
}

//************************************************************************************************
//***
//***	DS_SetSoundFlag
//***
//***	easy on/off sound toggle (only the DS_ library obeys this!)
//***
void DS_SetSoundFlag( short flag )
{
	gDS_DATA.soundFlag = flag;
}

//************************************************************************************************
//***
//***	DS_SetMusicFlag
//***
//***	music on/off, starts or ends music.  note: use DS_LoadMusic to prep for this!
//***	note: this will wait for the sound peice to end before silencing, use
//***	DS_ClobberMusic to kill it immediately.
//***
void DS_SetMusicFlag( short flag )
{
	SndCommand		mySndCmd;
	OSErr			err;

	gDS_DATA.musicFlag = flag;
	
		// if the flag is set to true, start up the sound.  it will automatically repeat.
	if( flag )
		DS_MusicPlay();				// crank it up!
}

//************************************************************************************************
//***
//***	DS_LoadMusic
//***
//***	load in the music rsrc, set up for play.  use DS_SetMusicFlag to toggle music on/off.
//***
void DS_LoadMusic( Str255 musicRsrcName )
{
	DS_ClobberMusic();				// unlock any current music.. (it does err checking)
	
		// load in the music from the sound resource passed in (use "\pSoundName" convention).
	gDS_DATA.musicHandle = GetNamedResource( 'snd ', musicRsrcName );

		// assuming the resource was loaded successfully (musicHandle isn't nil),
		// load it into high memory and lock it down.
	if( gDS_DATA.musicHandle )	// watch out for nil! (eg. rsrc doesn't exist...)
	{
		LoadResource( gDS_DATA.musicHandle );	// load it in..
		MoveHHi( gDS_DATA.musicHandle );		// get it out of the way..
		HLock( gDS_DATA.musicHandle );			// lock it down..
		HPurge( gDS_DATA.musicHandle );			// once it's unlocked its free to go.
	}
}

//************************************************************************************************
//***
//***	DS_MusicPlay
//***
//***	this is the recursive music play routine.  it should only be called internally!
//***
void DS_MusicPlay( void )
{
	SndCommand		mySndCmd;
	OSErr			err;
		
		// check to see that music is on (musicFlag) and that it was loaded 
		// (musicHandle).  If so, play it with instructions to use the callback
		// command.
	if( gDS_DATA.musicFlag && gDS_DATA.musicHandle )	// music on and exists?
	{
			// begin playing the sound.
		err = SndPlay( gDS_DATA.musicChannel, gDS_DATA.musicHandle, true );
		mySndCmd.cmd = 13;			// callBackCmd 
		mySndCmd.param1 = 99; 		// arbitrary 
		mySndCmd.param2 = SetCurrentA5();	
			// instruct the sound to use the callback when done.
		err = SndDoCommand( gDS_DATA.musicChannel, &mySndCmd, false );
	}
}

//************************************************************************************************
//***
//***	DS_SoundPlay
//***
//*** quiet any current sound, set up for new, and go!
//***
void DS_SoundPlay( Str255 soundRsrcName, short clobber )
{
	SndCommand		mySndCmd;
	OSErr			err;

	if( gDS_DATA.soundFlag )			// make sure sound is on before using it.
	{
		if( (gDS_DATA.soundChannelActive && clobber) 
			|| !(gDS_DATA.soundChannelActive) )	// release (unlock) mem before we forget!
		{
			DS_ClobberSound();			// if there is an existing sound, quiet it.
		
				// load in the sound resource. the Str255 should look like "\pSoundName"
			gDS_DATA.soundHandle = GetNamedResource( 'snd ', soundRsrcName );
		
				// if it exists, load it into high memory and lock it down.  call HPurge
				// so that once it's unlocked it can be removed from the heap.
				// play the sound and instruct it to use the callback when done.
			if( gDS_DATA.soundHandle )	// watch out for nil! (eg. rsrc doesn't exist...)
			{
				LoadResource( gDS_DATA.soundHandle );		// load it in..
				MoveHHi( gDS_DATA.soundHandle );			// move it out of the way..
				HLock( gDS_DATA.soundHandle );				// lock it down..
				HPurge( gDS_DATA.soundHandle );				// once its unlocked, its free to go.
				err = SndPlay( gDS_DATA.soundChannel, gDS_DATA.soundHandle, true );
				gDS_DATA.soundChannelActive = true;
				mySndCmd.cmd = 13;			// callBackCmd 
				mySndCmd.param1 = 99; 		// arbitrary 
				mySndCmd.param2 = SetCurrentA5();
				err = SndDoCommand( gDS_DATA.soundChannel, &mySndCmd, false );
			}
		}
	}
}

//************************************************************************************************
//***
//***	DS_SoundBusy
//***
//*** 	is a sound currently being produced?
//***
Boolean DS_SoundBusy( void )
{
	return( gDS_DATA.soundChannelActive );
}

//************************************************************************************************
//***
//***	DS_WaitForQuiet
//***
//*** 	wait for the sound to end.
//***
void	DS_WaitForQuiet( void )
{
	while( gDS_DATA.soundChannelActive )
		;
}


//************************************************************************************************
//***
//***	DS_ClobberSound
//***
//*** 	if sound is active, silence it, then unlock the sound for purging.
//***
void DS_ClobberSound( void )
{
	SndCommand	mySndCmd;
	OSErr		err;
	
	if( gDS_DATA.soundChannelActive )		// is a sound being played at the moment?
	{
		mySndCmd.cmd = 3;					// quietCmd 
		
			// quiet the sound channel
		err = SndDoImmediate( gDS_DATA.soundChannel, &mySndCmd );
		
			// unlock the memory, so that it can be purged when the heap is cleaned up.
		HUnlock( gDS_DATA.soundHandle );	// unlock the mem, hopefully the user is careful..
		
		gDS_DATA.soundChannelActive = false;
	}
}

//************************************************************************************************
//***
//***	DS_ClobberMusic
//***
//*** 	unlock the music for purging.
//***
void DS_ClobberMusic( void )
{	
	SndCommand	mySndCmd;
	OSErr		err;
	
	if( gDS_DATA.musicHandle )
	{
		mySndCmd.cmd = 3;					// quietCmd 
		err = SndDoImmediate( gDS_DATA.musicChannel, &mySndCmd );
	
		HUnlock( gDS_DATA.musicHandle );	// unlock the mem, hopefully the user is careful..	
		
		gDS_DATA.musicFlag = false;
	}
}

//************************************************************************************************
//***
//*** SoundCallBack
//***
//*** this is the function that gets called when a sound is done playing.
//*** the SndCommand should containt these parameters for this callback:
//***		mySndCmd.cmd = 13;			// callBackCmd 
//***		mySndCmd.param1 = 99; 		// arbitrary 
//***		mySndCmd.param2 = SetCurrentA5();	
//***
pascal void DS_SoundCallBack( SndChannel *Chan, SndCommand cmd )
{		
	long	myA5;
	if( cmd.param1 == 99 )
	{
				myA5 = SetA5(cmd.param2);
				HUnlock( gDS_DATA.soundHandle );		// unlock the memory.
				gDS_DATA.soundChannelActive = false;	// sound is officially done.
				myA5 = SetA5(myA5);
	}
}

//************************************************************************************************
//***
//*** MusicCallBack
//***
//*** this gets called when the music is done.  It just starts it up again!
//*** the SndCommand should containt these parameters for this callback:
//***		mySndCmd.cmd = 13;			// callBackCmd 
//***		mySndCmd.param1 = 99; 		// arbitrary 
//***		mySndCmd.param2 = SetCurrentA5();	
//***
pascal void DS_MusicCallBack( SndChannel *Chan, SndCommand cmd )
{	
	long	myA5;
	if( cmd.param1 == 99 )
	{
				myA5 = SetA5(cmd.param2);
				DS_MusicPlay();							// play it again sam!
				myA5 = SetA5(myA5);
	}
}
