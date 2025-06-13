#include <Sound.h>

pascal OSErr	HandleNewSound(Point *defaultObjectSize,WEObjectReference objectRef);
pascal OSErr	HandleDisposeSound( WEObjectReference objectRef );
pascal OSErr	HandleDrawSound(Rect *destRect, WEObjectReference objectRef );
pascal Boolean	HandleClickSound(	Point hitPt, 
							short modifiers, 
							long clickTime, 
							WEObjectReference objectRef);
							
SndChannelPtr	CreateNewSoundChannel( void );

#define		kSoundIconID		550

//
// If you want the sound object handler's sound channel to
// be create on initialization, define CREATE_CHANNEL_AT_INIT
// to the value 1.  Define CREATE_CHANNEL_AT_INIT to the value
// 0 if you want the channel to be created when first needed.
//

#define CREATE_CHANNEL_AT_INIT 1