/* test it.c */

void main( void )
{
	if( DS_InitSound() != noErr )			// Allocate soundchannels, etc.
		SysBeep(0);
	else
	{
		DS_LoadMusic( "\ploopback" );		// load in the music.
		DS_SetMusicFlag( true );			// start the music.
	
		DS_SoundPlay( "\pforce", false );	// play a sound.
		DS_WaitForQuiet();					// wait for that sound to end.
		
		DS_SoundPlay( "\pfire", true );		// play a sound, clober existing sound.
		DS_SoundPlay( "\pforce", false );	// play another sound, but don't clobber
											// any existing sound. (you won't hear
											// this one because "fire" will still
											// be playing when this line is executed,
											// and it will be ignored (becase a sound
											// is playing)).
			
		while( !Button() )					// wait for a mouseclick to end.
			;
			
		DS_DisposeSound();					// lose the memory.
	}
}
