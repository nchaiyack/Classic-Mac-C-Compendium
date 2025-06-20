#include <sound.h>

#define kAlertSound 128

void AlertSound( void )
{
	Handle		soundHandle;
	OSErr		err;
	short		i;
	
	soundHandle = GetResource( 'snd ', kAlertSound );
	
	HLock( soundHandle );	// not really nec. but I feel safer.. 
	for( i=0 ; i<1 ; i++ )
	{
		err = SndPlay( nil, soundHandle, false );
	}
	HPurge( soundHandle );
	HUnlock( soundHandle );
}
